//=======================================================================================
// ____          ____    __    ______     __________   __      __       __        __
// \    \       |    |  |  |  |   _   \  |___    ___| |  |    |  |     /  \      |  |
//  \    \      |    |  |  |  |  |_)   |     |  |     |  |    |  |    /    \     |  |
//   \    \     |    |  |  |  |   _   /      |  |     |  |    |  |   /  /\  \    |  |
//    \    \    |    |  |  |  |  | \  \      |  |     |   \__/   |  /  ____  \   |  |____
//     \    \   |    |  |__|  |__|  \__\     |__|      \________/  /__/    \__\  |_______|
//      \    \  |    |   ________________________________________________________________
//       \    \ |    |  |  ______________________________________________________________|
//        \    \|    |  |  |         __          __     __     __     ______      _______
//         \         |  |  |_____   |  |        |  |   |  |   |  |   |   _  \    /  _____)
//          \        |  |   _____|  |  |        |  |   |  |   |  |   |  | \  \   \_______
//           \       |  |  |        |  |_____   |   \_/   |   |  |   |  |_/  /    _____  |
//            \ _____|  |__|        |________|   \_______/    |__|   |______/    (_______/
//
//  This file is part of VirtualFluids. VirtualFluids is free software: you can
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  VirtualFluids is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with VirtualFluids (see COPYING.txt). If not, see <http://www.gnu.org/licenses/>.
//
//! \file MultiphasePressureFilterLBMKernel.cpp
//! \ingroup LBMKernel
//! \author M. Geier, K. Kutscher, Hesameddin Safari
//=======================================================================================

#include "MultiphasePressureFilterLBMKernel.h"
#include "BCArray3D.h"
#include "Block3D.h"
#include "D3Q27EsoTwist3DSplittedVector.h"
#include "D3Q27System.h"
#include "DataSet3D.h"
#include "LBMKernel.h"
#include <cmath>

#define PROOF_CORRECTNESS

//////////////////////////////////////////////////////////////////////////
MultiphasePressureFilterLBMKernel::MultiphasePressureFilterLBMKernel() { this->compressible = false; }
//////////////////////////////////////////////////////////////////////////
void MultiphasePressureFilterLBMKernel::initDataSet()
{
	SPtr<DistributionArray3D> f(new D3Q27EsoTwist3DSplittedVector( nx[0] + 4, nx[1] + 4, nx[2] + 4, 0.0));
	SPtr<DistributionArray3D> h(new D3Q27EsoTwist3DSplittedVector( nx[0] + 4, nx[1] + 4, nx[2] + 4, 0.0)); // For phase-field

	//SPtr<PhaseFieldArray3D> divU1(new PhaseFieldArray3D(            nx[0] + 4, nx[1] + 4, nx[2] + 4, 0.0));
	CbArray3D<LBMReal, IndexerX3X2X1>::CbArray3DPtr pressure(new  CbArray3D<LBMReal, IndexerX3X2X1>(    nx[0] + 4, nx[1] + 4, nx[2] + 4, 0.0));
	pressureOld = CbArray3D<LBMReal, IndexerX3X2X1>::CbArray3DPtr(new  CbArray3D<LBMReal, IndexerX3X2X1>(nx[0] + 4, nx[1] + 4, nx[2] + 4, 0.0));
	dataSet->setFdistributions(f);
	dataSet->setHdistributions(h); // For phase-field
	//dataSet->setPhaseField(divU1);
	dataSet->setPressureField(pressure);

	phaseField = CbArray3D<LBMReal, IndexerX3X2X1>::CbArray3DPtr(new CbArray3D<LBMReal, IndexerX3X2X1>(nx[0] + 4, nx[1] + 4, nx[2] + 4, 0.0));

	divU = CbArray3D<LBMReal, IndexerX3X2X1>::CbArray3DPtr(new CbArray3D<LBMReal, IndexerX3X2X1>(nx[0] + 4, nx[1] + 4, nx[2] + 4, 0.0));
}
//////////////////////////////////////////////////////////////////////////
SPtr<LBMKernel> MultiphasePressureFilterLBMKernel::clone()
{
	SPtr<LBMKernel> kernel(new MultiphasePressureFilterLBMKernel());
	kernel->setNX(nx);
	dynamicPointerCast<MultiphasePressureFilterLBMKernel>(kernel)->initDataSet();
	kernel->setCollisionFactorMultiphase(this->collFactorL, this->collFactorG);
	kernel->setDensityRatio(this->densityRatio);
	kernel->setMultiphaseModelParameters(this->beta, this->kappa);
	kernel->setContactAngle(this->contactAngle);
	kernel->setPhiL(this->phiL);
	kernel->setPhiH(this->phiH);
	kernel->setPhaseFieldRelaxation(this->tauH);
	kernel->setMobility(this->mob);
	kernel->setInterfaceWidth(this->interfaceWidth);

	kernel->setBCProcessor(bcProcessor->clone(kernel));
	kernel->setWithForcing(withForcing);
	kernel->setForcingX1(muForcingX1);
	kernel->setForcingX2(muForcingX2);
	kernel->setForcingX3(muForcingX3);
	kernel->setIndex(ix1, ix2, ix3);
	kernel->setDeltaT(deltaT);
	kernel->setGhostLayerWidth(2);
	dynamicPointerCast<MultiphasePressureFilterLBMKernel>(kernel)->initForcing();
    dynamicPointerCast<MultiphasePressureFilterLBMKernel>(kernel)->setPhaseFieldBC(this->phaseFieldBC);

	return kernel;
}
//////////////////////////////////////////////////////////////////////////
void  MultiphasePressureFilterLBMKernel::forwardInverseChimeraWithKincompressible(LBMReal& mfa, LBMReal& mfb, LBMReal& mfc, LBMReal vv, LBMReal v2, LBMReal Kinverse, LBMReal K, LBMReal oneMinusRho) {
	using namespace UbMath;
	LBMReal m2 = mfa + mfc;
	LBMReal m1 = mfc - mfa;
	LBMReal m0 = m2 + mfb;
	mfa = m0;
	m0 *= Kinverse;
	m0 += oneMinusRho;
	mfb = (m1 * Kinverse - m0 * vv) * K;
	mfc = ((m2 - c2 * m1 * vv) * Kinverse + v2 * m0) * K;
}

////////////////////////////////////////////////////////////////////////////////
void  MultiphasePressureFilterLBMKernel::backwardInverseChimeraWithKincompressible(LBMReal& mfa, LBMReal& mfb, LBMReal& mfc, LBMReal vv, LBMReal v2, LBMReal Kinverse, LBMReal K, LBMReal oneMinusRho) {
	using namespace UbMath;
	LBMReal m0 = (((mfc - mfb) * c1o2 + mfb * vv) * Kinverse + (mfa * Kinverse + oneMinusRho) * (v2 - vv) * c1o2) * K;
	LBMReal m1 = (((mfa - mfc) - c2 * mfb * vv) * Kinverse + (mfa * Kinverse + oneMinusRho) * (-v2)) * K;
	mfc = (((mfc + mfb) * c1o2 + mfb * vv) * Kinverse + (mfa * Kinverse + oneMinusRho) * (v2 + vv) * c1o2) * K;
	mfa = m0;
	mfb = m1;
}


////////////////////////////////////////////////////////////////////////////////
void  MultiphasePressureFilterLBMKernel::forwardChimera(LBMReal& mfa, LBMReal& mfb, LBMReal& mfc, LBMReal vv, LBMReal v2) {
	using namespace UbMath;
	LBMReal m1 = (mfa + mfc) + mfb;
	LBMReal m2 = mfc - mfa;
	mfc = (mfc + mfa) + (v2 * m1 - c2 * vv * m2);
	mfb = m2 - vv * m1;
	mfa = m1;
}


void  MultiphasePressureFilterLBMKernel::backwardChimera(LBMReal& mfa, LBMReal& mfb, LBMReal& mfc, LBMReal vv, LBMReal v2) {
	using namespace UbMath;
	LBMReal ma = (mfc + mfa * (v2 - vv)) * c1o2 + mfb * (vv - c1o2);
	LBMReal mb = ((mfa - mfc) - mfa * v2) - c2 * mfb * vv;
	mfc = (mfc + mfa * (v2 + vv)) * c1o2 + mfb * (vv + c1o2);
	mfb = mb;
	mfa = ma;
}


void MultiphasePressureFilterLBMKernel::calculate(int step)
{
	using namespace D3Q27System;
	using namespace UbMath;

	forcingX1 = 0.0;
	forcingX2 = 0.0;
	forcingX3 = 0.0;

	LBMReal oneOverInterfaceScale = c4 / interfaceWidth; //1.0;//1.5;
														 /////////////////////////////////////

	localDistributionsF    = dynamicPointerCast<D3Q27EsoTwist3DSplittedVector>(dataSet->getFdistributions())->getLocalDistributions();
	nonLocalDistributionsF = dynamicPointerCast<D3Q27EsoTwist3DSplittedVector>(dataSet->getFdistributions())->getNonLocalDistributions();
	zeroDistributionsF     = dynamicPointerCast<D3Q27EsoTwist3DSplittedVector>(dataSet->getFdistributions())->getZeroDistributions();

	localDistributionsH1    = dynamicPointerCast<D3Q27EsoTwist3DSplittedVector>(dataSet->getHdistributions())->getLocalDistributions();
	nonLocalDistributionsH1 = dynamicPointerCast<D3Q27EsoTwist3DSplittedVector>(dataSet->getHdistributions())->getNonLocalDistributions();
	zeroDistributionsH1     = dynamicPointerCast<D3Q27EsoTwist3DSplittedVector>(dataSet->getHdistributions())->getZeroDistributions();

	CbArray3D<LBMReal, IndexerX3X2X1>::CbArray3DPtr pressure = dataSet->getPressureField();

	SPtr<BCArray3D> bcArray = this->getBCProcessor()->getBCArray();

	const int bcArrayMaxX1 = (int)bcArray->getNX1();
	const int bcArrayMaxX2 = (int)bcArray->getNX2();
	const int bcArrayMaxX3 = (int)bcArray->getNX3();

	int minX1 = ghostLayerWidth;
	int minX2 = ghostLayerWidth;
	int minX3 = ghostLayerWidth;
	int maxX1 = bcArrayMaxX1 - ghostLayerWidth;
	int maxX2 = bcArrayMaxX2 - ghostLayerWidth;
	int maxX3 = bcArrayMaxX3 - ghostLayerWidth;

	for (int x3 = minX3-ghostLayerWidth; x3 < maxX3+ghostLayerWidth; x3++) {
		for (int x2 = minX2-ghostLayerWidth; x2 < maxX2+ghostLayerWidth; x2++) {
			for (int x1 = minX1-ghostLayerWidth; x1 < maxX1+ghostLayerWidth; x1++) {
				if (!bcArray->isSolid(x1, x2, x3) && !bcArray->isUndefined(x1, x2, x3)) {
					int x1p = x1 + 1;
					int x2p = x2 + 1;
					int x3p = x3 + 1;

					LBMReal mfcbb = (*this->localDistributionsH1)(D3Q27System::ET_E, x1, x2, x3);
					LBMReal mfbcb = (*this->localDistributionsH1)(D3Q27System::ET_N, x1, x2, x3);
					LBMReal mfbbc = (*this->localDistributionsH1)(D3Q27System::ET_T, x1, x2, x3);
					LBMReal mfccb = (*this->localDistributionsH1)(D3Q27System::ET_NE, x1, x2, x3);
					LBMReal mfacb = (*this->localDistributionsH1)(D3Q27System::ET_NW, x1p, x2, x3);
					LBMReal mfcbc = (*this->localDistributionsH1)(D3Q27System::ET_TE, x1, x2, x3);
					LBMReal mfabc = (*this->localDistributionsH1)(D3Q27System::ET_TW, x1p, x2, x3);
					LBMReal mfbcc = (*this->localDistributionsH1)(D3Q27System::ET_TN, x1, x2, x3);
					LBMReal mfbac = (*this->localDistributionsH1)(D3Q27System::ET_TS, x1, x2p, x3);
					LBMReal mfccc = (*this->localDistributionsH1)(D3Q27System::ET_TNE, x1, x2, x3);
					LBMReal mfacc = (*this->localDistributionsH1)(D3Q27System::ET_TNW, x1p, x2, x3);
					LBMReal mfcac = (*this->localDistributionsH1)(D3Q27System::ET_TSE, x1, x2p, x3);
					LBMReal mfaac = (*this->localDistributionsH1)(D3Q27System::ET_TSW, x1p, x2p, x3);
					LBMReal mfabb = (*this->nonLocalDistributionsH1)(D3Q27System::ET_W, x1p, x2, x3);
					LBMReal mfbab = (*this->nonLocalDistributionsH1)(D3Q27System::ET_S, x1, x2p, x3);
					LBMReal mfbba = (*this->nonLocalDistributionsH1)(D3Q27System::ET_B, x1, x2, x3p);
					LBMReal mfaab = (*this->nonLocalDistributionsH1)(D3Q27System::ET_SW, x1p, x2p, x3);
					LBMReal mfcab = (*this->nonLocalDistributionsH1)(D3Q27System::ET_SE, x1, x2p, x3);
					LBMReal mfaba = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BW, x1p, x2, x3p);
					LBMReal mfcba = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BE, x1, x2, x3p);
					LBMReal mfbaa = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BS, x1, x2p, x3p);
					LBMReal mfbca = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BN, x1, x2, x3p);
					LBMReal mfaaa = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BSW, x1p, x2p, x3p);
					LBMReal mfcaa = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BSE, x1, x2p, x3p);
					LBMReal mfaca = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BNW, x1p, x2, x3p);
					LBMReal mfcca = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BNE, x1, x2, x3p);

					LBMReal mfbbb = (*this->zeroDistributionsH1)(x1, x2, x3);
					(*phaseField)(x1, x2, x3) = (((mfaaa + mfccc) + (mfaca + mfcac)) + ((mfaac + mfcca)  + (mfcaa + mfacc))  ) +
						(((mfaab + mfacb) + (mfcab + mfccb)) + ((mfaba + mfabc) + (mfcba + mfcbc)) +
							((mfbaa + mfbac) + (mfbca + mfbcc))) + ((mfabb + mfcbb) +
								(mfbab + mfbcb) + (mfbba + mfbbc)) + mfbbb;

					////// read F-distributions for velocity formalism

					mfcbb = (*this->localDistributionsF)(D3Q27System::ET_E, x1, x2, x3);
					mfbcb = (*this->localDistributionsF)(D3Q27System::ET_N, x1, x2, x3);
					mfbbc = (*this->localDistributionsF)(D3Q27System::ET_T, x1, x2, x3);
					mfccb = (*this->localDistributionsF)(D3Q27System::ET_NE, x1, x2, x3);
					mfacb = (*this->localDistributionsF)(D3Q27System::ET_NW, x1p, x2, x3);
					mfcbc = (*this->localDistributionsF)(D3Q27System::ET_TE, x1, x2, x3);
					mfabc = (*this->localDistributionsF)(D3Q27System::ET_TW, x1p, x2, x3);
					mfbcc = (*this->localDistributionsF)(D3Q27System::ET_TN, x1, x2, x3);
					mfbac = (*this->localDistributionsF)(D3Q27System::ET_TS, x1, x2p, x3);
					mfccc = (*this->localDistributionsF)(D3Q27System::ET_TNE, x1, x2, x3);
					mfacc = (*this->localDistributionsF)(D3Q27System::ET_TNW, x1p, x2, x3);
					mfcac = (*this->localDistributionsF)(D3Q27System::ET_TSE, x1, x2p, x3);
					mfaac = (*this->localDistributionsF)(D3Q27System::ET_TSW, x1p, x2p, x3);
					mfabb = (*this->nonLocalDistributionsF)(D3Q27System::ET_W, x1p, x2, x3);
					mfbab = (*this->nonLocalDistributionsF)(D3Q27System::ET_S, x1, x2p, x3);
					mfbba = (*this->nonLocalDistributionsF)(D3Q27System::ET_B, x1, x2, x3p);
					mfaab = (*this->nonLocalDistributionsF)(D3Q27System::ET_SW, x1p, x2p, x3);
					mfcab = (*this->nonLocalDistributionsF)(D3Q27System::ET_SE, x1, x2p, x3);
					mfaba = (*this->nonLocalDistributionsF)(D3Q27System::ET_BW, x1p, x2, x3p);
					mfcba = (*this->nonLocalDistributionsF)(D3Q27System::ET_BE, x1, x2, x3p);
					mfbaa = (*this->nonLocalDistributionsF)(D3Q27System::ET_BS, x1, x2p, x3p);
					mfbca = (*this->nonLocalDistributionsF)(D3Q27System::ET_BN, x1, x2, x3p);
					mfaaa = (*this->nonLocalDistributionsF)(D3Q27System::ET_BSW, x1p, x2p, x3p);
					mfcaa = (*this->nonLocalDistributionsF)(D3Q27System::ET_BSE, x1, x2p, x3p);
					mfaca = (*this->nonLocalDistributionsF)(D3Q27System::ET_BNW, x1p, x2, x3p);
					mfcca = (*this->nonLocalDistributionsF)(D3Q27System::ET_BNE, x1, x2, x3p);

					mfbbb = (*this->zeroDistributionsF)(x1, x2, x3);

					LBMReal rhoH = 1.0;
					LBMReal rhoL = 1.0 / densityRatio;

					LBMReal rhoToPhi = (rhoH - rhoL) / (phiH - phiL);

					LBMReal drho = (mfaaa + mfaac + mfaca + mfcaa + mfacc + mfcac + mfccc + mfcca)
						+ (mfaab + mfacb + mfcab + mfccb) + (mfaba + mfabc + mfcba + mfcbc) + (mfbaa + mfbac + mfbca + mfbcc)
						+ (mfabb + mfcbb) + (mfbab + mfbcb) + (mfbba + mfbbc) + mfbbb;

					LBMReal rho = rhoH + rhoToPhi * ((*phaseField)(x1, x2, x3) - phiH);

					(*pressureOld)(x1, x2, x3) = (*pressure)(x1, x2, x3) + rho * c1o3 * drho;
				}
			}
		}
	}

	LBMReal collFactorM;

	////Periodic Filter
	for (int x3 = minX3-1; x3 <= maxX3; x3++) {
		for (int x2 = minX2-1; x2 <= maxX2; x2++) {
			for (int x1 = minX1-1; x1 <= maxX1; x1++) {
				if (!bcArray->isSolid(x1, x2, x3) && !bcArray->isUndefined(x1, x2, x3)) {

					LBMReal sum = 0.;

					///Version for boundaries
					for (int xx = -1; xx <= 1; xx++) {
						//int xxx = (xx+x1 <= maxX1) ? ((xx + x1 > 0) ? xx + x1 : maxX1) : 0;
						int xxx = xx + x1;

						for (int yy = -1; yy <= 1; yy++) {
							//int yyy = (yy+x2 <= maxX2) ?( (yy + x2 > 0) ? yy + x2 : maxX2) : 0;
							int yyy = yy + x2;

							for (int zz = -1; zz <= 1; zz++) {
								//int zzz = (zz+x3 <= maxX3) ? zzz = ((zz + x3 > 0) ? zz + x3 : maxX3 ): 0;
								int zzz = zz + x3;

								if (!bcArray->isSolid(xxx, yyy, zzz) && !bcArray->isUndefined(xxx, yyy, zzz)) {
									sum+= 64.0/(216.0*(c1+c3*abs(xx))* (c1 + c3 * abs(yy))* (c1 + c3 * abs(zz)))*(*pressureOld)(xxx, yyy, zzz);
								}
								else{ sum+= 64.0 / (216.0 * (c1 + c3 * abs(xx)) * (c1 + c3 * abs(yy)) * (c1 + c3 * abs(zz))) * (*pressureOld)(x1, x2, x3);
								}


							}
						}
					}
					(*pressure)(x1, x2, x3) = sum;
				}
			}
		}
	}

	////!filter

	for (int x3 = minX3; x3 < maxX3; x3++) {
		for (int x2 = minX2; x2 < maxX2; x2++) {
			for (int x1 = minX1; x1 < maxX1; x1++) {
				if (!bcArray->isSolid(x1, x2, x3) && !bcArray->isUndefined(x1, x2, x3)) {
					int x1p = x1 + 1;
					int x2p = x2 + 1;
					int x3p = x3 + 1;

					//////////////////////////////////////////////////////////////////////////
					// Read distributions and phase field
					////////////////////////////////////////////////////////////////////////////
					//////////////////////////////////////////////////////////////////////////

					// E   N  T
					// c   c  c
					//////////
					// W   S  B
					// a   a  a

					// Rest ist b

					// mfxyz
					// a - negative
					// b - null
					// c - positive

					// a b c
					//-1 0 1

					findNeighbors(phaseField, x1, x2, x3);

					LBMReal mfcbb = (*this->localDistributionsF)(D3Q27System::ET_E, x1, x2, x3);
					LBMReal mfbcb = (*this->localDistributionsF)(D3Q27System::ET_N, x1, x2, x3);
					LBMReal mfbbc = (*this->localDistributionsF)(D3Q27System::ET_T, x1, x2, x3);
					LBMReal mfccb = (*this->localDistributionsF)(D3Q27System::ET_NE, x1, x2, x3);
					LBMReal mfacb = (*this->localDistributionsF)(D3Q27System::ET_NW, x1p, x2, x3);
					LBMReal mfcbc = (*this->localDistributionsF)(D3Q27System::ET_TE, x1, x2, x3);
					LBMReal mfabc = (*this->localDistributionsF)(D3Q27System::ET_TW, x1p, x2, x3);
					LBMReal mfbcc = (*this->localDistributionsF)(D3Q27System::ET_TN, x1, x2, x3);
					LBMReal mfbac = (*this->localDistributionsF)(D3Q27System::ET_TS, x1, x2p, x3);
					LBMReal mfccc = (*this->localDistributionsF)(D3Q27System::ET_TNE, x1, x2, x3);
					LBMReal mfacc = (*this->localDistributionsF)(D3Q27System::ET_TNW, x1p, x2, x3);
					LBMReal mfcac = (*this->localDistributionsF)(D3Q27System::ET_TSE, x1, x2p, x3);
					LBMReal mfaac = (*this->localDistributionsF)(D3Q27System::ET_TSW, x1p, x2p, x3);
					LBMReal mfabb = (*this->nonLocalDistributionsF)(D3Q27System::ET_W, x1p, x2, x3);
					LBMReal mfbab = (*this->nonLocalDistributionsF)(D3Q27System::ET_S, x1, x2p, x3);
					LBMReal mfbba = (*this->nonLocalDistributionsF)(D3Q27System::ET_B, x1, x2, x3p);
					LBMReal mfaab = (*this->nonLocalDistributionsF)(D3Q27System::ET_SW, x1p, x2p, x3);
					LBMReal mfcab = (*this->nonLocalDistributionsF)(D3Q27System::ET_SE, x1, x2p, x3);
					LBMReal mfaba = (*this->nonLocalDistributionsF)(D3Q27System::ET_BW, x1p, x2, x3p);
					LBMReal mfcba = (*this->nonLocalDistributionsF)(D3Q27System::ET_BE, x1, x2, x3p);
					LBMReal mfbaa = (*this->nonLocalDistributionsF)(D3Q27System::ET_BS, x1, x2p, x3p);
					LBMReal mfbca = (*this->nonLocalDistributionsF)(D3Q27System::ET_BN, x1, x2, x3p);
					LBMReal mfaaa = (*this->nonLocalDistributionsF)(D3Q27System::ET_BSW, x1p, x2p, x3p);
					LBMReal mfcaa = (*this->nonLocalDistributionsF)(D3Q27System::ET_BSE, x1, x2p, x3p);
					LBMReal mfaca = (*this->nonLocalDistributionsF)(D3Q27System::ET_BNW, x1p, x2, x3p);
					LBMReal mfcca = (*this->nonLocalDistributionsF)(D3Q27System::ET_BNE, x1, x2, x3p);

					LBMReal mfbbb = (*this->zeroDistributionsF)(x1, x2, x3);

					LBMReal rhoH = 1.0;
					LBMReal rhoL = 1.0 / densityRatio;

					LBMReal rhoToPhi = (rhoH - rhoL) / (phiH - phiL);

					LBMReal dX1_phi = gradX1_phi();
					LBMReal dX2_phi = gradX2_phi();
					LBMReal dX3_phi = gradX3_phi();

					LBMReal denom = sqrt(dX1_phi * dX1_phi + dX2_phi * dX2_phi + dX3_phi * dX3_phi) + 1e-9;
					LBMReal normX1 = dX1_phi / denom;
					LBMReal normX2 = dX2_phi / denom;
					LBMReal normX3 = dX3_phi / denom;

					dX1_phi = normX1 * (1.0 - phi[REST]) * (phi[REST]) * oneOverInterfaceScale;
                    dX2_phi = normX2 * (1.0 - phi[REST]) * (phi[REST]) * oneOverInterfaceScale;
                    dX3_phi = normX3 * (1.0 - phi[REST]) * (phi[REST]) * oneOverInterfaceScale;

					collFactorM = collFactorL + (collFactorL - collFactorG) * (phi[REST] - phiH) / (phiH - phiL);


					LBMReal mu = 2 * beta * phi[REST] * (phi[REST] - 1) * (2 * phi[REST] - 1) - kappa * nabla2_phi();

					//----------- Calculating Macroscopic Values -------------
					LBMReal rho = rhoH + rhoToPhi * (phi[REST] - phiH);

					LBMReal m0, m1, m2;
					LBMReal rhoRef=c1;

					LBMReal vvx = ((((mfccc - mfaaa) + (mfcac - mfaca)) + ((mfcaa - mfacc) + (mfcca - mfaac))) +
						(((mfcba - mfabc) + (mfcbc - mfaba)) + ((mfcab - mfacb) + (mfccb - mfaab))) +
						(mfcbb - mfabb))/rhoRef;
					LBMReal vvy = ((((mfccc - mfaaa) + (mfaca - mfcac)) + ((mfacc - mfcaa) + (mfcca - mfaac))) +
						(((mfbca - mfbac) + (mfbcc - mfbaa)) + ((mfacb - mfcab) + (mfccb - mfaab))) +
						(mfbcb - mfbab))/rhoRef;
					LBMReal vvz = ((((mfccc - mfaaa) + (mfcac - mfaca)) + ((mfacc - mfcaa) + (mfaac - mfcca))) +
						(((mfbac - mfbca) + (mfbcc - mfbaa)) + ((mfabc - mfcba) + (mfcbc - mfaba))) +
						(mfbbc - mfbba))/rhoRef;

					LBMReal gradPx = 0.0;
					LBMReal gradPy = 0.0;
					LBMReal gradPz = 0.0;
					for (int dir1 = -1; dir1 <= 1; dir1++) {
						for (int dir2 = -1; dir2 <= 1; dir2++) {
							int yyy = x2 + dir1;
							int zzz = x3 + dir2;
							if (!bcArray->isSolid(x1-1, yyy, zzz) && !bcArray->isUndefined(x1-1, yyy, zzz)) {
								gradPx -= (*pressure)(x1 - 1, yyy, zzz) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							else {
								gradPx -= (*pressure)(x1, x2, x3) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							if (!bcArray->isSolid(x1 + 1, yyy, zzz) && !bcArray->isUndefined(x1 + 1, yyy, zzz)) {
								gradPx += (*pressure)(x1 + 1, yyy, zzz) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							else {
								gradPx += (*pressure)(x1, x2, x3) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}

							int xxx = x1 + dir1;
							if (!bcArray->isSolid(xxx, x2-1, zzz) && !bcArray->isUndefined(xxx, x2-1, zzz)) {
								gradPy -= (*pressure)(xxx, x2-1, zzz) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							else {
								gradPy -= (*pressure)(x1, x2, x3) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							if (!bcArray->isSolid(xxx, x2+1, zzz) && !bcArray->isUndefined(xxx, x2+1, zzz)) {
								gradPy += (*pressure)(xxx, x2+1, zzz) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							else {
								gradPy += (*pressure)(x1, x2, x3) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}

							yyy = x2 + dir2;
							if (!bcArray->isSolid(xxx, yyy, x3-1) && !bcArray->isUndefined(xxx, yyy, x3-1)) {
								gradPz -= (*pressure)(xxx, yyy, x3-1) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							else {
								gradPz -= (*pressure)(x1, x2, x3) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							if (!bcArray->isSolid(xxx, yyy, x3+1) && !bcArray->isUndefined(xxx, yyy, x3+1)) {
								gradPz += (*pressure)(xxx, yyy, x3+1) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}
							else {
								gradPz += (*pressure)(x1, x2, x3) * c2o9 / ((c1 + c3 * abs(dir1)) * (c1 + c3 * abs(dir2)));
							}

						}
					}

					//Viscosity increase by pressure gradient
					LBMReal errPhi = (((1.0 - phi[REST]) * (phi[REST]) * oneOverInterfaceScale)- denom);
					//LBMReal limVis = 0.0000001*10;//0.01;
					// collFactorM =collFactorM/(c1+limVis*(errPhi*errPhi)*collFactorM);
					// collFactorM = (collFactorM < 1.8) ? 1.8 : collFactorM;
					errPhi = errPhi * errPhi* errPhi * errPhi * errPhi * errPhi;
					//collFactorM = collFactorM + (1.8 - collFactorM) * errPhi / (errPhi + limVis);

					//3.0 * ((WEIGTH[TNE] * (((phi2[TNE] - phi2[BSW]) - (phi2[BSE] - phi2[TNW])) + ((phi2[TSE] - phi2[BNW]) - (phi2[BNE] - phi2[TSW])))
					//+WEIGTH[NE] * (((phi2[TE] - phi2[BW]) - (phi2[BE] - phi2[TW])) + ((phi2[TS] - phi2[BN]) + (phi2[TN] - phi2[BS])))) +
					//+WEIGTH[N] * (phi2[T] - phi2[B]));

					muRho = rho;

					forcingX1 = muForcingX1.Eval()/rho - gradPx/rho;
					forcingX2 = muForcingX2.Eval()/rho - gradPy/rho;
					forcingX3 = muForcingX3.Eval()/rho - gradPz/rho;

					forcingX1 += mu * dX1_phi / rho;
                    forcingX2 += mu * dX2_phi / rho;
                    forcingX3 += mu * dX3_phi / rho;

					vvx += forcingX1 * deltaT * 0.5; // X
					vvy += forcingX2 * deltaT * 0.5; // Y
					vvz += forcingX3 * deltaT * 0.5; // Z

                    ///surface tension force
					//vvx += mu * dX1_phi * c1o2 / rho;
					//vvy += mu * dX2_phi * c1o2 / rho ;
					//vvz += mu * dX3_phi * c1o2 / rho;

					//Abbas
					LBMReal pStar = ((((((mfaaa + mfccc) + (mfaac + mfcca)) + ((mfcac + mfaca) + (mfcaa + mfacc)))
						+ (((mfaab + mfccb) + (mfacb + mfcab)) + ((mfaba + mfcbc) + (mfabc + mfcba)) + ((mfbaa + mfbcc) + (mfbac + mfbca))))
						+ ((mfabb + mfcbb) + (mfbab + mfbcb) + (mfbba + mfbbc))) + mfbbb) * c1o3;

					LBMReal M200 = ((((((mfaaa + mfccc) + (mfaac + mfcca)) + ((mfcac + mfaca) + (mfcaa + mfacc)))
						+ (((mfaab + mfccb) + (mfacb + mfcab)) + ((mfaba + mfcbc) + (mfabc + mfcba))))
						+ ((mfabb + mfcbb))));
					LBMReal M020 = ((((((mfaaa + mfccc) + (mfaac + mfcca)) + ((mfcac + mfaca) + (mfcaa + mfacc)))
						+ (((mfaab + mfccb) + (mfacb + mfcab)) + ((mfbaa + mfbcc) + (mfbac + mfbca))))
						+ ((mfbab + mfbcb))));
					LBMReal M002 = ((((((mfaaa + mfccc) + (mfaac + mfcca)) + ((mfcac + mfaca) + (mfcaa + mfacc)))
						+ (+((mfaba + mfcbc) + (mfabc + mfcba)) + ((mfbaa + mfbcc) + (mfbac + mfbca))))
						+ ((mfbba + mfbbc))));

					LBMReal M110 = ((((((mfaaa + mfccc) + (-mfcac - mfaca)) + ((mfaac + mfcca) + (-mfcaa - mfacc)))
						+ (((mfaab + mfccb) + (-mfacb - mfcab))))
						));
					LBMReal M101 = ((((((mfaaa + mfccc) - (mfaac + mfcca)) + ((mfcac + mfaca) - (mfcaa + mfacc)))
						+ (((mfaba + mfcbc) + (-mfabc - mfcba))))
						));
					LBMReal M011 = ((((((mfaaa + mfccc) - (mfaac + mfcca)) + ((mfcaa + mfacc) - (mfcac + mfaca)))
						+ (((mfbaa + mfbcc) + (-mfbac - mfbca))))
						));
					LBMReal vvxI = vvx;
					LBMReal vvyI = vvy;
					LBMReal vvzI = vvz;

					LBMReal collFactorStore = collFactorM;
					LBMReal stress;
					for (int iter = 0; iter < 1; iter++) {
						LBMReal OxxPyyPzz = 1.0;
						LBMReal mxxPyyPzz = (M200 - vvxI * vvxI) + (M020 - vvyI * vvyI) + (M002 - vvzI * vvzI);
						mxxPyyPzz -= c3 * pStar;

						LBMReal mxxMyy = (M200 - vvxI * vvxI) - (M020 - vvyI * vvyI);
						LBMReal mxxMzz = (M200 - vvxI * vvxI) - (M002 - vvzI * vvzI);
						LBMReal mxy = M110 - vvxI * vvyI;
						LBMReal mxz = M101 - vvxI * vvzI;
						LBMReal myz = M011 - vvyI * vvzI;

						///////Bingham
						//LBMReal dxux = -c1o2 * collFactorM * (mxxMyy + mxxMzz) + c1o2 * OxxPyyPzz * (/*mfaaa*/ -mxxPyyPzz);
						//LBMReal dyuy = dxux + collFactorM * c3o2 * mxxMyy;
						//LBMReal dzuz = dxux + collFactorM * c3o2 * mxxMzz;
						//LBMReal Dxy = -three * collFactorM * mxy;
						//LBMReal Dxz = -three * collFactorM * mxz;
						//LBMReal Dyz = -three * collFactorM * myz;

						//LBMReal tau0 = phi[REST] * 1.0e-7;//(phi[REST]>0.01)?1.0e-6: 0;
						//LBMReal shearRate =fabs(pStar)*0.0e-2+ sqrt(c2 * (dxux * dxux + dyuy * dyuy + dzuz * dzuz) + Dxy * Dxy + Dxz * Dxz + Dyz * Dyz) / (rho);
						//collFactorM = collFactorM * (UbMath::one - (collFactorM * tau0) / (shearRate * c1o3 /* *rho*/ + 1.0e-15));
						//collFactorM = (collFactorM < -1000000) ? -1000000 : collFactorM;
						////if(collFactorM < 0.1) {
						////	int test = 1;
						////}
						//////!Bingham


						mxxMyy *= c1 - collFactorM * c1o2;
						mxxMzz *= c1 - collFactorM * c1o2;
						mxy *= c1 - collFactorM * c1o2;
						mxz *= c1 - collFactorM * c1o2;
						myz *= c1 - collFactorM * c1o2;
						mxxPyyPzz *= c1 - OxxPyyPzz * c1o2;
						//mxxPyyPzz += c3o2 * pStar;
						LBMReal mxx = (mxxMyy + mxxMzz + mxxPyyPzz) * c1o3;
						LBMReal myy = (-c2 * mxxMyy + mxxMzz + mxxPyyPzz) * c1o3;
						LBMReal mzz = (mxxMyy - c2 * mxxMzz + mxxPyyPzz) * c1o3;
						vvxI = vvx - (mxx * dX1_phi + mxy * dX2_phi + mxz * dX3_phi) * rhoToPhi / (rho);
						vvyI = vvy - (mxy * dX1_phi + myy * dX2_phi + myz * dX3_phi) * rhoToPhi / (rho);
						vvzI = vvz - (mxz * dX1_phi + myz * dX2_phi + mzz * dX3_phi) * rhoToPhi / (rho);



					}


					forcingX1 += c2 * (vvxI - vvx);
					forcingX2 += c2 * (vvyI - vvy);
					forcingX3 += c2 * (vvzI - vvz);

					mfabb += c1o2 * (-forcingX1) * c2o9;
					mfbab += c1o2 * (-forcingX2) * c2o9;
					mfbba += c1o2 * (-forcingX3) * c2o9;
					mfaab += c1o2 * (-forcingX1 - forcingX2) * c1o18;
					mfcab += c1o2 * (forcingX1 - forcingX2) * c1o18;
					mfaba += c1o2 * (-forcingX1 - forcingX3) * c1o18;
					mfcba += c1o2 * (forcingX1 - forcingX3) * c1o18;
					mfbaa += c1o2 * (-forcingX2 - forcingX3) * c1o18;
					mfbca += c1o2 * (forcingX2 - forcingX3) * c1o18;
					mfaaa += c1o2 * (-forcingX1 - forcingX2 - forcingX3) * c1o72;
					mfcaa += c1o2 * (forcingX1 - forcingX2 - forcingX3) * c1o72;
					mfaca += c1o2 * (-forcingX1 + forcingX2 - forcingX3) * c1o72;
					mfcca += c1o2 * (forcingX1 + forcingX2 - forcingX3) * c1o72;
					mfcbb += c1o2 * (forcingX1)*c2o9;
					mfbcb += c1o2 * (forcingX2)*c2o9;
					mfbbc += c1o2 * (forcingX3)*c2o9;
					mfccb += c1o2 * (forcingX1 + forcingX2) * c1o18;
					mfacb += c1o2 * (-forcingX1 + forcingX2) * c1o18;
					mfcbc += c1o2 * (forcingX1 + forcingX3) * c1o18;
					mfabc += c1o2 * (-forcingX1 + forcingX3) * c1o18;
					mfbcc += c1o2 * (forcingX2 + forcingX3) * c1o18;
					mfbac += c1o2 * (-forcingX2 + forcingX3) * c1o18;
					mfccc += c1o2 * (forcingX1 + forcingX2 + forcingX3) * c1o72;
					mfacc += c1o2 * (-forcingX1 + forcingX2 + forcingX3) * c1o72;
					mfcac += c1o2 * (forcingX1 - forcingX2 + forcingX3) * c1o72;
					mfaac += c1o2 * (-forcingX1 - forcingX2 + forcingX3) * c1o72;



					vvx = vvxI;
					vvy = vvyI;
					vvz = vvzI;

					//!Abbas


					LBMReal vx2;
					LBMReal vy2;
					LBMReal vz2;
					vx2 = vvx * vvx;
					vy2 = vvy * vvy;
					vz2 = vvz * vvz;
					///////////////////////////////////////////////////////////////////////////////////////////               
					LBMReal oMdrho;


					oMdrho = mfccc + mfaaa;
					m0 = mfaca + mfcac;
					m1 = mfacc + mfcaa;
					m2 = mfaac + mfcca;
					oMdrho += m0;
					m1 += m2;
					oMdrho += m1;
					m0 = mfbac + mfbca;
					m1 = mfbaa + mfbcc;
					m0 += m1;
					m1 = mfabc + mfcba;
					m2 = mfaba + mfcbc;
					m1 += m2;
					m0 += m1;
					m1 = mfacb + mfcab;
					m2 = mfaab + mfccb;
					m1 += m2;
					m0 += m1;
					oMdrho += m0;
					m0 = mfabb + mfcbb;
					m1 = mfbab + mfbcb;
					m2 = mfbba + mfbbc;
					m0 += m1 + m2;
					m0 += mfbbb; //hat gefehlt
					oMdrho = (rhoRef - (oMdrho + m0))/rhoRef;// 12.03.21 check derivation!!!!

															 ////////////////////////////////////////////////////////////////////////////////////
					LBMReal wadjust;
					LBMReal qudricLimit = 0.01;
					////////////////////////////////////////////////////////////////////////////////////
					//Hin
					////////////////////////////////////////////////////////////////////////////////////
					// mit 1/36, 1/9, 1/36, 1/9, 4/9, 1/9, 1/36, 1/9, 1/36  Konditionieren
					////////////////////////////////////////////////////////////////////////////////////
					// Z - Dir
					m2 = mfaaa + mfaac;
					m1 = mfaac - mfaaa;
					m0 = m2 + mfaab;
					mfaaa = m0;
					m0 += c1o36 * oMdrho;
					mfaab = m1 - m0 * vvz;
					mfaac = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfaba + mfabc;
					m1 = mfabc - mfaba;
					m0 = m2 + mfabb;
					mfaba = m0;
					m0 += c1o9 * oMdrho;
					mfabb = m1 - m0 * vvz;
					mfabc = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfaca + mfacc;
					m1 = mfacc - mfaca;
					m0 = m2 + mfacb;
					mfaca = m0;
					m0 += c1o36 * oMdrho;
					mfacb = m1 - m0 * vvz;
					mfacc = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfbaa + mfbac;
					m1 = mfbac - mfbaa;
					m0 = m2 + mfbab;
					mfbaa = m0;
					m0 += c1o9 * oMdrho;
					mfbab = m1 - m0 * vvz;
					mfbac = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfbba + mfbbc;
					m1 = mfbbc - mfbba;
					m0 = m2 + mfbbb;
					mfbba = m0;
					m0 += c4o9 * oMdrho;
					mfbbb = m1 - m0 * vvz;
					mfbbc = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfbca + mfbcc;
					m1 = mfbcc - mfbca;
					m0 = m2 + mfbcb;
					mfbca = m0;
					m0 += c1o9 * oMdrho;
					mfbcb = m1 - m0 * vvz;
					mfbcc = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfcaa + mfcac;
					m1 = mfcac - mfcaa;
					m0 = m2 + mfcab;
					mfcaa = m0;
					m0 += c1o36 * oMdrho;
					mfcab = m1 - m0 * vvz;
					mfcac = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfcba + mfcbc;
					m1 = mfcbc - mfcba;
					m0 = m2 + mfcbb;
					mfcba = m0;
					m0 += c1o9 * oMdrho;
					mfcbb = m1 - m0 * vvz;
					mfcbc = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfcca + mfccc;
					m1 = mfccc - mfcca;
					m0 = m2 + mfccb;
					mfcca = m0;
					m0 += c1o36 * oMdrho;
					mfccb = m1 - m0 * vvz;
					mfccc = m2 - 2. * m1 * vvz + vz2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					// mit  1/6, 0, 1/18, 2/3, 0, 2/9, 1/6, 0, 1/18 Konditionieren
					////////////////////////////////////////////////////////////////////////////////////
					// Y - Dir
					m2 = mfaaa + mfaca;
					m1 = mfaca - mfaaa;
					m0 = m2 + mfaba;
					mfaaa = m0;
					m0 += c1o6 * oMdrho;
					mfaba = m1 - m0 * vvy;
					mfaca = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfaab + mfacb;
					m1 = mfacb - mfaab;
					m0 = m2 + mfabb;
					mfaab = m0;
					mfabb = m1 - m0 * vvy;
					mfacb = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfaac + mfacc;
					m1 = mfacc - mfaac;
					m0 = m2 + mfabc;
					mfaac = m0;
					m0 += c1o18 * oMdrho;
					mfabc = m1 - m0 * vvy;
					mfacc = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfbaa + mfbca;
					m1 = mfbca - mfbaa;
					m0 = m2 + mfbba;
					mfbaa = m0;
					m0 += c2o3 * oMdrho;
					mfbba = m1 - m0 * vvy;
					mfbca = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfbab + mfbcb;
					m1 = mfbcb - mfbab;
					m0 = m2 + mfbbb;
					mfbab = m0;
					mfbbb = m1 - m0 * vvy;
					mfbcb = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfbac + mfbcc;
					m1 = mfbcc - mfbac;
					m0 = m2 + mfbbc;
					mfbac = m0;
					m0 += c2o9 * oMdrho;
					mfbbc = m1 - m0 * vvy;
					mfbcc = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfcaa + mfcca;
					m1 = mfcca - mfcaa;
					m0 = m2 + mfcba;
					mfcaa = m0;
					m0 += c1o6 * oMdrho;
					mfcba = m1 - m0 * vvy;
					mfcca = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfcab + mfccb;
					m1 = mfccb - mfcab;
					m0 = m2 + mfcbb;
					mfcab = m0;
					mfcbb = m1 - m0 * vvy;
					mfccb = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfcac + mfccc;
					m1 = mfccc - mfcac;
					m0 = m2 + mfcbc;
					mfcac = m0;
					m0 += c1o18 * oMdrho;
					mfcbc = m1 - m0 * vvy;
					mfccc = m2 - 2. * m1 * vvy + vy2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					// mit     1, 0, 1/3, 0, 0, 0, 1/3, 0, 1/9            Konditionieren
					////////////////////////////////////////////////////////////////////////////////////
					// X - Dir
					m2 = mfaaa + mfcaa;
					m1 = mfcaa - mfaaa;
					m0 = m2 + mfbaa;
					mfaaa = m0;
					m0 += 1. * oMdrho;
					mfbaa = m1 - m0 * vvx;
					mfcaa = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfaba + mfcba;
					m1 = mfcba - mfaba;
					m0 = m2 + mfbba;
					mfaba = m0;
					mfbba = m1 - m0 * vvx;
					mfcba = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfaca + mfcca;
					m1 = mfcca - mfaca;
					m0 = m2 + mfbca;
					mfaca = m0;
					m0 += c1o3 * oMdrho;
					mfbca = m1 - m0 * vvx;
					mfcca = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfaab + mfcab;
					m1 = mfcab - mfaab;
					m0 = m2 + mfbab;
					mfaab = m0;
					mfbab = m1 - m0 * vvx;
					mfcab = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfabb + mfcbb;
					m1 = mfcbb - mfabb;
					m0 = m2 + mfbbb;
					mfabb = m0;
					mfbbb = m1 - m0 * vvx;
					mfcbb = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfacb + mfccb;
					m1 = mfccb - mfacb;
					m0 = m2 + mfbcb;
					mfacb = m0;
					mfbcb = m1 - m0 * vvx;
					mfccb = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfaac + mfcac;
					m1 = mfcac - mfaac;
					m0 = m2 + mfbac;
					mfaac = m0;
					m0 += c1o3 * oMdrho;
					mfbac = m1 - m0 * vvx;
					mfcac = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfabc + mfcbc;
					m1 = mfcbc - mfabc;
					m0 = m2 + mfbbc;
					mfabc = m0;
					mfbbc = m1 - m0 * vvx;
					mfcbc = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					m2 = mfacc + mfccc;
					m1 = mfccc - mfacc;
					m0 = m2 + mfbcc;
					mfacc = m0;
					m0 += c1o9 * oMdrho;
					mfbcc = m1 - m0 * vvx;
					mfccc = m2 - 2. * m1 * vvx + vx2 * m0;
					////////////////////////////////////////////////////////////////////////////////////
					// Cumulants
					////////////////////////////////////////////////////////////////////////////////////

					// mfaaa = 0.0;
					LBMReal OxxPyyPzz = 1.; //omega2 or bulk viscosity
											//  LBMReal OxyyPxzz = 1.;//-s9;//2+s9;//
											//  LBMReal OxyyMxzz  = 1.;//2+s9;//
					LBMReal O4 = 1.;
					LBMReal O5 = 1.;
					LBMReal O6 = 1.;

					/////fourth order parameters; here only for test. Move out of loop!

					LBMReal OxyyPxzz = 8.0 * (collFactorM - 2.0) * (OxxPyyPzz * (3.0 * collFactorM - 1.0) - 5.0 * collFactorM) / (8.0 * (5.0 - 2.0 * collFactorM) * collFactorM + OxxPyyPzz * (8.0 + collFactorM * (9.0 * collFactorM - 26.0)));
					LBMReal OxyyMxzz = 8.0 * (collFactorM - 2.0) * (collFactorM + OxxPyyPzz * (3.0 * collFactorM - 7.0)) / (OxxPyyPzz * (56.0 - 42.0 * collFactorM + 9.0 * collFactorM * collFactorM) - 8.0 * collFactorM);
					LBMReal Oxyz = 24.0 * (collFactorM - 2.0) * (4.0 * collFactorM * collFactorM + collFactorM * OxxPyyPzz * (18.0 - 13.0 * collFactorM) + OxxPyyPzz * OxxPyyPzz * (2.0 + collFactorM * (6.0 * collFactorM - 11.0))) / (16.0 * collFactorM * collFactorM * (collFactorM - 6.0) - 2.0 * collFactorM * OxxPyyPzz * (216.0 + 5.0 * collFactorM * (9.0 * collFactorM - 46.0)) + OxxPyyPzz * OxxPyyPzz * (collFactorM * (3.0 * collFactorM - 10.0) * (15.0 * collFactorM - 28.0) - 48.0));
					LBMReal A = (4.0 * collFactorM * collFactorM + 2.0 * collFactorM * OxxPyyPzz * (collFactorM - 6.0) + OxxPyyPzz * OxxPyyPzz * (collFactorM * (10.0 - 3.0 * collFactorM) - 4.0)) / ((collFactorM - OxxPyyPzz) * (OxxPyyPzz * (2.0 + 3.0 * collFactorM) - 8.0 * collFactorM));
					//FIXME:  warning C4459: declaration of 'B' hides global declaration (message : see declaration of 'D3Q27System::B' )
					LBMReal BB = (4.0 * collFactorM * OxxPyyPzz * (9.0 * collFactorM - 16.0) - 4.0 * collFactorM * collFactorM - 2.0 * OxxPyyPzz * OxxPyyPzz * (2.0 + 9.0 * collFactorM * (collFactorM - 2.0))) / (3.0 * (collFactorM - OxxPyyPzz) * (OxxPyyPzz * (2.0 + 3.0 * collFactorM) - 8.0 * collFactorM));


					//Cum 4.
					//LBMReal CUMcbb = mfcbb - ((mfcaa + c1o3 * oMdrho) * mfabb + 2. * mfbba * mfbab); // till 18.05.2015
					//LBMReal CUMbcb = mfbcb - ((mfaca + c1o3 * oMdrho) * mfbab + 2. * mfbba * mfabb); // till 18.05.2015
					//LBMReal CUMbbc = mfbbc - ((mfaac + c1o3 * oMdrho) * mfbba + 2. * mfbab * mfabb); // till 18.05.2015

					LBMReal CUMcbb = mfcbb - ((mfcaa + c1o3) * mfabb + 2. * mfbba * mfbab);
					LBMReal CUMbcb = mfbcb - ((mfaca + c1o3) * mfbab + 2. * mfbba * mfabb);
					LBMReal CUMbbc = mfbbc - ((mfaac + c1o3) * mfbba + 2. * mfbab * mfabb);

					LBMReal CUMcca = mfcca - ((mfcaa * mfaca + 2. * mfbba * mfbba) + c1o3 * (mfcaa + mfaca) * oMdrho + c1o9 * (oMdrho - c1) * oMdrho);
					LBMReal CUMcac = mfcac - ((mfcaa * mfaac + 2. * mfbab * mfbab) + c1o3 * (mfcaa + mfaac) * oMdrho + c1o9 * (oMdrho - c1) * oMdrho);
					LBMReal CUMacc = mfacc - ((mfaac * mfaca + 2. * mfabb * mfabb) + c1o3 * (mfaac + mfaca) * oMdrho + c1o9 * (oMdrho - c1) * oMdrho);

					//Cum 5.
					LBMReal CUMbcc = mfbcc - (mfaac * mfbca + mfaca * mfbac + 4. * mfabb * mfbbb + 2. * (mfbab * mfacb + mfbba * mfabc)) - c1o3 * (mfbca + mfbac) * oMdrho;
					LBMReal CUMcbc = mfcbc - (mfaac * mfcba + mfcaa * mfabc + 4. * mfbab * mfbbb + 2. * (mfabb * mfcab + mfbba * mfbac)) - c1o3 * (mfcba + mfabc) * oMdrho;
					LBMReal CUMccb = mfccb - (mfcaa * mfacb + mfaca * mfcab + 4. * mfbba * mfbbb + 2. * (mfbab * mfbca + mfabb * mfcba)) - c1o3 * (mfacb + mfcab) * oMdrho;

					//Cum 6.
					LBMReal CUMccc = mfccc + ((-4. * mfbbb * mfbbb
						- (mfcaa * mfacc + mfaca * mfcac + mfaac * mfcca)
						- 4. * (mfabb * mfcbb + mfbab * mfbcb + mfbba * mfbbc)
						- 2. * (mfbca * mfbac + mfcba * mfabc + mfcab * mfacb))
						+ (4. * (mfbab * mfbab * mfaca + mfabb * mfabb * mfcaa + mfbba * mfbba * mfaac)
							+ 2. * (mfcaa * mfaca * mfaac)
							+ 16. * mfbba * mfbab * mfabb)
						- c1o3 * (mfacc + mfcac + mfcca) * oMdrho - c1o9 * oMdrho * oMdrho
						- c1o9 * (mfcaa + mfaca + mfaac) * oMdrho * (1. - 2. * oMdrho) - c1o27 * oMdrho * oMdrho * (-2. * oMdrho)
						+ (2. * (mfbab * mfbab + mfabb * mfabb + mfbba * mfbba)
							+ (mfaac * mfaca + mfaac * mfcaa + mfaca * mfcaa)) * c2o3 * oMdrho) + c1o27 * oMdrho;

					//2.
					// linear combinations
					LBMReal mxxPyyPzz = mfcaa + mfaca + mfaac;

					//  LBMReal mfaaaS = (mfaaa * (-4 - 3 * OxxPyyPzz * (-1 + rho)) + 6 * mxxPyyPzz * OxxPyyPzz * (-1 + rho)) / (-4 + 3 * OxxPyyPzz * (-1 + rho));
					mxxPyyPzz -= mfaaa ;//12.03.21 shifted by mfaaa
										//mxxPyyPzz-=(mfaaa+mfaaaS)*c1o2;//12.03.21 shifted by mfaaa
					LBMReal mxxMyy = mfcaa - mfaca;
					LBMReal mxxMzz = mfcaa - mfaac;

					LBMReal dxux =  -c1o2 * collFactorM * (mxxMyy + mxxMzz) + c1o2 * OxxPyyPzz * (/*mfaaa*/ -mxxPyyPzz);
					LBMReal dyuy =  dxux + collFactorM * c3o2 * mxxMyy;
					LBMReal dzuz =  dxux + collFactorM * c3o2 * mxxMzz;

					LBMReal Dxy = -three * collFactorM * mfbba;
					LBMReal Dxz = -three * collFactorM * mfbab;
					LBMReal Dyz = -three * collFactorM * mfabb;

					//relax
					mxxPyyPzz += OxxPyyPzz * (/*mfaaa*/ - mxxPyyPzz) - 3. * (1. - c1o2 * OxxPyyPzz) * (vx2 * dxux + vy2 * dyuy + vz2 * dzuz);
					mxxMyy += collFactorM * (-mxxMyy) - 3. * (1. - c1o2 * collFactorM) * (vx2 * dxux - vy2 * dyuy);
					mxxMzz += collFactorM * (-mxxMzz) - 3. * (1. - c1o2 * collFactorM) * (vx2 * dxux - vz2 * dzuz);

					mfabb += collFactorM * (-mfabb);
					mfbab += collFactorM * (-mfbab);
					mfbba += collFactorM * (-mfbba);

					////updated pressure
					//mfaaa += (dX1_phi * vvx + dX2_phi * vvy + dX3_phi * vvz) * correctionScaling;
					mfaaa = 0.0; // Pressure elimination as in standard velocity model
								 //  mfaaa += (rho - c1) * (dxux + dyuy + dzuz);

					mxxPyyPzz += mfaaa; // 12.03.21 shifted by mfaaa

										// mxxPyyPzz += (mfaaa + mfaaaS) * c1o2;
										//mfaaa = mfaaaS;
										// linear combinations back
					mfcaa = c1o3 * (mxxMyy + mxxMzz + mxxPyyPzz);
					mfaca = c1o3 * (-2. * mxxMyy + mxxMzz + mxxPyyPzz);
					mfaac = c1o3 * (mxxMyy - 2. * mxxMzz + mxxPyyPzz);

					//3.
					// linear combinations
					LBMReal mxxyPyzz = mfcba + mfabc;
					LBMReal mxxyMyzz = mfcba - mfabc;

					LBMReal mxxzPyyz = mfcab + mfacb;
					LBMReal mxxzMyyz = mfcab - mfacb;

					LBMReal mxyyPxzz = mfbca + mfbac;
					LBMReal mxyyMxzz = mfbca - mfbac;

					//relax
					wadjust = Oxyz + (1. - Oxyz) * fabs(mfbbb) / (fabs(mfbbb) + qudricLimit);
					mfbbb += wadjust * (-mfbbb);
					wadjust = OxyyPxzz + (1. - OxyyPxzz) * fabs(mxxyPyzz) / (fabs(mxxyPyzz) + qudricLimit);
					mxxyPyzz += wadjust * (-mxxyPyzz);
					wadjust = OxyyMxzz + (1. - OxyyMxzz) * fabs(mxxyMyzz) / (fabs(mxxyMyzz) + qudricLimit);
					mxxyMyzz += wadjust * (-mxxyMyzz);
					wadjust = OxyyPxzz + (1. - OxyyPxzz) * fabs(mxxzPyyz) / (fabs(mxxzPyyz) + qudricLimit);
					mxxzPyyz += wadjust * (-mxxzPyyz);
					wadjust = OxyyMxzz + (1. - OxyyMxzz) * fabs(mxxzMyyz) / (fabs(mxxzMyyz) + qudricLimit);
					mxxzMyyz += wadjust * (-mxxzMyyz);
					wadjust = OxyyPxzz + (1. - OxyyPxzz) * fabs(mxyyPxzz) / (fabs(mxyyPxzz) + qudricLimit);
					mxyyPxzz += wadjust * (-mxyyPxzz);
					wadjust = OxyyMxzz + (1. - OxyyMxzz) * fabs(mxyyMxzz) / (fabs(mxyyMxzz) + qudricLimit);
					mxyyMxzz += wadjust * (-mxyyMxzz);

					// linear combinations back
					mfcba = (mxxyMyzz + mxxyPyzz) * c1o2;
					mfabc = (-mxxyMyzz + mxxyPyzz) * c1o2;
					mfcab = (mxxzMyyz + mxxzPyyz) * c1o2;
					mfacb = (-mxxzMyyz + mxxzPyyz) * c1o2;
					mfbca = (mxyyMxzz + mxyyPxzz) * c1o2;
					mfbac = (-mxyyMxzz + mxyyPxzz) * c1o2;

					//4.
					CUMacc = -O4 * (one / collFactorM - c1o2) * (dyuy + dzuz) * c2o3 * A + (one - O4) * (CUMacc);
					CUMcac = -O4 * (one / collFactorM - c1o2) * (dxux + dzuz) * c2o3 * A + (one - O4) * (CUMcac);
					CUMcca = -O4 * (one / collFactorM - c1o2) * (dyuy + dxux) * c2o3 * A + (one - O4) * (CUMcca);
					CUMbbc = -O4 * (one / collFactorM - c1o2) * Dxy * c1o3 * BB + (one - O4) * (CUMbbc);
					CUMbcb = -O4 * (one / collFactorM - c1o2) * Dxz * c1o3 * BB + (one - O4) * (CUMbcb);
					CUMcbb = -O4 * (one / collFactorM - c1o2) * Dyz * c1o3 * BB + (one - O4) * (CUMcbb);

					//5.
					CUMbcc += O5 * (-CUMbcc);
					CUMcbc += O5 * (-CUMcbc);
					CUMccb += O5 * (-CUMccb);

					//6.
					CUMccc += O6 * (-CUMccc);

					//back cumulants to central moments
					//4.
					//mfcbb = CUMcbb + ((mfcaa + c1o3 * oMdrho) * mfabb + 2. * mfbba * mfbab); // till 18.05.2015
					//mfbcb = CUMbcb + ((mfaca + c1o3 * oMdrho) * mfbab + 2. * mfbba * mfabb); // till 18.05.2015
					//mfbbc = CUMbbc + ((mfaac + c1o3 * oMdrho) * mfbba + 2. * mfbab * mfabb); // till 18.05.2015

					mfcbb = CUMcbb + ((mfcaa + c1o3) * mfabb + 2. * mfbba * mfbab);
					mfbcb = CUMbcb + ((mfaca + c1o3) * mfbab + 2. * mfbba * mfabb);
					mfbbc = CUMbbc + ((mfaac + c1o3) * mfbba + 2. * mfbab * mfabb);

					mfcca = CUMcca + (mfcaa * mfaca + 2. * mfbba * mfbba) + c1o3 * (mfcaa + mfaca) * oMdrho + c1o9 * (oMdrho - c1) * oMdrho;
					mfcac = CUMcac + (mfcaa * mfaac + 2. * mfbab * mfbab) + c1o3 * (mfcaa + mfaac) * oMdrho + c1o9 * (oMdrho - c1) * oMdrho;
					mfacc = CUMacc + (mfaac * mfaca + 2. * mfabb * mfabb) + c1o3 * (mfaac + mfaca) * oMdrho + c1o9 * (oMdrho - c1) * oMdrho;

					//5.
					mfbcc = CUMbcc + (mfaac * mfbca + mfaca * mfbac + 4. * mfabb * mfbbb + 2. * (mfbab * mfacb + mfbba * mfabc)) + c1o3 * (mfbca + mfbac) * oMdrho;
					mfcbc = CUMcbc + (mfaac * mfcba + mfcaa * mfabc + 4. * mfbab * mfbbb + 2. * (mfabb * mfcab + mfbba * mfbac)) + c1o3 * (mfcba + mfabc) * oMdrho;
					mfccb = CUMccb + (mfcaa * mfacb + mfaca * mfcab + 4. * mfbba * mfbbb + 2. * (mfbab * mfbca + mfabb * mfcba)) + c1o3 * (mfacb + mfcab) * oMdrho;

					//6.
					mfccc = CUMccc - ((-4. * mfbbb * mfbbb
						- (mfcaa * mfacc + mfaca * mfcac + mfaac * mfcca)
						- 4. * (mfabb * mfcbb + mfbac * mfbca + mfbba * mfbbc)
						- 2. * (mfbca * mfbac + mfcba * mfabc + mfcab * mfacb))
						+ (4. * (mfbab * mfbab * mfaca + mfabb * mfabb * mfcaa + mfbba * mfbba * mfaac)
							+ 2. * (mfcaa * mfaca * mfaac)
							+ 16. * mfbba * mfbab * mfabb)
						- c1o3 * (mfacc + mfcac + mfcca) * oMdrho - c1o9 * oMdrho * oMdrho
						- c1o9 * (mfcaa + mfaca + mfaac) * oMdrho * (1. - 2. * oMdrho) - c1o27 * oMdrho * oMdrho * (-2. * oMdrho)
						+ (2. * (mfbab * mfbab + mfabb * mfabb + mfbba * mfbba)
							+ (mfaac * mfaca + mfaac * mfcaa + mfaca * mfcaa)) * c2o3 * oMdrho) - c1o27 * oMdrho;


					////////


					////////////////////////////////////////////////////////////////////////////////////
					//forcing
					//mfbaa = -mfbaa;
					//mfaba = -mfaba;
					//mfaab = -mfaab;
					//////////////////////////////////////////////////////////////////////////////////////
					//mfbaa += c1o3 * (c1 / collFactorM - c1o2) * rhoToPhi * (2 * dxux * dX1_phi + Dxy * dX2_phi + Dxz * dX3_phi) / (rho);
					//mfaba += c1o3 * (c1 / collFactorM - c1o2) * rhoToPhi * (Dxy * dX1_phi + 2 * dyuy * dX2_phi + Dyz * dX3_phi) / (rho);
					//mfaab += c1o3 * (c1 / collFactorM - c1o2) * rhoToPhi * (Dxz * dX1_phi + Dyz * dX2_phi + 2 * dyuy * dX3_phi) / (rho);
					////////////////////////////////////////////////////////////////////////////////////
					//back
					////////////////////////////////////////////////////////////////////////////////////
					//mit 1, 0, 1/3, 0, 0, 0, 1/3, 0, 1/9   Konditionieren
					////////////////////////////////////////////////////////////////////////////////////
					// Z - Dir
					m0 = mfaac * c1o2 + mfaab * (vvz - c1o2) + (mfaaa + 1. * oMdrho) * (vz2 - vvz) * c1o2;
					m1 = -mfaac - 2. * mfaab * vvz + mfaaa * (1. - vz2) - 1. * oMdrho * vz2;
					m2 = mfaac * c1o2 + mfaab * (vvz + c1o2) + (mfaaa + 1. * oMdrho) * (vz2 + vvz) * c1o2;
					mfaaa = m0;
					mfaab = m1;
					mfaac = m2;
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfabc * c1o2 + mfabb * (vvz - c1o2) + mfaba * (vz2 - vvz) * c1o2;
					m1 = -mfabc - 2. * mfabb * vvz + mfaba * (1. - vz2);
					m2 = mfabc * c1o2 + mfabb * (vvz + c1o2) + mfaba * (vz2 + vvz) * c1o2;
					mfaba = m0;
					mfabb = m1;
					mfabc = m2;
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfacc * c1o2 + mfacb * (vvz - c1o2) + (mfaca + c1o3 * oMdrho) * (vz2 - vvz) * c1o2;
					m1 = -mfacc - 2. * mfacb * vvz + mfaca * (1. - vz2) - c1o3 * oMdrho * vz2;
					m2 = mfacc * c1o2 + mfacb * (vvz + c1o2) + (mfaca + c1o3 * oMdrho) * (vz2 + vvz) * c1o2;
					mfaca = m0;
					mfacb = m1;
					mfacc = m2;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfbac * c1o2 + mfbab * (vvz - c1o2) + mfbaa * (vz2 - vvz) * c1o2;
					m1 = -mfbac - 2. * mfbab * vvz + mfbaa * (1. - vz2);
					m2 = mfbac * c1o2 + mfbab * (vvz + c1o2) + mfbaa * (vz2 + vvz) * c1o2;
					mfbaa = m0;
					mfbab = m1;
					mfbac = m2;
					/////////b//////////////////////////////////////////////////////////////////////////
					m0 = mfbbc * c1o2 + mfbbb * (vvz - c1o2) + mfbba * (vz2 - vvz) * c1o2;
					m1 = -mfbbc - 2. * mfbbb * vvz + mfbba * (1. - vz2);
					m2 = mfbbc * c1o2 + mfbbb * (vvz + c1o2) + mfbba * (vz2 + vvz) * c1o2;
					mfbba = m0;
					mfbbb = m1;
					mfbbc = m2;
					/////////b//////////////////////////////////////////////////////////////////////////
					m0 = mfbcc * c1o2 + mfbcb * (vvz - c1o2) + mfbca * (vz2 - vvz) * c1o2;
					m1 = -mfbcc - 2. * mfbcb * vvz + mfbca * (1. - vz2);
					m2 = mfbcc * c1o2 + mfbcb * (vvz + c1o2) + mfbca * (vz2 + vvz) * c1o2;
					mfbca = m0;
					mfbcb = m1;
					mfbcc = m2;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfcac * c1o2 + mfcab * (vvz - c1o2) + (mfcaa + c1o3 * oMdrho) * (vz2 - vvz) * c1o2;
					m1 = -mfcac - 2. * mfcab * vvz + mfcaa * (1. - vz2) - c1o3 * oMdrho * vz2;
					m2 = mfcac * c1o2 + mfcab * (vvz + c1o2) + (mfcaa + c1o3 * oMdrho) * (vz2 + vvz) * c1o2;
					mfcaa = m0;
					mfcab = m1;
					mfcac = m2;
					/////////c//////////////////////////////////////////////////////////////////////////
					m0 = mfcbc * c1o2 + mfcbb * (vvz - c1o2) + mfcba * (vz2 - vvz) * c1o2;
					m1 = -mfcbc - 2. * mfcbb * vvz + mfcba * (1. - vz2);
					m2 = mfcbc * c1o2 + mfcbb * (vvz + c1o2) + mfcba * (vz2 + vvz) * c1o2;
					mfcba = m0;
					mfcbb = m1;
					mfcbc = m2;
					/////////c//////////////////////////////////////////////////////////////////////////
					m0 = mfccc * c1o2 + mfccb * (vvz - c1o2) + (mfcca + c1o9 * oMdrho) * (vz2 - vvz) * c1o2;
					m1 = -mfccc - 2. * mfccb * vvz + mfcca * (1. - vz2) - c1o9 * oMdrho * vz2;
					m2 = mfccc * c1o2 + mfccb * (vvz + c1o2) + (mfcca + c1o9 * oMdrho) * (vz2 + vvz) * c1o2;
					mfcca = m0;
					mfccb = m1;
					mfccc = m2;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					//mit 1/6, 2/3, 1/6, 0, 0, 0, 1/18, 2/9, 1/18   Konditionieren
					////////////////////////////////////////////////////////////////////////////////////
					// Y - Dir
					m0 = mfaca * c1o2 + mfaba * (vvy - c1o2) + (mfaaa + c1o6 * oMdrho) * (vy2 - vvy) * c1o2;
					m1 = -mfaca - 2. * mfaba * vvy + mfaaa * (1. - vy2) - c1o6 * oMdrho * vy2;
					m2 = mfaca * c1o2 + mfaba * (vvy + c1o2) + (mfaaa + c1o6 * oMdrho) * (vy2 + vvy) * c1o2;
					mfaaa = m0;
					mfaba = m1;
					mfaca = m2;
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfacb * c1o2 + mfabb * (vvy - c1o2) + (mfaab + c2o3 * oMdrho) * (vy2 - vvy) * c1o2;
					m1 = -mfacb - 2. * mfabb * vvy + mfaab * (1. - vy2) - c2o3 * oMdrho * vy2;
					m2 = mfacb * c1o2 + mfabb * (vvy + c1o2) + (mfaab + c2o3 * oMdrho) * (vy2 + vvy) * c1o2;
					mfaab = m0;
					mfabb = m1;
					mfacb = m2;
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfacc * c1o2 + mfabc * (vvy - c1o2) + (mfaac + c1o6 * oMdrho) * (vy2 - vvy) * c1o2;
					m1 = -mfacc - 2. * mfabc * vvy + mfaac * (1. - vy2) - c1o6 * oMdrho * vy2;
					m2 = mfacc * c1o2 + mfabc * (vvy + c1o2) + (mfaac + c1o6 * oMdrho) * (vy2 + vvy) * c1o2;
					mfaac = m0;
					mfabc = m1;
					mfacc = m2;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfbca * c1o2 + mfbba * (vvy - c1o2) + mfbaa * (vy2 - vvy) * c1o2;
					m1 = -mfbca - 2. * mfbba * vvy + mfbaa * (1. - vy2);
					m2 = mfbca * c1o2 + mfbba * (vvy + c1o2) + mfbaa * (vy2 + vvy) * c1o2;
					mfbaa = m0;
					mfbba = m1;
					mfbca = m2;
					/////////b//////////////////////////////////////////////////////////////////////////
					m0 = mfbcb * c1o2 + mfbbb * (vvy - c1o2) + mfbab * (vy2 - vvy) * c1o2;
					m1 = -mfbcb - 2. * mfbbb * vvy + mfbab * (1. - vy2);
					m2 = mfbcb * c1o2 + mfbbb * (vvy + c1o2) + mfbab * (vy2 + vvy) * c1o2;
					mfbab = m0;
					mfbbb = m1;
					mfbcb = m2;
					/////////b//////////////////////////////////////////////////////////////////////////
					m0 = mfbcc * c1o2 + mfbbc * (vvy - c1o2) + mfbac * (vy2 - vvy) * c1o2;
					m1 = -mfbcc - 2. * mfbbc * vvy + mfbac * (1. - vy2);
					m2 = mfbcc * c1o2 + mfbbc * (vvy + c1o2) + mfbac * (vy2 + vvy) * c1o2;
					mfbac = m0;
					mfbbc = m1;
					mfbcc = m2;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfcca * c1o2 + mfcba * (vvy - c1o2) + (mfcaa + c1o18 * oMdrho) * (vy2 - vvy) * c1o2;
					m1 = -mfcca - 2. * mfcba * vvy + mfcaa * (1. - vy2) - c1o18 * oMdrho * vy2;
					m2 = mfcca * c1o2 + mfcba * (vvy + c1o2) + (mfcaa + c1o18 * oMdrho) * (vy2 + vvy) * c1o2;
					mfcaa = m0;
					mfcba = m1;
					mfcca = m2;
					/////////c//////////////////////////////////////////////////////////////////////////
					m0 = mfccb * c1o2 + mfcbb * (vvy - c1o2) + (mfcab + c2o9 * oMdrho) * (vy2 - vvy) * c1o2;
					m1 = -mfccb - 2. * mfcbb * vvy + mfcab * (1. - vy2) - c2o9 * oMdrho * vy2;
					m2 = mfccb * c1o2 + mfcbb * (vvy + c1o2) + (mfcab + c2o9 * oMdrho) * (vy2 + vvy) * c1o2;
					mfcab = m0;
					mfcbb = m1;
					mfccb = m2;
					/////////c//////////////////////////////////////////////////////////////////////////
					m0 = mfccc * c1o2 + mfcbc * (vvy - c1o2) + (mfcac + c1o18 * oMdrho) * (vy2 - vvy) * c1o2;
					m1 = -mfccc - 2. * mfcbc * vvy + mfcac * (1. - vy2) - c1o18 * oMdrho * vy2;
					m2 = mfccc * c1o2 + mfcbc * (vvy + c1o2) + (mfcac + c1o18 * oMdrho) * (vy2 + vvy) * c1o2;
					mfcac = m0;
					mfcbc = m1;
					mfccc = m2;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					//mit 1/36, 1/9, 1/36, 1/9, 4/9, 1/9, 1/36, 1/9, 1/36 Konditionieren
					////////////////////////////////////////////////////////////////////////////////////
					// X - Dir
					m0 = mfcaa * c1o2 + mfbaa * (vvx - c1o2) + (mfaaa + c1o36 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfcaa - 2. * mfbaa * vvx + mfaaa * (1. - vx2) - c1o36 * oMdrho * vx2;
					m2 = mfcaa * c1o2 + mfbaa * (vvx + c1o2) + (mfaaa + c1o36 * oMdrho) * (vx2 + vvx) * c1o2;
					mfaaa = m0;
					mfbaa = m1;
					mfcaa = m2;
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfcba * c1o2 + mfbba * (vvx - c1o2) + (mfaba + c1o9 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfcba - 2. * mfbba * vvx + mfaba * (1. - vx2) - c1o9 * oMdrho * vx2;
					m2 = mfcba * c1o2 + mfbba * (vvx + c1o2) + (mfaba + c1o9 * oMdrho) * (vx2 + vvx) * c1o2;
					mfaba = m0;
					mfbba = m1;
					mfcba = m2;
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfcca * c1o2 + mfbca * (vvx - c1o2) + (mfaca + c1o36 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfcca - 2. * mfbca * vvx + mfaca * (1. - vx2) - c1o36 * oMdrho * vx2;
					m2 = mfcca * c1o2 + mfbca * (vvx + c1o2) + (mfaca + c1o36 * oMdrho) * (vx2 + vvx) * c1o2;
					mfaca = m0;
					mfbca = m1;
					mfcca = m2;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfcab * c1o2 + mfbab * (vvx - c1o2) + (mfaab + c1o9 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfcab - 2. * mfbab * vvx + mfaab * (1. - vx2) - c1o9 * oMdrho * vx2;
					m2 = mfcab * c1o2 + mfbab * (vvx + c1o2) + (mfaab + c1o9 * oMdrho) * (vx2 + vvx) * c1o2;
					mfaab = m0;
					mfbab = m1;
					mfcab = m2;
					///////////b////////////////////////////////////////////////////////////////////////
					m0 = mfcbb * c1o2 + mfbbb * (vvx - c1o2) + (mfabb + c4o9 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfcbb - 2. * mfbbb * vvx + mfabb * (1. - vx2) - c4o9 * oMdrho * vx2;
					m2 = mfcbb * c1o2 + mfbbb * (vvx + c1o2) + (mfabb + c4o9 * oMdrho) * (vx2 + vvx) * c1o2;
					mfabb = m0;
					mfbbb = m1;
					mfcbb = m2;
					///////////b////////////////////////////////////////////////////////////////////////
					m0 = mfccb * c1o2 + mfbcb * (vvx - c1o2) + (mfacb + c1o9 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfccb - 2. * mfbcb * vvx + mfacb * (1. - vx2) - c1o9 * oMdrho * vx2;
					m2 = mfccb * c1o2 + mfbcb * (vvx + c1o2) + (mfacb + c1o9 * oMdrho) * (vx2 + vvx) * c1o2;
					mfacb = m0;
					mfbcb = m1;
					mfccb = m2;
					////////////////////////////////////////////////////////////////////////////////////
					////////////////////////////////////////////////////////////////////////////////////
					m0 = mfcac * c1o2 + mfbac * (vvx - c1o2) + (mfaac + c1o36 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfcac - 2. * mfbac * vvx + mfaac * (1. - vx2) - c1o36 * oMdrho * vx2;
					m2 = mfcac * c1o2 + mfbac * (vvx + c1o2) + (mfaac + c1o36 * oMdrho) * (vx2 + vvx) * c1o2;
					mfaac = m0;
					mfbac = m1;
					mfcac = m2;
					///////////c////////////////////////////////////////////////////////////////////////
					m0 = mfcbc * c1o2 + mfbbc * (vvx - c1o2) + (mfabc + c1o9 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfcbc - 2. * mfbbc * vvx + mfabc * (1. - vx2) - c1o9 * oMdrho * vx2;
					m2 = mfcbc * c1o2 + mfbbc * (vvx + c1o2) + (mfabc + c1o9 * oMdrho) * (vx2 + vvx) * c1o2;
					mfabc = m0;
					mfbbc = m1;
					mfcbc = m2;
					///////////c////////////////////////////////////////////////////////////////////////
					m0 = mfccc * c1o2 + mfbcc * (vvx - c1o2) + (mfacc + c1o36 * oMdrho) * (vx2 - vvx) * c1o2;
					m1 = -mfccc - 2. * mfbcc * vvx + mfacc * (1. - vx2) - c1o36 * oMdrho * vx2;
					m2 = mfccc * c1o2 + mfbcc * (vvx + c1o2) + (mfacc + c1o36 * oMdrho) * (vx2 + vvx) * c1o2;
					mfacc = m0;
					mfbcc = m1;
					mfccc = m2;

					////forcing

					mfabb += c1o2 * (-forcingX1) * c2o9;
					mfbab += c1o2 * (-forcingX2) * c2o9;
					mfbba += c1o2 * (-forcingX3) * c2o9;
					mfaab += c1o2 * (-forcingX1 - forcingX2) * c1o18;
					mfcab += c1o2 * (forcingX1 - forcingX2) * c1o18;
					mfaba += c1o2 * (-forcingX1 - forcingX3) * c1o18;
					mfcba += c1o2 * (forcingX1 - forcingX3) * c1o18;
					mfbaa += c1o2 * (-forcingX2 - forcingX3) * c1o18;
					mfbca += c1o2 * (forcingX2 - forcingX3) * c1o18;
					mfaaa += c1o2 * (-forcingX1 - forcingX2 - forcingX3) * c1o72;
					mfcaa += c1o2 * (forcingX1 - forcingX2 - forcingX3) * c1o72;
					mfaca += c1o2 * (-forcingX1 + forcingX2 - forcingX3) * c1o72;
					mfcca += c1o2 * (forcingX1 + forcingX2 - forcingX3) * c1o72;
					mfcbb += c1o2 * (forcingX1)*c2o9;
					mfbcb += c1o2 * (forcingX2)*c2o9;
					mfbbc += c1o2 * (forcingX3)*c2o9;
					mfccb += c1o2 * (forcingX1 + forcingX2) * c1o18;
					mfacb += c1o2 * (-forcingX1 + forcingX2) * c1o18;
					mfcbc += c1o2 * (forcingX1 + forcingX3) * c1o18;
					mfabc += c1o2 * (-forcingX1 + forcingX3) * c1o18;
					mfbcc += c1o2 * (forcingX2 + forcingX3) * c1o18;
					mfbac += c1o2 * (-forcingX2 + forcingX3) * c1o18;
					mfccc += c1o2 * (forcingX1 + forcingX2 + forcingX3) * c1o72;
					mfacc += c1o2 * (-forcingX1 + forcingX2 + forcingX3) * c1o72;
					mfcac += c1o2 * (forcingX1 - forcingX2 + forcingX3) * c1o72;
					mfaac += c1o2 * (-forcingX1 - forcingX2 + forcingX3) * c1o72;




					//////////////////////////////////////////////////////////////////////////
					//proof correctness
					//////////////////////////////////////////////////////////////////////////
					//#ifdef  PROOF_CORRECTNESS
					LBMReal rho_post = (mfaaa + mfaac + mfaca + mfcaa + mfacc + mfcac + mfccc + mfcca)
						+ (mfaab + mfacb + mfcab + mfccb) + (mfaba + mfabc + mfcba + mfcbc) + (mfbaa + mfbac + mfbca + mfbcc)
						+ (mfabb + mfcbb) + (mfbab + mfbcb) + (mfbba + mfbbc) + mfbbb;
					//			   //LBMReal dif = fabs(drho - rho_post);
					//               LBMReal dif = drho + (dX1_phi * vvx + dX2_phi * vvy + dX3_phi * vvz) * correctionScaling - rho_post;
					//#ifdef SINGLEPRECISION
					//			   if (dif > 10.0E-7 || dif < -10.0E-7)
					//#else
					//			   if (dif > 10.0E-15 || dif < -10.0E-15)
					//#endif
					//			   {
					//				   UB_THROW(UbException(UB_EXARGS, "drho=" + UbSystem::toString(drho) + ", rho_post=" + UbSystem::toString(rho_post)
					//					   + " dif=" + UbSystem::toString(dif)
					//					   + " drho is not correct for node " + UbSystem::toString(x1) + "," + UbSystem::toString(x2) + "," + UbSystem::toString(x3)));
					//				   //UBLOG(logERROR,"LBMKernelETD3Q27CCLB::collideAll(): drho is not correct for node "+UbSystem::toString(x1)+","+UbSystem::toString(x2)+","+UbSystem::toString(x3));
					//				   //exit(EXIT_FAILURE);
					//			   }
					//#endif

					if (UbMath::isNaN(rho_post) || UbMath::isInfinity(rho_post))
						UB_THROW(UbException(
							UB_EXARGS, "rho_post is not a number (nan or -1.#IND) or infinity number -1.#INF, node=" + UbSystem::toString(x1) + "," +
							UbSystem::toString(x2) + "," + UbSystem::toString(x3)));

					//////////////////////////////////////////////////////////////////////////
					//write distribution
					//////////////////////////////////////////////////////////////////////////
					(*this->localDistributionsF)(D3Q27System::ET_E, x1, x2, x3) = mfabb         ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_N, x1, x2, x3) = mfbab         ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_T, x1, x2, x3) = mfbba         ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_NE, x1, x2, x3) = mfaab        ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_NW, x1p, x2, x3) = mfcab       ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_TE, x1, x2, x3) = mfaba        ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_TW, x1p, x2, x3) = mfcba       ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_TN, x1, x2, x3) = mfbaa        ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_TS, x1, x2p, x3) = mfbca       ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_TNE, x1, x2, x3) = mfaaa       ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_TNW, x1p, x2, x3) = mfcaa      ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_TSE, x1, x2p, x3) = mfaca      ;//* rho * c1o3;
					(*this->localDistributionsF)(D3Q27System::ET_TSW, x1p, x2p, x3) = mfcca     ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_W, x1p, x2, x3) = mfcbb     ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_S, x1, x2p, x3) = mfbcb     ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_B, x1, x2, x3p) = mfbbc     ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_SW, x1p, x2p, x3) = mfccb   ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_SE, x1, x2p, x3) = mfacb    ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_BW, x1p, x2, x3p) = mfcbc   ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_BE, x1, x2, x3p) = mfabc    ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_BS, x1, x2p, x3p) = mfbcc   ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_BN, x1, x2, x3p) = mfbac    ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_BSW, x1p, x2p, x3p) = mfccc ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_BSE, x1, x2p, x3p) = mfacc  ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_BNW, x1p, x2, x3p) = mfcac  ;//* rho * c1o3;
					(*this->nonLocalDistributionsF)(D3Q27System::ET_BNE, x1, x2, x3p) = mfaac   ;//* rho * c1o3;

					(*this->zeroDistributionsF)(x1, x2, x3) = mfbbb;// *rho* c1o3;
																																		// !Old Kernel
/////////////////////  P H A S E - F I E L D   S O L V E R
////////////////////////////////////////////
/////CUMULANT PHASE-FIELD
					LBMReal omegaD =1.0/( 3.0 * mob + 0.5);
					{
						mfcbb = (*this->localDistributionsH1)(D3Q27System::ET_E, x1, x2, x3);
						mfbcb = (*this->localDistributionsH1)(D3Q27System::ET_N, x1, x2, x3);
						mfbbc = (*this->localDistributionsH1)(D3Q27System::ET_T, x1, x2, x3);
						mfccb = (*this->localDistributionsH1)(D3Q27System::ET_NE, x1, x2, x3);
						mfacb = (*this->localDistributionsH1)(D3Q27System::ET_NW, x1p, x2, x3);
						mfcbc = (*this->localDistributionsH1)(D3Q27System::ET_TE, x1, x2, x3);
						mfabc = (*this->localDistributionsH1)(D3Q27System::ET_TW, x1p, x2, x3);
						mfbcc = (*this->localDistributionsH1)(D3Q27System::ET_TN, x1, x2, x3);
						mfbac = (*this->localDistributionsH1)(D3Q27System::ET_TS, x1, x2p, x3);
						mfccc = (*this->localDistributionsH1)(D3Q27System::ET_TNE, x1, x2, x3);
						mfacc = (*this->localDistributionsH1)(D3Q27System::ET_TNW, x1p, x2, x3);
						mfcac = (*this->localDistributionsH1)(D3Q27System::ET_TSE, x1, x2p, x3);
						mfaac = (*this->localDistributionsH1)(D3Q27System::ET_TSW, x1p, x2p, x3);
						mfabb = (*this->nonLocalDistributionsH1)(D3Q27System::ET_W, x1p, x2, x3);
						mfbab = (*this->nonLocalDistributionsH1)(D3Q27System::ET_S, x1, x2p, x3);
						mfbba = (*this->nonLocalDistributionsH1)(D3Q27System::ET_B, x1, x2, x3p);
						mfaab = (*this->nonLocalDistributionsH1)(D3Q27System::ET_SW, x1p, x2p, x3);
						mfcab = (*this->nonLocalDistributionsH1)(D3Q27System::ET_SE, x1, x2p, x3);
						mfaba = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BW, x1p, x2, x3p);
						mfcba = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BE, x1, x2, x3p);
						mfbaa = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BS, x1, x2p, x3p);
						mfbca = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BN, x1, x2, x3p);
						mfaaa = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BSW, x1p, x2p, x3p);
						mfcaa = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BSE, x1, x2p, x3p);
						mfaca = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BNW, x1p, x2, x3p);
						mfcca = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BNE, x1, x2, x3p);
						mfbbb = (*this->zeroDistributionsH1)(x1, x2, x3);


						////////////////////////////////////////////////////////////////////////////////////
						//! - Calculate density and velocity using pyramid summation for low round-off errors as in Eq. (J1)-(J3) \ref
						//! <a href="https://doi.org/10.1016/j.camwa.2015.05.001"><b>[ M. Geier et al. (2015), DOI:10.1016/j.camwa.2015.05.001 ]</b></a>
						//!
						////////////////////////////////////////////////////////////////////////////////////
						// second component
						LBMReal concentration =
							((((mfccc + mfaaa) + (mfaca + mfcac)) + ((mfacc + mfcaa) + (mfaac + mfcca))) +
								(((mfbac + mfbca) + (mfbaa + mfbcc)) + ((mfabc + mfcba) + (mfaba + mfcbc)) + ((mfacb + mfcab) + (mfaab + mfccb))) +
								((mfabb + mfcbb) + (mfbab + mfbcb) + (mfbba + mfbbc))) + mfbbb;
						////////////////////////////////////////////////////////////////////////////////////
						LBMReal oneMinusRho = c1- concentration;

						LBMReal cx =
							((((mfccc - mfaaa) + (mfcac - mfaca)) + ((mfcaa - mfacc) + (mfcca - mfaac))) +
								(((mfcba - mfabc) + (mfcbc - mfaba)) + ((mfcab - mfacb) + (mfccb - mfaab))) +
								(mfcbb - mfabb));
						LBMReal cy =
							((((mfccc - mfaaa) + (mfaca - mfcac)) + ((mfacc - mfcaa) + (mfcca - mfaac))) +
								(((mfbca - mfbac) + (mfbcc - mfbaa)) + ((mfacb - mfcab) + (mfccb - mfaab))) +
								(mfbcb - mfbab));
						LBMReal cz =
							((((mfccc - mfaaa) + (mfcac - mfaca)) + ((mfacc - mfcaa) + (mfaac - mfcca))) +
								(((mfbac - mfbca) + (mfbcc - mfbaa)) + ((mfabc - mfcba) + (mfcbc - mfaba))) +
								(mfbbc - mfbba));

						////////////////////////////////////////////////////////////////////////////////////
						// calculate the square of velocities for this lattice node
						LBMReal cx2 = cx * cx;
						LBMReal cy2 = cy * cy;
						LBMReal cz2 = cz * cz;
						////////////////////////////////////////////////////////////////////////////////////
						//! - Chimera transform from well conditioned distributions to central moments as defined in Appendix J in \ref
						//! <a href="https://doi.org/10.1016/j.camwa.2015.05.001"><b>[ M. Geier et al. (2015), DOI:10.1016/j.camwa.2015.05.001 ]</b></a>
						//! see also Eq. (6)-(14) in \ref
						//! <a href="https://doi.org/10.1016/j.jcp.2017.05.040"><b>[ M. Geier et al. (2017), DOI:10.1016/j.jcp.2017.05.040 ]</b></a>
						//!
						////////////////////////////////////////////////////////////////////////////////////
						// Z - Dir
						forwardInverseChimeraWithKincompressible(mfaaa, mfaab, mfaac, cz, cz2, c36, c1o36, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfaba, mfabb, mfabc, cz, cz2, c9, c1o9, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfaca, mfacb, mfacc, cz, cz2, c36, c1o36, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfbaa, mfbab, mfbac, cz, cz2, c9, c1o9, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfbba, mfbbb, mfbbc, cz, cz2, c9o4, c4o9, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfbca, mfbcb, mfbcc, cz, cz2, c9, c1o9, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfcaa, mfcab, mfcac, cz, cz2, c36, c1o36, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfcba, mfcbb, mfcbc, cz, cz2, c9, c1o9, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfcca, mfccb, mfccc, cz, cz2, c36, c1o36, oneMinusRho);

						////////////////////////////////////////////////////////////////////////////////////
						// Y - Dir
						forwardInverseChimeraWithKincompressible(mfaaa, mfaba, mfaca, cy, cy2, c6, c1o6, oneMinusRho);
						forwardChimera(mfaab, mfabb, mfacb, cy, cy2);
						forwardInverseChimeraWithKincompressible(mfaac, mfabc, mfacc, cy, cy2, c18, c1o18, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfbaa, mfbba, mfbca, cy, cy2, c3o2, c2o3, oneMinusRho);
						forwardChimera(mfbab, mfbbb, mfbcb, cy, cy2);
						forwardInverseChimeraWithKincompressible(mfbac, mfbbc, mfbcc, cy, cy2, c9o2, c2o9, oneMinusRho);
						forwardInverseChimeraWithKincompressible(mfcaa, mfcba, mfcca, cy, cy2, c6, c1o6, oneMinusRho);
						forwardChimera(mfcab, mfcbb, mfccb, cy, cy2);
						forwardInverseChimeraWithKincompressible(mfcac, mfcbc, mfccc, cy, cy2, c18, c1o18, oneMinusRho);

						////////////////////////////////////////////////////////////////////////////////////
						// X - Dir
						forwardInverseChimeraWithKincompressible(mfaaa, mfbaa, mfcaa, cx, cx2, c1, c1, oneMinusRho);
						forwardChimera(mfaba, mfbba, mfcba, cx, cx2);
						forwardInverseChimeraWithKincompressible(mfaca, mfbca, mfcca, cx, cx2, c3, c1o3, oneMinusRho);
						forwardChimera(mfaab, mfbab, mfcab, cx, cx2);
						forwardChimera(mfabb, mfbbb, mfcbb, cx, cx2);
						forwardChimera(mfacb, mfbcb, mfccb, cx, cx2);
						forwardInverseChimeraWithKincompressible(mfaac, mfbac, mfcac, cx, cx2, c3, c1o3, oneMinusRho);
						forwardChimera(mfabc, mfbbc, mfcbc, cx, cx2);
						forwardInverseChimeraWithKincompressible(mfacc, mfbcc, mfccc, cx, cx2, c3, c1o9, oneMinusRho);

						////////////////////////////////////////////////////////////////////////////////////
						//! - experimental Cumulant ... to be published ... hopefully
						//!

						// linearized orthogonalization of 3rd order central moments
						LBMReal Mabc = mfabc - mfaba * c1o3;
						LBMReal Mbca = mfbca - mfbaa * c1o3;
						LBMReal Macb = mfacb - mfaab * c1o3;
						LBMReal Mcba = mfcba - mfaba * c1o3;
						LBMReal Mcab = mfcab - mfaab * c1o3;
						LBMReal Mbac = mfbac - mfbaa * c1o3;
						// linearized orthogonalization of 5th order central moments
						LBMReal Mcbc = mfcbc - mfaba * c1o9;
						LBMReal Mbcc = mfbcc - mfbaa * c1o9;
						LBMReal Mccb = mfccb - mfaab * c1o9;

						// collision of 1st order moments
						cx = cx * (c1 - omegaD) + omegaD * vvx * concentration +
							normX1 * (c1 - 0.5 * omegaD) * (1.0 - phi[REST]) * (phi[REST]) * c1o3 * oneOverInterfaceScale;
						cy = cy * (c1 - omegaD) + omegaD * vvy * concentration +
							normX2 * (c1 - 0.5 * omegaD) * (1.0 - phi[REST]) * (phi[REST]) * c1o3 * oneOverInterfaceScale;
						cz = cz * (c1 - omegaD) + omegaD * vvz * concentration +
							normX3 * (c1 - 0.5 * omegaD) * (1.0 - phi[REST]) * (phi[REST]) * c1o3 * oneOverInterfaceScale;

						cx2 = cx * cx;
						cy2 = cy * cy;
						cz2 = cz * cz;

						// equilibration of 2nd order moments
						mfbba = zeroReal;
						mfbab = zeroReal;
						mfabb = zeroReal;

						mfcaa = c1o3 * concentration;
						mfaca = c1o3 * concentration;
						mfaac = c1o3 * concentration;

						// equilibration of 3rd order moments
						Mabc = zeroReal;
						Mbca = zeroReal;
						Macb = zeroReal;
						Mcba = zeroReal;
						Mcab = zeroReal;
						Mbac = zeroReal;
						mfbbb = zeroReal;

						// from linearized orthogonalization 3rd order central moments to central moments
						mfabc = Mabc + mfaba * c1o3;
						mfbca = Mbca + mfbaa * c1o3;
						mfacb = Macb + mfaab * c1o3;
						mfcba = Mcba + mfaba * c1o3;
						mfcab = Mcab + mfaab * c1o3;
						mfbac = Mbac + mfbaa * c1o3;

						// equilibration of 4th order moments
						mfacc = c1o9 * concentration;
						mfcac = c1o9 * concentration;
						mfcca = c1o9 * concentration;

						mfcbb = zeroReal;
						mfbcb = zeroReal;
						mfbbc = zeroReal;

						// equilibration of 5th order moments
						Mcbc = zeroReal;
						Mbcc = zeroReal;
						Mccb = zeroReal;

						// from linearized orthogonalization 5th order central moments to central moments
						mfcbc = Mcbc + mfaba * c1o9;
						mfbcc = Mbcc + mfbaa * c1o9;
						mfccb = Mccb + mfaab * c1o9;

						// equilibration of 6th order moment
						mfccc = c1o27 * concentration;

						////////////////////////////////////////////////////////////////////////////////////
						//! - Chimera transform from central moments to well conditioned distributions as defined in Appendix J in
						//! <a href="https://doi.org/10.1016/j.camwa.2015.05.001"><b>[ M. Geier et al. (2015), DOI:10.1016/j.camwa.2015.05.001 ]</b></a>
						//! see also Eq. (88)-(96) in
						//! <a href="https://doi.org/10.1016/j.jcp.2017.05.040"><b>[ M. Geier et al. (2017), DOI:10.1016/j.jcp.2017.05.040 ]</b></a>
						//!
						////////////////////////////////////////////////////////////////////////////////////
						// X - Dir
						backwardInverseChimeraWithKincompressible(mfaaa, mfbaa, mfcaa, cx, cx2, c1, c1, oneMinusRho);
						backwardChimera(mfaba, mfbba, mfcba, cx, cx2);
						backwardInverseChimeraWithKincompressible(mfaca, mfbca, mfcca, cx, cx2, c3, c1o3, oneMinusRho);
						backwardChimera(mfaab, mfbab, mfcab, cx, cx2);
						backwardChimera(mfabb, mfbbb, mfcbb, cx, cx2);
						backwardChimera(mfacb, mfbcb, mfccb, cx, cx2);
						backwardInverseChimeraWithKincompressible(mfaac, mfbac, mfcac, cx, cx2, c3, c1o3, oneMinusRho);
						backwardChimera(mfabc, mfbbc, mfcbc, cx, cx2);
						backwardInverseChimeraWithKincompressible(mfacc, mfbcc, mfccc, cx, cx2, c9, c1o9, oneMinusRho);

						////////////////////////////////////////////////////////////////////////////////////
						// Y - Dir
						backwardInverseChimeraWithKincompressible(mfaaa, mfaba, mfaca, cy, cy2, c6, c1o6, oneMinusRho);
						backwardChimera(mfaab, mfabb, mfacb, cy, cy2);
						backwardInverseChimeraWithKincompressible(mfaac, mfabc, mfacc, cy, cy2, c18, c1o18, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfbaa, mfbba, mfbca, cy, cy2, c3o2, c2o3, oneMinusRho);
						backwardChimera(mfbab, mfbbb, mfbcb, cy, cy2);
						backwardInverseChimeraWithKincompressible(mfbac, mfbbc, mfbcc, cy, cy2, c9o2, c2o9, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfcaa, mfcba, mfcca, cy, cy2, c6, c1o6, oneMinusRho);
						backwardChimera(mfcab, mfcbb, mfccb, cy, cy2);
						backwardInverseChimeraWithKincompressible(mfcac, mfcbc, mfccc, cy, cy2, c18, c1o18, oneMinusRho);

						////////////////////////////////////////////////////////////////////////////////////
						// Z - Dir
						backwardInverseChimeraWithKincompressible(mfaaa, mfaab, mfaac, cz, cz2, c36, c1o36, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfaba, mfabb, mfabc, cz, cz2, c9, c1o9, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfaca, mfacb, mfacc, cz, cz2, c36, c1o36, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfbaa, mfbab, mfbac, cz, cz2, c9, c1o9, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfbba, mfbbb, mfbbc, cz, cz2, c9o4, c4o9, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfbca, mfbcb, mfbcc, cz, cz2, c9, c1o9, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfcaa, mfcab, mfcac, cz, cz2, c36, c1o36, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfcba, mfcbb, mfcbc, cz, cz2, c9, c1o9, oneMinusRho);
						backwardInverseChimeraWithKincompressible(mfcca, mfccb, mfccc, cz, cz2, c36, c1o36, oneMinusRho);



						(*this->localDistributionsH1)(D3Q27System::ET_E,   x1,  x2,  x3) = mfabb;
						(*this->localDistributionsH1)(D3Q27System::ET_N,   x1,  x2,  x3) = mfbab;
						(*this->localDistributionsH1)(D3Q27System::ET_T,   x1,  x2,  x3) = mfbba;
						(*this->localDistributionsH1)(D3Q27System::ET_NE,  x1,  x2,  x3) = mfaab;
						(*this->localDistributionsH1)(D3Q27System::ET_NW,  x1p, x2,  x3) = mfcab;
						(*this->localDistributionsH1)(D3Q27System::ET_TE,  x1,  x2,  x3) = mfaba;
						(*this->localDistributionsH1)(D3Q27System::ET_TW,  x1p, x2,  x3) = mfcba;
						(*this->localDistributionsH1)(D3Q27System::ET_TN,  x1,  x2,  x3) = mfbaa;
						(*this->localDistributionsH1)(D3Q27System::ET_TS,  x1,  x2p, x3) = mfbca;
						(*this->localDistributionsH1)(D3Q27System::ET_TNE, x1,  x2,  x3) = mfaaa;
						(*this->localDistributionsH1)(D3Q27System::ET_TNW, x1p, x2,  x3) = mfcaa;
						(*this->localDistributionsH1)(D3Q27System::ET_TSE, x1,  x2p, x3) = mfaca;
						(*this->localDistributionsH1)(D3Q27System::ET_TSW, x1p, x2p, x3) = mfcca;

						(*this->nonLocalDistributionsH1)(D3Q27System::ET_W,   x1p, x2,  x3 ) = mfcbb;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_S,   x1,  x2p, x3 ) = mfbcb;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_B,   x1,  x2,  x3p) = mfbbc;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_SW,  x1p, x2p, x3 ) = mfccb;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_SE,  x1,  x2p, x3 ) = mfacb;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_BW,  x1p, x2,  x3p) = mfcbc;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_BE,  x1,  x2,  x3p) = mfabc;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_BS,  x1,  x2p, x3p) = mfbcc;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_BN,  x1,  x2,  x3p) = mfbac;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_BSW, x1p, x2p, x3p) = mfccc;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_BSE, x1,  x2p, x3p) = mfacc;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_BNW, x1p, x2,  x3p) = mfcac;
						(*this->nonLocalDistributionsH1)(D3Q27System::ET_BNE, x1,  x2,  x3p) = mfaac;

						(*this->zeroDistributionsH1)(x1,x2,x3) = mfbbb;
					}
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////

LBMReal MultiphasePressureFilterLBMKernel::gradX1_phi()
{
	using namespace D3Q27System;
	return 3.0* ((WEIGTH[TNE] * (((phi[TNE] - phi[BSW]) + (phi[BSE] - phi[TNW])) + ((phi[TSE] - phi[BNW]) + (phi[BNE] - phi[TSW])))
		+ WEIGTH[NE] * (((phi[TE] - phi[BW]) + (phi[BE] - phi[TW])) + ((phi[SE] - phi[NW]) + (phi[NE] - phi[SW])))) +
		+WEIGTH[N] * (phi[E] - phi[W]));
}

LBMReal MultiphasePressureFilterLBMKernel::gradX2_phi()
{
	using namespace D3Q27System;
	return 3.0 * ((WEIGTH[TNE] * (((phi[TNE] - phi[BSW]) - (phi[BSE] - phi[TNW])) + ((phi[BNE] - phi[TSW])- (phi[TSE] - phi[BNW])))
		+ WEIGTH[NE] * (((phi[TN] - phi[BS]) + (phi[BN] - phi[TS])) + ((phi[NE] - phi[SW])- (phi[SE] - phi[NW])))) +
		+WEIGTH[N] * (phi[N] - phi[S]));
}

LBMReal MultiphasePressureFilterLBMKernel::gradX3_phi()
{
	using namespace D3Q27System;
	return 3.0 * ((WEIGTH[TNE] * (((phi[TNE] - phi[BSW]) - (phi[BSE] - phi[TNW])) + ((phi[TSE] - phi[BNW]) - (phi[BNE] - phi[TSW])))
		+ WEIGTH[NE] * (((phi[TE] - phi[BW]) - (phi[BE] - phi[TW])) + ((phi[TS] - phi[BN]) + (phi[TN] - phi[BS])))) +
		+WEIGTH[N] * (phi[T] - phi[B]));
}

LBMReal MultiphasePressureFilterLBMKernel::nabla2_phi()
{
	using namespace D3Q27System;
	LBMReal sum = 0.0;
	sum += WEIGTH[TNE] * ((((phi[TNE] - phi[REST]) + (phi[BSW] - phi[REST])) + ((phi[TSW] - phi[REST]) + (phi[BNE] - phi[REST])))
		+ (((phi[TNW] - phi[REST]) + (phi[BSE] - phi[REST])) + ((phi[TSE] - phi[REST]) + (phi[BNW] - phi[REST]))));
	sum += WEIGTH[TN] * (
		(((phi[TN] - phi[REST]) + (phi[BS] - phi[REST])) + ((phi[TS] - phi[REST]) + (phi[BN] - phi[REST])))
		+	(((phi[TE] - phi[REST]) + (phi[BW] - phi[REST])) + ((phi[TW] - phi[REST]) + (phi[BE] - phi[REST])))
		+	(((phi[NE] - phi[REST]) + (phi[SW] - phi[REST])) + ((phi[NW] - phi[REST]) + (phi[SE] - phi[REST])))
		);
	sum += WEIGTH[T] * (
		((phi[T] - phi[REST]) + (phi[B] - phi[REST]))
		+	((phi[N] - phi[REST]) + (phi[S] - phi[REST]))
		+	((phi[E] - phi[REST]) + (phi[W] - phi[REST]))
		);

	return 6.0 * sum;
}

void MultiphasePressureFilterLBMKernel::computePhasefield()
{
	using namespace D3Q27System;
	SPtr<DistributionArray3D> distributionsH = dataSet->getHdistributions();

	int minX1 = ghostLayerWidth;
	int minX2 = ghostLayerWidth;
	int minX3 = ghostLayerWidth;
	int maxX1 = (int)distributionsH->getNX1() - ghostLayerWidth;
	int maxX2 = (int)distributionsH->getNX2() - ghostLayerWidth;
	int maxX3 = (int)distributionsH->getNX3() - ghostLayerWidth;

	//------------- Computing the phase-field ------------------
	for (int x3 = minX3; x3 < maxX3; x3++) {
		for (int x2 = minX2; x2 < maxX2; x2++) {
			for (int x1 = minX1; x1 < maxX1; x1++) {
				// if(!bcArray->isSolid(x1,x2,x3) && !bcArray->isUndefined(x1,x2,x3))
				{
					int x1p = x1 + 1;
					int x2p = x2 + 1;
					int x3p = x3 + 1;

					h[E]   = (*this->localDistributionsH1)(D3Q27System::ET_E, x1, x2, x3);
					h[N]   = (*this->localDistributionsH1)(D3Q27System::ET_N, x1, x2, x3);
					h[T]   = (*this->localDistributionsH1)(D3Q27System::ET_T, x1, x2, x3);
					h[NE]  = (*this->localDistributionsH1)(D3Q27System::ET_NE, x1, x2, x3);
					h[NW]  = (*this->localDistributionsH1)(D3Q27System::ET_NW, x1p, x2, x3);
					h[TE]  = (*this->localDistributionsH1)(D3Q27System::ET_TE, x1, x2, x3);
					h[TW]  = (*this->localDistributionsH1)(D3Q27System::ET_TW, x1p, x2, x3);
					h[TN]  = (*this->localDistributionsH1)(D3Q27System::ET_TN, x1, x2, x3);
					h[TS]  = (*this->localDistributionsH1)(D3Q27System::ET_TS, x1, x2p, x3);
					h[TNE] = (*this->localDistributionsH1)(D3Q27System::ET_TNE, x1, x2, x3);
					h[TNW] = (*this->localDistributionsH1)(D3Q27System::ET_TNW, x1p, x2, x3);
					h[TSE] = (*this->localDistributionsH1)(D3Q27System::ET_TSE, x1, x2p, x3);
					h[TSW] = (*this->localDistributionsH1)(D3Q27System::ET_TSW, x1p, x2p, x3);

					h[W]   = (*this->nonLocalDistributionsH1)(D3Q27System::ET_W, x1p, x2, x3);
					h[S]   = (*this->nonLocalDistributionsH1)(D3Q27System::ET_S, x1, x2p, x3);
					h[B]   = (*this->nonLocalDistributionsH1)(D3Q27System::ET_B, x1, x2, x3p);
					h[SW]  = (*this->nonLocalDistributionsH1)(D3Q27System::ET_SW, x1p, x2p, x3);
					h[SE]  = (*this->nonLocalDistributionsH1)(D3Q27System::ET_SE, x1, x2p, x3);
					h[BW]  = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BW, x1p, x2, x3p);
					h[BE]  = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BE, x1, x2, x3p);
					h[BS]  = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BS, x1, x2p, x3p);
					h[BN]  = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BN, x1, x2, x3p);
					h[BSW] = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BSW, x1p, x2p, x3p);
					h[BSE] = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BSE, x1, x2p, x3p);
					h[BNW] = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BNW, x1p, x2, x3p);
					h[BNE] = (*this->nonLocalDistributionsH1)(D3Q27System::ET_BNE, x1, x2, x3p);

					h[REST] = (*this->zeroDistributionsH1)(x1, x2, x3);
				}
			}
		}
	}
}

void MultiphasePressureFilterLBMKernel::findNeighbors(CbArray3D<LBMReal, IndexerX3X2X1>::CbArray3DPtr ph, int x1, int x2,
	int x3)
{
	using namespace D3Q27System;

	SPtr<BCArray3D> bcArray = this->getBCProcessor()->getBCArray();

	phi[REST] = (*ph)(x1, x2, x3);


	for (int k = FSTARTDIR; k <= FENDDIR; k++) {

		if (!bcArray->isSolid(x1 + DX1[k], x2 + DX2[k], x3 + DX3[k])) {
			phi[k] = (*ph)(x1 + DX1[k], x2 + DX2[k], x3 + DX3[k]);
		} else {
            phi[k] = phaseFieldBC;
		}
	}
}

void MultiphasePressureFilterLBMKernel::swapDistributions()
{
	LBMKernel::swapDistributions();
	dataSet->getHdistributions()->swap();
}

void MultiphasePressureFilterLBMKernel::initForcing()
{
	muForcingX1.DefineVar("x1", &muX1); muForcingX1.DefineVar("x2", &muX2); muForcingX1.DefineVar("x3", &muX3);
	muForcingX2.DefineVar("x1", &muX1); muForcingX2.DefineVar("x2", &muX2); muForcingX2.DefineVar("x3", &muX3);
	muForcingX3.DefineVar("x1", &muX1); muForcingX3.DefineVar("x2", &muX2); muForcingX3.DefineVar("x3", &muX3);

	muDeltaT = deltaT;

	muForcingX1.DefineVar("dt", &muDeltaT);
	muForcingX2.DefineVar("dt", &muDeltaT);
	muForcingX3.DefineVar("dt", &muDeltaT);

	muNu = (1.0 / 3.0) * (1.0 / collFactor - 1.0 / 2.0);

	muForcingX1.DefineVar("nu", &muNu);
	muForcingX2.DefineVar("nu", &muNu);
	muForcingX3.DefineVar("nu", &muNu);

	muForcingX1.DefineVar("rho",&muRho); 
	muForcingX2.DefineVar("rho",&muRho); 
	muForcingX3.DefineVar("rho",&muRho); 

}

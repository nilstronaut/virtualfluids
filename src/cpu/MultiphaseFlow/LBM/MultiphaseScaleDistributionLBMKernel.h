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
//! \file MultiphaseScaleDistributionLBMKernel.h
//! \ingroup LBMKernel
//! \author M. Geier, K. Kutscher, Hesameddin Safari
//=======================================================================================

#ifndef MultiphaseScaleDistributionLBMKernel_H
#define MultiphaseScaleDistributionLBMKernel_H

#include "LBMKernel.h"
#include "BCSet.h"
#include "D3Q27System.h"
#include "basics/utilities/UbTiming.h"
#include "basics/container/CbArray4D.h"
#include "basics/container/CbArray3D.h"

//! \brief  Multiphase Cascaded Cumulant LBM kernel. 
//! \details CFD solver that use Cascaded Cumulant Lattice Boltzmann method for D3Q27 model
//! \author  M. Geier, K. Kutscher, Hesameddin Safari
class MultiphaseScaleDistributionLBMKernel : public LBMKernel
{
public:
    MultiphaseScaleDistributionLBMKernel();
    virtual ~MultiphaseScaleDistributionLBMKernel(void) = default;
    void calculate(int step) override;
    SPtr<LBMKernel> clone() override;


    ///refactor
    //CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr pressure;


    double getCalculationTime() override { return .0; }
protected:
    virtual void initDataSet();
    void swapDistributions() override;

    void initForcing();

    void forwardInverseChimeraWithKincompressible(real& mfa, real& mfb, real& mfc, real vv, real v2, real Kinverse, real K, real oneMinusRho);
    void backwardInverseChimeraWithKincompressible(real& mfa, real& mfb, real& mfc, real vv, real v2, real Kinverse, real K, real oneMinusRho);
    void forwardChimera(real& mfa, real& mfb, real& mfc, real vv, real v2);
    void backwardChimera(real& mfa, real& mfb, real& mfc, real vv, real v2);

    real f1[D3Q27System::ENDF+1];

    CbArray4D<real,IndexerX4X3X2X1>::CbArray4DPtr localDistributionsF;
    CbArray4D<real,IndexerX4X3X2X1>::CbArray4DPtr nonLocalDistributionsF;
    CbArray3D<real,IndexerX3X2X1>::CbArray3DPtr   zeroDistributionsF;

    CbArray4D<real,IndexerX4X3X2X1>::CbArray4DPtr localDistributionsH1;
    CbArray4D<real,IndexerX4X3X2X1>::CbArray4DPtr nonLocalDistributionsH1;
    CbArray3D<real,IndexerX3X2X1>::CbArray3DPtr   zeroDistributionsH1;

    CbArray4D<real, IndexerX4X3X2X1>::CbArray4DPtr localDistributionsH2;
    CbArray4D<real, IndexerX4X3X2X1>::CbArray4DPtr nonLocalDistributionsH2;
    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr   zeroDistributionsH2;

    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr pressureOld;
    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr p1Old;

    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr phaseField;
    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr phaseFieldOld;
    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr divU; 

    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr rhoNode;
    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr vxNode;
    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr vyNode;
    CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr vzNode;

    real h  [D3Q27System::ENDF+1];
    real h2[D3Q27System::ENDF + 1];
    real g  [D3Q27System::ENDF+1];
    real phi[D3Q27System::ENDF+1];
    real phi2[D3Q27System::ENDF + 1];
    real pr1[D3Q27System::ENDF+1];
    real phi_cutoff[D3Q27System::ENDF+1];

    real gradX1_phi();
    real gradX2_phi();
    real gradX3_phi();
	real gradX1_rhoInv(real rhoL, real rhoDIV);
	real gradX2_rhoInv(real rhoL, real rhoDIV);
	real gradX3_rhoInv(real rhoL, real rhoDIV);
    real gradX1_phi2();
    real gradX2_phi2();
    real gradX3_phi2();
    void computePhasefield();
    void findNeighbors(CbArray3D<real,IndexerX3X2X1>::CbArray3DPtr ph /*Phase-Field*/, int x1, int x2, int x3);
    void findNeighbors2(CbArray3D<real, IndexerX3X2X1>::CbArray3DPtr ph, int x1, int x2, int x3);
    bool isGas(real phiLim, real* phi, real* phi2);

    real nabla2_phi();

    real computeCurvature_phi();

    mu::value_type muX1,muX2,muX3;
    mu::value_type muDeltaT;
    mu::value_type muNu;
    mu::value_type muRho;
    real forcingX1;
    real forcingX2;
    real forcingX3;
};

/// @brief The function computes a fancy expression
/// @param phiLim 
/// @param phi 
/// @param phi2 
/// @return 
inline bool MultiphaseScaleDistributionLBMKernel::isGas(real phiLim, real* phi, real* phi2)
{
    using namespace vf::lbm::dir;
    return (phi2[DIR_000] <= phiLim) || ((phi[DIR_000] <= phiLim) &&
                        (
						(phi[DIR_P00] > phiLim) ||
						(phi[DIR_M00] > phiLim) ||
						(phi[DIR_00P] > phiLim) ||
						(phi[DIR_00M] > phiLim) ||
						(phi[DIR_0M0] > phiLim) ||
						(phi[DIR_0P0] > phiLim) ||
						(phi[DIR_PP0] > phiLim) ||
						(phi[DIR_PM0] > phiLim) ||
						(phi[DIR_P0P] > phiLim) ||
						(phi[DIR_P0M] > phiLim) ||
						(phi[DIR_MP0] > phiLim) ||
						(phi[DIR_MM0] > phiLim) ||
						(phi[DIR_M0P] > phiLim) ||
						(phi[DIR_M0M] > phiLim) ||
						(phi[DIR_0PM] > phiLim) ||
						(phi[DIR_0MM] > phiLim) ||
						(phi[DIR_0PP] > phiLim) ||
						(phi[DIR_0MP] > phiLim) ||
						(phi[DIR_PPP] > phiLim) ||
						(phi[DIR_PMP] > phiLim) ||
						(phi[DIR_MPP] > phiLim) ||
						(phi[DIR_MMP] > phiLim) ||
						(phi[DIR_PPM] > phiLim) ||
						(phi[DIR_PMM] > phiLim) ||
						(phi[DIR_MPM] > phiLim) ||
						(phi[DIR_MMM] > phiLim)
						));
}

#endif

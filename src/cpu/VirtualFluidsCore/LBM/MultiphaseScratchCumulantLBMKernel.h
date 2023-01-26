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
//! \file MultiphaseScratchCumulantLBMKernel.h
//! \ingroup LBMKernel
//! \author Hesameddin Safari
//=======================================================================================

#ifndef MultiphaseScratchCumulantLBMKernel_H
#define MultiphaseScratchCumulantLBMKernel_H

#include "LBMKernel.h"
#include "BCProcessor.h"
#include "D3Q27System.h"
#include "basics/utilities/UbTiming.h"
#include "basics/container/CbArray4D.h"
#include "basics/container/CbArray3D.h"

//! \brief  Multiphase Cascaded Cumulant LBM kernel. 
//! \details CFD solver that use Cascaded Cumulant Lattice Boltzmann method for D3Q27 model
//! \author  H. Safari, K. Kutscher, M. Geier
class MultiphaseScratchCumulantLBMKernel : public LBMKernel
{
public:
   MultiphaseScratchCumulantLBMKernel();
   virtual ~MultiphaseScratchCumulantLBMKernel(void) = default;
   void calculate(int step) override;
   SPtr<LBMKernel> clone() override;
   void forwardInverseChimeraWithKincompressible(real& mfa, real& mfb, real& mfc, real vv, real v2, real Kinverse, real K, real oneMinusRho);
   void backwardInverseChimeraWithKincompressible(real& mfa, real& mfb, real& mfc, real vv, real v2, real Kinverse, real K, real oneMinusRho);
   void forwardChimera(real& mfa, real& mfb, real& mfc, real vv, real v2);
   void backwardChimera(real& mfa, real& mfb, real& mfc, real vv, real v2);

   real getCalculationTime() override { return .0; }
protected:
   virtual void initDataSet();
   void swapDistributions() override;
   real f1[D3Q27System::ENDF+1];

   CbArray4D<real,IndexerX4X3X2X1>::CbArray4DPtr localDistributionsF;
   CbArray4D<real,IndexerX4X3X2X1>::CbArray4DPtr nonLocalDistributionsF;
   CbArray3D<real,IndexerX3X2X1>::CbArray3DPtr   zeroDistributionsF;

   CbArray4D<real,IndexerX4X3X2X1>::CbArray4DPtr localDistributionsH;
   CbArray4D<real,IndexerX4X3X2X1>::CbArray4DPtr nonLocalDistributionsH;
   CbArray3D<real,IndexerX3X2X1>::CbArray3DPtr   zeroDistributionsH;

   //CbArray3D<LBMReal,IndexerX3X2X1>::CbArray3DPtr   phaseField;

   real h  [D3Q27System::ENDF+1];
   real g  [D3Q27System::ENDF+1];
   real phi[D3Q27System::ENDF+1];
   real pr1[D3Q27System::ENDF+1];
   real phi_cutoff[D3Q27System::ENDF+1];

   real gradX1_phi();
   real gradX2_phi();
   real gradX3_phi();
   //LBMReal gradX1_pr1();
   //LBMReal gradX2_pr1();
   //LBMReal gradX3_pr1();
   //LBMReal dirgradC_phi(int n, int k);
   void computePhasefield();
   void findNeighbors(CbArray3D<real,IndexerX3X2X1>::CbArray3DPtr ph /*Phase-Field*/, int x1, int x2, int x3);
   //void findNeighbors(CbArray3D<LBMReal,IndexerX3X2X1>::CbArray3DPtr ph /*Phase-Field*/, CbArray3D<LBMReal,IndexerX3X2X1>::CbArray3DPtr pf /*Pressure-Field*/, int x1, int x2, int x3);
   //void pressureFiltering(CbArray3D<LBMReal,IndexerX3X2X1>::CbArray3DPtr pf /*Pressure-Field*/, CbArray3D<LBMReal,IndexerX3X2X1>::CbArray3DPtr pf_filtered /*Pressure-Field*/);

   real nabla2_phi();


   mu::value_type muX1,muX2,muX3;
   mu::value_type muDeltaT;
   mu::value_type muNu;
   real forcingX1;
   real forcingX2;
   real forcingX3;
};

#endif

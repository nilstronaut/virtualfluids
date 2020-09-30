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
//! \file BCAlgorithm.h
//! \ingroup BoundarConditions
//! \author Konstantin Kutscher
//=======================================================================================

#ifndef BOUNDARYCONDITIONS_H
#define BOUNDARYCONDITIONS_H

#include <PointerDefinitions.h>

#include "D3Q27System.h"

class DistributionArray3D;
class BCArray3D;
class BoundaryConditions;

//! \brief Abstract class of baundary conditions algorithm
//! \details  BCAlgorithm provides interface for implementation of diferent boundary conditions
class BCAlgorithm
{
public:
   static const char VelocityBCAlgorithm = 0;
   static const char EqDensityBCAlgorithm = 1;
   static const char NonEqDensityBCAlgorithm = 2;
   static const char NoSlipBCAlgorithm = 3;
   static const char SlipBCAlgorithm = 4;
   static const char HighViscosityNoSlipBCAlgorithm = 5;
   static const char ThinWallNoSlipBCAlgorithm = 6;
   static const char VelocityWithDensityBCAlgorithm = 7;
   static const char NonReflectingOutflowBCAlgorithm = 8;

public:
   BCAlgorithm();
   virtual ~BCAlgorithm() {}
   
   virtual void addDistributions(SPtr<DistributionArray3D> distributions) = 0;
   void setNodeIndex(int x1, int x2, int x3);
   void setBcPointer(SPtr<BoundaryConditions> bcPtr);
   void setCompressible(bool c);
   void setCollFactor(LBMReal cf);
   char getType();
   bool isPreCollision();
   virtual SPtr<BCAlgorithm> clone() = 0;
   SPtr<BCArray3D> getBcArray();
   void setBcArray(SPtr<BCArray3D> bcarray);
   virtual void applyBC() = 0;

protected:
   bool compressible;
   char type;
   bool preCollision;

   SPtr<BoundaryConditions> bcPtr;
   SPtr<DistributionArray3D> distributions;
   SPtr<BCArray3D> bcArray;

   LBMReal collFactor;
   int x1, x2, x3;

   LBMReal compressibleFactor;

   typedef void(*CalcMacrosFct)(const LBMReal* const& /*f[27]*/, LBMReal& /*rho*/, LBMReal& /*vx1*/, LBMReal& /*vx2*/, LBMReal& /*vx3*/);
   typedef LBMReal(*CalcFeqForDirFct)(const int& /*direction*/, const LBMReal& /*(d)rho*/, const LBMReal& /*vx1*/, const LBMReal& /*vx2*/, const LBMReal& /*vx3*/);
   typedef  void(*CalcFeqFct)(LBMReal* const& /*feq/*[27]*/, const LBMReal& /*rho*/, const LBMReal& /*vx1*/, const LBMReal& /*vx2*/, const LBMReal& /*vx3*/);
   
   CalcFeqForDirFct calcFeqsForDirFct ;
   CalcMacrosFct    calcMacrosFct;
   CalcFeqFct       calcFeqFct; 
};


#endif 

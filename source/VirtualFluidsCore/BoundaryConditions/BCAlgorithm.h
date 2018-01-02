
#ifndef BOUNDARYCONDITIONS_H
#define BOUNDARYCONDITIONS_H

#include <vector>
#include <string>

#include "BoundaryConditions.h"
#include "D3Q27System.h"
#include "EsoTwist3D.h"
#include "BCArray3D.h"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

#include <boost/shared_ptr.hpp>

class BCAlgorithm;
typedef boost::shared_ptr<BCAlgorithm> BCAlgorithmPtr;

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
   
   virtual void addDistributions(DistributionArray3DPtr distributions) = 0;
   void setNodeIndex(int x1, int x2, int x3);
   void setBcPointer(BoundaryConditionsPtr bcPtr);
   void setCompressible(bool c);
   void setCollFactor(LBMReal cf);
   char getType();
   bool isPreCollision();
   virtual BCAlgorithmPtr clone()=0;
   BCArray3DPtr getBcArray();
   void setBcArray(BCArray3DPtr bcarray);
   virtual void applyBC() = 0;

protected:
   bool compressible;
   char type;
   bool preCollision;

   BoundaryConditionsPtr bcPtr;
   DistributionArray3DPtr distributions;
   BCArray3DPtr bcArray;

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

#include "NonReflectingVelocityBCAlgorithm.h"

NonReflectingVelocityBCAlgorithm::NonReflectingVelocityBCAlgorithm()
{
   BCAlgorithm::type = BCAlgorithm::NonReflectingVelocityBCAlgorithm;
   BCAlgorithm::preCollision = false;
}
//////////////////////////////////////////////////////////////////////////
NonReflectingVelocityBCAlgorithm::~NonReflectingVelocityBCAlgorithm()
{

}
//////////////////////////////////////////////////////////////////////////
BCAlgorithmPtr NonReflectingVelocityBCAlgorithm::clone()
{
   BCAlgorithmPtr bc(new NonReflectingVelocityBCAlgorithm());
   return bc;
}
//////////////////////////////////////////////////////////////////////////
void NonReflectingVelocityBCAlgorithm::addDistributions(DistributionArray3DPtr distributions)
{
   this->distributions = distributions;
}
//////////////////////////////////////////////////////////////////////////
void NonReflectingVelocityBCAlgorithm::applyBC()
{
   //velocity bc for non reflecting pressure bc
   LBMReal f[D3Q27System::ENDF+1];
   LBMReal feq[D3Q27System::ENDF+1];
   distributions->getDistributionInv(f, x1, x2, x3);
   LBMReal rho, vx1, vx2, vx3, drho;
   calcMacrosFct(f, drho, vx1, vx2, vx3);
   calcFeqFct(feq, drho, vx1, vx2, vx3);
   
   //if (compressible)
   //{
   //   rho = 1.0+drho;
   //}
   //else
   //{
   //   rho = 1.0;
   //}

   rho = 1.0+drho*compressibleFactor;

   for (int fdir = D3Q27System::FSTARTDIR; fdir <= D3Q27System::FENDDIR; fdir++)
   {
      if (bcPtr->hasVelocityBoundaryFlag(fdir))
      {
         const int invDir = D3Q27System::INVDIR[fdir];
         LBMReal q = 1.0;//bcPtr->getQ(invDir);// m+m q=0 stabiler
         LBMReal velocity = bcPtr->getBoundaryVelocity(invDir);
         //LBMReal fReturn = ((1.0-q)/(1.0+q))*((ftemp[invDir]-feq[invDir]*collFactor)/(1.0-collFactor))+((q*(ftemp[invDir]+ftemp[fdir])-velocity)/(1.0+q));
         //LBMReal fReturn = ((1.0 - q) / (1.0 + q))*((f[invDir] - feq[invDir]) / (1.0 - collFactor) + feq[invDir]) + ((q*(f[invDir] + f[fdir]) - velocity) / (1.0 + q))-drho*D3Q27System::WEIGTH[invDir];
         LBMReal fReturn = ((1.0 - q) / (1.0 + q))*((f[invDir] - feq[invDir]) / (1.0 - collFactor) + feq[invDir]) + ((q*(f[invDir] + f[fdir]) - velocity*rho) / (1.0 + q))-drho*D3Q27System::WEIGTH[invDir];
         distributions->setDistributionForDirection(fReturn, x1 + D3Q27System::DX1[invDir], x2 + D3Q27System::DX2[invDir], x3 + D3Q27System::DX3[invDir], fdir);
      }
   }
}

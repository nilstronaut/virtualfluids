#ifndef ChangeRandomQs_h__
#define ChangeRandomQs_h__

#include "LBMKernel.h"
#include "IntegrateValuesHelper.h"
#include "BoundaryConditions.h"
#include "BCArray3D.h"
#include "BCProcessor.h"

namespace Utilities
{
   void ChangeRandomQs(IntegrateValuesHelperPtr integrateValues)
   {
      std::vector<IntegrateValuesHelper::CalcNodes> cnodes = integrateValues->getCNodes();
      
      BOOST_FOREACH(IntegrateValuesHelper::CalcNodes cn, cnodes)
      {
         LBMKernelPtr kernel = cn.block->getKernel();
         BCArray3D& bcArray = kernel->getBCProcessor()->getBCArray();
         BOOST_FOREACH(UbTupleInt3 node, cn.nodes)
         {
            BoundaryConditionsPtr bc = bcArray.getBC(val<1>(node), val<2>(node), val<3>(node));
            if (bc)
            {
	            for (int fdir=D3Q27System::FSTARTDIR; fdir<=D3Q27System::FENDDIR; fdir++)
	            {
                  if (bc->hasNoSlipBoundaryFlag(fdir))
                  {
                     const int invDir = D3Q27System::INVDIR[fdir];
                     float q = bc->getQ(invDir);
                     //double r = (double)UbRandom::rand(-50, 50);
                     float r = (float)UbRandom::rand(-10, 10);
                     float q_temp = q + q/r;
                     if (q_temp < 0.0)
                     {
                        q_temp = 0.0001f;
                     }
                     else if (q_temp > 1.0)
                     {
                        q_temp = 0.9999f;
                     }
                     bc->setQ(q_temp, fdir);
                  }
	            }
            }
         }
      }
   }

}
#endif // ChangeRandomQs_h__

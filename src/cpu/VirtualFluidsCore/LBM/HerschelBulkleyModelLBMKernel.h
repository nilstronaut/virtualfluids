#ifndef HerschelBulkleyModelLBMKernel_H
#define HerschelBulkleyModelLBMKernel_H

#include "ThixotropyModelLBMKernel.h"
#include "Thixotropy.h"

//! \brief    Cumulant LBM kernel + Herschel-Bulkley plastic model 
//! \author K. Kutscher, M. Geier
class HerschelBulkleyModelLBMKernel : public ThixotropyModelLBMKernel
{
public:
	HerschelBulkleyModelLBMKernel() {};
	~HerschelBulkleyModelLBMKernel() {};
	SPtr<LBMKernel> clone() override
	{
		SPtr<LBMKernel> kernel(new HerschelBulkleyModelLBMKernel());
		kernel->setNX(nx);
		kernel->setCollisionFactor(collFactor);
		dynamicPointerCast<HerschelBulkleyModelLBMKernel>(kernel)->initDataSet();
		kernel->setBCProcessor(bcProcessor->clone(kernel));
		kernel->setWithForcing(withForcing);
		kernel->setForcingX1(muForcingX1);
		kernel->setForcingX2(muForcingX2);
		kernel->setForcingX3(muForcingX3);
		kernel->setIndex(ix1, ix2, ix3);
		kernel->setDeltaT(deltaT);

		return kernel;
	}
protected:
	LBMReal getThyxotropyCollFactor(LBMReal omegaInf, LBMReal shearRate, LBMReal drho) const override
	{
		return Thixotropy::getHerschelBulkleyCollFactor(omegaInf, shearRate, drho);
	}
};


#endif
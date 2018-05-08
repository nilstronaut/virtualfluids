#include "VzFFTCalculator.h"

#include "Utilities\Results\Results.h"


std::shared_ptr<VzFFTCalculator> VzFFTCalculator::getNewInstance(double viscosity, std::shared_ptr<PhiAndNuTestResults> testResults)
{
	return std::shared_ptr<VzFFTCalculator>(new VzFFTCalculator(viscosity, testResults));
}

void VzFFTCalculator::setVectorToCalc()
{
	data = simResults->getVz();
}

VzFFTCalculator::VzFFTCalculator(double viscosity, std::shared_ptr<PhiAndNuTestResults> testResults) : FFTCalculator(viscosity, testResults)
{
}

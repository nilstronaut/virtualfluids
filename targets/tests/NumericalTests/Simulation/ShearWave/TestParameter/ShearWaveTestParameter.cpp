#include "ShearWaveTestParameter.h"

#include "Simulation/ShearWave/InitialConditions/InitialConditionShearWave.h"
#include "Utilities/Calculator/FFTCalculator/VzFFTCalculator/VzFFTCalculator.h"
#include "Tests/PhiAndNuTest/PhiAndNuTest.h"
#include "Utilities/SimulationResults/SimulationResults.h"

#include <sstream>

std::shared_ptr<TestParameter> ShearWaveTestParameter::getNewInstance(	real u0, real v0, real viscosity, unsigned int lx, unsigned int numberOfTimeSteps, unsigned int basisTimeStepLength, 
																		unsigned int startStepCalculation, unsigned int ySliceForCalculation, std::string gridPath, bool writeFiles,
																		unsigned int startStepFileWriter, std::string filePath, std::shared_ptr<PhiAndNuTest> testResults,
																		std::vector<int> devices)
{
	return std::shared_ptr<TestParameter>(new ShearWaveTestParameter(	u0, v0, viscosity, lx, numberOfTimeSteps, basisTimeStepLength, startStepCalculation, ySliceForCalculation, gridPath,
																		writeFiles, startStepFileWriter, filePath,testResults, devices));
}

double ShearWaveTestParameter::getMaxVelocity()
{
	if(u0 > v0)
		return u0 / (lx / l0);
	return v0 / (lx / l0);
}

ShearWaveTestParameter::ShearWaveTestParameter(	real u0, real v0, real viscosity, unsigned int lx, unsigned int numberOfTimeSteps, unsigned int basisTimeStepLength, unsigned int startStepCalculation, unsigned int ySliceForCalculation, std::string gridPath, bool writeFiles, unsigned int startStepFileWriter, std::string filePath, std::shared_ptr<PhiAndNuTest> testResults, std::vector<int> devices)
:TestParameterImp(viscosity, lx, numberOfTimeSteps, basisTimeStepLength, startStepCalculation, ySliceForCalculation, gridPath, writeFiles, startStepFileWriter, testResults, devices), u0(u0), v0(v0)
{
	std::ostringstream oss;
	oss << filePath + "/ShearWave/grid" << lx;
	this->filePath = oss.str();

	initialCondition = InitialConditionShearWave::getNewInstance((double)lx, (double)lz, (double)l0, u0, v0, rho0);
	calculator = VzFFTCalculator::getNewInstance(viscosity, testResults);
}
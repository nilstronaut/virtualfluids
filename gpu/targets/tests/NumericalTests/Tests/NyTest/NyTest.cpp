#include "NyTest.h"

#include "Utilities/ColorConsoleOutput/ColorConsoleOutput.h"
#include "Utilities/TestSimulation/TestSimulation.h"
#include "Utilities/SimulationInfo/SimulationInfo.h"

#include "Tests/NyTest/PostProcessingStrategy/NyTestPostProcessingStrategy.h"
#include "Tests/NyTest/NyTestParameterStruct.h"

#include <iomanip>

std::shared_ptr<NyTest> NyTest::getNewInstance(std::shared_ptr<ColorConsoleOutput> colorOutput, double viscosity, std::shared_ptr<NyTestParameterStruct> testPara, std::string dataToCalculate)
{
	return std::shared_ptr<NyTest>(new NyTest(colorOutput, viscosity, testPara, dataToCalculate));
}

void NyTest::evaluate()
{
	for (int i = 0; i < postProStrategies.size(); i++)
		ny.push_back(postProStrategies.at(i)->getNy(dataToCalculate));
	
	if (checkNy(ny)) {
		nyDiff = calcNyDiff(ny);
		orderOfAccuracy = calcOrderOfAccuracy(nyDiff);
		testStatus = checkTestPassed(orderOfAccuracy);
	}
	else
		testStatus = test_error;
	

	makeConsoleOutput();
}

void NyTest::update()
{
	TestImp::update();
}

void NyTest::addSimulation(std::shared_ptr<NumericalTestSimulation> sim, std::shared_ptr<SimulationInfo> simInfo, std::shared_ptr<NyTestPostProcessingStrategy> postProStrategy)
{
	TestImp::addSimulation(sim, simInfo, postProStrategy);
	postProStrategies.push_back(postProStrategy);
	lx.push_back(postProStrategy->getNumberOfXNodes());
}

std::string NyTest::getDataToCalculate()
{
	return dataToCalculate;
}

std::vector<int> NyTest::getLx()
{
	std::vector<int> lxINT;
	for (int i = 0; i < lx.size(); i++)
		lxINT.push_back((int)lx.at(i));
	return lxINT;
}

std::vector<double> NyTest::getNy()
{
	return ny;
}

std::vector<double> NyTest::getNyDiff()
{
	return nyDiff;
}

double NyTest::getOrderOfAccuracyNyDiff()
{
	return orderOfAccuracy;
}

NyTest::NyTest(std::shared_ptr<ColorConsoleOutput> colorOutput, double viscosity, std::shared_ptr<NyTestParameterStruct> testPara, std::string dataToCalculate)
	: TestImp(colorOutput), viscosity(viscosity), dataToCalculate(dataToCalculate)
{
	minOrderOfAccuracy = testPara->minOrderOfAccuracy;
	startStepCalculation = testPara->startTimeStepCalculation;
	endStepCalculation = testPara->endTimeStepCalculation;

	lx.resize(0);
	nyDiff.resize(0);
}

double NyTest::calcOrderOfAccuracy(std::vector<double> data)
{
	double ooa = log(data.at(0) / data.at(1)) / log(lx.at(1) / lx.at(0));
	
	return ooa;
}

TestStatus NyTest::checkTestPassed(double orderOfAccuracy)
{
	if (orderOfAccuracy > minOrderOfAccuracy)
		return passed;
	else
		return failed;
}

bool NyTest::checkNy(std::vector<double> ny)
{
	for(int i = 0; i < ny.size(); i++)
		if(ny.at(i) < 0.0)
			return false;
	return true;
}

std::vector<double> NyTest::calcNyDiff(std::vector<double> ny)
{
	std::vector<double> results;
	for (int i = 0; i < ny.size(); i++)
		results.push_back(abs((ny.at(i) - viscosity) / viscosity));
	return results;
}

std::vector<std::string> NyTest::buildTestOutput()
{
	std::vector<std::string> output = buildBasicTestOutput();
	std::ostringstream oss;

	for (int i = 0; i < ny.size(); i++) {
		oss << "Ny" << simInfos.at(i)->getLx() << ": " << ny.at(i);
		output.push_back(oss.str());
		oss.str(std::string());

		oss << "NyDiff" << simInfos.at(i)->getLx() << ": " << nyDiff.at(i);
		output.push_back(oss.str());
		oss.str(std::string());
	}
	oss << "OrderOfAccuracy: " << orderOfAccuracy;
	output.push_back(oss.str());
	oss.str(std::string());

	return output;
}

std::vector<std::string> NyTest::buildBasicTestOutput()
{
	std::vector<std::string> output;
	std::ostringstream oss;

	output.push_back("Ny Test");

	oss << "Kernel: " << simInfos.at(0)->getKernelName();
	output.push_back(oss.str());
	oss.str(std::string());

	oss << "Viscosity: " << simInfos.at(0)->getViscosity();
	output.push_back(oss.str());
	oss.str(std::string());

	output.push_back(oss.str());

	oss << simInfos.at(0)->getSimulationName();
	output.push_back(oss.str());
	oss.str(std::string());

	for (int i = 0; i < simInfos.size(); i++) {
		oss << "L: " << std::setfill(' ') << std::right << std::setw(4) << simInfos.at(i)->getLx() << simInfos.at(i)->getSimulationParameterString();
		output.push_back(oss.str());
		oss.str(std::string());
	}

	output.push_back(oss.str());

	oss << "DataToCalculate: " << dataToCalculate;
	output.push_back(oss.str());
	oss.str(std::string());

	oss << "StartTimeStep: " << startStepCalculation;
	output.push_back(oss.str());
	oss.str(std::string());

	oss << "EndTimeStep: " << endStepCalculation;
	output.push_back(oss.str());
	oss.str(std::string());

	output.push_back(oss.str());

	return output;
}

std::vector<std::string> NyTest::buildErrorTestOutput()
{
	std::vector<std::string> output = buildBasicTestOutput();
	std::ostringstream oss;

	oss << "Error Message: Ny < 0";
	output.push_back(oss.str());
	oss.str(std::string());

	return output;
}

std::vector<std::string> NyTest::buildSimulationFailedTestOutput()
{
	std::vector<std::string> output = buildBasicTestOutput();
	std::ostringstream oss;

	oss << "Simulation crashed!";
	output.push_back(oss.str());
	oss.str(std::string());

	return output;
}

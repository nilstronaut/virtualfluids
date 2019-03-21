#include "ConcentrationByPosition.h"

#include <iostream>

ConcentrationByPosition::ConcentrationByPosition(uint roadlength, real * concArrayStart, uint maxSpeed)
{
	if (concArrayStart == nullptr) {
		std::cout << "using ConcentrationByPosition::concentration for concentrations" << std::endl;
		concentration.resize(roadlength);
	}
	else {
		std::cout << "using passed array for concentrations" << std::endl;
		useLBMConcArray = true;
		this->roadLength = roadLength;
		this->concArrayStart = concArrayStart;
	}

}

//void ConcentrationByPosition::calculateConcFromCarDistribution(const std::vector<int>& currentCarDistribution)
//{
//	for (uint i = 0; i < currentCarDistribution.size(); i++) {
//		if (currentCarDistribution[i] >= 0) 
//			concentration[i] = 1.0;
//		else
//			concentration[i] = 0.0;
//	}
//
//	//dispConcentration();
//}


void ConcentrationByPosition::calculateConcForSingleCar(uint index, uint oldSpeed, uint speed)
{
	putConcIntoArrayOrVector(index, 1.0);
}

void ConcentrationByPosition::calculateConcForJunctionCar(uint index, uint oldSpeed, uint speed)
{
	addConcToArrayOrVector(index, 1.0);
}



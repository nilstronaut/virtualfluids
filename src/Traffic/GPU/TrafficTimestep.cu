#include "TrafficTimestep.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <curand.h>
#include <curand_kernel.h>

#include <cmath>
#include <sstream>

#include <thrust/device_vector.h>
#include <thrust/reduce.h>
#include <thrust/device_ptr.h>
#include <thrust/host_vector.h>

#include <iomanip>

//#include "TrafficMovement.h"
#include "RoadNetwork/RoadNetworkData.h"
#include "Junction/Junction.h"
#include "Sink/Sink.h"
#include "Source/Source.h"

__global__   void trafficTimestepKernel(int* roadCurrent, int* roadNext, int* neighbors, int* oldSpeeds, uint* junctionInCellIndices, bool* junctionCarCanEnter, int* junctionCarsOnJunction, uint* junctionAlreadyMoved, uint* JunctionOldSpeeds, bool* sinkCarCanEnter,
	uint size_road, uint maxVelocity, uint maxAcceleration, uint safetyDistance, bool useSlowToStart, real slowStartPossibility, real dawdlePossibility);

__device__ inline void trafficTimestepFunction(int* roadCurrent, int* roadNext, int* neighbors, int* oldSpeeds, uint* junctionInCellIndices, bool* junctionCarCanEnter, int* junctionCarsOnJunction, uint* junctionAlreadyMoved, uint* JunctionOldSpeeds, bool* sinkCarCanEnter,
	uint size_road, uint maxVelocity, uint maxAcceleration, uint safetyDistance, bool useSlowToStart, real slowStartPossibility, real dawdlePossibility, uint index);


__global__	void sourceTimestepKernel(int* roadCurrent, int* roadNext, int* neighbors, uint* sourceIndices, bool* sinkCarCanEnter, float* sourcePossibilities, int* oldSpeeds, uint maxVelocity, uint safetyDistance, uint size_sources);


__device__ inline uint getJunctionInCellsVectorIndex(uint * junctionInCellIndices, uint size_road, uint cell);
__device__ inline uint getGapAfterOutCell(int* roadCurrent, int* neighbors, bool* sinkCarCanEnter, int sourceIndex, uint speed, uint safetyDistance);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TrafficTimestep::TrafficTimestep(std::shared_ptr<RoadNetworkData> road)
{
	//calculate sizes
	size_roads = road->roadLength;
	size_junctions = road->junctions.size();
	size_sources = road->sources.size();
	size_sinks = road->sinks.size();

	//set attributes
	maxVelocity = road->maxVelocity;
	safetyDistance = road->safetyDistance;

	dawdlePossibility = 0.5f;
	useSlowToStart = false;
	slowStartPossibility = 0.1f;
	maxAcceleration = 2;

	//prepare road
	this->neighbors.resize(size_roads);
	this->neighbors = road->neighbors;
	roadCurrent.resize(size_roads);
	roadNext.resize(size_roads);
	oldSpeeds.resize(size_roads);

	//prepare junctions
	combineJunctionInCellIndices(road->junctions);
	junctionCarCanEnter.resize(junctionInCellIndices.size());
	junctionCarsOnJunction.resize(junctionInCellIndices.size());
	junctionAlreadyMoved.resize(junctionInCellIndices.size());
	JunctionOldSpeeds.resize(junctionInCellIndices.size());

	//prepare sinks
	sinkCarCanEnter.resize(size_sinks);

	//prepare sources
	combineSourcePossibilities(road->sources);
	combineSourceIndices(road->sources);

}

void TrafficTimestep::run(std::shared_ptr<RoadNetworkData> road)
{
	//copy road to device, reset next
	this->roadCurrent = *road->pcurrent;
	std::fill(roadNext.begin(), roadNext.end(), -1);

	//reset oldSpeeds
	std::fill(oldSpeeds.begin(), oldSpeeds.end(), -1);

	//copy junctions an sinks to device
	combineJunctionCarCanEnter(road->junctions);
	combineJunctionCarsOnJunction(road->junctions);
	combineJunctionAlreadyMoved(road->junctions);
	combineJunctionOldSpeeds(road->junctions);
	combineSinkCarCanEnterSink(road->sinks);

	callTrafficMovementKernel();
	//cudaDeviceSynchronize();
	getLastCudaError("trafficTimestepKernel execution failed");
	
	callSourceKernel();
	cudaDeviceSynchronize();
	getLastCudaError("sourceTimestepKernel execution failed");

	thrust::copy(roadNext.begin(), roadNext.end(), (*road->pnext).begin());
	thrust::copy(roadCurrent.begin(), roadCurrent.end(), (*road->pcurrent).begin());
	thrust::copy(oldSpeeds.begin(), oldSpeeds.end(), road->oldSpeeds.begin());

	//combineJunctionCarCanEnter(road->junctions);
	//combineJunctionCarsOnJunction(road->junctions);
	//combineJunctionAlreadyMoved(road->junctions);
	//combineJunctionOldSpeeds(road->junctions);
}

void TrafficTimestep::callTrafficMovementKernel()
{
	unsigned int numberOfThreads = 128;
	int Grid = (size_roads / numberOfThreads) + 1;
	int Grid1, Grid2;
	if (Grid > 512)
	{
		Grid1 = 512;
		Grid2 = (Grid / Grid1) + 1;
	}
	else
	{
		Grid1 = 1;
		Grid2 = Grid;
	}
	dim3 grid(Grid1, Grid2);
	dim3 threads(numberOfThreads, 1, 1);

	trafficTimestepKernel << < grid, threads >> > (
		roadCurrent.data().get(),
		roadNext.data().get(),
		neighbors.data().get(),
		oldSpeeds.data().get(),
		junctionInCellIndices.data().get(),
		junctionCarCanEnter.data().get(),
		junctionCarsOnJunction.data().get(),
		junctionAlreadyMoved.data().get(),
		JunctionOldSpeeds.data().get(),
		sinkCarCanEnter.data().get(),
		size_roads,
		maxVelocity,
		maxAcceleration,
		safetyDistance,
		useSlowToStart,
		slowStartPossibility,
		dawdlePossibility
		);
}

void TrafficTimestep::callSourceKernel()
{
	//calculate	grid and threads
	unsigned int numberOfThreads = 128;
	int Grid = (size_sinks / numberOfThreads) + 1;
	int Grid1, Grid2;
	if (Grid > 512)
	{
		Grid1 = 512;
		Grid2 = (Grid / Grid1) + 1;
	}
	else
	{
		Grid1 = 1;
		Grid2 = Grid;
	}
	dim3 grid(Grid1, Grid2);
	dim3 threads(numberOfThreads, 1, 1);

	sourceTimestepKernel << < grid, threads >> > ( 
		roadCurrent.data().get(),
		roadNext.data().get(), 
		neighbors.data().get(),
		sourceIndices.data().get(),
		sinkCarCanEnter.data().get(),
		sourcePossibilities.data().get(),
		oldSpeeds.data().get(),
		maxVelocity,
		safetyDistance,
		size_sources);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__global__ void trafficTimestepKernel(int* roadCurrent, int* roadNext, int* neighbors, int* oldSpeeds, uint* junctionInCellIndices, bool* junctionCarCanEnter, int* junctionCarsOnJunction, uint* junctionAlreadyMoved, uint* JunctionOldSpeeds, bool* sinkCarCanEnter,
	uint size_road, uint maxVelocity, uint maxAcceleration, uint safetyDistance, bool useSlowToStart, real slowStartPossibility, real dawdlePossibility)
{
	//////////////////////////////////////////////////////////////////////////
	const uint x = threadIdx.x;  // Globaler x-Index 
	const uint y = blockIdx.x;   // Globaler y-Index 
	const uint z = blockIdx.y;   // Globaler z-Index 

	const uint nx = blockDim.x;
	const uint ny = gridDim.x;

	const uint index = nx*(ny*z + y) + x;
	////////////////////////////////////////////////////////////////////////////////

	if (index >= size_road) return;


	trafficTimestepFunction(roadCurrent, roadNext, neighbors, oldSpeeds, junctionInCellIndices, junctionCarCanEnter, junctionCarsOnJunction, junctionAlreadyMoved, JunctionOldSpeeds, sinkCarCanEnter,
		size_road, maxVelocity, maxAcceleration, safetyDistance, useSlowToStart, slowStartPossibility, dawdlePossibility, index);
}

__device__ void trafficTimestepFunction(int* roadCurrent, int* roadNext, int* neighbors, int* oldSpeeds, uint* junctionInCellIndices, bool* junctionCarCanEnter, int* junctionCarsOnJunction, uint* junctionAlreadyMoved, uint* JunctionOldSpeeds, bool* sinkCarCanEnter,
	uint size_road, uint maxVelocity, uint maxAcceleration, uint safetyDistance, bool useSlowToStart, real slowStartPossibility, real dawdlePossibility, uint index)
{
	if (roadCurrent[index] < 0) return;

	uint speed = roadCurrent[index];



	//// accelerate car ////////////////////////////////////////////////////////////////////

	if (speed <= maxVelocity - maxAcceleration) speed += maxAcceleration;



	//// brake car /////////////////////////////////////////////////////////////////////////

	//getGapAfterCar
	uint gap;
	int neighbor = neighbors[index];
	uint currentCell = index;
	for (uint i = 0; i < (speed + safetyDistance); i++) {

		//sink
		if (neighbor <= -2000) {
			if (i <= speed && sinkCarCanEnter[(neighbor + 2000)*-1]) gap = speed;
			else gap = i;
			break;
		}

		//junction
		if (neighbor <= -1000) {
			if (junctionCarCanEnter[getJunctionInCellsVectorIndex(junctionInCellIndices, size_road, currentCell)] && i <= speed) gap = speed;
			else gap = i;
			break;
		}

		//car in Cell
		if (roadCurrent[neighbor] > -1) {
			if (i <= safetyDistance) gap = 0;
			else gap = i - safetyDistance;
			break;
		}

		//empty cell -> get next neighbor, update currentCell
		currentCell = neighbor;
		neighbor = neighbors[neighbor];
	}
	//brake
	if (speed > gap)
		speed = gap;



	//// dawdleCar ///////////////////////////////////////////////////////////////////////////
	curandState state;
	curand_init((unsigned long long)clock(), index, 0, &state);
	float randomNumber = curand_uniform_double(&state);



	//Barlovic / SlowToStart
	if (useSlowToStart == true && roadCurrent[index] == 0) {
		if (randomNumber < slowStartPossibility) speed = 0;
	}
	else if (randomNumber < dawdlePossibility) //Standard NaSch
		if (speed >= maxAcceleration)
			speed -= maxAcceleration;
		else
			speed = 0;



	//// moveCar /////////////////////////////////////////////////////////////////////////////
	if (speed == 0) {
		(roadNext)[index] = 0;
		oldSpeeds[index] = roadCurrent[index];
		return;
	}

	neighbor = neighbors[index];
	currentCell = index;

	// iterateNeighborsInMove
	uint numberOfCellsMoved = 1;
	for (uint i = 2; i <= speed; i++) {
		if (neighbor >= 0) {
			currentCell = neighbor;
			neighbor = neighbors[neighbor];
			++numberOfCellsMoved;
		}
		else
			break;
	}


	if (neighbor <= -1000 && neighbor > -2000) {
		//registerCar
		uint index = getJunctionInCellsVectorIndex(junctionInCellIndices, size_road, currentCell);
		junctionCarsOnJunction[index] = speed - 1; //all cars, which enter the junction have to slow down by one increment
		//getCarsOnJunction[index] = 0; //all cars, which enter the junction have to stop
		JunctionOldSpeeds[index] = roadCurrent[index];
		junctionCarCanEnter[index] = false;
		junctionAlreadyMoved[index] = numberOfCellsMoved;
		return;
	}

	if (neighbor >= 0) {
		roadNext[neighbor] = speed;
		oldSpeeds[neighbor] = roadCurrent[index];
	}
}




__global__ void sourceTimestepKernel(int* roadCurrent, int* roadNext, int* neighbors, uint* sourceIndices, bool* sinkCarCanEnter, float* sourcePossibilities, int* oldSpeeds, uint maxVelocity, uint safetyDistance, uint size_sources)
{
	//////////////////////////////////////////////////////////////////////////
	const uint x = threadIdx.x;  // Globaler x-Index 
	const uint y = blockIdx.x;   // Globaler y-Index 
	const uint z = blockIdx.y;   // Globaler z-Index 

	const uint nx = blockDim.x;
	const uint ny = gridDim.x;

	const uint index = nx*(ny*z + y) + x;
	////////////////////////////////////////////////////////////////////////////////

	if (index >= size_sources) return;

	int sourceIndex = sourceIndices[index];
	uint gap = getGapAfterOutCell(roadCurrent, neighbors, sinkCarCanEnter, sourceIndex, maxVelocity, safetyDistance);
	if (gap > 0) {
		//get car with random speed
		curandState state;
		curand_init((unsigned long long)clock(), /* the seed can be the same for each thread */
			index,   /* the sequence number should be different for each core */
			0,    /* the offset is how much extra we advance in the sequence for each call, can be 0 */
			&state);
		unsigned int speed = ceilf(curand_uniform(&state) * maxVelocity);

		roadNext[sourceIndex] = speed;
		oldSpeeds[sourceIndex] = speed;
	}
}




inline __device__ uint getGapAfterOutCell(int* roadCurrent, int* neighbors, bool* sinkCarCanEnter, int sourceIndex, uint speed, uint safetyDistance)
{
	if (roadCurrent[sourceIndex] > -1)
		return 0;

	for (uint i = 0; i < (speed + safetyDistance); i++) {
		//sink
		if (sourceIndex <= -2000) {
			if (i <= speed && sinkCarCanEnter[(sourceIndex + 2000)*-1])
				return speed;
			return i;
		}
		//junction
		if (sourceIndex <= -1000)
			return i;

		//car in Cell
		if (roadCurrent[sourceIndex] > -1) {
			if (i <= safetyDistance) return 0;
			return i - safetyDistance;
		}

		//empty cell -> get next neighbor
		sourceIndex = neighbors[sourceIndex];
	}
	return speed;
}

__device__ uint getJunctionInCellsVectorIndex(uint* junctionInCellIndices, uint size_road, uint cell) {
	for (uint i = 0; i < size_road; i++)
		if (junctionInCellIndices[i] == cell)
			return i;
	// Error: "no matching incoming cell to a junction found in: getJunctionInCellsVectorIndex()";
	return 65000;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TrafficTimestep::TrafficTimestep(RoadNetworkData* road, TrafficMovement* traffic)
//{
//	maxVelocity = road->maxVelocity;
//	vehicleLength = road->vehicleLength;
//	safetyDistance = road->safetyDistance;
//
//	dawdlePossibility = traffic->getDawdlePossibility();
//	useSlowToStart = traffic->getUseSlowToStart();
//	slowStartPossibility = traffic->getSlowToStartPossibility();
//	maxAcceleration = traffic->getMaxAcceleration();
//}



void TrafficTimestep::combineJunctionInCellIndices(std::vector<std::shared_ptr<Junction>> junctions)
{
	for (auto& j : junctions)
		for (uint i : j->getInCellIndices())
			this->junctionInCellIndices.push_back(i);
}

void TrafficTimestep::combineJunctionCarCanEnter(std::vector<std::shared_ptr<Junction>> junctions)
{
	uint it = 0;
	for (auto& j : junctions)
		for (bool i : j->getCarCanEnter()) {
			this->junctionCarCanEnter[it] = i;
			it++;
		}

}

void TrafficTimestep::combineJunctionCarsOnJunction(std::vector<std::shared_ptr<Junction>> junctions)
{
	uint it = 0;
	for (auto& j : junctions)
		for (int i : j->getCarsOnJunction()) {
			this->junctionCarsOnJunction[it] = i;
			it++;
		}
}


void TrafficTimestep::combineSinkCarCanEnterSink(std::vector<std::shared_ptr<Sink>> sinks)
{
	uint it = 0;
	for (auto& s : sinks) {
		this->sinkCarCanEnter[it] = s->carCanEnter();
		it++;
	}
}


void TrafficTimestep::combineSourcePossibilities(std::vector<std::shared_ptr<Source>> sources) {
	for (auto& s : sources)
		this->sourcePossibilities.push_back(s->getPossibility());
}

void TrafficTimestep::combineSourceIndices(std::vector<std::shared_ptr<Source>> sources)
{
	for (auto& s : sources)
		this->sourceIndices.push_back(s->getIndex());
}


void TrafficTimestep::combineJunctionAlreadyMoved(std::vector<std::shared_ptr<Junction>> junctions)
{
	uint it = 0;
	for (auto& j : junctions)
		for (int i : j->getCarsOnJunction()) {
			this->junctionAlreadyMoved[it] = i;
			it++;
		}
}

void TrafficTimestep::combineJunctionOldSpeeds(std::vector<std::shared_ptr<Junction>> junctions)
{
	uint it = 0;
	for (auto& j : junctions)
		for (int i : j->getCarsOnJunction()) {
			this->JunctionOldSpeeds[it] = i;
			it++;
		}
}
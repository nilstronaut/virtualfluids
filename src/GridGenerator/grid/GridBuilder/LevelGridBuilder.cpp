#include "LevelGridBuilder.h"

#include <stdio.h>
#include <iostream>
#include <GridGenerator/grid/GridStrategy/GridGpuStrategy/GridGpuStrategy.h>
#include <GridGenerator/grid/GridStrategy/GridCpuStrategy/GridCpuStrategy.h>
#include <GridGenerator/grid/partition/Partition.h>

#include <GridGenerator/geometries/Triangle/Triangle.h>
#include <GridGenerator/geometries/BoundingBox/BoundingBox.h>
#include <GridGenerator/geometries/TriangularMesh/TriangularMesh.h>


#include <GridGenerator/io/GridVTKWriter/GridVTKWriter.h>
#include <GridGenerator/io/SimulationFileWriter/SimulationFileWriter.h>
#include <GridGenerator/io/VTKWriterWrapper/UnstructuredGridWrapper.h>
#include <GridGenerator/io/VTKWriterWrapper/PolyDataWriterWrapper.h>

#include <GridGenerator/grid/NodeValues.h>

#include <GridGenerator/geometries/Arrow/ArrowImp.h>
#include <GridGenerator/utilities/transformator/ArrowTransformator.h>

#include <utilities/logger/Logger.h>

#include <GridGenerator/grid/GridFactory.h>
#include "grid/GridInterface.h"
#include "grid/Grid.h"

#include "grid/BoundaryConditions/BoundaryCondition.h"
#include "grid/BoundaryConditions/Side.h"

#include <GridGenerator/io/QLineWriter.h>


#define GEOFLUID 19
#define GEOSOLID 16

LevelGridBuilder::LevelGridBuilder(Device device, const std::string& d3qxx) : device(device), d3qxx(d3qxx)
{
}

std::shared_ptr<LevelGridBuilder> LevelGridBuilder::makeShared(Device device, const std::string& d3qxx)
{
    return SPtr<LevelGridBuilder>(new LevelGridBuilder(device, d3qxx));
}

void LevelGridBuilder::setVelocityBoundaryCondition(SideType sideType, real vx, real vy, real vz)
{
    if (sideType == SideType::GEOMETRY)
        setVelocityGeometryBoundaryCondition(vx, vy, vz);
    else
    {
        for (int level = 0; level < getNumberOfGridLevels(); level++)
        {
            SPtr<VelocityBoundaryCondition> velocityBoundaryCondition = VelocityBoundaryCondition::make(vx, vy, vz);

            auto side = SideFactory::make(sideType);

            velocityBoundaryCondition->side = side;
            velocityBoundaryCondition->side->addIndices(grids, level, velocityBoundaryCondition);

            boundaryConditions[level]->velocityBoundaryConditions.push_back(velocityBoundaryCondition);
        }
    }

	//// DEBUG
	//{
	//	std::ofstream file;

	//	file.open("M:/TestGridGeneration/results/Box2_25_Qs.csv");

	//	for (auto nodeQs : boundaryConditions[0]->geometryBoundaryCondition->qs) {
	//		for (auto q : nodeQs) {
	//			file << q << ", ";
	//		}
	//		file << "100" << std::endl;
	//	}

	//	file.close();
	//}
}

void LevelGridBuilder::setVelocityGeometryBoundaryCondition(real vx, real vy, real vz)
{
    geometryHasValues = true;

    for (int level = 0; level < getNumberOfGridLevels(); level++)
    {
		if (boundaryConditions[level]->geometryBoundaryCondition != nullptr)
		{
			boundaryConditions[level]->geometryBoundaryCondition->vx = vx;
			boundaryConditions[level]->geometryBoundaryCondition->vy = vy;
			boundaryConditions[level]->geometryBoundaryCondition->vz = vz;
			boundaryConditions[level]->geometryBoundaryCondition->side->addIndices(grids, level, boundaryConditions[level]->geometryBoundaryCondition);
		}
    }
}

void LevelGridBuilder::setPressureBoundaryCondition(SideType sideType, real rho)
{
    for (int level = 0; level < getNumberOfGridLevels(); level++)
    {
        SPtr<PressureBoundaryCondition> pressureBoundaryCondition = PressureBoundaryCondition::make(rho);

        auto side = SideFactory::make(sideType);
        pressureBoundaryCondition->side = side;
        pressureBoundaryCondition->side->addIndices(grids, level, pressureBoundaryCondition);

        boundaryConditions[level]->pressureBoundaryConditions.push_back(pressureBoundaryCondition);
    }
}

void LevelGridBuilder::setPeriodicBoundaryCondition(bool periodic_X, bool periodic_Y, bool periodic_Z)
{
    for( uint level = 0; level < this->grids.size(); level++ )
        grids[level]->setPeriodicity(periodic_X, periodic_Y, periodic_Z);
}

void LevelGridBuilder::setNoSlipBoundaryCondition(SideType sideType)
{
    for (int level = 0; level < getNumberOfGridLevels(); level++)
    {
        SPtr<VelocityBoundaryCondition> noSlipBoundaryCondition = VelocityBoundaryCondition::make(0.0, 0.0, 0.0);

        auto side = SideFactory::make(sideType);

        noSlipBoundaryCondition->side = side;
        noSlipBoundaryCondition->side->addIndices(grids, level, noSlipBoundaryCondition);

        boundaryConditions[level]->noSlipBoundaryConditions.push_back(noSlipBoundaryCondition);
    }
}



void LevelGridBuilder::copyDataFromGpu()
{
    for (const auto grid : grids)
    {
        auto gridGpuStrategy = std::dynamic_pointer_cast<GridGpuStrategy>(grid->getGridStrategy());
        if(gridGpuStrategy)
            gridGpuStrategy->copyDataFromGPU(std::static_pointer_cast<GridImp>(grid));
    }
        
}

LevelGridBuilder::~LevelGridBuilder()
{
    for (const auto grid : grids)
        grid->freeMemory();
}

SPtr<Grid> LevelGridBuilder::getGrid(uint level)
{
    return grids[level];
}


void LevelGridBuilder::getGridInformations(std::vector<int>& gridX, std::vector<int>& gridY,
    std::vector<int>& gridZ, std::vector<int>& distX, std::vector<int>& distY,
    std::vector<int>& distZ)
{
    for (const auto grid : grids)
    {
        gridX.push_back(int(grid->getNumberOfNodesX()));
        gridY.push_back(int(grid->getNumberOfNodesY()));
        gridZ.push_back(int(grid->getNumberOfNodesZ()));

        distX.push_back(int(grid->getStartX()));
        distY.push_back(int(grid->getStartY()));
        distZ.push_back(int(grid->getStartZ()));
    }
}


uint LevelGridBuilder::getNumberOfGridLevels() const
{
    return uint(grids.size());
}

uint LevelGridBuilder::getNumberOfNodesCF(int level)
{
    return this->grids[level]->getNumberOfNodesCF();
}

uint LevelGridBuilder::getNumberOfNodesFC(int level)
{
    return this->grids[level]->getNumberOfNodesFC();
}

void LevelGridBuilder::getGridInterfaceIndices(uint* iCellCfc, uint* iCellCff, uint* iCellFcc, uint* iCellFcf, int level) const
{
    this->grids[level]->getGridInterfaceIndices(iCellCfc, iCellCff, iCellFcc, iCellFcf);
}

void LevelGridBuilder::setOffsetFC(real * xOffFC, real * yOffFC, real * zOffFC, int level)
{
    for (uint i = 0; i < getNumberOfNodesFC(level); i++)
    {
        uint offset = this->grids[level]->getFC_offset()[i];

		xOffFC[i] = - this->grids[level]->getDirection()[ 3*offset + 0 ];
        yOffFC[i] = - this->grids[level]->getDirection()[ 3*offset + 1 ];
        zOffFC[i] = - this->grids[level]->getDirection()[ 3*offset + 2 ];
    }
}

void LevelGridBuilder::setOffsetCF(real * xOffCF, real * yOffCF, real * zOffCF, int level)
{
    for (uint i = 0; i < getNumberOfNodesCF(level); i++)
    {
        uint offset = this->grids[level]->getCF_offset()[i];

        xOffCF[i] = - this->grids[level]->getDirection()[ 3*offset + 0 ];
		yOffCF[i] = - this->grids[level]->getDirection()[ 3*offset + 1 ];
		zOffCF[i] = - this->grids[level]->getDirection()[ 3*offset + 2 ];
    }
}


uint LevelGridBuilder::getNumberOfNodes(unsigned int level) const
{
    return grids[level]->getSparseSize();
}


std::shared_ptr<Grid> LevelGridBuilder::getGrid(int level, int box)
{
    return this->grids[level];
}

void LevelGridBuilder::checkLevel(int level)
{
    if (level >= grids.size())
    { 
        std::cout << "wrong level input... return to caller\n";
        return; 
    }
}

void LevelGridBuilder::getDimensions(int &nx, int &ny, int &nz, const int level) const
{
    nx = grids[level]->getNumberOfNodesX();
    ny = grids[level]->getNumberOfNodesY();
    nz = grids[level]->getNumberOfNodesZ();
}

void LevelGridBuilder::getNodeValues(real *xCoords, real *yCoords, real *zCoords, unsigned int *neighborX, unsigned int *neighborY, unsigned int *neighborZ, unsigned int *geo, const int level) const
{
    grids[level]->getNodeValues(xCoords, yCoords, zCoords, neighborX, neighborY, neighborZ, geo);
}


uint LevelGridBuilder::getVelocitySize(int level) const
{
    uint size = 0;
    for (auto boundaryCondition : boundaryConditions[level]->velocityBoundaryConditions)
    {
        size += uint(boundaryCondition->indices.size());
    }
    return size;
}

void LevelGridBuilder::getVelocityValues(real* vx, real* vy, real* vz, int* indices, int level) const
{
    int allIndicesCounter = 0;
    for (auto boundaryCondition : boundaryConditions[level]->velocityBoundaryConditions)
    {
        for(int i = 0; i < boundaryCondition->indices.size(); i++)
        {
            indices[allIndicesCounter] = grids[level]->getSparseIndex(boundaryCondition->indices[i]) +1;  

            vx[allIndicesCounter] = boundaryCondition->vx;
            vy[allIndicesCounter] = boundaryCondition->vy;
            vz[allIndicesCounter] = boundaryCondition->vz;
            allIndicesCounter++;
        }
    }
}

uint LevelGridBuilder::getPressureSize(int level) const
{
    uint size = 0;
    for (auto boundaryCondition : boundaryConditions[level]->pressureBoundaryConditions)
    {
        size += uint(boundaryCondition->indices.size());
    }
    return size;
}

void LevelGridBuilder::getPressureValues(real* rho, int* indices, int* neighborIndices, int level) const
{
    int allIndicesCounter = 0;
    for (auto boundaryCondition : boundaryConditions[level]->pressureBoundaryConditions)
    {
        for (int i = 0; i < boundaryCondition->indices.size(); i++)
        {
            indices[allIndicesCounter] = grids[level]->getSparseIndex(boundaryCondition->indices[i]) + 1;

            neighborIndices[allIndicesCounter] = grids[level]->getSparseIndex(boundaryCondition->neighborIndices[i]) + 1;

            rho[allIndicesCounter] = boundaryCondition->rho;
            allIndicesCounter++;
        }
    }
}


void LevelGridBuilder::getVelocityQs(real* qs[27], int level) const
{
    int allIndicesCounter = 0;
    for (auto boundaryCondition : boundaryConditions[level]->velocityBoundaryConditions)
    {
        for (int i = 0; i < boundaryCondition->indices.size(); i++)
        {

            for (int dir = 0; dir < grids[level]->getEndDirection(); dir++)
            {
                if (grids[level]->getDirection()[dir * DIMENSION + boundaryCondition->side->getCoordinate()] != boundaryCondition->side->getDirection())
                    qs[dir][allIndicesCounter] = -1.0;
                else
                    qs[dir][allIndicesCounter] = 0.5;

				//uint neigborIndex = grids[level]->getSparseIndex(boundaryCondition->neighborIndices[i]) + 1;
            }
            allIndicesCounter++;
        }
    }
}

void LevelGridBuilder::getPressureQs(real* qs[27], int level) const
{
    int allIndicesCounter = 0;
    for (auto boundaryCondition : boundaryConditions[level]->pressureBoundaryConditions)
    {
        for (int i = 0; i < boundaryCondition->indices.size(); i++)
        {
            for (int dir = 0; dir < grids[level]->getEndDirection(); dir++)
            {
                if (grids[level]->getDirection()[dir * DIMENSION + boundaryCondition->side->getCoordinate()] != boundaryCondition->side->getDirection())
                    qs[dir][allIndicesCounter] = -1.0;
                else
                    qs[dir][allIndicesCounter] = 0.5;
            }
            allIndicesCounter++;
        }
    }
}


uint LevelGridBuilder::getGeometrySize(int level) const
{
    if (boundaryConditions[level]->geometryBoundaryCondition)
        return  boundaryConditions[level]->geometryBoundaryCondition->indices.size();
    
    return 0;
}

void LevelGridBuilder::getGeometryIndices(int* indices, int level) const
{
    for (uint i = 0; i <  boundaryConditions[level]->geometryBoundaryCondition->indices.size(); i++)
    {
        indices[i] = grids[level]->getSparseIndex(boundaryConditions[level]->geometryBoundaryCondition->indices[i]) + 1;
    }
}

bool LevelGridBuilder::hasGeometryValues() const
{
    return geometryHasValues;
}


void LevelGridBuilder::getGeometryValues(real* vx, real* vy, real* vz, int level) const
{
    for (uint i = 0; i < boundaryConditions[level]->geometryBoundaryCondition->indices.size(); i++)
    {
		vx[i] = boundaryConditions[level]->geometryBoundaryCondition->vx;
		vy[i] = boundaryConditions[level]->geometryBoundaryCondition->vy;
		vz[i] = boundaryConditions[level]->geometryBoundaryCondition->vz;
    }
}


void LevelGridBuilder::getGeometryQs(real* qs[27], int level) const
{
    for (int i = 0; i < boundaryConditions[level]->geometryBoundaryCondition->indices.size(); i++)
    {
        for (int dir = 0; dir <= grids[level]->getEndDirection(); dir++)
        {
            qs[dir][i] = boundaryConditions[level]->geometryBoundaryCondition->qs[i][dir];
        }
    }
}





void LevelGridBuilder::writeArrows(std::string fileName) const 
{
    QLineWriter::writeArrows(fileName, boundaryConditions[getNumberOfGridLevels() - 1]->geometryBoundaryCondition, grids[getNumberOfGridLevels() - 1]);
}

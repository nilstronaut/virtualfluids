//=======================================================================================
// ____          ____    __    ______     __________   __      __       __        __         
// \    \       |    |  |  |  |   _   \  |___    ___| |  |    |  |     /  \      |  |        
//  \    \      |    |  |  |  |  |_)   |     |  |     |  |    |  |    /    \     |  |        
//   \    \     |    |  |  |  |   _   /      |  |     |  |    |  |   /  /\  \    |  |        
//    \    \    |    |  |  |  |  | \  \      |  |     |   \__/   |  /  ____  \   |  |____    
//     \    \   |    |  |__|  |__|  \__\     |__|      \________/  /__/    \__\  |_______|   
//      \    \  |    |   ________________________________________________________________    
//       \    \ |    |  |  ______________________________________________________________|   
//        \    \|    |  |  |         __          __     __     __     ______      _______    
//         \         |  |  |_____   |  |        |  |   |  |   |  |   |   _  \    /  _____)   
//          \        |  |   _____|  |  |        |  |   |  |   |  |   |  | \  \   \_______    
//           \       |  |  |        |  |_____   |   \_/   |   |  |   |  |_/  /    _____  \   
//            \ _____|  |__|        |________|   \_______/    |__|   |______/    (_______/   
//
//  This file is part of VirtualFluids. VirtualFluids is free software: you can 
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of 
//  the License, or (at your option) any later version.
//  
//  VirtualFluids is distributed in the hope that it will be useful, but WITHOUT 
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
//  for more details.
//  
//  You should have received a copy of the GNU General Public License along
//  with VirtualFluids (see COPYING.txt). If not, see <http://www.gnu.org/licenses/>.
//
//! \file GridImp.h
//! \ingroup grid
//! \author Soeren Peters, Stephan Lenz
//=======================================================================================
#ifndef GRID_IMP_H
#define GRID_IMP_H

#include <array>

#include "Core/LbmOrGks.h"

#include "global.h"

#include "grid/distributions/Distribution.h"
#include "grid/Grid.h"
#include "grid/Cell.h"
#include "grid/Field.h" 

struct Vertex;
class GridStrategy;
class Object;
class BoundingBox;

extern int DIRECTIONS[DIR_END_MAX][DIMENSION];

class VF_PUBLIC GridImp : public enableSharedFromThis<GridImp>, public Grid
{
private:
    GridImp();
    GridImp(Object* object, real startX, real startY, real startZ, real endX, real endY, real endZ, real delta, SPtr<GridStrategy> gridStrategy, Distribution d, uint level);

public:
    virtual ~GridImp();
    static SPtr<GridImp> makeShared(Object* object, real startX, real startY, real startZ, real endX, real endY, real endZ, real delta, SPtr<GridStrategy> gridStrategy, Distribution d, uint level);

private:
    void initalNumberOfNodesAndSize();
    bool isValidSolidStopper(uint index) const;
	bool shouldBeBoundarySolidNode(uint index) const;
	bool isValidEndOfGridStopper(uint index) const;
    bool isValidEndOfGridBoundaryStopper(uint index) const;
    bool isOutSideOfGrid(Cell &cell) const;
    bool contains(Cell &cell, char type) const;
    void setNodeTo(Cell &cell, char type);

    bool nodeInPreviousCellIs(int index, char type) const;
    bool nodeInCellIs(Cell& cell, char type) const override;

    uint getXIndex(real x) const;
    uint getYIndex(real y) const;
    uint getZIndex(real z) const;

    uint level;

    real startX = 0.0, startY = 0.0, startZ = 0.0;
    real endX, endY, endZ;
    real delta = 1.0;

    bool xOddStart = false, yOddStart = false, zOddStart = false;

	uint nx, ny, nz;

	uint size;
    uint sparseSize;
    bool periodicityX = false, periodicityY = false, periodicityZ = false;

    Field field;
    Object* object;

    int *neighborIndexX, *neighborIndexY, *neighborIndexZ, *neighborIndexNegative;
    int *sparseIndices;

	uint *qIndices;     //maps from matrix index to qIndex
	real *qValues;
    uint *qPatches;

    SPtr<GridStrategy> gridStrategy;

public:
    void inital(const SPtr<Grid> fineGrid, uint numberOfLayers) override;

    void setPeriodicity(bool periodicityX, bool periodicityY, bool periodicityZ) override;
    void setPeriodicityX(bool periodicity) override;
    void setPeriodicityY(bool periodicity) override;
    void setPeriodicityZ(bool periodicity) override;

    bool getPeriodicityX() override;
    bool getPeriodicityY() override;
    bool getPeriodicityZ() override;

    void setCellTo(uint index, char type);
    void setNonStopperOutOfGridCellTo(uint index, char type);

    uint transCoordToIndex(const real &x, const real &y, const real &z) const override;
    void transIndexToCoords(uint index, real &x, real &y, real &z) const override;

    void freeMemory() override;

    uint getLevel(real levelNull) const;
    uint getLevel() const;

public:
    Distribution distribution;

    void initalNodeToOutOfGrid(uint index);

    void findInnerNode(uint index);

	void findEndOfGridStopperNode(uint index);

    void setNodeTo(uint index, char type);
    bool isNode(uint index, char type) const;
    bool nodeInNextCellIs(int index, char type) const;
    bool hasAllNeighbors(uint index) const;
    bool hasNeighborOfType(uint index, char type)const;
    bool cellContainsOnly(Cell &cell, char type) const;
    bool cellContainsOnly(Cell &cell, char typeA, char typeB) const;

    const Object* getObject() const override;

    Field getField() const;
    char getFieldEntry(uint index) const override;
    void setFieldEntry(uint matrixIndex, char type) override;


    real getDelta() const override;
    uint getSize() const override;
    uint getSparseSize() const override;
    int getSparseIndex(uint matrixIndex) const override;
    real* getDistribution() const override;
    int* getDirection() const override;
    int getStartDirection() const override;
    int getEndDirection() const override;

    real getStartX() const override;
    real getStartY() const override;
    real getStartZ() const override;
    real getEndX() const override;
    real getEndY() const override;
    real getEndZ() const override;
    uint getNumberOfNodesX() const override;
    uint getNumberOfNodesY() const override;
    uint getNumberOfNodesZ() const override;
    void getNodeValues(real *xCoords, real *yCoords, real *zCoords, uint *neighborX, uint *neighborY, uint *neighborZ, uint *neighborNegative, uint *geo) const override;

    int* getNeighborsX() const override;
    int* getNeighborsY() const override;
    int* getNeighborsZ() const override;
    int* getNeighborsNegative() const override;

    SPtr<GridStrategy> getGridStrategy() const override;


public:
    virtual void findSparseIndices(SPtr<Grid> fineGrid);

    void updateSparseIndices();
    void setNeighborIndices(uint index);
    real getFirstFluidNode(real coords[3], int direction, real startCoord) const;
    real getLastFluidNode(real coords[3], int direction, real startCoord) const;
private:
    void setStopperNeighborCoords(uint index);
    void getNeighborCoords(real &neighborX, real &neighborY, real &neighborZ, real x, real y, real z) const;
    real getNeighborCoord(bool periodicity, real endCoord, real coords[3], int direction) const;
    void getNegativeNeighborCoords(real &neighborX, real &neighborY, real &neighborZ, real x, real y, real z) const;
    real getNegativeNeighborCoord(bool periodicity, real endCoord, real coords[3], int direction) const;
    

    int getSparseIndex(const real &expectedX, const real &expectedY, const real &expectedZ) const;

private:
    friend class GridGpuStrategy;
    friend class GridCpuStrategy;
};

#endif

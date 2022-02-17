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
//           \       |  |  |        |  |_____   |   \_/   |   |  |   |  |_/  /    _____  |
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
//! \file Side.cpp
//! \ingroup grid
//! \author Soeren Peters, Stephan Lenz
//=======================================================================================
#include "Side.h"

#include "grid/BoundaryConditions/BoundaryCondition.h"
#include "grid/Grid.h"
#include "grid/NodeValues.h"

#include "utilities/math/Math.h"

using namespace gg;

void Side::addIndices(SPtr<Grid> grid, SPtr<BoundaryCondition> boundaryCondition, std::string coord, real constant,
                      real startInner, real endInner, real startOuter, real endOuter)
{
    for (real v2 = startOuter; v2 <= endOuter; v2 += grid->getDelta())
    {
        for (real v1 = startInner; v1 <= endInner; v1 += grid->getDelta())
        {
            const uint index = getIndex(grid, coord, constant, v1, v2);

            if ((index != INVALID_INDEX) && (  grid->getFieldEntry(index) == vf::gpu::FLUID
                                            || grid->getFieldEntry(index) == vf::gpu::FLUID_CFC
                                            || grid->getFieldEntry(index) == vf::gpu::FLUID_CFF
                                            || grid->getFieldEntry(index) == vf::gpu::FLUID_FCC
                                            || grid->getFieldEntry(index) == vf::gpu::FLUID_FCF ))
            {
                grid->setFieldEntry(index, boundaryCondition->getType());
                boundaryCondition->indices.push_back(index);
                setPressureNeighborIndices(boundaryCondition, grid, index);
                setStressSamplingIndices(boundaryCondition, grid, index);

                setQs(grid, boundaryCondition, index);

                boundaryCondition->patches.push_back(0);
            }

        }
    }
}

void Side::setPressureNeighborIndices(SPtr<BoundaryCondition> boundaryCondition, SPtr<Grid> grid, const uint index)
{
    auto pressureBoundaryCondition = std::dynamic_pointer_cast<PressureBoundaryCondition>(boundaryCondition);
    if (pressureBoundaryCondition)
    {
        real x, y, z;
        grid->transIndexToCoords(index, x, y, z);

        real nx = x;
        real ny = y;
        real nz = z;

        if (boundaryCondition->side->getCoordinate() == X_INDEX)
            nx = -boundaryCondition->side->getDirection() * grid->getDelta() + x;
        if (boundaryCondition->side->getCoordinate() == Y_INDEX)
            ny = -boundaryCondition->side->getDirection() * grid->getDelta() + y;
        if (boundaryCondition->side->getCoordinate() == Z_INDEX)
            nz = -boundaryCondition->side->getDirection() * grid->getDelta() + z;

        int neighborIndex = grid->transCoordToIndex(nx, ny, nz);
        pressureBoundaryCondition->neighborIndices.push_back(neighborIndex);
    }
}

void Side::setStressSamplingIndices(SPtr<BoundaryCondition> boundaryCondition, SPtr<Grid> grid, const uint index)
{
    auto stressBoundaryCondition = std::dynamic_pointer_cast<StressBoundaryCondition>(boundaryCondition);
    if (stressBoundaryCondition)
    {
        real x, y, z;
        grid->transIndexToCoords(index, x, y, z);

        real nx = x;
        real ny = y;
        real nz = z;

        if (boundaryCondition->side->getCoordinate() == X_INDEX)
            nx = -boundaryCondition->side->getDirection() * stressBoundaryCondition->samplingOffset * grid->getDelta() + x;
        if (boundaryCondition->side->getCoordinate() == Y_INDEX)
            ny = -boundaryCondition->side->getDirection() * stressBoundaryCondition->samplingOffset * grid->getDelta() + y;
        if (boundaryCondition->side->getCoordinate() == Z_INDEX)
            nz = -boundaryCondition->side->getDirection() * stressBoundaryCondition->samplingOffset * grid->getDelta() + z;

        uint samplingIndex = grid->transCoordToIndex(nx, ny, nz);
        stressBoundaryCondition->velocitySamplingIndices.push_back(samplingIndex);
    }
}

void Side::setQs(SPtr<Grid> grid, SPtr<BoundaryCondition> boundaryCondition, uint index)
{

    std::vector<real> qNode(grid->getEndDirection() + 1);

    for (int dir = 0; dir <= grid->getEndDirection(); dir++)
    {
        real x,y,z;
        grid->transIndexToCoords( index, x, y, z );

        x += grid->getDirection()[dir * DIMENSION + 0] * grid->getDelta();
        y += grid->getDirection()[dir * DIMENSION + 1] * grid->getDelta();
        z += grid->getDirection()[dir * DIMENSION + 2] * grid->getDelta();

        uint neighborIndex = grid->transCoordToIndex( x, y, z );
                
        if( grid->getFieldEntry(neighborIndex) == vf::gpu::STOPPER_OUT_OF_GRID_BOUNDARY ||
            grid->getFieldEntry(neighborIndex) == vf::gpu::STOPPER_OUT_OF_GRID ||
            grid->getFieldEntry(neighborIndex) == vf::gpu::STOPPER_SOLID )
            qNode[dir] = 0.5;
        else
            qNode[dir] = -1.0;

    }

    boundaryCondition->qs.push_back(qNode);
}

uint Side::getIndex(SPtr<Grid> grid, std::string coord, real constant, real v1, real v2)
{
    if (coord == "x")
        return grid->transCoordToIndex(constant, v1, v2);
    if (coord == "y")
        return grid->transCoordToIndex(v1, constant, v2);
    if (coord == "z")
        return grid->transCoordToIndex(v1, v2, constant);
    return INVALID_INDEX;
}


void MX::addIndices(std::vector<SPtr<Grid> > grid, uint level, SPtr<BoundaryCondition> boundaryCondition)
{
    real startInner = grid[level]->getStartY();
    real endInner = grid[level]->getEndY();

    real startOuter = grid[level]->getStartZ();
    real endOuter = grid[level]->getEndZ();

    real coordinateNormal = grid[level]->getStartX() + grid[level]->getDelta();

    if( coordinateNormal > grid[0]->getStartX() + grid[0]->getDelta() ) return;

    Side::addIndices(grid[level], boundaryCondition, "x", coordinateNormal, startInner, endInner, startOuter, endOuter);

}

void PX::addIndices(std::vector<SPtr<Grid> > grid, uint level, SPtr<BoundaryCondition> boundaryCondition)
{
    real startInner = grid[level]->getStartY();
    real endInner = grid[level]->getEndY();

    real startOuter = grid[level]->getStartZ();
    real endOuter = grid[level]->getEndZ();

    real coordinateNormal = grid[level]->getEndX() - grid[level]->getDelta();

    if( coordinateNormal < grid[0]->getEndX() - grid[0]->getDelta() ) return;

    Side::addIndices(grid[level], boundaryCondition, "x", coordinateNormal, startInner, endInner, startOuter, endOuter);
}

void MY::addIndices(std::vector<SPtr<Grid> > grid, uint level, SPtr<BoundaryCondition> boundaryCondition)
{
    real startInner = grid[level]->getStartX();
    real endInner = grid[level]->getEndX();

    real startOuter = grid[level]->getStartZ();
    real endOuter = grid[level]->getEndZ();

    real coordinateNormal = grid[level]->getStartY() + grid[level]->getDelta();

    if( coordinateNormal > grid[0]->getStartY() + grid[0]->getDelta() ) return;

    Side::addIndices(grid[level], boundaryCondition, "y", coordinateNormal, startInner, endInner, startOuter, endOuter);
}


void PY::addIndices(std::vector<SPtr<Grid> > grid, uint level, SPtr<BoundaryCondition> boundaryCondition)
{
    real startInner = grid[level]->getStartX();
    real endInner = grid[level]->getEndX();

    real startOuter = grid[level]->getStartZ();
    real endOuter = grid[level]->getEndZ();

    real coordinateNormal = grid[level]->getEndY() - grid[level]->getDelta();

    if( coordinateNormal < grid[0]->getEndY() - grid[0]->getDelta() ) return;

    Side::addIndices(grid[level], boundaryCondition, "y", coordinateNormal, startInner, endInner, startOuter, endOuter);
}


void MZ::addIndices(std::vector<SPtr<Grid> > grid, uint level, SPtr<BoundaryCondition> boundaryCondition)
{
    real startInner = grid[level]->getStartX();
    real endInner = grid[level]->getEndX();

    real startOuter = grid[level]->getStartY();
    real endOuter = grid[level]->getEndY();

    real coordinateNormal = grid[level]->getStartZ() + grid[level]->getDelta();

    if( coordinateNormal > grid[0]->getStartZ() + grid[0]->getDelta() ) return;

    Side::addIndices(grid[level], boundaryCondition, "z", coordinateNormal, startInner, endInner, startOuter, endOuter);
}

void PZ::addIndices(std::vector<SPtr<Grid> > grid, uint level, SPtr<BoundaryCondition> boundaryCondition)
{
    real startInner = grid[level]->getStartX();
    real endInner = grid[level]->getEndX();

    real startOuter = grid[level]->getStartY();
    real endOuter = grid[level]->getEndY();

    real coordinateNormal = grid[level]->getEndZ() - grid[level]->getDelta();

    if( coordinateNormal < grid[0]->getEndZ() - grid[0]->getDelta() ) return;
    
    Side::addIndices(grid[level], boundaryCondition, "z", coordinateNormal, startInner, endInner, startOuter, endOuter);
}

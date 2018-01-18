#ifndef GRID_STRATEGY_H
#define GRID_STRATEGY_H

#include "core/PointerDefinitions.h"

struct Vertex;
struct Geometry;
struct Grid;

class GridStrategy
{
public:
    virtual ~GridStrategy() {};

    virtual void allocateGridMemory(SPtr<Grid> grid) = 0;

    virtual void initalNodes(SPtr<Grid> grid) = 0;
    virtual void mesh(SPtr<Grid> grid, Geometry &geom) = 0;

    virtual void removeOverlapNodes(SPtr<Grid> grid, SPtr<Grid> finerGrid) = 0;

    virtual void deleteSolidNodes(SPtr<Grid> grid) = 0;

    virtual void freeMemory(SPtr<Grid> grid) = 0;

};

#endif

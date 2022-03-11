#ifndef GRID_INTERFACE_H
#define GRID_INTERFACE_H

#include "gpu/GridGenerator/global.h"

class GridImp;

class GridInterface
{
public:
    HOSTDEVICE GRIDGENERATOR_EXPORT GridInterface();
    HOSTDEVICE GRIDGENERATOR_EXPORT ~GridInterface();

    HOSTDEVICE void GRIDGENERATOR_EXPORT findInterfaceCF(const uint& indexOnCoarseGrid, GridImp* coarseGrid, GridImp* fineGrid);
    HOSTDEVICE void GRIDGENERATOR_EXPORT findBoundaryGridInterfaceCF(const uint& indexOnCoarseGrid, GridImp* coarseGrid, GridImp* fineGrid);


	HOSTDEVICE void GRIDGENERATOR_EXPORT findInterfaceCF_GKS(const uint& indexOnCoarseGrid, GridImp* coarseGrid, GridImp* fineGrid);

	HOSTDEVICE void GRIDGENERATOR_EXPORT findInterfaceFC(const uint& indexOnCoarseGrid, GridImp* coarseGrid, GridImp* fineGrid);
    HOSTDEVICE void GRIDGENERATOR_EXPORT findOverlapStopper(const uint& indexOnCoarseGrid, GridImp* coarseGrid, GridImp* fineGrid);
    
    HOSTDEVICE void GRIDGENERATOR_EXPORT findInvalidBoundaryNodes(const uint& indexOnCoarseGrid, GridImp* coarseGrid);

    HOSTDEVICE void GRIDGENERATOR_EXPORT findForGridInterfaceSparseIndexCF(GridImp* coarseGrid, GridImp* fineGrid, uint index);
    HOSTDEVICE void GRIDGENERATOR_EXPORT findForGridInterfaceSparseIndexFC(GridImp* coarseGrid, GridImp* fineGrid, uint index);

    CUDA_HOST void GRIDGENERATOR_EXPORT repairGridInterfaceOnMultiGPU(SPtr<GridImp> coarseGrid, SPtr<GridImp> fineGrid);

    HOSTDEVICE void GRIDGENERATOR_EXPORT print() const;

    struct Interface
    {
        uint *fine, *coarse;
        uint numberOfEntries = 0;
        uint *offset;
    } fc, cf;


private:
    HOSTDEVICE uint getCoarseToFineIndexOnFineGrid(const uint& indexOnCoarseGrid, const GridImp* coarseGrid, const GridImp* fineGrid);
    HOSTDEVICE bool isNeighborFineInvalid(real x, real y, real z, const GridImp* coarseGrid, const GridImp* fineGrid);

    HOSTDEVICE uint getFineToCoarseIndexOnFineGrid(const uint& indexOnCoarseGrid, const GridImp* coarseGrid, const GridImp* fineGrid);

    HOSTDEVICE static void findSparseIndex(uint* indices, GridImp* grid, uint index);

    HOSTDEVICE uint findOffsetCF( const uint& indexOnCoarseGrid, GridImp* coarseGrid, uint interfaceIndex );

    HOSTDEVICE uint findOffsetFC( const uint& indexOnCoarseGrid, GridImp* coarseGrid, uint interfaceIndex );
};


#endif
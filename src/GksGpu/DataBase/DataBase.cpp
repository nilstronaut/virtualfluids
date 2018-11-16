#include "DataBase.h"

#include <string>

#include "DataBaseAllocator.h"
#include "DataBaseStruct.h"

#include "GksMeshAdapter/GksMeshAdapter.h"

DataBase::DataBase( std::string type ) 
        : myAllocator    ( DataBaseAllocator::create( type ) ),
          numberOfNodes      (0),
          numberOfCells      (0),
          numberOfFaces      (0),
          numberOfLevels     (0),
          numberOfCoarseGhostCells(0),
          numberOfFineGhostCells(0),
          cellToCell     (nullptr),
          faceToCell     (nullptr),
          parentCell     (nullptr),
          faceCenter     (nullptr),
          cellCenter     (nullptr),
          faceIsWall     (nullptr),
          fineToCoarse   (nullptr),
          coarseToFine   (nullptr),
          data           (nullptr),
          dataUpdate     (nullptr),
          massFlux       (nullptr)
{
}

DataBase::~DataBase()
{
    this->myAllocator->freeMemory( shared_from_this() );
}

void DataBase::setMesh(GksMeshAdapter & adapter)
{
    this->numberOfNodes      = adapter.nodes.size();

    this->numberOfCells      = adapter.cells.size();

    this->numberOfFaces      = adapter.faces.size();

    this->numberOfLevels     = adapter.numberOfLevels;

    this->perLevelCount.resize( this->numberOfLevels );

    for( uint level = 0; level < this->numberOfLevels; level++ ){

        perLevelCount[ level ].numberOfBulkCells = adapter.numberOfBulkCellsPerLevel[ level ];
        perLevelCount[ level ].startOfBulkCells  = adapter.startOfCellsPerLevel     [ level ];

        perLevelCount[ level ].numberOfFacesX = adapter.numberOfFacesPerLevelXYZ[ 3 * level     ];
        perLevelCount[ level ].startOfFacesX  = adapter.startOfFacesPerLevelXYZ [ 3 * level     ];

        perLevelCount[ level ].numberOfFacesY = adapter.numberOfFacesPerLevelXYZ[ 3 * level + 1 ];
        perLevelCount[ level ].startOfFacesY  = adapter.startOfFacesPerLevelXYZ [ 3 * level + 1 ];

        perLevelCount[ level ].numberOfFacesZ = adapter.numberOfFacesPerLevelXYZ[ 3 * level + 2 ];
        perLevelCount[ level ].startOfFacesZ  = adapter.startOfFacesPerLevelXYZ [ 3 * level + 2 ];

        perLevelCount[ level ].numberOfFaces  = perLevelCount[ level ].numberOfFacesX
                                              + perLevelCount[ level ].numberOfFacesY
                                              + perLevelCount[ level ].numberOfFacesZ;

        perLevelCount[ level ].numberOfFineToCoarse = adapter.numberOfFineToCoarsePerLevel[ level ];
        perLevelCount[ level ].numberOfCoarseToFine = adapter.numberOfCoarseToFinePerLevel[ level ];

        perLevelCount[ level ].startOfFineToCoarse = adapter.startOfFineToCoarsePerLevel[ level ];
        perLevelCount[ level ].startOfCoarseToFine = adapter.startOfCoarseToFinePerLevel[ level ];
    }

    this->numberOfCoarseGhostCells = adapter.fineToCoarse.size();

    this->numberOfFineGhostCells   = adapter.coarseToFine.size();

    this->myAllocator->allocateMemory( shared_from_this() );

    this->myAllocator->copyMesh( shared_from_this(), adapter );
}

void DataBase::copyDataHostToDevice()
{
    this->myAllocator->copyDataHostToDevice( shared_from_this() );
}

void DataBase::copyDataDeviceToHost()
{
    this->myAllocator->copyDataDeviceToHost( shared_from_this(), this->dataHost.data() );
}

void DataBase::copyDataDeviceToHost( real* dataHost )
{
    this->myAllocator->copyDataDeviceToHost( shared_from_this(), dataHost );
}

DataBaseStruct DataBase::toStruct()
{
    DataBaseStruct dataBase;

    dataBase.numberOfCells            = this->numberOfCells;
    dataBase.numberOfFaces            = this->numberOfFaces;

    dataBase.numberOfCoarseGhostCells = this->numberOfCoarseGhostCells;
    dataBase.numberOfFineGhostCells   = this->numberOfFineGhostCells;

    dataBase.cellToCell               = this->cellToCell;
    dataBase.faceToCell               = this->faceToCell;

    dataBase.fineToCoarse             = this->fineToCoarse;
    dataBase.coarseToFine             = this->coarseToFine;

    dataBase.faceCenter               = this->faceCenter;
    dataBase.cellCenter               = this->cellCenter;

    dataBase.faceIsWall               = this->faceIsWall;

    dataBase.fineToCoarse             = this->fineToCoarse;
    dataBase.coarseToFine             = this->coarseToFine;

    dataBase.data                     = this->data;
    dataBase.dataUpdate               = this->dataUpdate;

    dataBase.massFlux                 = this->massFlux;

    return dataBase;
}

uint DataBase::getCellLevel(uint cellIdx)
{
    uint level = 0;

    while( cellIdx < this->perLevelCount[level].startOfBulkCells
                   + this->perLevelCount[level].numberOfBulkCells ) level++;

    return level;
}

uint DataBase::getFaceLevel(uint faceIdx)
{
    uint level = 0;

    while( faceIdx < this->perLevelCount[level].startOfFacesX
                   + this->perLevelCount[level].numberOfFaces ) level++;

    return level;
}

bool DataBase::isGhostCell(uint cellIdx)
{
    return cellIdx < this->perLevelCount[ this->getCellLevel( cellIdx ) ].numberOfBulkCells;
}

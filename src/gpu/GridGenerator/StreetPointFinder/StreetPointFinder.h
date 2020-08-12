#ifndef StreetPointFinder_H
#define StreetPointFinder_H

#include <vector>
#include <string>

#include "GridGenerator_export.h"

#include "Core/PointerDefinitions.h"
#include "Core/DataTypes.h"

#include <VirtualFluidsDefinitions.h>

class Grid;

struct GRIDGENERATOR_EXPORT Street
{
    // The start and end coordinates are stored for cell centers!
    //
    //     |---x---|---x---|---x---|---x---|---x---|---x---|---x---|---x---|---x---|---x---|
    //         |--->                       |<----->|                               <---|
    //         xStart                          dx                                   xEnd
    //
    // dx = (xStart - xEnd) / (numberOfCells - 1)

    uint numberOfCells;
    real xStart, yStart, xEnd, yEnd;

    std::vector<uint> matrixIndicesLB;
    std::vector<uint> sparseIndicesLB;

    // The constructor expect start and end for cells
    Street( real xStartCell, real yStartCell, real xEndCell, real yEndCell, real dx );

    real getCoordinateX( int cellIndex );
    real getCoordinateY( int cellIndex );

	real getVectorX();
	real getVectorY();

    void findIndicesLB( SPtr<Grid> grid, real initialSearchHeight);
};

struct GRIDGENERATOR_EXPORT StreetPointFinder
{
    std::vector<Street> streets;

    std::vector<uint> sparseIndicesLB;
    std::vector<uint> mapNashToConc;

    void prepareSimulationFileData();

    void readStreets(std::string filename);

    void findIndicesLB( SPtr<Grid> grid, real initialSearchHeight );

	void writeVTK(std::string filename, const std::vector<int>& cars = std::vector<int>());

	void writeReducedVTK(std::string filename, const std::vector<int>& cars = std::vector<int>());

	void prepareWriteVTK(std::ofstream& file, uint & numberOfCells);

	void writeCarsVTK(std::ofstream& file, uint numberOfCells, const std::vector<int>& cars);

	void writeLengthsVTK(std::ofstream& file, uint numberOfCells);

	void writeStreetsVTK(std::ofstream& file, uint numberOfCells);

    void writeConnectionVTK(std::string filename, SPtr<Grid> grid);

	void writeSimulationFile(std::string gridPath, real concentration, uint numberOfLevels, uint level);

	void writeStreetVectorFile(std::string gridPath, real concentration, uint numberOfLevels, uint level);

    void writeSimulationFileSorted( std::string gridPath, real concentration, uint numberOfLevels, uint level );

    void writeMappingFile( std::string gridPath );

	//////////////////////////////////////////////////////////////////////////
	// 3D cars writer hacked by Stephan L.

	void write3DVTK(std::string filename, const std::vector<int>& cars = std::vector<int>());

	void prepareWrite3DVTK(std::ofstream& file, uint & numberOfCells, const std::vector<int>& cars);
};


#endif
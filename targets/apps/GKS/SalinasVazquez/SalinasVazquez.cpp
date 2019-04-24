//#define MPI_LOGGING

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <iostream>
#include <exception>
#include <fstream>
#include <memory>
#include <thread>
#include <sstream>

#include "Core/Timer/Timer.h"
#include "Core/PointerDefinitions.h"
#include "Core/DataTypes.h"
#include "Core/VectorTypes.h"
#include "Core/Logger/Logger.h"

#include "GridGenerator/geometries/Cuboid/Cuboid.h"
#include "GridGenerator/geometries/Conglomerate/Conglomerate.h"

#include "GridGenerator/grid/GridBuilder/LevelGridBuilder.h"
#include "GridGenerator/grid/GridBuilder/MultipleGridBuilder.h"
#include "GridGenerator/grid/GridFactory.h"
#include "GridGenerator/geometries/BoundingBox/BoundingBox.h"
#include "GridGenerator/utilities/communication.h"

#include "GksMeshAdapter/GksMeshAdapter.h"

#include "GksVtkAdapter/VTKInterface.h"

#include "GksGpu/DataBase/DataBase.h"
#include "GksGpu/Parameters/Parameters.h"
#include "GksGpu/Initializer/Initializer.h"

#include "GksGpu/FlowStateData/FlowStateDataConversion.cuh"

#include "GksGpu/BoundaryConditions/BoundaryCondition.h"
#include "GksGpu/BoundaryConditions/IsothermalWall.h"
#include "GksGpu/BoundaryConditions/Periodic.h"
#include "GksGpu/BoundaryConditions/Pressure.h"
#include "GksGpu/BoundaryConditions/AdiabaticWall.h"
#include "GksGpu/BoundaryConditions/SalinasVazquez.h"

#include "GksGpu/Communication/Communicator.h"
#include "GksGpu/Communication/MpiUtility.h"

#include "GksGpu/TimeStepping/NestedTimeStep.h"

#include "GksGpu/Analyzer/CupsAnalyzer.h"
#include "GksGpu/Analyzer/ConvergenceAnalyzer.h"
#include "GksGpu/Analyzer/TurbulenceAnalyzer.h"

#include "GksGpu/CudaUtility/CudaUtility.h"

//uint deviceMap [2] = {2,3};
uint deviceMap [2] = {0,1};

void simulation( std::string path, std::string simulationName )
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int mpiWorldSize = 1;
    MPI_Comm_size(MPI_COMM_WORLD, &mpiWorldSize);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    uint nx = 128;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    real L = 1.0;
    real H = 0.25;

    real dx = L / real(nx);

    real Ra = 1.58e9;

    real Ba  = 0.1;
    real eps = 0.132;
    real Pr  = 0.71;
    real K   = 2.0;
    
    real g   = 1.0;
    real rho = 1.0;

    real lambda     = Ba / ( 2.0 * g * L );
    real lambdaHot  = lambda / ( 1.0 + eps * 0.5 );
    real lambdaCold = lambda / ( 1.0 - eps * 0.5 );
    
    real mu = sqrt( Pr * eps * g * L * L * L / Ra ) * rho ;

    real cs  = sqrt( ( ( K + 4.0 ) / ( K + 2.0 ) ) / ( 2.0 * lambda ) );
    real U   = sqrt( Ra ) * mu / ( rho * L );

    real CFL = 0.5;

    real dt  = CFL * ( dx / ( ( U + cs ) * ( one + ( two * mu ) / ( U * dx * rho ) ) ) );

    *logging::out << logging::Logger::INFO_HIGH << "dt = " << dt << " s\n";
    *logging::out << logging::Logger::INFO_HIGH << "U  = " << U  << " s\n";
    *logging::out << logging::Logger::INFO_HIGH << "mu = " << mu << " s\n";

    //////////////////////////////////////////////////////////////////////////

    Parameters parameters;

    parameters.K  = K;
    parameters.Pr = Pr;
    parameters.mu = mu;

    parameters.force.x = 0;
    parameters.force.y = 0;
    parameters.force.z = -g;

    parameters.dt = dt;
    parameters.dx = dx;

    parameters.lambdaRef = lambda;

    parameters.viscosityModel = ViscosityModel::sutherlandsLaw;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                M e s h    G e n e r a t i o n
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    auto gridFactory = GridFactory::make();
    gridFactory->setGridStrategy(Device::CPU);
    gridFactory->setTriangularMeshDiscretizationMethod(TriangularMeshDiscretizationMethod::POINT_IN_OBJECT);

    auto gridBuilder = MultipleGridBuilder::makeShared(gridFactory);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    real startX, endX;
    real startY, endY;
    real startZ, endZ;

    if( rank % 2 == 0 ) startX = -0.5 * L;
    else                startX = -3.0 * dx;
    if( rank % 2 == 0 ) endX   =  3.0 * dx;
    else                endX   =  0.5 * L;

    startY =   rank/2         * H - 3.0 * dx;
    endY   = ( rank/2 + 1.0 ) * H + 3.0 * dx;

    startZ = -0.5 * L;
    endZ   =  0.5 * L;

    gridBuilder->addCoarseGrid(startX, startY, startZ,  
                               endX  , endY  , endZ  , dx);

    //////////////////////////////////////////////////////////////////////////

    real refL[4] = { 0.35, 0.45, 0.475, 0.495 };

    gridBuilder->setNumberOfLayers(6,6);

    uint numberOfRefinements = 3;

    for( uint ref = 0; ref < numberOfRefinements; ref++ )
    {
        Cuboid* refRegion;

        if( rank % 2 == 0 ) refRegion = new Cuboid (-100.0,     -100.0, -100.0, 
                                                    -refL[ref],  100.0,  100.0 );
        else                refRegion = new Cuboid ( refL[ref], -100.0, -100.0, 
                                                     100.0,      100.0,  100.0 );

        gridBuilder->addGrid( refRegion, ref + 1);
    }

    //////////////////////////////////////////////////////////////////////////

    if( rank % 2 == 0 ) startX = -100.0;
    else                startX =    0.0;
    if( rank % 2 == 0 ) endX   =    0.0;
    else                endX   =  100.0;

    startY =   rank/2         * H;
    endY   = ( rank/2 + 1.0 ) * H;

    startZ = -100.0;
    endZ   =  100.0;

    gridBuilder->setSubDomainBox( std::make_shared<BoundingBox>( startX, endX, 
                                                                 startY, endY, 
                                                                 startZ, endZ ) );

    //////////////////////////////////////////////////////////////////////////

    gridBuilder->setPeriodicBoundaryCondition(false, false, false);

    gridBuilder->buildGrids(GKS, false);
            
    //////////////////////////////////////////////////////////////////////////

    if( rank%2 == 0 ) gridBuilder->findCommunicationIndices( CommunicationDirections::PX, GKS );
    else              gridBuilder->findCommunicationIndices( CommunicationDirections::MX, GKS );

    if( rank%2 == 0 ) gridBuilder->setCommunicationProcess ( CommunicationDirections::PX, rank + 1 );
    else              gridBuilder->setCommunicationProcess ( CommunicationDirections::MX, rank - 1 );

    //////////////////////////////////////////////////////////////////////////
    
    if( mpiWorldSize > 2 )
    {
        gridBuilder->findCommunicationIndices(CommunicationDirections::PY, GKS);
        gridBuilder->findCommunicationIndices(CommunicationDirections::MY, GKS);

        gridBuilder->setCommunicationProcess(CommunicationDirections::PY, (rank + 2 + mpiWorldSize) % mpiWorldSize);
        gridBuilder->setCommunicationProcess(CommunicationDirections::MY, (rank - 2 + mpiWorldSize) % mpiWorldSize);
    }

    //gridBuilder->writeGridsToVtk(path + "/Grid_rank_" + std::to_string(rank) + "_lev_");     

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    GksMeshAdapter meshAdapter( gridBuilder );

    meshAdapter.inputGrid();

    meshAdapter.findPeriodicBoundaryNeighbors();    

    meshAdapter.getCommunicationIndices();

    //meshAdapter.writeMeshFaceVTK( path + "grid/MeshFaces_" + std::to_string( threadIndex ) + ".vtk" );

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    auto dataBase = std::make_shared<DataBase>( "GPU" );

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                 B o u n d a r y    C o n d i t i o n s
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    SPtr<BoundaryCondition> bcMX = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaHot , false );
    SPtr<BoundaryCondition> bcPX = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaCold, false );

    bcMX->findBoundaryCells( meshAdapter, true, [&](Vec3 center){ return center.x < -0.5*L; } );
    bcPX->findBoundaryCells( meshAdapter, true, [&](Vec3 center){ return center.x >  0.5*L; } );

    //////////////////////////////////////////////////////////////////////////

    SPtr<BoundaryCondition> bcMZ = std::make_shared<SalinasVazquez>( dataBase, lambdaHot, lambdaCold, 0.3371, -0.2642,  0.5301, -2.6438, true );
    SPtr<BoundaryCondition> bcPZ = std::make_shared<SalinasVazquez>( dataBase, lambdaHot, lambdaCold, 0.6559, -0.2037, -0.5420, -2.7318, true );

    bcMZ->findBoundaryCells( meshAdapter, true, [&](Vec3 center){ return center.z < -0.5*L; } );
    bcPZ->findBoundaryCells( meshAdapter, true, [&](Vec3 center){ return center.z >  0.5*L; } );

    //////////////////////////////////////////////////////////////////////////

    if( mpiWorldSize == 2 )
    {
        SPtr<BoundaryCondition> bcMY = std::make_shared<Periodic>(dataBase);
        SPtr<BoundaryCondition> bcPY = std::make_shared<Periodic>(dataBase);

        bcMX->findBoundaryCells(meshAdapter, true, [&](Vec3 center) { return center.y < startY; });
        bcPX->findBoundaryCells(meshAdapter, true, [&](Vec3 center) { return center.y > endY; });

        dataBase->boundaryConditions.push_back(bcMY);
        dataBase->boundaryConditions.push_back(bcPY);
    }

    //////////////////////////////////////////////////////////////////////////

    dataBase->boundaryConditions.push_back( bcMZ );
    dataBase->boundaryConditions.push_back( bcPZ );

    dataBase->boundaryConditions.push_back( bcMX );
    dataBase->boundaryConditions.push_back( bcPX );

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                 I n i t i a l    C o n d i t i o n s
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    dataBase->setMesh( meshAdapter );

    dataBase->setCommunicators( meshAdapter );

    CudaUtility::printCudaMemoryUsage();

    Initializer::interpret(dataBase, [&] ( Vec3 cellCenter ) -> ConservedVariables{

        real Th = 1.0 / lambdaHot;
        real Tc = 1.0 / lambdaCold;
        real T = Th - (Th - Tc)*( (cellCenter.x + 0.5 * L) / L);
        real lambdaLocal = 1.0 / T;

        return toConservedVariables( PrimitiveVariables( rho, 0.0, 0.0, 0.0, lambda ), parameters.K );
    });

    dataBase->copyDataHostToDevice();

    Initializer::initializeDataUpdate(dataBase);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                  R u n
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    if( rank == 0 ) writeVtkXMLParallelSummaryFile( dataBase, parameters, path + simulationName + "_0", mpiWorldSize );

    writeVtkXML( dataBase, parameters, 0, path + simulationName + "_0" + "_rank_" + std::to_string(rank) );

    CupsAnalyzer cupsAnalyzer( dataBase, true, 300.0 );

    ConvergenceAnalyzer convergenceAnalyzer( dataBase );

    auto turbulenceAnalyzer = std::make_shared<TurbulenceAnalyzer>( dataBase, 200000 );
    //auto turbulenceAnalyzer = std::make_shared<TurbulenceAnalyzer>( dataBase, 200 );

    //////////////////////////////////////////////////////////////////////////

    cupsAnalyzer.start();

    for( uint iter = 1; iter <= 10000000; iter++ )
    {
        TimeStepping::nestedTimeStep(dataBase, parameters, 0);

        if( 
            //( iter < 10     && iter % 1     == 0 ) ||
            //( iter < 100    && iter % 10    == 0 ) ||
            //( iter < 1000   && iter % 100   == 0 ) ||
            //( iter < 10000  && iter % 1000  == 0 ) 
            ( iter < 10000000 && iter % 50000 == 0 )
          )
        {
            dataBase->copyDataDeviceToHost();

            if( rank == 0 ) writeVtkXMLParallelSummaryFile( dataBase, parameters, path + simulationName + "_" + std::to_string( iter ), mpiWorldSize );

            writeVtkXML( dataBase, parameters, 0, path + simulationName + "_" + std::to_string( iter ) + "_rank_" + std::to_string(rank) );
        }

        cupsAnalyzer.run( iter );

        convergenceAnalyzer.run( iter );

        turbulenceAnalyzer->run( iter, parameters );

        if( iter > 200000 && iter % 100000 == 0 )
        {
            turbulenceAnalyzer->download();

            if( rank == 0 ) writeTurbulenceVtkXMLParallelSummaryFile( dataBase, turbulenceAnalyzer, parameters, path + simulationName + "_Turbulence_" + std::to_string( iter ), mpiWorldSize );

            writeTurbulenceVtkXML( dataBase, turbulenceAnalyzer, 0, path + simulationName + "_Turbulence_" + std::to_string( iter ) + "_rank_" + std::to_string(rank) );
        }
    }

    //////////////////////////////////////////////////////////////////////////

    dataBase->copyDataDeviceToHost();
}

int main( int argc, char* argv[])
{
    //////////////////////////////////////////////////////////////////////////

    int rank         = MpiUtility::getMpiRankBeforeInit();
    int mpiWorldSize = MpiUtility::getMpiWorldSizeBeforeInit();

    if( mpiWorldSize < 2 || mpiWorldSize%2 != 0 )
    {
        *logging::out << logging::Logger::INFO_HIGH << "Error: MpiWolrdSize must be multiple of 2!\n";
        return 1;
    }

    //////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
    std::string path( "F:/Work/Computations/out/SalinazVasques/" );
#else
    std::string path( "out/" );
#endif

    std::string simulationName ( "SalinasVazquez" );

    //////////////////////////////////////////////////////////////////////////

    logging::Logger::addStream(&std::cout);
    
    std::ofstream logFile( path + simulationName + "_rank_" + std::to_string(rank) + ".log" );
    logging::Logger::addStream(&logFile);

    logging::Logger::setDebugLevel(logging::Logger::Level::INFO_LOW);
    logging::Logger::timeStamp(logging::Logger::ENABLE);

    //////////////////////////////////////////////////////////////////////////

    // Important: for Cuda-Aware MPI the device must be set before MPI_Init()
    int deviceCount = CudaUtility::getCudaDeviceCount();

    if(deviceCount == 0)
    {
        std::stringstream msg;
        msg << "No devices devices found!" << std::endl;
        *logging::out << logging::Logger::WARNING << msg.str(); msg.str("");
    }

    CudaUtility::setCudaDevice( rank % deviceCount );

    //////////////////////////////////////////////////////////////////////////

    MPI_Init(&argc, &argv);

    //////////////////////////////////////////////////////////////////////////

    if( sizeof(real) == 4 )
        *logging::out << logging::Logger::INFO_HIGH << "Using Single Precison\n";
    else
        *logging::out << logging::Logger::INFO_HIGH << "Using Double Precision\n";

    try
    {
        simulation(path, simulationName);
    }
    catch (const std::exception& e)
    {     
        *logging::out << logging::Logger::ERROR << e.what() << "\n";
    }
    catch (const std::bad_alloc& e)
    {  
        *logging::out << logging::Logger::ERROR << "Bad Alloc:" << e.what() << "\n";
    }
    catch (...)
    {
        *logging::out << logging::Logger::ERROR << "Unknown exception!\n";
    }

    logFile.close();

    MPI_Finalize();

    return 0;
}

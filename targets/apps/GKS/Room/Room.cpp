//#define MPI_LOGGING

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <iostream>
#include <exception>
#include <fstream>
#include <memory>

#include "Core/Timer/Timer.h"
#include "Core/PointerDefinitions.h"
#include "Core/DataTypes.h"
#include "Core/VectorTypes.h"
#include "Core/Logger/Logger.h"

#include "GridGenerator/geometries/Cuboid/Cuboid.h"
#include "GridGenerator/geometries/Sphere/Sphere.h"
#include "GridGenerator/geometries/VerticalCylinder/VerticalCylinder.h"
#include "GridGenerator/geometries/Conglomerate/Conglomerate.h"

#include "GridGenerator/grid/GridBuilder/LevelGridBuilder.h"
#include "GridGenerator/grid/GridBuilder/MultipleGridBuilder.h"
#include "GridGenerator/grid/GridFactory.h"

#include "GksMeshAdapter/GksMeshAdapter.h"

#include "GksVtkAdapter/VTKInterface.h"

#include "GksGpu/DataBase/DataBase.h"
#include "GksGpu/Parameters/Parameters.h"
#include "GksGpu/Initializer/Initializer.h"

#include "GksGpu/BoundaryConditions/BoundaryCondition.h"
#include "GksGpu/BoundaryConditions/IsothermalWall.h"
#include "GksGpu/BoundaryConditions/Periodic.h"
#include "GksGpu/BoundaryConditions/Pressure.h"
#include "GksGpu/BoundaryConditions/AdiabaticWall.h"

#include "GksGpu/TimeStepping/NestedTimeStep.h"

#include "GksGpu/Analyzer/CupsAnalyzer.h"
#include "GksGpu/Analyzer/ConvergenceAnalyzer.h"
#include "GksGpu/Analyzer/TurbulenceAnalyzer.h"

#include "GksGpu/CudaUtility/CudaUtility.h"

void thermalCavity( std::string path, std::string simulationName )
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    uint nx = 128;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    real L = 4.0;
    real H = 3.0;

    real dx = H / real(nx);


    real Ra = 1.0e9;

    real Ba  = 0.1;
    real eps = 1.2;
    real Pr  = 0.71;
    real K   = 2.0;
    
    real g   = 9.81;
    real rho = 1.2;

    real lambda     = Ba / ( 2.0 * g * H );
    real lambdaHot  = lambda / ( 1.0 + eps * 0.5 );
    real lambdaCold = lambda / ( 1.0 - eps * 0.5 );
    
    real mu = sqrt( Pr * eps * g * H * H * H / Ra ) * rho;

    real cs  = sqrt( ( ( K + 5.0 ) / ( K + 3.0 ) ) / ( 2.0 * lambda ) );
    real U   = sqrt( Ra ) * mu / ( rho * L );

    real CFL = 0.25;

    real dt  = CFL * ( dx / ( ( U + cs ) * ( one + ( two * mu ) / ( U * dx * rho ) ) ) );

    *logging::out << logging::Logger::INFO_HIGH << "dt = " << dt << " s\n";
    *logging::out << logging::Logger::INFO_HIGH << "U  = " << U  << " m/s\n";
    *logging::out << logging::Logger::INFO_HIGH << "mu = " << mu << " kg/sm\n";

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

    auto gridFactory = GridFactory::make();
    gridFactory->setGridStrategy(Device::CPU);
    gridFactory->setTriangularMeshDiscretizationMethod(TriangularMeshDiscretizationMethod::POINT_IN_OBJECT);

    auto gridBuilder = MultipleGridBuilder::makeShared(gridFactory);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //gridBuilder->addCoarseGrid(-0.5*L, -0.5*L, -0.5*H,  
    //                            0.5*L,  0.5*L,  0.5*H, dx);

    gridBuilder->addCoarseGrid(-0.5*L, -0.5*L,  0.0,  
                                0.5*L,  0.5*L,  H  , dx);

    Sphere           sphere  ( 0.0, 0.0, 0.0, 0.6 );
    VerticalCylinder cylinder( 0.0, 0.0, 0.0, 0.6, 2.0*H );

    //gridBuilder->addGrid( &refRegion_1, 1);
    //gridBuilder->addGrid( &refRegion_2, 2);
    //gridBuilder->addGrid( &refRegion_3, 3);
    //gridBuilder->addGrid( &refRegion_4, 4);

    gridBuilder->setNumberOfLayers(0,10);

    gridBuilder->addGrid( &sphere, 2 );
    //gridBuilder->addGrid( &cylinder, 2 );

    gridBuilder->setPeriodicBoundaryCondition(false, false, false);

    gridBuilder->buildGrids(GKS, false);

    //gridBuilder->writeGridsToVtk(path + "grid/Grid_lev_");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    GksMeshAdapter meshAdapter( gridBuilder );

    meshAdapter.inputGrid();

    //meshAdapter.writeMeshVTK( path + "grid/Mesh.vtk" );

    //meshAdapter.writeMeshFaceVTK( path + "grid/MeshFaces.vtk" );

    //meshAdapter.findPeriodicBoundaryNeighbors();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    CudaUtility::setCudaDevice(0);

    auto dataBase = std::make_shared<DataBase>( "GPU" );

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    SPtr<BoundaryCondition> bcMX = std::make_shared<AdiabaticWall>( dataBase, Vec3(0.0, 0.0, 0.0), false );
    SPtr<BoundaryCondition> bcPX = std::make_shared<AdiabaticWall>( dataBase, Vec3(0.0, 0.0, 0.0), false );
    //SPtr<BoundaryCondition> bcMX = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaCold,  0.0, false );
    //SPtr<BoundaryCondition> bcPX = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaCold,  0.0, false );

    bcMX->findBoundaryCells( meshAdapter, false, [&](Vec3 center){ return center.x < -0.5*L; } );
    bcPX->findBoundaryCells( meshAdapter, false, [&](Vec3 center){ return center.x >  0.5*L; } );

    //////////////////////////////////////////////////////////////////////////

    SPtr<BoundaryCondition> bcMY = std::make_shared<AdiabaticWall>( dataBase, Vec3(0.0, 0.0, 0.0), false );
    SPtr<BoundaryCondition> bcPY = std::make_shared<AdiabaticWall>( dataBase, Vec3(0.0, 0.0, 0.0), false );
    //SPtr<BoundaryCondition> bcMY = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaCold,  0.0, false );
    //SPtr<BoundaryCondition> bcPY = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaCold,  0.0, false );

    bcMY->findBoundaryCells( meshAdapter, false, [&](Vec3 center){ return center.y < -0.5*L; } );
    bcPY->findBoundaryCells( meshAdapter, false, [&](Vec3 center){ return center.y >  0.5*L; } );

    //////////////////////////////////////////////////////////////////////////
    
    SPtr<BoundaryCondition> bcMZ = std::make_shared<AdiabaticWall>( dataBase, Vec3(0.0, 0.0, 0.0), true );
    SPtr<BoundaryCondition> bcPZ = std::make_shared<AdiabaticWall>( dataBase, Vec3(0.0, 0.0, 0.0), true );
    //SPtr<BoundaryCondition> bcMZ = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaCold,  0.0, true );
    //SPtr<BoundaryCondition> bcPZ = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaCold,  0.0, true );
    
    bcMZ->findBoundaryCells( meshAdapter, true, [&](Vec3 center){ return center.z < 0.0; } );
    bcPZ->findBoundaryCells( meshAdapter, true, [&](Vec3 center){ return center.z > H  ; } );

    //////////////////////////////////////////////////////////////////////////

    SPtr<BoundaryCondition> hotPlate = std::make_shared<IsothermalWall>( dataBase, Vec3(0.0, 0.0, 0.0), lambdaHot, true );

    hotPlate->findBoundaryCells( meshAdapter, false, [&](Vec3 center){ 
        //return center.z < 0.0 && 
        //       std::fabs(center.x) < 0.5 && 
        //       std::fabs(center.y) < 0.5; 

        return center.z < 0.0 && std::sqrt(center.x*center.x + center.y*center.y) < 0.5;
    } );

    //////////////////////////////////////////////////////////////////////////

    dataBase->boundaryConditions.push_back( bcMX );
    dataBase->boundaryConditions.push_back( bcPX );
    
    dataBase->boundaryConditions.push_back( bcMY );
    dataBase->boundaryConditions.push_back( bcPY );

    dataBase->boundaryConditions.push_back( bcMZ );
    dataBase->boundaryConditions.push_back( bcPZ );

    dataBase->boundaryConditions.push_back( hotPlate );

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    dataBase->setMesh( meshAdapter );

    CudaUtility::printCudaMemoryUsage();

    Initializer::interpret(dataBase, [&] ( Vec3 cellCenter ) -> ConservedVariables{

        real rhoLocal = rho * std::exp( - ( 2.0 * g * H * lambdaCold ) * cellCenter.z / H );

        return toConservedVariables( PrimitiveVariables( rhoLocal, 0.0, 0.0, 0.0, lambdaCold ), parameters.K );
    });

    dataBase->copyDataHostToDevice();

    for( auto bc : dataBase->boundaryConditions ) 
        for( uint level = 0; level < dataBase->numberOfLevels; level++ )
            bc->runBoundaryConditionKernel( dataBase, parameters, 0 );

    Initializer::initializeDataUpdate(dataBase);

    dataBase->copyDataDeviceToHost();

    writeVtkXML( dataBase, parameters, 0, path + simulationName + "_0" );

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    CupsAnalyzer cupsAnalyzer( dataBase, true, 30.0 );

    ConvergenceAnalyzer convergenceAnalyzer( dataBase );

    //auto turbulenceAnalyzer = std::make_shared<TurbulenceAnalyzer>( dataBase, 50000 );

    //////////////////////////////////////////////////////////////////////////

    cupsAnalyzer.start();

    for( uint iter = 1; iter <= 100000000; iter++ )
    {
        if( iter < 20000 )
        {
            std::dynamic_pointer_cast<IsothermalWall>(hotPlate)->lambda = lambdaCold + ( lambdaHot - lambdaCold ) * ( real(iter) / 20000.0 );
        }
        else
        {
            std::dynamic_pointer_cast<IsothermalWall>(hotPlate)->lambda = lambdaHot;
        }

        TimeStepping::nestedTimeStep(dataBase, parameters, 0);

        if( 
            //( iter < 10     && iter % 1     == 0 ) ||
            //( iter < 100    && iter % 10    == 0 ) ||
            //( iter < 1000   && iter % 100   == 0 ) ||
            //( iter < 10000  && iter % 1000  == 0 ) ||
            //( iter < 10000000 && iter % 100000 == 0 )
            ( iter >= 10000 && iter % 100000 == 0 )
          )
        {
            dataBase->copyDataDeviceToHost();

            writeVtkXML( dataBase, parameters, 0, path + simulationName + "_" + std::to_string( iter ) );
        }

        cupsAnalyzer.run( iter );

        convergenceAnalyzer.run( iter );

        //turbulenceAnalyzer->run( iter, parameters );
    }

    //////////////////////////////////////////////////////////////////////////

    dataBase->copyDataDeviceToHost();

    //writeVtkXML( dataBase, parameters, 0, path + "grid/Test_1" );

    //turbulenceAnalyzer->download();

    //writeTurbulenceVtkXML(dataBase, turbulenceAnalyzer, 0, path + simulationName + "_Turbulence");
}

int main( int argc, char* argv[])
{
    //std::string path( "F:/Work/Computations/out/" );
    std::string path( "out/" );
    std::string simulationName ( "Room" );

    logging::Logger::addStream(&std::cout);
    logging::Logger::setDebugLevel(logging::Logger::Level::INFO_LOW);
    logging::Logger::timeStamp(logging::Logger::ENABLE);

    if( sizeof(real) == 4 )
        *logging::out << logging::Logger::INFO_HIGH << "Using Single Precison\n";
    else
        *logging::out << logging::Logger::INFO_HIGH << "Using Double Precision\n";

    try
    {
        thermalCavity( path, simulationName );
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

   return 0;
}

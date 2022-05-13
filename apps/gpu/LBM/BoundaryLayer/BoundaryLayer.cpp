
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <exception>
#include <memory>

//////////////////////////////////////////////////////////////////////////

#include "Core/DataTypes.h"
#include "PointerDefinitions.h"

#include "Core/StringUtilities/StringUtil.h"

#include "Core/VectorTypes.h"

#include <basics/config/ConfigurationFile.h>

#include <logger/Logger.h>


//////////////////////////////////////////////////////////////////////////

#include "GridGenerator/grid/GridBuilder/LevelGridBuilder.h"
#include "GridGenerator/grid/GridBuilder/MultipleGridBuilder.h"
#include "GridGenerator/grid/BoundaryConditions/Side.h"
#include "GridGenerator/grid/GridFactory.h"

#include "GridGenerator/io/SimulationFileWriter/SimulationFileWriter.h"
#include "GridGenerator/io/GridVTKWriter/GridVTKWriter.h"
#include "GridGenerator/io/STLReaderWriter/STLReader.h"
#include "GridGenerator/io/STLReaderWriter/STLWriter.h"

//////////////////////////////////////////////////////////////////////////

#include "VirtualFluids_GPU/LBM/Simulation.h"
#include "VirtualFluids_GPU/Communication/Communicator.h"
#include "VirtualFluids_GPU/DataStructureInitializer/GridReaderGenerator/GridGenerator.h"
#include "VirtualFluids_GPU/DataStructureInitializer/GridProvider.h"
#include "VirtualFluids_GPU/DataStructureInitializer/GridReaderFiles/GridReader.h"
#include "VirtualFluids_GPU/Parameter/Parameter.h"
#include "VirtualFluids_GPU/Output/FileWriter.h"
#include "VirtualFluids_GPU/PreCollisionInteractor/ActuatorLine.h"
#include "VirtualFluids_GPU/PreCollisionInteractor/Probes/PointProbe.h"
#include "VirtualFluids_GPU/PreCollisionInteractor/Probes/PlaneProbe.h"
#include "VirtualFluids_GPU/PreCollisionInteractor/Probes/PlanarAverageProbe.h"
#include "VirtualFluids_GPU/PreCollisionInteractor/Probes/WallModelProbe.h"

#include "VirtualFluids_GPU/Kernel/Utilities/KernelFactory/KernelFactoryImp.h"
#include "VirtualFluids_GPU/PreProcessor/PreProcessorFactory/PreProcessorFactoryImp.h"

#include "VirtualFluids_GPU/GPU/CudaMemoryManager.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string path(".");

std::string simulationName("BoundayLayer");

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void multipleLevel(const std::string& configPath)
{

    logging::Logger::addStream(&std::cout);
    logging::Logger::setDebugLevel(logging::Logger::Level::INFO_LOW);
    logging::Logger::timeStamp(logging::Logger::ENABLE);
    logging::Logger::enablePrintedRankNumbers(logging::Logger::ENABLE);
    
    auto gridFactory = GridFactory::make();
    auto gridBuilder = MultipleGridBuilder::makeShared(gridFactory);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    vf::gpu::Communicator& communicator = vf::gpu::Communicator::getInstance();

    vf::basics::ConfigurationFile config;
    config.load(configPath);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////^
    SPtr<Parameter> para = std::make_shared<Parameter>(config, communicator.getNummberOfProcess(), communicator.getPID());

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //          U s e r    s e t t i n g s
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    LbmOrGks lbmOrGks = LBM;

    const real H = 1000.0; // boundary layer height in m

    const real L_x = 6*H;
    const real L_y = 4*H;
    const real L_z = 1*H;

    const real z0  = 0.1; // roughness length in m
    const real u_star = 0.4; //friction velocity in m/s
    const real kappa = 0.4; // von Karman constant 

    const real viscosity = 1.56e-5;

    const real velocity  = u_star/kappa*log(L_z/z0); //max mean velocity at the top in m/s

    const real mach = config.contains("Ma")? config.getValue<real>("Ma"): 0.1;

    const uint nodes_per_H = config.contains("nz")? config.getValue<uint>("nz"): 64;

    // all in s
    const float tStartOut   = config.getValue<real>("tStartOut");
    const float tOut        = config.getValue<real>("tOut");
    const float tEnd        = config.getValue<real>("tEnd"); // total time of simulation

    const float tStartAveraging     =  config.getValue<real>("tStartAveraging");
    const float tStartTmpAveraging  =  config.getValue<real>("tStartTmpAveraging");
    const float tAveraging          =  config.getValue<real>("tAveraging");
    const float tStartOutProbe      =  config.getValue<real>("tStartOutProbe");
    const float tOutProbe           =  config.getValue<real>("tOutProbe"); 


    const real dx = L_z/real(nodes_per_H);

    const real dt = dx * mach / (sqrt(3) * velocity);

    const real velocityLB = velocity * dt / dx; // LB units

    const real viscosityLB = viscosity * dt / (dx * dx); // LB units

    const real pressureGradient = u_star * u_star / H ;
    const real pressureGradientLB = u_star * u_star / H / (dx/(dt*dt)); // LB units

    VF_LOG_INFO("velocity  [dx/dt] = {}", velocityLB);
    VF_LOG_INFO("dt   = {}", dt);
    VF_LOG_INFO("dx   = {}", dx);
    VF_LOG_INFO("viscosity [10^8 dx^2/dt] = {}", viscosityLB*1e8);
    VF_LOG_INFO("u* /(dx/dt) = {}", u_star*dt/dx);
    VF_LOG_INFO("dpdx  = {}", pressureGradient);
    VF_LOG_INFO("dpdx /(dx/dt^2) = {}", pressureGradientLB);
    
    // double u_DP = 9.9*dt/dx;
    // double dpdx_DP = u_star * u_star / H / (dx/(dt*dt));
    // printf( "%1.20f \t %1.20f \t %1.20f \n" ,u_DP ,dpdx_DP, (double)(u_DP+dpdx_DP));
    real u_SP = 9.9*dt/dx;
    real dpdx_SP = u_star * u_star / H / (dx/(dt*dt));
    printf( " u = %1.20f \t dpdx = %1.20f \t u+dpdx = %1.20f \n" ,u_SP ,dpdx_SP,(real)(u_SP+dpdx_SP));
    real A = (u_SP+dpdx_SP);
    A -= u_SP;
    double res_A = (double)A-(double)dpdx_SP;
    printf("Res A: %1.20f \n", res_A);
    real res_B = A-dpdx_SP;  
    // double B = (double)u_SP+dpdx_SP;
    // B-= u_SP;
    printf("Res B: %1.20f \n", res_B);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    para->setOutputPrefix( simulationName );

    para->setFName(para->getOutputPath() + "/" + para->getOutputPrefix());

    para->setPrintFiles(true);

    para->setForcing(pressureGradientLB, 0, 0);
    para->setVelocity(velocityLB);
    para->setViscosity(viscosityLB);
    para->setVelocityRatio( dx / dt );
    para->setViscosityRatio( dx*dx/dt );
    para->setDensityRatio( 1.0 );

    if(para->getUseAMD())
        para->setMainKernel("TurbulentViscosityCumulantK17CompChim");
    else 
        para->setMainKernel("CumulantK17CompChim");
    
    para->setIsBodyForce( config.getValue<bool>("bodyForce") );

    para->setTStartOut(uint(tStartOut/dt) );
    para->setTOut( uint(tOut/dt) );
    para->setTEnd( uint(tEnd/dt) );

    // para->setTOut( 100 );
    // para->setTEnd( 100000 );

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    gridBuilder->addCoarseGrid(0.0, 0.0, 0.0,
                                L_x,  L_y,  L_z, dx);
    // gridBuilder->setNumberOfLayers(0,0);
    // gridBuilder->addGrid( new Cuboid( 300., 300., 300., 1000. , 1000., 600.), 1 );

    gridBuilder->setPeriodicBoundaryCondition(true, true, false);

	gridBuilder->buildGrids(lbmOrGks, false); // buildGrids() has to be called before setting the BCs!!!!

    uint samplingOffset = 2;
    // gridBuilder->setVelocityBoundaryCondition(SideType::MZ, 0.0, 0.0, 0.0);
    gridBuilder->setStressBoundaryCondition(SideType::MZ, 
                                            0.0, 0.0, 1.0,              // wall normals
                                            samplingOffset, z0/dx);     // wall model settinng
    para->setHasWallModelMonitor(true);


    // gridBuilder->setVelocityBoundaryCondition(SideType::PZ, 0.0, 0.0, 0.0);
    gridBuilder->setSlipBoundaryCondition(SideType::PZ,  0.0,  0.0, 0.0);

    para->setInitialCondition([&](real coordX, real coordY, real coordZ, real &rho, real &vx, real &vy, real &vz) {
        rho = (real)0.0;
        vx  = (0.4/0.4 * log(coordZ/z0)) * dt / dx; //10*coordZ/H * dt / dx;
        vy  = (real)0.0;
        vz  = 8.0*u_star/0.4*(sin(8.0*coordY*3.14/H)*sin(8.0*coordZ*3.14/H)+sin(3.14*8.0*coordX/L_x))/(pow(L_z/2.0-coordZ, c2o1)+c1o1) * dt / dx;
    });
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    SPtr<CudaMemoryManager> cudaMemoryManager = CudaMemoryManager::make(para);

    SPtr<GridProvider> gridGenerator = GridProvider::makeGridGenerator(gridBuilder, para, cudaMemoryManager);

    SPtr<PlanarAverageProbe> planarAverageProbe = SPtr<PlanarAverageProbe>( new PlanarAverageProbe("planeProbe", para->getOutputPath(), tStartAveraging/dt, tStartTmpAveraging/dt, tAveraging/dt , tStartOutProbe/dt, tOutProbe/dt, 'z') );
    planarAverageProbe->addAllAvailableStatistics();
    planarAverageProbe->setFileNameToNOut();
    para->addProbe( planarAverageProbe );

    para->setHasWallModelMonitor(true);
    SPtr<WallModelProbe> wallModelProbe = SPtr<WallModelProbe>( new WallModelProbe("wallModelProbe", para->getOutputPath(), tStartAveraging/dt, tStartTmpAveraging/dt, tAveraging/dt , tStartOutProbe/dt, tOutProbe/dt) );
    wallModelProbe->addAllAvailableStatistics();
    wallModelProbe->setFileNameToNOut();
    wallModelProbe->setForceOutputToStress(true);
    if(para->getIsBodyForce())
        wallModelProbe->setEvaluatePressureGradient(true);
    para->addProbe( wallModelProbe );

    Simulation sim(communicator);
    SPtr<FileWriter> fileWriter = SPtr<FileWriter>(new FileWriter());
    SPtr<KernelFactoryImp> kernelFactory = KernelFactoryImp::getInstance();
    SPtr<PreProcessorFactoryImp> preProcessorFactory = PreProcessorFactoryImp::getInstance();
    sim.setFactories(kernelFactory, preProcessorFactory);
    sim.init(para, gridGenerator, fileWriter, cudaMemoryManager);        
    sim.run();
    sim.free();
}

int main( int argc, char* argv[])
{
    if ( argv != NULL )
    {
        try
        {
            vf::logging::Logger::initalizeLogger();

            if( argc > 1){ path = argv[1]; }

            multipleLevel(path + "/configBoundaryLayer.txt");
        }
        catch (const spdlog::spdlog_ex &ex) {
            std::cout << "Log initialization failed: " << ex.what() << std::endl;
        }

        catch (const std::bad_alloc& e)
        { 
            VF_LOG_CRITICAL("Bad Alloc: {}", e.what());
        }
        catch (const std::exception& e)
        {   
            VF_LOG_CRITICAL("exception: {}", e.what());
        }
        catch (...)
        {
            VF_LOG_CRITICAL("Unknown exception!");
        }
    }
    return 0;
}

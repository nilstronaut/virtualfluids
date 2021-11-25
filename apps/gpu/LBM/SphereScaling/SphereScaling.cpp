
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <exception>
#include <memory>

#include "mpi.h"

//////////////////////////////////////////////////////////////////////////

#include "basics/Core/DataTypes.h"
#include "basics/PointerDefinitions.h"
#include "basics/Core/VectorTypes.h"

#include "basics/Core/LbmOrGks.h"
#include "basics/Core/StringUtilities/StringUtil.h"
#include "basics/config/ConfigurationFile.h"
#include "basics/Core/Logger/Logger.h"

//////////////////////////////////////////////////////////////////////////

#include "GridGenerator/grid/GridBuilder/LevelGridBuilder.h"
#include "GridGenerator/grid/GridBuilder/MultipleGridBuilder.h"
#include "GridGenerator/grid/BoundaryConditions/Side.h"
#include "GridGenerator/grid/GridFactory.h"

#include "geometries/Sphere/Sphere.h"
#include "geometries/Conglomerate/Conglomerate.h"
#include "geometries/TriangularMesh/TriangularMesh.h"

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

#include "VirtualFluids_GPU/Kernel/Utilities/KernelFactory/KernelFactoryImp.h"
#include "VirtualFluids_GPU/PreProcessor/PreProcessorFactory/PreProcessorFactoryImp.h"

#include "VirtualFluids_GPU/GPU/CudaMemoryManager.h"

//////////////////////////////////////////////////////////////////////////

#include "utilities/communication.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//          U s e r    s e t t i n g s
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Tesla 03
//  std::string outPath("E:/temp/SphereScalingResults/");
//  std::string gridPathParent = "E:/temp/GridSphereScaling/";
//  std::string simulationName("SphereScaling");
// std::string stlPath("C:/Users/Master/Documents/MasterAnna/STL/Sphere/");

// Phoenix
std::string outPath("/work/y0078217/Results/SphereScalingResults/");
std::string gridPathParent = "/work/y0078217/Grids/GridSphereScaling/";
std::string simulationName("SphereScaling");
std::string stlPath("/home/y0078217/STL/Sphere/");

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
    gridFactory->setGridStrategy(Device::CPU);
    gridFactory->setTriangularMeshDiscretizationMethod(TriangularMeshDiscretizationMethod::POINT_IN_OBJECT);

    auto gridBuilder = MultipleGridBuilder::makeShared(gridFactory);
    
	vf::gpu::Communicator* comm = vf::gpu::Communicator::getInstanz();
    vf::basics::ConfigurationFile config;
    std::cout << configPath << std::endl;
    config.load(configPath);
    SPtr<Parameter> para = std::make_shared<Parameter>(config, comm->getNummberOfProcess(), comm->getPID());



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool useGridGenerator                  = true;
    bool useStreams                        = true;
    bool useLevels                         = true;
    para->useReducedCommunicationAfterFtoC = true;
    std::string scalingType                = "weak"; // "strong" // "weak"

    if (para->getNumprocs() == 1) {
       useStreams       = false;
       para->useReducedCommunicationAfterFtoC = false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string gridPath(gridPathParent); // only for GridGenerator, for GridReader the gridPath needs to be set in the config file

    real dxGrid      = (real)0.2;
    real vxLB = (real)0.0005; // LB units
    real viscosityLB = 0.001; //(vxLB * dxGrid) / Re;

    para->setVelocity(vxLB);
    para->setViscosity(viscosityLB);
    para->setVelocityRatio((real) 58.82352941);
    para->setViscosityRatio((real) 0.058823529);
    para->setDensityRatio((real) 998.0);

    *logging::out << logging::Logger::INFO_HIGH << "velocity LB [dx/dt] = " << vxLB << " \n";
    *logging::out << logging::Logger::INFO_HIGH << "viscosity LB [dx^2/dt] = " << viscosityLB << "\n";
    *logging::out << logging::Logger::INFO_HIGH << "velocity real [m/s] = " << vxLB * para->getVelocityRatio()<< " \n";
    *logging::out << logging::Logger::INFO_HIGH << "viscosity real [m^2/s] = " << viscosityLB * para->getViscosityRatio() << "\n";
    *logging::out << logging::Logger::INFO_HIGH << "dxGrid = " << dxGrid << "\n";
    *logging::out << logging::Logger::INFO_HIGH << "useGridGenerator = " << useGridGenerator << "\n";
    *logging::out << logging::Logger::INFO_HIGH << "useStreams = " << useStreams << "\n";
    *logging::out << logging::Logger::INFO_HIGH << "number of processes = " << para->getNumprocs() << "\n";
    *logging::out << logging::Logger::INFO_HIGH << "para->useReducedCommunicationAfterFtoC = " <<  para->useReducedCommunicationAfterFtoC << "\n";
    *logging::out << logging::Logger::INFO_HIGH << "scalingType = " <<  scalingType << "\n";
    
    // para->setTOut(10);
    // para->setTEnd(10);

    para->setCalcDragLift(false);
    para->setUseWale(false);

    if (para->getOutputPath().size() == 0) {
        para->setOutputPath(outPath);
    }
    para->setOutputPrefix(simulationName);
    para->setFName(para->getOutputPath() + para->getOutputPrefix());
    para->setPrintFiles(true);
    std::cout << "Write result files to " << para->getFName() << std::endl;

    if (useLevels)
        para->setMaxLevel(2);
    else
        para->setMaxLevel(1);


    if (useStreams)
        para->setUseStreams();
    //para->setMainKernel("CumulantK17CompChim");
    para->setMainKernel("CumulantK17CompChimStream");
    *logging::out << logging::Logger::INFO_HIGH << "Kernel: " << para->getMainKernel() << "\n";

    // if (para->getNumprocs() == 4) {
    //     para->setDevices(std::vector<uint>{ 0u, 1u, 2u, 3u });
    //     para->setMaxDev(4);
    // } else if (para->getNumprocs() == 2) {
    //     para->setDevices(std::vector<uint>{ 2u, 3u });
    //     para->setMaxDev(2);
    // } else 
    //     para->setDevices(std::vector<uint>{ 0u });
    //     para->setMaxDev(1);



    //////////////////////////////////////////////////////////////////////////


    if (useGridGenerator) {
        real sideLengthCube;
        if (useLevels)
            sideLengthCube = 76; 
        else
            sideLengthCube = 86; // 60, 80 läuft
        real xGridMin          = 0.0; 
        real xGridMax          = sideLengthCube;
        real yGridMin          = 0.0;
        real yGridMax          = sideLengthCube;
        real zGridMin          = 0.0;
        real zGridMax          = sideLengthCube;
        const real dSphere     = 10.0;
        const real dSphereLev1 = 22.0;

        if (para->getNumprocs() > 1) {
            const uint generatePart = vf::gpu::Communicator::getInstanz()->getPID();

            real overlap = (real)8.0 * dxGrid;
            gridBuilder->setNumberOfLayers(10, 8);

            if (comm->getNummberOfProcess() == 2) {
                real zSplit = 0.5 * sideLengthCube;
                    
                if (scalingType == "weak"){
                    if (useLevels) {
                        dxGrid = dxGrid / pow(2.0, (1.0/3.0));
                        // dxGrid = round(dxGrid*100.0)/100.0  // round to tow decimal places
                    }
                    else {
                        zSplit = zGridMax;
                        zGridMax = zGridMax + sideLengthCube;
                    }
                }

                if (generatePart == 0) {
                    gridBuilder->addCoarseGrid(xGridMin, yGridMin, zGridMin, xGridMax, yGridMax, zSplit + overlap,
                                               dxGrid);
                }
                if (generatePart == 1) {
                    gridBuilder->addCoarseGrid(xGridMin, yGridMin, zSplit - overlap, xGridMax, yGridMax, zGridMax,
                                               dxGrid);
                }

                if (useLevels) {
                    gridBuilder->addGrid(new Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphereLev1), 1);
                }

                if (scalingType == "weak" && !useLevels) {
                    // Sphere* sphere1 = new  Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphere);
                    // Sphere* sphere2 = new  Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 1.5 * sideLengthCube, dSphere);
                    // auto conglo = Conglomerate::makeShared();
                    // conglo->add(sphere1);
                    // conglo->add(sphere2);
                    // gridBuilder->addGeometry(conglo.get());

                   TriangularMesh *sphereSTL = TriangularMesh::make(stlPath + "Spheres_2GPU.stl");
                   gridBuilder->addGeometry(sphereSTL);
                } else {
                    gridBuilder->addGeometry(new Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphere));
                }

                if (generatePart == 0)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xGridMax, yGridMin, yGridMax, zGridMin, zSplit));
                if (generatePart == 1)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xGridMax, yGridMin, yGridMax, zSplit, zGridMax));

                gridBuilder->buildGrids(LBM, true); // buildGrids() has to be called before setting the BCs!!!!
                                
                if (generatePart == 0) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PZ, 1);
                }

                if (generatePart == 1) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MZ, 0);
                }

                gridBuilder->setPeriodicBoundaryCondition(false, false, false);
                //////////////////////////////////////////////////////////////////////////
                gridBuilder->setVelocityBoundaryCondition(SideType::MX, vxLB, 0.0, 0.0);
                gridBuilder->setPressureBoundaryCondition(SideType::PX, 0.0);
                gridBuilder->setVelocityBoundaryCondition(SideType::MY, vxLB, 0.0, 0.0);
                gridBuilder->setVelocityBoundaryCondition(SideType::PY, vxLB, 0.0, 0.0);
                if (generatePart == 0)
                    gridBuilder->setVelocityBoundaryCondition(SideType::MZ, vxLB, 0.0, 0.0);
                if (generatePart == 1)
                    gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, 0.0, 0.0);
                // gridBuilder->setVelocityBoundaryCondition(SideType::GEOMETRY, 0.0, 0.0, 0.0);
                //////////////////////////////////////////////////////////////////////////
           
            } else if (comm->getNummberOfProcess() == 4) {
                real ySplit= 0.5 * sideLengthCube;
                real zSplit= 0.5 * sideLengthCube;

                if (scalingType == "weak") {
                    if (useLevels){
                        dxGrid = dxGrid / pow(2.0, (2.0/3.0));
                        // dxGrid = round(dxGrid*100.0)/100.0  // round to tow decimal places
                    }
                    else{
                        ySplit = yGridMax;
                        yGridMax = yGridMax + (yGridMax-yGridMin);
                        zSplit = zGridMax;
                        zGridMax = zGridMax + (zGridMax-zGridMin);
                    }
                }

                if (generatePart == 0) {
                    gridBuilder->addCoarseGrid(xGridMin, yGridMin, zGridMin, xGridMax , ySplit + overlap,
                                               zSplit + overlap, dxGrid);
                }
                if (generatePart == 1) {
                    gridBuilder->addCoarseGrid(xGridMin, ySplit - overlap, zGridMin, xGridMax, yGridMax,
                                               zSplit + overlap, dxGrid);
                }
                if (generatePart == 2) {
                    gridBuilder->addCoarseGrid(xGridMin, yGridMin, zSplit - overlap, xGridMax, ySplit + overlap,
                                               zGridMax, dxGrid);
                }
                if (generatePart == 3) {
                    gridBuilder->addCoarseGrid(xGridMin, ySplit - overlap, zSplit - overlap, xGridMax, yGridMax,
                                               zGridMax, dxGrid);
                }

                if (useLevels) {
                    gridBuilder->addGrid(new Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphereLev1), 1);                    
                }

                if (scalingType == "weak" && !useLevels) {
                   TriangularMesh *sphereSTL = TriangularMesh::make(stlPath + "Spheres_4GPU.stl");
                   gridBuilder->addGeometry(sphereSTL);
                } else {
                    gridBuilder->addGeometry(new Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphere));
                }

                if (generatePart == 0)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xGridMax, yGridMin, ySplit, zGridMin, zSplit));
                if (generatePart == 1)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xGridMax, ySplit, yGridMax, zGridMin, zSplit));
                if (generatePart == 2)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xGridMax, yGridMin, ySplit, zSplit, zGridMax));
                if (generatePart == 3)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xGridMax, ySplit, yGridMax, zSplit, zGridMax));


                gridBuilder->buildGrids(LBM, true); // buildGrids() has to be called before setting the BCs!!!!
                gridBuilder->setPeriodicBoundaryCondition(false, false, false);
                                
                if (generatePart == 0) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PY, 1);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PZ, 2);
                }
                if (generatePart == 1) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MY, 0);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PZ, 3);
                }
                if (generatePart == 2) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PY, 3);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MZ, 0);
                }
                if (generatePart == 3) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MY, 2);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MZ, 1);
                }

                //////////////////////////////////////////////////////////////////////////
                if (generatePart == 0) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::MY, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::MZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 1) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::PY, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::MZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 2) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::MY, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, 0.0, 0.0);                    
                }
                if (generatePart == 3) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::PY, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, 0.0, 0.0);
                }
                gridBuilder->setVelocityBoundaryCondition(SideType::MX, vxLB, 0.0, 0.0);
                gridBuilder->setPressureBoundaryCondition(SideType::PX, 0.0);
                // gridBuilder->setVelocityBoundaryCondition(SideType::GEOMETRY, 0.0, 0.0, 0.0);
                //////////////////////////////////////////////////////////////////////////
            } else if (comm->getNummberOfProcess() == 8) {
                real xSplit = 0.5 * sideLengthCube;
                real ySplit = 0.5 * sideLengthCube;
                real zSplit = 0.5 * sideLengthCube;

                if (scalingType == "weak") {
                    if (useLevels){
                        dxGrid = dxGrid / 2.0;
                        // dxGrid = round(dxGrid*100.0)/100.0  // round to tow decimal places
                    }
                    else {
                        xSplit = xGridMax;
                        xGridMax = xGridMax + (xGridMax-xGridMin);
                        ySplit = yGridMax;
                        yGridMax = yGridMax + (yGridMax-yGridMin);
                        zSplit = zGridMax;
                        zGridMax = zGridMax + (zGridMax-zGridMin);
                    }
                }

                if (generatePart == 0) {
                    gridBuilder->addCoarseGrid(xGridMin, yGridMin, zGridMin, xSplit + overlap, ySplit + overlap,
                                               zSplit + overlap, dxGrid);
                }
                if (generatePart == 1) {
                    gridBuilder->addCoarseGrid(xGridMin, ySplit - overlap, zGridMin, xSplit + overlap, yGridMax,
                                               zSplit + overlap, dxGrid);
                }
                if (generatePart == 2) {
                    gridBuilder->addCoarseGrid(xSplit - overlap, yGridMin, zGridMin, xGridMax, ySplit + overlap,
                                               zSplit + overlap, dxGrid);
                }
                if (generatePart == 3) {
                    gridBuilder->addCoarseGrid(xSplit - overlap, ySplit - overlap, zGridMin, xGridMax, yGridMax,
                                               zSplit + overlap, dxGrid);
                }
                if (generatePart == 4) {
                    gridBuilder->addCoarseGrid(xGridMin, yGridMin, zSplit - overlap, xSplit + overlap, ySplit + overlap,
                                               zGridMax, dxGrid);
                }
                if (generatePart == 5) {
                    gridBuilder->addCoarseGrid(xGridMin, ySplit - overlap, zSplit - overlap, xSplit + overlap, yGridMax,
                                               zGridMax, dxGrid);
                }
                if (generatePart == 6) {
                    gridBuilder->addCoarseGrid(xSplit - overlap, yGridMin, zSplit - overlap, xGridMax, ySplit + overlap,
                                               zGridMax, dxGrid);
                }
                if (generatePart == 7) {
                    gridBuilder->addCoarseGrid(xSplit - overlap, ySplit - overlap, zSplit - overlap, xGridMax, yGridMax,
                                               zGridMax, dxGrid);
                }

                if (useLevels) {
                    gridBuilder->addGrid(new Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphereLev1), 1);                    
                }

                if (scalingType == "weak" && !useLevels) {
                   TriangularMesh *sphereSTL = TriangularMesh::make(stlPath + "Spheres_8GPU.stl");
                   gridBuilder->addGeometry(sphereSTL);
                } else {
                    gridBuilder->addGeometry(new Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphere));
                }
                
                if (generatePart == 0)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xSplit, yGridMin, ySplit, zGridMin, zSplit));
                if (generatePart == 1)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xSplit, ySplit, yGridMax, zGridMin, zSplit));
                if (generatePart == 2)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xSplit, xGridMax, yGridMin, ySplit, zGridMin, zSplit));
                if (generatePart == 3)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xSplit, xGridMax, ySplit, yGridMax, zGridMin, zSplit));
                if (generatePart == 4)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xSplit, yGridMin, ySplit, zSplit, zGridMax));
                if (generatePart == 5)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xGridMin, xSplit, ySplit, yGridMax, zSplit, zGridMax));
                if (generatePart == 6)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xSplit, xGridMax, yGridMin, ySplit, zSplit, zGridMax));
                if (generatePart == 7)
                    gridBuilder->setSubDomainBox(
                        std::make_shared<BoundingBox>(xSplit, xGridMax, ySplit, yGridMax, zSplit, zGridMax));

                gridBuilder->buildGrids(LBM, true); // buildGrids() has to be called before setting the BCs!!!!
                gridBuilder->setPeriodicBoundaryCondition(false, false, false);

                if (generatePart == 0) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PY, 1);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PX, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PX, 2);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PZ, 4);
                }
                if (generatePart == 1) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MY, 0);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PX, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PX, 3);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PZ, 5);
                }
                if (generatePart == 2) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PY, 3);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MX, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MX, 0);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PZ, 6);
                }
                if (generatePart == 3) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MY, 2);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MX, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MX, 1);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PZ, 7);
                }
                if (generatePart == 4) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PY, 5);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PX, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PX, 6);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MZ, 0);
                }
                if (generatePart == 5) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MY, 4);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PX, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PX, 7);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MZ, 1);
                }
                if (generatePart == 6) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::PY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::PY, 7);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MX, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MX, 4);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MZ, 2);
                }
                if (generatePart == 7) {
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MY, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MY, 6);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MX, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MX, 5);
                    gridBuilder->findCommunicationIndices(CommunicationDirections::MZ, LBM);
                    gridBuilder->setCommunicationProcess(CommunicationDirections::MZ, 3);
                }

                //////////////////////////////////////////////////////////////////////////
                if (generatePart == 0) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::MX, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::MY, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::MZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 1) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::MX, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::PY, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::MZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 2) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::MY, vxLB, 0.0, 0.0);
                    gridBuilder->setPressureBoundaryCondition(SideType::PX, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::MZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 3) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::PY, vxLB, 0.0, 0.0);
                    gridBuilder->setPressureBoundaryCondition(SideType::PX, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::MZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 4) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::MX, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::MY, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 5) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::MX, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::PY, vxLB, 0.0, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 6) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::MY, vxLB, 0.0, 0.0);
                    gridBuilder->setPressureBoundaryCondition(SideType::PX, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, 0.0, 0.0);
                }
                if (generatePart == 7) {
                    gridBuilder->setVelocityBoundaryCondition(SideType::PY, vxLB, 0.0, 0.0);
                    gridBuilder->setPressureBoundaryCondition(SideType::PX, 0.0);
                    gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, 0.0, 0.0);
                }
                // gridBuilder->setVelocityBoundaryCondition(SideType::GEOMETRY, 0.0, 0.0, 0.0);
                //////////////////////////////////////////////////////////////////////////                
            }
            if (para->getKernelNeedsFluidNodeIndicesToRun())
                gridBuilder->findFluidNodes(useStreams);

            // gridBuilder->writeGridsToVtk(outPath + "grid/part" +
            // std::to_string(generatePart) + "_"); gridBuilder->writeGridsToVtk(outPath +
            // std::to_string(generatePart) + "/grid/"); gridBuilder->writeArrows(outPath + 
            // std::to_string(generatePart) + " /arrow");

            SimulationFileWriter::write(gridPath + std::to_string(generatePart) + "/", gridBuilder,
                                        FILEFORMAT::BINARY);
        } else {

            gridBuilder->addCoarseGrid(xGridMin, yGridMin, zGridMin, xGridMax, yGridMax, zGridMax, dxGrid);

            if (useLevels) {
                gridBuilder->setNumberOfLayers(10, 8);
                gridBuilder->addGrid(new Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphereLev1), 1);
            }
                
            if (scalingType == "weak" && !useLevels){
                   TriangularMesh *sphereSTL = TriangularMesh::make(stlPath + "Spheres_1GPU.stl");
                   gridBuilder->addGeometry(sphereSTL);
            } else {
                gridBuilder->addGeometry(new Sphere(0.5 * sideLengthCube, 0.5 * sideLengthCube, 0.5 * sideLengthCube, dSphere));
            }

            gridBuilder->buildGrids(LBM, true); // buildGrids() has to be called before setting the BCs!!!!
            
            gridBuilder->setPeriodicBoundaryCondition(false, false, false);
            //////////////////////////////////////////////////////////////////////////
            gridBuilder->setVelocityBoundaryCondition(SideType::PY, vxLB, 0.0, 0.0);
            gridBuilder->setVelocityBoundaryCondition(SideType::MY, vxLB, 0.0, 0.0);
            gridBuilder->setPressureBoundaryCondition(SideType::PX, 0.0);
            gridBuilder->setVelocityBoundaryCondition(SideType::MX, vxLB, 0.0, 0.0);
            gridBuilder->setVelocityBoundaryCondition(SideType::MZ, vxLB, 0.0, 0.0);
            gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, 0.0, 0.0);

            // gridBuilder->setVelocityBoundaryCondition(SideType::GEOMETRY, 0.0, 0.0, 0.0);
            //////////////////////////////////////////////////////////////////////////
            if (para->getKernelNeedsFluidNodeIndicesToRun())
                gridBuilder->findFluidNodes(useStreams);

            // gridBuilder->writeGridsToVtk("E:/temp/MusselOyster/" + "/grid/");
            // gridBuilder->writeArrows ("E:/temp/MusselOyster/" + "/arrow");

            SimulationFileWriter::write(gridPath, gridBuilder, FILEFORMAT::BINARY);
        }        
    }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    SPtr<CudaMemoryManager> cudaMemoryManager = CudaMemoryManager::make(para);

    SPtr<GridProvider> gridGenerator;
    if (useGridGenerator)
        gridGenerator = GridProvider::makeGridGenerator(gridBuilder, para, cudaMemoryManager);
    else {
        gridGenerator = GridProvider::makeGridReader(FILEFORMAT::BINARY, para, cudaMemoryManager);
    }
           
    Simulation sim;
    SPtr<FileWriter> fileWriter = SPtr<FileWriter>(new FileWriter());
    SPtr<KernelFactoryImp> kernelFactory = KernelFactoryImp::getInstance();
    SPtr<PreProcessorFactoryImp> preProcessorFactory = PreProcessorFactoryImp::getInstance();
    sim.setFactories(kernelFactory, preProcessorFactory);
    sim.init(para, gridGenerator, fileWriter, cudaMemoryManager);
    sim.run();
    sim.free();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
}

int main( int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    std::string str, str2, configFile;

    if ( argv != NULL )
    {
        
        try
        {
            //////////////////////////////////////////////////////////////////////////

			std::string targetPath;

			targetPath = __FILE__;

            if (argc == 2) {
                configFile = argv[1];
                std::cout << "Using configFile command line argument: " << configFile << std::endl;
            }

#ifdef _WIN32
            targetPath = targetPath.substr(0, targetPath.find_last_of('\\') + 1);
#else
            targetPath = targetPath.substr(0, targetPath.find_last_of('/') + 1);
#endif

			std::cout << targetPath << std::endl;

            if (configFile.size()==0) {
                configFile = targetPath + "config.txt";
            }        

			multipleLevel(configFile);

            //////////////////////////////////////////////////////////////////////////
		}
        catch (const std::bad_alloc& e)
        { 
            *logging::out << logging::Logger::LOGGER_ERROR << "Bad Alloc:" << e.what() << "\n";
        }
        catch (const std::exception& e)
        {   
            *logging::out << logging::Logger::LOGGER_ERROR << e.what() << "\n";
        }
        catch (...)
        {
            *logging::out << logging::Logger::LOGGER_ERROR << "Unknown exception!\n";
        }
    }

   MPI_Finalize();
   return 0;
}

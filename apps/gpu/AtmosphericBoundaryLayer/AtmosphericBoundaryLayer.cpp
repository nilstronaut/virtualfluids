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
//  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
//  for more details.
//
//  SPDX-License-Identifier: GPL-3.0-or-later
//  SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder
//
//! \addtogroup AtmosphericBoundaryLayer
//! \ingroup gpu_apps
//! \{
//! \author Henry Korb, Henrik Asmuth
//=======================================================================================
#include <numeric>

#include <basics/DataTypes.h>
#include <basics/config/ConfigurationFile.h>
#include <basics/constants/NumericConstants.h>

#include <logger/Logger.h>

#include <parallel/MPICommunicator.h>

//////////////////////////////////////////////////////////////////////////

#include "GridGenerator/TransientBCSetter/TransientBCSetter.h"
#include "GridGenerator/geometries/BoundingBox/BoundingBox.h"
#include "GridGenerator/geometries/Cuboid/Cuboid.h"
#include "GridGenerator/grid/BoundaryConditions/BoundaryCondition.h"
#include "GridGenerator/grid/BoundaryConditions/Side.h"
#include "GridGenerator/grid/GridBuilder/LevelGridBuilder.h"
#include "GridGenerator/grid/GridBuilder/MultipleGridBuilder.h"
#include "GridGenerator/utilities/communication.h"

//////////////////////////////////////////////////////////////////////////

#include "gpu/core/BoundaryConditions/BoundaryConditionFactory.h"
#include "gpu/core/Calculation/Simulation.h"
#include "gpu/core/Cuda/CudaMemoryManager.h"
#include "gpu/core/GridScaling/GridScalingFactory.h"
#include "gpu/core/Kernel/KernelTypes.h"
#include "gpu/core/Parameter/Parameter.h"
#include "gpu/core/Samplers/PrecursorWriter.h"
#include "gpu/core/Samplers/PlanarAverageProbe.h"
#include "gpu/core/Samplers/Probe.h"
#include "gpu/core/Samplers/WallModelProbe.h"
#include "gpu/core/TurbulenceModels/TurbulenceModelFactory.h"

using namespace vf::basics::constant;

const std::string defaultConfigFile = "abl.cfg";

void run(const vf::basics::ConfigurationFile& config)
{
    const std::string simulationName("AtmosphericBoundaryLayer");

    const uint samplingOffsetWallModel = 2;
    const uint averagingTimestepsPlaneProbes = 10;
    const uint maximumNumberOfTimestepsPerPrecursorFile = 1000;
    const real viscosity = 1.56e-5;
    const real machNumber = 0.1;

    const real boundaryLayerHeight = config.getValue("boundaryLayerHeight", 1000.0);

    const real lengthX = 6 * boundaryLayerHeight;
    const real lengthY = 4 * boundaryLayerHeight;
    const real lengthZ = boundaryLayerHeight;

    const real roughnessLength = config.getValue("z0", c1o10);
    const real frictionVelocity = config.getValue("u_star", c4o10);
    const real vonKarmanConstant = config.getValue("vonKarmanConstant", c4o10);

    const uint nodesPerBoundaryLyerHeight = config.getValue<uint>("NodesPerBoundaryLayerHeight", 64);

    const float periodicShift = config.getValue<float>("PeriodicShift", c0o1);

    const bool writePrecursor = config.getValue("WritePrecursor", false);

    const bool useDistributionsForPrecursor = config.getValue<bool>("UseDistributions", false);
    std::string precursorDirectory = config.getValue<std::string>("PrecursorDirectory", "precursor/");
    if (precursorDirectory.back() != '/')
        precursorDirectory += '/';
    const int timeStepsWritePrecursor = config.getValue<int>("nTimestepsWritePrecursor", 10);
    const real timeStartPrecursor = config.getValue<real>("tStartPrecursor", 36000.);
    const real positionXPrecursorSamplingPlane = config.getValue<real>("posXPrecursor", c1o2 * lengthX);

    const bool usePrecursorInflow = config.getValue("ReadPrecursor", false);
    const int timestepsBetweenReadsPrecursor = config.getValue<int>("nTimestepsReadPrecursor", 10);

    const bool useRefinement = config.getValue<bool>("Refinement", false);

    // all in s
    const float timeStartOut = config.getValue<real>("tStartOut");
    const float timeOut = config.getValue<real>("tOut");
    const float timeEnd = config.getValue<real>("tEnd");

    const float timeStartAveraging = config.getValue<real>("tStartAveraging");
    const float timeStartTemporalAveraging = config.getValue<real>("tStartTmpAveraging");
    const float timeAveraging = config.getValue<real>("tAveraging");
    const float timeStartOutProbe = config.getValue<real>("tStartOutProbe");
    const float timeOutProbe = config.getValue<real>("tOutProbe");

    //////////////////////////////////////////////////////////////////////////
    // compute parameters in lattice units
    //////////////////////////////////////////////////////////////////////////
    const auto velocityProfile = [&](real coordZ) {
        return frictionVelocity / vonKarmanConstant * log(coordZ / roughnessLength + c1o1);
    };
    const real velocity = c1o2 * velocityProfile(boundaryLayerHeight);

    const real deltaX = boundaryLayerHeight / real(nodesPerBoundaryLyerHeight);

    const real deltaT = c1oSqrt3 * deltaX * machNumber / velocity;

    const real velocityLB = velocity * deltaT / deltaX;

    const real viscosityLB = viscosity * deltaT / (deltaX * deltaX);

    const real pressureGradient = frictionVelocity * frictionVelocity / boundaryLayerHeight;
    const real pressureGradientLB = pressureGradient * (deltaT * deltaT) / deltaX;

    const uint timeStepStartAveraging = uint(timeStartAveraging / deltaT);
    const uint timeStepStartTemporalAveraging = uint(timeStartTemporalAveraging / deltaT);
    const uint timeStepAveraging = uint(timeAveraging / deltaT);
    const uint timeStepStartOutProbe = uint(timeStartOutProbe / deltaT);
    const uint timeStepOutProbe = uint(timeOutProbe / deltaT);

    const uint timeStepStartPrecursor = uint(timeStartPrecursor / deltaT);

    //////////////////////////////////////////////////////////////////////////
    // create grid
    //////////////////////////////////////////////////////////////////////////

    vf::parallel::Communicator& communicator = *vf::parallel::MPICommunicator::getInstance();

    const int numberOfProcesses = communicator.getNumberOfProcesses();
    const int processID = communicator.getProcessID();
    const bool isMultiGPU = numberOfProcesses > 1;

    const real lengthXPerProcess = lengthX / numberOfProcesses;
    const real overlap = 8.0 * deltaX;

    const real xMin = processID * lengthXPerProcess;
    const real xMax = (processID + 1) * lengthXPerProcess;
    real xGridMin = processID * lengthXPerProcess;
    real xGridMax = (processID + 1) * lengthXPerProcess;

    const real yMin = c0o1;
    const real yMax = lengthY;
    const real zMin = c0o1;
    const real zMax = lengthZ;

    const bool isFirstSubDomain = isMultiGPU && (processID == 0);
    const bool isLastSubDomain = isMultiGPU && (processID == numberOfProcesses - 1);
    const bool isMidSubDomain = isMultiGPU && !(isFirstSubDomain || isLastSubDomain);

    if (isFirstSubDomain) {
        xGridMax += overlap;
    }
    if (isFirstSubDomain && !usePrecursorInflow) {
        xGridMin -= overlap;
    }

    if (isLastSubDomain) {
        xGridMin -= overlap;
    }

    if (isLastSubDomain && !usePrecursorInflow) {
        xGridMax += overlap;
    }

    if (isMidSubDomain) {
        xGridMax += overlap;
        xGridMin -= overlap;
    }

    auto gridBuilder = std::make_shared<MultipleGridBuilder>();
    auto scalingFactory = GridScalingFactory();

    gridBuilder->addCoarseGrid(xGridMin, c0o1, c0o1, xGridMax, lengthY, lengthZ, deltaX);

    if (useRefinement) {
        gridBuilder->setNumberOfLayers(4, 0);
        real xMaxRefinement = xGridMax;
        if (usePrecursorInflow) {
            xMaxRefinement = xGridMax - boundaryLayerHeight;
        }
        gridBuilder->addGrid(std::make_shared<Cuboid>(xGridMin, c0o1, c0o1, xMaxRefinement, lengthY, c1o2 * lengthZ), 1);
        scalingFactory.setScalingFactory(GridScalingFactory::GridScaling::ScaleCompressible);
    }

    if (numberOfProcesses > 1) {
        gridBuilder->setSubDomainBox(std::make_shared<BoundingBox>(xMin, xMax, yMin, yMax, zMin, zMax));
        gridBuilder->setPeriodicBoundaryCondition(false, true, false);
    } else {
        gridBuilder->setPeriodicBoundaryCondition(!usePrecursorInflow, true, false);
    }

    if (!usePrecursorInflow) {
        gridBuilder->setPeriodicShiftOnXBoundaryInYDirection(periodicShift);
    }

    gridBuilder->buildGrids(true); // buildGrids() has to be called before setting the BCs!!!!

    //////////////////////////////////////////////////////////////////////////
    // set parameters
    //////////////////////////////////////////////////////////////////////////
    auto para = std::make_shared<Parameter>(numberOfProcesses, communicator.getProcessID(), &config);

    para->setOutputPrefix(simulationName);

    para->setPrintFiles(true);

    if (!usePrecursorInflow)
        para->setForcing(pressureGradientLB, 0, 0);
    para->setVelocityLB(velocityLB);
    para->setViscosityLB(viscosityLB);
    para->setVelocityRatio(deltaX / deltaT);
    para->setViscosityRatio(deltaX * deltaX / deltaT);
    para->setDensityRatio(c1o1);

    para->setUseStreams(numberOfProcesses > 1);
    para->configureMainKernel(vf::collisionKernel::compressible::K17CompressibleNavierStokes);

    para->setTimestepStartOut(uint(timeStartOut / deltaT));
    para->setTimestepOut(uint(timeOut / deltaT));
    para->setTimestepEnd(uint(timeEnd / deltaT));

    std::vector<uint> devices(10);
    std::iota(devices.begin(), devices.end(), 0);
    para->setDevices(devices);
    para->setMaxDev(numberOfProcesses);
    if (usePrecursorInflow) {
        para->setInitialCondition([&](real coordX, real coordY, real coordZ, real& rho, real& vx, real& vy, real& vz) {
            rho = c0o1;
            vx = velocityProfile(coordZ) * deltaT / deltaX;
            vy = c0o1;
            vz = c0o1;
        });
    } else {
        para->setInitialCondition([&](real coordX, real coordY, real coordZ, real& rho, real& vx, real& vy, real& vz) {
            const real relativeX = coordX / lengthX;
            const real relativeY = coordY / lengthY;
            const real relativeZ = coordZ / lengthZ;

            const real horizontalPerturbation = sin(cPi * c16o1 * relativeX);
            const real verticalPerturbation = sin(cPi * c8o1 * relativeZ) / (pow(relativeZ, c2o1) + c1o1);
            const real perturbation = c2o1 * horizontalPerturbation * verticalPerturbation;

            rho = c0o1;
            vx = (velocityProfile(coordZ) + perturbation) * (c1o1 - c1o10 * abs(relativeY - c1o2)) * deltaT / deltaX;
            vy = perturbation * deltaT / deltaX;
            vz = c8o1 * frictionVelocity / vonKarmanConstant *
                 (sin(cPi * c8o1 * coordY / boundaryLayerHeight) * sin(cPi * c8o1 * relativeZ) +
                  sin(cPi * c8o1 * relativeX)) /
                 (pow(c1o2 * lengthZ - coordZ, c2o1) + c1o1) * deltaT / deltaX;
        });
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (numberOfProcesses > 1) {
        if (isFirstSubDomain || isMidSubDomain) {
            gridBuilder->findCommunicationIndices(CommunicationDirections::PX);
            gridBuilder->setCommunicationProcess(CommunicationDirections::PX, processID + 1);
        }

        if (isLastSubDomain || isMidSubDomain) {
            gridBuilder->findCommunicationIndices(CommunicationDirections::MX, true);
            gridBuilder->setCommunicationProcess(CommunicationDirections::MX, processID - 1);
        }

        if (isFirstSubDomain && !usePrecursorInflow) {
            gridBuilder->findCommunicationIndices(CommunicationDirections::MX);
            gridBuilder->setCommunicationProcess(CommunicationDirections::MX, numberOfProcesses - 1);
        }

        if (isLastSubDomain && !usePrecursorInflow) {
            gridBuilder->findCommunicationIndices(CommunicationDirections::PX);
            gridBuilder->setCommunicationProcess(CommunicationDirections::PX, 0);
        }
    }

    if (usePrecursorInflow) {
        if (!isMultiGPU || isFirstSubDomain) {
            auto precursor = createFileCollection(precursorDirectory + "precursor", TransientBCFileType::VTK);
            gridBuilder->setPrecursorBoundaryCondition(SideType::MX, precursor, timestepsBetweenReadsPrecursor);
        }

        if (!isMultiGPU || isLastSubDomain) {
            gridBuilder->setPressureBoundaryCondition(SideType::PX, c0o1);
        }
    }

    gridBuilder->setStressBoundaryCondition(SideType::MZ, c0o1, c0o1, c1o1, samplingOffsetWallModel, roughnessLength,
                                            deltaX);

    gridBuilder->setSlipBoundaryCondition(SideType::PZ, c0o1, c0o1, -c1o1);

    BoundaryConditionFactory bcFactory = BoundaryConditionFactory();
    bcFactory.setVelocityBoundaryCondition(BoundaryConditionFactory::VelocityBC::VelocityInterpolatedCompressible);
    bcFactory.setStressBoundaryCondition(BoundaryConditionFactory::StressBC::StressBounceBackPressureCompressible);
    bcFactory.setSlipBoundaryCondition(BoundaryConditionFactory::SlipBC::SlipTurbulentViscosityCompressible);
    bcFactory.setPressureBoundaryCondition(BoundaryConditionFactory::PressureBC::OutflowNonReflective);
    if (useDistributionsForPrecursor) {
        bcFactory.setPrecursorBoundaryCondition(BoundaryConditionFactory::PrecursorBC::PrecursorDistributions);
    } else {
        bcFactory.setPrecursorBoundaryCondition(BoundaryConditionFactory::PrecursorBC::PrecursorNonReflectiveCompressible);
    }

    //////////////////////////////////////////////////////////////////////////
    // add probes
    //////////////////////////////////////////////////////////////////////////
    auto cudaMemoryManager = std::make_shared<CudaMemoryManager>(para);

    if (!usePrecursorInflow && (isFirstSubDomain || !isMultiGPU)) {
        const auto planarAverageProbe = std::make_shared<PlanarAverageProbe>(
            para, cudaMemoryManager, para->getOutputPath(), "planarAverageProbe", timeStepStartAveraging,
            timeStepStartTemporalAveraging, timeStepAveraging, timeStepStartOutProbe, timeStepOutProbe, PlanarAverageProbe::PlaneNormal::z, true);
        planarAverageProbe->addAllAvailableStatistics();
        planarAverageProbe->setFileNameToNOut();
        para->addSampler(planarAverageProbe);

        const auto wallModelProbe = std::make_shared<WallModelProbe>(
            para, cudaMemoryManager, para->getOutputPath(), "wallModelProbe", timeStepStartAveraging,
            timeStepStartTemporalAveraging, timeStepAveraging / 4, timeStepStartOutProbe, timeStepOutProbe, false, true, true, para->getIsBodyForce());

        para->addSampler(wallModelProbe);

        para->setHasWallModelMonitor(true);
    }

    for (int iPlane = 0; iPlane < 3; iPlane++) {
        const std::string name = "planeProbe" + std::to_string(iPlane);
        const auto horizontalProbe =
            std::make_shared<Probe>(para, cudaMemoryManager, para->getOutputPath(), name, timeStepStartAveraging,
                                         averagingTimestepsPlaneProbes, timeStepStartOutProbe, timeStepOutProbe, false, false);
        horizontalProbe->addProbePlane(c0o1, c0o1, iPlane * lengthZ / c4o1, lengthX, lengthY, deltaX);
        horizontalProbe->addAllAvailableStatistics();
        para->addSampler(horizontalProbe);
    }

    auto crossStreamPlane = std::make_shared<Probe>(para, cudaMemoryManager, para->getOutputPath(), "crossStreamPlane",
                                                         timeStepStartAveraging, averagingTimestepsPlaneProbes,
                                                         timeStepStartOutProbe, timeStepOutProbe, false, false);
    crossStreamPlane->addProbePlane(c1o2 * lengthX, c0o1, c0o1, deltaX, lengthY, lengthZ);
    crossStreamPlane->addAllAvailableStatistics();
    para->addSampler(crossStreamPlane);

    if (usePrecursorInflow) {
        auto streamwisePlane = std::make_shared<Probe>(
            para, cudaMemoryManager, para->getOutputPath(), "streamwisePlane", timeStepStartAveraging,
            averagingTimestepsPlaneProbes, timeStepStartOutProbe, timeStepOutProbe, false, false);
        streamwisePlane->addProbePlane(c0o1, c1o2 * lengthY, c0o1, lengthX, deltaX, lengthZ);
        streamwisePlane->addAllAvailableStatistics();
        para->addSampler(streamwisePlane);
    }

    if (writePrecursor) {
        const std::string fullPrecursorDirectory = para->getOutputPath() + precursorDirectory;
        const auto outputVariable =
            useDistributionsForPrecursor ? OutputVariable::Distributions : OutputVariable::Velocities;
        auto precursorWriter = std::make_shared<PrecursorWriter>(
            para, cudaMemoryManager, fullPrecursorDirectory, "precursor", positionXPrecursorSamplingPlane, c0o1, lengthY,
            c0o1, lengthZ, timeStepStartPrecursor, timeStepsWritePrecursor, outputVariable,
            maximumNumberOfTimestepsPerPrecursorFile);
        para->addSampler(precursorWriter);
    }

    auto tmFactory = std::make_shared<TurbulenceModelFactory>(para);
    tmFactory->readConfigFile(config);

    VF_LOG_INFO("process parameter:");
    VF_LOG_INFO("Number of Processes {} process ID {}", numberOfProcesses, processID);
    if (isFirstSubDomain)
        VF_LOG_INFO("Process ID {} is the first subdomain");
    if (isLastSubDomain)
        VF_LOG_INFO("Process ID {} is the last subdomain");
    if (isMidSubDomain)
        VF_LOG_INFO("Process ID {} is a mid subdomain");
    printf("\n");

    Simulation simulation(para, cudaMemoryManager, gridBuilder, &bcFactory, tmFactory, &scalingFactory);
    simulation.run();
}

int main(int argc, char* argv[])
{
    try {
        vf::logging::Logger::initializeLogger();
        auto config = vf::basics::loadConfig(argc, argv, defaultConfigFile);
        run(config);
    } catch (const std::exception& e) {
        VF_LOG_WARNING("{}", e.what());
        return 1;
    }
    return 0;
}

//! \}

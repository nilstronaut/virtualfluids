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
//! \addtogroup DrivenCavity
//! \ingroup gpu_apps
//! \{
//! \author Martin Schoenherr
//=======================================================================================
#include <string>

#include <basics/DataTypes.h>
#include <basics/config/ConfigurationFile.h>

#include <logger/Logger.h>

#include <GridGenerator/geometries/Cuboid/Cuboid.h>
#include <GridGenerator/grid/BoundaryConditions/Side.h>
#include <GridGenerator/grid/GridBuilder/LevelGridBuilder.h>
#include <GridGenerator/grid/GridBuilder/MultipleGridBuilder.h>

#include <gpu/core/BoundaryConditions/BoundaryConditionFactory.h>
#include <gpu/core/Calculation/Simulation.h>
#include <gpu/core/GridScaling/GridScalingFactory.h>
#include <gpu/core/Kernel/KernelTypes.h>
#include <gpu/core/Parameter/Parameter.h>

// velocity    = 1 m/s
// velocityLB  = 0.0305
// viscosityLB = 0.018605
// Re = 100
// dx = 0.0163934
// dt = 0.0005

void run(const vf::basics::ConfigurationFile& config)
{
    //////////////////////////////////////////////////////////////////////////
    // Simulation parameters
    //////////////////////////////////////////////////////////////////////////
    std::string path("./output/DrivenCavity");
    std::string simulationName("LidDrivenCavity");

    // Physical Parameters
    const real length = config.getValue<real>("L");
    const real velocity = config.getValue<real>("u");
    const real reynoldsNumber = config.getValue<real>("Re");

    // LBM Parameters
    const real velocityLB = config.getValue<real>("uLB"); // LB units
    const uint numberOfNodesX = config.getValue<unsigned int>("Nx");

    // Simulation Parameters
    const uint timeStepOut = config.getValue<unsigned int>("dtOut");
    const uint timeStepEnd = config.getValue<unsigned int>("tEnd");;

    bool refine = false;
    if (config.contains("refine"))
        refine = config.getValue<bool>("refine");

    if (config.contains("output_path"))
        path = config.getValue<std::string>("output_path");

    //////////////////////////////////////////////////////////////////////////
    // compute parameters in lattice units
    //////////////////////////////////////////////////////////////////////////

    const real deltaX = length / real(numberOfNodesX);
    const real deltaT = velocityLB / velocity * deltaX;

    const real vxLB = velocityLB / sqrt(2.0); // LB units
    const real vyLB = velocityLB / sqrt(2.0); // LB units

    const real viscosityLB = numberOfNodesX * velocityLB / reynoldsNumber; // LB units

    //////////////////////////////////////////////////////////////////////////
    // create grid
    //////////////////////////////////////////////////////////////////////////

    auto gridBuilder = std::make_shared<MultipleGridBuilder>();

    gridBuilder->addCoarseGrid(-0.5 * length, -0.5 * length, -0.5 * length, 0.5 * length, 0.5 * length, 0.5 * length, deltaX);
    if (refine)
        gridBuilder->addGrid(std::make_shared<Cuboid>(-0.25, -0.25, -0.25, 0.25, 0.25, 0.25), 1);

    GridScalingFactory scalingFactory = GridScalingFactory();
    scalingFactory.setScalingFactory(GridScalingFactory::GridScaling::ScaleCompressible);

    gridBuilder->setPeriodicBoundaryCondition(false, false, false);

    gridBuilder->buildGrids(false);

    //////////////////////////////////////////////////////////////////////////
    // set parameters
    //////////////////////////////////////////////////////////////////////////
    auto para = std::make_shared<Parameter>();

    para->worldLength = length;

    para->setOutputPath(path);
    para->setOutputPrefix(simulationName);

    para->setPrintFiles(true);

    para->setVelocityLB(velocityLB);
    para->setViscosityLB(viscosityLB);

    para->setVelocityRatio(velocity / velocityLB);
    para->setDensityRatio(1.0);

    para->setTimestepOut(timeStepOut);
    para->setTimestepEnd(timeStepEnd);

    para->configureMainKernel(vf::collision_kernel::compressible::K17CompressibleNavierStokes);

    //////////////////////////////////////////////////////////////////////////
    // set boundary conditions
    //////////////////////////////////////////////////////////////////////////

    gridBuilder->setNoSlipBoundaryCondition(SideType::PX);
    gridBuilder->setNoSlipBoundaryCondition(SideType::MX);
    gridBuilder->setNoSlipBoundaryCondition(SideType::PY);
    gridBuilder->setNoSlipBoundaryCondition(SideType::MY);
    gridBuilder->setNoSlipBoundaryCondition(SideType::MZ);
    gridBuilder->setVelocityBoundaryCondition(SideType::PZ, vxLB, vyLB, 0.0);

    BoundaryConditionFactory bcFactory;

    bcFactory.setNoSlipBoundaryCondition(BoundaryConditionFactory::NoSlipBC::NoSlipBounceBack);
    bcFactory.setVelocityBoundaryCondition(BoundaryConditionFactory::VelocityBC::VelocityBounceBack);

    //////////////////////////////////////////////////////////////////////////
    // run simulation
    //////////////////////////////////////////////////////////////////////////

    Simulation simulation(para, gridBuilder, &bcFactory, &scalingFactory);
    simulation.run();
}

int main(int argc, char* argv[])
{
    try {
        vf::logging::Logger::initializeLogger();
        vf::basics::ConfigurationFile config =
            vf::basics::loadConfig(argc, argv, "./apps/gpu/DrivenCavity/drivencavity_1level.cfg");
        run(config);
    } catch (const std::exception& e) {
        VF_LOG_WARNING("{}", e.what());
        return 1;
    }
    return 0;
}

//! \}

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
//! \addtogroup gpu_Output Output
//! \ingroup gpu_core core
//! \{
//! \author Soeren Peters
//=======================================================================================
#include "MetaDataCreator.h"

#include "Parameter/Parameter.h"
#include "cuda_helper/DeviceInfo.h"
#include "logger/Logger.h"

#include <omp.h>

namespace vf::gpu
{

vf::basics::MetaData createMetaData(const Parameter& parameter)
{
    vf::basics::MetaData meta_data;

    meta_data.name = parameter.getOutputPrefix();

    meta_data.world.length = parameter.worldLength;
    meta_data.world.velocity = parameter.getVelocityRatio() * parameter.getVelocity();
    meta_data.discretization.dx = parameter.worldLength / parameter.getParH(0)->gridNX;

    meta_data.simulation.lb_velocity = parameter.getVelocity();
    meta_data.simulation.lb_viscosity = parameter.getViscosity();

    meta_data.discretization.dt = meta_data.simulation.lb_velocity / meta_data.world.velocity * meta_data.discretization.dx;
    meta_data.simulation.reynoldsNumber = parameter.getRe();

    meta_data.simulation.numberOfTimeSteps = parameter.getTimestepEnd();

    meta_data.simulation.collisionKernel = parameter.getMainKernel();

    meta_data.simulation.quadricLimiters[0] = parameter.getQuadricLimitersHost()[0];
    meta_data.simulation.quadricLimiters[1] = parameter.getQuadricLimitersHost()[1];
    meta_data.simulation.quadricLimiters[2] = parameter.getQuadricLimitersHost()[2];

    meta_data.discretization.totalNumberOfNodes = 0.;
    for (int level = parameter.getCoarse(); level <= parameter.getFine(); level++) {
        meta_data.discretization.totalNumberOfNodes += parameter.getParH(level)->numberOfNodes;
        meta_data.discretization.numberOfNodesPerLevel.push_back(parameter.getParH(level)->numberOfNodes);
    }

    meta_data.discretization.numberOfLevels = parameter.getMaxLevel() + 1;

    meta_data.numberOfProcesses = parameter.getNumprocs();

    const int numOfThreads = omp_get_num_threads();
    meta_data.numberOfThreads = numOfThreads;

    meta_data.vf_hardware = "GPU";

    int count = 1;
    for (const auto& deviceId : parameter.getDevices()) {
        meta_data.gpus.push_back({ vf::cuda::getGPUName(deviceId), vf::cuda::getComputeCapability(deviceId) });
        if (count >= parameter.getNumprocs())
            break;
        count++;
    }

    return meta_data;
}

} // namespace vf::gpu

//! \}

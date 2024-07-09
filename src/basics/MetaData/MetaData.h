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
//! \addtogroup MetaData
//! \ingroup basics
//! \{
//! \author Soeren Peters
//=======================================================================================
#ifndef VF_BASICS_METADATA_H
#define VF_BASICS_METADATA_H

#include <array>
#include <string>
#include <vector>

#include "DataTypes.h"

#include <logger/Logger.h>

namespace vf::basics
{

std::string getCurrentTime();

struct MetaData
{
    MetaData();

    struct Simulation
    {
        std::string startDateTime;
        double runtimeSeconds;

        double reynoldsNumber;
        double lb_velocity;
        double lb_viscosity;

        double nups;
        std::string collisionKernel;

        uint numberOfTimeSteps;
        std::array<real, 3> quadricLimiters;
    };

    struct World
    {
        double length;
        double velocity;
    };

    struct Discretization
    {
        double dt;
        double dx;
        double totalNumberOfNodes;
        uint numberOfLevels;
        std::vector<int> numberOfNodesPerLevel;
    };

    struct BuildInfo
    {
        std::string git_commit_hash;
        std::string git_branch;
        std::string build_type;
        std::string remote;
        std::string compiler_flags;
        std::string precision;
        std::string compiler_definitions;
        std::string compiler;
        std::string compiler_version;
#ifdef VF_MPI
        std::string mpi_library;
        std::string mpi_version;
#endif
#ifdef VF_OPENMP
        std::string openmp_library;
        std::string openmp_version;
#endif
    };

    struct GPU
    {
        std::string name;
        std::string compute_capability;
    };

    std::string name {};

    uint numberOfProcesses {};
    uint numberOfThreads {};

    std::string vf_hardware {};

    Simulation simulation;
    World world;
    Discretization discretization;
    BuildInfo buildInfo;
    std::vector<GPU> gpus;
};

void logPreSimulation(const MetaData& meta_data);

void logPostSimulation(const MetaData& meta_data);

} // namespace vf::basics

#endif

//! \}

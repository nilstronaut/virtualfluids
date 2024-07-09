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
#include "YAML_MetaData.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

#include "MetaData.h"

namespace YAML
{

template <>
struct convert<vf::basics::MetaData::BuildInfo>
{
    static Node encode(const vf::basics::MetaData::BuildInfo& rhs)
    {
        Node root;
        root["git_commit_hash"] = rhs.git_commit_hash;
        root["git_branch"] = rhs.git_branch;
        root["build_type"] = rhs.build_type;
        root["remote"] = rhs.remote;
        root["precision"] = rhs.precision;
        root["compiler"] = rhs.compiler;
        root["compiler_version"] = rhs.compiler_version;
        root["compiler_flags"] = rhs.compiler_flags;
        root["compiler_definitions"] = rhs.compiler_definitions;
#ifdef VF_MPI
        root["mpi_library"] = rhs.mpi_library;
        root["mpi_version"] = rhs.mpi_version;
#endif
#ifdef VF_OPENMP
        root["openmp_library"] = rhs.openmp_library;
        root["openmp_version"] = rhs.openmp_version;
#endif
        return root;
    }
};

template <>
struct convert<vf::basics::MetaData::Discretization>
{
    static Node encode(const vf::basics::MetaData::Discretization& rhs)
    {
        Node root;
        root["dt"] = rhs.dt;
        root["dx"] = rhs.dx;
        root["totalNumberOfNodes"] = rhs.totalNumberOfNodes;
        root["numberOfLevels"] = rhs.numberOfLevels;

        Node node;
        for (const auto& nodes : rhs.numberOfNodesPerLevel)
            node.push_back(nodes);

        root["numberOfNodesPerLevel"] = node;

        return root;
    }
};

template <>
struct convert<vf::basics::MetaData::World>
{
    static Node encode(const vf::basics::MetaData::World& rhs)
    {
        Node root;
        root["length"] = rhs.length;
        root["velocity"] = rhs.velocity;
        return root;
    }
};

template <>
struct convert<vf::basics::MetaData::Simulation>
{
    static Node encode(const vf::basics::MetaData::Simulation& rhs)
    {
        Node root;
        root["startDateTime"] = rhs.startDateTime;
        root["runtimeSeconds"] = rhs.runtimeSeconds;
        root["nups"] = rhs.nups;
        root["numberOfTimeSteps"] = rhs.numberOfTimeSteps;

        root["collisionKernel"] = rhs.collisionKernel;
        root["reynoldsNumber"] = rhs.reynoldsNumber;
        root["lb_velocity"] = rhs.lb_velocity;
        root["lb_viscosity"] = rhs.lb_viscosity;
        root["quadricLimiter"] = rhs.quadricLimiters;

        return root;
    }
};

template <>
struct convert<vf::basics::MetaData::GPU>
{
    static Node encode(const vf::basics::MetaData::GPU& rhs)
    {
        Node root;
        root["name"] = rhs.name;
        root["compute_capability"] = rhs.compute_capability;
        return root;
    }
};

template <>
struct convert<vf::basics::MetaData>
{
    static Node encode(const vf::basics::MetaData& rhs)
    {
        Node root;
        root["name"] = rhs.name;
        root["number_of_processes"] = rhs.numberOfProcesses;
        root["number_of_threads"] = rhs.numberOfThreads;
        root["vf_hardware"] = rhs.vf_hardware;

        root["simulation"] = rhs.simulation;

        root["world"] = rhs.world;

        root["discretization"] = rhs.discretization;

        root["build_info"] = rhs.buildInfo;

        if (rhs.vf_hardware == "GPU") {
            Node node;
            for (const auto& nodes : rhs.gpus)
                node.push_back(nodes);

            root["gpus"] = node;
        }

        return root;
    }
};

} // namespace YAML

namespace vf::basics
{

void writeYAML(const MetaData& meta_data, const std::string& filename)
{
    YAML::Node node;
    node = meta_data;
    std::ofstream fout(filename);
    fout << node;
}

} // namespace vf::basics

//! \}

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

namespace buildInfo
{

const char* gitCommitHash()
{
    return "@git_commit_hash@";
}

const char* gitBranch()
{
    return "@git_branch@";
}

const char* buildType()
{
    return "@CMAKE_BUILD_TYPE@";
}

const char* compilerFlags()
{
    return "@BUILD_COMPILE_OPTIONS@";
}

const char* compilerFlagWarnings()
{
    return "@BUILD_COMPILE_WARNINGS@";
}

const char* compilerDefinitions()
{
    return "@BUILD_COMPILE_DEFINITIONS@";
}

const char* buildMachine()
{
    return "@BUILD_computerName@";
}

const char* projectDir()
{
    return "@CMAKE_SOURCE_DIR@";
}

const char* binaryDir()
{
    return "@CMAKE_BINARY_DIR@";
}

const char* precision()
{
    return "@BUILD_PRECISION@";
}

const char* compiler()
{
    return "@CMAKE_CXX_COMPILER_ID@";
}

const char* compiler_version()
{
    return "@CMAKE_CXX_COMPILER_VERSION@";
}

#ifdef VF_MPI
const char* mpi_library()
{
    return "@MPI_CXX_LIBRARIES@";
}

const char* mpi_version()
{
    return "@MPI_CXX_VERSION@";
}
#endif

#ifdef VF_OPENMP
const char* openmp_library()
{
    return "@OpenMP_CXX_LIBRARIES@";
}

const char* openmp_version()
{
    return "@OpenMP_CXX_VERSION@";
}
#endif

} // namespace buildInfo

//! \}

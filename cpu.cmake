# #######################################################################################
# ____          ____    __    ______     __________   __      __       __        __
# \    \       |    |  |  |  |   _   \  |___    ___| |  |    |  |     /  \      |  |
#  \    \      |    |  |  |  |  |_)   |     |  |     |  |    |  |    /    \     |  |
#   \    \     |    |  |  |  |   _   /      |  |     |  |    |  |   /  /\  \    |  |
#    \    \    |    |  |  |  |  | \  \      |  |     |   \__/   |  /  ____  \   |  |____
#     \    \   |    |  |__|  |__|  \__\     |__|      \________/  /__/    \__\  |_______|
#      \    \  |    |   ________________________________________________________________
#       \    \ |    |  |  ______________________________________________________________|
#        \    \|    |  |  |         __          __     __     __     ______      _______
#         \         |  |  |_____   |  |        |  |   |  |   |  |   |   _  \    /  _____)
#          \        |  |   _____|  |  |        |  |   |  |   |  |   |  | \  \   \_______
#           \       |  |  |        |  |_____   |   \_/   |   |  |   |  |_/  /    _____  |
#            \ _____|  |__|        |________|   \_______/    |__|   |______/    (_______/
#
#  This file is part of VirtualFluids. VirtualFluids is free software: you can
#  redistribute it and/or modify it under the terms of the GNU General Public
#  License as published by the Free Software Foundation, either version 3 of
#  the License, or (at your option) any later version.
#
#  VirtualFluids is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
#  for more details.
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#  SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder
#
# #################################################################################

#############################################################
###                  Options                              ###
#############################################################
SET(VF_CPU_ENABLE_VTK OFF CACHE BOOL "include VTK library support")
SET(VF_CP_ENABLE_CATALYST OFF CACHE BOOL "include Paraview Catalyst support")

IF(${VF_CPU_ENABLE_VTK})
    FIND_PACKAGE(VTK REQUIRED)
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
    target_compile_definitions(project_options INTERFACE VF_VTK)
ENDIF()

IF(${VF_CP_ENABLE_CATALYST})
    find_package(ParaView 4.3 REQUIRED COMPONENTS vtkPVPythonCatalyst)
    include("${PARAVIEW_USE_FILE}")
    target_compile_definitions(project_options INTERFACE VF_CATALYST)
ENDIF()

#############################################################
###                  Libraries                            ###
#############################################################

add_subdirectory(${VF_THIRD_DIR}/MuParser)

add_subdirectory(src/cpu/core)

if(VF_ENABLE_PYTHON_BINDINGS)
    add_subdirectory(src/cpu/simulationconfig)
endif()

#############################################################
###                  Apps                                 ###
#############################################################

add_subdirectory(apps/cpu/LidDrivenCavity)
add_subdirectory(apps/cpu/LaminarPlaneFlow)
add_subdirectory(apps/cpu/LaminarPipeFlow)


if(VF_ENABLE_BOOST)
    add_subdirectory(apps/cpu/GyroidsRow)
endif()

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

option(VF_GPU_ENABLE_NUMERIC_TESTS "Build numeric tests" OFF)

#############################################################
###                  Libraries                            ###
#############################################################

add_subdirectory(src/gpu/cuda_helper)
add_subdirectory(src/gpu/GridGenerator)
add_subdirectory(src/gpu/core)

#############################################################
###                      Apps                             ###
#############################################################

if(VF_ENABLE_ALL_APPS)
    list(APPEND USER_APPS
    "apps/gpu/DrivenCavityMultiGPU"
    "apps/gpu/AtmosphericBoundaryLayer"
    "apps/gpu/ActuatorLine"
    "apps/gpu/SphereMultiGPU" 
    "apps/gpu/TaylorGreenVortex"
    )
endif()

add_subdirectory(apps/gpu/DrivenCavity)
add_subdirectory(apps/gpu/SphereInChannel)
add_subdirectory(apps/gpu/LaminarPipeFlowGPU)

#############################################################
###                   Numeric Tests                       ###
#############################################################

if(VF_GPU_ENABLE_NUMERIC_TESTS)
    if(NOT VF_ENABLE_UNIT_TESTS) # in this case googletest is already included.
        add_subdirectory(${VF_THIRD_DIR}/googletest)
    endif()

    add_subdirectory(tests/numerical-tests/gpu/)
endif()

#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder
source ./tests/regression-tests/__regression_test_executer.sh


# 1. set reference data directory (must match the folder structure in https://git.rz.tu-bs.de/irmb/virtualfluids-reference-data)
REFERENCE_DATA_DIR=regression_tests/gpu/DrivenCavity_2Levels

# 2. set cmake flags for the build of VirtualFluids
CMAKE_FLAGS="--preset=make_gpu -DCMAKE_BUILD_TYPE=Release -DCMAKE_CUDA_ARCHITECTURES=75"

# 3. define the application to be executed
APPLICATION="./build/bin/DrivenCavity ./apps/gpu/DrivenCavity/drivencavity_2level.cfg"

# 4. set the path to the produced data
RESULT_DATA_DIR=output/DrivenCavity


run_regression_test "$REFERENCE_DATA_DIR" "$CMAKE_FLAGS" "$APPLICATION" "$RESULT_DATA_DIR"

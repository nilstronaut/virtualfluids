# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © VirtualFluids Project contributors, see AUTHORS.md in root folder

download_reference_data () {
    rm -rf reference_performance_tests && mkdir -p reference_performance_tests
    git clone --depth 1 --filter=blob:none --sparse https://git.rz.tu-bs.de/irmb/virtualfluids-reference-data reference_performance_tests
    cd reference_performance_tests
    git sparse-checkout add $1
    cd ..
}

# run performance test - arguments:
# 1. REFERENCE_DATA_DIR - to download the reference data and compare against
# 2. CMAKE_FLAGS - cmake flags for the build of VirtualFluids
# 3. APPLICATION - the application to be executed
# 4. RESULT_DATA_DIR - the path to the produced data to be compared
# 5. META_DATA_NAME - the name of the metadata file (.yaml)
run_performance_test () {
    download_reference_data $1

    rm -rf build && mkdir -p build
    cmake -B build $2
    cmake --build build --parallel 8

    # execute the application
    $3

    # execute comare_nups.py script
    python3 tests/performance-tests/compare_nups.py "$4/$5" "reference_performance_tests/$1/$5"

    # return exit code from compare_nups.py
    exit $?
}

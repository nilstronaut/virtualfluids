include (${CMAKE_SOURCE_DIR}/${cmakeMacroPath}/Cuda/Link.cmake)
linkCuda(${targetName})
include (${CMAKE_SOURCE_DIR}/${cmakeMacroPath}/OpenMP/Link.cmake)
linkOpenMP(${targetName})

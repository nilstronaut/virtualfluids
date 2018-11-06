include (${CMAKE_SOURCE_DIR}/${cmakeMacroPath}/Boost/Link.cmake)
linkBoost(${targetName} "thread;serialization")
include (${CMAKE_SOURCE_DIR}/${cmakeMacroPath}/MPI/Link.cmake)
linkMPI(${targetName})
include (${CMAKE_SOURCE_DIR}/${cmakeMacroPath}/JsonCpp/Link.cmake)
linkJsonCpp(${targetName})
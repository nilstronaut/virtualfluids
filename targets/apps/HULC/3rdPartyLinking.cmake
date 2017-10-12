include (${CMAKE_SOURCE_DIR}/${cmakeMacroPath}/MPI/Link.cmake)
linkMPI(${targetName})
include (${CMAKE_SOURCE_DIR}/${cmakeMacroPath}/Cuda/Link.cmake)
linkCuda(${targetName})

if(HULC.BUILD_JSONCPP)
  include (${CMAKE_SOURCE_DIR}/${cmakeMacroPath}/JsonCpp/Link.cmake)
  linkJsonCpp(${targetName})
endif()
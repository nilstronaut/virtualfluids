#Don't change:
SET(METIS_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/metis/metis-5.1.0 CACHE PATH "METIS ROOT")
#SET(GMOCK_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/googletest CACHE PATH "GMOCK ROOT")
SET(JSONCPP_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/jsoncpp CACHE PATH "JSONCPP ROOT")
SET(FFTW_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/fftw/fftw-3.3.7 CACHE PATH "JSONCPP ROOT")

SET(GMOCK_ROOT C:\\Libraries\\googletest-release-1.10.0 CACHE PATH "GMOCK ROOT")

#SET TO CORRECT PATH:
SET(BOOST_ROOT  "C:\\Libraries\\boost_1_65_1"  CACHE PATH "BOOST_ROOT")
SET(BOOST_LIBRARYDIR  "C:\\Libraries\\boost_1_65_1\\lib" CACHE PATH "BOOST_LIBRARYDIR")

#SET(VTK_DIR "F:/Libraries/vtk/VTK-8.2.0/build" CACHE PATH "VTK directory override" FORCE)

IF(${USE_METIS})
    SET(METIS_INCLUDEDIR "C:/Libraries/metis-5.1.0//include")
    SET(METIS_DEBUG_LIBRARY "C:/Libraries/metis-5.1.0/build/libmetis/Debug/metis.lib")
    SET(METIS_RELEASE_LIBRARY "C:/Libraries/metis-5.1.0/build/libmetis/Release/metis.lib")
ENDIF()
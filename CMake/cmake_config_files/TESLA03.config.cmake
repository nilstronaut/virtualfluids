#################################################################################
# VirtualFluids MACHINE FILE
# Responsible: Martin Schoenherr
# OS:          Windows 10
#################################################################################

#Don't change:
SET(METIS_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/metis/metis-5.1.0 CACHE PATH "METIS ROOT")
SET(GMOCK_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/googletest CACHE PATH "GMOCK ROOT")
SET(JSONCPP_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/jsoncpp CACHE PATH "JSONCPP ROOT")
SET(FFTW_ROOT ${CMAKE_SOURCE_DIR}/3rdParty/fftw/fftw-3.3.7 CACHE PATH "JSONCPP ROOT")


#SET TO CORRECT PATH:
SET(BOOST_ROOT  "C:\\Program Files\\boost\\boost_1_63_0"  CACHE PATH "BOOST_ROOT")
SET(BOOST_LIBRARYDIR  "C:\\Program Files\\boost\\boost_1_63_0\\stage\\x64\\lib" CACHE PATH "BOOST_LIBRARYDIR")

SET(VTK_DIR "F:/Libraries/vtk/VTK-8.2.0/build" CACHE PATH "VTK directory override" FORCE)

INCLUDE(${SOURCE_ROOT}/DemCoupling/CMakePackage.txt)
INCLUDE(${SOURCE_ROOT}/DemCoupling/physicsEngineAdapter/CMakePackage.txt)
INCLUDE(${SOURCE_ROOT}/DemCoupling/physicsEngineAdapter/dummy/CMakePackage.txt)
INCLUDE(${SOURCE_ROOT}/DemCoupling/physicsEngineAdapter/pe/CMakePackage.txt)
INCLUDE(${SOURCE_ROOT}/DemCoupling/reconstructor/CMakePackage.txt)

INCLUDE(${SOURCE_ROOT}/DemCoupling/IncludsList.cmake)

SET(LINK_LIBRARY optimized ${PE_RELEASE_LIBRARY} debug ${PE_DEBUG_LIBRARY})
SET(CAB_ADDITIONAL_LINK_LIBRARIES ${CAB_ADDITIONAL_LINK_LIBRARIES} ${LINK_LIBRARY})

SET(LINK_LIBRARY optimized ${BLOCKFOREST_RELEASE_LIBRARY} debug ${BLOCKFOREST_DEBUG_LIBRARY})
SET(CAB_ADDITIONAL_LINK_LIBRARIES ${CAB_ADDITIONAL_LINK_LIBRARIES} ${LINK_LIBRARY})

SET(LINK_LIBRARY optimized ${DOMAIN_DECOMPOSITION_RELEASE_LIBRARY} debug ${DOMAIN_DECOMPOSITION_DEBUG_LIBRARY})
SET(CAB_ADDITIONAL_LINK_LIBRARIES ${CAB_ADDITIONAL_LINK_LIBRARIES} ${LINK_LIBRARY})

SET(LINK_LIBRARY optimized ${CORE_RELEASE_LIBRARY} debug ${CORE_DEBUG_LIBRARY})
SET(CAB_ADDITIONAL_LINK_LIBRARIES ${CAB_ADDITIONAL_LINK_LIBRARIES} ${LINK_LIBRARY})

IF(${USE_GCC})
   SET(CAB_ADDITIONAL_LINK_LIBRARIES ${CAB_ADDITIONAL_LINK_LIBRARIES} "stdc++fs")
ENDIF()

IF(${USE_METIS})
   SET(LINK_LIBRARY optimized ${METIS_RELEASE_LIBRARY} debug ${METIS_DEBUG_LIBRARY})
   SET(CAB_ADDITIONAL_LINK_LIBRARIES ${CAB_ADDITIONAL_LINK_LIBRARIES} ${LINK_LIBRARY})
ENDIF()
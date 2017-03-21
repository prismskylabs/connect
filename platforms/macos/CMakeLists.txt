# Copyright (C) 2017 Prism Skylabs

include(../shared/shared.cmake)
findBoostCommon()
findCurlCommon()
setFlagsCommon()
buildSdk()

add_subdirectory(../shared/tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
add_subdirectory(../shared/examples ${CMAKE_CURRENT_BINARY_DIR}/examples)

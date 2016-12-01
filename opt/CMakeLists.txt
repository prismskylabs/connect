macro(set_cache_variable VAR_NAME VAR_DESCRIPTION)
    set(${VAR_NAME} ${${VAR_NAME}} CACHE INTERNAL ${VAR_DESCRIPTION})
    message(STATUS "Set ${VAR_NAME} to ${${VAR_NAME}}.")
endmacro()

set(JSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/json/src CACHE INTERNAL "")

set(BUILD_CPR_TESTS OFF CACHE BOOL "Don't build cpr tests")
set(USE_SYSTEM_CURL ON CACHE BOOL "Use system curl")

add_subdirectory(cpr)

add_subdirectory(fmt)
set(FMT_FOUND TRUE)
set(FMT_LIBRARIES fmt)
set(FMT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/fmt)

set_cache_variable(FMT_FOUND "Set if libfmt was found or built")
set_cache_variable(FMT_LIBRARIES "Location of libfmt")
set_cache_variable(FMT_INCLUDE_DIRS "Location of fmt include files")

set(BUILD_opencv_videoio 0 CACHE INTERNAL "")
add_subdirectory(opencv)
include(${PROJECT_BINARY_DIR}/OpenCVConfig.cmake)
set_cache_variable(OpenCV_LIBRARIES "Location of all opencv libraries")
set_cache_variable(OpenCV_INCLUDE_DIRS "Location of all opencv include directories")

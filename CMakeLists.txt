# Copyright (C) 2016-2017 Prism Skylabs
cmake_minimum_required (VERSION 2.8)
project (ConnectSDK)

include(${PROJECT_SOURCE_DIR}/version.cmake)
include(${PROJECT_SOURCE_DIR}/deps.cmake)

# configure header file to pass CMake settings to source file
configure_file (
    "${PROJECT_SOURCE_DIR}/ConnectSDKConfig.h.in"
    "${PROJECT_BINARY_DIR}/ConnectSDKConfig.h")

# add this path to enable finding *Config.h
include_directories("${PROJECT_BINARY_DIR}")

if (CMAKE_CROSSCOMPILING)
    if (NOT DEFINED PRISM_PLATFORM)
        message (FATAL_ERROR "When crosscompiling, PRISM_PLATFORM must be set")
    endif ()  
elseif (NOT DEFINED PRISM_PLATFORM)
    find_program (CMAKE_UNAME uname /bin /usr/bin /usr/local/bin)
    if (CMAKE_UNAME)
	    exec_program(uname OUTPUT_VARIABLE PRISM_HOST_OS_NAME)
	    if (PRISM_HOST_OS_NAME STREQUAL "Darwin")
	        set (PRISM_PLATFORM macos)
	    elseif (PRISM_HOST_OS_NAME STREQUAL "Linux")
                set (PRISM_PLATFORM xenial)
	    endif ()
    endif (CMAKE_UNAME)
  
    if (NOT DEFINED PRISM_PLATFORM)
        message (FATAL_ERROR "Unsupported OS")
    endif ()
endif ()

if (DEFINED ENV{PRISM_INSTALL_DIR})
    set (CMAKE_INSTALL_PREFIX $ENV{PRISM_INSTALL_DIR}/)
else ()
    set (CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/platforms/${PRISM_PLATFORM}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/platforms/${PRISM_PLATFORM}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/platforms/${PRISM_PLATFORM}/)
    
message ("Platform: ${PRISM_PLATFORM}")
message ("Install prefix: ${CMAKE_INSTALL_PREFIX}")

add_subdirectory(platforms/${PRISM_PLATFORM})

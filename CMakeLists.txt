# Copyright (C) 2016-2017 Prism Skylabs
cmake_minimum_required (VERSION 2.8)
project (ConnectSDK)

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
	        set (PRISM_PLATFORM linux)
	    endif ()
    endif (CMAKE_UNAME)
  
    if (NOT DEFINED PRISM_PLATFORM)
        message (FATAL_ERROR "Unsupported OS")
    endif ()
endif ()

if (DEFINED ENV{PRISM_INSTALL_DIR})
    set (CMAKE_INSTALL_PREFIX $ENV{PRISM_INSTALL_DIR}/${PRISM_PLATFORM})
else ()
    set (CMAKE_INSTALL_PREFIX $ENV{HOME}/prism/${PRISM_PLATFORM})
endif()
    
message ("Platform: ${PRISM_PLATFORM}")
message ("Install prefix: ${CMAKE_INSTALL_PREFIX}")

add_subdirectory(platforms/${PRISM_PLATFORM})


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

message ("Platform: ${PRISM_PLATFORM}")

function(buildSdk)
    message ("buildSdk()")
    message ("Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
    
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

    message(STATUS "CURL include dirs: ${CURL_INCLUDE_DIRS}")
    message(STATUS "CURL library dirs: ${CURL_LIBRARY_DIRS}")
    message(STATUS "CURL libraries: ${CURL_LIBRARIES}")

    set (CONNECT_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/ext/easylogging-8.91
        ${CMAKE_SOURCE_DIR}/ext/rapidjson-1.1.0/include
    )

    set (CONNECT_SOURCES
        ${CMAKE_SOURCE_DIR}/src/client.cpp
        ${CMAKE_SOURCE_DIR}/src/curl-session.cpp
        ${CMAKE_SOURCE_DIR}/src/domain-types.cpp
        ${CMAKE_SOURCE_DIR}/src/util.cpp
        ${CMAKE_SOURCE_DIR}/src/const-strings.cpp
    )

    include_directories(
        ${CONNECT_INCLUDE_DIRS}         
        ${Boost_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS})
        
    link_directories(
        ${Boost_LIBRARY_DIRS} 
        ${CURL_LIBRARY_DIRS})

    add_library(connect STATIC ${CONNECT_SOURCES})
    target_link_libraries(connect ${Boost_LIBRARIES} ${CURL_LIBRARIES})
endfunction()

add_subdirectory(platforms/${PRISM_PLATFORM})


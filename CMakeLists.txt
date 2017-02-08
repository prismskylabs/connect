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
    set (CMAKE_INSTALL_PREFIX $ENV{PRISM_INSTALL_DIR}/)
else ()
    set (CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/platforms/${PRISM_PLATFORM}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/platforms/${PRISM_PLATFORM}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/platforms/${PRISM_PLATFORM}/)
    
message ("Platform: ${PRISM_PLATFORM}")
message ("Install prefix: ${CMAKE_INSTALL_PREFIX}")

function(buildSdk)
    message ("Configuring Connect SDK...")
    message ("Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
    message ("Current library destination dir: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    
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
    
    install (TARGETS connect
	RUNTIME DESTINATION bin/platforms/${PRISM_PLATFORM}
	LIBRARY DESTINATION bin/platforms/${PRISM_PLATFORM}/lib
	ARCHIVE DESTINATION bin/platforms/${PRISM_PLATFORM}/lib)
             
    set (CONNECT_HEADERS         
        ${CMAKE_SOURCE_DIR}/include/common-types.h
        ${CMAKE_SOURCE_DIR}/include/domain-types.h
        ${CMAKE_SOURCE_DIR}/include/client.h)
          
    install (FILES ${CONNECT_HEADERS}
        DESTINATION include)
             
endfunction()

add_subdirectory(platforms/${PRISM_PLATFORM})


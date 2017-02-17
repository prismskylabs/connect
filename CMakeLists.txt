# Copyright (C) 2016-2017 Prism Skylabs
cmake_minimum_required (VERSION 2.8)
project (ConnectSDK)

set(ConnectSDK_VERSION_MAJOR 1)
set(ConnectSDK_VERSION_MINOR 0)
set(ConnectSDK_VERSION_REVISION 2)

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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/platforms/${PRISM_PLATFORM}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/platforms/${PRISM_PLATFORM}/)
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
    
    # this may be conditional, if necessary
    add_custom_command (
        TARGET connect
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install)

    # this may be conditional, if necessary
#    add_custom_command (
#        TARGET connect
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target delivery)

    install (TARGETS connect
	RUNTIME DESTINATION bin/platforms/${PRISM_PLATFORM}/
	LIBRARY DESTINATION bin/platforms/${PRISM_PLATFORM}/
	ARCHIVE DESTINATION bin/platforms/${PRISM_PLATFORM}/)
             
    set (CONNECT_HEADERS         
        ${CMAKE_SOURCE_DIR}/include/common-types.h
        ${CMAKE_SOURCE_DIR}/include/domain-types.h
        ${CMAKE_SOURCE_DIR}/include/client.h
        # util.h is internal header and shall not be exposed
        )
          
    install (FILES ${CONNECT_HEADERS}
        DESTINATION include)
             
    set (DELIVERY_FILES
        bin/platforms/${PRISM_PLATFORM}/
        include/)

    if (NOT CMAKE_BUILD_TYPE)
        set (CONNECT_BUILD_TYPE None)
    else ()
        set (CONNECT_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    endif ()

    if (DEFINED ENV{BUILD_NUMBER})
        set (CONNECT_BUILD_NUMBER $ENV{BUILD_NUMBER})
    else ()
        message (WARNING "BUILD_NUMBER env variable not set, using value 0 for build number")
        set (CONNECT_BUILD_NUMBER 0)
    endif ()

    execute_process(COMMAND printf %02d.%02d.%03d.%05d ${ConnectSDK_VERSION_MAJOR}
        ${ConnectSDK_VERSION_MINOR} ${ConnectSDK_VERSION_REVISION} ${CONNECT_BUILD_NUMBER}
        OUTPUT_VARIABLE SDK_VER)

    message ("SDK version incl. build number: ${SDK_VER}")

    execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE REV_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)

    message("git revision short hash: ${REV_HASH}")

    add_custom_target(delivery
        tar -czf ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}_${SDK_VER}_${REV_HASH}_${PRISM_PLATFORM}.${CONNECT_BUILD_TYPE}.tar.gz ${DELIVERY_FILES}
        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})

endfunction()

add_subdirectory(platforms/${PRISM_PLATFORM})


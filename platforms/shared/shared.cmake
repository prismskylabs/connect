# Copyright (C) 2017 Prism Skylabs
function(download_artifact repoUrl platform projectName version dstPath)

    set(archiveName artifact.tar.gz)

    message(STATUS "download_artifact: repoUrl: ${repoUrl}, platform: ${platform}")
    message(STATUS "download_artifact: projectName: ${projectName}, version: ${version}")
    message(STATUS "download_artifact: dstPath: ${dstPath}")

    #check if artifact already exists locally
    if(EXISTS ${dstPath}/${archiveName})
       message(STATUS "Artifact exists, skipping download")
       return()
    endif()

    message(STATUS "Making download URL for artifact ${repoUrl}, ${projectName}, ${platform}, ${version}")
    set(downloadUrl "${repoUrl}/${projectName}/${platform}/${projectName}_${version}_${platform}.Release.tar.gz")
    message(STATUS "Download URL: ${downloadUrl}")

    file(MAKE_DIRECTORY ${dstPath})

    file(DOWNLOAD ${downloadUrl} ${dstPath}/${archiveName}
        STATUS status
        SHOW_PROGRESS)

    list (GET status 0 retCode)
    list (GET status 1 retMessage)

    if (NOT "${retCode}" STREQUAL "0")
        message(FATAL_ERROR "Error downloading artifact: ${retCode} ${retMessage}")
        execute_process(COMMAND rm -rf ./* WORKING_DIRECTORY ${dstPath})
    endif()

    execute_process(
       COMMAND tar -xvzf ${archiveName}
       WORKING_DIRECTORY ${dstPath}
       RESULT_VARIABLE resultVar
       ERROR_VARIABLE errorVar
    )

    if (NOT "${resultVar}" STREQUAL "0")
        message(FATAL_ERROR "Error unpacking artifact archive: ${errorVar}")
        execute_process(COMMAND rm -rf ./* WORKING_DIRECTORY ${dstPath})
    endif()
endfunction()

macro(findBoostCommon)
    if (NOT DEFINED ENV{BOOST_ROOT})
        set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/ext/boost_${PRISM_PLATFORM}_${BOOST_VERSION})
        download_artifact("${ARTIFACTORY_URL}/libs-release-public" "${PRISM_PLATFORM}" boost "${BOOST_VERSION}" "${BOOST_ROOT}")
    else ()
        set(BOOST_ROOT $ENV{BOOST_ROOT})
    endif ()

    message (STATUS "BOOST_ROOT: ${BOOST_ROOT}")
    message (STATUS "BOOST_VERSION: ${BOOST_VERSION}")

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)

    set(BOOST_COMPONENTS atomic chrono date_time filesystem program_options regex system thread)
    find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS ${BOOST_COMPONENTS})
endmacro()

macro(findCurlCommon)
    find_package(CURL REQUIRED)
endmacro()

macro(setFlagsCommon)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${STD_VER} -Wextra -pedantic")
endmacro()

macro(findOpencvCommon)
    # the message will be removed as soon as EDGE-280 is fixed
    message(FATAL_ERROR "Not implemented")
    if (NOT DEFINED ENV{OPENCV_ROOT})
        set(OPENCV_ROOT ${PROJECT_SOURCE_DIR}/ext/opencv_${PRISM_PLATFORM}_${OPENCV_VERSION})
        download_artifact("${ARTIFACTORY_URL}/libs-release-public" "${PRISM_PLATFORM}" opencv "${OPENCV_VERSION}" "${OPENCV_ROOT}")
    else ()
        set(OPENCV_ROOT $ENV{OPENCV_ROOT})
    endif ()

    find_package(OpenCV ${OPENCV_VERSION} REQUIRED PATHS ${OPENCV_ROOT} NO_DEFAULT_PATH NO_CMAKE_SYSTEM_PATH )
endmacro()

function(buildSdk)
    message (STATUS "Configuring Connect SDK...")
    message (STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
    message (STATUS "Current library destination dir: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

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
        ${CMAKE_SOURCE_DIR}/ext/catch-1.9.1
    )

    set (CONNECT_SOURCES
        ${CMAKE_SOURCE_DIR}/src/client.cpp
        ${CMAKE_SOURCE_DIR}/src/curl-wrapper.cpp
        ${CMAKE_SOURCE_DIR}/src/curl-session.cpp
        ${CMAKE_SOURCE_DIR}/src/domain-types.cpp
        ${CMAKE_SOURCE_DIR}/src/util.cpp
        ${CMAKE_SOURCE_DIR}/src/const-strings.cpp
        ${CMAKE_SOURCE_DIR}/src/public-util.cpp
        ${CMAKE_SOURCE_DIR}/src/payload-holder.cpp
        ${CMAKE_SOURCE_DIR}/src/artifact-uploader.cpp
        ${CMAKE_SOURCE_DIR}/src/UploadArtifactTask.cpp
        ${CMAKE_SOURCE_DIR}/src/UploadQueue.cpp
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

    add_custom_target(
        install_wrapper
        COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target install)

    install (TARGETS connect
        RUNTIME DESTINATION bin/platforms/${PRISM_PLATFORM}/
        LIBRARY DESTINATION bin/platforms/${PRISM_PLATFORM}/
        ARCHIVE DESTINATION bin/platforms/${PRISM_PLATFORM}/)

    set (CONNECT_HEADERS
        ${CMAKE_SOURCE_DIR}/include/artifact-uploader.h
        ${CMAKE_SOURCE_DIR}/include/client.h
        ${CMAKE_SOURCE_DIR}/include/common-types.h
        ${CMAKE_SOURCE_DIR}/include/domain-types.h
        ${CMAKE_SOURCE_DIR}/include/payload-holder.h
        ${CMAKE_SOURCE_DIR}/include/public-util.h
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
        message ("BUILD_NUMBER env variable not set, using value 0 for build number")
        set (CONNECT_BUILD_NUMBER 0)
    endif ()

    execute_process(COMMAND printf %02d.%02d.%03d.%05d ${ConnectSDK_VERSION_MAJOR}
        ${ConnectSDK_VERSION_MINOR} ${ConnectSDK_VERSION_REVISION} ${CONNECT_BUILD_NUMBER}
        OUTPUT_VARIABLE SDK_VER
        RESULT_VARIABLE RET_CODE)

    if (NOT "${RET_CODE}" STREQUAL "0")
        message(FATAL_ERROR "Error printing SDK version.")
    endif()

    message (STATUS "SDK version incl. build number: ${SDK_VER}")

    execute_process(COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE REV_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE RET_CODE)

    if (NOT "${RET_CODE}" STREQUAL "0")
        message(FATAL_ERROR "Error retrieving revision hash from git.")
    endif()

    message(STATUS "git revision short hash: ${REV_HASH}")

    set(DELIVERY_NAME ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}_${SDK_VER}_${REV_HASH}_cpp${STD_VER}_${PRISM_PLATFORM}.${CONNECT_BUILD_TYPE}.tar.gz)

    # consider using add_custom_command() to create archive only, when dependency changed
    add_custom_target(delivery
        COMMAND tar -czf ${DELIVERY_NAME} ${DELIVERY_FILES}
        DEPENDS install_wrapper
        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})

    add_custom_target(upload_artifactory
        COMMAND jfrog rt u ${DELIVERY_NAME} libs-release-public/${PROJECT_NAME}/${PRISM_PLATFORM}/
        DEPENDS delivery
        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})

endfunction()

include(../shared/shared.cmake)
include(deps.cmake)

macro(findBoost)
    if (NOT DEFINED ENV{BOOST_ROOT})
        set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/ext/boost_${PRISM_PLATFORM}_${BOOST_VERSION})
        download_artifact("${ARTIFACTORY_URL}/libs-release-public" "${PRISM_PLATFORM}" boost "${BOOST_VERSION}" "${BOOST_ROOT}")
    else ()
        set(BOOST_ROOT $ENV{BOOST_ROOT})
    endif ()

    message (STATUS "BOOST_ROOT: ${BOOST_ROOT}")
    message (STATUS "BOOST_VERSION: ${BOOST_VERSION}")

    set (Boost_LIBRARIES ${BOOST_ROOT}/boost.framework)
endmacro()

macro(findOpenCV)
    if (NOT DEFINED ENV{OPENCV_ROOT})
            set(OPENCV_ROOT ${PROJECT_SOURCE_DIR}/ext/opencv_${PRISM_PLATFORM}_${OPENCV_VERSION})
            download_artifact("${ARTIFACTORY_URL}/libs-release-public" "${PRISM_PLATFORM}" opencv "${OPENCV_VERSION}" "${OPENCV_ROOT}")
    else ()
            set(OPENCV_ROOT $ENV{OPENCV_ROOT})
    endif ()

    message (STATUS "OPENCV_ROOT: ${OPENCV_ROOT}")
    message (STATUS "OPENCV_VERSION: ${OPENCV_VERSION}")

    set (OpenCV_LIBRARIES ${OPENCV_ROOT}/opencv2.framework)
endmacro()

macro(findCurl)
    if (NOT DEFINED ENV{CURL_ROOT})
        set(CURL_ROOT ${PROJECT_SOURCE_DIR}/ext/curl_${PRISM_PLATFORM}_${CURL_VERSION})
        download_artifact("${ARTIFACTORY_URL}/libs-release-public" "${PRISM_PLATFORM}" curl "${CURL_VERSION}" "${CURL_ROOT}")
    else ()
        set(CURL_ROOT $ENV{CURL_ROOT})
    endif ()

    message (STATUS "CURL_ROOT: ${CURL_ROOT}")
    message (STATUS "CURL_VERSION: ${CURL_VERSION}")

    set (CURL_INCLUDE_DIRS ${CURL_ROOT}/curl/include)
    set (CURL_LIBRARY_DIRS ${CURL_ROOT}/curl/lib)
    set (CURL_LIBRARIES "curl")
endmacro()

findBoost()
findOpenCV()
findCurl()
setFlagsCommon()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=10.0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=10.0")

if (${SIMULATOR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mios-simulator-version-min=10.0")
endif ()

buildSdk()

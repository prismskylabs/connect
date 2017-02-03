if (NOT DEFINED ENV{CONNECT_PREBUILT_ROOT})
    message (FATAL_ERROR "Please, define CONNECT_PREBUILT_ROOT environment variable")
endif ()

set (BOOST_IOS_ROOT $ENV{CONNECT_PREBUILT_ROOT}/boost)
set (Boost_INCLUDE_DIRS ${BOOST_IOS_ROOT}/include)
set (Boost_LIBRARY_DIRS ${BOOST_IOS_ROOT}/lib)
set (Boost_LIBRARIES "boost")

set (CURL_IOS_ROOT $ENV{CONNECT_PREBUILT_ROOT}/curl)
set (CURL_INCLUDE_DIRS ${CURL_IOS_ROOT}/include)
set (CURL_LIBRARY_DIRS ${CURL_IOS_ROOT}/lib)
set (CURL_LIBRARIES "curl")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++98 -stdlib=libc++ -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=9.0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT} -miphoneos-version-min=9.0")

if (${SIMULATOR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mios-simulator-version-min=9.0")
endif ()

buildSdk()


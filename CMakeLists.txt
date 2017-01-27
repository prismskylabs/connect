# Copyright (C) 2016-2017 Prism Skylabs
cmake_minimum_required (VERSION 2.8)
project (ConnectSDK)

if (DEFINED IOS_PLATFORM)

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

else ()

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)

    find_package(Boost 1.60.0 REQUIRED COMPONENTS chrono)

    if (NOT Boost_FOUND)
        if (NOT DEFINED ENV{BOOST_ROOT})
            message (FATAL_ERROR "Please, define BOOST_ROOT environment variable")
        endif ()

        set (BOOST_ROOT $ENV{BOOST_ROOT})

        find_package(Boost 1.60.0 REQUIRED COMPONENTS chrono)
    endif()

    find_package(CURL REQUIRED)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")

endif (DEFINED IOS_PLATFORM)

message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

message(STATUS "CURL include dirs: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL library dirs: ${CURL_LIBRARY_DIRS}")
message(STATUS "CURL libraries: ${CURL_LIBRARIES}")

set (CONNECT_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/ext/easylogging-8.91
    ${CMAKE_SOURCE_DIR}/ext/rapidjson-1.1.0/include
)

set (CONNECT_SOURCES
    src/client.cpp
    src/curl-session.cpp
    src/domain-types.cpp
    src/util.cpp
    src/const-strings.cpp
)

include_directories(${CONNECT_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS} ${CURL_LIBRARY_DIRS})

add_library(connect STATIC ${CONNECT_SOURCES})
target_link_libraries(connect ${Boost_LIBRARIES} ${CURL_LIBRARIES})

if (NOT DEFINED IOS_PLATFORM)
    add_subdirectory(examples)
    add_subdirectory(tests)
endif (NOT DEFINED IOS_PLATFORM)

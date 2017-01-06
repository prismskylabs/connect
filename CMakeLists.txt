# Copyright (C) 2016-2017 Prism Skylabs
cmake_minimum_required (VERSION 2.8)
project (ConnectSDK)

set (CONNECT_SOURCES
    src/client.cpp
    src/curl-session.cpp
    src/domain-types.cpp
    src/util.cpp
    src/const-strings.cpp
)

find_package(Boost REQUIRED)

message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

find_package(CURL REQUIRED)

message(STATUS "CURL include dirs: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL library dirs: ${CURL_LIBRARY_DIRS}")
message(STATUS "CURL libraries: ${CURL_LIBRARIES}")

set (CONNECT_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/ext/easylogging-8.91
    ${CMAKE_SOURCE_DIR}/ext/rapidjson-1.1.0/include
)

include_directories(${CONNECT_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_library(connect STATIC ${CONNECT_SOURCES})

set (CONNECT_LIBS
    boost_chrono
    curl
)

target_link_libraries(connect ${CONNECT_LIBS})

add_subdirectory(examples)

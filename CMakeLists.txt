cmake_minimum_required (VERSION 2.8)
project (ConnectSDK)

set (CONNECT_SOURCES
    src/client.cpp
    src/curl-session.cpp
    src/domain-types.cpp
    src/util.cpp
    examples/yt_camera/main.cpp
)

# manually set paths to Boost include and lib folders
# using BOOST_ROOT environment variable
#set (Boost_INCLUDE_DIRS $ENV{BOOST_ROOT}/include)
#set (Boost_LIBRARY_DIRS $ENV{BOOST_ROOT}/lib)
#set (Boost_LIBRARIES)

find_package(Boost REQUIRED)

message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")

# Boost_LIBRARIES is empty
#message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

find_package(CURL REQUIRED)

message(STATUS "CURL include dirs: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL libraries: ${CURL_LIBRARIES}")

set (CONNECT_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${CONNECT_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/ext/easylogging-8.91
    ${CMAKE_SOURCE_DIR}/ext/rapidjson-1.1.0/include
)

include_directories(${CONNECT_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

#add_library(connect STATIC ${CONNECT_SOURCES})
add_executable(connect ${CONNECT_SOURCES})

set (CONNECT_LIBS
    boost_system
    curl
)

target_link_libraries(connect ${CONNECT_LIBS})


cmake_minimum_required (VERSION 2.8)
project (ConnectSDK)

set (CONNECT_SOURCES
    src/connect.cpp
)

set (CONNECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# manually set paths to Boost include and lib folders
# using BOOST_ROOT environment variable
set (Boost_INCLUDE_DIRS $ENV{BOOST_ROOT}/include)
set (Boost_LIBRARY_DIRS $ENV{BOOST_ROOT}/lib)
set (Boost_LIBRARIES)

message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")

include_directories(${Boost_INCLUDE_DIRS} ${CONNECT_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_library(connect STATIC ${CONNECT_SOURCES})

target_link_libraries(connect ${Boost_LIBRARIES})


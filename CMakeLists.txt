cmake_minimum_required(VERSION 3.2)

project(connect)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(CONNECT_LIBRARIES connect CACHE INTERNAL "Prism Connect Library")
set(CONNECT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Prism Connect Includes")

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.token)
    message(FATAL_ERROR "Please make sure you have a file named .token in the root directory with your API Token")
endif()

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/.token API_TOKEN)
string(STRIP ${API_TOKEN} API_TOKEN)
add_definitions(-DAPI_TOKEN="${API_TOKEN}")
add_definitions(-DAPI_ROOT="https://connect.prism.com/v1/")

add_subdirectory(opt)
add_subdirectory(src)

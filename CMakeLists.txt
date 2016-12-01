cmake_minimum_required(VERSION 3.2)

project(connect)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(CONNECT_LIBRARIES connect CACHE INTERNAL "Prism Connect Library")
set(CONNECT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Prism Connect Includes")

add_subdirectory(opt)
add_subdirectory(src)
add_subdirectory(examples)

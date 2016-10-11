include_directories(
    ${CONNECT_INCLUDE_DIRS}
    ${CPR_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS})

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/api/*.cpp)
add_library(${CONNECT_LIBRARIES} ${SOURCES})

target_link_libraries(${CONNECT_LIBRARIES} ${CPR_LIBRARIES})

add_subdirectory(executables)

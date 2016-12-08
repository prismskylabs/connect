find_package( OpenCV 3 REQUIRED )

include_directories(
    ${CONNECT_INCLUDE_DIRS}
    ${CPR_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS}
    ${FMT_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS})

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/api/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cpp)

add_library(${CONNECT_LIBRARIES}
    ${SOURCES})

target_link_libraries(${CONNECT_LIBRARIES}
    ${CPR_LIBRARIES}
    ${FMT_LIBRARIES}
    ${OpenCV_LIBRARIES})

add_subdirectory(executables)

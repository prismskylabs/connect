find_package(OpenCV 3.1 REQUIRED)

include_directories(
    ${CONNECT_INCLUDE_DIRS}
    ${CPR_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS})
add_executable(yt_camera src/main.cpp)
target_link_libraries(yt_camera ${CONNECT_LIBRARIES} ${OpenCV_LIBRARIES})

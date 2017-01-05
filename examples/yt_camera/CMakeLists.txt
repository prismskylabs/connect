# Copyright (C) 2016-2017 Prism Skylabs
find_package(OpenCV 3.1 REQUIRED)

include_directories(
    ${CONNECT_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

add_executable(yt_camera main.cpp)
target_link_libraries(yt_camera connect ${OpenCV_LIBRARIES})

# Copyright (C) 2016-2017 Prism Skylabs
add_subdirectory(yt_camera)

add_executable(basic-app basic-app.cpp)
target_link_libraries(basic-app connect)


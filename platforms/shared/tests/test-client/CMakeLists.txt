# Copyright (C) 2016-2017 Prism Skylabs
find_package(OpenCV 3.1 REQUIRED)

include_directories(
    ${CONNECT_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

set (TEST_CLIENT_LIBS
    ${OpenCV_LIBRARIES}
    connect
    ${Boost_LIBRARIES}
)

add_executable(test-client main.cpp testArtifactUploader.cpp testUtils.cpp)
target_link_libraries(test-client ${TEST_CLIENT_LIBS})

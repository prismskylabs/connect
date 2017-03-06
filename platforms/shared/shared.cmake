# Copyright (C) 2017 Prism Skylabs
macro(findBoostCurl)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)

    find_package(Boost 1.60.0 REQUIRED COMPONENTS chrono)

    if (NOT Boost_FOUND)
        if (NOT DEFINED ENV{BOOST_ROOT})
            message (FATAL_ERROR "Please, define BOOST_ROOT environment variable")
        endif ()

        set (BOOST_ROOT $ENV{BOOST_ROOT})

        find_package(Boost 1.60.0 REQUIRED COMPONENTS chrono)
    endif()

    find_package(CURL REQUIRED)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
endmacro()

function(buildSdk)
    message ("Configuring Connect SDK...")
    message ("Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
    message ("Current library destination dir: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

    message(STATUS "CURL include dirs: ${CURL_INCLUDE_DIRS}")
    message(STATUS "CURL library dirs: ${CURL_LIBRARY_DIRS}")
    message(STATUS "CURL libraries: ${CURL_LIBRARIES}")

    set (CONNECT_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/ext/easylogging-8.91
        ${CMAKE_SOURCE_DIR}/ext/rapidjson-1.1.0/include
    )

    set (CONNECT_SOURCES
        ${CMAKE_SOURCE_DIR}/src/client.cpp
        ${CMAKE_SOURCE_DIR}/src/curl-session.cpp
        ${CMAKE_SOURCE_DIR}/src/domain-types.cpp
        ${CMAKE_SOURCE_DIR}/src/util.cpp
        ${CMAKE_SOURCE_DIR}/src/const-strings.cpp
    )

    include_directories(
        ${CONNECT_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS})

    link_directories(
        ${Boost_LIBRARY_DIRS}
        ${CURL_LIBRARY_DIRS})

    add_library(connect STATIC ${CONNECT_SOURCES})
    target_link_libraries(connect ${Boost_LIBRARIES} ${CURL_LIBRARIES})

    add_custom_target(
        install_wrapper
        COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target install)

    install (TARGETS connect
        RUNTIME DESTINATION bin/platforms/${PRISM_PLATFORM}/
        LIBRARY DESTINATION bin/platforms/${PRISM_PLATFORM}/
        ARCHIVE DESTINATION bin/platforms/${PRISM_PLATFORM}/)

    set (CONNECT_HEADERS
        ${CMAKE_SOURCE_DIR}/include/common-types.h
        ${CMAKE_SOURCE_DIR}/include/domain-types.h
        ${CMAKE_SOURCE_DIR}/include/client.h
        # util.h is internal header and shall not be exposed
        )

    install (FILES ${CONNECT_HEADERS}
        DESTINATION include)

    set (DELIVERY_FILES
        bin/platforms/${PRISM_PLATFORM}/
        include/)

    if (NOT CMAKE_BUILD_TYPE)
        set (CONNECT_BUILD_TYPE None)
    else ()
        set (CONNECT_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    endif ()

    if (DEFINED ENV{BUILD_NUMBER})
        set (CONNECT_BUILD_NUMBER $ENV{BUILD_NUMBER})
    else ()
        message (WARNING "BUILD_NUMBER env variable not set, using value 0 for build number")
        set (CONNECT_BUILD_NUMBER 0)
    endif ()

    execute_process(COMMAND printf %02d.%02d.%03d.%05d ${ConnectSDK_VERSION_MAJOR}
        ${ConnectSDK_VERSION_MINOR} ${ConnectSDK_VERSION_REVISION} ${CONNECT_BUILD_NUMBER}
        OUTPUT_VARIABLE SDK_VER
        RESULT_VARIABLE RET_CODE)

    if (NOT "${RET_CODE}" STREQUAL "0")
        message(FATAL_ERROR "Error printing SDK version.")
    endif()

    message ("SDK version incl. build number: ${SDK_VER}")

    execute_process(COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE REV_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE RET_CODE)

        if (NOT "${RET_CODE}" STREQUAL "0")
            message(FATAL_ERROR "Error retrieving revision hash from git.")
        endif()

    message("git revision short hash: ${REV_HASH}")

    set(DELIVERY_NAME ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}_${SDK_VER}_${REV_HASH}_${PRISM_PLATFORM}.${CONNECT_BUILD_TYPE}.tar.gz)

    # consider using add_custom_command() to create archive only, when dependency changed
    add_custom_target(delivery
        COMMAND tar -czf ${DELIVERY_NAME} ${DELIVERY_FILES}
        DEPENDS install_wrapper
        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})

    add_custom_target(upload_artifactory
        COMMAND jfrog rt u ${DELIVERY_NAME} libs-release-public/${PROJECT_NAME}/${PRISM_PLATFORM}/
        DEPENDS delivery
        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})

endfunction()

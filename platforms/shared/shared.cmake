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


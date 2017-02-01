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
    message ("Building Connect SDK...")
    message ("Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
    
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
    
    install (TARGETS connect
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
             
    set (CONNECT_HEADERS         
        ${CMAKE_SOURCE_DIR}/include/common-types.h
        ${CMAKE_SOURCE_DIR}/include/domain-types.h
        ${CMAKE_SOURCE_DIR}/include/client.h)
          
    install (FILES ${CONNECT_HEADERS}
        DESTINATION include)
             
endfunction()


include(ExternalProject)
include(ProcessorCount)
include(../shared/shared.cmake)

set(BOOST_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/src/boost)

file(WRITE ${BOOST_BUILD_PATH}/user-config.jam "using gcc : arm : \"$ENV{WN}/arm-linux-gnueabi-g++\"")

set (BOOST_ARCH_NAME boost_1_63_0.tar.bz2)
set (BOOST_URL https://www.dropbox.com/s/diobark5uu1kg78/${BOOST_ARCH_NAME})
set (DOWNLOADS_DIR "${CMAKE_SOURCE_DIR}/downloads")
set (BOOST_ARCH_FILE "${DOWNLOADS_DIR}/${BOOST_ARCH_NAME}")

if (NOT EXISTS ${BOOST_ARCH_FILE})
    file (DOWNLOAD ${BOOST_URL} ${BOOST_ARCH_FILE} SHOW_PROGRESS)
    
    if (NOT EXISTS ${BOOST_ARCH_FILE})
        message (FATAL_ERROR "Failed to download ${BOOST_URL} and save as ${BOOST_ARCH_FILE}")
    endif ()
endif ()

ProcessorCount(N)

if(N EQUAL 0)
    set (N 1)
endif()

message ("CPU count: ${N}")

ExternalProject_Add(boost
    URL ${BOOST_ARCH_FILE}
    URL_HASH SHA1=9f1dd4fa364a3e3156a77dc17aa562ef06404ff6
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ./bootstrap.sh
    BUILD_COMMAND ./b2 install
        --with-chrono
        --prefix=${CMAKE_INSTALL_PREFIX}
        -j${N}
    INSTALL_COMMAND ""
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
)

# find_package() doesn't work here, as Boost isn't yet built,
# so setting paths manually

set (Boost_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
set (Boost_LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/lib)

buildSdk()

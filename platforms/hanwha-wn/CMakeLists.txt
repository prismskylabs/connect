include(ExternalProject)

set(BOOST_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/src/boost)

file(WRITE ${BOOST_BUILD_PATH}/user-config.jam "using gcc : arm : \"$ENV{WN}/arm-linux-gnueabi-g++\"")

ExternalProject_Add(boost
    URL https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.bz2
    URL_HASH SHA1=9f1dd4fa364a3e3156a77dc17aa562ef06404ff6
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ./bootstrap.sh
    BUILD_COMMAND ./b2 install
        --without-python
        --without-mpi
        --disable-icu
        --prefix=${CMAKE_CURRENT_BINARY_DIR}/install
        -j8
    INSTALL_COMMAND ""
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install
)

# find_package() doesn't work here, as Boost isn't yet built,
# so setting paths manually

set (Boost_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/install/include)
set (Boost_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/install/lib)

buildSdk()

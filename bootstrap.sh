#!/usr/bin/env bash


# add repository for g++-5
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test

# add repository for clang-3.7
sudo add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main"
wget --quiet -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -

# add repository for git 2.7.2
sudo add-apt-repository ppa:git-core/ppa

# add repository for cmake 3.2.2
sudo add-apt-repository -y ppa:george-edison55/cmake-3.x

# update apt
sudo apt-get update -y

# install compilers
sudo apt-get install -y g++-5
sudo apt-get install -y clang-3.7

# install additional tools
sudo apt-get install -y cmake
sudo apt-get install -y clang-format-3.7
sudo apt-get install -y lldb-3.7

# required for building certain Boost libraries
sudo apt-get install -y libbz2-dev
sudo apt-get install -y python-dev

# install valgrind 3.10.1
sudo apt-get install -y valgrind

# install git
sudo apt-get install -y git

# install libssl and libssh2-1
sudo apt-get install -y libssl-dev
sudo apt-get install -y libssh2-1-dev

# set up symlink to gcc and clang toolchains
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 20
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 20
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-3.7 20
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-3.7 20
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-3.7 20

# install opencv 2.4
sudo apt-get install -y libopencv-dev

# redo symlinks if they became stale
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 20
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 20
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-3.7 20
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-3.7 20
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-3.7 20

# download, build, and install boost
if [ ! -d /usr/local/include/boost ]
then
    readonly BOOST_MINOR_VERSION=60
    readonly BOOST_VERSION=1.${BOOST_MINOR_VERSION}.0
    readonly BOOST_DIRECTORY=boost_1_${BOOST_MINOR_VERSION}_0
    readonly BOOST_TAR=${BOOST_DIRECTORY}.tar.bz2
    wget --quiet https://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/${BOOST_TAR}
    tar --bzip2 -xvf ${BOOST_TAR}
    cd ${BOOST_DIRECTORY}
    ./bootstrap.sh
    # installs Boost libraries to /usr/local/lib and headers to /usr/local/include
    sudo ./b2 install
    cd /home/vagrant
    sudo rm -rf ${BOOST_DIRECTORY}
    rm ${BOOST_TAR}
fi

# flex our new powers
cmake --version
g++ --version
clang++ --version
clang-format --version
valgrind --version
git --version

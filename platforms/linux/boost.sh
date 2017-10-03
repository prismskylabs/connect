#!/bin/bash
# Copyright (C) 2017 Prism Skylabs
#
# Builds Boost libraries for Ubuntu 14.04.
# Place this script into boost folder aquired from
# https://sourceforge.net/projects/boost/files/boost/{BOOST_VERSION}

./bootstrap.sh --without-libraries=python
./b2 -j 8 cxxflags="-std=c++11" link=shared,static threading=multi stage

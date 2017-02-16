#!/bin/bash
# Build and make delivery package script
# Parameters:
#	-t|--type=Debug|Release specify build type
# 	--delivery              create delivery package

# uncomment for debugging or call with "bash -uvx build.sh <build-params>"
# set -uvx
set -e

BUILD_DIR="./build"
BUILD_TYPE=Release
NJOBS="$(getconf _NPROCESSORS_ONLN)"
MAKE_DELIVERY=0
PRC_CMAKE_EXTRA_FLAGS=""

parse_cmd_line(){
    for i in "$@"
    do
    case $i in
        -t=*|--type=*)
        BUILD_TYPE="${i#*=}";;
        --delivery)
        MAKE_DELIVERY=1;;
        --debug)
        PRC_CMAKE_EXTRA_FLAGS="-DCMAKE_RULE_MESSAGES:BOOL=OFF -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON";;
        *)
        ;;
    esac
    done
}

parse_cmd_line $@

# generate make file
cmake $PRC_CMAKE_EXTRA_FLAGS -B$BUILD_DIR -H. "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

# build
cmake --build $BUILD_DIR -- -j$NJOBS --no-print-directory

if [ -z ${TEAMCITY_VERSION+x} ]; then
    # no-op
    :
else
    TAG=$(git describe --exact-match HEAD 2>/dev/null || :)

    if [ "${TAG}" ]; then
        MAKE_DELIVERY=1
    fi
fi

echo "Make delivery: ${MAKE_DELIVERY}"

# archive package
if (( ${MAKE_DELIVERY} != 0 )); then
    cmake --build $BUILD_DIR --target delivery -- --no-print-directory
fi

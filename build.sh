#!/bin/bash
# Build and make delivery package script
# Parameters:
#	-t|--type=Debug|Release specify build type
# 	--delivery              create delivery package

set -evx

BUILD_DIR="./build"
BUILD_TYPE=Release
NJOBS="$(getconf _NPROCESSORS_ONLN)"
MAKE_DELIVERY=0

parse_cmd_line(){
    for i in "$@"
    do
    case $i in
        -t=*|--type=*)
        BUILD_TYPE="${i#*=}";;
        --delivery)
        MAKE_DELIVERY=1;;
        *)
        ;;
    esac
    done
}

parse_cmd_line $@

# generate make file
cmake -DCMAKE_RULE_MESSAGES:BOOL=OFF -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -B$BUILD_DIR -H. "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

# build
cmake --build $BUILD_DIR -- -j$NJOBS

if [ -z ${TEAMCITY_VERSION+x} ]; then
    # no-op
    :
else
    TAG=`git describe --exact-match HEAD 2>/dev/null`

    if [ "${TAG}" ]; then
        MAKE_DELIVERY=1
    fi
fi

echo "Make delivery: ${MAKE_DELIVERY}"

# archive package
if (( ${MAKE_DELIVERY} != 0 )); then
    cmake --build $BUILD_DIR --target delivery;
fi

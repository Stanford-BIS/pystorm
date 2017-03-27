#!/bin/bash

# Usage: build_driver.sh [-d] [-c] [-b <relative_path]
#
# -b <relative_path>    relative path build directory (default is build/)
# -c                    run 'make clean' first
# -d                    delete build dir before running make

CWD=$(pwd)
BUILD_DIR=$(pwd)/build
CLEAN=FALSE
DEL_DIR=FALSE


ARGS=`getopt -o dcb: -n 'build_driver.sh' -- "$@"`
eval set -- "$ARGS"

while true; do
    case "$1" in
        -c)
            CLEAN=TRUE
            shift
            ;;
        -d)
            DEL_DIR=TRUE
            shift
            ;;
        -b)
            case "$2" in
                "")
                    shift 2
                    ;;
                *)
                    BUILD_DIR=$(pwd)/$2
                    shift 2
                    ;;
            esac
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Error"
            exit 1
            ;;
    esac
done

# delete the directory if it exists
if [ "${DEL_DIR}" == "TRUE" ]; then
    rm -rf ${BUILD_DIR}
fi

# clean the directory if it exists
if [ "${CLEAN}" == "TRUE" ]; then
    if [ -d ${BUILD_DIR} ]; then
        cd ${BUILD_DIR}
        make clean
        cd ${CWD}
    fi
fi

# if the directory does not exist, create it and run cmake ..
if [ ! -d ${BUILD_DIR} ]; then
    mkdir ${BUILD_DIR}
fi

# The script will always cd to the build dir and run cmake .. followed
# by make
cd ${BUILD_DIR}
cmake ..
make
cd ${CWD}

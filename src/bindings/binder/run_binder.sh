#!/bin/bash

BINDER_ROOT="/mnt/f/dev/binder/build/llvm-4.0/build_release_40.linux.Nazgul.release/bin"
PRJ_HOME=" /mnt/f/Studio/pystorm"
CONFIG_FILE="${PRJ_HOME}/src/bindings/binder/binder.config"
BIND_HEADER="${PRJ_HOME}/src/bindings/binder/driver_binder.h"

${BINDER_ROOT}/binder -max-file-size 1000000 --root-module BDDriver --prefix \
${PRJ_HOME}/src/bindings/python/3.5/ --config ${CONFIG_FILE} --single-file \
${BIND_HEADER} -- -x c++ -std=c++14 -I${PRJ_HOME}/src/bddriver \
-I${PRJ_HOME}src/bddriver/common -DNDEBUG
#!/bin/bash
# script for running ctest in release mode
# usage: run_docker <docker image name> <cofiguration>
#   Arguments:
#     docker image : name of docker image
#     configuration : <[Release, Debug]> whether to run in release or debug configuration

HOSTIP=$(ip route show | awk '/default/ {print $3}')
# Timeout after 5 minutes, in case of deadlocks
ssh $HOSTIP "docker run --rm -i ${1} ctest -C ${2} -j6 -T test -VV --timeout 300"

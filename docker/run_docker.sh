#!/bin/bash

HOSTIP=$(ip route show | awk '/default/ {print $3}')
# Timeout after 5 minutes, in case of deadlocks
ssh $HOSTIP "docker run --rm -i $1 ctest -C Debug -j6 -T test -VV --timeout 300"
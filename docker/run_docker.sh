#!/bin/bash

HOSTIP=$(ip route show | awk '/default/ {print $3}')
ssh $HOSTIP "docker run --rm -i $1 ctest -j -T test -VV"

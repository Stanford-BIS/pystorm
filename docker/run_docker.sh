#!/bin/bash

HOSTIP=$(ip route show | awk '/default/ {print $3}')
ssh $HOSTIP "docker run --rm -i -t $1 ninja test -v"

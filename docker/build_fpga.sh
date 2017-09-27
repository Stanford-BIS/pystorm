#!/bin/bash

HOSTIP=$(ip route show | awk '/default/ {print $3}')
ssh $HOSTIP "docker run --rm -i -v /home/quartus:/home/quartus -v $1:/artifacts $2"
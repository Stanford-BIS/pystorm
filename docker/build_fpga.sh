#!/bin/bash

HOSTIP=$(ip route show | awk '/default/ {print $3}')
JENKINS_HOST_PATH=$(echo "$1" | sed -e 's/^\/var/\/home\/jenkins/g')
ssh $HOSTIP "docker run --rm -i -v /home/quartus:/home/quartus -v ${JENKINS_HOST_PATH}:/pystorm $2"
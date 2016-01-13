#!/bin/sh

cd /sys/class/gpio
echo 129 >export
cd gpio129
echo "out" >direction
echo 1 >value


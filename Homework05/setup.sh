#!/bin/bash
cd /sys/class/gpio
echo 31 > export
echo 50 > export
echo in > gpio31/direction
echo in > gpio50/direction

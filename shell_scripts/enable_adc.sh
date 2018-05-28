#!/bin/sh
# shell script to set up hrtimer trigger for ADC buffer
# change permissions with chmod +x /home/debian/MAE156B_Team6/shell_scripts/enable_adc.sh
echo 15 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio15/direction
echo 1 > /sys/class/gpio/gpio15/value
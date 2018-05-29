#!/bin/sh
# shell script to set up sysfs trigger for ADC buffer
# change permissions with chmod +x /home/debian/MAE156B_Team6/shell_scripts/setup_sysfs_trigger.sh
echo 1 > /sys/bus/iio/devices/iio_sysfs_trigger/add_trigger
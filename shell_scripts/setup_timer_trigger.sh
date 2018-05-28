#!/bin/sh
# shell script to set up hrtimer trigger for ADC buffer
# change permissions with chmod +x /home/debian/MAE156B_Team6/shell_scripts/setup_timer_trigger.sh
mkdir /sys/kernel/config/iio/triggers/hrtimer/trigger0

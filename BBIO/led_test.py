# This is a script to test BBIO using python.
#
# Blinks an LED on/off
#
# P8_2 is GND and P8_12 is output

import Adafruit_BBIO.GPIO as GPIO
import time

GPIO.setup("P8_12", GPIO.OUT)

while True:
	GPIO.output("P8_12", GPIO.HIGH)
	time.sleep(0.5)
	GPIO.output("P8_12", GPIO.LOW)
	time.sleep(0.5)

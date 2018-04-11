# This is a script to test SPI using Adafruit_BBIO.
#
# tests functionality of SPI pins using BBIO
# make sure P9_17, P9_18, P9_21, P9_22 are configured as spi pins using 'config-pin' utility

import Adafruit_BBIO.SPI as SPI
from Adafruit_BBIO.SPI import SPI

#/dev/spidev1.0, i think
spi = SPI(1,0)

#connect pins 9_18 and 9_21 to see if it works
print(spi.xfer2([32, 11, 110, 22, 220]))
spi.close()
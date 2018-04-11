#I'm commenting something
#beagle bone black 8x1 muxxing

import Adafruit_BBIO.GPIO as GPIO
import Adafruit_BBIO.ADC as ADC


 #  Thruth table for MUX/DEMUX, 'Chan on' column is pin on IC not
 #  Arduino pins
 #  |   E   |   s2  |   s1  |   s0  | MUX  On  | NODE | 
 #  |  Low  |  Low  |  Low  |  Low  |  pin 13  |  7   |
 #  |  Low  |  Low  |  Low  |  High |  pin 14  |  6   |
 #  |  Low  |  Low  |  High |  Low  |  pin 15  |  5   |
 #  |  Low  |  Low  |  High |  High |  pin 12  |  8   |
 #  |  Low  |  High |  Low  |  Low  |  pin 1   |  1   |
 #  |  Low  |  High |  Low  |  High |  pin 5   |  4   |
 #  |  Low  |  High |  High |  Low  |  pin 2   |  2   |
 #  |  Low  |  High |  High |  High |  pin 4   |  3   |
 #  |  High |  NA   |  NA   |  NA   |  all off |  NA  |

chan = [[1,0,0],[1,1,0],[1,1,1],
        [1,0,1],[0,1,0],[0,0,1],
        [0,0,0],[0,1,1]]

demux1 = [1,2,3,4,5,6,7,8]

demux2 = [6,5,8,7,1,4,3]

mux = [[2,3,4,5,7,8],[1,3,4,6,7,8],[1,2,4,5,6,7],
       [1,2,3,5,6,8],[1,3,4,6,7,8],[2,3,4,5,7,8],
       [1,2,3,5,6,8],[1,2,4,5,6,7]]

#GPIO pins used for muxs
demux1_s0 = "p8_12"
demux1_s1 = "p8_14"
demux1_s2 = "p8_16"

demux2_s0 = "p8_21"
demux2_s1 = "p8_23"
demux2_s2 = "p8_25"

mux_s0 = "p8_11"
mux_s1 = "p8_15"
mux_s2 = "p8_17"

#adc pin used for voltage measurements
volt_read = "p9_33"

#GPIO pin initializations
GPIO.setup(demux1_s0, GPIO.OUT)
GPIO.setup(demux1_s1, GPIO.OUT)
GPIO.setup(demux1_s2, GPIO.OUT)
GPIO.setup(demux2_s0, GPIO.OUT)
GPIO.setup(demux2_s1, GPIO.OUT)
GPIO.setup(demux2_s2, GPIO.OUT)
GPIO.setup(mux_s0, GPIO.OUT)
GPIO.setup(mux_s1, GPIO.OUT)
GPIO.setup(mux_s2, GPIO.OUT)

#ADC initialization
ADC.setup()

#ADC resolution 
bits_to_volts = 1.8/4095

flag = 1

while (flag):#loops continously until you ctrl c
    for i in range(0,8):#current and ground cycling
        GPIO.output(demux1_s0,chan[demux1[i]-1][2])
        GPIO.output(demux1_s1,chan[demux1[i]-1][1])
        GPIO.output(demux1_s2,chan[demux1[i]-1][0])
        GPIO.output(demux2_s0,chan[demux2[i]-1][2])
        GPIO.output(demux2_s1,chan[demux2[i]-1][1])
        GPIO.output(demux2_s2,chan[demux2[i]-1][0])
        for j in range(0,6):#voltage measurements for each current/ground config
            GPIO.output(mux_s0,chan[mux[i][j]-1][2])
            GPIO.output(mux_s1,chan[mux[i][j]-1][1])
            GPIO.output(mux_s2,chan[mux[i][j]-1][0])
            
            value = ADC.read_raw(volt_read)
            voltage = value*bits_to_volts
            print(voltage,end= ' ')
    print("\t")            
print("\n")

            

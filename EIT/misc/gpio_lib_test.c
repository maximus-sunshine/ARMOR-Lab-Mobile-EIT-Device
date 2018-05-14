/* Script to test gpio_lib functionality/performance */

#include "gpiolib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//GPIO pin 70, gpio2_6, P8.45
int bank = 2;
int pin  = 6;
gpio_info *test_pin;

int main(){
	
	uint32_t pinmask = bit(pin); 
	gpio_init();
	test_pin = gpio_attach(bank, pinmask, GPIO_OUT);
	gpio_set(test_pin);
	usleep(5000000);
	gpio_clear(test_pin);
	gpio_detach(test_pin);
	gpio_finish();
}
/**
 * MAE 156B Spring 2018 Team 6
 *
 * Basic interface for the TI-ADS8684 ADC, an iio device
 *
 * Copied from James Strawson's GitHub
 */

#include <stdio.h>
#include <stdlib.h> // for atoi
#include <errno.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include "gpiolib.h"
#include "ti-ads8684.h"
#include "eit_config.h"

double scale = 0.078127104;
int size[5] = {0, 0, 0, 0 ,0};

gpio_info *adc_rst;

int main(int argc, char **argv)
{

	printf("\n\n\nthis is the size of %d\n\n\n", sizeof(size));
	printf("\n entered MAIN...");
	fflush(stdout);

	/**************************
	* PARSE ARGS
	**************************/
	int usr_chan = atoi(argv[1]);
	printf("\n parsed args. reading ADC channel %d...",usr_chan);
	fflush(stdout);

	/**************************
	* INITIALIZE ADC INTERFACE
	**************************/	
	ti_adc_init();
	printf("\n initialized ADC interface...");
	fflush(stdout);

	/**************************
	* SET UP GPIO PINS
	**************************/
	char dir_buf_rd[MAX_BUF];
	char val_buf_rd[MAX_BUF];
	
	snprintf(dir_buf_rd, sizeof(dir_buf_rd), "/sys/class/gpio/gpio13/direction");
	snprintf(val_buf_rd, sizeof(val_buf_rd), "/sys/class/gpio/gpio13/value");
	
	int adc_rst_dir_fd = open(dir_buf_rd, O_WRONLY);
	int adc_rst_val_fd = open(val_buf_rd, O_WRONLY);

	char dir_buf_wr[MAX_BUF];
	char val_buf_wr[MAX_BUF];
	
	snprintf(dir_buf_wr, sizeof(dir_buf_wr), "out");
	snprintf(val_buf_wr, sizeof(val_buf_wr), "1");
	
	write(adc_rst_dir_fd, dir_buf_wr, sizeof(dir_buf_wr));
	write(adc_rst_val_fd, val_buf_wr, sizeof(val_buf_wr));

	close(adc_rst_dir_fd);
	close(adc_rst_val_fd);

	printf("\n ADC reset pin set...\n");
	fflush(stdout);
	usleep(1 * 1000000);

	// if(gpio_init()){
	// 	fprintf(stderr, "gpio_init failed with %i\n", gpio_errno);
	// }
	// printf("\n initialized gpiolib...");
	// fflush(stdout);

	// // gpio_info *adc_rst;
	// // printf("\n declared ADC reset pin struct...");
	// // // fflush(stdout);

	// adc_rst = gpio_attach(ADC_RESET_GPIO/32, bit(ADC_RESET_GPIO%32), GPIO_OUT);
	// printf("\n attached ADC gpio pin...");
	// fflush(stdout);

	// gpio_set(adc_rst);
	// printf("\n set ADC gpio pin...");
	// fflush(stdout);

	/**************************
	* CONFIGURE ADC CHANNEL
	**************************/	
	ti_adc_set_scale(usr_chan,scale); //set channel 1 scale
	printf("\n setting ADC scale to %.9f...\n", scale);
	fflush(stdout);

	ti_adc_set_offset(usr_chan,0); //set channel 1 offset
	printf("\n set ADC offset...\n");
	fflush(stdout);

	printf("\n begin sampling...");
	fflush(stdout);

	/**************************
	* READ
	**************************/	
	printf("\n\n Voltage on channel %d: \n", usr_chan);
	fflush(stdout);
	
	int j;
	int loops = 1;
	for(j=0;j<loops;j++){
		printf("  %0.5f V\r", ti_adc_read_raw(usr_chan)*scale/1000);
	}

	/**************************
	* CLEANUP
	**************************/	
	printf("\n\n Done reading, cleaning up...", usr_chan);
	fflush(stdout);

	// gpio_detach(adc_rst);
	// printf("\n detached ADC reset pin...", usr_chan);
	// fflush(stdout);

	// gpio_finish();
	// printf("\n closed gpiolib cleanly...", usr_chan);
	// fflush(stdout);

	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...", usr_chan);
	fflush(stdout);

	printf("\n FINISHED!", usr_chan);
	fflush(stdout);
}
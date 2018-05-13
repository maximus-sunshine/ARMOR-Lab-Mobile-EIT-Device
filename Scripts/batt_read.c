/************************************************************************************
 * MAE 156B Spring 2018 Team 6 (5/9/18)
 *	- Maxwell Sun
 *	- Matthew Williams
 *	- Aaron Gunn
 *	- Jacob Rutheiser
 *
 * Read the battery
 * 
 * Using gpio_lib (https://bitbucket.org/vanguardiasur/gpiolib) for GPIO toggling (~3 MHz)
 * 
 * Using sysfs to read ADC (best ~15 kHz), need to improve (add buffer to adc driver?)
 *
 * compile with "gcc -pthread batt_read.c ti-ads8684.c -o batt_read"
 *
 * TODO: -find faster way to read ADC
 *		 -clean up code, move stuff to header file
 *       -error handling
 *       -pthread, write data to .txt
 *		 -write Makefile
 **
 ************************************************************************************/

/************************************************************************************
* INCLUDES
*************************************************************************************/
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>		// for atoi
#include <fcntl.h>		// for open
#include <unistd.h>		// for close
#include <stdio.h>		
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>

#include <assert.h>		//for gpiolib stuff
#include <signal.h>
#include <sys/time.h>
#include "eit_config.h"
#include "ti-ads8684.h"

#define VOLT_DATA_TXT "~/MAE156B_Team6/"

/************************************************************************************
* SETUP SIGINT HANDLER
*************************************************************************************/
void sigint(int s __attribute__((unused)));

/************************************************************************************
* DECLARE PTHREAD FUNCTIONS
*************************************************************************************/
void* write_data(void *ptr);
pthread_t write_data_thread;

/************************************************************************************
* SETUP
*************************************************************************************/
double scale = 0.078127104;
int flag = 0;                //flag so pthread knows when to stop

/************************************************************************************
* MAIN
*************************************************************************************/
int main()
{
	flag = 1;
	//creating pthread
	
	pthread_create(&write_data_thread,NULL,write_data,(void *) NULL);


	printf("\n entered MAIN...");
	fflush(stdout);

	/**************************
	* SETUP SIGINT HANDLER
	**************************/	
	signal(SIGINT, sigint);
	printf("\n SIGINT handler set up...");
	fflush(stdout);

	/**************************
	* INITIALIZE ADC INTERFACE
	**************************/	
	ti_adc_init();
	printf("\n ADC interface initialized...");
	fflush(stdout);

	ti_adc_enable();
	printf("\n ADC reset pin set...");
	fflush(stdout);

	//Set scale
	//TODO: put scales and offsets in header file
	ti_adc_set_scale(2, scale);
	ti_adc_set_offset(2, 0);

	printf("\n ADC scales and offsets set...");
	fflush(stdout);
	
	/**********************************
	* EXECUTE SAMPLING
	***********************************/
	printf("\n BEGIN SAMPLING...\n\n");
	fflush(stdout);

	while(1){
		// read ADC.
		printf("Battery Voltage:  %0.5f V\n", ti_adc_read_raw(2)*scale/1000);
		fflush(stdout);
		usleep(1*1e6);
	}

	/**********************************
	* CLEANUP
	***********************************/
	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);

	printf("\n FINISHED!\n\n");
	fflush(stdout);
}

/************************************************************************************
* SETUP SIGINT HANDLER
*************************************************************************************/
void* write_data(void *ptr){
	
	printf(VOLT_DATA_TXT);
	// File *fp_write = fopen(VOLT_DATA_TXT, "a");

	//File *fp_read = fopen("/dev/iio:device1","r");
	int i = 0;
	while(flag == 1){
		// fread(buffer,sizeof)
		// fprintf(fp, "%d", buff[i]);
		// i++;
		printf("pthread ran %d times...\n",i);
		i++;
		usleep(0.5*1e6);
	}

	//fclose(fp);
	return NULL;
}

void sigint(int s __attribute__((unused))) {
	printf("\n\n received SIGINT, exiting cleanly...\n");

	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);

	printf("\n FINISHED!\n\n");
	fflush(stdout);

	printf("(waiting for pthread to join)\n");
	flag = 0;
	pthread_join(write_data_thread, NULL);
	printf("pthread joined \n");

	exit(0);
}


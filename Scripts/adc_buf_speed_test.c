/************************************************************************************
 * MAE 156B Spring 2018 Team 6 (5/9/18)
 *	- Maxwell Sun
 *	- Matthew Williams
 *	- Aaron Gunn
 *	- Jacob Rutheiser
 *
 * Script to sample sensing skin using new ADC buffer.
 * 
 * Using gpio_lib (https://bitbucket.org/vanguardiasur/gpiolib) for GPIO toggling (~3 MHz)
 * 
 * Using sysfs to read ADC (best ~15 kHz), need to improve (add buffer to adc driver?)
 *
 * compile with "gcc -pthread adc_buf_speed_test.c ti-ads8684.c -o adc_buf_speed_test"
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
#include "gpiolib.h"
#include "eit_config.h"
#include "eit.h"
#include "ti-ads8684.h"

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

int flag = 0;      

#define VOLT_DATA_TXT "~/MAE156B_Team6/"

/************************************************************************************
* MAIN
*************************************************************************************/
int main()
{
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
	ti_adc_set_scale(0, scale);
	ti_adc_set_offset(0, 0);

	ti_adc_set_scale(2, scale);
	ti_adc_set_offset(2, 0);

	printf("\n ADC scales and offsets set...");
	fflush(stdout);

	//enable channels to be read by buffer, disable others
	ti_adc_disable_channel(0);
	ti_adc_disable_channel(1);
	ti_adc_enable_channel(2);
	ti_adc_disable_channel(3);

	printf("\n ADC channels enabled to be read by buffer...");
	fflush(stdout);

	//set ADC sampling frequency
	int sample_rate = 5000; //Hz
	ti_adc_set_sample_rate(sample_rate); 

	printf("\n ADC sampling frequency set to %d Hz...", sample_rate);
	fflush(stdout);

	//Set ADC buffer length
	int buf_length = 1000;
	ti_adc_set_buf_length(buf_length);

	printf("\n Buffer length set to %d...", buf_length);
	fflush(stdout);

	/**********************************
	* EXECUTE SAMPLING
	***********************************/
	printf("\n BEGIN:");
	fflush(stdout);

	//set flag and start pthread
	flag = 1;
	pthread_create(&write_data_thread,NULL,write_data,(void *) NULL);
	printf("\n pthread created...\n");
	fflush(stdout);

	//enable ADC buffer
	ti_adc_enable_buf();
	printf("\n enabled ADC buffer...");
	fflush(stdout);
	
	//Timing of one cycle
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
 	
 	//wait for SIGINT
  	int i = 0;
	while(1){	
		printf("\n MAIN loop has run %d times...",i);
		fflush(stdout);
		usleep(0.5*1e6);
		i++;
	}
	gettimeofday(&t2, NULL);
	long usec = 1e6 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec;
	printf("\n DONE, elapsed time: %f seconds", usec/1e6);
	fflush(stdout);
	
	/**********************************
	* CLEANUP
	***********************************/
	ti_adc_disable_buf();
	printf("\n disabled ADC buffer...");
	fflush(stdout);

	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);

	printf("\n FINISHED!\n\n");
	fflush(stdout);
}

/************************************************************************************
* PTHREADS
*************************************************************************************/
void* write_data(void *ptr){
	
	// File *fp_write = fopen(VOLT_DATA_TXT, "a");

	// File *fp_read = fopen("/dev/iio:device1","r");

	int i = 0;
	while(flag == 1){
		// fcontent = (char*)malloc(sizeof())

		// fread(fp_write,sizeof(int),1,fp_read);
		// fprintf(fp, "%d", buff[i]);
		// i++;

		printf("pthread ran %d times...\n",i);
		i++;
		usleep(0.5*1e6);
	}

	//fclose(fp);
	return NULL;
}

/************************************************************************************
* SIGINT HANDLER
*************************************************************************************/
void sigint(int s __attribute__((unused))) {
	printf("\n\n received SIGINT, exiting cleanly...\n");

	ti_adc_disable_buf();
	printf("\n disabled ADC buffer...");
	fflush(stdout);

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
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
 * compile with "gcc eit_sample.c eit.c gpiolib.c ti-ads8684.c -o eit_sample"
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

#include <assert.h>		//for gpiolib stuff
#include <signal.h>
#include <sys/time.h>
#include "gpiolib.h"
#include "eit_config.h"
#include "eit.h"
#include "ti-ads8684.h"

/************************************************************************************
* DEFINES
*************************************************************************************/
#define MUX_PINS 5

/************************************************************************************
* SETUP SIGINT HANDLER
*************************************************************************************/
void sigint(int s __attribute__((unused)));

/************************************************************************************
* SETUP
*************************************************************************************/

//mux array declarations
int current_mux[NODAL_NUM];              // current?                                           
int ground_mux[NODAL_NUM];               // ground?
int voltage_mux[NODAL_NUM][NODAL_NUM-2]; // voltage sampling?

// gpio pin IDs, used for exporting pins 
// gpio pin ID = gpio_bank*32 + gpio_pinmask (e.g. gpio 86 = gpio2_22 (86 = 2*32 + 22))
// mux channels {A4, A3, A2, A1, A0}
int current_mux_gpio[5] = CURRENT_MUX_GPIO;
int ground_mux_gpio[5]  = GROUND_MUX_GPIO;
int voltage_mux_gpio[5] = VOLTAGE_MUX_GPIO;

int adc_reset_gpio      = ADC_RESET_GPIO;
int i_sense_reset_gpio  = I_SENSE_RESET_GPIO;

int loop_flag = 0; //signal p_thread to stop

//create structs for mux logic gpios
gpio_info *current_mux_gpio_info[MUX_PINS];
gpio_info *ground_mux_gpio_info[MUX_PINS];
gpio_info *voltage_mux_gpio_info[MUX_PINS];
// gpio_info *adc_reset_gpio_info;
// gpio_info *i_sense_reset_gpio_info;

double scale = 0.078127104;

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
	ti_adc_enable_channel(0);
	ti_adc_enable_channel(1);
	ti_adc_disable_channel(2);
	ti_adc_disable_channel(3);

	printf("\n ADC channels enabled to be read by buffer...");
	fflush(stdout);

	//set ADC sampling frequency
	int sample_rate = 10; //Hz
	ti_adc_set_sample_rate(sample_rate); 

	printf("\n ADC sampling frequency set to %d Hz...", sample_rate);
	fflush(stdout);

	//Set ADC buffer length
	int buf_length = 10000;
	ti_adc_set_buf_length(buf_length);

	printf("\n Buffer length set to %d...", buf_length);
	fflush(stdout);

	/**************************
	* SET UP GPIO PINS
	**************************/
	//initialize gpio_lib
	if(gpio_init()){
		fprintf(stderr, "gpio_init failed with %i\n", gpio_errno);
	}
	printf("\n gpiolib intialized...");
	fflush(stdout);

	int i;
	for(i = 0; i < MUX_PINS; i++){
		current_mux_gpio_info[i] = malloc(sizeof(gpio_info));
		ground_mux_gpio_info[i] = malloc(sizeof(gpio_info));
		voltage_mux_gpio_info[i] = malloc(sizeof(gpio_info));
	}
	printf("\n gpio_info structs set up and memory allocated...");
	fflush(stdout);

	//attach current gpio pins
	for(i = 0; i < MUX_PINS; i++){
		int bank = current_mux_gpio[i]/32;
		int mask = bit(current_mux_gpio[i]%32);
		current_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	}

	//attach ground gpio pins
	for(i = 0; i < MUX_PINS; i++){                            
		int bank = ground_mux_gpio[i]/32;
		int mask = bit(ground_mux_gpio[i]%32);
		ground_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	}

	//attach voltage gpio pins
	for(i = 0; i < MUX_PINS; i++){                            
		int bank = voltage_mux_gpio[i]/32;
		int mask = bit(voltage_mux_gpio[i]%32);
		voltage_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
	}
	printf("\n mux logic gpio pins attached...");
	fflush(stdout);

	// //attach other gpio pins
	// adc_reset_gpio_info = gpio_attach(adc_reset_gpio/32, bit(adc_reset_gpio%32), GPIO_OUT);
	// i_sense_reset_gpio_info  = gpio_attach(i_sense_reset_gpio/32, bit(i_sense_reset_gpio%32), GPIO_OUT);
	
	/**********************************
	* SET UP MUX SWITCHING PATTERN
	***********************************/
	//configures current and ground nodes according to # of nodes(NODAL_NUM)
	cur_gnd_config(current_mux,ground_mux);
	printf("\n current and ground switching patterns configured...");
	fflush(stdout);

	//configures voltage sampling nodes according to # of nodes(NODAL_NUM)
	volt_samp_config(current_mux,ground_mux,voltage_mux);
	printf("\n voltage sampling pattern configured...");
	fflush(stdout);

	/**********************************
	* TODO: TURN ON CURRENT SOURCE
	***********************************/
	
	/**********************************
	* EXECUTE SAMPLING
	***********************************/
	printf("\n BEGIN SAMPLING...");
	fflush(stdout);

	// printf("\n\n HERE COMES THE DATA:\n\n VOLTAGE\tCURRENT\n\n\n");
	// fflush(stdout);

	//Timing of one cycle
	struct timeval t1, t2;

	gettimeofday(&t1, NULL);

	//enable ADC buffer
	ti_adc_enable_buf();
	printf("\n ADC buffer enabled...");
	fflush(stdout);

	int flag = 0;
	int cycles = 100;
	loop_flag = 1;

	//start pthread
  	//runs "cycles" times
	while(flag < cycles){
		flag++;
		// printf("\n\nFLAG: %d\n\n",flag);
		// fflush(stdout);

		printf("\n\n\n******************** Cycle %d *************************",flag);
		fflush(stdout);
		
		//Outer loop, move current and ground
		for(i = 0; i < NODAL_NUM; i++){
			
			printf("\n\n--------------Configuration: Current at node %d, GND at node %d -------------\n\n\tVOLTAGE (V)\tCURRENT (uA)\n", current_mux[i]+1, ground_mux[i]+1);
			fflush(stdout);

			//Set current and ground mux logic pins
			int k;
			for(k = 0; k < MUX_PINS; k++){ 

				/// switch(CHAN[current_mux[i][k]){
				// 		case (1) : gpio_set(current_mux_gpio_info[k]);
				// 				   break;

				// 		case (0) : gpio_clear(current_mux_gpio_info[k]);
				// 				   break;

				// 		default  : printf("logic table is wrong");
				// 				   break;
				// 	}

				if(CHAN[current_mux[i]][k]==1){
					gpio_set(current_mux_gpio_info[k]);
				}
				else{
					gpio_clear(current_mux_gpio_info[k]);
				}

				if(CHAN[ground_mux[i]][k]==1){
					gpio_set(ground_mux_gpio_info[k]);
				}
				else{
					gpio_clear(ground_mux_gpio_info[k]);
				}
			}

			//Inner loop, measure voltage
			int j;
			for(j = 0; j < (NODAL_NUM-2); j++){
				for(k = 0; k < MUX_PINS; k++){
					// switch(CHAN[voltage_mux[i][j]][k]){
					// 	case (1) : gpio_set(voltage_mux_gpio_info[k]);
					// 			   break;

					// 	case (0) : gpio_clear(voltage_mux_gpio_info[k]);
					// 			   break;

					// 	default  : printf("logic table is wrong");
					// 			   break;
					// }


					if(CHAN[voltage_mux[i][j]][k]==1){
						gpio_set(voltage_mux_gpio_info[k]);
					}
					else{
						gpio_clear(voltage_mux_gpio_info[k]);
					}
				}



				// ti_adc_read_raw(0);
				// ti_adc_read_raw(1);

				// // read ADC.
				// printf("Voltage at node %d:  %0.5f V\n", voltage_mux[i][j]+1, ti_adc_read_raw(0)*scale/1000);
				// fflush(stdout);
		  		// // usleep(1 * 1000000);
				
				// printf("Node %d:\t%0.8f\t%0.8f\n", voltage_mux[i][j]+1, ti_adc_read_raw(0)*scale/1000, (((ti_adc_read_raw(1)*scale/1000)-(.00009694))/2350)*1e6);
				// fflush(stdout);

				// //Sleep
				// float sleep_time = 0.5;
				// fflush(stdout);
				// usleep(sleep_time * 1000000);

				// printf("%0.9f\t%0.9f\n", ti_adc_read_raw(0)*scale/1000, ti_adc_read_raw(1)*scale/1000);
				// fflush(stdout);
			}
		}
	}

	loop_flag = 0;

	gettimeofday(&t2, NULL);
	long usec = 1e6 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec;
	printf("\n DONE SAMPLING %d nodes, %d cycles in %f seconds!\n\n avg. cyclic frequency:\t%f Hz\n avg. cyclic period:\t%f s\n\n", NODAL_NUM, cycles, usec/1e6, 1/(usec/1e6/cycles), usec/1e6/cycles);
	fflush(stdout);
	
	/**********************************
	* CLEANUP
	***********************************/
	for(i=0;i<MUX_PINS;i++){
		gpio_detach(current_mux_gpio_info[i]);
		gpio_detach(ground_mux_gpio_info[i]);
		gpio_detach(voltage_mux_gpio_info[i]);
	}	
	printf("\n detached all gpio pins");
	fflush(stdout);

	gpio_finish();
	printf("\n closed gpiolib cleanly...");
	fflush(stdout);

	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);

	printf("\n FINISHED!\n\n");
	fflush(stdout);
}

/************************************************************************************
* SETUP SIGINT HANDLER
*************************************************************************************/
void sigint(int s __attribute__((unused))) {
	printf("\n\n received SIGINT, exiting cleanly...\n");
	
	//This could mess things up if gpios never get attached
	//TODO: add flag to make sure gpios got set up properly
	int i;
	for(i=0;i<MUX_PINS;i++){
		gpio_detach(current_mux_gpio_info[i]);
		gpio_detach(ground_mux_gpio_info[i]);
		gpio_detach(voltage_mux_gpio_info[i]);
	}	
	printf("\n detached all gpio pins...");
	fflush(stdout);

	gpio_finish();
	printf("\n closed gpiolib cleanly...");
	fflush(stdout);

	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);

	printf("\n FINISHED!\n\n");
	fflush(stdout);

	exit(0);
}

void* write_data(void *buff[]){
	
	
	File *fp = fopen(VOLT_DATA_TXT, "a");
	
	int i = 0
	while(loop_flag==1){
		fprintf(fp, "%d", buff[i]);
	}
	fclose(fp);
	return NULL;
	

	
}


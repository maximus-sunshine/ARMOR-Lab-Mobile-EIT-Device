/************************************************************************************
 * MAE 156B Spring 2018 Team 6
 *	- Maxwell Sun
 *	- Matthew Williams
 *	- Aaron Gunn
 *	- Jacob Rutheiser
 *
 *	Similar to eit_sample.c except prints to screen instead of writing to file.
 * 
 *	Prints voltage measurements at nodes and voltage across current sensor.
 *	
 *	Current setpoint, number of nodes, and ADC scale/offset are all set before running. If changed, needs to be recompiled	 
 *
 * compile with "gcc -pthread eit_sample_print.c src/eit.c src/gpiolib.c src/ti-ads8684.c -o eit_sample_print"
 *
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
#include <pthread.h>
#include <malloc.h>
 
#include <assert.h>		//for gpiolib stuff
#include <signal.h>
#include <sys/time.h>
#include "includes/gpiolib.h"
#include "includes/eit_config.h"
#include "includes/eit.h"
#include "includes/ti-ads8684.h"

/************************************************************************************
* DEFINES
*************************************************************************************/
#define MUX_PINS 5


/************************************************************************************
* PTHREAD SETUP
*************************************************************************************/
// pthread_t data_exporting_thread;
// //pthread function declaration
// void* data_exporting(void *ptr);
// //data file declaration
// FILE* fp;

/************************************************************************************
* SETUP SIGINT HANDLER
*************************************************************************************/
//void sigint(int s __attribute__((unused)));

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
int current_switch_gpio[10] = CURRENT_SWITCH_GPIO;
int mux_disable_gpio[3] = MUX_DISABLE_GPIO;

int adc_reset_gpio      = ADC_RESET_GPIO;
int i_sense_reset_gpio  = I_SENSE_RESET_GPIO;

int chan0; //voltage adc reading channel
int chan1; //current sense channel

//create structs for mux logic gpios
gpio_info *current_mux_gpio_info[MUX_PINS];
gpio_info *ground_mux_gpio_info[MUX_PINS];
gpio_info *voltage_mux_gpio_info[MUX_PINS];

gpio_info *current_switch_gpio_info[10];
gpio_info *adc_reset_gpio_info;
gpio_info *i_sense_reset_gpio_info;

gpio_info *mux_disable_gpio_info[3];

//Variables
double scale = 0.078127104;
int current_setpoint = 9; //current setpoint 100uA-2000uA(0-19, 100uA) TODO, make this better

// //struct declarations
// Array dynamic_buffer;
// size_t init_size = 100;
// int size = 0;
// int flag = 1;

/************************************************************************************
* MAIN
*************************************************************************************/
int main()
{
	printf("\n entered MAIN...");
	fflush(stdout);
	
	//Sigint setup, needs work
	signal(SIGINT, sigint);
	printf("setup SIGINT...\n");
	fflush(stdout);

	/*************************
	* INITIALIZE BUFFER ARRAY
	**************************/	
	// initArray(&dynamic_buffer,init_size);

	/**************************
	* allocate memory for gpio_info structs
	**************************/	
	int i;
	for(i = 0; i < MUX_PINS; i++){
		current_mux_gpio_info[i] = malloc(sizeof(gpio_info));
		ground_mux_gpio_info[i] = malloc(sizeof(gpio_info));
		voltage_mux_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	for(i = 0; i < 10; i++){
		current_switch_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	for(i = 0; i < 3; i++){
		mux_disable_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	/**************************
	* SET UP GPIO PINS
	**************************/
	//initialize gpio_lib
	if(gpio_init()){
		fprintf(stderr, "gpio_init failed with %i\n", gpio_errno);
	}
	printf("\n gpiolib intialized...");
	fflush(stdout);

	//attach current gpio pins
	int bank, mask;
	for(i = 0; i < MUX_PINS; i++){
		bank = current_mux_gpio[i]/32;
		mask = bit(current_mux_gpio[i]%32);
		current_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	}
	//attach ground gpio pins
	for(i = 0; i < MUX_PINS; i++){                            
		bank = ground_mux_gpio[i]/32;
		mask = bit(ground_mux_gpio[i]%32);
		ground_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	}
	//attach voltage gpio pins
	for(i = 0; i < MUX_PINS; i++){                            
		bank = voltage_mux_gpio[i]/32;
		mask = bit(voltage_mux_gpio[i]%32);
		voltage_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
	}
	//attach current switch gpio pins
	for(i = 0; i < 10; i++){                            
		int bank = current_switch_gpio[i]/32;
		int mask = bit(current_switch_gpio[i]%32);
		current_switch_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
	}
	//attach mux enable gpio pins
	for(i = 0; i < 3; i++){                            
		int bank = mux_disable_gpio[i]/32;
		int mask = bit(mux_disable_gpio[i]%32);
		mux_disable_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
	}
	//adc reset attach
	bank = adc_reset_gpio/32;
	mask = bit(adc_reset_gpio%32);
	adc_reset_gpio_info = gpio_attach(bank, mask, GPIO_OUT);
	//i sense reset attach
	bank = i_sense_reset_gpio/32;
	mask = bit(i_sense_reset_gpio%32);
	i_sense_reset_gpio_info = gpio_attach(bank, mask, GPIO_OUT);

	printf("\n gpio pins attached...");
	fflush(stdout);

	/**************************
	* INITIALIZE ADC INTERFACE
	**************************/	
	//Enable ADC pin
	gpio_set(adc_reset_gpio_info);
	printf("\n ADC enabled");
	fflush(stdout);

	ti_adc_init();
	printf("\n ADC interface initialized...");
	fflush(stdout);

	// ti_adc_enable();
	// printf("\n ADC reset pin set...");
	// fflush(stdout);

	//TODO: put scales and offsets in header file
	ti_adc_set_scale(0, scale); //chan0
	ti_adc_set_offset(0, 0);

	ti_adc_set_scale(1, scale); //chan1
	ti_adc_set_offset(1, 0);

	printf("\n ADC scales and offsets configured...");
	fflush(stdout);
	
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
	* Disable Muxs
	***********************************/
	int n;
	for(n = 0;n < 3; n++){
		gpio_set(mux_disable_gpio_info[n]);
	}

	/**********************************
	* TURN ON CURRENT SOURCE
	***********************************/
	printf("\n\n current set to %d uA...",(current_setpoint+1)*100);
	fflush(stdout);

	for(i = 0; i< 10; i++){
		if(CURRENT[current_setpoint][i]==1){
			gpio_set(current_switch_gpio_info[i]);
		}
		else{
			gpio_clear(current_switch_gpio_info[i]);
		}
	}
	/**********************************
	* EXECUTE SAMPLING
	***********************************/
	printf("\n beginning sample cycle...");
	fflush(stdout);

	int count = 0;
	int cycles = 10; //runs "cycles" times

	struct timeval t1, t2; //time stuff 
	gettimeofday(&t1, NULL);
  	
	// //start pthread
	// pthread_create(&data_exporting_thread, NULL, data_exporting, (void*) NULL);
	// printf("\n pthread created...");
	// fflush(stdout);

	while(count < cycles){
		count++;

		printf("\n\n\n******************** Cycle %d *************************\n\n",count);

		//Outer loop, move current and ground
		for(i = 0; i < NODAL_NUM; i++){

			printf("\n--------------Current Configuration: Current at node %d, GND at node %d ------------\n", current_mux[i]+1, ground_mux[i]+1);
			fflush(stdout);

			//Set current and ground mux logic pins
			int k;
			for(k = 0; k < MUX_PINS; k++){                            
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
					if(CHAN[voltage_mux[i][j]][k]==1){
						gpio_set(voltage_mux_gpio_info[k]);
					}
					else{
						gpio_clear(voltage_mux_gpio_info[k]);
					}
				}

				//enabling muxs
				for(n = 0;n < 3; n++){
					gpio_clear(mux_disable_gpio_info[n]);
				}

				// //test current switches
				// printf("\n Switching currents...");
				// fflush(stdout);
				// int q;
				// for(q=0;q<20;q++){
				// 	printf("\n\n current set to %d uA...",(q+1)*100);
				// 	fflush(stdout);
				// 	for(i = 0; i< 10; i++){
				// 		if(CURRENT[q][i]==1){
				// 			gpio_set(current_switch_gpio_info[i]);
				// 		}
				// 		else{
				// 			gpio_clear(current_switch_gpio_info[i]);
				// 		}
				// 	}
				// 	usleep(2*1e6);
				// }

				//read ADC
				chan0 = ti_adc_read_raw(0);
				chan1 = ti_adc_read_raw(1);
		        printf("Voltage at node %d:  %0.5f V\t\t Voltage across current sensor: %0.5f V\n", voltage_mux[i][j]+1,chan0*scale/1000,chan1*scale/1000);
		        fflush(stdout);
				
				// //record adc raw voltage into buffer (must be an int)
				// insertArray(&dynamic_buffer,chan0);
				// insertArray(&dynamic_buffer,chan1);
				// size=size+2;
		        // printf("\ninserted Array...");
		        // fflush(stdout);

				//disabling muxs
				for(n = 0;n < 3; n++){
					gpio_set(mux_disable_gpio_info[n]);
				}
		        // usleep(1 * 1e6);
		        // printf("\nmuxes disabled...");
		        // fflush(stdout);
	      	}
	    }
 	}
 	//Print timing data to screen!
 	gettimeofday(&t2, NULL);
 	long usec = 1e6 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec;
	printf("\n Done sampling %d nodes, %d cycles in %0.5f seconds...\n Avg. cyclic frequency: %0.5f",NODAL_NUM, cycles, usec/1e6, cycles/(usec/1e6));
	fflush(stdout);

	//Cleanup
	
	for(i=0;i<MUX_PINS;i++){
		gpio_detach(current_mux_gpio_info[i]);
		gpio_detach(ground_mux_gpio_info[i]);
		gpio_detach(voltage_mux_gpio_info[i]);
	}	
  
	for(i=0;i<10;i++){
		gpio_detach(current_switch_gpio_info[i]);
		
	}	
	for(i=0;i<3;i++){
		gpio_detach(mux_disable_gpio_info[i]);
		
	}	
	gpio_detach(adc_reset_gpio_info);
	gpio_detach(i_sense_reset_gpio_info);
	
	printf("\n Detached all gpio pins");
	fflush(stdout);

	gpio_finish();
	printf("\n closed gpiolib cleanly...");
	fflush(stdout);

	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);
	
	// printf("\n waiting for pthread to join");
	// flag = 0;

	// pthread_join(data_exporting_thread, NULL);
	// printf("# samples: %d\n",size);
	
	// fclose(fp);

	// printf("file has closed\n");
	printf("\n FINISHED!\n\n");
	fflush(stdout);
}

/************************************************************************************
* SIGINT HANDLER, TODO: improve
*************************************************************************************/
void sigint(int s __attribute__((unused))) {
	printf("\nReceived SIGINT:\n");
	fflush(stdout);

	printf("\n Done sampling...");
	fflush(stdout);

	//Cleanup
	int i;
	for(i=0;i<MUX_PINS;i++){
		gpio_detach(current_mux_gpio_info[i]);
		gpio_detach(ground_mux_gpio_info[i]);
		gpio_detach(voltage_mux_gpio_info[i]);
	}	

	for(i=0;i<10;i++){
		gpio_detach(current_switch_gpio_info[i]);
		
	}	
	for(i=0;i<3;i++){
		gpio_detach(mux_disable_gpio_info[i]);
		
	}	
	gpio_detach(adc_reset_gpio_info);
	gpio_detach(i_sense_reset_gpio_info);

	printf("\n Detached all gpio pins");
	fflush(stdout);

	gpio_finish();
	printf("\n closed gpiolib cleanly...");
	fflush(stdout);

	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);
	
	// printf("waiting for pthread to join\n");
	// flag = 0;
	// pthread_join(data_exporting_thread, NULL);
	// printf("\n pthread has returned %d",size);
	// fclose(fp);
	// printf("\n file has closed");
	exit(0);
}

// /************************************************************************************
// * PTHREAD
// *************************************************************************************/

// void* data_exporting(void *ptr){
// 	fp = fopen(VOLT_DATA_TEXT,"a");
// 	printf("\n Data file opened...\n");	
// 	int i = 0;
// 	usleep(2*1e6); //delay to make sure pthread enters while loop
// 	fprintf(fp,"Chan 0\tChan 1");
	
// 	while(i < size && size > 1){
// 	      // printf("pthread recorded %d value\n", dynamic_buffer.array[i]);
// 	      fprintf(fp,"%0.5f\t%0.5f\n",dynamic_buffer.array[i]*scale/1000,dynamic_buffer.array[i+1]*scale/1000);
// 	    i=i+2;
// 	    // if(flag ==1){
// 	    //     usleep(2*1e6);
// 	    // }    
// 	}
	
// 	printf("\n waiting for pthread to finish...");
// 	return NULL;
// }

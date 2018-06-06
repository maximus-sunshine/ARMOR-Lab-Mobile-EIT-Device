/************************************************************************************
 * MAE 156B Spring 2018 Team 6
 *	- Maxwell Sun
 *	- Matthew Williams
 *	- Aaron Gunn
 *	- Jacob Rutheiser
 *
 *	edited version to do testing in Ken's lab
 *
 * compile with "gcc -pthread eit_sample2_data.c src/eit_data.c src/gpiolib.c src/ti-ads8684_data.c -o eit_sample2_data"
 * 
 *
 * TODO: -find faster way to read ADC
 *		 -clean up code, move stuff to header file
 *       -error handling
 *       -pthread, write data to .txt
 *       -write Makefile
 *
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
#include "includes/eit_data.h"
#include "includes/ti-ads8684_data.h"

/************************************************************************************
* DEFINES
*************************************************************************************/
#define MUX_PINS 5

/************************************************************************************
* SETUP SIGINT HANDLER
*************************************************************************************/
//void sigint(int s __attribute__((unused)));

/************************************************************************************
* SETUP
*************************************************************************************/
//mux pin array declarations
int current_mux[NODAL_NUM];              // current                                           
int ground_mux[NODAL_NUM];               // ground
// int voltage_mux[NODAL_NUM][NODAL_NUM-2]; // voltage sampling WORKS
int voltage_mux[NODAL_NUM][NODAL_NUM];	// Test

//gpio_info structs for all GPIO pins
gpio_info *current_mux_gpio_info[MUX_PINS];	//mux logic pins
gpio_info *ground_mux_gpio_info[MUX_PINS];
gpio_info *voltage_mux_gpio_info[MUX_PINS];
gpio_info *current_switch_gpio_info[10];	//current source switch logic
gpio_info *adc_reset_gpio_info;				//ADC RST pin, must be high for ADC to work
gpio_info *i_sense_reset_gpio_info;			//current sense RST pin
gpio_info *mux_disable_gpio_info[3];		//mux disable pins

// gpio pin IDs, see eit_config.h
int current_mux_gpio[5] = CURRENT_MUX_GPIO;
int ground_mux_gpio[5]  = GROUND_MUX_GPIO;
int voltage_mux_gpio[5] = VOLTAGE_MUX_GPIO;
int current_switch_gpio[10] = CURRENT_SWITCH_GPIO;
int mux_disable_gpio[3] = MUX_DISABLE_GPIO;
int adc_reset_gpio      = ADC_RESET_GPIO;
int i_sense_reset_gpio  = I_SENSE_RESET_GPIO;



/************************************************************************************
* CONFIGURATION, USER CAN CHANGE VARIABLES HERE
*************************************************************************************/
//ADC channels to read, TODO: make this more robust
int chan0; //voltage adc reading channel
int chan1; //current sense channel
//int chan2; //battery voltage
//int chan3; //unused

//Other
double scale = 0.078127104;	//ADC scale {0.312504320 0.156254208 0.078127104}
int current_setpoint = 11;	//current setpoint 100uA-2000uA (0-19, 100uA) TODO, make this better
int i_setpoint;
int cycles = 1000;			//specify how many cycles to run
//NODAL_NUM 				//change this in eit.h


/************************************************************************************
* MAIN
*************************************************************************************/
int main()
{
	printf("\n entered MAIN...");
	fflush(stdout);
	
	//Sigint setup, needs work
	signal(SIGINT, sigint);
	printf("\n setup SIGINT...");
	fflush(stdout);

	/*******************************
	* TAKE USER INPUT
	********************************/
	//Current setpoint
	printf("\n Choose current setpoint (100uA-2000uA in 100uA steps): ");
	scanf("%d", &i_setpoint);
	if (i_setpoint < 100 || i_setpoint > 2000 || i_setpoint%100 != 0) 
	{
		fprintf(stderr, "\nWARNING: incorrect usage, invalid current setpoint");
		exit(1);
	}
	fflush(stdout);
	current_setpoint = (i_setpoint/100) - 1; 

	;

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
		fprintf(stderr, "\n gpio_init failed with %i", gpio_errno);
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

	ti_adc_set_scale(2, scale); //chan2
	ti_adc_set_offset(2, 0);

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
	printf("\n current set to %d uA...",(current_setpoint+1)*100);
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
	* set up data writing
	***********************************/
	fp = fopen(VOLT_DATA_TEXT,"w");
	printf("\n Data file opened...");
 	char raw_buf[8];

	/**********************************
	* EXECUTE SAMPLING
	***********************************/
	struct timeval t1, t2; //time stuff 
	gettimeofday(&t1, NULL);

	printf("\n\n BEGINNING sampling: %d cycles...\n",cycles);
	fflush(stdout);

	int count = 0;
	while(count < cycles){
		count++;

		// printf("\n\n\n******************** Cycle %d *************************\n\n",count);

		//Outer loop, move current and ground
		for(i = 0; i < NODAL_NUM; i++){

			// printf("--------------Current Configuration: Current at node %d, GND at node %d ------------\n", current_mux[i]+1, ground_mux[i]+1);
			// fflush(stdout);

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

			//THIS IS THE ONE THAT WORKS
			/*******************************************************************
			// //Inner loop, measure voltage
			// int j;
			// for(j = 0; j < (NODAL_NUM-2); j++){
			// 	for(k = 0; k < MUX_PINS; k++){
			// 		if(CHAN[voltage_mux[i][j]][k]==1){
			// 			gpio_set(voltage_mux_gpio_info[k]);
			// 		}
			// 		else{
			// 			gpio_clear(voltage_mux_gpio_info[k]);
			// 		}
			// 	}
			// 	//enabling muxs
			// 	for(n = 0;n < 3; n++){
			// 		gpio_clear(mux_disable_gpio_info[n]);
			// 	}
			// 	// //test current switches
			// 	// printf("\n Switching currents...");
			// 	// fflush(stdout);
			// 	// int q;
			// 	// for(q=0;q<20;q++){
			// 	// 	printf("\n\n current set to %d uA...",(q+1)*100);
			// 	// 	fflush(stdout);
			// 	// 	for(i = 0; i< 10; i++){
			// 	// 		if(CURRENT[q][i]==1){
			// 	// 			gpio_set(current_switch_gpio_info[i]);
			// 	// 		}
			// 	// 		else{
			// 	// 			gpio_clear(current_switch_gpio_info[i]);
			// 	// 		}
			// 	// 	}
			// 	// 	usleep(2*1e6);
			// 	// }
			// 	//read ADC
			// 	chan0 = ti_adc_read_raw(0);
			// 	chan1 = ti_adc_read_raw(1);
		 	//  // printf("Voltage at node %d:  %0.5f V\n", voltage_mux[i][j]+1,chan0*scale/1000);
		 	//  fflush(stdout);
		 	//	record adc raw voltage into buffer (must be an int)
				
		     	// printf("\ninserted Array...");
		        // fflush(stdout);
			*************************************************************************************/

			/*************************************************************************************
			THIS IS THE ONE BEING TESTED
			*************************************************************************************/
			//Inner loop, measure voltage
			int j;
			for(j = 0; j < (NODAL_NUM); j++){
				
				if(i==j || ground_mux[i] == current_mux[j]){
          				//writing 0 to text file
          				strcpy(raw_buf, "0");
		      			strcat(raw_buf, "\n");
		      			fputs(raw_buf,fp);
				}

				else{
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

					//read ADC
					//chan0 = ti_adc_read_raw(0);
          
          				//writing adc measurement to file
          				//strcpy(raw_buf, ti_adc_read_raw(0));
		      			//strcat(raw_buf, "\n");
	      				fputs(ti_adc_read_raw(0),fp);
          
          
			        // printf("Voltage at node %d:  %0.5f V\n", voltage_mux[i][j]+1,chan0*scale/1000);
				//	fflush(stdout);
				}
				
				// //record adc raw voltage into buffer (must be an int)
				// insertArray(&dynamic_buffer,chan0);
				// size++;
		    /********************************************************************************************/

				// if(size==1){
				// 	pthread_create(&data_exporting_thread, NULL, data_exporting, (void*) NULL);
				// }
				// printf("\npthread created...");
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
	printf("\n\n DONE SAMPLING %d nodes, %d cycles in %0.5f seconds: Avg. cyclic frequency: %0.5f\n",NODAL_NUM, cycles, usec/1e6, cycles/(usec/1e6));
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

	printf("\n detached all gpio pins");
	fflush(stdout);

	gpio_finish();
	printf("\n closed gpiolib cleanly...");
	fflush(stdout);

	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);
  
	fclose(fp);
  
  	printf("Converting and formatting datan\n");
 	data_conversion();
	printf("Conversion/Formatting complete\n");
	//fflush(stdout);
}


/************************************************************************************
* SIGINT HANDLER, TODO: improve
*************************************************************************************/
void sigint(int s __attribute__((unused))) {
	printf("\n Received SIGINT: \n");
	fflush(stdout);

	printf("\n Exiting cleanly...");
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

	fclose(fp);
  	printf("Converting and formatting datan\n");
  	data_conversion();
	printf("Conversion/Formatting complete\n");
	exit(0);
}

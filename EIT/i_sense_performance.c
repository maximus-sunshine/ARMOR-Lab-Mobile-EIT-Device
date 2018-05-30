/************************************************************************************
 * MAE 156B Spring 2018 Team 6
 *	- Maxwell Sun
 *	- Matthew Williams
 *	- Aaron Gunn
 *	- Jacob Rutheiser
 *
 * (5/20/18) edited version of eit_sample.c to run current sensor performance test
 *	- sets up muxes in first config and turns on current source to desired setpoint
 *	- takes 1 argument, current source setpoint (0-19, 100-2000uA)
 *	- example usage: "sudo ./i_sense_performance.c 4" 
 *		-runs script with 500uA
 *
 * (5/20/18) updated declaration of voltage_mux to be a square matrix (the way Ken's lab does it)
 *	
 * compile with "gcc -pthread i_sense_performance.c src/eit.c src/gpiolib.c src/ti-ads8684.c -o i_sense_performance"
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
* SETUP
*************************************************************************************/
//mux pin array declarations
int current_mux[NODAL_NUM];              // current                                           
int ground_mux[NODAL_NUM];               // ground
int voltage_mux[NODAL_NUM][NODAL_NUM]; // voltage sampling

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
//int chan0; //voltage adc reading channel
int chan1; //current sense channel
//int chan2; //battery voltage
//int chan3; //unused

//Other
double scale = 0.078127104;	//ADC scale {0.312504320 0.156254208 0.078127104}
int cycles = 1000;			//specify how many cycles to run
int i_setpoint, config, print_flag;
//NODAL_NUM 				//change this in eit.h

/************************************************************************************
* MAIN
*************************************************************************************/
//int main(int argc, char **argv)
int main()
{
	//Take some user input TODO: add error handling
	printf("\n Choose current setpoint (100uA-2000uA in 100uA steps): ");
	scanf("%d", &i_setpoint);
	if (i_setpoint < 100 || i_setpoint > 2000 || i_setpoint%100 != 0) 
	{
		fprintf(stderr, "\nWARNING: incorrect usage, invalid current setpoint");
		exit(1);
	}
	fflush(stdout);

	printf("\n Choose where to inject current (node 1-%d): ",NODAL_NUM);
	scanf("%d", &config);
	if (config < 1 || config > NODAL_NUM) 
	{
		fprintf(stderr, "\nWARNING: incorrect usage, invalid node selection");
		exit(1);
	}
	fflush(stdout);

	printf("\n Print data to screen? (0 or 1): ");
	if (print_flag != 0 && print_flag != 1) 
	{
		fprintf(stderr, "\nWARNING: incorrect usage, enter 0 or 1");
		exit(1);
	}
	scanf("%d", &print_flag);
	fflush(stdout);

	/*******************************
	* Error handling for input args
	********************************/
	// if (argc != 2) 
	// {
	// 	fprintf(stderr, "\nWARNING: incorrect usage, wrong number of arguments\n\n usage: sudo %s <current setpoint (0-19)>\n\n", argv[0]);
	// 	exit(1);
	// }

	// if (atoi(argv[1]) < 0 || atoi(argv[1]) > 19) 
	// {
	// 	fprintf(stderr, "\nWARNING: incorrect usage, current setpoint out of bounds\n\n usage: sudo %s <current setpoint (0-19)>\n\n", argv[0]);
	// 	exit(1);
	// }


	printf("\n entered MAIN...");
	fflush(stdout);
	
	//Sigint setup, needs work
	signal(SIGINT, sigint);
	printf("\n setup SIGINT...");
	fflush(stdout);

	/**************************
	* allocate memory for gpio_info structs
	**************************/	
	int i;
	for(i = 0; i < MUX_PINS; i++)
	{
		current_mux_gpio_info[i] = malloc(sizeof(gpio_info));
		ground_mux_gpio_info[i] = malloc(sizeof(gpio_info));
		voltage_mux_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	for(i = 0; i < 10; i++)
	{
		current_switch_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	for(i = 0; i < 3; i++)
	{
		mux_disable_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	/**************************
	* SET UP GPIO PINS
	**************************/
	//initialize gpio_lib
	if(gpio_init())
	{
		fprintf(stderr, "\n gpio_init failed with %i", gpio_errno);
	}
	printf("\n gpiolib intialized...");
	fflush(stdout);

	//attach current gpio pins
	int bank, mask;
	for(i = 0; i < MUX_PINS; i++)
	{
		bank = current_mux_gpio[i]/32;
		mask = bit(current_mux_gpio[i]%32);
		current_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	}
	//attach ground gpio pins
	for(i = 0; i < MUX_PINS; i++)
	{                            
		bank = ground_mux_gpio[i]/32;
		mask = bit(ground_mux_gpio[i]%32);
		ground_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	}
	//attach voltage gpio pins
	for(i = 0; i < MUX_PINS; i++)
	{                            
		bank = voltage_mux_gpio[i]/32;
		mask = bit(voltage_mux_gpio[i]%32);
		voltage_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
	}
	//attach current switch gpio pins
	for(i = 0; i < 10; i++)
	{                            
		int bank = current_switch_gpio[i]/32;
		int mask = bit(current_switch_gpio[i]%32);
		current_switch_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
	}
	//attach mux enable gpio pins
	for(i = 0; i < 3; i++)
	{                            
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

	/**********************************
	* Disable Muxs
	***********************************/
	int n,k;
	for(n = 0;n < 3; n++)
	{
		gpio_set(mux_disable_gpio_info[n]);
	}

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

	//TODO: put scales and offsets in header file
	ti_adc_set_scale(1, scale); //chan2
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
	* EXECUTE SAMPLING
	***********************************/
	printf("\n\n Setting muxes...\n");
	fflush(stdout);

	i = config-1;		//set configuration
	//i = 0;			//set configuration

	//Set current and ground
	for(k = 0; k < MUX_PINS; k++)
	{   
		if(CHAN[current_mux[i]][k]==1)
		{
			gpio_set(current_mux_gpio_info[k]);
		}
		else
		{
			gpio_clear(current_mux_gpio_info[k]);
		}

		if(CHAN[ground_mux[i]][k]==1)
		{
			gpio_set(ground_mux_gpio_info[k]);
		}
		else
		{
			gpio_clear(ground_mux_gpio_info[k]);
		}
	}

	// //Inner loop, measure voltage
	// int j=0;
	// for(k = 0; k < MUX_PINS; k++)
	// {
	// 	if(CHAN[voltage_mux[i][j]][k]==1)
	// 	{
	// 		gpio_set(voltage_mux_gpio_info[k]);
	// 	}
	// 	else
	// 	{
	// 		gpio_clear(voltage_mux_gpio_info[k]);
	// 	}
	// }

	printf("--------------Current Configuration: Current at node %d, GND at node %d ------------\n", current_mux[i]+1, ground_mux[i]+1);
	fflush(stdout);

	//enabling muxs
	for(n = 0;n < 3; n++)
	{
		gpio_clear(mux_disable_gpio_info[n]);
	}
	printf("\n muxes enabled...");
	fflush(stdout);

	//set current
	int current_setpoint = (i_setpoint/100)-1;	//current setpoint 100uA-2000uA (0-19, 100uA) TODO, make this better
	printf("\n current set to %d uA...",(current_setpoint+1)*100);
	fflush(stdout);

	for(i = 0; i< 10; i++)
	{
		if(CURRENT[current_setpoint][i]==1)
		{
			gpio_set(current_switch_gpio_info[i]);
		}
		else
		{
			gpio_clear(current_switch_gpio_info[i]);
		}
	}
	
	//read ADC
	if(print_flag)
	{
		printf("\n\n Current sensor voltage (V):\n");
		fflush(stdout);
		for(int q=0;q<1000;q++)
		{
			printf("%0.5f\n", ti_adc_read_raw(1)*scale/1000);
			fflush(stdout);
		}
	}

	//Sleep for a little
	printf("\n\n setpoint: %d uA. Pausing for manual ammeter reading...\n",(current_setpoint+1)*100);
	fflush(stdout);
	usleep(5*1e6);

	//Cleanup
	printf("\n\nDone, cleaning up...");
	fflush(stdout);

	for(i=0;i<MUX_PINS;i++)
	{
		gpio_detach(current_mux_gpio_info[i]);
		gpio_detach(ground_mux_gpio_info[i]);
		gpio_detach(voltage_mux_gpio_info[i]);
	}	
  
	for(i=0;i<10;i++)
	{
		gpio_detach(current_switch_gpio_info[i]);
		
	}	
	for(i=0;i<3;i++)
	{
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
	
	printf("\n FINISHED!\n\n");
	fflush(stdout);
}

/************************************************************************************
* SIGINT HANDLER, TODO: improve
*************************************************************************************/
void sigint(int s __attribute__((unused))) 
{
	printf("\n Received SIGINT: \n");
	fflush(stdout);

	printf("\n Exiting cleanly...");
	fflush(stdout);

	//Cleanup
	int i;
	for(i=0;i<MUX_PINS;i++)
	{
		gpio_detach(current_mux_gpio_info[i]);
		gpio_detach(ground_mux_gpio_info[i]);
		gpio_detach(voltage_mux_gpio_info[i]);
	}	

	for(i=0;i<10;i++)
	{
		gpio_detach(current_switch_gpio_info[i]);
	}	
	for(i=0;i<3;i++)
	{
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
	
	printf("\n done...\n\n");
	fflush(stdout);
	exit(0);
}

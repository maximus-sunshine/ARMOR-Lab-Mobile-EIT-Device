/************************************************************************************
 * MAE 156B Spring 2018 Team 6
 *	- Maxwell Sun
 *	- Matthew Williams
 *	- Aaron Gunn
 *	- Jacob Rutheiser
 *
 * Script to sample sensing skin.
 * 
 * Using gpio_lib (https://bitbucket.org/vanguardiasur/gpiolib) for GPIO toggling (~3 MHz)
 * 
 * Using sysfs to read ADC (best ~15 kHz), need to improve (add buffer to adc driver?)
 *
 * TODO: -find faster way to read ADC
 *		 -clean up code, move stuff to header file
 *       -error handling
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
 
#include <assert.h>		//for gpiolib stuff
#include <signal.h>
#include <sys/time.h>
#include "gpiolib.h"

/************************************************************************************
* DEFINES
*************************************************************************************/
#define NODAL_NUM 32			//# of nodes, can be 8,12,16,20,24,and 32
#define side_len (NODAL_NUM/4)	//# of nodes per side
#define MAX_BUF 64				//max buffer length
#define VOLT_CHANNEL 2          //adc channel to read voltage

/************************************************************************************
* SETUP
*************************************************************************************/

/* MUX */
//logic array declaration {A4, A3, A2, A1, A0}
int chan[32][5] = {{0,0,0,0,0},{0,0,0,0,1},{0,0,0,1,0},{0,0,0,1,1}, 
                   {0,0,1,0,0},{0,0,1,0,1},{0,0,1,1,0},{0,0,1,1,1},
                   {0,1,0,0,0},{0,1,0,0,1},{0,1,0,1,0},{0,1,0,1,1},
                   {0,1,1,0,0},{0,1,1,0,1},{0,1,1,1,0},{0,1,1,1,1},
                   {1,0,0,0,0},{1,0,0,0,1},{1,0,0,1,0},{1,0,0,1,1},
                   {1,0,1,0,0},{1,0,1,0,1},{1,0,1,1,0},{1,0,1,1,1},
                   {1,1,0,0,0},{1,1,0,0,1},{1,1,0,1,0},{1,1,0,1,1},
                   {1,1,1,0,0},{1,1,1,0,1},{1,1,1,1,0},{1,1,1,1,1}};

//mux array declarations
int current_mux[NODAL_NUM];              // current?                                           
int ground_mux[NODAL_NUM];               // ground?
int voltage_mux[NODAL_NUM][NODAL_NUM-2]; // voltage sampling?

//geometry
int node_index = 3*(side_len); //starting node_index of ground

// gpio pin IDs, used for exporting pins 
// gpio pin ID = gpio_bank*32 + gpio_pinmask (e.g. gpio 86 = gpio2_22 (86 = 2*32 + 22))
// mux channels {A4, A3, A2, A1, A0}
int current_mux_gpio[5] = { 86,  87, 10,  9,  8};
int ground_mux_gpio[5]  = { 78,  76, 74, 72, 70};
int voltage_mux_gpio[5] = {125, 117, 14, 49, 15};

int adc_reset_gpio      = 13;
int i_sense_reset_gpio  = 27;

//TODO: Add other GPIOs

/************************************************************************************
* MAIN
*************************************************************************************/
int main(){
	/**************************
	* INITIALIZE ADC INTERFACE
	**************************/	
	ti_adc_init();

	/**************************
	* SET UP GPIO PINS
	**************************/
	//initialize gpio_lib
	if(gpio_init()){
		fprintf(stderr, "gpio_init failed with %i\n", gpio_errno);
	}

	//setup for attaching gpio pins
	struct gpio_info current_mux_gpio_info[sizeof(current_mux_gpio)];
	struct gpio_info ground_mux_gpio_info[sizeof(current_mux_gpio)];
	struct gpio_info voltage_mux_gpio_info[sizeof(current_mux_gpio)];
	
	//attach current gpio pins
	int i;
	for(i=0;i<sizeof(current_mux_gpio);i++){
		int bank = current_mux_gpio[i]/32;
		int mask = current_mux_gpio[i]%32;
		current_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	}

	//attach ground gpio pins
	int i;
	for(i=0;i<sizeof(ground_mux_gpio);i++){                            
		int bank = ground_mux_gpio[i]/32;
		int mask = ground_mux_gpio[i]%32;
		ground_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	}

	//attach voltage gpio pins
	int i;
	for(i=0;i<sizeof(voltage_mux_gpio);i++){                            
		int bank = voltage_mux_gpio[i]/32;
		int mask = voltage_mux_gpio[i]%32;
		voltage_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
	}

	//attach other gpio pins
	adc_reset_gpio_info = gpio_attach(adc_reset_gpio/32, adc_reset_gpio%32, GPIO_OUT);
	i_sense_reset_gpio  = gpio_attach(i_sense_reset_gpio/32, i_sense_reset_gpio%32, GPIO_OUT);
	
	/**********************************
	* SET UP MUX SWITCHING PATTERN
	***********************************/
	//configures current and ground nodes according to # of nodes(NODAL_NUM)
	int n;
	for(n = 0; n < NODAL_NUM; n++){
	  ground_mux[n] = node_index;				//ground starts at last node of third side
	  current_mux[n] = n;						//current starts at first node and increments to the end
	  node_index  = node_index - 1;				//ground moves CC
	  if((node_index % (side_len))==0){			//once it passes an edge node it adds half the # of nodes to node_index
	    node_index = node_index + (NODAL_NUM/2);
	    if (node_index > NODAL_NUM){			//if node_index ends up being greater then NODAL_NUM it takes the remainder
	      node_index = node_index % NODAL_NUM;
	    }
	  }
	}

	//configures voltage sampling nodes according to # of nodes(NODAL_NUM)
	int k = 0;
	int a,b;
	for(a = 0; a < NODAL_NUM; a++){
	  for(b = 0; b < NODAL_NUM; b++){
	    if((a != b) && (ground_mux[a] != current_mux[b])){
	      voltage_mux[a][k] = current_mux[b];
	      k++;
	    }
	  }
	  k = 0;
	}

	/**********************************
	* EXECUTE SAMPLING
	***********************************/
	int flag = 0;
  	//runs 200 times
	while(flag < 200){
	for(i = 0; i < NODAL_NUM; i++){
    	//power and ground distribution
		for(k=0;k<sizeof(ground_mux_gpio);k++){                            
			if(chan[current_mux[i]][k]==1){
				gpio_set(current_mux_gpio_info,current_mux_gpio[k]);
			}
			else{
				gpio_clear(current_mux_gpio_info,current_mux_gpio[k]);
			}

			if(chan[ground_mux[i]-1][k]==1){
				gpio_set(ground_mux_gpio_info,ground_mux_gpio[k]);
			}
			else{
				gpio_clear(ground_mux_gpio_info,ground_mux_gpio[k]);
			}
		}

		//inner loop controls sampling
		int j;
		for(j =0; j <= (NODAL_NUM-3); j++){
			if(chan[volt_mux[i]][k]==1){
				gpio_set(voltage_mux_gpio_info,voltage_mux_gpio[k]);
			}
			else{
				gpio_clear(voltage_mux_gpio_info,voltage_mux_gpio[k]);
			}

	        //reading adc
	        //TODO: implement fast method to store values, (e.g. store values in temporary buffer and write to .txt file at low priority, pthread?)
	        ti_adc_read_raw(VOLT_CHANNEL);
      	}

        printf("\n");
        //TODO: print correct configuration
        printf("--------------Current Configuration: Current at node %d, GND at node %d, Measure at node %d ------------------ \n", i, i, i);
    }
      flag++;
      printf(" ******************** Cycle %d *************************",flag);
  }



}
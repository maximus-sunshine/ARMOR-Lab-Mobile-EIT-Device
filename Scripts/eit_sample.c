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
 * compile with "gcc -pthread eit_sample.c eit.c gpiolib.c ti-ads8684.c -o eit_sample"
 * (5/13/18) Matthew updated to match the one we lost on BBB
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
* PTHREAD SETUP
*************************************************************************************/
pthread_t data_exporting_thread;
//pthread function declaration
void* data_exporting(void *ptr);
//data file declaration
//FILE* fp;

/************************************************************************************
* SETUP SIGINT HANDLER
*************************************************************************************/
void sigint(int s __attribute__((unused)));

/************************************************************************************
* SETUP PTHREADS
*************************************************************************************/
void* write_data(void *ptr);
pthread_t write_data_thread;

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
int mux_enable_gpio[3] = MUX_ENABLE_GPIO;

//current switches 
int current_switch_gpio[10] = CURRENT_SWITCH_GPIO;

int adc_reset_gpio      = ADC_RESET_GPIO;
int i_sense_reset_gpio  = I_SENSE_RESET_GPIO;

int chan0; //voltage adc reading channel

//create structs for mux logic gpios
gpio_info *current_mux_gpio_info[MUX_PINS];
gpio_info *ground_mux_gpio_info[MUX_PINS];
gpio_info *voltage_mux_gpio_info[MUX_PINS];

gpio_info *current_switch_gpio_info[10];
gpio_info *adc_reset_gpio_info;
gpio_info *i_sense_reset_gpio_info;

gpio_info *mux_enable_gpio_info[3]

//allocate memory for gpio_info structs
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
		mux_enable_gpio_info[i] = malloc(sizeof(gpio_info));
	}

double scale = 0.078127104;

//struct declarations
Array dynamic_buffer;
size_t init_size = 100;
int size = 0;
int flag = 1;

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
		gpio_detach(mux_enable_gpio_info[i]);
		
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
	
	printf("waiting for pthread to join\n");
	flag = 0;
	pthread_join(data_exporting_thread, NULL);
	printf("pthread has returned %d\n",size);
	//fclose(fp);
	printf("file has closed\n");
	exit(0);
}

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
	
	/**************************
	* INITIALIZE BUFFER ARRAY
	**************************/	
	initArray(&dynamic_buffer,init_size);
	
	/**************************
	* INITIALIZE ADC INTERFACE
	**************************/	
	ti_adc_init();
	printf("\n ADC interface initialized...");
	fflush(stdout);

	ti_adc_enable();
	printf("\n ADC reset pin set...");
	fflush(stdout);

	//TODO: put scales and offsets in header file
	ti_adc_set_scale(0, scale);
	ti_adc_set_offset(0, 0);

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
	int i, bank, mask;
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
		int bank = mux_enable_gpio[i]/32;
		int mask = bit(mux_enable_gpio[i]%32);
		mux_enable_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
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
	int current_setpoint = 0; //current setpoint 100uA-2000uA(0-19, 100uA)
	for(i = 0; i< 10; i++){
		if(Current[current_setpoint][i]==1){
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

	int flag_2 = 0;
	int cycles = 1;
  //runs "cycles" times
  
	/**********************************
	* Disabling Muxs
	***********************************/
	int n;
	for(n = 0;n < 3; n++){
		gpio_clear(mux_enable_gpio_info[n]);
	}

	while(flag_2 < cycles){
		
		printf("\n\n\n******************** Cycle %d *************************\n\n",flag_2);

	
		
		//Outer loop, move current and ground
		for(i = 0; i < NODAL_NUM; i++){
			
			printf("--------------Current Configuration: Current at node %d, GND at node %d ------------\n", current_mux[i]+1, ground_mux[i]+1);
			fflush(stdout);

			//mux disable


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
						gpio_clear(voltage_mux_gpio_info[j]);
					}
				}
				//enabling muxs
				for(n = 0;n < 3; n++){
					gpio_set(mux_enable_gpio_info[n]);
				}
				chan0 = ti_adc_read_raw(0);
				//read ADC
		        printf("Voltage at node %d:  %0.5f V\n", voltage_mux[i][j]+1,chan0*scale/1000);
				//record adc raw voltage into buffer (must be an int)
			insertArray(&dynamic_buffer,chan0);
			size++;
			if(size==1){
				pthread_create(&data_exporting_thread, NULL, data_exporting, (void*) NULL);
			}
			//disabling muxs
			for(n = 0;n < 3; n++){
				gpio_clear(mux_enable_gpio_info[n]);
			}
		        usleep(1 * 1e6);
	      	}
	    }
	    cycles++;
 	}
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
		gpio_detach(mux_enable_gpio_info[i]);
		
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
	
	printf("waiting for pthread to join\n");
	flag = 0;
	pthread_join(data_exporting_thread, NULL);
	printf("pthread has returned %d\n",size);
	//fclose(fp);
	printf("file has closed\n");
}

	printf("\n FINISHED!\n\n");
	fflush(stdout);

void* data_exporting(void *ptr){
	///fp = fopen(VOLT_DATA_TXT,"a");
	int i = 0;
	
	while(i < size){
	   // fprintf(fp,"%d\n",dyanamic_buffer.array[i]);
	      printf("pthread recorded %d value", dynamic_buffer.array[i]);
	    i++;
	    if(flag ==1){
	        usleep(30*1000);
	    }    
	}
	
	printf("thread is returning\n");
	return NULL;
}

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
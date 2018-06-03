/***************************************************************************
 * ------------------------------------------------------------------------
 * ARMOR Lab @UC San Diego, Kenneth Loh Ph.D
 * 
 * MAE 156B Spring 2018 Team 6: Warfighter Protection
 * 	- Maxwell Sun		(maxsun96@gmail.com)
 *	- Jacob Rutheiser	(jrutheiser@gmail.com)
 *	- Matthew Williams	(mwilliams31243@gmail.com)
 *	- Aaron Gunn		(gunnahg@gmail.com)
 * ------------------------------------------------------------------------
 * 
 * Main.c (compile with 'gcc -pthread Main.c src/eit.c src/gpiolib.c src/ti-ads8684.c -o Main')
 * 
 * The Big Kahuna. This is the script that runs on boot. It does everything.
 ***************************************************************************/

/************************************************************************************
* INCLUDES
*************************************************************************************/
#include "Main.h"

/************************************************************************************
* STRUCTS
*************************************************************************************/
config_t config = {
    .nodal_num		= 32,
    .adc_scale		= {0.078127104,0.078127104,0.078127104,0.078127104},
    .adc_offset		= {0,0,0,0},
    .channels		= {1,0,0,0},
    .sample_mode	= CYCLES,
    .time			= 20,
    .cycles			= 100,
    .sample_geom	= ACROSS,
    .i_setpoint		= 100,
};

/************************************************************************************
* SETUP
*************************************************************************************/
//mux array declarations
int current_mux[NODAL_NUM];				// current                                           
int ground_mux[NODAL_NUM];				// ground
int voltage_mux[NODAL_NUM][NODAL_NUM];	// voltage

// // gpio pin IDs, see eit_config.h
// int current_mux_gpio[MUX_PINS]			= CURRENT_MUX_GPIO;
// int ground_mux_gpio[MUX_PINS]			= GROUND_MUX_GPIO;
// int voltage_mux_gpio[MUX_PINS]			= VOLTAGE_MUX_GPIO;
// int current_switch_gpio[I_SWTCH_PINS]	= CURRENT_SWITCH_GPIO;
// int mux_disable_gpio[3]					= MUX_DISABLE_GPIO;
// int adc_reset_gpio 						= ADC_RESET_GPIO;
// int i_sense_reset_gpio					= I_SENSE_RESET_GPIO;

// //gpio_info structs for all GPIO pins
// gpio_info *current_mux_gpio_info[MUX_PINS];			//mux logic pins
// gpio_info *ground_mux_gpio_info[MUX_PINS];
// gpio_info *voltage_mux_gpio_info[MUX_PINS];
// gpio_info *mux_disable_gpio_info[3];				//mux disable pins
// gpio_info *current_switch_gpio_info[I_SWTCH_PINS];	//current source switch logic pins
// gpio_info *adc_reset_gpio_info;						//ADC RST pin, must be high for ADC to work
// gpio_info *i_sense_reset_gpio_info;					//current sense RST pin

long elapsed_time;
int count, data, n, k;
struct timeval t1, t2;

//data storage
FILE* fp;

/************************************************************************************
* PTHREADS
*************************************************************************************/
//battery reading thread
pthread_t batt_read_thread;
void* batt_read(void *ptr);

/************************************************************************************
* FUNCTION DECLARATIONS
*************************************************************************************/
int sample();

/************************************************************************************
* MAIN
*************************************************************************************/
int main(){

	printf("\n entered MAIN...");
	fflush(stdout);

	signal(SIGINT, sigint);
	printf("\n setup SIGINT...");
	fflush(stdout);

	/* INITIALIZE LIBRARIES */
	//initialize gpio_lib
	if(gpio_init()){
		fprintf(stderr, "\n gpio_init failed with %i", gpio_errno);
		exit(1);
	}
	printf("\n gpiolib intialized...");
	fflush(stdout);	

	//initialize ADC library
	if(ti_adc_init()<0){
		fprintf(stderr, "\n ti_adc_init failed\n");
		exit(1);
	};
	printf("\n ADC interface initialized...");
	fflush(stdout);

	/* SETUP HARDWARE */
	//allocate memory for gpio_info structs
	int i;
	for(i = 0; i < MUX_PINS; i++){
		current_mux_gpio_info[i] = malloc(sizeof(gpio_info));
		ground_mux_gpio_info[i] = malloc(sizeof(gpio_info));
		voltage_mux_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	for(i = 0; i < I_SWTCH_PINS; i++){
		current_switch_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	for(i = 0; i < 3; i++){
		mux_disable_gpio_info[i] = malloc(sizeof(gpio_info));
	}

	//attach gpio pins
	if(attach_all_gpio()<0){
		perror("\nERROR: unable to attach gpio pins, exiting...\n");
		fprintf(stderr, "maybe device tree is too old or pin is already exported");	
		exit(1);
	}
	printf("\n gpio pins attached...");
	fflush(stdout);

	//enable ADC
	gpio_set(adc_reset_gpio_info);
	printf("\n ADC enabled...");
	fflush(stdout);

	/* START PTHREAD TO READ BATTERY */	
	pthread_create(&batt_read_thread, NULL, batt_read, (void*) NULL);
	printf("\n battery reading pthread created...");
	fflush(stdout);

	/* ENTER UI */
	//switch
	//case
	sample();

	/* CLEANUP */
	for(i=0;i<MUX_PINS;i++){
		gpio_detach(current_mux_gpio_info[i]);
		gpio_detach(ground_mux_gpio_info[i]);
		gpio_detach(voltage_mux_gpio_info[i]);
	}	
	for(i=0;i<I_SWTCH_PINS;i++){
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

	printf("\n file has closed");
	printf("\n FINISHED!\n\n");
	fflush(stdout);
}

/************************************************************************************
* FUNCTIONS
*************************************************************************************/
int sample()
{
	//set ADC scales and offsets
	int i;
	for(i=0;i<CHANNELS;i++){
		if(ti_adc_set_scale(i, config.adc_scale[i])<0){
			perror("\nERROR: ti_adc_set_scale failed\n");
			fprintf(stderr, "channel %d failed", i);	
			return -1;
		}
		if(ti_adc_set_offset(i, config.adc_offset[i])<0){
			perror("\nERROR: ti_adc_set_offset failed\n");
			fprintf(stderr, "channel %d failed", i);
			return -1;
		}
	}
	printf("\n ADC scales and offsets configured...");
	fflush(stdout);

	//configure mux switching patterns
	cur_gnd_config(current_mux,ground_mux);
	volt_samp_config(current_mux,ground_mux,voltage_mux);
	printf("\n mux switching patterns configured...");
	fflush(stdout);

	//disable muxes for safety
	for(i = 0;i < 3; i++){
		gpio_set(mux_disable_gpio_info[i]);
	}
	printf("\n muxes disabled...");
	fflush(stdout);

	//turn on current source
	int current_setpoint = (config.i_setpoint/100)-1;
	for(i = 0; i< 10; i++){
		if(CURRENT[current_setpoint][i]==1){
			gpio_set(current_switch_gpio_info[i]);
		}
		else{
			gpio_clear(current_switch_gpio_info[i]);
		}
	}
	printf("\n current set to %d",config.i_setpoint);
	fflush(stdout);

	printf("\n\nBEGINNING sampling!");
	fflush(stdout);

	//execute sampling
	while((config.sample_mode == TIMED && elapsed_time < config.time) || (config.sample_mode == CYCLES && count < config.cycles) || config.sample_mode == CONTINUOUS)
	{
		printf("\n\n\n******************** Cycle %d *************************\n\n",count);
		fflush(stdout);

		//outer loop, move current and ground
		for(i = 0; i < NODAL_NUM; i++){
			printf("--------------Current Configuration: Current at node %d, GND at node %d ------------\n", current_mux[i]+1, ground_mux[i]+1);
			fflush(stdout);

			//set current and ground mux logic pins
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

			//inner loop, measure voltage
			int j;
			for(j = 0; j < (NODAL_NUM); j++){
				
				if(i==j || ground_mux[i] == current_mux[j]){
					data = 0;
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

					//enable muxs
					for(n = 0;n < 3; n++){
						gpio_clear(mux_disable_gpio_info[n]);
					}

					//read ADC
					data = ti_adc_read_raw(NODE);
			        printf("Voltage at node %d:  %0.5f V\n", voltage_mux[i][j]+1,data*config.adc_scale[0]/1000);
					fflush(stdout);
				}

				//disable muxs
				for(n = 0;n < 3; n++){
					gpio_set(mux_disable_gpio_info[n]);
				}

				// if(j==NODAL_NUM-1){
				// 	fprintf(fp,"%0.5f\n",chan0*scale/1000);
				// }
				// else{
				// 	fprintf(fp,"%0.5f\t",chan0*scale/1000);
				// }
			}
		}
	gettimeofday(&t2, NULL);
	elapsed_time = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)/1e6;
	count++;
	}
 	//Print timing data to screen
	gettimeofday(&t2, NULL);
	long usec = 1e6 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec;
	printf("\n\n DONE SAMPLING %d nodes, %d cycles in %0.5f seconds: Avg. cyclic frequency: %0.5f\n",NODAL_NUM, count, usec/1e6, count/(usec/1e6));
	fflush(stdout);
	return 0;
}

/************************************************************************************
* PTHREADS
*************************************************************************************/
void* batt_read(void *ptr){

}
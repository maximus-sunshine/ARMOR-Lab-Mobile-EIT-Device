/***************************************************************************
 * --------------------------------------------------------------------------
 * ARMOR Lab @UC San Diego, Kenneth Loh Ph.D
 * 
 * MAE 156B Spring 2018 Team 6: Warfighter Protection
 * 	- Maxwell Sun		(maxsun96@gmail.com)
 *	- Jacob Rutheiser	(jrutheiser@gmail.com)
 *	- Matthew Williams	(mwilliams31243@gmail.com)
 *	- Aaron Gunn		(gunnahg@gmail.com)
 * -------------------------------------------------------------------------
 * 
 * Main.c (compile with 'gcc -pthread Main.c src/eit.c src/gpiolib.c src/ti-ads8684.c src/UI.c src/example_app.c src/I2C.c src/SSD1306_OLED.c -o Main')
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
	.nodal_num		= 8,
	.adc_scale		= {0.078127104,0.078127104,0.078127104,0.078127104},
	.adc_offset		= {0,0,0,0},
	.channels		= {1,0,0,0},
	.sample_mode	= CYCLES,
	.time			= 5,
	.cycles			= 10,
	.sample_geom	= ACROSS,
	.i_setpoint		= 100,
};

state_t state = {
	.menu = HOME,
	.index = 0,
	.len = HOME_OPTS_LEN,
	.batt = 0.0,
	.system = RUNNING,
};

/************************************************************************************
* SETUP
*************************************************************************************/

//data storage
FILE* fp;

void sigint(int s __attribute__((unused)));

/************************************************************************************
* PTHREADS
*************************************************************************************/
//battery reading thread
pthread_t batt_read_thread;
void* batt_read(void *ptr);

//button poll thread
pthread_t button_poll_thread;
void* button_poll(void* ptr);

/************************************************************************************
* FUNCTION DECLARATIONS
*************************************************************************************/
int sample();
int process_button();
int update_UI();

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
	//oled reset attach
	bank = oled_reset_gpio/32;
	mask = bit(oled_reset_gpio%32);
	oled_reset_gpio_info = gpio_attach(bank, mask, GPIO_OUT);
	//oled reset attach
	bank = oled_power_gpio/32;
	mask = bit(oled_power_gpio%32);
	oled_power_gpio_info = gpio_attach(bank, mask, GPIO_OUT);

	printf("\n gpio pins attached...");
	fflush(stdout);

	//power on OLED
	gpio_set(oled_power_gpio_info);
	gpio_set(oled_reset_gpio_info);
	printf("\n OLED power and reset pins set high...");
	fflush(stdout);

    //reset OLED
	gpio_clear(oled_reset_gpio_info);
	usleep(0.5*1e6);
	gpio_set(oled_reset_gpio_info);
	printf("\n OLED display reset...");
	fflush(stdout);

    //Initialize I2C bus and connect to the I2C Device
	if(init_i2c_dev2(SSD1306_OLED_ADDR) == 0)
	{
		printf("\n (Main)i2c-2: Bus Connected to SSD1306");
		fflush(stdout);
	}
	else
	{
		printf("\n (Main)i2c-2: OOPS! Something Went Wrong");
		fflush(stdout);
		exit(1);
	}

	//enable ADC
	printf("\n enabling ADC...");
	gpio_set(adc_reset_gpio_info);
	printf("\n ADC enabled...");
	fflush(stdout);

	/* START PTHREAD TO READ BATTERY */	
	pthread_create(&batt_read_thread, NULL, batt_read, (void*) NULL);
	printf("\n battery reading pthread created...");
	fflush(stdout);

	/* Run SDD1306 Initialization Sequence */
	display_Init_seq();
	printf("\n init sequence displayed...");
	fflush(stdout);
	printf("\n OLED initialized...");
	fflush(stdout);

    /* Display ARMOR logo */
	printf("\n Displaying ARMOR logo...");
	fflush(stdout);
	display_bitmap();
	Display();
	usleep(2*1e6);

    /* Start button poll pthread */
	pthread_create(&button_poll_thread, NULL, button_poll, (void*) NULL);
	printf("\n polling buttons...");
	fflush(stdout);

	/* ENTER UI */
	printf("\n entering menu interface...");
	fflush(stdout);

	state.system = RUNNING;
	fflush(stdout);
	usleep(0.5*1e6);
	button = -1;
	while(state.system == RUNNING){
		update_UI();
	}

	/* CLEANUP */
    //display ARMOR logo
	printf("\n displaying ARMOR logo");
	fflush(stdout);
	display_bitmap();
	Display();
	usleep(2*1e6);

    //join pthreads
	pthread_join(button_poll_thread, NULL);
	pthread_join(batt_read_thread, NULL);
	printf("\n pthreads joined... \n");

    //detach gpio pins
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
	gpio_detach(oled_reset_gpio_info);
	gpio_detach(oled_power_gpio_info);

	printf("\n detached all gpio pins");
	fflush(stdout);

	//clean up gpio library
	gpio_finish();
	printf("\n closed gpiolib cleanly...");
	fflush(stdout);

	//clean up adc library
	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...");
	fflush(stdout);

	// fclose(fp);

	// printf("\n file has closed");
	printf("\n FINISHED!\n\n");
	fflush(stdout);
}

/************************************************************************************
* FUNCTIONS
*************************************************************************************/
int sample()
{
	long elapsed_time;
	int count, data, n, k;
	struct timeval t1, t2;

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
	//mux array declarations
	int current_mux[config.nodal_num];	// current                                           
	int ground_mux[config.nodal_num];	// ground
	int voltage_mux[config.nodal_num];	// voltage

	printf("\n declared mux matrices...");
	fflush(stdout);

	mux_config(config.nodal_num,current_mux,ground_mux,voltage_mux);
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
		if(I_SWITCH[current_setpoint][i]==1){
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
	gettimeofday(&t1, NULL);
	while((config.sample_mode == TIMED && elapsed_time < config.time) || (config.sample_mode == CYCLES && count < config.cycles) || config.sample_mode == CONTINUOUS)
	{
		printf("\n\n\n******************** Cycle %d *************************\n\n",count);
		fflush(stdout);

		//outer loop, move current and ground
		for(i = 0; i < config.nodal_num; i++){
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
			for(j = 0; j < (config.nodal_num); j++){
				
				if(i==j || ground_mux[i] == current_mux[j]){
					data = 0;
				}
				
				else{
					for(k = 0; k < MUX_PINS; k++){
						if(CHAN[voltage_mux[j]][k]==1){
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
					data = ti_adc_read_raw(BATTERY);
					printf("Voltage at node %d:  %0.5f V\n", voltage_mux[j]+1,data*config.adc_scale[NODE]/1000);
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
	printf("\n\n DONE SAMPLING %d nodes, %d cycles in %0.5f seconds: Avg. cyclic frequency: %0.5f\n",config.nodal_num, count, usec/1e6, count/(usec/1e6));
	fflush(stdout);
	return 0;
}

int process_button(char opt_list[][OPT_STR_LEN])
{
	switch(button) {
		case SELECT:
		mainSelect(state, opt_list);
		button = -1;
		return 1;
		break;

		case PREV:
		prevSelect();
		state.index--;
		button = -1;
		return 0;
		break;

		case NEXT:
		nextSelect();
		state.index++;
		button = -1;
		return 0;
		break;

		case BACK:
		state.index = 0;
		if(state.menu > 0) state.menu--;
		button = -1;
		return 0;
		break;

		case -1:
		return 0;
		break;
	}
}

int update_UI(){
	int len;
	switch (state.menu) {
		case HOME:
		printUI(state, HOME_OPTS);
		if(process_button(HOME_OPTS)){
			if(mod(state.index,len) == 0) sample();
			else{
				state.menu = SETTINGS;
				state.index = 0;
			}
		}
		break;

		case SETTINGS:
		state.len = SETTINGS_OPTS_LEN;
		printUI(state, SETTINGS_OPTS);
		if(process_button(SETTINGS_OPTS)){
			state.menu = mod(state.index,len) + 2;
			state.index = 0; 
		};
		break;

		case NODES:
		state.len = NODES_OPTS_LEN;
		printUI(state, NODES_OPTS);
		if(process_button(NODES_OPTS)){
			config.nodal_num = atoi(state.opts[mod(state.index,len)]);
			state.menu == SETTINGS;
			state.index = 0;
		};
		break;

		case CURRENT:
		state.len = CURRENT_OPTS_LEN;
		printUI(state, CURRENT_OPTS);
		if(process_button(CURRENT_OPTS)){
			config.i_setpoint = atoi(state.opts[mod(state.index,len)]);
			state.menu = SETTINGS;
			state.index = 1;
		};
		break;

		case CONFIG:
		state.len = CONFIG_OPTS_LEN;
		printUI(state, CONFIG_OPTS);
		if(process_button(CONFIG_OPTS)){
			config.sample_geom = mod(state.index,len);
			state.menu = SETTINGS;
			state.index = 2;
		};
		break;

		case SAMPLING:
		state.len = SAMPLING_OPTS_LEN;
		printUI(state, SAMPLING_OPTS);
		if(process_button(SAMPLING_OPTS)){
			config.sample_mode = mod(state.index,len);
			state.menu = SETTINGS;
			state.index = 3;
		};
		break;
	}
	return 0;
}

/************************************************************************************
* PTHREADS
*************************************************************************************/

/* BUTTON POLL THREAD */
void* button_poll(void* ptr){
	struct pollfd fdset[3];
	int nfds = 3;
	int gpio_fd[3], timeout, rc;
	char *buf[MAX_BUF];
	unsigned int gpio[3];
	int len;

    gpio[0] = 61; // button select
    gpio[1] = 88; // button previous
    gpio[2] = 89; // button next

    for(int i=0;i<3;i++){
    	gpio_export(gpio[i]);
    	gpio_set_dir(gpio[i], 0);
    	gpio_set_edge(gpio[i], "rising");
    	gpio_fd[i] = gpio_fd_open(gpio[i]);
    }

    timeout = POLL_TIMEOUT;

    while(state.system != EXITING){
    	memset((void*)fdset, 0, sizeof(fdset));

    	for(int i=0;i<3;i++){
    		fdset[i].fd = gpio_fd[i];
    		fdset[i].events = POLLPRI;
    	}

    	rc = poll(fdset, nfds, timeout);

    	if (rc < 0) {
    		if (errno == EINTR) {
    			printf("\nInterrupted system call... continuing\n");
    			continue;
    		}
    		perror("\npoll() failed!\n");
    		return NULL;
    	}

    	if (rc == 0) {
    		printf(".");
    	}

    	if (fdset[SELECT].revents & POLLPRI) {
    		lseek(fdset[SELECT].fd, 0, SEEK_SET);
    		len = read(fdset[SELECT].fd, buf, MAX_BUF);
    		button = SELECT;
    	}

    	if (fdset[PREV].revents & POLLPRI) {
    		lseek(fdset[PREV].fd, 0, SEEK_SET);
    		len = read(fdset[PREV].fd, buf, MAX_BUF);
    		button = PREV;
    	}

    	if (fdset[NEXT].revents & POLLPRI) {
    		lseek(fdset[NEXT].fd, 0, SEEK_SET);
    		len = read(fdset[NEXT].fd, buf, MAX_BUF);
    		button = NEXT;
    	}     

    	fflush(stdout);
    }

    for(int i=0;i<4;i++){
    	gpio_unexport(gpio[i]);
    	gpio_fd_close(gpio_fd[i]);
    }

    return NULL;
}

/* BATTERY READ THREAD */
void* batt_read(void *ptr){
	//set ADC scales and offsets
	if(ti_adc_set_scale(BATTERY, config.adc_scale[BATTERY])<0){
		perror("\nERROR: ti_adc_set_scale failed\n");
		fprintf(stderr, "channel %d failed", BATTERY);	
		return NULL;
	}
	if(ti_adc_set_offset(BATTERY, config.adc_offset[BATTERY])<0){
		perror("\nERROR: ti_adc_set_offset failed\n");
		fprintf(stderr, "channel %d failed", BATTERY);
		return NULL;
	}

	printf("\n ADC scales and offsets configured...");
	fflush(stdout);
	int batt;
	while(state.system != EXITING){
		batt = ti_adc_read_raw(BATTERY);
		switch(batt) {
			case 54001 ... 55050:
			state.batt = 100.0;
			break;

			case 51773 ... 54000:
			state.batt = 80.0;
			break;

			case 49807 ... 51772:
			state.batt = 60.0;
			break;

			case 49152 ... 49806:
			state.batt = 40.0;
			break;

			case 48496 ... 49151:
			state.batt = 20.0;
			break;

			case 47185 ... 48495:
			state.batt = 10.0;
			break;

			case 45219 ... 47184:
			state.batt = 5.0;
			break;

			case 0 ... 45218:
			state.batt = 0.0;
			break;
		}
		usleep(.25*1e6);
	}
	return NULL;
}

/************************************************************************************
* SIGINT
*************************************************************************************/
void sigint(int s __attribute__((unused))) {
	printf("\n Received SIGINT: \n");
	fflush(stdout);

	printf("\n Exiting cleanly...");
	fflush(stdout);

	state.system = EXITING;

    //Display ARMOR logo
	clearDisplay();
	display_bitmap();
	Display();
	usleep(2*1e6);

    //join pthreads
	pthread_join(button_poll_thread, NULL);
	pthread_join(batt_read_thread, NULL);
	printf("\n pthreads joined... \n");

    //detach gpio pins
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
	gpio_detach(oled_reset_gpio_info);
	gpio_detach(oled_power_gpio_info);

	printf("\n Detached all gpio pins");
	fflush(stdout);

    //clean up gpio library
	gpio_finish();
	printf("\n closed gpiolib cleanly...");
	fflush(stdout);

    //clean up adc library
	ti_adc_cleanup();
	printf("\n cleaned up ADC interface...\n\n");
	fflush(stdout);

    // fclose(fp);
    // printf("\n file has closed\n\n");
	exit(0);
}
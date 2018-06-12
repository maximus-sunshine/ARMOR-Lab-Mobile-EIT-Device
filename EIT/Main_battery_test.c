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
 * Main.c (compile with 'gcc -pthread Main.c src/gpiolib.c src/ti-ads8684.c src/UI.c src/I2C.c src/SSD1306_OLED.c -o Main')
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
	.adc_scale		= {0.078127104,0.078127104,0.078127104,0.078127104},	//see ti-ads8684.h for available scales
	.adc_offset		= {0,0,0,0},											//see ti-ads8684.h for available offsets
	.channels		= {1,0,0,0},
	.sample_mode	= CYCLES,
	.time			= 5,
	.cycles			= 10,
	.sample_geom	= ACROSS,
	.i_setpoint		= 100,
};

state_t state = {
	.menu = HOME,
	.back = HOME,
	.index = 0,
	.prev_index = 0,
	.len = HOME_OPTS_LEN,
	.batt = 0.0,
	.system = UI,
};

/************************************************************************************
* SETUP
*************************************************************************************/
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
int manage_menu();

/************************************************************************************
* MAIN
*************************************************************************************/
int main(){

	printf("\n entered MAIN...\n");

	signal(SIGINT, sigint);
	printf(" setup SIGINT...\n");

	/* INITIALIZE LIBRARIES */
	//initialize gpio_lib
	if(gpio_init()){
		fprintf(stderr, " gpio_init failed with %i\n", gpio_errno);
		exit(1);
	}
	printf(" gpiolib intialized...\n");

	//initialize ADC library
	if(ti_adc_init()<0){
		fprintf(stderr, " ti_adc_init failed\n");
		exit(1);
	};
	printf(" ADC interface initialized...\n");

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

	printf(" gpio pins attached...\n");
	

	//power on OLED
	gpio_set(oled_power_gpio_info);
	gpio_set(oled_reset_gpio_info);
	printf(" OLED power and reset pins set high...\n");
	

    //reset OLED
	gpio_clear(oled_reset_gpio_info);
	usleep(0.5*1e6);
	gpio_set(oled_reset_gpio_info);
	printf(" OLED display reset...\n");
	

    //Initialize I2C bus and connect to the I2C Device
	if(init_i2c_dev2(SSD1306_OLED_ADDR) == 0)
	{
		printf(" (Main)i2c-2: Bus Connected to SSD1306\n");
		
	}
	else
	{
		printf(" (Main)i2c-2: OOPS! Something Went Wrong\n");
		
		exit(1);
	}

	//enable ADC
	printf(" enabling ADC...\n");
	gpio_set(adc_reset_gpio_info);
	printf(" ADC enabled...\n");
	

	/* START PTHREAD TO READ BATTERY */	
	pthread_create(&batt_read_thread, NULL, batt_read, (void*) NULL);
	printf(" battery reading pthread created...\n");
	

	/* Run SDD1306 Initialization Sequence */
	display_Init_seq();
	printf(" init sequence displayed...\n");
	
	printf(" OLED initialized...\n");
	

    /* Display ARMOR logo */
	printf(" Displaying ARMOR logo...\n");
	
	display_bitmap();
	Display();
	usleep(2*1e6);

    /* Start button poll pthread */
	pthread_create(&button_poll_thread, NULL, button_poll, (void*) NULL);
	printf(" polling buttons...\n");
	

	/* ENTER UI */
	printf(" entering menu interface...\n");
	

	state.system = UI;
	
	usleep(0.5*1e6);
	button = -1;
	while(state.system != EXITING){
		if(state.system == UI){
			manage_menu();
		} 
		else if (state.system == RUNNING){
			sample();
			state.system = UI;
		} 
	}

	/* CLEANUP */
    //display ARMOR logo
	printf(" displaying ARMOR logo\n");
	
	display_bitmap();
	Display();
	usleep(2*1e6);

    //join pthreads
	pthread_join(button_poll_thread, NULL);
	pthread_join(batt_read_thread, NULL);
	printf(" pthreads joined... \n");

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

	printf(" detached all gpio pins\n");
	

	//clean up gpio library
	gpio_finish();
	printf(" closed gpiolib cleanly...\n");
	

	//clean up adc library
	ti_adc_cleanup();
	printf(" cleaned up ADC interface...\n");
	

	// fclose(fp);

	// printf("\n file has closed");
	printf("\n FINISHED!\n\n");
	
}

/************************************************************************************
* FUNCTIONS
*************************************************************************************/
int sample()
{
	printf("\n\n CONFIGURING SAMPLE PROCESS...\n");
	

	long elapsed_time = 0;
	int count = 0;
	int data, n, k;
	struct timeval t1, t2;

	print_sample_screen();

	//set ADC scales and offsets
	int i;
	for(i=0;i<CHANNELS;i++){
		if(ti_adc_set_scale(i, config.adc_scale[i])<0){
			perror("\n ERROR: ti_adc_set_scale failed\n");
			fprintf(stderr, "channel %d failed\n", i);	
			return -1;
		}
		if(ti_adc_set_offset(i, config.adc_offset[i])<0){
			perror("\n ERROR: ti_adc_set_offset failed\n");
			fprintf(stderr, "channel %d failed\n", i);
			return -1;
		}
	}
	printf(" ADC scales and offsets configured...\n");
	

	//configure mux switching patterns
	int current_mux[config.nodal_num];	// current                                           
	int ground_mux[config.nodal_num];	// ground
	int voltage_mux[config.nodal_num];	// voltage

	if(config.sample_geom == ACROSS) mux_config_across(config.nodal_num,current_mux,ground_mux,voltage_mux);
	else if(config.sample_geom == ADJACENT) mux_config_adjacent(config.nodal_num,current_mux,ground_mux,voltage_mux);
	printf(" mux switching patterns configured...\n");
	

	//disable muxes for safety
	for(i = 0;i < 3; i++){
		gpio_set(mux_disable_gpio_info[i]);
	}
	printf(" muxes disabled...\n");
	

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
	printf(" current set to %d\n",config.i_setpoint);
	

	// set up data writing
	//fp = fopen(VOLT_DATA_TEXT,"w");
	//printf(" Data file opened...\n");
 	//char raw_buf[8];

 	//execute sampling
	printf("\n BEGINNING sampling!\n");
	

	gettimeofday(&t1, NULL);
	while(1){
	
		// //Uncomment for DEBUG
		// printf("\n\n\n******************** Cycle %d *************************\n\n",count);
		// 

		//outer loop, move current and ground
		for(i = 0; i < config.nodal_num; i++){
			// //Uncomment for DEBUG
			// printf("--------------Current Configuration: Current at node %d, GND at node %d ------------\n", current_mux[i]+1, ground_mux[i]+1);
			// 

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
					//fill current and ground nodes with placeholder data
          			//strcpy(raw_buf, "0\n");
		      		//fputs(raw_buf,fp);
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
					ti_adc_read_str(NODE);
					//strcpy(raw_buf, ti_adc_read_str(NODE));
					//fputs(raw_buf,fp);
					// // Uncomment for DEBUG
					// printf("Voltage at node %d:  %.4f V\n", voltage_mux[j]+1,atoi(raw_buf)*config.adc_scale[NODE]/1000);
					// 
				}

				//disable muxs
				for(n = 0;n < 3; n++){
					gpio_set(mux_disable_gpio_info[n]);
				}
			}
		}
	//	gettimeofday(&t2, NULL);
	//	elapsed_time = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)/1e6;
	//	count++;
	}
 	//Print timing data to screen
	//gettimeofday(&t2, NULL);
	//long usec = 1e6 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec;
	///float freq = count/(usec/1e6);
	///float time = usec/1e6;
	//printf("\n DONE SAMPLING %d nodes, %d cycles in %0.5f seconds: Avg. cyclic frequency: %0.5f\n",config.nodal_num, count, time, freq);
	

	//finish writing data
    //	fclose(fp);
  	//printf(" Converting and formatting data\n");

 	//data_conversion(config.i_setpoint,config.nodal_num, count, time, freq);
    //	printf(" Conversion/Formatting complete\n\n\n");
	
	return 0;
}

int process_button(const char opt_list[][OPT_STR_LEN])
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
		state.menu = state.back;
		state.index = state.prev_index;
		button = -1;
		return 0;
		break;

		case -1:
		return 0;
		break;
	}
}

int manage_menu(){
	switch (state.menu) {
		case HOME:
		state.prev_index = 0;
		state.back = HOME;
		state.len = HOME_OPTS_LEN;
		printUI(state, HOME_OPTS);
		if(process_button(HOME_OPTS)){
			if(mod(state.index,state.len) == 0) {
				state.index = 0;
				state.system = RUNNING;
			}
			else{
				state.menu = SETTINGS;
				state.index = 0;
			}
		}
		break;

		case SETTINGS:
		state.back = HOME;
		state.prev_index = 1;
		state.len = SETTINGS_OPTS_LEN;
		printUI(state, SETTINGS_OPTS);
		if(process_button(SETTINGS_OPTS)){
			state.menu = mod(state.index,state.len) + 2;
			state.prev_index = state.index;
			state.index = 0; 
		};
		break;

		case NODES:
		state.back = SETTINGS;
		state.prev_index = 0;
		state.len = NODES_OPTS_LEN;
		printUI(state, NODES_OPTS);
		if(process_button(NODES_OPTS)){
			config.nodal_num = atoi(NODES_OPTS[mod(state.index,state.len)]);
			printf(" nodal num set to %d\n",config.nodal_num);
			state.menu = state.back;
			state.index = state.prev_index;
		};
		break;

		case CURRENT:
		state.back = SETTINGS;
		state.prev_index = 1;
		state.len = CURRENT_OPTS_LEN;
		printUI(state, CURRENT_OPTS);
		if(process_button(CURRENT_OPTS)){
			config.i_setpoint = atoi(CURRENT_OPTS[mod(state.index,state.len)]);
			printf(" current set to %d\n",config.i_setpoint);
			state.menu = state.back;
			state.index = state.prev_index;
		};
		break;

		case CONFIG:
		state.back = SETTINGS;
		state.prev_index = 2;
		state.len = CONFIG_OPTS_LEN;
		printUI(state, CONFIG_OPTS);
		if(process_button(CONFIG_OPTS)){
			config.sample_geom = mod(state.index,state.len);
			printf(" geometry set\n");
			state.menu = state.back;
			state.index = state.prev_index;
		};
		break;

		case SAMPLING:
		state.back = SETTINGS;
		state.prev_index = 3;
		state.len = SAMPLING_OPTS_LEN;
		printUI(state, SAMPLING_OPTS);
		if(process_button(SAMPLING_OPTS)){
			config.sample_mode = mod(state.index,state.len);
			printf(" sampling mode set to %s\n",SAMPLING_OPTS[config.sample_mode]);
			if (mod(state.index,state.len) == CONTINUOUS){
				state.menu = state.back;
				state.index = state.prev_index;
			}
			else {
				state.menu = mod(state.index,state.len) + 6;
				state.index = 0;
			}
		};
		break;

		case TIME:
		state.back = SAMPLING;
		state.prev_index = 0;
		state.len = TIME_OPTS_LEN;
		printUI(state, TIME_OPTS);
		if(process_button(TIME_OPTS)){
			config.time = time_opts[mod(state.index,state.len)];
			printf(" time set to %d seconds\n",config.time);
			state.menu = state.back;
			state.index = state.prev_index;
		};
		break;

		case CYCLE:
		state.back = SAMPLING;
		state.prev_index = 1;
		state.len = CYCLE_OPTS_LEN;
		printUI(state, CYCLE_OPTS);
		if(process_button(CYCLE_OPTS)){
			config.cycles = atoi(CYCLE_OPTS[mod(state.index,state.len)]);
			printf(" cycles set to %d\n",config.cycles);
			state.menu = state.back;
			state.index = state.prev_index;
		};
		break;

	}
	return 0;
	usleep(0.1*1e6);
}

/************************************************************************************
* PTHREADS
*************************************************************************************/

/* BUTTON POLL THREAD */
void* button_poll(void* ptr){
	int nfds = 4;
	struct pollfd fdset[nfds];
	int gpio_fd[nfds], timeout, rc;
	char *buf[MAX_BUF];
	unsigned int gpio[nfds];
	int len;

    gpio[0] = 61; // button select
    gpio[1] = 88; // button previous
    gpio[2] = 89; // button next
    gpio[3] = 11; // button back

    for(int i=0;i<nfds;i++){
    	gpio_export(gpio[i]);
    	gpio_set_dir(gpio[i], 0);
    	gpio_set_edge(gpio[i], "rising");
    	gpio_fd[i] = gpio_fd_open(gpio[i]);
    }

    timeout = POLL_TIMEOUT;

    while(state.system != EXITING){
    	memset((void*)fdset, 0, sizeof(fdset));

    	for(int i=0;i<nfds;i++){
    		fdset[i].fd = gpio_fd[i];
    		fdset[i].events = POLLPRI;
    	}

    	rc = poll(fdset, nfds, timeout);

    	if (rc < 0) {
    		if (errno == EINTR) {
    			printf("\n Interrupted system call... continuing\n");
    			continue;
    		}
    		perror("\n poll() failed!\n");
    		return NULL;
    	}

    	switch(state.system){
    		
    		case UI:
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
    		if (fdset[BACK].revents & POLLPRI) {
    			lseek(fdset[BACK].fd, 0, SEEK_SET);
    			len = read(fdset[BACK].fd, buf, MAX_BUF);
    			button = BACK;
    		}
    		break;

    		case RUNNING:
    		if (fdset[BACK].revents & POLLPRI) {
    			state.system = UI;
    			for(int i=0;i<nfds;i++){
    				lseek(fdset[i].fd, 0, SEEK_SET);
    				len = read(fdset[i].fd, buf, MAX_BUF);
    			}
    		}
    		break;
    		
    	}
    }

    for(int i=0;i<nfds;i++){
    	gpio_unexport(gpio[i]);
    	gpio_fd_close(gpio_fd[i]);
    }

    return NULL;
}

/* BATTERY READ THREAD */
void* batt_read(void *ptr){
	long elapsed_time = 0;
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	//set ADC scales and offsets
	float battery_volt;
	double volt_scale = 0.078127104;


	// set up data writing
	fp = fopen("/home/debian/MAE156B_Team6/data/battery_data.txt","w");
	printf(" Data file opened...\n");
 	//char raw_buf[8];


	if(ti_adc_set_scale(BATTERY, config.adc_scale[BATTERY])<0){
		perror(" ERROR: ti_adc_set_scale failed\n");
		fprintf(stderr, " channel %d failed\n", BATTERY);	
		return NULL;
	}
	if(ti_adc_set_offset(BATTERY, config.adc_offset[BATTERY])<0){
		perror(" ERROR: ti_adc_set_offset failed\n");
		fprintf(stderr, " channel %d failed\n", BATTERY);
		return NULL;
	}

	printf(" ADC scales and offsets configured...\n");
	
	int batt;
	while(state.system != EXITING){
		battery_volt = ti_adc_read_raw(BATTERY)*(volt_scale/1000);
		fprintf(fp,"%0.5f\n", battery_volt);
		if(battery_volt <= 3.0){
				gettimeofday(&t2, NULL);
				elapsed_time = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)/1e6;
				long usec = 1e6 * (t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec;
				float time = usec/1e6;
				fprintf(fp, "%f",time);
				fclose(fp);
				printf("\n Received SIGINT: \n");
	

				printf(" Exiting cleanly...\n");
				

				state.system = EXITING;

    //Display ARMOR logo
				clearDisplay();
				display_bitmap();
				Display();
				usleep(2*1e6);

    //join pthreads
				pthread_join(button_poll_thread, NULL);
				pthread_join(batt_read_thread, NULL);
				printf(" pthreads joined...\n");

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

				printf(" Detached all gpio pins\n");
				

    //clean up gpio library
				gpio_finish();
				printf(" closed gpiolib cleanly...\n");
				

    //clean up adc library
				ti_adc_cleanup();
				printf(" cleaned up ADC interface...\n\n");
				system("shutdown now");
		}
		usleep(1*1e6);
	}
	return NULL;
}

/************************************************************************************
* SIGINT
*************************************************************************************/
void sigint(int s __attribute__((unused))) {
	printf("\n Received SIGINT: \n");
	

	printf(" Exiting cleanly...\n");
	

	state.system = EXITING;

    //Display ARMOR logo
	clearDisplay();
	display_bitmap();
	Display();
	usleep(2*1e6);

    //join pthreads
	pthread_join(button_poll_thread, NULL);
	pthread_join(batt_read_thread, NULL);
	printf(" pthreads joined...\n");

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

	printf(" Detached all gpio pins\n");
	

    //clean up gpio library
	gpio_finish();
	printf(" closed gpiolib cleanly...\n");
	

    //clean up adc library
	ti_adc_cleanup();
	printf(" cleaned up ADC interface...\n\n");
	

    // fclose(fp);
    // printf("\n file has closed\n\n");
	exit(0);
}
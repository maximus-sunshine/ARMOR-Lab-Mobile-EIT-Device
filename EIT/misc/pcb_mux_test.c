/* MAE 156B Team 6
* 
* Preliminary Mux Switching Test
* 
* User selects Current, GND, and Voltage nodes from command line
* 
* 4/30/18
*/

/************************************************************************************
* INCLUDES/DEFINES
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
#include "eit_config.h" //project specific setup stuff

#define MUX_PINS 5

/************************************************************************************
* SIGINT HANDLER, TODO: improve
*************************************************************************************/
void sigint(int s __attribute__((unused))) {
	printf("\nReceived SIGINT:\n");
	fflush(stdout);

	gpio_info *current_mux_gpio_info[MUX_PINS];
	gpio_info *ground_mux_gpio_info[MUX_PINS];
	gpio_info *voltage_mux_gpio_info[MUX_PINS];

	int i;
	for(i=0;i<MUX_PINS;i++){
		gpio_detach(current_mux_gpio_info[i]);
		gpio_detach(ground_mux_gpio_info[i]);
		gpio_detach(voltage_mux_gpio_info[i]);
	}
	gpio_finish();
	exit(0);
}

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

/************************************************************************************
* MAIN
*************************************************************************************/
int main(int argc, char **argv) {
	printf("\nentered MAIN...\n");
	fflush(stdout);

	//Sigint setup, needs work
	signal(SIGINT, sigint);
	printf("setup SIGINT...\n");
	fflush(stdout);

	//initialize gpiolib
	if (gpio_init()) {
		fprintf(stderr, "gpio_init failed with %i\n", gpio_errno);
		exit(1);
	}
	printf("intialized gpiolib...\n");
	fflush(stdout);

	//TODO: Figure out arrays/pointers/structs
	// gpio_info *current_mux_gpio_info[MUX_PINS];
	// gpio_info *ground_mux_gpio_info[MUX_PINS];
	// gpio_info *voltage_mux_gpio_info[MUX_PINS];
	// printf("\n setup structs");
	
	//Setup gpiolib structures for mux pins
	gpio_info *current_A4;
	gpio_info *current_A3;
	gpio_info *current_A2;
	gpio_info *current_A1;
	gpio_info *current_A0;
	gpio_info *ground_A4;
	gpio_info *ground_A3;
	gpio_info *ground_A2;
	gpio_info *ground_A1;
	gpio_info *ground_A0;
	gpio_info *voltage_A4;
	gpio_info *voltage_A3;
	gpio_info *voltage_A2;
	gpio_info *voltage_A1;
	gpio_info *voltage_A0;
	printf("defined mux GPIO pins...\n");
	fflush(stdout);

	//Setup gpiolib structures for current source pins 
	gpio_info *i_source_A04;
	gpio_info *i_source_A05;
	gpio_info *i_source_A06;
	gpio_info *i_source_A07;
	gpio_info *i_source_A08;
	gpio_info *i_source_A09;
	gpio_info *i_source_A10;
	gpio_info *i_source_A11;
	gpio_info *i_source_A12;
	gpio_info *i_source_A13;
	printf("defined current source GPIO pins...\n");
	fflush(stdout);

	//Attach all GPIO pins 
	//Current
	int i, bank, mask;
	bank = current_mux_gpio[0]/32;
	mask = bit(current_mux_gpio[0]%32);
	current_A4 = gpio_attach(bank, mask, GPIO_OUT);

	bank = current_mux_gpio[1]/32;
	mask = bit(current_mux_gpio[1]%32);
	current_A3 = gpio_attach(bank, mask, GPIO_OUT);

	bank = current_mux_gpio[2]/32;
	mask = bit(current_mux_gpio[2]%32);
	current_A2 = gpio_attach(bank, mask, GPIO_OUT);

	bank = current_mux_gpio[3]/32;
	mask = bit(current_mux_gpio[3]%32);
	current_A1 = gpio_attach(bank, mask, GPIO_OUT);

	bank = current_mux_gpio[4]/32;
	mask = bit(current_mux_gpio[4]%32);
	current_A0 = gpio_attach(bank, mask, GPIO_OUT);

	//Ground
	bank = ground_mux_gpio[0]/32;
	mask = bit(ground_mux_gpio[0]%32);
	ground_A4 = gpio_attach(bank, mask, GPIO_OUT);

	bank = ground_mux_gpio[1]/32;
	mask = bit(ground_mux_gpio[1]%32);
	ground_A3 = gpio_attach(bank, mask, GPIO_OUT);

	bank = ground_mux_gpio[2]/32;
	mask = bit(ground_mux_gpio[2]%32);
	ground_A2 = gpio_attach(bank, mask, GPIO_OUT);

	bank = ground_mux_gpio[3]/32;
	mask = bit(ground_mux_gpio[3]%32);
	ground_A1 = gpio_attach(bank, mask, GPIO_OUT);

	bank = ground_mux_gpio[4]/32;
	mask = bit(ground_mux_gpio[4]%32);
	ground_A0 = gpio_attach(bank, mask, GPIO_OUT);

	//Voltage
	bank = voltage_mux_gpio[0]/32;
	mask = bit(voltage_mux_gpio[0]%32);
	voltage_A4 = gpio_attach(bank, mask, GPIO_OUT);

	bank = voltage_mux_gpio[1]/32;
	mask = bit(voltage_mux_gpio[1]%32);
	voltage_A3 = gpio_attach(bank, mask, GPIO_OUT);

	bank = voltage_mux_gpio[2]/32;
	mask = bit(voltage_mux_gpio[2]%32);
	voltage_A2 = gpio_attach(bank, mask, GPIO_OUT);

	bank = voltage_mux_gpio[3]/32;
	mask = bit(voltage_mux_gpio[3]%32);
	voltage_A1 = gpio_attach(bank, mask, GPIO_OUT);

	bank = voltage_mux_gpio[4]/32;
	mask = bit(voltage_mux_gpio[4]%32);
	voltage_A0 = gpio_attach(bank, mask, GPIO_OUT);
	
	//Current Source
	i_source_A04 = gpio_attach(2,bit(2),GPIO_OUT);
	i_source_A05 = gpio_attach(1,bit(13),GPIO_OUT);
	i_source_A06 = gpio_attach(2,bit(3),GPIO_OUT);
	i_source_A07 = gpio_attach(1,bit(12),GPIO_OUT);
	i_source_A08 = gpio_attach(0,bit(23),GPIO_OUT);
	i_source_A09 = gpio_attach(2,bit(4),GPIO_OUT);
	i_source_A10 = gpio_attach(1,bit(15),GPIO_OUT);
	i_source_A11 = gpio_attach(2,bit(5),GPIO_OUT);
	i_source_A12 = gpio_attach(0,bit(26),GPIO_OUT);
	i_source_A13 = gpio_attach(1,bit(14),GPIO_OUT);
	
	printf("attached all GPIO pins...\n");
	fflush(stdout);

	// TODO: structs/arrays/pointers
	// //attach ground gpio pins
	// for(i=0;i<sizeof(ground_mux_gpio);i++){                            
	// 	bank = ground_mux_gpio[i]/32;
	// 	mask = bit(ground_mux_gpio[i]%32);
	// 	ground_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);
	// }

	// //attach voltage gpio pins
	// for(i=0;i<sizeof(voltage_mux_gpio);i++){                            
	// 	bank = voltage_mux_gpio[i]/32;
	// 	mask = bit(voltage_mux_gpio[i]%32);
	// 	voltage_mux_gpio_info[i] = gpio_attach(bank, mask, GPIO_OUT);	
	// }
	// printf("\n attached mux pins");

	//Parse input arguments
	int current_node = atoi(argv[1])-1;
	int ground_node = atoi(argv[2])-1;
	int voltage_node = atoi(argv[3])-1;
	printf("parsed args. current node: %d, gnd node: %d, V node: %d...\n",current_node+1, ground_node+1, voltage_node+1);
	fflush(stdout);

	//Set and clear logic pins
	//Current mux
	if(CHAN[current_node][0]==0){
		gpio_clear(current_A4);
	}
	else{
		gpio_set(current_A4);
	}
	
	if(CHAN[current_node][1]==0){
		gpio_clear(current_A3);
	}
	else{
		gpio_set(current_A3);
	}

	if(CHAN[current_node][2]==0){
		gpio_clear(current_A2);
	}
	else{
		gpio_set(current_A2);
	}

	if(CHAN[current_node][3]==0){
		gpio_clear(current_A1);
	}
	else{
		gpio_set(current_A1);
	}

	if(CHAN[current_node][4]==0){
		gpio_clear(current_A0);
	}
	else{
		gpio_set(current_A0);
	}

	//Ground mux
	if(CHAN[ground_node][0]==0){
		gpio_clear(ground_A4);
	}
	else{
		gpio_set(ground_A4);
	}
	
	if(CHAN[ground_node][1]==0){
		gpio_clear(ground_A3);
	}
	else{
		gpio_set(ground_A3);
	}

	if(CHAN[ground_node][2]==0){
		gpio_clear(ground_A2);
	}
	else{
		gpio_set(ground_A2);
	}

	if(CHAN[ground_node][3]==0){
		gpio_clear(ground_A1);
	}
	else{
		gpio_set(ground_A1);
	}

	if(CHAN[ground_node][4]==0){
		gpio_clear(ground_A0);
	}
	else{
		gpio_set(ground_A0);
	}

	//Voltage mux
	if(CHAN[voltage_node][0]==0){
		gpio_clear(voltage_A4);
	}
	else{
		gpio_set(voltage_A4);
	}
	
	if(CHAN[voltage_node][1]==0){
		gpio_clear(voltage_A3);
	}
	else{
		gpio_set(voltage_A3);
	}

	if(CHAN[voltage_node][2]==0){
		gpio_clear(voltage_A2);
	}
	else{
		gpio_set(voltage_A2);
	}

	if(CHAN[voltage_node][3]==0){
		gpio_clear(voltage_A1);
	}
	else{
		gpio_set(voltage_A1);
	}

	if(CHAN[voltage_node][4]==0){
		gpio_clear(voltage_A0);
	}
	else{
		gpio_set(voltage_A0);
	}
	printf("Set mux logic pins... \n");
	fflush(stdout);

	//Set current source logic pins
	gpio_clear(i_source_A04);
	gpio_clear(i_source_A05);
	gpio_clear(i_source_A06);
	gpio_clear(i_source_A07);
	gpio_clear(i_source_A08);
	gpio_clear(i_source_A09);
	gpio_clear(i_source_A10);
	gpio_clear(i_source_A11);
	gpio_clear(i_source_A12);
	gpio_clear(i_source_A13);

	printf("set current source logic pins...\n");
	fflush(stdout);

	//TODO: structs/arrays/pointers
	// //Current
	// for(i=0;i<MUX_PINS;i++){
	// 	if(CHAN[current_node][i]==0){
	// 		gpio_clear(current_mux_gpio_info[i]);
	// 	}
	// 	else{
	// 		gpio_set(current_mux_gpio_info[i]);
	// 	}
	// }

	// //Ground
	// for(i=0;i<MUX_PINS;i++){
	// 	if(CHAN[ground_node][i]==0){
	// 		gpio_clear(ground_mux_gpio_info[i]);
	// 	}
	// 	else{
	// 		gpio_set(ground_mux_gpio_info[i]);
	// 	}
	// }

	// //Voltage
	// for(i=0;i<MUX_PINS;i++){
	// 	if(CHAN[voltage_node][i]==0){
	// 		gpio_clear(voltage_mux_gpio_info[i]);
	// 	}
	// 	else{
	// 		gpio_set(voltage_mux_gpio_info[i]);
	// 	}
	//}

	//Sleep
	int sleep_time = 5;

	printf("Sleeping\n");
	fflush(stdout);

	usleep(sleep_time * 1000000);
	
	//Detach all GPIO pins
	gpio_detach(current_A4);
	gpio_detach(current_A3);
	gpio_detach(current_A2);
	gpio_detach(current_A1);
	gpio_detach(current_A0);
	gpio_detach(ground_A4);
	gpio_detach(ground_A3);
	gpio_detach(ground_A2);
	gpio_detach(ground_A1);
	gpio_detach(ground_A0);
	gpio_detach(voltage_A4);
	gpio_detach(voltage_A3);
	gpio_detach(voltage_A2);
	gpio_detach(voltage_A1);
	gpio_detach(voltage_A0);


	gpio_clear(i_source_A04);
	gpio_clear(i_source_A05);
	gpio_clear(i_source_A06);
	gpio_clear(i_source_A07);
	gpio_clear(i_source_A08);
	gpio_clear(i_source_A09);
	gpio_clear(i_source_A10);
	gpio_clear(i_source_A11);
	gpio_clear(i_source_A12);
	gpio_clear(i_source_A13);
	gpio_detach(i_source_A04);
	gpio_detach(i_source_A05);
	gpio_detach(i_source_A06);
	gpio_detach(i_source_A07);
	gpio_detach(i_source_A08);
	gpio_detach(i_source_A09);
	gpio_detach(i_source_A10);
	gpio_detach(i_source_A11);
	gpio_detach(i_source_A12);
	gpio_detach(i_source_A13);
	printf("cleared and detached all pins... \n");
	fflush(stdout);

	//TODO: Structs/arrays/pointers
	// for(i=0;i<MUX_PINS;i++){
	// 	gpio_detach(current_mux_gpio_info[i]);
	// 	gpio_detach(ground_mux_gpio_info[i]);
	// 	gpio_detach(voltage_mux_gpio_info[i]);
	// }

	//Finish up
	gpio_finish();
	printf("FINISHED\n");
	fflush(stdout);
}
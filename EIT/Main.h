/****************************************************************************
 * ------------------------------------------------------------------------
 * ARMOR Lab @UC San Diego, Kenneth Loh Ph.D
 * 
 * MAE 156B Spring 2018 Team 6: Warfighter Protection
 *  - Maxwell Sun       (maxsun96@gmail.com)
 *  - Jacob Rutheiser   (jrutheiser@gmail.com)
 *  - Matthew Williams  (mwilliams31243@gmail.com)
 *  - Aaron Gunn        (gunnahg@gmail.com)
 * ------------------------------------------------------------------------
 * 
 * Main.h
 * 
 * The Big Kahuna. This is the script that runs on boot. It does everything.
 ***************************************************************************/

/************************************************************************************
* INCLUDES
*************************************************************************************/
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>     // for atoi
#include <fcntl.h>      // for open
#include <unistd.h>     // for close
#include <stdio.h>      
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <poll.h>
#include <dirent.h>

#include "includes/ti-ads8684.h"
#include "includes/gpiolib.h"
#include "includes/UI.h"

/************************************************************** 
*DATA FILE EXPORT PATH
***************************************************************/
#define VOLT_DATA_TEXT "/home/debian/MAE156B_Team6/data/data.txt"
#define TEMP_VOLT_DATA_TEXT "/media/card/data/temp_data.txt" 
#define RAW_PATH "/media/card/data/data"

/************************************************************** 
*FILE POINTERS
***************************************************************/
FILE* fp;
FILE* fp_temp;
int fd;

/************************************************************************************
* ENUMS/DEFINES/VARIABLES
*************************************************************************************/
/* GPIO PINS */
#define MUX_PINS 5      //number of mux logic pins
#define I_SWTCH_PINS 10 //number of current source switch logic pins

//adc reset
#define ADC_RESET_GPIO 15 //p9_24

//current sense reset
#define I_SENSE_RESET_GPIO 27 //p8_17

//oled power/reset
#define OLED_POWER_GPIO 81 //p8_34
#define OLED_RESET_GPIO 80 //p8_36

//current source switch logic
#define CUR_SOURCE_A4  26   //p8_14
#define CUR_SOURCE_A5  66   //p8_7
#define CUR_SOURCE_A6  68   //p8_10
#define CUR_SOURCE_A7  23   //p8_13
#define CUR_SOURCE_A8  47   //p8_15
#define CUR_SOURCE_A9  69   //p8_9
#define CUR_SOURCE_A10 45   //p8_11
#define CUR_SOURCE_A11 46   //p8_16
#define CUR_SOURCE_A12 67   //p8_8
#define CUR_SOURCE_A13 44   //p8_12

//mux disable
#define CUR_MUX_DISABLE  79 //p8_38
#define GND_MUX_DISABLE  71 //p8_46
#define VOLT_MUX_DISABLE 22 //p8_19

//mux logic
#define CUR_MUX_A0  8    //p8_35    //current
#define CUR_MUX_A1  9    //p8_33
#define CUR_MUX_A2  10   //p8_31
#define CUR_MUX_A3  87   //p8_29
#define CUR_MUX_A4  86   //p8_27
#define GND_MUX_A0  70   //p8_45    //ground
#define GND_MUX_A1  72   //p8_43
#define GND_MUX_A2  74   //p8_41
#define GND_MUX_A3  76   //p8_39
#define GND_MUX_A4  78   //p8_37
#define VOLT_MUX_A0  7   //p9_24    //voltage
#define VOLT_MUX_A1  20  //p9_41
#define VOLT_MUX_A2  14  //p9_26
#define VOLT_MUX_A3  117 //p9_25
#define VOLT_MUX_A4  115 //p9_27

//arrays
#define CURRENT_MUX_GPIO    {CUR_MUX_A4, CUR_MUX_A3, CUR_MUX_A2, CUR_MUX_A1, CUR_MUX_A0}
#define GROUND_MUX_GPIO     {GND_MUX_A4, GND_MUX_A3, GND_MUX_A2, GND_MUX_A1, GND_MUX_A0}
#define VOLTAGE_MUX_GPIO    {VOLT_MUX_A4,VOLT_MUX_A3,VOLT_MUX_A2,VOLT_MUX_A1,VOLT_MUX_A0}
#define CURRENT_SWITCH_GPIO {CUR_SOURCE_A4,CUR_SOURCE_A5,CUR_SOURCE_A6,CUR_SOURCE_A7,CUR_SOURCE_A8,CUR_SOURCE_A9,CUR_SOURCE_A10,CUR_SOURCE_A11,CUR_SOURCE_A12,CUR_SOURCE_A13}
#define MUX_DISABLE_GPIO    {CUR_MUX_DISABLE, GND_MUX_DISABLE, VOLT_MUX_DISABLE}         

/* TRUTH TABLES (MUX & CURRENT SOURCE SWITCH) */
//mux truth table
//[*][ ]:   nodes 1 to 32
//[ ][*]:   mux logic pins {A4,A3,A2,A1,A0}
#define LOGIC_ROW 32
#define LOGIC_COL 5
int CHAN[LOGIC_ROW][LOGIC_COL] = {{0,0,0,0,0},{0,0,0,0,1},{0,0,0,1,0},{0,0,0,1,1},
                                 {0,0,1,0,0},{0,0,1,0,1},{0,0,1,1,0},{0,0,1,1,1},
                                 {0,1,0,0,0},{0,1,0,0,1},{0,1,0,1,0},{0,1,0,1,1},
                                 {0,1,1,0,0},{0,1,1,0,1},{0,1,1,1,0},{0,1,1,1,1},
                                 {1,0,0,0,0},{1,0,0,0,1},{1,0,0,1,0},{1,0,0,1,1},
                                 {1,0,1,0,0},{1,0,1,0,1},{1,0,1,1,0},{1,0,1,1,1},
                                 {1,1,0,0,0},{1,1,0,0,1},{1,1,0,1,0},{1,1,0,1,1},
                                 {1,1,1,0,0},{1,1,1,0,1},{1,1,1,1,0},{1,1,1,1,1}};

//current source switch truth table
//[*][ ]:   100ua to 20000ua in 100ua steps
//[ ][*]:   logic pins {A4,A5,A6,A7,A8,A9,A10,A11,A12,A13} 
#define CUR_ROW 20
#define CUR_COL 10
int I_SWITCH[CUR_ROW][CUR_COL] = {{1,0,0,0,0,0,0,0,0,0},{1,1,0,0,0,0,0,0,0,0},
                                 {1,1,1,0,0,0,0,0,0,0},{1,1,1,1,0,0,0,0,0,0},
                                 {1,1,1,1,1,0,0,0,0,0},{1,1,1,1,1,1,0,0,0,0},
                                 {1,1,1,1,1,1,1,0,0,0},{1,1,1,1,1,1,1,1,0,0},
                                 {1,1,1,1,1,0,0,0,1,0},{1,1,1,1,1,1,0,0,1,0},
                                 {1,1,1,1,1,1,1,0,1,0},{1,1,1,1,1,1,1,1,1,0},
                                 {1,1,1,0,0,0,1,0,1,1},{1,1,1,0,0,0,1,1,1,1},
                                 {1,1,1,0,1,0,1,0,1,1},{1,1,1,1,0,0,0,0,1,1},
                                 {1,1,1,1,0,0,1,0,1,1},{1,1,1,1,1,0,0,0,1,1},
                                 {1,1,1,1,1,0,1,0,1,1},{1,1,1,1,1,1,0,0,1,1}};

//create variables from defines
int current_mux_gpio[MUX_PINS]          = CURRENT_MUX_GPIO;
int ground_mux_gpio[MUX_PINS]           = GROUND_MUX_GPIO;
int voltage_mux_gpio[MUX_PINS]          = VOLTAGE_MUX_GPIO;
int current_switch_gpio[I_SWTCH_PINS]   = CURRENT_SWITCH_GPIO;
int mux_disable_gpio[3]                 = MUX_DISABLE_GPIO;
int adc_reset_gpio                      = ADC_RESET_GPIO;
int i_sense_reset_gpio                  = I_SENSE_RESET_GPIO;
int oled_reset_gpio                     = OLED_RESET_GPIO;
int oled_power_gpio                     = OLED_POWER_GPIO;

//gpio_info structs (see gpiolib.h)
gpio_info *current_mux_gpio_info[MUX_PINS];         //mux logic pins
gpio_info *ground_mux_gpio_info[MUX_PINS];
gpio_info *voltage_mux_gpio_info[MUX_PINS];
gpio_info *mux_disable_gpio_info[3];                //mux disable pins
gpio_info *current_switch_gpio_info[I_SWTCH_PINS];  //current source switch logic pins
gpio_info *adc_reset_gpio_info;                     //ADC RST pin, must be high for ADC to work
gpio_info *i_sense_reset_gpio_info;                 //current sense RST pin
gpio_info *oled_reset_gpio_info;                    //OLED reset pin, pull low for a few milliseconds to reinitialize display
gpio_info *oled_power_gpio_info;                    //OLED power pin, pull high to turn on display

//UI
int button = -1;

/* Oh Compiler-Please leave me as is */
volatile unsigned char flag = 0;

enum menus
{   
    HOME,
    SETTINGS,
    NODES,
    CURRENT,
    CONFIG,
    SAMPLING,
    TIME,
    CYCLE,
};

const char MENU_OPTS[8][OPT_STR_LEN] = {"HOME","SETTINGS","NODES","CURRENT","CONFIG","MODE","TIME","CYCLES"};
const char HOME_OPTS[2][OPT_STR_LEN] = {"START","SETTINGS"};
const char SETTINGS_OPTS[4][OPT_STR_LEN] = {"NODES","CURRENT","CONFIG","MODE"};
const char NODES_OPTS[7][OPT_STR_LEN] = {"8","12","16","20","24","28","32"};
const char CURRENT_OPTS[20][OPT_STR_LEN] = {"100","200","300","400","500","600","700","800","900","1000","1100","1200","1300","1400","1500","1600","1700","1800","1900","2000"};
const char CONFIG_OPTS[2][OPT_STR_LEN] = {"ACROSS","ADJACENT"};
const char SAMPLING_OPTS[3][OPT_STR_LEN] = {"TIMED","CYCLES","CONT."};
const char TIME_OPTS[13][OPT_STR_LEN] = {"5 s","10 s","20 s","30 s","45 s","1 min","2 min","5 min","10 min","20 min","30 min","1 hr","2 hr"};
const char CYCLE_OPTS[13][OPT_STR_LEN] = {"1","5","10","20","30","40","50","100","200","500","1000","5000","10000"};

const int time_opts[13] = {5, 10, 20, 30, 45, 60, 120, 300, 600, 1200, 1800, 3600, 7200}; //time options in seconds

#define HOME_OPTS_LEN 2
#define SETTINGS_OPTS_LEN 4
#define NODES_OPTS_LEN 7
#define CURRENT_OPTS_LEN 20
#define CONFIG_OPTS_LEN 2
#define SAMPLING_OPTS_LEN 3
#define TIME_OPTS_LEN 13
#define CYCLE_OPTS_LEN 13

//enums
enum buttons
{
    SELECT,
    PREV,
    NEXT,
    BACK,
    EXIT,
};

enum sample_mode
{
    TIMED,
    CYCLES,
    CONTINUOUS,
};

enum sample_geom
{
    ACROSS,
    ADJACENT,
};

enum adc_channels
{
    NODE,
    CURRENT_SENSOR,
    BATTERY,
};

enum system_states
{
    UI,
    RUNNING, 
    EXITING,
};

/************************************************************************************
* STRUCTS
*************************************************************************************/
typedef struct config_t{
    int nodal_num;
    double adc_scale[CHANNELS];
    int adc_offset[CHANNELS];
    int channels[CHANNELS];
    int sample_mode;
    int time;
    int cycles;
    int sample_geom;
    int i_setpoint;
} config_t;

/************************************************************************************
* FUNCTIONS
*************************************************************************************/

/****************************************************************************
* int mux_config_across(int nodal_num, int cur[],int gnd[], int volt[])
*
* Configures current and ground nodes in "across" switching pattern.
* Works for square samples with N nodes per edge.
*
* Inputs :  nodal_num, # of nodes to be sampled
*           cur[], zero current node array (size = N*4)
*           gnd[], zero gnd node array     (size = N*4)
*           volt[], zero volt node array   (size = N*4)
* 
* Outputs:  TODO: return -1 on failure
******************************************************************************/
int mux_config_across(int nodal_num, int cur[],int gnd[], int volt[]){
    int i;
    int side_len = (nodal_num/4); 
    int node_index = 3*side_len;
    for(i=0; i < nodal_num; i++){
        cur[i]  = i;
        gnd[i]  = node_index - 1;
        volt[i] = i;
        node_index = node_index - 1;
        if((node_index % (side_len))==0){
            node_index = node_index + (nodal_num/2);
            if (node_index > nodal_num){
                node_index = node_index % nodal_num;
            }
        }
    }
    return 0;
}

/****************************************************************************
* int mux_config_sym_across(int nodal_num, int cur[],int gnd[], int volt[])
*
* Configures current and ground nodes in "across" switching pattern.
* Utilizes symmetry by sampling half the amount of nodes
* Works for square samples with N nodes per edge.
*
* Inputs :  nodal_num, # of nodes to be sampled
*           cur[], zero current node array (size = N*2)
*           gnd[], zero gnd node array     (size = N*2)
*           volt[], zero volt node array   (size = N*2)
* 
* Outputs:  TODO: return -1 on failure
******************************************************************************/
int mux_config_sym_across(int nodal_num, int cur[],int gnd[], int volt[]){
    int i,j;
    int side_len = (nodal_num/4); 
    int node_index = 3*side_len;
		
    for(i=0; i < (nodal_num/2); i++){
        cur[i]  = i;
        gnd[i]  = node_index - 1;
        node_index = node_index - 1;
        if((node_index % (side_len))==0){
            node_index = node_index + (nodal_num/2);
            if (node_index > nodal_num){
                node_index = node_index % nodal_num;
            }
        }
    }
		
    for(j =0; j< nodal_num; j++){
	volt[j] = j;    
    }
	
    return 0;
}

/****************************************************************************
* int mux_config_adjacent(int nodal_num,int cur[],int gnd[],int volt[])
*
* Configures current,ground, and voltage nodes in "adjacent" switching pattern.
* Works for square samples with N nodes per edge.
*
* Inputs :  nodal_num, # of nodes to be sampled
*           cur[],  zero current node array (size = N*4)
*           gnd[],  zero gnd node array     (size = N*4)
*           volt[], zero volt node array    (size = N*4)
*
* Outputs:
*****************************************************************************/
int mux_config_adjacent(int nodal_num,int cur[],int gnd[],int volt[]){
    int i;
    for(i = 0; i < nodal_num; i++){
        cur[i] = i;
        gnd[i] = i + 1;
        volt[i] = i;
        if (i == (nodal_num-1) ){
            gnd[i] = 0;
        }
    }
    return 0;
}

/****************************************************************************
* int data_conversion()
*
* -Reads raw voltages from VOLT_DATA_TEXT
* -Performs voltage conversion on raw measurement and writes data to TEMP_VOLT_DATA_TEXT
* -Places all voltages for one cycle on a tab separated row
* Original raw file is removed and tempory file is renamed to highest incremented file in directory
* 
* Do not write/store any additional text files to /media/card/data
* 
* Inputs :  currnt, nodal_num, cycles, time, and frequency
* Outputs: return -1 on failure, 0 on success
*                 
*****************************************************************************/
int data_conversion(int current, int nodal_num, int cycles, float time, float freq){
        //buffers
	char data_buff[8];
	char write_buff[150];
	int len;
	int index = 0;
        //voltage value and conversion factor
	float volt_value;
	double volt_scale = 0.078127104;



        //opening raw data file for reading
	fp = fopen(VOLT_DATA_TEXT,"r");
	if(NULL == fp) {
		perror("ERROR in opening raw data file\n");
		return -1;
	}
        //creating text file for converted and formatted voltage values
	fd = open(TEMP_VOLT_DATA_TEXT,O_RDWR | O_CREAT | S_IRGRP | S_IROTH, 750);
	if(fd<0){
		perror("ERROR in opening temporary data file\n");
		fprintf(stderr, "path may not exist\n");
		return -1;
	}

        /*COMMENT THIS OUT TO REMOVE DATA FILE HEADERS*/
	len = snprintf(write_buff, sizeof(write_buff),"Current:\t%duA\nNodes:\t\t%d\nCycles:\t\t%d\nElapsed time:\t%0.5f seconds\nFrequency:\t%0.5f Hz\n\n",current,nodal_num,cycles,time,freq);     
	write(fd,write_buff,len);
        ////////////////////////////////////////////////

        // converts/formats raw values and writes them to new text file
	while(fgets(data_buff,8,fp)!= NULL){
		volt_value = atoi(data_buff)*(volt_scale/1000);
                //prints a newline once a nodal_num values have been written
                //tab seperates values in row
		if(index == (nodal_num-1)){
			len = snprintf(write_buff, sizeof(write_buff),"%.9f\n",volt_value);     
			write(fd,write_buff,len);
			index = 0;
		}
		else{
			len = snprintf(write_buff, sizeof(write_buff),"%.9f\t",volt_value);     
			write(fd,write_buff,len);
			index++;
		}

	}

        //closes text files
	fclose(fp);
	close(fd);
        //removes raw data file
	remove(VOLT_DATA_TEXT);


	DIR * dirp;
	struct dirent * entry;
	char *p1, *p2;
	int file_count = 0;
	int ret;

        //counts # of text files inside specified directory
	dirp = opendir("/media/card/data");
	while ((entry = readdir(dirp)) != NULL){
		p1=strtok(entry->d_name,".");
		p2=strtok(NULL,".");
		if(p2!=NULL){
			ret=strcmp(p2,"txt");
			if(ret==0){
				file_count++;
			}
		}

	}
	closedir(dirp);

        //renames text file to a highest increment within directory
	char path[66];
	int i = 1;
	int k =0;
	snprintf(path,sizeof(path),RAW_PATH "_%d.txt",i);
	while(1){
		if( (access( path,F_OK ) != -1) ) {
			k++;
		}
		i++;
		snprintf(path,sizeof(path),RAW_PATH "_%d.txt",i);
		if(k == (file_count-1)){
			if(k == 0){
				snprintf(path,sizeof(path),RAW_PATH "_%d.txt",1);

			}
			break;
		}

	}

	rename(TEMP_VOLT_DATA_TEXT,path);

	return 0;
}

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

#include "includes/ti-ads8684.h"
#include "includes/gpiolib.h"
#include "includes/eit.h"
#include "includes/UI.h"

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
int eit_gpio_attach(int gpio_pin, gpio_info *info){
    int bank, mask;
    bank = gpio_pin/32;
    mask = bit(gpio_pin%32);
    info = gpio_attach(bank, mask, GPIO_OUT);
    if(info == NULL){
        perror("ERROR in eit_gpio_attach, unable to attach gpio\n");
        fprintf(stderr, "maybe device tree is too old or the gpio pin is already exported\n");
        return -1;
    }
    return 0;
}

int attach_all_gpio(){
    int i;
    for(i = 0; i < MUX_PINS; i++){
        if(eit_gpio_attach(current_mux_gpio[i], current_mux_gpio_info[i])<0){                   //current
            perror("ERROR in eit_gpio_attach, unable to attach gpio\n");
            fprintf(stderr, "maybe device tree is too old or gpio%d is already exported\n",current_mux_gpio[i]);
            return -1;
        }
        if(eit_gpio_attach(ground_mux_gpio[i], ground_mux_gpio_info[i])<0){                     //ground
            perror("ERROR in eit_gpio_attach, unable to attach gpio\n");
            fprintf(stderr, "maybe device tree is too old or gpio%d is already exported\n",ground_mux_gpio[i]);
            return -1;
        }
        if(eit_gpio_attach(voltage_mux_gpio[i], voltage_mux_gpio_info[i])<0){                   //voltage
            perror("ERROR in eit_gpio_attach, unable to attach gpio\n");
            fprintf(stderr, "maybe device tree is too old or gpio%d is already exported\n",voltage_mux_gpio[i]);
            return -1;
        }
    }

    for(i = 0; i < 10; i++){
        if(eit_gpio_attach(current_switch_gpio[i], current_switch_gpio_info[i])<0){             //current source switches
            perror("ERROR in eit_gpio_attach, unable to attach gpio\n");
            fprintf(stderr, "maybe device tree is too old or gpio%d is already exported\n",current_switch_gpio[i]);
            return -1;
        }
    }

    for(i = 0; i < 3; i++){
        if(eit_gpio_attach(mux_disable_gpio[i], mux_disable_gpio_info[i])<0){                   //mux disable
            perror("ERROR in eit_gpio_attach, unable to attach gpio\n");
            fprintf(stderr, "maybe device tree is too old or gpio%d is already exported\n",mux_disable_gpio[i]);
            return -1;
        }
    }

    if(eit_gpio_attach(adc_reset_gpio, adc_reset_gpio_info)<0){                                 //adc reset
        perror("ERROR in eit_gpio_attach, unable to attach gpio\n");
        fprintf(stderr, "maybe device tree is too old or gpio%d is already exported\n",adc_reset_gpio);
        return -1;
    }

    if(eit_gpio_attach(i_sense_reset_gpio, i_sense_reset_gpio_info)<0){                         //current sensor reset
        perror("ERROR in eit_gpio_attach, unable to attach gpio\n");
        fprintf(stderr, "maybe device tree is too old or gpio%d is already exported\n",i_sense_reset_gpio);
        return -1;
    }

    return 0;
}
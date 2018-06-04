/***************************************************************************
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

#include "includes/ti-ads8684.h"
#include "includes/gpiolib.h"
#include "includes/eit_config.h"
#include "includes/eit.h"
// #include "includes/UI.h"

/************************************************************************************
* ENUMS/DEFINES/VARIABLES
*************************************************************************************/
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

#define MUX_PINS 5
#define I_SWTCH_PINS 10

// gpio pin IDs, see eit_config.h
int current_mux_gpio[MUX_PINS]          = CURRENT_MUX_GPIO;
int ground_mux_gpio[MUX_PINS]           = GROUND_MUX_GPIO;
int voltage_mux_gpio[MUX_PINS]          = VOLTAGE_MUX_GPIO;
int current_switch_gpio[I_SWTCH_PINS]   = CURRENT_SWITCH_GPIO;
int mux_disable_gpio[3]                 = MUX_DISABLE_GPIO;
int adc_reset_gpio                      = ADC_RESET_GPIO;
int i_sense_reset_gpio                  = I_SENSE_RESET_GPIO;

//gpio_info structs for all GPIO pins
gpio_info *current_mux_gpio_info[MUX_PINS];         //mux logic pins
gpio_info *ground_mux_gpio_info[MUX_PINS];
gpio_info *voltage_mux_gpio_info[MUX_PINS];
gpio_info *mux_disable_gpio_info[3];                //mux disable pins
gpio_info *current_switch_gpio_info[I_SWTCH_PINS];  //current source switch logic pins
gpio_info *adc_reset_gpio_info;                     //ADC RST pin, must be high for ADC to work
gpio_info *i_sense_reset_gpio_info;                 //current sense RST pin

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

    // fclose(fp);
    // printf("\n file has closed\n\n");
    exit(0);
}
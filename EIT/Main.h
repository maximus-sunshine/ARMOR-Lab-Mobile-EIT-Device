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
 * Main.h
 * 
 * The Big Kahuna. This is the script that runs on boot. It does everything.
 ***************************************************************************/

/************************************************************************************
* INCLUDES
*************************************************************************************/
#include "includes/ti-ads8684.h"
#include "includes/gpiolib.h"
#include "includes/eit_config.h"
#include "includes/eit.h"
#include "includes/UI.h"


/************************************************************************************
* ENUMS/DEFINES
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
        fprintf(stderr, "maybe device tree is too old or the gpio pin is already\n");
        return -1;
    }
    return 0;
}

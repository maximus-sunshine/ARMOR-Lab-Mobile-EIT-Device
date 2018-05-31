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
 * Main.c
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
#include "includes/Main.h"

/************************************************************************************
* STRUCTS
*************************************************************************************/
config_t config = {
    .nodal_num		= 32,
    .adc_scale		= 0.078127104,
    .adc_offset		= 0,
    .sample_mode	= CONTINUOUS,
    .time			= 20,
    .cycles			= 100,
    .sample_geom	= ACROSS,
    .i_setpoint		= 100,
};

/************************************************************************************
* MAIN
*************************************************************************************/

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

typedef struct config_t{
    int nodal_num;
    double adc_scale;
    int adc_offset;
    int sample_mode;
    int time;
    int cycles;
    int sample_geom;
    int i_setpoint;
} config_t;

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

#define MUX_PINS 5
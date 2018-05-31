/*****************************************************************
 * ---------------------------------------------------------------
 * ARMOR Lab @UC San Diego, Kenneth Loh Ph.D
 * 
 * MAE 156B Spring 2018 Team 6: Warfighter Protection
 * 	- Maxwell Sun		(maxsun96@gmail.com)
 *	- Jacob Rutheiser	(jrutheiser@gmail.com)
 *	- Matthew Williams	(mwilliams31243@gmail.com)
 *	- Aaron Gunn		(gunnahg@gmail.com)
 * ---------------------------------------------------------------
 * 
 * ti-ads8684.h
 * 
 * Basic interface for the TI-ADS8684 ADC, an iio device
 ********************************************************************/

#ifndef TI_ADS8684_H
#define TI_ADS8684_H

//ADC characteristics
#define CHANNELS 4
#define IIO_DIR				"/sys/bus/iio/devices/"
#define ADC_NAME 			"iio:device1"
#define HRTIMER_TRIG_NAME	"trigger0"
#define SYSFS_TRIG_NAME		"trigger1"
#define MAX_BUF 64

#define SCALES_AVAIL	{0.312504320, 0.156254208, 0.078127104}
#define OFFSETS_AVAIL  	{-32768, 0}

/***************************************************************
* FUNCTION DECLARATIONS
****************************************************************/

/****************************************************************************
* int ti_adc_init()
*
* Initializes ADC library by opening file descriptors used by functions defined in this library.
* THIS DOES NOT ENABLE THE ADC. The ADC must be enabled by pulling the enable pin high. 
* This function sets an init_flag high.
*
* Inputs : none
* 
* Outputs: returns 0 on succes, -1 on failure
*****************************************************************************/
int ti_adc_init();

/****************************************************************************
* int ti_adc_cleanup()
*
* Safely cleans up ADC library by closing all file descriptors. Sets init_flag low.
*
* Inputs : none
* 
* Outputs: returns 0 on success
*****************************************************************************/
int ti_adc_cleanup();

/****************************************************************************
* int ti_adc_enable()
*
* Sets ADC reset pin high
*
* Inputs : none
* Outputs: returns 0 on success, -1 on failure
*****************************************************************************/
int ti_adc_enable();

/****************************************************************************
* int ti_adc_disable()
*
* Sets ADC reset pin low
*
* Inputs : none
* Outputs: returns 0 on success, -1 on failure
*****************************************************************************/
int ti_adc_disable();

/****************************************************************************
* int ti_adc_set_offset(int ch, int offset)
*
* Sets offset of specified channel
*
* Inputs : 	ch, 	channel [0-4]
*			offset,	offset 	[-32768, 0]
* 
* Outputs:	0 on success, -1 on failure
*****************************************************************************/
int ti_adc_set_offset(int ch, int offset);

/**************************************************************************************************************
* int ti_adc_set_scale(int ch, float scale)
*
* Sets offset of specified channel
*
* Inputs : 	ch, 	channel [0-4]
*			scale,	scale 	[0.312504320, 0.156254208, 0.078127104] corresponds to 20.48V, 10.24V, or 5.12V FSR
* 
* Outputs:	0 on success, -1 on failure
***************************************************************************************************************/
int ti_adc_set_scale(int ch, double scale);

/****************************************************************************
* int ti_adc_enable_channel(int ch)
*
* Enables specified channel for buffered scanning
*
* Inputs :	ch,	channel [0-4]
* 
* Outputs:	0 on success, -1 on failure
*****************************************************************************/
int ti_adc_enable_channel(int ch);

/****************************************************************************
* int ti_adc_disable_channel(int ch)
*
* Disables specified channel for buffered scanning
*
* Inputs :	ch,	channel [0-4]
* 
* Outputs:	0 on success, -1 on failure
*****************************************************************************/
int ti_adc_disable_channel(int ch);

/****************************************************************************
* int ti_adc_set_buf_length()
*
* Sets buffer length
*
* Inputs :	length, length of buffer (integer)
* 
* Outputs:	0 on success, -1 on failure
*****************************************************************************/
int ti_adc_set_buf_length(int length);

/****************************************************************************
* int ti_adc_set_hrtimer_freq(int freq)
*
* Sets hrtimer_trigger frequency
*
* Inputs :	freq, hrtimer frequency (integer?)
* 
* Outputs:	0 on success, -1 on failure
*****************************************************************************/
int ti_adc_set_hrtimer_freq(int freq);

/****************************************************************************
* int ti_adc_enable_buf()
*
* Enables buffered scanning of ADC
*
* Inputs : none
* 
* Outputs: 0 on success, -1 on failure
*****************************************************************************/
int ti_adc_enable_buf();

/****************************************************************************
* int ti_adc_disable_buf(int ch)
*
* Disables buffered scanning of ADC
*
* Inputs : none
* 
* Outputs: 0 on success, -1 on failure
*****************************************************************************/
int ti_adc_disable_buf();

/****************************************************************************
* int ti_adc_read_raw(int ch)
*
* Reads raw, 16-bit value of specified ADC channel
*
* Inputs : ch,	channel [0-4]
* 
* Outputs: ADC measurement on success, -1 on failure
*****************************************************************************/
int ti_adc_read_raw(int ch);

/****************************************************************************
* int ti_adc_sysfs_read()
*
* Writes to sysfs trigger for a buffered ADC scan
*
* Inputs : none
* 
* Outputs: 0 on success, -1 on failure
*****************************************************************************/
int ti_adc_sysfs_read();


#endif //TI_ADS8684_H
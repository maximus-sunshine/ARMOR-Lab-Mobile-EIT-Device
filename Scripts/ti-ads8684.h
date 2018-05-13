/*****************************************************************
 * MAE 156B Spring 2018 Team 6
 *
 * Basic interface for the TI-ADS8684 ADC, an iio device
 *
 * Copied from James Strawson's GitHub
 *******************************************************************/

#ifndef TI_ADS8684_H
#define TI_ADS8684_H

//ADC characteristics
#define CHANNELS 4
#define IIO_DIR "/sys/bus/iio/devices/iio:device1"
#define MAX_BUF 64

/***************************************************************
* FUNCTION DECLARATIONS
****************************************************************/

/****************************************************************************
* int ti_adc_init()
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_init();

/****************************************************************************
* int ti_adc_cleanup()
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_cleanup();

/****************************************************************************
* int ti_adc_enable()
*
* Set ADC Reset pin to high
*
* TODO: ERROR HANDLING 
*
* Inputs : 
* Outputs:	
*****************************************************************************/
int ti_adc_enable();

/****************************************************************************
* int ti_adc_disable()
*
* Set ADC Reset pin to low
*
* TODO: ERROR HANDLING 
*
* Inputs : 
* Outputs:	
*****************************************************************************/
int ti_adc_disable();

/****************************************************************************
* int ti_adc_set_offset(int ch, int offset)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_set_offset(int ch, int offset);

/****************************************************************************
* int ti_adc_set_scale(int ch, float scale)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_set_scale(int ch, double scale);

/****************************************************************************
* int ti_adc_enable_channel(int ch)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_enable_channel(int ch);

/****************************************************************************
* int ti_adc_disable_channel(int ch)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_disable_channel(int ch);

/****************************************************************************
* int ti_adc_set_buf_length()
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_set_buf_length(int length);

/****************************************************************************
* int ti_adc_set_sample_rate(int length)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_set_sample_rate(int freq);

/****************************************************************************
* int ti_adc_enable_buf(int ch)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_enable_buf();

/****************************************************************************
* int ti_adc_disable_buf(int ch)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_disable_buf();

/****************************************************************************
* int ti_adc_read_raw(int ch)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_read_raw(int ch);



#endif //TI_ADS8684_H
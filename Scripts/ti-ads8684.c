/*****************************************************************
 * MAE 156B Spring 2018 Team 6
 *
 * Basic interface for the TI-ADS8684 ADC, an iio device
 *
 * Copied from James Strawson's GitHub
 *
 * TODO: -comment more, add DESCRIPTIONS to functions
 *       -add error handling for incorrect offset or scale
 *******************************************************************/

/***************************************************************
* INCLUDES/DEFINES
****************************************************************/

#include <stdio.h>
#include <stdlib.h> // for atoi
#include <errno.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include "ti-ads8684.h"

// preposessor macros
#define unlikely(x)	__builtin_expect (!!(x), 0)



// float SCALES_AVAIL[3] {0.312504320, 0.156254208, 0.078127104};
// int OFFSETS_AVAIL[2] {-32768, 0};
// #define RAW_MAX 65536
// #define RAW_MIN -32,768

static int init_flag = 0;		// boolean to check if mem mapped
static int fd_raw[CHANNELS];	// file descriptors for 4 channels (raw)
static int fd_offset[CHANNELS];	// file descriptors for 4 channels (offset)
static int fd_scale[CHANNELS];	// file descriptors for 4 channels (scale)
static int adc_rst_dir_fd;	// file descriptors for ADC RST pin (direction)
static int adc_rst_val_fd;	// file descriptors for ADC RST pin (value)


/***************************************************************
* FUNCTION DEFINITIONS
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
int ti_adc_init()
{
	char buf[MAX_BUF];
	int i, temp_fd;

	//open file descriptors for reading ADC channels
	for(i=0;i<CHANNELS;i++){
		snprintf(buf, sizeof(buf), IIO_DIR "/in_voltage%d_raw",i);
		temp_fd = open(buf, O_RDONLY);
		if(temp_fd<0){
			perror("ERROR in ti_adc_init, failed to open adc interface for reading\n");
			fprintf(stderr, "maybe kernel or device tree is too old\n");
			return -1;
		}
		fd_raw[i]=temp_fd;
	}

	//open file descriptors for setting ADC channel offset
	for(i=0;i<CHANNELS;i++){
		snprintf(buf, sizeof(buf), IIO_DIR "/in_voltage%d_offset",i);
		temp_fd = open(buf, O_WRONLY);
		if(temp_fd<0){
			perror("ERROR in ti_adc_init, failed to open adc interface for writing offset\n");
			fprintf(stderr, "maybe kernel or device tree is too old\n");
			return -1;
		}
		fd_offset[i]=temp_fd;
	}

	//open file descriptors for setting ADC channel scale
	for(i=0;i<CHANNELS;i++){
		snprintf(buf, sizeof(buf), IIO_DIR "/in_voltage%d_scale",i);
		temp_fd = open(buf, O_WRONLY);
		if(temp_fd<0){
			perror("ERROR in ti_adc_init, failed to open adc interface for writing scale\n");
			fprintf(stderr, "maybe kernel or device tree is too old\n");
			return -1;
		}
		fd_scale[i]=temp_fd;
	}

	//open file descriptors for ADC reset pins and set direction to out
	char dir_buf_rd[MAX_BUF];
	char val_buf_rd[MAX_BUF];
	
	snprintf(dir_buf_rd, sizeof(dir_buf_rd), "/sys/class/gpio/gpio13/direction");
	snprintf(val_buf_rd, sizeof(val_buf_rd), "/sys/class/gpio/gpio13/value");
	
	adc_rst_dir_fd = open(dir_buf_rd, O_WRONLY);
	adc_rst_val_fd = open(val_buf_rd, O_WRONLY);

	char dir_buf_wr[MAX_BUF];
	snprintf(dir_buf_wr, sizeof(dir_buf_wr), "out");
	write(adc_rst_dir_fd, dir_buf_wr, sizeof(dir_buf_wr));

	init_flag = 1;
	return 0;
}

/****************************************************************************
* int ti_adc_cleanup()
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_cleanup()
{
	int i;
	for(i=0;i<CHANNELS;i++){
		close(fd_raw[i]);
		close(fd_offset[i]);
		close(fd_scale[i]);
	}
	close(adc_rst_dir_fd);
	close(adc_rst_val_fd);
	init_flag = 0;
	return 0;
}

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
int ti_adc_enable(){
	char val_buf_wr[MAX_BUF];
	snprintf(val_buf_wr, sizeof(val_buf_wr), "1");
	write(adc_rst_val_fd, val_buf_wr, sizeof(val_buf_wr));
	return 0;
}

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
int ti_adc_disable(){
	char val_buf_wr[MAX_BUF];
	snprintf(val_buf_wr, sizeof(val_buf_wr), "0");
	write(adc_rst_val_fd, val_buf_wr, sizeof(val_buf_wr));
	return 0;
}

/****************************************************************************
* int ti_adc_set_offset(int ch, int offset)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_set_offset(int ch, int offset)
{
	char buf[MAX_BUF];
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_set_offset, please initialize with ti_adc_init() first\n");
		return -1;
	}
	if(unlikely(ch<0 || ch>=CHANNELS)){
		fprintf(stderr,"ERROR: in ti_adc_set_offset, adc channel must be between 0 & %d\n", CHANNELS-1);
		return -1;
	}

	//write offset
	snprintf(buf, sizeof(buf), "%d",offset);
	if(unlikely(write(fd_offset[ch], buf, sizeof(buf))==-1)){
		perror("ERROR in ti_adc_set_offset, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

/****************************************************************************
* int ti_adc_set_scale(int ch, float scale)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_set_scale(int ch, double scale)
{
	char buf[MAX_BUF];
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_set_offset, please initialize with ti_adc_init() first\n");
		return -1;
	}
	if(unlikely(ch<0 || ch>=CHANNELS)){
		fprintf(stderr,"ERROR: in ti_adc_set_offset, adc channel must be between 0 & %d\n", CHANNELS-1);
		return -1;
	}

	//write scale, TODO: format %f correctly
	snprintf(buf, sizeof(buf), "%.9f", scale);
	if(unlikely(write(fd_scale[ch], buf, sizeof(buf))==-1)){
		perror("ERROR in ti_adc_set_scale, can't write to iio adc fd. attempted to write");
		return -1;
	}
	return 0;
}

/****************************************************************************
* int ti_adc_read_raw(int ch)
*
* DESCRIPTION
*
* Inputs :	
* 
* Outputs:	
*****************************************************************************/
int ti_adc_read_raw(int ch)
{
	char buf[5];
	int i;
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in rc_adc_read_raw, please initialize with rc_adc_init() first\n");
		return -1;
	}
	if(unlikely(ch<0 || ch>=CHANNELS)){
		fprintf(stderr,"ERROR: in rc_adc_read_raw, adc channel must be between 0 & %d\n", CHANNELS-1);
		return -1;
	}

	if(unlikely(lseek(fd_raw[ch],0,SEEK_SET)<0)){
		perror("ERROR: in rc_adc_read_raw, failed to seek to beginning of FD");
		return -1;
	}

	if(unlikely(read(fd_raw[ch], buf, sizeof(buf))==-1)){
		perror("ERROR in rc_adc_read_raw, can't read iio adc fd");
		return -1;
	}
	i=atoi(buf);
	// if(i>RAW_MAX || i< RAW_MIN){
	// 	fprintf(stderr, "ERROR: in rc_adc_read_raw, value out of bounds: %d\n", i);
	// 	return -1;
	// }
	return i;
}

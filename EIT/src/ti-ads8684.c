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
 * ti-ads8684.c
 * 
 * Basic interface for the TI-ADS8684 ADC, an iio device
 ********************************************************************/

/********************************************************************
* INCLUDES/DEFINES
********************************************************************/

#include <stdio.h>
#include <stdlib.h> // for atoi
#include <errno.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include "../includes/ti-ads8684.h"

// preprocessor macros
#define unlikely(x)	__builtin_expect (!!(x), 0)

static int init_flag = 0;		// boolean to check if mem mapped

/********************************************************************
* FILE DESCRIPTORS
********************************************************************/

//ADC channels
static int fd_raw[CHANNELS];	// file descriptors for 4 channels (raw)
static int fd_offset[CHANNELS];	// file descriptors for 4 channels (offset)
static int fd_scale[CHANNELS];	// file descriptors for 4 channels (scale)
static int fd_enable[CHANNELS];	// file descriptors for 4 channels (enable)

//available scales and offsets
static int scales_avail[3] = SCALES_AVAIL;
static int offsets_avail[2] = OFFSETS_AVAIL;

//Buffer
static int buf_length_fd;		// file descriptor for buffer length
static int buf_enable_fd;		// file descriptor for buffer length

//Trigger
static int current_trigger_fd;	// file descriptor for adc current trigger
static int sysfs_trig_fd;		// file descriptor for sysfs trigger
static int hrt_frequency_fd;	// file descriptor for hrtimer trigger frequency

//ADC reset pin
static int adc_rst_dir_fd;		// file descriptor for ADC RST pin (direction)
static int adc_rst_val_fd;		// file descriptor for ADC RST pin (value)

/***************************************************************
* FUNCTION DEFINITIONS
****************************************************************/

int ti_adc_init()
{
	char buf[MAX_BUF];
	int i, temp_fd;

	/* RAW READ CHANNELS */
	for(i=0;i<CHANNELS;i++){
		snprintf(buf, sizeof(buf), IIO_DIR ADC_NAME "/in_voltage%d_raw",i);
		temp_fd = open(buf, O_RDONLY);
		if(temp_fd<0){
			perror("ERROR in ti_adc_init, failed to open adc interface for reading\n");
			fprintf(stderr, "maybe kernel or device tree is too old\n");
			return -1;
		}
		fd_raw[i]=temp_fd;
	}


	/* CHANNEL OFFSETS */
	for(i=0;i<CHANNELS;i++){
		snprintf(buf, sizeof(buf), IIO_DIR ADC_NAME "/in_voltage%d_offset",i);
		temp_fd = open(buf, O_WRONLY);
		if(temp_fd<0){
			perror("ERROR in ti_adc_init, failed to open adc interface for writing offset\n");
			fprintf(stderr, "maybe kernel or device tree is too old\n");
			return -1;
		}
		fd_offset[i]=temp_fd;
	}


	/* CHANNEL SCALES */
	for(i=0;i<CHANNELS;i++){
		snprintf(buf, sizeof(buf), IIO_DIR ADC_NAME "/in_voltage%d_scale",i);
		temp_fd = open(buf, O_WRONLY);
		if(temp_fd<0){
			perror("ERROR in ti_adc_init, failed to open adc interface for writing scale\n");
			fprintf(stderr, "maybe kernel or device tree is too old\n");
			return -1;
		}
		fd_scale[i]=temp_fd;
	}


	/* SCAN ELEMENTS CHANNEL ENABLES */
	for(i=0;i<CHANNELS;i++){
		snprintf(buf, sizeof(buf), IIO_DIR ADC_NAME "/scan_elements/in_voltage%d_en",i);
		temp_fd = open(buf, O_WRONLY);
		if(temp_fd<0){
			perror("ERROR in ti_adc_init, failed to open adc interface for writing enable\n");
			fprintf(stderr, "maybe kernel or device tree is too old\n");
			return -1;
		}
		fd_enable[i]=temp_fd;
	}

	/* BUFFER */
	//open file descriptors for buffer. (length and enable)
	char buf1[MAX_BUF];
	char buf2[MAX_BUF];
	snprintf(buf1, sizeof(buf1), IIO_DIR ADC_NAME "/buffer/length");
	snprintf(buf2, sizeof(buf2), IIO_DIR ADC_NAME "/buffer/enable");
	
	buf_length_fd = open(buf1, O_WRONLY);
	buf_enable_fd = open(buf2, O_WRONLY);
	
	if(buf_length_fd<0 || buf_enable_fd<0){
		perror("ERROR in ti_adc_init, failed to open adc interface for buffer\n");
		fprintf(stderr, "maybe kernel or device tree is too old\n");
		return -1;
	}


	/* TRIGGER */
	//open file descriptor for ADC trigger
	snprintf(buf, sizeof(buf), IIO_DIR ADC_NAME "/trigger/current_trigger");
	current_trigger_fd = open(buf, O_WRONLY);
	if(current_trigger_fd<0){
		perror("ERROR in ti_adc_init, failed to open adc interface for current_trigger\n");
		fprintf(stderr, "maybe kernel or device tree is too old\n");
		return -1;
	}


	/* SYSFS_TRIGGER */
	//open file descriptor for sysfs trigger
	snprintf(buf, sizeof(buf), IIO_DIR SYSFS_TRIG_NAME "/trigger_now");
	sysfs_trig_fd = open(buf, O_WRONLY);
	if(sysfs_trig_fd<0){
		perror("ERROR in ti_adc_init, failed to open adc interface for sysfs trigger\n");
		fprintf(stderr, "maybe kernel or device tree is too old\n");
		return -1;
	}

	//set ADC trigger to be hrtimer trigger
	snprintf(buf, sizeof(buf), IIO_DIR ADC_NAME "/trigger/current_trigger");
	current_trigger_fd = open(buf, O_WRONLY);

	snprintf(buf, sizeof(buf), SYSFS_TRIG_NAME);
	if(write(current_trigger_fd, buf, sizeof(buf))<0){
		perror("ERROR in ti_adc_init, failed to write to current_trigger\n");
		fprintf(stderr, "maybe kernel or device tree is too old\n");
		return -1;
	}

	/* ADC ENABLE */
	// //open file descriptors for ADC reset pins and set direction to out
	// snprintf(buf1, sizeof(buf1), "/sys/class/gpio/gpio13/direction");
	// snprintf(buf2, sizeof(buf2), "/sys/class/gpio/gpio13/value");
	
	// adc_rst_dir_fd = open(buf1, O_WRONLY);
	// adc_rst_val_fd = open(buf2, O_WRONLY);
	
	// if(adc_rst_dir_fd<0 || adc_rst_val_fd<0){
	// 	perror("ERROR in ti_adc_init, failed to open adc interface for reset pin\n");
	// 	fprintf(stderr, "maybe kernel or device tree is too old\n");
	// 	return -1;
	// }

	// char dir_buf_wr[MAX_BUF];
	// snprintf(dir_buf_wr, sizeof(dir_buf_wr), "out");
	// if(write(adc_rst_dir_fd, dir_buf_wr, sizeof(dir_buf_wr))<0){
	// 	perror("ERROR in ti_adc_init, failed to write to adc_rst_dir\n");
	// 	fprintf(stderr, "maybe kernel or device tree is too old\n");
	// 	return -1;
	// }


	/* HRTIMER_TRIGGER */
	// // open file descriptor for hrtimer trigger frequency
	// snprintf(buf, sizeof(buf), IIO_DIR HRTIMER_TRIG_NAME "/sampling_frequency");
	// hrt_frequency_fd = open(buf, O_WRONLY);
	
	// if(hrt_frequency_fd<0){
	// 	perror("ERROR in ti_adc_init, failed to open adc interface for hrtimer trigger frequency\n");
	// 	fprintf(stderr, "maybe kernel or device tree is too old\n");
	// 	return -1;
	// }

	// snprintf(buf, sizeof(buf), HRTIMER_TRIG_NAME);
	// if(write(current_trigger_fd, buf, sizeof(buf))<0){
	// 	perror("ERROR in ti_adc_init, failed to to write to current_trigger\n");
	// 	fprintf(stderr, "maybe kernel or device tree is too old\n");
	// 	return -1;
	// }


	/* RAISE INIT FLAG AND RETURN */
	init_flag = 1;
	return 0;
}

int ti_adc_cleanup()
{
	int i;
	for(i=0;i<CHANNELS;i++){
		close(fd_raw[i]);
		close(fd_offset[i]);
		close(fd_scale[i]);
		close(fd_enable[i]);
	}
	close(adc_rst_dir_fd);
	close(adc_rst_val_fd);
	close(buf_length_fd);
	close(buf_enable_fd);
	close(sysfs_trig_fd);
	close(current_trigger_fd);
	close(hrt_frequency_fd);
	init_flag = 0;
	return 0;
}

int ti_adc_enable(){
	char val_buf_wr[MAX_BUF];
	snprintf(val_buf_wr, sizeof(val_buf_wr), "1");
	if(write(adc_rst_val_fd, val_buf_wr, sizeof(val_buf_wr))<0){
		perror("ERROR in ti_adc_enable, failed to write to enable pin\n");
		fprintf(stderr, "maybe kernel or device tree is too old, or maybe gpio pin is not exported properly\n");
		return -1;
	}
	return 0;
}

int ti_adc_disable(){
	char val_buf_wr[MAX_BUF];
	snprintf(val_buf_wr, sizeof(val_buf_wr), "0");
	if(write(adc_rst_val_fd, val_buf_wr, sizeof(val_buf_wr))<0){
		perror("ERROR in ti_adc_disable, failed to write to enable pin\n");
		fprintf(stderr, "maybe kernel or device tree is too old, or maybe gpio pin is not exported properly\n");
		return -1;
	}
	return 0;
}

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
	if(unlikely(offset != offsets_avail[0] && offset != offsets_avail[1])){
		fprintf(stderr,"ERROR: in ti_adc_set_offset, offset must be either %d or %d\n", offsets_avail[0], offsets_avail[1]);
		return -1;
	}

	//write offset
	snprintf(buf, sizeof(buf), "%d",offset);
	if(unlikely(write(fd_offset[ch], buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_set_offset, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

int ti_adc_set_scale(int ch, double scale)
{
	char buf[MAX_BUF];
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_set_scale, please initialize with ti_adc_init() first\n");
		return -1;
	}
	if(unlikely(ch<0 || ch>=CHANNELS)){
		fprintf(stderr,"ERROR: in ti_adc_set_scale, adc channel must be between 0 & %d\n", CHANNELS-1);
		return -1;
	}
	if(unlikely(scale != scales_avail[0] && scale != scales_avail[1] && scale != scales_avail[2])){
		fprintf(stderr,"ERROR: in ti_adc_set_scale, scale must be %d, %d, or %d\n", scales_avail[0], scales_avail[1], scales_avail[2]);
		return -1;
	}

	//set scale
	snprintf(buf, sizeof(buf), "%.9f", scale);
	if(unlikely(write(fd_scale[ch], buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_set_scale, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

int ti_adc_enable_channel(int ch)
{
	char buf[5];
	int i;
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_enable_channel, please initialize with ti_adc_init() first\n");
		return -1;
	}
	if(unlikely(ch<0 || ch>=CHANNELS)){
		fprintf(stderr,"ERROR: in ti_adc_enable_channel, adc channel must be between 0 & %d\n", CHANNELS-1);
		return -1;
	}

	//enable channel
	snprintf(buf, sizeof(buf), "1");
	if(unlikely(write(fd_enable[ch], buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_enable_channel, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

int ti_adc_disable_channel(int ch)
{
	char buf[5];
	int i;
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_disable_channel, please initialize with ti_adc_init() first\n");
		return -1;
	}
	if(unlikely(ch<0 || ch>=CHANNELS)){
		fprintf(stderr,"ERROR: in ti_adc_disable_channel, adc channel must be between 0 & %d\n", CHANNELS-1);
		return -1;
	}

	//disable channel
	snprintf(buf, sizeof(buf), "0");
	if(unlikely(write(fd_enable[ch], buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_disable_channel, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

int ti_adc_set_buf_length(int length)
{
	char buf[MAX_BUF];
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_set_buf_length, please initialize with ti_adc_init() first\n");
		return -1;
	}

	//set buffer length
	snprintf(buf, sizeof(buf), "%d", length);
	if(unlikely(write(buf_length_fd, buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_set_buf_length, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

int ti_adc_set_hrtimer_freq(int freq)
{
	char buf[MAX_BUF];
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_set_hrtimer_freq, please initialize with ti_adc_init() first\n");
		return -1;
	}

	//set frequency
	snprintf(buf, sizeof(buf), "%d", freq);
	if(unlikely(write(hrt_frequency_fd, buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_set_hrtimer_freq, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

int ti_adc_enable_buf()
{
	char buf[5];
	int i;
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_enable_buf, please initialize with ti_adc_init() first\n");
		return -1;
	}

	//enable buffer
	snprintf(buf, sizeof(buf), "1");
	if(unlikely(write(buf_enable_fd, buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_enable_buf, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

int ti_adc_disable_buf()
{
	char buf[5];
	int i;
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_disable_buf, please initialize with ti_adc_init() first\n");
		return -1;
	}

	//disable buffer
	snprintf(buf, sizeof(buf), "0");
	if(unlikely(write(buf_enable_fd, buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_disable_buf, can't write to iio adc fd");
		return -1;
	}
	return 0;
}

int ti_adc_read_raw(int ch)
{
	char buf[5];
	int i;
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_read_raw, please initialize with ti_adc_init() first\n");
		return -1;
	}
	if(unlikely(ch<0 || ch>=CHANNELS)){
		fprintf(stderr,"ERROR: in ti_adc_read_raw, adc channel must be between 0 & %d\n", CHANNELS-1);
		return -1;
	}
	if(unlikely(lseek(fd_raw[ch],0,SEEK_SET)<0)){
		perror("ERROR: in ti_adc_read_raw, failed to seek to beginning of FD");
		return -1;
	}
	if(unlikely(read(fd_raw[ch], buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_read_raw, can't read iio adc fd");
		return -1;
	}
	i=atoi(buf);
	return i;
}

int ti_adc_sysfs_read()
{
	char buf[5];
	int i;
	
	//sanity checks
	if(unlikely(!init_flag)){
		fprintf(stderr,"ERROR in ti_adc_sysfs_read, please initialize with ti_adc_init() first\n");
		return -1;
	}

	//write to sysfs_trigger
	snprintf(buf, sizeof(buf), "1");
	if(unlikely(write(sysfs_trig_fd, buf, sizeof(buf))<0)){
		perror("ERROR in ti_adc_sysfs_read, can't write to iio adc fd");
		return -1;
	}
	return 0;
}
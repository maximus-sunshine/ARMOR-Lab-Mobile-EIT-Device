/**
 * MAE 156B Spring 2018 Team 6
 *
 * Basic interface to the reading the TI-ADS8684 ADC, an iio device
 *
 * Copied from James Strawson's GitHub
 */

#include <stdio.h>
#include <stdlib.h> // for atoi
#include <errno.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

// preposessor macros
#define unlikely(x)	__builtin_expect (!!(x), 0)

#define CHANNELS 4
#define IIO_DIR "/sys/bus/iio/devices/iio:device1"
#define MAX_BUF 64
// #define RAW_MAX 65536
// #define RAW_MIN -32,768

static int init_flag = 0; // boolean to check if mem mapped
static int fd[CHANNELS]; // file descriptors for 8 channels

/***************************************************************
* FUNCTIONS
****************************************************************/
int ti_adc_init()
{
	char buf[MAX_BUF];
	int i, temp_fd;

	for(i=0;i<CHANNELS;i++){
		snprintf(buf, sizeof(buf), IIO_DIR "/in_voltage%d_raw",i);
		temp_fd = open(buf, O_RDONLY);
		if(temp_fd<0){
			perror("ERROR in ti_adc_init, failed to open adc interface\n");
			fprintf(stderr, "maybe kernel or device tree is too old\n");
			return -1;
		}
		fd[i]=temp_fd;
	}
	init_flag = 1;
	return 0;
}

int adc_cleanup()
{
	int i;
	for(i=0;i<CHANNELS;i++){
		close(fd[i]);
	}
	init_flag = 0;
	return 0;
}

int adc_read_raw(int ch)
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
	// if(unlikely(lseek(fd[ch],0,SEEK_SET)<0)){
	// 	perror("ERROR: in rc_adc_read_raw, failed to seek to beginning of FD");
	// 	return -1;
	// }
	if(unlikely(read(fd[ch], buf, sizeof(buf))==-1)){
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

int loops = 100000;
float scale = 0.078127104;
float SAMPLE_RATE = 500000;

int main()
{
	
	int j;
	for(j=0;j<loops;j++){
		ti_adc_init();
		printf("%f V\n", adc_read_raw(2)*scale/1000);
		adc_cleanup();
		usleep(1000000/SAMPLE_RATE);
	}
	
}

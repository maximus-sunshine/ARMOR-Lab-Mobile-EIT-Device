/************************************************************************************
 * MAE 156B Spring 2018 Team 6 (5/9/18)
 *	- Maxwell Sun
 *	- Matthew Williams
 *	- Aaron Gunn
 *	- Jacob Rutheiser
 *
 * Script to test functionality of libiio
 * 
 *
 * compile with "gcc batt_read.c ti-ads8684.c -o batt_read"
 *
 ************************************************************************************/
	
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>

#ifdef __APPLE__
#include <iio/iio.h>
#else
#include <iio.h>
#endif

//Config stuff
static char *name        = "ads8684";
static char *trigger_str = "trigger0";
static int buffer_length = 2;
static int count         = -1;

// Streaming devices
static struct iio_device *dev;

/* IIO structs required for streaming */
static struct iio_context *ctx;
static struct iio_buffer  *rxbuf;
static struct iio_channel **channels;
static int channel_count;

static bool stop;
static bool has_repeat;

/* cleanup and exit */
static void shutdown()
{
	if (channels) { free(channels); }

	printf("* Destroying buffers\n");
	if (rxbuf) { iio_buffer_destroy(rxbuf); }

	printf("* Disassociate trigger\n");
	if (dev) { iio_device_set_trigger(dev, NULL); }

	printf("* Destroying context\n");
	if (ctx) { iio_context_destroy(ctx); }
	exit(0);
}

void sigint(int s __attribute__((unused))) {
	printf("waiting for process to finish...\n");
	stop = true;
}

/************************************************************************************
* MAIN
*************************************************************************************/
int main(){
	// Hardware trigger
	struct iio_device *trigger;
	
	// Listen to ctrl+c and assert
	signal(SIGINT, sigint);

	//Acquire IIO context
	printf("* Acquiring IIO context\n");
	assert((ctx = iio_create_default_context()) && "No context");
	assert(iio_context_get_devices_count(ctx) > 0 && "No devices");

	//Acquire IIO device
	printf("* Acquiring device %s\n", name);
	dev = iio_context_find_device(ctx, name);
	if (!dev) {
		perror("No device found");
		shutdown();
	}

	//Initializing streaming channels
	printf("* Initializing IIO streaming channels:\n");
	//for (int i = 0; i < iio_device_get_channels_count(dev); ++i) {
	for (int i = 0; i < 2; ++i) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);
		if (iio_channel_is_scan_element(chn)) {
			printf("%s\n", iio_channel_get_id(chn));
			channel_count++;
		}
	}
	if (channel_count == 0) {
		printf("No scan elements found\n");
		shutdown();
	}
	channels = calloc(channel_count, sizeof *channels);
	if (!channels) {
		perror("Channel array allocation failed");
		shutdown();
	}
	for (int i = 0; i < channel_count; ++i) {
		struct iio_channel *chn = iio_device_get_channel(dev, i);
		if (iio_channel_is_scan_element(chn))
			channels[i] = chn;
	}

	//Acquire trigger
	printf("* Acquiring trigger %s\n", trigger_str);
	trigger = iio_context_find_device(ctx, trigger_str);
	if (!trigger || !iio_device_is_trigger(trigger)) {
		perror("No trigger found (try setting up the iio-trig-hrtimer module)");
		shutdown();
	}

	//Enable streaming channels
	printf("* Enabling IIO streaming channels for buffered capture\n");
	for (int i = 0; i < channel_count; ++i)
		iio_channel_enable(channels[i]);

	//Enable buffer trigger
	printf("* Enabling IIO buffer trigger\n");
	if (iio_device_set_trigger(dev, trigger)) {
		perror("Could not set trigger\n");
		shutdown();
	}

	//Create buffer
	printf("* Creating non-cyclic IIO buffers with %d samples\n", buffer_length);
	rxbuf = iio_device_create_buffer(dev, buffer_length, false);
	if (!rxbuf) {
		perror("Could not create buffer");
		shutdown();
	}

	//Start streaming
	// printf("* Starting IO streaming (press CTRL+C to cancel)\n");
	// bool has_ts = strcmp(iio_channel_get_id(channels[channel_count-1]), "timestamp") == 0;
	// int64_t last_ts = 0;


	//Timing
	struct timeval t1, t2;
	gettimeofday(&t1, NULL);
	int count = 0;

	while (!stop)
	{
		ssize_t nbytes_rx;
		void *p_dat, *p_end;
		ptrdiff_t p_inc;
		// int64_t now_ts;

		// Refill RX buffer
		nbytes_rx = iio_buffer_refill(rxbuf);
		if (nbytes_rx < 0) {
			printf("Error refilling buf: %d\n", (int)nbytes_rx);
			shutdown();
		}

		p_inc = iio_buffer_step(rxbuf);
		p_end = iio_buffer_end(rxbuf);

		// // Print timestamp delta in ms
		// if (has_ts){
		// 	for (p_dat = iio_buffer_first(rxbuf, channels[channel_count-1]); p_dat < p_end; p_dat += p_inc) {
		// 		now_ts = (((int64_t *)p_dat)[0]);
		// 		printf("[%04ld] ", last_ts > 0 ? (now_ts - last_ts)/1000/1000 : 0);
		// 		last_ts = now_ts;
		// 	}
		// }

		// Print each captured sample
		for (int i = 0; i < channel_count; ++i) {
			const struct iio_data_format *fmt = iio_channel_get_data_format(channels[i]);
			unsigned int repeat = has_repeat ? fmt->repeat : 1;

			//printf("%s ", iio_channel_get_id(channels[i]));
			for (p_dat = iio_buffer_first(rxbuf, channels[i]); p_dat < p_end; p_dat += p_inc) {
				for (int j = 0; j < repeat; ++j) {
					if (fmt->length/8 == sizeof(int16_t))
						((int16_t *)p_dat)[j];
						//printf("%i ", ((int16_t *)p_dat)[j]);
					else if (fmt->length/8 == sizeof(int64_t))
						//printf("%ld ", ((int64_t *)p_dat)[j]);
						((int64_t *)p_dat)[j];
				}
			}
		}
		//printf("\n");
		count++;
	}
	//end timing
	gettimeofday(&t2, NULL);
	double usec = 1e6*(t2.tv_sec - t1.tv_sec) + t2.tv_usec - t1.tv_usec;
	printf("\n\nTime elapsed: %0.5f seconds, frequency: %0.5f Hz\n\n", usec/1e6, count/(usec/1e6));

	shutdown();

	return 0;
}
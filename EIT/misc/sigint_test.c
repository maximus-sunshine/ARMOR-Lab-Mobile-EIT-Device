/************************************************************************************
 * MAE 156B Spring 2018 Team 6
 *	- Maxwell Sun
 *	- Matthew Williams
 *	- Aaron Gunn
 *	- Jacob Rutheiser
 *
 * Script to test SIGINT handling.
 * 
 **
 ************************************************************************************/

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

void sigint(int s __attribute__((unused)));

int main(){
	signal(SIGINT, sigint);
	int i = 0;
	while(1){
		printf("\nran %d times...", i);
		fflush(stdout);
		i++;
		usleep(0.1 * 1e6);
	}
}

void sigint(int s __attribute__((unused))) {
	printf("\n\n received SIGINT, exiting cleanly...\n\n");
	exit(0);
}
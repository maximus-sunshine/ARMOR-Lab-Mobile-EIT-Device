#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>

#define BUFFER_MAX 3

int main(){
  int pin = 61;
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;
  fd = open("/sys/class/gpio/export",O_WRONLY);
  if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}
  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return 0;
}


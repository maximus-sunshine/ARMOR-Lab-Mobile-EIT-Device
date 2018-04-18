#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>

#define BUFFER_MAX 3

int main(){
//exporting gpio pin 61	
  int pin = 61;
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;
  fd = open("/sys/class/gpio/export",O_WRONGLY);
  if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}
  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
//setting gpio pin direction to out	
  char buffer_2[4];
  int fd_dir;
  char direction[] = "out";
  fd_dir = open("/sys/class/gpio/gpio%d/direction",pin);
  if (-1 == fd) {
		 fprintf(stderr, "Failed to open direction for writing!\n");
		 return(-1);
  }
  int bytes_written_2 = snprintf(buffer_2, 4, "%s",direction);
  write(fd_dir,buffer_2,bytes_written_2);
  close(fd_dir);
  return 0;
}

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int export_gpio(int pin);
static int set_gpio_dir(int pin, int dir);
static int set_gpio_value(int pin, int val);
static int unexport_gpio(int pin);
static int read_gpio(int pin);
static int clean_up(int pin);
static int adc_init(int channel);
static int read_adc_raw(int channel);
static int adc_cleanup(int channel);

int fd_adc[4];
int volt_channel = 0;

int main(){
  int chan[8][3] = {{1,0,0},{1,1,0},{1,1,1},
                    {1,0,1},{0,0,1},{0,1,0},
                    {0,1,1},{0,0,0}};
  int demux1[8] = {1,2,3,4,5,6,7,8};
  int demux2[8] = {6,5,8,7,2,1,4,3};
  int mux[8][6] = {{2,3,4,5,7,8},{1,3,4,6,7,8},
                  {1,2,4,5,6,7},{1,2,3,5,6,8},
                  {1,3,4,6,7,8},{2,3,4,5,7,8},
                  {1,2,3,5,6,8},{1,2,4,5,6,7}};


  int demux1_a0 = 5;
  int demux1_a1 = 6;
  int demux1_a2 = 7;
  int demux2_a0 = 8;
  int demux2_a1 = 9;
  int demux2_a2 = 10;
  int mux_a0 = 2;
  int mux_a1 = 3;
  int mux_a2 = 4;
  //adc initialization
  adc_init(volt_channel);
  // exporting gpio pins needed
  export_gpio(demux1_a0);
  export_gpio(demux1_a1);
  export_gpio(demux1_a2);
  export_gpio(demux2_a0);
  export_gpio(demux2_a1);
  export_gpio(demux2_a2);
  export_gpio(mux_a0);
  export_gpio(mux_a1);
  export_gpio(mux_a2);

	
  // setting gpio direction to outputs(1)	
  set_gpio_dir(demux1_a0,1);
  set_gpio_dir(demux1_a1,1);
  set_gpio_dir(demux1_a2,1);
  set_gpio_dir(demux2_a0,1);
  set_gpio_dir(demux2_a1,1);
  set_gpio_dir(demux2_a2,1);
  set_gpio_dir(mux_a0,1);
  set_gpio_dir(mux_a1,1);
  set_gpio_dir(mux_a2,1);


  float bits_to_volts = 5/32767;
  int flag = 0;
  while(flag < 200){
    for(int i = 0; i<=7; i++){
      //power and ground distribution
      set_gpio_value(demux1_a0,chan[demux1[i]-1][2]);
      set_gpio_value(demux1_a1,chan[demux1[i]-1][1]);
      set_gpio_value(demux1_a2,chan[demux1[i]-1][0]);

      set_gpio_value(demux2_a0,chan[demux1[i]-1][2]);
      set_gpio_value(demux2_a1,chan[demux1[i]-1][1]);
      set_gpio_value(demux2_a2,chan[demux1[i]-1][0]);


      //inner loop controls sampling
      for(int j =0; j <= 5; j++){
        set_gpio_value(mux_a0, chan[mux[i][j]-1][2]);
        set_gpio_value(mux_a1, chan[mux[i][j]-1][1]);
        set_gpio_value(mux_a2, chan[mux[i][j]-1][0]);
	      
        int value = read_adc_raw(volt_channel);
        float voltage = value*bits_to_volts;
        printf(" %.3f",voltage);
      }
        printf("\n");
        printf("--------------Current Configuration %d ------------------ \n",i+1);
    }
      k++;
      printf(" ******************** Cylce %d *************************",k);
  }
  //setting all gpio mux pins to low
  clean_up(demux1_a0);
  clean_up(demux1_a1);
  clean_up(demux1_a2);
  clean_up(demux2_a0);
  clean_up(demux2_a1);
  clean_up(demux2_a2);
  clean_up(mux_a0);
  clean_up(mux_a1);
  clean_up(mux_a2);
  //closing adc file	
  adc_clean_up(volt_channel);

  return 0;
}


static int adc_init(int channel){
  char path[66];
  int temp_fd;
  snprintf(path,66,"/sys/bus/iio/devices/iio:device1/in_voltage%d_raw",channel);
  temp_fd = open(path,O_RDONLY);
  if (-1 == fd){
    fprintf(stderr,"Failed to open adc for reading!\n");
    return -1;
  }
  fd_adc[channel] = temp_fd;
}

static int read_adc_raw(int channel){
  char value_read[20];
  if(-1 == lseek(fd_adc[channel],0,SEEK_SET)){
     fprintf(stderr, "Failed to go to begginning of file\n");
     return -1;
  }
  int fd_read = read(fd_adc[channel],value_read,20);
  if (-1 == fd_read)) {
    fprintf(stderr, "Failed to read value!\n");
    return(-1);
  }
  return(atoi(value_read));
}

static int adc_cleanup(int channel){
  close(fd_adc[channel]);
  return 0;
}

static int export_gpio(int pin){
  char buffer[4];
  ssize_t bytes_written;
  int fd;
  fd = open("/sys/class/gpio/export",O_WRONLY);
  if (-1 == fd){
    fprintf(stderr,"Failed to open export for writing!\n");
    return -1;
  }
  bytes_written = snprintf(buffer, 4, "%d", pin);
  write(fd,buffer,bytes_written);
  close(fd);
  return 0;
}

static  int set_gpio_dir(int pin, int dir){
  char buffer[4];
  char path[35];
  int fd;
  if (dir == 1)
    char direction[] = "out";
  else
    char direction[] = "in";

  snprintf(path,35,"/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd){
    fprintf(stderr,"Failed to open direction for writing!\n");
    return -1;
  }
  int bytes_written = snprintf(buffer,4,"%s",direction);
  write(fd,buffer,bytes_written);
  close(fd);
  return 0;
}

static int set_gpio_value(int pin, int val){
  char buffer[2];
  char path[35];
  int fd;
  snprintf(path,35,"/sys/class/gpio/gpio%d/value",pin);
  fd = open(path,O_WRONLY);
  if (-1 == fd){
    fprintf(stderr,"Failed to open value for writing!\n");
    return -1;
  }
  int bytes_written = snprintf(buffer,2,"%d",val);
  write(fd,buffer,bytes_written);
  close(fd);
  return 0;
}

static int unexport_gpio(int pin){
  char buffer[4];
  ssize_t bytes_written;
  int fd;
  fd = open("/sys/class/gpio/unexport",O_WRONLY);
  if (-1 == fd){
    fprintf(stderr,"Failed to open unexport for writing!\n");
    return -1;
  }
  bytes_written = snprintf(buffer, 4, "%d", pin);
  write(fd,buffer,bytes_written);
  close(fd);
  return 0;
}

static int read_gpio(int pin){
  char path[35];
  char value_read[20];
  int fd;
  snprintf(path,35,"/sys/class/gpio/gpio%d/value",pin);
  fd = open(path,O_WRONLY);
  if (-1 == fd){
    fprintf(stderr,"Failed to open value for reading!\n");
    return -1;
  }
  if (-1 == read(fd, value_read, 20)) {
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}
  close(fd);
  return(atoi(value_read));
}

static int clean_up(int pin){
  set_gpio_value(pin, 0);
  return 0;
}

/**
 * MAE 156B Spring 2018 Team 6
 *
 * Test script to blink 32 LEDS
 *
 */

/**********************
* INCLUDES
***********************/
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h> // for atoi
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/**********************
* DEFINES
***********************/
#define NODAL_NUM 32           //# of nodes, can be 8,12,16,20,24,and 32
#define side_len (NODAL_NUM/4) //# of nodes per side

/**********************
* SETUP
***********************/

/* MUX */
//logic array declaration
int chan[32][5] = {{0,0,0,0,0},{0,0,0,0,1},{0,0,0,1,0},{0,0,0,1,1}, 
                   {0,0,1,0,0},{0,0,1,0,1},{0,0,1,1,0},{0,0,1,1,1},
                   {0,1,0,0,0},{0,1,0,0,1},{0,1,0,1,0},{0,1,0,1,1},
                   {0,1,1,0,0},{0,1,1,0,1},{0,1,1,1,0},{0,1,1,1,1},
                   {1,0,0,0,0},{1,0,0,0,1},{1,0,0,1,0},{1,0,0,1,1},
                   {1,0,1,0,0},{1,0,1,0,1},{1,0,1,1,0},{1,0,1,1,1},
                   {1,1,0,0,0},{1,1,0,0,1},{1,1,0,1,0},{1,1,0,1,1},
                   {1,1,1,0,0},{1,1,1,0,1},{1,1,1,1,0},{1,1,1,1,1}};

//mux array declarations
int current_mux[NODAL_NUM];           // current?                                           
int ground_mux[NODAL_NUM];            // ground?
int volt_mux[NODAL_NUM][NODAL_NUM-2]; // voltage sampling?

//geometry
int node_index = 3*(side_len); //starting node_index of ground

//gpio pin IDs, used for exporting pins
int current_mux_a0 = 8;
int current_mux_a1 = 9;
int current_mux_a2 = 10;
int current_mux_a3 = 87;
int current_mux_a4 = 86;

/**********************
* FUNCTION DEFINITIONS
***********************/

//exports pin for gpio use
static int export_gpio(int pin){
  char buffer[4];
  ssize_t bytes_written;
  int fd;
  fd = open("/sys/class/gpio/export",O_WRONLY);
  if (fd<0){
    fprintf(stderr,"Failed to open export for writing!\n");
    return -1;
  }
  bytes_written = snprintf(buffer, 4, "%d", pin);
  write(fd,buffer,bytes_written);
  close(fd);
  return 0;
}

//sets the direction of gpio pin to either in our out
static  int set_gpio_dir(int pin, int dir){
  char buffer[4];
  char path[35];
  int fd;
  char direction[4];
  if (1 == dir){
    strcpy(direction, "out");
}
  else{
    strcpy(direction, "in");;
}

  snprintf(path,35,"/sys/class/gpio/gpio%d/direction", pin);
  printf("\n/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (fd<0){
    fprintf(stderr,"Failed to open direction for writing!\n");
    return -1;
  }
  int bytes_written = snprintf(buffer,4,"%s",direction);
  write(fd,buffer,bytes_written);
  close(fd);
  return 0;
}

//sets value of gpio pin to either high(1) or low(0)
static int set_gpio_value(int pin, int val){
  char buffer[2];
  char path[35];
  int fd;
  snprintf(path,35,"/sys/class/gpio/gpio%d/value",pin);
  fd = open(path,O_WRONLY);
  if (fd<0){
    fprintf(stderr,"Failed to open value for writing!\n");
    return -1;
  }
  int bytes_written = snprintf(buffer,2,"%d",val);
  write(fd,buffer,bytes_written);
  close(fd);
  return 0;
}

//unexports gpio pin
static int unexport_gpio(int pin){
  char buffer[4];
  ssize_t bytes_written;
  int fd;
  fd = open("/sys/class/gpio/unexport",O_WRONLY);
  if (-1 == fd){
    fprintf(stderr,"Unable to unexport: failed to open unexport for writing!\n");
    return -1;
  }
  bytes_written = snprintf(buffer, 4, "%d", pin);
  write(fd,buffer,bytes_written);
  close(fd);
  return 0;
}

//reads gpio pin
//direction needs to be in to read
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

//sets gpio value to low
//unexports gpio value
static int clean_up(int pin){
  if(set_gpio_value(pin, 0)<1){
    fprintf(stderr,"Failed to clean up: failed to open value for reading!\n");
  };
  unexport_gpio(pin);
  return 0;
}

/**********************
* MAIN
***********************/

int main(){

  //configures current and ground nodes according to # of nodes(NODAL_NUM)
  int n;
  for(n = 0;n<=(NODAL_NUM-1);n++){
    current_mux[n] = n + 1;          //current starts at first node and increments to the end
  }

  // //export GPIO pins
  // export_gpio(current_mux_a0);
  // export_gpio(current_mux_a1);
  // export_gpio(current_mux_a2);
  // export_gpio(current_mux_a3);
  // export_gpio(current_mux_a4);

  // //set direction to output
  // set_gpio_dir(current_mux_a0,1);
  // set_gpio_dir(current_mux_a1,1);
  // set_gpio_dir(current_mux_a2,1);
  // set_gpio_dir(current_mux_a3,1);
  // set_gpio_dir(current_mux_a4,1);

  // //blink them LEDs
  // int i;
  // int flag = 0;
  // int loops = 2;
  // //runs 'loops' times
  // while(flag < loops){
  //   for(i = 0; i <= (NODAL_NUM-1); i++){
  //     set_gpio_value(current_mux_a0,chan[current_mux[i]-1][4]);
  //     set_gpio_value(current_mux_a1,chan[current_mux[i]-1][3]);
  //     set_gpio_value(current_mux_a2,chan[current_mux[i]-1][2]);
  //     set_gpio_value(current_mux_a3,chan[current_mux[i]-1][1]);
  //     set_gpio_value(current_mux_a4,chan[current_mux[i]-1][0]);
  //   }
  //   flag++;
  //   usleep(100000);
  // }

  //blink them LEDs
  int i;
  int flag = 0;
  int loops = 2;
  //runs 'loops' times
  while(flag < loops){
    for(i = 0; i <= (NODAL_NUM-1); i++){
      set_gpio_value(current_mux_a0,1);
      set_gpio_value(current_mux_a1,1);
      set_gpio_value(current_mux_a2,1);
      set_gpio_value(current_mux_a3,1);
      set_gpio_value(current_mux_a4,1);
    }
    flag++;
    usleep(100000);
  }

  // //set all gpio mux pins to low and unexport them
  // clean_up(current_mux_a0);
  // clean_up(current_mux_a1);
  // clean_up(current_mux_a2);
  // clean_up(current_mux_a3);
  // clean_up(current_mux_a4);

  // //unexport GPIO pins
  // unexport_gpio(current_mux_a0);
  // unexport_gpio(current_mux_a1);
  // unexport_gpio(current_mux_a2);
  // unexport_gpio(current_mux_a3);
  // unexport_gpio(current_mux_a4);
  
  return 0;
}
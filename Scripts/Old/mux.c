/**
 * MAE 156B Spring 2018 Team 6
 *
 * Basic interface for MUX switching
 *
 */

/**********************
* INCLUDES
***********************/
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/**********************
* DEFINES
***********************/
#define NODAL_NUM 32//# of nodes, can be 8,12,16,20,24,and 32

/**********************
* FUNCTION DECLARATIONS
***********************/

//GPIO function declarations
static int export_gpio(int pin);
static int set_gpio_dir(int pin, int dir);
static int set_gpio_value(int pin, int val);
static int unexport_gpio(int pin);
static int read_gpio(int pin);
static int clean_up(int pin);

//ADC fucntion declarations
static int adc_init(int channel);
static int adc_cleanup(int channel);
static int read_adc_raw(int channel);

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
int side_len = NODAL_NUM/4; //# of nodes per side
int node_index = 3*(side_len); //starting node_index of ground

//configures current and ground nodes according to # of nodes(NODAL_NUM)
int n;
for(n = 0;n<=(NODAL_NUM-1);n++){
  ground_mux[n] = node_index;           //ground starts at last node of third side
  current_mux[n] = n + 1;          //current starts at first node and increments to the end
  node_index  = node_index - 1;              //ground moves cc
  if((node_index % (side_len))==0){     //once it passes an edge node it adds half the # of nodes to node_index
    node_index = node_index + (NODAL_NUM/2);
    if (node_index > NODAL_NUM){        //if node_index ends up being greater then NODAL_NUM it takes the remainder
      node_index = node_index % NODAL_NUM;
    }
  }
}

//configures voltage sampling nodes according to # of nodes(NODAL_NUM)
int k = 0;
int a,b;
for(a = 0; a <= (NODAL_NUM-1); a++){
  for(b= 0; b <= (NODAL_NUM-1); b++){
    if((a != b) && (ground_mux[a] != current_mux[b])){
      volt_mux[a][k] = current_mux[b];
      k++;
    }
  }
  k = 0;
}

//gpio pin IDs, used for exporting pins
int current_mux_a0 = 8;
int current_mux_a1 = 9;
int current_mux_a2 = 10;
int current_mux_a3 = 87;
int current_mux_a4 = 86;

int ground_mux_a0 = 70;
int ground_mux_a1 = 72;
int ground_mux_a2 = 74;
int ground_mux_a3 = 76;
int ground_mux_a4 = 78;

int volt_mux_a0 = 15;
int volt_mux_a1 = 49;
int volt_mux_a2 = 14;
int volt_mux_a3 = 117;
int volt_mux_a4 = 125;

/* ADC */
int fd_adc[4];//4 files for adc
int volt_channel = 0;//voltage sampling done on channel one

int main(){

//exporting gpio pins for muxes
  export_gpio(current_mux_a0);
  export_gpio(current_mux_a1);
  export_gpio(current_mux_a2);
  export_gpio(current_mux_a3);
  export_gpio(current_mux_a4);
  export_gpio(ground_mux_a0);
  export_gpio(ground_mux_a1);
  export_gpio(ground_mux_a2);
  export_gpio(ground_mux_a3);
  export_gpio(ground_mux_a4);
  export_gpio(volt_mux_a0);
  export_gpio(volt_mux_a1);
  export_gpio(volt_mux_a2);
  export_gpio(volt_mux_a3);
  export_gpio(volt_mux_a4);


//setting the direction of gpio pins used for muxes to outputs(1)
  set_gpio_dir(current_mux_a0,1);
  set_gpio_dir(current_mux_a1,1);
  set_gpio_dir(current_mux_a2,1);
  set_gpio_dir(current_mux_a3,1);
  set_gpio_dir(current_mux_a4,1);
  set_gpio_dir(ground_mux_a0,1);
  set_gpio_dir(ground_mux_a1,1);
  set_gpio_dir(ground_mux_a2,1);
  set_gpio_dir(ground_mux_a3,1);
  set_gpio_dir(ground_mux_a4,1);
  set_gpio_dir(volt_mux_a0,1);
  set_gpio_dir(volt_mux_a1,1);
  set_gpio_dir(volt_mux_a2,1);
  set_gpio_dir(volt_mux_a3,1);
  set_gpio_dir(volt_mux_a4,1);

  int i,j;
  float bits_to_volts = 0.078127104/1000;//bits to volts conversion
  int flag = 0;
  //runs 200 times
  while(flag < 200){
    for(i = 0; i <= (NODAL_NUM-1); i++){
      //power and ground distribution
      set_gpio_value(current_mux_a0,chan[current_mux[i]-1][4]);
      set_gpio_value(current_mux_a1,chan[current_mux[i]-1][3]);
      set_gpio_value(current_mux_a2,chan[current_mux[i]-1][2]);
      set_gpio_value(current_mux_a3,chan[current_mux[i]-1][1]);
      set_gpio_value(current_mux_a4,chan[current_mux[i]-1][0]);

      set_gpio_value(ground_mux_a0,chan[ground_mux[i]-1][4]);
      set_gpio_value(ground_mux_a1,chan[ground_mux[i]-1][3]);
      set_gpio_value(ground_mux_a2,chan[ground_mux[i]-1][2]);
      set_gpio_value(ground_mux_a3,chan[ground_mux[i]-1][1]);
      set_gpio_value(ground_mux_a4,chan[ground_mux[i]-1][0]);

      //inner loop controls sampling
      for(j =0; j <= (NODAL_NUM-3); j++){
        set_gpio_value(volt_mux_a0, chan[mux[i][j]-1][4]);
        set_gpio_value(volt_mux_a1, chan[mux[i][j]-1][3]);
        set_gpio_value(volt_mux_a2, chan[mux[i][j]-1][2]);
        set_gpio_value(volt_mux_a3, chan[mux[i][j]-1][1]);
        set_gpio_value(volt_mux_a4, chan[mux[i][j]-1][0]);

        //opening adc file
        adc_init(volt_channel);
        //reading adc
        int value = read_adc_raw(volt_channel);
        float voltage = value*bits_to_volts;
        //closing adc file so it can update
        adc_cleanup(volt_channel);
        //printing voltage
        printf(" %.4f",voltage);
      }
        printf("\n");
        printf("--------------Current Configuration %d ------------------ \n",i+1);
    }
      flag++;
      printf(" ******************** Cylce %d *************************",flag);
  }
  //setting all gpio mux pins to low and unexporting them
  clean_up(current_mux_a0);
  clean_up(current_mux_a1);
  clean_up(current_mux_a2);
  clean_up(current_mux_a3);
  clean_up(current_mux_a4);
  clean_up(ground_mux_a0);
  clean_up(ground_mux_a1);
  clean_up(ground_mux_a2);
  clean_up(ground_mux_a3);
  clean_up(ground_mux_a4);
  clean_up(volt_mux_a0);
  clean_up(volt_mux_a1);
  clean_up(volt_mux_a2);
  clean_up(volt_mux_a3);
  clean_up(volt_mux_a4);

  return 0;
}

//opens specific channel file of adc
static int adc_init(int channel){
  char path[66];
  int temp_fd;
  snprintf(path,66,"/sys/bus/iio/devices/iio:device1/in_voltage%d_raw",channel);
  temp_fd = open(path,O_RDONLY);
  if (-1 == temp_fd){
    fprintf(stderr,"Failed to open adc for reading!\n");
    return -1;
  }
  fd_adc[channel] = temp_fd;
  return 0;
}

//read raw adc value from file
static int read_adc_raw(int channel){
  char value_read[20];
  int fd_read = read(fd_adc[channel],value_read,20);

  if (-1 == fd_read) {
    fprintf(stderr, "Failed to read value!\n");
    return(-1);
  }
  return(atoi(value_read));
}

//closes specific channel file of adc
static int adc_cleanup(int channel){
  close(fd_adc[channel]);
  return 0;
}

//exports pin for gpio use
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

//sets value of gpio pin to either high(1) or low(0)
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

//unexports gpio pin
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
  set_gpio_value(pin, 0);
  unexport_gpio(pin);
  return 0;
}
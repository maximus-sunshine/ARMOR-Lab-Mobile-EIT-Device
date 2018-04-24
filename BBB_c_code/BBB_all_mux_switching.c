#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define NODAL_NUM 32//# of nodes, can be 8,12,16,20,24,and 32

//gpio function declarations
static int export_gpio(int pin);
static int set_gpio_dir(int pin, int dir);
static int set_gpio_value(int pin, int val);
static int unexport_gpio(int pin);
static int read_gpio(int pin);
static int clean_up(int pin);
//adc fucntion declarations
static int adc_init(int channel);
static int adc_cleanup(int channel);
static int read_adc_raw(int channel);

int fd_adc[4];//4 files for adc
int volt_channel = 0;//voltage sampling done on channel one

int main(){

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
  int demux1[NODAL_NUM];
  int demux2[NODAL_NUM];
  int mux[NODAL_NUM][NODAL_NUM-2];

  int side_len = NODAL_NUM/4; //# of nodes per side
  int index = 3*(side_len); //starting index of ground

  //configures current and ground nodes according to # of nodes(NODAL_NUM)
  int n;
  for(n = 0;n<=(NODAL_NUM-1);n++){
    demux2[n] = index; // ground starts at last node of third side
    demux1[n] = n+1;  // current starts at first node and increments to the end
    index  = index -1; // ground moves cc
    if((index % (side_len))==0){ //once it passes an edge node it adds half the # of nodes to index
      index = index + (NODAL_NUM/2);
      if (index > NODAL_NUM){ // if index ends up being greater then NODAL_NUM it takes the remainder
        index = index % NODAL_NUM;
      }
    }
  }
  //configures voltage sampling nodes according to # of nodes(NODAL_NUM)
  int k = 0;
  int a,b;
  for(a = 0; a <= (NODAL_NUM-1); a++){
    for(b= 0; b <= (NODAL_NUM-1); b++){
      if((a != b) && (demux2[a] != demux1[b])){
        mux[a][k] = demux1[b];
        k++;
      }
    }
    k = 0;
  }


  //gpio pin declarations, used for muxes
  int demux1_a0 = 24;
  int demux1_a1 = 26;
  int demux1_a2 = 28;
  int demux1_a3 = 30;
  int demux1_a4 = 32;

  int demux2_a0 = 34;
  int demux2_a1 = 36;
  int demux2_a2 = 38;
  int demux2_a3 = 40;
  int demux2_a4 = 42;

  int mux_a0 = 44;
  int mux_a1 = 46;
  int mux_a2 = 48;
  int mux_a3 = 50;
  int mux_a4 = 52;

//exporting gpio pins for muxes
  export_gpio(demux1_a0);
  export_gpio(demux1_a1);
  export_gpio(demux1_a2);
  export_gpio(demux1_a3);
  export_gpio(demux1_a4);
  export_gpio(demux2_a0);
  export_gpio(demux2_a1);
  export_gpio(demux2_a2);
  export_gpio(demux2_a3);
  export_gpio(demux2_a4);
  export_gpio(mux_a0);
  export_gpio(mux_a1);
  export_gpio(mux_a2);
  export_gpio(mux_a3);
  export_gpio(mux_a4);


//setting the direction of gpio pins used for muxes to outputs(1)
  set_gpio_dir(demux1_a0,1);
  set_gpio_dir(demux1_a1,1);
  set_gpio_dir(demux1_a2,1);
  set_gpio_dir(demux1_a3,1);
  set_gpio_dir(demux1_a4,1);
  set_gpio_dir(demux2_a0,1);
  set_gpio_dir(demux2_a1,1);
  set_gpio_dir(demux2_a2,1);
  set_gpio_dir(demux2_a3,1);
  set_gpio_dir(demux2_a4,1);
  set_gpio_dir(mux_a0,1);
  set_gpio_dir(mux_a1,1);
  set_gpio_dir(mux_a2,1);
  set_gpio_dir(mux_a3,1);
  set_gpio_dir(mux_a4,1);

  int i,j;
  float bits_to_volts = 0.078127104/1000;//bits to volts conversion
  int flag = 0;
  //runs 200 times
  while(flag < 200){
    for(i = 0; i <= (NODAL_NUM-1); i++){
      //power and ground distribution
      set_gpio_value(demux1_a0,chan[demux1[i]-1][4]);
      set_gpio_value(demux1_a1,chan[demux1[i]-1][3]);
      set_gpio_value(demux1_a2,chan[demux1[i]-1][2]);
      set_gpio_value(demux1_a3,chan[demux1[i]-1][1]);
      set_gpio_value(demux1_a4,chan[demux1[i]-1][0]);

      set_gpio_value(demux2_a0,chan[demux2[i]-1][4]);
      set_gpio_value(demux2_a1,chan[demux2[i]-1][3]);
      set_gpio_value(demux2_a2,chan[demux2[i]-1][2]);
      set_gpio_value(demux2_a3,chan[demux2[i]-1][1]);
      set_gpio_value(demux2_a4,chan[demux2[i]-1][0]);

      //inner loop controls sampling
      for(j =0; j <= (NODAL_NUM-3); j++){
        set_gpio_value(mux_a0, chan[mux[i][j]-1][4]);
        set_gpio_value(mux_a1, chan[mux[i][j]-1][3]);
        set_gpio_value(mux_a2, chan[mux[i][j]-1][2]);
        set_gpio_value(mux_a3, chan[mux[i][j]-1][1]);
        set_gpio_value(mux_a4, chan[mux[i][j]-1][0]);

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
  clean_up(demux1_a0);
  clean_up(demux1_a1);
  clean_up(demux1_a2);
  clean_up(demux1_a3);
  clean_up(demux1_a4);
  clean_up(demux2_a0);
  clean_up(demux2_a1);
  clean_up(demux2_a2);
  clean_up(demux2_a3);
  clean_up(demux2_a4);
  clean_up(mux_a0);
  clean_up(mux_a1);
  clean_up(mux_a2);
  clean_up(mux_a3);
  clean_up(mux_a4);

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

#ifndef eit_sample
#define eit_sample

//TRUTH TABLE DIMENSIONS
#define LOGIC_ROW 32
#define LOGIC_COL 5

//TRUTH TABLE DECLARATION
int CHAN[LOGIC_ROW][LOGIC_COL] = {{0,0,0,0,0},{0,0,0,0,1},{0,0,0,1,0},{0,0,0,1,1},
                                 {0,0,1,0,0},{0,0,1,0,1},{0,0,1,1,0},{0,0,1,1,1},
                                 {0,1,0,0,0},{0,1,0,0,1},{0,1,0,1,0},{0,1,0,1,1},
                                 {0,1,1,0,0},{0,1,1,0,1},{0,1,1,1,0},{0,1,1,1,1},
                                 {1,0,0,0,0},{1,0,0,0,1},{1,0,0,1,0},{1,0,0,1,1},
                                 {1,0,1,0,0},{1,0,1,0,1},{1,0,1,1,0},{1,0,1,1,1},
                                 {1,1,0,0,0},{1,1,0,0,1},{1,1,0,1,0},{1,1,0,1,1},
                                 {1,1,1,0,0},{1,1,1,0,1},{1,1,1,1,0},{1,1,1,1,1}};
//NUMBER OF NODES
#define NODAL_NUM 32
//NUMBER OF NODES PER SIDE
#define SIDE_LEN (NODAL_NUM/4)                                 

//FUNCTION DECLARATIONS
int cur_gnd_config(int cur_mux[],int gnd_mux[]);
int volt_samp_config(int cur_mux[],int gnd_mux[],int volt[][NODAL_NUM-2]);




///////////////PIN DECLARATIONS////////////
//adc pins
#define ADC_RESET_GPIO 13
//current sense pin
#define I_SENSE_RESET_GPIO 27
//power mux declaration
#define CUR_MUX_A0  8    //p8_35
#define CUR_MUX_A1  9    //p8_33
#define CUR_MUX_A2  10   //p8_31
#define CUR_MUX_A3  87   //p8_29
#define CUR_MUX_A4  86   //p8_27
//ground mux declaraton
#define GND_MUX_A0  70   //p8_45
#define GND_MUX_A1  72   //p8_43
#define GND_MUX_A2  74   //p8_41
#define GND_MUX_A3  76   //p8_39
#define GND_MUX_A4  78   //p8_37
//voltage sampling delcaration
#define VOLT_MUX_A0 15   //p9_24
#define VOLT_MUX_A1 49   //p9_23
#define VOLT_MUX_A2 14   //p9_26
#define VOLT_MUX_A3 117  //p9_25
#define VOLT_MUX_A4 125  //p9_27

/////////////MUX ARRAY DECLARATIONS
#define CURRENT_MUX_GPIO {CUR_MUX_A4,CUR_MUX_A3,CUR_MUX_A2,CUR_MUX_A1,CUR_MUX_A0}
#define GROUND_MUX_GPIO {GND_MUX_A4,GND_MUX_A3,GND_MUX_A2,GND_MUX_A1,GND_MUX_A0}
#define VOLTAGE_MUX_GPIO {VOLT_MUX_A4,VOLT_MUX_A3,VOLT_MUX_A2,VOLT_MUX_A1,VOLT_MUX_A0}



#endif	//eit_sample
#ifndef eit_sample
#define eit_sample

////DATA text file
//#define VOLT_DATA_TXT "/home/debian/eit_sample/first_text.txt"

//TRUTH TABLE DIMENSIONS
#define LOGIC_ROW 32
#define LOGIC_COL 5

#define CUR_ROW 20
#define CUR_COL 10

//MUX TRUTH TABLE DECLARATION
//{A4,A3,A2,A1,A0}
int CHAN[LOGIC_ROW][LOGIC_COL] = {{0,0,0,0,0},{0,0,0,0,1},{0,0,0,1,0},{0,0,0,1,1},
                                 {0,0,1,0,0},{0,0,1,0,1},{0,0,1,1,0},{0,0,1,1,1},
                                 {0,1,0,0,0},{0,1,0,0,1},{0,1,0,1,0},{0,1,0,1,1},
                                 {0,1,1,0,0},{0,1,1,0,1},{0,1,1,1,0},{0,1,1,1,1},
                                 {1,0,0,0,0},{1,0,0,0,1},{1,0,0,1,0},{1,0,0,1,1},
                                 {1,0,1,0,0},{1,0,1,0,1},{1,0,1,1,0},{1,0,1,1,1},
                                 {1,1,0,0,0},{1,1,0,0,1},{1,1,0,1,0},{1,1,0,1,1},
                                 {1,1,1,0,0},{1,1,1,0,1},{1,1,1,1,0},{1,1,1,1,1}};

//CURRENT LOGIC TABLE 
//rows go from 100ma to 20000ma
//columns {A4,A5,A6,A7,A8,A9,A10,A11,A12,A3}
int CURRENT[CUR_ROW][CUR_COL] = {{0,0,1,0,0,0,0,0,0,0},{0,0,1,1,0,0,0,0,0,0},
				{0,0,1,1,1,0,0,0,0,0},{0,0,1,1,1,1,0,0,0,0},
				{0,0,1,1,1,1,1,0,0,0},{0,0,1,1,1,1,1,1,0,0},
		        {1,0,1,1,1,1,1,1,0,0},{1,1,1,1,1,1,1,1,0,0},
				{0,1,1,1,1,1,0,0,1,0},{1,1,1,1,1,1,0,0,1,0},
		 		{0,1,1,1,1,1,1,0,0,1},{1,1,1,1,1,1,1,0,0,1},
				{0,1,1,1,1,1,1,1,0,1},{1,1,1,1,1,1,1,1,0,1},
		   		{0,1,1,1,1,0,1,0,1,1},{1,1,1,1,1,0,1,0,1,1},
				{0,1,1,1,1,1,0,0,1,1},{1,1,1,1,1,1,0,0,1,1},
				{0,1,1,1,1,1,1,0,1,1},{1,1,1,1,1,1,1,0,1,1}};

//NUMBER OF NODES
#define NODAL_NUM 32
//NUMBER OF NODES PER SIDE
#define SIDE_LEN (NODAL_NUM/4)                                 

//FUNCTION DECLARATIONS
int cur_gnd_config(int cur_mux[],int gnd_mux[]);
int volt_samp_config(int cur_mux[],int gnd_mux[],int volt[][NODAL_NUM-2]);
//int data_file_export(int volt_data);

#define MAX_BUF 64	//max buffer length


///////////////PIN DECLARATIONS////////////
//adc pins
#define ADC_RESET_GPIO 13
#define VOLT_CHANNEL 2    //adc channel to read voltage
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
#define VOLT_MUX_A4 115  //p9_27

/////////////MUX ARRAY DECLARATIONS
#define CURRENT_MUX_GPIO {CUR_MUX_A4,CUR_MUX_A3,CUR_MUX_A2,CUR_MUX_A1,CUR_MUX_A0}
#define GROUND_MUX_GPIO {GND_MUX_A4,GND_MUX_A3,GND_MUX_A2,GND_MUX_A1,GND_MUX_A0}
#define VOLTAGE_MUX_GPIO {VOLT_MUX_A4,VOLT_MUX_A3,VOLT_MUX_A2,VOLT_MUX_A1,VOLT_MUX_A0}



#endif	//eit_sample
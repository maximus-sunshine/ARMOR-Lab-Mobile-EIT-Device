/************************************************************** 
* MAE 156B, Team 6
*
* Configuration file for EIT device
*
* 4/30/18
* 5/13/18 - Edited by Matthew to add current source pins
****************************************************************/
#ifndef EIT_CONFIG_H
#define EIT_CONFIG_H

/************************************************************** 
* DATA STORAGE
***************************************************************/

////DATA text file path
//#define VOLT_DATA_TXT "/home/debian/eit_sample/first_text.txt"

// ************************************************************* 
// * SAMPLE GEOMETRY (Must be square sample with N nodes per edge)
// **************************************************************

// //NUMBER OF NODES
// #define NODAL_NUM 8

// //NUMBER OF NODES PER SIDE
// #define SIDE_LEN (NODAL_NUM/4) 

/************************************************************** 
* GPIO PIN DECLARATIONS
***************************************************************/

//ADC PINS
#define ADC_RESET_GPIO 15 //p9_24
#define VOLT_CHANNEL 	2 //adc channel to read voltage

//CURRENT SENSE PINS
#define I_SENSE_RESET_GPIO 27	//p8_17

//CURRENT LOGIC PINS

#define CUR_SOURCE_A4  26   //p8_14
#define CUR_SOURCE_A7  23   //p8_13
#define CUR_SOURCE_A8  47   //p8_15
#define CUR_SOURCE_A11 46   //p8_16

// #define CUR_SOURCE_A4  46   //p8_6
#define CUR_SOURCE_A5  66   //p8_7
#define CUR_SOURCE_A6  68   //p8_10
// #define CUR_SOURCE_A7  34   //p8_5
// #define CUR_SOURCE_A8  38   //p8_3
#define CUR_SOURCE_A9  69   //p8_9
#define CUR_SOURCE_A10 45   //p8_11
// #define CUR_SOURCE_A11 39   //p8_4
#define CUR_SOURCE_A12 67   //p8_8
#define CUR_SOURCE_A13 44   //p8_12

//mux disable pin
#define CUR_MUX_DISABLE  79    //p8_38
#define GND_MUX_DISABLE  71    //p8_46
// #define VOLT_MUX_DISABLE 62    //p8_21
#define VOLT_MUX_DISABLE 22    //p8_19

#define MUX_DISABLE_GPIO {CUR_MUX_DISABLE, GND_MUX_DISABLE, VOLT_MUX_DISABLE}


//MUX LOGIC PINS
//power mux declaration
#define CUR_MUX_A0  8	 //p8_35
#define CUR_MUX_A1  9	 //p8_33
#define CUR_MUX_A2  10	 //p8_31
#define CUR_MUX_A3  87	 //p8_29
#define CUR_MUX_A4  86	 //p8_27
//ground mux declaraton
#define GND_MUX_A0  70	 //p8_45
#define GND_MUX_A1  72	 //p8_43
#define GND_MUX_A2  74	 //p8_41
#define GND_MUX_A3  76	 //p8_39
#define GND_MUX_A4  78	 //p8_37
//voltage sampling delcaration
#define VOLT_MUX_A0  7	 //p9_42
#define VOLT_MUX_A1  20	 //p9_41
#define VOLT_MUX_A2  14	 //p9_26
#define VOLT_MUX_A3  117 //p9_25
#define VOLT_MUX_A4  115 //p9_27
//array definitions
#define CURRENT_MUX_GPIO	{CUR_MUX_A4, CUR_MUX_A3, CUR_MUX_A2, CUR_MUX_A1, CUR_MUX_A0}
#define GROUND_MUX_GPIO		{GND_MUX_A4, GND_MUX_A3, GND_MUX_A2, GND_MUX_A1, GND_MUX_A0}
#define VOLTAGE_MUX_GPIO	{VOLT_MUX_A4,VOLT_MUX_A3,VOLT_MUX_A2,VOLT_MUX_A1,VOLT_MUX_A0}

#define CURRENT_SWITCH_GPIO     {CUR_SOURCE_A4,CUR_SOURCE_A5,CUR_SOURCE_A6,CUR_SOURCE_A7,CUR_SOURCE_A8,CUR_SOURCE_A9,CUR_SOURCE_A10,CUR_SOURCE_A11,CUR_SOURCE_A12,CUR_SOURCE_A13}
				

/************************************************************** 
* TRUTH TABLES (mux and current switching)
****************************************************************/

//TRUTH TABLE DIMENSIONS
#define LOGIC_ROW 32
#define LOGIC_COL 5

#define CUR_ROW 20
#define CUR_COL 10

//MUX TRUTH TABLE DECLARATION 
//rows:		nodes 1 to 32
//colums:	mux logic pins {A4,A3,A2,A1,A0}
int CHAN[LOGIC_ROW][LOGIC_COL] = {{0,0,0,0,0},{0,0,0,0,1},{0,0,0,1,0},{0,0,0,1,1},
								 {0,0,1,0,0},{0,0,1,0,1},{0,0,1,1,0},{0,0,1,1,1},
								 {0,1,0,0,0},{0,1,0,0,1},{0,1,0,1,0},{0,1,0,1,1},
								 {0,1,1,0,0},{0,1,1,0,1},{0,1,1,1,0},{0,1,1,1,1},
								 {1,0,0,0,0},{1,0,0,0,1},{1,0,0,1,0},{1,0,0,1,1},
								 {1,0,1,0,0},{1,0,1,0,1},{1,0,1,1,0},{1,0,1,1,1},
								 {1,1,0,0,0},{1,1,0,0,1},{1,1,0,1,0},{1,1,0,1,1},
								 {1,1,1,0,0},{1,1,1,0,1},{1,1,1,1,0},{1,1,1,1,1}};

//CURRENT SOURCE SWITCHING TRUTH TABLE
//rows:		100ua to 20000ua in 100ua steps
//columns:	logic pins {A4,A5,A6,A7,A8,A9,A10,A11,A12,A13} 
int CURRENT[CUR_ROW][CUR_COL] = {{1,0,0,0,0,0,0,0,0,0},{1,1,0,0,0,0,0,0,0,0},
								 {1,1,1,0,0,0,0,0,0,0},{1,1,1,1,0,0,0,0,0,0},
								 {1,1,1,1,1,0,0,0,0,0},{1,1,1,1,1,1,0,0,0,0},
								 {1,1,1,1,1,1,1,0,0,0},{1,1,1,1,1,1,1,1,0,0},
								 {1,1,1,1,1,0,0,0,1,0},{1,1,1,1,1,1,0,0,1,0},
								 {1,1,1,1,1,1,1,0,1,0},{1,1,1,1,1,1,1,1,1,0},
								 {1,1,1,0,0,0,1,0,1,1},{1,1,1,0,0,0,1,1,1,1},
								 {1,1,1,0,1,0,1,0,1,1},{1,1,1,1,0,0,0,0,1,1},
								 {1,1,1,1,0,0,1,0,1,1},{1,1,1,1,1,0,0,0,1,1},
								 {1,1,1,1,1,0,1,0,1,1},{1,1,1,1,1,1,0,0,1,1}};
#endif //EIT_CONFIG_H

/************************************************************** 
* MAE 156B, Team 6
*
* Library header file for project specific functions (eit.c)
*
* 4/30/18- created
* 5/13/18- edited by Matthew 
*        - added initArray and insertArray
*         -made text file a string
*6/3/18  - added data conversion func declaration
*        - removed all stuff related to ring buffer
*        - added new file paths and file pointers
****************************************************************/

#ifndef EIT_H
#define EIT_H

/************************************************************** 
* SAMPLE GEOMETRY (Must be square sample with N nodes per edge)
***************************************************************/

//NUMBER OF NODES

#define NODAL_NUM 32

//NUMBER OF NODES PER SIDE
#define SIDE_LEN (NODAL_NUM/4) 



/************************************************************** 
*DATA FILE EXPORT PATH
***************************************************************/
#define VOLT_DATA_TEXT "/home/debian/MAE156B_Team6/data/data.txt"
#define TEMP_VOLT_DATA_TEXT "/home/debian/MAE156B_Team6/data/temp_data.txt"
#define RAW_PATH "/home/debian/MAE156B_Team6/data/data"

/************************************************************** 
*FILE POINTERS
***************************************************************/
FILE* fp;
FILE* fp_temp;

/************************************************************************************
* FUNCTION DECLARATIONS
*************************************************************************************/

/****************************************************************************
* int cur_gnd_config(int cur[],int gnd[])
*
* Configures current and ground nodes in "across" switching pattern.
* Works for square samples with N nodes per edge.
*
* Inputs :	cur[], zero current node array (size = N*4)
*			gnd[], zero gnd node array     (size = N*4)
* 
* Outputs:	TODO: return -1 on failure
*****************************************************************************/
int cur_gnd_config(int cur[],int gnd[]);


/****************************************************************************
* int volt_samp_config(int cur[], int gnd[], int volt[][NODAL_NUM-2])
*
* Configures voltage sampling nodes in "across" switching pattern.
* Works for square samples with N nodes per edge.
* 
* NOTE: must run cur_gnd_config first!
*
* Inputs :	cur[],	configured current node array	(size = [N*4])
*			gnd[],	configured gnd node array		(size = [N*4])
*			volt[],	zero voltage node array			(size = [N*4][N*4-2])		
* 
* Outputs:	TODO: -return -1 on failure
*				  -add flag so this can't run unless cur_gnd_config has already been run
*				  -alternatively, just merge this into cur_gnd_config...
*****************************************************************************/

/****************************************************************************
THIS WORKS
*****************************************************************************/
// int volt_samp_config(int cur[], int gnd[], int volt[][NODAL_NUM-2]);

/****************************************************************************
THIS IS BEING TESTED
*****************************************************************************/
int volt_samp_config(int cur[], int gnd[], int volt[][NODAL_NUM]);


/****************************************************************************
* int data_conversion()
*
* Reads raw voltages from VOLT_DATA_TEXT
* Preforms voltage conversion on raw measurement and writes data to TEMP_VOLT_DATA_TEXT
* Places all voltages for one cycle on a tab seberated row
* Original raw file is removed and tempory file is renamed to original file
*
* Inputs :	

* Outputs: return -1 on failure, 0 on success
*				  
*****************************************************************************/
int data_conversion();

/****************************************************************************
* void sigint(int s __attribute__((unused)))
*
* Awaits a signal(ctrl c) and preforms cleanup duties before exiting program
* Cleans up gpio pins and adc
* Writes data to text file
* 
* Inputs :		ctrl c, user entered keyboard signal
*			
* 
* TODO: Add safety checks,improve documentation, add additional clean up steps
*****************************************************************************/
void sigint(int s __attribute__((unused)));
	

#endif //EIT_H

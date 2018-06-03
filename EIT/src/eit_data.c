/************************************************************** 
* MAE 156B, Team 6
*
* Library for project specific functions
*
* 4/30/18- created
* 5/13/18- edited by Matthew
*	 - added initArray and insertArray
* 6/3/18 - added data conversion function
*        - removed ring buffer functions
***************************************************************/

#include <stdio.h>
#include "../includes/eit.h"
#include <malloc.h>

/************************************************************************************
* FUNCTION DEFINITIONS
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
******************************************************************************/
int cur_gnd_config(int cur[],int gnd[]){
	int i;
	int node_index = 3*SIDE_LEN;
	for(i=0; i < NODAL_NUM; i++){
		cur[i] = i;
		gnd[i] = node_index - 1;
		node_index = node_index - 1;
		if((node_index % (SIDE_LEN))==0){
			node_index = node_index + (NODAL_NUM/2);
			if (node_index > NODAL_NUM){
				node_index = node_index % NODAL_NUM;
			}
		}
	}
	return 0;
}

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
// int volt_samp_config(int cur[], int gnd[], int volt[][NODAL_NUM-2]){
// 	int k = 0;
// 	int i,j;
// 	for(i = 0; i < NODAL_NUM; i++){
// 		for(j = 0; j < NODAL_NUM; j++){
// 			if((i != j) && (gnd[i] != cur[j])){
// 				volt[i][k] = cur[j];
// 				k++;
// 			}
// 		}
// 		k = 0;
// 	}
// 	return 0;
// }

/****************************************************************************
THIS IS BEING TESTED
*****************************************************************************/
int volt_samp_config(int cur[], int gnd[], int volt[][NODAL_NUM]){
	int k = 0;
	int i,j;
	for(i = 0; i < NODAL_NUM; i++){
		for(j = 0; j < NODAL_NUM; j++){
				volt[i][k] = cur[j];
				k++;
		}
		k = 0;
	}
	return 0;
}



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
int data_conversion(){
	float volt_value;
	char data_buff[8];
	int index = 0;

	
	fp = fopen(VOLT_DATA_TEXT,"r");
	if(NULL == fp) {
        perror("ERROR in opening raw data file\n");
		return -1;
    }
	fp_temp = fopen(TEMP_VOLT_DATA_TEXT,"w");
	if(NULL == fp_temp) {
        perror("ERROR in creating text file for converted/formatted data\n");
		return -1;
    }

	while(fgets(data_buff,8,fp)!= NULL){
		volt_value = atoi(data_buff)*(scale/1000);

		if(index == (NODAL_NUM-1)){ 	
			fprintf(fp_temp,"%.9f\n",volt_value);
			index = 0;
		}
		else{
			fprintf(fp_temp,"%.9f\t",volt_value);
			index++;
		}

	}
	
	fclose(fp);
	fclose(fp_temp);
	
	remove(VOLT_DATA_TEXT);
	rename(TEMP_VOLT_DATA_TEXT,VOLT_DATA_TEXT);

	return 0;
}

/************************************************************** 
* MAE 156B, Team 6
*
* Library for project specific functions
*
* 4/30/18- created
* 5/13/18- edited by Matthew
*	 - added initArray and insertArray
***************************************************************/

#include <stdio.h>
#include "eit.h"

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
*****************************************************************************/
int cur_gnd_config(int cur[],int gnd[]){
	int i;
	int node_index = 3*SIDE_LEN;
	for(i=0; i <= NODAL_NUM-1; i++){
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
int volt_samp_config(int cur[], int gnd[], int volt[][NODAL_NUM-2]){
	int k = 0;
	int i,j;
	for(i = 0; i < NODAL_NUM; i++){
		for(j = 0; j < NODAL_NUM; j++){
			if((i != j) && (gnd[i] != cur[j])){
				volt[i][k] = cur[j];
				k++;
			}
		}
		k = 0;
	}
	return 0;
}



/****************************************************************************
* void initArray(Array *a, size_t initialSize)
*
* Initializes a dynamic array in heap memory through malloc
* Initializes used elements to zero and ininital array size according to intialSize
*
* Inputs :	Array *a, a pointer to stuct Array
*			initialSize, initialze size of array
* 
* TODO: Add safety checks
*****************************************************************************/
void initArray(Array *a, size_t initialSize){
	a->array = (int *)malloc(initialSize * sizeof(int));
	a->used = 0;
	a->size = initialSize;
}

/****************************************************************************
* void insertArray(Array *a, int element)
*
* Inserts elements into array and updates # of elements used in array
* If the array fills up, it doubles in size through realloc
*
* Inputs :	Array *a, a pointer to stuct Array
*			element, an integer that will be added to array
* 
* TODO: Add safety checks
*****************************************************************************/
void insertArray(Array *a, int element){
	
	if(a->used == a->size){
		a->size *= 2;
		a->array = (int *)realloc(a->array, a->size * sizeof(int));
	}
	a->array[a->used] = element;	
	a->used++;
}

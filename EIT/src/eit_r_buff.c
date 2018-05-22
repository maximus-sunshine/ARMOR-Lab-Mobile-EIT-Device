/************************************************************** 
* MAE 156B, Team 6
*
* Library for project specific functions
*
* 4/30/18- created
* 5/22/18- edited by Matthew
*	 - added ring buffer functionality
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
* void initArray(RING_BUFFER *ring_buffer, size_t initial_Size)
*
* Initializes a ring buffer in heap memory through malloc
* Initializes index and read index to 0 and sets size of array to initial_Size
*
* Inputs :	RING_BUFFER *ring_buffer
*			size_t initial_Size
* 
* TODO: Add safety checks
*****************************************************************************/
void initArray(RING_BUFFER *ring_buffer, size_t initial_Size){
	ring_buffer->buffer = (int *)malloc(initial_Size * sizeof(int));
	ring_buffer->initialSize = initial_Size;
	ring_buffer->index = 0;
	ring_buffer->read_index = 0;
}

/****************************************************************************
* void insertArray(RING_BUFFER *ring_buffer, int element)
*
* Inserts elements into ring buffer
* increments index, sets it back to 0 when it surpasses the initial size
*
* Inputs :	RING_BUFFER *ring_buffer
*			element, an integer that will be added to array
* 
* TODO: Add safety checks
*****************************************************************************/
void insertArray(RING_BUFFER *ring_buffer, int element){
	
	if(ring_buffer->index >= ring_buffer->initialSize){sddfsdf
		ring_buffer->index = 0;
	}
	ring_buffer->buffer[ring_buffer->index] = element;
	ring_buffer->index++;
}

/****************************************************************************
* int readArray(RING_BUFFER *ring_buffer)
*
* returns integer values in ring_buffer
* increments read index, sets it back to 0 when it surpasses the initial size
*
* Inputs :	RING_BUFFER *ring_buffer
*			
* 
* TODO: Add safety checks
*****************************************************************************/
int readArray(RING_BUFFER *ring_buffer){
	int value;
	if(ring_buffer->read_index >= ring_buffer->initialSize){
		ring_buffer->read_index = 0;
	}
 	value = ring_buffer->buffer[ring_buffer->read_index];
	ring_buffer->read_index++;
	return value;
}

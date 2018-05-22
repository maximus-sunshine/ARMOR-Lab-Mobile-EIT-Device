/************************************************************** 
* MAE 156B, Team 6
*
* Library header file for project specific functions (eit.c)
*
* 4/30/18- created
* 5/22/18- edited by Matthew 
*        - added ring buffer fucntion and struct declarations
*         
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
*ARRAY STRUCTURE FOR RING BUFFER
****************************************************************/
typedef struct {
	
	size_t index;           //index in buffer where elements are added currently
	size_t initialSize;     //size of ring buffer
	size_t read_index;      //index in buffer where elements are being read currently
	int *buffer;            // pointer array
} 	RING_BUFFER;          //name of struct

/************************************************************** 
*DATA FILE EXPORT PATH
***************************************************************/
#define VOLT_DATA_TEXT "/home/debian/MAE156B_Team6/data/data.txt"


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
int volt_samp_config(int cur[], int gnd[], int volt[][NODAL_NUM-2]);

	
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
void initArray(RING_BUFFER *buffer, size_t initial_Size);
	

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
void insertArray(RING_BUFFER *buffer, int element);
 
 
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
int readArray(RING_BUFFER *buffer);


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

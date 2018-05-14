/************************************************************** 
* MAE 156B, Team 6
*
* Library header file for project specific functions (eit.c)
*
* 4/30/18- created
* 5/13/18- edited by Matthew 
*        - added initArray and insertArray
*         -made text file a string
****************************************************************/

#ifndef EIT_H
#define EIT_H

/************************************************************** 
* SAMPLE GEOMETRY (Must be square sample with N nodes per edge)
***************************************************************/

//NUMBER OF NODES

#define NODAL_NUM 8

//NUMBER OF NODES PER SIDE
#define SIDE_LEN (NODAL_NUM/4) 


/************************************************************** 
*ARRAY STRUCTURE FOR BUFFER
****************************************************************/
typedef struct {
	
	size_t used; //# of elements that have been put into array
	size_t size; //size of array
	int *array; // pointer array
} Array; //name of struct

/************************************************************** 
*DATA FILE EXPORT PATH
***************************************************************/
#define VOLT_DATA_TEXT "/home/debian/MAE156B_Team6/Scripts/text.txt"


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
void initArray(Array *a, size_t initialSize);
	

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
void insertArray(Array *a, int element);

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

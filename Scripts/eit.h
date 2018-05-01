/************************************************************** 
* MAE 156B, Team 6
*
* Library header file for project specific functions (eit.c)
*
* 4/30/18
***************************************************************/

#ifndef EIT_H
#define EIT_H

/************************************************************** 
* SAMPLE GEOMETRY (Must be square sample with N nodes per edge)
***************************************************************/

//NUMBER OF NODES
#define NODAL_NUM 32

//NUMBER OF NODES PER SIDE
#define SIDE_LEN (NODAL_NUM/4) 

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

#endif //EIT_H

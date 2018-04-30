#include <stdio.h>
#include "eit_sample.h"

//CONFIGURES CURRENT AND GROUND NODES ACCORDING TO # OF NODES(NODE_NUM)
int cur_gnd_config(int cur[],int gnd[]){
	int i;
	int node_index = 3*SIDE_LEN;
	for(i=0; i <= NODAL_NUM-1; i++){
		cur[i] = i+1; //current starts at first node and increments to end
		gnd[i] = node_index; //ground starts at last node of third side
		node_index = node_index -1; //ground moves cc
		if((node_index % (SIDE_LEN))==0){  //once it passes an edge node it adds half the # of nodes to index
			node_index = node_index + (NODAL_NUM/2);
			if (node_index > NODAL_NUM){   //if index end up being greater then NODAL_NUM it takes the remainder 
				node_index = node_index % NODAL_NUM;
			}
		}
	}
	return 0;
}

//CONFIGURES VOLTAGE SAMPLING NODES ACCORDING TO # OF NODES(NODE_NUM)
int volt_samp_config(int cur[], int gnd[], int volt[]){
	int k = 0;
	int i,j;
	for(i = 0; i < NODAL_NUM; i++){
		for(j = 0; j < NODAL_NUM; j++){
			if((i != j) && (gnd[i] != cur[j])){ //skips current and ground node
				volt[i][k] = cur[j];
				k++;
			}
		}
		k = 0;
	}
	return 0;
}

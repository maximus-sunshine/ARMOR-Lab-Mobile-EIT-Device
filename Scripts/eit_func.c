#include <stdio.h>
#include "eit_sample.h"

int cur_gnd_config(int cur[],int gnd[]){
	int i;
	int node_index = 3*SIDE_LEN;
	for(i=0; i <= NODAL_NUM-1; i++){
		cur[i] = i+1;
		gnd[i] = node_index;
		node_index = node_index -1;
		if((node_index % (SIDE_LEN))==0){
			node_index = node_index + (NODAL_NUM/2);
			if (node_index > NODAL_NUM){
				node_index = node_index % NODAL_NUM;
			}
		}
	}
	return 0;
}

int volt_samp_config(int cur[], int gnd[], int volt[]){
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
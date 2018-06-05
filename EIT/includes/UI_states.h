#ifndef UI_STATES_H
#define UI_STATES_H

#include "UI.h"

/* Oh Compiler-Please leave me as is */
volatile unsigned char flag = 0;

char MENU_OPTS[6][OPT_STR_LEN] = {"HOME","SETTINGS","NODES","CURRENT","CONFIG","MODE"};
char HOME_OPTS[2][OPT_STR_LEN] = {"START","SETTINGS"};
char SETTINGS_OPTS[4][OPT_STR_LEN] = {"NODES","CURRENT","CONFIG","MODE"};
char NODES_OPTS[7][OPT_STR_LEN] = {"8","12","16","20","24","28","32"};
char CURRENT_OPTS[20][OPT_STR_LEN] = {"100","200","300","400","500","600","700","800","900","1000","1100","1200","1300","1400","1500","1600","1700","1800","1900","2000"};
char CONFIG_OPTS[2][OPT_STR_LEN] = {"ACROSS","ADJACENT"};
char SAMPLING_OPTS[3][OPT_STR_LEN] = {"TIMED","CYCLES","CONTINUOUS"};

#endif //UI_STATES_H
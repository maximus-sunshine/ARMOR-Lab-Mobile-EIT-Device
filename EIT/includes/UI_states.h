#ifndef UI_STATES_H
#define UI_STATES_H

#include "UI.h"

/* Oh Compiler-Please leave me as is */
volatile unsigned char flag = 0;

UI_state_t UI_start = {
    .menu_main = "START",
    .menu_prev = "",
    .menu_next = "SETTINGS",
    .menu_back = "HOME",
    .button_select = START,
    .button_prev = SETTINGS,
    .button_next = SETTINGS,
    .button_back = START,
};

UI_state_t UI_settings = {
    .menu_main = "SETTINGS",
    .menu_prev = "START",
    .menu_next = "",
    .menu_back = "HOME",
    .button_select = NODES,
    .button_prev = START,
    .button_next = START,
    .button_back = START, 
};

UI_state_t UI_nodes = {
    .menu_main = "NODES",
    .menu_prev = "",
    .menu_next = "CURRENT",
    .menu_back = "SETTINGS",
    .button_select = NUM_NODES8,
    .button_prev = NODES,
    .button_next = CURRENT,
    .button_back = SETTINGS,  
};

UI_state_t UI_nodes8 = {
    .menu_main = "8",
    .menu_prev = "",
    .menu_next = "16",
    .menu_back = "NODES",
    .button_select = START,
    .button_prev = NUM_NODES32,
    .button_next = NUM_NODES16,
    .button_back = NODES,
};

UI_state_t UI_nodes16 = {
    .menu_main = "16",
    .menu_prev = "8",
    .menu_next = "32",
    .menu_back = "NODES",
    .button_select = START,
    .button_prev = NUM_NODES16,
    .button_next = NUM_NODES32,
    .button_back = NODES, 
};

UI_state_t UI_nodes32 = {
    .menu_main = "32",
    .menu_prev = "16",
    .menu_next = "8",
    .menu_back = "NODES",
    .button_select = START,
    .button_prev = NUM_NODES16,
    .button_next = NUM_NODES8,
    .button_back = NODES,   
};

UI_state_t UI_current = {
    .menu_main = "CURRENT",
    .menu_prev = "NODES",
    .menu_next = "CONFIG",
    .menu_back = "SETTINGS",
    .button_select = CURRENT_AUTO, 
    .button_prev = NODES,
    .button_next = CONFIG,
    .button_back = SETTINGS,    
};

UI_state_t UI_current_auto = {
    .menu_main = "AUTO",
    .menu_prev = "",
    .menu_next = "MANUAL",
    .menu_back = "CURRENT",
    .button_select = START,
    .button_prev = CURRENT_MANUAL,
    .button_next = CURRENT_MANUAL,
    .button_back = SETTINGS,  
};

UI_state_t UI_current_manual = {
    .menu_main = "MANUAL",
    .menu_prev = "AUTO",
    .menu_next = "",
    .menu_back = "CURRENT",
    .button_select = START, 
    .button_prev = CURRENT_AUTO,
    .button_next = CURRENT_AUTO,
    .button_back = SETTINGS,   
};

UI_state_t UI_current_100;
UI_state_t UI_current_200;
UI_state_t UI_current_300;
UI_state_t UI_current_400;
UI_state_t UI_current_500;
UI_state_t UI_current_600;
UI_state_t UI_current_700;
UI_state_t UI_current_800;
UI_state_t UI_current_900;
UI_state_t UI_current_1000;
UI_state_t UI_current_1100;
UI_state_t UI_current_1200;
UI_state_t UI_current_1300;
UI_state_t UI_current_1400;
UI_state_t UI_current_1500;
UI_state_t UI_current_1600;
UI_state_t UI_current_1700;
UI_state_t UI_current_1800;
UI_state_t UI_current_1900;
UI_state_t UI_current_2000;

UI_state_t UI_config = {
    .menu_main = "CONFIG",
    .menu_prev = "CURRENT",
    .menu_next = "SAMPLING",
    .menu_back = "SETTINGS",
    .button_select = START,
    .button_prev = CURRENT,
    .button_next = CONFIG,
    .button_back = SETTINGS,    
};

#endif //UI_STATES_H
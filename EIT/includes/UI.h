/*****************************************************************
 * ---------------------------------------------------------------
 * ARMOR Lab @UC San Diego, Kenneth Loh Ph.D
 * 
 * MAE 156B Spring 2018 Team 6: Warfighter Protection
 * 	- Maxwell Sun		(maxsun96@gmail.com)
 *	- Jacob Rutheiser	(jrutheiser@gmail.com)
 *	- Matthew Williams	(mwilliams31243@gmail.com)
 *	- Aaron Gunn		(gunnahg@gmail.com)
 * ---------------------------------------------------------------
 * 
 * UI.h
 * 
 * Header file for the EIT device's user interface
 ********************************************************************/
#ifndef UI_H
#define UI_H

/*******************************************************************************
* INCLUDES/EXTERNS
*******************************************************************************/
/* Lib Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>

/* Header Files */
#include "I2C.h"
#include "SSD1306_OLED.h"
#include "example_app.h"
#include "gpiolib.h"

/* Externs - I2C.c */
extern I2C_DeviceT I2C_DEV_2;

/*******************************************************************************
* ENUMS/DEFINES/VARIABLES
*******************************************************************************/
#define SELECT 0 //TODO: change to enum
#define PREV 1
#define NEXT 2
#define BACK 3

#define START 0	//TODO: change to enum
#define SETTINGS 1
#define NODES 2
#define NUM_NODES8 3
#define NUM_NODES16 4
#define NUM_NODES32 5
#define CURRENT 6
#define CURRENT_AUTO 7
#define CURRENT_MANUAL 8
#define CONFIG 9

#define RUNNING 1 //TODO: change to enum
#define STOPPED 0
#define PAUSED 0

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

/************************************************************************************
* STRUCTS
*************************************************************************************/
typedef struct UI_state_t{
    const unsigned char *menu_main;
    const unsigned char *menu_prev;
    const unsigned char *menu_next;
    const unsigned char *menu_back;
    int button_select;
    int button_prev;
    int button_next;
    int button_back;
} UI_state_t;

typedef struct state_t{
    float batt;
    int system;
} state_t;

/************************************************************************************
* FUNCTION DECLARATIONS
*************************************************************************************/

/* OLED FUNCTIONS */
/****************************************************************************
* void printCenter(const unsigned char *text, int size)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printCenter(const unsigned char *text, int size);

/****************************************************************************
* void printCenterX(const unsigned char *text, int y1, int size)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printCenterX(const unsigned char *text, int y1, int size);

/****************************************************************************
* void printCenterY(const unsigned char *text, int x1, int size)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printCenterY(const unsigned char *text, int x1, int size);

/****************************************************************************
* void printRight(const unsigned char *text, int y1, int size)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printRight(const unsigned char *text, int y1, int size);

/****************************************************************************
* void printLeft(const unsigned char *text, int y1, int size)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printLeft(const unsigned char *text, int y1, int size);

/****************************************************************************
* void printMenuCenterSelect(const unsigned char *text, int size)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printMenuCenterSelect(const unsigned char *text, int size);

/****************************************************************************
* void prevSelect()
* 
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void prevSelect();

/****************************************************************************
* void nextSelect()
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void nextSelect();

/****************************************************************************
* void backSelect()
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void backSelect();

/****************************************************************************
* void printBattery(float batt)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printBattery(float batt);

/****************************************************************************
* void printUI(UI_state_t UI_state, state_t state)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printUI(UI_state_t UI_state, state_t state);

/****************************************************************************
* void mainSelect(UI_state_t UI_state)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void mainSelect(UI_state_t UI_state);

/****************************************************************************
* int process_button(UI_state_t UI_state, int button, int menu)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int process_button(UI_state_t UI_state, int button, int menu);

/* USER BUTTON FUNCTIONS */
/****************************************************************************
* int gpio_export(unsigned int gpio)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int gpio_export(unsigned int gpio);

/****************************************************************************
* int gpio_unexport(unsigned int gpio)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int gpio_unexport(unsigned int gpio);

/****************************************************************************
* vint gpio_set_dir(unsigned int gpio, unsigned int out_flag)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);

/****************************************************************************
* int gpio_set_value(unsigned int gpio, unsigned int value)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value);

/****************************************************************************
* int gpio_get_value(unsigned int gpio, unsigned int *value)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value);

/****************************************************************************
* int gpio_set_edge(unsigned int gpio, char *edge)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int gpio_set_edge(unsigned int gpio, char *edge);

/****************************************************************************
* int gpio_fd_open(unsigned int gpio)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int gpio_fd_open(unsigned int gpio);

/****************************************************************************
* int gpio_fd_close(int fd)
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
int gpio_fd_close(int fd);

#endif //UI_H
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
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define OPT_STR_LEN 11   
#define MAX_BUF 64
#define mod(a,b) (a%b+b)%b

/************************************************************************************
* STRUCTS
*************************************************************************************/

typedef struct state_t{
    int menu;
    int back;
    int index;
    int prev_index;
    int len; 
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
* void printUI(state_t state, char opt_list[][OPT_STR_LEN])
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void printUI(state_t state, const char opt_list[][OPT_STR_LEN]);

/****************************************************************************
* void print_sample_screen()
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void print_sample_screen();

/****************************************************************************
* void mainSelect(state_t state, char opt_list[][OPT_STR_LEN])
*
* DESCRIPTION
*
* Inputs : 
* 
* Outputs: 
*****************************************************************************/
void mainSelect(state_t state, const char opt_list[][OPT_STR_LEN]);

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
/*
 * Main.c
 *
 *  Created on  : Sep 6, 2017
 *  Author      : Vinay Divakar
 *  Description : Example usage of the SSD1306 Driver API's
 *  Website     : www.deeplyembedded.org
 *
 *  compile with "gcc UI_test.c example_app.c I2C.c SSD1306_OLED.c -o UI_test"
 *
 */

/* Lib Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>


/* Header Files */
#include "I2C.h"
#include "SSD1306_OLED.h"
#include "example_app.h"

/* Externs - I2C.c */
extern I2C_DeviceT I2C_DEV_2;

/* Oh Compiler-Please leave me as is */
volatile unsigned char flag = 0;




/*******************************************************************************
* state_t
*
* 
*******************************************************************************/

typedef struct state_t{
    float batt;                               // Left wheel current position (rad)
    const unsigned char *menu_main;
    const unsigned char *menu_prev;
    const unsigned char *menu_next;
    const unsigned char *menu_back;
    int system;           // Right wheel current position (rad)
} state_t;

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define DEBOUNCE 0*1e6       // 1 seconds
#define MAX_BUF 64


#define SELECT 0
#define PREV 1
#define NEXT 2
#define BACK 3

#define START 0
#define SETTINGS 1
#define NODES 2
#define NUM_NODES8 3
#define NUM_NODES16 4
#define NUM_NODES32 5
#define CURRENT 6
#define CURRENT_AUTO 7
#define CURRENT_MANUAL 8
#define CONFIG 9

#define RUNNING 1
#define STOPPED 0


state_t state;
int button;
int menu;



void printCenter(const unsigned char *text, int size)
{
    int charcount = strlen(text);
    int pad = 5;

    int x1 = (SSD1306_LCDWIDTH - 6*size*charcount)/2 + 1;
    int x2 = (SSD1306_LCDWIDTH + 6*size*charcount)/2 - 1;

    int y1 = (SSD1306_LCDHEIGHT - 8*size)/2 + 1;
    int y2 = (SSD1306_LCDHEIGHT + 8*size)/2 - 1;

    fillRect(12,y1 - pad,SSD1306_LCDWIDTH - 12*2,(y2 - y1) + 2*pad,BLACK);
    setTextSize(size);
    setTextColor(WHITE);
    setCursor(x1,y1);
    print_str(text);
}


void printCenterX(const unsigned char *text, int y1, int size)
{
    int charcount = strlen(text);

    int x1 = (SSD1306_LCDWIDTH - 6*size*charcount)/2 + 1;

    setTextSize(size);
    setTextColor(WHITE);
    setCursor(x1,y1);
    print_str(text);
}

void printCenterY(const unsigned char *text, int x1, int size)
{
    int charcount = strlen(text);

    int y1 = (SSD1306_LCDHEIGHT - 8*size)/2 + 1;

    setTextSize(size);
    setTextColor(WHITE);
    setCursor(x1,y1);
    print_str(text);
}

void printRight(const unsigned char *text, int y1, int size)
{
    int charcount = strlen(text);
    int x1 = SSD1306_LCDWIDTH - 6*size*charcount;

    setTextSize(size);
    setTextColor(WHITE);
    setCursor(x1,y1);
    print_str(text);
}

void printLeft(const unsigned char *text, int y1, int size)
{
    int charcount = strlen(text);
    int x1 = 1;

    setTextSize(size);
    setTextColor(WHITE);
    setCursor(x1,y1);
    print_str(text);
}


void printMenuCenterSelect(const unsigned char *text, int size)
{
    int charcount = strlen(text);
    int pad = 5;

    int x1 = (SSD1306_LCDWIDTH - 6*size*charcount)/2 + 1;
    int x2 = (SSD1306_LCDWIDTH + 6*size*charcount)/2 - 1;

    int y1 = (SSD1306_LCDHEIGHT - 8*size)/2 + 1;
    int y2 = (SSD1306_LCDHEIGHT + 8*size)/2 - 1;

    fillRect(12,y1 - pad,SSD1306_LCDWIDTH - 12*2,(y2 - y1) + 2*pad,BLACK);
    fillRect(x1 - pad,y1 - pad,(x2 - x1) + 2*pad,(y2 - y1) + 2*pad,WHITE);
    setTextSize(size);
    setTextColor(INVERSE);
    setCursor(x1,y1);
    print_str(text);
}

void prevSelect()
{
    int pad = 2;
    int size = 2;

    int x1 = 1;
    int x2 = 12;

    int y1 = (SSD1306_LCDHEIGHT - 8*size)/2 + 1;
    int y2 = (SSD1306_LCDHEIGHT + 8*size)/2 - 1;

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,BLACK);

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,WHITE);

    setTextSize(size);
    setTextColor(INVERSE);
    setCursor(x1,y1);
    print_str("<");
    Display();
    usleep(0.1*1e6);

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,BLACK);
    setTextSize(size);
    setTextColor(WHITE);
    setCursor(x1,y1);
    print_str("<");
    Display();
}

void nextSelect()
{
    int pad = 2;
    int size = 2;

    int x1 = SSD1306_LCDWIDTH-12-1;
    int x2 = SSD1306_LCDWIDTH;

    int y1 = (SSD1306_LCDHEIGHT - 8*size)/2 + 1;
    int y2 = (SSD1306_LCDHEIGHT + 8*size)/2 - 1;

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,BLACK);

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,WHITE);

    setTextSize(size);
    setTextColor(INVERSE);
    setCursor(x1,y1);
    print_str(">");
    Display();
    usleep(0.1*1e6);

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,BLACK);
    setTextSize(size);
    setTextColor(WHITE);
    setCursor(x1,y1);
    print_str(">");
    Display();
}

void backSelect()
{
    int pad = 2;
    int size = 2;

    int x1 = SSD1306_LCDWIDTH-12;
    int x2 = SSD1306_LCDWIDTH;

    int y1 = (SSD1306_LCDHEIGHT - 8*size)/2 + 1;
    int y2 = (SSD1306_LCDHEIGHT + 8*size)/2 - 1;

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,BLACK);

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,WHITE);

    setTextSize(size);
    setTextColor(INVERSE);
    setCursor(x1,y1);
    print_str(">");
    Display();
    usleep(0.1*1e6);

    fillRect(x1,y1-pad,12,(y2 - y1) + 2*pad,BLACK);
    setTextSize(size);
    setTextColor(WHITE);
    setCursor(x1,y1);
    print_str(">");
    Display();
}

void printBattery(float batt)
{   
    setTextSize(1);
    setTextColor(WHITE);

    if(batt >= 100){
        int charcount = 6;
        int x1 = SSD1306_LCDWIDTH - 6*charcount - 1;
        setCursor(x1,1);
        printFloat(batt,1);
        print_str("%"); 
    }
    else if(batt >= 10 && batt < 100){
        int charcount = 5;
        int x1 = SSD1306_LCDWIDTH - 6*charcount - 1;
        setCursor(x1,1);
        printFloat(batt,1);
        print_str("%"); 
    }
    else{
        int charcount = 4;
        int x1 = SSD1306_LCDWIDTH - 6*charcount - 1;
        setCursor(x1,1);
        printFloat(batt,1);
        print_str("%"); 
    }
}

void printUI(){
    // printBattery(batt);
    printCenter(state.menu_main,2);
    printLeft(state.menu_back,1,1);
    printBattery(state.batt);
    printLeft(state.menu_prev,SSD1306_LCDHEIGHT-9,1);
    printRight(state.menu_next,SSD1306_LCDHEIGHT-9,1);
    printCenterY("<",1,2);
    printCenterY(">",SSD1306_LCDWIDTH-12-1,2);
}

void mainSelect(){
    printMenuCenterSelect(state.menu_main,2);
    Display();
    usleep(0.1*1e6);

    printCenter(state.menu_main,2);
    Display();
}

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio)
{
        int fd, len;
        char buf[MAX_BUF];
 
        fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
        if (fd < 0) {
                perror("gpio/export");
                return fd;
        }
 
        len = snprintf(buf, sizeof(buf), "%d", gpio);
        write(fd, buf, len);
        close(fd);
 
        return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio)
{
        int fd, len;
        char buf[MAX_BUF];
 
        fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
        if (fd < 0) {
                perror("gpio/export");
                return fd;
        }
 
        len = snprintf(buf, sizeof(buf), "%d", gpio);
        write(fd, buf, len);
        close(fd);
        return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
        int fd, len;
        char buf[MAX_BUF];
 
        len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
        fd = open(buf, O_WRONLY);
        if (fd < 0) {
                perror("gpio/direction");
                return fd;
        }
 
        if (out_flag)
                write(fd, "out", 4);
        else
                write(fd, "in", 3);
 
        close(fd);
        return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value)
{
        int fd, len;
        char buf[MAX_BUF];
 
        len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
        fd = open(buf, O_WRONLY);
        if (fd < 0) {
                perror("gpio/set-value");
                return fd;
        }
 
        if (value)
                write(fd, "1", 2);
        else
                write(fd, "0", 2);
 
        close(fd);
        return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value)
{
        int fd, len;
        char buf[MAX_BUF];
        char ch;

        len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
        fd = open(buf, O_RDONLY);
        if (fd < 0) {
                perror("gpio/get-value");
                return fd;
        }
 
        read(fd, &ch, 1);

        if (ch != '0') {
                *value = 1;
        } else {
                *value = 0;
        }
 
        close(fd);
        return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
        int fd, len;
        char buf[MAX_BUF];

        len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
 
        fd = open(buf, O_WRONLY);
        if (fd < 0) {
                perror("gpio/set-edge");
                return fd;
        }
 
        write(fd, edge, strlen(edge) + 1); 
        close(fd);
        return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
        int fd, len;
        char buf[MAX_BUF];

        len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
        fd = open(buf, O_RDONLY | O_NONBLOCK );
        if (fd < 0) {
                perror("gpio/fd_open");
        }
        return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
        return close(fd);
}


void* select_poll(void* ptr){
    struct pollfd fdset[1];
    int nfds = 1;
    int gpio_fd, timeout, rc;
    char *buf[MAX_BUF];
    unsigned int gpio;
    int len;
    int n = 0;
    double debounce = DEBOUNCE;

    gpio = 61;

    gpio_export(gpio);
    gpio_set_dir(gpio, 0);
    gpio_set_edge(gpio, "rising");
    gpio_fd = gpio_fd_open(gpio);

    timeout = POLL_TIMEOUT;

    while(state.system){
        memset((void*)fdset, 0, sizeof(fdset));
        fdset[0].fd = gpio_fd;
        fdset[0].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);

        if (rc < 0) {
            if (errno == EINTR) {
                printf("\nInterrupted system call... continuing\n");
                continue;
            }
            perror("\npoll() failed!\n");
            return NULL;
        }

        if (rc == 0) {
            printf(".");
        }

        if (fdset[0].revents & POLLPRI) {
            lseek(fdset[0].fd, 0, SEEK_SET);
            len = read(fdset[0].fd, buf, MAX_BUF);
            printf("\npoll() GPIO %d interrupt occurred %d times!\n", gpio, n);
             button = SELECT;
            if (n % 2 == 0){
                
            }
            n++;
        }
        usleep(debounce);
        fflush(stdout);
    }
    gpio_unexport(gpio);
    gpio_fd_close(gpio_fd);
    return NULL;
}

void* prev_poll(void* ptr){
    struct pollfd fdset[1];
    int nfds = 1;
    int gpio_fd, timeout, rc;
    char *buf[MAX_BUF];
    unsigned int gpio;
    int len;
    int n = 0;
    double debounce = DEBOUNCE;

    gpio = 88;

    gpio_export(gpio);
    gpio_set_dir(gpio, 0);
    gpio_set_edge(gpio, "rising");
    gpio_fd = gpio_fd_open(gpio);

    timeout = POLL_TIMEOUT;

    while(state.system){
        memset((void*)fdset, 0, sizeof(fdset));
        fdset[0].fd = gpio_fd;
        fdset[0].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);

        if (rc < 0) {
            if (errno == EINTR) {
                printf("\nInterrupted system call... continuing\n");
                continue;
            }
            perror("\npoll() failed!\n");
            return NULL;
        }

        if (rc == 0) {
            printf(".");
        }

        if (fdset[0].revents & POLLPRI) {
            lseek(fdset[0].fd, 0, SEEK_SET);
            len = read(fdset[0].fd, buf, MAX_BUF);
            printf("\npoll() GPIO %d interrupt occurred %d times!\n", gpio, n);
             button = PREV;
            if (n % 2 == 0){
                
            }
            n++;
        }
        usleep(debounce);
        fflush(stdout);
    }
    gpio_unexport(gpio);
    gpio_fd_close(gpio_fd);
    return NULL;
}

void* next_poll(void* ptr){
    struct pollfd fdset[1];
    int nfds = 1;
    int gpio_fd, timeout, rc;
    char *buf[MAX_BUF];
    unsigned int gpio;
    int len;
    int n = 0;
    double debounce = DEBOUNCE;

    gpio = 89;

    gpio_export(gpio);
    gpio_set_dir(gpio, 0);
    gpio_set_edge(gpio, "rising");
    gpio_fd = gpio_fd_open(gpio);

    timeout = POLL_TIMEOUT;

    while(state.system){
        memset((void*)fdset, 0, sizeof(fdset));
        fdset[0].fd = gpio_fd;
        fdset[0].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);

        if (rc < 0) {
            if (errno == EINTR) {
                printf("\nInterrupted system call... continuing\n");
                continue;
            }
            perror("\npoll() failed!\n");
            return NULL;
        }

        if (rc == 0) {
            printf(".");
        }

        if (fdset[0].revents & POLLPRI) {
            lseek(fdset[0].fd, 0, SEEK_SET);
            len = read(fdset[0].fd, buf, MAX_BUF);
            printf("\npoll() GPIO %d interrupt occurred %d times!\n", gpio, n);
             button = NEXT;
            if (n % 2 == 0){
                
            }
            n++;
        }
        usleep(debounce);
        fflush(stdout);
    }
    gpio_unexport(gpio);
    gpio_fd_close(gpio_fd);
    return NULL;
}




int main()
{

    /* Initialize I2C bus and connect to the I2C Device */
    if(init_i2c_dev2(SSD1306_OLED_ADDR) == 0)
    {
        printf("(Main)i2c-2: Bus Connected to SSD1306\r\n");
    }
    else
    {
        printf("(Main)i2c-2: OOPS! Something Went Wrong\r\n");
        exit(1);
    }

    /* Register the Alarm Handler */


    /* Run SDD1306 Initialization Sequence */
    display_Init_seq();
    state.batt = 69.0;
    state.system = RUNNING;

    pthread_t  select_thread;
    pthread_create(&select_thread, NULL, select_poll, (void*) NULL);


    pthread_t  next_thread;
    pthread_create(&next_thread, NULL, next_poll, (void*) NULL);

    pthread_t  prev_thread;
    pthread_create(&prev_thread, NULL, prev_poll, (void*) NULL);
    // pthread_t  next_thread;
    // pthread_create(&next_thread, NULL, next_poll, (void*) NULL);

    // pthread_t  prev_thread;
    // pthread_create(&prev_thread, NULL, prev_poll, (void*) NULL);



    while(state.system){
        switch(menu) {
            // LEVEL 1
            case START:
            clearDisplay();
            state.menu_main = "START";
            state.menu_prev = "";
            state.menu_next = "SETTINGS";
            state.menu_back = "HOME";
            printUI();
            Display();

            switch(button) {
                case SELECT:
                    mainSelect();
                    button = -1;
                    break;
                case PREV:
                    prevSelect();
                    button = -1;
                    break;
                case NEXT:
                    nextSelect();
                    menu = SETTINGS;
                    button = -1;
                    break;
                case BACK:
                    button = -1;
                    break;
            }
            break;

            case SETTINGS:
                
                clearDisplay();
                state.menu_main = "SETTINGS";
                state.menu_prev = "START";
                state.menu_next = "";
                state.menu_back = "HOME";
                printUI();
                Display();

                switch(button) {
                    case SELECT:
                        mainSelect();
                        menu = NODES;
                        button = -1;
                        break;
                    case PREV:
                        prevSelect();
                        menu = START;
                        button = -1;
                        break;
                    case NEXT:
                        nextSelect();
                        button = -1;
                        break;
                    case BACK:
                        menu = START;
                        button = -1;
                        break;
                }

            //LEVEL 2

            case NODES:
                
                clearDisplay();
                state.menu_main = "NODES";
                state.menu_prev = "";
                state.menu_next = "CURRENT";
                state.menu_back = "SETTINGS";
                printUI();
                Display();

                switch(button) {
                    case SELECT:
                        mainSelect();
                        menu = NUM_NODES8;
                        button = -1;
                        break;
                    case PREV:
                        prevSelect();
                        button = -1;
                        break;
                    case NEXT:
                        nextSelect();
                        menu = CURRENT;
                        button = -1;
                        break;
                    case BACK:
                        menu = SETTINGS;
                        button = -1;
                        break;
                }

                case NUM_NODES8:
                    
                    
                    clearDisplay();
                    state.menu_main = "8";
                    state.menu_prev = "";
                    state.menu_next = "16";
                    state.menu_back = "NODES";
                    printUI();
                    Display();

                    switch(button) {
                        case SELECT:
                            mainSelect();
                            button = -1;
                            break;
                        case PREV:
                            prevSelect();
                            button = -1;
                            break;
                        case NEXT:
                            nextSelect();
                            menu = NUM_NODES16;
                            button = -1;
                            break;
                        case BACK:
                            menu = NODES;
                            button = -1;
                            break;
                    }

                case NUM_NODES16:

                    
                    clearDisplay();
                    state.menu_main = "16";
                    state.menu_prev = "8";
                    state.menu_next = "32";
                    state.menu_back = "NODES";
                    printUI();
                    Display();

                    switch(button) {
                        case SELECT:
                            mainSelect();
                            button = -1;
                            break;
                        case PREV:
                            prevSelect();
                            menu = NUM_NODES8;
                            button = -1;
                            break;
                        case NEXT:
                            nextSelect();
                            menu = NUM_NODES32;
                            button = -1;
                            break;
                        case BACK:
                            menu = NODES;
                            button = -1;
                            break;
                    }       

                case NUM_NODES32:


                    clearDisplay();
                    state.menu_main = "32";
                    state.menu_prev = "16";
                    state.menu_next = "";
                    state.menu_back = "NODES";
                    printUI();
                    Display();

                    switch(button) {
                        case SELECT:
                            mainSelect();
                            button = -1;
                            break;
                        case PREV:
                            prevSelect();
                            menu = NUM_NODES16;
                            button = -1;
                            break;
                        case NEXT:
                            nextSelect();
                            button = -1;
                            break;
                        case BACK:
                            menu = NODES;
                            button = -1;
                            break;
                    }             

            case CURRENT:


                clearDisplay();
                state.menu_main = "CURRENT";
                state.menu_prev = "NODES";
                state.menu_next = "CONFIG";
                state.menu_back = "SETTINGS";
                printUI();
                Display();

                switch(button) {
                    case SELECT:
                        mainSelect();
                        menu = CURRENT_AUTO;
                        button = -1;
                        break;
                    case PREV:
                        prevSelect();
                        button = -1;
                        break;
                    case NEXT:
                        nextSelect();
                        menu = CONFIG;
                        button = -1;
                        break;
                    case BACK:
                        menu = SETTINGS;
                        button = -1;
                        break;

                case CURRENT_AUTO:
                    
                    clearDisplay();
                    state.menu_main = "AUTO";
                    state.menu_prev = "";
                    state.menu_next = "MANUAL";
                    state.menu_back = "CURRENT";
                    printUI();
                    Display();

                    switch(button) {
                        case SELECT:
                            mainSelect();
                            button = -1;
                            break;
                        case PREV:
                            prevSelect();
                            button = -1;
                            break;
                        case NEXT:
                            nextSelect();
                            menu = CURRENT_MANUAL;
                            button = -1;
                            break;
                        case BACK:
                            menu = NODES;
                            button = -1;
                            break;
                    }

                case CURRENT_MANUAL:

                    clearDisplay();
                    state.menu_main = "MANUAL";
                    state.menu_prev = "AUTO";
                    state.menu_next = "";
                    state.menu_back = "CURRENT";
                    printUI();
                    Display();

                    switch(button) {
                        case SELECT:
                            mainSelect();
                            button = -1;
                            break;
                        case PREV:
                            prevSelect();
                            menu = CURRENT_AUTO;
                            button = -1;
                            break;
                        case NEXT:
                            nextSelect();
                            break;
                        case BACK:
                            menu = CURRENT;
                            button = -1;
                            break;
                    }                  }   

        }
    }

    pthread_join(select_thread, NULL);
    pthread_join(next_thread, NULL);
    pthread_join(prev_thread, NULL);
    printf("\n EXITED CLEANLY \n");







    // /* Clear display */
    // clearDisplay();
    // state.batt = 12;
    // state.menu_main = "START";
    // state.menu_prev = "SETTINGS";
    // state.menu_next = "NODES";
    // state.menu_back = "HOME";
    // printUI();
    // Display();


    // usleep(1e6);
    // mainSelect();
    // clearDisplay();
    // state.batt = 13;
    // state.menu_main = "SETTINGS";
    // state.menu_prev = "START";
    // state.menu_next = "CURRENT";
    // state.menu_back = "HOME";
    // printUI();
    // Display();
    // usleep(1e6);

    // prevSelect();
    // usleep(1e6);
    // nextSelect();

    // usleep(1e6);
    // mainSelect();

    // // draw a single pixel
    // drawPixel(0, 1, WHITE);
    // Display();
    // usleep(1000000);
    // clearDisplay();

    // // draw many lines
    // testdrawline();
    // usleep(1000000);
    // clearDisplay();

    // // draw rectangles
    // testdrawrect();
    // usleep(1000000);
    // clearDisplay();

    // // draw multiple rectangles
    // testfillrect();
    // usleep(1000000);
    // clearDisplay();

    // // draw mulitple circles
    // testdrawcircle();
    // usleep(1000000);
    // clearDisplay();

    // // draw a white circle, 10 pixel radius
    // fillCircle(SSD1306_LCDWIDTH/2, SSD1306_LCDHEIGHT/2, 10, WHITE);
    // Display();
    // usleep(1000000);
    // clearDisplay();

    // // draw a white circle, 10 pixel radius
    // testdrawroundrect();
    // usleep(1000000);
    // clearDisplay();

    // // Fill the round rectangle
    // testfillroundrect();
    // usleep(1000000);
    // clearDisplay();

    // // Draw triangles
    // testdrawtriangle();
    // usleep(1000000);
    // clearDisplay();

    // // Fill triangles
    // testfilltriangle();
    // usleep(1000000);
    // clearDisplay();

    // // draw the first ~12 characters in the font
    // testdrawchar();
    // usleep(1000000);
    // clearDisplay();

    // // Display "scroll" and scroll around
    // testscrolltext();
    // usleep(1000000);
    // clearDisplay();

    // // Display Texts and Numbers
    // display_texts();
    // Display();
    // usleep(1000000);
    // clearDisplay();

    // Display miniature bitmap
    // display_bitmap();

    // clearDisplay();
    // // printRight("50%",0,1);
    // // printLeft("Battery",0,1);
    // printBattery(69.431);
    // printCenter(state.menu_loc,2);
    // printLeft("<-",SSD1306_LCDHEIGHT-9-8,1);
    // printRight("->",SSD1306_LCDHEIGHT-9-8,1);
    // Display();
    // usleep(1e6);

    // clearDisplay();
    // printBattery(69.431);
    // printLeft("SETTINGS",1,1);
    // printCenter("SETTINGS",2);
    // printLeft("<-",SSD1306_LCDHEIGHT-9-8,1);
    // printRight("->",SSD1306_LCDHEIGHT-9-8,1);
    // Display();
    // usleep(1e6);


    // clearDisplay();
    // printBattery(69.431);
    // printCenterSelect("SETTINGS",2);
    // printLeft("<-",SSD1306_LCDHEIGHT-9-8,1);
    // printRight("->",SSD1306_LCDHEIGHT-9-8,1);
    // Display();

    // clearDisplay();
    // printBattery(69.431);
    // printCenter("# NODES",2);
    // printLeft("SETTINGS",1,1);
    // printLeft("<-",SSD1306_LCDHEIGHT-9-8,1);
    // printRight("->",SSD1306_LCDHEIGHT-9-8,1);
    // Display();
    // usleep(1e6);

    // clearDisplay();
    // printBattery(69.431);
    // printCenter("CURRENT",2);
    // printLeft("SETTINGS",1,1);
    // printLeft("<-",SSD1306_LCDHEIGHT-9-8,1);
    // printRight("->",SSD1306_LCDHEIGHT-9-8,1);
    // Display();
    // usleep(1e6);

    // clearDisplay();
    // printBattery(69.431);
    // printLeft("SETTINGS",1,1);
    // printCenter("CONFIG",2);
    // printLeft("<-",SSD1306_LCDHEIGHT-9-8,1);
    // printRight("->",SSD1306_LCDHEIGHT-9-8,1);
    // Display();
    // usleep(1e6);

    // clearDisplay();
    // printBattery(69.431);
    // printLeft("SETTINGS",1,1);
    // printCenter("SAMPLES",2);
    // printLeft("<-",SSD1306_LCDHEIGHT-9-8,1);
    // printRight("->",SSD1306_LCDHEIGHT-9-8,1);
    // Display();
    // usleep(1e6);



    // printf("\n %d \n",x1);
    // printf("\n %d \n",x2-x1);
    // printf("\n %d \n",y1);
    // printf("\n %d \n",y2);

    // setTextSize(2);
    // drawRect(x1,y1,(x2 - x1),(y2 - y1),WHITE);
    // setTextColor(WHITE);
    // setCursor(x1,y1);
    // print_str("START");
    // Display();
    // usleep(1000000);
    // fillRect(x1,y1,(x2 - x1),(y2 - y1),BLACK);
    // Display();


    // fillRect(15,15,100,30,WHITE);
    // setTextColor(INVERSE);
    // setCursor(21,20);
    // print_str("START");
    // Display();

    // usleep(1000000);

    // fillRect(15,15,100,30,BLACK);
    // setTextColor(INVERSE);
    // setCursor(21,20);
    // print_str("START");
    // Display();

    // setTextWrap(0);
    // setTextSize(3);
    // fillRect(15,15,100,30,BLACK);
    // setTextColor(WHITE);
    // setCursor(21,20);
    // print_str("START");
    // Display();

    // usleep(1000000);
    // startscrollright(0x02, 0x07);
    // usleep(1000000);
    // stopscroll();





    // // Display Inverted image and normalize it back
    // display_invert_normal();
    // clearDisplay();
    // usleep(1000000);
    // Display();

    // Generate Signal after 20 Seconds
        alarm(20);

    // // draw a bitmap icon and 'animate' movement
    // testdrawbitmap_eg();
    // clearDisplay();
    // usleep(1000000);
    // Display();

    // // Good bye fellas :)
    // deeplyembedded_credits();
    // Display();
    }

/* Alarm Signal Handler */
void ALARMhandler(int sig)
{
    /* Set flag */
    flag = 5;
    state.system = STOPPED;

    clearDisplay();
}

    /*****************************************************************
 * MAE 156B Spring 2018 Team 6
 *
 *  UI demo script for Preliminary Design presentation on 5/23/18
 * 
 *  compile with "gcc -pthread demo.c gpiolib.c example_app.c I2C.c SSD1306_OLED.c -o demo"
 *
 ********************************************************************/
 
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

/* Oh Compiler-Please leave me as is */
volatile unsigned char flag = 0;

/*******************************************************************************
* DEFINES/VARIABLES
*******************************************************************************/
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define DEBOUNCE 0*1e6          // 1 seconds
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
#define PAUSED 0

//GPIO pin IDs
int oled_rst_gpio   = 80;
int oled_pwr_gpio   = 81;

gpio_info *oled_rst_gpio_info;  //OLED reset pin, pull low for a few milliseconds to reinitialize display
gpio_info *oled_pwr_gpio_info;  //OLED power pin, pull high to turn on display

/*******************************************************************************
* state_t
*
* 
*******************************************************************************/

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

state_t state;
int button = -1;
int menu;
pthread_t button_thread;

/************************************************************************************
* SETUP SIGINT HANDLER
*************************************************************************************/
void sigint(int s __attribute__((unused)));
void ALARMhandler(int sig);

/************************************************************************************
* DECLARE PTHREADS
*************************************************************************************/
void* button_poll(void* ptr);

/************************************************************************************
* FUNCTIONS
*************************************************************************************/
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

void printUI(UI_state_t UI_state){
    clearDisplay();
    printCenter(UI_state.menu_main,2);
    printLeft(UI_state.menu_back,1,1);
    printBattery(state.batt);
    printLeft(UI_state.menu_prev,SSD1306_LCDHEIGHT-9,1);
    printRight(UI_state.menu_next,SSD1306_LCDHEIGHT-9,1);
    printCenterY("<",1,2);
    printCenterY(">",SSD1306_LCDWIDTH-12-1,2);
    Display();
}

void mainSelect(UI_state_t UI_state){
    printMenuCenterSelect(UI_state.menu_main,2);
    Display();
    usleep(0.1*1e6);

    printCenter(UI_state.menu_main,2);
    Display();
}

int process_button(UI_state_t UI_state){
    switch(button) {
        case SELECT:
            mainSelect(UI_state);
            menu = UI_state.button_select;
            button = -1;
            return 1;
            break;
        case PREV:
            prevSelect();
            menu = UI_state.button_prev;
            button = -1;
            return 0;
            break;
        case NEXT:
            nextSelect();
            menu = UI_state.button_next;
            button = -1;
            return 0;
            break;
        case BACK:
            menu = UI_state.button_back;
            button = -1;
            return 0;
            break;
        case -1:
            break;
    }
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

/************************************************************************************
* DEFINE PTHREAD
*************************************************************************************/
void* button_poll(void* ptr){
    struct pollfd fdset[3];
    int nfds = 3;
    int gpio_fd[3], timeout, rc;
    char *buf[MAX_BUF];
    unsigned int gpio[3];
    int len;

    gpio[0] = 61; // button select
    gpio[1] = 88; // button previous
    gpio[2] = 89; // button next

    for(int i=0;i<3;i++){
        gpio_export(gpio[i]);
        gpio_set_dir(gpio[i], 0);
        gpio_set_edge(gpio[i], "rising");
        gpio_fd[i] = gpio_fd_open(gpio[i]);
    }

    timeout = POLL_TIMEOUT;

    while(state.system){
        memset((void*)fdset, 0, sizeof(fdset));
        
        for(int i=0;i<3;i++){
            fdset[i].fd = gpio_fd[i];
            fdset[i].events = POLLPRI;
        }

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

        if (fdset[SELECT].revents & POLLPRI) {
            lseek(fdset[SELECT].fd, 0, SEEK_SET);
            len = read(fdset[SELECT].fd, buf, MAX_BUF);
            printf("\npoll() GPIO select %d interrupt occurred!\n", gpio[SELECT]);
            button = SELECT;
        }

        if (fdset[PREV].revents & POLLPRI) {
            lseek(fdset[PREV].fd, 0, SEEK_SET);
            len = read(fdset[PREV].fd, buf, MAX_BUF);
            printf("\npoll() GPIO prev %d interrupt occurred!\n", gpio[PREV]);
            button = PREV;
        }

        if (fdset[NEXT].revents & POLLPRI) {
            lseek(fdset[NEXT].fd, 0, SEEK_SET);
            len = read(fdset[NEXT].fd, buf, MAX_BUF);
            printf("\npoll() GPIO next %d interrupt occurred!\n", gpio[NEXT]);
            button = NEXT;
        }     

        fflush(stdout);
    }

    for(int i=0;i<4;i++){
        gpio_unexport(gpio[i]);
        gpio_fd_close(gpio_fd[i]);
    }

    return NULL;
}

/************************************************************************************
* MAIN
*************************************************************************************/
int main()
{
    /* initialize gpiolib */
    if(gpio_init()){
        fprintf(stderr, "\n gpio_init failed with %i", gpio_errno);
    }
    printf("\n gpiolib intialized...");
    fflush(stdout);

    /* attach OLED RST and PWR buttons */
    int bank = oled_rst_gpio/32;
    int mask = bit(oled_rst_gpio%32);
    oled_rst_gpio_info = gpio_attach(bank, mask, GPIO_OUT);

    bank = oled_pwr_gpio/32;
    mask = bit(oled_pwr_gpio%32);
    oled_pwr_gpio_info = gpio_attach(bank, mask, GPIO_OUT);
    printf("\n OLED power and reset pins attached...");
    fflush(stdout);

    /* power on OLED */
    gpio_set(oled_pwr_gpio_info);
    gpio_set(oled_rst_gpio_info);
    printf("\n OLED power and reset pins set high...");
    fflush(stdout);

    /* reset OLED */
    gpio_clear(oled_rst_gpio_info);
    usleep(0.5*1e6);
    gpio_set(oled_rst_gpio_info);
    printf("\n OLED display reset...");
    fflush(stdout);

    /* Initialize I2C bus and connect to the I2C Device */
    if(init_i2c_dev2(SSD1306_OLED_ADDR) == 0)
    {
        printf("\n (Main)i2c-2: Bus Connected to SSD1306");
        fflush(stdout);
    }
    else
    {
        printf("\n (Main)i2c-2: OOPS! Something Went Wrong");
        fflush(stdout);
        exit(1);
    }


    // // Register the Alarm Handler 
    // signal(SIGALRM, ALARMhandler);
    // printf("\n Alarm handler registered...");
    // fflush(stdout);

    /* Run SDD1306 Initialization Sequence */
    display_Init_seq();
    printf("\n init sequence displayed...");
    fflush(stdout);
    state.batt = 99.0;
    menu = START;

    printf("\n OLED initialized...");
    fflush(stdout);

    /* Display ARMOR logo */
    printf("\n Displaying ARMOR logo...");
    fflush(stdout);
    display_bitmap();
    Display();
    usleep(2*1e6);

    /* start button poll pthread */
    pthread_create(&button_thread, NULL, button_poll, (void*) NULL);
    printf("\n polling buttons...");
    fflush(stdout);
    

    /* register sigint */
    signal(SIGINT,sigint);
    printf("\n SIGINT registered...");
    fflush(stdout);

    /* enter menu interface */
    printf("\n entering menu interface...");
    fflush(stdout);


    
    state.system = RUNNING;
    usleep(0.5*1e6);
    button = -1;
    
    // int next_menu, next_button;
    while(state.system == RUNNING){
        switch(menu) {
            // LEVEL 1
            case START:
                printUI(UI_start);
                if(process_button(UI_start)){ // Set state to running
                }
                break;

            case SETTINGS:

                printUI(UI_settings);
                process_button(UI_settings);
                break;

            case NODES:

                printUI(UI_nodes);
                process_button(UI_nodes);
                break;

            case NUM_NODES8:

                printUI(UI_nodes8);
                if(process_button(UI_nodes8)){
                    // set nodes to 8
                }
                break;

            case NUM_NODES16:

                printUI(UI_nodes16);
                if(process_button(UI_nodes16)){
                    // set nodes to 16
                }
                break;     

            case NUM_NODES32:

                printUI(UI_nodes32);
                if(process_button(UI_nodes32)){
                    // set nodes to 32
                }
                break;           

            case CURRENT:

                printUI(UI_current);
                process_button(UI_current);
                break;

            case CURRENT_AUTO:

                printUI(UI_current_auto);
                if(process_button(UI_current_auto)){
                        // set current to auto
                }
                break;

            case CURRENT_MANUAL:

                printUI(UI_current_manual);
                process_button(UI_current_manual);
                break;

            case CONFIG:

                printUI(UI_config);
                process_button(UI_config);
                break;                  
        }
    }

    /* Display ARMOR logo */
    printf("\n displaying ARMOR logo");
    fflush(stdout);
    display_bitmap();
    Display();
    usleep(2*1e6);

    /* clean up */
    gpio_detach(oled_rst_gpio_info);
    gpio_detach(oled_pwr_gpio_info);
    printf("\n detached gpio pins");
    fflush(stdout);

    gpio_finish();
    printf("\n closed gpiolib cleanly...");
    fflush(stdout);

    pthread_join(button_thread, NULL);
    printf("\n EXITED CLEANLY \n");
}

/*************************************************************************************
* DECLARE SIGINT
*************************************************************************************/
void sigint(int s __attribute__((unused))) {
    printf("\n Received SIGINT: Exiting cleanly...");
    fflush(stdout);

    /* Display ARMOR logo */
    clearDisplay();
    display_bitmap();
    Display();
    usleep(2*1e6);

    state.system = STOPPED;
    
    gpio_detach(oled_rst_gpio_info);
    gpio_detach(oled_pwr_gpio_info);
    printf("\n detached gpio pins");
    fflush(stdout);

    gpio_finish();
    printf("\n closed gpiolib cleanly...");
    fflush(stdout);

    pthread_join(button_thread, NULL);
    exit(0);
}

// /* Alarm Signal Handler */
// void ALARMhandler(int sig)
// {
//     /* Set flag */
//     flag = 5;
//     clearDisplay();
// }

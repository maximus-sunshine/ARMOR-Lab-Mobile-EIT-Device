/* Copyright (c) 2011, RidgeRun
 * All rights reserved.
 *
 * Contributors include:
 *   Todd Fischer
 *   Brad Lu
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the RidgeRun.
 * 4. Neither the name of the RidgeRun nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY RIDGERUN ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL RIDGERUN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * compile with "gcc invert.c example_app.c I2C.c SSD1306_OLED.c -o invert"
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>

/* OLED includes */
#include "I2C.h"
#include "SSD1306_OLED.h"
#include "example_app.h"

/* Externs - I2C.c */
extern I2C_DeviceT I2C_DEV_2;

/* Oh Compiler-Please leave me as is */
volatile unsigned char flag = 0;

/* Alarm Signal Handler */
void ALARMhandler(int sig)
{
    /* Set flag */
    flag = 5;
}

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define DEBOUNCE 0*1e6       // 1 seconds
#define MAX_BUF 64

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

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
        struct pollfd fdset[1];
        int nfds = 1;
        int gpio_fd, timeout, rc;
        char *buf[MAX_BUF];
        unsigned int gpio;
        int len;
        int n = 0;
        double debounce = DEBOUNCE;

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
        signal(SIGALRM, ALARMhandler);

        /* Run SDD1306 Initialization Sequence */
        display_Init_seq();

        /* Clear display */
        clearDisplay();

        // Generate Signal after 20 Seconds
        alarm(20);


        if (argc < 2) {
                printf("Usage: gpio-int <gpio-pin>\n\n");
                printf("Waits for a change in the GPIO pin voltage level or input on stdin\n");
                exit(-1);
        }

        gpio = atoi(argv[1]);

        gpio_export(gpio);
        gpio_set_dir(gpio, 0);
        gpio_set_edge(gpio, "rising");
        gpio_fd = gpio_fd_open(gpio);

        timeout = POLL_TIMEOUT;

        // Display ARMOR logo
        display_bitmap();
        Display();
 
        while (1) {
                memset((void*)fdset, 0, sizeof(fdset));

                // fdset[0].fd = STDIN_FILENO;
                // fdset[0].events = POLLIN;
      
                // fdset[1].fd = gpio_fd;
                // fdset[1].events = POLLPRI;

                fdset[0].fd = gpio_fd;
                fdset[0].events = POLLPRI;

                rc = poll(fdset, nfds, timeout);      

                if (rc < 0) {
                        if (errno == EINTR) {
                                printf("\nInterrupted system call... continuing\n");
                                continue;
                        }
                        perror("\npoll() failed!\n");
                        return -1;
                }
      
                if (rc == 0) {
                        printf(".");
                }
                
                //Invert display on button press and print to screen
                if (fdset[0].revents & POLLPRI) {
                        lseek(fdset[0].fd, 0, SEEK_SET);
                        len = read(fdset[0].fd, buf, MAX_BUF);
                        printf("\npoll() GPIO %d interrupt occurred %d times!\n", gpio, n);
                        if (n % 2 == 0){
                                invertDisplay(SSD1306_NORMALIZE_DISPLAY);
                                Display();    
                        }
                        else{
                                invertDisplay(SSD1306_INVERT_DISPLAY);
                                Display();   
                        }
                        n++;
                }

                // if (fdset[0].revents & POLLIN) {
                //         (void)read(fdset[0].fd, buf, 1);
                //         printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
                // }
                //debounce
                usleep(debounce);
                fflush(stdout);
        }

        clearDisplay();
        gpio_unexport(gpio);
        gpio_fd_close(gpio_fd);
        return 0;
}

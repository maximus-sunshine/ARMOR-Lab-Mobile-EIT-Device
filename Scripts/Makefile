.PHONY:clean all utils
CFLAGS=-Wall -Wextra -O99
SHELL=/bin/bash
NAME=gpiolib

all: $(NAME).o gpio_lib_test


%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(NAME).so $(NAME).o
	$(MAKE) -C utils clean

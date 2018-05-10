TARGETS := libiio_test

CFLAGS = -Wall

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	CFLAGS += -framework iio
else
	LDFLAGS += -liio
endif

.PHONY: all clean

all: $(TARGETS)

libiio_test : libiio_test.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGETS) $(TARGETS:%=%.o)

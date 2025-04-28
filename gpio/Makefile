CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET ?= gpio_button
LDFLAGS ?= -lpthread -lrt -lgpiod

all:	$(TARGET)

$(TARGET):	gpio_button.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) 

.PHONY:	all clean


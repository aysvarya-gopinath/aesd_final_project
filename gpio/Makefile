CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET ?= gpio_button
LDFLAGS ?= -lpthread -lrt 
LIBS    += -lgpiod

all:	$(TARGET)

$(TARGET):gpio_button.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@  $(LIBS)
	
clean:
	rm -f $(TARGET) 

.PHONY:	all clean





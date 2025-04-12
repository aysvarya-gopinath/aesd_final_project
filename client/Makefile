CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET ?= tcp_client
LDFLAGS ?= -lpthread -lrt

all:	$(TARGET)

$(TARGET):	tcp_client.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) 

.PHONY:	all clean

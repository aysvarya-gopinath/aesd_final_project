CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
TARGET ?= tcp_server
LDFLAGS ?= -lpthread -lrt

all:	$(TARGET)

$(TARGET):	tcp_server.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) 

.PHONY:	all clean

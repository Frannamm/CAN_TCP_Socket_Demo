SRCS += ../../iso14229.c main.c
HDRS += ../../iso14229.h
TARGET = client
CFLAGS = -DUDS_TP_ISOTP_SOCK=1 -g -I../..

all: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
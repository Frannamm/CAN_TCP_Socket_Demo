CC = gcc
CFLAGS = -Wall

all: sender receiver server client

server: TCP_server.c
	$(CC)	$(CFLAGS) -o server TCP_server.c

client: TCP_client.c
	$(CC)	$(CFLAGS) -o client TCP_client.c

sender: can_sender.c
	$(CC)	$(CFLAGS) -o sender can_sender.c

receiver: can_receiver.c
	$(CC)	$(CFLAGS) -o receiver can_receiver.c

clean:
	rm -f server client sender receiver

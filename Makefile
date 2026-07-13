CC = gcc
CFLAGS = -Wall

all: server client

server: TCP_server.c
	$(CC)	$(CFLAGS) -o server TCP_server.c

client: TCP_client.c
	$(CC)	$(CFLAGS) -o client TCP_client.c

clean:
	rm -f server client

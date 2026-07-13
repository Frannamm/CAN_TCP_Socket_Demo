# TCP Client-Server Demo

A simple TCP client-server implementation in C using the sockets API. 
This project demonstrates a basic connection between a client and server, 
including socket creation, connecting, and receiving data.

## Files

- `TCP_server.c` — Sets up a server socket, listens for incoming connections, 
  and sends data to a connected client.
- `TCP_client.c` — Connects to the server and receives the data sent.

## How to Compile

```bash
gcc TCP_server.c -o server
gcc TCP_client.c -o client
```

## How to Run

1. Start the server first:
```bash
./server
```

2. In a separate terminal, run the client:
```bash
./client
```

## Notes

This is a basic demonstration of TCP socket communication and can be 
extended further (e.g. bidirectional messaging, handling multiple clients, 
error handling improvements).

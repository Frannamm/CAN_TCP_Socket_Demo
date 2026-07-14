# TCP Client-Server Demo
A simple TCP client-server implementation in C using the sockets API.
This project demonstrates TCP communication between a client and server,
including socket creation, connecting, sending/receiving data, handling
multiple simultaneous clients using `fork()`, and detecting client
connect/disconnect events.

## Files
- `TCP_server.c` — Sets up a server socket, listens for incoming connections,
  and forks a new child process to handle each connected client. Each child:
  - Prints the connecting client's IP and port
  - Prints its own process ID (used as a client identifier)
  - Sends a greeting message to the client
  - Receives data from the client in a loop, printing each message and its
    length
  - Detects and prints when the client disconnects or an error occurs
- `TCP_client.c` — Connects to the server (given an IP address, port, and a
  data string), receives the server's greeting, sends its own data, and
  prints:
  - The data received from the server
  - The data it sent, and its length
  - Its own process ID
- `Makefile` — Builds both `server` and `client` binaries.

## How to Compile
```bash
make
```
This builds two binaries: `server` and `client`.

To remove the compiled binaries:
```bash
make clean
```

## How to Run
The server takes an IP address and port. The client takes an IP address,
port, and a message to send.

1. Start the server first:
```bash
./server 127.0.0.1 5555
```

2. In a separate terminal, run the client with a message to send:
```bash
./client 127.0.0.1 5555 "hello server"
```

You can run multiple clients (in separate terminals) against the same
running server to see the multi-client handling in action — each client
is served by its own forked child process on the server.

## Notes
- Client and server process IDs are independent and will not match — each
  is a separate, unrelated process on its own machine.
- Possible extensions: bidirectional continuous messaging, more robust
  error handling, and configurable buffer sizes.

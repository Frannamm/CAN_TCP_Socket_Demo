# CAN & TCP Socket Programming Demos

A collection of C socket programming demos: a TCP client-server, and a
CAN bus client-server using SocketCAN (with a virtual CAN interface for
testing without hardware).

## Contents
- `TCP_server.c` / `TCP_client.c` — TCP client-server demo (see below)
- `can_sender.c` / `can_receiver.c` — CAN bus sender/receiver demo (see below)
- `Makefile` — builds all four binaries

## Build
```bash
make
```
Builds `server`, `client`, `sender`, `receiver`.

```bash
make clean
```
Removes all compiled binaries.

---

## TCP Client-Server

A TCP client-server implementation demonstrating socket creation,
connecting, sending/receiving data, multi-client handling via `fork()`,
and connect/disconnect detection.

### Files
- `TCP_server.c` — Sets up a server socket, listens for incoming
  connections, and forks a new child process to handle each connected
  client. Each child:
  - Prints the connecting client's IP and port
  - Prints its own process ID (used as a client identifier)
  - Sends a greeting message to the client
  - Receives data from the client in a loop, printing each message and
    its length
  - Detects and prints when the client disconnects or an error occurs
- `TCP_client.c` — Connects to the server (given an IP address, port,
  and a data string), receives the server's greeting, sends its own
  data, and prints the data received, the data sent (and its length),
  and its own process ID.

### Run
```bash
./server 127.0.0.1 5555
./client 127.0.0.1 5555 "hello server"
```
You can run multiple clients against the same server to see the
multi-client handling in action.

---

## CAN Sender/Receiver

A SocketCAN demo showing how to send and receive raw CAN frames over a
CAN interface (tested using a virtual interface, `vcan0`).

### Files
- `can_receiver.c` — Binds to a CAN interface (given as a CLI argument)
  and reads a CAN frame, printing its ID, data length, and data bytes.
- `can_sender.c` — Binds to a CAN interface and sends a CAN frame with
  a CAN ID and data bytes provided as CLI arguments (in hex).

### Setting up a virtual CAN interface (for testing without hardware)
```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```
Note: virtual CAN interfaces do not persist across reboots — you'll
need to recreate `vcan0` after restarting your machine.

### Run
```bash
./receiver vcan0
```
In a separate terminal:
```bash
./sender vcan0 123 DEADBEEF
```
This sends a frame with CAN ID `0x123` and data bytes `DE AD BE EF`.
The receiver should immediately print the received frame's details.

## Notes
- CAN is a broadcast protocol — there's no `connect()`, and every node
  on the bus sees every frame. "Sender" and "receiver" here are just
  roles, not a client/server relationship like TCP.
- Possible extensions: continuous receive loop, CAN ID filtering,
  multiple simultaneous senders.

## Running the TCP Server as a systemd Service (Auto-start on Boot)

The TCP server can be installed as a `systemd` service so it starts
automatically on boot and restarts if it crashes.

### 1. Build the binary first
```bash
make
```
The service depends on the compiled `server` binary existing at a fixed
path — if you ever run `make clean` or clone the repo fresh, rebuild
before (re)starting the service, or it will fail with `status=203/EXEC`.

### 2. Create the service file
```bash
sudo nano /etc/systemd/system/TCP_demo.service
```
```ini
[Unit]
Description=TCP Demo Server
After=network.target

[Service]
ExecStart=/absolute/path/to/CAN_TCP_Socket_Demo/server 127.0.0.1 5555
WorkingDirectory=/absolute/path/to/CAN_TCP_Socket_Demo
Restart=always
User=your_username
Group=your_username

[Install]
WantedBy=multi-user.target
```
Replace the paths and username with your own (`pwd` and `whoami` to check).

### 3. Enable and start it
```bash
sudo systemctl daemon-reload
sudo systemctl enable TCP_demo
sudo systemctl start TCP_demo
```

### 4. Check status / logs
```bash
sudo systemctl status TCP_demo
sudo journalctl -u TCP_demo
```

### Useful commands
```bash
sudo systemctl restart TCP_demo   # after rebuilding the binary
sudo systemctl stop TCP_demo
sudo systemctl disable TCP_demo   # remove auto-start on boot
```
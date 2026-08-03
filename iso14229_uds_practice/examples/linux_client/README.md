# Linux Client Example

Basic UDS client example using Linux kernel ISO-TP sockets (socketcan).

## Overview

This example demonstrates a UDS client running on Linux, communicating with
a UDS server over SocketCAN. On startup, it requests an Extended Diagnostic
Session (0x10), then sends a ReadDataByIdentifier (0x22) request for a test
DID and prints the response.

The client is driven by an explicit request/response state machine
(`STATE_INIT → STATE_AWAIT_SESSION → STATE_AWAIT_RDBI → STATE_DONE`, with
`STATE_ERROR` reachable from either awaiting state). It retries each request
up to `MAX_RETRIES` times on timeout before giving up.

## Development history

`raw_socket_client_attempt1.c` is an earlier version written using raw
SOCK_RAW/CAN_RAW sockets and hand-built ISO-TP frames, before switching to the
iso14229 library's client API in `main.c`. Kept for reference.

`main.c` itself has evolved through three stages (see git history):
1. Initial client sending a single RDBI request
2. Added Diagnostic Session Control + explicit state machine
3. Added bounded retry logic on request timeout

## Files

- `main.c` - Client implementation: session control → RDBI, with retry-on-timeout

## Building

```bash
make
```

## Running

Requires a CAN interface (virtual or physical) with ISO-TP support, and a UDS
server (see `../linux_server`) already running on the same interface.

```bash
./client vcan0
```

## Known limitations

- No P2/P2* timing override handling — the client does not currently
  distinguish or adapt to server-specified timing constraints
- Error handling on `UDS_EVT_Err` is generic; does not branch on specific
  negative response codes (NRCs)

## Requirements

- Linux kernel with ISO-TP support (CONFIG_CAN_ISOTP)
- SocketCAN interface
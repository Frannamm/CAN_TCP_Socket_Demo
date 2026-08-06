# Linux Client Example

Basic UDS client example using Linux kernel ISO-TP sockets (socketcan).

## Overview

This example demonstrates a UDS client running on Linux, communicating with
a UDS server over SocketCAN. It requests an Extended Diagnostic Session
(0x10), then sends a ReadDataByIdentifier (0x22) request for a chosen DID
and prints the response.

The client is driven by an explicit request/response state machine
(`STATE_INIT → STATE_AWAIT_SESSION → STATE_AWAIT_RDBI → STATE_DONE`, with
`STATE_ERROR` reachable from either awaiting state). It retries each request
up to `MAX_RETRIES` times on timeout before giving up. Sends are only issued
from the main loop (never from inside the event callback) and automatically
retried if the library reports `UDS_ERR_BUSY`.

DID definitions are shared with the server via `dids.h`.

## Development history

`raw_socket_client_attempt1.c` is an earlier version written using raw
SOCK_RAW/CAN_RAW sockets and hand-built ISO-TP frames, before switching to the
iso14229 library's client API in `main.c`. Kept for reference.

`main.c` has evolved through several stages (see git history): initial
single-RDBI client → session control + state machine → retry-on-timeout →
fixed silent send failures caused by calling send functions from inside the
event callback → interactive multi-DID testing mode.

## Files

- `main.c` — client implementation
- `dids.h` — shared DID definitions (also used by `../linux_server`)

## Building

```bash
make
```

## Running

Requires a CAN interface (virtual or physical) with ISO-TP support, and a UDS
server (see `../linux_server`) already running on the same interface.

**One-shot mode** (test a single DID, exits after):
```bash
./client vcan0 F190
```

**Interactive mode** (test multiple DIDs without restarting):
```bash
./client vcan0
# Enter DID (hex, e.g. F190) or 'q' to quit: DEAF
# ...
# Enter DID (hex, e.g. F190) or 'q' to quit: q
```

## Available test DIDs (see dids.h)

| DID | Description |
|---|---|
| F190 | VIN (fixed value) |
| DEAD | Read/write scratch value |
| F18C | ECU serial (fixed) |
| F18D | ECU HW version (fixed) |
| F18E | ECU SW version (fixed) |
| DEAF | Slow scratch — 2x pending (0x78) before responding, ~3s |
| BEEF | Slow scratch 2 — 4x pending (0x78) before responding, ~6s |

## Known limitations

- Error handling on `UDS_EVT_Err` distinguishes timeout from other errors
  (retries only on timeout) but does not branch on specific NRC codes beyond
  that

## Requirements

- Linux kernel with ISO-TP support (CONFIG_CAN_ISOTP)
- SocketCAN interface
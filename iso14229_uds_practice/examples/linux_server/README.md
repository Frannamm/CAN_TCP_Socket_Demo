# Linux Server Example

Basic UDS server example using Linux kernel ISO-TP sockets (socketcan).

## Overview

This example demonstrates a UDS server running on Linux using the kernel's
built-in ISO-TP support via SocketCAN. It handles:

- **DiagnosticSessionControl (0x10)** — accepts Extended Diagnostic Session
  (`UDS_LEV_DS_EXTDS`), rejects other requested session types with
  `UDS_NRC_SubFunctionNotSupported`
- **SessionTimeout** — acknowledges automatic reversion to the default
  session
- **ReadDataByIdentifier (0x22)** and **WriteDataByIdentifier (0x2E)** for
  two test DIDs: `0xF190` (read-only, fixed VIN-like value) and `0xDEAD`
  (read/write scratch value)

## Files

- `main.c` — server implementation, including session control, timeout,
  and RDBI/WDBI handlers

## Building

```bash
make
```

## Running

Requires a CAN interface (virtual or physical) with ISO-TP support.

```bash
# Create virtual CAN interface (one-time setup, see top-level README)
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

# Run the server
./server vcan0
```

## Testing manually

In another terminal:

```bash
# Request Extended Diagnostic Session
cansend vcan0 7E0#02.10.03

# Read DID 0xF190
cansend vcan0 7E0#03.22.F1.90

# Read DID 0xDEAD
cansend vcan0 7E0#03.22.DE.AD

# Write 3 bytes to DID 0xDEAD
cansend vcan0 7E0#06.2E.DE.AD.11.22.33

# Watch traffic
candump vcan0
```

## Known limitations

- Does not currently override P2/P2* timing values in its
  `DiagSessCtrl` response (fields available in `UDSDiagSessCtrlArgs_t`
  but unused)

## Requirements

- Linux kernel with ISO-TP support (`CONFIG_CAN_ISOTP`)
- SocketCAN interface
- `can-utils` (`cansend`, `candump`)
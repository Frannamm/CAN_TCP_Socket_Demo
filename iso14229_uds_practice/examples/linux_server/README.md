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
- **ReadDataByIdentifier (0x22)** for several test DIDs (see `dids.h`),
  including two DIDs that deliberately demonstrate ISO 14229's P2/P2*
  timing mechanism via NRC 0x78 (ResponsePending)
- **WriteDataByIdentifier (0x2E)** for the scratch DID (`0xDEAD`)

## Files

- `main.c` — server implementation
- `dids.h` — shared DID definitions (also used by `../linux_client`)

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

## Available test DIDs

| DID | Behavior |
|---|---|
| F190 | Fixed VIN-like value, read-only |
| DEAD | Read/write scratch value |
| F18C | Fixed ECU serial |
| F18D | Fixed ECU HW version |
| F18E | Fixed ECU SW version |
| DEAF | Responds with NRC 0x78 (pending) twice, then real data (~3s total) |
| BEEF | Responds with NRC 0x78 (pending) four times, then real data (~6s total) |

## Testing manually

```bash
# Request Extended Diagnostic Session
cansend vcan0 7E0#02.10.03

# Read DID 0xF190
cansend vcan0 7E0#03.22.F1.90

# Trigger the slow-response (0x78 pending) demonstration
cansend vcan0 7E0#03.22.DE.AF

# Watch traffic
candump vcan0
```

## Notes on timing

P2/P2* defaults come from the library:
`UDS_SERVER_DEFAULT_P2_MS=50`, `UDS_SERVER_DEFAULT_P2_STAR_MS=5000`. The
minimum interval between consecutive 0x78 responses is `0.3 * p2_star_ms`
(1500ms with defaults), per ISO14229-2:2013 Table 4 footnote b. This is
handled internally by the library — the server's event handler only needs
to return `UDS_NRC_RequestCorrectlyReceived_ResponsePending` when it isn't
ready, and the library manages re-invocation and timing automatically.

## Known limitations

- Does not currently override P2/P2* values per-session in its
  `DiagSessCtrl` response (fields available in `UDSDiagSessCtrlArgs_t`
  but left at server defaults)

## Requirements

- Linux kernel with ISO-TP support (`CONFIG_CAN_ISOTP`)
- SocketCAN interface
- `can-utils` (`cansend`, `candump`)
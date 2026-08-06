# UDS (ISO 14229) Practice — CAN Client/Server

Educational project practicing UDS diagnostics over CAN, based on the
[iso14229](https://github.com/driftregion/iso14229) library by Nick Kirkby (MIT
licensed), with modifications for hands-on learning.

## What's here
- `iso14229.c` / `iso14229.h` — the UDS library (upstream, unmodified)
- `examples/linux_server/` — UDS server over SocketCAN ISO-TP: handles
  DiagnosticSessionControl (0x10), SessionTimeout, ReadDataByIdentifier (0x22),
  and WriteDataByIdentifier (0x2E), including a deliberate demonstration of
  ISO 14229's P2/P2* timing mechanism via NRC 0x78 (ResponsePending)
- `examples/linux_client/` — UDS client with an explicit request/response
  state machine, retry-on-timeout, and an interactive mode for testing any DID
  without restarting
- `dids.h` (in each example dir) — DID definitions shared between client and
  server, so both sides always agree on values

## What I changed from upstream
- Added `case UDS_EVT_DiagSessCtrl`, `UDS_EVT_SessionTimeout`,
  `UDS_EVT_ReadDataByIdent`, and `UDS_EVT_WriteDataByIdent` handlers in
  `examples/linux_server/main.c`
- Added two DIDs (`0xDEAF`, `0xBEEF`) that intentionally respond slowly via
  NRC 0x78, to exercise the library's automatic P2* timing/retry mechanism
- Added `examples/linux_client/main.c` from scratch using the library's
  client API — evolved through session control, a state machine, retry
  logic, and an interactive multi-DID test mode
- Fixed a bug where sending a new request from inside the client's event
  callback could be silently rejected (`UDS_ERR_BUSY`) if the library's
  internal state hadn't reset yet — sends now happen from the main loop only
- Named constants for addresses/DIDs instead of magic numbers, consolidated
  into a shared `dids.h` per example directory

## Prerequisite: CAN support in WSL2

The default WSL2 kernel does **not** include CAN/ISO-TP support. This
project was developed against a custom-built WSL2 kernel with
`CONFIG_CAN`, `CONFIG_CAN_ISOTP`, `CONFIG_CAN_VCAN`, etc. enabled.

If `modprobe can_isotp` or similar fails with something like *"Address
family not supported by protocol"*, you're on the stock kernel. Building a
custom kernel is a one-time, fairly involved process (`make menuconfig` →
`make -j<cores>` → `make modules_install` → copy `vmlinux` to Windows →
point `.wslconfig` at it → `wsl --shutdown` from PowerShell). This is
outside the scope of this README; see the separate kernel build notes.

**Also note:** loaded CAN modules and the `vcan0` interface do **not**
persist across a WSL2 restart — the setup commands below need to be re-run
each new session.

## Setup: virtual CAN interface

```bash
sudo modprobe can
sudo modprobe can_raw
sudo modprobe can_isotp
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
ip link show vcan0
```

## Running the server

```bash
cd examples/linux_server
make
./server vcan0
```

## Running the client

**Interactive mode** — prompts for a DID, runs a full transaction, repeats
until you type `q`:
```bash
cd examples/linux_client
make
./client vcan0
```

**One-shot mode** — test a single DID and exit:
```bash
./client vcan0 F190
```

## Available test DIDs (see `dids.h`)

| DID | Behavior |
|---|---|
| F190 | Fixed VIN-like value, read-only |
| DEAD | Read/write scratch value |
| F18C | Fixed ECU serial |
| F18D | Fixed ECU HW version |
| F18E | Fixed ECU SW version |
| DEAF | Responds with NRC 0x78 (pending) twice, then real data (~3s total) |
| BEEF | Responds with NRC 0x78 (pending) four times, then real data (~6s total) |

## Manual testing with cansend / candump

```bash
# Request Extended Diagnostic Session
cansend vcan0 7E0#02.10.03

# Read DID 0xF190
cansend vcan0 7E0#03.22.F1.90

# Write 3 bytes to DID 0xDEAD
cansend vcan0 7E0#06.2E.DE.AD.11.22.33

# Trigger the slow-response (0x78 pending) demonstration
cansend vcan0 7E0#03.22.DE.AF

# Watch traffic
candump vcan0
```

## Known limitations

- Server does not override P2/P2* per-session in its `DiagSessCtrl`
  response (fields available in `UDSDiagSessCtrlArgs_t` but left at
  library defaults)
- Client's `UDS_EVT_Err` handling distinguishes timeout from other errors
  (retries only on timeout) but does not branch on specific NRC codes
  beyond that

## Requirements

- Linux kernel with ISO-TP support (`CONFIG_CAN_ISOTP`) — custom-built for
  WSL2, see Prerequisite section above
- SocketCAN interface
- `can-utils` (`cansend`, `candump`)

## License

`iso14229.c` / `iso14229.h` retain their original MIT license — see `LICENSE`.
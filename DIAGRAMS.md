# System Architecture Diagrams

## High-Level System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        BOOT SEQUENCE                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  1. Power On                                                     │
│  2. Serial Init (115200 baud)                                   │
│  3. OTA Check (WiFi temporarily enabled)                        │
│       ├─ Connect to open WiFi                                   │
│       ├─ Fetch version from server                              │
│       ├─ Compare version (v17)                                  │
│       └─ Download & flash if newer                              │
│  4. WiFi Disconnect                                             │
│  5. BLE Init                                                     │
│       ├─ CONTROLLER: Start as Client                            │
│       └─ NODE: Start as Server                                  │
│  6. Begin Stimulation                                           │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

## BLE Connection Architecture

```
┌──────────────────────────┐                    ┌──────────────────────────┐
│      CONTROLLER          │                    │          NODE            │
│    (BLE Client)          │                    │      (BLE Server)        │
├──────────────────────────┤                    ├──────────────────────────┤
│                          │                    │                          │
│  Setup:                  │                    │  Setup:                  │
│  • Init BLE              │                    │  • Init BLE              │
│  • Set callback          │                    │  • Set callback          │
│  • Start scanning ──────────────────────────► │  • Start advertising     │
│                          │                    │    as "CMCO_NODE"        │
│  • Find "CMCO_NODE" ◄────────────────────────────                        │
│  • Connect ─────────────────────────────────► │  • Accept connection     │
│  • Get service           │                    │                          │
│  • Subscribe to notify   │                    │                          │
│  • Ready ✓               │                    │  • Ready ✓               │
│                          │                    │                          │
│  Loop:                   │                    │  Loop:                   │
│  • Generate pattern      │                    │  • Wait for sync         │
│  • Send SyncPacket ─────────────────────────► │  • Receive SyncPacket    │
│                          │  [~284 bytes]      │  • Update pattern        │
│                          │                    │  • Send AckPacket ◄──────┤
│  • Receive AckPacket ◄───────────────────────    [8 bytes]              │
│  • Calculate RTT         │                    │                          │
│  • Update sync offset    │                    │                          │
│  • Update stimulation    │                    │  • Update stimulation    │
│                          │                    │                          │
│  If disconnected:        │                    │  If disconnected:        │
│  • Auto re-scan          │                    │  • Restart advertising   │
│  • Auto reconnect        │                    │  • Wait for connection   │
│                          │                    │                          │
└──────────────────────────┘                    └──────────────────────────┘
```

## Data Flow Diagram

```
CONTROLLER                                                    NODE
─────────────────────────────────────────────────────────────────────

┌─────────────────┐
│  Generate       │
│  New Pattern    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Prepare        │
│  SyncPacket     │  • t_send_us (timestamp)
│                 │  • stimPeriods[20] (pattern data)
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ BleSync::send() │
└────────┬────────┘
         │
         │  [BLE Write Without Response]
         │  UUID: 12340003-B5A3-...
         │
         └──────────────────────────────────►  ┌─────────────────┐
                                                │ RX Callback     │
                                                │ Triggered       │
                                                └────────┬────────┘
                                                         │
                                                         ▼
                                                ┌─────────────────┐
                                                │ onSyncReceive() │
                                                │ • Parse packet  │
                                                │ • Update stim   │
                                                │ • Record time   │
                                                └────────┬────────┘
                                                         │
                                                         ▼
                                                ┌─────────────────┐
                                                │  Prepare        │
                                                │  AckPacket      │
                                                │ • t_recv_us     │
                                                │ • t_send_us     │
                                                └────────┬────────┘
                                                         │
                                                         ▼
                                                ┌─────────────────┐
                                                │ BleSync::send() │
                                                └────────┬────────┘
                                                         │
                 ┌────────────────────────────────────────┘
                 │  [BLE Notification]
                 │  UUID: 12340002-B5A3-...
                 │
                 ▼
┌─────────────────┐
│ Notify Callback │
│ Triggered       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  onAckReceive() │
│  • Parse packet │
│  • Calculate RTT│
│  • Update offset│
└─────────────────┘
```

## BLE Service Structure

```
┌───────────────────────────────────────────────────────────────────┐
│                    CMCO BLE Services                              │
├───────────────────────────────────────────────────────────────────┤
│                                                                    │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │  Sync Service (Controller ↔ Node)                          │ │
│  │  UUID: 12340001-B5A3-F393-E0A9-E50E24DCCA9E                │ │
│  ├─────────────────────────────────────────────────────────────┤ │
│  │                                                             │ │
│  │  ┌─────────────────────────────────────────────────────┐  │ │
│  │  │ TX Characteristic (Server → Client)                 │  │ │
│  │  │ UUID: 12340002-B5A3-F393-E0A9-E50E24DCCA9E         │  │ │
│  │  │ Properties: NOTIFY                                  │  │ │
│  │  │ Used for: AckPacket from Node to Controller        │  │ │
│  │  └─────────────────────────────────────────────────────┘  │ │
│  │                                                             │ │
│  │  ┌─────────────────────────────────────────────────────┐  │ │
│  │  │ RX Characteristic (Client → Server)                 │  │ │
│  │  │ UUID: 12340003-B5A3-F393-E0A9-E50E24DCCA9E         │  │ │
│  │  │ Properties: WRITE, WRITE_WITHOUT_RESPONSE          │  │ │
│  │  │ Used for: SyncPacket from Controller to Node       │  │ │
│  │  └─────────────────────────────────────────────────────┘  │ │
│  │                                                             │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                                                                    │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │  UART Service (Optional - iPhone App)                      │ │
│  │  UUID: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E                │ │
│  ├─────────────────────────────────────────────────────────────┤ │
│  │                                                             │ │
│  │  ┌─────────────────────────────────────────────────────┐  │ │
│  │  │ TX Characteristic (Device → Phone)                  │  │ │
│  │  │ UUID: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E         │  │ │
│  │  │ Properties: NOTIFY                                  │  │ │
│  │  │ Used for: Status messages to iPhone                │  │ │
│  │  └─────────────────────────────────────────────────────┘  │ │
│  │                                                             │ │
│  │  ┌─────────────────────────────────────────────────────┐  │ │
│  │  │ RX Characteristic (Phone → Device)                  │  │ │
│  │  │ UUID: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E         │  │ │
│  │  │ Properties: WRITE, WRITE_WITHOUT_RESPONSE          │  │ │
│  │  │ Used for: Commands from iPhone                     │  │ │
│  │  └─────────────────────────────────────────────────────┘  │ │
│  │                                                             │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                                                                    │
└───────────────────────────────────────────────────────────────────┘
```

## Packet Structure Diagram

```
┌────────────────────────────────────────────────────────────────┐
│                        SyncPacket                              │
│                      (Total: ~284 bytes)                       │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────────┐                                 │
│  │  t_send_us (4 bytes)     │  Timestamp in microseconds      │
│  │  uint32_t                │                                 │
│  └──────────────────────────┘                                 │
│                                                                 │
│  ┌──────────────────────────┐                                 │
│  │ stimPeriods[0]           │ ┐                               │
│  ├──────────────────────────┤ │                               │
│  │ stimPeriods[1]           │ │                               │
│  ├──────────────────────────┤ │                               │
│  │ stimPeriods[2]           │ │                               │
│  ├──────────────────────────┤ │  20 Stimulation              │
│  │       ...                │ │  Periods                      │
│  ├──────────────────────────┤ │  (~280 bytes)                │
│  │ stimPeriods[18]          │ │                               │
│  ├──────────────────────────┤ │                               │
│  │ stimPeriods[19]          │ ┘                               │
│  └──────────────────────────┘                                 │
│                                                                 │
└────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────┐
│                         AckPacket                              │
│                       (Total: 8 bytes)                         │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────────┐                                 │
│  │  t_recv_us (4 bytes)     │  Node receive timestamp         │
│  │  uint32_t                │                                 │
│  └──────────────────────────┘                                 │
│                                                                 │
│  ┌──────────────────────────┐                                 │
│  │  t_send_us (4 bytes)     │  Original Controller timestamp  │
│  │  uint32_t                │                                 │
│  └──────────────────────────┘                                 │
│                                                                 │
└────────────────────────────────────────────────────────────────┘
```

## Timing Diagram

```
Time →
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

CONTROLLER:
    │
    ├─ Generate Pattern (1ms)
    │
    ├─ Send SyncPacket ──────────────┐
    │  t_send = 1000000µs             │
    │                                  │  BLE Transmission
    │                                  │  (~5-10ms)
    │                                  │
    │                                  ▼
    │                           ┌─────────────┐
    │                           │ NODE Process │
    │                           │ (~1-2ms)     │
    │                           └─────────────┘
    │                                  │
    │                                  │  BLE Transmission
    │                                  │  (~5-10ms)
    │                                  │
    ├─ Receive AckPacket ◄─────────────┘
    │  t_now = 1025000µs
    │
    ├─ Calculate RTT = 25000µs (25ms)
    │  One-way ≈ 12500µs
    │
    ├─ Update Sync Offset
    │
    └─ Continue...


NODE:
    │
    ├─ Wait for Sync...
    │
    ├─ Receive SyncPacket ◄─────┐
    │  t_recv = 1012000µs        │
    │                            │
    ├─ Parse & Update (1ms)      │
    │                            │
    ├─ Send AckPacket ──────────┘
    │  (t_send=1000000, t_recv=1012000)
    │
    ├─ Execute Stimulation
    │
    └─ Continue...
```

## State Machine Diagram

```
CONTROLLER States:
┌─────────┐
│  INIT   │
└────┬────┘
     │
     ▼
┌─────────┐      Not Found      ┌─────────┐
│SCANNING │─────────────────────►│ WAITING │
└────┬────┘                      └────┬────┘
     │                                │
     │ Found NODE                     │ Retry
     │                                │ (5s)
     ▼                                │
┌──────────┐     Failed         ┌────┴────┐
│CONNECTING│────────────────────►│  ERROR  │
└────┬─────┘                     └─────────┘
     │
     │ Success
     │
     ▼
┌──────────┐
│CONNECTED │◄───────┐
└────┬─────┘        │
     │              │
     │ Send Sync    │ Pattern Complete
     │              │ (Send new sync)
     ▼              │
┌──────────┐        │
│  SYNCED  │────────┘
└────┬─────┘
     │
     │ Disconnect
     │
     ▼
┌──────────┐
│  RETRY   │───────► Back to SCANNING
└──────────┘


NODE States:
┌─────────┐
│  INIT   │
└────┬────┘
     │
     ▼
┌──────────┐
│ADVERTISING│
└────┬─────┘
     │
     │ Client Connects
     │
     ▼
┌──────────┐
│ WAITING  │
└────┬─────┘
     │
     │ Receive Sync
     │
     ▼
┌──────────┐
│  SYNCED  │◄───────┐
└────┬─────┘        │
     │              │
     │ Execute      │ Receive
     │ Pattern      │ New Sync
     │              │
     ▼              │
┌──────────┐        │
│  ACTIVE  │────────┘
└────┬─────┘
     │
     │ Disconnect
     │
     ▼
┌──────────┐
│ADVERTISING│───────► Back to start
└──────────┘
```

## Memory Layout

```
ESP32-S3 Memory Map (4MB Flash):
┌────────────────────────────────────┐ 0x000000
│   Bootloader (32KB)                │
├────────────────────────────────────┤ 0x008000
│   Partition Table (4KB)            │
├────────────────────────────────────┤ 0x009000
│   NVS (20KB)                       │
├────────────────────────────────────┤ 0x00E000
│   OTA Data (8KB)                   │
├────────────────────────────────────┤ 0x010000
│   App0 (1.9MB) ← Current Firmware  │
│   • BLE Stack (~200KB)             │
│   • Application Code (~100KB)      │
│   • NimBLE Library (~200KB)        │
│   • Arduino Core (~300KB)          │
│   • Unused (~1100KB)               │
├────────────────────────────────────┤ 0x1F0000
│   App1 (1.9MB) ← OTA Upload        │
│   • Next firmware version          │
├────────────────────────────────────┤ 0x3D0000
│   SPIFFS (190KB) ← Optional        │
│   • Config files                   │
│   • Data storage                   │
└────────────────────────────────────┘ 0x3FFFFF

RAM Usage:
┌────────────────────────────────────┐
│ Total SRAM: 512KB                  │
├────────────────────────────────────┤
│ BLE Stack: ~50KB                   │
│ Application: ~30KB                 │
│ Arduino Core: ~20KB                │
│ Free Heap: ~400KB                  │
└────────────────────────────────────┘
```

---

These diagrams show the complete system architecture from boot to operation.

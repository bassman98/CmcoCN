# BLE Migration Guide

## Overview
This project has been migrated from **ESP-NOW** to **BLE (Bluetooth Low Energy)** for communication between the Controller and Node devices, while maintaining full OTA (Over-The-Air) update functionality.

## What Changed

### Architecture Changes
- **Communication Protocol**: ESP-NOW → BLE (NimBLE library)
- **Connection Model**: 
  - Controller acts as **BLE Client** (scans and connects to Node)
  - Node acts as **BLE Server** (advertises and waits for Controller)
- **Buffering**: Implemented message buffering using C++ `std::queue`
- **OTA**: Preserved and fully compatible with BLE (uses WiFi temporarily during boot)

## Key Features

### 1. BLE Synchronization (`ble_sync.h`)
- Custom BLE service for controller-node synchronization
- Separate UUIDs for sync (controller-node) and UART (iPhone connectivity)
- Buffered message handling for reliable communication
- Automatic reconnection logic
- Low-latency connection parameters optimized for real-time sync

### 2. Dual BLE Support
The system now supports TWO independent BLE connections:
- **Sync Connection**: Between Controller and Node (ble_sync.h)
- **iPhone Connection**: For external control/monitoring (ble_iphone.h)

### 3. OTA Compatibility
- OTA check happens at boot before BLE initialization
- WiFi is enabled temporarily, then disabled after OTA check
- BLE starts cleanly after OTA verification
- Firmware version incremented to v17

## File Changes

### New Files
- **`ble_sync.h`**: Complete BLE synchronization implementation with buffering

### Modified Files
- **`controllernode.ino`**: 
  - Removed ESP-NOW includes and functions
  - Added BLE initialization and connection handling
  - Implemented auto-reconnection in main loop
  - Updated packet send/receive callbacks for BLE
  
- **`config.h`**:
  - Added `NODE_BLE_NAME` constant for Controller to discover Node
  - Added BLE Sync Service UUIDs (separate from iPhone UART UUIDs)
  - Updated firmware version to 17

## Configuration

### Controller Configuration
In `config.h`, define:
```cpp
#define CONTROLLER
```

The controller will:
1. Initialize BLE as a client
2. Scan for devices named "CMCO_NODE"
3. Connect automatically when found
4. Send synchronization packets
5. Receive acknowledgment packets

### Node Configuration
In `config.h`, define:
```cpp
#define NODE
```

The node will:
1. Initialize BLE as a server
2. Advertise as "CMCO_NODE"
3. Wait for controller connection
4. Receive synchronization packets
5. Send acknowledgment packets

## BLE Service UUIDs

### Sync Service (Controller ↔ Node)
- **Service**: `12340001-B5A3-F393-E0A9-E50E24DCCA9E`
- **TX Characteristic**: `12340002-B5A3-F393-E0A9-E50E24DCCA9E` (Notify)
- **RX Characteristic**: `12340003-B5A3-F393-E0A9-E50E24DCCA9E` (Write)

### UART Service (iPhone Connection)
- **Service**: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` (Nordic UART)
- **TX Characteristic**: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
- **RX Characteristic**: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`

## Arduino IDE Setup

### Required Libraries
Install these libraries via Arduino IDE Library Manager:
1. **NimBLE-Arduino** by h2zero (version 1.4.0 or later)
2. **ArduinoJson** (for OTA)

### Board Configuration
- Board: **Adafruit QT Py ESP32-S3**
- Partition Scheme: **Minimal SPIFFS (Large APP with OTA)**
- Flash Size: 4MB
- PSRAM: 2MB

## Usage

### Building & Flashing
1. Open `controllernode.ino` in Arduino IDE
2. For Controller: Set `#define CONTROLLER` in `config.h`
3. For Node: Set `#define NODE` in `config.h`
4. Select correct board and port
5. Upload firmware

### Testing Connection
1. Power on the **Node** first (it will start advertising)
2. Power on the **Controller** (it will scan and connect)
3. Watch Serial Monitor for connection status:
   - Node: "Starting BLE in NODE mode (server)"
   - Controller: "Starting BLE in CONTROLLER mode (client)"
   - Controller: "Connected! Starting pattern..."

### Serial Monitor Output
**Controller:**
```
Controller: Waiting for BLE connection to NODE...
[BLE Sync] Starting scan...
[BLE Sync] Found target NODE
[BLE Sync] Connected! Getting service...
[BLE Sync] Ready for communication
Connected! Starting pattern...
Sync Packet Sent via BLE
```

**Node:**
```
Node: Waiting for CONTROLLER connection...
Starting BLE in NODE mode (server)
[BLE Sync] Server started, advertising as: CMCO_NODE
[BLE Sync] Client connected
Sync Packet Received via BLE
```

## Advantages of BLE vs ESP-NOW

### Benefits
1. **Standard Protocol**: BLE is universally supported
2. **Better Range**: Can achieve similar or better range with proper antenna
3. **Dual Connections**: Can maintain sync + iPhone connections simultaneously
4. **Security**: BLE supports pairing and encryption (can be added)
5. **Phone Compatibility**: Direct iPhone/Android connectivity
6. **Lower Power**: BLE can be more power-efficient with proper sleep modes

### Considerations
1. **Latency**: Slightly higher than ESP-NOW (~10-20ms vs ~5ms)
2. **Connection Overhead**: Requires scan/connect phase
3. **Library Size**: NimBLE adds ~200KB to firmware size

## Troubleshooting

### Controller Can't Find Node
- Ensure Node is powered on first
- Check that both devices have correct firmware (v17+)
- Verify `NODE_BLE_NAME` matches in config.h
- Check serial monitor for scan results

### Connection Drops Frequently
- Reduce distance between devices
- Check for WiFi interference (2.4GHz)
- Increase BLE TX power (already at max: `ESP_PWR_LVL_P9`)
- Check battery voltage (low voltage can cause instability)

### OTA Not Working
- Ensure WiFi credentials are correct
- Check that device can reach update server
- Verify firmware version number is incremented on server
- OTA happens at boot before BLE, so it won't interfere

## Power Saver Mode
The `POWER_SAVER` mode is supported but simplified:
- BLE maintains connection during light sleep
- No need to reinitialize BLE after wake (unlike ESP-NOW)
- Sleep duration configured by `ESP_SLEEP` constant

## Future Enhancements
- [ ] Add BLE bonding/pairing for security
- [ ] Implement connection parameter negotiation
- [ ] Add multiple node support (one controller, many nodes)
- [ ] Optimize MTU size for larger packets
- [ ] Add BLE mesh networking capability

## Migration from ESP-NOW Projects
If you have other ESP-NOW projects to migrate:
1. Copy `ble_sync.h` to your project
2. Add BLE UUIDs to your config
3. Replace `esp_now_send()` with `BleSync::send()`
4. Replace ESP-NOW callbacks with BLE callbacks
5. Add `BleSync::update()` to your main loop
6. Initialize with `BleSync::init()` and appropriate server/client start

## Support
For issues, refer to:
- NimBLE-Arduino documentation: https://github.com/h2zero/NimBLE-Arduino
- ESP32 BLE documentation: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/index.html

---
**Version**: 17  
**Last Updated**: February 2026  
**Author**: CMCO Development Team

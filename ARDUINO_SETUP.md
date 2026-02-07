# Quick Start Guide - Arduino IDE Setup

## Prerequisites
- Arduino IDE 2.x or 1.8.19+
- ESP32 board support installed
- USB-C cable for QT Py ESP32-S3

## Step 1: Install ESP32 Board Support

### Arduino IDE 2.x:
1. Go to **File → Preferences**
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search for "esp32"
5. Install **"esp32 by Espressif Systems"** (version 2.0.11 or later recommended)

### Arduino IDE 1.8.x:
Same steps as above, but menu locations may differ slightly.

## Step 2: Install Required Libraries

Go to **Tools → Manage Libraries** (or Sketch → Include Library → Manage Libraries)

### Required Libraries:
1. **NimBLE-Arduino** by h2zero
   - Search: "NimBLE"
   - Version: 1.4.0 or later
   - Click **Install**

2. **ArduinoJson** by Benoit Blanchon
   - Search: "ArduinoJson"
   - Version: 6.21.0 or later
   - Click **Install**

## Step 3: Board Configuration

### Select Board:
1. Go to **Tools → Board → ESP32 Arduino**
2. Select: **"Adafruit QT Py ESP32-S3 (no PSRAM)"** or **"ESP32S3 Dev Module"**

### Configure Settings:

#### If using "Adafruit QT Py ESP32-S3":
- USB CDC On Boot: **Enabled**
- CPU Frequency: **240MHz (WiFi)**
- Flash Size: **4MB (32Mb)**
- Partition Scheme: **Minimal SPIFFS (Large APP with OTA)** ⚠️ IMPORTANT
- PSRAM: **OPI PSRAM** (if available) or **Disabled**

#### If using "ESP32S3 Dev Module":
- USB CDC On Boot: **Enabled**
- CPU Frequency: **240MHz (WiFi)**
- Flash Mode: **QIO 80MHz**
- Flash Size: **4MB (32Mb)**
- Partition Scheme: **Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)** ⚠️ IMPORTANT
- PSRAM: **OPI PSRAM** or **Disabled**
- Upload Speed: **921600**

### Select Port:
- **Windows**: COM3, COM4, etc.
- **Mac/Linux**: /dev/ttyUSB0, /dev/cu.usbserial, etc.

## Step 4: Configure Device Role

Open `config.h` and set the device role:

### For Controller Device:
```cpp
// Role
#define CONTROLLER
//#define NODE
```

### For Node Device:
```cpp
// Role
//#define CONTROLLER
#define NODE
```

### Optional Features:
```cpp
// Other
#define BLUETOOTH     // Enable iPhone BLE connectivity
//#define POWER_SAVER  // Enable power saving mode
```

## Step 5: Compile and Upload

1. Click **Verify** (✓) to compile
2. Wait for compilation (first time takes longer)
3. If successful, click **Upload** (→)
4. Wait for upload to complete
5. Open **Serial Monitor** (115200 baud) to see output

## Expected Serial Output

### Controller Output:
```
QT Py ESP32-S3 OTA Boot
Current FW Version: 17
WiFi unavailable, running current firmware
Starting BLE in CONTROLLER mode (client)
[BLE Sync] Initialized
BLE Device Address: 12:34:56:78:9a:bc
Controller: Waiting for BLE connection to NODE...
[BLE Sync] Starting scan...
[BLE Sync] Found device: CMCO_NODE
[BLE Sync] Found target NODE
[BLE Sync] Connecting to device...
[BLE Sync] Connected! Getting service...
[BLE Sync] Ready for communication
Connected! Starting pattern...
Sync Packet Sent via BLE
ACK received, RTT: 15234 us
```

### Node Output:
```
QT Py ESP32-S3 OTA Boot
Current FW Version: 17
WiFi unavailable, running current firmware
Starting BLE in NODE mode (server)
[BLE Sync] Initialized
[BLE Sync] Server started, advertising as: CMCO_NODE
BLE Device Address: aa:bb:cc:dd:ee:ff
Node: Waiting for CONTROLLER connection...
[BLE Sync] Client connected
Sync Packet Received via BLE
```

## Troubleshooting

### Compilation Errors

**Error: "NimBLEDevice.h: No such file"**
- Solution: Install NimBLE-Arduino library (see Step 2)

**Error: "ArduinoJson.h: No such file"**
- Solution: Install ArduinoJson library (see Step 2)

**Error: "Sketch too big"**
- Solution: Change Partition Scheme to "Minimal SPIFFS (Large APP with OTA)"

### Upload Errors

**Error: "Failed to connect to ESP32"**
- Hold BOOT button while clicking Upload
- Try different USB cable
- Check port selection
- Lower upload speed to 115200

**Error: "Timed out waiting for packet header"**
- Press RESET button
- Disconnect and reconnect USB
- Try putting device in bootloader mode (hold BOOT, press RESET, release BOOT)

### Runtime Issues

**Serial Monitor shows garbage characters**
- Set baud rate to **115200**
- Check "Both NL & CR" line ending

**Controller can't find Node**
- Flash Node firmware first
- Power on Node before Controller
- Check that NODE_BLE_NAME matches in both firmwares
- Ensure both devices are v17 firmware

**BLE connection unstable**
- Keep devices within 10 meters
- Move away from WiFi routers
- Check power supply (use USB power, not battery initially)

## Building for Production

### Optimizations:
```cpp
// In config.h, disable debugging features:
//#define BLUETOOTH  // Disable if not using iPhone
//#define POWER_SAVER // Enable for battery operation
```

### OTA Configuration:
Ensure your OTA server is configured correctly:
- Update `STATUS_JS_URL` in `config.h`
- Increment `FW_VERSION` for each release
- Upload new firmware binary to your server

## Library Versions Tested

| Library | Version | Status |
|---------|---------|--------|
| NimBLE-Arduino | 1.4.1 | ✅ Tested |
| ArduinoJson | 6.21.4 | ✅ Tested |
| ESP32 Core | 2.0.14 | ✅ Tested |
| ESP32 Core | 3.0.0 | ⚠️ Not tested |

## Additional Resources

- **NimBLE Documentation**: https://github.com/h2zero/NimBLE-Arduino
- **ESP32 Arduino Core**: https://github.com/espressif/arduino-esp32
- **Adafruit QT Py Guide**: https://learn.adafruit.com/adafruit-qt-py-esp32-s3

## Next Steps

After successful upload:
1. Build and upload to both Controller and Node
2. Test basic connectivity
3. Configure OTA server for remote updates
4. Integrate with your iPhone app (if using BLUETOOTH feature)
5. Optimize power consumption for battery operation

---
**Need Help?** Check the [BLE_MIGRATION.md](BLE_MIGRATION.md) file for detailed technical information.

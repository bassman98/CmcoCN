# Troubleshooting Guide

## Quick Diagnostics

### Is it working correctly?
Look for these indicators:
- ✅ Controller shows "Connected! Starting pattern..."
- ✅ Node shows "Sync Packet Received via BLE"
- ✅ Controller shows "ACK received, RTT: XXXXX us"
- ✅ Both devices show BLE address
- ✅ Stimulation patterns are synchronized

---

## Common Issues & Solutions

### 1. Controller Can't Find Node

#### Symptoms:
```
[BLE Sync] Starting scan...
[BLE Sync] Found device: <other devices>
No target device found
```

#### Causes & Solutions:

**A. Node not powered on**
- ✅ Solution: Power on Node BEFORE Controller
- ✅ Node should show "Server started, advertising as: CMCO_NODE"

**B. Node firmware incorrect**
- ✅ Check Node serial monitor for "Starting BLE in NODE mode"
- ✅ Verify config.h has `#define NODE` (not CONTROLLER)
- ✅ Re-upload correct firmware to Node

**C. Device names don't match**
- ✅ Check config.h: `DEVICE_BLE_NAME` and `NODE_BLE_NAME`
- ✅ Both should use firmware v17
- ✅ Re-compile if names were changed

**D. Range too far**
- ✅ Move devices within 5 meters for initial connection
- ✅ Clear line of sight
- ✅ Remove metal objects between devices

**E. BLE interference**
- ✅ Turn off other BLE devices nearby
- ✅ Move away from WiFi routers
- ✅ Try in different location

#### Debug Steps:
```cpp
// Add to Controller code temporarily:
void onResult(NimBLEAdvertisedDevice* dev) {
    Serial.print("Found: ");
    Serial.println(dev->getName().c_str());
}
```

---

### 2. Connection Drops Frequently

#### Symptoms:
```
[BLE Sync] Client disconnected
Connection lost, attempting to reconnect...
```

#### Causes & Solutions:

**A. Low battery voltage**
- ✅ Use USB power for testing
- ✅ Check battery voltage (should be >3.3V)
- ✅ Replace batteries if below 3.5V

**B. Too much distance**
- ✅ Test at 2-3 meters first
- ✅ Gradually increase distance
- ✅ Maximum reliable range: ~10m indoor

**C. Obstacles/interference**
- ✅ Remove metal objects
- ✅ Avoid thick walls
- ✅ Test in open area first

**D. Power management issues**
- ✅ Disable POWER_SAVER for testing
- ✅ Comment out `#define POWER_SAVER` in config.h

**E. BLE stack overflow**
- ✅ Reduce packet send frequency
- ✅ Add delay between sync packets
- ✅ Check free heap: `Serial.println(ESP.getFreeHeap());`

#### Debug Code:
```cpp
// Add to loop():
if (millis() % 5000 == 0) {
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    Serial.print("BLE Connected: ");
    Serial.println(BleSync::isConnected(bleSyncCtx));
}
```

---

### 3. Compilation Errors

#### Error: "NimBLEDevice.h: No such file or directory"

**Solution:**
```
1. Open Arduino IDE
2. Tools → Manage Libraries
3. Search "NimBLE"
4. Install "NimBLE-Arduino" by h2zero
5. Restart Arduino IDE
6. Try compiling again
```

#### Error: "Sketch too big"

**Current size:** ~500KB  
**Required:** 1.9MB partition

**Solution:**
```
1. Tools → Partition Scheme
2. Select "Minimal SPIFFS (Large APP with OTA)"
3. This provides 1.9MB for app
4. Compile again
```

#### Error: "undefined reference to `BleSync::init`"

**Cause:** Missing include or wrong file location

**Solution:**
```cpp
// Ensure ble_sync.h is in same folder as .ino
// Check #include at top of controllernode.ino:
#include "ble_sync.h"
```

#### Error: "'NODE_BLE_NAME' was not declared in this scope"

**Cause:** Old config.h file

**Solution:**
```cpp
// Add to config.h under CONTROLLER section:
#ifdef CONTROLLER
static constexpr char NODE_BLE_NAME[] = "CMCO_NODE";
#endif
```

---

### 4. Upload Errors

#### Error: "A fatal error occurred: Failed to connect to ESP32"

**Solutions:**

**Method 1: Boot Mode**
```
1. Hold BOOT button
2. Click Upload in Arduino IDE
3. Release BOOT when "Connecting..." appears
```

**Method 2: Manual Reset**
```
1. Press and hold BOOT
2. Press and release RESET
3. Release BOOT
4. Click Upload immediately
```

**Method 3: USB Cable**
```
1. Try different USB cable (must support data)
2. Try different USB port
3. Avoid USB hubs
```

**Method 4: Driver Issues**
```
Windows:
- Install CP210x or CH340 drivers
- Check Device Manager

Mac:
- Install driver from Silicon Labs or WCH

Linux:
- Add user to dialout group:
  sudo usermod -a -G dialout $USER
```

---

### 5. Serial Monitor Issues

#### Garbage Characters

**Cause:** Wrong baud rate

**Solution:**
```
1. Set Serial Monitor to 115200 baud
2. Check line ending: "Both NL & CR"
3. If still garbage, press RESET button
```

#### No Output

**Cause:** USB CDC not enabled

**Solution:**
```
1. Tools → USB CDC On Boot → Enabled
2. Re-upload firmware
3. Wait 2 seconds after upload
4. Open Serial Monitor
```

#### Output Stops After Boot

**Cause:** Serial disconnected

**Solution:**
```
1. Press RESET button
2. Or reconnect Serial Monitor
3. Check USB cable connection
```

---

### 6. OTA Update Issues

#### OTA Check Fails

**Symptoms:**
```
WiFi unavailable, running current firmware
```

**Causes & Solutions:**

**A. No WiFi available**
- ✅ Expected behavior if no WiFi
- ✅ Device will run normally without OTA
- ✅ Add a known open WiFi for testing

**B. WiFi password wrong**
- ✅ Check `cmco_password` in ota.h
- ✅ Default: "cmco123456789!"
- ✅ Update if your network uses different password

**C. Server unreachable**
- ✅ Check `STATUS_JS_URL` in config.h
- ✅ Test URL in web browser
- ✅ Ensure server is online

#### OTA Downloads But Doesn't Flash

**Symptoms:**
```
OTA failed, continuing current firmware
```

**Solutions:**
```
1. Check partition scheme (needs OTA support)
2. Verify firmware file size < 1.9MB
3. Check firmware URL is accessible
4. Ensure firmware is for correct device
```

---

### 7. Stimulation Not Working

#### No Stimulation Output

**Checks:**

**A. Verify PWM Pins**
```cpp
// Add to setup():
Serial.println("Testing PWM outputs...");
for (int i = 0; i < NUM_FINGERS; i++) {
    Serial.print("Pin ");
    Serial.print(PWM_PINS[i]);
    Serial.print(": ");
    ledcAttach(PWM_PINS[i], 300, PWM_RESOLUTION);
    ledcWrite(PWM_PINS[i], 511);
    delay(500);
    ledcWrite(PWM_PINS[i], 0);
    Serial.println("OK");
}
```

**B. Check Hardware**
- ✅ Verify wiring to motors/actuators
- ✅ Check power supply to actuators
- ✅ Test with multimeter on PWM pins
- ✅ Ensure motors are not damaged

**C. Pattern Issue**
- ✅ Check stimulation parameters in config.h
- ✅ Verify pattern generation logic
- ✅ Add debug output in stim.update()

---

### 8. iPhone Connection Issues (if BLUETOOTH enabled)

#### iPhone Can't See Device

**Solutions:**
```
1. Ensure #define BLUETOOTH is uncommented
2. Verify iPhone BLE is ON
3. Forget device in iPhone settings
4. Restart both devices
5. Check device name in iOS Settings → Bluetooth
```

#### iPhone Connects But No Data

**Debug:**
```cpp
// Add to loop():
if (BleIphone::isConnected(bleIphoneCtx)) {
    static uint32_t lastSend = 0;
    if (millis() - lastSend > 1000) {
        BleIphone::write(bleIphoneCtx, "TEST\n");
        lastSend = millis();
    }
}
```

---

### 9. Performance Issues

#### High Latency (>50ms)

**Causes & Solutions:**

**A. BLE connection parameters**
```cpp
// In ble_sync.h, ConnectionCallbacks:
pClient->updateConnParams(6, 6, 0, 60);
// Try even lower (faster, more power):
pClient->updateConnParams(6, 6, 0, 100);
```

**B. Packet too large**
```
Current SyncPacket: ~284 bytes
BLE MTU default: 23 bytes
Solution: Packets are automatically fragmented
Consider reducing NUM_PERIODS if needed
```

**C. Too much processing**
```cpp
// Optimize callbacks - avoid Serial.print in time-critical paths
// Move Serial output outside callbacks
```

#### Sync Drift

**Symptoms:** Patterns become unsynchronized over time

**Solutions:**
```cpp
// Increase alpha in updateSyncDelay():
constexpr float alpha = 0.2f;  // More responsive (was 0.1)

// Or send sync more frequently:
// Reduce pattern duration in config.h
```

---

### 10. Memory Issues

#### Stack Overflow / Crashes

**Symptoms:**
```
Guru Meditation Error
Backtrace: 0x... 0x...
```

**Solutions:**

**A. Reduce queue size**
```cpp
// In ble_sync.h, limit queue size:
if (g_ctx->rxBuffer.size() > 10) {
    g_ctx->rxBuffer.pop(); // Drop old packets
}
```

**B. Monitor heap**
```cpp
void loop() {
    static uint32_t lastCheck = 0;
    if (millis() - lastCheck > 1000) {
        uint32_t free = ESP.getFreeHeap();
        if (free < 50000) {
            Serial.print("WARNING: Low heap: ");
            Serial.println(free);
        }
        lastCheck = millis();
    }
}
```

**C. Reduce BLE stack size** (advanced)
```
menuconfig → Component config → Bluetooth → NimBLE
Reduce connection count, service count
```

---

## Diagnostic Commands

### Add these to your code for debugging:

```cpp
// In setup(), after BLE init:
void printDiagnostics() {
    Serial.println("\n=== DIAGNOSTICS ===");
    Serial.print("Chip Model: ");
    Serial.println(ESP.getChipModel());
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    Serial.print("Flash Size: ");
    Serial.println(ESP.getFlashChipSize());
    Serial.print("BLE Address: ");
    Serial.println(NimBLEDevice::getAddress().toString().c_str());
    
    #ifdef CONTROLLER
    Serial.println("Role: CONTROLLER (Client)");
    #endif
    #ifdef NODE
    Serial.println("Role: NODE (Server)");
    #endif
    
    Serial.print("Firmware Version: ");
    Serial.println(FW_VERSION);
    Serial.println("===================\n");
}
```

### Call in setup():
```cpp
void setup() {
    Serial.begin(115200);
    delay(1500);
    printDiagnostics();
    // ... rest of setup
}
```

---

## Getting Help

### Before Asking for Help

Collect this information:

1. **Hardware:**
   - Board type: ________________
   - USB or battery power: ________________
   - Distance between devices: ________________

2. **Software:**
   - Arduino IDE version: ________________
   - ESP32 core version: ________________
   - NimBLE version: ________________
   - Firmware version: ________________

3. **Configuration:**
   ```cpp
   // From config.h:
   #define CONTROLLER  or  #define NODE
   #define BLUETOOTH   (yes/no)
   #define POWER_SAVER (yes/no)
   ```

4. **Serial Output:**
   ```
   Paste last 50 lines from Serial Monitor
   Include output from BOTH devices if possible
   ```

5. **Symptoms:**
   - What you expected: ________________
   - What actually happened: ________________
   - When it started: ________________

### Where to Get Help

1. **Check Documentation:**
   - [BLE_MIGRATION.md](BLE_MIGRATION.md) - Technical details
   - [ARDUINO_SETUP.md](ARDUINO_SETUP.md) - Setup guide
   - [COMPARISON.md](COMPARISON.md) - ESP-NOW vs BLE
   - [DIAGRAMS.md](DIAGRAMS.md) - System architecture

2. **Test with Examples:**
   - NimBLE-Arduino examples
   - Arduino ESP32 BLE examples

3. **Community Support:**
   - ESP32 forum: https://esp32.com
   - Arduino forum: https://forum.arduino.cc
   - NimBLE GitHub Issues

---

## Reset to Factory Settings

If all else fails, try a complete reset:

```cpp
// Add to setup() temporarily:
void factoryReset() {
    Serial.println("Factory reset...");
    NimBLEDevice::deinit(true);
    nvs_flash_erase();
    nvs_flash_init();
    ESP.restart();
}
```

---

## Advanced Debugging

### Enable BLE Debug Logging

```cpp
// Add before NimBLEDevice::init():
NimBLEDevice::setSecurityAuth(true, true, true);
NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_DEFAULT);
```

### Monitor BLE Stack

```cpp
// In loop():
if (millis() % 10000 == 0) {
    Serial.print("BLE Connected: ");
    Serial.println(BleSync::isConnected(bleSyncCtx));
    Serial.print("Server Connections: ");
    if (bleSyncCtx.server) {
        Serial.println(bleSyncCtx.server->getConnectedCount());
    }
    Serial.print("RX Buffer Size: ");
    Serial.println(bleSyncCtx.rxBuffer.size());
}
```

---

**Last Resort**: If nothing works, revert to ESP-NOW firmware v16 and file a bug report with complete diagnostics.

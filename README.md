# CMCO Controller & Node - BLE Version

> **Vibrotactile Stimulation System with Bluetooth Low Energy Communication**

[![Firmware](https://img.shields.io/badge/Firmware-v17-blue.svg)]()
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-orange.svg)]()
[![Protocol](https://img.shields.io/badge/Protocol-BLE-green.svg)]()
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)]()

---

## 🎯 Overview

This project implements a synchronized vibrotactile stimulation system using **Bluetooth Low Energy (BLE)** for communication between a Controller and Node device. The system supports:

- ✅ Real-time pattern synchronization
- ✅ BLE communication with buffering
- ✅ Over-The-Air (OTA) firmware updates
- ✅ Optional iPhone connectivity
- ✅ Low-latency stimulation control
- ✅ Automatic reconnection

### Recent Changes (v17)

**Migrated from ESP-NOW to BLE** - See [BLE_MIGRATION.md](BLE_MIGRATION.md) for details.

---

## 📋 Table of Contents

- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Quick Start](#-quick-start)
- [Documentation](#-documentation)
- [Configuration](#-configuration)
- [Usage](#-usage)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)

---

## ✨ Features

### Core Functionality
- **BLE Synchronization**: Low-latency communication between Controller and Node
- **Pattern Generation**: Randomized tactile stimulation patterns
- **Time Synchronization**: Sub-millisecond accuracy with RTT compensation
- **Automatic Reconnection**: Handles disconnections gracefully

### Advanced Features
- **OTA Updates**: Update firmware wirelessly via WiFi
- **Dual BLE**: Separate connections for sync and iPhone app
- **Power Management**: Optional power-saving mode
- **Buffered Communication**: Queue-based message handling

### Developer Features
- **Arduino IDE Compatible**: Easy to build and modify
- **Well Documented**: Comprehensive guides and diagrams
- **Modular Design**: Clean separation of concerns
- **Debug Support**: Extensive serial output

---

## 🔧 Hardware Requirements

### Required
- **2x Adafruit QT Py ESP32-S3** (or compatible ESP32-S3 boards)
- **USB-C cables** for programming and power
- **Vibrotactile actuators** (connected to PWM pins)

### Recommended
- **LiPo batteries** for portable operation
- **3D printed enclosures** (models in `3dModels/`)

### Pin Configuration (QT Py ESP32-S3)
```
PWM Outputs:
- A0 (GPIO 18) → Finger 0
- A1 (GPIO 17) → Finger 1
- A2 (GPIO 9)  → Finger 2
- A3 (GPIO 8)  → Finger 3
```

---

## 🚀 Quick Start

### 1. Install Arduino IDE

Download from: https://www.arduino.cc/en/software

### 2. Install ESP32 Board Support

**Arduino IDE:**
1. File → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board → Boards Manager
4. Search "esp32" and install

### 3. Install Required Libraries

**Tools → Manage Libraries:**
- **NimBLE-Arduino** by h2zero (v1.4.0+)
- **ArduinoJson** by Benoit Blanchon (v6.21.0+)

### 4. Configure Device Role

**Open `controllernode/config.h`:**

For **Controller** device:
```cpp
#define CONTROLLER
//#define NODE
```

For **Node** device:
```cpp
//#define CONTROLLER
#define NODE
```

### 5. Board Settings

**Tools Menu:**
- Board: **"Adafruit QT Py ESP32-S3"**
- Partition Scheme: **"Minimal SPIFFS (Large APP with OTA)"** ⚠️ Important
- USB CDC: **Enabled**
- Port: Select your device

### 6. Upload Firmware

1. Connect device via USB
2. Select correct port
3. Click **Upload** (→)
4. Open **Serial Monitor** (115200 baud)

### 7. Test Connection

1. Power on **Node** first
2. Power on **Controller**
3. Watch Serial Monitor for connection confirmation

**Expected Output:**
```
Controller: Connected! Starting pattern...
Node: Sync Packet Received via BLE
```

---

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [**ARDUINO_SETUP.md**](ARDUINO_SETUP.md) | Step-by-step Arduino IDE setup |
| [**BLE_MIGRATION.md**](BLE_MIGRATION.md) | Technical details of ESP-NOW→BLE migration |
| [**COMPARISON.md**](COMPARISON.md) | ESP-NOW vs BLE performance comparison |
| [**DIAGRAMS.md**](DIAGRAMS.md) | System architecture diagrams |
| [**TROUBLESHOOTING.md**](TROUBLESHOOTING.md) | Common issues and solutions |
| [**IMPLEMENTATION_SUMMARY.md**](IMPLEMENTATION_SUMMARY.md) | Complete change summary |

---

## ⚙️ Configuration

### config.h Settings

```cpp
// Device Role (choose one)
#define CONTROLLER    // Acts as BLE Client
//#define NODE        // Acts as BLE Server

// Board Type (choose one)
#define QTPY_ESP32_S3
//#define GENERIC_ESP32_S3

// Optional Features
//#define BLUETOOTH    // Enable iPhone connectivity
//#define POWER_SAVER  // Enable power saving mode

// Firmware Version
static constexpr uint8_t FW_VERSION = 17;

// Stimulation Parameters
static constexpr uint8_t NUM_PERIODS = 20;
static constexpr uint8_t NUM_FINGERS = 4;
static constexpr float TOTAL_TIME_MS = 166.5f;
static constexpr float PULSE_WIDTH_MS = 100.0f;
```

### BLE Settings

```cpp
// Sync Service (Controller ↔ Node)
Service UUID:  12340001-B5A3-F393-E0A9-E50E24DCCA9E
TX UUID:       12340002-B5A3-F393-E0A9-E50E24DCCA9E
RX UUID:       12340003-B5A3-F393-E0A9-E50E24DCCA9E

// UART Service (iPhone connection, optional)
Service UUID:  6E400001-B5A3-F393-E0A9-E50E24DCCA9E
TX UUID:       6E400003-B5A3-F393-E0A9-E50E24DCCA9E
RX UUID:       6E400002-B5A3-F393-E0A9-E50E24DCCA9E
```

---

## 💡 Usage

### Basic Operation

1. **Power On Sequence:**
   - Turn on NODE device
   - Wait for "Server started, advertising..."
   - Turn on CONTROLLER device
   - Wait for "Connected! Starting pattern..."

2. **Monitor Status:**
   - Open Serial Monitor (115200 baud)
   - Watch for sync messages
   - Check RTT values (~15-30ms normal)

3. **Pattern Execution:**
   - Controller generates random patterns
   - Sends to Node via BLE
   - Both execute synchronized stimulation
   - Repeats automatically

### With iPhone App (Optional)

1. Enable BLUETOOTH in config.h:
   ```cpp
   #define BLUETOOTH
   ```

2. Use iOS app (LightBlue, nRF Connect) to connect

3. Look for device named:
   - "CMCO_CONTROLLER" or
   - "CMCO_NODE"

4. Connect to UART Service (6E400001-...)

### OTA Updates

1. **Setup Server:**
   - Host firmware binary online
   - Update `STATUS_JS_URL` in config.h
   - Increment `FW_VERSION`

2. **Perform Update:**
   - Device checks for updates at boot
   - Connects to WiFi automatically
   - Downloads and flashes new firmware
   - Reboots with new version

3. **No WiFi Available:**
   - Device runs normally
   - OTA check skipped
   - Current firmware continues

---

## 🐛 Troubleshooting

### Common Issues

| Problem | Solution |
|---------|----------|
| Controller can't find Node | Power on Node first, check device names |
| Compilation error | Install NimBLE-Arduino library |
| Upload fails | Hold BOOT button during upload |
| Connection drops | Reduce distance, check battery |
| No stimulation | Verify PWM pin wiring |

See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for detailed solutions.

### Debug Output

Enable diagnostics in code:
```cpp
void setup() {
    Serial.begin(115200);
    delay(1500);
    
    // Print diagnostics
    Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
    Serial.println("BLE Address: " + String(NimBLEDevice::getAddress().toString().c_str()));
    
    // Rest of setup...
}
```

---

## 📊 Performance

| Metric | Value |
|--------|-------|
| Connection Time | 5-10 seconds |
| Round-Trip Latency | 15-30ms |
| Range (Indoor) | 10-30 meters |
| Power Consumption | 30-100mA |
| Packet Size | 284 bytes (sync) |
| Update Rate | ~6 Hz |
| Sync Accuracy | <5ms |

---

## 🗂️ Project Structure

```
controllernode/
├── controllernode.ino          # Main application
├── ble_sync.h                  # BLE synchronization (new)
├── ble_iphone.h                # iPhone BLE connectivity
├── config.h                    # Configuration settings
├── ota.h                       # OTA update functionality
├── stimulation_sequence.h      # Pattern generation
├── buzzer_tunes.h              # Audio feedback
├── buzzer_tunes.cpp
└── piano_notes.h               # Note definitions

Documentation/
├── README.md                   # This file
├── ARDUINO_SETUP.md            # Setup guide
├── BLE_MIGRATION.md            # Migration details
├── COMPARISON.md               # Protocol comparison
├── DIAGRAMS.md                 # Architecture diagrams
├── TROUBLESHOOTING.md          # Debug guide
└── IMPLEMENTATION_SUMMARY.md   # Change summary

3dModels/
└── Controller Box.3mf          # Enclosure design
```

---

## 🔄 Version History

| Version | Protocol | Date | Notes |
|---------|----------|------|-------|
| 1-15 | ESP-NOW | - | Original development |
| 16 | ESP-NOW | - | Final ESP-NOW version |
| **17** | **BLE** | **Feb 2026** | **Current version** |

---

## 🤝 Contributing

### Reporting Issues

1. Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
2. Search existing issues
3. Provide:
   - Hardware details
   - Software versions
   - Serial output
   - Steps to reproduce

### Submitting Changes

1. Fork the repository
2. Create feature branch
3. Test thoroughly
4. Update documentation
5. Submit pull request

### Code Style

- Follow existing formatting
- Comment complex logic
- Update relevant docs
- Test on actual hardware

---

## 📝 License

MIT License - See LICENSE file for details

---

## 🙏 Acknowledgments

- **NimBLE-Arduino** by h2zero
- **ESP32 Arduino Core** by Espressif
- **Adafruit** for QT Py hardware
- CMCO Development Team

---

## 📞 Support

### Resources

- **Documentation**: See docs/ folder
- **Examples**: NimBLE-Arduino examples
- **Forums**: 
  - ESP32: https://esp32.com
  - Arduino: https://forum.arduino.cc

### Contact

For project-specific questions:
1. Check documentation first
2. Review troubleshooting guide
3. Search existing issues
4. Create new issue with details

---

## 🎯 Roadmap

### Short Term
- [ ] Multi-node support (1 controller, N nodes)
- [ ] Connection quality monitoring
- [ ] LED status indicators

### Medium Term
- [ ] BLE bonding/pairing
- [ ] OTA over BLE (no WiFi needed)
- [ ] Web-based configuration

### Long Term
- [ ] BLE Mesh networking
- [ ] Mobile app development
- [ ] Cloud synchronization

---

## 📈 Statistics

- **Lines of Code**: ~1,500
- **Header Files**: 7
- **Implementation Files**: 2
- **Documentation Pages**: 6
- **Supported Boards**: ESP32-S3 series
- **BLE Services**: 2 (Sync + UART)

---

**Built with ❤️ using ESP32-S3 and BLE**

*For detailed technical information, see [BLE_MIGRATION.md](BLE_MIGRATION.md)*

*For setup instructions, see [ARDUINO_SETUP.md](ARDUINO_SETUP.md)*

*For troubleshooting, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md)*

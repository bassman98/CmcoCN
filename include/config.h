#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Role
#define NODE
//#define CONTROLLER

// Board
#define QTPY_ESP32_S3
//#define GENERIC_ESP32_S3

// Other
//#define BLUETOOTH
//#define POWER_SAVER

static constexpr uint64_t ESP_SLEEP = 100000; // us

static constexpr char STATUS_JS_URL[] = "https://ikincaid01.wixsite.com/chameleoncollc/_functions/softwareversions";

#ifdef CONTROLLER
static constexpr uint8_t FW_VERSION = 19;
static constexpr char DEVICE_BLE_NAME[] = "CMCO_CONTROLLER";
static constexpr char NODE_BLE_NAME[] = "CMCO_NODE";
static constexpr char SYNC_ROLE[] = "controller";
#endif

#ifdef NODE
static constexpr uint8_t FW_VERSION = 19;
static constexpr char DEVICE_BLE_NAME[] = "CMCO_NODE";
static constexpr char SYNC_ROLE[] = "node";
#endif

static constexpr char CMCO_PREFIX[] = "CMCO";
static constexpr uint8_t BROADCASTADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // ALL

static constexpr uint32_t SERIAL_BAUD = 115200;

// PWM
static constexpr uint8_t PWM_RESOLUTION =  10; // 10 bit resolution (values 0-1023)
static constexpr float DEFAULT_FREQ = 300.0f;
static constexpr uint8_t DUTYCYCLE_ON = 511; // 511 = 50% duty cycle
static constexpr uint8_t DUTYCYCLE_OFF = 0;

#ifdef QTPY_ESP32_S3
static constexpr uint8_t PWM_PINS[] = {18, 17, 9, 8}; //A0-3
#endif

#ifdef GENERIC_ESP32_S3
static constexpr uint8_t PWM_PINS[] = {4, 5, 6, 7}; // 4-7
#endif

static constexpr uint8_t NUM_PERIODS = 20;
static constexpr uint8_t NUM_FINGERS = 4;
static constexpr float TOTAL_TIME_MS  = 166.5f;
static constexpr float PULSE_WIDTH_MS = 100.0f;
static constexpr float MAX_PRE_JITTER_MS  = 31.5f;
static constexpr float MAX_JITTER_MS  = 66.5f;

// -------------------------
// User-configurable params
// -------------------------
static constexpr bool JITTER_ENABLED = true;
static constexpr bool FREQ_RANDOM_ENABLED = false;
static constexpr float FREQ_RANDOM_MIN = 80;
static constexpr float FREQ_RANDOM_MAX = 10000;

// Master name prefix to match during scanning
static constexpr char MASTER_NAME_PREFIX[] = "CMCO";

// BLE Sync UUIDs (custom for controller-node sync)
static constexpr char SYNC_SERVICE_UUID[] = "12340001-B5A3-F393-E0A9-E50E24DCCA9E";
static constexpr char SYNC_TX_UUID[] = "12340002-B5A3-F393-E0A9-E50E24DCCA9E"; // Notify from peripheral
static constexpr char SYNC_RX_UUID[] = "12340003-B5A3-F393-E0A9-E50E24DCCA9E"; // Write to peripheral

// BLE UART UUIDs (Nordic UART for iPhone)
static constexpr char UART_SERVICE_UUID[] = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static constexpr char UART_RX_UUID[] = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; // Write to peripheral
static constexpr char UART_TX_UUID[] = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; // Notify from peripheral

#endif

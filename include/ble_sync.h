// BLE Synchronization for Controller and Node (replaces ESP-NOW)
#ifndef BLE_SYNC_H
#define BLE_SYNC_H

#include <Arduino.h>
#if defined(USE_ESPNOW)
#include <WiFi.h>
#include <esp_now.h>
#else
#include <NimBLEDevice.h>
#endif
#include <queue>
#include <vector>
#include "config.h"
#include "stimulation_sequence.h"

// Packet structures (same as ESP-NOW)
typedef struct {
  uint32_t t_send_us;    // Controller timestamp
  StimulationPeriod stimPeriods[NUM_PERIODS];
  uint32_t startDelayUs; // how long after receipt the node should start (microseconds)
} SyncPacket;

typedef struct {
  uint32_t t_recv_us;
  uint32_t t_send_us;
} AckPacket;

// BLE Context for sync communication
struct BleSyncContext {
#if !defined(USE_ESPNOW)
  NimBLEServer *server = nullptr;
  NimBLEClient *client = nullptr;
  NimBLEAdvertisedDevice *peerDevice = nullptr;
  NimBLECharacteristic *txChar = nullptr;
  NimBLECharacteristic *rxChar = nullptr;
  NimBLERemoteCharacteristic *remoteTxChar = nullptr;
  NimBLERemoteCharacteristic *remoteRxChar = nullptr;
#endif
  std::queue<std::vector<uint8_t>> rxBuffer;
  bool connected = false;
  bool scanning = false;
};

namespace BleSync {
static BleSyncContext *g_ctx = nullptr;
static void (*g_onReceiveCallback)(const uint8_t*, size_t) = nullptr;

#if defined(USE_ESPNOW)
// ---------------- ESP-NOW (NODE) IMPLEMENTATION ----------------

static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
  if (!g_ctx) return;
  std::vector<uint8_t> vec(data, data + len);
  g_ctx->rxBuffer.push(vec);
  if (g_onReceiveCallback) g_onReceiveCallback(vec.data(), vec.size());
  g_ctx->connected = true;
}

inline void init(BleSyncContext &ctx) {
  g_ctx = &ctx;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("[ESP-NOW] init failed"));
  } else {
    esp_now_register_recv_cb(espnow_recv_cb);
    Serial.println(F("[ESP-NOW] Initialized"));
  }
}

inline void setReceiveCallback(void (*callback)(const uint8_t*, size_t)) {
  g_onReceiveCallback = callback;
}

inline void startServer(BleSyncContext &ctx) {
  // For ESP-NOW, nothing to advertise; ensure initialized
  g_ctx = &ctx;
  ctx.connected = false;
}

inline bool scanAndConnect(BleSyncContext &ctx, uint32_t /*scanTimeSeconds*/ = 5) {
  // ESP-NOW does not scan; rely on broadcast/peer setup. Return whether we've seen controller.
  return ctx.connected;
}

inline bool send(BleSyncContext &ctx, const uint8_t *data, size_t len) {
  (void)ctx;
  // Send as broadcast; controller should receive and respond
  esp_err_t res = esp_now_send(BROADCASTADDRESS, data, (int)len);
  return (res == ESP_OK);
}

inline bool hasData(const BleSyncContext &ctx) {
  return !ctx.rxBuffer.empty();
}

inline std::vector<uint8_t> receive(BleSyncContext &ctx) {
  if (ctx.rxBuffer.empty()) return std::vector<uint8_t>();
  auto v = ctx.rxBuffer.front();
  ctx.rxBuffer.pop();
  return v;
}

inline bool isConnected(const BleSyncContext &ctx) {
  return ctx.connected;
}

inline void update(BleSyncContext &ctx) {
  (void)ctx; // nothing periodic to do for ESP-NOW
}

#else

// ==================== SERVER (NODE) ====================

class ServerCallbacks : public NimBLEServerCallbacks {
 public:
  void onConnect(NimBLEServer* pServer) {
    Serial.println(F("[BLE Sync] Client connected"));
    if (g_ctx) g_ctx->connected = true;
  }
  void onDisconnect(NimBLEServer* pServer) {
    Serial.println(F("[BLE Sync] Client disconnected"));
    if (g_ctx) {
      g_ctx->connected = false;
      // Restart advertising
      if (g_ctx->server) {
        NimBLEDevice::getAdvertising()->start();
        Serial.println(F("[BLE Sync] Restarted advertising"));
      }
    }
  }
};

class RxCallbacks : public NimBLECharacteristicCallbacks {
 public:
  void onWrite(NimBLECharacteristic *chr) {
    if (!g_ctx) return;
    std::string val = chr->getValue();
    if (val.size() > 0) {
      std::vector<uint8_t> data(val.begin(), val.end());
      g_ctx->rxBuffer.push(data);
      
      // Immediately process if callback is set
      if (g_onReceiveCallback) {
        g_onReceiveCallback(data.data(), data.size());
      }
    }
  }
};

// ==================== CLIENT (CONTROLLER) ====================

class ClientCallbacks : public NimBLEClientCallbacks {
 public:
  void onConnect(NimBLEClient* pClient) {
    Serial.println(F("[BLE Sync] Connected to server"));
    pClient->updateConnParams(6, 6, 0, 60); // Fast connection for low latency
  }

  void onDisconnect(NimBLEClient* pClient) {
    Serial.println(F("[BLE Sync] Disconnected from server"));
    if (g_ctx) {
      g_ctx->connected = false;
      // Will need to scan and reconnect
    }
  }
};

// Callback for scan results (process devices found in the results)
static void scanCompleteCB(NimBLEScanResults results) {
  if (!g_ctx) return;

  int count = results.getCount();
  for (int i = 0; i < count; ++i) {
    const NimBLEAdvertisedDevice* advertisedDevice = results.getDevice(i);
    if (!advertisedDevice) continue;

    String name = String(advertisedDevice->getName().c_str());
    Serial.print(F("[BLE Sync] Found device: "));
    Serial.println(name);

    // Check if this is our target (NODE if we're CONTROLLER)
    #ifdef CONTROLLER
    if (name.equals(NODE_BLE_NAME)) {
      Serial.println(F("[BLE Sync] Found target NODE"));
      g_ctx->peerDevice = new NimBLEAdvertisedDevice(*advertisedDevice);
      g_ctx->scanning = false;
      break;
    }
    #endif
  }
}

// Notification callback function (NimBLE 2.x uses function callbacks)
static void notifyCB(NimBLERemoteCharacteristic* pRemoteChar, uint8_t* pData, size_t length, bool isNotify) {
  if (!g_ctx) return;
  std::vector<uint8_t> data(pData, pData + length);
  g_ctx->rxBuffer.push(data);
  
  // Immediately process if callback is set
  if (g_onReceiveCallback) {
    g_onReceiveCallback(data.data(), length);
  }
}

// ==================== COMMON FUNCTIONS ====================

inline void init(BleSyncContext &ctx) {
  g_ctx = &ctx;
  NimBLEDevice::init(DEVICE_BLE_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Max power for range
  Serial.println(F("[BLE Sync] Initialized"));
}

inline void setReceiveCallback(void (*callback)(const uint8_t*, size_t)) {
  g_onReceiveCallback = callback;
}

// ==================== SERVER (NODE) FUNCTIONS ====================

inline void startServer(BleSyncContext &ctx) {
  ctx.server = NimBLEDevice::createServer();
  
  static ServerCallbacks serverCallbacks;
  ctx.server->setCallbacks(&serverCallbacks);
  
  // Create sync service
  auto *service = ctx.server->createService(SYNC_SERVICE_UUID);
  
  // TX characteristic (server sends to client)
  ctx.txChar = service->createCharacteristic(
    SYNC_TX_UUID, 
    NIMBLE_PROPERTY::NOTIFY
  );
  
  // RX characteristic (server receives from client)
  ctx.rxChar = service->createCharacteristic(
    SYNC_RX_UUID, 
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  
  static RxCallbacks rxCallbacks;
  ctx.rxChar->setCallbacks(&rxCallbacks);
  
  service->start();
  
  // Start advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SYNC_SERVICE_UUID);
  NimBLEAdvertisementData scanResp;
  pAdvertising->setScanResponseData(scanResp);
  pAdvertising->start();
  
  Serial.print(F("[BLE Sync] Server started, advertising as: "));
  Serial.println(DEVICE_BLE_NAME);
}

// ==================== CLIENT (CONTROLLER) FUNCTIONS ====================

inline bool scanAndConnect(BleSyncContext &ctx, uint32_t scanTimeSeconds = 5) {
  if (ctx.connected) return true;
  
  Serial.println(F("[BLE Sync] Starting scan..."));
  ctx.scanning = true;
  
  NimBLEScan* pScan = NimBLEDevice::getScan();
  // AdvertisedDeviceCallbacks not needed; using scanCompleteCB via start()
  pScan->setInterval(97);
  pScan->setWindow(37);
  pScan->setActiveScan(true);
  pScan->start(scanTimeSeconds, scanCompleteCB, false);
  
  if (!ctx.peerDevice) {
    Serial.println(F("[BLE Sync] No target device found"));
    ctx.scanning = false;
    return false;
  }
  
  // Connect to the device
  if (!ctx.client) {
    ctx.client = NimBLEDevice::createClient();
    static ClientCallbacks clientCallbacks;
    ctx.client->setClientCallbacks(&clientCallbacks);
    ctx.client->setConnectionParams(6, 6, 0, 60);
    ctx.client->setConnectTimeout(5);
  }
  
  Serial.println(F("[BLE Sync] Connecting to device..."));
  if (!ctx.client->connect(ctx.peerDevice)) {
    Serial.println(F("[BLE Sync] Connection failed"));
    delete ctx.peerDevice;
    ctx.peerDevice = nullptr;
    return false;
  }
  
  Serial.println(F("[BLE Sync] Connected! Getting service..."));
  
  // Get the service
  NimBLERemoteService* pRemoteService = ctx.client->getService(SYNC_SERVICE_UUID);
  if (!pRemoteService) {
    Serial.println(F("[BLE Sync] Service not found"));
    ctx.client->disconnect();
    return false;
  }
  
  // Get characteristics
  ctx.remoteTxChar = pRemoteService->getCharacteristic(SYNC_TX_UUID);
  ctx.remoteRxChar = pRemoteService->getCharacteristic(SYNC_RX_UUID);
  
  if (!ctx.remoteTxChar || !ctx.remoteRxChar) {
    Serial.println(F("[BLE Sync] Characteristics not found"));
    ctx.client->disconnect();
    return false;
  }
  
  // Subscribe to notifications
  if (ctx.remoteTxChar->canNotify()) {
    ctx.remoteTxChar->subscribe(true, notifyCB);
  }
  
  ctx.connected = true;
  Serial.println(F("[BLE Sync] Ready for communication"));
  return true;
}

// ==================== SEND/RECEIVE ====================

inline bool send(BleSyncContext &ctx, const uint8_t *data, size_t len) {
  if (!ctx.connected) return false;
  
  #ifdef CONTROLLER
  // Client mode: write to remote characteristic
  if (ctx.remoteRxChar) {
    return ctx.remoteRxChar->writeValue(data, len, false); // No response needed
  }
  #endif
  
  #ifdef NODE
  // Server mode: notify through TX characteristic
  if (ctx.txChar && ctx.server && ctx.server->getConnectedCount() > 0) {
    ctx.txChar->setValue(data, len);
    ctx.txChar->notify();
    return true;
  }
  #endif
  
  return false;
}

inline bool hasData(const BleSyncContext &ctx) {
  return !ctx.rxBuffer.empty();
}

inline std::vector<uint8_t> receive(BleSyncContext &ctx) {
  if (ctx.rxBuffer.empty()) {
    return std::vector<uint8_t>();
  }
  std::vector<uint8_t> data = ctx.rxBuffer.front();
  ctx.rxBuffer.pop();
  return data;
}

inline bool isConnected(const BleSyncContext &ctx) {
  return ctx.connected;
}

inline void update(BleSyncContext &ctx) {
  #ifdef CONTROLLER
  // Periodically try to reconnect if disconnected
  static uint32_t lastScanAttempt = 0;
  if (!ctx.connected && !ctx.scanning && (millis() - lastScanAttempt > 5000)) {
    lastScanAttempt = millis();
    scanAndConnect(ctx, 5);
  }
  #endif
}

} // namespace BleSync

// Close transport selection (#if defined(NODE) / #else)
#endif

#endif // BLE_SYNC_H

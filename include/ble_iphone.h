// BLE iPhone communication (coexists with ESP-NOW)
#ifndef BLE_IPHONE_H
#define BLE_IPHONE_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <queue>
#include <vector>
#include <string>
#include "config.h"

struct BleIphoneContext {
  NimBLEServer *server = nullptr;
  NimBLECharacteristic *txChar = nullptr; // notify to iPhone
  NimBLECharacteristic *rxChar = nullptr; // writes from iPhone
  NimBLEAdvertising *advertising = nullptr;
  std::queue<std::string> inbox;
  bool connected = false;
};

namespace BleIphone {
static BleIphoneContext *g_ctx = nullptr;

class RxCallbacks : public NimBLECharacteristicCallbacks {
 public:
  void onWrite(NimBLECharacteristic *chr) {
    if (!g_ctx) return;
    std::string val = chr->getValue();
    g_ctx->inbox.push(val);
  }
};

class ServerCallbacks : public NimBLEServerCallbacks {
 public:
  void onConnect(NimBLEServer* pServer) {
    Serial.print(F("[BLE iPhone] Client connected. Connected count: "));
    Serial.println(pServer->getConnectedCount());
    if (g_ctx) g_ctx->connected = true;
  }
  void onDisconnect(NimBLEServer* pServer) {
    Serial.print(F("[BLE iPhone] Client disconnected. Connected count: "));
    Serial.println(pServer->getConnectedCount());
    if (g_ctx) g_ctx->connected = false;
  }
};

inline void init(BleIphoneContext &ctx) {
  g_ctx = &ctx;
  
  // Initialize BLE (can coexist with WiFi/ESP-NOW)
  NimBLEDevice::init(DEVICE_BLE_NAME);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); // max power for range
  
  Serial.println(F("[BLE iPhone] Initialized"));
}

inline void start(BleIphoneContext &ctx) {
  ctx.server = NimBLEDevice::createServer();
  
  // Set server callbacks
  static ServerCallbacks serverCallbacks;
  ctx.server->setCallbacks(&serverCallbacks);
  
  // Create UART service
  auto *service = ctx.server->createService(UART_SERVICE_UUID);
  ctx.txChar = service->createCharacteristic(UART_TX_UUID, NIMBLE_PROPERTY::NOTIFY);
  ctx.rxChar = service->createCharacteristic(UART_RX_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  static RxCallbacks rxCallbacks;
  ctx.rxChar->setCallbacks(&rxCallbacks);
  service->start();

  // Configure advertising
  ctx.advertising = NimBLEDevice::getAdvertising();
  ctx.advertising->setMinInterval(32); // 20ms in 0.625ms units
  ctx.advertising->setMaxInterval(160); // 100ms in 0.625ms units
  
  NimBLEAdvertisementData advData;
  advData.setName(DEVICE_BLE_NAME);
  advData.setCompleteServices(NimBLEUUID(UART_SERVICE_UUID));
  
  ctx.advertising->setAdvertisementData(advData);
  ctx.advertising->start();
  
  Serial.print(F("[BLE iPhone] Advertising as '"));
  Serial.print(DEVICE_BLE_NAME);
  Serial.println(F("' with UART service"));
  Serial.println(F("[BLE iPhone] Ready for iPhone connection"));
}

inline bool isConnected(const BleIphoneContext &ctx) {
  return ctx.connected && ctx.server && ctx.server->getConnectedCount() > 0;
}

inline void write(const BleIphoneContext &ctx, const std::string &payload) {
  if (payload.empty()) return;
  
  if (ctx.txChar && ctx.server && ctx.server->getConnectedCount() > 0) {
    ctx.txChar->setValue(payload);
    ctx.txChar->notify();
  }
}

inline std::vector<std::string> readLines(BleIphoneContext &ctx) {
  std::vector<std::string> lines;
  while (!ctx.inbox.empty()) {
    std::string raw = ctx.inbox.front();
    ctx.inbox.pop();
    size_t start = 0;
    while (start < raw.size()) {
      size_t pos = raw.find('\n', start);
      if (pos == std::string::npos) pos = raw.size();
      std::string piece = raw.substr(start, pos - start);
      if (!piece.empty()) lines.push_back(piece);
      start = pos + 1;
    }
  }
  return lines;
}
} // namespace BleIphone

#endif // BLE_IPHONE_H

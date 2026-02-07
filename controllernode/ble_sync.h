// Legacy shim: forward to project-wide canonical header at include/ble_sync.h

#ifndef CONTROLLERNODE_BLE_SYNC_SHIM_H
#define CONTROLLERNODE_BLE_SYNC_SHIM_H

#include "../include/ble_sync.h"

#endif // CONTROLLERNODE_BLE_SYNC_SHIM_H
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

#endif

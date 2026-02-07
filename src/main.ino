#include <WiFi.h>
#include "ota.h"
#include "stimulation_sequence.h"
#include "config.h"
#include "buzzer_tunes.h"
#include "ble_sync.h"
#include <deque>

#ifdef BLUETOOTH
#include "ble_iphone.h"
BleIphoneContext bleIphoneCtx;  // BLE for iPhone
bool lastBleIphoneConnected = false;
#endif

// BLE synchronization context
BleSyncContext bleSyncCtx;

// Synchronization packets defined in ble_sync.h
SyncPacket packet_0;
AckPacket ack;
volatile bool requestSleep = false;

StimulationSequence stim;
float oneWaySyncDelay = 0.0f;

#ifdef NODE
struct PendingSync {
  SyncPacket pkt;
  uint64_t startTimeUs;
};
std::deque<PendingSync> syncQueue;
#endif

#ifdef CONTROLLER
void sendSyncPacket() {
  // Send the sequence and include a buffer delay so nodes can schedule playback
  packet_0.t_send_us = esp_timer_get_time();
  packet_0.startDelayUs = 200000; // 200ms buffer by default
  memcpy(&packet_0.stimPeriods, stim.stimPeriods, sizeof(packet_0.stimPeriods));
  
  bool sent = BleSync::send(bleSyncCtx, (uint8_t *)&packet_0, sizeof(SyncPacket));
  
  if (sent) {
    Serial.println("Sync Packet Sent via BLE");
  } else {
    Serial.println("Failed to send sync packet (not connected)");
  }
  
  #ifdef POWER_SAVER
  requestSleep = true;
  #endif
}

void onAckReceive(const uint8_t *data, size_t len) {
  if (len != sizeof(AckPacket)) return;
  
  const AckPacket *ack = (const AckPacket *)data;

  uint32_t t_now = esp_timer_get_time();
  uint32_t rtt_us = t_now - ack->t_send_us;
  float oneWay_us = rtt_us * 0.5f;

  updateSyncDelay(oneWay_us);
  Serial.print("ACK received, RTT: ");
  Serial.print(rtt_us);
  Serial.println(" us");
}

void updateSyncDelay(float newEstimate) {
  constexpr float alpha = 0.1f;

  oneWaySyncDelay =
      (1.0f - alpha) * oneWaySyncDelay +
      alpha * newEstimate;

  stim.setSyncOffset(oneWaySyncDelay);  // live update
}
#endif

#ifdef NODE
void onSyncReceive(const uint8_t *data, size_t len) {
  if (len != sizeof(SyncPacket)) return;
  
  const SyncPacket *pkt = (const SyncPacket *)data;
  uint32_t t_now = esp_timer_get_time();

  // Queue the packet for buffered playback
  PendingSync ps;
  memcpy(&ps.pkt, pkt, sizeof(SyncPacket));
  ps.startTimeUs = (uint64_t)t_now + (uint64_t)pkt->startDelayUs;
  syncQueue.push_back(ps);

  // Send ACK back with original controller send timestamp and our recv time
  ack.t_send_us = pkt->t_send_us;
  ack.t_recv_us = t_now;
  BleSync::send(bleSyncCtx, (uint8_t *)&ack, sizeof(AckPacket));

  Serial.println("Sync Packet queued for buffered playback");

  #ifdef POWER_SAVER
  requestSleep = true;
  #endif
}
#endif

void setupBLE() {
  // Initialize BLE
  BleSync::init(bleSyncCtx);
  
  #ifdef CONTROLLER
  // Controller acts as BLE client
  Serial.println("Starting BLE in CONTROLLER mode (client)");
  BleSync::setReceiveCallback(onAckReceive);
  // Will scan and connect in loop
  #endif
  
  #ifdef NODE
  // Node acts as BLE server
  Serial.println("Starting BLE in NODE mode (server)");
  BleSync::setReceiveCallback(onSyncReceive);
  BleSync::startServer(bleSyncCtx);
  #endif
  
  Serial.print("BLE Device Address: ");
  Serial.println(NimBLEDevice::getAddress().toString().c_str());
}

void testPWMOutputs() {
  Serial.println("PWM Pin Testing");
  delay(5000);
  for (int i = 0; i < NUM_FINGERS; i++) {
    Serial.print("Testing Index: ");
    Serial.print(i);
    Serial.print(" PWM PIN: ");
    Serial.println(PWM_PINS[i]);
    delay(500);
    uint8_t channel = i % 16;
    ledcSetup(channel, DEFAULT_FREQ, PWM_RESOLUTION);
    ledcAttachPin(PWM_PINS[i], channel);
    ledcWrite(channel, DUTYCYCLE_ON);
    delay(500);
    ledcWrite(channel, DUTYCYCLE_OFF);
    delay(1000);
  }
  delay(5000);
}

// setup
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1500);

  //testPWMOutputs();

  OTA::otaCheck();

  setupBLE();

  playSuccess();

  #ifdef BLUETOOTH
  // Initialize BLE for iPhone
  BleIphone::init(bleIphoneCtx);
  BleIphone::start(bleIphoneCtx);
  #endif

  #ifdef CONTROLLER
  //Start new pattern
  //playMario();
  Serial.println("Controller: Waiting for BLE connection to NODE...");
  stim.begin(analogRead(0)); // Seed the random number generator with a random value
  #endif

  #ifdef NODE
  //playRadRacer();
  Serial.println("Node: Waiting for CONTROLLER connection...");
  #endif
}

// main loop
void loop() {
  // Update BLE connection status (controller will auto-reconnect)
  BleSync::update(bleSyncCtx);
  
  #ifdef CONTROLLER
  // Try to connect if not connected
  static bool hasConnected = false;
  if (!BleSync::isConnected(bleSyncCtx)) {
    if (hasConnected) {
      Serial.println("Connection lost, attempting to reconnect...");
    }
    static uint32_t lastConnectAttempt = 0;
    if (millis() - lastConnectAttempt > 5000) {
      lastConnectAttempt = millis();
      if (BleSync::scanAndConnect(bleSyncCtx, 5)) {
        hasConnected = true;
        Serial.println("Connected! Starting pattern...");
        stim.begin(analogRead(0));
        sendSyncPacket();
      }
    }
    return; // Don't run stimulation if not connected
  } else if (!hasConnected) {
    hasConnected = true;
    Serial.println("Initial connection established");
    sendSyncPacket();
  }
  #endif
  
  stim.update();

#ifdef NODE
  // Process pending buffered syncs
  if (!syncQueue.empty()) {
    uint64_t now = esp_timer_get_time();
    while (!syncQueue.empty() && syncQueue.front().startTimeUs <= now) {
      // Start the queued sequence
      auto ready = syncQueue.front();
      memcpy(&stim.stimPeriods, ready.pkt.stimPeriods, sizeof(stim.stimPeriods));
      stim.reset();
      syncQueue.pop_front();
      Serial.println("Starting buffered sync sequence");
    }
  }
#endif

  if (stim.isFinished()) {
      #ifdef CONTROLLER
      if (BleSync::isConnected(bleSyncCtx)) {
        Serial.println("Sequence complete");
        Serial.println("Begin New Pattern:");
        stim.begin(analogRead(0));
        sendSyncPacket();
      }
      #endif
  }

  if (!stim.isActive()) {
    #ifdef BLUETOOTH
    // Handle iPhone BLE connection status
    bool bleIphoneConnected = BleIphone::isConnected(bleIphoneCtx);
    if (bleIphoneConnected && !lastBleIphoneConnected) {
      Serial.println(F("[MASTER] iPhone connected via BLE"));
      BleIphone::write(bleIphoneCtx, std::string("ROLE:MASTER\n"));
    }
    lastBleIphoneConnected = bleIphoneConnected;

    // Process iPhone messages
    auto iphoneLines = BleIphone::readLines(bleIphoneCtx);
    for (const auto &ln : iphoneLines) {
      Serial.print(F("[BLE iPhone] Received: "));
      Serial.println(ln.c_str());
      // Add iPhone command handling here if needed
    }
    #endif
  }

  #ifdef POWER_SAVER
  if (requestSleep) {
    requestSleep = false;

    // Note: BLE sleep handling is different from ESP-NOW
    // BLE connection will be maintained during light sleep
    esp_sleep_enable_timer_wakeup(ESP_SLEEP);
    esp_light_sleep_start();
  }
  #endif

}

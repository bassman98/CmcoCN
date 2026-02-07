/***************************************************
*  CMCO Vibrotactile Glove Controller (Arduino port)
*  QT Py ESP32-S3 OTA-on-Boot Firmware
*  Flash: 4MB | PSRAM: 2MB
*  Partition: Minimal SPIFFS (Large APP with OTA)
***************************************************/

#ifndef OTA_H
#define OTA_H

#include <Arduino.h>               // Required for millis(), delay(), String, Serial
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "config.h"

namespace OTA {

  // ---- Forward declarations ----
  struct Status;
  
  const char* cmco_password = "cmco123456789!";

  inline bool connectWiFi(uint32_t timeoutMs = 10000);
  inline Status fetchOtaStatus(uint32_t timeoutMs = 5000);
  inline bool checkForUpdate(Status payload);
  inline bool performOTA(const String& firmwareURL);
  void otaCheck();

  // ---- OTA status struct ----
  struct Status {
    bool valid = false;
    String version;
    String firmwareUrl;
  };

  // -------- WiFi --------

  inline bool connectAnyWifi()
  {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(500);

    int n = WiFi.scanNetworks();
    Serial.printf("Found %d networks\n", n);

    // First pass: try open networks (no password)
    for (int i = 0; i < n; i++) {
      String ssid = WiFi.SSID(i);
      wifi_auth_mode_t auth = (wifi_auth_mode_t)WiFi.encryptionType(i);

      Serial.printf("%2d: %s  auth:%d\n", i, ssid.c_str(), auth);

      if (auth == WIFI_AUTH_OPEN) {
        Serial.printf("Trying open network: %s\n", ssid.c_str());

        WiFi.begin(ssid.c_str());
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
          delay(250);
          Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
          Serial.printf("Connected to %s, IP: %s\n",
                        ssid.c_str(),
                        WiFi.localIP().toString().c_str());
          return true;
        } else {
          Serial.println("Failed, moving to next open network");
        }
      }
    }

    // Second pass: try secured networks with shared password
    for (int i = 0; i < n; i++) {
      String ssid = WiFi.SSID(i);
      wifi_auth_mode_t auth = (wifi_auth_mode_t)WiFi.encryptionType(i);

      if (auth != WIFI_AUTH_OPEN) {
        Serial.printf("Trying secured network: %s (auth:%d)\n", ssid.c_str(), auth);
        WiFi.begin(ssid.c_str(), cmco_password);
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
          delay(250);
          Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
          Serial.printf("Connected to %s, IP: %s\n",
                        ssid.c_str(),
                        WiFi.localIP().toString().c_str());
          return true;
        } else {
          Serial.println("Failed, moving to next secured network");
        }
      }
    }

    Serial.println("No WiFi networks connected");
    return false;
  }

  // -------- OTA Boot Check --------
  void otaCheck()
  {
    Serial.println("\nQT Py ESP32-S3 OTA Boot");
    Serial.printf("Current FW Version: %d\n", FW_VERSION);

    if (!OTA::connectAnyWifi()) {
      Serial.println("WiFi unavailable, running current firmware");
      return;
    }

    Serial.println("WiFi connected");
    Serial.println("Checking website for JS-based OTA updates...");

    Status status = OTA::fetchOtaStatus();

    if (!status.valid) {
      Serial.println("Status fetch failed");
      return;
    }

    Serial.println("Status OK");
    Serial.println(status.version);
    Serial.println(status.firmwareUrl);

    if (OTA::checkForUpdate(status)) {
      Serial.println("New firmware available");
      if (OTA::performOTA(status.firmwareUrl)) {
        Serial.println("OTA success, rebooting...");
        delay(100);
        ESP.restart();
      } else {
        Serial.println("OTA failed, continuing current firmware");
      }
    } else {
      Serial.println("Firmware up to date");
    }
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }

  // -------- Get OTA Status from Web --------
  inline Status fetchOtaStatus(uint32_t timeoutMs)
  {
    Status result;

    WiFiClientSecure client;
    client.setInsecure();   // OK for now (Cloudflare/Wix)

    HTTPClient https;
    https.setTimeout(timeoutMs);

    if (!https.begin(client, STATUS_JS_URL)) {
      return result;
    }

    int code = https.GET();
    if (code != HTTP_CODE_OK) {
      https.end();
      return result;
    }

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, https.getStream());
    https.end();

    if (err) {
      return result;
    }

    result.valid = true;
    result.version = doc[SYNC_ROLE]["version"] | "";
    result.firmwareUrl = doc[SYNC_ROLE]["firmwareUrl"] | "";

    return result;
  }

  // -------- Check Manifest --------
  inline bool checkForUpdate(Status payload)
  {
    if (!payload.valid) return false;
    int availableVersion = atoi(payload.version.c_str());
    return (availableVersion > FW_VERSION);
  }

  // -------- Perform OTA --------
  inline bool performOTA(const String& firmwareURL)
  {
    WiFiClientSecure client;
    client.setInsecure();  // OK for now; pin CA cert later

    HTTPClient http;
    http.setTimeout(10000);

    Serial.println("http.begin...");
    if (!http.begin(client, firmwareURL)) return false;

    Serial.println("http.GET...");
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
      http.end();
      return false;
    }

    int contentLen = http.getSize();
    Serial.printf("contentLen = %d\n", contentLen);

    if (contentLen <= 0) {
      Serial.println("Invalid firmware size");
      http.end();
      return false;
    }

    if (!Update.begin(contentLen)) {
      Serial.printf("Update.begin error: %d\n", Update.getError());
      http.end();
      return false;
    }

    WiFiClient& stream = *http.getStreamPtr();
    size_t written = Update.writeStream(stream);
    Serial.printf("written = %u\n", (unsigned)written);

    bool success = Update.end(true);
    Serial.println("Update.end...");
    http.end();

    return success;
  }

} // namespace OTA

#endif // OTA_H

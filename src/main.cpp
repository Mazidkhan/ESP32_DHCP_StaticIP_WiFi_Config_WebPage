#include "constants.h"
#include "html.h"
#include "certificate.h"
#include "mqtt.h"
#include "ip_change.h"
#include "wifi.h"
#include "sensordata.h"

void setup() {
  Serial.begin(9600);
  SPIFFS.begin();
  preferences.begin("wifi", false);
  pinMode(resetButtonPin, INPUT_PULLUP);
  bme.begin(0x76);
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  downloadCertificate();
  loadCertificate("/isrgrootx1.pem");
  connectMQTT();
  server.on("/", handleRoot);
  server.on("/esp.js", HTTP_GET, handleScript);
  server.on("/set_ip_mode", handleSetIPMode);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/sensorData", HTTP_GET, handleSensorData);
  server.on("/connectedNetwork", HTTP_GET, handleConnectedNetwork);
  server.on("/availableNetworks", HTTP_GET, handleAvailableNetworks);
  server.begin();
  Serial.println("HTTP Server started");
}

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();
  if (digitalRead(resetButtonPin) == LOW) {  // Button pressed
    Serial.println("Reset button pressed, deleting Wi-Fi credentials...");
    deleteWiFiCredentials();  // Delete Wi-Fi credentials and start AP mode
    WiFi.disconnect(true);    // Disconnect from any existing Wi-Fi connections
    wifiReset = true;         // Set flag to indicate Wi-Fi credentials have been reset
    delay(1000);              // Debounce delay
  }
  if (WiFi.status() == WL_CONNECTED) {
     WiFi.softAPdisconnect(true);
     client.loop();
     publishSensorData();
     delay(1000);
  }
  // Check if 24 hours have passed since the last download
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    downloadCertificate();
  }
}
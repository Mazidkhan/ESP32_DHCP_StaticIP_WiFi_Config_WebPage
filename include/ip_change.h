void handleSetIPMode() {
  String ipModeSelected = server.arg("ip_mode");  // Get selected IP mode (DHCP or Static)
  if (ipModeSelected == "dhcp") {
    ipMode = "DHCP";
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);  // Reset to DHCP mode
    WiFi.begin();  // Reconnect to Wi-Fi using DHCP
    Serial.println("Switched to DHCP mode");
  }
  else if (ipModeSelected == "static") {
    ipMode = "Static";
    String staticIPStr = server.arg("static_ip");
    String gatewayStr = server.arg("gateway_ip");
    String subnetStr = server.arg("subnet_ip");
    preferences.putString("static_ip", staticIPStr);
    preferences.putString("gateway", gatewayStr);
    preferences.putString("subnet", subnetStr);

    if (staticIPStr.length() >= 0 && gatewayStr.length() >= 0 && subnetStr.length() >= 0) {
      staticIP.fromString(staticIPStr);
      gateway.fromString(gatewayStr);
      subnet.fromString(subnetStr);
      WiFi.config(staticIP, gateway, subnet);  // Set static IP configuration
      WiFi.begin();  // Reconnect with static IP
      Serial.println("Switched to Static IP mode");
      Serial.print("Static IP: ");
      Serial.println(staticIP);
      unsigned long startAttemptTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        Serial.print(".");
        delay(500);
      }
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to Wi-Fi with Static IP");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("\nFailed to connect to Wi-Fi with Static IP, starting AP mode.");
        WiFi.softAP("ESP32_AP");
        return;  // Exit if Wi-Fi connection fails
      }
    } else {
      Serial.println("Invalid static IP configuration provided.");
      return;  // Exit if static IP configuration is invalid
    }
  }
  loadCertificate("/isrgrootx1.pem");
  connectMQTT();
  publishSensorData();
}
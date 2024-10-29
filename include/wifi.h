void connectToWiFi() {
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");

  String static_ip = preferences.getString("static_ip", "");
  String gatewayStr = preferences.getString("gateway", "");
  String subnetStr = preferences.getString("subnet", "");
    
  if (ssid.length() > 0) {
    WiFi.softAPdisconnect(true);  // Disconnect from current AP (if any)
    Serial.println("Connecting to saved Wi-Fi: " + ssid);
    
    /*if (static_ip.length() >= 0 && gatewayStr.length() >= 0 && subnetStr.length() >= 0) {
      staticIP.fromString(static_ip);
      gateway.fromString(gatewayStr);
      subnet.fromString(subnetStr);
      WiFi.begin(ssid.c_str(), password.c_str());
      WiFi.config(staticIP, gateway, subnet);  // Set static IP configuration
    }*/

    WiFi.begin(ssid.c_str(), password.c_str());  // Attempt to connect to Wi-Fi
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      Serial.print(".");
      delay(500);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to Wi-Fi");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFailed to connect to saved Wi-Fi, starting AP mode.");
      WiFi.softAP("ESP32_AP");
    }
  } else {
    Serial.println("No saved Wi-Fi credentials found, starting AP mode.");
    WiFi.softAP("ESP32_AP");
  }
}

void handleConnect() {
  String wifiName = server.arg("wifi_name");
  String wifiPassword = server.arg("wifi_password");
  WiFi.softAPdisconnect(true);  // Stops the access point and disconnects any connected devices
  if (wifiName.length() > 0) {
    preferences.putString("ssid", wifiName);
    preferences.putString("password", wifiPassword);
    WiFi.disconnect();
    delay(1000);  // Wait 1 second for disconnect
    WiFi.begin(wifiName.c_str(), wifiPassword.c_str());
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startAttemptTime > 10000) {  // Timeout after 10 seconds
        Serial.println("Failed to connect to Wi-Fi within timeout period");
        server.send(200, "text/html", "<html><body><h1>Error</h1><p>Failed to connect to Wi-Fi within timeout.</p><a href=\"/\">Back</a></body></html>");
        return;
      }
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi: " + wifiName);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  connectMQTT();
  publishSensorData();
}

void deleteWiFiCredentials() {
  preferences.remove("ssid");
  preferences.remove("password");
  Serial.println("Wi-Fi credentials deleted.");
  WiFi.softAP("ESP32_AP");
  Serial.println("Access Point started: ESP32_AP");
}

void handleAvailableNetworks() {
  int n = WiFi.scanNetworks();
  String jsonResponse = "{\"networks\": [";
  for (int i = 0; i < n; i++) {
    jsonResponse += "{\"SSID\": \"" + WiFi.SSID(i) + "\", ";
    jsonResponse += "\"RSSI\": " + String(WiFi.RSSI(i)) + "}";
    if (i < n - 1) {
      jsonResponse += ", ";
    }
  }
  jsonResponse += "]}";
  server.send(200, "application/json", jsonResponse);
}

void handleConnectedNetwork() {
  String jsonResponse;
  if (WiFi.status() == WL_CONNECTED) {
    jsonResponse = "{\"SSID\": \"" + WiFi.SSID() + "\", ";  // Get connected SSID
    jsonResponse += "\"IP_Mode\": \"" + ipMode + "\", ";  // Determine IP mode (DHCP or Static)
    jsonResponse += "\"IP_Address\": \"" + WiFi.localIP().toString() + "\"} ";  // Get IP Address
  } else {
    // If not connected, send an empty response or a message
    jsonResponse = "{\"error\": \"Not connected to any network\"}";
  }
  server.send(200, "application/json", jsonResponse);
}


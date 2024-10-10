#include <Wire.h>
#include <FS.h>   // Filesystem library
#include <SPIFFS.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h> // Include Preferences library to store Wi-Fi credentials

Preferences preferences; // Create an instance of Preferences

const int resetButtonPin = 0; // Example: GPIO0 (adjust according to your setup)
bool wifiReset = false;  // Flag to track Wi-Fi reset
float temperature;    // Declare global variables
float humidity;

WebServer server(80);
Adafruit_BME280 bme; // I2C
WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

const char* broker_host = "bigdata.reconindia.in";  // MQTT Broker Host
const int broker_port = 8883;                       // MQTT Broker Port
const char* mqtt_username = "glxo6iiu1m66didv8h34"; // MQTT Username
const char* mqtt_password = "3zt89q3i3rhp9rrxise0"; // MQTT Password
const char* client_id = "gqikox6lvft0r9j9icc9";     // Client ID
const char* topic = "v1/devices/me/telemetry";

IPAddress staticIP;
IPAddress gateway;
IPAddress subnet;
String ipMode = "DHCP"; // Default IP mode

void handleRoot() {
  File file = SPIFFS.open("/esp.html", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  String html = "";
  while (file.available()) {
    html += (char)file.read();
  }
  file.close();
  
  server.send(200, "text/html", html);
}
void handleScript() {
  File file = SPIFFS.open("/esp.js", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  String js = "";
  while (file.available()) {
    js += (char)file.read();
  }
  file.close();
  
  server.send(200, "application/javascript", js);
}
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
  connectMQTT();
  publishSensorData();
}

void connectMQTT() {
  SPIFFS.begin(true);
  loadCertificate("/isrgrootx1.pem");

  client.setServer(broker_host, broker_port);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(client_id, mqtt_username, mqtt_password)) {
      Serial.println(" connected.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds.");
      delay(5000);
    }
  }
}

// Function to connect to Wi-Fi using stored credentials
void connectToWiFi() {
  WiFi.softAPdisconnect(true);  // Disconnect from current AP (if any)
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  if (ssid.length() > 0) {
    Serial.println("Connecting to saved Wi-Fi: " + ssid);
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
    }
  } else {
    Serial.println("No saved Wi-Fi credentials found, starting AP mode.");
    WiFi.softAP("ESP32_AP");
    //return false;  // Return false when connection fails or no credentials are found
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


bool loadCertificate(const char* path) {
  File cert = SPIFFS.open(path, "r");
  if (!cert) {
    Serial.println("Failed to open certificate file");
    return false;
  }

  String certContent;
  while (cert.available()) {
    certContent += (char)cert.read();
  }
  cert.close();
  wifiClient.setCACert(certContent.c_str());

  Serial.println("Certificate loaded successfully");
  return true;  // Return true regardless since we can't verify the success of setCACert()
}

void publishSensorData() {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  String payload = "{\"temperature\":" + String(temperature) + ", \"humidity\":" + String(humidity) + "}";
  if (client.publish(topic, payload.c_str())) {
    Serial.println("Data published: " + payload);
  } else {
    Serial.println("Failed to publish data");
  }
}

// Function to delete saved Wi-Fi credentials
void deleteWiFiCredentials() {
  preferences.remove("ssid");
  preferences.remove("password");
  Serial.println("Wi-Fi credentials deleted.");
  WiFi.softAP("ESP32_AP");
  Serial.println("Access Point started: ESP32_AP");
}

void handleSensorData() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  String jsonResponse = "{\"temperature\": " + String(temperature) + ", ";
  jsonResponse += "\"humidity\": " + String(humidity) + "}";
  server.send(200, "application/json", jsonResponse);
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

  // Check if the device is connected to a Wi-Fi network
  if (WiFi.status() == WL_CONNECTED) {
    //String ipMode = (WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0))) ? "DHCP" : "Static";
    
    jsonResponse = "{\"SSID\": \"" + WiFi.SSID() + "\", ";  // Get connected SSID
    jsonResponse += "\"IP_Mode\": \"" + ipMode + "\", ";  // Determine IP mode (DHCP or Static)
    jsonResponse += "\"IP_Address\": \"" + WiFi.localIP().toString() + "\"} ";  // Get IP Address
  } else {
    // If not connected, send an empty response or a message
    jsonResponse = "{\"error\": \"Not connected to any network\"}";
  }

  server.send(200, "application/json", jsonResponse);
}

void setup() {
  Serial.begin(115200);
  bme.begin(0x76); // Initialize BME280 sensor
  WiFi.softAP("ESP32_AP");
  pinMode(resetButtonPin, INPUT_PULLUP);  // Set up the reset button
  preferences.begin("wifi", false);  // Open preferences with a namespace "wifi"
  connectToWiFi();
  server.on("/", handleRoot);
  server.on("/esp.js", HTTP_GET, handleScript);
  server.on("/set_ip_mode", handleSetIPMode);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/sensorData", HTTP_GET, handleSensorData);
  server.on("/connectedNetwork", HTTP_GET, handleConnectedNetwork);
  server.on("/availableNetworks", HTTP_GET, handleAvailableNetworks);
  server.begin();
}

void loop() {
  server.handleClient();
  if (digitalRead(resetButtonPin) == LOW) {  // Button pressed
    Serial.println("Reset button pressed, deleting Wi-Fi credentials...");
    deleteWiFiCredentials();  // Delete Wi-Fi credentials and start AP mode
    WiFi.disconnect(true);    // Disconnect from any existing Wi-Fi connections
    wifiReset = true;         // Set flag to indicate Wi-Fi credentials have been reset
    delay(1000);              // Debounce delay
  }
  if (!wifiReset && WiFi.status() == WL_CONNECTED) {
     WiFi.softAPdisconnect(true);
     client.loop();
     connectMQTT();
  }
  publishSensorData();
  delay(1000);  // Send data every 5 seconds

}
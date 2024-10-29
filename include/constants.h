#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

unsigned long interval = 86400000;
unsigned long previousMillis = 0;
const char* cert_url = "https://letsencrypt.org/certs/isrgrootx1.pem";
const char* cert_filename = "/isrgrootx1.pem";
const char* broker_host = "bigdata.reconindia.in";  // MQTT Broker Host
const int broker_port = 8883;                       // MQTT Broker Port
const char* mqtt_username = "glxo6iiu1m66didv8h34"; // MQTT Username
const char* mqtt_password = "3zt89q3i3rhp9rrxise0"; // MQTT Password
const char* client_id = "gqikox6lvft0r9j9icc9";     // Client ID
const char* topic = "v1/devices/me/telemetry";
const int resetButtonPin = 0; // Example: GPIO0 (adjust according to your setup)
bool wifiReset = false;  // Flag to track Wi-Fi reset
float temperature;    // Declare global variables
float humidity;

WebServer server(80);
Adafruit_BME280 bme; // I2C
WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);
Preferences preferences; // Create an instance of Preferences
IPAddress staticIP,gateway,subnet;
String ipMode = "DHCP"; // Default IP mode

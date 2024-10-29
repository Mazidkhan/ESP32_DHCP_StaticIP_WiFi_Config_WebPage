void connectMQTT() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  client.setServer(broker_host, broker_port);

  int retryCount = 0;
  const int maxRetries = 10;  // Limit retries to avoid infinite loop
  while (!client.connected() && retryCount < maxRetries) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(client_id, mqtt_username, mqtt_password)) {
      Serial.println(" connected.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds.");
      delay(5000);
      retryCount++;
    }
  }

  if (retryCount >= maxRetries) {
    Serial.println("Failed to connect to MQTT after maximum retries.");
  }
}

void publishSensorData() {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  String payload = "{\"temperature\":" + String(temperature) + ", \"humidity\":" + String(humidity) + "}";
  if (client.publish(topic, payload.c_str())) {
    Serial.println("Data published: " + payload);
  } else {
    Serial.println("Failed to publish data reconnecting...");
    loadCertificate("/isrgrootx1.pem");
    connectMQTT();
    publishSensorData();
  }
}

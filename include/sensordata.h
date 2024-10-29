void handleSensorData() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  String jsonResponse = "{\"temperature\": " + String(temperature) + ", ";
  jsonResponse += "\"humidity\": " + String(humidity) + "}";
  server.send(200, "application/json", jsonResponse);
}
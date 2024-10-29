void downloadCertificate() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(cert_url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Downloaded certificate content:");
      //Serial.println(payload);
      // Save the certificate to SPIFFS
      File file = SPIFFS.open(cert_filename, FILE_WRITE);
      if (file) {
        file.print(payload);
        file.close();
        Serial.println("Certificate saved to SPIFFS.");
      } else {
        Serial.println("Failed to open file for writing.");
      }
    } else {
      Serial.printf("Failed to download certificate. HTTP error code: %d\n", httpCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}

void loadCertificate(const char* path) {
  // Open certificate file
  File cert = SPIFFS.open(path, "r");
  if (!cert) {
    Serial.println("Failed to open certificate file");
  }

  // Determine file size and allocate a character array
  size_t certSize = cert.size();
  char* certContent = new char[certSize + 1];  // Create buffer to hold certificate content
  cert.readBytes(certContent, certSize);
  certContent[certSize] = '\0';  // Null-terminate the certificate string

  // Close the file after reading
  cert.close();

  // Set CA certificate for SSL (note that setCACert returns void, so no need for a bool result)
  wifiClient.setCACert(certContent);

  Serial.println("Certificate loaded successfully");

  // Free the dynamically allocated buffer
  delete[] certContent;

}

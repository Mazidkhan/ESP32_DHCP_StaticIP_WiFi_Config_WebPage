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
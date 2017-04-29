void sendDummyResponse(WiFiClient client);
void sendErrorResponse(WiFiClient client);

void sendResponseHeader(int code, WiFiClient client) {
  if (code == 200) {
    client.println(F("HTTP/1.1 200 OK"));
  } else {
    client.println(F("HTTP/1.1 500 Internal Server Error"));
  }
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close")); 
  client.println();
}

void sendDummyResponse(WiFiClient client) {
  sendResponseHeader(200, client);
  client.println(F("<!DOCTYPE HTML>\
                 <html>\
                 <link rel='shortcut icon' type='image/x-icon' href='http://www.arduino.cc/favicon.ico' />\
                 <h1>It works!</h1>\
                 </html>"));
  // give the web browser time to receive the data
  delay(1);
}

void sendErrorResponse(WiFiClient client) {
  sendResponseHeader(500, client);
  client.println(F("<!DOCTYPE HTML>\
                 <html>\
                 <link rel='shortcut icon' type='image/x-icon' href='http://www.arduino.cc/favicon.ico' />\
                 <h1>An error has been encountered by the Arduino Board.</h1>\
                 </html>"));
  // give the web browser time to receive the data
  delay(1);
}
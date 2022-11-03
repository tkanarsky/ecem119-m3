#include <Arduino.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include <LSM6DS3.h>

char ssid[] = "Nano33";
char password[] = "sussusamogus";

WiFiServer server(80);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  asm(".global _printf_float"); // enable pretty-printing floats
  Serial.begin(115200);
  int8_t status = WiFi.beginAP(ssid, password);
  if (status != WL_AP_LISTENING) {
    Serial.println("Failed to setup access point");
    while (true);
  }
  Serial.println("Established hotspot, listening for TCP connections");
  server.begin();
  while(!IMU.begin()) {delay(10);}
  Serial.println("Established IMU");
}

void loop() {
  // create static variables to store xyz acceleration and xyz rotation
  static float ax, ay, az, rx, ry, rz;
  static char print_buffer[200];
  // if IMU readings are available, read them
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    if (!IMU.readAcceleration(ax, ay, az)) {
      Serial.println("Failed to read acceleration"); 
    }
    if (!IMU.readGyroscope(rx, ry, rz)) {
      Serial.println("Failed to read gyroscope"); 
    }
  }

  // HTTP handling code taken from https://www.arduino.cc/en/Tutorial/LibraryExamples/WiFiNINAWiFiWebServer
  // Modified to return json instead of html
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/json");
          // allow any origin cors
          client.println("Access-Control-Allow-Origin: *");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          snprintf(print_buffer, 200, "{\"ax\": \"%.2f\", \"ay\": \"%.2f\", \"az\": \"%.2f\", \"rx\": \"%.2f\", \"ry\": \"%.2f\", \"rz\": \"%.2f\"}", ax, ay, az, rx, ry, rz);
          client.println(print_buffer);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
  }
  delay(10);
  client.stop();
}
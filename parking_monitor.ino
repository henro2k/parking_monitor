/*
  Parking indicator for the XIAO 0.96" IPS Display (ESP32-S3 Plus)

  Behavior
    1. On boot, connect to WiFi and wait for an IP address (DHCP).
    2. Once the IP is assigned, fill the screen GREEN.
    3. Listen on HTTP port 80. A request for /parking.html turns the screen RED.
    4. Pressing USR1 (D6) or USR2 (D7) turns the screen GREEN again.

  Board:   Tools > Board > esp32 > XIAO_ESP32S3_PLUS  (esp32 core 3.3.11)
  Library: Seeed_GFX2 (manual ZIP install)
*/

#include <WiFi.h>
#include <WebServer.h>

// ---------------------------------------------------------------------------
// DISPLAY SETUP -- copy these two things from Seeed's "Hello, XIAO" example
// (xiao_esp32s3_096_hello.ino) and paste them here:
//   1. the #include line(s) for Seeed_GFX2
//   2. the line that declares the `display` object
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <Seeed_GFX.h>
#include "board/boards/XIAO_LCD_Board.h"
#include "driver/tft/Driver_ST7789.h"
#include "panel/Panel_TFT.h"

Seeed_GFX display;


// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------
const char* WIFI_SSID = "";            // Add SSID to connect to
const char* WIFI_PASS = "";            // leave empty for an open network

const int BTN_USR1 = D6;               // active-low
const int BTN_USR2 = D7;               // active-low

// RGB565 colors
const uint16_t COLOR_BLACK = 0x0000;
const uint16_t COLOR_GREEN = 0x07E0;
const uint16_t COLOR_RED   = 0xF800;

WebServer server(80);
uint16_t currentColor = 0x1234;        // arbitrary value so the first call always draws

// Fill the whole screen, but only redraw when the color actually changes.
void showColor(uint16_t color) {
  if (color == currentColor) return;
  currentColor = color;
  display.fillScreen(color);
}

void handleParking() {
  Serial.println("parking.html requested -> RED");
  showColor(COLOR_RED);
  server.send(200, "text/html",
              "<!DOCTYPE html><html><head><title>Parking</title></head>"
              "<body><h1>Parking</h1></body></html>");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_USR1, INPUT_PULLUP);
  pinMode(BTN_USR2, INPUT_PULLUP);

  // Display init (taken from Seeed's wiki examples)
  display.begin<Board_XIAO_0inch96_LCD<13, 12>,
                Config_Seeed_0inch96_LCD_ST7789>();
  showColor(COLOR_BLACK);              // black while connecting

  // Connect to WiFi and wait for a DHCP-assigned IP address
  WiFi.mode(WIFI_STA);
  if (strlen(WIFI_PASS) > 0) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  } else {
    WiFi.begin(WIFI_SSID);
  }

  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  Serial.print("Got IP address: ");
  Serial.println(WiFi.localIP());

  showColor(COLOR_GREEN);              // IP assigned -> green

  // Web server on port 80
  server.on("/parking.html", HTTP_GET, handleParking);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server listening on port 80");
}

void loop() {
  server.handleClient();

  // Either user button resets the display to green
  if (digitalRead(BTN_USR1) == LOW || digitalRead(BTN_USR2) == LOW) {
    showColor(COLOR_GREEN);
  }
}

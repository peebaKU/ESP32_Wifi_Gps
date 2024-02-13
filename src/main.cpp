#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../lib/connect_Wifi.h"

//Oled 128x64
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//Wifi
const char *SSID = "ship";
const char *PASSWORD = "b123456789";


void setup() {
  Serial.begin(115200);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.printf("ssid: %s", SSID);
  display.setCursor(0, 20);
  display.printf("pass: %s", PASSWORD);
  display.display(); 

  connecnt_Wifi(SSID, PASSWORD);
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("connected..");
  display.display(); 


}


void loop() {
}


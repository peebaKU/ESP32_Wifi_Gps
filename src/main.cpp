#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <stdio.h>
#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include "../lib/connect_Wifi.h"
#include "../lib/convert_double_to_string.h"

#define led_sos 19
#define StatusShip_Pin 23
bool status_sos = false;   
bool status_ship = false; 
//Button Interrupt GPIO2
struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button1 = {2, 0, false};

unsigned long button_time = 0;  
unsigned long last_button_time = 0; 

void IRAM_ATTR isr() {
  button_time = millis();
  if (button_time - last_button_time > 250)
  {
    button1.numberKeyPresses++;
    button1.pressed = true;
    last_button_time = button_time;
  }
}

void IRAM_ATTR IO_INT_ISR()
{
  status_ship=!status_ship;
  digitalWrite(LED_BUILTIN, status_ship);
}

//Oled 128x64 SCL=> GPIO22 , SDA=>  GPIO21
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//Wifi
const char *SSID = "ship";
const char *PASSWORD = "b123456789";

//GPS GPIO17=>Rx, GPIO16=>Tx
#define GPS_BAUDRATE 9600
TinyGPSPlus gps;  
int connect_gps();


void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE);
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
  display.println("Connected..");
  display.display(); 

  //Init GPS
  display.clearDisplay();
  display.setCursor(0, 5);
  display.printf("GPS");
  display.setCursor(0, 20);
  display.println("Connecting..");
  display.display(); 

  while (connect_gps())

  //intterrupt
    Serial.begin(115200);
    pinMode(button1.PIN, INPUT_PULLUP);
    attachInterrupt(button1.PIN, isr, RISING);
    pinMode(led_sos, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(StatusShip_Pin, INPUT);
    attachInterrupt(StatusShip_Pin, IO_INT_ISR, FAIL); 
}


void loop() {
    digitalWrite(led_sos,status_sos);
    if (button1.pressed) {
        Serial.printf("Button has been pressed %u times\n", button1.numberKeyPresses);
        button1.pressed = false;
        status_sos=!status_sos;
    }
    if (Serial2.available() > 0) {
      if (gps.encode(Serial2.read())) {
        if (gps.location.isValid()) {
          char* lat = doubleToString(gps.location.lat());
          char* lng = doubleToString(gps.location.lng());     
          Serial.print(F("- latitude: "));
          Serial.println(lat);
          Serial.print(F("- longitude: "));
          Serial.println(lng);
          display.clearDisplay();
          display.setCursor(0, 5);
          display.printf("lat: %s",lat);
          display.setCursor(0, 20);
          display.printf("lng: %s",lng);
          display.display();
        }else {
          Serial.println(F("Disconnect Gps"));
        }
        Serial.println();
    }
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}


int connect_gps()
{
    if (Serial2.available() > 0) {
      if (gps.encode(Serial2.read())) {
        if (gps.location.isValid()) {
          return 0;
        }else {
          Serial.print(F("."));
        }
    }
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
  return 1;
}
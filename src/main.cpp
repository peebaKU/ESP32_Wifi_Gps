#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include "../lib/connect_Wifi.h"

struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button1 = {2, 0, false};

//variables to keep track of the timing of recent interrupts
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



//Oled 128x64 SCL=> GPIO22 , SDA=>  GPIO21
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//Wifi
const char *SSID = "ship";
const char *PASSWORD = "b123456789";

//GPS
int connect_gps();
static const int RXPin = 22, TXPin = 21;
static const uint32_t GPSBaud = 4800;
// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
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

  while (connect_gps) {
    delay(500);
    Serial.print(".");
  }

  //intterrupt
    Serial.begin(115200);
    pinMode(button1.PIN, INPUT_PULLUP);
    attachInterrupt(button1.PIN, isr, RISING);
}

void loop() {
    if (button1.pressed) {
        Serial.printf("Button has been pressed %u times\n", button1.numberKeyPresses);
        button1.pressed = false;
    }
}

int connect_gps()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    return 0;
  }
  return 1;
}

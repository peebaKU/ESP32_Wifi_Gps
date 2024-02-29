#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include "../lib/connect_Wifi.h"
#include "../lib/convert_double_to_string.h"
#include "../lib/patch_location.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

#define Led_status 19
#define StatusShip_Pin 23
bool status_sos = false;   
bool status_ship = false;
bool status_help = false; 
int count_sos = 0;
double num_lat;
double num_lng;

const String boatID = "65e08c51d545c2aab1eee3a8";
const String baseURL = "https://boat-protector-backend.onrender.com/";
const String UpdateLocation_Url = baseURL+"boats/"+boatID+"/position";


int timestamp = 9999999;

//Button Interrupt GPIO18
struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button1 = {18, 0, false};

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
  digitalWrite(Led_status, status_ship || status_sos);

}


//Oled 128x64 SCL=> GPIO22 , SDA=>  GPIO21
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//Wifi
const String SSID = "ship";
const String PASSWORD = "b123456789";

//GPS GPIO17=>Rx, GPIO16=>Tx
#define GPS_BAUDRATE 9600
TinyGPSPlus gps;  
int connect_gps();
void Task1code( void * pvParameters );
void Task2code( void * pvParameters );

void sendPatchRequest(String url, int timestamp, double latitude, double longitude);

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

  // while (connect_gps())

  //intterrupt
    Serial.begin(115200);
    pinMode(Led_status, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(button1.PIN, INPUT_PULLUP);
    attachInterrupt(button1.PIN, isr, FALLING);
    pinMode(StatusShip_Pin, INPUT);
    attachInterrupt(StatusShip_Pin, IO_INT_ISR, FAIL);
    delay(500); 
    
    xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 
  
  xTaskCreatePinnedToCore(
                    Task2code,  
                    "Task2",     
                    10000,       
                    NULL,        
                    1,           
                    &Task2,      
                    1);         
    delay(500); 
}


void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  while(true){
    if (button1.pressed) {
        button1.pressed = false;
        status_sos=!status_sos;
        digitalWrite(Led_status, status_ship || status_sos);
    }
    delay(10);
  } 
  
}

void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

   while(true){
    if (Serial2.available() > 0) {
      if (gps.encode(Serial2.read())) {
        if (gps.location.isValid()) {
          String lat = (String)gps.location.lat();
          String lng = (String)gps.location.lng();
          num_lat = gps.location.lat();
          num_lng = gps.location.lng();
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
          sendPatchRequest(UpdateLocation_Url, timestamp, num_lat, num_lng);
        }else {
          Serial.println(F("Disconnect Gps"));
        }
        Serial.println(status_sos);
        Serial.println(status_ship);
        delay(1000);
    }
  }
  
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
  }
}




void loop() {
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

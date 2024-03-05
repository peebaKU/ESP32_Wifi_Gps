#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <stdio.h>
#include <QRCode.h>
#include <string.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include "../lib/ConnectWifi.h"
#include "../lib/ConvertDoubleToString.h"
#include "../lib/GenerateQR.h"
#include "../lib/PatchLocation.h"
#include "../lib/GetStatusShip.h"
#include "../lib/PostStatus.h"
#include "../lib/setStatusToNormal.h"
#include "../lib/CheckID.h"
#include "../lib/shortLink.h"
#include "../lib/CheckLogin.h"


TaskHandle_t Task1;
TaskHandle_t Task2;

#define Led_status 19
#define StatusShip_Pin 23
String StatusShip = "normal";
bool status_sos = false;   
bool status_ship = false;
bool status_help = false;
double num_lat = 0.00;
double num_lng = 0.00;

String boatID ;
const String baseURL = "https://boat-protector-backend.onrender.com/";
const String GetStatusShip_Url = baseURL+"boats/"+boatID;
const String UpdateLocation_Url = baseURL+"boats/"+boatID+"/position";
const String UpdateStatus_Url = baseURL+"boats/"+boatID+"/emergency";
const String PatchStatusNormal_Url = baseURL+"boats/"+boatID+"/emergency/cancel";
const String Register_Url = "https://boat-map-website.vercel.app/auth/register/";

String  textStatus = "normal";
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
  if(status_ship){
  status_sos = false;
  }
  digitalWrite(Led_status, status_ship);

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

  String Mac = connecnt_Wifi(SSID, PASSWORD);
  String ID = Mac+Mac;
  boatID = ID;
  bool firstCheckID = true;
  String link = Register_Url+ID;  
  String shortLink = makeTinyURL(link);
    const int length = shortLink.length(); 
  
    // declaring character array (+1 for null terminator) 
    char* char_array = new char[length]; 
  
    // copying the contents of the 
    // string to char array 
    strcpy(char_array, shortLink.c_str()); 
  checkUserID(ID);
  String urlCheckID = baseURL+"boats/"+ID; 
  while(checkLogin(urlCheckID)){
      if(firstCheckID){
        generateQR(char_array,display);
        Serial.println(char_array);
        firstCheckID = false;
      }
  }
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
    attachInterrupt(StatusShip_Pin, IO_INT_ISR, FALLING);
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
        if(!status_ship){
          status_sos=!status_sos;
        }
        digitalWrite(Led_status, (status_ship || status_sos));
    }
    digitalWrite(Led_status, (status_ship || status_sos));
    delay(10);
  }
  
}

void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
   bool firstGet = true;
   while(true){
    if((status_ship || status_sos) || textStatus !="normal")//&& (num_lat != 0.00 && num_lng != 0.00)
    {

        if(!status_ship && !status_sos){
          patchStatusToNormal(PatchStatusNormal_Url);
          textStatus = "normal";
          firstGet = true;
          continue;
        }else{
          
        StatusShip = getStatusShip(GetStatusShip_Url);
        
        if(StatusShip == "normal" && firstGet){
          if(status_ship){
            textStatus = "sink";
            postMessage(UpdateStatus_Url, 1000, num_lat, num_lng, "sink");
            
          }
          else if(status_sos){
            textStatus = "SOS";
             postMessage(UpdateStatus_Url, 1000, num_lat, num_lng, "SOS");
          }
          firstGet = false;

        }
        else if(StatusShip == "normal"){
          textStatus = "normal";          
          status_ship = false;
          status_sos = false;
          firstGet = true;
          
        }else if(StatusShip == "SOS" && status_ship){                  
          textStatus = "sink";
          postMessage(UpdateStatus_Url, 1000, num_lat, num_lng, "sink");
        }else if(StatusShip == "sink" && status_sos){                      
          textStatus = "SOS";
          postMessage(UpdateStatus_Url, 1000, num_lat, num_lng, "SOS");
        }
    }
    }else{
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

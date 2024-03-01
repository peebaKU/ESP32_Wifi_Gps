#ifndef SetStatus
#define SetStatus 
int postMessage(String Url, int timestamp, double lat, double lng, String status){
    StaticJsonDocument<200> jsonDocument;
    jsonDocument["timestamp"] =  timestamp;
    jsonDocument["latitude"] =  lat;
    jsonDocument["longitude"] =  lng;
    jsonDocument["status"] = status;

    // Serialize JSON to a string
    String jsonString;
    serializeJson(jsonDocument, jsonString);
    
    // Send POST request
    HTTPClient http;
    http.begin(Url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
      status = httpResponseCode;
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
    return httpResponseCode;
}
#endif
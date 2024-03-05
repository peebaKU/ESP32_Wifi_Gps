#ifndef CheckID
#define CheckID 
int checkUserID(String ID){
    StaticJsonDocument<200> jsonDocument;
    jsonDocument["id"] =  ID;
    String Url = "https://boat-protector-backend.onrender.com/boats";
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
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
    if(httpResponseCode == 500 || httpResponseCode==201){
      return 1;
    }

    return 0;
}
#endif
#ifndef SetStatusNormal
#define SetStatusNormal 
void patchStatusToNormal(String Url){
    // Send PATCH request
    HTTPClient http;
    http.begin(Url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.PATCH({});

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
}
#endif
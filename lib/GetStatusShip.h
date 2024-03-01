
#ifndef GetStatusShip_H
#define GetStatusShip_H
String getStatusShip(String Url){

  // Parse JSON string
  StaticJsonDocument<200> doc;// Adjust the buffer size according to your JSON data
  // Send HTTP GET request
  HTTPClient http;
  http.begin(Url); // Replace with your endpoint
  int httpResponseCode = http.GET();

  // Check for a successful request
  Serial.println(httpResponseCode);
  if (httpResponseCode > 0) {
    String contentType = http.header("Content-Type, application/json");
    String payload = http.getString();
    Serial.println("Response:");
    Serial.println(payload);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else{
      String status = doc["status"];
      Serial.println(status);
      return status;
    }
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
  }

  http.end(); // Free resources
  return "Error";
}
#endif
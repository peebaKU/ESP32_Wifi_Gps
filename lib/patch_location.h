#ifndef Update_Location
#define Update_Location 
void sendPatchRequest(String url, int timestamp, double latitude, double longitude) {
  HTTPClient http;

  // Create a JSON object
  StaticJsonDocument<200> doc;
  doc["timestamp"] = timestamp;
  doc["latitude"] = latitude;
  doc["longitude"] = longitude;

  // Serialize the JSON to a string
  String payload;
  serializeJson(doc, payload);

  // Start the PATCH request
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Send the PATCH request with payload
  int httpResponseCode = http.PATCH(payload);

  // Check for successful response
  if (httpResponseCode > 0) {
    Serial.print("PATCH request sent, response code: ");
    Serial.println(httpResponseCode);

    // Read the response
    String response = http.getString();
    Serial.println("Response:");
    Serial.println(response);
  } else {
    Serial.print("Error in PATCH request, response code: ");
    Serial.println(httpResponseCode);
  }

  // End the request
  http.end();
}
#endif
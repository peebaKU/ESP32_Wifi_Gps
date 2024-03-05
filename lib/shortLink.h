#ifndef ShortUrl
#define ShortUrl
String makeTinyURL(String longURL) {//Check the current connection status
    HTTPClient http;

    String requestURL = "http://tinyurl.com/api-create.php?url=" + longURL;
    Serial.print("Requesting URL: ");
    Serial.println(requestURL);
    String payload;
    http.begin(requestURL); //Specify the URL and certificate
    int httpCode = http.GET(); //Make the request

    if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println("Response:");
      Serial.println(payload);
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end(); //Free the resources
    return payload;
}
#endif
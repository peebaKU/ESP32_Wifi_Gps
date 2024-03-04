#ifndef GenerateQR
#define GenerateQR 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
void generateQR(char *url, Adafruit_SSD1306 display){
  display.clearDisplay();
  
  // Generate QR code for google.com
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)]; // Change version here
  qrcode_initText(&qrcode, qrcodeData, 3, 0, url); // Change URL here
  
  // Calculate position and size to center the QR code on the OLED display
  int qrSize = qrcode.size;
  
  int scale = min(SCREEN_WIDTH / qrcode.size, SCREEN_HEIGHT / qrcode.size);

  int xOffset = (SCREEN_WIDTH - qrSize*scale) / 2;
  
  int yOffset = (SCREEN_HEIGHT - qrSize*scale) / 2;
  

  // Display QR code on OLED
  for (u_int8_t y = 0; y < qrSize; y++) {
    for (u_int8_t x = 0; x < qrSize; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(xOffset + x*scale, y*scale + yOffset,scale,scale,WHITE);
      }
    }
  }
  
  // Display the contents of the buffer
  display.display();
}
#endif
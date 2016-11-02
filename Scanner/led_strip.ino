//////////////////////////////
// RGB LED STRIP
/////////////////////////////

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < LedStrip.numPixels(); i++) {
    LedStrip.setPixelColor(i, c);
    LedStrip.show();
    delay(wait);
  }
  for (uint16_t i = 0; i < LedStrip.numPixels(); i++) {
    LedStrip.setPixelColor(i, 0);
    LedStrip.show();
    delay(wait);
  }
}


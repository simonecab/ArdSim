#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include <TinyGPS.h>

#define RGBPIN 5

TinyGPS gps;
SoftwareSerial ss(6, 7);
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, RGBPIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.



int media = 0;
float flatm[2];
float flonm[2];
#define NMEDIA 4
float flat[8], flon[8];
int idx = 0;
void setup()
{
  Serial.begin(9600);
  ss.begin(9600);
  flatm[0] = flatm[1] = flonm[0] = flonm[1] = 0.0;

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Serial.println("Pyramid scout ");


}

void loop()
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
float oldlat, oldlon, newlat, newlon;
Serial.println("movimento un metro");
while(!Serial.available());delay(500);while(Serial.available())Serial.read();
oldlat= 41.913872; 
oldlon= 12.574518;
newlat= 41.913870;
newlon= 12.574530;
  blink_progress(oldlat, oldlon, newlat, newlon);

 delay(1000);
 Serial.println("movimento 5 metri parallelo");
  while(!Serial.available());delay(500);while(Serial.available())Serial.read();
oldlat= 41.913872; 
oldlon= 12.574518;
newlat= 41.9139;
newlon= 12.574578;
  blink_progress(oldlat, oldlon, newlat, newlon);
 delay(1000);

  Serial.println("movimento 5 metri distanza");
 while(!Serial.available());delay(500);while(Serial.available())Serial.read();
oldlat= 41.913872; 
oldlon= 12.574518;
newlat= 41.913833;
newlon= 12.574557;
  blink_progress(oldlat, oldlon, newlat, newlon);
 delay(1000);

   Serial.println("movimento 10 metri vicino");
 while(!Serial.available());delay(500);while(Serial.available())Serial.read();
oldlat= 41.913872; 
oldlon= 12.574518;
newlat= 41.913974;
newlon= 12.574433;
  blink_progress(oldlat, oldlon, newlat, newlon);
 delay(1000);

    Serial.println("movimento 10 metri lontano");
 while(!Serial.available());delay(500);while(Serial.available())Serial.read();
oldlat= 41.913872; 
oldlon= 12.574518;
newlat= 41.913810;
newlon= 12.574606;
  blink_progress(oldlat, oldlon, newlat, newlon);
 delay(1000);

}





const float destlat  = 42.30;
const float destlon  = 12.50;
void blink_progress(float oldlat, float oldlon, float newlat, float newlon)
{
  float distanza_old = sqrt((oldlat - destlat) * (oldlat - destlat) + (oldlon - destlon) * (oldlon - destlon));
  float distanza_new = sqrt((newlat - destlat) * (newlat - destlat) + (newlon - destlon) * (newlon - destlon));
  float movimento    = sqrt((oldlat - newlat)  * (oldlat - newlat)  + (oldlon - newlon)  * (oldlon - newlon));
  Serial.println("  ");
  Serial.print("old: ");       Serial.println(distanza_old, 6);
  Serial.print("new: ");       Serial.println(distanza_new, 6);
  Serial.print("mov: ");       Serial.println(movimento, 6);
  if ((distanza_new + 0.00003) < distanza_old )  // distanza diminuita >3 metri
  {
    colorWipe(strip.Color(0, 255, 0), 50);       // Green
  } else {
    if ((distanza_new - 0.00003) > distanza_old ) // distanza aumentata >3 metri
    {
      colorWipe(strip.Color(255, 0, 0), 50);     // red
    } else {
      if (movimento > 0.000015)                        // movimento maggiore di un paio di metri
      {
        colorWipe(strip.Color(255, 100, 0), 50);      // yellow
      } else {
        colorWipe(strip.Color(100, 100, 100), 50);         // white  (poco movimento)
      }
    }
  }

}



// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}


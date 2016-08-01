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


unsigned long fix_age, tempo, date;
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

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }
#define LAT2METRI(x) ((x - 41.89) * 1852 * 60);
#define LON2METRI(x) ((x - 12.49) * 83000);
  if (newData)
  {
    unsigned long age;
    strip.setPixelColor(0, strip.Color(0, 0, 100));
    strip.show();
    gps.f_get_position(&flat[idx], &flon[idx], &age);
 //    Serial.print("LAT=");
 //   Serial.print(flat[idx] == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat[idx], 6);
  //  Serial.print(" LON=");
 //   Serial.print(flon[idx] == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon[idx], 6);
    Serial.println(" ");
    flat[idx] = LAT2METRI(flat[idx]);
    flon[idx] = LON2METRI(flon[idx]);
//    Serial.print("LAT=");
//    Serial.print(flat[idx] == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat[idx], 6);
//    Serial.print(" LON=");
//    Serial.print(flon[idx] == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon[idx], 6);
//    Serial.print(" SAT=");
//    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
//    Serial.print(" PREC=");
//    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
Serial.println(" ");
    flatm[idx >> 2] += flat[idx]; // media[0] sui primi 4, media[1] sui secondi 4
    flonm[idx >> 2] += flon[idx];


    if (idx == 3) // completed media 0, [-0,1,2,3,3] is new position; media 1 [4,4,5,6,-7] is old
    {
      float oldlat = (flatm[1] + flat[4] - flat[7]) / 4.0;
      float newlat = (flatm[0] + flat[3] - flat[0]) / 4.0;
      float oldlon = (flonm[1] + flon[4] - flon[7]) / 4.0;
      float newlon = (flonm[0] + flon[3] - flon[0]) / 4.0;
      blink_progress(oldlat, oldlon, newlat, newlon);
      flatm[1] = flonm[1] = 0.0;
    }


    if (idx == 7) // completed media 1 [-4,5,6,7,7] is new position. media 0 [0,0,1,2,-3]  is old
    {
      float oldlat = (flatm[0] + flat[0] - flat[3]) / 4.0;
      float newlat = (flatm[1] + flat[7] - flat[4]) / 4.0;
      float oldlon = (flonm[0] + flon[0] - flon[3]) / 4.0;
      float newlon = (flonm[1] + flon[7] - flon[4]) / 4.0;
      blink_progress(oldlat, oldlon, newlat, newlon);
      flatm[0] = flonm[0] = 0.0;
    }

    idx++;
    if (idx == 8) idx = 0;

  } else {
    strip.setPixelColor(0, strip.Color(100, 0,  0));
    strip.show();
    gps.stats(&chars, &sentences, &failed);
    Serial.print("\n CHARS=");
    Serial.print(chars);
    Serial.print(" SENTENCES=");
    Serial.print(sentences);
    Serial.print(" CSUM ERR=");
    Serial.println(failed);
  }


  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");

}





const float destlat  = LAT2METRI(41.892475);
const float destlon  = LON2METRI(12.490230);
void blink_progress(float oldlat, float oldlon, float newlat, float newlon)
{
  float distanza_old = sqrt((oldlat - destlat) * (oldlat - destlat) + (oldlon - destlon) * (oldlon - destlon));
  float distanza_new = sqrt((newlat - destlat) * (newlat - destlat) + (newlon - destlon) * (newlon - destlon));
  float movimento    = sqrt((oldlat - newlat)  * (oldlat - newlat)  + (oldlon - newlon)  * (oldlon - newlon));



  Serial.println("  ");
  Serial.print("old: ");       Serial.println(distanza_old, 2);
  Serial.print("new: ");       Serial.println(distanza_new, 2);
  Serial.print("movimento:     ");       Serial.println(movimento, 2);
  Serial.print("avvicinamento: ");       Serial.println(distanza_old -distanza_new, 2);
  // distanza diminuita
  if ((distanza_new + 2) < distanza_old ) strip.setPixelColor(1, strip.Color(0, 255, 0));// Green
  if ((distanza_new + 5) < distanza_old ) strip.setPixelColor(2, strip.Color(0, 255, 0));// Green
  if ((distanza_new + 10) < distanza_old ) strip.setPixelColor(3, strip.Color(0, 255, 0));// Green
  // distanza aumentata
  if ((distanza_new - 2) > distanza_old ) strip.setPixelColor(1, strip.Color(255, 0, 0));// red
  if ((distanza_new - 5) > distanza_old ) strip.setPixelColor(2, strip.Color(255, 0, 0));// red
  if ((distanza_new - 10) > distanza_old ) strip.setPixelColor(3, strip.Color(255, 0, 0));// red

  if (((distanza_new + 3) > distanza_old ) && ((distanza_new - 3) < distanza_old))
  {
    // no movimento
    if (movimento > 1.5) strip.setPixelColor(1, strip.Color(255, 125, 0));  // yellow
    if (movimento > 4) strip.setPixelColor(2, strip.Color(255, 125, 0));  // yellow
    if (movimento > 8) strip.setPixelColor(3, strip.Color(255, 125, 0));  // yellow


    if (movimento < 4)
    {
      strip.setPixelColor(1, strip.Color(100, 100, 100));  // white
    }

  }
  strip.show();
  delay(300);
  colorWipe(strip.Color(0, 0, 0), 300);
}





// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {

  for (uint16_t i = 1; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
    delay(wait);
  }


}

#include <SoftwareSerial.h> 
SoftwareSerial modem(6,7); 
void setup() 
{ Serial.begin(9600); modem.begin(9600); 
Serial.println("GPS Test"); delay(2000);

}
 
void loop() { 
  while (modem.available() > 0) 
  Serial.write(modem.read()); 
  while (Serial.available() > 0) modem.write(Serial.read()); 
  } 

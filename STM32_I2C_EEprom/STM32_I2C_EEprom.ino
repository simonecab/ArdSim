#include <Wire.h>



#ifdef STM32
#define pinSDA PB4
#define pinSCL PB5

TwoWire Mywire(pinSCL, pinSDA);
#else
#define Mywire Wire
#endif

void writeByte
(
  byte    address,
  byte    data
)
{ int error;
  Mywire.beginTransmission(0x50);
  Mywire.write( address);
  Mywire.write( data);
  error = Mywire.endTransmission();
  if (error == 0)
    Serial.print("I2C x device found ");
  delay(5);
}

byte readByte (     byte address ) {
  Mywire.beginTransmission((byte)(0x50));
  Mywire.write(address);
  Mywire.endTransmission();
  Mywire.requestFrom( (byte)0x50, (byte)1);
  byte data = 0;
  delay(5);
  if (Mywire.available())
  {
    data = Mywire.read();
  } else {
    Serial.print("no data ");
  }
  return data;
}




void setup() {
  // put your setup code here, to run once:
  // Initialize serial communication.
  Serial.begin(9600);
  delay(6000);
  Mywire.begin();
  delay(6000);

  Serial.println("starting 1 ... ");
  while (1) {
    Serial.println("write ");
    delay(100);
    writeByte (0x0, 0xBB);
    delay(5);
    writeByte (0x7, 0x77);
    delay(5);
    Serial.println("read ");
    Serial.println(readByte(0x7), HEX);
    //           delay(100);
    Serial.println(readByte(0x7), HEX);
    //          delay(100);
    Serial.println(readByte(0x0), HEX);
    delay(100);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

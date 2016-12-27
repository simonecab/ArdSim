#include <Wire.h>


#define OLED_address  0x3C

int pinSDA = PB4;
int pinSCL = PB5;

TwoWire MyWire(pinSCL, pinSDA);

void writeByte
(
    word    address,
    byte    data
){
    MyWire.beginTransmission(0x57);
    MyWire.write((unsigned char)(address & 0xFF));
    MyWire.write((unsigned char)data);
    MyWire.endTransmission();
}

byte readByte (     word address ){
    Wire.beginTransmission((byte)(0x57));
    Wire.write((unsigned char)address & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom((byte)(0x57), (byte)1);
    byte data = 0;
    if (Wire.available())
    {
        data = Wire.read();
    }
    return data;
}




void setup() {
  // put your setup code here, to run once:
    // Initialize serial communication.
    Serial.begin(9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
      delay(2000);
    Serial.println("starting ... ");
  writeByte (0x0,0xAA);
  writeByte (0x7,0x77);
        Serial.println(readByte(0x7));
                Serial.println(readByte(0x7));
                        Serial.println(readByte(0x0));
}

void loop() {
  // put your main code here, to run repeatedly:

}


//////////////////////////////
// SYSTEM UTILITIES
/////////////////////////////
#ifdef UTILITIES
void EraseEEprom()
{
  int Eptr;
  Serial.println(F("Erase EEprom"));
  for (Eptr = 0; Eptr < 510; Eptr++)EEPROM[Eptr] = 0;
}



//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void EEPROMWriteInt(int p_address, int p_value)
{
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int EEPROMReadInt(int p_address)
{
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}



int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif



void set_register(int address, unsigned char r, unsigned char v) {
  Wire.beginTransmission(address);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}



unsigned char get_register(int address, unsigned char r) {
  unsigned char value = 0;
  Wire.beginTransmission(address);    // Get the slave's attention, tell it we're sending a command byte
  Wire.write(r);                               //  The command byte 
  Wire.endTransmission();                  // "Hang up the line" so others can use it (can have multiple slaves & masters connected)

  Wire.requestFrom(address, 1);         // Tell slave we need to read 1byte from the current register
  value = Wire.read();
  Wire.endTransmission();                  // "Hang up the line" so others can use it (can have multiple slaves & masters connected)
  return value;

}


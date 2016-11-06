
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

void setup()
     {
     Serial.begin(9600);
     
     EEPROMWriteInt(0, 0xABCD);
     
     Serial.print("Read the following int at the eeprom address 0: ");
     Serial.println(EEPROMReadInt(0), HEX);
     }


int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

//////////////////////////////
// ON BOARD LED
/////////////////////////////
void blink(int tempo, int velocita)
{
  long int start = millis();
  while (millis() < (start + 1000 * (long int)tempo))
  {
    digitalWrite(LEDPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay( velocita);            // wait for a time
    digitalWrite(LEDPIN, LOW);    // turn the LED off by making the voltage LOW
    delay( velocita);
  }
}



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


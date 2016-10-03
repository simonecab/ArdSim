#include <SoftwareSerial.h>
SoftwareSerial modem(8,9); // modem(rx,tx);
#define GSM_BOOT_PIN 3 // NEOWAY BOOT PIN
void setup()
{
  pinMode(10, INPUT);
  pinMode(GSM_BOOT_PIN, OUTPUT);
  digitalWrite(GSM_BOOT_PIN, LOW);
  Serial.begin(9600); 
  Serial.println("M590E Test. ATTENZIONE A CAUSA DEL BOOT QUESTO PROGRAMMA FUNZIONA OGNI 2 RESET"); delay(2000);
/* Volendo provare diversi baudrate  */

  prova(2400L);
  prova(4800L);
  prova(9600L);
  prova(14400L);
  prova(19200L);
  prova(28800L);
  prova(38400L);
  prova(57600L);
  prova(115200L);
  prova(38400L);
//  prova(19200L);
//    prova(9600L);
// AT+IPR=<baud rate>
  prova(9600L);





}

void loop() {
  // if(modem.overflow())  Serial.println("overflow");
  while (modem.available() > 0)
  {
    Serial.write(modem.read());
    //delay(10);
  }
  while (Serial.available() > 0)
  {
    modem.write(Serial.read());
    //    if (modem.available() > 0)
    //      Serial.write(modem.read());
  }
}




void  prova(long int baud)
{
  modem.end();
  modem.begin(baud);
  Serial.println(baud);
  modem.println("AT");
  delay (1000); while (modem.available() > 0)    Serial.write(modem.read());
}

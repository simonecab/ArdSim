#include <SoftwareSerial.h> 
SoftwareSerial modem(6,7); 
void setup() 
{ Serial.begin(9600); 
Serial.println("Please enter NMEA command with initial $, but without trailing star EG: $PMTK161,0"); 

}
char buff[100];
void loop() { 
  int i=0;
  int j;
  char chk=0;
  delay(100);
  while (Serial.available() > 0){ buff[i++] = Serial.read(); }
   if(i)  Serial.println(i);
      if(i)  Serial.write("$");
  for(j=1;j<i-2;j++) { Serial.write(buff[j])  ; chk ^= buff[j] ; }
  if(i) { Serial.write("*"); Serial.println((int)chk,HEX); }
  } 

  

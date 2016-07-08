#include <SoftwareSerial.h> 
SoftwareSerial modem(6,7); 
void setup() 
{ Serial.begin(9600); modem.begin(9600); 
Serial.println("GPS Test"); delay(2000);
//Turning off all GPS NMEA strings apart from GPGGA on the uBlox modules
modem.print("$PUBX,40,GLL,0,0,0,0*5C\r\n");
modem.print("$PUBX,40,ZDA,0,0,0,0*44\r\n");
modem.print("$PUBX,40,VTG,0,0,0,0*5E\r\n");
modem.print("$PUBX,40,GSV,0,0,0,0*59\r\n");
modem.print("$PUBX,40,GSA,0,0,0,0*4E\r\n");
modem.print("$PUBX,40,RMC,0,0,0,0*47\r\n"); 

char *str = "PUBX,40,RMC,0,0,0,0,0,0";
//Serial.println(ubxCrc(str, (unsigned int) strlen(str)),HEX);
Serial.println((nmea_generateChecksum(str)),HEX);


}
 
void loop() { 
  while (modem.available() > 0) 
  Serial.write(modem.read()); 
  while (Serial.available() > 0) modem.write(Serial.read()); 
  } 


#include <stdio.h>
#include <string.h>

unsigned short ubxCrc(char* data, unsigned int size) {
unsigned int crc_a = 0;
unsigned int crc_b = 0;
if (size > 0) {
do {
crc_a += *data++;
crc_b += crc_a;
} while (--size);
crc_a &= 0xff;
crc_b &= 0xff;
}
return (unsigned short)(crc_a | (crc_b << 8));
}

// this returns a single binary byte that is the checksum
// you must convert it to hex if you are going to print it or send it
unsigned char nmea_generateChecksum(char *strPtr)
{
int p;
char c;
unsigned char chksum;

c = strPtr[0]; // get first chr
chksum = c;
p = 1;
while ( c != 0x00 )
{
c = strPtr[p]; // get next chr
if ( c != 0x00 ) { chksum = chksum ^ c; }
p++;
}

return chksum;
}


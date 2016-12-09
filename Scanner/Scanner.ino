#include <gsmsim800.h>
#include <Wire.h>
#include "PCF8574.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>
//#include <AltSoftSerial.h>
#include <Eeprom24C04_16.h>

char TmpBuffer[200];

/////////////////////////////////////////
// PIN DEFINITION
/////////////////////////////////////////
#define SCANNER_RX  4        //can be disconnected to TX of the Serial MP3 Player module
#define SCANNER_TX  12        //connect to RX of the module

#define GSM_RX    8
#define GSM_TX    9
#define GSM_BOOT_PIN  3

#define LEDPIN        13  // default arduino LED


/////////////////////////////////////////
// GSM/FTP DEFINITION AND SETUP
/////////////////////////////////////////
SoftwareSerial GsmSerial(GSM_RX, GSM_TX); // RX, TX GSM
//AltSoftSerial GsmSerial;
GSMSIM GSMSIM(GSM_BOOT_PIN, TmpBuffer, sizeof(TmpBuffer),  GsmSerial);


/////////////////////////////////////////
// PCF8574 DEFINITION
/////////////////////////////////////////
PCF8574 PCF(0x20);



/////////////////////////////////////////
// APP DEFINITION
/////////////////////////////////////////
#define UPDATETIMEFINAL 600     // Final update interval 
#define UPDATETIMEINITIAL 60;   // initial update interval

long int NextConnectionTime = 15;

/////////////////////////////////////////
// EEPROM24C04 DEFINITION
/////////////////////////////////////////
#define EEPROM_ADDRESS 0x50
static Eeprom24C04_16 eeprom(EEPROM_ADDRESS);




/////////////////////////////////////////
// SETUP AND SELF TEST
/////////////////////////////////////////

void setup()
{
  int i;
  Serial.begin(9600);

  //***************************************
  //SETUP GSM
  //***************************************
  GsmSerial.begin(19200);
  GsmSerial.listen();
  boot();



  //***************************************
  //SETUP GPIO CONTROLLER
  //***************************************
  Serial.print ("TEST PCF8574 ... ");

  PCF.write8(0x0);
  if(PCF.read8() != 0) Serial.println("PCF error reading 0");
  PCF.write(4, 1); delay(100);
  if(PCF.read8() != 16) Serial.println("PCF error reading 16");
  // while (1) { PCF.write8(0x0); delay(2500);   Serial.println(PCF.read8(),HEX); PCF.write8(0xFF); delay(2500);   Serial.println(PCF.read8(),HEX);}
  Serial.println("  COMPLETED");


  //***************************************
  //EEPROM
  //***************************************
  Serial.print("TEST EEPROM ... ");
  eeprom.initialize();
  for (i = 0; i < 100; i++)
  {
    TmpBuffer[i] = 0;
  }
  eeprom.writeBytes(0, 100, (byte *) TmpBuffer);
  eeprom.readBytes(0, 100, (byte *) TmpBuffer);
  for (i = 0; i < 100; i++)
  {
    if (TmpBuffer[i]) {
      Serial.println("Eeprom zeroing error");
      break;
    }
  }
  Serial.print("ZEROED ... ");

  eeprom.writeByte(4, 0x37);     delay(100);
  eeprom.writeByte(31, 0x53);    delay(100);

  if (eeprom.readByte(4) != 0x37 ||  eeprom.readByte(31) != 0x53)  Serial.println("Eeprom Error");
  else   Serial.println(" COMPLETED");


  //***************************************
  //SETUP ON BOARD LED
  //***************************************

  pinMode(LEDPIN, OUTPUT);
  //blink(10, BLINK_FAST);   //blink(10, BLINK_NORM); //SECONDI DI BLINK  e VELOCITA'


  //***************************************
  //END SETUP
  //***************************************

  printHelp();


}
void printHelp()
{
  Serial.print(F("\n.AT cmd, h(alt), p(ost),  g(et), G(PS log), b(oot), \n"));
  Serial.println(F("cmd# "));
}


void loop() // run over and over
{


  //////////////////////////////////////////////////////
  // CONSOLE COMMAND PROCESSING
  //////////////////////////////////////////////////////
  if (Serial.available())
  {
    GsmSerial.listen();
    char a = Serial.read();
    switch (a)
    {
      case '.': GSMSIM.ProxyGSM();  break;
      case 'b': boot(); break;
      case 'e': endGSM(); break;
      case 'g': get(); break;
      case 'G': Serial.println(GPS()); break;
      case 'p': post("SCANID1,  SEQN,TIME,Lat,Lon"); break;
      default: printHelp();


    }
    {
      long int start = millis();
      while (millis() < start + 200)
        if (Serial.available()) Serial.read();
    }

    Serial.println(F("cmd# "));

  }


}//end loop


int  boot()
{
  int retry;
  if ( GSMSIM.GSM_AT(F("AT+CREG?")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+CSQ")) != GSMOK) return GSMERROR ;
  do {
    if ( GSMSIM.GSM_AT(F("AT+CGATT?")) != GSMOK) return GSMERROR ;
    delay(500);
  } while (!strstr(TmpBuffer, "1"));

  if ( GSMSIM.GSM_AT(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\"")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+SAPBR=3,1,\"APN\",\"ibox.tim.it\"")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+CGNSPWR=1")) != GSMOK) return GSMERROR ;

  retry = 5;


  if ( GSMSIM.GSM_AT(F("AT+SAPBR=2,1")) != GSMOK) return GSMERROR ;
  if (strstr(TmpBuffer, "0.0.0.0"))
  {
    while (--retry &&  ( GSMSIM.GSM_AT(F("AT+SAPBR=1,1")) != GSMOK))delay(2000);
    if (!retry)return GSMERROR ;
    if (  GSMSIM.GSM_AT(F("AT+HTTPINIT")) != GSMOK) return GSMERROR ;
  }



  //endGSM();

}

char  *GPS()
{

  GsmSerial.println(F("AT+CGNSINF"));
  GSMSIM.GSM_Response(2);

  return TmpBuffer + 10;
}


int  get()
{

  GSMSIM.HTTP_get(8, 90, F("http://184.73.165.170/b2bg/xxx.log"));
}

void endGSM()
{
  GSMSIM.GSM_AT(F("AT+HTTPTERM")) ;
  GSMSIM.GSM_AT(F("AT+SAPBR=0,1")) ;
}

int  post(char *msg)
{

  char ScanBuffer[70];

  unsigned char c = 0 ;
  int i;
  for (i = 0; i < 64; i++) ScanBuffer[i] = 0;
  strcpy(ScanBuffer, msg);

  for (i = 0; i < 64; i++) c ^= ScanBuffer[i];
  ScanBuffer[64] = c;

  GSMSIM.HTTP_post(ScanBuffer, 65, F("http://184.73.165.170/b2bg/xxx.php" ));
  Serial.println(TmpBuffer);
  if ((int) * (strstr(TmpBuffer, "#") + 1) != (int) ScanBuffer[64]) Serial.println("Disastro!\n"); else Serial.println("Successo!\n");
}





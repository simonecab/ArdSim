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
#define SCANNER_TX      3
#define SCANNER_RX      4
#define SCANNER_AIM     12
#define SCANNER_TRIGGER 13
#define SCANNER_PWRDOWN 5

#define SCAN_BUTTON     2

#define GSM_RX    8
#define GSM_TX    9
#define GSM_BOOT_PIN  10


// AltSoftSerial - Can be sensitive to interrupt usage by other libraries.
// SoftwareSerial Can have multiple instances on almost any pins, but only 1 can be active at a time.
//               Can interfere with other libraries or HardwareSerial if used at slower baud rates.
//               Can be sensitive to interrupt usage by other libraries.
// see also: https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html


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
  //SETUP PIN
  //***************************************

  pinMode(SCANNER_AIM, OUTPUT);
  pinMode(SCANNER_TRIGGER, OUTPUT);
  pinMode(SCANNER_PWRDOWN, INPUT);
  pinMode(SCAN_BUTTON, OUTPUT);
  pinMode(GSM_BOOT_PIN, OUTPUT);



  //***************************************
  //SETUP GSM
  //***************************************
  GsmSerial.begin(19200);
  GsmSerial.listen();
  GSMSIM.BootGSM();


  //***************************************
  //SETUP GPIO CONTROLLER
  //***************************************
  Serial.print ("TEST PCF8574 ... ");

  PCF.write8(0x0);
  if (PCF.read8() != 0) Serial.println("PCF error reading 0");
  PCF.write(4, 1); delay(100);
  if (PCF.read8() != 16) Serial.println("PCF error reading 16");
  // while (1) { PCF.write8(0x0); delay(2500);   Serial.println(PCF.read8(),HEX); PCF.write8(0xFF); delay(2500);   Serial.println(PCF.read8(),HEX);}
  Serial.println("  COMPLETED");


  //***************************************
  //EEPROM
  //***************************************
  Serial.print("TEST EXTERNAL EEPROM ... ");
  eeprom.initialize();
  eeprom.writeByte(4, 0x37);     delay(100);
  eeprom.writeByte(31, 0x53);    delay(100);
  if (eeprom.readByte(4) != 0x37 ||  eeprom.readByte(31) != 0x53)  Serial.println("Eeprom Error 1");
  for (i = 0; i < 100; i++)    TmpBuffer[i] = i;
  eeprom.writeBytes(0, 100, (byte *) TmpBuffer);
  eeprom.readBytes(0, 100, (byte *) TmpBuffer + 1);
  for (i = 0; i < 100; i++)
  {
    if (TmpBuffer[i + 1] != i) {
      Serial.println("Eeprom  error 2");
      break;
    }
    TmpBuffer[i] = 0;
  }
  eeprom.writeBytes(0, 100, (byte *) TmpBuffer);

  Serial.print("ZEROED ... ");


  Serial.println(" COMPLETED");



  //***************************************
  //END SETUP
  //***************************************

  printHelp();


  // TEST ONLY
  pinMode(6, INPUT_PULLUP);




}

void printHelp()
{
  Serial.print(F("\n.AT cmd, p(ost),  g(et), G(PS log), b(oot), f(fun=0)\n"));
  Serial.println(F("cmd# "));
}


void loop() // run over and over
{


  // TEST ONLY
  if (!digitalRead(6)) {
    Serial.println("SCANNNN!");
    delay(500);
  }



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
      case 'b': GSMSIM.BootGSM(); break;
      case 'g': get(); break;
      case 'G': GPS(); break;
      case 'f': Serial.println(F("low function"));GSMSIM.ConfGSM(0); break;
      case 'p': post("SCANID1,  SEQN,TIME,Lat,Lon"); break;
      default: printHelp();
    }
    { //per svuotare il buffer di input
      long int start = millis();
      while (millis() < start + 200)
        if (Serial.available()) Serial.read();
    }

    Serial.println(F("cmd# "));
  }
}//end loop


void GPS()
{
  GsmSerial.println(F("AT+CGNSINF"));
  GSMSIM.GSM_Response(2);
  Serial.println(TmpBuffer);
  strtok(TmpBuffer, ",");
  Serial.println(strtok(0, ","));// fix
  Serial.println(strtok(0, "."));// time stamp
  (strtok(0, ","));
  Serial.println(strtok(0, ","));// lat
  Serial.println(strtok(0, ","));// lon
  Serial.println(strtok(0, ","));// alt
    Serial.println("speed course mode" );
  Serial.println(strtok(0, ",")); // speed
  Serial.println(strtok(0, ",")); // course
  Serial.println(strtok(0, ",")); // mode

  Serial.println("3 DOP" );
  Serial.println(strtok(0, ",")); //HDOP
  Serial.println(strtok(0, ",")); //PDOP
  Serial.println(strtok(0, ",")); //VDOP
  Serial.println("SAT VIEW, USED");
  Serial.println(strtok(0, ",")); 
  Serial.println(strtok(0, ",")); 
  Serial.println("END");


}


int  get()
{
  GSMSIM.HTTP_get(8, 90, F("http://184.73.165.170/b2bg/xxx.log"));
  Serial.println(TmpBuffer);
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





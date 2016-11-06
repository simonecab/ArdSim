#include <gsmsim800.h>



#include <EEPROM.h>
#include <Wire.h>
#include <SoftwareSerial.h>
//#include <AltSoftSerial.h>
char TmpBuffer[200];

/////////////////////////////////////////
// PIN DEFINITION
/////////////////////////////////////////
#define AUDIO_RX  4        //can be disconnected to TX of the Serial MP3 Player module
#define AUDIO_TX  12        //connect to RX of the module

#define GPS_RX    6         // (was 6) connect to TX of module 
#define GPS_TX    7         // (was 7)connect to RX of module

// PIN POWER NEOWAY 2 e 4 CONNECTED TO ARDUINO 5V e GND congigui (2to5V  4toGND)
// PIN POWER CEPRI  4 e 2 CONNECTED TO ARDUINO 5V e GND congigui (2toGND 4toVCC)
#define GSM_RX    8      // CEPRI PIN 9   : NEOWAY PIN 14  : ITRUKG  PIN T
#define GSM_TX    9      // CEPRI PIN 10  : NEOWAY PIN 16  : ITRUKG  PIN R
#define GSM_BOOT_PIN  3       // Neoway M590 boot pin if available : CEPRI PIN 14 : NEOWAY PIN 19 : ITRUKG  PIN K
// #define GSM_BOOT_PIN  -1   // if Neoway M590 boot pin NOT available

#define LEDSTRIPPIN   5   // RGB LED Strip
#define LEDPIN        13  // default arduino LED
#define VOLTINPIN     1   // Analog input connected to intermediate battery (less than 5v)


/////////////////////////////////////////
// GPS DEFINITIONS
/////////////////////////////////////////
#include <TinyGPS.h>
SoftwareSerial GpsSerial(GPS_RX, GPS_TX); // RX, TX GPS
//SoftwareSerial GpsSerial(11, GPS_TX); // RX, TX DISABLE GPS
TinyGPS Gps;


/////////////////////////////////////////
// GSM/FTP DEFINITION AND SETUP
/////////////////////////////////////////
SoftwareSerial GsmSerial(GSM_RX, GSM_TX); // RX, TX GSM
//AltSoftSerial GsmSerial;
GSMSIM GSMSIM(GSM_BOOT_PIN, TmpBuffer, sizeof(TmpBuffer),  GsmSerial);







/////////////////////////////////////////
// APP DEFINITION
/////////////////////////////////////////
#define UPDATETIMEFINAL 600     // Final update interval 
#define UPDATETIMEINITIAL 60;   // initial update interval
int MFile = 1; // id file multipli
long int NextConnectionTime = 15;


#define REDUCE_LED 1
#define GPS_LOG 2
int option = REDUCE_LED;


#define HUNKNOWN 0
#define HERROR -1
#define HOK 1
#define HPERFECT 2
int Happy = HUNKNOWN;


#define BLINK_FAST 40
#define BLINK_NORM 300
#define BLINK_SLOW 800



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
  //***************************************
  //SETUP AND TEST GPS
  //***************************************
  pinMode(GPS_TX, OUTPUT);
  pinMode(GPS_RX, INPUT);
  GpsSerial.begin(9600);


  GpsSerial.flush();

  delay(500);
  GpsSerial.listen();
  i = 0;
  TmpBuffer[0] = 0;
  TmpBuffer[1] = 0;
  if (GpsSerial.available())
    while (millis() < 10000 && i < 190)
      while (GpsSerial.available())
      {
        TmpBuffer[i] = GpsSerial.read();
        //Serial.print(TmpBuffer[i]);
        if (TmpBuffer[0] == '$') {
          i++;
          if (TmpBuffer[1] == 'G') break;
        }
      }
  if (!strncmp(TmpBuffer, "$G", 2))Serial.println(F("GPS TEST PASSED"));
  else Serial.println(F("GPS TIMEOUT"));

  //***************************************
  //SETUP ON BOARD LED
  //***************************************

  pinMode(LEDPIN, OUTPUT);
  //blink(10, BLINK_FAST);   //blink(10, BLINK_NORM); //SECONDI DI BLINK  e VELOCITA'


  //***************************************
  //END SETUP
  //***************************************

  printHelp();
  GpsSerial.listen();

}
void printHelp()
{
  Serial.print(F("\n.AT cmd, h(alt), p(ost), l(ogin), g(ps_status), G(PS log)\
  \nr(eadFtp), b(oot), c(onf_gsm), s(tatus), t(est), R(ead), S(end)\n"));
  Serial.println(F("cmd# "));
}


void loop() // run over and over
{
  //////////////////////////////////////////////////////
  // GPS PROCESSING
  //////////////////////////////////////////////////////

  if (GpsSerial.available())
  {
    char c;
    c = GpsSerial.read();
    Gps.encode(c);
    if (option & GPS_LOG) Serial.print(c);
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
      case 'b': boot(); break;
      case 'e': endGSM(); break;
      case 'p': post(); break;
      default: printHelp();


    }
    {
      long int start = millis();
      while (millis() < start + 200)
        if (Serial.available()) Serial.read();
    }

    Serial.println(F("cmd# "));
    GpsSerial.listen();
  }


}//end loop


int  boot()
{
  int retry;
  if ( GSMSIM.GSM_AT(F("AT+CREG?")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+CSQ")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+CGATT?")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\"")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+SAPBR=3,1,\"APN\",\"ibox.tim.it\"")) != GSMOK) return GSMERROR ;

  retry = 5;
  while (--retry &&  ( GSMSIM.GSM_AT(F("AT+SAPBR=1,1")) != GSMOK))delay(2000);
  if (!retry)return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+SAPBR=2,1")) != GSMOK) return GSMERROR ;

  if (  GSMSIM.GSM_AT(F("AT+HTTPINIT")) != GSMOK) return GSMERROR ;
  if (    GSMSIM.GSM_AT(F("AT+HTTPPARA=\"CID\",1")) != GSMOK) return GSMERROR ;
  if (  GSMSIM.GSM_AT(F("AT+HTTPPARA=\"URL\",\"http://184.73.165.170/b2bg/xxx.log\"")) != GSMOK) return GSMERROR ; \

  GsmSerial.println(F("AT+HTTPACTION=0"));
  GSMSIM.GSM_Response(2);

  if (  GSMSIM.GSM_AT(F("AT+HTTPREAD=0,50")) != GSMOK) return GSMERROR ;
  //endGSM();

}

void endGSM()
{
  GSMSIM.GSM_AT(F("AT+HTTPTERM")) ;
  GSMSIM.GSM_AT(F("AT+SAPBR=0,1")) ;
}

int  post()
{
  int retry;
  if ( GSMSIM.GSM_AT(F("AT+CREG?")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+CSQ")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+CGATT?")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\"")) != GSMOK) return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+SAPBR=3,1,\"APN\",\"ibox.tim.it\"")) != GSMOK) return GSMERROR ;

  retry = 5;
  while (--retry &&  ( GSMSIM.GSM_AT(F("AT+SAPBR=1,1")) != GSMOK))delay(2000);
  if (!retry)return GSMERROR ;
  if ( GSMSIM.GSM_AT(F("AT+SAPBR=2,1")) != GSMOK) return GSMERROR ;

  if (  GSMSIM.GSM_AT(F("AT+HTTPINIT")) != GSMOK) return GSMERROR ;
  if (    GSMSIM.GSM_AT(F("AT+HTTPPARA=\"CID\",1")) != GSMOK) return GSMERROR ;
  if (  GSMSIM.GSM_AT(F("AT+HTTPPARA=\"URL\",\"http://184.73.165.170/b2bg/xxx.php\"")) != GSMOK) return GSMERROR ; \

  GsmSerial.println(F("AT+HTTPDATA=10,10000"));

  {
    long int start = millis();
    delay(400);
    while (GsmSerial.available())
      Serial.write(GsmSerial.read());
  }

GsmSerial.write("Un topo!  ",10);
  if ( GSMSIM.GSM_AT(F("AT")) != GSMOK) return GSMERROR ;

  GsmSerial.println(F("AT+HTTPACTION=1"));
    GSMSIM.GSM_Response(2);
    
    if (  GSMSIM.GSM_AT(F("AT+HTTPREAD=0,50")) != GSMOK) return GSMERROR ;


}



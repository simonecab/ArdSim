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


#define GSM_RX    8      // CEPRI PIN 9   : NEOWAY PIN 14  : ITRUKG  PIN T
#define GSM_TX    9      // CEPRI PIN 10  : NEOWAY PIN 16  : ITRUKG  PIN R
#define GSM_BOOT_PIN  3       // Neoway M590 boot pin if available : CEPRI PIN 14 : NEOWAY PIN 19 : ITRUKG  PIN K
// #define GSM_BOOT_PIN  -1   // if Neoway M590 boot pin NOT available

#define LEDSTRIPPIN   5   // RGB LED Strip
#define LEDPIN        13  // default arduino LED
#define VOLTINPIN     1   // Analog input connected to intermediate battery (less than 5v)




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
  }

  if (  GSMSIM.GSM_AT(F("AT+HTTPINIT")) != GSMOK) return GSMERROR ;

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
  if (  GSMSIM.GSM_AT(F("AT+HTTPPARA=\"CID\",1")) != GSMOK) return GSMERROR ;
  if (  GSMSIM.GSM_AT(F("AT+HTTPPARA=\"URL\",\"http://184.73.165.170/b2bg/xxx.log\"")) != GSMOK) return GSMERROR ; \

  GsmSerial.println(F("AT+HTTPACTION=0"));
  GSMSIM.GSM_Response(2);

  if (  GSMSIM.GSM_AT(F("AT+HTTPREAD=0,50")) != GSMOK) return GSMERROR ;
}

void endGSM()
{
  GSMSIM.GSM_AT(F("AT+HTTPTERM")) ;
  GSMSIM.GSM_AT(F("AT+SAPBR=0,1")) ;
}

int  post(char *msg)
{
  int retry;
  char ScanBuffer[70];

  unsigned char c = 0 ;
  int i;
  for (i = 0; i < 64; i++) ScanBuffer[i] = 0;
  strcpy(ScanBuffer, msg);

  for (i = 0; i < 64; i++) c ^= ScanBuffer[i];
  ScanBuffer[64] = c;

  
  if (  GSMSIM.GSM_AT(F("AT+HTTPPARA=\"CID\",1")) != GSMOK) return GSMERROR ;
  if (  GSMSIM.GSM_AT(F("AT+HTTPPARA=\"URL\",\"http://184.73.165.170/b2bg/xxx.php\"")) != GSMOK) return GSMERROR ; \

  // PREPARE TO SEND POST PAYLOAD
  GsmSerial.println(F("AT+HTTPDATA=65,10000"));

  // WAIT "DOWNLOAD" REQUEST 
  {
    int i=0;
    TmpBuffer[0]=0;
    long int start = millis();
    while( (millis()< (start + 500)) && !strstr(TmpBuffer,"DOWNLOAD"))
    {
      if(GsmSerial.available()) { TmpBuffer[i]=GsmSerial.read(); Serial.write(TmpBuffer[i]); i++ ; }
    }
  }

  // SEND POST PAYLOAD 
  GsmSerial.write(ScanBuffer, 65);

  // WAIT "OK" DOWNLOAD
  if ( GSMSIM.GSM_AT(F("")) != GSMOK) return GSMERROR ;

  // DO POST
  GsmSerial.println(F("AT+HTTPACTION=1"));
  GSMSIM.GSM_Response(2);

  // DO READ POST RETURNED BUFFER
  if (  GSMSIM.GSM_AT(F("AT+HTTPREAD=0,50")) != GSMOK) return GSMERROR ;
  Serial.println(TmpBuffer);
  if ((int) * (strstr(TmpBuffer, "#") + 1) != (int) ScanBuffer[64]) Serial.println("Disastro!\n"); else Serial.println("Successo!\n");
}





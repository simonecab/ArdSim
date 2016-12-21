#include "Arduino.h"
#include "gsmsim800.h"




/////////////////////////////////////////
// GSM/FTP DEFINITION
/////////////////////////////////////////

long int GSMErrors = 0;
#define GSM_RX    8      // CEPRI PIN 9   : NEOWAY PIN 14
#define GSM_TX    9      // CEPRI PIN 10  : NEOWAY PIN 16
//#define GSM_2400BAUD_PATCH  // not working with softserial!

static char *ExtBuffer;
static int BootPin = 0;

/////////////////////////////////////////
// SETUP GSM
/////////////////////////////////////////

GSMSIM::GSMSIM(int BOOT_PIN, char*buffer, int ExtBufferSize, Stream& S)
  : m_gsmSerial(S)
{
  ExtBuffer = buffer;
  if (ExtBufferSize < EXTBUFFERSIZE) ExtBuffer = 0;

  //***************************************
  //SETUP GSM
  //***************************************
  pinMode(BOOT_PIN, OUTPUT);    
  digitalWrite(BOOT_PIN, 0);
  BootPin = BOOT_PIN;
}

//////////////////////////////////////////////////////
// CONFIGURE GSM CFUN
////////////////////////////////////////////////////////
int GSMSIM::ConfGSM(int cfun)
{
if(cfun!=0)
{
    if ( GSM_AT(F("AT+CFUN=1")) != GSMOK) return GSMERROR ;
}
else
{
	if ( GSM_AT(F("AT+CFUN=0")) != GSMOK) return GSMERROR ;
}
return GSMOK;

}

////////////////////////////////////////////////////

int  GSMSIM::ReadSMS()
{

  int ret;
  char *p, *p1;
  Serial.println(F(" - Read SMS "));
  //m_gsmSerial.listen();
  GSM_AT(F("AT+CSQ"));
  if ( GSM_AT(F("AT+CMGL=4")) != GSMOK) return GSMERROR;
  return GSMOK;
}
////////////////////////////////////////////////////

int  GSMSIM::DeleteAllSMS()
{
  //m_gsmSerial.listen();
  if ( GSM_AT(F("AT+CMGD=0,4")) != GSMOK) return GSMERROR;
  return GSMOK;
}

////////////////////////////////////////////////////

int  GSMSIM::LoginFTP()
{


  return GSMOK;
}

////////////////////////////////////////////////////

int  GSMSIM::StatusFTP()
{
  return GSMOK;
}

////////////////////////////////////////////////////

int GSMSIM::PutFTP(const char *file, char *obuf)
{

  return GSMOK;
}

////////////////////////////////////////////////////

char *GSMSIM::ReadFTP(char *filename)
{


  return ExtBuffer;
}

////////////////////////////////////////////////////

int GSMSIM::GSM_AT(const __FlashStringHelper * ATCommand)
{
  int i = 0;
  int done = GSMUNKNOWN;
  long int start = millis();

  ExtBuffer[0] = 0;
  Serial.println(ATCommand);

  m_gsmSerial.println(ATCommand);
  while ((millis() < (start + 4000)) && (done == GSMUNKNOWN))
  {
    if (m_gsmSerial.available()) ExtBuffer[i++] = m_gsmSerial.read();
    if (i > 2) if (!strncmp(ExtBuffer + i - 3, "OK", 2)) done = GSMOK;
    if (i > 5) if (!strncmp(ExtBuffer + i - 6, "ERROR", 5)) done = GSMERROR;
    if (i > EXTBUFFERSIZE) {
      Serial.println(F("BUFFER FULL\n"));
      done = GSMERROR;
    }
  };
  ExtBuffer[i] = 0;
  if (done == GSMERROR || done == GSMUNKNOWN  )
  {
    Serial.println(F(" - GSM: "));
    if (done == GSMUNKNOWN)  Serial.println(F(" TIMEOUT, BOOT? "));
    GSMErrors++;
  }
  Serial.print(ExtBuffer);
  //Serial.println("");
  start = millis();
  while (millis() < start + 50) if (m_gsmSerial.available()) {
      Serial.write(m_gsmSerial.read());
    }
  return done;
}

////////////////////////////////////////////////////

int GSMSIM::GSM_Response(int n)  {
  long int start = millis(); long int timeout = 20000;
  char a = 0; int pcnt = 0; int i = 0;

 // Serial.print (n); Serial.println(F(" RESP : "));
  //if (m_gsmSerial.overflow())      Serial.println(F("OVERFLOWWWWWW\n");
  while (millis() < start + timeout)
  {
    if (m_gsmSerial.available())
    {
      a = m_gsmSerial.read();
      Serial.write(a);

      if (a == '+') {
        pcnt++;
        if (pcnt == n)  {
          start = millis();
          timeout = 2000;
        }
      }
      ExtBuffer[i] = a;
      if (i < 199) i++;
    }
  }

  if (i > EXTBUFFERSIZE) {
    Serial.println(F("BUFFER FULL\n"));
    GSMErrors++;
    return GSMERROR;
  }
  ExtBuffer[i] = 0;
  //Serial.println(F("\nEND RESP"));

  if (pcnt < n   ||  (strstr(ExtBuffer, "Error") > 0)) {
    Serial.println(F("\nTimeout / Error response"));

    GSMErrors++;
    return GSMERROR;
  } else {
    return GSMOK;
  }
}

////////////////////////////////////////////////////

void GSMSIM::SendSMS(char *number, char* message)
{

  long int start;
  //m_gsmSerial.listen();

  //GSM_AT(F("AT+CMGD=4[,<delflag>]

  sprintf(ExtBuffer, "- SEND SMS \"%s\" TO %s:", message, number);
  Serial.println(ExtBuffer);
#ifdef GSM_2400BAUD_PATCH
  {
    int i = 8;
    while (i--) m_gsmSerial.write(' ');
  }
#endif
  m_gsmSerial.write("AT+CMGS=");
  delay(100);
  sprintf (ExtBuffer, "\"%s\"\r", number); // quoted number
  m_gsmSerial.println(ExtBuffer);
  delay(100);
  //mySerial.println("\"+393356930892\"\r"); // Replace x with mobile number
  m_gsmSerial.write(message);// The SMS text you want to send
  delay(100);
  m_gsmSerial.write((char)26);// ASCII code of CTRL+Z
  GSM_Response(2);

}

///////////////////////////////////////////////////

void GSMSIM::PowerOffGSM()
{
  long int start;
  Serial.println(F("PowerOffGSM"));
  //m_gsmSerial.listen();
  //  digitalWrite(GSM_BOOT_PIN, LOW);
  //  delay(700);
  //  digitalWrite(GSM_BOOT_PIN, HIGH);

  while ( GSM_AT(F("AT+CPWROFF")) == GSMOK)   delay(2000);   GSMErrors--;
  return;
}

////////////////////////////////////////////////////

int GSMSIM::BootGSM(char *imei)
{
  long int start;
  int retry = 3;

  
  if ( 1 )
  {
    if ( GSM_AT(F("AT")) != GSMOK) 
	{
		Serial.println(F("Booting GSM ..."));
        digitalWrite(BootPin, HIGH);
	    delay(2000);
        digitalWrite(BootPin, LOW);
		Serial.println(F("done"));
		delay(1000);
	}
  
   if ( GSM_AT(F("AT")) != GSMOK) 
	{
		Serial.println(F("Booting GSM ..."));
        digitalWrite(BootPin, HIGH);
	    delay(2000);
        digitalWrite(BootPin, LOW);
		Serial.println(F("done"));
		delay(1000);
	}
  }
  
  
  if (! ExtBuffer ) Serial.println(F("Error: Ext Buffer size too small"));
    if ( GSM_AT(F("AT+CFUN=1")) != GSMOK) return GSMERROR ;
    if ( GSM_AT(F("AT+CREG?")) != GSMOK) return GSMERROR ;
    if ( GSM_AT(F("AT+CSQ")) != GSMOK) return GSMERROR ;
    do {
      if ( GSM_AT(F("AT+CGATT?")) != GSMOK) return GSMERROR ;
      delay(500);
    } while (!strstr(ExtBuffer, "1"));

    if ( GSM_AT(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\"")) != GSMOK) return GSMERROR ;
    if ( GSM_AT(F("AT+SAPBR=3,1,\"APN\",\"ibox.tim.it\"")) != GSMOK) return GSMERROR ;
    if ( GSM_AT(F("AT+CGNSPWR=1")) != GSMOK) return GSMERROR ;

    retry = 5;


    if ( GSM_AT(F("AT+SAPBR=2,1")) != GSMOK) return GSMERROR ;
    if (strstr(ExtBuffer, "0.0.0.0"))
    {
      while (--retry &&  ( GSM_AT(F("AT+SAPBR=1,1")) != GSMOK))delay(2000);
      if (!retry)return GSMERROR ;
      if ( GSM_AT(F("AT+HTTPINIT")) != GSMOK) return GSMERROR ;
	  if ( GSM_AT(F("AT+SAPBR=2,1")) != GSMOK) return GSMERROR ;
    }

    if ( GSM_AT(F("AT+GSN")) != GSMOK) return GSMERROR ;
	strncpy(imei,ExtBuffer+9,15);
}

void GSMSIM::ProxyGSM()
{
  //m_gsmSerial.listen();
  Serial.println(F("<<"));
  long int start = millis();

  delay(100);
  while (Serial.available()) {
    char a = Serial.read();
    if (a == '|') m_gsmSerial.println("");
    else m_gsmSerial.write(a);
  }
  while (millis() < start + 5000)
    while (m_gsmSerial.available()) 
	{
      char a;
      Serial.write(a = m_gsmSerial.read());
      //  if((a<'A' || a>'Z') && (a<'a'||a>'z') && (a!='\n') && (a!='+') )
      //{Serial.print("<" );Serial.print ((unsigned char)a,HEX);Serial.println(">" );}
    }
  //  if (m_gsmSerial.overflow()) {
  //    Serial.println("SoftwareSerial overflow!");
  //  }
  Serial.println(F(">>"));
}





int  GSMSIM::HTTP_post(char *payload, int payloadSize, const __FlashStringHelper *URL)
{
  if ( GSM_AT(F("AT+HTTPPARA=\"CID\",1")) != GSMOK) return GSMERROR ;
  m_gsmSerial.print(F("AT+HTTPPARA=\"URL\",\""));
  m_gsmSerial.print(URL);
  m_gsmSerial.print("\"");
  if ( GSM_AT(F("")) != GSMOK) return GSMERROR ;

  // PREPARE TO SEND POST PAYLOAD
  m_gsmSerial.print(F("AT+HTTPDATA="));
  m_gsmSerial.print(payloadSize);
  m_gsmSerial.println(F(",10000")); // TIMEOUT


  // WAIT "DOWNLOAD" REQUEST
  {
    int i = 0;
    ExtBuffer[0] = 0;
    long int start = millis();
    while ( (millis() < (start + 500)) && !strstr(ExtBuffer, "DOWNLOAD"))
    {
      if (m_gsmSerial.available()) {
        ExtBuffer[i] = m_gsmSerial.read();
        Serial.write(ExtBuffer[i]);
        i++ ;
      }
    }
  }

  // SEND POST PAYLOAD
  m_gsmSerial.write(payload, payloadSize);

  // WAIT "OK" DOWNLOAD
  if ( GSM_AT(F("")) != GSMOK) return GSMERROR ;

  // DO POST
  m_gsmSerial.println(F("AT+HTTPACTION=1"));
  GSM_Response(2);

  // DO READ POST RETURNED BUFFER
  if (  GSM_AT(F("AT+HTTPREAD=0,50")) != GSMOK) return GSMERROR ;
  return GSMOK;
}

int  GSMSIM::HTTP_get(int dataOffset, int dataSize, const __FlashStringHelper *URL)
{
  if ( GSM_AT(F("AT+HTTPPARA=\"CID\",1")) != GSMOK) return GSMERROR ;
  m_gsmSerial.print(F("AT+HTTPPARA=\"URL\",\""));
  m_gsmSerial.print(URL);
  if ( GSM_AT(F("\"")) != GSMOK) return GSMERROR ;

  // DO GET
  m_gsmSerial.println(F("AT+HTTPACTION=0"));
  GSM_Response(2);

  // DO READ GET  RETURNED BUFFER
  m_gsmSerial.print(F("AT+HTTPREAD="));
  m_gsmSerial.print(dataOffset);
  m_gsmSerial.print(",");
  m_gsmSerial.print(dataSize);
  if (  GSM_AT(F("")) != GSMOK) return GSMERROR ;
  return GSMOK;
}




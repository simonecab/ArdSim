#include "Arduino.h"
#include "gsmsim.h"
#include <SoftwareSerial.h>

/////////////////////////////////////////
// GSM/FTP DEFINITION
/////////////////////////////////////////
//extern SoftwareSerial GsmSerial(GSM_RX, GSM_TX); // RX, TX GSM

long int GSMErrors = 0;
#define GSM_RX    8      // CEPRI PIN 9   : NEOWAY PIN 14
#define GSM_TX    9      // CEPRI PIN 10  : NEOWAY PIN 16
SoftwareSerial GsmSerial(GSM_RX,GSM_TX);
static char *ExtBuffer;
static int BootPin=0;

/////////////////////////////////////////
// SETUP GSM
/////////////////////////////////////////

GSMSIM::GSMSIM(int BOOT_PIN, char*buffer, int ExtBufferSize)
{
	ExtBuffer = buffer;
	if (ExtBufferSize < EXTBUFFERSIZE) Serial.println(F("Error: Ext Buffer size too small"));
	//***************************************
	//SETUP GSM
	//***************************************

	GsmSerial.begin(19200);
	//  GsmSerial.listen();
	//  GsmSerial.flush();
	if (BOOT_PIN >= 0 )
	{
		digitalWrite(BOOT_PIN, HIGH);
		pinMode(BOOT_PIN, OUTPUT);
	}
	BootPin = BOOT_PIN;
}

//////////////////////////////////////////////////////
// CONFIGURE GSM FOR IP AFTER BOOT
////////////////////////////////////////////////////////
int GSMSIM::ConfGSM()
{
	int retryCmd;
	int ret;

	Serial.println(F(" - GSM Conf: "));
	digitalWrite(LEDPIN, HIGH);   // turn the LED on

	GsmSerial.listen();

	// Check network registration status
	retryCmd = 30;
	do {
		delay(2000);
		if ( GSM_AT(F("AT+CREG?")) != GSMOK) return GSMERROR ;
	}   while ((strstr(ExtBuffer, ",1") <= 0 ) && --retryCmd); // not registered on network
	if (retryCmd <= 0) {
		return GSMERROR ;
	} ;

	if ( GSM_AT(F("AT+CGDCONT=1,\"IP\",\"ibox.tim.it\"")) != GSMOK) return GSMERROR; // set GPRS PDP format
	if ( GSM_AT(F("AT+XGAUTH=1,1,\"\",\"\"")) != GSMOK) return GSMERROR; //PDP authentication
	if ( GSM_AT(F("AT+XIIC=1"))               != GSMOK) return GSMERROR; //establish PPP link

	// Check the status of PPP link.
	retryCmd = 10;
	do {
		delay(1000);
		GSM_AT(F("AT+XIIC?"));
	} while ((strstr(ExtBuffer, "1,") <= 0) && --retryCmd);
	if (!retryCmd) {
		return GSMERROR ;
	} ;

	// check the receiving signal intensity only
	retryCmd = 20;
	do {
		GSM_AT(F("AT+CSQ"));
	} while ((strstr(ExtBuffer, "9,9") > 0) && --retryCmd);
	if (!retryCmd) {
		return GSMERROR ;
	} ;

	digitalWrite(LEDPIN, LOW);

	// retryCmd = 2;
	// do {
	// GsmSerial.println(F("AT+DNS=\"ftp.cabasino.com\""));
	// if (GSMOK != GSM_Response(3)) { --retryCmg; }
	// if (!retryCmd) { return GSMERROR ; } ;

	Serial.println(F("- DONE"));
	return GSMOK;

}

////////////////////////////////////////////////////

int  GSMSIM::ReadSMS()
{
	
	int ret;
	char *p, *p1;
	Serial.println(F(" - Read SMS "));
	GsmSerial.listen();
	GSM_AT(F("AT+CSQ"));
	if ( GSM_AT(F("AT+CMGL=4")) != GSMOK) return GSMERROR;
	return GSMOK;
}
////////////////////////////////////////////////////

int  GSMSIM::DeleteAllSMS()
{
	GsmSerial.listen();
	if ( GSM_AT(F("AT+CMGD=0,4")) != GSMOK) return GSMERROR;
	return GSMOK;
}

////////////////////////////////////////////////////

int  GSMSIM::LoginFTP()
{
	int retry;

	GsmSerial.listen();


	Serial.println(F(" - LoginFTP: "));
	GsmSerial.println(F("AT"));

	long int start = millis();
	while (millis() < start + 1000) if (GsmSerial.available())Serial.print((char) GsmSerial.read());

	retry = 2;
	do {

		GsmSerial.println(F("At+ftplogin=217.64.195.210,21,cabasino.com,Catto1"));
	} while ((GSMOK != GSM_Response(2)) && (--retry)) ;
	if (!retry) return GSMERROR;
	Serial.println(F(" - DONE"));

	return GSMOK;
}

////////////////////////////////////////////////////

int  GSMSIM::StatusFTP()
{
	int ret;

	GsmSerial.listen();
	Serial.println(F(" - StatusFTP: "));
	GsmSerial.println(F("AT+FTPSTATUS"));
	GSM_Response(2);

	Serial.println(F(" - DONE"));
	ret = (strstr(ExtBuffer, ":login") > 0);
	if (ret) return GSMOK; else return GSMERROR;
}

////////////////////////////////////////////////////

int GSMSIM::PutFTP(const char *file, char *obuf)
{
	int i = 0; int result = -1;
	char putcmd[100];
	GsmSerial.listen();

	if (StatusFTP() != GSMOK) return GSMERROR;

	Serial.println(F(" - PutFTP: "));

	sprintf(putcmd, "AT+FTPPUT=%s,1,1,%d", file, strlen(obuf));

	GsmSerial.println(putcmd);
	{
		long int start = millis(); char a;
		while (millis() < start + 2000)
		{
			if (GsmSerial.available())
			{
				Serial.write(a = GsmSerial.read());
				ExtBuffer[i++] = a;

				if (a == '>') {
					result = 1;
				}
				if (a == '+') {
					result = -1;
				}
			}
		}
	}

	ExtBuffer[i] = 0;
	if (result == -1) {
		Serial.println(F(" - DONE NO PUT"));
		GSMErrors += 1000;
		return GSMERROR;
	}
	GsmSerial.write(obuf);// The  text you want to send
	GsmSerial.write('\n');
	Serial.println(obuf);  Serial.println(strlen(obuf));
	if (GSM_Response(1) != GSMOK) {
		Serial.println(F("NO RESP"));
		GSMErrors += 1000;
		return GSMERROR;
	}
	Serial.println(F("DONE"));
	return GSMOK;
}

////////////////////////////////////////////////////

char *GSMSIM::ReadFTP(char *filename)
{
	int i = 0;
	char putcmd[100];

	GsmSerial.listen();

	if (!StatusFTP()) return "Error";

	Serial.println(F(" - GetFTP: "));
	sprintf(putcmd, "AT + FTPGET = % s, 1, 1", filename);
	GsmSerial.println(putcmd);
	{
		long int start = millis(); char a;
		while (millis() < start + 15000)  //wait 15 sec
		{
			if (GsmSerial.available())
			{
				Serial.write(a = GsmSerial.read());
				ExtBuffer[i++] = a;
				if (i > 10) {
					if (!strcmp(ExtBuffer - 3, ": OK")) start = millis() - 10000; //wait 15-10=5 sec
				}
				if (i > EXTBUFFERSIZE) {
					Serial.println(F("Buffer Overflow"));
					i--;
				}

			}
		}
	}
	ExtBuffer[i] = 0;

	if (strstr(ExtBuffer, ": ") <= 0) {
		strcpy(ExtBuffer, "Error");
	}
	Serial.println(F(" - DONE READ"));

	return ExtBuffer;
}

////////////////////////////////////////////////////

int GSMSIM::GSM_AT(const __FlashStringHelper * ATCommand)
{
	int i = 0;
	int done = GSMUNKNOWN;
	long int start = millis();

	ExtBuffer[0] = 0;
	//Serial.println(ATCommand);
	GsmSerial.println(ATCommand);
	while ((millis() < (start + 3000)) && !done )
	{
		if (GsmSerial.available()) ExtBuffer[i++] = GsmSerial.read();
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
	Serial.println(ExtBuffer);

	start = millis();
	while (millis() < start + 50) if (GsmSerial.available())  Serial.write(GsmSerial.read());
	return done;
}

////////////////////////////////////////////////////

int GSMSIM::GSM_Response(int n)  {
	long int start = millis(); long int timeout = 20000;
	char a = 0; int pcnt = 0; int i = 0;

	Serial.print (n); Serial.println(F(" RESP : "));
	if (GsmSerial.overflow())      Serial.println("OVERFLOWWWWWW\n");
	while (millis() < start + timeout)
	{
		if (GsmSerial.available())
		{
			a = GsmSerial.read();
			Serial.write(a);

			if (a == '+') {
				pcnt++;
				if (pcnt == n)  {
					start = millis();
					timeout = 500;
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
	Serial.println(F("\nEND RESP"));

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
	GsmSerial.listen();

	//GSM_AT(F("AT+CMGD=4[,<delflag>]

	sprintf(ExtBuffer, "- SEND SMS \"%s\" TO %s:", message, number);
	Serial.println(ExtBuffer);
	GsmSerial.write("AT+CMGS=");
	delay(100);
	sprintf (ExtBuffer, "\"%s\"\r", number); // quoted number
	GsmSerial.println(ExtBuffer);
	delay(100);
	//mySerial.println("\"+393356930892\"\r"); // Replace x with mobile number
	GsmSerial.write(message);// The SMS text you want to send
	delay(100);
	GsmSerial.write((char)26);// ASCII code of CTRL+Z
	GSM_Response(2);

}

///////////////////////////////////////////////////

void GSMSIM::PowerOffGSM()
{
	long int start;
	Serial.println(F("PowerOffGSM"));
	GsmSerial.listen();
	//  digitalWrite(GSM_BOOT_PIN, LOW);
	//  delay(700);
	//  digitalWrite(GSM_BOOT_PIN, HIGH);

	while ( GSM_AT(F("AT+CPWROFF")) == GSMOK)   delay(2000);   GSMErrors--;
	return;
}

////////////////////////////////////////////////////

int GSMSIM::BootGSM()
{
	long int start;
	int retry = 3;

	GsmSerial.listen();
	start = millis();

	while (GSM_AT(F("AT")) != GSMOK && --retry)
	{
		int i=0;
		Serial.println(F("BootGSM"));
		if (BootPin <= 0) return GSMERROR;
		GSMErrors--;
		digitalWrite(BootPin, LOW);
		delay(700);
		digitalWrite(BootPin, HIGH);
		while (((millis() < (start + 8000))&& !strstr(ExtBuffer, "STARTUP")) )
		{
			if (GsmSerial.available()){
				ExtBuffer[i]=GsmSerial.read();
				Serial.write(ExtBuffer[i]); 
				i++;
			}
		}

		start = millis() + 4000; // wait 12 seconds
	}
    ExtBuffer[0]=0;
	delay(1500); // wait sim busy
	if (retry)  // boot OK
	{
		GSM_AT(F("ATE1"));
		GSM_AT(F("AT+CMEE=2")); // FULL DIAG
		GSM_AT(F("AT+CMGF=1")); //  FOR SMS
		// GSM_AT(F("AT+COPS?")); 0 is automatic registration, 2 is deregister
		// GSM_AT(F("AT+COPS=0")); ONLY IF SIM PROBLEM
		//  if ( GSM_AT(F("AT+CREG=1"))       != GSMOK) return GSMERROR; //allow the network registration to provide result code
		//if ( GSM_AT(F("ATE0")) != GSMOK) return GSMERROR; //set no echo
		if ( GSM_AT(F("AT+CSCS=\"GSM\"")) != GSMOK) return GSMERROR; //set character set
		if ( GSM_AT(F("AT+XISP=0"))       != GSMOK) return GSMERROR; //Select internal protocol stack
		return GSMOK ;
	}
	else return GSMERROR;

}

void GSMSIM::ProxyGSM()
{ 
	GsmSerial.listen();
	Serial.println("<<");
	long int start = millis();

	delay(100);
	while (Serial.available())GsmSerial.write(Serial.read());
	while (millis() < start + 5000)
	while (GsmSerial.available()) {
		Serial.write(GsmSerial.read());
	}
	if (GsmSerial.overflow()) {
		Serial.println("SoftwareSerial overflow!");
	}
	Serial.println(">>");
}
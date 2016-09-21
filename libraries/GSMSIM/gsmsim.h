#ifndef GSMSIM_h
#define GSMSIM_h
#include <SoftwareSerial.h>
/////////////////////////////////////////
// GSM/FTP DEFINITION
/////////////////////////////////////////
//extern SoftwareSerial GsmSerial(GSM_RX, GSM_TX); // RX, TX GSM
#include "Arduino.h"
extern long int GSMErrors;

#define GSMIGNOREERROR 1
#define GSMERROR       2
#define GSMOK          1
#define GSMUNKNOWN     0


#define LEDPIN        13  // default arduino LED

/////////////////////////////////////////
// SETUP GSM
/////////////////////////////////////////
class GSMSIM
{
public:
GSMSIM(int BOOT, char *TmpBuffer);
void SetupGSM();
int  ConfGSM();
int  ReadSMS();
int  DeleteAllSMS();
int  LoginFTP();
int  StatusFTP();
int  PutFTP(const char *file, char *obuf);
char *ReadFTP(char *filename);
void SendSMS(char *number, char* message);
void PowerOffGSM();
int  BootGSM();
void ProxyGSM();

private:

int GSM_AT(const __FlashStringHelper * ATCommand);
int GSM_Response(int n) ;
};

#endif
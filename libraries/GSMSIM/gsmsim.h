#ifndef GSMSIM_h
#define GSMSIM_h

/////////////////////////////////////////
// GSM/FTP DEFINITION
/////////////////////////////////////////
//extern SoftwareSerial GsmSerial(GSM_RX, GSM_TX); // RX, TX GSM
#include "Arduino.h"
extern long int GSMErrors;

#define EXTBUFFERSIZE  200
#define GSMIGNOREERROR 1
#define GSMERROR       2
#define GSMFATAL       3
#define GSMOK          1
#define GSMUNKNOWN     0



#define LEDPIN        13  // default arduino LED

/////////////////////////////////////////
// SETUP GSM
/////////////////////////////////////////
class GSMSIM
{


Stream& m_gsmSerial;
public:
GSMSIM(int BOOT, char *TmpBuffer, int TmpBuffersize, Stream& m_gsmSerial);
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
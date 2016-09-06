//////////////////////////////////////////////////////
// CONFIGURE GSM AFTER BOOT
////////////////////////////////////////////////////////
int ConfGSM()
{
  int retryCmd;
  int ret;

  Serial.println(F(" - GSM Conf: "));
  digitalWrite(LEDPIN, HIGH);   // turn the LED on

  GsmSerial.listen();
  GpsSerial.flush();
  colorWipe(LedStrip.Color(100, 100, 100), 50);   // white
  GSM_AT(F("ATE1"));
  GSM_AT(F("AT+CMEE=2"));
  GSM_AT(F("AT+CMGF=1")); // ONLY FOR SMS
  // GSM_AT(F("AT+COPS=0")); ONLY IF SIM PROBLEM

  //  if ( GSM_AT(F("AT + CREG = 1"))       != GSMOK) return GSMERROR; //allow the network registration to provide result code
  //if ( GSM_AT(F("ATE0")) != GSMOK) return GSMERROR; //set no echo
  if ( GSM_AT(F("AT+CSCS=\"GSM\"")) != GSMOK) return GSMERROR; //set character set
  if ( GSM_AT(F("AT+XISP=0"))       != GSMOK) return GSMERROR; //Select internal protocol stack


  // Check network registration status
  retryCmd = 30;
  do {
    delay(2000);
    if ( GSM_AT(F("AT+CREG?")) != GSMOK) if ( GSM_AT(F("AT+CREG?")) != GSMOK) return GSMERROR ; // retry if timeout
    colorWipe(LedStrip.Color(255, 2550, 0), 50); // yellow
  }   while ((strstr(TmpBuffer, ",1") <= 0 ) && --retryCmd); // not registered on network
  if (retryCmd <= 0) {
    return GSMERROR ;
  } ;


  if ( GSM_AT(F("AT+CGDCONT=1,\"IP\",\"ibox.tim.it\"")) != GSMOK) return ret; // set GPRS PDP format
  if ( GSM_AT(F("AT+XGAUTH=1,1,\"\",\"\"")) != GSMOK) return GSMERROR; //PDP authentication
  if ( GSM_AT(F("AT+XIIC=1"))               != GSMOK) return GSMERROR; //establish PPP link

  // Check the status of PPP link.
  retryCmd = 10;
  do {
    delay(1000);
    GSM_AT(F("AT+XIIC?"));
  } while ((strstr(TmpBuffer, "1,") <= 0) && --retryCmd);
  if (!retryCmd) {
    return GSMERROR ;
  } ;


  // check the receiving signal intensity only
  retryCmd = 20;
  do {
    GSM_AT(F("AT+CSQ"));
  } while ((strstr(TmpBuffer, "9,9") > 0) && --retryCmd);
  if (!retryCmd) {
    return GSMERROR ;
  } ;


  digitalWrite(LEDPIN, LOW);


  // retryCmd = 2;
  // do {
  // GsmSerial.println(F("AT+DNS=\"ftp.cabasino.com\""));
  // if (GSMOK != GSMResponse(3)) { --retryCmg; }
  // if (!retryCmd) { return GSMERROR ; } ;


  Serial.println(F("- DONE"));
  return GSMOK;

}





////////////////////////////////////////////////////

int  LoginFTP()
{
  int retry;

  GsmSerial.listen();
  delay(2000);

  Serial.println(F(" - LoginFTP: "));
  GsmSerial.println(F("AT"));

  long int start = millis();
  while (millis() < start + 1000) if (GsmSerial.available())Serial.print((char) GsmSerial.read());

  retry = 2;
  do {

    GsmSerial.println(F("At+ftplogin=217.64.195.210,21,cabasino.com,Catto1"));
  } while ((GSMOK != GSMResponse(2)) && (--retry)) ;
  if (!retry) return GSMERROR;
  Serial.println(F(" - DONE"));

  return GSMOK;
}


////////////////////////////////////////////////////

int  StatusFTP()
{
  int ret;


  GsmSerial.listen();
  Serial.println(F(" - StatusFTP: "));
  GsmSerial.println(F("AT+FTPSTATUS"));
  GSMResponse(2);

  Serial.println(F(" - DONE"));
  ret = (strstr(TmpBuffer, ":login") > 0);
  if (ret) return GSMOK; else return GSMERROR;
}



////////////////////////////////////////////////////

int PutFTP(const char *file, char *obuf)
{
  int i = 0; int result = -1;
  char putcmd[100];

  if (StatusFTP() != GSMOK) return GSMERROR;
  GsmSerial.listen();

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
        TmpBuffer[i++] = a;

        if (a == '>') {
          result = 1;
        }
        if (a == '+') {
          result = -1;
        }
      }
    }
  }

  TmpBuffer[i] = 0;
  if (result == -1) {
    Serial.println(F(" - DONE NO PUT"));
    GErrors += 1000;
    return GSMERROR;
  }
  GsmSerial.write(obuf);// The  text you want to send
  GsmSerial.write('\n');
  Serial.println(obuf);  Serial.println(strlen(obuf));
  if (GSMResponse(1) != GSMOK) {
    Serial.println(F("NO RESP"));
    GErrors += 1000;
    return GSMERROR;
  }
  Serial.println(F("DONE"));
  return GSMOK;
}

////////////////////////////////////////////////////

char *ReadFTP(char *filename)
{
  int i = 0;
  char putcmd[100];

  if (!StatusFTP()) return "Error";
  GsmSerial.listen();

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
        TmpBuffer[i++] = a;
        if (i > 10) {
          if (!strcmp(TmpBuffer - 3, ": OK")) start = millis() - 10000; //wait 15-10=5 sec
        }
        if (i > sizeof (TmpBuffer)) {
          Serial.println(F("Buffer Overflow"));
          i--;
        }

      }
    }
  }
  TmpBuffer[i] = 0;

  if (strstr(TmpBuffer, ": ") <= 0) {
    strcpy(TmpBuffer, "Error");
  }
  Serial.println(F(" - DONE READ"));

  return TmpBuffer;
}






////////////////////////////////////////////////////

int GSM_AT(const __FlashStringHelper * ATCommand)
{
  int i = 0;
  int done = GSMUNKNOWN;
  long int start = millis();
  colorWipe(LedStrip.Color(50, 50, 50), 50);
  TmpBuffer[0] = 0;
  //Serial.println(ATCommand);
  GsmSerial.println(ATCommand);
  while ((millis() < (start + 2000)) && !done )
  {
    if (GsmSerial.available()) TmpBuffer[i++] = GsmSerial.read();
    if (i > 2) if (!strncmp(TmpBuffer + i - 3, "OK", 2)) done = GSMOK;
    if (i > 5) if (!strncmp(TmpBuffer + i - 6, "ERROR", 5)) done = GSMERROR;
    if (i > sizeof(TmpBuffer)) {
      Serial.println(F("BUFFER FULL\n"));
      done = GSMERROR;
      GErrors++;
    }
  };
  TmpBuffer[i] = 0;
  if (done == GSMERROR || done == GSMUNKNOWN  )
  {
    Serial.println(F(" - GSM: "));
    if (done == GSMUNKNOWN)  Serial.println(F(" TIMEOUT, BOOT ? "));
    GErrors++;
  }
  Serial.println(TmpBuffer);

  start = millis();
  while (millis() < start + 50)
    if (GsmSerial.available())
      Serial.write(GsmSerial.read());
  return done;
}


////////////////////////////////////////////////////

int GSMResponse(int n)  {
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
      TmpBuffer[i] = a;
      if (i < 199) i++;
    }
  }

  if (i > sizeof(TmpBuffer)) {
    Serial.println(F("BUFFER FULL\n"));
    GErrors++;
    return GSMERROR;
  }
  TmpBuffer[i] = 0;
  Serial.println(F("\nEND RESP"));


  if (pcnt < n   ||  (strstr(TmpBuffer, "Error") > 0)) {
    Serial.println(F("\nTimeout / Error response"));
    colorWipe(LedStrip.Color(255, 0, 0), 50); // Red
    GErrors++;
    return GSMERROR;
  } else {
    return GSMOK;
  }
}




////////////////////////////////////////////////////
void SendSMS(char *number, char* message)
{

  long int start;
  GsmSerial.listen();

  //GSM_AT(F("AT+CMGD=4[,<delflag>]

  sprintf(TmpBuffer, "- SEND SMS \"%s\" TO %s:", message, number);
  Serial.println(TmpBuffer);
  GsmSerial.write("AT+CMGS=");
  delay(100);
  sprintf (TmpBuffer, "\"%s\"\r", number); // quoted number
  GsmSerial.println(TmpBuffer);
  delay(100);
  //mySerial.println("\"+393356930892\"\r"); // Replace x with mobile number
  GsmSerial.write(message);// The SMS text you want to send
  delay(100);
  GsmSerial.write((char)26);// ASCII code of CTRL+Z
  start = millis();
  while ((millis() < (start + 7000)))
    if (GsmSerial.available()) Serial.write(GsmSerial.read());

}



///////////////////////////////////////////////////

void PowerOffGSM()
{
  long int start;
  Serial.println(F("PowerOffGSM"));
  GsmSerial.listen();
  //  digitalWrite(GSM_BOOT_PIN, LOW);
  //  delay(700);
  //  digitalWrite(GSM_BOOT_PIN, HIGH);

  while ( GSM_AT(F("AT + CPWROFF")) == GSMOK)   delay(2000);   GErrors--;
  return;
}


////////////////////////////////////////////////////

int BootGSM()
{
  long int start;
  int retry = 3;

  GsmSerial.listen();
  start = millis();
  while (GSM_AT(F("AT")) != GSMOK && --retry)
  {
    if (GSM_BOOT_PIN < 0) return GSMERROR;
    Serial.println(F("BootGSM"));
    GErrors--;
    digitalWrite(GSM_BOOT_PIN, LOW);
    delay(700);
    digitalWrite(GSM_BOOT_PIN, HIGH);
    while ((millis() < (start + 8000)))
      if (GsmSerial.available()) Serial.write(GsmSerial.read());
    start = millis() + 8000; // wait 16 seconds
  }

  //  GsmSerial.println("AT+IPR=4800");
  //  GsmSerial.begin(4800);
  //  delay(500);
  //  GsmSerial.flush();

  if (retry) return GSMOK ; else return GSMERROR;
}


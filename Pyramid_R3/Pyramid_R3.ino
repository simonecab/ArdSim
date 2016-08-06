
#include <EEPROM.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>

//#include <Vcc.h>
//#ifdef __AVR__
//#include <avr/power.h>
//#endif



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
#define GSM_RX    8      // CEPRI PIN 9   : NEOWAY PIN 14
#define GSM_TX    9      // CEPRI PIN 10  : NEOWAY PIN 16
#define GSM_BOOT_PIN  3       // Neoway M590 boot pin if available : CEPRI PIN 14 : NEOWAY PIN 19
// #define GSM_BOOT_PIN  -1   // if Neoway M590 boot pin NOT available

#define LEDSTRIPPIN   5   // RGB LED Strip

#define LEDPIN        13  // default arduino LED

#define VOLTINPIN     1   // Analog input connected to intermediate battery (less than 5v)


/////////////////////////////////////////
// AUDIO DEFINITIONS
/////////////////////////////////////////
SoftwareSerial AudioSerial(AUDIO_RX, AUDIO_TX);

#define CMD_PLAY_W_INDEX 0X03
#define CMD_SET_VOLUME 0X06
#define CMD_SEL_DEV 0X09
#define DEV_TF 0X02
#define CMD_PLAY 0X0D
#define CMD_PAUSE 0X0E
#define CMD_SINGLE_CYCLE 0X19
#define SINGLE_CYCLE_ON 0X00
#define SINGLE_CYCLE_OFF 0X01
#define CMD_PLAY_W_VOL 0X22

/////////////////////////////////////////
// SENSORS INCLUDE AND DEFINITIONS
/////////////////////////////////////////

int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;  // Temp is ignored

// REQUIRED IF MPU6050 ACCEL/GYRO IS USED
const int MPU_addr = 0x68; // I2C address of the MPU-6050


// REQUIRED ONLY IF ADXL345 ACCEL IS USED
#ifdef ADXL345
#include <ADXL345.h>
ADXL345 adxl; //variable adxl is an instance of the ADXL345 library
#endif

// REQUIRED ONLY IF HMC5883 COMPASS IS USED
const int HMC5883_addr = 0x1E; //0011110b, I2C 7bit address of HMC5883



/////////////////////////////////////////
// RGB DEFINITIONS
/////////////////////////////////////////
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel LedStrip = Adafruit_NeoPixel(4, LEDSTRIPPIN, NEO_GRB + NEO_KHZ800);


/////////////////////////////////////////
// GPS DEFINITIONS
/////////////////////////////////////////
#include <TinyGPS.h>
SoftwareSerial GpsSerial(GPS_RX, GPS_TX); // RX, TX GPS
TinyGPS Gps;


/////////////////////////////////////////
// GSM/FTP DEFINITION
/////////////////////////////////////////
SoftwareSerial GsmSerial(GSM_RX, GSM_TX); // RX, TX GSM
//AltSoftSerial GsmSerial;

long int GErrors = 0;

#define GSMIGNOREERROR 1
#define GSMERROR       2
#define GSMOK          1
#define GSMUNKNOWN     0




/////////////////////////////////////////
// APP DEFINITION
/////////////////////////////////////////
#define UPDATETIMEFINAL 600     // Final update interval 
#define UPDATETIMEINITIAL 60;   // initial update interval
int MFile = 1; // id file multipli
long int NextConnectionTime = 15;

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
  //SETUP AUDIO
  //***************************************
  pinMode(AUDIO_TX, OUTPUT);
  pinMode(AUDIO_RX, INPUT);
  AudioSerial.begin(9600);
  ConfAudio();
  AudioPlay(1, 0x5); // file 1 level 15

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
  while (millis() < 10000 && i < 190)
    if (GpsSerial.available())
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
  //SETUP RGB LED STRIP
  //***************************************
  LedStrip.begin();
  LedStrip.show(); // Initialize all pixels to 'off'


  //***************************************
  //SETUP ON BOARD LED
  //***************************************

  pinMode(LEDPIN, OUTPUT);
  //blink(10, BLINK_FAST);   //blink(10, BLINK_NORM); //SECONDI DI BLINK  e VELOCITA'


  //***************************************
  //SETUP GSM
  //***************************************
  pinMode(GSM_TX, OUTPUT);
  pinMode(GSM_RX, INPUT);
  GsmSerial.begin(19200);
  //  GsmSerial.listen();
  //  GsmSerial.flush();
  if (GSM_BOOT_PIN >= 0 )
  {
    digitalWrite(GSM_BOOT_PIN, HIGH);
    pinMode(GSM_BOOT_PIN, OUTPUT);
  }

  //***************************************
  //SETUP I2C SENSORS
  //***************************************
  SetupAccelMPU();  // MPU 6050 accelerometer&gyro if present
#ifdef ADXL345
  adxl.powerOn();   // adxl345 accelerometer if present
#endif
  //Put the HMC5883 compass into the correct operating mode
  {
    Wire.beginTransmission(HMC5883_addr); //open communication with HMC5883
    Wire.write((uint8_t)0x02); //select mode register
    Wire.write((uint8_t)0x00); //continuous measurement mode
    Wire.endTransmission();
  }



  //***************************************
  //END SETUP
  //***************************************

  Serial.print(F("\nAT cmd, h(alt), a(udio), p(ut), l(ogin), g(ps)\nr(ead), b(oot), c(onf), s(tatus), t(est), S(ms)\n"));
  Serial.println(F("cmd# "));
  GpsSerial.listen();

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
    //Serial.print(c);
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
      case 'p': PutFTPGps();  break;
      case 'l': LoginFTP();  break;
      case 'c': ConfGSM();  break;
      case 's': StatusFTP();  break;
      case 'a': Serial.println(F("play file 2")); AudioPlay(2, 0x8); break;
      case 'b': BootGSM();  break;
      case 'g': GetGps();  break;
      case 'h': Serial.println(F("send ogni ora")); NextConnectionTime = 3600000; break;
      case 'r': Serial.println(ReadFTP("command.txt")); break;
      case 't': TestSensors(); break;
      case 'S': SendSMS("3296315064", "ciao bongo");  break;

      default:
        { Serial.println("<<");
          long int start = millis();
          GsmSerial.write(a);
          delay(100);
          while (Serial.available())GsmSerial.write(Serial.read());
          while (millis() < start + 5000)
            if (GsmSerial.available()) {
              delay(2);
              Serial.write(a = GsmSerial.read());
            }
          Serial.println(">>");
          GpsSerial.listen();
        }
    }
    {
      long int start = millis();
      while (millis() < start + 200)
        if (Serial.available()) Serial.read();
    }

    Serial.println(F("cmd# "));
    GpsSerial.listen();
  }

  //////////////////////////////////////////////////////
  // FTP PERIODIC PROCESSING
  //////////////////////////////////////////////////////

  if ((millis() / 1000) > NextConnectionTime) {
    long int now = millis() / 1000;

    if (NextConnectionTime < UPDATETIMEFINAL)
    {
      NextConnectionTime = now + UPDATETIMEINITIAL ;
    }
    else
    {
      NextConnectionTime = now + UPDATETIMEFINAL;
    }

    blink(2, BLINK_FAST);


    Happy = HUNKNOWN;
    colorWipe(LedStrip.Color(255, 255, 255), 50); // white
    blink(2, BLINK_FAST);
    if ( BootGSM() != GSMOK) {
      Happy = HERROR;
      return;
    }
    if ( ConfGSM() != GSMOK) {
      Happy = HERROR;
      PowerOffGSM();
      NextConnectionTime = (millis() / 1000) + UPDATETIMEINITIAL;
      return;
    }
    colorWipe(LedStrip.Color(255, 0, 255), 50); // Blue
    if ( LoginFTP() != GSMOK) {
      Happy = HERROR;
      PowerOffGSM();
      NextConnectionTime = (millis() / 1000) + UPDATETIMEINITIAL;
      return;
    }
    blink(2, BLINK_FAST);
    colorWipe(LedStrip.Color(0, 0, 255), 50); // Blue
    if ( PutFTPGps() != GSMOK) {
      Happy = HERROR;
      PowerOffGSM();
      NextConnectionTime = (millis() / 1000) + UPDATETIMEINITIAL;
      return;
    }
    AudioPlay(2, 0x4);
    Happy = HPERFECT;
    PowerOffGSM();
    Serial.print("Now: "); Serial.print(millis() / 1000); Serial.print(" next "); Serial.println(NextConnectionTime);

    Serial.println(F("cmd# "));
    GpsSerial.listen();
  }



  //////////////////////////////////////////////////////
  // RGB LED STRIP PERIODIC PROCESSING
  //////////////////////////////////////////////////////
  if ((!((millis() / 1000) % 3)))
  {
    switch (Happy)
    {
      case HERROR:   colorWipe(LedStrip.Color(255, 0,   0),   50); blink(1, BLINK_SLOW); break; // red
      case HUNKNOWN: colorWipe(LedStrip.Color(100, 100, 100), 50); break;// white
      case HPERFECT: colorWipe(LedStrip.Color(0,   255, 0),   50); blink(1, BLINK_FAST); break; // green
      case HOK:      colorWipe(LedStrip.Color(0,   0,   255), 50); blink(1, BLINK_FAST); break; // blue
      default:       colorWipe(LedStrip.Color(255, 255, 0),   50); break;// yellow
    }
    ReadAccelMPU();

    colorWipe(LedStrip.Color(AcX > 4000 ? 255 : 0, AcY > 4000 ? 255 : 0, AcZ > 4000 ? 255 : 0), 50);
    GpsSerial.listen();
  }

}//end loop


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

//////////////////////////////////////////////////////
// GET GPS LAT AND LON
//////////////////////////////////////////////////////
long lat, lon;
unsigned long fix_age, time, date, speed, course;

unsigned long chars;
unsigned short sentences, failed_checksum;

void GetGps()
{
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  Gps.stats(&chars, &sentences, &failed_checksum);
  Serial.print(F("rcv:fail = "));
  Serial.print(sentences);
  Serial.print(":");
  Serial.println(failed_checksum);
  // retrieves +/- lat/long in 100000ths of a degree
  Gps.get_position(&lat, &lon, &fix_age);

  Serial.println(lat);
  Serial.println(lon);

  // time in hhmmsscc, date in ddmmyy
  Gps.get_datetime(&date, &time, &fix_age);
  if (fix_age == 4294967295 )
  {
    colorWipe(LedStrip.Color(255, 255, 0), 100); // yellow
    Serial.println(F("NO Gps FIX"));
  }
  Serial.println(fix_age);
  Serial.println(time);
  Serial.println(F(" - DONE"));

}


//////////////////////////////////////////////////////
// PUT VIA FTP GPS INFO
//////////////////////////////////////////////////////
int PutFTPGps()
{

  char text[100];
  char file[40];
  int i;

  GetGps();

  int Volt = analogRead(VOLTINPIN);

  sprintf(text, "< S = %3d %9ld %9ld %9ld V3=%04d E=%ld >", Gps.satellites(), lat, lon, time + 2000000, Volt, GErrors  );
  sprintf(file, "test%04d.txt", MFile++);

  return PutFTP( file, text);
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


//////////////////////////////
// RGB LED STRIP
/////////////////////////////

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < LedStrip.numPixels(); i++) {
    LedStrip.setPixelColor(i, c);
    LedStrip.show();
    delay(wait);
  }
  for (uint16_t i = 0; i < LedStrip.numPixels(); i++) {
    LedStrip.setPixelColor(i, 0);
    LedStrip.show();
    delay(wait);
  }
}



//////////////////////////////
// SERIAL MP3
/////////////////////////////

void AudioPlay(unsigned int file, unsigned int vol)
{

  sendCommand(CMD_PLAY_W_VOL, (vol << 8) + file);

}

void ConfAudio()
{
  delay(500);//Wait chip initialization is complete
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card
  delay(200);//wait for 200ms
}


void sendCommand(int8_t command, int16_t dat)
{

  delay(20);
  TmpBuffer[0] = 0x7e; //starting byte
  TmpBuffer[1] = 0xff; //version
  TmpBuffer[2] = 0x06; //the number of bytes of the command without starting byte and ending byte
  TmpBuffer[3] = command; //
  TmpBuffer[4] = 0x00;//0x00 = no feedback, 0x01 = feedback
  TmpBuffer[5] = (int8_t)(dat >> 8);//datah
  TmpBuffer[6] = (int8_t)(dat); //datal
  TmpBuffer[7] = 0xef; //ending byte
  for (uint8_t i = 0; i < 8; i++) //
  {
    AudioSerial.write(TmpBuffer[i]) ;
  }
}



//////////////////////////////
// SENSORS
/////////////////////////////
void SetupAccelMPU()
{
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void ReadAccelMPU()
{
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  int Tmp = Wire.read() << 8 | Wire.read(); //  IGNORED 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

void TestSensors()
{
#ifndef TESTSENSORS
  char a;
  Serial.println("Press x to stop");
  while (Serial.read() != 'x') {

    // MPU 6050
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    int Tmp = Wire.read() << 8 | Wire.read(); //  IGNORED 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    Serial.print("MPU6050 Accel X = "); Serial.print(AcX);
    Serial.print(" Y = "); Serial.print(AcY) ;
    Serial.print(" Z = "); Serial.print(AcZ);
    Serial.print("  Gyro X = "); Serial.print(GyX);
    Serial.print(" Y = "); Serial.print(GyY) ;
    Serial.print(" Z = "); Serial.println(GyZ);


#ifdef ADXL345
    //ADXL345
    int x, y, z;
    adxl.readAccel(&AcX, &AcY, &AcZ); //read the accelerometer values and store them in variables  x,y,z
    Serial.print("ADXL345 Accel X = "); Serial.print(AcX);
    Serial.print(" Y = "); Serial.print(AcY) ;
    Serial.print(" Z = "); Serial.println(AcZ);
#endif



    {
      int Mx, My, Mz; //triple axis magnetic data

      //Tell the HMC5883L where to begin reading data
      Wire.beginTransmission(HMC5883_addr);
      Wire.write((uint8_t)0x03); //select register 3, X MSB register
      Wire.endTransmission();

      //Read data from each axis, 2 registers per axis
      Wire.requestFrom(HMC5883_addr, 6);
      if (6 <= Wire.available()) {
        Mx = Wire.read() << 8  | Wire.read(); //X msb ,  lsb
        Mz = Wire.read() << 8  | Wire.read(); //Z msb ,  lsb
        My = Wire.read() << 8  | Wire.read(); //Y msb ,  lsb
      }

      //Print out values of each axis
      Serial.print("HMC5883 COMPASS x : ");
      Serial.print(Mx);
      Serial.print("  y : ");
      Serial.print(My);
      Serial.print("  z : ");
      Serial.println(Mz);


      int Volt = analogRead(VOLTINPIN);
      Serial.print("  VOLTPIN ");       Serial.println(Volt);
    }
    delay(800);
  }
#endif
}

//////////////////////////////
// ON BOARD LED
/////////////////////////////
void blink(int tempo, int velocita)
{
  long int start = millis();
  while (millis() < (start + 1000 * (long int)tempo))
  {
    digitalWrite(LEDPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay( velocita);            // wait for a time
    digitalWrite(LEDPIN, LOW);    // turn the LED off by making the voltage LOW
    delay( velocita);
  }
}


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



//////////////////////////////
// SYSTEM UTILITIES
/////////////////////////////
#ifdef UTILITIES
void EraseEEprom()
{
  int Eptr;
  Serial.println(F("Erase EEprom"));
  for (Eptr = 0; Eptr < 510; Eptr++)EEPROM[Eptr] = 0;
}


int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

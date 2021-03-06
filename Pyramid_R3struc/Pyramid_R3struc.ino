#include <gsmsim.h>


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
// AUDIO DEFINITIONS
/////////////////////////////////////////
SoftwareSerial AudioSerial(AUDIO_RX, AUDIO_TX);

#define CMD_PLAY_W_INDEX 0X03
#define CMD_SET_VOLUME 0X06
#define CMD_SEL_DEV 0X09
//#define CMD_SLEEP 0X07
#define CMD_SLEEP 0X0A
#define CMD_WAKE 0X0B
#define CMD_RESET 0X0C
#define DEV_TF 0X02
#define CMD_PLAY 0X0D
#define CMD_PAUSE 0X0E
#define CMD_SINGLE_CYCLE 0X19
#define SINGLE_CYCLE_ON 0X00
#define SINGLE_CYCLE_OFF 0X01
#define CMD_PLAY_W_VOL 0X22
#define CMD_DAC 0X1A
#define DAC_STOP 0x1
#define DAC_START 0x0



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
  //SETUP AUDIO
  //***************************************
  pinMode(AUDIO_TX, OUTPUT);
  pinMode(AUDIO_RX, INPUT);
  AudioSerial.begin(9600);
  ConfAudio();
  AudioPlay(1, 0x12); // file 1 , level

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

  printHelp();
  GpsSerial.listen();

}
void printHelp()
{
  Serial.print(F("\n.AT cmd, h(alt), a(udio), p(ut), l(ogin), g(ps_status), G(PS log)\
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
      case 'p': PutFTPGps(GSMOK);  break;
      case 'l': GSMSIM.LoginFTP();  break;
      case 'c': GSMSIM.ConfGSM();  break;
      case 's': GSMSIM.StatusFTP();  break;
      case 'a': Serial.println(F("play file 2")); AudioPlay(2, 18); break;
      case 'b': GSMSIM.BootGSM();  break;
      case 'g': PutFTPGps(GSMUNKNOWN);  break;
      case 'h': Serial.println(F("send ogni ora")); printHelp(); NextConnectionTime = 3600000; option ^= REDUCE_LED; break;
      case 'r': Serial.println(GSMSIM.ReadFTP("command.txt")); break;
      case 'R': Serial.println(GSMSIM.ReadSMS());Serial.println(TmpBuffer); break;
      case 't': TestSensors(); break;
      case 'S': GSMSIM.SendSMS("3296315064", "ciao bongo");  break;
      case 'G': option ^= GPS_LOG; break;
      case '.': GSMSIM.ProxyGSM();  break;

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

  //////////////////////////////////////////////////////
  // FTP PERIODIC PROCESSING
  //////////////////////////////////////////////////////

  if ((millis() / 1000) > NextConnectionTime) {
        GsmSerial.listen();
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
    if ( GSMSIM.BootGSM() != GSMOK) {
      Happy = HERROR;
      return;
    }
    if ( GSMSIM.ConfGSM() != GSMOK) {
      Happy = HERROR;
      GSMSIM.PowerOffGSM();
      NextConnectionTime = (millis() / 1000) + UPDATETIMEINITIAL;
      return;
    }
    colorWipe(LedStrip.Color(255, 0, 255), 50); // Blue
    if ( GSMSIM.LoginFTP() != GSMOK) {
      Happy = HERROR;
      GSMSIM.PowerOffGSM();
      NextConnectionTime = (millis() / 1000) + UPDATETIMEINITIAL;
      return;
    }
    blink(2, BLINK_FAST);
    colorWipe(LedStrip.Color(0, 0, 255), 50); // Blue
    if ( PutFTPGps(1) != GSMOK) {
      Happy = HERROR;
      GSMSIM.PowerOffGSM();
      NextConnectionTime = (millis() / 1000) + UPDATETIMEINITIAL;
      return;
    }
    AudioPlay(2, 8);
    Happy = HPERFECT;
    GSMSIM.PowerOffGSM();
    Serial.print("Now: "); Serial.print(millis() / 1000); Serial.print(" next "); Serial.println(NextConnectionTime);

    Serial.println(F("cmd# "));
    GpsSerial.listen();
  }



  //////////////////////////////////////////////////////
  // MESSAGE PERIODIC PROCESSING
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
    if (option ^ REDUCE_LED)
    {
      ReadAccelMPU();
      colorWipe(LedStrip.Color(AcX > 4000 ? 255 : 0, AcY > 4000 ? 255 : 0, AcZ > 4000 ? 255 : 0), 50);
    }
    GpsSerial.listen();
  }

}//end loop




//////////////////////////////////////////////////////
// PUT VIA FTP GPS INFO
//////////////////////////////////////////////////////
long lat, lon;

unsigned long fix_age, gpsTime, date, speed, course;

unsigned long chars;
unsigned short sentences, failed_checksum;
int PutFTPGps(int transmit)
{

  char text[100];
  char file[40];
  int i;
  int  gpsHdop,gpsSat;
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  int Volt = analogRead(VOLTINPIN);
  
  Gps.stats(&chars, &sentences, &failed_checksum);
  Gps.get_position(&lat, &lon, &fix_age);
  Gps.get_datetime(&date, &gpsTime, &fix_age);
  
  Serial.print(F("Chars: "));  Serial.print(chars);
  Serial.print(F(" Fix: "));   Serial.print(sentences);
  Serial.print(F(" Fail: "));  Serial.println(failed_checksum);
  Serial.print(F("Time: "));   Serial.println(gpsTime);


  if (fix_age == 4294967295 )
  {
    Serial.println(F("NO Gps FIX"));
  } else {
    Serial.println(F("fix lat lon"));
    Serial.println(fix_age);
    Serial.println(lat);
    Serial.println(lon);
  }

  Serial.print(F("sat = "));
  Serial.println(gpsSat = Gps.satellites());
  Serial.print(F("HDOP= "));
  Serial.println(gpsHdop = Gps.hdop());


  sprintf(text, "< S = %3d/%4d %9ld %9ld %9ld V3=%04d E=%ld >", gpsSat, gpsHdop, lat, lon, gpsTime + 2000000, Volt, GSMErrors  );
  sprintf(file, "test%04d.txt", MFile++);
if(transmit)
  return GSMSIM.PutFTP( file, text);
  else
  return GSMOK;
}






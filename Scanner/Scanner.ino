#include <gsmsim800.h>
#include <Wire.h>

#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <Eeprom24C04_16.h>

char TmpBuffer[200];
char ScanBuffer[206];
char IMEI[20];
/////////////////////////////////////////
// PIN DEFINITION
/////////////////////////////////////////



#define SCANNER_TX      3
#define SCANNER_RX      4

#define SCANNER_AIM     12
#define SCANNER_TRIGGER 13
#define SCANNER_PWRDOWN 5
#define SCAN_BUTTON     2
#define BUZZER          6

#define GSM_RX    8
#define GSM_TX    9
#define GSM_BOOT_PIN  7


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
// APP DEFINITION


/////////////////////////////////////////
// EEPROM24C04 DEFINITION
/////////////////////////////////////////
//#define EEPROM_ADDRESS 0x50
//#define EEPROM_ADDRESS 0x57
//static Eeprom24C04_16 eeprom(EEPROM_ADDRESS);

/////////////////////////////////////////
// SCANNER DEFINITION
/////////////////////////////////////////
SoftwareSerial ScannerSerial(SCANNER_RX, SCANNER_TX); // RX, TX GSM

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

  pinMode(SCANNER_AIM, OUTPUT);     digitalWrite(SCANNER_AIM, 1);
  pinMode(SCANNER_TRIGGER, OUTPUT); digitalWrite(SCANNER_TRIGGER, 1);
  pinMode(GSM_BOOT_PIN, OUTPUT);    digitalWrite(GSM_BOOT_PIN, 0);
  pinMode(BUZZER, OUTPUT);          digitalWrite(BUZZER, 0);
  pinMode(SCANNER_PWRDOWN, INPUT);
  pinMode(SCAN_BUTTON, INPUT_PULLUP);


  //***************************************
  //SETUP BUZZER
  //***************************************
  analogWrite(BUZZER, 150);
  delay(1000);
  analogWrite(BUZZER, 0);


  //***************************************
  //SETUP SCANNER
  //***************************************
  ScannerSerial.begin(9600);
  //ScannerSerial.listen();
  digitalWrite(SCANNER_AIM, 1);
  Serial.print(F("Scanner powerdown pin: "));
  Serial.println(digitalRead(SCANNER_PWRDOWN));
  //while(digitalRead(SCANNER_PWRDOWN)) digitalWrite(SCANNER_AIM, 0);



  //***************************************
  //SETUP GPIO CONTROLLER
  //***************************************
#define GPIO_INPUT 0
#define GPIO_OUTPUT 1
#define GPIO_INVERSION 2
#define GPIO_CONFIGURATION 3
#define GPIO_ADDR 0x18

#define LED_RED    1
#define LED_YELLOW 3
#define LED_GREEN  2
#define LED_OFF    0

#define GSM_LED    1
#define POWER_LED  0
#define SCANNER_LED   2

  Wire.begin();

  set_register(GPIO_ADDR, GPIO_INVERSION, 0x0);
  set_register(GPIO_ADDR, GPIO_CONFIGURATION, 0x0);

  delay(500);
  led(POWER_LED, LED_RED);
  led(GSM_LED, LED_RED);
  led(SCANNER_LED, LED_RED);
  delay(700);
  led(POWER_LED, LED_YELLOW);
  led(GSM_LED, LED_YELLOW);
  led(SCANNER_LED, LED_YELLOW);




  //***************************************
  //SETUP GSM
  //***************************************
  Serial.print(F("GSM setup: "));
  GsmSerial.begin(19200);
  GsmSerial.listen();
  while (GSMSIM.BootGSM(IMEI) == GSMERROR)
  {
    led(GSM_LED, LED_RED);
    led(POWER_LED, LED_OFF);
    analogWrite(BUZZER, 150);
    delay(500);
    analogWrite(BUZZER, 0);
    led(POWER_LED, LED_RED);
  }
  led(GSM_LED, LED_GREEN);


  if ( GPS() ) led(POWER_LED, LED_GREEN);
  else led(POWER_LED, LED_YELLOW);

  //***************************************
  //EEPROM
  //***************************************
#ifdef EEPROM
  Serial.print(F("TEST EXTERNAL EEPROM ... "));
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
  Serial.println(F(" COMPLETED"));
#endif

  //***************************************
  //END SETUP
  //***************************************

  printHelp();
  analogWrite(BUZZER, 150);
  delay(1000);
  analogWrite(BUZZER, 0);

  led(POWER_LED, LED_GREEN);
  led(SCANNER_LED, LED_OFF);

}

void printHelp()
{
  Serial.print(F("\n.AT cmd, p(ost),  g(et), G(PS log), b(oot), f(fun=0)\n"));
  Serial.println(F("cmd# "));
}


void loop() // run over and over
{

  if (!digitalRead(SCAN_BUTTON))
  {
    delay (10);
    if (!digitalRead(SCAN_BUTTON)) scan(0);
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
      case 'b': GSMSIM.BootGSM(IMEI); break;
      case 'g': get(); break;
      case 'G': GPS(); break;
      case 'f': Serial.println(F("low power")); GSMSIM.ConfGSM(0); break;
      case 'p': post("SCANID1,  SEQN,TIME,Lat,Lon"); break;
      case 's': scan(1); break;
      case 't': test(); break;
      default: printHelp();
    }
    { //per svuotare il buffer di input
      long int start = millis();
      while (millis() < start + 200)
        if (Serial.available()) Serial.read();
    }
    Serial.println(F("cmd# "));
  }
  if ((millis() % 20000) == 0) {
    Serial.println(F("GPS"));
    led(POWER_LED, LED_OFF);
    if ( GPS() ) led(POWER_LED, LED_GREEN);
    else led(POWER_LED, LED_YELLOW);
  }
}//end loop

void pipe()
{
  strcat(ScanBuffer, "|");
}

void test()
{

}


void scan(int mode)
{
  unsigned char c = 0 ;
  int i;
  int fix = 0;
  char  scanTmp[100];

  /*
    0     magic                   S + 2
    1     timestamp   15 + 2
    2     lat     10 + 2
    3     long      10 + 2
    4     deltalat    5+ 2
    5     deltalong   5+ 2
    6     imei      15+ 2
    7     barcode           128 + 2
    8     checksum    1
    190+16
  */
  led(SCANNER_LED, LED_YELLOW);

  ScannerSerial.listen();
  digitalWrite(SCANNER_TRIGGER, 0);
  Serial.println(F("start scan"));
  while (!ScannerSerial.available() &&  (mode || !digitalRead(SCAN_BUTTON)));
  if (!ScannerSerial.available())   // non ho letto nulla
  {
    int j;
    digitalWrite(SCANNER_TRIGGER, 1);
    for (j = 0; j < 1000; j++)
    {
      digitalWrite(BUZZER, 1);
      delayMicroseconds(200);
      digitalWrite(BUZZER, 0);
      delayMicroseconds(200);
    }
    Serial.println(F("No acquisition!\n"));
    led(SCANNER_LED, LED_RED);
    delay(500);
    led(SCANNER_LED, LED_OFF);
    return;
  }
  digitalWrite(SCANNER_TRIGGER, 1);
  led(SCANNER_LED, LED_GREEN);
  analogWrite(BUZZER, 150);
  delay(500);
  analogWrite(BUZZER, 30);
  delay(500);
  analogWrite(BUZZER, 0);
  Serial.println(F("End scan"));

  i = 0;
  while (ScannerSerial.available() && i < 100 ) scanTmp[i++] = ScannerSerial.read();
  if (i == 100) {
    Serial.println(F("buffer overflow ")); while (ScannerSerial.available()) ScannerSerial.read();
  }
  scanTmp[i - 1] = 0;
  // for(i=0;scanTmp[i]; i++) Serial.println((int)scanTmp[i]);
  Serial.println(F("READ: "));
  Serial.println(scanTmp);
  led(GSM_LED, LED_YELLOW);
  GsmSerial.listen();
  GsmSerial.println(F("AT+CGNSINF"));
  GSMSIM.GSM_Response(2);
  ScanBuffer[0] = 'S';
  ScanBuffer[1] = 0;
  pipe();

  strtok(TmpBuffer, ",");
  if ( *strtok(0, ",") == '1') fix = 1; //fix
  strcat(ScanBuffer, strtok(0, ".")); // time stamp prendo sempre il piu' recente
  pipe();
  if (!fix) {
    strncpy(TmpBuffer, ScanBuffer + 100, 100);
    strtok(TmpBuffer, ",");
    strtok(0, ",");
    strtok(0, ".");
    ScanBuffer[0] = '?';
    led(POWER_LED, LED_YELLOW);
  } else {
    led(POWER_LED, LED_GREEN);
  }



  strtok(0, ","); //reserved
  strcat(ScanBuffer, strtok(0, ",")); // lat
  pipe();

  strcat(ScanBuffer, strtok(0, ",")); // lon
  pipe();
  strtok(0, ",");// alt
  strtok(0, ","); // speed
  strtok(0, ","); // course
  strtok(0, ","); // mode
  strcat(ScanBuffer, strtok(0, ",")); //HDOP
  pipe();
  strcat(ScanBuffer, strtok(0, ",")); //PDOP
  pipe();
  strtok(0, ","); //VDOP
  strcat(ScanBuffer, IMEI); // IMEI
  pipe();
  strcat(ScanBuffer, "w"); // simbologia
  pipe();
  strcat(ScanBuffer, itoa(strlen(scanTmp), TmpBuffer, 10)); // barcode  len
  pipe();
  strcat(ScanBuffer, scanTmp); // barcode
  pipe();
  Serial.println(ScanBuffer);
  c = 0;
  for (i = 0; ScanBuffer[i]; i++) c ^= ScanBuffer[i];
  ScanBuffer[i] = c;
  ScanBuffer[i + 1] = 0;
  Serial.println(c);


  GSMSIM.HTTP_post(ScanBuffer, i + 1, F("http://184.73.165.170/recapitocerto/xxx.php" ));
  // GSMSIM.HTTP_post(ScanBuffer, i + 1, F("http://184.73.165.170/b2bg/xxx.php" ));
  //  Serial.println(TmpBuffer);
  if ((unsigned int) * (strstr(TmpBuffer, "#") + 1) != (unsigned int) ScanBuffer[i])
  {
    Serial.println("Disastro!\n");
    Serial.println( (unsigned char) * (strstr(TmpBuffer, "OK") + 1));
    Serial.println(ScanBuffer + i);
    led(GSM_LED, LED_RED);
  }
  else
  {
    Serial.println("Successo!\n");
    led(GSM_LED, LED_GREEN);
  }
  led(SCANNER_LED, LED_OFF);
}


int GPS()
{
  int fix = 0;
  GsmSerial.println(F("AT+CGNSINF"));
  GSMSIM.GSM_Response(2);
  Serial.println(TmpBuffer);
  strncpy(ScanBuffer, TmpBuffer, 100);
  strtok(TmpBuffer, ",");
  if ( *strtok(0, ",") == '1') fix = 1; //fix
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
  (strtok(0, ",")); //VDOP
  Serial.println("SAT VIEW, USED");
  Serial.println(strtok(0, ","));
  Serial.println(strtok(0, ","));
  Serial.println("END");

  if ( fix) {
    strncpy(ScanBuffer + 100, ScanBuffer, 100);   // salviamo ogni tanto
    Serial.println (F("SAVE"));
  }
  return fix;
}


int  get()
{
  GSMSIM.HTTP_get(8, 90, F("http://184.73.165.170/b2bg/xxx.log"));
  Serial.println(TmpBuffer);
}


int  post(char *msg)
{


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

int statusLed = 0;
void led(int n, int c)
{
  statusLed &= ~(0x3 << (2 + n * 2));
  statusLed |=   (c << (2 + n * 2));
  set_register(GPIO_ADDR, GPIO_OUTPUT, statusLed);
}


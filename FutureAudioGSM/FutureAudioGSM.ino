#include <gsmsim.h>

#include <EEPROM.h>
#include <Wire.h>
#include <SoftwareSerial.h>



// PIN POWER NEOWAY 2 e 4 CONNECTED TO ARDUINO 5V e GND congigui (2to5V  4toGND)
// PIN POWER CEPRI  4 e 2 CONNECTED TO ARDUINO 5V e GND congigui (2toGND 4toVCC)
#define GSM_RX    8      // CEPRI PIN 9   : NEOWAY PIN 14
#define GSM_TX    9      // CEPRI PIN 10  : NEOWAY PIN 16
#define GSM_BOOT_PIN  3       // Neoway M590 boot pin if available : CEPRI PIN 14 : NEOWAY PIN 19
// #define GSM_BOOT_PIN  -1   // if Neoway M590 boot pin NOT available



char TmpBuffer[200];
GSMSIM GSMSIM(GSM_BOOT_PIN, TmpBuffer, sizeof(TmpBuffer), GSM_RX, GSM_TX);

/////////////////////////////////////////&
// PIN DEFINITION
/////////////////////////////////////////
#define AUDIO_RX  4        //can be disconnected to TX of the Serial MP3 Player module
#define AUDIO_TX  12        //connect to RX of the module





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
// APP DEFINITION
/////////////////////////////////////////

#define START_AUTO_LOOP_TIME 10000
int autoLoop = 1;


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


  delay(500);
  i = 0;
  TmpBuffer[0] = 0;
  TmpBuffer[1] = 0;


  //***************************************
  //SETUP ON BOARD LED
  //***************************************

  pinMode(LEDPIN, OUTPUT);


  //***************************************
  //SETUP GSM
  //***************************************
  GSMSIM.BootGSM();

  //***************************************
  //END SETUP
  //***************************************

  help();
}

void help()
{
  Serial.print(F("\n.AT cmd,  S(ms), c(onfIP), a(audio), l(oop), b(oot), P(rocess coded SMS),\n h(alt loop), w(poweroff), R(ead SMS), S(endSMS)\n"));
  Serial.println(F("cmd# "));
}
void loop() // run over and over
{
  //////////////////////////////////////////////////////
  // CONSOLE COMMAND PROCESSING
  //////////////////////////////////////////////////////
  if (Serial.available())
  {
    char a = Serial.read();
    switch (a)
    {
      case 'c': GSMSIM.ConfGSM();  break;
      case 's': GSMSIM.StatusFTP();  break;
      case 'a': Serial.println(F("play file 1")); AudioPlay(1, 0x8); break;
      case 'l': Serial.println(F("audio loop")); loopAudio(); break;
      case 'b': GSMSIM.BootGSM();  break;
      case 'r': Serial.println(GSMSIM.ReadFTP("command.txt")); break;
      case 'P': ReadCodedSMS(); break;
      case 'h': Serial.println(F("no auto loop")); autoLoop = 0;  break;
      case 'R': Serial.println(GSMSIM.ReadSMS()); Serial.println(TmpBuffer); break;
      case 'w': GSMSIM.PowerOffGSM(); break;
      case 'S': GSMSIM.SendSMS("3296315064", "ciao bongo");  break;
      case '.': GSMSIM.ProxyGSM();  break;
      default: help();
    }
    {
      long int start = millis();
      while (millis() < start + 200)
        if (Serial.available()) Serial.read();
    }
    Serial.println(F("cmd# "));
  }

  if (autoLoop)
    if (millis() > START_AUTO_LOOP_TIME)
    {
      Serial.println("Auto Loop!");
      GSMSIM.BootGSM();
      loopAudio();
    }
}//end loop



//////////////////////////////
// coded SMS change Song
/////////////////////////////
int nsong = 1;

void ReadCodedSMS()
{
  char *p;
  Serial.println(F(" - Read SMS #coded: "));
  if (GSMSIM.ReadSMS() != GSMOK)
  {
    Serial.println("\nNO SMS");
  } else {
    Serial.println("\nSMS: ");
    Serial.println(TmpBuffer);
    p = strstr(TmpBuffer, "##");
    if (p)
    {
      if (p[2]  && p[2] >= '1' && p[2] <= '9')
      {
        char mess[30] = "Audio=";
        strcat(mess, p + 2);
        nsong = p[2] - '0';
        Serial.print("\n\nAUDIO=");
        Serial.println(nsong);
        if (p[3] == '!') GSMSIM.SendSMS("3296315064", mess);
        p[3] = 0;
      } else {
        Serial.println("No change");
      }
    }
  }
  GSMSIM.DeleteAllSMS();
  TmpBuffer[0] = 0;
}

void loopAudio()
{
  while (1)
  {
    sendCommand(CMD_PLAY_W_VOL, 0x1500 + nsong );//play the n song with volume
    delay(29000);
    GSMSIM.BootGSM();
    ReadCodedSMS();
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



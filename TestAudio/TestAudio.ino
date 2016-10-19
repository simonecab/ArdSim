/***********************************************************/
//Demo for the Serial MP3 Player by Catalex
//Hardware: Serial MP3 Player *1
//Board:  Arduino UNO R3
//IDE:    Arduino-1.0
//Function:  To play the first song in the micro sd card.
//Store: http://www.aliexpress.com/store/1199788
//          http://www.dx.com/
#include <SoftwareSerial.h>

#define ARDUINO_RX 4//connect to TX of the module (OR NOT CONNECTED)
#define ARDUINO_TX 12//should connect to RX of the Serial MP3 Player module

SoftwareSerial mySerial(ARDUINO_RX, ARDUINO_TX);

// il file in play e' /01/001xxx.wav


static unsigned char Send_buf[8] = {0} ;

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
void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(500);//Wait chip initialization is complete
  sendCommand(CMD_RESET, 0);//chip reset
  delay(200);//wait for 200ms
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card
  delay(200);//wait for 200ms
  sendCommand(CMD_DAC, DAC_START);//start DAC
  delay(2000);//wait for 200ms
  sendCommand(CMD_PLAY_W_VOL, 0X1001);//play the first song with volume 15 class
  delay(3000);

  sendCommand(CMD_PLAY_W_VOL, 0X1002);//play the first song with volume 15 class
  delay(3000);//wait for 200ms

  sendCommand(CMD_PLAY_W_VOL, 0X1003);//play the first song with volume 15 class

  delay(3000);//wait for 200ms
  Serial.println("insert track number 1-9 or a letter a-z for volume control (a min z max), R=reset, S=sleep, W=wake");
}
int vol = 0x15;
void loop()
{
  if (mySerial.available())
  {
    Serial.print("\nR< ");
    {
      long int start = millis();
      while (millis() < start + 500)
        if (mySerial.available()) {
          Serial.print ((int)mySerial.read(), HEX);
          Serial.print (" ");
        }
    }
    Serial.println(">");
  }
  if (Serial.available())
  {
    char a = Serial.read();
    if (a == 'R')
    {
      //   sendCommand(CMD_DAC, DAC_START);
      //  Wake();
      //       sendCommand(CMD_WAKE, NULL);
      sendCommand(CMD_RESET, 0);
    }
    if (a == 'W')
    {
      Wake();
        sendCommand(CMD_DAC, DAC_START);//start DAC
    }
    // sendCommand(CMD_WAKE, NULL);
    //  sendCommand(CMD_RESET, 0);

  if (a == 'S')
  {
    //       sendCommand(CMD_DAC, DAC_STOP);
    //     sendCommand(CMD_SLEEP, NULL);
    //      sendCommand(CMD_DAC, DAC_STOP);
    Sleep();
  }
  
  if (a >= '0' && a <= '9')
  {


    sendCommand(CMD_PLAY_W_VOL, (vol << 8) + a - '0');
    delay(3000);
    //          sendCommand(CMD_DAC, DAC_STOP);

  }
  if (a >= 'a' && a <= 'z')   vol = 30 + (a - 'z') ;

}
}



// 7E FF 06 0A 00 00 00 FE F1 EF
void Sleep()
{

  mySerial.write((unsigned char )0x7E) ;
  mySerial.write((unsigned char )0xFF) ;
  mySerial.write((unsigned char )0x06) ;
  mySerial.write((unsigned char )0x0A) ;
  mySerial.write((unsigned char )0x00) ;
  mySerial.write((unsigned char )0x00) ;
  mySerial.write((unsigned char )0x00) ;
  mySerial.write((unsigned char )0xFE) ;
  mySerial.write((unsigned char )0xF1) ;
  mySerial.write((unsigned char )0xEF) ;
  Serial.print((unsigned char )0x7E, HEX) ;
  Serial.print((unsigned char )0xFF, HEX) ;
  Serial.print((unsigned char )0x06, HEX) ;
  Serial.print((unsigned char )0x0A, HEX) ;
  Serial.print((unsigned char )0x00, HEX) ;
  Serial.print((unsigned char )0x00, HEX) ;
  Serial.print((unsigned char )0x00, HEX) ;
  Serial.print((unsigned char )0xFE, HEX) ;
  Serial.print((unsigned char )0xF1, HEX) ;
  Serial.println((unsigned char )0xEF, HEX) ;
  delay(400);
  if (mySerial.available())
  {
    Serial.print("\nR< ");
    {
      long int start = millis();
      while (millis() < start + 500)
        if (mySerial.available()) {
          Serial.print ((int)mySerial.read(), HEX);
          Serial.print (" ");
        }
    }
    Serial.println(">");
  }
}


//7E FF 06 0B 00 00 00 FE F0 EF
void Wake()
{

  mySerial.write((unsigned char )0x7E) ;
  mySerial.write((unsigned char )0xFF) ;
  mySerial.write((unsigned char )0x06) ;
  mySerial.write((unsigned char )0x0B) ;
  mySerial.write((unsigned char )0x00) ;
  mySerial.write((unsigned char )0x00) ;
  mySerial.write((unsigned char )0x00) ;
  mySerial.write((unsigned char )0xFE) ;
  mySerial.write((unsigned char )0xF0) ;
  mySerial.write((unsigned char )0xEF) ;
  Serial.print((unsigned char )0x7E, HEX) ;
  Serial.print((unsigned char )0xFF, HEX) ;
  Serial.print((unsigned char )0x06, HEX) ;
  Serial.print((unsigned char )0x0B, HEX) ;
  Serial.print((unsigned char )0x00, HEX) ;
  Serial.print((unsigned char )0x00, HEX) ;
  Serial.print((unsigned char )0x00, HEX) ;
  Serial.print((unsigned char )0xFE, HEX) ;
  Serial.print((unsigned char )0xF0, HEX) ;
  Serial.println((unsigned char )0xEF, HEX) ;
  delay(400);
  if (mySerial.available())
  {
    Serial.print("\nR< ");
    {
      long int start = millis();
      while (millis() < start + 500)
        if (mySerial.available()) {
          Serial.print ((int)mySerial.read(), HEX);
          Serial.print (" ");
        }
    }
    Serial.println(">");
  }
}


void sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0xff; //version
  Send_buf[2] = 0x06; //the number of bytes of the command without starting byte and ending byte
  Send_buf[3] = command; //
  Send_buf[4] = 0x00;//0x00 = no feedback, 0x01 = feedback
  Send_buf[5] = (int8_t)(dat >> 8);//datah
  Send_buf[6] = (int8_t)(dat); //datal
  Send_buf[7] = 0xef; //ending byte
  for (uint8_t i = 0; i < 8; i++) //
  {
    Serial.print(Send_buf[i], HEX);   Serial.print(" ");
    mySerial.write(Send_buf[i]) ;
  }
  Serial.println(" ");
  delay(400);
  if (mySerial.available())
  {
    Serial.print("\nR< ");
    {
      long int start = millis();
      while (millis() < start + 500)
        if (mySerial.available()) {
          Serial.print ((int)mySerial.read(), HEX);
          Serial.print (" ");
        }
    }
    Serial.println(">");
  }
}


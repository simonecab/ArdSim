// Standalone code for BY8001-16P MP3 Player
// C.Bosch July 2016
//
#include "SoftwareSerial.h"
SoftwareSerial mp3Serial(4, 12);

void setup () {
  Serial.begin(9600);
  Serial.println("start");
  mp3Serial.begin (9600);

  delay(800); // wait for boot

  // Ask Version, for debug
  byte Command_line[5] = { 0x7E, 0x03, 0x14, 0x17, 0xEF };
  //  byte Command_line[5] = { 0x7E, 0x03, 0x11, 0x12, 0xEF }; // ask volume
  for (byte k = 0; k < 5; k++)
  {
    mp3Serial.write(Command_line[k]);
  }
  unsigned long mTimer = millis(); while (millis() - mTimer < 200) {}
  char receiveBuffer[4];
  mp3Serial.readBytes(receiveBuffer, 6);
  Serial.println(receiveBuffer[0], HEX);
  Serial.println(receiveBuffer[1], HEX);
  Serial.println(receiveBuffer[1], HEX);
  Serial.println(receiveBuffer[3], HEX);
  Serial.println("play");
  /*
    // RESET MODULE
    byte Command_line[5] = { 0x7E, 0x03, 0x09, 0x0A, 0xEF };
    for (byte k=0; k<5; k++)
    { mp3Serial.write(Command_line[k]); }
    unsigned long mTimer = millis(); while (millis() - mTimer < 3000) {}
  */
  setVolume(4); // When UNO 5V powered, reset if > 0x10. Use external power supply !
  setEq(0);
  playNext();
}

void loop () {
}
// CMD (Command, 0=NoParam/1=OneParam, Param)
void pause()
{
  execute_CMD(0x02, 0, 0);
}
void play()
{
  execute_CMD(0x01, 0, 0);
}
void playStop()
{
  execute_CMD(0x0E, 0, 0);
}
void playNext()
{
  execute_CMD(0x03, 0, 0);
}
void playPrev()
{
  execute_CMD(0x04, 0, 0);
}
void setVolume(int volume)
{
  execute_CMD(0x31, 1, volume);  // Set the volume (0x00~0x1E) (0~30)
}
void incVolume()
{
  execute_CMD(0x05, 0, 0);
}
void decVolume()
{
  execute_CMD(0x06, 0, 0);
}
void setEq(int eq)
{
  execute_CMD(0x32, 1, eq);  // EQ : 0=Normal/1=Pop/2=Rock/3=Jazz/4=Classic/5=Bass
}

//
// 0x7E / Number of bytes following / Command / Params / Check / 0xEF
// ex : Play = 7E 03 01 02 EF, with 02 = 03 XOR 01 (must XOR length + Command + Params)
//
void execute_CMD(byte CMD, bool Par1, int Par2)
// Excecute the command and parameters
{
  switch (Par1) {
    case 0:
      {
        byte count = 0;
        byte Command_line[5] = { 0x7E, 0x03, CMD, count, 0xEF };
        for (int i = 1; i < 3; i++) {
          count = count ^ Command_line[i]; // XOR
        }
        Command_line[3] = count;
        for (byte k = 0; k < 5; k++)
        {
          mp3Serial.write(Command_line[k]);
        }
        break;
      }
    case 1:
      byte count = 0;
      byte bPar2 = Par2;
      byte Command_line[6] = { 0x7E, 0x04, CMD, bPar2, count, 0xEF };
      for (int i = 1; i < 4; i++) {
        count = count ^ Command_line[i]; // XOR
      }
      Command_line[4] = count;
      for (byte k = 0; k < 6; k++)
      {
        mp3Serial.write(Command_line[k]);
      }
      break;
  }
  unsigned long mTimer = millis(); while (millis() - mTimer < 100) {}
}

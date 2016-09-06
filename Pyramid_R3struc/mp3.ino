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




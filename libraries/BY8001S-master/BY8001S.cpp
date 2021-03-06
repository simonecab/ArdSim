// BY8001S.cpp, BY8001S Mp3 Player Function Library
// Date: June 2015
// Author: borland of Arduino forum 
// No affiliation with Balway Electronics Technology or Shenzhen Electronics Technology Ltd.
// Released in public domain

#include "Arduino.h"
#include "BY8001S.h"

BY8001S::BY8001S() {
   sendBuffer[0] = 0x7E;
   sendBuffer[6] = 0xEF;
}

void BY8001S::setup(Stream& serial){
   mp3Serial = &serial;
}

void BY8001S::sendCommand(byte cmd){
   sendBuffer[1] = 0x03; // length
   sendBuffer[2] = cmd;
   fillCheckCode();
   sendBuffer[4] = 0xEF;
   txFunc();
}

void BY8001S::sendCommand(byte cmd, byte pmtr){
   sendBuffer[1] = 0x04;  // length
   sendBuffer[2] = cmd;
   sendBuffer[3] = pmtr;
   fillCheckCode();
   sendBuffer[5] = 0xEF;
   txFunc();
}

void BY8001S::sendCommand(byte cmd, word pmtr){
   sendBuffer[1] = 0x05;  // length
   sendBuffer[2] = cmd;
   sendBuffer[3] = highByte(pmtr); sendBuffer[4] = lowByte(pmtr);
   fillCheckCode();
   txFunc();
}

void BY8001S::sendCommand(byte cmd, byte pmtr1, byte pmtr2){
   sendBuffer[1] = 0x05;  // length
   sendBuffer[2] = cmd;
   sendBuffer[3] = pmtr1; sendBuffer[4] = pmtr2;
   fillCheckCode();
   txFunc();
}

void BY8001S::fillCheckCode(){
  byte checkCode = getCheckCode();
  sendBuffer[(int)sendBuffer[1]] = checkCode; 
}

byte BY8001S::getCheckCode(){
  byte count = 0;
  int j = sendBuffer[1];
  for(int i=1; i<j; i++){
   count = count ^ sendBuffer[i]; // XOR checksum or check code 
  }
  return count;
}

void BY8001S::txFunc(){
int i;
  while ( mp3Serial->available() >0 ){ Serial.print( (int) mp3Serial->read()); Serial.print( " "); } ;  Serial.println("") ;
  mp3Serial->write(sendBuffer, (int)sendBuffer[1] + 2); 
  Serial.print( "TX ");
  for(i=0;i<((int)sendBuffer[1]) + 2; i++) { Serial.print( ((int)sendBuffer[i])&0xFF,HEX); Serial.print( " "); }
  Serial.println("") ;
}

void BY8001S::rxFunc(int length){
	  int i; int el;
  delay (500);
  unsigned long mTimer = millis(); while (millis() - mTimer < 130) {}
  el=(int)mp3Serial->readBytes(receiveBuffer, length);
  if(el!=length)   {  Serial.print( "READ "); Serial.print( el);Serial.print( "EXPECT  "); Serial.print( length); }
    Serial.print( "RX ");
  for(i=0;i<el; i++) { Serial.print( ((int)receiveBuffer[i])&0xFF,HEX); Serial.print( " "); }
   while ( mp3Serial->available() >0 ){ Serial.print( (int) mp3Serial->read()); Serial.print( " "); } ;  Serial.println("") ;

  Serial.println("") ;
}

word BY8001S::getReply(int length, int offset){
  rxFunc(length);
  word value = strtoul(receiveBuffer + offset, NULL, 16);
  return value;
}

// User Command Functions:
void BY8001S::play(){
  sendCommand(0x01);
}

void BY8001S::pause(){
  sendCommand(0x02); 
}

void BY8001S::nextTrack(){
  sendCommand(0x03); 
}

void BY8001S::previousTrack(){
  sendCommand(0x04); 
}

void BY8001S::increaseVolume(){
  sendCommand(0x05); 
}

void BY8001S::decreaseVolume(){
  sendCommand(0x06); 
}

void BY8001S::toggleStandbyMode(){
  sendCommand(0x07); 
}
 
void BY8001S::resetModule(){
  sendCommand(0x09); 
}

void BY8001S::fastForward(){
  sendCommand(0x0A); 
}

void BY8001S::fastRewind(){
  sendCommand(0x0B); 
}

void BY8001S::stopPlayback(){
  sendCommand(0x0E); 
}

void BY8001S::setVolume(int volume){
  sendCommand(0x31, (byte) volume);
  // delay 100ms
  unsigned long mTimer = millis(); while (millis() - mTimer < 100) {}
}
void BY8001S::setEqualizerProfile(int equalizerProfile){
  sendCommand(0x32,(byte) equalizerProfile );
  // delay 100ms
  unsigned long mTimer = millis(); while (millis() - mTimer < 100) {}
}

void BY8001S::setLoopPlaybackMode(int loopPlaybackMode){
  sendCommand(0x33,(byte) loopPlaybackMode );
  // delay 100ms
  unsigned long mTimer = millis(); while (millis() - mTimer < 100) {}
}

void BY8001S::switchFolder(int swDirection){
  sendCommand(0x34, (byte) swDirection);
}

void BY8001S::switchDevice(int device){
  sendCommand(0x35,(byte) device ); 
}

void BY8001S::playTrackByIndexNumber(word trackNumber){
  sendCommand(0x41, trackNumber); 
}

void BY8001S::playTrackFromFolder(int folderNumber, int trackNumber){
 sendCommand(0x42, (byte) folderNumber, (byte) trackNumber); 
}

void BY8001S::playInsertionSpot(int trackNumber){
 sendCommand(0x43, (byte) trackNumber); 
}

void BY8001S::playInsertionFromFolder(int folderNumber, int trackNumber){
 sendCommand(0x44, (byte) folderNumber, (byte) trackNumber); 
}

// User Query Commands:
byte BY8001S::getPlaybackStatus(){
  sendCommand(0x10);
  byte currentStatus = getReply(8,0);
  return currentStatus; 
}

byte BY8001S::getVolumeSetting(){
  sendCommand(0x11);
  byte volume = getReply(8,0);
  return volume;
}

byte BY8001S::getEqualizerProfile(){
  sendCommand(0x12);
  word profile = getReply(8,0);
  return (byte)profile;
}

byte BY8001S::getLoopPlaybackMode(){
  sendCommand(0x13);
  byte loopPlaybackMode = getReply(8,0);
  return loopPlaybackMode;
}

char* BY8001S::getFirmwareVersion(){
  sendCommand(0x14);
  rxFunc(4); 
  String versionReturned = receiveBuffer;
  char * fwVersion = new char[5];
  versionReturned = versionReturned.substring(0,4);
  versionReturned.toCharArray(fwVersion,5);
  return fwVersion;
}

word BY8001S::getNumberOfTracksTF(){
  sendCommand(0x15);
  word numberOfTracksTF = getReply(8,0);
  return numberOfTracksTF;
}

word BY8001S::getNumberOfTracksUSB(){
  sendCommand(0x16);
  word numberOfTracksUSB = getReply(8,0);
  return numberOfTracksUSB;
}

byte BY8001S::getCurrentPlaybackDevice(){
  sendCommand(0x18);
  byte device = getReply(8,0);
  return device;
}

word BY8001S::getCurrentTrackTF(){
  sendCommand(0x19);
  word currentTrackTF = getReply(8,0);
  return currentTrackTF;
}

word BY8001S::getCurrentTrackUSB(){
  sendCommand(0x1A);
  word currentTrackUSB = getReply(8,0);
  return currentTrackUSB;
}

word BY8001S::getElapsedTrackPlaybackTime(){
  sendCommand(0x1C);
  word elapsedTrackPlaybackTime = getReply(8,2);
  return elapsedTrackPlaybackTime; 
}

word BY8001S::getTotalTrackPlaybackTime(){
  sendCommand(0x1D);
  word totalTrackPlaybackTime = getReply(8,2);
  return totalTrackPlaybackTime; 
}

char* BY8001S::getFileNameCurrentTrack(){
  // delay 130ms
  unsigned long mTimer = millis(); while (millis() - mTimer < 130) {}
  sendCommand(0x1E);
  rxFunc(15);
  String nameReturned = receiveBuffer;
  char * name = new char[10];
  nameReturned = nameReturned.substring(0,15);
  if (nameReturned.endsWith("\r\n") == true) {
    nameReturned = nameReturned.substring(2,10);
  }
  else {
    nameReturned = nameReturned.substring(0,8);
  }
  nameReturned.toCharArray(name,9);
  return name;
}

word BY8001S::getNumberOfTracksCurrentFolder(){
 sendCommand(0x1F);
 word numberTracksCurrentFolder = getReply(8,2);
 return numberTracksCurrentFolder; 
}
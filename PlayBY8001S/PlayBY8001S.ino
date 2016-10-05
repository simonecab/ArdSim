#include <BY8001S.h>


// BY8001S MP3 Player Library, Example Program 1; Sequential Playback Demo 
// Date: June 2015
// Author: borland of Arduino forum 
// No affiliation with Balway Electronics Technology or Shenzhen Electronics Technology Ltd.
// Released in public domain.
// 
// This example sketch uses SoftwareSerial to communnicate with BY8001S
// and sequentially plays tracks, & sends track file names to Arduino IDE for display on serial monitor 
//
// Recommend module ABC keypad mode set to '111' in order to support all serial commands 
// Uses Arduino's hardware serial port to connect to PC

// Requires 3.3V TTL level RX/TX such as Ardiuno Mini Pro (8Mhz, 3.3V)
// can use 5V/3.3V level shifter or try 1K ohm resistor on TX line as shown in BY8001S datasheet


//
// Connect Arduino's RX pin to BY8001S's TX pin 4
// Connect Arduino's TX pin to BY8001S's RX pin 12
//

#include <SoftwareSerial.h>


// function prototype
bool checkFor_mp3IsStopped();

SoftwareSerial mp3Serial(4, 12);  // RX, TX
BY8001S mp3;  // creating an instance of class BY8001S and call it 'mp3'

void setup(){
  Serial.begin(9600);  // set serial monitor baud rate to Arduino IDE
  mp3Serial.begin(9600);  // BY8001S set to 9600 baud (required)  

  mp3.setup(mp3Serial); // tell BY8001S library which serial port to use. 
  mp3Serial.setTimeout(1500);
  delay(2000);  // allow time for BY8001S cold boot; may adjust depending on flash storage size

  char * fwVersion = mp3.getFirmwareVersion();
  Serial.print("BY8001S module firmware version: "); Serial.println(fwVersion);
  delay(2000);  // allow time for BY8001S cold boot; may adjust depending on flash storage size

  int BY8001S_VolumeSetting = 29; //  Set volume to 15 (0-30 range) if not already set
  if ((int)mp3.getVolumeSetting() != BY8001S_VolumeSetting) mp3.setVolume(BY8001S_VolumeSetting);
  
  int volume = mp3.getVolumeSetting(); // just verifying has been set before serial print to console
  Serial.print("speaker volume setting: "); Serial.println(volume);
  
  word numOfTracksTF = mp3.getNumberOfTracksTF();
  Serial.print("\nNumber of files on micro SD-card: "); Serial.println(numOfTracksTF);
  mp3.stopPlayback();
  

}

void loop(){
  static word totalTime, elapsedTime, playback;
  
  delay(900);
  
  if (checkFor_mp3IsStopped() == true){
    mp3.nextTrack();
    char* fileName = mp3.getFileNameCurrentTrack();
    Serial.print("\n\nplaying file name: "); Serial.println(fileName);
    totalTime = mp3.getTotalTrackPlaybackTime();
    Serial.print("playback duration: "); Serial.print(totalTime); Serial.println(" seconds");
  }
  elapsedTime = mp3.getElapsedTrackPlaybackTime();
  playback = totalTime - elapsedTime;
  Serial.print(playback); Serial.print(",");
 if(millis()>15000) while (1);
}

// check for if Mp3 Player is stopped
bool checkFor_mp3IsStopped() {
  if (mp3Serial.available() > 0){
    if (mp3.getPlaybackStatus() == 0){
      return true;
    }
  }
  else return false; 
}

/*   command set usage:
  mp3.play();
  mp3.pause();
  mp3.nextTrack();
  mp3.previousTrack();
  mp3.increaseVolume();
  mp3.decreaseVolume();
  mp3.toggleStandbyMode();
  mp3.resetModule();
  mp3.fastForward();
  mp3.fastRewind();
  mp3.stopPlayback();
  mp3.setVolume(int volume);  // 0-30
  mp3.setEqualizerProfile(int equalizerProfile); // 0/1/2/3/4/5  Normal/Pop/Rock/Jazz/Classic/Bass
  mp3.setLoopPlaybackMode(int loopPlaybackMode); // 0/1/2/3/4 All/Folder/Single/Random/Disabled 
  mp3.switchFolder(int swDirection); // 0/1 Previous/Next
  mp3.switchDevice(int device); // 0/1 USB/TFcard
  mp3.playTrackByIndexNumber(word trackNumber); // 1-65535
  mp3.playTrackFromFolder(int folderNumber, int trackNumber);  00-99 folder, 001-255 track  
  mp3.playInsertionSpot(int trackNumber);  // TF device not supported
  mp3.playInsertionFromFolder(int folderNumber, int trackNumber);  // TF device not supported
  
  byte playbackStatus = mp3.getPlaybackStatus(); // 
  byte volumeSetting = mp3.getVolumeSetting();
  byte equalizerProfile = mp3.getEqualizerProfile();
  byte loopPlaybackMode = mp3.getLoopPlaybackMode();
  char* fwVersion = mp3.getModuleFirmwareVersion(); Serial.println("firmware ver.: " + fwVersion);
  word numOfTracksTF = mp3.getNumberOfTracksTF();
  word numOfTracksUSB = mp3.getNumberOfTracksUSB();
  byte playbackDevice = mp3.getCurrentPlaybackDevice();
  word currentTrackTF = mp3.getCurrentTrackTF();
  word currentTrackUSB = mp3.getCurrentTrackUSB();
  word elapsedTrackPlaybackTime = mp3.getElapsedTrackPlaybackTime();
  word totalTrackPlaybackTime = mp3.getTotalTrackPlaybackTime();
  char* trackFileName = mp3.getFileNameCurrentTrack(); Serial.print("track file name: "); Serial.println(trackFileName);
  word numTracksInFolder = mp3.getNumberOfTracksCurrentFolder();
  */
 

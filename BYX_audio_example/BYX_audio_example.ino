
#include "BY8X01-16P.h"
#include <SoftwareSerial.h>
// FURTHER INFO https://github.com/NachtRaveVL/BY8X01-16P-Arduino

#define ARDUINO_RX 4//connect to TX of the module (OR NOT CONNECTED)
#define ARDUINO_TX 12//should connect to RX of the Serial MP3 Player module

SoftwareSerial mySerial(ARDUINO_RX, ARDUINO_TX);
BY8X0116P audioController(mySerial);          // Library using default Serial1 UART and no busy pin hookup

void setup() {
    mySerial.begin(9600);            // Serial1 must be started first - only supported UART baud rate is 9600
    Serial.begin(9600);            // Serial1 must be started first - only supported UART baud rate is 9600
Serial.println("Eccomi");
    pinMode(ARDUINO_RX, INPUT);          // Must manually setup pin modes for RX/TX pins
    pinMode(ARDUINO_TX, OUTPUT);
    audioController.init();         // Initializes module

    audioController.setVolume(20);  // Sets player volume to 20 (out of 30 max)

    audioController.printModuleInfo();

    audioController.play();         // Starts playback of loaded tracks
}


void loop()
{}

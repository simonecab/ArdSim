
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


volatile boolean TurnDetected;
volatile boolean up;

const int PinCLK = 2;                 // Used for generating interrupts using CLK signal
const int PinDT = 3;                  // Used for reading DT signal
const int PinSW = 4;                  // Used for the push button switch

  static long virtualPosition = 0;  // without STATIC it does not count correctly!!!

void isr ()  {                    // Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK
  if (digitalRead(PinCLK))
    virtualPosition += digitalRead(PinDT)? 1 : -1;


}





void setup()   {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  //display.display();
  pinMode(PinCLK, INPUT);
  pinMode(PinDT, INPUT);
  pinMode(PinSW, INPUT_PULLUP);
  attachInterrupt (digitalPinToInterrupt(PinCLK), isr, FALLING); // interrupt 0 is always connected to pin 2 on Arduino UNO
  delay(2000);
  display.setRotation(0);
  display.clearDisplay();
  display.display();
  delay(3000);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(0);
  display.println("Hello");
  display.display();
  delay(3000);

}

int i;
void loop() {

  if (!(digitalRead(PinSW))) {      // check if pushbutton is pressed
    virtualPosition = 0;            // if YES, then reset counter to ZERO
    Serial.print ("Reset = ");      // Using the word RESET instead of COUNT here to find out a buggy encoder
    Serial.println (virtualPosition);
    delay(300);
  }



  display.setCursor(8, 0);
  display.setTextSize(0);
  display.println("Rotator");
  display.setCursor(8, 8);
  display.println(virtualPosition);
  display.display();
  display.clearDisplay();

}



void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
}

void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.clearDisplay();
  display.println("scroll");
  display.display();

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}


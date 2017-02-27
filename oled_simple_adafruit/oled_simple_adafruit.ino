
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);


void setup()   {
  Serial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  //display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(0);
  display.println("Hello, world!");
  display.println("I am here");
  display.display();
  display.clearDisplay();
  Serial.println("starting");

}

int i;
void loop() {
  int i;
  delay (500);
  display.setTextSize(0);
  for (i=0;i<11;i++)
  {
    display.setCursor(i*11, 0);
    display.print(i);  
  }
  for (i=0;i<10;i++)
  {
    display.setCursor(i*10, 7);
         if (!(rand()&0x3)) display.print("."); else
    if (rand()&1) display.print("-"); else  display.print("+"); 

  }
  
 // for (i=11;i<19;i++)
 // {
 //   display.setCursor((i-11)*16, 17);
 //   display.print(i);  
 // }
  for (i=11;i<20;i++)
  {
    display.setCursor((i-11)*16+1, 24);
         if (!(rand()&0x3)) display.print("?"); else
    if (rand()&1) display.print("-"); else  display.print("+"); 

  }


  display.display();
  delay(1000);
  display.clearDisplay();

}




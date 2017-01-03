/*

  Ele_logo.pde 
  Put the Elecrow logo on the display.
  >>> Before compiling: Please remove comment from the constructor of the 
  >>> connected graphics display (see below).
  
  Universal 8bit Graphics Library, http://code.google.com/p/u8glib/
  https://code.google.com/p/u8glib/wiki/fontsize
  Copyright (c) 2012, olikraus@gmail.com
  All rights reserved.
*/


#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);	

void drawURL(void)
{
   u8g.setFont(u8g_font_9x18);
   u8g.drawStr(25,10,"Elecrow");
   u8g.setFont(u8g_font_7x14);
   u8g.drawStr(5,35,"www.elecrow.com");

}
void setup(void) {
}

void loop(void) {
  // picture loop
  u8g.firstPage();  
  do {
    drawURL();
  u8g.setColorIndex(1);
  } while( u8g.nextPage() );  
  // rebuild the picture after some delay
  delay(200);  
}


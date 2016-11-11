#include <Wire.h> 
#include <PCF8574_HD44780_I2C.h>


// Address 0x27, 16 chars, 2 line display
PCF8574_HD44780_I2C lcd(0x3f,16,2);

char Message[12];
//HC RS04 Sensore ultrasuoni
//Giuseppe Caccavale
const int triggerPort = 9;
const int echoPort = 10;
const int led = 13;

void setup() {

pinMode(triggerPort, OUTPUT);
pinMode(echoPort, INPUT);
pinMode(led, OUTPUT);
Serial.begin(9600);
Serial.print( "Sensore Ultrasuoni: ");
  lcd.init();           // LCD Initialization              
  lcd.backlight();      // Backlight ON
  lcd.clear();          // Clear the display
  
  lcd.setCursor(0,0);   // Set the cursor to col 0, row 0
  lcd.print("Hello");   // Print the first word
  lcd.setCursor(0,1);   // Set the cursor to col 0, row 1
  lcd.print("World!");  // Print the second word
}

void loop() {

//porta bassa l'uscita del trigger
digitalWrite( triggerPort, LOW );
//invia un impulso di 10microsec su trigger
digitalWrite( triggerPort, HIGH );
delayMicroseconds( 10 );
digitalWrite( triggerPort, LOW );

long durata = pulseIn( echoPort, HIGH );

long distanza = 0.034 * durata / 2;

Serial.print("distanza: ");

//dopo 38ms è fuori dalla portata del sensore
if( durata > 38000 ){
Serial.println("Fuori portata   ");
sprintf(Message,"Fuori portata   ");
}
else{ 
Serial.print(distanza); 
Serial.println(" cm     ");
sprintf(Message,"%d cm", distanza);
}

  lcd.clear();          // Clear the display
  
  lcd.setCursor(0,0);   // Set the cursor to col 0, row 0
  lcd.print(Message);   // Print the first word
delay(1000);
if(distanza < 10){
 digitalWrite(led, HIGH);
}
else{
 digitalWrite(led, LOW);
}

//Aspetta 1000 microsecondi
delay(1000);
}

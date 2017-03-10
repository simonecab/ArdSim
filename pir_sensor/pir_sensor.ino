
volatile boolean TurnDetected;
volatile boolean up;

const int PinCLK=2;                   // Used for generating interrupts using CLK signal
const int PinDT=3;                    // Used for reading DT signal
const int PinSW=4;                    // Used for the push button switch

void isr ()  {                    // Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK
 if (digitalRead(PinCLK))
   up = !digitalRead(PinDT);
 else
   up = digitalRead(PinDT);
 TurnDetected = true;
}


void setup ()  {
 pinMode(PinCLK,INPUT);
 pinMode(PinDT,INPUT);  
 pinMode(PinSW,INPUT_PULLUP);
// attachInterrupt (digitalPinToInterrupt(PinCLK),isr,FALLING);   // interrupt 0 is always connected to pin 2 on Arduino UNO
 Serial.begin (9600);
 Serial.println("Start");
}

void loop ()  {
   Serial.println (digitalRead(3));
   delay(300);
 }  
 
 

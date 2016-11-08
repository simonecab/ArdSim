

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor
#define BEEP 5
#define LASER 6
#define SENSOR 7
void setup() {
pinMode(SENSOR, INPUT);
pinMode(LASER, OUTPUT);
Serial.begin(9600);
analogWrite(BEEP, 0);
}
int i=0;
void loop() {

if(Serial.available()) if (Serial.read()=='q') i^=120;
  //  sensorValue = analogRead(sensorPin);
  // turn the ledPin on
  digitalWrite(LASER,LOW);
  delay(200);
  Serial.println(digitalRead(SENSOR));
  delay(200);
  digitalWrite(LASER,HIGH);
  analogWrite(BEEP, 0);
  delay(200);
  if( digitalRead(SENSOR)) analogWrite(BEEP, i);
  Serial.println(digitalRead(SENSOR));
  delay(200);
}

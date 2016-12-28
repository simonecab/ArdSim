#define pinLED PC13

void setup() {
  Serial.begin(9600);
  pinMode(pinLED, OUTPUT);
  Serial.println("START");  
}

void loop() {
  digitalWrite(pinLED, HIGH);
  delay(500);
  digitalWrite(pinLED, LOW);
    delay(100);
  Serial.println("Hello World");  
}

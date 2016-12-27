void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);  //This initialize serial at 9600 baud
Serial1.begin(9600);
Serial2.begin(9600);
delay(5000);
Serial.println("uno uno inizio");

}
int i=0;
void loop() {
delay(100);

Serial.println("uno uno bis");Serial.println(i++);
Serial1.println("due");
Serial2.println("tre");
}

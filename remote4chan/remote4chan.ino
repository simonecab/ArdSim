
#define STROBE 12
#define DATA0 8
#define DATA1 9
#define DATA2 10  
#define DATA3 11
void setup() {

pinMode(STROBE, INPUT);
pinMode(DATA0, INPUT);
pinMode(DATA1, INPUT);
pinMode(DATA2, INPUT);
pinMode(DATA3, INPUT);

Serial.begin(9600);

}

void loop() {
if(  digitalRead(STROBE))
{
int i=digitalRead(DATA0)*0+ digitalRead(DATA1)*1 + digitalRead(DATA2) *2 + digitalRead(DATA3)*3;

    Serial.println((char)('A'+i));
  delay(200);
}
}

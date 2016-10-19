//   light  monitor
#define MOSPIN 11

unsigned long int duration;
unsigned long int start;

int innescato = 0;
void setup()
{
  Serial.begin(9600);

  pinMode(MOSPIN, OUTPUT);
  digitalWrite(MOSPIN, HIGH);
  delay(1000);

}



void loop()

{

  Serial.print("low  ");
    digitalWrite(MOSPIN, LOW);
  delay(4000);
    Serial.print("high  ");
    digitalWrite(MOSPIN, HIGH);
  delay(4000);
  }


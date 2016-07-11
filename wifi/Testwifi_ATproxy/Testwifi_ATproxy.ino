#include <SoftwareSerial.h>
SoftwareSerial wifi(8, 9);
//                       TOP VIEW         -resistor-
//  TX-8            GND
//  CH_PD-3.3       GPIO2
//  RST             GPIO0
//  VCC-3.3         RX-9           -black area-


char TmpBuffer[200];
#define WIFIIGNOREERROR 1
#define WIFIERROR       2
#define WIFIOK          1
#define WIFIUNKNOWN     0

void setup()
{
  wifi.begin(4800);
  Serial.begin(9600);
  Serial.println("WIFI ESP8266"); delay(2000);
  /* Volendo provare diversi baudrate  */

  while (0) {
    prova(4800L);
    prova(9600L);
    prova(19200L);
    prova(38400L);
    prova(57600L);
    prova(115200L);
    wifi.println("AT+UART_DEF=4800,8,1,0,0");
    prova(4800L);
  }
  prova(4800L);
  Serial.print(F("\nAT cmd, s(can), a(ttach)\n"));
  Serial.println(F("cmd# "));

}

void loop() {
  //////////////////////////////////////////////////////
  // CONSOLE COMMAND PROCESSING
  //////////////////////////////////////////////////////
  while (wifi.available())
    Serial.write(wifi.read());
  if (Serial.available())
  {
    char a = Serial.read();
    switch (a)
    {
      case 's': wifiScan(); while (Serial.available()) Serial.read(); break;
      case 'a': wifiAttach(); while (Serial.available()) Serial.read(); break;

      default:
        {
          long int start = millis();
          wifi.write(a);
          delay(100);
          while (Serial.available())wifi.write(Serial.read());
          while (millis() < start + 4000)
            if (wifi.available()) {
              Serial.write(a = wifi.read());
            }
        }
    }

  }
}

void wifiAttach()
{
  WIFI_AT(F("AT+CWJAP_CUR=\"arakneneat\",\"x1x2x3x4x5\""));
  WIFI_AT(F("AT+PING=\"8.8.8.8\""));
}

void wifiScan()
{
  WIFI_AT(F("AT+CWLAP"));
}

////////////////////////////////////////////////////

int WIFI_AT(const __FlashStringHelper * ATCommand)
{
  long int start = millis();
  wifi.println(ATCommand);
  Serial.println(ATCommand);
  while (millis() < start + 5000)
    if (wifi.available())
      Serial.write(wifi.read());
  return 1;
}




void  prova(long int baud)
{
  long int start = millis();
  wifi.end();
  wifi.begin(baud);
  Serial.println(baud);
  wifi.println("AT");
  while (millis() < start + 1000)
    if (wifi.available())
      Serial.write(wifi.read());
}

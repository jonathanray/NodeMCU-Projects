#include <Arduino.h>
#define LED1 LED_BUILTIN
#define LED2 LED_BUILTIN_AUX
#define ON HIGH
#define OFF LOW
#define ONE_SECOND 1000

void setup()
{
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void loop()
{
  digitalWrite(LED1, ON);
  digitalWrite(LED2, OFF);

  delay(ONE_SECOND);

  digitalWrite(LED1, OFF);
  digitalWrite(LED2, ON);

  delay(ONE_SECOND);
}

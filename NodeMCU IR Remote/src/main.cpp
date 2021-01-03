#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define LED1 LED_BUILTIN
#define LED2 LED_BUILTIN_AUX
#define ON LOW
#define OFF HIGH

const uint64_t IRKEY_UP = 0x40BF00FF;
const uint64_t IRKEY_DOWN = 0x40BF20DF;
const uint64_t IRKEY_1 = 0x40BF609F;
const uint64_t IRKEY_2 = 0x40BF50AF;
const uint64_t IRKEY_3 = 0x40BF708F;
const uint64_t IRKEY_4 = 0x40BF906F;
const uint64_t IRKEY_5 = 0x40BFB04F;
const uint64_t IRKEY_MEM = 0x40BF6897;
const uint64_t IRKEY_REPEAT = 0xFFFFFFFFFFFFFFFF;

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = D5;

IRrecv irrecv(kRecvPin);

decode_results results;

void flash(int times);

void setup()
{
  Serial.begin(9600);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  irrecv.enableIRIn(); // Start the receiver

  while (!Serial) // Wait for the serial connection to be establised.
  {
    delay(50);
  }

  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);
}

void loop()
{
  if (irrecv.decode(&results))
  {
    if (results.value == IRKEY_UP)
    {
      digitalWrite(LED2, ON);
      digitalWrite(LED1, OFF);
    }
    else if (results.value == IRKEY_DOWN)
    {
      digitalWrite(LED1, ON);
      digitalWrite(LED2, OFF);
    }
    else if (results.value == IRKEY_1)
    {
      flash(1);
    }
    else if (results.value == IRKEY_2)
    {
      flash(2);
    }
    else if (results.value == IRKEY_3)
    {
      flash(3);
    }
    else if (results.value == IRKEY_4)
    {
      flash(4);
    }
    else if (results.value == IRKEY_5)
    {
      flash(5);
    }
    else if (results.value == IRKEY_MEM)
    {
      digitalWrite(LED1, OFF);
      digitalWrite(LED2, OFF);
    }
    else if (results.value != IRKEY_REPEAT)
    {
      serialPrintUint64(results.value, HEX);
      Serial.println("");
    }

    irrecv.resume(); // Receive the next value
  }
  delay(100);
}

void flash(int times)
{
  Serial.printf("flash(%i)\n", times);
  for (int i = 0; i < times; i++)
  {
    digitalWrite(LED1, ON);
    digitalWrite(LED2, ON);
    delay(500);
    digitalWrite(LED1, OFF);
    digitalWrite(LED2, OFF);
    delay(500);
  }
}
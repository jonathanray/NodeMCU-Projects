#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define SPEAKER_PIN D2

// const uint64_t IRKEY_UP = 0x40BF00FF;
// const uint64_t IRKEY_DOWN = 0x40BF20DF;
// const uint64_t IRKEY_1 = 0x40BF609F;
// const uint64_t IRKEY_2 = 0x40BF50AF;
// const uint64_t IRKEY_3 = 0x40BF708F;
// const uint64_t IRKEY_4 = 0x40BF906F;
// const uint64_t IRKEY_5 = 0x40BFB04F;
// const uint64_t IRKEY_MEM = 0x40BF6897;
// const uint64_t IRKEY_REPEAT = 0xFFFFFFFFFFFFFFFF;


const uint64_t IRKEY_UP = 0x20DF40BF;
const uint64_t IRKEY_DOWN = 0x20DFC03F;
const uint64_t IRKEY_1 = 0x20DF8877;
const uint64_t IRKEY_2 = 0x20DF48B7;
const uint64_t IRKEY_3 = 0x20DFC837;
const uint64_t IRKEY_4 = 0x20DF28D7;
const uint64_t IRKEY_5 = 0x20DFA857;
const uint64_t IRKEY_MEM = 0x20DF08F7;
const uint64_t IRKEY_REPEAT = 0xFFFFFFFFFFFFFFFF;

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = D5;

IRrecv irrecv(kRecvPin);

decode_results results;

const uint32 smallStep = 10;
const uint32 bigStep = 100;
const uint32 minFreq = 100;
const uint32 maxFreq = bigStep * 6;

uint32 frequency = 0;
uint64_t lastKey = 0;

void setup()
{
  Serial.begin(9600);

  irrecv.enableIRIn(); // Start the receiver

  while (!Serial) // Wait for the serial connection to be establised.
  {
    delay(50);
  }

  Serial.println();
  Serial.print("Ready");
  Serial.println(kRecvPin);
}

void play()
{
  pinMode(SPEAKER_PIN, OUTPUT);
  analogWriteFreq(frequency);
  analogWrite(SPEAKER_PIN, 512);
  Serial.printf("Play %u hz\n", frequency);
}

void stop()
{
  analogWrite(SPEAKER_PIN, 0);
  pinMode(SPEAKER_PIN, INPUT);
  Serial.println("Stop");
}

void loop()
{
  if (irrecv.decode(&results))
  {
    uint64_t key = results.value;
    if (key == IRKEY_REPEAT && (lastKey == IRKEY_UP || lastKey == IRKEY_DOWN))
    {
      key = lastKey;
    }

    if (key == IRKEY_UP)
    {
      lastKey = key;
      if (frequency < maxFreq)
      {
        frequency += smallStep;
        play();
      }
    }
    else if (key == IRKEY_DOWN)
    {
      lastKey = key;
      if (frequency > minFreq)
      {
        frequency -= smallStep;
        play();
      }
    }
    else if (key == IRKEY_1)
    {
      lastKey = key;
      frequency = bigStep;
      play();
    }
    else if (key == IRKEY_2)
    {
      lastKey = key;
      frequency = bigStep * 2;
      play();
    }
    else if (key == IRKEY_3)
    {
      lastKey = key;
      frequency = bigStep * 3;
      play();
    }
    else if (key == IRKEY_4)
    {
      lastKey = key;
      frequency = bigStep * 4;
      play();
    }
    else if (key == IRKEY_5)
    {
      frequency = bigStep * 5;
      play();
    }
    else if (key == IRKEY_MEM)
    {
      stop();
    }
    else if (key != IRKEY_REPEAT)
    {
      Serial.print("Unrecognized IR Code: ");
      serialPrintUint64(results.value, HEX);
      Serial.println("");
    }

    if (key != IRKEY_REPEAT)
    {
      lastKey = key;
    }

    irrecv.resume(); // Receive the next value
  }
  delay(100);
}

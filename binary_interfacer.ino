#include <Arduino.h>

const int CLOCK_PIN = 5;
const int DATA_PIN = 4;
const int LATCH_PIN = 6;

void setup() {
  Serial.begin(9600);
	pinMode(DATA_PIN, INPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  digitalWrite(LATCH_PIN, 1);
  digitalWrite(CLOCK_PIN, 0);
}

void loop() {
  
  // Latch data
  digitalWrite(LATCH_PIN, 0);
  delayMicroseconds(20);
  digitalWrite(LATCH_PIN, 1);
  delayMicroseconds(20);
  
  // Read data
  uint16_t shift_num = shiftIn165(DATA_PIN, CLOCK_PIN, MSBFIRST, 16);
  Serial.println(shift_num);

  delay(100);
}

uint16_t shiftIn165(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, int bits) {
  uint16_t value = 0;

  for (uint16_t i = 0; i < bits; i++) {
    delayMicroseconds(20);
    digitalWrite(clockPin, LOW);
    if (bitOrder == LSBFIRST) {
      value |= digitalRead(dataPin) << i;
    }
    else {
      value |= digitalRead(dataPin) << ((bits - 1) - i);
    }
    delayMicroseconds(20);
    digitalWrite(clockPin, HIGH);
  }
  return value;
}
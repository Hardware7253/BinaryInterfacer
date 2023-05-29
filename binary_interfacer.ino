#include <Arduino.h>

const int CLOCK_PIN = 5;
const int DATA_PIN = 11;
const int LATCH_PIN = 12;

// Shifts given number into shift register
void ShiftOut(int dataPin, int clockPin, int bits, uint16_t num, bool msbfirst) {
    for (int i = 0; i < bits; i++) {
        
        if (!msbfirst) {
            digitalWrite(dataPin, BitOn(num, i)); 
        } else {
            digitalWrite(dataPin, BitOn(num, abs(i - (bits - 1)))); 
        }
        
        delayMicroseconds(1); // Data hold time

        // Pulse clock to shiftin data
        digitalWrite(clockPin, 1);
        delayMicroseconds(1);
        digitalWrite(clockPin, 0);
        delayMicroseconds(1);
    }
}

// Returns true if a bit is on in a number
bool BitOn(uint16_t num, int bit) {
    uint16_t num_from_bit = 1 << bit;
    if ((num ^ num_from_bit) < num) {
        return true;
    }
    return false;
}

void setup() {
  // Initialize the LCD
  Serial.begin(9600);

  // Initialize shift register pins
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);  

  digitalWrite(DATA_PIN, 0);
  digitalWrite(CLOCK_PIN, 0);
  digitalWrite(LATCH_PIN, 0);

  delay(1);

  // Shift data to shift register
  ShiftOut(DATA_PIN, CLOCK_PIN, 16, 9, true);

  delayMicroseconds(1); // Data hold time

  // Latch data
  digitalWrite(LATCH_PIN, 1);
  delayMicroseconds(1);
  digitalWrite(LATCH_PIN, 0);
  delayMicroseconds(1);
}

void loop() {
  
}
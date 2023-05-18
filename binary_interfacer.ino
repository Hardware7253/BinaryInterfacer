#include <Arduino.h>

// Include the libraries:
// LiquidCrystal_I2C.h: https://github.com/johnrickman/LiquidCrystal_I2C
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD

// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); // Change to (0x27,20,4) for 20x4 LCD.

const int CLOCK_PIN = 5;
const int DATA_PIN = 4;
const int LATCH_PIN = 6;

void setup() {
  // Initialize the LCD
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(DATA_PIN, INPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  digitalWrite(LATCH_PIN, 1);
  digitalWrite(CLOCK_PIN, 0);
  
}

// Displays number as binary on the character lcd
void DisplayNumAsBin(int num) {
  char binStr[16] = "0000000000000000";
  for (int i = 0; i < 16; i++) {
    if (BitOn(num, i)) {
      binStr[abs(i - 15)] = '1';
    }
  }
  lcd.print(binStr);
} 

// Returns true if a given bit is on in a number
bool BitOn(int num, int bit) {
  uint16_t num_from_bit = 1 << bit;
  if ((num ^ num_from_bit) < num) {
      return true;
  }
  return false;
}

uint16_t shiftIn165(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t bitOrder, int bits) {
  
  // Latch data
  digitalWrite(latchPin, 0);
  delayMicroseconds(20);
  digitalWrite(latchPin, 1);
  delayMicroseconds(20);
  
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

void loop() {
  
  // Get number from binary bus
  uint16_t busNum = shiftIn165(DATA_PIN, CLOCK_PIN, LATCH_PIN, MSBFIRST, 16);
  Serial.println(busNum);

  // Output bus number on the lcd
  lcd.setCursor(0, 0);
  DisplayNumAsBin(busNum); // Print binary number on first line of the character lcd
  lcd.setCursor(0, 1);
  lcd.print(busNum); // Print decimal number on the second line of the character lcd
  lcd.print("                "); // Clear old characters on the second line
  delay(100);
}
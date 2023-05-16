// Include the libraries:
// LiquidCrystal_I2C.h: https://github.com/johnrickman/LiquidCrystal_I2C
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD

// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); // Change to (0x27,20,4) for 20x4 LCD.

int displayNum = 42;

void setup() {
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
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
  Serial.println(binStr);
} 

// Returns true if a given bit is on in a number
bool BitOn(int num, int bit) {
  uint16_t num_from_bit = 1 << bit;
  if ((num ^ num_from_bit) < num) {
      return true;
  }
  return false;
}

void loop() {
  lcd.setCursor(0, 0);
  DisplayNumAsBin(displayNum);
  delay(100);
}
#include <Arduino.h>

// Include the libraries:
// LiquidCrystal_I2C.h: https://github.com/johnrickman/LiquidCrystal_I2C
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD

// Wiring: SDA pin is connected to A4 and SCL pin to A5.
// Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); // Change to (0x27,20,4) for 20x4 LCD

// Keyapd number            0,  1,  2,  3, 4,  5,  6, 7, 8, 9
const int KEYPAD_PINS[] = {A0, A1, A2, A3, 3, 12, 10, 9, 8, 7};
uint32_t keypadNum = 0;

// Shift register pins
const int SHARED_CLOCK_PIN = 5;
const int SHARED_LATCH_PIN = 6;
const int SHARED_DATA_PIN = 4;

// Mode select pin and mode variable
const int MODE_SELECT_PIN = 2;
bool readMode = true;


// Output enable pin for shift out register
const int OE_PIN = 11;

int lastKey = -1; // Last key pressed on the keypad
unsigned long lastKeypadTime = 0; // Time of last keypad button press
unsigned long lastModeSwitchTime = 0; // Time of last modeswitch button press
unsigned long debounceTime = 80; // Milliseconds between input changes

// Mode button ISR
// Reset keypad number
void modeInterrupt() {
  keypadNum = 0;
}

// Initialise an array of pins, as input or output
void InitPinArray(const int pinsArray[], int len, byte io) {
  for (int i = 0; i < len; i++) {
    pinMode(pinsArray[i], io);
  }
}


void setup() {
  Serial.begin(9600);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Initialise output enable pin
  pinMode(OE_PIN, OUTPUT);
  digitalWrite(OE_PIN, readMode);

  // Initialise keypad pins
  InitPinArray(KEYPAD_PINS, 10, INPUT_PULLUP);

  // Initialise shift in register pins
  // Don't initialise data pin as it is switched between input and output on the fly
  pinMode(SHARED_CLOCK_PIN, OUTPUT);
  pinMode(SHARED_LATCH_PIN, OUTPUT);
  digitalWrite(SHARED_CLOCK_PIN, 0);
  digitalWrite(SHARED_LATCH_PIN, 1);

  pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODE_SELECT_PIN), modeInterrupt, FALLING);
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

// Shifts in a binary number from a shift register (e.g. 74HC165)
uint16_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t bits, bool msbfirst) {
  pinMode(dataPin, INPUT);

  // Latch data
  digitalWrite(latchPin, 0);
  delayMicroseconds(1);
  digitalWrite(latchPin, 1);
  delayMicroseconds(1);

  // Data hold time
  delayMicroseconds(1);
  
  uint16_t value = 0;

  // Sequentially shift in a binary number by adding the value of 1 bit at a time
  for (uint16_t i = 0; i < bits; i++) {
    digitalWrite(clockPin, LOW);
    if (!msbfirst) {
      value |= digitalRead(dataPin) << i;
    }
    else {
      value |= digitalRead(dataPin) << ((bits - 1) - i);
    }
    delayMicroseconds(1);
    digitalWrite(clockPin, HIGH);
  }
  return value;
}

// Shifts given number into a shift register (e.g. 74HC595)
void ShiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t bits, uint16_t num, bool msbfirst) {
  pinMode(dataPin, OUTPUT);

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

  delayMicroseconds(1); // Data hold time

    // Pulse latch pin to latch data
    digitalWrite(latchPin, 1);
    delayMicroseconds(1);
    digitalWrite(latchPin, 0);
    delayMicroseconds(1);
}

// Returns true if a bit is on in a number
bool BitOn(uint16_t num, int bit) {
  uint16_t num_from_bit = 1 << bit;
  if ((num ^ num_from_bit) < num) {
      return true;
  }
  return false;
}

// Return value of the key being pressed
// So the keypad key 4 will return 4
// Return -1 if more than 1 key is being pressed
int ReadKeypad(const int pins[]) {

  int num = -1;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(pins[i]) == LOW) {
      if (num == -1) {
        num = i;
      } else {
        return -1;
      }
    }
  }

  return num;
}

// Updates number based off keypad inputs
uint32_t KeypadInputNum(const int keypadPins[], uint32_t num) {
  int key = ReadKeypad(keypadPins);

  // Only update num if a key has been pressed (-1 represents no keys)
  if (key != -1) {
    unsigned long buttonTime = millis();

    if (((buttonTime - lastKeypadTime) > debounceTime) || (key != lastKey)) {
      num *= 10;
      num += key;
    }

    lastKeypadTime = buttonTime;
  }

  lastKey = key;

  return num;
}

void loop() {

  uint32_t displayNum = 0;

  // When the mode select switch is pressed, change the mode
  if (!digitalRead(MODE_SELECT_PIN)) {
    if ((millis() - lastModeSwitchTime) > debounceTime) {
      readMode = !readMode;
      digitalWrite(OE_PIN, readMode);
    }

    lastModeSwitchTime = millis();
  }

  if (readMode) {
    
    // When in read mode the displayNum is read from the bus
    displayNum = shiftIn(SHARED_DATA_PIN, SHARED_CLOCK_PIN, SHARED_LATCH_PIN, 16, true);
  } else {

   // When in write mode the displayNum is read from the numpad, and outputted as binary on the bus
    keypadNum = KeypadInputNum(KEYPAD_PINS, keypadNum);

    ShiftOut(SHARED_DATA_PIN, SHARED_CLOCK_PIN, SHARED_LATCH_PIN, 16, keypadNum, true);

    displayNum = keypadNum;
  }

  // Reset the display and keypad numbers if they are greater than 16 bits
  if (displayNum > 65535) {
    displayNum = 0;
    keypadNum = 0;
  }
  
  // Output displayNum as decimal and binary on the lcd
  lcd.setCursor(0, 0);
  DisplayNumAsBin(displayNum); // Print binary number on first line of the character lcd
  lcd.setCursor(0, 1);
  lcd.print(displayNum); // Print decimal number on the second line of the character lcd
  lcd.print("                "); // Clear old characters on the second line
  lcd.setCursor (15, 1);

  // Display what mode the device is in (read or write mode)
  if (readMode) {
    lcd.print("R");
  } else {
    lcd.print("W");
  }

  // Print bus number to serial console
  Serial.print("Bus number: ");
  Serial.println(displayNum);
  
  
  delayMicroseconds(1);
}
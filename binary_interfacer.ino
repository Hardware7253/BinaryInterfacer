#include <Arduino.h>

// Keyapd number    00, 01, 02, 03, 04, 05, 06, 7, 8, 9
int keypadPins[] = {A0, A1, A2, A3, 3, 2, 10, 9, 8, 7};
uint16_t keypadNum = 0;

unsigned long lastButtonTime = 0; // Time of last button press
unsigned long buttonTime = 0; // time of most recent button press

// Initialise an array of pins, as input or output
void InitPinArray(int pinsArray[], int len, byte io) {
  for (int i = 0; i < len; i++) {
    pinMode(pinsArray[i], io);
  }
}

void setup() {
  
  Serial.begin(9600);

  InitPinArray(keypadPins, 10, INPUT_PULLUP);
}

// Return value of the key being pressed
// So the keypad key 4 will return 4
// Return -1 if more than 1 key is being pressed
int ReadKeypad(int pins[]) {

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
uint16_t KeypadInputNum(int keypadPins[], uint16_t num) {
  int key = ReadKeypad(keypadPins);

  // Only update num if a key has been pressed (-1 represents no keys)
  if (key != -1) {
    lastButtonTime = buttonTime;
    buttonTime = millis();

    // Only register the keypress if the last key was pressed greater than buttonDelayMs ago
    // buttonDelayMs must be greater than any delays in the for loop
    int buttonDelayMs = 10;

    if ((buttonTime - lastButtonTime) > buttonDelayMs) {
      num *= 10;
      num += key;
    }
  }
  return num;
}


void loop() {
  keypadNum = KeypadInputNum(keypadPins, keypadNum);
  Serial.println(keypadNum);
  delay(1);
}

#include <Keyboard.h>

// ZX81 USB Keyboard for Leonardo
// (c) Dave Curran
// 2013-04-27

// Modified with Function keys by Tony Smith
// 2014-02-15

// Modified for use with Osbone Computer Keyboard by Keith
// Added 8 by 8 matrix
// Changed DEBOUNCE vaule
// 2018-04-02


#define NUM_ROWS 8
#define NUM_COLS 8

// Keymap for normal use
byte keyMap[NUM_ROWS][NUM_COLS] = {
   //0   1   2   3   4   5   6   7
 {177,179,128,' ',129,176,178,'['}, //A0
 {' ',' ',',',193,' ',' ',' ',' '}, // A7
 {'1','2','3','4','5','6','7','8'}, //A1
 {'q','w','e','r','t','y','u','i'}, //A2
 {'a','s','d','f','g','h','j','k'}, //A3
 {'z','x','c','v','b','n','m',','}, //A4
 {215,217,'-','/',';','|','l','='}, //A6
 {218,216,'0',' ','.','p','o','9'}, //A5 
};

// Keymap if Shift is pressed
byte keyMapShifted[NUM_ROWS][NUM_COLS] = {
   //0   1   2   3   4   5   6   7
 {177,179,128,' ',129,176,212,'['}, //A0
 {' ',' ',',',193,' ',' ',' ',' '}, // A7
 {'!','@','#','$','%','^','&','*'}, //A1
 {'Q','W','E','R','R','Y','U','I'}, //A2
 {'A','S','D','F','G','H','J','K'}, //A3
 {'Z','X','C','V','B','N','M','<'}, //A4
 {215,217,'_','?',':','"','L','+'}, //A6
 {218,216,')',' ','>','P','O','('}, //A5 
};

// keymap if Shift is pressed plus New Line
// *Next* key read should be from this table
byte keyMapAlt[NUM_ROWS][NUM_COLS] = {
   //0   1   2   3   4   5   6   7
 {177,179,128,' ',129,176,212,'['}, //A0
 {' ',' ',',',193,' ',' ',' ',' '}, // A7
 {'!','@','#','$','%','^','&','*'}, //A1
 {'Q','W','E','R','R','Y','U','I'}, //A2
 {'A','S','D','F','G','H','J','K'}, //A3
 {'Z','X','C','V','B','N','M','<'}, //A4
 {215,217,'_','?',':','"','L','+'}, //A6
 {218,216,')',' ','>','P','O','('}, //A5 
};

// Global variables
int debounceCount[NUM_ROWS][NUM_COLS];
int altKeyFlag;
int ctrlKeyFlag;

// Define the row and column pins
byte colPins[NUM_COLS] = {8, 9, 10, 11, 12, 13, A0, A1};
byte rowPins[NUM_ROWS] = {A2, 1, 2, 3, 4, 5, 6, 7};

// Where is the Shift key
#define SHIFT_COL 4
#define SHIFT_ROW 0

// How many times does a key need to register as pressed?
#define DEBOUNCE_VALUE 100
#define REPEAT_DELAY 300

void setup() {
  // set all pins as inputs and activate pullups
  for (byte c = 0 ; c < NUM_COLS ; c++) {
    pinMode(colPins[c], INPUT);
    digitalWrite(colPins[c], HIGH);

    // Clear debounce counts
    for (byte r = 0 ; r < NUM_ROWS ; r++) {
      debounceCount[r][c] = 0;
    }
  }

  // Set all pins as inputs
  for (byte r = 0 ; r < NUM_ROWS ; r++) {
    pinMode(rowPins[r], INPUT);
  }

  // Zero Function modifier flag
  altKeyFlag = 0;
  ctrlKeyFlag = 0;

  // Initialize control over the keyboard
  // This is an Arduino Leonardo routine
  Keyboard.begin();
}

void loop() {
  // Check for Shift
  bool shifted = false;
  bool keyPressed = false;
  
  pinMode(rowPins[SHIFT_ROW], OUTPUT);

  if (digitalRead(colPins[SHIFT_COL]) == LOW) {
    shifted = true;
  }

  if (shifted == true && altKeyFlag == 255) {
    // NOP in order to prevent Function selection from auto-repeating
  } else {
    pinMode(rowPins[SHIFT_ROW], INPUT);

    // For each row
    for (byte r = 0 ; r < NUM_ROWS ; r++) {
      // Turn the row on
      pinMode(rowPins[r], OUTPUT);
      digitalWrite(rowPins[r], LOW);

      for (byte c = 0 ; c < NUM_COLS ; c++) {
        if (digitalRead(colPins[c]) == LOW) {
          // Increase the debounce count
          debounceCount[r][c]++;

          // Has the switch been pressed continually for long enough?
          int count = debounceCount[r][c];
          if (count == DEBOUNCE_VALUE) {
            // First press
            keyPressed = true;          
            pressKey(r, c, shifted);
          } else if (count > DEBOUNCE_VALUE) {
            // Check for repeats
            count -= DEBOUNCE_VALUE;
            if (count % REPEAT_DELAY == 0) {
              // Send repeat press
              keyPressed = true;          
              pressKey(r, c, shifted);
            }  
          }
        } else {
          // No key pressed, so reset debounce count
          debounceCount[r][c] = 0;
        }
      }

      // Turn the row back off
      pinMode(rowPins[r], INPUT);
    }

    digitalWrite(rowPins[SHIFT_ROW], LOW);
  }
}

// Send the keypress
void pressKey(byte r, byte c, bool shifted) {  
  byte key = shifted ? keyMapShifted[r][c] : keyMap[r][c];

  if (altKeyFlag == 255) {
    // Get the Alt key pressed after Function has been selected
    key = keyMapAlt[r][c];
    altKeyFlag = 0;
  }

  if (key == KEY_F5) {
    // If the Function key pressed (Shift + New Line)
    altKeyFlag = 255;
    key = 0;
    debounceCount[r][c] = 0;
  }
  
  if (key == KEY_F6) {
    ctrlKeyFlag = 255;
    key = 0;
    debounceCount[r][c] = 0;
  }

  if (key > 0) {
    // Send the key
    if (ctrlKeyFlag == 255) {
      ctrlKeyFlag = 0;
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(key);
      delay(100);
      Keyboard.releaseAll();
      return;
    }
    
    Keyboard.write(key);
  }
}

///
///
//

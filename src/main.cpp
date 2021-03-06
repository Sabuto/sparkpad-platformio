#include <Arduino.h>

#define PROTOTYPE_PCB

/*
 * Files to include
 */

#include "HID-Project.h"
#include <Keypad.h>

// Sparkpad firmware files
#include "oled.h"
#include "knob.h"
#include "leds.h"

/*
 * Definitions
 */

// Map keys for the buttons and knob here. For a list of available keys, check out:
// https://github.com/NicoHood/HID/blob/master/src/KeyboardLayouts/ImprovedKeylayouts.h

#define BUTTON_1 KEY_F13
#define BUTTON_2 KEY_F14
#define BUTTON_3 KEY_F15
#define BUTTON_4 KEY_F16
#define BUTTON_5 KEY_F17
#define BUTTON_6 KEY_F18
#define BUTTON_7 KEY_F19
#define BUTTON_8 KEY_F20
#define BUTTON_9 KEY_F21
#define BUTTON_10 KEY_RIGHT_SHIFT
#define BUTTON_11 KEY_F23
#define BUTTON_12 KEY_F24

#define KNOB_INCREASE MEDIA_VOLUME_UP
#define KNOB_DECREASE MEDIA_VOLUME_DOWN
#define KNOB_BUTTON MEDIA_VOLUME_MUTE

// Choose your OS here (Comment out incorrect OS)
#define OS_WINDOWS10
//#define OS_UBUNTU

#ifdef OS_WINDOWS10
#define VOLUME_RANGE 50
#endif

#ifdef OS_UBUNTU
#define VOLUME_RANGE 18
#endif

/*
 * Firmware beigns here
 */

// Setup the physical button layout

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'0', '3', '6', '9'},
  {'1', '4', '7', ':'},
  {'2', '5', '8', ';'},
  {'A', 'B', 'C', 'D'}
};

#ifdef PROTOTYPE_PCB
byte rowPins[ROWS] = {20, 16, 10, 14};
byte colPins[COLS] = {19, 18, 15, 21};
#else
byte rowPins[ROWS] = {5, 15, 9, 8};
byte colPins[COLS] = {6, 7, 18, 4};
#endif

Keypad keypad = Keypad(makeKeymap(keys), colPins, rowPins, ROWS, COLS);

long previous_bar_value;

void knob_button()
{
  Consumer.press(KNOB_BUTTON);
  if(bar_value != 0) {
    previous_bar_value = bar_value;
    bar_value = 0;
  } else {
    bar_value = previous_bar_value;
  }

  float translated = bar_value * (10.0 / VOLUME_RANGE);
  update_bar(round(translated));
}

// Add event listener to we can prossess button presses

void KeyEventListener(KeypadEvent key, KeyState kpadState) {

  if(kpadState == PRESSED) {

    switch(key) {

      case '0':
        Keyboard.press(BUTTON_1);
        break;
      case '1':
        Keyboard.press(BUTTON_2);
        break;
      case '2':
        Keyboard.press(BUTTON_3);
        break;
      case '3':
        Keyboard.press(BUTTON_4);
        break;
      case '4':
        Keyboard.press(BUTTON_5);
        break;
      case '5':
        Keyboard.press(BUTTON_6);
        break;
      case '6':
        Keyboard.press(BUTTON_7);
        break;
      case '7':
        Keyboard.press(BUTTON_8);
        break;
      case '8':
        Keyboard.press(BUTTON_9);
        break;
      case '9':
        Keyboard.press(BUTTON_10);
        break;
      case ':':
        Keyboard.press(BUTTON_11);
        break;
      case ';':
        Keyboard.press(BUTTON_12);
        break;
      case 'A':
        knob_button();
        break;
      case 'B':
        nav.doNav(downCmd);
        nav.doOutput();
        break;
      case 'C':
        nav.doNav(enterCmd);
        nav.doOutput();
        break;
      case 'D':
        nav.doNav(upCmd);
        nav.doOutput();
        break;
      default:
        break;
    }
  } else if (kpadState == RELEASED) {
    switch (key) {

      case '0':
        Keyboard.release(BUTTON_1);
        break;
      case '1':
        Keyboard.release(BUTTON_2);
        break;
      case '2':
        Keyboard.release(BUTTON_3);
        break;
      case '3':
        Keyboard.release(BUTTON_4);
        break;
      case '4':
        Keyboard.release(BUTTON_5);
        break;
      case '5':
        Keyboard.release(BUTTON_6);
        break;
      case '6':
        Keyboard.release(BUTTON_7);
        break;
      case '7':
        Keyboard.release(BUTTON_8);
        break;
      case '8':
        Keyboard.release(BUTTON_9);
        break;
      case '9':
        Keyboard.release(BUTTON_10);
        break;
      case ':':
        Keyboard.release(BUTTON_11);
        break;
      case ';':
        Keyboard.release(BUTTON_12);
        break;
      case 'A':
        Consumer.release(KNOB_BUTTON);
        break;
      default:
        break;
    }
  }
}

void update_leds() {
  setupDisplay(true, ledBrightness);
  update_all_leds(ledColour);
}

void setup()
{
  //EEPROM settings
  ledColour = EEPROM.read(ledColourAddress);
  ledBrightness = EEPROM.read(ledBrightnessAddress);
  led_colour_current = ledColour;
  led_brightness_current = ledBrightness;
  bar_value = EEPROM.read(bar_address);

  // keyboard
  Keyboard.begin();
  keypad.addEventListener(KeyEventListener);
  Consumer.begin();

  // Serial output
  // Serial.begin(9600);
  
  // while(!Serial);

  //OLED
  Wire.begin();
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(menuFont);
  oled.clear();

  // If commeneted out, screen initialises blank
  // nav.doOutput();

  // LEDS
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(strobePin, OUTPUT);

  digitalWrite(strobePin, HIGH);
  digitalWrite(clockPin, HIGH);

  update_leds();

  // update the led bar
  float translated = bar_value * (10.0 / VOLUME_RANGE);
  update_bar(round(translated));
}

void loop() {
  keypad.getKeys();

  long knob_value_new = knob.read();

  if(knob_value_new != knob_value) {
    if(knob_value_new > knob_value) Consumer.write(KNOB_INCREASE);
    else Consumer.write(KNOB_DECREASE);

    bar_value = min(max(0, bar_value + (knob_value_new - knob_value)), VOLUME_RANGE);
    knob_value = knob_value_new;
    float translated = bar_value * (10.0 / VOLUME_RANGE);
    update_bar(round(translated));
  }

  if(led_colour_current != ledColour) {
    led_colour_current = ledColour;
    update_leds();
  }

  if(led_brightness_current != ledBrightness) {
    led_brightness_current = ledBrightness;
    update_leds();
  }
}
// Reacts to sounds using LEDs

#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS_1 800
#define NUM_LEDS_2 800
#define NUM_LEDS_3 800

// Usable pins:
//   Teensy LC:   1, 4, 5, 24
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
//   Teensy 3.5:  1, 5, 8, 10, 26, 32, 33, 48
//   Teensy 3.6:  1, 5, 8, 10, 26, 32, 33
//   Teensy 4.0:  1, 8, 14, 17, 20, 24, 29, 39
//   Teensy 4.1:  1, 8, 14, 17, 20, 24, 29, 35, 47, 53

#define DATA_PIN_1 1
#define DATA_PIN_2 8
#define DATA_PIN_3 14

#define BRIGHTNESS 100

int Front_Left_Bottom = 0;
int Rear_Left_Bottom = 600;
CRGB leds_1[NUM_LEDS_1]; // Left wall

int Front_Right_Bottom = 340;
CRGB leds_2[NUM_LEDS_2]; // Center wall

int Rear_Right_Bottom = 506;
CRGB leds_3[NUM_LEDS_3]; // Right / Back wall

typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 100; // rotating "base color" used by many of the patterns

void setup() {
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_1, BGR>(leds_1, NUM_LEDS_1);
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_2, BGR>(leds_2, NUM_LEDS_2);
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_3, BGR>(leds_3, NUM_LEDS_3);
  LEDS.setBrightness(BRIGHTNESS);
}

void loop() {
  for (int i = 0; i < 800; ++i) {
    leds_1[i] = CHSV(gHue, 255, 192);
    leds_2[i] = CHSV(gHue, 255, 192);
    leds_3[i] = CHSV(gHue, 255, 192);
  }
  ++gHue;
  FastLED.show();
}

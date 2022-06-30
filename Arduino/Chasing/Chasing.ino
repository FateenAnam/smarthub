#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS_1 600
#define NUM_LEDS_2 600
#define NUM_LEDS_3 600
#define DATA_PIN_1 5
#define DATA_PIN_2 6
#define DATA_PIN_3 7
#define BRIGHTNESS 100
CRGB leds_1[NUM_LEDS_1];
CRGB leds_2[NUM_LEDS_2];
CRGB leds_3[NUM_LEDS_3];
int ledOn = NUM_LEDS_1;

void setup() {
  Serial.begin(9600); // open the serial port at 9600 bps:
  FastLED.addLeds<NEOPIXEL, DATA_PIN_1>(leds_1, NUM_LEDS_1);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_2>(leds_2, NUM_LEDS_2);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_3>(leds_3, NUM_LEDS_3);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  allWhite();
}

void allWhite() {
  // LED Strip #1
  for (int i = 0; i < NUM_LEDS_1; i += 5) {
    leds_1[i] = CRGB::White;
    FastLED.show();
    leds_1[i] = CRGB::Black;
  }

  // LED Strip #2
  for (int i = 0; i < NUM_LEDS_2; i += 5) {
    leds_2[i] = CRGB::White;
    FastLED.show();
    leds_2[i] = CRGB::Black;
  }

  // LED Strip #3
  for (int i = 0; i < NUM_LEDS_3; i += 5) {
    leds_3[i] = CRGB::White;
    FastLED.show();
    leds_3[i] = CRGB::Black;
  }
}

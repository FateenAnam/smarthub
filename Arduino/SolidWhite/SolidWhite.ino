#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS_1 600
//#define NUM_LEDS_2 600
//#define NUM_LEDS_3 600
#define DATA_PIN_1 2
#define DATA_PIN_2 6
#define DATA_PIN_3 7
#define BRIGHTNESS 50
CRGB leds_1[NUM_LEDS_1];
//CRGB leds_2[NUM_LEDS_2];
//CRGB leds_3[NUM_LEDS_3];
int ledOn = NUM_LEDS_1;

void setup() {
  Serial.begin(9600); // open the serial port at 9600 bps:
  FastLED.addLeds<NEOPIXEL, DATA_PIN_1>(leds_1, NUM_LEDS_1);
//  FastLED.addLeds<NEOPIXEL, DATA_PIN_2>(leds_2, NUM_LEDS_2);
//  FastLED.addLeds<NEOPIXEL, DATA_PIN_3>(leds_3, NUM_LEDS_3);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  testRGB();
}

void allWhite() {
   // LED Strip #1
  for (int i = 0; i < NUM_LEDS_1; ++i) {
    leds_1[i] = CRGB::White;
  }
  FastLED.show();
}

void testRGB() {
  // LED Strip #1
  for (int i = 0; i < NUM_LEDS_1; ++i) {
    leds_1[i] = CRGB::Red;
  }
  FastLED.show();
  delay(1000);

  // LED Strip #2
  for (int i = 0; i < NUM_LEDS_1; ++i) {
    leds_1[i] = CRGB::Green;
  }
  FastLED.show();
  delay(1000);

  // LED Strip #3
  for (int i = 0; i < NUM_LEDS_1; ++i) {
    leds_1[i] = CRGB::Blue;
  }
  FastLED.show();
  delay(1000);
}

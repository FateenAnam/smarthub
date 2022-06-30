#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS_1 600
#define NUM_LEDS_2 600
#define DATA_PIN_1 5
#define DATA_PIN_2 6
#define BRIGHTNESS 100
CRGB leds_1[NUM_LEDS_1];
CRGB leds_2[NUM_LEDS_2];
int ledOn = NUM_LEDS_1;

int testSwitch = 1;

void setup() {
  Serial.begin(9600); // open the serial port at 9600 bps:
  FastLED.addLeds<NEOPIXEL, DATA_PIN_1>(leds_1, NUM_LEDS_1);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_2>(leds_2, NUM_LEDS_2);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  // Call the reactive to sounds function
  //  reactive(leds_1, NUM_LEDS_1);
  //  reactive(leds_2, NUM_LEDS_2);
  //  delay(100);
  white1();
}

void white1() {
  for (int i = 0; i < NUM_LEDS_1; ++i) {
    leds_1[i] = CRGB::White;
  }
  for (int i = 0; i < NUM_LEDS_2; ++i) {
    leds_2[i] = CRGB::White;
  }
   FastLED.show();
}

//**********************************************************************
// Helper method - reactive()
// Reacts to sounds
//**********************************************************************
void reactive(CRGB& leds, int NUM_LEDS) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CRGB::Black;
  }

  for (int i = 0; i < NUM_LEDS; ++i) {
    //    if (testSwitch == 1) {
    //      leds[i] = CRGB::White; // set to full white!
    //    }
    //    else {
    //      leds[i] = CRGB::Black;
    //    }
    leds[i] = CRGB::White; // set to full white!
  }

  //  if (testSwitch == 1) {
  //    testSwitch = 2;
  //  }
  //  else {
  //    testSwitch = 1;
  //  }

  FastLED.show();
}

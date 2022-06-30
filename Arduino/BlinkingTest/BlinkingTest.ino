#include "FastLED.h"

CRGB leds[1];
void setup() {
  FastLED.addLeds<NEOPIXEL, 3>(leds, 1);
}
void loop() {
  leds[0] = CRGB::Red; FastLED.show(); delay(30);
  leds[0] = CRGB::Black; FastLED.show(); delay(30);
}

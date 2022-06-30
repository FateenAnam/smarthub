#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 300
#define DATA_PIN 3
#define CLOCK_PIN 13
#define BRIGHTNESS          1
#define FRAMES_PER_SECOND  120
#define MIDDLE_LED 150
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
bool beat = false;
CRGB leds[NUM_LEDS];
int sensorPin = A5;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor
int ledOn = 0 ;
unsigned int globL = 0;
unsigned int globR = 0;
int ti = 0;
uint32_t timebase = 0;
uint32_t phase = 49151;
bool finished = true;
bool running = false;
int blueToothVal = 0;           //value sent over via bluetooth
int Mode = 2;    //stores mode
int Col = 6;    //stores Color
int trueCol ;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

}

typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 100; // rotating "base color" used by many of the patterns

void loop() {

  //**********************************************************************
  //read value from sensor
  unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;


  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(5);
    if (sample < 1024)  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  double volts = (peakToPeak * 5.0) / 1024;  // convert to volts


  //**********************************************************************
  //read bluetooth value
  if (Serial.available()) {
    blueToothVal = Serial.read() - 48; //read it
    if (not(blueToothVal == 1) and blueToothVal <= 5 ) {
      Mode = blueToothVal;
    }
    if (not(blueToothVal == 1) and blueToothVal >= 6 ) {
      Col = blueToothVal;
    }
  }



  //***************************************************************************



  //***************************************************************************
  // number of leds on
  ledOn = ceil(volts * 40);
  //Serial.println(ledOn);
  //Turns all leds off
  for (int i = 0; i < (NUM_LEDS); i++) {
    leds[i] = CRGB::Black;
  }

  //Sets correct LEDs on
  // for (int i = 0; i < (ledOn + 1); i++){
  // fill_rainbow( leds, ledOn, gHue, 7);
  //  }
  if ((blueToothVal == 1) and running == false) {
    on();
    running = true;
  }

  if (blueToothVal == 2 and running == true) {
    off();
    running = false;
  }


  if (running == true) {
//    if (Mode == 2) {
//      //  reactive();
//    }
    if (Mode == 3) {
      //Sparkle(random(255),random(255),random(255),0);
      // RunningLights(random(255),random(255),random(255),5);
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV( gHue, 255, 192);
      }
      FastLED.show();
    }
    if (Mode == 4) {
      juggle();
    }
    if (Mode == 5) {
      rainbow();
    }

    if (Col == 9) {
      if (ledOn > 70 ) {
        gHue = gHue + 51;

      }
      EVERY_N_MILLISECONDS( 20 ) {
        gHue++;
      }
    }

    if (Col == 6) {
      EVERY_N_MILLISECONDS( 500 ) {
        gHue++;
      }
    }
    FastLED.show();
  }

  FastLED.show();
  blueToothVal = 0;
}
//*****************************************************************


void reactive() {
  fill_rainbow( leds, ledOn, gHue, 3);
  fill_rainbow( leds - ledOn + NUM_LEDS, ledOn, gHue, 3);


  for (int i = 0; i < (29); i++) {
    leds[MIDDLE_LED + i] = CHSV( gHue, 255, 192) ;
    leds[MIDDLE_LED - i] = CHSV( gHue, 255, 192) ;
  }
  FastLED.show();
}

void sinelonL() {

  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = globL;

  leds[pos] = gHue;
  leds[pos - 1] = gHue;
  leds[pos + 1] = gHue;
}
void sinelonR() {

  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = globR;

  leds[pos] = gHue;
  leds[pos - 1] = gHue;
  leds[pos + 1] = gHue;
}

void pulse() {
  sinelonR();
  sinelonL();
}

void on() {
  gHue = 0;
  for (int i = 0; i < (NUM_LEDS); i++) {
    leds[i] = CRGB::Black ;
  }

  FastLED.show();
  for (int i = 0; i < (NUM_LEDS); i++) {
    leds[i] = CRGB::Red + i  ;
    delay(1);
    FastLED.show();
  }


  for (int i = 0; i < (ceil(NUM_LEDS / 2 + ledOn)); i++) {
    leds[MIDDLE_LED + i] = CRGB::Black ;
    leds[MIDDLE_LED - i] = CRGB::Black ;
    delay(2);
    FastLED.show();

  }
  for (int i = 0; i < (29); i++) {
    leds[MIDDLE_LED + i] = CHSV( gHue, 255, 192) ;
    leds[MIDDLE_LED - i] = CHSV( gHue, 255, 192) ;
    delay(5);
    FastLED.show();
  }
  Mode = 2;
  Col = 6;


}

void off() {

  for (int i = 0; i < (NUM_LEDS); i++) {
    leds[i] = CRGB::Red + i  ;
  }
  delay(100);
  FastLED.show();
  for (int i = 0; i < (NUM_LEDS); i++) {
    leds[NUM_LEDS - i] = CRGB::Black ;
    delay(1);
    FastLED.show();

  }
  delay(100);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}


void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}


void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

//*****************************************************************
void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[MIDDLE_LED];
  int cooldown;

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < MIDDLE_LED; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);

    if (cooldown > heat[i]) {
      heat[i] = 0;
    } else {
      heat[i] = heat[i] - cooldown;
    }
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = MIDDLE_LED - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if ( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160, 255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for ( int j = 0; j < MIDDLE_LED; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  showStrip();
  delay(SpeedDelay);
}


void Sparkle(byte red, byte green, byte blue, int SpeedDelay) {
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel, red, green, blue);
  showStrip();
  delay(SpeedDelay);
  setPixel(Pixel, 0, 0, 0);

}


void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if ( t192 > 0x80) {                    // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if ( t192 > 0x40 ) {            // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}

void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position = 0;

  for (int j = 0; j < NUM_LEDS * 2; j++)
  {
    Position++; // = 0; //Position + Rate;
    for (int i = 0; i < NUM_LEDS; i++) {
      // sine wave, 3 offset waves make a rainbow!
      //float level = sin(i+Position) * 127 + 128;
      //setPixel(i,level,0,0);
      //float level = sin(i+Position) * 127 + 128;
      setPixel(i, ((sin(i + Position) * 127 + 128) / 255)*red,
               ((sin(i + Position) * 127 + 128) / 255)*green,
               ((sin(i + Position) * 127 + 128) / 255)*blue);
    }

    showStrip();
    delay(WaveDelay);
    if (not Mode == 3) {
      break;
    }
  }
}

void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for (j = 0; j < 16; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < 4; i++) {
      c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      setPixel(i, *c, *(c + 1), *(c + 2));
      if (not Mode == 5) {
        break;
      }
    }
    showStrip();
    delay(SpeedDelay);
    if (not Mode == 5) {
      break;
    }
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];

  if (WheelPos < 85) {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  } else {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }

  return c;

}
//************************
void showStrip() {
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.show();
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  FastLED.show();
#endif
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
#ifdef ADAFRUIT_NEOPIXEL_H
  // NeoPixel
  strip.setPixelColor(Pixel, strip.Color(red, green, blue));
#endif
#ifndef ADAFRUIT_NEOPIXEL_H
  // FastLED
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
#endif
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < MIDDLE_LED; i++ ) {
    setPixel(i, red, green, blue);
  }
  showStrip();
}

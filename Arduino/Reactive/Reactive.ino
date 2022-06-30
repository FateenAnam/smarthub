// Reacts to sounds using LEDs
#include <ArduinoQueue.h>
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
int Front_Left_TopL = 163;
int Rear_Left_Bottom = 600;
int Rear_Left_TopL = Rear_Left_Bottom - 163 + 67; // 504
int Rear_Left_TopR = 668;
CRGB leds_1[NUM_LEDS_1]; // Left wall

int Front_Left_TopR = 0;
int Front_Right_Bottom = 340;
int Front_Right_TopL = 340 - 163;
CRGB leds_2[NUM_LEDS_2]; // Center wall

int Front_Right_TopR = 0;
int Rear_Right_Bottom = 506;
int Rear_Right_Num = 149;
int Rear_Right_TopR = 506 - Rear_Right_Num; // 357
int Rear_Right_TopL = 507;
CRGB leds_3[NUM_LEDS_3]; // Right / Back wall

int ledNewOn = NUM_LEDS_1; // How many LEDS to turn on
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
int microphonePin = A8;    // select the input pin for the potentiometer
unsigned int sample;

uint32_t lastColorChange;

typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 100; // rotating "base color" used by many of the patterns

void reactive();
void movingRainbow();
void calibrateStripPosition(CRGB* start);

/**
 * @class VirtualStrip
 * @brief Allows the user to group the strips into their logical locations. For example,
 * the front-left bar can have its own strip
 */
class VirtualStrip {
  public:
  VirtualStrip(CRGB* strip, uint16_t begin, uint16_t numLeds, bool reversed = false) 
      : stripPtr(new CRGB*[numLeds]), // Dynamically allocate array
        numLeds(numLeds),
        reversed(reversed)
      {
        if (!reversed) {
          for (int i = 0; i < numLeds; ++i) {
            stripPtr[i] = &(strip[begin + i]);
          }
        }
        else {
          for (int i = 0; i < numLeds; ++i) {
            stripPtr[i] = &(strip[begin + numLeds - i]);
          }
        }
  }

  ~VirtualStrip() {
    delete stripPtr;
  }

  void appendStrip(VirtualStrip& newStrip) {
    uint16_t newLength = numLeds + newStrip.getLength();
    CRGB** newStripPtr = new CRGB*[newLength];

    // Transfer original array
    for (int i = 0; i < numLeds; ++i) {
      newStripPtr[i] = stripPtr[i];
    }
    // Add new strip
    for (int i = numLeds; i < newLength; ++i) {
      newStripPtr[i] = &(newStrip[i - numLeds]);
    }

    numLeds = newLength;

    CRGB** tmp = stripPtr;
    stripPtr = newStripPtr;
    delete tmp;
  }

  CRGB& operator[](int index) {
    // if (index >= numLeds) {
    //   return *(stripPtr[numLeds]);
    // }

    // Index into array based on if it is reversed or not
    return *(stripPtr[index]);
  }

  uint16_t getLength() {
    return numLeds;
  }

  private:
    CRGB** stripPtr;
    uint16_t numLeds;
    bool reversed;
};

// Define volume bars
VirtualStrip frontLeftStrip(leds_1, Front_Left_Bottom, Front_Left_TopL - Front_Left_Bottom);
VirtualStrip frontRightStrip(leds_2, Front_Right_TopL, Front_Right_Bottom - Front_Right_TopL, true);
VirtualStrip rearLeftStrip(leds_1, Rear_Left_TopL, Rear_Left_Bottom - Rear_Left_TopL, true);
VirtualStrip rearRightStrip(leds_3, Rear_Right_Bottom, Rear_Right_Num - Rear_Right_Bottom, true);

// Define ceiling bars
VirtualStrip rightCeilingStrip(leds_3, Front_Right_TopR, Rear_Right_TopR - Front_Right_TopR);
VirtualStrip rearCeilingStrip(leds_3, Rear_Right_TopL, Rear_Left_TopR - Rear_Right_TopL);
VirtualStrip leftCeilingStrip(leds_1, Front_Left_TopL, Rear_Left_TopL - Front_Left_TopL, true);
// VirtualStrip leftCeilingStrip(leds_1, 163, 300, false);
VirtualStrip frontCeilingStrip(leds_2, Front_Left_TopR, Front_Right_TopL - Front_Left_TopR);

// Master ceiling
VirtualStrip ceilingStrip(leds_2, Front_Left_TopR, Front_Right_TopL - Front_Left_TopR);

void setup() {
  Serial.begin(9600); // open the serial port at 9600 bps:
  // Native strips
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_1, BGR>(leds_1, NUM_LEDS_1);
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_2, BGR>(leds_2, NUM_LEDS_2);
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_3, BGR>(leds_3, NUM_LEDS_3);

  // LEDS.addLeds<WS2812SERIAL, DATA_PIN_3, BGR>(rearRightStrip, 149);
  LEDS.setBrightness(BRIGHTNESS);

  ceilingStrip.appendStrip(rightCeilingStrip);
  ceilingStrip.appendStrip(rearCeilingStrip);
  ceilingStrip.appendStrip(leftCeilingStrip);
}



CRGB* test[800];

void loop() {
  // for (int i = 0; i < 600; ++i) {
  //   test[i] = &(leds_1[i]);
  // }
  // for (int i = 0; i < rearRightStrip.getLength(); ++i) {
  //   *(test[i]) = CRGB::White;
  // }
  // reactive();
  // movingRainbow();  

  // fill_rainbow(leds_1, 600, 100);
  // for (int i = 0; i < NUM_LEDS_1; ++i) {
  //   leds_1[i] = CRGB::Black;
  //   leds_2[i] = CRGB::Black;
  //   leds_3[i] = CRGB::Black;
  // }

  for (int i = 0; i < ceilingStrip.getLength(); ++i) {
    ceilingStrip[i] = CRGB::White;
    FastLED.show();
    delay(1);
  }

  // for (int i = 0; i < rightCeilingStrip.getLength(); ++i) {
  //   rightCeilingStrip[i] = CRGB::White;
  //   FastLED.show();
  //   delay(1);
  // }

  // for (int i = 0; i < rearCeilingStrip.getLength(); ++i) {
  //   rearCeilingStrip[i] = CRGB::White;
  //   FastLED.show();
  //   delay(1);
  // }

  // for (int i = 0; i < leftCeilingStrip.getLength(); ++i) {
  //   leftCeilingStrip[i] = CRGB::White;
  //   FastLED.show();
  //   delay(1);
  // }
  
  
  // calibrateStripPosition(leds_3 + Front_Right_TopL);
}

//**********************************************************************
// Helper method - calibrateStripPosition()
// Finds the position of a strip
//**********************************************************************
void calibrateStripPosition(CRGB* start) {
  fill_rainbow(leds_3 + 660, 20, 100, 50);
  FastLED.show();
}

//**********************************************************************
// Helper method - movingRainbow()
// Reacts to sounds
//**********************************************************************
void movingRainbow() {
  // for (int i = 0; i < 800; ++i) {
  //   leds_1[i] = CHSV(gHue + i*5, 255, 192);
  //   leds_2[i] = CHSV(gHue + i*5, 255, 192);
  //   leds_3[i] = CHSV(gHue + i*5, 255, 192);
  // }
  fill_rainbow(leds_1, NUM_LEDS_1, gHue, 1);
  fill_rainbow(leds_2, NUM_LEDS_2, gHue, 1);
  fill_rainbow(leds_3, NUM_LEDS_3, gHue, 1);
  gHue += 1;
  FastLED.show();
}

//**********************************************************************
// Helper method - reactive()
// Reacts to sounds
//**********************************************************************
void reactive() {
    //  **********************************************************************
  //   Read from microphone
  //  **********************************************************************
  unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for sampleWindow mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(microphonePin);
    //    Serial.println(analogRead(microphonePin));
    if (sample < 1024)  // Sanitize input
    {
      if (sample > signalMax) // Save just the max levels
      {
        signalMax = sample;
      }
      else if (sample < signalMin) // Save just the min levels
      {
        signalMin = sample;
      }
    }
  }

  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude

  double volts = (peakToPeak * 5.0) / 1024;  // convert to volts

  ledNewOn = ceil(volts * 37.5) + 50;

  // Cut-off at NUM_LEDS
  if (ledNewOn > NUM_LEDS_1) {
    ledNewOn = NUM_LEDS_1;
  }

  for (int i = 0; i < NUM_LEDS_1; ++i) {
    leds_1[i] = CRGB::Black;
    leds_2[i] = CRGB::Black;
    leds_3[i] = CRGB::Black;
  }

  if (ledNewOn > 163) {
    ledNewOn = 163;
  }

   for (int i = 0; i < ledNewOn; ++i) {
     leds_1[Front_Left_Bottom + i] = CHSV(i + gHue, 255, 192);
     leds_2[Front_Right_Bottom - i] = CHSV(i + gHue, 255, 192);
     leds_3[min(Rear_Right_Bottom, Rear_Right_Bottom - i + 14)] = CHSV(i + gHue, 255, 192);
     leds_1[Rear_Left_Bottom - i + 67] = CHSV(i + gHue, 255, 192);
   }

  // Front Rainbow Bar
  for (int i = 0; i < (Front_Right_Bottom - 163); ++i) {
    leds_2[i] = CHSV(gHue, 255, 192);
  }

  // Left Rainbow Bar
  for (int i = Front_Left_TopR; i < Rear_Left_TopL; ++i) {
    leds_1[i] = CHSV(gHue, 255, 192);
  }

  // Right Rainbow Bar
  for (int i = 0; i < Rear_Right_TopR; ++i) {
    leds_3[i] = CHSV(gHue, 255, 192);
  }

    // Back Rainbow Bar
  for (int i = Rear_Right_TopL; i < 750; ++i) {
    leds_3[i] = CHSV(gHue, 255, 192);
  }

  // ledOldOn = ledNewOn;
  gHue = gHue + 1;

  Serial.println(millis() - lastColorChange);

  // Color shift on bass drop
  if (millis() - lastColorChange > 100 && ledNewOn > 120) {
    lastColorChange = millis();
    gHue += 100;
  }

  FastLED.show();
}

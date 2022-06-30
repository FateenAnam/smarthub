// Reacts to sounds using LEDs

#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>
#include <ArduinoQueue.h>
#include "VirtualStrip.h"

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

#define BRIGHTNESS 255

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

//chasing variables
//front side
int ledFrontR = 0; //right side index
int ledFrontL = 0; //left side index
ArduinoQueue<int> chaseIndexFrontR(1000); //Q to go right
ArduinoQueue<int> chaseIndexFrontL(1000); //Q to go left

//backside
int ledBackR = 0; //right side index
int ledBackL = 0; //left side index
ArduinoQueue<int> chaseIndexBackR(1000); //Q to go right
ArduinoQueue<int> chaseIndexBackL(1000); //Q to go left

uint32_t lastColorChange;

typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 100; // rotating "base color" used by many of the patterns

void solid(CRGB color);
void on();
void reactive();
void movingRainbow();
void calibrateStripPosition(CRGB* start);
void sexy();
void study();
void sky();
void flicker();
void physical();
void virtualShow();

// Define volume bars
VirtualStrip frontLeftStrip(leds_1, Front_Left_Bottom, Front_Left_TopL - Front_Left_Bottom);
VirtualStrip frontRightStrip(leds_2, Front_Right_TopL, Front_Right_Bottom - Front_Right_TopL, true);
VirtualStrip rearLeftStrip(leds_1, Rear_Left_TopL, Rear_Left_Bottom - Rear_Left_TopL, true);
VirtualStrip rearRightStrip(leds_3, Rear_Right_TopR, Rear_Right_Num, true);

// Define ceiling bars
VirtualStrip rightCeilingStrip(leds_3, Front_Right_TopR, Rear_Right_TopR - Front_Right_TopR);
VirtualStrip rearCeilingStrip(leds_3, Rear_Right_TopL, Rear_Left_TopR - Rear_Right_TopL);
VirtualStrip leftCeilingStrip(leds_1, Front_Left_TopL, Rear_Left_TopL - Front_Left_TopL, true);
VirtualStrip frontCeilingStrip(leds_2, Front_Left_TopR, Front_Right_TopL - Front_Left_TopR);

// Master ceiling
VirtualStrip ceilingStrip;

// Master Strip
VirtualStrip masterStrip;

void setup() {
  // Power-up safety precauation
  delay(1000);

  // Open the serial port
  Serial.begin(9600);
  
  // Add native strips and set brightness
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_1, BGR>(leds_1, NUM_LEDS_1).setCorrection(TypicalLEDStrip);
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_2, BGR>(leds_2, NUM_LEDS_2).setCorrection(TypicalLEDStrip);
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_3, BGR>(leds_3, NUM_LEDS_3).setCorrection(TypicalLEDStrip);

  // LEDS.addLeds<WS2812SERIAL, DATA_PIN_1, BGR>(leds_1, NUM_LEDS_1);
  // LEDS.addLeds<WS2812SERIAL, DATA_PIN_2, BGR>(leds_2, NUM_LEDS_2);
  // LEDS.addLeds<WS2812SERIAL, DATA_PIN_3, BGR>(leds_3, NUM_LEDS_3);

  LEDS.setBrightness(BRIGHTNESS);

  // Set-up the ceiling strip
  ceilingStrip.appendStrip(frontCeilingStrip);
  ceilingStrip.appendStrip(rightCeilingStrip);
  ceilingStrip.appendStrip(rearCeilingStrip);
  ceilingStrip.appendStrip(leftCeilingStrip);

  // Set-up the master strip
  masterStrip.appendStrip(ceilingStrip);
  masterStrip.appendStrip(frontLeftStrip);
  masterStrip.appendStrip(frontRightStrip);
  masterStrip.appendStrip(rearLeftStrip);
  masterStrip.appendStrip(rearRightStrip);

  // Call on animation
  on();
}

void loop() {
  // reactive();
  // sexy();
  study();
  // sky();
  // physical();
  // virtualShow();
}

void virtualShow() {
    for (int i = 0; i < 800; ++i) {
        frontLeftStrip[i] = CHSV(0 * 45, 255, 255); 
        rearRightStrip[i] = CHSV(0 * 45, 255, 255); 
        rearLeftStrip[i] = CHSV(2 * 45, 255, 255); 
        frontRightStrip[i] = CHSV(2 * 45, 255, 255); 
        frontCeilingStrip[i] = CHSV(4 * 45, 255, 255); 
        rearCeilingStrip[i] = CHSV(5 * 45, 255, 255); 
        leftCeilingStrip[i] = CHSV(7 * 45, 255, 255); 
        rightCeilingStrip[i] = CHSV(7 * 45, 255, 255); 
  }
  LEDS.show();
}

void physical() {
  for (int i = 0; i < 800; ++i) {
    leds_1[i] = CHSV(0, 255, 255); 
    leds_2[i] = CHSV(120, 255, 255); 
    leds_3[i] = CHSV(240, 255, 255); 
  }
  LEDS.show();
}

void flicker() {
  // On
  for (int i = 0; i < masterStrip.getLength(); ++i) {
    masterStrip[i] = CRGB(255, 255, 255);
  }
  LEDS.show();

  delay(100);

  // Off
  for (int i = 0; i < masterStrip.getLength(); ++i) {
    masterStrip[i] = CRGB::Black;
  }
  LEDS.show();
  delay(100);
}

void sky(){
  // LEDS.setTemperature(ClearBlueSky);
  for (int i = 0; i < ceilingStrip.getLength(); ++i) {
      ceilingStrip[i] = CRGB(140, 100, 255);
  }
  LEDS.show();

}

void study() {
  LEDS.setTemperature(Candle);
  for (int i = 0; i < masterStrip.getLength(); ++i) {
    masterStrip[i] = CRGB(255, 200, 100);
  }
  LEDS.show();
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

  for (int i = 0; i < 800; ++i) {
    leds_1[i] = CRGB::Black;
    leds_2[i] = CRGB::Black;
    leds_3[i] = CRGB::Black;
  }

  if (ledNewOn > 163) {
    ledNewOn = 163;
  }

  for (int i = 0; i < ledNewOn; ++i) {
     frontLeftStrip[i] = CHSV(i + gHue, 255, 192);
     frontRightStrip[i] = CHSV(i + gHue, 255, 192);
     rearLeftStrip[max(i - 67, 0)] = CHSV(i + gHue, 255, 192);
     rearRightStrip[max(0, i - 14)] = CHSV(i + gHue, 255, 192);
   }

  // Ceiling Rainbow Bar
  for (int i = 0; i < ceilingStrip.getLength(); ++i) {
    ceilingStrip[i] = CHSV(gHue, 255, 192);
  }

  // Shift color;
  ++gHue;

  // Color shift on bass drop
  if (millis() - lastColorChange > 500 && ledNewOn > 140) {
    lastColorChange = millis();
    gHue += 100;
  }

  //chasing 
  int middlefront = frontCeilingStrip.getLength()/2;
  int middleback = frontCeilingStrip.getLength() + rightCeilingStrip.getLength()+ rearCeilingStrip.getLength()/2;
  int middleleft = ceilingStrip.getLength()-leftCeilingStrip.getLength()/2;
  int middleright = frontCeilingStrip.getLength()+rightCeilingStrip.getLength()/2;

  int size = 9;
  int speed = 11; //size should be more than speed
  int offset = size + 2*speed; 

  //front side
  if(ledNewOn > 120){
    chaseIndexFrontR.enqueue(middlefront);
    chaseIndexFrontL.enqueue(middlefront);
  }
  //right side
    for (int i = 0; i < chaseIndexFrontR.itemCount(); ++i) {
      ledFrontR = chaseIndexFrontR.dequeue();

      for (int i = (-size); i<=size ;++i) {
        ceilingStrip[ledFrontR+i] = CRGB::White;
      }

      //if wont go off end of strip, increment
      if (ledFrontR <= middleright) { 
        chaseIndexFrontR.enqueue(ledFrontR + speed);
      
      }
  }

  //left side
  for (int i = 0; i < chaseIndexFrontL.itemCount(); ++i) {
      
      ledFrontL = chaseIndexFrontL.dequeue();

      for(int i = (-size); i<=size ;++i){
        ceilingStrip[ledFrontL+i] = CRGB::White;
      }

      //if wont go off end of strip, decrement

      if(( ledFrontL>= middleleft) or (middlefront >= ledFrontL and ledFrontL>= 0+offset)){ 
          chaseIndexFrontL.enqueue(ledFrontL - speed);
      } else if(ledFrontL < 0+offset) {
          chaseIndexFrontL.enqueue(ceilingStrip.getLength()-size-1);
      }
  }


  //middle shooter
  int numMiddleOn = ledNewOn/5-11;
  for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
        ceilingStrip[middlefront+i] = CRGB::White;
        
      }


  //backside
  if(ledNewOn > 120){
    chaseIndexBackR.enqueue(middleback);
    chaseIndexBackL.enqueue(middleback);
  }

  //backleft
    for (int i = 0; i < chaseIndexBackL.itemCount(); ++i) {
      ledBackL = chaseIndexBackL.dequeue();
    

      for(int i = (-size); i<=size ;++i){
        ceilingStrip[ledBackL+i] = CRGB::White;
        
      }

      //if wont go off end of strip, increment
      if (ledBackL <=middleleft) { 
        chaseIndexBackL.enqueue(ledBackL + speed);
      
      }

  }

  //backright
  for (int i = 0; i < chaseIndexBackR.itemCount(); ++i) {
      ledBackR = chaseIndexBackR.dequeue();
    

      for(int i = (-size); i<=size ;++i){
        ceilingStrip[ledBackR+i] = CRGB::White;
      }

      //if wont go off end of strip, increment
      if (ledBackR >=middleright) { 
        chaseIndexBackR.enqueue(ledBackR - speed);
      
      }

  }
  //middle shooter
  
  for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
        ceilingStrip[middleback+i] = CRGB::White;
        
      }

  FastLED.show();
}

void on(){

  solid(CRGB::Black);

  //On animation

  
  //Up from floors
  int index=1;
  int rearRightDifference =frontLeftStrip.getLength()- rearRightStrip.getLength();
  int rearLeftDifference =frontLeftStrip.getLength()- rearLeftStrip.getLength();
  while(index<frontLeftStrip.getLength()-1){

    if(index<frontLeftStrip.getLength()-1){
      frontLeftStrip[index-1]=CHSV(gHue, 255, 192);
      frontLeftStrip[index]=CHSV(gHue, 255, 192);
      frontLeftStrip[index+1]=CHSV(gHue, 255, 192);
    }

    if(index<frontRightStrip.getLength()-1){
      frontRightStrip[index-1]=CHSV(gHue, 255, 192);
      frontRightStrip[index]=CHSV(gHue, 255, 192);
      frontRightStrip[index+1]=CHSV(gHue, 255, 192);
    }
    
    if(index-rearRightDifference<rearRightStrip.getLength()-1 and index>rearRightDifference){
      rearRightStrip[index-1-rearRightDifference]=CHSV(gHue, 255, 192);
      rearRightStrip[index-rearRightDifference]=CHSV(gHue, 255, 192);
      rearRightStrip[index+1-rearRightDifference]=CHSV(gHue, 255, 192);
    }
    
    if(index-rearLeftDifference<rearLeftStrip.getLength()-1 and index>rearLeftDifference){
      rearLeftStrip[index-1-rearLeftDifference]=CHSV(gHue, 255, 192);
      rearLeftStrip[index-rearLeftDifference]=CHSV(gHue, 255, 192);
      rearLeftStrip[index+1-rearLeftDifference]=CHSV(gHue, 255, 192);
    }
    

    index+=3;
    gHue+=3;
    FastLED.show();
  }


  //ceiling
  index=1;
  while(index<rightCeilingStrip.getLength()-1){

    if(index<frontCeilingStrip.getLength()-1){
      frontCeilingStrip[index-1]=CHSV(gHue, 255, 192);
      frontCeilingStrip[index]=CHSV(gHue, 255, 192);
      frontCeilingStrip[index+1]=CHSV(gHue, 255, 192);
    }

    if(index<rightCeilingStrip.getLength()-1){
      rightCeilingStrip[index-1]=CHSV(gHue, 255, 192);
      rightCeilingStrip[index]=CHSV(gHue, 255, 192);
      rightCeilingStrip[index+1]=CHSV(gHue, 255, 192);
    }
    
    if(index<leftCeilingStrip.getLength()-1){
      leftCeilingStrip[index-1]=CHSV(gHue, 255, 192);
      leftCeilingStrip[index]=CHSV(gHue, 255, 192);
      leftCeilingStrip[index+1]=CHSV(gHue, 255, 192);
    }
    
    if(index<rearCeilingStrip.getLength()-1){
      rearCeilingStrip[index-1]=CHSV(gHue, 255, 192);
      rearCeilingStrip[index]=CHSV(gHue, 255, 192);
      rearCeilingStrip[index+1]=CHSV(gHue, 255, 192);
    }

    index+=3;
    gHue+=3;
    FastLED.show();
  }

  //ceiling go to constant hue
  gHue=0;
  int decrementIndex = frontLeftStrip.getLength()-1;
  int incrementIndex = 1;
  index=1;
  while(incrementIndex<rightCeilingStrip.getLength()-1){
    index = incrementIndex;
    if(index<frontCeilingStrip.getLength()-1){
      frontCeilingStrip[index-1]=CHSV(gHue, 255, 192);
      frontCeilingStrip[index]=CHSV(gHue, 255, 192);
      frontCeilingStrip[index+1]=CHSV(gHue, 255, 192);
    }

    if(index<rightCeilingStrip.getLength()-1){
      rightCeilingStrip[index-1]=CHSV(gHue, 255, 192);
      rightCeilingStrip[index]=CHSV(gHue, 255, 192);
      rightCeilingStrip[index+1]=CHSV(gHue, 255, 192);
    }
    
    if(index<leftCeilingStrip.getLength()-1){
      leftCeilingStrip[index-1]=CHSV(gHue, 255, 192);
      leftCeilingStrip[index]=CHSV(gHue, 255, 192);
      leftCeilingStrip[index+1]=CHSV(gHue, 255, 192);
    }
    
    if(index<rearCeilingStrip.getLength()-1){
      rearCeilingStrip[index-1]=CHSV(gHue, 255, 192);
      rearCeilingStrip[index]=CHSV(gHue, 255, 192);
      rearCeilingStrip[index+1]=CHSV(gHue, 255, 192);
    }

    //push down to reactive offset
    index=decrementIndex;
    if(index>50){
      frontLeftStrip[index-1]=CRGB::Black;
      frontLeftStrip[index]=CRGB::Black;
      frontLeftStrip[index+1]=CRGB::Black;
    }

    if(index>50){
      frontRightStrip[index-1]=CRGB::Black;
      frontRightStrip[index]=CRGB::Black;
      frontRightStrip[index+1]=CRGB::Black;
    }
    
    if(index - rearRightDifference>50){
      rearRightStrip[index-1-rearRightDifference]=CRGB::Black;
      rearRightStrip[index-rearRightDifference]=CRGB::Black;
      rearRightStrip[index+1-rearRightDifference]=CRGB::Black;
    }
    
     if(index - rearLeftDifference> 2){
      rearLeftStrip[index-1-rearLeftDifference-1]=CRGB::Black;
      rearLeftStrip[index-rearLeftDifference-1]=CRGB::Black;
      rearLeftStrip[index+1-rearLeftDifference-1]=CRGB::Black;
    }

    
    incrementIndex+=3;
    decrementIndex-=1;
    LEDS.show();
  }
}


  static float pulseSpeed = 0.25;  // Larger value gives faster pulse.

  // uint8_t hueA = 15;  // Start hue at valueMin.
  uint8_t satA = 0;  // Start saturation at valueMin.
  float valueMin = 230.0;  // Pulse minimum value (Should be less then valueMax).

  // uint8_t hueB = 95;  // End hue at valueMax.
  uint8_t satB = 15;  // End saturation at valueMax.
  float valueMax = 255.0;  // Pulse maximum value (Should be larger then valueMin).

  // uint8_t hue = hueA;  // Do Not Edit
  uint8_t sat = satA;  // Do Not Edit
  float val = valueMin;  // Do Not Edit
  // uint8_t hueDelta = hueA - hueB;  // Do Not Edit
  static float delta = (valueMax - valueMin) / 2.35040238;  // Do Not Edit
void sexy(){
  // float dV = ((exp(sin(pulseSpeed * millis()/2000.0*PI)) - 0.36787944) * delta);
  // val = valueMin + dV;
  // // hue = map(val, valueMin, valueMax, hueA, hueB);  // Map hue based on current val
  // sat = map(val, valueMin, valueMax, satA, satB);  // Map sat based on current val

  // for (int i = 0; i < masterStrip.getLength(); i++) {
  //   masterStrip[i] = CRGB(255, sat, sat);
  //   LEDS.setBrightness(255);
  //   // masterStrip[i] = CHSV(0, sat, val);

  //   // You can experiment with commenting out these dim8_video lines
  //   // to get a different sort of look.
  //   // masterStrip[i].r = dim8_video(masterStrip[i].r);
  //   // masterStrip[i].g = dim8_video(masterStrip[i].g);
  //   // masterStrip[i].b = dim8_video(masterStrip[i].b);
  // }

  // FastLED.show();

  // // Serial.println(millis() - start);

  // // solid(CRGB::Red);
  // // for (int i = 200; i > 100; ++i) {
  // //   LEDS.setBrightness(i);
  // //   LEDS.show();
  // // }
  for (int i = 30; i >= 0; --i) {
    solid(CRGB(255, i, i));
    LEDS.show();
    delay(100);
  }
  
  for (int i = 0; i < 30; ++i) {
    solid(CRGB(255, i, i));
    LEDS.show();
    delay(100);
  }
  

}

void solid(CRGB color){
  //turn everything off
  for(int i=0;i<ceilingStrip.getLength();++i){
    ceilingStrip[i]=color;
  }

  for(int i=0;i<frontRightStrip.getLength();++i){
    frontRightStrip[i]=color;
  }

  for(int i=0;i<frontLeftStrip.getLength();++i){
    frontLeftStrip[i]=color;
  }

  for(int i=0;i<rearLeftStrip.getLength();++i){
    rearLeftStrip[i]=color;
  }

   for(int i=0;i<rearRightStrip.getLength();++i){
    rearRightStrip[i]=color;
  }

  for(int i=0;i<rearLeftStrip.getLength();++i){
    rearLeftStrip[i]=color;
  }
}




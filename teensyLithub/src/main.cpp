#include "FastLED.h"

// #include <OctoWS2811.h>

// #include <Adafruit_NeoPixel.h>
// #ifdef __AVR__
//  #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
// #endif
#include <PololuLedStrip.h>
#include <ArduinoQueue.h>
#include <VirtualStrip.h>
//#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
//#include <SerialFlash.h>
//#include <usb_audio.h>

#include <SPI.h> 
//#include <SD.h>


#include<time.h>
#include<stdio.h>

#define NUM_LEDS 133
#define LED_PIN 2

CRGB leds[NUM_LEDS];

#define DATA_PIN_1 2 //LED_PIN

#define BRIGHTNESS 255

int ledNewOn = NUM_LEDS; // How many LEDS to turn on
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
int microphonePin = 0;    // select the input pin for the potentiometer
unsigned int sample;

int Left_Bottom = 0;
int ceiling = 50;
int Right_Bottom = 80;

void solid(CRGB color);
void movingRainbow();
void reactive();
void nightSky();
void meteor();

//meteor variables
struct trailLED;
struct trailLED{
  int index;
  uint8_t color;
  int saturation;
  int brightness;
};

ArduinoQueue<trailLED> trailQueue(1800); //Q for trail
ArduinoQueue<trailLED> meteorQueue(1800); //Q for meteor

ArduinoQueue<trailLED> brightStar(1800); 
ArduinoQueue<trailLED> dimStar(1800); 
uint32_t lastColorChange;

typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 100; // rotating "base color" used by many of the patterns

VirtualStrip leftBottom (leds, 0, 50, false);
VirtualStrip ceilingT (leds, ceiling, 30, false);
VirtualStrip rightBottom (leds, Right_Bottom, 50, true);
VirtualStrip ceilingStrip(leds, 0, 130, false);
//chasing variables
//front side
// int ledFrontR = 0; //right side index
// int ledFrontL = 0; //left side index
// ArduinoQueue<int> chaseIndexFrontR(1800); //Q to go right
// ArduinoQueue<int> chaseIndexFrontL(1800); //Q to go left

// //backside
// int ledBackR = 0; //right side index
// int ledBackL = 0; //left side index
// ArduinoQueue<int> chaseIndexBackR(1800); //Q to go right
// ArduinoQueue<int> chaseIndexBackL(1800); //Q to go left
// Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, LED_PIN, NEO_GRB + NEO_KHZ800);
 
void setup() {
  Serial.begin(9600);
  
  // Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, LED_PIN, NEO_GRB + NEO_KHZ800);
  // strip.begin();
  // strip.show();
  // strip.setBrightness(100);
  

  
  FastLED.addLeds<WS2811, LED_PIN, BRG> (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(12, 2000);
}

void loop() {
  // for (int i = 0; i < NUM_LEDS; i++) {
  //   leds[i] = CRGB(255, 255, 255);
  // }
  //FastLED.show();

  //solid(CRGB::Blue);
  //movingRainbow();
  //reactive();
  //nightSky();
  //meteor();
delay(30);
  // leds[99] = CRGB::Blue;
  FastLED.show();
  // for(int i = 0; i < 1; i++) {
  //   strip.setPixelColor(i, strip.Color(255, 255, 255));
  // }
  // //strip.setPixelColor(60, strip.Color(255, 255, 255));
  
  // strip.show();
  
}

int meteorTip=0;
  uint8_t hueTip=100;
  int saturation=255;
  int initialBrightness = 255;
  int acc=0;

void meteor(){

 trailLED ledToBeAdded1;
// solid(CRGB::Black);
// (meteorTip+2)<strip.getLength();

 //while(true){
    if (meteorTip>=ceilingStrip.getLength()-5){
      meteorTip=0;
    }
    
    //make tip bright
    ceilingStrip[meteorTip]=CHSV(hueTip,255,initialBrightness); // (hue,saturation, brightness)
    ceilingStrip[meteorTip+1]=CHSV(hueTip,saturation,100);

    
    //light trail
    for (int i = 0; i < trailQueue.itemCount(); ++i) {
      ledToBeAdded1 = trailQueue.dequeue();
    
      ceilingStrip[ledToBeAdded1.index]=CHSV(ledToBeAdded1.color+rand()%30,ledToBeAdded1.saturation,ledToBeAdded1.brightness);
      
      if(rand()%4==0){
        ledToBeAdded1.brightness = (rand() % ledToBeAdded1.brightness + 1);
      }
      
      if (ledToBeAdded1.brightness>10){
        trailQueue.enqueue(ledToBeAdded1);
      }else{
        ceilingStrip[ledToBeAdded1.index]=CRGB::Black;
      }

  }

    //add tip to trail queue
    ledToBeAdded1.index = meteorTip;
    ledToBeAdded1.color = hueTip;
    ledToBeAdded1.saturation = saturation;
    ledToBeAdded1.brightness = initialBrightness;

    trailQueue.enqueue(ledToBeAdded1);
    ledToBeAdded1.index = meteorTip+1;
    trailQueue.enqueue(ledToBeAdded1);

    //sample = analogRead(microphonePin);
    //Serial.println(peakToPeak);
    //printf(sample);
    
    meteorTip+=1;
  

}

// void on(){

//   solid(CRGB::Black);

//   //On animation

  
//   //Up from floors
//   int index=1;
//   int rearRightDifference =frontLeftStrip.getLength()- rearRightStrip.getLength();
//   int rearLeftDifference =frontLeftStrip.getLength()- rearLeftStrip.getLength();
//   while(index<frontLeftStrip.getLength()-1){

//     if(index<frontLeftStrip.getLength()-1){
//       frontLeftStrip[index-1]=CHSV(gHue, 255, 255);
//       frontLeftStrip[index]=CHSV(gHue, 255, 255);
//       frontLeftStrip[index+1]=CHSV(gHue, 255, 255);
//     }

//     if(index<frontRightStrip.getLength()-1){
//       frontRightStrip[index-1]=CHSV(gHue, 255, 255);
//       frontRightStrip[index]=CHSV(gHue, 255, 255);
//       frontRightStrip[index+1]=CHSV(gHue, 255, 255);
//     }
    
//     if(index-rearRightDifference<rearRightStrip.getLength()-1 and index>rearRightDifference){
//       rearRightStrip[index-1-rearRightDifference]=CHSV(gHue, 255, 255);
//       rearRightStrip[index-rearRightDifference]=CHSV(gHue, 255, 255);
//       rearRightStrip[index+1-rearRightDifference]=CHSV(gHue, 255, 255);
//     }
    
//     if(index-rearLeftDifference<rearLeftStrip.getLength()-1 and index>rearLeftDifference){
//       rearLeftStrip[index-1-rearLeftDifference]=CHSV(gHue, 255, 255);
//       rearLeftStrip[index-rearLeftDifference]=CHSV(gHue, 255, 255);
//       rearLeftStrip[index+1-rearLeftDifference]=CHSV(gHue, 255, 255);
//     }
    

//     index+=3;
//     gHue+=3;
//     FastLED.show();
//   }


//   //ceiling
//   index=1;
//   while(index<rightCeilingStrip.getLength()){

//     if(index<frontCeilingStrip.getLength()-1){
//       frontCeilingStrip[index-1]=CHSV(gHue, 255, 255);
//       frontCeilingStrip[index]=CHSV(gHue, 255, 255);
//       frontCeilingStrip[index+1]=CHSV(gHue, 255, 255);
//     }

//     if(index<rightCeilingStrip.getLength()-1){
//       rightCeilingStrip[index-1]=CHSV(gHue, 255, 255);
//       rightCeilingStrip[index]=CHSV(gHue, 255, 255);
//       rightCeilingStrip[index+1]=CHSV(gHue, 255, 255);
//     }
    
//     if(index<leftCeilingStrip.getLength()-1){
//       leftCeilingStrip[index-1]=CHSV(gHue, 255, 255);
//       leftCeilingStrip[index]=CHSV(gHue, 255, 255);
//       leftCeilingStrip[index+1]=CHSV(gHue, 255, 255);
//     }
    
//     if(index<rearCeilingStrip.getLength()-1){
//       rearCeilingStrip[index-1]=CHSV(gHue, 255, 255);
//       rearCeilingStrip[index]=CHSV(gHue, 255, 255);
//       rearCeilingStrip[index+1]=CHSV(gHue, 255, 255);
//     }

//     index+=3;
//     gHue+=3;
//     FastLED.show();
//   }

//   //ceiling go to constant hue
//   gHue=0;
//   int decrementIndex = frontLeftStrip.getLength()-1;
//   int incrementIndex = 1;
//   index=1;
//   while(incrementIndex<rightCeilingStrip.getLength()-1){
//     index = incrementIndex;
//     if(index<frontCeilingStrip.getLength()-1){
//       frontCeilingStrip[index-1]=CHSV(gHue, 255, 255);
//       frontCeilingStrip[index]=CHSV(gHue, 255, 255);
//       frontCeilingStrip[index+1]=CHSV(gHue, 255, 255);
//     }

//     if(index<rightCeilingStrip.getLength()-1){
//       rightCeilingStrip[index-1]=CHSV(gHue, 255, 255);
//       rightCeilingStrip[index]=CHSV(gHue, 255, 255);
//       rightCeilingStrip[index+1]=CHSV(gHue, 255, 255);
//     }
    
//     if(index<leftCeilingStrip.getLength()-1){
//       leftCeilingStrip[index-1]=CHSV(gHue, 255, 255);
//       leftCeilingStrip[index]=CHSV(gHue, 255, 255);
//       leftCeilingStrip[index+1]=CHSV(gHue, 255, 255);
//     }
    
//     if(index<rearCeilingStrip.getLength()-1){
//       rearCeilingStrip[index-1]=CHSV(gHue, 255, 255);
//       rearCeilingStrip[index]=CHSV(gHue, 255, 255);
//       rearCeilingStrip[index+1]=CHSV(gHue, 255, 255);
//     }
    

//     //push down to reactive offset
//     index=decrementIndex;
//     if(index>50){
//       frontLeftStrip[index-1]=CRGB::Black;
//       frontLeftStrip[index]=CRGB::Black;
//       frontLeftStrip[index+1]=CRGB::Black;
//     }

//     if(index>50){
//       frontRightStrip[index-1]=CRGB::Black;
//       frontRightStrip[index]=CRGB::Black;
//       frontRightStrip[index+1]=CRGB::Black;
//     }
    
//     if(index - rearRightDifference>50){
//       rearRightStrip[index-1-rearRightDifference]=CRGB::Black;
//       rearRightStrip[index-rearRightDifference]=CRGB::Black;
//       rearRightStrip[index+1-rearRightDifference]=CRGB::Black;
//     }
    
//      if(index - rearLeftDifference> 2){
//       rearLeftStrip[index-1-rearLeftDifference-1]=CRGB::Black;
//       rearLeftStrip[index-rearLeftDifference-1]=CRGB::Black;
//       rearLeftStrip[index+1-rearLeftDifference-1]=CRGB::Black;
//     }

    
//     incrementIndex+=3;
//     decrementIndex-=1;
//     FastLED.show();
//   }

// }

void nightSky(){

    trailLED ledToBeAdded;
    int random=rand()%100;

    if(random>80){
      
      ledToBeAdded.index=rand()%ceilingStrip.getLength();
      //ledToBeAdded.index=index;
      ledToBeAdded.brightness=0;
      ledToBeAdded.saturation=rand()%50;
      ledToBeAdded.color=20;
      brightStar.enqueue(ledToBeAdded);
    }


  //increases brightness
    for(int i=0;i<brightStar.itemCount();++i){
      ledToBeAdded = brightStar.dequeue();
      if (ledToBeAdded.brightness<235)
      {
        ledToBeAdded.brightness+=rand()%15;
        brightStar.enqueue(ledToBeAdded);
        ceilingStrip[ledToBeAdded.index]= CHSV(ledToBeAdded.color, ledToBeAdded.saturation, ledToBeAdded.brightness);

      }else{
        dimStar.enqueue(ledToBeAdded);

      }
      
    }

  //decreases brightness
    for(int i=0;i<dimStar.itemCount();++i){
      ledToBeAdded = dimStar.dequeue();
      

      if (ledToBeAdded.brightness>10)
      {
        ledToBeAdded.brightness-=rand()%15;
        dimStar.enqueue(ledToBeAdded);
        ceilingStrip[ledToBeAdded.index] = CHSV(ledToBeAdded.color, ledToBeAdded.saturation, ledToBeAdded.brightness);
      }else{
        ceilingStrip[ledToBeAdded.index] = CRGB::Black;
      }


    }   

}

// //**********************************************************************
// // Helper method - reactive()
// // Reacts to sounds
// //**********************************************************************
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
    Serial.println(analogRead(microphonePin));
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
  if (ledNewOn > NUM_LEDS) {
    ledNewOn = NUM_LEDS;
  }

  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CRGB::Black;
    // leds_2[i] = CRGB::Black;
    // leds_3[i] = CRGB::Black;
  }

  // if (ledNewOn > 163) { DO WE NEED THIS????
  //   ledNewOn = 163;
  // }

  //bottom strips
   for (int i = 0; i < ledNewOn; ++i) {
     leds[Right_Bottom + i] = CHSV(i + gHue, 255, 255);
     //leds_2[Front_Right_Bottom - i] = CHSV(i + gHue, 255, 255);
     //leds_3[min(Rear_Right_Bottom, Rear_Right_Bottom - i + 14)] = CHSV(i + gHue, 255, 255);
     leds[Left_Bottom - i + 67] = CHSV(i + gHue, 255, 255);
   }

  // for (int i = 0; i < ledNewOn; ++i) {
  //    //frontLeftStrip[i] = CHSV(i + gHue, 255, 255);
  //   //  frontRightStrip[i] = CHSV(i + gHue, 255, 255);
  //   //  rearLeftStrip[max(i - 67, 0)] = CHSV(i + gHue, 255, 255);
  //   //  rearRightStrip[max(0, i - 14)] = CHSV(i + gHue, 255, 255);
  //  }

  // Ceiling Rainbow Bar
  for (int i = 0; i < ceilingT.getLength(); ++i) {
    ceilingT[i] = CHSV(gHue, 255, 255);
  }

  // Shift color;
  gHue = gHue + 1;

  // Color shift on bass drop
  if (millis() - lastColorChange > 1000 && ledNewOn > 140) {
    lastColorChange = millis();
    gHue += 100;
  }

  //chasing 
  // int middlefront = frontCeilingStrip.getLength()/2;
  // int middleback = frontCeilingStrip.getLength() + rightCeilingStrip.getLength()+ rearCeilingStrip.getLength()/2;
  // int middleleft = ceilingStrip.getLength()-leftCeilingStrip.getLength()/2;
  // int middleright = frontCeilingStrip.getLength()+rightCeilingStrip.getLength()/2;

  int size = 9;
  int speed = 11; //size should be more than speed
  int offset = size + 2*speed; 

  //front side
  // if(ledNewOn > 120){
  //   chaseIndexFrontR.enqueue(middlefront);
  //   chaseIndexFrontL.enqueue(middlefront);
  // }
  // //right side
  //   for (int i = 0; i < chaseIndexFrontR.itemCount(); ++i) {
  //     ledFrontR = chaseIndexFrontR.dequeue();
    

  //     for(int i = (-size); i<=size ;++i){
  //       ceilingStrip[ledFrontR+i] = CRGB::White;
        
  //     }

  //     //if wont go off end of strip, increment
  //     if (ledFrontR <= middleright) { 
  //       chaseIndexFrontR.enqueue(ledFrontR + speed);
      
  //     }

  
  // }
  // //left side
  // for (int i = 0; i < chaseIndexFrontL.itemCount(); ++i) {
      
  //     ledFrontL = chaseIndexFrontL.dequeue();

  //     for(int i = (-size); i<=size ;++i){
  //       ceilingStrip[ledFrontL+i] = CRGB::White;
  //     }

  //     //if wont go off end of strip, decrement

  //     if(( ledFrontL>= middleleft) or (middlefront >= ledFrontL and ledFrontL>= 0+offset)){ 
  //         chaseIndexFrontL.enqueue(ledFrontL - speed);
  //     }else if(ledFrontL < 0+offset) {
  //         chaseIndexFrontL.enqueue(ceilingStrip.getLength()-size-1);
  //     }
  // }


  // //middle shooter
  // int numMiddleOn = ledNewOn/4-13;
  // for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
  //       ceilingStrip[middlefront+i] = CRGB::White;
        
  //     }


  // //backside
  // if(ledNewOn > 120){
  //   chaseIndexBackR.enqueue(middleback);
  //   chaseIndexBackL.enqueue(middleback);
  // }

  // //backleft
  //   for (int i = 0; i < chaseIndexBackL.itemCount(); ++i) {
  //     ledBackL = chaseIndexBackL.dequeue();
    

  //     for(int i = (-size); i<=size ;++i){
  //       ceilingStrip[ledBackL+i] = CRGB::White;
        
  //     }

  //     //if wont go off end of strip, increment
  //     if (ledBackL <=middleleft) { 
  //       chaseIndexBackL.enqueue(ledBackL + speed);
      
  //     }

  // }

  // //backright
  // for (int i = 0; i < chaseIndexBackR.itemCount(); ++i) {
  //     ledBackR = chaseIndexBackR.dequeue();
    

  //     for(int i = (-size); i<=size ;++i){
  //       ceilingStrip[ledBackR+i] = CRGB::White;
        
  //     }

  //     //if wont go off end of strip, increment
  //     if (ledBackR >=middleright) { 
  //       chaseIndexBackR.enqueue(ledBackR - speed);
      
  //     }

  // }
  // //middle shooter
  
  // for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
  //       ceilingStrip[middleback+i] = CRGB::White;
        
  //     }
}

void solid(CRGB color){
  //turn everything off
  for(int i=0;i<rightBottom.getLength();++i){
    rightBottom[i]=color;
  }

  for(int i=0;i<ceilingT.getLength();++i){
    ceilingT[i]=color;
  }

  for(int i=0;i<leftBottom.getLength();++i){
    leftBottom[i]=color;
  }

  // for(int i=0;i<rearLeftStrip.getLength();++i){
  //   rearLeftStrip[i]=color;
  // }

  //  for(int i=0;i<rearRightStrip.getLength();++i){
  //   rearRightStrip[i]=color;
  // }

  // for(int i=0;i<rearLeftStrip.getLength();++i){
  //   rearLeftStrip[i]=color;
  // }
}

void movingRainbow() {
   for (int i = 0; i < NUM_LEDS; ++i) {
     leds[i] = CHSV(gHue + i*5, 255, 255);
  //   leds_2[i] = CHSV(gHue + i*5, 255, 255);
  //   leds_3[i] = CHSV(gHue + i*5, 255, 255);
   }
  //fill_rainbow(leds, NUM_LEDS, gHue, 1);
  // fill_rainbow(leds_2, NUM_LEDS_2, gHue, 1);
  // fill_rainbow(leds_3, NUM_LEDS_3, gHue, 1);
  gHue += 1;
  //FastLED.show();
}

// void solidfade(int color, int saturation, int brightness){
//   //turn everything off
//   for(int i=0;i<ceilingStrip.getLength();++i){
//     ceilingStrip[i]=CHSV(color,saturation,brightness);
//   }

//   for(int i=0;i<frontRightStrip.getLength();++i){
//     frontRightStrip[i]=CHSV(color,saturation,brightness);
//   }

//   for(int i=0;i<frontLeftStrip.getLength();++i){
//     frontLeftStrip[i]=CHSV(color,saturation,brightness);
//   }

//   for(int i=0;i<rearLeftStrip.getLength();++i){
//     rearLeftStrip[i]=CHSV(color,saturation,brightness);
//   }

//    for(int i=0;i<rearRightStrip.getLength();++i){
//     rearRightStrip[i]=CHSV(color,saturation,brightness);
//   }

//   for(int i=0;i<rearLeftStrip.getLength();++i){
//     rearLeftStrip[i]=CHSV(color,saturation,brightness);
//   }
// }

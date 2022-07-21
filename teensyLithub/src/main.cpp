// Reacts to sounds using LEDs

#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>
#include <ArduinoQueue.h>


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <usb_audio.h>

#include<time.h>
#include<stdio.h>


// How many leds in your strip?
#define NUM_LEDS_1 150 //changed from 800
// #define NUM_LEDS_2 800
// #define NUM_LEDS_3 800

// Usable pins:
//   Teensy LC:   1, 4, 5, 24
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
//   Teensy 3.5:  1, 5, 8, 10, 26, 32, 33, 48
//   Teensy 3.6:  1, 5, 8, 10, 26, 32, 33
//   Teensy 4.0:  1, 8, 14, 17, 20, 24, 29, 39
//   Teensy 4.1:  1, 8, 14, 17, 20, 24, 29, 35, 47, 53

#define DATA_PIN_1 1 //1
// #define DATA_PIN_2 8 //8
// #define DATA_PIN_3 14
#define DATA_PIN_D 18
#define DATA_PIN_A 20
#define DATA_PIN_C 19

#define BRIGHTNESS 255

// int Front_Left_Bottom = 0;
// int Front_Left_TopL = 163;
// int Rear_Left_Bottom = 600;
// int Rear_Left_TopL = Rear_Left_Bottom - 163 + 67; // 504
// int Rear_Left_TopR = 668;
CRGB leds_1[NUM_LEDS_1]; // Left wall

// int Front_Left_TopR = 0;
// int Front_Right_Bottom = 340;
// int Front_Right_TopL = 340 - 163;
// CRGB leds_2[NUM_LEDS_2]; // Center wall

// int Front_Right_TopR = 0;
// int Rear_Right_Bottom = 506;
// int Rear_Right_Num = 149;
// int Rear_Right_TopR = 506 - Rear_Right_Num; // 357
// int Rear_Right_TopL = 507;
// CRGB leds_3[NUM_LEDS_3]; // Right / Back wall

int ledNewOn = NUM_LEDS_1; // How many LEDS to turn on
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
int microphonePin = A8;    // select the input pin for the potentiometer
unsigned int sample;

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


//meteor variables
struct trailLED;
struct trailLED{
  int index;
  uint8_t color;
  int saturation;
  int brightness;
};

// ArduinoQueue<trailLED> trailQueue(1800); //Q for trail
// ArduinoQueue<trailLED> meteorQueue(1800); //Q for meteor

// ArduinoQueue<trailLED> brightStar(1800); 
// ArduinoQueue<trailLED> dimStar(1800); 
uint32_t lastColorChange;

typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 100; // rotating "base color" used by many of the patterns



void solid(CRGB color);
// void solidfade(int color, int saturation, int brightness);
// void on();
// void reactive();
//void movingRainbow();
// void calibrateStripPosition(CRGB* start);
// void sexy();
// void flicker();
// void dashed();
// void animation();
// void nightSky();
// void fapOn();

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
VirtualStrip frontLeftStrip(leds_1, 0, 149); //VirtualStrip frontLeftStrip(leds_1, Front_Left_Bottom, Front_Left_TopL - Front_Left_Bottom);

// VirtualStrip frontRightStrip(leds_2, Front_Right_TopL, Front_Right_Bottom - Front_Right_TopL, true);
// VirtualStrip rearLeftStrip(leds_1, Rear_Left_TopL, Rear_Left_Bottom - Rear_Left_TopL, true);
// VirtualStrip rearRightStrip(leds_3, Rear_Right_TopR, Rear_Right_Num, true);

// // Define ceiling bars
// VirtualStrip rightCeilingStrip(leds_3, Front_Right_TopR, Rear_Right_TopR - Front_Right_TopR);
// VirtualStrip rearCeilingStrip(leds_3, Rear_Right_TopL, Rear_Left_TopR - Rear_Right_TopL);
// VirtualStrip leftCeilingStrip(leds_1, Front_Left_TopL, Rear_Left_TopL - Front_Left_TopL, true);
// VirtualStrip frontCeilingStrip(leds_2, Front_Left_TopR, Front_Right_TopL - Front_Left_TopR);

// // Master ceiling
// VirtualStrip ceilingStrip(leds_2, Front_Left_TopR, Front_Right_TopL - Front_Left_TopR);


//define meteor
// void meteor();
// void resetMeteorQueues();
// void fadein(VirtualStrip strip);
// void pomodoro();
// void seconds();

void setup() {
  Serial.begin(9600); // open the serial port at 9600 bps:
  
  // Native strips
  LEDS.addLeds<WS2812SERIAL, DATA_PIN_1, BGR>(leds_1, NUM_LEDS_1).setCorrection(TypicalLEDStrip);
  // LEDS.addLeds<WS2812SERIAL, DATA_PIN_2, BGR>(leds_2, NUM_LEDS_2).setCorrection(TypicalLEDStrip);
  // LEDS.addLeds<WS2812SERIAL, DATA_PIN_3, BGR>(leds_3, NUM_LEDS_3).setCorrection(TypicalLEDStrip);

  // LEDS.addLeds<WS2812SERIAL, DATA_PIN_3, BGR>(rearRightStrip, 149);
  LEDS.setBrightness(BRIGHTNESS);

  // pinMode(DATA_PIN_A, INPUT);
  // pinMode(DATA_PIN_C, INPUT);
  // pinMode(DATA_PIN_D, INPUT);
  

  // ceilingStrip.appendStrip(rightCeilingStrip);
  // ceilingStrip.appendStrip(rearCeilingStrip);
  // ceilingStrip.appendStrip(leftCeilingStrip);

 //on();
}



// CRGB* test[800];
// clock_t start, end;
// double totalTime=0;
// int minute=0;

int stateA=0;
int stateC=0;
int stateD=0;

void loop() {     
  if(digitalRead(DATA_PIN_A)==1){
      stateC=0;
      stateD=0;
      if(stateA==0){
      //on();
      stateA=1;
      }
      else{
        stateA=0;
        stateC=0;
        stateD=0;
        solid(CRGB::Black);
        FastLED.show();
              delay(1000);
      }
    }

    if(digitalRead(DATA_PIN_C)==1){
      stateC=1;
      stateA=1;
    }
    if(digitalRead(DATA_PIN_D)==1){
      stateD=1;
      stateA=1;
    }


//applies the changes based on buttons pressed


    // if(stateA==1){
    //   on();
    //   stateA=0;
    // }

    if(stateC==1){
      //nightSky();
    }
    if(stateD==1){
      //meteor();
    }

     //start = clock();
    //  ... /* Do the work. */
    //  end = clock();
    //  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    //  if (totalTime>1000){
    //  seconds(); 
    //  totalTime=0;
    //  minute++;
    //  }
   
  //resetMeteorQueues();
  
  //dashed();



  // for (int i = 0; i < ceilingStrip.getLength(); ++i) {
  //   ceilingStrip[i] = CRGB::SkyBlue;
  // }

  // if (rand()%7==5){
  //   meteor();
  // }
  //Fateen And Patty animation (FAP animation)
  //  FastLED.show();
  //  solid(CRGB::Black);
    
  solid(CRGB::Red);
//  nightSky();
 //  meteor();
  //fapOn();

  //fadein(ceilingStrip);
  //pomodoro();
  //reactive();
  FastLED.show();
  //delay(1000);
  // end=clock();
  // totalTime+=(bool)(end-start);
}

//**********************************************************************
// Helper method - calibrateStripPosition()
// Finds the position of a strip
//**********************************************************************
// void calibrateStripPosition(CRGB* start) {
//   // fill_rainbow(leds_3 + 660, 20, 100, 50);
//   FastLED.show();
// }

//**********************************************************************
// Helper method - movingRainbow()
// Reacts to sounds
//**********************************************************************
void movingRainbow() {
   for (int i = 0; i < 800; ++i) {
     leds_1[i] = CHSV(gHue + i*5, 255, 255);
  //   leds_2[i] = CHSV(gHue + i*5, 255, 255);
  //   leds_3[i] = CHSV(gHue + i*5, 255, 255);
   }
  fill_rainbow(leds_1, NUM_LEDS_1, gHue, 1);
  // fill_rainbow(leds_2, NUM_LEDS_2, gHue, 1);
  // fill_rainbow(leds_3, NUM_LEDS_3, gHue, 1);
  gHue += 1;
  FastLED.show();
}

// //**********************************************************************
// // Helper method - reactive()
// // Reacts to sounds
// //**********************************************************************
// void reactive() {
//     //  **********************************************************************
//   //   Read from microphone
//   //  **********************************************************************
//   unsigned long startMillis = millis(); // Start of sample window
//   unsigned int peakToPeak = 0;   // peak-to-peak level

//   unsigned int signalMax = 0;
//   unsigned int signalMin = 1024;

//   // collect data for sampleWindow mS
//   while (millis() - startMillis < sampleWindow)
//   {
//     sample = analogRead(microphonePin);
//     //    Serial.println(analogRead(microphonePin));
//     if (sample < 1024)  // Sanitize input
//     {
//       if (sample > signalMax) // Save just the max levels
//       {
//         signalMax = sample;
//       }
//       else if (sample < signalMin) // Save just the min levels
//       {
//         signalMin = sample;
//       }
//     }
//   }

//   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude

//   double volts = (peakToPeak * 5.0) / 1024;  // convert to volts

//   ledNewOn = ceil(volts * 37.5) + 50;

//   // Cut-off at NUM_LEDS
//   if (ledNewOn > NUM_LEDS_1) {
//     ledNewOn = NUM_LEDS_1;
//   }

//   for (int i = 0; i < NUM_LEDS_1; ++i) {
//     leds_1[i] = CRGB::Black;
//     // leds_2[i] = CRGB::Black;
//     // leds_3[i] = CRGB::Black;
//   }

//   if (ledNewOn > 163) {
//     ledNewOn = 163;
//   }

//   //  for (int i = 0; i < ledNewOn; ++i) {
//   //    leds_1[Front_Left_Bottom + i] = CHSV(i + gHue, 255, 255);
//   //    leds_2[Front_Right_Bottom - i] = CHSV(i + gHue, 255, 255);
//   //    leds_3[min(Rear_Right_Bottom, Rear_Right_Bottom - i + 14)] = CHSV(i + gHue, 255, 255);
//   //    leds_1[Rear_Left_Bottom - i + 67] = CHSV(i + gHue, 255, 255);
//   //  }

//   for (int i = 0; i < ledNewOn; ++i) {
//      frontLeftStrip[i] = CHSV(i + gHue, 255, 255);
//     //  frontRightStrip[i] = CHSV(i + gHue, 255, 255);
//     //  rearLeftStrip[max(i - 67, 0)] = CHSV(i + gHue, 255, 255);
//     //  rearRightStrip[max(0, i - 14)] = CHSV(i + gHue, 255, 255);
//    }

//   // // Ceiling Rainbow Bar
//   // for (int i = 0; i < ceilingStrip.getLength(); ++i) {
//   //   ceilingStrip[i] = CHSV(gHue, 255, 255);
//   // }

//   // Shift color;
//   gHue = gHue + 1;

//   // Color shift on bass drop
//   if (millis() - lastColorChange > 1000 && ledNewOn > 140) {
//     lastColorChange = millis();
//     gHue += 100;
//   }

//   //chasing 
//   // int middlefront = frontCeilingStrip.getLength()/2;
//   // int middleback = frontCeilingStrip.getLength() + rightCeilingStrip.getLength()+ rearCeilingStrip.getLength()/2;
//   // int middleleft = ceilingStrip.getLength()-leftCeilingStrip.getLength()/2;
//   // int middleright = frontCeilingStrip.getLength()+rightCeilingStrip.getLength()/2;

//   int size = 9;
//   int speed = 11; //size should be more than speed
//   int offset = size + 2*speed; 

//   //front side
//   if(ledNewOn > 120){
//     chaseIndexFrontR.enqueue(middlefront);
//     chaseIndexFrontL.enqueue(middlefront);
//   }
//   //right side
//     for (int i = 0; i < chaseIndexFrontR.itemCount(); ++i) {
//       ledFrontR = chaseIndexFrontR.dequeue();
    

//       for(int i = (-size); i<=size ;++i){
//         ceilingStrip[ledFrontR+i] = CRGB::White;
        
//       }

//       //if wont go off end of strip, increment
//       if (ledFrontR <= middleright) { 
//         chaseIndexFrontR.enqueue(ledFrontR + speed);
      
//       }

  
//   }
//   //left side
//   for (int i = 0; i < chaseIndexFrontL.itemCount(); ++i) {
      
//       ledFrontL = chaseIndexFrontL.dequeue();

//       for(int i = (-size); i<=size ;++i){
//         ceilingStrip[ledFrontL+i] = CRGB::White;
//       }

//       //if wont go off end of strip, decrement

//       if(( ledFrontL>= middleleft) or (middlefront >= ledFrontL and ledFrontL>= 0+offset)){ 
//           chaseIndexFrontL.enqueue(ledFrontL - speed);
//       }else if(ledFrontL < 0+offset) {
//           chaseIndexFrontL.enqueue(ceilingStrip.getLength()-size-1);
//       }
//   }


//   //middle shooter
//   int numMiddleOn = ledNewOn/4-13;
//   for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
//         ceilingStrip[middlefront+i] = CRGB::White;
        
//       }


//   //backside
//   if(ledNewOn > 120){
//     chaseIndexBackR.enqueue(middleback);
//     chaseIndexBackL.enqueue(middleback);
//   }

//   //backleft
//     for (int i = 0; i < chaseIndexBackL.itemCount(); ++i) {
//       ledBackL = chaseIndexBackL.dequeue();
    

//       for(int i = (-size); i<=size ;++i){
//         ceilingStrip[ledBackL+i] = CRGB::White;
        
//       }

//       //if wont go off end of strip, increment
//       if (ledBackL <=middleleft) { 
//         chaseIndexBackL.enqueue(ledBackL + speed);
      
//       }

//   }

//   //backright
//   for (int i = 0; i < chaseIndexBackR.itemCount(); ++i) {
//       ledBackR = chaseIndexBackR.dequeue();
    

//       for(int i = (-size); i<=size ;++i){
//         ceilingStrip[ledBackR+i] = CRGB::White;
        
//       }

//       //if wont go off end of strip, increment
//       if (ledBackR >=middleright) { 
//         chaseIndexBackR.enqueue(ledBackR - speed);
      
//       }

//   }
//   //middle shooter
  
//   for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
//         ceilingStrip[middleback+i] = CRGB::White;
        
//       }


//  // FastLED.show();
// }

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

// void sexy(){

//   solid(CRGB::Red);

// }

void solid(CRGB color){
  // //turn everything off
  // for(int i=0;i<ceilingStrip.getLength();++i){
  //   ceilingStrip[i]=color;
  // }

  // for(int i=0;i<frontRightStrip.getLength();++i){
  //   frontRightStrip[i]=color;
  // }

  for(int i=0;i<frontLeftStrip.getLength();++i){
    frontLeftStrip[i]=color;
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


// void flicker(){
//   // solid(CRGB::Black);
//   // FastLED.show();


//   // solid(CRGB::White);
//   // delay(250);

//   // solid(CRGB::Black);
//   // solid(CRGB::White);
//   // solid(CRGB::Black);
// }


//   int meteorTip=0;
//   uint8_t hueTip=155;
//   int saturation=255;
//   int initialBrightness = 255;
//   int acc=0;

// void meteor(){

// trailLED ledToBeAdded1;
//  // solid(CRGB::Black);

// //  (meteorTip+2)<strip.getLength()
// //  while(true){
//     if (meteorTip>=ceilingStrip.getLength()-5){
//       meteorTip=0;
//     }
    
//     //make tip bright
//     ceilingStrip[meteorTip]=CHSV(hueTip,255,initialBrightness); // (hue,saturation, brightness)
//     ceilingStrip[meteorTip+1]=CHSV(hueTip,saturation,100);

    
//     //light trail
//     for (int i = 0; i < trailQueue.itemCount(); ++i) {
//       ledToBeAdded1 = trailQueue.dequeue();
    
//       ceilingStrip[ledToBeAdded1.index]=CHSV(ledToBeAdded1.color+rand()%30,ledToBeAdded1.saturation,ledToBeAdded1.brightness);
      
//       if(rand()%4==0){
//         ledToBeAdded1.brightness = (rand() % ledToBeAdded1.brightness + 1);
//       }
      
//       if (ledToBeAdded1.brightness>10){
//         trailQueue.enqueue(ledToBeAdded1);
//       }else{
//         ceilingStrip[ledToBeAdded1.index]=CRGB::Black;
//       }

//   }

//     //add tip to trail queue
//     ledToBeAdded1.index = meteorTip;
//     ledToBeAdded1.color = hueTip;
//     ledToBeAdded1.saturation = saturation;
//     ledToBeAdded1.brightness = initialBrightness;

//     trailQueue.enqueue(ledToBeAdded1);
//     ledToBeAdded1.index = meteorTip+1;
//     trailQueue.enqueue(ledToBeAdded1);

//     //sample = analogRead(microphonePin);
//     //Serial.println(peakToPeak);
//     //printf(sample);
    
//     meteorTip+=2;
  
// }


// void resetMeteorQueues(){
//     while(!trailQueue.isEmpty()){
//       trailQueue.dequeue();
//     }
// }



// //Fateen And Patty animation (FAP animation)

//   // int meteorTip=0; //
//   // uint8_t hueTip=150; //color of the trail
//   // int saturation=255;
//   // int initialBrightness = 255;

// void nightSky(){

//     trailLED ledToBeAdded;
//     int random=rand()%100;

//     if(random>80){
      
//       ledToBeAdded.index=rand()%ceilingStrip.getLength();
//       //ledToBeAdded.index=index;
//       ledToBeAdded.brightness=0;
//       ledToBeAdded.saturation=rand()%50;
//       ledToBeAdded.color=20;
//       brightStar.enqueue(ledToBeAdded);
//     }


//   //increases brightness
//     for(int i=0;i<brightStar.itemCount();++i){
//       ledToBeAdded = brightStar.dequeue();
//       if (ledToBeAdded.brightness<235)
//       {
//         ledToBeAdded.brightness+=rand()%15;
//         brightStar.enqueue(ledToBeAdded);
//         ceilingStrip[ledToBeAdded.index]= CHSV(ledToBeAdded.color, ledToBeAdded.saturation, ledToBeAdded.brightness);

//       }else{
//         dimStar.enqueue(ledToBeAdded);

//       }
      
//     }

//   //decreases brightness
//     for(int i=0;i<dimStar.itemCount();++i){
//       ledToBeAdded = dimStar.dequeue();
      

//       if (ledToBeAdded.brightness>10)
//       {
//         ledToBeAdded.brightness-=rand()%15;
//         dimStar.enqueue(ledToBeAdded);
//         ceilingStrip[ledToBeAdded.index] = CHSV(ledToBeAdded.color, ledToBeAdded.saturation, ledToBeAdded.brightness);
//       }else{
//         ceilingStrip[ledToBeAdded.index] = CRGB::Black;
//       }


//     }   

// }

// void fadein(VirtualStrip strip){
//   solid(CRGB::Black);
//   int bright=0;
//   while(bright<250){
//   for (int i=0; i<strip.getLength();++i){
//     strip[i]=CHSV(150,250,bright);
//   }
//   FastLED.show();
//   bright+=1;
//   }

//   while(bright>0){
//   for (int i=0; i<strip.getLength();++i){
//     strip[i]=CHSV(150,250,bright);
//   }
//   FastLED.show();
//   bright-=1;
//   }
// }

// void dashed(){
//   //alternative on animation
//   solid(CRGB::Black);

//   //Up from floors
//   int index=1;
//   int rearRightDifference =frontLeftStrip.getLength()- rearRightStrip.getLength();
//   int rearLeftDifference =frontLeftStrip.getLength()- rearLeftStrip.getLength();
//   CRGB curCol = CRGB::White;
//   while(index<frontLeftStrip.getLength()-1){
//     if((index/2)%2==1){
//       curCol = CRGB::White;
//     }else{
//       curCol = CRGB::Black;
//     }
//     if(index<frontLeftStrip.getLength()-1){
//       frontLeftStrip[index-1]=curCol;
//       frontLeftStrip[index]=curCol;
//       frontLeftStrip[index+1]=curCol;
//     }

//     if(index<frontRightStrip.getLength()-1){
//       frontRightStrip[index-1]=curCol;
//       frontRightStrip[index]=curCol;
//       frontRightStrip[index+1]=curCol;
//     }
    
//     if(index-rearRightDifference<rearRightStrip.getLength()-1 and index>rearRightDifference){
//       rearRightStrip[index-1-rearRightDifference]=curCol;
//       rearRightStrip[index-rearRightDifference]=curCol;
//       rearRightStrip[index+1-rearRightDifference]=curCol;
//     }
    
//     if(index-rearLeftDifference<rearLeftStrip.getLength()-1 and index>rearLeftDifference){
//       rearLeftStrip[index-1-rearLeftDifference]=curCol;
//       rearLeftStrip[index-rearLeftDifference]=curCol;
//       rearLeftStrip[index+1-rearLeftDifference]=curCol;
//     }
    
//     index+=2;
//     FastLED.show();
//   }



// //ceiling
// //ceiling
//   index=1;
//   while(index<rightCeilingStrip.getLength()/2){
//     if((index/2)%2==1){
//       curCol = CRGB::White;
//     }else{
//       curCol = CRGB::Black;
//     }

    
//     if(index<=frontCeilingStrip.getLength()/2){
//       frontCeilingStrip[index-1]=curCol;
//       frontCeilingStrip[index]=curCol;
//       frontCeilingStrip[index+1]=curCol;

//       frontCeilingStrip[frontCeilingStrip.getLength()-index-1]=curCol;
//       frontCeilingStrip[frontCeilingStrip.getLength()-index]=curCol;
//       frontCeilingStrip[frontCeilingStrip.getLength()-index+1]=curCol;
//     }

//     if(index<=rightCeilingStrip.getLength()/2){
//       rightCeilingStrip[index-1]=curCol;
//       rightCeilingStrip[index]=curCol;
//       rightCeilingStrip[index+1]=curCol;

//       rightCeilingStrip[rightCeilingStrip.getLength()-index-1]=curCol;
//       rightCeilingStrip[rightCeilingStrip.getLength()-index]=curCol;
//       rightCeilingStrip[rightCeilingStrip.getLength()-index+1]=curCol;
//     }
    
//     if(index<=leftCeilingStrip.getLength()/2){
//       leftCeilingStrip[index-1]=curCol;
//       leftCeilingStrip[index]=curCol;
//       leftCeilingStrip[index+1]=curCol;

//       leftCeilingStrip[leftCeilingStrip.getLength()-index-1]=curCol;
//       leftCeilingStrip[leftCeilingStrip.getLength()-index]=curCol;
//       leftCeilingStrip[leftCeilingStrip.getLength()-index+1]=curCol;
//     }
    
//     if(index<=rearCeilingStrip.getLength()/2+1){
//       rearCeilingStrip[index-1]=curCol;
//       rearCeilingStrip[index]=curCol;
//       rearCeilingStrip[index+1]=curCol;
      
//       rearCeilingStrip[rearCeilingStrip.getLength()-index-1]=curCol;
//       rearCeilingStrip[rearCeilingStrip.getLength()-index]=curCol;
//       rearCeilingStrip[rearCeilingStrip.getLength()-index+1]=curCol;

  
//     }
//     index+=2;
//     FastLED.show();
//   }

// }

// void fapOn(){
//   solid(CRGB::Black);

//   gHue=150;
//   int index=1;
//   int rearRightDifference =frontLeftStrip.getLength()- rearRightStrip.getLength();
//   int rearLeftDifference =frontLeftStrip.getLength()- rearLeftStrip.getLength();

//   //initial start up signal
//   // bool start = false;
//   // while(!start){
    
//   // solid(CRGB::Black);
//       //  **********************************************************************
//   //   Read from microphone
//   //  **********************************************************************
//   // unsigned long startMillis = millis(); // Start of sample window
//   // unsigned int peakToPeak = 0;   // peak-to-peak level

//   // unsigned int signalMax = 0;
//   // unsigned int signalMin = 1024;

//   // // collect data for sampleWindow mS
//   // while (millis() - startMillis < sampleWindow)
//   // {
//   //   sample = analogRead(microphonePin);
//   //   //    Serial.println(analogRead(microphonePin));
//   //   if (sample < 1024)  // Sanitize input
//   //   {
//   //     if (sample > signalMax) // Save just the max levels
//   //     {
//   //       signalMax = sample;
//   //     }
//   //     else if (sample < signalMin) // Save just the min levels
//   //     {
//   //       signalMin = sample;
//   //     }
//   //   }
//   // }
//   // peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  
//   // int numMiddleOn = peakToPeak/15-1;
//   // if(numMiddleOn>25){
//   //     start=true;
//   //     break;
//   //   }
//   // for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
//   //       rearCeilingStrip[rearCeilingStrip.getLength()/2+i] = CRGB::BlueViolet;
        
//   //     }
//   //   FastLED.show();
    
    
//   // }
// int count=0;
// int decrementIndex = 1;

// int middleBack = frontCeilingStrip.getLength()+rightCeilingStrip.getLength()+rearCeilingStrip.getLength()/2;
// int speed = 2;
// int incrementIndex = speed/2;
// while (count<(leftCeilingStrip.getLength()+rearCeilingStrip.getLength()/2)-10)
// { 
//   if((middleBack-decrementIndex-speed/2) < frontCeilingStrip.getLength()){
//     for(int i = incrementIndex-speed/2-15; i <= incrementIndex+speed/2-15; ++i){
//       frontRightStrip[frontRightStrip.getLength()-i]=CHSV(250,255,255);
//     }
// }

// else{
//   for(int i = middleBack-decrementIndex-speed/2; i <= middleBack-decrementIndex+speed/2; ++i){
//     ceilingStrip[i]=CHSV(150,255,255);
    
//   }
// }

//   if(decrementIndex+speed/2+middleBack < ceilingStrip.getLength()-1){
//   for(int i = middleBack+decrementIndex-speed/2; i <= middleBack+decrementIndex+speed/2; ++i){
//       ceilingStrip[i]=CHSV(10,255,255);
//     }
//    }
  
//   else{
    
//     for(int i = incrementIndex-speed/2; i <= incrementIndex+speed/2; ++i){
//       if(frontLeftStrip.getLength()-i<=0)
//       return;
//       frontLeftStrip[frontLeftStrip.getLength()-i]=CHSV(250,255,255);
//     }
//     incrementIndex+=speed;
//   }
  

//   count++;
//   decrementIndex+=speed;
  
//   FastLED.show();

// }



 

  
  

// }

// int progress=0;
// int front=0;

// void seconds(){
//   int sec=frontCeilingStrip.getLength()/60;

//   for (int i=0; i<sec;i++){
//     frontCeilingStrip[front+i]=CHSV(100,255,255);
//   }
//   front+=sec;
// }

// void pomodoro(){

  
//   int sec=frontCeilingStrip.getLength()/60;

//   for (int i=0; i<sec;i++){
//     frontCeilingStrip[front+i]=CHSV(100,255,255);

//   }
//   front+=sec;

//   for (int i=0; i<5;++i){
//   rightCeilingStrip[progress+i]=CHSV(250,255,255);
//   }
//   progress+=5;
// }





// //functiom to make the lights go off
// void lightsOff(){
//   solid(CRGB::Black);
//   FastLED.show();
// } 





// //function to change the light colors to a random color
// void randomColor(){
//   for(int i=0; i<frontCeilingStrip.getLength();++i){
//     frontCeilingStrip[i]=CHSV(random(0,255),255,255);
//   }
//   for(int i=0; i<frontLeftStrip.getLength();++i){
//     frontLeftStrip[i]=CHSV(random(0,255),255,255);
//   }
//   for(int i=0; i<frontRightStrip.getLength();++i){
//     frontRightStrip[i]=CHSV(random(0,255),255,255);
//   }
//   for(int i=0; i<rearCeilingStrip.getLength();++i){
//     rearCeilingStrip[i]=CHSV(random(0,255),255,255);
//   }
//   for(int i=0; i<rearLeftStrip.getLength();++i){
//     rearLeftStrip[i]=CHSV(random(0,255),255,255);
//   }
//   for(int i=0; i<rearRightStrip.getLength();++i){
//     rearRightStrip[i]=CHSV(random(0,255),255,255);
//   }
//   for(int i=0; i<leftCeilingStrip.getLength();++i){
//     leftCeilingStrip[i]=CHSV(random(0,255),255,255);
//   }
//   for(int i=0; i<rightCeilingStrip.getLength();++i){
//     rightCeilingStrip[i]=CHSV(random(0,255),255,255);
//   }
//   for(int i=0; i<ceilingStrip.getLength();++i){
//     ceilingStrip[i]=CHSV(random(0,255),255,255);
//   }

//   //function to change the color of the lights
//   FastLED.show();
// }




//  //function to make the lights closing animation
// void opening(){
//   int count=0;
//   int decrementIndex = 1;
//   int middleBack = frontCeilingStrip.getLength()+rightCeilingStrip.getLength()+rearCeilingStrip.getLength()/2;
//   int speed = 2;
//   int incrementIndex = speed/2;
//   while (count<(leftCeilingStrip.getLength()+rearCeilingStrip.getLength()/2)-10)
//   { 
//     if((middleBack-decrementIndex-speed/2) < frontCeilingStrip.getLength()){
//       for(int i = incrementIndex-speed/2-15; i <= incrementIndex+speed/2-15; ++i){
//         frontRightStrip[frontRightStrip.getLength()-i]=CHSV(250,255,255);
//       }
//     }
  
//     else{
//       for(int i = middleBack-decrementIndex-speed/2; i <= middleBack-decrementIndex+speed/2; ++i){
//         ceilingStrip[i]=CHSV(150,255,255);
        
//       }
//     }
  
//     if(decrementIndex+speed/2+middleBack < ceilingStrip.getLength()-1){
//       for(int i = middleBack+decrementIndex-speed/2; i <= middleBack+decrementIndex+speed/2; ++i){
//         ceilingStrip[i]=CHSV(10,255,255);
//       }
//      }
    
//     else{
      
//       for(int i = incrementIndex-speed/2; i <= incrementIndex+speed/2; ++i){
//         if(frontLeftStrip.getLength()-i<=0)
//         return;
//         frontLeftStrip[frontLeftStrip.getLength()-i]=CHSV(250,255,255);
//       }
//     }
//   }
// }

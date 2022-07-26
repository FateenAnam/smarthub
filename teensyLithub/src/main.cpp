#include "FastLED.h"

// #include <OctoWS2811.h>

// #include <Adafruit_NeoPixel.h>
// #ifdef __AVR__
//  #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
// #endif

#include <ArduinoQueue.h>
#include <VirtualStrip.h>
#include <FastLED.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <usb_audio.h>

#include<time.h>
#include<stdio.h>


#define DATA_PIN_D 3
#define DATA_PIN_C 4
#define DATA_PIN_B 5
#define DATA_PIN_A 6
#define DATA_PIN_V 7

#define NUM_LEDS 258   //Variable
#define LED_PIN 1     //Variable
CRGB leds[NUM_LEDS];
#define DATA_PIN_1 LED_PIN //LED_PIN
#define BRIGHTNESS bright   
int bright = 50; //variable
 

int ledNewOn = NUM_LEDS; // How many LEDS to turn on
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
int microphonePin = A5;    // select the input pin for the potentiometer   //variable
unsigned int sample;


//UPDATE THESE VARIABLES
/////////////////
int Front_Left_Bottom = 0;
int Front_Left_Top = 100;
 int Front_Right_Bottom = NUM_LEDS;
 int Front_Right_Top = 158;
CRGB leds_1[NUM_LEDS]; // the main LED

// Define volume bars
VirtualStrip frontLeftStrip(leds, Front_Left_Bottom, Front_Left_Top-Front_Left_Bottom-3);
VirtualStrip frontRightStrip(leds, Front_Right_Top,Front_Right_Bottom-Front_Right_Top-3, true);
// Define ceiling bars
VirtualStrip frontCeilingStrip(leds, Front_Left_Top, Front_Right_Top-Front_Left_Top-3);
// Master strip
VirtualStrip masterStrip(leds, Front_Left_Bottom, NUM_LEDS-3);  


//chasing variables
//front side
int ledFrontR = 0; //right side index
int ledFrontL = 0; //left side index
ArduinoQueue<int> chaseIndexFrontR(1800); //Q to go right
ArduinoQueue<int> chaseIndexFrontL(1800); //Q to go left

//meteor variales
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


void on();
void solid(CRGB color);
void movingRainbow();
void reactive();
void nightSky();
void meteor();

void setup() {
  Serial.begin(9600);
  
  FastLED.addLeds<WS2811, LED_PIN, GRB> (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(12, 2000);   //Why?????

  // pinMode(DATA_PIN_A, INPUT);
  // pinMode(DATA_PIN_B, INPUT);
  // pinMode(DATA_PIN_C, INPUT);
  // pinMode(DATA_PIN_D, INPUT);
  // pinMode(DATA_PIN_V, INPUT);
}

int startTime = millis();
int cycle =60;  //VARIABLE 
int fullTurn= 0;



int stateA=0;
int stateB=0;
int stateC=0;
int stateD=0;

void loop() {
  //movingRainbow();


// reactive();
// Serial.println(analogRead(A5));

//solid(CRGB::Red);
//on();

 //Meteor with Night Sky. Must follow this order and add delay
  if(((millis()/1000)%cycle)<cycle/2){  //VARIABLE
  solid(CRGB::Black);
  nightSky();  //recommended delay 100
  meteor();  //must add delay of >20
 delay(30);  
  }
  else{
    reactive();
  }
//delay(300);
FastLED.show(); 
}

float meteorTip=0;
  uint8_t hueTip=155;
  int saturation=255;
  int initialBrightness = 255;
  int acc=0;

void meteor(){

 trailLED ledToBeAdded1;

    if (floor(meteorTip)>=masterStrip.getLength()-5){
      meteorTip=0;
      fullTurn++;
    }
    
    //make tip bright
    masterStrip[floor(meteorTip)]=CHSV(hueTip,255,initialBrightness); // (hue,saturation, brightness)
    masterStrip[floor(meteorTip+1)]=CHSV(hueTip,saturation,255);
    masterStrip[floor(meteorTip+2)]=CHSV(hueTip,saturation,255);
    masterStrip[floor(meteorTip+3)]=CHSV(hueTip,saturation,255);

    //light trail
    for (int i = 0; i < trailQueue.itemCount(); ++i) {
      ledToBeAdded1 = trailQueue.dequeue();
    
      masterStrip[ledToBeAdded1.index]=CHSV(ledToBeAdded1.color+rand()%30,ledToBeAdded1.saturation,ledToBeAdded1.brightness);
      
      if(rand()%4==0){
        ledToBeAdded1.brightness = (rand() % ledToBeAdded1.brightness + 1);
      }
      
      if (ledToBeAdded1.brightness>10){   //Variable
        trailQueue.enqueue(ledToBeAdded1);
      }else{
        masterStrip[ledToBeAdded1.index]=CRGB::Black;
      }
  }

    //add tip to trail queue
    ledToBeAdded1.index = floor(meteorTip);
    ledToBeAdded1.color = hueTip;
    ledToBeAdded1.saturation = saturation;
    ledToBeAdded1.brightness = initialBrightness;

    trailQueue.enqueue(ledToBeAdded1);
    ledToBeAdded1.index = floor(meteorTip)+1;
    trailQueue.enqueue(ledToBeAdded1);

    //sample = analogRead(microphonePin);
    //Serial.println(peakToPeak);
    //printf(sample);
    
    meteorTip+=0.45;
  

}

void on(){

  solid(CRGB::Black);

  //On animation

  
  //Up from floors
  int index=1;
  // int rearRightDifference =frontLeftStrip.getLength()- rearRightStrip.getLength();
  // int rearLeftDifference =frontLeftStrip.getLength()- rearLeftStrip.getLength();
  while(index<masterStrip.getLength()){

    if(index<frontLeftStrip.getLength()){
      frontLeftStrip[index-1]=CHSV(gHue, 255, 255);
      frontLeftStrip[index]=CHSV(gHue, 255, 255);
      frontLeftStrip[index+1]=CHSV(gHue, 255, 255);
    }

    if(index<frontRightStrip.getLength()){
      frontRightStrip[index-1]=CHSV(gHue, 255, 255);
      frontRightStrip[index]=CHSV(gHue, 255, 255);
      frontRightStrip[index+1]=CHSV(gHue, 255, 255);
    }

    if(index<frontCeilingStrip.getLength()){
      frontCeilingStrip[index-1]=CHSV(gHue, 255, 255);
      frontCeilingStrip[index]=CHSV(gHue, 255, 255);
      frontCeilingStrip[index+1]=CHSV(gHue, 255, 255);
    }
    
    // if(index-rearRightDifference<rearRightStrip.getLength()-1 and index>rearRightDifference){
    //   rearRightStrip[index-1-rearRightDifference]=CHSV(gHue, 255, 255);
    //   rearRightStrip[index-rearRightDifference]=CHSV(gHue, 255, 255);
    //   rearRightStrip[index+1-rearRightDifference]=CHSV(gHue, 255, 255);
    // }
    
    // if(index-rearLeftDifference<rearLeftStrip.getLength()-1 and index>rearLeftDifference){
    //   rearLeftStrip[index-1-rearLeftDifference]=CHSV(gHue, 255, 255);
    //   rearLeftStrip[index-rearLeftDifference]=CHSV(gHue, 255, 255);
    //   rearLeftStrip[index+1-rearLeftDifference]=CHSV(gHue, 255, 255);
    // }
    

    index+=3;
    gHue+=15;  //default 3
    FastLED.show();
    delay(50);
  }


  //ceiling
  // index=1;
  // while(index<rightCeilingStrip.getLength()){

  //   if(index<frontCeilingStrip.getLength()-1){
  //     frontCeilingStrip[index-1]=CHSV(gHue, 255, 255);
  //     frontCeilingStrip[index]=CHSV(gHue, 255, 255);
  //     frontCeilingStrip[index+1]=CHSV(gHue, 255, 255);
  //   }

  //   if(index<rightCeilingStrip.getLength()-1){
  //     rightCeilingStrip[index-1]=CHSV(gHue, 255, 255);
  //     rightCeilingStrip[index]=CHSV(gHue, 255, 255);
  //     rightCeilingStrip[index+1]=CHSV(gHue, 255, 255);
  //   }
    
  //   if(index<leftCeilingStrip.getLength()-1){
  //     leftCeilingStrip[index-1]=CHSV(gHue, 255, 255);
  //     leftCeilingStrip[index]=CHSV(gHue, 255, 255);
  //     leftCeilingStrip[index+1]=CHSV(gHue, 255, 255);
  //   }
    
  //   if(index<rearCeilingStrip.getLength()-1){
  //     rearCeilingStrip[index-1]=CHSV(gHue, 255, 255);
  //     rearCeilingStrip[index]=CHSV(gHue, 255, 255);
  //     rearCeilingStrip[index+1]=CHSV(gHue, 255, 255);
  //   }

  //   index+=3;
  //   gHue+=3;
  //   FastLED.show();
  // }

  // //ceiling go to constant hue
  // gHue=0;
  // int decrementIndex = frontLeftStrip.getLength()-1;
  // int incrementIndex = 1;
  // index=1;
  // while(incrementIndex<rightCeilingStrip.getLength()-1){
  //   index = incrementIndex;
  //   if(index<frontCeilingStrip.getLength()-1){
  //     frontCeilingStrip[index-1]=CHSV(gHue, 255, 255);
  //     frontCeilingStrip[index]=CHSV(gHue, 255, 255);
  //     frontCeilingStrip[index+1]=CHSV(gHue, 255, 255);
  //   }

  //   if(index<rightCeilingStrip.getLength()-1){
  //     rightCeilingStrip[index-1]=CHSV(gHue, 255, 255);
  //     rightCeilingStrip[index]=CHSV(gHue, 255, 255);
  //     rightCeilingStrip[index+1]=CHSV(gHue, 255, 255);
  //   }
    
  //   if(index<leftCeilingStrip.getLength()-1){
  //     leftCeilingStrip[index-1]=CHSV(gHue, 255, 255);
  //     leftCeilingStrip[index]=CHSV(gHue, 255, 255);
  //     leftCeilingStrip[index+1]=CHSV(gHue, 255, 255);
  //   }
    
  //   if(index<rearCeilingStrip.getLength()-1){
  //     rearCeilingStrip[index-1]=CHSV(gHue, 255, 255);
  //     rearCeilingStrip[index]=CHSV(gHue, 255, 255);
  //     rearCeilingStrip[index+1]=CHSV(gHue, 255, 255);
  //   }
    

  //   //push down to reactive offset
  //   index=decrementIndex;
  //   if(index>50){
  //     frontLeftStrip[index-1]=CRGB::Black;
  //     frontLeftStrip[index]=CRGB::Black;
  //     frontLeftStrip[index+1]=CRGB::Black;
  //   }

  //   if(index>50){
  //     frontRightStrip[index-1]=CRGB::Black;
  //     frontRightStrip[index]=CRGB::Black;
  //     frontRightStrip[index+1]=CRGB::Black;
  //   }
    
  //   if(index - rearRightDifference>50){
  //     rearRightStrip[index-1-rearRightDifference]=CRGB::Black;
  //     rearRightStrip[index-rearRightDifference]=CRGB::Black;
  //     rearRightStrip[index+1-rearRightDifference]=CRGB::Black;
  //   }
    
  //    if(index - rearLeftDifference> 2){
  //     rearLeftStrip[index-1-rearLeftDifference-1]=CRGB::Black;
  //     rearLeftStrip[index-rearLeftDifference-1]=CRGB::Black;
  //     rearLeftStrip[index+1-rearLeftDifference-1]=CRGB::Black;
  //   }

    
  //   incrementIndex+=3;
  //   decrementIndex-=1;
  //   FastLED.show();
  // }

// delay(3000);
}

void nightSky(){

    trailLED ledToBeAdded;
    int random=rand()%100;

    if(random>80){
      
      ledToBeAdded.index=rand()%masterStrip.getLength();
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
        masterStrip[ledToBeAdded.index]= CHSV(ledToBeAdded.color, ledToBeAdded.saturation, ledToBeAdded.brightness);

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
        masterStrip[ledToBeAdded.index] = CHSV(ledToBeAdded.color, ledToBeAdded.saturation, ledToBeAdded.brightness);
      }else{
        masterStrip[ledToBeAdded.index] = CRGB::Black;
      }


    }   

}




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

//Variable
int sensitivity = 1;

//Calculate Volume Bar New LEDS
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  
  double volts = (peakToPeak * 5.0) / 1024;  // convert to volts

  ledNewOn = ceil(volts * 10)+10;     //VARIABLE
  // Cut-off at NUM_LEDS and Left Strip
  if (ledNewOn > NUM_LEDS) {
    ledNewOn = NUM_LEDS;
  }
  if (ledNewOn > Front_Left_Top) {
    ledNewOn = Front_Left_Top-5;
  }
   
  Serial.println(ledNewOn);

  for (int i = 0; i < NUM_LEDS; ++i) {
    masterStrip[i] = CRGB::Black;
    // Other LED Strips go here 
    // leds_2[i] = CRGB::Black;
  }

int volumeBarColor = 2;  //Variable 

  for (int i = 0; i < ledNewOn; ++i) {
     frontLeftStrip[i] = CHSV((i*volumeBarColor + gHue), 255, 255);     //VARIABLE constants multiplied
     frontRightStrip[i] = CHSV(i*volumeBarColor + gHue, 255, 255);      //VARIABLE constants multiplied
   }

  // Ceiling Rainbow Bar
  for (int i = 0; i < frontCeilingStrip.getLength()-4; ++i) {   
    frontCeilingStrip[i] = CHSV(gHue+i*7, 255, 255);       //VARIABLE constants multiplied
  }

  // Shift color;
  gHue = gHue + 1;
  int chaserTrigger = 35;         //VARIABLE   

  // Color shift on bass drop
  if (millis() - lastColorChange > 1000 && ledNewOn > chaserTrigger) {
    lastColorChange = millis();
    gHue += 100;
  }

  //chasing 
  int middlefront = Front_Left_Top+ frontCeilingStrip.getLength()/2-4;  //VARIABLE 
   int middleleft = frontLeftStrip.getLength()/2; //VARIABLE
   int middleright = Front_Right_Top+ frontRightStrip.getLength()/2; //VARIABLE 

  int size = 2;   //used to be 9                //VARIABLE 
  int speed = 3; //size should be more than speed           //VARIABLE 
  int offset = size + 2*speed;        //VARIABLE 


  //front side 
  if(ledNewOn > chaserTrigger){  //This value Triggers the Chasers  //Variable
    chaseIndexFrontR.enqueue(middlefront);
    chaseIndexFrontL.enqueue(middlefront);
  }


//chasers
  // //right side
    for (int i = 0; i < chaseIndexFrontR.itemCount(); ++i) {
      ledFrontR = chaseIndexFrontR.dequeue();
    
      for(int i = (-size); i<=size ;++i){
        masterStrip[ledFrontR+i] = CRGB::White;
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
        masterStrip[ledFrontL+i] = CRGB::White;
      }

      //if wont go off end of strip, decrement
      if(( ledFrontL>= middleleft) or (middlefront >= ledFrontL and ledFrontL>= 0+offset)){ 
          chaseIndexFrontL.enqueue(ledFrontL - speed);
      }
      // else if(ledFrontL < 0+offset) {
      //     chaseIndexFrontL.enqueue(masterStrip.getLength()-size-1);
      // }
   }

  //middle shooter
  int numMiddleOn = ledNewOn/4;     //Variable
  for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
        masterStrip[middlefront+i+2] = CRGB::White;
  }
  FastLED.show();
}




void solid(CRGB color){
  //turn everything off
  for(int i=0;i<masterStrip.getLength();++i){
    masterStrip[i]=color;
  }

  for(int i=0;i<frontRightStrip.getLength();++i){
    frontRightStrip[i]=color;
  }

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

void solidfade(int color, int saturation, int brightness){
  //turn everything off
  for(int i=0;i<masterStrip.getLength();++i){
    masterStrip[i]=CHSV(color,saturation,brightness);
  }

  for(int i=0;i<frontRightStrip.getLength();++i){
    frontRightStrip[i]=CHSV(color,saturation,brightness);
  }

  for(int i=0;i<frontLeftStrip.getLength();++i){
    frontLeftStrip[i]=CHSV(color,saturation,brightness);
  }

  // for(int i=0;i<rearLeftStrip.getLength();++i){
  //   rearLeftStrip[i]=CHSV(color,saturation,brightness);
  // }

  //  for(int i=0;i<rearRightStrip.getLength();++i){
  //   rearRightStrip[i]=CHSV(color,saturation,brightness);
  // }

  // for(int i=0;i<rearLeftStrip.getLength();++i){
  //   rearLeftStrip[i]=CHSV(color,saturation,brightness);
  // }
}

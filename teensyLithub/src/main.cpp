#include "FastLED.h"
#include <ArduinoQueue.h>
#include <VirtualStrip.h>
#include <Wire.h>
#include <SPI.h>
#include <SPI.h> 
#include<time.h>
#include<stdio.h>

#define DATA_PIN_D 3
#define DATA_PIN_C 4
#define DATA_PIN_B 5
#define DATA_PIN_A 6
#define DATA_PIN_V 7
// #include <Adafruit_NeoPixel.h>
// #ifdef __AVR__
//  #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
// #endif

//#include <Audio.h>
//#include <SD.h>
//#include <SerialFlash.h>
//#include <usb_audio.h>
//#include <SD.h>




#define NUM_LEDS 258   //Variable
#define BOX_NUM_LEDS 37
#define LED_PIN 1     //Variable
#define LED_BOX_PIN 9
//declare strip 
CRGB leds[NUM_LEDS];
CRGB box[BOX_NUM_LEDS];
#define DATA_PIN_1 LED_PIN //LED_PIN

int bright = 50; //variable
#define BRIGHTNESS bright    
int ledNewOn = NUM_LEDS; // How many LEDS to turn on     
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
int microphonePin = A5;    // select the input pin for the potentiometer   //variable
unsigned int sample;


//Update these variables depending on how you want to set up the strip
/////////////////
int strip_start = 0;
int strip_top_left_corner = 97;
 int strip_end = NUM_LEDS;
 int strip_top_right_corner = 155;
CRGB led_strip[NUM_LEDS]; // the main LED


//mode declarations
void on();
void solid(CRGB color);
void boxWhite();
void movingRainbow();
void reactive();
void nightSky();
void meteor();
void pomodoro();
void theaterChase(CRGB color, int wait);
void boxChase(CRGB color, int wait);

//chasing variables
//front side
int ledFrontR = 0; //right side index
int ledFrontL = 0; //left side index
ArduinoQueue<int> chaseIndexFrontR(1800); //Q to go right
ArduinoQueue<int> chaseIndexFrontL(1800); //Q to go left


// Define volume bars
VirtualStrip left_strip(leds, 0, strip_top_left_corner-strip_start);
VirtualStrip right_strip(leds, strip_top_right_corner+1,strip_end-strip_top_right_corner-2, true);
// Define ceiling bars
VirtualStrip top_strip(leds, strip_top_left_corner+3, strip_top_right_corner-strip_top_left_corner-2);
// Master strip
VirtualStrip masterStrip(leds, 0, NUM_LEDS);  
// Box strip
VirtualStrip box_strip(box, 0, BOX_NUM_LEDS);

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
 
void setup() {
  Serial.begin(9600);
  
  FastLED.addLeds<WS2811, LED_PIN, BRG> (leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2811, LED_BOX_PIN, BRG> (box, BOX_NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(12, 2000);

  pinMode(DATA_PIN_A, INPUT);
  pinMode(DATA_PIN_B, INPUT);
  pinMode(DATA_PIN_C, INPUT);
  pinMode(DATA_PIN_D, INPUT);
  pinMode(DATA_PIN_V, INPUT);
}

int startTime = millis();
int cycle =60;  //VARIABLE 
int fullTurn= 0;



int stateA=0;
int stateB=0;
int stateC=0;
int stateD=0;


void loop() {
//uncomment function for mode
//theaterChase(CRGB::LightCoral, 10);
//solid(CRGB::Blue);
//movingRainbow();
//on();
reactive();
//nightSky();
//meteor();
//pomodoro();
//boxChase(CRGB::LightCoral, 10);
boxWhite();

//----------------------------------------------------------------
//Meteor with Night Sky. Must follow this order and add delay
  // if(((millis()/1000)%cycle)<cycle/2){  //VARIABLE
  // solid(CRGB::Black);
  // nightSky();  //recommended delay 100
  // meteor();  //must add delay of >20
  // delay(25);
  // }
  // else{
  //   reactive();
  // }
//----------------------------------------------------------------

  FastLED.show();
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
    if (meteorTip>=masterStrip.getLength()-5){
      meteorTip=0;
    }
    
    //make tip bright
    masterStrip[meteorTip]=CHSV(hueTip,255,initialBrightness); // (hue,saturation, brightness)
    masterStrip[meteorTip+1]=CHSV(hueTip,saturation,100);

    
    //light trail
    for (int i = 0; i < trailQueue.itemCount(); ++i) {
      ledToBeAdded1 = trailQueue.dequeue();
    
      masterStrip[ledToBeAdded1.index]=CHSV(ledToBeAdded1.color+rand()%30,ledToBeAdded1.saturation,ledToBeAdded1.brightness);
      
      if(rand()%4==0){
        ledToBeAdded1.brightness = (rand() % ledToBeAdded1.brightness + 1);
      }
      
      if (ledToBeAdded1.brightness>10){
        trailQueue.enqueue(ledToBeAdded1);
      }else{
        masterStrip[ledToBeAdded1.index]=CRGB::Black;
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

void on(){

  solid(CRGB::Black);
  //startup animation
  
  //Up from floors
  int index=1;
  while(index<left_strip.getLength()-1){

    if(index<left_strip.getLength()-1){
      left_strip[index-1]=CHSV(gHue, 255, 255);
      left_strip[index]=CHSV(gHue, 255, 255);
      left_strip[index+1]=CHSV(gHue, 255, 255);
    }

    if(index<right_strip.getLength()-1){
      right_strip[index-1]=CHSV(gHue, 255, 255);
      right_strip[index]=CHSV(gHue, 255, 255);
      right_strip[index+1]=CHSV(gHue, 255, 255);
    }

    index+=3;
    gHue+=3;
    FastLED.show();
  }
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



//Calculate Volume Bar New LEDS
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude

  double volts = (peakToPeak * 5.0) / 1024;  // convert to volts

  ledNewOn = ceil(volts * 37.5);     //VARIABLE
  // Cut-off at NUM_LEDS and Left Strip
  if (ledNewOn > NUM_LEDS) {
    ledNewOn = NUM_LEDS;
  }
  if (ledNewOn > strip_top_left_corner) {
    ledNewOn = strip_top_left_corner-5;
  }
   Serial.println(ledNewOn);



  for (int i = 0; i < NUM_LEDS; ++i) {
    masterStrip[i] = CRGB::Black;
  }

  for (int i = 0; i < ledNewOn; ++i) {
     left_strip[i] = CHSV((i*6 + gHue), 255, 255);     //VARIABLE constants multiplied
     right_strip[i] = CHSV(i*6 + gHue, 255, 255);      //VARIABLE constants multiplied
   }


  // Ceiling Rainbow Bar
  for (int i = 0; i < top_strip.getLength()-4; ++i) {   
    top_strip[i] = CHSV(gHue+i*7, 255, 255);       //VARIABLE constants multiplied
  }

  // Shift color;
  gHue = gHue + 1;

  // Color shift on bass drop
  if (millis() - lastColorChange > 1000 && ledNewOn > 140) {
    lastColorChange = millis();
    gHue += 100;
  }

  //chasing 
  int middlefront = strip_top_left_corner+ top_strip.getLength()/2 - 2;           //VARIABLE 
  int middleleft = left_strip.getLength()/2;                                  //VARIABLE
  int middleright = strip_top_right_corner+ right_strip.getLength()/2;        //VARIABLE 

  int size = 2;   //used to be 9                                              //VARIABLE 
  int speed = 3; //size should be more than speed                             //VARIABLE 
  int offset = size + 2*speed;                                                //VARIABLE 

int chaserTrigger = 90;                                                       //VARIABLE 
  //front side 
  if(ledNewOn > chaserTrigger){  //This value Triggers the Chasers            //VARIABLE
    chaseIndexFrontR.enqueue(middlefront);
    chaseIndexFrontL.enqueue(middlefront);
  }

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
  int numMiddleOn = ledNewOn/7;                                               //VARIABLE
  for(int i = (-numMiddleOn); i<=numMiddleOn ;++i){
        masterStrip[middlefront+i+2] = CRGB::White;
  } 
}

void solid(CRGB color){
  //turn everything off
  for(int i=0;i<masterStrip.getLength();++i){
    masterStrip[i]=color;
  }

  for(int i=0;i<right_strip.getLength();++i){
    right_strip[i]=color;
  }

  for(int i=0;i<left_strip.getLength();++i){
    left_strip[i]=color;
  }
}

void solidfade(int color, int saturation, int brightness){

  //turn everything off
  for(int i=0;i<masterStrip.getLength();++i){
    masterStrip[i]=CHSV(color,saturation,brightness);
  }

  for(int i=0;i<right_strip.getLength();++i){
    right_strip[i]=CHSV(color,saturation,brightness);
  }

  for(int i=0;i<left_strip.getLength();++i){
    left_strip[i]=CHSV(color,saturation,brightness);
  }
}

void pomodoro() {
  int count = 0;
  int studyTime = 5; //Variable 
  int visual_left = strip_top_left_corner/studyTime;
  int visual_right = strip_top_right_corner/studyTime;
  int increment = visual_left;
  for(int i = 0; i < studyTime; i++) {
    for(int i = 0; i < visual_left; i++) {
      left_strip[i] = CRGB::Green;
      right_strip[i] = CRGB::Green;
      
      FastLED.show();

    } 
    
    visual_left+= increment;
    delay(60000);
  }
  
    theaterChase(CRGB::Red, 50);
    solid(CRGB::Black);
    FastLED.show();
    delay(300000);  //5 minute break
    count = 0; 
    visual_left = increment;
  
}

void theaterChase(CRGB color, int wait) {
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      FastLED.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<NUM_LEDS; c += 3) {
        leds[c] = color; // Set pixel 'c' to value 'color'
      }
      FastLED.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

void boxChase(CRGB color, int wait) {
    for(int a=0; a<30; a++) {  // Repeat 30 times...
        for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
          FastLED.clear();         //   Set all pixels in RAM to 0 (off)
          // 'c' counts up from 'b' to end of strip in steps of 3...
          for(int c=b; c<BOX_NUM_LEDS; c += 3) {
            box[c] = color; // Set pixel 'c' to value 'color'
          }
          FastLED.show(); // Update strip with new contents
          delay(wait);  // Pause for a moment
        }
      }
}

void boxWhite() {
  // int hue = 100;
  for(int i=0;i<box_strip.getLength();++i){
    box_strip[i]= CHSV(100, 0, 255);
  }
}

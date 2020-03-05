
/* 
Source code for Project Halo - a lamp designed in the course Tangible Interaction.
Uses three components:
- SOFTPOT: For sensing movement throughout the ring
https://learn.sparkfun.com/tutorials/softpot-hookup-guide/all
1) Set timer
  - Map to LED LIGHTS (2)

- LED STRIP: Lights
https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use

1) Startup
  - Indicate that it is "ready" by lighting up all individual lights sequentially in different colors.
  - Pulsate
  - Wait
  - Turn them off

2) Timer set
  - Map softpot values to lights, lighting up all the lights in that range
  - When user stops pushing softpot, wait a delay in case of a slip
  - Pulsate the enabled lights, maybe in different colors or something like that
  - Start timer

3) Timer finished
   - Light up all lights and pulsate them to indiciate that the timer is out

- Distance Sensor: To sense turn on / turn off
https://create.arduino.cc/projecthub/jenniferchen/distance-measuring-sensor-900520

1) Wait for startup
 - Wait for the value to pass the threshold value
 - Pass to LED STRIPS (1)
2) Wait for turn off
  - Turn off all LED STRIPS lights


*/

// COMPONENT SETUP //

// -- LED STRIPS
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#include "FastLED.h"


// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 31

int sessions[100];
int sessionActive = 0;
int lightsInSession;

CRGB leds[LED_COUNT];



// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int lightsOn = 0;

const uint32_t red = strip.Color(255,   0,   0);


// -- SOFTPOT 
const int SOFT_POT_PIN = A0; // Pin connected to softpot wiper, used for setting timer

const int GRAPH_LENGTH = 40; // Length of line graph

const int SOFT_POT_PIN_2 = A2; // Pin connected to the second softpot wiper, used for changing colors


// -- IR Distance Sensor
int distancePin = A1;
int distanceVal = 0;

// if above val + threshold, we light it up
const int THRESHOLD = 100;
//roughly the default measuring value
const int STANDARD_VAL = 330;


void setup() 
{
  Serial.begin(19200);
  pinMode(SOFT_POT_PIN, INPUT);

  

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  pinMode(distancePin, INPUT);

  startUpLights();
}

void loop() 
{
  // Read in the soft pot's ADC value
  int softPotADC = analogRead(SOFT_POT_PIN);
  int softPotADC_2 = analogRead(SOFT_POT_PIN_2);
  // Map the 0-1023 value to 0-40
  int softPotPosition = map(softPotADC, 0, 1023, 0, GRAPH_LENGTH);
  int softPotPosition_2 = map(softPotADC_2, 0, 1023, 0, GRAPH_LENGTH);

  if(lightsOn == 1){
    fillLights(softPotADC, softPotADC_2);
  }

  // Print a line graph:
  Serial.print("<"); // Starting end
  for (int i=0; i<GRAPH_LENGTH; i++)
  {
    if (i == softPotPosition) Serial.print("|");
    else Serial.print("-");
  }
  Serial.println("> (" + String(softPotADC) + ")");


  //delay(500);

  senseOnOff();
}


void fillLights(int softpotValue, int softpotValue2){
  int combinedValue = softpotValue + softpotValue2;
   
  //softpot value is 0-1023
  //ledcount
  int threshold = 1023 * 2 / LED_COUNT;

  // calc number of lights to turn on
  int lights = combinedValue / threshold;
  lightsInSession = lights;
  
  for (int i=0; i <= lights; i++){
    strip.setPixelColor(i, red);
    strip.show();
  }

 
}

int findLastIndex(){
  for(int i = 0; i < sizeof(sessions); i++){
    if (sessions[i] == 0){
      return i;
    }
  }

  return 101;
}


void startSession(int lights){
  int minutes = lights * 2;
  int seconds = minutes * 60;

  int timeLeft = seconds;

  sessionActive = 1;

  int index = findLastIndex();
  if(index != 101){
    sessions[index] = seconds;

  runSession(seconds, lights);
  }
  
  
}

void runSession(int seconds, int lights){
  int timer = seconds;
  
  int currentPixel = lights; 
  while (seconds > 0){
    delay(1000);
    seconds = seconds - 1;

    //each light is worth 2 minutes
    if (seconds % 120 == 0){
      strip.setPixelColor(currentPixel, red);
      currentPixel = currentPixel - 1;
      strip.show();
    }
  }
  
}

void setColors(int softpotValue){
  
}



void showHistory(){
  int totalseconds;
  for(int i = 0; i< (sizeof(sessions) / sizeof(sessions[0])); i++){
    totalseconds = totalseconds + sessions[i];
  }

  int average = totalseconds / (sizeof(sessions) / sizeof(sessions[0]));

  for(int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, red);
  }

  strip.show();
}


void startUpLights(){
  lightsOn = 1;
  
    for (int i = 0; i < strip.numPixels(); i++){
      strip.setPixelColor(i, red);
    }
    
    // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
 /* for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }*/
    strip.show(); // Update strip with new contents
    delay(1000);  // Pause for a moment
  
}

void turnOffLights(){
  lightsOn = 0;
  strip.clear();
  delay(1000);
}

void senseOnOff(){
  distanceVal = analogRead(distancePin); 
  Serial.println(distanceVal);

  if (STANDARD_VAL + THRESHOLD > 400){
    if (lightsOn == 0){
      //startUpLights();
    }

    else {
      //turnOffLights();
    }
  }

}




/*
  Source code for Project Halo - a lamp designed in the course Tangible Interaction.
  Uses three components:
  - SOFTPOT: For sensing movement throughout the ring
  https://learn.sparkfun.com/tutorials/softpot-hookup-guide/all


  - LED STRIP: Lights
  https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use


  - Distance Sensor: To sense turn on / turn off
  https://create.arduino.cc/projecthub/jenniferchen/distance-measuring-sensor-900520
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
#define LED_COUNT 29

#define MIN_BRIGHTNESS 10                   // watch the power!
#define MAX_BRIGHTNESS 120                   // watch the power!


int fadeAmount = 5;  // Set the amount to fade I usually do 5, 10, 15, 20, 25 etc even up to 255.
int brightness = 0;



int sessions[100];
int sessionActive = 0;

int lightsOn = 0;


CRGB leds[LED_COUNT];
int lightsInSession = 0;

CRGB ACTIVE_COLOR = CRGB::Red;
uint8_t hue = 0;




// -- SOFTPOT
const int SOFT_POT_PIN = A0; // Pin connected to softpot wiper, used for setting timer

const int GRAPH_LENGTH = 40; // Length of line graph

const int SOFT_POT_PIN_2 = A2; // Pin connected to the second softpot wiper, used for changing colors

int softPotADC;
int softPotADC_2;


// -- IR Distance Sensor
int distancePin = A1;
int distanceVal = 0;

// if above val + threshold, we light it up
const int THRESHOLD = 100;
//roughly the default measuring value
const int STANDARD_VAL = 100;

const int TOLERANCE = 250;



unsigned long previousMillis = 0;    // will store last time LED was updated


long currentMillis;
long second = 1000;
long sessionLength = 10000;
unsigned long startMillis;  //some global variables available anywhere in the program



int currentPixel;


void setup()
{

  Serial.begin(19200);
  pinMode(SOFT_POT_PIN, INPUT);
  pinMode(SOFT_POT_PIN_2, INPUT);

  FastLED.addLeds<NEOPIXEL, 6>(leds, LED_COUNT);

  pinMode(distancePin, INPUT);

  FastLED.clear();

  for (int i = 0; i <= LED_COUNT; i++) {

    leds[i] = ACTIVE_COLOR;
  }

  FastLED.show();

  lightsInSession = LED_COUNT;



}

void loop()
{

  currentMillis = millis();

  // Read in the soft pot's ADC value
  softPotADC = analogRead(SOFT_POT_PIN);
  softPotADC_2 = analogRead(SOFT_POT_PIN_2);

  int distanceVal = analogRead(distancePin);



  if (softPotADC > 8) {
    fillLights(softPotADC);
  }

  if (softPotADC_2 > 1) {
    setColors(softPotADC_2);
  }



  if (sessionActive == 1) {
    if (currentMillis - startMillis >= second) {

      turnOffOneLED();
      startMillis = currentMillis;

      if (lightsInSession == 0) {
        sessionActive = 0;
      }


    }
  }
  else {
    pulsate2();

    if (softPotADC_2 > 1) {
      setColors(softPotADC_2);
    }
  }


}


void fillLights(int softpotValue) {

  //softpot value is 0-1023
  //ledcount
  int threshold = 1023 / LED_COUNT;

  // calc number of lights to turn on
  int lights = softpotValue / threshold;
  lightsInSession = lights + 1;

  FastLED.clear();
  for (int i = 0; i <= lightsInSession; i++) {

    leds[i] = ACTIVE_COLOR;
  }



  FastLED.show();


  startSession(lights);
}






int findLastIndex() {
  for (int i = 0; i < sizeof(sessions); i++) {
    if (sessions[i] == 0) {
      return i;
    }
  }

  return 101;
}


void startSession(int lights) {
  int minutes = lights * 2;
  int seconds = minutes * 60;

  int timeLeft = seconds;

  //sessionActive = 1;

  int index = findLastIndex();
  if (index != 101) {
    sessions[index] = seconds;

    currentPixel = lights + 1;

    sessionActive = 1;

  }

}


void turnOffOneLED() {


  Serial.println("Turning off one led");
  leds[currentPixel] = CRGB::Black;
  lightsInSession = lightsInSession - 1;
  currentPixel = currentPixel - 1;
  FastLED.show();
}


void setColors(int softpotValue) {

  int mapped = map(softpotValue, 0, 1023, 0, 255);
  CRGB color = CHSV(mapped, 255, 255);

  FastLED.clear();
  for (int i = 0; i < lightsInSession; i++) {
    leds[i] = color;
    ACTIVE_COLOR = color;

  }

  FastLED.show();


}


void pulsate() {
  int hue = 0;
  int divisor = 30;

  float breath = (exp(sin(millis() / 5000.0 * PI)) - 0.36787944) * 108.0;
  breath = map(breath, 0, 255, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  FastLED.setBrightness(breath);
  fill_rainbow(leds, LED_COUNT, (hue++ / divisor));
  if (hue == (255 * divisor)) {
    hue = 0;
  }
  FastLED.show();

  FastLED.setBrightness(MAX_BRIGHTNESS);

  FastLED.show();

}



void showHistory() {
  int totalseconds;
  for (int i = 0; i < (sizeof(sessions) / sizeof(sessions[0])); i++) {
    totalseconds = totalseconds + sessions[i];
  }

  int average = totalseconds / (sizeof(sessions) / sizeof(sessions[0]));

  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = ACTIVE_COLOR;
  }

  FastLED.show();
}


void pulsate2() {
  for (int i = 0; i < LED_COUNT; i++ )
  {
    leds[i] = ACTIVE_COLOR;  // Set Color HERE!!!
    leds[i].fadeLightBy(brightness);
  }
  FastLED.show();
  brightness = brightness + fadeAmount;
  // reverse the direction of the fading at the ends of the fade:
  if (brightness == 0 || brightness == 255)
  {
    fadeAmount = -fadeAmount ;
  }
  delay(65);  // This delay sets speed
}


void turnOffLights() {
  lightsOn = 0;
  FastLED.clear();
}

void senseOff() {

  if (STANDARD_VAL + THRESHOLD > 400) {
    //turnOffLights();


  }

}

#include <Adafruit_NeoPixel.h>

#define PIN 2
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 24

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayCircle = 100; // delay for half a second
int delayBlink = 250;
int colorR = 50;
int colorG = 0;
int colorB = 50;

void setup() 
{
  pixels.begin(); // This initializes the NeoPixel library.
}

void wanderCircle(int repeat) {
  for(int i = 0; i<repeat; i++) {
    for(int j = 0; j<NUMPIXELS; j++) {
    pixels.clear();
    pixels.setPixelColor(j, pixels.Color(colorR,colorG, colorB));
    pixels.setPixelColor(j - 1, pixels.Color(colorR * 0.5, colorG * 0.5, colorB * 0.5));
    pixels.show();
    delay(delayCircle);
    }
  } 
}

void blink(int repeat) {
  for(int i=0; i<repeat; i++) {
    for(int j=0; j<NUMPIXELS; j++) {
      pixels.setPixelColor(j, pixels.Color(colorR,colorG,colorB));
    }
    pixels.show();
    delay(delayBlink);
    pixels.clear();
    pixels.show();
    delay(delayBlink);
  }
}

void loop() 
{
  wanderCircle(3);
  delay(1000);
  blink(3);
  delay(1000);
}
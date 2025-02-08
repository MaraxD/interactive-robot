#include <Adafruit_NeoPixel.h>

#define LED_PIN 5    // Change to your chosen GPIO
#define NUM_LEDS 12  // Number of LEDs in your strip

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
int initialBrightness=255, flip;

void setup() {
  strip.begin();
  strip.setBrightness(initialBrightness);
  strip.show();  // Initialize all pixels to 'off'

}

void loop() {
  strip.setPixelColor(6, strip.Color(255, 0, 0));  // Red

  initialBrightness-=10*flip;
  strip.setBrightness(initialBrightness);
  strip.show();

  if(initialBrightness==100){
    flip=-1;
  }else if(initialBrightness==255){
    flip=1;
  }
  delay(100);
}

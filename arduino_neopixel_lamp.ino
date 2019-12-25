/*
  Arduino NeoPixel Lamp v1.0.1

  Simple lamp script with neopixels.

  Features:
   - Button to cycle between animations.
   - Saves last state to EEPROM.

   Animations:
    - RAINBOW: Simple animation modified from the NeoPixel standtest.
    - RAINBOW2: Very slow rainbow animation. Sets most of the strip to the same color.
    - FIXED: Fixes the last color.
    - WHITE: White :)

  Alfredo Rius
  alfredo.rius@gmail.com
  12/11/2019
*/

#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define LED_COUNT 16

#define LED_PIN     A3
#define BUTTON_PIN  2

#define RAINBOW  0
#define RAINBOW2 1
#define FIXED    2
#define WHITE    3
#define LAST_STATE WHITE

#define RAINBOW_SPEED 256
#define RAINBOW2_SPEED 10

#define DEBOUNCE 500

#define STATE_ADDR 0
#define HUE_ADDR   1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
uint16_t pixelHue;
uint8_t state = 0;
unsigned long last_button;
uint8_t startup;

void setup() {
  // Init hardware
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  strip.begin();
  strip.show();
  strip.setBrightness(255);

  // Init previous state
  state = EEPROM.read(STATE_ADDR);
  if(state > LAST_STATE)button(); // Ensure valid state
  pixelHue = readHUE();

  // Button interrupt
  last_button = millis();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button, FALLING);

  // Reset startup
  startup = 1;
}

void loop() {
  if(state == RAINBOW){
    rainbow(10,LED_COUNT,state);
  }else if(state == RAINBOW2){
    rainbow(10,LED_COUNT*50,state);
  }else if(state == FIXED){
    if(!startup){
      // The lamp will blink twice to show that the
      // state has changed.
      setHUE(pixelHue);
      delay(300);
      setColor(strip.Color(0,0,0));
      delay(300);
      setHUE(pixelHue);
      delay(300);
      setColor(strip.Color(0,0,0));
      delay(300);
    }
    setHUE(pixelHue);
    while(state == FIXED)delay(10);
  }else if(state == WHITE){
    setColor(strip.Color(255,255,255));
  }
  delay(100);
}

void writeHUE(uint16_t hue){
  // Write uint16_t HUE value
  uint8_t tmp;
  tmp = hue;
  EEPROM.write(HUE_ADDR,tmp);
  tmp = hue >> 8;
  EEPROM.write(HUE_ADDR+1,tmp);
}

uint16_t readHUE(){
  // Read uint16_t HUE value
  uint16_t tmp;
  tmp = EEPROM.read(HUE_ADDR+1) << 8;
  tmp &= 0xFF00;
  tmp |= EEPROM.read(HUE_ADDR);
  return tmp;
}

void button(){
  // Button ISR
  unsigned long this_button = millis();

  // If any state change is detected, then, this is not the startup.
  startup = 0;

  // Check debounce
  if(last_button + DEBOUNCE < this_button){
    if(state >= LAST_STATE){
      state = 0;
    }else{
      state++;
    }

    // Save to eeprom
    EEPROM.write(STATE_ADDR,state);
    writeHUE(pixelHue);

    // Reset debounce
    last_button = this_button;
  }  
}

void setHUE(uint16_t hue) {
  // Simple set hue
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
  }
  strip.show();
}

void setColor(uint32_t color) {
  // Simple set white
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void rainbow(int wait,uint16_t range,uint8_t current_state) {
  // Modified from NeoPixel standtest for two kinds of rainbows, one fast with the full
  // length of the strip, the second slow and almos the same color in all the strip.
  for(long firstPixelHue = pixelHue; firstPixelHue < 5*65536 && state == current_state; firstPixelHue += (current_state == RAINBOW)?RAINBOW_SPEED:RAINBOW2_SPEED) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      pixelHue = firstPixelHue + (i * 65536L / range);
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

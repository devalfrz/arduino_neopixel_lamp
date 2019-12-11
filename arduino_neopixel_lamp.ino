/*
  Arduino NeoPixel Lamp v1.0

  Simple lamp script with neopixels.

  Features:
   - Button to cycle between animations.
   - Saves last state to EEPROM.

   Animations:
    - RAINBOW: Simple animation modified from the NeoPixel standtest.
    - FIXED: Fixes the color of the first led in the previews animation.
    - RAINBOW2: Very slow rainbow animation. Sets most of the strip to the same color.
    - WHITE: White :)

  Alfredo Rius
  alfredo.rius@gmail.com
  12/11/2019
*/

#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define LED_COUNT 8

#define LED_PIN     A3
#define BUTTON_PIN  2

#define RAINBOW  0
#define FIXED    1
#define RAINBOW2 2
#define WHITE    3
#define LAST_STATE WHITE

#define RAINBOW_SPEED 256
#define RAINBOW2_SPEED 10

#define DEBOUNCE 300

#define STATE_ADDR 0
#define HUE_ADDR   1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
uint16_t pixelHue;
uint8_t state = 0;
unsigned long last_button;

void setup() {
  // Init hardware
  pinMode(BUTTON_PIN,INPUT);
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
}

void loop() {
  if(state == RAINBOW){
    rainbow(10,LED_COUNT,state);
  }else if(state == RAINBOW2){
    rainbow(10,LED_COUNT*50,state);
  }else if(state == FIXED){
    setColor(pixelHue);
  }else if(state == WHITE){
    setWhite();
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

void setColor(uint16_t hue) {
  // Simple set hue
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
  }
  strip.show();
}

void setWhite() {
  // Simple set white
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(255,255,255));
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

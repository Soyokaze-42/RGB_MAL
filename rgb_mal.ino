
#include <FastLED.h>

//7 strips of 6 tripplets each
//These should be wired as part of the same strip with alternating directions
#define NUM_STRIPS 7
#define NUM_LEDS 6 //LEDs per strip

//Define the data pin
#define DATA_PIN1 5

//define pins for the buttons
#define ANIMATION_BUTTON_PIN 3
#define PALETTE_BUTTON_PIN 2

//This audjusts how quickly things happen
#define FRAMES_PER_SECOND 60

//define a constant for the brightness
#define BRIGHTNESS 80

//define percent likely a random cell will be alive for Conway' Game of Life
#define LIFECHANCE = 20

//Define LED array
CRGB leds[NUM_LEDS*NUM_STRIPS];

volatile int animation = 1;

CRGBPalette16 currentPalette( HeatColors_p );

CRGBPalette16 targetPalette( PartyColors_p );

//This is a flag that is checked inside loops to make sure the animation change 
// button hasn't been pressed between loop iterations
bool break_flag = 0;

void setup() {

//  Serial.begin(57600);
//  Serial.println("resetting");

  random16_add_entropy( random());
  
  //add LEDs to FastLED
  FastLED.addLeds<WS2811, DATA_PIN1, BRG>(leds, NUM_LEDS*NUM_STRIPS);

  pinMode(ANIMATION_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PALETTE_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ANIMATION_BUTTON_PIN), change_animation, LOW);
  attachInterrupt(digitalPinToInterrupt(PALETTE_BUTTON_PIN), change_palette, LOW);

  FastLED.setBrightness(BRIGHTNESS);

  //initialize with a random palette
  change_palette();
}

void change_animation() {
  
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();

 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 200) 
 {
    if (animation < 4) {
      animation++;
    } else {
      animation = 0;
    }
  }
  last_interrupt_time = interrupt_time;

  //blank out the LEDs for the next animation
  fadeall();
}

void change_palette() {
  
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 static uint8_t palette = random8(8);

 // If interrupts come faster than 500ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 500) 
 {
    switch (palette) {
      case 1 :
        currentPalette = RainbowColors_p;
        break;
      case 2 :
        currentPalette = CloudColors_p;
        break;
      case 3 :
        currentPalette = LavaColors_p;
        break;
      case 4 :
        currentPalette = OceanColors_p;
        break;
      case 5 :
        currentPalette = ForestColors_p;
        break;
      case 6 :
        currentPalette = RainbowStripeColors_p;
        break;
      case 7 :
        currentPalette = PartyColors_p;
        break;
      case 8 :
        currentPalette = CRGBPalette16 (
                                   0x480000,  0xF0A860,  0x000000,  0xF09048,
                                   0xD84830, 0x000000,  0xF0C060, 0x000000,
                                   0x66330, 0xFFFFFF ,  0xFF0000,  0xFF6600,
                                   0xFFCC00, 0x480000, 0xF0A860,  0xF0C060 );
      default :
        currentPalette = HeatColors_p;
        palette = 0;
    }
    palette++;
//    Serial.print("palette: ");
//    Serial.println(palette);
  }
  last_interrupt_time = interrupt_time;

}

//This mostly comes from the fastLED cylon example.12E
void fadeall() { 
  for(int i = 0; i < NUM_LEDS * NUM_STRIPS; i++) { 
    leds[i].nscale8(256);
  } 
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  
  for( int i = 0; i < NUM_LEDS * NUM_STRIPS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i*16), BRIGHTNESS);
    colorIndex += 3;
  }
}


void ChangePalettePeriodically()
{
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand ) {
    lastSecond = secondHand;
    CRGB p = CHSV( HUE_PURPLE, 255, 255);
    CRGB g = CHSV( HUE_GREEN, 255, 255);
    CRGB b = CRGB::Black;
    CRGB w = CRGB::White;
    if( secondHand ==  0)  { targetPalette = RainbowColors_p; }
    if( secondHand == 10)  { targetPalette = CRGBPalette16( g,g,b,b, p,p,b,b, g,g,b,b, p,p,b,b); }
    if( secondHand == 20)  { targetPalette = CRGBPalette16( b,b,b,w, b,b,b,w, b,b,b,w, b,b,b,w); }
    if( secondHand == 30)  { targetPalette = LavaColors_p; }
    if( secondHand == 40)  { targetPalette = CloudColors_p; }
    if( secondHand == 50)  { targetPalette = PartyColors_p; }
  }
}

//from https://gist.github.com/kriegsman/1f7ccbbfa492a73c015e
void pallet_fade(){
  
  ChangePalettePeriodically();

  // Crossfade current palette slowly toward the target palette
  //
  // Each time that nblendPaletteTowardPalette is called, small changes
  // are made to currentPalette to bring it closer to matching targetPalette.
  // You can control how many changes are made in each call:
  //   - the default of 24 is a good balance
  //   - meaningful values are 1-48.  1=veeeeeeeery slow, 48=quickest
  //   - "0" means do not change the currentPalette at all; freeze
  
  uint8_t maxChanges = 24; 
  nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);


  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  FillLEDsFromPaletteColors( startIndex);

  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

//mostly from the fastLED examples
void cylon(){
  
  static uint8_t hue = 0;
  // First slide the led in one direction
  for(int i = 0; i < NUM_STRIPS; i++) {
    // Set the i'th led strip to red 
    for(int j = 0; j < NUM_LEDS;J++) {
      leds[i*NUM_LEDS+j] = CHSV(hue++, 255, 255);
    }
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();

    // Break immediately if the change animation button was pressed
    if ( break_flag ) {
      break_flag = 0;
      fadeall();
      break;
    }
    
    // Wait a little bit before we loop around and do it again
    delay(15);
  }

  // Now go in the other direction.  
  for(int i = NUM_STRIPS-1; i >= 0; i--) {
    // Set the i'th led strip to red 
    for(int j = 0; j < NUM_LEDS;J++) {
      leds[i*NUM_LEDS+j] = CHSV(hue++, 255, 255);
    }
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    
    // Break immediately if the change animation button was pressed
    if ( break_flag ) {
      break_flag = 0;
      fadeall();
      break;
    }
    
    // Wait a little bit before we loop around and do it again
    delay(15);
  }
}

void random_walk() {
  
  for(int i = 0; i < NUM_LEDS * NUM_STRIPS; i++) {
    leds[i] = ColorFromPalette( currentPalette, random8());
    // Show the leds
    FastLED.show(); 

    // Break immediately if the change animation button was pressed
    if ( break_flag ) {
      break_flag = 0;
      fadeall();
      break;
    }

    // Wait a little bit before we loop around and do it again
    delay(100);
  }
  delay(300);
  fadeall();
  delay(100);
  
}

void fullrandom() {
  leds[random8(NUM_LEDS * NUM_STRIPS)] = ColorFromPalette( currentPalette, random8());

//    Serial.print("random: ");
//    Serial.println(random8(NUM_LEDS * NUM_STRIPS));
  FastLED.show();
  delay(50);
}

void fireflys() {

  
}

void conways_life_seed() {
  uint8_t rndNum = random8( 99 );
  if ( rndNum > LIFECHANCE ) {
    return ( 1 );
  } else {
    return ( 0 );
  }
}

void conways_life() {
  static bool lifeChanges[NUM_LEDS][NUM_STRIPS] {}; //initialize all 0
  static CRGBPalette16 startingPallet {}; //initialize null (I hope)

  // changing the palette also resets the game
  if ( startingPallet != currentPallet ) {
    startingPallet = currentPallet;
    fadeall();
    for ( int i = 0; i < NUM_LEDS; i++ ) {
      for ( int j = 0; j < NUM_STRIPS ) {
        lifeChanges[i][j] = conways_life_seed();
      }
    }
  } else {
    // This is where the game counts neighbors to see what changes need to happen
  }

    for ( int i = 0; i < NUM_LEDS; i++ ) {
      for ( int j = 0; j < NUM_STRIPS ) {
        //set all the cells here
      }
    }

  FastLED.show();
  delay(500);
  
}

void loop() {
  
  switch(animation){
    case 0 :
      cylon();
      break;
    case 1 :
      fire();
      break;
    case 2 :
      pallet_fade();
      break;
    case 3 :
      random_walk();
      break;
    case 4 :
      fullrandom();
      break;
  }

//  Serial.print("animation: ");
//  Serial.println(animation);
}

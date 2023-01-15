//Pranav Kallem and Swagat Adhikary
//COMP 89, Final Project
//Game: Flappy Bird

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "SevSeg.h"
SevSeg sevseg; 
LiquidCrystal_I2C lcd(0x27,16,2);

//"sense, compute, actuate" philosophy used for abstraction.

void loop(){
  sense();
  compute();
  actuate();
}


////////////////////
// CODE BEGINS HERE
///////////////////


// These are the global variables that are referenced throughout the program.

#define PIN_BUTTON 2
#define PIN_AUTOPLAY 1
#define PIN_READWRITE 10
#define PIN_CONTRAST 7

#define BIRD_RUN1 1
#define BIRD_RUN2 2
#define BIRD_JUMP 3
#define BIRD_JUMP_UPPER '.'        
#define BIRD_JUMP_LOWER 4
#define BIRD_MAP_EMPTY ' '    
#define BIRD_MAP_SOLID 5
#define BIRD_MAP_SOLID_RIGHT 6
#define BIRD_MAP_SOLID_LEFT 7

// This shows us where the bird is on the screen, which in this case is the center because its a 16x2 LCD.
#define BIRD_HORIZONTAL_POSITION 8    

#define MAP_WIDTH 16
#define MAP_EMPTY 0
#define MAP_LOWER_BLOCK 1
#define MAP_UPPER_BLOCK 2

#define BIRD_POSITION_OFF 0         
#define BIRD_POSITION_RUN_LOWER_1 1  
#define BIRD_POSITION_RUN_LOWER_2 2                       

#define BIRD_POSITION_JUMP_1 3      
#define BIRD_POSITION_JUMP_2 4      
#define BIRD_POSITION_JUMP_3 5      
#define BIRD_POSITION_JUMP_4 6  
#define BIRD_POSITION_JUMP_5 7  
#define BIRD_POSITION_JUMP_6 8   
#define BIRD_POSITION_JUMP_7 9   
#define BIRD_POSITION_JUMP_8 10    

#define BIRD_POSITION_RUN_UPPER_1 11
#define BIRD_POSITION_RUN_UPPER_2 12 

static char mapUpper[MAP_WIDTH + 1];
static char mapLower[MAP_WIDTH + 1];
static bool buttonPushed = false;
const int ledPin =  3;

//The delay time is what we will be using to make the game flow faster/slower. A higher delay time means slower game.
int delayTime = 200;

static byte birdPos = BIRD_POSITION_RUN_LOWER_1;
static byte newTerrainType = MAP_EMPTY;
static byte newTerrainDuration = 1;
static bool playing = false;
static bool blink = false;
static unsigned int distance = 0;

void setup(){
  Serial.begin(9600);
  // All the LEDs, LCDs, etc. are all mapped appropriately to their inputs/outputs. 
  // All hardware is prepared before the actual game starts running.
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_READWRITE, OUTPUT);
  digitalWrite(PIN_READWRITE, LOW);
  pinMode(PIN_CONTRAST, OUTPUT);
  digitalWrite(PIN_CONTRAST, LOW);
  digitalWrite(PIN_BUTTON, HIGH);
  pinMode(ledPin, OUTPUT);
  byte numDigits = 4;
  byte digitPins[] = {10, 11, 12, 13};
  byte segmentPins[] = {9, A1, A0, 5, 6, 8, 7, 4};
  bool resistorsOnSegments = true; 
  bool updateWithDelaysIn = true;
  byte hardwareConfig = COMMON_CATHODE; 
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(90);
  pinMode(PIN_AUTOPLAY, OUTPUT);
  digitalWrite(PIN_AUTOPLAY, HIGH);
  lcd.backlight();
  attachInterrupt(0, buttonPush, FALLING);
  initializeGraphics();  
  lcd.begin();
}


//SENSE, COMPUTE, ACTUATE METHODS

void sense() {


  if (!playing) {
    drawBIRD((blink) ? BIRD_POSITION_OFF : birdPos, mapUpper, mapLower, distance >> 3);
    if (blink) {
      //This shows the default "Press to start" message before each game.
      lcd.setCursor(0,0);
      lcd.print("Press to start");
    }
    // This is how fast the message blinks.
    delay(250);
    // This causes the blink.
    blink = !blink;
    if (buttonPushed) {
      // The characters are re-initialized when the button is pushed.
      initializeGraphics();
      birdPos = BIRD_POSITION_RUN_LOWER_1;
      playing = true;
      buttonPushed = false;
      // The score is reset, with a variety of other default values.
      distance = 0;
    }
    return;
  }


}

void compute() {
  // This computes where the poles have to be placed in the map, and computes the bird's position based on the button input.
  advanceTerrain(mapLower, newTerrainType == MAP_LOWER_BLOCK ? BIRD_MAP_SOLID : BIRD_MAP_EMPTY);
  advanceTerrain(mapUpper, newTerrainType == MAP_UPPER_BLOCK ? BIRD_MAP_SOLID : BIRD_MAP_EMPTY);
  
  if (--newTerrainDuration == 0) {
    if (newTerrainType == MAP_EMPTY) {
      newTerrainType = (random(3) == 0) ? MAP_UPPER_BLOCK : MAP_LOWER_BLOCK;
      newTerrainDuration = 2 + random(10);
    } else {
      newTerrainType = MAP_EMPTY;
      newTerrainDuration = 10 + random(10);
    }
  }
    
  if (buttonPushed) {
    // This makes the LED light up.
    digitalWrite(ledPin, HIGH);
    if (birdPos <= BIRD_POSITION_RUN_LOWER_2) birdPos = BIRD_POSITION_JUMP_1;
    buttonPushed = false;
  } else {
    // This makes the LED dim after the button press.
    digitalWrite(ledPin, LOW);
  }
  
}

void actuate() {
  // This sets the bird position to the desired location based on a variety of conditions. 
  if (drawBIRD(birdPos, mapUpper, mapLower, distance >> 3)) {
    playing = false; 
  } else {
    if (birdPos == BIRD_POSITION_RUN_LOWER_2 || birdPos == BIRD_POSITION_JUMP_8) {
      birdPos = BIRD_POSITION_RUN_LOWER_1;
    } else if ((birdPos >= BIRD_POSITION_JUMP_3 && birdPos <= BIRD_POSITION_JUMP_5) && mapLower[BIRD_HORIZONTAL_POSITION] != BIRD_MAP_EMPTY) {
      birdPos = BIRD_POSITION_RUN_UPPER_1;
    } else if (birdPos >= BIRD_POSITION_RUN_UPPER_1 && mapLower[BIRD_HORIZONTAL_POSITION] == BIRD_MAP_EMPTY) {
      birdPos = BIRD_POSITION_JUMP_5;
    } else if (birdPos == BIRD_POSITION_RUN_UPPER_2) {
      birdPos = BIRD_POSITION_RUN_UPPER_1;
    } else {
      ++birdPos;
    }
    ++distance;
    // This shows the information on the LCD. 
    digitalWrite(PIN_AUTOPLAY, mapLower[BIRD_HORIZONTAL_POSITION + 2] == BIRD_MAP_EMPTY ? HIGH : LOW);
  }
  // This delayTime decides how fast the game moves. 
  delay(delayTime);
}






//OTHER SUPPORTING METHODS

void initializeGraphics(){ 
  static byte graphics[] = {
    //Running
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B01111,
    B01111,
    B00000,

    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00011,
    B01111,
    B00000,


    // Jumping
    B00000,
    B00000,
    B00001,
    B00010,
    B01100,
    B00000,
    B00000,
    B00000,
    
    B00000,
    B00000,
    B00000,
    B00000,
    B01100,
    B00010,
    B00001,
    B00000,
    
    // Poles
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,

    B00011,
    B00011,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,

    B11000,
    B11000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,

  };
  
  // We are creating a character for the LCD for each of the objects that will be displayed in the game. 
  int i;
  for (i = 0; i < 7; ++i) {
    lcd.createChar(i + 1, & graphics[i * 8]);
  }

  // An empty map is being created.
  for (i = 0; i < MAP_WIDTH; ++i) {
    mapUpper[i] = BIRD_MAP_EMPTY;
    mapLower[i] = BIRD_MAP_EMPTY;
  }
}

void advanceTerrain(char* map, byte newTerrain){ 

  // The outer loop executes for each segment in the 16 vertical columns of the LCD. 
  for (int i = 0; i < MAP_WIDTH; ++i) {
    char current = map[i];
  
    // If the map is at the end, new terrain is created. If we are not at the end, we advance to the next part of the map.
    char next = (i == MAP_WIDTH-1) ? newTerrain : map[i+1];
    switch (current){
      // The following are switch cases regarding what should be done in each scenario of the map. 
      case BIRD_MAP_EMPTY:
        map[i] = (next == BIRD_MAP_SOLID) ? BIRD_MAP_SOLID_RIGHT : BIRD_MAP_EMPTY;
        break;
      case BIRD_MAP_SOLID:
        map[i] = (next == BIRD_MAP_EMPTY) ? BIRD_MAP_SOLID_LEFT : BIRD_MAP_SOLID;
        break;
      case BIRD_MAP_SOLID_RIGHT:
        map[i] = BIRD_MAP_SOLID;
        break;
      case BIRD_MAP_SOLID_LEFT:
        map[i] = BIRD_MAP_EMPTY;
        break;
    }
  }
}

bool drawBIRD(byte position, char* mapUpper, char* mapLower, unsigned int score) { //Actuates pixels.
  // A boolean for collision is used to keep track of whether the bird has collided into a pole or not.
  bool collide = false;
  char upperSave = mapUpper[BIRD_HORIZONTAL_POSITION];
  char lowerSave = mapLower[BIRD_HORIZONTAL_POSITION];
  byte upper, lower;
  switch (position) {
    case BIRD_POSITION_OFF:
      upper = lower = BIRD_MAP_EMPTY;
      break;
    case BIRD_POSITION_RUN_LOWER_1:
      upper = BIRD_MAP_EMPTY;
      lower = BIRD_RUN1;
      break;
    case BIRD_POSITION_RUN_LOWER_2:
      upper = BIRD_MAP_EMPTY;
      lower = BIRD_RUN2;
      break;
    case BIRD_POSITION_JUMP_1:
    case BIRD_POSITION_JUMP_8:
      upper = BIRD_MAP_EMPTY;
      lower = BIRD_JUMP;
      break;
    case BIRD_POSITION_JUMP_2:
    case BIRD_POSITION_JUMP_7:
      upper = BIRD_JUMP_UPPER;
      lower = BIRD_JUMP_LOWER;
      break;
    case BIRD_POSITION_JUMP_3:
    case BIRD_POSITION_JUMP_4:
    case BIRD_POSITION_JUMP_5:
    case BIRD_POSITION_JUMP_6:
      upper = BIRD_JUMP;
      lower = BIRD_MAP_EMPTY;
      break;
    case BIRD_POSITION_RUN_UPPER_1:
      upper = BIRD_RUN1;
      lower = BIRD_MAP_EMPTY;
      break;
    case BIRD_POSITION_RUN_UPPER_2:
      upper = BIRD_RUN2;
      lower = BIRD_MAP_EMPTY;
      break;
  }
  if (upper != ' ') {
    mapUpper[BIRD_HORIZONTAL_POSITION] = upper;
    collide = (upperSave == BIRD_MAP_EMPTY) ? false : true;
  }
  if (lower != ' ') {
    mapLower[BIRD_HORIZONTAL_POSITION] = lower;
    collide |= (lowerSave == BIRD_MAP_EMPTY) ? false : true;
  }
  
  byte digits = (score > 9999) ? 5 : (score > 999) ? 4 : (score > 99) ? 3 : (score > 9) ? 2 : 1;
  // We show the score on the 4-digit 7-segment display.
  sevseg.setNumber(score);
  sevseg.refreshDisplay();
  
  // This updates the speed of the game using a mathematical equation based on the score. 
  if (score < 15) {
    delayTime = (20 - score) * 10;
  } else {
    delayTime = 40;
  }
  
  mapUpper[MAP_WIDTH] = '\0';
  mapLower[MAP_WIDTH] = '\0';
  char temp = mapUpper[16-digits];
  mapUpper[16-digits] = '\0';
  lcd.setCursor(0,0);
  lcd.print(mapUpper);
  mapUpper[16-digits] = temp;  
  lcd.setCursor(0,1);
  lcd.print(mapLower);
  
  lcd.setCursor(16 - digits,0);
  lcd.print(score);

  mapUpper[BIRD_HORIZONTAL_POSITION] = upperSave;
  mapLower[BIRD_HORIZONTAL_POSITION] = lowerSave;
  return collide;
}

void buttonPush() { 
  // Sense: This method makes buttonPushed true.
  buttonPushed = true;
}
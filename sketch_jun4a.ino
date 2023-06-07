// Dot sniping game by Nathan Fong
// Implements code from arduino example codes as well as segments of code from previous activities

#include <Adafruit_CircuitPlayground.h>
#include <AsyncDelay.h>

// scoreboard
int hitsCur;
int hitsMax;
String results;

// Sensor values
int micOut;
int accOut;
int avgMic[4] = {0,0,0,0};
int avgMicValue;
int avgAcc[4] = {0,0,0,0};
int avgAccValue;

// position and modulus position, count is linear and mod is limited to 0-9 cycling
int aimCount;
int tarCount;
int aimMod;
int tarMod;
int tarLMod;
int tarRMod;

// Value with toggle between 1 or -1 to indicate direction of aim dot
int directionAim;

// velocity values
int aimVel;
int tarVel;

// upon resolution of these delays, calculate new position variables which will be updated to the LEDs during the position engine, then take sensors to calculate next velocity set/start that delay. Delays should not go below 10ms
AsyncDelay delay_aim;
AsyncDelay delay_target;

// interrupt settup
const byte switchPin = 7;
volatile bool switchToggle;
volatile bool intSFlag = 1;
const byte rButtonPin = 4;
volatile bool intRBFlag = 1;
const byte lButtonPin = 5;
volatile bool intLBFlag = 1;

// Music variables
float midi[127];
int A_four = 440;  // a is 440 hz...
// Arrays to store Scale MIDI pitch sequence
int c_major[8] = { 60, 62, 64, 65, 67, 69, 71, 72 };  // c_major scale on C4

void setup() {
  // setup basics
  Serial.begin(9600);
  CircuitPlayground.begin();
  generateMIDI();

  // setup interrupts
  pinMode(switchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(switchPin), sLink, CHANGE);
  pinMode(rButtonPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(rButtonPin), rBLink, RISING);
  pinMode(lButtonPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(lButtonPin), lBLink, RISING);

  // Set flags to current pin state since switch can be toggled on initialization
  intSFlag = digitalRead(switchPin);
  intRBFlag = digitalRead(rButtonPin);
  intLBFlag = digitalRead(lButtonPin);
  switchToggle = digitalRead(switchPin);

  // Start
  startUpGame();
}

void loop() {
  if (intSFlag == 1){
    switchToggle = digitalRead(switchPin);
    intSFlag = 0;
    if (switchToggle) {
      startUpGame();
    }
    else {
      ko(0,0,0);
    }
  }
  if (switchToggle) {
    if (intRBFlag == 1) {
      delay(10);
      intRBFlag = 0;
      if (aimMod == tarMod) {
        effectWin(2);
      }
      else if (aimMod == tarLMod || aimMod == tarRMod) {
        effectWin(1);
      }
      else {
        effectLoss();
      }
    }
    if (intLBFlag == 1) {
      intLBFlag = 0;
      if (directionAim == 1) {
        directionAim = -1;
      }
      else if (directionAim == -1) {
        directionAim = 1;
      }
    }
    
    // Run calculations for linear to circular position
    aimMod = translateTen(aimCount);
    tarMod = translateTen(tarCount);
    tarLMod = translateTen(tarCount-1);
    tarRMod = translateTen(tarCount+1);
    
    // Position engine
    for (int i = 0; i < 10; ++i) {
      if (i==aimMod) {
        CircuitPlayground.setPixelColor(i, 0, 0, 255);
      }
      else if (i==tarMod) {
        CircuitPlayground.setPixelColor(i, 255, 0, 0);
      }
      else if (i==tarLMod || i==tarRMod) {
        CircuitPlayground.setPixelColor(i, 255, 25, 25);
      }
      else {
        CircuitPlayground.setPixelColor(i, 0, 0, 0);
      }
    }
    
    // velocity engines
    // When delay done: change indicated position by 1 either + or -
    if (delay_aim.isExpired()) {
      reCalcAcc();
      accOut = map(avgAccValue,8,38,0,-3000);
      aimVel = 800 + accOut -hitsCur;
      if (aimVel <= 20) {
        aimVel = 20;
      }
      aimCount += directionAim;
      // Serial.println(aimCount);
      // Serial.println(aimVel);
      delay_aim.start(aimVel, AsyncDelay::MILLIS);
    }
    if (delay_target.isExpired()) {
      reCalcMic();
      micOut = map(avgMicValue,50,100,0,-1200);
      tarVel = 650 + micOut;
      if (tarVel <= 20) {
        tarVel = 20;
      }
      tarCount += 1;
      // Serial.println(tarCount);
      // Serial.println(tarVel);
      delay_target.start(tarVel, AsyncDelay::MILLIS);
    }
    results =("");
    results += ("\n\n\nSession High Score: ");
    results += (hitsMax);
    results += ("\n     Current Score: ");
    results += (hitsCur);
    Serial.println(results);
    delay(10);
  }
}


// Start of functions

void setColor(int color, int ccolor) {
  for (int i = 0; i < 10; ++i) {
    CircuitPlayground.setPixelColor(i, color, color, ccolor);
  }
}

// Abstracted way to set all leds to same color
void ko(int numOne, int numTwo, int numThree) {
  for (int i = 0; i < 10; ++i) {
    CircuitPlayground.setPixelColor(i, numOne, numTwo, numThree);
  }
}

// Do music things
void generateMIDI() {
  for (int x = 0; x < 127; ++x) {
    midi[x] = (A_four / 32.0) * pow(2.0, ((x - 9.0) / 12.0));
  }
}
void noiseIsGood() {
  // Serial.println("GoodNoise");
  CircuitPlayground.playTone(midi[c_major[0]], 40);
  CircuitPlayground.playTone(midi[c_major[7]], 40);
}
void noiseIsBad() {
  // Serial.println("BadNoise");
  CircuitPlayground.playTone(midi[c_major[7]], 150);
  CircuitPlayground.playTone(midi[c_major[0]], 250);
}

// Interrupts set up flags
void sLink() {
  intSFlag = 1;
  // Serial.println("switch trip");
}
void rBLink() {
  intRBFlag = 1;
  // Serial.println("right button trip");
}
void lBLink() {
  intLBFlag = 1;
  // Serial.println("left button trip");
}

// Random brightness with certain colors
int graySync;
void sparkle(int decideC) {
  // Make an offset based on the current millisecond count scaled by the current speed.
  uint32_t offset = millis() / 100;
  // Loop through each pixel and set it to an incremental color wheel value.
  for (int i = 0; i < 10; ++i) {
    // Serial.println(count);
    delay(5);
    graySync = random(255);
    CircuitPlayground.setPixelColor(i, graySync, graySync * decideC, graySync * decideC);
  }
}

// Set of outputs that play on hit/miss
void effectWin(int pointCount) {
  sparkle(1);
  noiseIsGood();
  // Serial.println("goodnoise");
  aimCount += 5;
  hitsCur += pointCount;
  if (hitsCur > hitsMax) {
    hitsMax = hitsCur;
  }
  for (int i = 0; i < 10; ++i) {
    sparkle(1);
  }
  ko(255,255,255);
  intRBFlag = 0;
}
void effectLoss() {
  sparkle(0);
  noiseIsBad();
  // Serial.println("badnoise");
  for (int i = 0; i < 15; ++i) {
    sparkle(0);
  }
  ko(255,0,0);
  startUpGame();
  intRBFlag = 0;
}

// Function that initializes all variables
void startUpGame() {
  ko(0,0,0);
  delay_target.expire();
  delay_aim.expire();
  
  // reseting variables
  aimCount = 3;
  tarCount = 7;
  aimMod = translateTen(aimCount);
  tarMod = translateTen(tarCount);
  tarLMod = translateTen(tarCount-1);
  tarRMod = translateTen(tarCount+1);
  directionAim = 1;

  // set gamestate
  hitsCur = 0;
  ko(0,0,255);
  delay(1000);
  delay_target.start(2000, AsyncDelay::MILLIS);
  delay_aim.start(2000, AsyncDelay::MILLIS);
}
int translateTen(int linearPos) {
  if (linearPos < 0) {

    linearPos = 10 + (linearPos % 10);
  }
  return((linearPos % 10));
}
int accRoCount = 0;
int accRoTotal = 0;
void reCalcAcc() {
  accRoTotal = 0;
  for (int i = 0; i < 4; ++i) {
    if (i==accRoCount){
      avgAcc[i] = abs(CircuitPlayground.motionX()) + abs(CircuitPlayground.motionY()) + abs(CircuitPlayground.motionZ());
    }
    accRoTotal += avgAcc[i];
  }
  accRoCount += 1;
  if (accRoCount==4) {
    accRoCount = 0;
  }
  avgAccValue=accRoTotal/4;
}
int micRoCount = 0;
int micRoTotal = 0;
void reCalcMic() {
  micRoTotal = 0;
  for (int i = 0; i < 4; ++i) {
    if (i==micRoCount){
      avgMic[i] = abs(CircuitPlayground.mic.soundPressureLevel(10));
    }
    micRoTotal += avgMic[i];
  }
  micRoCount += 1;
  if (micRoCount==4) {
    micRoCount = 0;
  }
  avgMicValue=micRoTotal/4;
}
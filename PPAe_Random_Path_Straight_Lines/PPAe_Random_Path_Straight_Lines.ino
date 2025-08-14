#include <AccelStepper.h>
#include <MultiStepper.h>

// Define the pins for the joystick and servo
const int joyXpin = A2; //analog pins
const int joyYpin = A3;
const int joyButton = 13; // Button pin for saving position

// Define variables for joystick readings
int xJoyVal = 0; //initializes x value to be 0
int yJoyVal = 0; //initializes y value to be 0
int buttonState = 0; // To store button press state

// Define stepper motor pins
const int xDirPin = 2;
const int xStepPin = 3;

const int yDirPin = 5;
const int yStepPin = 6;
const int SleepPin = 4;
const int microstepPin = 7;

const int laserPin = 11;

// Joystick position range
int xPosRange[2] = {-180, 180};
int yPosRange[2] = {-180, 180};
long xyPos[2] = {0, 0}; // Stores the positions [x, y] for the two motors

// Create stepper objects
AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

// Variables to store the saved stepper positions (4 points)
long savedPositions[4][2]; // Array to store 4 positions: [0] = X, [1] = Y
int pressCount = 0; // Count of button presses

float absMaxSpeed = 1200;//Remove if maxed out of variables
float speedSF = 1;
float tempMaxSpeed = speedSF*absMaxSpeed;

bool clickable = true;
const int SpeakerPin = 12;

void randLines(int minutes);

void setup() {
  pinMode(microstepPin, OUTPUT);
  pinMode(SleepPin, OUTPUT);
  // Initialize stepper motors

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  digitalWrite(SleepPin, HIGH);  //Active low: sleeps when set to low
  digitalWrite(microstepPin, HIGH); //High for 1/16, low for 1/2 microstepping
  
  tempMaxSpeed = 160;

  pinMode(joyButton, INPUT_PULLUP);
  
  Serial.begin(9600);

  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, HIGH);

  xStepper.setMaxSpeed(400);
  yStepper.setMaxSpeed(400);
  randomSeed(millis());
  xPosRange[0] = -100;
  xPosRange[1] = 100;
  yPosRange[0] = xPosRange[0];
  yPosRange[1] = xPosRange[1];

  randLines(1);
}

void loop() {
}

void randLines(int minutes){
  long endTime = millis() + minutes*60*1000; //currently doesn't work for 60, does for 10
  while(millis()<endTime){
    xyPos[0] = random(xPosRange[0], xPosRange[1] + 1);
    xyPos[1] = random(xPosRange[0], xPosRange[1] + 1);
    bothSteppers.moveTo(xyPos);
    bothSteppers.runSpeedToPosition();
    if(random(0,3)){
      xStepper.setMaxSpeed(random(200,500));
      yStepper.setMaxSpeed(random(200,500));
      delay(random(0,1000));
      if(!random(0,3)){
        delay(random(0,500));
        if(!random(0,3)){
          delay(random(0,800));
        }
      }
    }
  }
  digitalWrite(SleepPin, LOW);  //Active low: sleeps when set to low
}

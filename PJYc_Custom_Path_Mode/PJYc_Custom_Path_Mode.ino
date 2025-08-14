 
#include <AccelStepper.h>
#include <MultiStepper.h>

// Joystick Pins
// Define the pins for the joystick and servo
const int joyXpin = A2; //analog pins
const int joyYpin = A3;

// Stepper motor pins (for a 4-wire stepper using AccelStepper)
const int xDirPin = 2;
const int xStepPin = 3;
const int SleepPin = 4;

const int yDirPin = 5;
const int yStepPin = 6;
const int microstepPin = 7;
int xPosRange[2] = {-400, 400};
int yPosRange[2] = {-400, 400};
long xyPos[2] = {0,0}; //Stores the positions [x,y] for the two motors

float absMaxSpeed = 1200;
float speedSF = 1;
float tempMaxSpeed = speedSF*absMaxSpeed;

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

void setup() {
  Serial.begin(9600);

  //Old
  xStepper.setMaxSpeed(1200);
  yStepper.setMaxSpeed(1200);

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  digitalWrite(SleepPin, HIGH);  //Active low: sleeps when set to low
  digitalWrite(microstepPin, HIGH); //High for 1/16, low for 1/2 microstepping
}

void loop() {
  // Read the joystick position
  int xJoyVal = analogRead(joyXpin);
  int yJoyVal = analogRead(joyYpin);

  // Map the joystick values (0-1023) to a speed range for the stepper motors
  int joystickSpeedX = map(xJoyVal, 0, 1023, -tempMaxSpeed, tempMaxSpeed);  // Speed based on X-axis
  int joystickSpeedY = map(yJoyVal, 0, 1023, -tempMaxSpeed, tempMaxSpeed);  // Speed based on Y-axis
  //Look into nonlinear mapping.

  if (abs(xJoyVal-512) < 45){
    joystickSpeedX = 0;
  }
  if (abs(yJoyVal-512) < 45){
    joystickSpeedY = 0;
  }

  // Set the speed of the stepper motors
  xStepper.setSpeed(-joystickSpeedX);  // Set speed for xStepper
  yStepper.setSpeed(-joystickSpeedY);  // Set speed for yStepper

  // Move the motors based on the joystick input
  xStepper.runSpeed();  // Move xStepper at the set speed
  yStepper.runSpeed();  // Move yStepper at the set speed
}



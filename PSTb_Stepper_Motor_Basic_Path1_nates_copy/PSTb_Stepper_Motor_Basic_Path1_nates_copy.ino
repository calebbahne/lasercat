// Tells a Nema 17 stepper motor basic rotation commands
// Can take user input and go to a position.
#include <AccelStepper.h>
#include <MultiStepper.h>

// Stepper motor pins
const int xDirPin = 2;
const int xStepPin = 3;
const int xSleepPin = 4;

const int yDirPin = 5;
const int yStepPin = 6;
const int ySleepPin = 7;

// Position 
int xPosRange[2] = {-180, 180};
int yPosRange[2] = {-180, 180};
long xyPos[2] = {0, 0}; // Changed from int to long

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

void setup() {
  xStepper.setMaxSpeed(1200);
  yStepper.setMaxSpeed(1200);

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  digitalWrite(xSleepPin, HIGH);  // Active low: sleeps when set to low
  digitalWrite(ySleepPin, HIGH);
  Serial.begin(9600);
}

void loop() {
  // Generate random positions for x and y within specified ranges
  xyPos[0] = random(xPosRange[0], xPosRange[1]);
  xyPos[1] = random(yPosRange[0], yPosRange[1]);

  bothSteppers.moveTo(xyPos);
  bothSteppers.runSpeedToPosition(); // Removed the argument

  delay(20);
}

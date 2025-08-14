#include <AccelStepper.h>
#include <MultiStepper.h>

// Define the pins for the joystick and servo
const int joyXpin = A2; //analog pins
const int joyYpin = A3;
const int joyButton = 13;

// Define variables for joystick readings
int xJoyVal = 0; //initializes x value to be 0
int yJoyVal = 0; //initializes y value to be 0
int buttonState = 0;

const int xDirPin = 2;
const int xStepPin = 3;
const int xSleepPin = 4;

const int yDirPin = 5;
const int yStepPin = 6;
const int ySleepPin = 7;
int xPosRange[2] = {-100, 100};
int yPosRange[2] = {-100, 100};
long xyPos[2] = {0,0}; //Stores the positions [x,y] for the two motors
int prevPos[2] = {0,0};
int moveThreshold = 10;

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

void setup() {
 // laserServo.attach(servoPin); // Attach the servo to the pin
//  laserServo.write(90); // Initialize the servo to the middle position

  xStepper.setMaxSpeed(2000);
  yStepper.setMaxSpeed(2000);

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  digitalWrite(xSleepPin, HIGH);  //Active low: sleeps when set to low
  digitalWrite(ySleepPin, HIGH);
  Serial.begin(9600);
}

void loop() {
  // Read joystick positions
  xJoyVal = analogRead(joyXpin); // Read X-axis
  yJoyVal = analogRead(joyYpin); // Read Y-axis

  // Map joystick values to servo angles (0 to 180 degrees)
 xyPos[0] = map(xJoyVal, 0, 1023, xPosRange[0], xPosRange[1]); // Map X-axis
 xyPos[1] = map(yJoyVal, 0, 1023, yPosRange[0], yPosRange[1]); // Map Y-axis

 if(abs(xyPos[0]-prevPos[0]) > moveThreshold || abs(xyPos[1]-prevPos[1]) > moveThreshold) {
  bothSteppers.moveTo(xyPos);
  bothSteppers.run();
  prevPos[0] = xyPos[0];
  prevPos[1] = xyPos[1];
 }
 delay(1);
}

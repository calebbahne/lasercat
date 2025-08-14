#include <AccelStepper.h>
#include <MultiStepper.h>

// Stepper motor pins
const int xDirPin = 2;
const int xStepPin = 3;
const int xSleepPin = 4;

const int yDirPin = 5;
const int yStepPin = 6;
const int ySleepPin = 7;

// Stepper configuration
AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

void setup() {
  // Initialize Serial
  Serial.begin(9600);
  
  // Activate motors
  pinMode(xSleepPin, OUTPUT);
  pinMode(ySleepPin, OUTPUT);
  digitalWrite(xSleepPin, HIGH);  // Active low: sleeps when set to low
  digitalWrite(ySleepPin, HIGH);

  // Set initial speeds
  setSpeeds();
}

void loop() {
  // Continuously run both steppers at the set speed
  xStepper.runSpeed();
  yStepper.runSpeed();
  
  // Check for new speed input
  if (Serial.available() > 0) {
    setSpeeds();
  }
}

void setSpeeds() {
  Serial.println("Enter speed for stepper:");
  while (Serial.available() == 0) {} // Wait for user input
  int xSpeed = Serial.parseInt();
  xStepper.setMaxSpeed(xSpeed);
  xStepper.setSpeed(xSpeed);
  yStepper.setMaxSpeed(xSpeed);
  yStepper.setSpeed(xSpeed);
  Serial.print("Stepper speed set to: ");
  Serial.println(xSpeed);
}

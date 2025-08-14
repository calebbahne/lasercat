//Tells a Nema 17 stepper motor basic rotation commands
//Can take user input and go to a position.
#include<AccelStepper.h>

//Stepper motors
const int xDirPin = 2;
const int xStepPin = 3;
const int xSleepPin = 4;
int xPos = 200;

const int yDirPin = 5;
const int yStepPin = 6;
const int ySleepPin = 7;
int yPos = 200;

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);

void setup() {
	xStepper.setMaxSpeed(1000);
  xStepper.setAcceleration(5000);
  xStepper.setCurrentPosition(0);

  yStepper.setMaxSpeed(1000);
  yStepper.setAcceleration(5000);
  yStepper.setCurrentPosition(0);

  digitalWrite(xSleepPin, HIGH);  //Active low: sleeps when set to low
  digitalWrite(ySleepPin, HIGH);
  Serial.begin(9600);
}

void loop() {
  
  xStepper.moveTo(xPos);
  yStepper.moveTo(yPos);
  while (xStepper.currentPosition() != xPos || yStepper.currentPosition() != yPos){
    xStepper.run();
    yStepper.run();
  }
  delay(2000);
  xPos = xPos + 200;
  yPos = yPos -400;
  
  /*
  if (Serial.available() > 0){
    String input = Serial.readStringUntil('\n');
    xPos = input.toInt();
    xStepper.moveTo(xPos);
  }
  xStepper.run();
  */
}


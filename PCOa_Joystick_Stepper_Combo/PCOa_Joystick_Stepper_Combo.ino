//Tells a Nema 17 stepper motor to move to the position 
//given by the joystick
#include <AccelStepper.h>
#include <MultiStepper.h>

//Joystick pins
const int joyXpin = A2; //analog pins
const int joyYpin = A3;
const int joyButton = 13;

//Joystick Variables
int xJoyVal = 0; //initializes x value to be 0
int yJoyVal = 0; //initializes y value to be 0
int buttonState = 0;

//Stepper motor pins
const int xDirPin = 2;
const int xStepPin = 3;
const int xSleepPin = 4;

const int yDirPin = 5;
const int yStepPin = 6;
const int ySleepPin = 7; // ******Combine x and y sleep pins

//Position
int xPosRange[2] = {-200, 200};//200 steps per rev
int yPosRange[2] = {-200, 200};
long xyPos[2] = {0,0}; //Stores the positions [x,y] for the two motors

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

void setup() {
  xStepper.setMaxSpeed(1000);
  yStepper.setMaxSpeed(1000);

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  digitalWrite(xSleepPin, HIGH);  //Active low: sleeps when set to low
  digitalWrite(ySleepPin, HIGH);
  Serial.begin(9600);
}

void loop() {
  getJoystickValues();
  setPosToJoy();

  bothSteppers.moveTo(xyPos);
  bothSteppers.run();//make sure right

  delay(10);
}

void homeSteppers(){
  //Set both x and y steppers to 0
  //Set the status of the steppers to "Homed"
}

void getJoystickValues(){
  xJoyVal = analogRead(joyXpin);
  yJoyVal = analogRead(joyYpin);
  buttonState = digitalRead(joyButton);
  
  Serial.print("X: ");
  Serial.print(xJoyVal);
  Serial.print(" | Y: ");
  Serial.print(yJoyVal);
  Serial.print(" | Button: ");
  Serial.println(buttonState);
}

//Sets xyPos to the position indicated by the joystick
void setPosToJoy(){
  xyPos[0] = map(xJoyVal, 0, 1023, xPosRange[0], xPosRange[1]);
  xyPos[1] = map(yJoyVal, 0, 1023, yPosRange[0], yPosRange[1]);

  Serial.print("x: ");
  Serial.print(xyPos[0]);
  Serial.print(" | y: ");
  Serial.println(xyPos[1]);
}

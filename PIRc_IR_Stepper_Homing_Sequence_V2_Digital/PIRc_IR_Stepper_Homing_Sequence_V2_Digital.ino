#include <AccelStepper.h>
#include <MultiStepper.h>

//Checks for the homing indicator in front of the x and y steppers, tells when it's homed.
const int xIR_digPin = 10;
const int yIR_digPin = 9;

//Laser
const int laserPin = 11; //Needs PWM
int laserIntensity = 255;

//====Old Stuff

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
const int SleepPin = 4;

const int yDirPin = 5;
const int yStepPin = 6;
const int microstepPin = 7;
int xPosRange[2] = {-400, 400};
int yPosRange[2] = {-400, 400};
long xyPos[2] = {0,0}; //Stores the positions [x,y] for the two motors

float absMaxSpeed = 1200;//Remove if maxed out of variables
float speedSF = 1; //Might not need
float tempMaxSpeed = speedSF*absMaxSpeed;

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;
void setup()
{
  Serial.begin(9600);
  pinMode(xIR_digPin, INPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(SleepPin, OUTPUT);//=====================================
  pinMode(microstepPin, OUTPUT);//=====================
  digitalWrite(laserPin, HIGH);

  //Old
  xStepper.setMaxSpeed(2000);
  yStepper.setMaxSpeed(2000);

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  digitalWrite(SleepPin, HIGH);  //Active low: sleeps when set to low
  digitalWrite(microstepPin, HIGH); //High for 1/16, low for 1/2 microstepping
}

void loop(){
  for(int i = 0; i<= 10000; i++){
    xStepper.setSpeed(1200);  // Set speed for xStepper
  yStepper.setSpeed(1200);  // Set speed for yStepper

  // Move the motors based on the joystick input
  xStepper.runSpeed();  // Move xStepper at the set speed
  yStepper.runSpeed(); 
  }
  homeSteppers();
  Serial.println("Main loop");
}

bool homeSteppers() {
  Serial.println("Homing Steppers");
  digitalWrite(SleepPin, HIGH);
  tempMaxSpeed = 200;
  bool xHomed = 0;
  bool yHomed = 0;
  xStepper.setSpeed(tempMaxSpeed);  
  yStepper.setSpeed(tempMaxSpeed); 
  
  while(!xHomed || !yHomed){
    if(!xHomed){
      xHomed = digitalRead(xIR_digPin);
      if(xHomed){
        xStepper.setSpeed(0); 
        Serial.print("X");
      }
    }
    if(!yHomed){
      yHomed = digitalRead(yIR_digPin);
      if(yHomed){
        yStepper.setSpeed(0); 
        Serial.print("Y");
      }
    }
    xStepper.runSpeed();  // Move xStepper at the set speed
    yStepper.runSpeed();  // Move xStepper at the set speed
  }
  xStepper.setCurrentPosition(0);
  yStepper.setCurrentPosition(0);
  digitalWrite(SleepPin, LOW);
  return true;
}

void setMicrostepping(int microsteps){
  //Write microstep pin to high low
  //If microsteps = high, set up for 1/16th and redefine boundaries
  //Do same if low, 1/2
}
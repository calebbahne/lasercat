#include <AccelStepper.h>
#include <MultiStepper.h>

//Checks for the homing indicator in front of the x and y steppers, tells when it's homed.
const int xIR_anaPin = A0;//consider switching to digital; faster
const int yIR_anaPin = A1;
const int xIR_digPin = 10;
const int yIR_digPin = 9;
int xIR_anaValue = 0;
int yIR_anaValue = 0; 
int homingTol = 200;

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
float speedSF = 1;
float tempMaxSpeed = speedSF*absMaxSpeed;

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;
void setup()
{
  Serial.begin(9600);
  pinMode(xIR_anaPin,INPUT);
  pinMode(yIR_anaPin,INPUT);
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
  homeSteppers();
  Serial.println("Main loop");
}

bool homeSteppers() {
  Serial.println("Homing Steppers");
  xIR_anaValue = analogRead(xIR_anaPin);
  yIR_anaValue = analogRead(yIR_anaPin);
  
  while(xIR_anaValue > homingTol || yIR_anaValue > homingTol){
    xJoyVal = analogRead(joyXpin); // Read X-axis
    yJoyVal = analogRead(joyYpin); // Read Y-axis
    xIR_anaValue = analogRead(xIR_anaPin);
    yIR_anaValue = analogRead(yIR_anaPin);

    //Serial.println(xIR_anaValue);
    //Serial.println(yIR_anaValue);
      // Map joystick values to servo angles (0 to 180 degrees)
    xyPos[0] = map(xJoyVal, 0, 1023, xPosRange[0], xPosRange[1]); // Map X-axis
    xyPos[1] = map(yJoyVal, 0, 1023, yPosRange[0], yPosRange[1]); // Map Y-axis

    bothSteppers.moveTo(xyPos);
    bothSteppers.run();
  
    //xIR_anaValue = analogRead(xIR_anaPin);
    //yIR_anaValue = analogRead(yIR_anaPin);
  }
}

void setMicrostepping(int microsteps){
  //Write microstep pin to high low
  //If microsteps = high, set up for 1/16th and redefine boundaries
  //Do same if low, 1/2
}
//Homes the steppers by changing position, not speed.
#include <AccelStepper.h>
#include <MultiStepper.h>

//Checks for the homing indicator in front of the x and y steppers, tells when it's homed.
//const int xIR_digPin = 10;
//const int yIR_digPin = 9;
const int xIR_anaPin = A0;//consider switching to digital; faster
const int yIR_anaPin = A1;

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
int i = 1;
void setup()
{
  Serial.begin(9600);
  //pinMode(xIR_digPin, INPUT);
  pinMode(xIR_anaPin,INPUT);
  pinMode(yIR_anaPin,INPUT);
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
  //homeSteppers();
    xStepper.setSpeed(1000);
}

void loop(){
  while(true){
    xStepper.moveTo(xStepper.currentPosition()+190);
    xStepper.setSpeed(1000);
    xStepper.runSpeed();
  }
  //doStuff();
}

void run(){
  xStepper.setSpeed(1000);  // Set speed to 1000 steps per second
    for (int i = 0; i < 1000; i++) {
      xStepper.run();  // Move the motor at the set speed
    }
    delay(500);
}

void forLoopMove(){
  //.run() takes a while to get going. You can't delay in between it though or it restarts
    // okay to do a delay outside of calls to .run, but really slows it down.
    xStepper.setSpeed(1000);  // Set speed to 1000 steps per second
    
    // Move the stepper 10 steps
    xStepper.move(1000);  // Move 10 steps forward
    for (int i = 0; i < 1000; i++) {
      xStepper.run();  // Move the motor at the set speed
    }
          delay(10);
}

void doStuff(){
  //for (int i = 0; i < 3; i++) {
    //int stepIncrement = 50;
    //Serial.println("X motor moving");
    xStepper.setSpeed(tempMaxSpeed);  
    yStepper.setSpeed(tempMaxSpeed); 
    //xStepper.move(stepIncrement);  // Move a small increment for X axis
    //while (true) {                                                              //******Caused the problem
      xStepper.runSpeed();  // Call run() repeatedly to move the motor
    //}
  //}
}

bool homeSteppers() {
  digitalWrite(SleepPin, HIGH);
  Serial.println("Homing Steppers");
  int xIR_anaValue = 1000;
  int yIR_anaValue = 1000; 
  int homingTol = 300;
  int stepIncrement = 50;
  bool xHomed = 0;
  bool yHomed = 0;
  tempMaxSpeed = 1200;
  xStepper.setSpeed(tempMaxSpeed);  
  yStepper.setSpeed(tempMaxSpeed); 
  delay(500);

        xStepper.move(stepIncrement);  // Move a small increment for X axis
        while (true) {
          xStepper.runSpeed();  // Call run() repeatedly to move the motor
        }
        xStepper.stop();
  
        while (true) {
            yStepper.runSpeed();
        }
        yStepper.stop();
}
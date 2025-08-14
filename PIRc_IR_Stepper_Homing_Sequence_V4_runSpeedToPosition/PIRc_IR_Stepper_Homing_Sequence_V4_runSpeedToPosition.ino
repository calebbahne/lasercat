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
  homeSteppers();
}

void loop(){
}

bool homeSteppers() {
  digitalWrite(SleepPin, HIGH);
  Serial.println();
  Serial.println("Homing Steppers");
  int xIR_anaValue = 1000;
  int yIR_anaValue = 1000; 
  int homingTol = 80;
  int increment = 100;
  bool xHomed = 0;
  bool yHomed = 0;

  delay(500);

  while (!xHomed || !yHomed) {
    xIR_anaValue = analogRead(xIR_anaPin);
    yIR_anaValue = analogRead(yIR_anaPin);

    // Move x motor if not yet homed
    if (!xHomed) {
      //Serial.println("X motor moving");
      xStepper.move(increment);
      xStepper.runSpeedToPosition();
      // Update IR sensor readings
      xIR_anaValue = analogRead(xIR_anaPin);
  
      // Check if X IR sensor is within homing tolerance
      if (xIR_anaValue <= homingTol) {
        delay(20);  // Small delay to debounce
        xIR_anaValue = analogRead(xIR_anaPin);  // Read again to confirm
        Serial.print("X3: ");
        Serial.println(xIR_anaValue);

        if (xIR_anaValue <= 300) {  // Check if sensor value is below threshold
          xStepper.setCurrentPosition(0);  // Homing complete for X
          xHomed = 1;
          Serial.println("X Axis Homed");
        }
      }
    }
    // Move y motor if not yet homed
    if (!yHomed) {
      //yStepper.move(stepIncrement);  // Move a small increment for Y axis
      yStepper.move(increment);
      yStepper.runSpeedToPosition();

      yIR_anaValue = analogRead(yIR_anaPin);
      //Serial.print("Y2: ");
      //Serial.println(yIR_anaValue);

      // Check if Y IR sensor is within homing tolerance
      if (yIR_anaValue <= homingTol) {
        delay(20);  // Small delay to debounce
        yIR_anaValue = analogRead(yIR_anaPin);  // Read again to confirm
        Serial.print("Y3: ");
        Serial.println(yIR_anaValue);

        if (yIR_anaValue <= homingTol) {  // Check if sensor value is below threshold
          yStepper.setCurrentPosition(0);  // Homing complete for Y
          yHomed = 1;
          Serial.println("Y Axis Homed");
        }
      }
    }
  }

  Serial.println("Both steppers homed.");
  digitalWrite(SleepPin, LOW);  // Disable stepper driver sleep mode if necessary
  return true;
}

void setMicrostepping(int microsteps){
  //Write microstep pin to high low
  //If microsteps = high, set up for 1/16th and redefine boundaries
  //Do same if low, 1/2
}
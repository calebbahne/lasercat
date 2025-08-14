#include <AccelStepper.h>
#include <MultiStepper.h>
#include <math.h> // Include for sin() and cos()

// IR Sensors
#define xIR_anaPin A0
#define yIR_anaPin A1

const int laserPin = 11;

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

bool homeSteppers(int homeSpeed = 100);
void setup()
{
  Serial.begin(9600);
  pinMode(SleepPin, OUTPUT);//=====================================
  pinMode(microstepPin, OUTPUT);//=====================
  pinMode(laserPin, OUTPUT);

  digitalWrite(laserPin, HIGH);

  //Old
  xStepper.setMaxSpeed(1200);
  yStepper.setMaxSpeed(1200);

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  digitalWrite(SleepPin, HIGH);  //Active low: sleeps when set to low
  digitalWrite(microstepPin, HIGH); //High for 1/16, low for 1/2 microstepping

  delay(1000);
  homeSteppers(1000);
  digitalWrite(SleepPin, HIGH); 
  //square(20);
}

void loop(){

  //for(int i = 0; i < 100; i++){
    //bounceSteppersX();
  //}
    /*
  for(int i = 0; i < 100; i++){
    bounceSteppersY();
  }
  */
  //shrinkingSquare(20); //20 rectangles drawn
  drawSquareSpiral(60, 1, false); //sf = scale factor, works good at 1
  bounceSteppers();
  drawSquareSpiral(80, 1, true); //spirals
  drawCircleSpiral(20, 1, true);  //spirals
}

/*
void bounceSteppersX(){ //need to be called multiple times
  long bouncePos = xPosRange[1]/20;
  xStepper.moveTo(bouncePos);
  yStepper.setSpeed(100);
  for(int i = 0; i <= 50; i++){
    if(xStepper.distanceToGo() == 0){
      xStepper.moveTo(-xStepper.currentPosition());
    }
    xStepper.run();
    yStepper.runSpeed();
  }
  yStepper.stop();
}
*/

void bounceSteppers() { 
  digitalWrite(microstepPin, HIGH); // Set to 1/2 stepping for faster motion

  for (int i = 0; i < 1; i++) {
    for (int h = 0; h < 30; h++) { // Move right (slower horizontal movement)
      xStepper.setSpeed(25); // Slow rightward motion
      for (int v = 0; v < 2000; v++) { // Vertical line up (4x taller)
        yStepper.setSpeed(1000); 
        xStepper.runSpeed();
        yStepper.runSpeed();
      }
      for (int v = 0; v < 2000; v++) { // Vertical line down (4x taller)
        yStepper.setSpeed(-1000);
        xStepper.runSpeed();
        yStepper.runSpeed();
      }
    }

    for (int h = 0; h < 30; h++) { // Move left (slower horizontal movement)
      xStepper.setSpeed(-25); // Slow leftward motion
      for (int v = 0; v < 2000; v++) { // Vertical line up (4x taller)
        yStepper.setSpeed(1000);
        xStepper.runSpeed();
        yStepper.runSpeed();
      }
      for (int v = 0; v < 2000; v++) { // Vertical line down (4x taller)
        yStepper.setSpeed(-1000);
        xStepper.runSpeed();
        yStepper.runSpeed();
      }
    }
  }
}

/*
void drawSquare(int numSquares){ //numSquares = # of squares drawn
  digitalWrite(microstepPin, HIGH); //Go at 1/2 stepping, faster

  //may need to move to zero first.

  tempMaxSpeed = 1000;

  //numPoints should increase when maxSpeed is low, decrease when it's high

  int numPoints = numSquares*4*1; //4 lines per square, 100 points per line
  for(int i = 0; i < numSquares; i++){//make this many full squares
    for(int j = numPoints; j > 0; j--){ //make this many points, split into four lines
      if(j > 3*numPoints/4){
        //Horiz, moving up
        xStepper.setSpeed(tempMaxSpeed);
        yStepper.setSpeed(0);
      }
      if(j <= 3*numPoints/4 && j > numPoints/2){
        //Vert, tiny left
        xStepper.setSpeed(0);
        yStepper.setSpeed(tempMaxSpeed);
      }
      if(j <= numPoints/2 && j > numPoints/4){
        //left, tiny down
        xStepper.setSpeed(-tempMaxSpeed);
        yStepper.setSpeed(0);
      }
      if(j <= numPoints/4 && j > 0){
        //Down, tiny right
        xStepper.setSpeed(0);
        yStepper.setSpeed(-tempMaxSpeed);
      }
      xStepper.runSpeed();
      yStepper.runSpeed();
      delayMicroseconds(200);
      //tempMaxSpeed = tempMaxSpeed - tempMaxSpeed*10/numPoints;
    }
  }
}
*/

void drawSquareSpiral(int numSquares, int sf, bool spiral) { 
  digitalWrite(microstepPin, HIGH); // Set to 1/2 stepping for faster motion

  tempMaxSpeed = 1000;
  float scaleFactor = 1.0*sf;
  // Calculate sinusoidal scaling factor to grow/shrink square size

  int numPoints = numSquares * 4 * 1; // 4 lines per square
  for (int i = 0; i < numSquares; i++) { // Draw this many full squares
    if(spiral){
      scaleFactor = 0.5 + 0.5 * sin((2 * PI / 5000.0) * millis()); // Oscillates between 0 and 1
    }
    for (int j = numPoints; j > 0; j--) { // Draw each side of the square
      if (j > 3 * numPoints / 4) {
        // Rightward (positive x)
        xStepper.setSpeed(tempMaxSpeed * scaleFactor);
        yStepper.setSpeed(0);
      } 
      else if (j <= 3 * numPoints / 4 && j > numPoints / 2) {
        // Upward (positive y)
        xStepper.setSpeed(0);
        yStepper.setSpeed(tempMaxSpeed * scaleFactor);
      } 
      else if (j <= numPoints / 2 && j > numPoints / 4) {
        // Leftward (negative x)
        xStepper.setSpeed(-tempMaxSpeed * scaleFactor);
        yStepper.setSpeed(0);
      } 
      else if (j <= numPoints / 4 && j > 0) {
        // Downward (negative y)
        xStepper.setSpeed(0);
        yStepper.setSpeed(-tempMaxSpeed * scaleFactor);
      }

      xStepper.runSpeed();
      yStepper.runSpeed();
      delayMicroseconds(200);
    }
  }
}


/*
void drawSquare(int size) {
  // Move to draw the square
  xStepper.move(size); // Move right
  xStepper.runToPosition();

  yStepper.move(size); // Move up
  yStepper.runToPosition();

  xStepper.move(-size); // Move left
  xStepper.runToPosition();

  yStepper.move(-size); // Move down
  yStepper.runToPosition();
}
*/

/*
void drawCircle(long numCircles) {
  digitalWrite(microstepPin, HIGH); // Set to 1/2 stepping for faster motion

  long tempMaxSpeed = 1200;  // Maximum speed
  int pointsPerCircle = 360; // Number of points in one circle

  for (int i = 0; i < numCircles; i++) {
    for (int j = 0; j < pointsPerCircle; j++) {
      // Calculate the angle for this point
      float angle = j * (360.0 / pointsPerCircle); // Angle in degrees

      // Set speeds for the stepper motors
      xStepper.setSpeed(cos(angle * (PI / 180.0)) * tempMaxSpeed);
      yStepper.setSpeed(sin(angle * (PI / 180.0)) * tempMaxSpeed);

      // Run steppers for one point
      xStepper.runSpeed();
      yStepper.runSpeed();
    }
  }
}
*/

void drawCircleSpiral(long numCircles, int sf, bool spiral) {
  digitalWrite(microstepPin, HIGH); // Set to 1/2 stepping for faster motion

  long tempMaxSpeed = 1200;  // Maximum speed
  int pointsPerCircle = 360; // Number of points in one circle
  float scaleFactor = 1.0*sf;

  for (int i = 0; i < numCircles; i++) {
    for (int j = 0; j < pointsPerCircle; j++) {
      // Calculate the angle for this point
      float angle = j * (360.0 / pointsPerCircle); // Angle in degrees

      // ** New: Calculate the scaling factor for radius over time (sinusoidal growth and shrinkage) **
      if(spiral){
        scaleFactor = 0.5 + 0.5 * sin((2 * PI / 5000.0) * millis()); // Scales between 0 and 1
      }

      // Set speeds for the stepper motors, with the radius scaled by scaleFactor
      xStepper.setSpeed(cos(angle * (PI / 180.0)) * tempMaxSpeed * scaleFactor);
      yStepper.setSpeed(sin(angle * (PI / 180.0)) * tempMaxSpeed * scaleFactor);

      // Run steppers for one point
      xStepper.runSpeed();
      yStepper.runSpeed();
    }
  }
}


bool homeSteppers(int homeSpeed = 100) { //Defaults to 100
  digitalWrite(SleepPin, HIGH);
  
  int xIR_anaValue = 1000;
  int yIR_anaValue = 1000;
  int homingTol = 80;
  long nextPos = 0;
  int increment = -10;
  bool xHomed = false;
  bool yHomed = false;
  delay(200);

  xStepper.setSpeed(600); //Send steppers to consistent starting position to increase repeatibility
  yStepper.setSpeed(600);

  while (!xHomed || !yHomed) {

    xIR_anaValue = analogRead(xIR_anaPin);
    yIR_anaValue = analogRead(yIR_anaPin);

    // Move x motor if not yet homed
    if (!xHomed) {
      nextPos = xStepper.currentPosition() + increment;
      while (xStepper.currentPosition() > nextPos) {
        xStepper.setSpeed(-homeSpeed);
        xStepper.runSpeed();  // Move the motor
      }
      xStepper.stop();
      
      // Update IR sensor readings
      xIR_anaValue = analogRead(xIR_anaPin);

      // Check if X IR sensor is within homing tolerance
      if (xIR_anaValue <= homingTol) {
        delay(20);  // Small delay to debounce
        xIR_anaValue = analogRead(xIR_anaPin);  // Read again to confirm
        Serial.print(F("X3: "));
        Serial.println(xIR_anaValue);

        if (xIR_anaValue <= 300) {  // Check if sensor value is below threshold
          xStepper.setCurrentPosition(0);  // Homing complete for X
          xHomed = true;
          Serial.println(F("X Axis Homed"));
        }
      }
    }

    // Move y motor if not yet homed
    if (!yHomed) {
      // yStepper.move(stepIncrement);  // Move a small increment for Y axis
      nextPos = yStepper.currentPosition() + increment;
      while (yStepper.currentPosition() > nextPos) {
        yStepper.setSpeed(-homeSpeed);
        yStepper.runSpeed();
      }
      yStepper.stop();

      yIR_anaValue = analogRead(yIR_anaPin);

      // Check if Y IR sensor is within homing tolerance
      if (yIR_anaValue <= homingTol) {
        delay(20);  // Small delay to debounce
        yIR_anaValue = analogRead(yIR_anaPin);  // Read again to confirm
        Serial.print(F("Y3: "));
        Serial.println(yIR_anaValue);

        if (yIR_anaValue <= 300) {  // Check if sensor value is below threshold
          yStepper.setCurrentPosition(0);  // Homing complete for Y
          yHomed = true;
          Serial.println(F("Y Axis Homed"));
        }
      }
    }
  }

  delay(500);
  xyPos[0] = 360; //Homing Offset 400
  xyPos[1] = 280; //Homing offset 320
  bothSteppers.moveTo(xyPos);
  bothSteppers.runSpeedToPosition();

  xStepper.setCurrentPosition(0);
  yStepper.setCurrentPosition(0);

  delay(500); //remove

  //bothSteppers.moveTo(xyPos);
  //bothSteppers.runSpeedToPosition();
  digitalWrite(SleepPin, LOW);  // Disable stepper driver sleep mode if necessary
  return true;
}


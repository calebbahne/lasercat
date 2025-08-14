#include <AccelStepper.h>
#include <MultiStepper.h>
#include <math.h> // Include for sin() and cos()

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
  drawSquare(20);
  //drawTriangle(20);
  //drawCircle(20);
  
}

void bounceSteppersX(){ //need to be called multiple times -- add the for loop.
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

void bounceSteppersY(){
  long bouncePos = yPosRange[1]*0.5;
  yStepper.moveTo(bouncePos);
  xStepper.setSpeed(100);
  for(int i = 0; i <= 50; i++){
    if(yStepper.distanceToGo() == 0){//yStepper.currentPosition()-bouncePos > 10
      yStepper.moveTo(-yStepper.currentPosition());
    }
    yStepper.run();
    xStepper.runSpeed();
  }
  xStepper.stop();
}

void shrinkingSquare(long numSquares){ //numSquares = # of squares drawn
  digitalWrite(microstepPin, LOW); //Go at 1/2 stepping, faster

  //may need to move to zero first.

  tempMaxSpeed = 600;

  //numPoints should increase when maxSpeed is low, decrease when it's high

  int numPoints = numSquares*4*20; //4 lines per square, 100 points per line
  for(int i = 0; i < numSquares; i++){//make this many full squares
    tempMaxSpeed = tempMaxSpeed - 10;  
    for(int j = numPoints; j > 0; j--){ //make this many points, split into four lines
      if(j > 3*numPoints/4){
        //Horiz, moving up
        xStepper.setSpeed(tempMaxSpeed);
        yStepper.setSpeed(50);
      }
      if(j <= 3*numPoints/4 && j > numPoints/2){
        //Vert, tiny left
        xStepper.setSpeed(-50);
        yStepper.setSpeed(tempMaxSpeed);
      }
      if(j <= numPoints/2 && j > numPoints/4){
        //left, tiny down
        xStepper.setSpeed(-tempMaxSpeed);
        yStepper.setSpeed(-50);
      }
      if(j <= numPoints/4 && j > 0){
        //Down, tiny right
        xStepper.setSpeed(50);
        yStepper.setSpeed(-tempMaxSpeed);
      }
      xStepper.runSpeed();
      yStepper.runSpeed();
      //tempMaxSpeed = tempMaxSpeed - tempMaxSpeed*10/numPoints;
    }
  }
}

void drawSquare(int numSquares){ //numSquares = # of squares drawn
  digitalWrite(microstepPin, LOW); //Go at 1/2 stepping, faster

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

/*
void drawSquare(long numSquares) { 
  digitalWrite(microstepPin, LOW); // Go at 1/2 stepping, faster

  // Define initial speed and size parameters
  long tempMaxSpeed = 100;
  int size = 0;          // Initial size in points per line

  for (int i = 0; i < numSquares; i++) { // Make this many full squares
    int numPoints = size * 4; // 4 lines per square

    for (int j = numPoints; j > 0; j--) { // Draw points for this square
      if (j > 3 * numPoints / 4) {
        // Horizontal, moving up
        xStepper.move(size);
        yStepper.move(0);
      } else if (j <= 3 * numPoints / 4 && j > numPoints / 2) {
        // Vertical, moving left
        xStepper.move(0);
        yStepper.move(size);
      } else if (j <= numPoints / 2 && j > numPoints / 4) {
        // Left, moving down
        xStepper.move(-size);
        yStepper.move(0);
      } else if (j <= numPoints / 4 && j > 0) {
        // Down, moving right
        xStepper.move(0);
        yStepper.move(-size);
      }

      // Run steppers to create movement
      xStepper.run();
      yStepper.run();
    }

    // Adjust the size for the next square
    if (i < numSquares/2) {
      size += 5;
    } 
    else {
      size -= 5;
    }
  }
}
*/

void drawTriangle(int numTriangles) { // numTriangles = # of triangles drawn
  digitalWrite(microstepPin, LOW); // Set to 1/2 stepping for faster motion

  tempMaxSpeed = 1000; // Maximum speed

  // Calculate points for triangle: 3 sides per triangle, 100 points per side
  int numPoints = numTriangles * 3;

  for (int i = 0; i < numTriangles; i++) { // Draw this many full triangles
    for (int j = numPoints; j > 0; j--) { // Create points for each triangle
      if (j > 2 * numPoints / 3) {
        // Side 1: Move diagonally up and right
        xStepper.setSpeed(tempMaxSpeed/2);
        yStepper.setSpeed(tempMaxSpeed/2);
      } else if (j <= 2 * numPoints / 3 && j > numPoints / 3) {
        // Side 2: Move diagonally down and right
        xStepper.setSpeed(tempMaxSpeed/2);
        yStepper.setSpeed(-tempMaxSpeed/2);
      } else if (j <= numPoints / 3 && j > 0) {
        // Side 3: Move horizontally left
        xStepper.setSpeed(-tempMaxSpeed);
        yStepper.setSpeed(0);
      }

      // Run the motors for each point
      xStepper.runSpeed();
      yStepper.runSpeed();
      delayMicroseconds(200);
    }
  }
}

void drawCircle(long numCircles) { // numCircles = # of circles to draw
  digitalWrite(microstepPin, LOW); // Set to 1/2 stepping for faster motion

  long tempMaxSpeed = 200;  // Maximum speed

  for (int i = 0; i < numCircles; i++) { // Draw this many full circles
    for (int j = 0; j < 360; j++) { // Loop through 360 degrees, 360 = points per circle. 

      // Set speeds for the stepper motors
      xStepper.setSpeed(cos(j * (PI / 180.0))*tempMaxSpeed);
      yStepper.setSpeed(sin(j * (PI / 180.0))*tempMaxSpeed);

      // Run steppers for one point
      xStepper.runSpeed();
      yStepper.runSpeed();
    }
  }
}

//Can do a random point path with no delay in between each of these, send it back to 0,0 before doing the shape?

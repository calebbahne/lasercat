#include "Random2DCurve.h"
//Adjustments:

//1. Library should be able to return the value of points (xi, yi). Generate it all first, then have a [xi, yi] = xyPos = getPoints() thing
//2. Scale the points by xTempBoundary and yTempBoundary

// Need to adjust Random2DCurve to go between xmin and xmax, ymin and ymax
int xPosRange[2] = {-180, 180};   //May need to make long
int yPosRange[2] = {-180, 180};
int xTempBoundary[2] = {-180, 180};
int yTempBoundary[2] = {-180, 180};
long xyPos[2] = {0,0}; //Stores the positions [x,y] for the two motors

//Path generator
unsigned int numPoints = 200;         // Number of points to generate
float curviness = 7.5;                // Curviness of the path (higher number = more curvy)
unsigned long sampleIntervalMs = 100; // Delay between points when printing

// Create the curve object
Random2DCurve curve(xTempBoundary[1], yTempBoundary[1]);

void setup() {
    Serial.begin(9600); // Initialize serial communication
}

void loop() {

  //Be able to go:
  //curve.generatePoints(numPoints, curviness, sampleIntervalMs);
  //  Ideally, should generate all the points at once, say "generating path"
  //while true
  //xyPos = getPoints();
  //bothSteppers.moveTo(xyPos);
  //bothSteppers.run();//make sure right
  //would a delay be blocking to the steppers?


    // Generate a new path
    curve.generatePoints(numPoints, curviness, sampleIntervalMs);

    // Print all points
    curve.printPoints();

    // Access the arrays if needed
    float* xPoints = curve.getXPoints();
    float* yPoints = curve.getYPoints();

    delay(5000); // Wait before generating a new path
}

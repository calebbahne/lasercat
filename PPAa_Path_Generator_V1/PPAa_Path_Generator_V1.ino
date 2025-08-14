#include "Random2DCurve.h"

// Define maxX and maxY
float maxX = 10.0;
float maxY = 10.0;

// Create the curve object
Random2DCurve curve(maxX, maxY);

void setup() {
    Serial.begin(9600); // Initialize serial communication
}

void loop() {
    unsigned int numPoints = 200;         // Number of points to generate
    float curviness = 7.5;                // Curviness of the path (higher number = more curvy)
    unsigned long sampleIntervalMs = 100; // Delay between points when printing

    // Generate a new path
    curve.generatePoints(numPoints, curviness, sampleIntervalMs);

    // Print all points
    curve.printPoints();

    // Access the arrays if needed
    float* xPoints = curve.getXPoints();
    float* yPoints = curve.getYPoints();

    delay(5000); // Wait before generating a new path
}

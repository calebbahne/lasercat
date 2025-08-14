#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <EEPROM.h>

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Buzzer Definition
const int SpeakerPin = 12;  // Pin for the speaker
bool clickable = false;     // Change this variable to test the sounds
const int laserPin = 11;

// Joystick Definitions
const int joyButton = 13;  // Joystick button
const int joyXpin = A3;    // Joystick X axis
const int joyYpin = A2;    // Joystick Y axis

int xJoyVal = 0;
int yJoyVal = 0;
int buttonState = 0;

//Stepper motor pins
const int xDirPin = 2;
const int xStepPin = 3;
const int SleepPin = 4;

const int yDirPin = 5;
const int yStepPin = 6;
const int microstepPin = 7;

// Cat Selection Variables
const String catNames[] = {
  "V Garfield",
  "V Pete the Cat",
  "V Cat in the Hat",
  "V Tigger",
  "V Puss in Boots",
  "V Main Menu"
};
const int numCats = sizeof(catNames) / sizeof(catNames[0]);
int currentCat = 0;

//Position
int xPosRange[2] = { -400, 400 };
int yPosRange[2] = { -400, 400 };
long xyPos[2] = { 0, 0 };  //Stores the positions [x,y] for the two motors

float absMaxSpeed = 1200;//Remove if maxed out of variables
float speedSF = 1;
float tempMaxSpeed = speedSF*absMaxSpeed;

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

//IR Sensors
const int xIR_anaPin = A0;
const int yIR_anaPin = A1;

// Constants for path generation
const int SEGMENT_AMOUNT = 100; // Number of segments in the path 100
const int NUM_CURVE_POINTS = 20; // Number of points for curved segments 20
const int NUM_STRAIGHT_POINTS = 20; // Number of points for the final straight segment 20

// Structure to represent a point in 2D space
struct Point {
    float x;
    float y;
};

// Variables to hold the current target and center points
Point currentTarget; // Current target point for the segment
Point currentCenter; // Current center point for the segment
Point startPosition; // Starting position for the path

// Function Prototypes
void generatePath(Point* start = nullptr); // Generate the path based on a starting point
void transmitData(const Point& start, const Point& target, const Point& center, const Point& nextTarget); // Transmit generated data via Serial
Point generateRandomPoint(); // Generate a random point within defined bounds
void generateStraightLine(const Point& start, const Point& end, int numPoints); // Generate points for a straight line
void generateCurve(const Point& start, const Point& end, const Point& control, int numPoints); // Generate points for a curve
void delayForReadability(); // Delay for output readability

void setup() {
    Serial.begin(9600); // Start Serial communication at 9600 baud rate
    //Stepper stuff
    xStepper.setMaxSpeed(1200);
    yStepper.setMaxSpeed(1200);

    pinMode(SleepPin, OUTPUT);
    pinMode(microstepPin, OUTPUT);
    digitalWrite(SleepPin, LOW);  //Active low: sleeps when set to low
    digitalWrite(microstepPin, HIGH); //High for 1/16, low for 1/2 microstepping

    Serial.println("The code is starting");

    bothSteppers.addStepper(xStepper);
    bothSteppers.addStepper(yStepper);

    //Path stuff
    while (!Serial) { ; } // Wait for Serial to be ready (only needed on some boards)

    delay(10); // Optional delay for variability
    randomSeed(millis() + analogRead(0)); // Seed random number generator with millis and analog reading

    generatePath(); // Generate the path; can pass a specific start point if needed
}

void loop() {
    // Empty loop; no continuous tasks required
}

void generatePath(Point* start = nullptr) {
  digitalWrite(SleepPin, HIGH);  //Active low: sleeps when set to low
    startPosition = start ? *start : generateRandomPoint();
    
    currentTarget = generateRandomPoint();
    
    currentCenter = {
        (startPosition.x + currentTarget.x) / 2.0,
        (startPosition.y + currentTarget.y) / 2.0
    };

    // Send graph range and ranges before starting point generation
    Serial.println("GRAPH_RANGE:");
    Serial.print("X_RANGE: ");
    Serial.print(xPosRange[0]);
    Serial.print(" to ");
    Serial.println(xPosRange[1]);

    Serial.print("Y_RANGE: ");
    Serial.print(yPosRange[0]);
    Serial.print(" to ");
    Serial.println(yPosRange[1]);

    Serial.println("POINTS_STARTED");

    generateStraightLine(startPosition, currentCenter, NUM_STRAIGHT_POINTS);

    for (int seg = 0; seg < SEGMENT_AMOUNT; ++seg) {
        Point nextTarget = generateRandomPoint();
        
        Point nextCenter = {
            (currentTarget.x + nextTarget.x) / 2.0,
            (currentTarget.y + nextTarget.y) / 2.0
        };

        // Generate a curve from currentCenter to nextCenter using currentTarget as the control point
        generateCurve(currentCenter, nextCenter, currentTarget, NUM_CURVE_POINTS);
        
        currentTarget = nextTarget;
        currentCenter = nextCenter;
    }

    generateStraightLine(currentCenter, currentTarget, NUM_STRAIGHT_POINTS);

    Serial.println("POINTS_ENDED");
}

Point generateRandomPoint() {
    // Generate a random point within defined boundaries
    Point p;
    p.x = static_cast<float>(random(xPosRange[0], xPosRange[1] + 1)); // Random X coordinate
    p.y = static_cast<float>(random(yPosRange[0], yPosRange[1] + 1)); // Random Y coordinate
    return p; // Return the generated point
}

void generateStraightLine(const Point& start, const Point& end, int numPoints) {
    // Generate points for a straight line from start to end
    for (int i = 0; i <= numPoints; ++i) {
        float t = (float)i / numPoints; // Calculate parameter t
        float x = start.x + t * (end.x - start.x); // Interpolate X coordinate
        float y = start.y + t * (end.y - start.y); // Interpolate Y coordinate
        moveSteppers(x, y);
        delayForReadability(); // Delay for better readability of output
    }
}

void generateCurve(const Point& start, const Point& end, const Point& control, int numPoints) {
    // Generate points for a quadratic Bézier curve defined by start, end, and control points
    for (int i = 0; i <= numPoints; ++i) {
        float t = (float)i / numPoints; // Calculate parameter t
        // Calculate the curve point using the Bézier formula
        float x = pow(1 - t, 2) * start.x + 2 * (1 - t) * t * control.x + pow(t, 2) * end.x;
        float y = pow(1 - t, 2) * start.y + 2 * (1 - t) * t * control.y + pow(t, 2) * end.y;
        moveSteppers(x, y);
        delayForReadability(); // Delay for better readability of output
    }
}

void delayForReadability() {
    delay(10); // Delay for 20 milliseconds
}

void moveSteppers(float x, float y){
  xyPos[0] = (long) x;
  xyPos[1] = (long) x;

  xStepper.moveTo(xyPos[0]);
  yStepper.moveTo(xyPos[1]);
  xStepper.run();//make sure right
  yStepper.run();//make sure right

  /*
  Serial.print("(");
  Serial.print(xyPos[0]); // Print X coordinate with 2 decimal places
  Serial.print(",");
  Serial.print(xyPos[1]); // Print Y coordinate with 2 decimal places
  Serial.println(")");
  */
}
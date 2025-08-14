#include <AccelStepper.h>
#include <MultiStepper.h>

// Define the pins for the joystick and servo
const int joyXpin = A2; //analog pins
const int joyYpin = A3;
const int joyButton = 13; // Button pin for saving position

// Define variables for joystick readings
int xJoyVal = 0; //initializes x value to be 0
int yJoyVal = 0; //initializes y value to be 0
int buttonState = 0; // To store button press state

// Define stepper motor pins
const int xDirPin = 2;
const int xStepPin = 3;

const int yDirPin = 5;
const int yStepPin = 6;
const int SleepPin = 4;
const int microstepPin = 7;

// Joystick position range
int xPosRange[2] = {-180, 180};
int yPosRange[2] = {-180, 180};
long xyPos[2] = {0, 0}; // Stores the positions [x, y] for the two motors

// Create stepper objects
AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

// Variables to store the saved stepper positions (4 points)
long savedPositions[4][2]; // Array to store 4 positions: [0] = X, [1] = Y
int pressCount = 0; // Count of button presses

// Variables to define the square boundary
long xMin = -180;
long xMax = 180;
long yMin = -180;
long yMax = 180;

float absMaxSpeed = 1200;//Remove if maxed out of variables
float speedSF = 1;
float tempMaxSpeed = speedSF*absMaxSpeed;

bool clickable = true;
const int SpeakerPin = 12;

void setup() {
  pinMode(microstepPin, OUTPUT);
  pinMode(SleepPin, OUTPUT);
  // Initialize stepper motors
  xStepper.setMaxSpeed(1200);
  yStepper.setMaxSpeed(1200);

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  digitalWrite(SleepPin, HIGH);  //Active low: sleeps when set to low
  digitalWrite(microstepPin, HIGH); //High for 1/16, low for 1/2 microstepping
  
  tempMaxSpeed = 160;

  pinMode(joyButton, INPUT_PULLUP);
  
  Serial.begin(9600);
}

void loop() {
  // Read joystick positions
  xJoyVal = analogRead(joyXpin); // Read X-axis
  yJoyVal = analogRead(joyYpin); // Read Y-axis

  // Map joystick values to motor positions
  int joystickSpeedX = map(xJoyVal, 0, 1023, -tempMaxSpeed, tempMaxSpeed);  
  int joystickSpeedY = map(yJoyVal, 0, 1023, -tempMaxSpeed, tempMaxSpeed);

  if (abs(xJoyVal-512) < 45){
    joystickSpeedX = 0;
  }
  if (abs(yJoyVal-512) < 45){
    joystickSpeedY = 0;
  }

  // Set the speed of the stepper motors
  xStepper.setSpeed(-joystickSpeedX);  // Set speed for xStepper
  yStepper.setSpeed(-joystickSpeedY);  // Set speed for yStepper

  // Move the motors based on the joystick input
  xStepper.runSpeed();  // Move xStepper at the set speed
  yStepper.runSpeed();  // Move yStepper at the set speed

  // If the joystick button is pressed, save the current position of the steppers
  if (isButtonReleased()) {
    setBoundaries();
  }

  //delay(1); // Small delay for stability
}

// Function to save the current stepper positions and define square boundaries
void setBoundaries() {
  // Save the current position of the stepper motors when the button is pressed
  savedPositions[pressCount][0] = xStepper.currentPosition();  // Save X position
  savedPositions[pressCount][1] = yStepper.currentPosition();  // Save Y position
  
  // Print saved position to Serial Monitor
  Serial.print("Saved Position ");
  Serial.print(pressCount + 1);
  Serial.print(": X=");
  Serial.print(savedPositions[pressCount][0]);
  Serial.print(", Y=");
  Serial.println(savedPositions[pressCount][1]);

  // Increment the press count
  pressCount++;

  // After 4 button presses, calculate the boundaries of the square
  if (pressCount == 4) {
    // Find the min and max X and Y values from the saved positions
    xMin = xMax = savedPositions[0][0];
    yMin = yMax = savedPositions[0][1];

    // Loop through the saved positions and calculate the min/max
    for (int i = 1; i < 4; i++) {
      if (savedPositions[i][0] < xMin) xMin = savedPositions[i][0];
      if (savedPositions[i][0] > xMax) xMax = savedPositions[i][0];
      if (savedPositions[i][1] < yMin) yMin = savedPositions[i][1];
      if (savedPositions[i][1] > yMax) yMax = savedPositions[i][1];
    }

    // Print the final square boundaries to the Serial Monitor
    Serial.print("Square Boundary Set: ");
    Serial.print("X Min=");
    Serial.print(xMin);
    Serial.print(", X Max=");
    Serial.print(xMax);
    Serial.print(", Y Min=");
    Serial.print(yMin);
    Serial.print(", Y Max=");
    Serial.println(yMax);

    // Reset press count for next round of boundary setting
    pressCount = 0;
  }
}

bool isButtonReleased() {
  static bool lastState = HIGH;
  bool buttonState = digitalRead(joyButton);

  // Check for button release
  if (buttonState == HIGH && lastState == LOW) {
    lastState = HIGH;
    buzzer(clickable);  // Play sound based on clickable state
    return true;
  }

  lastState = buttonState;
  return false;
}

void buzzer(bool isClickable) {
  if (isClickable) {
    tone(SpeakerPin, 2400, 75);
    Serial.println("Clicked sound played.");
  } else {
    tone(SpeakerPin, 300, 75);
    delay(125);
    tone(SpeakerPin, 300, 100);
    Serial.println("Not clickable sound played.");
  }
}

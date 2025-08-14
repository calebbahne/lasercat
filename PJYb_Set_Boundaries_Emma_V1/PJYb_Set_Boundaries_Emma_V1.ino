const int joyXpin = A2; //analog pins
const int joyYpin = A3;
const int joyButton = 13;

int points[4][2];  // Array to store the 4 points: points[i][0] for X, points[i][1] for Y
int squarePoints[4][2]; // Array to store the 4 points of the square
int pointCount = 0; // Keeps track of the number of points saved
bool buttonState = false; // Current state of the button
bool lastButtonState = false; // Previous state of the button

void setup() {
  Serial.begin(9600);
  pinMode(joyButton, INPUT_PULLUP);  // Set button pin as input with pull-up resistor
}

void loop() {
  // Read joystick positions
  int xPosition = analogRead(joyXpin);
  int yPosition = analogRead(joyYpin);
  
  // Read button state (active low)
  buttonState = digitalRead(joyButton) == LOW;
  
  // If button is pressed and not already pressed, save the current joystick position as a point
  if (buttonState && !lastButtonState) {
    if (pointCount < 4) {
      points[pointCount][0] = xPosition;
      points[pointCount][1] = yPosition;
      Serial.print("Point ");
      Serial.print(pointCount + 1);
      Serial.print(": X = ");
      Serial.print(xPosition);
      Serial.print(", Y = ");
      Serial.println(yPosition);
      pointCount++;
    }
    delay(200);  // Debounce the button
  }
  
  // Once we have 4 points, calculate the square's points
  if (pointCount == 4) {
    calculateSquare();
    displaySquarePoints();
  }
  
  // Update button state
  lastButtonState = buttonState;
}

// Function to calculate the square from the 4 points
void calculateSquare() {
  // Get the first point (used as one corner of the square)
  int x1 = points[0][0];
  int y1 = points[0][1];
  
  // Calculate the width and height of the square (use the distance between the first and second point)
  int width = abs(points[1][0] - x1);
  int height = abs(points[1][1] - y1);
  
  // Calculate the 4 points of the square based on width and height
  squarePoints[0][0] = x1;                 // First point (x1, y1)
  squarePoints[0][1] = y1;

  squarePoints[1][0] = x1 + width;        // Second point (x2, y1)
  squarePoints[1][1] = y1;

  squarePoints[2][0] = x1 + width;        // Third point (x2, y2)
  squarePoints[2][1] = y1 + height;

  squarePoints[3][0] = x1;                 // Fourth point (x1, y2)
  squarePoints[3][1] = y1 + height;
}

// Function to display the square points to the Serial Monitor
void displaySquarePoints() {
  Serial.println("Square Points:");
  for (int i = 0; i < 4; i++) {
    Serial.print("Point ");
    Serial.print(i + 1);
    Serial.print(": X = ");
    Serial.print(squarePoints[i][0]);
    Serial.print(", Y = ");
    Serial.println(squarePoints[i][1]);
  }
  
  // Reset point count to allow for a new square to be created
  pointCount = 0;
  delay(2000);  // Wait before starting a new square
}

//Issues:
//1. Set high low left right, not square points. 
//  Or, could just use the minimums of these
//2. Integrate with the speed one.

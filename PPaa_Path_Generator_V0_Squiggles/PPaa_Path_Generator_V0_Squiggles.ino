// Canvas Boundaries
const int X_MIN = 0;
const int X_MAX = 100;
const int Y_MIN = 0;
const int Y_MAX = 100;

// Movement and Segmentation Parameters
const int DELTA_MAX = 2;               // Maximum step size in any direction
const int SEGMENT_AMOUNT = 5;          // Total number of segments
const int SEGMENT_LENGTH_MIN = 50;     // Minimum number of points per segment
const int SEGMENT_LENGTH_MAX = 75;     // Maximum number of points per segment

const float BIAS_SHIFT_PERCENT = 0.9;  // After 90% steps, start shifting bias towards target
const float MIN_BIAS_DISTANCE = 20.0;  // Minimum distance between bias points

// Optional Start Position
const bool USE_CUSTOM_START = false;    // Set to true to use a custom start
const int CUSTOM_START_X = 10;         // X-coordinate of custom start
const int CUSTOM_START_Y = 10;         // Y-coordinate of custom start

// Struct to hold position data
struct Point {
  int x;
  int y;
};

// Arrays to store target positions and segment lengths
Point targetPositions[SEGMENT_AMOUNT];
int segmentLengths[SEGMENT_AMOUNT];

// Current and Target Positions
Point currentPosition;
Point initialPosition;

// Segment Tracking Variables
int currentSegment = 0;
int stepsInCurrentSegment = 0;
int totalStepsInSegment = 0;

// Function Prototypes
bool generateTargetPosition(Point &target, Point existingPoints[], int existingCount);
void generateAllSegments();
void transmitBiasPoints();
void generateAndTransmitPoint();

// Setup Function
void setup() {
  Serial.begin(9600);             // Initialize Serial communication at 9600 baud
  randomSeed(analogRead(A0));     // Initialize random seed using a floating analog pin for better randomness

  // Initialize starting position
  if (USE_CUSTOM_START) {
    initialPosition.x = constrain(CUSTOM_START_X, X_MIN, X_MAX);
    initialPosition.y = constrain(CUSTOM_START_Y, Y_MIN, Y_MAX);
  } else {
    initialPosition.x = X_MIN;
    initialPosition.y = Y_MIN;
  }

  currentPosition = initialPosition;

  // Generate all segments' target positions and lengths with spreading out
  generateAllSegments();

  // Transmit bias points and segment lengths
  transmitBiasPoints();
}

// Main Loop Function
void loop() {
  if (currentSegment >= SEGMENT_AMOUNT) {
    // All segments completed
    Serial.println("END");
    delay(5000); // Wait before generating the next scribble

    // Reset for the next scribble
    currentSegment = 0;
    stepsInCurrentSegment = 0;

    currentPosition = initialPosition;

    // Optionally, regenerate segments for the next scribble
    generateAllSegments();
    transmitBiasPoints();

    return;
  }

  // Generate and broadcast the next point in the current segment
  generateAndTransmitPoint();
}

// Function to Generate a Target Position with minimum distance from existing points
bool generateTargetPosition(Point &target, Point existingPoints[], int existingCount) {
  const int MAX_ATTEMPTS = 100;
  int attempts = 0;
  
  while (attempts < MAX_ATTEMPTS) {
    target.x = random(X_MIN, X_MAX + 1);
    target.y = random(Y_MIN, Y_MAX + 1);
    
    bool tooClose = false;
    for (int i = 0; i < existingCount; i++) {
      int dx = target.x - existingPoints[i].x;
      int dy = target.y - existingPoints[i].y;
      float distance = sqrt(dx * dx + dy * dy);
      if (distance < MIN_BIAS_DISTANCE) {
        tooClose = true;
        break;
      }
    }

    if (!tooClose) {
      return true; // Successfully found a suitable target
    }

    attempts++;
  }

  return false; // Failed to find a target after max attempts
}

// Function to Generate All Segments' Target Positions and Lengths
void generateAllSegments() {
  Point existingPoints[SEGMENT_AMOUNT + 1]; // Including starting point
  existingPoints[0] = initialPosition;

  for (int i = 0; i < SEGMENT_AMOUNT; i++) {
    bool success = generateTargetPosition(targetPositions[i], existingPoints, i + 1);
    if (!success) {
      // If failed to find a target with min distance, pick randomly without constraint
      targetPositions[i].x = random(X_MIN, X_MAX + 1);
      targetPositions[i].y = random(Y_MIN, Y_MAX + 1);
    }

    existingPoints[i + 1] = targetPositions[i]; // Add to existing points for next targets

    // Determine segment length randomly between min and max
    segmentLengths[i] = random(SEGMENT_LENGTH_MIN, SEGMENT_LENGTH_MAX + 1);
  }
}

// Function to Transmit Bias Points and Segment Lengths
void transmitBiasPoints() {
  // Transmit Starting Position with label 1
  Serial.print("BIAS_START: 1: ");
  Serial.print(initialPosition.x);
  Serial.print(" ");
  Serial.println(initialPosition.y);

  // Transmit Target Positions with labels 2,3,4,...
  for (int i = 0; i < SEGMENT_AMOUNT; i++) {
    Serial.print("BIAS_TARGET_");
    Serial.print(i + 2); // Label starts from 2
    Serial.print(": ");
    Serial.print(targetPositions[i].x);
    Serial.print(" ");
    Serial.println(targetPositions[i].y);
  }

  // Transmit Segment Lengths
  for (int i = 0; i < SEGMENT_AMOUNT; i++) {
    Serial.print("SEGMENT_LENGTH_");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(segmentLengths[i]);
  }

  // Indicate the end of bias points transmission
  Serial.println("START_POINTS");
}

// Function to Generate and Transmit the Next Point
void generateAndTransmitPoint() {
  if (stepsInCurrentSegment >= segmentLengths[currentSegment]) {
    // Move to the next segment
    currentSegment++;
    stepsInCurrentSegment = 0;
    return;
  }

  // Determine current step threshold for bias shift
  int shiftStep = ceil(BIAS_SHIFT_PERCENT * segmentLengths[currentSegment]); // Last 10% steps

  float biasTowardsPrevious;
  float biasTowardsTarget;

  if (stepsInCurrentSegment < shiftStep) {
    // Before bias shift
    biasTowardsPrevious = 1.0;
    biasTowardsTarget = 0.0;
  } else {
    // During bias shift: linearly shift from previous to target
    int stepsInto_shift = stepsInCurrentSegment - shiftStep;
    int total_shift_steps = segmentLengths[currentSegment] - shiftStep;

    if (total_shift_steps > 0) {
      float shift_ratio = float(stepsInto_shift) / float(total_shift_steps);
      biasTowardsPrevious = 1.0 - shift_ratio;
      biasTowardsTarget = shift_ratio;
    } else {
      // Edge case: no shift steps
      biasTowardsPrevious = 0.0;
      biasTowardsTarget = 1.0;
    }
  }

  // Calculate direction towards target
  int deltaX = targetPositions[currentSegment].x - currentPosition.x;
  int deltaY = targetPositions[currentSegment].y - currentPosition.y;

  // Normalize direction
  float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
  float normX = (distance != 0) ? (deltaX / distance) : 0;
  float normY = (distance != 0) ? (deltaY / distance) : 0;

  // Apply bias to direction
  float stepDirX = normX * biasTowardsTarget;
  float stepDirY = normY * biasTowardsTarget;

  // Generate random step with DELTA_MAX
  int stepX = random(-DELTA_MAX, DELTA_MAX + 1);
  int stepY = random(-DELTA_MAX, DELTA_MAX + 1);

  // Combine random step with biased direction
  float combinedX = (float(stepX) * biasTowardsPrevious) + stepDirX;
  float combinedY = (float(stepY) * biasTowardsPrevious) + stepDirY;

  // Update current position
  int newX = constrain(currentPosition.x + round(combinedX), X_MIN, X_MAX);
  int newY = constrain(currentPosition.y + round(combinedY), Y_MIN, Y_MAX);
  currentPosition.x = newX;
  currentPosition.y = newY;

  // Transmit the new point via Serial in (x,y) format
  Serial.print("(");
  Serial.print(currentPosition.x);
  Serial.print(",");
  Serial.print(currentPosition.y);
  Serial.println(")");

  stepsInCurrentSegment++;
  delay(50); // Delay for readability (adjust as needed)
}

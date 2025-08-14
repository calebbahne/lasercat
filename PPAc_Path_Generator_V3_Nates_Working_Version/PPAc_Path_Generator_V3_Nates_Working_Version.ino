// Constants defining the boundaries for random point generation
int xPosRange[2] = {-180, 180};
int yPosRange[2] = {-180, 180};

// Constants for path generation
const int SEGMENT_AMOUNT = 100; // Number of segments in the path
const int NUM_CURVE_POINTS = 20; // Number of points for curved segments
const int NUM_STRAIGHT_POINTS = 20; // Number of points for the final straight segment

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
    while (!Serial) { ; } // Wait for Serial to be ready (only needed on some boards)

    delay(10); // Optional delay for variability
    randomSeed(millis() + analogRead(0)); // Seed random number generator with millis and analog reading

    generatePath(); // Generate the path; can pass a specific start point if needed
}

void loop() {
    // Empty loop; no continuous tasks required
}

void generatePath(Point* start = nullptr) {
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
        Serial.print("(");
        Serial.print(x, 2); // Print X coordinate with 2 decimal places
        Serial.print(",");
        Serial.print(y, 2); // Print Y coordinate with 2 decimal places
        Serial.println(")");
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
        Serial.print("(");
        Serial.print(x, 2); // Print X coordinate with 2 decimal places
        Serial.print(",");
        Serial.print(y, 2); // Print Y coordinate with 2 decimal places
        Serial.println(")");
        delayForReadability(); // Delay for better readability of output
    }
}

void delayForReadability() {
    delay(20); // Delay for 20 milliseconds
}

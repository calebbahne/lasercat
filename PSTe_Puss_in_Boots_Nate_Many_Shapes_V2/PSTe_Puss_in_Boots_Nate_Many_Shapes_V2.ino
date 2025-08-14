#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <EEPROM.h>
#include <math.h>

#define SpeakerPin    12
#define laserPin      11
#define SleepPin      4
#define microstepPin  7
#define joyButton     13

#define xIR_anaPin    A0
#define yIR_anaPin    A1

LiquidCrystal_I2C lcd(0x27, 16, 2);

AccelStepper xStepper(AccelStepper::DRIVER, 3, 2);
AccelStepper yStepper(AccelStepper::DRIVER, 6, 5);
MultiStepper bothSteppers;

int xPosRange[2] = {-140, 80};
int yPosRange[2] = {-90, 60};
long xyPos[2] = {0, 0};

int xCenterPos = 0;
int yCenterPos = 0;

struct Point {
    float x;
    float y;
};

bool homeSteppers(int homeSpeed = 100);
void generatePath(int duration, int baseSpeed, Point* start = nullptr);
Point generateRandomPoint();
void moveSteppers(long x, long y);
bool isButtonReleased();
bool isQuietModeActivated();
void buzzer(bool isClickable);

void generatePolygonPoints(int numSides, int radius, Point* points);
void drawRegularPolygon(int numSides, int baseSpeed, unsigned long endTime);
void demonstrateShapes(int startSides, int duration);

bool clickable = false;
bool joystickBusy = false;
unsigned long lastDebounce = 0;
#define DEBOUNCE_DELAY 200UL

#define QUIET_MODE_ADDRESS 0

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();

    pinMode(SpeakerPin, OUTPUT);
    digitalWrite(SpeakerPin, LOW);

    pinMode(laserPin, OUTPUT);
    digitalWrite(laserPin, LOW);

    pinMode(SleepPin, OUTPUT);
    digitalWrite(SleepPin, LOW);

    pinMode(microstepPin, OUTPUT);
    digitalWrite(microstepPin, HIGH);

    pinMode(joyButton, INPUT_PULLUP);

    xStepper.setMaxSpeed(1200);
    yStepper.setMaxSpeed(1200);
    bothSteppers.addStepper(xStepper);
    bothSteppers.addStepper(yStepper);

    homeSteppers(800);
    homeSteppers();

    if (!isQuietModeActivated()) {}

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pete the Cat");
    lcd.setCursor(0, 1);
    lcd.print("Drawing Path...");
    digitalWrite(laserPin, HIGH);
    demonstrateShapes(3, 60000);  // Start with triangle, run for 1 minute
    digitalWrite(laserPin, LOW);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Path Complete");
}

void loop() {
    if (isButtonReleased()) {
        xStepper.stop();
        yStepper.stop();
        digitalWrite(laserPin, LOW);
        digitalWrite(SleepPin, LOW);  // Disable steppers when interrupted
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Interrupted");
        delay(2000);
    }
}

bool homeSteppers(int homeSpeed) {
    lcd.clear();
    lcd.setCursor((16 - 11) / 2, 0);
    lcd.print(F("The LaserCat"));
    lcd.setCursor(0, 1);
    lcd.print(F("Homing Steppers"));

    digitalWrite(SleepPin, HIGH);

    int xIR_anaValue = 1000;
    int yIR_anaValue = 1000;
    int homingTol = 80;
    long nextPos = 0;
    int increment = -10;
    bool xHomed = false;
    bool yHomed = false;
    delay(200);

    xStepper.setSpeed(600);
    yStepper.setSpeed(600);
    moveSteppers(50, 50);

    while (!xHomed || !yHomed) {
        xIR_anaValue = analogRead(xIR_anaPin);
        yIR_anaValue = analogRead(yIR_anaPin);

        if (!xHomed) {
            nextPos = xStepper.currentPosition() + increment;
            while (xStepper.currentPosition() > nextPos) {
                xStepper.setSpeed(-homeSpeed);
                xStepper.runSpeed();
            }
            xStepper.stop();

            xIR_anaValue = analogRead(xIR_anaPin);

            if (xIR_anaValue <= homingTol) {
                delay(20);
                xIR_anaValue = analogRead(xIR_anaPin);
                Serial.print(F("X3: "));
                Serial.println(xIR_anaValue);

                if (xIR_anaValue <= 300) {
                    xStepper.setCurrentPosition(0);
                    xHomed = true;
                    Serial.println(F("X Axis Homed"));
                }
            }
        }

        if (!yHomed) {
            nextPos = yStepper.currentPosition() + increment;
            while (yStepper.currentPosition() > nextPos) {
                yStepper.setSpeed(-homeSpeed);
                yStepper.runSpeed();
            }
            yStepper.stop();

            yIR_anaValue = analogRead(yIR_anaPin);

            if (yIR_anaValue <= homingTol) {
                delay(20);
                yIR_anaValue = analogRead(yIR_anaPin);
                Serial.print(F("Y3: "));
                Serial.println(yIR_anaValue);

                if (yIR_anaValue <= 300) {
                    yStepper.setCurrentPosition(0);
                    yHomed = true;
                    Serial.println(F("Y Axis Homed"));
                }
            }
        }
    }

    lcd.clear();
    lcd.print(F("Steppers Homed"));
    delay(500);

    xyPos[0] = 360;
    xyPos[1] = 280;
    bothSteppers.moveTo(xyPos);
    while (bothSteppers.run()) {}

    xStepper.setCurrentPosition(0);
    yStepper.setCurrentPosition(0);

    delay(500);

    xyPos[0] = xCenterPos;
    xyPos[1] = yCenterPos;
    bothSteppers.moveTo(xyPos);
    while (bothSteppers.run()) {}

    digitalWrite(SleepPin, LOW);
    return true;
}

Point generateRandomPoint() {
    Point p;
    p.x = static_cast<float>(random(xPosRange[0], xPosRange[1] + 1));
    p.y = static_cast<float>(random(yPosRange[0], yPosRange[1] + 1));
    return p;
}

void moveSteppers(long x, long y) {
    xyPos[0] = x;
    xyPos[1] = y;
    bothSteppers.moveTo(xyPos);
    while (bothSteppers.run()) {}
}

void generatePath(int duration, int baseSpeed, Point* start) {
    digitalWrite(SleepPin, HIGH);  // Enable steppers for movement
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Drawing for");
    lcd.setCursor(0, 1);
    lcd.print("Pete the Cat!");

    // Set acceleration for smooth speed changes
    xStepper.setAcceleration(500);
    yStepper.setAcceleration(500);
    xStepper.setMaxSpeed(baseSpeed);
    yStepper.setMaxSpeed(baseSpeed);

    Point currentPos = start ? *start : generateRandomPoint();
    Point targetPos = generateRandomPoint();
    unsigned long startTime = millis();
    unsigned long lastUpdate = startTime;
    const unsigned long updateInterval = 50; // Update speeds every 50ms
    float progress = 0;

    while (millis() - startTime < duration) {
        unsigned long currentTime = millis();
        
        // Generate new target if we're close to current target
        float distToTarget = sqrt(pow(targetPos.x - currentPos.x, 2) + pow(targetPos.y - currentPos.y, 2));
        if (distToTarget < 10) {
            targetPos = generateRandomPoint();
        }

        // Update speeds periodically
        if (currentTime - lastUpdate >= updateInterval) {
            // Calculate direction vector
            float dx = targetPos.x - currentPos.x;
            float dy = targetPos.y - currentPos.y;
            float distance = sqrt(dx*dx + dy*dy);
            
            // Normalize direction and apply speed
            if (distance > 0) {
                float xSpeed = (dx / distance) * baseSpeed;
                float ySpeed = (dy / distance) * baseSpeed;
                
                // Add sinusoidal variation for smooth, organic movement
                progress += 0.05;
                float speedMod = sin(progress) * 0.3 + 0.7; // Varies speed between 40% and 100%
                
                xStepper.setSpeed(xSpeed * speedMod);
                yStepper.setSpeed(ySpeed * speedMod);
            }

            // Update current position
            currentPos.x = xStepper.currentPosition();
            currentPos.y = yStepper.currentPosition();
            
            lastUpdate = currentTime;
        }

        // Run the steppers
        xStepper.runSpeed();
        yStepper.runSpeed();

        if (isButtonReleased()) {
            xStepper.stop();
            yStepper.stop();
            digitalWrite(laserPin, LOW);
            lcd.clear();
            lcd.print("Path Interrupted");
            delay(2000);
            return;
        }
    }

    // Smoothly stop the motors
    xStepper.stop();
    yStepper.stop();
    digitalWrite(SleepPin, LOW);  // Disable steppers after path completion
}

void generatePolygonPoints(int numSides, int radius, Point* points) {
    for (int i = 0; i < numSides; i++) {
        float angle = i * 2 * M_PI / numSides;
        points[i].x = cos(angle) * radius;
        points[i].y = sin(angle) * radius;
    }
}

void drawRegularPolygon(int numSides, int baseSpeed, unsigned long endTime) {
    digitalWrite(SleepPin, HIGH);  // Enable steppers for movement
    
    // Set acceleration for smooth changes
    xStepper.setAcceleration(500);
    yStepper.setAcceleration(500);
    xStepper.setMaxSpeed(baseSpeed);
    yStepper.setMaxSpeed(baseSpeed);

    int radius = 20;
    Point points[6];  // Static array with maximum size
    generatePolygonPoints(numSides, radius, points);
    
    int currentPoint = 0;
    unsigned long lastUpdate = millis();
    const unsigned long updateInterval = 1;
    
    moveSteppers(points[0].x, points[0].y);

    while (millis() < endTime) {
        unsigned long currentTime = millis();
        
        if (currentTime - lastUpdate >= updateInterval) {
            int nextPoint = (currentPoint + 1) % numSides;
            
            Point currentPos = {
                (float)xStepper.currentPosition(),
                (float)yStepper.currentPosition()
            };
            
            float dx = points[nextPoint].x - currentPos.x;
            float dy = points[nextPoint].y - currentPos.y;
            float distance = sqrt(dx*dx + dy*dy);
            
            if (distance < 0.5) {
                currentPoint = nextPoint;
            }
            
            if (distance > 0) {
                float xSpeed = (dx / distance) * baseSpeed;
                float ySpeed = (dy / distance) * baseSpeed;
                
                xStepper.setSpeed(xSpeed);
                yStepper.setSpeed(ySpeed);
            }
            
            lastUpdate = currentTime;
        }

        xStepper.runSpeed();
        yStepper.runSpeed();

        if (isButtonReleased()) {
            xStepper.stop();
            yStepper.stop();
            digitalWrite(laserPin, LOW);
            lcd.clear();
            lcd.print("Path Interrupted");
            delay(2000);
            return;
        }
    }
}

void demonstrateShapes(int startSides, int duration) {
    int currentSides = startSides;
    unsigned long startTime = millis();
    unsigned long lastShapeChange = startTime;
    
    while (millis() - startTime < duration) {
        // Update LCD with current shape info
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sides: ");
        lcd.print(currentSides);
        
        // Calculate when this shape should end
        unsigned long shapeEndTime = lastShapeChange + 2000;
        
        // Draw the current shape
        digitalWrite(laserPin, HIGH);
        drawRegularPolygon(currentSides, 1200, shapeEndTime);
        
        // Move to next shape
        currentSides++;
        if (currentSides > 6) {
            currentSides = 3;  // Reset back to triangle
        }
        
        lastShapeChange = millis();
        moveSteppers(0, 0);
    }
    
    digitalWrite(laserPin, LOW);
    digitalWrite(SleepPin, LOW);
}

bool isButtonReleased() {
    static bool lastState = HIGH;
    bool buttonState = digitalRead(joyButton);

    if (buttonState == HIGH && lastState == LOW) {
        lastState = HIGH;
        buzzer(clickable);
        return true;
    }

    lastState = buttonState;
    return false;
}

bool isQuietModeActivated() {
    int eepromValue = EEPROM.read(QUIET_MODE_ADDRESS);
    return (eepromValue == 1);
}

void buzzer(bool isClickable) {
    if (!isQuietModeActivated()) {
        if (isClickable) {
            tone(SpeakerPin, 2400, 75);
            Serial.println(F("Clicked sound played."));
        } else {
            tone(SpeakerPin, 300, 75);
            delay(125);
            tone(SpeakerPin, 300, 100);
            Serial.println(F("Not clickable sound played."));
        }
    }
}
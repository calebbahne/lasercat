#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <math.h>

#define SpeakerPin    12
#define laserPin      11
#define SleepPin      4
#define microstepPin  7
#define joyButton     13

LiquidCrystal_I2C lcd(0x27, 16, 2);

AccelStepper xStepper(AccelStepper::DRIVER, 3, 2);
AccelStepper yStepper(AccelStepper::DRIVER, 6, 5);
MultiStepper bothSteppers;

struct Point {
    float x;
    float y;
};

void generatePolygonPoints(int numSides, float radius, Point* points);
void drawRegularPolygon(int numSides, int baseSpeed, unsigned long endTime);
void demonstrateShapes(int startSides, int duration);
bool isButtonReleased();
void moveSteppers(long x, long y);

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

    // Start the shape demonstration
    demonstrateShapes(3, 60000);  // Start with triangle, run for 1 minute
}

void loop() {
    // Empty - all work done in demonstrateShapes
}

void generatePolygonPoints(int numSides, float radius, Point* points) {
    for (int i = 0; i < numSides; i++) {
        float angle = i * 2 * M_PI / numSides;
        points[i].x = cos(angle) * radius;
        points[i].y = sin(angle) * radius;
    }
}

void moveSteppers(long x, long y) {
    long positions[2] = {x, y};
    bothSteppers.moveTo(positions);
    while (bothSteppers.run()) {}
}

void drawRegularPolygon(int numSides, int baseSpeed, unsigned long endTime) {
    digitalWrite(SleepPin, HIGH);
    
    xStepper.setAcceleration(1000);
    yStepper.setAcceleration(1000);
    xStepper.setMaxSpeed(baseSpeed);
    yStepper.setMaxSpeed(baseSpeed);

    const float radius = 20.0;
    Point points[6];
    generatePolygonPoints(numSides, radius, points);
    
    int currentPoint = 0;
    unsigned long lastUpdate = millis();
    const unsigned long updateInterval = 5;
    
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
            
            if (distance > 0.5) {
                float xSpeed = (dx / distance) * baseSpeed;
                float ySpeed = (dy / distance) * baseSpeed;
                
                // Add a minimum speed threshold to prevent tiny adjustments
                if (abs(xSpeed) < 5) xSpeed = 0;
                if (abs(ySpeed) < 5) ySpeed = 0;
                
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
    const int MAX_SIDES = 6;
    unsigned long startTime = millis();
    unsigned long lastShapeChange = startTime;
    const unsigned long shapeChangeDuration = 5000;
    
    while (millis() - startTime < duration) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sides: ");
        lcd.print(currentSides);
        
        unsigned long shapeEndTime = lastShapeChange + shapeChangeDuration;
        
        digitalWrite(laserPin, HIGH);
        drawRegularPolygon(currentSides, 600, shapeEndTime);
        
        currentSides++;
        if (currentSides > MAX_SIDES) {
            currentSides = 3;
        }
        
        lastShapeChange = millis();
        moveSteppers(0, 0);
    }
    
    digitalWrite(laserPin, LOW);
    digitalWrite(SleepPin, LOW);
}

bool isButtonReleased() {
    return digitalRead(joyButton) == LOW;
}
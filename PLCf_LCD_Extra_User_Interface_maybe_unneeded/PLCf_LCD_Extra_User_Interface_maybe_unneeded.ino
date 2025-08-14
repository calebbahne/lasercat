#include <LiquidCrystal.h>

// Initialize the LCD with appropriate pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Function prototypes
void runLCD();
void defineBoundaries();
void runRandomPath();
void drawCustomPath();
void schedulePath();
void exitScreen();
void checkJoyLRUD();

// Global variables
int counter = 1; // Counter for mode selection
const int buttonPin = 6; // Pin for button
const int joystickXPin = A0; // Pin for joystick X
const int joystickYPin = A1; // Pin for joystick Y

void setup() {
    // Initialize the LCD
    lcd.begin(16, 2);
    pinMode(buttonPin, INPUT);
    
    // Power on display
    runLCD();
}

void loop() {
    // Check for button press
    if (digitalRead(buttonPin) == HIGH) {
        runLCD();
    }
}

void runLCD() {
    lcd.clear();
    lcd.print("CatBot3000");
    delay(3000); // Display name for 3 seconds
    
    // Display mode selection
    lcd.clear();
    lcd.print("Select a Mode");
    lcd.setCursor(0, 1);
    lcd.print("<  Define Boundaries  >");
    
    while (true) {
        checkJoyLRUD(); // Check joystick input
        
        // Check if a button is pressed to select mode
        if (digitalRead(buttonPin) == HIGH) {
            switch (counter) {
                case 1:
                    defineBoundaries();
                    break;
                case 2:
                    runRandomPath();
                    break;
                case 3:
                    drawCustomPath();
                    break;
                case 4:
                    schedulePath();
                    break;
                case 5:
                    exitScreen();
                    return; // Exit the mode selection
            }
        }
    }
}

void checkJoyLRUD() {
    int joystickX = analogRead(joystickXPin);
    int joystickY = analogRead(joystickYPin);
    
    // Joystick left/right
    if (joystickX < 200) { // Threshold for left
        counter = max(1, counter - 1);
        updateModeDisplay();
    } else if (joystickX > 800) { // Threshold for right
        counter = min(5, counter + 1);
        updateModeDisplay();
    }
    
    // Joystick down to exit
    if (joystickY > 800) { // Threshold for down
        exitScreen();
        return;
    }
}

void updateModeDisplay() {
    lcd.clear();
    lcd.print("Select a Mode");
    lcd.setCursor(0, 1);
    
    switch (counter) {
        case 1:
            lcd.print("<  Define Boundaries  >");
            break;
        case 2:
            lcd.print("<  Run Random Path   >");
            break;
        case 3:
            lcd.print("<  Draw Custom Path  >");
            break;
        case 4:
            lcd.print("<  Schedule a Path   >");
            break;
        case 5:
            lcd.print("<  Exit             >");
            break;
    }
}

void defineBoundaries() {
    // Implementation for defining boundaries
    lcd.clear();
    lcd.print("Define Boundaries");
    // Prompt user for coordinates, etc.
}

void runRandomPath() {
    // Implementation for running random path
    lcd.clear();
    lcd.print("Running Random Path");
}

void drawCustomPath() {
    // Implementation for drawing custom path
    lcd.clear();
    lcd.print("Drawing Custom Path");
}

void schedulePath() {
    // Implementation for scheduling a path
    lcd.clear();
    lcd.print("Scheduling Path");
}

void exitScreen() {
    lcd.clear();
    lcd.print("Exiting...");
    delay(1000); // Pause before returning to main loop
    lcd.clear();
}

//Issues with quiet mode
//Won't exit random path once done--trouble returning to main menu
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <EEPROM.h>
#include <pitches.h>

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Buzzer Definition
#define SpeakerPin 12
bool clickable = false; // Change this variable to test the sounds
#define laserPin 11

// Joystick Definitions
#define joyButton 13  // Joystick button
#define joyXpin A3    // Joystick X axis
#define joyYpin A2    // Joystick Y axis

// Stepper motor pins
#define xDirPin 2
#define xStepPin 3
#define SleepPin 4

#define yDirPin 5
#define yStepPin 6
#define microstepPin 7

// IR Sensors
#define xIR_anaPin A1
#define yIR_anaPin A0

// LCD Variables - Menu Options stored in PROGMEM
const char menuOptions[][20] PROGMEM = {
  "< Random  Path >",
  "<     Quiet    >",
  "<    Sleep     >",
  "<Set Boundaries>",
  "<   Schedule   >",
  "< Custom  Path >"
};

const int numOptions = sizeof(menuOptions) / sizeof(menuOptions[0]);
int currentOption = 0;

// Cat Selection Variables stored in PROGMEM
const char catNames[][20] PROGMEM = {
  "V Garfield",
  "V Pete the Cat",
  "V Puss in Boots",
  "V Main Menu"
};

const int numCats = sizeof(catNames) / sizeof(catNames[0]);
int currentCat = 0;

// Position
int xPosRange[2] = { -400, 400 };
int yPosRange[2] = { -400, 400 };
long xyPos[2] = { 0, 0 };  // Stores the positions [x,y] for the two motors
int xCenterPos = 0;
int yCenterPos = 0;

// Variables to store the saved stepper positions (4 points)
long savedPositions[4][2]; // Array to store 4 positions: [0] = X, [1] = Y
int pressCount = 0; // Count of button presses

float absMaxSpeed = 1200; // Remove if maxed out of variables
float tempMaxSpeed = 0.5 * absMaxSpeed;

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

// States
#define DISPLAY_TITLE 0
#define SELECT_MODE 1
#define EXECUTE_MODE 2
#define LOW_POWER 3
int currentState = DISPLAY_TITLE;

// Current Mode
int selectedMode = -1;

// Timing Variables
unsigned long lastActivity = 0;
unsigned long titleStartTime = 0;
unsigned long progressStartTime = 0;

// Constants
#define INACTIVITY_TIMEOUT 30000UL  // 30 sec
#define PROGRESS_INTERVAL 250UL     // 250ms

// Flags
bool lcdOn = true;
int progressBarLength = 0;

// Debounce Variables
bool joystickBusy = false;
unsigned long lastDebounce = 0;
#define DEBOUNCE_DELAY 200UL

// Flag to ensure joystick returns to neutral before next input
bool joystickNeutral = true;

struct Point {
    float x;
    float y;
};

//======Path Stuff
// Variables to hold the current target and center points
Point currentTarget; // Current target point for the segment
Point currentCenter; // Current center point for the segment
Point startPosition; // Starting position for the path

// Function Prototypes
void generatePath(int segAmt, int numCurvPts, int numStrtPts, int speed, int stopTime, Point* start = nullptr); // Generate the path based on a starting point
void transmitData(const Point& start, const Point& target, const Point& center, const Point& nextTarget); // Transmit generated data via Serial
Point generateRandomPoint(); // Generate a random point within defined bounds
void generateStraightLine(const Point& start, const Point& end, int numPoints); // Generate points for a straight line
void generateCurve(const Point& start, const Point& end, const Point& control, int numPoints); // Generate points for a curve

// Function Prototypes
void displayTitle();
void displayMenu();
void updateProgressBar();
String getJoystickDirection();
bool isButtonReleased();
void executeSelectedMode();
void returnToMenu();
void enterLowPowerMode();
void exitLowPowerMode();
void buzzer(bool isClickable);  // Prototype for the buzzer function
void displayCatSelection();
void cycleCats();
void executeCatAction(int selectedCat);
void pinkPanther();

// Mode Functions
bool randomPathMode();
bool sleepMode();
bool quietMode();
bool scheduleMode();
bool customPathMode();
bool setBoundaries();

// Helper functions placed here...

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  displayTitle();
  titleStartTime = millis();
  lastActivity = millis();
  progressStartTime = millis();

  //====== Stepper and Joystick setup
  xStepper.setMaxSpeed(2000);
  yStepper.setMaxSpeed(2000);

  pinMode(SleepPin, OUTPUT);
  pinMode(microstepPin, OUTPUT);
  digitalWrite(SleepPin, LOW);  // Active low: sleeps when set to low
  digitalWrite(microstepPin, HIGH); // High for 1/16, low for 1/2 microstepping

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  pinMode(joyButton, INPUT_PULLUP);

  pinMode(xIR_anaPin, INPUT);
  pinMode(yIR_anaPin, INPUT);

  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, LOW);

  randomSeed(millis());
}

void loop() {  // Main state machine
  unsigned long currentMillis = millis();

  switch (currentState) {
    case DISPLAY_TITLE:
      clickable = false;
      // Update progress bar
      if (currentMillis - progressStartTime >= PROGRESS_INTERVAL && progressBarLength < 16) {
        isButtonReleased();
        updateProgressBar();
        progressBarLength = min(progressBarLength + 1, 16);  // Ensure it doesn't exceed 16
        progressStartTime = currentMillis;
      }
      // Check if title duration passed
      if (currentMillis - titleStartTime >= 4000) {
        currentState = SELECT_MODE;
        displayMenu();
        lastActivity = currentMillis;
      }
      break;

    case SELECT_MODE:
      // Check inactivity
      if (currentMillis - lastActivity >= INACTIVITY_TIMEOUT) {
        clickable = true;
        enterLowPowerMode();
      } else {
        // Handle joystick input
        String direction = getJoystickDirection();
        if (direction != "Neutral" && joystickNeutral) {  // Only process if neutral
          lastActivity = currentMillis;
          if (direction == "Left") {
            currentOption = (currentOption + 1) % numOptions;
            clickable = true;
            if (!isQuietModeActivated()) {
              tone(SpeakerPin, 1200, 100);
            }
            displayMenu();
            Serial.println(F("Joystick Left"));
            joystickNeutral = false;
          } else if (direction == "Right") {
            currentOption = (currentOption - 1 + numOptions) % numOptions;
            clickable = true;
            if (!isQuietModeActivated()) {
              tone(SpeakerPin, 1200, 100);
            }
            displayMenu();
            Serial.println(F("Joystick Right"));
            joystickNeutral = false;
          }
        }

        // Check button release
        if (isButtonReleased()) {
          selectedMode = currentOption;
          currentState = EXECUTE_MODE;
          lastActivity = currentMillis;
          Serial.println(F("Button Released"));
          executeSelectedMode();
        }

        // Check if joystick has returned to neutral
        int x = analogRead(joyXpin);
        int y = analogRead(joyYpin);
        if (x >= 250 && x <= 750 && y >= 250 && y <= 750) {
          joystickNeutral = true;
        }
      }
      break;

    case EXECUTE_MODE:
      // Mode functions handle their own logic
      // Might not need this
      break;

    case LOW_POWER:
      // Wait for any input to wake up
      if (getJoystickDirection() != "Neutral" || isButtonReleased()) {
        exitLowPowerMode();
        currentState = SELECT_MODE;
        displayMenu();
        lastActivity = millis();
        Serial.println(F("Woke up from Low Power Mode"));
      }
      break;
  }
}

// Function Definitions

void displayTitle() {
  lcd.clear();
  lcd.setCursor((16 - 11) / 2, 0);  // "The LaserCat" is 11 characters
  lcd.print(F("The LaserCat"));

  // Initialize empty progress bar
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(F(" "));
  }

  progressBarLength = 0;
  Serial.println(F("Displaying Title"));
}

void updateProgressBar() {
  if (progressBarLength < 16) {
    lcd.setCursor(progressBarLength, 1);
    lcd.write(byte(255));  // Full block
  }
}

void displayMenu() {
  clickable = true;
  lcd.clear();
  lcd.setCursor((16 - strlen_P(PSTR("Select Mode"))) / 2, 0);
  lcd.print(F("Select Mode"));

  // Display current option
  lcd.setCursor(0, 1);
  char buffer[20];
  strcpy_P(buffer, menuOptions[currentOption]);
  lcd.print(buffer);  // This is a cool way of doing it

  Serial.print(F("Displaying Menu: "));
  Serial.println(buffer);
}

String getJoystickDirection() {
  String dir = "Neutral";
  int x = analogRead(joyXpin);
  int y = analogRead(joyYpin);

  // Thresholds (adjust as needed)
  if (!joystickBusy) {
    if (x < 250) {
      dir = "Left";
      joystickBusy = true;
      lastDebounce = millis();
    } else if (x > 750) {
      dir = "Right";
      joystickBusy = true;
      lastDebounce = millis();
    } else if (y < 250) {
      dir = "Up";
      joystickBusy = true;
      lastDebounce = millis();
    } else if (y > 750) {
      dir = "Down";
      joystickBusy = true;
      lastDebounce = millis();
    }
  } else {
    if (millis() - lastDebounce >= DEBOUNCE_DELAY) {
      joystickBusy = false;
    }
  }

  return dir;
}

// Detect button release
bool isButtonReleased() {
  static bool lastState = HIGH; // set to high the first time but retains its value everytime after that
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

void executeSelectedMode() {
  Serial.print(F("Executing Mode: "));
  char buffer[20];
  strcpy_P(buffer, menuOptions[selectedMode]);
  Serial.println(buffer);
  lcd.clear();

  switch (selectedMode) {
    case 0:
      randomPathMode();
      break;
    case 1:
      quietMode();
      break;
    case 2:
      sleepMode();
      break;
    case 3:
      setBoundaries();
      break;
    case 4:
      scheduleMode();
      break;
    case 5:
      customPathMode();
      break;
    default:
      Serial.println(F("Invalid Mode"));
      returnToMenu();
      break;
  }
}

void returnToMenu() {
  currentState = SELECT_MODE;
  joystickNeutral = true; // Reset joystick state
  displayMenu();
  lastActivity = millis();
  Serial.println(F("Returned to Menu"));
}

void enterLowPowerMode() {
  lcd.clear();
  lcd.noBacklight();
  lcdOn = false;
  currentState = LOW_POWER;
  Serial.println(F("Entered Low Power Mode"));
}

void exitLowPowerMode() {
  lcd.backlight();
  lcdOn = true;
  Serial.println(F("Exited Low Power Mode"));
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

void pinkPanther(){
  int melody[] = {
    REST, REST, REST, NOTE_DS4, 
    NOTE_E4, REST, NOTE_FS4, NOTE_G4, REST, NOTE_DS4,
    NOTE_E4, NOTE_FS4,  NOTE_G4, NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_B4,   
    NOTE_AS4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_D4, 
    NOTE_E4, REST, REST
  };

  int durations[] = {
    2, 4, 8, 8, 
    4, 8, 8, 4, 8, 8,
    8, 8,  8, 8, 8, 8, 8, 8,   
    2, 16, 16, 16, 16, 
    2, 4, 8,
  };

  int size = sizeof(durations) / sizeof(int);

  for (int note = 0; note < size; note++) {
    int duration = 1100 / durations[note];
    tone(SpeakerPin, melody[note], duration);
    delay(duration * 1.30);
    
    //stop the tone playing:
    noTone(SpeakerPin);
  }
}

// =========== MODE FUNCTION DEFINITIONS ============

bool randomPathMode() {
  Serial.print(F("Entering"));
  // Initial cat selection display
  displayCatSelection();  // Show the first cat option

  // Allow the user to cycle through cats
  while (true) {
    cycleCats();  // Handle joystick input to select cat

    // Add your logic here to start generating a random path
    // For example, you might want to wait for another button press to proceed
    if (isButtonReleased()) {
      executeCatAction(currentCat);  // Execute the action based on the selected cat
      break;                           // Exit the loop once an action has been taken
    }
  }
  Serial.print(F("Exiting"));
  returnToMenu();
  return true;
}

bool scheduleMode() {
  lcd.clear();
  lcd.setCursor((16 - strlen_P(PSTR("Schedule"))) / 2, 0);
  lcd.print(F("Schedule"));

  // Clear the bottom line
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(F(" "));
  }

  delay(4000);
  returnToMenu();
  return true;
}

bool customPathMode() {
  lcd.clear();
  lcd.setCursor((16 - strlen_P(PSTR("Custom Path"))) / 2, 0);
  lcd.print(F("Custom Path"));

  // Clear the bottom line
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(F(" "));
  }

  homeSteppers();
  digitalWrite(laserPin, HIGH);
  if (!isQuietModeActivated()) {
    pinkPanther();
  }
  executeCustomPath();
  digitalWrite(laserPin, LOW);

  returnToMenu();
  return true;
}

bool setBoundaries() {
  lcd.clear();
  lcd.setCursor((16 - strlen_P(PSTR("Set Boundaries"))) / 2, 0);
  lcd.print(F("Set Boundaries"));
  bool boundariesSet = false;

  homeSteppers();
  digitalWrite(laserPin, HIGH);

  digitalWrite(SleepPin, HIGH);
  tempMaxSpeed = 125;

  // Clear the bottom line
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(F(" "));
  }

  while (!boundariesSet) {
    // Read joystick positions
    int xJoyVal = analogRead(joyXpin); // Read X-axis
    int yJoyVal = analogRead(joyYpin); // Read Y-axis

    // Map joystick values to motor positions
    int joystickSpeedX = map(xJoyVal, 0, 1023, -tempMaxSpeed, tempMaxSpeed); 
    int joystickSpeedY = map(yJoyVal, 0, 1023, -tempMaxSpeed, tempMaxSpeed);

    if (abs(xJoyVal - 512) < 65) {
      joystickSpeedX = 0;
    }
    if (abs(yJoyVal - 512) < 65) {
      joystickSpeedY = 0;
    }

    // Set the speed of the stepper motors
    xStepper.setSpeed(joystickSpeedX);  // Set speed for xStepper
    yStepper.setSpeed(joystickSpeedY);  // Set speed for yStepper

    // Move the motors based on the joystick input
    xStepper.runSpeed();  // Move xStepper at the set speed
    yStepper.runSpeed();  // Move yStepper at the set speed

    // If the joystick button is pressed, save the current position of the steppers
    if (isButtonReleased()) {
      boundariesSet = setFourCorners();
    }
  }
  xStepper.setMaxSpeed(1200);  // Set speed for xStepper
  yStepper.setMaxSpeed(1200);
  digitalWrite(SleepPin, LOW);
  digitalWrite(laserPin, LOW);
  delay(1000);
  returnToMenu();
  return true;
}

bool quietMode() {
  // Read the current state from EEPROM
  int eepromValue = EEPROM.read(0);    // Read from the first byte
  bool isActive = (eepromValue == 1); // True if activated

  // Toggle the state
  isActive = !isActive;  // Switch between activated and deactivated

  // Display Quiet Mode
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Quiet Mode:"));

  // Display Activated or Deactivated
  lcd.setCursor(0, 1);  // Align to the left
  if (isActive) {
    lcd.print(F("Activated   "));
  } else {
    lcd.print(F("Deactivated "));
  }

  // Save the new state to EEPROM
  Serial.print(F("Current Quiet Mode state: "));
  Serial.println(isActive ? F("Activated") : F("Deactivated"));

  // Write the new state to EEPROM (1 for activated, 0 for deactivated)
  EEPROM.write(0, isActive ? 1 : 0);

  delay(2000);  // Wait for 2 seconds
  returnToMenu();
  return true;
}

//==========Functions called by the mode functions================

//----------LCD/Logic Functions-----------------

bool sleepMode() {
  // Implement sleep mode logic
  enterLowPowerMode();
  return false;
}

bool isQuietModeActivated() {
  // Read the current state from EEPROM
  int eepromValue = EEPROM.read(0);  // Read from the first byte
  return (eepromValue == 1);         // Return true if activated, false otherwise
}

bool askCatSkillLevel() {
  // To implement: determine the skill level of the cat
  return false;
}

//----------Stepper Functions-----------------

bool homeSteppers() {
  digitalWrite(SleepPin, HIGH);
  Serial.println();
  Serial.println(F("Homing Steppers"));
  lcd.clear();
  lcd.print(F("Homing Steppers"));
  
  int xIR_anaValue = 1000;
  int yIR_anaValue = 1000;
  int homingTol = 80;
  long nextPos = 0;
  int increment = 10;
  bool xHomed = false;
  bool yHomed = false;
  delay(200);

  while (!xHomed || !yHomed) {
    xIR_anaValue = analogRead(xIR_anaPin);
    yIR_anaValue = analogRead(yIR_anaPin);

    // Move x motor if not yet homed
    if (!xHomed) {
      nextPos = xStepper.currentPosition() + increment;
      while (xStepper.currentPosition() < nextPos) {
        xStepper.setSpeed(1200);
        xStepper.runSpeed();  // Move the motor
      }
      xStepper.stop();
      
      // Update IR sensor readings
      xIR_anaValue = analogRead(xIR_anaPin);

      // Check if X IR sensor is within homing tolerance
      if (xIR_anaValue <= homingTol) {
        delay(20);  // Small delay to debounce
        xIR_anaValue = analogRead(xIR_anaPin);  // Read again to confirm
        Serial.print(F("X3: "));
        Serial.println(xIR_anaValue);

        if (xIR_anaValue <= 300) {  // Check if sensor value is below threshold
          xStepper.setCurrentPosition(0);  // Homing complete for X
          xHomed = true;
          Serial.println(F("X Axis Homed"));
        }
      }
    }

    // Move y motor if not yet homed
    if (!yHomed) {
      // yStepper.move(stepIncrement);  // Move a small increment for Y axis
      nextPos = yStepper.currentPosition() + increment;
      while (yStepper.currentPosition() < nextPos) {
        yStepper.setSpeed(1200);
        yStepper.runSpeed();
      }
      yStepper.stop();

      yIR_anaValue = analogRead(yIR_anaPin);

      // Check if Y IR sensor is within homing tolerance
      if (yIR_anaValue <= homingTol) {
        delay(20);  // Small delay to debounce
        yIR_anaValue = analogRead(yIR_anaPin);  // Read again to confirm
        Serial.print(F("Y3: "));
        Serial.println(yIR_anaValue);

        if (yIR_anaValue <= 300) {  // Check if sensor value is below threshold
          yStepper.setCurrentPosition(0);  // Homing complete for Y
          yHomed = true;
          Serial.println(F("Y Axis Homed"));
        }
      }
    }
  }

  lcd.clear();
  lcd.print(F("Steppers Homed"));
  delay(500);
  xyPos[0] = 400; //Homing Offset 400
  xyPos[1] = 320; //Homing offset 320
  bothSteppers.moveTo(xyPos);
  bothSteppers.runSpeedToPosition();

  xStepper.setCurrentPosition(0);
  yStepper.setCurrentPosition(0);

  delay(500); //remove

  xyPos[0] = xCenterPos; //Homing Offset 400
  xyPos[1] = yCenterPos; //Homing offset 320
  bothSteppers.moveTo(xyPos);
  bothSteppers.runSpeedToPosition();
  digitalWrite(SleepPin, LOW);  // Disable stepper driver sleep mode if necessary
  return true;
}

void setMicrostepping(int microsteps){
  // Implement microstepping configuration
}

// Read joystick and set position based on it
void setPosToJoy() {
  // Feel free to use this, it's basically from the ME115 joystick code
  // Read joystick positions
  int xJoyVal = analogRead(joyXpin); // Read X-axis
  int yJoyVal = analogRead(joyYpin); // Read Y-axis

  // Map joystick values to motor positions
  xyPos[0] = map(xJoyVal, 0, 1023, xPosRange[0], xPosRange[1]); // Map X-axis
  xyPos[1] = map(yJoyVal, 0, 1023, yPosRange[0], yPosRange[1]); // Map Y-axis

  bothSteppers.moveTo(xyPos);
  bothSteppers.run();
  delay(1);
}

//---------Random Path Functions---------

// Function to display cat levels and names
void displayCatSelection() {
  lcd.clear();
  // Set the display text based on the current cat selection
  char displayText[20];

  if (currentCat < numCats - 1) {  // For levels 1 to 5
    sprintf(displayText, "^ Level %d", currentCat + 1);
  } else {  // For level 6 (Return to Main Menu)
    strcpy(displayText, "^ Return To");
  }

  // Display on the top line
  lcd.setCursor(0, 0);
  lcd.print(F("^ "));

  // To save program memory, don't store displayText in PROGMEM
  lcd.print(displayText);

  // Display the selected cat name on the bottom line
  lcd.setCursor(0, 1);
  char catName[20];
  strcpy_P(catName, catNames[currentCat]);
  lcd.print(catName);
}

void cycleCats() {
  String direction = getJoystickDirection();

  // Check if joystick is in the neutral position
  if (joystickNeutral) {
    if (direction == "Up") {
      if (!isQuietModeActivated()) {
        tone(SpeakerPin, 1200, 100);
      }
      currentCat = (currentCat + 1) % numCats;  // Cycle up
      displayCatSelection();
      joystickNeutral = false;  // Set to false to wait for neutral position
    } else if (direction == "Down") {
      if (!isQuietModeActivated()) {
        tone(SpeakerPin, 1200, 100);
      }
      currentCat = (currentCat - 1 + numCats) % numCats;  // Cycle down
      displayCatSelection();
      joystickNeutral = false;  // Set to false to wait for neutral position
    }
  }

  // Check for joystick returning to neutral
  int x = analogRead(joyXpin);
  int y = analogRead(joyYpin);
  if (x >= 250 && x <= 750 && y >= 250 && y <= 750) {
    joystickNeutral = true;  // Joystick is back in neutral position
  }
}

void executeCatAction(int selectedCat) {
  char buffer[20];
  strcpy_P(buffer, catNames[selectedCat]);

  switch (selectedCat) {
    case 0:
      // Code for Garfield
      lcd.clear();
      lcd.print(buffer);

      homeSteppers();
      digitalWrite(laserPin, HIGH);
      if (!isQuietModeActivated()) {
        pinkPanther();
      }
      digitalWrite(laserPin, LOW);
      break;
    case 1:
      // Code for Pete the Cat
      lcd.clear();
      lcd.print(buffer);

      homeSteppers();
      digitalWrite(laserPin, HIGH);
      if (!isQuietModeActivated()) {
        pinkPanther();
      }
      generatePath(100, 20, 20, 200, 2000); // (int segAmt, int numCurvPts, int numStrtPts, int speed, int stopTime, Point* start = nullptr)
      digitalWrite(laserPin, LOW);
      break;
    case 2:
      // Code for Puss in Boots
      lcd.clear();
      digitalWrite(laserPin, HIGH);
      lcd.print(buffer);
      digitalWrite(laserPin, LOW);
      break;
    case 3:
      // Return to Main Menu
      returnToMenu();
      return;
  }
}

bool executeCustomPath(){
  digitalWrite(SleepPin, HIGH);  // Active low: sleeps when set to low
  Serial.println(F("Custom Path"));
  lcd.clear();
  lcd.print(F("  Draw a Path!"));
  lcd.setCursor(0, 1);
  lcd.print(F(" Click to Exit."));
  tempMaxSpeed = 200;
  
  while(!isButtonReleased()){            //Change this so it can exit this part
    // Read the joystick position
    int xJoyVal = analogRead(joyXpin);
    int yJoyVal = analogRead(joyYpin);

    // Map the joystick values (0-1023) to a speed range for the stepper motors
    int joystickSpeedX = map(xJoyVal, 0, 1023, -tempMaxSpeed, tempMaxSpeed);  // Speed based on X-axis
    int joystickSpeedY = map(yJoyVal, 0, 1023, -tempMaxSpeed, tempMaxSpeed);  // Speed based on Y-axis
    // Look into nonlinear mapping.

    if (abs(xJoyVal - 512) < 45){
      joystickSpeedX = 0;
      if(outOfBounds()){
        moveSteppers(xCenterPos,yCenterPos);
      }
    }
    if (abs(yJoyVal - 512) < 45){
      joystickSpeedY = 0;
      if(outOfBounds()){
        moveSteppers(xCenterPos,yCenterPos);
      }
    }

    // Set the speed of the stepper motors
    xStepper.setSpeed(joystickSpeedX);  // Set speed for xStepper
    yStepper.setSpeed(joystickSpeedY);  // Set speed for yStepper

    // Move the motors based on the joystick input
    xStepper.runSpeed();  // Move xStepper at the set speed
    yStepper.runSpeed();  // Move yStepper at the set speed
  }
  digitalWrite(SleepPin, LOW);  // Active low: sleeps when set to low
  return true;
}

bool outOfBounds(){
  if (xStepper.currentPosition() < xPosRange[0] || xStepper.currentPosition() > xPosRange[1]){
    return true;
  }
  else if (yStepper.currentPosition() < yPosRange[0] || yStepper.currentPosition() > yPosRange[1]){
    return true;
  }
  else{
    return false;
  }
}

//====================Set Boundaries functions

bool setFourCorners() {
  // Save the current position of the stepper motors when the button is pressed
  savedPositions[pressCount][0] = xStepper.currentPosition();  // Save X position
  savedPositions[pressCount][1] = yStepper.currentPosition();  // Save Y position
  
  // Print saved position to Serial Monitor
  Serial.print(F("Saved Position "));
  Serial.print(pressCount + 1);
  Serial.print(F(": X="));
  Serial.print(savedPositions[pressCount][0]);
  Serial.print(F(", Y="));
  Serial.println(savedPositions[pressCount][1]);

  // Increment the press count
  pressCount++;

  // After 4 button presses, calculate the boundaries of the square
  if (pressCount == 4) {
    // Initialize the min/max variables
    int xMin1 = 5000, xMin2 = 5000;
    int xMax1 = -5000, xMax2 = -5000;
    int yMin1 = 5000, yMin2 = 5000;
    int yMax1 = -5000, yMax2 = -5000;

    // Find the first and second smallest/largest for X and Y
    for (int i = 0; i < 4; i++) {
      int x = savedPositions[i][0];
      int y = savedPositions[i][1];

      // Update first and second smallest X
      if (x < xMin1) {
        xMin2 = xMin1;
        xMin1 = x;
      } else if (x < xMin2) {
        xMin2 = x;
      }

      // Update first and second largest X
      if (x > xMax1) {
        xMax2 = xMax1;
        xMax1 = x;
      } else if (x > xMax2) {
        xMax2 = x;
      }

      // Update first and second smallest Y
      if (y < yMin1) {
        yMin2 = yMin1;
        yMin1 = y;
      } else if (y < yMin2) {
        yMin2 = y;
      }

      // Update first and second largest Y
      if (y > yMax1) {
        yMax2 = yMax1;
        yMax1 = y;
      } else if (y > yMax2) {
        yMax2 = y;
      }
    }

    // Assign the second smallest/largest values to boundaries
    xPosRange[0] = xMin2;  // Second smallest X
    xPosRange[1] = xMax2;  // Second largest X
    yPosRange[0] = yMin2;  // Second smallest Y
    yPosRange[1] = yMax2;  // Second largest Y

    // Print the final square boundaries to the Serial Monitor
    Serial.print(F("Square Boundary Set: "));
    Serial.print(F("X Min="));
    Serial.print(xPosRange[0]);
    Serial.print(F(", X Max="));
    Serial.print(xPosRange[1]);
    Serial.print(F(", Y Min="));
    Serial.print(yPosRange[0]);
    Serial.print(F(", Y Max="));
    Serial.println(yPosRange[1]);

    xCenterPos = (xPosRange[0] + xPosRange[1]) / 2;
    yCenterPos = (yPosRange[0] + yPosRange[1]) / 2;

    // Reset press count for next round of boundary setting
    pressCount = 0;
    lcd.clear();
    lcd.print(F("Boundaries Set!"));
    return true;
  } else {
    return false;
  }
}

//==================Generate Random Path===========================
void generatePath(int segAmt, int numCurvPts, int numStrtPts, int speed, int stopTime, Point* start = nullptr) {
  // Constants for path generation
  digitalWrite(SleepPin, HIGH);  //Active low: sleeps when set to low

  lcd.clear();
  lcd.print(F("Drawing Path For"));
  lcd.setCursor(0, 1);
  lcd.print(F(" Pete the Cat!"));
  const int SEGMENT_AMOUNT = segAmt; // Number of segments in the path 100
  const int NUM_CURVE_POINTS = numCurvPts; // Number of points for curved segments 20F
  const int NUM_STRAIGHT_POINTS = numStrtPts; // Number of points for the final straight segment 20

  xStepper.setMaxSpeed(speed);
  yStepper.setMaxSpeed(speed);

  startPosition = start ? *start : generateRandomPoint();                 //Change this to current position
  
  currentTarget = generateRandomPoint();
  
  currentCenter = {
      (startPosition.x + currentTarget.x) / 2.0,
      (startPosition.y + currentTarget.y) / 2.0
  };

  //Remove this after testing*****
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
    
    if(!random(0,10)){
      //delay(random(0,stopTime));
    }
    if(isButtonReleased()){
      xStepper.setMaxSpeed(1200);
      yStepper.setMaxSpeed(1200);
      moveSteppers(0,0);
      digitalWrite(SleepPin, LOW);
      return;
    }
  }
  generateStraightLine(currentCenter, currentTarget, NUM_STRAIGHT_POINTS);
  
  //Remove after testing***
  Serial.println("POINTS_ENDED");

  xStepper.setMaxSpeed(1200);
  yStepper.setMaxSpeed(1200);
  moveSteppers(0,0);
  digitalWrite(SleepPin, LOW);
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
        int t = (float)i / numPoints; // Calculate parameter t
        int x = start.x + t * (end.x - start.x); // Interpolate X coordinate
        int y = start.y + t * (end.y - start.y); // Interpolate Y coordinate
        moveSteppers(x, y);
    }
}

void generateCurve(const Point& start, const Point& end, const Point& control, int numPoints) {
    // Generate points for a quadratic Bezier curve defined by start, end, and control points
    for (int i = 0; i <= numPoints; ++i) {
        int t = i / numPoints; // Calculate parameter t
        // Calculate the curve point using the Bézier formula
        int x = pow(1 - t, 2) * start.x + 2 * (1 - t) * t * control.x + pow(t, 2) * end.x;
        int y = pow(1 - t, 2) * start.y + 2 * (1 - t) * t * control.y + pow(t, 2) * end.y;
        moveSteppers(x, y);
    }
}

void moveSteppers(int x, int y){
  xyPos[0] = x;//(long) x;
  xyPos[1] = y;//(long) y;

  //Remove after testing****
  Serial.print("(");
  Serial.print(x); // Print X coordinate with 2 decimal places
  Serial.print(",");
  Serial.print(y); // Print Y coordinate with 2 decimal places
  Serial.println(")");
  delay(20);

  //bothSteppers.moveTo(xyPos);
  //bothSteppers.runSpeedToPosition();
}
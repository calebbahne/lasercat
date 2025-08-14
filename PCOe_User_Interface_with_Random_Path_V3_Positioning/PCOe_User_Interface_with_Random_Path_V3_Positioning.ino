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

AccelStepper xStepper(AccelStepper::DRIVER, xStepPin, xDirPin);
AccelStepper yStepper(AccelStepper::DRIVER, yStepPin, yDirPin);
MultiStepper bothSteppers;

//Path generator
const int SEGMENT_AMOUNT = 100;      // Number of segments in the path
const int NUM_CURVE_POINTS = 20;     // Number of points for curved segments
const int NUM_STRAIGHT_POINTS = 20;  // Number of points for the final straight segment

//=======LCD Variables========
// Menu Options
const String menuOptions[] = {
  "< Random  Path >",
  "<     Quiet    >",
  "<    Sleep     >",
  "<Set Boundaries>",
  "<   Schedule   >",
  "< Custom  Path >"
};
const int numOptions = sizeof(menuOptions) / sizeof(menuOptions[0]);
int currentOption = 0;

// States
const int DISPLAY_TITLE = 0;
const int SELECT_MODE = 1;
const int EXECUTE_MODE = 2;
const int LOW_POWER = 3;
int currentState = DISPLAY_TITLE;

// Current Mode
int selectedMode = -1;

// Timing Variables
unsigned long lastActivity = 0;
unsigned long titleStartTime = 0;
unsigned long progressStartTime = 0;

// Constants
const unsigned long INACTIVITY_TIMEOUT = 30000;  // 30 sec
const unsigned long PROGRESS_INTERVAL = 250;     // 250ms

// Flags
bool lcdOn = true;
int progressBarLength = 0;

// Debounce Variables
bool joystickBusy = false;
unsigned long lastDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 200;

//Flag to ensure joystick returns to neutral before next input
bool joystickNeutral = true;

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

// Mode Functions
bool randomPathMode();
bool sleepMode();
bool quietMode();
bool scheduleMode();
bool customPathMode();
bool setBoundaries();

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  displayTitle();
  titleStartTime = millis();
  lastActivity = millis();
  progressStartTime = millis();

  //======Stepper and Joystick stuff
  xStepper.setMaxSpeed(2000);
  yStepper.setMaxSpeed(2000);

  pinMode(SleepPin, OUTPUT);//=====================================
  digitalWrite(SleepPin, LOW);  //Active low: sleeps when set to low
  pinMode(microstepPin, OUTPUT);//=====================
  digitalWrite(microstepPin, HIGH); //High for 1/16, low for 1/2 microstepping

  bothSteppers.addStepper(xStepper);
  bothSteppers.addStepper(yStepper);

  pinMode(joyButton, INPUT_PULLUP);
}

void loop() {  //Goes through the different operations the interface will do. Leave this alone for the most part.
  unsigned long currentMillis = millis();

  switch (currentState) {
    case DISPLAY_TITLE:
      clickable = false;
      // Update progress bar
      if (currentMillis - progressStartTime >= PROGRESS_INTERVAL && progressBarLength < 16) {
        isButtonReleased();
        updateProgressBar();
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
    Serial.print("test");
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
            Serial.println("Joystick " + direction);
            joystickNeutral = false;
          } else if (direction == "Right") {
            currentOption = (currentOption - 1 + numOptions) % numOptions;
            clickable = true;
            if (!isQuietModeActivated()) {
              tone(SpeakerPin, 1200, 100);
            }
            displayMenu();
            Serial.println("Joystick " + direction);
            joystickNeutral = false;
          }
        }

        // Check button release
        if (isButtonReleased()) {
          selectedMode = currentOption;
          currentState = EXECUTE_MODE;
          lastActivity = currentMillis;
          Serial.println("Button Released");
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
        Serial.println("Woke up from Low Power Mode");
      }
      break;
  }
}

// Function Definitions
void displayTitle() {
  lcd.clear();
  String title = "The LaserCat";
  int pos = (16 - title.length()) / 2;
  lcd.setCursor(pos, 0);
  lcd.print(title);

  // Initialize empty progress bar
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }

  progressBarLength = 0;
  Serial.println("Displaying Title");
}

void updateProgressBar() {
  if (progressBarLength < 16) {
    lcd.setCursor(progressBarLength, 1);
    lcd.write(byte(255));  // Full block
    progressBarLength++;
  }
}

void displayMenu() {
  clickable = true;
  lcd.clear();
  String header = "Select Mode";
  int pos = (16 - header.length()) / 2;
  lcd.setCursor(pos, 0);
  lcd.print(header);

  // Display current option
  lcd.setCursor(0, 1);
  lcd.print(menuOptions[currentOption]);  //This is a cool way of doing it

  Serial.println("Displaying Menu: " + menuOptions[currentOption]);
}

String getJoystickDirection() {
  //Need to modify this with joystick Code from before
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

//Detect button release
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

void executeSelectedMode() {
  Serial.println("Executing Mode: " + menuOptions[selectedMode]);
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
      Serial.println("Invalid Mode");
      returnToMenu();
      break;
  }
}

void returnToMenu() {
  currentState = SELECT_MODE;
  joystickNeutral = true; // Reset joystick state
  displayMenu();
  lastActivity = millis();
  Serial.println("Returned to Menu1");
  Serial.println("CS"+currentState);
}

void enterLowPowerMode() {
  lcd.clear();
  lcd.noBacklight();
  lcdOn = false;
  currentState = LOW_POWER;
  Serial.println("Entered Low Power Mode");
}

void exitLowPowerMode() {
  lcd.backlight();
  lcdOn = true;
  Serial.println("Exited Low Power Mode");
}

void buzzer(bool isClickable) {
  if (!isQuietModeActivated()) {
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
}

// =========== MODE FUNCTION DEFINITIONS ============
//       +++++    EDIT FUNCTIONS HERE    +++++

bool randomPathMode() {
  // Caleb
  // Will generate a random path while moving the laser to the specified points
  // Will check if the user has set boundaries, notify them if they haven't
  // Check the cat's skill level
  // Will ask the user for a time limit they'd like to generate a random path for
  // Don't forget to sound the buzzer
  // Garfield
  //    Really slow, lots of pauses, keep it in a central area/come back to the same points lots
  // L2
  //    Point to point
  // L3
  //    Middle speed, continuous and curved path, with accelerations. Longer pauses
  // L4
  //    Really fast, L3 but faster pace, fewer/shorter stop length
  // Puss in Boots
  //    The most fast you could possibly imagine
  //    Generate squares, draw lines and shapes on the board
  // While generating the path, count down til done on the screen. Show the difficulty level
  // Press and hold the button to stop the path generation.

  // Initial cat selection display
  displayCatSelection();  // Show the first cat option

  // Allow the user to cycle through cats
  while (true) {
    cycleCats();  // Handle joystick input to select cat

    // Add your logic here to start generating a random path
    // For example, you might want to wait for another button press to proceed
    if (isButtonReleased()) {
      executeCatAction(currentCat);  // Execute the action based on the selected cat
      break;                         // Exit the loop once an action has been taken
    }
  }

}

bool scheduleMode() {
  // Jose
  // Ask the user how long they'd like to wait to schedule a random path
  // Then have the user select the time limit they'd like to use for the path. The path will run for this time length
  // Ask the user if they'd like the path to repeat, and how many times they would like it to repeat
  // Ask the user how skilled their cat is with askCatSkillLevel();
  // Talk to Caleb to figure out how to send over the cat skill level
  // Once scheduled, will sleep until it's time to run random path
  // Wake up the screen at this point, and sound the buzzer
  // Then run the random path with a call to randomPathMode();
  lcd.clear();
  String modeName = "Schedule";
  int start = (16 - modeName.length()) / 2;
  lcd.setCursor(start, 0);
  lcd.print(modeName);

  // Clear the bottom line
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }

  delay(4000);
  returnToMenu();
}

bool customPathMode() {
  // Emma and Hailey
  // Will give the user the ability to draw a random path with the joystick
  // Sound the buzzer and turn on the laser
  // Take the joystick's position and map it to the stepper positions (Caleb will try and make a function for that)
  // Will need to ask the user for some sort of responsiveness variable
  //    If they want the laser to move really fast, make the steppers more responsive to joystick movement
  // See if it's possible to save the path for future use

  lcd.clear();
  String modeName = "Custom Path";
  int start = (16 - modeName.length()) / 2;
  lcd.setCursor(start, 0);
  lcd.print(modeName);

  // Clear the bottom line
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }

  delay(4000);
  returnToMenu();
}

bool setBoundaries() {
  //____
  // Will use the joystick to set the top, bottom, left, and right boundaries to be used as limits
  //    when generating a random path
  // Will prompt the user to input the each of the four boundaries in turn. They'll move the laser
  //    to where they want it, then press the button.
  // Will need to have some sort of neutral zone in the center of the joystick's motion where the position won't change
  // Once each of the four points have been selected, output that the boundaries have been set.
  // xTempBoundary and yTempBoundary are arrays for storing the allowable left and right ranges.
  lcd.clear();
  String modeName = "Set Boundaries";
  int start = (16 - modeName.length()) / 2;
  lcd.setCursor(start, 0);
  lcd.print(modeName);

  // Clear the bottom line
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }

  while(true){
  setPosToJoy();
  }
  delay(4000);
  returnToMenu();
}

bool quietMode() {
  // Read the current state from EEPROM
  int eepromValue = EEPROM.read(0);    // Read from the first byte
  bool isActive = (eepromValue == 1);  // True if activated

  // Toggle the state
  isActive = !isActive;  // Switch between activated and deactivated

  // Display Quiet Mode
  lcd.clear();
  String modeName = "Quiet Mode:";
  lcd.setCursor(0, 0);  // Align to the left
  lcd.print(modeName);

  // Display Activated or Deactivated
  lcd.setCursor(0, 1);  // Align to the left
  if (isActive) {
    lcd.print("Activated   ");
  } else {
    lcd.print("Deactivated ");
  }

  // Save the new state to EEPROM
  Serial.print("Current Quiet Mode state: ");
  Serial.println(isActive ? "Activated" : "Deactivated");

  // Write the new state to EEPROM (1 for activated, 0 for deactivated)
  EEPROM.write(0, isActive ? 1 : 0);

  delay(2000);  // Wait for 2 seconds
  returnToMenu();
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
  //Will determine the skill level of the cat, prompting the user, from Garfield to Puss in Boots level
}

//----------Stepper Functions-----------------
bool homeSteppers() {
  //This will set the steppers to their home positions
}

void setPosToJoy() {
  //Feel free to use this, it's basically from the ME115 joystick code
  // Read joystick positions
  xJoyVal = analogRead(joyXpin); // Read X-axis
  yJoyVal = analogRead(joyYpin); // Read Y-axis

  // Map joystick values to servo angles (0 to 180 degrees)
 xyPos[0] = map(xJoyVal, 0, 1023, xPosRange[0], xPosRange[1]); // Map X-axis
 xyPos[1] = map(yJoyVal, 0, 1023, yPosRange[0], yPosRange[1]); // Map Y-axis

  bothSteppers.moveTo(xyPos);
  bothSteppers.run();
 delay(1);
}

//----------Other Funtions--------------
void soundBuzzer() {
  //____
}

//---------Random Path Functions---------

// Function to display cat levels and names
void displayCatSelection() {
  lcd.clear();
  String displayText;

  // Set the display text based on the current cat selection
  if (currentCat < numCats - 1) {  // For levels 1 to 5
    displayText = "^ Level " + String(currentCat + 1);
  } else {  // For level 6 (Return to Main Menu)
    displayText = "^ Return To";
  }

  // Display on the top line
  lcd.setCursor(0, 0);
  lcd.print(displayText);

  // Display the selected cat name on the bottom line
  lcd.setCursor(0, 1);
  lcd.print(catNames[currentCat]);
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

  // Check button release to select the current cat
  if (isButtonReleased()) {
    executeCatAction(currentCat);
  }
}


void executeCatAction(int selectedCat) {
  switch (selectedCat) {
    case 0:
      // Code for Garfield
      lcd.clear();
      lcd.print("Garfield");
      break;
    case 1:
      // Code for Pete the Cat
      lcd.clear();
      lcd.print("Pete the Cat");
      break;
    case 2:
      // Code for Cat in the Hat
      lcd.clear();
      lcd.print("Cat in the Hat");
      break;
    case 3:
      // Code for Tigger
      lcd.clear();
      lcd.print("Tigger");
      break;
    case 4:
      // Code for Puss in Boots
      lcd.clear();
      lcd.print("Puss in Boots");
      break;
    case 5:
      // Return to Main Menu
      returnToMenu();
      return;
      Serial.print("A");
  }
  delay(2000);     // Display the selection for 2 seconds
  returnToMenu();  // Return to the menu after selection
  Serial.print("B");
}

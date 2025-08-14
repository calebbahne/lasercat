#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin Definitions
const int joyButton = 13;  // Joystick button
const int joyXpin = A3;    // Joystick X axis
const int joyYpin = A2;    // Joystick Y axis

// Menu Options
const String menuOptions[] = {
  "< Random  Path >",
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
const unsigned long INACTIVITY_TIMEOUT = 30000;  // 1 minute
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

// Mode Functions
bool randomPathMode();
bool sleepMode();
bool scheduleMode();
bool customPathMode();

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(joyButton, INPUT_PULLUP);

  displayTitle();
  titleStartTime = millis();
  lastActivity = millis();
  progressStartTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    case DISPLAY_TITLE:
      // Update progress bar
      if (currentMillis - progressStartTime >= PROGRESS_INTERVAL && progressBarLength < 16) {
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
      // Check inactivity
      if (currentMillis - lastActivity >= INACTIVITY_TIMEOUT) {
        enterLowPowerMode();
      } else {
        // Handle joystick input
        String direction = getJoystickDirection();
        if (direction != "Neutral" && joystickNeutral) {  // Only process if neutral
          lastActivity = currentMillis;
          if (direction == "Left") {
            currentOption = (currentOption + 1) % numOptions;
            displayMenu();
            Serial.println("Joystick " + direction);
            joystickNeutral = false;  // Wait for joystick to return to neutral
          } else if (direction == "Right") {
            currentOption = (currentOption - 1 + numOptions) % numOptions;
            displayMenu();
            Serial.println("Joystick " + direction);
            joystickNeutral = false;  // Wait for joystick to return to neutral
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
  lcd.clear();
  String header = "Select Mode";
  int pos = (16 - header.length()) / 2;
  lcd.setCursor(pos, 0);
  lcd.print(header);

  // Display current option
  lcd.setCursor(0, 1);
  lcd.print(menuOptions[currentOption]);

  Serial.println("Displaying Menu: " + menuOptions[currentOption]);
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

//Detect button release
bool isButtonReleased() {
  static bool lastState = HIGH;
  bool buttonState = digitalRead(joyButton);
  if (buttonState == HIGH && lastState == LOW) {
    lastState = HIGH;
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
      sleepMode();
      break;
    case 2:
      setBoundaries();
      break;
    case 3:
      scheduleMode();
      break;
    case 4:
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
  displayMenu();
  lastActivity = millis();
  Serial.println("Returned to Menu");
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

// Mode Function Implementations

bool randomPathMode() {
  lcd.clear();
  String modeName = "Random Path";
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

bool scheduleMode() {
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

  delay(4000);
  returnToMenu();
}

bool sleepMode() {
  // Implement sleep mode logic
  enterLowPowerMode();
  return false;
}

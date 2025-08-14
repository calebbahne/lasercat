#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the LCD 
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
const int joyButton = 13;   // Joystick button
const int joyXpin = A3;  // Joystick X axis
const int joyYpin = A2;  // Joystick Y axis

// Joystick Input Structures
struct JoystickInput {
  String direction;  // "Left", "Right", "Up", "Down", "Neutral"
  bool buttonClicked;
};

struct JoystickPositions {
  int x;
  int y;
};

// Menu options
const String menuOptions[] = {
  "< Random  Path >",
  "<    Sleep     >",
  "<Set Boundaries>",
  "<   Schedule   >",
  "< Custom  Path >",
};
const int numOptions = sizeof(menuOptions) / sizeof(menuOptions[0]);
int currentOption = 0;

// States
enum State {
  DISPLAY_TITLE,
  SELECT_MODE,
  EXECUTE_MODE,
  LOW_POWER
};
State currentState = DISPLAY_TITLE;

// Current mode function pointer
typedef bool (*ModeFunction)();
ModeFunction currentModeFunction = nullptr;

// Timing variables
unsigned long lastActivity = 0;
unsigned long titleDisplayStart = 0;
const unsigned long TITLE_DISPLAY_DURATION = 4000;  // 4 seconds
const unsigned long INACTIVITY_TIMEOUT = 60000;     // 1 minute

// Debounce variables
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// LCD state
bool lcdOn = true;

// Loading bar variables
int progressBarLength = 0;
unsigned long lastProgressUpdate = 0;
const unsigned long PROGRESS_INTERVAL = 250;  // Update every 250ms

// Joystick Debounce Flag
bool joystickAwaitingRelease = false;

// Function prototypes
JoystickInput readJoystickInput();
JoystickPositions readJoystickPositions();
bool isButtonClicked();
void enterLowPower();
void exitLowPower();
bool checkJoystickMoved();
void executeSelectedMode();
void returnToHome();
void enterSleepMode();
void updateLoadingBar();
void clearLCD();
void updateDisplay();

// Function prototypes for modes
bool loadRandomPath();
bool loadCustomPath();
bool loadSchedule();
bool setBoundaries();
bool loadSleepMode();

void setup() {
  // Initialize Serial
  Serial.begin(9600);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize joystick
  pinMode(joyButton, INPUT_PULLUP);  // Assuming active low

  // Display "CatBot3000" centered on top line
  lcd.clear();
  String title = "CatBot3000";
  int titleStart = (16 - title.length()) / 2;
  lcd.setCursor(titleStart, 0);
  lcd.print(title);

  // Initialize second line with empty progress bar
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }

  titleDisplayStart = millis();
  lastActivity = millis();
  lastProgressUpdate = millis();
  progressBarLength = 0;

  Serial.println("Display Title: CatBot3000");
}

void loop() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    case DISPLAY_TITLE:
      // Update loading bar
      if (currentMillis - lastProgressUpdate >= PROGRESS_INTERVAL && progressBarLength < 16) {
        updateLoadingBar();
        lastProgressUpdate = currentMillis;
      }

      // Check if title display duration has passed
      if (currentMillis - titleDisplayStart >= TITLE_DISPLAY_DURATION) {
        currentState = SELECT_MODE;
        updateDisplay();
        lastActivity = currentMillis;
      }
      break;

    case SELECT_MODE:
      // Check for inactivity
      if (currentMillis - lastActivity >= INACTIVITY_TIMEOUT) {
        enterLowPower();
      } else {
        // Handle joystick input
        JoystickInput joystickInput = readJoystickInput();
        if (joystickInput.direction != "Neutral") {
          // User moved joystick
          lastActivity = currentMillis;

          if (joystickInput.direction == "Left") {
            currentOption = (currentOption + 1) % numOptions;
            updateDisplay();
            Serial.println("Joystick Left");
          } else if (joystickInput.direction == "Right") {
            currentOption = (currentOption - 1 + numOptions) % numOptions;
            updateDisplay();
            Serial.println("Joystick Right");
          } else if (joystickInput.direction == "Up") {
            // Implement Up navigation if needed
            // For example, scroll up or perform another action
            Serial.println("Joystick Up");
          } else if (joystickInput.direction == "Down") {
            // Implement Down navigation if needed
            // For example, scroll down or perform another action
            Serial.println("Joystick Down");
          }
        }

        // Check for button click
        if (joystickInput.buttonClicked) {
          // Handle button press
          currentState = EXECUTE_MODE;
          executeSelectedMode();
          lastActivity = currentMillis;
          Serial.println("Button Clicked");
        }
      }
      break;

    case EXECUTE_MODE:
      if (currentModeFunction != nullptr) {
        bool modeCompleted = currentModeFunction();
        if (modeCompleted) {
          currentModeFunction = nullptr;
          returnToHome();
        }
      }
      break;

    case LOW_POWER:
      // Wait for joystick movement or button click to wake up
      JoystickInput sleepWakeInput = readJoystickInput();
      if (sleepWakeInput.direction != "Neutral" || sleepWakeInput.buttonClicked) {
        exitLowPower();
        lastActivity = millis();
        currentState = SELECT_MODE;
        updateDisplay();
        Serial.println("Woke up from Low Power Mode");
      }
      break;
  }
}

// ------------------ Function Definitions ------------------ //

// Function to clear the LCD screen
void clearLCD() {
  lcd.clear();
  // Optionally, you can add further initialization here if needed
}

// Function to read overall joystick direction and button click
JoystickInput readJoystickInput() {
  JoystickInput input;
  input.direction = "Neutral";
  input.buttonClicked = false;

  // Read joystick positions
  int xValue = analogRead(joyXpin);
  int yValue = analogRead(joyYpin);

  // Define thresholds - Adjust based on your joystick's behavior
  const int LEFT_THRESHOLD = 250;
  const int RIGHT_THRESHOLD = 750;
  const int UP_THRESHOLD = 250;
  const int DOWN_THRESHOLD = 750;

  // Determine direction with hysteresis to prevent multiple triggers
  if (joystickAwaitingRelease) {
    // Check if joystick has returned to neutral
    if (xValue > (LEFT_THRESHOLD + 100) && xValue < (RIGHT_THRESHOLD - 100) &&
        yValue > (UP_THRESHOLD + 100) && yValue < (DOWN_THRESHOLD - 100)) {
      joystickAwaitingRelease = false;
    }
    // If not yet released, ignore other inputs
  } else {
    if (xValue < LEFT_THRESHOLD) {
      input.direction = "Left";
      joystickAwaitingRelease = true;
    } else if (xValue > RIGHT_THRESHOLD) {
      input.direction = "Right";
      joystickAwaitingRelease = true;
    } else if (yValue < UP_THRESHOLD) {
      input.direction = "Up";
      joystickAwaitingRelease = true;
    } else if (yValue > DOWN_THRESHOLD) {
      input.direction = "Down";
      joystickAwaitingRelease = true;
    }
    // Else, Neutral
  }

  // Detect button click
  bool buttonPressed = digitalRead(joyButton) == LOW;

  static bool lastButtonState = HIGH;
  if (buttonPressed && lastButtonState == HIGH) {
    // Button pressed
    lastButtonState = LOW;
    // Do not trigger on press
  } else if (!buttonPressed && lastButtonState == LOW) {
    // Button released
    lastButtonState = HIGH;
    input.buttonClicked = true;
  }

  return input;
}

// Function to get specific X and Y positions
JoystickPositions readJoystickPositions() {
  JoystickPositions pos;
  pos.x = analogRead(joyXpin);
  pos.y = analogRead(joyYpin);
  return pos;
}

// Function to check if joystick was moved (for waking up from low power)
bool checkJoystickMoved() {
  JoystickPositions pos = readJoystickPositions();

  // Define movement thresholds
  // Consider movement if X or Y changes beyond a threshold from center (e.g., 512)
  if (pos.x < 200 || pos.x > 800 || pos.y < 200 || pos.y > 800) {
    return true;
  }
  return false;
}

// Function to enter low power mode
void enterLowPower() {
  clearLCD();          // Clear the screen
  lcd.noBacklight();   // Turn off backlight for low power
  currentState = LOW_POWER;
  Serial.println("Entered Low Power Mode");
}

// Function to exit low power mode
void exitLowPower() {
  lcd.backlight();  // Turn on backlight
  // Don't clear screen here; it will be cleared in updateDisplay
  Serial.println("Exiting Low Power Mode");
}

// Function to execute the selected mode
void executeSelectedMode() {
  switch (currentOption) {
    case 0:
      currentModeFunction = loadRandomPath;
      break;
    case 1:
      currentModeFunction = loadSleepMode;
      break;
    case 2:
      currentModeFunction = setBoundaries;
      break;
    case 3:
      currentModeFunction = loadSchedule;
      break;
    case 4:
      currentModeFunction = loadCustomPath;
      break;
    default:
      Serial.println("Invalid Option Selected");
      returnToHome();
      break;
  }

  Serial.print("Executing ");
  Serial.println(menuOptions[currentOption]);
}

// Function to return to home/select mode
void returnToHome() {
  currentState = SELECT_MODE;
  updateDisplay();
  lastActivity = millis();
  Serial.println("Returned to Select Mode");
}

// Function to update the loading bar on the title screen
void updateLoadingBar() {
  if (progressBarLength < 16) {
    lcd.setCursor(progressBarLength, 1);
    lcd.write(byte(255));  // Full block character for progress
    progressBarLength++;
  }
}

// Function to update the LCD display based on current state
void updateDisplay() {
  lcd.clear();
  if (currentState == DISPLAY_TITLE) {
    String title = "CatBot3000";
    int titleStart = (16 - title.length()) / 2;
    lcd.setCursor(titleStart, 0);
    lcd.print(title);

    // Initialize empty progress bar
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }

    Serial.println("Display Title: CatBot3000");
  } else if (currentState == SELECT_MODE) {
    // Display "Select Mode" centered on top
    String selectMode = "Select Mode";
    int selectStart = (16 - selectMode.length()) / 2;
    lcd.setCursor(selectStart, 0);
    lcd.print(selectMode);

    // Display current menu option on bottom
    lcd.setCursor(0, 1);
    lcd.print(menuOptions[currentOption]);

    Serial.println("Display Select Mode");
  } else if (currentState == EXECUTE_MODE) {
    // The specific mode functions handle their own display
  }
}

// Function to enter sleep mode from any mode
void enterSleepMode() {
  enterLowPower();
}

// ------------------ Mode Functions ------------------ //
// Each mode function returns a boolean indicating whether it has completed its execution
// They use static variables to maintain their state between calls

// Function for "Random Path" mode
bool loadRandomPath() {
  static unsigned long modeStartTime = 0;
  static bool initialized = false;

  if (!initialized) {
    // Initialize mode
    clearLCD();
    String modeName = "Random Path";
    int start = (16 - modeName.length()) / 2;
    lcd.setCursor(start, 0);
    lcd.print(modeName);

    // Clear the bottom line
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }

    modeStartTime = millis();
    initialized = true;

    Serial.println("Executing Random Path Mode");
    return false;  // Mode not yet completed
  }

  // Check if 4 seconds have passed
  if (millis() - modeStartTime >= 4000) {
    // Mode completed
    initialized = false;  // Reset for next execution
    return true;
  }

  // Mode is still running
  return false;
}

// Function for "Custom Path" mode
bool loadCustomPath() {
  static unsigned long modeStartTime = 0;
  static bool initialized = false;

  if (!initialized) {
    // Initialize mode
    clearLCD();
    String modeName = "Custom Path";
    int start = (16 - modeName.length()) / 2;
    lcd.setCursor(start, 0);
    lcd.print(modeName);

    // Clear the bottom line
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }

    modeStartTime = millis();
    initialized = true;

    Serial.println("Executing Custom Path Mode");
    return false;  // Mode not yet completed
  }

  // Check if 4 seconds have passed
  if (millis() - modeStartTime >= 4000) {
    // Mode completed
    initialized = false;  // Reset for next execution
    return true;
  }

  // Mode is still running
  return false;
}

// Function for "Schedule" mode
bool loadSchedule() {
  static unsigned long modeStartTime = 0;
  static bool initialized = false;

  if (!initialized) {
    // Initialize mode
    clearLCD();
    String modeName = "Schedule";
    int start = (16 - modeName.length()) / 2;
    lcd.setCursor(start, 0);
    lcd.print(modeName);

    // Clear the bottom line
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }

    modeStartTime = millis();
    initialized = true;

    Serial.println("Executing Schedule Mode");
    return false;  // Mode not yet completed
  }

  // Check if 4 seconds have passed
  if (millis() - modeStartTime >= 4000) {
    // Mode completed
    initialized = false;  // Reset for next execution
    return true;
  }

  // Mode is still running
  return false;
}

// Function for "Set Boundaries" mode
bool setBoundaries() {
  static unsigned long modeStartTime = 0;
  static bool initialized = false;

  if (!initialized) {
    // Initialize mode
    clearLCD();
    String modeName = "Set Boundaries";
    int start = (16 - modeName.length()) / 2;
    lcd.setCursor(start, 0);
    lcd.print(modeName);

    // Clear the bottom line
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      lcd.print(" ");
    }

    modeStartTime = millis();
    initialized = true;

    Serial.println("Executing Set Boundaries Mode");
    return false;  // Mode not yet completed
  }

  // Check if 4 seconds have passed
  if (millis() - modeStartTime >= 4000) {
    // Mode completed
    initialized = false;  // Reset for next execution
    return true;
  }

  // Mode is still running
  return false;
}

// Function for "Sleep" mode
bool loadSleepMode() {
  static bool initialized = false;

  if (!initialized) {
    // Initialize sleep mode
    enterLowPower();
    initialized = true;
    Serial.println("Executing Sleep Mode");
    // Sleep mode doesn't complete on its own; it waits for wake-up
    return false;
  }

  // Sleep mode remains active until user interaction wakes up the system
  return false;
}

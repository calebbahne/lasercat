//#include <LiquidCrystal.h>
#include <Wire.h>
//#include <Adafruit_LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>


//const int  en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;
/*
// Define I2C Address - change if reqiuired
const int i2c_addr = 0x3F;
*/
LiquidCrystal_I2C lcd(0x27, 16, 2);
//LCD
//const int rs = 12, enLCD = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
//LiquidCrystal_I2C lcd(rs, enLCD, d4, d5, d6, d7);

//lowPowerMode
unsigned long currentTime, prevTime;
bool lowPower = false;

//Buttons
const int button1 = 8;
const int button2 = 9;
int buttonState = 0;

void setup() {
  // set up the LCD's number of columns and rows:
  //lcd.begin(16, 2);
  Serial.begin(9600);
  prevTime = millis();
  lcd.setBacklight(1);
  lcd.begin(16, 2);
}

void loop() {
  long randomNumber = random(1000, 10000001);
  
  lcd.setCursor(0, 0); // Set cursor to top left
  lcd.print("LaserCat"); // Print "LaserCat"
  
  // Create a buffer to hold the number as a string
  char numberStr[12]; // Enough to hold up to 10,000,000 and the null terminator
  itoa(randomNumber, numberStr, 10); // Convert number to string in base 10
  
  // Print the number next to "LaserCat"
  lcd.print(numberStr);
  
  lcd.setCursor(0, 1); // Set cursor to bottom left
  lcd.print("<      OK      >"); // Ensure the text fits
  delay(2000); // Keep text on screen for 2 seconds
  
  lcd.setBacklight(0);
  lcd.clear();
  delay(1000);
  lcd.setBacklight(1);
}


//Goes through the functions of the LCD
void runLCD() {
  lcd.clear();
  lcd.print("    Welcome");
  lcd.setCursor(0, 1);  //Sets cursor to bottom left
  lcd.print(" Cool tagline");
  lcd.display();
  delay(2000);
  lcd.clear();
  delay(2000);
}

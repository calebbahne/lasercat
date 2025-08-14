#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.begin(16, 2);
  lcd.setBacklight(1); // Make sure to turn on the backlight
  lcd.setCursor(0,0);
  lcd.print("Hello, World!"); // Print a test message
}

void loop() {
  // You can add additional code here if needed
}

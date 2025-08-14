/*
 This sketch prints "Hello World!" to the LCD and uses the
 display() and noDisplay() functions to turn on and off
 the display.

 The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/

#include <LiquidCrystal.h>

//LCD
const int rs = 12, enLCD = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, enLCD, d4, d5, d6, d7);

//lowPowerMode
unsigned long currentTime, prevTime;
bool lowPower = false;

//Buttons
const int button1 = 8;
const int button2 = 9;
int buttonState = 0;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  Serial.begin(9600);
  prevTime = millis();
}

void loop() {
  currentTime = millis();
  if ((currentTime - prevTime >= 1000) && lowPower == false){
    lowPowerMode();
  }
  checkButtons();
  Serial.print("ButtonState = ");
  Serial.println(buttonState);
  if (buttonState > 0){
    lowPower = false;
    screenWake();
    runLCD();
    prevTime = millis();
  }
  delay(500);
}

void checkButtons(){
  //Consider making it an int and having it return a value
  //Returns 0 if no buttons pressed
  //Returns 1 if button 1 pressed
  //Returns 2 if button 2 pressed
  //Returns 3 if both buttons pressed
  buttonState = 0;//random(0,4); // 0, 1, 2, 3
}

//Displays the start up sequence and project name
void screenWake(){
  //digitalWrite(enLCD, HIGH);
  lcd.print("   CatBot3000");
  //Maybe we can change this to do a cool loading animation
  for (int i = 1; i <= 3; i++){
  	lcd.display();
  	delay(500);
    lcd.noDisplay();
  	delay(500);
  }
  lcd.clear(); 
  lcd.print("    Welcome");
  lcd.setCursor(0,1); //Sets cursor to bottom left
  lcd.print(" Cool tagline");
  lcd.display();
  delay(2000);
  lcd.noDisplay();
  delay(500);
  lcd.clear();
}

//Goes through the functions of the LCD
void runLCD(){
  Serial.println("runLCD");
}

//sets enable pins to low to conserve power
void lowPowerMode(){
  Serial.println("Low Power Mode");
  //digitalWrite(enLCD, LOW);
  //also write the steppers to low
  lowPower = true;
}


/*
  //Autoscrolls
  lcd.setCursor(16, 1);
  lcd.autoscroll();
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    lcd.print(thisChar);
    delay(500);
  }
  // turn off automatic scrolling
  lcd.noAutoscroll();
  lcd.clear();
  
  //Prints from the serial monitor
  if (Serial.available()) {
    // wait a bit for the entire message to arrive
    delay(100);
    // clear the screen
    lcd.clear();
    // read all the available characters
    while (Serial.available() > 0) {
      // display each character to the LCD
      lcd.write(Serial.read());
    }
  }
  */
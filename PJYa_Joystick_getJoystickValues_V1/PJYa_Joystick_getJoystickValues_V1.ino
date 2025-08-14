//Joystick
const int joyXpin = A2; //analog pins
const int joyYpin = A3;
const int joyButton = 13;
int xJoyVal = 0; //initializes x value to be 0
int yJoyVal = 0; //initializes y value to be 0
int buttonState = 0;

void setup()  {
  Serial.begin(9600);
}

void loop()  {
  //read analog X and Y values
  getJoystickValues();
  //map to stepper range
}

void getJoystickValues(){
  xJoyVal = analogRead(joyXpin);
  yJoyVal = analogRead(joyYpin);
  buttonState = digitalRead(joyButton);
  
  Serial.print("X: ");
  Serial.print(xJoyVal);
  Serial.print(" | Y: ");
  Serial.print(yJoyVal);
  Serial.print(" | Button: ");
  Serial.println(buttonState);
}

//Checks for the homing indicator in front of the x and y steppers, tells when it's homed.
const int xIR_anaPin = A0;
const int yIR_anaPin = A1;
int xIR_anaValue = 0;
int yIR_anaValue = 0; 
int homingTol = 200;

//Laser
const int laserPin = 11; //Needs PWM
int laserIntensity = 255;

void setup()
{
  Serial.begin(9600);
  pinMode(xIR_anaPin,INPUT);
  pinMode(yIR_anaPin,INPUT);
  pinMode(laserPin, OUTPUT);
}

void loop()
{
  digitalWrite(laserPin, HIGH);

  xIR_anaValue = analogRead(xIR_anaPin);
  yIR_anaValue = analogRead(yIR_anaPin);

  Serial.print("X Analog: ");
  Serial.print(xIR_anaValue);
  Serial.print("  Y Analog: ");
  Serial.println(yIR_anaValue);

  if (xIR_anaValue < homingTol){
    Serial.println("X Homed");
  }
  if (yIR_anaValue < homingTol){
    Serial.println("Y Homed");
  }

  delay(200);
}
//Checks for an object, sends to the screen
// Arduino Uno  -->   TCRT5000
// 5v           --->   VCC
// Grnd         --->   Grnd
// A3           --->   A0
// D12           --->   D0


const int xIR_digPin = 11;   //Eventually remove
const int yIR_digPin = 12;   //Eventually remove
const int xIR_anaPin = A0;
const int yIR_anaPin = A1;
int xIR_anaValue = 0;
int yIR_anaValue = 0; 
int xIR_digValue = 0;   //remove
int yIR_digValue = 0;   //remove

void setup()
{
  Serial.begin(9600);
  pinMode(xIR_digPin,INPUT);
  pinMode(xIR_anaPin,INPUT);
  pinMode(yIR_digPin,INPUT);
  pinMode(yIR_anaPin,INPUT);
}

void loop()
{
  //Note: currently both digital and analog are inputs. Should one be an output? Do i need both?
  xIR_anaValue = analogRead(xIR_anaPin);
  xIR_digValue = digitalRead(xIR_digPin);
  yIR_anaValue = analogRead(yIR_anaPin);
  yIR_digValue = digitalRead(yIR_digPin);


  Serial.print("X Analog: ");
  Serial.print(xIR_anaValue);
  Serial.print("\t X Digital:");  //Remove
  Serial.print(xIR_digValue);     //Remove

  Serial.print("  Y Analog: ");
  Serial.print(yIR_anaValue);
  Serial.print("\t Y Digital:");  //Remove
  Serial.println(yIR_digValue);   //Remove

  delay(200);
}
#include "pitches.h"

#define SpeakerPin 12

void pinkPanther();

void setup()
{
  pinMode(SpeakerPin, OUTPUT);
}

void loop(){
  pinkPanther();
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
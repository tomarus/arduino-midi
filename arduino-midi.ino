#include "MidiController.h"

// digital pins
const int START_BUTTON = 2;
const int STOP_BUTTON = 3;

const int GREEN_LED_PIN = 5;
const int BLUE_LED_PIN = 11;
const int RED_LED_PIN = 6;
const int BEAT_LED_PIN = 13;

// analog pins
const int POT_PIN = 0;

// misc defaults
const int channel = 12;
const int cc = 74;

int lastVal = 0;
int tick = 0;
int startPressed = 0;
int stopPressed = 0;
int stopMode = 0;

MidiController midiController;
    
void timerCallback() {
  analogWrite(BEAT_LED_PIN, 0);
  
  tick++;
  if (tick%24==0) {
    analogWrite(BEAT_LED_PIN, 255);
 }
}

void setup() {
  midiController.begin();
  midiController.TimerCallback(timerCallback);
//  Serial.begin(115200);
  stopMode = 1;
  tick=-1;
//  Serial.println("Ready.");
}

void loop() {
  midiController.loop();

  // handle stop/start
  if (digitalRead(START_BUTTON) == LOW) {
    startPressed++;
    if (startPressed == 1) {
//      Serial.println("Start button pressed.");
      midiController.Start();
      stopMode = 0;
      tick=-1;
    }
  } else {
    startPressed = 0;
  }

  if (digitalRead(STOP_BUTTON) == LOW) {
    stopPressed++;
    if (stopPressed == 1) {
      if (stopMode == 0) {
//        Serial.println("Stop button pressed.");
        midiController.Stop(false);
        stopMode = 1;
      } else if (stopMode == 1) {
        midiController.Stop(true);
        stopMode = 2;
      }
    }
  } else {
    stopPressed = 0;
  }

  return;

  // handle potmeter
  int sensorValue = analogRead(POT_PIN);

  int redValue = constrain(map(sensorValue, 0, 512, 255, 0), 0, 255);
  int greenValue = constrain(map(sensorValue, 0, 512, 0, 255),0,255)-constrain(map(sensorValue, 512, 1023, 0, 255),0,255);
  int blueValue = constrain(map(sensorValue, 512, 1023, 0, 255), 0, 255);

  analogWrite(RED_LED_PIN, redValue/2);
  analogWrite(GREEN_LED_PIN, greenValue/2);
  analogWrite(BLUE_LED_PIN, blueValue/2);

  if (sensorValue/8 == lastVal/8) {
    return;
  }
  lastVal = sensorValue;
//  midiOUT.sendControlChange(cc, sensorValue / 8, channel);
}


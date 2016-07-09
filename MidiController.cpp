#include "Arduino.h"
#include "MidiController.h"
#include <SoftwareSerial.h>
#include <TimerOne.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

float bpm = 125;
MidiController *instance;

//

struct mySettings : public midi::DefaultSettings {
  static const bool UseRunningStatus = false;
};
 
SoftwareSerial softSerial(0,7);
MIDI_CREATE_CUSTOM_INSTANCE(SoftwareSerial, softSerial, midiOUT, mySettings);

SoftwareSerial softINSerial(8,0);
MIDI_CREATE_CUSTOM_INSTANCE(SoftwareSerial, softINSerial, midiIN, mySettings);

//

static void handleClock() {
  Serial.println("Received clock message.");
}

static void handleTimerCallback() {
  instance->Clock();
  if (instance->timerCallback) {
    instance->timerCallback();
  }
}

static void handleSysex(byte *array, unsigned size) {
  if (size < 5) return;
  if (!(array[1] == 0x7d && array[2] == 0x2a && array[3] == 0x4d)) return;
  
  Serial.print("Received sysex length ");
  Serial.println(size);

  for (int i=4; i<size-1; i++) {
    int command = array[i];
    Serial.print("Command received ");
    Serial.println(command, HEX);
    if (command >= 0x40) {
      return;
    }
    
    switch (command) {
    case 0: {
      byte response[] = {0xf0, 0x7d, 0x2a, 0x4d, 
        0x40, 0x01,            // RESPONSE 0x40, version 1
        0x01, 0x01,            // 1 inport, 1 outport
        byte(int(bpm) >> 7),   // current speet msb
        byte(int(bpm) & 0x7f), // current speed lsb
        0xf7};
      midiOUT.sendSysEx(sizeof(response), response, true);
      break;
    }
    case 1: {
      i += 7;
      break;
    }
    case 2: {
      bpm = (array[i+1] << 7) + array[i+2];
      Timer1.setPeriod(60000000.0/bpm/24.0);
      i += 2;
      break;
    }
    default: {
      Serial.print("Unsupported command: ");
      Serial.println(command, HEX);
      break;
    }
    }
  }
}

//

MidiController::MidiController() {}

void MidiController::begin() {
  Serial.println("MidiController begin"); 
  instance = this;
  midiOUT.begin();
  midiOUT.turnThruOff();
  
  midiIN.begin();
  midiIN.turnThruOff();
  midiIN.setHandleClock(handleClock);
  midiIN.setHandleSystemExclusive(handleSysex);

  this->setupTimer();
}

void MidiController::loop() {
  if (midiOUT.read()) {
    Serial.println("Received type " + midiOUT.getType());
  }
  
  if (midiIN.read()) {
    int t = midiIN.getType();
    if (!(t == midi::Clock || t == midi::ActiveSensing || t == midi::SystemReset || t == midi::SystemExclusive)) {
      Serial.print("Received type ");
      Serial.println(t, HEX);
    }
  }
}

void MidiController::Start() {
  midiOUT.sendRealTime(midi::Start);
  this->setupTimer();
}

void MidiController::Stop(bool hard) {
  midiOUT.sendRealTime(midi::Stop);
  if (hard) {
    Timer1.stop();
  }
}

void MidiController::Clock() {
  midiOUT.sendRealTime(midi::Clock);
}

void MidiController::TimerCallback(void (*fn)(void)) {
  timerCallback = fn;
}

//

void MidiController::setupTimer() {
  Timer1.initialize(60000000.0/bpm/24.0);
  Timer1.attachInterrupt(handleTimerCallback);
}


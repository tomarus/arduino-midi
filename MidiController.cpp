#include "Arduino.h"
#include "MidiController.h"
#include <SoftwareSerial.h>
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
  static const bool Use1ByteParsing = true;
  static const bool HandleNullVelocityNoteOnAsNoteOff = false;
};
 
SoftwareSerial softSerial(8,7);
MIDI_CREATE_CUSTOM_INSTANCE(SoftwareSerial, softSerial, midiOUT, mySettings);

SoftwareSerial softSerial2(1,12);
MIDI_CREATE_CUSTOM_INSTANCE(SoftwareSerial, softSerial2, midiOUT2, mySettings);

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, midiIN, mySettings);

// Temporary for debugging
const int GREEN_LED_PIN = 5;
const int BLUE_LED_PIN = 11;
const int RED_LED_PIN = 6;
void randomize() {
  int sensorValue = random(1023);
  int redValue = constrain(map(sensorValue, 0, 512, 255, 0), 0, 255);
  int greenValue = constrain(map(sensorValue, 0, 512, 0, 255),0,255)-constrain(map(sensorValue, 512, 1023, 0, 255),0,255);
  int blueValue = constrain(map(sensorValue, 512, 1023, 0, 255), 0, 255);
  analogWrite(RED_LED_PIN, redValue/2);
  analogWrite(GREEN_LED_PIN, greenValue/2);
  analogWrite(BLUE_LED_PIN, blueValue/2);
}

static void outByte(byte b) {
  Serial.write(b);
  softSerial2.write(b);
  softSerial.write(b);
}

static void handleAllMessages(midi::MidiMessage msg) {
  if (msg.type == midi::Start) {
    outByte(msg.type);
    instance->ResetTimer();
    return;
  }
  if (msg.type == midi::Stop) {
    outByte(msg.type);
    return;
  }

  if (msg.type < 0xf0) {
    randomize();
    Serial.write(msg.type + msg.channel-1);
    Serial.write(msg.data1);
    softSerial2.write(msg.type + msg.channel-1);
    softSerial2.write(msg.data1);
    softSerial.write(msg.type + msg.channel-1);
    softSerial.write(msg.data1);
    if (msg.type <= 0xf0 || msg.data2) {
      Serial.write(msg.data2);
      softSerial2.write(msg.data2);
      softSerial.write(msg.data2);
    }
  }
}

static void handleSysex(byte *array, unsigned size) {
  randomize();
  if (size < 5) return;
  if (!(array[1] == 0x7d && array[2] == 0x2a && array[3] == 0x4d)) return;

  for (int i=4; i<size-1; i++) {
    int command = array[i];
    if (command >= 0x40) {
      return;
    }

    switch (command) {
    case 0: {
      byte response[] = {0xf0, 0x7d, 0x2a, 0x4d, 
        0x40, 0x01,            // RESPONSE 0x40, version 1
        0x01, 0x02,            // 1 inport, 2 outports
        byte(int(bpm) >> 7),   // current speet msb
        byte(int(bpm) & 0x7f), // current speed lsb
        0xf7};
      midiIN.sendSysEx(sizeof(response), response, true);
      midiOUT.sendSysEx(sizeof(response), response, true);
      midiOUT2.sendSysEx(sizeof(response), response, true);
      return;
    }
    case 1: {
      i += 7;
      break;
    }
    case 2: {
      int newbpm = (array[i+1] << 7) + array[i+2];
      instance->SetBPM(newbpm);
    }
    case 3: {
      instance->Start();
      break;
    }
    case 4: {
      instance->Stop(false);
      break;
    }
    default: {
      break;
    }
    }
  }
}

//

MidiController::MidiController() {}

void MidiController::begin() {
  instance = this;

  midiOUT.begin();
  midiOUT.turnThruOff();

  midiOUT2.begin();
  midiOUT2.turnThruOff();

  midiIN.begin(MIDI_CHANNEL_OMNI);
  midiIN.turnThruOff();
  midiIN.setHandleSystemExclusive(handleSysex);
  midiIN.setHandleAllMessages(handleAllMessages);

  this->setupTimer();
}

void MidiController::loop() {
  unsigned long now = micros();
  if (now >= _next) {
    _next += _sleep;

    Clock();
    if (_timerCallback) {
      _timerCallback();
    }
  }

  midiOUT2.read();
  midiOUT.read();
  midiIN.read();
}

void MidiController::Start() {
  midiIN.sendRealTime(midi::Start);
  midiOUT.sendRealTime(midi::Start);
  midiOUT2.sendRealTime(midi::Start);
  this->ResetTimer();
}

void MidiController::Stop(bool hard) {
  midiIN.sendRealTime(midi::Stop);
  midiOUT.sendRealTime(midi::Stop);
  midiOUT2.sendRealTime(midi::Stop);
//  if (hard) {
//    Timer1.stop();
//  }
}

void MidiController::Clock() {
  midiIN.sendRealTime(midi::Clock);
  midiOUT.sendRealTime(midi::Clock);
  midiOUT2.sendRealTime(midi::Clock);
}

void MidiController::TimerCallback(void (*fn)(void)) {
  _timerCallback = fn;
}

void MidiController::SetBPM(float newbpm) {
  bpm = newbpm;
  setupTimer();
}

//

void MidiController::ResetTimer() {
    this->setupTimer();
    _next = micros() - 1000;
}

void MidiController::setupTimer() {
  _sleep = 60000000.0/bpm/24.0;
}


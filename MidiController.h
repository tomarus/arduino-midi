#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include "Arduino.h"

class MidiController
{
  public:
    MidiController();
    void begin();
    void loop();
    void Start();
    void Stop(bool hard);
    void Clock();
    void TimerCallback(void (*)());
    void SetBPM(float);
    void ResetTimer();
  private:
    void setupTimer();

    void (*_timerCallback)();
    unsigned long _sleep;
    unsigned long _next;
};

#endif


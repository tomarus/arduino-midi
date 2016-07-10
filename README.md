Arduino MIDI controller
=======================

This project is a hardware version of https://github.com/tomarus/midiseq

It's in really early stages of experimentation. It uses an Arduino Uno with Serial mapped to the default MIDI in/out and there are an additional 2 SoftSerials for output and thru/routing only.

Most configuration and interactivity with http://midi.tomarus.io is done using Sysex.

It needs the custom MIDI library from https://github.com/tomarus/arduino_midi_library

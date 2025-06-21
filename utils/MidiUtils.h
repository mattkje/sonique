//
// Created by Matti Kjellstadli on 21/06/2025.
//

#pragma once
#include <string>
#include <vector>
#include <fluidsynth.h>

// Handles MIDI events for the synth
int midi_event_handler(void *data, fluid_midi_event_t *event);

// Gets the initial tempo (BPM) from a MIDI file
int GetMidiInitialTempoBPM(const std::string &midiPath);

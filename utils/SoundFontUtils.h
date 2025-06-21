//
// Created by Matti Kjellstadli on 21/06/2025.
//

#pragma once
#include <string>
#include <vector>
#include <fluidsynth.h>

// Ensures the SoundFont directory exists, creates if missing
bool EnsureSoundFontDir(const std::string& dirPath);

// Returns a list of .sf2 SoundFont files in the directory
std::vector<std::string> ScanSoundFonts(const std::string& dirPath);

// Loads a SoundFont into the synth, returns the SoundFont ID
int LoadSoundFont(fluid_synth_t* synth, const std::string& sf2Path, int resetPresets = 1);
#pragma once
#include <string>
#include <vector>
#include <fluidsynth.h>
#include "raylib.h"

constexpr int NUM_WHITE_KEYS = 52;
constexpr int NUM_BLACK_KEYS = 36;
constexpr int NUM_TOTAL_KEYS = 88;
constexpr float KEYBOARD_ASPECT = 10.0f; // width:height ratio

const std::array<std::string, 12> noteNames = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

const std::array<std::string, 7> whiteKeyNames = {"A", "B", "C", "D", "E", "F", "G"};
const std::array<int, 7> blackKeyPattern = {1, 0, 1, 1, 1, 0, 1}; // A#, C#, D#, F#, G#
const std::array<std::string, 5> blackKeyNames = {"A#", "C#", "D#", "F#", "G#"};

struct PianoKey {
    Rectangle rect;
    bool isBlack;
    int midiNumber;
    std::string label;
};

std::vector<PianoKey> GeneratePianoKeys(int windowWidth, int keyboardY, int keyboardHeight);

void DrawPianoKeys(
    const std::vector<PianoKey>& keys,
    std::vector<bool>& keyWasPressed,
    fluid_synth_t* synth,
    Font font,
    bool showKeyLabels,
    Texture2D whiteKey,
    Texture2D whiteKeyPressed,
    Texture2D blackKey,
    Texture2D blackKeyPressed,
    const std::vector<std::vector<bool>>& midiKeyStates
);

void ResetKeyPressedStates(std::vector<bool>& keyWasPressed);
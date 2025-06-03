#include "raylib.h"
#include <array>
#include <string>
#include <vector>
#include <fluidsynth.h>

constexpr bool showKeyLabels = false;

constexpr int NUM_WHITE_KEYS = 52;
constexpr int NUM_BLACK_KEYS = 36;
constexpr int NUM_TOTAL_KEYS = 88;
constexpr float KEYBOARD_ASPECT = 10.0f; // width:height ratio

const std::array<std::string, 7> whiteKeyNames = {"C", "D", "E", "F", "G", "A", "B"};
const std::array<int, 7> blackKeyPattern = {1, 1, 0, 1, 1, 1, 0};
const std::array<std::string, 5> blackKeyNames = {"C#", "D#", "F#", "G#", "A#"};

struct PianoKey {
    Rectangle rect;
    bool isBlack;
    int midiNumber;
    std::string label;
};

// In GeneratePianoKeys, assign correct MIDI numbers to each key
std::vector<PianoKey> GeneratePianoKeys(int windowWidth, int keyboardY, int keyboardHeight) {
    std::vector<PianoKey> keys;
    float whiteKeyWidth = static_cast<float>(windowWidth) / NUM_WHITE_KEYS;
    float blackKeyWidth = whiteKeyWidth * 0.6f;
    float blackKeyHeight = keyboardHeight * 0.6f;

    int midi = 21; // Start at A0
    int whiteKeyIndex = 0;

    // Generate white keys
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
        int noteInOctave = whiteKeyIndex % 7;
        int octave = (midi / 12) - 1;
        std::string label = whiteKeyNames[noteInOctave] + std::to_string(octave);
        keys.push_back({
            Rectangle{
                whiteKeyIndex * whiteKeyWidth, static_cast<float>(keyboardY), whiteKeyWidth,
                static_cast<float>(keyboardHeight)
            },
            false,
            midi,
            label
        });
        ++whiteKeyIndex;
        // Skip black keys in MIDI numbering
        do {
            ++midi;
        } while ((midi % 12 == 1) || (midi % 12 == 3) || (midi % 12 == 6) || (midi % 12 == 8) || (midi % 12 == 10));
    }

    // Generate black keys
    midi = 22; // First black key is A#0
    whiteKeyIndex = 0;
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
        int noteInOctave = whiteKeyIndex % 7;
        int octave = (midi / 12) - 1;
        if (blackKeyPattern[noteInOctave]) {
            std::string label = blackKeyNames[(noteInOctave < 2) ? noteInOctave : noteInOctave - 1] + std::to_string(octave);
            float x = (whiteKeyIndex + 1) * whiteKeyWidth - blackKeyWidth / 2;
            keys.push_back({
                Rectangle{x, static_cast<float>(keyboardY), blackKeyWidth, blackKeyHeight},
                true,
                midi,
                label
            });
            // Advance to next black key MIDI number
            do {
                ++midi;
            } while ((midi % 12 != 1) && (midi % 12 != 3) && (midi % 12 != 6) && (midi % 12 != 8) && (midi % 12 != 10));
        }
        ++whiteKeyIndex;
        // Advance midi to next white key
        do {
            ++midi;
        } while ((midi % 12 == 1) || (midi % 12 == 3) || (midi % 12 == 6) || (midi % 12 == 8) || (midi % 12 == 10));
    }
    return keys;
}

int main() {
    fluid_settings_t* settings = new_fluid_settings();
    fluid_synth_t* synth = new_fluid_synth(settings);
    fluid_audio_driver_t* adriver = new_fluid_audio_driver(settings, synth);

    // Load your SF2 file (replace with your actual path)
    int sfid_piano = fluid_synth_sfload(synth, "SoundFonts/piano.sf2", 1);
    int sfid_drums = fluid_synth_sfload(synth, "SoundFonts/drums.sf2", 1);
    // set the default soundfont to piano
    fluid_synth_program_select(synth, 0, sfid_piano, 0, 0);
    fluid_synth_program_select(synth, 9, sfid_drums, 128, 0);



    const int initialWidth = 1220;
    const int initialHeight = 800;
    InitWindow(initialWidth, initialHeight, "Sonique - Full Piano");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Font font = LoadFont("Neutraface.otf");
    Texture2D background = LoadTexture("assets/background.png");
    Texture2D whiteKey = LoadTexture("assets/whiteKey.png");
    Texture2D whiteKeyPressed = LoadTexture("assets/whiteKeyPressed.png");
    Texture2D blackKey = LoadTexture("assets/blackKey.png");
    Texture2D blackKeyPressed = LoadTexture("assets/blackKeyPressed.png");

    while (!WindowShouldClose()) {
        int windowWidth = GetScreenWidth();
        int windowHeight = GetScreenHeight();

        // Maintain fixed aspect ratio for keyboard
        int keyboardHeight = static_cast<int>(windowWidth / KEYBOARD_ASPECT);
        if (keyboardHeight > windowHeight) keyboardHeight = windowHeight;
        int keyboardY = windowHeight - keyboardHeight;

        auto keys = GeneratePianoKeys(windowWidth, keyboardY, keyboardHeight);

        BeginDrawing();
        DrawTexturePro(
            background,
            Rectangle{0, 0, (float) background.width, (float) background.height},
            Rectangle{0, 0, (float) windowWidth, (float) windowHeight},
            Vector2{0, 0},
            0.0f,
            WHITE
        );

        static std::vector<bool> keyWasPressed(keys.size(), false);

        // Draw white keys
        int midi = 24; // Start at A0
        for (size_t i = 0, whiteIndex = 0; i < keys.size(); ++i) {
            auto &key = keys[i];
            if (!key.isBlack) {
                int noteInOctave = whiteIndex % 7;
                int octave = whiteIndex / 7 + 1; // A0 is midi 21, so first octave is 1
                key.label = whiteKeyNames[noteInOctave] + std::to_string(octave);
                key.midiNumber = midi;

                // In the drawing loop, use key.midiNumber for note on/off
                bool pressed = CheckCollisionPointRec(GetMousePosition(), key.rect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
                if (pressed && !keyWasPressed[i]) {
                    fluid_synth_noteon(synth, 0, key.midiNumber, 100);
                }
                if (!pressed && keyWasPressed[i]) {
                    fluid_synth_noteoff(synth, 0, key.midiNumber);
                }
                keyWasPressed[i] = pressed;
                keyWasPressed[i] = pressed;
                Texture2D tex = pressed ? whiteKeyPressed : whiteKey;
                if (tex.id != 0) {
                    DrawTexturePro(
                        tex,
                        Rectangle{0, 0, (float) tex.width, (float) tex.height},
                        key.rect,
                        Vector2{0, 0},
                        0.0f,
                        WHITE
                    );
                } else {
                    DrawRectangleRec(key.rect, pressed ? LIGHTGRAY : RAYWHITE);
                }
                DrawRectangleLinesEx(key.rect, 1, GRAY);
                if (showKeyLabels) {
                    Vector2 textSize = MeasureTextEx(font, key.label.c_str(), 12, 1);
                    DrawTextEx(font, key.label.c_str(),
                               {key.rect.x + key.rect.width / 2 - textSize.x / 2, key.rect.y + key.rect.height - 18},
                               12, 1, DARKGRAY);
                }
                // Skip black keys in MIDI numbering
                do {
                    ++midi;
                } while ((midi % 12 == 1) || (midi % 12 == 3) || (midi % 12 == 6) || (midi % 12 == 8) || (midi % 12 == 10));
                ++whiteIndex;
            }
        }

        // Draw black keys
        int blackKeyIndex = 0;
        for (size_t i = 0; i < keys.size(); ++i) {
            const auto& key = keys[i];
            if (!key.isBlack) {
                int noteInOctave = blackKeyIndex % 7;
                // Only place a black key if the pattern says so
                if (blackKeyPattern[noteInOctave]) {
                    // Find the position between this white key and the next
                    float whiteKeyWidth = key.rect.width;
                    float blackKeyWidth = whiteKeyWidth * 0.6f;
                    float blackKeyHeight = key.rect.height * 0.6f;
                    float x = key.rect.x + whiteKeyWidth - blackKeyWidth / 2;
                    Rectangle blackRect = {x, key.rect.y, blackKeyWidth, blackKeyHeight};

                    // In the drawing loop, use key.midiNumber for note on/off
                    bool pressed = CheckCollisionPointRec(GetMousePosition(), key.rect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
                    if (pressed && !keyWasPressed[i]) {
                        fluid_synth_noteon(synth, 0, key.midiNumber, 100);
                    }
                    if (!pressed && keyWasPressed[i]) {
                        fluid_synth_noteoff(synth, 0, key.midiNumber);
                    }
                    keyWasPressed[i] = pressed;
                    keyWasPressed[i + NUM_WHITE_KEYS] = pressed;
                    Texture2D tex = pressed ? blackKeyPressed : blackKey;
                    if (tex.id != 0) {
                        DrawTexturePro(
                            tex,
                            Rectangle{0, 0, (float) tex.width, (float) tex.height},
                            blackRect,
                            Vector2{0, 0},
                            0.0f,
                            WHITE
                        );
                    } else {
                        DrawRectangleRec(blackRect, pressed ? GRAY : BLACK);
                    }
                    if (showKeyLabels) {
                        std::string label = blackKeyNames[(noteInOctave < 2) ? noteInOctave : noteInOctave - 1] +
                                            std::to_string((key.midiNumber / 12) - 1);
                        Vector2 textSize = MeasureTextEx(font, label.c_str(), 10, 1);
                        DrawTextEx(font, label.c_str(),
                                   {blackRect.x + blackRect.width / 2 - textSize.x / 2, blackRect.y + blackRect.height - 16},
                                   10, 1, RAYWHITE);
                    }
                    // Advance to next black key MIDI number
                    do {
                        ++midi;
                    } while ((midi % 12 != 1) && (midi % 12 != 3) && (midi % 12 != 6) && (midi % 12 != 8) && (midi % 12 != 10));
                }
                ++blackKeyIndex;
                // Advance midi to next white key
                do {
                    ++midi;
                } while ((midi % 12 == 1) || (midi % 12 == 3) || (midi % 12 == 6) || (midi % 12 == 8) || (midi % 12 == 10));
            }
        }

        // Draw thin red line on top of the keyboard
       DrawLineEx({0, (float)(keyboardY - 1)}, {(float)windowWidth, (float)(keyboardY - 1)}, 3.0f, RED);

        DrawTextEx(font, "Sonique", {10, 10}, 20, 2, WHITE);

        EndDrawing();
    }

    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    UnloadTexture(whiteKey);
    UnloadTexture(whiteKeyPressed);
    UnloadTexture(blackKey);
    UnloadTexture(blackKeyPressed);
    UnloadFont(font);
    UnloadTexture(background);
    CloseWindow();
    return 0;
}

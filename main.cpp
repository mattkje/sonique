#include "raylib.h"
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <fluidsynth.h>
#include <CoreFoundation/CoreFoundation.h>

constexpr bool showKeyLabels = false;

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

std::string GetResourcePath(const std::string &filename) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *) path, PATH_MAX)) {
        std::string fullPath = std::string(path) + "/" + filename;
        CFRelease(resourcesURL);
        return fullPath;
    }
    CFRelease(resourcesURL);
    return filename; // fallback
}

std::vector<PianoKey> GeneratePianoKeys(int windowWidth, int keyboardY, int keyboardHeight) {
    std::vector<PianoKey> keys;
    float whiteKeyWidth = static_cast<float>(windowWidth) / NUM_WHITE_KEYS;
    float blackKeyWidth = whiteKeyWidth * 0.6f;
    float blackKeyHeight = keyboardHeight * 0.6f;

    // Generate all keys from MIDI 21 (A0) to 108 (C8)
    int whiteIndex = 0;
    for (int midi = 21; midi <= 108; ++midi) {
        int noteInOctave = midi % 12;
        int octave = (midi / 12) - 1;
        std::string label = noteNames[noteInOctave] + std::to_string(octave);

        // White keys: C, D, E, F, G, A, B
        if (noteNames[noteInOctave].length() == 1) {
            keys.push_back({
                Rectangle{
                    whiteIndex * whiteKeyWidth, static_cast<float>(keyboardY), whiteKeyWidth,
                    static_cast<float>(keyboardHeight)
                },
                false,
                midi,
                label
            });
            ++whiteIndex;
        }
    }

    // Black keys
    whiteIndex = 0;
    for (int midi = 22; midi <= 108; ++midi) {
        int noteInOctave = midi % 12;
        int octave = (midi / 12) - 1;
        std::string label = noteNames[noteInOctave] + std::to_string(octave);

        // Black keys: C#, D#, F#, G#, A#
        if (noteNames[noteInOctave].length() == 2) {
            // Position black key between the two adjacent white keys
            float x = (whiteIndex + 1) * whiteKeyWidth - blackKeyWidth / 2;
            keys.push_back({
                Rectangle{x, static_cast<float>(keyboardY), blackKeyWidth, blackKeyHeight},
                true,
                midi,
                label
            });
        }
        // Only increment whiteIndex after a white key
        if (noteNames[noteInOctave].length() == 1) {
            ++whiteIndex;
        }
    }
    return keys;
}


// At global scope
std::vector midiKeyStates(16, std::vector(NUM_TOTAL_KEYS, false));

int midi_event_handler(void *data, fluid_midi_event_t *event) {
    fluid_synth_t *synth = static_cast<fluid_synth_t *>(data);
    int type = fluid_midi_event_get_type(event);
    int key = fluid_midi_event_get_key(event);
    int channel = fluid_midi_event_get_channel(event);
    if (key >= 21 && key <= 108 && channel != 9) { // Exclude drums
        int idx = key - 21;
        if (type == FLUID_SEQ_NOTEON && fluid_midi_event_get_velocity(event) > 0) {
            midiKeyStates[channel][idx] = true;
        } else if (type == FLUID_SEQ_NOTEOFF || (type == FLUID_SEQ_NOTEON && fluid_midi_event_get_velocity(event) == 0)) {
            midiKeyStates[channel][idx] = false;
        }
    }
    return fluid_synth_handle_midi_event(synth, event);
}

// When setting the callback:

void DrawPianoKeys(const std::vector<PianoKey> &keys, std::vector<bool> &keyWasPressed, fluid_synth_t *synth, Font font,
                   bool showKeyLabels, Texture2D whiteKey, Texture2D whiteKeyPressed, Texture2D blackKey,
                   Texture2D blackKeyPressed) {
    // First, check if any black key is pressed at the mouse position

    Vector2 mousePos = GetMousePosition();
    bool blackKeyPressedAtMouse = false;
    int blackKeyIndex = -1;
    for (size_t i = 0; i < keys.size(); ++i) {
        const auto &key = keys[i];
        if (key.isBlack && CheckCollisionPointRec(mousePos, key.rect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            blackKeyPressedAtMouse = true;
            blackKeyIndex = i;
            break;
        }
    }

    // Draw white keys first (so black keys are on top)
    for (size_t i = 0; i < keys.size(); ++i) {
        const auto &key = keys[i];
        if (!key.isBlack) {
            // Only allow white key press if no black key is pressed at this mouse position
            bool midiPressed = false;

            bool pressed = midiPressed || !blackKeyPressedAtMouse &&
                           CheckCollisionPointRec(mousePos, key.rect) &&
                           IsMouseButtonDown(MOUSE_LEFT_BUTTON);

            if (pressed && !keyWasPressed[i]) {
                fluid_synth_noteon(synth, 0, key.midiNumber, 100);
            }
            if (!pressed && keyWasPressed[i]) {
                fluid_synth_noteoff(synth, 0, key.midiNumber);
            }
            keyWasPressed[i] = pressed;
            for (int ch = 0; ch < 16; ++ch) {
                if (midiKeyStates[ch][key.midiNumber - 21]) {
                    midiPressed = true;
                    break;
                }
            }
            Texture2D tex = pressed || midiPressed ? whiteKeyPressed : whiteKey;
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
        }
    }
    // Draw black keys on top (unchanged)
    for (size_t i = 0; i < keys.size(); ++i) {
        const auto &key = keys[i];
        if (key.isBlack) {
            bool midiPressed = false;

            bool pressed = midiPressed || CheckCollisionPointRec(mousePos, key.rect) && IsMouseButtonDown(
                               MOUSE_LEFT_BUTTON);
            if (pressed && !keyWasPressed[i]) {
                fluid_synth_noteon(synth, 0, key.midiNumber, 100);
            }
            if (!pressed && keyWasPressed[i]) {
                fluid_synth_noteoff(synth, 0, key.midiNumber);
            }
            keyWasPressed[i] = pressed;
            for (int ch = 0; ch < 16; ++ch) {
                if (midiKeyStates[ch][key.midiNumber - 21]) {
                    midiPressed = true;
                    break;
                }
            }
            Texture2D tex = pressed || midiPressed ? blackKeyPressed : blackKey;
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
                DrawRectangleRec(key.rect, pressed ? GRAY : BLACK);
            }
            if (showKeyLabels) {
                Vector2 textSize = MeasureTextEx(font, key.label.c_str(), 10, 1);
                DrawTextEx(font, key.label.c_str(),
                           {key.rect.x + key.rect.width / 2 - textSize.x / 2, key.rect.y + key.rect.height - 16},
                           10, 1, RAYWHITE);
            }
        }
    }
}

int main() {
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);
    fluid_audio_driver_t *adriver = new_fluid_audio_driver(settings, synth);

    // Load your piano SoundFont
    int general = fluid_synth_sfload(synth, GetResourcePath("SoundFonts/general3.sf2").c_str(), 1);


    // All other channels: general SoundFont
    for (int i = 1; i < 16; ++i) {
        if (i == 9) continue; // already set for drums
        fluid_synth_program_select(synth, i, general, 0, 0);
    }

    fluid_player_t *player = new_fluid_player(synth);
    fluid_player_add(player, GetResourcePath("assets/cstnd.mid").c_str());
    fluid_player_set_playback_callback(player, midi_event_handler, synth);
    fluid_player_play(player);

    const int initialWidth = 1220;
    const int initialHeight = 800;
    InitWindow(initialWidth, initialHeight, "Sonique - Full Piano");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Font font = LoadFont(GetResourcePath("Neutraface.otf").c_str());
    Texture2D background = LoadTexture(GetResourcePath("assets/background.png").c_str());
    Texture2D whiteKey = LoadTexture(GetResourcePath("assets/whiteKey.png").c_str());
    Texture2D whiteKeyPressed = LoadTexture(GetResourcePath("assets/whiteKeyPressed.png").c_str());
    Texture2D blackKey = LoadTexture(GetResourcePath("assets/blackKey.png").c_str());
    Texture2D blackKeyPressed = LoadTexture(GetResourcePath("assets/blackKeyPressed.png").c_str());

    std::vector<PianoKey> keys;
    std::vector<bool> keyWasPressed;

    while (!WindowShouldClose()) {
        int windowWidth = GetScreenWidth();
        int windowHeight = GetScreenHeight();

        int keyboardHeight = static_cast<int>(windowWidth / KEYBOARD_ASPECT);
        if (keyboardHeight > windowHeight) keyboardHeight = windowHeight;
        int keyboardY = windowHeight - keyboardHeight;

        keys = GeneratePianoKeys(windowWidth, keyboardY, keyboardHeight);
        if (keyWasPressed.size() != keys.size()) keyWasPressed.assign(keys.size(), false);

        BeginDrawing();
        DrawTexturePro(
            background,
            Rectangle{0, 0, (float) background.width, (float) background.height},
            Rectangle{0, 0, (float) windowWidth, (float) windowHeight},
            Vector2{0, 0},
            0.0f,
            WHITE
        );

        DrawPianoKeys(keys, keyWasPressed, synth, font, showKeyLabels, whiteKey, whiteKeyPressed, blackKey,
                      blackKeyPressed);

        DrawLineEx({0, (float) (keyboardY - 1)}, {(float) windowWidth, (float) (keyboardY - 1)}, 3.0f, RED);
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

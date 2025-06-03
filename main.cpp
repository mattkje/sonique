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

std::vector<PianoKey> GeneratePianoKeys(int windowWidth, int keyboardY, int keyboardHeight) {
    std::vector<PianoKey> keys;
    float whiteKeyWidth = static_cast<float>(windowWidth) / NUM_WHITE_KEYS;
    float blackKeyWidth = whiteKeyWidth * 0.6f;
    float blackKeyHeight = keyboardHeight * 0.6f;

    int whiteKeyIndex = 0;
    int midi = 21; // A0 MIDI number

    // Generate white keys
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
        int octave = (whiteKeyIndex + 9) / 7;
        int noteInOctave = (whiteKeyIndex + 2) % 7;
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
        ++midi;
    }

    // Generate black keys
    whiteKeyIndex = 0;
    midi = 22;
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
        int noteInOctave = (whiteKeyIndex + 2) % 7;
        int octave = (whiteKeyIndex + 9) / 7;
        if (blackKeyPattern[noteInOctave]) {
            std::string label = blackKeyNames[(noteInOctave < 2) ? noteInOctave : noteInOctave - 1] +
                                std::to_string(octave);
            float x = (whiteKeyIndex + 1) * whiteKeyWidth - blackKeyWidth / 2;
            keys.push_back({
                Rectangle{x, static_cast<float>(keyboardY), blackKeyWidth, blackKeyHeight},
                true,
                midi,
                label
            });
            ++midi;
        }
        ++whiteKeyIndex;
        ++midi;
    }
    return keys;
}

int main() {
    fluid_settings_t* settings = new_fluid_settings();
    fluid_synth_t* synth = new_fluid_synth(settings);
    fluid_audio_driver_t* adriver = new_fluid_audio_driver(settings, synth);

    // Load your SF2 file (replace with your actual path)
    int sfid = fluid_synth_sfload(synth, "soundfont.sf2", 1);
    if (sfid == FLUID_FAILED) {
        printf("Failed to load SF2 file\n");
        // Handle error or exit
    }

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

        // Draw white keys
        for (const auto &key: keys) {
            if (!key.isBlack) {
                bool pressed = CheckCollisionPointRec(GetMousePosition(), key.rect) && IsMouseButtonDown(
                                   MOUSE_LEFT_BUTTON);
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
            }
        }

        // Draw black keys
        for (const auto &key: keys) {
            if (key.isBlack) {
                bool pressed = CheckCollisionPointRec(GetMousePosition(), key.rect) && IsMouseButtonDown(
                                   MOUSE_LEFT_BUTTON);
                Texture2D tex = pressed ? blackKeyPressed : blackKey;
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

#include "raylib.h"
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <fluidsynth.h>
#include <CoreFoundation/CoreFoundation.h>

constexpr bool showKeyLabels = true;

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
    float blackKeyHeight = keyboardHeight * 0.65f;

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
                    whiteIndex * whiteKeyWidth, static_cast<float>(keyboardY + 2), whiteKeyWidth,
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
    if (key >= 21 && key <= 108 && channel != 9) {
        // Exclude drums
        int idx = key - 21;
        if (type == FLUID_SEQ_NOTEON && fluid_midi_event_get_velocity(event) > 0) {
            midiKeyStates[channel][idx] = true;
        } else if (type == FLUID_SEQ_NOTEOFF || (type == FLUID_SEQ_NOTEON && fluid_midi_event_get_velocity(event) ==
                                                 0)) {
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

            if (showKeyLabels && key.label[0] == 'C') {
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

            bool pressed = CheckCollisionPointRec(mousePos, key.rect) && IsMouseButtonDown(
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
            // Draw a border (gap) behind the black key: thin sides/top, thick bottom
            float sideBorder = 1.5f;
            float topBorder = 1.0f;
            float bottomBorder = 2.0f;
            Rectangle borderRect = {
                key.rect.x - sideBorder - 1,
                key.rect.y - topBorder + 4,
                key.rect.width + 2 * sideBorder,
                key.rect.height + topBorder + bottomBorder
            };
            DrawRectangleRec(borderRect, BLACK);

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
            if (false) {
                Vector2 textSize = MeasureTextEx(font, key.label.c_str(), 10, 1);
                DrawTextEx(font, key.label.c_str(),
                           {key.rect.x + key.rect.width / 2 - textSize.x / 2, key.rect.y + key.rect.height - 16},
                           10, 1, RAYWHITE);
            }
        }
    }
}

int main() {
    bool isPlaying = false;
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
    fluid_player_add(player, GetResourcePath("assets/testSong.mid").c_str());
    fluid_player_set_playback_callback(player, midi_event_handler, synth);

    const int initialWidth = 1220;
    const int initialHeight = 800;
    InitWindow(initialWidth, initialHeight, "Sonique");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Font font = LoadFont(GetResourcePath("Neutraface.otf").c_str());
    Texture2D background = LoadTexture(GetResourcePath("assets/background.png").c_str());
    Texture2D whiteKey = LoadTexture(GetResourcePath("assets/whiteKey.png").c_str());
    Texture2D whiteKeyPressed = LoadTexture(GetResourcePath("assets/whiteKeyPressed.png").c_str());
    Texture2D blackKey = LoadTexture(GetResourcePath("assets/black-key-raised.png").c_str());
    Texture2D blackKeyPressed = LoadTexture(GetResourcePath("assets/black-key-pressed.png").c_str());
    Texture2D playIcon = LoadTexture(GetResourcePath("assets/play.png").c_str());
    Texture2D pauseIcon = LoadTexture(GetResourcePath("assets/pause.png").c_str());

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


        // Draw toolbar background
        // Define button properties
        float playBtnWidth = 80;
        float playBtnHeight = 30;
        float playBtnX = (windowWidth - playBtnWidth) / 2;
        float playBtnY = 10;
        Rectangle playBtn = {playBtnX, playBtnY, playBtnWidth, playBtnHeight};
        Rectangle metroBtn = {200, 10, 80, 30}; // Adjust or reposition as needed

        // Draw toolbar background
        DrawRectangle(0, 0, windowWidth, 50, BLACK);
        DrawLineEx({0, 50}, {(float) windowWidth, 50}, 1.0f, DARKGRAY);

        // Progress Bar
        float progressBarWidth = windowWidth;
        float progressBarHeight = 30;
        float progressBarX = 0;
        float progressBarY = 51;
        DrawRectangleRec({progressBarX, progressBarY, progressBarWidth, progressBarHeight}, GRAY);
        DrawLineEx({0, 82}, {(float) windowWidth, 81}, 1.0f, DARKGRAY);
        // Calculate progress
        long total_ticks = fluid_player_get_total_ticks(player);
        long current_tick = fluid_player_get_current_tick(player);

        double progress = (double) current_tick / (double) total_ticks;

        // Draw progress
        DrawRectangleRec(
            {progressBarX, progressBarY, (float) (progress * progressBarWidth), progressBarHeight},
            SKYBLUE
        );


        // Draw buttons
        DrawRectangleRec(metroBtn, DARKGRAY);
        Texture2D icon = isPlaying ? pauseIcon : playIcon;
        float iconSize = 24;
        DrawTexturePro(
            icon,
            Rectangle{0, 0, (float) icon.width, (float) icon.height},
            Rectangle{
                playBtn.x + (playBtn.width - iconSize) / 2, playBtn.y + (playBtn.height - iconSize) / 2, iconSize,
                iconSize
            },
            Vector2{0, 0},
            0.0f,
            WHITE
        );
        DrawTextEx(font, "Metronome", {metroBtn.x + 15, metroBtn.y + 7}, 16, 1, WHITE);

        // Handle button clicks
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, playBtn)) {
                if (isPlaying) {
                    fluid_player_stop(player);
                    isPlaying = false;
                } else {
                    fluid_player_play(player);
                    isPlaying = true;
                }
            }
        }
        DrawLineEx({0, (float) (keyboardY + 1)}, {(float) windowWidth, (float) (keyboardY + 1)}, 3.0f, RED);
        DrawPianoKeys(keys, keyWasPressed, synth, font, showKeyLabels, whiteKey, whiteKeyPressed, blackKey,
                      blackKeyPressed);


        //DrawTextEx(font, "Sonique", {10, 10}, 20, 0, WHITE);

        EndDrawing();
    }

    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    UnloadTexture(playIcon);
    UnloadTexture(pauseIcon);
    UnloadTexture(whiteKey);
    UnloadTexture(whiteKeyPressed);
    UnloadTexture(blackKey);
    UnloadTexture(blackKeyPressed);
    UnloadFont(font);
    UnloadTexture(background);
    CloseWindow();
    return 0;
}

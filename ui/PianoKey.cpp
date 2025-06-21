//
// Created by Matti Kjellstadli on 21/06/2025.
//

#include "PianoKey.h"


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

void DrawPianoKeys(
    const std::vector<PianoKey> &keys,
    std::vector<bool> &keyWasPressed,
    fluid_synth_t *synth,
    Font font,
    bool showKeyLabels,
    Texture2D whiteKey,
    Texture2D whiteKeyPressed,
    Texture2D blackKey,
    Texture2D blackKeyPressed,
    const std::vector<std::vector<bool> > &midiKeyStates // <-- add this
) {
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

// main.cpp
#include "raylib.h"
#include <array>
#include <string>

constexpr int NUM_WHITE_KEYS = 7;
constexpr int NUM_BLACK_KEYS = 5;
constexpr int WHITE_KEY_WIDTH = 60;
constexpr int WHITE_KEY_HEIGHT = 300;
constexpr int BLACK_KEY_WIDTH = 40;
constexpr int BLACK_KEY_HEIGHT = 180;
constexpr int KEYBOARD_X = 100;
constexpr int KEYBOARD_Y = 150;

const std::array<std::string, NUM_WHITE_KEYS> whiteKeyLabels = {"C", "D", "E", "F", "G", "A", "B"};
const std::array<int, NUM_BLACK_KEYS> blackKeyOffsets = {0, 1, 3, 4, 5}; // C#, D#, F#, G#, A#
const std::array<std::string, NUM_BLACK_KEYS> blackKeyLabels = {"C#", "D#", "F#", "G#", "A#"};

int main() {
    InitWindow(800, 600, "Sonique");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw white keys
        for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
            Rectangle keyRect = {
                static_cast<float>(KEYBOARD_X + i * WHITE_KEY_WIDTH),
                static_cast<float>(KEYBOARD_Y),
                static_cast<float>(WHITE_KEY_WIDTH),
                static_cast<float>(WHITE_KEY_HEIGHT)
            };
            bool pressed = CheckCollisionPointRec(GetMousePosition(), keyRect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
            DrawRectangleRec(keyRect, pressed ? SKYBLUE : WHITE);
            DrawRectangleLinesEx(keyRect, 2, BLACK);
            DrawText(whiteKeyLabels[i].c_str(),
                     keyRect.x + WHITE_KEY_WIDTH/2 - 10,
                     keyRect.y + WHITE_KEY_HEIGHT - 30,
                     20, DARKGRAY);
        }

        // Draw black keys
        for (int i = 0; i < NUM_BLACK_KEYS; ++i) {
            int offset = blackKeyOffsets[i];
            // Skip E-F and B-C gaps
            if (offset == 2) continue;
            Rectangle keyRect = {
                static_cast<float>(KEYBOARD_X + (offset + 1) * WHITE_KEY_WIDTH - BLACK_KEY_WIDTH/2),
                static_cast<float>(KEYBOARD_Y),
                static_cast<float>(BLACK_KEY_WIDTH),
                static_cast<float>(BLACK_KEY_HEIGHT)
            };
            bool pressed = CheckCollisionPointRec(GetMousePosition(), keyRect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
            DrawRectangleRec(keyRect, pressed ? BLUE : BLACK);
            DrawText(blackKeyLabels[i].c_str(),
                     keyRect.x + BLACK_KEY_WIDTH/2 - 14,
                     keyRect.y + BLACK_KEY_HEIGHT - 30,
                     16, RAYWHITE);
        }

        DrawText("Click keys to highlight (expand for more features)", 180, 80, 20, GRAY);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
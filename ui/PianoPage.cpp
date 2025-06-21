#include "PianoPage.h"
#include "../utils/MidiUtils.h"
#include "../utils/FileUtils.h"
#include <iostream>

PianoPage::PianoPage(
    fluid_synth_t* synth,
    fluid_player_t* player,
    std::vector<std::string>& loadedMidiFiles,
    std::vector<SongInfo>& loadedSongInfos,
    std::vector<int>& midiBpms,
    std::vector<std::vector<bool>>& midiKeyStates
)
    : synth(synth),
      player(player),
      loadedMidiFiles(loadedMidiFiles),
      loadedSongInfos(loadedSongInfos),
      midiBpms(midiBpms),
      midiKeyStates(midiKeyStates)
{
    tempo = midiBpms.empty() ? 120 : midiBpms[0];
    currentSongIndex = 0;
    amountOfSongs = static_cast<int>(loadedMidiFiles.size());
    dropdownOpen = false;
    isPlaying = false;
    dropdownX = 20; dropdownY = 10; dropdownWidth = 260; dropdownHeight = 30;
    dropdownBox = {dropdownX, dropdownY, dropdownWidth, dropdownHeight};
    LoadResources();
}

PianoPage::~PianoPage() {
    UnloadResources();
}

void PianoPage::LoadResources() {
    font = LoadFont(GetResourcePath("Lexend.ttf").c_str());
    background = LoadTexture(GetResourcePath("assets/background.png").c_str());
    whiteKey = LoadTexture(GetResourcePath("assets/whiteKey.png").c_str());
    whiteKeyPressed = LoadTexture(GetResourcePath("assets/whiteKeyPressed.png").c_str());
    blackKey = LoadTexture(GetResourcePath("assets/black-key-raised.png").c_str());
    blackKeyPressed = LoadTexture(GetResourcePath("assets/black-key-pressed.png").c_str());
    playIcon = LoadTexture(GetResourcePath("assets/play.png").c_str());
    pauseIcon = LoadTexture(GetResourcePath("assets/pause.png").c_str());
}

void PianoPage::UnloadResources() {
    UnloadTexture(playIcon);
    UnloadTexture(pauseIcon);
    UnloadTexture(whiteKey);
    UnloadTexture(whiteKeyPressed);
    UnloadTexture(blackKey);
    UnloadTexture(blackKeyPressed);
    UnloadFont(font);
    UnloadTexture(background);
}

void PianoPage::Draw() {
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
        Rectangle{0, 0, (float)background.width, (float)background.height},
        Rectangle{0, 0, (float)windowWidth, (float)windowHeight},
        Vector2{0, 0},
        0.0f,
        WHITE
    );

    // Toolbar
    DrawRectangle(0, 0, windowWidth, 50, BLACK);
    DrawLineEx({0, 50}, {(float)windowWidth, 50}, 1.0f, DARKGRAY);

    // Tempo box
    DrawRectangleRec({dropdownX + 290, dropdownY, 90, 30}, DARKGRAY);
    std::string tempoStr = std::to_string(tempo);
    Vector2 tempoTextSize = MeasureTextEx(font, tempoStr.c_str(), 16, 1);
    DrawTextEx(font, tempoStr.c_str(), {dropdownX + 340 - tempoTextSize.x / 2, dropdownY + 6}, 16, 1, YELLOW);

    // Up/Down arrows
    Rectangle upBtn = {dropdownX + 352, dropdownY + 2, 24, 12};
    Rectangle downBtn = {dropdownX + 352, dropdownY + 16, 24, 12};
    DrawRectangleRec(upBtn, GRAY);
    DrawTriangle(
        Vector2{upBtn.x + 12, upBtn.y + 3},
        Vector2{upBtn.x + 5, upBtn.y + 10},
        Vector2{upBtn.x + 19, upBtn.y + 10},
        BLACK
    );
    DrawRectangleRec(downBtn, GRAY);
    DrawTriangle(
        Vector2{downBtn.x + 12, downBtn.y + 9},
        Vector2{downBtn.x + 5, downBtn.y + 2},
        Vector2{downBtn.x + 19, downBtn.y + 2},
        BLACK
    );

    // Progress bar
    float progressBarY = 51;
    DrawRectangleRec({0, progressBarY, (float)windowWidth, 30}, GRAY);
    DrawLineEx({0, 82}, {(float)windowWidth, 81}, 1.0f, DARKGRAY);
    long total_ticks = fluid_player_get_total_ticks(player);
    long current_tick = fluid_player_get_current_tick(player);
    double progress = (double)current_tick / (double)total_ticks;
    DrawRectangleRec({0, progressBarY, (float)(progress * windowWidth), 30}, SKYBLUE);

    // Dropdown
    DrawRectangleRec(dropdownBox, DARKGRAY);
    DrawTextEx(font, loadedSongInfos[currentSongIndex].displayName.c_str(),
               {dropdownX + 10, dropdownY + 6}, 16, 1, WHITE);
    DrawTriangle(
        Vector2{dropdownX + dropdownWidth - 20, dropdownY + 12},
        Vector2{dropdownX + dropdownWidth - 10, dropdownY + 12},
        Vector2{dropdownX + dropdownWidth - 15, dropdownY + 22},
        BLACK
    );
    if (dropdownOpen) {
        for (int i = 0; i < amountOfSongs; ++i) {
            Rectangle itemRect = {
                dropdownX, dropdownY + dropdownHeight + i * dropdownHeight, dropdownWidth, dropdownHeight
            };
            DrawRectangleRec(itemRect, (i == currentSongIndex) ? GRAY : DARKGRAY);
            std::string display = loadedSongInfos[i].displayName + " - " + loadedSongInfos[i].artist;
            DrawTextEx(font, display.c_str(), {dropdownX + 10, itemRect.y + 6}, 16, 1, WHITE);
        }
    }

    // Play/Pause button
    float playBtnWidth = 80, playBtnHeight = 30;
    float playBtnX = (windowWidth - playBtnWidth) / 2, playBtnY = 10;
    Rectangle playBtn = {playBtnX, playBtnY, playBtnWidth, playBtnHeight};
    Texture2D icon = isPlaying ? pauseIcon : playIcon;
    float iconSize = 24;
    DrawTexturePro(
        icon,
        Rectangle{0, 0, (float)icon.width, (float)icon.height},
        Rectangle{
            playBtn.x + (playBtn.width - iconSize) / 2, playBtn.y + (playBtn.height - iconSize) / 2, iconSize, iconSize
        },
        Vector2{0, 0},
        0.0f,
        WHITE
    );

    // Piano keys
    DrawLineEx({0, (float)(keyboardY + 1)}, {(float)windowWidth, (float)(keyboardY + 1)}, 3.0f, RED);
    DrawPianoKeys(keys, keyWasPressed, synth, font, true, whiteKey, whiteKeyPressed, blackKey, blackKeyPressed, midiKeyStates);

    EndDrawing();
}

void PianoPage::HandleInput() {
    Vector2 mouse = GetMousePosition();

    // Tempo up/down
    Rectangle upBtn = {dropdownX + 352, dropdownY + 2, 24, 12};
    Rectangle downBtn = {dropdownX + 352, dropdownY + 16, 24, 12};
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, upBtn)) {
            tempo += 1;
            fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, tempo);
        } else if (CheckCollisionPointRec(mouse, downBtn)) {
            if (tempo > 20) tempo -= 1;
            fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, tempo);
        }
    }

    // Dropdown
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, dropdownBox)) {
            dropdownOpen = !dropdownOpen;
        } else if (dropdownOpen) {
            for (int i = 0; i < amountOfSongs; ++i) {
                Rectangle itemRect = {
                    dropdownX, dropdownY + dropdownHeight + i * dropdownHeight, dropdownWidth, dropdownHeight
                };
                if (CheckCollisionPointRec(mouse, itemRect)) {
                    if (currentSongIndex != i) {
                        ReloadSong(i);
                    }
                    dropdownOpen = false;
                }
            }
            if (!CheckCollisionPointRec(mouse, {
                    dropdownX, dropdownY + dropdownHeight, dropdownWidth, dropdownHeight * amountOfSongs
                })) {
                dropdownOpen = false;
            }
        }
    }

    // Play/Pause
    float playBtnWidth = 80, playBtnHeight = 30;
    float playBtnX = (GetScreenWidth() - playBtnWidth) / 2, playBtnY = 10;
    Rectangle playBtn = {playBtnX, playBtnY, playBtnWidth, playBtnHeight};
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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
}

void PianoPage::Update() {
    // No per-frame logic needed for now, but could update progress, animations, etc.
}

void PianoPage::ReloadSong(int songIndex) {
    if (currentSongIndex == songIndex) return;
    currentSongIndex = songIndex;
    fluid_player_stop(player);
    player = new_fluid_player(synth);
    fluid_player_add(player, loadedMidiFiles[currentSongIndex].c_str());
    fluid_player_set_playback_callback(player, midi_event_handler, synth);
    tempo = midiBpms[currentSongIndex];
    fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, tempo);
    isPlaying = false;
}
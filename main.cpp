#include "raylib.h"
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <fluidsynth.h>
#ifndef FLUID_PLAYER_TEMPO_EXTERNAL_BPM
#define FLUID_PLAYER_TEMPO_EXTERNAL_BPM 1
#endif
#include <fstream>
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

int tempo = 120;
int currentSongIndex = 0;
int amountOfSongs = 0;

bool dropdownOpen = false;
float dropdownX = 20, dropdownY = 10, dropdownWidth = 260, dropdownHeight = 30;
Rectangle dropdownBox = {dropdownX, dropdownY, dropdownWidth, dropdownHeight};

struct PianoKey {
    Rectangle rect;
    bool isBlack;
    int midiNumber;
    std::string label;
};

struct SongInfo {
    std::string midiFile;
    std::string displayName;
    std::string artist;
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

// This methods manually reads the MIDI file to find the initial tempo in BPM.
// Yes... this is kinda hacky, but it works for most MIDI files.
int GetMidiInitialTempoBPM(const std::string &midiPath) {
    std::ifstream file(midiPath, std::ios::binary);
    if (!file) return -1;
    unsigned char buf[4];
    while (file.read((char *) buf, 1)) {
        if (buf[0] == 0xFF) {
            // Meta event
            file.read((char *) buf, 1);
            if (buf[0] == 0x51) {
                // Set Tempo
                file.read((char *) buf, 1); // length (should be 3)
                if (buf[0] == 3) {
                    file.read((char *) buf, 3);
                    int mpqn = (buf[0] << 16) | (buf[1] << 8) | buf[2];
                    if (mpqn > 0) return static_cast<int>(60000000.0 / mpqn);
                }
                break;
            }
        }
    }
    return -1;
}

int main() {
    bool isPlaying = false;
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);
    fluid_audio_driver_t *adriver = new_fluid_audio_driver(settings, synth);

    std::string soundFontDir = std::string(getenv("HOME")) + "/Documents/Sonique/soundFonts";
    // Create the SoundFont directory if it doesn't exist
    if (!std::filesystem::exists(soundFontDir)) {
        std::filesystem::create_directories(soundFontDir);
        std::cerr << "Created SoundFont directory at: " << soundFontDir << std::endl;
        return 0; // Exit the app gracefully
    }

    namespace fs = std::filesystem;
    std::vector<std::string> loadedSoundFonts;
    for (const auto &entry : fs::directory_iterator(soundFontDir)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            if (path.size() >= 4 &&
                (path.substr(path.size() - 4) == ".sf2" || path.substr(path.size() - 4) == ".SF2")) {
                loadedSoundFonts.push_back(path);
                }
        }
    }

    // Load the soundfont with default name
    int general = fluid_synth_sfload(synth, (soundFontDir + "/default.sf2").c_str(), 1);

    for (int i = 1; i < 16; ++i) {
        if (i == 9) continue;
        fluid_synth_program_select(synth, i, general, 0, 0);
    }

    std::vector<std::string> loadedMidiFiles;
    std::string midiDir = std::string(getenv("HOME")) + "/Documents/Sonique/midi";
    // Create the MIDI directory if it doesn't exist
    if (!fs::exists(midiDir)) {
        fs::create_directories(midiDir);
        std::cerr << "Created MIDI directory at: " << midiDir << std::endl;
        return 0; // Exit the app gracefully
    }

    for (const auto &entry: fs::directory_iterator(midiDir)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            if (path.size() >= 4 &&
                (path.substr(path.size() - 4) == ".mid" || path.substr(path.size() - 4) == ".MID")) {
                loadedMidiFiles.push_back(path);
            }
        }
    }
    amountOfSongs = loadedMidiFiles.size();

    std::vector<SongInfo> songInfos;
    std::ifstream infoFile(std::string(getenv("HOME")) + "/Documents/Sonique/songinfo");
    // Create the songinfo file if it doesn't exist
    if (!infoFile.is_open()) {
        std::ofstream createInfoFile(std::string(getenv("HOME")) + "/Documents/Sonique/songinfo");
        createInfoFile << "midi_file,display_name,artist\n"; // header
        createInfoFile.close();
        std::cerr << "Created songinfo file at: " << getenv("HOME") << "/Documents/Sonique/songinfo" << std::endl;
        return 0;
    }

    std::string line;
    while (std::getline(infoFile, line)) {
        size_t first = line.find(',');
        size_t second = line.find(',', first + 1);
        if (first != std::string::npos && second != std::string::npos) {
            SongInfo info;
            info.midiFile = line.substr(0, first);
            info.displayName = line.substr(first + 1, second - first - 1);
            info.artist = line.substr(second + 1);
            songInfos.push_back(info);
        }
    }
    infoFile.close();

    // Map loadedMidiFiles to songInfos
    std::vector<SongInfo> loadedSongInfos;
    for (const auto &midiPath: loadedMidiFiles) {
        std::string midiFile = midiPath.substr(midiPath.find_last_of('/') + 1);
        auto it = std::find_if(songInfos.begin(), songInfos.end(), [&](const SongInfo &s) {
            return s.midiFile == midiFile;
        });
        if (it != songInfos.end()) {
            loadedSongInfos.push_back(*it);
        } else {
            loadedSongInfos.push_back({midiFile, midiFile, "Unknown"});
        }
    }

    if (loadedMidiFiles.empty()) {
        std::cerr << "No MIDI files found in directory: " << midiDir << std::endl;
        return 1; // Exit the app gracefully
    }

    std::vector<int> midiBpms;
    for (const auto &path: loadedMidiFiles) {
        int bpm = GetMidiInitialTempoBPM(path);
        midiBpms.push_back(bpm > 0 ? bpm : 120); // fallback to 120 if not found
    }

    fluid_player_t *player = new_fluid_player(synth);
    fluid_player_add(player, loadedMidiFiles[currentSongIndex].c_str());
    fluid_player_set_playback_callback(player, midi_event_handler, synth);

    // get the tempo from the MIDI file manually and set it
    std::string midiPath = loadedMidiFiles[currentSongIndex];
    int midiBpm = GetMidiInitialTempoBPM(midiPath);
    if (midiBpm > 0) tempo = midiBpm;
    fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, tempo);


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

        // Define button properties
        float playBtnWidth = 80;
        float playBtnHeight = 30;
        float playBtnX = (windowWidth - playBtnWidth) / 2;
        float playBtnY = 10;
        Rectangle playBtn = {playBtnX, playBtnY, playBtnWidth, playBtnHeight};
        Rectangle metroBtn = {200, 10, 80, 30}; // Adjust or reposition as needed

        // Tempo control variables (define at appropriate scope)
        float tempoBoxWidth = 90;
        float tempoBoxHeight = 30;
        float tempoBoxX = metroBtn.x + metroBtn.width + 30;
        float tempoBoxY = metroBtn.y;
        Rectangle tempoBox = {tempoBoxX, tempoBoxY, tempoBoxWidth, tempoBoxHeight};
        Rectangle upBtn = {tempoBoxX + tempoBoxWidth - 28, tempoBoxY + 2, 24, 12};
        Rectangle downBtn = {tempoBoxX + tempoBoxWidth - 28, tempoBoxY + tempoBoxHeight - 14, 24, 12};

        // Draw toolbar background
        DrawRectangle(0, 0, windowWidth, 50, BLACK);
        DrawLineEx({0, 50}, {(float) windowWidth, 50}, 1.0f, DARKGRAY);

        // Draw tempo box
        DrawRectangleRec(tempoBox, DARKGRAY);

        // Draw tempo value
        std::string tempoStr = std::to_string(tempo);
        Vector2 tempoTextSize = MeasureTextEx(font, tempoStr.c_str(), 16, 1);
        DrawTextEx(font, tempoStr.c_str(),
                   {tempoBox.x + 50 - tempoTextSize.x / 2, tempoBox.y + 6}, 16, 1, YELLOW);

        // Draw up arrow
        DrawRectangleRec(upBtn, GRAY);
        DrawTriangle(
            Vector2{upBtn.x + 12, upBtn.y + 3},
            Vector2{upBtn.x + 5, upBtn.y + 10},
            Vector2{upBtn.x + 19, upBtn.y + 10},
            BLACK
        );
        // Draw down arrow
        DrawRectangleRec(downBtn, GRAY);
        DrawTriangle(
            Vector2{downBtn.x + 12, downBtn.y + 9},
            Vector2{downBtn.x + 5, downBtn.y + 2},
            Vector2{downBtn.x + 19, downBtn.y + 2},
            BLACK
        );

        // Handle tempo button clicks
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, upBtn)) {
                tempo += 1;
                fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, tempo);
            } else if (CheckCollisionPointRec(mouse, downBtn)) {
                if (tempo > 20) tempo -= 1;
                fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, tempo);
            }
        }

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

        // Draw dropdown
        DrawRectangleRec(dropdownBox, DARKGRAY);
        DrawTextEx(font, loadedSongInfos[currentSongIndex].displayName.c_str(),
                   {dropdownX + 10, dropdownY + 6}, 16, 1, WHITE);
        DrawTriangle(
            Vector2{dropdownX + dropdownWidth - 20, dropdownY + 12},
            Vector2{dropdownX + dropdownWidth - 10, dropdownY + 12},
            Vector2{dropdownX + dropdownWidth - 15, dropdownY + 22},
            BLACK
        );

        // Handle dropdown click
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, dropdownBox)) {
                dropdownOpen = !dropdownOpen;
            } else if (dropdownOpen) {
                // Check if a song is clicked
                for (int i = 0; i < amountOfSongs; ++i) {
                    Rectangle itemRect = {
                        dropdownX, dropdownY + dropdownHeight + i * dropdownHeight, dropdownWidth, dropdownHeight
                    };
                    if (CheckCollisionPointRec(mouse, itemRect)) {
                        if (currentSongIndex != i) {
                            currentSongIndex = i;
                            // Reload MIDI
                            fluid_player_stop(player);
                            player = new_fluid_player(synth);
                            fluid_player_add(player, loadedMidiFiles[currentSongIndex].c_str());
                            fluid_player_set_playback_callback(player, midi_event_handler, synth);
                            tempo = midiBpms[currentSongIndex];
                            fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, tempo);
                            isPlaying = false;
                        }
                        dropdownOpen = false;
                    }
                }
                // Click outside closes dropdown
                if (!CheckCollisionPointRec(mouse, {
                                                dropdownX, dropdownY + dropdownHeight, dropdownWidth,
                                                dropdownHeight * amountOfSongs
                                            })) {
                    dropdownOpen = false;
                }
            }
        }

        // Draw dropdown items if open
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


        // Draw buttons

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

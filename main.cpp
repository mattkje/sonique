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
#include <filesystem>

#include "utils/MidiUtils.h"
#include "ui/PianoKey.h"
#include "utils/SongInfo.h"
#include "utils/SoundFontUtils.h"
#include "ui/PianoPage.h"
#include "ui/MainMenuPage.h"

constexpr bool showKeyLabels = true;
// Add this at global scope in main.cpp (outside any function)
std::vector<std::vector<bool> > midiKeyStates(16, std::vector<bool>(NUM_TOTAL_KEYS, false));
enum class AppPage { MainMenu, Piano };


int main() {
    // --- FluidSynth and MIDI setup ---
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);
    fluid_audio_driver_t *adriver = new_fluid_audio_driver(settings, synth);

    std::string soundFontDir = std::string(getenv("HOME")) + "/Documents/Sonique/soundFonts";
    if (!EnsureSoundFontDir(soundFontDir)) {
        return 0;
    }
    std::vector<std::string> loadedSoundFonts = ScanSoundFonts(soundFontDir);
    int general = LoadSoundFont(synth, soundFontDir + "/general.sf2", 1);

    for (int i = 1; i < 16; ++i) {
        if (i == 9) continue;
        fluid_synth_program_select(synth, i, general, 0, 0);
    }

    std::vector<std::string> loadedMidiFiles;
    std::string midiDir = std::string(getenv("HOME")) + "/Documents/Sonique/midi";
    namespace fs = std::filesystem;
    if (!fs::exists(midiDir)) {
        fs::create_directories(midiDir);
        std::cerr << "Created MIDI directory at: " << midiDir << std::endl;
        return 0;
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
    int amountOfSongs = loadedMidiFiles.size();

    std::vector<SongInfo> songInfos = LoadSongInfos(std::string(getenv("HOME")) + "/Documents/Sonique/songinfo");

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
        return 1;
    }

    std::vector<int> midiBpms;
    for (const auto &path: loadedMidiFiles) {
        int bpm = GetMidiInitialTempoBPM(path);
        midiBpms.push_back(bpm > 0 ? bpm : 120);
    }

    // --- MIDI player setup ---
    int currentSongIndex = 0;
    fluid_player_t *player = new_fluid_player(synth);
    fluid_player_add(player, loadedMidiFiles[currentSongIndex].c_str());
    fluid_player_set_playback_callback(player, midi_event_handler, synth);

    int tempo = midiBpms[currentSongIndex];
    fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, tempo);

    // --- Key states ---
    std::vector<std::vector<bool> > midiKeyStates(16, std::vector<bool>(NUM_TOTAL_KEYS, false));

    // --- Window and UI ---
    const int initialWidth = 1220;
    const int initialHeight = 800;
    InitWindow(initialWidth, initialHeight, "Sonique");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    AppPage currentPage = AppPage::MainMenu;
    PianoPage pianoPage(
        synth, player, loadedMidiFiles, loadedSongInfos, midiBpms, midiKeyStates
    );
    MainMenuPage mainMenu([&]() { currentPage = AppPage::Piano; });

    while (!WindowShouldClose()) {
        switch (currentPage) {
            case AppPage::MainMenu:
                mainMenu.HandleInput();
                mainMenu.Update();
                mainMenu.Draw();
                break;
            case AppPage::Piano:
                pianoPage.HandleInput();
                pianoPage.Update();
                pianoPage.Draw();
                break;
        }
    }

    // --- Cleanup ---
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    CloseWindow();
    return 0;
}

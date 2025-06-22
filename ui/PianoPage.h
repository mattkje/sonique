#pragma once

#include <vector>
#include <string>
#include <raylib.h>
#include <fluidsynth.h>
#include "../utils/SongInfo.h"
#include "../MidiLogic/MidiBlock.h"
#include "PianoKey.h"

class PianoPage {
public:
    PianoPage(
        fluid_synth_t* synth,
        fluid_player_t* player,
        std::vector<std::string>& loadedMidiFiles,
        std::vector<SongInfo>& loadedSongInfos,
        std::vector<int>& midiBpms,
        std::vector<std::vector<bool>>& midiKeyStates
    );
    ~PianoPage();

    void Draw();
    void HandleInput();
    void Update();

    int GetCurrentSongIndex() const;
    int GetTempo() const;
    bool IsPlaying() const;

private:
    // UI state
    int tempo;
    int currentSongIndex;
    int amountOfSongs;
    bool dropdownOpen;
    bool isPlaying;

    // Falling blocks
    int fallSpeed = 200;

    // UI geometry
    float dropdownX, dropdownY, dropdownWidth, dropdownHeight;
    Rectangle dropdownBox{};

    // External dependencies
    fluid_synth_t* synth;
    fluid_player_t* player;
    std::vector<std::string>& loadedMidiFiles;
    std::vector<SongInfo>& loadedSongInfos;
    std::vector<int>& midiBpms;
    std::vector<std::vector<bool>>& midiKeyStates;

    // Resources
    Font font{};
    Texture2D background{}, whiteKey{}, whiteKeyPressed{}, blackKey{}, blackKeyPressed{}, playIcon{}, pauseIcon{};

    // Piano keys
    std::vector<PianoKey> keys;
    std::vector<bool> keyWasPressed;

    void ReloadSong(int songIndex);
    void LoadResources();
    void UnloadResources();
};
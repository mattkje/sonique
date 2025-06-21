//
// Created by Matti Kjellstadli on 21/06/2025.
//

#include "SoundFontUtils.h"
#include <filesystem>
#include <iostream>

bool EnsureSoundFontDir(const std::string& dirPath) {
    namespace fs = std::filesystem;
    if (!fs::exists(dirPath)) {
        fs::create_directories(dirPath);
        std::cerr << "Created SoundFont directory at: " << dirPath << std::endl;
        return false;
    }
    return true;
}

std::vector<std::string> ScanSoundFonts(const std::string& dirPath) {
    namespace fs = std::filesystem;
    std::vector<std::string> soundFonts;
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            if (path.size() >= 4 &&
                (path.substr(path.size() - 4) == ".sf2" || path.substr(path.size() - 4) == ".SF2")) {
                soundFonts.push_back(path);
            }
        }
    }
    return soundFonts;
}

int LoadSoundFont(fluid_synth_t* synth, const std::string& sf2Path, int resetPresets) {
    return fluid_synth_sfload(synth, sf2Path.c_str(), resetPresets);
}

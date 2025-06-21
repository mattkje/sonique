//
// Created by Matti Kjellstadli on 21/06/2025.
//

#pragma once
#include <string>
#include <vector>

struct SongInfo {
    std::string midiFile;
    std::string displayName;
    std::string artist;
};

std::vector<SongInfo> LoadSongInfos(const std::string& infoFilePath);
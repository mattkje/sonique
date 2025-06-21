//
// Created by Matti Kjellstadli on 21/06/2025.
//
#include "SongInfo.h"
#include <fstream>

std::vector<SongInfo> LoadSongInfos(const std::string& infoFilePath) {
    std::vector<SongInfo> songInfos;
    std::ifstream infoFile(infoFilePath);
    std::string line;
    // Skip header
    std::getline(infoFile, line);
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
    return songInfos;
}

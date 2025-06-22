// utils/MidiUtils.cpp

#include "MidiUtils.h"
#include "../MidiLogic/MidiBlock.h"
#include <vector>
#include <map>
#include <fstream>
#include <cstdint>
#include <iostream>

#define NOTE_OFF 0x80
#define NOTE_ON  0x90

class MidiBlock;
extern std::vector<std::vector<bool> > midiKeyStates;
extern std::vector<MidiBlock> midiBlocks;
int ticksPerQuarter = 480;

#include <fluidsynth.h>

int midi_event_handler(void *data, fluid_midi_event_t *event) {
    int type = fluid_midi_event_get_type(event);
    int channel = fluid_midi_event_get_channel(event);
    int key = fluid_midi_event_get_key(event);

    if (channel != 9 && key >= 21 && key <= 108) {
        int idx = key - 21;
        if (type == NOTE_ON && fluid_midi_event_get_velocity(event) > 0) {
            midiKeyStates[channel][idx] = true;
        } else if (type == NOTE_OFF || (type == NOTE_ON && fluid_midi_event_get_velocity(event) == 0)) {
            midiKeyStates[channel][idx] = false;
        }
    }

    fluid_synth_handle_midi_event((fluid_synth_t *) data, event);
    return FLUID_OK;
}

int GetMidiInitialTempoBPM(const std::string &midiPath) {
    std::ifstream file(midiPath, std::ios::binary);
    if (!file) return -1;
    unsigned char buf[4];
    while (file.read((char *) buf, 1)) {
        if (buf[0] == 0xFF) {
            file.read((char *) buf, 1);
            if (buf[0] == 0x51) {
                file.read((char *) buf, 1);
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


// Helper to read variable-length quantity
uint32_t readVarLen(std::ifstream &file) {
    uint32_t value = 0;
    unsigned char c;
    do {
        file.read((char *) &c, 1);
        value = (value << 7) | (c & 0x7F);
    } while (c & 0x80);
    return value;
}

// C++
void LoadMidiBlocks(const std::string &midiPath) {
    std::cout << "Loading MIDI blocks from: " << midiPath << std::endl;
    midiBlocks.clear();
    std::ifstream file(midiPath, std::ios::binary);
    if (!file) return;

    // Read header
    char header[14];
    file.read(header, 14);
    if (std::string(header, 4) != "MThd") return;
    uint16_t format = (header[8] << 8) | (header[9] & 0xFF);
    uint16_t ntrks = (header[10] << 8) | (header[11] & 0xFF);
    uint16_t division = (header[12] << 8) | (header[13] & 0xFF);
    ticksPerQuarter = division;
    double tempo = 500000.0; // default 120 BPM

    std::cout << "Format: " << format << ", ntrks: " << ntrks << ", division: " << division << std::endl;

    for (int t = 0; t < ntrks; ++t) {
        char trkHeader[8];
        file.read(trkHeader, 8);
        if (file.gcount() < 8 || std::string(trkHeader, 4) != "MTrk") break;
        uint32_t trkLen = (trkHeader[4] << 24) | ((trkHeader[5] & 0xFF) << 16) | ((trkHeader[6] & 0xFF) << 8) | (trkHeader[7] & 0xFF);
        std::streampos trackEnd = file.tellg();
        trackEnd += trkLen;

        std::map<int, double> noteOnTimes[16]; // channel -> key -> time
        uint8_t runningStatus = 0;
        double absTicks = 0.0;

        while (file.tellg() < trackEnd) {
            uint32_t delta = readVarLen(file);
            absTicks += delta;
            double timeSec = (absTicks * tempo) / (ticksPerQuarter * 1000000.0);

            uint8_t status;
            file.read((char *) &status, 1);
            if (status < 0x80) {
                file.unget();
                status = runningStatus;
            } else {
                runningStatus = status;
            }

            if ((status & 0xF0) == NOTE_ON || (status & 0xF0) == NOTE_OFF) {
                uint8_t key, vel;
                file.read((char *) &key, 1);
                file.read((char *) &vel, 1);
                int channel = status & 0x0F;
                if ((status & 0xF0) == NOTE_ON && vel > 0) {
                    noteOnTimes[channel][key] = timeSec;
                } else {
                    if (noteOnTimes[channel].count(key)) {
                        if (channel != 9) { // Ignore drums
                            double start = noteOnTimes[channel][key];
                            double duration = timeSec - start;
                            midiBlocks.emplace_back(
                                key,
                                channel,
                                start,
                                duration,
                                MidiBlock::colorForChannel(channel)
                            );
                        }
                        noteOnTimes[channel].erase(key);
                    }
                }
            } else if (status == 0xFF) {
                uint8_t metaType;
                file.read((char *) &metaType, 1);
                uint32_t len = readVarLen(file);
                if (metaType == 0x51 && len == 3) {
                    unsigned char tbuf[3];
                    file.read((char *) tbuf, 3);
                    tempo = (tbuf[0] << 16) | (tbuf[1] << 8) | tbuf[2];
                } else {
                    file.seekg(len, std::ios::cur);
                }
            } else if ((status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0) {
                file.seekg(1, std::ios::cur);
            } else {
                file.seekg(2, std::ios::cur);
            }
        }
        file.seekg(trackEnd);
    }
    std::cout << "Loaded blocks: " << midiBlocks.size() << std::endl;
}


int GetTicksPerQuarterFromMidi(const std::string& midiPath) {
    std::ifstream file(midiPath, std::ios::binary);
    if (!file) return 480; // default if file can't be opened

    char header[14];
    file.read(header, 14);
    if (file.gcount() < 14) return 480;

    // MIDI header: bytes 12-13 are ticks per quarter note (big endian)
    int tpq = (static_cast<unsigned char>(header[12]) << 8) | static_cast<unsigned char>(header[13]);
    return tpq;
}
//
// Created by Matti Kjellstadli on 21/06/2025.
//

#include "MidiUtils.h"
#include <fstream>

// You may need to extern midiKeyStates if it's still global
extern std::vector<std::vector<bool>> midiKeyStates;

int midi_event_handler(void *data, fluid_midi_event_t *event) {
    fluid_synth_t *synth = static_cast<fluid_synth_t *>(data);
    int type = fluid_midi_event_get_type(event);
    int key = fluid_midi_event_get_key(event);
    int channel = fluid_midi_event_get_channel(event);
    if (key >= 21 && key <= 108 && channel != 9) {
        int idx = key - 21;
        if (type == FLUID_SEQ_NOTEON && fluid_midi_event_get_velocity(event) > 0) {
            midiKeyStates[channel][idx] = true;
        } else if (type == FLUID_SEQ_NOTEOFF || (type == FLUID_SEQ_NOTEON && fluid_midi_event_get_velocity(event) == 0)) {
            midiKeyStates[channel][idx] = false;
        }
    }
    return fluid_synth_handle_midi_event(synth, event);
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
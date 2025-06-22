// MidiBlock.h
#pragma once

struct MidiColor {
    float r, g, b, a;
    MidiColor(float r, float g, float b, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
};

class MidiBlock {
public:
    int key;
    double start; // MIDI key number (21-108)
    int channel;        // MIDI channel (0-15)
    double startTime;   // When the note starts (seconds)
    double duration;    // How long the note lasts (seconds)
    MidiColor color;    // Color for the channel

    MidiBlock(int key, int channel, double startTime, double duration, const MidiColor& color);

    // Returns true if the block should be visible at currentTime
    bool isActive(double currentTime) const;

    // Returns the vertical position of the top of the block at currentTime
    float getY(float keyboardY, float speed, double currentTime) const;

    // Returns the height of the block (matches note duration)
    float getHeight(float speed) const;

    // Color mapping per channel
    static MidiColor colorForChannel(int channel);
};
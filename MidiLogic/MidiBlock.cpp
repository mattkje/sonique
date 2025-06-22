// MidiBlock.cpp
#include "MidiBlock.h"

#include <vector>


std::vector<MidiBlock> midiBlocks;

MidiBlock::MidiBlock(int key, int channel, double startTime, double duration, const MidiColor& color)
    : key(key), channel(channel), startTime(startTime), duration(duration), color(color) {}

bool MidiBlock::isActive(double currentTime) const {
    return currentTime >= startTime && currentTime <= (startTime + duration);
}

float MidiBlock::getY(float keyboardY, float speed, double currentTime) const {
    // Blocks fall down: higher startTime means higher initial Y
    // At startTime, block's bottom is at keyboardY
    // As time increases, block moves down (Y increases)
    float elapsed = static_cast<float>(currentTime - startTime);
    return keyboardY - getHeight(speed) + elapsed * speed;
}

float MidiBlock::getHeight(float speed) const {
    // Height is proportional to note duration
    return static_cast<float>(duration * speed);
}

MidiColor MidiBlock::colorForChannel(int channel) {
    if (channel == 0) {
        // Right hand: #66E5D2 (teal)
        return MidiColor(0.40f, 0.90f, 0.82f, 1.0f);
    } else if (channel == 1) {
        // Left hand: #A65BFF (purple)
        return MidiColor(0.65f, 0.36f, 1.0f, 1.0f);
    }
    // Default: use hues
    static const float hues[16] = {
        0.0f, 0.08f, 0.16f, 0.24f, 0.32f, 0.40f, 0.48f, 0.56f,
        0.64f, 0.72f, 0.80f, 0.88f, 0.96f, 0.12f, 0.28f, 0.44f
    };
    float h = hues[channel % 16];
    float s = 0.7f, v = 1.0f;
    float r, g, b;
    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    return MidiColor(r, g, b, 1.0f);
}
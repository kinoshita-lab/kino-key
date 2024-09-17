#pragma once
#ifndef LEDDISPLAY_HPP
#define LEDDISPLAY_HPP

#include <FastLED.h>

namespace kinoshita_lab
{
namespace kinokey_25
{
template <uint8_t data_pin>
class LedDisplay
{
private:
    enum
    {
        kNumLeds = 14,
    };

    enum
    {
        kLedOctaveMinus4 = 0,
        kLedOctaveMinus3 = 1,
        kLedOctaveMinus2 = 2,
        kLedOctaveMinus1 = 3,
        kLedOctave0      = 4,
        kLedOctave1      = 5,
        kLedOctave2      = 6,
        kLedOctave3      = 7,
        kLedOctave4      = 8,

        kLedSw1 = 9,
        kLedSw2 = 10,
        kLedSw3 = 11,
        kLedSw4 = 12,

        kNumOctaveLeds = 9,
    };
    CRGB leds_[kNumLeds];

    bool shouldUpdate_ = false;

public:
    LedDisplay()
    {
        FastLED.addLeds<WS2812, data_pin, GRB>(leds_, kNumLeds);
    }

    void initialize()
    {
        FastLED.clear();
        shouldUpdate_ = true;
    }

    void setOctave(const int octave)
    {
        for (int i = 0; i < kNumOctaveLeds; i++) {
            leds_[i] = CRGB::Black;
        }
        leds_[kLedOctaveMinus4 + octave - 1] = CRGB::White;
        leds_[kLedOctaveMinus4 + octave - 1].maximizeBrightness(16);

        shouldUpdate_ = true;
    }

    void setSw1(const bool on)
    {
        leds_[kLedSw1] = on ? CRGB::White : CRGB::Black;
        leds_[kLedSw1].maximizeBrightness(16);
        shouldUpdate_ = true;
    }

    void setSw2(const bool on)
    {
        leds_[kLedSw2] = on ? CRGB::White : CRGB::Black;
        leds_[kLedSw2].maximizeBrightness(16);
        shouldUpdate_ = true;
    }

    void setSw3(const bool on)
    {
        leds_[kLedSw3] = on ? CRGB::White : CRGB::Black;
        leds_[kLedSw3].maximizeBrightness(16);
        shouldUpdate_ = true;

        if (on) {
            setSw4(false);
        }
    }

    void setSw4(const bool on)
    {
        leds_[kLedSw4] = on ? CRGB::White : CRGB::Black;
        leds_[kLedSw4].maximizeBrightness(16);
        shouldUpdate_ = true;

        if (on) {
            setSw3(false);
        }
    }

    void loop()
    {
        if (shouldUpdate_) {
            FastLED.show();
            shouldUpdate_ = false;
        }
    }

    void test()
    {
        FastLED.clear();
        CRGB color = CRGB::White;
        for (int i = 0; i < kNumLeds; i++) {
            leds_[i] = color;
            leds_[i].maximizeBrightness(16);
        }
        FastLED.show();
    }
};
}
}
#endif // LEDDISPLAY_HPP

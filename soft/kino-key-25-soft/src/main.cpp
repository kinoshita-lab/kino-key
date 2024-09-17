#include "MIDI.h"
#include "ShiftregisterSwScanner.hpp"
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include "LedDisplay.hpp"

enum
{
    kNUM_KEYS = 25,
};

enum
{
    kNPL_PIN   = 6,
    kCLOCK_PIN = 7,

    kOUTPUT_PIN_1 = 4,
    kOUTPUT_PIN_2 = 2,

    kNeoPixelData_PIN = 3,
};

enum
{
    kSwitchIndexOctMinus = 13,
    kSwitchIndexOctPlus  = 14,
    kSwitchIndexSw1      = 15,

    kSwitchIndexSw2 = kinoshita_lab::ShiftregisterSwScanner25Keys::DEFAULT_NUM_SWITCH_EACH + 13,
    kSwitchIndexSw3 = kinoshita_lab::ShiftregisterSwScanner25Keys::DEFAULT_NUM_SWITCH_EACH + 14,
    kSwitchIndexSw4 = kinoshita_lab::ShiftregisterSwScanner25Keys::DEFAULT_NUM_SWITCH_EACH + 15,
};
struct KeyScanStatus
{
    bool shouldSend;
    bool offOn;
    KeyScanStatus() : shouldSend(false), offOn(false) {}
};
KeyScanStatus keyScanStatus[kNUM_KEYS];

struct NoteKeyInfo
{
    bool offOn     = false;
    int byKeyIndex = -1;
};
NoteKeyInfo noteKeyInfo[INT8_MAX];

struct ControllerStatus
{
    int midi_channel       = 1;
    int octave             = 0;
    bool shouldSendNoteOff = false;
    int program            = 0;
    bool sendProgramChange = false;
    enum
    {
        kOctaveMin = 0,
        kOctaveMax = 9,
    };
};
ControllerStatus controllerStatus;

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI_USB);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, serial_midi);

kinoshita_lab::kinokey_25::LedDisplay<kNeoPixelData_PIN> led_display;

void switchHandler(uint32_t switch_index, const int off_on);

kinoshita_lab::ShiftregisterSwScanner25Keys scanner(kNPL_PIN, kCLOCK_PIN,
                                                    kOUTPUT_PIN_1,
                                                    kOUTPUT_PIN_2,
                                                    switchHandler, 16, 1);

inline int clip(const int min, const int max, const int value)
{
    return value < min ? min : (value > max ? max : value);
}

void switchHandler(uint32_t switch_index, const int off_on)
{
    if (switch_index < 16) {
        // keys
        if (kSwitchIndexOctMinus > switch_index) {
            keyScanStatus[switch_index].shouldSend = true;
            keyScanStatus[switch_index].offOn      = off_on;
        }

        // otherwise
        if (switch_index == kSwitchIndexOctMinus) {
            if (off_on) {
                controllerStatus.octave = clip(ControllerStatus::kOctaveMin, ControllerStatus::kOctaveMax,
                                               controllerStatus.octave - 1);
                led_display.setOctave(controllerStatus.octave);
                Serial.printf("octave: %d\n", controllerStatus.octave);
            }
        }
        if (switch_index == kSwitchIndexOctPlus) {
            if (off_on) {
                controllerStatus.octave = clip(ControllerStatus::kOctaveMin, ControllerStatus::kOctaveMax,
                                               controllerStatus.octave + 1);

                led_display.setOctave(controllerStatus.octave);
            }
        }
        if (switch_index == kSwitchIndexSw1) { // program change -
            if (off_on) {
                controllerStatus.program           = clip(0, 127, controllerStatus.program - 1);
                controllerStatus.sendProgramChange = true;
            }
            led_display.setSw1(off_on);
        }

    } else {
        // keys
        if (switch_index < kSwitchIndexSw2) {
            const auto offset                               = 4;
            keyScanStatus[switch_index - offset].shouldSend = true;
            keyScanStatus[switch_index - offset].offOn      = off_on;
            Serial.printf("key index:%d\n", switch_index - offset);
        }

        // otherwise
        if (switch_index == kSwitchIndexSw2) { // program change +
            if (off_on) {
                controllerStatus.program           = clip(0, 127, controllerStatus.program + 1);
                controllerStatus.sendProgramChange = true;
            }
            led_display.setSw2(off_on);
        }

        if (switch_index == kSwitchIndexSw3) { // ch 1
            if (off_on) {
                controllerStatus.midi_channel      = 1;
                controllerStatus.shouldSendNoteOff = true;
                led_display.setSw3(off_on);
            }
        }

        if (switch_index == kSwitchIndexSw4) {
            if (off_on) {
                controllerStatus.midi_channel      = 10; // ch 2 drum
                controllerStatus.shouldSendNoteOff = true;
                led_display.setSw4(off_on);
            }
        }
    }
}

void sendNoteOn(const int note, const int velocity)
{
    if (note > INT8_MAX) {
        return;
    }
    const auto channel = controllerStatus.midi_channel;

    MIDI_USB.sendNoteOn(note, velocity, channel);
    serial_midi.sendNoteOn(note, velocity, channel);
    Serial.printf("note on %d\n", note);
}

void sendNoteOff(const int note, const int velocity)
{
    if (note > INT8_MAX) {
        return;
    }
    const auto channel = controllerStatus.midi_channel;
    MIDI_USB.sendNoteOff(note, velocity, channel);
    serial_midi.sendNoteOff(note, velocity, channel);
    Serial.printf("note off %d\n", note);
}

void sendProgramChange(const int program)
{
    const auto channel = controllerStatus.midi_channel;
    MIDI_USB.sendProgramChange(program, channel);
    serial_midi.sendProgramChange(program, channel);
}

void setup()
{
    for (int i = 0; i < kNUM_KEYS; i++) {
        keyScanStatus[i].shouldSend = false;
        keyScanStatus[i].offOn      = false;
    }

    // put your setup code here, to run once:
    MIDI_USB.begin(MIDI_CHANNEL_OMNI);
    serial_midi.begin(MIDI_CHANNEL_OMNI);
    Serial.begin(115200);
    delay(100);
    controllerStatus.octave = 5;
    led_display.setOctave(controllerStatus.octave);
    led_display.setSw3(true);
}

void setup1()
{
    led_display.initialize();
}

void loop1()
{
    led_display.loop();
}

void loop()
{

    scanner.update();

    if (controllerStatus.shouldSendNoteOff) {
        Serial.println("send note off");
        for (int i = 0; i < INT8_MAX; i++) {
            if (noteKeyInfo[i].offOn) {
                noteKeyInfo[i].offOn = false;
                sendNoteOff(i, 0);
            }
        }
        controllerStatus.shouldSendNoteOff = false;
    }

    if (controllerStatus.sendProgramChange) {
        controllerStatus.sendProgramChange = false;
        sendProgramChange(controllerStatus.program);
    }

    for (int i = 0; i < kNUM_KEYS; i++) {
        if (keyScanStatus[i].shouldSend) {
            keyScanStatus[i].shouldSend = false;
            const auto numNotePerOctave = 12;
            auto note                   = numNotePerOctave * controllerStatus.octave + i;
            if (keyScanStatus[i].offOn) {
                if (note >= INT8_MAX) {
                    break;
                }
                sendNoteOn(note, 127);
                noteKeyInfo[note].offOn      = true;
                noteKeyInfo[note].byKeyIndex = i;
            } else {
                const auto offset = note % numNotePerOctave;
                for (auto oct = 0; oct < ControllerStatus::kOctaveMax + 2; oct++) {
                    const auto actual_note = numNotePerOctave * oct + offset;
                    if (actual_note >= INT8_MAX) {
                        break;
                    }
                    if (noteKeyInfo[actual_note].byKeyIndex == i) {
                        if (noteKeyInfo[actual_note].offOn) {
                            noteKeyInfo[actual_note].offOn      = false;
                            noteKeyInfo[actual_note].byKeyIndex = -1;
                            sendNoteOff(actual_note, 0);
                            break;
                        }
                    }
                }
            }
        }
    }
}

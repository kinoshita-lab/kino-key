import utime
from shiftregister_scanner import ShiftregisterSwScanner
from led_display import LedDisplay
import kino_midi

# Pin definitions
NPL_PIN = 6
CLOCK_PIN = 7
OUTPUT_PIN_1 = 4
OUTPUT_PIN_2 = 2
NEOPIXEL_PIN = 3

NUM_KEYS = 25

kSwitchIndexOctMinus = 13
kSwitchIndexOctPlus  = 14
kSwitchIndexSw1      = 15

kSwitchIndexSw2 = 16 + 13
kSwitchIndexSw3 = 16 + 14
kSwitchIndexSw4 = 16 + 15

class ControllerStatus:
    def __init__(self):
        self.midi_channel = 0  # 0-indexed (Ch 1)
        self.octave = 5
        self.program = 0
        self.kOctaveMin = 0
        self.kOctaveMax = 9

controller = ControllerStatus()
led = LedDisplay(NEOPIXEL_PIN)

# Keep track of active notes to turn them off correctly if octave changes
note_key_info = {i: -1 for i in range(128)}  # note -> switch_index (-1 means off)

def clip(min_val, max_val, val):
    return max(min_val, min(max_val, val))

def switch_handler(switch_index, off_on):
    global controller
    
    if switch_index < 16:
        # keys
        if switch_index < kSwitchIndexOctMinus:
            handle_key(switch_index, off_on)
        
        # function switches
        elif switch_index == kSwitchIndexOctMinus and off_on:
            controller.octave = clip(controller.kOctaveMin, controller.kOctaveMax, controller.octave - 1)
            led.set_octave(controller.octave)
            
        elif switch_index == kSwitchIndexOctPlus and off_on:
            controller.octave = clip(controller.kOctaveMin, controller.kOctaveMax, controller.octave + 1)
            led.set_octave(controller.octave)
            
        elif switch_index == kSwitchIndexSw1:
            if off_on:
                controller.program = clip(0, 127, controller.program - 1)
                kino_midi.program_change(controller.program, controller.midi_channel)
            led.set_sw1(off_on)

    else:
        # keys
        if switch_index < kSwitchIndexSw2:
            offset = 4
            handle_key(switch_index - offset, off_on)
            
        # function switches
        elif switch_index == kSwitchIndexSw2:
            if off_on:
                controller.program = clip(0, 127, controller.program + 1)
                kino_midi.program_change(controller.program, controller.midi_channel)
            led.set_sw2(off_on)
            
        elif switch_index == kSwitchIndexSw3:
            if off_on:
                turn_off_all_notes()
                controller.midi_channel = 0 # Ch 1
                led.set_sw3(True)
                
        elif switch_index == kSwitchIndexSw4:
            if off_on:
                turn_off_all_notes()
                controller.midi_channel = 9 # Ch 10 (drums)
                led.set_sw4(True)

def handle_key(key_index, off_on):
    num_note_per_octave = 12
    note = num_note_per_octave * controller.octave + key_index
    
    if off_on:
        if note < 128:
            kino_midi.note_on(note, 127, controller.midi_channel)
            note_key_info[note] = key_index
    else:
        offset = key_index % num_note_per_octave
        for oct_val in range(controller.kOctaveMax + 2):
            actual_note = num_note_per_octave * oct_val + offset
            if actual_note < 128 and note_key_info[actual_note] == key_index:
                note_key_info[actual_note] = -1
                kino_midi.note_off(actual_note, 0, controller.midi_channel)
                break

def turn_off_all_notes():
    for note in range(128):
        if note_key_info[note] != -1:
            note_key_info[note] = -1
            kino_midi.note_off(note, 0, controller.midi_channel)

def setup():
    kino_midi.init_serial_midi()  # Initialize UART MIDI OUT
    led.set_octave(controller.octave)
    led.set_sw3(True)

def main():
    scanner = ShiftregisterSwScanner(NPL_PIN, CLOCK_PIN, OUTPUT_PIN_1, OUTPUT_PIN_2, switch_handler)
    setup()
    
    print("Kino-Key25 MicroPython MIDI Keyboard Started.")
    while True:
        try:
            scanner.update()
            led.loop()
            utime.sleep_ms(4)
        except KeyboardInterrupt:
            turn_off_all_notes()
            led.clear()
            led.loop()
            break

if __name__ == '__main__':
    main()

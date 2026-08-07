from machine import Pin
import neopixel

class LedDisplay:
    kNumLeds = 14
    
    kLedOctaveMinus4 = 0
    kLedOctave4      = 8
    
    kLedSw1 = 9
    kLedSw2 = 10
    kLedSw3 = 11
    kLedSw4 = 12

    def __init__(self, pin_num=3):
        self.np = neopixel.NeoPixel(Pin(pin_num, Pin.OUT), self.kNumLeds)
        self.should_update = False
        self.clear()

    def clear(self):
        for i in range(self.kNumLeds):
            self.np[i] = (0, 0, 0)
        self.should_update = True

    def _set_color(self, index, on):
        # 16 is max brightness roughly similar to C++ maximizeBrightness(16)
        color = (16, 16, 16) if on else (0, 0, 0)
        self.np[index] = color
        self.should_update = True

    def set_octave(self, octave):
        for i in range(9):
            self.np[i] = (0, 0, 0)
        
        # C++ does octave - 1. We clamp it to 0-8 to avoid out-of-bounds if octave=0.
        idx = max(0, min(8, octave - 1))
        self._set_color(self.kLedOctaveMinus4 + idx, True)

    def set_sw1(self, on):
        self._set_color(self.kLedSw1, on)

    def set_sw2(self, on):
        self._set_color(self.kLedSw2, on)

    def set_sw3(self, on):
        self._set_color(self.kLedSw3, on)
        if on:
            self.set_sw4(False)

    def set_sw4(self, on):
        self._set_color(self.kLedSw4, on)
        if on:
            self.set_sw3(False)

    def loop(self):
        if self.should_update:
            self.np.write()
            self.should_update = False

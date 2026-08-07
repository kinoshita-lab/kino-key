"""
Shift-register key scanner for Kino-Key25 (MicroPython port).

Port of soft/kino-key-25-soft/src/ShiftregisterSwScanner.hpp.
Scans two 74HC165-style shift-register chains (16 switches each) connected to
the RP2040 via NPL (latch), CLOCK, OUT1 (chain-1 data), OUT2 (chain-2 data).

Switch reads are INVERTED: physical OFF = HIGH (1), ON = LOW (0).
A state change is reported only after two consecutive scans agree (debounce).
The handler is called per edge with (switch_index, off_on):
  switch_index: 0..15 for chain 1, 16..31 for chain 2
  off_on: 1 = pressed (ON), 0 = released (OFF)
"""

from machine import Pin

class ShiftregisterSwScanner:
    def __init__(self, npl_pin, clock_pin, out1_pin, out2_pin, handler, num_switches=16):
        self.npl = Pin(npl_pin, Pin.OUT)
        self.clock = Pin(clock_pin, Pin.OUT)
        self.out1 = Pin(out1_pin, Pin.IN)
        self.out2 = Pin(out2_pin, Pin.IN)
        self.num = num_switches
        self.handler = handler

        # All buffers init to 1 (OFF = HIGH), matching the C++ reference.
        self.scan = [[1] * num_switches for _ in range(2)]
        self.former = [[1] * num_switches for _ in range(2)]
        self.status = [[1] * num_switches for _ in range(2)]

        # Idle state: NPL HIGH (shift mode), CLOCK LOW.
        self.npl.value(1)
        self.clock.value(0)

    def _read_chain(self):
        """Latch parallel data, then shift in num_switches bits from both chains."""
        # Parallel load (NPL LOW latches the inputs).
        self.npl.value(0)
        self.clock.value(0)
        # Enter shift mode (NPL HIGH); data is presented at Q7 (OUT1/OUT2).
        self.npl.value(1)
        
        for i in range(self.num):
            self.clock.value(0)
            self.scan[0][self.num - i - 1] = self.out1.value()
            self.scan[1][self.num - i - 1] = self.out2.value()
            self.clock.value(1)

    def update(self):
        """Do one scan + debounce. Call this periodically (~every 4 ms)."""
        self._read_chain()
        for j in range(2):
            for i in range(self.num):
                # Confirm only when current scan agrees with the previous scan.
                if self.scan[j][i] == self.former[j][i]:
                    new_status = self.scan[j][i]
                    if self.status[j][i] != new_status:
                        self.status[j][i] = new_status
                        # INVERTED reporting: ON (0) -> off_on=1
                        off_on = 0 if self.status[j][i] else 1
                        switch_index = i if j == 0 else i + self.num
                        if self.handler:
                            self.handler(switch_index, off_on)
                self.former[j][i] = self.scan[j][i]

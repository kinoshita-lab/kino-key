# Kino-Key25 — MicroPython port

MicroPython implementation of the Kino-Key25 USB-MIDI keyboard for the **Seeed XIAO RP2040**. A separate, parallel implementation of the C/C++ reference at [`../../kino-key-25-soft/`](../../kino-key-25-soft).

See [`AGENTS.md`](AGENTS.md) for the full project context and constraints.

## Hardware (GPIO)

| Signal | GPIO | Purpose |
|--------|------|---------|
| NPL    | 6    | Shift-register latch/parallel-load (RP2040 out) |
| CLOCK  | 7    | Shift-register clock (RP2040 out) |
| OUT1   | 4    | Shift-register chain-1 serial data (RP2040 **in**) |
| OUT2   | 2    | Shift-register chain-2 serial data (RP2040 **in**) |
| NeoPixel data | 3 | WS2812 LED strip (14 LEDs) |

Source of truth for pin numbers: [`../../kino-key-25-soft/src/main.cpp`](../../kino-key-25-soft/src/main.cpp).

## Running application code (no build needed)

This project uses the **MicroPico** VSCode extension — **no PlatformIO**. Open this folder in VSCode, then:
- **Run** a file on-device (MicroPico: "Run"), or
- **Sync** files to the device (MicroPico: "Upload project").

> You need a custom MicroPython firmware on the XIAO RP2040 first. **USB-MIDI requires a custom firmware build (below).**

## Building the MicroPython firmware (needed for USB-MIDI)

Stock MicroPython exposes only USB-CDC (serial) — **not** the USB-MIDI class. To send MIDI over USB you must build a custom MicroPython rp2 firmware that adds a MIDI interface (USB descriptor + TinyUSB MIDI class driver). The standard build is below; the MIDI-specific source modifications are specified in the implementation notes (`.planning/02-micropython-usb-midi.md`).

### Prerequisites
- Python 3.8+, Git, CMake
- GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`, 10.3+)
- A host C compiler (to build `mpy-cross`)

### Build steps (from a fresh clone)
```bash
mkdir firmware
cd firmware
git clone https://github.com/micropython/micropython
cd micropython
git checkout 06bcfd5b74c6d275ae0991a19dab8704299e4e05
git submodule update --init lib/pico-sdk lib/tinyusb

# 1) Build the MicroPython cross-compiler (host)
make -C mpy-cross

# 2) Fetch rp2 board submodules
cd ports/rp2
make submodules

# 3) Build the firmware for Seeed XIAO RP2040
make BOARD=SEEED_XIAO_RP2040
```

Output firmware:
```
ports/rp2/build-SEEED_XIAO_RP2040/firmware.uf2
```

### Flash to the board
1. Hold **BOOTSEL** on the XIAO RP2040 while plugging in USB (UF2 bootloader).
2. A mass-storage drive appears — copy `firmware.uf2` onto it.
3. The device reboots running the new firmware.

### Enabling USB-MIDI (the custom part)
Plain `make BOARD=SEEED_XIAO_RP2040` produces CDC-only firmware. To add the MIDI class and TinyUSB support, apply the provided patch file to your downloaded MicroPython repository before building:

```bash
cd /path/to/your/kino-key-25-micropico-version/firmware/micropython
patch -p1 < ../micropython_usb_midi.patch
```

Then compile and flash the firmware as usual. You will now be able to use `import usb_midi` in Python!

## Status
Basic functionality is implemented: key scanning (`shiftregister_scanner.py`), LED display (`led_display.py`), and USB-MIDI (`kino_midi.py`). Everything is tied together in `main.py`.

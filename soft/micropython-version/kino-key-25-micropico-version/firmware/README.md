# MicroPython Firmware Build Instructions

This directory contains the custom MicroPython firmware build required for USB-MIDI support on the Kino-Key25.

## Prerequisites
- Python 3.8+, Git, CMake
- GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`, 10.3+)
- A host C compiler (to build `mpy-cross`)

## Build Steps

To ensure compatibility with the provided `micropython_usb_midi.patch`, we use a specific commit of MicroPython (`06bcfd5b74c6d275ae0991a19dab8704299e4e05`).

```bash
# 1. Clone MicroPython and checkout the verified commit
git clone https://github.com/micropython/micropython
cd micropython
git checkout 06bcfd5b74c6d275ae0991a19dab8704299e4e05
git submodule update --init lib/pico-sdk lib/tinyusb

# 2. Apply the USB-MIDI patch
# The patch adds the MIDI class descriptor and TinyUSB MIDI driver.
patch -p1 < ../micropython_usb_midi.patch

# 3. Build the MicroPython cross-compiler (host)
make -C mpy-cross

# 4. Fetch rp2 board submodules
cd ports/rp2
make submodules

# 5. Build the firmware for Seeed XIAO RP2040
make BOARD=SEEED_XIAO_RP2040
```

## Output Firmware
The compiled firmware will be located at:
```
micropython/ports/rp2/build-SEEED_XIAO_RP2040/firmware.uf2
```

## Flashing
1. Hold **BOOTSEL** on the XIAO RP2040 while plugging in USB (UF2 bootloader mode).
2. A mass-storage drive will appear. Copy `firmware.uf2` onto it.
3. The device will reboot running the new firmware with `usb_midi` support enabled.

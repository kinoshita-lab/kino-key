# AGENTS.md

Guidance for AI coding agents working in **this MicroPython project** — the Kino-Key25 MicroPython port (USB-MIDI keyboard firmware for Seeed XIAO RP2040).

## Scope

- Implement the 25-key MIDI controller in **MicroPython**: key scanning, USB-MIDI out, WS2812 LED display.
- This is a **separate, parallel implementation**, not a replacement for the C/C++ reference.
- **Do not modify or build the C/C++ reference** at `../../kino-key-25-soft/`. Treat it as a read-only source of behavior to port from.

## Hardware target

Seeed **XIAO RP2040**. Pin assignments (GPIO numbers) — source of truth is `../../kino-key-25-soft/src/main.cpp`:

| Signal | GPIO | Purpose |
|--------|------|---------|
| NPL    | 6    | Shift-register latch/parallel-load (output) |
| CLOCK  | 7    | Shift-register clock (output) |
| OUT1   | 4    | RP2040 **input**: reads shift-register chain-1 serial out |
| OUT2   | 2    | RP2040 **input**: reads shift-register chain-2 serial out |
| NeoPixel data | 3 | WS2812 LED strip (14 LEDs) |

2 shift-register chains × 16 switches = 32 slots; 25 keys + 6 function switches (Oct-, Oct+, Sw1–Sw4) are mapped with offset-based chain-boundary handling (see `switchHandler` in the C/C++ `main.cpp`). Note `25 + 6 ≠ 32` — read `switchHandler` before assuming indices.

## MicroPython toolchain

- Uses the **MicroPico** VSCode extension (identified by `.micropico`). **No PlatformIO, no `pio` commands.**
- Code is edited, synced, and run on-device through MicroPico. There is no build/test/lint CLI for application code.
- Pylance `typeCheckingMode: basic`. Stubs live at `~/.micropico-stubs/included` (**outside this repo**, populated by MicroPico — run the stub sync on a fresh clone). `reportMissingModuleSource: none` is intentional; don't "fix" it.
- `.vscode/extensions.json` recommends `paulober.micropico` (the MicroPico extension).

## USB MIDI — the central technical constraint

MicroPython's RP2 port **already bundles TinyUSB** (the same upstream stack the C/C++ port wraps via Adafruit TinyUSB). Do NOT try to "add" TinyUSB. The real work is enabling the **MIDI class**, which is **not present in stock firmware** (default USB = CDC serial only; no `usb_midi` module, no MIDI option in `mpconfigport.h`).

Two realistic paths (verified against the MicroPython rp2 port `mpconfigport.h` as of 2026-08; upstream may change — re-check before relying):
1. **`machine.USBDevice` runtime descriptors** (no rebuild) — `MICROPY_HW_ENABLE_USB_RUNTIME_DEVICE` is default-on. Prototype MIDI descriptors from Python first, but treat actual MIDI data transfer as **unverified** until tested.
2. **Custom firmware build** (most reliable) — patch MicroPython rp2 to add a MIDI class descriptor + TinyUSB MIDI driver. Requires `pico-sdk` + `arm-none-eabi-gcc` + CMake + `mpy-cross`; flash the resulting `firmware.uf2`.

Implication: this project needs a **firmware build step** on top of the MicroPico edit/flash loop.

## Reference architecture (C/C++ — read-only, port from this)

Three files in `../../kino-key-25-soft/src/`:
- `main.cpp` — pin map, MIDI note mapping, octave/program-change/channel logic, main scan loop. Dual MIDI out: USB + `Serial1`. **Dual-core**: core0 = scan + MIDI, core1 = LED refresh.
- `ShiftregisterSwScanner.hpp` — key scanner. Debounced by two agreeing reads. **Switch reads are inverted: OFF = HIGH, ON = LOW.** Handler fires per edge with `(switch_index, off_on)`.
- `LedDisplay.hpp` — WS2812, 14 LEDs (13 used; index 13 unused). `setSw3`/`setSw4` mutually exclusive.

Porting notes:
- C/C++ uses FastLED; MicroPython should use the `neopixel`/`rp2` PIO driver.
- C/C++ dual-core (`setup1`/`loop1`) → MicroPython `_thread` or cooperative scheduling.
- Key scan in MicroPython: drive NPL/CLOCK via `machine.Pin` and read OUT1/OUT2, or use a PIO program for shift-register timing.

## Conventions

- Commit messages: short, English, optional area prefix (`[PCB]`, `[mech]`); close issues GitHub-style (`Fixes #N`).
- Repo docs are bilingual: `README.md` (EN) and `README.ja.md` (JA) at the repo root — keep both in sync if you touch them.

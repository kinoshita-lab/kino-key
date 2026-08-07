import usb_midi
from machine import UART, Pin

# --- UART MIDI settings ---
# Note: RP2040 GPIO0 only routes to UART0 (not UART1).
# Physical pin D6/GPIO0 remains the same; we use UART0 instead of UART1.
_UART_ID = 0            # GPIO0 is UART0 TX on RP2040
_UART_TX_PIN = 0        # GPIO0 = D6/TX on XIAO RP2040
_UART_BAUDRATE = 31250  # MIDI standard baudrate (must be 31250)
_UART_TIMEOUT_MS = 50   # Brief blocking to avoid silent partial writes

_serial_midi = None

def init_serial_midi():
    """Initialize UART for serial MIDI OUT. Call once in main.setup()."""
    global _serial_midi
    try:
        _serial_midi = UART(_UART_ID, baudrate=_UART_BAUDRATE,
                            tx=Pin(_UART_TX_PIN), timeout=_UART_TIMEOUT_MS)
    except Exception as e:
        print('[kino_midi] UART MIDI init failed:', e)
        _serial_midi = None

def _serial_write(msg):
    """Send raw MIDI bytes over UART. msg: bytes/bytearray of standard MIDI message."""
    if _serial_midi is not None:
        try:
            _serial_midi.write(msg)
        except Exception:
            pass

def _send_usb(msg):
    """Send raw MIDI bytes over USB-MIDI. TinyUSB adds CIN header internally."""
    try:
        usb_midi.write(msg)
    except:
        pass

def note_on(note, velocity=127, channel=0):
    if note < 0 or note > 127: return
    ch = channel & 0x0F
    # Raw MIDI message (same for USB and UART)
    # TinyUSB adds CIN header (0x09 for Note On) internally
    raw = bytes([0x90 | ch, note, velocity])
    _send_usb(raw)
    _serial_write(raw)

def note_off(note, velocity=0, channel=0):
    if note < 0 or note > 127: return
    ch = channel & 0x0F
    # Raw MIDI message (same for USB and UART)
    # TinyUSB adds CIN header (0x08 for Note Off) internally
    raw = bytes([0x80 | ch, note, velocity])
    _send_usb(raw)
    _serial_write(raw)

def program_change(program, channel=0):
    if program < 0 or program > 127: return
    ch = channel & 0x0F
    # Raw MIDI message (same for USB and UART)
    # TinyUSB adds CIN header (0x0C for Program Change) internally
    # Program Change is 2 bytes (no data2)
    raw = bytes([0xC0 | ch, program])
    _send_usb(raw)
    _serial_write(raw)

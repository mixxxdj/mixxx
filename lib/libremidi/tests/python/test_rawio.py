#!/usr/bin/env python
"""
Tests for the raw I/O backend and all send_message / send_ump overloads.
Run with: python test_rawio.py
"""
import pylibremidi as lm
import gc
import sys

failures = []

def check(name, condition):
    if not condition:
        failures.append(name)
        print(f"  FAIL: {name}")
    else:
        print(f"  ok: {name}")

# ============================================================
# Helper: MIDI 1 rawio loopback with automatic cleanup
# ============================================================
class Midi1Loopback:
    def __init__(self):
        self._stored_cb = [None]
        self.received = []

        in_config = lm.InputConfiguration()
        received = self.received  # avoid capturing self in lambda
        in_config.on_message = lambda msg: received.append(msg)
        in_config.direct = True

        rawio_in = lm.RawioInputConfiguration()
        rawio_in.set_receive_callback = lambda cb: self._stored_cb.__setitem__(0, cb)
        rawio_in.stop_receive = lambda: self._stored_cb.__setitem__(0, None)

        self.midi_in = lm.MidiIn(in_config, rawio_in)
        self.midi_in.open_virtual_port("test_in")

        out_config = lm.OutputConfiguration()
        rawio_out = lm.RawioOutputConfiguration()
        stored = self._stored_cb
        rawio_out.write_bytes = lambda data: (stored[0](data, 0) if stored[0] else None, lm.Error())[-1]

        self.midi_out = lm.MidiOut(out_config, rawio_out)
        self.midi_out.open_virtual_port("test_out")

    def close(self):
        self.midi_in.close_port()
        self.midi_out.close_port()
        self.received.clear()
        del self.midi_in, self.midi_out

# ============================================================
# Helper: MIDI 2 (UMP) rawio loopback with automatic cleanup
# ============================================================
class UmpLoopback:
    def __init__(self, ignore_sysex=True):
        self._stored_cb = [None]
        self.received = []

        in_config = lm.UmpInputConfiguration()
        received = self.received  # avoid capturing self in lambda
        in_config.on_message = lambda msg: received.append(msg)
        in_config.ignore_sysex = ignore_sysex
        in_config.direct = True

        rawio_in = lm.RawioUmpInputConfiguration()
        rawio_in.set_receive_callback = lambda cb: self._stored_cb.__setitem__(0, cb)
        rawio_in.stop_receive = lambda: self._stored_cb.__setitem__(0, None)

        self.midi_in = lm.MidiIn(in_config, rawio_in)
        self.midi_in.open_virtual_port("test_ump_in")

        out_config = lm.OutputConfiguration()
        rawio_out = lm.RawioUmpOutputConfiguration()
        stored = self._stored_cb
        rawio_out.write_ump = lambda data: (stored[0](data, 0) if stored[0] else None, lm.Error())[-1]

        self.midi_out = lm.MidiOut(out_config, rawio_out)
        self.midi_out.open_virtual_port("test_ump_out")

    def close(self):
        self.midi_in.close_port()
        self.midi_out.close_port()
        self.received.clear()
        del self.midi_in, self.midi_out


# ============================================================
# Test: API detection
# ============================================================
print("--- API detection ---")
t = Midi1Loopback()
check("midi1 input api is RAW_IO", t.midi_in.get_current_api() == lm.API.RAW_IO)
check("midi1 output api is RAW_IO", t.midi_out.get_current_api() == lm.API.RAW_IO)
t.close()

t = UmpLoopback()
check("ump input api is RAW_IO_UMP", t.midi_in.get_current_api() == lm.API.RAW_IO_UMP)
check("ump output api is RAW_IO_UMP", t.midi_out.get_current_api() == lm.API.RAW_IO_UMP)
t.close()


# ============================================================
# Test: send_message overloads (MIDI 1)
# ============================================================
print("\n--- send_message overloads ---")

# send_message(b0) - 1 byte
t = Midi1Loopback()
t.midi_out.send_message(0xFA)  # Start (not filtered by default, unlike Active Sensing)
check("send_message(b0): 1 message received", len(t.received) == 1)
check("send_message(b0): correct byte", t.received[0].bytes[0] == 0xFA)
t.close()

# send_message(b0, b1) - 2 bytes
t = Midi1Loopback()
t.midi_out.send_message(0xC0, 42)  # Program Change
check("send_message(b0,b1): 1 message received", len(t.received) == 1)
check("send_message(b0,b1): correct status", t.received[0].bytes[0] == 0xC0)
check("send_message(b0,b1): correct data", t.received[0].bytes[1] == 42)
t.close()

# send_message(b0, b1, b2) - 3 bytes
t = Midi1Loopback()
t.midi_out.send_message(0x90, 60, 100)  # Note On
check("send_message(b0,b1,b2): 1 message received", len(t.received) == 1)
check("send_message(b0,b1,b2): correct status", t.received[0].bytes[0] == 0x90)
check("send_message(b0,b1,b2): correct note", t.received[0].bytes[1] == 60)
check("send_message(b0,b1,b2): correct velocity", t.received[0].bytes[2] == 100)
t.close()

# send_message(list) - vector overload
t = Midi1Loopback()
t.midi_out.send_message([0xB0, 7, 80])  # CC
check("send_message(list): 1 message received", len(t.received) == 1)
check("send_message(list): correct status", t.received[0].bytes[0] == 0xB0)
check("send_message(list): correct cc", t.received[0].bytes[1] == 7)
check("send_message(list): correct value", t.received[0].bytes[2] == 80)
t.close()

# send_message(Message) - message object overload
t = Midi1Loopback()
msg = lm.Message()
msg.bytes = [0xE0, 0x00, 0x40]  # Pitch Bend center
t.midi_out.send_message(msg)
check("send_message(Message): 1 message received", len(t.received) == 1)
check("send_message(Message): correct status", t.received[0].bytes[0] == 0xE0)
check("send_message(Message): correct lsb", t.received[0].bytes[1] == 0x00)
check("send_message(Message): correct msb", t.received[0].bytes[2] == 0x40)
t.close()
del msg

# schedule_message(timestamp, list)
t = Midi1Loopback()
t.midi_out.schedule_message(12345, [0x90, 48, 127])
check("schedule_message(ts,list): 1 message received", len(t.received) == 1)
check("schedule_message(ts,list): correct note", t.received[0].bytes[1] == 48)
t.close()


# ============================================================
# Test: send_ump overloads (MIDI 2 / UMP)
# ============================================================
print("\n--- send_ump overloads ---")

# send_ump(u0) - 1 word (System Real-Time Start, type 1 - not subject to MIDI1->2 upconversion)
t = UmpLoopback()
t.midi_out.send_ump(0x10FA0000)  # System RT Start, group 0
check("send_ump(u0): 1 message received", len(t.received) == 1)
check("send_ump(u0): correct word0", t.received[0].data[0] == 0x10FA0000)
t.close()

# send_ump(u0, u1) - 2 words (MIDI 2 channel voice)
t = UmpLoopback()
t.midi_out.send_ump(0x4090003C, 0xC0000000)  # Note On
check("send_ump(u0,u1): 1 message received", len(t.received) == 1)
check("send_ump(u0,u1): correct word0", t.received[0].data[0] == 0x4090003C)
check("send_ump(u0,u1): correct word1", t.received[0].data[1] == 0xC0000000)
t.close()

# send_ump(u0, u1, u2, u3) - 4 words (SysEx8, type 5 - need ignore_sysex=False)
t = UmpLoopback(ignore_sysex=False)
t.midi_out.send_ump(0x50010000, 0x01020304, 0x05060708, 0x090A0B0C)
check("send_ump(u0,u1,u2,u3): 1 message received", len(t.received) == 1)
if t.received:
    check("send_ump(u0,u1,u2,u3): correct word0", t.received[0].data[0] == 0x50010000)
    check("send_ump(u0,u1,u2,u3): correct word1", t.received[0].data[1] == 0x01020304)
    check("send_ump(u0,u1,u2,u3): correct word2", t.received[0].data[2] == 0x05060708)
    check("send_ump(u0,u1,u2,u3): correct word3", t.received[0].data[3] == 0x090A0B0C)
t.close()

# send_ump(list) - vector overload
t = UmpLoopback()
t.midi_out.send_ump([0x4090003C, 0xC0000000])
check("send_ump(list): 1 message received", len(t.received) == 1)
check("send_ump(list): correct word0", t.received[0].data[0] == 0x4090003C)
check("send_ump(list): correct word1", t.received[0].data[1] == 0xC0000000)
t.close()

# send_ump(Ump) - ump object overload
t = UmpLoopback()
ump = lm.Ump()
ump.data = (0x4090003C, 0xC0000000, 0, 0)
t.midi_out.send_ump(ump)
check("send_ump(Ump): 1 message received", len(t.received) == 1)
check("send_ump(Ump): correct word0", t.received[0].data[0] == 0x4090003C)
check("send_ump(Ump): correct word1", t.received[0].data[1] == 0xC0000000)
t.close()
del ump

# schedule_ump(timestamp, list)
t = UmpLoopback()
t.midi_out.schedule_ump(99999, [0x4090003C, 0xC0000000])
check("schedule_ump(ts,list): 1 message received", len(t.received) == 1)
check("schedule_ump(ts,list): correct word0", t.received[0].data[0] == 0x4090003C)
t.close()


# ============================================================
# Test: multiple messages in sequence
# ============================================================
print("\n--- multiple messages ---")

t = Midi1Loopback()
t.midi_out.send_message(0x90, 60, 100)
t.midi_out.send_message(0x80, 60, 0)
t.midi_out.send_message(0xB0, 7, 80)
t.midi_out.send_message([0xB0, 10, 64])
check("multiple midi1: 4 messages", len(t.received) == 4)
check("multiple midi1: msg0 note on", t.received[0].bytes[0] == 0x90)
check("multiple midi1: msg1 note off", t.received[1].bytes[0] == 0x80)
check("multiple midi1: msg2 cc7", t.received[2].bytes[1] == 7)
check("multiple midi1: msg3 cc10", t.received[3].bytes[1] == 10)
t.close()

t = UmpLoopback()
t.midi_out.send_ump(0x4090003C, 0xC0000000)
t.midi_out.send_ump(0x4080003C, 0x00000000)
t.midi_out.send_ump([0x40B00007, 0x80000000])
check("multiple ump: 3 messages", len(t.received) == 3)
t.close()


# ============================================================
# Test: close_port calls stop_receive
# ============================================================
print("\n--- close_port ---")

stopped = [False]
_in_config = lm.InputConfiguration()
_in_config.on_message = lambda msg: None
_in_config.direct = True

_rawio_in = lm.RawioInputConfiguration()
_rawio_in.set_receive_callback = lambda cb: None
_rawio_in.stop_receive = lambda: stopped.__setitem__(0, True)

_midi_in = lm.MidiIn(_in_config, _rawio_in)
_midi_in.open_virtual_port("test")
check("stop_receive not called yet", not stopped[0])
_midi_in.close_port()
check("stop_receive called on close", stopped[0])
del _midi_in, _in_config, _rawio_in


# ============================================================
# Test: Error default constructor
# ============================================================
print("\n--- Error ---")
err = lm.Error()
check("default Error is falsy (no error)", not err)
del err


# ============================================================
# Test: edge cases - large values, boundary values
# ============================================================
print("\n--- edge cases ---")

# Maximum MIDI 1 byte values
t = Midi1Loopback()
t.midi_out.send_message(0xFF)  # System Reset
check("send_message(0xFF): received", len(t.received) == 1)
check("send_message(0xFF): correct", t.received[0].bytes[0] == 0xFF)
t.close()

# UMP with max values in data portion (type 4, 2 words)
t = UmpLoopback()
t.midi_out.send_ump(0x409F7F7F, 0xFFFFFFFF)
check("send_ump(large values): received", len(t.received) == 1)
check("send_ump(large values): word0", t.received[0].data[0] == 0x409F7F7F)
check("send_ump(large values): word1", t.received[0].data[1] == 0xFFFFFFFF)
t.close()


# ============================================================
# Ensure cleanup before exit
# ============================================================
del t
gc.collect()

# ============================================================
# Summary
# ============================================================
print(f"\n{'='*40}")
if failures:
    print(f"FAILED: {len(failures)} test(s)")
    for f in failures:
        print(f"  - {f}")
    sys.exit(1)
else:
    print("All tests passed.")
    sys.exit(0)

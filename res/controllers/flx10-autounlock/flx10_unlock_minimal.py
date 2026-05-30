#!/usr/bin/env python3
"""flx10_unlock_minimal.py — send ONLY the 7 vendor unlock commands.

No snd-usb-audio unbind/rebind: that step in flx10_unlock_v2.py exists only to
force the AUDIO driver to re-probe sample rates. If Linux audio already works,
the screen/HID unlock should be just these 7 vendor OUT control transfers.

Run as root (libusb needs device access): sudo python3 flx10_unlock_minimal.py
"""
import sys
import time
import usb.core
import usb.util

VID, PID = 0x2B73, 0x0041
VENDOR_CMDS = [
    (0x0100, 0xC028),
    (0x0000, 0xC029),
    (0x0200, 0xC013),
    (0x0000, 0xC02B),
    (0x0100, 0xC026),
    (0x0000, 0xC01D),
    (0x0100, 0xC027),
]

dev = usb.core.find(idVendor=VID, idProduct=PID)
if dev is None:
    print(f"FLX10 (VID 0x{VID:04X} PID 0x{PID:04X}) not found — plug it in.")
    sys.exit(1)

ok = 0
for i, (wValue, wIndex) in enumerate(VENDOR_CMDS, 1):
    try:
        # bmRequestType 0x40 = vendor, host->device, recipient=device; bRequest 3
        dev.ctrl_transfer(0x40, 3, wValue, wIndex, None)
        print(f"[{i}/7] wValue=0x{wValue:04X} wIndex=0x{wIndex:04X}  OK")
        ok += 1
    except usb.core.USBError as e:
        print(f"[{i}/7] wValue=0x{wValue:04X} wIndex=0x{wIndex:04X}  FAIL: {e}")
    time.sleep(0.005)

usb.util.dispose_resources(dev)
print(f"done: {ok}/7 vendor commands sent")
sys.exit(0 if ok == 7 else 2)

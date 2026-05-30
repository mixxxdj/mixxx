#!/usr/bin/env python3
"""
flx10_unlock_v2.py — Unlock the DDJ-FLX10 and force snd-usb-audio to re-probe.

Strategy:
  1. Find the device in sysfs.
  2. Unbind snd-usb-audio from every audio interface via /sys/.../unbind.
     This is more aggressive than libusb detach — the driver stays gone
     until we explicitly bind it back, so it can't auto-rebind mid-unlock.
  3. Send the seven captured vendor OUT commands.
  4. Bind snd-usb-audio back via /sys/.../bind, forcing a fresh probe.
     With the device now unlocked, the Clock Source query should return
     valid sample rates and PCM substreams should get created.
  5. Show /proc/asound/cardN/ contents to verify.

Run as root. Close Mixxx and any other audio user of the FLX10 first.
"""

import os
import sys
import time
import glob
import subprocess
import usb.core
import usb.util

VID = 0x2B73
PID = 0x0041

VENDOR_CMDS = [
    (0x0100, 0xC028),
    (0x0000, 0xC029),
    (0x0200, 0xC013),
    (0x0000, 0xC02B),
    (0x0100, 0xC026),
    (0x0000, 0xC01D),
    (0x0100, 0xC027),
]


def find_sysfs_device(vid, pid):
    """Return e.g. '/sys/bus/usb/devices/1-3' for the matching device."""
    for vendor_file in glob.glob("/sys/bus/usb/devices/*/idVendor"):
        try:
            with open(vendor_file) as f:
                v = int(f.read().strip(), 16)
            if v != vid:
                continue
            product_file = vendor_file.replace("idVendor", "idProduct")
            with open(product_file) as f:
                p = int(f.read().strip(), 16)
            if p == pid:
                return os.path.dirname(vendor_file)
        except (OSError, ValueError):
            continue
    return None


def interfaces_bound_to(sysfs_dev, driver_name):
    """Return list of interface names (e.g. '1-3:1.0') currently bound to driver."""
    intfs = []
    for intf_dir in sorted(glob.glob(f"{sysfs_dev}/*:*")):
        drv_link = os.path.join(intf_dir, "driver")
        if os.path.islink(drv_link):
            drv = os.path.basename(os.readlink(drv_link))
            if drv == driver_name:
                intfs.append(os.path.basename(intf_dir))
    return intfs


def write_sysfs(path, value):
    try:
        with open(path, "w") as f:
            f.write(value)
        return True, None
    except OSError as e:
        return False, str(e)


def find_card_number(sysfs_dev):
    """Find which /proc/asound/cardN/ corresponds to this USB device."""
    # Look for sound/card* dirs under any of the device's interfaces
    for sound_dir in glob.glob(f"{sysfs_dev}/*:*/sound/card*"):
        name = os.path.basename(sound_dir)  # 'card4'
        return name.replace("card", "")
    return None


def main():
    if os.geteuid() != 0:
        print("Need root. Re-run with sudo.")
        sys.exit(1)

    print(f"Looking for FLX10 (VID=0x{VID:04X} PID=0x{PID:04X})…")
    sysfs = find_sysfs_device(VID, PID)
    if not sysfs:
        print("ERROR: FLX10 not present in sysfs. Plug it in.")
        sys.exit(1)
    print(f"  sysfs:  {sysfs}")

    audio_intfs = interfaces_bound_to(sysfs, "snd-usb-audio")
    print(f"  snd-usb-audio currently bound to: {audio_intfs}")

    if not audio_intfs:
        print("  (snd-usb-audio claims no interfaces — odd, continuing anyway)")

    print("\n[1] Unbinding snd-usb-audio from all audio interfaces…")
    for intf in audio_intfs:
        ok, err = write_sysfs("/sys/bus/usb/drivers/snd-usb-audio/unbind", intf)
        print(f"    {intf}: {'OK' if ok else f'FAIL ({err})'}")

    # Brief settle time after unbind
    time.sleep(0.3)

    print("\n[2] Sending vendor unlock handshake…")
    dev = usb.core.find(idVendor=VID, idProduct=PID)
    if dev is None:
        print("ERROR: pyusb can't see the device. Aborting.")
        sys.exit(1)

    for i, (wValue, wIndex) in enumerate(VENDOR_CMDS, 1):
        try:
            dev.ctrl_transfer(0x40, 3, wValue, wIndex, None)
            print(f"    [{i}/7] vendor OUT wValue=0x{wValue:04X} "
                  f"wIndex=0x{wIndex:04X}  OK")
        except usb.core.USBError as e:
            print(f"    [{i}/7] vendor OUT wValue=0x{wValue:04X} "
                  f"wIndex=0x{wIndex:04X}  FAIL: {e}")
        time.sleep(0.005)

    usb.util.dispose_resources(dev)

    # Let the device finish whatever the unlock kicks off
    time.sleep(0.5)

    print("\n[3] Rebinding snd-usb-audio (forces re-probe)…")
    for intf in audio_intfs:
        ok, err = write_sysfs("/sys/bus/usb/drivers/snd-usb-audio/bind", intf)
        print(f"    {intf}: {'OK' if ok else f'FAIL ({err})'}")

    time.sleep(0.5)

    print("\n[4] Result")
    card_num = find_card_number(sysfs)
    if card_num:
        card_dir = f"/proc/asound/card{card_num}"
        print(f"    card directory: {card_dir}")
        try:
            entries = sorted(os.listdir(card_dir))
            print(f"    contents: {entries}")
            for streamfile in ["stream0", "stream1"]:
                full = os.path.join(card_dir, streamfile)
                if os.path.exists(full):
                    print(f"\n    --- {streamfile} ---")
                    with open(full) as f:
                        print(f.read())
        except OSError as e:
            print(f"    couldn't list {card_dir}: {e}")
    else:
        print("    no /proc/asound/cardN directory found — card was not created.")

    print("\n--- aplay -l ---")
    subprocess.run(["aplay", "-l"])
    print("\n--- arecord -l ---")
    subprocess.run(["arecord", "-l"])


if __name__ == "__main__":
    main()
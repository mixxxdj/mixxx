# DDJ-FLX10 auto-unlock

The FLX10 boots in a "locked" state: the jog-wheel HID screen won't accept
waveform/state data until a short **vendor handshake** is sent over USB. These
files run that handshake **automatically on plug-in** (and at boot if it's
already connected), via udev + systemd — so you never run the unlock script by
hand.

> The handshake is **7 vendor control transfers** (`bmRequestType=0x40,
> bRequest=3`). That is *not* a HID report, so Mixxx (which talks to controllers
> through hidapi) can't send it — and it needs USB (libusb) access, which is why
> it runs as root via systemd rather than from inside Mixxx.

## Files

| File | Installed to | Purpose |
|------|--------------|---------|
| `flx10_unlock_minimal.py` | `/usr/local/bin/` | Sends the 7 vendor unlock commands. No audio unbind/rebind. |
| `flx10-unlock.service` | `/etc/systemd/system/` | Oneshot service that runs the script (as root). |
| `99-flx10-unlock.rules` | `/etc/udev/rules.d/` | Starts the service when VID `2b73` / PID `0041` is plugged in. |

`flx10_unlock_v2.py` (in the controllers folder) is the **fuller** version that
also unbinds/rebinds `snd-usb-audio` to force an audio sample-rate re-probe.
You only need that if Linux audio for the FLX10 *doesn't* work; for screen
unlock alone, the minimal script is enough.

## Install (one time)

```bash
sudo install -m 0755 flx10_unlock_minimal.py /usr/local/bin/flx10_unlock_minimal.py
sudo install -m 0644 flx10-unlock.service    /etc/systemd/system/flx10-unlock.service
sudo install -m 0644 99-flx10-unlock.rules   /etc/udev/rules.d/99-flx10-unlock.rules
sudo systemctl daemon-reload
sudo udevadm control --reload-rules
```

## Test

Replug the FLX10 (or `sudo udevadm trigger -c add -s usb`), then:

```bash
journalctl -u flx10-unlock.service -n 20 --no-pager   # expect: "done: 7/7 vendor commands sent"
```

Open Mixxx — the jog screen should render with no manual step.

## Requirements

- `pyusb` available to **root's** python3 (`sudo pip install pyusb`, or your
  distro's `python-pyusb`).
- Runs as root (systemd default) so libusb can reach the device.

## Disable / uninstall

```bash
# stop auto-trigger only (keep files):
sudo rm /etc/udev/rules.d/99-flx10-unlock.rules && sudo udevadm control --reload-rules

# full uninstall:
sudo rm /etc/udev/rules.d/99-flx10-unlock.rules \
        /etc/systemd/system/flx10-unlock.service \
        /usr/local/bin/flx10_unlock_minimal.py
sudo systemctl daemon-reload && sudo udevadm control --reload-rules
```

## How it works

```
plug in FLX10 ──> udev matches VID 2b73/PID 0041 (99-flx10-unlock.rules)
              ──> ENV{SYSTEMD_WANTS} starts flx10-unlock.service
              ──> python3 /usr/local/bin/flx10_unlock_minimal.py  (as root)
              ──> 7 vendor control transfers ──> screen unlocked
```

"""Shared spix RPC primitives used by all step modules.

Every helper here returns a plain bool (or a value) and never raises for
"condition not met" -- callers decide whether that is an assertion failure
(Then steps) or a keep-polling signal (retry loops). Having exactly one
implementation of each primitive keeps the timeout/poll behavior identical
across step modules; modules that expose the same names with different
defaults must adapt via thin wrappers, not re-implementations.
"""

import time

QT_LEFT_BUTTON = 1
QT_RIGHT_BUTTON = 2
QT_CONTROL_MODIFIER = 2

QT_KEY_ENTER = 0x01000005
QT_KEY_SPACE = 0x20
QT_KEY_DOWN = 0x01000015
QT_KEY_UP = 0x01000013
QT_KEY_TAB = 0x01000001
QT_KEY_A = 0x41


def is_visible(rpc, path):
    """True when the item exists, is visible, and has a non-degenerate box."""
    x, y, width, height = rpc.getBoundingBox(path)
    return rpc.existsAndVisible(path) and width * height > 0


def wait_visible(rpc, path, timeout=30):
    """Poll ``existsAndVisible`` until true or ``timeout`` seconds elapse."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            if rpc.existsAndVisible(path):
                return True
        except Exception:
            pass
        time.sleep(0.3)
    return False


def wait_hidden(rpc, path, timeout=30):
    """Poll ``existsAndVisible`` until false or ``timeout`` seconds elapse."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            if not rpc.existsAndVisible(path):
                return True
        except Exception:
            pass
        time.sleep(0.3)
    return False


def wait_clickable(rpc, path, timeout=5):
    """Wait until an item is visible with a non-degenerate bounding box.

    spix's ``existsAndVisible`` reports an item as visible as soon as its
    ``visible`` chain is true, before its layout has settled. A recycled
    delegate can briefly report a stale/zero-sized box, and clicking such a
    stale position silently misses (spix records "Item not found" without
    raising; see ``getErrors``). Only return true once the item has a real
    on-screen box.
    """
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            if rpc.existsAndVisible(path):
                x, y, width, height = rpc.getBoundingBox(path)
                if x >= 0 and y >= 0 and width > 0 and height > 0:
                    return True
        except Exception:
            pass
        time.sleep(0.3)
    return False


def click(rpc, path):
    rpc.mouseClick(path)


def get_control_value(rpc, group, key):
    """Read a ControlObject via the custom ``getControlValue`` command."""
    rpc.command("getControlValue", f"{group},{key}")
    return float(rpc.getStringProperty("mainWindow", "lastControlValue"))

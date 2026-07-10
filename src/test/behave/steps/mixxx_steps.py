import http.client
import json
import os
import sys
import re
import time
import socket
import xmlrpc.client
from behave import given, when, then
import mixxx_profile as profile
import random
from spix_helpers import (
    QT_CONTROL_MODIFIER,
    QT_KEY_DOWN,
    QT_KEY_ENTER,
    QT_KEY_SPACE,
    QT_KEY_TAB,
    QT_KEY_UP,
    QT_LEFT_BUTTON,
    QT_RIGHT_BUTTON,
    click as _click_impl,
    get_control_value as _get_control_value_impl,
    is_visible as _is_visible_impl,
    wait_clickable as _wait_clickable_impl,
    wait_hidden as _wait_hidden_impl,
    wait_visible as _wait_visible_impl,
)

_RPC_ERRORS = (
    xmlrpc.client.Fault,
    ConnectionRefusedError,
    socket.timeout,
    TimeoutError,
    OSError,
    EOFError,
    BrokenPipeError,
)

# --- RPC helpers ---

class _TimeoutTransport(xmlrpc.client.Transport):
    """Transport that sets a per-connection timeout instead of global socket timeout."""

    def __init__(self, timeout=30, **kwargs):
        super().__init__(**kwargs)
        self._timeout = timeout

    def make_connection(self, host):
        if self._connection and host == self._connection[0]:
            return self._connection[1]
        chost, self._extra_headers, x509 = self.get_host_info(host)
        conn = http.client.HTTPConnection(chost, timeout=self._timeout)
        self._connection = host, conn
        return conn


class RobustRpcProxy:
    """XML-RPC proxy with connection recovery.

    Read-only methods are safe to retry: ladder is retry with the same
    proxy, then reconnect (fresh proxy), then retry once more.

    Everything else (mouseClick, command, enterKey, inputText, invokeMethod,
    setStringProperty, quit, ...) is stateful: a timed-out call may already
    have been executed server-side, so blindly retrying it could duplicate
    the action (a double click, a directory added twice, ...). Those calls
    are attempted exactly once; on a transport error the proxy is recreated
    so that the *next* call gets a fresh connection, and the error is raised
    immediately. The sanctioned retry layer for stateful actions is the
    step-level polling helpers (they verify state and re-issue an action
    only when it did not take effect) and the scenario autoretry.
    """

    URL = "http://localhost:9000/"

    READ_ONLY_METHODS = frozenset({
        "getStringProperty",
        "getBoundingBox",
        "existsAndVisible",
        "getErrors",
    })

    def __init__(self):
        self._proxy = self._make_proxy()

    @staticmethod
    def _make_proxy():
        transport = _TimeoutTransport(timeout=30)
        return xmlrpc.client.ServerProxy(
            RobustRpcProxy.URL, transport=transport, allow_none=True
        )

    def __getattr__(self, name):
        def wrapped(*args, **kwargs):
            if name in self.READ_ONLY_METHODS:
                return self._call_read_only(name, args, kwargs)
            return self._call_stateful(name, args, kwargs)
        return wrapped

    def _call_read_only(self, name, args, kwargs):
        # Level 1: retry with current proxy
        try:
            return getattr(self._proxy, name)(*args, **kwargs)
        except xmlrpc.client.Fault:
            # The server processed the read and answered with an error;
            # repeating it would fail the same way.
            raise
        except _RPC_ERRORS:
            pass
        time.sleep(0.5)
        # Level 2: reconnect (fresh proxy)
        self._proxy = self._make_proxy()
        try:
            return getattr(self._proxy, name)(*args, **kwargs)
        except _RPC_ERRORS as e:
            raise ConnectionError(
                f"RPC call {name}() failed after all recovery attempts"
            ) from e

    def _call_stateful(self, name, args, kwargs):
        try:
            return getattr(self._proxy, name)(*args, **kwargs)
        except xmlrpc.client.Fault:
            raise
        except _RPC_ERRORS as e:
            # Transport-level failure: the call may or may not have been
            # executed server-side. Refresh the proxy for subsequent calls,
            # but never re-run the failed call itself.
            self._proxy = self._make_proxy()
            raise ConnectionError(
                f"RPC call {name}() failed; not retried to avoid duplicating "
                f"a possibly executed action"
            ) from e

    def __repr__(self):
        return f"<RobustRpcProxy for {self.URL}>"


# Thin adapters over spix_helpers: this module treats "condition not met"
# as an assertion failure, so its helpers raise instead of returning False.
def _wait_for_visible(rpc, path, timeout=30):
    if not _wait_visible_impl(rpc, path, timeout):
        raise AssertionError(f"Timed out waiting for '{path}' to be visible/opened")


def _wait_for_hidden(rpc, path, timeout=30):
    if not _wait_hidden_impl(rpc, path, timeout):
        raise AssertionError(f"Timed out waiting for '{path}' to be hidden")


# --- Mouse helpers ---

def _click(rpc, path):
    _click_impl(rpc, path)


def _double_click(rpc, path):
    _click(rpc, path)
    time.sleep(0.2)
    _click(rpc, path)


def _right_click(rpc, path):
    assert rpc.existsAndVisible(path), f"{path} cannot be clicked as it does not exists"
    rpc.mouseClickWithButton(path, QT_RIGHT_BUTTON, 0)


def _long_press(rpc, path, button=QT_LEFT_BUTTON, hold_ms=1000):
    rpc.mouseClickAndHold(path, button, 0, hold_ms)


def _is_visible(rpc, path):
    return _is_visible_impl(rpc, path)


def _wait_for_clickable(rpc, path, timeout=5):
    if not _wait_clickable_impl(rpc, path, timeout):
        raise AssertionError(f"Timed out waiting for '{path}' to be clickable")


def _is_column_visible(rpc, col, stabilize_duration=1):
    # Letting time for UI to stabilize
    time.sleep(stabilize_duration)
    index = _column_index(rpc, col)
    if index < 0:
        return False
    columnWidth = float(rpc.invokeMethod(TRACK_TABLE_PATH, "columnWidth", [index]) or 0)
    return columnWidth > 0


def _get_bb(rpc, path):
    bb = rpc.getBoundingBox(path)
    if isinstance(bb, (list, tuple)):
        return {"x": bb[0], "y": bb[1], "width": bb[2], "height": bb[3]}
    return bb


def _scroll_tableview_to_row(rpc, row):
    """Scroll the track TableView so that ``row`` lies inside the visible viewport.

    The TableView recycles its delegates (``reuseItems: true``) and each
    item is a per-column cell named ``trackRow_<row>`` (see Cell.qml). Rows that
    are not currently on screen are never instantiated, so a click targeted at
    ``trackRow_<row>`` resolves to a stale pooled cell or fails outright
    ("Item not found") — the missed click this helper prevents. Driving the
    flickable ``contentY`` scrolls the list the way a user would.
    """
    deadline = time.time() + 15
    last_error = None
    while time.time() < deadline:
        try:
            table_visible = rpc.existsAndVisible(TRACK_TABLE_PATH)
            y = float(rpc.getStringProperty(TRACK_TABLE_PATH, "contentY"))
            height = float(rpc.getStringProperty(TRACK_TABLE_PATH, "contentHeight"))
            viewport = float(rpc.getStringProperty(TRACK_TABLE_PATH, "height")) or 0
        except Exception as e:
            last_error = e
            time.sleep(0.3)
            continue
        if not table_visible or viewport <= 0:
            raise AssertionError(
                "The track table is not visible; the library must be open to "
                "interact with tracks"
            )
        if height <= viewport:
            return
        target = row * _ROW_HEIGHT
        if y <= target and y + viewport >= target + _ROW_HEIGHT:
            return
        new_y = target if target < y else target + _ROW_HEIGHT - viewport
        new_y = max(0.0, min(new_y, height - viewport))
        rpc.setStringProperty(TRACK_TABLE_PATH, "contentY", str(new_y))
        time.sleep(0.3)
    raise AssertionError(
        f"Failed to scroll track table so row {row} is in view"
        + (f" (last error: {last_error})" if last_error else "")
    )


_ROW_HEIGHT = 30


def _track_row_is_on_screen(rpc, row):
    """True when ``row`` is inside the visible TableView viewport.

    Reliable against delegate recycling: derived from the flickable geometry
    rather than a specific delegate instance (whose ``isVisible()`` is flaky
    for table cells).
    """
    y = float(rpc.getStringProperty(TRACK_TABLE_PATH, "contentY"))
    height = float(rpc.getStringProperty(TRACK_TABLE_PATH, "contentHeight"))
    viewport = float(rpc.getStringProperty(TRACK_TABLE_PATH, "height")) or 0
    if viewport <= 0:
        return False
    columnWidth = float(rpc.invokeMethod(TRACK_TABLE_PATH, "columnWidth", [index]) or 0)
    return columnWidth > 0


def _get_bb(rpc, path):
    bb = rpc.getBoundingBox(path)
    if isinstance(bb, (list, tuple)):
        return {"x": bb[0], "y": bb[1], "width": bb[2], "height": bb[3]}
    return bb


def _set_property(rpc, path, prop, value):
    rpc.setStringProperty(path, prop, str(value))


def _get_property(rpc, path, prop):
    return rpc.getStringProperty(path, prop)


def _get_control_value(rpc, group, key):
    return _get_control_value_impl(rpc, group, key)


def _set_control_value(rpc, group, key, value):
    rpc.command("setControlValue", f"{group},{key},{value}")


def _load_track(rpc, deck, filepath):
    rpc.command("loadTrack", f"{deck},{filepath}")


def _library_command(rpc, action, path, scan=False):
    payload = json.dumps({"action": action, "path": path, "scan": scan})
    rpc.command("library", payload)


# --- Path resolution ---

BUTTON_PATHS = {
    "LIBRARY": "mainWindow/library",
    "4DECKS": "mainWindow/show4DecksButton",
    "EDIT": "mainWindow/editDeckButton",
}

LIBRARY_CONTENT = "mainWindow/libraryContent"
TRACKLIST_PATH = f"{LIBRARY_CONTENT}/trackList"
COLUMN_HEADER_PATH = f"{TRACKLIST_PATH}/columnHeader"
COLUMN_PICKER_MENU_PATH = "mainWindow/columnPickerMenu"
TRACK_TABLE_PATH = f"{TRACKLIST_PATH}/trackTableView"
TRACK_ROW_PATH = f"{TRACK_TABLE_PATH}"
TRACK_CONTEXT_MENU_PATH = "mainWindow/trackContextMenu"

# TODO can we auto detect it from the QML file?
TRACK_MENU = [
    ("Load to", [
        ("Deck", [
            ("Deck 1", []),
            ("Deck 2", []),
            ("Deck 3", []),
            ("Deck 4", []),
        ]),
        ("Sampler", []),
    ]),
    ("Add to playlists", []),
    ("Crates", []),
    ("Analyze", []),
]


def _button_path(name):
    return BUTTON_PATHS.get(name, f"mainWindow/{name.lower()}")


def _column_header_path(column):
    return f"{COLUMN_HEADER_PATH}/{column}"


def _column_index(rpc, column):
    """Return the model column index for a display label, or -1 if unknown.

    Resolved from the column model rather than the header delegate, because a
    hidden column has no header delegate to read ``index`` from.
    """
    raw = rpc.getStringProperty(TRACKLIST_PATH, "columnLabels")
    if not raw:
        return -1
    try:
        return json.loads(raw).index(column)
    except (ValueError, json.JSONDecodeError):
        return -1


def _track_row_path(row):
    return f"{TRACK_ROW_PATH}/trackRow_{row}"


DECK_PATHS = {
    1: "mainWindow/deck1",
    2: "mainWindow/deck2",
    3: "mainWindow/deck3",
    4: "mainWindow/deck4",
}

DECK_PATH_MAP = {
    "[Channel1]": "mainWindow/deck1",
    "[Channel2]": "mainWindow/deck2",
    "[Channel3]": "mainWindow/deck3",
    "[Channel4]": "mainWindow/deck4",
}

DECK_GROUPS = {
    1: "[Channel1]",
    2: "[Channel2]",
    3: "[Channel3]",
    4: "[Channel4]",
}

DECK_PROPERTY_MAP = {
    "BPM": "bpm",
    "tempo rate": "rate",
}

DECK_BUTTON_PATHS = {
    "play": "playButton",
    "cue": "cueButton",
    "beatjump_forward": "beatjumpForwardButton",
    "beatjump_backward": "beatjumpBackwardButton",
    "loop_in": "loopIn",
    "loop_out": "loopOut",
    "rate": "rateSlider",
    "reloop_toggle": "reloopToggle",
    "sync": "syncButton",
    "range": "rangeButton",
    "loop_halve": "loopHalve",
    "loop_double": "loopDouble",
}

LOOP_BUTTONS = {
    ("halve", "beatloop"): "loop_halve",
    ("double", "beatloop"): "loop_double",
}

REMEMBERED_CO_KEYS = {
    "beatloop size": "beatloop_size",
    "rate range": "rateRange",
    "playback position": "playposition",
}

TRACK_ACTIONS = {
    "click": _click,
    "double-click": _double_click,
    "right-click": _right_click,
    "long-press": _long_press,
}


def _deck_hotcue_path(deck, hotcue_number):
    return f"{_deck_path(deck)}/hotcue_{hotcue_number}"


def _deck_path(deck):
    return DECK_PATHS.get(deck, f"mainWindow/deck{deck}")


def _deck_group(deck):
    return DECK_GROUPS.get(deck, f"[Channel{deck}]")


def _deck_button_path(deck, button):
    suffix = DECK_BUTTON_PATHS.get(button, button)
    return f"{_deck_path(deck)}/{suffix}"


KNOWN_COLUMNS = [
    "", "Preview", "Title", "Artist", "Album", "AlbumArtist", "Year",
    "Genre", "Composer", "Grouping", "Track Number", "File Type",
    "Comment", "Duration", "Bitrate", "BPM", "ReplayGain", "Key",
    "Color", "Cover", "Rating", "Date Added", "Times Played",
]


# --- Session helpers ---

def _mixxx_running(context):
    mixxx = getattr(context, "mixxx", None)
    if mixxx is None or mixxx.process is None or mixxx.process.poll():
        return False
    try:
        if hasattr(context, "mixxx_rpc") and context.mixxx_rpc:
            # Only checking this isn't an empty string or a False boolean
            return bool(context.mixxx_rpc.getStringProperty("mainWindow", "visible"))
    except Exception as e:
        print(f"Unable to probe RPC: {e}")
    return False


def _mixxx_process_alive(context):
    """True when a Mixxx process this session owns is still running, RPC or not.

    Unlike :func:`_mixxx_running` this deliberately ignores the RPC probe: an
    instance that is stuck starting up, or whose RPC server never came up, is
    exactly the one holding port 9000. Treating the failed RPC probe as "not
    running" used to let such instances leak, and every later fresh-profile
    spawn of the run then timed out against the port they held.
    """
    session = context._session
    for mixxx in (session.get("mixxx"), getattr(context, "mixxx", None)):
        if mixxx is not None and mixxx.process is not None and mixxx.process.poll() is None:
            return True
    return False


def _stop_mixxx(context):
    """Stop every Mixxx process this session owns, RPC-responsive or not.

    The session instance and ``context.mixxx`` can diverge after a failed
    ``start()`` (context then holds the never-ready process), so both are
    reaped. A graceful quit is attempted first when the RPC still answers;
    ``MixxxProcess.stop()`` escalates to SIGKILL, which reliably frees
    port 9000.
    """
    session = context._session
    quit_attempted = False
    for mixxx in (session.get("mixxx"), getattr(context, "mixxx", None)):
        if mixxx is None:
            continue
        if not quit_attempted and mixxx.process is not None and mixxx.process.poll() is None:
            quit_attempted = True
            try:
                rpc = getattr(context, "mixxx_rpc", None)
                if rpc is not None:
                    rpc.quit()
                    time.sleep(1)
            except Exception:
                pass
        mixxx.stop()
    session["mixxx"] = None
    session["rpc"] = None
    session["ready_key"] = None
    session["registered_devices"] = None
    context.mixxx = None
    context.mixxx_rpc = None


def _device_signature(context):
    devices = getattr(context, "_soundMockDevices", None)
    if not devices:
        return None
    return tuple(
        sorted(
            (
                d["name"],
                d.get("api", "Mock"),
                d.get("outputChannels", 0),
                d.get("inputChannels", 0),
            )
            for d in devices
        )
    )


def _mock_devices_key(devices):
    """Canonical form of a scenario's mock-device table, for change detection.

    ``None`` and ``[]`` both normalize to "no devices" so a device-less
    scenario compares equal to an instance with nothing registered.
    """
    return json.dumps(devices or [], sort_keys=True)


def _session_key(context):
    return (getattr(context, "profile_dir", None), _device_signature(context))


def _ensure_profile(context, profile_type, force=False):
    session = context._session
    if session.get("active_profile_type") == profile_type and _mixxx_running(context):
        return
    if _mixxx_process_alive(context):
        # Reap whatever instance we still own before abandoning its profile.
        # An RPC-unresponsive instance must not be mistaken for "not
        # running": it holds port 9000, and leaving it alive makes the next
        # spawn time out and cascade through the rest of the run.
        _stop_mixxx(context)
    profile_dir = profile.make_temp_profile(profile_type)
    context.profile_dir = profile_dir
    context.active_profile_type = profile_type
    session["profile_dir"] = profile_dir
    session["active_profile_type"] = profile_type
    session["profile_dirs"].append(profile_dir)


# --- Given steps ---

@given("a new empty profile")
def step_new_empty_profile(context):
    _ensure_profile(context, "empty")


@given("Mixxx is open and ready to operate")
def step_open_and_ready(context):
    tracks_dir = context.config.userdata["tracks_dir"]
    if not _mixxx_running(context):
        if _mixxx_process_alive(context):
            # A live instance whose RPC probe failed (stuck startup, hung
            # RPC) is the one holding port 9000; reap it before spawning a
            # replacement or the replacement's start() will time out.
            _stop_mixxx(context)
        binary = context.config.userdata["binary"]
        if "profile_dir" not in context:
            _ensure_profile(context, "empty")
        context.mixxx = profile.MixxxProcess(
            binary,
            context.profile_dir,
            output_dir=context.config.userdata.get("mixxx_output_dir"),
        )
        context.mixxx.start()
        context.mixxx_rpc = RobustRpcProxy()
        context._session["mixxx"] = context.mixxx
        context._session["rpc"] = context.mixxx_rpc
        _library_command(context.mixxx_rpc, "addDirectory", tracks_dir, scan=True)

    # FIXME we are forcing the QML reload even on fresh instance because adding a directory on an empty library seems to corrupt the column model on Xcb QP
    context.mixxx_rpc.command("reloadQml", "")
    time.sleep(1)

    _wait_for_visible(context.mixxx_rpc, "mainWindow")
    _wait_for_hidden(context.mixxx_rpc, "mainWindow/splashScreen")
    _wait_for_visible(context.mixxx_rpc, "mainWindow/library")
    context.mixxx_rpc.setStringProperty("mainWindow", "enableDiagnosticClick", "true")
    time.sleep(0.3)

    if "_column_idx" not in context:
        context._column_idx = {
            col: context.mixxx_rpc.getStringProperty(_column_header_path(col), "index")
            for col in KNOWN_COLUMNS
        }
    if "_default_props" not in context:
        context._default_props = {
            "show4DecksButton": {"checked": "false"},
            "editDeckButton": {"checked": "false"},
        }
    if "_remembered" not in context:
        context._remembered = {}


# --- When: window/button steps ---

@when("I resize the window's width to {width:d}px")
def step_resize_window_width(context, width):
    s = context.mixxx_rpc
    _set_property(s, "mainWindow", "width", width)
    time.sleep(0.5)


@when('I check on the button "{button}" in the main toolbar')
def step_check_button(context, button):
    s = context.mixxx_rpc
    path = _button_path(button)
    assert _is_visible(s, path), f"Button {button} (path: {path}) not found or not visible"


@when('I click the "{button}" button')
def step_click_button(context, button):
    s = context.mixxx_rpc
    _click(s, _button_path(button))
    time.sleep(1)


# --- When: deck button steps ---

@when('I click the "{button}" button on deck {deck:d}')
def step_click_deck_button(context, button, deck):
    _click(context.mixxx_rpc, _deck_button_path(deck, button))
    time.sleep(1)


@when('I long-press the "{button}" button on deck {deck:d}')
def step_long_press_deck_button(context, button, deck):
    path = _deck_button_path(deck, button)
    if button == "sync":
        context.mixxx_rpc.invokeMethod(path, "toggleLeader", [])
    else:
        _long_press(context.mixxx_rpc, path)
    time.sleep(0.5)


@when('I {direction} the "{component}" size on deck {deck:d}')
def step_direction_component_size(context, direction, component, deck):
    button = LOOP_BUTTONS.get((direction, component))
    if not button:
        raise ValueError(f"Unknown {direction} for {component}")
    _click(context.mixxx_rpc, _deck_button_path(deck, button))
    time.sleep(0.3)


@when("I seek to {position:f} in deck {deck:d}")
def step_seek_in_deck(context, position, deck):
    group = _deck_group(deck)
    context.mixxx_rpc.mouseClickWithProportion(_deck_button_path(deck, "overview"), position, 0.5)
    time.sleep(0.5)


@when("I set the rate of deck {deck:d} to {value:d}%")
def step_set_rate(context, deck, value):
    path = f'{_deck_button_path(deck, "rate")}/handle'
    bb = context.mixxx_rpc.getBoundingBox(_deck_button_path(deck, "rate"))
    context.mixxx_rpc.mouseDrag(path, 0, 0, 0, bb[3] / 4 * -(value / 100), 1000)


# --- When: hotcue steps ---

@when("I set hotcue {hotcue:d} on deck {deck:d}")
@when("I clear hotcue {hotcue:d} on deck {deck:d}")
def step_toggle_hotcue(context, hotcue, deck):
    _click(context.mixxx_rpc, _deck_hotcue_path(deck, hotcue))
    time.sleep(0.3)


# --- When: loop/remember steps ---

@when('I remember the "{prop}" of deck {deck:d}')
def step_remember(context, prop, deck):
    group = _deck_group(deck)
    key = REMEMBERED_CO_KEYS[prop]
    context._remembered[prop] = float(
        _get_control_value(context.mixxx_rpc, group, key)
    )


# --- When: column steps ---

@when('I click the column header "{column}"')
def step_click_column_header(context, column):
    s = context.mixxx_rpc
    path = _column_header_path(column)
    _click(s, path)
    time.sleep(0.5)


@when('I drag the column "{column}" before the column "{target}"')
def step_drag_column(context, column, target):
    # FIXME not working with column, using bare "moveColumn" instead
    s = context.mixxx_rpc
    column_idx = _column_index(s, column)
    assert column_idx >= 0, f"Column '{column}' cannot be found ({column_idx})"
    target_idx = _column_index(s, target)
    assert target_idx >= 0, f"Column '{target}' cannot be found ({target_idx})"
    s.invokeMethod(TRACK_TABLE_PATH, "moveColumn", [column_idx, target_idx])
    time.sleep(0.5)


@when("I open the column picker menu")
def step_open_column_picker(context):
    s = context.mixxx_rpc
    _right_click(s, _column_header_path("Title"))
    _wait_for_visible(s, COLUMN_PICKER_MENU_PATH)


@when('I toggle the column "{column}" in the column picker')
def step_toggle_column(context, column):
    s = context.mixxx_rpc
    index = _column_index(s, column)
    if index < 0:
        raise KeyError(f'column {column} unknown')
    _get_current_action = lambda: int(context.mixxx_rpc.getStringProperty(COLUMN_PICKER_MENU_PATH, "currentIndex"))
    if sys.platform != "win32":
        if _get_current_action() == -1:
            _click(s, COLUMN_PICKER_MENU_PATH)
            time.sleep(0.3)
        assert _is_visible(s, COLUMN_PICKER_MENU_PATH), "Context menu disappear"
    if _get_current_action() == -1:
        s.enterKey("mainWindow", QT_KEY_DOWN, 0)
        time.sleep(0.3)
    assert _is_visible(s, COLUMN_PICKER_MENU_PATH), "Context menu disappear"
    if _get_current_action() == -1:
        s.enterKey("mainWindow", QT_KEY_TAB, 0)
        time.sleep(0.3)
    assert _is_visible(s, COLUMN_PICKER_MENU_PATH), "Context menu disappear"
    assert _get_current_action() != -1, "Unable to focus the context menu"

    current = _get_current_action()
    delta = int(index) - current
    key = QT_KEY_DOWN if delta > 0 else QT_KEY_UP
    for i in range(abs(delta)):
        s.enterKey("mainWindow", key, 0)
        s.wait(200)
    s.wait(200)
    s.enterKey("mainWindow", QT_KEY_SPACE, 0)
    time.sleep(0.5)


# --- When: track steps ---

def _track_is_selected(rpc, path):
    try:
        return _get_property(rpc, path, "selected") == "true"
    except Exception:
        return False


def _wait_for_track_selected(rpc, path, timeout=5):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if _track_is_selected(rpc, path):
            return True
        time.sleep(0.2)
    return False


def _wait_for_context_menu(rpc, timeout=5):
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            if _is_visible(rpc, TRACK_CONTEXT_MENU_PATH):
                return True
        except Exception:
            pass
        time.sleep(0.2)
    return False


def _perform_track_action(context, action, row):
    s = context.mixxx_rpc
    _scroll_tableview_to_row(s, row)
    path = _track_row_path(row)
    _wait_for_clickable(s, path)
    time.sleep(0.5)
    TRACK_ACTIONS[action](s, path)
    time.sleep(0.3)
    # The synthetic tap that selects a row is occasionally dropped under load
    # (e.g. a CI runner that is also capturing video), so re-click once if the
    # row did not end up selected.
    if action == "click" and not _wait_for_track_selected(s, path):
        _click(s, path)
        time.sleep(0.3)
    elif action == "right-click" and not _wait_for_context_menu(s):
        _right_click(s, path)
        time.sleep(0.3)


@when("I {action} the track at row {row:d}")
def step_track_action(context, action, row):
    TRACK_ACTIONS[action](context.mixxx_rpc, _track_row_path(row))
    time.sleep(0.3)

@when('I select {path} on the track menu')
def step_select_track_menu(context, path):
    s = context.mixxx_rpc
    assert _is_visible(s, TRACK_CONTEXT_MENU_PATH), "Track context menu is not visible"

    steps = list(map(lambda raw: raw.strip()[1:-1], path.split('>')))
    current = TRACK_MENU
    indices = []
    while steps:
        step = steps.pop(0)
        action = [(i, s) for i, s in enumerate(current) if s[0] == step]
        assert len(action) == 1, f"Cannot find {repr(step)} in the current menu or submenu: {current} {action}"
        index, action = action[0]
        indices.append(index)
        current = action[1]

    s.enterKey("mainWindow", QT_KEY_DOWN, 0)
    while indices:
        index = indices.pop(0)
        for i in range(index):
            s.wait(200)
            s.enterKey("mainWindow", QT_KEY_DOWN, 0)
        s.wait(200)
        s.enterKey("mainWindow", QT_KEY_ENTER, 0)
    time.sleep(1)


@given("a track is loaded on deck {deck:d}")
def step_load_track_to_deck(context, deck):
    s = context.mixxx_rpc
    tracks_dir = context.config.userdata["tracks_dir"]
    track_files = sorted(os.listdir(tracks_dir))
    if not track_files:
        raise RuntimeError(f"No track files found in {tracks_dir}")
    idx = random.randint(0, len(track_files) - 1)
    filepath = os.path.join(tracks_dir, track_files[idx])
    _load_track(s, deck, filepath)
    time.sleep(3)


@given("no track is loaded on deck {deck:d}")
def step_no_track_loaded_to_deck(context, deck):
    group = _deck_group(deck)
    if _get_control_value(context.mixxx_rpc, group, "track_loaded"):
        _set_control_value(context.mixxx_rpc, group, "eject", 1.0)
        time.sleep(0.01)
        _set_control_value(context.mixxx_rpc, group, "eject", 0.0)
    time.sleep(0.3)
    assert not _get_control_value(context.mixxx_rpc, group, "track_loaded"), f"Track is still loaded on {group}"


@given("the {prop} on deck {deck:d} is set to {value:f}")
def step_deck_set_rate(context, prop, deck, value):
    group = _deck_group(deck)
    key = DECK_PROPERTY_MAP.get(prop, prop)
    _set_control_value(context.mixxx_rpc, group, key, value)
    time.sleep(0.3)


@given("the sync_on deck {deck:d} is {state}")
def step_deck_set_sync(context, deck, state):
    group = _deck_group(deck)
    _set_control_value(context.mixxx_rpc, group, "sync_enabled", float(state != "disabled"))
    time.sleep(0.3)


# --- When: edit mode steps ---

@when('I move the "{component}" component in deck {deck:d} {cardinality} the "{target}" component')
def step_move_component_after(context, component, deck, cardinality, target):
    component_path = _deck_button_path(deck, component)
    target_path = _deck_button_path(deck, target)

    component_bb = context.mixxx_rpc.getBoundingBox(component_path)
    target_bb = context.mixxx_rpc.getBoundingBox(target_path)
    delta = target_bb[0] - component_bb[0], target_bb[1] - component_bb[1]

    context.mixxx_rpc.mouseDrag(component_path, 0.5, 0.5, *delta, 1000)
    time.sleep(2)


@when('I move the selected group in deck {deck:d} after the "{target}" component')
def step_move_group_after(context, deck, target):
    component_path = _deck_button_path(deck, "selectedGroupOverlay")
    target_path = _deck_button_path(deck, target)

    component_bb = context.mixxx_rpc.getBoundingBox(component_path)
    target_bb = context.mixxx_rpc.getBoundingBox(target_path)
    delta = target_bb[0] - component_bb[0] + target_bb[2]/2, target_bb[1] - component_bb[1]

    context.mixxx_rpc.mouseDrag(component_path, 0, 0, *delta, 1000)
    time.sleep(2)


@when('I select the group containing "{component}" in deck {deck:d} with a {action}')
def step_select_component_group(context, component, deck, action):
    path = _deck_button_path(deck, component)
    if action == "long press":
        context.mixxx_rpc.mouseClickAndHold(path, QT_LEFT_BUTTON, 0, 1000)
    elif action == "ctrl+click":
        context.mixxx_rpc.mouseClickWithButton(path, QT_LEFT_BUTTON, QT_CONTROL_MODIFIER)
    else:
        raise NotImplementedError(f"Unsupported action '{action}'")
    time.sleep(0.3)


# --- Given/When/Then: wait ---

@given("I wait for {second:d} second")
@when("I wait for {second:d} second")
@then("I wait for {second:d} second")
def step_wait(context, second):
    time.sleep(second)


# --- Then: library layout ---

@then("the library is shown for {operator} than {percent:d}% of the Window's height")
def step_library_width(context, operator, percent):
    s = context.mixxx_rpc
    mw = _get_bb(s, "mainWindow")
    lb = _get_bb(s, LIBRARY_CONTENT)
    ratio = float(lb["height"]) / float(mw["height"])
    if operator == "less":
        assert ratio < percent / 100.0, f"Library covers {ratio*100:.1f}%, expected less than {percent}%"
    else:
        assert ratio > percent / 100.0, f"Library covers {ratio*100:.1f}%, expected more than {percent}%"


# --- Then: column visibility ---

@then('only the column "{columns}" are shown')
def step_only_columns_shown(context, columns):
    s = context.mixxx_rpc
    expected = [c.strip() for c in re.split(r"[,\.]", columns)]
    for col in expected:
        if not col:
            continue
        assert _is_column_visible(s, col, 0), (
            f"Column '{col}' should be visible but is not"
        )
    for col in KNOWN_COLUMNS:
        if col and col not in expected:
            assert not _is_column_visible(s, col, 0), (
                f"Column '{col}' should not be visible but is"
            )


@then('the column "{column}" should {state} visible')
def step_column_visible(context, column, state):
    s = context.mixxx_rpc
    requested = "not" not in state
    currentState = "not" if requested else "still"
    assert _is_column_visible(s, column) is requested, f"Column {column} is {currentState} visible"


@then('the column "{column}" should appear before the column "{other}"')
def step_column_order(context, column, other):
    s = context.mixxx_rpc
    col_bb = _get_bb(s, _column_header_path(column))
    other_bb = _get_bb(s, _column_header_path(other))
    assert col_bb["x"] < other_bb["x"], (
        f"Column {column} (x={col_bb['x']}) is not before {other} (x={other_bb['x']})"
    )


# --- Then: sort order ---

@then('the results should be sorted by "{column}" in "{order}" order')
def step_sorted_by(context, column, order):
    s = context.mixxx_rpc
    column_idx = _column_index(s, column)
    assert column_idx >= 0, f"Unknown column: {column}"
    sort_col = _get_property(s, COLUMN_HEADER_PATH, "sortingColumn")
    assert sort_col == str(column_idx), (
        f"Expected results sorted by '{column}' (column index {column_idx}) "
        f"but sortingColumn is {sort_col}"
    )
    assert order in ("ascending", "descending"), f"Unknown order: {order}"
    expected_order = "0" if order == "ascending" else "1"
    sort_order = _get_property(s, COLUMN_HEADER_PATH, "sortingOrder")
    assert sort_order == expected_order, (
        f"Expected {order} sort (Qt value {expected_order}), got {sort_order}"
    )


# --- Then: track selection/context ---

@then("the track at row {row:d} should be selected")
def step_track_selected(context, row):
    s = context.mixxx_rpc
    path = _track_row_path(row)
    selected = _get_property(s, path, "selected")
    assert selected == "true", f"Track at row {row} is not selected (selected={selected})"


@then("the track context menu should be visible")
def step_track_context_menu_visible(context):
    s = context.mixxx_rpc
    assert _is_visible(s, TRACK_CONTEXT_MENU_PATH), "Track context menu is not visible"


# --- Then: deck visibility ---

@then('the deck for "{group}" should {assertion} visible')
def step_deck_visible(context, group, assertion):
    if assertion == "be":
        _wait_for_visible(context.mixxx_rpc, DECK_PATH_MAP[group])
    else:
        _wait_for_hidden(context.mixxx_rpc, DECK_PATH_MAP[group])


# --- Then: deck state ---

@then("the deck {deck:d} should be {assertion}")
def step_deck_playing(context, deck, assertion):
    group = _deck_group(deck)
    expected = assertion == "playing"
    value = bool(_get_control_value(context.mixxx_rpc, group, "play"))
    label = lambda x: 'playing' if x else 'stopped'
    assert value is expected, (
        f"Play button on deck {deck} ({group}) is {label(value)} but expected {label(expected)}"
    )

@then("deck {deck_a:d} {prop} should be equal to deck {deck_b:d}")
def step_deck_property_equal(context, deck_a, prop, deck_b):
    group_a = _deck_group(deck_a)
    group_b = _deck_group(deck_b)
    key = DECK_PROPERTY_MAP.get(prop, prop)
    current = _get_control_value(context.mixxx_rpc, group_a, key)
    expected = _get_control_value(context.mixxx_rpc, group_b, key)
    assert current == expected, (
        f"{prop} on deck {deck_a} ({group_a}) differs from {deck_b} ({group_b}): {current} != {expected}"
    )

@then("the play button on deck {deck:d} should be {assertion}")
def step_play_pressed(context, deck, assertion):
    group = _deck_group(deck)
    expected = assertion == "pressed"
    value = context.mixxx_rpc.getStringProperty(_deck_button_path(deck, "play"), "highlight") == "true"
    label = lambda x: 'pressed' if x else 'released'
    assert value is expected, (
        f"Play button on deck {deck} ({group}) is {label(value)} but expected {label(expected)}"
    )


@then("the cue point should be set on deck {deck:d}")
def step_cue_set(context, deck):
    group = _deck_group(deck)
    value = _get_control_value(context.mixxx_rpc, group, "cue_point")
    assert float(value) > 0, f"cue_point not set (value={value})"


# --- Then: hotcue ---

@then("hotcue {hotcue:d} should {assertion} set on deck {deck:d}")
def step_hotcue_assertion(context, hotcue, assertion, deck):
    group = _deck_group(deck)
    expected = assertion == "be"
    value = float(_get_control_value(context.mixxx_rpc, group, f"hotcue_{hotcue}_status"))
    assert (value > 0) == expected, (
        f"hotcue_{hotcue} {'not set' if expected else 'still set'} (value={value})"
    )


# --- Then: loop ---

@then("the loop should {assertion} enabled on deck {deck:d}")
def step_loop_assertion(context, assertion, deck):
    group = _deck_group(deck)
    expected = assertion == "be"
    value = float(_get_control_value(context.mixxx_rpc, group, "loop_enabled"))
    assert (value > 0) == expected, (
        f"Loop on deck {deck} ({group}) is {'not enabled' if expected else 'still enabled'} (loop_enabled={value})"
    )


@then('the "{prop}" of deck {deck:d} should have changed')
def step_value_changed(context, prop, deck):
    group = _deck_group(deck)
    key = REMEMBERED_CO_KEYS[prop]
    current = float(_get_control_value(context.mixxx_rpc, group, key))
    saved = context._remembered.get(prop)
    assert saved is not None, f"No saved {prop} (forgot 'I remember' step?)"
    assert current != saved, (
        f"{prop} did not change on deck {deck} ({group}): "
        f"was {saved}, is {current}"
    )
    context._remembered[prop] = current


# --- Then: sync ---

@then("sync should be {assertion} on deck {deck:d}")
def step_sync_assertion(context, assertion, deck):
    group = _deck_group(deck)
    value = float(_get_control_value(context.mixxx_rpc, group, "sync_enabled"))
    expected = assertion == "enabled"
    assert (value > 0) == expected, (
        f"Sync on deck {deck} ({group}) is {'not ' if expected else ''}enabled (value={value})"
    )


@then("deck {deck:d} should be the sync leader")
def step_sync_leader(context, deck):
    group = _deck_group(deck)
    leader = _get_control_value(context.mixxx_rpc, group, "sync_leader")
    assert float(leader) > 0, (
        f"Deck {deck} ({group}) is not the sync leader (sync_leader={leader})"
    )


# --- Then: rate ---

@then("the rate of deck {deck:d} should be near {target}")
def step_rate_near(context, deck, target):
    group = _deck_group(deck)
    actual = float(_get_control_value(context.mixxx_rpc, group, "rate"))
    tol = 0.01
    assert abs(actual - float(target)) < tol, (
        f"Rate on deck {deck} ({group}) is {actual}, expected ~{target}"
    )


@then("the rate ratio of deck {deck:d} should be near {target}")
def step_rate_ratio_near(context, deck, target):
    group = _deck_group(deck)
    actual = float(_get_control_value(context.mixxx_rpc, group, "rate_ratio"))
    tol = 0.01
    assert abs(actual - float(target)) < tol, (
        f"Rate ratio on deck {deck} ({group}) is {actual}, expected ~{target}"
    )


# --- Then: edit mode ---

@then("edit mode should {assertion} enabled")
def step_edit_mode_assertion(context, assertion):
    checked = _get_property(context.mixxx_rpc, "mainWindow/editDeckButton", "checked")
    expected = assertion == "be"
    assert (checked == "true") == expected, (
        f"Edit mode {'not enabled' if expected else 'still enabled'} (checked={checked})"
    )


@then('the edit overlay should {action} visible on the "{component}" component in deck {deck:d}')
def step_edit_overlay_visible(context, action, component, deck):
    path = f"{_deck_button_path(deck, component)}Item/overlayItem"
    if action == "be":
        assert _is_visible(context.mixxx_rpc, path), f"Component '{component}' is not visible"
    else:
        assert not _is_visible(context.mixxx_rpc, path), f"Component '{component}' is visible"


@then('the "{component}" component should appear {cardinality} the "{target}" component in deck {deck:d}')
def step_component_order_after(context, component, cardinality, target, deck):
    cmp = lambda a, b: a[0] > b[0]
    if cardinality == "before":
        cmp = lambda a, b: a[0] < b[0]
    elif cardinality == "under":
        cmp = lambda a, b: a[1] > b[1]
    elif cardinality == "above":
        cmp = lambda a, b: a[1] < b[1]
    component_bb = context.mixxx_rpc.getBoundingBox(_deck_button_path(deck, component))
    target_bb = context.mixxx_rpc.getBoundingBox(_deck_button_path(deck, target))
    assert cmp(component_bb, target_bb), (
        f"Component {component} (x={component_bb[0]}) is not {cardinality} {target} (x={target_bb[0]})"
    )

# --- Then: track loading ---

@then("a track {assertion} loaded on deck {deck:d}")
def step_track_is_loaded_on_deck(context, assertion, deck):
    group = _deck_group(deck)
    expected = assertion == "is"
    reverse = "is not" if expected else "is"
    assert bool(_get_control_value(context.mixxx_rpc, group, "track_loaded")) is expected, f"Track {reverse} loaded on {group}"

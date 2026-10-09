import http.client
import json
import os
import sys
import re
import time
import functools
import socket
import dataclasses
import xmlrpc.client
from typing import Optional
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

    def __init__(self, timeout=15, **kwargs):
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
        transport = _TimeoutTransport(timeout=15)
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
def _wait_for_visible(rpc, path, timeout=15):
    if not _wait_visible_impl(rpc, path, timeout):
        raise AssertionError(f"Timed out waiting for '{path}' to be visible/opened")


def _wait_for_hidden(rpc, path, timeout=15):
    if not _wait_hidden_impl(rpc, path, timeout):
        raise AssertionError(f"Timed out waiting for '{path}' to be hidden")


# --- Static path resolution ---

BUTTON_PATHS = {
    "LIBRARY": "mainWindow/library",
    "4DECKS": "mainWindow/show4DecksButton",
    "EDIT": "mainWindow/editDeckButton",
    "PREFERENCES": "mainWindow/showPreferencesButton",
}

LIBRARY_CONTENT = "mainWindow/libraryContent"
TRACKLIST_PATH = f"{LIBRARY_CONTENT}/trackList"
COLUMN_HEADER_PATH = f"{TRACKLIST_PATH}/columnHeader"
COLUMN_PICKER_MENU_PATH = "mainWindow/columnPickerMenu"
TRACK_TABLE_PATH = f"{TRACKLIST_PATH}/trackTableView"
TRACK_ROW_PATH = f"{TRACK_TABLE_PATH}"
TRACK_CONTEXT_MENU_PATH = "mainWindow/trackContextMenu"


# --- Mouse helpers ---

def _click(rpc, path):
    _click_impl(rpc, path)


def _double_click(rpc, path):
    _click(rpc, path)
    time.sleep(0.2)
    _click(rpc, path)


def _right_click(rpc, path):
    assert rpc.existsAndVisible(path), f"{path} cannot be clicked as it does not exists"
    # Use a short hold rather than a plain click so the synthetic press is
    # fully delivered before the release, which makes Qt's TapHandler
    # recognise the right-button tap even under load.
    rpc.mouseClickAndHold(path, QT_RIGHT_BUTTON, 0, 250)


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


def _scroll_tableview_to_row(rpc, row, table=TRACK_TABLE_PATH):
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
            table_visible = rpc.existsAndVisible(table)
            y = float(rpc.getStringProperty(table, "contentY"))
            height = float(rpc.getStringProperty(table, "contentHeight"))
            viewport = float(rpc.getStringProperty(table, "height")) or 0
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
        rpc.setStringProperty(table, "contentY", str(new_y))
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
    if height <= viewport:
        return True
    target = row * _ROW_HEIGHT
    return y <= target and y + viewport >= target + _ROW_HEIGHT


def _last_track_row(rpc, timeout=5):
    """Index of the last row in the track table, derived from its content height.

    Keeps the "below the fold" scenario independent of the number of tracks
    that happen to be in the database.
    """
    deadline = time.time() + timeout
    while time.time() < deadline:
        height = float(rpc.getStringProperty(TRACK_TABLE_PATH, "contentHeight"))
        row = int(height // _ROW_HEIGHT) - 1
        if row >= 0:
            return row
        time.sleep(0.3)
    raise AssertionError("The track table has no rows")


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


def _get_library_state(rpc):
    rpc.command("getLibraryState", "")
    return json.loads(rpc.getStringProperty("mainWindow", "lastLibraryState"))


def _wait_for_library_scan(rpc, scan_generation, timeout=20):
    """Wait for a library scan triggered after ``scan_generation`` to finish.

    The ``library`` command starts the scan asynchronously and returns
    immediately (running a nested event loop while the library models are
    mutated crashes the QML delegate model). The ``getLibraryState`` command
    exposes a monotonically increasing ``scanGeneration`` counter and a
    ``scanInProgress`` flag so tests can wait without blocking the app's main
    thread.
    """
    deadline = time.time() + timeout
    last_state = None
    while time.time() < deadline:
        try:
            last_state = _get_library_state(rpc)
            if (last_state.get("scanGeneration", 0) > scan_generation and
                    not last_state.get("scanInProgress", False)):
                return
        except Exception as e:
            last_state = e
        time.sleep(0.2)
    raise AssertionError(
        f"Timed out waiting for the library scan to finish "
        f"(last state: {last_state})"
    )


def _library_command(rpc, action, path, scan=False):
    payload = json.dumps({"action": action, "path": path, "scan": scan})
    scan_generation = None
    if scan:
        scan_generation = _get_library_state(rpc).get("scanGeneration", 0)
    rpc.command("library", payload)
    if scan:
        _wait_for_library_scan(rpc, scan_generation)


# --- Path resolution ---

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
    "beatjump": "beatjump",
    "beatjump_forward": "beatjumpForwardButton",
    "beatjump_backward": "beatjumpBackwardButton",
    "hotcue": "hotcueAndStem",
    "loop_in": "loopIn",
    "loop_out": "loopOut",
    "rate": "rateSlider",
    "reloop_toggle": "reloopToggle",
    "spinny": "spinny",
    "sync": "syncButton",
    "range": "rangeButton",
    "loop_halve": "loopHalve",
    "loop_double": "loopDouble",
    "tempo": "rateSlider",
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

QT_KEY_ESCAPE = 0x01000000
QT_KEY_DELETE = 0x01000007


# --- Library search / split view paths ---

SEARCH_PANE_PATH = f"{LIBRARY_CONTENT}/browsingView/searchPane"
SEARCH_FIELD_PATH = f"{SEARCH_PANE_PATH}/searchField"
SEARCH_CLEAR_BUTTON_PATH = f"{SEARCH_PANE_PATH}/searchClearButton"
SEARCH_SUGGESTION_LIST_PATH = f"{SEARCH_PANE_PATH}/searchSuggestionList"
SEARCH_SUGGESTION_FIELD_LIST_PATH = f"{SEARCH_PANE_PATH}/searchSuggestionFieldList"
SEARCH_RECENT_LIST_PATH = f"{SEARCH_PANE_PATH}/searchRecentList"
SPLIT_VIEW_BUTTON_PATH = f"{LIBRARY_CONTENT}/tracklistMenu/splitViewButton"
RIGHT_TRACKLIST_PATH = f"{LIBRARY_CONTENT}/rightTrackList"
RIGHT_TRACKLIST_TABLE_PATH = f"{RIGHT_TRACKLIST_PATH}/trackTableView"

def _suggestion_path(suggestion):
    """Path of a suggestion entry, based on which list shows it.

    Field suggestions ("Artist:") live in searchSuggestionFieldList; value
    suggestions live in searchSuggestionList.
    """
    if suggestion.endswith(":"):
        return f"{SEARCH_SUGGESTION_FIELD_LIST_PATH}/suggestion_{suggestion}"
    return f"{SEARCH_SUGGESTION_LIST_PATH}/suggestion_{suggestion}"


# Delay after typing into the search bar: the query is applied through a
# debouncing timer (searchDebounce, 800 ms in res/qml/Library.qml).
SEARCH_APPLY_DELAY = 1.5


def _search_activated(rpc):
    return _get_property(rpc, SEARCH_PANE_PATH, "activated") == "true"


def _activate_library_search(context, timeout=15):
    # AGENTS.md click-retry exception: the collapsed bar opens via TapHandler
    # and spix synthetic taps on TapHandlers can be dropped silently; every
    # retry is verified against the pane's activated state.
    s = context.mixxx_rpc
    deadline = time.time() + timeout
    while time.time() < deadline:
        if _search_activated(s):
            return
        _click(s, SEARCH_PANE_PATH)
        time.sleep(0.3)
    assert _search_activated(s), "Library search bar did not open"


def _deactivate_library_search(context, timeout=5):
    # Escape triggers persistSearch() + deactivateSearch() in the search bar's
    # key handler, which is the deterministic way to give up focus. Re-Escape
    # is needed after a dropped synthetic tap re-opened the bar.
    s = context.mixxx_rpc
    deadline = time.time() + timeout
    while time.time() < deadline:
        if not _search_activated(s):
            return
        s.enterKey("mainWindow", QT_KEY_ESCAPE, 0)
        time.sleep(0.3)
    assert not _search_activated(s), "Library search bar is still activated"


def _track_rows(rpc, tracklist_path):
    table = f"{tracklist_path}/trackTableView"
    height = float(rpc.getStringProperty(table, "contentHeight") or 0)
    return max(0, int(height // _ROW_HEIGHT))


def _track_row_by_title(rpc, tracklist_path, title, timeout=5):
    """Index of the first row whose displayed title matches, or -1."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        rows = _track_rows(rpc, tracklist_path)
        for row in range(rows):
            track_title = str(
                rpc.invokeMethod(tracklist_path, "trackTitleForRow", [row]) or ""
            )
            if track_title == title:
                return row
        time.sleep(0.3)
    return -1


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
    if not force and session.get("active_profile_type") == profile_type and _mixxx_running(context):
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

@given("a {profile_type} profile")
def step_new_empty_profile(context, profile_type):
    # The Gherkin text is "a fresh new empty profile", so {profile_type} binds
    # to "fresh new empty" -- the leading "a " belongs to the pattern, not the
    # capture. Testing for "a fresh" here could therefore never match, which
    # silently disabled the fresh-profile opt-out and made every scenario
    # reuse the first scenario's profile and library.
    _ensure_profile(
        context,
        profile_type.split(" ")[-1],
        force=profile_type.startswith("fresh"),
    )


@given("Mixxx is open and ready to operate")
def step_open_and_ready(context):
    session = context._session
    if session.get("ready_key") == _session_key(context) and _mixxx_running(context):
        return

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
        session["mixxx"] = context.mixxx
        session["rpc"] = context.mixxx_rpc
        # A fresh instance has no mock devices registered; the sync below
        # must know that to avoid a pointless reload on device-less spawns.
        session["registered_devices"] = _mock_devices_key(None)
        _wait_for_hidden(context.mixxx_rpc, "mainWindow/splashScreen")
        if context.active_profile_type == "library-ready":
            _library_command(context.mixxx_rpc, "addDirectory", tracks_dir, scan=True)

    # Sync the mock devices with the scenario's device table. Registering or
    # clearing devices forces a reloadQml (full QML rebuild) because the
    # sound pages cache the device list and do not always pick up device
    # changes on their own — even on a freshly spawned instance. That cost
    # is only justified when the requested device set actually differs from
    # what the running instance already has registered; same-devices
    # scenarios reuse the instance untouched.
    desired_devices = _mock_devices_key(getattr(context, "_soundMockDevices", None))
    if session.get("registered_devices") != desired_devices:
        context.mixxx_rpc.command("clearMockDevices", "")
        if getattr(context, "_soundMockDevices", None):
            context.mixxx_rpc.command(
                "registerMockDevices",
                json.dumps({"devices": context._soundMockDevices}),
            )
        context.mixxx_rpc.command("reloadQml", "")
        session["registered_devices"] = desired_devices
    time.sleep(0.5)

    if "_remembered" not in context:
        context._remembered = {}

    _wait_for_visible(context.mixxx_rpc, "mainWindow/library")
    _wait_for_hidden(context.mixxx_rpc, "mainWindow/splashScreen")
    context.mixxx_rpc.setStringProperty("mainWindow", "enableDiagnosticClick", "true")
    session["ready_key"] = _session_key(context)


# --- When: window/button steps ---

@given("the window's width is {width:d}px")
def step_window_width_is(context, width):
    step_resize_window_width(context, width)


@given("the window's height is {height:d}px")
def step_window_height_is(context, height):
    step_resize_window_height(context, height)


@given("the window size is default")
def step_window_size_default(context):
    s = context.mixxx_rpc
    _set_property(s, "mainWindow", "width", 1792)
    _set_property(s, "mainWindow", "height", 1008)
    time.sleep(0.5)


@given("the library columns are in their default state")
def step_library_columns_default(context):
    context.mixxx_rpc.invokeMethod(TRACKLIST_PATH, "resetColumns", [])
    time.sleep(0.5)


@when("I resize the window's width to {width:d}px")
def step_resize_window_width(context, width):
    s = context.mixxx_rpc
    _set_property(s, "mainWindow", "width", width)
    time.sleep(0.5)


@when("I resize the window's height to {height:d}px")
def step_resize_window_height(context, height):
    s = context.mixxx_rpc
    _set_property(s, "mainWindow", "height", height)
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
    _get_current_action = lambda: int(s.getStringProperty(COLUMN_PICKER_MENU_PATH, "currentIndex"))
    # Needed if QPA == xcb
    # if _get_current_action() == -1:
    #     _click(s, COLUMN_PICKER_MENU_PATH)
    #     time.sleep(0.3)
    # assert _is_visible(s, COLUMN_PICKER_MENU_PATH), "Context menu disappear"
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


def _perform_track_action(context, action, row, table=TRACK_TABLE_PATH):
    s = context.mixxx_rpc
    _scroll_tableview_to_row(s, row, table)
    path = f"{table}/trackRow_{row}"
    _wait_for_clickable(s, path, timeout=10) # Some CI platform like Windows 10 are very slow to load the table
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
@when("I {action} the track at row {row:d} on the {panel} track list")
def step_track_action(context, action, row, panel="left"):
    _perform_track_action(context, action, row, RIGHT_TRACKLIST_TABLE_PATH if panel == "right" else TRACK_TABLE_PATH)


@when("I click a track below the fold")
def step_click_track_below_fold(context):
    row = _last_track_row(context.mixxx_rpc)
    context._fold_track_row = row
    _perform_track_action(context, "click", row)

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
    idx = scenario_rng(context).randint(0, len(track_files) - 1)
    filepath = os.path.join(tracks_dir, track_files[idx])
    # Remember the loaded track as "deck N" with catalog metadata when the
    # file is known, so later steps can pin and assert on its tags.
    catalog = context.config.userdata.get("tracks_catalog") or []
    entry = next(
        (e for e in catalog if e.get("location") == filepath), None)
    if entry is None:
        entry = {"title": os.path.splitext(track_files[idx])[0],
                 "location": filepath}
    remember_track(context, entry, name=f"deck {deck}")
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


@then("the track at row {row:d} should be visible on screen")
def step_track_on_screen(context, row):
    s = context.mixxx_rpc
    assert _track_row_is_on_screen(s, row), (
        f"Track at row {row} was not scrolled into the visible viewport"
    )


@then("the track below the fold should be visible on screen")
def step_fold_track_on_screen(context):
    row = getattr(context, "_fold_track_row", None)
    assert row is not None, "No track below the fold was clicked"
    s = context.mixxx_rpc
    assert _track_row_is_on_screen(s, row), (
        f"Track at row {row} was not scrolled into the visible viewport"
    )


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


@then('the library should {assertion} visible')
def step_library_visible(context, assertion):
    if assertion == "be":
        _wait_for_visible(context.mixxx_rpc, LIBRARY_CONTENT)
    else:
        _wait_for_hidden(context.mixxx_rpc, LIBRARY_CONTENT)


@given("the library is not maximized")
def step_library_not_maximized(context):
    _set_control_value(context.mixxx_rpc, "[Skin]", "show_maximized_library", 0)
    time.sleep(0.5)


@given("the library is maximized")
def step_library_maximized(context):
    _set_control_value(context.mixxx_rpc, "[Skin]", "show_maximized_library", 1)
    _wait_for_visible(context.mixxx_rpc, LIBRARY_CONTENT)


@then('the "{button}" button in the main toolbar should {assertion} visible')
def step_toolbar_button_visible(context, button, assertion):
    path = _button_path(button)
    if assertion == "be":
        _wait_for_visible(context.mixxx_rpc, path)
    else:
        _wait_for_hidden(context.mixxx_rpc, path)


@then('the "{component}" component should {assertion} visible in deck {deck:d}')
def step_deck_component_visible(context, component, assertion, deck):
    path = _deck_button_path(deck, component)
    if assertion == "be":
        _wait_for_visible(context.mixxx_rpc, path)
    else:
        _wait_for_hidden(context.mixxx_rpc, path)


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




@when("I dump the library debug state")
def step_dump_debug(context):
    s = context.mixxx_rpc
    for probe in (
        "mainWindow/libraryContent/trackList",
        "mainWindow/libraryContent/trackList/columnHeader",
        "mainWindow/libraryContent/trackList/trackTableView",
        "mainWindow/libraryContent/browsingView",
        "mainWindow/libraryContent/libraryContent/trackList",
        "mainWindow/libraryContent/trackList/columnHeader/Title",
        "mainWindow/splashScreen",
    ):
        try:
            vis = s.existsAndVisible(probe)
        except Exception as e:
            e_str = str(e)[:80]
            print(f"probe {probe}: ERR {e_str}")
            continue
        print(f"probe {probe}: {vis}")
    print("columnLabels:", s.getStringProperty(
        "mainWindow/libraryContent/trackList", "columnLabels"))
    for r in range(3):
        print("title", r, "=>", repr(s.invokeMethod(
            "mainWindow/libraryContent/trackList", "trackTitleForRow", [r])))
    try:
        state = _get_library_state(s)
        print("library state:", json.dumps(state)[:300])
    except Exception as e:
        print("library state err", e)


# --- When: library search ---

def _type_into_library_search(context, text):
    _activate_library_search(context)
    s = context.mixxx_rpc
    # inputText() appends to the text that is already in the field, so clear
    # it first; typing goes through real key events so the search bar's
    # onTextEdited handlers run (setting the `text` property directly would
    # bypass them).
    _set_property(s, SEARCH_FIELD_PATH, "text", "")
    time.sleep(0.3)
    s.inputText(SEARCH_FIELD_PATH, text)
    time.sleep(SEARCH_APPLY_DELAY)


@when("I activate the library search")
def step_activate_library_search(context):
    _activate_library_search(context)


@when("I deactivate the library search")
def step_deactivate_library_search(context):
    _deactivate_library_search(context)


@when('I type "{text}" into the library search')
def step_type_library_search(context, text):
    _type_into_library_search(context, text)


@when('I paste "{query}" into the library search')
def step_paste_library_search(context, query):
    """Entry of a whole query in one edit; text is set via RPC and
    tryConvertToFieldToken() is called explicitly, mirroring the sidebar
    restore in Library.qml (setting text does not fire onTextEdited)."""
    s = context.mixxx_rpc
    query = re.sub(
            r"<(\w+) of this track>",
            lambda match: str(_this_track_field(context, match.group(1))),
            query)
    _activate_library_search(context)
    _set_property(s, SEARCH_FIELD_PATH, "text", query)
    time.sleep(0.3)
    s.invokeMethod(SEARCH_PANE_PATH, "tryConvertToFieldToken", [])
    time.sleep(SEARCH_APPLY_DELAY)


@when("I clear the library search")
def step_clear_library_search(context):
    s = context.mixxx_rpc
    _click(s, SEARCH_CLEAR_BUTTON_PATH)
    _wait_for_hidden(s, SEARCH_CLEAR_BUTTON_PATH, 5)


@when('I select the library search suggestion "{suggestion}"')
@then('I select the library search suggestion "{suggestion}"')
def step_select_suggestion(context, suggestion):
    s = context.mixxx_rpc
    _click(s, _suggestion_path(suggestion))
    time.sleep(0.5)


# --- When: split view ---

@when("I click the split view button")
def step_click_split_view(context):
    _click(context.mixxx_rpc, SPLIT_VIEW_BUTTON_PATH)
    time.sleep(1)



# --- Then: library search ---

@then("the library search bar should be visible")
def step_library_search_bar_visible(context):
    assert _search_activated(context.mixxx_rpc), "Library search bar is not open"


@then("this track should be visible in the {side} track list")
def step_this_track_visible_in_side_list(context, side):
    track = _remembered_track(context)
    tracklist = RIGHT_TRACKLIST_PATH if side == "right" else TRACKLIST_PATH
    _wait_for_visible(context.mixxx_rpc, tracklist)
    found = _track_row_by_title(context.mixxx_rpc, tracklist, track.title)
    assert found >= 0, f"Track '{track}' is not in the {side} track list"


@then("no other track should be visible in the results")
def step_no_other_track_in_results(context, ):
    """Typing a full track title keeps only the matching row(s)."""
    s = context.mixxx_rpc
    assert _track_rows(s, TRACKLIST_PATH) <= 1, (
        f"{_track_rows(s, TRACKLIST_PATH)} tracks are shown, expected only the typed one")


@then("all tracks in the library should be shown again")
def step_all_tracks_shown_again(context, ):
    s = context.mixxx_rpc
    total = _get_library_state(s).get("visibleTrackCount", 0)
    deadline = time.time() + 5
    rows = -1
    while time.time() < deadline:
        rows = _track_rows(s, TRACKLIST_PATH)
        if rows == total:
            return
        time.sleep(0.3)
    raise AssertionError(
            f"{rows} tracks are shown, expected all {total}")


@then("all tracks in the library should be shown without a search filter")
def step_all_tracks_shown_without_filter(context):
    step_all_tracks_shown_again(context)


@then('the library search suggestion "{suggestion}" should be visible')
def step_suggestion_visible(context, suggestion):
    s = context.mixxx_rpc
    # While a query is being typed, the suggestion list replaces the recent
    # searches, so the recents must have been dismissed first.
    _wait_for_hidden(s, SEARCH_RECENT_LIST_PATH)
    _wait_for_visible(s, _suggestion_path(suggestion))


@then('the library search suggestion "{suggestion}" should not be visible')
def step_suggestion_not_visible(context, suggestion):
    s = context.mixxx_rpc
    path = _suggestion_path(suggestion)
    time.sleep(SEARCH_APPLY_DELAY)
    assert not _is_visible(s, path), (
        f'The field suggestion "{suggestion}" is visible')


@then('the library recent search "{needle}" should be visible')
def step_recent_search_visible(context, needle):
    s = context.mixxx_rpc
    expected = _remembered_track(context).title if "title of this track" in needle else needle
    _wait_for_visible(s, SEARCH_RECENT_LIST_PATH)
    recent_path = f"{SEARCH_RECENT_LIST_PATH}/recent_{expected}"
    _wait_for_visible(s, recent_path)
    # Verify that token labels are correctly shown (not empty strings)
    token_field_names = _get_property(s, recent_path, "tokenFieldNames")
    if not token_field_names:
        return
    fields = json.loads(token_field_names)
    for i, field in enumerate(fields):
        assert field, (
            f"Recent search token {i} has empty label "
            f"(fields={fields}, recent_path={recent_path})"
        )


@then('a search token "{field}" should be shown in the search bar')
def step_search_token_shown(context, field):
    s = context.mixxx_rpc
    count = int(_get_property(s, SEARCH_PANE_PATH, "criteriaCount") or 0)
    assert count > 0, f"Search bar has no criteria tokens ({count})"
    fields = _search_criteria_fields(s)
    assert field in fields, (
        f"Search bar has no token for '{field}' (criteria: {fields})"
    )


# --- Then: split view ---

@then('the track list on the right should {assertion} visible')
def step_right_tracklist_visible(context, assertion):
    s = context.mixxx_rpc
    if assertion == "be":
        _wait_for_visible(s, RIGHT_TRACKLIST_PATH)
    else:
        _wait_for_hidden(s, RIGHT_TRACKLIST_PATH)


@then('the track at row {row:d} on the right track list should be selected')
def step_track_selected_right_list(context, row):
    s = context.mixxx_rpc
    path = f"{RIGHT_TRACKLIST_PATH}/trackTableView/trackRow_{row}"
    selected = _get_property(s, path, "selected")
    assert selected == "true", f"Track at row {row} is not selected (selected={selected})"


# --- Library search: token-based criteria ---

QT_KEY_BACKSPACE = 0x01000003
QT_KEY_LEFT = 0x01000012
QT_KEY_RIGHT = 0x01000014
QT_KEY_F = 0x46

# Human-readable key names of the search-bar scenarios, mapped to Qt key
# codes.
_SEARCH_KEYS = {
    "Escape": QT_KEY_ESCAPE,
    "Tab": QT_KEY_TAB,
    "Backspace": QT_KEY_BACKSPACE,
    "Enter": QT_KEY_ENTER,
    "Delete": QT_KEY_DELETE,
    "Left": QT_KEY_LEFT,
    "Up": QT_KEY_UP,
    "Right": QT_KEY_RIGHT,
    "Down": QT_KEY_DOWN,
    # "Ctrl+F" is a special case: it keys on a window-level Shortcut and is
    # delivered as a (Qt key code, Qt modifier) pair.
    "Ctrl+F": (QT_KEY_F, QT_CONTROL_MODIFIER),
}


def _search_criteria_fields(rpc):
    """Field names of the criteria tokens in insertion order (JSON list)."""
    raw = _get_property(rpc, SEARCH_PANE_PATH, "criteriaFields") or "[]"
    try:
        return json.loads(raw)
    except (TypeError, ValueError):
        return []


@dataclasses.dataclass(frozen=True)
class RememberedTrack:
    """A tracks-catalog entry remembered by an earlier step under a name.

    Scenarios reference it as "this track" and must not depend on which tracks
    happen to be in the test profile or on their order. The catalog (persisted
    by the runner from ``load_track_manifest`` at download time) supplies both
    the track choice and its metadata, so no live-library introspection is
    needed. Later scenarios (crates, playlists, ...) can remember tracks under
    their own names through the same store.
    """
    title: str
    artist: str
    location: str
    bpm: Optional[float]
    first_beat: Optional[int]
    samplerate: Optional[float]
    tags: Optional[str]

    def __str__(self):
        return f"{repr(self.title)} - {repr(self.artist)} | {self.location}"


def remember_track(context, entry, name="this"):
    """Add ``entry`` (a tracks-catalog entry with ``location``) to
    ``context.remembered_tracks`` under ``name``."""
    if not hasattr(context, "remembered_tracks"):
        context.remembered_tracks = {}
    track = RememberedTrack(
        title=entry["title"],
        artist=entry.get("artist", ""),
        location=entry["location"],
        bpm=entry.get("bpm"),
        first_beat=entry.get("first_beat"),
        samplerate=entry.get("samplerate"),
        tags=entry.get("tags"),
    )
    context.remembered_tracks[name] = track
    print(f"Track picked as {repr(name)}: {track}", flush=True)
    return track


def scenario_rng(context):
    """The scenario's deterministic RNG (derived from the run seed in
    environment.py), falling back to the global module if a step runs
    without a scenario seed."""
    rng = getattr(context, "rng", None)
    return rng if rng is not None else random


def _remembered_track(context, name="this"):
    """The track remembered under ``name`` (default: "this track")."""
    tracks = getattr(context, "remembered_tracks", {})
    assert name in tracks, f"No track was remembered as \"{name} track\""
    return tracks[name]


def _remember_catalog_track(context, unique_attr=False):
    """Randomly remember an available track, optionally with a unique attribute
    among the available tracks.
    """
    catalog = context.config.userdata.get("tracks_catalog")
    assert catalog, "No tracks catalog was persisted by the test runner"
    pool = catalog
    if unique_attr:
        values = [entry.get(unique_attr) for entry in catalog]
        pool = [
            entry for entry in catalog
            if entry.get(unique_attr) and values.count(entry[unique_attr]) == 1
        ]
    assert pool, f"No tracks available in the catalog"
    assert all(entry.get("title") for entry in pool), (
        "The catalog contains tracks without a title"
    )
    return remember_track(context, scenario_rng(context).choice(pool))


def _this_track_field(context, field):
    track = _remembered_track(context)
    value = getattr(track, field, None)
    assert value, (
        f"The remembered track has no '{field}'"
    )
    return value


def _type_this_track(context, field, transform=None):
    value = str(_this_track_field(context, field))
    if transform:
        value = transform(value)
    _type_into_library_search(context, value)
    return value


@given("a track available in the library")
@given("a track available in the library {rule}")
def step_remember_any_track(context, rule=None):
    if rule and rule.startswith("with a unique"):
        _remember_catalog_track(context, unique_attr=rule[len("with a unique"):].strip())
        return

    _remember_catalog_track(context)


@given("no search is currently active")
def step_no_search_active(context):
    """Reset the search bar to its fresh state: no criteria tokens, no query
    and deactivated. Clears without persisting, so no recent-search entry is
    created for a leftover query."""
    s = context.mixxx_rpc
    pane = SEARCH_PANE_PATH
    has_criteria = int(_get_property(s, pane, "criteriaCount") or 0) > 0
    has_query = bool(_get_property(s, pane, "activeQuery"))
    if has_criteria or has_query:
        s.invokeMethod(pane, "clearAllCriteria", [])
    if _search_activated(s):
        s.invokeMethod(pane, "deactivatePane", [])
    deadline = time.time() + 5
    criteria_count = -1
    query = ""
    while time.time() < deadline:
        criteria_count = int(_get_property(s, pane, "criteriaCount") or 0)
        query = _get_property(s, pane, "activeQuery") or ""
        if criteria_count == 0 and query == "" and not _search_activated(s):
            return
        time.sleep(0.3)
    raise AssertionError(
            f"The search bar could not be reset (criteria: {criteria_count}, "
            f"query: \"{query}\")")


@given('the library search criteria include the field "{field}" with the value "{value}"')
def step_search_criteria_include(context, field, value):
    _type_into_library_search(context, f"{field.lower()}:")
    _type_into_library_search(context, value)


@when('I type the {attr_with_transform} of this track into the library search')
@when('I type the {attr_with_transform} of this track into the library search {rule}')
def step_type_this_title(context, attr_with_transform, rule=None):
    def wrap(transform, wrapper):
        return lambda v: wrapper(transform(v) if transform else v)
    transform = None

    if rule and rule.startswith("prefixed with"):
        prefix = rule[len("prefixed with") + 1:]
        if prefix.startswith("\"") and prefix.endswith("\""):
            prefix = prefix[1:-1]
        assert prefix, "Prefix cannot be empty"
        transform = wrap(transform, lambda value: f"{prefix}{value}")

    attr = attr_with_transform
    if attr_with_transform.startswith("first word of the "):
        attr = attr_with_transform[len("first word of the "):]
        transform = wrap(transform, lambda value: value.split(" ")[0])

    _type_this_track(context, attr, transform)


@when('I press the "{key}" key in the library search')
@then('I press the "{key}" key in the library search')
def step_press_key_in_search(context, key):
    s = context.mixxx_rpc
    binding = _SEARCH_KEYS.get(key)
    assert binding is not None, f"Unsupported key '{key}'"
    if isinstance(binding, tuple):
        code, modifiers = binding
    else:
        code, modifiers = binding, 0
    s.enterKey("mainWindow", code, modifiers)
    time.sleep(0.3)


@when('I click the recent library search "{search}"')
def step_click_recent_search(context, search):
    s = context.mixxx_rpc
    expected = _remembered_track(context).title if "title of this track" in search else search
    path = f"{SEARCH_RECENT_LIST_PATH}/recent_{expected}"
    _wait_for_clickable(s, path)
    _click(s, path)
    time.sleep(0.5)


@when("I wait for the search to settle")
def step_wait_search_settle(context):
    # The query is applied through an 800 ms debounce timer; allow one full
    # cycle plus reaction time.
    time.sleep(SEARCH_APPLY_DELAY + 0.5)


@then("the library search bar should be present")
def step_search_bar_present(context):
    assert _is_visible(context.mixxx_rpc, SEARCH_PANE_PATH), (
        "The search bar is not present in the library pane")


@then("the library search bar should be anchored to the bottom-right of the library pane")
def step_search_bar_anchored(context):
    s = context.mixxx_rpc
    pane = _get_bb(s, SEARCH_PANE_PATH)
    host = _get_bb(s, f"{LIBRARY_CONTENT}/browsingView")
    tolerance = 10
    right = abs((pane["x"] + pane["width"]) - (host["x"] + host["width"])) <= tolerance
    bottom = abs((pane["y"] + pane["height"]) - (host["y"] + host["height"])) <= tolerance
    assert right and bottom, (
        f"Search bar {pane} is not anchored to the bottom-right of the "
        f"library pane {host}")


@then("the library search bar should be expanded")
def step_search_bar_expanded(context):
    s = context.mixxx_rpc
    deadline = time.time() + 5
    state = ""
    while time.time() < deadline:
        state = _get_property(s, SEARCH_PANE_PATH, "state") or ""
        if state == "expanded":
            return
        time.sleep(0.3)
    raise AssertionError(f"The search bar is not expanded (state={state})")


@then('the search placeholder "{text}" should be visible')
def step_search_placeholder_visible(context, text):
    s = context.mixxx_rpc
    deadline = time.time() + 5
    seen = ""
    while time.time() < deadline:
        for path in (
                f"{SEARCH_PANE_PATH}/searchPlaceholder",
                f"{SEARCH_PANE_PATH}/searchCollapsedPlaceholder"):
            if _is_visible(s, path):
                seen = _get_property(s, path, "text") or ""
                if seen == text:
                    return
        time.sleep(0.3)
    raise AssertionError(
        f'The search placeholder "{text}" is not visible (visible text: "{seen}")')


@then('the search hint "{text}" should be visible')
def step_search_hint_visible(context, text):
    s = context.mixxx_rpc
    path = f"{SEARCH_PANE_PATH}/searchTabHint"
    deadline = time.time() + 5
    seen = ""
    while time.time() < deadline:
        if _is_visible(s, path):
            seen = _get_property(s, path, "text") or ""
            if seen == text:
                return
        time.sleep(0.3)
    raise AssertionError(
        f'The search hint "{text}" is not visible (visible text: "{seen}")')


@then('the search token "{field}" should be active')
def step_search_token_active(context, field):
    s = context.mixxx_rpc
    deadline = time.time() + 5
    fields = []
    index = -1
    while time.time() < deadline:
        fields = _search_criteria_fields(s)
        index = int(_get_property(s, SEARCH_PANE_PATH, "activeTokenIndex") or -1)
        if 0 <= index < len(fields) and fields[index] == field:
            return
        time.sleep(0.3)
    active = fields[index] if 0 <= index < len(fields) else "none"
    raise AssertionError(
        f'The token "{field}" is not active (active: {active}, criteria: {fields})')


@then('the library search query should be "{query}"')
def step_search_query_is(context, query):
    s = context.mixxx_rpc
    expected = query
    if "title of this track" in expected:
        expected = expected.replace(
                "<title of this track>", _remembered_track(context).title)
    deadline = time.time() + 5
    seen = ""
    while time.time() < deadline:
        seen = _get_property(s, SEARCH_PANE_PATH, "activeQuery") or ""
        if seen == expected:
            return
        time.sleep(0.3)
    raise AssertionError(
            f'The search query is "{seen}", expected "{expected}"')


@then('the library search criteria should be "{fields}"')
def step_search_criteria_equal(context, fields):
    """Criteria token field names in insertion order."""
    s = context.mixxx_rpc
    expected = [field for field in fields.split(",") if field]
    deadline = time.time() + 5
    seen = []
    while time.time() < deadline:
        seen = _search_criteria_fields(s)
        if seen == expected:
            return
        time.sleep(0.3)
    raise AssertionError(
            f"The search criteria are {seen}, expected {expected}")


@then('the library search free text should be "{text}"')
@then("the library search free text should be empty")
def step_search_free_text(context, text=None):
    s = context.mixxx_rpc
    expected = text or ""
    if "title of this track" in expected:
        expected = expected.replace(
                "<title of this track>", _remembered_track(context).title)
    deadline = time.time() + 5
    seen = ""
    while time.time() < deadline:
        seen = _get_property(s, SEARCH_FIELD_PATH, "text") or ""
        if seen == expected:
            return
        time.sleep(0.3)
    raise AssertionError(
            f'The library search free text is "{seen}", expected "{expected}"')


@then("the library search bar should be empty")
def step_search_bar_empty(context):
    s = context.mixxx_rpc
    deadline = time.time() + 5
    seen = None
    while time.time() < deadline:
        count = int(_get_property(s, SEARCH_PANE_PATH, "criteriaCount") or 0)
        text = _get_property(s, SEARCH_FIELD_PATH, "text") or ""
        seen = (count, text)
        if count == 0 and text == "":
            return
        time.sleep(0.3)
    raise AssertionError(
        f"The search bar is not empty: {seen[0]} criteria, text \"{seen[1]}\"")


@then("the library search suggestion showing the artist of this track should be visible")
def step_artist_suggestion_visible(context):
    track = _remembered_track(context)
    assert track.artist, "The remembered track has no artist"
    _wait_for_search_suggestion(context, track.artist)


def _search_suggestion_texts(rpc):
    """Display texts of the currently suggested values (JSON list)."""
    raw = _get_property(rpc, SEARCH_PANE_PATH, "suggestionTexts") or "[]"
    try:
        return json.loads(raw)
    except (TypeError, ValueError):
        return []


def _wait_for_search_suggestion(context, value, timeout=15):
    s = context.mixxx_rpc
    deadline = time.time() + timeout
    texts = []
    while time.time() < deadline:
        texts = _search_suggestion_texts(s)
        if value in texts:
            return
        time.sleep(0.3)
    raise AssertionError(
        f'The search suggestion "{value}" is not visible (suggestions: {texts})')


def search_field_quote(value):
    """Quote a criterion value the same way res/qml/Library/SearchPane.qml does."""
    if re.search(r"[\s\"'=~-]", value):
        return '"' + value + '"'
    return value


@then("the library search query should contain the artist of this track")
def step_query_contains_artist(context):
    s = context.mixxx_rpc
    expected = "artist:" + search_field_quote(_this_track_field(context, "artist"))
    deadline = time.time() + 5
    query = ""
    while time.time() < deadline:
        query = _get_property(s, SEARCH_PANE_PATH, "activeQuery") or ""
        if expected in query:
            return
        time.sleep(0.3)
    raise AssertionError(
        f"The search query \"{query}\" does not contain \"{expected}\"")


def _this_track_results_visible(context):
    s = context.mixxx_rpc
    title = _this_track_field(context, "title")
    return _track_row_by_title(s, TRACKLIST_PATH, title) >= 0


@then("this track should be visible in the results")
def step_this_track_visible(context):
    assert _this_track_results_visible(context), (
        f'The remembered track "{_remembered_track(context)}" '
        "is not in the results")


@then("only this track should be visible in the results")
def step_only_this_track_visible(context):
    s = context.mixxx_rpc
    title = _this_track_field(context, "title")
    row = _track_row_by_title(s, TRACKLIST_PATH, title)
    assert row >= 0, (
        f'The remembered track "{title}" is not in the results')
    rows = _track_rows(s, TRACKLIST_PATH)
    assert rows == 1, (
        f"{rows} track rows are shown; only the track \"{title}\" was expected")


@then("not all tracks should be visible in the results")
def step_not_all_tracks_visible(context):
    s = context.mixxx_rpc
    total = _get_library_state(s).get("visibleTrackCount", 0)
    rows = _track_rows(s, TRACKLIST_PATH)
    assert rows < total, (
        f"{rows} track rows are shown, expected fewer than all {total}")

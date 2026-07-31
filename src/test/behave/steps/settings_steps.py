import json
import time
from behave import given, when, then
from mixxx_steps import _get_library_state, _library_command
from spix_helpers import (
    QT_CONTROL_MODIFIER,
    QT_KEY_A,
    QT_KEY_DOWN,
    QT_KEY_ENTER,
    QT_KEY_SPACE,
    QT_KEY_UP,
    get_control_value as _get_control_value_impl,
    is_visible as _is_visible_impl,
    wait_clickable as _wait_clickable_impl,
    wait_hidden as _wait_hidden_impl,
    wait_visible as _wait_visible_impl,
)

# --- Path constants ---

SETTINGS_POPUP = "mainWindow/settingsPopup"
SETTINGS_POPUP_ITEM = "mainWindow/settingsPopupItem"
CATEGORY_LIST = f"{SETTINGS_POPUP_ITEM}/categoryList"
TAB_BAR = f"{SETTINGS_POPUP_ITEM}/tabBar"
SEARCH_SETTING = f"{SETTINGS_POPUP_ITEM}/searchSetting"
SEARCH_INPUT = f"{SETTINGS_POPUP_ITEM}/searchInput"
CATEGORIES_ITEM = f"{SETTINGS_POPUP_ITEM}/settingsCategories"
SHOW_CATEGORIES_BUTTON = f"{SETTINGS_POPUP_ITEM}/showCategoriesButton"

SETTING_CATEGORY_SLUG = ["sound","library","controller", "interface"]
CATEGORY_SCROLLBARS_TEMPLATE = "mainWindow/%sSettingsScrollBar"
ACTION_BUTTON_TEMPLATE = "mainWindow/%s%sButton"
COMMITTING_OVERLAY = "mainWindow/committingOverlay"
ENGINE_SECTION = "mainWindow/engineSection"
DELAYS_SECTION = "mainWindow/delaysSection"
STATS_SECTION = "mainWindow/statsSection"
ROUTER_SECTION = "mainWindow/routerSection"
ROUTER_MODE = "mainWindow/routerMode"
INPUT_COLUMN = "mainWindow/inputColumn"
OUTPUT_COLUMN = "mainWindow/outputColumn"

# Model of Mixxx.Config.paletteNames(), i.e. PredefinedColorPalettes::kPalettes
# order. Index in this list == currentIndex of the palette ComboBoxes.
PALETTE_NAMES = [
    "Mixxx Hotcue Colors",
    "Serato DJ Pro Hotcue Colors",
    "Rekordbox COLD1 Hotcue Colors",
    "Rekordbox COLD2 Hotcue Colors",
    "Rekordbox COLORFUL Hotcue Colors",
    "Mixxx Track Colors",
    "Rekordbox Track Colors",
    "Serato DJ Pro Track Colors",
    "Traktor Pro Track Colors",
    "VirtualDJ Track Colors",
    "Mixxx Key Colors",
    "Traktor Key Colors",
    "Mixed In Key - Key Colors",
    "Protanopia / Protanomaly Key Colors",
    "Deuteranopia / Deuteranomaly Key Colors",
    "Tritanopia / Tritanomaly Key Colors",
]

# kinds:
#   "ratio"   -> RatioChoice: option item `path/<value>`, read `selected`
#   "combo"   -> ComboBox: open popup and click `path_option_<index>`, read `currentIndex`
#   "spinbox" -> Settings/SpinBox: write/read `realValue`
#   "slider"  -> Settings/Slider: write/read `value`
#   "color"   -> DefaultColorSelector: click `path_color_<index>`, read `currentIndex`
SETTING_MAP = {
    # Sound category (RatioChoice)
    "Main Mix": {"path": "mainWindow/setting_MainMix", "kind": "ratio", "options": ["on", "off"]},
    "Main Output Mode": {"path": "mainWindow/setting_MainOutputMode", "kind": "ratio", "options": ["mono", "stereo"]},
    "Sound Clock": {"path": "mainWindow/setting_SoundClock", "kind": "ratio", "options": ["soundcard", "network"]},
    "Keylock engine": {"path": "mainWindow/setting_KeylockEngine", "kind": "ratio"},
    "Sound API": {"path": "mainWindow/setting_SoundAPI", "kind": "ratio"},
    "Sample Rate": {"path": "mainWindow/setting_SampleRate", "kind": "ratio"},
    # Interface category — theme & color tab
    "skin": {"path": "mainWindow/setting_skin", "kind": "combo", "model": ["Unnamed"]},
    "color": {"path": "mainWindow/setting_color", "kind": "combo", "model": ["Dark", "Light"]},
    "layout": {"path": "mainWindow/setting_layout", "kind": "combo", "model": ["Performance", "Broadcast"]},
    "tool tips": {"path": "mainWindow/setting_toolTips", "kind": "ratio", "options": ["off", "library", "all"]},
    "disable screen saver": {"path": "mainWindow/setting_disableScreenSaver", "kind": "ratio", "options": ["no", "while running", "while playing"]},
    "start in full-screen mode": {"path": "mainWindow/setting_startFullscreen", "kind": "ratio", "options": ["on", "off"]},
    "auto-hide the menu bar": {"path": "mainWindow/setting_autoHideMenuBar", "kind": "ratio", "options": ["on", "off"]},
    "search completion": {"path": "mainWindow/setting_searchCompletion", "kind": "ratio", "options": ["on", "off"]},
    "search history keyboard shortcuts": {"path": "mainWindow/setting_searchHistoryKeyboardShortcuts", "kind": "ratio", "options": ["on", "off"]},
    "bpm display precision": {"path": "mainWindow/setting_bpmDisplayPrecision", "kind": "spinbox", "precision": 0},
    "library row height": {"path": "mainWindow/setting_libraryRowHeight", "kind": "slider", "min": 14, "max": 100, "markers": [14, 20, 50, 80]},
    "track palette": {"path": "mainWindow/setting_trackPalette", "kind": "combo", "model": PALETTE_NAMES},
    "hotcue palette": {"path": "mainWindow/setting_hotcuePalette", "kind": "combo", "model": PALETTE_NAMES},
    "key color palette": {"path": "mainWindow/setting_keyColorPalette", "kind": "combo", "model": PALETTE_NAMES},
    "key color": {"path": "mainWindow/setting_keyColor", "kind": "ratio", "options": ["on", "off"]},
    "hotcue default color": {"path": "mainWindow/setting_hotcueDefaultColor", "kind": "color"},
    "loop default color": {"path": "mainWindow/setting_loopDefaultColor", "kind": "color"},
    # Interface category — decks tab
    "cue mode": {"path": "mainWindow/setting_cueMode", "kind": "combo", "model": ["Mixxx", "Mixxx (no blinking)", "Pioneer", "Denon", "Numark", "CUP"]},
    "time format": {"path": "mainWindow/setting_timeFormat", "kind": "combo", "model": ["Traditional", "Traditional (Coarse)", "Seconds", "Seconds (Long)", "Kiloseconds", "Hectoseconds"]},
    "set intro start to main cue": {"path": "mainWindow/setting_introStartToMainCue", "kind": "ratio", "options": ["on", "off"]},
    "track time display": {"path": "mainWindow/setting_trackTimeDisplay", "kind": "ratio", "options": ["elapsed", "remaining", "both"]},
    "double-press load to clone": {"path": "mainWindow/setting_doublePressLoadToClone", "kind": "ratio", "options": ["on", "off"]},
    "track load point": {"path": "mainWindow/setting_trackLoadPoint", "kind": "combo", "model": ["Main cue", "Beginning of track", "First sound", "Intro start", "First hotcue"]},
    "loading a track when playing": {"path": "mainWindow/setting_loadingTrackWhenPlaying", "kind": "ratio", "options": ["reject", "allow", "when stopped"]},
    "reset on track load": {"path": "mainWindow/setting_resetOnTrackLoad", "kind": "ratio", "options": ["none", "key", "both", "tempo"]},
    "slider range": {"path": "mainWindow/setting_sliderRange", "kind": "combo", "model": ["4%", "6% (semitone)", "8% (Technics SL-1210)", "10%", "16%", "24%", "50%", "90%"]},
    "sync mode": {"path": "mainWindow/setting_syncMode", "kind": "ratio", "options": ["follow soft leader", "use steady"]},
    "slider orientation": {"path": "mainWindow/setting_sliderOrientation", "kind": "ratio", "options": ["down", "up"]},
    "keylock mode": {"path": "mainWindow/setting_keylockMode", "kind": "ratio", "options": ["original key", "current key"]},
    "keyunlock mode": {"path": "mainWindow/setting_keyunlockMode", "kind": "ratio", "options": ["reset key", "keep key"]},
    "pitch bend behaviour": {"path": "mainWindow/setting_pitchBendBehaviour", "kind": "ratio", "options": ["abrupt jump", "smooth ramping"]},
    "temporary coarse adjustment": {"path": "mainWindow/setting_temporaryCoarseAdjustment", "kind": "spinbox", "precision": 2},
    "temporary fine adjustment": {"path": "mainWindow/setting_temporaryFineAdjustment", "kind": "spinbox", "precision": 2},
    "permanent coarse adjustment": {"path": "mainWindow/setting_permanentCoarseAdjustment", "kind": "spinbox", "precision": 2},
    "permanent fine adjustment": {"path": "mainWindow/setting_permanentFineAdjustment", "kind": "spinbox", "precision": 2},
    "ramping sensitivity": {"path": "mainWindow/setting_rampingSensitivity", "kind": "slider", "min": 100, "max": 2500, "markers": []},
}

# Config keys written by saveInterface()/saveDeck() (see QmlConfigProxy).
CONFIG_SAVE_MAP = {
    "Main Mix": {
        "group": "[Config]",
        "key": "InhibitScreensaver",
        "expected": {"no": "0", "while running": "1", "while playing": "2"},
    },
    "disable screen saver": {
        "group": "[Config]",
        "key": "InhibitScreensaver",
        "expected": {"no": "0", "while running": "1", "while playing": "2"},
    },
    "slider range": {
        "group": "[Control]",
        "key": "RateRangePercent",
        "expected": lambda value: str(int(float(value.rstrip("%")))),
    },
    "slider orientation": {
        "group": "[Control]",
        "key": "RateDir",
        "expected": {"down": "", "up": "0"},
    },
    "track palette": {"group": "[Config]", "key": "TrackColorPalette"},
}

# ControlObject side-effects of saveDeck().
CONTROL_VALUE_MAP = {
    "slider range": {"key": "rateRange", "expected": lambda value: float(value.rstrip("%"))/100},
    "slider orientation": {"key": "rate_dir", "expected": {"down": -1, "up": 1}},
}

# --- Helpers ---

def _rpc(context):
    return context.mixxx_rpc


def _settings_path(*parts):
    return "/".join([SETTINGS_POPUP_ITEM] + list(parts))


def _entity_path(name):
    return f"mainWindow/entity_{name.replace('/', '')}"


def _edge_path(entity, address, channel=0):
    return f"mainWindow/entity_{entity}/edge_{address}{channel}"


def _setting_spec(setting):
    spec = SETTING_MAP.get(setting)
    if not spec:
        raise ValueError(f"Unknown setting: {setting}")
    return spec


def _setting_option_visible(s, path):
    try:
        return _is_visible(s, path)
    except Exception:
        return False


def _active_button(s, button):
    # The save/cancel/reset buttons are per-category, so the target of a
    # bare "I click the save button" is whatever category the popup has
    # active at that moment. Scenarios must select their category first
    # (the feature Backgrounds do) for this resolution to be unambiguous.
    active_category_idx = s.getStringProperty(SETTINGS_POPUP_ITEM, "activeCategoryIndex")
    assert active_category_idx != "", "Cannot resolve the currently active category"
    active_category_slug = SETTING_CATEGORY_SLUG[int(active_category_idx)]
    return ACTION_BUTTON_TEMPLATE % (active_category_slug, button.title())

def _get_control_value(s, group, key):
    return _get_control_value_impl(s, group, key)


def _get_config_value(s, group, key):
    s.command("getConfigValue", f"{group},{key}")
    return s.getStringProperty("mainWindow", "lastConfigValue")


def _wait_for_music_directory_count(rpc, expected, timeout=10):
    """Wait for the settings list to hold `expected` entries.

    Returns (ok, last_seen_count) so a failure message can report what the
    model actually contained instead of just "does not".
    """
    deadline = time.time() + timeout
    last = None
    while time.time() < deadline:
        try:
            if _is_visible(rpc, MUSIC_DIRECTORY_LIST):
                last = int(rpc.getStringProperty(MUSIC_DIRECTORY_LIST, "count"))
                if last == expected:
                    return True, last
        except Exception:
            pass
        time.sleep(0.3)
    return False, last


def _parse_library_state_expectation(value):
    value = value.strip()
    match = re.match(r"([<>]=?|!=|=)\s*(-?\d+)", value)
    if match:
        return {"op": match.group(1), "num": int(match.group(2))}
    return {"op": "=", "num": int(value)}


def _expectation_matches(actual, expectation):
    op, num = expectation["op"], expectation["num"]
    if op == "=":
        return actual == num
    if op == "!=":
        return actual != num
    if op == ">":
        return actual > num
    if op == ">=":
        return actual >= num
    if op == "<":
        return actual < num
    if op == "<=":
        return actual <= num
    return False


def _wait_for_library_state(rpc, expected, timeout=30):
    """Wait until the library state matches ``expected``.

    Returns ``(matched, last_state)`` so a failure can report the last state
    actually observed (or the RPC error that prevented reading it) instead of
    a bare "does not".
    """
    deadline = time.time() + timeout
    last_state = None
    while time.time() < deadline:
        try:
            state = _get_library_state(rpc)
            last_state = state
            matched = True
            for key, expectation in expected.items():
                actual = state.get(key)
                if key == "sources" and isinstance(actual, list):
                    actual = len(actual)
                if not _expectation_matches(actual, expectation):
                    matched = False
                    break
            if matched:
                return True, state
        except Exception as e:
            last_state = f"RPC error: {e}"
        time.sleep(0.3)
    return False, last_state


def _category_path(label):
    return f"{SETTINGS_POPUP_ITEM}/category_{label}"


def _tab_path(name):
    return f"{TAB_BAR}/tab_{name}"


# Thin adapters over spix_helpers: this module treats "condition not met"
# as a False return (callers assert on it), keeping its shorter timeouts.
def _wait_for_visible(rpc, path, timeout=5):
    return _wait_visible_impl(rpc, path, timeout)


def _wait_for_hidden(rpc, path, timeout=10):
    return _wait_hidden_impl(rpc, path, timeout)


def _click(rpc, path):
    rpc.mouseClick(path)


def _tap(rpc, path):
    """Slow left-tap (250 ms hold) for TapHandler-driven controls.

    A plain synthetic click posts the press and release almost back-to-back,
    which Qt's TapHandler can miss when the machine is under load (e.g. a CI
    runner that is also capturing video). A short hold lets the press be fully
    delivered before the release, so the tap is recognised reliably.
    """
    rpc.mouseClickAndHold(path, 1, 0, 250)


def _is_visible(rpc, path):
    return _is_visible_impl(rpc, path)


# --- Given steps ---

@given("the settings popup is open")
def step_settings_popup_open(context):
    s = _rpc(context)
    if not _is_visible(s, SETTINGS_POPUP_ITEM):
        _click(s, "mainWindow/showPreferencesButton")
        assert _wait_for_visible(s, SETTINGS_POPUP_ITEM), "Settings popup did not open"
        time.sleep(0.5)


@given('the "{category}" category is selected')
def step_category_selected(context, category):
    s = _rpc(context)
    path = _category_path(category)
    _wait_for_visible(s, path)
    index = s.getStringProperty(path, "index")
    s.setStringProperty(CATEGORY_LIST, "currentIndex", index)
    time.sleep(0.3)


@given('the "{tab}" tab is selected')
def step_tab_selected(context, tab):
    s = _rpc(context)
    path = _tab_path(tab)
    _wait_for_visible(s, path)
    _click(s, path)
    time.sleep(0.3)


@given('the "{setting}" setting is "{value}"')
def step_setting_is(context, setting, value):
    s = _rpc(context)
    spec = _setting_spec(setting)
    if spec["kind"] != "ratio":
        raise ValueError(f"Cannot preset non-ratio setting: {setting}")
    s.setStringProperty(spec["path"], "selected", value)
    time.sleep(0.3)
    step_click_action(context, 'save')
    _wait_for_hidden(s, COMMITTING_OVERLAY)
    # Verify the preset survived the save, so scenarios do not build on
    # silent no-op preconditions.
    _assert_setting_value_matches(s, setting, spec, value)


@given('the router mode is "{mode}"')
def step_router_mode_is(context, mode):
    s = _rpc(context)
    current = _get_router_mode(s)
    if current != mode:
        _set_router_mode(s, mode)


@given('a saved connection exists from "{edge}" on "{entity}" to an output device')
def step_saved_connection_exists(context, edge, entity):
    s = _rpc(context)
    source_path = _edge_path(entity, edge)
    assert _wait_for_visible(s, source_path), f"Source edge {edge} on {entity} not visible"
    _click(s, source_path)
    time.sleep(0.5)
    sink_path = _find_first_free_output_edge(s)
    assert sink_path and _wait_for_visible(s, sink_path), "No free output device edge found"
    _click(s, sink_path)
    time.sleep(0.3)
    step_click_action(context, 'save')
    _wait_for_hidden(s, COMMITTING_OVERLAY)


@given("I have the following sound devices")
def step_register_mock_devices(context):
    context._soundMockDevices = []
    for row in context.table:
        context._soundMockDevices.append({
            "name": row["name"],
            "api": row.get("api", "Mock"),
            "outputChannels": int(row.get("outputChannels", 0)),
            "inputChannels": int(row.get("inputChannels", 0)),
        })


# --- When steps ---

@when("I click the settings close button")
def step_click_close_button(context):
    s = _rpc(context)
    _click(s, f"{SETTINGS_POPUP_ITEM}/settingsCloseButton")
    time.sleep(0.3)


@when('I select the "{category}" category')
def step_select_category(context, category):
    s = _rpc(context)
    path = _category_path(category)
    assert _wait_for_visible(s, path), f"Category '{category}' not visible"
    index = s.getStringProperty(path, "index")
    s.setStringProperty(CATEGORY_LIST, "currentIndex", index)
    time.sleep(0.3)


@when('I click the "{tab}" tab')
def step_click_tab(context, tab):
    s = _rpc(context)
    path = _tab_path(tab)
    assert _wait_for_visible(s, path), f"Tab '{tab}' not visible"
    _click(s, path)
    time.sleep(0.3)


@when('I toggle the "{setting}" setting to "{value}"')
@when('I set the "{setting}" setting to "{value}"')
@when('I toggle the "{setting}" setting to "{value}" with {method}')
@when('I set the "{setting}" setting to "{value}" with {method}')
def step_set_setting(context, setting, value, method=None):
    s = _rpc(context)
    spec = _setting_spec(setting)
    path = spec["path"]
    kind = spec["kind"]
    if kind == "ratio":
        options = spec.get("options")
        if not options:
            raise ValueError(
                f"Ratio setting '{setting}' has no options mapping; "
                "cannot resolve a clickable option"
            )
        try:
            index = options.index(value)
        except ValueError:
            raise ValueError(f"Setting '{setting}' has no option '{value}'")
        option_path = f"{path}/option{index}"
        if _setting_option_visible(s, option_path):
            _click(s, option_path)
        else:
            # Maximum 40 attempts
            for _ in range(40):
                current = s.getStringProperty(path, "selected")
                # contentSpin (compacted) mode: the options are not individually
                # clickable, so drive the SpinBox next/prev buttons (real user
                # input) the needed number of times from the current selection.
                try:
                    current_idx = options.index(current)
                except ValueError:
                    raise RuntimeError(
                        f"Setting '{setting}' current '{current}' not in options"
                    )
                delta = index - current_idx
                if delta == 0: break
                button = f"{path}/nextButton" if delta > 0 else f"{path}/prevButton"
                assert _wait_for_visible(s, button), (
                    f"Setting '{setting}' next/prev button not visible"
                )
                _click_until_value_moves(s, path, button, "selected")
    elif kind == "combo":
        try:
            index = spec["model"].index(value)
        except (ValueError, AttributeError):
            raise ValueError(f"Setting '{setting}' has no option '{value}'")
        assert _wait_for_visible(s, path), f"Setting '{setting}' control not visible"
        _click(s, path)
        option_path = f"{path}_option_{index}"
        if _wait_for_visible(s, option_path):
            _click(s, option_path)
        else:
            _combo_select_keyboard(s, path, index)
    elif kind == "spinbox":
        precision = spec.get("precision", 0)
        step = 1.0 / (10 ** precision)
        # Maximum 40 attempts
        for _ in range(40):
            current = float(s.getStringProperty(path, "realValue"))
            target = float(value)
            clicks = int(round((target - current) / step))
            if clicks == 0: break
            button = f"{path}/upButton" if clicks > 0 else f"{path}/downButton"
            assert _wait_for_visible(s, button), f"Setting '{setting}' button not visible"
            _click_until_value_moves(s, path, button, "realValue")
    elif kind == "slider":
        method = method or "a drag"
        if method == "a drag":
            _drag_slider_handle(s, spec, value)
        elif method == "the spinbox":
            assert _wait_for_visible(s, f"{path}/Value"), (
                f"Setting '{setting}' value field not visible"
            )
            _enter_text_value(s, f"{path}/Value", value)
        else:
            raise ValueError(f"Unsupported method '{method}' for slider")
    elif kind == "color":
        option_path = f"{path}_color_{value}"
        assert _wait_for_visible(s, option_path), (
            f"Setting '{setting}' color '{value}' not visible"
        )
        _click(s, option_path)
    else:
        raise ValueError(f"Unknown setting kind: {kind}")
    # Prove the change actually took effect; a stuck control fails here
    # instead of at a later (or missing) Then. The bounded poll returns as
    # soon as the value matches instead of always burning a fixed sleep.
    _wait_for_setting_value(s, setting, spec, value)


@when("I click the {button} button")
def step_click_action(context, button):
    s = _rpc(context)
    path = _active_button(s, button)
    assert _is_visible(s, path), f"{button.title()} ({path}) button not visible"
    _click(s, path)
    time.sleep(0.5)


@when('I set the router mode to "{mode}"')
def step_set_router_mode(context, mode):
    s = _rpc(context)
    _set_router_mode(s, mode)


@when('I search for "{text}"')
def step_search_settings(context, text):
    s = _rpc(context)
    _tap(s, SEARCH_SETTING)
    if not _wait_for_search_active(s):
        _tap(s, SEARCH_SETTING)
    time.sleep(0.3)
    s.inputText(SEARCH_INPUT, text)
    time.sleep(0.5)


def _wait_for_search_active(s, timeout=2):
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            if s.getStringProperty(SEARCH_SETTING, "active") == "true":
                return True
        except Exception:
            pass
        time.sleep(0.1)
    return False


@when('I connect the "{edge}" edge on "{entity}" entity to a free output device with {mean}')
@when('I connect the "{edge}" edge on "{entity}" entity to a free output device')
def step_connect_edge_to_output_alt(context, edge, entity, mean="two clicks"):
    s = _rpc(context)
    source_path = _edge_path(entity, edge)
    assert _wait_for_visible(s, source_path), f"Source edge {edge} on {entity} not visible"
    if mean == "two clicks":
        _click(s, source_path)
        time.sleep(0.5)
        sink_path = _find_first_free_output_edge(s)
        assert sink_path and _wait_for_visible(s, sink_path), "No free output device edge found"
        _click(s, sink_path)
        time.sleep(0.5)
    else:
        # FIXME not correctly working
        source_bb = context.mixxx_rpc.getBoundingBox(source_path)
        sink_path = _find_first_free_output_edge(s)
        assert sink_path and _wait_for_visible(s, sink_path), "No free output device edge found"
        sink_bb = context.mixxx_rpc.getBoundingBox(sink_path)
        delta = sink_bb[0] - source_bb[0], sink_bb[1] - source_bb[1]
        s.mouseDrag(source_path, 0.5, 0.5, *delta, 1000)



@when('I hover over the "{edge}" edge on "{entity}" entity')
def step_hover_edge(context, edge, entity):
    # TODO: spix has no mouseHover/mouseMoveTo API. mouseClick triggers onPressed,
    # not onEntered. The AudioEntity edge MouseArea uses onEntered to set the
    # AboutToDelete flag (AudioConnection.Flags). Need either:
    #   1. A spix API for hover events (mouseHover / mouseMoveTo)
    #   2. A custom C++ command to directly set the AboutToDelete flag
    raise NotImplementedError(
        "spix has no hover API; mouseClick triggers onPressed not onEntered. "
        "Cannot simulate hover to set AudioConnection.Flags.AboutToDelete."
    )


@when('I connect the "{source_edge}" edge on "{source_entity}" to the "{sink_edge}" edge on "{sink_entity}"')
def step_connect_edge_to_edge(context, source_edge, source_entity, sink_edge, sink_entity):
    s = _rpc(context)
    source_path = _edge_path(source_entity, source_edge)
    assert _wait_for_visible(s, source_path), f"Source edge {source_edge} on {source_entity} not visible"
    _click(s, source_path)
    time.sleep(0.5)
    sink_path = _edge_path(sink_entity, sink_edge)
    assert _wait_for_visible(s, sink_path), f"Sink edge {sink_edge} on {sink_entity} not visible"
    _click(s, sink_path)
    time.sleep(0.5)


# --- Then steps ---

@then("the settings popup should be visible")
def step_settings_visible(context):
    s = _rpc(context)
    assert _wait_for_visible(s, SETTINGS_POPUP_ITEM), "Settings popup is not visible"


@then("the settings popup should not be visible")
def step_settings_not_visible(context):
    s = _rpc(context)
    assert _wait_for_hidden(s, SETTINGS_POPUP_ITEM), "Settings popup is still visible"


@then('the "{category}" category should be selected')
def step_category_should_be_selected(context, category):
    s = _rpc(context)
    path = _category_path(category)
    assert _is_visible(s, path), f"Category '{category}' is not visible"
    expected = s.getStringProperty(path, "index")
    actual = s.getStringProperty(CATEGORY_LIST, "currentIndex")
    assert actual == expected, (
        f"Category '{category}' should be selected (index {expected}) "
        f"but the selected index is {actual}"
    )


@then('the "{tab}" tab should be visible')
def step_tab_visible(context, tab):
    s = _rpc(context)
    path = _tab_path(tab)
    assert _is_visible(s, path), f"Tab '{tab}' is not visible"


@then('the "{tab}" tab should be selected')
def step_tab_selected_then(context, tab):
    s = _rpc(context)
    path = _tab_path(tab)
    assert _wait_for_visible(s, path), f"Tab '{tab}' not visible"
    checked = s.getStringProperty(path, "checked")
    assert checked == "true", f"Tab '{tab}' should be selected but checked={checked}"


@then("the engine section should be visible")
def step_engine_visible(context):
    s = _rpc(context)
    assert _is_visible(s, ENGINE_SECTION), "Engine section is not visible"


@then("the delays section should be visible")
def step_delays_visible(context):
    s = _rpc(context)
    assert _is_visible(s, DELAYS_SECTION), "Delays section is not visible"

def _button_is_enabled(s, path):
    """Read the ``enabled`` property, failing loudly when it is missing.

    Treating an unreadable property as ``false`` used to make a ``should be
    disabled`` assertion pass vacuously (e.g. after the button disappeared).
    """
    enabled = s.getStringProperty(path, "enabled")
    assert enabled in ("true", "false"), (
        f"Cannot read 'enabled' of '{path}' (got '{enabled}')"
    )
    return enabled == "true"


@then("the {button} button should be enabled")
def step_button_enabled(context, button):
    s = _rpc(context)
    path = _active_button(s, button)
    assert _button_is_enabled(s, path), f"{button.title()} button is not enabled"


@then("the {button} button should be disabled")
def step_button_disabled(context, button):
    s = _rpc(context)
    path = _active_button(s, button)
    assert not _button_is_enabled(s, path), f"{button.title()} button is not disabled"


@then("the {button} button should be visible")
def step_button_visible(context, button):
    s = _rpc(context)
    path = _active_button(s, button)
    assert _is_visible(s, path), f"{button.title()} button is not visible"


@then("the {button} button should not be visible")
def step_button_not_visible(context, button):
    s = _rpc(context)
    path = _active_button(s, button)
    assert not _is_visible(s, path), f"{button.title()} button is visible"


def _wait_for_setting_value(s, setting, spec, value, timeout=3):
    """Poll the setting until it matches ``value`` (bounded settle wait).

    Equivalent to a fixed sleep followed by :func:`_assert_setting_value_matches`,
    but returns as soon as the value settles and keeps the same failure
    message when the timeout expires.
    """
    deadline = time.time() + timeout
    while True:
        try:
            _assert_setting_value_matches(s, setting, spec, value)
            return
        except AssertionError as err:
            last_error = err
        if time.time() >= deadline:
            raise last_error
        time.sleep(0.1)


def _assert_setting_value_matches(s, setting, spec, value):
    """Assert the setting control's current value matches ``value``.

    Shared by the ``should be`` Then step and by the When/Given steps that
    change a setting, so a change that silently did not happen fails right
    where it was made instead of at a later (or missing) assertion.
    """
    actual = _read_setting_value(s, spec)
    kind = spec["kind"]
    if kind == "combo":
        try:
            expected = str(spec["model"].index(value))
        except (ValueError, AttributeError):
            raise ValueError(f"Setting '{setting}' has no option '{value}'")
        assert actual == expected, (
            f"Setting '{setting}' should be '{value}' but is '{actual}'"
        )
    elif kind == "slider":
        tolerance = min(1.0, 0.01 * (spec["max"] - spec["min"]))
        diff = abs(float(actual) - float(value))
        assert diff <= tolerance, (
            f"Setting '{setting}' should be '{value}' but is '{actual}' "
            f"(diff {diff} > tolerance {tolerance})"
        )
    else:
        assert actual == value, f"Setting '{setting}' should be '{value}' but is '{actual}'"


@then('the "{setting}" setting should be "{value}"')
def step_setting_should_be(context, setting, value):
    s = _rpc(context)
    _assert_setting_value_matches(s, setting, _setting_spec(setting), value)


@then('the "{setting}" setting should be enabled')
def step_setting_should_be_enabled(context, setting):
    s = _rpc(context)
    spec = _setting_spec(setting)
    actual = s.getStringProperty(spec["path"], "enabled")
    assert actual == "true", f"Setting '{setting}' should be enabled but enabled={actual}"


@then('the "{setting}" setting should be disabled')
def step_setting_should_be_disabled(context, setting):
    s = _rpc(context)
    spec = _setting_spec(setting)
    actual = s.getStringProperty(spec["path"], "enabled")
    assert actual == "false", f"Setting '{setting}' should be disabled but enabled={actual}"


@then('the "{setting}" should be saved as "{value}"')
def step_setting_saved_as(context, setting, value):
    s = _rpc(context)
    spec = CONFIG_SAVE_MAP.get(setting)
    if not spec:
        raise ValueError(f"No config mapping for setting: {setting}")
    actual = _get_config_value(s, spec["group"], spec["key"])
    expected = spec["expected"]
    if callable(expected):
        expected = expected(value)
    elif isinstance(expected, dict):
        expected = expected[value]
    assert actual == expected, (
        f"Config for '{setting}' should be saved as '{value}' but is '{actual}'"
    )


@then('the "{setting}" should be saved')
def step_setting_saved(context, setting):
    s = _rpc(context)
    spec = CONFIG_SAVE_MAP.get(setting)
    if not spec:
        raise ValueError(f"No config mapping for setting: {setting}")
    actual = _get_config_value(s, spec["group"], spec["key"])
    widget = _setting_spec(setting)
    current = s.getStringProperty(widget["path"], "currentText")
    assert actual == current, (
        f"Config for '{setting}' should be saved as '{current}' but is '{actual}'"
    )


@then('the "{setting}" on deck {deck:d} should be "{value}"')
def step_setting_control(context, setting, deck, value):
    s = _rpc(context)
    spec = CONTROL_VALUE_MAP.get(setting)
    if not spec:
        raise ValueError(f"No control mapping for setting: {setting}")
    actual = _get_control_value(s, f"[Channel{deck}]", spec["key"])
    expected = spec["expected"]
    if callable(expected):
        expected = expected(value)
    elif isinstance(expected, dict):
        expected = expected[value]
    assert abs(actual - expected) < 1e-6, (
        f"Setting '{setting}' on deck {deck} should be '{value}'({expected}) but is '{actual}'"
    )


@then("the committing overlay should be visible")
def step_committing_visible(context):
    s = _rpc(context)
    assert _is_visible(s, COMMITTING_OVERLAY), "Committing overlay is not visible"


@then("the committing overlay should not be visible after commit")
@when("the committing overlay is not visible")
def step_committing_hidden(context):
    s = _rpc(context)
    assert _wait_for_hidden(s, COMMITTING_OVERLAY, timeout=15), "Committing overlay did not disappear"


@then("the router section should be visible")
def step_router_visible(context):
    s = _rpc(context)
    assert _is_visible(s, ROUTER_SECTION), "Router section is not visible"


@then('the router mode should be "{mode}"')
def step_router_mode_should_be(context, mode):
    s = _rpc(context)
    current = _get_router_mode(s)
    assert current == mode, f"Router mode should be '{mode}' but is '{current}'"


@then("the outputs column should be visible")
def step_outputs_visible(context):
    s = _rpc(context)
    assert _is_visible(s, OUTPUT_COLUMN), "Outputs column is not visible"


@then("the inputs column should be visible")
def step_inputs_visible(context):
    s = _rpc(context)
    assert _is_visible(s, INPUT_COLUMN), "Inputs column is not visible"


@then("the input column should not be visible")
def step_input_not_visible(context):
    s = _rpc(context)
    assert not _is_visible(s, INPUT_COLUMN), "Input column should not be visible"


@then('the "{entity}" entity should be visible in the router')
def step_entity_visible(context, entity):
    s = _rpc(context)
    path = _entity_path(entity)
    assert _is_visible(s, path), f"Entity '{entity}' is not visible in the router"


@then('the "{entity}" entity should not be visible in the router')
def step_entity_not_visible(context, entity):
    s = _rpc(context)
    path = _entity_path(entity)
    assert not _is_visible(s, path), f"Entity '{entity}' should not be visible in the router"


@then('the "{edge}" edge on "{entity}" entity should show {state} state')
def step_edge_state(context, edge, entity, state):
    s = _rpc(context)
    path = _edge_path(entity, edge)
    assert _wait_for_visible(s, path), f"Edge {edge} on {entity} not visible"
    actual = s.getStringProperty(path, "state")
    assert actual == state, f"Edge {edge} on {entity} should show '{state}' state but shows '{actual}'"


@then('the connection from "{edge}" on "{entity}" should show {state} state')
def step_connection_state(context, edge, entity, state):
    s = _rpc(context)
    path = _edge_path(entity, edge)
    assert _wait_for_visible(s, path), f"Edge {edge} on {entity} not visible"
    connectionObjectName = s.invokeMethod(path, "connectionObjectName", [])
    actual = s.getStringProperty(f"mainWindow/{connectionObjectName}", "state")
    assert actual == state, f"Connection state is not '{state}', but '{actual}'"


@then("the router should match the saved configuration")
def step_router_matches_saved(context):
    s = _rpc(context)
    assert _is_visible(s, ROUTER_SECTION), "Router section is not visible"
    has_changes = s.getStringProperty(ROUTER_SECTION, "hasChanges")
    # Require an exact "false": a missing/empty property ("" from spix when
    # the item or property is gone) must fail, not pass vacuously.
    assert has_changes == "false", (
        f"Router has unsaved changes (hasChanges={has_changes})"
    )


@then('the "{option}" option should be visible')
def step_option_visible(context, option):
    s = _rpc(context)
    candidates = []
    idx = _ROUTER_MODE_OPTION_INDEX.get(option)
    if idx is not None:
        candidates.append(f"mainWindow/routerMode/option{idx}")
    idx = _MULTI_SOUNDCARD_OPTION_INDEX.get(option)
    if idx is not None:
        candidates.append(f"mainWindow/multiSoundcard/option{idx}")
    found = any(_is_visible(s, p) for p in candidates)
    assert found, f"Option '{option}' is not visible"


@then("the search results should be visible")
def step_search_results_visible(context):
    s = _rpc(context)
    result_list = f"{SETTINGS_POPUP_ITEM}/settingResultList"
    assert _wait_for_visible(s, result_list, timeout=10), "Search results are not visible"


# --- Responsiveness steps ---


def _active_scrollbar(s):
    active_category_idx = s.getStringProperty(SETTINGS_POPUP_ITEM, "activeCategoryIndex")
    assert active_category_idx, "Cannot resolve the currently active category"
    active_category_slug = SETTING_CATEGORY_SLUG[int(active_category_idx)]
    return CATEGORY_SCROLLBARS_TEMPLATE % (active_category_slug)


@then("the settings categories should {assertion} visible")
def step_settings_categories_visible(context, assertion):
    s = _rpc(context)
    expected = assertion == "be"
    assert _is_visible(s, CATEGORIES_ITEM) is expected, (
        f"Settings categories should {assertion} visible"
    )


@then("the button to reveal the categories should {assertion} visible")
def step_categories_button_visible(context, assertion):
    s = _rpc(context)
    expected = assertion == "be"
    assert _is_visible(s, SHOW_CATEGORIES_BUTTON) is expected, (
        f"Categories button should {assertion} visible"
    )


@when("I click the button to reveal the categories")
def step_click_categories_button(context):
    s = _rpc(context)
    assert _wait_for_visible(s, SHOW_CATEGORIES_BUTTON), "Categories button is not visible"
    _click(s, SHOW_CATEGORIES_BUTTON)
    time.sleep(0.3)


@then('the "{setting}" setting should be visible')
def step_setting_visible(context, setting):
    s = _rpc(context)
    spec = _setting_spec(setting)
    assert _wait_for_visible(s, spec["path"]), f"Setting '{setting}' is not visible"


@then('the "{setting}" setting should be expanded')
def step_setting_expanded(context, setting):
    s = _rpc(context)
    spec = _setting_spec(setting)
    assert _wait_for_visible(s, f"{spec['path']}/option0"), (
        f"Setting '{setting}' should be expanded but its option pills are not visible"
    )


@then('the "{setting}" setting should be compacted')
def step_setting_compacted(context, setting):
    s = _rpc(context)
    spec = _setting_spec(setting)
    assert _wait_for_visible(s, f"{spec['path']}/nextButton"), (
        f"Setting '{setting}' should be compacted but its spinbox buttons are not visible"
    )


@when("I scroll down in the settings")
def step_scroll_down_settings(context):
    s = _rpc(context)
    bar = _active_scrollbar(s)
    assert _wait_for_visible(s, bar), "The vertical scrollbar is not visible (nothing to scroll)"
    before = s.getStringProperty(bar, "position")

    # 1) Real interaction: a click on the scrollbar groove.
    s.mouseClickWithProportion(bar, 0.5, 0.9)
    time.sleep(0.5)
    if s.getStringProperty(bar, "position") != before:
        return

    # 2) Real interaction: a drag on the scrollbar. spix has no wheel API,
    #    so a drag is the closest real-interaction equivalent of a wheel
    #    spin. mouseDrag deltas are item-local pixels (see spix Drag.cpp).
    bar_height = s.getBoundingBox(bar)[3]
    s.mouseDrag(bar, 0.5, 0.2, 0, bar_height * 0.5, 300)
    time.sleep(0.5)
    if s.getStringProperty(bar, "position") != before:
        return

    # 3) Last resort: drive the scrollbar position directly. This bypasses
    #    the UI and only covers environments where the synthetic events do
    #    not reach the scrollbar; the warning keeps the bypass visible in
    #    the transcript/video.
    print(
        "WARNING: 'I scroll down in the settings' fell back to writing the "
        "scrollbar position directly (synthetic click and drag had no effect)"
    )
    s.setStringProperty(bar, "position", str(1.0 - float(s.getStringProperty(bar, "size"))))
    time.sleep(0.5)


@then('the "{setting}" setting should {assertion} visible on screen')
def step_setting_on_screen(context, setting, assertion):
    s = _rpc(context)
    spec = _setting_spec(setting)
    expected = assertion == "be"
    assert _wait_for_visible(s, spec["path"]), f"Setting '{setting}' is not present"
    setting_bb = s.getBoundingBox(spec["path"])
    bar = _active_scrollbar(s)
    bar_bb = s.getBoundingBox(bar) if _is_visible(s, bar) else None
    on_screen = True
    if bar_bb and bar_bb[2] > 0 and bar_bb[3] > 0:
        _, setting_y, _, setting_h = setting_bb
        _, bar_y, _, bar_h = bar_bb
        on_screen = setting_y + setting_h > bar_y and setting_y < bar_y + bar_h
    assert on_screen is expected, (
        f"Setting '{setting}' should {assertion} visible on screen"
    )


# --- Internal helpers ---

def _get_router_mode(s):
    try:
        return s.getStringProperty(ROUTER_MODE, "selected")
    except Exception:
        return None


# routerMode RatioChoice options: ["simple", "advanced"|"advanced (!)", "legacy"]
# (AudioRouter.qml); multiSoundcard: ["experimental", "default", "disabled"].
# Index-based option objectNames replaced the former modelData objectNames.
_ROUTER_MODE_OPTION_INDEX = {"simple": 0, "advanced": 1, "advanced (!)": 1, "legacy": 2}
_MULTI_SOUNDCARD_OPTION_INDEX = {"experimental": 0, "default": 1, "disabled": 2}


def _set_router_mode(s, mode):
    index = _ROUTER_MODE_OPTION_INDEX.get(mode)
    if index is None:
        raise ValueError(f"Unknown router mode option '{mode}'")
    path = f"{ROUTER_MODE}/option{index}"
    assert _wait_for_visible(s, path), f"Router mode option '{mode}' not visible"
    _click(s, path)
    time.sleep(0.5)


def _find_first_output_edge(s, only_not_connected=False):
    # FIXME: This function is fragile. It depends on:
    #   1. System audio hardware being present (no devices = empty outputList = returns None)
    #   2. The ListView item_0, item_1... naming convention matching spix's ListView traversal
    #   3. Edge addresses being "Default" or "Output" (actual addresses are device-specific)
    # All scenarios using "to an output device" will fail without real audio hardware.
    # Consider adding a custom C++ command to register a fake output device for testing.
    output_count = int(s.getStringProperty(f"{OUTPUT_COLUMN}/outputList", "count"))
    for i in range(output_count):
        item_path = f"{OUTPUT_COLUMN}/outputList/output{i}"
        name = s.getStringProperty(item_path, "name")
        assert name, f"Cannot find {item_path}"
        channels = int(s.getStringProperty(f"{item_path}/node", "count"))
        for channel in range(channels):
            edge_path = f"{item_path}/edge_Default{channel}"
            assert _is_visible(s, edge_path), f"Cannot find {edge_path}"
            if only_not_connected and s.getStringProperty(edge_path, "state") != "idle":
                continue
            return edge_path
    return None

def _find_first_free_output_edge(s):
    return _find_first_output_edge(s, only_not_connected=True)


def _value_to_ratio(value, minv, maxv, markers):
    """Port of Slider.qml valueToRatio (markers-aware inverse mapping)."""
    if not markers:
        return (value - minv) / (maxv - minv)
    m = list(markers)
    if m[-1] != maxv:
        m.append(maxv)
    value_count = len(m) - 1
    step_idx = 0
    for step in m:
        if step >= value:
            break
        step_idx += 1
    step_idx = min(step_idx - 1, value_count - 1)
    delta = m[step_idx + 1] - m[step_idx]
    return (step_idx + (value - m[step_idx]) / delta) / value_count


def _drag_slider_handle(s, spec, value):
    """Drag the slider handle (real user input) to set an approximate value.

    Mirrors the deck "component move" drag: begin a drag on the {path}/Handler
    and drop it at the target ratio position along the {path}/Track. Lands
    within ~1-2px, so the caller asserts with tolerance.
    """
    path = spec["path"]
    handler = f"{path}/Handler"
    track = f"{path}/Track"
    assert _wait_for_visible(s, handler), f"Slider handle not visible at {handler}"
    assert _wait_for_visible(s, track), f"Slider track not visible at {track}"
    target_vp = _value_to_ratio(
        float(value), spec["min"], spec["max"], spec.get("markers", [])
    )
    s.mouseBeginDrag(handler, 0.5, 0.5, track, target_vp, 0.5)
    s.mouseEndDrag(handler)
    time.sleep(0.5)


def _enter_text_value(s, path, value):
    """Type an exact value into an editable value field (real user input).

    Click to focus, select all, type the value, and commit with Enter. Used by
    the slider 'with the spinbox' method (the right-hand numeric field).
    """
    _click(s, path)
    time.sleep(0.2)
    s.enterKey("mainWindow", QT_KEY_A, QT_CONTROL_MODIFIER)
    s.wait(100)
    s.inputText(path, value)
    s.wait(100)
    s.enterKey("mainWindow", QT_KEY_ENTER, 0)
    time.sleep(0.3)


def _combo_select_keyboard(s, path, index):
    """Select a combo option via keyboard when its delegate is not rendered.

    The ComboBox popup only creates `popupMaxItem` (6) delegates, so options
    beyond that index have no objectName path to click. Keyboard events are
    posted to the window and follow normal Qt focus routing into the open
    popup (same pattern as the column picker step).
    """
    highlighted = int(s.getStringProperty(path, "highlightedIndex"))
    if highlighted == -1:
        s.enterKey("mainWindow", QT_KEY_DOWN, 0)
        s.wait(200)
        highlighted = int(s.getStringProperty(path, "highlightedIndex"))
    key = QT_KEY_DOWN if index > highlighted else QT_KEY_UP
    for _ in range(abs(index - highlighted)):
        s.enterKey("mainWindow", key, 0)
        s.wait(200)
    time.sleep(0.3)
    s.enterKey("mainWindow", QT_KEY_ENTER, 0)
    time.sleep(0.3)


def _read_setting_value(s, spec):
    kind = spec["kind"]
    if kind == "ratio":
        return s.getStringProperty(spec["path"], "selected")
    if kind == "combo":
        return s.getStringProperty(spec["path"], "currentIndex")
    if kind == "spinbox":
        return s.getStringProperty(spec["path"], "realValue")
    if kind == "slider":
        return s.getStringProperty(spec["path"], "value")
    if kind == "color":
        return s.getStringProperty(spec["path"], "currentIndex")
    raise ValueError(f"Unknown setting kind: {kind}")

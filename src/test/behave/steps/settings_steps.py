import json
import time
from behave import given, when, then

# --- Path constants ---

SETTINGS_POPUP = "mainWindow/settingsPopup"
SETTINGS_POPUP_ITEM = "mainWindow/settingsPopupItem"
CATEGORY_LIST = f"{SETTINGS_POPUP_ITEM}/categoryList"
TAB_BAR = f"{SETTINGS_POPUP_ITEM}/tabBar"
SEARCH_SETTING = f"{SETTINGS_POPUP_ITEM}/searchSetting"
SEARCH_INPUT = f"{SETTINGS_POPUP_ITEM}/searchInput"

SOUND_SAVE_BUTTON = "mainWindow/soundSaveButton"
SOUND_CANCEL_BUTTON = "mainWindow/soundCancelButton"
COMMITTING_OVERLAY = "mainWindow/committingOverlay"
ENGINE_SECTION = "mainWindow/engineSection"
DELAYS_SECTION = "mainWindow/delaysSection"
STATS_SECTION = "mainWindow/statsSection"
ROUTER_SECTION = "mainWindow/routerSection"
ROUTER_MODE = "mainWindow/routerMode"
INPUT_COLUMN = "mainWindow/inputColumn"
OUTPUT_COLUMN = "mainWindow/outputColumn"

SETTING_CONTROL_MAP = {
    "Main Mix": "setting_MainMix",
    "Main Output Mode": "setting_MainOutputMode",
    "Sound Clock": "setting_SoundClock",
    "Keylock engine": "setting_KeylockEngine",
    "Sound API": "setting_SoundAPI",
    "Sample Rate": "setting_SampleRate",
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


def _setting_option_path(setting, value):
    control = SETTING_CONTROL_MAP.get(setting)
    if not control:
        raise ValueError(f"Unknown setting: {setting}")
    return f"mainWindow/{control}/{value}"


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
    control = SETTING_CONTROL_MAP.get(setting)
    if not control:
        raise ValueError(f"Unknown setting: {setting}")
    s.setStringProperty(f"mainWindow/{control}", "selected", value)
    time.sleep(0.3)
    step_click_save(context)
    time.sleep(1)
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
    time.sleep(0.5)
    _click(s, SOUND_SAVE_BUTTON)
    time.sleep(1)
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
def step_toggle_setting(context, setting, value):
    s = _rpc(context)
    option_path = _setting_option_path(setting, value)
    assert _wait_for_visible(s, option_path), f"Setting '{setting}' option '{value}' not visible"
    _click(s, option_path)
    time.sleep(0.3)


@when("I click the save button")
def step_click_save(context):
    s = _rpc(context)
    _click(s, SOUND_SAVE_BUTTON)
    time.sleep(0.5)


@when("I click the cancel button")
def step_click_cancel(context):
    s = _rpc(context)
    _click(s, SOUND_CANCEL_BUTTON)
    time.sleep(0.5)


@when('I set the router mode to "{mode}"')
def step_set_router_mode(context, mode):
    s = _rpc(context)
    _set_router_mode(s, mode)


@when('I search for "{text}"')
def step_search_settings(context, text):
    s = _rpc(context)
    _click(s, SEARCH_SETTING)
    time.sleep(0.3)
    s.inputText(SEARCH_INPUT, text)
    time.sleep(0.5)

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


@then("the save button should be disabled")
def step_save_disabled(context):
    s = _rpc(context)
    assert _is_visible(s, SOUND_SAVE_BUTTON), "Save button not visible"
    enabled = s.getStringProperty(SOUND_SAVE_BUTTON, "enabled")
    assert enabled == "false", f"Save button should be disabled but enabled={enabled}"


@then("the save button should be enabled")
def step_save_enabled(context):
    s = _rpc(context)
    assert _is_visible(s, SOUND_SAVE_BUTTON), "Save button not visible"
    enabled = s.getStringProperty(SOUND_SAVE_BUTTON, "enabled")
    assert enabled == "true", f"Save button should be enabled but enabled={enabled}"


@then("the cancel button should be visible")
def step_cancel_visible(context):
    s = _rpc(context)
    assert _is_visible(s, SOUND_CANCEL_BUTTON), "Cancel button is not visible"


@then("the cancel button should not be visible")
def step_cancel_not_visible(context):
    s = _rpc(context)
    assert not _is_visible(s, SOUND_CANCEL_BUTTON), "Cancel button should not be visible"


@then('the "{setting}" setting should be "{value}"')
def step_setting_should_be(context, setting, value):
    s = _rpc(context)
    control = SETTING_CONTROL_MAP.get(setting)
    if not control:
        raise ValueError(f"Unknown setting: {setting}")
    actual = s.getStringProperty(f"mainWindow/{control}", "selected")
    assert actual == value, f"Setting '{setting}' should be '{value}' but is '{actual}'"


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
    found = False
    for path in [
        f"mainWindow/routerMode/{option}",
        f"mainWindow/multiSoundcard/{option}",
    ]:
        if _is_visible(s, path):
            found = True
            break
    assert found, f"Option '{option}' is not visible"


@then("the search results should be visible")
def step_search_results_visible(context):
    s = _rpc(context)
    result_list = f"{SETTINGS_POPUP_ITEM}/settingResultList"
    assert _is_visible(s, result_list), "Search results are not visible"


# --- Internal helpers ---

def _get_router_mode(s):
    try:
        return s.getStringProperty(ROUTER_MODE, "selected")
    except Exception:
        return None


def _set_router_mode(s, mode):
    path = f"{ROUTER_MODE}/{mode}"
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

// Name: Reloop Mixage
// Author: HorstBaerbel / gqzomer
// Version: 1.1.4 requires Mixxx 2.4 or higher

var Mixage = {};

// ----- User-configurable settings -----
Mixage.scratchByWheelTouch = false; // Set to true to scratch by touching the jog wheel instead of having to toggle the disc button. Default is false
Mixage.scratchTicksPerRevolution = 620; // Number of jog wheel ticks that make a full revolution when scratching. Reduce to "scratch more" of the track, increase to "scratch less". Default is 620 (measured)
Mixage.jogWheelScrollSpeed = 1.0; // Scroll speed when the jog wheel is used to scroll through the track. The higher, the faster. Default is 1.0
Mixage.shortPressTimeout = 400; // timeout to decide between a short press or a long press for various controls, default 400
Mixage.doublePressTimeout = 800; // timeout to decide between a single or double press, default 800 (currently only used on the Traxx button)

// ----- Internal variables (don't touch) -----

// engine connections
Mixage.vuMeterConnection = [];
Mixage.trackLoadedConnection = [];
Mixage.fxOnConnection = [];
Mixage.fxSelectConnection = [];

// timers
Mixage.traxxPressTimer = 0;
Mixage.loopLengthPressTimer = {
    "[Channel1]": 0,
    "[Channel2]": 0,
};
Mixage.dryWetPressTimer = {
    "[Channel1]": 0,
    "[Channel2]": 0,
};
Mixage.scratchTogglePressTimer = {
    "[Channel1]": 0,
    "[Channel2]": 0,
};

// constants
Mixage.numEffectUnits = 4;
Mixage.numEffectSlots = 3;
const ON = 0x7F, OFF = 0x00, DOWN = 0x7F;
const QUICK_PRESS = 1, DOUBLE_PRESS = 2;

// these objects store the state of different buttons and modes
Mixage.channels = [
    "[Channel1]",
    "[Channel2]",
];

Mixage.scratchToggleState = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.scrollToggleState = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.wheelTouched = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.loopLengthPressed = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.dryWetPressed = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.scratchPressed = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.adjustLoop = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.adjustLoopIn = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.adjustLoopOut = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.effectSlotState = {
    "[EffectRack1_EffectUnit1]": new Array(Mixage.numEffectSlots).fill(1),
    "[EffectRack1_EffectUnit2]": new Array(Mixage.numEffectSlots).fill(1),
};

Mixage.blinkTimer = {
    "[Channel1]": {},
    "[Channel2]": {},
};

Mixage.amountValue = {
    "[Channel1]": 0,
    "[Channel2]": 0,
};

// Maps channels and their controls to a MIDI control number to toggle their LEDs
Mixage.ledMap = {
    "[Channel1]": {
        "cue_indicator": 0x0B,
        "cue_play": 0x0A,
        "play_indicator": 0x0C,
        "load_indicator": 0x0D,
        "pfl": 0x0E,
        "loop": 0x05,
        "reloop": 0x06,
        "sync_enabled": 0x09,
        "fx_on": 0x08,
        "fx_sel": 0x07,
        "scratch_active": 0x04,
        "scroll_active": 0x03,
        "vu_meter": 0x1D,
    },
    "[Channel2]": {
        "cue_indicator": 0x19,
        "cue_play": 0x18,
        "play_indicator": 0x1A,
        "load_indicator": 0x1B,
        "pfl": 0x1C,
        "loop": 0x13,
        "reloop": 0x14,
        "sync_enabled": 0x17,
        "fx_on": 0x16,
        "fx_sel": 0x15,
        "scratch_active": 0x12,
        "scroll_active": 0x11,
        "vu_meter": 0x1E,
    }
};

// Maps mixxx controls to a function that toggles their LEDs
Mixage.connectionMap = {
    "cue_indicator": {"function": function(v, g, c) { Mixage.toggleLED(v, g, c); }},
    "cue_play": {"function": function(v, g, c) { Mixage.toggleLED(v, g, c); }},
    "play_indicator": {"function": function(v, g, c) { Mixage.toggleLED(v, g, c); Mixage.toggleLED(v, g, "load_indicator"); }},
    "pfl": {"function": function(v, g, c) { Mixage.toggleLED(v, g, c); }},
    "loop_enabled": {"function": function(v, g) { Mixage.toggleLoopLEDS(v, g); }},
    "loop_in": {"function": function(v, g) { if (v === 1) { Mixage.toggleLED(ON, g, "loop"); } }},
    "loop_out": {"function": function(v, g) { if (v === 1) { Mixage.toggleLED(ON, g, "reloop"); } }},
    "sync_enabled": {"function": function(v, g, c) { Mixage.toggleLED(v, g, c); }},
};

// ----- Internal variables functions -----

// Set or remove functions to call when the state of a mixxx control changes
Mixage.connectControlsToFunctions = function(group, remove) {
    for (const control in Mixage.connectionMap) {
        if (remove !== undefined) {
            Mixage.connectionMap[control][group].disconnect();
        } else {
            Mixage.connectionMap[control][group] = engine.makeConnection(group, control, Mixage.connectionMap[control].function);
        }
    }
};

Mixage.init = function(_id, _debugging) {

    // all button LEDs off
    for (let i = 0; i < 255; i++) {
        midi.sendShortMsg(0x90, i, 0);
    }

    // find controls and make engine connections for each channel in Mixage.channels
    // A predefined list with channels is used instead of a for loop to prevent engine connections to be overwritten
    Mixage.channels.forEach(function(channel) {
        const deck = script.deckFromGroup(channel);
        Mixage.connectControlsToFunctions(channel);
        const effectUnit = `[EffectRack1_EffectUnit${deck}]`;

        // set soft takeovers for effect slot Meta knobs
        for (let effectSlot = 1; effectSlot <= Mixage.numEffectSlots; effectSlot++) {
            const groupString = `[EffectRack1_EffectUnit${deck}_Effect${effectSlot}]`;
            engine.softTakeover(groupString, "meta", true);
        }

        for (let effectUnit = 1; effectUnit <= Mixage.numEffectUnits; effectUnit++) {
            // make connections for the fx on LEDs
            const fxControl = `group_${channel}_enable`;
            const fxGroup = `[EffectRack1_EffectUnit${effectUnit}]`;
            Mixage.fxOnConnection.push(engine.makeConnection(fxGroup, fxControl, function() { Mixage.toggleFxLED(channel); }));

            // set soft takeovers for effectunit meta
            engine.softTakeover(fxGroup, "super1", true);
            engine.setValue(fxGroup, "show_focus", 1);
        }

        // set soft takeover for Quick Effect
        engine.softTakeover(`[QuickEffectRack1_${channel}]`, "super1", true);

        // make connections for status LEDs
        Mixage.vuMeterConnection.push(engine.makeUnbufferedConnection(channel, "vu_meter", function(val) { midi.sendShortMsg(0x90, Mixage.ledMap[channel].vu_meter, val * 7); }));
        Mixage.trackLoadedConnection.push(engine.makeConnection(channel, "track_loaded", function() { if (Mixage.adjustLoop[channel]) { Mixage.stopLoopAdjust(channel); } }));
        Mixage.fxSelectConnection.push(engine.makeConnection(effectUnit, "focused_effect", function(value) { Mixage.handleFxSelect(value, channel); }));

        // get current status and set LEDs accordingly
        Mixage.toggleFxLED(channel);
        Mixage.handleFxSelect(engine.getValue(effectUnit, "focused_effect"), channel);
    });
};

Mixage.shutdown = function() {

    // Disconnect all engine connections that are present
    Mixage.vuMeterConnection.forEach(function(connection) { connection.disconnect(); });
    Mixage.trackLoadedConnection.forEach(function(connection) { connection.disconnect(); });
    Mixage.fxSelectConnection.forEach(function(connection) { connection.disconnect(); });
    Mixage.fxOnConnection.forEach(function(connection) { connection.disconnect(); });

    // Disconnect all controls from functions
    Mixage.channels.forEach(function(channel) { Mixage.connectControlsToFunctions(channel, true); });

    // all button LEDs off
    for (let i = 0; i < 255; i++) {
        midi.sendShortMsg(0x90, i, 0);
    }
};

// Toggle the LED on the MIDI controller by sending a MIDI message
Mixage.toggleLED = function(value, group, control) {
    midi.sendShortMsg(0x90, Mixage.ledMap[group][control], value ? 0x7F : 0);
};

// Toggles the FX On LED / Off when no effect unit is activated for a channel / On when any effect unit is active for a channel
Mixage.toggleFxLED = function(group) {
    const fxChannel = `group_${group}_enable`;
    const enabledFxGroups = [];

    for (let i = 1; i <= Mixage.numEffectUnits; i++) {
        enabledFxGroups.push(engine.getValue(`[EffectRack1_EffectUnit${i}]`, fxChannel));
    }

    if (enabledFxGroups.indexOf(1) !== -1) {
        Mixage.toggleLED(ON, group, "fx_on");
    } else {
        Mixage.toggleLED(OFF, group, "fx_on");
    }
};

// Turns the loop in and reloop LEDs on or off
Mixage.toggleLoopLEDS = function(value, group) {
    Mixage.toggleLED(value, group, "loop");
    Mixage.toggleLED(value, group, "reloop");
};

// Enable the adjustment of the loop end or start position with the jogwheel
Mixage.startLoopAdjust = function(group, adjustpoint) {

    // enable adjustment of the loop in point
    if (adjustpoint === "start" || adjustpoint === undefined) {
        Mixage.adjustLoopIn[group] = true;
        Mixage.blinkLED(Mixage.ledMap[group].loop, group, 250);

        if (Mixage.adjustLoopOut[group] && adjustpoint === "start") {
            Mixage.adjustLoopOut[group] = false;
            Mixage.blinkLED(Mixage.ledMap[group].reloop, group, 0);
            Mixage.toggleLED(ON, group, "reloop");
        }
    }

    // enable adjustment of the loop out point
    if (adjustpoint === "end" || adjustpoint === undefined) {
        Mixage.adjustLoopOut[group] = true;
        Mixage.blinkLED(Mixage.ledMap[group].reloop, group, 250);

        if (Mixage.adjustLoopIn[group] && adjustpoint === "end") {
            Mixage.adjustLoopIn[group] = false;
            Mixage.blinkLED(Mixage.ledMap[group].loop, group, 0);
            Mixage.toggleLED(ON, group, "loop");
        }
    }

    // disable scratch mode if active
    if (Mixage.scratchToggleState[group]) {
        Mixage.toggleLED(OFF, group, "scratch_active");
        Mixage.scratchToggleState[group] = false;
    }

    // disable scroll mode if active
    if (Mixage.scrollToggleState[group]) {
        Mixage.toggleLED(OFF, group, "scroll_active");
        Mixage.scrollToggleState[group] = false;
    }
};

// Disable the adjustment of the loop end or start position with the jogwheel
Mixage.stopLoopAdjust = function(group, adjustpoint) {
    if (adjustpoint === "start" | adjustpoint === undefined) {
        Mixage.adjustLoopIn[group] = false;
        Mixage.blinkLED(Mixage.ledMap[group].loop, group, 0);
    }

    if (adjustpoint === "end" | adjustpoint === undefined) {
        Mixage.adjustLoopOut[group] = false;
        Mixage.blinkLED(Mixage.ledMap[group].reloop, group, 0);
    }

    if (adjustpoint === undefined) {
        Mixage.adjustLoop[group] = false;
    }

    if (engine.getValue(group, "loop_enabled") === 1) {
        Mixage.toggleLoopLEDS(ON, group);
    }
};

// Start blinking the LED for a given control based on the time parameter, stops blinking a control light if time is set to zero
// blinking is synchronized with the "indicator_250millis" control and the time parameter is rounded to the closest division of 250ms
Mixage.blinkLED = function(control, group, time) {

    // remove any connection that might be present
    if (Object.prototype.hasOwnProperty.call(Mixage.blinkTimer[group], control)) {
        Mixage.blinkTimer[group][control].timer.disconnect();
        delete Mixage.blinkTimer[group][control];
        midi.sendShortMsg(0x90, control, OFF);
    }

    if (time > 0) { // if a time is given start blinking the led
        const cycles = Math.round(time / 250); //convert time to cycles of 250ms
        Mixage.blinkTimer[group][control] = {};
        Mixage.blinkTimer[group][control].toggle = 0;
        Mixage.blinkTimer[group][control].counter = 0;

        Mixage.blinkTimer[group][control].timer = engine.makeConnection("[App]", "indicator_250ms", function() {
            Mixage.blinkTimer[group][control].counter += 1;

            if (Mixage.blinkTimer[group][control].counter === cycles) {
                Mixage.blinkTimer[group][control].toggle = !Mixage.blinkTimer[group][control].toggle;
                midi.sendShortMsg(0x90, control, Mixage.blinkTimer[group][control].toggle);
                Mixage.blinkTimer[group][control].counter = 0;
            }
        });
    }
};

// Runs every time the focused_effect for a channel is changed either by controller or mixxx
Mixage.handleFxSelect = function(value, group) {
    const unitNr = script.deckFromGroup(group);
    const effectUnit = `[EffectRack1_EffectUnit${unitNr}]`;
    if (value === 0) {
        Mixage.toggleLED(OFF, group, "fx_sel");
        if (engine.getValue(effectUnit, "super1") !== Mixage.amountValue[group]) {
            engine.softTakeoverIgnoreNextValue(effectUnit, "super1");
        }
    } else {
        Mixage.toggleLED(ON, group, "fx_sel");
        const effectSlot = `[EffectRack1_EffectUnit${unitNr}_Effect${value}]`;
        if (engine.getValue(effectSlot, "meta") !== Mixage.amountValue[group]) {
            engine.softTakeoverIgnoreNextValue(effectSlot, "meta");
        }
    }
};

// Callback function for handleTraxPress
// previews a track on a quick press and maximize/minimize the library on double press
Mixage.TraxPressCallback = function(_channel, _control, _value, _status, group, event) {
    if (event === QUICK_PRESS) {
        if (engine.getValue("[PreviewDeck1]", "play")) {
            engine.setValue("[PreviewDeck1]", "stop", true);
        } else {
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", true);
        }
    }
    if (event === DOUBLE_PRESS) {
        script.toggleControl(group, "maximize_library");
    }
    Mixage.traxxPressTimer = 0;
};

// toggles the focussed effect or all effect slots in an effect unit on or off
Mixage.toggleEffect = function(group) {
    const unitNr = script.deckFromGroup(group);
    const effectUnit = `EffectRack1_EffectUnit${unitNr}`;
    const effectUnitGroup = `[${effectUnit}]`;
    const focusedEffect = engine.getValue(effectUnitGroup, "focused_effect");
    const enabledFxSlots = [];

    if (focusedEffect === 0) {
        for (let effectSlot = 1; effectSlot <= Mixage.numEffectSlots; effectSlot++) {
            enabledFxSlots.push(engine.getValue(`[${effectUnit}_Effect${effectSlot}]`, "enabled"));
        }

        if (enabledFxSlots.indexOf(1) === -1) {
            Mixage.effectSlotState[effectUnitGroup].map(function(state, effect) {
                engine.setValue(`[${effectUnit}_Effect${effect+1}]`, "enabled", state);
            });
        } else {
            Mixage.effectSlotState[effectUnitGroup] = enabledFxSlots;
            for (let effectSlot = 1; effectSlot <= Mixage.numEffectSlots; effectSlot++) {
                engine.setValue(`[${effectUnit}_Effect${effectSlot}]`, "enabled", 0);
            }
        }
    } else {
        script.toggleControl(`[${effectUnit}_Effect${focusedEffect}]`, "enabled");
    }
};

// ----- functions mapped to buttons -----

// selects the loop in point in loop adjustment mode, otherwise trigger "beatloop_activate"
Mixage.handleLoop = function(_channel, _control, value, _status, group) {
    if (Mixage.adjustLoop[group]) { // loop adjustment mode is active
        if (Mixage.adjustLoopOut[group] && value === DOWN) { // loop out is currently being adjusted, switch to loop in
            Mixage.startLoopAdjust(group, "start");
        } else if (Mixage.adjustLoopIn[group] && value === DOWN) { // loop in is currently being adjusted switch to loop in and out
            Mixage.startLoopAdjust(group);
        }
    } else {
        if (value === DOWN) { // loop adjustment mode is not active
            engine.setValue(group, "beatloop_activate", 1);
        } else {
            engine.setValue(group, "beatloop_activate", 0);
        }
    }
};

// selects the loop out point in loop adjustment mode, otherwise trigger reloop
Mixage.handleReloop = function(_channel, _control, value, _status, group) {
    if (Mixage.adjustLoop[group]) { // loop adjustment mode is active
        if (Mixage.adjustLoopIn[group] && value === DOWN) { // loop in is currently being adjusted, switch to loop out
            Mixage.startLoopAdjust(group, "end");
        } else if (Mixage.adjustLoopOut[group] && value === DOWN) { // loop out is currently being adjusted switch to loop in and out
            Mixage.startLoopAdjust(group);
        }
    } else {
        if (value === DOWN) { // loop adjustment mode is not active
            engine.setValue(group, "reloop_toggle", 1);
        } else {
            engine.setValue(group, "reloop_toggle", 0);
        }
    }
};

// set a loop in point if none is defined, otherwise enable adjustment of the start position with the jogwheel
Mixage.handleLoopIn = function(_channel, _control, value, _status, group) {
    if (Mixage.adjustLoop[group]) { // loop adjustment mode is active
        if (Mixage.adjustLoopOut[group] && value === DOWN) { // loop out is currently being adjusted, switch to loop in
            Mixage.startLoopAdjust(group, "start");
        } else if (Mixage.adjustLoopIn[group] && value === DOWN) { // loop in is currently being adjusted switch to loop in and out
            Mixage.startLoopAdjust(group);
        }
    } else { // loop adjustment mode is not active
        if (value === DOWN) {
            engine.setValue(group, "loop_in", 1);
        } else {
            engine.setValue(group, "loop_in", 0);
        }
    }
};

// set a loop in point if none is defined, otherwise enable adjustment of the start position with the jogwheel
Mixage.handleLoopOut = function(_channel, _control, value, _status, group) {
    if (Mixage.adjustLoop[group]) { // loop adjustment mode is active
        if (Mixage.adjustLoopIn[group] && value === DOWN) { // loop in is currently being adjusted, switch to loop out
            Mixage.startLoopAdjust(group, "end");
        } else if (Mixage.adjustLoopOut[group] && value === DOWN) { // loop out is currently being adjusted switch to loop in and out
            Mixage.startLoopAdjust(group);
        }
    } else {
        if (value === DOWN) { // loop adjustment mode is not active
            engine.setValue(group, "loop_out", 1);
        } else {
            engine.setValue(group, "loop_out", 0);
        }
    }
};

// Toggle play and make sure the preview deck stops when starting to play in a deck
// brake or softStart a while the scratch toggle button is held
Mixage.handlePlay = function(_channel, _control, value, _status, group) {
    const deck = script.deckFromGroup(group);
    if (value === DOWN && Mixage.scratchPressed[group]) { // scratch toggle is pressed
        if (engine.getValue(group, "play") === 0) {
            engine.softStart(deck, true, 1.5);
        } else {
            engine.brake(deck, true, 0.75);
        }
    } else if (value === DOWN) { // scratch toggle is not pressed
        script.toggleControl(group, "play");
    }
};

// Checks whether the Traxx button is double pressed
Mixage.handleTraxPress = function(channel, control, value, status, group) {
    if (value === DOWN) {
        if (Mixage.traxxPressTimer === 0) { // first press
            Mixage.traxxPressTimer = engine.beginTimer(Mixage.doublePressTimeout, function() {
                Mixage.TraxPressCallback(channel, control, value, status, group, QUICK_PRESS);
            }, true);
        } else { // 2nd press (before timer's out)
            engine.stopTimer(Mixage.traxxPressTimer);
            Mixage.TraxPressCallback(channel, control, value, status, group, DOUBLE_PRESS);
        }
    }
};

// select track when turning the Traxx button
Mixage.selectTrack = function(_channel, _control, value, _status, _group) {
    const diff = value - 64; // 0x40 (64) centered control
    engine.setValue("[Playlist]", "SelectTrackKnob", diff);
};

// select playlist when turning the Traxx button
Mixage.selectPlaylist = function(_channel, _control, value, _status, _group) {
    const diff = value - 64; // 0x40 (64) centered control
    engine.setValue("[Playlist]", "SelectPlaylist", diff);
};

// Stops a preview that might be playing and loads the selected track regardless
Mixage.handleTrackLoading = function(_channel, _control, value, _status, group) {
    if (value === DOWN) {
        engine.setValue("[PreviewDeck1]", "stop", true);
        engine.setValue(group, "LoadSelectedTrack", true);
    }
};

// Cycle through the effectslots of the effectunit that corresponds to a channel
Mixage.nextEffect = function(_channel, _control, value, _status, group) {
    const unitNr = script.deckFromGroup(group);
    const controlString = `[EffectRack1_EffectUnit${unitNr}]`;
    if (value === DOWN) {
        if (engine.getValue(controlString, "focused_effect") === Mixage.numEffectSlots) { // after cycling through all effectslot go back to the start
            engine.setValue(controlString, "focused_effect", 0);
        } else { // next effect slot
            const currentSelection = engine.getValue(controlString, "focused_effect");
            engine.setValue(controlString, "focused_effect", currentSelection + 1);
        }
    }
};

// Handle turning of the Dry/Wet nob
// control the dry/wet when no effect slot is selected else selects the effect for the currently selected effect slot
Mixage.handleEffectDryWet = function(_channel, _control, value, _status, group) {
    const unitNr = script.deckFromGroup(group);
    const controlString = `[EffectRack1_EffectUnit${unitNr}]`;
    const diff = (value - 64); // 0x40 (64) centered control
    if (Mixage.dryWetPressed[group]) {
        engine.setValue(controlString, "chain_preset_selector", diff);
    } else if (engine.getValue(controlString, "focused_effect") === 0) { // no effect slot is selected
        const dryWetValue = engine.getValue(controlString, "mix");
        engine.setValue(controlString, "mix", dryWetValue + (diff / 16.0));
    } else {
        const focussedEffect = engine.getValue(controlString, "focused_effect");
        engine.setValue(`[EffectRack1_EffectUnit${unitNr}_Effect${focussedEffect}]`, "effect_selector", diff);
    }
};

// Turns a currently selected effect slot on, if none are selected all effect slots are turned off
Mixage.handleDryWetPressed = function(_channel, _control, value, _status, group) {
    if (value === DOWN) {
        Mixage.dryWetPressed[group] = true;
        Mixage.dryWetPressTimer[group] = engine.beginTimer(Mixage.shortPressTimeout, function() {
            Mixage.dryWetPressTimer[group] = 0;
        }, true);
    } else {
        Mixage.dryWetPressed[group] = false;
        if (Mixage.dryWetPressTimer[group] !== 0) {
            engine.stopTimer(Mixage.dryWetPressTimer[group]);
            Mixage.dryWetPressTimer[group] = 0;
            Mixage.toggleEffect(group);
        }
    }
};

// Controls the meta for an effect slot if selected, otherwise controls the meta for an effect unit
Mixage.handleFxAmount = function(_channel, _control, value, _status, group) {
    const unitNr = script.deckFromGroup(group);
    const controlString = `[EffectRack1_EffectUnit${unitNr}]`;
    const focussedEffect = engine.getValue(controlString, "focused_effect");
    const amountValue = value / 127;
    Mixage.amountValue[group] = amountValue;
    // Mixage.takeOver[group] = true;
    if (focussedEffect === 0) { // no effect slot is selected
        engine.setValue(controlString, "super1", amountValue);
    } else {
        engine.setValue(`[EffectRack1_EffectUnit${unitNr}_Effect${focussedEffect}]`, "meta", amountValue);
    }
};

// Turn off any effect units that are enabled for the channel, if none are enabled enable the corresponding effect unit
Mixage.handleFxPress = function(_channel, _control, value, _status, group) {
    if (value === DOWN) {
        const fxChannel = `group_${group}_enable`;
        const unitNr = script.deckFromGroup(group);
        const enabledFxGroups = [];

        for (let i = 1; i <= Mixage.numEffectUnits; i++) {
            enabledFxGroups.push(engine.getValue(`[EffectRack1_EffectUnit${i}]`, fxChannel));
        }

        if (enabledFxGroups.indexOf(1) !== -1) {
            for (let effectUnit = 1; effectUnit <= Mixage.numEffectUnits; effectUnit++) {
                engine.setValue(`[EffectRack1_EffectUnit${effectUnit}]`, fxChannel, false);
            }
        } else {
            engine.setValue(`[EffectRack1_EffectUnit${unitNr}]`, fxChannel, true);
        }
    }
};

// This function is necessary to allow for soft takeover of the filter amount button
// see https://github.com/mixxxdj/mixxx/wiki/Midi-Scripting#soft-takeover
Mixage.handleFilter = function(_channel, _control, value, _status, group) {
    const amountValue = value / 127;
    Mixage.amountValue[group] = amountValue;
    engine.setValue(`[QuickEffectRack1_${group}]`, "super1", value / 127);
};

// Handles setting soft takeovers when pressing shift
Mixage.handleShift = function(_channel, _control, value, _status, group) {
    const unitNr = script.deckFromGroup(group);
    if (value === DOWN) {
        if (engine.getValue(`[QuickEffectRack1_${group}]`, "super1") !== Mixage.amountValue[group]) {
            engine.softTakeoverIgnoreNextValue(`[QuickEffectRack1_${group}]`, "super1");
        }
    } else {
        const effectUnit = `[EffectRack1_EffectUnit${unitNr}]`;
        const focusedEffect = engine.getValue(effectUnit, "focused_effect");
        if (focusedEffect === 0) {
            if (engine.getValue(effectUnit, "super1") !== Mixage.amountValue[group]) {
                engine.softTakeoverIgnoreNextValue(effectUnit, "super1");
            }
        } else {
            const effectSlot = `[EffectRack1_EffectUnit${unitNr}_Effect${focusedEffect}]`;
            if (engine.getValue(effectSlot, "meta") !== Mixage.amountValue[group]) {
                engine.softTakeoverIgnoreNextValue(effectSlot, "meta");
            }
        }
    }
};

// The "disc" button that enables/disables scratching
Mixage.scratchToggle = function(_channel, _control, value, _status, group) {
    if (value === DOWN) {
        Mixage.scratchPressed[group] = true;
        Mixage.scratchTogglePressTimer[group] = engine.beginTimer(Mixage.shortPressTimeout, function() {
            Mixage.scratchTogglePressTimer[group] = 0;
        }, true);
    } else {
        Mixage.scratchPressed[group] = false;
        if (Mixage.scratchTogglePressTimer[group] !== 0) {
            engine.stopTimer(Mixage.scratchTogglePressTimer[group]);
            Mixage.scratchTogglePressTimer[group] = 0;
            Mixage.stopLoopAdjust(group);
            Mixage.scratchToggleState[group] = !Mixage.scratchToggleState[group];
            Mixage.toggleLED(Mixage.scratchToggleState[group], group, "scratch_active");
            if (Mixage.scrollToggleState[group]) {
                Mixage.scrollToggleState[group] = !Mixage.scrollToggleState[group];
                Mixage.toggleLED(Mixage.scrollToggleState[group], group, "scroll_active");
            }
        }
    }
};

// The "loupe" button that enables/disables track scrolling
Mixage.scrollToggle = function(_channel, _control, value, _status, group) {
    if (value === DOWN) {
        Mixage.stopLoopAdjust(group);
        Mixage.scrollToggleState[group] = !Mixage.scrollToggleState[group];
        Mixage.toggleLED(Mixage.scrollToggleState[group], group, "scroll_active");
        if (Mixage.scratchToggleState[group]) {
            Mixage.scratchToggleState[group] = !Mixage.scratchToggleState[group];
            Mixage.toggleLED(Mixage.scratchToggleState[group], group, "scratch_active");
        }
    }
};

// The touch function on the wheels that enables/disables scratching
Mixage.wheelTouch = function(_channel, _control, value, _status, group) {
    const unitNr = script.deckFromGroup(group);

    if (value === DOWN) {
        Mixage.wheelTouched[group] = true;
    } else {
        Mixage.wheelTouched[group] = false;
    }

    if (Mixage.scratchByWheelTouch || Mixage.scratchToggleState[group]) {
        if (value === DOWN) {
            const alpha = 1.0 / 8.0;
            const beta = alpha / 32.0;
            engine.scratchEnable(unitNr, Mixage.scratchTicksPerRevolution, 33.33, alpha, beta);
        } else {
            engine.scratchDisable(unitNr);
        }
    }
};

// The wheel that controls the scratching / jogging
Mixage.wheelTurn = function(_channel, _control, value, _status, group) {
    const diff = value - 64; // 0x40 (64) centered control
    if (Mixage.adjustLoop[group]) {  // loop adjustment
        // triple the adjustment rate if the top of the jogwheel is being touched
        const factor = Mixage.wheelTouched[group] ? 100 : 33;
        if (Mixage.adjustLoopIn[group]) {
            const newStartPosition = engine.getValue(group, "loop_start_position") + (diff * factor);
            if (newStartPosition < engine.getValue(group, "loop_end_position")) {
                engine.setValue(group, "loop_start_position", newStartPosition);
            }
        }
        if (Mixage.adjustLoopOut[group]) {
            const newEndPosition = engine.getValue(group, "loop_end_position") + (diff * factor);
            if (newEndPosition > engine.getValue(group, "loop_start_position")) {
                engine.setValue(group, "loop_end_position", newEndPosition);
            }
        }
    } else if (Mixage.scratchByWheelTouch || Mixage.scratchToggleState[group] || Mixage.scrollToggleState[group]) {
        if (Mixage.scrollToggleState[group]) { // scroll deck
            // triple the scroll rate if the top of the jogwheel is being touched
            const speedFactor = Mixage.wheelTouched[group] ? 0.00020 : 0.000066;
            const currentPosition = engine.getValue(group, "playposition");
            engine.setValue(group, "playposition", currentPosition + speedFactor * diff * Mixage.jogWheelScrollSpeed);
        } else if (Mixage.wheelTouched[group]) {
            const deckNr = script.deckFromGroup(group);
            engine.scratchTick(deckNr, diff); // scratch deck
        } else {
            engine.setValue(group, "jog", diff); // pitch bend deck
        }
    }
};

// stop or start loop adjustment mode
Mixage.handleBeatLoopPress = function(_channel, _control, value, _status, group) {
    if (Mixage.adjustLoop[group] && value === DOWN) {
        Mixage.stopLoopAdjust(group);
    } else if (value === DOWN && engine.getValue(group, "loop_start_position") !== -1 && engine.getValue(group, "loop_end_position") !== -1) {
        Mixage.adjustLoop[group] = true;
        Mixage.startLoopAdjust(group);
    }
};

// move the track or an active loop "beatjump_size" number of beats
Mixage.handleBeatMove = function(_channel, _control, value, _status, group) {
    const beatjumpSize = (value - 64) * engine.getValue(group, "beatjump_size");
    engine.setValue(group, "beatjump", beatjumpSize);
};

// clears a loop on a short press, set internal variable to true to adjust "beatjump_size"
Mixage.handleLoopLengthPress = function(_channel, _control, value, _status, group) {
    if (value === DOWN) {
        Mixage.loopLengthPressed[group] = true;
        Mixage.loopLengthPressTimer[group] = engine.beginTimer(Mixage.shortPressTimeout, function() {
            Mixage.loopLengthPressTimer[group] = 0;
        }, true);
    } else {
        Mixage.loopLengthPressed[group] = false;
        if (Mixage.loopLengthPressTimer[group] !== 0) {
            engine.stopTimer(Mixage.loopLengthPressTimer[group]);
            Mixage.loopLengthPressTimer[group] = 0;
            if (Mixage.adjustLoop[group]) {
                Mixage.stopLoopAdjust(group);
            }
            script.triggerControl(group, "loop_remove", 100);
        }
    }
};

// changes the loop length if Mixage.loopLengthPressed[group] is false otherwise adjusts the "beatjump_size"
Mixage.handleLoopLength = function(_channel, _control, value, _status, group) {
    const diff = (value - 64); // 0x40 (64) centered control
    if (Mixage.loopLengthPressed[group]) {
        const beatjumpSize = engine.getParameter(group, "beatjump_size");
        const newBeatJumpSize = diff > 0 ? 2 * beatjumpSize : beatjumpSize / 2;
        engine.setParameter(group, "beatjump_size", newBeatJumpSize);
    } else {
        const loopScale = diff > 0 ? "loop_double" : "loop_halve";
        engine.setValue(group, loopScale, true);
    }
};

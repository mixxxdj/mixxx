// Name: Reloop Mixage
// Author: HorstBaerbel
// Version: 1.0.5 needs Mixxx 2.1+

var Mixage = {};

// ----- User-configurable settings -----
Mixage.scratchByWheelTouch = false; // Set to true to scratch by touching the jog wheel instead of having to toggle the disc button. Default is false
Mixage.scratchTicksPerRevolution = 620; // Number of jog wheel ticks that make a full revolution when scratching. Reduce to "scratch more" of the track, increase to "scratch less". Default is 620 (measured)
Mixage.jogWheelScrollSpeed = 2.0; // Scroll speed when the jog wheel is used to scroll through the track. The higher, the faster. Default is 1.0
Mixage.autoMaximizeLibrary = false; // Set to true to automatically max- and minimize the library when the browse button is used. Default is false
Mixage.libraryHideTimeout = 4000; // Time in ms after which the library will automatically minimized. Default is 4000
Mixage.libraryReducedHideTimeout = 500; // Time in ms after which the library will be minimized after loading a song into a deck. Default is 500

// ----- Internal variables (don't touch) -----
var ON = 0x7F, OFF = 0x00, DOWN = 0x7F;
var QUICK_PRESS = 1, DOUBLE_PRESS = 2;

Mixage.vuMeterConnection = [];
Mixage.loopConnection = [];
Mixage.beatConnection = [];
Mixage.fxConnection = [];
Mixage.testConnection = [];
Mixage.libraryHideTimer = 0;
Mixage.libraryRemainingTime = 0;
Mixage.doublePressTimer = 0;

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

Mixage.scratching = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.isBeatMovePressed = {
    "[Channel1]": false,
    "[Channel2]": false,
};

Mixage.init = function(_id, _debugging) {
    // all button LEDs off
    for (var i = 0; i < 255; i++) {
        midi.sendShortMsg(0x90, i, 0);
    }

    // make connection for updating the VU meters
    // Mixage.vuMeterConnection[0] = engine.makeConnection("[Channel1]", "VuMeter", function(val) {
    //     midi.sendShortMsg(0x90, 29, val * 7);
    // });
    // Mixage.vuMeterConnection[1] = engine.makeConnection("[Channel2]", "VuMeter", function(val) {
    //     midi.sendShortMsg(0x90, 30, val * 7);
    // });

    // make connection for showing the beats on the loop button when a loop is active
    Mixage.loopConnection[0] = engine.makeConnection("[Channel1]", "loop_enabled", function(value) {
        if (value === 1) {
            Mixage.beatConnection[0] = engine.makeConnection("[Channel1]", "beat_active", function(value) {
                if (engine.getValue("[Channel1]", "beatloop_size") > 0.125) { Mixage.toggleLED(value, "[Channel1]", "loop"); } else {
                    Mixage.toggleLED(ON, "[Channel1]", "loop");
                }
            });
        } else {
            Mixage.beatConnection[0].disconnect();
            Mixage.toggleLED(OFF, "[Channel1]", "loop");
        }
    });
    Mixage.loopConnection[1] = engine.makeConnection("[Channel2]", "loop_enabled", function(value) {
        if (value === 1) {
            Mixage.beatConnection[1] = engine.makeConnection("[Channel2]", "beat_active", function(value) {
                if (engine.getValue("[Channel2]", "beatloop_size") > 0.125) { Mixage.toggleLED(value, "[Channel2]", "loop"); } else {
                    Mixage.toggleLED(ON, "[Channel2]", "loop");
                }
            });
        } else {
            Mixage.beatConnection[1].disconnect();
            Mixage.toggleLED(OFF, "[Channel2]", "loop");
        }
    });

    var numEffectSlots = engine.getValue("[EffectRack1_EffectUnit1]", "num_effects");
    // var numDecks = engine.getValue("[Master]", "num_decks");

    Mixage.channels.forEach(function(channel) {
        var deck = script.deckFromGroup(channel);
        Mixage.connectControlsToFunctions(channel);

        for (var effect = 1; effect < numEffectSlots; effect++) {
            var groupString = "[EffectRack1_EffectUnit"+ deck +"_Effect" + effect + "]";
            engine.softTakeover(groupString, "meta", true);
        }

        for (var effectUnit = 1; effectUnit <= 4; effectUnit++) {
            var fxGroup = "group_"+channel+"_enable";
            Mixage.testConnection.push(engine.makeConnection("[EffectRack1_EffectUnit"+effectUnit+"]", fxGroup, function() { Mixage.toggleFxLED(channel); }));
            engine.softTakeover("[EffectRack1_EffectUnit"+effectUnit+"]", "super1", true);
            engine.setValue("[EffectRack1_EffectUnit"+effectUnit+"]", "show_focus", 1);
        }

        Mixage.vuMeterConnection.push(engine.makeConnection(channel, "VuMeter", function(val) { midi.sendShortMsg(0x90, Mixage.ledMap[channel]["VuMeter"], val * 7); }));
        Mixage.fxConnection.push(engine.makeConnection("[EffectRack1_EffectUnit"+deck+"]", "focused_effect", function(value) { Mixage.handleFxSelect(value, channel); }));

        Mixage.toggleFxLED(channel);
        Mixage.handleFxSelect(engine.getValue("[EffectRack1_EffectUnit"+deck+"]", "focused_effect"), channel);
        engine.softTakeover("[QuickEffectRack1_"+channel+"]", "super1", true);
    });
};

Mixage.shutdown = function() {

    Mixage.vuMeterConnection.forEach(function(connection) { connection.disconnect(); });
    Mixage.loopConnection.forEach(function(connection) { connection.disconnect(); });
    Mixage.fxConnection.forEach(function(connection) { connection.disconnect(); });
    Mixage.testConnection.forEach(function(connection) { connection.disconnect(); });

    Mixage.connectControlsToFunctions("[Channel1]", true);
    Mixage.connectControlsToFunctions("[Channel2]", true);

    // all button LEDs off
    for (var i = 0; i < 255; i++) {
        midi.sendShortMsg(0x90, i, 0);
    }
};

// Maps channels and their controls to a MIDI control number to toggle their LEDs
Mixage.ledMap = {
    "[Channel1]": {
        "cue_indicator": 0x0A,
        "cue_default": 0x0B,
        "play_indicator": 0x0C,
        "load_indicator": 0x0D,
        "pfl": 0x0E,
        "loop": 0x05,
        "loop_enabled": 0x06,
        "sync_enabled": 0x09,
        "fx_on": 0x08,
        "fx_sel": 0x07,
        "scratch_active": 0x04,
        "scroll_active": 0x03,
        "VuMeter": 0x1D,
    },
    "[Channel2]": {
        "cue_indicator": 0x18,
        "cue_default": 0x19,
        "play_indicator": 0x1A,
        "load_indicator": 0x1B,
        "pfl": 0x1C,
        "loop": 0x13,
        "loop_enabled": 0x14,
        "sync_enabled": 0x17,
        "fx_on": 0x16,
        "fx_sel": 0x15,
        "scratch_active": 0x12,
        "scroll_active": 0x11,
        "VuMeter": 0x1E,
    }
};

// Maps mixxx controls to a function that toggles their LEDs
Mixage.connectionMap = {
    "[Channel1]": {
        "cue_indicator": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
        "cue_default": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
        "play_indicator": [function(v, g, c) { Mixage.handlePlay(v, g, c); }, null],
        "pfl": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
        "loop_enabled": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
        "sync_enabled": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
    },
    "[Channel2]": {
        "cue_indicator": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
        "cue_default": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
        "play_indicator": [function(v, g, c) { Mixage.handlePlay(v, g, c); }, null],
        "pfl": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
        "loop_enabled": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
        "sync_enabled": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
    },
};

// Set or remove functions to call when the state of a mixxx control changes
Mixage.connectControlsToFunctions = function(group, remove) {
    remove = (remove !== undefined) ? remove : false;
    for (var control in Mixage.connectionMap[group]) {
        if (remove) {
            Mixage.connectionMap[group][control][1].disconnect();
        } else {
            Mixage.connectionMap[group][control][1] = engine.makeConnection(group, control, Mixage.connectionMap[group][control][0]);
        }
    }
};

// Toggle the LED on the MIDI controller by sending a MIDI message
Mixage.toggleLED = function(value, group, control) {
    midi.sendShortMsg(0x90, Mixage.ledMap[group][control], (value === 1 || value) ? 0x7F : 0);
};

Mixage.toggleFxLED = function(group) {
    var numUnits = engine.getValue("[EffectRack1]", "num_effectunits");
    var fxChannel = "group_" + group + "_enable";
    var enabledFxGroups = [];

    for (var i = 1; i <= numUnits; i++) {
        enabledFxGroups.push(engine.getValue("[EffectRack1_EffectUnit" + i + "]", fxChannel));
    }

    if (enabledFxGroups.indexOf(1) !== -1) {
        Mixage.toggleLED(ON, group, "fx_on");
    } else {
        Mixage.toggleLED(OFF, group, "fx_on");
    }
};

Mixage.handleFxSelect = function(value, group) {
    if (value === 0) {
        Mixage.toggleLED(OFF, group, "fx_sel");
        engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit1]", "super1");
    } else {
        Mixage.toggleLED(ON, group, "fx_sel");
        engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit2_Effect" + value + "]", "meta");
    }
};

// Toggle the LED on play button and make sure the preview deck stops when starting to play in a deck
Mixage.handlePlay = function(value, group, control) {
    // toggle the play indicator LED
    Mixage.toggleLED(value, group, control);
    // make sure to stop preview deck
    engine.setValue("[PreviewDeck1]", "stop", true);
    // toggle the LOAD button LED for the deck
    Mixage.toggleLED(value, group, "load_indicator");
};

// Set the library visible and hide it when libraryHideTimeOut is reached
Mixage.setLibraryMaximized = function(visible) {
    if (visible === true) {
        Mixage.libraryRemainingTime = Mixage.libraryHideTimeout;
        // maximize library if not maximized already
        if (engine.getValue("[Master]", "maximize_library") !== true) {
            engine.setValue("[Master]", "maximize_library", true);
            if (Mixage.libraryHideTimer === 0) {
                // timer not running. start it
                Mixage.libraryHideTimer = engine.beginTimer(Mixage.libraryHideTimeout / 5, Mixage.libraryCheckTimeout);
            }
        }
    } else {
        if (Mixage.libraryHideTimer !== 0) {
            engine.stopTimer(Mixage.libraryHideTimer);
            Mixage.libraryHideTimer = 0;
        }
        Mixage.libraryRemainingTime = 0;
        engine.setValue("[Master]", "maximize_library", false);
    }
};

Mixage.libraryCheckTimeout = function() {
    Mixage.libraryRemainingTime -= Mixage.libraryHideTimeout / 5;
    if (Mixage.libraryRemainingTime <= 0) {
        engine.stopTimer(Mixage.libraryHideTimer);
        Mixage.libraryHideTimer = 0;
        Mixage.libraryRemainingTime = 0;
        engine.setValue("[Master]", "maximize_library", false);
    }
};

Mixage.handleTraxPress = function(channel, control, value, status, group) {
    if (value === DOWN) {
        if (Mixage.doublePressTimer === 0) { // first press
            Mixage.doublePressTimer = engine.beginTimer(400, function() {
                Mixage.TraxPressCallback(channel, control, value, status, group, QUICK_PRESS);
            }, true);
        } else { // 2nd press (before timer's out)
            engine.stopTimer(Mixage.doublePressTimer);
            Mixage.TraxPressCallback(channel, control, value, status, group, DOUBLE_PRESS);
        }
    }
};

Mixage.handleTraxTurn = function(_channel, control, value, _status, _group) {
    var newValue = value - 64;
    if (Mixage.autoMaximizeLibrary) {
        Mixage.setLibraryMaximized(true);
    }
    if (control === 0x5E) { // was shift pressed?
        engine.setValue("[Playlist]", "SelectPlaylist", newValue);
    } else {
        engine.setValue("[Playlist]", "SelectTrackKnob", newValue);
    }
};

Mixage.handleTrackLoading = function(_channel, control, value, _status, group) {
    if (value === DOWN) {
        engine.setValue("[PreviewDeck1]", "stop", true);
        engine.setValue(group, control > 0x1B ? "LoadSelectedTrackAndPlay" : "LoadSelectedTrack", true);
        Mixage.libraryRemainingTime = Mixage.libraryReducedHideTimeout;
    }
};

Mixage.TraxPressCallback = function(_channel, _control, _value, _status, group, event) {
    if (event === QUICK_PRESS) {
        if (Mixage.autoMaximizeLibrary) {
            Mixage.setLibraryMaximized(true);
        }
        if (engine.getValue("[PreviewDeck1]", "play")) {
            engine.setValue("[PreviewDeck1]", "stop", true);
        } else {
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", true);
        }
    }
    if (event === DOUBLE_PRESS) {
        engine.setValue(group, "maximize_library", !engine.getValue(group, "maximize_library"));
    }
    Mixage.doublePressTimer = 0;
};

Mixage.nextEffect = function(_channel, _control, value, _status, group) {
    var unitNr = script.deckFromGroup(group);
    var controlString = "[EffectRack1_EffectUnit" + unitNr + "]";
    //for some reason one more effect slot is returned than visible in the ui, thus the minus 1
    var numEffectSlots = engine.getValue(controlString, "num_effectslots") - 1;
    if (value === DOWN) {
        if (engine.getValue(controlString, "focused_effect") === numEffectSlots) {
            for (var i = 1; i === numEffectSlots; i++) {
                var groupString = "[EffectRack1_EffectUnit" + unitNr + "_Effect" + i + "]";
                engine.softTakeoverIgnoreNextValue(groupString, "meta");
            }
            engine.softTakeoverIgnoreNextValue(controlString, "super1");
            engine.setValue(controlString, "focused_effect", 0);
        } else {
            var currentSelection = engine.getValue(controlString, "focused_effect");
            engine.setValue(controlString, "focused_effect", currentSelection + 1);
        }
    }
};

Mixage.handleEffectDryWet = function(_channel, _control, value, _status, group) {
    var unitNr = script.deckFromGroup(group);
    var controlString = "[EffectRack1_EffectUnit" + unitNr + "]";
    var diff = (value - 64); // 16.0;
    if (Mixage.isBeatMovePressed[group]) {
        Mixage.setFxChannels(diff);
    } else if (engine.getValue(controlString, "focused_effect") === 0) {
        var dryWetValue = engine.getValue(controlString, "mix");
        engine.setValue(controlString, "mix", dryWetValue + (diff / 16.0));
    } else {
        var focussedEffect = engine.getValue(controlString, "focused_effect");
        engine.setValue("[EffectRack1_EffectUnit" + unitNr + "_Effect" + focussedEffect + "]", "effect_selector", diff);
    }
};

Mixage.handleDryWetPressed = function(_channel, _control, value, _status, group) {
    var unitNr = script.deckFromGroup(group);
    var controlString = "[EffectRack1_EffectUnit" + unitNr + "]";
    var focussedEffect = engine.getValue(controlString, "focused_effect");
    var numEffectSlots = engine.getValue(controlString, "num_effects");
    if (value === DOWN && focussedEffect !== 0) {
        var effectStatus = engine.getValue("[EffectRack1_EffectUnit" + unitNr + "_Effect" + focussedEffect + "]", "enabled");
        engine.setValue("[EffectRack1_EffectUnit" + unitNr + "_Effect" + focussedEffect + "]", "enabled", !effectStatus);
    }
    if (value === DOWN && focussedEffect === 0) {
        for (var i = 1; i < numEffectSlots; i++) {
            engine.setValue("[EffectRack1_EffectUnit" + unitNr + "_Effect" + i + "]", "enabled", false);
        }
    }
};

Mixage.handleFxAmount = function(_channel, _control, value, _status, group) {
    var unitNr = script.deckFromGroup(group);
    var controlString = "[EffectRack1_EffectUnit" + unitNr + "]";
    var focussedEffect = engine.getValue(controlString, "focused_effect");
    if (focussedEffect === 0) {
        engine.setValue(controlString, "super1", value / 127);
    } else {
        engine.setValue("[EffectRack1_EffectUnit" + unitNr + "_Effect" + focussedEffect + "]", "meta", value / 127);
    }
};

Mixage.handleFxPress = function(_channel, _control, value, _status, group) {
    if (value === DOWN) {
        var numUnits = engine.getValue("[EffectRack1]", "num_effectunits");
        var fxChannel = "group_" + group + "_enable";
        var unitNr = script.deckFromGroup(group);
        var enabledFxGroups = [];

        for (var i = 1; i <= numUnits; i++) {
            enabledFxGroups.push(engine.getValue("[EffectRack1_EffectUnit" + i + "]", fxChannel));
        }

        if (enabledFxGroups.indexOf(1) !== -1) {
            for (var effectUnit = 1; effectUnit <= numUnits; effectUnit++) {
                engine.setValue("[EffectRack1_EffectUnit" + effectUnit + "]", fxChannel, false);
            }
        } else {
            engine.setValue("[EffectRack1_EffectUnit" + unitNr + "]", fxChannel, true);
        }
    }
};

// This function is necessary to allow for soft takeover of the filter amount button
// see https://github.com/mixxxdj/mixxx/wiki/Midi-Scripting#soft-takeover
Mixage.handleFilter = function(_channel, _control, value, _status, group) {
    engine.setValue("[QuickEffectRack1_"+ group +"]", "super1", value / 127);
};

//Handles setting soft takeovers when pressing shift
Mixage.handleShift = function(_channel, _control, value, _status, group) {
    if (value === DOWN) {
        var unitNr = script.deckFromGroup(group);
        engine.softTakeoverIgnoreNextValue("[QuickEffectRack1_"+group+"]", "super1");
        engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit"+unitNr+"]", "super1");
    }
};

// The "disc" button that enables/disables scratching
Mixage.scratchToggle = function(_channel, _control, value, _status, group) {
    // check for pressed->release or released->press
    if (value === DOWN) {
        Mixage.scratchToggleState[group] = !Mixage.scratchToggleState[group];
        Mixage.toggleLED(Mixage.scratchToggleState[group], group, "scratch_active");
        if (Mixage.scrollToggleState[group]) {
            Mixage.scrollToggleState[group] = !Mixage.scrollToggleState[group];
            Mixage.toggleLED(Mixage.scrollToggleState[group], group, "scroll_active");
        }
    }
};

// The "loupe" button that enables/disables track scrolling
Mixage.scrollToggle = function(_channel, _control, value, _status, group) {
    // check for pressed->release or released->press
    if (value === DOWN) {
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
    var unitNr = script.deckFromGroup(group);
    // check if scratching should be enabled
    if (Mixage.scratchByWheelTouch || Mixage.scratchToggleState[group]) {
        if (value === DOWN) {
            // turn on scratching on this deck
            var alpha = 1.0 / 8.0;
            var beta = alpha / 32.0;
            engine.scratchEnable(unitNr, Mixage.scratchTicksPerRevolution, 33.33, alpha, beta);
            Mixage.scratching[group] = true;
        } else {
            engine.scratchDisable(unitNr);
            Mixage.scratching[group] = false;
        }
    }
};

// The wheel that controls the scratching / jogging
Mixage.wheelTurn = function(_channel, _control, value, _status, group) {
    // calculate deck number from MIDI control. 0x24 controls deck 1, 0x25 deck 2
    var deckNr = script.deckFromGroup(group);
    // only enable wheel if functionality has been enabled
    if (Mixage.scratchByWheelTouch || Mixage.scratchToggleState[group] || Mixage.scrollToggleState[group]) {
        // control centers on 0x40 (64), calculate difference to that value
        var newValue = value - 64;
        if (Mixage.scrollToggleState[group]) { // scroll deck
            var currentPosition = engine.getValue(group, "playposition");
            var speedFactor = 0.00005;
            engine.setValue(group, "playposition", currentPosition + speedFactor * newValue * Mixage.jogWheelScrollSpeed);
        } else if (Mixage.scratching[group]) {
            engine.scratchTick(deckNr, newValue); // scratch deck
        } else {
            engine.setValue(group, "jog", newValue); // pitch bend deck
        }
    }
};

Mixage.handleBeatMove = function(_channel, _control, value, _status, group) {
    // control centers on 0x40 (64), calculate difference to that
    var diff = (value - 64);
    var position = diff > 0 ? "beatjump_forward" : "beatjump_backward";
    engine.setValue(group, position, true);
};

Mixage.handleBeatMovePressed = function(_channel, _control, value, _status, group) {
    Mixage.isBeatMovePressed[group] = value === DOWN ? true : false;
};

Mixage.handleLoopLength = function(_channel, _control, value, _status, group) {
    // control centers on 0x40 (64), calculate difference to that
    var diff = (value - 64);
    if (Mixage.isBeatMovePressed[group]) {
        var beatjumpSize = engine.getParameter(group, "beatjump_size");
        var newBeatJumpSize = diff > 0 ? 2 * beatjumpSize : beatjumpSize / 2;
        engine.setParameter(group, "beatjump_size", newBeatJumpSize);
    } else {
        var loopScale = diff > 0 ? "loop_double" : "loop_halve";
        engine.setValue(group, loopScale, true);
        print(engine.getValue(group, "beatloop_size"));
    }
};

// The PAN rotary control used here for panning the master
Mixage.handlePan = function(_channel, _control, value, _status, _group) {
    // control centers on 0x40 (64), calculate difference to that value and scale down
    var diff = (value - 64) / 16.0;
    var mixValue = engine.getValue("[Master]", "balance");
    engine.setValue("[Master]", "balance", mixValue + diff);
};
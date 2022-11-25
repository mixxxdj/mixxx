// Name: Reloop Mixage
// Author: Bim Overbohm
// Version: 1.0.3 needs Mixxx 2.1+

var Mixage = {};

Mixage.vuMeterConnection = [];
Mixage.libraryHideTimer = 0;
Mixage.libraryReducedHideTimeout = 500;
Mixage.libraryHideTimeout = 4000;
Mixage.libraryRemainingTime = 0;
Mixage.scratchPressed = false;
Mixage.scrollPressed = false;
Mixage.scratchByWheelTouch = false;
Mixage.beatMovePressed = false;
Mixage.effectRackSelected = [[true, false], [true, false]]; // if effect rack 1/2 is selected for channel 1/2
Mixage.effectRackEnabled = [false, false]; // if effect rack 1/2 is enabled for channel 1/2

Mixage.init = function(/*id, debugging*/) {
    Mixage.connectControlsToFunctions("[Channel1]");
    Mixage.connectControlsToFunctions("[Channel2]");
    // all button LEDs off
    for (var i = 0; i < 255; i++) {
        midi.sendShortMsg(0x90, i, 0);
    }
    // start timers for updating the VU meters
    Mixage.vuMeterConnection[0] = engine.makeConnection("[Channel1]", "VuMeter", function(val) {
        midi.sendShortMsg(0x90, 29, val * 7);
    });
    Mixage.vuMeterConnection[1] = engine.makeConnection("[Channel2]", "VuMeter", function(val) {
        midi.sendShortMsg(0x90, 30, val * 7);
    });
    // disable scrating on both decks
    engine.scratchDisable(1);
    engine.scratchDisable(2);
    // disable effects for both channels
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", false);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", false);
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", false);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", false);
};

Mixage.shutdown = function() {
    Mixage.vuMeterConnection[0].disconnect();
    Mixage.vuMeterConnection[1].disconnect();
    Mixage.setLibraryMaximized(false);
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
        "pfl": 0x0E,
        "loop_enabled": 0x06,
        "sync_enabled": 0x09,
        "master_on": 0x07,
        "fx_on": 0x08
    },
    "[Channel2]": {
        "cue_indicator": 0x18,
        "cue_default": 0x19,
        "play_indicator": 0x1A,
        "pfl": 0x1C,
        "loop_enabled": 0x14,
        "sync_enabled": 0x17,
        "master_on": 0x15,
        "fx_on": 0x16
    }
};

// Maps mixxx controls to a function that toggles their LEDs
Mixage.connectionMap = {
    "cue_indicator": "Mixage.toggleLED",
    "cue_default": "Mixage.toggleLED",
    "play_indicator": "Mixage.handlePlay",
    "pfl": "Mixage.toggleLED",
    "loop_enabled": "Mixage.toggleLED",
    "sync_enabled": "Mixage.toggleLED"
};

// Set or remove functions to call when the state of a mixxx control changes
Mixage.connectControlsToFunctions = function(group, remove) {
    remove = (typeof remove !== "undefined") ? remove : false;
    for (var control in Mixage.connectionMap) {
        engine.connectControl(group, control, Mixage.connectionMap[control], remove);
        if (!remove) {
            engine.trigger(group, control);
        }
    }
};

// Toggle the LED on the MIDI controller by sending a MIDI message
Mixage.toggleLED = function(value, group, control) {
    midi.sendShortMsg(0x90, Mixage.ledMap[group][control], (value === 1) ? 0x7F : 0);
};

// Toggle the LED on play button and make sure the preview deck stops when starting to play in a deck
Mixage.handlePlay = function(value, group, control) {
    Mixage.toggleLED(value, group, control);
    engine.setValue("[PreviewDeck1]", "stop", true);
};

// Helper function to scroll the playlist
Mixage.scrollLibrary = function(value) {
    Mixage.setLibraryMaximized(true);
    //engine.setValue('[Library]', 'MoveVertical', value);
    engine.setValue("[Playlist]", "SelectTrackKnob", value); // for Mixxx < 2.1
};

// A button for the playlist was pressed
Mixage.handleLibrary = function(channel, control, value, status/*, group*/) {
    // "push2browse" button was moved somehow
    if (control === 0x1F) { // "push2browse" button was pushed
        if (status === 0x90 && value === 0x7F) {
            Mixage.setLibraryMaximized(true);
            // stop the currently playing track. if it wasn't playing, start it
            if (engine.getValue("[PreviewDeck1]", "play")) {
                engine.setValue("[PreviewDeck1]", "stop", true);
            } else {
                engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", true);
            }
        } else if (status === 0xB0) { // "push2browse" button was turned
            var newValue = value - 64;
            Mixage.scrollLibrary(newValue);
        }
    } else if (control === 0x0D || control === 0x4C) { // load into deck 1
        if (value === 0x7F) {
            engine.setValue("[PreviewDeck1]", "stop", true);
            engine.setValue("[Channel1]", control === 0x4C ? "LoadSelectedTrackAndPlay" : "LoadSelectedTrack", true);
            Mixage.libraryRemainingTime = Mixage.libraryReducedHideTimeout;
        }
    } else if (control === 0x1B || control === 0x5A) { // load into deck 2
        if (value === 0x7F) {
            engine.setValue("[PreviewDeck1]", "stop", true);
            engine.setValue("[Channel2]", control === 0x5A ? "LoadSelectedTrackAndPlay" : "LoadSelectedTrack", true);
            Mixage.libraryRemainingTime = Mixage.libraryReducedHideTimeout;
        }
    }
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

// The "record" button that enables/disables scratching
Mixage.scratchActive = function(channel, control, value/*, status, group*/) {
    // calculate deck number from MIDI control. 0x04 controls deck 1, 0x12 deck 2
    var deckNr = control === 0x04 ? 1 : 2;
    if (value === 0x7F) {
        Mixage.scratchPressed = true;
        var alpha = 1.0 / 8.0;
        var beta = alpha / 32.0;
        engine.scratchEnable(deckNr, 620, 20.0/*33.0+1.0/3.0*/, alpha, beta);
    } else {
        Mixage.scratchPressed = false;
        engine.scratchDisable(deckNr);
    }
};

// The "magnifying glass" button that enables/disables playlist scrolling
Mixage.scrollActive = function(channel, control, value/*, status, group*/) {
    // calculate deck number from MIDI control. 0x04 controls deck 1, 0x12 deck 2
    Mixage.scrollPressed = value === 0x7F;
    if (Mixage.scrollPressed) {
        Mixage.setLibraryMaximized(true);
    }
};

// The touch function on the wheels that enables/disables scratching
Mixage.wheelTouch = function(channel, control, value/*, status, group*/) {
    // check if scratching through wheel touch is enabled
    if (Mixage.scratchByWheelTouch) {
        // calculate deck number from MIDI control. 0x24 controls deck 1, 0x25 deck 2
        var deckNr = control - 0x24 + 1;
        if (value === 0x7F) {
            var alpha = 1.0 / 8;
            var beta = alpha / 32.0;
            engine.scratchEnable(deckNr, 620, 33.0 + 1.0 / 3.0, alpha, beta);
        } else {
            engine.scratchDisable(deckNr);
        }
    }
};

// The wheel that actually controls the scratching / jogging
Mixage.wheelTurn = function(channel, control, value/*, status, group*/) {
    // calculate deck number from MIDI control. 0x24 controls deck 1, 0x25 deck 2
    var deckNr = control - 0x24 + 1;
    // Control centers on 0x40 (64), calculate difference to that value
    var newValue = value - 64;
    // In either case, register the movement
    if (Mixage.scratchPressed) {
        engine.scratchTick(deckNr, newValue); // scratch
    } else {
        engine.scratchDisable(deckNr);
    }
    if (Mixage.scrollPressed) {
        Mixage.scrollLibrary(newValue);
    }
    //engine.setValue('[Channel'+deckNr+']', 'jog', newValue); // Pitch bend
};

// The MASTER button that toggles routing master through effects
Mixage.handleEffectMasterOn = function(channel, control, value/*, status, group*/) {
    // calculate effect unit number from MIDI control. 0x46 controls unit 1, 0x54 unit 2
    var unitNr = control === 0x46 ? 1 : 2;
    // react only on first message / keydown
    if (value === 0x7F) {
        // check and toggle enablement
        var controlString = "[EffectRack1_EffectUnit" + unitNr + "]";
        var keyString = "group_[Master]_enable";
        var isEnabled = !engine.getValue(controlString, keyString);
        engine.setValue(controlString, keyString, isEnabled);
        Mixage.toggleLED(isEnabled ? 1 : 0, "[Channel" + unitNr + "]", "master_on");
    }
};

// The FX ON button that toggles routing channel 1/2 through effects
Mixage.handleEffectChannelOn = function(channel, control, value/*, status, group*/) {
    // calculate effect unit number from MIDI control. 0x08 controls unit 1, 0x16 unit 2
    var unitNr = control === 0x08 ? 1 : 2;
    // react only on first message / keydown
    if (value === 0x7F) {
        // check and toggle enablement
        Mixage.effectRackEnabled[unitNr - 1] = !Mixage.effectRackEnabled[unitNr - 1];
        var isEnabled = Mixage.effectRackEnabled[unitNr - 1];
        // update controls
        var keyString = "group_[Channel" + unitNr + "]_enable";
        engine.setValue("[EffectRack1_EffectUnit1]", keyString, isEnabled && Mixage.effectRackSelected[unitNr - 1][0]);
        engine.setValue("[EffectRack1_EffectUnit2]", keyString, isEnabled && Mixage.effectRackSelected[unitNr - 1][1]);
        Mixage.toggleLED(isEnabled ? 1 : 0, "[Channel" + unitNr + "]", "fx_on");
    }
};

// The FX SEL button that selects which effects are enabled for channel 1/2
Mixage.handleEffectChannelSelect = function(channel, control, value/*, status, group*/) {
    // calculate effect unit number from MIDI control. 0x07 controls unit 1, 0x15 unit 2
    var unitNr = control === 0x07 ? 1 : 2;
    // react only on first message / keydown
    if (value === 0x7F) {
        // check and toggle select state
        var selected1 = Mixage.effectRackSelected[unitNr - 1][0];
        var selected2 = Mixage.effectRackSelected[unitNr - 1][1];
        if (selected1 && selected2) {
            selected1 = true;
            selected2 = false;
        } else if (selected1) {
            selected1 = false;
            selected2 = true;
        } else {
            selected1 = true;
            selected2 = true;
        }
        Mixage.effectRackSelected[unitNr - 1][0] = selected1;
        Mixage.effectRackSelected[unitNr - 1][1] = selected2;
        // update controls
        var isEnabled = Mixage.effectRackEnabled[unitNr - 1];
        var keyString = "group_[Channel" + unitNr + "]_enable";
        engine.setValue("[EffectRack1_EffectUnit1]", keyString, isEnabled && Mixage.effectRackSelected[unitNr - 1][0]);
        engine.setValue("[EffectRack1_EffectUnit2]", keyString, isEnabled && Mixage.effectRackSelected[unitNr - 1][1]);
    }
};

Mixage.handleEffectDryWet = function(channel, control, value/*, status, group*/) {
    // calculate effect unit number from MIDI control. 0x21 controls unit 1, 0x25 unit 2
    var unitNr = control === 0x21 ? 1 : 2;
    // Control centers on 0x40 (64), calculate difference to that value and multiply by 4
    var diff = (value - 64) / 10.0;
    // In either case, register the movement
    var controlString = "[EffectRack1_EffectUnit" + unitNr + "]";
    var dryWetValue = engine.getValue(controlString, "mix");
    engine.setValue(controlString, "mix", dryWetValue + diff);
};

// The PAN rotary control used here for the overall effect super control
Mixage.handleEffectSuper = function(channel, control, value/*, status, group*/) {
    // calculate effect unit number from MIDI control. 0x60 controls unit 1, 0x62 unit 2
    var unitNr = control === 0x60 ? 1 : 2;
    // Control centers on 0x40 (64), calculate difference to that value and multiply by 4
    var diff = (value - 64) / 10.0;
    // In either case, register the movement
    var controlString = "[EffectRack1_EffectUnit" + unitNr + "]";
    var mixValue = engine.getValue(controlString, "super1");
    engine.setValue(controlString, "super1", mixValue + diff);
};

// The BEAT MOVE rotary control is used as an extra "shift" key when pushed down
Mixage.handleBeatMovePressed = function(channel, control, value/*, status, group*/) {
    Mixage.beatMovePressed = value === 0x7f;
};

Mixage.handleBeatMoveLength = function(channel, control, value/*, status, group*/) {
    // calculate effect unit number from MIDI control. 0x20 controls unit 1, 0x22 unit 2
    var unitNr = control === 0x20 ? 1 : 2;
    var direction = unitNr === 1 ? 1 : -1;
    // Control centers on 0x40 (64), calculate difference to that
    var diff = (value - 64);
    // In either case, register the movement
    if (Mixage.beatMovePressed) {
        var loopLength = engine.getParameter("[Channel" + unitNr + "]", "beatjump_size");
        loopLength = direction * diff > 0 ? 2 * loopLength : loopLength / 2;
        engine.setParameter("[Channel" + unitNr + "]", "beatjump_size", loopLength);
    } else {
        var loopScale = direction * diff > 0 ? "loop_double" : "loop_halve";
        engine.setValue("[Channel" + unitNr + "]", loopScale, true);
    }
};

Mixage.handleBeatMove = function(channel, control, value/*, status, group*/) {
    // calculate effect unit number from MIDI control. 0x5f controls unit 1, 0x61 unit 2
    var unitNr = control === 0x5f ? 1 : 2;
    var direction = unitNr === 1 ? 1 : -1;
    // Control centers on 0x40 (64), calculate difference to that
    var diff = (value - 64);
    // In either case, register the movement
    var position = direction * diff > 0 ? "beatjump_forward" : "beatjump_backward";
    engine.setValue("[Channel" + unitNr + "]", position, true);
};

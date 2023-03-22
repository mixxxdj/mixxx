// Name: Reloop Mixage
// Author: Bim Overbohm
// Version: 1.0.4 needs Mixxx 2.1+

var Mixage = {};

// ----- User-configurable settings -----
Mixage.scratchByWheelTouch = false; // Set to true to scratch by touching the wheel instead of having to press the disc button
Mixage.autoMaximizeLibrary = false; // Set to true to automatically max- and minimize the library when the browse button is used
Mixage.libraryHideTimeout = 4000; // Time in ms after which the library will automatically minimized
Mixage.libraryReducedHideTimeout = 500; // Time in ms after which the library will be minimized after loading a song into a deck

// ----- Internal variables (don't touch) -----
Mixage.vuMeterConnection = [];
Mixage.libraryHideTimer = 0;
Mixage.libraryRemainingTime = 0;
Mixage.beatMovePressed = false;
// Note that the following lists have 3 entries, but we use only 2 decks / effect units. This saves us having to write "deckNr - 1" everywhere...
Mixage.scratchToggleLastState = [0, 0, 0]; // helper array to enable scratch toggling by disc button
Mixage.isScratchActive = [false, false, false]; // true if scratching currently enabled for deck
Mixage.scrollToggleLastState = [0, 0, 0]; // helper array to enable scroll toggling by loupe button
Mixage.isScrollActive = [false, false, false]; // true if scrolling currently enabled for deck
Mixage.effectRackSelected = [[true, false], [true, false], [true, false]]; // if effect rack 1/2 is selected for channel 1/2
Mixage.effectRackEnabled = [false, false, false]; // if effect rack 1/2 is enabled for channel 1/2

Mixage.init = function(_id, _debugging) {
    // all button LEDs off
    for (var i = 0; i < 255; i++) {
        midi.sendShortMsg(0x90, i, 0);
    }
    Mixage.connectControlsToFunctions("[Channel1]");
    Mixage.connectControlsToFunctions("[Channel2]");
    // make connection for updating the VU meters
    Mixage.vuMeterConnection[0] = engine.makeConnection("[Channel1]", "VuMeter", function(val) {
        midi.sendShortMsg(0x90, 29, val * 7);
    });
    Mixage.vuMeterConnection[1] = engine.makeConnection("[Channel2]", "VuMeter", function(val) {
        midi.sendShortMsg(0x90, 30, val * 7);
    });
    // initially disable scratching for both decks
    if (!Mixage.scratchByWheelTouch) {
        engine.scratchDisable(1);
        engine.scratchDisable(2);
    }
    // restore deck sync master LEDs (function currently not working as of 2.3.3)
    //var isDeck1SyncLeader = engine.getValue("[Channel1]", "sync_master");
    //Mixage.toggleLED(isDeck1SyncLeader ? 1 : 0, "[Channel1]", "sync_master");
    //var isDeck2SyncLeader = engine.getValue("[Channel2]", "sync_master");
    //Mixage.toggleLED(isDeck2SyncLeader ? 1 : 0, "[Channel2]", "sync_master");
    // restore deck 1 effect states from user preferences
    Mixage.effectRackSelected[1][0] = engine.getValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable");
    Mixage.effectRackSelected[1][1] = engine.getValue("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable");
    Mixage.effectRackEnabled[1] = Mixage.effectRackSelected[1][0] || Mixage.effectRackSelected[1][1];
    Mixage.toggleLED(Mixage.effectRackEnabled[1] ? 1 : 0, "[Channel1]", "fx_on");
    // restore deck 2 effect states from user preferences
    Mixage.effectRackSelected[2][0] = engine.getValue("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable");
    Mixage.effectRackSelected[2][1] = engine.getValue("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable");
    Mixage.effectRackEnabled[2] = Mixage.effectRackSelected[2][0] || Mixage.effectRackSelected[2][1];
    Mixage.toggleLED(Mixage.effectRackEnabled[2] ? 1 : 0, "[Channel2]", "fx_on");
};

Mixage.shutdown = function() {
    Mixage.vuMeterConnection[0].disconnect();
    Mixage.vuMeterConnection[1].disconnect();
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
        "loop_enabled": 0x06,
        "sync_enabled": 0x09,
        "sync_master": 0x07,
        "fx_on": 0x08,
        "scratch_active": 0x04,
        "scroll_active": 0x03,
    },
    "[Channel2]": {
        "cue_indicator": 0x18,
        "cue_default": 0x19,
        "play_indicator": 0x1A,
        "load_indicator": 0x1B,
        "pfl": 0x1C,
        "loop_enabled": 0x14,
        "sync_enabled": 0x17,
        "sync_master": 0x15,
        "fx_on": 0x16,
        "scratch_active": 0x12,
        "scroll_active": 0x11
    }
};

// Maps mixxx controls to a function that toggles their LEDs
Mixage.connectionMap = {
    "cue_indicator": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
    "cue_default": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
    "play_indicator": [function(v, g, c) { Mixage.handlePlay(v, g, c); }, null],
    "pfl": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
    "loop_enabled": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null],
    "sync_enabled": [function(v, g, c) { Mixage.toggleLED(v, g, c); }, null]
};

// Set or remove functions to call when the state of a mixxx control changes
Mixage.connectControlsToFunctions = function(group, remove) {
    remove = (remove !== undefined) ? remove : false;
    for (var control in Mixage.connectionMap) {
        if (remove) {
            Mixage.connectionMap[control][1].disconnect();
        } else {
            Mixage.connectionMap[control][1] = engine.makeConnection(group, control, Mixage.connectionMap[control][0]);
        }
    }
};

// Toggle the LED on the MIDI controller by sending a MIDI message
Mixage.toggleLED = function(value, group, control) {
    midi.sendShortMsg(0x90, Mixage.ledMap[group][control], (value === 1) ? 0x7F : 0);
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

// Helper function to scroll the playlist
Mixage.scrollLibrary = function(value) {
    if (Mixage.autoMaximizeLibrary) {
        Mixage.setLibraryMaximized(true);
    }
    //engine.setValue('[Library]', 'MoveVertical', value); // this does not work for me
    engine.setValue("[Playlist]", "SelectTrackKnob", value); // for Mixxx < 2.1
};

// A button for the playlist was pressed
Mixage.handleLibrary = function(channel, control, value, status, _group) {
    // "push2browse" button was moved somehow
    if (control === 0x1F) { // "push2browse" button was pushed
        if (status === 0x90 && value === 0x7F) {
            if (Mixage.autoMaximizeLibrary) {
                Mixage.setLibraryMaximized(true);
            }
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

// Switch the controller and Mixxx state for scratching
Mixage.setScratching = function(deckNr, active) {
    // check if setting changed
    if (Mixage.isScratchActive[deckNr] !== active) {
        Mixage.isScratchActive[deckNr] = active;
        if (active) {
            // turn off scrolling first
            Mixage.setScrolling(deckNr, false);
        } else {
            engine.scratchDisable(deckNr);
        }
        var controlString = "[Channel" + deckNr + "]";
        Mixage.toggleLED(Mixage.isScratchActive[deckNr] ? 1 : 0, controlString, "scratch_active");
    }
};

// The "disc" button that enables/disables scratching
Mixage.scratchToggle = function(channel, control, value, _status, _group) {
    // calculate deck number from MIDI control. 0x04 controls deck 1, 0x12 deck 2
    var deckNr = control === 0x04 ? 1 : 2;
    // check for pressed->release or released->press
    if (Mixage.scratchToggleLastState[deckNr] !== value) {
        Mixage.scratchToggleLastState[deckNr] = value;
        if (value === 0) {
            return;
        }
        Mixage.setScratching(deckNr, !Mixage.isScratchActive[deckNr]);
    }
};

// Switch the controller and Mixxx state for scrolling
Mixage.setScrolling = function(deckNr, active) {
    // check if setting changed
    if (Mixage.isScrollActive[deckNr] !== active) {
        Mixage.isScrollActive[deckNr] = active;
        if (active) {
            // turn off scratching first
            Mixage.setScratching(deckNr, false);
        } else {
            engine.scratchDisable(deckNr);
        }
        var controlString = "[Channel" + deckNr + "]";
        Mixage.toggleLED(Mixage.isScrollActive[deckNr] ? 1 : 0, controlString, "scroll_active");
    }
};

// The "loupe" button that enables/disables track scrolling
Mixage.scrollToggle = function(channel, control, value, _status, _group) {
    // calculate deck number from MIDI control. 0x03 controls deck 1, 0x12 deck 2
    var deckNr = control === 0x03 ? 1 : 2;
    // check for pressed->release or released->press
    if (Mixage.scrollToggleLastState[deckNr] !== value) {
        Mixage.scrollToggleLastState[deckNr] = value;
        if (value === 0) {
            return;
        }
        Mixage.setScrolling(deckNr, !Mixage.isScrollActive[deckNr]);
    }
};

// The touch function on the wheels that enables/disables scratching
Mixage.wheelTouch = function(channel, control, value, _status, _group) {
    // calculate deck number from MIDI control. 0x24 controls deck 1, 0x25 deck 2
    var deckNr = control === 0x24 ? 1 : 2;
    // check if scratching or scrolling should be enabled
    if (Mixage.scratchByWheelTouch || Mixage.isScratchActive[deckNr] || Mixage.isScrollActive[deckNr]) {
        if (value === 0x7F) {
            // turn on scratching or scrolling on this deck
            var alpha = Mixage.isScrollActive[deckNr] ? 1.0 / 2.0 : 1.0 / 8.0;
            var beta = alpha / 32.0;
            var ticksPerRevolution = Mixage.isScrollActive[deckNr] ? 128 : 620;
            engine.scratchEnable(deckNr, ticksPerRevolution, 33.0 + 1.0 / 3.0, alpha, beta);
        } else {
            engine.scratchDisable(deckNr);
        }
    }
};

// The wheel that actually controls the scratching / jogging / library browsing
Mixage.wheelTurn = function(channel, control, value, _status, _group) {
    // calculate deck number from MIDI control. 0x24 controls deck 1, 0x25 deck 2
    var deckNr = control === 0x24 ? 1 : 2;
    // control centers on 0x40 (64), calculate difference to that value
    var newValue = value - 64;
    if (engine.isScratching(deckNr)) {
        engine.scratchTick(deckNr, newValue); // scratch or scroll deck
    } else {
        engine.setValue("[Channel" + deckNr + "]", "jog", newValue); // pitch bend deck
    }
};

// The MASTER button that toggles a deck as sync leader / master
Mixage.handleDeckSyncMode = function(_channel, _control, _value, _status, _group) {
    // Function currently not working as of 2.3.3: https://manual.mixxx.org/2.4/gl/chapters/appendix/mixxx_controls.html#control-[ChannelN]-sync_master
    // Disable until this is working
    /*// calculate effect unit number from MIDI control. 0x46 controls unit 1, 0x54 unit 2
    var deckNr = control === 0x46 ? 1 : 2;
    // react only on first message / keydown
    if (value === 0x7F) {
        // check and toggle enablement
        var controlString = "[Channel" + deckNr + "]";
        var keyString = "sync_master";
        var isSyncLeader = !engine.getValue(controlString, keyString);
        // if we want to make this deck sync leader, turn off sync leader on the other deck
        if (isSyncLeader) {
        	var otherDeckNr = deckNr === 1 ? 2 : 1;
        	engine.setValue("[Channel" + otherDeckNr + "]", keyString, false);
        }
        engine.setValue(controlString, keyString, isSyncLeader);
        Mixage.toggleLED(isSyncLeader ? 1 : 0, controlString, keyString);  
    }*/
};

// The FX ON button that toggles routing channel 1/2 through effects
Mixage.handleEffectChannelOn = function(channel, control, value, _status, _group) {
    // calculate effect unit number from MIDI control. 0x08 controls unit 1, 0x16 unit 2
    var unitNr = control === 0x08 ? 1 : 2;
    // react only on first message / keydown
    if (value === 0x7F) {
        // check and toggle enablement
        Mixage.effectRackEnabled[unitNr] = !Mixage.effectRackEnabled[unitNr];
        var isEnabled = Mixage.effectRackEnabled[unitNr];
        // update controls
        var keyString = "group_[Channel" + unitNr + "]_enable";
        engine.setValue("[EffectRack1_EffectUnit1]", keyString, isEnabled && Mixage.effectRackSelected[unitNr][0]);
        engine.setValue("[EffectRack1_EffectUnit2]", keyString, isEnabled && Mixage.effectRackSelected[unitNr][1]);
        Mixage.toggleLED(isEnabled ? 1 : 0, "[Channel" + unitNr + "]", "fx_on");
    }
};

// The FX SEL button that selects which effects are enabled for channel 1/2
Mixage.handleEffectChannelSelect = function(channel, control, value, _status, _group) {
    // calculate effect unit number from MIDI control. 0x07 controls unit 1, 0x15 unit 2
    var unitNr = control === 0x07 ? 1 : 2;
    // react only on first message / keydown
    if (value === 0x7F) {
        // check and toggle select state
        var selected1 = Mixage.effectRackSelected[unitNr][0];
        var selected2 = Mixage.effectRackSelected[unitNr][1];
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
        Mixage.effectRackSelected[unitNr][0] = selected1;
        Mixage.effectRackSelected[unitNr][1] = selected2;
        // update controls
        var isEnabled = Mixage.effectRackEnabled[unitNr];
        var keyString = "group_[Channel" + unitNr + "]_enable";
        engine.setValue("[EffectRack1_EffectUnit1]", keyString, isEnabled && Mixage.effectRackSelected[unitNr][0]);
        engine.setValue("[EffectRack1_EffectUnit2]", keyString, isEnabled && Mixage.effectRackSelected[unitNr][1]);
    }
};

Mixage.handleEffectDryWet = function(channel, control, value, _status, _group) {
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
Mixage.handleEffectSuper = function(channel, control, value, _status, _group) {
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
Mixage.handleBeatMovePressed = function(channel, control, value, _status, _group) {
    Mixage.beatMovePressed = value === 0x7f;
};

Mixage.handleBeatMoveLength = function(channel, control, value, _status, _group) {
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

Mixage.handleBeatMove = function(channel, control, value, _status, _group) {
    // calculate effect unit number from MIDI control. 0x5f controls unit 1, 0x61 unit 2
    var unitNr = control === 0x5f ? 1 : 2;
    var direction = unitNr === 1 ? 1 : -1;
    // Control centers on 0x40 (64), calculate difference to that
    var diff = (value - 64);
    // In either case, register the movement
    var position = direction * diff > 0 ? "beatjump_forward" : "beatjump_backward";
    engine.setValue("[Channel" + unitNr + "]", position, true);
};

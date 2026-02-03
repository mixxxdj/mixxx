// Pioneer-DDJ-FLX4-Player2-script.js
// ****************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-FLX4 (Player 2 Configuration).
// * This mapping is designed for a second controller that defaults to decks 3-4.
// * Based on the standard DDJ-FLX4 mapping.
// *
// * Features:
// *   - Controls decks 3 and 4 by default
// *   - Double-tap SHIFT to toggle between decks 3-4 and 1-2
// *   - Uses Headphones 2 output when controlling decks 3-4
// *   - Uses Headphones 1 output when controlling decks 1-2
// *
// * Authors: Original DDJ-FLX4 mapping by Warker, nschloe, dj3730, jusko, Robert904
// *          Player 2 adaptation for multi-headphone support
// ****************************************************************************

var PioneerDDJFLX4P2 = {};

// Deck configuration - Player 2 defaults to decks 3 and 4
PioneerDDJFLX4P2.deckPairs = {
    primary: {left: 3, right: 4},    // Default: decks 3-4
    secondary: {left: 1, right: 2}   // Alternative: decks 1-2
};
PioneerDDJFLX4P2.usingPrimaryDecks = true;

// Double-tap detection for shift button
PioneerDDJFLX4P2.shiftDoubleTapTime = 300; // ms window for double-tap
PioneerDDJFLX4P2.lastShiftPress = [0, 0];  // timestamps for each deck side

// Get the current deck group based on channel (0=left, 1=right) and deck pair mode
PioneerDDJFLX4P2.getDeckGroup = function(channel) {
    if (PioneerDDJFLX4P2.usingPrimaryDecks) {
        return "[Channel" + (channel === 0 ? PioneerDDJFLX4P2.deckPairs.primary.left : PioneerDDJFLX4P2.deckPairs.primary.right) + "]";
    } else {
        return "[Channel" + (channel === 0 ? PioneerDDJFLX4P2.deckPairs.secondary.left : PioneerDDJFLX4P2.deckPairs.secondary.right) + "]";
    }
};

// Get the deck number for a channel
PioneerDDJFLX4P2.getDeckNumber = function(channel) {
    if (PioneerDDJFLX4P2.usingPrimaryDecks) {
        return channel === 0 ? PioneerDDJFLX4P2.deckPairs.primary.left : PioneerDDJFLX4P2.deckPairs.primary.right;
    } else {
        return channel === 0 ? PioneerDDJFLX4P2.deckPairs.secondary.left : PioneerDDJFLX4P2.deckPairs.secondary.right;
    }
};

// Get the appropriate headphone group for current deck pair
PioneerDDJFLX4P2.getHeadphoneGroup = function() {
    // Decks 3-4 use Headphone 2, Decks 1-2 use Headphone 1
    return PioneerDDJFLX4P2.usingPrimaryDecks ? "[Headphone2]" : "[Headphone1]";
};

PioneerDDJFLX4P2.lights = {
    beatFx: {
        status: 0x94,
        data1: 0x47,
    },
    shiftBeatFx: {
        status: 0x94,
        data1: 0x43,
    },
    deck1: {
        vuMeter: {
            status: 0xB0,
            data1: 0x02,
        },
        playPause: {
            status: 0x90,
            data1: 0x0B,
        },
        shiftPlayPause: {
            status: 0x90,
            data1: 0x47,
        },
        cue: {
            status: 0x90,
            data1: 0x0C,
        },
        shiftCue: {
            status: 0x90,
            data1: 0x48,
        },
        hotcueMode: {
            status: 0x90,
            data1: 0x1B,
        },
        keyboardMode: {
            status: 0x90,
            data1: 0x69,
        },
        padFX1Mode: {
            status: 0x90,
            data1: 0x1E,
        },
        padFX2Mode: {
            status: 0x90,
            data1: 0x6B,
        },
        beatJumpMode: {
            status: 0x90,
            data1: 0x20,
        },
        beatLoopMode: {
            status: 0x90,
            data1: 0x6D,
        },
        samplerMode: {
            status: 0x90,
            data1: 0x22,
        },
        keyShiftMode: {
            status: 0x90,
            data1: 0x6F,
        },
    },
    deck2: {
        vuMeter: {
            status: 0xB0,
            data1: 0x02,
        },
        playPause: {
            status: 0x91,
            data1: 0x0B,
        },
        shiftPlayPause: {
            status: 0x91,
            data1: 0x47,
        },
        cue: {
            status: 0x91,
            data1: 0x0C,
        },
        shiftCue: {
            status: 0x91,
            data1: 0x48,
        },
        hotcueMode: {
            status: 0x91,
            data1: 0x1B,
        },
        keyboardMode: {
            status: 0x91,
            data1: 0x69,
        },
        padFX1Mode: {
            status: 0x91,
            data1: 0x1E,
        },
        padFX2Mode: {
            status: 0x91,
            data1: 0x6B,
        },
        beatJumpMode: {
            status: 0x91,
            data1: 0x20,
        },
        beatLoopMode: {
            status: 0x91,
            data1: 0x6D,
        },
        samplerMode: {
            status: 0x91,
            data1: 0x22,
        },
        keyShiftMode: {
            status: 0x91,
            data1: 0x6F,
        },
    },
};

// Store timer IDs
PioneerDDJFLX4P2.timers = {};

// Keep alive timer
PioneerDDJFLX4P2.sendKeepAlive = function() {
    midi.sendSysexMsg([0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x05, 0x00, 0x50, 0x02, 0xf7], 12);
};

// Jog wheel constants
PioneerDDJFLX4P2.vinylMode = true;
PioneerDDJFLX4P2.alpha = 1.0/8;
PioneerDDJFLX4P2.beta = PioneerDDJFLX4P2.alpha/32;

// Multiplier for fast seek through track using SHIFT+JOGWHEEL
PioneerDDJFLX4P2.fastSeekScale = 150;
PioneerDDJFLX4P2.bendScale = 0.8;

PioneerDDJFLX4P2.tempoRanges = [0.06, 0.10, 0.16, 0.25];

PioneerDDJFLX4P2.shiftButtonDown = [false, false];

// Jog wheel loop adjust
PioneerDDJFLX4P2.loopAdjustIn = [false, false];
PioneerDDJFLX4P2.loopAdjustOut = [false, false];
PioneerDDJFLX4P2.loopAdjustMultiply = 50;

// Beatjump pad (beatjump_size values)
PioneerDDJFLX4P2.beatjumpSizeForPad = {
    0x20: -1, // PAD 1
    0x21: 1,  // PAD 2
    0x22: -2, // PAD 3
    0x23: 2,  // PAD 4
    0x24: -4, // PAD 5
    0x25: 4,  // PAD 6
    0x26: -8, // PAD 7
    0x27: 8   // PAD 8
};

// Stems (KEYBOARD) pads mode status - dynamically set based on current deck pair
PioneerDDJFLX4P2.getStemsPadsModesStatus = function() {
    const leftDeck = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightDeck = PioneerDDJFLX4P2.getDeckGroup(1);
    const result = {};
    result[leftDeck] = [0x97, 0x98];
    result[rightDeck] = [0x99, 0x9a];
    return result;
};

// Stems (KEYBOARD) pad 1 control
PioneerDDJFLX4P2.stemMutePadsFirstControl = 0x40;

// Stems (KEYBOARD) pad 5 control
PioneerDDJFLX4P2.stemFxPadsFirstControl = 0x44;

// Pitch shift (KEY SHIFT) pads mode status - dynamically set based on current deck pair
PioneerDDJFLX4P2.getPitchPadsModesStatus = function() {
    const leftDeck = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightDeck = PioneerDDJFLX4P2.getDeckGroup(1);
    const result = {};
    result[leftDeck] = [0x97, 0x98];
    result[rightDeck] = [0x99, 0x9a];
    return result;
};

// Pitch shift (KEY SHIFT) pad 1 control
PioneerDDJFLX4P2.pitchPadsFirstControl = 0x70;

PioneerDDJFLX4P2.quickJumpSize = 32;

// Used for tempo slider
PioneerDDJFLX4P2.highResMSB = {};

PioneerDDJFLX4P2.trackLoadedLED = function(value, group, _control) {
    // Extract deck number and map to controller LED
    const deckNum = parseInt(group.match(/\[Channel(\d+)\]/)[1]);
    let ledChannel;

    if (PioneerDDJFLX4P2.usingPrimaryDecks) {
        if (deckNum === PioneerDDJFLX4P2.deckPairs.primary.left) {
            ledChannel = 0;
        } else if (deckNum === PioneerDDJFLX4P2.deckPairs.primary.right) {
            ledChannel = 1;
        } else {
            return;
        }
    } else {
        if (deckNum === PioneerDDJFLX4P2.deckPairs.secondary.left) {
            ledChannel = 0;
        } else if (deckNum === PioneerDDJFLX4P2.deckPairs.secondary.right) {
            ledChannel = 1;
        } else {
            return;
        }
    }

    midi.sendShortMsg(0x9F, ledChannel, value > 0 ? 0x7F : 0x00);
};

PioneerDDJFLX4P2.toggleLight = function(midiIn, active) {
    midi.sendShortMsg(midiIn.status, midiIn.data1, active ? 0x7F : 0);
};

// Flash all deck LEDs to indicate deck switch
PioneerDDJFLX4P2.flashDeckSwitchIndicator = function() {
    // Flash the track loaded LEDs to indicate deck switch
    midi.sendShortMsg(0x9F, 0x00, 0x7F);
    midi.sendShortMsg(0x9F, 0x01, 0x7F);

    engine.beginTimer(200, function() {
        midi.sendShortMsg(0x9F, 0x00, 0x00);
        midi.sendShortMsg(0x9F, 0x01, 0x00);
    }, true);

    engine.beginTimer(400, function() {
        midi.sendShortMsg(0x9F, 0x00, 0x7F);
        midi.sendShortMsg(0x9F, 0x01, 0x7F);
    }, true);

    engine.beginTimer(600, function() {
        // Final state based on track loaded status
        const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
        const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);
        midi.sendShortMsg(0x9F, 0x00, engine.getValue(leftGroup, "track_loaded") ? 0x7F : 0x00);
        midi.sendShortMsg(0x9F, 0x01, engine.getValue(rightGroup, "track_loaded") ? 0x7F : 0x00);
    }, true);
};

// Toggle between deck pairs
PioneerDDJFLX4P2.toggleDeckPair = function() {
    PioneerDDJFLX4P2.usingPrimaryDecks = !PioneerDDJFLX4P2.usingPrimaryDecks;

    const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);

    print("DDJ-FLX4 Player 2: Switched to decks " +
          PioneerDDJFLX4P2.getDeckNumber(0) + " and " +
          PioneerDDJFLX4P2.getDeckNumber(1));
    print("DDJ-FLX4 Player 2: Using " + PioneerDDJFLX4P2.getHeadphoneGroup() + " for cue");

    // Flash LEDs to indicate switch
    PioneerDDJFLX4P2.flashDeckSwitchIndicator();

    // Update VU meter connections
    PioneerDDJFLX4P2.reconnectVuMeters();

    // Re-initialize high-res MSB storage for new deck groups
    PioneerDDJFLX4P2.highResMSB[leftGroup] = PioneerDDJFLX4P2.highResMSB[leftGroup] || {};
    PioneerDDJFLX4P2.highResMSB[rightGroup] = PioneerDDJFLX4P2.highResMSB[rightGroup] || {};
};

// Reconnect VU meters to current deck pair
PioneerDDJFLX4P2.vuMeterConnections = [];

PioneerDDJFLX4P2.reconnectVuMeters = function() {
    // Disconnect old connections
    PioneerDDJFLX4P2.vuMeterConnections.forEach(function(conn) {
        conn.disconnect();
    });
    PioneerDDJFLX4P2.vuMeterConnections = [];

    // Connect to current deck pair
    const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);

    PioneerDDJFLX4P2.vuMeterConnections.push(
        engine.makeConnection(leftGroup, "vu_meter", PioneerDDJFLX4P2.vuMeterUpdateLeft)
    );
    PioneerDDJFLX4P2.vuMeterConnections.push(
        engine.makeConnection(rightGroup, "vu_meter", PioneerDDJFLX4P2.vuMeterUpdateRight)
    );
};

//
// Init
//

PioneerDDJFLX4P2.init = function() {
    print("DDJ-FLX4 Player 2: Initializing for decks 3 and 4");

    engine.setValue("[EffectRack1_EffectUnit2]", "show_focus", 1);

    // Initialize high-res MSB storage for default deck groups
    const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);
    PioneerDDJFLX4P2.highResMSB[leftGroup] = {};
    PioneerDDJFLX4P2.highResMSB[rightGroup] = {};
    // Also initialize for the alternative deck pair
    PioneerDDJFLX4P2.highResMSB["[Channel1]"] = {};
    PioneerDDJFLX4P2.highResMSB["[Channel2]"] = {};

    PioneerDDJFLX4P2.reconnectVuMeters();

    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.deck1.vuMeter, false);
    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.deck2.vuMeter, false);

    // Soft takeover for all 4 channels (since we can switch between them)
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);
    engine.softTakeover("[Channel3]", "rate", true);
    engine.softTakeover("[Channel4]", "rate", true);

    // Use Effect Unit 2 for Player 2
    engine.softTakeover("[EffectRack1_EffectUnit2_Effect1]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit2_Effect2]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit2_Effect3]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit2]", "mix", true);

    const samplerCount = 16;
    if (engine.getValue("[App]", "num_samplers") < samplerCount) {
        engine.setValue("[App]", "num_samplers", samplerCount);
    }
    for (let i = 1; i <= samplerCount; ++i) {
        engine.makeConnection("[Sampler" + i + "]", "play", PioneerDDJFLX4P2.samplerPlayOutputCallbackFunction);
    }

    // Connect track loaded LED for all 4 channels
    engine.makeConnection("[Channel1]", "track_loaded", PioneerDDJFLX4P2.trackLoadedLED);
    engine.makeConnection("[Channel2]", "track_loaded", PioneerDDJFLX4P2.trackLoadedLED);
    engine.makeConnection("[Channel3]", "track_loaded", PioneerDDJFLX4P2.trackLoadedLED);
    engine.makeConnection("[Channel4]", "track_loaded", PioneerDDJFLX4P2.trackLoadedLED);

    // play the "track loaded" animation on both decks at startup
    midi.sendShortMsg(0x9F, 0x00, 0x7F);
    midi.sendShortMsg(0x9F, 0x01, 0x7F);

    PioneerDDJFLX4P2.setLoopButtonLights(0x90, 0x7F);
    PioneerDDJFLX4P2.setLoopButtonLights(0x91, 0x7F);

    // Connect loop enabled for all 4 channels
    engine.makeConnection("[Channel1]", "loop_enabled", PioneerDDJFLX4P2.loopToggle);
    engine.makeConnection("[Channel2]", "loop_enabled", PioneerDDJFLX4P2.loopToggle);
    engine.makeConnection("[Channel3]", "loop_enabled", PioneerDDJFLX4P2.loopToggle);
    engine.makeConnection("[Channel4]", "loop_enabled", PioneerDDJFLX4P2.loopToggle);

    // Use Effect Unit 2 for Player 2
    for (let i = 1; i <= 3; i++) {
        engine.makeConnection("[EffectRack1_EffectUnit2_Effect" + i +"]", "enabled", PioneerDDJFLX4P2.toggleFxLight);
    }
    engine.makeConnection("[EffectRack1_EffectUnit2]", "focused_effect", PioneerDDJFLX4P2.toggleFxLight);

    // Register callbacks for stems on all 4 channels
    for (let deck = 1; deck <= 4; deck++) {
        engine.makeConnection("[Channel" + deck + "]", "stem_count", PioneerDDJFLX4P2.stemCountChanged);
        for (let stem = 1; stem <= 4; stem++) {
            engine.makeConnection("[Channel" + deck + "_Stem" + stem + "]", "mute", PioneerDDJFLX4P2.stemMuteChanged);
            engine.makeConnection("[QuickEffectRack1_[Channel" + deck + "_Stem" + stem + "]]", "enabled", PioneerDDJFLX4P2.stemFxChanged);
        }
    }

    // Register callbacks for pitch shift on all 4 channels
    for (let deck = 1; deck <= 4; deck++) {
        engine.makeConnection("[Channel" + deck + "]", "track_loaded", PioneerDDJFLX4P2.pitchAdjusted);
        engine.makeConnection("[Channel" + deck + "]", "pitch_adjust", PioneerDDJFLX4P2.pitchAdjusted);
    }

    PioneerDDJFLX4P2.keepAliveTimer = engine.beginTimer(200, PioneerDDJFLX4P2.sendKeepAlive);

    // query the controller for current control positions on startup
    PioneerDDJFLX4P2.sendKeepAlive();

    print("DDJ-FLX4 Player 2: Ready - Double-tap SHIFT to toggle between deck pairs");
};

//
// Waveform zoom
//

PioneerDDJFLX4P2.waveformZoom = function(midichan, control, value, status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(0); // Use left deck for zoom
    if (value === 0x7f) {
        script.triggerControl(actualGroup, "waveform_zoom_up", 100);
    } else {
        script.triggerControl(actualGroup, "waveform_zoom_down", 100);
    }
};

//
// Channel level lights
//

PioneerDDJFLX4P2.vuMeterUpdateLeft = function(value, group) {
    const newVal = value * 127;
    midi.sendShortMsg(0xB0, 0x02, newVal);
};

PioneerDDJFLX4P2.vuMeterUpdateRight = function(value, group) {
    const newVal = value * 127;
    midi.sendShortMsg(0xB1, 0x02, newVal);
};

//
// Effects - Using Effect Unit 2 for Player 2
//

PioneerDDJFLX4P2.toggleFxLight = function(_value, _group, _control) {
    const enabled = engine.getValue(PioneerDDJFLX4P2.focusedFxGroup(), "enabled");

    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.beatFx, enabled);
    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.shiftBeatFx, enabled);
};

PioneerDDJFLX4P2.focusedFxGroup = function() {
    const focusedFx = engine.getValue("[EffectRack1_EffectUnit2]", "focused_effect");
    return "[EffectRack1_EffectUnit2_Effect" + focusedFx + "]";
};

PioneerDDJFLX4P2.beatFxLevelDepthRotate = function(_channel, _control, value) {
    if (PioneerDDJFLX4P2.shiftButtonDown[0] || PioneerDDJFLX4P2.shiftButtonDown[1]) {
        engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit2]", "mix");
        engine.setParameter(PioneerDDJFLX4P2.focusedFxGroup(), "meta", value / 0x7F);
    } else {
        engine.softTakeoverIgnoreNextValue(PioneerDDJFLX4P2.focusedFxGroup(), "meta");
        engine.setParameter("[EffectRack1_EffectUnit2]", "mix", value / 0x7F);
    }
};

PioneerDDJFLX4P2.changeFocusedEffectBy = function(numberOfSteps) {
    let focusedEffect = engine.getValue("[EffectRack1_EffectUnit2]", "focused_effect");
    focusedEffect -= 1;
    const numberOfEffectsPerEffectUnit = 3;
    focusedEffect = (((focusedEffect + numberOfSteps) % numberOfEffectsPerEffectUnit) + numberOfEffectsPerEffectUnit) % numberOfEffectsPerEffectUnit;
    focusedEffect += 1;
    engine.setValue("[EffectRack1_EffectUnit2]", "focused_effect", focusedEffect);
};

PioneerDDJFLX4P2.beatFxSelectPressed = function(_channel, _control, value) {
    if (value === 0) { return; }
    engine.setValue(PioneerDDJFLX4P2.focusedFxGroup(), "next_effect", value);
};

PioneerDDJFLX4P2.beatFxSelectShiftPressed = function(_channel, _control, value) {
    if (value === 0) { return; }
    engine.setValue(PioneerDDJFLX4P2.focusedFxGroup(), "prev_effect", value);
};

PioneerDDJFLX4P2.beatFxLeftPressed = function(_channel, _control, value) {
    if (value === 0) { return; }
    PioneerDDJFLX4P2.changeFocusedEffectBy(-1);
};

PioneerDDJFLX4P2.beatFxRightPressed = function(_channel, _control, value) {
    if (value === 0) { return; }
    PioneerDDJFLX4P2.changeFocusedEffectBy(1);
};

PioneerDDJFLX4P2.beatFxOnOffPressed = function(_channel, _control, value) {
    if (value === 0) { return; }
    const toggleEnabled = !engine.getValue(PioneerDDJFLX4P2.focusedFxGroup(), "enabled");
    engine.setValue(PioneerDDJFLX4P2.focusedFxGroup(), "enabled", toggleEnabled);
};

PioneerDDJFLX4P2.beatFxOnOffShiftPressed = function(_channel, _control, value) {
    if (value === 0) { return; }
    engine.setParameter("[EffectRack1_EffectUnit2]", "mix", 0);
    engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit2]", "mix");
    for (let i = 1; i <= 3; i++) {
        engine.setValue("[EffectRack1_EffectUnit2_Effect" + i + "]", "enabled", 0);
    }
    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.beatFx, false);
    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.shiftBeatFx, false);
};

PioneerDDJFLX4P2.beatFxChannel = function(channel, control, value, _status, group) {
    const enableChannel = value === 0x7f ? 1 : 0;
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(group, "group_" + actualGroup + "_enable", enableChannel);
};

//
// Loop IN/OUT ADJUST
//

PioneerDDJFLX4P2.toggleLoopAdjustIn = function(channel, _control, value, _status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    if (value === 0 || engine.getValue(actualGroup, "loop_enabled" === 0)) {
        return;
    }
    PioneerDDJFLX4P2.loopAdjustIn[channel] = !PioneerDDJFLX4P2.loopAdjustIn[channel];
    PioneerDDJFLX4P2.loopAdjustOut[channel] = false;
};

PioneerDDJFLX4P2.toggleLoopAdjustOut = function(channel, _control, value, _status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    if (value === 0 || engine.getValue(actualGroup, "loop_enabled" === 0)) {
        return;
    }
    PioneerDDJFLX4P2.loopAdjustOut[channel] = !PioneerDDJFLX4P2.loopAdjustOut[channel];
    PioneerDDJFLX4P2.loopAdjustIn[channel] = false;
};

PioneerDDJFLX4P2.setReloopLight = function(status, value) {
    midi.sendShortMsg(status, 0x4D, value);
    midi.sendShortMsg(status, 0x50, value);
};

PioneerDDJFLX4P2.setLoopButtonLights = function(status, value) {
    [0x10, 0x11, 0x4E, 0x4C].forEach(function(control) {
        midi.sendShortMsg(status, control, value);
    });
};

PioneerDDJFLX4P2.startLoopLightsBlink = function(channel, control, status, group) {
    let blink = 0x7F;

    PioneerDDJFLX4P2.stopLoopLightsBlink(group, control, status);

    PioneerDDJFLX4P2.timers[group][control] = engine.beginTimer(500, () => {
        blink = 0x7F - blink;

        if (PioneerDDJFLX4P2.loopAdjustOut[channel]) {
            midi.sendShortMsg(status, 0x10, 0x00);
            midi.sendShortMsg(status, 0x4C, 0x00);
        } else {
            midi.sendShortMsg(status, 0x10, blink);
            midi.sendShortMsg(status, 0x4C, blink);
        }

        if (PioneerDDJFLX4P2.loopAdjustIn[channel]) {
            midi.sendShortMsg(status, 0x11, 0x00);
            midi.sendShortMsg(status, 0x4E, 0x00);
        } else {
            midi.sendShortMsg(status, 0x11, blink);
            midi.sendShortMsg(status, 0x4E, blink);
        }
    });
};

PioneerDDJFLX4P2.stopLoopLightsBlink = function(group, control, status) {
    PioneerDDJFLX4P2.timers[group] = PioneerDDJFLX4P2.timers[group] || {};

    if (PioneerDDJFLX4P2.timers[group][control] !== undefined) {
        engine.stopTimer(PioneerDDJFLX4P2.timers[group][control]);
    }
    PioneerDDJFLX4P2.timers[group][control] = undefined;
    PioneerDDJFLX4P2.setLoopButtonLights(status, 0x7F);
};

PioneerDDJFLX4P2.loopToggle = function(value, group, control) {
    // Check if this group is currently active
    const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);

    let status, channel;
    if (group === leftGroup) {
        status = 0x90;
        channel = 0;
    } else if (group === rightGroup) {
        status = 0x91;
        channel = 1;
    } else {
        return; // Not our active deck
    }

    PioneerDDJFLX4P2.setReloopLight(status, value ? 0x7F : 0x00);

    if (value) {
        PioneerDDJFLX4P2.startLoopLightsBlink(channel, control, status, group);
    } else {
        PioneerDDJFLX4P2.stopLoopLightsBlink(group, control, status);
        PioneerDDJFLX4P2.loopAdjustIn[channel] = false;
        PioneerDDJFLX4P2.loopAdjustOut[channel] = false;
    }
};

//
// CUE/LOOP CALL
//

PioneerDDJFLX4P2.cueLoopCallLeft = function(channel, _control, value, _status, group) {
    if (value) {
        const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
        engine.setValue(actualGroup, "loop_scale", 0.5);
    }
};

PioneerDDJFLX4P2.cueLoopCallRight = function(channel, _control, value, _status, group) {
    if (value) {
        const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
        engine.setValue(actualGroup, "loop_scale", 2.0);
    }
};

//
// BEAT SYNC
//

PioneerDDJFLX4P2.syncPressed = function(channel, control, value, status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    if (engine.getValue(actualGroup, "sync_enabled") && value > 0) {
        engine.setValue(actualGroup, "sync_enabled", 0);
    } else {
        engine.setValue(actualGroup, "beatsync", value);
    }
};

PioneerDDJFLX4P2.syncLongPressed = function(channel, control, value, status, group) {
    if (value) {
        const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
        engine.setValue(actualGroup, "sync_enabled", 1);
    }
};

PioneerDDJFLX4P2.cycleTempoRange = function(channel, _control, value, _status, group) {
    if (value === 0) { return; }

    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const currRange = engine.getValue(actualGroup, "rateRange");
    let idx = 0;

    for (let i = 0; i < PioneerDDJFLX4P2.tempoRanges.length; i++) {
        if (currRange === PioneerDDJFLX4P2.tempoRanges[i]) {
            idx = (i + 1) % PioneerDDJFLX4P2.tempoRanges.length;
            break;
        }
    }
    engine.setValue(actualGroup, "rateRange", PioneerDDJFLX4P2.tempoRanges[idx]);
};

//
// Jog wheels
//

PioneerDDJFLX4P2.jogTurn = function(channel, _control, value, _status, group) {
    const deckNum = PioneerDDJFLX4P2.getDeckNumber(channel);
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    let newVal = value - 64;

    const loopEnabled = engine.getValue(actualGroup, "loop_enabled");
    if (loopEnabled > 0) {
        if (PioneerDDJFLX4P2.loopAdjustIn[channel]) {
            newVal = newVal * PioneerDDJFLX4P2.loopAdjustMultiply + engine.getValue(actualGroup, "loop_start_position");
            engine.setValue(actualGroup, "loop_start_position", newVal);
            return;
        }
        if (PioneerDDJFLX4P2.loopAdjustOut[channel]) {
            newVal = newVal * PioneerDDJFLX4P2.loopAdjustMultiply + engine.getValue(actualGroup, "loop_end_position");
            engine.setValue(actualGroup, "loop_end_position", newVal);
            return;
        }
    }

    if (engine.isScratching(deckNum)) {
        engine.scratchTick(deckNum, newVal);
    } else {
        engine.setValue(actualGroup, "jog", newVal * PioneerDDJFLX4P2.bendScale);
    }
};

PioneerDDJFLX4P2.jogSearch = function(channel, _control, value, _status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const newVal = (value - 64) * PioneerDDJFLX4P2.fastSeekScale;
    engine.setValue(actualGroup, "jog", newVal);
};

PioneerDDJFLX4P2.jogTouch = function(channel, _control, value) {
    const deckNum = PioneerDDJFLX4P2.getDeckNumber(channel);

    if (PioneerDDJFLX4P2.loopAdjustIn[channel] || PioneerDDJFLX4P2.loopAdjustOut[channel]) {
        return;
    }

    if (value !== 0 && PioneerDDJFLX4P2.vinylMode) {
        engine.scratchEnable(deckNum, 720, 33+1/3, PioneerDDJFLX4P2.alpha, PioneerDDJFLX4P2.beta);
    } else {
        engine.scratchDisable(deckNum);
    }
};

//
// Shift button - with double-tap deck switching
//

PioneerDDJFLX4P2.shiftPressed = function(channel, _control, value, _status, _group) {
    const isPressed = value === 0x7F;
    PioneerDDJFLX4P2.shiftButtonDown[channel] = isPressed;

    if (isPressed) {
        const now = Date.now();
        const timeSinceLastPress = now - PioneerDDJFLX4P2.lastShiftPress[channel];

        if (timeSinceLastPress < PioneerDDJFLX4P2.shiftDoubleTapTime) {
            // Double-tap detected - toggle deck pair
            PioneerDDJFLX4P2.toggleDeckPair();
            PioneerDDJFLX4P2.lastShiftPress[channel] = 0; // Reset to prevent triple-tap
        } else {
            PioneerDDJFLX4P2.lastShiftPress[channel] = now;
        }
    }
};

//
// Tempo sliders
//

PioneerDDJFLX4P2.tempoSliderMSB = function(channel, control, value, status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    PioneerDDJFLX4P2.highResMSB[actualGroup] = PioneerDDJFLX4P2.highResMSB[actualGroup] || {};
    PioneerDDJFLX4P2.highResMSB[actualGroup].tempoSlider = value;
};

PioneerDDJFLX4P2.tempoSliderLSB = function(channel, control, value, status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    PioneerDDJFLX4P2.highResMSB[actualGroup] = PioneerDDJFLX4P2.highResMSB[actualGroup] || {};
    const fullValue = (PioneerDDJFLX4P2.highResMSB[actualGroup].tempoSlider << 7) + value;
    engine.setValue(actualGroup, "rate", 1 - (fullValue / 0x2000));
};

//
// Beat Jump mode
//

PioneerDDJFLX4P2.beatjumpPadPressed = function(channel, control, value, _status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "beatjump_size", Math.abs(PioneerDDJFLX4P2.beatjumpSizeForPad[control]));
    engine.setValue(actualGroup, "beatjump", PioneerDDJFLX4P2.beatjumpSizeForPad[control]);
};

PioneerDDJFLX4P2.increaseBeatjumpSizes = function(channel, control, value, _status, group) {
    if (value === 0 || PioneerDDJFLX4P2.beatjumpSizeForPad[0x21] * 16 > 16) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    Object.keys(PioneerDDJFLX4P2.beatjumpSizeForPad).forEach(function(pad) {
        PioneerDDJFLX4P2.beatjumpSizeForPad[pad] = PioneerDDJFLX4P2.beatjumpSizeForPad[pad] * 16;
    });
    engine.setValue(actualGroup, "beatjump_size", PioneerDDJFLX4P2.beatjumpSizeForPad[0x21]);
};

PioneerDDJFLX4P2.decreaseBeatjumpSizes = function(channel, control, value, _status, group) {
    if (value === 0 || PioneerDDJFLX4P2.beatjumpSizeForPad[0x21] / 16 < 1/16) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    Object.keys(PioneerDDJFLX4P2.beatjumpSizeForPad).forEach(function(pad) {
        PioneerDDJFLX4P2.beatjumpSizeForPad[pad] = PioneerDDJFLX4P2.beatjumpSizeForPad[pad] / 16;
    });
    engine.setValue(actualGroup, "beatjump_size", PioneerDDJFLX4P2.beatjumpSizeForPad[0x21]);
};

//
// Sampler mode
//

PioneerDDJFLX4P2.samplerPlayOutputCallbackFunction = function(value, group, _control) {
    if (value === 1) {
        const curPad = group.match(script.samplerRegEx)[1];
        let deckIndex = 0;
        let padIndex = 0;

        if (curPad >=1 && curPad <= 4) {
            deckIndex = 0;
            padIndex = curPad - 1;
        } else if (curPad >=5 && curPad <= 8) {
            deckIndex = 2;
            padIndex = curPad - 5;
        } else if (curPad >=9 && curPad <= 12) {
            deckIndex = 0;
            padIndex = curPad - 5;
        } else if (curPad >=13 && curPad <= 16) {
            deckIndex = 2;
            padIndex = curPad - 9;
        }

        PioneerDDJFLX4P2.startSamplerBlink(0x97 + deckIndex, 0x30 + padIndex, group);
    }
};

PioneerDDJFLX4P2.padModeKeyPressed = function(_channel, _control, value, _status, _group) {
    const deck = (_status === 0x90 ? PioneerDDJFLX4P2.lights.deck1 : PioneerDDJFLX4P2.lights.deck2);

    if (_control === 0x1B) {
        PioneerDDJFLX4P2.toggleLight(deck.hotcueMode, true);
    } else if (_control === 0x69) {
        PioneerDDJFLX4P2.toggleLight(deck.keyboardMode, true);
    } else if (_control === 0x1E) {
        PioneerDDJFLX4P2.toggleLight(deck.padFX1Mode, true);
    } else if (_control === 0x6B) {
        PioneerDDJFLX4P2.toggleLight(deck.padFX2Mode, true);
    } else if (_control === 0x20) {
        PioneerDDJFLX4P2.toggleLight(deck.beatJumpMode, true);
    } else if (_control === 0x6D) {
        PioneerDDJFLX4P2.toggleLight(deck.beatLoopMode, true);
    } else if (_control === 0x22) {
        PioneerDDJFLX4P2.toggleLight(deck.samplerMode, true);
    } else if (_control === 0x6F) {
        PioneerDDJFLX4P2.toggleLight(deck.keyShiftMode, true);
    }
};

PioneerDDJFLX4P2.samplerPadPressed = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "cue_gotoandplay", value);
    } else {
        engine.setValue(group, "LoadSelectedTrack", value);
    }
};

PioneerDDJFLX4P2.samplerPadShiftPressed = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "play")) {
        engine.setValue(group, "cue_gotoandstop", value);
    } else if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "eject", value);
    }
};

PioneerDDJFLX4P2.startSamplerBlink = function(channel, control, group) {
    let val = 0x7f;

    PioneerDDJFLX4P2.stopSamplerBlink(channel, control);
    PioneerDDJFLX4P2.timers[channel][control] = engine.beginTimer(250, () => {
        val = 0x7f - val;
        midi.sendShortMsg(channel, control, val);
        midi.sendShortMsg((channel+1), control, val);

        const isPlaying = engine.getValue(group, "play") === 1;

        if (!isPlaying) {
            PioneerDDJFLX4P2.stopSamplerBlink(channel, control);
            midi.sendShortMsg(channel, control, 0x7f);
            midi.sendShortMsg((channel+1), control, 0x7f);
        }
    });
};

PioneerDDJFLX4P2.stopSamplerBlink = function(channel, control) {
    PioneerDDJFLX4P2.timers[channel] = PioneerDDJFLX4P2.timers[channel] || {};

    if (PioneerDDJFLX4P2.timers[channel][control] !== undefined) {
        engine.stopTimer(PioneerDDJFLX4P2.timers[channel][control]);
        PioneerDDJFLX4P2.timers[channel][control] = undefined;
    }
};

PioneerDDJFLX4P2.toggleQuantize = function(channel, _control, value, _status, group) {
    if (value) {
        const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
        script.toggleControl(actualGroup, "quantize");
    }
};

PioneerDDJFLX4P2.quickJumpForward = function(channel, _control, value, _status, group) {
    if (value) {
        const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
        engine.setValue(actualGroup, "beatjump", PioneerDDJFLX4P2.quickJumpSize);
    }
};

PioneerDDJFLX4P2.quickJumpBack = function(channel, _control, value, _status, group) {
    if (value) {
        const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
        engine.setValue(actualGroup, "beatjump", -PioneerDDJFLX4P2.quickJumpSize);
    }
};

//
// Generic control handler - routes to appropriate deck
//

PioneerDDJFLX4P2.deckControl = function(channel, control, value, status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    // Extract control name from group (e.g., "[Channel1]" -> control name from mapping)
    // This is used for simple controls that just need group remapping
    engine.setValue(actualGroup, control, value);
};

// Play/Pause handler
PioneerDDJFLX4P2.playPause = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    script.toggleControl(actualGroup, "play");
};

// Cue handler
PioneerDDJFLX4P2.cueDefault = function(channel, control, value, status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "cue_default", value > 0 ? 1 : 0);
};

// Reverse roll (censor)
PioneerDDJFLX4P2.reverseRoll = function(channel, control, value, status, group) {
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "reverseroll", value > 0 ? 1 : 0);
};

// Start play
PioneerDDJFLX4P2.startPlay = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "start_play", 1);
};

// Load track
PioneerDDJFLX4P2.loadTrack = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "LoadSelectedTrack", 1);
};

// Headphone cue toggle - uses appropriate headphone output
PioneerDDJFLX4P2.headphoneCue = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    script.toggleControl(actualGroup, "pfl");
};

//
// Stems mode
//

PioneerDDJFLX4P2.stemMutePadPressed = function(channel, control, value, _status, group) {
    if (value !== 0x7f) { return; }

    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const stemCount = Math.min(engine.getValue(actualGroup, "stem_count"), 4);

    if (control - PioneerDDJFLX4P2.stemMutePadsFirstControl + 1 > stemCount) { return; }

    const deckName = actualGroup.substring(1, actualGroup.length-1);
    const stemGroup = "[" + deckName + "_Stem" + (control - PioneerDDJFLX4P2.stemMutePadsFirstControl + 1) + "]";

    if (engine.getValue(stemGroup, "mute")) {
        engine.setValue(stemGroup, "mute", 0);
    } else {
        engine.setValue(stemGroup, "mute", 1);
    }
};

PioneerDDJFLX4P2.stemMutePadShiftPressed = function(channel, control, value, _status, group) {
    if (value !== 0x7f) { return; }

    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const stemCount = Math.min(engine.getValue(actualGroup, "stem_count"), 4);

    if (control - PioneerDDJFLX4P2.stemMutePadsFirstControl + 1 > stemCount) { return; }

    const deckName = actualGroup.substring(1, actualGroup.length-1);

    for (let stemIdx=1; stemIdx<=stemCount; stemIdx++) {
        const stemGroup = "[" + deckName + "_Stem" + stemIdx + "]";

        if (stemIdx + PioneerDDJFLX4P2.stemMutePadsFirstControl - 1 === control) {
            engine.setValue(stemGroup, "mute", 0);
        } else {
            engine.setValue(stemGroup, "mute", 1);
        }
    }
};

PioneerDDJFLX4P2.stemFxPadPressed = function(channel, control, value, _status, group) {
    if (value !== 0x7f) { return; }

    if (control - PioneerDDJFLX4P2.stemFxPadsFirstControl + 1 > 4) { return; }

    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const deckName = actualGroup.substring(1, actualGroup.length-1);
    const stemGroup = "[QuickEffectRack1_[" + deckName + "_Stem" + (control - PioneerDDJFLX4P2.stemFxPadsFirstControl + 1) + "]]";

    if (engine.getValue(stemGroup, "enabled")) {
        engine.setValue(stemGroup, "enabled", 0);
    } else {
        engine.setValue(stemGroup, "enabled", 1);
    }
};

PioneerDDJFLX4P2.stemFxPadShiftPressed = function(channel, control, value, _status, group) {
    if (value !== 0x7f) { return; }

    if (control - PioneerDDJFLX4P2.stemFxPadsFirstControl + 1 > 4) { return; }

    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const deckName = actualGroup.substring(1, actualGroup.length-1);
    const stemGroup = "[QuickEffectRack1_[" + deckName + "_Stem" + (control - PioneerDDJFLX4P2.stemFxPadsFirstControl + 1) + "]]";

    engine.setValue(stemGroup, "next_chain_preset", 1);
};

PioneerDDJFLX4P2.stemCountChanged = function(_value, group, _control) {
    // Check if this is one of our active decks
    const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);

    if (group !== leftGroup && group !== rightGroup) { return; }

    const deckName = group.substring(1, group.length-1);

    for (let stem=1; stem<=4; stem++) {
        PioneerDDJFLX4P2.stemMuteChanged(
            engine.getValue("[" + deckName + "_Stem" + stem + "]", "mute"),
            "[" + deckName + "_Stem" + stem + "]",
            _control
        );

        PioneerDDJFLX4P2.stemFxChanged(
            engine.getValue("[QuickEffectRack1_[" + deckName + "_Stem" + stem + "]]", "enabled"),
            "[QuickEffectRack1_[" + deckName + "_Stem" + stem + "]]",
            _control
        );
    }
};

PioneerDDJFLX4P2.stemMuteChanged = function(value, group, _control) {
    const channelStem = group.match(/\[Channel(\d+)_Stem(\d+)\]/);
    if (!channelStem) { return; }

    const deck = Number(channelStem[1]);
    const stem = Number(channelStem[2]);
    const channel = "[Channel" + deck + "]";

    if (stem > 4) { return; }

    // Check if this deck is currently active
    const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);

    if (channel !== leftGroup && channel !== rightGroup) { return; }

    const stemCount = engine.getValue(channel, "stem_count");

    let code = 0x00;
    if (stem <= stemCount && value <= 0.5) {
        code = 0x7f;
    }

    const stemsPadsModesStatus = PioneerDDJFLX4P2.getStemsPadsModesStatus();
    for (let i=0; i<stemsPadsModesStatus[channel].length; i++) {
        midi.sendShortMsg(
            stemsPadsModesStatus[channel][i],
            PioneerDDJFLX4P2.stemMutePadsFirstControl + stem - 1,
            code
        );
    }
};

PioneerDDJFLX4P2.stemFxChanged = function(value, group, _control) {
    const channelStem = group.match(/\[QuickEffectRack1_\[Channel(\d+)_Stem(\d+)\]\]/);
    if (!channelStem) { return; }

    const deck = Number(channelStem[1]);
    const stem = Number(channelStem[2]);
    const channel = "[Channel" + deck + "]";

    if (stem > 4) { return; }

    // Check if this deck is currently active
    const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);

    if (channel !== leftGroup && channel !== rightGroup) { return; }

    const stemsPadsModesStatus = PioneerDDJFLX4P2.getStemsPadsModesStatus();
    for (let i=0; i<stemsPadsModesStatus[channel].length; i++) {
        midi.sendShortMsg(
            stemsPadsModesStatus[channel][i],
            PioneerDDJFLX4P2.stemFxPadsFirstControl + stem - 1,
            value <= 0.5 ? 0x00 : 0x7f
        );
    }
};

//
// Pitch Shift mode
//

PioneerDDJFLX4P2.pitchAdjusted = function(_value, group, _control) {
    // Check if this is one of our active decks
    const leftGroup = PioneerDDJFLX4P2.getDeckGroup(0);
    const rightGroup = PioneerDDJFLX4P2.getDeckGroup(1);

    if (group !== leftGroup && group !== rightGroup) { return; }

    const pitchAdjust = Math.round(engine.getValue(group, "pitch_adjust"));
    let lights = 0b00000000;

    if (pitchAdjust === 0) {
        lights = 0b10000001;
    } else if (pitchAdjust === 1) {
        lights = 0b01000000;
    } else if (pitchAdjust === 2) {
        lights = 0b00100000;
    } else if (pitchAdjust === 3) {
        lights = 0b00010000;
    } else if (pitchAdjust === 4) {
        lights = 0b10010000;
    } else if (pitchAdjust === 5) {
        lights = 0b01010000;
    } else if (pitchAdjust === 6) {
        lights = 0b00110000;
    } else if (pitchAdjust === 7) {
        lights = 0b10110000;
    } else if (pitchAdjust === 8) {
        lights = 0b01110000;
    } else if (pitchAdjust > 8) {
        lights = 0b11110000;
    } else if (pitchAdjust === -1) {
        lights = 0b00000010;
    } else if (pitchAdjust === -2) {
        lights = 0b00000100;
    } else if (pitchAdjust === -3) {
        lights = 0b00001000;
    } else if (pitchAdjust === -4) {
        lights = 0b00001001;
    } else if (pitchAdjust === -5) {
        lights = 0b00001010;
    } else if (pitchAdjust === -6) {
        lights = 0b00001100;
    } else if (pitchAdjust === -7) {
        lights = 0b00001101;
    } else if (pitchAdjust === -8) {
        lights = 0b00001110;
    } else if (pitchAdjust < -8) {
        lights = 0b00001111;
    } else {
        lights = 0b11111111;
    }

    const pitchPadsModesStatus = PioneerDDJFLX4P2.getPitchPadsModesStatus();

    for (let i=0; i<8; i++) {
        let code = 0x00;
        const pad = 0b10000000 >>> i;

        if (lights & pad) {
            code = 0x7f;
        } else {
            code = 0x00;
        }

        pitchPadsModesStatus[group].forEach(
            (padMode) => midi.sendShortMsg(
                padMode,
                PioneerDDJFLX4P2.pitchPadsFirstControl + i,
                code
            )
        );
    }
};

PioneerDDJFLX4P2.pitchPadPressed = function(channel, control, value, _status, group) {
    if (value !== 0x7f) { return; }

    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const pad = control - PioneerDDJFLX4P2.pitchPadsFirstControl;
    let pitch = 0;

    if (pad === 0) {
        pitch = 0;
    } else if (pad === 1) {
        pitch = 1;
    } else if (pad === 2) {
        pitch = 2;
    } else if (pad === 3) {
        pitch = 3;
    } else if (pad === 4) {
        pitch = -3;
    } else if (pad === 5) {
        pitch = -2;
    } else if (pad === 6) {
        pitch = -1;
    } else if (pad === 7) {
        pitch = 0;
    }

    engine.setValue(actualGroup, "pitch_adjust", pitch);
};

PioneerDDJFLX4P2.pitchPadShiftPressed = function(channel, control, value, _status, group) {
    if (value !== 0x7f) { return; }

    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const pad = control - PioneerDDJFLX4P2.pitchPadsFirstControl;
    let currentPitch = engine.getValue(actualGroup, "pitch_adjust");

    if (pad === 0) {
        currentPitch += 1;
    } else if (pad === 1) {
        currentPitch += 2;
    } else if (pad === 2) {
        currentPitch += 3;
    } else if (pad === 3) {
        currentPitch += 4;
    } else if (pad === 4) {
        currentPitch += -4;
    } else if (pad === 5) {
        currentPitch += -3;
    } else if (pad === 6) {
        currentPitch += -2;
    } else if (pad === 7) {
        currentPitch += -1;
    }

    engine.setValue(actualGroup, "pitch_adjust", currentPitch);
};

//
// Hot Cue handlers
//

PioneerDDJFLX4P2.hotcueActivate = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const hotcueNum = (control & 0x0F) + 1; // Extract hotcue number from control
    engine.setValue(actualGroup, "hotcue_" + hotcueNum + "_activate", 1);
};

PioneerDDJFLX4P2.hotcueClear = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    const hotcueNum = (control & 0x0F) + 1;
    engine.setValue(actualGroup, "hotcue_" + hotcueNum + "_clear", 1);
};

//
// Loop handlers
//

PioneerDDJFLX4P2.loopIn = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "loop_in", 1);
};

PioneerDDJFLX4P2.loopOut = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "loop_out", 1);
};

PioneerDDJFLX4P2.reloopToggle = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "reloop_toggle", 1);
};

PioneerDDJFLX4P2.reloopExit = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    engine.setValue(actualGroup, "reloop_andstop", 1);
};

PioneerDDJFLX4P2.beatloopActivate = function(channel, control, value, status, group) {
    if (value === 0) { return; }
    const actualGroup = PioneerDDJFLX4P2.getDeckGroup(channel);
    // Map control to beatloop size
    const sizes = {0x14: 0.5, 0x15: 1, 0x16: 2, 0x17: 4, 0x18: 8, 0x19: 16, 0x1A: 32, 0x1B: 64};
    const size = sizes[control] || 4;
    engine.setValue(actualGroup, "beatloop_" + size + "_toggle", 1);
};

//
// Shutdown
//

PioneerDDJFLX4P2.shutdown = function() {
    print("DDJ-FLX4 Player 2: Shutting down");

    // Disconnect VU meter connections
    PioneerDDJFLX4P2.vuMeterConnections.forEach(function(conn) {
        conn.disconnect();
    });

    // reset vumeter
    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.deck1.vuMeter, false);
    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.deck2.vuMeter, false);

    // turn off all Sampler LEDs
    for (var i = 0; i <= 7; ++i) {
        midi.sendShortMsg(0x97, 0x30 + i, 0x00);
        midi.sendShortMsg(0x98, 0x30 + i, 0x00);
        midi.sendShortMsg(0x99, 0x30 + i, 0x00);
        midi.sendShortMsg(0x9A, 0x30 + i, 0x00);
    }
    // turn off all Hotcue LEDs
    for (i = 0; i <= 7; ++i) {
        midi.sendShortMsg(0x97, 0x00 + i, 0x00);
        midi.sendShortMsg(0x98, 0x00 + i, 0x00);
        midi.sendShortMsg(0x99, 0x00 + i, 0x00);
        midi.sendShortMsg(0x9A, 0x00 + i, 0x00);
    }

    // turn off loop in and out lights
    PioneerDDJFLX4P2.setLoopButtonLights(0x90, 0x00);
    PioneerDDJFLX4P2.setLoopButtonLights(0x91, 0x00);

    // turn off reloop lights
    PioneerDDJFLX4P2.setReloopLight(0x90, 0x00);
    PioneerDDJFLX4P2.setReloopLight(0x91, 0x00);

    // stop any flashing lights
    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.beatFx, false);
    PioneerDDJFLX4P2.toggleLight(PioneerDDJFLX4P2.lights.shiftBeatFx, false);

    // stop the keepalive timer
    engine.stopTimer(PioneerDDJFLX4P2.keepAliveTimer);
};

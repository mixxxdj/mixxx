/**
 * Stanton DJC4 controller script v1.0 for Mixxx v2.2.3
 *
 * Written by Martin Bruset Solberg
 * Adopted for v2.2.3 by Christoph Zimmermann
 *
 * Based on MC2000 script by Esteban Serrano Roloff
 * and Denon MC7000 script by OsZ
 *
 *
 * TODO:
 * Effects browsing
 * Beat multiplier
 *
 **/

var djc4 = {};

// ----------   Global variables    ----------

// MIDI Reception commands (from spec)
djc4.leds = {
    loopminus: 2,
    loopplus: 3,
    loopin: 4,
    loopout: 5,
    loopon: 6,
    loopdel: 7,
    hotcue1: 8,
    hotcue2: 9,
    hotcue3: 10,
    hotcue4: 11,
    sample1: 12,
    sample2: 13,
    sample3: 14,
    sample4: 15,
    keylock: 16,
    sync: 18,
    pbendminus: 19,
    pbendplus: 20,
    scratch: 21,
    tap: 22,
    cue: 23,
    play: 24,
    highkill: 25,
    midkill: 26,
    lowkill: 27,
    pfl: 28,
    fxon: 30,
    fxexf1: 31,
    fxexf2: 32,
    fxexf3: 33,
    loadac: 34,
    loadbd: 35,
    videofx: 36,
    xflink: 37,
    keyon: 38,
    filteron: 39,
    tx: 46,
    fx: 47
};

djc4.scratchMode = [false, false, false, false];

// ----------   Functions    ----------

// Called when the MIDI device is opened & set up.
djc4.init = function(id, debug) {
    djc4.id = id;
    djc4.debug = debug;

    // Put all LEDs to default state.
    djc4.allLed2Default();

    // ---- Connect controls -----------

    // ---- Controls for Channel 1 to 4
    var i = 0;
    for (i = 1; i <= 4; i++) {
    // Cue 1-4
        var j = 0;
        for (j = 1; j <= 4; j++) {
            engine.makeConnection("[Channel" + i + "]", "hotcue_" + j + "_enabled",
                djc4.hotcueSetLed);
        }

        // Cue
        engine.makeConnection("[Channel" + i + "]", "cue_indicator",
            djc4.cueSetLed);
        // Play
        engine.makeConnection("[Channel" + i + "]", "play_indicator",
            djc4.playSetLed);

        // Loop in
        engine.makeConnection("[Channel" + i + "]", "loop_start_position",
            djc4.loopStartSetLed);
        // Loop out
        engine.makeConnection("[Channel" + i + "]", "loop_end_position",
            djc4.loopEndSetLed);
        // Loop enabled
        engine.makeConnection("[Channel" + i + "]", "loop_enabled",
            djc4.loopEnabledSetLed);
        // Loop double
        engine.makeConnection("[Channel" + i + "]", "loop_double",
            djc4.loopDoubleSetLed);
        // Loop halve
        engine.makeConnection("[Channel" + i + "]", "loop_halve",
            djc4.loopHalveSetLed);

        // Monitor cue
        engine.makeConnection("[Channel" + i + "]", "pfl", djc4.pflSetLed);

        // Kills
        engine.makeConnection("[Channel" + i + "]", "filterHighKill",
            djc4.highkillSetLed);
        engine.makeConnection("[Channel" + i + "]", "filterMidKill",
            djc4.midkillSetLed);
        engine.makeConnection("[Channel" + i + "]", "filterLowKill",
            djc4.lowkillSetLed);

        engine.makeConnection("[QuickEffectRack1_[Channel" + i + "]_Effect1]",
            "enabled", djc4.filterSetLed);

        // Keylock
        engine.makeConnection("[Channel" + i + "]", "keylock", djc4.keylockSetLed);

        // Pitch bend
        engine.makeConnection("[Channel" + i + "]", "rate_temp_down",
            djc4.ratetempdownSetLed);
        engine.makeConnection("[Channel" + i + "]", "rate_temp_up",
            djc4.ratetempupSetLed);
    }

    // ---- Controls for Sampler 1 - 8
    for (i = 1; i <= 8; i++) {
        engine.makeConnection("[Sampler" + i + "]", "track_loaded",
            djc4.samplerSetLed);
        if (engine.getValue("[Sampler" + i + "]", "track_loaded") === 1) {
            djc4.samplerSetLed(1, "[Sampler" + i + "]");
        }
    }

    // ---- Controls for EffectUnit 1 to 2
    for (i = 1; i <= 2; i++) {
    // Effects 1-3
        for (j = 1; j <= 3; j++) {
            engine.makeConnection("[EffectRack1_EffectUnit" + i + "_Effect" + j + "]",
                "enabled", djc4.fxenabledSetLed);
        }
    }
    // Effect enabled for Channel
    engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable",
        djc4.fxon1SetLed);
    engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel3]_enable",
        djc4.fxon3SetLed);
    engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable",
        djc4.fxon2SetLed);
    engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel4]_enable",
        djc4.fxon4SetLed);

    // ---- VU meter (Master is shown)
    engine.makeConnection("[Master]", "VuMeterL", djc4.VuMeterLSetLed);
    engine.makeConnection("[Master]", "VuMeterR", djc4.VuMeterRSetLed);

    // Enable load LEDs because Channels are empty at start
    djc4.setLed(1, djc4.leds["loadac"], 1);
    djc4.setLed(3, djc4.leds["loadac"], 1);
    djc4.setLed(2, djc4.leds["loadbd"], 1);
    djc4.setLed(4, djc4.leds["loadbd"], 1);
};

// Called when the MIDI device is closed
djc4.shutdown = function() {
    // Put all LEDs to default state.
    djc4.allLed2Default();
};

// === FOR MANAGING LEDS ===

djc4.allLed2Default = function() {
    // All LEDs OFF for deck 1 to 4
    var i = 0;
    for (i = 1; i <= 4; i++) {
        for (var led in djc4.leds) {
            djc4.setLed(i, djc4.leds[led], 0);
        }
        // Channel VU meter
        midi.sendShortMsg(0xB0 + (i - 1), 2, 0);
    }
    // Master VU meter
    midi.sendShortMsg(0xB0, 3, 0);
    midi.sendShortMsg(0xB0, 4, 0);
};

// Set leds function
djc4.setLed = function(deck, led, status) {
    var ledStatus = 0x00; // Default OFF
    switch (status) {
    case 0:
        ledStatus = 0x00;
        break; // OFF
    case false:
        ledStatus = 0x00;
        break; // OFF
    case 1:
        ledStatus = 0x7F;
        break; // ON
    case true:
        ledStatus = 0x7F;
        break; // ON
    default:
        break;
    }
    midi.sendShortMsg(0x90 + (deck - 1), led, ledStatus);
};

// === MISC COMMON ===

djc4.group2Deck = function(group) {
    var matches = group.match(/\[Channel(\d+)\]/);
    if (matches === null) {
        return -1;
    } else {
        return matches[1];
    }
};

djc4.group2Sampler = function(group) {
    var matches = group.match(/^\[Sampler(\d+)\]$/);
    if (matches === null) {
        return -1;
    } else {
        return matches[1];
    }
};

// === Scratch control ===

djc4.toggleScratchMode = function(channel, control, value, status, group) {
    if (!value)
        return;

    var deck = djc4.group2Deck(group);
    // Toggle setting
    djc4.scratchMode[deck - 1] = !djc4.scratchMode[deck - 1];
    djc4.scratchSetLed(djc4.scratchMode[deck - 1], group);
};

// === JOG WHEEL ===

// Touch platter
djc4.wheelTouch = function(channel, control, value) {
    var deck = channel + 1;

    if (control === 0x58) { // If shift is pressed, do a fast search
        if (value === 0x7F) { // If touch
            var alpha = 1.0 / 8;
            var beta = alpha / 32;

            var rpm = 40.0;

            engine.scratchEnable(deck, 128, rpm, alpha, beta, true);
        } else { // If button up
            engine.scratchDisable(deck);
        }
    } else if (djc4.scratchMode[channel] === true) { // If scratch enabled
        if (value === 0x7F) {                          // If touch
            alpha = 1.0 / 8;
            beta = alpha / 32;

            rpm = 150.0;

            engine.scratchEnable(deck, 128, rpm, alpha, beta, true);
        } else { // If button up
            engine.scratchDisable(deck);
        }
    } else if (value === 0x00) {
    // In case shift is let go before the platter,
    // ensure scratch is disabled
        engine.scratchDisable(deck);
    }
};

// Wheel
djc4.wheelTurn = function(channel, control, value, status, group) {
    // var deck = channel + 1;
    var deck = script.deckFromGroup(group);

    // B: For a control that centers on 0x40 (64):
    var newValue = (value - 64);

    // See if we're scratching. If not, skip this.
    if (!engine.isScratching(deck)) {
        engine.setValue(group, "jog", newValue / 4);
        return;
    }

    // In either case, register the movement
    engine.scratchTick(deck, newValue);
};

// === Browser ===

djc4.browseMove = function(channel, control, value, status, group) {
    // Next/previous track
    if (value === 0x41) {
        engine.setValue(group, "MoveUp", true);
    } else if (value === 0x3F) {
        engine.setValue(group, "MoveDown", true);
    } else
        return;
};

djc4.browseScroll = function(channel, control, value, status, group) {
    // Next/previous page
    if (value === 0x41) {
        engine.setValue(group, "ScrollUp", true);
    } else if (value === 0x3F) {
        engine.setValue(group, "ScrollDown", true);
    } else
        return;
};

// === Sampler Volume Control ===

djc4.samplerVolume = function(channel, control, value) {
    // check if the Sampler Volume is at Zero and if so hide the sampler bank
    if (value > 0x00) {
        engine.setValue("[Samplers]", "show_samplers", true);
    } else {
        engine.setValue("[Samplers]", "show_samplers", false);
    }
    // get the Sampler Row opened with its details
    engine.setValue("[SamplerRow1]", "expanded", true);

    // control up to 8 sampler volumes with the one knob on the mixer
    for (var i = 1; i <= 8; i++) {
        engine.setValue("[Sampler" + i + "]", "pregain",
            script.absoluteNonLin(value, 0, 1.0, 4.0));
    }
};

// === SET LED FUNCTIONS ===

// Hot cues

djc4.hotcueSetLed = function(value, group, control) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["hotcue" + control[7]], value);
};

// PFL

djc4.pflSetLed = function(
    value,
    group) { djc4.setLed(djc4.group2Deck(group), djc4.leds["pfl"], value); };

// Play/Cue

djc4.playSetLed = function(value, group) {
    // var deck = channel + 1;
    var deck = djc4.group2Deck(group);

    djc4.setLed(djc4.group2Deck(group), djc4.leds["play"], value);

    // if a deck is playing it is not possible to load a track
    // -> disable corresponding load LED
    if (deck === 1 || deck === 3) {
        djc4.setLed(djc4.group2Deck(group), djc4.leds["loadac"], !value);
    } else {
        djc4.setLed(djc4.group2Deck(group), djc4.leds["loadbd"], !value);
    }
};

djc4.cueSetLed = function(
    value,
    group) { djc4.setLed(djc4.group2Deck(group), djc4.leds["cue"], value); };

// Keylock

djc4.keylockSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["keylock"], value);
};

// Loops

djc4.loopStartSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["loopin"], value !== -1);
};

djc4.loopEndSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["loopout"], value !== -1);
};

djc4.loopEnabledSetLed = function(
    value,
    group) { djc4.setLed(djc4.group2Deck(group), djc4.leds["loopon"], value); };

djc4.loopDoubleSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["loopplus"], value);
};

djc4.loopHalveSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["loopminus"], value);
};

// Kills

djc4.highkillSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["highkill"], value);
};

djc4.midkillSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["midkill"], value);
};

djc4.lowkillSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["lowkill"], value);
};

djc4.filterSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["filteron"], !value);
};

// Scratch button

djc4.scratchSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["scratch"], value);
};

// Pitch bend buttons
djc4.ratetempdownSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["pbendminus"], value);
};

djc4.ratetempupSetLed = function(value, group) {
    djc4.setLed(djc4.group2Deck(group), djc4.leds["pbendplus"], value);
};

djc4.fxenabledSetLed = function(value, group) {
    var matches = group.match(/^\[EffectRack1_EffectUnit(\d+)_Effect(\d+)\]$/);
    if (matches !== null) {
        var led = djc4.leds["fxexf1"] - 1 + parseInt(matches[2], 10);

        // FX1 is on deck A/C
        if (parseInt(matches[1], 10) === 1) {
            djc4.setLed(1, led, value);
            djc4.setLed(3, led, value);
        } else {
            djc4.setLed(2, led, value);
            djc4.setLed(4, led, value);
        }
    }
};

djc4.fxon1SetLed = function(
    value) { djc4.setLed(1, djc4.leds["fxon"], value); };

djc4.fxon2SetLed = function(
    value) { djc4.setLed(2, djc4.leds["fxon"], value); };

djc4.fxon3SetLed = function(
    value) { djc4.setLed(3, djc4.leds["fxon"], value); };

djc4.fxon4SetLed = function(
    value) { djc4.setLed(4, djc4.leds["fxon"], value); };

// Sampler

djc4.samplerSetLed = function(value, group) {
    var sampler = djc4.group2Sampler(group);

    if (sampler <= 4) {
    // Sampler 1 - 4 are on deck A/C
        var led = djc4.leds["sample1"] - 1 + parseInt(sampler, 10);
        djc4.setLed(1, led, value);
        djc4.setLed(3, led, value);
    } else {
    // Sampler 5 - 8 are on deck B/D
        led = djc4.leds["sample1"] - 1 - 4 + parseInt(sampler, 10);
        djc4.setLed(2, led, value);
        djc4.setLed(4, led, value);
    }
};

// === VU Meter ===

djc4.VuMeterLSetLed = function(value) {
    var ledStatus = (value * 119);
    midi.sendShortMsg(0xB0, 3, ledStatus);
};

djc4.VuMeterRSetLed = function(value) {
    var ledStatus = (value * 119);
    midi.sendShortMsg(0xB0, 4, ledStatus);
};

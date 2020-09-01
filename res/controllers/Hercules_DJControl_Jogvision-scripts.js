// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Jogvision.
// * Author: DJ Phatso, contributions by Kerrick Staley and David TV
// * Version 1.16 (Jun 2020)
// * Version 1.15 (Jun 2020)
// * Version 1.14 (Jun 2020)
// * Version 1.13 (May 2020)
// * Version 1.11 (May 2020)
// * Version 1.10 (May 2020)
// * Version 1.9 (May 2020)
// * Version 1.8 (May 2020)
// * Version 1.6 (January 2020)
// * Version 1.5 (January 2020)
// * Version 1.4 (December 2019)
// * Version 1.3 (November 2019)
// * Version 1.2 (November 2019)
// * Version 1.1 (March 2019)
// * Forum: https://www.mixxx.org/forums/viewtopic.php?f=7&t=12580
// * Wiki: https://www.mixxx.org/wiki/doku.php/hercules_dj_control_jogvision
//
// Changes to v1.16
// - Added a "beatDetection" variable to use old school one (way better beat match, but no song position relative)
//
// Changes to v1.15
// - Reset shift-"Loop ON", to do what Hercules wanted it to be: select FX2 for given channel. Now,
//   the loop creation is done with "MODE"+"Loop ON" combo (here, MODE key works like "SHIFT" key ;)
//
// Changes to v1.14
// - Better beat_active matching
//
// Changes to v1.13
// - Added "shift"+AIRFX to do a high pass (apart from the default low pass)
// - Added shift+<multi FX> to set beatgrid at current play position
//
// Changes to v1.12
// - Added "alternate" beat leds mode
// - Changed "follow" beats led algorithm with a better/more elegant version
//
// Changes to v1.11
// - Changed beats led algorithm to match song beats (forward and backward) and fix leads positions,
//   what allows backward led activation!! (didn't have that working previously)
//
// Changes to v1.10
// - Changed jogwheel outer led movement based on playposition (much better!)
// - Added a user variable to set or not some effects at ini
// - Minor code convention corrections
//
// Changes to v1.9
// - Added jogwheel outer led movement when scratch and back/forward spin
//
// Changes to v1.8
// - Added normal, reverse, blink and follow modes for beat active led (see variable: beatActiveMode)
//
// Changes to v1.7
// - Added back-spin (and forward-spin) effect on "vinyl" mode when jog wheel pushed from top
//
// Changes to v1.6
// - Minor code-style changes (use camelCase, use "double quotes" and other if/else styles)
//
// Changes to v1.5
// - Fixed regression not updating the VuMeters
// - Fixed regression not updating the CUE/MIX buttons
//
// Changes to v1.4
// - Code cleanup and standarization
// - Added "orage" track led color (ala Serato) when play position is beyond the
//   part of the track, but before the "end of track" Mixxx signal
//
// Changes to v1.3
// - Enabled the creation of beatloop (beatloopActivate) by using SHIFT+LoopON
// - Changed "LOOP SIZE" to adjust the beatjumpSize (you still can change size
//   with surrounding buttons: reloop_toggle key)
// - Added SHIFT+"LOOP SIZE" to move loop left or right by N beats (beatjump)
//
// Changes to v1.2
// - Enabled Jogwheel Outer LED rotation
// - Enabled Beat LEDs
//
// Changes to v1.1
// - Controller knob/slider values are queried on startup, so MIXXX is synced.
// - Fixed vinyl button behavior the first time it"s pressed.
//
// v1.0 : Original release
//
// ****************************************************************************

// ****************************************************************************
// User available variables (you may modify them)
var beatActiveMode = "normal"; // normal (default), reverse, blink, follow, alternate
var beatDetection = "normal"; // normal (default), follow (song position accurate, beat much less accurate)
var initUpdateEffects = 1; // 1 (default) - set some effects values at startup; 0 - do not touch effects at startup

// ****************************************************************************
// Other internal constants (you may NOT modify anything from here)
var on = 0x7F;
var off = 0x00;
var alpha = 1.0 / 8;
var beta = alpha / 16;
var speed = 33 + 1/3;
var ledSpeed = (speed / 60) * 127;
var masterLeds = 0x90;
var ledRotationTimer = 0;
var beatMax;
if (beatActiveMode.match(/^(?:normal|reverse)$/g)) {
    beatMax = 4;
} else if (beatActiveMode.match(/^(?:blink|alternate)$/g)) {
    beatMax = 2;
} else {
    beatMax = 8;
}

var DJCJV = {};
DJCJV.vinylModeActive = true;
DJCJV.Channel = [];
DJCJV.Channel["[Channel1]"] = {"central": 0x90, "deck": 0xB0, "beatPosition": 1, "rotation": 0x00, "n": 1, "onBeat": 0, "beatsPassed": 0, "shiftPressed": 0, "modePressed": 0};
DJCJV.Channel["[Channel2]"] = {"central": 0x91, "deck": 0xB1, "beatPosition": 1, "rotation": 0x00, "n": 2, "onBeat": 0, "beatsPassed": 0, "shiftPressed": 0, "modePressed": 0};

// ****************************************************************************
// General functions

// Function to rotate jogs' outer led (borrowed from the 'Pioneer-DDJ-SX-scripts.js' mapping)
DJCJV.updateJogLeds = function(value, group, control) {
    var elapsedTime = value * engine.getValue(group, "duration");
    var wheelPos = parseInt(((value >= 0) ? 0 : 127) + 1 + ((ledSpeed * elapsedTime) % 127));

    // Only send midi message when the position is actually updated.
    if (DJCJV.Channel[group].rotation !== wheelPos) {
        midi.sendShortMsg(DJCJV.Channel[group].deck, 0x60, wheelPos); // Update the outer (spin) jog leds
        DJCJV.wheelInnerUpdate(value, group, control); // Also update the inner jog leds with updated song position
    }
    DJCJV.Channel[group].rotation = wheelPos;

    DJCJV.Channel[group].beatsPassed = Math.round((value * engine.getValue(group, "duration")) * (engine.getValue(group, "bpm") / 60));
    DJCJV.Channel[group].beatPosition = Math.floor((DJCJV.Channel[group].beatsPassed % beatMax)) + 1;

    if (beatDetection === "follow") {
        // If on beat_active, update the beat leds
        if (engine.getValue(group, "beat_active") || ((engine.getValue(group, "beat_closest") < engine.getValue(group, "beat_next"))) && (!DJCJV.Channel[group].onBeat)) {
            DJCJV.beatActive(0, group);
            DJCJV.Channel[group].onBeat = true;
        } else if (engine.getValue(group, "beat_closest") >= engine.getValue(group, "beat_next")) {
            DJCJV.Channel[group].onBeat = false;
        }
    }
};

// Initialization
DJCJV.init = function(id) {

    print("Hercules DJControl Jogvision id: \""+id+"\" initializing...");
    print("Using beatActiveMode="+beatActiveMode+" (beatMax="+beatMax+")");

    // Set all LED states to off
    midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x7F, off);
    midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x7F, off);

    // Set Vinyl button LED On.
    midi.sendShortMsg(masterLeds, 0x45, on);
    DJCJV.vinylModeActive = true;
    midi.sendShortMsg(masterLeds, 0x46, on);

    // Set Headphone CUE/MIX LED state
    if (engine.getValue("[Master]", "headMix") > 0.5) {
        midi.sendShortMsg(masterLeds, 0x4C, on); // headset "Mix" button LED
        midi.sendShortMsg(masterLeds, 0x4D, off);
    } else {
        midi.sendShortMsg(masterLeds, 0x4C, off);
        midi.sendShortMsg(masterLeds, 0x4D, on); // headset "Cue" button LED
    }

    // Enable Soft takeover
    engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[QuickEffectRack1_[Channel1]]", "super1", true);
    engine.softTakeover("[QuickEffectRack1_[Channel2]]", "super1", true);

    if (initUpdateEffects === 1) {
        // Set effects Levels - Dry/Wet - Filters
        // Done to work around the limited amount of controls in the Jogvision controller
        engine.setParameter("[EffectRack1_EffectUnit1_Effect1]", "meta", 0.6);
        engine.setParameter("[EffectRack1_EffectUnit1_Effect2]", "meta", 0.6);
        engine.setParameter("[EffectRack1_EffectUnit1_Effect3]", "meta", 0.6);
        engine.setParameter("[EffectRack1_EffectUnit2_Effect1]", "meta", 0.6);
        engine.setParameter("[EffectRack1_EffectUnit2_Effect2]", "meta", 0.6);
        engine.setParameter("[EffectRack1_EffectUnit2_Effect3]", "meta", 0.6);
        engine.setParameter("[EffectRack1_EffectUnit1]", "mix", 0.6);
        engine.setParameter("[EffectRack1_EffectUnit2]", "mix", 0.6);
        engine.setParameter("[QuickEffectRack1_[Channel1]]", "super1", 0.5);
        engine.setParameter("[QuickEffectRack1_[Channel2]]", "super1", 0.5);
    }

    // Connect the VUMeters
    engine.connectControl("[Channel1]", "VuMeter", "DJCJV.vuMeterUpdate");
    engine.connectControl("[Channel2]", "VuMeter", "DJCJV.vuMeterUpdate");

    // Set inner jog leds to 0
    midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x61, 0);
    midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x61, 0);
    // Set outer jog leds to 0
    midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x60, 1);
    midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x60, 1);

    // Enable jogs' outer leds rotation and Inner LEDs song position update
    engine.connectControl("[Channel1]", "playposition", "DJCJV.updateJogLeds");
    engine.connectControl("[Channel2]", "playposition", "DJCJV.updateJogLeds");
    if (beatDetection === "normal") {
        // Connect the beat_active with beat leds
        engine.connectControl("[Channel1]", "beat_active", "DJCJV.beatActive");
        engine.connectControl("[Channel2]", "beat_active", "DJCJV.beatActive");
        engine.connectControl("[Channel1]", "stop", "DJCJV.beatInactive");
        engine.connectControl("[Channel2]", "stop", "DJCJV.beatInactive");
    }

    // Ask the controller to send all current knob/slider values over MIDI, which will update the corresponding GUI controls in MIXXX.
    midi.sendShortMsg(0xB0, 0x7F, on);

    print("Hercules DJControl Jogvision id: \""+id+"\" initialized");
};

// Finalization
DJCJV.shutdown = function() {
    if (ledRotationTimer) {
        engine.stopTimer(ledRotationTimer);
        ledRotationTimer = 0;
    }
    // Set all LED states to off
    midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x7F, off);
    midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x7F, off);
};

// Beat led ACTIVATE (move)
DJCJV.beatActive = function(value, group, _control) {
    if (value === 1) {
        return;
    }
    var led = DJCJV.Channel[group].central;
    var pos = DJCJV.Channel[group].beatPosition;

    // Normal
    if (beatActiveMode === "normal") {
        midi.sendShortMsg(led, 0x3A, pos === 1 ? on : off);
        midi.sendShortMsg(led, 0x3B, pos === 2 ? on : off);
        midi.sendShortMsg(led, 0x3C, pos === 3 ? on : off);
        midi.sendShortMsg(led, 0x3D, pos === 4 ? on : off);
    // Reverse
    } else if (beatActiveMode === "reverse") {
        midi.sendShortMsg(led, 0x3A, pos !== 1 ? on : off);
        midi.sendShortMsg(led, 0x3B, pos !== 2 ? on : off);
        midi.sendShortMsg(led, 0x3C, pos !== 3 ? on : off);
        midi.sendShortMsg(led, 0x3D, pos !== 4 ? on : off);
    // Reverse
    } else if (beatActiveMode === "blink") {
        midi.sendShortMsg(led, 0x3A, pos === 1 ? on : off);
        midi.sendShortMsg(led, 0x3B, pos === 1 ? on : off);
        midi.sendShortMsg(led, 0x3C, pos === 1 ? on : off);
        midi.sendShortMsg(led, 0x3D, pos === 1 ? on : off);
    // Follow
    } else if (beatActiveMode === "follow") {
        midi.sendShortMsg(led, 0x3A, ((pos >= 1) && (pos <= 4)) ? on : off);
        midi.sendShortMsg(led, 0x3B, ((pos >= 2) && (pos <= 5)) ? on : off);
        midi.sendShortMsg(led, 0x3C, ((pos >= 3) && (pos <= 6)) ? on : off);
        midi.sendShortMsg(led, 0x3D, ((pos >= 4) && (pos <= 7)) ? on : off);
    // Alternate
    } else if (beatActiveMode === "alternate") {
        midi.sendShortMsg(led, 0x3A, pos === 1 ? on : off);
        midi.sendShortMsg(led, 0x3B, pos === 1 ? on : off);
        midi.sendShortMsg(led, 0x3C, pos === 2 ? on : off);
        midi.sendShortMsg(led, 0x3D, pos === 2 ? on : off);
    }
};
// Beat led DEACTIVATE (off all)
DJCJV.beatInactive = function(value, group, _control) {
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x3A, off);
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x3B, off);
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x3C, off);
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x3D, off);

    DJCJV.Channel[group].beatPosition = 1;
};

// Jogwheels inner LED display - Play position
DJCJV.wheelInnerUpdate = function(value, group, _control) {
    var playPos = value * 127;
    midi.sendShortMsg(DJCJV.Channel[group].deck, 0x61, playPos);

    // Also update the "track" led information
    if (engine.getValue(group, "end_of_track")) {
        // Let Mixxx"s engine turn flashing red automatically
        return;
    } else if (playPos > 64) {
        // Turn "track" led to orange if position is beyond the half
        midi.sendShortMsg(DJCJV.Channel[group].central, 0x45, 0x02);
    } else {
        // Turn "track" led to green if position is in the first half
        midi.sendShortMsg(DJCJV.Channel[group].central, 0x45, 0x07);
    }
};

// Vu Meter
DJCJV.vuMeterUpdate = function(value, group, _control) {
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x44, value * 6);
};

// Headphone CUE/MIX buttons status
DJCJV.headCue = function(midino, control, value, status, group) {
    if (engine.getValue(group, "headMix") === 0) {
        engine.setValue(group, "headMix", -1.0);
        midi.sendShortMsg(masterLeds, 0x4D, 0x7f);
        midi.sendShortMsg(masterLeds, 0x4C, 0x00);
    }
};
DJCJV.headMix = function(midino, control, value, status, group) {
    if (engine.getValue(group, "headMix") !== 1) {
        engine.setValue(group, "headMix", 0);
        midi.sendShortMsg(masterLeds, 0x4D, 0x00);
        midi.sendShortMsg(masterLeds, 0x4C, 0x7f);
    }
};

// Filter (AIR FX)
DJCJV.Filter = function(channel, control, value, status, group) {
    var deck = group.substr(18, 10);
    var delta = DJCJV.Channel[deck].shiftPressed ? (value / 255) : (-1 * (value / 255));
    engine.setValue(group, "super1", 0.5 + delta);
};

// Sniff decks' SHIFT presses and store them
DJCJV.shiftKey = function(channel, control, value, status, group) {
    DJCJV.Channel[group].shiftPressed = (value === 127);
    return value;
};

// Sniff decks' MODE presses and store them
DJCJV.modeKey = function(channel, control, value, status, group) {
    DJCJV.Channel[group].modePressed = (value === 127);
    return value;
};

// Loop section
// SHIFT + Loop ON creates a loop at given point
DJCJV.beatloopActivate = function(channel, control, value, status, group) {
    if (!DJCJV.Channel[group].modePressed) {
        return value;
    }
    engine.setValue(group, "beatloop_activate", (value !== 0));
};

// SHIFT+FX(Loop) Knob MOVES the existing loop one beat forward/backward
DJCJV.beatjumpMove = function(channel, control, value, status, group) {
    if (value > 64) {
        engine.setValue(group, "beatjump_backward", 1);
        engine.setValue(group, "beatjump_backward", 0);
    } else {
        engine.setValue(group, "beatjump_forward", 1);
        engine.setValue(group, "beatjump_forward", 0);
    }
};
// FX(Loop) Knob changes the AMOUNT of beats to move the loop when requested
DJCJV.beatjumpSize = function(channel, control, value, status, group) {
    var currentValue = engine.getValue(group, "beatjump_size");
    if (value > 64) {
        engine.setValue(group, "beatjump_size", currentValue /= 2);
    } else {
        engine.setValue(group, "beatjump_size", currentValue *= 2);
    }
};

// The Vinyl button, used to enable or disable scratching on the jog wheels.
DJCJV.vinylButton = function(channel, control, value, _status, _group) {
    if (!value) {
        return;
    }

    if (DJCJV.vinylModeActive) {
        DJCJV.vinylModeActive = false;
        midi.sendShortMsg(masterLeds, 0x46, off);
    } else {
        DJCJV.vinylModeActive = true;
        midi.sendShortMsg(masterLeds, 0x46, on);
    }
};

// The pressure action over the jog wheel
DJCJV.wheelTouch = function(channel, control, value, status, group) {
    if (value > 0 && (engine.getValue(group, "play") !== 1 || DJCJV.vinylModeActive)) {
        engine.scratchEnable(DJCJV.Channel[group].n, 400, speed, alpha, beta); //  Touching the wheel
    } else {
        engine.scratchDisable(DJCJV.Channel[group].n); // Released the wheel
    }
};

// Using the top of wheel for scratching (Vinyl button On) and bending (Vinyl button Off)
DJCJV.scratchWheel = function(channel, control, value, status, group) {
    if (engine.isScratching(DJCJV.Channel[group].n)) {
        // Scratch!
        if (value >= 64) {
            // Backward
            engine.scratchTick(DJCJV.Channel[group].n, value - 128);
        } else {
            // Forward
            engine.scratchTick(DJCJV.Channel[group].n, value);
        }
    } else {
        // Pitch bend
        DJCJV.bendWheel(channel, control, value, status, group);
    }
};

// Bending by either using the side of wheel, or with the Job surface when not in vinyl-mode
DJCJV.bendWheel = function(channel, control, value, status, group) {
    // if scratching engaged, do back/forward spin (keep on scratching while jog wheel moves...)
    if (engine.isScratching(DJCJV.Channel[group].n)) {
        if (value >= 64) {
            // Backward spin
            engine.scratchTick(DJCJV.Channel[group].n, -1.5);
        } else {
            // Forward spin
            engine.scratchTick(DJCJV.Channel[group].n, 1.5);
        }
    } else {
        engine.setValue(group, "jog", (value >= 64) ? value - 128 : value); // Pitch bend
    }
};

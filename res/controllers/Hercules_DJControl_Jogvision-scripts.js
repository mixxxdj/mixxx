// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Jogvision.
// * Author: DJ Phatso, contributions by Kerrick Staley and David TV
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

var on = 0x7F;
var off = 0x00;
var alpha = 1.0 / 8;
var beta = alpha / 16;
var ledRotationSpeed = 60; // The bigger, the slower
var ledRotationTimer = 0;
var masterLeds = 0x90;
var beatActiveMode = "normal"; // normal, reverse, blink, follow
var beatMax;
if (beatActiveMode.match(/^(?:normal|reverse)$/g)) {
    beatMax = 4;
} else if (beatActiveMode === "blink") {
    beatMax = 2;
} else {
    beatMax = 8;
}

var DJCJV = {};

DJCJV.vinylModeActive = true;
DJCJV.Channel = [];
DJCJV.Channel["[Channel1]"] = {"central": 0x90, "deck": 0xB0, "beatPosition": 1, "rotation": 0x00, "n": 1};
DJCJV.Channel["[Channel2]"] = {"central": 0x91, "deck": 0xB1, "beatPosition": 1, "rotation": 0x00, "n": 2};

// Initialization
DJCJV.init = function(id) {

    print("Hercules DJControl Jogvision id: \""+id+"\" initializing...");
    print("Using beatActiveMode="+beatActiveMode+" (beatMax="+beatMax+")");

    //Set all LED states to off
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

    //Enable Soft takeover
    engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[QuickEffectRack1_[Channel1]]", "super1", true);
    engine.softTakeover("[QuickEffectRack1_[Channel2]]", "super1", true);

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

    // Connect the VUMeters and Jog Inner LED
    engine.connectControl("[Channel1]", "VuMeter", "DJCJV.vuMeterUpdate");
    engine.connectControl("[Channel2]", "VuMeter", "DJCJV.vuMeterUpdate");
    engine.connectControl("[Channel1]", "playposition", "DJCJV.wheelInnerUpdate");
    engine.connectControl("[Channel2]", "playposition", "DJCJV.wheelInnerUpdate");

    // Connect the beat_active with beat leds
    engine.connectControl("[Channel1]", "beat_active", "DJCJV.beatActive");
    engine.connectControl("[Channel2]", "beat_active", "DJCJV.beatActive");
    engine.connectControl("[Channel1]", "stop", "DJCJV.beatInactive");
    engine.connectControl("[Channel2]", "stop", "DJCJV.beatInactive");

    // Set inner jog leds to 0
    midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x61, 0);
    midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x61, 0);
    // Set outer jog leds to 0
    midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x60, 1);
    midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x60, 1);

    // Enable jogs' outer leds rotation by timer (when channel is playing)
    ledRotationTimer = engine.beginTimer(20, function() {
        if (engine.getValue("[Channel1]", "play") === 1) {
            midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x60, DJCJV.Channel["[Channel1]"].rotation);
            DJCJV.Channel["[Channel1]"].rotation = DJCJV.Channel["[Channel1]"].rotation >= 127 ? 1 : DJCJV.Channel["[Channel1]"].rotation + (engine.getValue("[Channel1]", "bpm") / ledRotationSpeed);
        }
        if (engine.getValue("[Channel2]", "play") === 1) {
            midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x60, DJCJV.Channel["[Channel2]"].rotation);
            DJCJV.Channel["[Channel2]"].rotation = DJCJV.Channel["[Channel2]"].rotation >= 127 ? 1 : DJCJV.Channel["[Channel2]"].rotation + (engine.getValue("[Channel2]", "bpm") / ledRotationSpeed);
        }
    });

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
    //Set all LED states to off
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
        if (pos >= 1) {
            midi.sendShortMsg(led, 0x3A, on);
        }
        if (pos >= 2) {
            midi.sendShortMsg(led, 0x3B, on);
        }
        if (pos >= 3) {
            midi.sendShortMsg(led, 0x3C, on);
        }
        if (pos >= 4) {
            midi.sendShortMsg(led, 0x3D, on);
        }
        if (pos >= 5) {
            midi.sendShortMsg(led, 0x3A, off);
        }
        if (pos >= 6) {
            midi.sendShortMsg(led, 0x3B, off);
        }
        if (pos >= 7) {
            midi.sendShortMsg(led, 0x3C, off);
        }
        if (pos >= 8) {
            midi.sendShortMsg(led, 0x3D, off);
        }
    }

    DJCJV.Channel[group].beatPosition = DJCJV.Channel[group].beatPosition >= beatMax ? 1 : DJCJV.Channel[group].beatPosition  + 1;
};
// Beat led DEACTIVATE (off all)
DJCJV.beatInactive = function(value, group, _control) {
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x3A, off);
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x3B, off);
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x3C, off);
    midi.sendShortMsg(DJCJV.Channel[group].central, 0x3D, off);

    DJCJV.Channel[group].beatPosition = 1;
};

//Jogwheels inner LED display - Play position
DJCJV.wheelInnerUpdate = function(value, group, _control) {
    var playPos = value * 127;
    midi.sendShortMsg(DJCJV.Channel[group].deck, 0x61, playPos);
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

//Vu Meter
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

// Filter (Hercules" AIR FX)
DJCJV.Filter = function(channel, control, value, status, group) {
    engine.setValue(group, "super1", 0.5 - value / 255);
};

// Loop section
// SHIFT + Loop ON creates a loop at given point
DJCJV.beatloopActivate = function(channel, control, value, status, group) {
    engine.setValue(group, "beatloop_activate", value === 0 ? 0 : 1);
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
    var currentValue = engine.getValue(group, "beatjumpSize");
    if (value > 64) {
        engine.setValue(group, "beatjumpSize", currentValue /= 2);
    } else {
        engine.setValue(group, "beatjumpSize", currentValue *= 2);
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
        engine.scratchEnable(DJCJV.Channel[group].n, 400, 33 + 1/3, alpha, beta); //  Touching the wheel
    } else {
        engine.scratchDisable(DJCJV.Channel[group].n); // Released the wheel
    }
};

// Using the top of wheel for scratching (Vinyl button On) and bending (Vinyl button Off)
DJCJV.scratchWheel = function(channel, control, value, status, group) {
    if (engine.isScratching(DJCJV.Channel[group].n)) {
        engine.scratchTick(DJCJV.Channel[group].n, (value >= 64) ? value - 128 : value); // Scratch!
    } else {
        DJCJV.bendWheel(channel, control, value, status, group); // Pitch bend
    }
};

// Bending by either using the side of wheel, or with the Job surface when not in vinyl-mode
DJCJV.bendWheel = function(channel, control, value, status, group) {
    //if scratching engaged, do back/forward spin (keep on scratching while jog wheel moves...)
    if (engine.isScratching(DJCJV.Channel[group].n)) {
        engine.scratchTick(DJCJV.Channel[group].n, (value >= 64) ? -1.5 : 1.5); // back/forward spin
    } else {
        engine.setValue(group, "jog", (value >= 64) ? value - 128 : value); // Pitch bend
    }
};

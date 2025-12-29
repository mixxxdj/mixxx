/*
 * JS for the Pioneer DDJ GRV6 DJ Controller
 * very experimental so far.
 * 
 */


var PioneerDDJGRV6 = {};


PioneerDDJGRV6.lights = {
    beatFx: {
        status: 0x94,
        data1: 0x47,
    },
    shiftBeatFx: {
        status: 0x94,
        data1: 0x43,
    },
    decks: []
};
// The F5 rotation knob returns one of these mapping. 
// This is an observation by experimenting with the knob.
// see fxSelectAbsolute()
PioneerDDJGRV6.fxTable = [
    0.038, // Pos 0
    0.115, // Pos 1
    0.192, // Pos 2
    0.269, // Pos 3
    0.346, // Pos 4
    0.423, // Pos 5
    0.500, // Pos 6
    0.577, // Pos 7
    0.654, // Pos 8
    0.731, // Pos 9
    0.808, // Pos 10
    0.885, // Pos 11
    0.962  // Pos 12
];

// Mapping: Hardware-Position -> Interne Mixxx-ID
// see fxSelectAbsolute()
PioneerDDJGRV6.effectIdTable = [
    "org.mixxx.effects.autopan",             // Pos 0:  AUTOPAN
    "org.mixxx.effects.tremolo",             // Pos 1:  TREMOLO
    "org.mixxx.effects.reverb",              // Pos 2:  REVERB
    "org.mixxx.effects.pitchshift",          // Pos 3:  PITCH SHIFT
    "org.mixxx.effects.parametriceq",        // Pos 4:  PARAM EQ
    "org.mixxx.effects.metronome",           // Pos 5:  METRONOME
    "org.mixxx.effects.loudnesscontour",     // Pos 6:  LOUDNESS
    "org.mixxx.effects.glitch",              // Pos 7:  GLITCH
    "org.mixxx.effects.biquadfullkilllowpassfilter", // Pos 8: FILTER
    "org.mixxx.effects.distortedflanger",    // Pos 9:  DISTORTION
    "org.mixxx.effects.biquadfullkillhighpassfilter", // Pos 10: HPF
    "org.mixxx.effects.bitcrusher",          // Pos 11: BITCRUSHER
    "org.mixxx.effects.echo"                 // Pos 12: ECHO
];

PioneerDDJGRV6.playBlinkTimers = {};
PioneerDDJGRV6.playBlinkStates = {};


// see beatSyncHandler()
PioneerDDJGRV6.syncTimers = {};
//
// Globales Objekt für Loop-Timer 
PioneerDDJGRV6.loopTimers = {};

for (var i = 0; i < 4; i++) {
    PioneerDDJGRV6.lights.decks[i] = {
        vuMeter: {
            status: 0xB0 + i, 
            data1: 0x02,
            levels: {
                off: 0x00,
                oneGreen: 0x26,     // Leuchtet 1 grünes Segment 
                twoGreens: 0x41,    // Leuchtet 2 grüne Segmente 
                orangeGreen: 0x57,  // 1 Orange + 2 Grüne 
                twoOranges: 0x65,   // 2 Oranges + 2 Grüne 
                redFull: 0x77       // Rot + 2 Oranges + 2 Grüne (Peak) 
            }
        },
        playPause: {
            status: 0x90 + i, // 0x90, 0x91, 0x92, 0x93
            data1: 0x0B,
        },
        shiftPlayPause: {
            status: 0x90 + i,
            data1: 0x47,
        },
        cue: {
            status: 0x90 + i,
            data1: 0x0C,
        },
        shiftCue: {
            status: 0x90 + i,
            data1: 0x48,
        },
    };
}
// Store timer IDs
PioneerDDJGRV6.timers = {};

// Jog wheel constants
PioneerDDJGRV6.vinylMode = true;
PioneerDDJGRV6.alpha = 1.0/8;
PioneerDDJGRV6.beta = PioneerDDJGRV6.alpha/32;

// Multiplier for fast seek through track using SHIFT+JOGWHEEL
PioneerDDJGRV6.fastSeekScale = 150;
PioneerDDJGRV6.bendScale = 0.8;

PioneerDDJGRV6.tempoRanges = [0.06, 0.10, 0.16, 0.25];

PioneerDDJGRV6.shiftButtonDown = [false, false];

// Jog wheel loop adjust - 4 channels
PioneerDDJGRV6.loopAdjustIn = [false, false, false, false];
PioneerDDJGRV6.loopAdjustOut = [false, false, false, false];
PioneerDDJGRV6.loopAdjustMultiply = 50;

// Beatjump pad (beatjump_size values)
PioneerDDJGRV6.beatjumpSizeForPad = {
    0x20: -1, // PAD 1
    0x21: 1,  // PAD 2
    0x22: -2, // PAD 3
    0x23: 2,  // PAD 4
    0x24: -4, // PAD 5
    0x25: 4,  // PAD 6
    0x26: -8, // PAD 7
    0x27: 8   // PAD 8
};

PioneerDDJGRV6.quickJumpSize = 32;

// Used for tempo slider
PioneerDDJGRV6.highResMSB = {
    "[Channel1]": {},
    "[Channel2]": {},
    "[Channel3]": {},
    "[Channel4]": {}
};

PioneerDDJGRV6.trackLoadedLED = function(value, group, _control) {
    midi.sendShortMsg(
        0x9F,
        group.match(script.channelRegEx)[1] - 1,
        value > 0 ? 0x7F : 0x00
    );
};

PioneerDDJGRV6.toggleLight = function(midiIn, active) {
    midi.sendShortMsg(midiIn.status, midiIn.data1, active ? 0x7F : 0);
};

PioneerDDJGRV6.LightOff = function() {
    for (var i = 0; i < 4; i++) {
        engine.makeUnbufferedConnection("[Channel"+(i+1)+"]", "vu_meter", PioneerDDJGRV6.vuMeterUpdate);
        midi.sendShortMsg(PioneerDDJGRV6.lights.decks[i].vuMeter.status, 0x02, 0x00);

        // switch headphone (CUE) indicator off
        midi.sendShortMsg(0x90 + i, 0x54, 0x00);

        // beat sync LED off
        midi.sendShortMsg(0x90 + i, 0x58, 0x00);
    }

    // housekeeping
    // turn off all Sampler LEDs
    for (var i = 0; i <= 7; ++i) {
        midi.sendShortMsg(0x97, 0x30 + i, 0x00);    // Deck 1 pads
        midi.sendShortMsg(0x98, 0x30 + i, 0x00);    // Deck 1 pads with SHIFT
        midi.sendShortMsg(0x99, 0x30 + i, 0x00);    // Deck 2 pads  
        midi.sendShortMsg(0x9A, 0x30 + i, 0x00);    // Deck 2 pads with SHIFT
    }
    // turn off all Hotcue LEDs
    for (var i = 0; i <= 7; ++i) {
        midi.sendShortMsg(0x97, 0x00 + i, 0x00);    // Deck 1 pads
        midi.sendShortMsg(0x98, 0x00 + i, 0x00);    // Deck 1 pads with SHIFT
        midi.sendShortMsg(0x99, 0x00 + i, 0x00);    // Deck 2 pads
        midi.sendShortMsg(0x9A, 0x00 + i, 0x00);    // Deck 2 pads with SHIFT
    }

    // turn off loop in and out lights
    PioneerDDJGRV6.setLoopButtonLights(0x90, 0x00);
    PioneerDDJGRV6.setLoopButtonLights(0x91, 0x00);

    // turn off reloop lights
    PioneerDDJGRV6.setReloopLight(0x90, 0x00);
    PioneerDDJGRV6.setReloopLight(0x91, 0x00);

    // stop any flashing lights
    PioneerDDJGRV6.toggleLight(PioneerDDJGRV6.lights.beatFx, false);
    PioneerDDJGRV6.toggleLight(PioneerDDJGRV6.lights.shiftBeatFx, false);
}

PioneerDDJGRV6.initFx = function() {
    console.log("Pioneer DDJ-GRV6: initFx start");
    var unit = "[EffectRack1_EffectUnit1]";

    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 1);
    engine.setValue("[EffectRack1_EffectUnit1_Effect1]", "effect_selector", 0.115);
    engine.setValue("[EffectRack1_EffectUnit1_Effect1]", "enabled", 1);

    console.log("Pioneer DDJ-GRV6: initFx end");
};


PioneerDDJGRV6.init = function() {
    console.log("Pioneer DDJ-GRV6: init() starting...");

    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);
    engine.softTakeover("[Channel3]", "rate", true);
    engine.softTakeover("[Channel4]", "rate", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect1]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect2]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect3]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1]", "mix", true);


    engine.setValue("[EffectRack1_EffectUnit1]", "show_focus", 1);
    PioneerDDJGRV6.LightOff();

    // SAMPLER SETUP ---
    const samplerCount = 16;
    if (engine.getValue("[App]", "num_samplers") < samplerCount) {
        engine.setValue("[App]", "num_samplers", samplerCount);
    }
    for (var i = 1; i <= samplerCount; i++) {
        engine.makeConnection("[Sampler" + i + "]", "play", PioneerDDJGRV6.samplerPlayOutputCB);
    }

    for (var i = 1; i <= 4; i++) {
        var group = "[Channel" + i + "]";
        var deckIdx = i - 1;

        // Rate & LEDs
        engine.softTakeover(group, "rate", true);
        engine.makeConnection(group, "track_loaded", PioneerDDJGRV6.trackLoadedLED);
        engine.makeConnection(group, "play", PioneerDDJGRV6.playLEDHandler);
        engine.makeConnection(group, "playposition", PioneerDDJGRV6.jogWheelLED);
        engine.makeConnection(group, "loop_enabled", PioneerDDJGRV6.loopToggle);

        // Hardware Resets (MIDI Status 0x90-0x93, 0xB0-0xB3)
        midi.sendShortMsg(0x90 + deckIdx, 0x3F, 0x40); // Jog LED
        midi.sendShortMsg(0xB0 + deckIdx, 0x3F, 0x7F); // Brightness
        midi.sendShortMsg(0xB0 + deckIdx, 0x0C, 0x7F); // Alt Brightness
        
        // enforce initial-Status
        engine.trigger(group, "play");
        engine.trigger(group, "playposition");
        engine.trigger(group, "cue_default");
        engine.trigger(group, "vu_meter");
    }

    PioneerDDJGRV6.setLoopButtonLights(0x90, 0x7F);
    PioneerDDJGRV6.setLoopButtonLights(0x91, 0x7F);
    midi.sendShortMsg(0x9F, 0x00, 0x7F); // Startup Animation Deck 1
    midi.sendShortMsg(0x9F, 0x01, 0x7F); // Startup Animation Deck 2

    for (var i = 1; i <= 3; i++) {
        engine.makeConnection("[EffectRack1_EffectUnit1_Effect" + i +"]", "enabled", PioneerDDJGRV6.toggleFxLight);
    }
    engine.makeConnection("[EffectRack1_EffectUnit1]", "focused_effect", PioneerDDJGRV6.toggleFxLight);
    engine.makeConnection("[EffectRack1_EffectUnit1]", "enabled", PioneerDDJGRV6.updateFxLed);

    // TODO: 2x check
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7E, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0xF7]);
    midi.sendSysexMsg([0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x06, 0x00, 0x03, 0x01, 0xF7], 12);

    PioneerDDJGRV6.updateFxLed(engine.getValue("[EffectRack1_EffectUnit1]", "enabled"));

    PioneerDDJGRV6.initFx();

    console.log("Pioneer DDJ-GRV6: Initialisierung abgeschlossen.");
};



//
// Channel level lights
//

PioneerDDJGRV6.vuMeterUpdate = function(value, group) {
    const newVal = value * 150;

    switch (group) {
    case "[Channel1]":
        midi.sendShortMsg(0xB0, 0x02, newVal);
        break;

    case "[Channel2]":
        midi.sendShortMsg(0xB1, 0x02, newVal);
        break;

    case "[Channel3]":
        midi.sendShortMsg(0xB2, 0x02, newVal);
        break;

    case "[Channel4]":
        midi.sendShortMsg(0xB3, 0x02, newVal);
        break;
    }
};

//
// Effects
//

PioneerDDJGRV6.toggleFxLight = function(_value, _group, _control) {
    const enabled = engine.getValue(PioneerDDJGRV6.focusedFxGroup(), "enabled");

    PioneerDDJGRV6.toggleLight(PioneerDDJGRV6.lights.beatFx, enabled);
    PioneerDDJGRV6.toggleLight(PioneerDDJGRV6.lights.shiftBeatFx, enabled);
};

PioneerDDJGRV6.focusedFxGroup = function() {
    var focusedFx = engine.getValue("[EffectRack1_EffectUnit1]", "focused_effect");
    // Falls 0 geliefert wird (kein Fokus), nehmen wir Slot 1 als Standard
    if (focusedFx < 1) {
        focusedFx = 1;
    }
    return "[EffectRack1_EffectUnit1_Effect" + focusedFx + "]";
};


//
// Loop IN/OUT ADJUST
//

PioneerDDJGRV6.toggleLoopAdjustIn = function(channel, _control, value, _status, group) {
    if (value === 0 || engine.getValue(group, "loop_enabled" === 0)) {
        return;
    }
    PioneerDDJGRV6.loopAdjustIn[channel] = !PioneerDDJGRV6.loopAdjustIn[channel];
    PioneerDDJGRV6.loopAdjustOut[channel] = false;
};

PioneerDDJGRV6.toggleLoopAdjustOut = function(channel, _control, value, _status, group) {
    if (value === 0 || engine.getValue(group, "loop_enabled" === 0)) {
        return;
    }
    PioneerDDJGRV6.loopAdjustOut[channel] = !PioneerDDJGRV6.loopAdjustOut[channel];
    PioneerDDJGRV6.loopAdjustIn[channel] = false;
};

// Two signals are sent here so that the light stays lit/unlit in its shift state too
PioneerDDJGRV6.setReloopLight = function(status, value) {
    midi.sendShortMsg(status, 0x4D, value);
    midi.sendShortMsg(status, 0x50, value);
};


PioneerDDJGRV6.setLoopButtonLights = function(status, value) {
    [0x10, 0x11, 0x4E, 0x4C].forEach(function(control) {
        midi.sendShortMsg(status, control, value);
    });
};

PioneerDDJGRV6.startLoopLightsBlink = function(channel, control, status, group) {
    var blink = 0x7F;

    PioneerDDJGRV6.stopLoopLightsBlink(group, control, status);

    PioneerDDJGRV6.timers[group][control] = engine.beginTimer(500, () => {
        blink = 0x7F - blink;

        // When adjusting the loop out position, turn the loop in light off
        if (PioneerDDJGRV6.loopAdjustOut[channel]) {
            midi.sendShortMsg(status, 0x10, 0x00);
            midi.sendShortMsg(status, 0x4C, 0x00);
        } else {
            midi.sendShortMsg(status, 0x10, blink);
            midi.sendShortMsg(status, 0x4C, blink);
        }

        // When adjusting the loop in position, turn the loop out light off
        if (PioneerDDJGRV6.loopAdjustIn[channel]) {
            midi.sendShortMsg(status, 0x11, 0x00);
            midi.sendShortMsg(status, 0x4E, 0x00);
        } else {
            midi.sendShortMsg(status, 0x11, blink);
            midi.sendShortMsg(status, 0x4E, blink);
        }
    });

};

PioneerDDJGRV6.stopLoopLightsBlink = function(group, control, status) {
    PioneerDDJGRV6.timers[group] = PioneerDDJGRV6.timers[group] || {};

    if (PioneerDDJGRV6.timers[group][control] !== undefined) {
        engine.stopTimer(PioneerDDJGRV6.timers[group][control]);
    }
    PioneerDDJGRV6.timers[group][control] = undefined;
    PioneerDDJGRV6.setLoopButtonLights(status, 0x7F);
};

PioneerDDJGRV6.loopToggle = function(value, group, control) {
    // TODO: 4 channels
    const status = group === "[Channel1]" ? 0x90 : 0x91,
        channel = group === "[Channel1]" ? 0 : 1;

    PioneerDDJGRV6.setReloopLight(status, value ? 0x7F : 0x00);

    if (value) {
        PioneerDDJGRV6.startLoopLightsBlink(channel, control, status, group);
    } else {
        PioneerDDJGRV6.stopLoopLightsBlink(group, control, status);
        PioneerDDJGRV6.loopAdjustIn[channel] = false;
        PioneerDDJGRV6.loopAdjustOut[channel] = false;
    }
};

/**
 * TODO: finish the implementation....
 * Handler für die BEAT SYNC Taste (D19)
 * - Short Press: Beatsync (1x angleichen) oder Toggle-Off (falls Sync aktiv)
 * - Long Press: Sync Lock (Dauerhaft)
 */
PioneerDDJGRV6.beatSyncHandler = function (channel, control, value, status, group) {
    var syncNote = 0x58; // D19 Note laut MIDI-Dokumentation
    
    var deckNum = parseInt(group.substring(8, 9)) - 1;
    var ledStatus = 0x90 + deckNum;

    if (value > 0) {
        PioneerDDJGRV6.syncTimers[group] = engine.beginTimer(300, function() {
            // DAS PASSIERT NACH 300ms (LONG PRESS)
            engine.setValue(group, "sync_enabled", 1);
            PioneerDDJGRV6.syncTimers[group] = 0; // Timer löschen
            
            midi.sendShortMsg(ledStatus, syncNote, 0x7F);
        }, true);
        
    } else {
        // --- TASTE LOSGELASSEN ---
        if (PioneerDDJGRV6.syncTimers[group]) {
            engine.stopTimer(PioneerDDJGRV6.syncTimers[group]);
            PioneerDDJGRV6.syncTimers[group] = 0;

            if (engine.getValue(group, "sync_enabled")) {
                engine.setValue(group, "sync_enabled", 0);
                midi.sendShortMsg(ledStatus, syncNote, 0x00);
            } else {
                engine.setValue(group, "beatsync", 1);
                engine.beginTimer(50, function() { 
                    engine.setValue(group, "beatsync", 0); 
                }, true);
                
                midi.sendShortMsg(ledStatus, syncNote, 0x00);
            }
        }
    }
};



//
// Jog wheels
//

PioneerDDJGRV6.jogTurn = function(channel, _control, value, _status, group) {
    const deckNum = channel + 1;
    // wheel center at 64; <64 rew >64 fwd
    var newVal = value - 64;

    // loop_in / out adjust
    const loopEnabled = engine.getValue(group, "loop_enabled");
    if (loopEnabled > 0) {
        if (PioneerDDJGRV6.loopAdjustIn[channel]) {
            newVal = newVal * PioneerDDJGRV6.loopAdjustMultiply + engine.getValue(group, "loop_start_position");
            engine.setValue(group, "loop_start_position", newVal);
            return;
        }
        if (PioneerDDJGRV6.loopAdjustOut[channel]) {
            newVal = newVal * PioneerDDJGRV6.loopAdjustMultiply + engine.getValue(group, "loop_end_position");
            engine.setValue(group, "loop_end_position", newVal);
            return;
        }
    }

    if (engine.isScratching(deckNum)) {
        engine.scratchTick(deckNum, newVal);
    } else { // fallback
        engine.setValue(group, "jog", newVal * this.bendScale);
    }
};


PioneerDDJGRV6.jogSearch = function(_channel, _control, value, _status, group) {
    const newVal = (value - 64) * PioneerDDJGRV6.fastSeekScale;
    engine.setValue(group, "jog", newVal);
};

PioneerDDJGRV6.jogTouch = function(channel, _control, value) {
    const deckNum = channel + 1;

    // skip while adjusting the loop points
    if (PioneerDDJGRV6.loopAdjustIn[channel] || PioneerDDJGRV6.loopAdjustOut[channel]) {
        return;
    }

    if (value !== 0 && this.vinylMode) {
        engine.scratchEnable(deckNum, 720, 33+1/3, this.alpha, this.beta);
    } else {
        engine.scratchDisable(deckNum);
    }
};

//
// Shift button
// TODO: is this required?
//

PioneerDDJGRV6.shiftPressed = function(channel, _control, value, _status, _group) {
    PioneerDDJGRV6.shiftButtonDown[channel] = value === 0x7F;
};


//
// Tempo sliders
//
// The tempo option in Mixxx's deck preferences determine whether down/up
// increases/decreases the rate. Therefore it must be inverted here so that the
// UI and the control sliders always move in the same direction.
//

PioneerDDJGRV6.tempoSliderMSB = function(channel, control, value, status, group) {
    PioneerDDJGRV6.highResMSB[group].tempoSlider = value;
};

PioneerDDJGRV6.tempoSliderLSB = function(channel, control, value, status, group) {
    const fullValue = (PioneerDDJGRV6.highResMSB[group].tempoSlider << 7) + value;

    engine.setValue(
        group,
        "rate",
        1 - (fullValue / 0x2000)
    );
};


PioneerDDJGRV6.samplerPlayOutputCB = function(value, group, _control) {
    if (value === 1) {
        const curPad = group.match(script.samplerRegEx)[1];
        PioneerDDJGRV6.startSamplerBlink(
            0x97 + (curPad > 8 ? 2 : 0),
            0x30 + ((curPad > 8 ? curPad - 8 : curPad) - 1),
            group);
    }
};


PioneerDDJGRV6.startSamplerBlink = function(channel, control, group) {
    var val = 0x7f;

    PioneerDDJGRV6.stopSamplerBlink(channel, control);
    PioneerDDJGRV6.timers[channel][control] = engine.beginTimer(250, () => {
        val = 0x7f - val;

        // blink the appropriate pad
        midi.sendShortMsg(channel, control, val);
        // also blink the pad while SHIFT is pressed
        midi.sendShortMsg((channel+1), control, val);

        const isPlaying = engine.getValue(group, "play") === 1;

        if (!isPlaying) {
            // kill timer
            PioneerDDJGRV6.stopSamplerBlink(channel, control);
            // set the pad LED to ON
            midi.sendShortMsg(channel, control, 0x7f);
            // set the pad LED to ON while SHIFT is pressed
            midi.sendShortMsg((channel+1), control, 0x7f);
        }
    });
};

PioneerDDJGRV6.stopSamplerBlink = function(channel, control) {
    PioneerDDJGRV6.timers[channel] = PioneerDDJGRV6.timers[channel] || {};

    if (PioneerDDJGRV6.timers[channel][control] !== undefined) {
        engine.stopTimer(PioneerDDJGRV6.timers[channel][control]);
        PioneerDDJGRV6.timers[channel][control] = undefined;
    }
};


PioneerDDJGRV6.vu_meter = function(value, group, control) {
    var deckIdx = -1;

    // TODO: fix below deckIdx
    if (group === "[Channel1]") deckIdx = 0;
    else if (group === "[Channel2]") deckIdx = 1;
    else if (group === "[Channel3]") deckIdx = 2;
    else if (group === "[Channel4]") deckIdx = 3;

    if (deckIdx === -1) return;

    var vuConfig = PioneerDDJGRV6.lights.decks[deckIdx].vuMeter;
    var levels = vuConfig.levels;
    var signal = levels.off;

    if (value > 0.95) {
        signal = levels.redFull;
    } else if (value > 0.80) {
        signal = levels.twoOranges;
    } else if (value > 0.60) {
        signal = levels.orangeGreen;
    } else if (value > 0.40) {
        signal = levels.twoGreens;
    } else if (value > 0.15) {
        signal = levels.oneGreen;
    } else {
        signal = levels.off;
    }

    midi.sendShortMsg(vuConfig.status, vuConfig.data1, signal);
};

PioneerDDJGRV6.lastJogPos = [-1, -1, -1, -1];

PioneerDDJGRV6.jogWheelLED = function (value, group, control) {
    var deckIdx = -1;
    if (group === "[Channel1]") deckIdx = 0;

    // TODO: fix below deckIdx
    else if (group === "[Channel2]") deckIdx = 1;
    else if (group === "[Channel3]") deckIdx = 2;
    else if (group === "[Channel4]") deckIdx = 3;
    if (deckIdx === -1) return;

    
    var duration = engine.getValue(group, "duration");
    if (duration <= 0) return;

    var elapsedTime = value * duration;
    var rpm = 33.33; 
    var secondsPerRotation = 60 / rpm;
    
    var rawPos = (elapsedTime % secondsPerRotation) / secondsPerRotation;
    var ledPos = Math.floor(rawPos * 127);

    if (PioneerDDJGRV6.lastJogPos[deckIdx] !== ledPos) {
        midi.sendShortMsg(0x90 + deckIdx, 0x3F, ledPos);
        PioneerDDJGRV6.lastJogPos[deckIdx] = 63; // ledPos;
    }
};


// F5: pick the effect
// FIXME: doesn't work
PioneerDDJGRV6.fxSelectAbsolute = function (channel, control, value, status, group) {
    console.log("PioneerDDJGRV6.fxSelectAbsolute("+channel+", "+ control+ ", "+value+", "+status+ ", "+ group +") F5");
    // WICHTIG: Nur auf das Einrasten reagieren (127), nicht auf das Loslassen (0)
    if (value !== 0x7F) return; 

    var idx = control - 0x20; 
    var effectId = PioneerDDJGRV6.effectIdTable[idx];
    if (effectId) {
        // Wir schreiben die Pfade komplett aus - das ist am sichersten gegen Tippfehler
        var slot1 = "[EffectRack1_EffectUnit1_Effect1]";
        var slot2 = "[EffectRack1_EffectUnit1_Effect2]";
        var slot3 = "[EffectRack1_EffectUnit1_Effect3]";
        var unit  = "[EffectRack1_EffectUnit1]";

        // 1. Slots deaktivieren (setValue für Zahlen 0/1)
        engine.setValue(slot2, "enabled", 0);
        engine.setValue(slot3, "enabled", 0);

        // 2. Effekt laden (setParameter für Text-IDs)
        //engine.setParameter(slot1, "load_effect", effectId);
        engine.setValue(slot1, "effect_selector", effectId);
        
        // 3. Aktivierung (setValue für Zahlen)
        engine.setValue(slot1, "enabled", 1);
        engine.setValue(slot1, "meta", 0.5);
        engine.setValue(unit, "group_[Channel1]_enable", 1);

        console.log("PioneerDDJGRV6.fxSelectAbsolute ERFOLG: Pos " + idx + " lädt: " + effectId);
    } else {
        console.log("PioneerDDJGRV6.fxSelectAbsolute: Keine ID für Index " + idx + " in Tabelle gefunden.");
    }
};

// F6: route the channel
PioneerDDJGRV6.fxChannelSelect = function (channel, control, value, status, group) {
    console.log("PioneerDDJGRV6.fxChannelSelect("+channel+", "+ control+ ", "+value+", "+status+ ", "+ group +") F6");
    if (value === 0) return 0; // ifnore the "off" note

    var midiToTarget = {
        0x10: "[Channel1]",
        0x11: "[Channel2]",
        0x12: "[Channel3]",
        0x13: "[Channel4]",
        0x14: "[Master]",
        0x16: "[Samplers]", // "SP" key in the F6 block of keys.
    };

    var targetDeck = midiToTarget[control];
    if (targetDeck) {
        var allTargets = ["[Channel1]", "[Channel2]", "[Channel3]", "[Channel4]", "[Master]", "[Samplers]"];
        
        // switch the targets correct: all to "off" except the one for "targetDeck"
        for (var i = 0; i < allTargets.length; i++) {
            var parameterName = "group_" + allTargets[i] + "_enable";
            var shouldBeEnabled = (targetDeck === allTargets[i]) ? 1 : 0;
            engine.setParameter(group, parameterName, shouldBeEnabled);
        }
        
        print("F6 CH SELECT: Routing zu " + targetDeck + " aktiviert.");
    }
};


PioneerDDJGRV6.updateFxLed = function (value) {
    var status = 0x94; 
    var midino = 0x47; 
    
    if (value > 0) {
        midi.sendShortMsg(status, midino, 0x7F); // Ganz hell
    } else {
        // send: off
        midi.sendShortMsg(status, midino, 0x00);
        // fallback. TODO: is this required?
        midi.sendShortMsg(0x84, midino, 0x00); 
    }
};


// F8: effect on/off 
PioneerDDJGRV6.beatFxOnOff = function (channel, control, value, status, group) {
    console.log("PioneerDDJGRV6.beatFxOnOff("+channel+", "+ control+ ", "+value+", "+status+ ", "+ group +") F8");

        if (value > 0) { // Nur beim Drücken
        var isEnabled = engine.getValue(group, "enabled");
        var newState = isEnabled ? 0 : 1;
        
        engine.setValue(group, "enabled", newState);

        if (newState === 1) {
            engine.setValue("[EffectRack1_EffectUnit1_Effect1]", "enabled", 1);
            if (engine.getValue("[EffectRack1_EffectUnit1_Effect1]", "meta") === 0) {
                engine.setValue("[EffectRack1_EffectUnit1_Effect1]", "meta", 0.5);
            }
        }
    }
};


PioneerDDJGRV6.f7_msb = 0;
PioneerDDJGRV6.f7_lsb = 0;

PioneerDDJGRV6.levelDepthDirect = function (channel, control, value, status, group) {
    console.log("PioneerDDJGRV6.levelDepthDirect("+channel+", "+ control+ ", "+value+", "+status+ ", "+ group +") f7_msb: "+PioneerDDJGRV6.f7_msb+" f7:lsb:"+PioneerDDJGRV6.f7_lsb+", F8");
    // control 0x02 (2) is MSB
    // control 0x22 (34) is LSB
    
    if (control === 0x02) {
        PioneerDDJGRV6.f7_msb = value;
    } else if (control === 0x22) {
        PioneerDDJGRV6.f7_lsb = value;
    }

    var fullVal = (PioneerDDJGRV6.f7_msb << 7) | PioneerDDJGRV6.f7_lsb;
    var normalized = fullVal / 16383;

    // move the UI element in mixxx
    engine.setValue(group, "super1", normalized);


    if (normalized > 0) {
        for (var i = 1; i <= 3; i++) {
            var slot = "[EffectRack1_EffectUnit1_Effect" + i + "]";
            if (engine.getValue(slot, "enabled") === 0) engine.setValue(slot, "enabled", 1);
        }
    }
};





// Funktion für den ON/OFF Button (F8)

PioneerDDJGRV6.playLEDHandler = function (value, group, control) {
    var deckNum = parseInt(group.substring(8, 9)) - 1;
    var status = 0x90 + deckNum;
    var midino = 0x0B; // D22 Taste

    if (value > 0) {
        // --- constant on: song is playing
        if (PioneerDDJGRV6.playBlinkTimers[group]) {
            engine.stopTimer(PioneerDDJGRV6.playBlinkTimers[group]);
            PioneerDDJGRV6.playBlinkTimers[group] = null;
        }
        midi.sendShortMsg(status, midino, 0x7F); // Dauer-An
    } else {
        // just blink, even when a track is loaded
        if (engine.getValue(group, "track_loaded")) {
            if (!PioneerDDJGRV6.playBlinkTimers[group]) {
                PioneerDDJGRV6.playBlinkStates[group] = false;
                PioneerDDJGRV6.playBlinkTimers[group] = engine.beginTimer(500, function() {
                    PioneerDDJGRV6.playBlinkStates[group] = !PioneerDDJGRV6.playBlinkStates[group];
                    var ledVal = PioneerDDJGRV6.playBlinkStates[group] ? 0x7F : 0x00;
                    midi.sendShortMsg(status, midino, ledVal);
                });
            }
        } else {
            // no track loaded: LED to off
            if (PioneerDDJGRV6.playBlinkTimers[group]) {
                engine.stopTimer(PioneerDDJGRV6.playBlinkTimers[group]);
                PioneerDDJGRV6.playBlinkTimers[group] = null;
            }
            midi.sendShortMsg(status, midino, 0x00);
        }
    }
};



PioneerDDJGRV6.shutdown = function() {
    // reset vumeter
    PioneerDDJGRV6.LightOff();
};

// just for debugging

PioneerDDJGRV6.clickCount = 0;

PioneerDDJGRV6.scanEffects = function (channel, control, value, status, group) {
    if (value === 0) return; 

    var slot1 = "[EffectRack1_EffectUnit1_Effect1]";

    if (control === 0x21) {
        console.log("--- RESET ---");
        for(var i=0; i<20; i++) engine.setValue(slot1, "prev_effect", 1);
        PioneerDDJGRV6.clickCount = 0;
        return;
    }

    engine.setValue(slot1, "next_effect", 1);
    PioneerDDJGRV6.clickCount++;

    console.log("--- GRV6 FX search ---");
    console.log("clicks since start: " + PioneerDDJGRV6.clickCount);
    console.log("----------------------");
};


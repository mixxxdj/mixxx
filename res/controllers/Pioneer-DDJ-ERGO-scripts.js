//Controller
var DDJERGO = {};

//This will hold the last FX button clicked, so the FX on that slot can be manipulated
var selectedEffects = [
    0,
    0,
];

//This helps grab FX dynamically
const effectsStrings = [
    [
        "[EffectRack1_EffectUnit1_Effect1]",
        "[EffectRack1_EffectUnit1_Effect2]",
        "[EffectRack1_EffectUnit1_Effect3]",
    ],
    [
        "[EffectRack1_EffectUnit2_Effect1]",
        "[EffectRack1_EffectUnit2_Effect2]",
        "[EffectRack1_EffectUnit2_Effect3]",
    ],
];

//This stores the last position of the FX control knob so when this changes a significant amount, a new effect is chosen
var previousEffectKnobPosition = 0;

//Sample vol knob triggers the last sample played when clicked, and changes it when shift + turn
var selectedSamples = [
    0,
    0,
];

//This helps grab samples dynamically
const samplerStrings = [
    "[Sampler1]",
    "[Sampler2]",
    "[Sampler3]",
    "[Sampler4]",
    "[Sampler5]",
    "[Sampler6]",
    "[Sampler7]",
    "[Sampler8]",
];

//These store the last beat played timestamp in milliseconds to implement the pulse mode
var timestamps = [
    0,
    0,
];

//These help blink the loop in/out buttons in a Pioneer way
var loopIn = 0;
var loopOut = 0;

//These store the loop in/out buttons blink state for each channel
var blink = [
    1,
    1,
];

//Init function
DDJERGO.init = function() {
    //Sysex message to initialize sound card
    const ControllerStatusSysex = [0xB0, 0x4B, 0x20, 0x00, 0xB0, 0x4B, 0x40, 0xF7, 0xB0, 0x4B, 0x42, 0xF7];
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
    //Messages to initialize fader LEDs
    let i;
    for (i = 0; i < 6; i++) {
        midi.sendShortMsg(0xB6, 0x1F-i, 0x40);
    }
    for (i = 0; i < 4; i++) {
        midi.sendShortMsg(0xB6, 0x19-2*i, 0x7F);
    }
    for (i = 0; i < 12; i++) {
        midi.sendShortMsg(0xB6, 0x12-i, 0x40);
    }
    for (i = 0; i < 4; i++) {
        midi.sendShortMsg(0xB6, 0x19-2*i, 0x00);
    }
    //effect unit 1
    DDJERGO.effectUnitChannel1 = new components.EffectUnit([1, 3]);
    DDJERGO.effectUnitChannel1.enableButtons[1].midi = [0x94, 0x47];
    DDJERGO.effectUnitChannel1.enableButtons[2].midi = [0x94, 0x48];
    DDJERGO.effectUnitChannel1.enableButtons[3].midi = [0x94, 0x49];
    DDJERGO.effectUnitChannel1.knobs[1].midi = [0xB4, 0x02];
    DDJERGO.effectUnitChannel1.knobs[2].midi = [0xB4, 0x04];
    DDJERGO.effectUnitChannel1.knobs[3].midi = [0xB4, 0x06];
    DDJERGO.effectUnitChannel1.dryWetKnob.midi = [0xB4, 0x00];
    DDJERGO.effectUnitChannel1.effectFocusButton.midi = [0x94, 0x43];
    DDJERGO.effectUnitChannel1.init();
    //effect unit 2
    DDJERGO.effectUnitChannel2 = new components.EffectUnit([2, 4]);
    DDJERGO.effectUnitChannel2.enableButtons[1].midi = [0x95, 0x47];
    DDJERGO.effectUnitChannel2.enableButtons[2].midi = [0x95, 0x48];
    DDJERGO.effectUnitChannel2.enableButtons[3].midi = [0x95, 0x49];
    DDJERGO.effectUnitChannel2.knobs[1].midi = [0xB5, 0x02];
    DDJERGO.effectUnitChannel2.knobs[2].midi = [0xB5, 0x04];
    DDJERGO.effectUnitChannel2.knobs[3].midi = [0xB5, 0x06];
    DDJERGO.effectUnitChannel2.dryWetKnob.midi = [0xB5, 0x00];
    DDJERGO.effectUnitChannel2.effectFocusButton.midi = [0x95, 0x43];
    DDJERGO.effectUnitChannel2.init();
    //callback connections
    engine.makeConnection("[Channel1]", "beat_active", pulseModeCallback);
    engine.makeConnection("[Channel2]", "beat_active", pulseModeCallback);
    engine.makeConnection("[Channel1]", "beat_active", loopLEDsCallback);
    engine.makeConnection("[Channel2]", "beat_active", loopLEDsCallback);
    engine.makeConnection("[Channel1]", "loop_in", loopLEDsCallback);
    engine.makeConnection("[Channel2]", "loop_in", loopLEDsCallback);
    engine.makeConnection("[Channel1]", "loop_out", loopLEDsCallback);
    engine.makeConnection("[Channel2]", "loop_out", loopLEDsCallback);
    engine.makeConnection("[Channel1]", "loop_enabled", loopLEDsCallback);
    engine.makeConnection("[Channel2]", "loop_enabled", loopLEDsCallback);
    engine.makeUnbufferedConnection("[Channel1]", "beat_active", volumeLEDsCallback);
    engine.makeUnbufferedConnection("[Channel2]", "beat_active", volumeLEDsCallback);
    engine.makeConnection("[QuickEffectRack1_[Channel1]]", "super1", filterLEDsCallback);
    engine.makeConnection("[QuickEffectRack1_[Channel2]]", "super1", filterLEDsCallback);
    engine.makeConnection("[Channel1]", "LoadSelectedTrack", loadingAnimationCallback);
    engine.makeConnection("[Channel2]", "LoadSelectedTrack", loadingAnimationCallback);
};

//shutdown
DDJERGO.shutdown = function() {

};

//Scroll through library with browse knob
DDJERGO.browseLibrary = function(channel, control, value) {
    let newValue;
    if (value-64 > 0) { newValue = value-128; } else { newValue = value; }
    engine.setValue("[Library]", "MoveVertical", newValue);
};

//Nudge track for beat matching when wheel is turned
DDJERGO.nudge = function(channel, control, value, status, group) {
    //this is because the controller never sends a 0x40 message and hence never stops nudging
    if (Math.abs(value-0x40)===1) {
        value = 0x40;
    }
    const newValue = value-64;
    // 0.5 * val / 64 seems to yield the best results
    engine.setValue(group, "wheel", 0.5*newValue/64);
};

//Start a beatloop when autoloop is pressed
DDJERGO.beatloop = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (engine.getParameter(group, "loop_enabled") === 1) {
            engine.setParameter(group, "beatloop_activate", 1);
        } else {
            engine.setParameter(group, "beatloop_activate", 1);
        }
    }
};

// The button that enables/disables scratching
DDJERGO.wheelTouch = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const alpha = 1.0/8;
    const beta = alpha/32;
    if (value === 0x7F) {
        engine.scratchEnable(deckNumber, 2048, 33+1/3, alpha, beta);
    } else {
        engine.scratchDisable(deckNumber);
    }
};

// The wheel that actually controls the scratching
DDJERGO.wheelTurn = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const newValue = value - 64;
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue);
    } else {
        engine.setValue(group, "jog", newValue);
    }
};

//multiply/divide loop size when autoloop wheel is turned
DDJERGO.autoloopMultiply = function(channel, control, value, status, group) {
    const newValue = value-64;
    if (newValue > 0) {
        engine.setParameter(group, "loop_halve", 1);
    }
    if (newValue < 0) {
        engine.setParameter(group, "loop_double", 1);
    }
};

//this inverts the tempo slider. Didn't seem to work in the xml
DDJERGO.rate = function(channel, control, value, status, group) {
    if (control === 0x00) {
        const newValue = value - 64;
        engine.setValue(group, "rate", -1 * newValue / 64);
    }
};

//change tempo rate in 8% increments (32% max) with shift + keylock button
DDJERGO.setTempoRange = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        let curr = engine.getValue(group, "rateRange");
        if (curr > 0.89) {
            curr = 0.08;
        } else if (curr > 0.49) {
            curr = 0.90;
        } else if (curr > 0.31) {
            curr = 0.50;
        } else {
            curr += 0.08;
        }
        engine.setValue(group, "rateRange", curr);
    }
};

//move quickly through the track when shift + jog wheel touch + turn
DDJERGO.searchTrack = function(channel, control, value, status, group) {
    //this is because the controller never sends a 0x40 message and hence never stops searching
    if (Math.abs(value-0x40)===1) {
        value = 0x40;
    }
    const newValue = value-64;
    // 3 * val seems to yield the best results
    engine.setValue(group, "rateSearch", 3*newValue);
};

//move the current loop by one beat when shift + in/out
DDJERGO.loopMove = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (control === 0x4C) {
            engine.setValue(group, "loop_move", -1);
        }
        if (control === 0x4D) {
            engine.setValue(group, "loop_move", 1);
        }
    }
};

//move the beat markers to match the beats with shift + auto loop
DDJERGO.adjustGrid = function(channel, control, value, status, group) {
    let newValue;
    if (value-64 > 0) { newValue = value-128; } else { newValue = value; }
    if (newValue > 0) {
        engine.setValue(group, "beats_translate_later", 1);
    }
    if (newValue < 0) {
        engine.setValue(group, "beats_translate_earlier", 1);
    }
};

//this opens/closes directories when browsing the sidebar and shift + turn the browse wheel
DDJERGO.browseSidebar = function(channel, control, value) {
    let newValue;
    if (value-64 > 0) { newValue = value-128; } else { newValue = value; }
    if (newValue > 0) {
        engine.setParameter("[Library]", "ScrollVertical", newValue);
    }
    if (newValue < 0) {
        engine.setParameter("[Library]", "ScrollVertical", newValue);
    }
};

//this plays the samples on the samplers with a click on the samplers buttons and stores its value for the sample vol knob to manipulate it
DDJERGO.samplers = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        const deck = status-0x90;
        if (engine.getParameter(group, "play") === 1) {
            engine.setParameter(group, "cue_gotoandstop", 1);
        } else {
            engine.setParameter(group, "cue_gotoandplay", 1);
        }
        //store sample numbers
        selectedSamples[0] = (control - 0x3C)/2 + (deck)*4;
        selectedSamples[1] = (control - 0x3C)/2 + (deck)*4;
    }
};

//this modifies the selected sample volume with sample vol knob rotations
DDJERGO.samplerVolume = function(channel, control, value, status) {
    const deck = status-0xB0;
    let newValue;
    if (value-64 > 0) { newValue = value-128; } else { newValue = value; }
    if (newValue > 0) {
        script.triggerControl(samplerStrings[selectedSamples[deck]], "pregain_up");
    }
    if (newValue < 0) {
        script.triggerControl(samplerStrings[selectedSamples[deck]], "pregain_down");
    }
};

//this plays the selected sample with a sample vol knob click
DDJERGO.samplerKnobClick = function(channel, control, value, status) {
    const deck = status-0x90;
    const sample = selectedSamples[deck];
    if (value === 0x7F) {
        if (engine.getParameter(samplerStrings[sample], "play") === 1) {
            engine.setParameter(samplerStrings[sample], "cue_gotoandstop", 1);
        } else {
            engine.setParameter(samplerStrings[sample], "cue_gotoandplay", 1);
        }
    }
};

//this changes the sample stored in the knob being shift + turned
DDJERGO.rotateSamples = function(channel, control, value, status) {
    const deck = status-0xB0;
    let newValue;
    if (value-64 > 0) { newValue = value-128; } else { newValue = value; }
    selectedSamples[deck] += newValue;
    //makes sure samples stay within range
    if (selectedSamples[deck] < 0) {
        selectedSamples[deck] += 8;
    }
    if (selectedSamples[deck] > 7) {
        selectedSamples[deck] -= 8;
    }
};

//this changes both the mic and aux pregains with the aux/mic vol knob
DDJERGO.micAuxGain = function(channel, control, value) {
    //weird non-linear mapping for pregain
    if (value < 64) {
        value = value/63;
    } else {
        value = 3*value/64 - 125/64;
    }
    engine.setValue("[Microphone]", "pregain", value);
    engine.setValue("[Auxiliary1]", "pregain", value);
};

//this toggles on/off effect 1 or 2 based on FX [1-2] buttons clicks
DDJERGO.quickEffects = function(channel, control, value, status) {
    if (value === 0x7F) {
        const deck = control % 2;
        const effect = (control - deck - 0x4C)/4;
        const toggleState = 1 - engine.getParameter(effectsStrings[deck][effect], "enabled");
        engine.setParameter(effectsStrings[deck][effect], "enabled", toggleState);
        //LEDs for FX number button and FX [1-2]
        midi.sendShortMsg(0x94 + deck, 0x47 + effect, 0x7F*toggleState);
        midi.sendShortMsg(status, control, 0x7F*toggleState);
    }
};

//callback for pulse mode intensity calculation on every beat
const pulseModeCallback = function(value, group) {
    //on beat
    if (value === 1) {
        //store timestamps
        if (group === "[Channel1]") {
            timestamps[0] = Date.now();
        } else {
            timestamps[1] = Date.now();
        }
        //calculate delta time in ms and divide it by the ms between beats
        //a 0 delta or a millis delta means maximum beat alignment
        //a 0.5 * millis delta means minimal beat alignment
        const delta = Math.abs(timestamps[0] - timestamps[1]);
        const bpm = engine.getValue(group, "bpm");
        const millis = 1/(bpm/60000);
        const match = -1*Math.abs(delta-(millis/2))/(millis/2) + 1;
        //send pulse signals to the plates
        midi.sendShortMsg(0xBB, 0x14, 0x40-0x40*match);
        midi.sendShortMsg(0xBB, 0x15, 0x40-0x40*match);
    }
};

//callback to turn on/off/blink loop in/out LEDs based on clicks and loop enabled/disabled
const loopLEDsCallback = function(value, group, control) {
    //loop in clicked or autoloop
    if (control === "loop_in" || engine.getParameter(group, "loop_enabled") === 1) {
        loopIn = 1;
    }
    //loop out clicked or autoloop
    if (control === "loop_out" || engine.getParameter(group, "loop_enabled") === 1) {
        loopOut = 1;
    }
    //loop was disabled
    if (control === "loop_enabled" && engine.getParameter(group, "loop_enabled") === 0) {
        loopIn = 0;
        loopOut = 0;
    }
    const channel = group === "[Channel1]" ? 0 : 1;
    //if not looping, LED on, else toggle LED on every beat
    //loop in manipulates blink since loop in blinks while waiting for a loop out click
    if (loopIn === 0) {
        midi.sendShortMsg(0x90+channel, 0x10, 0x7F);
    } else if (control === "beat_active" && value === 1) {
        midi.sendShortMsg(0x90+channel, 0x10, (1-blink[channel])*0x7F);
        blink[channel] = 1 - blink[channel];
    }
    //if not looping, LED on, else toggle LED on every beat
    if (loopOut === 0) {
        midi.sendShortMsg(0x90+channel, 0x11, 0x7F);
    } else if (control === "beat_active" && value === 1) {
        midi.sendShortMsg(0x90+channel, 0x11, blink[channel]*0x7F);
    }
};

//this blinks the faders on every beat and varying intensity based on fader position
const volumeLEDsCallback = function(value, group) {
    const channel = group === "[Channel1]" ? 0 : 1;
    const volume = engine.getParameter(group, "volume");
    //message to set intensity + two on/off messages since it requires them with two different midinos
    midi.sendShortMsg(0xB6, 0x13+2*channel, 0x7F*volume);
    midi.sendShortMsg(0x9B, 0x74+channel, 0x7F*value);
    midi.sendShortMsg(0x9B, 0x78+channel, 0x7F*value);
};

//this triggers an LED animation when the filter knob is turned
const filterLEDsCallback = function(value, group) {
    const channel = group === "[QuickEffectRack1_[Channel1]]" ? 0 : 1;
    midi.sendShortMsg(0xB6, 0x1B+channel, value === 0.5 ? 0x40 : 0x7F*value);
};

//this triggers an LED animation when a track is loaded into the deck
const loadingAnimationCallback = function(value, group) {
    if (engine.getParameter(group, "play") !== 1 && engine.getParameter(group, "track_loaded") === 1) {
        const channel = group === "[Channel1]" ? 0 : 1;
        midi.sendShortMsg(0x9B, 0x0C+channel, 0x7F);
    }
};

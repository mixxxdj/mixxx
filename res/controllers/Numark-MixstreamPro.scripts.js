
// https://github.com/mixxxdj/mixxx/wiki/midi%20scripting
// Example MIDI Msg : status 0x94 (ch 5, opcode 0x9), ctrl 0x03, val 0x01
// Numark-MixstreamPro-scripts by ptrex (aka audministrator) / Daniel Kinahan 

var MixstreamPro = {};

MixstreamPro.settings = {
    stutterPlayOnShiftPlay: false,
    hotCueWhilePlaying: true,
    enableVUMeter: true, // Produces a lot of MIDI traffic that makes it difficult to debug
};

// MIDI constants
const MIDI_STATUS = {
    GENERAL: 0x90,
    DECK1: 0x92,
    DECK2: 0x93,
    FX: 0x94,
    VU_BASE: 0xBF
};

const MIDI_CC = {
    INIT_ALL: 0x75,
    VU_L: 0x20,
    VU_R: 0x21
};

// LED address constants for pad modes
const LED_ADDR = {
    HOTCUE: 0x0B,
    SAVEDLOOP: 0x0C,
    BEATLOOPROLL: 0x0D,
    AUTOLOOP: 0x0E,
    SLIP: 0x23,
    PAD_MODE_1: 15,
    PAD_MODE_2: 16,
    PAD_MODE_3: 17,
    PAD_MODE_4: 18
};

// Pad configurations for each mode (Hotcue, Autoloop, BeatloopRoll)
MixstreamPro.padConfigs = {
    1: { autoloopBank1: 4, autoloopBank2: 0.25, beatloopRollBank1: 0.125, beatloopRollBank2: 0.5, midiLED: 0x0F },
    2: { autoloopBank1: 8, autoloopBank2: 0.5, beatloopRollBank1: 0.1667, beatloopRollBank2: 0.6667, midiLED: 0x10 },
    3: { autoloopBank1: 16, autoloopBank2: 1, beatloopRollBank1: 0.25, beatloopRollBank2: 1, midiLED: 0x11 },
    4: { autoloopBank1: 32, autoloopBank2: 2, beatloopRollBank1: 0.3333, beatloopRollBank2: 2, midiLED: 0x12 }
};

MixstreamPro.pitchRanges = [0.04, 0.08, 0.10, 0.20, 0.5, 1];

// TOGGLE EFFECT buttons - Effect state data
// Blink timer holds the engine timer ID, LEDblink tracks current LED state, midiCC is the CC number for the effect button
MixstreamPro.effectStates = {
    1: { toggle: false, blinktimer: 0, LEDblink: true, midiCC: 0x00 },
    2: { toggle: false, blinktimer: 0, LEDblink: true, midiCC: 0x01 },
    3: { toggle: false, blinktimer: 0, LEDblink: true, midiCC: 0x02 },
    4: { toggle: false, blinktimer: 0, LEDblink: true, midiCC: 0x03 }
}

// Init Hotcue variables - Deck state containers
MixstreamPro.deck = {
    1: {
        //LED blink state for the pad mode toggles and performance pads
        padModeButton: {
            blinktimer: 0,
            LEDblink: true,
        },
        padButton: {
            blinktimer: 0,
            LEDblink: true,
        },
        midiStatus: 0x92,
        channel: "[Channel1]",
        auxChannel: "[Auxiliary2]",
        padModes: {
            hotcue: 0,
            savedloop: 0,
            autoloop: 0,
            roll: 0,
            sampler: 0,
        },
        slipenabledToggle: false,
        effectToggle: false,
        jogWheel: {
            MSB: 0,
            LSB: 0,
            previousValue: 0,
            paused: false,
        },
        pitchSlider: new components.Pot({
            group: '[Channel1]',
            inKey: 'rate'
        }),
    },
    2: {
        padModeButton: {
            blinktimer: 0,
            LEDblink: true,
        },
        padButton: {
            blinktimer: 0,
            LEDblink: true,
        },
        midiStatus: 0x93,
        channel: "[Channel2]",
        auxChannel: "[Auxiliary1]",
        padModes: {
            hotcue: 0,
            savedloop: 0,
            autoloop: 0,
            roll: 0,
            sampler: 0,
        },
        slipenabledToggle: false,
        effectToggle: false,
        jogWheel: {
            MSB: 0,
            LSB: 0,
            previousValue: 0,
            paused: false,
        },
        pitchSlider: new components.Pot({
            group: '[Channel2]',
            inKey: 'rate'
        }),
    }
};

MixstreamPro.init = function (_id, _debugging) {
    // Init Callbacks
    // VuMeters
    engine.makeConnection("[Main]", "vu_meter_left", MixstreamPro.vuCallback);
    engine.makeConnection("[Main]", "vu_meter_right", MixstreamPro.vuCallback);
    // TrackLoaded
    engine.makeConnection("[Channel1]", "track_loaded", MixstreamPro.trackLoadedCallback);
    engine.makeConnection("[Channel2]", "track_loaded", MixstreamPro.trackLoadedCallback);
    // TrackPlaying
    engine.makeConnection("[Channel1]", "play_indicator", MixstreamPro.playIndicatorCallback1);
    engine.makeConnection("[Channel2]", "play_indicator", MixstreamPro.playIndicatorCallback2);

    // Init LEDs
    initLEDs()
}

// Helper utilities for LED and timer safety
function ledOn(status, cc) {
    try { midi.sendShortMsg(status, cc, 0x7f); } catch (e) { }
}

function ledOff(status, cc) {
    try { midi.sendShortMsg(status, cc, 0x00); } catch (e) { }
}

function ledDim(status, cc) {
    try { midi.sendShortMsg(status, cc, 0x01); } catch (e) { }
}
// Pad index helpers: convert pad number (1-4) to CC offset and hotcue/loop index
function padIndexToCC(padNumber) {
    return 14 + padNumber;  // Pads start at CC 15 for pad 1
}

function hotcueIndex(padNumber, bank) {
    return padNumber + (4 * (bank - 1));
}

function initLEDs() {
    // Turn ON all LEDs
    engine.beginTimer(250, function () {
        ledOn(0x90, 0x75);
    }, 1);

    // Turn OFF all LEDs
    engine.beginTimer(500, function () {
        ledOff(0x90, 0x75);
    }, 1);

    // Turn ON some LEDs DECK1
    engine.beginTimer(750, function () {
        for (var cc = 8; cc <= 14; cc++) { ledDim(0x92, cc); }
        ledDim(0x92, 35);
    }, 1);

    // Turn ON some LEDs DECK2
    engine.beginTimer(1000, function () {
        for (var cc = 8; cc <= 14; cc++) { ledDim(0x93, cc); }
        ledDim(0x93, 35);
    }, 1);

    // Turn ON FX LEDs
    engine.beginTimer(1250, function () {
        ledDim(0x90, 13);
        ledDim(0x91, 13);
        ledDim(0x94, 0);
        ledDim(0x94, 1);
        ledDim(0x94, 2);
        ledDim(0x94, 3);
    }, 1);
}

// Turn OFF ALL LEDs at SHUTDOWN
MixstreamPro.shutdown = function () {
    // Stop deck blink timers and turn off LEDs for decks
    Object.keys(MixstreamPro.deck).forEach(function (dn) {
        var deckstate = MixstreamPro.deck[dn];
        if (deckstate && deckstate.padModeButton.blinktimer) {
            engine.stopTimer(deckstate.padModeButton.blinktimer);
            deckstate.padModeButton.blinktimer = 0;
        }
        // try to turn off common pad LEDs for the deck
        try {
            for (var cc = 8; cc <= 14; cc++) { ledOff(ds.midiStatus, cc); }
            ledOff(ds.midiStatus, 35);
        } catch (e) { }
    });

    // Stop effect blink timers and turn off effect LEDs
    Object.keys(MixstreamPro.effectStates).forEach(function (k) {
        var es = MixstreamPro.effectStates[k];
        if (es && es.blinktimer) {
            engine.stopTimer(es.blinktimer);
            es.blinktimer = 0;
        }
        try { ledDim(MIDI_STATUS.FX, es.midiCC); } catch (e) { }
    });

    // Fallback: general LED off
    try { ledOff(MIDI_STATUS.GENERAL, MIDI_CC.INIT_ALL); } catch (e) { }
}

// Init VU Meter variables
MixstreamPro.prevVuLevelL = 0;
MixstreamPro.prevVuLevelR = 0;
MixstreamPro.maxVuLevel = 85

// VuMeter Callback functions
MixstreamPro.vuCallback = function (value, group, control) {
    // Top LED lights up at 0x66
    let level = (value * 70)
    level = Math.ceil(level)
    let enableVUMeter = MixstreamPro.settings.enableVUMeter;

    if (group === '[Main]' && control === 'vu_meter_left') {
        if (enableVUMeter) midi.sendShortMsg(MIDI_STATUS.VU_BASE, MIDI_CC.VU_L, 0x00);
        if (engine.getValue(group, "peak_indicator_left")) {
            level = MixstreamPro.maxVuLevel;
        }

        if (MixstreamPro.prevVuLevelL !== level) {
            if (enableVUMeter) midi.sendShortMsg(MIDI_STATUS.VU_BASE, MIDI_CC.VU_L, level);
            MixstreamPro.prevVuLevelL = level;
        }

    } else if (group === '[Main]' && control === 'vu_meter_right') {
        if (enableVUMeter) midi.sendShortMsg(MIDI_STATUS.VU_BASE, MIDI_CC.VU_R, 0x00);
        if (engine.getValue(group, "peak_indicator_right")) {
            level = MixstreamPro.maxVuLevel
        }

        if (MixstreamPro.prevVuLevelR !== level) {
            if (enableVUMeter) midi.sendShortMsg(MIDI_STATUS.VU_BASE, MIDI_CC.VU_R, level);
            MixstreamPro.prevVuLevelR = level;
        }
    }
}

// Init WIFI button
MixstreamPro.WIFI = true

// TOGGLE WIFI Button (Aux 1 Channel)
MixstreamPro.playAux1 = function(_channel, _control, value, _status, _group) {
    if (value === 127) {
        if (MixstreamPro.WIFI === true) {
            engine.setValue("[Auxiliary1]", "master", 1)
            // Ouput level to 0.1 very low
            engine.setValue("[Auxiliary1]", "pregain", 0.1)

            if (engine.getValue("[Channel1]", "play") === 1) {
                engine.setValue("[Auxiliary1]", "orientation", 2)

            } else if (engine.getValue("[Channel2]", "play") === 1) {
                engine.setValue("[Auxiliary1]", "orientation", 0)
            }

        MixstreamPro.WIFI = false

        } else if (MixstreamPro.WIFI === false) {
            engine.setValue("[Auxiliary1]", "master", 0)
            engine.setValue("[Auxiliary1]", "pregain", 0)

            MixstreamPro.WIFI = true
        }
    }
}

MixstreamPro.shift = false

MixstreamPro.shiftButton = function (_channel, _control, _value, _status, _group) {
    // Set shift explicitly: true while held (value 127), false on release (value 0)
    // MixstreamPro.shift = (value === 127);
    MixstreamPro.shift = !MixstreamPro.shift;
}

// Press and hold Shift and then press this button to “stutter-play” the track from the initial cue point.
MixstreamPro.play = function (_channel, _control, value, _status, group) {
    let deckNum = script.deckFromGroup(group);
    let playStatus = engine.getValue(group, "play_indicator")

    if (value === 0x00) {
        if (MixstreamPro.shift) {
            if (MixstreamPro.settings.stutterPlayOnShiftPlay) {
                engine.setValue(group, "cue_gotoandplay", 1);
            } else {
                // A setting is added here for ptrex preferred behaviour
                playStatus === 1 ? engine.brake(deckNum, true, 0.7) : engine.softStart(deckNum, true, 3);
            }
        } else {
            playStatus === 1 ? engine.setValue(group, "play", 0) : engine.setValue(group, "play", 1);
        }
    }
}

MixstreamPro.playIndicatorCallback1 = function (_channel, _control, _value, _status, _group) {
    engine.setValue("[Auxiliary1]", "orientation", 2)
}

MixstreamPro.playIndicatorCallback2 = function (_channel, _control, _value, _status, _group) {
    engine.setValue("[Auxiliary1]", "orientation", 0)
}

MixstreamPro.jogWheelTicksPerRevolution = 894;

// Generic JogWheel MSB handler for both decks
JogCombined = function (group) {
    let POS = engine.getValue(group, "playposition");
    let deckNumber = script.deckFromGroup(group);
    let deckState = MixstreamPro.deck[deckNumber];
    let value = (deckState.jogWheel.MSB << 7) | deckState.jogWheel.LSB;
    let interval = value - deckState.jogWheel.previousValue;
    
    // If wheel spins past end of track reset it
    const pos = engine.getValue(group, "playposition");
    if (pos <= 0) {
        engine.setValue(group, "playposition", 1);
    } else if (pos >= 1) {
        engine.setValue(group, "playposition", 0);
    }
    
    deckState.jogWheel.previousValue = value;

    if (Math.abs(interval) > 45) {
        return; // Ignore large jumps
    }

    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, interval); // Scratch!
    }
    else // Jog
    {
        engine.setValue(group, "jog", interval);
    }
};

MixstreamPro.JogMSB = function (_channel, _control, value, _status, group) {
    let deckNumber = script.deckFromGroup(group);
    let deckState = MixstreamPro.deck[deckNumber];
    if (deckState.jogWheel.MSB === value) {
        return;
    } else if (deckState.jogWheel.MSB > value) {
        deckState.jogWheel.LSB = 127;
    } else if (deckState.jogWheel.MSB < value) {
        deckState.jogWheel.LSB = 0;
    }
    deckState.jogWheel.MSB = value;
    JogCombined(group);
};

MixstreamPro.JogLSB = function (_channel, _control, value, _status, group) {
    let deckNumber = script.deckFromGroup(group);
    let deckState = MixstreamPro.deck[deckNumber];
    deckState.jogWheel.LSB = value;
    JogCombined(group);
};

MixstreamPro.WheelTouch = function (_channel, _control, value, _status, group) {
    let deckNumber = script.deckFromGroup(group);
    let deckState = MixstreamPro.deck[deckNumber];

    if (deckState.slipenabledToggle) {
        if (value === 0x7F) {    // If WheelTouch
            let alpha = 1.0 / 8;
            let beta = alpha / 32;
            engine.scratchEnable(deckNumber, MixstreamPro.jogWheelTicksPerRevolution, 33 + 1 / 3, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(deckNumber);
        }
    } else {
        if (value === 0x7F && engine.getValue(group, "play")) {
            engine.setValue(group, "play", 0)
            deckState.jogWheel.paused = true;
        } else if (deckState.jogWheel.paused) {
            engine.setValue(group, "play", 1)
            deckState.jogWheel.paused = false;
        }
    }
}

// Generic track loaded callback - works for both decks
MixstreamPro.trackLoadedCallback = function (_value, group, _control) {
    let deckNum = script.deckFromGroup(group);
    if (!deckNum || deckNum < 1 || deckNum > 2) {
        print("Warning: Invalid deck number in trackLoadedCallback: " + deckNum + " from group: " + group);
        return;
    }
    let deckState = MixstreamPro.deck[deckNum];
    let otherDeckNum = deckNum === 1 ? 2 : 1;
    let otherChannel = MixstreamPro.deck[otherDeckNum].channel;

    // AUX channel SPOTIFY Helper
    if (!engine.getValue(group, "track_loaded") && !engine.getValue(otherChannel, "track_loaded")) {
        ledOn(deckState.midiStatus, 0x0E);
        ledOn(deckState.midiStatus, 0x0F);
        ledOn(deckState.midiStatus, 0x10);
        ledOn(deckState.midiStatus, 0x11);
        ledOn(deckState.midiStatus, 0x12);
    } else if (engine.getValue(group, "track_loaded") && !engine.getValue(otherChannel, "play_indicator")) {
        engine.setValue(deckState.auxChannel, "orientation", deckNum === 1 ? 2 : 0);
    } else if (engine.getValue(group, "track_loaded") && engine.getValue(otherChannel, "play_indicator")) {
        engine.setValue(deckState.auxChannel, "orientation", deckNum === 1 ? 0 : 2);
    }

// ptrex
    engine.setValue(group, "loop_remove", true)
    engine.setValue(group, "beatloop_activate", false)
    engine.setValue(group, "beatloop_size", 8)

    ledOff(deckState.midiStatus, 15); 
    ledOff(deckState.midiStatus, 16);
    ledOff(deckState.midiStatus, 17);
    ledOff(deckState.midiStatus, 18);

    // Activate HOTCUE MODE ; see XML values
    if (deckState === 1){
        ledOn(deckState.midiStatus, 11);
        MixstreamPro.togglePadMode(null,LED_ADDR.HOTCUE,0x7f,0x92,group, true)
        MixstreamPro.padModeConfigs.hotcue.onActivate(group, deckState);
    } else {
        ledOn(deckState.midiStatus, 11);
        MixstreamPro.togglePadMode(null,LED_ADDR.HOTCUE,0x7f,0x93,group, true)
        MixstreamPro.padModeConfigs.hotcue.onActivate(group, deckState);
    }

// ptrex

    //ledDim(deckState.midiStatus, 11);
    ledDim(deckState.midiStatus, 12);
    ledDim(deckState.midiStatus, 13);
    ledDim(deckState.midiStatus, 14);

    deckState.jogWheel.previousValue = 0;
}

// Generic slip_enabled_toggle function for both decks
MixstreamPro.toggleScratch = function (_channel, _control, value, status, group) {
    if (value === 0) { return }

    if (value === 127) {
        let deckNum = script.deckFromGroup(group);
        let deckState = MixstreamPro.deck[deckNum];

        if (!deckState.slipenabledToggle) {
            engine.setValue(group, "slip_enabled", true);
            ledOn(status, LED_ADDR.SLIP);
            deckState.slipenabledToggle = true;
        } else {
            engine.setValue(group, "slip_enabled", false);
            ledDim(status, LED_ADDR.SLIP);
            deckState.slipenabledToggle = false;
        }
    }
}

MixstreamPro.pitchBend = function (_channel, control, value, _status, group) {
    if (value === 127) {
        if (MixstreamPro.shift) {
            // Change range of slider
            let rate = engine.getValue(group, "rateRange");
            let index = MixstreamPro.pitchRanges.indexOf(rate);
            if (control === 29) {
                engine.setValue(group, "rateRange", MixstreamPro.pitchRanges[Math.max(0, index - 1)]);
            } else if (control === 30) {
                engine.setValue(group, "rateRange", MixstreamPro.pitchRanges[Math.min(MixstreamPro.pitchRanges.length - 1, index + 1)]);
            }

        } else {
            // Temporary pitch bend
            if (control === 29) {
                engine.setValue(group, "rate_temp_down", 1);
            } else if (control === 30) {
                engine.setValue(group, "rate_temp_up", 1);
            }
        }
    } else {
        // Reset temporary pitch bend
        if (control === 29) {
            engine.setValue(group, "rate_temp_down", 0);
        } else if (control === 30) {
            engine.setValue(group, "rate_temp_up", 0);
        }
    }

}

// Toggle mode configurations: defines which toggle, LED address, and other toggles to reset
MixstreamPro.padModeConfigs = {
    hotcue: {
        ledAddress: LED_ADDR.HOTCUE,
        requiresTrack: true,
        onActivate: function (group, deckState) {
            for (let i = 1; i <= 4; i++) {
                let cueNum = hotcueIndex(i, deckState.padModes.hotcue);
                if (engine.getValue(group, "hotcue_" + cueNum + "_type") === 4) {
                    ledOff(deckState.midiStatus, padIndexToCC(i));
                } else if (engine.getValue(group, "hotcue_" + cueNum + "_status")) {
                    ledOn(deckState.midiStatus, padIndexToCC(i));
                }
            }
        }
    },
    savedloop: {
        ledAddress: LED_ADDR.SAVEDLOOP,
        requiresTrack: true,
        onActivate: function (group, deckState) {
            for (let i = 1; i <= 4; i++) {
                let loopNum = hotcueIndex(i, deckState.padModes.savedloop);
                if (engine.getValue(group, "hotcue_" + loopNum + "_type") === 1) {
                    ledOff(deckState.midiStatus, padIndexToCC(i));
                } else if (engine.getValue(group, "hotcue_" + loopNum + "_status")) {
                    ledOn(deckState.midiStatus, padIndexToCC(i));
                }
            }
        }
    },
    autoloop: {
        ledAddress: LED_ADDR.AUTOLOOP,
        requiresTrack: false,
        onActivate: function (_group, _deckState) { }
    },
    roll: {
        ledAddress: LED_ADDR.BEATLOOPROLL,
        requiresTrack: false,
        onActivate: function (_group, _deckState) { }
    },
    sampler: {
        ledAddress: LED_ADDR.BEATLOOPROLL,
        requiresTrack: false,
        onActivate: function (_group, deckState) {
            let samplerBank = deckState.padModes.sampler;
            for (let i = 1; i <= 4; i++) {
                let sample = i + (4 * (samplerBank - 1));
                if (engine.getValue("[Sampler" + sample + "]", "track_loaded")) {
                    // Turn on LED for loaded sampler
                    ledOn(deckState.midiStatus, padIndexToCC(i));
                }
            }
        }
    }
};

// Generic toggle handler for mode switching
MixstreamPro.togglePadMode = function (_channel, control, value, status, group, onload = false) {
    if (value === 0) { return }

    let configKey;

    switch (control) {
        case 11:
            configKey = "hotcue";
            break;
        case 12:
            configKey = "savedloop";
            break;
        case 13:
            MixstreamPro.shift ? configKey = "sampler" : configKey = "roll";
            break;
        case 14:
            configKey = "autoloop";
            break;
        default:
            return; // Invalid control for this function
    }

    let deckNum = script.deckFromGroup(group);
    let deckState = MixstreamPro.deck[deckNum];
    let padModes = MixstreamPro.deck[deckNum].padModes;
    let config = MixstreamPro.padModeConfigs[configKey];

    if (value === 127 && (!config.requiresTrack || engine.getValue(group, "track_loaded"))) {
        // Dim the pad-mode indicator LEDs before mode change
        ledDim(deckState.midiStatus, LED_ADDR.PAD_MODE_1);
        ledDim(deckState.midiStatus, LED_ADDR.PAD_MODE_2);
        ledDim(deckState.midiStatus, LED_ADDR.PAD_MODE_3);
        ledDim(deckState.midiStatus, LED_ADDR.PAD_MODE_4);

        // Stop any existing pad button blink timer
        engine.stopTimer(deckState.padButton.blinktimer);
        deckState.padButton.blinktimer = 0;

        let currentValue = padModes[configKey];
        let isActive = currentValue !== 0 && currentValue !== false;

        if (!isActive) {
            // Reset other toggles
            Object.keys(padModes).forEach(toggleName => {
                padModes[toggleName] = 0;
            });
        }
        if (currentValue === 0 || currentValue === 2) {
            // Activate this mode, set to 1
            padModes[configKey] = 1;

            // Turn off (dim) other mode LEDs
            Object.keys(MixstreamPro.padModeConfigs).forEach(key => {
                ledDim(status, MixstreamPro.padModeConfigs[key].ledAddress);
            });

            if (deckState.padModeButton.blinktimer) {
                engine.stopTimer(deckState.padModeButton.blinktimer);
                deckState.padModeButton.blinktimer = 0;
            }

            // Turn on the LED for this mode
            ledOn(status, config.ledAddress);
            config.onActivate(group, deckState);
        } else if (currentValue === 1 && onload === false) {
            // Toggle to state 2
            padModes[configKey] = 2;

            // It makes no sense to have the LED tracked in the deckstate for the padmode config but whatever
            ledDim(status, config.ledAddress);
            deckState.padModeButton.blinktimer = engine.beginTimer(500, function () {
                if (deckState.padModeButton.LEDblink) {
                    ledOn(status, config.ledAddress);
                    deckState.padModeButton.LEDblink = false;
                } else {
                    ledDim(status, config.ledAddress);
                    deckState.padModeButton.LEDblink = true;
                }
            });

            config.onActivate(group, deckState);
        }
    }
}

MixstreamPro.performancePad = function (_channel, control, value, status, group) {

    let deckNum = script.deckFromGroup(group);
    let deckState = MixstreamPro.deck[deckNum];
    let PlayStatus = engine.getValue(group, "play_indicator");
    let padNumber = control - 14; // Pads start at control 15 (0x0F), so Pad1=15, Pad2=16, etc.

    let config = MixstreamPro.padConfigs[padNumber];

    // HOTCUE MODE
    if (deckState.padModes.hotcue && (MixstreamPro.settings.hotCueWhilePlaying || !PlayStatus)) {
        let hotcueNum = padNumber + (4 * (deckState.padModes.hotcue - 1));
        // Only set if not a saved loop
        if (engine.getValue(group, "hotcue_" + hotcueNum + "_type") != 4) {
            if (value === 127) {
                if (MixstreamPro.shift) {
                    engine.setValue(group, "hotcue_" + hotcueNum + "_clear", 1);
                    ledDim(deckState.midiStatus, control);
                } else {
                    engine.setValue(group, "hotcue_" + hotcueNum + "_activatecue", 1);
                    ledOn(deckState.midiStatus, control);
                }
            } else if (value === 0) {
                engine.setValue(group, "hotcue_" + hotcueNum + "_activatecue", 0);
            }
        }
    }

    // SAVEDLOOP MODE
    if (deckState.padModes.savedloop) {
        let loopNum = hotcueIndex(padNumber, deckState.padModes.savedloop);
        // Only set if not a hotcue
        if (engine.getValue(group, "hotcue_" + loopNum + "_type") != 1) {
            if (value === 127) {
                if (MixstreamPro.shift) {
                    engine.setValue(group, "hotcue_" + loopNum + "_clear", 1);
                    ledDim(deckState.midiStatus, control);
                } else {
                    if (engine.getValue(group, "hotcue_" + loopNum + "_type")) {
                        engine.setValue(group, "hotcue_" + loopNum + "_activateloop", 1);
                    } else if (!engine.getValue(group, "loop_in")) {
                        engine.setValue(group, "loop_in", 1);
                        // Make the LED flash here
                        deckState.padButton.blinktimer = engine.beginTimer(250, function () {
                            if (deckState.padButton.LEDblink) {
                                ledOn(status, control);
                                deckState.padButton.LEDblink = false;
                            } else {
                                ledDim(status, control);
                                deckState.padButton.LEDblink = true;
                            }
                        });
                    } else {
                        engine.setValue(group, "loop_out", 1);
                        engine.setValue(group, "hotcue_" + loopNum + "_activateloop", 1);
                        engine.setValue(group, "loop_in", 0);
                        // Stop LED flashing and turn LED solid on
                        engine.stopTimer(deckState.padButton.blinktimer);
                        deckState.padButton.blinktimer = 0;
                        ledOn(deckState.midiStatus, control);
                        // Small delay before activating loop to ensure Mixxx has processed loop_out
                        engine.beginTimer(100, function () {
                            engine.setValue(group, "hotcue_" + loopNum + "_activateloop", 1);
                        }, 1);
                    }
                }
            } else if (value === 0) {
                engine.setValue(group, "hotcue_" + loopNum + "_activateloop", 0);
            }
        }
    }
    // AUTOLOOP MODE
    if (value === 127 && deckState.padModes.autoloop) {
        let loopSize = deckState.padModes.autoloop === 1 ? config.autoloopBank1 : config.autoloopBank2;
        engine.setValue(group, "beatloop_" + loopSize + "_toggle", true);
        console.log(engine.getValue(group, "beatloop_" + loopSize + "_active"))

        // Send LED feedback
        for (let i = 1; i <= 4; i++) {
            if (i === padNumber && engine.getValue(group, "beatloop_" + loopSize + "_enabled")) {
                ledOn(status, padIndexToCC(i));
            } else {
                ledDim(status, padIndexToCC(i));
            }
        }
    }

    // BEATLOOPROLL MODE
    if (deckState.padModes.roll) {
        if (value === 127) {
            let loopSize = deckState.padModes.roll === 1 ? config.beatloopRollBank1 : config.beatloopRollBank2;
            engine.setValue(group, "beatloop_size", loopSize);
            if (!engine.getValue(group, "loop_enabled")) {
                engine.setValue(group, "beatlooproll_activate", true);
            }
            // Send LED feedback
            ledOn(status, control);
        } else if (value === 0) {
            engine.setValue(group, "beatlooproll_activate", false);
            ledDim(status, control);
            // Restore slip mode after beatloop roll deactivates (Mixxx disables it automatically)
            if (deckState.slipenabledToggle) {
                engine.beginTimer(10, function () {
                    engine.setValue(group, "slip_enabled", true);
                    ledOn(status, LED_ADDR.SLIP);
                }, 1);
            }
        }
    }

    // SAMPLER MODE
    if (value === 127 && deckState.padModes.sampler) {
        let sample = padNumber + (4 * (deckState.padModes.sampler - 1));
        engine.setValue("[Sampler" + sample + "]", "cue_gotoandplay", 1);
    }
}

MixstreamPro.effectToggleSwitch = function (channel, _control, value, _status, _group) {
    // Get effect number based on channel (assuming 4 effects controlled by this)
    // Check enabled states from effectStates

    // Determine if toggle is ON (value 1 or 2)
    let toggleIsOn = (value === 1 || value === 2);
    let deck = channel - 3;
    MixstreamPro.deck[deck].effectToggle = toggleIsOn;

    if (channel === 4) {
        for (let i = 1; i <= 3; i++) {
            let effectKey = "[EffectRack1_EffectUnit1_Effect" + i + "]";
            let shouldEnable = toggleIsOn && MixstreamPro.effectStates[i].toggle ? 1 : 0;
            engine.setValue(effectKey, "enabled", shouldEnable);
        }
    }

    if (channel === 5) {
        for (let i = 1; i <= 3; i++) {
            let effectKey = "[EffectRack1_EffectUnit2_Effect" + i + "]";
            let shouldEnable = toggleIsOn && MixstreamPro.effectStates[i].toggle ? 1 : 0;
            engine.setValue(effectKey, "enabled", shouldEnable);
        }
    }
}

// Generic Effect button handler - maps CC -> effect and toggles effect enabled + LED
MixstreamPro.effectButton = function (_channel, control, value, _status, _group) {
    if (value === 0) { return }

    let effectNum = control + 1; // CC 0 -> effect 1
    let effectState = MixstreamPro.effectStates[effectNum];

    if (value === 127) {
        // If currently disabled -> enable and start blinking
        if (!effectState.toggle) {
            // Ensure any previous timer is stopped
            if (effectState.blinktimer !== 0) {
                engine.stopTimer(effectState.blinktimer);
                effectState.blinktimer = 0;
            }

            effectState.toggle = true; // now enabled
            effectState.LEDblink = true;
            let effectStateRef = effectState;
            let midiCC = effectState.midiCC;
            effectState.blinktimer = engine.beginTimer(500, function () {
                if (effectStateRef.LEDblink) {
                    ledOn(MIDI_STATUS.FX, midiCC);
                    effectStateRef.LEDblink = false;
                } else {
                    ledDim(MIDI_STATUS.FX, midiCC);
                    effectStateRef.LEDblink = true;
                }
            });
        } else {
            // Currently enabled -> disable and stop blinking
            effectState.toggle = false;

            if (effectState.blinktimer !== 0) {
                engine.stopTimer(effectState.blinktimer);
                effectState.blinktimer = 0;
            }

            // Ensure LED shows off state
            ledDim(MIDI_STATUS.FX, effectState.midiCC);
        }

        // After toggling the effect button, trigger the toggle switches to re-evaluate
        // This ensures the effect is applied/removed based on current toggle switch state
        let deck1Value = MixstreamPro.deck[1].effectToggle ? 1 : 0;
        let deck2Value = MixstreamPro.deck[2].effectToggle ? 1 : 0;
        MixstreamPro.effectToggleSwitch(4, null, deck1Value, null, null);
        MixstreamPro.effectToggleSwitch(5, null, deck2Value, null, null);
    }
}

const SETTINGS = Object.freeze({
    // Stutter on shift + play, turning this to false makes shift+play do a slow start effect
    shiftPlayBehaviour: engine.getSetting("shiftPlayBehaviour"),
    // Enable pressing hotcues while the track is moving
    hotCueWhilePlaying: engine.getSetting("hotCueWhilePlaying"),
    // Default pad mode to load tracks with: hotcue, savedloop, autoloop, roll, empty (keeps current engaged mode)
    defaultPadMode: engine.getSetting("defaultPadMode"),
    // Produces a lot of MIDI traffic that makes it difficult to debug
    enableVUMeter: true,
    // Enable backspinning (or forward spinning) when you lift your finger off the platter
    // Not compatible with Mixxx versions below 2.6
    backspin: engine.getSetting("backspin"),
    // Amount of values to smooth jog speed over. Raising this makes it lag behind
    //  your movement and lowering it makes it sound warbly
    jogWheelBufferSize: engine.getSetting("jogWheelBufferSize"),
    // Speed at which the scratching will turn off. This can make the track stuck in a speed if too low
    jogWheelThreshold: engine.getSetting("jogWheelThreshold"),
    // If the jog wheel speed gets stuck, this will reset it every n milliseconds
    jogWheelStuckTimeout: engine.getSetting("jogWheelStuckTimeout"),
});

// Pad configurations for each mode (Hotcue, Autoloop, BeatloopRoll)
const PAD_CONFIGS = Object.freeze({
    1: {autoloopBank1: 4, autoloopBank2: 0.25, beatloopRollBank1: 0.125, beatloopRollBank2: 0.5, midiLED: 0x0F},
    2: {autoloopBank1: 8, autoloopBank2: 0.5, beatloopRollBank1: 0.1667, beatloopRollBank2: 0.6667, midiLED: 0x10},
    3: {autoloopBank1: 16, autoloopBank2: 1, beatloopRollBank1: 0.25, beatloopRollBank2: 1, midiLED: 0x11},
    4: {autoloopBank1: 32, autoloopBank2: 2, beatloopRollBank1: 0.3333, beatloopRollBank2: 2, midiLED: 0x12}
});

// Pitch ranges that can be set using Shift + Pitch Bend
const PITCH_RANGES = Object.freeze([0.04, 0.08, 0.10, 0.20, 0.5, 1]);

// MIDI constants
const MIDI_STATUS = Object.freeze({
    GENERAL: 0x90,
    DECK1: 0x92,
    DECK2: 0x93,
    FX: 0x94,
    VU_BASE: 0xBF
});

const MIDI_CC = Object.freeze({
    INIT_ALL: 0x75,
    VU_L: 0x20,
    VU_R: 0x21
});

// LED address constants for pad modes
const LED_ADDR = Object.freeze({
    HOTCUE: 0x0B,
    SAVEDLOOP: 0x0C,
    BEATLOOPROLL: 0x0D,
    AUTOLOOP: 0x0E,
    SLIP: 0x23,
    PAD_MODE_1: 15,
    PAD_MODE_2: 16,
    PAD_MODE_3: 17,
    PAD_MODE_4: 18
});

const SCRATCH_MODE = Object.freeze({
    jog: 0,
    vinyl: 1,
    smart: 2
});

// Helper utilities for LED and timer safety
const ledOn = function(status, cc) {
    midi.sendShortMsg(status, cc, 0x7f);
};

const ledOff = function(status, cc) {
    midi.sendShortMsg(status, cc, 0x00);
};

const ledDim = function(status, cc) {
    midi.sendShortMsg(status, cc, 0x01);
};

// Pad index helpers: convert pad number (1-4) to CC offset and hotcue/loop index
const padIndexToCC = function(padNumber) {
    return 14 + padNumber;  // Pads start at CC 15 for pad 1
};

const padIndex = function(padNumber, bank, type) {
    if (type === "hotcue") {
        // Hotcues use 1-4, 9-13
        return padNumber + ((bank - 1) * 8);
    } else if (type === "savedloop") {
        // Saved loops use 5-8, 14-17
        return padNumber + 4 + ((bank - 1) * 8);
    } else if (type === "sampler") {
        return padNumber + (4 * (bank - 1));
    }
};

const initLEDs = function() {
    // Turn ON all LEDs
    engine.beginTimer(250, function() {
        ledOn(0x90, 0x75);
    }, 1);

    // Turn OFF all LEDs
    engine.beginTimer(500, function() {
        ledOff(0x90, 0x75);
    }, 1);

    // Turn ON some LEDs DECK1
    engine.beginTimer(750, function() {
        for (let cc = 8; cc <= 14; cc++) { ledDim(0x92, cc); }
        ledDim(0x92, 35);
    }, 1);

    // Turn ON some LEDs DECK2
    engine.beginTimer(1000, function() {
        for (let cc = 8; cc <= 14; cc++) { ledDim(0x93, cc); }
        ledDim(0x93, 35);
    }, 1);

    // Turn ON FX LEDs
    engine.beginTimer(1250, function() {
        ledDim(0x90, 13);
        ledDim(0x91, 13);
        ledDim(0x94, 0);
        ledDim(0x94, 1);
        ledDim(0x94, 2);
        ledDim(0x94, 3);
    }, 1);
};

const clearPadMode = function(padModes) {
    Object.keys(padModes).forEach(toggleName => {
        padModes[toggleName] = 0;
    });
};

// Generic toggle handler for mode switching
const togglePadMode = function(status, group, configKey) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
    const config = MixstreamPro.padModeConfigs[configKey];
    const padModes = deckState.padModes;

    if ((!config.requiresTrack || engine.getValue(group, "track_loaded"))) {
        // Dim the pad-mode indicator LEDs before mode change
        ledDim(deckState.midiStatus, LED_ADDR.PAD_MODE_1);
        ledDim(deckState.midiStatus, LED_ADDR.PAD_MODE_2);
        ledDim(deckState.midiStatus, LED_ADDR.PAD_MODE_3);
        ledDim(deckState.midiStatus, LED_ADDR.PAD_MODE_4);

        // Stop any existing pad button blink timer
        engine.stopTimer(deckState.padButton.blinktimer);
        deckState.padButton.blinktimer = 0;

        if (padModes[configKey] === 0 || padModes[configKey] === 2) {
            clearPadMode(padModes);
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
        } else if (padModes[configKey] === 1) {
            // Toggle to state 2
            padModes[configKey] = 2;

            // It makes no sense to have the LED tracked in the deckstate for the padmode config but whatever
            ledDim(status, config.ledAddress);
            deckState.padModeButton.blinktimer = engine.beginTimer(500, function() {
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
};

var MixstreamPro = {};

// TOGGLE EFFECT buttons - Effect state data
// Blink timer holds the engine timer ID, LEDblink tracks current LED state, midiCC is the CC number for the effect button
MixstreamPro.effectStates = {
    1: {toggle: false, blinktimer: 0, LEDblink: true, midiCC: 0x00},
    2: {toggle: false, blinktimer: 0, LEDblink: true, midiCC: 0x01},
    3: {toggle: false, blinktimer: 0, LEDblink: true, midiCC: 0x02},
    4: {toggle: false, blinktimer: 0, LEDblink: true, midiCC: 0x03}
};

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
        shift: false,
        padModes: {
            hotcue: 0,
            savedloop: 0,
            autoloop: 0,
            roll: 0,
            sampler: 0,
        },
        effectToggle: false,
        jogWheel: {
            MSB: 0,
            LSB: 0,
            previousValue: 0,
            previousTimestamp: 0,
            buffer: [],
            paused: false,
            touched: false,
            speed: 0,
            previousSpeed: 0,
            scratchMode: SCRATCH_MODE.vinyl,
        },
        pitchSlider: new components.Pot({
            group: "[Channel1]",
            inKey: "rate"
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
        shift: false,
        padModes: {
            hotcue: 0,
            savedloop: 0,
            autoloop: 0,
            roll: 0,
            sampler: 0,
        },
        effectToggle: false,
        jogWheel: {
            MSB: 0,
            LSB: 0,
            previousValue: 0,
            buffer: [],
            previousTimestamp: 0,
            paused: false,
            touched: false,
            speed: 0,
            previousSpeed: 0,
            scratchMode: SCRATCH_MODE.vinyl,
        },
        pitchSlider: new components.Pot({
            group: "[Channel2]",
            inKey: "rate"
        }),
    }
};

// Toggle mode configurations: defines which toggle, LED address, and other toggles to reset
MixstreamPro.padModeConfigs = {
    hotcue: {
        ledAddress: LED_ADDR.HOTCUE,
        requiresTrack: true,
        onActivate: function(group, deckState) {
            for (let i = 1; i <= 4; i++) {
                const cueNum = padIndex(i, deckState.padModes.hotcue, "hotcue");
                if (engine.getValue(group, `hotcue_${cueNum}_type`) === 4) {
                    ledOff(deckState.midiStatus, padIndexToCC(i));
                } else if (engine.getValue(group, `hotcue_${cueNum}_status`)) {
                    ledOn(deckState.midiStatus, padIndexToCC(i));
                }
            }
        }
    },
    savedloop: {
        ledAddress: LED_ADDR.SAVEDLOOP,
        requiresTrack: true,
        onActivate: function(group, deckState) {
            for (let i = 1; i <= 4; i++) {
                const loopNum = padIndex(i, deckState.padModes.savedloop, "savedloop");
                if (engine.getValue(group, `hotcue_${loopNum}_type`) === 1) {
                    ledOff(deckState.midiStatus, padIndexToCC(i));
                } else if (engine.getValue(group, `hotcue_${loopNum}_status`)) {
                    ledOn(deckState.midiStatus, padIndexToCC(i));
                }
            }
        }
    },
    autoloop: {
        ledAddress: LED_ADDR.AUTOLOOP,
        requiresTrack: false,
        onActivate: function(_group, _deckState) { }
    },
    roll: {
        ledAddress: LED_ADDR.BEATLOOPROLL,
        requiresTrack: false,
        onActivate: function(_group, _deckState) { }
    },
    sampler: {
        ledAddress: LED_ADDR.BEATLOOPROLL,
        requiresTrack: false,
        onActivate: function(_group, deckState) {
            for (let i = 1; i <= 4; i++) {
                const sample = padIndex(i, deckState.padModes.sampler, "sampler");
                if (engine.getValue(`[Sampler${sample}]`, "track_loaded")) {
                    // Turn on LED for loaded sampler
                    ledOn(deckState.midiStatus, padIndexToCC(i));
                }
            }
        }
    }
};


// Init VU Meter variables
MixstreamPro.prevVuLevelL = 0;
MixstreamPro.prevVuLevelR = 0;
MixstreamPro.maxVuLevel = 85;

MixstreamPro.WIFI = true;

MixstreamPro.init = function(_id, _debugging) {
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
    initLEDs();

    // Unsticks the scratch every so often
    if (SETTINGS.backspin) {
        engine.beginTimer(SETTINGS.jogWheelStuckTimeout, function() {
            for (const deck in MixstreamPro.deck) {
                const group = `[Channel${deck}]`;
                const jogWheel = MixstreamPro.deck[deck].jogWheel;
                if (engine.getValue(group, "scratch2_enable") &&
                    jogWheel.speed === jogWheel.previousSpeed &&
                    !engine.isBrakeActive(deck) &&
                    !engine.isSoftStartActive(deck)) {
                    engine.setValue(group, "scratch2", 0);
                    engine.setValue(group, "scratch2_enable", false);
                }
                jogWheel.previousSpeed = jogWheel.speed;
            }
        }, false);
    }
};

// Turn OFF ALL LEDs at SHUTDOWN
MixstreamPro.shutdown = function() {
    // Stop deck blink timers and turn off LEDs for decks
    Object.keys(MixstreamPro.deck).forEach(function(dn) {
        const deckstate = MixstreamPro.deck[dn];
        if (deckstate && deckstate.padModeButton.blinktimer) {
            engine.stopTimer(deckstate.padModeButton.blinktimer);
            deckstate.padModeButton.blinktimer = 0;
        }
        // try to turn off common pad LEDs for the deck
        try {
            for (let cc = 8; cc <= 14; cc++) { ledOff(deckstate.midiStatus, cc); }
            ledOff(deckstate.midiStatus, 35);
        } catch (e) {
            console.info(`Error: ${e}`);
        }
    });

    // Stop effect blink timers and turn off effect LEDs
    Object.keys(MixstreamPro.effectStates).forEach(function(k) {
        const es = MixstreamPro.effectStates[k];
        if (es && es.blinktimer) {
            engine.stopTimer(es.blinktimer);
            es.blinktimer = 0;
        }
        try { ledDim(MIDI_STATUS.FX, es.midiCC); } catch (e) { console.info(`Error: ${e}`); }
    });

    // Fallback: general LED off
    try { ledOff(MIDI_STATUS.GENERAL, MIDI_CC.INIT_ALL); } catch (e) { console.info(`Error: ${e}`); }
};

// VuMeter Callback functions
MixstreamPro.vuCallback = function(value, group, control) {
    // Top LED lights up at 0x66
    let level = (value * 70);
    level = Math.ceil(level);

    if (group === "[Main]" && control === "vu_meter_left") {
        if (SETTINGS.enableVUMeter) { midi.sendShortMsg(MIDI_STATUS.VU_BASE, MIDI_CC.VU_L, 0x00); }
        if (engine.getValue(group, "peak_indicator_left")) {
            level = MixstreamPro.maxVuLevel;
        }

        if (MixstreamPro.prevVuLevelL !== level) {
            if (SETTINGS.enableVUMeter) { midi.sendShortMsg(MIDI_STATUS.VU_BASE, MIDI_CC.VU_L, level); }
            MixstreamPro.prevVuLevelL = level;
        }

    } else if (group === "[Main]" && control === "vu_meter_right") {
        if (SETTINGS.enableVUMeter) { midi.sendShortMsg(MIDI_STATUS.VU_BASE, MIDI_CC.VU_R, 0x00); }
        if (engine.getValue(group, "peak_indicator_right")) {
            level = MixstreamPro.maxVuLevel;
        }

        if (MixstreamPro.prevVuLevelR !== level) {
            if (SETTINGS.enableVUMeter) { midi.sendShortMsg(MIDI_STATUS.VU_BASE, MIDI_CC.VU_R, level); }
            MixstreamPro.prevVuLevelR = level;
        }
    }
};

MixstreamPro.loadTrack = function(_channel, _control, value, _status, group) {
    if (value === 0) { return; }

    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
    if (deckState.shift) {
        engine.setValue(group, "eject", 1);
    } else {
        engine.setValue(group, "LoadSelectedTrack", 1);
    }
};

// Generic track loaded callback - works for both decks
MixstreamPro.trackLoadedCallback = function(_value, group, _control) {
    const deckNum = script.deckFromGroup(group);
    if (!deckNum || deckNum < 1 || deckNum > 2) {
        console.log(`Warning: Invalid deck number in trackLoadedCallback: ${deckNum} from group: ${group}`);
        return;
    }
    const deckState = MixstreamPro.deck[deckNum];
    const otherDeckNum = deckNum === 1 ? 2 : 1;
    const otherChannel = MixstreamPro.deck[otherDeckNum].channel;

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

    ledDim(deckState.midiStatus, 11);
    ledDim(deckState.midiStatus, 12);
    ledDim(deckState.midiStatus, 13);
    ledDim(deckState.midiStatus, 14);

    deckState.jogWheel.previousValue = 0;

    engine.setValue(group, "loop_remove", true);
    engine.setValue(group, "beatloop_activate", false);
    engine.setValue(group, "beatloop_size", 8);

    if (SETTINGS.defaultPadMode) {
        clearPadMode(deckState.padModes);
        engine.beginTimer(400, function() {
            togglePadMode(deckState.midiStatus, group, SETTINGS.defaultPadMode);
        }, true);
    }
};

MixstreamPro.playIndicatorCallback1 = function(_channel, _control, _value, _status, _group) {
    engine.setValue("[Auxiliary1]", "orientation", 2);
};

MixstreamPro.playIndicatorCallback2 = function(_channel, _control, _value, _status, _group) {
    engine.setValue("[Auxiliary1]", "orientation", 0);
};

// TOGGLE AUX 1 Button
MixstreamPro.playAux1 = function(_channel, _control, value, _status, _group) {
    if (value === 127) {
        if (MixstreamPro.WIFI === true) {
            engine.setValue("[Auxiliary1]", "master", 1);
            engine.setValue("[Auxiliary1]", "pregain", 0.1);

            if (engine.getValue("[Channel1]", "play_indicator") === 1) {
                engine.setValue("[Auxiliary1]", "orientation", 2);

            } else
                if (engine.getValue("[Channel2]", "play_indicator") === 1) {
                    engine.setValue("[Auxiliary1]", "orientation", 0);
                }

            MixstreamPro.WIFI = false;
        } else
            if (MixstreamPro.WIFI === false) {
                engine.setValue("[Auxiliary1]", "master", 0);
                engine.setValue("[Auxiliary1]", "pregain", 0);
                MixstreamPro.WIFI = true;
            }
    }
};

MixstreamPro.shiftButton = function(_channel, _control, value, _status, group) {
    // Set shift explicitly: true while held (value 127), false on release (value 0)
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
    deckState.shift = (value === 127);
};

// Press and hold Shift and then press this button to “stutter-play” the track from the initial cue point.
MixstreamPro.play = function(_channel, _control, value, _status, group) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
    const playStatus = engine.getValue(group, "play_indicator");

    if (value === 127) {
        if (deckState.shift) {
            if (SETTINGS.shiftPlayBehaviour === "stutter") {
                engine.setValue(group, "cue_gotoandplay", 1);
            } else {
                if (playStatus === 1) {
                    engine.brake(deckNum, true, 0.7);
                } else {
                    engine.softStart(deckNum, true, 3);
                }
            }
        } else {
            script.toggleControl(group, "play");
        }
    }
};

MixstreamPro.cue = function(_channel, _control, value, _status, group) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];

    if (deckState.shift) {
        engine.setValue(group, "cue_gotoandstop", 1);
    } else {
        engine.setValue(group, "cue_default", (value === 127));
    }
};

// Generic JogWheel MSB handler for both decks
const JogCombined = function(group) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
    const value = (deckState.jogWheel.MSB << 7) | deckState.jogWheel.LSB;
    const timestamp = Date.now();

    const interval = value - deckState.jogWheel.previousValue;
    const dt = timestamp - deckState.jogWheel.previousTimestamp;

    // If wheel spins past end of track reset it
    const pos = engine.getValue(group, "playposition");
    if (pos <= 0) {
        engine.setValue(group, "playposition", 1);
    } else if (pos >= 1) {
        engine.setValue(group, "playposition", 0);
    }

    deckState.jogWheel.previousValue = value;
    deckState.jogWheel.previousTimestamp = timestamp;

    if (Math.abs(interval) > 45 || dt <= 0 || dt > 1000) {
        if (dt > 1000) {
            deckState.jogWheel.buffer = [];
        }
        return; // Ignore large jumps
    }

    const speed = interval / dt * 10;

    // Smooth over speed to reduce warbling effect
    const buffer = deckState.jogWheel.buffer;
    buffer.push(speed);
    if (buffer.length > SETTINGS.jogWheelBufferSize) {
        buffer.shift();
    }

    const smoothedSpeed = buffer.length > 0 ?
        buffer.slice().sort((a, b) => a - b)[Math.floor(buffer.length / 2)] :
        speed;

    // Keep scratching until the wheel has stopped spinning
    if (SETTINGS.backspin && !deckState.jogWheel.touched && Math.abs(speed) <= SETTINGS.jogWheelThreshold) {
        engine.setValue(group, "scratch2", 0);
        engine.setValue(group, "scratch2_enable", false);

        if (deckState.jogWheel.paused) {
            engine.setValue(group, "play", 1);
            deckState.jogWheel.paused = false;
        }
        if (deckState.jogWheel.scratchMode === SCRATCH_MODE.smart) {
            // It doesn't work to just flip this, there needs to be a delay before its turned back on
            engine.setValue(group, "slip_enabled", false);
            engine.beginTimer(400, function() {
                engine.setValue(group, "slip_enabled", true);
            }, true);
        }
        return;
    }

    if (engine.isScratching(deckNum)) {
        // If you uncomment this, apply the buffer over interval to smooth out weird outlier values
        // engine.scratchTick(deckNum, interval);
        engine.setValue(group, "scratch2", smoothedSpeed);
    } else {
        engine.setValue(group, "jog", interval);
    }
    deckState.jogWheel.speed = smoothedSpeed;
};

MixstreamPro.JogMSB = function(_channel, _control, value, _status, group) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
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

MixstreamPro.JogLSB = function(_channel, _control, value, _status, group) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
    deckState.jogWheel.LSB = value;
    JogCombined(group);
};

MixstreamPro.WheelTouch = function(_channel, _control, value, _status, group) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];

    if (value === 0x7F) {
        // If scratchMode not in jog mode
        if (deckState.jogWheel.scratchMode) {
            engine.setValue(group, "scratch2_enable", true);
            deckState.jogWheel.touched = true;
            if (engine.getValue(group, "play")) {
                engine.setValue(group, "play", 0);
                deckState.jogWheel.paused = true;
            }
        }
    } else {
        deckState.jogWheel.touched = false;
        if (SETTINGS.backspin) {
            if (Math.abs(deckState.jogWheel.speed) <= SETTINGS.jogWheelThreshold) {
                engine.setValue(group, "scratch2", 0);
                engine.setValue(group, "scratch2_enable", false);
            }
        } else {
            engine.setValue(group, "scratch2", 0);
            engine.setValue(group, "scratch2_enable", false);
        }

        if (deckState.jogWheel.paused) {
            engine.setValue(group, "play", 1);
            deckState.jogWheel.paused = false;
        }
    }
};

// Generic slip_enabled_toggle function for both decks
MixstreamPro.toggleScratch = function(_channel, _control, value, status, group) {
    if (value === 0) { return; }

    if (value === 127) {
        const deckNum = script.deckFromGroup(group);
        const deckState = MixstreamPro.deck[deckNum];

        if (deckState.shift) {
            if (deckState.jogWheel.scratchMode === SCRATCH_MODE.jog) {
                deckState.jogWheel.scratchMode = SCRATCH_MODE.vinyl;
                ledDim(status, LED_ADDR.SLIP);
            } else {
                deckState.jogWheel.scratchMode = SCRATCH_MODE.jog;
                ledOff(status, LED_ADDR.SLIP);
            }
        } else {
            if (deckState.jogWheel.scratchMode === SCRATCH_MODE.vinyl) {
                engine.setValue(group, "slip_enabled", true);
                deckState.jogWheel.scratchMode = SCRATCH_MODE.smart;
                ledOn(status, LED_ADDR.SLIP);
            } else if (deckState.jogWheel.scratchMode === SCRATCH_MODE.smart) {
                engine.setValue(group, "slip_enabled", false);
                deckState.jogWheel.scratchMode = SCRATCH_MODE.vinyl;
                ledDim(status, LED_ADDR.SLIP);
            }
        }
    }
};

MixstreamPro.pitchBend = function(_channel, control, value, _status, group) {
    if (value === 127) {
        const deckNum = script.deckFromGroup(group);
        const deckState = MixstreamPro.deck[deckNum];
        if (deckState.shift) {
            // Change range of slider
            const rate = engine.getValue(group, "rateRange");
            const index = PITCH_RANGES.indexOf(rate);
            if (control === 29) {
                engine.setValue(group, "rateRange", PITCH_RANGES[Math.max(0, index - 1)]);
            } else if (control === 30) {
                engine.setValue(group, "rateRange", PITCH_RANGES[Math.min(PITCH_RANGES.length - 1, index + 1)]);
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

};

// Mode toggle wrappers for each control
MixstreamPro.toggleHotCueOrStems = function(_channel, _control, value, status, group) {
    if (value === 127) { togglePadMode(status, group, "hotcue"); }
};

MixstreamPro.toggleSavedLoop = function(_channel, _control, value, status, group) {
    if (value === 127) { togglePadMode(status, group, "savedloop"); }
};

MixstreamPro.toggleAutoloop = function(_channel, _control, value, status, group) {
    if (value === 127) { togglePadMode(status, group, "autoloop"); }
};

MixstreamPro.toggleRollOrSampler = function(_channel, _control, value, status, group) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
    const padMode = deckState.shift ? "sampler" : "roll";

    if (value === 127) { togglePadMode(status, group, padMode); }
};

MixstreamPro.performancePad = function(_channel, control, value, status, group) {
    const deckNum = script.deckFromGroup(group);
    const deckState = MixstreamPro.deck[deckNum];
    const PlayStatus = engine.getValue(group, "play_indicator");
    const padNumber = control - 14; // Pads start at control 15 (0x0F), so Pad1=15, Pad2=16, etc.

    const config = PAD_CONFIGS[padNumber];

    // HOTCUE MODE
    if (deckState.padModes.hotcue && (SETTINGS.hotCueWhilePlaying || !PlayStatus)) {
        const hotcueNum = padIndex(padNumber, deckState.padModes.hotcue, "hotcue");
        // Only set if not a saved loop
        if (engine.getValue(group, `hotcue_${hotcueNum}_type`) !== 4) {
            if (value === 127) {
                if (deckState.shift) {
                    engine.setValue(group, `hotcue_${hotcueNum}_clear`, 1);
                    ledDim(deckState.midiStatus, control);
                } else {
                    engine.setValue(group, `hotcue_${hotcueNum}_activatecue`, 1);
                    ledOn(deckState.midiStatus, control);
                }
            } else if (value === 0) {
                engine.setValue(group, `hotcue_${hotcueNum}_activatecue`, 0);
            }
        }
    }

    // SAVEDLOOP MODE
    if (deckState.padModes.savedloop) {
        const loopNum = padIndex(padNumber, deckState.padModes.savedloop, "savedloop");
        // Only set if not a hotcue
        if (engine.getValue(group, `hotcue_${loopNum}_type`) !== 1) {
            if (value === 127) {
                if (deckState.shift) {
                    engine.setValue(group, `hotcue_${loopNum}_clear`, 1);
                    ledDim(deckState.midiStatus, control);
                } else {
                    if (engine.getValue(group, `hotcue_${loopNum}_type`)) {
                        engine.setValue(group, `hotcue_${loopNum}_activateloop`, 1);
                    } else if (!engine.getValue(group, "loop_in")) {
                        engine.setValue(group, "loop_in", 1);
                        // Make the LED flash here
                        deckState.padButton.blinktimer = engine.beginTimer(250, function() {
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
                        engine.setValue(group, `hotcue_${loopNum}_activateloop`, 1);
                        engine.setValue(group, "loop_in", 0);
                        // Stop LED flashing and turn LED solid on
                        engine.stopTimer(deckState.padButton.blinktimer);
                        deckState.padButton.blinktimer = 0;
                        ledOn(deckState.midiStatus, control);
                        // Small delay before activating loop to ensure Mixxx has processed loop_out
                        engine.beginTimer(100, function() {
                            engine.setValue(group, `hotcue_${loopNum}_activateloop`, 1);
                        }, 1);
                    }
                }
            } else if (value === 0) {
                engine.setValue(group, `hotcue_${loopNum}_activateloop`, 0);
            }
        }
    }
    // AUTOLOOP MODE
    if (value === 127 && deckState.padModes.autoloop) {
        const loopSize = deckState.padModes.autoloop === 1 ? config.autoloopBank1 : config.autoloopBank2;
        engine.setValue(group, `beatloop_${loopSize}_toggle`, true);

        // Send LED feedback
        for (let i = 1; i <= 4; i++) {
            if (i === padNumber && engine.getValue(group, `beatloop_${loopSize}_enabled`)) {
                ledOn(status, padIndexToCC(i));
            } else {
                ledDim(status, padIndexToCC(i));
            }
        }
    }

    // BEATLOOPROLL MODE
    if (deckState.padModes.roll) {
        if (value === 127) {
            const loopSize = deckState.padModes.roll === 1 ? config.beatloopRollBank1 : config.beatloopRollBank2;
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
            if (deckState.jogWheel.scratchMode === SCRATCH_MODE.smart) {
                engine.beginTimer(10, function() {
                    engine.setValue(group, "slip_enabled", true);
                    ledOn(status, LED_ADDR.SLIP);
                }, 1);
            }
        }
    }

    // SAMPLER MODE
    if (value === 127 && deckState.padModes.sampler) {
        const sample = padIndex(padNumber, deckState.padModes.sampler, "sampler");
        if (deckState.shift) {
            engine.setValue(`[Sampler${sample}]`, "stop", 1);
        } else {
            engine.setValue(`[Sampler${sample}]`, "cue_gotoandplay", 1);
        }
    }
};

MixstreamPro.effectToggleSwitch = function(channel, __control, value, _status, _group) {
    // Get effect number based on channel (assuming 4 effects controlled by this)
    // Check enabled states from effectStates

    // Determine if toggle is ON (value 1 or 2)
    const toggleIsOn = (value === 1 || value === 2);
    const deck = channel - 3;
    MixstreamPro.deck[deck].effectToggle = toggleIsOn;

    if (channel === 4) {
        for (let i = 1; i <= 3; i++) {
            const effectKey = `[EffectRack1_EffectUnit1_Effect${i}]`;
            const shouldEnable = toggleIsOn && MixstreamPro.effectStates[i].toggle ? 1 : 0;
            engine.setValue(effectKey, "enabled", shouldEnable);
        }
    }

    if (channel === 5) {
        for (let i = 1; i <= 3; i++) {
            const effectKey = `[EffectRack1_EffectUnit2_Effect${i}]`;
            const shouldEnable = toggleIsOn && MixstreamPro.effectStates[i].toggle ? 1 : 0;
            engine.setValue(effectKey, "enabled", shouldEnable);
        }
    }
};

// Generic Effect button handler - maps CC -> effect and toggles effect enabled + LED
MixstreamPro.effectButton = function(_channel, control, value, _status, _group) {
    if (value === 0) { return; }

    const effectNum = control + 1; // CC 0 -> effect 1
    const effectState = MixstreamPro.effectStates[effectNum];

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
            const effectStateRef = effectState;
            const midiCC = effectState.midiCC;
            effectState.blinktimer = engine.beginTimer(500, function() {
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
        const deck1Value = MixstreamPro.deck[1].effectToggle ? 1 : 0;
        const deck2Value = MixstreamPro.deck[2].effectToggle ? 1 : 0;
        MixstreamPro.effectToggleSwitch(4, null, deck1Value, null, null);
        MixstreamPro.effectToggleSwitch(5, null, deck2Value, null, null);
    }
};

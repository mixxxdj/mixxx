// DJControl_Inpulse_300_script.js
//
// ***************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Inpulse 300.
// * Author: DJ Phatso, contributions by Kerrick Staley and BoredGuy1
// * Version 1.3 (May 2024)
// * Forum: https://www.mixxx.org/forums/viewtopic.php?f=7&t=12599
// * Wiki: https://mixxx.org/wiki/doku.php/hercules_djcontrol_inpulse_300
//
// Changes to v1.3
// - Added ability to stop samplers (shift + button)
// - Added toneplay
// - Added shift + toneplay controls
// - Added slicer/slicer loop
// - Replaced the song end warning with an actual beatmatch guide
// - Changed the way scratching works (wheels have inertia, allowing backspins and other tricks)
// - Updated VU meters (replaced vu_meter with VuMeter, connectControl with makeConnection, etc)
//
// Changes to v1.2
// - Code cleanup.
//
// Changes to v1.1
// - Fix seek-to-start and cue-master behavior.
// - Tweak scratch, seek, and bend behavior.
// - Controller knob/slider values are queried on startup, so MIXXX is synced.
// - Fixed vinyl button behavior the first time it's pressed.
//
// v1.0 : Original forum release
//
// TO DO: Functions that could be implemented to the script:
//
// * ROLL: Keep SLIP active (if already enabled) when exiting from rolls
//
// * FX: See how to preselect effects for a rack
//
// ****************************************************************************
var DJCi300 = {};
///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// Beatmatch guide LED tolerances
DJCi300.beatmatchTempoTolerance = .1;
DJCi300.beatmatchAlignTolerance = .01;

// Determines how fast the wheel must be moving to be considered "slipping"
DJCi300.slipThreshold = 0.1;

// How fast scratching is.
DJCi300.scratchScale = 1.0;

// How much faster seeking (shift+scratch) is than scratching.
DJCi300.scratchShiftMultiplier = 4;

// How fast bending is.
DJCi300.bendScale = 1.0;

// Other scratch related options
DJCi300.kScratchActionBend = 0;
DJCi300.kScratchActionScratch = 1;
DJCi300.kScratchActionSeek = 2;

// Pad modes
DJCi300.padModeNone = -1;
DJCi300.padModeHotcue = 0;
DJCi300.padModeRoll = 1;
DJCi300.padModeSlicer = 2;
DJCi300.padModeSampler = 3;
DJCi300.padModeToneplay = 4;
DJCi300.padModeFX = 5;
DJCi300.padModeSlicerloop = 6;
DJCi300.padModeBeatjump = 7;

// Slicer connections
let slicerSizeConnection1;
let slicerTrackConnection1;
let slicerBeatConnection1;
let slicerTrackConnection2;
let slicerSizeConnection2;
let slicerBeatConnection2;

DJCi300.vuMeterUpdateMaster = function(value, _group, _control) {
    value = (value * 125);
    midi.sendShortMsg(0xB0, 0x40, value);
    midi.sendShortMsg(0xB0, 0x41, value);
};

DJCi300.vuMeterUpdateDeck = function(value, group, _control, _status) {
    value = (value * 125);
    const status = (group === "[Channel1]") ? 0xB1 : 0xB2;
    midi.sendShortMsg(status, 0x40, value);
};

DJCi300.init = function() {
    // Scratch button state
    DJCi300.scratchButtonState = {
        1: true,
        2: true
    };
    // Scratch Action
    DJCi300.scratchAction = {
        1: DJCi300.kScratchActionBend,
        2: DJCi300.kScratchActionBend
    };
    // Platter state (whether the jog wheel is pressed or not)
    DJCi300.wheelTouchState = {
        1: false,
        2: false
    };

    // Pad mode variables
    // Initialize to padModeNone
    DJCi300.padMode = {
        1: DJCi300.padModeNone,
        2: DJCi300.padModeNone
    };

    // Toneplay variables
    // Toneplay offset (shifts the toneplay keyboard up or down)
    DJCi300.toneplayOffset = {
        1: 0,
        2: 0
    };

    // Slicer variables
    // Slicer storage (stores slicer button positions)
    DJCi300.slicerPoints = {
        1: [-1, -1, -1, -1, -1, -1, -1, -1, -1],
        2: [-1, -1, -1, -1, -1, -1, -1, -1, -1]
    };
    // Slicer buttons (stores whether or not slicer button is pressed)
    DJCi300.slicerButtonEnabled = {
        1: [0, 0, 0, 0, 0, 0, 0, 0],
        2: [0, 0, 0, 0, 0, 0, 0, 0]
    };
    // Slicer beat (stores what beat slicer is on)
    DJCi300.slicerBeat = {
        1: 0,
        2: 0
    };

    // Loop variables
    // loopMode is on (equal to 1) if the current loop is created by the "Loop In" button
    // and off (equal to 0) if it is created by the slicer buttons
    DJCi300.loopMode = {
        1: 0,
        2: 0
    };

    // Turn On Vinyl buttons LED(one for each deck).
    midi.sendShortMsg(0x91, 0x03, 0x7F);
    midi.sendShortMsg(0x92, 0x03, 0x7F);

    // Turn On Browser button LED
    midi.sendShortMsg(0x90, 0x04, 0x05);

    // Turn On Toneplay LED
    midi.sendShortMsg(0x96, 0x40, 0x7F);
    midi.sendShortMsg(0x96, 0x48, 0x7F);
    midi.sendShortMsg(0x97, 0x40, 0x7F);
    midi.sendShortMsg(0x97, 0x48, 0x7F);

    //Softtakeover for Pitch fader
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);
    engine.softTakeoverIgnoreNextValue("[Channel1]", "rate");
    engine.softTakeoverIgnoreNextValue("[Channel2]", "rate");

    // Connect the VUMeters
    engine.makeConnection("[Channel1]", "VuMeter", DJCi300.vuMeterUpdateDeck);
    engine.makeConnection("[Channel2]", "VuMeter", DJCi300.vuMeterUpdateDeck);
    engine.makeConnection("[Master]", "VuMeterL", DJCi300.vuMeterUpdateMaster);
    engine.makeConnection("[Master]", "VuMeterR", DJCi300.vuMeterUpdateMaster);

    // Connect beatmatch LED functions
    engine.makeConnection("[Channel1]", "beat_distance", DJCi300.updateBeatmatchTempoLED);
    engine.makeConnection("[Channel2]", "beat_distance", DJCi300.updateBeatmatchTempoLED);
    engine.makeConnection("[Channel1]", "beat_distance", DJCi300.updateBeatmatchAlignLED);
    engine.makeConnection("[Channel2]", "beat_distance", DJCi300.updateBeatmatchAlignLED);

    // Connect jogwheel functions
    engine.makeConnection("[Channel1]", "scratch2", DJCi300.updateScratchAction);
    engine.makeConnection("[Channel2]", "scratch2", DJCi300.updateScratchAction);

    // Connect the toneplay LED updates
    engine.makeConnection("[Channel1]", "pitch", DJCi300.updateToneplayLED);
    engine.makeConnection("[Channel2]", "pitch", DJCi300.updateToneplayLED);

    // Define slicer connections by connecting them
    DJCi300.connectSlicerFunctions(1);
    DJCi300.connectSlicerFunctions(2);
    // Then disconnect them since we're not in slicer mode (yet)
    DJCi300.disconnectSlicerFunctions(1);
    DJCi300.disconnectSlicerFunctions(2);

    // Ask the controller to send all current knob/slider values over MIDI, which will update
    // the corresponding GUI controls in MIXXX.
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
};

// Update beatmatch tempo LEDs
DJCi300.updateBeatmatchTempoLED = function(_value, _group, _control) {
    const deck1tempo = engine.getValue("[Channel1]", "bpm");
    const deck2tempo = engine.getValue("[Channel2]", "bpm");

    // If successfully synced, or if one of the songs are paused, turn all lights off
    if ((Math.abs(deck1tempo - deck2tempo) < DJCi300.beatmatchTempoTolerance) ||
        (engine.getValue("[Channel1]", "play") === 0) ||
        (engine.getValue("[Channel2]", "play") === 0)) {
        midi.sendShortMsg(0x91, 0x1E, 0x00);
        midi.sendShortMsg(0x91, 0x1F, 0x00);
        midi.sendShortMsg(0x92, 0x1E, 0x00);
        midi.sendShortMsg(0x92, 0x1F, 0x00);
    // If deck 1 is faster, lights tell user to slow down 1 and speed up 2
    } else if (deck1tempo > deck2tempo) {
        midi.sendShortMsg(0x91, 0x1E, 0x7F);
        midi.sendShortMsg(0x91, 0x1F, 0x00);
        midi.sendShortMsg(0x92, 0x1E, 0x00);
        midi.sendShortMsg(0x92, 0x1F, 0x7F);
    // If deck 2 is faster, lights tell user to slow down 2 and speed up 1
    } else if (deck1tempo < deck2tempo) {
        midi.sendShortMsg(0x91, 0x1E, 0x00);
        midi.sendShortMsg(0x91, 0x1F, 0x7F);
        midi.sendShortMsg(0x92, 0x1E, 0x7F);
        midi.sendShortMsg(0x92, 0x1F, 0x00);
    }
};

// Update beatmatch align LEDs
DJCi300.updateBeatmatchAlignLED = function(_value, _group, _control) {
    let deck1Align = engine.getValue("[Channel1]", "beat_distance");
    let deck2Align = engine.getValue("[Channel2]", "beat_distance");

    // Because beat_distance resets to 0 every new beat, it's possible for the two decks to have
    // very different beat values and still be almost aligned. So we must adjust for this
    if (Math.abs(deck1Align - deck2Align) > .5) {
        // Add 1 to the smaller number to compensate for roll over
        if (deck1Align < deck2Align) {
            deck1Align += 1;
        } else {
            deck2Align += 1;
        }
    }

    // If successfully synced, or if one of the songs are paused, turn all lights off
    if ((Math.abs(deck1Align - deck2Align) < DJCi300.beatmatchAlignTolerance) ||
        (engine.getValue("[Channel1]", "play") === 0) ||
        (engine.getValue("[Channel2]", "play") === 0)) {
        midi.sendShortMsg(0x91, 0x1C, 0x00);
        midi.sendShortMsg(0x91, 0x1D, 0x00);
        midi.sendShortMsg(0x92, 0x1C, 0x00);
        midi.sendShortMsg(0x92, 0x1D, 0x00);
    // If deck 1 is ahead, lights tell user to push 1 back and push 2 ahead
    } else if (deck1Align > deck2Align) {
        midi.sendShortMsg(0x91, 0x1C, 0x00);
        midi.sendShortMsg(0x91, 0x1D, 0x7F);
        midi.sendShortMsg(0x92, 0x1C, 0x7F);
        midi.sendShortMsg(0x92, 0x1D, 0x00);
    // If deck 2 is ahead, lights tell user to push 2 back and push 1 ahead
    } else if (deck1Align < deck2Align) {
        midi.sendShortMsg(0x91, 0x1C, 0x7F);
        midi.sendShortMsg(0x91, 0x1D, 0x00);
        midi.sendShortMsg(0x92, 0x1C, 0x00);
        midi.sendShortMsg(0x92, 0x1D, 0x7F);
    }
};

// The Vinyl button, used to enable or disable scratching on the jog wheels (One per deck).
DJCi300.vinylButton = function(channel, control, value, status, _group) {
    const deck = channel;

    if (value) {
        if (DJCi300.scratchButtonState[deck]) {
            DJCi300.scratchButtonState[deck] = false;
            midi.sendShortMsg(status, control, 0x00);
        } else {
            DJCi300.scratchButtonState[deck] = true;
            midi.sendShortMsg(status, control, 0x7F);
        }
    }
};

DJCi300._scratchEnable = function(deck) {
    const alpha = 1.0/8;
    const beta = alpha/32;
    engine.scratchEnable(deck, 248, 33 + 1/3, alpha, beta);
};

DJCi300._convertWheelRotation = function(value) {
    // When you rotate the jogwheel, the controller always sends either 0x1
    // (clockwise) or 0x7F (counter clockwise). 0x1 should map to 1, 0x7F
    // should map to -1 (IOW it's 7-bit signed).
    return value < 0x40 ? 1 : -1;
};

// This is called immediately after the wheel is released and we want to switch between scratching and jogging gracefully.
// It is also connected to controls and called regularly to see if the wheel has slowed down enough. Once it does, then we switch from scratching to jogging
DJCi300.updateScratchAction = function(value, group, _control) {
    const deck = (group === "[Channel1]") ? 1 : 2;

    // Stop scratching only if the jogwheel is slow enough and the wheels are not being touched
    if (((Math.abs(value) < DJCi300.slipThreshold)) && (engine.isScratching(deck))
            && !DJCi300.wheelTouchState[deck]) {
        engine.scratchDisable(deck);
        DJCi300.scratchAction[deck] = DJCi300.kScratchActionBend;
    }
};

// The touch action on the jog wheel's top surface
DJCi300.wheelTouch = function(channel, _control, value, _status, group) {
    const deck = channel;
    if (value > 0) {
        // Enable scratching in vinyl mode OR if the deck is not playing
        if ((engine.getValue(group, "play") !== 1) || (DJCi300.scratchButtonState[deck])) {
            DJCi300._scratchEnable(deck);
            DJCi300.wheelTouchState[deck] = true;
            DJCi300.scratchAction[deck] = DJCi300.kScratchActionScratch;
        } else {
            DJCi300.scratchAction[deck] = DJCi300.kScratchActionBend;
        }
    } else {
        // Released the wheel.
        DJCi300.wheelTouchState[deck] = false;
        const scratchValue = engine.getValue(group, "scratch2");
        DJCi300.updateScratchAction(scratchValue, group);
    }
};

// The touch action on the jog wheel's top surface while holding shift
DJCi300.wheelTouchShift = function(channel, _control, value, _status, group) {
    const deck = channel - 3;
    // We always enable scratching regardless of button state.
    if (value > 0) {
        DJCi300._scratchEnable(deck);
        DJCi300.wheelTouchState[deck] = true;
        DJCi300.scratchAction[deck] = DJCi300.kScratchActionSeek;
    // Released the wheel.
    } else {
        DJCi300.wheelTouchState[deck] = false;
        const scratchValue = engine.getValue(group, "scratch2");
        DJCi300.updateScratchAction(scratchValue, group);
    }
};

// Using the jog wheel (spinning the jog wheel, regardless of whether surface or shift is held)
DJCi300.jogWheel = function(_channel, _control, value, status, _group) {
    var deck;
    switch (status) {
    case 0xB1:
    case 0xB4:
        deck  = 1;
        break;
    case 0xB2:
    case 0xB5:
        deck  = 2;
        break;
    default:
        return;
    }
    var interval = DJCi300._convertWheelRotation(value);
    var scratchAction = DJCi300.scratchAction[deck];
    if (scratchAction === DJCi300.kScratchActionScratch) {
        engine.scratchTick(deck, interval * DJCi300.scratchScale);
    } else if (scratchAction === DJCi300.kScratchActionSeek) {
        engine.scratchTick(deck,
            interval *  DJCi300.scratchScale *
            DJCi300.scratchShiftMultiplier);
    } else {
        engine.setValue(
            "[Channel" + deck + "]", "jog", interval * DJCi300.bendScale);
    }
};

// Helper function that calculates samples per beat
DJCi300._samplesPerBeat = function(deck) {
    const sampleRate = engine.getValue(`[Channel${  deck  }]`, "track_samplerate");
    const bpm = engine.getValue(`[Channel${  deck  }]`, "local_bpm");
    // For some reason, multiplying by 60 makes the size 1/2 as large as it's supposed to be
    // Hence, we multiply by 120 instead
    const secondsPerBeat = 120/bpm;
    const samplesPerBeat = secondsPerBeat * sampleRate;
    return samplesPerBeat;
};

// Helper function that calculates current position of play indicator in samples
DJCi300._currentPosition = function(deck) {
    const beatClosest = engine.getValue(`[Channel${  deck  }]`, "beat_closest");
    let beatDistance = engine.getValue(`[Channel${  deck  }]`, "beat_distance");

    // Map beatDistance so that it scales from 0 to .5, then -.5 to 0
    beatDistance = (beatDistance > .5) ? (beatDistance - 1) : beatDistance;
    // Adjust beatClosest and return
    return (DJCi300._samplesPerBeat(deck) * beatDistance) + beatClosest;
};

// Mode buttons
DJCi300.changeMode = function(channel, control, value, _status, _group) {
    const deck = channel;
    const oldPadMode = DJCi300.padMode[deck];
    DJCi300.padMode[deck] = control - 15;

    // Connect slicer functions when entering slicer or slicerloop mode
    if ((DJCi300.padMode[deck] === DJCi300.padModeSlicer) ||
        (DJCi300.padMode[deck] === DJCi300.padModeSlicerloop)) {

        if (value) {
            // Initialize slicer if it is not already initialized
            if (DJCi300.slicerPoints[deck][0] === -1) {
                DJCi300.slicerInit(deck, engine.getValue(`[Channel${  deck  }]`, "beat_closest"));
                DJCi300.connectSlicerFunctions(deck);
                // Turn off loop mode
                DJCi300.loopMode[deck] = 0;
            // If slicer is already initialized, clear it
            } else {
                DJCi300.slicerClear(deck);
                DJCi300.disconnectSlicerFunctions(deck);
                DJCi300.updateSlicerLED(deck);
            }
        }
    // When switching from slicer/slicer loop mode into the other modes, disconnect slicer functions
    } else if ((oldPadMode === DJCi300.padModeSlicer) ||
        (oldPadMode === DJCi300.padModeSlicerloop)) {

        // In loop mode, only clear slicer points (preserve the loop)
        if (DJCi300.loopMode[deck] === 1) {
            for (let i = 0; i <= 8; i++) {
                DJCi300.slicerPoints[deck][i] = -1;
            }
        // Otherwise call slicerClear
        } else {
            DJCi300.slicerClear(deck);
        }
        DJCi300.disconnectSlicerFunctions(deck);
        DJCi300.updateSlicerLED(deck);
    }
};

// Toneplay
DJCi300.toneplay = function(channel, control, value, _status, _group) {
    const deck = channel - 5;
    const button = control - 0x40 + 1;

    if (value) {
        // Jump to the most recently used hotcue
        const recentHotcue = engine.getValue(`[Channel${  deck  }]`, "hotcue_focus");
        if ((recentHotcue !== -1) && (engine.getValue(`[Channel${  deck  }]`,
            `hotcue_${  recentHotcue  }_enabled`))) {

            engine.setValue(`[Channel${  deck  }]`, `hotcue_${  recentHotcue  }_goto`, 1);
        } else {
            // If that hotcue doesn't exist or was deleted, jump to cue
            engine.setValue(`[Channel${  deck  }]`,
                "cue_goto", 1);
        }

        // Adjust pitch
        engine.setValue(`[Channel${  deck  }]`, "reset_key", 1);
        // Apply offset
        if (DJCi300.toneplayOffset[deck] >= 0) {
            for (let i = 0; i < DJCi300.toneplayOffset[deck]; i++) {
                engine.setValue(`[Channel${  deck  }]`, "pitch_up", 1);
            }
        } else {
            for (i = 0; i > DJCi300.toneplayOffset[deck]; i--) {
                engine.setValue(`[Channel${  deck  }]`, "pitch_down", 1);
            }
        }
        if (button <= 4) {
            // Buttons 1-4 are +0 to +3 semitones
            for (let i = 1; i < button; i++) {
                engine.setValue(`[Channel${  deck  }]`, "pitch_up", 1);
            }
            // Buttons 5-8 are -4 to -1 semitones
        } else {
            for (i = 8; i >= button; i--) {
                engine.setValue(`[Channel${  deck  }]`, "pitch_down", 1);
            }
        }
    }
};

// Toneplay shift (shift the toneplay keyboard up or down)
DJCi300.toneplayShift = function(channel, control, value, _status, group) {
    const deck = channel - 5;
    const direction = (control === 0x4B) ? 1 : 0;

    if (value) {
        // Because the keyboard ranges from -4 to 3 semitones and Mixxx can only shift
        // up to 6 semitones, the valid range of toneplayOffset is -2 to 3
        if (direction === 1) {
            DJCi300.toneplayOffset[deck] = Math.min(DJCi300.toneplayOffset[deck] + 1, 3);
        } else {
            DJCi300.toneplayOffset[deck] = Math.max(DJCi300.toneplayOffset[deck] - 1, -2);
        }
        // Update LEDs (because the keyboard has changed)
        const newValue = engine.getValue(group, "pitch");
        DJCi300.updateToneplayLED(newValue, group);
    }
};

// Update toneplay LEDs (LEDS will change depending on pitch, even if not caused by toneplay)
DJCi300.updateToneplayLED = function(value, group, _control) {
    const status = (group === "[Channel1]") ? 0x96 : 0x97;
    const deck = status - 0x95;
    let control = 0x40;

    // Apply offset
    value -= DJCi300.toneplayOffset[deck];

    // Cut off the value at -4 and 3 semitones, then round
    value = Math.min(value, 3);
    value = Math.max(value, -4);
    value = Math.round(value);

    // Buttons 1-4 (ctrl 0x40-0x43) are +0 to +3 semitones
    // Buttons 5-8 (ctrl 0x44-0x47) are -4 to -1 semitones
    if (value >= 0) {
        control = control + value;
    } else {
        control = control + 8 + value;
    }

    // Do the following for normal LEDs and the shifted LEDs
    // Turn off all LEDs
    for (let i = 0; i < 8; i++) {
        midi.sendShortMsg(status, 0x40 + i, 0x00);
        midi.sendShortMsg(status, 0x40 + i + 8, 0x00);
    }
    // Turn on current LED
    midi.sendShortMsg(status, control, 0x7F);
    midi.sendShortMsg(status, control + 8, 0x7F);
};

// Functions that connect and disconnect slicer functions to Mixxx controls, respectively
DJCi300.connectSlicerFunctions = function(deck) {
    if (deck === 1) {
        slicerSizeConnection1 = engine.makeConnection("[Channel1]", "beatloop_size", DJCi300.slicerChangeSize);
        slicerTrackConnection1 = engine.makeConnection("[Channel1]", "LoadSelectedTrack", DJCi300.slicerLoadTrack);
        slicerBeatConnection1 = engine.makeConnection("[Channel1]", "beat_distance", DJCi300.slicerCountBeat);
    } else {
        slicerTrackConnection2 = engine.makeConnection("[Channel2]", "LoadSelectedTrack", DJCi300.slicerLoadTrack);
        slicerSizeConnection2 = engine.makeConnection("[Channel2]", "beatloop_size", DJCi300.slicerChangeSize);
        slicerBeatConnection2 = engine.makeConnection("[Channel2]", "beat_distance", DJCi300.slicerCountBeat);
    }
};

DJCi300.disconnectSlicerFunctions = function(deck) {
    if (deck === 1) {
        if (slicerSizeConnection1.isConnected) { slicerSizeConnection1.disconnect(); }
        if (slicerTrackConnection1.isConnected) { slicerTrackConnection1.disconnect(); }
        if (slicerBeatConnection1.isConnected) { slicerBeatConnection1.disconnect(); }
    } else {
        if (slicerSizeConnection2.isConnected) { slicerSizeConnection2.disconnect(); }
        if (slicerTrackConnection2.isConnected) { slicerTrackConnection2.disconnect(); }
        if (slicerBeatConnection2.isConnected) { slicerBeatConnection2.disconnect(); }
    }
};

// This function is called every time we enter slicer mode
// It creates a loop to illustrate the active slicer section, and it
// also calculates the 8 slicer points and stores them in an array
// The startPos is the starting position of the loop (in samples)
DJCi300.slicerInit = function(deck, startPos) {
    const samplesBetweenPts = DJCi300._samplesPerBeat(deck) * engine.getValue(`[Channel${  deck  }]`, "beatloop_size") / 8;
    for (let i = 0; i <= 8; i++) {
        DJCi300.slicerPoints[deck][i] = startPos + (samplesBetweenPts * i);
    }

    // Disable the old loop (if it exists)
    if (engine.getValue(`[Channel${  deck  }]`, "loop_enabled") === 1) {
        engine.setValue(`[Channel${  deck  }]`, "reloop_toggle", 1);
    }
    // Set a new loop at startPos
    engine.setValue(`[Channel${  deck  }]`, "loop_start_position", DJCi300.slicerPoints[deck][0]);
    engine.setValue(`[Channel${  deck  }]`, "loop_end_position", DJCi300.slicerPoints[deck][8]);

    // Enable the loop if in slicer loop mode and loop is currently disabled
    if (DJCi300.padMode[deck] === DJCi300.padModeSlicerloop) {
        if (engine.getValue(`[Channel${  deck  }]`, "loop_enabled") === 0) {
            engine.setValue(`[Channel${  deck  }]`, "reloop_toggle", 1);
        }
    // If in normal slicer mode, disable the loop if it is currently enabled
    } else {
        if (engine.getValue(`[Channel${  deck  }]`, "loop_enabled") === 1) {
            engine.setValue(`[Channel${  deck  }]`, "reloop_toggle", 1);
        }
    }
};

// This function clears all set slicer points and loop points
DJCi300.slicerClear = function(deck) {
    // Clear slicer points
    for (let i = 0; i <= 8; i++) {
        DJCi300.slicerPoints[deck][i] = -1;
    }
    // Remove all loop points
    engine.setValue(`[Channel${  deck  }]`, "loop_start_position", -1);
    engine.setValue(`[Channel${  deck  }]`, "loop_end_position", -1);
};

// This function calls slicerClear when a new track is loaded
DJCi300.slicerLoadTrack = function(_value, group, _control) {
    const deck = (group === "[Channel1]") ? 1 : 2;
    DJCi300.disconnectSlicerFunctions(deck);
    DJCi300.slicerClear(deck);
    DJCi300.updateSlicerLED(deck);
};

// This function calls slicerInit when the length of the slicer section is adjusted
DJCi300.slicerChangeSize = function(_value, group, _control) {
    const deck = (group === "[Channel1]") ? 1 : 2;
    DJCi300.slicerInit(deck, DJCi300.slicerPoints[deck][0]);
};

// This function counts the beat that the slicer is on
// This is useful for moving the loop forward or lighting the LEDs
DJCi300.slicerCountBeat = function(_value, group, _control) {
    const deck = (group === "[Channel1]") ? 1 : 2;

    // Calculate current position in samples
    const currentPos = DJCi300._currentPosition(deck);

    // Calculate beat
    DJCi300.slicerBeat[deck] = -1;
    for (let i = 0; i <= 8; i++) {
        DJCi300.slicerBeat[deck] = (currentPos >= DJCi300.slicerPoints[deck][i]) ?
            (DJCi300.slicerBeat[deck] + 1) : DJCi300.slicerBeat[deck];
    }

    // If in slicer mode (not slicer loop mode), check to see if the slicer section needs to be moved
    if (DJCi300.padMode[deck] === DJCi300.padModeSlicer) {

        // If slicerBeat is 8, move the slicer section forward
        if (DJCi300.slicerBeat[deck] > 7) {
            DJCi300.slicerInit(deck, DJCi300.slicerPoints[deck][8]);
        }
    }

    DJCi300.updateSlicerLED(deck);
};

// Slicer pad buttons
DJCi300.slicerButton = function(channel, control, value, _status, group) {
    const deck = channel - 5;
    const button = control % 0x20;

    // Update array. 1 for on, 0 for off
    if (value) {
        DJCi300.slicerButtonEnabled[deck][button] = 1;
    } else {
        DJCi300.slicerButtonEnabled[deck][button] = 0;
    }

    const start = DJCi300.slicerButtonEnabled[deck].indexOf(1);
    const end = DJCi300.slicerButtonEnabled[deck].lastIndexOf(1) + 1;

    // If the slicer points are uninitialized, then do nothing. Otherwise:
    if (DJCi300.slicerPoints[deck][0] !== -1) {
        // If at least one button is pressed, create a loop between those points
        if (start !== -1) {
            engine.setValue(group, "loop_start_position", DJCi300.slicerPoints[deck][start]);
            engine.setValue(group, "loop_end_position", DJCi300.slicerPoints[deck][end]);
            engine.setValue(group, "loop_in_goto", 1);
            // Enable a loop if it doesn't already exist
            if (engine.getValue(group, "loop_enabled") === 0) {
                engine.setValue(group, "reloop_toggle", 1);
            }
        // Otherwise, reset the loop (make it 4 beats again, or however long the spinbox is)
        } else {
            engine.setValue(group, "loop_start_position", DJCi300.slicerPoints[deck][0]);
            engine.setValue(group, "loop_end_position", DJCi300.slicerPoints[deck][8]);

            // Disable the loop (unless we're in slicer loop mode)
            if (DJCi300.padMode[deck] !== DJCi300.padModeSlicerloop) {
                engine.setValue(group, "reloop_toggle", 1);
            }
        }

        DJCi300.updateSlicerLED(deck);
    }
};

// Slicer LED update
DJCi300.updateSlicerLED = function(deck) {
    const control = (DJCi300.padMode[deck] === DJCi300.padModeSlicer) ? 0x20 : 0x60;
    const status = (deck === 1) ? 0x96 : 0x97;

    const start = DJCi300.slicerButtonEnabled[deck].indexOf(1);
    const end = DJCi300.slicerButtonEnabled[deck].lastIndexOf(1) + 1;

    // Turn off all LEDs
    for (let i = 0; i < 8; i++) {
        midi.sendShortMsg(status, control + i, 0x00);
    }
    // If the slicer points are uninitialized, then do nothing. Otherwise:
    if (DJCi300.slicerPoints[deck][0] !== -1) {
        // If at least 1 button is held down, light that up
        // Or in the case of 2+ buttons, light up everything between the outer 2 buttons
        if (start !== -1) {
            for (i = start; i < end; i++) {
                midi.sendShortMsg(status, control + i, 0x7F);
            }
        // Otherwise, light up the LED corresponding to the beat
        } else {
            midi.sendShortMsg(status, control + Math.min(DJCi300.slicerBeat[deck], 7), 0x7F);
        }
    }
};

// Loop in button
DJCi300.loopInButton = function(channel, _control, value, _status, group) {
    const deck = channel;

    if (value) {
        // Override the active slicer if it exists
        DJCi300.slicerClear(deck);
        DJCi300.disconnectSlicerFunctions(deck);
        DJCi300.updateSlicerLED(deck);

        // Turn on loop mode
        DJCi300.loopMode[deck] = 1;

        // Create a 4 beat loop
        engine.setValue(group, "beatloop_4_activate", 1);
    }
};

// Loop out button
DJCi300.loopOutButton = function(channel, _control, value, _status, group) {
    const deck = channel;

    if (value) {
        // Override the active slicer if it exists
        DJCi300.slicerClear(deck);
        DJCi300.disconnectSlicerFunctions(deck);
        DJCi300.updateSlicerLED(deck);

        // Turn off loop mode
        DJCi300.loopMode[deck] = 0;

        // Disable the current loop if it exists
        if (engine.getValue(group, "loop_enabled") === 1) { engine.setValue(group, "reloop_toggle", 1); }
    }
};

DJCi300.shutdown = function() {
    midi.sendShortMsg(0xB0, 0x7F, 0x00);
};

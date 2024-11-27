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
// - Updated VU meter syntax (replaced vu_meter with VuMeter, connectControl with makeConnection, etc)
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
// * HOTCUES: Make loop hotcues more intuitive. Currently, the pads are always lit when loop cues are set,
//            regardless of whether or not the loop is enabled (maybe make the pad blink when set but inactive?)
//
// * ROLL: Keep SLIP active (if already enabled) when exiting from rolls
//
// * FX: See how to preselect effects for a rack
//
// * SLICER: Currently resource intensive because it's connected to beat_distance (which is always updated).
//           Is there a better way?
//
// ****************************************************************************
var DJCi300 = {};
///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// Beatmatch LED guide tolerances
DJCi300.beatmatchTempoTolerance = .1; // Measured in BPM (e.g. LEDS turn off if decks are <0.1 BPM apart)
DJCi300.beatmatchAlignTolerance = .01; // Measured in beats (e.g. LEDS turn off if decks are <0.01 beats apart)

// Determines how fast the wheel must be moving to be considered "slipping"
// Higher numbers result in longer backspins
DJCi300.slipThreshold = .1; // Must be between 0 and 1, non-inclusive

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
DJCi300.padModeNone = 0;
// These correspond directly to the MIDI control values
DJCi300.padModeHotcue = 15;
DJCi300.padModeRoll = 16;
DJCi300.padModeSlicer = 17;
DJCi300.padModeSampler = 18;
DJCi300.padModeToneplay = 19;
DJCi300.padModeFX = 20;
DJCi300.padModeSlicerloop = 21;
DJCi300.padModeBeatjump = 22;

DJCi300.vuMeterUpdateMain = function(value, _group, _control) {
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
        "[Channel1]": true,
        "[Channel2]": true
    };
    // Scratch Action
    DJCi300.scratchAction = {
        "[Channel1]": DJCi300.kScratchActionBend,
        "[Channel2]": DJCi300.kScratchActionBend
    };
    // Platter state (whether the jog wheel is pressed or not)
    DJCi300.wheelTouchState = {
        "[Channel1]": false,
        "[Channel2]": false
    };

    // Pad mode variables
    // Initialize to padModeNone
    DJCi300.padMode = {
        "[Channel1]": DJCi300.padModeNone,
        "[Channel2]": DJCi300.padModeNone
    };

    // Slicer variables
    // Slicer storage (stores slicer button positions)
    DJCi300.slicerPoints = {
        "[Channel1]": [-1, -1, -1, -1, -1, -1, -1, -1, -1],
        "[Channel2]": [-1, -1, -1, -1, -1, -1, -1, -1, -1]
    };
    // Slicer buttons (stores whether or not slicer button is pressed)
    DJCi300.slicerButtonEnabled = {
        "[Channel1]": [false, false, false, false, false, false, false, false],
        "[Channel2]": [false, false, false, false, false, false, false, false]
    };
    // Slicer beat (stores what beat slicer is on)
    DJCi300.slicerBeat = {
        "[Channel1]": 0,
        "[Channel2]": 0
    };

    // Slicer connections
    DJCi300.slicerConnections = {
        size: [undefined, undefined], // Connected to beatloop_size
        beat: [undefined, undefined], // Connected to beat_distance
        load: [undefined, undefined], // Connected to LoadSelectedTrack
    }

    // Loop variables
    // loopMode is true if the current loop is created by the "Loop In" button
    // and false if it is created by the slicer buttons
    DJCi300.loopMode = {
        "[Channel1]": false,
        "[Channel2]": false
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

    // Connect main VUMeters
    engine.makeConnection("[Main]", "vu_meter_left", DJCi300.vuMeterUpdateMain);
    engine.makeConnection("[Main]", "vu_meter_right", DJCi300.vuMeterUpdateMain);

    for (const group of ["[Channel1]","[Channel2]"]) {
        // Connect left and right VUMeters
        engine.makeConnection(group, "vu_meter", DJCi300.vuMeterUpdateDeck);

        //Softtakeover for Pitch fader
        engine.softTakeover(group, "rate", true);
        engine.softTakeoverIgnoreNextValue(group, "rate");

        // Connect jogwheel functions
        engine.makeConnection(group, "scratch2", DJCi300.updateScratchAction);

        // Connect the toneplay LED updates
        engine.makeConnection(group, "pitch", DJCi300.updateToneplayLED);

        // Connect beatmatch LED functions
        engine.makeConnection(group, "bpm", DJCi300.updateBeatmatchTempoLED);
        // We also want to update all beatmatch LEDs when a song is played/paused
        engine.makeConnection(group, "play", DJCi300.updateBeatmatchAlignLED);
        engine.makeConnection(group, "play", DJCi300.updateBeatmatchTempoLED);
    }
    // Only connect one channel to updateBeatmatchAlignLED because beatmatch LEDs are only enabled when both decks are playing
    engine.makeConnection("[Channel1]", "beat_distance", DJCi300.updateBeatmatchAlignLED);

    // Define slicer connections by connecting them
    DJCi300.connectSlicerFunctions("[Channel1]");
    DJCi300.connectSlicerFunctions("[Channel2]");
    // Then disconnect them since we're not in slicer mode (yet)
    DJCi300.disconnectSlicerFunctions("[Channel1]");
    DJCi300.disconnectSlicerFunctions("[Channel2]");

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
DJCi300.updateBeatmatchAlignLED = function(value, _group, _control) {
    let deck1Align = value;
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
DJCi300.vinylButton = function(_channel, control, value, status, group) {
    if (value) {
        if (DJCi300.scratchButtonState[group]) {
            DJCi300.scratchButtonState[group] = false;
            midi.sendShortMsg(status, control, 0x00);
        } else {
            DJCi300.scratchButtonState[group] = true;
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
// It is also connected to callbacks and called regularly to see if the wheel has slowed down enough. Once it does, then we switch from scratching to jogging
DJCi300.updateScratchAction = function(value, group, _control) {
    const deck = script.deckFromGroup(group);

    // Stop scratching only if the jogwheel is slow enough and the wheels are not being touched
    if (((Math.abs(value) < DJCi300.slipThreshold)) && (engine.isScratching(deck))
            && !DJCi300.wheelTouchState[group]) {
        engine.scratchDisable(deck);
        DJCi300.scratchAction[group] = DJCi300.kScratchActionBend;
    }
};

// The touch action on the jog wheel's top surface
DJCi300.wheelTouch = function(_channel, _control, value, _status, group) {
    const deck = script.deckFromGroup(group);
    if (value > 0) {
        // Enable scratching in vinyl mode OR if the deck is not playing
        if ((engine.getValue(group, "play") !== 1) || (DJCi300.scratchButtonState[group])) {
            DJCi300._scratchEnable(deck);
            DJCi300.wheelTouchState[group] = true;
            DJCi300.scratchAction[group] = DJCi300.kScratchActionScratch;
        } else {
            DJCi300.scratchAction[group] = DJCi300.kScratchActionBend;
        }
    } else {
        // Released the wheel.
        DJCi300.wheelTouchState[group] = false;
        const scratchValue = engine.getValue(group, "scratch2");
        DJCi300.updateScratchAction(scratchValue, group);
    }
};

// The touch action on the jog wheel's top surface while holding shift
DJCi300.wheelTouchShift = function(_channel, _control, value, _status, group) {
    const deck = script.deckFromGroup(group);
    // We always enable scratching regardless of button state.
    if (value > 0) {
        DJCi300._scratchEnable(deck);
        DJCi300.wheelTouchState[group] = true;
        DJCi300.scratchAction[group] = DJCi300.kScratchActionSeek;
    // Released the wheel.
    } else {
        DJCi300.wheelTouchState[group] = false;
        const scratchValue = engine.getValue(group, "scratch2");
        DJCi300.updateScratchAction(scratchValue, group);
    }
};

// Using the jog wheel (spinning the jog wheel, regardless of whether surface or shift is held)
DJCi300.jogWheel = function(_channel, _control, value, _status, group) {
    const deck = script.deckFromGroup(group);

    var interval = DJCi300._convertWheelRotation(value);
    var scratchAction = DJCi300.scratchAction[group];
    if (scratchAction === DJCi300.kScratchActionScratch) {
        engine.scratchTick(deck, interval * DJCi300.scratchScale);
    } else if (scratchAction === DJCi300.kScratchActionSeek) {
        engine.scratchTick(deck,
            interval *  DJCi300.scratchScale *
            DJCi300.scratchShiftMultiplier);
    } else {
        engine.setValue(
            group, "jog", interval * DJCi300.bendScale);
    }
};

// Helper function that calculates samples per beat
DJCi300._samplesPerBeat = function(group) {
    const sampleRate = engine.getValue(group, "track_samplerate");
    const bpm = engine.getValue(group, "local_bpm");
    // The sample rate includes both channels (i.e. it is double the framerate)
    // Hence, we multiply by 2*60 (120) instead of 60 to get the correct sample rate
    const secondsPerBeat = 120/bpm;
    const samplesPerBeat = secondsPerBeat * sampleRate;
    return samplesPerBeat;
};

// Helper function that calculates current position of play indicator in samples
DJCi300._currentPosition = function(group) {
    const beatClosest = engine.getValue(group, "beat_closest");
    let beatDistance = engine.getValue(group, "beat_distance");

    // Map beatDistance so that it scales from 0 to .5, then -.5 to 0
    beatDistance = (beatDistance > .5) ? (beatDistance - 1) : beatDistance;
    // Adjust beatClosest and return
    return (DJCi300._samplesPerBeat(group) * beatDistance) + beatClosest;
};

// Mode buttons
DJCi300.changeMode = function(_channel, control, value, _status, group) {
    const oldPadMode = DJCi300.padMode[group];
    DJCi300.padMode[group] = control;

    // Connect slicer functions when entering slicer or slicerloop mode
    if ((DJCi300.padMode[group] === DJCi300.padModeSlicer) ||
        (DJCi300.padMode[group] === DJCi300.padModeSlicerloop)) {

        if (value) {
            // Initialize slicer if it is not already initialized
            if (DJCi300.slicerPoints[group][0] === -1) {
                DJCi300.slicerInit(group, engine.getValue(group, "beat_closest"));
                DJCi300.connectSlicerFunctions(group);
                // Turn off loop mode
                DJCi300.loopMode[group] = false;
            // If slicer is already initialized, clear it
            } else {
                DJCi300.slicerClear(group);
                DJCi300.disconnectSlicerFunctions(group);
                DJCi300.updateSlicerLED(group);
            }
        }
    // When switching from slicer/slicer loop mode into the other modes, disconnect slicer functions
    } else if ((oldPadMode === DJCi300.padModeSlicer) ||
        (oldPadMode === DJCi300.padModeSlicerloop)) {

        // In loop mode, only clear slicer points (preserve the loop)
        if (DJCi300.loopMode[group] === true) {
            for (let i = 0; i <= 8; i++) {
                DJCi300.slicerPoints[group][i] = -1;
            }
        // Otherwise call slicerClear
        } else {
            DJCi300.slicerClear(group);
        }
        DJCi300.disconnectSlicerFunctions(group);
        DJCi300.updateSlicerLED(group);
    }
};

// Toneplay
DJCi300.toneplay = function(_channel, control, value, _status, group) {
    let button = control - 0x40 + 1;

    if (value) {
        // Pad buttons (buttons 1-8) will jump to a hotcue and change pitch
        // Shift + pad buttons (buttons 9-16) will only change pitch without jumping
        if (button <= 8) {
            // Jump to the most recently used hotcue
            const recentHotcue = engine.getValue(group, "hotcue_focus");
            if ((recentHotcue > 0) && (engine.getValue(group,
                `hotcue_${  recentHotcue  }_status`) > 0)) {

                engine.setValue(group, `hotcue_${  recentHotcue  }_goto`, 1);
            } else {
                // If that hotcue doesn't exist or was deleted, jump to cue
                engine.setValue(group,
                    "cue_goto", 1);
            }
        }

        // Adjust pitch
        if (button <= 4) {
            // Buttons 1-4 are +0 to +3 semitones
            engine.setValue(group, "pitch", button - 1);
            // Buttons 5-8 are -4 to -1 semitones
        } else {
            engine.setValue(group, "pitch", button - 9);
        }
    }
};

// Update toneplay LEDs (LEDS will change depending on pitch, even if not caused by toneplay)
DJCi300.updateToneplayLED = function(value, group, _control) {
    const status = (group === "[Channel1]") ? 0x96 : 0x97;
    let control = 0x40;

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

// Functions that connect and disconnect slicer functions to Mixxx callbacks, respectively
DJCi300.connectSlicerFunctions = function(group) {
    const index = script.deckFromGroup(group) - 1;
    DJCi300.slicerConnections.size[index] = engine.makeConnection(group, "beatloop_size", DJCi300.slicerChangeSize);
    DJCi300.slicerConnections.load[index] = engine.makeConnection(group, "LoadSelectedTrack", DJCi300.slicerLoadTrack);
    DJCi300.slicerConnections.beat[index] = engine.makeConnection(group, "beat_distance", DJCi300.slicerCountBeat);
};

DJCi300.disconnectSlicerFunctions = function(group) {
    const index = script.deckFromGroup(group) - 1;
    if (DJCi300.slicerConnections.size[index].isConnected) { DJCi300.slicerConnections.size[index].disconnect(); }
    if (DJCi300.slicerConnections.load[index].isConnected) { DJCi300.slicerConnections.load[index].disconnect(); }
    if (DJCi300.slicerConnections.beat[index].isConnected) { DJCi300.slicerConnections.beat[index].disconnect(); }
};

// This function is called every time we enter slicer mode
// It creates a loop to illustrate the active slicer section, and it
// also calculates the 8 slicer points and stores them in an array
// The startPos is the starting position of the loop (in samples)
DJCi300.slicerInit = function(group, startPos) {
    const samplesBetweenPts = DJCi300._samplesPerBeat(group) * engine.getValue(group, "beatloop_size") / 8;
    for (let i = 0; i <= 8; i++) {
        DJCi300.slicerPoints[group][i] = startPos + (samplesBetweenPts * i);
    }

    // Disable the old loop (if it exists)
    if (engine.getValue(group, "loop_enabled") === 1) {
        engine.setValue(group, "reloop_toggle", 1);
    }
    // Set a new loop at startPos
    engine.setValue(group, "loop_start_position", DJCi300.slicerPoints[group][0]);
    engine.setValue(group, "loop_end_position", DJCi300.slicerPoints[group][8]);

    // Enable the loop if in slicer loop mode (and if loop is currently disabled)
    if (DJCi300.padMode[group] === DJCi300.padModeSlicerloop) {
        if (engine.getValue(group, "loop_enabled") === 0) {
            engine.setValue(group, "reloop_toggle", 1);
        }
    // If in normal slicer mode, disable the loop (and if it is currently enabled)
    } else {
        if (engine.getValue(group, "loop_enabled") === 1) {
            engine.setValue(group, "reloop_toggle", 1);
        }
    }
};

// This function clears all set slicer points and loop points
DJCi300.slicerClear = function(group) {
    // Clear slicer points
    for (let i = 0; i <= 8; i++) {
        DJCi300.slicerPoints[group][i] = -1;
    }
    // Remove all loop points
    engine.setValue(group, "loop_start_position", -1);
    engine.setValue(group, "loop_end_position", -1);
};

// This function calls slicerClear when a new track is loaded
DJCi300.slicerLoadTrack = function(_value, group, _control) {
    DJCi300.disconnectSlicerFunctions(group);
    DJCi300.slicerClear(group);
    DJCi300.updateSlicerLED(group);
};

// This function calls slicerInit when the length of the slicer section is adjusted
DJCi300.slicerChangeSize = function(_value, group, _control) {
    DJCi300.slicerInit(group, DJCi300.slicerPoints[group][0]);
};

// This function counts the beat that the slicer is on
// This is useful for moving the loop forward or lighting the LEDs
DJCi300.slicerCountBeat = function(_value, group, _control) {
    // Calculate current position in samples
    const currentPos = DJCi300._currentPosition(group);

    // Calculate beat
    DJCi300.slicerBeat[group] = -1;
    for (let i = 0; i <= 8; i++) {
        DJCi300.slicerBeat[group] = (currentPos >= DJCi300.slicerPoints[group][i]) ?
            (DJCi300.slicerBeat[group] + 1) : DJCi300.slicerBeat[group];
    }

    // If in slicer mode (not slicer loop mode), check to see if the slicer section needs to be moved
    if (DJCi300.padMode[group] === DJCi300.padModeSlicer) {

        // If slicerBeat is 8, move the slicer section forward
        if (DJCi300.slicerBeat[group] > 7) {
            DJCi300.slicerInit(group, DJCi300.slicerPoints[group][8]);
        }
    }

    DJCi300.updateSlicerLED(group);
};

// Slicer pad buttons
DJCi300.slicerButton = function(_channel, control, value, _status, group) {
    const button = control % 0x20;

    // Update array. 1 for on, 0 for off
    if (value) {
        DJCi300.slicerButtonEnabled[group][button] = true;
    } else {
        DJCi300.slicerButtonEnabled[group][button] = false;
    }

    const start = DJCi300.slicerButtonEnabled[group].indexOf(true);
    const end = DJCi300.slicerButtonEnabled[group].lastIndexOf(true) + 1;

    // If the slicer points are uninitialized, then do nothing. Otherwise:
    if (DJCi300.slicerPoints[group][0] !== -1) {
        // If at least one button is pressed, create a loop between those points
        if (start !== -1) {
            engine.setValue(group, "loop_start_position", DJCi300.slicerPoints[group][start]);
            engine.setValue(group, "loop_end_position", DJCi300.slicerPoints[group][end]);
            engine.setValue(group, "loop_in_goto", 1);
            // Enable a loop if it doesn't already exist
            if (engine.getValue(group, "loop_enabled") === 0) {
                engine.setValue(group, "reloop_toggle", 1);
            }
        // Otherwise, reset the loop (make it 4 beats again, or however long the spinbox is)
        } else {
            engine.setValue(group, "loop_start_position", DJCi300.slicerPoints[group][0]);
            engine.setValue(group, "loop_end_position", DJCi300.slicerPoints[group][8]);

            // Disable the loop (unless we're in slicer loop mode)
            if (DJCi300.padMode[group] !== DJCi300.padModeSlicerloop) {
                engine.setValue(group, "reloop_toggle", 1);
            }
        }

        DJCi300.updateSlicerLED(group);
    }
};

// Slicer LED update
DJCi300.updateSlicerLED = function(group) {
    const control = (DJCi300.padMode[group] === DJCi300.padModeSlicer) ? 0x20 : 0x60;
    const status = (group === "[Channel1]") ? 0x96 : 0x97;

    const start = DJCi300.slicerButtonEnabled[group].indexOf(true);
    const end = DJCi300.slicerButtonEnabled[group].lastIndexOf(true) + 1;

    // Turn off all LEDs
    for (let i = 0; i < 8; i++) {
        midi.sendShortMsg(status, control + i, 0x00);
    }
    // If the slicer points are uninitialized, then do nothing. Otherwise:
    if (DJCi300.slicerPoints[group][0] !== -1) {
        // If at least 1 button is held down, light that up
        // Or in the case of 2+ buttons, light up everything between the outer 2 buttons
        if (start !== -1) {
            for (let i = start; i < end; i++) {
                midi.sendShortMsg(status, control + i, 0x7F);
            }
        // Otherwise, light up the LED corresponding to the beat
        } else {
            midi.sendShortMsg(status, control + Math.min(DJCi300.slicerBeat[group], 7), 0x7F);
        }
    }
};

// Loop in button
DJCi300.loopInButton = function(_channel, _control, value, _status, group) {
    if (value) {
        // Override the active slicer if it exists
        DJCi300.slicerClear(group);
        DJCi300.disconnectSlicerFunctions(group);
        DJCi300.updateSlicerLED(group);

        // Turn on loop mode
        DJCi300.loopMode[group] = true;

        // Create a 4 beat loop
        engine.setValue(group, "beatloop_4_activate", 1);
    }
};

// Loop out button
DJCi300.loopOutButton = function(_channel, _control, value, _status, group) {
    if (value) {
        // Override the active slicer if it exists
        DJCi300.slicerClear(group);
        DJCi300.disconnectSlicerFunctions(group);
        DJCi300.updateSlicerLED(group);

        // Turn off loop mode
        DJCi300.loopMode[group] = false;

        // Disable the current loop if it exists
        if (engine.getValue(group, "loop_enabled") === 1) { engine.setValue(group, "reloop_toggle", 1); }
    }
};

DJCi300.shutdown = function() {
    midi.sendShortMsg(0xB0, 0x7F, 0x00);
};

// DJControl_Inpulse_300_script.js
//
// ***************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Inpulse 300.
// * Author: DJ Phatso, contributions by Kerrick Staley and BoredGuy1
// * Version 1.3 (Jan 2024)
// * Forum: https://www.mixxx.org/forums/viewtopic.php?f=7&t=12599
// * Wiki: https://mixxx.org/wiki/doku.php/hercules_djcontrol_inpulse_300
//
// Changes to v1.3
// - Added ability to stop samplers (shift + button)
// - Added toneplay
// - Changed scratching (wheels now have inertia, allowing backspins, etc)
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
// * SLICER/SLICER LOOP
//      - Make loop size changeable on the fly (perhaps using in/out loop?)
//      - Update slicer to support scratching backwards when enabled
// * SCRATCHING: Fix so that jogging is possible again (MEDIUM PRIORITY)
//
// * BEATMATCH GUIDE (MEDIUM PRIORITY)
//
// * TONEPLAY: Map shift buttons (LOW PRIORITY)
//
// * BEATJUMP: Add LEDs (LOW PRIORITY)
//
// ****************************************************************************
var DJCi300 = {};
///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// How fast scratching is.
DJCi300.scratchScale = 1.0;

// How much faster seeking (shift+scratch) is than scratching.
DJCi300.scratchShiftMultiplier = 4;

// How fast bending is.
DJCi300.bendScale = 1.0;

// Other scratch related options
DJCi300.kScratchActionNone = 0;
DJCi300.kScratchActionScratch = 1;
DJCi300.kScratchActionSeek = 2;
DJCi300.kScratchActionBend = 3;

// Pad modes
DJCi300.padModeHotcue = 0;
DJCi300.padModeRoll = 1;
DJCi300.padModeSlicer = 2;
DJCi300.padModeSampler = 3;
DJCi300.padModeToneplay = 4;
DJCi300.padModeFX = 5;
DJCi300.padModeSlicerloop = 6;
DJCi300.padModeBeatjump = 7;

// Timer lengths
DJCi300.wheelTimerLength = 25;

// Determines how fast the wheel must be moving to be considered "slipping"
DJCi300.slipThreshold = 0.01;

// Slicer variables
DJCi300.slicerLoopLength = 4;

DJCi300.vuMeterUpdateMaster = function(value, _group, _control) {
    value = (value * 122) + 5;
    midi.sendShortMsg(0xB0, 0x40, value);
    midi.sendShortMsg(0xB0, 0x41, value);
};

DJCi300.vuMeterUpdateDeck = function(value, group, _control, _status) {
    value = (value * 122) + 5;
    var status = (group === "[Channel1]") ? 0xB1 : 0xB2;
    midi.sendShortMsg(status, 0x40, value);
};

DJCi300.init = function() {
    // Scratch button state
    DJCi300.scratchButtonState = true;
    // Scratch Action
    DJCi300.scratchAction = {
    1: DJCi300.kScratchActionNone,
    2: DJCi300.kScratchActionNone
    };

    // Jogwheel timer
    DJCi300.wheelTimer = {
        1: 0,
        2: 0
    };

    // Pad mode
    DJCi300.padMode = {
        1: DJCi300.padModeHotcue,
        2: DJCi300.padModeHotcue
    };

    // Slicer storage (stores slicer button positions)
    DJCi300.slicerPoints = {
        1: [0,0,0,0,0,0,0,0,0],
        2: [0,0,0,0,0,0,0,0,0]
    };

    // Slicer buttons (stores whether or not slicer button is pressed)
    DJCi300.slicerButtonEnabled = {
        1: [0,0,0,0,0,0,0,0],
        2: [0,0,0,0,0,0,0,0]
    };

    // Slicer beat (stores what beat slicer is on)
    DJCi300.slicerBeatCount = {
        1: 0,
        2: 0
    };

    // Turn On Vinyl buttons LED(one for each deck).
    midi.sendShortMsg(0x91, 0x03, 0x7F);
    midi.sendShortMsg(0x92, 0x03, 0x7F);

    //Turn On Browser button LED
    midi.sendShortMsg(0x90, 0x04, 0x05);

   //Softtakeover for Pitch fader
   engine.softTakeover("[Channel1]", "rate", true);
   engine.softTakeover("[Channel2]", "rate", true);
   engine.softTakeoverIgnoreNextValue("[Channel1]", "rate");
   engine.softTakeoverIgnoreNextValue("[Channel2]", "rate");

   // Connect the VUMeters
    engine.connectControl("[Channel1]", "vu_meter", "DJCi300.vuMeterUpdateDeck");
	engine.getValue("[Channel1]", "vu_meter", "DJCi300.vuMeterUpdateDeck");
    engine.connectControl("[Channel2]", "vu_meter", "DJCi300.vuMeterUpdateDeck");
	engine.getValue("[Channel2]", "vu_meter", "DJCi300.vuMeterUpdateDeck");
    engine.connectControl("[Main]", "vu_meter_left", "DJCi300.vuMeterUpdateMaster");
    engine.connectControl("[Main]", "vu_meter_right", "DJCi300.vuMeterUpdateMaster");
	engine.getValue("[Main]", "vu_meter_left", "DJCi300.vuMeterUpdateMaster");
    engine.getValue("[Main]", "vu_meter_right", "DJCi300.vuMeterUpdateMaster");

    // Connect the LED updates
    engine.connectControl("[Channel1]", "pitch", "DJCi300.updateToneplayLED");
    engine.connectControl("[Channel2]", "pitch", "DJCi300.updateToneplayLED");

    // Connect slicer timers
    engine.connectControl("[Channel1]", "beat_distance", "DJCi300.updateSlicerBeat");
    engine.connectControl("[Channel2]", "beat_distance", "DJCi300.updateSlicerBeat");

    // Ask the controller to send all current knob/slider values over MIDI, which will update
    // the corresponding GUI controls in MIXXX.
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
};

// The Vinyl button, used to enable or disable scratching on the jog wheels (One per deck).
DJCi300.vinylButton = function(_channel, _control, value, status, _group) {
    if (value) {
        if (DJCi300.scratchButtonState) {
            DJCi300.scratchButtonState = false;
            midi.sendShortMsg(status, 0x03, 0x00);
        } else {
            DJCi300.scratchButtonState = true;
            midi.sendShortMsg(status, 0x03, 0x7F);
        }
    }
};

DJCi300._scratchEnable = function(deck) {
    var alpha = 1.0/8;
    var beta = alpha/32;
    engine.scratchEnable(deck, 248, 33 + 1/3, alpha, beta);
};

DJCi300._convertWheelRotation = function(value) {
    // When you rotate the jogwheel, the controller always sends either 0x1
    // (clockwise) or 0x7F (counter clockwise). 0x1 should map to 1, 0x7F
    // should map to -1 (IOW it's 7-bit signed).
    return value < 0x40 ? 1 : -1;
};

DJCi300._isSlipping = function(group) {
    // Function that checks whether or not the jogwheel is "slipping"
    // (IOW it was spun with touch action, but now it's still spinning
    // due to inertia)

    // If the jogwheel is at a certain speed, it is considered slipping
    // This allows us to do tricks like backspins
    var scratchRate = engine.getValue(group, "scratch2");

    if (Math.abs(scratchRate) > DJCi300.slipThreshold) {
        return true;
    } else {
        return false;
    }
};

DJCi300._wheelRelease = function(deck) {
    // This is called after the wheel is released and we want to
    // switch between scratching and jogging gracefully
    // Create a timer that disables slipping after some time (switches from scratching to bending)
    // Reset timer (if it exists)
    if (DJCi300.wheelTimer !== 0) {
        engine.stopTimer(DJCi300.wheelTimer);
        DJCi300.wheelTimer = 0;
    }
    // Start timer
    DJCi300.wheelTimer = engine.beginTimer(DJCi300.wheelTimerLength,
        function() {
            DJCi300.wheelTimer = 0;
            engine.scratchDisable(deck);
            DJCi300.scratchAction[deck] = DJCi300.kScratchActionBend;
        },
        true);
}

// The touch action on the jog wheel's top surface
DJCi300.wheelTouch = function(channel, control, value, _status, group) {
    var deck = channel;
    if (value > 0) {
        //  Touching the wheel.
        if (engine.getValue("[Channel" + deck + "]", "play") !== 1 || DJCi300.scratchButtonState) {
            DJCi300._scratchEnable(deck);
            DJCi300.scratchAction[deck] = DJCi300.kScratchActionScratch;
        } else {
            DJCi300.scratchAction[deck] = DJCi300.kScratchActionBend;
        }
    } else {
        // Released the wheel.
        // Only disable scratching if the wheel is not spinning
        if (!DJCi300._isSlipping(deck)) {
            DJCi300._wheelRelease(deck);
        }
        
    }
};

// The touch action on the jog wheel's top surface while holding shift
DJCi300.wheelTouchShift = function(channel, control, value, _status, _group) {
    var deck = channel - 3;
    // We always enable scratching regardless of button state.
    if (value > 0) {
        DJCi300._scratchEnable(deck);
        DJCi300.scratchAction[deck] = DJCi300.kScratchActionSeek;
    } else {
        // Released the wheel.
        engine.scratchDisable(deck);
        DJCi300.scratchAction[deck] = DJCi300.kScratchActionNone;
    }
};

// Scratching on the jog wheel (rotating it while pressing the top surface)
DJCi300.scratchWheel = function(channel, control, value, status, _group) {
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
        DJCi300._wheelRelease(channel);
        engine.setValue(
            "[Channel" + deck + "]", "jog", interval * DJCi300.bendScale);
    }
};

// Bending on the jog wheel (rotating using the edge)
DJCi300.bendWheel = function(channel, control, value, _status, group) {
    var interval = DJCi300._convertWheelRotation(value);

    // Keep scratching if the wheel is already spinning (due to inertia)
    if (DJCi300._isSlipping) {
        engine.scratchTick(channel, interval * DJCi300.scratchScale);
    } else {
    // Otherwise, switch to bending
        DJCi300._wheelRelease(channel);
        engine.setValue(
            "[Channel" + channel + "]", "jog", interval * DJCi300.bendScale);
    }
};

// Calculate samples per beat
DJCi300._samplesPerBeat = function(deck) {
    var sampleRate = engine.getValue("[Channel" + deck + "]", "track_samplerate");
    var bpm = engine.getValue("[Channel" + deck + "]", "bpm");
    // For some reason, multiplying by 60 makes the size 1/2 as large as it's supposed to be
    // Hence, we multiply by 120 instead
    secondsPerBeat = 120/bpm;
    samplesPerBeat = secondsPerBeat * sampleRate;
    return samplesPerBeat;
}

// Calculates current position in samples
DJCi300._currentPosition = function(deck) {
    var beatClosest = engine.getValue("[Channel" + deck + "]", "beat_closest");
    var beatDistance = engine.getValue("[Channel" + deck + "]", "beat_distance");

    // Map beatDistance so that it scales from 0 to .5, then -.5 to 0
    beatDistance = (beatDistance > .5) ? (beatDistance - 1) : beatDistance;
    // Adjust beatClosest and return
    return (DJCi300._samplesPerBeat(deck) * beatDistance) + beatClosest;
}

// Mode buttons
DJCi300.changeMode = function(channel, control, value, _status, group) {
    var deck = channel;
    DJCi300.padMode[deck] = control - 15;

    print(deck)
    print(deck)
    print(deck)
    print(deck)
    print(deck)

    // We only need to trigger certain functions for slicer and slicerloop
    // But you could theoretically mod this to do cool stuff when entering other modes as well
    if (((DJCi300.padMode[deck] === DJCi300.padModeSlicer) ||
        (DJCi300.padMode[deck] === DJCi300.padModeSlicerloop)) && value === 0x7F ) {
        DJCi300.slicerInit(deck);
    }
};

// Toneplay
DJCi300.toneplay = function(channel, control, value, status, _group) {
    const deck = channel - 5;
    const button = control - 0x40 + 1;

    if (value === 0x7F) {
        // Jump to the most recently used hotcue
        const recentHotcue = engine.getValue("[Channel" + deck + "]", "hotcue_focus");
        if ((recentHotcue !== -1) && (engine.getValue("[Channel" + deck + "]",
            "hotcue_" + recentHotcue + "_enabled"))) {

            engine.setValue("[Channel" + deck + "]", "hotcue_" + recentHotcue + "_goto", 1);
        } else {
            // If that hotcue doesn't exist or was deleted, jump to cue
            engine.setValue("[Channel" + deck + "]",
                "cue_goto", 1);
        }

        // Adjust pitch
        // Buttons 1-4 are +0 to +3 semitones
        // Buttons 5-8 are -4 to -1 semitones
        // This mimics the original Inpulse 300's toneplay
        engine.setValue("[Channel" + deck + "]", "reset_key", 1);
        if (button <= 4) {
            for (var i = 1; i < button; i++) {
                engine.setValue("[Channel" + deck + "]", "pitch_up", 1);
            }
        } else {
            for (i = 8; i >= button; i--) {
                engine.setValue("[Channel" + deck + "]", "pitch_down", 1);
            }
        }
    }
};

// Update toneplay LEDs (will change depending on pitch, even if not caused by toneplay)
DJCi300.updateToneplayLED = function(value, group, _control) {
    var status = (group === "[Channel1]") ? 0x96 : 0x97;
    var control = 0x40

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

    // Turn off all LEDs
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(status, 0x40 + i, 0x00);
    }
    // Turn on current LED
    midi.sendShortMsg(status, control, 0x7F);
};

DJCi300.slicerInit = function(deck) {
    // This function is called every time we enter slicer mode
    // It creates a loop to illustrate the active slicer loop, and it
    // also calculates the 8 slicer points and stores them in an array

    // Get current location and the interval between slicer points
    var currentPos = engine.getValue("[Channel" + deck + "]", "beat_closest");
    var samplesBetweenPts = DJCi300._samplesPerBeat(deck) * DJCi300.slicerLoopLength / 8; 
    for (var i = 0; i <= 8; i++) {
        DJCi300.slicerPoints[deck][i] = currentPos + (samplesBetweenPts * i);
    }

    // Clear the old loop (if it exists) and set a new one to mark the slicer loop
    if (engine.getValue("[Channel" + deck + "]", "loop_enabled") === 1) {
        engine.setValue("[Channel" + deck + "]", "reloop_toggle", 1);
    }
    engine.setValue("[Channel" + deck + "]", "beatloop_" + DJCi300.slicerLoopLength + "_activate", 1);    
    // Enable the loop if in slicer loop mode and a loop doesn't already exist
    if (DJCi300.padMode[deck] === DJCi300.padModeSlicerloop) {
        if (engine.getValue("[Channel" + deck + "]", "loop_enabled") === 0) {
            engine.setValue("[Channel" + deck + "]", "reloop_toggle", 1);
        }
    // If in normal slicer mode, disable the loop if it already exists
    } else {
        if (engine.getValue("[Channel" + deck + "]", "loop_enabled") === 1) {
            engine.setValue("[Channel" + deck + "]", "reloop_toggle", 1);
        }
    }
};

// Slicer button
DJCi300.slicerButton = function(channel, control, value, status, _group) {
    var deck = channel - 5;
    var button = control % 0x20;

    // Update array. 1 for on, 0 for off
    if (value === 0x7F) {
        DJCi300.slicerButtonEnabled[deck][button] = 1;
    } else {
        DJCi300.slicerButtonEnabled[deck][button] = 0;
    }

    var start = DJCi300.slicerButtonEnabled[deck].indexOf(1);
    var end = DJCi300.slicerButtonEnabled[deck].lastIndexOf(1) + 1;

    // If at least one button is pressed, create a loop between those points
    if (start !== -1) {
        engine.setValue("[Channel" + deck + "]", "loop_start_position", DJCi300.slicerPoints[deck][start]);
        engine.setValue("[Channel" + deck + "]", "loop_end_position", DJCi300.slicerPoints[deck][end]);
        engine.setValue("[Channel" + deck + "]", "loop_in_goto", 1);
        // Enable a loop if it doesn't already exist
        if (engine.getValue("[Channel" + deck + "]", "loop_enabled") === 0) {
            engine.setValue("[Channel" + deck + "]", "reloop_toggle", 1);
        }
    // Otherwise, reset the loop (make it 4 beats again, or whatever the constant is set to)
    } else {
        engine.setValue("[Channel" + deck + "]", "loop_start_position", DJCi300.slicerPoints[deck][0]);
        engine.setValue("[Channel" + deck + "]", "loop_end_position", DJCi300.slicerPoints[deck][8]);
        
        // Disable the loop (unless we're in slicer loop mode)
        if (DJCi300.padMode[deck] !== DJCi300.padModeSlicerloop) {
            engine.setValue("[Channel" + deck + "]", "reloop_toggle", 1);
        }
    }

    DJCi300.updateSlicerLED(deck, status);
};

// Slicer counter (counts the beat that the slicer is on)
// This is useful for moving the loop forward or lighting the LEDs
DJCi300.updateSlicerBeat = function(_value, group, _control) {
    var deck = (group === "[Channel1]") ? 1 : 2;
    var status = (deck === 1) ? 0x96 : 0x97;

    // Calculate current position in samples
    var currentPos = DJCi300._currentPosition(deck);

    // Calculate beat
    DJCi300.slicerBeatCount[deck] = 0;
    for (var i = 1; i <= 8; i++) {
        DJCi300.slicerBeatCount[deck] = (currentPos >= DJCi300.slicerPoints[deck][i]) ? 
            (DJCi300.slicerBeatCount[deck] + 1) : DJCi300.slicerBeatCount[deck];
    }
    print(engine.getValue(group, "beat_closest"))
    print(engine.getValue(group, "beat_distance"))

    // Move the loop if in slicer mode (not slicer loop mode)
    if (DJCi300.padMode[deck] === DJCi300.padModeSlicer) {
        if (DJCi300.slicerBeatCount[deck] > 7) {
            DJCi300.slicerInit(deck);
        }
    }

    DJCi300.updateSlicerLED(deck, status);
}

// Slicer LED update
DJCi300.updateSlicerLED = function(deck, status) {
    var control = (DJCi300.padMode[deck] === DJCi300.padModeSlicer) ? 0x20 : 0x60;

    var start = DJCi300.slicerButtonEnabled[deck].indexOf(1);
    var end = DJCi300.slicerButtonEnabled[deck].lastIndexOf(1) + 1;

    // Turn off all LEDs
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(status, control + i, 0x00);
    }
    // If at least 1 button is held down, light that up
    // Or in the case of 2+ buttons, light up everything between the outer 2 buttons
    if (start !== -1) {
        // needs led offset for control
        for (i = start; i < end; i++) {
            midi.sendShortMsg(status, control + i, 0x7F);
        }
    // Otherwise, light up the LED corresponding to the beat
    } else {
        midi.sendShortMsg(status, control + Math.min(DJCi300.slicerBeatCount[deck], 7), 0x7F);
    }
}

DJCi300.shutdown = function() {
    midi.sendShortMsg(0xB0, 0x7F, 0x00);
};

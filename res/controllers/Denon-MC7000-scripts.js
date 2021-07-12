/**
 * Denon DJ MC7000 DJ controller script for Mixxx 2.3
 *
 * Started in Dec. 2019 by OsZ
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Before using the mapping please make sure your MC7000 controller works for
 * your operating system. For Windows you need driver software by Denon, Mac users
 * should be lucky as it shall work out-of-the-box. Linux users need to know that
 * the MC7000 internal audio interface is not available out-of-the-box for
 * older Linux Kernels. You should upgrade your Kernel to minimum versions
 * LTS: 4.19.105 or 5.4.21, stable branch 5.5.5 or current 5.6 (2020-Feb-19).
 * Newer Kernels will provide native audio support for this controller.
**/

var MC7000 = {};

/*///////////////////////////////////
//      USER VARIABLES BEGIN       //
///////////////////////////////////*/

// switch on experimental features
// experimental features are:
// 1) Beat LED in Slicer mode (counts every 8 beats AFTER the CUE point)
//    only works for CONSTANT TEMPO tracks
//    needs beat grid and CUE point set
MC7000.experimental = false;

// Wanna have Needle Search active while playing a track ?
// In any case Needle Search is available holding "SHIFT" down.
// can be true or false (recommended: false)
MC7000.needleSearchPlay = false;

// Possible pitchfader rate ranges given in percent.
// can be cycled through by the RANGE buttons.
MC7000.rateRanges = [
    4/100,  // default: 4/100
    6/100,  // default: 6/100
    8/100,  // default: 8/100
    10/100, // default: 10/100
    16/100, // default: 16/100
    24/100, // default: 24/100
];

// Platter Ring LED mode
// Mode 0 = Single "off" LED chase (all others "on")
// Mode 1 = Single "on" LED chase (all others "off")
// use "SHIFT" + "DECK #" to toggle between both modes
MC7000.modeSingleLED = 1; // default: 1

// Set Vinyl Mode on ("true") or off ("false") when MIXXX starts.
// This sets the Jog Wheel touch detection / Vinyl Mode
// and the Jog LEDs ("VINYL" on = spinny, "VINYL" off = track position).
MC7000.VinylModeOn = true; // default: true

// Scratch algorithm parameters
MC7000.scratchParams = {
    recordSpeed: 33 + 1/3, // default: 33 + 1/3
    alpha: (1.0/10),   // default: (1.0/10)
    beta: (1.0/10)/32  // default: (1.0/10)/32
};

// Sensitivity factor of the jog wheel (also depends on audio latency)
// 0.5 for half, 2 for double sensitivity - Recommendation:
// set to 0.5 with audio buffer set to 50ms
// set to 1 with audio buffer set to 25ms
// set to 3 with audio buffer set to 5ms

MC7000.jogSensitivity = 1; // default: 1.0 with audio buffer set to 23ms

/*/////////////////////////////////
//      USER VARIABLES END       //
/////////////////////////////////*/


/* OTHER VARIABLES - DON'T TOUCH UNLESS YOU KNOW WHAT YOU'RE DOING */

// Resolution of the jog wheel, set so the spinny
// Jog LED to match exactly the movement of the Jog Wheel
// The physical resolution seems to be around 1100
MC7000.jogWheelTicksPerRevolution = 894;

// must be "true" for Needle Search to be active
MC7000.needleSearchTouched = [true, true, true, true];

// initial value for VINYL mode per Deck (see above for user input)
MC7000.isVinylMode = [MC7000.VinylModeOn, MC7000.VinylModeOn, MC7000.VinylModeOn, MC7000.VinylModeOn];

// used to keep track of which the rateRange of each slider.
// value used as an index to MC7000.rateRanges
MC7000.currentRateRangeIndex = [0, 0, 0, 0];

// initialize the "factor" function for Spinback
MC7000.factor = [];

//Set Shift button state to false as default
MC7000.shift = [false, false, false, false];

// initialize the PAD Mode to Hot Cue and all others off when starting
MC7000.PADModeCue = [true, true, true, true];
MC7000.PADModeCueLoop = [false, false, false, false];
MC7000.PADModeFlip = [false, false, false, false];
MC7000.PADModeRoll = [false, false, false, false];
MC7000.PADModeSavedLoop = [false, false, false, false];
MC7000.PADModeSlicer = [false, false, false, false];
MC7000.PADModeSlicerLoop = [false, false, false, false];
MC7000.PADModeSampler = [false, false, false, false];
MC7000.PADModeVelSamp = [false, false, false, false];
MC7000.PADModePitch = [false, false, false, false];

// PAD Mode 'Beatloop Roll' sizes
MC7000.beatLoopRoll = [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8];

// PAD Mode - 'Fixed Loop' sizes
MC7000.fixedLoop = [1, 2, 4, 8, 16, 32, 64, 128];

// PAD Mode - 'Beatjump' sizes
MC7000.beatJump = [1, 2, 4, 8, 16, 32, 64, 128];

// Define the MIDI signal for red LED at VU Meters
MC7000.VuMeterLEDPeakValue = 0x76;

// initialize variables to compare LED status for VU, Jog and PAD LEDs
MC7000.prevVuLevel = [0, 0, 0, 0];
MC7000.prevJogLED = [0, 0, 0, 0];
MC7000.prevPadLED = [0, 0, 0, 0];

// PAD Mode Colors
MC7000.padColor = {
    "alloff": 0x01,         // switch off completely
    // Hot Cue
    "hotcueon": 0x04,       // blue Hot Cue active
    "hotcueoff": 0x02,      // dark blue Hot Cue inactive
    // Cue Loop
    "cueloopon": 0x0D,      // green Cueloop colour for activated cue point
    "cueloopoff": 0x1A,     // dark green Cueloop colour inactive
    // Roll
    "rollon": 0x20,         // cyan BeatloopRoll active colour
    "rolloff": 0x06,        // dark cyan BeatloopRoll off colour
    // Saved Loop
    "fixedloopon": 0x3D,    // yellow Saved Loop active
    "fixedloopoff": 0x15,   // dark yellow Saved Loop active
    // Slicer
    "sliceron": 0x11,       // dark red activated Slicer
    "slicerJumpFwd": 0x31,  // red Sliver forward jump
    "slicerJumpBack": 0x31, // red Sliver backward jump
    // Sampler
    "samplerloaded": 0x38,  // pink Sampler loaded colour
    "samplerplay": 0x09,    // green Sampler playing
    "sampleroff": 0x12      // dark pink Sampler standard colour
};

/* DECK INITIALIZATION */
MC7000.init = function() {

    var i;

    // Softtakeover for Pitch Faders
    for (i = 1; i <= 4; i++) {
        engine.softTakeover("[Channel" + i + "]", "rate", true);
    }

    // VU meters
    engine.makeConnection("[Channel1]", "VuMeter", MC7000.VuMeter);
    engine.makeConnection("[Channel2]", "VuMeter", MC7000.VuMeter);
    engine.makeConnection("[Channel3]", "VuMeter", MC7000.VuMeter);
    engine.makeConnection("[Channel4]", "VuMeter", MC7000.VuMeter);

    // Platter Ring LED mode
    midi.sendShortMsg(0x90, 0x64, MC7000.modeSingleLED);
    midi.sendShortMsg(0x91, 0x64, MC7000.modeSingleLED);
    midi.sendShortMsg(0x92, 0x64, MC7000.modeSingleLED);
    midi.sendShortMsg(0x93, 0x64, MC7000.modeSingleLED);

    // Track Position LEDs for Jog Wheel and Slicer
    engine.makeConnection("[Channel1]", "playposition", MC7000.TrackPositionLEDs);
    engine.makeConnection("[Channel2]", "playposition", MC7000.TrackPositionLEDs);
    engine.makeConnection("[Channel3]", "playposition", MC7000.TrackPositionLEDs);
    engine.makeConnection("[Channel4]", "playposition", MC7000.TrackPositionLEDs);

    // Vinyl mode LEDs
    midi.sendShortMsg(0x90, 0x07, MC7000.isVinylMode ? 0x7F: 0x01);
    midi.sendShortMsg(0x91, 0x07, MC7000.isVinylMode ? 0x7F: 0x01);
    midi.sendShortMsg(0x92, 0x07, MC7000.isVinylMode ? 0x7F: 0x01);
    midi.sendShortMsg(0x93, 0x07, MC7000.isVinylMode ? 0x7F: 0x01);

    // HotCue Mode LEDs
    for (i = 1; i <= 8; i++) {
        engine.makeConnection("[Channel1]", "hotcue_"+i+"_enabled", MC7000.HotCueLED);
        engine.makeConnection("[Channel2]", "hotcue_"+i+"_enabled", MC7000.HotCueLED);
        engine.makeConnection("[Channel3]", "hotcue_"+i+"_enabled", MC7000.HotCueLED);
        engine.makeConnection("[Channel4]", "hotcue_"+i+"_enabled", MC7000.HotCueLED);
    }

    // Sampler Mode LEDs
    for (i = 1; i <= 8; i++) {
        engine.makeConnection("[Sampler"+i+"]", "track_loaded", MC7000.SamplerLED);
        engine.makeConnection("[Sampler"+i+"]", "play", MC7000.SamplerLED);
    }

    // send Controller Status SysEx message delayed to avoid conflicts with Softtakeover
    engine.beginTimer(2000, MC7000.delayedSysEx, true);
};

// SysEx message to receive all knob and fader positions
MC7000.delayedSysEx = function() {
    var ControllerStatusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
};

// Sampler Volume Control
MC7000.samplerLevel = function(channel, control, value) {
    // check if the Sampler Volume is at Zero and if so hide the sampler bank
    if (value > 0) {
        engine.setValue("[Samplers]", "show_samplers", true);
    } else {
        engine.setValue("[Samplers]", "show_samplers", false);
    }
    //control the 8 sampler volumes with the one knob on the mixer
    for (var i = 1; i <= 8; i++) {
        engine.setValue("[Sampler"+i+"]", "pregain", script.absoluteNonLin(value, 0, 1.0, 4.0));
    }
};

// PAD Mode Hot Cue
MC7000.padModeCue = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = true;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = false;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = false;

    // change PAD color when switching to Hot Cue Mode
    for (var i = 1; i <= 8; i++) {
        if (engine.getValue(group, "hotcue_" + i + "_enabled", true)) {
            midi.sendShortMsg(0x94 + deckOffset, 0x14 + i - 1, MC7000.padColor.hotcueon);
        } else {
            midi.sendShortMsg(0x94 + deckOffset, 0x14 + i - 1, MC7000.padColor.hotcueoff);
        }
    }
};

// PAD Mode Cue Loop
MC7000.padModeCueLoop = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = true;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = false;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = false;

    // switch off PAD illumination
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.alloff);
    }
};

// PAD Mode Flip
MC7000.padModeFlip = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = true;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = false;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = false;

    // switch off PAD illumination
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0x94 + deckOffset, 0x1C + i, MC7000.padColor.alloff);
    }
};

// PAD Mode Roll
MC7000.padModeRoll = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = true;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = false;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = false;

    // change PAD color when switching to Roll Mode
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.rolloff);
    }
};

// PAD Mode Saved Loop
MC7000.padModeSavedLoop = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = true;
    MC7000.PADModeSlicer[deckNumber] = false;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = false;

    // change PAD color when switching to Saved Loop Mode
    for (var i = 0; i < 8; i++) {
        var activeLED = engine.getValue(group, "beatloop_" + MC7000.fixedLoop[i] + "_enabled") ? MC7000.padColor.fixedloopon : MC7000.padColor.fixedloopoff;
        midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, activeLED);
    }
};

// PAD Mode Slicer
MC7000.padModeSlicer = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = true;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = false;

    // change PAD color when switching to Slicer Mode
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.sliceron);
    }
};

// PAD Mode Slicer Loop
MC7000.padModeSlicerLoop = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = false;
    MC7000.PADModeSlicerLoop[deckNumber] = true;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = false;

    // switch off PAD illumination
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.alloff);
    }
};

// PAD Mode Sampler
MC7000.padModeSampler = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = false;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = true;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = false;

    // change PAD color when switching to Sampler Mode
    for (var i = 1; i <= 8; i++) {
        if (engine.getValue("[Sampler" + i + "]", "play")) {
            midi.sendShortMsg(0x94 + deckOffset, 0x14 + i - 1, MC7000.padColor.samplerplay);
        } else if (engine.getValue("[Sampler" + i + "]", "track_loaded") === 0) {
            midi.sendShortMsg(0x94 + deckOffset, 0x14 + i - 1, MC7000.padColor.sampleroff);
        } else if (engine.getValue("[Sampler" + i + "]", "track_loaded") === 1 && engine.getValue("[Sampler" + i + "]", "play") === 0) {
            midi.sendShortMsg(0x94 + deckOffset, 0x14 + i - 1, MC7000.padColor.samplerloaded);
        }
    }
};

// PAD Mode Velocity Sampler
MC7000.padModeVelSamp = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = false;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = true;
    MC7000.PADModePitch[deckNumber] = false;

    // switch off PAD illumination
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.alloff);
    }
};

// PAD Mode Pitch
MC7000.padModePitch = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADModeCue[deckNumber] = false;
    MC7000.PADModeCueLoop[deckNumber] = false;
    MC7000.PADModeFlip[deckNumber] = false;
    MC7000.PADModeRoll[deckNumber] = false;
    MC7000.PADModeSavedLoop[deckNumber] = false;
    MC7000.PADModeSlicer[deckNumber] = true;
    MC7000.PADModeSlicerLoop[deckNumber] = false;
    MC7000.PADModeSampler[deckNumber] = false;
    MC7000.PADModeVelSamp[deckNumber] = false;
    MC7000.PADModePitch[deckNumber] = true;

    // switch off PAD illumination
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.alloff);
        midi.sendShortMsg(0x94 + deckOffset, 0x1C + i, MC7000.padColor.alloff);
    }
};

// PAD buttons
MC7000.PadButtons = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    var i, j;

    // activate and clear Hot Cues
    if (MC7000.PADModeCue[deckNumber] && engine.getValue(group, "track_loaded") === 1) {
        for (i = 1; i <= 8; i++) {
            if (control === 0x14 + i - 1 && value >= 0x01) {
                engine.setValue(group, "hotcue_" + i + "_activate", true);
            } else {
                engine.setValue(group, "hotcue_" + i + "_activate", false);
            }
            if (control === 0x1C + i - 1 && value >= 0x01) {
                engine.setValue(group, "hotcue_" + i + "_clear", true);
                midi.sendShortMsg(0x94 + deckOffset, 0x1C + i - 1, MC7000.padColor.hotcueoff);
            }
        }
    } else if (MC7000.PADModeCueLoop[deckNumber]) {
        return;
    } else if (MC7000.PADModeFlip[deckNumber]) {
        return;
    } else if (MC7000.PADModeRoll[deckNumber]) {
        // TODO(all): check for actual beatloop_size and apply back after a PAD Roll
        i = control - 0x14;
        if (control === 0x14 + i && value > 0x00) {
            engine.setValue(group, "beatlooproll_" + MC7000.beatLoopRoll[i] + "_activate", true);
            midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.rollon);
        } else if (control === 0x14 + i && value === 0x00) {
            engine.setValue(group, "beatlooproll_activate", false);
            midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.rolloff);
        }
    } else if (MC7000.PADModeSavedLoop[deckNumber]) {
        if (value === 0x00) {
            return; // don't respond to note off messages
        }
        i = control - 0x14;
        engine.setValue(group, "beatloop_" + MC7000.fixedLoop[i] + "_toggle", true);
        for (j =0; j < 8; j++) {
            var activeLED = engine.getValue(group, "beatloop_" + MC7000.fixedLoop[j] + "_enabled") ? MC7000.padColor.fixedloopon : MC7000.padColor.fixedloopoff;
            midi.sendShortMsg(0x94 + deckOffset, 0x14 + j, activeLED);
        }
    } else if (MC7000.PADModeSlicer[deckNumber]) {
        if (value > 0) {
            i = control - 0x14; // unshifted button
            j = control - 0x1C; // shifted button
            // forward buttons (PAD buttons upper row)
            if (control >= 0x14 && control <= 0x17) {
                engine.setValue(group, "beatjump_" + MC7000.beatJump[i] + "_forward", true);
                midi.sendShortMsg(0x94 + deckOffset, control, MC7000.padColor.slicerJumpFwd);
            // backward buttons (PAD buttons lower row)
            } else if (control >= 0x18 && control <= 0x1B) {
                engine.setValue(group, "beatjump_" + MC7000.beatJump[i - 4] + "_backward", true);
                midi.sendShortMsg(0x94 + deckOffset, control, MC7000.padColor.slicerJumpBack);
            // forward buttons (PAD buttons upper row - shifted controls)
            } else if (control >= 0x1C && control <= 0x1F) {
                engine.setValue(group, "beatjump_" + MC7000.beatJump[j + 4] + "_forward", true);
                midi.sendShortMsg(0x94 + deckOffset, control, MC7000.padColor.slicerJumpFwd);
            // backward buttons (PAD buttons lower row - shifted controls)
            } else if (control >= 0x20 && control <= 0x23) {
                engine.setValue(group, "beatjump_" + MC7000.beatJump[j] + "_backward", true);
                midi.sendShortMsg(0x94 + deckOffset, control, MC7000.padColor.slicerJumpBack);
            }
        } else {
            midi.sendShortMsg(0x94 + deckOffset, control, MC7000.padColor.sliceron);
        }
    } else if (MC7000.PADModeSlicerLoop[deckNumber]) {
        return;
    } else if (MC7000.PADModeSampler[deckNumber]) {
        for (i = 1; i <= 8; i++) {
            if (control === 0x14 + i - 1 && value >= 0x01) {
                if (engine.getValue("[Sampler" + i + "]", "track_loaded") === 0) {
                    engine.setValue("[Sampler" + i + "]", "LoadSelectedTrack", 1);
                } else if (engine.getValue("[Sampler" + i + "]", "track_loaded") === 1) {
                    engine.setValue("[Sampler" + i + "]", "cue_gotoandplay", 1);
                }
            } else if (control === 0x1C + i - 1 && value >= 0x01) {
                if (engine.getValue("[Sampler" + i + "]", "play") === 1) {
                    engine.setValue("[Sampler" + i + "]", "cue_gotoandstop", 1);
                    midi.sendShortMsg(0x94 + deckOffset, 0x1C + i - 1, MC7000.padColor.samplerloaded);
                } else {
                    engine.setValue("[Sampler" + i + "]", "eject", 1);
                    midi.sendShortMsg(0x94 + deckOffset, 0x1C + i - 1, MC7000.padColor.sampleroff);
                    engine.setValue("[Sampler" + i + "]", "eject", 0);
                }
            }
        }
    } else if (MC7000.PADModeVelSamp[deckNumber]) {
        return;
    } else if (MC7000.PADModePitch[deckNumber]) {
        return;
    }
};

// Shift Button
MC7000.shiftButton = function(channel, control, value, status, group) {
    var deckOffset = script.deckFromGroup(group) - 1;
    MC7000.shift[deckOffset] = ! MC7000.shift[deckOffset];
    midi.sendShortMsg(0x90 + deckOffset, 0x32,
        MC7000.shift[deckOffset] ? 0x7F : 0x01);
};

// Toggle Vinyl Mode
MC7000.vinylModeToggle = function(channel, control, value, status, group) {
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    var deckOffset = script.deckFromGroup(group) - 1;
    MC7000.isVinylMode[deckOffset] = !MC7000.isVinylMode[deckOffset];
    midi.sendShortMsg(0x90 + deckOffset, 0x07,
        MC7000.isVinylMode[deckOffset] ? 0x7F : 0x01);
};

// Use select button to load and eject track from deck
MC7000.loadLongPress = false;
MC7000.loadTimer = 0;

MC7000.loadAssertLongPress = function() {
    MC7000.loadLongPress = true;
    MC7000.loadTimer = 0;
};

MC7000.loadDown = function() {
    MC7000.loadLongPress = false;
    MC7000.loadTimer = engine.beginTimer(500, MC7000.loadAssertLongPress, true);
};

MC7000.loadUp = function(group) {
    if (MC7000.loadTimer !== 0) {
        engine.stopTimer(MC7000.loadTimer);
        MC7000.loadTimer = 0;
    }
    if (MC7000.loadLongPress) {
        script.triggerControl(group, "eject", 100);
    } else {
        script.triggerControl(group, "LoadSelectedTrack", 100);
    }
};

MC7000.loadButton = function(channel, control, value, status, group) {
    //LOAD hold <500ms: load track, >500ms: eject
    if (value === 0x7F) {
        MC7000.loadDown();
    } else {
        MC7000.loadUp(group);
    }
};

// The button that enables/disables scratching
MC7000.wheelTouch = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (MC7000.isVinylMode[deckOffset]) {
        if (value === 0x7F) {
            engine.scratchEnable(deckNumber, MC7000.jogWheelTicksPerRevolution,
                MC7000.scratchParams.recordSpeed,
                MC7000.scratchParams.alpha,
                MC7000.scratchParams.beta);
        } else {
            engine.scratchDisable(deckNumber);
        }
    }
};

// The wheel that actually controls the scratching
MC7000.wheelTurn = function(channel, control, value, status, group) {
    // TODO(all): check for latency and use it to normalize the jog factor so jog wont be
    // depending on audio latency anymore.

    // A: For a control that centers on 0:
    var numTicks = (value < 0x64) ? value : (value - 128);
    var adjustedSpeed = numTicks * MC7000.jogSensitivity * 25;
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (engine.isScratching(deckNumber)) {
    // Scratch!
        engine.scratchTick(deckNumber, numTicks * MC7000.jogSensitivity);
    } else {
        if (MC7000.shift[deckOffset]) {
            // While Shift Button pressed -> Search through track
            var jogSearch = 300 * adjustedSpeed / MC7000.jogWheelTicksPerRevolution;
            engine.setValue(group, "jog", jogSearch);
        } else {
            // While Shift Button released -> Pitch Bend
            var jogDelta = adjustedSpeed / MC7000.jogWheelTicksPerRevolution;
            var jogAbsolute = jogDelta + engine.getValue(group, "jog");
            engine.setValue(group, "jog", jogAbsolute);
        }
    }
};

// Needle Search Touch detection
MC7000.needleSearchTouch = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (engine.getValue(group, "play")) {
        MC7000.needleSearchTouched[deckNumber] = MC7000.needleSearchPlay && (!!value);
    } else {
        MC7000.needleSearchTouched[deckNumber] = !!value;
    }
};

// Needle Search Touch while "SHIFT" button is pressed
MC7000.needleSearchTouchShift = function(channel, control, value, status,
    group) {
    var deckNumber = script.deckFromGroup(group);
    MC7000.needleSearchTouched[deckNumber] = !!value;
};

// Needle Search Position detection (MSB)
MC7000.needleSearchMSB = function(channel, control, value) {
    MC7000.needleDropMSB = value; // just defining rough position
};

// Needle Search Position detection (MSB + LSB)
MC7000.needleSearchStripPosition = function(channel, control, value, status,
    group) {
    var deckNumber = script.deckFromGroup(group);
    if (MC7000.needleSearchTouched[deckNumber]) {
        var fullValue = (MC7000.needleDropMSB << 7) + value; // move MSB 7 binary digits to the left and add LSB
        var position = (fullValue / 0x3FFF); // divide by all possible positions to get relative between 0 - 1
        engine.setParameter(group, "playposition", position);
    }
};

// Pitch Fader (MSB)
MC7000.pitchFaderMSB = function(channel, control, value) {
    MC7000.pitchMSB = value; // just defining rough position
};

// Pitch Fader Position (MSB + LSB)
MC7000.pitchFaderPosition = function(channel, control, value, status, group) {
    var fullValue = (MC7000.pitchMSB << 7) + value;
    var position = 1 - (fullValue / 0x3FFF); // 1 - () to turn around the direction
    engine.setParameter(group, "rate", position);
};

// Next Rate range toggle
MC7000.nextRateRange = function(midichan, control, value, status, group) {
    if (value === 0) {
        return; // don't respond to note off messages
    }
    var deckOffset = script.deckFromGroup(group) - 1;
    // increment currentRateRangeIndex and check for overflow
    if (++MC7000.currentRateRangeIndex[deckOffset] ===
      MC7000.rateRanges.length) {
        MC7000.currentRateRangeIndex[deckOffset] = 0;
    }
    engine.setValue(group, "rateRange", MC7000.rateRanges[MC7000.currentRateRangeIndex[deckOffset]]);
};

// Previous Rate range toggle
MC7000.prevRateRange = function(midichan, control, value, status, group) {
    if (value === 0) {
        return; // don't respond to note off messages
    }
    var deckOffset = script.deckFromGroup(group) - 1;
    // decrement currentRateRangeIndex and check for underflow
    if (--MC7000.currentRateRangeIndex[deckOffset] < 0) {
        MC7000.currentRateRangeIndex[deckOffset] = MC7000.rateRanges.length - 1;
    }
    engine.setValue(group, "rateRange", MC7000.rateRanges[MC7000.currentRateRangeIndex[deckOffset]]);
};

// Key & Waveform zoom Select
MC7000.keySelect = function(midichan, control, value, status, group) {
    var deckOffset = script.deckFromGroup(group) - 1;
    // While Shift Button is pressed: Waveform Zoom
    if (MC7000.shift[deckOffset]) {
        if (value === 0x7F) {
            script.triggerControl(group, "waveform_zoom_up", 100);
        } else {
            script.triggerControl(group, "waveform_zoom_down", 100);
        }
    // While Shift Button is released: Key Select
    } else {
        if (value === 0x7F) {
            script.triggerControl(group, "pitch_down", 100);
        } else {
            script.triggerControl(group, "pitch_up", 100);
        }
    }
};

// Key & Waveform zoom Reset
MC7000.keyReset = function(channel, control, value, status, group) {
    var deckOffset = script.deckFromGroup(group) - 1;
    if (value === 0x00) {
        return;
    }
    // While Shift Button is pressed: Waveform Zoom Reset
    if (MC7000.shift[deckOffset]) {
        script.triggerControl(group, "waveform_zoom_set_default", 100);
    // While Shift Button is released: Key Reset
    } else {
        script.triggerControl(group, "reset_key", 100);
    }
};

// Assign Channel to Crossfader
MC7000.crossfaderAssign = function(channel, control, value, status, group) {
    if (value === 0x00) {
        engine.setValue(group, "orientation", 1); // Centre position
    } else if (value === 0x01) {
        engine.setValue(group, "orientation", 0); // Right position
    } else if (value === 0x02) {
        engine.setValue(group, "orientation", 2); // Left position
    }
};

// Assign Spinback length to STOP TIME knob
MC7000.stopTime = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    // "factor" for engine.brake()
    // this formula produces factors between 31 (min STOP TIME for ca 7 sec back
    // in track) and 1 (max STOP TIME for ca 18.0 sec back in track)
    MC7000.factor[deckNumber] = (1.1 - (value / 127)) * 30 - 2;
};

// Use SHIFT + CENSOR button as Spinback with STOP TIME adjusted length
MC7000.reverse = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value > 0) {
        // while the button is pressed spin back
        engine.brake(deckNumber, true, MC7000.factor[deckNumber], - 15); // start at a rate of -15 and decrease by "factor"
    } else {
        // when releasing the button the track starts softly again
        engine.softStart(deckNumber, true, MC7000.factor[deckNumber]);
    }
};

// Use of Reverse w/ and w/o Slip mode
MC7000.censor = function(channel, control, value, status, group) {
    if (engine.getValue(group, "slip_enabled")) {
        // This would be the "normal" CENSOR function"
        if (value > 0) {
            engine.setValue(group, "reverseroll", 1);
        } else {
            engine.setValue(group, "reverseroll", 0);
        }
    } else {
        // reverse play while button pressed
        if (value > 0) {
            engine.setValue(group, "reverse", 1);
        } else {
            engine.setValue(group, "reverse", 0);
        }
    }
};

// Set Crossfader Curve
MC7000.crossFaderCurve = function(control, value) {
    script.crossfaderCurve(value);
};

// Set FX wet/dry value
MC7000.fxWetDry = function(channel, control, value, status, group) {
    var numTicks = (value < 0x64) ? value : (value - 128);
    var newVal = engine.getValue(group, "mix") + numTicks/64*2;
    engine.setValue(group, "mix", Math.max(0, Math.min(1, newVal)));
};

// Sort the library for Artist, Title, BPM and Key
MC7000.sortLibrary = function(channel, control, value) {
    if (value === 0) {
        return;
    }
    var sortColumn;
    switch (control) {
    case 0x12:  // TITLE
        sortColumn = 2;
        break;
    case 0x13:  // BPM
        sortColumn = 15;
        break;
    case 0x14:  // ARTIST
        sortColumn = 1;
        break;
    case 0x20:  // KEY
        sortColumn = 20;
        break;
    }
    engine.setValue("[Library]", "sort_column_toggle", sortColumn);
};

/* LEDs for VuMeter */
// VuMeters only for Channel 1-4 / Master is on Hardware
MC7000.VuMeter = function(value, group) {
    var deckOffset = script.deckFromGroup(group) - 1;
    // sends either PeakIndicator or scales value (0..1) to (0..117) while truncating to each LED
    var vuLevelOutValue = engine.getValue(group, "PeakIndicator") ? MC7000.VuMeterLEDPeakValue : Math.floor(Math.pow(value, 2.5) * 9) * 13;
    // only send Midi signal when LED value has changed
    if (MC7000.prevVuLevel[deckOffset] !== vuLevelOutValue) {
        midi.sendShortMsg(0xB0 + deckOffset, 0x1F, vuLevelOutValue);
        MC7000.prevVuLevel[deckOffset] = vuLevelOutValue;
    }
};

// Spinning Platter LEDs & Slicer Loop PAD LEDs as beat counter
// pulled together for the calculation to be done only once and then
// send LED signals to Jog or PAD LEDs when needed.
MC7000.TrackPositionLEDs = function(value, group) {
    // do nothing before track starts
    if (value === 0) {
        return;
    }
    // lets define some variables first
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    var trackDuration = engine.getValue(group, "duration"); // in seconds
    var beatLength = engine.getValue(group, "file_bpm") / 60; // in Beats Per Seconds
    var cuePosition = engine.getValue(group, "cue_point") / engine.getValue(group, "track_samplerate") / 2; // in seconds
    var playPosition = value * trackDuration; // in seconds
    var jogLEDPosition = playPosition / 60 * MC7000.scratchParams.recordSpeed;
    var jogLEDNumber = 48; // LED ring contains 48 segments each triggered by the next even Midi value
    // check for Vinyl Mode and decide to spin the Jog LED or show play position
    var activeJogLED = MC7000.isVinylMode[deckOffset] ? Math.round(jogLEDPosition * jogLEDNumber) % jogLEDNumber : Math.round(value * jogLEDNumber);
    // count the beats (1 to 8) after the CUE point
    var beatCountLED = (Math.floor((playPosition - cuePosition) * beatLength) % 8); //calculate PAD LED position

    // TODO(all): check for playposition < (trackduration - warning length) for sending position signals
    // check if a Jog LED has changed and if so then send the signal to the next Jog LED
    if (MC7000.prevJogLED[deckOffset] !== activeJogLED) {
        midi.sendShortMsg(0x90 + deckOffset, 0x06, activeJogLED * 2); // only each 2nd midi signal triggers the next LED
        MC7000.prevJogLED[deckOffset] = activeJogLED;
    }
    // TODO(all): else blink the platter LEDs

    // check if Slicer mode is active and illuminate PAD LEDs counting with the beat while playing
    if (!MC7000.experimental) {
        return;
    }
    if (MC7000.PADModeSlicer[deckNumber]) {
        // only send new LED status when beatCountLED really changes
        if (MC7000.prevPadLED[deckOffset] !== beatCountLED) {
            // first set all LEDs to default color incl shifted
            for (var i = 0; i < 16; i++) {
                midi.sendShortMsg(0x94 + deckOffset, 0x14 + i, MC7000.padColor.sliceron);
            }
            // now chose which PAD LED to turn on (+8 means shifted PAD LEDs)
            if (beatCountLED === 0) {
                midi.sendShortMsg(0x94 + deckOffset, 0x14, MC7000.padColor.hotcueon);
                midi.sendShortMsg(0x94 + deckOffset, 0x14 + 8, MC7000.padColor.hotcueon);
            } else if (beatCountLED === 7) {
                midi.sendShortMsg(0x94 + deckOffset, 0x1B, MC7000.padColor.hotcueon);
                midi.sendShortMsg(0x94 + deckOffset, 0x1B + 8, MC7000.padColor.hotcueon);
            } else if (beatCountLED > 0 && beatCountLED < 7) {
                midi.sendShortMsg(0x94 + deckOffset, 0x14 + beatCountLED, MC7000.padColor.hotcueon);
                midi.sendShortMsg(0x94 + deckOffset, 0x14 + 8 + beatCountLED, MC7000.padColor.hotcueon);
            }
        }
        MC7000.prevPadLED[deckOffset] = beatCountLED;
    }
};

// initial HotCue LED when loading a track with already existing hotcues
MC7000.HotCueLED = function(value, group) {
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    if (MC7000.PADModeCue[deckNumber]) {
        for (var i = 1; i <= 8; i++) {
            if (value === 1) {
                if (engine.getValue(group, "hotcue_"+i+"_enabled") === 1) {
                    midi.sendShortMsg(0x94 + deckOffset, 0x14 + i - 1, MC7000.padColor.hotcueon);
                }
            } else {
                if (engine.getValue(group, "hotcue_"+i+"_enabled") === 0) {
                    midi.sendShortMsg(0x94 + deckOffset, 0x14 + i - 1, MC7000.padColor.hotcueoff);
                }
            }
        }
    }
};

// Sampler LED
MC7000.SamplerLED = function() {
    for (var j = 1; j <= 4; j++) {
        for (var i = 1; i <= 8; i++) {
            if (MC7000.PADModeSampler[j]) {
                if (engine.getValue("[Sampler"+i+"]", "track_loaded") === 1) {
                    if (engine.getValue("[Sampler"+i+"]", "play") === 0) {
                        midi.sendShortMsg(0x94 + j - 1, 0x14 + i - 1, MC7000.padColor.samplerloaded);
                    } else {
                        midi.sendShortMsg(0x94 + j - 1, 0x14 + i - 1, MC7000.padColor.samplerplay);
                    }
                } else if (engine.getValue("[Sampler"+i+"]", "track_loaded") === 0) {
                    midi.sendShortMsg(0x94 + j - 1, 0x14 + i - 1, MC7000.padColor.sampleroff);
                }
            }
        }
    }
};

/* CONTROLLER SHUTDOWN */
MC7000.shutdown = function() {

    // Need to switch off LEDs one by one,
    // otherwise the controller cannot handle the signal traffic

    // Switch off Transport section LEDs
    for (var i = 0; i <= 3; i++) {
        midi.sendShortMsg(0x90 + i, 0x00, 0x01);
        midi.sendShortMsg(0x90 + i, 0x01, 0x01);
        midi.sendShortMsg(0x90 + i, 0x02, 0x01);
        midi.sendShortMsg(0x90 + i, 0x03, 0x01);
        midi.sendShortMsg(0x90 + i, 0x04, 0x01);
        midi.sendShortMsg(0x90 + i, 0x05, 0x01);
    }
    // Switch off Loop Section LEDs
    for (i = 0; i <= 3; i++) {
        midi.sendShortMsg(0x94 + i, 0x32, 0x01);
        midi.sendShortMsg(0x94 + i, 0x33, 0x01);
        midi.sendShortMsg(0x94 + i, 0x34, 0x01);
        midi.sendShortMsg(0x94 + i, 0x35, 0x01);
        midi.sendShortMsg(0x94 + i, 0x38, 0x01);
        midi.sendShortMsg(0x94 + i, 0x39, 0x01);
        // switch PAD Mode to CUE LED
        midi.sendShortMsg(0x94 + i, 0x00, 0x04);
    }
    // Switch all PAD LEDs to HotCue mode
    for (i = 0x14; i <= 0x1B; i++) {
        midi.sendShortMsg(0x94, i, 0x02);
        midi.sendShortMsg(0x95, i, 0x02);
        midi.sendShortMsg(0x96, i, 0x02);
        midi.sendShortMsg(0x97, i, 0x02);
    }
    // Switch off Channel Cue, VINYL, SLIP, KEY LOCK LEDs
    for (i = 0; i <= 3; i++) {
        midi.sendShortMsg(0x90 + i, 0x07, 0x01);
        midi.sendShortMsg(0x90 + i, 0x0F, 0x01);
        midi.sendShortMsg(0x90 + i, 0x0D, 0x01);
        midi.sendShortMsg(0x90 + i, 0x1B, 0x01);
    }
    // Switch off FX Section LEDs
    for (i = 0; i <= 1; i++) {
        midi.sendShortMsg(0x98 + i, 0x00, 0x01);
        midi.sendShortMsg(0x98 + i, 0x01, 0x01);
        midi.sendShortMsg(0x98 + i, 0x02, 0x01);
        midi.sendShortMsg(0x98 + i, 0x04, 0x01);
        midi.sendShortMsg(0x98 + i, 0x0A, 0x01);
        midi.sendShortMsg(0x98 + i, 0x05, 0x01);
        midi.sendShortMsg(0x98 + i, 0x06, 0x01);
        midi.sendShortMsg(0x98 + i, 0x07, 0x01);
        midi.sendShortMsg(0x98 + i, 0x08, 0x01);
    }
    // Reset Level Meters and JogLED
    for (i = 0; i <= 3; i++) {
        // Switch off Level Meters
        midi.sendShortMsg(0xB0 + i, 0x1F, 0x00);
        // Platter Ring: Reset JogLED to Zero position
        midi.sendShortMsg(0x90 + i, 0x06, 0x00);
        // Platter Ring: Switch all LEDs on
        midi.sendShortMsg(0x90 + i, 0x64, 0x00);
    }
};

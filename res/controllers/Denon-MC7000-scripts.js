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
 * Newer Kernels will surely also provide native audio support for this controller.
**/

var MC7000 = {};

/*///////////////////////////////////
//      USER VARIABLES BEGIN       //
///////////////////////////////////*/

// Wanna have Needle Search active while playing a track ?
// In any case Needle Search is available holding "SHIFT" down.
// can be true or false (recommended: false)
MC7000.needleSearchPlay = false;

// Possible pitchfader rateranges given in percent.
// can be cycled through be the RANGE buttons.
MC7000.rateRanges = [
    4/100,  // default: 4/100
    6/100,  // default: 6/100
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
    recordSpeed: 33.3, // default: 33.3
    alpha: (1.0/10),   // default: (1.0/10)
    beta: (1.0/10)/32  // default: (1.0/10)/32
};

// Sensitivity of the jog wheel (also depends on audio latency)
MC7000.jogParams = {
    // Lower values for less, higher values for more sensitive
    jogSensitivity: 30, // default: 30
    // this will limit the parameter of "jog" (keep between 0.5 and 3)
    maxJogValue: 3      // default: 3
};

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

// Beatloop Roll sizes
MC7000.beatLoopRoll = [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8];

// Define the MIDI signal for red LED at VU Meters
MC7000.VuMeterLEDPeakValue = 0x76;

// PAD Mode Colors
MC7000.padColor = {
    "alloff": 0x01,         // switch off completely
    // Hot Cue
    "hotcueon": 0x04,       // darkblue Hot Cue active
    "hotcueoff": 0x02,      // lightblue Hot Cue inactive
    // Cue Loop
    "cueloopon": 0x0D,      // Cueloop colour for activated cue point
    "cueloopoff": 0x1A,     // Cueloop colour inactive
    // Roll
    "rollon": 0x20,         // BeatloopRoll active colour
    "rolloff": 0x06,        // BeatloopRoll off colour
    // Slicer
    "sliceron": 0x11,       // activated Slicer
    "slicerJumpFwd": 0x31,  // Sliver forward jump
    "slicerJumpBack": 0x31, // Sliver backward jump
    // Sampler
    "samplerloaded": 0x38,  // dark pink Sampler loaded colour
    "samplerplay": 0x09,    // green Sampler playing
    "sampleroff": 0x12     // light pink Sampler standard colour
};

/* DECK INITIALIZATION */
MC7000.init = function() {

    // set default Master Volume to 85% to give a little head room for mixing
    // engine.setValue("[Master]", "gain", 0.85);

    // The SysEx message to send to the controller to force the midi controller
    // to send the status of every item on the control surface.
    var ControllerStatusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];

    // After midi controller receive this Outbound Message request SysEx
    // Message, midi controller will send the status of every item on the
    // control surface. (Mixxx will be initialized with current values)
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);

    // VU meters
    engine.makeConnection("[Channel1]", "VuMeter", MC7000.VuMeter);
    engine.makeConnection("[Channel2]", "VuMeter", MC7000.VuMeter);
    engine.makeConnection("[Channel3]", "VuMeter", MC7000.VuMeter);
    engine.makeConnection("[Channel4]", "VuMeter", MC7000.VuMeter);

    // Platter Ring LED
    midi.sendShortMsg(0x90, 0x64, MC7000.modeSingleLED);
    midi.sendShortMsg(0x91, 0x64, MC7000.modeSingleLED);
    midi.sendShortMsg(0x92, 0x64, MC7000.modeSingleLED);
    midi.sendShortMsg(0x93, 0x64, MC7000.modeSingleLED);
    engine.makeConnection("[Channel1]", "playposition", MC7000.JogLed);
    engine.makeConnection("[Channel2]", "playposition", MC7000.JogLed);
    engine.makeConnection("[Channel3]", "playposition", MC7000.JogLed);
    engine.makeConnection("[Channel4]", "playposition", MC7000.JogLed);

    // Vinyl mode LEDs
    midi.sendShortMsg(0x90, 0x07, MC7000.isVinylMode ? 0x7F: 0x01);
    midi.sendShortMsg(0x91, 0x07, MC7000.isVinylMode ? 0x7F: 0x01);
    midi.sendShortMsg(0x92, 0x07, MC7000.isVinylMode ? 0x7F: 0x01);
    midi.sendShortMsg(0x93, 0x07, MC7000.isVinylMode ? 0x7F: 0x01);

    // HotCue Mode LEDs
    for (var i = 1; i <= 8; i++) {
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

    // Sampler Volume Control
    MC7000.samplerLevel = function(channel, control, value) {
        // check if the Sampler Volume is at Zero and if so hide the sampler bank
        if (value > 0x00) {
            engine.setValue("[Samplers]", "show_samplers", true);
        } else {
            engine.setValue("[Samplers]", "show_samplers", false);
        }
        // get the Sampler Rows opened with its details
        engine.setValue("[SamplerRow1]", "expanded", true);
        engine.setValue("[SamplerRow2]", "expanded", true);

        //control up to 16 sampler volumes with the one knob on the mixer
        for (var i = 1; i <= 16; i++) {
            engine.setValue("[Sampler"+i+"]", "pregain", script.absoluteNonLin(value, 0, 1.0, 4.0));
        }
    };

    // Activate Softtakeover for Pitch Faders
    for (i = 1; i <= 4; i++) {
        engine.softTakeover("[Channel" + i + "]", "rate", true);
    }
};

// PAD Mode Hot Cue
MC7000.padModeCue = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
    // set HotCue Mode true
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
    }
    // change PAD color when switching to Hot Cue Mode
    for (var i = 1; i <= 8; i++) {
        if (engine.getValue(group, "hotcue_" + i + "_enabled", true)) {
            midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
                MC7000.padColor.hotcueon);
        } else {
            midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
                MC7000.padColor.hotcueoff);
        }
    }
};
// PAD Mode Cue Loop
MC7000.padModeCueLoop = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
            MC7000.padColor.alloff);
    }
};
// PAD Mode Flip
MC7000.padModeFlip = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x1C + i - 1,
            MC7000.padColor.alloff);
    }
};
// PAD Mode Roll
MC7000.padModeRoll = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
            MC7000.padColor.rolloff);
    }
};
// PAD Mode Saved Loop
MC7000.padModeSavedLoop = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
            MC7000.padColor.alloff);
    }
};
// PAD Mode Slicer
MC7000.padModeSlicer = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
            MC7000.padColor.sliceron);
    }
};
// PAD Mode Slicer Loop
MC7000.padModeSlicerLoop = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
            MC7000.padColor.alloff);
    }
};
// PAD Mode Sampler
MC7000.padModeSampler = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    // change PAD color when switching to Sampler Mode
    for (var i = 1; i <= 8; i++) {
        if (engine.getValue("[Sampler" + i + "]", "play")) {
            midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
                MC7000.padColor.samplerplay);
        } else if (engine.getValue("[Sampler" + i + "]", "track_loaded") === 0) {
            midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
                MC7000.padColor.sampleroff);
        } else if (engine.getValue("[Sampler" + i + "]", "track_loaded") === 1 &&
               engine.getValue("[Sampler" + i + "]", "play") === 0) {
            midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
                MC7000.padColor.samplerloaded);
        }
    }
};
// PAD Mode Velocity Sampler
MC7000.padModeVelSamp = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
            MC7000.padColor.alloff);
    }
};
// PAD Mode Pitch
MC7000.padModePitch = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value === 0x00)
        return; // don't respond to note off messages
    if (value === 0x7F) {
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
    }
    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1,
            MC7000.padColor.alloff);
        midi.sendShortMsg(0x94 + deckNumber - 1, 0x1C + i - 1,
            MC7000.padColor.alloff);
    }
};

// PAD buttons
MC7000.PadButtons = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);

    // activate and clear Hot Cues
    if (MC7000.PADModeCue[deckNumber] &&
      engine.getValue(group, "track_loaded") === 1) {
        for (var i = 1; i <= 8; i++) {
            if (control === 0x14 + i - 1 && value >= 0x01) {
                engine.setValue(group, "hotcue_" + i + "_activate", true);
            } else {
                engine.setValue(group, "hotcue_" + i + "_activate", false);
            }
            if (control === 0x1C + i - 1 && value >= 0x01) {
                engine.setValue(group, "hotcue_" + i + "_clear", true);
                midi.sendShortMsg(0x94 + deckNumber - 1, 0x1C + i - 1,
                    MC7000.padColor.hotcueoff);
            }
        }
    } else if (MC7000.PADModeCueLoop[deckNumber]) {
        return;
    } else if (MC7000.PADModeFlip[deckNumber]) {
        return;
    } else if (MC7000.PADModeRoll[deckNumber]) {
        // todo:
        // check for actual beatloop_size and apply back after a PAD Roll
        i = control - 0x14;
        if (control === 0x14 + i && value > 0x00) {
            engine.setValue(
                group, "beatlooproll_" + MC7000.beatLoopRoll[i] + "_activate",
                true);
            midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i,
                MC7000.padColor.rollon);
        } else if (control === 0x14 + i && value === 0x00) {
            engine.setValue(group, "beatlooproll_activate", false);
            midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i,
                MC7000.padColor.rolloff);
        }
    } else if (MC7000.PADModeSavedLoop[deckNumber]) {
        return;
    } else if (MC7000.PADModeSlicer[deckNumber]) {
        if (value > 0) {
            var beats = 1 << (control % 4);
            if (control > 0x17) {
                engine.setValue(group, "beatjump_" + beats + "_backward", value);
                midi.sendShortMsg(0x94 + deckNumber - 1, control,
                    MC7000.padColor.slicerJumpBack);
            } else {
                engine.setValue(group, "beatjump_" + beats + "_forward", value);
                midi.sendShortMsg(0x94 + deckNumber - 1, control,
                    MC7000.padColor.slicerJumpFwd);
            }
        } else {
            midi.sendShortMsg(0x94 + deckNumber - 1, control,
                MC7000.padColor.sliceron);
        }
    } else if (MC7000.PADModeSlicerLoop[deckNumber]) {
        return;
    } else if (MC7000.PADModeSampler[deckNumber]) {
        for (i = 1; i <= 8; i++) {
            if (control === 0x14 + i - 1 && value >= 0x01) {
                if (engine.getValue("[Sampler" + i + "]", "track_loaded") === 0) {
                    engine.setValue("[Sampler" + i + "]", "LoadSelectedTrack", 1);
                } else if (engine.getValue("[Sampler" + i + "]", "track_loaded") ===
                     1) {
                    engine.setValue("[Sampler" + i + "]", "cue_gotoandplay", 1);
                }
            } else if (control === 0x1C + i - 1 && value >= 0x01) {
                if (engine.getValue("[Sampler" + i + "]", "play") === 1) {
                    engine.setValue("[Sampler" + i + "]", "cue_gotoandstop", 1);
                    midi.sendShortMsg(0x94 + deckNumber - 1, 0x1C + i - 1,
                        MC7000.padColor.samplerloaded);
                } else {
                    engine.setValue("[Sampler" + i + "]", "eject", 1);
                    midi.sendShortMsg(0x94 + deckNumber - 1, 0x1C + i - 1,
                        MC7000.padColor.sampleroff);
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

// Toggle Vinyl Mode
MC7000.vinylModeToggle = function(channel, control, value, status, group) {
    if (value === 0x00)
        return; // don't respond to note off messages

    if (value === 0x7F) {
        var deckNumber = script.deckFromGroup(group);
        MC7000.isVinylMode[deckNumber] = !MC7000.isVinylMode[deckNumber];
        midi.sendShortMsg(0x90 + channel, 0x07,
            MC7000.isVinylMode[deckNumber] ? 0x7F : 0x01);
    }
};

// The button that enables/disables scratching
MC7000.wheelTouch = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (MC7000.isVinylMode[deckNumber]) {
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
    // A: For a control that centers on 0:
    var numTicks = (value < 0x64) ? value : (value - 128);
    var deckNumber = script.deckFromGroup(group);
    if (engine.isScratching(deckNumber)) {
    // Scratch!
        engine.scratchTick(deckNumber, numTicks);
    } else {
    // Pitch bend
        var jogDelta = numTicks / MC7000.jogWheelTicksPerRevolution *
                   MC7000.jogParams.jogSensitivity;
        var jogAbsolute = jogDelta + engine.getValue(group, "jog");
        engine.setValue(
            group, "jog",
            Math.max(-MC7000.jogParams.maxJogValue,
                Math.min(MC7000.jogParams.maxJogValue, jogAbsolute)));
    }
};

// Needle Search Touch detection
MC7000.needleSearchTouch = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (engine.getValue(group, "play")) {
        MC7000.needleSearchTouched[deckNumber] =
        MC7000.needleSearchPlay && (!!value);
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
        var fullValue = (MC7000.needleDropMSB << 7) +
                    value; // move MSB 7 binary gigits to the left and add LSB
        var position = (fullValue / 0x3FFF); // divide by all possible positions to
        // get relative between 0 - 1
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
    var position =
      1 - (fullValue / 0x3FFF); // 1 - () to turn around the direction
    engine.setParameter(group, "rate", position);
};

// Next Rate range toggle
MC7000.nextRateRange = function(midichan, control, value, status, group) {
    if (value === 0)
        return; // don't respond to note off messages
    var deckNumber = script.deckFromGroup(group);
    // increment currentRateRangeIndex and check for overflow
    if (++MC7000.currentRateRangeIndex[deckNumber - 1] ===
      MC7000.rateRanges.length) {
        MC7000.currentRateRangeIndex[deckNumber - 1] = 0;
    }
    engine.setValue(
        group, "rateRange",
        MC7000.rateRanges[MC7000.currentRateRangeIndex[deckNumber - 1]]);
};

// Previous Rate range toggle
MC7000.prevRateRange = function(midichan, control, value, status, group) {
    if (value === 0)
        return; // don't respond to note off messages
    var deckNumber = script.deckFromGroup(group);
    // decrement currentRateRangeIndex and check for underflow
    if (--MC7000.currentRateRangeIndex[deckNumber - 1] < 0) {
        MC7000.currentRateRangeIndex[deckNumber - 1] = MC7000.rateRanges.length - 1;
    }
    engine.setValue(
        group, "rateRange",
        MC7000.rateRanges[MC7000.currentRateRangeIndex[deckNumber - 1]]);
};

// Key Select
MC7000.keySelect = function(midichan, control, value, status, group) {
    if (value === 0x01) {
        engine.setValue(group, "pitch_up", true);
    } else if (value === 0x7F) {
        engine.setValue(group, "pitch_down", true);
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

// Use the CENSOR button as Spinback with STOP TIME adjusted length
MC7000.censor = function(channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    var deck =
      parseInt(group.substring(8, 9)); // work out which deck we are using
    engine.brake(deck, value > 0, MC7000.factor[deckNumber],
        -15); // start at a rate of -15 and decrease by "factor"
};

/* SET CROSSFADER CURVE */
MC7000.crossFaderCurve = function(control, value) {
    script.crossfaderCurve(value);
};

/* Set FX wet/dry value */
MC7000.fxWetDry = function(midichan, control, value, status, group) {
    var numTicks = (value < 0x64) ? value: (value - 128);
    var newVal = engine.getValue(group, "mix") + numTicks/64*2;
    engine.setValue(group, "mix", Math.max(0, Math.min(1, newVal)));


};

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
    var VULevelOutValue = engine.getValue(group, "PeakIndicator") ? MC7000.VuMeterLEDPeakValue : value*value*value*value*0x69,
        deckNumber = script.deckFromGroup(group);

    midi.sendShortMsg(0xB0 + deckNumber - 1, 0x1F, VULevelOutValue);
};

/* LEDs around Jog wheel */
MC7000.JogLed = function(value, group) {
    var deckNumber = script.deckFromGroup(group);
    // do nothing before track starts
    if (value < 0) return;

    var trackDuration = engine.getValue(group, "duration"),
        position = value * trackDuration / 60 * MC7000.scratchParams.recordSpeed,
        // LED ring contains 48 segments with each LED activated by the next even number
        LEDmidiSignal = 48 * 2,
        activeLED = MC7000.isVinylMode[deckNumber] ? Math.round(position * LEDmidiSignal) % LEDmidiSignal : value * LEDmidiSignal;

    midi.sendShortMsg(0x90 + deckNumber -1, 0x06, activeLED);
};

// initial HotCue LED when loading a track with already existing hotcues
MC7000.HotCueLED = function(value, group) {
    var deckNumber = script.deckFromGroup(group);
    if (MC7000.PADModeCue[deckNumber]) {
        for (var i = 1; i <= 8; i++) {
            if (value === 1) {
                if (engine.getValue(group, "hotcue_"+i+"_enabled") === 1) {
                    midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1, MC7000.padColor.hotcueon);
                }
            } else {
                if (engine.getValue(group, "hotcue_"+i+"_enabled") === 0) {
                    midi.sendShortMsg(0x94 + deckNumber - 1, 0x14 + i - 1, MC7000.padColor.hotcueoff);
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

/*
 * Denon DJ MC7000 DJ controller script for Mixxx 2.3.1
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
 */

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
// can be true or false
MC7000.needleSearchPlay = false;

// select if the previous sampler shall stop before a new sampler starts
// true: a running sampler will stop before the new sampler starts
// false: all triggered samplers will play simultaneously
MC7000.prevSamplerStop = engine.getSetting("prevSamplerStop") ?? true;

// Quantity of Samplers used in mixxx possible values 16 and 32
// To use 32 samplers instead of 16 you can set the user variable
// MC7000.SamplerQty to 32. Deck 1 will trigger sampler 1 to 8,
// Deck 2 will trigger sampler 9 to 16, Deck 3 will trigger
// sampler 17 to 24 and Deck 4 will trigger sampler 25 to 32.
// Please note that your Mixxx skin needs to support more than 16 samplers.
MC7000.SamplerQty = parseInt(engine.getSetting("samplerQty") ?? "16");

// Set Vinyl Mode on ("true") or off ("false") when MIXXX starts.
// This sets the Jog Wheel touch detection / Vinyl Mode
// and the Jog LEDs ("VINYL" on = spinny, "VINYL" off = track position).
MC7000.VinylModeOn = true;

// Possible pitchfader rate ranges given in percent.
// can be cycled through by the RANGE buttons.
// All default values are the same as selectable in Mixxx Preferences
MC7000.rateRanges = [
    4/100,
    6/100,
    8/100,
    10/100,
    16/100,
    24/100,
    50/100,
    90/100,
];

// Platter Ring LED mode
// Mode 0 = Single "off" LED chase (all others "on")
// Mode 1 = Single "on" LED chase (all others "off")
// use "SHIFT" + "DECK #" to toggle between both modes
MC7000.modeSingleLED = 1;

// Scratch algorithm parameters
MC7000.scratchParams = {
    recordSpeed: 33 + 1/3,
    alpha: (1.0/10),
    beta: (1.0/10)/32
};

// Jog wheel parameters
MC7000.jogParams = {
    // Sensitivity factor of the jog wheel (also depends on audio latency)
    // 0.5 for half, 2 for double sensitivity - Recommendation:
    // set to 0.5 with audio buffer set to 50ms
    // set to 1 with audio buffer set to 25ms
    // set to 3 with audio buffer set to 5ms
    sensitivity: engine.getSetting("jogSensitivity") || 1,
    // Acceleration settings for the jog wheel in vinyl mode
    // If enabled, the track speed will accelerate faster than the physical jogwheel movement. Be aware that the absolute track position will drift relative to the jogwheel position in this mode!
    // (exponent: 0 and coefficient: 1 = no acceleration)
    acceleration: {
        // Toggles acceleration entirely.
        enabled: engine.getSetting("jogAccelerationEnabled") || false,
        // Acceleration function exponent
        exponent: engine.getSetting("jogAccelerationExponent") || 0.8,
        // Acceleration function scaling factor
        coefficient: engine.getSetting("jogAccelerationCoefficient") || 1
    }
};

// Parameter button settings (the orange buttons at the bottom left/right of the controller).
MC7000.parameterButtonSettings = {
    // Parameter button mode. Available modes are `starsAndColor`, `beatjump` and `introOutro`.
    mode: engine.getSetting("parameterButtonMode") ?? "starsAndColor",
    // Whether to use the parameter buttons to change the pitch range during
    // pitch play mode. If this option is enabled, the pitch change
    // functionality overrides the normal parameter button mode during pitch play.
    parameterButtonPitchPlayOverrideEnabled: engine.getSetting("parameterButtonPitchPlayOverrideEnabled") ?? true,
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

// initialize the "spinnbackFactor" function for Spinback
MC7000.spinbackFactor = [];
// initialize the "brakeSoftstartFactor" function for Brake and Softstart
MC7000.brakeSoftstartFactor = [];

//Set Shift button state to false as default
MC7000.shift = [false, false, false, false];

// For each side whether the top or bottom deck is active.
MC7000.topDeckActive = [true, true];

// initialize the PAD Mode to Hot Cue when starting
MC7000.PADMode = ["Cue", "Cue", "Cue", "Cue"];

// PAD Mode 'Beatloop Roll' sizes
MC7000.beatLoopRoll = [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8];

// PAD Mode - 'Fixed Loop' sizes
MC7000.fixedLoop = [1, 2, 4, 8, 16, 32, 64, 128];

// PAD Mode - 'Beatjump' sizes
MC7000.beatJump = [1, 2, 4, 8, 16, 32, 64, 128];

// PAD Mode - "pitch" values
MC7000.halftoneToPadMap = [[4, 5, 6, 7, 0, 1, 2, 3], [4, 5, 6, 7, 0, 1, 2, 3], [4, 5, 6, 7, 0, 1, 2, 3], [4, 5, 6, 7, 0, 1, 2, 3]];

// Define the MIDI signal for red LED at VU Meters
MC7000.VuMeterLEDPeakValue = 0x76;

// initialize variables to compare LED status for VU, Jog and PAD LEDs
MC7000.prevVuLevel = [0, 0, 0, 0];
MC7000.prevJogLED = [0, 0, 0, 0];
MC7000.prevPadLED = [0, 0, 0, 0];

/*
Color Codes:
Colors are encoded using the following schema: Take the individual components of the color (R, G, B). Then use
the two most significant bits of that color (rr, gg, bb) and pack that into a 7-byte integer using the following schema `0b0rrggbb`. Then add 1 before sending to the controller.
*/

/*
button colors not set yet
slicer loop blue
flip red
*/

// PAD Mode Colors
MC7000.padColor = {
    "alloff": 0x01,         // switch off completely
    // Hot Cue
    "hotcueon": 0x04,       // blue Hot Cue active
    "hotcueoff": 0x02,      // dark blue Hot Cue inactive
    // Cue Loop
    "cueloopon": 0x19,      // green Cueloop colour for activated cue point
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
    "samplerplay": 0x09,    // middle green Sampler playing
    "sampleroff": 0x12,     // dark pink Sampler standard colour
    // Velocity Sampler
    "velsamploaded": 0x24,  // purple VelocitySampler loaded colour
    "velsampplay": 0x0A,    // light green VelocitySampler playing
    "velsampoff": 0x13,     // dark purple VelocitySampler standard colour
    // Pitch Play
    "pitchon": 0x0D,        // green for Pitch Play on
    "pitchoff": 0x05        // dark green for Pitch Play off
};

/* DECK INITIALIZATION */
MC7000.init = function() {

    // obtain all knob and slider positions
    const ControllerStatusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);

    // VU meters
    engine.makeUnbufferedConnection("[Channel1]", "vu_meter", MC7000.VuMeter);
    engine.makeUnbufferedConnection("[Channel2]", "vu_meter", MC7000.VuMeter);
    engine.makeUnbufferedConnection("[Channel3]", "vu_meter", MC7000.VuMeter);
    engine.makeUnbufferedConnection("[Channel4]", "vu_meter", MC7000.VuMeter);

    // Switch to active decks
    midi.sendShortMsg(MC7000.topDeckActive[0] ? 0x90 : 0x92, 0x08, 0x7F);
    midi.sendShortMsg(MC7000.topDeckActive[1] ? 0x91 : 0x93, 0x08, 0x7F);

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

    for (let chanIdx = 1; chanIdx <= 4; chanIdx++) {
        // HotCue Mode LEDs
        for (let cueIdx = 1; cueIdx <= 8; cueIdx++) {
            engine.makeConnection(`[Channel${chanIdx}]`, `hotcue_${cueIdx}_status`, MC7000.HotCueLED);
        }
        // Pitch LEDs
        for (let pitchIdx = 1; pitchIdx <= 8; pitchIdx++) {
            engine.makeConnection(`[Channel${chanIdx}]`, `hotcue_${pitchIdx}_status`, MC7000.PitchLED);
        }
    }
    // Sampler Mode LEDs and Velocity Sampler Mode LEDs
    if (engine.getValue("[App]", "num_samplers") < MC7000.SamplerQty) {
        engine.setValue("[App]", "num_samplers", MC7000.SamplerQty);
    }
    for (let samplerIdx = 1; samplerIdx <= MC7000.SamplerQty; samplerIdx++) {
        engine.makeConnection("[Sampler"+samplerIdx+"]", "track_loaded", MC7000.SamplerLED);
        engine.makeConnection("[Sampler"+samplerIdx+"]", "play", MC7000.SamplerLED);
        engine.makeConnection("[Sampler"+samplerIdx+"]", "track_loaded", MC7000.VelSampLED);
        engine.makeConnection("[Sampler"+samplerIdx+"]", "play", MC7000.VelSampLED);
    }
    // send Softtakeover delayed to avoid conflicts with ControllerStatusSysex
    engine.beginTimer(2000, () => {
        // Softtakeover for Pitch Faders only
        for (let chanIdx = 1; chanIdx <= 4; chanIdx++) {
            engine.softTakeover("[Channel" + chanIdx + "]", "rate", true);
        }
    }, true);
};

// Sampler Volume Control
MC7000.samplerLevel = function(channel, control, value) {
    // check if the Sampler Volume is at Zero and if so hide the sampler bank
    if (value > 0) {
        engine.setValue("[Samplers]", "show_samplers", true);
    } else {
        engine.setValue("[Samplers]", "show_samplers", false);
    }
    //control the sampler volumes with the one knob on the mixer
    for (let i = 1; i <= MC7000.SamplerQty; i++) {
        engine.setValue("[Sampler"+i+"]", "volume", script.absoluteLin(value, 0, 1.0, 4.0));
    }
};

// PAD Mode Hot Cue
MC7000.padModeCue = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "Cue";
    // change PAD color when switching to Hot Cue Mode
    for (let cueIdx = 1; cueIdx <= 8; cueIdx++) {
        const hotcueEnabled = engine.getValue(group, `hotcue_${cueIdx}_status`);
        midi.sendShortMsg(0x94 + deckIndex, 0x14 + cueIdx - 1, hotcueEnabled ? MC7000.padColor.hotcueon : MC7000.padColor.hotcueoff);
    }
};

// PAD Mode Cue Loop
MC7000.padModeCueLoop = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "CueLoop";
    // switch off PAD illumination
    MC7000.setAllPadColor(deckIndex, MC7000.padColor.alloff);
};

// PAD Mode Flip
MC7000.padModeFlip = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "Flip";
    // switch off PAD illumination
    MC7000.setAllPadColor(deckIndex, MC7000.padColor.alloff);
};

// PAD Mode Roll
MC7000.padModeRoll = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "Roll";
    // change PAD color when switching to Roll Mode
    MC7000.setAllPadColor(deckIndex, MC7000.padColor.rolloff);
};

// PAD Mode Saved Loop
MC7000.padModeSavedLoop = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "SavedLoop";
    // change PAD color when switching to Saved Loop Mode
    for (let savedLoopIdx = 0; savedLoopIdx < 8; savedLoopIdx++) {
        const activeLED = engine.getValue(group, "beatloop_" + MC7000.fixedLoop[savedLoopIdx] + "_enabled") ? MC7000.padColor.fixedloopon : MC7000.padColor.fixedloopoff;
        midi.sendShortMsg(0x94 + deckIndex, 0x14 + savedLoopIdx, activeLED);
    }
};

// PAD Mode Slicer
MC7000.padModeSlicer = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "Slicer";
    // change PAD color when switching to Slicer Mode
    MC7000.setAllPadColor(deckIndex, MC7000.padColor.sliceron);
};

// PAD Mode Slicer Loop
MC7000.padModeSlicerLoop = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "SlicerLoop";
    // switch off PAD illumination
    MC7000.setAllPadColor(deckIndex, MC7000.padColor.alloff);
};

// PAD Mode Sampler
MC7000.padModeSampler = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "Sampler";
    // change PAD color when switching to Sampler Mode
    MC7000.SamplerLED();
};

// PAD Mode Velocity Sampler
MC7000.padModeVelSamp = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "VelSamp";
    // change PAD color when switching to Velocity Sampler Mode
    MC7000.VelSampLED();
};

// PAD Mode Pitch
MC7000.HotcueSelectedGroup = [0, 0, 0, 0];
MC7000.padModePitch = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    MC7000.PADMode[deckIndex] = "Pitch";
    MC7000.halftoneToPadMap[deckIndex] = [4, 5, 6, 7, 0, 1, 2, 3];
    // switch on initial PAD illumination = hotcue for pitch or if
    // MC7000.HotcueSelectedGroup selected = pad mode pitchoff color
    for (let pitchIdx = 1; pitchIdx <= 8; pitchIdx++) {
        if (MC7000.HotcueSelectedGroup[deckIndex] !== 0) {
            midi.sendShortMsg(0x94 + deckIndex, 0x14 + pitchIdx - 1, MC7000.padColor.pitchoff);
        } else {
            const hotcueEnabled = engine.getValue(group, `hotcue_${pitchIdx}_status`);
            midi.sendShortMsg(0x94 + deckIndex, 0x14 + pitchIdx - 1, hotcueEnabled ? MC7000.padColor.hotcueon : MC7000.padColor.hotcueoff);
        }
    }
};


// PAD buttons
MC7000.PadButtons = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    let i, j;

    // The following modes are currently unhandled and could be
    // added as if-branches in the future:

    // - MC7000.PADModeCueLoop
    // - MC7000.PADModeFlip
    // - MC7000.PADModeSlicerLoop

    // activate and clear Hot Cues
    if (MC7000.PADMode[deckIndex] === "Cue" && engine.getValue(group, "track_loaded") === 1) {
        for (let cueIdx = 1; cueIdx <= 8; cueIdx++) {
            if (control === 0x14 + cueIdx - 1 && value === 0x7F) {
                engine.setValue(group, "hotcue_" + cueIdx + "_activate", true);
            } else if (control === 0x14 + cueIdx - 1 && value === 0x00) {
                engine.setValue(group, "hotcue_" + cueIdx + "_activate", false);
                if (engine.getValue(group, "slip_enabled")) {
                    engine.setValue(group, "slip_enabled", false);
                    engine.beginTimer(50, () => engine.setValue(group, "slip_enabled", true), true);
                }
            } else if (control === 0x1C + cueIdx - 1 && value === 0x7F) {
                engine.setValue(group, "hotcue_" + cueIdx + "_clear", true);
                midi.sendShortMsg(0x94 + deckIndex, 0x1C + cueIdx - 1, MC7000.padColor.hotcueoff);
            }
        }
    } else if (MC7000.PADMode[deckIndex] === "Roll") {
        // TODO(all): check for actual beatloop_size and apply back after a PAD Roll
        i = control - 0x14;
        if (control === 0x14 + i && value > 0x00) {
            engine.setValue(group, "beatlooproll_" + MC7000.beatLoopRoll[i] + "_activate", true);
            midi.sendShortMsg(0x94 + deckIndex, 0x14 + i, MC7000.padColor.rollon);
        } else if (control === 0x14 + i && value === 0x00) {
            engine.setValue(group, "beatlooproll_activate", false);
            midi.sendShortMsg(0x94 + deckIndex, 0x14 + i, MC7000.padColor.rolloff);
        }
    } else if (MC7000.PADMode[deckIndex] === "SavedLoop") {
        if (value === 0x00) {
            return; // don't respond to note off messages
        }
        i = control - 0x14;
        engine.setValue(group, "beatloop_" + MC7000.fixedLoop[i] + "_toggle", true);
        for (let loopIdx = 0; loopIdx < 8; loopIdx++) {
            const activeLED = engine.getValue(group, "beatloop_" + MC7000.fixedLoop[loopIdx] + "_enabled") ? MC7000.padColor.fixedloopon : MC7000.padColor.fixedloopoff;
            midi.sendShortMsg(0x94 + deckIndex, 0x14 + loopIdx, activeLED);
        }
    } else if (MC7000.PADMode[deckIndex] === "Slicer") {
        if (value > 0) {
            i = control - 0x14; // unshifted button
            j = control - 0x1C; // shifted button
            // forward buttons (PAD buttons upper row)
            if (control >= 0x14 && control <= 0x17) {
                engine.setValue(group, "beatjump_" + MC7000.beatJump[i] + "_forward", true);
                midi.sendShortMsg(0x94 + deckIndex, control, MC7000.padColor.slicerJumpFwd);
            // backward buttons (PAD buttons lower row)
            } else if (control >= 0x18 && control <= 0x1B) {
                engine.setValue(group, "beatjump_" + MC7000.beatJump[i - 4] + "_backward", true);
                midi.sendShortMsg(0x94 + deckIndex, control, MC7000.padColor.slicerJumpBack);
            // forward buttons (PAD buttons upper row - shifted controls)
            } else if (control >= 0x1C && control <= 0x1F) {
                engine.setValue(group, "beatjump_" + MC7000.beatJump[j + 4] + "_forward", true);
                midi.sendShortMsg(0x94 + deckIndex, control, MC7000.padColor.slicerJumpFwd);
            // backward buttons (PAD buttons lower row - shifted controls)
            } else if (control >= 0x20 && control <= 0x23) {
                engine.setValue(group, "beatjump_" + MC7000.beatJump[j] + "_backward", true);
                midi.sendShortMsg(0x94 + deckIndex, control, MC7000.padColor.slicerJumpBack);
            }
        } else {
            midi.sendShortMsg(0x94 + deckIndex, control, MC7000.padColor.sliceron);
        }
    } else if (MC7000.PADMode[deckIndex] === "Sampler") {
        let samplerOffset = 0;
        let samplerOffsetStop = 0;
        let deckOffset = 0;
        for (let samplerIdx = 1; samplerIdx <= 8; samplerIdx++) {
            if (MC7000.SamplerQty === 16) {
                deckOffset = (deckIndex % 2) * 8;
            } else if (MC7000.SamplerQty === 32) {
                deckOffset = deckIndex * 8;
            }
            samplerOffset = deckOffset + samplerIdx;
            if (control === 0x14 + samplerIdx - 1 && value >= 0x01) {
                if (engine.getValue("[Sampler" + samplerOffset + "]", "track_loaded") === 0) {
                    engine.setValue("[Sampler" + samplerOffset + "]", "LoadSelectedTrack", 1);
                } else if (engine.getValue("[Sampler" + samplerOffset + "]", "track_loaded") === 1) {
                    if (MC7000.prevSamplerStop) {
                        // stop playing all other samplers on this deck
                        for (let samplerStopIdx = 1; samplerStopIdx <= 8; samplerStopIdx++) {
                            samplerOffsetStop = deckOffset + samplerStopIdx;
                            if (engine.getValue("[Sampler" + samplerOffsetStop + "]", "play") === 1) {  // if sampler is playing then stop it
                                engine.setValue("[Sampler" + samplerOffsetStop + "]", "cue_gotoandstop", 1);
                            }
                        }
                    }
                    // ... before the actual sampler to play gets started
                    engine.setValue("[Sampler" + samplerOffset + "]", "pregain", 1);
                    engine.setValue("[Sampler" + samplerOffset + "]", "cue_gotoandplay", 1);
                }
            } else if (control === 0x1C + samplerIdx - 1 && value >= 0x01) { //shifted button deactivates playing sampler or ejects sampler
                if (engine.getValue("[Sampler" + samplerOffset + "]", "play") === 1) {
                    engine.setValue("[Sampler" + samplerOffset + "]", "cue_gotoandstop", 1);
                } else {
                    engine.setValue("[Sampler" + samplerOffset + "]", "eject", 1);
                    engine.setValue("[Sampler" + samplerOffset + "]", "eject", 0);
                }
            }
            MC7000.SamplerLED();
        }
    } else if (MC7000.PADMode[deckIndex] === "VelSamp") {
        let samplerOffset = 0;
        let samplerOffsetStop = 0;
        let deckOffset = 0;
        for (let velSampIdx = 1; velSampIdx <= 8; velSampIdx++) {
            if (MC7000.SamplerQty === 16) {
                deckOffset = (deckIndex % 2) * 8;
            } else if (MC7000.SamplerQty === 32) {
                deckOffset = deckIndex * 8;
            }
            samplerOffset = deckOffset + velSampIdx;
            if (control === 0x14 + velSampIdx - 1 && value >= 0x01) { // if padbutton for sampler VelSampIdx pressed
                if (engine.getValue("[Sampler" + samplerOffset + "]", "track_loaded") === 0) {  // if sampler is not loaded, load sampler and set color to loaded
                    engine.setValue("[Sampler" + samplerOffset + "]", "LoadSelectedTrack", 1);
                } else if (engine.getValue("[Sampler" + samplerOffset + "]", "track_loaded") === 1) {
                    if (MC7000.prevSamplerStop) {
                        // stop playing all other samplers on this deck
                        for (let samplerStopIdx = 1; samplerStopIdx <= 8; samplerStopIdx++) {
                            samplerOffsetStop = deckOffset + samplerStopIdx;
                            if (engine.getValue("[Sampler" + samplerOffsetStop + "]", "play") === 1) {  // if sampler is playing then stop it
                                engine.setValue("[Sampler" + samplerOffsetStop + "]", "cue_gotoandstop", 1);
                            }
                        }
                    }
                    // ... before the actual sampler to play gets started
                    engine.setValue("[Sampler" + samplerOffset + "]", "pregain", script.absoluteNonLin(value, 0, 1.0, 4.0));
                    engine.setValue("[Sampler" + samplerOffset + "]", "cue_gotoandplay", 1);
                }
            } else if (control === 0x1C + velSampIdx - 1 && value >= 0x01) { //shifted button deactivates playing sampler or ejects sampler
                engine.setValue("[Sampler" + samplerOffset + "]", "pregain", 1);
                if (engine.getValue("[Sampler" + samplerOffset + "]", "play") === 1) {
                    engine.setValue("[Sampler" + samplerOffset + "]", "cue_gotoandstop", 1);
                } else {
                    engine.setValue("[Sampler" + samplerOffset + "]", "eject", 1);
                    engine.setValue("[Sampler" + samplerOffset + "]", "eject", 0);
                }
            }
            MC7000.VelSampLED();
        }
    } else if (MC7000.PADMode[deckIndex] === "Pitch") {  // TODO play and cue dependency to play and cue button
        if (engine.getValue(group, "track_loaded") === 1) {
            for (let pitchIdx = 1; pitchIdx <= 8; pitchIdx++) {
                // intermediate variables
                const isButtonPressed = (value === 0x7F);
                const isButtonReleased = (value === 0x00);
                const isControlAddress = (control === 0x14 + pitchIdx - 1);
                const isControlAddressShift = (control === 0x1C + pitchIdx - 1);
                const hotcueEnabled = engine.getValue(group, `hotcue_${pitchIdx}_status`), HotcueSelectedOnDeck = MC7000.HotcueSelectedGroup[deckIndex];
                if (isButtonPressed && isControlAddress) {
                    if (!HotcueSelectedOnDeck) {
                        MC7000.setAllPadColor(deckIndex, MC7000.padColor.pitchoff);
                        MC7000.HotcueSelectedGroup[deckIndex] = pitchIdx; // store which hotcue should be used for pitch
                        if (!hotcueEnabled) { //hotcue select if none available
                            engine.setValue(group, "hotcue_" + pitchIdx + "_activate", true); // set hotcue if not set before
                        }
                    } else { // hotcue selected and button pressed // TODO: play if play, stop if cue
                        engine.setValue(group, "hotcue_" + MC7000.HotcueSelectedGroup[deckIndex] + "_gotoandstop", true); // stop
                        MC7000.setAllPadColor(deckIndex, MC7000.padColor.pitchoff);
                        engine.setValue(group, "pitch", MC7000.halftoneToPadMap[deckIndex][pitchIdx - 1]);
                        engine.setValue(group, "hotcue_" + MC7000.HotcueSelectedGroup[deckIndex] + "_gotoandplay", true);
                        midi.sendShortMsg(0x94 + deckIndex, 0x14 + pitchIdx - 1, MC7000.padColor.pitchon); // if pitch is pressed switch to pitch on color
                        midi.sendShortMsg(0x94 + deckIndex, 0x1C + pitchIdx - 1, MC7000.padColor.pitchon); // keep color when shift is pressed
                    }
                } else if (isButtonReleased && isControlAddress) { // button release change color and stop play
                    // engine.setValue(group, "hotcue_" + MC7000.HotcueSelectedGroup[deckIndex] + "_gotoandstop", true); // stop  //TODO if for setting continue to play or stop on button release
                    // midi.sendShortMsg(0x94 + deckIndex, 0x14 + pitchIdx - 1, MC7000.padColor.pitchoff); // switch to pitch off color
                    if (engine.getValue(group, "slip_enabled")) {
                        engine.setValue(group, "slip_enabled", false);
                        engine.beginTimer(50, () => engine.setValue(group, "slip_enabled", true), true);
                    }
                } else if (isButtonPressed && isControlAddressShift) { //shifted buttons deselect hotcue for pitch
                    engine.setValue(group, "pitch", 0);
                    MC7000.HotcueSelectedGroup[deckIndex] = 0;
                    for (let padIdx = 1; padIdx <= 8; padIdx++) {
                        const padHotcueEnabled = engine.getValue(group, `hotcue_${padIdx}_status`);
                        midi.sendShortMsg(0x94 + deckIndex, 0x14 + padIdx - 1, padHotcueEnabled ? MC7000.padColor.hotcueon : MC7000.padColor.hotcueoff);
                        midi.sendShortMsg(0x94 + deckIndex, 0x1C + padIdx - 1, padHotcueEnabled ? MC7000.padColor.hotcueon : MC7000.padColor.hotcueoff); // keep color when shift is pressed
                    }
                }
            }
        }
    }
};

// Set single LED color for future implementation
//MC7000.setSinglePadColor = function(deckIdx, padIdx, colorValue) {
//    switch 1 buttons to selected color, also when shift is pressed
//    midi.sendShortMsg(0x94 + deckIdx, 0x14 + padIdx, colorValue);
//    midi.sendShortMsg(0x94 + deckIdx, 0x1C + padIdx, colorValue); // keep color when shift is pressed
//};

MC7000.setAllPadColor = function(deckIndex, colorValue) {
    for (let padIdx = 0; padIdx < 8; padIdx++) {
        // switch 8 buttons to selected color
        midi.sendShortMsg(0x94 + deckIndex, 0x14 + padIdx, colorValue);
        midi.sendShortMsg(0x94 + deckIndex, 0x1C + padIdx, colorValue); // keep color when shift is pressed
    }
};

// Shift Button
MC7000.shiftButton = function(channel, control, value, status, group) {
    const deckIndex = script.deckFromGroup(group) - 1;
    MC7000.shift[deckIndex] = value > 0;
    midi.sendShortMsg(0x90 + deckIndex, 0x32,
        MC7000.shift[deckIndex] ? 0x7F : 0x01);
};

// Toggle Vinyl Mode
MC7000.vinylModeToggle = function(channel, control, value, status, group) {
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    const deckIndex = script.deckFromGroup(group) - 1;
    MC7000.isVinylMode[deckIndex] = !MC7000.isVinylMode[deckIndex];
    midi.sendShortMsg(0x90 + deckIndex, 0x07,
        MC7000.isVinylMode[deckIndex] ? 0x7F : 0x01);
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
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    const libraryMaximized = engine.getValue("[Skin]", "show_maximized_library") > 0;
    if (MC7000.isVinylMode[deckIndex] && !libraryMaximized) {
        if (value === 0x7F) {
            engine.scratchEnable(deckNumber, MC7000.jogWheelTicksPerRevolution,
                MC7000.scratchParams.recordSpeed,
                MC7000.scratchParams.alpha,
                MC7000.scratchParams.beta);
        } else {
            if (engine.getValue(group, "slip_enabled")) {
                engine.scratchDisable(deckNumber, false); // stops scratching immediately
                engine.setValue(group, "slip_enabled", false);
                engine.beginTimer(50, () => engine.setValue(group, "slip_enabled", true), true);
            } else {
                engine.scratchDisable(deckNumber); // continues scratching e.g. for backspin
            }
        }
    }
};

// The wheel that actually controls the scratching
MC7000.wheelTurn = function(channel, control, value, status, group) {
    // TODO(all): check for latency and use it to normalize the jog factor so
    // jog won't be depending on audio latency anymore.

    // A: For a control that centers on 0:
    const numTicks = (value < 0x64) ? value : (value - 128);
    const baseSpeed = numTicks * MC7000.jogParams.sensitivity;
    const adjustedSpeed = baseSpeed / 10;
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    const libraryMaximized = engine.getValue("[Skin]", "show_maximized_library");
    if (libraryMaximized === 1 && numTicks > 0) {
        engine.setValue("[Library]", "MoveDown", 1);
    } else if (libraryMaximized === 1 && numTicks < 0) {
        engine.setValue("[Library]", "MoveUp", 1);
    } else if (engine.isScratching(deckNumber)) {
        // Scratch!
        let scratchSpeed = baseSpeed;
        const acceleration = MC7000.jogParams.acceleration;
        if (acceleration && acceleration.enabled) {
            const accelerationFactor = Math.pow(Math.abs(baseSpeed), acceleration.exponent) * acceleration.coefficient;
            scratchSpeed *= accelerationFactor;
        }
        engine.scratchTick(deckNumber, scratchSpeed);
    } else {
        if (MC7000.shift[deckIndex]) {
            // While Shift Button pressed -> Search through track
            const jogSearch = 100 * adjustedSpeed; // moves 100 times faster than normal jog
            engine.setValue(group, "jog", jogSearch);
        } else {
            // While Shift Button released -> Pitch Bend
            engine.setValue(group, "jog", adjustedSpeed);
        }
    }
};

// Needle Search Touch detection
MC7000.needleSearchTouch = function(channel, control, value, status, group) {
    const deckIndex = script.deckFromGroup(group) - 1;
    if (engine.getValue(group, "play")) {
        MC7000.needleSearchTouched[deckIndex] = MC7000.needleSearchPlay && (!!value);
    } else {
        MC7000.needleSearchTouched[deckIndex] = !!value;
    }
};

// Needle Search Touch while "SHIFT" button is pressed
MC7000.needleSearchTouchShift = function(channel, control, value, status,
    group) {
    const deckIndex = script.deckFromGroup(group) - 1;
    MC7000.needleSearchTouched[deckIndex] = !!value;
};

// Needle Search Position detection (MSB)
MC7000.needleSearchMSB = function(channel, control, value) {
    MC7000.needleDropMSB = value; // just defining rough position
};

// Needle Search Position detection (MSB + LSB)
MC7000.needleSearchStripPosition = function(channel, control, value, status,
    group) {
    const deckIndex = script.deckFromGroup(group) - 1;
    if (MC7000.needleSearchTouched[deckIndex]) {
        const fullValue = (MC7000.needleDropMSB << 7) + value; // move MSB 7 binary digits to the left and add LSB
        const position = (fullValue / 0x3FFF); // divide by all possible positions to get relative between 0 - 1
        engine.setParameter(group, "playposition", position);
    }
};

// Pitch Fader (MSB)
MC7000.pitchFaderMSB = function(channel, control, value) {
    MC7000.pitchMSB = value; // just defining rough position
};

// Pitch Fader Position (MSB + LSB)
MC7000.pitchFaderPosition = function(channel, control, value, status, group) {
    const fullValue = (MC7000.pitchMSB << 7) + value;
    const position = 1 - (fullValue / 0x3FFF); // 1 - () to turn around the direction
    engine.setParameter(group, "rate", position);
};

// Next Rate range toggle
MC7000.nextRateRange = function(midichan, control, value, status, group) {
    if (value === 0) {
        return; // don't respond to note off messages
    }
    const currRateRange = engine.getValue(group, "rateRange");
    engine.setValue(group, "rateRange", MC7000.getNextRateRange(currRateRange));
};

MC7000.getNextRateRange = function(currRateRange) {
    for (let i = 0; i < MC7000.rateRanges.length; i++) {
        if (MC7000.rateRanges[i] > currRateRange) {
            return MC7000.rateRanges[i];
        }
    }
    return MC7000.rateRanges[0];
};

// Previous Rate range toggle
MC7000.prevRateRange = function(midichan, control, value, status, group) {
    if (value === 0) {
        return; // don't respond to note off messages
    }
    const currRateRange = engine.getValue(group, "rateRange");
    engine.setValue(group, "rateRange", MC7000.getPrevRateRange(currRateRange));
};

MC7000.getPrevRateRange = function(currRateRange) {
    for (let i = MC7000.rateRanges.length; i >= 0; i--) {
        if (MC7000.rateRanges[i] < currRateRange) {
            return MC7000.rateRanges[i];
        }
    }
    return MC7000.rateRanges[MC7000.rateRanges.length - 1];
};

// Key & Waveform zoom Select
MC7000.keySelect = function(midichan, control, value, status, group) {
    const deckIndex = script.deckFromGroup(group) - 1;
    // While Shift Button is pressed: Waveform Zoom
    if (MC7000.shift[deckIndex]) {
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
    const deckIndex = script.deckFromGroup(group) - 1;
    if (value === 0x00) {
        return;
    }
    // While Shift Button is pressed: Waveform Zoom Reset
    if (MC7000.shift[deckIndex]) {
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
    const deckIndex = script.deckFromGroup(group) - 1;
    // "factor" for engine.brake() and engine.softStart()
    // this formula produces factors between 31 (min STOP TIME for ca 7 sec back
    // in track) and 1 (max STOP TIME for ca 18.0 sec back in track)
    MC7000.spinbackFactor[deckIndex] = (1.1 - (value / 127)) * 30 - 2;
    MC7000.brakeSoftstartFactor[deckIndex] = (127.69 - value);
};

MC7000.lastpress = [0, 0, 0, 0];
MC7000.play = function(channel, control, value, status, group) {
    if (value === 0x00) {
        return; // don't respond to note off messages
    }
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    // set a variable to toggle between play and pause, based on current play status
    const playToggle = engine.getValue(group, "play");
    if (MC7000.spinbackFactor[deckIndex] === 31) { // factor 31 means stop time knob is at zero position
        engine.setValue(group, "play", !playToggle);
        MC7000.lastpress[deckIndex] = 0;
    } else {
        if (playToggle) {
            if (!MC7000.lastpress[deckIndex]) {
                engine.brake(deckNumber, true, MC7000.brakeSoftstartFactor[deckIndex]);
                MC7000.lastpress[deckIndex] = 1;
            } else {
                engine.softStart(deckNumber, true, MC7000.brakeSoftstartFactor[deckIndex]);
                MC7000.lastpress[deckIndex] = 0;
            }
        } else {
            engine.softStart(deckNumber, true, MC7000.brakeSoftstartFactor[deckIndex]);
            MC7000.lastpress[deckIndex] = 0;
        }
    }
};

// Use SHIFT + CENSOR button as Spinback with STOP TIME adjusted length
MC7000.reverse = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (value > 0) {
        // while the button is pressed spin back
        // start at a rate of -10 and decrease by "MC7000.spinbackFactor"
        engine.brake(deckNumber, true, MC7000.spinbackFactor[deckIndex], -10);
    } else {
        if (engine.getValue(group, "slip_enabled")) {
            engine.brake(deckNumber, false); // disable brake effect
            engine.setValue(group, "play", 1);
            engine.setValue(group, "slip_enabled", false);
            engine.beginTimer(50, () => engine.setValue(group, "slip_enabled", true), true);
        } else {
            engine.softStart(deckNumber, true, MC7000.spinbackFactor[deckIndex]);
        }
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
        engine.beginTimer(50, () => engine.setValue(group, "slip_enabled", true), true);
    } else {
        // reverse play while button pressed
        if (value > 0) {
            engine.setValue(group, "reverse", 1);
        } else {
            engine.setValue(group, "reverse", 0);
        }
    }
};

// Parameter Buttons
MC7000.parameterButton = function(value, group, {isLeftButton, isShiftPressed}) {
    if (value === 0) {
        return;
    }

    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    const settings = MC7000.parameterButtonSettings;

    if (settings.parameterButtonPitchPlayOverrideEnabled && MC7000.PADMode[deckIndex] === "Pitch") {
        const pitchDelta = isLeftButton ? -8 : 8;
        for (let padIdx = 0; padIdx < 8; padIdx++) {
            MC7000.halftoneToPadMap[deckIndex][padIdx] += pitchDelta;
        }
    } else {
        switch (settings.mode) {
        case "starsAndColor":
            if (isShiftPressed) {
                script.triggerControl(group, `track_color_${isLeftButton ? "prev" : "next"}`);
            } else {
                script.triggerControl(group, `stars_${isLeftButton ? "down" : "up"}`);
            }
            break;
        case "beatjump":
            if (isShiftPressed) {
                const beatJumpSize = engine.getValue(group, "beatjump_size");
                const indexDelta = isLeftButton ? -1 : 1;
                const newIndex = Math.max(0, Math.min(MC7000.beatJump.length - 1, MC7000.beatJump.indexOf(beatJumpSize) + indexDelta));
                const newBeatJumpSize = MC7000.beatJump[newIndex];
                engine.setValue(group, "beatjump_size", newBeatJumpSize);
            } else {
                script.triggerControl(group, `beatjump_${isLeftButton ? "backward" : "forward"}`);
            }
            break;
        case "introOutro":
            {
                const cue = isLeftButton ? "intro_end" : "outro_start";
                const action = isShiftPressed ? "clear" : "activate";
                script.triggerControl(group, `${cue}_${action}`);
            }
            break;
        default:
            break;
        }
    }
};

// Parameter Button '<'
MC7000.parameterButtonLeft = function(channel, control, value, status, group) {
    MC7000.parameterButton(value, group, {
        isLeftButton: true,
        isShiftPressed: false
    });
};

// Parameter Button '>'
MC7000.parameterButtonRight = function(channel, control, value, status, group) {
    MC7000.parameterButton(value, group, {
        isLeftButton: false,
        isShiftPressed: false
    });
};

// Parameter Button '<' + 'SHIFT'
MC7000.parameterButtonLeftShifted = function(channel, control, value, status, group) {
    MC7000.parameterButton(value, group, {
        isLeftButton: true,
        isShiftPressed: true
    });
};

// Parameter Button '>' + 'SHIFT'
MC7000.parameterButtonRightShifted = function(channel, control, value, status, group) {
    MC7000.parameterButton(value, group, {
        isLeftButton: false,
        isShiftPressed: true
    });
};

// Set Crossfader Curve
MC7000.crossFaderCurve = function(channel, control, value) {
    script.crossfaderCurve(value);
};

// Update state on deck changes
MC7000.switchDeck = function(channel, control, value, status) {
    const deckIndex = status - 0x90;
    const isTopDeck = deckIndex < 2;
    const side = deckIndex % 2;
    const previousdeckIndex = (deckIndex + 2) % 4;

    // We need to 'transfer' the shift state when switching decks,
    // otherwise it will get stuck and result in an 'inverted'
    // shift after switching back to the deck.
    // Since the controller switches immediately upon pressing down,
    // we only do this when value is high.

    if (value === 0x7F && MC7000.topDeckActive[side] !== isTopDeck) {
        MC7000.topDeckActive[side] = isTopDeck;
        MC7000.shift[deckIndex] = MC7000.shift[previousdeckIndex];
        MC7000.shift[previousdeckIndex] = false;
    }
};

// Set FX wet/dry value
MC7000.fxWetDry = function(channel, control, value, status, group) {
    const numTicks = (value < 0x64) ? value : (value - 128);
    const newVal = engine.getValue(group, "mix") + numTicks/64*2;
    engine.setValue(group, "mix", Math.max(0, Math.min(1, newVal)));
};

// Sort the library for Artist, Title, BPM and Key
MC7000.sortLibrary = function(channel, control, value) {
    if (value === 0) {
        return;
    }
    let sortColumn;
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
    const deckIndex = script.deckFromGroup(group) - 1;
    // sends either peak_indicator or scales value (0..1) to (0..117) while truncating to each LED
    const vuLevelOutValue = engine.getValue(group, "peak_indicator") ? MC7000.VuMeterLEDPeakValue : Math.floor(Math.pow(value, 2.5) * 9) * 13;
    // only send Midi signal when LED value has changed
    if (MC7000.prevVuLevel[deckIndex] !== vuLevelOutValue) {
        midi.sendShortMsg(0xB0 + deckIndex, 0x1F, vuLevelOutValue);
        MC7000.prevVuLevel[deckIndex] = vuLevelOutValue;
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
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    const trackDuration = engine.getValue(group, "duration"); // in seconds
    const beatLength = engine.getValue(group, "file_bpm") / 60; // in Beats Per Seconds
    const cuePosition = engine.getValue(group, "cue_point") / engine.getValue(group, "track_samplerate") / 2; // in seconds
    const playPosition = value * trackDuration; // in seconds
    const jogLEDPosition = playPosition / 60 * MC7000.scratchParams.recordSpeed;
    const jogLEDNumber = 48; // LED ring contains 48 segments each triggered by the next even Midi value
    // check for Vinyl Mode and decide to spin the Jog LED or show play position
    const activeJogLED = MC7000.isVinylMode[deckIndex] ? Math.round(jogLEDPosition * jogLEDNumber) % jogLEDNumber : Math.round(value * jogLEDNumber);
    // count the beats (1 to 8) after the CUE point
    const beatCountLED = (Math.floor((playPosition - cuePosition) * beatLength) % 8); //calculate PAD LED position
    // TODO(all): check for playposition < (trackduration - warning length) for sending position signals
    // check if a Jog LED has changed and if so then send the signal to the next Jog LED
    if (MC7000.prevJogLED[deckIndex] !== activeJogLED) {
        midi.sendShortMsg(0x90 + deckIndex, 0x06, activeJogLED * 2); // only each 2nd midi signal triggers the next LED
        MC7000.prevJogLED[deckIndex] = activeJogLED;
    }
    // TODO(all): else blink the platter LEDs
    // check if Slicer mode is active and illuminate PAD LEDs counting with the beat while playing
    if (!MC7000.experimental) {
        return;
    }
    if (MC7000.PADMode[deckIndex] === "Slicer") {
        // only send new LED status when beatCountLED really changes
        if (MC7000.prevPadLED[deckIndex] !== beatCountLED) {
            // first set all LEDs to default color incl shifted
            for (let slicerIdx = 0; slicerIdx < 16; slicerIdx++) {
                midi.sendShortMsg(0x94 + deckIndex, 0x14 + slicerIdx, MC7000.padColor.sliceron);
            }
            // now chose which PAD LED to turn on (+8 means shifted PAD LEDs)
            if (beatCountLED === 0) {
                midi.sendShortMsg(0x94 + deckIndex, 0x14, MC7000.padColor.slicerJumpFwd);
                midi.sendShortMsg(0x94 + deckIndex, 0x14 + 8, MC7000.padColor.slicerJumpFwd);
            } else if (beatCountLED === 7) {
                midi.sendShortMsg(0x94 + deckIndex, 0x1B, MC7000.padColor.slicerJumpFwd);
                midi.sendShortMsg(0x94 + deckIndex, 0x1B + 8, MC7000.padColor.slicerJumpFwd);
            } else if (beatCountLED > 0 && beatCountLED < 7) {
                midi.sendShortMsg(0x94 + deckIndex, 0x14 + beatCountLED, MC7000.padColor.slicerJumpFwd);
                midi.sendShortMsg(0x94 + deckIndex, 0x14 + 8 + beatCountLED, MC7000.padColor.slicerJumpFwd);
            }
        }
        MC7000.prevPadLED[deckIndex] = beatCountLED;
    }
};

// initial HotCue LED when loading a track with already existing hotcues
MC7000.HotCueLED = function(value, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (MC7000.PADMode[deckIndex] === "Cue") {
        for (let padIdx = 1; padIdx <= 8; padIdx++) {
            if (value === 1) {
                if (engine.getValue(group, `hotcue_${padIdx}_status`) === 1) {
                    midi.sendShortMsg(0x94 + deckIndex, 0x14 + padIdx - 1, MC7000.padColor.hotcueon);
                    midi.sendShortMsg(0x94 + deckIndex, 0x1C + padIdx - 1, MC7000.padColor.hotcueon);
                }
            } else {
                if (engine.getValue(group, `hotcue_${padIdx}_status`) === 0) {
                    midi.sendShortMsg(0x94 + deckIndex, 0x14 + padIdx - 1, MC7000.padColor.hotcueoff);
                    midi.sendShortMsg(0x94 + deckIndex, 0x1C + padIdx - 1, MC7000.padColor.hotcueoff);
                }
            }
        }
    }
};

// Sampler LED
MC7000.SamplerLED = function() {
    for (let deckIdx = 0; deckIdx < 4; deckIdx++) {
        for (let samplerIdx = 1; samplerIdx <= 8; samplerIdx++) {
            let sampNo = 0;
            if (MC7000.PADMode[deckIdx] === "Sampler") {
                if (MC7000.SamplerQty === 16) {
                    // use sampler 1-16 for deck 1 and deck 2 and sampler 1-16 for deck 3 and 4
                    sampNo = (deckIdx % 2) * 8 + samplerIdx;
                } else if (MC7000.SamplerQty === 32) {
                    // use sampler 1 - sampler 32 for deck 1 - 4
                    sampNo = deckIdx * 8 + samplerIdx;
                }
                if (engine.getValue("[Sampler"+sampNo+"]", "track_loaded") === 1) {
                    const samplerIsNotPlaying = engine.getValue("[Sampler"+sampNo+"]", "play") === 0;
                    if (samplerIsNotPlaying) {
                        midi.sendShortMsg(0x94 + deckIdx, 0x14 + samplerIdx - 1, MC7000.padColor.samplerloaded); // set pad color without shift pressed
                        midi.sendShortMsg(0x94 + deckIdx, 0x1C + samplerIdx - 1, MC7000.padColor.samplerloaded); // shift pressed sets the same color
                    } else {
                        midi.sendShortMsg(0x94 + deckIdx, 0x14 + samplerIdx - 1, MC7000.padColor.samplerplay);
                    }
                } else if (engine.getValue("[Sampler"+sampNo+"]", "track_loaded") === 0) {
                    midi.sendShortMsg(0x94 + deckIdx, 0x14 + samplerIdx - 1, MC7000.padColor.sampleroff); // set pad color without shift pressed
                    midi.sendShortMsg(0x94 + deckIdx, 0x1C + samplerIdx - 1, MC7000.padColor.sampleroff); // shift pressed sets the same color
                }
            }
        }
    }
};

// Velocity Sampler LED
MC7000.VelSampLED = function() {
    for (let deckIdx = 0; deckIdx < 4; deckIdx++) {
        for (let velSampIdx = 1; velSampIdx <= 8; velSampIdx++) {
            let sampNo = 0;
            if (MC7000.PADMode[deckIdx] === "VelSamp") {
                if (MC7000.SamplerQty === 16) {
                    // use sampler 1-16 for deck 1 and deck 2 and sampler 1-16 for deck 3 and 4
                    sampNo = (deckIdx % 2) * 8 + velSampIdx;
                } else if (MC7000.SamplerQty === 32) {
                    // use sampler 1 - sampler 32 for deck 1 - 4
                    sampNo = deckIdx * 8 + velSampIdx;
                }
                if (engine.getValue("[Sampler"+sampNo+"]", "track_loaded") === 1) {
                    const samplerIsNotPlaying = engine.getValue("[Sampler"+sampNo+"]", "play") === 0;
                    if (samplerIsNotPlaying) {
                        midi.sendShortMsg(0x94 + deckIdx, 0x14 + velSampIdx - 1, MC7000.padColor.velsamploaded); // set pad color without shift pressed
                        midi.sendShortMsg(0x94 + deckIdx, 0x1C + velSampIdx - 1, MC7000.padColor.velsamploaded); // shift pressed sets the same color
                        engine.setValue("[Sampler"+sampNo+"]", "pregain", 1);
                    } else {
                        midi.sendShortMsg(0x94 + deckIdx, 0x14 + velSampIdx - 1, MC7000.padColor.velsampplay);
                    }
                } else if (engine.getValue("[Sampler"+sampNo+"]", "track_loaded") === 0) {
                    midi.sendShortMsg(0x94 + deckIdx, 0x14 + velSampIdx - 1, MC7000.padColor.velsampoff); // set pad color without shift pressed
                    midi.sendShortMsg(0x94 + deckIdx, 0x1C + velSampIdx - 1, MC7000.padColor.velsampoff); // shift pressed sets the same color
                }
            }
        }
    }
};

// Pitch LED when loading a track with already existing hotcues for pitch
MC7000.PitchLED = function(value, group) {
    const deckNumber = script.deckFromGroup(group);
    const deckIndex = deckNumber - 1;
    if (MC7000.PADMode[deckIndex] === "Pitch") {
        for (let pitchIdx = 0; pitchIdx < 8; pitchIdx++) {
            if (engine.getValue("[Channel"+deckNumber+"]", "play") === 0) { // stopped
                if (MC7000.HotcueSelectedGroup[deckIndex] !== 0) { // hotcue selected
                    midi.sendShortMsg(0x94 + deckIndex, 0x14 + pitchIdx, MC7000.padColor.pitchoff);
                } else {
                    const hotcueEnabled = engine.getValue(group, `hotcue_${pitchIdx + 1}_status`);
                    midi.sendShortMsg(0x94 + deckIndex, 0x14 + pitchIdx, hotcueEnabled ? MC7000.padColor.hotcueon : MC7000.padColor.hotcueoff);
                }
            } else {
                midi.sendShortMsg(0x94 + deckIndex, 0x14 + pitchIdx, MC7000.padColor.pitchon);
            }
        }
    }
};



/* CONTROLLER SHUTDOWN */
MC7000.shutdown = function() {
    // Need to switch off LEDs one by one,
    // otherwise the controller cannot handle the signal traffic

    // Switch off Transport section LEDs
    for (let i = 0; i <= 3; i++) {
        midi.sendShortMsg(0x90 + i, 0x00, 0x01);
        midi.sendShortMsg(0x90 + i, 0x01, 0x01);
        midi.sendShortMsg(0x90 + i, 0x02, 0x01);
        midi.sendShortMsg(0x90 + i, 0x03, 0x01);
        midi.sendShortMsg(0x90 + i, 0x04, 0x01);
        midi.sendShortMsg(0x90 + i, 0x05, 0x01);
    }
    // Switch off Loop Section LEDs
    for (let i = 0; i <= 3; i++) {
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
    for (let i = 0x14; i <= 0x1B; i++) {
        midi.sendShortMsg(0x94, i, 0x02);
        midi.sendShortMsg(0x95, i, 0x02);
        midi.sendShortMsg(0x96, i, 0x02);
        midi.sendShortMsg(0x97, i, 0x02);
    }
    // Switch off Channel Cue, VINYL, SLIP, KEY LOCK LEDs
    for (let i = 0; i <= 3; i++) {
        midi.sendShortMsg(0x90 + i, 0x07, 0x01);
        midi.sendShortMsg(0x90 + i, 0x0F, 0x01);
        midi.sendShortMsg(0x90 + i, 0x0D, 0x01);
        midi.sendShortMsg(0x90 + i, 0x1B, 0x01);
    }
    // Switch off FX Section LEDs
    for (let i = 0; i <= 1; i++) {
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
    for (let i = 0; i <= 3; i++) {
        // Switch off Level Meters
        midi.sendShortMsg(0xB0 + i, 0x1F, 0x00);
        // Platter Ring: Reset JogLED to Zero position
        midi.sendShortMsg(0x90 + i, 0x06, 0x00);
        // Platter Ring: Switch all LEDs on
        midi.sendShortMsg(0x90 + i, 0x64, 0x00);
    }
};

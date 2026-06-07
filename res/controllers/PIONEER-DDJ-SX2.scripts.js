// Pioneer DDJ-SX2 mapping for Mixxx
// Based on tildearrow's mapping for the DDJ-SX2
// Modifications by Krafting, eXWoLL(+Assistant)

// Thanks to:
// 		hrudham for making the DDJ-SR mapping
// 		pioneer for making such an awesome controller
//
// =============================================================================
// SCRIPT INDEX
// =============================================================================
// 1.  CONSTANTS, DEBUGGING & HELPERS ............
// 2.  STATE VARIABLES & SETTINGS ................
// 3.  INITIALIZATION & SHUTDOWN .................
// 4.  TIMER & BLINKING ..........................
// 5.  CONNECTIONS (Signals) .....................
// 6.  LIGHTING & FEEDBACK .......................
// 7.  JOGWHEEL & SCRATCHING .....................
// 8.  MIXER & TRANSPORT (14-bit Support) ........
// 8b. KNOB KILL FUNCTIONS (Shift+Knob Down) .....
// 9.  FX SECTION ................................
// 10. VIEW, GRID, & PANELS ......................
// 11. SAMPLER & PAD PARAMETERS ..................
// 12. PAD MODES .................................
// 13. LIBRARY & BROWSER .........................
// 14. FLIP SECTION ..............................
// =============================================================================
//
// License: (MIT)
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

var PioneerDDJSX2 = {};

// Guidelines :
// Buttons function should have these variables as their parameters, in this order :
//      channel, control, value, status, group
// For MIDI control (LEDs) with makeConnections :
//      value, group, control
// CC function should have these instead :
//      channel, control, value

// =============================================================================
// 1. CONSTANTS, DEBUGGING & HELPERS
// =============================================================================

// SysEx constants for Controller Initialization & Keep Alive.
// The SX2 requires a "Keep Alive" signal periodically to prevent it from
// falling back to a standby/demo state or locking the jog wheels.
PioneerDDJSX2.keepAlive = [0xF0, 0x00, 0x20, 0x7F, 0x50, 0x01, 0xF7];
PioneerDDJSX2.recallState = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];

// Performance pad colors (Hex to MIDI value mapping)
// Maps standard HTML-style Hex codes to the specific MIDI velocity values
// required by the Pioneer RGB pads.
// 0: Off, 1-127: Various colors and brightness levels.

// Performance pad colors
// 0: off
// 1: blue
// 12: cyan
// 22: green
// 31: yellow
// 41: red
// 48: magenta
// 62: blue
// 63: black
// 64: white (actually light blue)
// 65-126: invalid
// 127: current mode default

PioneerDDJSX2.padColors = new ColorMapper({
    0x0000FF: 1,
    0x0010FF: 2,
    0x0028FF: 3,
    0x0040FF: 4,
    0x0060FF: 5,
    0x0080FF: 6,
    0x00A0FF: 7,
    0x00B0FF: 8,
    0x00D0FF: 9,
    0x00E0FF: 10,
    0x00F0FF: 11,

    0x00FFFF: 12,
    0x00FFF0: 13,
    0x00FFE0: 14,
    0x00FFD0: 15,
    0x00FFB0: 16,
    0x00FF90: 17,
    0x00FF70: 18,
    0x00FF50: 19,
    0x00FF30: 20,
    0x00FF10: 21,

    0x00FF00: 22,
    0x10FF00: 23,
    0x30FF00: 24,
    0x40FF00: 25,
    0x60FF00: 26,
    0x80FF00: 27,
    0xA0FF00: 28,
    0xC0FF00: 29,
    0xE0FF00: 30,

    0xFFFF00: 31,
    0xFFE000: 32,
    0xFFC000: 33,
    0xFFB000: 34,
    0xFFA000: 35,
    0xFF9000: 36,
    0xFF7000: 37,
    0xFF5000: 38,
    0xFF3000: 39,
    0xFF1000: 40,

    0xFF0000: 41,
    0xFF0020: 42,
    0xFF0030: 43,
    0xFF0060: 44,
    0xFF0090: 45,
    0xFF00A0: 46,
    0xFF00E0: 47,

    0xFF00FF: 48,
    0xF000FF: 49,
    0xE000FF: 50,
    0xD000FF: 51,
    0xC000FF: 52,
    0xB000FF: 53,
    0xA000FF: 54,
    0x9000FF: 55,
    0x8000FF: 56,
    0x6000FF: 57,
    0x5000FF: 58,
    0x4000FF: 59,
    0x3000FF: 60,
    0x2000FF: 61,
    0x1000FF: 62,

    0x000000: 63,
    0xF0FFFF: 64,
});


// =============================================================================
// 2. STATE VARIABLES & SETTINGS
// =============================================================================

// Timers and System State
PioneerDDJSX2.lightsTimer = 0; // Reference to the Keep-Alive/Blink timer
PioneerDDJSX2.SyncEnableTimer = null; // Timer for Sync vs BeatSync logic
PioneerDDJSX2.conns = []; // Stores signal connections to unbind on shutdown
PioneerDDJSX2.channels = {
    0x00: {},
    0x01: {},
    0x02: {},
    0x03: {}
};

// Deck & Pad State
PioneerDDJSX2.shiftState = [0, 0, 0, 0]; // Shift button state per deck
PioneerDDJSX2.reverse = [0, 0, 0, 0]; // Reverse playback state
PioneerDDJSX2.vinylOn = [1, 1, 1, 1]; // Vinyl Mode enabled
PioneerDDJSX2.padMode = [0, 0, 0, 0]; // Current Pad Mode (Hotcue, Roll, Slicer, etc.)
// 0: 8% of max, 1: 16%, 2: 32%, 3: 64%
PioneerDDJSX2.tempoRange = [0, 0, 0, 0];
PioneerDDJSX2.closestBeatToLoopIn = [0, 0, 0, 0];
PioneerDDJSX2.isBraking = 0; // Tracks if deck is currently in braking animation

// Jogwheel & Grid State
PioneerDDJSX2.gridSlide = [0, 0, 0, 0]; // Modifier state for Slide Grid
PioneerDDJSX2.gridAdjust = [0, 0, 0, 0]; // Modifier state for Adjust Grid
PioneerDDJSX2.TurnTablePos = [0, 0, 0, 0]; // Current virtual rotation position
PioneerDDJSX2.FinalTurnPos = [-1, -1, -1, -1]; // Last sent LED position (deduplication)

// LED Optimization State
// Critical for SX2: Throttles MIDI messages to prevent bus saturation
PioneerDDJSX2.lastLightUpdate = [0, 0, 0, 0];
PioneerDDJSX2.currentBeat = [0, 0, 0, 0];
PioneerDDJSX2.blinkState = 0; // Toggles every timer tick for blinking LEDs

// Hotcue Warning Red Center Lights LED
PioneerDDJSX2.HotCueLEDStatus = {};
PioneerDDJSX2.isThereHotCue = [];

// View & Layout
PioneerDDJSX2.curPanel = 0; // Tracks visible GUI panels
PioneerDDJSX2.curView = 0; // Tracks visible waveform views

// Pad Parameters
PioneerDDJSX2.rollPrec = [2, 2, 2, 2]; // Precision for Roll loop sizes
PioneerDDJSX2.beat = [0, 0, 0, 0]; // Current beat calculation for Slicer/LEDs
PioneerDDJSX2.beatjumpPrec = [2, 2, 2, 2]; // Precision for Beatjump/Slicer

// Sampler State
PioneerDDJSX2.samplerVolume = 1.0;
PioneerDDJSX2.sampleVolume = new Array(64).fill(0.9);
PioneerDDJSX2.samplerBank = [0, 0, 0, 0]; // Offset for sampler banks (0=1-8, 1=9-16, etc)

// LOAD button hold state per channel.
// Set true on press, false on release.
// Used to intercept rotary input for QuickEffect cycling.
PioneerDDJSX2.loadHeld = [false, false, false, false];

// Tracks whether the rotary was turned while LOAD was held.
// If true on LOAD release, the normal track-load is suppressed.
PioneerDDJSX2.loadRotaryUsed = [false, false, false, false];

// Effects State
PioneerDDJSX2.currentEffect = [3, 3]; // Selected FX unit
PioneerDDJSX2.currentEffectparamset = [0, 0, 0, 0, 0, 0, 0, 0];

// Kill/mute state for EQ and Filter knobs, per deck (0-3).
// Each knob tracks:
//   killed        — whether the kill is currently active
//   anchor        — the knob position at the start of the current shift gesture,
//                   used to measure how far the knob has moved
//   lastToggleDir — the direction ("up"/"down") of the most recent toggle,
//                   used to prevent re-triggering while continuing the same movement
PioneerDDJSX2.killState = {};
for (let i = 0; i < 4; i++) {
    PioneerDDJSX2.killState[i] = {
        eqHigh: {
            killed: false,
            anchor: null,
            lastToggleDir: null,
        },
        eqMid: {
            killed: false,
            anchor: null,
            lastToggleDir: null,
        },
        eqLow: {
            killed: false,
            anchor: null,
            lastToggleDir: null,
        },
        filter: {
            killed: false,
            anchor: null,
            lastToggleDir: null,
        },
    };
}

// Snap window for position-based restores (center and minimum detection).
PioneerDDJSX2.KillTolerance = 0.005;

// Minimum knob travel (as a 0.0-1.0 fraction of the full range) required
// to trigger or restore a kill while shift is held. Prevents MIDI jitter
// and the MSB/LSB message pairs the SX2 sends per tick from double-firing.
PioneerDDJSX2.KillThreshold = 0.005;

// Returns true if any of the four shift buttons is currently held.
// Used so kills can be triggered from either side of the controller.
PioneerDDJSX2.anyShiftHeld = function() {
    return PioneerDDJSX2.shiftState.some(function(v) { return v > 0; });
};

// 14-bit MIDI Cache
// Stores the MSB (Most Significant Byte) to combine with LSB for high-res control
PioneerDDJSX2.midi14bit = {
    tempo: {
        0: 0,
        1: 0,
        2: 0,
        3: 0
    },
    volume: {
        0: 0,
        1: 0,
        2: 0,
        3: 0
    },
    eqHigh: {
        0: 0,
        1: 0,
        2: 0,
        3: 0
    },
    eqMid: {
        0: 0,
        1: 0,
        2: 0,
        3: 0
    },
    eqLow: {
        0: 0,
        1: 0,
        2: 0,
        3: 0
    },
    filter: {
        0: 0,
        1: 0,
        2: 0,
        3: 0
    },
    pregain: {
        0: 0,
        1: 0,
        2: 0,
        3: 0
    }
};

// Settings (Pulled from Preferences or Defaults)
PioneerDDJSX2.settings = {
    alpha: 1.0 / 8,
    beta: (1.0 / 8) / 32,
    jogResolution: 2054, // Resolution per rotation for scratching
    vinylSpeed: 33 + 1 / 3,
    loopIntervals: ["0.03125", "0.0625", "0.125", "0.25", "0.5", "1", "2", "4", "8", "16", "32", "64"],
    tempoRanges: [0.08, 0.16, 0.32, 0.64],
    rollColors: [0x19, 0x20, 0x13, 0x0e, 0x05],
    beatjumpColors: [0x3c, 0x3A, 0x38, 0x36, 0x34, 0x32, 0x30, 0x2e, 0x2c, 0x2a, 0x28],
    cueLoopColors: [0x30, 0x35, 0x3A, 0x01, 0x05, 0x0a, 0x10, 0x15, 0x1a, 0x24, 0x27, 0x2a],
    NeedleSearchBehaviour: engine.getSetting("NeedleSearchBehaviour"),
    LoadBehaviour: engine.getSetting("LoadBehaviour"),
    KeySyncBehaviour: engine.getSetting("KeySyncBehaviour"),
    SoftTakeoverBehaviour: engine.getSetting("SoftTakeoverBehaviour"),
    safeScratchTimeout: engine.getSetting("safeScratchTimeout"),
    CenterRedLightsBehavior: engine.getSetting("CenterRedLightsBehavior"),
    CenterWhiteLightsBehavior: engine.getSetting("CenterWhiteLightsBehavior"), // 0 for rotations, 1 for duration
    DoNotTrickController: engine.getSetting("DoNotTrickController"),
    SoftStartTime: engine.getSetting("SoftStartTime"),
    BrakeTime: engine.getSetting("BrakeTime"),
    UseShiftToBreak: engine.getSetting("UseShiftToBreak")
};

// Groups for efficient lookup
PioneerDDJSX2.groups = {
    channelGroups: {
        "[Channel1]": 0x00,
        "[Channel2]": 0x01,
        "[Channel3]": 0x02,
        "[Channel4]": 0x03
    },
    samplerGroups: {},
    hotcueIndex: {}
};

// Populate Enums dynamically
for (var i = 0; i < 64; i++) {
    PioneerDDJSX2.groups.samplerGroups[`[Sampler${  i + 1  }]`] = i;
}
for (let i = 1; i <= 24; i++) {
    PioneerDDJSX2.groups.hotcueIndex[`hotcue_${  i  }_status`] = i;
}

// Tracks which channel is currently active on each physical side
// Side A (left deck):  ch1 or ch3
// Side B (right deck): ch2 or ch4
PioneerDDJSX2.activeDeck = {A: 1, B: 2};

// =============================================================================
// 3. INITIALIZATION & SHUTDOWN
// =============================================================================

// Called when the controller is detected or the script is reloaded.
PioneerDDJSX2.init = function(id) {
    // Send Serato Recall Sysex (Forces controller into correct mode)
    midi.sendSysexMsg(PioneerDDJSX2.recallState, PioneerDDJSX2.recallState.length);

    // Enable soft-takeover for all CC controls.
    if (PioneerDDJSX2.settings.SoftTakeoverBehaviour === false) {
        // Waits 3 seconds for the hardware position's burst, then enable soft takeover
        engine.beginTimer(3000, function() {

            // -- FX Knobs --
            for (var j = 1; j <= 4; j++) {
                engine.softTakeover("[EffectRack1_EffectUnit1_Effect1]", `parameter${  j}`, true);
                engine.softTakeover("[EffectRack1_EffectUnit2_Effect1]", `parameter${  j}`, true);
                engine.softTakeover("[EffectRack1_EffectUnit1_Effect2]", `parameter${  j}`, true);
                engine.softTakeover("[EffectRack1_EffectUnit2_Effect2]", `parameter${  j}`, true);
                engine.softTakeover("[EffectRack1_EffectUnit1_Effect3]", `parameter${  j}`, true);
                engine.softTakeover("[EffectRack1_EffectUnit2_Effect3]", `parameter${  j}`, true);
            }

            // -- META Knobs --
            engine.softTakeover("[EffectRack1_EffectUnit1_Effect1]", "meta", true);
            engine.softTakeover("[EffectRack1_EffectUnit2_Effect1]", "meta", true);
            engine.softTakeover("[EffectRack1_EffectUnit1_Effect2]", "meta", true);
            engine.softTakeover("[EffectRack1_EffectUnit2_Effect2]", "meta", true);
            engine.softTakeover("[EffectRack1_EffectUnit1_Effect3]", "meta", true);
            engine.softTakeover("[EffectRack1_EffectUnit2_Effect3]", "meta", true);

            // -- Per Channel Controls --
            for (var i = 1; i <= 4; i++) {

                // Filter Knobs
                engine.softTakeover(`[QuickEffectRack1_[Channel${  i  }]]`, "super1", true);

                // Channel Faders
                engine.softTakeover(`[Channel${  i  }]`, "volume", true);

                // EQ Knobs
                engine.softTakeover(`[EqualizerRack1_[Channel${  i  }]_Effect1]`, "parameter1", true);
                engine.softTakeover(`[EqualizerRack1_[Channel${  i  }]_Effect1]`, "parameter2", true);
                engine.softTakeover(`[EqualizerRack1_[Channel${  i  }]_Effect1]`, "parameter3", true);

                // Pregain/Trim
                engine.softTakeover(`[Channel${  i  }]`, "pregain", true);

                // Tempo Slider
                engine.softTakeover(`[Channel${  i  }]`, "rate", true);
            }

            //--Crossfader--
            engine.softTakeover("[Master]", "crossfader", true);
        }, true);
    }

    // Startup Animation (Sends lights to the Master Level Meter)
    midi.sendShortMsg(0x90, 0x0B, 0x10);
    midi.sendShortMsg(0x91, 0x0B, 0x10);
    midi.sendShortMsg(0x92, 0x0B, 0x10);
    midi.sendShortMsg(0x93, 0x0B, 0x10);

    // Bind Mixxx Controls to JS functions using engine.makeConnection
    PioneerDDJSX2.BindControlConnections();

    // Turn off Deck Lights initially to ensure clean state
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0xBB, i, 0);
    }

    // Initialize Deck defaults (Vinyl mode on, Tempo range reset)
    for (let i = 0; i < 4; i++) {
        // Slip LED
        midi.sendShortMsg(0x90 + i, 0x0D, 0x7F);
        engine.setValue(`[Channel${  i + 1  }]`, "rateRange", PioneerDDJSX2.settings.tempoRanges[PioneerDDJSX2.tempoRange[i]]);
        PioneerDDJSX2.RepaintSampler(i);
    }

    PioneerDDJSX2.FXLeds();

    // Restore Hotcues LED state from engine
    for (let i = 1; i <= 4; i++) {
        for (var j = 1; j <= 16; j++) {
            PioneerDDJSX2.HotCuePerformancePadLed(engine.getValue(`[Channel${  i  }]`, `hotcue_${  j  }_status`), `[Channel${  i  }]`, `hotcue_${  j  }_status`);
        }
    }

    // Enable Effects Units for all channels by default
    // Left Deck : only effect bank no. 1  --  Right Deck : only effect bank no. 2
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 1);
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel3]_enable", 1);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 1);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel4]_enable", 1);

    // Start Blink/Keep-Alive Timer (250ms interval)
    PioneerDDJSX2.lightsTimer = engine.beginTimer(250, PioneerDDJSX2.doTimer, 0);

    // Initialize 14-bit cache structures if undefined
    if (typeof PioneerDDJSX2.midi14bit === "undefined") {
        PioneerDDJSX2.midi14bit = {};
    }
    if (typeof PioneerDDJSX2.midi14bit.pregain === "undefined") {
        PioneerDDJSX2.midi14bit.pregain = [0, 0, 0, 0]; // For 4 channels
    }

};

// Called when the controller is disconnected or script is stopped.
PioneerDDJSX2.shutdown = function() {
    PioneerDDJSX2.UnbindControlConnections();
    // Turn off center lights
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0xBB, i, 0);
    };

    // Reset Load Lights
    midi.sendShortMsg(0x96, 0x46, 0x7F);
    midi.sendShortMsg(0x96, 0x47, 0x7F);
    midi.sendShortMsg(0x96, 0x48, 0x00);
    midi.sendShortMsg(0x96, 0x49, 0x00);

    // Reset to HOT CUE Mode + LED
    // For All Channel
    midi.sendShortMsg(0x90, 0x1B, 0x7F);
    midi.sendShortMsg(0x91, 0x1B, 0x7F);
    midi.sendShortMsg(0x92, 0x1B, 0x7F);
    midi.sendShortMsg(0x93, 0x1B, 0x7F);

    // Reset All Pad LEDs
    // For All 4 channels
    for (let i = 0; i < 8; i++) {
        midi.sendShortMsg(0x97, 0x00 + i, 0x00);
        midi.sendShortMsg(0x98, 0x00 + i, 0x00);
        midi.sendShortMsg(0x99, 0x00 + i, 0x00);
        midi.sendShortMsg(0x9A, 0x00 + i, 0x00);
    }

    // Stop main timer
    if (PioneerDDJSX2.lightsTimer) {
        engine.stopTimer(PioneerDDJSX2.lightsTimer);
    };
};



// =============================================================================
// 4. TIMER & BLINKING
// =============================================================================

// Global timer function. Handles "Keep Alive" messages and LED blinking.
PioneerDDJSX2.doTimer = function() {
    // Send Keep Alive Sysex to prevent controller sleep/demo mode
    if (!PioneerDDJSX2.settings.DoNotTrickController) {
        midi.sendSysexMsg(PioneerDDJSX2.keepAlive, PioneerDDJSX2.keepAlive.length);
    }

    // Handle blinking LEDs (Slip Mode and Reverse) for each Deck
    for (var i = 0; i < 4; i++) {
        // Slip Blink
        if (engine.getValue(`[Channel${  i + 1  }]`, "slip_enabled")) {
            midi.sendShortMsg(0x90 + i, 0x40, PioneerDDJSX2.blinkState ? 0x7F : 0x00); // Slip Button normal
        } else {
            midi.sendShortMsg(0x90 + i, 0x40, 0x00); // Slip Button normal
        }

        // Reverse Blink
        if (PioneerDDJSX2.reverse[i]) {
            midi.sendShortMsg(0x90 + i, 0x38, PioneerDDJSX2.blinkState ? 0x7F : 0x00); // Reverse Button when shifting
            midi.sendShortMsg(0x90 + i, 0x15, PioneerDDJSX2.blinkState ? 0x7F : 0x00); // Reverse Button normal
        } else {
            midi.sendShortMsg(0x90 + i, 0x38, 0x00); // Reverse Button when shifting
            midi.sendShortMsg(0x90 + i, 0x15, 0x00); // Reverse Button normal
        }
    }
    // Toggle blink state for next tick
    PioneerDDJSX2.blinkState = !PioneerDDJSX2.blinkState;
};

// =============================================================================
// 5. CONNECTIONS (Signals)
// =============================================================================

// Binds Mixxx Engine signals (e.g., "play", "vu_meter") to script functions.
// This allows the controller LEDs to react to changes in the software UI.
PioneerDDJSX2.BindControlConnections = function() {
    for (var i = 1; i <= 4; i++) {
        var group = `[Channel${  i  }]`;

        // CRITICAL: JOGWHEEL & SLICER FEEDBACK
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "playposition", PioneerDDJSX2.DeckLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "beat_next", PioneerDDJSX2.SlicerLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "beat_active", PioneerDDJSX2.UpdateCenterBeatLights));

        // NEW: Reset beat counter when playposition changes (needle drop, hotcue jumps)
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "playposition", PioneerDDJSX2.ResetBeatCounter));

        // STANDARD INDICATORS
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "vu_meter", PioneerDDJSX2.VuMeter));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "play_indicator", PioneerDDJSX2.PlayLeds));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "sync_enabled", PioneerDDJSX2.SyncLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "cue_indicator", PioneerDDJSX2.CueLeds));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "pfl", PioneerDDJSX2.HeadphoneCueLed));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "keylock", PioneerDDJSX2.KeyLockLeds));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "loop_double", PioneerDDJSX2.LoopDouble));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "loop_halve", PioneerDDJSX2.LoopHalve));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "rate", PioneerDDJSX2.RateLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "eject", PioneerDDJSX2.UnloadLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "loop_enabled", PioneerDDJSX2.LoopLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "loop_in", PioneerDDJSX2.LoopLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "loop_out", PioneerDDJSX2.LoopLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "loop_anchor", PioneerDDJSX2.LoopLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "track_samples", PioneerDDJSX2.LoadActions));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "pitch_adjust", PioneerDDJSX2.PitchAdjust));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "repeat", PioneerDDJSX2.ParamLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "quantize", PioneerDDJSX2.QuantizeLights));

        // HOTCUES (Status and Color)
        for (var j = 1; j <= 16; j++) {
            PioneerDDJSX2.conns.push(engine.makeConnection(group, `hotcue_${  j  }_status`, PioneerDDJSX2.HotCuePerformancePadLed));
            PioneerDDJSX2.conns.push(engine.makeConnection(group, `hotcue_${  j  }_color`, PioneerDDJSX2.HotCuePerformancePadLed));
        }
    }

    // FX Connections (Enable/Disable LEDs)
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", PioneerDDJSX2.FX1CH1));
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", PioneerDDJSX2.FX2CH1));
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", PioneerDDJSX2.FX1CH2));
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", PioneerDDJSX2.FX2CH2));
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel3]_enable", PioneerDDJSX2.FX1CH3));
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel3]_enable", PioneerDDJSX2.FX2CH3));
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel4]_enable", PioneerDDJSX2.FX1CH4));
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel4]_enable", PioneerDDJSX2.FX2CH4));

    // Effect Leds
    for (let i = 1; i <= 3; i++) {
        PioneerDDJSX2.conns.push(engine.makeConnection(`[EffectRack1_EffectUnit1_Effect${  i  }]`, "enabled", PioneerDDJSX2.FXLeds));
        PioneerDDJSX2.conns.push(engine.makeConnection(`[EffectRack1_EffectUnit2_Effect${  i  }]`, "enabled", PioneerDDJSX2.FXLeds));
    }

    // Sampler Connections (Play/Load state)
    var numSamplers = engine.getValue("[App]", "num_samplers");
    for (let i = 1; i <= numSamplers; i++) {
        PioneerDDJSX2.conns.push(engine.makeConnection(`[Sampler${  i  }]`, "play", PioneerDDJSX2.SamplerLight));
        PioneerDDJSX2.conns.push(engine.makeConnection(`[Sampler${  i  }]`, "track_loaded", PioneerDDJSX2.SamplerLight));
    }
};

// Unbinds all connections. Used during Shutdown.
PioneerDDJSX2.UnbindControlConnections = function() {
    for (var i = 0; i < PioneerDDJSX2.conns.length; i++) {
        if (PioneerDDJSX2.conns[i]) {
            PioneerDDJSX2.conns[i].disconnect();
        }
    }
    PioneerDDJSX2.conns = [];
};

// =============================================================================
// 6. LIGHTING & FEEDBACK
// =============================================================================
// This calculates the rotating LED ring on the jog wheel.
// INCLUDES THROTTLING: The SX2 can freeze if too many MIDI messages are sent
// rapidly to the jog rings. This function limits updates to ~30ms.

// Handles WHITE outer ring ONLY
PioneerDDJSX2.DeckLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];

    // THROTTLE: Limit update rate to ~30ms to prevent controller freeze
    var now = Date.now();
    if (now - PioneerDDJSX2.lastLightUpdate[channel] < 30) {
        return;
    }
    PioneerDDJSX2.lastLightUpdate[channel] = now;

    var trackSamples = engine.getValue(group, "track_samples");
    var trackRate = engine.getValue(group, "track_samplerate");

    if (!trackSamples || !trackRate) {
        return;
    }

    // Calculate rotation position for WHITE outer ring
    PioneerDDJSX2.TurnTablePos[channel] = (engine.getValue(group, "playposition") * (trackSamples / trackRate) / 2);
    var finalPos = Math.floor(1 + (PioneerDDJSX2.TurnTablePos[channel] * 39.96) % 0x48);

    // This calculate the track progress to show it to the user.
    var trackProgression = Math.floor(1 + engine.getValue(group, "playposition") * 72);

    // Only send MIDI if position changed to reduce traffic
    if (finalPos !== PioneerDDJSX2.FinalTurnPos[channel]) {
        PioneerDDJSX2.FinalTurnPos[channel] = finalPos;

        // Check if the song is soon at the end
        var isEndOfTrack = engine.getValue(group, "end_of_track");

        // END-OF-TRACK WARNING BLINK (WHITE outer ring)
        if (isEndOfTrack === 1 && !engine.isScratching(channel + 1)) {
            if (PioneerDDJSX2.settings.CenterWhiteLightsBehavior === true) {
                midi.sendShortMsg(0xBB, channel, PioneerDDJSX2.blinkState ? trackProgression : 0x00);
            } else {
                midi.sendShortMsg(0xBB, channel, PioneerDDJSX2.blinkState ? finalPos : 0x00);
            }
        } else {
            // Normal playback - steady white ring rotation
            if (PioneerDDJSX2.settings.CenterWhiteLightsBehavior === true) {
                midi.sendShortMsg(0xBB, channel, trackProgression);
            } else {
                midi.sendShortMsg(0xBB, channel, finalPos);
            }
        }
    }
};

// CENTER RED JOGWHEEL LIGHTS (4 LEDs - 2 MODES ONLY)
// =============================================================================
// Handles BOTH beat modes for RED center LEDs
// MODE 0 (Checkbox OFF): Beat-by-beat - 1 LED per beat, 4 beat cycle
// MODE 1 (Checkbox ON):  Bar-by-bar - 1 LED per bar (4 beats), 4 bar cycle (16 beats)
PioneerDDJSX2.UpdateCenterBeatLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];

    if (value === 0) {
        return;
    };

    // WIP: This handles the whole red center light blinking when a hot cue is getting close (It will blink 4 times, meaning the hotcue is close by 8 beats)
    // The code is there, the blinking happens, this could probably be better, maybe also be put in a setting option later ?
    var trackSampleRate = engine.getValue(group, "track_samplerate");
    var playposition = engine.getValue(group, "playposition");
    var duration = engine.getValue(group, "duration");
    var bpm = engine.getValue(group, "file_bpm");
    // Get infos for each hotcues
    for (var i = 1; i <= 16; i++) {
        var hotcuePos = engine.getValue(group, `hotcue_${  i  }_position`);
        // Check if hotcue is defined
        if (hotcuePos >= 0 && hotcuePos !== undefined) {
            // Get the current hotcue position in the track in seconds
            var realHotCuePos = (hotcuePos / trackSampleRate / 2);
            // Get track positionn in secondes
            // playposition is a percentage of the duration between 0 and 1
            var trackPositionInSeconds = (playposition * duration);
            // Calculate beats per seconds
            var secondsPerBeat = 60 / bpm;
            // Number of beats before making the LEDs blink
            var numberOfBeats = 8;
            // We check if the hotcue is before the playhead, and just before the playhead by 4 beats
            if (realHotCuePos >= trackPositionInSeconds && realHotCuePos <= (trackPositionInSeconds + (secondsPerBeat * numberOfBeats))) {
                midi.sendShortMsg(0xBB, 0x04 + channel, PioneerDDJSX2.HotCueLEDStatus[i] ? 0 : 4);
                PioneerDDJSX2.isThereHotCue[i] = true;
                PioneerDDJSX2.HotCueLEDStatus[i] = PioneerDDJSX2.HotCueLEDStatus[i] ? 0 : 1;
            } else {
                PioneerDDJSX2.isThereHotCue[i] = false;
            }
        }
    }

    // We chech if ANY hotcue is closeby, if yes we store it so the red center light doesn't get overwritten by the bar-by-bar lights
    var isThereDefinitelyHotCue = false;
    for (var j = 1; j <= PioneerDDJSX2.isThereHotCue.length; j++) {
        if (PioneerDDJSX2.isThereHotCue[j] === true) {
            isThereDefinitelyHotCue = true;
            break;
        }
    }

    var rawSetting = engine.getSetting("CenterRedLightsBehavior");
    var barByBarMode = (rawSetting === null) ? true : Boolean(Number(rawSetting));

    // Increment beat counter
    PioneerDDJSX2.currentBeat[channel]++;
    if (!isThereDefinitelyHotCue) {
        if (barByBarMode) {
            // ===== MODE 1: BAR-BY-BAR (Checkbox ON) =====
            // 32 beat cycle (8 bars): 4 bars ON + 4 bars OFF

            if (PioneerDDJSX2.currentBeat[channel] > 32) {
                PioneerDDJSX2.currentBeat[channel] = 1;
            }

            var barNumber = Math.ceil(PioneerDDJSX2.currentBeat[channel] / 4);
            midi.sendShortMsg(0xBB, 0x04 + channel, barNumber);

        } else {
            // ===== MODE 0: BEAT-BY-BEAT (Checkbox OFF, default) =====
            // 8 beat cycle: 4 beats ON + 4 beats OFF

            if (PioneerDDJSX2.currentBeat[channel] > 8) {
                PioneerDDJSX2.currentBeat[channel] = 1;
            }

            midi.sendShortMsg(0xBB, 0x04 + channel, PioneerDDJSX2.currentBeat[channel]);
        }
    }
};

// Reset beat counter when track loads OR playposition changes
PioneerDDJSX2.LoadActions = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    // Reset beat counter to 0 so first beat = LED 1
    PioneerDDJSX2.currentBeat[channel] = 0;

    // Turn off center red LEDs
    midi.sendShortMsg(0xBB, 0x04 + channel, 0);

    // Light up load button
    midi.sendShortMsg(0x9B, channel, 0x7F);
};

// Update Lights for the Parameters buttons
// After some tests, it seems we CANNOT set the lights on Parameter 1 when we send the Serato Keepalive command
// The lights will just blink when we press the buttons
// Except for the CUE mode where we can set the lights as static.
PioneerDDJSX2.ParamLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    // For the repeat setting of each channels
    // SHIFT + Rec Flip Button
    var isRepeat = engine.getValue(group, "repeat");
    midi.sendShortMsg(0x90 + channel, 0x5A, isRepeat ? 0x7F : 0x00);
};

// Function to set PAD MODE Leds when needed
PioneerDDJSX2.QuantizeLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];

    var isQuantize = engine.getValue(group, "quantize");
    midi.sendShortMsg(0x90 + channel, 0x79, isQuantize ? 0x7F : 0x00);
};

// Track last playposition to detect manual jumps
PioneerDDJSX2.lastPlayPosition = [0, 0, 0, 0];

PioneerDDJSX2.ResetBeatCounter = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    var lastPos = PioneerDDJSX2.lastPlayPosition[channel];

    // Only reset if this is a JUMP (difference > 0.001 = ~0.1% of track)
    // Normal playback moves very smoothly, jumps are much larger
    var positionDiff = Math.abs(value - lastPos);

    if (positionDiff > 0.001) {
        // This is a manual jump (needle drop, hotcue, waveform click)
        PioneerDDJSX2.currentBeat[channel] = 0;
        midi.sendShortMsg(0xBB, 0x04 + channel, 0);
    }

    // Store current position for next comparison
    PioneerDDJSX2.lastPlayPosition[channel] = value;
};

// Slicer Lights
// Handles the chasing lights on the pads when Slicer mode is active.
PioneerDDJSX2.slicerTick = [0, 0, 0, 0];

PioneerDDJSX2.SlicerLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];

    // Exit immediately if not in Slicer mode
    if (PioneerDDJSX2.padMode[channel] !== 2 && PioneerDDJSX2.padMode[channel] !== 6) {
        return;
    }

    PioneerDDJSX2.slicerTick[channel] = (PioneerDDJSX2.slicerTick[channel] + 1) % 8;

    var slicePosition = PioneerDDJSX2.slicerTick[channel];

    // Colors
    var isLoop = (PioneerDDJSX2.padMode[channel] === 6);
    var offset = isLoop ? 0x60 : 0x20;
    var onColor = isLoop ? 0x28 : 0x01;
    var offColor = isLoop ? 0x01 : 0x28;

    // Update all 8 pads
    for (var i = 0; i < 8; i++) {
        var isActive = (slicePosition === i);
        midi.sendShortMsg(0x97 + channel, offset + i, isActive ? onColor : offColor);
    }
};

// Updates the Performance Pads for Hotcues and Hotcues Loops
PioneerDDJSX2.HotCuePerformancePadLed = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    var i = PioneerDDJSX2.groups.hotcueIndex[control];

    if (i === undefined || value === 2) {
        return;
    }

    var color = 0;
    if (value === 1) {
        // Map Mixxx Hotcue color to Pioneer MIDI Color
        color = PioneerDDJSX2.padColors.getValueForNearestColor(engine.getValue(group, `hotcue_${  i  }_color`));
    }

    if (i <= 8) {
        midi.sendShortMsg(0x97 + channel, 0x00 + i-1, color);
        midi.sendShortMsg(0x97 + channel, 0x08 + i-1, color);
    } else if (i > 8 && i <= 16) {
        midi.sendShortMsg(0x97 + channel, 0x40 + (i-1) % 8, color);
        midi.sendShortMsg(0x97 + channel, 0x48 + (i-1) % 8, color);
    }
};

// Updates Play/Pause Button LED
PioneerDDJSX2.PlayLeds = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x0B, value ? 0x7F : 0x00);
    midi.sendShortMsg(0x90 + channel, 0x47, value ? 0x7F : 0x00);
    if (PioneerDDJSX2.settings.DoNotTrickController) {
        midi.sendShortMsg(0x9B, 0x0c + channel, value ? 0x7F : 0x00);
    }
};

// Updates VU Meter LEDs
PioneerDDJSX2.VuMeter = function(value, group, control) {
    var level = value * 0x7F;
    var channel = 0xB0 + PioneerDDJSX2.groups.channelGroups[group];
    midi.sendShortMsg(channel, 0x02, level);
};

// Updates Sync Button LED
PioneerDDJSX2.SyncLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x58, value ? 0x7F : 0x00);
};

// Clears all LEDs on a specific deck (used on Eject)
PioneerDDJSX2.UnloadLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];

    // Clear all pads LEDs
    for (var k = 0; k < 0x30; k++) {
        midi.sendShortMsg(0x97 + channel, k, 0x00);
    }
    for (let k = 0x40; k < 0x70; k++) {
        midi.sendShortMsg(0x97 + channel, k, 0x00);
    }

    midi.sendShortMsg(0xBB, channel, 0);
    midi.sendShortMsg(0xBB, 4 + channel, 0);
};

// Updates Headphone Cue (PFL) LED
PioneerDDJSX2.HeadphoneCueLed = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x54, value ? 0x7F : 0x00);
};

// Updates Main Cue Button LED
PioneerDDJSX2.CueLeds = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x48, value ? 0x7F : 0x00);
    midi.sendShortMsg(0x90 + channel, 0x0C, value ? 0x7F : 0x00);
};

// Updates Key Lock / Master Tempo LED
PioneerDDJSX2.KeyLockLeds = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x1A, value ? 0x7F : 0x00);
};

// Updates Loop 2X LED
PioneerDDJSX2.LoopDouble = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x13, value ? 0x7F : 0x00);
};

// Updates Loop 1/2 LED
PioneerDDJSX2.LoopHalve = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x12, value ? 0x7F : 0x00);
};

// Updates Loop In/Out/Loop Active LEDs
PioneerDDJSX2.LoopLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    // Auto Loop Light
    midi.sendShortMsg(0x90 + channel, 0x14, engine.getValue(group, "loop_enabled") ? 0x7F : 0x00);

    // Loop In
    midi.sendShortMsg(0x90 + channel, 0x10, (engine.getValue(group, "loop_start_position") > -1) ? 0x7F : 0x00);

    // Loop Out
    midi.sendShortMsg(0x90 + channel, 0x11, (engine.getValue(group, "loop_end_position") > -1) ? 0x7F : 0x00);

    // For the loop anchor of each channels
    // Reloop Exit Button
    var isAnchor = engine.getValue(group, "loop_anchor");
    midi.sendShortMsg(0x90 + channel, 0x4D, isAnchor ? 0x7F : 0x00);
};

// Updates Key Shift (Pitch Adjust) LEDs
PioneerDDJSX2.PitchAdjust = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    if (value !== 0) {
        if (value > 0) {
            midi.sendShortMsg(0x90 + channel, 0x4a, 0x00);
            midi.sendShortMsg(0x90 + channel, 0x4b, 0x7F);
        } else {
            midi.sendShortMsg(0x90 + channel, 0x4a, 0x7F);
            midi.sendShortMsg(0x90 + channel, 0x4b, 0x00);
        }
    } else {
        midi.sendShortMsg(0x90 + channel, 0x4a, 0x00);
        midi.sendShortMsg(0x90 + channel, 0x4b, 0x00);
    }
};

// FX Status LEDs
PioneerDDJSX2.SetFXLed = function(note) {
    return function(value, group, control) {
        midi.sendShortMsg(0x96, note, value ? 0x7F : 0x00);
    };
};
PioneerDDJSX2.FX1CH1 = PioneerDDJSX2.SetFXLed(0x4C);
PioneerDDJSX2.FX2CH1 = PioneerDDJSX2.SetFXLed(0x50);
PioneerDDJSX2.FX1CH2 = PioneerDDJSX2.SetFXLed(0x4D);
PioneerDDJSX2.FX2CH2 = PioneerDDJSX2.SetFXLed(0x51);
PioneerDDJSX2.FX1CH3 = PioneerDDJSX2.SetFXLed(0x4E);
PioneerDDJSX2.FX2CH3 = PioneerDDJSX2.SetFXLed(0x52);
PioneerDDJSX2.FX1CH4 = PioneerDDJSX2.SetFXLed(0x4F);
PioneerDDJSX2.FX2CH4 = PioneerDDJSX2.SetFXLed(0x53);

// Updates Rate/Tempo Slider direction LEDs (Up/Down arrows)
PioneerDDJSX2.RateLights = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    if (engine.getValue(group, "rate") === 0) {
        midi.sendShortMsg(0x90 + channel, 0x37, 0x00);
        midi.sendShortMsg(0x90 + channel, 0x34, 0x00);
        return;
    }
    if (engine.getValue(group, "rate") < 0) {
        midi.sendShortMsg(0x90 + channel, 0x34, 0x7F);
        midi.sendShortMsg(0x90 + channel, 0x37, 0x00);
    } else {
        midi.sendShortMsg(0x90 + channel, 0x37, 0x7F);
        midi.sendShortMsg(0x90 + channel, 0x34, 0x00);
    }
};

// Updates FX Unit On/Off LEDs
PioneerDDJSX2.FXLeds = function() {
    for (var i = 0; i < 2; i++) {
        for (var j = 0; j < 3; j++) {
            midi.sendShortMsg(0x94 + i, 0x47 + j, engine.getValue(`[EffectRack1_EffectUnit${  i + 1  }_Effect${  j + 1  }]`, "enabled") ? 0x7F : 0x00);
        }
        midi.sendShortMsg(0x94 + i, 0x4a, engine.getValue(`[EffectRack1_EffectUnit${  i + 1  }]`, "mix_mode") ? 0x7F : 0x00);
    }
};

// Feedback for Roll Pads (Unused)
PioneerDDJSX2.RollPerformancePadLed = function(value, group, control) {
    var channel = PioneerDDJSX2.groups.channelGroups[group];
    var padIndex = 0;
    for (var i = 0; i < 8; i++) {
        if (control === `beatloop_${  PioneerDDJSX2.settings.loopIntervals[i + 2]  }_enabled`) {
            break;
        }
        padIndex++;
    }
    if (engine.getValue("[Channel1]", "play")) {
        midi.sendShortMsg(0x97 + channel, 0x10 + padIndex, value ? 0x7F : 0x00);
    }
};

// =============================================================================
// 7. JOGWHEEL & SCRATCHING
// =============================================================================

// Calculates the delta movement of the jogwheel
PioneerDDJSX2.getJogWheelDelta = function(value) {
    return value - 0x40;
};

// Enables/Disables scratching engine for a specific deck
PioneerDDJSX2.toggleScratch = function(channel, isEnabled) {
    var deck = channel + 1;
    PioneerDDJSX2.channels[channel].disableScratchTimer = 0;
    if (isEnabled) {
        engine.scratchEnable(deck, PioneerDDJSX2.settings.jogResolution, PioneerDDJSX2.settings.vinylSpeed, PioneerDDJSX2.settings.alpha, PioneerDDJSX2.settings.beta);
    } else {
        engine.scratchDisable(deck);
    }
};

// Handles pitch bending (nudging) via the outer jog ring
PioneerDDJSX2.pitchBend = function(channel, movement) {
    var deck = channel + 1;
    movement = movement / 5;
    movement = movement > 3 ? 3 : movement;
    movement = movement < -3 ? -3 : movement;
    engine.setValue(`[Channel${  deck  }]`, "jog", movement);
};

// Safety timer: Disables scratch if touch is lost
PioneerDDJSX2.scheduleDisableScratch = function(channel) {
    PioneerDDJSX2.channels[channel].disableScratchTimer = engine.beginTimer(PioneerDDJSX2.settings.safeScratchTimeout, function() {
        PioneerDDJSX2.toggleScratch(channel, false);
    }, true);
};

PioneerDDJSX2.unscheduleDisableScratch = function(channel) {
    if (PioneerDDJSX2.channels[channel].disableScratchTimer) {
        engine.stopTimer(PioneerDDJSX2.channels[channel].disableScratchTimer);
    }
};

PioneerDDJSX2.postponeDisableScratch = function(channel) {
    PioneerDDJSX2.unscheduleDisableScratch(channel);
    PioneerDDJSX2.scheduleDisableScratch(channel);
};

// Touched the Jog Platter (top surface)
PioneerDDJSX2.jogScratchTouch = function(channel, control, value) {
    if (value === 0x7F && PioneerDDJSX2.vinylOn[channel]) {
        PioneerDDJSX2.unscheduleDisableScratch(channel);
        PioneerDDJSX2.toggleScratch(channel, true);
    } else {
        PioneerDDJSX2.scheduleDisableScratch(channel);
    }
};

// Turned the Jog Platter (top surface) - Scratching or Grid Adjust
PioneerDDJSX2.jogScratchTurn = function(channel, control, value) {
    var deck = channel + 1;
    if (engine.isScratching(deck) && !PioneerDDJSX2.gridSlide[channel] && !PioneerDDJSX2.gridAdjust[channel]) {
        engine.scratchTick(deck, PioneerDDJSX2.getJogWheelDelta(value));
    } else {
        if (PioneerDDJSX2.gridSlide[channel]) {
            engine.setValue(`[Channel${  deck  }]`, (value < 64) ? "beats_translate_earlier" : "beats_translate_later", 1);
        }
        if (PioneerDDJSX2.gridAdjust[channel]) {
            engine.setValue(`[Channel${  deck  }]`, (value < 64) ? "beats_adjust_faster" : "beats_adjust_slower", 1);
        }
    }
};

// Outer Jog Ring Touch (Seeking)
PioneerDDJSX2.jogSeekTouch = function(channel, control, value) {
    if (!engine.getValue(`[Channel${  channel + 1  }]`, "play")) {
        PioneerDDJSX2.toggleScratch(channel, value === 0x7F);
    }
};

// Outer Jog Ring Turn (Seeking or Nudging)
PioneerDDJSX2.jogSeekTurn = function(channel, control, value) {
    var deck = channel + 1;
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, PioneerDDJSX2.getJogWheelDelta(value));
    } else {
        PioneerDDJSX2.pitchBend(channel, PioneerDDJSX2.getJogWheelDelta(value));
    }
};

// Seek through track (Shift + Jog Turn)
PioneerDDJSX2.jogSeek = function(channel, control, value) {
    engine.setValue(`[Channel${  channel + 1  }]`, "beatjump", PioneerDDJSX2.getJogWheelDelta(value) / 16);
};

PioneerDDJSX2.jogPitchBend = function(channel, control, value) {
    var deck = channel + 1;
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, PioneerDDJSX2.getJogWheelDelta(value));
        PioneerDDJSX2.postponeDisableScratch(channel);
    } else {
        if (engine.getValue(`[Channel${  deck  }]`, "play")) {
            PioneerDDJSX2.pitchBend(channel, PioneerDDJSX2.getJogWheelDelta(value));
        }
    }
};

// Toggle Vinyl Mode (Shift + Slip)
PioneerDDJSX2.ToggleVinyl = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.vinylOn[channel] = !PioneerDDJSX2.vinylOn[channel];
    }
};

// =============================================================================
// 8. MIXER & TRANSPORT (14-bit Support)
// =============================================================================

// Play/Pause Button
// Handles Logic for SoftStart/Braking if Shift is pressed/configured
PioneerDDJSX2.Play = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var isPlaying = engine.getValue(group, "play");
    var lastHotcue = engine.getValue(group, "hotcue_focus");
    if (lastHotcue !== 0) {
        var isLastHotcuePlaying = (lastHotcue !== -1) ? engine.getValue(group, `hotcue_${  lastHotcue  }_status`) : 0;
    };

    if (value === 0x7F) {
        if (((control === 71 && PioneerDDJSX2.settings.UseShiftToBreak === false) || (control !== 71 && PioneerDDJSX2.settings.UseShiftToBreak === true)) && isLastHotcuePlaying !== 2) {
            if (isPlaying && PioneerDDJSX2.isBraking === 0) {
                PioneerDDJSX2.isBraking = 1;
                if (PioneerDDJSX2.settings.BrakeTime >= 0) {
                    engine.brake(deck, true, PioneerDDJSX2.settings.BrakeTime);
                } else {
                    engine.setValue(group, "play", 0);
                }
            } else if (isPlaying && PioneerDDJSX2.isBraking === 1) {
                PioneerDDJSX2.isBraking = 0;
                if (PioneerDDJSX2.settings.SoftStartTime >= 0) {
                    engine.softStart(deck, true, PioneerDDJSX2.settings.SoftStartTime);
                } else {
                    engine.brake(deck, false);
                    engine.softStart(deck, false);
                }
            } else {
                PioneerDDJSX2.isBraking = 0;
                if (PioneerDDJSX2.settings.SoftStartTime >= 0) {
                    engine.softStart(deck, true, PioneerDDJSX2.settings.SoftStartTime);
                } else {
                    engine.setValue(group, "play", 1);
                }
            }
        } else {
            PioneerDDJSX2.isBraking = 0;
            engine.setValue(group, "play", !isPlaying);
        }
    }
};

// Deck Select Buttons (7-10 on the controller)
// Directly assigns the active deck per physical side when pressed.
PioneerDDJSX2.DeckToggle = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (channel === 0 || channel === 2) {
            PioneerDDJSX2.activeDeck.A = channel + 1; // 0+1=ch1, 2+1=ch3
        } else {
            PioneerDDJSX2.activeDeck.B = channel + 1; // 1+1=ch2, 3+1=ch4
        }
    }
};

PioneerDDJSX2.NeedleSearch = function(channel, control, value, status, group) {
    // For isShifted, 3 = Not Shifting ; 28 = Shifting
    var isShifted = control;
    var isPlaying = engine.getValue(group, "play");

    // var actualPlayPosition = 1/value;
    var actualPlayPosition = value / 0x7F;

    // Check if it is Shifted and apply the NeedleSearchBehaviour setting if the song is playing
    if (isShifted === 3 && isPlaying) {
        if (PioneerDDJSX2.settings.NeedleSearchBehaviour === true) {
            engine.setValue(group, "playposition", actualPlayPosition);
        }
    } else {
        engine.setValue(group, "playposition", actualPlayPosition);
    }
};

// Sync Button (Long press logic)
// Quick press : Enable beatsync for the current deck
// Long press (>1000ms) : enable sync behaviour of syncing both left and right tracks
PioneerDDJSX2.SyncEnable = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.SyncEnableTimer = engine.beginTimer(1000, function() {
            if (channel === 0 || channel === 2) {
                engine.setValue(`[Channel${  channel + 1  }]`, "sync_enabled", 1);
                engine.setValue(`[Channel${  channel + 2  }]`, "sync_enabled", 1);
            } else {
                engine.setValue(`[Channel${  channel + 1  }]`, "sync_enabled", 1);
                engine.setValue(`[Channel${  channel  }]`, "sync_enabled", 1);
            }
            PioneerDDJSX2.SyncEnableTimer = null;
        }, 1);
    } else {
        if (PioneerDDJSX2.SyncEnableTimer !== null) {
            engine.setValue(`[Channel${  channel + 1  }]`, "beatsync", 1);
            engine.stopTimer(PioneerDDJSX2.SyncEnableTimer);
        }
    }
};

// Sync Disable (Shift + Sync)
PioneerDDJSX2.SyncDisable = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (channel === 0 || channel === 2) {
            engine.setValue(`[Channel${  channel + 1  }]`, "sync_enabled", 0);
            engine.setValue(`[Channel${  channel + 2  }]`, "sync_enabled", 0);
        } else {
            engine.setValue(`[Channel${  channel + 1  }]`, "sync_enabled", 0);
            engine.setValue(`[Channel${  channel  }]`, "sync_enabled", 0);
        }
    }
};

// Slip Button
PioneerDDJSX2.SlipEnabled = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (engine.getValue(`[Channel${  channel + 1  }]`, "play")) {

            engine.setValue(`[Channel${  channel + 1  }]`, "slip_enabled", !engine.getValue(`[Channel${  channel + 1  }]`, "slip_enabled"));
        } else {
            engine.setValue(`[Channel${  channel + 1  }]`, "slip_enabled", !engine.getValue(`[Channel${  channel + 1  }]`, "slip_enabled"));
        }
    }
};

// Crossfader Curve Adjust (Front panel knob)
PioneerDDJSX2.CrossfaderCurve = function(value, group, control) {
    engine.setValue("[Mixer Profile]", "xFaderCurve", control / 16);
};

// Input Select Switches (Front panel)
PioneerDDJSX2.InputSelect = function(channel, control, value, status, group) {
    engine.setValue(`[Channel${  channel + 1  }]`, "mute", value ? 1 : 0);
};

// Loop In Button
PioneerDDJSX2.LoopIn = function(channel, control, value, status, group) {
    engine.setValue(`[Channel${  channel + 1  }]`, "loop_in", value ? 1 : 0);
    if (value === 0x7F) {
        PioneerDDJSX2.closestBeatToLoopIn[channel] = engine.getValue(`[Channel${  channel + 1  }]`, "beat_closest");
    }
};

// 4 Beat Loop Button (Above Loop IN)
// I think this feature could be enhanced, it is a bit finicky I think...
// Needs ideas
PioneerDDJSX2.FourBeat = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        var loop_enable = engine.getValue(channel, "loop_enabled");

        engine.setValue(group, "loop_start_position", PioneerDDJSX2.closestBeatToLoopIn[channel]);
        engine.setValue(group, "loop_end_position", PioneerDDJSX2.closestBeatToLoopIn[channel] + engine.getValue(group, "track_samplerate") * (480 / engine.getValue(group, "file_bpm")));

        // If loop is not enabled, we enable it
        if (!loop_enable) {
            engine.setValue(channel, "reloop_toggle", 1);
        }
    }
};

// Handle the Reloop/Exit Loop button
// This toggles the Loop Anchor state
PioneerDDJSX2.ReloopExit = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        script.toggleControl(group, "loop_anchor");
        var isAnchor = engine.getValue(group, "loop_anchor");
    }
};


// Slip Mode Logic
PioneerDDJSX2.SlipMode = function(channel, control, value, status, group) {
    if (engine.getValue(control, "play")) {
        midi.sendShortMsg(0x90 + channel, 0x40, channel ? 0x7F : 0x00);
    }
};

// Shift Button
// Records shift state and clears kill gesture memory on release.
// Clearing on release means each new shift press starts a fresh gesture —
// the knob position at the moment shift is pressed becomes the new anchor.
PioneerDDJSX2.Shift = function(channel, _control, value, _status, _group) {
    PioneerDDJSX2.shiftState[channel] = value;
    if (value === 0) {
        const knobTypes = ["eqHigh", "eqMid", "eqLow", "filter"];
        for (let i = 0; i < 4; i++) {
            for (let k = 0; k < knobTypes.length; k++) {
                const state = PioneerDDJSX2.killState[i][knobTypes[k]];
                state.anchor = null;
                state.lastToggleDir = null;
            }
        }
    }
};

// Reverse Button (Toggle)
PioneerDDJSX2.Reverse = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.reverse[channel] = !PioneerDDJSX2.reverse[channel];
        engine.setValue(`[Channel${  channel + 1  }]`, "reverse", PioneerDDJSX2.reverse[channel]);
    }
};

// Reverse Button (Hold - Censor)
PioneerDDJSX2.ReverseHold = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.reverse[channel] = 1;
        engine.setValue(`[Channel${  channel + 1  }]`, "reverse", PioneerDDJSX2.reverse[channel]);
    } else {
        PioneerDDJSX2.reverse[channel] = 0;
        engine.setValue(`[Channel${  channel + 1  }]`, "reverse", PioneerDDJSX2.reverse[channel]);
    }
};

// Auto Loop Button
PioneerDDJSX2.AutoLoop = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (engine.getValue(`[Channel${  channel + 1  }]`, "loop_enabled")) {
            engine.setValue(`[Channel${  channel + 1  }]`, "reloop_toggle", 1);
        } else {
            engine.setValue(`[Channel${  channel + 1  }]`, "beatloop_activate", 1);
        }
    }
};

// --- 14-BIT MIDI IMPLEMENTATION ---
// The SX2 sends high-resolution MIDI using two messages (MSB and LSB).
// We cache the MSB and wait for the LSB to apply the full value.

// Tempo/Rate Slider (14-bit)
PioneerDDJSX2.RateSlider = function(channel, control, value, status, group) {
    var deckGroup = `[Channel${  channel + 1  }]`;
    var storage = PioneerDDJSX2.midi14bit.tempo;
    if (control === 0x00) {
        storage[channel] = value;
        return;
    }
    if (control === 0x20) {
        var msb = storage[channel];
        var value14bit = (msb << 7) | value;
        var rate;
        // Calculate centered rate (-1 to 1)
        if (value14bit < 8192) {
            rate = (value14bit - 8192) / 8192;
        } else {
            rate = (value14bit - 8192) / 8191;
        }

        rate = -rate;
        var fileBPM = engine.getValue(deckGroup, "file_bpm");
        var tempoRange = engine.getValue(deckGroup, "rateRange");

        // BPM Quantization logic for smoother sliders
        if (fileBPM > 0 && tempoRange > 0) {
            var bpmChange = rate * tempoRange * fileBPM;
            var quantizedBpmChange = Math.round(bpmChange * 10) / 10;
            var quantizedRate = quantizedBpmChange / (tempoRange * fileBPM);
            engine.setValue(deckGroup, "rate", quantizedRate);
        } else {
            engine.setValue(deckGroup, "rate", rate);
        }
    }
};

// Channel Volume Faders (14-bit)
PioneerDDJSX2.VolumeSlider = function(channel, control, value, status, group) {
    var deckGroup = `[Channel${  channel + 1  }]`;
    var storage = PioneerDDJSX2.midi14bit.volume;
    if (control === 0x13) {
        storage[channel] = value;
        engine.setParameter(deckGroup, "volume", value / 127);
    } else if (control === 0x33) {
        var value14bit = (storage[channel] << 7) | value;
        engine.setParameter(deckGroup, "volume", value14bit / 16383.0);
    }
};

// EQ Knobs (14-bit)
PioneerDDJSX2.EQHigh = function(channel, control, value) {
    PioneerDDJSX2.handle14bitEQ(channel, control, value, PioneerDDJSX2.midi14bit.eqHigh, 0x07, 0x27, "parameter3");
};
PioneerDDJSX2.EQMid = function(channel, control, value) {
    PioneerDDJSX2.handle14bitEQ(channel, control, value, PioneerDDJSX2.midi14bit.eqMid, 0x0B, 0x2B, "parameter2");
};
PioneerDDJSX2.EQLow = function(channel, control, value) {
    PioneerDDJSX2.handle14bitEQ(channel, control, value, PioneerDDJSX2.midi14bit.eqLow, 0x0F, 0x2F, "parameter1");
};

// The SX2 sends two MIDI messages per knob tick: MSB first, then LSB.
// Kill logic runs on the LSB (full 14-bit combined) message only, so each
// physical tick is evaluated once. The MSB path caches the value and applies
// it directly (if not currently killed) to keep the UI responsive during fast turns.
// 7-bit MSB midpoint and range for scaling to [0,1]
PioneerDDJSX2.EQMSBMid = 64;
PioneerDDJSX2.EQMSBRange = 64.0;
// 14-bit midpoint and range for scaling to [0,1]
PioneerDDJSX2.EQ14BitMid = 8192;
PioneerDDJSX2.EQ14BitRange = 8192.0;

PioneerDDJSX2.handle14bitEQ = function(channel, control, value, storage, msbCC, lsbCC, param) {
    const group = `[EqualizerRack1_[Channel${ channel + 1 }]_Effect1]`;
    const knobTypeMap = {
        "parameter3": "eqHigh",
        "parameter2": "eqMid",
        "parameter1": "eqLow",
    };

    var knobType = knobTypeMap[param];

    if (control === msbCC) {
        storage[channel] = value;
        if (!PioneerDDJSX2.killState[channel][knobType].killed) {
            engine.setParameter(group, param, ((value - PioneerDDJSX2.EQMSBMid) / PioneerDDJSX2.EQMSBRange + 1) / 2);
        }
    } else if (control === lsbCC) {
        const value14bit = (storage[channel] << 7) | value;
        const paramValue = ((value14bit - PioneerDDJSX2.EQ14BitMid) / PioneerDDJSX2.EQ14BitRange + 1) / 2;
        if (PioneerDDJSX2.handleEQKill(channel, knobType, param, paramValue)) {
            return;
        }
        engine.setParameter(group, param, paramValue);
    }
};

// Filter knob MSB CC range (channels 1-4)
PioneerDDJSX2.FilterMSBBaseCC = 0x17; // first filter MSB CC
PioneerDDJSX2.FilterMSBMaxCC  = 0x1A; // last filter MSB CC
// Filter knob LSB CC range (channels 1-4)
PioneerDDJSX2.FilterLSBBaseCC = 0x37; // first filter LSB CC
PioneerDDJSX2.FilterLSBMaxCC  = 0x3A; // last filter LSB CC

// Filter Knobs (14-bit)
// Same MSB/LSB split as EQ — kill logic on LSB path only.
// MSB applies directly without kill check for UI responsiveness.
PioneerDDJSX2.Filter = function(_channel, control, value, _status, _group) {
    const storage = PioneerDDJSX2.midi14bit.filter;

    if (control >= PioneerDDJSX2.FilterMSBBaseCC && control <= PioneerDDJSX2.FilterMSBMaxCC) {
        const offset = control - PioneerDDJSX2.FilterMSBBaseCC;
        storage[offset] = value;
        engine.setParameter(`[QuickEffectRack1_[Channel${ offset + 1 }]]`, "super1", ((value - PioneerDDJSX2.EQMSBMid) / PioneerDDJSX2.EQMSBRange + 1) / 2);
    } else if (control >= PioneerDDJSX2.FilterLSBBaseCC && control <= PioneerDDJSX2.FilterLSBMaxCC) {
        const offset = control - PioneerDDJSX2.FilterLSBBaseCC;
        const value14bit = (storage[offset] << 7) | value;
        const paramValue = ((value14bit - PioneerDDJSX2.EQ14BitMid) / PioneerDDJSX2.EQ14BitRange + 1) / 2;
        PioneerDDJSX2.handleFilterKill(offset, paramValue);
        engine.setParameter(`[QuickEffectRack1_[Channel${ offset + 1 }]]`, "super1", paramValue);
    }
};

// Trim/Pregain Knobs (14-bit)
// Includes special curve mapping because SX2 hardware trim knobs are logarithmic/weird.
PioneerDDJSX2.Pregain = function(channel, control, value) {
    var group = `[Channel${  channel + 1  }]`;
    var cache = PioneerDDJSX2.midi14bit.pregain;
    var value14bit;

    // 1. DUAL-UPDATE LOGIC (Linear and single-curved approaches didn"t work with these knobs)
    if (control === 0x04) {
        cache[channel] = value;
        value14bit = value << 7;
    } else if (control === 0x24) {
        value14bit = (cache[channel] << 7) | value;
    } else {
        return;
    }

    // 2. ASYMMETRIC MAPPING
    // Center: 8192 (Standard MIDI center)
    // Max: 12550 (Calibrated to hardware log)
    var centerVal = 8192.0;
    var maxVal = 12550.0;
    var gainValue;

    if (value14bit <= centerVal) {
        // ZONE A: 0 to Center (12 o"clock)
        // Map MIDI (0 -> 8192) to Gain (0.0 -> 1.0)
        var norm = value14bit / centerVal;

        // x^3 suppresses the value in the early stages.
        // 9 o"clock physical read significantly lower in software.
        gainValue = norm * norm * norm;

    } else {
        // ZONE B: Center to Max
        // Map MIDI (8192 -> 12550) to Gain (1.0 -> 4.0)

        // Normalize this upper section to 0.0 - 1.0
        let norm = (value14bit - centerVal) / (maxVal - centerVal);

        // Clamp to 1.0 in case of hardware jitter at max
        if (norm > 1.0) {
            norm = 1.0;
        }

        // Scale linearly to the top range (Unity to +12dB)
        // 1.0 (Start) + (norm * 3.0 range)
        gainValue = 1.0 + (norm * 3.0);
    }

    engine.setValue(group, "pregain", gainValue);
};

// =============================================================================
// 8b. KNOB KILL FUNCTIONS (Shift+Knob Down)
// =============================================================================
// Shift + turn a knob to mute it.
//
// Unmute by:
//   a) Shift + turn back to the muted spot
//   b) Move knob to center (EQ/Filter only)
//   c) EQ only: turn fully counter-clockwise
//
// Uses Mixxx's EQ kill buttons and Filter effect.

PioneerDDJSX2.EQParamCenter = 0.5;

// Clears gesture tracking state between shift sessions.
PioneerDDJSX2.clearKillGesture = function(state) {
    state.anchor = null;
    state.lastToggleDir = null;
};

// -- Kill Toggle Gate --
// Prevents accidental muting. After pressing Shift, you must turn the knob past
// a small threshold before muting triggers. Once muted, you can keep turning
// the same direction without re-triggering. Turn back past the threshold to unmute.
// This makes "mute → keep turning → reverse to unmute" work smoothly.
PioneerDDJSX2.killShouldToggle = function(state, newParamValue) {
    if (state.anchor === null) {
        state.anchor = newParamValue;
        return false;
    }

    const delta = newParamValue - state.anchor;

    if (Math.abs(delta) < PioneerDDJSX2.KillThreshold) {
        return false;
    }

    const dir = (delta > 0) ? "up" : "down";

    if (state.lastToggleDir !== null && dir === state.lastToggleDir) {
        // Still moving the same way as the last toggle — slide anchor forward.
        state.anchor = newParamValue;
        return false;
    }

    // Knob has reversed direction (or this is the first toggle): fire.
    state.lastToggleDir = dir;
    state.anchor = newParamValue;
    return true;
};

// EQ Kill Handler
// Intercepts EQ parameter changes when shift is held and applies kill logic.
// Returns true if the caller should NOT apply the normal EQ value (kill is
// active and the value should be suppressed), false if normal apply should proceed.
//
// Shift held + knob moved KillThreshold in any direction >> toggles kill on/off.
// Reversing direction by KillThreshold while shift is still held >> toggles again.
// Knob returned to center (0.5) or minimum (0.0) without shift >> deactivates kill.
//
// When kill is active, the EQ kill button in the Mixxx UI is pressed, which
// silences that frequency band. On restore, the button is released and the
// current knob position is applied immediately.
PioneerDDJSX2.handleEQKill = function(channel, knobType, paramName, newParamValue) {
    const state = PioneerDDJSX2.killState[channel][knobType];
    const isShifted = PioneerDDJSX2.anyShiftHeld();
    const group = `[EqualizerRack1_[Channel${ channel + 1 }]_Effect1]`;

    if (isShifted) {
        if (PioneerDDJSX2.killShouldToggle(state, newParamValue)) {
            state.killed = !state.killed;
            engine.setValue(group, `button_${ paramName }`, state.killed ? 1 : 0);
            if (!state.killed) {
                engine.setParameter(group, paramName, newParamValue);
            }
        }
        return state.killed;
    }

    // Shift not held: clear gesture memory and check position-based restore.
    PioneerDDJSX2.clearKillGesture(state);

    if (state.killed) {
        const atCenter = Math.abs(newParamValue - PioneerDDJSX2.EQParamCenter) < PioneerDDJSX2.KillTolerance;
        const atMinimum = newParamValue < PioneerDDJSX2.KillTolerance;
        if (atCenter || atMinimum) {
            state.killed = false;
            engine.setValue(group, `button_${ paramName }`, 0);
            engine.setParameter(group, paramName, newParamValue);
            return true;
        }
        // Kill is still active and knob is not at a restore position:
        // suppress the value to keep the band silenced until the user
        // explicitly restores it via center, minimum, or shift gesture.
        return true;
    }

    return false;
};

// Filter Kill Handler
// Same gesture logic as handleEQKill via killShouldToggle.
// Unlike EQ, the filter knob value always passes through to the engine
// regardless of kill state. Mixxx silences the filter by disabling the
// QuickEffectRack rather than by blocking the knob value.
//
// Shift held + knob moved KillThreshold >> toggles filter enabled/disabled.
// Knob returned to center (0.5) without shift >> re-enables filter.
PioneerDDJSX2.handleFilterKill = function(channel, newParamValue) {
    const state = PioneerDDJSX2.killState[channel].filter;
    const isShifted = PioneerDDJSX2.anyShiftHeld();
    const group = `[QuickEffectRack1_[Channel${ channel + 1 }]]`;

    if (isShifted) {
        if (PioneerDDJSX2.killShouldToggle(state, newParamValue)) {
            state.killed = !state.killed;
            engine.setValue(group, "enabled", state.killed ? 0 : 1);
        }
        return false;
    }

    PioneerDDJSX2.clearKillGesture(state);

    if (state.killed) {
        const atCenter = Math.abs(newParamValue - PioneerDDJSX2.EQParamCenter) < PioneerDDJSX2.KillTolerance;
        if (atCenter) {
            state.killed = false;
            engine.setValue(group, "enabled", 1);
        }
    }

    return false;
};

// =============================================================================
// 9. FX SECTION
// =============================================================================

// Handle Beats Knob (Turning) - Adjusts Wet/Dry Mix
PioneerDDJSX2.EffectJog = function(value, group, control) {
    if (control > 63) {
        engine.setValue(`[EffectRack1_EffectUnit${  value - 3  }]`, "mix", engine.getValue(`[EffectRack1_EffectUnit${  value - 3  }]`, "mix") - 0.0625 * (128 - control));
    } else {
        engine.setValue(`[EffectRack1_EffectUnit${  value - 3  }]`, "mix", engine.getValue(`[EffectRack1_EffectUnit${  value - 3  }]`, "mix") + 0.0625 * (control));
    }
};

// Beats Button Press (Cycles selected effect focus)
PioneerDDJSX2.BeatsPressFX = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.currentEffect[channel - 4]++;
        if (PioneerDDJSX2.currentEffect[channel - 4] > 3) {
            PioneerDDJSX2.currentEffect[channel - 4] = 0;
        }
        PioneerDDJSX2.FXLeds();
    }
};

// Shift + Beats Button Press (Cycles parameters)
PioneerDDJSX2.ShiftBeatsPressFX = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        // ID Effect is the ID of the currently focus effect control
        //  (3 = main effects knobs)
        //  (0 = first effect settings)
        //  (1 = second effect settings)
        //  (2 = third effect settings)
        var id_effect = ((channel === 5) ? (4) : (0)) + PioneerDDJSX2.currentEffect[channel - 4];
        PioneerDDJSX2.currentEffectparamset[id_effect]++;
        if (PioneerDDJSX2.currentEffectparamset[id_effect] >= (engine.getValue(`[EffectRack1_EffectUnit${  channel - 3  }_Effect${  PioneerDDJSX2.currentEffect[channel - 4] + 1  }]`, "num_parameters") / 3)) {
            PioneerDDJSX2.currentEffectparamset[id_effect] = 0;
        }
        PioneerDDJSX2.FXLeds();
    }
};

// Select specific effect in slot
PioneerDDJSX2.EffectSelect = function(value, group, control) {
    engine.setValue(`[EffectRack1_EffectUnit${  value - 3  }_Effect${  group - 98  }]`, "effect_selector", (control === 0x7F) ? 1 : 0);
};

// Control FX Parameters via knobs
PioneerDDJSX2.EffectKnob = function(value, group, control) {
    if (PioneerDDJSX2.currentEffect[value - 4] === 3) {
        switch (group) {
        case 2:
            engine.setValue(`[EffectRack1_EffectUnit${  value - 3  }_Effect1]`, "meta", control / 0x7F);
            break;
        case 4:
            engine.setValue(`[EffectRack1_EffectUnit${  value - 3  }_Effect2]`, "meta", control / 0x7F);
            break;
        case 6:
            engine.setValue(`[EffectRack1_EffectUnit${  value - 3  }_Effect3]`, "meta", control / 0x7F);
            break;
        }
    } else {
        var effectNumer = PioneerDDJSX2.currentEffect[value-4] + 1;
        var offsetNumer = (PioneerDDJSX2.currentEffectparamset[((value-4)*4)+PioneerDDJSX2.currentEffect[value-4]]*3);
        switch (group) {
        case 2:
            engine.setParameter(`[EffectRack1_EffectUnit${  value - 3  }_Effect${  effectNumer  }]`, `parameter${  1 + offsetNumer}`, control/0x7F);
            break;
        case 4:
            engine.setParameter(`[EffectRack1_EffectUnit${  value - 3  }_Effect${  effectNumer  }]`, `parameter${  2 + offsetNumer}`, control/0x7F);
            break;
        case 6:
            engine.setParameter(`[EffectRack1_EffectUnit${  value - 3  }_Effect${  effectNumer  }]`, `parameter${  3 + offsetNumer}`, control/0x7F);
            break;
        }

    }
};

// Enable/Disable specific effect
PioneerDDJSX2.EffectButton = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (PioneerDDJSX2.currentEffect[channel - 4] === 3) {
            engine.setValue(`[EffectRack1_EffectUnit${  channel - 3  }_Effect${  control - 70  }]`, "enabled", !engine.getValue(`[EffectRack1_EffectUnit${  channel - 3  }_Effect${  control - 70  }]`, "enabled"));
        }
    }
};

// Tap Button (Mix Mode Toggle)
PioneerDDJSX2.EffectTap = function(channel, control, value, status, group) {
    if (value === 0x7F && PioneerDDJSX2.currentEffect[channel - 4] === 3) {
        engine.setValue(`[EffectRack1_EffectUnit${  channel - 3  }]`, "mix_mode", !engine.getValue(`[EffectRack1_EffectUnit${  channel - 3  }]`, "mix_mode"));
        PioneerDDJSX2.FXLeds();
    }
};

// =============================================================================
// 10. VIEW, GRID, & PANELS
// =============================================================================

// Cycle through Mixxx Panel layouts (FX Rack, Samplers)
PioneerDDJSX2.PanelSelect = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.curPanel += ((1 - (control - 120)) * 2) - 1;
        if (PioneerDDJSX2.curPanel < 0) {
            PioneerDDJSX2.curPanel = 3;
        }
        if (PioneerDDJSX2.curPanel > 3) {
            PioneerDDJSX2.curPanel = 0;
        }
        engine.setValue("[Skin]", "show_samplers", (PioneerDDJSX2.curPanel & 1));
        engine.setValue("[Skin]", "show_effectrack", (PioneerDDJSX2.curPanel & 2) >> 1);
    }
};

// Cycle through views (2 deck / 4 deck / maximized library)
PioneerDDJSX2.ViewButton = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.curView++;
        if (PioneerDDJSX2.curView > 7) {
            PioneerDDJSX2.curPanel = 0;
        }

        engine.setValue("[Skin]", "show_4decks", PioneerDDJSX2.curView & 1);
    }
};

// Enable Grid Slide Mode (used with Jog Wheel)
PioneerDDJSX2.SetGridSlide = function(channel, control, value, status, group) {
    PioneerDDJSX2.gridSlide[channel] = value ? 1 : 0;
    midi.sendShortMsg(0x90 + channel, 0x0a, value ? 0x7F : 0x00);
};

// Enable Grid Adjust Mode (used with Jog Wheel)
PioneerDDJSX2.SetGridAdjust = function(channel, control, value, status, group) {
    PioneerDDJSX2.gridAdjust[channel] = value ? 1 : 0;
    midi.sendShortMsg(0x90 + channel, 0x79, value ? 0x7F : 0x00);
};

// Clear Grid Adjustment
PioneerDDJSX2.ClearGrid = function(channel, control, value, status, group) {
    engine.setValue(`[Channel${  channel + 1  }]`, "beats_undo_adjustment", 1);
    midi.sendShortMsg(0x90 + channel, 0x79, value ? 0x7F : 0x00);
};

// =============================================================================
// 11. SAMPLER & PAD PARAMETERS
// =============================================================================

// Plays sampler or Loads selected track if empty
PioneerDDJSX2.SamplerPlay = function(channel, control, value, status, group) {
    var sampler = `[Sampler${  1 + (control & 7) + (PioneerDDJSX2.samplerBank[channel - 7] * 8)  }]`;
    if (value === 0x7F) {
        if (engine.getValue(sampler, "track_loaded")) {
            engine.setParameter(sampler, "start_play", value & 1);
        } else {
            engine.setParameter(sampler, "LoadSelectedTrack", value & 1);
        }
    }
};

// Stops sampler (Short press) or Ejects (Long press)
PioneerDDJSX2.SamplerStop = function(channel, control, value, status, group) {
    var sampler = `[Sampler${  1 + (control & 7) + (PioneerDDJSX2.samplerBank[channel - 7] * 8)  }]`;
    var isPlaying = engine.getValue(sampler, "play");
    var trackLoaded = engine.getValue(sampler, "track_loaded");

    if (value === 0x7F) {
        if (trackLoaded && !isPlaying) {
            engine.setParameter(sampler, "eject", 1);
        } else {
            engine.setParameter(sampler, "stop", 1);
        }
    }
};

// Updates Sampler LEDs based on state (Loaded/Playing)
PioneerDDJSX2.SamplerLight = function(value, group, control) {
    var sampler = PioneerDDJSX2.groups.samplerGroups[group];
    if (sampler === undefined) {
        return;
    }

    var loaded = engine.getValue(group, "track_loaded");
    var isPlaying = engine.getValue(group, "play");

    for (var i = 0; i < 4; i++) {
        if ((PioneerDDJSX2.samplerBank[i] << 3) === (sampler & (~7))) {
            var offset = (sampler & 7);
            var color = 0;

            if (!loaded) {
                color = 0;
            } else if (value && isPlaying) {
                // White when playing
                color = 0x40;
            } else {
                color = 0x7F;
            }

            if (PioneerDDJSX2.padMode[i] === 3) {
                midi.sendShortMsg(0x97 + i, 0x30 + offset, color);
                midi.sendShortMsg(0x97 + i, 0x38 + offset, color);
            } else if (PioneerDDJSX2.padMode[i] === 7) {
                midi.sendShortMsg(0x97 + i, 0x70 + offset, color);
                midi.sendShortMsg(0x97 + i, 0x78 + offset, color);
            }
        }
    }
};

// Set Sampler Gain (Velocity sensitive)
PioneerDDJSX2.SetSampleGain = function(group, control, value) {
    var where = (control & 7) + (PioneerDDJSX2.samplerBank[group - 7] * 8);
    PioneerDDJSX2.sampleVolume[where] = value / 0x7F;
    engine.setParameter(`[Sampler${  1 + where  }]`, "pregain", PioneerDDJSX2.samplerVolume * PioneerDDJSX2.sampleVolume[where]);
};

// Master Sampler Volume Knob
PioneerDDJSX2.SetSamplerVol = function(value, group, control) {
    PioneerDDJSX2.samplerVolume = control / 0x7F;
    var num = engine.getValue("[App]", "num_samplers");
    for (var i = 0; i < num; i++) {
        engine.setParameter(`[Sampler${  i + 1  }]`, "pregain", PioneerDDJSX2.samplerVolume * PioneerDDJSX2.sampleVolume[i]);
    }
};

// Refreshes all sampler LEDs (used when switching banks)
PioneerDDJSX2.RepaintSampler = function(channel) {
    for (var i = 0; i < 8; i++) {
        var ai = i + PioneerDDJSX2.samplerBank[channel] * 8;
        var prefix = `[Sampler${  ai + 1  }]`;
        var loaded = engine.getValue(prefix, "track_loaded");
        var playing = engine.getValue(prefix, "play");

        var color = 0;
        if (loaded) {
            color = playing ? 0x40 : 0x7F;
        }

        if (PioneerDDJSX2.padMode[channel] === 3) {
            midi.sendShortMsg(0x97 + channel, 0x30 + i, color);
            midi.sendShortMsg(0x97 + channel, 0x38 + i, color);
        } else if (PioneerDDJSX2.padMode[channel] === 7) {
            midi.sendShortMsg(0x97 + channel, 0x70 + i, color);
            midi.sendShortMsg(0x97 + channel, 0x78 + i, color);
        }
    }
};
// Cycle Tempo Ranges
PioneerDDJSX2.SetTempoRange = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.tempoRange[channel]++;
        if (PioneerDDJSX2.tempoRange[channel] > 3) {
            PioneerDDJSX2.tempoRange[channel] = 0;
        }
        engine.setValue(`[Channel${  channel + 1  }]`, "rateRange", PioneerDDJSX2.settings.tempoRanges[PioneerDDJSX2.tempoRange[channel]]);
    }
};

// Previous Sampler Bank
PioneerDDJSX2.SamplerParam1L = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.samplerBank[channel]--;
        if (PioneerDDJSX2.samplerBank[channel] < 0) {
            PioneerDDJSX2.samplerBank[channel] = 0;
        } else {
            PioneerDDJSX2.RepaintSampler(channel);
        }
    }
};

// Next Sampler Bank
PioneerDDJSX2.SamplerParam1R = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.samplerBank[channel]++;
        if (PioneerDDJSX2.samplerBank[channel] > 7) {
            PioneerDDJSX2.samplerBank[channel] = 7;
        } else {
            PioneerDDJSX2.RepaintSampler(channel);
        }
    }
    midi.sendShortMsg(0x90 + channel, 0x2F, 0x7F);
};

// Decrease Roll Size with Parameter 1
PioneerDDJSX2.RollParam1L = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.rollPrec[channel]--;
        if (PioneerDDJSX2.rollPrec[channel] < 0) {
            PioneerDDJSX2.rollPrec[channel] = 0;
        }
        midi.sendShortMsg(0x90 + channel, 0x1E, PioneerDDJSX2.settings.rollColors[PioneerDDJSX2.rollPrec[channel]]);
    }
};

// Increase Roll Size with Parameter 1
PioneerDDJSX2.RollParam1R = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.rollPrec[channel]++;
        if (PioneerDDJSX2.rollPrec[channel] > 4) {
            PioneerDDJSX2.rollPrec[channel] = 4;
        }
        midi.sendShortMsg(0x90 + channel, 0x1E, PioneerDDJSX2.settings.rollColors[PioneerDDJSX2.rollPrec[channel]]);
    }
};

// Decrease Slicer Precision with Parameter 1
PioneerDDJSX2.SlicerParam1L = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.beatjumpPrec[channel]--;
        if (PioneerDDJSX2.beatjumpPrec[channel] < 0) {
            PioneerDDJSX2.beatjumpPrec[channel] = 0;
        }
        midi.sendShortMsg(0x90 + channel, 0x20, PioneerDDJSX2.settings.beatjumpColors[PioneerDDJSX2.beatjumpPrec[channel]]);
    }
};

// Increase Slicer Precision with Parameter 1
PioneerDDJSX2.SlicerParam1R = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.beatjumpPrec[channel]++;
        if (PioneerDDJSX2.beatjumpPrec[channel] > 8) {
            PioneerDDJSX2.beatjumpPrec[channel] = 8;
        }
        midi.sendShortMsg(0x90 + channel, 0x20, PioneerDDJSX2.settings.beatjumpColors[PioneerDDJSX2.beatjumpPrec[channel]]);
    }
};

// Parameter 1 for HOT CUE (Right)
// Not Used
PioneerDDJSX2.CueParam1R = function(channel, control, value, status, group) {
};

// Parameter 1 for HOT CUE (Left)
// Not Used
PioneerDDJSX2.CueParam1L = function(channel, control, value, status, group) {
};

// Parameter 1 for HOT CUE LOOPS (Left)
// Not Used
PioneerDDJSX2.CueLoopParam1R = function(channel, control, value, status, group) {
};

// Parameter 1 for HOT CUE LOOPS (Left)
// Not Used
PioneerDDJSX2.CueLoopParam1L = function(channel, control, value, status, group) {
};

// =============================================================================
// 12. PAD MODES
// =============================================================================

// Enable HotCue Mode
PioneerDDJSX2.SetHotCueMode = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.padMode[channel] = 0;
        midi.sendShortMsg(0x90 + channel, 0x1B, 0x7F);
    }
};

// Enable Roll Mode
PioneerDDJSX2.SetRollMode = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.padMode[channel] = 1;
        midi.sendShortMsg(0x90 + channel, 0x1E, PioneerDDJSX2.settings.rollColors[PioneerDDJSX2.rollPrec[channel]]);
    }
};

// Enable Slicer Mode
PioneerDDJSX2.SetSlicerMode = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.padMode[channel] = 2;
        midi.sendShortMsg(0x90 + channel, 0x20, 0x7F);
    }
};

// Enable Sampler Mode
PioneerDDJSX2.SetSamplerMode = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.padMode[channel] = 3;
        PioneerDDJSX2.RepaintSampler(channel);
        midi.sendShortMsg(0x90 + channel, 0x22, 0x7F);
    }
};

// Enable Cue Loop Mode
PioneerDDJSX2.SetCueLoopMode = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.padMode[channel] = 4;
        midi.sendShortMsg(0x90 + channel, 0x69, 0x7F);
    }
};

// Enable Saved Loop Mode
PioneerDDJSX2.SetSavedLoopMode = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.padMode[channel] = 5;
        midi.sendShortMsg(0x90 + channel, 0x6b, 0x7F);
    }
};

// Enable Slicer Loop Mode
PioneerDDJSX2.SetSlicerLoopMode = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.padMode[channel] = 6;
        midi.sendShortMsg(0x90 + channel, 0x6d, 0x7F);
    }
};

// Enable Velocity Sampler Mode
PioneerDDJSX2.SetVelocitySamplerMode = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        PioneerDDJSX2.padMode[channel] = 7;
        PioneerDDJSX2.RepaintSampler(channel);
        midi.sendShortMsg(0x90 + channel, 0x6f, 0x7F);
    }
};

// Logic for Saved Loop Pads
PioneerDDJSX2.SavedLoop = function(channel, control, value, status, group) {
    var deck = channel - 6;

    var interval = PioneerDDJSX2.settings.loopIntervals[control - 0x50 + 4];
    var isAutoLoopEnabled = engine.getValue(group, `beatloop_${  interval  }_enabled`);

    if (value === 0x7F) {
        for (var i = 0; i < 8; i++) {
            midi.sendShortMsg(0x97 + deck - 1, 80 + i, 0x00);
        }

        if (isAutoLoopEnabled !== 1) {
            engine.setValue(group, `beatloop_${  interval  }_activate`, 1);
            midi.sendShortMsg(0x97 + deck - 1, control, 0x7F);
        } else {
            engine.setValue(group, `beatloop_${  interval  }_toggle`, 1);
            midi.sendShortMsg(0x97 + deck - 1, control, 0x00);
        }
    }
};

// Logic for Beat Jump Pads
// TODO(XXX) : BeatJump is mapped to Slicer, create a real Slicer or wait for Mixxx to implement one.
PioneerDDJSX2.BeatJump = function(channel, control, value, status, group) {
    var deck = channel - 7;

    const which = (control & 3) + PioneerDDJSX2.beatjumpPrec[deck];

    if (value === 0x7F) {
        var interval = PioneerDDJSX2.settings.loopIntervals[which];
        if (control & 4) {
            engine.setValue(group, `beatjump_${  interval  }_backward`, 1);
        } else {
            engine.setValue(group, `beatjump_${  interval  }_forward`, 1);
        }
    }
    midi.sendShortMsg(0x97 + deck, control, (value === 0x7F) ? (PioneerDDJSX2.settings.beatjumpColors[which]) : (0x00));
};

// Logic for Roll Pads
PioneerDDJSX2.RollPerformancePad = function(channel, control, value, status, group) {
    var deck = channel - 6;

    var interval = PioneerDDJSX2.settings.loopIntervals[control - 0x10 + PioneerDDJSX2.rollPrec[channel - 7]];

    if (value === 0x7F) {
        engine.setValue(group, `beatlooproll_${  interval  }_activate`, 1);
    } else {
        engine.setValue(group, `beatlooproll_${  interval  }_activate`, 0);
    }

    midi.sendShortMsg(0x97 + deck - 1, control, (value === 0x7F) ? (PioneerDDJSX2.settings.rollColors[PioneerDDJSX2.rollPrec[channel - 7]]) : (0x00));
};

// =============================================================================
// 13. LIBRARY & BROWSER
// =============================================================================

// Center point of the relative rotary encoder (value > this means clockwise)
//0x40;

// Sort button CC numbers and their corresponding Mixxx library column indices
PioneerDDJSX2.SortButtons = {
    88: {column: 15,}, // Artist
    89: {column: 2,}, // Title
    96: {column: 9,}, // BPM
    97: {column: 1,}, // Tracknumber
};

// Rotary Selector Navigation - Works across ALL library widgets.
// Uses [Library],MoveVertical which handles sidebar, tracks, searchbox, and dialogs.
// If any channel's LOAD is held, cycle that channel's QuickEffect preset
// instead of scrolling the library. Works on all 4 channels independently.
PioneerDDJSX2.RotarySelector = function(_channel, _control, value, _status, _group) {
    let delta = 0x40 - Math.abs(0x40 - value);
    if (value > 0x40) {
        delta *= -1;
    }

    // Check if any channel's LOAD button is currently held
    let loadChannel = -1;
    for (let i = 0; i < 4; i++) {
        if (PioneerDDJSX2.loadHeld[i]) {
            loadChannel = i;
            break;
        }
    }

    if (loadChannel !== -1) {
        // LOAD is held — mark rotary as used (suppresses track load on release)
        // and cycle the QuickEffect preset for that channel
        PioneerDDJSX2.loadRotaryUsed[loadChannel] = true;

        const filterGroup = `[QuickEffectRack1_[Channel${ loadChannel + 1 }]]`;
        if (delta > 0) {
            engine.setValue(filterGroup, "next_chain_preset", 1);
        } else if (delta < 0) {
            engine.setValue(filterGroup, "prev_chain_preset", 1);
        }
        // Suppress library scroll — do NOT fall through to MoveVertical
        return;
    }

    // Normal library scroll
    engine.setValue("[Library]", "MoveVertical", delta);
};

// Sort Library via Shift + Load Buttons
PioneerDDJSX2.SortLibrary = function(_channel, control, value, _status, _group) {
    if (value === 0x7F) {
        const sortButton = PioneerDDJSX2.SortButtons[control];
        if (sortButton) {
            engine.setValue("[Library]", "sort_column_toggle", sortButton.column);
        }
    }
};

// Function when loading tracks, with a setting to enable PFL for the current track and disable all the other.
// Tracks LOAD hold state for LOAD+Rotary QuickEffect cycling.
// If the rotary was turned while LOAD was held, the track load is suppressed on release.

// Base MIDI control number for the Load buttons (decks 1-4)
PioneerDDJSX2.LoadSelectedTrack = function(_channel, control, value, _status, group) {
    const channel = script.deckFromGroup(group) - 1;
    if (value === 0x7F) {
        // Press — arm hold state only, do NOT load yet
        PioneerDDJSX2.loadHeld[channel] = true;
        PioneerDDJSX2.loadRotaryUsed[channel] = false;
    } else {
        // Release — clear hold state
        PioneerDDJSX2.loadHeld[channel] = false;
        // Only load if the rotary was not used during the hold
        if (!PioneerDDJSX2.loadRotaryUsed[channel]) {
            engine.setValue(group, "LoadSelectedTrack", 1);
            if (PioneerDDJSX2.settings.LoadBehaviour && !engine.getValue(group, "play")) {
                for (let i = 1; i <= 4; i++) {
                    engine.setValue(`[Channel${ i }]`, "pfl", 0);
                }
                engine.setValue(group, "pfl", 1);
                engine.setValue(group, "cue_goto", 1);
            }
        }
    }
};

// Back Button - returns focus to Library sidebar
// Uses MoveFocus to move back through: tracks -> searchbox -> sidebar
PioneerDDJSX2.BackButton = function(channel, control, value, status) {
    if (value === 0x7F) {
        engine.setValue("[Library]", "MoveFocus", -1);
    }
};

// Rotary Click - Context-aware action based on focused widget
// - In sidebar (focused_widget=0): Uses ToggleSelectedSidebarItem to expand/collapse or jump to tracks
// - In tracks/searchbox/dialogs: Uses GoToItem to load track or confirm action
PioneerDDJSX2.RotarySelectorClick = function(channel, control, value, status) {
    if (value === 0x7F) {
        var focusedWidget = engine.getValue("[Library]", "focused_widget");

        if (focusedWidget === 0) {
            // In sidebar - toggle expansion or jump to tracks
            engine.setValue("[Playlist]", "ToggleSelectedSidebarItem", 1);
        } else {
            // In tracks table, searchbox, or dialogs - perform action
            engine.setValue("[Library]", "GoToItem", 1);
        }
    }
};

// Shift + Rotary Click - Loads selected track to preview deck
// Always uses GoToItem regardless of focused widget
PioneerDDJSX2.rotarySelectorShiftedClick = function(channel, control, value) {
    if (value === 0x7F) {
        engine.setValue("[Library]", "GoToItem", 1);
    }
};

// =============================================================================
// 13. FLIP SECTION
// =============================================================================
// This is the Flip section of each deck, Mixxx doesn"t have Flip feature,
// so we can map them to something else

// Flip Slot Button, Sync key with the playing track on the adjacent channel
// Reads the key of the master (playing) channel and
// uses sync_key to match it.
// ALTERNATIVE SYNC: Opposed physical deck partner key sync.
// Mixxx's sync_key always syncs to the first numerically ordered deck that is playing with a detected beatgrid,
// which can be the wrong target when more than 2 decks are active simultaneously.
// Instead, this finds whichever channel is currently playing on the opposite physical side (A: ch1/3, B: ch2/4)
// and manually matches the key offset to that specific deck, replicating and bypassing sync_key entirely.
PioneerDDJSX2.FlipSlot = function(channel, control, value, status, group) {
    if (value === 0x7F && engine.getValue(group, "track_loaded")) {
        if (PioneerDDJSX2.settings.KeySyncBehaviour === true) {
            const deckNum = script.deckFromGroup(group);

            // Use the currently active deck on the opposite physical side
            const partnerDeck = (deckNum === 1 || deckNum === 3) ? PioneerDDJSX2.activeDeck.B : PioneerDDJSX2.activeDeck.A;

            const partnerGroup = `[Channel${  partnerDeck  }]`;

            if (!engine.getValue(partnerGroup, "track_loaded") ||
                engine.getValue(partnerGroup, "key") === 0) {
                return;
            }

            const myFileKey   = engine.getValue(group, "file_key");
            const partnerKey  = engine.getValue(partnerGroup, "key");
            const partnerDist = engine.getValue(partnerGroup, "visual_key_distance");

            const myMajor      = myFileKey >= 1 && myFileKey <= 12;
            const partnerMajor = partnerKey >= 1 && partnerKey <= 12;

            let targetKey = partnerKey;
            if (myMajor !== partnerMajor) {
                targetKey = (partnerKey > 12) ? partnerKey - 12 : partnerKey + 12;
            }

            const myTonic     = (myFileKey - 1) % 12;
            const targetTonic = (targetKey - 1) % 12;
            let steps         = targetTonic - myTonic;

            if (steps > 6) {
                steps -= 12;
            } else if (steps < -6) {
                steps += 12;
            }

            if (steps < -2) {
                steps = 5 + steps;
            } else if (steps > 2) {
                steps = -5 + steps;
            }

            const pitchToTakeOctaves = (steps + partnerDist) / 12.0;
            engine.setValue(group, "pitch", pitchToTakeOctaves * 12);

        } else {
            engine.setValue(group, "sync_key", 1);
        }
    }
};

// Flip Save (SHIFT + Slot) Button, Reset key to original
PioneerDDJSX2.FlipSave = function(channel, control, value, status, group) {
    if (engine.getValue(group, "track_loaded")) {
        if (value === 0x7F) {
            engine.setValue(group, "pitch_adjust_set_zero", 1);
        }
    }
};

// Flip Rec Button, Set the pitch of the track down
PioneerDDJSX2.FlipRec = function(channel, control, value, status, group) {
    if (engine.getValue(group, "track_loaded")) {
        if (value === 0x7F) {
            engine.setValue(group, "pitch_down", 1);
        }
    }
};

// Flip Save (SHIFT + Rec) Button, repeat setting for the current track
PioneerDDJSX2.FlipLoop = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        script.toggleControl(group, "repeat");
        var isRepeat = engine.getValue(group, "repeat");
    }
};

// Flip Start Button, Set the pitch of the track up
PioneerDDJSX2.FlipStart = function(channel, control, value, status, group) {
    if (engine.getValue(group, "track_loaded")) {
        if (value === 0x7F) {
            engine.setValue(group, "pitch_up", 1);
        }
    }
};

// Flip On/Off Button
PioneerDDJSX2.FlipOnOff = function(channel, control, value, status, group) {
};

/* eslint-disable no-var */
/* eslint-disable no-undef */
// Pioneer DDJ-SX2 mapping for Mixxx - WIP
// Based on hrudham's mapping for the DDJ-SR
// Modifications by tildearrow, Krafting, eXWoLL

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

// =============================================================================
// 1. CONSTANTS, DEBUGGING & HELPERS
// =============================================================================

// SysEx constants for Controller Initialization & Keep Alive.
// The SX2 requires a "Keep Alive" signal periodically to prevent it from 
// falling back to a standby/demo state or locking the jog wheels.
PioneerDDJSX2.keepAlive = [0xF0, 0x00, 0x20, 0x7f, 0x50, 0x01, 0xF7];
PioneerDDJSX2.recallState = [0xF0, 0x00, 0x20, 0x7f, 0x03, 0x01, 0xF7];

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
PioneerDDJSX2.shift = [0, 0, 0, 0]; // Shift button state per deck
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

// Effects State
PioneerDDJSX2.currentEffect = [3, 3]; // Selected FX unit
PioneerDDJSX2.currentEffectparamset = [0, 0, 0, 0, 0, 0, 0, 0];
PioneerDDJSX2.linkTypeTimer = 0;
PioneerDDJSX2.linkType = [
    [],
    []
];

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
    loopIntervals: ['0.03125', '0.0625', '0.125', '0.25', '0.5', '1', '2', '4', '8', '16', '32', '64'],
    tempoRanges: [0.08, 0.16, 0.32, 0.64],
    rollColors: [0x19, 0x20, 0x13, 0x0e, 0x05],
    beatjumpColors: [0x3c, 0x3a, 0x38, 0x36, 0x34, 0x32, 0x30, 0x2e, 0x2c, 0x2a, 0x28],
    cueLoopColors: [0x30, 0x35, 0x3a, 0x01, 0x05, 0x0a, 0x10, 0x15, 0x1a, 0x24, 0x27, 0x2a],
    safeScratchTimeout: engine.getSetting('safeScratchTimeout'),
    CenterRedLightsBehavior: engine.getSetting('CenterRedLightsBehavior'),
    CenterWhiteLightsBehavior: engine.getSetting('CenterWhiteLightsBehavior'), // 0 for rotations, 1 for duration
    NeedleSearchBehaviour: engine.getSetting("NeedleSearchBehaviour"),
    DoNotTrickController: engine.getSetting('DoNotTrickController'),
    SoftStartTime: engine.getSetting('SoftStartTime'),
    BrakeTime: engine.getSetting('BrakeTime'),
    UseShiftToBreak: engine.getSetting('UseShiftToBreak')
};

// Enums for efficient lookup
PioneerDDJSX2.enums = {
    rotarySelector: {
        targets: {
            libraries: 0,
            tracklist: 1
        }
    },
    channelGroups: {
        '[Channel1]': 0x00,
        '[Channel2]': 0x01,
        '[Channel3]': 0x02,
        '[Channel4]': 0x03
    },
    samplerGroups: {},
    hotcueIndex: {}
};

// Populate Enums dynamically
for (var i = 0; i < 64; i++) {
    PioneerDDJSX2.enums.samplerGroups['[Sampler' + (i + 1) + ']'] = i;
}
for (var i = 1; i <= 24; i++) {
    PioneerDDJSX2.enums.hotcueIndex['hotcue_' + i + '_status'] = i;
}

PioneerDDJSX2.status = {rotarySelector: {target: PioneerDDJSX2.enums.rotarySelector.targets.tracklist}};

// =============================================================================
// 3. INITIALIZATION & SHUTDOWN
// =============================================================================

// Called when the controller is detected or the script is reloaded.
PioneerDDJSX2.init = function(id) {
    // Send Serato Recall Sysex (Forces controller into correct mode)
    midi.sendSysexMsg(PioneerDDJSX2.recallState, PioneerDDJSX2.recallState.length);

    PioneerDDJSX2.channels = {
        0x00: {},
        0x01: {},
        0x02: {},
        0x03: {}
    };

    // Startup Animation (Sends lights to the Master Level Meter)
    midi.sendShortMsg(0x90, 0x0b, 0x10);
    midi.sendShortMsg(0x91, 0x0b, 0x10);
    midi.sendShortMsg(0x92, 0x0b, 0x10);
    midi.sendShortMsg(0x93, 0x0b, 0x10);
    midi.sendShortMsg(0x90, 0x1b, 0x7f);
    midi.sendShortMsg(0x91, 0x1b, 0x7f);
    midi.sendShortMsg(0x92, 0x1b, 0x7f);
    midi.sendShortMsg(0x93, 0x1b, 0x7f);

    // Bind Mixxx Controls to JS functions using engine.makeConnection
    PioneerDDJSX2.BindControlConnections();

    // Reset Hardware state (Quick Effects default to Filter)
    engine.setValue("[QuickEffectRack1_[Channel1]_Effect1]", "parameter2", 4);
    engine.setValue("[QuickEffectRack1_[Channel2]_Effect1]", "parameter2", 4);
    engine.setValue("[QuickEffectRack1_[Channel3]_Effect1]", "parameter2", 4);
    engine.setValue("[QuickEffectRack1_[Channel4]_Effect1]", "parameter2", 4);

    // Turn off Deck Lights initially to ensure clean state
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0xbb, i, 0);
    }

    // Initialize Deck defaults (Vinyl mode on, Tempo range reset)
    for (var i = 0; i < 4; i++) {
        midi.sendShortMsg(0x90 + i, 0x0d, 0x7f);
        engine.setValue("[Channel" + (i + 1) + "]", "rateRange", PioneerDDJSX2.settings.tempoRanges[PioneerDDJSX2.tempoRange[i]]);
        PioneerDDJSX2.RepaintSampler(i);
    }

    PioneerDDJSX2.FXLeds();

    // Restore Hotcues LED state from engine
    for (var i = 1; i <= 4; i++) {
        for (var j = 1; j <= 16; j++) {
            PioneerDDJSX2.HotCuePerformancePadLed(engine.getValue("[Channel" + i + "]", "hotcue_" + j + "_status"), "[Channel" + i + "]", "hotcue_" + j + "_status");
        }
    }

    // Enable Effects Units for all channels by default
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 1);
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel3]_enable", 1);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 1);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel4]_enable", 1);

    // Start Blink/Keep-Alive Timer (250ms interval)
    PioneerDDJSX2.lightsTimer = engine.beginTimer(250, PioneerDDJSX2.doTimer, 0);

    // Initialize 14-bit cache structures if undefined
    if (typeof PioneerDDJSX2.midi14bit === 'undefined') {
        PioneerDDJSX2.midi14bit = {};
    }
    if (typeof PioneerDDJSX2.midi14bit.pregain === 'undefined') {
        PioneerDDJSX2.midi14bit.pregain = [0, 0, 0, 0]; // For 4 channels
    }

}

// Called when the controller is disconnected or script is stopped.
PioneerDDJSX2.shutdown = function() {
    PioneerDDJSX2.UnbindControlConnections();
    // Turn off lights
    for (var i = 0; i < 8; i++) {
        midi.sendShortMsg(0xbb, i, 0);
    };
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
        if (engine.getValue("[Channel" + (i + 1) + "]", "slip_enabled")) {
            midi.sendShortMsg(0x90 + i, 0x40, PioneerDDJSX2.blinkState ? 0x7F : 0x00); // Slip Button normal
        } else {
            midi.sendShortMsg(0x90 + i, 0x40, 0x00); // Slip Button normal
        }

        // Reverse Blink
        if (PioneerDDJSX2.reverse[i]) {
            midi.sendShortMsg(0x90 + i, 0x38, PioneerDDJSX2.blinkState ? 0x7f : 0x00); // Reverse Button when shifting
            midi.sendShortMsg(0x90 + i, 0x15, PioneerDDJSX2.blinkState ? 0x7f : 0x00); // Reverse Button normal
        } else {
            midi.sendShortMsg(0x90 + i, 0x38, 0x00); // Reverse Button when shifting
            midi.sendShortMsg(0x90 + i, 0x15, 0x00); // Reverse Button normal
        }
    }
    // Toggle blink state for next tick
    PioneerDDJSX2.blinkState = !PioneerDDJSX2.blinkState;
}

// =============================================================================
// 5. CONNECTIONS (Signals)
// =============================================================================

// Binds Mixxx Engine signals (e.g., 'play', 'vu_meter') to script functions.
// This allows the controller LEDs to react to changes in the software UI.
PioneerDDJSX2.BindControlConnections = function() {
    for (var i = 1; i <= 4; i++) {
        var group = '[Channel' + i + ']';

        // CRITICAL: JOGWHEEL & SLICER FEEDBACK
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'playposition', PioneerDDJSX2.deckLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'beat_next', PioneerDDJSX2.SlicerLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'beat_active', PioneerDDJSX2.UpdateCenterBeatLights));

        // NEW: Reset beat counter when playposition changes (needle drop, hotcue jumps)
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'playposition', PioneerDDJSX2.ResetBeatCounter));

        // STANDARD INDICATORS
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'vu_meter', PioneerDDJSX2.VuMeter));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'play_indicator', PioneerDDJSX2.PlayLeds));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'sync_enabled', PioneerDDJSX2.SyncLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'cue_indicator', PioneerDDJSX2.CueLeds));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'pfl', PioneerDDJSX2.HeadphoneCueLed));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'keylock', PioneerDDJSX2.KeyLockLeds));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'loop_double', PioneerDDJSX2.LoopDouble));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'loop_halve', PioneerDDJSX2.LoopHalve));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'rate', PioneerDDJSX2.RateLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'eject', PioneerDDJSX2.UnloadLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'loop_enabled', PioneerDDJSX2.ReloopExit));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'loop_in', PioneerDDJSX2.ReloopExit));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "loop_out", PioneerDDJSX2.ReloopExit));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'track_samples', PioneerDDJSX2.LoadActions));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, 'pitch_adjust', PioneerDDJSX2.PitchAdjust));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "quantize", PioneerDDJSX2.ParamLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "loop_anchor", PioneerDDJSX2.ParamLights));
        PioneerDDJSX2.conns.push(engine.makeConnection(group, "repeat", PioneerDDJSX2.ParamLights));

        // HOTCUES (Status and Color)
        for (var j = 1; j <= 16; j++) {
            PioneerDDJSX2.conns.push(engine.makeConnection(group, 'hotcue_' + j + '_status', PioneerDDJSX2.HotCuePerformancePadLed));
            PioneerDDJSX2.conns.push(engine.makeConnection(group, 'hotcue_' + j + '_color', PioneerDDJSX2.HotCuePerformancePadLed));
        }
    }

    // FX Connections (Enable/Disable LEDs)
    PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Channel1]_enable', PioneerDDJSX2.FX1CH1));
    PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit2]', 'group_[Channel1]_enable', PioneerDDJSX2.FX2CH1));
    PioneerDDJSX2.conns.push(engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", PioneerDDJSX2.FX1CH2));
    PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit2]', 'group_[Channel2]_enable', PioneerDDJSX2.FX2CH2));
    PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Channel3]_enable', PioneerDDJSX2.FX1CH3));
    PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit2]', 'group_[Channel3]_enable', PioneerDDJSX2.FX2CH3));
    PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Channel4]_enable', PioneerDDJSX2.FX1CH4));
    PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit2]', 'group_[Channel4]_enable', PioneerDDJSX2.FX2CH4));

    // Effect Leds
    for (var i = 1; i <= 3; i++) {
        PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit1_Effect' + i + ']', 'enabled', PioneerDDJSX2.FXLeds));
        PioneerDDJSX2.conns.push(engine.makeConnection('[EffectRack1_EffectUnit2_Effect' + i + ']', 'enabled', PioneerDDJSX2.FXLeds));
    }

    // Sampler Connections (Play/Load state)
    var numSamplers = engine.getValue('[App]', 'num_samplers');
    for (var i = 1; i <= numSamplers; i++) {
        PioneerDDJSX2.conns.push(engine.makeConnection('[Sampler' + i + ']', 'play', PioneerDDJSX2.SamplerLight));
        PioneerDDJSX2.conns.push(engine.makeConnection('[Sampler' + i + ']', 'track_loaded', PioneerDDJSX2.SamplerLight));
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

// 1. Handles WHITE outer ring ONLY
PioneerDDJSX2.deckLights = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];

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
            if (PioneerDDJSX2.settings.CenterWhiteLightsBehavior === 1) {
                midi.sendShortMsg(0xbb, channel, PioneerDDJSX2.blinkState ? trackProgression : 0x00);
            } else {
                midi.sendShortMsg(0xbb, channel, PioneerDDJSX2.blinkState ? finalPos : 0x00);
            }
        } else {
            // Normal playback - steady white ring rotation
            if (PioneerDDJSX2.settings.CenterWhiteLightsBehavior === 1) {
                midi.sendShortMsg(0xbb, channel, trackProgression);
            } else {
                midi.sendShortMsg(0xbb, channel, finalPos);
            }
        }
    }
};


// CENTER RED JOGWHEEL LIGHTS (4 LEDs - 2 MODES ONLY)
// =============================================================================
// 2. Handles BOTH beat modes for RED center LEDs
// MODE 0 (Checkbox OFF): Beat-by-beat - 1 LED per beat, 4 beat cycle
// MODE 1 (Checkbox ON):  Bar-by-bar - 1 LED per bar (4 beats), 4 bar cycle (16 beats)
PioneerDDJSX2.UpdateCenterBeatLights = function(value, group, control) {
    if (value === 0) {
        return
    };

    var rawSetting = engine.getSetting("CenterRedLightsBehavior");
    var barByBarMode = (rawSetting === null) ? true : Boolean(Number(rawSetting));

    var channel = PioneerDDJSX2.enums.channelGroups[group];

    // Increment beat counter
    PioneerDDJSX2.currentBeat[channel]++;

    if (barByBarMode) {
        // ===== MODE 1: BAR-BY-BAR (Checkbox ON) =====
        // 32 beat cycle (8 bars): 4 bars ON + 4 bars OFF

        if (PioneerDDJSX2.currentBeat[channel] > 32) {
            PioneerDDJSX2.currentBeat[channel] = 1;
        }

        var barNumber = Math.ceil(PioneerDDJSX2.currentBeat[channel] / 4);
        midi.sendShortMsg(0xbb, 0x04 + channel, barNumber);

    } else {
        // ===== MODE 0: BEAT-BY-BEAT (Checkbox OFF) =====
        // 8 beat cycle: 4 beats ON + 4 beats OFF

        if (PioneerDDJSX2.currentBeat[channel] > 8) {
            PioneerDDJSX2.currentBeat[channel] = 1;
        }

        midi.sendShortMsg(0xbb, 0x04 + channel, PioneerDDJSX2.currentBeat[channel]);
    }
};

// 3. Reset beat counter when track loads OR playposition changes
PioneerDDJSX2.LoadActions = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    if (value) {
        // Reset beat counter to 0 so first beat = LED 1
        PioneerDDJSX2.currentBeat[channel] = 0;

        // Turn off center red LEDs
        midi.sendShortMsg(0xbb, 0x04 + channel, 0);

        // Light up load button
        midi.sendShortMsg(0x9B, channel, 0x7F);
    }
};

// Update Lights for the Parameters buttons
// This is because we can set quantize in the CUE and CUE LOOP mode
// So that the leds are always in sync
// Plus, this allows changing the setting in mixxx and being reflected on the controller
// After some tests, it seems we CANNOT set the lights on Parameter 1 when we send the Serato Keepalive command
// The lights will just blink when we press the buttons
// Except for the CUE mode where we can set the lights as static.
PioneerDDJSX2.ParamLights = function() {
    // For each channel (deck)
    for (var i = 0; i < 4; i++) {
        var channel = `[Channel${  i+1  }]`;

        // For the Quantize of each channels
        // Rec Flip Button
        var isQuantize = engine.getValue(channel, "quantize");
        midi.sendShortMsg(0x90 + i, 0x4A, isQuantize ? 0x7F : 0x00);

        // For the loop anchor of each channels
        // Start Flip Button
        var isAnchor = engine.getValue(channel, "loop_anchor");
        midi.sendShortMsg(0x90 + i, 0x4B, isAnchor ? 0x7F : 0x00);

        // For the repeat setting of each channels
        // SHIFT + Rec Flip Button
        var isAnchor = engine.getValue(channel, "repeat");
        midi.sendShortMsg(0x90 + i, 0x5A, isAnchor ? 0x7F : 0x00);
    }
};

// 4. Track last playposition to detect manual jumps
PioneerDDJSX2.lastPlayPosition = [0, 0, 0, 0];

PioneerDDJSX2.ResetBeatCounter = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    var lastPos = PioneerDDJSX2.lastPlayPosition[channel];

    // Only reset if this is a JUMP (difference > 0.001 = ~0.1% of track)
    // Normal playback moves very smoothly, jumps are much larger
    var positionDiff = Math.abs(value - lastPos);

    if (positionDiff > 0.001) {
        // This is a manual jump (needle drop, hotcue, waveform click)
        PioneerDDJSX2.currentBeat[channel] = 0;
        midi.sendShortMsg(0xbb, 0x04 + channel, 0);
    }

    // Store current position for next comparison
    PioneerDDJSX2.lastPlayPosition[channel] = value;
};

// Slicer Lights
// Handles the chasing lights on the pads when Slicer mode is active.
PioneerDDJSX2.slicerTick = [0, 0, 0, 0];

PioneerDDJSX2.SlicerLights = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];

    // Exit immediately if not in Slicer mode
    if (PioneerDDJSX2.padMode[channel] !== 2 && PioneerDDJSX2.padMode[channel] !== 6) {
        return;
    }

    PioneerDDJSX2.slicerTick[channel] = (PioneerDDJSX2.slicerTick[channel] + 1) % 8;

    var slicePosition = PioneerDDJSX2.slicerTick[channel];

    // Couleurs
    var isLoop = (PioneerDDJSX2.padMode[channel] === 6);
    var offset = isLoop ? 0x60 : 0x20;
    var onColor = isLoop ? 0x28 : 0x01;
    var offColor = isLoop ? 0x01 : 0x28;

    // Mise à jour des 8 pads
    for (var i = 0; i < 8; i++) {
        var isActive = (slicePosition === i);
        midi.sendShortMsg(0x97 + channel, offset + i, isActive ? onColor : offColor);
    }
};


// Updates the Performance Pads for Hotcues and Hotcues Loops
PioneerDDJSX2.HotCuePerformancePadLed = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    var i = PioneerDDJSX2.enums.hotcueIndex[control];

    if (i === undefined || value === 2) {
        return;
    }

    var color = 0;
    if (value === 1) {
        // Map Mixxx Hotcue color to Pioneer MIDI Color
        color = PioneerDDJSX2.padColors.getValueForNearestColor(engine.getValue(group, 'hotcue_' + (i) + '_color'));
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
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x0B, value ? 0x7F : 0x00);
    midi.sendShortMsg(0x90 + channel, 0x47, value ? 0x7F : 0x00);
    if (PioneerDDJSX2.settings.DoNotTrickController) {
        midi.sendShortMsg(0x9B, 0x0c + channel, value ? 0x7F : 0x00);
    }
};

// Updates VU Meter LEDs
PioneerDDJSX2.VuMeter = function(value, group, control) {
    var level = value * 127;
    var channel = 0xB0 + PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(channel, 0x02, level);
};

// Updates Sync Button LED
PioneerDDJSX2.SyncLights = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x58, value ? 0x7f : 0x00);
};

// Clears all LEDs on a specific deck (used on Eject)
PioneerDDJSX2.UnloadLights = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];

    // Clear all pads LEDs
    for (var k = 0; k < 0x30; k++) {
        midi.sendShortMsg(0x97 + channel, k, 0x00);
    }
    for (var k = 0x40; k < 0x70; k++) {
        midi.sendShortMsg(0x97 + channel, k, 0x00);
    }

    midi.sendShortMsg(0xbb, channel, 0);
    midi.sendShortMsg(0xbb, 4 + channel, 0);
};

// Updates Headphone Cue (PFL) LED
PioneerDDJSX2.HeadphoneCueLed = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x54, value ? 0x7F : 0x00);
};

// Updates Main Cue Button LED
PioneerDDJSX2.CueLeds = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x48, value ? 0x7f : 0x00);
    midi.sendShortMsg(0x90 + channel, 0x0C, value ? 0x7f : 0x00);
};

// Updates Key Lock / Master Tempo LED
PioneerDDJSX2.KeyLockLeds = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x1A, value ? 0x7F : 0x00);
};

// Updates Loop 2X LED
PioneerDDJSX2.LoopDouble = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x13, value ? 0x7F : 0x00);
};

// Updates Loop 1/2 LED
PioneerDDJSX2.LoopHalve = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x12, value ? 0x7F : 0x00);
};

// Updates Loop In/Out/Exit LEDs
PioneerDDJSX2.ReloopExit = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x14, engine.getValue(group, "loop_enabled") ? 0x7F : 0x00);
    midi.sendShortMsg(0x90 + channel, 0x10, (engine.getValue(group, "loop_start_position") > -1) ? 0x7F : 0x00);
    midi.sendShortMsg(0x90 + channel, 0x11, (engine.getValue(group, "loop_end_position") > -1) ? 0x7F : 0x00);
};

// Updates Key Shift (Pitch Adjust) LEDs
PioneerDDJSX2.PitchAdjust = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
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
function setFXLed(note) {
    return function(value) {
        midi.sendShortMsg(0x96, note, value ? 0x7F : 0x00);
    };
}
PioneerDDJSX2.FX1CH1 = setFXLed(0x4C);
PioneerDDJSX2.FX2CH1 = setFXLed(0x50);
PioneerDDJSX2.FX1CH2 = setFXLed(0x4D);
PioneerDDJSX2.FX2CH2 = setFXLed(0x51);
PioneerDDJSX2.FX1CH3 = setFXLed(0x4E);
PioneerDDJSX2.FX2CH3 = setFXLed(0x52);
PioneerDDJSX2.FX1CH4 = setFXLed(0x4F);
PioneerDDJSX2.FX2CH4 = setFXLed(0x53);

// Updates Rate/Tempo Slider direction LEDs (Up/Down arrows)
PioneerDDJSX2.RateLights = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    if (engine.getValue(group, 'rate') === 0) {
        midi.sendShortMsg(0x90 + channel, 0x37, 0x00);
        midi.sendShortMsg(0x90 + channel, 0x34, 0x00);
        return;
    }
    if (engine.getValue(group, 'rate') < 0) {
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
            midi.sendShortMsg(0x94 + i, 0x47 + j, engine.getValue("[EffectRack1_EffectUnit" + (i + 1) + "_Effect" + (j + 1) + "]", "enabled") ? 0x7F : 0x00);
        }
        midi.sendShortMsg(0x94 + i, 0x4a, engine.getValue("[EffectRack1_EffectUnit" + (i + 1) + "]", "mix_mode") ? 0x7F : 0x00);
    }
};

// =============================================================================
// 7. JOGWHEEL & SCRATCHING
// =============================================================================

// Calculates the delta movement of the jogwheel
PioneerDDJSX2.getJogWheelDelta = function(value) {
    return value - 0x40;
}

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
    engine.setValue('[Channel' + deck + ']', 'jog', movement);
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
            engine.setValue("[Channel" + deck + "]", (value < 64) ? "beats_translate_earlier" : "beats_translate_later", 1);
        }
        if (PioneerDDJSX2.gridAdjust[channel]) {
            engine.setValue("[Channel" + deck + "]", (value < 64) ? "beats_adjust_faster" : "beats_adjust_slower", 1);
        }
    }
};

// Outer Jog Ring Touch (Seeking)
PioneerDDJSX2.jogSeekTouch = function(channel, control, value) {
    if (!engine.getValue('[Channel' + (channel + 1) + ']', 'play')) {
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
    engine.setValue("[Channel" + (channel + 1) + "]", "beatjump", PioneerDDJSX2.getJogWheelDelta(value) / 16);
};

PioneerDDJSX2.jogPitchBend = function(channel, control, value) {
    var deck = channel + 1;
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, PioneerDDJSX2.getJogWheelDelta(value));
        PioneerDDJSX2.postponeDisableScratch(channel);
    } else {
        if (engine.getValue('[Channel' + deck + ']', 'play')) {
            PioneerDDJSX2.pitchBend(channel, PioneerDDJSX2.getJogWheelDelta(value));
        }
    }
};

// Toggle Vinyl Mode (Shift + Slip)
PioneerDDJSX2.ToggleVinyl = function(group, control, value) {
    if (value === 127) {
        PioneerDDJSX2.vinylOn[group] = !PioneerDDJSX2.vinylOn[group];
    }
};

// =============================================================================
// 8. MIXER & TRANSPORT (14-bit Support)
// =============================================================================

// Play/Pause Button
// Handles Logic for SoftStart/Braking if Shift is pressed/configured
PioneerDDJSX2.Play = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var isPlaying = engine.getValue(group, 'play');
    var lastHotcue = engine.getValue(group, 'hotcue_focus');
    if (lastHotcue !== 0) {
        var isLastHotcuePlaying = (lastHotcue !== -1) ? engine.getValue(group, 'hotcue_' + lastHotcue + '_status') : 0;
    };

    if (value === 127) {
        if (((control === 71 && PioneerDDJSX2.settings.UseShiftToBreak === false) || (control !== 71 && PioneerDDJSX2.settings.UseShiftToBreak === true)) && isLastHotcuePlaying !== 2) {
            if (isPlaying && PioneerDDJSX2.isBraking === 0) {
                PioneerDDJSX2.isBraking = 1;
                if (PioneerDDJSX2.settings.BrakeTime >= 0) {
                    engine.brake(deck, true, PioneerDDJSX2.settings.BrakeTime);
                } else {
                    engine.setValue(group, 'play', 0);
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
                    engine.setValue(group, 'play', 1);
                }
            }
        } else {
            PioneerDDJSX2.isBraking = 0;
            engine.setValue(group, 'play', !isPlaying);
        }
    }
};

PioneerDDJSX2.NeedleSearch = function(channel, control, value, status, group) {
    // For isShifted, 3 = Not Shifting ; 28 = Shifting
    var isShifted = control;
    var isPlaying = engine.getValue(group, "play");

    // var actualPlayPosition = 1/value;
    var actualPlayPosition = value / 127;

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
// Long press (>1000ms) : enable old behaviour of syncing both left and right tracks
PioneerDDJSX2.SyncEnable = function(value, group, control) {
    if (control === 127) {
        PioneerDDJSX2.SyncEnableTimer = engine.beginTimer(1000, function() {
            if (value === 0 || value === 2) {
                engine.setValue("[Channel" + (value + 1) + "]", "sync_enabled", 1);
                engine.setValue("[Channel" + (value + 2) + "]", "sync_enabled", 1);
            } else {
                engine.setValue("[Channel" + (value + 1) + "]", "sync_enabled", 1);
                engine.setValue("[Channel" + (value) + "]", "sync_enabled", 1);
            }
            PioneerDDJSX2.SyncEnableTimer = null;
        }, 1);
    } else {
        if (PioneerDDJSX2.SyncEnableTimer !== null) {
            engine.setValue("[Channel" + (value + 1) + "]", "beatsync", 1);
            engine.stopTimer(PioneerDDJSX2.SyncEnableTimer);
        }
    }
};

// Sync Disable (Shift + Sync)
PioneerDDJSX2.SyncDisable = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    if (control === 127) {
        if (value === 0 || value === 2) {
            engine.setValue("[Channel" + (value + 1) + "]", "sync_enabled", 0);
            engine.setValue("[Channel" + (value + 2) + "]", "sync_enabled", 0);
        } else {
            engine.setValue("[Channel" + (value + 1) + "]", "sync_enabled", 0);
            engine.setValue("[Channel" + (value) + "]", "sync_enabled", 0);
        }
    }
};

// Slip Button
PioneerDDJSX2.SlipEnabled = function(value, group, control) {
    if (control === 127) {
        if (engine.getValue("[Channel" + (value + 1) + "]", "play")) {
            engine.setValue("[Channel" + (value + 1) + "]", "slip_enabled", !engine.getValue("[Channel" + (value + 1) + "]", "slip_enabled"));
        } else {
            engine.setValue("[Channel" + (value + 1) + "]", "slip_enabled", !engine.getValue("[Channel" + (value + 1) + "]", "slip_enabled"));
        }
    }
};

// Crossfader Curve Adjust (Front panel knob)
PioneerDDJSX2.CrossfaderCurve = function(value, group, control) {
    engine.setValue("[Mixer Profile]", "xFaderCurve", control / 16);
};

// Input Select Switches (Front panel)
PioneerDDJSX2.InputSelect = function(group, control, value, status) {
    engine.setValue("[Channel" + (group + 1) + "]", "mute", value ? 1 : 0);
}

// Loop In Button
PioneerDDJSX2.LoopIn = function(group, control, value, status) {
    engine.setValue("[Channel" + (group + 1) + "]", "loop_in", value ? 1 : 0);
    if (value === 0x7f) {
        PioneerDDJSX2.closestBeatToLoopIn[group] = engine.getValue("[Channel" + (group + 1) + "]", "beat_closest");
    }
}

// 4 Beat Loop Button
PioneerDDJSX2.FourBeat = function(group, control, value, status) {
    var channel = "[Channel" + (group + 1) + "]";
    if (value === 0x7f) {
        var loop_enable = engine.getValue(channel, 'loop_enabled')
        engine.setValue(channel, "loop_start_position", PioneerDDJSX2.closestBeatToLoopIn[group]);
        engine.setValue(channel, "loop_end_position", PioneerDDJSX2.closestBeatToLoopIn[group] + engine.getValue(channel, 'track_samplerate') * (480 / engine.getValue(channel, 'file_bpm')));

        if (!loop_enable) {
            engine.setValue(channel, "reloop_toggle", 1);
        }
    }
}

// Slip Mode Logic
PioneerDDJSX2.SlipMode = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    if (engine.getValue(group, "play")) {
        midi.sendShortMsg(0x90 + channel, 0x40, value ? 0x7F : 0x00);
    }
};

PioneerDDJSX2.Shift = function(channel, control, value, status) {
    PioneerDDJSX2.shift[channel] = value;
};

// Reverse Button (Toggle)
PioneerDDJSX2.Reverse = function(value, group, control) {
    if (control === 127) {
        PioneerDDJSX2.reverse[value] = !PioneerDDJSX2.reverse[value];
        engine.setValue("[Channel" + (value + 1) + "]", "reverse", PioneerDDJSX2.reverse[value]);
    }
};

// Reverse Button (Hold - Censor)
PioneerDDJSX2.ReverseHold = function(value, group, control) {
    if (control === 127) {
        PioneerDDJSX2.reverse[value] = 1;
        engine.setValue("[Channel" + (value + 1) + "]", "reverse", PioneerDDJSX2.reverse[value]);
    } else {
        PioneerDDJSX2.reverse[value] = 0;
        engine.setValue("[Channel" + (value + 1) + "]", "reverse", PioneerDDJSX2.reverse[value]);
    }
};

// Auto Loop Button
PioneerDDJSX2.AutoLoop = function(channel, control, value, status) {
    if (value === 127) {
        if (engine.getValue("[Channel" + (channel + 1) + "]", "loop_enabled")) {
            engine.setValue("[Channel" + (channel + 1) + "]", "reloop_toggle", 1);
        } else {
            engine.setValue("[Channel" + (channel + 1) + "]", "beatloop_activate", 1);
        }
    }
};

// --- 14-BIT MIDI IMPLEMENTATION ---
// The SX2 sends high-resolution MIDI using two messages (MSB and LSB).
// We cache the MSB and wait for the LSB to apply the full value.

// Tempo/Rate Slider (14-bit)
PioneerDDJSX2.RateSlider = function(channel, control, value, status) {
    var deckGroup = "[Channel" + (channel + 1) + "]";
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
PioneerDDJSX2.VolumeSlider = function(channel, control, value, status) {
    var deckGroup = '[Channel' + (channel + 1) + ']';
    var storage = PioneerDDJSX2.midi14bit.volume;
    if (control === 0x13) {
        storage[channel] = value;
        engine.setParameter(deckGroup, 'volume', value / 127.0);
    } else if (control === 0x33) {
        var value14bit = (storage[channel] << 7) | value;
        engine.setParameter(deckGroup, 'volume', value14bit / 16383.0);
    }
};

// EQ Knobs (14-bit)
PioneerDDJSX2.EQHigh = function(channel, control, value) {
    PioneerDDJSX2.handle14bitEQ(channel, control, value, PioneerDDJSX2.midi14bit.eqHigh, 0x07, 0x27, 'parameter3');
};
PioneerDDJSX2.EQMid = function(channel, control, value) {
    PioneerDDJSX2.handle14bitEQ(channel, control, value, PioneerDDJSX2.midi14bit.eqMid, 0x0B, 0x2B, 'parameter2');
};
PioneerDDJSX2.EQLow = function(channel, control, value) {
    PioneerDDJSX2.handle14bitEQ(channel, control, value, PioneerDDJSX2.midi14bit.eqLow, 0x0F, 0x2F, 'parameter1');
};

PioneerDDJSX2.handle14bitEQ = function(channel, control, value, storage, msbCC, lsbCC, param) {
    var deckGroup = '[EqualizerRack1_[Channel' + (channel + 1) + ']_Effect1]';
    if (control === msbCC) {
        storage[channel] = value;
        engine.setParameter(deckGroup, param, ((value - 64) / 63.0 + 1) / 2);
    } else if (control === lsbCC) {
        var value14bit = (storage[channel] << 7) | value;
        engine.setParameter(deckGroup, param, ((value14bit - 8192) / 8191.0 + 1) / 2);
    }
};

// Filter Knobs (14-bit)
PioneerDDJSX2.Filter = function(channel, control, value) {
    var storage = PioneerDDJSX2.midi14bit.filter;
    if (control >= 0x17 && control <= 0x1A) {
        var offset = control - 0x17;
        storage[offset] = value;
        engine.setParameter('[QuickEffectRack1_[Channel' + (offset + 1) + ']]', 'super1', ((value - 64) / 63.0 + 1) / 2);
    } else if (control >= 0x37 && control <= 0x3A) {
        var offset = control - 0x37;
        var value14bit = (storage[offset] << 7) | value;
        engine.setParameter('[QuickEffectRack1_[Channel' + (offset + 1) + ']]', 'super1', ((value14bit - 8192) / 8191.0 + 1) / 2);
    }
};

// Trim/Pregain Knobs (14-bit)
// Includes special curve mapping because SX2 hardware trim knobs are logarithmic/weird.
PioneerDDJSX2.Pregain = function(channel, control, value) {
    var deckGroup = '[Channel' + (channel + 1) + ']';
    var cache = PioneerDDJSX2.midi14bit.pregain;
    var value14bit;

    // 1. DUAL-UPDATE LOGIC (Linear and single-curved approaches didn't work with these knobs)
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
        // ZONE A: 0 to Center (12 o'clock)
        // Map MIDI (0 -> 8192) to Gain (0.0 -> 1.0)
        var norm = value14bit / centerVal;

        // x^3 suppresses the value in the early stages.
        // 9 o'clock physical read significantly lower in software.
        gainValue = norm * norm * norm;

    } else {
        // ZONE B: Center to Max
        // Map MIDI (8192 -> 12550) to Gain (1.0 -> 4.0)

        // Normalize this upper section to 0.0 - 1.0
        var norm = (value14bit - centerVal) / (maxVal - centerVal);

        // Clamp to 1.0 in case of hardware jitter at max
        if (norm > 1.0) {
            norm = 1.0;
        }

        // Scale linearly to the top range (Unity to +12dB)
        // 1.0 (Start) + (norm * 3.0 range)
        gainValue = 1.0 + (norm * 3.0);
    }

    engine.setValue(deckGroup, 'pregain', gainValue);
};

// =============================================================================
// 9. FX SECTION 
// =============================================================================

// Handle Beats Knob (Turning) - Adjusts Wet/Dry Mix
PioneerDDJSX2.EffectJog = function(value, group, control) {
    if (control > 63) {
        engine.setValue("[EffectRack1_EffectUnit" + (value - 3) + "]", "mix", engine.getValue("[EffectRack1_EffectUnit" + (value - 3) + "]", "mix") - 0.0625 * (128 - control));
    } else {
        engine.setValue("[EffectRack1_EffectUnit" + (value - 3) + "]", "mix", engine.getValue("[EffectRack1_EffectUnit" + (value - 3) + "]", "mix") + 0.0625 * (control));
    }
};

// Beats Button Press (Cycles selected effect focus)
PioneerDDJSX2.BeatsPressFX = function(value, group, control) {
    if (control === 127) {
        PioneerDDJSX2.currentEffect[value - 4]++;
        if (PioneerDDJSX2.currentEffect[value - 4] > 3) {
            PioneerDDJSX2.currentEffect[value - 4] = 0;
        }
        PioneerDDJSX2.FXLeds();
    }
};

// Shift + Beats Button Press (Cycles parameters)
PioneerDDJSX2.ShiftBeatsPressFX = function(value, group, control) {
    if (control === 127) {
        var idx = ((value === 5) ? (4) : (0)) + PioneerDDJSX2.currentEffect[value - 4];
        PioneerDDJSX2.currentEffectparamset[idx]++;
        if (PioneerDDJSX2.currentEffectparamset[idx] >= (engine.getValue("[EffectRack1_EffectUnit" + (value - 3) + "_Effect" + (PioneerDDJSX2.currentEffect[value - 4] + 1) + "]", "num_parameters") / 3)) {
            PioneerDDJSX2.currentEffectparamset[idx] = 0;
        }
        PioneerDDJSX2.FXLeds();
    }
};

// Select specific effect in slot
PioneerDDJSX2.EffectSelect = function(value, group, control) {
    engine.setValue("[EffectRack1_EffectUnit" + (value - 3) + "_Effect" + (group - 98) + "]", "effect_selector", (control === 127) ? 1 : 0);
};

// Control FX Parameters via knobs
PioneerDDJSX2.EffectKnob = function(value, group, control) {
    if (PioneerDDJSX2.currentEffect[value - 4] === 3) {
        switch (group) {
            case 2:
                engine.setValue("[EffectRack1_EffectUnit" + (value - 3) + "_Effect1]", "meta", control / 127);
                break;
            case 4:
                engine.setValue("[EffectRack1_EffectUnit" + (value - 3) + "_Effect2]", "meta", control / 127);
                break;
            case 6:
                engine.setValue("[EffectRack1_EffectUnit" + (value - 3) + "_Effect3]", "meta", control / 127);
                break;
        }
    } else {
        var effectNumer = PioneerDDJSX2.currentEffect[value-4] + 1;
        var offsetNumer = (PioneerDDJSX2.currentEffectparamset[((value-4)*4)+PioneerDDJSX2.currentEffect[value-4]]*3);
        switch (group) {
            case 2:
                engine.setParameter("[EffectRack1_EffectUnit" + (value - 3) + "_Effect" + effectNumer + "]", "parameter" + (1 + offsetNumer), control/127);
                break;
            case 4:
                engine.setParameter("[EffectRack1_EffectUnit" + (value - 3) + "_Effect" + effectNumer + "]", "parameter" + (2 + offsetNumer), control/127);
                break;
            case 6:
                engine.setParameter("[EffectRack1_EffectUnit" + (value - 3) + "_Effect" + effectNumer + "]", "parameter" + (3 + offsetNumer), control/127);
                break;
        }
        
    }
};

// Enable/Disable specific effect
PioneerDDJSX2.EffectButton = function(value, group, control) {
    if (control === 127) {
        if (PioneerDDJSX2.currentEffect[value - 4] === 3) {
            engine.setValue("[EffectRack1_EffectUnit" + (value - 3) + "_Effect" + (group - 70) + "]", "enabled", !engine.getValue("[EffectRack1_EffectUnit" + (value - 3) + "_Effect" + (group - 70) + "]", "enabled"));
        }
    }
};

// Tap Button (Mix Mode Toggle)
PioneerDDJSX2.EffectTap = function(value, group, control) {
    if (control === 127 && PioneerDDJSX2.currentEffect[value - 4] === 3) {
        engine.setValue("[EffectRack1_EffectUnit" + (value - 3) + "]", "mix_mode", !engine.getValue("[EffectRack1_EffectUnit" + (value - 3) + "]", "mix_mode"));
        PioneerDDJSX2.FXLeds();
    }
};

// =============================================================================
// 10. VIEW, GRID, & PANELS 
// =============================================================================

// Cycle through Mixxx Panel layouts (FX Rack, Samplers)
PioneerDDJSX2.PanelSelect = function(value, group, control) {
    if (control === 127) {
        PioneerDDJSX2.curPanel += ((1 - (group - 120)) * 2) - 1;
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
PioneerDDJSX2.ViewButton = function(value, group, control) {
    if (control === 127) {
        PioneerDDJSX2.curView++;
        if (PioneerDDJSX2.curView > 7) {
            PioneerDDJSX2.curPanel = 0;
        }

        engine.setValue("[Skin]", "show_4decks", PioneerDDJSX2.curView & 1);
    }
};

// Enable Grid Slide Mode (used with Jog Wheel)
PioneerDDJSX2.SetGridSlide = function(value, group, control) {
    PioneerDDJSX2.gridSlide[value] = control ? 1 : 0;
    midi.sendShortMsg(0x90 + value, 0x0a, control ? 0x7F : 0x00);
};

// Enable Grid Adjust Mode (used with Jog Wheel)
PioneerDDJSX2.SetGridAdjust = function(value, group, control) {
    PioneerDDJSX2.gridAdjust[value] = control ? 1 : 0;
    midi.sendShortMsg(0x90 + value, 0x79, control ? 0x7F : 0x00);
};

// Clear Grid Adjustment
PioneerDDJSX2.ClearGrid = function(value, group, control) {
    engine.setValue("[Channel" + (value + 1) + "]", "beats_undo_adjustment", 1);
    midi.sendShortMsg(0x90 + value, 0x79, control ? 0x7F : 0x00);
};

// =============================================================================
// 11. SAMPLER & PAD PARAMETERS 
// =============================================================================

// Plays sampler or Loads selected track if empty
PioneerDDJSX2.SamplerPlay = function(group, control, value, status) {
    var sampler = "[Sampler" + (1 + (control & 7) + (PioneerDDJSX2.samplerBank[group - 7] * 8)) + "]";
    if (value === 127) {
        if (engine.getValue(sampler, "track_loaded")) {
            engine.setParameter(sampler, "start_play", value & 1);
        } else {
            engine.setParameter(sampler, "LoadSelectedTrack", value & 1);
        }
    }
};

// Stops sampler (Short press) or Ejects (Long press)
PioneerDDJSX2.SamplerStop = function(group, control, value, status) {
    var sampler = "[Sampler" + (1 + (control & 7) + (PioneerDDJSX2.samplerBank[group - 7] * 8)) + "]";
    var isPlaying = engine.getValue(sampler, "play");
    var trackLoaded = engine.getValue(sampler, "track_loaded");

    if (value === 127) {
        if (trackLoaded && !isPlaying) {
            engine.setParameter(sampler, "eject", 1);
        } else {
            engine.setParameter(sampler, "stop", 1);
        }
    }
};

// Updates Sampler LEDs based on state (Loaded/Playing)
PioneerDDJSX2.SamplerLight = function(value, group, control) {
    var sampler = PioneerDDJSX2.enums.samplerGroups[group];
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
                color = 0x7f;
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
    PioneerDDJSX2.sampleVolume[where] = value / 127;
    engine.setParameter("[Sampler" + (1 + where) + "]", "pregain", PioneerDDJSX2.samplerVolume * PioneerDDJSX2.sampleVolume[where]);
};

// Master Sampler Volume Knob
PioneerDDJSX2.SetSamplerVol = function(value, group, control) {
    PioneerDDJSX2.samplerVolume = control / 127;
    var num = engine.getValue('[App]', 'num_samplers');
    for (var i = 0; i < num; i++) {
        engine.setParameter("[Sampler" + (i + 1) + "]", "pregain", PioneerDDJSX2.samplerVolume * PioneerDDJSX2.sampleVolume[i]);
    }
};

// Refreshes all sampler LEDs (used when switching banks)
PioneerDDJSX2.RepaintSampler = function(group) {
    for (var i = 0; i < 8; i++) {
        var ai = i + PioneerDDJSX2.samplerBank[group] * 8;
        var prefix = "[Sampler" + (ai + 1) + "]";
        var loaded = engine.getValue(prefix, "track_loaded");
        var playing = engine.getValue(prefix, "play");

        var color = 0;
        if (loaded) {
            color = playing ? 0x40 : 0x7F;
        }

        if (PioneerDDJSX2.padMode[group] === 3) {
            midi.sendShortMsg(0x97 + group, 0x30 + i, color);
            midi.sendShortMsg(0x97 + group, 0x38 + i, color);
        } else if (PioneerDDJSX2.padMode[group] === 7) {
            midi.sendShortMsg(0x97 + group, 0x70 + i, color);
            midi.sendShortMsg(0x97 + group, 0x78 + i, color);
        }
    }
};
// Cycle Tempo Ranges
PioneerDDJSX2.SetTempoRange = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.tempoRange[group]++;
        if (PioneerDDJSX2.tempoRange[group] > 3) {
            PioneerDDJSX2.tempoRange[group] = 0;
        }
        engine.setValue(`[Channel${  group + 1  }]`, "rateRange", PioneerDDJSX2.settings.tempoRanges[PioneerDDJSX2.tempoRange[group]]);
    }
};

// Previous Sampler Bank
PioneerDDJSX2.SamplerParam1L = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.samplerBank[group]--;
        if (PioneerDDJSX2.samplerBank[group] < 0) {
            PioneerDDJSX2.samplerBank[group] = 0;
        } else {
            PioneerDDJSX2.RepaintSampler(group);
        }
    }
};

// Next Sampler Bank
PioneerDDJSX2.SamplerParam1R = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.samplerBank[group]++;
        if (PioneerDDJSX2.samplerBank[group] > 7) {
            PioneerDDJSX2.samplerBank[group] = 7;
        } else {
            PioneerDDJSX2.RepaintSampler(group);
        }
    }
    midi.sendShortMsg(0x90 + group, 0x2F, 0x7F);
};

// Decrease Roll Size with Parameter 1
PioneerDDJSX2.RollParam1L = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.rollPrec[group]--;
        if (PioneerDDJSX2.rollPrec[group] < 0) {
            PioneerDDJSX2.rollPrec[group] = 0;
        }
        midi.sendShortMsg(0x90 + group, 0x1e, PioneerDDJSX2.settings.rollColors[PioneerDDJSX2.rollPrec[group]]);
    }
};

// Increase Roll Size with Parameter 1
PioneerDDJSX2.RollParam1R = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.rollPrec[group]++;
        if (PioneerDDJSX2.rollPrec[group] > 4) {
            PioneerDDJSX2.rollPrec[group] = 4;
        }
        midi.sendShortMsg(0x90 + group, 0x1e, PioneerDDJSX2.settings.rollColors[PioneerDDJSX2.rollPrec[group]]);
    }
};

// Decrease Slicer Precision with Parameter 1
PioneerDDJSX2.SlicerParam1L = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.beatjumpPrec[group]--;
        if (PioneerDDJSX2.beatjumpPrec[group] < 0) {
            PioneerDDJSX2.beatjumpPrec[group] = 0;
        }
        midi.sendShortMsg(0x90 + group, 0x20, PioneerDDJSX2.settings.beatjumpColors[PioneerDDJSX2.beatjumpPrec[group]]);
    }
};

// Increase Slicer Precision with Parameter 1
PioneerDDJSX2.SlicerParam1R = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.beatjumpPrec[group]++;
        if (PioneerDDJSX2.beatjumpPrec[group] > 8) {
            PioneerDDJSX2.beatjumpPrec[group] = 8;
        }
        midi.sendShortMsg(0x90 + group, 0x20, PioneerDDJSX2.settings.beatjumpColors[PioneerDDJSX2.beatjumpPrec[group]]);
    }
};

// Parameter 1 for HOT CUE (Right)
// Not Used
PioneerDDJSX2.CueParam1R = function(group, control, value, status, channel) {
};

// Parameter 1 for HOT CUE (Left)
// Not Used
PioneerDDJSX2.CueParam1L = function(group, control, value, status, channel) {
};

// Parameter 1 for HOT CUE LOOPS (Left)
// Not Used
PioneerDDJSX2.CueLoopParam1R = function(group, control, value, status, channel) {
};

// Parameter 1 for HOT CUE LOOPS (Left)
// Not Used
PioneerDDJSX2.CueLoopParam1L = function(group, control, value, status, channel) {
};

// Feedback for Roll Pads
PioneerDDJSX2.RollPerformancePadLed = function(value, group, control) {
    var channel = PioneerDDJSX2.enums.channelGroups[group];
    var padIndex = 0;
    for (var i = 0; i < 8; i++) {
        if (control === 'beatloop_' + PioneerDDJSX2.settings.loopIntervals[i + 2] + '_enabled') {
            break;
        }
        padIndex++;
    }
    if (engine.getValue('[Channel1]', 'play')) {
        midi.sendShortMsg(0x97 + channel, 0x10 + padIndex, value ? 0x7F : 0x00);
    }
};

// =============================================================================
// 12. PAD MODES
// =============================================================================

// Enable HotCue Mode
PioneerDDJSX2.SetHotCueMode = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.padMode[group] = 0;
        midi.sendShortMsg(0x90 + group, 0x1b, 0x7f);
    }
};

// Enable Roll Mode
PioneerDDJSX2.SetRollMode = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.padMode[group] = 1;
        midi.sendShortMsg(0x90 + group, 0x1e, PioneerDDJSX2.settings.rollColors[PioneerDDJSX2.rollPrec[group]]);
    }
};

// Enable Slicer Mode
PioneerDDJSX2.SetSlicerMode = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.padMode[group] = 2;
        midi.sendShortMsg(0x90 + group, 0x20, 0x7f);
    }
};

// Enable Sampler Mode
PioneerDDJSX2.SetSamplerMode = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.padMode[group] = 3;
        PioneerDDJSX2.RepaintSampler(group);
        midi.sendShortMsg(0x90 + group, 0x22, 0x7f);
    }
};

// Enable Cue Loop Mode
PioneerDDJSX2.SetCueLoopMode = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.padMode[group] = 4;
        midi.sendShortMsg(0x90 + group, 0x69, PioneerDDJSX2.settings.cueLoopColors[3]);
    }
};

// Enable Saved Loop Mode
PioneerDDJSX2.SetSavedLoopMode = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.padMode[group] = 5;
        midi.sendShortMsg(0x90 + group, 0x6b, 0x7f);
    }
};

// Enable Slicer Loop Mode
PioneerDDJSX2.SetSlicerLoopMode = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.padMode[group] = 6;
        midi.sendShortMsg(0x90 + group, 0x6d, 0x7f);
    }
};

// Enable Velocity Sampler Mode
PioneerDDJSX2.SetVelocitySamplerMode = function(group, control, value, status) {
    if (value === 127) {
        PioneerDDJSX2.padMode[group] = 7;
        PioneerDDJSX2.RepaintSampler(group);
        midi.sendShortMsg(0x90 + group, 0x6f, 0x7f);
    }
};

// Logic for Saved Loop Pads
PioneerDDJSX2.SavedLoop = function(performanceChannel, control, value, status) {
    var deck = performanceChannel - 6;
    var group = '[Channel' + deck + ']';
    var interval = PioneerDDJSX2.settings.loopIntervals[control - 0x50 + 4];
    var isAutoLoopEnabled = engine.getValue(group, 'beatloop_' + interval + '_enabled');

    if (value === 0x7F) {
        for (var i = 0; i < 8; i++) {
            midi.sendShortMsg(0x97 + deck - 1, 80 + i, 0x00);
        }

        if (isAutoLoopEnabled !== 1) {
            engine.setValue(group, 'beatloop_' + interval + '_activate', 1);
            midi.sendShortMsg(0x97 + deck - 1, control, 0x7f);
        } else {
            engine.setValue(group, 'beatloop_' + interval + '_toggle', 1);
            midi.sendShortMsg(0x97 + deck - 1, control, 0x00);
        }
    }
};

// Logic for Beat Jump Pads
// TODO(XXX) : BeatJump is mapped to Slicer, create a real Slicer or wait for Mixxx to implement one. 
PioneerDDJSX2.BeatJump = function(performanceChannel, control, value, status) {
    var deck = performanceChannel - 7;
    var group = '[Channel' + (deck + 1) + ']';
    const which = (control & 3) + PioneerDDJSX2.beatjumpPrec[deck];

    if (value === 0x7F) {
        var interval = PioneerDDJSX2.settings.loopIntervals[which];
        if (control & 4) {
            engine.setValue(group, 'beatjump_' + interval + '_backward', 1);
        } else {
            engine.setValue(group, 'beatjump_' + interval + '_forward', 1);
        }
    }
    midi.sendShortMsg(0x97 + deck, control, (value === 0x7f) ? (PioneerDDJSX2.settings.beatjumpColors[which]) : (0x00));
};

// Logic for Roll Pads
PioneerDDJSX2.RollPerformancePad = function(performanceChannel, control, value, status) {
    var deck = performanceChannel - 6;
    var group = '[Channel' + deck + ']';
    var interval = PioneerDDJSX2.settings.loopIntervals[control - 0x10 + PioneerDDJSX2.rollPrec[performanceChannel - 7]];

    if (value === 0x7F) {
        engine.setValue(group, 'beatlooproll_' + interval + '_activate', 1);
    } else {
        engine.setValue(group, 'beatlooproll_' + interval + '_activate', 0);
    }

    midi.sendShortMsg(0x97 + deck - 1, control, (value === 0x7f) ? (PioneerDDJSX2.settings.rollColors[PioneerDDJSX2.rollPrec[performanceChannel - 7]]) : (0x00));
};

// =============================================================================
// 13. LIBRARY & BROWSER
// =============================================================================

// Rotary Selector Navigation (Tracklist / Library)
PioneerDDJSX2.RotarySelector = function(channel, control, value, status) {
    var delta = 0x40 - Math.abs(0x40 - value);
    if (value > 0x40) {
        delta *= -1;
    }

    if (PioneerDDJSX2.status.rotarySelector.target === 1) {
        engine.setValue('[Playlist]', 'SelectTrackKnob', delta);
    } else if (delta > 0) {
        engine.setValue('[Playlist]', 'SelectNextPlaylist', 1);
    } else if (delta < 0) {
        engine.setValue('[Playlist]', 'SelectPrevPlaylist', 1);
    }
};

// Sort Library via Shift + Load Buttons
PioneerDDJSX2.SortLibrary = function(performanceChannel, control, value, status) {
    if (value === 127) {
        if (control === 88) {
            engine.setValue("[Library]", 'sort_column_toggle', 15);
        }
        if (control === 89) {
            engine.setValue("[Library]", 'sort_column_toggle', 2);
        }
        if (control === 96) {
            engine.setValue("[Library]", 'sort_column_toggle', 9);
        }
        if (control === 97) {
            engine.setValue("[Library]", 'sort_column_toggle', 1);
        }
    }
};

// Back Button - returns focus to Library sidebar
PioneerDDJSX2.BackButton = function(channel, control, value, status) {
    if (value === 0x7F) {
        PioneerDDJSX2.status.rotarySelector.target = PioneerDDJSX2.enums.rotarySelector.targets.libraries;
    }
};

// Rotary Click - Toggles folder expansion or enters playlist
PioneerDDJSX2.RotarySelectorClick = function(channel, control, value, status) {
    if (value === 0x7F) {
        var target = PioneerDDJSX2.enums.rotarySelector.targets.tracklist;

        var tracklist = PioneerDDJSX2.enums.rotarySelector.targets.tracklist;
        var libraries = PioneerDDJSX2.enums.rotarySelector.targets.libraries;

        // Expand/Collapse the current item
        engine.setValue("[Playlist]", "ToggleSelectedSidebarItem", 1);

        switch (PioneerDDJSX2.status.rotarySelector.target) {
            case tracklist:
                target = libraries;
                break;
            case libraries:
                target = tracklist;
                break;
        }

        PioneerDDJSX2.status.rotarySelector.target = target;
    }
};

// Shift + Rotary Click - Loads selected track
PioneerDDJSX2.rotarySelectorShiftedClick = function(channel, control, value) {
    if (value) {
        script.toggleControl("[Library]", "GoToItem");
    }
};

// =============================================================================
// 14. FLIP SECTION
// =============================================================================
// This is the Flip section of each deck, Mixxx doesn't have Flip feature,
// so we can map them to something else

// Flip Start Button, toggle Loop Anchor
PioneerDDJSX2.FlipStart = function(group, control, value, status, channel) {
    if (value === 127) {
        script.toggleControl(channel, "loop_anchor");
        var isAnchor = engine.getValue(channel, "loop_anchor");
        midi.sendShortMsg(0x90 + group, 0x4B, isAnchor ? 0x7F : 0x00);
    }
};

// Flip Rec Button, toggle Hot Cue Quantize
PioneerDDJSX2.FlipRec = function(group, control, value, status, channel) {
    if (value === 127) {
        script.toggleControl(channel, "quantize");
        var isQuantize = engine.getValue(channel, "quantize");
        midi.sendShortMsg(0x90 + group, 0x4A, isQuantize ? 0x7F : 0x00);
    }
};

// Flip Slot Button, Reset current track key to default (pitch)
PioneerDDJSX2.FlipSlot = function(group, control, value, status, channel) {
    if (engine.getValue(channel, "track_loaded")) {
        if (value === 127) {
            engine.setValue(channel, "pitch_adjust_set_zero", 1);
        }
    }
};

// Flip Save (SHIFT + Slot) Button, Set the pitch of the track down
PioneerDDJSX2.FlipSave = function(group, control, value, status, channel) {
    if (engine.getValue(channel, "track_loaded")) {
        if (value === 127) {
            engine.setValue(channel, "pitch_down", 1);
        }
    }
};

// Flip Save (SHIFT + Rec) Button, repeat setting for the current track
PioneerDDJSX2.FlipLoop = function(group, control, value, status, channel) {
    if (value === 127) {
        script.toggleControl(channel, "repeat");
        var isAnchor = engine.getValue(channel, "repeat");
        midi.sendShortMsg(0x90 + group, 0x5A, isAnchor ? 0x7F : 0x00);
    }
};

// Flip Save (SHIFT + Start) Button, Set the pitch of the track up
PioneerDDJSX2.FlipOnOff = function(group, control, value, status, channel) {
    if (engine.getValue(channel, "track_loaded")) {
        if (value === 127) {
            engine.setValue(channel, "pitch_up", 1);
        }
    }
};
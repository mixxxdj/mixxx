

/*
 * Pioneer DDJ-REV1 mapping for Mixxx
 * Files: Pioneer-DDJ-REV1-scripts.js, Pioneer DDJ-REV1.midi.xml
 *
 * Copyright (C) 2024-2026 AKOI, Alexandr, LauraRozier, Mjlyon, 0b1-twan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 */

// @ts-nocheck
if (typeof include === "function") {
    include("controllers/components.js");
} else if (typeof script !== "undefined" && typeof script.include === "function") {
    script.include("controllers/components.js");
}

// ****************************************************************************
// * Authors: AKOI, Alexandr, LauraRozier, Mjlyon, 0b1-twan 
// * Reviewers: ronso, martinprad0
// * Manual: https://downloads.support.alphatheta.com/manuals/DDJ_REV1_DRI1744B_manual/?page=1
// * Forum: https://mixxx.discourse.group/t/pioneer-ddj-rev1-mapping-update-2-6/32603
// * MIDI: https://downloads.support.alphatheta.com/software_info/dj-controllers/DDJ-REV1/DDJ-REV1_MIDI_Message_List_E1.pdf
// * User Guide: https://manual.mixxx.org/2.6/en/hardware/controllers/pioneer_ddj_rev1
// * Mapping Version: AKOI v2.4 mapping for the Pioneer DDJ-REV1
// ****************************************************************************
//
//  Implemented (as per manufacturer's manual):
//      * Mixer Section (Faders, EQ, Filter, Gain, Cue, Selector)
//      * Browsing and loading
//      * Vinyl/CDJ Mode
//      * Jogwheels, Scratching, Bending, Loop adjust
//      * Cycle Temporange
//      * Beat Sync 
//      * Hot Cue Mode 
//      * Auto Loop Mode 
//      * Tracking Mode 
//      * Beat Jump Mode 
//      * Roll Mode 
//      * Trans Mode 
//      * Vu Meter
//      * Sampler Mode 
//      * Fader Start
// 		* Level/Depth working for FX1/FX2 
//      * Waveform Zoom
//      * FX Lights
//	    * Scratch Bank
//
//  Custom (Mixxx specific mappings):
//      *Vinyl Mode: 
//                 - Starts in Vinyl mode. User configurable
//      - STEMS **MIXXX_2.6**
//      * Sampler Mode(Pads 1-8): 
//	                  -Play/Pause
//					  		Pressed(while sample loaded): Play Sample 
//							Pressed(while sample empty): Load currently selected track
//							Pressed+Shift (while sample playing): Stop Sample
//							Pressed+Shift (while sample stopped): Eject Sample
//		* Beat Sync:
//					Sync Pressed: Beat Sync to Master deck
//					Sync Pressed+Shift: Lock Beat Sync ON
//      * Beat jump: 
//				    - User configurable
//		* Auto Loop: 
//					- Pads 1-8 trigger an autoloop of variable sizes 
//		* Roll: 
//					- Holding pads 1-4 activates a loop roll of varying sizes. Release the pad to exit the Loop Roll
//		* Effects:
//				-LEVEL/DEPTH knob:
//					Adjusts the parameter of the enabled effects of FX1/FX2
//				-SHIFT+FX1: 
//					Cycle to the next EffectChain preset after the currently loaded preset. (Descending) 
//				-SHIFT+FX2: 
//					Adjust the average BPM up by +0.01 - The beatgrid lines move closer to each other
//				-SHIFT+FX3: 
//					Adjust the average BPM down by -0.01 - The beatgrid lines move further apart from each other
//              -FX{N}_{N} + Rotary Selector:
//                  Designate effect for selected FX button. (Descending) 
//          
//		* CUE HEADPHONES: 
//      				-Toggle quantize:
//							Press Once:  CUE Headphones+SHIFT
//						-BPM Adjust:
//							Repeatedly Press: CUE Headphones+SHIFT - press - Adjust BPM to match tapped BPM 
//      * CrossFader Start: 
//                      - Shift + Crossfader: All the way left or right. See manual
//                      - To disable: Utilities Mode Pad 3 is lit: Fader Start is turned off
//                      - Limitation: - Playing deck is always opposite of starting deck irrespective of active opposite deck
//                                    - Controller only sends 0x52 despite Utilities mode settings Sync(0x51) and Non-Sync(0x66)  
//      * Channel Fader Start:
//                      - Shift + UP or Down of opposite Channel Fader: All the way down or up.  Varies from manual. 
//                      - To disable: Utilities Mode Pad 3 is lit: Fader Start is turned off. (MIXXX RESTART may be required)
//                      - Limitation: - Playing deck is always opposite of starting deck irrespective of active opposite deck
//                                    - Controller only sends 0x52 despite Utilities mode settings Sync(0x51) and Non-Sync(0x66)  
//		* Library Sort: 
//                      - Decks 1-4: Sort by configured library column (see controller preferences):
//                          Press SHIFT + LOAD on that deck
//                          Configure per-deck sort target in XML user options
//		* Bonus:
//				- Fixed Tempo Sliders:
//					Tempo Sliders now aligned with REV1 Deck

//				-Scratch Bank/STEMS (All Decks):
//					Pad 1: STEM VOICE Mute
//					Pad 2: STEM MELODY Mute
//					Pad 3: STEM BASS Mute 
//					Pad 4: STEM DRUM Mute
//				    Pad 5: STEM Voice Effect  
//                  Pad 6: STEM MELODY Effect 
//                  Pad 7: STEM BASS Effect 
//                  Pad 8: STEM DRUM Effect   
//             
//					Pad 1 + SHIFT: AutoDJ
//					Pad 2 + SHIFT: AutoDJ Fade to Next
//					Pad 3 + SHIFT: Toggle Microphone
//					Pad 4 + SHIFT: Record Mix
//					Pad 5 + SHIFT: Key Match 
//					Pad 6 + SHIFT: Beat Grid
//					Pad 7 + SHIFT: Pitch UP
//					Pad 8 + SHIFT: Pitch Down
//
///					Pad 1 + Level/Depth: STEM VOICE Volume
//					Pad 2 + Level/Depth: STEM MELODY Volume
//					Pad 3 + Level/Depth: STEM BASS Volume
//					Pad 4 + Level/Depth: STEM DRUM Volume
///					Pad 5 + Level/Depth: STEM EFFECT Volume 
//					Pad 6 + Level/Depth: STEM EFFECT Volume
//					Pad 7 + Level/Depth: STEM EFFECT Volume 
//					Pad 8 + Level/Depth: STEM EFFECT Volume
//              -Lights behavior:
//                  - Stem pads lights and Mixxx skin will light on load(button) of STEM file, off on Load of non STEM file
//                  -Vinyl/CDJ:
//                      - Vinyl mode: 2 fast blink 
//                      - CDJ mode: 3 slower blink
//
//              -Sampler Volume (All Decks): 
//					-Sampler + Level/Depth: Sets Samplers 1-8 Gain levels
//                  -Configurable show sampler skin when enabled.
//              -Waveform zoom (All Decks):
//                  -If configuration enabled, Vinyl mode pitch bend is disabled and jog wheel side controls waveform zoom. CDJ mode unchanged.
//              -Mixxxed Mode
//      
//


// eslint-disable-next-line no-var -- must be global for functionprefix
var PioneerDDJREV1 = {};


/////////////////////////************/////////////////////////////////
//                       USER OPTIONS                              //
/////////////////////////***********////////////////////////////////
// If true DECK{N} start in Vinyl Mode, if false DECK{N} start in CDJ Mode. 
PioneerDDJREV1.vinylMode = [true, true, true, true];

// Per-deck SHIFT+LOAD library sort fields.
PioneerDDJREV1.librarySortDefaults = ["bpm", "artist", "date", "key"];
PioneerDDJREV1.librarySortDeckModes = PioneerDDJREV1.librarySortDefaults.slice(0);
// Last SHIFT+LOAD sort: repeat same field toggles order; different field uses sortByField default.
PioneerDDJREV1.lastLibrarySortField = null;
PioneerDDJREV1.lastLibrarySortOrder = null;

// If true, AutoSlip is enabled in Vinyl mode:
// jog touch enables slip and jog release disables slip.
PioneerDDJREV1.VinylSlipAutoff = false;


// If true, sampler UI is shown only while sampler volume is being adjusted.
PioneerDDJREV1.tempSamplerSkin = false;
PioneerDDJREV1.disableStartFader = false;

// Waveform zoom:
// - waveformZoomEnabled: zoom enabled
// - waveformZoomMode: "vinyl" | "cdj" determines which deck mode gets zoom-on-bend
PioneerDDJREV1.waveformZoomEnabled = false;
PioneerDDJREV1.waveformZoomMode = "vinyl";

// VU meter routing:
// - "per_deck": legacy (each deck meter follows its own channel)
// - "main_lr": all deck meters track `[Main]` L/R master output.
PioneerDDJREV1.vuMeterMode = "per_deck";

// Braking behavior for play/pause transitions.
PioneerDDJREV1.brakingEnabled = false;
PioneerDDJREV1.brakingStartProfile = "off";
PioneerDDJREV1.brakingStopProfile = "off";
PioneerDDJREV1.brakingReverse = false;
// When false, non-classic brake profiles still use engine.brake(); only the delayed play=0 watchdog timer is skipped.
PioneerDDJREV1.brakeForceStopTimerEnabled = true;

// LED pulse behavior safeguards.
PioneerDDJREV1.syncPressPulseEnabled = true;
PioneerDDJREV1.syncPressPulseMs = 180;
PioneerDDJREV1.cdjVinylTogglePulseEnabled = false;

//To disable Fader Start(s)p enter "Utilities Mode". Press Performance Pads 3 on the left deck. Fader Start is turned off.

// Level/Depth routing: Off (default) controls both FX units; On routes FX1 vs SHIFT+FX2.
PioneerDDJREV1.splitFx = false;

////////////////////////*************/////////////////////////////////
//                       USER OPTIONS                              //
////////////////////////************////////////////////////////////



// MIDI LED velocity constants
PioneerDDJREV1.LED_OFF = 0x00;
PioneerDDJREV1.LED_ON  = 0x7F;

PioneerDDJREV1.lights = {
    beatFx: {
        status: 0x94,
        data1: 0x47,
    },
    shiftBeatFx: {
        status: 0x94,
        data1: 0x43,
    },
};
(function() {
    const deckLightDefs = [
        { key: "vuMeter",        statusBase: 0xB0, data1: 0x02, },
        { key: "deckSelect",     statusBase: 0x90, data1: 0x17, },
        { key: "playPause",      statusBase: 0x90, data1: 0x0B, },
        { key: "shiftPlayPause", statusBase: 0x90, data1: 0x47, },
        { key: "cue",            statusBase: 0x90, data1: 0x0C, },
        { key: "shiftCue",       statusBase: 0x90, data1: 0x48, },
    ];
    for (let d = 0; d < 4; d++) {
        let deck = {};
        for (let j = 0; j < deckLightDefs.length; j++) {
            const def = deckLightDefs[j];
            deck[def.key] = { status: def.statusBase + d, data1: def.data1, };
        }
        PioneerDDJREV1.lights["deck" + (d + 1)] = deck;
    }
})();

PioneerDDJREV1._samplerGroups = [];
(function() {
    for (let i = 1; i <= 16; i++) {
        PioneerDDJREV1._samplerGroups.push("[Sampler" + i + "]");
    }
})();

PioneerDDJREV1.lastStemChannel = null;

// Store timer IDs
PioneerDDJREV1.timers = {};
PioneerDDJREV1.brakeForceStopTimers = {};
PioneerDDJREV1.brakeGeneration = {};
PioneerDDJREV1.syncPulseTimers = {};
PioneerDDJREV1.syncPulseIgnoreUntil = {};

// Jogwheels
PioneerDDJREV1.alpha = 1.0 / 8;
PioneerDDJREV1.beta = PioneerDDJREV1.alpha / 32;
PioneerDDJREV1.nonShiftScratchResolution = 720;

// Multiplier for fast seek through track using SHIFT+JOGWHEEL
PioneerDDJREV1.fastSeekScale = 50;
PioneerDDJREV1.bendScale = 0.8;

PioneerDDJREV1.tempoRangeProfile = "default";
// Must match Components.Settings.tempoRangesForProfile("default") until init applies user options.
PioneerDDJREV1.tempoRanges = [0.08, 0.16, 0.50];
PioneerDDJREV1.tempoRangeHoldTimers = [null, null, null, null];

// Object to store the tempo slider for each deck
PioneerDDJREV1.loopAdjustIn = [false, false, false, false];
PioneerDDJREV1.loopAdjustOut = [false, false, false, false];
PioneerDDJREV1.loopAdjustMultiply = 10;
PioneerDDJREV1.jogPlatterTouched = [false, false, false, false];
PioneerDDJREV1.brakingActive = {};
PioneerDDJREV1.brakeInterruptedByScratch = [false, false, false, false];
PioneerDDJREV1.hotcuePreviewFromBrake = {};
PioneerDDJREV1.hotcueHoldActive = {};

PioneerDDJREV1.stopAllLoopLightTimers = function() {
    let stopped = 0;
    const groups = PioneerDDJREV1.timers || {};
    Object.keys(groups).forEach(function(group) {
        const groupTimers = groups[group] || {};
        Object.keys(groupTimers).forEach(function(control) {
            const timerId = groupTimers[control];
            if (timerId !== undefined) {
                engine.stopTimer(timerId);
                groupTimers[control] = undefined;
                stopped += 1;
            }
        });
    });
    return stopped;
};

PioneerDDJREV1.clearDeckTimer = function(store, key, emptyValue) {
    const cleared = arguments.length > 2 ? emptyValue : null;
    const t = store[key];
    if (t != null) {
        engine.stopTimer(t);
        store[key] = cleared;
    }
};

// Beatjump pad (beatjump_size values)
PioneerDDJREV1.beatJumpActions = [
    "beatjump_backward",
    "beatjump_size_halve",
    "beatjump_size_double",
    "beatjump_forward",
    "prev", // go to previous track track, must be stopped/paused
    "back", // fast rewind
    "fwd", // fast forward
    "reverseroll" // censor
];


//These values are set in the Controller settings UI. 
// If JavaScript can't get the setting from the engine, then they use the default option
PioneerDDJREV1.beatLoopRollSizes = [
    1 / 16,
    1 / 8,
    1 / 4,
    1 / 2,
    1,
    2,
    4,
    8
];
PioneerDDJREV1.beatLoopRollDefaults = PioneerDDJREV1.beatLoopRollSizes.slice(0);
PioneerDDJREV1.beatLoopRollConfigs = PioneerDDJREV1.beatLoopRollDefaults.map(function(defaultSize) {
    return {
        mode: "fixed",
        value: defaultSize,
    };
});
PioneerDDJREV1.beatLoopPadResolvedSizes = PioneerDDJREV1.beatLoopRollDefaults.slice(0);

// Beatjump pad configs: full override via parseBeatJumpPadSetting (action + optional size).
PioneerDDJREV1.beatJumpDefaults = new Array(8).fill("default");
PioneerDDJREV1.beatJumpConfigs = PioneerDDJREV1.beatJumpDefaults.map(function() {
    return { kind: "default", };
});

// Autoloop pad size configs (defaults match current fixed autoloop keys).
PioneerDDJREV1.autoLoopDefaults = [0.25, 0.5, 1, 2, 4, 8, 16, 32];
PioneerDDJREV1.autoLoopConfigs = PioneerDDJREV1.autoLoopDefaults.map(function(defaultSize) {
    return { mode: "fixed", value: defaultSize, };
});
PioneerDDJREV1.autoLoopPadResolvedSizes = PioneerDDJREV1.autoLoopDefaults.slice(0);

PioneerDDJREV1.beatLoopPadStates = [false, false, false, false, false, false, false, false];

PioneerDDJREV1.shiftPressed = false;
PioneerDDJREV1.vuMeterLastSent = [-1, -1, -1, -1];
PioneerDDJREV1.vuMeterStereoLastSent = [-1, -1];
PioneerDDJREV1.stemShiftPressed = false; 
PioneerDDJREV1.stemEffectPressed = false;
PioneerDDJREV1.sampleShiftPressed = false;
PioneerDDJREV1.samplerChannel = 0;
PioneerDDJREV1.wave = 5;
let currentStemMute;
let currentStemEffect;

// Stem pad state is hot-path during pad spam. Use fixed-size arrays to avoid dynamic JS
// object property churn in the controller thread.
PioneerDDJREV1.stemShiftHeld = new Array(16).fill(false);
PioneerDDJREV1.stemEffectHeld = new Array(16).fill(false);
PioneerDDJREV1.stemPadMovedWhileHeld = new Array(16).fill(false);
PioneerDDJREV1.stemToggleLastMs = new Array(16).fill(0);
PioneerDDJREV1.stemLoadInProgress = [false, false, false, false];
PioneerDDJREV1.stemSyncTimers = [null, null, null, null];
PioneerDDJREV1.hotcueRefreshTimers = [null, null, null, null];
PioneerDDJREV1.supportsStems = false;
PioneerDDJREV1.detectedMixxxVersion = "";
PioneerDDJREV1.componentContainer = null;
PioneerDDJREV1.useComponentsJS = true;
PioneerDDJREV1.scratchBankOnStemPads = false;
PioneerDDJREV1.samplerMode = "standard16";
PioneerDDJREV1.samplePadLayout = "standard";
PioneerDDJREV1.resolvedOperatingPolicy = null;
PioneerDDJREV1.scratchBankGuardWarnings = {};

// Mixxxed-Mode (Auto Loop row): per-deck modes 1=autoloop 2=slicer 3=piano roll 4=scratch bank
PioneerDDJREV1.mixxxedModeEnabled = false;
PioneerDDJREV1._loadMixxxedModeInvocation = false;
PioneerDDJREV1.leftActiveDeck = 1;
PioneerDDJREV1.rightActiveDeck = 2;
PioneerDDJREV1.padRowMode = [1, 1, 1, 1];
PioneerDDJREV1.MixxxedModeLockTimer = [null, null, null, null];
PioneerDDJREV1.MixxxedModeLocked = [true, true, true, true];
PioneerDDJREV1.MixxxedModeLockHoldTimer = [null, null, null, null];
PioneerDDJREV1.MixxxedModeLockHoldActive = [false, false, false, false];
PioneerDDJREV1.pianoRollEngaged = [false, false, false, false];
PioneerDDJREV1.pianoRollScale = "major";
PioneerDDJREV1.beatSlicerPattern = "linear";
PioneerDDJREV1.pianoRollPitchDeck = [0, 0, 0, 0];
PioneerDDJREV1.pianoRollHeldCount = 0;
PioneerDDJREV1.pianoRollSavedKeylock = false;
PioneerDDJREV1.pianoRollAnchorRatio = 0;
PioneerDDJREV1.pianoRollAnchorSet = false;
PioneerDDJREV1.pianoRollHotcueOverride = 0;
PioneerDDJREV1.pianoRollArmedForPlay = false;
PioneerDDJREV1.pianoRollLatchedPlay = false;
PioneerDDJREV1.pianoRollLatchOnNextPlay = false;
PioneerDDJREV1.pianoRollPlaypositionConnection = null;
PioneerDDJREV1.pianoRollShiftBlinkTimer = null;
PioneerDDJREV1.pianoRollShiftBlinkOn = false;
PioneerDDJREV1.bigLibraryShiftPush = false;
PioneerDDJREV1.eqKnobMsb = [
    { low: 0, mid: 0, high: 0, },
    { low: 0, mid: 0, high: 0, },
    { low: 0, mid: 0, high: 0, },
    { low: 0, mid: 0, high: 0, }
];
PioneerDDJREV1.MixxxedModeLockHoldMode = [1, 1, 1, 1];
PioneerDDJREV1.autoloopLedConnections = [];
PioneerDDJREV1.autoloopLedCache = [
    new Array(8).fill(-1), new Array(8).fill(-1),
    new Array(8).fill(-1), new Array(8).fill(-1)
];
PioneerDDJREV1.autoloopShiftLedCache = [
    new Array(8).fill(-1), new Array(8).fill(-1),
    new Array(8).fill(-1), new Array(8).fill(-1)
];
PioneerDDJREV1.lifecycleConnections = [];

PioneerDDJREV1.slicerBeatConnections = [];
PioneerDDJREV1.slicerLedState = [
    new Array(8).fill(-1), new Array(8).fill(-1),
    new Array(8).fill(-1), new Array(8).fill(-1)
];
PioneerDDJREV1.slicerShiftLedState = [
    new Array(8).fill(-1), new Array(8).fill(-1),
    new Array(8).fill(-1), new Array(8).fill(-1)
];
PioneerDDJREV1.slicerDomains = [8, 16, 32, 64];
PioneerDDJREV1.selectedSlicerDomain = [8, 8, 8, 8];
PioneerDDJREV1.slicerActive = [false, false, false, false];
PioneerDDJREV1.slicerAlreadyJumped = [false, false, false, false];
PioneerDDJREV1.slicerButton = [0, 0, 0, 0];
PioneerDDJREV1.slicerModes = { contSlice: 0, loopSlice: 1, };
PioneerDDJREV1.activeSlicerMode = [0, 0, 0, 0];
PioneerDDJREV1.slicerLoopVisual = true;
PioneerDDJREV1.slicerPreviousRelBeats = [0, 0, 0, 0];
PioneerDDJREV1.slicerReleaseStopDebounceMs = 220;
PioneerDDJREV1.slicerHoldStopTimer = [null, null, null, null];
PioneerDDJREV1.slicerReleaseStopTimer = [null, null, null, null];
PioneerDDJREV1.slicerPendingLoopActivate = [null, null, null, null];
PioneerDDJREV1.slicerHeldSliceEndBeat = [0, 0, 0, 0];
PioneerDDJREV1.slicerHeldByUser = [false, false, false, false];
PioneerDDJREV1.slicerStartedStoppedDeck = [false, false, false, false];
PioneerDDJREV1.slicerLastPadKey = [-1, -1, -1, -1];
PioneerDDJREV1.slicerLastPadSection = [0, 0, 0, 0];
PioneerDDJREV1.slicerLastPadTargetBeat = [0, 0, 0, 0];
PioneerDDJREV1.slicerLoopStartBeat = [0, 0, 0, 0];
PioneerDDJREV1.slicerLoopStartSamples = [-1, -1, -1, -1];
PioneerDDJREV1.slicerLoopEndSamples = [-1, -1, -1, -1];
PioneerDDJREV1.slicerLoopDomain = [8, 8, 8, 8];
PioneerDDJREV1.slicerPreviousPlaySample = [0, 0, 0, 0];
PioneerDDJREV1.slicerLastPadTargetSample = [0, 0, 0, 0];
PioneerDDJREV1.slicerLastPadSliceEndSample = [0, 0, 0, 0];
PioneerDDJREV1.slicerLastKnownPlayposition = [-1, -1, -1, -1];
PioneerDDJREV1.slicerSuppressSeekInvalidate = [false, false, false, false];
PioneerDDJREV1.slicerPlaypositionConnections = [];

PioneerDDJREV1.Components = {};
PioneerDDJREV1.Components._resolveCache = {};

PioneerDDJREV1.Components.resolve = function(componentKey) {
    if (!componentKey) {
        return null;
    }
    const cached = PioneerDDJREV1.Components._resolveCache[componentKey];
    if (cached !== undefined) {
        return cached;
    }
    let result = null;
    if (PioneerDDJREV1.componentContainer && PioneerDDJREV1.componentContainer[componentKey]) {
        result = PioneerDDJREV1.componentContainer[componentKey];
    } else if (PioneerDDJREV1.Components[componentKey]) {
        result = PioneerDDJREV1.Components[componentKey];
    } else {
        const pascalKey = componentKey.charAt(0).toUpperCase() + componentKey.slice(1);
        result = PioneerDDJREV1.Components[pascalKey] || null;
    }
    PioneerDDJREV1.Components._resolveCache[componentKey] = result;
    return result;
};

PioneerDDJREV1.Components.invalidateResolveCache = function() {
    PioneerDDJREV1.Components._resolveCache = {};
};

PioneerDDJREV1.Components.invoke = function(componentKey, methodName, argsArray) {
    const owner = PioneerDDJREV1.Components.resolve(componentKey);
    if (!owner || typeof owner[methodName] !== "function") {
        return undefined;
    }
    const args = argsArray || [];
    switch (args.length) {
    case 0: return owner[methodName]();
    case 1: return owner[methodName](args[0]);
    case 2: return owner[methodName](args[0], args[1]);
    case 3: return owner[methodName](args[0], args[1], args[2]);
    case 4: return owner[methodName](args[0], args[1], args[2], args[3]);
    case 5: return owner[methodName](args[0], args[1], args[2], args[3], args[4]);
    default: return owner[methodName].apply(owner, args);
    }
};

PioneerDDJREV1.ComponentJSTransport = {
    enabled: false,
    decks: [],
    hotcuePressStartedPlaying: {},
    hotcueHoldStates: {},
    activateStatusToDeck: {
        0x97: 1,
        0x99: 2,
        0x9B: 3,
        0x9D: 4,
    },
    clearStatusToDeck: {
        0x98: 1,
        0x9A: 2,
        0x9C: 3,
        0x9E: 4,
    },
    isAvailable: function() {
        return typeof components !== "undefined" &&
            typeof components.PlayButton === "function" &&
            typeof components.CueButton === "function" &&
            typeof components.SyncButton === "function" &&
            typeof components.HotcueButton === "function";
    },
    deckGroup: function(deckNum) {
        return "[Channel" + deckNum + "]";
    },
    deckStatus: function(deckNum) {
        return 0x90 + (deckNum - 1);
    },
    activateHotcueStatus: function(deckNum) {
        return 0x97 + (deckNum - 1) * 2;
    },
    hotcueLedConnections: [],
    hotcueLedCallback: function(value, group, control) {
        const match = control.match(/^hotcue_(\d+)_status$/);
        if (!match) {
            return;
        }
        const hotcueNum = parseInt(match[1], 10);
        let deckNum = script.deckFromGroup(group);
        if (!deckNum || hotcueNum < 1 || hotcueNum > 8) {
            return;
        }
        const midiStatus = PioneerDDJREV1.ComponentJSTransport.activateHotcueStatus(deckNum);
        const midiNote = hotcueNum - 1;
        midi.sendShortMsg(midiStatus, midiNote, value > 0 ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
        if (deckNum === 1 && PioneerDDJREV1.pianoRollEngaged.some(Boolean)) {
            PioneerDDJREV1.pianoRollRefreshAnchorShiftLeds();
        }
    },
    refreshHotcueLeds: function(deckNum) {
        let group = "[Channel" + deckNum + "]";
        const midiStatus = this.activateHotcueStatus(deckNum);
        for (let i = 1; i <= 8; i++) {
            const status = engine.getValue(group, "hotcue_" + i + "_status");
            midi.sendShortMsg(midiStatus, i - 1, status > 0 ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
        }
        if (deckNum === 1 && PioneerDDJREV1.pianoRollEngaged.some(Boolean)) {
            PioneerDDJREV1.pianoRollRefreshAnchorShiftLeds();
        }
    },
    refreshAllHotcueLeds: function() {
        for (let d = 1; d <= 4; d++) {
            this.refreshHotcueLeds(d);
        }
    },
    registerHotcueLedConnections: function() {
        this.unregisterHotcueLedConnections();
        for (let deckNum = 1; deckNum <= 4; deckNum++) {
            let group = "[Channel" + deckNum + "]";
            for (let i = 1; i <= 8; i++) {
                const conn = engine.makeConnection(group, "hotcue_" + i + "_status",
                    PioneerDDJREV1.ComponentJSTransport.hotcueLedCallback);
                if (conn) {
                    this.hotcueLedConnections.push(conn);
                }
            }
        }
        this.refreshAllHotcueLeds();
    },
    unregisterHotcueLedConnections: function() {
        for (let i = 0; i < this.hotcueLedConnections.length; i++) {
            if (this.hotcueLedConnections[i] && typeof this.hotcueLedConnections[i].disconnect === "function") {
                this.hotcueLedConnections[i].disconnect();
            }
        }
        this.hotcueLedConnections = [];
    },
    makeDeckComponents: function(deckNum) {
        const group = this.deckGroup(deckNum);
        const status = this.deckStatus(deckNum);
        const deckComponents = {
            group: group,
            playButton: new components.PlayButton({
                midi: [status, 0x0B],
                group: group,
            }),
            cueButton: new components.CueButton({
                midi: [status, 0x0C],
                group: group,
            }),
            // Keep sync fully script-owned to avoid ComponentJS one-shot
            // LED/timer behavior fighting manual sync pulse output.
            syncButton: null,
            hotcueButtons: {},
        };

        // Preserve existing transport semantics while adopting ComponentJS controls.
        deckComponents.playButton.input = function(_channel, _control, value, _status, inputGroup) {
            PioneerDDJREV1.Components.Transport.playPressed(value, inputGroup || group);
        };
        const hotcueStatus = this.activateHotcueStatus(deckNum);
        for (let i = 1; i <= 8; i++) {
            const control = 0x00 + (i - 1);
            deckComponents.hotcueButtons[i] = new components.HotcueButton({
                number: i,
                group: group,
                midi: [hotcueStatus, control],
            });
        }
        return deckComponents;
    },
    initialize: function() {
        this.shutdown();
        if (!this.isAvailable()) {
            this.enabled = false;
            return;
        }
        this.hotcuePressStartedPlaying = {};
        this.hotcueHoldStates = {};
        this.decks = [];
        for (let deckNum = 1; deckNum <= 4; deckNum++) {
            this.decks[deckNum] = this.makeDeckComponents(deckNum);
        }
        this.enabled = true;
    },
    shutdown: function() {
        if (this.decks && this.decks.length) {
            for (let deckNum = 1; deckNum <= 4; deckNum++) {
                const deck = this.decks[deckNum];
                if (!deck) {
                    continue;
                }
                const controls = [deck.playButton, deck.cueButton, deck.syncButton];
                for (let i = 1; i <= 8; i++) {
                    controls.push(deck.hotcueButtons && deck.hotcueButtons[i]);
                }
                controls.forEach(function(control) {
                    if (control && typeof control.disconnect === "function") {
                        control.disconnect();
                    }
                });
            }
        }
        this.hotcuePressStartedPlaying = {};
        this.hotcueHoldStates = {};
        this.decks = [];
        this.enabled = false;
    },
    holdStateKey: function(deckGroup, hotcueIndex) {
        return deckGroup + ":hotcue_" + hotcueIndex;
    },
    hasActiveHold: function(deckGroup) {
        const prefix = deckGroup + ":hotcue_";
        return Object.keys(this.hotcueHoldStates).some((key) => {
            const state = this.hotcueHoldStates[key];
            return key.indexOf(prefix) === 0 && state && !!state.active;
        });
    },
    recomputeDeckHotcueHoldActive: function(deckGroup) {
        PioneerDDJREV1.hotcueHoldActive[deckGroup] = this.hasActiveHold(deckGroup);
        if (!PioneerDDJREV1.hotcueHoldActive[deckGroup]) {
            PioneerDDJREV1.hotcuePreviewFromBrake[deckGroup] = false;
        }
    },
    notePlayTapDuringHotcueHold: function(deckGroup) {
        const prefix = deckGroup + ":hotcue_";
        Object.keys(this.hotcueHoldStates).forEach((key) => {
            const state = this.hotcueHoldStates[key];
            if (key.indexOf(prefix) === 0 && state && state.active) {
                state.playTappedDuringHold = true;
            }
        });
    },
    StopOnHotcueRelease: function(holdState) {
        if (!holdState) {
            return false;
        }
        // Keep-playing is only intentional when play was explicitly tapped during hold.
        return !holdState.playTappedDuringHold;
    },
    resolveDeck: function(status, group) {
        const groupMatch = group && group.match(/\[Channel([1-4])\]/);
        if (groupMatch) {
            return parseInt(groupMatch[1], 10);
        }
        if (this.activateStatusToDeck[status]) {
            return this.activateStatusToDeck[status];
        }
        if (this.clearStatusToDeck[status]) {
            return this.clearStatusToDeck[status];
        }
        return null;
    },
    playInput: function(channel, control, value, status, group) {
        const deckNum = this.resolveDeck(status, group);
        const resolvedGroup = group || this.deckGroup(deckNum || (channel + 1));
        const deck = deckNum && this.decks[deckNum];
        const pianoPlayLatch = value > 0 && resolvedGroup === "[Channel1]"
            && PioneerDDJREV1.pianoRollEngaged.some(Boolean)
            && PioneerDDJREV1.pianoRollScaleUsesAnchor(PioneerDDJREV1.pianoRollScale)
            && PioneerDDJREV1.pianoRollHeldCount > 0;
        if (pianoPlayLatch) {
            let alreadyPlaying = false;
            try {
                alreadyPlaying = engine.getValue(resolvedGroup, "play") > 0;
            } catch (e) {
                // ignore
            }
            if (alreadyPlaying && !PioneerDDJREV1.pianoRollLatchOnNextPlay) {
                PioneerDDJREV1.pianoRollLatchedPlay = false;
            } else {
                PioneerDDJREV1.pianoRollLatchOnNextPlay = false;
                PioneerDDJREV1.pianoRollLatchedPlay = true;
                PioneerDDJREV1.pianoRollArmedForPlay = false;
                try {
                    engine.setValue(resolvedGroup, "play", 1);
                } catch (e) {
                    // ignore
                }
                return;
            }
        }
        if (this.enabled && deck && deck.playButton && typeof deck.playButton.input === "function") {
            deck.playButton.input(channel, control, value, status, resolvedGroup);
        } else {
            PioneerDDJREV1.Components.Transport.playPressed(value, resolvedGroup);
        }
        if (value > 0 && resolvedGroup === "[Channel1]" && PioneerDDJREV1.pianoRollEngaged.some(Boolean)
            && PioneerDDJREV1.pianoRollScaleUsesAnchor(PioneerDDJREV1.pianoRollScale)) {
            PioneerDDJREV1.pianoRollArmedForPlay = true;
        }
    },
    cueInput: function(channel, control, value, status, group) {
        const deckNum = this.resolveDeck(status, group);
        const resolvedGroup = group || this.deckGroup(deckNum || (channel + 1));
        const deck = script.deckFromGroup(resolvedGroup);
        if (value > 0 && PioneerDDJREV1.brakingActive[resolvedGroup]) {
            PioneerDDJREV1.Components.invoke("transport", "cancelBrakeForceStop", [resolvedGroup]);
            if (deck) {
                engine.brake(deck, false);
                engine.softStart(deck, false);
            }
            // Force CUE preview semantics by entering stopped state first.
            engine.setValue(resolvedGroup, "play", 0);
        }
        const componentDeck = deckNum && this.decks[deckNum];
        if (this.enabled && componentDeck &&
                componentDeck.cueButton && typeof componentDeck.cueButton.input === "function") {
            componentDeck.cueButton.input(channel, control, value, status, resolvedGroup);
            return;
        }
        engine.setValue(resolvedGroup, "cue_default", value);
    },
    syncInput: function(channel, control, value, status, group) {
        const deckNum = this.resolveDeck(status, group);
        const deck = deckNum && this.decks[deckNum];
        if (this.enabled && deck && deck.syncButton && typeof deck.syncButton.input === "function") {
            deck.syncButton.input(channel, control, value, status, group || deck.group);
            return;
        }
        PioneerDDJREV1.Components.Transport.syncPressed(
            value,
            group || this.deckGroup(deckNum || (channel + 1)),
            status
        );
    },
    hotcueInput: function(channel, control, value, status, group) {
        const deckNum = this.resolveDeck(status, group);
        const hotcueIndex = control + 1;
        if (hotcueIndex < 1 || hotcueIndex > 8) {
            return;
        }
        const deckGroup = group || this.deckGroup(deckNum || (channel + 1));
        const holdStateKey = this.holdStateKey(deckGroup, hotcueIndex);
        if (value > 0 && PioneerDDJREV1.brakingActive[deckGroup]) {
            // Clear both timer and brakingActive state to avoid stale brake gating.
            PioneerDDJREV1.Components.invoke("transport", "cancelActiveBrake", [deckGroup]);
            // Hotcue hold during brake to enter preview (stopped) state.
            engine.setValue(deckGroup, "play", 0);
            PioneerDDJREV1.hotcuePreviewFromBrake[deckGroup] = true;
        }
        if (this.clearStatusToDeck[status]) {
            if (value > 0) {
                engine.setValue(deckGroup, "hotcue_" + hotcueIndex + "_clear", 1);
            }
            // Clear-status events can arrive without a matching activate release path.
            // Proactively clear any hold bookkeeping for this pad to avoid stale hold gates.
            if (this.hotcueHoldStates[holdStateKey]) {
                this.hotcueHoldStates[holdStateKey].active = false;
                delete this.hotcueHoldStates[holdStateKey];
            }
            delete this.hotcuePressStartedPlaying[holdStateKey];
            this.recomputeDeckHotcueHoldActive(deckGroup);
            return;
        }
        const componentDeck = deckNum && this.decks[deckNum];
        const hotcueButton = componentDeck && componentDeck.hotcueButtons && componentDeck.hotcueButtons[hotcueIndex];
        const useComponentJS = this.enabled && hotcueButton && typeof hotcueButton.input === "function";
        if (value > 0) {
            const startedPlaying = engine.getValue(deckGroup, "play") > 0;
            this.hotcuePressStartedPlaying[holdStateKey] = startedPlaying;
            this.hotcueHoldStates[holdStateKey] = {
                startedPlaying: startedPlaying,
                playTappedDuringHold: false,
                brakeStateAtPress: !!PioneerDDJREV1.brakingActive[deckGroup],
                pressTimestamp: Date.now(),
                active: true,
            };
            this.recomputeDeckHotcueHoldActive(deckGroup);
            if (useComponentJS) {
                hotcueButton.input(channel, control, value, status, deckGroup);
            } else {
                engine.setValue(deckGroup, "hotcue_" + hotcueIndex + "_activate", 1);
            }
            return;
        }
        const holdState = this.hotcueHoldStates[holdStateKey] || {
            startedPlaying: !!this.hotcuePressStartedPlaying[holdStateKey],
            playTappedDuringHold: false,
            active: false,
        };
        delete this.hotcuePressStartedPlaying[holdStateKey];
        // Clear/deactivate hold first so downstream play decisions cannot read stale hold state.
        if (this.hotcueHoldStates[holdStateKey]) {
            this.hotcueHoldStates[holdStateKey].active = false;
            delete this.hotcueHoldStates[holdStateKey];
        }
        this.recomputeDeckHotcueHoldActive(deckGroup);
        const sstop = this.StopOnHotcueRelease(holdState);
        if (useComponentJS) {
            // Always forward release to ComponentJS so button state cannot remain latched.
            hotcueButton.input(channel, control, 0, status, deckGroup);
        } else if (sstop) {
            engine.setValue(deckGroup, "hotcue_" + hotcueIndex + "_activate", 0);
        }
        if (!sstop && holdState.playTappedDuringHold) {
            // Keep expected play-through behavior when hold release should not stop playback.
            engine.setValue(deckGroup, "play", 0);
        }
    },
};

PioneerDDJREV1.componentPlayButtonInput = {
    input: function(channel, control, value, status, group) {
        PioneerDDJREV1.ComponentJSTransport.playInput(channel, control, value, status, group);
    },
};

PioneerDDJREV1.componentCueButtonInput = {
    input: function(channel, control, value, status, group) {
        PioneerDDJREV1.ComponentJSTransport.cueInput(channel, control, value, status, group);
    },
};

PioneerDDJREV1.componentSyncButtonInput = {
    input: function(channel, control, value, status, group) {
        PioneerDDJREV1.ComponentJSTransport.syncInput(channel, control, value, status, group);
    },
};

PioneerDDJREV1.componentHotcueInput = {
    input: function(channel, control, value, status, group) {
        PioneerDDJREV1.pianoRollClearGhostHold();
        PioneerDDJREV1.ComponentJSTransport.hotcueInput(channel, control, value, status, group);
    },
};

PioneerDDJREV1.Components.Settings = {
    readString: function(settingName, defaultValue) {
        const raw = engine.getSetting(settingName);
        if (raw === undefined || raw === null || raw === "") {
            return defaultValue;
        }
        return String(raw).trim();
    },
    readBoolean: function(settingName, defaultValue) {
        const raw = engine.getSetting(settingName);
        if (raw === undefined || raw === null || raw === "") {
            return defaultValue;
        }
        if (typeof raw === "boolean") {
            return raw;
        }
        if (typeof raw === "number") {
            return raw > 0;
        }
        const normalized = String(raw).trim().toLowerCase();
        if (normalized === "1" || normalized === "true" || normalized === "yes" || normalized === "on") {
            return true;
        }
        if (normalized === "0" || normalized === "false" || normalized === "no" || normalized === "off") {
            return false;
        }
        return defaultValue;
    },
    normalizeDeckStartupMode: function(rawMode, fallbackVinyl) {
        const normalized = String(rawMode || "").trim().toLowerCase();
        if (normalized === "cdj" || normalized === "0" || normalized === "false" || normalized === "off" || normalized === "no") {
            return false;
        }
        if (normalized === "vinyl" || normalized === "1" || normalized === "true" || normalized === "on" || normalized === "yes") {
            return true;
        }
        return fallbackVinyl;
    },
    normalizeLibrarySortField: function(rawField, fallbackField) {
        const normalized = String(rawField || "").trim().toLowerCase();
        const allowed = new Set([
            "artist",
            "title",
            "album",
            "albumartist",
            "year",
            "genre",
            "composer",
            "grouping",
            "tracknumber",
            "filetype",
            "nativelocation",
            "comment",
            "duration",
            "bitrate",
            "bpm",
            "replaygain",
            "date",
            "timesplayed",
            "rating",
            "key",
            "preview",
            "coverart",
            "trackcolor",
            "lastplayed",
        ]);
        if (allowed.has(normalized)) {
            return normalized;
        }
        return fallbackField;
    },
    normalizeScratchResolution: function(rawValue, fallbackValue) {
        const parsed = Number(rawValue);
        // Backward compat: older UI used 900 (PLX). Normalize to 920.
        if (parsed === 900) {
            return 920;
        }
        if (parsed === 360 || parsed === 720 || parsed === 920 || parsed === 1536 || parsed === 2048) {
            return parsed;
        }
        return fallbackValue;
    },
    parseBeatLoopRollSetting: function(rawValue, fallbackValue) {
        const normalized = String(rawValue || "").trim().toLowerCase();
        if (normalized === "half") {
            return {
                mode: "relative",
                op: "half",
            };
        }
        if (normalized === "double") {
            return {
                mode: "relative",
                op: "double",
            };
        }
        const parsed = Number(rawValue);
        if (Number.isFinite(parsed) && parsed > 0) {
            return {
                mode: "fixed",
                value: parsed,
            };
        }
        return {
            mode: "fixed",
            value: fallbackValue,
        };
    },
    normalizeSamplePadLayout: function(rawValue, fallbackValue) {
        const normalized = String(rawValue || "").trim().toLowerCase();
        if (normalized === "standard" || normalized === "linear" || normalized === "0") {
            return "standard";
        }
        if (normalized === "banked_rows" || normalized === "bankedrows" || normalized === "banked" || normalized === "rows" || normalized === "1") {
            return "banked_rows";
        }
        // New: per-deck sampler banks 1–32 (8 per pad).
        if (normalized === "32" || normalized === "pad32" || normalized === "per_pad_32" || normalized === "per-pad-32") {
            return "deck32";
        }
        return fallbackValue;
    },
    normalizeWaveformZoomMode: function(rawValue, fallbackValue) {
        const normalized = String(rawValue || "").trim().toLowerCase();
        if (normalized === "vinyl" || normalized === "0") {
            return "vinyl";
        }
        if (normalized === "cdj" || normalized === "1") {
            return "cdj";
        }
        return fallbackValue;
    },
    parsePadSizeSetting: function(rawValue, fallbackValue) {
        const normalized = String(rawValue || "").trim().toLowerCase();
        if (normalized === "" || normalized === "default" || normalized === "inherit") {
            return { mode: "inherit", };
        }
        if (normalized === "half") {
            return { mode: "relative", op: "half", };
        }
        if (normalized === "double") {
            return { mode: "relative", op: "double", };
        }
        const parsed = Number(rawValue);
        if (Number.isFinite(parsed) && parsed > 0) {
            return { mode: "fixed", value: parsed, };
        }
        return this.parsePadSizeSetting(fallbackValue, "default");
    },
    /** @returns {{ kind: "default" }|{ kind: "legacy_size", size: object }|{ kind: "action", action: string, size: (null|number) }} */
    parseBeatJumpPadSetting: function(rawValue, padIndex, fallbackValue) {
        const raw = String(rawValue || "").trim();
        const normalized = raw.toLowerCase();
        if (normalized === "" || normalized === "default" || normalized === "inherit") {
            return { kind: "default", };
        }
        const allActions = [
            "prev", "back", "fwd", "reverseroll",
            "beatjump_backward", "beatjump_forward", "beatjump_size_halve", "beatjump_size_double",
        ];
        const sizedJump = normalized.match(/^(beatjump_backward|beatjump_forward):(.+)$/);
        if (sizedJump) {
            const action = sizedJump[1];
            const num = Number(String(sizedJump[2]).trim());
            if (Number.isFinite(num) && num > 0) {
                return { kind: "action", action: action, size: num, };
            }
            return this.parseBeatJumpPadSetting(fallbackValue, padIndex, "default");
        }
        if (allActions.indexOf(normalized) >= 0) {
            return { kind: "action", action: normalized, size: null, };
        }
        const legacy = this.parsePadSizeSetting(rawValue, fallbackValue);
        if (legacy.mode !== "inherit") {
            return { kind: "legacy_size", size: legacy, };
        }
        return { kind: "default", };
    },
    tempoRangesForProfile: function(rawValue) {
        const normalized = String(rawValue || "").trim().toLowerCase();
        if (normalized === "classic") {
            return [0.06, 0.10, 0.16, 0.25];
        }
        if (normalized === "alt_step_size" || normalized === "alt-step-size" || normalized === "alt") {
            return [0.08, 0.24, 0.50];
        }
        if (normalized === "extreme") {
            return [0.08, 0.16, 0.50, 1.00];
        }

        // default
        return [0.08, 0.16, 0.50];
    },
    applyUserOptions: function() {
        for (let i = 0; i < 4; i++) {
            const variableName = "startupModeDeck" + (i + 1);
            const fallbackMode = PioneerDDJREV1.vinylMode[i] ? "vinyl" : "cdj";
            const configuredMode = this.readString(variableName, fallbackMode);
            PioneerDDJREV1.vinylMode[i] = this.normalizeDeckStartupMode(configuredMode, PioneerDDJREV1.vinylMode[i]);

            const sortSettingName = "librarySortDeck" + (i + 1);
            const fallbackSortField = PioneerDDJREV1.librarySortDefaults[i] || "bpm";
            const configuredSortField = this.readString(sortSettingName, fallbackSortField);
            PioneerDDJREV1.librarySortDeckModes[i] = this.normalizeLibrarySortField(configuredSortField, fallbackSortField);
        }
        PioneerDDJREV1.VinylSlipAutoff = this.readBoolean("vinylSlipAutoff", PioneerDDJREV1.VinylSlipAutoff);
        PioneerDDJREV1.nonShiftScratchResolution = this.normalizeScratchResolution(
            this.readString("nonShiftScratchFeel", String(PioneerDDJREV1.nonShiftScratchResolution)),
            720
        );
        PioneerDDJREV1.tempSamplerSkin = this.readBoolean("tempSamplerSkin", PioneerDDJREV1.tempSamplerSkin);
        PioneerDDJREV1.samplePadLayout = this.normalizeSamplePadLayout(
            this.readString("samplePadLayout", PioneerDDJREV1.samplePadLayout),
            PioneerDDJREV1.samplePadLayout
        );
        PioneerDDJREV1.waveformZoomEnabled = this.readBoolean("waveformZoomEnabled", PioneerDDJREV1.waveformZoomEnabled);
        PioneerDDJREV1.waveformZoomMode = this.normalizeWaveformZoomMode(
            this.readString("waveformZoomMode", PioneerDDJREV1.waveformZoomMode),
            PioneerDDJREV1.waveformZoomMode
        );
        PioneerDDJREV1.vuMeterMode = String(this.readString("vuMeterMode", PioneerDDJREV1.vuMeterMode)).trim().toLowerCase();
        if (PioneerDDJREV1.vuMeterMode === "stereo_lr") {
            PioneerDDJREV1.vuMeterMode = "per_deck";
        }
        if (PioneerDDJREV1.vuMeterMode !== "per_deck" && PioneerDDJREV1.vuMeterMode !== "main_lr") {
            PioneerDDJREV1.vuMeterMode = "per_deck";
        }
        PioneerDDJREV1.bigLibraryShiftPush = this.readBoolean("bigLibraryShiftPush", PioneerDDJREV1.bigLibraryShiftPush);
        PioneerDDJREV1.pianoRollScale = String(this.readString("pianoRollScale", PioneerDDJREV1.pianoRollScale || "major")).trim().toLowerCase();
        const _validPianoScales = ["major", "minor", "playthrough"];
        if (!_validPianoScales.includes(PioneerDDJREV1.pianoRollScale)) {
            PioneerDDJREV1.pianoRollScale = "major";
        }
        PioneerDDJREV1.beatSlicerPattern = String(this.readString("beatSlicerPattern", PioneerDDJREV1.beatSlicerPattern || "linear")).trim().toLowerCase();
        const _validBeatSlicerPatterns = ["linear", "loop", "quantize_loop"];
        if (!_validBeatSlicerPatterns.includes(PioneerDDJREV1.beatSlicerPattern)) {
            PioneerDDJREV1.beatSlicerPattern = "linear";
        }
        PioneerDDJREV1.brakingEnabled = this.readBoolean("brakingEnabled", PioneerDDJREV1.brakingEnabled);
        PioneerDDJREV1.brakingReverse = this.readBoolean("brakingReverse", PioneerDDJREV1.brakingReverse);
        PioneerDDJREV1.splitFx = this.readBoolean("SplitFx", PioneerDDJREV1.splitFx);
        for (let i = 0; i < 8; i++) {
            const settingName = "beatLoopRollsSize" + (i + 1);
            const parsed = this.parseBeatLoopRollSetting(
                this.readString(settingName, String(PioneerDDJREV1.beatLoopRollDefaults[i])),
                PioneerDDJREV1.beatLoopRollDefaults[i]
            );
            PioneerDDJREV1.beatLoopRollConfigs[i] = parsed;
            const fixedValue = parsed.mode === "fixed" ? parsed.value : PioneerDDJREV1.beatLoopRollDefaults[i];
            PioneerDDJREV1.beatLoopRollSizes[i] = fixedValue;
            PioneerDDJREV1.beatLoopPadResolvedSizes[i] = fixedValue;
        }
        for (let i = 0; i < 8; i++) {
            const settingName = "beatJumpSize" + (i + 1);
            PioneerDDJREV1.beatJumpConfigs[i] = this.parseBeatJumpPadSetting(
                this.readString(settingName, PioneerDDJREV1.beatJumpDefaults[i]),
                i,
                PioneerDDJREV1.beatJumpDefaults[i]
            );
        }
        for (let i = 0; i < 8; i++) {
            const settingName = "autoLoopSize" + (i + 1);
            const parsed = this.parsePadSizeSetting(
                this.readString(settingName, String(PioneerDDJREV1.autoLoopDefaults[i])),
                String(PioneerDDJREV1.autoLoopDefaults[i])
            );
            // Autoloop does not support "inherit" because it must resolve to a numeric beatloop_size.
            PioneerDDJREV1.autoLoopConfigs[i] = parsed.mode === "inherit"
                ? { mode: "fixed", value: PioneerDDJREV1.autoLoopDefaults[i], }
                : parsed;
        }
        const brakingStartRaw = this.readString("brakingStartProfile", "0").toLowerCase();
        const brakingStopRaw = this.readString("brakingStopProfile", "0").toLowerCase();
        const normalizeBrakingProfile = function(rawValue) {
            if (rawValue === "1" || rawValue === "classic") {
                return "classic";
            }
            if (rawValue === "2" || rawValue === "slow") {
                return "slow";
            }
            return "off";
        };
        PioneerDDJREV1.brakingStartProfile = normalizeBrakingProfile(brakingStartRaw);
        PioneerDDJREV1.brakingStopProfile = normalizeBrakingProfile(brakingStopRaw);
        if (!PioneerDDJREV1.brakingEnabled) {
            PioneerDDJREV1.brakingStartProfile = "off";
            PioneerDDJREV1.brakingStopProfile = "off";
        }
        PioneerDDJREV1.mixxxedModeEnabled = this.readBoolean("mixxxedModeEnabled", PioneerDDJREV1.mixxxedModeEnabled);
        PioneerDDJREV1.disableStartFader = this.readBoolean("disableStartFader", PioneerDDJREV1.disableStartFader);
        PioneerDDJREV1.tempoRangeProfile = this.readString("tempoRangeProfile", PioneerDDJREV1.tempoRangeProfile);
        PioneerDDJREV1.tempoRanges = this.tempoRangesForProfile(PioneerDDJREV1.tempoRangeProfile);
    },
};

PioneerDDJREV1.Components.Capabilities = {
    isMeaningfulProbeValue: function(value) {
        if (value === undefined || value === null) {
            return false;
        }
        if (typeof value === "number" && Number.isFinite(value)) {
            return value !== 0;
        }
        return String(value).trim() !== "" && String(value).trim() !== "0";
    },
    parseMajorMinor: function(versionString) {
        const normalized = String(versionString || "").trim();
        const match = normalized.match(/(\d+)\.(\d+)/);
        if (!match) {
            return null;
        }
        return {
            major: parseInt(match[1], 10),
            minor: parseInt(match[2], 10),
        };
    },
    compareMajorMinor: function(lhs, rhs) {
        if (!lhs || !rhs) {
            return 0;
        }
        if (lhs.major !== rhs.major) {
            return lhs.major < rhs.major ? -1 : 1;
        }
        if (lhs.minor !== rhs.minor) {
            return lhs.minor < rhs.minor ? -1 : 1;
        }
        return 0;
    },
    isMixxx26OrNewer: function(versionString) {
        const parsed = this.parseMajorMinor(versionString);
        if (!parsed) {
            return false;
        }
        return this.compareMajorMinor(parsed, {major: 2, minor: 6,}) >= 0;
    },
    detectVersionString: function() {
        try {
            if (typeof engine.getProductVersion === "function") {
                return String(engine.getProductVersion() || "");
            }
        } catch (e) {
            // fall through to empty version string
        }
        try {
            if (typeof mixxxVersion !== "undefined") {
                return String(mixxxVersion || "");
            }
        } catch (e) {
            // fall through to empty version string
        }
        try {
            const appVersion = engine.getValue("[App]", "version");
            if (appVersion !== undefined && appVersion !== null && appVersion !== "" && appVersion !== 0 && appVersion !== 0.0) {
                return String(appVersion);
            }
        } catch (e) {
            // fall through to empty version string
        }
        return "";
    },
    probeStemsSupport: function() {
        try {
            const stemCount = engine.getValue("[Channel1]", "stem_count");
            const stemMute = engine.getValue("[Channel1_Stem1]", "mute");
            const stemVolume = engine.getValue("[Channel1_Stem1]", "volume");
            const stemEffectEnabled = engine.getValue("[QuickEffectRack1_[Channel1_Stem1]]", "enabled");
            const allDefined = stemCount !== undefined &&
                stemMute !== undefined &&
                stemVolume !== undefined &&
                stemEffectEnabled !== undefined;
            if (!allDefined) {
                return false;
            }
            // On 2.5 unknown controls may report 0.0; do not treat that as stems availability.
            return this.isMeaningfulProbeValue(stemCount) ||
                this.isMeaningfulProbeValue(stemMute) ||
                this.isMeaningfulProbeValue(stemVolume) ||
                this.isMeaningfulProbeValue(stemEffectEnabled);
        } catch (e) {
            return false;
        }
    },
    resolveOperatingPolicy: function(probedSupportsStems) {
        const v = PioneerDDJREV1.detectedMixxxVersion || "";
        const parsed = this.parseMajorMinor(v);
        let tier26 = false;
        if (parsed) {
            tier26 = this.compareMajorMinor(parsed, {major: 2, minor: 6,}) >= 0;
        } else {
            tier26 = probedSupportsStems;
        }
        if (!tier26) {
            return {
                supportsStems: false,
                scratchBankOnStemPads: true,
                samplerMode: "standard16",
                stemsPriority: false,
                runtimeTier: "25",
            };
        }
        return {
            supportsStems: probedSupportsStems,
            scratchBankOnStemPads: false,
            samplerMode: "standard16",
            stemsPriority: true,
            runtimeTier: "26",
        };
    },
    detectStems: function() {
        // Guard startup/race defaults until a single policy resolution is applied.
        PioneerDDJREV1.supportsStems = false;
        PioneerDDJREV1.scratchBankOnStemPads = false;
        PioneerDDJREV1.samplerMode = "standard16";
        PioneerDDJREV1.detectedMixxxVersion = this.detectVersionString();
        const probedSupportsStems = this.probeStemsSupport();
        const policy = this.resolveOperatingPolicy(probedSupportsStems);
        PioneerDDJREV1.resolvedOperatingPolicy = policy;
        PioneerDDJREV1.supportsStems = policy.supportsStems;
        PioneerDDJREV1.scratchBankOnStemPads = policy.scratchBankOnStemPads;
        PioneerDDJREV1.samplerMode = policy.samplerMode || "standard16";
    },
};

PioneerDDJREV1.Components.ModeGate = {
    isStemsPriorityVersion: function() {
        if (PioneerDDJREV1.resolvedOperatingPolicy &&
                PioneerDDJREV1.resolvedOperatingPolicy.stemsPriority !== undefined) {
            return !!PioneerDDJREV1.resolvedOperatingPolicy.stemsPriority;
        }
        return PioneerDDJREV1.Components.Capabilities.isMixxx26OrNewer(PioneerDDJREV1.detectedMixxxVersion);
    },
    enforceStemsPriority: function() {
        if (!PioneerDDJREV1.scratchBankOnStemPads) {
            return;
        }
        const stemsWin = this.isStemsPriorityVersion() && PioneerDDJREV1.supportsStems;
        if (!stemsWin) {
            return;
        }
        PioneerDDJREV1.scratchBankOnStemPads = false;
    },
};

PioneerDDJREV1.Components.Stems = {
    stemLoadRetryDelayMs: 100,
    stemLoadMaxRetries: 4,
    stemLedConnections: [],
    canToggleStem: function(group) {
        const idx = this.stemIdxForGroup(group);
        if (idx < 0) {
            return true;
        }
        const now = Date.now();
        const last = PioneerDDJREV1.stemToggleLastMs[idx] || 0;
        if (now - last < 50) {
            return false;
        }
        PioneerDDJREV1.stemToggleLastMs[idx] = now;
        return true;
    },
    getDeckFromGroup: function(group) {
        const channelMatch = group && group.match(/\[Channel([1-4])\]/);
        if (channelMatch) {
            return parseInt(channelMatch[1], 10);
        }
        const stemMatch = group && group.match(/Channel([1-4])_Stem[1-4]/);
        if (stemMatch) {
            return parseInt(stemMatch[1], 10);
        }
        return parseInt(script.deckFromGroup(group), 10);
    },
    deckStatusFromGroup: function(group) {
        const deck = this.getDeckFromGroup(group);
        const statusByDeck = {
            1: 0x97,
            2: 0x99,
            3: 0x9B,
            4: 0x9D,
        };
        return statusByDeck[deck] || 0x97;
    },
    parseStemAddress: function(group) {
        const stemMatch = group && group.match(/Channel([1-4])_Stem([1-4])/);
        if (!stemMatch) {
            return null;
        }
        const deck = parseInt(stemMatch[1], 10);
        const stemIndex = parseInt(stemMatch[2], 10);
        return {
            deck: deck,
            stemIndex: stemIndex,
            deckGroup: "[Channel" + deck + "]",
        };
    },
    stemIdxForGroup: function(group) {
        const addr = this.parseStemAddress(group);
        if (!addr) {
            return -1;
        }
        return (addr.deck - 1) * 4 + (addr.stemIndex - 1);
    },
    stemMuteNoteForIndex: function(stemIndex) {
        return 0x70 + (4 - stemIndex);
    },
    stemEffectNoteForIndex: function(stemIndex) {
        return 0x74 + (4 - stemIndex);
    },
    stemGroupForControl: function(group) {
        const deck = this.getDeckFromGroup(group);
        return "[Channel" + deck + "_Stem";
    },
    stemEffectGroupForControl: function(group) {
        const deck = this.getDeckFromGroup(group);
        return "[QuickEffectRack1_[Channel" + deck + "_Stem";
    },
    stemQuickEffectRackGroup: function(deck, stemIndex) {
        return "[QuickEffectRack1_[Channel" + deck + "_Stem" + stemIndex + "]]";
    },
    isStemQuickEffectRackAvailable: function(rackGroup) {
        try {
            const v = engine.getValue(rackGroup, "enabled");
            return v !== undefined && v !== null;
        } catch (e) {
            return false;
        }
    },
    quickRacksForHeldStemEffectPads: function() {
        const out = [];
        if (!PioneerDDJREV1.supportsStems) {
            return out;
        }
        for (let i = 0; i < 16; i++) {
            if (!PioneerDDJREV1.stemEffectHeld[i]) {
                continue;
            }
            const deck = Math.floor(i / 4) + 1;
            const stem = (i % 4) + 1;
            const deckGroup = "[Channel" + deck + "]";
            let stemCount = 4;
            try {
                const raw = engine.getValue(deckGroup, "stem_count");
                stemCount = Math.min(4, Math.max(0, Math.floor(Number(raw))));
            } catch (e) {
                continue;
            }
            if (stemCount < 1 || stem > stemCount) {
                continue;
            }
            const rack = this.stemQuickEffectRackGroup(deck, stem);
            if (this.isStemQuickEffectRackAvailable(rack)) {
                out.push(rack);
            }
        }
        return out;
    },
    applyStemQuickChainStep: function(rackGroups, forward) {
        for (let r = 0; r < rackGroups.length; r++) {
            const rg = rackGroups[r];
            let ok = false;
            try {
                if (forward) {
                    engine.setValue(rg, "next_chain_preset", 1);
                } else {
                    engine.setValue(rg, "prev_chain_preset", 1);
                }
                ok = true;
            } catch (e) {
                // continue fallbacks
            }
            if (ok) {
                continue;
            }
            try {
                engine.setValue(rg, "chain_preset_selector", forward ? 1 : -1);
                ok = true;
            } catch (e2) {
                // continue
            }
            if (ok) {
                continue;
            }
            try {
                engine.setValue(rg, "next_chain_preset", forward ? 1 : -1);
            } catch (e3) {
                // Rack may not expose chain controls on this Mixxx build.
            }
        }
    },
    hasStemControls: function(group) {
        if (!PioneerDDJREV1.supportsStems) {
            return false;
        }
        try {
            const stemGroup = this.stemGroupForControl(group) + "1]";
            const stemMute = engine.getValue(stemGroup, "mute");
            const stemVolume = engine.getValue(stemGroup, "volume");
            const stemEffectEnabled = engine.getValue(this.stemEffectGroupForControl(group) + "1]]", "enabled");
            return stemMute !== undefined &&
                stemVolume !== undefined &&
                stemEffectEnabled !== undefined;
        } catch (e) {
            return false;
        }
    },
    clearStemPadLeds: function(group) {
        const status = this.deckStatusFromGroup(group);
        for (let note = 0x70; note <= 0x77; note++) {
            midi.sendShortMsg(status, note, PioneerDDJREV1.LED_OFF);
        }
    },
    refreshDeckStemLeds: function(group) {
        if (!this.hasStemControls(group)) {
            return;
        }
        const activeStemCount = Math.min(4, Math.max(0, Math.floor(Number(engine.getValue(group, "stem_count")))));
        const stemMutePrefix = this.stemGroupForControl(group);
        const stemEffectPrefix = this.stemEffectGroupForControl(group);
        const status = this.deckStatusFromGroup(group);
        for (let i = 1; i <= 4; i++) {
            const muteNote = this.stemMuteNoteForIndex(i);
            const effectNote = this.stemEffectNoteForIndex(i);
            if (activeStemCount < i) {
                midi.sendShortMsg(status, muteNote, PioneerDDJREV1.LED_OFF);
                midi.sendShortMsg(status, effectNote, PioneerDDJREV1.LED_OFF);
                continue;
            }
            const stemMuteGroup = stemMutePrefix + i + "]";
            const stemMuted = Number(engine.getValue(stemMuteGroup, "mute")) > 0;
            midi.sendShortMsg(status, muteNote, stemMuted ? PioneerDDJREV1.LED_OFF : PioneerDDJREV1.LED_ON);
            const stemEffectGroup = stemEffectPrefix + i + "]]";
            const stemEffectOn = Number(engine.getValue(stemEffectGroup, "enabled")) > 0;
            midi.sendShortMsg(status, effectNote, stemEffectOn ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
        }
    },
    onStemMuteChanged: function(value, group) {
        const stemAddress = this.parseStemAddress(group);
        if (!stemAddress) {
            return;
        }
        const deckIdx = this.getDeckFromGroup(stemAddress.deckGroup) - 1;
        if (deckIdx >= 0 && PioneerDDJREV1.stemLoadInProgress[deckIdx]) {
            return;
        }
        const status = this.deckStatusFromGroup(stemAddress.deckGroup);
        const note = this.stemMuteNoteForIndex(stemAddress.stemIndex);
        midi.sendShortMsg(status, note, Number(value) > 0 ? PioneerDDJREV1.LED_OFF : PioneerDDJREV1.LED_ON);
    },
    onStemEffectChanged: function(value, group) {
        const stemAddress = this.parseStemAddress(group);
        if (!stemAddress) {
            return;
        }
        const deckIdx = this.getDeckFromGroup(stemAddress.deckGroup) - 1;
        if (deckIdx >= 0 && PioneerDDJREV1.stemLoadInProgress[deckIdx]) {
            return;
        }
        const status = this.deckStatusFromGroup(stemAddress.deckGroup);
        const note = this.stemEffectNoteForIndex(stemAddress.stemIndex);
        midi.sendShortMsg(status, note, Number(value) > 0 ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
    },
    onDeckTrackLoaded: function(value, group) {
        if (!PioneerDDJREV1.supportsStems) {
            return;
        }
        const deckIdx = this.getDeckFromGroup(group) - 1;
        if (deckIdx >= 0 && PioneerDDJREV1.stemSyncTimers[deckIdx] !== null) {
            engine.stopTimer(PioneerDDJREV1.stemSyncTimers[deckIdx]);
            PioneerDDJREV1.stemSyncTimers[deckIdx] = null;
        }
        if (Number(value) > 0) {
            PioneerDDJREV1.stemSyncTimers[deckIdx] = engine.beginTimer(120, function() {
                PioneerDDJREV1.stemSyncTimers[deckIdx] = null;
                PioneerDDJREV1.Components.invoke("stems", "syncStemLedsOnTrackLoad", [group]);
            }, true);
            return;
        }
        this.clearStemPadLeds(group);
    },
    registerStemLedConnections: function() {
        this.unregisterStemLedConnections();
        if (!PioneerDDJREV1.supportsStems) {
            return;
        }
        for (let deck = 1; deck <= 4; deck++) {
            const deckGroup = "[Channel" + deck + "]";
            if (!this.hasStemControls(deckGroup)) {
                continue;
            }
            // track_loaded is now handled by the unified trackLoadedLED callback
            const stemMutePrefix = this.stemGroupForControl(deckGroup);
            const stemEffectPrefix = this.stemEffectGroupForControl(deckGroup);
            for (let si = 1; si <= 4; si++) {
                const stemMuteGroup = stemMutePrefix + si + "]";
                const stemEffectGroup = stemEffectPrefix + si + "]]";
                const muteConn = engine.makeConnection(stemMuteGroup, "mute", function(value, group) {
                    PioneerDDJREV1.Components.invoke("stems", "onStemMuteChanged", [value, group]);
                });
                if (muteConn && typeof muteConn.disconnect === "function") {
                    this.stemLedConnections.push(muteConn);
                }
                const effConn = engine.makeConnection(stemEffectGroup, "enabled", function(value, group) {
                    PioneerDDJREV1.Components.invoke("stems", "onStemEffectChanged", [value, group]);
                });
                if (effConn && typeof effConn.disconnect === "function") {
                    this.stemLedConnections.push(effConn);
                }
            }
            this.syncStemLedsOnTrackLoad(deckGroup, 0);
        }
    },
    unregisterStemLedConnections: function() {
        PioneerDDJREV1.stemToggleLastMs.fill(0);
        if (!this.stemLedConnections || !this.stemLedConnections.length) {
            return;
        }
        for (let i = 0; i < this.stemLedConnections.length; i++) {
            const connection = this.stemLedConnections[i];
            if (connection && typeof connection.disconnect === "function") {
                connection.disconnect();
            }
        }
        this.stemLedConnections = [];
    },
    syncStemLedsOnTrackLoad: function(group, retryCount) {
        if (!PioneerDDJREV1.supportsStems) {
            return;
        }
        const attempt = retryCount || 0;
        const retryDeckIdx = this.getDeckFromGroup(group) - 1;
        if (!this.hasStemControls(group)) {
            if (attempt < this.stemLoadMaxRetries) {
                if (retryDeckIdx >= 0 && PioneerDDJREV1.stemSyncTimers[retryDeckIdx] !== null) {
                    engine.stopTimer(PioneerDDJREV1.stemSyncTimers[retryDeckIdx]);
                }
                PioneerDDJREV1.stemSyncTimers[retryDeckIdx] = engine.beginTimer(this.stemLoadRetryDelayMs, function() {
                    PioneerDDJREV1.stemSyncTimers[retryDeckIdx] = null;
                    PioneerDDJREV1.Components.invoke("stems", "syncStemLedsOnTrackLoad", [group, attempt + 1]);
                }, true);
            }
            return;
        }
        const rawStemCount = engine.getValue(group, "stem_count");
        const stemCount = Number(rawStemCount);
        if (!Number.isFinite(stemCount) || stemCount <= 0) {
            if (attempt < this.stemLoadMaxRetries) {
                if (retryDeckIdx >= 0 && PioneerDDJREV1.stemSyncTimers[retryDeckIdx] !== null) {
                    engine.stopTimer(PioneerDDJREV1.stemSyncTimers[retryDeckIdx]);
                }
                PioneerDDJREV1.stemSyncTimers[retryDeckIdx] = engine.beginTimer(this.stemLoadRetryDelayMs, function() {
                    PioneerDDJREV1.stemSyncTimers[retryDeckIdx] = null;
                    PioneerDDJREV1.Components.invoke("stems", "syncStemLedsOnTrackLoad", [group, attempt + 1]);
                }, true);
            }
            return;
        }
        const activeStemCount = Math.min(4, Math.max(0, Math.floor(stemCount)));
        const deckIdx = this.getDeckFromGroup(group) - 1;
        if (deckIdx >= 0) {
            PioneerDDJREV1.stemLoadInProgress[deckIdx] = true;
        }
        const stemMutePrefix = this.stemGroupForControl(group);
        for (let i = 1; i <= 4; i++) {
            const stemMuteGroup = stemMutePrefix + i + "]";
            engine.setValue(stemMuteGroup, "mute", activeStemCount >= i ? 0 : 1);
        }
        this.refreshDeckStemLeds(group);
        if (deckIdx >= 0) {
            PioneerDDJREV1.stemLoadInProgress[deckIdx] = false;
        }
    },
    levelDepth: function(value) {
        // Level/Depth for stem mute/effect adjustment while pads are held.
        if (PioneerDDJREV1.stemShiftPressed && PioneerDDJREV1.lastStemChannel) {
            const stemVolume = value / 127;
            engine.setValue(PioneerDDJREV1.lastStemChannel, "volume", stemVolume);
            const idx = this.stemIdxForGroup(PioneerDDJREV1.lastStemChannel);
            if (idx >= 0) {
                PioneerDDJREV1.stemPadMovedWhileHeld[idx] = true;
            }
            return;
        }
        if (PioneerDDJREV1.stemEffectPressed && PioneerDDJREV1.lastStemChannel) {
            const effectAmount = value / 127;
            engine.setValue(PioneerDDJREV1.lastStemChannel, "super1", effectAmount);
            const idx = this.stemIdxForGroup(PioneerDDJREV1.lastStemChannel);
            if (idx >= 0) {
                PioneerDDJREV1.stemPadMovedWhileHeld[idx] = true;
            }
            return;
        }
        // Level/Depth for sampler volume follows the active sampler layout.
        if (PioneerDDJREV1.sampleShiftPressed) {
            if (PioneerDDJREV1.tempSamplerSkin) {
                engine.setParameter("[Skin]", "show_samplers", true);
            }
            const sampleGain = value / 127;
            if (PioneerDDJREV1._cachedMaxSampler === undefined) {
                const mode = PioneerDDJREV1.samplerMode === "dual8_exception" ? "dual8_exception" : "standard16";
                const configuredCount = Number(engine.getValue("[App]", "num_samplers"));
                const availableCount = (!Number.isFinite(configuredCount) || configuredCount <= 0)
                    ? 16
                    : Math.max(1, Math.floor(configuredCount));
                PioneerDDJREV1._cachedMaxSampler = mode === "dual8_exception" ? 8 : Math.min(16, availableCount);
            }
            const maxSampler = PioneerDDJREV1._cachedMaxSampler;
            const groups = PioneerDDJREV1._samplerGroups;
            for (let i = 0; i < maxSampler; ++i) {
                engine.setValue(groups[i], "pregain", sampleGain);
            }
            return;
        }
        // Regular Level/Depth FX1/FX2
        if (!PioneerDDJREV1.stemShiftPressed && !PioneerDDJREV1.stemEffectPressed && !PioneerDDJREV1.sampleShiftPressed) {
            const volumeBit = value / 127;
            if (PioneerDDJREV1.splitFx) {
                if (PioneerDDJREV1.shiftPressed) {
                    engine.setValue("[EffectRack1_EffectUnit2]", "super1", volumeBit);
                } else {
                    engine.setValue("[EffectRack1_EffectUnit1]", "super1", volumeBit);
                }
            } else {
                engine.setValue("[EffectRack1_EffectUnit1]", "super1", volumeBit);
                engine.setValue("[EffectRack1_EffectUnit2]", "super1", volumeBit);
            }
        }
    },
    stemEffect: function(value, group) {
        if (!PioneerDDJREV1.supportsStems) {
            return;
        }
        PioneerDDJREV1.Components.ModeGate.enforceStemsPriority();
        PioneerDDJREV1.lastStemChannel = group;
        const idx = this.stemIdxForGroup(group);
        if (idx >= 0) {
            PioneerDDJREV1.stemEffectHeld[idx] = value > 0;
        }
        PioneerDDJREV1.stemEffectPressed = PioneerDDJREV1.stemEffectHeld.some(function(held) { return held; });
        if (value > 0) {
            if (idx >= 0) {
                PioneerDDJREV1.stemPadMovedWhileHeld[idx] = false;
            }
            return;
        }
        const movedWhileHeld = idx >= 0 ? PioneerDDJREV1.stemPadMovedWhileHeld[idx] : false;
        if (!movedWhileHeld && this.canToggleStem(group)) {
            currentStemEffect = engine.getValue(group, "enabled");
            const nextEffectVal = currentStemEffect === 0 ? 1 : 0;
            engine.setValue(group, "enabled", nextEffectVal);
            const stemAddr = this.parseStemAddress(group);
            if (stemAddr) {
                midi.sendShortMsg(
                    this.deckStatusFromGroup(stemAddr.deckGroup),
                    this.stemEffectNoteForIndex(stemAddr.stemIndex),
                    nextEffectVal > 0 ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF
                );
            }
        }
        if (idx >= 0) {
            PioneerDDJREV1.stemPadMovedWhileHeld[idx] = false;
        }
    },
    stemShift: function(channel, control, value, status, group) {
        if (value > 0 && control >= 0x70 && control <= 0x73 && PioneerDDJREV1.scratchBankOnStemPads) {
            PioneerDDJREV1.Components.invoke("scratchBank", "loadScratchToDeck", [channel, control, value, status]);
            return;
        }
        if (!PioneerDDJREV1.supportsStems) {
            return;
        }
        PioneerDDJREV1.Components.ModeGate.enforceStemsPriority();
        PioneerDDJREV1.lastStemChannel = group;
        const idx = this.stemIdxForGroup(group);
        if (idx >= 0) {
            PioneerDDJREV1.stemShiftHeld[idx] = value > 0;
        }
        PioneerDDJREV1.stemShiftPressed = PioneerDDJREV1.stemShiftHeld.some(function(held) { return held; });
        if (value > 0) {
            if (idx >= 0) {
                PioneerDDJREV1.stemPadMovedWhileHeld[idx] = false;
            }
            return;
        }
        const movedWhileHeld = idx >= 0 ? PioneerDDJREV1.stemPadMovedWhileHeld[idx] : false;
        if (!movedWhileHeld && this.canToggleStem(group)) {
            currentStemMute = engine.getValue(group, "mute");
            const nextMuteVal = currentStemMute === 0 ? 1 : 0;
            engine.setValue(group, "mute", nextMuteVal);
            const stemAddr = this.parseStemAddress(group);
            if (stemAddr) {
                midi.sendShortMsg(
                    this.deckStatusFromGroup(stemAddr.deckGroup),
                    this.stemMuteNoteForIndex(stemAddr.stemIndex),
                    nextMuteVal > 0 ? PioneerDDJREV1.LED_OFF : PioneerDDJREV1.LED_ON
                );
            }
        }
        if (idx >= 0) {
            PioneerDDJREV1.stemPadMovedWhileHeld[idx] = false;
        }
    },
    loadSelectedTrack: function(value, group) {
        engine.setValue(group, "LoadSelectedTrack", value);
    },
};

PioneerDDJREV1.Components.Headphones = {
    anyPflActive: function() {
        for (let i = 1; i <= 4; i++) {
            if (engine.getValue("[Channel" + i + "]", "pfl")) {
                return true;
            }
        }
        return false;
    },
    toggleChannelCue: function(group, status, control) {
        const nextPfl = engine.getValue(group, "pfl") ? 0 : 1;
        engine.setValue(group, "pfl", nextPfl);
        midi.sendShortMsg(status, control, nextPfl ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
    },
    applyPflAdjustmentsHeadphoneCue: function(isChannelCueButton, isMasterCueButton, group, status, control) {
        const isAnyPflActive = this.anyPflActive();
        const currentHeadMix = engine.getParameter("[Master]", "headMix");

        if (isChannelCueButton) {
            this.toggleChannelCue(group, status, control);
            if (currentHeadMix > 0.0 && !isAnyPflActive) {
                engine.setParameter("[Master]", "headMix", 0.5);
            } else if (currentHeadMix > 0.49) {
                engine.setParameter("[Master]", "headMix", 1.0);
            }
            return;
        }
        if (isMasterCueButton) {
            if (isAnyPflActive) {
                engine.setParameter("[Master]", "headMix", currentHeadMix > 0.49 ? 0.0 : 0.5);
            } else {
                engine.setParameter("[Master]", "headMix", currentHeadMix > 0.49 ? 0.0 : 1.0);
            }
            midi.sendShortMsg(status, control, engine.getParameter("[Master]", "headMix") > 0.15 ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
        }
    },
    cueing: function(control, value, status, group) {
        const isChannelCueButton = control === 0x54;
        const isMasterCueButton = control === 0x63;
        if (!value) {
            return;
        }
        this.applyPflAdjustmentsHeadphoneCue(isChannelCueButton, isMasterCueButton, group, status, control);
    },
};

PioneerDDJREV1.Components.BeatPads = {
    resolvePadLoopRollSize: function(padIndex, value, group) {
        const config = PioneerDDJREV1.beatLoopRollConfigs[padIndex] || {
            mode: "fixed",
            value: PioneerDDJREV1.beatLoopRollDefaults[padIndex],
        };
        if (config.mode === "relative" && value) {
            const currentSize = Number(engine.getValue(group, "beatloop_size"));
            const fallbackSize = PioneerDDJREV1.beatLoopRollDefaults[padIndex];
            const baseSize = Number.isFinite(currentSize) && currentSize > 0 ? currentSize : fallbackSize;
            let nextSize = config.op === "half" ? (baseSize / 2) : (baseSize * 2);
            // Keep sizes in a sensible range for roll controls.
            nextSize = Math.max(1 / 32, Math.min(64, nextSize));
            engine.setValue(group, "beatloop_size", nextSize);
            return nextSize;
        }
        if (config.mode === "fixed") {
            return config.value;
        }
        return PioneerDDJREV1.beatLoopRollDefaults[padIndex];
    },
    loadPreviousTrackWithFallback: function(group) {
        // hardware workaround: send eject twice.
        engine.setValue(group, "eject", 1);
        engine.setValue(group, "eject", 1);
        engine.beginTimer(40, function() {
            engine.setValue(group, "eject", 1);
            engine.setValue(group, "eject", 0);
        }, true);
    },
    resolveBeatJumpAction: function(padIndex, cfg) {
        const c = cfg || { kind: "default", };
        if (c.kind === "action") {
            return c.action;
        }
        return PioneerDDJREV1.beatJumpActions[padIndex];
    },
    applyBeatJumpConfiguredSize: function(group, cfg) {
        const c = cfg || { kind: "default", };
        if (c.kind === "action" && typeof c.size === "number" && Number.isFinite(c.size) && c.size > 0) {
            const nextSize = Math.max(1 / 32, Math.min(64, c.size));
            engine.setValue(group, "beatjump_size", nextSize);
            return true;
        }
        if (c.kind === "legacy_size" && c.size && c.size.mode !== "inherit") {
            const sz = c.size;
            const currentSize = Number(engine.getValue(group, "beatjump_size"));
            const baseSize = Number.isFinite(currentSize) && currentSize > 0 ? currentSize : 1;
            let nextSize = baseSize;
            if (sz.mode === "relative") {
                nextSize = sz.op === "half" ? (baseSize / 2) : (baseSize * 2);
            } else if (sz.mode === "fixed") {
                nextSize = sz.value;
            }
            nextSize = Math.max(1 / 32, Math.min(64, nextSize));
            engine.setValue(group, "beatjump_size", nextSize);
            return true;
        }
        return false;
    },
    pulseBeatJumpControl: function(group, actionName) {
        engine.setValue(group, actionName, 1);
        engine.setValue(group, actionName, 0);
    },
    handleBeatLoopRoll: function(control, value, status, group) {
        const pressedBeatLoopPad = control - 0x50;
        if (pressedBeatLoopPad < 0 || pressedBeatLoopPad > 7) {
            return;
        }
        const pressedBeatLoopRollSize = this.resolvePadLoopRollSize(pressedBeatLoopPad, value, group);
        midi.sendShortMsg(status, control, value ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
        if (value) {
            PioneerDDJREV1.beatLoopPadStates[pressedBeatLoopPad] = true;
            PioneerDDJREV1.beatLoopPadResolvedSizes[pressedBeatLoopPad] = pressedBeatLoopRollSize;
        } else {
            PioneerDDJREV1.beatLoopPadStates[pressedBeatLoopPad] = false;
        }
        if (value && !engine.getValue(group, "beatlooproll_activate")) {
            engine.setParameter(group, "beatloop_size", pressedBeatLoopRollSize);
            engine.setValue(group, "beatlooproll_activate", true);
        } else if (value) {
            engine.setValue(group, "beatloop_size", pressedBeatLoopRollSize);
        } else {
            const stillPressedIndex = PioneerDDJREV1.beatLoopPadStates.findIndex(val => val === true);
            if (stillPressedIndex >= 0) {
                engine.setValue(group, "beatloop_size", PioneerDDJREV1.beatLoopPadResolvedSizes[stillPressedIndex]);
            } else {
                engine.setValue(group, "beatlooproll_activate", false);
            }
        }
    },
    handleBeatJump: function(control, value, status, group) {
        const pressedBeatJumpPad = control - 0x40;
        if (pressedBeatJumpPad < 0 || pressedBeatJumpPad > 7) {
            return;
        }
        const ledValue = value ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF;
        midi.sendShortMsg(status, control, ledValue);

        const cfg = PioneerDDJREV1.beatJumpConfigs[pressedBeatJumpPad] || { kind: "default", };
        const resolvedAction = this.resolveBeatJumpAction(pressedBeatJumpPad, cfg);
        const holdTypes = { back: true, fwd: true, reverseroll: true, };

        // Hold-type actions on any pad: real press/hold/release (not one-shot pulse).
        if (holdTypes[resolvedAction]) {
            if (value && (resolvedAction === "back" || resolvedAction === "fwd")) {
                this.applyBeatJumpConfiguredSize(group, cfg);
            }
            engine.setValue(group, resolvedAction, value ? 1 : 0);
            return;
        }

        // One-shot actions: ignore release.
        if (!value) {
            return;
        }
        this.applyBeatJumpConfiguredSize(group, cfg);
        if (resolvedAction === "prev") {
            this.loadPreviousTrackWithFallback(group);
            return;
        }
        const isSizeAdjustPad = resolvedAction === "beatjump_size_halve" || resolvedAction === "beatjump_size_double";
        const legacySz = cfg.kind === "legacy_size" ? cfg.size : null;
        const suppressEngineAction = isSizeAdjustPad &&
            (legacySz && legacySz.mode !== "inherit" ||
                cfg.kind === "action" && cfg.size !== null && cfg.size !== undefined);
        if (!suppressEngineAction) {
            this.pulseBeatJumpControl(group, resolvedAction);
        }
    },
};

PioneerDDJREV1.Components.Transport = {
    brakeProfiles: {
        off: {startFactor: 0, stopFactor: 0,},
        classic: {startFactor: 1.0, stopFactor: 1.0,},
        slow: {startFactor: 0.4, stopFactor: 0.5,},
    },
    startProfileConfig: function() {
        if (!PioneerDDJREV1.brakingEnabled) {
            return this.brakeProfiles.off;
        }
        return this.brakeProfiles[PioneerDDJREV1.brakingStartProfile] || this.brakeProfiles.off;
    },
    stopProfileConfig: function() {
        if (!PioneerDDJREV1.brakingEnabled) {
            return this.brakeProfiles.off;
        }
        return this.brakeProfiles[PioneerDDJREV1.brakingStopProfile] || this.brakeProfiles.off;
    },
    syncPressed: function(value, group, status) {
        if (!value) {
            return;
        }
        const resolvedGroup = group || PioneerDDJREV1.getDeckGroupFromStatus(status);
        const resolvedStatus = PioneerDDJREV1.resolveDeckStatusForSync(resolvedGroup, status);
        if (!resolvedGroup) {
            return;
        }
        if (PioneerDDJREV1.syncPressPulseEnabled) {
            PioneerDDJREV1.pulseSyncLed(resolvedGroup, resolvedStatus, PioneerDDJREV1.syncPressPulseMs || 180);
        }
        engine.setValue(resolvedGroup, "beatsync", 1);
    },
    syncLongPressed: function(value, group) {
        if (value) {
            // Shift/long press engage sync lock and keep LED steady.
            engine.setValue(group, "sync_enabled", 1);
        }
    },
    cycleTempoRange: function(value, group) {
        if (value === 0) {
            return;
        }
        const currRangeRaw = engine.getValue(group, "rateRange");
        const currRange = Number(currRangeRaw);
        let idx = 0;
        if (Number.isFinite(currRange)) {
            for (let i = 0; i < PioneerDDJREV1.tempoRanges.length; i++) {
                const r = PioneerDDJREV1.tempoRanges[i];
                if (Math.abs(currRange - r) < 0.0001) {
                    idx = (i + 1) % PioneerDDJREV1.tempoRanges.length;
                    break;
                }
            }
        }
        const next = PioneerDDJREV1.tempoRanges[idx];
        engine.setValue(group, "rateRange", next);
    },
    cueShift: function(value, group) {
        if (engine.getValue(group, "play")) {
            engine.setValue(group, "start_stop", value);
        } else if (engine.getValue(group, "track_loaded")) {
            engine.setValue(group, "eject", value);
            engine.setValue(group, "eject", value);
        }
    },
    cancelBrakeForceStop: function(group) {
        if (PioneerDDJREV1.brakeForceStopTimers[group] !== undefined) {
            engine.stopTimer(PioneerDDJREV1.brakeForceStopTimers[group]);
            PioneerDDJREV1.brakeForceStopTimers[group] = undefined;
        }
    },
    cancelActiveBrake: function(group) {
        this.cancelBrakeForceStop(group);
        PioneerDDJREV1.brakingActive[group] = false;
        PioneerDDJREV1.hotcuePreviewFromBrake[group] = false;
        const deck = script.deckFromGroup(group);
        if (deck) {
            engine.brake(deck, false);
            engine.softStart(deck, false);
        }
    },
    scheduleBrakeForceStop: function(group, stopFactor) {
        // Only enforce a delayed hard-stop for non-classic braking profiles.
        if (stopFactor >= 1.0 || stopFactor <= 0) {
            return;
        }
        if (!PioneerDDJREV1.brakeForceStopTimerEnabled) {
            return;
        }

        this.cancelBrakeForceStop(group);
        PioneerDDJREV1.brakeGeneration[group] = (PioneerDDJREV1.brakeGeneration[group] || 0) + 1;
        const generation = PioneerDDJREV1.brakeGeneration[group];
        const startedAt = Date.now();
        const minWaitMs = 2200;
        const maxWaitMs = 3600;
        let lastPos = engine.getValue(group, "playposition");
        let stableTicks = 0;

        PioneerDDJREV1.brakeForceStopTimers[group] = engine.beginTimer(120, () => {
            if (PioneerDDJREV1.brakeGeneration[group] !== generation) {
                this.cancelBrakeForceStop(group);
                return;
            }
            // Hotcue hold flow owns stop/play outcome during an active hold window.
            if (PioneerDDJREV1.ComponentJSTransport.hasActiveHold(group)) {
                this.cancelBrakeForceStop(group);
                return;
            }

            const isPlayingNow = engine.getValue(group, "play") > 0;
            if (!isPlayingNow) {
                this.cancelBrakeForceStop(group);
                return;
            }

            const now = Date.now();
            const pos = engine.getValue(group, "playposition");
            const delta = Math.abs(pos - lastPos);
            lastPos = pos;

            if (delta < 0.0002) {
                stableTicks += 1;
            } else {
                stableTicks = 0;
            }

            const oldEnough = (now - startedAt) >= minWaitMs;
            const timedOut = (now - startedAt) >= maxWaitMs;
            const stableEnough = stableTicks >= 4;
            if ((oldEnough && stableEnough) || timedOut) {
                engine.setValue(group, "play", 0);
                PioneerDDJREV1.brakingActive[group] = false;
                this.cancelBrakeForceStop(group);
            }
        });
    },
    shiftPlayBrake: function(value, group, ignoreReverseSwap) {
        const deck = script.deckFromGroup(group);
        const activate = value > 0;
        const isPlaying = engine.getValue(group, "play") > 0;
        const startProfile = this.startProfileConfig();
        const stopProfile = this.stopProfileConfig();

        if (!ignoreReverseSwap && PioneerDDJREV1.brakingEnabled && PioneerDDJREV1.brakingReverse) {
            if (activate) {
                this.cancelBrakeForceStop(group);
                script.toggleControl(group, "play");
            }
            return;
        }
        if (!PioneerDDJREV1.brakingEnabled) {
            // Shift+Play fallback: stutter play (jump to cue and continue playback).
            if (activate) {
                this.cancelBrakeForceStop(group);
                engine.setValue(group, "cue_gotoandplay", 1);
            }
            return;
        }
        if (!activate) {
            return;
        }

        if (isPlaying) {
            this.cancelBrakeForceStop(group);
            if (stopProfile.stopFactor <= 0) {
                PioneerDDJREV1.brakingActive[group] = false;
                engine.setValue(group, "play", 0);
                return;
            }
            PioneerDDJREV1.brakingActive[group] = true;
            if (stopProfile.stopFactor === 1.0) {
                engine.brake(deck, true);
                return;
            }
            engine.brake(deck, value > 0, stopProfile.stopFactor);
            this.scheduleBrakeForceStop(group, stopProfile.stopFactor);
            return;
        }

        if (startProfile.startFactor <= 0) {
            this.cancelBrakeForceStop(group);
            engine.setValue(group, "play", 1);
            return;
        }
        this.cancelBrakeForceStop(group);
        engine.softStart(deck, value > 0, startProfile.startFactor);
    },
    playPressed: function(value, group) {
        if (!value) {
            return;
        }
        const activeHold = PioneerDDJREV1.ComponentJSTransport.hasActiveHold(group);
        PioneerDDJREV1.hotcueHoldActive[group] = activeHold;
        if (activeHold) {
            PioneerDDJREV1.ComponentJSTransport.notePlayTapDuringHotcueHold(group);
        }
        if (!activeHold && PioneerDDJREV1.hotcuePreviewFromBrake[group]) {
            // Stale preview flag should not hijack normal play behavior.
            PioneerDDJREV1.hotcuePreviewFromBrake[group] = false;
        }
        if (activeHold && PioneerDDJREV1.hotcuePreviewFromBrake[group]) {
            PioneerDDJREV1.hotcuePreviewFromBrake[group] = false;
            this.cancelActiveBrake(group);
            engine.setValue(group, "play", 1);
            return;
        }
        if (activeHold && engine.getValue(group, "play") > 0) {
            this.cancelActiveBrake(group);
            engine.setValue(group, "play", 1);
            return;
        }
        if (PioneerDDJREV1.brakingEnabled && PioneerDDJREV1.brakingReverse) {
            this.cancelBrakeForceStop(group);
            this.shiftPlayBrake(1, group, true);
            return;
        }
        if (PioneerDDJREV1.brakingActive[group]) {
            this.cancelActiveBrake(group);
            // If deck is already playing, preserve normal toggle semantics.
            if (engine.getValue(group, "play") > 0) {
                script.toggleControl(group, "play");
                return;
            }
            engine.setValue(group, "play", 1);
            return;
        }
        script.toggleControl(group, "play");
    },
};

PioneerDDJREV1.Components.Jog = {
    // ** jogTurn ** - scratch or bend based on vinyl mode
    jogTurn: function(channel, control, value, group) {
        const deckNum = channel + 1;
        const newVal = value - 64;
        if (engine.isScratching(deckNum)) {
            engine.scratchTick(deckNum, newVal);
        } else {
            const trackLoaded = engine.getValue(group, "track_loaded");
            // Waveform zoom must ONLY apply to jog-side pitch bend (midino 0x21),
            // never the platter pitch bend (0x23) nor platter scratch tick (0x22).
            const isJogSide = (control & 0xFF) === 0x21;
            const zoomEnabled = PioneerDDJREV1.waveformZoomEnabled && isJogSide;
            const zoomMode = PioneerDDJREV1.waveformZoomMode || "vinyl";
            const deckIsVinyl = !!PioneerDDJREV1.vinylMode[channel];
            const modeAllowsZoom = (zoomMode === "vinyl" && deckIsVinyl) || (zoomMode === "cdj" && !deckIsVinyl);
            if (zoomEnabled && modeAllowsZoom && trackLoaded) {
                if (value === 63) {
                    PioneerDDJREV1.wave = PioneerDDJREV1.wave + 0.1;
                    engine.setValue(group, "waveform_zoom", PioneerDDJREV1.wave);
                } else if (value === 65) {
                    PioneerDDJREV1.wave = PioneerDDJREV1.wave - 0.1;
                    engine.setValue(group, "waveform_zoom", PioneerDDJREV1.wave);
                }
            } else {
                engine.setValue(group, "jog", newVal * PioneerDDJREV1.bendScale);
            }
        }
    },
    // ** jogSearch ** - wheel position search when loop adjust active
    jogSearch: function(channel, value, group) {
        const deckNum = channel + 1;
        let newVal = value - 64;
        const loopEnabled = engine.getValue(group, "loop_enabled");
        if (loopEnabled > 0) {
            if (PioneerDDJREV1.loopAdjustIn[channel]) {
                newVal = newVal * PioneerDDJREV1.loopAdjustMultiply + engine.getValue(group, "loop_start_position");
                engine.setValue(group, "loop_start_position", newVal);
                return;
            }
            if (PioneerDDJREV1.loopAdjustOut[channel]) {
                newVal = newVal * PioneerDDJREV1.loopAdjustMultiply + engine.getValue(group, "loop_end_position");
                engine.setValue(group, "loop_end_position", newVal);
                return;
            }
        }
        newVal = newVal * PioneerDDJREV1.fastSeekScale;
        if (engine.isScratching(deckNum)) {
            engine.scratchTick(deckNum, newVal);
        } else {
            engine.setValue(group, "jog", newVal * PioneerDDJREV1.bendScale);
        }
    },
    // ** jogTouch ** - platter touch; enables scratch or search
    jogTouch: function(channel, control, value, group) {
        const deckNum = channel + 1;
        const vinylEnabled = PioneerDDJREV1.vinylMode[channel];

        PioneerDDJREV1.jogPlatterTouched[channel] = value !== 0;

        if (PioneerDDJREV1.VinylSlipAutoff) {
            engine.setValue(group, "slip_enabled", value !== 0 ? 1 : 0);
        }

        if (PioneerDDJREV1.loopAdjustIn[channel] || PioneerDDJREV1.loopAdjustOut[channel]) {
            return;
        }

        if (value !== 0 && vinylEnabled) {
            PioneerDDJREV1.brakeInterruptedByScratch[channel] = !!PioneerDDJREV1.brakingActive[group];
            PioneerDDJREV1.Components.invoke("transport", "cancelActiveBrake", [group]);
            engine.scratchEnable(deckNum, PioneerDDJREV1.nonShiftScratchResolution, 33 + 1 / 3, PioneerDDJREV1.alpha, PioneerDDJREV1.beta);
        } else {
            engine.scratchDisable(deckNum);
            if (PioneerDDJREV1.brakeInterruptedByScratch[channel]) {
                PioneerDDJREV1.brakeInterruptedByScratch[channel] = false;
                engine.setValue(group, "play", 0);
            }
            if (PioneerDDJREV1.VinylSlipAutoff && vinylEnabled) {
                engine.setValue(group, "slip_enabled", 0);
            }
        }
    },
    toggleVinylMode: function(channel, value, group) {
        if (value > 0) {
            const deckNum = channel + 1;
            PioneerDDJREV1.vinylMode[channel] = !PioneerDDJREV1.vinylMode[channel];
            PioneerDDJREV1.loopAdjustIn[channel] = false;
            PioneerDDJREV1.loopAdjustOut[channel] = false;
            if (engine.isScratching(deckNum)) {
                engine.scratchDisable(deckNum);
            }
            if (PioneerDDJREV1.VinylSlipAutoff) {
                engine.setValue(group, "slip_enabled", 0);
            }
            PioneerDDJREV1.syncHardwareVinylMode(channel);
            if (PioneerDDJREV1.cdjVinylTogglePulseEnabled) {
                PioneerDDJREV1.startDeckSelectModeBlink(channel, PioneerDDJREV1.vinylMode[channel]);
            }
        }
    },
    shiftButton: function(value) {
        const releasing = PioneerDDJREV1.shiftPressed && !(value > 0);
        const wasShift = PioneerDDJREV1.shiftPressed;
        PioneerDDJREV1.shiftPressed = value > 0;
        if (releasing) {
            PioneerDDJREV1.MixxxedModeOnShiftReleased();
        }
    },
};

PioneerDDJREV1.Components.Sampler = {
    samplerPadContextByGroup: {},
    getActiveSamplerMode: function() {
        return PioneerDDJREV1.samplerMode === "dual8_exception" ? "dual8_exception" : "standard16";
    },
    getUserPadLayout: function() {
        if (PioneerDDJREV1.samplePadLayout === "banked_rows") {
            return "banked_rows";
        }
        if (PioneerDDJREV1.samplePadLayout === "deck32") {
            return "deck32";
        }
        return "standard";
    },
    isLowerSamplerPadControl: function(control) {
        const padControl = Number(control);
        return Number.isFinite(padControl) && padControl >= 0x34 && padControl <= 0x37;
    },
    getAvailableSamplerCount: function() {
        const configuredCount = Number(engine.getValue("[App]", "num_samplers"));
        if (!Number.isFinite(configuredCount) || configuredCount <= 0) {
            return 16;
        }
        return Math.max(1, Math.floor(configuredCount));
    },
    normalizeDeckMidiChannel: function(channel) {
        const numeric = Number(channel);
        if (!Number.isFinite(numeric)) {
            return null;
        }
        const deckBaseChannelMap = {
            7: 0x97, 8: 0x97,
            9: 0x99, 10: 0x99,
            11: 0x9B, 12: 0x9B,
            13: 0x9D, 14: 0x9D,
            0x97: 0x97, 0x98: 0x97,
            0x99: 0x99, 0x9A: 0x99,
            0x9B: 0x9B, 0x9C: 0x9B,
            0x9D: 0x9D, 0x9E: 0x9D,
        };
        return deckBaseChannelMap[numeric] || null;
    },
    normalizeDeckMidiChannelFromStatus: function(status) {
        const numeric = Number(status);
        if (!Number.isFinite(numeric)) {
            return null;
        }
        const statusMap = {
            0x97: 0x97, 0x98: 0x97,
            0x99: 0x99, 0x9A: 0x99,
            0x9B: 0x9B, 0x9C: 0x9B,
            0x9D: 0x9D, 0x9E: 0x9D,
        };
        return statusMap[numeric] || null;
    },
    sideForInput: function(channel, status) {
        const normalizedFromStatus = this.normalizeDeckMidiChannelFromStatus(status);
        if (normalizedFromStatus === 0x97 || normalizedFromStatus === 0x9B) {
            return "left";
        }
        if (normalizedFromStatus === 0x99 || normalizedFromStatus === 0x9D) {
            return "right";
        }
        const normalized = this.normalizeDeckMidiChannel(channel);
        if (normalized === 0x97 || normalized === 0x9B) {
            return "left";
        }
        if (normalized === 0x99 || normalized === 0x9D) {
            return "right";
        }
        return null;
    },
    resolveSamplerNumber: function(channel, control, status) {
        const padControl = Number(control);
        if (padControl < 0x30 || padControl > 0x37) {
            return null;
        }
        const side = this.sideForInput(channel, status);
        if (!side) {
            return null;
        }
        const samplerMode = this.getActiveSamplerMode();
        const padLayout = this.getUserPadLayout();
        const padIndex = padControl - 0x30;
        if (samplerMode === "dual8_exception" && padIndex > 3) {
            return null;
        }

        if (padLayout === "deck32") {
            // Map each deck’s 8 pads to its own sampler bank:
            // Deck 1 -> 1–8, Deck 2 -> 9–16, Deck 3 -> 17–24, Deck 4 -> 25–32.
            const normalizedStatus = this.normalizeDeckMidiChannelFromStatus(status);
            const normalizedChannel = normalizedStatus !== null ? normalizedStatus : this.normalizeDeckMidiChannel(channel);
            const deckIndexByStatus = { 0x97: 0, 0x99: 1, 0x9B: 2, 0x9D: 3, };
            const deckIdx = deckIndexByStatus[normalizedChannel];
            if (deckIdx === undefined) {
                return null;
            }
            // Conflict note: ScratchBank currently uses 17–24.
            if (deckIdx === 2 && PioneerDDJREV1.scratchBankOnStemPads && !PioneerDDJREV1.scratchBankGuardWarnings.deck32ConflictLogged) {
                PioneerDDJREV1.scratchBankGuardWarnings.deck32ConflictLogged = true;
                print("WARNING: samplePadLayout=32 maps Deck 3 to Samplers 17–24 which conflicts with ScratchBank’s 17–24 pool.");
            }
            return (deckIdx * 8) + padIndex + 1;
        }
        const row = padIndex < 4 ? 0 : 1;
        const col = padIndex % 4;

        
        if (padLayout === "banked_rows" || "dual8_exception") {
            const rowOffset = row * 8;
            const sideOffset = side === "left" ? 0 : 4;
            return rowOffset + sideOffset + col + 1;
        }

        const sideOffset = side === "left" ? 0 : 8;
        return sideOffset + padIndex + 1;
    },
    resolveSamplerGroup: function(channel, control, status) {
        const samplerNumber = this.resolveSamplerNumber(channel, control, status);
        if (!samplerNumber) {
            return null;
        }
        const availableCount = this.getAvailableSamplerCount();
        if (samplerNumber > availableCount) {
            print(
                "Sampler pad ignored: mapped sampler " + samplerNumber +
                " exceeds available sampler count " + availableCount
            );
            return null;
        }
        return "[Sampler" + samplerNumber + "]";
    },
    samplerPlayOutputCallbackFunction: function(value, group) {
        if (value !== 1) {
            return;
        }
        const padContext = this.samplerPadContextByGroup[group];
        if (padContext) {
            this.startSamplerBlink(padContext.channel, padContext.control, group);
            return;
        }
        // If play was triggered without a pad press (e.g. via GUI), derive the LED target.
        const match = group && group.match(script.samplerRegEx);
        if (!match) {
            return;
        }
        const samplerNumber = Number(match[1]);
        if (!Number.isFinite(samplerNumber)) {
            return;
        }
        if (this.getUserPadLayout() !== "deck32") {
            return;
        }
        const deckIdx = Math.floor((samplerNumber - 1) / 8);
        const padIdx = (samplerNumber - 1) % 8;
        if (deckIdx < 0 || deckIdx > 3) {
            return;
        }
        const baseStatuses = [0x97, 0x99, 0x9B, 0x9D];
        const baseStatus = baseStatuses[deckIdx];
        const note = 0x30 + padIdx;
        this.startSamplerBlink(baseStatus, note, group);
    },
    getSamplerLedTargets: function(samplerNumber) {
        const samplerMode = this.getActiveSamplerMode();
        const targets = [];
        const maxPad = samplerMode === "dual8_exception" ? 3 : 7;
        const padLayout = this.getUserPadLayout();
        if (padLayout === "deck32") {
            const deckIdx = Math.floor((samplerNumber - 1) / 8);
            const padIdx = (samplerNumber - 1) % 8;
            if (deckIdx < 0 || deckIdx > 3) {
                return targets;
            }
            const baseStatuses = [0x97, 0x99, 0x9B, 0x9D];
            const baseStatus = baseStatuses[deckIdx];
            const note = 0x30 + padIdx;
            // Light both status layers for the deck (base + base+1) for consistency
            // with blink behavior which also targets channel+1.
            targets.push({ status: baseStatus, note: note, });
            targets.push({ status: baseStatus + 1, note: note, });
            return targets;
        }
        const sideConfigs = [
            {probeChannel: 7, probeStatus: 0x97, outputStatuses: [0x97, 0x9B],},
            {probeChannel: 9, probeStatus: 0x99, outputStatuses: [0x99, 0x9D],},
        ];
        sideConfigs.forEach((cfg) => {
            for (let padIndex = 0; padIndex <= maxPad; ++padIndex) {
                const note = 0x30 + padIndex;
                const resolvedSampler = this.resolveSamplerNumber(cfg.probeChannel, note, cfg.probeStatus);
                if (resolvedSampler !== samplerNumber) {
                    continue;
                }
                cfg.outputStatuses.forEach(function(outStatus) {
                    targets.push({ status: outStatus, note: note, });
                });
            }
        });
        return targets;
    },
    samplerTrackLoadedOutputCallbackFunction: function(value, group) {
        const match = group && group.match(script.samplerRegEx);
        if (!match) {
            return;
        }
        const samplerNumber = Number(match[1]);
        if (!Number.isFinite(samplerNumber)) {
            return;
        }
        const state = value >= 0.5 ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF;
        this.getSamplerLedTargets(samplerNumber).forEach(function(target) {
            midi.sendShortMsg(target.status, target.note, state);
        });
    },
    refreshSamplerTrackLoadedLeds: function() {
        const samplerMode = this.getActiveSamplerMode();
        const maxPad = samplerMode === "dual8_exception" ? 3 : 7;
        // Clear both deck status layers so no stale pad LEDs remain.
        [0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E].forEach(function(status) {
            for (let note = 0x30; note <= 0x30 + maxPad; ++note) {
                midi.sendShortMsg(status, note, PioneerDDJREV1.LED_OFF);
            }
        });
        const availableCount = this.getAvailableSamplerCount();
        const userLayout = this.getUserPadLayout();
        const maxByLayout = userLayout === "deck32" ? 32 : 16;
        const maxSampler = samplerMode === "dual8_exception"
            ? Math.min(8, availableCount)
            : Math.min(maxByLayout, availableCount);
        for (let samplerNumber = 1; samplerNumber <= maxSampler; ++samplerNumber) {
            const group = "[Sampler" + samplerNumber + "]";
            const trackLoaded = engine.getValue(group, "track_loaded");
            this.samplerTrackLoadedOutputCallbackFunction(trackLoaded, group);
        }
    },
    samplerPadPressed: function(channel, control, value, group, status) {
        const samplerGroup = this.resolveSamplerGroup(channel, control, status);
        if (!samplerGroup) {
            return;
        }
        PioneerDDJREV1.samplerChannel = channel;
        if (value > 0) {
            const normalizedChannel = this.normalizeDeckMidiChannel(channel);
            if (normalizedChannel !== null) {
                this.samplerPadContextByGroup[samplerGroup] = {
                    channel: normalizedChannel,
                    control: Number(control),
                };
            }
        }
        if (engine.getValue(samplerGroup, "track_loaded")) {
            engine.setValue(samplerGroup, "cue_gotoandplay", value);
        } else {
            engine.setValue(samplerGroup, "LoadSelectedTrack", value);
        }
    },
    samplerPadShiftPressed: function(channel, control, value, group, status) {
        const samplerGroup = this.resolveSamplerGroup(channel, control, status);
        if (!samplerGroup) {
            return;
        }
        if (engine.getValue(samplerGroup, "play")) {
            engine.setValue(samplerGroup, "cue_gotoandstop", value);
        } else if (engine.getValue(samplerGroup, "track_loaded")) {
            engine.setValue(samplerGroup, "eject", value);
        }
    },
    startSamplerBlink: function(channel, control, group) {
        let val = 0x7f;
        this.stopSamplerBlink(channel, control);
        PioneerDDJREV1.timers[channel][control] = engine.beginTimer(250, () => {
            val = 0x7f - val;
            midi.sendShortMsg(channel, control, val);
            midi.sendShortMsg((channel + 1), control, val);
            const isPlaying = engine.getValue(group, "play") === 1;
            if (!isPlaying) {
                this.stopSamplerBlink(channel, control);
                midi.sendShortMsg(channel, control, 0x7f);
                midi.sendShortMsg((channel + 1), control, 0x7f);
            }
        });
    },
    stopSamplerBlink: function(channel, control) {
        PioneerDDJREV1.timers[channel] = PioneerDDJREV1.timers[channel] || {};
        if (PioneerDDJREV1.timers[channel][control] !== undefined) {
            engine.stopTimer(PioneerDDJREV1.timers[channel][control]);
            PioneerDDJREV1.timers[channel][control] = undefined;
        }
    },
    samplerVolume: function(value, group) {
        PioneerDDJREV1.sampleShiftPressed = value > 0;
        PioneerDDJREV1.lastStemChannel = group;
        if (!PioneerDDJREV1.tempSamplerSkin) {
            return;
        }
        engine.setParameter("[Skin]", "show_samplers", false);
        this.refreshSamplerTrackLoadedLeds();
    },
};

PioneerDDJREV1.Components.ScratchBank = {
    deckMappings: {
        7: "[Channel1]",
        9: "[Channel2]",
        11: "[Channel3]",
        13: "[Channel4]",
        8: "[Channel1]",
        10: "[Channel2]",
        12: "[Channel3]",
        14: "[Channel4]",
    },
    samplerMappings: {
        7: {
            midiChannel: 0x97,
            side: "left",
        },
        9: {
            midiChannel: 0x99,
            side: "right",
        },
        11: {
            midiChannel: 0x9B,
            side: "left",
        },
        13: {
            midiChannel: 0x9D,
            side: "right",
        },
    },
    normalizeChannelKey: function(channel) {
        // ScratchBank sampler mappings are authored for odd channel keys (7/9/11/13).
        // Some MIDI paths can provide even channel variants, so normalize to the
        // nearest odd deck key to avoid silent sampler-map misses.
        const numeric = Number(channel);
        if (!Number.isFinite(numeric)) {
            return channel;
        }
        if (this.samplerMappings[numeric]) {
            return numeric;
        }
        const oddKey = (numeric % 2 === 0) ? (numeric - 1) : numeric;
        if (this.samplerMappings[oddKey]) {
            return oddKey;
        }
        return numeric;
    },
    resolveActiveSamples: function(channel) {
        const normalizedChannel = this.normalizeChannelKey(channel);
        const channelMapping = this.samplerMappings[normalizedChannel] || {
            midiChannel: 0x97,
            side: "left",
        };
        const availableCount = this.getAvailableSamplerCount();
        const pool = this.resolveScratchSamplerPool(availableCount);
        const sideStart = channelMapping.side === "right" ? pool.rightStart : pool.leftStart;
        const controlBase = PioneerDDJREV1.scratchBankOnStemPads ? 0x70 : 0x34;
        const samples = {};
        for (let i = 0; i < 4; ++i) {
            samples[controlBase + i] = sideStart + i;
        }
        return {
            midiChannel: channelMapping.midiChannel,
            samples: samples,
        };
    },
    resolveActiveSamplesForControlBase: function(channel, controlBase) {
        const normalizedChannel = this.normalizeChannelKey(channel);
        const channelMapping = this.samplerMappings[normalizedChannel] || {
            midiChannel: 0x97,
            side: "left",
        };
        const availableCount = this.getAvailableSamplerCount();
        const pool = this.resolveScratchSamplerPool(availableCount);
        const sideStart = channelMapping.side === "right" ? pool.rightStart : pool.leftStart;
        const base = Number(controlBase);
        const resolvedBase = Number.isFinite(base) ? base : 0x34;
        const samples = {};
        for (let i = 0; i < 4; ++i) {
            samples[resolvedBase + i] = sideStart + i;
        }
        return {
            midiChannel: channelMapping.midiChannel,
            samples: samples,
        };
    },
    getAvailableSamplerCount: function() {
        return PioneerDDJREV1.Components.Sampler.getAvailableSamplerCount();
    },
    resolveScratchSamplerPool: function() {
        // ScratchBank contract: fixed sample pool 17-24 only.
        return {
            leftStart: 17,
            rightStart: 21,
        };
    },
    isSamplerNumberAvailable: function(samplerNumber, availableCount) {
        return Number.isFinite(samplerNumber) && samplerNumber >= 1 && samplerNumber <= availableCount;
    },
    warnGuardOnce: function(key, message) {
        if (PioneerDDJREV1.scratchBankGuardWarnings[key]) {
            return;
        }
        PioneerDDJREV1.scratchBankGuardWarnings[key] = true;
        print(message);
    },
    // ** Guard ** - ScratchBank requires Samplers 17-24
    ensureScratchSamplerAvailable: function(samplerNumber, availableCount, control, channel) {
        if (samplerNumber < 17 || samplerNumber > 24) {
            this.warnGuardOnce(
                "pool-range",
                "ScratchBank blocked: Sampler17..24 required. Mapped Sampler" + samplerNumber + " is outside range."
            );
            return false;
        }
        if (!this.isSamplerNumberAvailable(samplerNumber, availableCount)) {
            this.warnGuardOnce(
                "pool-count-" + availableCount,
                "ScratchBank blocked: Sampler17..24 required. Current active samplers: " + availableCount + ". Increase sampler count and load ScratchBank tracks in 17–24."
            );
            return false;
        }
        return true;
    },
    // ** loadScratchToDeck ** - clone sampler to deck when pad pressed
    loadScratchToDeck: function(channel, control, value, status) {
        if (!value) {
            return;
        }
        const inv = PioneerDDJREV1._loadMixxxedModeInvocation;
        const isLowerSamplerPad = Number(control) >= 0x34 && Number(control) <= 0x37;
        if (isLowerSamplerPad && !inv) {
            // Lower row is normal sampler pads unless Mixxxed-Mode autoloop row calls in (inv).
            const samplerMethod = PioneerDDJREV1.shiftPressed
                ? "samplerPadShiftPressed"
                : "samplerPadPressed";
            PioneerDDJREV1.Components.invoke("sampler", samplerMethod, [channel, control, value, "", status]);
            return;
        }
        PioneerDDJREV1.Components.ModeGate.enforceStemsPriority();
        const upperScratchActivePress = PioneerDDJREV1.scratchBankOnStemPads && !isLowerSamplerPad;
        const MixxxedModeScratchPress =
            PioneerDDJREV1.mixxxedModeEnabled && inv && isLowerSamplerPad;
        if (!upperScratchActivePress && !MixxxedModeScratchPress) {
            return;
        }
        const normalizedChannel = this.normalizeChannelKey(channel);
        const deckNumber = this.deckMappings[normalizedChannel] || this.deckMappings[channel] || "[Channel1]";
        const activeMapping = MixxxedModeScratchPress
            ? this.resolveActiveSamplesForControlBase(normalizedChannel, 0x34)
            : this.resolveActiveSamples(normalizedChannel);
        const midiChannel = activeMapping.midiChannel;
        const samplerNumber = activeMapping.samples[control];
        const availableCount = this.getAvailableSamplerCount();
        if (!samplerNumber) {
            return;
        }
        if (!this.ensureScratchSamplerAvailable(samplerNumber, availableCount, control, channel)) {
            return;
        }
        const deckLoaded = engine.getValue(deckNumber, "play");
        if (deckLoaded) {
            print("Deck loaded. Unload deck to load scratch sample " + deckNumber);
            return;
        }
        const samplerGroup = "[Sampler" + samplerNumber + "]";
        if (!engine.getValue(samplerGroup, "track_loaded")) {
            this.warnGuardOnce(
                "pool-track-" + samplerNumber,
                "ScratchBank blocked: Sampler" + samplerNumber + " has no loaded track."
            );
            return;
        }
        try {
            Object.keys(activeMapping.samples).forEach(function(sampleControl) {
                midi.sendShortMsg(midiChannel, Number(sampleControl), PioneerDDJREV1.LED_OFF);
            });
            midi.sendShortMsg(midiChannel, control, PioneerDDJREV1.LED_ON);
            engine.setValue(deckNumber, "CloneFromSampler", samplerNumber);
            print("Loaded sample " + samplerNumber + " into deck " + deckNumber);
            const deckIdx = script.deckFromGroup(deckNumber);
            if (deckIdx && PioneerDDJREV1.mixxxedModeEnabled) {
                PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckIdx);
            }
        } catch (error) {
            print("Error loading sample to deck: " + error);
        }
    },
};

PioneerDDJREV1.Components.Effects = {
    effectStates: {
        FX1: [false, false, false],
        FX2: [false, false, false],
    },
    buttonStates: {
        FX1: [false, false, false],
        FX2: [false, false, false],
    },
    buttonPressBuffer: {},
    buttonTimeouts: {},
    fxSelectorMovedWhileHeld: {
        FX1: false,
        FX2: false,
    },
    fxUnitTable: {
        "[EffectRack1_EffectUnit1": { key: "FX1", status: 0x94, },
        "[EffectRack1_EffectUnit2": { key: "FX2", status: 0x95, },
    },
    fxUpdate: function(value, group) {
        const newState = (value === 1);
        const midiValue = newState ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF;
        const prefix = group.substring(0, group.indexOf("_Effect"));
        const entry = this.fxUnitTable[prefix];
        if (!entry) {
            return;
        }
        const effectNum = parseInt(group.match(/Effect(\d+)\]/)[1], 10);
        this.effectStates[entry.key][effectNum - 1] = newState;
        midi.sendShortMsg(entry.status, 0x6F + effectNum, midiValue);
    },
    bufferButtonPress: function(fxGroup, buttonIndex, mixxxUnit) {
        if (!this.buttonPressBuffer[fxGroup]) {
            this.buttonPressBuffer[fxGroup] = [];
        }
        if (!this.buttonTimeouts[fxGroup]) {
            this.fxSelectorMovedWhileHeld[fxGroup] = false;
        }
        this.buttonPressBuffer[fxGroup].push(buttonIndex);
        if (this.buttonTimeouts[fxGroup]) {
            engine.stopTimer(this.buttonTimeouts[fxGroup]);
        }
        this.buttonTimeouts[fxGroup] = engine.beginTimer(50, function() {
            PioneerDDJREV1.Components.invoke("effects", "processBufferedPresses", [fxGroup, mixxxUnit]);
        }, true);
    },
    processBufferedPresses: function(fxGroup, mixxxUnit) {
        const presses = this.buttonPressBuffer[fxGroup] || [];
        const pressedButtons = [...new Set(presses)];
        const uniquePressCount = pressedButtons.length;
        const skipEffectToggle = this.fxSelectorMovedWhileHeld[fxGroup];
        this.fxSelectorMovedWhileHeld[fxGroup] = false;
        if (!skipEffectToggle) {
            if (uniquePressCount === 1) {
                this.handleSinglePress(fxGroup, pressedButtons[0], mixxxUnit);
            } else if (uniquePressCount === 2) {
                this.handleDoublePress(fxGroup, pressedButtons, mixxxUnit);
            } else if (uniquePressCount >= 3) {
                this.handleTriplePress(fxGroup, mixxxUnit);
            }
        }
        this.buttonPressBuffer[fxGroup] = [];
        delete this.buttonTimeouts[fxGroup];
    },
    handleSinglePress: function(fxGroup, buttonIndex, mixxxUnit) {
        for (let i = 0; i < 3; i++) {
            this.effectStates[fxGroup][i] = (i === buttonIndex);
        }
        this.syncSingleEffect(fxGroup, buttonIndex, mixxxUnit);
    },
    handleDoublePress: function(fxGroup, buttonIndices, mixxxUnit) {
        for (let i = 0; i < 3; i++) {
            this.effectStates[fxGroup][i] = buttonIndices.includes(i);
        }
        this.syncDualEffects(fxGroup, mixxxUnit, buttonIndices);
    },
    handleTriplePress: function(fxGroup, mixxxUnit) {
        this.effectStates[fxGroup].fill(true);
        this.syncAllEffects(mixxxUnit);
    },
    FX: function(control, value, status) {
        let fxGroup = null;
        let mixxxUnit = null;
        if (status === 0x94) {
            fxGroup = "FX1";
            mixxxUnit = "[EffectRack1_EffectUnit1]";
        } else if (status === 0x95) {
            fxGroup = "FX2";
            mixxxUnit = "[EffectRack1_EffectUnit2]";
        }
        if (!fxGroup || !mixxxUnit) {
            return;
        }
        let buttonIndex = -1;
        switch (control) {
            case 0x70: buttonIndex = 0; break;
            case 0x71: buttonIndex = 1; break;
            case 0x72: buttonIndex = 2; break;
        }
        if (buttonIndex === -1) {
            return;
        }
        this.buttonStates[fxGroup][buttonIndex] = value > 0;
        this.bufferButtonPress(fxGroup, buttonIndex, mixxxUnit);
    },
    syncSingleEffect: function(fxGroup, activeButtonIndex, mixxxUnit) {
        const unitNumber = mixxxUnit.includes("Unit1") ? 1 : 2;
        if (this.buttonStates[fxGroup][activeButtonIndex]) {
            const activeGroup = "[EffectRack1_EffectUnit" + unitNumber + "_Effect" + (activeButtonIndex + 1) + "]";
            engine.setValue(activeGroup, "enabled", 0);
            return;
        }
        for (let i = 0; i < 3; i++) {
            const effectGroup = "[EffectRack1_EffectUnit" + unitNumber + "_Effect" + (i + 1) + "]";
            engine.setValue(effectGroup, "enabled", i === activeButtonIndex ? 1 : 0);
        }
    },
    syncDualEffects: function(_fxGroup, mixxxUnit, buttonIndices) {
        const unitNumber = mixxxUnit.includes("Unit1") ? 1 : 2;
        for (let i = 0; i < 3; i++) {
            const effectGroup = "[EffectRack1_EffectUnit" + unitNumber + "_Effect" + (i + 1) + "]";
            engine.setValue(effectGroup, "enabled", buttonIndices.includes(i) ? 1 : 0);
        }
    },
    syncAllEffects: function(mixxxUnit) {
        const unitNumber = mixxxUnit.includes("Unit1") ? 1 : 2;
        for (let i = 0; i < 3; i++) {
            const effectGroup = "[EffectRack1_EffectUnit" + unitNumber + "_Effect" + (i + 1) + "]";
            engine.setValue(effectGroup, "enabled", 1);
        }
    },
    syncNoEffects: function(mixxxUnit) {
        const unitNumber = mixxxUnit.includes("Unit1") ? 1 : 2;
        for (let i = 0; i < 3; i++) {
            const effectGroup = "[EffectRack1_EffectUnit" + unitNumber + "_Effect" + (i + 1) + "]";
            engine.setValue(effectGroup, "enabled", 0);
        }
    },
    selector: function(value, group) {
        const anyButtonPressed = this.buttonStates.FX1.some(function(state) { return state; }) ||
            this.buttonStates.FX2.some(function(state) { return state; });
        if (!anyButtonPressed) {
            const stemRacks = PioneerDDJREV1.Components.Stems.quickRacksForHeldStemEffectPads();
            if (stemRacks.length > 0 && (value === 0x7F || value === 0x01)) {
                // Selector movement while stem effect pad(s) are held should not toggle effect on release.
                for (let i = 0; i < 16; i++) {
                    if (PioneerDDJREV1.stemEffectHeld[i]) {
                        PioneerDDJREV1.stemPadMovedWhileHeld[i] = true;
                    }
                }
                PioneerDDJREV1.Components.Stems.applyStemQuickChainStep(stemRacks, value === 0x7F);
                return;
            }
            if (value === 0x7F) {
                engine.setValue(group, "MoveUp", 1);
            } else if (value === 0x01) {
                engine.setValue("[Library]", "MoveDown", 1);
            }
        }
        const fxGroups = {
            FX1: "[EffectRack1_EffectUnit1",
            FX2: "[EffectRack1_EffectUnit2",
        };
        for (const groupKey in fxGroups) {
            const unitGroup = fxGroups[groupKey];
            for (let i = 0; i < 3; i++) {
                if (this.buttonStates[groupKey][i]) {
                    if (value === 0x7F) {
                        engine.setValue(unitGroup + "_Effect" + (i + 1) + "]", "effect_selector", -1);
                        this.fxSelectorMovedWhileHeld[groupKey] = true;
                    } else if (value === 0x01) {
                        engine.setValue(unitGroup + "_Effect" + (i + 1) + "]", "effect_selector", 1);
                        this.fxSelectorMovedWhileHeld[groupKey] = true;
                    }
                }
            }
        }
    },
};

PioneerDDJREV1.Components.Mixer = {
    toggleQuantize: function(value, group) {
        if (value) {
            script.toggleControl(group, "quantize");
        }
    },
    librarySort: function(group) {
        const deckIndexByGroup = {
            "[Channel1]": 0,
            "[Channel2]": 1,
            "[Channel3]": 2,
            "[Channel4]": 3,
        };
        const sortByField = {
            artist: { order: 1, column: 1, },
            title: { order: 1, column: 2, },
            album: { order: 1, column: 3, },
            albumartist: { order: 1, column: 4, },
            year: { order: 1, column: 5, },
            genre: { order: 1, column: 6, },
            composer: { order: 1, column: 7, },
            grouping: { order: 1, column: 8, },
            tracknumber: { order: 0, column: 9, },
            filetype: { order: 1, column: 10, },
            nativelocation: { order: 1, column: 11, },
            comment: { order: 1, column: 12, },
            duration: { order: 1, column: 13, },
            bitrate: { order: 1, column: 14, },
            bpm: { order: 1, column: 15, },
            replaygain: { order: 1, column: 16, },
            date: { order: 0, column: 17, },
            timesplayed: { order: 1, column: 18, },
            rating: { order: 1, column: 19, },
            key: { order: 1, column: 20, },
            preview: { order: 1, column: 21, },
            coverart: { order: 1, column: 22, },
            trackcolor: { order: 1, column: 30, },
            lastplayed: { order: 1, column: 31, },
        };
        const deckIndex = deckIndexByGroup[group];
        if (deckIndex === undefined) {
            return;
        }
        const defaultField = PioneerDDJREV1.librarySortDefaults[deckIndex] || "bpm";
        const configuredField = PioneerDDJREV1.librarySortDeckModes[deckIndex] || defaultField;
        const sortConfig = sortByField[configuredField] || sortByField[defaultField] || sortByField.bpm;
        let sortOrder;
        if (configuredField === PioneerDDJREV1.lastLibrarySortField) {
            const prev = PioneerDDJREV1.lastLibrarySortOrder;
            sortOrder = prev === 1 ? 0 : 1;
        } else {
            sortOrder = sortConfig.order;
            PioneerDDJREV1.lastLibrarySortField = configuredField;
        }
        PioneerDDJREV1.lastLibrarySortOrder = sortOrder;
        engine.setValue("[Library]", "sort_order", sortOrder);
        engine.setValue("[Library]", "sort_column_toggle", sortConfig.column);
    },
    crossFaderStart: function(group) {
        if (PioneerDDJREV1.disableStartFader) {
            return;
        }
        const oppositeChannels = { 1: 2, 2: 1, 3: 4, 4: 3, };
        const glitchcorrectionChannels = { 1: 3, 2: 1, 3: 4, 4: 2, };
        const channelGroups = { "[Channel1]": 1, "[Channel2]": 2, "[Channel3]": 3, "[Channel4]": 4, };
        if (group in channelGroups && PioneerDDJREV1.shiftPressed) {
            const currentChannel = channelGroups[group];
            const oppositeChannel = oppositeChannels[currentChannel];
            const oppositeGroup = "[Channel" + oppositeChannel + "]";
            const stopChannel = glitchcorrectionChannels[currentChannel];
            const stopGroup = "[Channel" + stopChannel + "]";
            engine.setValue(oppositeGroup, "play", 1);
            engine.setValue(stopGroup, "play", 0);
            engine.setValue(stopGroup, "cue_gotoandstop", 1);
            engine.setValue(group, "play", 0);
            engine.setValue(group, "cue_gotoandstop", 1);
        }
    },
};

PioneerDDJREV1.Components.Lifecycle = {
    // ** connect ** - register callbacks, startup MIDI, soft takeover
    connect: function() {
        engine.setValue("[EffectRack1_EffectUnit1]", "show_focus", 1);
        engine.setValue("[EffectRack1_EffectUnit2]", "show_focus", 1);

        const conns = PioneerDDJREV1.lifecycleConnections;
        if (PioneerDDJREV1.vuMeterMode === "main_lr") {
            let mainGroup = "[Main]";
            const leftConn = engine.makeUnbufferedConnection(mainGroup, "vu_meter_left", PioneerDDJREV1.onVuMeterChangeL);
            if (!leftConn) {
                mainGroup = "[Master]";
                conns.push(engine.makeUnbufferedConnection(mainGroup, "vu_meter_left", PioneerDDJREV1.onVuMeterChangeL));
            } else {
                conns.push(leftConn);
            }
            conns.push(engine.makeUnbufferedConnection(mainGroup, "vu_meter_right", PioneerDDJREV1.onVuMeterChangeR));
        } else {
            for (let d = 1; d <= 4; d++) {
                conns.push(engine.makeUnbufferedConnection("[Channel" + d + "]", "vu_meter", PioneerDDJREV1.vuMeterUpdate));
            }
        }

        PioneerDDJREV1.toggleLight(PioneerDDJREV1.lights.deck1.vuMeter, false);
        PioneerDDJREV1.toggleLight(PioneerDDJREV1.lights.deck2.vuMeter, false);
        PioneerDDJREV1.toggleLight(PioneerDDJREV1.lights.deck3.vuMeter, false);
        PioneerDDJREV1.toggleLight(PioneerDDJREV1.lights.deck4.vuMeter, false);
        for (let ch = 1; ch <= 4; ch++) {
            engine.softTakeover("[Channel" + ch + "]", "rate", true);
        }
        for (let unit = 1; unit <= 2; unit++) {
            for (let eff = 1; eff <= 3; eff++) {
                engine.softTakeover("[EffectRack1_EffectUnit" + unit + "_Effect" + eff + "]", "meta", true);
            }
            engine.softTakeover("[EffectRack1_EffectUnit" + unit + "]", "mix", true);
        }
        PioneerDDJREV1.enableEqKnobSoftTakeover();

        const configuredSamplerCount = Number(engine.getValue("[App]", "num_samplers"));
        const availableSamplerCount = (!Number.isFinite(configuredSamplerCount) || configuredSamplerCount <= 0)
            ? 16
            : Math.max(1, Math.floor(configuredSamplerCount));
        const userLayout = PioneerDDJREV1.Components.Sampler.getUserPadLayout();
        const maxByLayout = userLayout === "deck32" ? 32 : 16;
        const samplerCount = Math.min(maxByLayout, availableSamplerCount);
        for (let i = 1; i <= samplerCount; ++i) {
            conns.push(engine.makeConnection("[Sampler" + i + "]", "play", PioneerDDJREV1.samplerPlayOutputCallbackFunction));
            conns.push(engine.makeConnection("[Sampler" + i + "]", "track_loaded", PioneerDDJREV1.samplerTrackLoadedOutputCallbackFunction));
        }
        PioneerDDJREV1.Components.invoke("sampler", "refreshSamplerTrackLoadedLeds", []);

        for (let d = 1; d <= 4; d++) {
            conns.push(engine.makeConnection("[Channel" + d + "]", "track_loaded", PioneerDDJREV1.trackLoadedLED));
        }
        PioneerDDJREV1.Components.invoke("stems", "registerStemLedConnections", []);
        PioneerDDJREV1.ComponentJSTransport.registerHotcueLedConnections();
        PioneerDDJREV1.registerSyncLedConnections();

        midi.sendShortMsg(0x9F, 0x00, PioneerDDJREV1.LED_ON);
        midi.sendShortMsg(0x9F, 0x01, PioneerDDJREV1.LED_ON);
        const startupDeckLedNotes = [0x10, 0x11, 0x4E, 0x4C];
        for (let d = 0; d < 2; d++) {
            const deckStatus = 0x90 + d;
            for (let n = 0; n < startupDeckLedNotes.length; n++) {
                midi.sendShortMsg(deckStatus, startupDeckLedNotes[n], PioneerDDJREV1.LED_ON);
            }
        }

        for (let d = 1; d <= 4; d++) {
            conns.push(engine.makeConnection("[Channel" + d + "]", "loop_enabled", PioneerDDJREV1.loopToggle));
        }

        conns.push(engine.makeConnection("[Master]", "crossfader", PioneerDDJREV1.crossFaderStart));

        for (let i = 1; i <= 3; i++) {
            conns.push(engine.makeConnection("[EffectRack1_EffectUnit1_Effect" + i + "]", "enabled", PioneerDDJREV1.fxUpdate));
            conns.push(engine.makeConnection("[EffectRack1_EffectUnit2_Effect" + i + "]", "enabled", PioneerDDJREV1.fxUpdate));
        }
        // Preserve current Mixxx FX state at init and only refresh controller LEDs.
        for (let i = 1; i <= 3; i++) {
            const unit1Group = "[EffectRack1_EffectUnit1_Effect" + i + "]";
            const unit2Group = "[EffectRack1_EffectUnit2_Effect" + i + "]";
            PioneerDDJREV1.fxUpdate(engine.getValue(unit1Group, "enabled"), unit1Group);
            PioneerDDJREV1.fxUpdate(engine.getValue(unit2Group, "enabled"), unit2Group);
        }

            midi.sendSysexMsg(
                [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x06, 0x00, 0x03, 0x01, 0xf7],
                12
            );
    },
    // ** disconnect ** - turn off LEDs, stop timers, disconnect connections
    disconnect: function() {
        for (let c = 0; c < PioneerDDJREV1.lifecycleConnections.length; c++) {
            const conn = PioneerDDJREV1.lifecycleConnections[c];
            if (conn && typeof conn.disconnect === "function") {
                conn.disconnect();
            }
        }
        PioneerDDJREV1.lifecycleConnections = [];

        PioneerDDJREV1.Components.invoke("stems", "unregisterStemLedConnections", []);
        PioneerDDJREV1.ComponentJSTransport.unregisterHotcueLedConnections();
        PioneerDDJREV1.unregisterSyncLedConnections();
        PioneerDDJREV1.unregisterPianoRollPlaypositionConnection();
        PioneerDDJREV1.stopPianoRollShiftBlinkTimer();

        for (let d = 1; d <= 4; d++) {
            PioneerDDJREV1.toggleLight(PioneerDDJREV1.lights["deck" + d].vuMeter, false);
        }

        // Turn off all sampler LEDs.
        for (let i = 0; i <= 7; ++i) {
            midi.sendShortMsg(0x97, 0x30 + i, PioneerDDJREV1.LED_OFF);
            midi.sendShortMsg(0x98, 0x30 + i, PioneerDDJREV1.LED_OFF);
            midi.sendShortMsg(0x99, 0x30 + i, PioneerDDJREV1.LED_OFF);
            midi.sendShortMsg(0x9A, 0x30 + i, PioneerDDJREV1.LED_OFF);
        }
        // Turn off all hotcue LEDs.
        for (let i = 0; i <= 7; ++i) {
            midi.sendShortMsg(0x97, 0x00 + i, PioneerDDJREV1.LED_OFF);
            midi.sendShortMsg(0x98, 0x00 + i, PioneerDDJREV1.LED_OFF);
            midi.sendShortMsg(0x99, 0x00 + i, PioneerDDJREV1.LED_OFF);
            midi.sendShortMsg(0x9A, 0x00 + i, PioneerDDJREV1.LED_OFF);
        }

        PioneerDDJREV1.setLoopButtonLights(0x90, PioneerDDJREV1.LED_OFF);
        PioneerDDJREV1.setLoopButtonLights(0x91, PioneerDDJREV1.LED_OFF);
        PioneerDDJREV1.setReloopLight(0x90, PioneerDDJREV1.LED_OFF);
        PioneerDDJREV1.setReloopLight(0x91, PioneerDDJREV1.LED_OFF);
        PioneerDDJREV1.stopAllLoopLightTimers();

        PioneerDDJREV1.toggleLight(PioneerDDJREV1.lights.beatFx, false);
        PioneerDDJREV1.toggleLight(PioneerDDJREV1.lights.shiftBeatFx, false);
        if (PioneerDDJREV1.startupVinylSyncTimer !== null) {
            engine.stopTimer(PioneerDDJREV1.startupVinylSyncTimer);
            PioneerDDJREV1.startupVinylSyncTimer = null;
        }
        for (let i = 0; i < 4; i++) {
            if (PioneerDDJREV1.startupModeBlinkTimers[i] !== null) {
                engine.stopTimer(PioneerDDJREV1.startupModeBlinkTimers[i]);
                PioneerDDJREV1.startupModeBlinkTimers[i] = null;
            }
            PioneerDDJREV1.clearDeckSelectModeBlink(i);
        }
    },
};

PioneerDDJREV1.Components.Bootstrap = {
    registerDomains: function() {
        if (!PioneerDDJREV1.componentContainer) {
            return;
        }
        PioneerDDJREV1.componentContainer.settings = PioneerDDJREV1.Components.Settings;
        PioneerDDJREV1.componentContainer.capabilities = PioneerDDJREV1.Components.Capabilities;
        PioneerDDJREV1.componentContainer.modeGate = PioneerDDJREV1.Components.ModeGate;
        PioneerDDJREV1.componentContainer.transport = PioneerDDJREV1.Components.Transport;
        PioneerDDJREV1.componentContainer.jog = PioneerDDJREV1.Components.Jog;
        PioneerDDJREV1.componentContainer.beatPads = PioneerDDJREV1.Components.BeatPads;
        PioneerDDJREV1.componentContainer.stems = PioneerDDJREV1.Components.Stems;
        PioneerDDJREV1.componentContainer.headphones = PioneerDDJREV1.Components.Headphones;
        PioneerDDJREV1.componentContainer.sampler = PioneerDDJREV1.Components.Sampler;
        PioneerDDJREV1.componentContainer.effects = PioneerDDJREV1.Components.Effects;
        PioneerDDJREV1.componentContainer.scratchBank = PioneerDDJREV1.Components.ScratchBank;
        PioneerDDJREV1.componentContainer.mixer = PioneerDDJREV1.Components.Mixer;
        PioneerDDJREV1.componentContainer.lifecycle = PioneerDDJREV1.Components.Lifecycle; 
     },
    connect: function() {
        if (!PioneerDDJREV1.useComponentsJS || !PioneerDDJREV1.componentContainer) {
            PioneerDDJREV1.Components.Lifecycle.connect();
            return;
        }
        const lifecycle = PioneerDDJREV1.componentContainer.lifecycle;
        if (lifecycle && typeof lifecycle.connect === "function") {
            lifecycle.connect();
            return;
        }
        PioneerDDJREV1.Components.Lifecycle.connect();
    },
    initialize: function() {
        if (typeof components === "undefined" || typeof components.ComponentContainer !== "function") {
            PioneerDDJREV1.useComponentsJS = false;
            PioneerDDJREV1.componentContainer = null;
            return;
        }
        PioneerDDJREV1.componentContainer = new components.ComponentContainer();
        this.registerDomains();
        PioneerDDJREV1.Components.invalidateResolveCache();
        PioneerDDJREV1.useComponentsJS = true;
    },
    disconnect: function() {
        if (!PioneerDDJREV1.useComponentsJS || !PioneerDDJREV1.componentContainer) {
            PioneerDDJREV1.Components.Lifecycle.disconnect();
            return;
        }
        const lifecycle = PioneerDDJREV1.componentContainer.lifecycle;
        if (lifecycle && typeof lifecycle.disconnect === "function") {
            lifecycle.disconnect();
            return;
        }
        PioneerDDJREV1.Components.Lifecycle.disconnect();
    },
    shutdown: function() {
        this.disconnect();
        if (PioneerDDJREV1.useComponentsJS && PioneerDDJREV1.componentContainer &&
                typeof PioneerDDJREV1.componentContainer.forEachComponent === "function") {
            PioneerDDJREV1.componentContainer.forEachComponent(function(component) {
                if (component && typeof component.disconnect === "function") {
                    component.disconnect();
                }
            });
        }
        PioneerDDJREV1.componentContainer = null;
        PioneerDDJREV1.useComponentsJS = false;
    },
};


PioneerDDJREV1.enableEqKnobSoftTakeover = function() {
    for (let d = 1; d <= 4; d++) {
        const eg = "[EqualizerRack1_[Channel" + d + "]_Effect1]";
        engine.softTakeover(eg, "parameter1", true);
        engine.softTakeover(eg, "parameter2", true);
        engine.softTakeover(eg, "parameter3", true);
    }
};

PioneerDDJREV1.eqBandToParameterKey = function(band) {
    if (band === "low") return "parameter1";
    if (band === "mid") return "parameter2";
    return "parameter3";
};

PioneerDDJREV1.handleEqKnobValue = function(deckNum, band, normalized) {
    if (deckNum < 1 || deckNum > 4) {
        return;
    }
    const value = Math.max(0, Math.min(1, normalized));
    const eg = "[EqualizerRack1_[Channel" + deckNum + "]_Effect1]";
    try {
        engine.setParameter(eg, PioneerDDJREV1.eqBandToParameterKey(band), value);
    } catch (e) {
        // ignore
    }
};

PioneerDDJREV1.eqKnobDeckFromStatus = function(status) {
    return (status & 0x0F) + 1;
};

PioneerDDJREV1.eqKnobStoreMSB = function(deckNum, band, value) {
    if (deckNum < 1 || deckNum > 4) {
        return;
    }
    PioneerDDJREV1.eqKnobMsb[deckNum - 1][band] = value & 0x7F;
};

PioneerDDJREV1.eqKnobCombineAndDispatch = function(deckNum, band, lsbValue) {
    if (deckNum < 1 || deckNum > 4) {
        return;
    }
    const msb = PioneerDDJREV1.eqKnobMsb[deckNum - 1][band] & 0x7F;
    const fullValue = (msb << 7) + (lsbValue & 0x7F);
    const normalized = fullValue / 0x3FFF;
    PioneerDDJREV1.handleEqKnobValue(deckNum, band, normalized);
};

PioneerDDJREV1.eqLowMSB = function(_channel, _control, value, status, _group) {
    PioneerDDJREV1.eqKnobStoreMSB(PioneerDDJREV1.eqKnobDeckFromStatus(status), "low", value);
};
PioneerDDJREV1.eqLowLSB = function(_channel, _control, value, status, _group) {
    PioneerDDJREV1.eqKnobCombineAndDispatch(PioneerDDJREV1.eqKnobDeckFromStatus(status), "low", value);
};
PioneerDDJREV1.eqMidMSB = function(_channel, _control, value, status, _group) {
    PioneerDDJREV1.eqKnobStoreMSB(PioneerDDJREV1.eqKnobDeckFromStatus(status), "mid", value);
};
PioneerDDJREV1.eqMidLSB = function(_channel, _control, value, status, _group) {
    PioneerDDJREV1.eqKnobCombineAndDispatch(PioneerDDJREV1.eqKnobDeckFromStatus(status), "mid", value);
};
PioneerDDJREV1.eqHighMSB = function(_channel, _control, value, status, _group) {
    PioneerDDJREV1.eqKnobStoreMSB(PioneerDDJREV1.eqKnobDeckFromStatus(status), "high", value);
};
PioneerDDJREV1.eqHighLSB = function(_channel, _control, value, status, _group) {
    PioneerDDJREV1.eqKnobCombineAndDispatch(PioneerDDJREV1.eqKnobDeckFromStatus(status), "high", value);
};

//
// Piano Roll (Mixxxed-Mode slot 3): two-deck lock, scale maps, pitch + keylock

// Anchor modes: playhead anchor (first pad only); SHIFT+pad = hotcue override; playthrough = pitch only.
// Transport: pad hold = play while held; Play alone = roll until pad press stops; Play during pad hold = latch.
// Two-deck pad layout: left autoloop row vs right follows leftActiveDeck (deck 1/3 vs 2/4).

PioneerDDJREV1.pianoRollIsRightSideDeck = function(deckNum) {
    return deckNum !== PioneerDDJREV1.leftActiveDeck;
};

PioneerDDJREV1.pianoRollUsesPentatonicLayout = function(scaleName) {
    return scaleName === "pentatonic" || scaleName === "dorian" || scaleName === "mixolydian" || scaleName === "blues";
};

PioneerDDJREV1.pianoRollUsesTwoRowDeckSplit = function(scaleName) {
    return scaleName === "major" || scaleName === "minor" || scaleName === "chromatic";
};

PioneerDDJREV1.beatSlicerPatternTable = function() {
    const tables = {
        linear: [0, 1, 2, 3, 4, 5, 6, 7],
        loop: [0, 1, 2, 3, 7, 6, 5, 4],
        quantize_loop: [0, 1, 2, 3, 7, 6, 5, 4],
    };
    return tables[PioneerDDJREV1.beatSlicerPattern] || tables.linear;
};

PioneerDDJREV1.beatSlicerSlicePatternName = function() {
    const p = PioneerDDJREV1.beatSlicerPattern;
    if (p === "quantize_loop") {
        return "loop";
    }
    return p;
};

PioneerDDJREV1.beatSlicerQuantizeSection = function(group) {
    const p = PioneerDDJREV1.beatSlicerPattern;
    if (p !== "linear" && p !== "loop" && p !== "quantize_loop") {
        return false;
    }
    try {
        return engine.getValue(group, "quantize") > 0;
    } catch (e) {
        return false;
    }
};

PioneerDDJREV1.slicerHasBeatGrid = function(group) {
    try {
        return engine.getValue(group, "beat_closest") !== engine.getValue(group, "beat_next");
    } catch (e) {
        return false;
    }
};

/** Loop capture anchor aligned with Mixxx beatloop: beats+grid when available, seconds otherwise. */
PioneerDDJREV1.slicerLoopCaptureStartBeat = function(beatsExact, domain, group, bpm) {
    if (PioneerDDJREV1.slicerHasBeatGrid(group)) {
        return Math.floor(Math.round(beatsExact) / domain) * domain;
    }
    const secondsExact = (beatsExact * 60) / bpm;
    const startSeconds = Math.floor(secondsExact / domain) * domain;
    return (startSeconds * bpm) / 60;
};

PioneerDDJREV1.slicerSectionStartBeat = function(beatsExact, domain, _group) {
    // Floor only: rounding to nearest beat would push the last slice (pad 8) into the next domain.
    return Math.floor(beatsExact / domain) * domain;
};

PioneerDDJREV1.slicerUsesLoopConstraint = function(deckIdx, _group) {
    const modeLoop = PioneerDDJREV1.activeSlicerMode[deckIdx] === PioneerDDJREV1.slicerModes.loopSlice;
    return modeLoop && PioneerDDJREV1.slicerLoopVisual;
};

PioneerDDJREV1.slicerGetLoopStartBeat = function(deckIdx, _group, domain, _bpm) {
    const modeLoop = PioneerDDJREV1.activeSlicerMode[deckIdx] === PioneerDDJREV1.slicerModes.loopSlice;
    if (modeLoop && PioneerDDJREV1.slicerLoopVisual &&
            PioneerDDJREV1.slicerLoopDomain[deckIdx] === domain) {
        return PioneerDDJREV1.slicerLoopStartBeat[deckIdx];
    }
    return 0;
};

/** Single source of truth for domain section, loop window, and chase position. */
PioneerDDJREV1.slicerSectionContext = function(deckIdx, group, beatsExact, domain, bpm) {
    const loopConstrained = PioneerDDJREV1.slicerUsesLoopConstraint(deckIdx, group);
    const modeLoop = PioneerDDJREV1.activeSlicerMode[deckIdx] === PioneerDDJREV1.slicerModes.loopSlice;
    let sectionStart;
    let loopStartBeat = 0;
    let posInDomain;
    if (loopConstrained && PioneerDDJREV1.slicerHasValidLoopSamples(deckIdx)) {
        const trackSamples = Number(engine.getValue(group, "track_samples"));
        const playposition = engine.getValue(group, "playposition");
        const loopStartSample = PioneerDDJREV1.slicerLoopStartSamples[deckIdx];
        const loopEndSample = PioneerDDJREV1.slicerLoopEndSamples[deckIdx];
        const loopSpan = loopEndSample - loopStartSample;
        loopStartBeat = PioneerDDJREV1.slicerBeatsFromSamplePos(group, loopStartSample);
        sectionStart = loopStartBeat;
        if (trackSamples > 0 && loopSpan > 0) {
            const playSample = playposition * trackSamples;
            posInDomain = ((playSample - loopStartSample) / loopSpan) * domain;
        } else {
            posInDomain = ((beatsExact - loopStartBeat) % domain + domain) % domain;
        }
    } else if (loopConstrained) {
        loopStartBeat = PioneerDDJREV1.slicerGetLoopStartBeat(deckIdx, group, domain, bpm);
        sectionStart = loopStartBeat;
        posInDomain = ((beatsExact - loopStartBeat) % domain + domain) % domain;
    } else {
        sectionStart = PioneerDDJREV1.slicerSectionStartBeat(beatsExact, domain, group);
        posInDomain = ((beatsExact - sectionStart) % domain + domain) % domain;
    }
    return {
        sectionStart: sectionStart,
        loopStartBeat: loopStartBeat,
        loopEndBeat: loopStartBeat + domain,
        loopEndSample: PioneerDDJREV1.slicerHasValidLoopSamples(deckIdx)
            ? PioneerDDJREV1.slicerLoopEndSamples[deckIdx] : 0,
        loopConstrained: loopConstrained,
        posInDomain: posInDomain,
        modeLoop: modeLoop,
    };
};

PioneerDDJREV1.slicerHasValidLoopSamples = function(deckIdx) {
    const start = PioneerDDJREV1.slicerLoopStartSamples[deckIdx];
    const end = PioneerDDJREV1.slicerLoopEndSamples[deckIdx];
    return start >= 0 && end > start;
};

PioneerDDJREV1.slicerSampleRatio = function(group, samplePos) {
    const trackSamples = Number(engine.getValue(group, "track_samples"));
    if (!(trackSamples > 0) || !(samplePos >= 0)) {
        return 0;
    }
    return samplePos / trackSamples;
};

PioneerDDJREV1.slicerBeatsFromSamplePos = function(group, samplePos) {
    const duration = engine.getValue(group, "duration");
    const bpm = engine.getValue(group, "bpm");
    return PioneerDDJREV1.slicerBeatsExact(
        PioneerDDJREV1.slicerSampleRatio(group, samplePos), duration, bpm);
};

PioneerDDJREV1.slicerSeekInvalidateThresholdRatio = function(deckIdx, group) {
    const domain = PioneerDDJREV1.selectedSlicerDomain[deckIdx];
    const bpm = engine.getValue(group, "bpm");
    const duration = engine.getValue(group, "duration");
    if (!(bpm > 0) || !(duration > 0)) {
        return 0.0005;
    }
    const halfSliceSec = ((domain / 8) * 60) / bpm / 2;
    return halfSliceSec / duration;
};

/** Clear pad replay cache when playhead moves (paused scrub or playing jog). Loop window stays fixed. */
PioneerDDJREV1.slicerInvalidatePadCacheIfSeeked = function(deckIdx, group) {
    const playposition = engine.getValue(group, "playposition");
    if (PioneerDDJREV1.slicerSuppressSeekInvalidate[deckIdx]) {
        PioneerDDJREV1.slicerLastKnownPlayposition[deckIdx] = playposition;
        return false;
    }
    const prev = PioneerDDJREV1.slicerLastKnownPlayposition[deckIdx];
    if (prev < 0) {
        PioneerDDJREV1.slicerLastKnownPlayposition[deckIdx] = playposition;
        return false;
    }
    const threshold = PioneerDDJREV1.slicerSeekInvalidateThresholdRatio(deckIdx, group);
    if (Math.abs(playposition - prev) < threshold) {
        return false;
    }
    PioneerDDJREV1.slicerLastPadKey[deckIdx] = -1;
    PioneerDDJREV1.slicerActive[deckIdx] = false;
    PioneerDDJREV1.slicerButton[deckIdx] = 0;
    PioneerDDJREV1.slicerLastKnownPlayposition[deckIdx] = playposition;
    return true;
};

PioneerDDJREV1.slicerSyncKnownPlayposition = function(deckIdx, group) {
    PioneerDDJREV1.slicerLastKnownPlayposition[deckIdx] = engine.getValue(group, "playposition");
};

PioneerDDJREV1.slicerFlashAndReadLoopSamples = function(group, domain) {
    let captured = null;
    try {
        engine.setValue(group, "beatloop_" + domain + "_toggle", 1);
        const start = Number(engine.getValue(group, "loop_start_position"));
        const end = Number(engine.getValue(group, "loop_end_position"));
        engine.setValue(group, "beatloop_" + domain + "_toggle", 1);
        if (start >= 0 && end > start) {
            captured = { start: start, end: end, };
        }
    } catch (e) {
        // ignore
    }
    return captured;
};

PioneerDDJREV1.slicerResolveLoopSliceTarget = function(deckIdx, sliceIndex, group, duration) {
    const loopStart = PioneerDDJREV1.slicerLoopStartSamples[deckIdx];
    const loopEnd = PioneerDDJREV1.slicerLoopEndSamples[deckIdx];
    const trackSamples = Number(engine.getValue(group, "track_samples"));
    const span = loopEnd - loopStart;
    const sliceSpan = span / 8;
    const targetSample = loopStart + sliceIndex * sliceSpan;
    const sliceEndSample = Math.min(loopStart + (sliceIndex + 1) * sliceSpan, loopEnd);
    const targetRatio = PioneerDDJREV1.slicerSampleRatio(group, targetSample);
    const sliceEndSec = (trackSamples > 0) ? (sliceEndSample / trackSamples) * duration : 0;
    return {
        targetSample: targetSample,
        sliceEndSample: sliceEndSample,
        targetRatio: targetRatio,
        sliceEndSec: sliceEndSec,
        usesLoopSamples: true,
    };
};

PioneerDDJREV1.slicerFauxLoopJumpTick = function(deckNum, deckIdx, group) {
    if (!PioneerDDJREV1.slicerHasValidLoopSamples(deckIdx)) {
        return;
    }
    const loopStart = PioneerDDJREV1.slicerLoopStartSamples[deckIdx];
    const loopEnd = PioneerDDJREV1.slicerLoopEndSamples[deckIdx];
    const trackSamples = Number(engine.getValue(group, "track_samples"));
    if (!(trackSamples > 0)) {
        return;
    }
    const playSample = engine.getValue(group, "playposition") * trackSamples;
    const prevPlay = PioneerDDJREV1.slicerPreviousPlaySample[deckIdx];
    if (playSample >= loopEnd - 1 &&
            prevPlay < loopEnd &&
            !PioneerDDJREV1.slicerAlreadyJumped[deckIdx]) {
        PioneerDDJREV1.slicerSuppressSeekInvalidate[deckIdx] = true;
        engine.setValue(group, "playposition", loopStart / trackSamples);
        PioneerDDJREV1.slicerSuppressSeekInvalidate[deckIdx] = false;
        PioneerDDJREV1.slicerAlreadyJumped[deckIdx] = true;
    } else if (playSample < loopEnd - 1) {
        PioneerDDJREV1.slicerAlreadyJumped[deckIdx] = false;
    }
    PioneerDDJREV1.slicerPreviousPlaySample[deckIdx] = playSample;
};

PioneerDDJREV1.slicerCaptureIntendedLoopRegion = function(deckNum, deckIdx, group) {
    const domain = PioneerDDJREV1.selectedSlicerDomain[deckIdx];
    const bpm = engine.getValue(group, "bpm");
    const duration = engine.getValue(group, "duration");
    const playposition = engine.getValue(group, "playposition");
    if (!(bpm > 0) || !(duration > 0)) {
        return;
    }
    const beatsExact = PioneerDDJREV1.slicerBeatsExact(playposition, duration, bpm);
    const captured = PioneerDDJREV1.slicerFlashAndReadLoopSamples(group, domain);
    if (captured) {
        PioneerDDJREV1.slicerLoopStartSamples[deckIdx] = captured.start;
        PioneerDDJREV1.slicerLoopEndSamples[deckIdx] = captured.end;
        PioneerDDJREV1.slicerLoopStartBeat[deckIdx] =
            PioneerDDJREV1.slicerBeatsFromSamplePos(group, captured.start);
    } else {
        PioneerDDJREV1.slicerLoopStartSamples[deckIdx] = -1;
        PioneerDDJREV1.slicerLoopEndSamples[deckIdx] = -1;
        PioneerDDJREV1.slicerLoopStartBeat[deckIdx] =
            PioneerDDJREV1.slicerLoopCaptureStartBeat(beatsExact, domain, group, bpm);
    }
    PioneerDDJREV1.slicerLoopDomain[deckIdx] = domain;
    PioneerDDJREV1.slicerPreviousRelBeats[deckIdx] =
        beatsExact - PioneerDDJREV1.slicerLoopStartBeat[deckIdx];
    const trackSamples = Number(engine.getValue(group, "track_samples"));
    PioneerDDJREV1.slicerPreviousPlaySample[deckIdx] =
        (trackSamples > 0) ? playposition * trackSamples : 0;
    PioneerDDJREV1.slicerLastPadKey[deckIdx] = -1;
    PioneerDDJREV1.slicerSyncKnownPlayposition(deckIdx, group);
};

PioneerDDJREV1.slicerResolveTargetBeat = function(ctx, domain, sliceIndex, sliceSize) {
    const sliceStart = Math.floor(sliceIndex * sliceSize);
    const targetBeat = ctx.sectionStart + sliceStart;
    let sliceEndBeat = targetBeat + sliceSize;
    if (ctx.loopConstrained && sliceEndBeat > ctx.loopEndBeat) {
        sliceEndBeat = ctx.loopEndBeat;
    }
    return {
        targetBeat: targetBeat,
        sliceEndBeat: sliceEndBeat,
        sectionStart: ctx.sectionStart,
    };
};

PioneerDDJREV1.slicerBeatsExact = function(playposition, duration, bpm) {
    return (playposition * duration) * (bpm / 60);
};

PioneerDDJREV1.slicerComputeSliceTarget = function(deckIdx, padIndex, useShiftRow, beatsExact, domain, group, bpm) {
    const duration = engine.getValue(group, "duration");
    const ctx = PioneerDDJREV1.slicerSectionContext(deckIdx, group, beatsExact, domain, bpm);
    const padKey = padIndex + (useShiftRow ? 8 : 0);
    let sliceIndex;
    let sliceSize;
    const slicePattern = PioneerDDJREV1.beatSlicerSlicePatternName();
    const patternTable = PioneerDDJREV1.beatSlicerPatternTable();
    const sliceTables = {
        linear: [0, 1, 2, 3, 4, 5, 6, 7],        loop: [0, 1, 2, 3, 7, 6, 5, 4],
        quantize_loop: [0, 1, 2, 3, 7, 6, 5, 4],
    };
    sliceIndex = (sliceTables[slicePattern] || patternTable)[padIndex];
    sliceSize = domain / 8;
    if (padKey === PioneerDDJREV1.slicerLastPadKey[deckIdx]) {
        if (ctx.loopConstrained && PioneerDDJREV1.slicerHasValidLoopSamples(deckIdx)) {
            const replayRatio = PioneerDDJREV1.slicerSampleRatio(
                group, PioneerDDJREV1.slicerLastPadTargetSample[deckIdx]);
            return {
                targetBeat: PioneerDDJREV1.slicerBeatsExact(replayRatio, duration, bpm),
                sliceEndBeat: PioneerDDJREV1.slicerBeatsExact(
                    PioneerDDJREV1.slicerSampleRatio(group, PioneerDDJREV1.slicerLastPadSliceEndSample[deckIdx]),
                    duration, bpm),
                sectionStart: ctx.sectionStart,
                targetRatio: replayRatio,
                sliceEndSec: (PioneerDDJREV1.slicerLastPadSliceEndSample[deckIdx] /
                    Number(engine.getValue(group, "track_samples"))) * duration,
                usesLoopSamples: true,
            };
        }
        const replayBeat = PioneerDDJREV1.slicerLastPadTargetBeat[deckIdx];
        const replaySection = PioneerDDJREV1.slicerLastPadSection[deckIdx];
        let sliceEndBeat = replayBeat + sliceSize;
        if (ctx.loopConstrained && sliceEndBeat > replaySection + domain) {
            sliceEndBeat = replaySection + domain;
        }
        return {
            targetBeat: replayBeat,
            sliceEndBeat: sliceEndBeat,
            sectionStart: replaySection,
        };
    }
    let result;
    if (ctx.loopConstrained && PioneerDDJREV1.slicerHasValidLoopSamples(deckIdx)) {
        const sampleTarget = PioneerDDJREV1.slicerResolveLoopSliceTarget(
            deckIdx, sliceIndex, group, duration);
        result = {
            targetBeat: PioneerDDJREV1.slicerBeatsExact(sampleTarget.targetRatio, duration, bpm),
            sliceEndBeat: PioneerDDJREV1.slicerBeatsExact(
                PioneerDDJREV1.slicerSampleRatio(group, sampleTarget.sliceEndSample), duration, bpm),
            sectionStart: ctx.sectionStart,
            targetRatio: sampleTarget.targetRatio,
            sliceEndSec: sampleTarget.sliceEndSec,
            targetSample: sampleTarget.targetSample,
            sliceEndSample: sampleTarget.sliceEndSample,
            usesLoopSamples: true,
        };
    } else {
        result = PioneerDDJREV1.slicerResolveTargetBeat(ctx, domain, sliceIndex, sliceSize);
    }
    PioneerDDJREV1.slicerLastPadKey[deckIdx] = padKey;
    PioneerDDJREV1.slicerLastPadSection[deckIdx] = ctx.sectionStart;
    PioneerDDJREV1.slicerLastPadTargetBeat[deckIdx] = result.targetBeat;
    if (result.usesLoopSamples) {
        PioneerDDJREV1.slicerLastPadTargetSample[deckIdx] = result.targetSample;
        PioneerDDJREV1.slicerLastPadSliceEndSample[deckIdx] = result.sliceEndSample;
    }
    return result;
};

PioneerDDJREV1.slicerScheduleReleaseStop = function(deckNum, deckIdx) {
    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerReleaseStopTimer, deckIdx);
    PioneerDDJREV1.slicerReleaseStopTimer[deckIdx] = engine.beginTimer(
        PioneerDDJREV1.slicerReleaseStopDebounceMs,
        function() {
            PioneerDDJREV1.slicerReleaseStopTimer[deckIdx] = null;
            PioneerDDJREV1.slicerStopHeldPlayback(deckNum, deckIdx);
            PioneerDDJREV1.slicerActive[deckIdx] = false;
            PioneerDDJREV1.slicerButton[deckIdx] = 0;
        },
        true
    );
};

PioneerDDJREV1.slicerStopHeldPlayback = function(deckNum, deckIdx) {
    const g = "[Channel" + deckNum + "]";
    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerHoldStopTimer, deckIdx);
    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerReleaseStopTimer, deckIdx);
    PioneerDDJREV1.slicerHeldByUser[deckIdx] = false;
    if (PioneerDDJREV1.slicerStartedStoppedDeck[deckIdx]) {
        try {
            engine.setValue(g, "play", 0);
        } catch (e) {
            // ignore
        }
        PioneerDDJREV1.slicerStartedStoppedDeck[deckIdx] = false;
    }
};

PioneerDDJREV1.slicerScheduleSliceEndWatch = function(deckNum, deckIdx, sliceEndBeat, bpm, duration) {
    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerHoldStopTimer, deckIdx);
    if (!(bpm > 0) || !(duration > 0)) {
        return;
    }
    const sliceEndSec = (sliceEndBeat * 60) / bpm;
    PioneerDDJREV1.slicerHoldStopTimer[deckIdx] = engine.beginTimer(25, function() {
        const g = "[Channel" + deckNum + "]";
        if (!PioneerDDJREV1.slicerHeldByUser[deckIdx] && !PioneerDDJREV1.slicerStartedStoppedDeck[deckIdx]) {
            PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerHoldStopTimer, deckIdx);
            return;
        }
        const playing = engine.getValue(g, "play") > 0;
        if (!playing) {
            PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerHoldStopTimer, deckIdx);
            return;
        }
        const pos = engine.getValue(g, "playposition") * duration;
        if (pos >= sliceEndSec - 0.002) {
            PioneerDDJREV1.slicerStopHeldPlayback(deckNum, deckIdx);
            PioneerDDJREV1.slicerActive[deckIdx] = false;
            PioneerDDJREV1.slicerButton[deckIdx] = 0;
        }
    });
};

PioneerDDJREV1.slicerSeekToTarget = function(deckNum, deckIdx, target, bpm, duration, padIndex, useShiftRow) {
    const g = "[Channel" + deckNum + "]";
    let targetRatio;
    if (target.usesLoopSamples && target.targetRatio != null) {
        targetRatio = target.targetRatio;
    } else {
        targetRatio = (target.targetBeat * 60 / bpm) / duration;
    }
    const wasPlaying = engine.getValue(g, "play") > 0;
    const auditioning = PioneerDDJREV1.slicerStartedStoppedDeck[deckIdx];

    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerReleaseStopTimer, deckIdx);

    if (!wasPlaying || auditioning) {
        PioneerDDJREV1.slicerStartedStoppedDeck[deckIdx] = true;
        engine.setValue(g, "play", 1);
    }
    PioneerDDJREV1.slicerSuppressSeekInvalidate[deckIdx] = true;
    engine.setValue(g, "playposition", targetRatio);
    PioneerDDJREV1.slicerSuppressSeekInvalidate[deckIdx] = false;

    if (!wasPlaying || auditioning) {
        PioneerDDJREV1.slicerHeldByUser[deckIdx] = true;
        PioneerDDJREV1.slicerHeldSliceEndBeat[deckIdx] = target.sliceEndBeat;
        if (target.sliceEndSec != null) {
            PioneerDDJREV1.slicerScheduleSliceEndWatchSec(deckNum, deckIdx, target.sliceEndSec);
        } else {
            PioneerDDJREV1.slicerScheduleSliceEndWatch(
                deckNum, deckIdx, target.sliceEndBeat, bpm, duration);
        }
    }
};

PioneerDDJREV1.slicerScheduleSliceEndWatchSec = function(deckNum, deckIdx, sliceEndSec) {
    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerHoldStopTimer, deckIdx);
    PioneerDDJREV1.slicerHoldStopTimer[deckIdx] = engine.beginTimer(25, function() {
        const g = "[Channel" + deckNum + "]";
        if (!PioneerDDJREV1.slicerHeldByUser[deckIdx] && !PioneerDDJREV1.slicerStartedStoppedDeck[deckIdx]) {
            PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerHoldStopTimer, deckIdx);
            return;
        }
        const playing = engine.getValue(g, "play") > 0;
        if (!playing) {
            PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerHoldStopTimer, deckIdx);
            return;
        }
        const duration = engine.getValue(g, "duration");
        const pos = engine.getValue(g, "playposition") * duration;
        if (pos >= sliceEndSec - 0.002) {
            PioneerDDJREV1.slicerStopHeldPlayback(deckNum, deckIdx);
            PioneerDDJREV1.slicerActive[deckIdx] = false;
            PioneerDDJREV1.slicerButton[deckIdx] = 0;
        }
    });
};

PioneerDDJREV1.slicerApplyPendingLoopActivate = function(deckNum, deckIdx, group) {
    const pending = PioneerDDJREV1.slicerPendingLoopActivate[deckIdx];
    if (!pending) {
        return;
    }
    PioneerDDJREV1.slicerPendingLoopActivate[deckIdx] = null;
    PioneerDDJREV1.slicerCaptureIntendedLoopRegion(deckNum, deckIdx, group);
};

PioneerDDJREV1.slicerActivateDomainLoop = function(deckGroup, deckIdx, deckNum) {
    const dom = PioneerDDJREV1.selectedSlicerDomain[deckIdx];
    const deck = deckNum || (deckIdx + 1);
    if (PioneerDDJREV1.beatSlicerQuantizeSection(deckGroup)) {
        PioneerDDJREV1.slicerPendingLoopActivate[deckIdx] = { domain: dom, };
        return;
    }
    PioneerDDJREV1.slicerCaptureIntendedLoopRegion(deck, deckIdx, deckGroup);
};

PioneerDDJREV1.slicerClearLoopVisualState = function(deckIdx, _deckNum) {
    PioneerDDJREV1.slicerLoopStartBeat[deckIdx] = 0;
    PioneerDDJREV1.slicerLoopStartSamples[deckIdx] = -1;
    PioneerDDJREV1.slicerLoopEndSamples[deckIdx] = -1;
    PioneerDDJREV1.slicerLoopDomain[deckIdx] = PioneerDDJREV1.selectedSlicerDomain[deckIdx];
    PioneerDDJREV1.slicerPreviousRelBeats[deckIdx] = 0;
    PioneerDDJREV1.slicerPreviousPlaySample[deckIdx] = 0;
};

PioneerDDJREV1.pianoRollScaleUsesAnchor = function(scaleName) {
    return scaleName !== "playthrough";
};

PioneerDDJREV1.pianoRollHotcuePosition = function(cueIndex) {
    if (cueIndex < 1 || cueIndex > 8) {
        return -1;
    }
    return engine.getValue("[Channel1]", "hotcue_" + cueIndex + "_position");
};

PioneerDDJREV1.pianoRollHotcueExists = function(cueIndex) {
    return PioneerDDJREV1.pianoRollHotcuePosition(cueIndex) >= 0;
};

PioneerDDJREV1.pianoRollAnchorRatioFromHotcue = function(cueIndex) {
    const group = "[Channel1]";
    const trackSamples = engine.getValue(group, "track_samples");
    const pos = PioneerDDJREV1.pianoRollHotcuePosition(cueIndex);
    if (!(trackSamples > 0) || pos < 0) {
        return null;
    }
    return pos / trackSamples;
};

PioneerDDJREV1.pianoRollResolveAnchorRatio = function() {
    if (PioneerDDJREV1.pianoRollHotcueOverride > 0) {
        const r = PioneerDDJREV1.pianoRollAnchorRatioFromHotcue(PioneerDDJREV1.pianoRollHotcueOverride);
        if (r !== null) {
            return r;
        }
    }
    return PioneerDDJREV1.pianoRollAnchorRatio;
};

PioneerDDJREV1.pianoRollSeekToAnchor = function() {
    const group = "[Channel1]";
    const duration = Number(engine.getValue(group, "duration"));
    if (!(duration > 0)) {
        return;
    }
    const ratio = PioneerDDJREV1.pianoRollResolveAnchorRatio();
    if (ratio < 0 || ratio > 1) {
        return;
    }
    try {
        engine.setValue(group, "playposition", ratio);
    } catch (e) {
        // ignore
    }
};

/** First pad (or Play-then-first-pad) captures anchor; later pads in the same phrase do not re-anchor. */
PioneerDDJREV1.pianoRollCaptureAnchorIfNeeded = function(forceFromPlayhead) {
    const group = "[Channel1]";
    if (PioneerDDJREV1.pianoRollHotcueOverride > 0) {
        const r = PioneerDDJREV1.pianoRollAnchorRatioFromHotcue(PioneerDDJREV1.pianoRollHotcueOverride);
        if (r !== null) {
            PioneerDDJREV1.pianoRollAnchorRatio = r;
            PioneerDDJREV1.pianoRollAnchorSet = true;
        }
        PioneerDDJREV1.pianoRollArmedForPlay = false;
        return;
    }
    if (forceFromPlayhead || PioneerDDJREV1.pianoRollArmedForPlay || !PioneerDDJREV1.pianoRollAnchorSet) {
        PioneerDDJREV1.pianoRollAnchorRatio = engine.getValue(group, "playposition");
        PioneerDDJREV1.pianoRollAnchorSet = true;
        PioneerDDJREV1.pianoRollArmedForPlay = false;
    }
};

PioneerDDJREV1.pianoRollClearPlaybackFlags = function() {
    PioneerDDJREV1.pianoRollArmedForPlay = false;
    PioneerDDJREV1.pianoRollLatchedPlay = false;
    PioneerDDJREV1.pianoRollLatchOnNextPlay = false;
};

PioneerDDJREV1.pianoRollStopOnTeardown = function() {
    return PioneerDDJREV1.pianoRollHeldCount > 0
        || PioneerDDJREV1.pianoRollHotcueOverride > 0;
};

PioneerDDJREV1.pianoRollRestorePitchOnTeardown = function() {
    if (PioneerDDJREV1.pianoRollStopOnTeardown()) {
        return true;
    }
    if (PioneerDDJREV1.pianoRollPitchDeck[0] !== 0) {
        return true;
    }
    const pitchAdjust = Number(engine.getValue("[Channel1]", "pitch_adjust"));
    return Number.isFinite(pitchAdjust) && Math.abs(pitchAdjust) > 1e-6;
};

PioneerDDJREV1.pianoRollResetChannel1Audio = function() {
    const stopPlay = PioneerDDJREV1.pianoRollStopOnTeardown();
    const restorePitch = PioneerDDJREV1.pianoRollRestorePitchOnTeardown();
    if (!stopPlay && !restorePitch) {
        PioneerDDJREV1.pianoRollClearPlaybackFlags();
        return;
    }
    try {
        if (stopPlay) {
            engine.setValue("[Channel1]", "play", 0);
        }
            //Reset pitch only after last deck leaves piano.
        if (restorePitch && PioneerDDJREV1.pianoRollPitchDeck[0] !== 0) {
            engine.setValue("[Channel1]", "keylock",
                PioneerDDJREV1.pianoRollSavedKeylock ? 1 : 0);
            engine.setValue("[Channel1]", "pitch_adjust", 0);
        }
    } catch (e) {
        // ignore
    }
    PioneerDDJREV1.pianoRollPitchDeck[0] = 0;
    PioneerDDJREV1.pianoRollHeldCount = 0;
    PioneerDDJREV1.pianoRollClearPlaybackFlags();
};

PioneerDDJREV1.pianoRollClearGhostHold = function() {
    if (PioneerDDJREV1.pianoRollHeldCount > 0) {
        PioneerDDJREV1.pianoRollHeldCount = 0;
        PioneerDDJREV1.pianoRollLatchOnNextPlay = false;
    }
};

/** Deck 1 rolling from Play (not pad-hold, not latched). Next piano pad stop transport first. */
PioneerDDJREV1.pianoRollIsPlayingFromPlayButton = function(group) {
    group = group || "[Channel1]";
    return PioneerDDJREV1.pianoRollArmedForPlay
        && PioneerDDJREV1.pianoRollHeldCount === 0
        && !PioneerDDJREV1.pianoRollLatchedPlay
        && engine.getValue(group, "play") > 0;
};

PioneerDDJREV1.pianoRollToggleHotcueOverride = function(cueIndex) {
    if (!PioneerDDJREV1.pianoRollHotcueExists(cueIndex)) {
        return;
    }
    if (PioneerDDJREV1.pianoRollHotcueOverride === cueIndex) {
        PioneerDDJREV1.pianoRollHotcueOverride = 0;
        PioneerDDJREV1.pianoRollAnchorSet = false;
        PioneerDDJREV1.stopPianoRollShiftBlinkTimer();
    } else {
        PioneerDDJREV1.pianoRollHotcueOverride = cueIndex;
        const r = PioneerDDJREV1.pianoRollAnchorRatioFromHotcue(cueIndex);
        if (r !== null) {
            PioneerDDJREV1.pianoRollAnchorRatio = r;
        }
        PioneerDDJREV1.pianoRollAnchorSet = true;
        PioneerDDJREV1.pianoRollArmedForPlay = false;
        PioneerDDJREV1.startPianoRollShiftBlinkTimer();
    }
    PioneerDDJREV1.pianoRollRefreshAnchorShiftLeds();
};

PioneerDDJREV1.pianoRollRefreshAnchorShiftLeds = function() {
    for (let d = 1; d <= 4; d++) {
        const idx = d - 1;
        if (PioneerDDJREV1.padRowMode[idx] === 3 && PioneerDDJREV1.pianoRollEngaged[idx]) {
            PioneerDDJREV1.refreshAutoloopRowLedsDeck(d);
        }
    }
};

PioneerDDJREV1.pianoRollPlaypositionCallback = function(_value, group, _control) {
    if (group !== "[Channel1]") {
        return;
    }
    if (!PioneerDDJREV1.pianoRollEngaged.some(Boolean)) {
        return;
    }
    if (!PioneerDDJREV1.pianoRollScaleUsesAnchor(PioneerDDJREV1.pianoRollScale)) {
        return;
    }
    if (PioneerDDJREV1.pianoRollHotcueOverride > 0) {
        return;
    }
    if (engine.getValue(group, "play") > 0) {
        return;
    }
    PioneerDDJREV1.pianoRollAnchorRatio = engine.getValue(group, "playposition");
    if (!PioneerDDJREV1.pianoRollAnchorSet) {
        PioneerDDJREV1.pianoRollAnchorSet = true;
    }
};

PioneerDDJREV1.unregisterPianoRollPlaypositionConnection = function() {
    const c = PioneerDDJREV1.pianoRollPlaypositionConnection;
    if (c && typeof c.disconnect === "function") {
        c.disconnect();
    }
    PioneerDDJREV1.pianoRollPlaypositionConnection = null;
};

PioneerDDJREV1.registerPianoRollPlaypositionConnection = function() {
    PioneerDDJREV1.unregisterPianoRollPlaypositionConnection();
    const conn = engine.makeConnection("[Channel1]", "playposition",
        PioneerDDJREV1.pianoRollPlaypositionCallback);
    if (conn) {
        PioneerDDJREV1.pianoRollPlaypositionConnection = conn;
    }
};

PioneerDDJREV1.stopPianoRollShiftBlinkTimer = function() {
    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1, "pianoRollShiftBlinkTimer");
    PioneerDDJREV1.pianoRollShiftBlinkOn = false;
};

PioneerDDJREV1.startPianoRollShiftBlinkTimer = function() {
    PioneerDDJREV1.stopPianoRollShiftBlinkTimer();
    if (PioneerDDJREV1.pianoRollHotcueOverride <= 0) {
        return;
    }
    const tick = function() {
        PioneerDDJREV1.pianoRollShiftBlinkOn = !PioneerDDJREV1.pianoRollShiftBlinkOn;
        PioneerDDJREV1.pianoRollRefreshAnchorShiftLeds();
    };
    PioneerDDJREV1.pianoRollShiftBlinkTimer = engine.beginTimer(400, tick, false);
};

PioneerDDJREV1.pianoRollResetAnchorState = function() {
    const group = "[Channel1]";
    PioneerDDJREV1.pianoRollAnchorRatio = engine.getValue(group, "playposition");
    PioneerDDJREV1.pianoRollAnchorSet = false;
    PioneerDDJREV1.pianoRollHotcueOverride = 0;
    PioneerDDJREV1.pianoRollClearPlaybackFlags();
    PioneerDDJREV1.stopPianoRollShiftBlinkTimer();
};

PioneerDDJREV1.pianoRollKeysForScale = function(scaleName) {
    const scales = {
        major: {
            bottom: [0, 2, 4, 5, 7, 9, 11, 12],
            top: [null, 1, 3, null, 6, 8, 10, null],
        },
        minor: {
            bottom: [0, 2, 3, 5, 7, 8, 10, 12],
            top: [null, 1, null, 4, 6, null, 9, null],
        },
        playthrough: {
            bottom: [0, 2, 4, 5, 7, 9, 11, 12],
            top: [null, 1, 3, null, 6, 8, 10, null],
            rightBottom: [12, 14, 16, 17, 19, 21, 23, 24],
            rightTop: [null, 13, 15, null, 18, 20, 22, null],
        },
    };
    return scales[scaleName] || scales.major;
};

PioneerDDJREV1.exitPianoRoll = function() {
    PioneerDDJREV1.unregisterPianoRollPlaypositionConnection();
    PioneerDDJREV1.stopPianoRollShiftBlinkTimer();
    // Reset deck 1 audio state — all pads target deck 1 regardless of active deck.
    let anyEngaged = false;
    for (let idx = 0; idx < 4; idx++) {
        if (PioneerDDJREV1.pianoRollEngaged[idx]) {
            PioneerDDJREV1.pianoRollEngaged[idx] = false;
            anyEngaged = true;
        }
    }
    if (anyEngaged) {
        PioneerDDJREV1.pianoRollResetChannel1Audio();
    }
    for (let d = 1; d <= 4; d++) {
        PioneerDDJREV1.refreshAutoloopRowLedsDeck(d);
    }
};

PioneerDDJREV1.pianoRollPadInput = function(deckNum, control, value, status) {
    const group = "[Channel1]";
    const isRightSide = PioneerDDJREV1.pianoRollIsRightSideDeck(deckNum);
    const scaleName = PioneerDDJREV1.pianoRollScale;
    const padIndex = (control & 0xFF) - 0x10;
    if (padIndex < 0 || padIndex > 7) {
        return;
    }
    const useShiftRow = PioneerDDJREV1.isAutoloopShiftStatus(status & 0xFF);
    const stLed = useShiftRow
        ? PioneerDDJREV1.autoloopShiftStatusForDeckNum(deckNum)
        : PioneerDDJREV1.autoloopStatusForDeckNum(deckNum);

    // Play Through: pitch only — full shift rows, no anchor/hotcue logic.
    if (scaleName === "playthrough") {
        const maps = PioneerDDJREV1.pianoRollKeysForScale(scaleName);
        let semis;
        if (isRightSide) {
            semis = useShiftRow
                ? (maps.rightTop || [])[padIndex]
                : (maps.rightBottom || [])[padIndex];
        } else {
            semis = useShiftRow ? maps.top[padIndex] : maps.bottom[padIndex];
        }
        if (semis === null || semis === undefined) {
            if (value > 0) {
                midi.sendShortMsg(stLed, control & 0xFF, PioneerDDJREV1.LED_OFF);
            }
            return;
        }
        if (value > 0) {
            try {
                engine.setValue(group, "keylock", 1);
                engine.setValue(group, "pitch_adjust", semis);
            } catch (e) {
                // ignore
            }
            PioneerDDJREV1.pianoRollPitchDeck[0] = semis;
            midi.sendShortMsg(stLed, control & 0xFF, PioneerDDJREV1.LED_OFF);
        } else {
            midi.sendShortMsg(stLed, control & 0xFF, PioneerDDJREV1.LED_ON);
        }
        return;
    }

    // Mixxxed piano mode only: SHIFT + Auto Loop pad toggles hotcue anchor override (deck 1).
    if (useShiftRow) {
        if (value > 0) {
            PioneerDDJREV1.pianoRollToggleHotcueOverride(padIndex + 1);
        }
        return;
    }

    const maps = PioneerDDJREV1.pianoRollKeysForScale(scaleName);
    let semis = null;
    if (scaleName === "realfeel") {
        semis = isRightSide
            ? (maps.rightBottom || [])[padIndex]
            : maps.bottom[padIndex];
    } else if (PioneerDDJREV1.pianoRollUsesPentatonicLayout(scaleName)) {
        semis = isRightSide
            ? (maps.rightBottom || [])[padIndex]
            : maps.bottom[padIndex];
    } else if (PioneerDDJREV1.pianoRollUsesTwoRowDeckSplit(scaleName)) {
        semis = isRightSide ? maps.top[padIndex] : maps.bottom[padIndex];
    } else {
        semis = isRightSide ? (maps.rightBottom || [])[padIndex] : maps.bottom[padIndex];
    }

    if (semis === null || semis === undefined) {
        if (value > 0) {
            midi.sendShortMsg(stLed, control & 0xFF, PioneerDDJREV1.LED_OFF);
        }
        return;
    }

    const semitone = Math.max(-12, Math.min(12, semis));

    if (value > 0) {
        try {
            engine.setValue(group, "keylock", 1);
            engine.setValue(group, "pitch_adjust", semitone);
        } catch (e) {
            // ignore
        }
        PioneerDDJREV1.pianoRollPitchDeck[0] = semitone;

        try {
            let forceNewAnchor = false;
            if (PioneerDDJREV1.pianoRollLatchedPlay && engine.getValue(group, "play") > 0) {
                engine.setValue(group, "play", 0);
                PioneerDDJREV1.pianoRollLatchedPlay = false;
                forceNewAnchor = true;
            }
            if (PioneerDDJREV1.pianoRollIsPlayingFromPlayButton(group)) {
                engine.setValue(group, "play", 0);
                PioneerDDJREV1.pianoRollArmedForPlay = false;
                forceNewAnchor = true;
            }
            PioneerDDJREV1.pianoRollCaptureAnchorIfNeeded(forceNewAnchor);
            PioneerDDJREV1.pianoRollSeekToAnchor();
            engine.setValue(group, "play", 1);
        } catch (e2) {
            // ignore
        }

        PioneerDDJREV1.pianoRollHeldCount++;
        PioneerDDJREV1.pianoRollLatchOnNextPlay = true;
        midi.sendShortMsg(stLed, control & 0xFF, PioneerDDJREV1.LED_OFF);
    } else {
        PioneerDDJREV1.pianoRollHeldCount = Math.max(0, PioneerDDJREV1.pianoRollHeldCount - 1);
        if (PioneerDDJREV1.pianoRollHeldCount === 0) {
            try {
                if (!PioneerDDJREV1.pianoRollLatchedPlay) {
                    engine.setValue(group, "play", 0);
                    PioneerDDJREV1.pianoRollSeekToAnchor();
                    PioneerDDJREV1.pianoRollPitchDeck[0] = 0;
                    PioneerDDJREV1.pianoRollClearPlaybackFlags();
                }
            } catch (e3) {
                // ignore
            }
        }
        midi.sendShortMsg(stLed, control & 0xFF, PioneerDDJREV1.LED_ON);
    }
};

//
// Mixxxed-Mode: autoloop row, BPM / mode cycle, slicer, Piano Roll (3), scratch bank mode 4
//

PioneerDDJREV1.autoloopStatusForDeckNum = function(deckNum) {
    const map = {1: 0x97, 2: 0x99, 3: 0x9B, 4: 0x9D, };
    return map[deckNum] || 0x97;
};

PioneerDDJREV1.autoloopShiftStatusForDeckNum = function(deckNum) {
    const map = {1: 0x98, 2: 0x9A, 3: 0x9C, 4: 0x9E, };
    return map[deckNum] || 0x98;
};

PioneerDDJREV1.isAutoloopShiftStatus = function(st) {
    const s = st & 0xFF;
    return s === 0x98 || s === 0x9A || s === 0x9C || s === 0x9E;
};

PioneerDDJREV1.clearPadRow = function(status) {
    for (let n = 0x10; n <= 0x17; n++) {
        midi.sendShortMsg(status, n, PioneerDDJREV1.LED_OFF);
    }
};

PioneerDDJREV1.clearAutoloopShiftRowLedsDeck = function(deckNum) {
    PioneerDDJREV1.clearPadRow(PioneerDDJREV1.autoloopShiftStatusForDeckNum(deckNum));
};

PioneerDDJREV1.unregisterAutoloopLedConnections = function() {
    const conns = PioneerDDJREV1.autoloopLedConnections;
    for (let i = 0; i < conns.length; i++) {
        const c = conns[i];
        if (c && typeof c.disconnect === "function") {
            c.disconnect();
        }
    }
    PioneerDDJREV1.autoloopLedConnections = [];
};

PioneerDDJREV1.autoloopLedCallback = function(value, group, control) {
    const deck = script.deckFromGroup(group);
    if (!deck) {
        return;
    }
    const idx = deck - 1;
    if (PioneerDDJREV1.mixxxedModeEnabled && PioneerDDJREV1.padRowMode[idx] !== 1) {
        return;
    }
    // Configurable autoloop sizes: drive LEDs from loop_enabled + beatloop_size.
    // control is ignored (it will be loop_enabled / beatloop_size / legacy beatloop_*_enabled).
    PioneerDDJREV1.refreshAutoloopRowLedsDeck(deck);
};

PioneerDDJREV1.registerAutoloopLedConnections = function() {
    PioneerDDJREV1.unregisterAutoloopLedConnections();
    for (let d = 1; d <= 4; d++) {
        const g = "[Channel" + d + "]";
        ["loop_enabled", "beatloop_size"].forEach(function(co) {
            const conn = engine.makeConnection(g, co, PioneerDDJREV1.autoloopLedCallback);
            if (conn) {
                PioneerDDJREV1.autoloopLedConnections.push(conn);
            }
        });
    }
};

PioneerDDJREV1.findAutoloopMatchPad = function(group) {
    if (engine.getValue(group, "loop_enabled") <= 0.5) {
        return -1;
    }
    const currentSize = Number(engine.getValue(group, "beatloop_size"));
    const size = Number.isFinite(currentSize) ? currentSize : 0;
    for (let i = 0; i < 8; i++) {
        const resolved = PioneerDDJREV1.autoLoopPadResolvedSizes[i] || PioneerDDJREV1.autoLoopDefaults[i];
        if (resolved && Math.abs(resolved - size) < 1e-6) {
            return i;
        }
    }
    return -1;
};

PioneerDDJREV1.sendPadRowDiff = function(status, cache, desired) {
    for (let i = 0; i < 8; i++) {
        if (cache[i] !== desired[i]) {
            cache[i] = desired[i];
            midi.sendShortMsg(status, 0x10 + i, desired[i]);
        }
    }
};

PioneerDDJREV1.refreshAutoloopRowLedsDeck = function(deckNum) {
    const idx = deckNum - 1;
    const g = "[Channel" + deckNum + "]";
    const st = PioneerDDJREV1.autoloopStatusForDeckNum(deckNum);
    const stShift = PioneerDDJREV1.autoloopShiftStatusForDeckNum(deckNum);
    const mainCache = PioneerDDJREV1.autoloopLedCache[idx];
    const shiftCache = PioneerDDJREV1.autoloopShiftLedCache[idx];
    const OFF = PioneerDDJREV1.LED_OFF;
    const ON = PioneerDDJREV1.LED_ON;
    if (!PioneerDDJREV1.mixxxedModeEnabled) {
        const desired = new Array(8).fill(OFF);
        const matchPad = PioneerDDJREV1.findAutoloopMatchPad(g);
        if (matchPad >= 0) {
            desired[matchPad] = ON;
        }
        PioneerDDJREV1.sendPadRowDiff(st, mainCache, desired);
        PioneerDDJREV1.sendPadRowDiff(stShift, shiftCache, new Array(8).fill(OFF));
        return;
    }
    const modeRow = PioneerDDJREV1.padRowMode[idx];
    const lockedRow = PioneerDDJREV1.MixxxedModeLocked[idx];
    if (PioneerDDJREV1.MixxxedModeLockHoldActive[idx]) {
        const m = PioneerDDJREV1.MixxxedModeLockHoldMode[idx];
        const col = Math.max(0, Math.min(3, (m || 1) - 1));
        const desired = new Array(8).fill(OFF);
        desired[col] = ON;
        PioneerDDJREV1.sendPadRowDiff(st, mainCache, desired);
        PioneerDDJREV1.sendPadRowDiff(stShift, shiftCache, new Array(8).fill(OFF));
        return;
    }
    if (modeRow === 1) {
        const desired = new Array(8).fill(OFF);
        const matchPad = PioneerDDJREV1.findAutoloopMatchPad(g);
        if (matchPad >= 0) {
            desired[matchPad] = ON;
        }
        PioneerDDJREV1.sendPadRowDiff(st, mainCache, desired);
        // Mode-1 indicator: keep SHIFT pad 5 (0x14) lit when mode 1 is selected.
        const shiftDesired = new Array(8).fill(OFF);
        shiftDesired[4] = ON;
        PioneerDDJREV1.sendPadRowDiff(stShift, shiftCache, shiftDesired);
        return;
    }
    if (modeRow === 2 && lockedRow) {
        PioneerDDJREV1.slicerRenderRowLedsDeck(deckNum, g, false);
        return;
    }
    if (!lockedRow && modeRow >= 1) {
        const col = modeRow - 1;
        const shiftDesired = new Array(8).fill(OFF);
        if (col >= 0 && col <= 3) {
            shiftDesired[col + 4] = ON;
        }
        PioneerDDJREV1.sendPadRowDiff(st, mainCache, new Array(8).fill(OFF));
        PioneerDDJREV1.sendPadRowDiff(stShift, shiftCache, shiftDesired);
        return;
    }
    PioneerDDJREV1.sendPadRowDiff(stShift, shiftCache, new Array(8).fill(OFF));
    if (PioneerDDJREV1.padRowMode[idx] === 4) {
        const chMap = { 1: 7, 2: 9, 3: 11, 4: 13, };
        const chKey = chMap[deckNum];
        const mapping = PioneerDDJREV1.Components.ScratchBank.resolveActiveSamplesForControlBase(chKey, 0x34);
        const desired = new Array(8).fill(OFF);
        for (let k = 0; k < 4; k++) {
            const sn = mapping.samples[0x34 + k];
            if (sn && engine.getValue("[Sampler" + sn + "]", "track_loaded")) {
                desired[k] = ON;
            }
        }
        PioneerDDJREV1.sendPadRowDiff(st, mainCache, desired);
        return;
    }
    if (PioneerDDJREV1.padRowMode[idx] === 3) {
        const scaleName = PioneerDDJREV1.pianoRollScale;
        const maps = PioneerDDJREV1.pianoRollKeysForScale(scaleName);
        const isRightSide = PioneerDDJREV1.pianoRollIsRightSideDeck(deckNum);
        const mainDesired = new Array(8).fill(OFF);
        const shiftDesired = new Array(8).fill(OFF);
        const anchorShiftLeds = PioneerDDJREV1.pianoRollScaleUsesAnchor(scaleName);
        const overrideCue = PioneerDDJREV1.pianoRollHotcueOverride;
        for (let pi = 0; pi < 8; pi++) {
            if (anchorShiftLeds) {
                const cueNum = pi + 1;
                if (!PioneerDDJREV1.pianoRollHotcueExists(cueNum)) {
                    shiftDesired[pi] = OFF;
                } else if (overrideCue === cueNum) {
                    shiftDesired[pi] = PioneerDDJREV1.pianoRollShiftBlinkOn ? ON : OFF;
                } else {
                    shiftDesired[pi] = ON;
                }
            }
            let bot = null;
            if (scaleName === "playthrough") {
                const bot = maps.bottom[pi];
                const top = maps.top[pi];
                const rbot = (maps.rightBottom || [])[pi];
                const rtop = (maps.rightTop || [])[pi];
                if (isRightSide) {
                    mainDesired[pi] = (rbot !== null && rbot !== undefined) ? ON : OFF;
                    shiftDesired[pi] = (rtop !== null && rtop !== undefined) ? ON : OFF;
                } else {
                    mainDesired[pi] = (bot !== null && bot !== undefined) ? ON : OFF;
                    shiftDesired[pi] = (top !== null && top !== undefined) ? ON : OFF;
                }
                continue;
            } else if (scaleName === "realfeel") {
                bot = isRightSide ? (maps.rightBottom || [])[pi] : maps.bottom[pi];
            } else if (PioneerDDJREV1.pianoRollUsesPentatonicLayout(scaleName)) {
                bot = isRightSide ? (maps.rightBottom || [])[pi] : maps.bottom[pi];
            } else if (PioneerDDJREV1.pianoRollUsesTwoRowDeckSplit(scaleName)) {
                bot = isRightSide ? maps.top[pi] : maps.bottom[pi];
            } else {
                bot = isRightSide ? (maps.rightBottom || [])[pi] : maps.bottom[pi];
            }
            mainDesired[pi] = (bot !== null && bot !== undefined) ? ON : OFF;
        }
        PioneerDDJREV1.sendPadRowDiff(st, mainCache, mainDesired);
        PioneerDDJREV1.sendPadRowDiff(stShift, shiftCache, shiftDesired);
        return;
    }
};

PioneerDDJREV1.unregisterSlicerBeatConnections = function() {
    const conns = PioneerDDJREV1.slicerBeatConnections;
    for (let i = 0; i < conns.length; i++) {
        const c = conns[i];
        if (c && typeof c.disconnect === "function") {
            c.disconnect();
        }
    }
    PioneerDDJREV1.slicerBeatConnections = [];
};

PioneerDDJREV1.unregisterSlicerPlaypositionConnections = function() {
    const conns = PioneerDDJREV1.slicerPlaypositionConnections;
    for (let i = 0; i < conns.length; i++) {
        const c = conns[i];
        if (c && typeof c.disconnect === "function") {
            c.disconnect();
        }
    }
    PioneerDDJREV1.slicerPlaypositionConnections = [];
};

PioneerDDJREV1.slicerRenderRowLedsDeck = function(deckNum, group, doFauxJump) {
    const deckIdx = deckNum - 1;
    const bpm = engine.getValue(group, "bpm");
    const playposition = engine.getValue(group, "playposition");
    const duration = engine.getValue(group, "duration");
    if (!(bpm > 0) || !(duration > 0)) {
        return;
    }
    const domain = PioneerDDJREV1.selectedSlicerDomain[deckIdx];
    const beatsExact = PioneerDDJREV1.slicerBeatsExact(playposition, duration, bpm);
    const ctx = PioneerDDJREV1.slicerSectionContext(deckIdx, group, beatsExact, domain, bpm);
    const stepsPerDomain = 8;
    const slicerPosInSection = Math.floor(ctx.posInDomain / (domain / stepsPerDomain));
    const patternTable = PioneerDDJREV1.beatSlicerPatternTable();
    let ledBeatState = true;
    if (ctx.modeLoop) {
        ledBeatState = false;
        if (doFauxJump) {
            PioneerDDJREV1.slicerFauxLoopJumpTick(deckNum, deckIdx, group);
        }
    }
    const st = PioneerDDJREV1.autoloopStatusForDeckNum(deckNum);
    const ledCache = PioneerDDJREV1.slicerLedState[deckIdx];
    const ON = PioneerDDJREV1.LED_ON;
    const OFF = PioneerDDJREV1.LED_OFF;
    for (let i = 0; i < 8; i++) {
        let lit;
        lit = (patternTable[i] === slicerPosInSection) ? ledBeatState : !ledBeatState;
        const midiVal = lit ? ON : OFF;
        const heldBtn = PioneerDDJREV1.slicerButton[deckIdx];
        if (PioneerDDJREV1.slicerActive[deckIdx] && (heldBtn === i || heldBtn === i + 8)) {
            continue;
        }
        if (ledCache[i] === midiVal) {
            continue;
        }
        ledCache[i] = midiVal;
        midi.sendShortMsg(st, 0x10 + i, midiVal);
    }
    const stShift = PioneerDDJREV1.autoloopShiftStatusForDeckNum(deckNum);
    const shiftCache = PioneerDDJREV1.slicerShiftLedState[deckIdx];
    const shiftDesired = new Array(8).fill(OFF);
    if (ctx.modeLoop) {
        shiftDesired[7] = ON;
    }
    PioneerDDJREV1.sendPadRowDiff(stShift, shiftCache, shiftDesired);
};

PioneerDDJREV1.slicerPlaypositionCallback = function(_value, group, _control) {
    const deck = script.deckFromGroup(group);
    if (!deck) {
        return;
    }
    const deckIdx = deck - 1;
    if (!PioneerDDJREV1.mixxxedModeEnabled || PioneerDDJREV1.padRowMode[deckIdx] !== 2 ||
            !PioneerDDJREV1.MixxxedModeLocked[deckIdx]) {
        return;
    }
    if (PioneerDDJREV1.MixxxedModeLockHoldActive[deckIdx]) {
        return;
    }
    const cacheCleared = PioneerDDJREV1.slicerInvalidatePadCacheIfSeeked(deckIdx, group);
    if (engine.getValue(group, "play") > 0) {
        return;
    }
    if (cacheCleared) {
        PioneerDDJREV1.slicerLedState[deckIdx].fill(-1);
    }
    PioneerDDJREV1.slicerRenderRowLedsDeck(deck, group, false);
    if (cacheCleared) {
        PioneerDDJREV1.refreshAutoloopRowLedsDeck(deck);
    }
};

PioneerDDJREV1.slicerPlayCallback = function(value, group, _control) {
    const deck = script.deckFromGroup(group);
    if (!deck) {
        return;
    }
    const deckIdx = deck - 1;
    PioneerDDJREV1.slicerSyncKnownPlayposition(deckIdx, group);
    if (value > 0) {
        return;
    }
    PioneerDDJREV1.slicerPlaypositionCallback(0, group, _control);
};

PioneerDDJREV1.registerSlicerBeatConnections = function() {
    PioneerDDJREV1.unregisterSlicerBeatConnections();
    PioneerDDJREV1.unregisterSlicerPlaypositionConnections();
    if (!PioneerDDJREV1.mixxxedModeEnabled) {
        return;
    }
    for (let d = 1; d <= 4; d++) {
        const g = "[Channel" + d + "]";
        const beatConn = engine.makeConnection(g, "beat_active", PioneerDDJREV1.slicerBeatLedCallback);
        if (beatConn) {
            PioneerDDJREV1.slicerBeatConnections.push(beatConn);
        }
        const posConn = engine.makeConnection(g, "playposition", PioneerDDJREV1.slicerPlaypositionCallback);
        if (posConn) {
            PioneerDDJREV1.slicerPlaypositionConnections.push(posConn);
        }
        const playConn = engine.makeConnection(g, "play", PioneerDDJREV1.slicerPlayCallback);
        if (playConn) {
            PioneerDDJREV1.slicerPlaypositionConnections.push(playConn);
        }
    }
};

PioneerDDJREV1.slicerBeatLedCallback = function(_value, group, _control) {
    const deck = script.deckFromGroup(group);
    if (!deck) {
        return;
    }
    const deckIdx = deck - 1;
    if (!PioneerDDJREV1.mixxxedModeEnabled || PioneerDDJREV1.padRowMode[deckIdx] !== 2 || !PioneerDDJREV1.MixxxedModeLocked[deckIdx]) {
        return;
    }
    if (PioneerDDJREV1.MixxxedModeLockHoldActive[deckIdx]) {
        return;
    }
    const bpm = engine.getValue(group, "bpm");
    const duration = engine.getValue(group, "duration");
    if (!(bpm > 0) || !(duration > 0)) {
        return;
    }
    // Quantize-loop anchor capture must run even when beatgrid is missing (LED chase bails below).
    PioneerDDJREV1.slicerApplyPendingLoopActivate(deck, deckIdx, group);

    const beatClosest = engine.getValue(group, "beat_closest");
    const beatNext = engine.getValue(group, "beat_next");
    if (beatClosest === beatNext) {
        return;
    }

    PioneerDDJREV1.slicerRenderRowLedsDeck(deck, group, true);
};

PioneerDDJREV1.slicerResetDeck = function(deckNum) {
    const deckIdx = deckNum - 1;
    if (deckIdx < 0 || deckIdx > 3) {
        return;
    }
    PioneerDDJREV1.slicerStopHeldPlayback(deckNum, deckIdx);
    PioneerDDJREV1.slicerPendingLoopActivate[deckIdx] = null;
    PioneerDDJREV1.slicerActive[deckIdx] = false;
    PioneerDDJREV1.slicerButton[deckIdx] = 0;
    PioneerDDJREV1.slicerLastPadKey[deckIdx] = -1;
    PioneerDDJREV1.slicerLastPadSection[deckIdx] = 0;
    PioneerDDJREV1.slicerLastPadTargetBeat[deckIdx] = 0;
    PioneerDDJREV1.slicerLastPadTargetSample[deckIdx] = 0;
    PioneerDDJREV1.slicerLastPadSliceEndSample[deckIdx] = 0;
    PioneerDDJREV1.slicerPreviousPlaySample[deckIdx] = 0;
    PioneerDDJREV1.slicerLoopStartSamples[deckIdx] = -1;
    PioneerDDJREV1.slicerLoopEndSamples[deckIdx] = -1;
    PioneerDDJREV1.slicerPreviousRelBeats[deckIdx] = 0;
    PioneerDDJREV1.slicerLastKnownPlayposition[deckIdx] = -1;
    PioneerDDJREV1.slicerSuppressSeekInvalidate[deckIdx] = false;
    PioneerDDJREV1.slicerClearLoopVisualState(deckIdx, deckNum);
};

PioneerDDJREV1.MixxxedModeSlicerPad = function(deckNum, control, value, useShiftRow) {
    useShiftRow = !!useShiftRow;
    const deckIdx = deckNum - 1;
    const g = "[Channel" + deckNum + "]";
    const padIndex = control - 0x10;
    if (padIndex < 0 || padIndex > 7) {
        return;
    }
    if (value <= 0) {
        if (PioneerDDJREV1.slicerStartedStoppedDeck[deckIdx]) {
            PioneerDDJREV1.slicerScheduleReleaseStop(deckNum, deckIdx);
        } else {
            PioneerDDJREV1.slicerActive[deckIdx] = false;
            PioneerDDJREV1.slicerButton[deckIdx] = 0;
            PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckNum);
        }
        return;
    }
    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.slicerReleaseStopTimer, deckIdx);
    const domain = PioneerDDJREV1.selectedSlicerDomain[deckIdx];
    const bpm = engine.getValue(g, "bpm");
    const duration = engine.getValue(g, "duration");
    const playposition = engine.getValue(g, "playposition");
    if (!(bpm > 0) || !(duration > 0)) {
        return;
    }
    const beatsExact = PioneerDDJREV1.slicerBeatsExact(playposition, duration, bpm);
    const target = PioneerDDJREV1.slicerComputeSliceTarget(
        deckIdx, padIndex, useShiftRow, beatsExact, domain, g, bpm);
    PioneerDDJREV1.slicerSeekToTarget(
        deckNum, deckIdx, target, bpm, duration, padIndex, useShiftRow);
    PioneerDDJREV1.slicerSyncKnownPlayposition(deckIdx, g);
    PioneerDDJREV1.slicerActive[deckIdx] = true;
    PioneerDDJREV1.slicerButton[deckIdx] = padIndex + (useShiftRow ? 8 : 0);
    PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckNum);
};

PioneerDDJREV1.MixxxedModeSlicerShiftPad = function(deckNum, control, value) {
    const deckIdx = deckNum - 1;
    const deckGroup = "[Channel" + deckNum + "]";
    const padIndex = control - 0x10;
    if (value <= 0) {
        return;
    }
    if (padIndex >= 6) {
        if (padIndex === 6) {
                const doms = PioneerDDJREV1.slicerDomains;
                const cur = PioneerDDJREV1.selectedSlicerDomain[deckIdx];
                let idx = doms.indexOf(cur);
                if (idx < 0) {
                    idx = 0;
                }
                idx = (idx + 1) % doms.length;
                PioneerDDJREV1.selectedSlicerDomain[deckIdx] = doms[idx];
                PioneerDDJREV1.slicerLastPadKey[deckIdx] = -1;
                const modeLoop = PioneerDDJREV1.activeSlicerMode[deckIdx] === PioneerDDJREV1.slicerModes.loopSlice;
                if (modeLoop && PioneerDDJREV1.slicerLoopVisual) {
                    PioneerDDJREV1.slicerActivateDomainLoop(deckGroup, deckIdx, deckNum);
                }
        } else {
            const cur = PioneerDDJREV1.activeSlicerMode[deckIdx];
            PioneerDDJREV1.activeSlicerMode[deckIdx] =
                cur === PioneerDDJREV1.slicerModes.contSlice
                    ? PioneerDDJREV1.slicerModes.loopSlice
                    : PioneerDDJREV1.slicerModes.contSlice;
            const modeLoop = PioneerDDJREV1.activeSlicerMode[deckIdx] === PioneerDDJREV1.slicerModes.loopSlice;
            if (modeLoop && PioneerDDJREV1.slicerLoopVisual) {
                PioneerDDJREV1.slicerActivateDomainLoop(deckGroup, deckIdx, deckNum);
            } else if (!modeLoop) {
                PioneerDDJREV1.slicerClearLoopVisualState(deckIdx, deckNum);
                PioneerDDJREV1.slicerPendingLoopActivate[deckIdx] = null;
            }
        }
        PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckNum);
        return;
    }
    PioneerDDJREV1.slicerActive[deckIdx] = false;
    PioneerDDJREV1.slicerButton[deckIdx] = 0;
    PioneerDDJREV1.slicerLedState[deckIdx].fill(-1);
    PioneerDDJREV1.autoloopLedCache[deckIdx].fill(-1);
    PioneerDDJREV1.autoloopShiftLedCache[deckIdx].fill(-1);
    PioneerDDJREV1.slicerClearLoopVisualState(deckIdx, deckNum);
    PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckNum);
};

PioneerDDJREV1.onPadRowModeChanged = function(deckNum, newMode) {
    const deckIdx = deckNum - 1;
    if (newMode !== 3 && PioneerDDJREV1.pianoRollEngaged[deckIdx]) {
        PioneerDDJREV1.pianoRollEngaged[deckIdx] = false;
        // Only reset deck 1 audio when no other deck still holds piano mode engaged.
        const anyStillEngaged = PioneerDDJREV1.pianoRollEngaged.some(Boolean);
        if (!anyStillEngaged) {
            PioneerDDJREV1.unregisterPianoRollPlaypositionConnection();
            PioneerDDJREV1.stopPianoRollShiftBlinkTimer();
            PioneerDDJREV1.pianoRollResetChannel1Audio();
        } else {
            PioneerDDJREV1.pianoRollHeldCount = 0;
            PioneerDDJREV1.pianoRollLatchOnNextPlay = false;
        }
    }
    PioneerDDJREV1.slicerResetDeck(deckNum);
    PioneerDDJREV1.slicerLedState[deckIdx].fill(-1);
    PioneerDDJREV1.slicerShiftLedState[deckIdx].fill(-1);
    PioneerDDJREV1.autoloopLedCache[deckIdx].fill(-1);
    PioneerDDJREV1.autoloopShiftLedCache[deckIdx].fill(-1);
    PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckNum);
};

PioneerDDJREV1.onMixxxedModeLocked = function(deckNum) {
    const idx = deckNum - 1;
    if (idx < 0 || idx > 3) {
        PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckNum);
        return;
    }
    if (PioneerDDJREV1.MixxxedModeLockHoldTimer[idx] !== null) {
        engine.stopTimer(PioneerDDJREV1.MixxxedModeLockHoldTimer[idx]);
        PioneerDDJREV1.MixxxedModeLockHoldTimer[idx] = null;
    }
    if (PioneerDDJREV1.padRowMode[idx] === 3) {
        const wasAnyEngaged = PioneerDDJREV1.pianoRollEngaged.some(Boolean);
        PioneerDDJREV1.pianoRollEngaged[idx] = true;
        PioneerDDJREV1.pianoRollHeldCount = 0;
        if (!wasAnyEngaged) {
            PioneerDDJREV1.pianoRollSavedKeylock = engine.getValue("[Channel1]", "keylock") > 0;
            PioneerDDJREV1.pianoRollResetAnchorState();
            PioneerDDJREV1.registerPianoRollPlaypositionConnection();
        }
    }
    PioneerDDJREV1.MixxxedModeLockHoldMode[idx] = PioneerDDJREV1.padRowMode[idx];
    PioneerDDJREV1.MixxxedModeLockHoldActive[idx] = true;
    PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckNum);
    PioneerDDJREV1.MixxxedModeLockHoldTimer[idx] = engine.beginTimer(400, function() {
        PioneerDDJREV1.MixxxedModeLockHoldTimer[idx] = null;
        PioneerDDJREV1.MixxxedModeLockHoldActive[idx] = false;
        PioneerDDJREV1.refreshAutoloopRowLedsDeck(deckNum);
    }, true);
};

PioneerDDJREV1.MixxxedModeOnShiftReleased = function() {
    if (!PioneerDDJREV1.mixxxedModeEnabled) {
        return;
    }
    for (let i = 0; i < 4; i++) {
        if (PioneerDDJREV1.MixxxedModeLocked[i]) {
            continue;
        }
        if (PioneerDDJREV1.MixxxedModeLockTimer[i] !== null) {
            engine.stopTimer(PioneerDDJREV1.MixxxedModeLockTimer[i]);
            PioneerDDJREV1.MixxxedModeLockTimer[i] = null;
        }
        PioneerDDJREV1.MixxxedModeLocked[i] = true;
        PioneerDDJREV1.onMixxxedModeLocked(i + 1);
    }
};

PioneerDDJREV1.autoloopPadInput = function(_channel, control, value, status, group) {
    const deck = script.deckFromGroup(group);
    if (!deck) {
        return;
    }
    const st = status & 0xFF;
    if (st !== 0x97 && st !== 0x98 &&
            st !== 0x99 && st !== 0x9A &&
            st !== 0x9B && st !== 0x9C &&
            st !== 0x9D && st !== 0x9E) {
        return;
    }
    const idx = deck - 1;
    const c = control & 0xFF;

    const applyAutoloopPad = () => {
        const padIndex = c - 0x10;
        if (padIndex < 0 || padIndex > 7) {
            return;
        }
        const cfg = PioneerDDJREV1.autoLoopConfigs[padIndex] || { mode: "fixed", value: PioneerDDJREV1.autoLoopDefaults[padIndex], };
        if (value && value > 0) {
            const loopEnabled = engine.getValue(group, "loop_enabled") > 0.5;
            const currentSize = Number(engine.getValue(group, "beatloop_size"));
            const fallbackSize = PioneerDDJREV1.autoLoopDefaults[padIndex] || 1;
            const baseSize = Number.isFinite(currentSize) && currentSize > 0 ? currentSize : fallbackSize;
            let nextSize = baseSize;
            if (cfg.mode === "relative") {
                nextSize = cfg.op === "half" ? (baseSize / 2) : (baseSize * 2);
            } else if (cfg.mode === "fixed") {
                nextSize = cfg.value;
            }
            nextSize = Math.max(1 / 32, Math.min(64, nextSize));
            PioneerDDJREV1.autoLoopPadResolvedSizes[padIndex] = nextSize;
            // If a loop is already running:
            // - pressing the same-size pad exits the loop
            // - pressing a different-size pad switches the loop size and keeps looping
            const epsilon = 1e-6;
            const isSameSize = Number.isFinite(currentSize) && Math.abs(currentSize - nextSize) < epsilon;
            if (loopEnabled && isSameSize) {
                engine.setValue(group, "loop_enabled", 0);
                return;
            }
            engine.setValue(group, "beatloop_size", nextSize);
            if (!loopEnabled) {
                // Start a new loop (toggle on).
                engine.setValue(group, "beatloop_activate", 1);
                engine.setValue(group, "beatloop_activate", 0);
                return;
            }
            // Switch to the new size while keeping loop active.
            // beatloop_activate is a toggle, so we re-apply by exiting then activating.
            engine.setValue(group, "loop_enabled", 0);
            engine.setValue(group, "beatloop_activate", 1);
            engine.setValue(group, "beatloop_activate", 0);
        }
    };

    if (!PioneerDDJREV1.mixxxedModeEnabled) {
        applyAutoloopPad();
        return;
    }
    const mode = PioneerDDJREV1.padRowMode[idx];
    const locked = PioneerDDJREV1.MixxxedModeLocked[idx];
    if (mode === 1) {
        if (!locked) {
            return;
        }
        applyAutoloopPad();
        return;
    }
    if (!locked) {
        return;
    }
    if (mode === 2) {
        if (PioneerDDJREV1.isAutoloopShiftStatus(st) || PioneerDDJREV1.shiftPressed) {
            PioneerDDJREV1.MixxxedModeSlicerShiftPad(deck, c, value);
        } else {
            PioneerDDJREV1.MixxxedModeSlicerPad(deck, c, value);
        }
        return;
    }
    if (mode === 3) {
        if (!PioneerDDJREV1.pianoRollEngaged[idx]) {
            return;
        }
        PioneerDDJREV1.pianoRollPadInput(deck, c, value, st);
        return;
    }
    if (mode === 4 && value > 0 && c >= 0x10 && c <= 0x13) {
        const chMap = { 1: 7, 2: 9, 3: 11, 4: 13, };
        PioneerDDJREV1._loadMixxxedModeInvocation = true;
        try {
            PioneerDDJREV1.Components.ScratchBank.loadScratchToDeck(
                chMap[deck],
                0x34 + (c - 0x10),
                value,
                st
            );
        } finally {
            PioneerDDJREV1._loadMixxxedModeInvocation = false;
        }
        return;
    }
};

PioneerDDJREV1.beatBpmOrMixxxedMode = function(_channel, control, value, status, _group) {
    const st = status & 0xFF;
    let deck;
    if (st === 0x94) {
        deck = PioneerDDJREV1.leftActiveDeck;
    } else if (st === 0x95) {
        deck = PioneerDDJREV1.rightActiveDeck;
    } else {
        return;
    }
    if (deck < 1 || deck > 4) {
        return;
    }
    const g = "[Channel" + deck + "]";
    if (!PioneerDDJREV1.mixxxedModeEnabled) {
        if (control === 0x06) {
            engine.setValue(g, "beats_adjust_faster", value > 0 ? 1 : 0);
        } else if (control === 0x07) {
            engine.setValue(g, "beats_adjust_slower", value > 0 ? 1 : 0);
        }
        return;
    }
    if (!value || value <= 0) {
        return;
    }
    if (control !== 0x06 && control !== 0x07) {
        return;
    }
    const idx = deck - 1;
    const prevMode = PioneerDDJREV1.padRowMode[idx];
    let m = prevMode;
    const maxMode = 4;
    if (control === 0x06) {
        m = m <= 1 ? maxMode : m - 1;
    } else {
        m = m >= maxMode ? 1 : m + 1;
    }
    PioneerDDJREV1.padRowMode[idx] = m;
    if (PioneerDDJREV1.MixxxedModeLockTimer[idx] !== null) {
        engine.stopTimer(PioneerDDJREV1.MixxxedModeLockTimer[idx]);
        PioneerDDJREV1.MixxxedModeLockTimer[idx] = null;
    }

    // onPadRowModeChanged handles per-deck piano-roll cleanup for mode 3.
    PioneerDDJREV1.MixxxedModeLocked[idx] = false;
    const deckNum = deck;
    PioneerDDJREV1.MixxxedModeLockTimer[idx] = engine.beginTimer(1500, function() {
        PioneerDDJREV1.MixxxedModeLockTimer[idx] = null;
        if (!PioneerDDJREV1.MixxxedModeLocked[idx]) {
            PioneerDDJREV1.MixxxedModeLocked[idx] = true;
            PioneerDDJREV1.onMixxxedModeLocked(deckNum);
        }
    }, true);
    PioneerDDJREV1.onPadRowModeChanged(deck, m);
};

PioneerDDJREV1.deckLayerInput = function(_channel, control, value, status, _group) {
    const st = status & 0xFF;
    const c = control & 0xFF;
    const stopHold = function(deckNum) {
        const idx = deckNum - 1;
        if (idx < 0 || idx > 3) {
            return;
        }
        if (PioneerDDJREV1.tempoRangeHoldTimers[idx] !== null) {
            engine.stopTimer(PioneerDDJREV1.tempoRangeHoldTimers[idx]);
            PioneerDDJREV1.tempoRangeHoldTimers[idx] = null;
        }
    };
    const deckNumForSelect = function() {
        if (c !== 0x3C) {
            return null;
        }
        if (st === 0x90) {
            return 1;
        }
        if (st === 0x91) {
            return 2;
        }
        if (st === 0x92) {
            return 3;
        }
        if (st === 0x93) {
            return 4;
        }
        return null;
    };

    const deckSelectNum = deckNumForSelect();
    if (deckSelectNum && value <= 0) {
        stopHold(deckSelectNum);
        return;
    }
    if (!value || value <= 0) {
        return;
    }
    if (st === 0x92 && c === 0x72) {
        PioneerDDJREV1.leftActiveDeck = PioneerDDJREV1.leftActiveDeck === 1 ? 3 : 1;
    } else if (st === 0x93 && c === 0x72) {
        PioneerDDJREV1.rightActiveDeck = PioneerDDJREV1.rightActiveDeck === 2 ? 4 : 2;
    } else if (st === 0x90 && c === 0x3C) {
        PioneerDDJREV1.leftActiveDeck = 1;
    } else if (st === 0x92 && c === 0x3C) {
        PioneerDDJREV1.leftActiveDeck = 3;
    } else if (st === 0x91 && c === 0x3C) {
        PioneerDDJREV1.rightActiveDeck = 2;
    } else if (st === 0x93 && c === 0x3C) {
        PioneerDDJREV1.rightActiveDeck = 4;
    }

    if (deckSelectNum && PioneerDDJREV1.shiftPressed) {
        const idx = deckSelectNum - 1;
        stopHold(deckSelectNum);
        const deckGroup = "[Channel" + deckSelectNum + "]";
        PioneerDDJREV1.tempoRangeHoldTimers[idx] = engine.beginTimer(1000, function() {
            PioneerDDJREV1.Components.invoke("transport", "cycleTempoRange", [1, deckGroup]);
        });
    }
};

PioneerDDJREV1.shiftLockFxUnitToggle = function(_channel, _control, value, status, _group) {
    script.midiDebug(_channel, _control, value, status, _group);

    const st = status & 0xFF;
    const unitGroup = (st === 0x94) ? "[EffectRack1_EffectUnit1]"
        : (st === 0x95) ? "[EffectRack1_EffectUnit2]" : null;
    if (!unitGroup) {
        return;
    }
    engine.setValue(unitGroup, "mix", value > 0 ? 1 : 0);
};

PioneerDDJREV1.MixxxedModeShutdown = function() {
    PioneerDDJREV1.exitPianoRoll();
    for (let i = 0; i < 4; i++) {
        if (PioneerDDJREV1.MixxxedModeLockTimer[i] !== null) {
            engine.stopTimer(PioneerDDJREV1.MixxxedModeLockTimer[i]);
            PioneerDDJREV1.MixxxedModeLockTimer[i] = null;
        }
        if (PioneerDDJREV1.MixxxedModeLockHoldTimer[i] !== null) {
            engine.stopTimer(PioneerDDJREV1.MixxxedModeLockHoldTimer[i]);
            PioneerDDJREV1.MixxxedModeLockHoldTimer[i] = null;
        }
        PioneerDDJREV1.padRowMode[i] = 1;
        PioneerDDJREV1.MixxxedModeLocked[i] = true;
        PioneerDDJREV1.MixxxedModeLockHoldActive[i] = false;
        PioneerDDJREV1.slicerResetDeck(i + 1);
        PioneerDDJREV1.slicerLedState[i].fill(-1);
        PioneerDDJREV1.slicerShiftLedState[i].fill(-1);
        PioneerDDJREV1.autoloopLedCache[i].fill(-1);
        PioneerDDJREV1.autoloopShiftLedCache[i].fill(-1);
        PioneerDDJREV1.slicerAlreadyJumped[i] = false;
        PioneerDDJREV1.slicerPreviousRelBeats[i] = 0;
    }
    PioneerDDJREV1._loadMixxxedModeInvocation = false;
    for (let d = 1; d <= 4; d++) {
        PioneerDDJREV1.clearAutoloopShiftRowLedsDeck(d);
    }
    PioneerDDJREV1.unregisterAutoloopLedConnections();
    PioneerDDJREV1.unregisterSlicerBeatConnections();
    PioneerDDJREV1.unregisterSlicerPlaypositionConnections();
};

//
// Init
//

PioneerDDJREV1.init = function () {
    PioneerDDJREV1.MixxxedModeShutdown();
    PioneerDDJREV1.leftActiveDeck = 1;
    PioneerDDJREV1.rightActiveDeck = 2;
    PioneerDDJREV1.Components.Settings.applyUserOptions();
    PioneerDDJREV1.startStartupModeBlink();
    PioneerDDJREV1.Components.Capabilities.detectStems();
    PioneerDDJREV1.Components.ModeGate.enforceStemsPriority();
    PioneerDDJREV1.Components.Bootstrap.initialize();
    PioneerDDJREV1.ComponentJSTransport.initialize();
    PioneerDDJREV1.Components.Bootstrap.connect();
    PioneerDDJREV1.registerAutoloopLedConnections();
    PioneerDDJREV1.registerSlicerBeatConnections();
};

//
// Channel level lights
//

// DDJ-REV1 meter CC 0x02: visual red ~midi 116; cap 122. Scale 121 aligns LEDs with Mixxx skin.
PioneerDDJREV1._VU_MIDI_MAX = 122;
PioneerDDJREV1._VU_MIDI_SCALE = 121;
PioneerDDJREV1.vuMeterLevelToMidi = function(level) {
    const x = Math.max(0, Math.min(1, Number(level)));
    if (!Number.isFinite(x)) {
        return 0;
    }
    return Math.min(PioneerDDJREV1._VU_MIDI_MAX, Math.round(x * PioneerDDJREV1._VU_MIDI_SCALE));
};

PioneerDDJREV1._vuMeterLookup = {
    "[Channel1]": { idx: 0, status: 0xB0, },
    "[Channel2]": { idx: 1, status: 0xB1, },
    "[Channel3]": { idx: 2, status: 0xB2, },
    "[Channel4]": { idx: 3, status: 0xB3, },
};
PioneerDDJREV1.vuMeterUpdate = function (value, group) {
    const entry = PioneerDDJREV1._vuMeterLookup[group];
    if (!entry) {
        return;
    }
    const newVal = PioneerDDJREV1.vuMeterLevelToMidi(value);
    if (newVal === PioneerDDJREV1.vuMeterLastSent[entry.idx]) {
        return;
    }
    PioneerDDJREV1.vuMeterLastSent[entry.idx] = newVal;
    midi.sendShortMsg(entry.status, 0x02, newVal);
};

// Main mix L/R drives left-side meters (Deck 1 & 3) and right-side meters (Deck 2 & 4).
PioneerDDJREV1.onVuMeterChangeL = function(value, _group, _control) {
    const v = PioneerDDJREV1.vuMeterLevelToMidi(value);
    if (v === PioneerDDJREV1.vuMeterStereoLastSent[0]) {
        return;
    }
    PioneerDDJREV1.vuMeterStereoLastSent[0] = v;
    midi.sendShortMsg(0xB0, 0x02, v);
    midi.sendShortMsg(0xB2, 0x02, v);
};
PioneerDDJREV1.onVuMeterChangeR = function(value, _group, _control) {
    const v = PioneerDDJREV1.vuMeterLevelToMidi(value);
    if (v === PioneerDDJREV1.vuMeterStereoLastSent[1]) {
        return;
    }
    PioneerDDJREV1.vuMeterStereoLastSent[1] = v;
    midi.sendShortMsg(0xB1, 0x02, v);
    midi.sendShortMsg(0xB3, 0x02, v);
};
PioneerDDJREV1.fxUpdate = function (value, group) {
    PioneerDDJREV1.Components.invoke("effects", "fxUpdate", [value, group]);
};
// ** trackLoadedLED ** - unified deck load handler (LED indicator + hotcue refresh + stem sync)
PioneerDDJREV1.trackLoadedLED = function (value, group, _control) {
    midi.sendShortMsg(
        0x9F,
        group.match(script.channelRegEx)[1] - 1,
        value > 0 ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF
    );
    const deckNum = script.deckFromGroup(group);
    if (deckNum) {
        const deckIdx = deckNum - 1;
        if (PioneerDDJREV1.hotcueRefreshTimers[deckIdx] !== null) {
            engine.stopTimer(PioneerDDJREV1.hotcueRefreshTimers[deckIdx]);
        }
        PioneerDDJREV1.hotcueRefreshTimers[deckIdx] = engine.beginTimer(150, function() {
            PioneerDDJREV1.hotcueRefreshTimers[deckIdx] = null;
            PioneerDDJREV1.ComponentJSTransport.refreshHotcueLeds(deckNum);
        }, true);
    }
    PioneerDDJREV1.Components.invoke("stems", "onDeckTrackLoaded", [value, group]);
};

PioneerDDJREV1.toggleLight = function (midiIn, active) {
    midi.sendShortMsg(midiIn.status, midiIn.data1, active ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
};

PioneerDDJREV1.getDeckStatusFromGroup = function(group) {
    const match = group && group.match(/\[Channel([1-4])\]/);
    if (!match) {
        return null;
    }
    return 0x8F + parseInt(match[1], 10);
};

PioneerDDJREV1.getDeckGroupFromStatus = function(status) {
    if (typeof status !== "number" || status < 0x90 || status > 0x93) {
        return null;
    }
    return "[Channel" + (status - 0x8F) + "]";
};

PioneerDDJREV1.resolveDeckStatusForSync = function(group, status) {
    if (typeof status === "number" && status >= 0x90 && status <= 0x93) {
        return status;
    }
    return PioneerDDJREV1.getDeckStatusFromGroup(group);
};

// ** pulseSyncLed ** - brief sync LED pulse on press
PioneerDDJREV1.pulseSyncLed = function(group, status, durationMs) {
    const resolvedStatus = PioneerDDJREV1.resolveDeckStatusForSync(group, status);
    const resolvedGroup = group || PioneerDDJREV1.getDeckGroupFromStatus(resolvedStatus);
    if (resolvedStatus === null) {
        return;
    }
    const timerKey = resolvedGroup || ("status:" + resolvedStatus);
    if (PioneerDDJREV1.syncPulseTimers[timerKey] !== undefined) {
        engine.stopTimer(PioneerDDJREV1.syncPulseTimers[timerKey]);
        PioneerDDJREV1.syncPulseTimers[timerKey] = undefined;
    }
    const pulseDuration = durationMs || 120;
    PioneerDDJREV1.syncPulseActive[timerKey] = true;
    midi.sendShortMsg(resolvedStatus, 0x58, PioneerDDJREV1.LED_ON);
    PioneerDDJREV1.syncPulseTimers[timerKey] = engine.beginTimer(pulseDuration, function() {
        PioneerDDJREV1.syncPulseTimers[timerKey] = undefined;
        PioneerDDJREV1.syncPulseActive[timerKey] = false;
        PioneerDDJREV1.syncPulseIgnoreUntil[timerKey] = 0;
        if (!resolvedGroup || !engine.getValue(resolvedGroup, "sync_enabled")) {
            midi.sendShortMsg(resolvedStatus, 0x58, PioneerDDJREV1.LED_OFF);
        }
    }, true);
};

PioneerDDJREV1.syncPulseActive = {};
PioneerDDJREV1.syncLedConnections = [];

// ** syncLedCallback ** - sync LED state from engine
PioneerDDJREV1.syncLedCallback = function(value, group) {
    const timerKey = group;
    const deckStatus = PioneerDDJREV1.getDeckStatusFromGroup(group);
    if (deckStatus === null) {
        return;
    }
    if (value > 0) {
        midi.sendShortMsg(deckStatus, 0x58, PioneerDDJREV1.LED_ON);
        return;
    }
    if (PioneerDDJREV1.syncPulseActive[timerKey]) {
        return;
    }
    midi.sendShortMsg(deckStatus, 0x58, PioneerDDJREV1.LED_OFF);
};

PioneerDDJREV1.registerSyncLedConnections = function() {
    PioneerDDJREV1.unregisterSyncLedConnections();
    for (let d = 1; d <= 4; d++) {
        const group = "[Channel" + d + "]";
        const conn = engine.makeConnection(group, "sync_enabled", PioneerDDJREV1.syncLedCallback);
        if (conn) {
            PioneerDDJREV1.syncLedConnections.push(conn);
        }
    }
    PioneerDDJREV1.refreshSyncLeds();
};

PioneerDDJREV1.unregisterSyncLedConnections = function() {
    for (let i = 0; i < PioneerDDJREV1.syncLedConnections.length; i++) {
        if (PioneerDDJREV1.syncLedConnections[i] && typeof PioneerDDJREV1.syncLedConnections[i].disconnect === "function") {
            PioneerDDJREV1.syncLedConnections[i].disconnect();
        }
    }
    PioneerDDJREV1.syncLedConnections = [];
    Object.keys(PioneerDDJREV1.syncPulseTimers).forEach(function(timerKey) {
        if (PioneerDDJREV1.syncPulseTimers[timerKey] !== undefined) {
            engine.stopTimer(PioneerDDJREV1.syncPulseTimers[timerKey]);
        }
    });
    PioneerDDJREV1.syncPulseTimers = {};
    PioneerDDJREV1.syncPulseActive = {};
    PioneerDDJREV1.syncPulseIgnoreUntil = {};
};

PioneerDDJREV1.refreshSyncLeds = function() {
    for (let d = 1; d <= 4; d++) {
        const group = "[Channel" + d + "]";
        const enabled = engine.getValue(group, "sync_enabled");
        const deckStatus = 0x90 + (d - 1);
        midi.sendShortMsg(deckStatus, 0x58, enabled > 0 ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);
    }
};

PioneerDDJREV1.modeBlinkTimers = [null, null, null, null];
PioneerDDJREV1.startupModeBlinkTimers = [null, null, null, null];
PioneerDDJREV1.startupVinylSyncTimer = null;
PioneerDDJREV1.deckSelectNotifyNote = 0x72;

PioneerDDJREV1.getDeckSelectPairStatus = function(channel) {
    if (channel < 0 || channel > 3) {
        return null;
    }
    return (channel === 0 || channel === 2) ? 0x92 : 0x93;
};

PioneerDDJREV1.sendDeckSelectNotification = function(channel, active) {
    const pairStatus = PioneerDDJREV1.getDeckSelectPairStatus(channel);
    if (pairStatus === null) {
        return;
    }
    midi.sendShortMsg(
        pairStatus,
        PioneerDDJREV1.deckSelectNotifyNote,
        active ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF
    );
};

PioneerDDJREV1.getDeckSelectLight = function(channel) {
    const deckLights = [
        PioneerDDJREV1.lights.deck1.deckSelect,
        PioneerDDJREV1.lights.deck2.deckSelect,
        PioneerDDJREV1.lights.deck3.deckSelect,
        PioneerDDJREV1.lights.deck4.deckSelect,
    ];
    return deckLights[channel] || null;
};

// Host→controller deck-select note 0x17 latches hardware vinyl (CC 0x22) vs CDJ (CC 0x23).
PioneerDDJREV1.syncHardwareVinylMode = function(channel) {
    if (channel < 0 || channel > 3) {
        return;
    }
    const deckSelectLight = PioneerDDJREV1.getDeckSelectLight(channel);
    if (!deckSelectLight) {
        return;
    }
    PioneerDDJREV1.toggleLight(deckSelectLight, !!PioneerDDJREV1.vinylMode[channel]);
};

PioneerDDJREV1.syncAllHardwareVinylModes = function() {
    for (let i = 0; i < 4; i++) {
        PioneerDDJREV1.syncHardwareVinylMode(i);
    }
};

PioneerDDJREV1.clearDeckSelectModeBlink = function(channel) {
    if (channel < 0 || channel > 3) {
        return;
    }
    PioneerDDJREV1.clearDeckTimer(PioneerDDJREV1.modeBlinkTimers, channel);
    PioneerDDJREV1.sendDeckSelectNotification(channel, false);
    PioneerDDJREV1.syncHardwareVinylMode(channel);
};

PioneerDDJREV1.startDeckSelectModeBlink = function(channel, isVinylMode) {
    if (PioneerDDJREV1.getDeckSelectPairStatus(channel) === null) {
        return;
    }
    PioneerDDJREV1.clearDeckSelectModeBlink(channel);
    const blinkCount = isVinylMode ? 3 : 2;
    const intervalMs = isVinylMode ? 180 : 260;
    const totalEdges = blinkCount * 2;
    let edgeCount = 0;
    let lightOn = false;
    PioneerDDJREV1.modeBlinkTimers[channel] = engine.beginTimer(intervalMs, function() {
        lightOn = !lightOn;
        PioneerDDJREV1.sendDeckSelectNotification(channel, lightOn);
        edgeCount++;
        if (edgeCount >= totalEdges) {
            PioneerDDJREV1.clearDeckSelectModeBlink(channel);
        }
    });
};

PioneerDDJREV1.startStartupModeBlink = function() {
    if (PioneerDDJREV1.startupVinylSyncTimer !== null) {
        engine.stopTimer(PioneerDDJREV1.startupVinylSyncTimer);
        PioneerDDJREV1.startupVinylSyncTimer = null;
    }
    for (let i = 0; i < 4; i++) {
        if (PioneerDDJREV1.startupModeBlinkTimers[i] !== null) {
            engine.stopTimer(PioneerDDJREV1.startupModeBlinkTimers[i]);
            PioneerDDJREV1.startupModeBlinkTimers[i] = null;
        }
        const delayMs = i * 120;
        PioneerDDJREV1.startupModeBlinkTimers[i] = engine.beginTimer(delayMs, function() {
            PioneerDDJREV1.startupModeBlinkTimers[i] = null;
            PioneerDDJREV1.startDeckSelectModeBlink(i, PioneerDDJREV1.vinylMode[i]);
        }, true);
    }
    // After last staggered blink (3×120 ms + 3×2×260 ms CDJ edges), restore hardware vinyl/CDJ for all decks.
    const startupSyncDelayMs = 3 * 120 + 3 * 2 * 260 + 80;
    PioneerDDJREV1.startupVinylSyncTimer = engine.beginTimer(startupSyncDelayMs, function() {
        PioneerDDJREV1.startupVinylSyncTimer = null;
        PioneerDDJREV1.syncAllHardwareVinylModes();
    }, true);
};

//
// Loop IN/OUT ADJUST
//

PioneerDDJREV1.toggleLoopAdjustIn = function (channel, _control, value, _status, group) {
    if (value === 0 || engine.getValue(group, "loop_enabled") === 0) {
        return;
    }
    PioneerDDJREV1.loopAdjustIn[channel] = !PioneerDDJREV1.loopAdjustIn[channel];
    PioneerDDJREV1.loopAdjustOut[channel] = false;
};

PioneerDDJREV1.toggleLoopAdjustOut = function (channel, _control, value, _status, group) {
    if (value === 0 || engine.getValue(group, "loop_enabled") === 0) {
        return;
    }
    PioneerDDJREV1.loopAdjustOut[channel] = !PioneerDDJREV1.loopAdjustOut[channel];
    PioneerDDJREV1.loopAdjustIn[channel] = false;
};

// Two signals are sent here so that the light stays lit/unlit in its shift state too
PioneerDDJREV1.setReloopLight = function(status, value) {
    midi.sendShortMsg(status, 0x4D, value);
    midi.sendShortMsg(status, 0x50, value);
};

// ** setLoopButtonLights ** - beat loop pad LEDs
PioneerDDJREV1.setLoopButtonLights = function(status, value) {
    [0x10, 0x11, 0x4E, 0x4C].forEach(function (control) {
        midi.sendShortMsg(status, control, value);
    });
};

PioneerDDJREV1.startLoopLightsBlink = function(channel, control, status, group) {
    let blink = PioneerDDJREV1.LED_ON;

    PioneerDDJREV1.stopLoopLightsBlink(control, status, group);

    PioneerDDJREV1.timers[group][control] = engine.beginTimer(500, () => {
        blink = PioneerDDJREV1.LED_ON - blink;

        if (PioneerDDJREV1.loopAdjustOut[channel]) {
            midi.sendShortMsg(status, 0x10, PioneerDDJREV1.LED_OFF);
            midi.sendShortMsg(status, 0x4C, PioneerDDJREV1.LED_OFF);
        } else {
            midi.sendShortMsg(status, 0x10, blink);
            midi.sendShortMsg(status, 0x4C, blink);
        }

        if (PioneerDDJREV1.loopAdjustIn[channel]) {
            midi.sendShortMsg(status, 0x11, PioneerDDJREV1.LED_OFF);
            midi.sendShortMsg(status, 0x4E, PioneerDDJREV1.LED_OFF);
        } else {
            midi.sendShortMsg(status, 0x11, blink);
            midi.sendShortMsg(status, 0x4E, blink);
        }
    });
};

PioneerDDJREV1.stopLoopLightsBlink = function(control, status, group) {
    PioneerDDJREV1.timers[group] = PioneerDDJREV1.timers[group] || {};

    if (PioneerDDJREV1.timers[group][control] !== undefined) {
        engine.stopTimer(PioneerDDJREV1.timers[group][control]);
    }
    PioneerDDJREV1.timers[group][control] = undefined;
    PioneerDDJREV1.setLoopButtonLights(status, PioneerDDJREV1.LED_ON);
};

// ** loopToggle ** - loop in/out button handler
PioneerDDJREV1._loopToggleLookup = {
    "[Channel1]": { status: 0x90, side: 0, },
    "[Channel2]": { status: 0x91, side: 1, },
    "[Channel3]": { status: 0x90, side: 0, },
    "[Channel4]": { status: 0x91, side: 1, },
};
PioneerDDJREV1.loopToggle = function (_channel, control, value, _status, group) {
    const entry = PioneerDDJREV1._loopToggleLookup[group];
    if (!entry) {
        return;
    }
    const status = entry.status;
    const side = entry.side;

    PioneerDDJREV1.setReloopLight(status, value ? PioneerDDJREV1.LED_ON : PioneerDDJREV1.LED_OFF);

    if (value) {
        PioneerDDJREV1.startLoopLightsBlink(side, control, status, group);
    } else {
        PioneerDDJREV1.stopLoopLightsBlink(control, status, group);
        PioneerDDJREV1.loopAdjustIn[side] = false;
        PioneerDDJREV1.loopAdjustOut[side] = false;
    }
};


// ** beatLoopRoll ** - roll pad hold/release
PioneerDDJREV1.beatLoopRoll = function (_channel, control, value, status, group) {
    PioneerDDJREV1.Components.invoke("beatPads", "handleBeatLoopRoll", [control, value, status, group]);
};


// ** beatJump ** - jump forward/back pads
PioneerDDJREV1.beatJump = function (_channel, control, value, status, group) {
    PioneerDDJREV1.Components.invoke("beatPads", "handleBeatJump", [control, value, status, group]);
};


// ** headphoneCueing ** - Studio-style cue mix auto-adjust
PioneerDDJREV1.headphoneCueing = function (_channel, control, value, status, group) {
    PioneerDDJREV1.Components.invoke("headphones", "cueing", [control, value, status, group]);
};

//
//Effects Buffering
// Button press buffering system to detect single, double, and triple presses
//

PioneerDDJREV1.bufferButtonPress = function (fxGroup, buttonIndex, mixxxUnit) {
    PioneerDDJREV1.Components.invoke("effects", "bufferButtonPress", [fxGroup, buttonIndex, mixxxUnit]);
};


PioneerDDJREV1.processBufferedPresses = function (fxGroup, mixxxUnit) {
    PioneerDDJREV1.Components.invoke("effects", "processBufferedPresses", [fxGroup, mixxxUnit]);
};

// Handle when single button in a group are pressed
PioneerDDJREV1.handleSinglePress = function (fxGroup, buttonIndex, mixxxUnit) {
    PioneerDDJREV1.Components.invoke("effects", "handleSinglePress", [fxGroup, buttonIndex, mixxxUnit]);
};
// Handle when dual button in a group are pressed
PioneerDDJREV1.handleDoublePress = function (fxGroup, buttonIndices, mixxxUnit) {
    PioneerDDJREV1.Components.invoke("effects", "handleDoublePress", [fxGroup, buttonIndices, mixxxUnit]);
};

PioneerDDJREV1.handleTriplePress = function (fxGroup, mixxxUnit) {
    PioneerDDJREV1.Components.invoke("effects", "handleTriplePress", [fxGroup, mixxxUnit]);
};


//
// CUE/LOOP CALL
//

PioneerDDJREV1.cueLoopCallLeft = function (_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "loop_scale", 0.5);
    }
};

PioneerDDJREV1.cueLoopCallRight = function (_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "loop_scale", 2.0);
    }
};


// ** syncPressed ** 
PioneerDDJREV1.syncPressed = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.ComponentJSTransport.syncInput(_channel, _control, value, _status, group);
};

PioneerDDJREV1.syncLongPressed = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("transport", "syncLongPressed", [value, group]);
};

PioneerDDJREV1.cycleTempoRange = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("transport", "cycleTempoRange", [value, group]);
};

//
// Jog wheels
//

PioneerDDJREV1.jogTurn = function (channel, control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("jog", "jogTurn", [channel, control, value, group]);
};

// Function to handle jog wheel search
PioneerDDJREV1.jogSearch = function (channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("jog", "jogSearch", [channel, value, group]);
};

// Function to handle jog wheel touch
PioneerDDJREV1.jogTouch = function (channel, control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("jog", "jogTouch", [channel, control, value, group]);
};

// Function to toggle vinyl mode for a specific deck
PioneerDDJREV1.toggleVinylMode = function (channel, _control, value, status, group) {
    let deckChannel = channel;
    let deckGroup = group;
    const st = status & 0xFF;
    if (st === 0x92) {
        deckChannel = PioneerDDJREV1.leftActiveDeck - 1;
        deckGroup = "[Channel" + PioneerDDJREV1.leftActiveDeck + "]";
    } else if (st === 0x93) {
        deckChannel = PioneerDDJREV1.rightActiveDeck - 1;
        deckGroup = "[Channel" + PioneerDDJREV1.rightActiveDeck + "]";
    }
    PioneerDDJREV1.Components.invoke("jog", "toggleVinylMode", [deckChannel, value, deckGroup]);
};

//
// Shift button
//

PioneerDDJREV1.shiftButton = function (_channel, _control, value, _status, _group) {
    PioneerDDJREV1.Components.invoke("jog", "shiftButton", [value]);
};
PioneerDDJREV1.shiftPlayBrake = function(_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("transport", "shiftPlayBrake", [value, group]);
};

PioneerDDJREV1.play = function(_channel, _control, value, _status, group) {
    PioneerDDJREV1.ComponentJSTransport.playInput(_channel, _control, value, _status, group);
};

//
// Sampler mode
//

PioneerDDJREV1.samplerPlayOutputCallbackFunction = function (value, group, _control) {
    PioneerDDJREV1.Components.invoke("sampler", "samplerPlayOutputCallbackFunction", [value, group]);
};

PioneerDDJREV1.samplerTrackLoadedOutputCallbackFunction = function (value, group, _control) {
    PioneerDDJREV1.Components.invoke("sampler", "samplerTrackLoadedOutputCallbackFunction", [value, group]);
};


PioneerDDJREV1.samplerPadPressed = function (channel, control, value, status, group) {
    PioneerDDJREV1.pianoRollClearGhostHold();
    PioneerDDJREV1.Components.invoke("sampler", "samplerPadPressed", [channel, control, value, group, status]);
};

PioneerDDJREV1.samplerPadShiftPressed = function (channel, control, value, status, group) {
    PioneerDDJREV1.Components.invoke("sampler", "samplerPadShiftPressed", [channel, control, value, group, status]);
};

PioneerDDJREV1.startSamplerBlink = function (channel, control, group) {
    PioneerDDJREV1.Components.invoke("sampler", "startSamplerBlink", [channel, control, group]);
};

PioneerDDJREV1.stopSamplerBlink = function (channel, control, _value, _status, _group) {
    PioneerDDJREV1.Components.invoke("sampler", "stopSamplerBlink", [channel, control]);
};

PioneerDDJREV1.toggleQuantize = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("mixer", "toggleQuantize", [value, group]);
};


//Load Selected Track and Sync Stem Lights 
PioneerDDJREV1.loadSelectedTrack = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("stems", "loadSelectedTrack", [value, group]);
};

//Cue Shift Manufacture Default
PioneerDDJREV1.cueShift = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("transport", "cueShift", [value, group]);
};

PioneerDDJREV1.cueDefault = function(_channel, _control, value, _status, group) {
    PioneerDDJREV1.ComponentJSTransport.cueInput(_channel, _control, value, _status, group);
};
//Sort Library
PioneerDDJREV1.librarySort = function (channel, control, value, status, group) {
    if (!value) {
        return;
    }
    PioneerDDJREV1.Components.invoke("mixer", "librarySort", [group]);
};

//
// Additional features
//

//Set Samplers 1-16 Gain 
PioneerDDJREV1.samplerVolume = function (channel, control, value, status, group) {
    PioneerDDJREV1.Components.invoke("sampler", "samplerVolume", [value, group]);
};

/////////////////////////////////////////////////
// Fader starts
/////////////////////////////////////////////////
PioneerDDJREV1.crossFaderStart = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("mixer", "crossFaderStart", [group]);
};

/////////////////////////////////////////////////
// STEMS
/////////////////////////////////////////////////
PioneerDDJREV1.levelDepth = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("stems", "levelDepth", [value]);
};
//Stem Effects
PioneerDDJREV1.stemEffect = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("stems", "stemEffect", [value, group]);
};
//Stem Mute
PioneerDDJREV1.stemShift = function (channel, control, value, status, group) {
    PioneerDDJREV1.Components.invoke("stems", "stemShift", [channel, control, value, status, group]);
};
/////////////////////////////////////////////////
// MAN. Effects
/////////////////////////////////////////////////
// FX button handler - processes button presses and maintains synchronization
PioneerDDJREV1.FX = function (channel, control, value, status, group) {
    PioneerDDJREV1.Components.invoke("effects", "FX", [control, value, status]);
};

// ** Single effect ** - enable one, disable others - sync to controller
PioneerDDJREV1.syncSingleEffect = function (fxGroup, activeButtonIndex, mixxxUnit) {
    PioneerDDJREV1.Components.invoke("effects", "syncSingleEffect", [fxGroup, activeButtonIndex, mixxxUnit]);
};

// ** Dual effects ** - sync to controller
PioneerDDJREV1.syncDualEffects = function (fxGroup, mixxxUnit, buttonIndices) {
    PioneerDDJREV1.Components.invoke("effects", "syncDualEffects", [fxGroup, mixxxUnit, buttonIndices]);
};

// ** All effects ** - three active – sync to controller
PioneerDDJREV1.syncAllEffects = function (mixxxUnit) {
    PioneerDDJREV1.Components.invoke("effects", "syncAllEffects", [mixxxUnit]);
};

// ** No effects ** - all off - sync to controller
PioneerDDJREV1.syncNoEffects = function (mixxxUnit) {
    PioneerDDJREV1.Components.invoke("effects", "syncNoEffects", [mixxxUnit]);
};

PioneerDDJREV1.browsePush = function (_ch, _ctrl, value, _st, group) {
    if (!value) {
        return;
    }
    if (PioneerDDJREV1.bigLibraryShiftPush && PioneerDDJREV1.shiftPressed) {
        const cur = engine.getValue("[Master]", "maximize_library");
        engine.setValue("[Master]", "maximize_library", cur ? 0 : 1);
        return;
    }
    engine.setValue(group, "MoveFocusForward", 1);
};

// Selector function for effect navigation
PioneerDDJREV1.selector = function (_channel, _control, value, _status, group) {
    PioneerDDJREV1.Components.invoke("effects", "selector", [value, group]);
};
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// Scratch Bank
PioneerDDJREV1.deckMappings = PioneerDDJREV1.Components.ScratchBank.deckMappings;
PioneerDDJREV1.samplerMappings = PioneerDDJREV1.Components.ScratchBank.samplerMappings;

// Main function to load scratch samples to decks
PioneerDDJREV1.loadScratchToDeck = function (channel, control, value, status, group) {
    PioneerDDJREV1.Components.invoke("scratchBank", "loadScratchToDeck", [channel, control, value, status]);
};



/////////////////////////////////////////////////
// Shutdown
/////////////////////////////////////////////////

PioneerDDJREV1.shutdown = function () {
    PioneerDDJREV1.MixxxedModeShutdown();
    PioneerDDJREV1.ComponentJSTransport.shutdown();
    PioneerDDJREV1.Components.Bootstrap.shutdown();
};

////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global midi                                                        */
/* global bpm                                                         */
/* global components                                                  */
////////////////////////////////////////////////////////////////////////
var PioneerDDJSX = function() {};

/*
	Author: 		DJMaxergy
	Version: 		1.19, 05/01/2018
	Description: 	Pioneer DDJ-SX Controller Mapping for Mixxx
    Source: 		http://github.com/DJMaxergy/mixxx/tree/pioneerDDJSX_mapping
    
    Copyright (c) 2018 DJMaxergy, licensed under GPL version 2 or later
    Copyright (c) 2014-2015 various contributors, base for this mapping, licensed under MIT license
    
    Contributors:
    - Michael Stahl (DG3NEC): original DDJ-SB2 mapping for Mixxx 2.0
    - Sophia Herzog: midiAutoDJ-scripts
    - Joan Ardiaca Jové (joan.ardiaca@gmail.com): Pioneer DDJ-SB mapping for Mixxx 2.0
    - wingcom (wwingcomm@gmail.com): start of Pioneer DDJ-SB mapping
      https://github.com/wingcom/Mixxx-Pioneer-DDJ-SB
    - Hilton Rudham: Pioneer DDJ-SR mapping
      https://github.com/hrudham/Mixxx-Pioneer-DDJ-SR
      
    GPL license notice for current version:
    This program is free software; you can redistribute it and/or modify it under the terms of the
    GNU General Public License as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License along with this program; if
    not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
    
    
    MIT License for earlier versions:
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// Sets the jogwheels sensitivity. 1 is default, 2 is twice as sensitive, 0.5 is half as sensitive.
PioneerDDJSX.jogwheelSensitivity = 1;

// Sets how much more sensitive the jogwheels get when holding shift.
// Set to 1 to disable jogwheel sensitivity increase when holding shift (default: 10).
PioneerDDJSX.jogwheelShiftMultiplier = 10;

// If true, vu meters twinkle if AutoDJ is enabled (default: true).
PioneerDDJSX.twinkleVumeterAutodjOn = true;
// If true, selected track will be added to AutoDJ queue-top on pressing shift + rotary selector,
// else track will be added to AutoDJ queue-bottom (default: false).
PioneerDDJSX.autoDJAddTop = false;
// Sets the duration of sleeping between AutoDJ actions if AutoDJ is enabled [ms] (default: 1000).
PioneerDDJSX.autoDJTickInterval = 1000;
// Sets the maximum adjustment of BPM allowed for beats to sync if AutoDJ is enabled [BPM] (default: 10).
PioneerDDJSX.autoDJMaxBpmAdjustment = 10;
// If true, AutoDJ queue is being shuffled after skipping a track (default: false).
// When using a fixed set of tracks without manual intervention, some tracks may be unreachable, 
// due to having an unfortunate place in the queue ordering. This solves the issue.
PioneerDDJSX.autoDJShuffleAfterSkip = false;

// If true, by releasing rotary selector, 
// track in preview player jumps forward to "jumpPreviewPosition"
// (default: jumpPreviewEnabled = true, jumpPreviewPosition = 0.3). 
PioneerDDJSX.jumpPreviewEnabled = true;
PioneerDDJSX.jumpPreviewPosition = 0.3;

// If true, pad press in SAMPLER-PAD-MODE repeatedly causes sampler to play 
// loaded track from cue-point, else it causes to play loaded track from the beginning (default: false).
PioneerDDJSX.samplerCueGotoAndPlay = false;

// If true, PFL / Cue (headphone) is being activated by loading a track into certain deck (default: true).
PioneerDDJSX.autoPFL = true;


///////////////////////////////////////////////////////////////
//               INIT, SHUTDOWN & GLOBAL HELPER              //
///////////////////////////////////////////////////////////////

PioneerDDJSX.shiftPressed = false;
PioneerDDJSX.rotarySelectorChanged = false;
PioneerDDJSX.panels = [false, false]; // view state of effect and sampler panel
PioneerDDJSX.shiftPanelSelectPressed = false;

PioneerDDJSX.syncRate = [0, 0, 0, 0];
PioneerDDJSX.gridAdjustSelected = [false, false, false, false];
PioneerDDJSX.gridSlideSelected = [false, false, false, false];
PioneerDDJSX.needleSearchTouched = [false, false, false, false];
PioneerDDJSX.chFaderStart = [null, null, null, null];
PioneerDDJSX.toggledBrake = [false, false, false, false];
PioneerDDJSX.scratchMode = [true, true, true, true];
PioneerDDJSX.wheelLedsBlinkStatus = [0, 0, 0, 0];
PioneerDDJSX.setUpSpeedSliderRange = [0.08, 0.08, 0.08, 0.08];

// PAD mode storage:
PioneerDDJSX.padModes = {
    'hotCue': 0,
    'loopRoll': 1,
    'slicer': 2,
    'sampler': 3,
    'group1': 4,
    'beatloop': 5,
    'group3': 6,
    'group4': 7
};
PioneerDDJSX.activePadMode = [
    PioneerDDJSX.padModes.hotCue,
    PioneerDDJSX.padModes.hotCue,
    PioneerDDJSX.padModes.hotCue,
    PioneerDDJSX.padModes.hotCue
];
PioneerDDJSX.samplerVelocityMode = [false, false, false, false];

// FX storage:
PioneerDDJSX.fxKnobMSBValue = [0, 0];
PioneerDDJSX.shiftFxKnobMSBValue = [0, 0];

// used for advanced auto dj features:
PioneerDDJSX.blinkAutodjState = false;
PioneerDDJSX.autoDJTickTimer = 0;
PioneerDDJSX.autoDJSyncBPM = false;
PioneerDDJSX.autoDJSyncKey = false;

// used for PAD parameter selection:
PioneerDDJSX.selectedSamplerBank = 0;
PioneerDDJSX.selectedLoopParam = [0, 0, 0, 0];
PioneerDDJSX.selectedLoopRollParam = [2, 2, 2, 2];
PioneerDDJSX.selectedLoopIntervals = [
    [1 / 4, 1 / 2, 1, 2, 4, 8, 16, 32],
    [1 / 4, 1 / 2, 1, 2, 4, 8, 16, 32],
    [1 / 4, 1 / 2, 1, 2, 4, 8, 16, 32],
    [1 / 4, 1 / 2, 1, 2, 4, 8, 16, 32]
];
PioneerDDJSX.selectedLooprollIntervals = [
    [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8],
    [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8],
    [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8],
    [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8]
];
PioneerDDJSX.loopIntervals = [
    [1 / 4, 1 / 2, 1, 2, 4, 8, 16, 32],
    [1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8, 16],
    [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8],
    [1 / 32, 1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4]
];
PioneerDDJSX.selectedSlicerQuantizeParam = [1, 1, 1, 1];
PioneerDDJSX.selectedSlicerQuantization = [1 / 4, 1 / 4, 1 / 4, 1 / 4];
PioneerDDJSX.slicerQuantizations = [1 / 8, 1 / 4, 1 / 2, 1];
PioneerDDJSX.selectedSlicerDomainParam = [0, 0, 0, 0];
PioneerDDJSX.selectedSlicerDomain = [8, 8, 8, 8];
PioneerDDJSX.slicerDomains = [8, 16, 32, 64];

// slicer storage:
PioneerDDJSX.slicerBeatsPassed = [0, 0, 0, 0];
PioneerDDJSX.slicerPreviousBeatsPassed = [0, 0, 0, 0];
PioneerDDJSX.slicerActive = [false, false, false, false];
PioneerDDJSX.slicerAlreadyJumped = [false, false, false, false];
PioneerDDJSX.slicerButton = [0, 0, 0, 0];
PioneerDDJSX.slicerModes = {
    'contSlice': 0,
    'loopSlice': 1
};
PioneerDDJSX.activeSlicerMode = [
    PioneerDDJSX.slicerModes.contSlice,
    PioneerDDJSX.slicerModes.contSlice,
    PioneerDDJSX.slicerModes.contSlice,
    PioneerDDJSX.slicerModes.contSlice
];


PioneerDDJSX.init = function(id) {
    PioneerDDJSX.scratchSettings = {
        'alpha': 1.0 / 8,
        'beta': 1.0 / 8 / 32,
        'jogResolution': 2048,
        'vinylSpeed': 33 + 1 / 3,
    };

    PioneerDDJSX.channelGroups = {
        '[Channel1]': 0x00,
        '[Channel2]': 0x01,
        '[Channel3]': 0x02,
        '[Channel4]': 0x03
    };

    PioneerDDJSX.samplerGroups = {
        '[Sampler1]': 0x00,
        '[Sampler2]': 0x01,
        '[Sampler3]': 0x02,
        '[Sampler4]': 0x03,
        '[Sampler5]': 0x04,
        '[Sampler6]': 0x05,
        '[Sampler7]': 0x06,
        '[Sampler8]': 0x07
    };

    PioneerDDJSX.fxUnitGroups = {
        '[EffectRack1_EffectUnit1]': 0x00,
        '[EffectRack1_EffectUnit2]': 0x01,
        '[EffectRack1_EffectUnit3]': 0x02,
        '[EffectRack1_EffectUnit4]': 0x03
    };

    PioneerDDJSX.fxEffectGroups = {
        '[EffectRack1_EffectUnit1_Effect1]': 0x00,
        '[EffectRack1_EffectUnit1_Effect2]': 0x01,
        '[EffectRack1_EffectUnit1_Effect3]': 0x02,
        '[EffectRack1_EffectUnit2_Effect1]': 0x00,
        '[EffectRack1_EffectUnit2_Effect2]': 0x01,
        '[EffectRack1_EffectUnit2_Effect3]': 0x02
    };

    PioneerDDJSX.ledGroups = {
        'hotCue': 0x00,
        'loopRoll': 0x10,
        'slicer': 0x20,
        'sampler': 0x30,
        'group1': 0x40,
        'group2': 0x50,
        'group3': 0x60,
        'group4': 0x70
    };

    PioneerDDJSX.nonPadLeds = {
        'headphoneCue': 0x54,
        'shiftHeadphoneCue': 0x68,
        'cue': 0x0C,
        'shiftCue': 0x48,
        'keyLock': 0x1A,
        'shiftKeyLock': 0x60,
        'play': 0x0B,
        'shiftPlay': 0x47,
        'vinyl': 0x0D,
        'sync': 0x58,
        'shiftSync': 0x5C,
        'autoLoop': 0x14,
        'shiftAutoLoop': 0x50,
        'loopHalve': 0x12,
        'shiftLoopHalve': 0x61,
        'loopDouble': 0x13,
        'shiftLoopDouble': 0x62,
        'loopIn': 0x10,
        'shiftLoopIn': 0x4C,
        'loopOut': 0x11,
        'shiftLoopOut': 0x4D,
        'censor': 0x15,
        'shiftCensor': 0x38,
        'slip': 0x40,
        'shiftSlip': 0x63,
        'gridAdjust': 0x79,
        'shiftGridAdjust': 0x64,
        'gridSlide': 0x0A,
        'shiftGridSlide': 0x65,
        'takeoverPlus': 0x34,
        'takeoverMinus': 0x37,
        'fx1on': 0x47,
        'shiftFx1on': 0x63,
        'fx2on': 0x48,
        'shiftFx2on': 0x64,
        'fx3on': 0x49,
        'shiftFx3on': 0x65,
        'fxTab': 0x4A,
        'shiftFxTab': 0x66,
        'fx1assignDeck1': 0x4C,
        'shiftFx1assignDeck1': 0x70,
        'fx1assignDeck2': 0x4D,
        'shiftFx1assignDeck2': 0x71,
        'fx1assignDeck3': 0x4E,
        'shiftFx1assignDeck3': 0x72,
        'fx1assignDeck4': 0x4F,
        'shiftFx1assignDeck4': 0x73,
        'fx2assignDeck1': 0x50,
        'shiftFx2assignDeck1': 0x54,
        'fx2assignDeck2': 0x51,
        'shiftFx2assignDeck2': 0x55,
        'fx2assignDeck3': 0x52,
        'shiftFx2assignDeck3': 0x56,
        'fx2assignDeck4': 0x53,
        'shiftFx2assignDeck4': 0x57,
        'masterCue': 0x63,
        'shiftMasterCue': 0x62,
        'loadDeck1': 0x46,
        'shiftLoadDeck1': 0x58,
        'loadDeck2': 0x47,
        'shiftLoadDeck2': 0x59,
        'loadDeck3': 0x48,
        'shiftLoadDeck3': 0x60,
        'loadDeck4': 0x49,
        'shiftLoadDeck4': 0x61,
        'hotCueMode': 0x1B,
        'shiftHotCueMode': 0x69,
        'rollMode': 0x1E,
        'shiftRollMode': 0x6B,
        'slicerMode': 0x20,
        'shiftSlicerMode': 0x6D,
        'samplerMode': 0x22,
        'shiftSamplerMode': 0x6F,
        'longPressSamplerMode': 0x41,
        'parameterLeftHotCueMode': 0x24,
        'shiftParameterLeftHotCueMode': 0x01,
        'parameterLeftRollMode': 0x25,
        'shiftParameterLeftRollMode': 0x02,
        'parameterLeftSlicerMode': 0x26,
        'shiftParameterLeftSlicerMode': 0x03,
        'parameterLeftSamplerMode': 0x27,
        'shiftParameterLeftSamplerMode': 0x04,
        'parameterLeftGroup1Mode': 0x28,
        'shiftParameterLeftGroup1Mode': 0x05,
        'parameterLeftGroup2Mode': 0x29,
        'shiftParameterLeftGroup2Mode': 0x06,
        'parameterLeftGroup3Mode': 0x2A,
        'shiftParameterLeftGroup3Mode': 0x07,
        'parameterLeftGroup4Mode': 0x2B,
        'shiftParameterLeftGroup4Mode': 0x08,
        'parameterRightHotCueMode': 0x2C,
        'shiftParameterRightHotCueMode': 0x09,
        'parameterRightRollMode': 0x2D,
        'shiftParameterRightRollMode': 0x7A,
        'parameterRightSlicerMode': 0x2E,
        'shiftParameterRightSlicerMode': 0x7B,
        'parameterRightSamplerMode': 0x2F,
        'shiftParameterRightSamplerMode': 0x7C,
        'parameterRightGroup1Mode': 0x30,
        'shiftParameterRightGroup1Mode': 0x7D,
        'parameterRightGroup2Mode': 0x31,
        'shiftParameterRightGroup2Mode': 0x7E,
        'parameterRightGroup3Mode': 0x32,
        'shiftParameterRightGroup3Mode': 0x7F,
        'parameterRightGroup4Mode': 0x33,
        'shiftParameterRightGroup4Mode': 0x00
    };

    PioneerDDJSX.illuminationControl = {
        'loadedDeck1': 0x00,
        'loadedDeck2': 0x01,
        'loadedDeck3': 0x02,
        'loadedDeck4': 0x03,
        'unknownDeck1': 0x04,
        'unknownDeck2': 0x05,
        'unknownDeck3': 0x06,
        'unknownDeck4': 0x07,
        'playPauseDeck1': 0x0C,
        'playPauseDeck2': 0x0D,
        'playPauseDeck3': 0x0E,
        'playPauseDeck4': 0x0F,
        'cueDeck1': 0x10,
        'cueDeck2': 0x11,
        'cueDeck3': 0x12,
        'cueDeck4': 0x13,
        'djAppConnect': 0x09
    };

    PioneerDDJSX.wheelLedCircle = {
        'minVal': 0x00,
        'maxVal': 0x48
    };

    PioneerDDJSX.valueVuMeter = {
        '[Channel1]_current': 0,
        '[Channel2]_current': 0,
        '[Channel3]_current': 0,
        '[Channel4]_current': 0,
        '[Channel1]_enabled': 1,
        '[Channel2]_enabled': 1,
        '[Channel3]_enabled': 1,
        '[Channel4]_enabled': 1
    };

    // set 32 Samplers as default:
    engine.setValue("[Master]", "num_samplers", 32);

    // activate vu meter timer for Auto DJ:
    if (PioneerDDJSX.twinkleVumeterAutodjOn) {
        PioneerDDJSX.vuMeterTimer = engine.beginTimer(200, "PioneerDDJSX.vuMeterTwinkle()");
    }

    // initiate control status request:
    midi.sendShortMsg(0x9B, 0x08, 0x7F);

    // bind controls and init deck parameters:
    PioneerDDJSX.bindNonDeckControlConnections(true);
    for (var index in PioneerDDJSX.channelGroups) {
        if (PioneerDDJSX.channelGroups.hasOwnProperty(index)) {
            PioneerDDJSX.initDeck(index);
        }
    }

    // init effects section:
    PioneerDDJSX.effectUnit = [];
    PioneerDDJSX.effectUnit[1] = new components.EffectUnit([1, 3]);
    PioneerDDJSX.effectUnit[2] = new components.EffectUnit([2, 4]);
    PioneerDDJSX.effectUnit[1].enableButtons[1].midi = [0x94, PioneerDDJSX.nonPadLeds.fx1on];
    PioneerDDJSX.effectUnit[1].enableButtons[2].midi = [0x94, PioneerDDJSX.nonPadLeds.fx2on];
    PioneerDDJSX.effectUnit[1].enableButtons[3].midi = [0x94, PioneerDDJSX.nonPadLeds.fx3on];
    PioneerDDJSX.effectUnit[1].effectFocusButton.midi = [0x94, PioneerDDJSX.nonPadLeds.fxTab];
    PioneerDDJSX.effectUnit[1].dryWetKnob.input = function(channel, control, value, status, group) {
        this.inSetParameter(this.inGetParameter() + PioneerDDJSX.getRotaryDelta(value) / 30);
    };
    PioneerDDJSX.effectUnit[1].init();
    PioneerDDJSX.effectUnit[2].enableButtons[1].midi = [0x95, PioneerDDJSX.nonPadLeds.fx1on];
    PioneerDDJSX.effectUnit[2].enableButtons[2].midi = [0x95, PioneerDDJSX.nonPadLeds.fx2on];
    PioneerDDJSX.effectUnit[2].enableButtons[3].midi = [0x95, PioneerDDJSX.nonPadLeds.fx3on];
    PioneerDDJSX.effectUnit[2].effectFocusButton.midi = [0x95, PioneerDDJSX.nonPadLeds.fxTab];
    PioneerDDJSX.effectUnit[2].dryWetKnob.input = function(channel, control, value, status, group) {
        this.inSetParameter(this.inGetParameter() + PioneerDDJSX.getRotaryDelta(value) / 30);
    };
    PioneerDDJSX.effectUnit[2].init();
};

PioneerDDJSX.shutdown = function() {
    PioneerDDJSX.resetDeck("[Channel1]");
    PioneerDDJSX.resetDeck("[Channel2]");
    PioneerDDJSX.resetDeck("[Channel3]");
    PioneerDDJSX.resetDeck("[Channel4]");

    PioneerDDJSX.resetNonDeckLeds();
};


///////////////////////////////////////////////////////////////
//                      VU - METER                           //
///////////////////////////////////////////////////////////////

PioneerDDJSX.vuMeterTwinkle = function() {
    if (engine.getValue("[AutoDJ]", "enabled")) {
        PioneerDDJSX.blinkAutodjState = !PioneerDDJSX.blinkAutodjState;
    }
    PioneerDDJSX.valueVuMeter["[Channel1]_enabled"] = PioneerDDJSX.blinkAutodjState ? 1 : 0;
    PioneerDDJSX.valueVuMeter["[Channel3]_enabled"] = PioneerDDJSX.blinkAutodjState ? 1 : 0;
    PioneerDDJSX.valueVuMeter["[Channel2]_enabled"] = PioneerDDJSX.blinkAutodjState ? 1 : 0;
    PioneerDDJSX.valueVuMeter["[Channel4]_enabled"] = PioneerDDJSX.blinkAutodjState ? 1 : 0;
};


///////////////////////////////////////////////////////////////
//                        AUTO DJ                            //
///////////////////////////////////////////////////////////////

PioneerDDJSX.autodjToggle = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl("[AutoDJ]", "enabled");
    }
};

PioneerDDJSX.autoDJToggleSyncBPM = function(channel, control, value, status, group) {
    if (value) {
        PioneerDDJSX.autoDJSyncBPM = !PioneerDDJSX.autoDJSyncBPM;
        PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.shiftLoadDeck1, PioneerDDJSX.autoDJSyncBPM);
    }
};

PioneerDDJSX.autoDJToggleSyncKey = function(channel, control, value, status, group) {
    if (value) {
        PioneerDDJSX.autoDJSyncKey = !PioneerDDJSX.autoDJSyncKey;
        PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.shiftLoadDeck2, PioneerDDJSX.autoDJSyncKey);
    }
};

PioneerDDJSX.autoDJTimer = function(value, group, control) {
    if (value) {
        PioneerDDJSX.autoDJTickTimer = engine.beginTimer(PioneerDDJSX.autoDJTickInterval, "PioneerDDJSX.autoDJControl()");
    } else if (PioneerDDJSX.autoDJTickTimer) {
        engine.stopTimer(PioneerDDJSX.autoDJTickTimer);
        PioneerDDJSX.autoDJTickTimer = 0;
    }
    engine.setValue("[Channel1]", "quantize", value);
    engine.setValue("[Channel2]", "quantize", value);
};

PioneerDDJSX.autoDJControl = function() {
    var prev = 1,
        next = 2,
        prevPos = 0,
        nextPos = 0,
        nextPlaying = 0,
        prevBpm = 0,
        nextBpm = 0,
        diffBpm = 0,
        diffBpmDouble = 0,
        keyOkay = 0,
        prevKey = 0,
        nextKey = 0,
        diffKey = 0;

    if (!PioneerDDJSX.autoDJSyncBPM && !PioneerDDJSX.autoDJSyncKey) {
        return;
    }

    prevPos = engine.getValue("[Channel" + prev + "]", "playposition");
    nextPos = engine.getValue("[Channel" + next + "]", "playposition");
    if (prevPos < nextPos) {
        var tmp = nextPos;
        nextPos = prevPos;
        prevPos = tmp;
        next = 1;
        prev = 2;
    }
    nextPlaying = engine.getValue("[Channel" + next + "]", "play_indicator");
    prevBpm = engine.getValue("[Channel" + prev + "]", "visual_bpm");
    nextBpm = engine.getValue("[Channel" + next + "]", "visual_bpm");
    diffBpm = Math.abs(nextBpm - prevBpm);
    // diffBpm, with bpm of ONE track doubled
    // Note: Where appropriate, Mixxx will automatically match two beats of one.
    if (nextBpm < prevBpm) {
        diffBpmDouble = Math.abs(2 * nextBpm - prevBpm);
    } else {
        diffBpmDouble = Math.abs(2 * prevBpm - nextBpm);
    }

    // Next track is playing --> Fade in progress
    // Note: play_indicator is falsely true, when analysis is needed and similar
    if (nextPlaying && (nextPos > 0.0)) {
        // Bpm synced up --> disable sync before new track loaded
        // Note: Sometimes, Mixxx does not sync close enough for === operator
        if (diffBpm < 0.01 || diffBpmDouble < 0.01) {
            engine.setValue("[Channel" + prev + "]", "sync_mode", 0.0);
            engine.setValue("[Channel" + next + "]", "sync_mode", 0.0);
        } else { // Synchronize
            engine.setValue("[Channel" + prev + "]", "sync_mode", 1.0); // First,  set prev to follower
            engine.setValue("[Channel" + next + "]", "sync_mode", 2.0); // Second, set next to master
        }

        // Only adjust key when approaching the middle of fading
        if (PioneerDDJSX.autoDJSyncKey) {
            var diffFader = Math.abs(engine.getValue("[Master]", "crossfader") - 0.5);
            if (diffFader < 0.25) {
                nextKey = engine.getValue("[Channel" + next + "]", "key");
                engine.setValue("[Channel" + prev + "]", "key", nextKey);
            }
        }
    } else if (!nextPlaying) { // Next track is stopped --> Disable sync and refine track selection
        // First, disable sync; should be off by now, anyway
        engine.setValue("[Channel" + prev + "]", "sync_mode", 0.0); // Disable sync, else loading new track...
        engine.setValue("[Channel" + next + "]", "sync_mode", 0.0); // ...or skipping tracks would break things.

        // Second, refine track selection
        var skip = 0;
        if (diffBpm > PioneerDDJSX.autoDJMaxBpmAdjustment && diffBpmDouble > PioneerDDJSX.autoDJMaxBpmAdjustment) {
            skip = 1;
        }
        // Mixing in key:
        //     1  the difference is exactly 12 (harmonic switch of tonality), or
        //     2  both are of same tonality, and
        //     2a difference is 0, 1 or 2 (difference of up to two semitones: equal key or energy mix)
        //     2b difference corresponds to neighbours in the circle of fifth (harmonic neighbours)
        //   If neither is the case, we skip.
        if (PioneerDDJSX.autoDJSyncKey) {
            keyOkay = 0;
            prevKey = engine.getValue("[Channel" + prev + "]", "visual_key");
            nextKey = engine.getValue("[Channel" + next + "]", "visual_key");
            diffKey = Math.abs(prevKey - nextKey);
            if (diffKey === 12.0) {
                keyOkay = 1; // Switch of tonality
            }
            // Both of same tonality:
            if ((prevKey < 13 && nextKey < 13) || (prevKey > 12 && nextKey > 12)) {
                if (diffKey < 3.0) {
                    keyOkay = 1; // Equal or Energy
                }
                if (diffKey === 5.0 || diffKey === 7.0) {
                    keyOkay = 1; // Neighbours in Circle of Fifth
                }
            }
            if (!keyOkay) {
                skip = 1;
            }
        }

        if (skip) {
            engine.setValue("[AutoDJ]", "skip_next", 1.0);
            engine.setValue("[AutoDJ]", "skip_next", 0.0); // Have to reset manually
            if (PioneerDDJSX.autoDJShuffleAfterSkip) {
                engine.setValue("[AutoDJ]", "shuffle_playlist", 1.0);
                engine.setValue("[AutoDJ]", "shuffle_playlist", 0.0); // Have to reset manually
            }
        }
    }
};


///////////////////////////////////////////////////////////////
//                      CONTROL BINDING                      //
///////////////////////////////////////////////////////////////

PioneerDDJSX.bindDeckControlConnections = function(channelGroup, bind) {
    var i,
        index,
        deck = PioneerDDJSX.channelGroups[channelGroup],
        controlsToFunctions = {
            'play_indicator': 'PioneerDDJSX.playLed',
            'cue_indicator': 'PioneerDDJSX.cueLed',
            'playposition': 'PioneerDDJSX.wheelLeds',
            'pfl': 'PioneerDDJSX.headphoneCueLed',
            'bpm_tap': 'PioneerDDJSX.shiftHeadphoneCueLed',
            'VuMeter': 'PioneerDDJSX.VuMeterLeds',
            'keylock': 'PioneerDDJSX.keyLockLed',
            'slip_enabled': 'PioneerDDJSX.slipLed',
            'quantize': 'PioneerDDJSX.quantizeLed',
            'loop_in': 'PioneerDDJSX.loopInLed',
            'loop_out': 'PioneerDDJSX.loopOutLed',
            'loop_enabled': 'PioneerDDJSX.autoLoopLed',
            'loop_double': 'PioneerDDJSX.loopDoubleLed',
            'loop_halve': 'PioneerDDJSX.loopHalveLed',
            'reloop_andstop': 'PioneerDDJSX.shiftLoopInLed',
            'beatjump_1_forward': 'PioneerDDJSX.loopShiftFWLed',
            'beatjump_1_backward': 'PioneerDDJSX.loopShiftBKWLed',
            'beatjump_forward': 'PioneerDDJSX.hotCueParameterRightLed',
            'beatjump_backward': 'PioneerDDJSX.hotCueParameterLeftLed',
            'reverse': 'PioneerDDJSX.reverseLed',
            'duration': 'PioneerDDJSX.loadLed',
            'sync_enabled': 'PioneerDDJSX.syncLed',
            'beat_active': 'PioneerDDJSX.slicerBeatActive'
        };

    for (i = 1; i <= 8; i++) {
        controlsToFunctions["hotcue_" + i + "_enabled"] = "PioneerDDJSX.hotCueLeds";
    }

    for (index in PioneerDDJSX.selectedLoopIntervals[deck]) {
        if (PioneerDDJSX.selectedLoopIntervals[deck].hasOwnProperty(index)) {
            controlsToFunctions["beatloop_" + PioneerDDJSX.selectedLoopIntervals[deck][index] + "_enabled"] = "PioneerDDJSX.beatloopLeds";
        }
    }

    for (index in PioneerDDJSX.selectedLooprollIntervals[deck]) {
        if (PioneerDDJSX.selectedLooprollIntervals[deck].hasOwnProperty(index)) {
            controlsToFunctions["beatlooproll_" + PioneerDDJSX.selectedLooprollIntervals[deck][index] + "_activate"] = "PioneerDDJSX.beatlooprollLeds";
        }
    }

    script.bindConnections(channelGroup, controlsToFunctions, !bind);

    for (index in PioneerDDJSX.fxUnitGroups) {
        if (PioneerDDJSX.fxUnitGroups.hasOwnProperty(index)) {
            if (PioneerDDJSX.fxUnitGroups[index] < 2) {
                engine.connectControl(index, "group_" + channelGroup + "_enable", "PioneerDDJSX.fxAssignLeds", !bind);
                if (bind) {
                    engine.trigger(index, "group_" + channelGroup + "_enable");
                }
            }
        }
    }
};

PioneerDDJSX.bindNonDeckControlConnections = function(bind) {
    var index;

    for (index in PioneerDDJSX.samplerGroups) {
        if (PioneerDDJSX.samplerGroups.hasOwnProperty(index)) {
            engine.connectControl(index, "duration", "PioneerDDJSX.samplerLeds", !bind);
            engine.connectControl(index, "play", "PioneerDDJSX.samplerLedsPlay", !bind);
            if (bind) {
                engine.trigger(index, "duration");
            }
        }
    }

    engine.connectControl("[Master]", "headSplit", "PioneerDDJSX.shiftMasterCueLed", !bind);
    if (bind) {
        engine.trigger("[Master]", "headSplit");
    }

    engine.connectControl("[AutoDJ]", "enabled", "PioneerDDJSX.autoDJTimer", !bind);
};


///////////////////////////////////////////////////////////////
//                     DECK INIT / RESET                     //
///////////////////////////////////////////////////////////////

PioneerDDJSX.initDeck = function(group) {
    var deck = PioneerDDJSX.channelGroups[group];

    // save set up speed slider range from the Mixxx settings:
    PioneerDDJSX.setUpSpeedSliderRange[deck] = engine.getValue(group, "rateRange");

    PioneerDDJSX.bindDeckControlConnections(group, true);

    PioneerDDJSX.updateParameterStatusLeds(
        group,
        PioneerDDJSX.selectedLoopRollParam[deck],
        PioneerDDJSX.selectedLoopParam[deck],
        PioneerDDJSX.selectedSamplerBank,
        PioneerDDJSX.selectedSlicerQuantizeParam[deck],
        PioneerDDJSX.selectedSlicerDomainParam[deck]
    );
    PioneerDDJSX.triggerVinylLed(deck);

    PioneerDDJSX.illuminateFunctionControl(
        PioneerDDJSX.illuminationControl["loadedDeck" + (deck + 1)],
        false
    );
    PioneerDDJSX.illuminateFunctionControl(
        PioneerDDJSX.illuminationControl["unknownDeck" + (deck + 1)],
        false
    );
    PioneerDDJSX.wheelLedControl(group, PioneerDDJSX.wheelLedCircle.minVal);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.hotCueMode, true); // set HOT CUE Pad-Mode
};

PioneerDDJSX.resetDeck = function(group) {
    PioneerDDJSX.bindDeckControlConnections(group, false);

    PioneerDDJSX.VuMeterLeds(0x00, group, 0x00); // reset VU meter Leds
    PioneerDDJSX.wheelLedControl(group, PioneerDDJSX.wheelLedCircle.minVal); // reset jogwheel Leds
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.hotCueMode, true); // reset HOT CUE Pad-Mode
    // pad Leds:
    for (var i = 0; i < 8; i++) {
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.hotCue, i, false, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.loopRoll, i, false, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.slicer, i, false, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.sampler, i, false, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.group2, i, false, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.hotCue, i, true, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.loopRoll, i, true, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.slicer, i, true, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.sampler, i, true, false);
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.group2, i, true, false);
    }
    // non pad Leds:
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.headphoneCue, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftHeadphoneCue, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.cue, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftCue, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.keyLock, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftKeyLock, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.play, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftPlay, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.vinyl, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.sync, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftSync, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.autoLoop, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftAutoLoop, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.loopHalve, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftLoopHalve, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.loopIn, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftLoopIn, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.loopOut, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftLoopOut, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.censor, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftCensor, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.slip, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftSlip, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.gridAdjust, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftGridAdjust, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.gridSlide, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftGridSlide, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverPlus, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverMinus, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftRollMode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftSlicerMode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftParameterLeftSlicerMode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftSamplerMode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftGroup2Mode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightRollMode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightSlicerMode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftParameterRightSlicerMode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightSamplerMode, false);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightGroup2Mode, false);
};


///////////////////////////////////////////////////////////////
//            HIGH RESOLUTION MIDI INPUT HANDLERS            //
///////////////////////////////////////////////////////////////

PioneerDDJSX.highResMSB = {
    '[Channel1]': {},
    '[Channel2]': {},
    '[Channel3]': {},
    '[Channel4]': {},
    '[Master]': {},
    '[Samplers]': {}
};

PioneerDDJSX.tempoSliderMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].tempoSlider = value;
};

PioneerDDJSX.tempoSliderLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].tempoSlider << 7) + value,
        sliderRate = 1 - (fullValue / 0x3FFF),
        deck = PioneerDDJSX.channelGroups[group];

    engine.setParameter(group, "rate", sliderRate);

    if (PioneerDDJSX.syncRate[deck] !== 0) {
        if (PioneerDDJSX.syncRate[deck] !== engine.getValue(group, "rate")) {
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverPlus, 0);
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverMinus, 0);
            PioneerDDJSX.syncRate[deck] = 0;
        }
    }
};

PioneerDDJSX.gainKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].gainKnob = value;
};

PioneerDDJSX.gainKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].gainKnob << 7) + value;
    engine.setParameter(group, "pregain", fullValue / 0x3FFF);
};

PioneerDDJSX.filterHighKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].filterHigh = value;
};

PioneerDDJSX.filterHighKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].filterHigh << 7) + value;
    engine.setParameter("[EqualizerRack1_" + group + "_Effect1]", "parameter3", fullValue / 0x3FFF);
};

PioneerDDJSX.filterMidKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].filterMid = value;
};

PioneerDDJSX.filterMidKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].filterMid << 7) + value;
    engine.setParameter("[EqualizerRack1_" + group + "_Effect1]", "parameter2", fullValue / 0x3FFF);
};

PioneerDDJSX.filterLowKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].filterLow = value;
};

PioneerDDJSX.filterLowKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].filterLow << 7) + value;
    engine.setParameter("[EqualizerRack1_" + group + "_Effect1]", "parameter1", fullValue / 0x3FFF);
};

PioneerDDJSX.deckFaderMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].deckFader = value;
};

PioneerDDJSX.deckFaderLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].deckFader << 7) + value;

    if (PioneerDDJSX.shiftPressed &&
        engine.getValue(group, "volume") === 0 &&
        fullValue !== 0 &&
        engine.getValue(group, "play") === 0
    ) {
        PioneerDDJSX.chFaderStart[channel] = engine.getValue(group, "playposition");
        engine.setValue(group, "play", 1);
    } else if (
        PioneerDDJSX.shiftPressed &&
        engine.getValue(group, "volume") !== 0 &&
        fullValue === 0 &&
        engine.getValue(group, "play") === 1 &&
        PioneerDDJSX.chFaderStart[channel] !== null
    ) {
        engine.setValue(group, "play", 0);
        engine.setValue(group, "playposition", PioneerDDJSX.chFaderStart[channel]);
        PioneerDDJSX.chFaderStart[channel] = null;
    }
    engine.setParameter(group, "volume", fullValue / 0x3FFF);
};

PioneerDDJSX.filterKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].filterKnob = value;
};

PioneerDDJSX.filterKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].filterKnob << 7) + value;
    engine.setParameter("[QuickEffectRack1_" + group + "]", "super1", fullValue / 0x3FFF);
};

PioneerDDJSX.crossfaderCurveKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].crossfaderCurveKnob = value;
};

PioneerDDJSX.crossfaderCurveKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].crossfaderCurveKnob << 7) + value;
    script.crossfaderCurve(fullValue, 0x00, 0x3FFF);
};

PioneerDDJSX.samplerVolumeFaderMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].samplerVolumeFader = value;
};

PioneerDDJSX.samplerVolumeFaderLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].samplerVolumeFader << 7) + value;
    for (var i = 1; i <= 32; i++) {
        engine.setParameter("[Sampler" + i + "]", "volume", fullValue / 0x3FFF);
    }
};

PioneerDDJSX.crossFaderMSB = function(channel, control, value, status, group) {
    PioneerDDJSX.highResMSB[group].crossFader = value;
};

PioneerDDJSX.crossFaderLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSX.highResMSB[group].crossFader << 7) + value;
    engine.setParameter(group, "crossfader", fullValue / 0x3FFF);
};


///////////////////////////////////////////////////////////////
//           SINGLE MESSAGE MIDI INPUT HANDLERS              //
///////////////////////////////////////////////////////////////

PioneerDDJSX.shiftButton = function(channel, control, value, status, group) {
    var index = 0;
    PioneerDDJSX.shiftPressed = (value === 0x7F);
    for (index in PioneerDDJSX.chFaderStart) {
        if (typeof index === "number") {
            PioneerDDJSX.chFaderStart[index] = null;
        }
    }
    if (value) {
        PioneerDDJSX.effectUnit[1].shift();
        PioneerDDJSX.effectUnit[2].shift();
    }
    if (!value) {
        PioneerDDJSX.effectUnit[1].unshift();
        PioneerDDJSX.effectUnit[2].unshift();
    }
};

PioneerDDJSX.playButton = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group],
        playing = engine.getValue(group, "play");

    if (value) {
        if (playing) {
            script.brake(channel, control, value, status, group);
            PioneerDDJSX.toggledBrake[deck] = true;
        } else {
            script.toggleControl(group, "play");
        }
    } else {
        if (PioneerDDJSX.toggledBrake[deck]) {
            script.brake(channel, control, value, status, group);
            script.toggleControl(group, "play");
            PioneerDDJSX.toggledBrake[deck] = false;
        }
    }
};

PioneerDDJSX.playStutterButton = function(channel, control, value, status, group) {
    engine.setValue(group, "play_stutter", value ? 1 : 0);
};

PioneerDDJSX.cueButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "cue_default");
};

PioneerDDJSX.jumpToBeginningButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "start_stop");
};

PioneerDDJSX.headphoneCueButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, "pfl");
    }
};

PioneerDDJSX.headphoneShiftCueButton = function(channel, control, value, status, group) {
    if (value) {
        bpm.tapButton(PioneerDDJSX.channelGroups[group] + 1);
    }
};

PioneerDDJSX.headphoneSplitCueButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, "headSplit");
    }
};

PioneerDDJSX.toggleHotCueMode = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    //HOTCUE
    if (value) {
        PioneerDDJSX.activePadMode[deck] = PioneerDDJSX.padModes.hotCue;
        PioneerDDJSX.activeSlicerMode[deck] = PioneerDDJSX.slicerModes.contSlice;
        PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.hotCueMode, value);
    }
};

PioneerDDJSX.toggleBeatloopRollMode = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    //ROLL
    if (value) {
        PioneerDDJSX.activePadMode[deck] = PioneerDDJSX.padModes.loopRoll;
        PioneerDDJSX.activeSlicerMode[deck] = PioneerDDJSX.slicerModes.contSlice;
        PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.rollMode, value);
    }
};

PioneerDDJSX.toggleSlicerMode = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    //SLICER
    if (value) {
        if (PioneerDDJSX.activePadMode[deck] === PioneerDDJSX.padModes.slicer &&
            PioneerDDJSX.activeSlicerMode[deck] === PioneerDDJSX.slicerModes.contSlice) {
            PioneerDDJSX.activeSlicerMode[deck] = PioneerDDJSX.slicerModes.loopSlice;
            engine.setValue(group, "slip_enabled", true);
        } else {
            PioneerDDJSX.activeSlicerMode[deck] = PioneerDDJSX.slicerModes.contSlice;
            engine.setValue(group, "slip_enabled", false);
        }
        PioneerDDJSX.activePadMode[deck] = PioneerDDJSX.padModes.slicer;
        PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.slicerMode, value);
    }
};

PioneerDDJSX.toggleSamplerMode = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    //SAMPLER
    if (value) {
        PioneerDDJSX.activePadMode[deck] = PioneerDDJSX.padModes.sampler;
        PioneerDDJSX.activeSlicerMode[deck] = PioneerDDJSX.slicerModes.contSlice;
        PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.samplerMode, value);
    }
};

PioneerDDJSX.toggleSamplerVelocityMode = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group],
        index = 0;
    PioneerDDJSX.samplerVelocityMode[deck] = value ? true : false;
    if (value) {
        PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.longPressSamplerMode, value);
        for (index = 1; index <= 32; index++) {
            engine.setParameter("[Sampler" + index + "]", "volume", 0);
        }
    } else {
        for (index = 1; index <= 32; index++) {
            engine.setParameter("[Sampler" + index + "]", "volume", 1);
        }
    }
};

PioneerDDJSX.toggleBeatloopMode = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    //GROUP2
    if (value) {
        PioneerDDJSX.activePadMode[deck] = PioneerDDJSX.padModes.beatloop;
        PioneerDDJSX.activeSlicerMode[deck] = PioneerDDJSX.slicerModes.contSlice;
        PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftRollMode, value);
    }
};

PioneerDDJSX.hotCueButtons = function(channel, control, value, status, group) {
    var index = control + 1;
    script.toggleControl(group, "hotcue_" + index + "_activate");
};

PioneerDDJSX.clearHotCueButtons = function(channel, control, value, status, group) {
    var index = control - 0x08 + 1;
    script.toggleControl(group, "hotcue_" + index + "_clear");
};

PioneerDDJSX.beatloopButtons = function(channel, control, value, status, group) {
    var index = control - 0x50,
        deck = PioneerDDJSX.channelGroups[group];
    script.toggleControl(
        group,
        "beatloop_" + PioneerDDJSX.selectedLoopIntervals[deck][index] + "_toggle"
    );
};

PioneerDDJSX.slicerButtons = function(channel, control, value, status, group) {
    var index = control - 0x20,
        deck = PioneerDDJSX.channelGroups[group],
        domain = PioneerDDJSX.selectedSlicerDomain[deck],
        beatsToJump = 0;

    if (PioneerDDJSX.activeSlicerMode[deck] === PioneerDDJSX.slicerModes.loopSlice) {
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.slicer, index, false, !value);
    } else {
        PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.slicer, index, false, value);
    }
    PioneerDDJSX.slicerActive[deck] = value ? true : false;
    PioneerDDJSX.slicerButton[deck] = index;

    if (value) {
        beatsToJump = (PioneerDDJSX.slicerButton[deck] * (domain / 8)) - ((PioneerDDJSX.slicerBeatsPassed[deck] % domain) + 1);
        if (PioneerDDJSX.slicerButton[deck] === 0 && beatsToJump === -domain) {
            beatsToJump = 0;
        }
        if (PioneerDDJSX.slicerBeatsPassed[deck] >= Math.abs(beatsToJump) &&
            PioneerDDJSX.slicerPreviousBeatsPassed[deck] !== PioneerDDJSX.slicerBeatsPassed[deck]) {
            PioneerDDJSX.slicerPreviousBeatsPassed[deck] = PioneerDDJSX.slicerBeatsPassed[deck];
            if (Math.abs(beatsToJump) > 0) {
                engine.setValue(group, "beatjump", beatsToJump);
            }
        }
    }

    if (PioneerDDJSX.activeSlicerMode[deck] === PioneerDDJSX.slicerModes.contSlice) {
        engine.setValue(group, "slip_enabled", value);
        engine.setValue(group, "beatloop_size", PioneerDDJSX.selectedSlicerQuantization[deck]);
        engine.setValue(group, "beatloop_activate", value);
    }
};

PioneerDDJSX.beatloopRollButtons = function(channel, control, value, status, group) {
    var index = control - 0x10,
        deck = PioneerDDJSX.channelGroups[group];
    script.toggleControl(
        group,
        "beatlooproll_" + PioneerDDJSX.selectedLooprollIntervals[deck][index] + "_activate"
    );
};

PioneerDDJSX.samplerButtons = function(channel, control, value, status, group) {
    var index = control - 0x30 + 1,
        deckOffset = PioneerDDJSX.selectedSamplerBank * 8,
        sampleDeck = "[Sampler" + (index + deckOffset) + "]",
        playMode = PioneerDDJSX.samplerCueGotoAndPlay ? "cue_gotoandplay" : "start_play";

    if (engine.getValue(sampleDeck, "track_loaded")) {
        engine.setValue(sampleDeck, playMode, value ? 1 : 0);
    } else {
        engine.setValue(sampleDeck, "LoadSelectedTrack", value ? 1 : 0);
    }
};

PioneerDDJSX.stopSamplerButtons = function(channel, control, value, status, group) {
    var index = control - 0x38 + 1,
        deckOffset = PioneerDDJSX.selectedSamplerBank * 8,
        sampleDeck = "[Sampler" + (index + deckOffset) + "]",
        trackLoaded = engine.getValue(sampleDeck, "track_loaded"),
        playing = engine.getValue(sampleDeck, "play");

    if (trackLoaded && playing) {
        script.toggleControl(sampleDeck, "stop");
    } else if (trackLoaded && !playing && value) {
        script.toggleControl(sampleDeck, "eject");
    }
};

PioneerDDJSX.samplerVelocityVolume = function(channel, control, value, status, group) {
    var index = control - 0x30 + 1,
        deck = PioneerDDJSX.channelGroups[group],
        deckOffset = PioneerDDJSX.selectedSamplerBank * 8,
        sampleDeck = "[Sampler" + (index + deckOffset) + "]",
        vol = value / 0x7F;

    if (PioneerDDJSX.samplerVelocityMode[deck]) {
        engine.setParameter(sampleDeck, "volume", vol);
    }
};

PioneerDDJSX.changeParameters = function(group, ctrl, value) {
    var deck = PioneerDDJSX.channelGroups[group],
        index,
        offset = 0,
        samplerIndex = 0,
        beatjumpSize = 0;

    //Hot Cue Mode:
    if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftHotCueMode) {
        engine.setValue(group, "beatjump_backward", value);
    }
    if (ctrl === PioneerDDJSX.nonPadLeds.parameterRightHotCueMode) {
        engine.setValue(group, "beatjump_forward", value);
    }
    if (ctrl === PioneerDDJSX.nonPadLeds.shiftParameterLeftHotCueMode) {
        PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftParameterLeftHotCueMode, value);
        if (value) {
            beatjumpSize = engine.getValue(group, "beatjump_size");
            engine.setValue(group, "beatjump_size", beatjumpSize / 2);
        }
    }
    if (ctrl === PioneerDDJSX.nonPadLeds.shiftParameterRightHotCueMode) {
        PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftParameterRightHotCueMode, value);
        if (value) {
            beatjumpSize = engine.getValue(group, "beatjump_size");
            engine.setValue(group, "beatjump_size", beatjumpSize * 2);
        }
    }

    // ignore other cases if button is released:
    if (!value) {
        return;
    }

    //Roll Mode:
    if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftRollMode || ctrl === PioneerDDJSX.nonPadLeds.parameterRightRollMode) {
        // unbind previous connected controls:
        for (index in PioneerDDJSX.selectedLooprollIntervals[deck]) {
            if (PioneerDDJSX.selectedLooprollIntervals[deck].hasOwnProperty(index)) {
                engine.connectControl(
                    group,
                    "beatlooproll_" + PioneerDDJSX.selectedLooprollIntervals[deck][index] + "_activate",
                    "PioneerDDJSX.beatlooprollLeds",
                    true
                );
            }
        }
        // change parameter set:
        if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftRollMode && PioneerDDJSX.selectedLoopRollParam[deck] > 0) {
            PioneerDDJSX.selectedLoopRollParam[deck] -= 1;
        } else if (ctrl === PioneerDDJSX.nonPadLeds.parameterRightRollMode && PioneerDDJSX.selectedLoopRollParam[deck] < 3) {
            PioneerDDJSX.selectedLoopRollParam[deck] += 1;
        }
        PioneerDDJSX.selectedLooprollIntervals[deck] = PioneerDDJSX.loopIntervals[PioneerDDJSX.selectedLoopRollParam[deck]];
        // bind new controls:
        for (index in PioneerDDJSX.selectedLooprollIntervals[deck]) {
            if (PioneerDDJSX.selectedLooprollIntervals[deck].hasOwnProperty(index)) {
                engine.connectControl(
                    group,
                    "beatlooproll_" + PioneerDDJSX.selectedLooprollIntervals[deck][index] + "_activate",
                    "PioneerDDJSX.beatlooprollLeds",
                    false
                );
            }
        }
    }

    //Group2 (Beatloop) Mode:
    if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftGroup2Mode || ctrl === PioneerDDJSX.nonPadLeds.parameterRightGroup2Mode) {
        // unbind previous connected controls:
        for (index in PioneerDDJSX.selectedLoopIntervals[deck]) {
            if (PioneerDDJSX.selectedLoopIntervals[deck].hasOwnProperty(index)) {
                engine.connectControl(
                    group,
                    "beatloop_" + PioneerDDJSX.selectedLoopIntervals[deck][index] + "_enabled",
                    "PioneerDDJSX.beatloopLeds",
                    true
                );
            }
        }
        // change parameter set:
        if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftGroup2Mode && PioneerDDJSX.selectedLoopParam[deck] > 0) {
            PioneerDDJSX.selectedLoopParam[deck] -= 1;
        } else if (ctrl === PioneerDDJSX.nonPadLeds.parameterRightGroup2Mode && PioneerDDJSX.selectedLoopParam[deck] < 3) {
            PioneerDDJSX.selectedLoopParam[deck] += 1;
        }
        PioneerDDJSX.selectedLoopIntervals[deck] = PioneerDDJSX.loopIntervals[PioneerDDJSX.selectedLoopParam[deck]];
        // bind new controls:
        for (index in PioneerDDJSX.selectedLoopIntervals[deck]) {
            if (PioneerDDJSX.selectedLoopIntervals[deck].hasOwnProperty(index)) {
                engine.connectControl(
                    group,
                    "beatloop_" + PioneerDDJSX.selectedLoopIntervals[deck][index] + "_enabled",
                    "PioneerDDJSX.beatloopLeds",
                    false
                );
            }
        }
    }

    //Sampler Mode:
    if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftSamplerMode || ctrl === PioneerDDJSX.nonPadLeds.parameterRightSamplerMode) {
        // unbind previous connected controls:
        for (index in PioneerDDJSX.samplerGroups) {
            if (PioneerDDJSX.samplerGroups.hasOwnProperty(index)) {
                offset = PioneerDDJSX.selectedSamplerBank * 8;
                samplerIndex = (PioneerDDJSX.samplerGroups[index] + 1) + offset;
                engine.connectControl(
                    "[Sampler" + samplerIndex + "]",
                    "duration",
                    "PioneerDDJSX.samplerLeds",
                    true
                );
                engine.connectControl(
                    "[Sampler" + samplerIndex + "]",
                    "play",
                    "PioneerDDJSX.samplerLedsPlay",
                    true
                );
            }
        }
        // change sampler bank:
        if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftSamplerMode && PioneerDDJSX.selectedSamplerBank > 0) {
            PioneerDDJSX.selectedSamplerBank -= 1;
        } else if (ctrl === PioneerDDJSX.nonPadLeds.parameterRightSamplerMode && PioneerDDJSX.selectedSamplerBank < 3) {
            PioneerDDJSX.selectedSamplerBank += 1;
        }
        // bind new controls:
        for (index in PioneerDDJSX.samplerGroups) {
            if (PioneerDDJSX.samplerGroups.hasOwnProperty(index)) {
                offset = PioneerDDJSX.selectedSamplerBank * 8;
                samplerIndex = (PioneerDDJSX.samplerGroups[index] + 1) + offset;
                engine.connectControl(
                    "[Sampler" + samplerIndex + "]",
                    "duration",
                    "PioneerDDJSX.samplerLeds",
                    false
                );
                engine.connectControl(
                    "[Sampler" + samplerIndex + "]",
                    "play",
                    "PioneerDDJSX.samplerLedsPlay",
                    false
                );
                engine.trigger("[Sampler" + samplerIndex + "]", "duration");
            }
        }
    }

    //Slicer Mode:
    if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftSlicerMode || ctrl === PioneerDDJSX.nonPadLeds.parameterRightSlicerMode) {
        // change parameter set:
        if (ctrl === PioneerDDJSX.nonPadLeds.parameterLeftSlicerMode && PioneerDDJSX.selectedSlicerQuantizeParam[deck] > 0) {
            PioneerDDJSX.selectedSlicerQuantizeParam[deck] -= 1;
        } else if (ctrl === PioneerDDJSX.nonPadLeds.parameterRightSlicerMode && PioneerDDJSX.selectedSlicerQuantizeParam[deck] < 3) {
            PioneerDDJSX.selectedSlicerQuantizeParam[deck] += 1;
        }
        PioneerDDJSX.selectedSlicerQuantization[deck] = PioneerDDJSX.slicerQuantizations[PioneerDDJSX.selectedSlicerQuantizeParam[deck]];
    }
    //Slicer Mode + SHIFT:
    if (ctrl === PioneerDDJSX.nonPadLeds.shiftParameterLeftSlicerMode || ctrl === PioneerDDJSX.nonPadLeds.shiftParameterRightSlicerMode) {
        // change parameter set:
        if (ctrl === PioneerDDJSX.nonPadLeds.shiftParameterLeftSlicerMode && PioneerDDJSX.selectedSlicerDomainParam[deck] > 0) {
            PioneerDDJSX.selectedSlicerDomainParam[deck] -= 1;
        } else if (ctrl === PioneerDDJSX.nonPadLeds.shiftParameterRightSlicerMode && PioneerDDJSX.selectedSlicerDomainParam[deck] < 3) {
            PioneerDDJSX.selectedSlicerDomainParam[deck] += 1;
        }
        PioneerDDJSX.selectedSlicerDomain[deck] = PioneerDDJSX.slicerDomains[PioneerDDJSX.selectedSlicerDomainParam[deck]];
    }

    // update parameter status leds:
    PioneerDDJSX.updateParameterStatusLeds(
        group,
        PioneerDDJSX.selectedLoopRollParam[deck],
        PioneerDDJSX.selectedLoopParam[deck],
        PioneerDDJSX.selectedSamplerBank,
        PioneerDDJSX.selectedSlicerQuantizeParam[deck],
        PioneerDDJSX.selectedSlicerDomainParam[deck]
    );
};

PioneerDDJSX.parameterLeft = function(channel, control, value, status, group) {
    PioneerDDJSX.changeParameters(group, control, value);
};

PioneerDDJSX.parameterRight = function(channel, control, value, status, group) {
    PioneerDDJSX.changeParameters(group, control, value);
};

PioneerDDJSX.shiftParameterLeft = function(channel, control, value, status, group) {
    PioneerDDJSX.changeParameters(group, control, value);
};

PioneerDDJSX.shiftParameterRight = function(channel, control, value, status, group) {
    PioneerDDJSX.changeParameters(group, control, value);
};

PioneerDDJSX.vinylButton = function(channel, control, value, status, group) {
    PioneerDDJSX.toggleScratch(channel, control, value, status, group);
};

PioneerDDJSX.slipButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, "slip_enabled");
    }
};

PioneerDDJSX.keyLockButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, "keylock");
    }
};

PioneerDDJSX.shiftKeyLockButton = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group],
        range = engine.getValue(group, "rateRange");

    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftKeyLock, value);

    if (range === 0.90) {
        range = PioneerDDJSX.setUpSpeedSliderRange[deck];
    } else if ((range * 2) > 0.90) {
        range = 0.90;
    } else {
        range = range * 2;
    }

    if (value) {
        engine.setValue(group, "rateRange", range);
    }
};

PioneerDDJSX.tempoResetButton = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    if (value) {
        engine.setValue(group, "rate", 0);
        if (PioneerDDJSX.syncRate[deck] !== engine.getValue(group, "rate")) {
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverPlus, 0);
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverMinus, 0);
            PioneerDDJSX.syncRate[deck] = 0;
        }
    }
};

PioneerDDJSX.autoLoopButton = function(channel, control, value, status, group) {
    if (value) {
        if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "reloop_toggle", true);
            engine.setValue(group, "reloop_toggle", false);
        } else {
            engine.setValue(group, "beatloop_activate", true);
            engine.setValue(group, "beatloop_activate", false);
        }
    }
};

PioneerDDJSX.loopActiveButton = function(channel, control, value, status, group) {
    engine.setValue(group, "reloop_toggle", value);
};

PioneerDDJSX.loopInButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "loop_in");
};

PioneerDDJSX.shiftLoopInButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "reloop_andstop");
};

PioneerDDJSX.loopOutButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "loop_out");
};

PioneerDDJSX.loopExitButton = function(channel, control, value, status, group) {
    engine.setValue(group, "reloop_toggle", value);
};

PioneerDDJSX.loopHalveButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "loop_halve");
};

PioneerDDJSX.loopDoubleButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "loop_double");
};

PioneerDDJSX.loopMoveBackButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "beatjump_1_backward");
};

PioneerDDJSX.loopMoveForwardButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "beatjump_1_forward");
};

PioneerDDJSX.loadButton = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(group, "LoadSelectedTrack", true);
        if (PioneerDDJSX.autoPFL) {
            for (var index in PioneerDDJSX.channelGroups) {
                if (PioneerDDJSX.channelGroups.hasOwnProperty(index)) {
                    if (index === group) {
                        engine.setValue(index, "pfl", true);
                    } else {
                        engine.setValue(index, "pfl", false);
                    }
                }
            }
        }
    }
};

PioneerDDJSX.crossfaderAssignCenter = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(group, "orientation", 1);
    }
};

PioneerDDJSX.crossfaderAssignLeft = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(group, "orientation", 0);
    }
};

PioneerDDJSX.crossfaderAssignRight = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(group, "orientation", 2);
    }
};

PioneerDDJSX.reverseRollButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "reverseroll");
};

PioneerDDJSX.reverseButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "reverse");
};

PioneerDDJSX.gridAdjustButton = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];

    PioneerDDJSX.gridAdjustSelected[deck] = value ? true : false;
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.gridAdjust, value);
};

PioneerDDJSX.gridSetButton = function(channel, control, value, status, group) {
    script.toggleControl(group, "beats_translate_curpos");
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftGridAdjust, value);
};

PioneerDDJSX.gridSlideButton = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];

    PioneerDDJSX.gridSlideSelected[deck] = value ? true : false;
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.gridSlide, value);
};

PioneerDDJSX.syncButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, "sync_enabled");
    }
};

PioneerDDJSX.quantizeButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, "quantize");
    }
};

PioneerDDJSX.needleSearchTouch = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    if (engine.getValue(group, "play")) {
        PioneerDDJSX.needleSearchTouched[deck] = PioneerDDJSX.shiftPressed && (value ? true : false);
    } else {
        PioneerDDJSX.needleSearchTouched[deck] = value ? true : false;
    }
};

PioneerDDJSX.needleSearchStripPosition = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    if (PioneerDDJSX.needleSearchTouched[deck]) {
        var position = value / 0x7F;
        engine.setValue(group, "playposition", position);
    }
};

PioneerDDJSX.panelSelectButton = function(channel, control, value, status, group) {
    if (value) {
        if ((PioneerDDJSX.panels[0] === false) && (PioneerDDJSX.panels[1] === false)) {
            PioneerDDJSX.panels[0] = true;
        } else if ((PioneerDDJSX.panels[0] === true) && (PioneerDDJSX.panels[1] === false)) {
            PioneerDDJSX.panels[1] = true;
        } else if ((PioneerDDJSX.panels[0] === true) && (PioneerDDJSX.panels[1] === true)) {
            PioneerDDJSX.panels[0] = false;
        } else if ((PioneerDDJSX.panels[0] === false) && (PioneerDDJSX.panels[1] === true)) {
            PioneerDDJSX.panels[1] = false;
        }

        engine.setValue("[Samplers]", "show_samplers", PioneerDDJSX.panels[0]);
        engine.setValue("[EffectRack1]", "show", PioneerDDJSX.panels[1]);
    }
};

PioneerDDJSX.shiftPanelSelectButton = function(channel, control, value, status, group) {
    var channelGroup;
    PioneerDDJSX.shiftPanelSelectPressed = value ? true : false;
    
    for (var index in PioneerDDJSX.fxUnitGroups) {
        if (PioneerDDJSX.fxUnitGroups.hasOwnProperty(index)) {
            if (PioneerDDJSX.fxUnitGroups[index] < 2) {
                for (channelGroup in PioneerDDJSX.channelGroups) {
                    if (PioneerDDJSX.channelGroups.hasOwnProperty(channelGroup)) {
                        engine.connectControl(index, "group_" + channelGroup + "_enable", "PioneerDDJSX.fxAssignLeds", value);
                        if (value) {
                            engine.trigger(index, "group_" + channelGroup + "_enable");
                        }
                    }
                }
            }
            if (PioneerDDJSX.fxUnitGroups[index] >= 2) {
                for (channelGroup in PioneerDDJSX.channelGroups) {
                    if (PioneerDDJSX.channelGroups.hasOwnProperty(channelGroup)) {
                        engine.connectControl(index, "group_" + channelGroup + "_enable", "PioneerDDJSX.fxAssignLeds", !value);
                        if (value) {
                            engine.trigger(index, "group_" + channelGroup + "_enable");
                        } else {
                            PioneerDDJSX.fxAssignLedControl(index, PioneerDDJSX.channelGroups[channelGroup], false);
                        }
                    }
                }
            }
        }
    }
};


///////////////////////////////////////////////////////////////
//                          LED HELPERS                      //
///////////////////////////////////////////////////////////////

PioneerDDJSX.deckConverter = function(group) {
    if (PioneerDDJSX.channelGroups.hasOwnProperty(group)) {
        return PioneerDDJSX.channelGroups[group];
    }
    return group;
};

PioneerDDJSX.flashLedState = 0;

PioneerDDJSX.flashLed = function(deck, ledNumber) {
    if (PioneerDDJSX.flashLedState === 0) {
        PioneerDDJSX.nonPadLedControl(deck, ledNumber, 1);
        PioneerDDJSX.flashLedState = 1;
    } else if (PioneerDDJSX.flashLedState === 1) {
        PioneerDDJSX.nonPadLedControl(deck, ledNumber, 0);
        PioneerDDJSX.flashLedState = 0;
    }
};

PioneerDDJSX.resetNonDeckLeds = function() {
    var indexFxUnit;

    // fx Leds:
    for (indexFxUnit in PioneerDDJSX.fxUnitGroups) {
        if (PioneerDDJSX.fxUnitGroups.hasOwnProperty(indexFxUnit)) {
            if (PioneerDDJSX.fxUnitGroups[indexFxUnit] < 2) {
                for (var indexFxLed in PioneerDDJSX.fxEffectGroups) {
                    if (PioneerDDJSX.fxEffectGroups.hasOwnProperty(indexFxLed)) {
                        PioneerDDJSX.fxLedControl(
                            PioneerDDJSX.fxUnitGroups[indexFxUnit],
                            PioneerDDJSX.fxEffectGroups[indexFxLed],
                            false,
                            false
                        );
                        PioneerDDJSX.fxLedControl(
                            PioneerDDJSX.fxUnitGroups[indexFxUnit],
                            PioneerDDJSX.fxEffectGroups[indexFxLed],
                            true,
                            false
                        );
                    }
                }
                PioneerDDJSX.fxLedControl(PioneerDDJSX.fxUnitGroups[indexFxUnit], 0x03, false, false);
                PioneerDDJSX.fxLedControl(PioneerDDJSX.fxUnitGroups[indexFxUnit], 0x03, true, false);
            }
        }
    }

    // fx assign Leds:
    for (indexFxUnit in PioneerDDJSX.fxUnitGroups) {
        if (PioneerDDJSX.fxUnitGroups.hasOwnProperty(indexFxUnit)) {
            for (var channelGroup in PioneerDDJSX.channelGroups) {
                if (PioneerDDJSX.channelGroups.hasOwnProperty(channelGroup)) {
                    PioneerDDJSX.fxAssignLedControl(
                        indexFxUnit,
                        PioneerDDJSX.channelGroups[channelGroup],
                        false
                    );
                }
            }
        }
    }

    // general Leds:
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.shiftMasterCue, false);
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.loadDeck1, false);
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.shiftLoadDeck1, false);
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.loadDeck2, false);
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.shiftLoadDeck2, false);
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.loadDeck3, false);
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.shiftLoadDeck3, false);
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.loadDeck4, false);
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.shiftLoadDeck4, false);
};

PioneerDDJSX.fxAssignLedControl = function(unit, ledNumber, active) {
    var fxAssignLedsBaseChannel = 0x96,
        fxAssignLedsBaseControl = 0;

    if (unit === "[EffectRack1_EffectUnit1]") {
        fxAssignLedsBaseControl = PioneerDDJSX.nonPadLeds.fx1assignDeck1;
    }
    if (unit === "[EffectRack1_EffectUnit2]") {
        fxAssignLedsBaseControl = PioneerDDJSX.nonPadLeds.fx2assignDeck1;
    }
    if (unit === "[EffectRack1_EffectUnit3]") {
        fxAssignLedsBaseControl = PioneerDDJSX.nonPadLeds.shiftFx1assignDeck1;
    }
    if (unit === "[EffectRack1_EffectUnit4]") {
        fxAssignLedsBaseControl = PioneerDDJSX.nonPadLeds.shiftFx2assignDeck1;
    }

    midi.sendShortMsg(
        fxAssignLedsBaseChannel,
        fxAssignLedsBaseControl + ledNumber,
        active ? 0x7F : 0x00
    );
};

PioneerDDJSX.fxLedControl = function(unit, ledNumber, shift, active) {
    var fxLedsBaseChannel = 0x94,
        fxLedsBaseControl = (shift ? 0x63 : 0x47);

    midi.sendShortMsg(
        fxLedsBaseChannel + unit,
        fxLedsBaseControl + ledNumber,
        active ? 0x7F : 0x00
    );
};

PioneerDDJSX.padLedControl = function(deck, groupNumber, ledNumber, shift, active) {
    var padLedsBaseChannel = 0x97,
        padLedControl = (shift ? 0x08 : 0x00) + groupNumber + ledNumber,
        midiChannelOffset = PioneerDDJSX.deckConverter(deck);

    if (midiChannelOffset !== null) {
        midi.sendShortMsg(
            padLedsBaseChannel + midiChannelOffset,
            padLedControl,
            active ? 0x7F : 0x00
        );
    }
};

PioneerDDJSX.nonPadLedControl = function(deck, ledNumber, active) {
    var nonPadLedsBaseChannel = 0x90,
        midiChannelOffset = PioneerDDJSX.deckConverter(deck);

    if (midiChannelOffset !== null) {
        midi.sendShortMsg(
            nonPadLedsBaseChannel + midiChannelOffset,
            ledNumber,
            active ? 0x7F : 0x00
        );
    }
};

PioneerDDJSX.illuminateFunctionControl = function(ledNumber, active) {
    var illuminationBaseChannel = 0x9B;

    midi.sendShortMsg(
        illuminationBaseChannel,
        ledNumber,
        active ? 0x7F : 0x00
    );
};

PioneerDDJSX.wheelLedControl = function(deck, ledNumber) {
    var wheelLedBaseChannel = 0xBB,
        channel = PioneerDDJSX.deckConverter(deck);

    if (channel !== null) {
        midi.sendShortMsg(
            wheelLedBaseChannel,
            channel,
            ledNumber
        );
    }
};

PioneerDDJSX.generalLedControl = function(ledNumber, active) {
    var generalLedBaseChannel = 0x96;

    midi.sendShortMsg(
        generalLedBaseChannel,
        ledNumber,
        active ? 0x7F : 0x00
    );
};

PioneerDDJSX.updateParameterStatusLeds = function(group, statusRoll, statusLoop, statusSampler, statusSlicerQuant, statusSlicerDomain) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftRollMode, statusRoll & (1 << 1));
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightRollMode, statusRoll & 1);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftGroup2Mode, statusLoop & (1 << 1));
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightGroup2Mode, statusLoop & 1);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftSamplerMode, statusSampler & (1 << 1));
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightSamplerMode, statusSampler & 1);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftSlicerMode, statusSlicerQuant & (1 << 1));
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightSlicerMode, statusSlicerQuant & 1);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftParameterLeftSlicerMode, statusSlicerDomain & (1 << 1));
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftParameterRightSlicerMode, statusSlicerDomain & 1);
};


///////////////////////////////////////////////////////////////
//                             LEDS                          //
///////////////////////////////////////////////////////////////

PioneerDDJSX.fxAssignLeds = function(value, group, control) {
    var channelGroup = control.replace("group_", '').replace("_enable", '');
    PioneerDDJSX.fxAssignLedControl(group, PioneerDDJSX.channelGroups[channelGroup], value);
};

PioneerDDJSX.headphoneCueLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.headphoneCue, value);
};

PioneerDDJSX.shiftHeadphoneCueLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftHeadphoneCue, value);
};

PioneerDDJSX.shiftMasterCueLed = function(value, group, control) {
    PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds.shiftMasterCue, value);
};

PioneerDDJSX.keyLockLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.keyLock, value);
};

PioneerDDJSX.playLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.play, value);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftPlay, value);
};

PioneerDDJSX.wheelLeds = function(value, group, control) {
    // Timing calculation is handled in seconds!
    var deck = PioneerDDJSX.channelGroups[group],
        duration = engine.getValue(group, "duration"),
        elapsedTime = value * duration,
        remainingTime = duration - elapsedTime,
        revolutionsPerSecond = PioneerDDJSX.scratchSettings.vinylSpeed / 60,
        speed = parseInt(revolutionsPerSecond * PioneerDDJSX.wheelLedCircle.maxVal),
        wheelPos = PioneerDDJSX.wheelLedCircle.minVal;

    if (value >= 0) {
        wheelPos = PioneerDDJSX.wheelLedCircle.minVal + 0x01 + ((speed * elapsedTime) % PioneerDDJSX.wheelLedCircle.maxVal);
    } else {
        wheelPos = PioneerDDJSX.wheelLedCircle.maxVal + 0x01 + ((speed * elapsedTime) % PioneerDDJSX.wheelLedCircle.maxVal);
    }
    // let wheel LEDs blink if remaining time is less than 30s:
    if (remainingTime > 0 && remainingTime < 30 && !engine.isScratching(deck + 1)) {
        var blinkInterval = parseInt(remainingTime / 3); //increase blinking according time left
        if (blinkInterval < 3) {
            blinkInterval = 3;
        }
        if (PioneerDDJSX.wheelLedsBlinkStatus[deck] < blinkInterval) {
            wheelPos = PioneerDDJSX.wheelLedCircle.minVal;
        } else if (PioneerDDJSX.wheelLedsBlinkStatus[deck] > (blinkInterval - parseInt(6 / blinkInterval))) {
            PioneerDDJSX.wheelLedsBlinkStatus[deck] = 0;
        }
        PioneerDDJSX.wheelLedsBlinkStatus[deck]++;
    }
    PioneerDDJSX.wheelLedControl(group, wheelPos);
};

PioneerDDJSX.cueLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.cue, value);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftCue, value);
};

PioneerDDJSX.loadLed = function(value, group, control) {
    var deck = PioneerDDJSX.channelGroups[group];
    if (value > 0) {
        PioneerDDJSX.wheelLedControl(group, PioneerDDJSX.wheelLedCircle.maxVal);
        PioneerDDJSX.generalLedControl(PioneerDDJSX.nonPadLeds["loadDeck" + (deck + 1)], true);
        PioneerDDJSX.illuminateFunctionControl(PioneerDDJSX.illuminationControl["loadedDeck" + (deck + 1)], true);
        engine.trigger(group, "playposition");
    } else {
        PioneerDDJSX.wheelLedControl(group, PioneerDDJSX.wheelLedCircle.minVal);
    }
};

PioneerDDJSX.reverseLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.censor, value);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftCensor, value);
};

PioneerDDJSX.slipLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.slip, value);
};

PioneerDDJSX.quantizeLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftSync, value);
};

PioneerDDJSX.syncLed = function(value, group, control) {
    var deck = PioneerDDJSX.channelGroups[group];
    var rate = engine.getValue(group, "rate");
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.sync, value);
    if (value) {
        PioneerDDJSX.syncRate[deck] = rate;
        if (PioneerDDJSX.syncRate[deck] > 0) {
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverMinus, 1);
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverPlus, 0);
        } else if (PioneerDDJSX.syncRate[deck] < 0) {
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverMinus, 0);
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverPlus, 1);
        }
    }
    if (!value) {
        if (PioneerDDJSX.syncRate[deck] !== rate || rate === 0) {
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverPlus, 0);
            PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.takeoverMinus, 0);
            PioneerDDJSX.syncRate[deck] = 0;
        }
    }
};

PioneerDDJSX.autoLoopLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.autoLoop, value);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftLoopOut, value);
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftAutoLoop, value);
};

PioneerDDJSX.loopInLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.loopIn, value);
};

PioneerDDJSX.shiftLoopInLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftLoopIn, value);
};

PioneerDDJSX.loopOutLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.loopOut, value);
};

PioneerDDJSX.loopHalveLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.loopHalve, value);
};

PioneerDDJSX.loopDoubleLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.loopDouble, value);
};

PioneerDDJSX.loopShiftFWLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftLoopDouble, value);
};

PioneerDDJSX.loopShiftBKWLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.shiftLoopHalve, value);
};

PioneerDDJSX.hotCueParameterRightLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterRightHotCueMode, value);
};

PioneerDDJSX.hotCueParameterLeftLed = function(value, group, control) {
    PioneerDDJSX.nonPadLedControl(group, PioneerDDJSX.nonPadLeds.parameterLeftHotCueMode, value);
};

PioneerDDJSX.samplerLeds = function(value, group, control) {
    var samplerIndex = (group.replace("[Sampler", '').replace(']', '') - 1) % 8,
        sampleDeck = "[Sampler" + (samplerIndex + 1) + "]",
        padNum = PioneerDDJSX.samplerGroups[sampleDeck];

    for (var index in PioneerDDJSX.channelGroups) {
        if (PioneerDDJSX.channelGroups.hasOwnProperty(index)) {
            PioneerDDJSX.padLedControl(
                PioneerDDJSX.channelGroups[index],
                PioneerDDJSX.ledGroups.sampler,
                padNum,
                false,
                value
            );
        }
    }
};

PioneerDDJSX.samplerLedsPlay = function(value, group, control) {
    var samplerIndex = (group.replace("[Sampler", '').replace(']', '') - 1) % 8,
        sampleDeck = "[Sampler" + (samplerIndex + 1) + "]",
        padNum = PioneerDDJSX.samplerGroups[sampleDeck];

    if (!engine.getValue(sampleDeck, "duration")) {
        return;
    }

    for (var index in PioneerDDJSX.channelGroups) {
        if (PioneerDDJSX.channelGroups.hasOwnProperty(index)) {
            PioneerDDJSX.padLedControl(
                PioneerDDJSX.channelGroups[index],
                PioneerDDJSX.ledGroups.sampler,
                padNum,
                false, !value
            );
            PioneerDDJSX.padLedControl(
                PioneerDDJSX.channelGroups[index],
                PioneerDDJSX.ledGroups.sampler,
                padNum,
                true,
                value
            );
        }
    }
};

PioneerDDJSX.beatloopLeds = function(value, group, control) {
    var padNum,
        shifted = false,
        deck = PioneerDDJSX.channelGroups[group];

    for (var index in PioneerDDJSX.selectedLoopIntervals[deck]) {
        if (PioneerDDJSX.selectedLoopIntervals[deck].hasOwnProperty(index)) {
            if (control === "beatloop_" + PioneerDDJSX.selectedLoopIntervals[deck][index] + "_enabled") {
                padNum = index % 8;
                PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.group2, padNum, shifted, value);
            }
        }
    }
};

PioneerDDJSX.beatlooprollLeds = function(value, group, control) {
    var padNum,
        shifted = false,
        deck = PioneerDDJSX.channelGroups[group];

    for (var index in PioneerDDJSX.selectedLooprollIntervals[deck]) {
        if (PioneerDDJSX.selectedLooprollIntervals[deck].hasOwnProperty(index)) {
            if (control === "beatlooproll_" + PioneerDDJSX.selectedLooprollIntervals[deck][index] + "_activate") {
                padNum = index % 8;
                PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.loopRoll, padNum, shifted, value);
            }
        }
    }
};

PioneerDDJSX.hotCueLeds = function(value, group, control) {
    var padNum = null,
        hotCueNum;

    for (hotCueNum = 1; hotCueNum <= 8; hotCueNum++) {
        if (control === "hotcue_" + hotCueNum + "_enabled") {
            padNum = (hotCueNum - 1);
            PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.hotCue, padNum, false, value);
            PioneerDDJSX.padLedControl(group, PioneerDDJSX.ledGroups.hotCue, padNum, true, value);
        }
    }
};

PioneerDDJSX.VuMeterLeds = function(value, group, control) {
    // Remark: Only deck vu meters can be controlled! Master vu meter is handled by hardware!
    var midiBaseAdress = 0xB0,
        channel = 0x02,
        midiOut = 0x00;

    value = parseInt(value * 0x76); //full level indicator: 0x7F

    if (engine.getValue(group, "PeakIndicator")) {
        value = value + 0x09;
    }

    PioneerDDJSX.valueVuMeter[group + "_current"] = value;

    for (var index in PioneerDDJSX.channelGroups) {
        if (PioneerDDJSX.channelGroups.hasOwnProperty(index)) {
            midiOut = PioneerDDJSX.valueVuMeter[index + "_current"];
            if (PioneerDDJSX.twinkleVumeterAutodjOn) {
                if (engine.getValue("[AutoDJ]", "enabled")) {
                    if (PioneerDDJSX.valueVuMeter[index + "_enabled"]) {
                        midiOut = 0;
                    }
                    if (midiOut < 5 && !PioneerDDJSX.valueVuMeter[index + "_enabled"]) {
                        midiOut = 5;
                    }
                }
            }
            midi.sendShortMsg(
                midiBaseAdress + PioneerDDJSX.channelGroups[index],
                channel,
                midiOut
            );
        }
    }
};


///////////////////////////////////////////////////////////////
//                          JOGWHEELS                        //
///////////////////////////////////////////////////////////////

PioneerDDJSX.getJogWheelDelta = function(value) {
    // The Wheel control centers on 0x40; find out how much it's moved by.
    return value - 0x40;
};

PioneerDDJSX.jogRingTick = function(channel, control, value, status, group) {
    PioneerDDJSX.pitchBendFromJog(group, PioneerDDJSX.getJogWheelDelta(value));
};

PioneerDDJSX.jogRingTickShift = function(channel, control, value, status, group) {
    PioneerDDJSX.pitchBendFromJog(
        group,
        PioneerDDJSX.getJogWheelDelta(value) * PioneerDDJSX.jogwheelShiftMultiplier
    );
};

PioneerDDJSX.jogPlatterTick = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];

    if (PioneerDDJSX.gridAdjustSelected[deck]) {
        if (PioneerDDJSX.getJogWheelDelta(value) > 0) {
            script.toggleControl(group, "beats_adjust_faster");
        }
        if (PioneerDDJSX.getJogWheelDelta(value) <= 0) {
            script.toggleControl(group, "beats_adjust_slower");
        }
        return;
    }
    if (PioneerDDJSX.gridSlideSelected[deck]) {
        if (PioneerDDJSX.getJogWheelDelta(value) > 0) {
            script.toggleControl(group, "beats_translate_later");
        }
        if (PioneerDDJSX.getJogWheelDelta(value) <= 0) {
            script.toggleControl(group, "beats_translate_earlier");
        }
        return;
    }

    if (PioneerDDJSX.scratchMode[deck] && engine.isScratching(deck + 1)) {
        engine.scratchTick(deck + 1, PioneerDDJSX.getJogWheelDelta(value));
    } else {
        PioneerDDJSX.pitchBendFromJog(group, PioneerDDJSX.getJogWheelDelta(value));
    }
};

PioneerDDJSX.jogPlatterTickShift = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];

    if (PioneerDDJSX.scratchMode[deck] && engine.isScratching(deck + 1)) {
        engine.scratchTick(deck + 1, PioneerDDJSX.getJogWheelDelta(value));
    } else {
        PioneerDDJSX.pitchBendFromJog(
            group,
            PioneerDDJSX.getJogWheelDelta(value) * PioneerDDJSX.jogwheelShiftMultiplier
        );
    }
};

PioneerDDJSX.jogTouch = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];

    if (PioneerDDJSX.scratchMode[deck]) {
        if (value) {
            engine.scratchEnable(
                deck + 1,
                PioneerDDJSX.scratchSettings.jogResolution,
                PioneerDDJSX.scratchSettings.vinylSpeed,
                PioneerDDJSX.scratchSettings.alpha,
                PioneerDDJSX.scratchSettings.beta,
                true
            );
        } else {
            engine.scratchDisable(deck + 1, true);
        }
    }
};

PioneerDDJSX.toggleScratch = function(channel, control, value, status, group) {
    var deck = PioneerDDJSX.channelGroups[group];
    if (value) {
        PioneerDDJSX.scratchMode[deck] = !PioneerDDJSX.scratchMode[deck];
        PioneerDDJSX.triggerVinylLed(deck);
    }
};

PioneerDDJSX.triggerVinylLed = function(deck) {
    PioneerDDJSX.nonPadLedControl(deck, PioneerDDJSX.nonPadLeds.vinyl, PioneerDDJSX.scratchMode[deck]);
};

PioneerDDJSX.pitchBendFromJog = function(group, movement) {
    engine.setValue(group, "jog", movement / 5 * PioneerDDJSX.jogwheelSensitivity);
};


///////////////////////////////////////////////////////////////
//             ROTARY SELECTOR & NAVIGATION BUTTONS          //
///////////////////////////////////////////////////////////////

PioneerDDJSX.loadPrepareButton = function(channel, control, value, status) {
    if (PioneerDDJSX.rotarySelectorChanged === true) {
        if (value) {
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", true);
        } else {
            if (PioneerDDJSX.jumpPreviewEnabled) {
                engine.setValue("[PreviewDeck1]", "playposition", PioneerDDJSX.jumpPreviewPosition);
            }
            PioneerDDJSX.rotarySelectorChanged = false;
        }
    } else {
        if (value) {
            if (engine.getValue("[PreviewDeck1]", "stop")) {
                script.toggleControl("[PreviewDeck1]", "play");
            } else {
                script.toggleControl("[PreviewDeck1]", "stop");
            }
        }
    }
};

PioneerDDJSX.backButton = function(channel, control, value, status) {
    script.toggleControl("[Library]", "MoveFocusBackward");
};

PioneerDDJSX.shiftBackButton = function(channel, control, value, status) {
    if (value) {
        script.toggleControl("[Master]", "maximize_library");
    }
};

PioneerDDJSX.getRotaryDelta = function(value) {
    var delta = 0x40 - Math.abs(0x40 - value),
        isCounterClockwise = value > 0x40;

    if (isCounterClockwise) {
        delta *= -1;
    }
    return delta;
};

PioneerDDJSX.rotarySelector = function(channel, control, value, status) {
    var delta = PioneerDDJSX.getRotaryDelta(value);

    engine.setValue("[Library]", "MoveVertical", delta);
    PioneerDDJSX.rotarySelectorChanged = true;
};

PioneerDDJSX.rotarySelectorShifted = function(channel, control, value, status) {
    var delta = PioneerDDJSX.getRotaryDelta(value),
        f = (delta > 0 ? "SelectNextPlaylist" : "SelectPrevPlaylist");

    engine.setValue("[Library]", "MoveHorizontal", delta);
};

PioneerDDJSX.rotarySelectorClick = function(channel, control, value, status) {
    script.toggleControl("[Library]", "GoToItem");
};

PioneerDDJSX.rotarySelectorShiftedClick = function(channel, control, value, status) {
    if (PioneerDDJSX.autoDJAddTop) {
        script.toggleControl("[Library]", "AutoDjAddTop");
    } else {
        script.toggleControl("[Library]", "AutoDjAddBottom");
    }
};


///////////////////////////////////////////////////////////////
//                             FX                            //
///////////////////////////////////////////////////////////////

PioneerDDJSX.fxAssignButton = function(channel, control, value, status, group) {
    if (value) {
        if ((control >= 0x4C) && (control <= 0x4F)) {
            script.toggleControl("[EffectRack1_EffectUnit1]", "group_" + group + "_enable");
        } else if ((control >= 0x50) && (control <= 0x53)) {
            script.toggleControl("[EffectRack1_EffectUnit2]", "group_" + group + "_enable");
        } else if ((control >= 0x70) && (control <= 0x73) && PioneerDDJSX.shiftPanelSelectPressed) {
            script.toggleControl("[EffectRack1_EffectUnit3]", "group_" + group + "_enable");
        } else if ((control >= 0x54) && (control <= 0x57) && PioneerDDJSX.shiftPanelSelectPressed) {
            script.toggleControl("[EffectRack1_EffectUnit4]", "group_" + group + "_enable");
        }
    }
};


///////////////////////////////////////////////////////////////
//                          SLICER                           //
///////////////////////////////////////////////////////////////

PioneerDDJSX.slicerBeatActive = function(value, group, control) {
    // This slicer implementation will work for constant beatgrids only!
    var deck = PioneerDDJSX.channelGroups[group],
        bpm = engine.getValue(group, "bpm"),
        playposition = engine.getValue(group, "playposition"),
        duration = engine.getValue(group, "duration"),
        slicerPosInSection = 0,
        ledBeatState = true,
        domain = PioneerDDJSX.selectedSlicerDomain[deck];

    if (engine.getValue(group, "beat_closest") === engine.getValue(group, "beat_next")) {
        return;
    }

    PioneerDDJSX.slicerBeatsPassed[deck] = Math.round((playposition * duration) * (bpm / 60));
    slicerPosInSection = Math.floor((PioneerDDJSX.slicerBeatsPassed[deck] % domain) / (domain / 8));

    if (PioneerDDJSX.activePadMode[deck] === PioneerDDJSX.padModes.slicer) {
        if (PioneerDDJSX.activeSlicerMode[deck] === PioneerDDJSX.slicerModes.contSlice) {
            ledBeatState = true;
        }
        if (PioneerDDJSX.activeSlicerMode[deck] === PioneerDDJSX.slicerModes.loopSlice) {
            ledBeatState = false;
            if (((PioneerDDJSX.slicerBeatsPassed[deck] - 1) % domain) === (domain - 1) &&
                !PioneerDDJSX.slicerAlreadyJumped[deck] &&
                PioneerDDJSX.slicerPreviousBeatsPassed[deck] < PioneerDDJSX.slicerBeatsPassed[deck]) {
                engine.setValue(group, "beatjump", -domain);
                PioneerDDJSX.slicerAlreadyJumped[deck] = true;
            } else {
                PioneerDDJSX.slicerAlreadyJumped[deck] = false;
            }
        }
        // PAD Led control:
        for (var i = 0; i < 8; i++) {
            if (PioneerDDJSX.slicerActive[deck]) {
                if (PioneerDDJSX.slicerButton[deck] !== i) {
                    PioneerDDJSX.padLedControl(
                        group,
                        PioneerDDJSX.ledGroups.slicer,
                        i,
                        false,
                        (slicerPosInSection === i) ? ledBeatState : !ledBeatState
                    );
                }
            } else {
                PioneerDDJSX.padLedControl(
                    group,
                    PioneerDDJSX.ledGroups.slicer,
                    i,
                    false,
                    (slicerPosInSection === i) ? ledBeatState : !ledBeatState
                );
            }
        }
    } else {
        PioneerDDJSX.slicerAlreadyJumped[deck] = false;
        PioneerDDJSX.slicerPreviousBeatsPassed[deck] = 0;
        PioneerDDJSX.slicerActive[deck] = false;
    }
};
var PioneerDDJSB2 = function() {};

/*
    Find the latest code at https://github.com/dg3nec/mixxx
   
    
    This mapping for the Pioneer DDJ-SB2 was made by DG3NEC, Michael Stahl
    Basing on DDj-SB for Mixxx 2.0 Joan Ardiaca Jové (joan.ardiaca@gmail.com),
    basing on the work of wingcom (wwingcomm@gmail.com, https://github.com/wingcom/Mixxx-Pioneer-DDJ-SB).
    which in turn was based on the work of Hilton Rudham (https://github.com/hrudham/Mixxx-Pioneer-DDJ-SR).
    Just as wingcom's and Rudham's work, this mapping is pusblished under the MIT license.
  
    TODO:
    - eliminate code for "virtual 4 deck mode DDJ-SB"
    - Softtakeover: Rate, Crossover
 
*/


///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// If true the sync button blinks with the beat, if false led is lit when sync is enabled.
PioneerDDJSB2.blinkingSync = false;

// If true, the vinyl button activates slip. Vinyl mode is then activated by using shift.
// Allows toggling slip faster, but is counterintuitive.
PioneerDDJSB2.invertVinylSlipButton = false;

// Sets the jogwheels sensivity. 1 is default, 2 is twice as sensitive, 0.5 is half as sensitive.
PioneerDDJSB2.jogwheelSensivity = 1.0;

// Sets how much more sensitive the jogwheels get when holding shift.
// Set to 1 to disable jogwheel sensitivity increase when holding shift.
PioneerDDJSB2.jogwheelShiftMultiplier = 20;

// Time per step (in ms) for pitch speed fade to normal
PioneerDDJSB2.speedRateToNormalTime = 200;

// If true Level-Meter shows VU-Master left & right. If false shows level of channel: 1/3  2/4 (depending active deck)
PioneerDDJSB2.showVumeterMaster = false;

// Cut's Level-Meter low and expand upper. Examples:
// 0.25 -> only signals greater 25%, expanded to full range
// 0.5 -> only signals greater 50%, expanded to full range
PioneerDDJSB2.cutVumeter = 0.25;

// If true VU-Level twinkle if AutoDJ is ON.
PioneerDDJSB2.twinkleVumeterAutodjOn = true;

// If true, by release browser knob jump forward to "position". 
PioneerDDJSB2.jumpPreviewEnabled = true;
PioneerDDJSB2.jumpPreviewPosition = 0.5;


///////////////////////////////////////////////////////////////
//               INIT, SHUTDOWN & GLOBAL HELPER              //
///////////////////////////////////////////////////////////////
PioneerDDJSB2.longButtonPress = false;
PioneerDDJSB2.speedRateToNormalTimer = new Array(4);

PioneerDDJSB2.init = function(id) {
    PioneerDDJSB2.scratchSettings = {
        'alpha': 1.0 / 8,
        'beta': 1.0 / 8 / 32,
        'jogResolution': 720,
        'vinylSpeed': 33 + 1 / 3,
        'safeScratchTimeout': 20
    };

    PioneerDDJSB2.channelGroups = {
        '[Channel1]': 0x00,
        '[Channel2]': 0x01,
        '[Channel3]': 0x02,
        '[Channel4]': 0x03
    };

    PioneerDDJSB2.samplerGroups = {
        '[Sampler1]': 0x00,
        '[Sampler2]': 0x01,
        '[Sampler3]': 0x02,
        '[Sampler4]': 0x03
    };

    PioneerDDJSB2.fxGroups = {
        '[EffectRack1_EffectUnit1]': 0x00,
        '[EffectRack1_EffectUnit2]': 0x01
    };

    PioneerDDJSB2.fxControls = {
        'group_[Channel1]_enable': 0x00,
        'group_[Channel3]_enable': 0x00,
        'group_[Headphone]_enable': 0x01,
        'group_[Master]_enable': 0x01,
        'group_[Channel2]_enable': 0x02,
        'group_[Channel4]_enable': 0x02
    };

    PioneerDDJSB2.shiftPressed = false;

    PioneerDDJSB2.chFaderStart = [
        null,
        null
    ];

    PioneerDDJSB2.fxButtonPressed = [
        [false, false, false],
        [false, false, false]
    ];

    // used for soft takeover workaround
    PioneerDDJSB2.fxParamsActiveValues = [
        [0, 0, 0, 0, 0],
        [0, 0, 0, 0, 0]
    ];

    PioneerDDJSB2.scratchMode = [false, false, false, false];

    PioneerDDJSB2.ledGroups = {
        'hotCue': 0x00,
        'autoLoop': 0x10,
        'manualLoop': 0x20,
        'sampler': 0x30
    };

    PioneerDDJSB2.nonPadLeds = {
        'headphoneCue': 0x54,
        'shiftHeadphoneCue': 0x68,
        'cue': 0x0C,
        'shiftCue': 0x48,
        'keyLock': 0x1A,
        'shiftKeyLock': 0x60,
        'play': 0x0B,
        'shiftPlay': 0x47,
        'vinyl': 0x17,
        'shiftVinyl': 0x4E,
        'sync': 0x58,
        'shiftSync': 0x5C
    };

    PioneerDDJSB2.valueVuMeter = {
        '[Channel1]_current': 0,
        '[Channel2]_current': 0,
        '[Channel3]_current': 0,
        '[Channel4]_current': 0,
        '[Channel1]_enabled': 1,
        '[Channel2]_enabled': 1,
        '[Channel3]_enabled': 1,
        '[Channel4]_enabled': 1,

    };

    PioneerDDJSB2.loopIntervals = [1, 2, 4, 8, 16, 32, 64];

    PioneerDDJSB2.looprollIntervals = [1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4, 8];

    PioneerDDJSB2.setAllSoftTakeover(false);
    PioneerDDJSB2.bindNonDeckControlConnections(false);
    PioneerDDJSB2.initDeck('[Channel1]');
    PioneerDDJSB2.initDeck('[Channel2]');
    PioneerDDJSB2.initDeck('[Channel3]');
    PioneerDDJSB2.initDeck('[Channel4]');

    if (PioneerDDJSB2.twinkleVumeterAutodjOn) {
        PioneerDDJSB2.vu_meter_timer = engine.beginTimer(100, "PioneerDDJSB2.vuMeterTwinkle()");
    }
};

PioneerDDJSB2.shutdown = function() {
    PioneerDDJSB2.bindAllControlConnections(true);
    PioneerDDJSB2.setAllSoftTakeover(true);
    PioneerDDJSB2.bindDeckControlConnections('[Channel3]', true);
    PioneerDDJSB2.bindDeckControlConnections('[Channel4]', true);
};

PioneerDDJSB2.longButtonPressWait = function() {
    engine.stopTimer(PioneerDDJSB2.longButtonPressTimer);
    PioneerDDJSB2.longButtonPress = true;
};

PioneerDDJSB2.speedRateToNormal = function(group, deck) {
    var speed = engine.getValue(group, 'rate');
    if (speed > 0) {
        engine.setValue(group, 'rate_perm_down_small', true);
        if (engine.getValue(group, 'rate') <= 0) {
            engine.stopTimer(PioneerDDJSB2.speedRateToNormalTimer[deck]);
            engine.setValue(group, 'rate', 0);
        }
    } else if (speed < 0) {
        engine.setValue(group, 'rate_perm_up_small', true);
        if (engine.getValue(group, 'rate') >= 0) {
            engine.stopTimer(PioneerDDJSB2.speedRateToNormalTimer[deck]);
            engine.setValue(group, 'rate', 0);
        }
    }
};


///////////////////////////////////////////////////////////////
//                      VU - Meter                           //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.blinkAutodjState = 0; // new for DDJ-SB2

PioneerDDJSB2.vuMeterTwinkle = function() {
    if (engine.getValue("[AutoDJ]", "enabled")) {
        PioneerDDJSB2.blinkAutodjState = PioneerDDJSB2.blinkAutodjState + 1;
        if (PioneerDDJSB2.blinkAutodjState > 3) {
            PioneerDDJSB2.blinkAutodjState = 0;
        }
        if (PioneerDDJSB2.blinkAutodjState === 0) {
            PioneerDDJSB2.valueVuMeter['[Channel1]_enabled'] = 0;
            PioneerDDJSB2.valueVuMeter['[Channel3]_enabled'] = 0;
            PioneerDDJSB2.valueVuMeter['[Channel2]_enabled'] = 0;
            PioneerDDJSB2.valueVuMeter['[Channel4]_enabled'] = 0;
        }
        if (PioneerDDJSB2.blinkAutodjState === 1) {
            PioneerDDJSB2.valueVuMeter['[Channel1]_enabled'] = 1;
            PioneerDDJSB2.valueVuMeter['[Channel3]_enabled'] = 1;
            PioneerDDJSB2.valueVuMeter['[Channel2]_enabled'] = 0;
            PioneerDDJSB2.valueVuMeter['[Channel4]_enabled'] = 0;
        }
        if (PioneerDDJSB2.blinkAutodjState === 2) {
            PioneerDDJSB2.valueVuMeter['[Channel1]_enabled'] = 1;
            PioneerDDJSB2.valueVuMeter['[Channel3]_enabled'] = 1;
            PioneerDDJSB2.valueVuMeter['[Channel2]_enabled'] = 1;
            PioneerDDJSB2.valueVuMeter['[Channel4]_enabled'] = 1;
        }
        if (PioneerDDJSB2.blinkAutodjState === 3) {
            PioneerDDJSB2.valueVuMeter['[Channel1]_enabled'] = 0;
            PioneerDDJSB2.valueVuMeter['[Channel3]_enabled'] = 0;
            PioneerDDJSB2.valueVuMeter['[Channel2]_enabled'] = 1;
            PioneerDDJSB2.valueVuMeter['[Channel4]_enabled'] = 1;
        }
    } else {
        PioneerDDJSB2.valueVuMeter['[Channel1]_enabled'] = 1;
        PioneerDDJSB2.valueVuMeter['[Channel3]_enabled'] = 1;
        PioneerDDJSB2.valueVuMeter['[Channel2]_enabled'] = 1;
        PioneerDDJSB2.valueVuMeter['[Channel4]_enabled'] = 1;
    }
};


///////////////////////////////////////////////////////////////
//                        AutoDJ                             //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.autodjSkipNext = function(channel, control, value, status, group) {
    if (value === 0) {
        return;
    }
    if (engine.getValue("[AutoDJ]", "enabled")) {
        engine.setValue("[AutoDJ]", "skip_next", true);
    }
};

PioneerDDJSB2.autodjToggle = function(channel, control, value, status, group) {
    if (value === 0) {
        return;
    }
    if (engine.getValue("[AutoDJ]", "enabled")) {
        engine.setValue("[AutoDJ]", "enabled", false);
    } else {
        engine.setValue("[AutoDJ]", "enabled", true);
    }
};


///////////////////////////////////////////////////////////////
//                      CONTROL BINDING                      //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.bindSamplerControlConnections = function(samplerGroup, isUnbinding) {
    engine.connectControl(samplerGroup, 'duration', 'PioneerDDJSB2.samplerLeds', isUnbinding);
};

PioneerDDJSB2.bindDeckControlConnections = function(channelGroup, isUnbinding) {
    var i,
        index,
        fxUnitIndex = 1,
        controlsToFunctions = {
            'play': 'PioneerDDJSB2.playLeds',
            'pfl': 'PioneerDDJSB2.headphoneCueLed',
            'keylock': 'PioneerDDJSB2.keyLockLed',
            'slip_enabled': 'PioneerDDJSB2.slipLed',
            'quantize': 'PioneerDDJSB2.quantizeLed',
            'loop_in': 'PioneerDDJSB2.loopInLed',
            'loop_out': 'PioneerDDJSB2.loopOutLed',
            'filterLowKill': 'PioneerDDJSB2.lowKillLed',
            'filterMidKill': 'PioneerDDJSB2.midKillLed',
            'filterHighKill': 'PioneerDDJSB2.highKillLed',
            'mute': 'PioneerDDJSB2.muteLed',
            'loop_enabled': 'PioneerDDJSB2.loopExitLed',
            'loop_double': 'PioneerDDJSB2.loopDoubleLed',
            'loop_halve': 'PioneerDDJSB2.loopHalveLed'
        };

    if (PioneerDDJSB2.blinkingSync) {
        controlsToFunctions.beat_active = 'PioneerDDJSB2.syncLed';
    } else {
        controlsToFunctions.sync_enabled = 'PioneerDDJSB2.syncLed';
    }

    for (i = 1; i <= 8; i++) {
        controlsToFunctions['hotcue_' + i + '_enabled'] = 'PioneerDDJSB2.hotCueLeds';
    }

    for (index in PioneerDDJSB2.loopIntervals) {
        controlsToFunctions['beatloop_' + PioneerDDJSB2.loopIntervals[index] + '_enabled'] = 'PioneerDDJSB2.beatloopLeds';
    }

    for (index in PioneerDDJSB2.looprollIntervals) {
        controlsToFunctions['beatlooproll_' + PioneerDDJSB2.looprollIntervals[index] + '_activate'] = 'PioneerDDJSB2.beatlooprollLeds';
    }

    script.bindConnections(channelGroup, controlsToFunctions, isUnbinding);

    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        engine.connectControl('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_' + channelGroup + '_enable', 'PioneerDDJSB2.fxLeds', isUnbinding);
        if (!isUnbinding) {
            engine.trigger('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_' + channelGroup + '_enable');
        }
    }
};

PioneerDDJSB2.bindNonDeckControlConnections = function(isUnbinding) {
    var samplerIndex,
        fxUnitIndex;

    for (samplerIndex = 1; samplerIndex <= 4; samplerIndex++) {
        PioneerDDJSB2.bindSamplerControlConnections('[Sampler' + samplerIndex + ']', isUnbinding);
    }

    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        engine.connectControl('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_[Headphone]_enable', 'PioneerDDJSB2.fxLeds', isUnbinding);
    }
    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        engine.connectControl('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_[Master]_enable', 'PioneerDDJSB2.fxLeds', isUnbinding);
    }

    if (PioneerDDJSB2.showVumeterMaster) {
        engine.connectControl('[Master]', 'VuMeterL', 'PioneerDDJSB2.VuMeterLeds', isUnbinding);
        engine.connectControl('[Master]', 'VuMeterR', 'PioneerDDJSB2.VuMeterLeds', isUnbinding);
    } else {
        engine.connectControl('[Channel1]', 'VuMeter', 'PioneerDDJSB2.VuMeterLeds', isUnbinding);
        engine.connectControl('[Channel2]', 'VuMeter', 'PioneerDDJSB2.VuMeterLeds', isUnbinding);
        engine.connectControl('[Channel3]', 'VuMeter', 'PioneerDDJSB2.VuMeterLeds', isUnbinding);
        engine.connectControl('[Channel4]', 'VuMeter', 'PioneerDDJSB2.VuMeterLeds', isUnbinding);
    }
};

PioneerDDJSB2.bindAllControlConnections = function(isUnbinding) {
    var samplerIndex,
        fxUnitIndex,
        channelIndex;

    for (samplerIndex = 1; samplerIndex <= 4; samplerIndex++) {
        PioneerDDJSB2.bindSamplerControlConnections('[Sampler' + samplerIndex + ']', isUnbinding);
    }

    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        engine.connectControl('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_[Headphone]_enable', 'PioneerDDJSB2.fxLeds', isUnbinding);
    }

    for (channelIndex = 1; channelIndex <= 2; channelIndex++) {
        PioneerDDJSB2.bindDeckControlConnections('[Channel' + channelIndex + ']', isUnbinding);
    }
};

PioneerDDJSB2.setDeckSoftTakeover = function(channel, isUnbinding) {
    engine.softTakeover(channel, "volume", !isUnbinding);
    engine.softTakeover(channel, "rate", !isUnbinding);
    engine.softTakeover(channel, "pregain", !isUnbinding);
    engine.softTakeover(channel, "filterHigh", !isUnbinding);
    engine.softTakeover(channel, "filterMid", !isUnbinding);
    engine.softTakeover(channel, "filterLow", !isUnbinding);
    engine.softTakeover("[QuickEffectRack1_" + channel + "]", "super1", !isUnbinding);
};

PioneerDDJSB2.setAllSoftTakeover = function(isUnbinding) {
    var channelIndex;
    for (channelIndex = 1; channelIndex <= 4; channelIndex++) {
        PioneerDDJSB2.setDeckSoftTakeover('[Channel' + channelIndex + ']', isUnbinding);
    }
};


///////////////////////////////////////////////////////////////
//                       DECK SWITCHING                      //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.deckSwitchTable = {
    '[Channel1]': '[Channel1]',
    '[Channel2]': '[Channel2]',
    '[Channel3]': '[Channel3]',
    '[Channel4]': '[Channel4]'

};

PioneerDDJSB2.deckShiftSwitchTable = {
    '[Channel1]': '[Channel3]',
    '[Channel2]': '[Channel4]',
    '[Channel3]': '[Channel1]',
    '[Channel4]': '[Channel2]'
};

PioneerDDJSB2.initDeck = function(group) {
    PioneerDDJSB2.bindDeckControlConnections(group, false);
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.shiftKeyLock, PioneerDDJSB2.channelGroups[group] > 1);
    PioneerDDJSB2.triggerVinylLed(PioneerDDJSB2.channelGroups[group]);
};


///////////////////////////////////////////////////////////////
//            HIGH RESOLUTION MIDI INPUT HANDLERS            //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.highResMSB = {
    '[Channel1]': {},
    '[Channel2]': {},
    '[Channel3]': {},
    '[Channel4]': {}
};

PioneerDDJSB2.tempoSliderMSB = function(channel, control, value, status, group) {
    PioneerDDJSB2.highResMSB[group].tempoSlider = value;
};

PioneerDDJSB2.tempoSliderLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB2.highResMSB[group].tempoSlider << 7) + value;
    engine.setValue(
        PioneerDDJSB2.deckSwitchTable[group],
        'rate',
        ((0x4000 - fullValue) - 0x2000) / 0x2000
    );
};

PioneerDDJSB2.gainKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSB2.highResMSB[group].gainKnob = value;
};

PioneerDDJSB2.gainKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB2.highResMSB[group].gainKnob << 7) + value;
    engine.setValue(
        PioneerDDJSB2.deckSwitchTable[group],
        'pregain',
        script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF)
    );
};

PioneerDDJSB2.filterHighKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSB2.highResMSB[group].filterHigh = value;
};

PioneerDDJSB2.filterHighKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB2.highResMSB[group].filterHigh << 7) + value;
    engine.setValue(
        group,
        'filterHigh',
        script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF)
    );
};

PioneerDDJSB2.filterMidKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSB2.highResMSB[group].filterMid = value;
};

PioneerDDJSB2.filterMidKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB2.highResMSB[group].filterMid << 7) + value;
    engine.setValue(
        group,
        'filterMid',
        script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF));
};

PioneerDDJSB2.filterLowKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSB2.highResMSB[group].filterLow = value;
};

PioneerDDJSB2.filterLowKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB2.highResMSB[group].filterLow << 7) + value;
    engine.setValue(
        group,
        'filterLow',
        script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF)
    );
};

PioneerDDJSB2.deckFaderMSB = function(channel, control, value, status, group) {
    PioneerDDJSB2.highResMSB[group].deckFader = value;
};

PioneerDDJSB2.deckFaderLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB2.highResMSB[group].deckFader << 7) + value;

    if (PioneerDDJSB2.shiftPressed &&
        engine.getValue(group, 'volume') === 0 &&
        fullValue !== 0 &&
        engine.getValue(group, 'play') === 0
    ) {
        PioneerDDJSB2.chFaderStart[channel] = engine.getValue(group, 'playposition');
        engine.setValue(group, 'play', 1);
    } else if (
        PioneerDDJSB2.shiftPressed &&
        engine.getValue(group, 'volume') !== 0 &&
        fullValue === 0 &&
        engine.getValue(group, 'play') === 1 &&
        PioneerDDJSB2.chFaderStart[channel] !== null
    ) {
        engine.setValue(group, 'play', 0);
        engine.setValue(group, 'playposition', PioneerDDJSB2.chFaderStart[channel]);
        PioneerDDJSB2.chFaderStart[channel] = null;
    }
    engine.setValue(group, 'volume', fullValue / 0x3FFF);
};

PioneerDDJSB2.filterKnobMSB = function(channel, control, value, status, group) {
    PioneerDDJSB2.highResMSB[group].filterKnob = value;
};

PioneerDDJSB2.filterKnobLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB2.highResMSB[group].filterKnob << 7) + value;
    engine.setValue('[QuickEffectRack1_' + group + ']', 'super1', fullValue / 0x3FFF);
};


///////////////////////////////////////////////////////////////
//           SINGLE MESSAGE MIDI INPUT HANDLERS              //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.shiftButton = function(channel, control, value, status, group) {
    var index = 0;
    PioneerDDJSB2.shiftPressed = (value == 0x7F);
    for (index in PioneerDDJSB2.chFaderStart) {
        PioneerDDJSB2.chFaderStart[index] = null;
    }
};

PioneerDDJSB2.playButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(PioneerDDJSB2.deckSwitchTable[group], 'play');
    }
};

PioneerDDJSB2.headphoneCueButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, 'pfl');
    }
};

PioneerDDJSB2.headphoneShiftCueButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(PioneerDDJSB2.deckShiftSwitchTable[group], 'pfl');
    }
};

PioneerDDJSB2.hotCueButtons = function(channel, control, value, status, group) {
    var hotCueIndex = (control >= 0x40 ? control - 0x40 + 5 : control + 1);
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'hotcue_' + hotCueIndex + '_activate', value);
};

PioneerDDJSB2.clearHotCueButtons = function(channel, control, value, status, group) {
    var hotCueIndex = (control >= 0x48 ? control - 0x48 + 5 : control - 7);
    if (value) {
        engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'hotcue_' + hotCueIndex + '_clear', 1);
    }
};

PioneerDDJSB2.cueButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'cue_default', value);
};

PioneerDDJSB2.beatloopButtons = function(channel, control, value, status, group) {
    var index = (control <= 0x13 ? control - 0x10 : control - 0x14);
    if (value) {
        engine.setValue(
            group,
            'beatloop_' + PioneerDDJSB2.loopIntervals[index] + '_toggle',
            1
        );
    }
};

PioneerDDJSB2.beatloopRollButtons = function(channel, control, value, status, group) {
    var index = (control <= 0x53 ? control - 0x50 : control - 0x54);
    engine.setValue(
        PioneerDDJSB2.deckSwitchTable[group],
        'beatlooproll_' + PioneerDDJSB2.looprollIntervals[index] + '_activate',
        value
    );
};

PioneerDDJSB2.vinylButton = function(channel, control, value, status, group) {
    if (PioneerDDJSB2.invertVinylSlipButton) {
        PioneerDDJSB2.toggleSlip(channel, control, value, status, group);
    } else {
        PioneerDDJSB2.toggleScratch(channel, control, value, status, group);
    }
};

PioneerDDJSB2.slipButton = function(channel, control, value, status, group) {
    if (PioneerDDJSB2.invertVinylSlipButton) {
        PioneerDDJSB2.toggleScratch(channel, control, value, status, group);
    } else {
        PioneerDDJSB2.toggleSlip(channel, control, value, status, group);
    }
};

PioneerDDJSB2.toggleSlip = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, 'slip_enabled');
    }
};

PioneerDDJSB2.keyLockButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, 'keylock');
    }
};

PioneerDDJSB2.shiftKeyLockButton = function(channel, control, value, status, group) {
    var deck = status - 0x90;
    if (value) {
        PioneerDDJSB2.speedRateToNormalTimer[deck] = engine.beginTimer(PioneerDDJSB2.speedRateToNormalTime, "PioneerDDJSB2.speedRateToNormal('" + group + "', " + deck + ")");
    }
};

PioneerDDJSB2.loopInButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'loop_in', value ? 1 : 0);
};

PioneerDDJSB2.loopOutButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'loop_out', value ? 1 : 0);
};

PioneerDDJSB2.loopExitButton = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'reloop_exit', 1);
    }
};

PioneerDDJSB2.loopHalveButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'loop_halve', value ? 1 : 0);
};

PioneerDDJSB2.loopDoubleButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'loop_double', value ? 1 : 0);
};

PioneerDDJSB2.loopMoveBackButton = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'loop_move', -1);
    }
};

PioneerDDJSB2.loopMoveForwardButton = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'loop_move', 1);
    }
};

PioneerDDJSB2.loadButton = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(group, 'LoadSelectedTrack', 1);
    }
};

PioneerDDJSB2.reverseRollButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'reverseroll', value);
};

PioneerDDJSB2.brakeButton = function(channel, control, value, status, group) {
    script.brake(channel, control, value, status, group);
};

PioneerDDJSB2.syncButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, 'sync_enabled');
    }
};

PioneerDDJSB2.quantizeButton = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, 'quantize');
    }
};

PioneerDDJSB2.lowKillButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'filterLowKill', value ? 1 : 0);
};

PioneerDDJSB2.midKillButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'filterMidKill', value ? 1 : 0);
};

PioneerDDJSB2.highKillButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'filterHighKill', value ? 1 : 0);
};

PioneerDDJSB2.muteButton = function(channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB2.deckSwitchTable[group], 'mute', value);
};


///////////////////////////////////////////////////////////////
//                          LED HELPERS                      //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.deckConverter = function(group) {
    var index;

    if (typeof group === "string") {
        for (index in PioneerDDJSB2.deckSwitchTable) {
            if (group === PioneerDDJSB2.deckSwitchTable[index]) {
                return PioneerDDJSB2.channelGroups[group];
            }
        }
        return null;
    }
    return group;
};

PioneerDDJSB2.fxLedControl = function(deck, ledNumber, shift, active) {
    var fxLedsBaseChannel = 0x94,
        fxLedsBaseControl = (shift ? 0x63 : 0x47),
        midiChannelOffset = PioneerDDJSB2.deckConverter(deck);

    if (midiChannelOffset !== null) {
        midi.sendShortMsg(
            fxLedsBaseChannel + midiChannelOffset,
            fxLedsBaseControl + ledNumber,
            active ? 0x7F : 0x00
        );
    }
};

PioneerDDJSB2.padLedControl = function(deck, groupNumber, shiftGroup, ledNumber, shift, active) {
    var padLedsBaseChannel = 0x97,
        padLedControl = (shiftGroup ? 0x40 : 0x00) + (shift ? 0x08 : 0x00) + groupNumber + ledNumber,
        midiChannelOffset = PioneerDDJSB2.deckConverter(deck);

    if (midiChannelOffset !== null) {
        midi.sendShortMsg(
            padLedsBaseChannel + midiChannelOffset,
            padLedControl,
            active ? 0x7F : 0x00
        );
    }
};

PioneerDDJSB2.nonPadLedControl = function(deck, ledNumber, active) {
    var nonPadLedsBaseChannel = 0x90,
        midiChannelOffset = PioneerDDJSB2.deckConverter(deck);

    if (midiChannelOffset !== null) {
        midi.sendShortMsg(
            nonPadLedsBaseChannel + midiChannelOffset,
            ledNumber,
            active ? 0x7F : 0x00
        );
    }
};


///////////////////////////////////////////////////////////////
//                             LEDS                          //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.fxLeds = function(value, group, control) {
    var deck = PioneerDDJSB2.fxGroups[group],
        ledNumber = PioneerDDJSB2.fxControls[control];

    if (PioneerDDJSB2.shiftPressed === false) {
        PioneerDDJSB2.fxLedControl(deck, ledNumber, false, value);
    } else {
        PioneerDDJSB2.fxLedControl(deck, ledNumber, true, value);
    }
};

PioneerDDJSB2.headphoneCueLed = function(value, group, control) {
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.headphoneCue, value);
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.shiftHeadphoneCue, value);
};

PioneerDDJSB2.keyLockLed = function(value, group, control) {
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.keyLock, value);
};

PioneerDDJSB2.playLeds = function(value, group, control) {
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.play, value);
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.shiftPlay, value);
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.cue, value);
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.shiftCue, value);
};

PioneerDDJSB2.slipLed = function(value, group, control) {
    var led = (PioneerDDJSB2.invertVinylSlipButton ? PioneerDDJSB2.nonPadLeds.vinyl : PioneerDDJSB2.nonPadLeds.shiftVinyl);

    PioneerDDJSB2.nonPadLedControl(group, led, value);
};

PioneerDDJSB2.quantizeLed = function(value, group, control) {
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.shiftSync, value);
};

PioneerDDJSB2.syncLed = function(value, group, control) {
    PioneerDDJSB2.nonPadLedControl(group, PioneerDDJSB2.nonPadLeds.sync, value);
};

PioneerDDJSB2.loopInLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, false, 0, false, value);
};

PioneerDDJSB2.loopOutLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, false, 1, false, value);
};

PioneerDDJSB2.loopExitLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, false, 2, false, value);
};

PioneerDDJSB2.loopHalveLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, false, 3, false, value);
};

PioneerDDJSB2.loopDoubleLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, false, 3, true, value);
};

PioneerDDJSB2.lowKillLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, true, 0, false, value);
};

PioneerDDJSB2.midKillLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, true, 1, false, value);
};

PioneerDDJSB2.highKillLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, true, 2, false, value);
};

PioneerDDJSB2.muteLed = function(value, group, control) {
    PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.manualLoop, true, 3, false, value);
};

PioneerDDJSB2.samplerLeds = function(value, group, control) {
    var sampler = PioneerDDJSB2.samplerGroups[group],
        channel;

    for (channel = 0; channel < 4; channel++) {
        PioneerDDJSB2.padLedControl(channel, PioneerDDJSB2.ledGroups.sampler, false, sampler, false, value);
        PioneerDDJSB2.padLedControl(channel, PioneerDDJSB2.ledGroups.sampler, false, sampler, true, value);
        PioneerDDJSB2.padLedControl(channel, PioneerDDJSB2.ledGroups.sampler, true, sampler, false, value);
        PioneerDDJSB2.padLedControl(channel, PioneerDDJSB2.ledGroups.sampler, true, sampler, true, value);
    }
};

PioneerDDJSB2.beatloopLeds = function(value, group, control) {
    var index,
        padNum,
        shifted;

    for (index in PioneerDDJSB2.loopIntervals) {
        if (control === 'beatloop_' + PioneerDDJSB2.loopIntervals[index] + '_enabled') {
            padNum = index % 4;
            shifted = (index >= 4);
            PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.autoLoop, false, padNum, shifted, value);
        }
    }
};

PioneerDDJSB2.beatlooprollLeds = function(value, group, control) {
    var index,
        padNum,
        shifted;

    for (index in PioneerDDJSB2.looprollIntervals) {
        if (control === 'beatlooproll_' + PioneerDDJSB2.looprollIntervals[index] + '_activate') {
            padNum = index % 4;
            shifted = (index >= 4);
            PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.autoLoop, true, padNum, shifted, value);
        }
    }
};

PioneerDDJSB2.hotCueLeds = function(value, group, control) {
    var shiftedGroup = false,
        padNum = null,
        hotCueNum;

    for (hotCueNum = 1; hotCueNum <= 8; hotCueNum++) {
        if (control === 'hotcue_' + hotCueNum + '_enabled') {
            padNum = (hotCueNum - 1) % 4;
            shiftedGroup = (hotCueNum > 4);
            PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.hotCue, shiftedGroup, padNum, false, value);
            PioneerDDJSB2.padLedControl(group, PioneerDDJSB2.ledGroups.hotCue, shiftedGroup, padNum, true, value);
        }
    }
};

PioneerDDJSB2.VuMeterLeds = function(value, group, control) {
    var midiBaseAdress = 0xB0,
        channel = 0,
        midiOut = 0;

    value = 1 / (1 - PioneerDDJSB2.cutVumeter) * (value - PioneerDDJSB2.cutVumeter);
    if (value < 0) {
        value = 0;
    }

    value = parseInt(value * 0x7F);
    if (value < 0) {
        value = 0;
    }
    if (value > 127) {
        value = 127;
    }

    if (group == "[Master]") {
        if (control == "VuMeterL") {
            PioneerDDJSB2.valueVuMeter['[Channel1]_current'] = value;
            PioneerDDJSB2.valueVuMeter['[Channel3]_current'] = value;
        } else {
            PioneerDDJSB2.valueVuMeter['[Channel2]_current'] = value;
            PioneerDDJSB2.valueVuMeter['[Channel4]_current'] = value;
        }
    } else {
        PioneerDDJSB2.valueVuMeter[group + '_current'] = value;
    }

    for (channel = 0; channel < 4; channel++) {
        midiOut = PioneerDDJSB2.valueVuMeter['[Channel' + (channel + 1) + ']_current'];
        if (PioneerDDJSB2.twinkleVumeterAutodjOn) {
            if (engine.getValue("[AutoDJ]", "enabled")) {
                if (PioneerDDJSB2.valueVuMeter['[Channel' + (channel + 1) + ']_enabled']) {
                    midiOut = 0;
                }
            }
        }
        if (PioneerDDJSB2.twinkleVumeterAutodjOn && engine.getValue("[AutoDJ]", "enabled") == 1) {
            if (midiOut < 5 && PioneerDDJSB2.valueVuMeter['[Channel' + (channel + 1) + ']_enabled'] === 0) {
                midiOut = 5;
            }
        }
        midi.sendShortMsg(
            midiBaseAdress + channel,
            2,
            midiOut
        );
    }
};


///////////////////////////////////////////////////////////////
//                          JOGWHEELS                        //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.getJogWheelDelta = function(value) { // O
    // The Wheel control centers on 0x40; find out how much it's moved by.
    return value - 0x40;
};

PioneerDDJSB2.jogRingTick = function(channel, control, value, status, group) {
    PioneerDDJSB2.pitchBendFromJog(group, PioneerDDJSB2.getJogWheelDelta(value));
};

PioneerDDJSB2.jogRingTickShift = function(channel, control, value, status, group) {
    PioneerDDJSB2.pitchBendFromJog(
        PioneerDDJSB2.deckSwitchTable[group],
        PioneerDDJSB2.getJogWheelDelta(value) * PioneerDDJSB2.jogwheelShiftMultiplier
    );
};

PioneerDDJSB2.jogPlatterTick = function(channel, control, value, status, group) {
    var deck = PioneerDDJSB2.channelGroups[PioneerDDJSB2.deckSwitchTable[group]];
    if (PioneerDDJSB2.scratchMode[deck]) {
        engine.scratchTick(deck + 1, PioneerDDJSB2.getJogWheelDelta(value));
    } else {
        PioneerDDJSB2.pitchBendFromJog(PioneerDDJSB2.deckSwitchTable[group], PioneerDDJSB2.getJogWheelDelta(value));
    }
};

PioneerDDJSB2.jogPlatterTickShift = function(channel, control, value, status, group) {
    var deck = PioneerDDJSB2.channelGroups[PioneerDDJSB2.deckSwitchTable[group]];
    if (PioneerDDJSB2.scratchMode[deck]) {
        engine.scratchTick(deck + 1, PioneerDDJSB2.getJogWheelDelta(value));
    } else {
        PioneerDDJSB2.pitchBendFromJog(
            PioneerDDJSB2.deckSwitchTable[group],
            PioneerDDJSB2.getJogWheelDelta(value) * PioneerDDJSB2.jogwheelShiftMultiplier
        );
    }
};

PioneerDDJSB2.jogTouch = function(channel, control, value, status, group) {
    var deck = PioneerDDJSB2.channelGroups[PioneerDDJSB2.deckSwitchTable[group]];

    if (PioneerDDJSB2.scratchMode[deck]) {
        if (value) {
            engine.scratchEnable(
                deck + 1,
                PioneerDDJSB2.scratchSettings.jogResolution,
                PioneerDDJSB2.scratchSettings.vinylSpeed,
                PioneerDDJSB2.scratchSettings.alpha,
                PioneerDDJSB2.scratchSettings.beta,
                true
            );
        } else {
            engine.scratchDisable(deck + 1, true);
        }
    }
};

PioneerDDJSB2.toggleScratch = function(channel, control, value, status, group) {
    var deck = PioneerDDJSB2.channelGroups[group];
    if (value) {
        PioneerDDJSB2.scratchMode[deck] = !PioneerDDJSB2.scratchMode[deck];
        PioneerDDJSB2.triggerVinylLed(deck);
        if (!PioneerDDJSB2.scratchMode[deck]) {
            engine.scratchDisable(deck + 1, true);
        }
    }
};

PioneerDDJSB2.triggerVinylLed = function(deck) {
    var led = (PioneerDDJSB2.invertVinylSlipButton ? PioneerDDJSB2.nonPadLeds.shiftVinyl : PioneerDDJSB2.nonPadLeds.vinyl);

    PioneerDDJSB2.nonPadLedControl(deck, led, PioneerDDJSB2.scratchMode[deck]);
};

PioneerDDJSB2.pitchBendFromJog = function(channel, movement) {
    var group = (typeof channel === "string" ? channel : '[Channel' + channel + 1 + ']');

    engine.setValue(group, 'jog', movement / 5 * PioneerDDJSB2.jogwheelSensivity);
};


///////////////////////////////////////////////////////////////
//                        ROTARY SELECTOR                    //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.rotarySelectorChanged = false; // new for DDJ-SB2

PioneerDDJSB2.getRotaryDelta = function(value) {
    var delta = 0x40 - Math.abs(0x40 - value),
        isCounterClockwise = value > 0x40;

    if (isCounterClockwise) {
        delta *= -1;
    }
    return delta;
};

PioneerDDJSB2.rotarySelector = function(channel, control, value, status) {
    var delta = PioneerDDJSB2.getRotaryDelta(value);
    engine.setValue('[Playlist]', 'SelectTrackKnob', delta);

    PioneerDDJSB2.rotarySelectorChanged = true;
};

PioneerDDJSB2.shiftedRotarySelector = function(channel, control, value, status) {
    var delta = PioneerDDJSB2.getRotaryDelta(value),
        f = (delta > 0 ? 'SelectNextPlaylist' : 'SelectPrevPlaylist');

    engine.setValue('[Playlist]', f, Math.abs(delta));
};

PioneerDDJSB2.rotarySelectorClick = function(channel, control, value, status) {
    if (PioneerDDJSB2.rotarySelectorChanged === true) {
        if (value) {
            engine.setValue('[PreviewDeck1]', 'LoadSelectedTrackAndPlay', true);
        } else {
            if (PioneerDDJSB2.jumpPreviewEnabled) {
                engine.setValue('[PreviewDeck1]', 'playposition', PioneerDDJSB2.jumpPreviewPosition);
            }
            PioneerDDJSB2.rotarySelectorChanged = false;
        }
    } else {
        if (value) {
            engine.setValue('[PreviewDeck1]', 'stop', 1);
        } else {
            PioneerDDJSB2.rotarySelectorChanged = true;
        }
    }
};

PioneerDDJSB2.rotarySelectorShiftedClick = function(channel, control, value, status) {
    if (value) {
        engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1);
    }
};


///////////////////////////////////////////////////////////////
//                             FX                            //
///////////////////////////////////////////////////////////////

PioneerDDJSB2.fxKnobMSB = [0, 0];
PioneerDDJSB2.fxKnobShiftedMSB = [0, 0];
PioneerDDJSB2.fxKnobParameterSet = false;

PioneerDDJSB2.fxButton = function(channel, control, value, status, group) {
    var deck = channel - 4,
        button = control - 0x47;

    PioneerDDJSB2.fxButtonPressed[deck][button] = (value === 0x7F);

    if (!value) {
        if (PioneerDDJSB2.fxKnobParameterSet) {
            PioneerDDJSB2.fxKnobParameterSet = false;
        } else {
            if (button === 0) {
                script.toggleControl(group, 'group_[Channel1]_enable');
            } else if (button === 1) {
                script.toggleControl(group, 'group_[Headphone]_enable');
            } else if (button === 2) {
                script.toggleControl(group, 'group_[Channel2]_enable');
            }
        }
    }
};

PioneerDDJSB2.fxButtonShifted = function(channel, control, value, status, group) {
    var button = control - 0x63;
    if (!value) {
        if (button === 0) {
            script.toggleControl(group, 'group_[Channel3]_enable');
        } else if (button === 1) {
            script.toggleControl(group, 'group_[Master]_enable');
        } else if (button === 2) {
            script.toggleControl(group, 'group_[Channel4]_enable');
        }
    }
};

PioneerDDJSB2.fxKnobShiftedMSB = function(channel, control, value, status) {
    PioneerDDJSB2.fxKnobShiftedMSB[channel - 4] = value;
};

PioneerDDJSB2.fxKnobShiftedLSB = function(channel, control, value, status) {
    var deck = channel - 4,
        fullValue = (PioneerDDJSB2.fxKnobShiftedMSB[deck] << 7) + value;

    if (PioneerDDJSB2.softTakeoverEmulation(deck, 4, PioneerDDJSB2.fxKnobShiftedMSB[deck])) {
        engine.setValue('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'super1', fullValue / 0x3FFF);
    }
};

PioneerDDJSB2.fxKnobMSB = function(channel, control, value, status) {
    PioneerDDJSB2.fxKnobMSB[channel - 4] = value;
};

PioneerDDJSB2.fxKnobLSB = function(channel, control, value, status) {
    var deck = channel - 4,
        anyButtonPressed = false,
        fullValue = (PioneerDDJSB2.fxKnobMSB[deck] << 7) + value,
        parameter;

    for (parameter = 0; parameter < 3; parameter++) {
        if (PioneerDDJSB2.fxButtonPressed[deck][parameter]) {
            anyButtonPressed = true;
        }
    }

    if (!anyButtonPressed) {
        if (PioneerDDJSB2.softTakeoverEmulation(deck, 3, PioneerDDJSB2.fxKnobMSB[deck])) {
            engine.setValue('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'mix', fullValue / 0x3FFF);
        }
    } else {
        for (parameter = 0; parameter < 3; parameter++) {
            if (PioneerDDJSB2.fxButtonPressed[deck][parameter] && PioneerDDJSB2.softTakeoverEmulation(deck, parameter, PioneerDDJSB2.fxKnobMSB[deck])) {
                engine.setParameter(
                    '[EffectRack1_EffectUnit' + (deck + 1) + '_Effect1]',
                    'parameter' + (parameter + 1),
                    fullValue / 0x3FFF
                );
                PioneerDDJSB2.fxKnobParameterSet = true;
            }
        }
    }
};

PioneerDDJSB2.softTakeoverEmulation = function(deck, index, currentValue) {
    var deltaToActive = currentValue - PioneerDDJSB2.fxParamsActiveValues[deck][index];

    if (Math.abs(deltaToActive) < 15) {
        PioneerDDJSB2.fxParamsActiveValues[deck][index] = currentValue;
        return true;
    }
    return false;
};


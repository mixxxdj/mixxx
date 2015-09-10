var PioneerDDJSB = function () {};

/*
    Find the latest code at https://github.com/jardiacaj/mixxx

    This mapping for the Pioneer DDJ-SB was made by Joan Ardiaca Jové (joan.ardiaca@gmail.com)
    basing on the work of wingcom (wwingcomm@gmail.com, https://github.com/wingcom/Mixxx-Pioneer-DDJ-SB).
    which in turn was based on the work of Hilton Rudham (https://github.com/hrudham/Mixxx-Pioneer-DDJ-SR).

    Just as wingcom's and Rudham's work, this mapping is pusblished under the MIT license.
 */

///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// If true the sync button blinks with the beat, if false led is lit when sync is enabled.
PioneerDDJSB.blinkingSync = false;

// If true, the vinyl button activates slip. Vinyl mode is then activated by using shift.
// Allows toggling slip faster, but is counterintuitive.
PioneerDDJSB.invertVinylSlipButton = false;

// Sets the jogwheels sensivity. 1 is default, 2 is twice as sensitive, 0.5 is half as sensitive.
PioneerDDJSB.jogwheelSensivity = 1.0;

// Sets how much more sensitive the jogwheels get when holding shift.
// Set to 1 to disable jogwheel sensitivity increase when holding shift.
PioneerDDJSB.jogwheelShiftMultiplier = 20;

///////////////////////////////////////////////////////////////
//                      INIT & SHUTDOWN                      //
///////////////////////////////////////////////////////////////


PioneerDDJSB.init = function (id) {
    PioneerDDJSB.scratchSettings = {
        'alpha': 1.0 / 8,
        'beta': 1.0 / 8 / 32,
        'jogResolution': 720,
        'vinylSpeed': 33 + 1/3,
        'safeScratchTimeout': 20
    };

    PioneerDDJSB.channelGroups = {
        '[Channel1]': 0x00,
        '[Channel2]': 0x01,
        '[Channel3]': 0x02,
        '[Channel4]': 0x03
    };

    PioneerDDJSB.samplerGroups = {
        '[Sampler1]': 0x00,
        '[Sampler2]': 0x01,
        '[Sampler3]': 0x02,
        '[Sampler4]': 0x03
    };

    PioneerDDJSB.fxGroups = {
        '[EffectRack1_EffectUnit1]': 0x00,
        '[EffectRack1_EffectUnit2]': 0x01
    };

    PioneerDDJSB.fxControls = {
        'group_[Channel1]_enable': 0x00,
        'group_[Channel3]_enable': 0x00,
        'group_[Headphone]_enable': 0x01,
        'group_[Channel2]_enable': 0x02,
        'group_[Channel4]_enable': 0x02
    };

    PioneerDDJSB.shiftPressed = false;

    PioneerDDJSB.chFaderStart = [
        null,
        null
    ];

    PioneerDDJSB.fxButtonPressed = [
        [false, false, false],
        [false, false, false]
    ];

    // used for soft takeover workaround
    PioneerDDJSB.fxParamsActiveValues = [
        [0, 0, 0, 0, 0],
        [0, 0, 0, 0, 0]
    ];

    PioneerDDJSB.scratchMode = [false, false, false, false];

    PioneerDDJSB.ledGroups = {
        'hotCue': 0x00,
        'autoLoop': 0x10,
        'manualLoop': 0x20,
        'sampler': 0x30
    };

    PioneerDDJSB.nonPadLeds = {
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

    PioneerDDJSB.loopIntervals = [1, 2, 4, 8, 16, 32, 64];

    PioneerDDJSB.looprollIntervals = [1/16, 1/8, 1/4, 1/2, 1, 2, 4, 8];

    PioneerDDJSB.setAllSoftTakeover(false);
    PioneerDDJSB.bindNonDeckControlConnections(false);
    PioneerDDJSB.initDeck('[Channel1]');
    PioneerDDJSB.initDeck('[Channel2]');
};

PioneerDDJSB.shutdown = function () {
    PioneerDDJSB.bindAllControlConnections(true);
    PioneerDDJSB.setAllSoftTakeover(true);
    PioneerDDJSB.bindDeckControlConnections('[Channel3]', true);
    PioneerDDJSB.bindDeckControlConnections('[Channel4]', true);
};


///////////////////////////////////////////////////////////////
//                      CONTROL BINDING                      //
///////////////////////////////////////////////////////////////


PioneerDDJSB.bindSamplerControlConnections = function (samplerGroup, isUnbinding) {
    engine.connectControl(samplerGroup, 'duration', 'PioneerDDJSB.samplerLeds', isUnbinding);
};

PioneerDDJSB.bindDeckControlConnections = function (channelGroup, isUnbinding) {
    var i,
        index,
        controlsToFunctions = {
            'play': 'PioneerDDJSB.playLeds',
            'pfl': 'PioneerDDJSB.headphoneCueLed',
            'keylock': 'PioneerDDJSB.keyLockLed',
            'slip_enabled': 'PioneerDDJSB.slipLed',
            'quantize': 'PioneerDDJSB.quantizeLed',
            'loop_in': 'PioneerDDJSB.loopInLed',
            'loop_out': 'PioneerDDJSB.loopOutLed',
            'filterLowKill': 'PioneerDDJSB.lowKillLed',
            'filterMidKill': 'PioneerDDJSB.midKillLed',
            'filterHighKill': 'PioneerDDJSB.highKillLed',
            'mute': 'PioneerDDJSB.muteLed',
            'loop_enabled': 'PioneerDDJSB.loopExitLed',
            'loop_double': 'PioneerDDJSB.loopDoubleLed',
            'loop_halve': 'PioneerDDJSB.loopHalveLed'
        };

    if (PioneerDDJSB.blinkingSync) {
        controlsToFunctions.beat_active = 'PioneerDDJSB.syncLed';
    } else {
        controlsToFunctions.sync_enabled = 'PioneerDDJSB.syncLed';
    }

    for (i = 1; i <= 8; i++) {
        controlsToFunctions['hotcue_' + i + '_enabled'] = 'PioneerDDJSB.hotCueLeds';
    }

    for (index in PioneerDDJSB.loopIntervals) {
        controlsToFunctions['beatloop_' + PioneerDDJSB.loopIntervals[index] + '_enabled'] = 'PioneerDDJSB.beatloopLeds';
    }

    for (index in PioneerDDJSB.looprollIntervals) {
        controlsToFunctions['beatlooproll_' + PioneerDDJSB.looprollIntervals[index] + '_activate'] = 'PioneerDDJSB.beatlooprollLeds';
    }

    script.bindConnections(channelGroup, controlsToFunctions, isUnbinding);

    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        engine.connectControl('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_' + channelGroup + '_enable', 'PioneerDDJSB.fxLeds', isUnbinding);
        if (!isUnbinding) {
            engine.trigger('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_' + channelGroup + '_enable');
        }
    }
};

PioneerDDJSB.bindNonDeckControlConnections = function (isUnbinding) {
    var samplerIndex,
        fxUnitIndex;

    for (samplerIndex = 1; samplerIndex <= 4; samplerIndex++) {
        PioneerDDJSB.bindSamplerControlConnections('[Sampler' + samplerIndex + ']', isUnbinding);
    }

    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        engine.connectControl('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_[Headphone]_enable', 'PioneerDDJSB.fxLeds', isUnbinding);
    }
};

PioneerDDJSB.bindAllControlConnections = function (isUnbinding) {
    var samplerIndex,
        fxUnitIndex,
        channelIndex;

    for (samplerIndex = 1; samplerIndex <= 4; samplerIndex++) {
        PioneerDDJSB.bindSamplerControlConnections('[Sampler' + samplerIndex + ']', isUnbinding);
    }

    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        engine.connectControl('[EffectRack1_EffectUnit' + fxUnitIndex + ']', 'group_[Headphone]_enable', 'PioneerDDJSB.fxLeds', isUnbinding);
    }

    for (channelIndex = 1; channelIndex <= 2; channelIndex++) {
        PioneerDDJSB.bindDeckControlConnections('[Channel' + channelIndex + ']', isUnbinding);
    }
};

PioneerDDJSB.setDeckSoftTakeover = function (channel, isUnbinding) {
    engine.softTakeover(channel, "volume", !isUnbinding);
    engine.softTakeover(channel, "rate", !isUnbinding);
    engine.softTakeover(channel, "pregain", !isUnbinding);
    engine.softTakeover(channel, "filterHigh", !isUnbinding);
    engine.softTakeover(channel, "filterMid", !isUnbinding);
    engine.softTakeover(channel, "filterLow", !isUnbinding);
    engine.softTakeover("[QuickEffectRack1_" + channel + "]", "super1", !isUnbinding);
};

PioneerDDJSB.setAllSoftTakeover = function (isUnbinding) {
    var channelIndex;
    for (channelIndex = 1; channelIndex <= 4; channelIndex++) {
        PioneerDDJSB.setDeckSoftTakeover('[Channel' + channelIndex + ']', isUnbinding);
    }
};


///////////////////////////////////////////////////////////////
//                       DECK SWITCHING                      //
///////////////////////////////////////////////////////////////

PioneerDDJSB.deckSwitchTable = {
    '[Channel1]': '[Channel1]',
    '[Channel2]': '[Channel2]'
};

PioneerDDJSB.initDeck = function (group) {
    PioneerDDJSB.bindDeckControlConnections(group, false);
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.shiftKeyLock, PioneerDDJSB.channelGroups[group] > 1);
    PioneerDDJSB.triggerVinylLed(PioneerDDJSB.channelGroups[group]);
};

PioneerDDJSB.deckToggleButton = function (channel, control, value, status, group) {
    var deckNumber = PioneerDDJSB.channelGroups[PioneerDDJSB.deckSwitchTable[group]] + 1;

    if (value) {
        // unbind current deck
        PioneerDDJSB.bindDeckControlConnections('[Channel' + deckNumber + ']', true);

        if (deckNumber <= 2) {
            deckNumber += 2;
        } else {
            deckNumber -= 2;
        }

        PioneerDDJSB.deckSwitchTable[group] = '[Channel' + deckNumber + ']';
        PioneerDDJSB.initDeck(PioneerDDJSB.deckSwitchTable[group]);
        PioneerDDJSB.setDeckSoftTakeover(PioneerDDJSB.deckSwitchTable[group], true);
        PioneerDDJSB.setDeckSoftTakeover(PioneerDDJSB.deckSwitchTable[group]);
    }
};


///////////////////////////////////////////////////////////////
//            HIGH RESOLUTION MIDI INPUT HANDLERS            //
///////////////////////////////////////////////////////////////

PioneerDDJSB.highResMSB = {
    '[Channel1]': {},
    '[Channel2]': {},
    '[Channel3]': {},
    '[Channel4]': {}
};

PioneerDDJSB.tempoSliderMSB = function (channel, control, value, status, group) {
    PioneerDDJSB.highResMSB[group].tempoSlider = value;
};

PioneerDDJSB.tempoSliderLSB = function (channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB.highResMSB[group].tempoSlider << 7) + value;
    engine.setValue(
        PioneerDDJSB.deckSwitchTable[group],
        'rate',
        ((0x4000 - fullValue) - 0x2000) / 0x2000
    );
};

PioneerDDJSB.gainKnobMSB = function (channel, control, value, status, group) {
    PioneerDDJSB.highResMSB[group].gainKnob = value;
};

PioneerDDJSB.gainKnobLSB = function (channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB.highResMSB[group].gainKnob << 7) + value;
    engine.setValue(
        PioneerDDJSB.deckSwitchTable[group],
        'pregain',
        script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF)
    );
};

PioneerDDJSB.filterHighKnobMSB = function (channel, control, value, status, group) {
    PioneerDDJSB.highResMSB[group].filterHigh = value;
};

PioneerDDJSB.filterHighKnobLSB = function (channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB.highResMSB[group].filterHigh << 7) + value;
    engine.setValue(
        PioneerDDJSB.deckSwitchTable[group],
        'filterHigh',
        script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF)
    );
};

PioneerDDJSB.filterMidKnobMSB = function (channel, control, value, status, group) {
    PioneerDDJSB.highResMSB[group].filterMid = value;
};

PioneerDDJSB.filterMidKnobLSB = function (channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB.highResMSB[group].filterMid << 7) + value;
    engine.setValue(
        PioneerDDJSB.deckSwitchTable[group],
        'filterMid',
        script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF))
    ;
};

PioneerDDJSB.filterLowKnobMSB = function (channel, control, value, status, group) {
    PioneerDDJSB.highResMSB[group].filterLow = value;
};

PioneerDDJSB.filterLowKnobLSB = function (channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB.highResMSB[group].filterLow << 7) + value;
    engine.setValue(
        PioneerDDJSB.deckSwitchTable[group],
        'filterLow',
        script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF)
    );
};

PioneerDDJSB.deckFaderMSB = function (channel, control, value, status, group) {
    PioneerDDJSB.highResMSB[group].deckFader = value;
};

PioneerDDJSB.deckFaderLSB = function (channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB.highResMSB[group].deckFader << 7) + value;
    if (PioneerDDJSB.shiftPressed &&
        engine.getValue(PioneerDDJSB.deckSwitchTable[group], 'volume') === 0 &&
        fullValue !== 0 &&
        engine.getValue(PioneerDDJSB.deckSwitchTable[group], 'play') === 0
    ) {
        PioneerDDJSB.chFaderStart[channel] = engine.getValue(PioneerDDJSB.deckSwitchTable[group], 'playposition');
        engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'play', 1);
    }
    else if (
        PioneerDDJSB.shiftPressed &&
        engine.getValue(PioneerDDJSB.deckSwitchTable[group], 'volume') !== 0 &&
        fullValue === 0 &&
        engine.getValue(PioneerDDJSB.deckSwitchTable[group], 'play') === 1 &&
        PioneerDDJSB.chFaderStart[channel] !== null
    ) {
        engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'play', 0);
        engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'playposition', PioneerDDJSB.chFaderStart[channel]);
        PioneerDDJSB.chFaderStart[channel] = null;
    }
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'volume', fullValue / 0x3FFF);
};

PioneerDDJSB.filterKnobMSB = function (channel, control, value, status, group) {
    PioneerDDJSB.highResMSB[group].filterKnob = value;
};

PioneerDDJSB.filterKnobLSB = function (channel, control, value, status, group) {
    var fullValue = (PioneerDDJSB.highResMSB[group].filterKnob << 7) + value;
    if (PioneerDDJSB.shiftPressed) {
        engine.setValue(
            PioneerDDJSB.deckSwitchTable[group],
            'pregain',
            script.absoluteNonLin(fullValue, 0.0, 1.0, 4.0, 0, 0x3FFF)
        );
    } else {
        engine.setValue('[QuickEffectRack1_' + PioneerDDJSB.deckSwitchTable[group] + ']', 'super1', fullValue / 0x3FFF);
    }
};


///////////////////////////////////////////////////////////////
//           SINGLE MESSAGE MIDI INPUT HANDLERS              //
///////////////////////////////////////////////////////////////

PioneerDDJSB.shiftButton = function (channel, control, value, status, group) {
    PioneerDDJSB.shiftPressed = (value == 0x7F);
    for (index in PioneerDDJSB.chFaderStart) {
        PioneerDDJSB.chFaderStart[index] = null;
    }
};

PioneerDDJSB.playButton = function (channel, control, value, status, group) {
    if (value) {
        script.toggleControl(PioneerDDJSB.deckSwitchTable[group], 'play');
    }
};

PioneerDDJSB.headphoneCueButton = function (channel, control, value, status, group) {
    if (value) {
        script.toggleControl(PioneerDDJSB.deckSwitchTable[group], 'pfl');
    }
};

PioneerDDJSB.hotCueButtons = function (channel, control, value, status, group) {
    var hotCueIndex = (control >= 0x40 ? control - 0x40 + 5 : control + 1);
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'hotcue_' + hotCueIndex + '_activate', value);
};

PioneerDDJSB.clearHotCueButtons = function (channel, control, value, status, group) {
    var hotCueIndex = (control >= 0x48 ? control - 0x48 + 5 : control - 7);
    if (value) {
        engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'hotcue_' + hotCueIndex + '_clear', 1);
    }
};

PioneerDDJSB.cueButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'cue_default', value);
};

PioneerDDJSB.beatloopButtons = function (channel, control, value, status, group) {
    var index = (control <= 0x13 ? control - 0x10 : control - 0x14);
    if (value) {
        engine.setValue(
            PioneerDDJSB.deckSwitchTable[group],
            'beatloop_' + PioneerDDJSB.loopIntervals[index] + '_toggle',
            1
        );
    }
};

PioneerDDJSB.beatloopRollButtons = function (channel, control, value, status, group) {
    var index = (control <= 0x53 ? control - 0x50 : control - 0x54);
    engine.setValue(
        PioneerDDJSB.deckSwitchTable[group],
        'beatlooproll_' + PioneerDDJSB.looprollIntervals[index] + '_activate',
        value
    );
};

PioneerDDJSB.vinylButton = function (channel, control, value, status, group) {
    if (PioneerDDJSB.invertVinylSlipButton) {
        PioneerDDJSB.toggleSlip(channel, control, value, status, group);
    } else {
        PioneerDDJSB.toggleScratch(channel, control, value, status, group);
    }
};

PioneerDDJSB.slipButton = function (channel, control, value, status, group) {
    if (PioneerDDJSB.invertVinylSlipButton) {
        PioneerDDJSB.toggleScratch(channel, control, value, status, group);
    } else {
        PioneerDDJSB.toggleSlip(channel, control, value, status, group);
    }
};

PioneerDDJSB.toggleSlip = function (channel, control, value, status, group) {
    if (value) {
        script.toggleControl(PioneerDDJSB.deckSwitchTable[group], 'slip_enabled');
    }
};

PioneerDDJSB.keyLockButton = function (channel, control, value, status, group) {
    if (value) {
        script.toggleControl(PioneerDDJSB.deckSwitchTable[group], 'keylock');
    }
};

PioneerDDJSB.loopInButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'loop_in', value ? 1 : 0);
};

PioneerDDJSB.loopOutButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'loop_out', value ? 1 : 0);
};

PioneerDDJSB.loopExitButton = function (channel, control, value, status, group) {
    if (value) {
        engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'reloop_exit', 1);
    }
};

PioneerDDJSB.loopHalveButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'loop_halve', value ? 1 : 0);
};

PioneerDDJSB.loopDoubleButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'loop_double', value ? 1 : 0);
};

PioneerDDJSB.loopMoveBackButton = function (channel, control, value, status, group) {
    if (value) {
        engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'loop_move', -1);
    }
};

PioneerDDJSB.loopMoveForwardButton = function (channel, control, value, status, group) {
    if (value) {
        engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'loop_move', 1);
    }
};

PioneerDDJSB.loadButton = function (channel, control, value, status, group) {
    if (value) {
        engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'LoadSelectedTrack', 1);
    }
};

PioneerDDJSB.reverseRollButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'reverseroll', value);
};

PioneerDDJSB.brakeButton = function (channel, control, value, status, group) {
    script.brake(channel, control, value, status, PioneerDDJSB.deckSwitchTable[group]);
};

PioneerDDJSB.syncButton = function (channel, control, value, status, group) {
    if (value) {
        script.toggleControl(PioneerDDJSB.deckSwitchTable[group], 'sync_enabled');
    }
};

PioneerDDJSB.quantizeButton = function (channel, control, value, status, group) {
    if (value) {
        script.toggleControl(PioneerDDJSB.deckSwitchTable[group], 'quantize');
    }
};

PioneerDDJSB.lowKillButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'filterLowKill', value ? 1 : 0);
};

PioneerDDJSB.midKillButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'filterMidKill', value ? 1 : 0);
};

PioneerDDJSB.highKillButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'filterHighKill', value ? 1 : 0);
};

PioneerDDJSB.muteButton = function (channel, control, value, status, group) {
    engine.setValue(PioneerDDJSB.deckSwitchTable[group], 'mute', value);
};


///////////////////////////////////////////////////////////////
//                          LED HELPERS                      //
///////////////////////////////////////////////////////////////

PioneerDDJSB.deckConverter = function (group) {
    var index;

    if (typeof group === "string") {
        for (index in PioneerDDJSB.deckSwitchTable) {
            if (group === PioneerDDJSB.deckSwitchTable[index]) {
                return PioneerDDJSB.channelGroups[group] % 2;
            }
        }
        return null;
    }
    return group % 2;
};

PioneerDDJSB.fxLedControl = function (deck, ledNumber, shift, active) {
    var fxLedsBaseChannel = 0x94,
        fxLedsBaseControl = (shift ? 0x63 : 0x47),
        midiChannelOffset = PioneerDDJSB.deckConverter(deck);

    if (midiChannelOffset !== null) {
        midi.sendShortMsg(
            fxLedsBaseChannel + midiChannelOffset,
            fxLedsBaseControl + ledNumber,
            active ? 0x7F : 0x00
        );
    }
};

PioneerDDJSB.padLedControl = function (deck, groupNumber, shiftGroup, ledNumber, shift, active) {
    var padLedsBaseChannel = 0x97,
        padLedControl = (shiftGroup ? 0x40 : 0x00) + (shift ? 0x08 : 0x00) + (+groupNumber) + (+ledNumber),
        midiChannelOffset = PioneerDDJSB.deckConverter(deck);

    if (midiChannelOffset !== null) {
        midi.sendShortMsg(
            padLedsBaseChannel + midiChannelOffset,
            padLedControl,
            active ? 0x7F : 0x00
        );
    }
};

PioneerDDJSB.nonPadLedControl = function (deck, ledNumber, active) {
    var nonPadLedsBaseChannel = 0x90,
        midiChannelOffset = PioneerDDJSB.deckConverter(deck);

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

PioneerDDJSB.fxLeds = function (value, group, control) {
    var deck = PioneerDDJSB.fxGroups[group],
        ledNumber = PioneerDDJSB.fxControls[control];

    PioneerDDJSB.fxLedControl(deck, ledNumber, false, value);
    PioneerDDJSB.fxLedControl(deck, ledNumber, true, value);
};

PioneerDDJSB.headphoneCueLed = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.headphoneCue, value);
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.shiftHeadphoneCue, value);
};

PioneerDDJSB.keyLockLed = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.keyLock, value);
};

PioneerDDJSB.playLeds = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.play, value);
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.shiftPlay, value);
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.cue, value);
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.shiftCue, value);
};

PioneerDDJSB.slipLed = function (value, group, control) {
    var led = (PioneerDDJSB.invertVinylSlipButton ? PioneerDDJSB.nonPadLeds.vinyl : PioneerDDJSB.nonPadLeds.shiftVinyl);
    PioneerDDJSB.nonPadLedControl(group, led, value);
};

PioneerDDJSB.quantizeLed = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.shiftSync, value);
};

PioneerDDJSB.syncLed = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.sync, value);
};

PioneerDDJSB.loopInLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, false, 0, false, value);
};

PioneerDDJSB.loopOutLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, false, 1, false, value);
};

PioneerDDJSB.loopExitLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, false, 2, false, value);
};

PioneerDDJSB.loopHalveLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, false, 3, false, value);
};

PioneerDDJSB.loopDoubleLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, false, 3, true, value);
};

PioneerDDJSB.lowKillLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, true, 0, false, value);
};

PioneerDDJSB.midKillLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, true, 1, false, value);
};

PioneerDDJSB.highKillLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, true, 2, false, value);
};

PioneerDDJSB.muteLed = function (value, group, control) {
    PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.manualLoop, true, 3, false, value);
};

PioneerDDJSB.samplerLeds = function (value, group, control) {
    var sampler = PioneerDDJSB.samplerGroups[group],
        channel;

    for (channel = 0; channel < 2; channel++) {
        PioneerDDJSB.padLedControl(channel, PioneerDDJSB.ledGroups.sampler, false, sampler, false, value);
        PioneerDDJSB.padLedControl(channel, PioneerDDJSB.ledGroups.sampler, false, sampler, true, value);
        PioneerDDJSB.padLedControl(channel, PioneerDDJSB.ledGroups.sampler, true, sampler, false, value);
        PioneerDDJSB.padLedControl(channel, PioneerDDJSB.ledGroups.sampler, true, sampler, true, value);
    }
};

PioneerDDJSB.beatloopLeds = function (value, group, control) {
    var index,
        padNum,
        shifted;

    for (index in PioneerDDJSB.loopIntervals) {
        if (control === 'beatloop_' + PioneerDDJSB.loopIntervals[index] + '_enabled') {
            padNum = index % 4;
            shifted = (index >= 4);
            PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.autoLoop, false, padNum, shifted, value);
        }
    }
};

PioneerDDJSB.beatlooprollLeds = function (value, group, control) {
    var index,
        padNum,
        shifted;

    for (index in PioneerDDJSB.looprollIntervals) {
        if (control === 'beatlooproll_' + PioneerDDJSB.looprollIntervals[index] + '_activate') {
            padNum = index % 4;
            shifted = (index >= 4);
            PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.autoLoop, true, padNum, shifted, value);
        }
    }
};

PioneerDDJSB.hotCueLeds = function (value, group, control) {
    var shiftedGroup = false,
        padNum = null,
        hotCueNum;

    for (hotCueNum = 1; hotCueNum <= 8; hotCueNum++) {
        if (control === 'hotcue_' + hotCueNum + '_enabled') {
            padNum = (hotCueNum - 1) % 4;
            shiftedGroup = (hotCueNum > 4);
            PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.hotCue, shiftedGroup, padNum, false, value);
            PioneerDDJSB.padLedControl(group, PioneerDDJSB.ledGroups.hotCue, shiftedGroup, padNum, true, value);
        }
    }
};


///////////////////////////////////////////////////////////////
//                          JOGWHEELS                        //
///////////////////////////////////////////////////////////////

// Work out the jog-wheel change / delta
PioneerDDJSB.getJogWheelDelta = function (value) {
    // The Wheel control centers on 0x40; find out how much it's moved by.
    return value - 0x40;
};

PioneerDDJSB.jogRingTick = function (channel, control, value, status, group) {
    PioneerDDJSB.pitchBendFromJog(PioneerDDJSB.deckSwitchTable[group], PioneerDDJSB.getJogWheelDelta(value));
};

PioneerDDJSB.jogRingTickShift = function (channel, control, value, status, group) {
    PioneerDDJSB.pitchBendFromJog(
        PioneerDDJSB.deckSwitchTable[group],
        PioneerDDJSB.getJogWheelDelta(value) * PioneerDDJSB.jogwheelShiftMultiplier
    );
};

PioneerDDJSB.jogPlatterTick = function (channel, control, value, status, group) {
    var deck = PioneerDDJSB.channelGroups[PioneerDDJSB.deckSwitchTable[group]];
    if (PioneerDDJSB.scratchMode[deck]) {
        engine.scratchTick(deck + 1, PioneerDDJSB.getJogWheelDelta(value));
    } else {
        PioneerDDJSB.pitchBendFromJog(PioneerDDJSB.deckSwitchTable[group], PioneerDDJSB.getJogWheelDelta(value));
    }
};

PioneerDDJSB.jogPlatterTickShift = function (channel, control, value, status, group) {
    var deck = PioneerDDJSB.channelGroups[PioneerDDJSB.deckSwitchTable[group]];
    if (PioneerDDJSB.scratchMode[deck]) {
        engine.scratchTick(deck + 1, PioneerDDJSB.getJogWheelDelta(value));
    } else {
        PioneerDDJSB.pitchBendFromJog(
            PioneerDDJSB.deckSwitchTable[group],
                PioneerDDJSB.getJogWheelDelta(value) * PioneerDDJSB.jogwheelShiftMultiplier
        );
    }
};

PioneerDDJSB.jogTouch = function (channel, control, value, status, group) {
    var deck = PioneerDDJSB.channelGroups[PioneerDDJSB.deckSwitchTable[group]];
    if (PioneerDDJSB.scratchMode[deck]) {
        if (value) {
            engine.scratchEnable(
                deck + 1,
                PioneerDDJSB.scratchSettings.jogResolution,
                PioneerDDJSB.scratchSettings.vinylSpeed,
                PioneerDDJSB.scratchSettings.alpha,
                PioneerDDJSB.scratchSettings.beta,
                true
            );
        } else {
            engine.scratchDisable(deck + 1, true);
        }
    }
};

PioneerDDJSB.toggleScratch = function (channel, control, value, status, group) {
    var deck = PioneerDDJSB.channelGroups[PioneerDDJSB.deckSwitchTable[group]];
    if (value) {
        PioneerDDJSB.scratchMode[deck] = !PioneerDDJSB.scratchMode[deck];
        PioneerDDJSB.triggerVinylLed(deck);
        if (!PioneerDDJSB.scratchMode[deck]) {
            engine.scratchDisable(deck + 1, true);
        }
    }
};

PioneerDDJSB.triggerVinylLed = function (deck) {
    var led = (PioneerDDJSB.invertVinylSlipButton ? PioneerDDJSB.nonPadLeds.shiftVinyl : PioneerDDJSB.nonPadLeds.vinyl);
    PioneerDDJSB.nonPadLedControl(deck % 2, led, PioneerDDJSB.scratchMode[deck]);
};

PioneerDDJSB.pitchBendFromJog = function (channel, movement) {
    var group = (typeof channel === "string" ? channel : '[Channel' + channel + 1 + ']');
    engine.setValue(group, 'jog', movement / 5 * PioneerDDJSB.jogwheelSensivity);
};


///////////////////////////////////////////////////////////////
//                        ROTARY SELECTOR                    //
///////////////////////////////////////////////////////////////
// Handles the rotary selector for choosing tracks, library items, crates, etc.

PioneerDDJSB.getRotaryDelta = function (value) {
    var delta = 0x40 - Math.abs(0x40 - value),
        isCounterClockwise = value > 0x40;

    if (isCounterClockwise) {
        delta *= -1;
    }
    return delta;
};

PioneerDDJSB.rotarySelector = function (channel, control, value, status) {
    var delta = PioneerDDJSB.getRotaryDelta(value);
    engine.setValue('[Playlist]', 'SelectTrackKnob', delta);
};

PioneerDDJSB.shiftedRotarySelector = function (channel, control, value, status) {
    var delta = PioneerDDJSB.getRotaryDelta(value),
        f = (delta > 0 ? 'SelectNextPlaylist' : 'SelectPrevPlaylist');

    engine.setValue('[Playlist]', f, Math.abs(delta));
};

PioneerDDJSB.rotarySelectorShiftedClick = function (channel, control, value, status) {
    if (value) {
        engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1);
    }
};


///////////////////////////////////////////////////////////////
//                             FX                            //
///////////////////////////////////////////////////////////////

PioneerDDJSB.fxKnobMSB = [0, 0];
PioneerDDJSB.fxKnobShiftedMSB = [0, 0];

PioneerDDJSB.fxButton = function (channel, control, value, status, group) {
    var deck = channel - 4,
        button = control - 0x47,
        channel = PioneerDDJSB.deckSwitchTable['[Channel' + (button === 0 ? 1 : 2) + ']'];

    PioneerDDJSB.fxButtonPressed[deck][button] = (value === 0x7F);

    if (button === 1) {
        engine.trigger(group, 'group_[Headphone]_enable');
    } else {
        engine.trigger(group, 'group_' + channel + '_enable');
    }
};

PioneerDDJSB.fxButtonShifted = function (channel, control, value, status, group) {
    var button = control - 0x63,
        channel = PioneerDDJSB.deckSwitchTable['[Channel' + (button === 0 ? 1 : 2) + ']'];

    if (value) {
        if (button === 1) {
            script.toggleControl(group, 'group_[Headphone]_enable');
        } else {
            script.toggleControl(group, 'group_' + channel + '_enable');
        }
    }
};

PioneerDDJSB.fxKnobShiftedMSB = function (channel, control, value, status) {
    PioneerDDJSB.fxKnobShiftedMSB[channel - 4] = value;
};

PioneerDDJSB.fxKnobShiftedLSB = function (channel, control, value, status) {
    var deck = channel - 4,
        fullValue = (PioneerDDJSB.fxKnobShiftedMSB[deck] << 7) + value;

    if (PioneerDDJSB.softTakeoverEmulation(deck, 4, PioneerDDJSB.fxKnobShiftedMSB[deck])) {
        engine.setValue('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'super1', fullValue / 0x3FFF);
    }
};

PioneerDDJSB.fxKnobMSB = function (channel, control, value, status) {
    PioneerDDJSB.fxKnobMSB[channel - 4] = value;
};

PioneerDDJSB.fxKnobLSB = function (channel, control, value, status) {
    var deck = channel - 4,
        anyButtonPressed = false,
        fullValue = (PioneerDDJSB.fxKnobMSB[deck] << 7) + value,
        parameter;

    for (parameter = 0; parameter < 3; parameter++) {
        if (PioneerDDJSB.fxButtonPressed[deck][parameter]) {
            anyButtonPressed = true;
        }
    }

    if (!anyButtonPressed) {
        if (PioneerDDJSB.softTakeoverEmulation(deck, 3, PioneerDDJSB.fxKnobMSB[deck])) {
            engine.setValue('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'mix', fullValue / 0x3FFF);
        }
    } else {
        for (parameter = 0; parameter < 3; parameter++) {
            if (PioneerDDJSB.fxButtonPressed[deck][parameter] && PioneerDDJSB.softTakeoverEmulation(deck, parameter, PioneerDDJSB.fxKnobMSB[deck])) {
                engine.setParameter(
                    '[EffectRack1_EffectUnit' + (deck + 1) + '_Effect1]',
                    'parameter' + (parameter + 1),
                    fullValue / 0x3FFF
                );
            }
        }
    }
};

PioneerDDJSB.softTakeoverEmulation = function (deck, index, currentValue) {
    var deltaToActive = currentValue - PioneerDDJSB.fxParamsActiveValues[deck][index];

    if (Math.abs(deltaToActive) < 15) {
        PioneerDDJSB.fxParamsActiveValues[deck][index] = currentValue;
        return true;
    }
    return false;
};
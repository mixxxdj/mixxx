var PioneerDDJSB = function () {};

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
        '[Channel2]': 0x01
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
        'group_[Channel2]_enable': 0x01,
        'group_[Headphone]_enable': 0x02
    };

    PioneerDDJSB.fxButtonPressed = [
        [false, false, false],
        [false, false, false]
    ];

    // used for soft takeover workaround
    PioneerDDJSB.fxParamsActiveValues = [
        [0, 0, 0, 0],
        [0, 0, 0, 0]
    ];

    PioneerDDJSB.fxPreviousKnobValues = [
        [0, 0, 0, 0],
        [0, 0, 0, 0]
    ];

    PioneerDDJSB.scratchMode = [false, false];

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
        'shiftSync': 0x5C,
        'sync': 0x58
    };

    PioneerDDJSB.loopIntervals = [1, 2, 4, 8, 16, 32, 64];

    PioneerDDJSB.looprollIntervals = [0.0625, 0.125, 0.25, 0.5];

    PioneerDDJSB.bindControlConnections(false);
};

PioneerDDJSB.shutdown = function () {
    PioneerDDJSB.bindControlConnections(true);
};

PioneerDDJSB.bulkBindControlConnections = function (group, controlsToFunctions, remove) {
    var control;
    remove = (remove === undefined) ? remove : false;

    for (control in controlsToFunctions) {
        engine.connectControl(group, control, controlsToFunctions[control], remove);
        if (!remove) {
            engine.trigger(group, control);
        }
    };
};

PioneerDDJSB.bindControlConnections = function (isUnbinding) {
    var controlsToFunctions = {},
        samplerIndex,
        samplerGroup,
        fxUnitIndex,
        effectUnitGroup,
        channelIndex,
        channelGroup,
        index,
        i;

    for (samplerIndex = 1; samplerIndex <= 4; samplerIndex++) {
        samplerGroup = '[Sampler' + samplerIndex + ']';
        engine.connectControl(samplerGroup, 'duration', 'PioneerDDJSB.samplerLeds', isUnbinding);
    }

    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        effectUnitGroup = '[EffectRack1_EffectUnit' + fxUnitIndex + ']';
        controlsToFunctions = {
            'group_[Headphone]_enable': 'PioneerDDJSB.fxLeds',
            'group_[Channel1]_enable': 'PioneerDDJSB.fxLeds',
            'group_[Channel2]_enable': 'PioneerDDJSB.fxLeds'
        };
        PioneerDDJSB.bulkBindControlConnections(effectUnitGroup, controlsToFunctions, isUnbinding);
    }

    for (channelIndex = 1; channelIndex <= 2; channelIndex++) {
        channelGroup = '[Channel' + channelIndex + ']';
        controlsToFunctions = {
            'play': 'PioneerDDJSB.playLeds',
            'cue_default': 'PioneerDDJSB.cueLeds',
            'pfl': 'PioneerDDJSB.headphoneCueLed',
            'keylock': 'PioneerDDJSB.keyLockLeds',
            'slip_enabled': 'PioneerDDJSB.vinylLed',
            'quantize': 'PioneerDDJSB.quantizeLed',
            'beat_active': 'PioneerDDJSB.beatLed',
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

        for (i = 1; i <= 8; i++) {
            controlsToFunctions['hotcue_' + i + '_enabled'] = 'PioneerDDJSB.hotCueLeds';
        }

        for (index in PioneerDDJSB.loopIntervals) {
            controlsToFunctions['beatloop_' + PioneerDDJSB.loopIntervals[index] + '_enabled'] = 'PioneerDDJSB.beatloopLeds';
        }

        for (index in PioneerDDJSB.looprollIntervals) {
            controlsToFunctions['beatlooproll_' + PioneerDDJSB.looprollIntervals[index] + '_activate'] = 'PioneerDDJSB.beatlooprollLeds';
        }

        PioneerDDJSB.bulkBindControlConnections(channelGroup, controlsToFunctions, isUnbinding);
    }
};


///////////////////////////////////////////////////////////////
//                      LED HELPERS SECTION                  //
///////////////////////////////////////////////////////////////

PioneerDDJSB.deckConverter = function (group) {
    if (typeof group === "string") {
        return PioneerDDJSB.channelGroups[group];
    }
    return group;
};

PioneerDDJSB.fxLedControl = function (deck, ledNumber, shift, active) {
    var fxLedsBaseChannel = 0x94,
        fxLedsBaseControl = (shift ? 0x63 : 0x47);

    midi.sendShortMsg(
        fxLedsBaseChannel + PioneerDDJSB.deckConverter(deck),
        fxLedsBaseControl + ledNumber,
        active ? 0x7F : 0x00
    );
};

PioneerDDJSB.padLedControl = function (deck, groupNumber, shiftGroup, ledNumber, shift, active) {
    var padLedsBaseChannel = 0x97,
        padLedControl = (shiftGroup ? 0x40 : 0x00) + (shift ? 0x08 : 0x00) + (+groupNumber) + (+ledNumber);

    midi.sendShortMsg(
        padLedsBaseChannel + PioneerDDJSB.deckConverter(deck),
        padLedControl,
        active ? 0x7F : 0x00
    );
};

PioneerDDJSB.nonPadLedControl = function (deck, ledNumber, active) {
    var nonPadLedsBaseChannel = 0x90;

    midi.sendShortMsg(
        nonPadLedsBaseChannel + PioneerDDJSB.deckConverter(deck),
        ledNumber,
        active ? 0x7F : 0x00
    );
};


///////////////////////////////////////////////////////////////
//                         LED SECTION                       //
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

PioneerDDJSB.cueLeds = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.cue, value);
};

PioneerDDJSB.keyLockLeds = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.keyLock, value);
};

PioneerDDJSB.playLeds = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.play, value);
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.shiftPlay, value);
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.cue, value);
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.shiftCue, value);
};

PioneerDDJSB.vinylLed = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.vinyl, value);
};

PioneerDDJSB.quantizeLed = function (value, group, control) {
    PioneerDDJSB.nonPadLedControl(group, PioneerDDJSB.nonPadLeds.shiftSync, value);
};

PioneerDDJSB.beatLed = function (value, group, control) {
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
//                     JOGWHEEL SECTION                      //
///////////////////////////////////////////////////////////////

// Work out the jog-wheel change / delta
PioneerDDJSB.getJogWheelDelta = function (value) {
    // The Wheel control centers on 0x40; find out how much it's moved by.
    return value - 0x40;
};

PioneerDDJSB.jogRingTick = function (channel, control, value, status, group) {
    PioneerDDJSB.pitchBend(channel, PioneerDDJSB.getJogWheelDelta(value));
};

PioneerDDJSB.jogRingTickShift = function (channel, control, value, status, group) {
    PioneerDDJSB.pitchBend(channel, PioneerDDJSB.getJogWheelDelta(value) * 20);
};

PioneerDDJSB.jogPlatterTick = function (channel, control, value, status, group) {
    if (PioneerDDJSB.scratchMode[channel]) {
        engine.scratchTick(channel + 1, PioneerDDJSB.getJogWheelDelta(value));
    } else {
        PioneerDDJSB.pitchBend(channel, PioneerDDJSB.getJogWheelDelta(value));
    }
};

PioneerDDJSB.jogPlatterTickShift = function (channel, control, value, status, group) {
    if (PioneerDDJSB.scratchMode[channel]) {
        engine.scratchTick(channel + 1, PioneerDDJSB.getJogWheelDelta(value));
    } else {
        PioneerDDJSB.pitchBend(channel, PioneerDDJSB.getJogWheelDelta(value) * 20);
    }
};

PioneerDDJSB.jogTouch = function (channel, control, value, status) {
    if (PioneerDDJSB.scratchMode[channel]) {
        if (value === 0x7F) {
            engine.scratchEnable(
                channel + 1,
                PioneerDDJSB.scratchSettings.jogResolution,
                PioneerDDJSB.scratchSettings.vinylSpeed,
                PioneerDDJSB.scratchSettings.alpha,
                PioneerDDJSB.scratchSettings.beta,
                true
            );
        } else {
            engine.scratchDisable(channel + 1, true);
        }
    }
};

PioneerDDJSB.vinylButton = function (channel, control, value, status) {
    if (value === 0x7F) {
        PioneerDDJSB.scratchMode[channel] = !PioneerDDJSB.scratchMode[channel];
        if (!PioneerDDJSB.scratchMode[channel]) {
            engine.scratchDisable(channel + 1, true);
        }
        PioneerDDJSB.nonPadLedControl(channel, PioneerDDJSB.nonPadLeds.shiftVinyl, PioneerDDJSB.scratchMode[channel]);
    }
};

PioneerDDJSB.pitchBend = function (channel, movement) {
    var deck = channel + 1,
        group = '[Channel' + deck + ']';
    engine.setValue(group, 'jog', movement / 5);
};


///////////////////////////////////////////////////////////////
//                    ROTARY SELECTOR SECTION                //
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
    if (value === 0x7F) {
        engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1);
    }
};


///////////////////////////////////////////////////////////////
//                         FX SECTION                        //
///////////////////////////////////////////////////////////////

PioneerDDJSB.fxButton = function (channel, control, value, status) {
    var deck = channel - 4,
        button = control - 0x47;

    PioneerDDJSB.fxButtonPressed[deck][button] = (value === 0x7F);

    if (button < 2) {
        engine.trigger('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'group_[Channel' + (button + 1) + ']_enable');

    } else {
        engine.trigger('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'group_[Headphone]_enable');
    }
};

PioneerDDJSB.fxKnob = function (channel, control, value, status) {
    var deck = channel - 4,
        anyButtonPressed = false,
        parameter;

    for (parameter = 0; parameter < 3; parameter++) {
        if (PioneerDDJSB.fxButtonPressed[deck][parameter]) {
            anyButtonPressed = true;
        }
    }

    if (!anyButtonPressed) {
        if (PioneerDDJSB.softTakeoverEmulation(deck, 3, value)) {
            engine.setValue('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'mix', value / 0x7F);
        }
    } else {
        for (parameter = 0; parameter < 3; parameter++) {
            if (PioneerDDJSB.fxButtonPressed[deck][parameter] && PioneerDDJSB.softTakeoverEmulation(deck, parameter, value)) {
                engine.setParameter(
                    '[EffectRack1_EffectUnit' + (deck + 1) + '_Effect1]',
                    'parameter' + (parameter + 1),
                    value / 0x7F
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
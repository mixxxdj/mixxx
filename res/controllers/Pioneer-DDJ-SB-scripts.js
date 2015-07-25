var PioneerDDJSB = function () {};

PioneerDDJSB.init = function (id) {
    PioneerDDJSB.settings = {
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

    PioneerDDJSB.loopIntervals = [1, 2, 4, 8, 16, 32, 64];

    PioneerDDJSB.looprollIntervals = [0.0625, 0.125, 0.25, 0.5];

    PioneerDDJSB.bindControlConnections(false);
};

PioneerDDJSB.shutdown = function () {
    PioneerDDJSB.bindControlConnections(true);
};

PioneerDDJSB.bulkBindControlConnections = function (group, controlsToFunctions, remove) {
    //noinspection AssignmentToFunctionParameterJS
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
            'loop_enabled': 'PioneerDDJSB.loopExitLed'
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
//                         LED SECTION                       //
///////////////////////////////////////////////////////////////

// Returns the control code for a led on the pads.
// groupShift is true when the group has been selected while pressing shift
PioneerDDJSB.padLedTranslator = function (ledGroup, ledNumber, shift, groupShift) {
    return (groupShift ? 0x40 : 0x00) + (shift ? 0x08 : 0x00) + ledGroup + (+ledNumber);
};


PioneerDDJSB.fxLeds = function (value, group, control) {
    midi.sendShortMsg(
        0x94 + PioneerDDJSB.fxGroups[group],
        0x47 + PioneerDDJSB.fxControls[control],
        value ? 0x7F : 0x00
    );
    midi.sendShortMsg(
        0x94 + PioneerDDJSB.fxGroups[group],
        0x63 + PioneerDDJSB.fxControls[control],
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.headphoneCueLed = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x54, value ? 0x7F : 0x00);
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x68, value ? 0x7F : 0x00);
};

PioneerDDJSB.cueLeds = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x0C, value ? 0x7F : 0x00);
};

PioneerDDJSB.keyLockLeds = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x1A, value ? 0x7F : 0x00);
};

PioneerDDJSB.playLeds = function (value, group, control) {
    var channel = PioneerDDJSB.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x0B, value ? 0x7F : 0x00); // Play / Pause LED
    midi.sendShortMsg(0x90 + channel, 0x0C, value ? 0x7F : 0x00); // Cue LED
    midi.sendShortMsg(0x90 + channel, 0x47, value ? 0x7F : 0x00); // Play / Pause LED when shifted
    midi.sendShortMsg(0x90 + channel, 0x48, value ? 0x7F : 0x00); // Cue LED when shifted
};

PioneerDDJSB.vinylLed = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x17, value ? 0x7F : 0x00);
};

PioneerDDJSB.quantizeLed = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x5C, value ? 0x7F : 0x00);
};

PioneerDDJSB.beatLed = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x58, value ? 0x7F : 0x00);
};

PioneerDDJSB.loopInLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 0, false, false),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.loopOutLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 1, false, false),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.loopExitLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 2, false, false),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.lowKillLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 0, false, true),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.midKillLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 1, false, true),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.highKillLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 2, false, true),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.muteLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 3, false, true),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.samplerLeds = function (value, group, control) {
    var sampler = PioneerDDJSB.samplerGroups[group],
        channel;

    for (channel = 0; channel < 2; channel++) {
        midi.sendShortMsg(
            0x97 + channel,
            PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, false, false),
            value ? 0x7F : 0x00
        );
        midi.sendShortMsg(
            0x97 + channel,
            PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, false, true),
            value ? 0x7F : 0x00
        );
        midi.sendShortMsg(
            0x97 + channel,
            PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, true, false),
            value ? 0x7F : 0x00
        );
        midi.sendShortMsg(
            0x97 + channel,
            PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, true, true),
            value ? 0x7F : 0x00
        );
    }
};

PioneerDDJSB.beatloopLeds = function (value, group, control) {
    var channel = PioneerDDJSB.channelGroups[group],
        index,
        padNum,
        shifted;

    for (index in PioneerDDJSB.loopIntervals) {
        if (control === 'beatloop_' + PioneerDDJSB.loopIntervals[index] + '_enabled') {
            padNum = index % 4;
            shifted = (index >= 4);
            midi.sendShortMsg(
                0x97 + channel,
                PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.autoLoop, padNum, shifted, false),
                value ? 0x7F : 0x00
            );
        }
    }
};

PioneerDDJSB.beatlooprollLeds = function (value, group, control) {
    var channel = PioneerDDJSB.channelGroups[group],
        index;

    for (index in PioneerDDJSB.looprollIntervals) {
        if (control === 'beatlooproll_' + PioneerDDJSB.looprollIntervals[index] + '_activate') {
            midi.sendShortMsg(
                0x97 + channel,
                PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.autoLoop, index, false, true),
                value ? 0x7F : 0x00
            );
        }
    }
};

PioneerDDJSB.hotCueLeds = function (value, group, control) {
    var shiftedGroup = false,
        channel = PioneerDDJSB.channelGroups[group],
        padIndex = null,
        i;

    for (i = 1; i <= 8; i++) {
        if (control === 'hotcue_' + i + '_enabled') {
            padIndex = i - 1;
        }
    }

    if (padIndex >= 4) {
        shiftedGroup = true;
        padIndex %= 4;
    }

    // Pad LED without shift key
    midi.sendShortMsg(
        0x97 + channel,
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.hotCue, padIndex, false, shiftedGroup),
        value ? 0x7F : 0x00
    );

    // Pad LED with shift key
    midi.sendShortMsg(
        0x97 + channel,
        PioneerDDJSB.padLedTranslator(PioneerDDJSB.ledGroups.hotCue, padIndex, true, shiftedGroup),
        value ? 0x7F : 0x00
    );
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
                PioneerDDJSB.settings.jogResolution,
                PioneerDDJSB.settings.vinylSpeed,
                PioneerDDJSB.settings.alpha,
                PioneerDDJSB.settings.beta,
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
        midi.sendShortMsg(0x90 + channel, 0x4E, PioneerDDJSB.scratchMode[channel] ? 0x7F : 0x00);
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
    var deltaToPreviousToActive = PioneerDDJSB.fxPreviousKnobValues[deck][index] - PioneerDDJSB.fxParamsActiveValues[deck][index],
        deltaToActive = currentValue - PioneerDDJSB.fxParamsActiveValues[deck][index],
        passedOverStoredValue = (deltaToActive * deltaToPreviousToActive) < 0;

    print("new: " + currentValue + " previous: " + PioneerDDJSB.fxPreviousKnobValues[deck][index] + " active: " + PioneerDDJSB.fxParamsActiveValues[deck][index]);

    PioneerDDJSB.fxPreviousKnobValues[deck][index] = currentValue;

    if (passedOverStoredValue || Math.abs(deltaToActive) < 15) {
        print("yep, passedover is " + passedOverStoredValue + " deltaToActive is " + deltaToActive);

        PioneerDDJSB.fxParamsActiveValues[deck][index] = currentValue;
        return true;
    }
    print("nope");
    return false;
};
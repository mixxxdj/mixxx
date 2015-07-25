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

    PioneerDDJSB.ledGroups = {
        'hotCue': 0x00,
        'autoLoop': 0x10,
        'manualLoop': 0x20,
        'sampler': 0x30
    };

    PioneerDDJSB.loopIntervals = [1, 2, 4, 8, 16, 32, 64];

    PioneerDDJSB.looprollIntervals = [0.0625, 0.125, 0.25, 0.5];

    PioneerDDJSB.setFXSoftTakeover(true);

    PioneerDDJSB.BindControlConnections(false);
};

PioneerDDJSB.shutdown = function () {
    PioneerDDJSB.BindControlConnections(true);
};

PioneerDDJSB.setFXSoftTakeover = function (activate) {
    // Set softTakeover for controls effected by the effects knobs
    var deck,
        parameter;

    for (deck = 1; deck <= 2; deck++) {
        engine.softTakeover('[EffectRack1_EffectUnit' + deck + ']', 'mix', activate);
        for (parameter = 1; parameter <= 3; parameter++) {
            engine.softTakeover('[EffectRack1_EffectUnit' + deck + '_Effect1]', 'parameter' + parameter, activate);
        }
    }
};

PioneerDDJSB.BulkBindControlConnections = function (group, controlsToFunctions, remove) {
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

PioneerDDJSB.BindControlConnections = function (isUnbinding) {
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
        engine.connectControl(samplerGroup, 'duration', 'PioneerDDJSB.SamplerLeds', isUnbinding);
    }

    for (fxUnitIndex = 1; fxUnitIndex <= 2; fxUnitIndex++) {
        effectUnitGroup = '[EffectRack1_EffectUnit' + fxUnitIndex + ']';
        controlsToFunctions = {
            'group_[Headphone]_enable': 'PioneerDDJSB.FXLeds',
            'group_[Channel1]_enable': 'PioneerDDJSB.FXLeds',
            'group_[Channel2]_enable': 'PioneerDDJSB.FXLeds'
        };
        PioneerDDJSB.BulkBindControlConnections(effectUnitGroup, controlsToFunctions, isUnbinding);
    }

    for (channelIndex = 1; channelIndex <= 2; channelIndex++) {
        channelGroup = '[Channel' + channelIndex + ']';
        controlsToFunctions = {
            'play': 'PioneerDDJSB.PlayLeds',
            'cue_default': 'PioneerDDJSB.CueLeds',
            'pfl': 'PioneerDDJSB.HeadphoneCueLed',
            'keylock': 'PioneerDDJSB.KeyLockLeds',
            'slip_enabled': 'PioneerDDJSB.ToggleVinylLed',
            'quantize': 'PioneerDDJSB.ToggleQuantizeLed',
            'beat_active': 'PioneerDDJSB.ToggleBeatLed',
            'loop_in': 'PioneerDDJSB.ToggleLoopInLed',
            'loop_out': 'PioneerDDJSB.ToggleLoopOutLed',
            'filterLowKill': 'PioneerDDJSB.ToggleLowKillLed',
            'filterMidKill': 'PioneerDDJSB.ToggleMidKillLed',
            'filterHighKill': 'PioneerDDJSB.ToggleHighKillLed',
            'mute': 'PioneerDDJSB.ToggleMuteLed',
            'loop_enabled': 'PioneerDDJSB.ToggleLoopExitLed'
        };

        for (i = 1; i <= 8; i++) {
            controlsToFunctions['hotcue_' + i + '_enabled'] = 'PioneerDDJSB.HotCuePerformancePadLed';
        }

        for (index in PioneerDDJSB.loopIntervals) {
            controlsToFunctions['beatloop_' + PioneerDDJSB.loopIntervals[index] + '_enabled'] = 'PioneerDDJSB.BeatloopPerformancePadLed';
        }

        for (index in PioneerDDJSB.looprollIntervals) {
            controlsToFunctions['beatlooproll_' + PioneerDDJSB.looprollIntervals[index] + '_activate'] = 'PioneerDDJSB.ToggleBeatlooprollLeds';
        }

        PioneerDDJSB.BulkBindControlConnections(channelGroup, controlsToFunctions, isUnbinding);
    }
};

///////////////////////////////////////////////////////////////
//                         LED SECTION                       //
///////////////////////////////////////////////////////////////

// Returns the control code for a led on the pads.
// groupShift is true when the group has been selected while pressing shift
PioneerDDJSB.PadLedTranslator = function (ledGroup, ledNumber, shift, groupShift) {
    return (groupShift ? 0x40 : 0x00) + (shift ? 0x08 : 0x00) + ledGroup + (+ledNumber);
};


PioneerDDJSB.FXLeds = function (value, group, control) {
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

PioneerDDJSB.HeadphoneCueLed = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x54, value ? 0x7F : 0x00);
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x68, value ? 0x7F : 0x00);
};

PioneerDDJSB.CueLeds = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x0C, value ? 0x7F : 0x00);
};

PioneerDDJSB.KeyLockLeds = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x1A, value ? 0x7F : 0x00);
};

PioneerDDJSB.PlayLeds = function (value, group, control) {
    var channel = PioneerDDJSB.channelGroups[group];
    midi.sendShortMsg(0x90 + channel, 0x0B, value ? 0x7F : 0x00); // Play / Pause LED
    midi.sendShortMsg(0x90 + channel, 0x0C, value ? 0x7F : 0x00); // Cue LED
    midi.sendShortMsg(0x90 + channel, 0x47, value ? 0x7F : 0x00); // Play / Pause LED when shifted
    midi.sendShortMsg(0x90 + channel, 0x48, value ? 0x7F : 0x00); // Cue LED when shifted
};

PioneerDDJSB.ToggleVinylLed = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x17, value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleQuantizeLed = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x5C, value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleBeatLed = function (value, group, control) {
    midi.sendShortMsg(0x90 + PioneerDDJSB.channelGroups[group], 0x58, value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleLoopInLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 0, false, false),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.ToggleLoopOutLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 1, false, false),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.ToggleLoopExitLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 2, false, false),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.ToggleLowKillLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 0, false, true),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.ToggleMidKillLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 1, false, true),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.ToggleHighKillLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 2, false, true),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.ToggleMuteLed = function (value, group, control) {
    midi.sendShortMsg(
        0x97 + PioneerDDJSB.channelGroups[group],
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 3, false, true),
        value ? 0x7F : 0x00
    );
};

PioneerDDJSB.SamplerLeds = function (value, group, control) {
    var sampler = PioneerDDJSB.samplerGroups[group],
        channel;

    for (channel = 0; channel < 2; channel++) {
        midi.sendShortMsg(
            0x97 + channel,
            PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, false, false),
            value ? 0x7F : 0x00
        );
        midi.sendShortMsg(
            0x97 + channel,
            PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, false, true),
            value ? 0x7F : 0x00
        );
        midi.sendShortMsg(
            0x97 + channel,
            PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, true, false),
            value ? 0x7F : 0x00
        );
        midi.sendShortMsg(
            0x97 + channel,
            PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, true, true),
            value ? 0x7F : 0x00
        );
    }
};

PioneerDDJSB.BeatloopPerformancePadLed = function (value, group, control) {
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
                PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.autoLoop, padNum, shifted, false),
                value ? 0x7F : 0x00
            );
        }
    }
};

PioneerDDJSB.ToggleBeatlooprollLeds = function (value, group, control) {
    var channel = PioneerDDJSB.channelGroups[group],
        index;

    for (index in PioneerDDJSB.looprollIntervals) {
        if (control === 'beatlooproll_' + PioneerDDJSB.looprollIntervals[index] + '_activate') {
            midi.sendShortMsg(
                0x97 + channel,
                PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.autoLoop, index, false, true),
                value ? 0x7F : 0x00
            );
        }
    }
};

PioneerDDJSB.HotCuePerformancePadLed = function (value, group, control) {
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
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.hotCue, padIndex, false, shiftedGroup),
        value ? 0x7F : 0x00
    );

    // Pad LED with shift key
    midi.sendShortMsg(
        0x97 + channel,
        PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.hotCue, padIndex, true, shiftedGroup),
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

// Toggle scratching for a channel
PioneerDDJSB.toggleScratch = function (channel, isEnabled) {
    var deck = channel + 1;
    if (isEnabled) {
        engine.scratchEnable(
            deck,
            PioneerDDJSB.settings.jogResolution,
            PioneerDDJSB.settings.vinylSpeed,
            PioneerDDJSB.settings.alpha,
            PioneerDDJSB.settings.beta
        );
    } else {
        engine.scratchDisable(deck);
    }
};

// Detect when the user touches and releases the jog-wheel while 
// jog-mode is set to vinyl to enable and disable scratching.
PioneerDDJSB.jogScratchTouch = function (channel, control, value, status, group) {
    /*var deck = channel + 1;

     if (!engine.getValue(group, 'play'))
     {
     PioneerDDJSB.toggleScratch(channel, value == 0x7F);
     }
     else
     {
     var activate = value > 0;

     if (activate)
     {
     engine.brake(deck, true, 1, 1); // enable brake effect
     PioneerDDJSB.toggleScratch(channel, true);
     }
     else
     {
     engine.brake(deck, false, 1, 1); // disable brake effect
     PioneerDDJSB.toggleScratch(channel, false);
     }
     }*/
};

// Scratch or seek with the jog-wheel.
PioneerDDJSB.jogScratchTurn = function (channel, control, value, status) {
    PioneerDDJSB.jogSeekTurn(channel, control, value, status);
    //var deck = channel + 1;

    // Only scratch if we're in scratching mode, when 
    // user is touching the top of the jog-wheel.
    //if (engine.isScratching(deck)) 
    //{
    // engine.scratchTick(deck, PioneerDDJSB.getJogWheelDelta(value));
    //}
};

// Pitch bend using the jog-wheel, or finish a scratch when the wheel 
// is still turning after having released it.
PioneerDDJSB.jogPitchBend = function (channel, control, value, status, group) {
    //if (!engine.getValue(group, 'play'))
    //return;

    var deck = channel + 1;
    PioneerDDJSB.pitchBend(channel, PioneerDDJSB.getJogWheelDelta(value));
};

// Pitch bend a channel
PioneerDDJSB.pitchBend = function (channel, movement) {
    var deck = channel + 1, group = '[Channel' + deck + ']';

    // Make this a little less sensitive.
    movement /= 5;

    engine.setValue(group, 'jog', movement);
};

// Called when the jog-mode is not set to vinyl, and the jog wheel is touched.
// If we are not playing we want to seek through it and this is done in scratch mode
PioneerDDJSB.jogSeekTouch = function (channel, control, value, status, group) {
    // if (engine.getValue(group, 'play'))
    // return;

    // var deck = channel + 1;
    // PioneerDDJSB.toggleScratch(channel, value == 0x7F);
};

// Call when the jog-wheel is turned. The related jogSeekTouch function 
// sets up whether we will be scratching or pitch-bending depending 
// on whether a song is playing or not.
PioneerDDJSB.jogSeekTurn = function (channel, control, value, status, group) {
    //PioneerDDJSB.toggleScratch(channel, value == 0x7F);
    //if (engine.getValue(group, 'play'))
    // return;

    var deck = channel + 1;
    engine.setValue('[Channel' + deck + ']', 'jog', PioneerDDJSB.getJogWheelDelta(value) / 3);
};

PioneerDDJSB.jogFastSeekTurn = function (channel, control, value, status, group) {
    //PioneerDDJSB.toggleScratch(channel, value == 0x7F);
    //if (engine.getValue(group, 'play'))
    // return;

    var deck = channel + 1;
    engine.setValue('[Channel' + deck + ']', 'jog', PioneerDDJSB.getJogWheelDelta(value) * 20);
};


///////////////////////////////////////////////////////////////
//                    ROTARY SELECTOR SECTION                //
///////////////////////////////////////////////////////////////
// Handles the rotary selector for choosing tracks, library items, crates, etc.

PioneerDDJSB.CalculateRotaryDelta = function (value) {
    var delta = 0x40 - Math.abs(0x40 - value),
        isCounterClockwise = value > 0x40;

    if (isCounterClockwise) {
        delta *= -1;
    }
    return delta;
};

PioneerDDJSB.RotarySelector = function (channel, control, value, status) {
    var delta = PioneerDDJSB.CalculateRotaryDelta(value);
    engine.setValue('[Playlist]', 'SelectTrackKnob', delta);
};

PioneerDDJSB.ShiftedRotarySelector = function (channel, control, value, status) {
    var delta = PioneerDDJSB.CalculateRotaryDelta(value),
        f = (delta > 0 ? 'SelectNextPlaylist' : 'SelectPrevPlaylist');

    engine.setValue('[Playlist]', f, Math.abs(delta));
};

PioneerDDJSB.RotarySelectorShiftedClick = function (channel, control, value, status) {
    if (value === 0x7F) {
        engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1);
    }
};

PioneerDDJSB.FXButton = function (channel, control, value, status) {
    var deck = channel - 4,
        button = control - 0x47;

    PioneerDDJSB.fxButtonPressed[deck][button] = (value === 0x7F);

    if (button < 2) {
        engine.trigger('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'group_[Channel' + (button + 1) + ']_enable');

    } else {
        engine.trigger('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'group_[Headphone]_enable');
    }
};

PioneerDDJSB.FXKnob = function (channel, control, value, status) {
    var deck = channel - 4,
        anyButtonPressed = false,
        i;

    for (i = 0; i < 3; i++) {
        if (PioneerDDJSB.fxButtonPressed[deck][i]) {
            anyButtonPressed = true;
        }
    }

    if (!anyButtonPressed) {
        engine.setValue('[EffectRack1_EffectUnit' + (deck + 1) + ']', 'mix', value / 0x7F);
    } else {
        for (i = 0; i < 3; i++) {
            if (PioneerDDJSB.fxButtonPressed[deck][i]) {
                engine.setParameter(
                    '[EffectRack1_EffectUnit' + (deck + 1) + '_Effect1]',
                    'parameter' + (i + 1),
                    value / 0x7F
                );
            }
        }
    }
};

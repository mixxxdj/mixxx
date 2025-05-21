///////////////////////////////////////////////////////////////////////////////////
// JSHint configuration                                                          //
///////////////////////////////////////////////////////////////////////////////////
/* jshint -W016                                                                  */
///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* Traktor Kontrol S2 MK3 HID controller script v1.01                            */
/* Last modification: February 2021                                              */
/* Author: Michael Schmidt                                                       */
/* https://github.com/mixxxdj/mixxx/wiki/Native%20Instruments%20Traktor%20Kontrol%20S2%20MK3 */
/*                                                                               */
///////////////////////////////////////////////////////////////////////////////////

var TraktorS2MK3 = new function() {
    this.controller = new HIDController();
    this.shiftPressed = {"[Channel1]": false, "[Channel2]": false};
    this.fxButtonState = {1: false, 2: false, 3: false, 4: false};
    this.padModeState = {"[Channel1]": 0, "[Channel2]": 0}; // 0 = Hotcues Mode, 1 = Samples Mode

    // Knob encoder states (hold values between 0x0 and 0xF)
    // Rotate to the right is +1 and to the left is means -1
    this.browseKnobEncoderState = {"[Channel1]": 0, "[Channel2]": 0};
    this.loopKnobEncoderState = {"[Channel1]": 0, "[Channel2]": 0};
    this.moveKnobEncoderState = {"[Channel1]": 0, "[Channel2]": 0};

    // Microphone button
    this.microphonePressedTimer = 0; // Timer to distinguish between short and long press

    // Sync buttons
    this.syncPressedTimer = {"[Channel1]": 0, "[Channel2]": 0}; // Timer to distinguish between short and long press

    // Jog wheels
    this.pitchBendMultiplier = 1.1;
    this.lastTickVal = [0, 0];
    this.lastTickTime = [0.0, 0.0];

    // VuMeter
    this.vuLeftConnection = {};
    this.vuRightConnection = {};
    this.clipLeftConnection = {};
    this.clipRightConnection = {};
    this.vuMeterThresholds = {"vu-18": (1 / 6), "vu-12": (2 / 6), "vu-6": (3 / 6), "vu0": (4 / 6), "vu6": (5 / 6)};

    // Sampler callbacks
    this.samplerCount = 16;
    this.samplerCallbacks = [];
    this.samplerHotcuesRelation = {
        "[Channel1]": {
            1: 1, 2: 2, 3: 3, 4: 4, 5: 9, 6: 10, 7: 11, 8: 12
        }, "[Channel2]": {
            1: 5, 2: 6, 3: 7, 4: 8, 5: 13, 6: 14, 7: 15, 8: 16
        }
    };
};

TraktorS2MK3.init = function(_id) {
    if (engine.getValue("[App]", "num_samplers") < TraktorS2MK3.samplerCount) {
        engine.setValue("[App]", "num_samplers", TraktorS2MK3.samplerCount);
    }
    TraktorS2MK3.registerInputPackets();
    TraktorS2MK3.registerOutputPackets();
    HIDDebug("TraktorS2MK3: Init done!");
};

TraktorS2MK3.registerInputPackets = function() {
    const messageShort = new HIDPacket("shortmessage", 0x01, this.messageCallback);
    const messageLong = new HIDPacket("longmessage", 0x02, this.messageCallback);

    this.registerInputButton(messageShort, "[Channel1]", "!play", 0x02, 0x08, this.playHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!play", 0x05, 0x20, this.playHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!cue_default", 0x02, 0x04, this.cueHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!cue_default", 0x05, 0x10, this.cueHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!shift", 0x01, 0x20, this.shiftHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!shift", 0x04, 0x80, this.shiftHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!sync", 0x02, 0x01, this.syncHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!sync", 0x05, 0x04, this.syncHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!keylock", 0x02, 0x02, this.keylockHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!keylock", 0x05, 0x08, this.keylockHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!hotcues", 0x01, 0x40, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!hotcues", 0x05, 0x01, this.padModeHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!samples", 0x01, 0x80, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!samples", 0x05, 0x02, this.padModeHandler);

    // Number pad buttons (Hotcues or Samplers depending on current mode)
    this.registerInputButton(messageShort, "[Channel1]", "!pad_1", 0x02, 0x10, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_2", 0x02, 0x20, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_3", 0x02, 0x40, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_4", 0x02, 0x80, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_5", 0x03, 0x01, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_6", 0x03, 0x02, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_7", 0x03, 0x04, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_8", 0x03, 0x08, this.numberButtonHandler);

    this.registerInputButton(messageShort, "[Channel2]", "!pad_1", 0x05, 0x40, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_2", 0x05, 0x80, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_3", 0x06, 0x01, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_4", 0x06, 0x02, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_5", 0x06, 0x04, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_6", 0x06, 0x08, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_7", 0x06, 0x10, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_8", 0x06, 0x20, this.numberButtonHandler);

    // Headphone buttons
    this.registerInputButton(messageShort, "[Channel1]", "!pfl", 0x04, 0x01, this.headphoneHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pfl", 0x04, 0x02, this.headphoneHandler);

    // Track browsing
    this.registerInputButton(messageShort, "[Channel1]", "!SelectTrack", 0x09, 0x0F, this.selectTrackHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!SelectTrack", 0x0A, 0xF0, this.selectTrackHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!LoadSelectedTrack", 0x07, 0x01, this.loadTrackHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!LoadSelectedTrack", 0x07, 0x08, this.loadTrackHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!MaximizeLibrary", 0x01, 0x08, this.maximizeLibraryHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!MaximizeLibrary", 0x04, 0x20, this.maximizeLibraryHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!AddTrack", 0x01, 0x04, this.addTrackHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!AddTrack", 0x04, 0x10, this.addTrackHandler);

    // Loop control
    this.registerInputButton(messageShort, "[Channel1]", "!SelectLoop", 0x0A, 0x0F, this.selectLoopHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!SelectLoop", 0x0B, 0xF0, this.selectLoopHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!ActivateLoop", 0x07, 0x04, this.activateLoopHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!ActivateLoop", 0x07, 0x20, this.activateLoopHandler);

    // Beatjump
    this.registerInputButton(messageShort, "[Channel1]", "!SelectBeatjump", 0x09, 0xF0, this.selectBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!SelectBeatjump", 0x0B, 0x0F, this.selectBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!ActivateBeatjump", 0x07, 0x02, this.activateBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!ActivateBeatjump", 0x07, 0x10, this.activateBeatjumpHandler);

    // There is only one button on the controller, we use to toggle quantization for all channels
    this.registerInputButton(messageShort, "[ChannelX]", "!quantize", 0x06, 0x40, this.quantizeHandler);

    // Microphone
    this.registerInputButton(messageShort, "[Microphone]", "!talkover", 0x06, 0x80, this.microphoneHandler);

    // Jog wheels
    this.registerInputButton(messageShort, "[Channel1]", "!jog_touch", 0x08, 0x40, this.jogTouchHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!jog_touch", 0x08, 0x80, this.jogTouchHandler);
    this.registerInputJog(messageShort, "[Channel1]", "!jog", 0x0C, 0xFFFFFFFF, this.jogHandler);
    this.registerInputJog(messageShort, "[Channel2]", "!jog", 0x10, 0xFFFFFFFF, this.jogHandler);

    // FX Buttons
    this.registerInputButton(messageShort, "[ChannelX]", "!fx1", 0x03, 0x10, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx2", 0x03, 0x20, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx3", 0x03, 0x40, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx4", 0x03, 0x80, this.fxHandler);

    // Rev / FLUX / GRID
    this.registerInputButton(messageShort, "[Channel1]", "!reverse", 0x01, 0x01, this.reverseHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!reverse", 0x04, 0x04, this.reverseHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!slip_enabled", 0x01, 0x02, this.fluxHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!slip_enabled", 0x04, 0x08, this.fluxHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!grid", 0x01, 0x10, this.beatgridHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!grid", 0x04, 0x40, this.beatgridHandler);

    this.controller.registerInputPacket(messageShort);

    this.registerInputScaler(messageLong, "[Channel1]", "rate", 0x01, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "rate", 0x09, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Channel1]", "volume", 0x03, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "volume", 0x07, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Channel1]", "pregain", 0x0B, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "pregain", 0x1D, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x0D, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x0F, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x11, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x1F, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x21, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x23, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel1]]", "super1", 0x13, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel2]]", "super1", 0x25, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Master]", "crossfader", 0x05, 0xFFFF, this.parameterHandler);
    /*
    do NOT map the "master" button because it also drives the analog output gain.
    Disabling this mapping is the only way to have independent controls for the
    digital master gain and the output level - the latter usually needs to be set
    at 100%.
    */
    // this.registerInputScaler(messageLong, "[Master]", "gain", 0x15, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Sampler]", "pregain", 0x17, 0xFFFF, this.samplerPregainHandler);
    this.registerInputScaler(messageLong, "[Master]", "headMix", 0x19, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headGain", 0x1B, 0xFFFF, this.parameterHandler);

    this.controller.registerInputPacket(messageLong);

    // Soft takeover for all knobs
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);

    engine.softTakeover("[Channel1]", "volume", true);
    engine.softTakeover("[Channel2]", "volume", true);

    engine.softTakeover("[Channel1]", "pregain", true);
    engine.softTakeover("[Channel2]", "pregain", true);

    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);

    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]", "parameter1", true);

    engine.softTakeover("[QuickEffectRack1_[Channel1]]", "super1", true);
    engine.softTakeover("[QuickEffectRack1_[Channel2]]", "super1", true);

    engine.softTakeover("[Master]", "crossfader", true);
    // see the above comment on master gain
    //engine.softTakeover("[Master]", "gain", true);
    engine.softTakeover("[Master]", "headMix", true);
    engine.softTakeover("[Master]", "headGain", true);
    for (let i = 1; i <= TraktorS2MK3.samplerCount; ++i) {
        engine.softTakeover("[Sampler" + i + "]", "pregain", true);
    }

    // Dirty hack to set initial values in the packet parser
    const data = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    TraktorS2MK3.incomingData(data);
};

TraktorS2MK3.registerInputJog = function(message, group, name, offset, bitmask, callback) {
    // Jog wheels have 4 byte input
    message.addControl(group, name, offset, "I", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS2MK3.registerInputScaler = function(message, group, name, offset, bitmask, callback) {
    message.addControl(group, name, offset, "H", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS2MK3.registerInputButton = function(message, group, name, offset, bitmask, callback) {
    message.addControl(group, name, offset, "B", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS2MK3.playHandler = function(field) {
    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue(field.group, "start_stop", field.value);
    } else if (field.value === 1) {
        script.toggleControl(field.group, "play");
    }
};

TraktorS2MK3.cueHandler = function(field) {
    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue(field.group, "cue_gotoandstop", field.value);
    } else {
        engine.setValue(field.group, "cue_default", field.value);
    }
};

TraktorS2MK3.shiftHandler = function(field) {
    TraktorS2MK3.shiftPressed[field.group] = field.value;
    engine.setValue("[Controls]", "touch_shift", field.value);
    TraktorS2MK3.outputHandler(field.value, field.group, "shift");
};

TraktorS2MK3.keylockHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "keylock");
};

TraktorS2MK3.syncHandler = function(field) {
    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue(field.group, "beatsync_phase", field.value);
        // Light LED while pressed
        TraktorS2MK3.outputHandler(field.value, field.group, "sync_enabled");
    } else {
        if (field.value) {
            if (engine.getValue(field.group, "sync_enabled") === 0) {
                script.triggerControl(field.group, "beatsync");
                // Start timer to measure how long button is pressed
                TraktorS2MK3.syncPressedTimer[field.group] = engine.beginTimer(300, () => {
                    engine.setValue(field.group, "sync_enabled", 1);
                    // Reset sync button timer state if active
                    if (TraktorS2MK3.syncPressedTimer[field.group] !== 0) {
                        TraktorS2MK3.syncPressedTimer[field.group] = 0;
                    }
                }, true);

                // Light corresponding LED when button is pressed
                TraktorS2MK3.outputHandler(1, field.group, "sync_enabled");
            } else {
                // Deactivate sync lock
                // LED is turned off by the callback handler for sync_enabled
                engine.setValue(field.group, "sync_enabled", 0);
            }
        } else {
            if (TraktorS2MK3.syncPressedTimer[field.group] !== 0) {
                // Timer still running -> stop it and unlight LED
                engine.stopTimer(TraktorS2MK3.syncPressedTimer[field.group]);
                TraktorS2MK3.outputHandler(0, field.group, "sync_enabled");
            }
        }
    }
};

TraktorS2MK3.padModeHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    if (TraktorS2MK3.padModeState[field.group] === 0 && field.name === "!samples") {
        // If we are in hotcues mode and samples mode is activated
        engine.setValue("[Samplers]", "show_samplers", 1);
        TraktorS2MK3.padModeState[field.group] = 1;
        TraktorS2MK3.outputHandler(0, field.group, "hotcues");
        TraktorS2MK3.outputHandler(1, field.group, "samples");

        // Light LEDs for all slots with loaded samplers
        for (const key in TraktorS2MK3.samplerHotcuesRelation[field.group]) {
            if (Object.hasOwnProperty.call(TraktorS2MK3.samplerHotcuesRelation[field.group], key)) {
                const loaded = engine.getValue("[Sampler" + TraktorS2MK3.samplerHotcuesRelation[field.group][key] + "]", "track_loaded");
                TraktorS2MK3.outputHandler(loaded, field.group, "pad_" + key);
            }
        }
    } else if (field.name === "!hotcues") {
        // If we are in samples mode and hotcues mode is activated
        TraktorS2MK3.padModeState[field.group] = 0;
        TraktorS2MK3.outputHandler(1, field.group, "hotcues");
        TraktorS2MK3.outputHandler(0, field.group, "samples");

        // Light LEDs for all enabled hotcues
        for (let i = 1; i <= 8; ++i) {
            const active = engine.getValue(field.group, "hotcue_" + i + "_enabled");
            if (active) {
                const color = engine.getValue(field.group, "hotcue_" + i + "_color");
                const colorValue = TraktorS2MK3.PadColorMap.getValueForNearestColor(color);
                TraktorS2MK3.outputHandler(colorValue, field.group, "pad_" + i);
            }
            else {
                TraktorS2MK3.outputHandler(0, field.group, `pad_${  i}`);
            }
        }
    }
};

TraktorS2MK3.PadColorMap = new ColorMapper({
    0x755771: 0x00,
    0xE9C9DA: 0x01,
    0x372C2B: 0x02,
    0x2B2B21: 0x03,
    0xAF262E: 0x04,
    0xCB242C: 0x05,
    0xFF2916: 0x06,
    0xFE354F: 0x07,
    0xB8332D: 0x08,
    0xD1322C: 0x09,
    0xFD5921: 0x0A,
    0xF95364: 0x0B,
    0xA53A2E: 0x0C,
    0xB03E2B: 0x0D,
    0xFD7BAB: 0x0E,
    0xFB8553: 0x0F,
    0xBE712F: 0x10,
    0xC57831: 0x11,
    0xF8AB36: 0x12,
    0xF6AA80: 0x13,
    0x9C7132: 0x14,
    0xB07B31: 0x15,
    0xE9BA3D: 0x16,
    0xECBC92: 0x17,
    0x626341: 0x18,
    0x6D6C43: 0x19,
    0xB6B959: 0x1A,
    0xA9BB91: 0x1B,
    0x2E5C45: 0x1C,
    0x2B7446: 0x1D,
    0x2CCF68: 0x1E,
    0x92C7B0: 0x1F,
    0x409654: 0x20,
    0x39A97F: 0x21,
    0x3FD2B0: 0x22,
    0xB0C6C9: 0x23,
    0x2C8FE4: 0x24,
    0x7F97DE: 0x25,
    0x0DE1FF: 0x26,
    0xB3D5FF: 0x27,
    0x059BF3: 0x28,
    0x6B97EF: 0x29,
    0x00C3FF: 0x2A,
    0x63D3FF: 0x2B,
    0x1860DD: 0x2C,
    0x0482F8: 0x2D,
    0x00A1FF: 0x2E,
    0x42B3FF: 0x2F,
    0x8D52DD: 0x30,
    0xB171EA: 0x31,
    0xE298FE: 0x32,
    0xCEB4FD: 0x33,
    0xEB4BD3: 0x34,
    0xEB5DDB: 0x35,
    0xF263E3: 0x36,
    0xF08CEA: 0x37,
    0xC33196: 0x38,
    0xC944A1: 0x39,
    0xFC4DC0: 0x3A,
    0xFA6DDC: 0x3B,
    0xA82C6B: 0x3C,
    0xB13A74: 0x3D,
    0xFB359F: 0x3E,
    0xF94EAE: 0x3F,
    0xD12A40: 0x40,
    0xDA394F: 0x41,
    0xFB3672: 0x42,
    0xF8487E: 0x43,
    0x6D5A77: 0x44,
    0x836295: 0x45,
    0xE5BADA: 0x46,
    0xE1BCE3: 0x47,
    0x7F6487: 0x48,
    0x916CA0: 0x49,
    0xDCB6D4: 0x4A,
    0xD6B3DB: 0x4B,
    0x795F82: 0x4C,
    0x8E689E: 0x4D,
    0xDDB9D4: 0x4E,
    0xD6B7D9: 0x4F
});

TraktorS2MK3.numberButtonHandler = function(field) {
    const padNumber = parseInt(field.id[field.id.length - 1]);
    if (TraktorS2MK3.padModeState[field.group] === 0) {
        // Hotcues mode
        if (TraktorS2MK3.shiftPressed[field.group]) {
            engine.setValue(field.group, "hotcue_" + padNumber + "_clear", field.value);
        } else {
            engine.setValue(field.group, "hotcue_" + padNumber + "_activate", field.value);
        }
    } else {
        // Samples mode
        const sampler = TraktorS2MK3.samplerHotcuesRelation[field.group][padNumber];
        if (TraktorS2MK3.shiftPressed[field.group]) {
            const playing = engine.getValue("[Sampler" + sampler + "]", "play");
            if (playing) {
                engine.setValue("[Sampler" + sampler + "]", "cue_default", field.value);
            } else {
                engine.setValue("[Sampler" + sampler + "]", "eject", field.value);
            }
        } else {
            const loaded = engine.getValue("[Sampler" + sampler + "]", "track_loaded");
            if (loaded) {
                engine.setValue("[Sampler" + sampler + "]", "cue_gotoandplay", field.value);
            } else {
                engine.setValue("[Sampler" + sampler + "]", "LoadSelectedTrack", field.value);
            }
        }
    }
};

TraktorS2MK3.headphoneHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "pfl");
};

TraktorS2MK3.selectTrackHandler = function(field) {
    let delta = 1;
    if ((field.value + 1) % 16 === TraktorS2MK3.browseKnobEncoderState[field.group]) {
        delta = -1;
    }

    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue("[Library]", "MoveHorizontal", delta);
    } else {
        engine.setValue("[Library]", "MoveVertical", delta);
    }

    TraktorS2MK3.browseKnobEncoderState[field.group] = field.value;
};

TraktorS2MK3.loadTrackHandler = function(field) {
    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue(field.group, "eject", field.value);
    } else {
        engine.setValue(field.group, "LoadSelectedTrack", field.value);
    }
};

TraktorS2MK3.addTrackHandler = function(field) {
    TraktorS2MK3.outputHandler(field.value, field.group, "addTrack");

    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue("[Library]", "AutoDjAddTop", field.value);
    } else {
        engine.setValue("[Library]", "AutoDjAddBottom", field.value);
    }
};

TraktorS2MK3.maximizeLibraryHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl("[Skin]", "show_maximized_library");
};

TraktorS2MK3.selectLoopHandler = function(field) {
    if ((field.value + 1) % 16 === TraktorS2MK3.loopKnobEncoderState[field.group]) {
        script.triggerControl(field.group, "loop_halve");
    } else {
        script.triggerControl(field.group, "loop_double");
    }

    TraktorS2MK3.loopKnobEncoderState[field.group] = field.value;
};

TraktorS2MK3.activateLoopHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue(field.group, "reloop_toggle", field.value);
    } else {
        engine.setValue(field.group, "beatloop_activate", field.value);
    }
};

TraktorS2MK3.selectBeatjumpHandler = function(field) {
    let delta = 1;
    if ((field.value + 1) % 16 === TraktorS2MK3.moveKnobEncoderState[field.group]) {
        delta = -1;
    }

    if (TraktorS2MK3.shiftPressed[field.group]) {
        const beatjumpSize = engine.getValue(field.group, "beatjump_size");
        if (delta > 0) {
            engine.setValue(field.group, "beatjump_size", beatjumpSize * 2);
        } else {
            engine.setValue(field.group, "beatjump_size", beatjumpSize / 2);
        }
    } else {
        if (delta < 0) {
            script.triggerControl(field.group, "beatjump_backward");
        } else {
            script.triggerControl(field.group, "beatjump_forward");
        }
    }

    TraktorS2MK3.moveKnobEncoderState[field.group] = field.value;
};

TraktorS2MK3.activateBeatjumpHandler = function(field) {
    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue(field.group, "reloop_andstop", field.value);
    } else {
        engine.setValue(field.group, "beatlooproll_activate", field.value);
    }
};

TraktorS2MK3.quantizeHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    const res = !(engine.getValue("[Channel1]", "quantize") && engine.getValue("[Channel2]", "quantize"));
    engine.setValue("[Channel1]", "quantize", res);
    engine.setValue("[Channel2]", "quantize", res);
    TraktorS2MK3.outputHandler(res, field.group, "quantize");
};

TraktorS2MK3.microphoneHandler = function(field) {
    if (field.value) {
        if (TraktorS2MK3.microphonePressedTimer === 0) {
            // Start timer to measure how long button is pressed
            TraktorS2MK3.microphonePressedTimer = engine.beginTimer(300, () => {
                // Reset microphone button timer status if active
                if (TraktorS2MK3.microphonePressedTimer !== 0) {
                    TraktorS2MK3.microphonePressedTimer = 0;
                }
            }, true);
        }

        script.toggleControl("[Microphone]", "talkover");
    } else {
        // Button is released, check if timer is still running
        if (TraktorS2MK3.microphonePressedTimer !== 0) {
            // short click -> permanent activation
            TraktorS2MK3.microphonePressedTimer = 0;
        } else {
            engine.setValue("[Microphone]", "talkover", 0);
        }
    }
};

TraktorS2MK3.parameterHandler = function(field) {
    engine.setParameter(field.group, field.name, field.value / 4095);
};

TraktorS2MK3.samplerPregainHandler = function(field) {
    // Map sampler gain knob of all sampler together.
    // Dirty hack, but the best we can do for now.
    for (let i = 1; i <= TraktorS2MK3.samplerCount; ++i) {
        engine.setParameter("[Sampler" + i + "]", field.name, field.value / 4095);
    }
};

TraktorS2MK3.jogTouchHandler = function(field) {
    const deckNumber = TraktorS2MK3.controller.resolveDeck(field.group);
    if (field.value > 0) {
        engine.scratchEnable(deckNumber, 1024, 33 + 1 / 3, 0.125, 0.125 / 8, true);
    } else {
        engine.scratchDisable(deckNumber);
    }
};

TraktorS2MK3.jogHandler = function(field) {
    const deckNumber = TraktorS2MK3.controller.resolveDeck(field.group);
    const deltas = TraktorS2MK3.wheelDeltas(deckNumber, field.value);
    const tickDelta = deltas[0];
    const timeDelta = deltas[1];

    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, tickDelta);
    } else {
        const velocity = (tickDelta / timeDelta) * TraktorS2MK3.pitchBendMultiplier;
        engine.setValue(field.group, "jog", velocity);
    }
};

TraktorS2MK3.wheelDeltas = function(deckNumber, value) {
    // When the wheel is touched, four bytes change, but only the first behaves predictably.
    // It looks like the wheel is 1024 ticks per revolution.
    const tickval = value & 0xFF;
    let timeval = value >>> 16;
    let prevTick = 0;
    let prevTime = 0;

    // Group 1 and 2 -> Array index 0 and 1
    prevTick = this.lastTickVal[deckNumber - 1];
    prevTime = this.lastTickTime[deckNumber - 1];
    this.lastTickVal[deckNumber - 1] = tickval;
    this.lastTickTime[deckNumber - 1] = timeval;

    if (prevTime > timeval) {
        // We looped around.  Adjust current time so that subtraction works.
        timeval += 0x10000;
    }
    let timeDelta = timeval - prevTime;
    if (timeDelta === 0) {
        // Spinning too fast to detect speed!  By not dividing we are guessing it took 1ms.
        timeDelta = 1;
    }

    let tickDelta = 0;
    if (prevTick >= 200 && tickval <= 100) {
        tickDelta = tickval + 256 - prevTick;
    } else if (prevTick <= 100 && tickval >= 200) {
        tickDelta = tickval - prevTick - 256;
    } else {
        tickDelta = tickval - prevTick;
    }

    return [tickDelta, timeDelta];
};

TraktorS2MK3.fxHandler = function(field) {
    /* We support 8 effects in total by having 2 effects per fx button. *
     * First press of the button will load the preset at the index of the quick effect preset list *
     * Second press will load the preset index + 4 *
     * Arrange your quick effect preset list from 1 to 8 like this: 1/5, 2/6, 3/7, 4/8 */
    if (field.value === 0) {
        return;
    }

    const availableGroups = ["[Channel1]", "[Channel2]"];
    const fxButtonCount = 4;
    let targetGroups = [];
    const fxNumber = parseInt(field.id[field.id.length - 1]);
    let noShift = false;

    // Target decks whose shift button is pressed.
    for (const group of availableGroups) {
        if (TraktorS2MK3.shiftPressed[group]) { targetGroups.push(group); }
    }

    // If no shift button was pressed fall back to target all decks.
    if (targetGroups.length === 0) {
        targetGroups = availableGroups;
        noShift = true;
    }

    const fxToApply = {};
    const activeFx = {};
    // Detect which fx should be enabled
    for (const group of targetGroups) {
        activeFx[group] = engine.getValue(`[QuickEffectRack1_${group}]`, "loaded_chain_preset") - 1;
        if (activeFx[group] === fxNumber) {
            // Pressing again the fx button
            fxToApply[group] = fxNumber + fxButtonCount;
        } else if (activeFx[group] === fxNumber + fxButtonCount) {
            // Already on secondary fx so resetting back to fxNumber
            fxToApply[group] = fxNumber;
        } else {
            // First press of a different fx button
            fxToApply[group] = fxNumber;
        }
    }

    /* When we target both decks - No shift pressed - we want *
     * to reset fx to same value for both */
    if (targetGroups.length === 2 && noShift) {
        const [deck1, deck2] = targetGroups;
        const activeFxDeck1 = activeFx[deck1];
        const activeFxDeck2 = activeFx[deck2];
        const fxToApplyDeck1 = fxToApply[deck1];
        const fxToApplyDeck2 = fxToApply[deck2];

        // If any of deck has already fx enabled but different value to apply
        if ((activeFxDeck1 === fxNumber || activeFxDeck2 === fxNumber) && fxToApplyDeck1 !== fxToApplyDeck2) {
        // find lower value to reset both to the same one
            const fxToApplyAll = Math.min(fxToApplyDeck1, fxToApplyDeck2);
            fxToApply[deck1] = fxToApplyAll;
            fxToApply[deck2] = fxToApplyAll;
        }
    }

    // Now apply the new fx value
    for (const group of targetGroups) {
        engine.setValue(`[QuickEffectRack1_${group}]`, "loaded_chain_preset", fxToApply[group] + 1);
    }
};

TraktorS2MK3.fxOutputHandler = function() {
    const fxButtonCount = 4;
    const availableGroups = ["[Channel1]", "[Channel2]"];

    const activeFx = {};
    for (const group of availableGroups) {
        activeFx[group] = engine.getValue(`[QuickEffectRack1_${group}]`, "loaded_chain_preset") - 1;
        // We want to lit the proper fx button even when we have applied a higher fxNumber
        if (activeFx[group] > fxButtonCount) { activeFx[group] = activeFx[group] - fxButtonCount; }
    }

    /* There is no way on the controller to indicate which deck the effect applies *
     * to, but keeping both lit indicates that different effects are in use.       */
    for (let fxButton = 1; fxButton <= fxButtonCount; ++fxButton) {
        let active = false;
        for (const group of availableGroups) {
            active = active || activeFx[group] === fxButton;
        }
        TraktorS2MK3.outputHandler(active, "[ChannelX]", "fxButton" + fxButton);
    }
};

TraktorS2MK3.reverseHandler = function(field) {
    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue(field.group, "reverseroll", field.value);
    } else {
        engine.setValue(field.group, "reverse", field.value);
    }

    TraktorS2MK3.outputHandler(field.value, field.group, "reverse");
};

TraktorS2MK3.fluxHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "slip_enabled");
};

TraktorS2MK3.beatgridHandler = function(field) {
    if (TraktorS2MK3.shiftPressed[field.group]) {
        engine.setValue(field.group, "beats_translate_match_alignment", field.value);
    } else {
        engine.setValue(field.group, "beats_translate_curpos", field.value);
    }

    TraktorS2MK3.outputHandler(field.value, field.group, "grid");
};

TraktorS2MK3.registerOutputPackets = function() {
    const output = new HIDPacket("output", 0x80);

    output.addOutput("[Channel1]", "play_indicator", 0x0C, "B");
    output.addOutput("[Channel2]", "play_indicator", 0x33, "B");

    output.addOutput("[Channel1]", "cue_indicator", 0x0B, "B");
    output.addOutput("[Channel2]", "cue_indicator", 0x32, "B");

    output.addOutput("[Channel1]", "shift", 0x06, "B");
    output.addOutput("[Channel2]", "shift", 0x2D, "B");

    output.addOutput("[Channel1]", "hotcues", 0x07, "B");
    output.addOutput("[Channel2]", "hotcues", 0x2E, "B");

    output.addOutput("[Channel1]", "samples", 0x08, "B");
    output.addOutput("[Channel2]", "samples", 0x2F, "B");

    output.addOutput("[Channel1]", "sync_enabled", 0x09, "B");
    output.addOutput("[Channel2]", "sync_enabled", 0x30, "B");

    output.addOutput("[Channel1]", "keylock", 0x0A, "B");
    output.addOutput("[Channel2]", "keylock", 0x31, "B");

    output.addOutput("[Channel1]", "pad_1", 0x0D, "B");
    output.addOutput("[Channel1]", "pad_2", 0x0E, "B");
    output.addOutput("[Channel1]", "pad_3", 0x0F, "B");
    output.addOutput("[Channel1]", "pad_4", 0x10, "B");
    output.addOutput("[Channel1]", "pad_5", 0x11, "B");
    output.addOutput("[Channel1]", "pad_6", 0x12, "B");
    output.addOutput("[Channel1]", "pad_7", 0x13, "B");
    output.addOutput("[Channel1]", "pad_8", 0x14, "B");

    output.addOutput("[Channel2]", "pad_1", 0x34, "B");
    output.addOutput("[Channel2]", "pad_2", 0x35, "B");
    output.addOutput("[Channel2]", "pad_3", 0x36, "B");
    output.addOutput("[Channel2]", "pad_4", 0x37, "B");
    output.addOutput("[Channel2]", "pad_5", 0x38, "B");
    output.addOutput("[Channel2]", "pad_6", 0x39, "B");
    output.addOutput("[Channel2]", "pad_7", 0x3A, "B");
    output.addOutput("[Channel2]", "pad_8", 0x3B, "B");

    output.addOutput("[Channel1]", "pfl", 0x1A, "B");
    output.addOutput("[Channel2]", "pfl", 0x1B, "B");

    output.addOutput("[Channel1]", "vu-18", 0x1C, "B");
    output.addOutput("[Channel1]", "vu-12", 0x1D, "B");
    output.addOutput("[Channel1]", "vu-6", 0x1E, "B");
    output.addOutput("[Channel1]", "vu0", 0x1F, "B");
    output.addOutput("[Channel1]", "vu6", 0x20, "B");
    output.addOutput("[Channel1]", "peak_indicator", 0x21, "B");

    output.addOutput("[Channel2]", "vu-18", 0x22, "B");
    output.addOutput("[Channel2]", "vu-12", 0x23, "B");
    output.addOutput("[Channel2]", "vu-6", 0x24, "B");
    output.addOutput("[Channel2]", "vu0", 0x25, "B");
    output.addOutput("[Channel2]", "vu6", 0x26, "B");
    output.addOutput("[Channel2]", "peak_indicator", 0x27, "B");

    output.addOutput("[ChannelX]", "fxButton1", 0x16, "B");
    output.addOutput("[ChannelX]", "fxButton2", 0x17, "B");
    output.addOutput("[ChannelX]", "fxButton3", 0x18, "B");
    output.addOutput("[ChannelX]", "fxButton4", 0x19, "B");

    output.addOutput("[Channel1]", "reverse", 0x01, "B");
    output.addOutput("[Channel2]", "reverse", 0x28, "B");

    output.addOutput("[Channel1]", "slip_enabled", 0x02, "B");
    output.addOutput("[Channel2]", "slip_enabled", 0x29, "B");

    output.addOutput("[Channel1]", "addTrack", 0x03, "B");
    output.addOutput("[Channel2]", "addTrack", 0x2A, "B");

    output.addOutput("[Channel1]", "grid", 0x05, "B");
    output.addOutput("[Channel2]", "grid", 0x2C, "B");

    output.addOutput("[Channel1]", "MaximizeLibrary", 0x04, "B");
    output.addOutput("[Channel2]", "MaximizeLibrary", 0x2B, "B");

    output.addOutput("[ChannelX]", "quantize", 0x3C, "B");
    output.addOutput("[Microphone]", "talkover", 0x3D, "B");

    this.controller.registerOutputPacket(output);

    this.linkOutput("[Channel1]", "play_indicator", this.outputHandler);
    this.linkOutput("[Channel2]", "play_indicator", this.outputHandler);

    this.linkOutput("[Channel1]", "cue_indicator", this.outputHandler);
    this.linkOutput("[Channel2]", "cue_indicator", this.outputHandler);

    this.linkOutput("[Channel1]", "sync_enabled", this.outputHandler);
    this.linkOutput("[Channel2]", "sync_enabled", this.outputHandler);

    this.linkOutput("[Channel1]", "keylock", this.outputHandler);
    this.linkOutput("[Channel2]", "keylock", this.outputHandler);

    for (let i = 1; i <= 8; ++i) {
        engine.makeConnection("[Channel1]", `hotcue_${  i  }_enabled`, this.hotcueOutputHandler);
        engine.makeConnection("[Channel2]", `hotcue_${  i  }_enabled`, this.hotcueOutputHandler);
        engine.makeConnection("[Channel1]", `hotcue_${  i  }_color`, this.hotcueColorHandler);
        engine.makeConnection("[Channel2]", `hotcue_${  i  }_color`, this.hotcueColorHandler);
    }

    this.linkOutput("[Channel1]", "pfl", this.outputHandler);
    this.linkOutput("[Channel2]", "pfl", this.outputHandler);

    this.linkOutput("[Channel1]", "slip_enabled", this.outputHandler);
    this.linkOutput("[Channel2]", "slip_enabled", this.outputHandler);

    this.linkOutput("[Microphone]", "talkover", this.outputHandler);

    // VuMeter
    this.vuLeftConnection = engine.makeUnbufferedConnection("[Channel1]", "vu_meter", this.vuMeterHandler);
    this.vuRightConnection = engine.makeUnbufferedConnection("[Channel2]", "vu_meter", this.vuMeterHandler);
    this.clipLeftConnection = engine.makeConnection("[Channel1]", "peak_indicator", this.peakOutputHandler.bind(this));
    this.clipRightConnection = engine.makeConnection("[Channel2]", "peak_indicator", this.peakOutputHandler.bind(this));

    // Sampler callbacks
    for (let i = 1; i <= TraktorS2MK3.samplerCount; ++i) {
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "track_loaded", this.samplesOutputHandler.bind(this)));
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "play", this.samplesOutputHandler.bind(this)));
    }

    this.fxCallbacks = [];
    for (const group of ["[Channel1]", "[Channel2]"]) {
        this.fxCallbacks.push(engine.makeConnection(`[QuickEffectRack1_${group}]`, "loaded_chain_preset", this.fxOutputHandler));
    }

    TraktorS2MK3.lightDeck(false);
};

/* Helper function to link output in a short form */
TraktorS2MK3.linkOutput = function(group, name, callback) {
    TraktorS2MK3.controller.linkOutput(group, name, group, name, callback);
};

TraktorS2MK3.vuMeterHandler = function(value, group, _key) {
    const vuKeys = Object.keys(TraktorS2MK3.vuMeterThresholds);
    for (let i = 0; i < vuKeys.length; ++i) {
        // Avoid spamming HID by only sending last LED update
        const last = (i === (vuKeys.length - 1));
        if (TraktorS2MK3.vuMeterThresholds[vuKeys[i]] > value) {
            TraktorS2MK3.controller.setOutput(group, vuKeys[i], 0x00, last);
        } else {
            TraktorS2MK3.controller.setOutput(group, vuKeys[i], 0x7E, last);
        }
    }
};

TraktorS2MK3.peakOutputHandler = function(value, group, key) {
    let ledValue = 0x00;
    if (value) {
        ledValue = 0x7E;
    }

    TraktorS2MK3.controller.setOutput(group, key, ledValue, true);
};

TraktorS2MK3.outputHandler = function(value, group, key) {
    // Custom value for multi-colored LEDs
    let ledValue = value;
    if (value === 0 || value === false) {
        // Off value (dimmed)
        ledValue = 0x7C;
    } else if (value === 1 || value === true) {
        // On value
        ledValue = 0x7E;
    }

    TraktorS2MK3.controller.setOutput(group, key, ledValue, true);
};

TraktorS2MK3.colorOutputHandler = function(value, group, key) {
    // Custom value for multi-colored LEDs
    const colorValue = TraktorS2MK3.PadColorMap.getValueForNearestColor(value);
    TraktorS2MK3.controller.setOutput(group, key, colorValue, true);
};

TraktorS2MK3.hotcueOutputHandler = function(value, group, key) {
    // Light button LED only when we are in hotcue mode
    if (TraktorS2MK3.padModeState[group] === 0) {
        const colorKey = key.replace("_enabled", "_color");
        const color = engine.getValue(group, colorKey);
        const padNum = key[7];
        if (value > 0) {
            TraktorS2MK3.colorOutputHandler(color, group, `pad_${  padNum}`);
        } else {
            TraktorS2MK3.outputHandler(0, group, `pad_${  padNum}`);
        }
    }
};

TraktorS2MK3.hotcueColorHandler = function(value, group, key) {
    // Light button LED only when we are in hotcue mode
    const padNum = key[7];
    if (TraktorS2MK3.padModeState[group] === 0) {
        TraktorS2MK3.colorOutputHandler(value, group, `pad_${  padNum}`);
    }
};

TraktorS2MK3.samplesOutputHandler = function(value, group, key) {
    // Sampler 1-4, 9-12 -> Channel1
    // Samples 5-8, 13-16 -> Channel2
    const sampler = TraktorS2MK3.resolveSampler(group);
    let deck = "[Channel1]";
    let num = sampler;
    if (sampler === undefined) {
        return;
    } else if (sampler > 4 && sampler < 9) {
        deck = "[Channel2]";
        num = sampler - 4;
    } else if (sampler > 8 && sampler < 13) {
        num = sampler - 4;
    } else if (sampler > 12 && sampler < 17) {
        deck = "[Channel2]";
        num = sampler - 8;
    }

    // If we are in samples modes light corresponding LED
    if (TraktorS2MK3.padModeState[deck] === 1) {
        if (key === "play" && engine.getValue(group, "track_loaded")) {
            if (value) {
                // Green light on play
                TraktorS2MK3.outputHandler(0x9E, deck, "pad_" + num);
            } else {
                // Reset LED to full white light
                TraktorS2MK3.outputHandler(1, deck, "pad_" + num);
            }
        } else if (key === "track_loaded") {
            TraktorS2MK3.outputHandler(value, deck, "pad_" + num);
        }
    }
};

TraktorS2MK3.resolveSampler = function(group) {
    if (group === undefined) {
        return undefined;
    }

    const result = group.match(script.samplerRegEx);

    if (result === null) {
        return undefined;
    }

    // Return sample number
    return result[1];
};

TraktorS2MK3.lightDeck = function(switchOff) {
    let softLight = 0x7C;
    let fullLight = 0x7E;
    if (switchOff) {
        softLight = 0x00;
        fullLight = 0x00;
    }

    let current = (engine.getValue("[Channel1]", "play_indicator") ? fullLight : softLight);
    TraktorS2MK3.controller.setOutput("[Channel1]", "play_indicator", current, false);
    current = (engine.getValue("[Channel2]", "play_indicator")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel2]", "play_indicator", current, false);

    current = (engine.getValue("[Channel1]", "cue_indicator")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel1]", "cue_indicator", current, false);
    current = (engine.getValue("[Channel2]", "cue_indicator")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel2]", "cue_indicator", current, false);

    TraktorS2MK3.controller.setOutput("[Channel1]", "shift", softLight, false);
    TraktorS2MK3.controller.setOutput("[Channel2]", "shift", softLight, false);

    current = (engine.getValue("[Channel1]", "sync_enabled")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel1]", "sync_enabled", current, false);
    current = (engine.getValue("[Channel1]", "sync_enabled")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel2]", "sync_enabled", current, false);

    // Hotcues mode is default start value
    TraktorS2MK3.controller.setOutput("[Channel1]", "hotcues", fullLight, false);
    TraktorS2MK3.controller.setOutput("[Channel2]", "hotcues", fullLight, false);

    TraktorS2MK3.controller.setOutput("[Channel1]", "samples", softLight, false);
    TraktorS2MK3.controller.setOutput("[Channel2]", "samples", softLight, false);

    current = (engine.getValue("[Channel1]", "keylock")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel1]", "keylock", current, false);
    current = (engine.getValue("[Channel2]", "keylock")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel2]", "keylock", current, false);

    for (let i = 1; i <= 8; ++i) {
        if (switchOff) {
            // do not dim but turn completely off
            TraktorS2MK3.outputHandler(0x02, "[Channel1]", "pad_" + i);
            TraktorS2MK3.outputHandler(0x02, "[Channel2]", "pad_" + i);
        } else {
            current = engine.getValue("[Channel1]", "hotcue_" + i + "_enabled");
            TraktorS2MK3.hotcueOutputHandler(current, "[Channel1]", "hotcue_" + i + "_enabled");
            current = engine.getValue("[Channel2]", "hotcue_" + i + "_enabled");
            TraktorS2MK3.hotcueOutputHandler(current, "[Channel2]", "hotcue_" + i + "_enabled");
        }
    }

    current = (engine.getValue("[Channel1]", "pfl")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel1]", "pfl", current, false);
    current = (engine.getValue("[Channel2]", "pfl")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel2]", "pfl", current, false);

    TraktorS2MK3.controller.setOutput("[ChannelX]", "fxButton1", softLight, false);
    TraktorS2MK3.controller.setOutput("[ChannelX]", "fxButton2", softLight, false);
    TraktorS2MK3.controller.setOutput("[ChannelX]", "fxButton3", softLight, false);
    TraktorS2MK3.controller.setOutput("[ChannelX]", "fxButton4", softLight, false);
    // Set FX button LED state according to active quick effects on start-up
    if (!switchOff) { TraktorS2MK3.fxOutputHandler(); }

    TraktorS2MK3.controller.setOutput("[Channel1]", "reverse", softLight, false);
    TraktorS2MK3.controller.setOutput("[Channel2]", "reverse", softLight, false);

    current = (engine.getValue("[Channel1]", "slip_enabled")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel1]", "slip_enabled", current, false);
    current = (engine.getValue("[Channel2]", "slip_enabled")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Channel2]", "slip_enabled", current, false);

    TraktorS2MK3.controller.setOutput("[Channel1]", "addTrack", softLight, false);
    TraktorS2MK3.controller.setOutput("[Channel2]", "addTrack", softLight, false);

    TraktorS2MK3.controller.setOutput("[Channel1]", "grid", softLight, false);
    TraktorS2MK3.controller.setOutput("[Channel2]", "grid", softLight, false);

    TraktorS2MK3.controller.setOutput("[Channel1]", "MaximizeLibrary", softLight, false);
    TraktorS2MK3.controller.setOutput("[Channel2]", "MaximizeLibrary", softLight, false);

    TraktorS2MK3.controller.setOutput("[ChannelX]", "quantize", softLight, false);

    // For the last output we should send the packet finally
    current = (engine.getValue("[Microphone]", "talkover")) ? fullLight : softLight;
    TraktorS2MK3.controller.setOutput("[Microphone]", "talkover", current, true);
};

TraktorS2MK3.messageCallback = function(packet, data) {
    for (const name in data) {
        if (Object.hasOwnProperty.call(data, name)) {
            TraktorS2MK3.controller.processButton(data[name]);
        }
    }
};

TraktorS2MK3.shutdown = function() {
    // Deactivate all LEDs
    TraktorS2MK3.lightDeck(true);

    HIDDebug("TraktorS2MK3: Shutdown done!");
};

TraktorS2MK3.incomingData = function(data, length) {
    TraktorS2MK3.controller.parsePacket(data, length);
};

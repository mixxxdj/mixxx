///////////////////////////////////////////////////////////////////////////////////
// JSHint configuration                                                          //
///////////////////////////////////////////////////////////////////////////////////
/* global engine                                                                 */
/* global script                                                                 */
/* global HIDDebug                                                               */
/* global HIDPacket                                                              */
/* global HIDController                                                          */
/* jshint -W016                                                                  */
///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* Traktor Kontrol S3 HID controller script v1.00                                */
/* Last modification: July 2020                                                  */
/* Author: Owen Williams                                                         */
/* https://www.mixxx.org/wiki/doku.php/native_instruments_traktor_kontrol_s3     */
/*                                                                               */
///////////////////////////////////////////////////////////////////////////////////

var TraktorS3 = new function() {
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
    this.wheelTouchInertiaTimer = {
        "[Channel1]": 0,
        "[Channel2]": 0,
        "[Channel3]": 0,
        "[Channel4]": 0
    };

    // VuMeter
    this.vuLeftConnection = {};
    this.vuRightConnection = {};
    this.clipLeftConnection = {};
    this.clipRightConnection = {};

    // Sampler callbacks
    this.samplerCallbacks = [];
    this.samplerHotcuesRelation = {
        "[Channel1]": {
            1: 1, 2: 2, 3: 3, 4: 4, 5: 9, 6: 10, 7: 11, 8: 12
        }, "[Channel2]": {
            1: 5, 2: 6, 3: 7, 4: 8, 5: 13, 6: 14, 7: 15, 8: 16
        }
    };
};

TraktorS3.init = function(id) {
    TraktorS3.registerInputPackets();
    TraktorS3.registerOutputPackets();
    HIDDebug("TraktorS3: Init done!");

    TraktorS3.debugLights();
};

TraktorS3.registerInputPackets = function() {
    var messageShort = new HIDPacket("shortmessage", 0x01, this.messageCallback);
    var messageLong = new HIDPacket("longmessage", 0x02, this.messageCallback);

    this.registerInputButton(messageShort, "[Channel1]", "!play", 0x03, 0x01, this.playHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!play", 0x06, 0x02, this.playHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!cue_default", 0x02, 0x80, this.cueHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!cue_default", 0x06, 0x01, this.cueHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!shift", 0x01, 0x01, this.shiftHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!shift", 0x04, 0x02, this.shiftHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!sync", 0x02, 0x08, this.syncHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!sync", 0x05, 0x10, this.syncHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!keylock", 0x02, 0x10, this.keylockHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!keylock", 0x05, 0x20, this.keylockHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!hotcues", 0x02, 0x20, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!hotcues", 0x05, 0x40, this.padModeHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!samples", 0x02, 0x40, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!samples", 0x05, 0x80, this.padModeHandler);

    // // Number pad buttons (Hotcues or Samplers depending on current mode)
    this.registerInputButton(messageShort, "[Channel1]", "!pad_1", 0x03, 0x02, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_2", 0x03, 0x04, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_3", 0x03, 0x08, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_4", 0x03, 0x10, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_5", 0x03, 0x20, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_6", 0x03, 0x40, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_7", 0x03, 0x80, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_8", 0x04, 0x01, this.numberButtonHandler);

    this.registerInputButton(messageShort, "[Channel2]", "!pad_1", 0x06, 0x04, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_2", 0x06, 0x08, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_3", 0x06, 0x10, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_4", 0x06, 0x20, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_5", 0x06, 0x40, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_6", 0x06, 0x80, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_7", 0x07, 0x01, this.numberButtonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_8", 0x07, 0x02, this.numberButtonHandler);

    // // Headphone buttons
    this.registerInputButton(messageShort, "[Channel1]", "!pfl", 0x08, 0x01, this.headphoneHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pfl", 0x08, 0x02, this.headphoneHandler);
    this.registerInputButton(messageShort, "[Channel3]", "!pfl", 0x07, 0x80, this.headphoneHandler);
    this.registerInputButton(messageShort, "[Channel4]", "!pfl", 0x08, 0x04, this.headphoneHandler);

    // // Track browsing
    // TODO: bind touch: 0x09/0x40, 0x0A/0x02
    this.registerInputButton(messageShort, "[Channel1]", "!SelectTrack", 0x0B, 0x0F, this.selectTrackHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!SelectTrack", 0x0C, 0xF0, this.selectTrackHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!LoadSelectedTrack", 0x09, 0x01, this.loadTrackHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!LoadSelectedTrack", 0x09, 0x08, this.loadTrackHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!MaximizeLibrary", 0x01, 0x40, this.maximizeLibraryHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!MaximizeLibrary", 0x04, 0x80, this.maximizeLibraryHandler);
    // this.registerInputButton(messageShort, "[Channel1]", "!AddTrack", 0x01, 0x04, this.addTrackHandler);
    // this.registerInputButton(messageShort, "[Channel2]", "!AddTrack", 0x04, 0x10, this.addTrackHandler);

    // // Loop control
    // TODO: bind touch detections: 0x0A/0x01, 0x0A/0x08
    this.registerInputButton(messageShort, "[Channel1]", "!SelectLoop", 0x0C, 0x0F, this.selectLoopHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!SelectLoop", 0x0D, 0xF0, this.selectLoopHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!ActivateLoop", 0x09, 0x04, this.activateLoopHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!ActivateLoop", 0x09, 0x20, this.activateLoopHandler);

    // // Beatjump
    // TODO: bind touch detections: 0x09/0x80, 0x0A/0x04
    this.registerInputButton(messageShort, "[Channel1]", "!SelectBeatjump", 0x0B, 0xF0, this.selectBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!SelectBeatjump", 0x0D, 0x0F, this.selectBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!ActivateBeatjump", 0x09, 0x02, this.activateBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!ActivateBeatjump", 0x09, 0x10, this.activateBeatjumpHandler);

    // // There is only one button on the controller, we use to toggle quantization for all channels
    // this.registerInputButton(messageShort, "[ChannelX]", "!quantize", 0x06, 0x40, this.quantizeHandler);

    // // Microphone
    // this.registerInputButton(messageShort, "[Microphone]", "!talkover", 0x06, 0x80, this.microphoneHandler);

    // // Jog wheels
    this.registerInputButton(messageShort, "[Channel1]", "!jog_touch", 0x0A, 0x10, this.jogTouchHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!jog_touch", 0x0A, 0x20, this.jogTouchHandler);
    this.registerInputJog(messageShort, "[Channel1]", "!jog", 0x0E, 0xFFFFFF, this.jogHandler);
    this.registerInputJog(messageShort, "[Channel2]", "!jog", 0x12, 0xFFFFFF, this.jogHandler);

    // // FX Buttons
    this.registerInputButton(messageShort, "[ChannelX]", "!fx1", 0x08, 0x08, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx2", 0x08, 0x10, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx3", 0x08, 0x20, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx4", 0x08, 0x40, this.fxHandler);

    // // Rev / FLUX / GRID
    this.registerInputButton(messageShort, "[Channel1]", "!reverse", 0x01, 0x04, this.reverseHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!reverse", 0x04, 0x08, this.reverseHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!slip_enabled", 0x01, 0x02, this.fluxHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!slip_enabled", 0x04, 0x04, this.fluxHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!grid", 0x01, 0x08, this.beatgridHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!grid", 0x05, 0x01, this.beatgridHandler);

    // // TODO: implement jog
    // this.registerInputButton(messageShort, "[Channel1]", "!grid", 0x02, 0x01, this.jogHandler);
    // this.registerInputButton(messageShort, "[Channel2]", "!grid", 0x05, 0x02, this.jpgHandler);

    // Unmapped: preview, star, list, encoder touches, jog
    // DECK ASSIGNMENT

    this.controller.registerInputPacket(messageShort);

    this.registerInputScaler(messageLong, "[Channel1]", "rate", 0x01, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "rate", 0x0D, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Channel1]", "volume", 0x05, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "volume", 0x07, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel3]", "volume", 0x03, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel4]", "volume", 0x09, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Channel1]", "pregain", 0x11, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "pregain", 0x13, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel3]", "pregain", 0x0F, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel4]", "pregain", 0x15, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x25, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x27, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x29, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x2B, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x2D, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x2F, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel3]_Effect1]", "parameter3", 0x1F, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel3]_Effect1]", "parameter2", 0x21, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel3]_Effect1]", "parameter1", 0x23, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel4]_Effect1]", "parameter3", 0x31, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel4]_Effect1]", "parameter2", 0x33, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel4]_Effect1]", "parameter1", 0x35, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel1]]", "super1", 0x39, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel2]]", "super1", 0x3B, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel3]]", "super1", 0x37, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel4]]", "super1", 0x3D, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Master]", "crossfader", 0x0B, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "gain", 0x17, 0xFFFF, this.parameterHandler);
    // this.registerInputScaler(messageLong, "[Sampler]", "pregain", 0x17, 0xFFFF, this.samplerPregainHandler);
    this.registerInputScaler(messageLong, "[Master]", "headMix", 0x1D, 0xFFFF, this.parameterHandler);
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
    engine.softTakeover("[Master]", "gain", true);
    engine.softTakeover("[Master]", "headMix", true);
    engine.softTakeover("[Master]", "headGain", true);

    for (var i = 1; i <= 16; ++i) {
        engine.softTakeover("[Sampler" + i + "]", "pregain", true);
    }

    // Dirty hack to set initial values in the packet parser
    var data = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    TraktorS3.incomingData(data);
};

TraktorS3.registerInputJog = function(message, group, name, offset, bitmask, callback) {
    // Jog wheels have 4 byte input
    message.addControl(group, name, offset, "I", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS3.registerInputScaler = function(message, group, name, offset, bitmask, callback) {
    message.addControl(group, name, offset, "H", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS3.registerInputButton = function(message, group, name, offset, bitmask, callback) {
    message.addControl(group, name, offset, "B", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS3.playHandler = function(field) {
    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue(field.group, "start_stop", field.value);
    } else if (field.value === 1) {
        script.toggleControl(field.group, "play");
    }
};

TraktorS3.cueHandler = function(field) {
    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue(field.group, "cue_gotoandstop", field.value);
    } else {
        engine.setValue(field.group, "cue_default", field.value);
    }
};

TraktorS3.shiftHandler = function(field) {
    HIDDebug("SHIFT!" + field.group + " " + field.value);
    TraktorS3.shiftPressed[field.group] = field.value;
    engine.setValue("[Controls]", "touch_shift", field.value);
    TraktorS3.outputHandler(field.value, field.group, "shift");
};

TraktorS3.keylockHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "keylock");
};

TraktorS3.syncHandler = function(field) {
    if (TraktorS3.shiftPressed[field.group]) {
        // engine.setValue(field.group, "start_stop", field.value);
    } else {
        //TODO: latching not working?
        engine.setValue(field.group, "sync_enabled", field.value);
    }
};

TraktorS3.padModeHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    if (TraktorS3.padModeState[field.group] === 0 && field.name === "!samples") {
        // If we are in hotcues mode and samples mode is activated
        engine.setValue("[Samplers]", "show_samplers", 1);
        TraktorS3.padModeState[field.group] = 1;
        TraktorS3.outputHandler(0, field.group, "hotcues");
        TraktorS3.outputHandler(1, field.group, "samples");

        // Light LEDs for all slots with loaded samplers
        for (var key in TraktorS3.samplerHotcuesRelation[field.group]) {
            if (TraktorS3.samplerHotcuesRelation[field.group].hasOwnProperty(key)) {
                var loaded = engine.getValue("[Sampler" + TraktorS3.samplerHotcuesRelation[field.group][key] + "]", "track_loaded");
                TraktorS3.outputHandler(loaded, field.group, "pad_" + key);
            }
        }
    } else if (field.name === "!hotcues") {
        // If we are in samples mode and hotcues mode is activated
        TraktorS3.padModeState[field.group] = 0;
        TraktorS3.outputHandler(1, field.group, "hotcues");
        TraktorS3.outputHandler(0, field.group, "samples");

        // Light LEDs for all enabled hotcues
        for (var i = 1; i <= 8; ++i) {
            var active = engine.getValue(field.group, "hotcue_" + i + "_enabled");
            TraktorS3.outputHandler(active, field.group, "pad_" + i);
        }
    }
};

TraktorS3.numberButtonHandler = function(field) {
    var padNumber = parseInt(field.id[field.id.length - 1]);
    if (TraktorS3.padModeState[field.group] === 0) {
        // Hotcues mode
        if (TraktorS3.shiftPressed[field.group]) {
            engine.setValue(field.group, "hotcue_" + padNumber + "_clear", field.value);
        } else {
            engine.setValue(field.group, "hotcue_" + padNumber + "_activate", field.value);
        }
    } else {
        // Samples mode
        var sampler = TraktorS3.samplerHotcuesRelation[field.group][padNumber];
        if (TraktorS3.shiftPressed[field.group]) {
            var playing = engine.getValue("[Sampler" + sampler + "]", "play");
            if (playing) {
                engine.setValue("[Sampler" + sampler + "]", "cue_default", field.value);
            } else {
                engine.setValue("[Sampler" + sampler + "]", "eject", field.value);
            }
        } else {
            var loaded = engine.getValue("[Sampler" + sampler + "]", "track_loaded");
            if (loaded) {
                engine.setValue("[Sampler" + sampler + "]", "cue_gotoandplay", field.value);
            } else {
                engine.setValue("[Sampler" + sampler + "]", "LoadSelectedTrack", field.value);
            }
        }
    }
};

TraktorS3.headphoneHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "pfl");
};

TraktorS3.selectTrackHandler = function(field) {
    var delta = 1;
    if ((field.value + 1) % 16 === TraktorS3.browseKnobEncoderState[field.group]) {
        delta = -1;
    }

    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue("[Library]", "MoveHorizontal", delta);
    } else {
        engine.setValue("[Library]", "MoveVertical", delta);
    }

    TraktorS3.browseKnobEncoderState[field.group] = field.value;
};

TraktorS3.loadTrackHandler = function(field) {
    HIDDebug("LOAD TRACK!!!");
    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue(field.group, "eject", field.value);
    } else {
        engine.setValue(field.group, "LoadSelectedTrack", field.value);
    }
};

TraktorS3.addTrackHandler = function(field) {
    TraktorS3.outputHandler(field.value, field.group, "addTrack");

    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue("[Library]", "AutoDjAddTop", field.value);
    } else {
        engine.setValue("[Library]", "AutoDjAddBottom", field.value);
    }
};

TraktorS3.maximizeLibraryHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl("[Master]", "maximize_library");
};

TraktorS3.selectLoopHandler = function(field) {
    if ((field.value + 1) % 16 === TraktorS3.loopKnobEncoderState[field.group]) {
        script.triggerControl(field.group, "loop_halve");
    } else {
        script.triggerControl(field.group, "loop_double");
    }

    TraktorS3.loopKnobEncoderState[field.group] = field.value;
};

TraktorS3.activateLoopHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue(field.group, "reloop_toggle", field.value);
    } else {
        engine.setValue(field.group, "beatloop_activate", field.value);
    }
};

TraktorS3.selectBeatjumpHandler = function(field) {
    var delta = 1;
    if ((field.value + 1) % 16 === TraktorS3.moveKnobEncoderState[field.group]) {
        delta = -1;
    }

    if (TraktorS3.shiftPressed[field.group]) {
        var beatjump_size = engine.getValue(field.group, "beatjump_size");
        if (delta > 0) {
            engine.setValue(field.group, "beatjump_size", beatjump_size * 2);
        } else {
            engine.setValue(field.group, "beatjump_size", beatjump_size / 2);
        }
    } else {
        if (delta < 0) {
            script.triggerControl(field.group, "beatjump_backward");
        } else {
            script.triggerControl(field.group, "beatjump_forward");
        }
    }

    TraktorS3.moveKnobEncoderState[field.group] = field.value;
};

TraktorS3.activateBeatjumpHandler = function(field) {
    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue(field.group, "reloop_andstop", field.value);
    } else {
        engine.setValue(field.group, "beatlooproll_activate", field.value);
    }
};

TraktorS3.quantizeHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    var res = !(engine.getValue("[Channel1]", "quantize") && engine.getValue("[Channel2]", "quantize"));
    engine.setValue("[Channel1]", "quantize", res);
    engine.setValue("[Channel2]", "quantize", res);
    TraktorS3.outputHandler(res, field.group, "quantize");
};

TraktorS3.microphoneHandler = function(field) {
    if (field.value) {
        if (TraktorS3.microphonePressedTimer === 0) {
            // Start timer to measure how long button is pressed
            TraktorS3.microphonePressedTimer = engine.beginTimer(300, function() {
                // Reset microphone button timer status if active
                if (TraktorS3.microphonePressedTimer !== 0) {
                    TraktorS3.microphonePressedTimer = 0;
                }
            }, true);
        }

        script.toggleControl("[Microphone]", "talkover");
    } else {
        // Button is released, check if timer is still running
        if (TraktorS3.microphonePressedTimer !== 0) {
            // short klick -> permanent activation
            TraktorS3.microphonePressedTimer = 0;
        } else {
            engine.setValue("[Microphone]", "talkover", 0);
        }
    }
};

TraktorS3.parameterHandler = function(field) {
    engine.setParameter(field.group, field.name, field.value / 4095);
};

TraktorS3.samplerPregainHandler = function(field) {
    // Map sampler gain knob of all sampler together.
    // Dirty hack, but the best we can do for now.
    for (var i = 1; i <= 16; ++i) {
        engine.setParameter("[Sampler" + i + "]", field.name, field.value / 4095);
    }
};

TraktorS3.jogTouchHandler = function(field) {
    if (TraktorS3.wheelTouchInertiaTimer[field.group] != 0) {
    // The wheel was touched again, reset the timer.
        engine.stopTimer(TraktorS3.wheelTouchInertiaTimer[field.group]);
        TraktorS3.wheelTouchInertiaTimer[field.group] = 0;
    }
    if (field.value !== 0) {
        var deckNumber = TraktorS3.controller.resolveDeck(group);
        engine.scratchEnable(deckNumber, 1024, 33.3333, 0.125, 0.125/8, true);
    } else {
    // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
    // Depending on how fast the platter was moving, lengthen the time we'll wait.
        var scratchRate = Math.abs(engine.getValue(field.group, "scratch2"));
        // Note: inertiaTime multiplier is controller-specific and should be factored out.
        var inertiaTime = Math.pow(1.8, scratchRate) * 2;
        if (inertiaTime < 100) {
            // Just do it now.
            TraktorS3.finishJogTouch(field.group);
        } else {
            TraktorS3.controller.wheelTouchInertiaTimer[field.group] = engine.beginTimer(
                inertiaTime, "TraktorS3.finishJogTouch(\"" + field.group + "\")", true);
        }
    }
};

TraktorS3.jogHandler = function(field) {
    var deltas = TraktorS3.wheelDeltas(field.group, field.value);
    var tick_delta = deltas[0];
    var time_delta = deltas[1];

    var velocity = TraktorS3.scalerJog(tick_delta, time_delta);
    HIDDebug("VELO: " + velocity);
    engine.setValue(field.group, "jog", velocity);
    if (engine.getValue(field.group, "scratch2_enable")) {
        var deckNumber = TraktorS3.controller.resolveDeck(group);
        engine.scratchTick(deckNumber, tick_delta);
    }
};

TraktorS3.scalerJog = function(tick_delta, time_delta) {
    // If it's playing nudge
    var multiplier = 1.0;
    if (TraktorS3.shiftPressed["[Channel1]"] || TraktorS3.shiftPressed["[Channel2]"]) {
        multiplier = 100.0;
    }
    if (engine.getValue(group, "play")) {
        return multiplier * (tick_delta / time_delta) / 3;
    }

    return (tick_delta / time_delta) * multiplier * 2.0;
};

TraktorS3.wheelDeltas = function(deckNumber, value) {
    // When the wheel is touched, four bytes change, but only the first behaves predictably.
    // It looks like the wheel is 1024 ticks per revolution.
    var tickval = value & 0xFF;
    var timeval = value >>> 16;
    var prevTick = 0;
    var prevTime = 0;

    // Group 1 and 2 -> Array index 0 and 1
    prevTick = this.lastTickVal[deckNumber - 1];
    prevTime = this.lastTickTime[deckNumber - 1];
    this.lastTickVal[deckNumber - 1] = tickval;
    this.lastTickTime[deckNumber - 1] = timeval;

    if (prevTime > timeval) {
        // We looped around.  Adjust current time so that subtraction works.
        timeval += 0x10000;
    }
    var timeDelta = timeval - prevTime;
    if (timeDelta === 0) {
        // Spinning too fast to detect speed!  By not dividing we are guessing it took 1ms.
        timeDelta = 1;
    }

    var tickDelta = 0;
    if (prevTick >= 200 && tickval <= 100) {
        tickDelta = tickval + 256 - prevTick;
    } else if (prevTick <= 100 && tickval >= 200) {
        tickDelta = tickval - prevTick - 256;
    } else {
        tickDelta = tickval - prevTick;
    }

    return [tickDelta, timeDelta];
};

TraktorS3.finishJogTouch = function(group) {
    TraktorS3.wheelTouchInertiaTimer[group] = 0;
    var deckNumber = TraktorS3.controller.resolveDeck(group);
    // No vinyl button (yet)
    /*if (this.vinylActive) {
    // Vinyl button still being pressed, don't disable scratch mode yet.
    this.wheelTouchInertiaTimer[group] = engine.beginTimer(
        100, "VestaxVCI400.Decks." + this.deckIdentifier + ".finishJogTouch()", true);
    return;
  }*/
    var play = engine.getValue(group, "play");
    if (play != 0) {
    // If we are playing, just hand off to the engine.
        engine.scratchDisable(deckNumber, true);
    } else {
    // If things are paused, there will be a non-smooth handoff between scratching and jogging.
    // Instead, keep scratch on until the platter is not moving.
        var scratchRate = Math.abs(engine.getValue(group, "scratch2"));
        if (scratchRate < 0.01) {
            // The platter is basically stopped, now we can disable scratch and hand off to jogging.
            engine.scratchDisable(deckNumber, false);
        } else {
            // Check again soon.
            TraktorS3.wheelTouchInertiaTimer[group] = engine.beginTimer(
                100, "TraktorS3.finishJogTouch(\"" + group + "\")", true);
        }
    }
};

TraktorS3.fxHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    var fxNumber = parseInt(field.id[field.id.length - 1]);
    var group = "[EffectRack1_EffectUnit" + fxNumber + "]";

    // Toggle effect unit
    TraktorS3.fxButtonState[fxNumber] = !TraktorS3.fxButtonState[fxNumber];

    engine.setValue(group, "group_[Channel1]_enable", TraktorS3.fxButtonState[fxNumber]);
    engine.setValue(group, "group_[Channel2]_enable", TraktorS3.fxButtonState[fxNumber]);
    TraktorS3.outputHandler(TraktorS3.fxButtonState[fxNumber], field.group, "fxButton" + fxNumber);
};

TraktorS3.reverseHandler = function(field) {
    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue(field.group, "reverseroll", field.value);
    } else {
        engine.setValue(field.group, "reverse", field.value);
    }

    TraktorS3.outputHandler(field.value, field.group, "reverse");
};

TraktorS3.fluxHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "slip_enabled");
};

TraktorS3.beatgridHandler = function(field) {
    if (TraktorS3.shiftPressed[field.group]) {
        engine.setValue(field.group, "beats_translate_match_alignment", field.value);
    } else {
        engine.setValue(field.group, "beats_translate_curpos", field.value);
    }

    TraktorS3.outputHandler(field.value, field.group, "grid");
};

TraktorS3.debugLights = function() {
    // Call this if you want to just send raw packets to the controller (good for figuring out what
    // bytes do what).
    //var data_strings = ["80 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 0A 0A 0A 0A 0A 0A 0A 0A 00 7F 00 00 00 00 0A 0A 0A 0A 0A 0A",
    //                    "81 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 7F 0A 0A 0A 0A 0A 7F 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A",
    //                    "82 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 00 00 7F 7F 7F 7F 7F 7F 00 00 7F 7F 7F 7F 7F 00 00 00 7F 7F 7F 7F 7F 7F 00 00 7F 7F 7F 7F 7F 00 00 00"];
    //                   00 01 02 03  04 05 06 07  08 09 0A 0B  0C 0D 0E 0F
    var data_strings = [
        "      FF 00  00 FF FF FF  00 00 00 00  FF FF FF 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 FF 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "FF ",
        "      FF 00  00 00 00 00  00 00 00 00  00 00 00 FF " +
        "FF 00 00 00  00 00 00 00  FF 00 00 00  00 00 FF FF " +
        "FF 00 00 00  00 00 00 00  FF 00 00 00  00 00 00 00 " +
        "FF 00 00 00  FF 00 00 FF  00 00 00 00  FF 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 "
    ];
    var data = [Object(), Object()];

    for (i = 0; i < data.length; i++) {
        var ok = true;
        var splitted = data_strings[i].split(/\s+/);
        HIDDebug("i " + i + " " + splitted);
        data[i].length = splitted.length;
        for (j = 0; j < splitted.length; j++) {
            var byte_str = splitted[j];
            if (byte_str.length === 0) {
                continue;
            }
            if (byte_str.length !== 2) {
                ok = false;
                HIDDebug("not two characters?? " + byte_str);
            }
            var b = parseInt(byte_str, 16);
            if (b < 0 || b > 255) {
                ok = false;
                HIDDebug("number out of range: " + byte_str + " " + b);
            }
            data[i][j] = b;
        }
        if (ok) {
            controller.send(data[i], data[i].length, 0x80 + i);
        }
    }
};

TraktorS3.registerOutputPackets = function() {
    var outputA = new HIDPacket("outputA", 0x80);

    outputA.addOutput("[Channel1]", "shift", 0x01, "B");
    outputA.addOutput("[Channel2]", "shift", 0x1A, "B");

    outputA.addOutput("[Channel1]", "slip_enabled", 0x02, "B");
    outputA.addOutput("[Channel2]", "slip_enabled", 0x1B, "B");

    outputA.addOutput("[Channel1]", "reverse", 0x03, "B");
    outputA.addOutput("[Channel2]", "reverse", 0x1C, "B");

    outputA.addOutput("[Channel1]", "keylock", 0x0D, "B");
    outputA.addOutput("[Channel2]", "keylock", 0x26, "B");

    outputA.addOutput("[Channel1]", "hotcues", 0x0E, "B");
    outputA.addOutput("[Channel2]", "hotcues", 0x27, "B");

    outputA.addOutput("[Channel1]", "samples", 0x0F, "B");
    outputA.addOutput("[Channel2]", "samples", 0x28, "B");

    outputA.addOutput("[Channel1]", "cue_indicator", 0x10, "B");
    outputA.addOutput("[Channel2]", "cue_indicator", 0x29, "B");

    outputA.addOutput("[Channel1]", "play_indicator", 0x11, "B");
    outputA.addOutput("[Channel2]", "play_indicator", 0x2A, "B");

    outputA.addOutput("[Channel1]", "sync_enabled", 0x0C, "B");
    outputA.addOutput("[Channel2]", "sync_enabled", 0x25, "B");

    outputA.addOutput("[Channel1]", "pad_1", 0x12, "B");
    outputA.addOutput("[Channel1]", "pad_2", 0x13, "B");
    outputA.addOutput("[Channel1]", "pad_3", 0x14, "B");
    outputA.addOutput("[Channel1]", "pad_4", 0x15, "B");
    outputA.addOutput("[Channel1]", "pad_5", 0x16, "B");
    outputA.addOutput("[Channel1]", "pad_6", 0x17, "B");
    outputA.addOutput("[Channel1]", "pad_7", 0x18, "B");
    outputA.addOutput("[Channel1]", "pad_8", 0x19, "B");

    outputA.addOutput("[Channel2]", "pad_1", 0x2B, "B");
    outputA.addOutput("[Channel2]", "pad_2", 0x2C, "B");
    outputA.addOutput("[Channel2]", "pad_3", 0x2D, "B");
    outputA.addOutput("[Channel2]", "pad_4", 0x2E, "B");
    outputA.addOutput("[Channel2]", "pad_5", 0x2F, "B");
    outputA.addOutput("[Channel2]", "pad_6", 0x30, "B");
    outputA.addOutput("[Channel2]", "pad_7", 0x31, "B");
    outputA.addOutput("[Channel2]", "pad_8", 0x32, "B");

    outputA.addOutput("[Channel1]", "pfl", 0x39, "B");
    outputA.addOutput("[Channel2]", "pfl", 0x3A, "B");
    outputA.addOutput("[Channel3]", "pfl", 0x38, "B");
    outputA.addOutput("[Channel4]", "pfl", 0x3B, "B");

    // outputA.addOutput("[Channel1]", "addTrack", 0x03, "B");
    // outputA.addOutput("[Channel2]", "addTrack", 0x2A, "B");

    outputA.addOutput("[Channel1]", "grid", 0x08, "B");
    outputA.addOutput("[Channel2]", "grid", 0x20, "B");

    outputA.addOutput("[Channel1]", "MaximizeLibrary", 0x07, "B");
    outputA.addOutput("[Channel2]", "MaximizeLibrary", 0x21, "B");

    // outputA.addOutput("[ChannelX]", "quantize", 0x3C, "B");
    // outputA.addOutput("[Microphone]", "talkover", 0x3D, "B");

    this.controller.registerOutputPacket(outputA);

    var outputB = new HIDPacket("outputB", 0x81);

    var VuOffsets = {
        "[Channel3]": 0x01,
        "[Channel1]": 0x10,
        "[Channel2]": 0x1F,
        "[Channel4]": 0x2E
    };
    for (ch in VuOffsets) {
        for (i = 0; i < 14; i++) {
            outputB.addOutput(ch, "!" + "VuMeter" + i, VuOffsets[ch] + i, "B");
        }
    }

    outputB.addOutput("[Channel3]", "PeakIndicator", 0x09, "B");
    outputB.addOutput("[Channel1]", "PeakIndicator", 0x1E, "B");
    outputB.addOutput("[Channel2]", "PeakIndicator", 0x2D, "B");
    outputB.addOutput("[Channel4]", "PeakIndicator", 0x3C, "B");

    outputB.addOutput("[ChannelX]", "fxButton1", 0x3C, "B");
    outputB.addOutput("[ChannelX]", "fxButton2", 0x3D, "B");
    outputB.addOutput("[ChannelX]", "fxButton3", 0x3E, "B");
    outputB.addOutput("[ChannelX]", "fxButton4", 0x3F, "B");

    this.controller.registerOutputPacket(outputB);

    this.linkOutput("[Channel1]", "play_indicator", this.outputHandler);
    this.linkOutput("[Channel2]", "play_indicator", this.outputHandler);

    this.linkOutput("[Channel1]", "cue_indicator", this.outputHandler);
    this.linkOutput("[Channel2]", "cue_indicator", this.outputHandler);

    this.linkOutput("[Channel1]", "sync_enabled", this.outputHandler);
    this.linkOutput("[Channel2]", "sync_enabled", this.outputHandler);

    this.linkOutput("[Channel1]", "keylock", this.outputHandler);
    this.linkOutput("[Channel2]", "keylock", this.outputHandler);

    for (var i = 1; i <= 8; ++i) {
        TraktorS3.controller.linkOutput("[Channel1]", "pad_" + i, "[Channel1]", "hotcue_" + i + "_enabled", this.hotcueOutputHandler);
        TraktorS3.controller.linkOutput("[Channel2]", "pad_" + i, "[Channel2]", "hotcue_" + i + "_enabled", this.hotcueOutputHandler);
    }

    this.linkOutput("[Channel1]", "pfl", this.outputHandler);
    this.linkOutput("[Channel2]", "pfl", this.outputHandler);

    this.linkOutput("[Channel1]", "slip_enabled", this.outputHandler);
    this.linkOutput("[Channel2]", "slip_enabled", this.outputHandler);

    this.linkOutput("[Microphone]", "talkover", this.outputHandler);

    // VuMeter
    this.vuLeftConnection = engine.makeConnection("[Channel1]", "VuMeter", this.vuMeterHandler);
    this.vuRightConnection = engine.makeConnection("[Channel2]", "VuMeter", this.vuMeterHandler);
    this.clipLeftConnection = engine.makeConnection("[Channel1]", "PeakIndicator", this.peakOutputHandler);
    this.clipRightConnection = engine.makeConnection("[Channel2]", "PeakIndicator", this.peakOutputHandler);

    // Sampler callbacks
    for (i = 1; i <= 16; ++i) {
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "track_loaded", this.samplesOutputHandler));
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "play", this.samplesOutputHandler));
    }

    TraktorS3.lightDeck(false);
};

/* Helper function to link output in a short form */
TraktorS3.linkOutput = function(group, name, callback) {
    TraktorS3.controller.linkOutput(group, name, group, name, callback);
};

// TraktorS3.vuMeterHandler = function (value, group, key) {
//     var vuKeys = Object.keys(TraktorS3.vuMeterThresholds);
//     for (var i = 0; i < vuKeys.length; ++i) {
//         // Avoid spamming HID by only sending last LED update
//         var last = (i === (vuKeys.length - 1));
//         if (TraktorS3.vuMeterThresholds[vuKeys[i]] > value) {
//             TraktorS3.controller.setOutput(group, vuKeys[i], 0x00, last);
//         } else {
//             TraktorS3.controller.setOutput(group, vuKeys[i], 0x7E, last);
//         }
//     }
// };

TraktorS3.vuMeterHandler = function(value, group, key) {
    // This handler is called a lot so it should be as fast as possible.

    // VU is drawn on 14 segments, the 15th indicates clip.
    // Figure out number of fully-illuminated segments.
    var scaledValue = value * 14.0;
    var fullIllumCount = Math.floor(scaledValue);

    // Figure out how much the partially-illuminated segment is illuminated.
    var partialIllum = (scaledValue - fullIllumCount) * 0x7F;

    for (i = 0; i < 14; i++) {
        var key = "!" + "VuMeter" + i;
        if (i < fullIllumCount) {
            // Don't update lights until they're all done, so the last term is false.
            TraktorS3.controller.setOutput(group, key, 0x7F, false);
        } else if (i == fullIllumCount) {
            TraktorS3.controller.setOutput(group, key, partialIllum, false);
        } else {
            TraktorS3.controller.setOutput(group, key, 0x00, false);
        }
    }
    TraktorS3.controller.OutputPackets["outputB"].send();
};

TraktorS3.peakOutputHandler = function(value, group, key) {
    var ledValue = 0x00;
    if (value) {
        ledValue = 0x7E;
    }

    TraktorS3.controller.setOutput(group, key, ledValue, true);
};

TraktorS3.outputHandler = function(value, group, key) {
    // Custom value for multi-colored LEDs
    var ledValue = value;
    if (value === 0 || value === false) {
        // Off value
        ledValue = 0x7C;
    } else if (value === 1 || value === true) {
        // On value
        ledValue = 0x7E;
    }

    TraktorS3.controller.setOutput(group, key, ledValue, true);
};

TraktorS3.hotcueOutputHandler = function(value, group, key) {
    // Light button LED only when we are in hotcue mode
    if (TraktorS3.padModeState[group] === 0) {
        TraktorS3.outputHandler(value, group, key);
    }
};

TraktorS3.samplesOutputHandler = function(value, group, key) {
    // Sampler 1-4, 9-12 -> Channel1
    // Samples 5-8, 13-16 -> Channel2
    var sampler = TraktorS3.resolveSampler(group);
    var deck = "[Channel1]";
    var num = sampler;
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
    if (TraktorS3.padModeState[deck] === 1) {
        if (key === "play" && engine.getValue(group, "track_loaded")) {
            if (value) {
                // Green light on play
                TraktorS3.outputHandler(0x9E, deck, "pad_" + num);
            } else {
                // Reset LED to full white light
                TraktorS3.outputHandler(1, deck, "pad_" + num);
            }
        } else if (key === "track_loaded") {
            TraktorS3.outputHandler(value, deck, "pad_" + num);
        }
    }
};

TraktorS3.resolveSampler = function(group) {
    if (group === undefined) {
        return undefined;
    }

    var result = group.match(script.samplerRegEx);

    if (result === null) {
        return undefined;
    }

    // Return sample number
    return result[1];
};

TraktorS3.lightDeck = function(switchOff) {
    var softLight = 0x7C;
    var fullLight = 0x7E;
    if (switchOff) {
        softLight = 0x00;
        fullLight = 0x00;
    }

    var current = (engine.getValue("[Channel1]", "play_indicator") ? fullLight : softLight);
    TraktorS3.controller.setOutput("[Channel1]", "play_indicator", current, false);
    current = (engine.getValue("[Channel2]", "play_indicator")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel2]", "play_indicator", current, false);

    current = (engine.getValue("[Channel1]", "cue_indicator")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel1]", "cue_indicator", current, false);
    current = (engine.getValue("[Channel2]", "cue_indicator")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel2]", "cue_indicator", current, false);

    TraktorS3.controller.setOutput("[Channel1]", "shift", softLight, false);
    TraktorS3.controller.setOutput("[Channel2]", "shift", softLight, false);

    current = (engine.getValue("[Channel1]", "sync_enabled")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel1]", "sync_enabled", current, false);
    current = (engine.getValue("[Channel1]", "sync_enabled")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel2]", "sync_enabled", current, false);

    // Hotcues mode is default start value
    TraktorS3.controller.setOutput("[Channel1]", "hotcues", fullLight, false);
    TraktorS3.controller.setOutput("[Channel2]", "hotcues", fullLight, false);

    TraktorS3.controller.setOutput("[Channel1]", "samples", softLight, false);
    TraktorS3.controller.setOutput("[Channel2]", "samples", softLight, false);

    current = (engine.getValue("[Channel1]", "keylock")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel1]", "keylock", current, false);
    current = (engine.getValue("[Channel2]", "keylock")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel2]", "keylock", current, false);

    for (var i = 1; i <= 8; ++i) {
        current = (engine.getValue("[Channel1]", "hotcue_" + i + "_enabled")) ? fullLight : softLight;
        TraktorS3.controller.setOutput("[Channel1]", "pad_" + i, current, false);
        current = (engine.getValue("[Channel2]", "hotcue_" + i + "_enabled")) ? fullLight : softLight;
        TraktorS3.controller.setOutput("[Channel2]", "pad_" + i, current, false);
    }

    current = (engine.getValue("[Channel1]", "pfl")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel1]", "pfl", current, false);
    current = (engine.getValue("[Channel2]", "pfl")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel2]", "pfl", current, false);

    TraktorS3.controller.setOutput("[ChannelX]", "fxButton1", softLight, false);
    TraktorS3.controller.setOutput("[ChannelX]", "fxButton2", softLight, false);
    TraktorS3.controller.setOutput("[ChannelX]", "fxButton3", softLight, false);
    TraktorS3.controller.setOutput("[ChannelX]", "fxButton4", softLight, false);

    TraktorS3.controller.setOutput("[Channel1]", "reverse", softLight, false);
    TraktorS3.controller.setOutput("[Channel2]", "reverse", softLight, false);

    current = (engine.getValue("[Channel1]", "slip_enabled")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel1]", "slip_enabled", current, false);
    current = (engine.getValue("[Channel2]", "slip_enabled")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Channel2]", "slip_enabled", current, false);

    TraktorS3.controller.setOutput("[Channel1]", "addTrack", softLight, false);
    TraktorS3.controller.setOutput("[Channel2]", "addTrack", softLight, false);

    TraktorS3.controller.setOutput("[Channel1]", "grid", softLight, false);
    TraktorS3.controller.setOutput("[Channel2]", "grid", softLight, false);

    TraktorS3.controller.setOutput("[Channel1]", "MaximizeLibrary", softLight, false);
    TraktorS3.controller.setOutput("[Channel2]", "MaximizeLibrary", softLight, false);

    TraktorS3.controller.setOutput("[ChannelX]", "quantize", softLight, false);

    // For the last output we should send the packet finally
    current = (engine.getValue("[Microphone]", "talkover")) ? fullLight : softLight;
    TraktorS3.controller.setOutput("[Microphone]", "talkover", current, true);
};

TraktorS3.messageCallback = function(packet, data) {
    for (var name in data) {
        if (data.hasOwnProperty(name)) {
            TraktorS3.controller.processButton(data[name]);
        }
    }
};

TraktorS3.shutdown = function() {
    // Deactivate all LEDs
    TraktorS3.lightDeck(true);

    HIDDebug("TraktorS3: Shutdown done!");
};

TraktorS3.incomingData = function(data, length) {
    TraktorS3.controller.parsePacket(data, length);
};

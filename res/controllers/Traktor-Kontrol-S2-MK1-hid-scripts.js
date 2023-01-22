/****************************************************************/
/*      Traktor Kontrol S2 MK1 HID controller script            */
/*      Copyright (C) 2021, leifhelm                            */
/*      Based on:                                               */
/*      Traktor Kontrol S2 MK2 HID controller script v1.00      */
/*      Copyright (C) 2020, Be <be@mixxx.org>                   */
/*      Copyright (C) 2017, z411 <z411@omaera.org>              */
/*      but feel free to tweak this to your heart's content!    */
/****************************************************************/

// ==== Jog Wheel Touch Calibration ====
// Set the threshold for scratching for each jog wheel.
// Bigger values mean more force. The unpressed value is around 3122.
// The fully pressed value is around 3728.
var JogWheelTouchThreshold = {
    "[Channel1]": 3328,
    "[Channel2]": 3328,
};

// ==== Friendly User Configuration ====
// The Cue button, when Shift is also held, can have two possible functions:
// 1. "REWIND": seeks to the very start of the track.
// 2. "REVERSEROLL": performs a temporary reverse or "censor" effect, where the track
//    is momentarily played in reverse until the button is released.
var ShiftCueButtonAction = "REWIND";

// Set the brightness of button LEDs which are off and on. This uses a scale from 0 to 0x1F (31).
// If you don't have the optional power adapter and are using the controller with USB bus power,
var ButtonBrightnessOff = 0x00;
var ButtonBrightnessOn = 0x1F;

// eslint definitions
var TraktorS2MK1 = new function() {
    this.controller = new HIDController();

    // When true, packets will not be sent to the controller.
    // Used when updating multiple LEDs simultaneously.
    this.batchingLEDUpdate = false;

    // Previous values, used for calculating deltas for encoder knobs.
    this.previousBrowse = 0;
    this.previousPregain = {
        "[Channel1]": 0,
        "[Channel2]": 0
    };
    this.previousLeftEncoder = {
        "[Channel1]": 0,
        "[Channel2]": 0
    };
    this.previousRightEncoder = {
        "[Channel1]": 0,
        "[Channel2]": 0
    };
    this.wheelTouchInertiaTimer = {
        "[Channel1]": 0,
        "[Channel2]": 0
    };

    this.gainEncoderPressed = {
        "[Channel1]": false,
        "[Channel2]": false
    };
    this.leftEncoderPressed = {
        "[Channel1]": false,
        "[Channel2]": false
    };
    this.shiftPressed = {
        "[Channel1]": false,
        "[Channel2]": false
    };

    this.padModes = {
        "hotcue": 0,
        "introOutro": 1,
        "sampler": 2
    };
    this.currentPadMode = {
        "[Channel1]": this.padModes.hotcue,
        "[Channel2]": this.padModes.hotcue
    };
    this.padConnections = {
        "[Channel1]": [],
        "[Channel2]": []
    };

    this.lastTickValue = [0, 0];
    this.lastTickTime = [0.0, 0.0];
    this.syncEnabledTime = {};

    this.longPressTimeoutMilliseconds = 275;

    this.effectButtonLongPressTimer = {
        "[EffectRack1_EffectUnit1]": [0, 0, 0, 0],
        "[EffectRack1_EffectUnit2]": [0, 0, 0, 0]
    };
    this.effectButtonIsLongPressed = {
        "[EffectRack1_EffectUnit1]": [false, false, false, false],
        "[EffectRack1_EffectUnit2]": [false, false, false, false]
    };
    this.effectFocusLongPressTimer = {
        "[EffectRack1_EffectUnit1]": 0,
        "[EffectRack1_EffectUnit2]": 0
    };
    this.effectFocusChooseModeActive = {
        "[EffectRack1_EffectUnit1]": false,
        "[EffectRack1_EffectUnit2]": false
    };
    this.effectFocusButtonPressedWhenParametersHidden = {
        "[EffectRack1_EffectUnit1]": false,
        "[EffectRack1_EffectUnit2]": false
    };
    this.previouslyFocusedEffect = {
        "[EffectRack1_EffectUnit1]": null,
        "[EffectRack1_EffectUnit2]": null
    };
    this.effectButtonLEDconnections = {
        "[EffectRack1_EffectUnit1]": [],
        "[EffectRack1_EffectUnit2]": []
    };
};

TraktorS2MK1.registerInputPackets = function() {
    var InputReport0x01 = new HIDPacket("InputReport_0x01", 0x01, this.inputReport0x01Callback);
    var InputReport0x02 = new HIDPacket("InputReport_0x02", 0x02, this.inputReport0x02Callback);

    // Values in input report 0x01 are all buttons, except the jog wheels.
    // An exclamation point indicates a specially-handled function.  Everything else is a standard
    // Mixxx control object name.

    InputReport0x01.addControl("[Channel1]", "!gain_encoder_press", 0x0E, "B", 0x01, false, this.gainEncoderPress);
    InputReport0x01.addControl("[Channel1]", "!shift", 0x0D, "B", 0x80, false, this.shift);
    InputReport0x01.addControl("[Channel1]", "!sync_enabled", 0x0D, "B", 0x40, false, this.syncButton);
    InputReport0x01.addControl("[Channel1]", "!cue_default", 0x0D, "B", 0x20, false, this.cueButton);
    InputReport0x01.addControl("[Channel1]", "!play", 0x0D, "B", 0x10, false, this.playButton);
    InputReport0x01.addControl("[Channel1]", "!pad1", 0x0D, "B", 0x08, false, this.padButton);
    InputReport0x01.addControl("[Channel1]", "!pad2", 0x0D, "B", 0x04, false, this.padButton);
    InputReport0x01.addControl("[Channel1]", "!pad3", 0x0D, "B", 0x02, false, this.padButton);
    InputReport0x01.addControl("[Channel1]", "!pad4", 0x0D, "B", 0x01, false, this.padButton);
    InputReport0x01.addControl("[Channel1]", "!loop_in", 0x09, "B", 0x40, false, this.loopInButton);
    InputReport0x01.addControl("[Channel1]", "!loop_out", 0x09, "B", 0x20, false, this.loopOutButton);
    InputReport0x01.addControl("[Channel1]", "!samples_button", 0x0B, "B", 0x02, false, this.samplerModeButton);
    InputReport0x01.addControl("[Channel1]", "!reset_button", 0x09, "B", 0x10, false, this.introOutroModeButton);
    InputReport0x01.addControl("[Channel1]", "!left_encoder_press", 0x0E, "B", 0x02, false, this.leftEncoderPress);
    InputReport0x01.addControl("[Channel1]", "!right_encoder_press", 0x0E, "B", 0x04, false, this.rightEncoderPress);
    InputReport0x01.addControl("[Channel1]", "!jog_wheel", 0x01, "I", 0xFFFFFFFF, false, this.jogMove);
    InputReport0x01.addControl("[Channel1]", "!load_track", 0x0B, "B", 0x08, false, this.loadTrackButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit1]", "!effect_focus_button",
        0x09, "B", 0x08, false, this.effectFocusButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit1]", "!effectbutton1", 0x09, "B", 0x04, false, this.effectButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit1]", "!effectbutton2", 0x09, "B", 0x02, false, this.effectButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit1]", "!effectbutton3", 0x09, "B", 0x01, false, this.effectButton);

    InputReport0x01.addControl("[Channel2]", "!gain_encoder_press", 0x0E, "B", 0x10, false, this.gainEncoderPress);
    InputReport0x01.addControl("[Channel2]", "!shift", 0x0C, "B", 0x80, false, this.shift);
    InputReport0x01.addControl("[Channel2]", "!sync_enabled", 0x0C, "B", 0x40, false, this.syncButton);
    InputReport0x01.addControl("[Channel2]", "!cue_default", 0x0C, "B", 0x20, false, this.cueButton);
    InputReport0x01.addControl("[Channel2]", "!play", 0x0C, "B", 0x10, false, this.playButton);
    InputReport0x01.addControl("[Channel2]", "!pad1", 0x0C, "B", 0x08, false, this.padButton);
    InputReport0x01.addControl("[Channel2]", "!pad2", 0x0C, "B", 0x04, false, this.padButton);
    InputReport0x01.addControl("[Channel2]", "!pad3", 0x0C, "B", 0x02, false, this.padButton);
    InputReport0x01.addControl("[Channel2]", "!pad4", 0x0C, "B", 0x01, false, this.padButton);
    InputReport0x01.addControl("[Channel2]", "!loop_in", 0x0B, "B", 0x40, false, this.loopInButton);
    InputReport0x01.addControl("[Channel2]", "!loop_out", 0x0B, "B", 0x20, false, this.loopOutButton);
    InputReport0x01.addControl("[Channel2]", "!samples_button", 0x0B, "B", 0x01, false, this.samplerModeButton);
    InputReport0x01.addControl("[Channel2]", "!reset_button", 0x0B, "B", 0x10, false, this.introOutroModeButton);
    InputReport0x01.addControl("[Channel2]", "!left_encoder_press", 0x0E, "B", 0x20, false, this.leftEncoderPress);
    InputReport0x01.addControl("[Channel2]", "!right_encoder_press", 0x0E, "B", 0x40, false, this.rightEncoderPress);
    InputReport0x01.addControl("[Channel2]", "!jog_wheel", 0x05, "I", 0xFFFFFFFF, false, this.jogMove);
    InputReport0x01.addControl("[Channel2]", "!load_track", 0x0B, "B", 0x04, false, this.loadTrackButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit2]", "!effect_focus_button",
        0x0A, "B", 0x80, false, this.effectFocusButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit2]", "!effectbutton1", 0xA, "B", 0x40, false, this.effectButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit2]", "!effectbutton2", 0xA, "B", 0x20, false, this.effectButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit2]", "!effectbutton3", 0xA, "B", 0x10, false, this.effectButton);

    InputReport0x01.addControl("[Channel1]", "!pfl", 0x09, "B", 0x80, false, this.pflButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x0A, "B", 0x02);
    InputReport0x01.addControl("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x0A, "B", 0x01);

    InputReport0x01.addControl("[Channel2]", "!pfl", 0x0B, "B", 0x80, false, this.pflButton);
    InputReport0x01.addControl("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x0A, "B", 0x08);
    InputReport0x01.addControl("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x0A, "B", 0x04);

    InputReport0x01.addControl("[Master]", "maximize_library", 0x0E, "B", 0x08, false, this.toggleButton);

    engine.makeConnection("[EffectRack1_EffectUnit1]", "show_parameters", TraktorS2MK1.onShowParametersChange);
    engine.makeConnection("[EffectRack1_EffectUnit2]", "show_parameters", TraktorS2MK1.onShowParametersChange);

    this.controller.registerInputPacket(InputReport0x01);

    // Most items in the input report 0x02 are controls that go from 0-4095.
    // There are also some 4 bit encoders.
    InputReport0x02.addControl("[Channel1]", "rate", 0x0F, "H");
    InputReport0x02.addControl("[Channel2]", "rate", 0x1F, "H");
    InputReport0x02.addControl("[Channel1]", "!left_encoder", 0x01, "B", 0xF0, false, this.leftEncoder);
    InputReport0x02.addControl("[Channel1]", "!right_encoder", 0x02, "B", 0x0F, false, this.rightEncoder);
    InputReport0x02.addControl("[Channel2]", "!left_encoder", 0x03, "B", 0xF0, false, this.leftEncoder);
    InputReport0x02.addControl("[Channel2]", "!right_encoder", 0x04, "B", 0x0F, false, this.rightEncoder);

    InputReport0x02.addControl("[EffectRack1_EffectUnit1]", "mix", 0x0B, "H");
    InputReport0x02.addControl("[EffectRack1_EffectUnit1]", "!effectknob1", 0x09, "H", 0xFFFF, false, this.effectKnob);
    InputReport0x02.addControl("[EffectRack1_EffectUnit1]", "!effectknob2", 0x07, "H", 0xFFFF, false, this.effectKnob);
    InputReport0x02.addControl("[EffectRack1_EffectUnit1]", "!effectknob3", 0x05, "H", 0xFFFF, false, this.effectKnob);

    InputReport0x02.addControl("[EffectRack1_EffectUnit2]", "mix", 0x1B, "H");
    InputReport0x02.addControl("[EffectRack1_EffectUnit2]", "!effectknob1", 0x19, "H", 0xFFFF, false, this.effectKnob);
    InputReport0x02.addControl("[EffectRack1_EffectUnit2]", "!effectknob2", 0x17, "H", 0xFFFF, false, this.effectKnob);
    InputReport0x02.addControl("[EffectRack1_EffectUnit2]", "!effectknob3", 0x15, "H", 0xFFFF, false, this.effectKnob);

    InputReport0x02.addControl("[Channel1]", "volume", 0x2B, "H");
    InputReport0x02.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x11, "H");
    InputReport0x02.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x25, "H");
    InputReport0x02.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x27, "H");
    InputReport0x02.addControl("[Channel1]", "pregain", 0x01, "B", 0x0F, false, this.gainEncoder);
    InputReport0x02.addControl("[Channel1]", "!jog_touch", 0x0D, "H", 0xFFFF, false, this.jogTouch);

    InputReport0x02.addControl("[Channel2]", "volume", 0x2D, "H");
    InputReport0x02.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x21, "H");
    InputReport0x02.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x23, "H");
    InputReport0x02.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x29, "H");
    InputReport0x02.addControl("[Channel2]", "pregain", 0x03, "B", 0x0F, false, this.gainEncoder);
    InputReport0x02.addControl("[Channel2]", "!jog_touch", 0x1D, "H", 0xFFFF, false, this.jogTouch);

    InputReport0x02.addControl("[Master]", "crossfader", 0x2F, "H");
    InputReport0x02.addControl("[Master]", "headMix", 0x31, "H");
    InputReport0x02.addControl("[Master]", "!samplerGain", 0x13, "H");
    InputReport0x02.setCallback("[Master]", "!samplerGain", this.samplerGainKnob);
    InputReport0x02.addControl("[Playlist]", "!browse", 0x02, "B", 0xF0, false, this.browseEncoder);

    // Soft takeover for knobs
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);

    engine.softTakeover("[Channel1]", "volume", true);
    engine.softTakeover("[Channel2]", "volume", true);

    engine.softTakeover("[Channel1]", "pregain", true);
    engine.softTakeover("[Channel2]", "pregain", true);

    engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[Master]", "headMix", true);
    for (var i = 1; i <= 8; i++) {
        engine.softTakeover("[Sampler" + i + "]", "pregain", true);
    }

    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);

    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", true);

    for (i = 1; i <= 3; i++) {
        engine.softTakeover("[EffectRack1_EffectUnit1_Effect" + i + "]", "meta", true);
        engine.softTakeover("[EffectRack1_EffectUnit2_Effect" + i + "]", "meta", true);
        for (var j = 1; j <= 3; j++) {
            engine.softTakeover("[EffectRack1_EffectUnit1_Effect" + i + "]", "parameter" + j, true);
            engine.softTakeover("[EffectRack1_EffectUnit2_Effect" + i + "]", "parameter" + j, true);
        }
    }

    // Set scalers
    TraktorS2MK1.scalerParameter.useSetParameter = true;
    this.controller.setScaler("volume", this.scalerVolume);
    this.controller.setScaler("headMix", this.scalerSlider);
    this.controller.setScaler("parameter1", this.scalerParameter);
    this.controller.setScaler("parameter2", this.scalerParameter);
    this.controller.setScaler("parameter3", this.scalerParameter);
    this.controller.setScaler("super1", this.scalerParameter);
    this.controller.setScaler("crossfader", this.scalerSlider);
    this.controller.setScaler("rate", this.scalerSlider);
    this.controller.setScaler("mix", this.scalerParameter);

    // Register packet
    this.controller.registerInputPacket(InputReport0x02);
};

TraktorS2MK1.registerOutputPackets = function() {
    var OutputReport0x80 = new HIDPacket("OutputReport_0x80", 0x80);

    OutputReport0x80.addOutput("[Channel1]", "track_loaded", 0x1F, "B");
    OutputReport0x80.addOutput("[Channel2]", "track_loaded", 0x1E, "B");

    var VuOffsets = {
        "[Channel1]": 0x15,
        "[Channel2]": 0x11,
    };
    for (var ch in VuOffsets) {
        for (var i = 0; i <= 0x03; i++) {
            OutputReport0x80.addOutput(ch, "!" + "VuMeter" + i, VuOffsets[ch] + i, "B");
        }
    }

    OutputReport0x80.addOutput("[Channel1]", "PeakIndicator", 0x01, "B");
    OutputReport0x80.addOutput("[Channel2]", "PeakIndicator", 0x25, "B");

    OutputReport0x80.addOutput("[Channel1]", "!reset_button", 0x06, "B");
    OutputReport0x80.addOutput("[Channel1]", "loop_in", 0x02, "B");
    OutputReport0x80.addOutput("[Channel1]", "loop_out", 0x05, "B");

    OutputReport0x80.addOutput("[Channel2]", "!reset_button", 0x26, "B");
    OutputReport0x80.addOutput("[Channel2]", "loop_in", 0x22, "B");
    OutputReport0x80.addOutput("[Channel2]", "loop_out", 0x21, "B");

    OutputReport0x80.addOutput("[Channel1]", "pfl", 0x20, "B");
    OutputReport0x80.addOutput("[Master]", "!warninglight", 0x31, "B");
    OutputReport0x80.addOutput("[Channel2]", "pfl", 0x1D, "B");

    OutputReport0x80.addOutput("[EffectRack1_EffectUnit1]", "!effect_focus_button", 0x1C, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit1]", "!effectbutton1", 0x1B, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit1]", "!effectbutton2", 0x1A, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit1]", "!effectbutton3", 0x19, "B");

    OutputReport0x80.addOutput("[EffectRack1_EffectUnit2]", "!effect_focus_button", 0x39, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit2]", "!effectbutton1", 0x38, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit2]", "!effectbutton2", 0x37, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit2]", "!effectbutton3", 0x36, "B");

    OutputReport0x80.addOutput("[Channel1]", "!samples_button", 0x35, "B");
    OutputReport0x80.addOutput("[Channel2]", "!samples_button", 0x34, "B");

    OutputReport0x80.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x3D, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x3C, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x3B, "B");
    OutputReport0x80.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x3A, "B");

    OutputReport0x80.addOutput("[Channel1]", "!shift", 0x08, "B");
    OutputReport0x80.addOutput("[Channel1]", "sync_enabled", 0x04, "B");
    OutputReport0x80.addOutput("[Channel1]", "cue_indicator", 0x07, "B");
    OutputReport0x80.addOutput("[Channel1]", "play_indicator", 0x03, "B");

    OutputReport0x80.addOutput("[Channel1]", "!pad_1_G", 0x0C, "B");
    OutputReport0x80.addOutput("[Channel1]", "!pad_1_B", 0x10, "B");

    OutputReport0x80.addOutput("[Channel1]", "!pad_2_G", 0x0B, "B");
    OutputReport0x80.addOutput("[Channel1]", "!pad_2_B", 0x0F, "B");

    OutputReport0x80.addOutput("[Channel1]", "!pad_3_G", 0x0A, "B");
    OutputReport0x80.addOutput("[Channel1]", "!pad_3_B", 0x0E, "B");

    OutputReport0x80.addOutput("[Channel1]", "!pad_4_G", 0x09, "B");
    OutputReport0x80.addOutput("[Channel1]", "!pad_4_B", 0x0D, "B");

    OutputReport0x80.addOutput("[Channel2]", "!shift", 0x28, "B");
    OutputReport0x80.addOutput("[Channel2]", "sync_enabled", 0x24, "B");
    OutputReport0x80.addOutput("[Channel2]", "cue_indicator", 0x27, "B");
    OutputReport0x80.addOutput("[Channel2]", "play_indicator", 0x23, "B");

    OutputReport0x80.addOutput("[Channel2]", "!pad_1_G", 0x2C, "B");
    OutputReport0x80.addOutput("[Channel2]", "!pad_1_B", 0x30, "B");

    OutputReport0x80.addOutput("[Channel2]", "!pad_2_G", 0x2B, "B");
    OutputReport0x80.addOutput("[Channel2]", "!pad_2_B", 0x2F, "B");

    OutputReport0x80.addOutput("[Channel2]", "!pad_3_G", 0x2A, "B");
    OutputReport0x80.addOutput("[Channel2]", "!pad_3_B", 0x2E, "B");

    OutputReport0x80.addOutput("[Channel2]", "!pad_4_G", 0x29, "B");
    OutputReport0x80.addOutput("[Channel2]", "!pad_4_B", 0x2D, "B");

    this.controller.registerOutputPacket(OutputReport0x80);

    // Link up control objects to their outputs
    TraktorS2MK1.linkDeckOutputs("sync_enabled", TraktorS2MK1.outputCallback);
    TraktorS2MK1.linkDeckOutputs("cue_indicator", TraktorS2MK1.outputCallback);
    TraktorS2MK1.linkDeckOutputs("play_indicator", TraktorS2MK1.outputCallback);

    TraktorS2MK1.setPadMode("[Channel1]", TraktorS2MK1.padModes.hotcue);
    TraktorS2MK1.setPadMode("[Channel2]", TraktorS2MK1.padModes.hotcue);

    TraktorS2MK1.linkDeckOutputs("loop_in", TraktorS2MK1.outputCallbackLoop);
    TraktorS2MK1.linkDeckOutputs("loop_out", TraktorS2MK1.outputCallbackLoop);
    TraktorS2MK1.linkDeckOutputs("LoadSelectedTrack", TraktorS2MK1.outputCallback);
    TraktorS2MK1.linkDeckOutputs("slip_enabled", TraktorS2MK1.outputCallback);
    TraktorS2MK1.linkChannelOutput("[Channel1]", "pfl", TraktorS2MK1.outputChannelCallback);
    TraktorS2MK1.linkChannelOutput("[Channel2]", "pfl", TraktorS2MK1.outputChannelCallback);
    TraktorS2MK1.linkChannelOutput("[Channel1]", "track_loaded", TraktorS2MK1.outputChannelCallback);
    TraktorS2MK1.linkChannelOutput("[Channel2]", "track_loaded", TraktorS2MK1.outputChannelCallback);
    TraktorS2MK1.linkChannelOutput("[Channel1]", "PeakIndicator", TraktorS2MK1.outputChannelCallbackDark);
    TraktorS2MK1.linkChannelOutput("[Channel2]", "PeakIndicator", TraktorS2MK1.outputChannelCallbackDark);
    TraktorS2MK1.linkChannelOutput("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", TraktorS2MK1.outputChannelCallback);
    TraktorS2MK1.linkChannelOutput("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", TraktorS2MK1.outputChannelCallback);
    TraktorS2MK1.linkChannelOutput("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", TraktorS2MK1.outputChannelCallback);
    TraktorS2MK1.linkChannelOutput("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", TraktorS2MK1.outputChannelCallback);

    engine.makeConnection("[EffectRack1_EffectUnit1]", "focused_effect", TraktorS2MK1.onFocusedEffectChange).trigger();
    engine.makeConnection("[EffectRack1_EffectUnit2]", "focused_effect", TraktorS2MK1.onFocusedEffectChange).trigger();
    TraktorS2MK1.connectEffectButtonLEDs("[EffectRack1_EffectUnit1]");
    TraktorS2MK1.connectEffectButtonLEDs("[EffectRack1_EffectUnit2]");

    engine.makeConnection("[Channel1]", "VuMeter", TraktorS2MK1.onVuMeterChanged).trigger();
    engine.makeConnection("[Channel2]", "VuMeter", TraktorS2MK1.onVuMeterChanged).trigger();

    engine.makeConnection("[Channel1]", "loop_enabled", TraktorS2MK1.onLoopEnabledChanged);
    engine.makeConnection("[Channel2]", "loop_enabled", TraktorS2MK1.onLoopEnabledChanged);
};

TraktorS2MK1.linkDeckOutputs = function(key, callback) {
    TraktorS2MK1.controller.linkOutput("[Channel1]", key, "[Channel1]", key, callback);
    TraktorS2MK1.controller.linkOutput("[Channel2]", key, "[Channel2]", key, callback);
};

TraktorS2MK1.linkDeckCustomOutputs = function(key, callback) {
    engine.makeConnection("[Channel1]", key, callback).trigger();
    engine.makeConnection("[Channel2]", key, callback).trigger();
};

TraktorS2MK1.linkChannelOutput = function(group, key, callback) {
    TraktorS2MK1.controller.linkOutput(group, key, group, key, callback);
};

TraktorS2MK1.lightGroup = function(packet, outputGroupName, coGroupName) {
    var groupObject = packet.groups[outputGroupName];
    for (var fieldName in groupObject) {
        var field = groupObject[fieldName];
        if (field.name[0] === "!") {
            continue;
        }
        if (field.mapped_callback) {
            var value = engine.getValue(coGroupName, field.name);
            field.mapped_callback(value, coGroupName, field.name);
        }
    // No callback, no light!
    }
};

TraktorS2MK1.lightDeck = function(group) {
    // Freeze the lights while we do this update so we don't spam HID.
    this.batchingLEDUpdate = true;
    for (var packetName in this.controller.OutputPackets) {
        var packet = this.controller.OutputPackets[packetName];
        TraktorS2MK1.lightGroup(packet, group, group);
        // These outputs show state managed by this script and do not react to ControlObject changes,
        // so manually set them here.
        TraktorS2MK1.outputCallback(0, group, "!shift");
        TraktorS2MK1.outputCallback(0, group, "!reset_button");
        TraktorS2MK1.outputCallback(0, group, "!samples_button");
    }

    this.batchingLEDUpdate = false;
    // And now send them all.
    for (packetName in this.controller.OutputPackets) {
        this.controller.OutputPackets[packetName].send();
    }
};

TraktorS2MK1.init = function() {
    if (!(ShiftCueButtonAction === "REWIND" || ShiftCueButtonAction === "REVERSEROLL")) {
        throw new Error("ShiftCueButtonAction must be either \"REWIND\" or \"REVERSEROLL\"\n" +
            "ShiftCueButtonAction is: " + ShiftCueButtonAction);
    }
    if (typeof ButtonBrightnessOff !== "number" || ButtonBrightnessOff < 0 || ButtonBrightnessOff > 0x1f) {
        throw new Error("ButtonBrightnessOff must be a number between 0 and 0x1f (31).\n" +
            "ButtonBrightnessOff is: " + ButtonBrightnessOff);
    }
    if (typeof ButtonBrightnessOff !== "number" || ButtonBrightnessOff < 0 || ButtonBrightnessOff > 0x1f) {
        throw new Error("ButtonBrightnessOn must be a number between 0 and 0x1f (31).\n" +
            "ButtonBrightnessOn is: " + ButtonBrightnessOn);
    }
    if (ButtonBrightnessOn < ButtonBrightnessOff) {
        throw new Error("ButtonBrightnessOn must be greater than ButtonBrightnessOff.\n" +
            "ButtonBrightnessOn is: " + ButtonBrightnessOn + "\n" +
            "ButtonBrightnessOff is: " + ButtonBrightnessOff);
    }

    TraktorS2MK1.registerInputPackets();

    var debugLEDs = false;
    if (debugLEDs) {
        var data = [0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f];
        controller.send(data, data.length, 0x80);
    } else {
        TraktorS2MK1.registerOutputPackets();
    }

    TraktorS2MK1.controller.setOutput("[Master]", "!warninglight", 0x00, true);
    TraktorS2MK1.lightDeck("[Channel1]");
    TraktorS2MK1.lightDeck("[Channel2]");
    TraktorS2MK1.lightDeck("[EffectRack1_EffectUnit1]");
    TraktorS2MK1.lightDeck("[EffectRack1_EffectUnit2]");
};

TraktorS2MK1.shutdown = function() {
    var data = [];
    for (var i = 0; i < 61; i++) {
        data[i] = 0;
    }
    controller.send(data, data.length, 0x80);
};

TraktorS2MK1.incomingData = function(data, length) {
    TraktorS2MK1.controller.parsePacket(data, length);
};

// The input report 0x01 handles buttons and jog wheels.
TraktorS2MK1.inputReport0x01Callback = function(packet, data) {
    for (var name in data) {
        var field = data[name];
        if (field.name === "!jog_wheel") {
            TraktorS2MK1.controller.processControl(field);
            continue;
        }

        TraktorS2MK1.controller.processButton(field);
    }
};

// There are no buttons handled by input report 0x02, so this is a little simpler.
TraktorS2MK1.inputReport0x02Callback = function(packet, data) {
    for (var name in data) {
        var field = data[name];
        TraktorS2MK1.controller.processControl(field);
    }
};

TraktorS2MK1.samplerGainKnob = function(field) {
    for (var i = 1; i <= 8; i++) {
        engine.setParameter("[Sampler" + i + "]", "pregain", field.value / 4095);
    }
};

TraktorS2MK1.toggleButton = function(field) {
    if (field.value > 0) {
        script.toggleControl(field.group, field.name);
    }
};

TraktorS2MK1.shift = function(field) {
    var shiftPressed = field.value > 0;
    TraktorS2MK1.shiftPressed[field.group] = shiftPressed;
    TraktorS2MK1.controller.setOutput(field.group, "!shift",
        shiftPressed ? ButtonBrightnessOn : ButtonBrightnessOff,
        !TraktorS2MK1.batchingLEDUpdate);
};

TraktorS2MK1.loadTrackButton = function(field) {
    if (TraktorS2MK1.shiftPressed[field.group]) {
        engine.setValue(field.group, "CloneFromDeck", field.value);
    } else {
        engine.setValue(field.group, "LoadSelectedTrack", field.value);
    }
};

TraktorS2MK1.syncButton = function(field) {
    var now = Date.now();

    // If shifted, just toggle.
    // TODO(later version): actually make this enable explicit master.
    if (TraktorS2MK1.shiftPressed[field.group]) {
        if (field.value === 0) {
            return;
        }
        var synced = engine.getValue(field.group, "sync_enabled");
        engine.setValue(field.group, "sync_enabled", !synced);
    } else {
        if (field.value === 1) {
            TraktorS2MK1.syncEnabledTime[field.group] = now;
            engine.setValue(field.group, "sync_enabled", 1);
        } else {
            if (!engine.getValue(field.group, "sync_enabled")) {
                // If disabled, and switching to disable... stay disabled.
                engine.setValue(field.group, "sync_enabled", 0);
                return;
            }
            // was enabled, and button has been let go.  maybe latch it.
            if (now - TraktorS2MK1.syncEnabledTime[field.group] > 300) {
                engine.setValue(field.group, "sync_enabled", 1);
                return;
            }
            engine.setValue(field.group, "sync_enabled", 0);
        }
    }
};

TraktorS2MK1.cueButton = function(field) {
    if (TraktorS2MK1.shiftPressed[field.group]) {
        if (ShiftCueButtonAction === "REWIND") {
            if (field.value === 0) {
                return;
            }
            engine.setValue(field.group, "start_stop", 1);
        } else if (ShiftCueButtonAction === "REVERSEROLL") {
            engine.setValue(field.group, "reverseroll", field.value);
        }
    } else {
        engine.setValue(field.group, "cue_default", field.value);
    }
};

TraktorS2MK1.playButton = function(field) {
    if (field.value === 0) {
        return;
    }
    if (TraktorS2MK1.shiftPressed[field.group]) {
        var locked = engine.getValue(field.group, "keylock");
        engine.setValue(field.group, "keylock", !locked);
    } else {
        var playing = engine.getValue(field.group, "play");
        var deckNumber = TraktorS2MK1.controller.resolveDeck(field.group);
        // Failsafe to disable scratching in case the finishJogTouch timer has not executed yet
        // after a backspin.
        if (engine.isScratching(deckNumber)) {
            engine.scratchDisable(deckNumber, false);
        }
        engine.setValue(field.group, "play", !playing);
    }
};

TraktorS2MK1.jogTouch = function(field) {
    if (TraktorS2MK1.wheelTouchInertiaTimer[field.group] !== 0) {
    // The wheel was touched again, reset the timer.
        engine.stopTimer(TraktorS2MK1.wheelTouchInertiaTimer[field.group]);
        TraktorS2MK1.wheelTouchInertiaTimer[field.group] = 0;
    }
    if (field.value > JogWheelTouchThreshold[field.group]) {
        var deckNumber = TraktorS2MK1.controller.resolveDeck(field.group);
        engine.scratchEnable(deckNumber, 1024, 33.3333, 0.125, 0.125/8, true);
    } else {
    // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
    // Depending on how fast the platter was moving, lengthen the time we'll wait.
        var scratchRate = Math.abs(engine.getValue(field.group, "scratch2"));
        // inertiaTime was experimentally determined. It should be enough time to allow the user to
        // press play after a backspin without normal playback starting before they can press the
        // button, but not so long that there is an awkward delay before stopping scratching after
        // a backspin.
        var inertiaTime;
        if (TraktorS2MK1.shiftPressed[field.group]) {
            inertiaTime = Math.pow(1.7, scratchRate / 10) / 1.6;
        } else {
            inertiaTime = Math.pow(1.7, scratchRate) / 1.6;
        }
        if (inertiaTime < 100) {
            // Just do it now.
            TraktorS2MK1.finishJogTouch(field.group);
        } else {
            TraktorS2MK1.wheelTouchInertiaTimer[field.group] = engine.beginTimer(
                inertiaTime, function() {
                    TraktorS2MK1.finishJogTouch(field.group);
                }, true);
        }
    }
};

TraktorS2MK1.finishJogTouch = function(group) {
    TraktorS2MK1.wheelTouchInertiaTimer[group] = 0;
    var deckNumber = TraktorS2MK1.controller.resolveDeck(group);
    var play = engine.getValue(group, "play");
    if (play !== 0) {
    // If we are playing, just hand off to the engine.
        engine.scratchDisable(deckNumber, true);
    } else {
    // If things are paused, there will be a non-smooth handoff between scratching and jogging.
    // Instead, keep scratch on until the platter is not moving.
        var scratchRate = Math.abs(engine.getValue(group, "scratch2"));
        if (scratchRate < 0.01) {
            // The platter is basically stopped, now we can disable scratch and hand off to jogging.
            engine.scratchDisable(deckNumber, true);
        } else {
            // Check again soon.
            TraktorS2MK1.wheelTouchInertiaTimer[group] = engine.beginTimer(
                1, function() {
                    TraktorS2MK1.finishJogTouch(group);
                }, true);
        }
    }
};

TraktorS2MK1.jogMove = function(field) {
    var deltas = TraktorS2MK1.wheelDeltas(field.group, field.value);
    var tickDelta = deltas[0];
    var timeDelta = deltas[1];

    if (engine.getValue(field.group, "scratch2_enable")) {
        var deckNumber = TraktorS2MK1.controller.resolveDeck(field.group);
        if (TraktorS2MK1.shiftPressed[field.group]) {
            tickDelta *= 10;
        }
        engine.scratchTick(deckNumber, tickDelta);
    } else {
        var velocity = TraktorS2MK1.scalerJog(tickDelta, timeDelta, field.group);
        engine.setValue(field.group, "jog", velocity);
    }
};

TraktorS2MK1.wheelDeltas = function(group, value) {
    // When the wheel is touched, four bytes change, but only the first behaves predictably.
    // It looks like the wheel is 1024 ticks per revolution.
    var tickval = value & 0xFF;
    var timeValue = value >>> 16;
    var previousTick = 0;
    var previousTime = 0;

    if (group[8] === "1" || group[8] === "3") {
        previousTick = TraktorS2MK1.lastTickValue[0];
        previousTime = TraktorS2MK1.lastTickTime[0];
        TraktorS2MK1.lastTickValue[0] = tickval;
        TraktorS2MK1.lastTickTime[0] = timeValue;
    } else {
        previousTick = TraktorS2MK1.lastTickValue[1];
        previousTime = TraktorS2MK1.lastTickTime[1];
        TraktorS2MK1.lastTickValue[1] = tickval;
        TraktorS2MK1.lastTickTime[1] = timeValue;
    }

    if (previousTime > timeValue) {
    // We looped around.  Adjust current time so that subtraction works.
        timeValue += 0x10000;
    }
    var timeDelta = timeValue - previousTime;
    if (timeDelta === 0) {
    // Spinning too fast to detect speed!  By not dividing we are guessing it took 1ms.
        timeDelta = 1;
    }

    var tickDelta = 0;
    if (previousTick >= 200 && tickval <= 100) {
        tickDelta = tickval + 256 - previousTick;
    } else if (previousTick <= 100 && tickval >= 200) {
        tickDelta = tickval - previousTick - 256;
    } else {
        tickDelta = tickval - previousTick;
    }
    //HIDDebug(group + " " + tickval + " " + previousTick + " " + tickDelta);
    return [tickDelta, timeDelta];
};

TraktorS2MK1.scalerJog = function(tickDelta, timeDelta, group) {
    if (engine.getValue(group, "play")) {
        return (tickDelta / timeDelta) / 3;
    } else {
        return (tickDelta / timeDelta) * 2.0;
    }
};

var introOutroKeys = [
    "intro_start",
    "intro_end",
    "outro_start",
    "outro_end"
];

var introOutroColors = [
    {green: 0x1F, blue: 0},
    {green: 0x1F, blue: 0},
    {green: 0, blue: 0x1F},
    {green: 0, blue: 0x1F}
];

var introOutroColorsDim = [
    {green: 0x05, blue: 0},
    {green: 0x05, blue: 0},
    {green: 0, blue: 0x05},
    {green: 0, blue: 0x05}
];


TraktorS2MK1.setPadMode = function(group, padMode) {
    TraktorS2MK1.padConnections[group].forEach(function(connection) {
        connection.disconnect();
    });
    TraktorS2MK1.padConnections[group] = [];

    if (padMode === TraktorS2MK1.padModes.hotcue) {
        for (var i = 1; i <= 4; i++) {
            TraktorS2MK1.padConnections[group].push(
                engine.makeConnection(group, "hotcue_" + i + "_enabled", TraktorS2MK1.outputHotcueCallback));
        }
    } else if (padMode === TraktorS2MK1.padModes.introOutro) {
        for (i = 1; i <= 4; i++) {
            // This function to create callback functions is needed so the loop index variable
            // i does not get captured in a closure within the callback.
            var makeIntroOutroCallback = function(padNumber) {
                return function(value, group, _control) {
                    if (value > 0) {
                        TraktorS2MK1.sendPadColor(group, padNumber, introOutroColors[padNumber-1]);
                    } else {
                        TraktorS2MK1.sendPadColor(group, padNumber, introOutroColorsDim[padNumber-1]);
                    }
                };
            };
            TraktorS2MK1.padConnections[group].push(engine.makeConnection(
                group, introOutroKeys[i-1] + "_enabled", makeIntroOutroCallback(i)));
        }
    } else if (padMode === TraktorS2MK1.padModes.sampler) {
        for (i = 1; i <= 4; i++) {
            var makeSamplerCallback = function(deckGroup, padNumber) {
                var samplerNumber = deckGroup === "[Channel1]" ? padNumber : padNumber + 4;
                var samplerGroup = "[Sampler" + samplerNumber + "]";
                return function(_value, _group, _control) {
                    if (engine.getValue(samplerGroup, "track_loaded")) {
                        if (engine.getValue(samplerGroup, "play") === 1) {
                            if (engine.getValue(samplerGroup, "repeat") === 1) {
                                TraktorS2MK1.sendPadColor(deckGroup, padNumber,
                                    {green: 0x1F, blue: 0x1F});
                            } else {
                                TraktorS2MK1.sendPadColor(deckGroup, padNumber,
                                    {green: 0x1F, blue: 0});
                            }
                        } else {
                            TraktorS2MK1.sendPadColor(deckGroup, padNumber, {green: 0x05, blue: 0x00});
                        }
                    } else {
                        TraktorS2MK1.sendPadColor(deckGroup, padNumber, {green: 0, blue: 0});
                    }
                };
            };

            var sNumber = group === "[Channel1]" ? i : i + 4;
            var sGroup = "[Sampler" + sNumber + "]";
            TraktorS2MK1.padConnections[group].push(engine.makeConnection(
                sGroup, "track_loaded", makeSamplerCallback(group, i)));
            TraktorS2MK1.padConnections[group].push(engine.makeConnection(
                sGroup, "play", makeSamplerCallback(group, i)));
            TraktorS2MK1.padConnections[group].push(engine.makeConnection(
                sGroup, "repeat", makeSamplerCallback(group, i)));
        }
    }

    TraktorS2MK1.padConnections[group].forEach(function(connection) {
        connection.trigger();
    });

    TraktorS2MK1.currentPadMode[group] = padMode;
};

TraktorS2MK1.hotcueButton = function(buttonNumber, group, value) {
    if (TraktorS2MK1.shiftPressed[group]) {
        engine.setValue(group, "hotcue_" + buttonNumber + "_clear", value);
    } else {
        engine.setValue(group, "hotcue_" + buttonNumber + "_activate", value);
    }
};

TraktorS2MK1.introOutroButton = function(buttonNumber, group, value) {
    if (TraktorS2MK1.shiftPressed[group]) {
        engine.setValue(group, introOutroKeys[buttonNumber-1] + "_clear", value);
    } else {
        engine.setValue(group, introOutroKeys[buttonNumber-1] + "_activate", value);
    }
};

TraktorS2MK1.samplerButton = function(buttonNumber, group, value) {
    if (value === 0) {
        return;
    }
    var samplerNumber = group === "[Channel1]" ? buttonNumber : buttonNumber + 4;
    var samplerGroup = "[Sampler" + samplerNumber + "]";
    if (TraktorS2MK1.shiftPressed[group]) {
        if (engine.getValue(samplerGroup, "play") === 1) {
            engine.setValue(samplerGroup, "play", 0);
        } else {
            script.triggerControl(samplerGroup, "eject");
        }
    } else {
        if (engine.getValue(samplerGroup, "track_loaded") === 0) {
            script.triggerControl(samplerGroup, "LoadSelectedTrack");
        } else {
            script.triggerControl(samplerGroup, "cue_gotoandplay");
        }
    }
};

TraktorS2MK1.padButton = function(field) {
    var buttonNumber = parseInt(field.name[field.name.length - 1]);
    var padMode = TraktorS2MK1.currentPadMode[field.group];

    if (padMode === TraktorS2MK1.padModes.hotcue) {
        TraktorS2MK1.hotcueButton(buttonNumber, field.group, field.value);
    } else if (padMode === TraktorS2MK1.padModes.introOutro) {
        TraktorS2MK1.introOutroButton(buttonNumber, field.group, field.value);
    } else if (padMode === TraktorS2MK1.padModes.sampler) {
        TraktorS2MK1.samplerButton(buttonNumber, field.group, field.value);
    }
};

TraktorS2MK1.samplerModeButton = function(field) {
    if (field.value === 0) {
        return;
    }
    var padMode = TraktorS2MK1.currentPadMode[field.group];
    if (padMode !== TraktorS2MK1.padModes.sampler) {
        TraktorS2MK1.setPadMode(field.group, TraktorS2MK1.padModes.sampler);
        TraktorS2MK1.controller.setOutput(field.group, "!samples_button", ButtonBrightnessOn, false);
        TraktorS2MK1.controller.setOutput(field.group, "!reset_button", ButtonBrightnessOff, !TraktorS2MK1.batchingLEDUpdate);
    } else {
        TraktorS2MK1.setPadMode(field.group, TraktorS2MK1.padModes.hotcue);
        TraktorS2MK1.controller.setOutput(field.group, "!samples_button", ButtonBrightnessOff, !TraktorS2MK1.batchingLEDUpdate);
    }
};

TraktorS2MK1.introOutroModeButton = function(field) {
    if (field.value === 0) {
        return;
    }
    var padMode = TraktorS2MK1.currentPadMode[field.group];
    if (padMode !== TraktorS2MK1.padModes.introOutro) {
        TraktorS2MK1.setPadMode(field.group, TraktorS2MK1.padModes.introOutro);
        TraktorS2MK1.controller.setOutput(field.group, "!reset_button", ButtonBrightnessOn, false);
        TraktorS2MK1.controller.setOutput(field.group, "!samples_button", ButtonBrightnessOff, !TraktorS2MK1.batchingLEDUpdate);
    } else {
        TraktorS2MK1.setPadMode(field.group, TraktorS2MK1.padModes.hotcue);
        TraktorS2MK1.controller.setOutput(field.group, "!reset_button", ButtonBrightnessOff, !TraktorS2MK1.batchingLEDUpdate);
    }
};

TraktorS2MK1.loopInButton = function(field) {
    engine.setValue(field.group, "loop_in", field.value);
};

TraktorS2MK1.loopOutButton = function(field) {
    engine.setValue(field.group, "loop_out", field.value);
};

// Refer to https://github.com/mixxxdj/mixxx/wiki/standard-effects-mapping for how to use this.
TraktorS2MK1.connectEffectButtonLEDs = function(effectUnitGroup) {
    TraktorS2MK1.effectButtonLEDconnections[effectUnitGroup].forEach(function(connection) {
        connection.disconnect();
    });

    var focusedEffect = engine.getValue(effectUnitGroup, "focused_effect");
    var makeButtonLEDcallback = function(effectNumber) {
        return function(value, _group, _control) {
            TraktorS2MK1.controller.setOutput(effectUnitGroup, "!effectbutton" + effectNumber,
                value === 1 ? ButtonBrightnessOn : ButtonBrightnessOff, !TraktorS2MK1.batchingLEDUpdate);
        };
    };

    // FIXME: Why do the LEDs flicker?
    TraktorS2MK1.batchingLEDUpdate = true;
    for (var i = 0; i <= 2; i++) {
        var effectGroup;
        var key;
        if (focusedEffect === 0) {
            effectGroup = effectUnitGroup.slice(0, -1) + "_Effect" + (i+1) + "]";
            key = "enabled";
        } else {
            effectGroup = effectUnitGroup.slice(0, -1) + "_Effect" + focusedEffect + "]";
            key = "button_parameter" + (i+1);
        }
        TraktorS2MK1.effectButtonLEDconnections[effectUnitGroup][i] = engine.makeConnection(
            effectGroup, key, makeButtonLEDcallback(i+1));
        TraktorS2MK1.effectButtonLEDconnections[effectUnitGroup][i].trigger();
    }
    TraktorS2MK1.batchingLEDUpdate = false;
    TraktorS2MK1.effectButtonLEDconnections[effectUnitGroup][2].trigger();
};

// Refer to https://github.com/mixxxdj/mixxx/wiki/standard-effects-mapping for how to use this.
TraktorS2MK1.onShowParametersChange = function(value, group, _control) {
    if (value === 0) {
        if (engine.getValue(group, "show_focus") > 0) {
            engine.setValue(group, "show_focus", 0);
            TraktorS2MK1.previouslyFocusedEffect[group] = engine.getValue(group, "focused_effect");
            engine.setValue(group, "focused_effect", 0);
        }
    } else {
        engine.setValue(group, "show_focus", 1);
        if (TraktorS2MK1.previouslyFocusedEffect[group] !== null) {
            engine.setValue(group, "focused_effect", TraktorS2MK1.previouslyFocusedEffect[group]);
        }
    }
    TraktorS2MK1.connectEffectButtonLEDs(group);
};

// Refer to https://github.com/mixxxdj/mixxx/wiki/standard-effects-mapping for how to use this.
TraktorS2MK1.onFocusedEffectChange = function(value, group, _control) {
    TraktorS2MK1.controller.setOutput(group, "!effect_focus_button", value > 0 ? ButtonBrightnessOn : ButtonBrightnessOff, !TraktorS2MK1.batchingLEDUpdate);
    if (value === 0) {
        for (var i = 1; i <= 2; i++) {
            // The previously focused effect is not available here, so iterate over all effects' parameter knobs.
            for (var j = 1; j < 3; j++) {
                engine.softTakeoverIgnoreNextValue(group.slice(0, -1) + "_Effect" + i + "]", "parameter" + j);
            }
        }
    } else {
        for (i = 1; i <= 2; i++) {
            engine.softTakeoverIgnoreNextValue(group.slice(0, -1) + "_Effect" + i + "]", "meta");
        }
    }
};

// Refer to https://github.com/mixxxdj/mixxx/wiki/standard-effects-mapping for how to use this.
TraktorS2MK1.effectFocusButton = function(field) {
    var showParameters = engine.getValue(field.group, "show_parameters");
    if (field.value > 0) {
        var effectUnitNumber = field.group.slice(-2, -1);
        if (TraktorS2MK1.shiftPressed["[Channel" + effectUnitNumber + "]"]) {
            engine.setValue(field.group, "load_preset", 1);
            return;
        }
        TraktorS2MK1.effectFocusLongPressTimer[field.group] = engine.beginTimer(TraktorS2MK1.longPressTimeoutMilliseconds, function() {
            TraktorS2MK1.effectFocusChooseModeActive[field.group] = true;
            TraktorS2MK1.effectButtonLEDconnections[field.group].forEach(function(connection) {
                connection.disconnect();
            });
            var makeButtonLEDcallback = function(buttonNumber) {
                return function(value, group, _control) {
                    TraktorS2MK1.controller.setOutput(group, "!effectbutton" + buttonNumber,
                        value === buttonNumber ? ButtonBrightnessOn : ButtonBrightnessOff, !TraktorS2MK1.batchingLEDUpdate);
                };
            };
            TraktorS2MK1.batchingLEDUpdate = true;
            for (var i = 0; i <= 2; i++) {
                TraktorS2MK1.effectButtonLEDconnections[i] = engine.makeConnection(
                    field.group, "focused_effect", makeButtonLEDcallback(i+1));
                TraktorS2MK1.effectButtonLEDconnections[i].trigger();
            }
            TraktorS2MK1.batchingLEDUpdate = false;
            TraktorS2MK1.effectButtonLEDconnections[2].trigger();
        });
        if (!showParameters) {
            engine.setValue(field.group, "show_parameters", 1);
            TraktorS2MK1.effectFocusButtonPressedWhenParametersHidden[field.group] = true;
        }
    } else {
        if (TraktorS2MK1.effectFocusLongPressTimer[field.group] !== 0) {
            engine.stopTimer(TraktorS2MK1.effectFocusLongPressTimer[field.group]);
            TraktorS2MK1.effectFocusLongPressTimer[field.group] = 0;
        }

        if (TraktorS2MK1.effectFocusChooseModeActive[field.group]) {
            TraktorS2MK1.effectFocusChooseModeActive[field.group] = false;
            TraktorS2MK1.connectEffectButtonLEDs(field.group);
        } else if (showParameters && !TraktorS2MK1.effectFocusButtonPressedWhenParametersHidden[field.group]) {
            engine.setValue(field.group, "show_parameters", 0);
        }

        TraktorS2MK1.effectFocusButtonPressedWhenParametersHidden[field.group] = false;
    }
};

// Refer to https://github.com/mixxxdj/mixxx/wiki/standard-effects-mapping for how to use this.
TraktorS2MK1.effectKnob = function(field) {
    var knobNumber = parseInt(field.id.slice(-1));
    var effectUnitGroup = field.group;
    var focusedEffect = engine.getValue(effectUnitGroup, "focused_effect");
    if (focusedEffect > 0) {
        engine.setParameter(effectUnitGroup.slice(0, -1) + "_Effect" + focusedEffect + "]",
            "parameter" + knobNumber,
            field.value / 4095);
    } else {
        engine.setParameter(effectUnitGroup.slice(0, -1) + "_Effect" + knobNumber + "]",
            "meta",
            field.value / 4095);
    }
};

// Refer to https://github.com/mixxxdj/mixxx/wiki/standard-effects-mapping for how to use this.
TraktorS2MK1.effectButton = function(field) {
    var buttonNumber = parseInt(field.id.slice(-1));
    var effectUnitGroup = field.group;
    var effectUnitNumber = field.group.match(script.effectUnitRegEx)[1];
    var focusedEffect = engine.getValue(effectUnitGroup, "focused_effect");

    var toggle = function() {
        var group;
        var key;
        if (focusedEffect === 0) {
            group = effectUnitGroup.slice(0, -1) + "_Effect" + buttonNumber + "]";
            key = "enabled";
        } else {
            group = effectUnitGroup.slice(0, -1) + "_Effect" + focusedEffect + "]";
            key = "button_parameter" + buttonNumber;
        }
        script.toggleControl(group, key);
    };

    if (field.value > 0) {
        if (TraktorS2MK1.shiftPressed["[Channel" + effectUnitNumber + "]"]) {
            engine.setValue(effectUnitGroup, "load_preset", buttonNumber+1);
        } else {
            if (TraktorS2MK1.effectFocusChooseModeActive[effectUnitGroup]) {
                if (focusedEffect === buttonNumber) {
                    engine.setValue(effectUnitGroup, "focused_effect", 0);
                } else {
                    engine.setValue(effectUnitGroup, "focused_effect", buttonNumber);
                }
                TraktorS2MK1.effectFocusChooseModeActive[effectUnitGroup] = false;
            } else {
                toggle();
                TraktorS2MK1.effectButtonLongPressTimer[effectUnitGroup][buttonNumber] =
          engine.beginTimer(TraktorS2MK1.longPressTimeoutMilliseconds,
              function() {
                  TraktorS2MK1.effectButtonIsLongPressed[effectUnitGroup][buttonNumber] = true;
                  TraktorS2MK1.effectButtonLongPressTimer[effectUnitGroup][buttonNumber] = 0;
              },
              true
          );
            }
        }
    } else {
        engine.stopTimer(TraktorS2MK1.effectButtonLongPressTimer[effectUnitGroup][buttonNumber]);
        TraktorS2MK1.effectButtonLongPressTimer[effectUnitGroup][buttonNumber] = 0;
        if (TraktorS2MK1.effectButtonIsLongPressed[effectUnitGroup][buttonNumber]) {
            toggle();
        }
        TraktorS2MK1.effectButtonIsLongPressed[effectUnitGroup][buttonNumber] = false;
    }
};

/// return value 1 === right turn
/// return value -1 === left turn
TraktorS2MK1.encoderDirection = function(newValue, oldValue) {
    var direction = 0;
    var min = 0;
    var max = 15;
    if (oldValue === max && newValue === min) {
        direction = 1;
    } else if (oldValue === min && newValue === max) {
        direction = -1;
    } else if (newValue > oldValue) {
        direction = 1;
    } else {
        direction = -1;
    }
    return direction;
};

TraktorS2MK1.gainEncoder = function(field) {
    var delta = 0.03333 * TraktorS2MK1.encoderDirection(field.value, TraktorS2MK1.previousPregain[field.group]);
    TraktorS2MK1.previousPregain[field.group] = field.value;

    if (TraktorS2MK1.shiftPressed[field.group]) {
        var currentPregain = engine.getParameter(field.group, "pregain");
        engine.setParameter(field.group, "pregain", currentPregain + delta);
    } else {
        var quickEffectGroup = "[QuickEffectRack1_" + field.group + "]";
        if (TraktorS2MK1.gainEncoderPressed[field.group]) {
            script.triggerControl(quickEffectGroup, delta > 0 ? "next_chain" : "prev_chain");
        } else {
            var currentQuickEffectSuperKnob = engine.getParameter(quickEffectGroup, "super1");
            engine.setParameter(quickEffectGroup, "super1", currentQuickEffectSuperKnob + delta);
        }
    }
};

TraktorS2MK1.gainEncoderPress = function(field) {
    if (field.value > 0) {
        TraktorS2MK1.gainEncoderPressed[field.group] = true;
        if (TraktorS2MK1.shiftPressed[field.group]) {
            script.triggerControl(field.group, "pregain_set_default");
        } else {
            script.triggerControl("[QuickEffectRack1_" + field.group + "]", "super1_set_default");
        }
    } else {
        TraktorS2MK1.gainEncoderPressed[field.group] = false;
    }
};

TraktorS2MK1.leftEncoder = function(field) {
    var delta = TraktorS2MK1.encoderDirection(field.value, TraktorS2MK1.previousLeftEncoder[field.group]);
    TraktorS2MK1.previousLeftEncoder[field.group] = field.value;

    if (TraktorS2MK1.shiftPressed[field.group]) {
        if (delta === 1) {
            script.triggerControl(field.group, "pitch_up_small");
        } else {
            script.triggerControl(field.group, "pitch_down_small");
        }
    } else {
        if (TraktorS2MK1.leftEncoderPressed[field.group]) {
            var beatjumpSize = engine.getValue(field.group, "beatjump_size");
            if (delta === 1) {
                beatjumpSize *= 2;
            } else {
                beatjumpSize /= 2;
            }
            engine.setValue(field.group, "beatjump_size", beatjumpSize);
        } else {
            if (delta === 1) {
                script.triggerControl(field.group, "beatjump_forward");
            } else {
                script.triggerControl(field.group, "beatjump_backward");
            }
        }
    }
};

TraktorS2MK1.leftEncoderPress = function(field) {
    TraktorS2MK1.leftEncoderPressed[field.group] = (field.value > 0);
    if (TraktorS2MK1.shiftPressed[field.group] && field.value > 0) {
        script.triggerControl(field.group, "pitch_adjust_set_default");
    }
};

TraktorS2MK1.rightEncoder = function(field) {
    var delta = TraktorS2MK1.encoderDirection(field.value, TraktorS2MK1.previousRightEncoder[field.group]);
    TraktorS2MK1.previousRightEncoder[field.group] = field.value;

    if (TraktorS2MK1.shiftPressed[field.group]) {
        if (delta === 1) {
            script.triggerControl(field.group, "beatjump_1_forward");
        } else {
            script.triggerControl(field.group, "beatjump_1_backward");
        }
    } else {
        if (delta === 1) {
            script.triggerControl(field.group, "loop_double");
        } else {
            script.triggerControl(field.group, "loop_halve");
        }
    }
};

TraktorS2MK1.rightEncoderPress = function(field) {
    if (field.value === 0) {
        return;
    }
    var loopEnabled = engine.getValue(field.group, "loop_enabled");
    // The actions triggered below change the state of loop_enabled,
    // so to simplify the logic, use script.triggerControl to only act
    // on press rather than resetting ControlObjects to 0 on release.
    if (TraktorS2MK1.shiftPressed[field.group]) {
        if (loopEnabled) {
            script.triggerControl(field.group, "reloop_andstop");
        } else {
            script.triggerControl(field.group, "reloop_toggle");
        }
    } else {
        if (loopEnabled) {
            script.triggerControl(field.group, "reloop_toggle");
        } else {
            script.triggerControl(field.group, "beatloop_activate");
        }
    }
};

TraktorS2MK1.browseEncoder = function(field) {
    var delta = TraktorS2MK1.encoderDirection(field.value, TraktorS2MK1.previousBrowse);
    TraktorS2MK1.previousBrowse = field.value;

    if (TraktorS2MK1.shiftPressed["[Channel1]"] || TraktorS2MK1.shiftPressed["[Channel2]"]) {
        delta *= 5;
    }
    engine.setValue("[Playlist]", "SelectTrackKnob", delta);
};

TraktorS2MK1.scalerParameter = function(group, name, value) {
    return script.absoluteLin(value, 0, 1, 16, 4080);
};

TraktorS2MK1.scalerVolume = function(group, name, value) {
    if (group === "[Master]") {
        return script.absoluteNonLin(value, 0, 1, 4, 16, 4080);
    } else {
        return script.absoluteNonLin(value, 0, 0.25, 1, 16, 4080);
    }
};

TraktorS2MK1.scalerSlider = function(group, name, value) {
    return script.absoluteLin(value, -1, 1, 16, 4080);
};

TraktorS2MK1.outputChannelCallback = function(value, group, key) {
    var ledValue = 0x05;
    if (value) {
        ledValue = 0x1F;
    }
    TraktorS2MK1.controller.setOutput(group, key, ledValue, !TraktorS2MK1.batchingLEDUpdate);
};

TraktorS2MK1.outputChannelCallbackDark = function(value, group, key) {
    var ledValue = 0x00;
    if (value) {
        ledValue = 0x1F;
    }
    TraktorS2MK1.controller.setOutput(group, key, ledValue, !TraktorS2MK1.batchingLEDUpdate);
};

TraktorS2MK1.outputCallback = function(value, group, key) {
    var ledValue = ButtonBrightnessOff;
    if (value) {
        ledValue = ButtonBrightnessOn;
    }
    TraktorS2MK1.controller.setOutput(group, key, ledValue, !TraktorS2MK1.batchingLEDUpdate);
};

TraktorS2MK1.outputCallbackLoop = function(value, group, key) {
    var ledValue = ButtonBrightnessOff;
    if (engine.getValue(group, "loop_enabled")) {
        ledValue = 0x1F;
    }
    TraktorS2MK1.controller.setOutput(group, key, ledValue, !TraktorS2MK1.batchingLEDUpdate);
};

TraktorS2MK1.outputCallbackDark = function(value, group, key) {
    var ledValue = 0x00;
    if (value) {
        ledValue = 0x1F;
    }
    TraktorS2MK1.controller.setOutput(group, key, ledValue, !TraktorS2MK1.batchingLEDUpdate);
};

TraktorS2MK1.pflButton = function(field) {
    if (field.value > 0) {
        if (TraktorS2MK1.shiftPressed[field.group]) {
            script.toggleControl(field.group, "quantize");
        } else {
            script.toggleControl(field.group, "pfl");
        }
    }
};

TraktorS2MK1.sendPadColor = function(group, padNumber, color) {
    var padKey = "!pad_" + padNumber + "_";
    var ColorBrightnessScaler = ButtonBrightnessOn / 0x1f;
    var green = color.green * ColorBrightnessScaler;
    var blue = color.blue * ColorBrightnessScaler;
    if (color.green === 0 && color.blue === 0) {
        green = ButtonBrightnessOff;
        blue = ButtonBrightnessOff;
    }
    TraktorS2MK1.controller.setOutput(group, padKey + "G", green, false);
    TraktorS2MK1.controller.setOutput(group, padKey + "B", blue, !TraktorS2MK1.batchingLEDUpdate);
};

TraktorS2MK1.outputHotcueCallback = function(value, group, key) {
    var hotcueNumber = key.charAt(7);
    var color;
    if (engine.getValue(group, "hotcue_" + hotcueNumber + "_enabled")) {
        color = {green: 0, blue: 0x1F};
    } else {
        color = {green: 0, blue: 0};
    }
    TraktorS2MK1.sendPadColor(group, hotcueNumber, color);
};

TraktorS2MK1.onVuMeterChanged = function(value, group, _key) {
    // This handler is called a lot so it should be as fast as possible.

    // Figure out number of fully-illuminated segments.
    var scaledValue = value * 4.0;
    var fullIllumCount = Math.floor(scaledValue);

    // Figure out how much the partially-illuminated segment is illuminated.
    var partialIllum = (scaledValue - fullIllumCount) * 0x1F;

    for (var i = 0; i <= 3; i++) {
        var key = "!VuMeter" + i;
        if (i < fullIllumCount) {
            // Don't update lights until they're all done, so the last term is false.
            TraktorS2MK1.controller.setOutput(group, key, 0x1F, false);
        } else if (i === fullIllumCount) {
            TraktorS2MK1.controller.setOutput(group, key, partialIllum, false);
        } else {
            TraktorS2MK1.controller.setOutput(group, key, 0x00, false);
        }
    }
    TraktorS2MK1.controller.OutputPackets["OutputReport_0x80"].send();
};

TraktorS2MK1.onLoopEnabledChanged = function(value, group, _key) {
    TraktorS2MK1.outputCallbackLoop(value, group, "loop_in");
    TraktorS2MK1.outputCallbackLoop(value, group, "loop_out");
};

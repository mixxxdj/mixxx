/////////////////////////////////////////////////////////////////////////////////////////
// JSHint configuration                                                                //
/////////////////////////////////////////////////////////////////////////////////////////
/* jshint -W016                                                                        */
/////////////////////////////////////////////////////////////////////////////////////////
/*                                                                                     */
/* Traktor Kontrol MX2 HID controller script v1.00                                     */
/* Last modification: January 2026                                                     */
/* Author: K7                                                                          */
/* https://manual.mixxx.org/2.6/en/hardware/controllers/native_instruments_traktor_mx2 */
/*                                                                                     */
/////////////////////////////////////////////////////////////////////////////////////////

// Constants used to scale raw velocity (tick delta / time delta) to the appropriate scratch2 value.
const TICKS_PER_REV = 1024;
const JOGWHEEL_CLOCK_HZ = 100000000;
const TARGET_RPM = 33 + 1 / 3;
const VELOCITY_TO_SCRATCH = JOGWHEEL_CLOCK_HZ / (TICKS_PER_REV * TARGET_RPM / 60);

// Affects how sensitive jogging/nudging (turning the wheel without touching the top) is.
// A constant of 0.5 makes jogging/nudging roughly equivalent to scratching.
const JOG_SENSITIVITY = 0.5;
const VELOCITY_TO_JOG = VELOCITY_TO_SCRATCH * JOG_SENSITIVITY;

// Interval (ms) at which jog velocity is polled after release to determine whether scratching should stop.
const JOGWHEEL_STOP_POLL_TIME = 20;

// Interval (ms) at which jog velocity is reduced stepwise after release.
const JOGWHEEL_DECAY_POLL_TIME = 20;

// Alpha for the jogwheel input's low pass filter.
// Range: 0-1. Lower = more smoothing.
const JOGWHEEL_ALPHA = 0.5;

// Threshold for the jogwheel input's dead zone. When the raw velocity is lower than this,
// the jogwheel is considered to be stopped, allowing it to change directions or exit scratching mode instantly.
const JOGWHEEL_EPSILON = 0.001;

var TraktorMX2 = new (function () {
    this.controller = new HIDController();

    this.shiftPressed = {"[Channel1]": false, "[Channel2]": false};

    this.jogModeState = {"[Channel1]": 0, "[Channel2]": 0}; // 0 = Scratch, 1 = Bend
    this.jogTimer = {"[Channel1]": 0, "[Channel2]": 0};

    this.padModeState = {"[Channel1]": 0, "[Channel2]": 0}; // 0 = Hotcues, 1 = Stems, 2 = Patterns, 3 = Loops
    this.padPressed = {
        "[Channel1]": {5: false, 6: false, 7: false, 8: false}, "[Channel2]": {5: false, 6: false, 7: false, 8: false}
    }; // State of pads for Stems mode

    this.bottomLedState = {
        "[Channel1]": {1: 0x00, 2: 0x00, 3: 0x00, 4: 0x00, 5: 0x00, 6: 0x00},
        "[Channel2]": {1: 0x00, 2: 0x00, 3: 0x00, 4: 0x00, 5: 0x00, 6: 0x00}
    };

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
    this.lastTickVal = [0, 0];
    this.lastTimestamp = [0, 0];
    this.lastVelocity = [0.0, 0.0];
    this.lastWallClock = [0, 0];
    this.jogStopTimerId = [null, null];
    this.jogDecayTimerId = [null, null];

    // VuMeter
    this.vuLeftConnection = {};
    this.vuRightConnection = {};
    this.clipLeftConnection = {};
    this.clipRightConnection = {};
    this.clipMainConnection = {};
    this.vuMeterThresholds = {
        "vu-18": 1 / 9,
        "vu-15": 2 / 9,
        "vu-12": 3 / 9,
        "vu-9": 4 / 9,
        "vu-6": 5 / 9,
        "vu-3": 6 / 9,
        vu0: 7 / 9,
        vu6: 8 / 9,
    };
})();

TraktorMX2.init = function (_id) {
    TraktorMX2.registerInputPackets();
    TraktorMX2.registerOutputPackets();
    console.log("TraktorMX2: Init done!");
};

TraktorMX2.registerInputPackets = function () {
    const messageShort = new HIDPacket("shortmessage", 0x01, this.messageCallback);
    const messageLong = new HIDPacket("longmessage", 0x02, this.messageCallback);
    const messageJog = new HIDPacket("reportmessage", 0x03, this.messageCallback);


    // Channel 1

    // // Channel FX
    this.registerInputButton(messageShort, "EffectUnit1", "misc", 0x01, 0x01, this.fxHandler);
    this.registerInputButton(messageShort, "EffectUnit1", "Effect1", 0x01, 0x02, this.fxHandler);
    this.registerInputButton(messageShort, "EffectUnit1", "Effect2", 0x01, 0x04, this.fxHandler);
    this.registerInputButton(messageShort, "EffectUnit1", "Effect3", 0x01, 0x08, this.fxHandler);

    // // Library Controls
    this.registerInputButton(messageShort, "[Channel1]", "!FavList", 0x01, 0x10, this.favListHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!AddTrack", 0x01, 0x20, this.addTrackHandler);
    this.registerInputButton(messageShort, "[PreviewDeck1]", "!PreviewL", 0x01, 0x40, this.previewHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!MaximizeLibrary", 0x01, 0x80, this.maximizeLibraryHandler);

    // // Rev / FLUX
    this.registerInputButton(messageShort, "[Channel1]", "!slip_enabled", 0x02, 0x01, this.fluxHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!reverse", 0x02, 0x02, this.reverseHandler);

    // // Jog Mode
    this.registerInputButton(messageShort, "[Channel1]", "!tt", 0x02, 0x04, this.jogModeHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!jog", 0x02, 0x08, this.jogModeHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!shift", 0x02, 0x10, this.shiftHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!sync", 0x02, 0x20, this.syncHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!sync_leader", 0x02, 0x40, this.masterHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!keylock", 0x02, 0x80, this.keylockHandler);

    // // Hotcue / Stem / Pattern / Loop Mode
    this.registerInputButton(messageShort, "[Channel1]", "!hotcues", 0x03, 0x01, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!stems", 0x03, 0x02, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!patterns", 0x03, 0x04, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!loops", 0x03, 0x08, this.padModeHandler);

    // // Pads (Hotcues, Stems, Patterns and Loops depending on current mode)
    this.registerInputButton(messageShort, "[Channel1]", "!pad_1", 0x03, 0x10, this.padHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_2", 0x03, 0x20, this.padHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_3", 0x03, 0x40, this.padHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_4", 0x03, 0x80, this.padHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_5", 0x04, 0x01, this.padHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_6", 0x04, 0x02, this.padHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_7", 0x04, 0x04, this.padHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pad_8", 0x04, 0x08, this.padHandler);

    // // Cue and Play
    this.registerInputButton(messageShort, "[Channel1]", "!cue_default", 0x04, 0x10, this.cueHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!play", 0x04, 0x20, this.playHandler);


    // Channel 2

    // // Channel FX
    this.registerInputButton(messageShort, "EffectUnit2", "misc", 0x04, 0x40, this.fxHandler);
    this.registerInputButton(messageShort, "EffectUnit2", "Effect1", 0x04, 0x80, this.fxHandler);
    this.registerInputButton(messageShort, "EffectUnit2", "Effect2", 0x05, 0x01, this.fxHandler);
    this.registerInputButton(messageShort, "EffectUnit2", "Effect3", 0x05, 0x02, this.fxHandler);

    // // Library Controls
    this.registerInputButton(messageShort, "[Channel2]", "!FavList", 0x05, 0x04, this.favListHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!AddTrack", 0x05, 0x08, this.addTrackHandler);
    this.registerInputButton(messageShort, "[PreviewDeck1]", "!PreviewR", 0x05, 0x10, this.previewHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!MaximizeLibrary", 0x05, 0x20, this.maximizeLibraryHandler);

    // // Rev / FLUX
    this.registerInputButton(messageShort, "[Channel2]", "!slip_enabled", 0x05, 0x40, this.fluxHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!reverse", 0x05, 0x80, this.reverseHandler);

    // // Jog Mode
    this.registerInputButton(messageShort, "[Channel2]", "!tt", 0x06, 0x01, this.jogModeHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!jog", 0x06, 0x02, this.jogModeHandler);

    // // Shift, Sync, Sync Master, Keylock
    this.registerInputButton(messageShort, "[Channel2]", "!shift", 0x06, 0x04, this.shiftHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!sync", 0x06, 0x08, this.syncHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!sync_leader", 0x06, 0x10, this.masterHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!keylock", 0x06, 0x20, this.keylockHandler);

    // // Hotcue / Stem / Pattern / Loop Mode
    this.registerInputButton(messageShort, "[Channel2]", "!hotcues", 0x06, 0x40, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!stems", 0x06, 0x80, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!patterns", 0x07, 0x01, this.padModeHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!loops", 0x07, 0x02, this.padModeHandler);

    // // Pads (Hotcues, Stems, Patterns and Loops depending on current mode)
    this.registerInputButton(messageShort, "[Channel2]", "!pad_1", 0x07, 0x04, this.padHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_2", 0x07, 0x08, this.padHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_3", 0x07, 0x10, this.padHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_4", 0x07, 0x20, this.padHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_5", 0x07, 0x40, this.padHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_6", 0x07, 0x80, this.padHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_7", 0x08, 0x01, this.padHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pad_8", 0x08, 0x02, this.padHandler);

    // // Cue and Play
    this.registerInputButton(messageShort, "[Channel2]", "!cue_default", 0x08, 0x04, this.cueHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!play", 0x08, 0x08, this.playHandler);


    // Mixer and global Effects

    // // Channel 1 FX Select and GFX / PFL
    this.registerInputButton(messageShort, "[Channel1]", "EffectUnit1", 0x08, 0x10, this.fxSelectHandler);
    this.registerInputButton(messageShort, "[Channel1]", "EffectUnit2", 0x08, 0x20, this.fxSelectHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!gfx_toggle", 0x08, 0x40, this.gfxToggleHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!pfl", 0x08, 0x80, this.headphoneHandler);

    // // Channel 2 FX Select and GFX / PFL
    this.registerInputButton(messageShort, "[Channel2]", "EffectUnit1", 0x09, 0x01, this.fxSelectHandler);
    this.registerInputButton(messageShort, "[Channel2]", "EffectUnit2", 0x09, 0x02, this.fxSelectHandler);

    this.registerInputButton(messageShort, "[Channel2]", "!gfx_toggle", 0x09, 0x04, this.gfxToggleHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!pfl", 0x09, 0x08, this.headphoneHandler);

    // // GFX
    this.registerInputButton(messageShort, "[ChannelX]", "!gfx0", 0x0a, 0x01, this.globalfxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!gfx1", 0x09, 0x10, this.globalfxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!gfx2", 0x09, 0x20, this.globalfxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!gfx3", 0x09, 0x40, this.globalfxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!gfx4", 0x09, 0x80, this.globalfxHandler);

    // // Microphone
    this.registerInputButton(messageShort, "[Microphone]", "!talkover", 0x0a, 0x02, this.microphoneHandler);


    // Other

    // // Selectors Press
    this.registerInputButton(messageShort, "[Channel1]", "!LoadSelectedTrack", 0x0a, 0x04, this.loadTrackHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!ActivateBeatjump", 0x0a, 0x08, this.activateBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!ActivateLoop", 0x0a, 0x10, this.activateLoopHandler);

    this.registerInputButton(messageShort, "[Channel2]", "!LoadSelectedTrack", 0x0a, 0x20, this.loadTrackHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!ActivateBeatjump", 0x0a, 0x40, this.activateBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!ActivateLoop", 0x0a, 0x80, this.activateLoopHandler);


    // // Jog Touch
    this.registerInputButton(messageShort, "[Channel1]", "!jog_touch", 0x0b, 0x01, this.jogTouchHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!jog_touch", 0x0b, 0x02, this.jogTouchHandler);

    // // Selectors Turn
    this.registerInputButton(messageShort, "[Channel1]", "!SelectTrack", 0x0c, 0x0f, this.selectTrackHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!SelectBeatjump", 0x0c, 0xf0, this.selectBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel1]", "!SelectLoop", 0x0d, 0x0f, this.selectLoopHandler);

    this.registerInputButton(messageShort, "[Channel2]", "!SelectTrack", 0x0d, 0xf0, this.selectTrackHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!SelectBeatjump", 0x0e, 0x0f, this.selectBeatjumpHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!SelectLoop", 0x0e, 0xf0, this.selectLoopHandler);

    this.controller.registerInputPacket(messageShort);


    this.registerInputScaler(messageLong, "[Channel1]", "pregain", 0x11, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "pregain", 0x1b, 0xffff, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Channel1]", "volume", 0x2b, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "volume", 0x2d, 0xffff, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Channel1]", "rate", 0x31, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "rate", 0x33, 0xffff, this.parameterHandler);

    // FX Parameter
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit1]", "mix", 0x01, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit1_Effect1]", "meta", 0x03, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit1_Effect2]", "meta", 0x05, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit1_Effect3]", "meta", 0x07, 0xffff, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit2]", "mix", 0x09, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit2_Effect1]", "meta", 0x0b, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit2_Effect2]", "meta", 0x0d, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit2_Effect3]", "meta", 0x0f, 0xffff, this.parameterHandler);

    // EQ Parameter
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x13, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x15, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x17, 0xffff, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x1d, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x1f, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x21, 0xffff, this.parameterHandler);

    // Global FX Parameter
    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel1]]", "super1", 0x19, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel2]]", "super1", 0x23, 0xffff, this.parameterHandler);

    // Master
    this.registerInputScaler(messageLong, "[Master]", "crossfader", 0x2f, 0xffff, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Master]", "headMix", 0x27, 0xffff, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headGain", 0x29, 0xffff, this.parameterHandler);

    this.controller.registerInputPacket(messageLong);

    this.registerInputJog(messageJog, "[Channel1]", "jog_timer", 0x04, 0xffffffff, this.jogHandler);
    this.registerInputJog(messageJog, "[Channel1]", "jog_wheel", 0x08, 0xffffffff, this.jogHandler);

    this.registerInputJog(messageJog, "[Channel2]", "jog_timer", 0x0c, 0xffffffff, this.jogHandler);
    this.registerInputJog(messageJog, "[Channel2]", "jog_wheel", 0x10, 0xffffffff, this.jogHandler);

    this.controller.registerInputPacket(messageJog);


    // Dirty hack to set initial values in the packet parser
    const data = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    TraktorMX2.incomingData(data);
};

TraktorMX2.registerInputJog = function (message, group, name, offset, bitmask, callback) {
    // Jog wheels have 4 byte input
    message.addControl(group, name, offset, "I", bitmask);
    message.setCallback(group, name, callback);
};

TraktorMX2.registerInputScaler = function (message, group, name, offset, bitmask, callback) {
    message.addControl(group, name, offset, "H", bitmask);
    message.setCallback(group, name, callback);
};

TraktorMX2.registerInputButton = function (message, group, name, offset, bitmask, callback) {
    message.addControl(group, name, offset, "B", bitmask);
    message.setCallback(group, name, callback);
};

TraktorMX2.playHandler = function (field) {
    if (TraktorMX2.shiftPressed[field.group]) {
        engine.setValue(field.group, "start_stop", field.value);
    } else if (field.value === 1) {
        script.toggleControl(field.group, "play");
    }
};

TraktorMX2.cueHandler = function (field) {
    if (TraktorMX2.shiftPressed[field.group]) {
        engine.setValue(field.group, "cue_gotoandstop", field.value);
    } else {
        engine.setValue(field.group, "cue_default", field.value);
    }
};

TraktorMX2.shiftHandler = function (field) {
    TraktorMX2.shiftPressed[field.group] = field.value;
    engine.setValue("[Controls]", "touch_shift", field.value);
};

TraktorMX2.keylockHandler = function (field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "keylock");
};

TraktorMX2.masterHandler = function (field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "sync_leader");
};

TraktorMX2.syncHandler = function (field) {
    if (TraktorMX2.shiftPressed[field.group]) {
        engine.setValue(field.group, "beatsync_phase", field.value);
        // Light LED while pressed
        TraktorMX2.outputHandler(field.value, field.group, "sync_enabled");
    } else {
        if (field.value) {
            if (engine.getValue(field.group, "sync_enabled") === 0) {
                script.triggerControl(field.group, "beatsync");
                // Start timer to measure how long button is pressed
                TraktorMX2.syncPressedTimer[field.group] = engine.beginTimer(300, () => {
                    engine.setValue(field.group, "sync_enabled", 1);
                    // Reset sync button timer state if active
                    if (TraktorMX2.syncPressedTimer[field.group] !== 0) {
                        TraktorMX2.syncPressedTimer[field.group] = 0;
                    }
                }, true);

                // Light corresponding LED when button is pressed
                TraktorMX2.outputHandler(1, field.group, "sync_enabled");
            } else {
                // Deactivate sync lock
                // LED is turned off by the callback handler for sync_enabled
                engine.setValue(field.group, "sync_enabled", 0);
            }
        } else {
            if (TraktorMX2.syncPressedTimer[field.group] !== 0) {
                // Timer still running -> stop it and unlight LED
                engine.stopTimer(TraktorMX2.syncPressedTimer[field.group]);
                TraktorMX2.outputHandler(0, field.group, "sync_enabled");
            }
        }
    }
};

TraktorMX2.padModeHandler = function (field) {

    if (field.value === 0) {
        return;
    }

    switch (field.name) {

        case "!hotcues":
            TraktorMX2.padModeState[field.group] = 0;
            TraktorMX2.outputHandler(1, field.group, "hotcues");
            TraktorMX2.outputHandler(0, field.group, "stems");
            TraktorMX2.outputHandler(0, field.group, "patterns");
            TraktorMX2.outputHandler(0, field.group, "loops");
        // Light LEDs (blue for all enabled hotcues, dimmed white for disabled)
            for (let i = 1; i <= 8; ++i) {
                const active = engine.getValue(field.group, `hotcue_${i}_status`);
                if (active) {
                    const color = engine.getValue(field.group, `hotcue_${i}_color`);
                    const colorValue = TraktorMX2.PadColorMap.getValueForNearestColor(color);
                    TraktorMX2.outputHandler(colorValue, field.group, "pad_" + i);
                } else {
                    TraktorMX2.outputHandler(TraktorMX2.baseColors.dimmedWhite, field.group, `pad_${i}`);
                }
            }
            break;

        case "!stems":
            TraktorMX2.padModeState[field.group] = 1;
            TraktorMX2.outputHandler(0, field.group, "hotcues");
            TraktorMX2.outputHandler(1, field.group, "stems");
            TraktorMX2.outputHandler(0, field.group, "patterns");
            TraktorMX2.outputHandler(0, field.group, "loops");
        // Light LEDs (stem color for all unmuted stems, dimmed red for muted)
            for (let i = 1; i <= engine.getValue(field.group, "stem_count"); i++) {
                const color = engine.getValue(`[Channel${field.group[field.group.length - 2]}_Stem${i}]`, "color");
                const status = engine.getValue(`[Channel${field.group[field.group.length - 2]}_Stem${i}]`, "mute");
                const colorValue = status ? TraktorMX2.baseColors.dimmedRed : TraktorMX2.PadColorMap.getValueForNearestColor(color);
                TraktorMX2.outputHandler(colorValue, field.group, "pad_" + i);
            }
            break;

        case "!patterns":
        // Patterns mode not implemented yet -> does nothing except lighting
            TraktorMX2.padModeState[field.group] = 2;
            TraktorMX2.outputHandler(0, field.group, "hotcues");
            TraktorMX2.outputHandler(0, field.group, "stems");
            TraktorMX2.outputHandler(1, field.group, "patterns");
            TraktorMX2.outputHandler(0, field.group, "loops");
            // Turn off LEDs
            for (let i = 1; i <= 8; ++i) {
                TraktorMX2.outputHandler(0x00, field.group, "pad_" + i);
            }
            break;

        case "!loops":
            TraktorMX2.padModeState[field.group] = 3;
            TraktorMX2.outputHandler(0, field.group, "hotcues");
            TraktorMX2.outputHandler(0, field.group, "stems");
            TraktorMX2.outputHandler(0, field.group, "patterns");
            TraktorMX2.outputHandler(1, field.group, "loops");
        // Turn LEDs green
            for (let i = 1; i <= 8; ++i) {
                TraktorMX2.outputHandler(TraktorMX2.baseColors.green, field.group, "pad_" + i);
            }
            break;
    }

};

TraktorMX2.padHandler = function (field) {
    const padNumber = parseInt(field.id[field.id.length - 1]);
    switch (TraktorMX2.padModeState[field.group]) {
        case 0:
            // Hotcues mode
            if (TraktorMX2.shiftPressed[field.group]) {
                engine.setValue(field.group, `hotcue_${padNumber}_clear`, field.value);
            } else {
                engine.setValue(field.group, `hotcue_${padNumber}_activate`, field.value);
            }

            break;

        case 1:
            // Stems Mode
            // ignore if no stemfile is loaded
            if (engine.getValue(field.group, "stem_count") === 0) {
                return;
            }

            // only first 4 pads are used for stem mute/unmute
            if (padNumber <= Math.min(4, engine.getValue(field.group, "stem_count"))) {
                if (field.value === 0) {
                    return;
                }
                script.toggleControl(`[Channel${field.group[field.group.length - 2]}_Stem${padNumber}]`, "mute");

                const color = engine.getValue(`[Channel${field.group[field.group.length - 2]}_Stem${padNumber}]`, "color");
                const status = engine.getValue(`[Channel${field.group[field.group.length - 2]}_Stem${padNumber}]`, "mute");
                const colorValue = status ? TraktorMX2.baseColors.dimmedRed : TraktorMX2.PadColorMap.getValueForNearestColor(color);
                TraktorMX2.outputHandler(colorValue, field.group, "pad_" + padNumber);

            } else {
                // pads 5-8 are used for volume and filter control of the stems
                TraktorMX2.padPressed[field.group][padNumber] = field.value;
            }
            break;

        case 2:
            // Patterns Mode
            break;

        case 3:
            // Loops Mode

            if (TraktorMX2.shiftPressed[field.group]) {
                engine.setValue(field.group, "beatlooproll_" + (2 ** (padNumber - 5)) + "_activate", field.value);
            } else {
                engine.setValue(field.group, "beatloop_" + 2 ** ((padNumber - 5)) + "_toggle", 1);
            }
            break;
    }
};

TraktorMX2.headphoneHandler = function (field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl(field.group, "pfl");
    TraktorMX2.outputHandler(engine.getValue(field.group, "pfl"), field.group, "pfl");
};

TraktorMX2.selectTrackHandler = function (field) {
    let delta = 1;
    if ((field.value + 1) % 16 === TraktorMX2.browseKnobEncoderState[field.group]) {
        delta = -1;
    }

    if (TraktorMX2.shiftPressed[field.group]) {
        engine.setValue("[Library]", "focused_widget", 2);
        engine.setValue("[Library]", "MoveVertical", delta);
    } else {
        engine.setValue("[Library]", "focused_widget", 3);
        engine.setValue("[Library]", "MoveVertical", delta);
    }

    TraktorMX2.browseKnobEncoderState[field.group] = field.value;
};

TraktorMX2.loadTrackHandler = function (field) {
    if (TraktorMX2.shiftPressed[field.group]) {
        engine.setValue("[Library]", "GoToItem", field.value);
    } else {
        engine.setValue(field.group, "LoadSelectedTrack", field.value);
    }
};

TraktorMX2.addTrackHandler = function (field) {
    TraktorMX2.outputHandler(field.value, field.group, "addTrack");

    if (TraktorMX2.shiftPressed[field.group]) {
        engine.setValue("[Library]", "AutoDjAddTop", field.value);
    } else {
        engine.setValue("[Library]", "AutoDjAddBottom", field.value);
    }
};

TraktorMX2.previewHandler = function (field) {
    if (field.value === 0) {
        return;
    }

    engine.setValue(field.group, "LoadSelectedTrackAndPlay", field.value);
};

TraktorMX2.favListHandler = function (field) {
    if (field.value === 0) {
        return;
    }
};

TraktorMX2.maximizeLibraryHandler = function (field) {
    if (field.value === 0) {
        return;
    }

    let group = "[Skin]"
    let control = "show_maximized_library"

    script.toggleControl(group, control);
    TraktorMX2.outputHandler(engine.getValue(group, control), "[Channel1]", "maximizeLibrary");
    TraktorMX2.outputHandler(engine.getValue(group, control), "[Channel2]", "maximizeLibrary");
};

TraktorMX2.selectLoopHandler = function (field) {

    let delta = 1;
    if ((field.value + 1) % 16 === TraktorMX2.loopKnobEncoderState[field.group]) {
        delta = -1;
    }

    if (TraktorMX2.padModeState[field.group] === 1 && Object.values(TraktorMX2.padPressed[field.group]).reduce((v, a) => v || a, false)) {
        // Change Functionality in Stems Mode with pressed Pad 5-8
        for (const padNum in TraktorMX2.padPressed[field.group]) {
            if (TraktorMX2.padPressed[field.group][padNum]) {

                if (!TraktorMX2.shiftPressed[field.group]) {
                    if (delta > 0) {
                        script.triggerControl("[QuickEffectRack1_[Channel" + field.group[field.group.length - 2] + "_Stem" + (padNum - 4) + "]]", "super1_up");
                    } else {
                        script.triggerControl("[QuickEffectRack1_[Channel" + field.group[field.group.length - 2] + "_Stem" + (padNum - 4) + "]]", "super1_down");
                    }
                } else {
                    if (delta > 0) {
                        engine.setValue("[QuickEffectRack1_[Channel" + field.group[field.group.length - 2] + "_Stem" + (padNum - 4) + "]]", "next_chain_preset", 1);
                    } else {
                        engine.setValue("[QuickEffectRack1_[Channel" + field.group[field.group.length - 2] + "_Stem" + (padNum - 4) + "]]", "prev_chain_preset", 1);
                    }
                }

            }
        }
    } else {

        if ((field.value + 1) % 16 === TraktorMX2.loopKnobEncoderState[field.group]) {
            script.triggerControl(field.group, "loop_halve");
        } else {
            script.triggerControl(field.group, "loop_double");
        }
    }

    TraktorMX2.loopKnobEncoderState[field.group] = field.value;
};

TraktorMX2.activateLoopHandler = function (field) {
    if (field.value === 0) {
        return;
    }

    if (TraktorMX2.padModeState[field.group] === 1 && Object.values(TraktorMX2.padPressed[field.group]).reduce((v, a) => v || a, false)) {
        // Change Functionality in Stems Mode with pressed any of Pad 5-8
        for (const padNum in TraktorMX2.padPressed[field.group]) {
            if (TraktorMX2.padPressed[field.group][padNum]) {
                script.toggleControl("[QuickEffectRack1_[Channel" + field.group[field.group.length - 2] + "_Stem" + (padNum - 4) + "]]", "enabled");
            }
        }
    } else {
        //Default Functionality
        if (TraktorMX2.shiftPressed[field.group]) {
            engine.setValue(field.group, "reloop_toggle", field.value);
        } else {
            engine.setValue(field.group, "beatloop_activate", field.value);
        }
    }
};

TraktorMX2.selectBeatjumpHandler = function (field) {
    let delta = 1;
    if ((field.value + 1) % 16 === TraktorMX2.moveKnobEncoderState[field.group]) {
        delta = -1;
    }

    if (TraktorMX2.padModeState[field.group] === 1 && Object.values(TraktorMX2.padPressed[field.group]).reduce((v, a) => v || a, false)) {
        // Change Functionality in Stems Mode with pressed any of Pad 5-8
        for (const padNum in TraktorMX2.padPressed[field.group]) {
            if (TraktorMX2.padPressed[field.group][padNum]) {
                if (delta > 0) {
                    script.triggerControl("[Channel" + field.group[field.group.length - 2] + "_Stem" + (padNum - 4) + "]", "volume_up");
                } else {
                    script.triggerControl("[Channel" + field.group[field.group.length - 2] + "_Stem" + (padNum - 4) + "]", "volume_down");
                }

            }
        }

    } else {
        //Default Functionality
        if (TraktorMX2.shiftPressed[field.group]) {
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
    }

    TraktorMX2.moveKnobEncoderState[field.group] = field.value;
};

TraktorMX2.activateBeatjumpHandler = function (field) {
    if (TraktorMX2.shiftPressed[field.group]) {
        engine.setValue(field.group, "reloop_andstop", field.value);
    } else {
        engine.setValue(field.group, "beatlooproll_activate", field.value);
    }
};

TraktorMX2.microphoneHandler = function (field) {
    if (field.value) {
        if (TraktorMX2.microphonePressedTimer === 0) {
            // Start timer to measure how long button is pressed
            TraktorMX2.microphonePressedTimer = engine.beginTimer(300, () => {
                // Reset microphone button timer status if active
                if (TraktorMX2.microphonePressedTimer !== 0) {
                    TraktorMX2.microphonePressedTimer = 0;
                }
            }, true);
        }

        script.toggleControl("[Microphone]", "talkover");
    } else {
        // Button is released, check if timer is still running
        if (TraktorMX2.microphonePressedTimer !== 0) {
            // short click -> permanent activation
            TraktorMX2.microphonePressedTimer = 0;
        } else {
            engine.setValue("[Microphone]", "talkover", 0);
        }
    }
};

TraktorMX2.parameterHandler = function (field) {
    engine.setParameter(field.group, field.name, field.value / 4095);
};

TraktorMX2.jogModeHandler = function (field) {
    if (field.value === 0) {
        return;
    }

    const deckNumber = TraktorMX2.controller.resolveDeck(field.group);

    if (field.name === "!tt") {
        // Turntable mode
        TraktorMX2.jogModeState[field.group] = 0;

        TraktorMX2.outputHandler(false, field.group, "jog");
        TraktorMX2.outputHandler(true, field.group, "tt");

    } else if (field.name === "!jog") {
        // Jog mode
        TraktorMX2.jogModeState[field.group] = 1;
        engine.scratchDisable(deckNumber);

        TraktorMX2.outputHandler(true, field.group, "jog");
        TraktorMX2.outputHandler(false, field.group, "tt");
    }
};

TraktorMX2.jogTouchHandler = function (field) {
    const deckNumber = TraktorMX2.controller.resolveDeck(field.group);
    if (TraktorMX2.jogModeState[field.group] === 0) {
        // Turntable mode
        if (field.value > 0) {
            // Cancel any existing stop timers
            if ((TraktorMX2.jogStopTimerId[deckNumber - 1] !== null) && (TraktorMX2.jogStopTimerId[deckNumber - 1] !== undefined)) {
                engine.stopTimer(TraktorMX2.jogStopTimerId[deckNumber - 1]);
                TraktorMX2.jogStopTimerId[deckNumber - 1] = null;
            }
            engine.setValue(field.group, "scratch2_enable", true);
        } else {
            TraktorMX2.jogStopper(field);
        }
    }
};

// Called after the wheel is released. Stops scratching when the wheel is slow enough, allowing for inertia.
TraktorMX2.jogStopper = function(field) {
    const deckNumber = TraktorMX2.controller.resolveDeck(field.group);

    // If the wheel is stopped, exit scratching mode
    if (Math.abs(engine.getValue(field.group, "scratch2")) <= JOGWHEEL_EPSILON * VELOCITY_TO_SCRATCH) {
        engine.setValue(field.group, "scratch2", 0);
        engine.setValue(field.group, "scratch2_enable", false);
        TraktorMX2.lastVelocity[deckNumber - 1] = 0;
        TraktorMX2.jogStopTimerId[deckNumber - 1] = null;
        // Otherwise, check again after a while
    } else {
        TraktorMX2.jogStopTimerId[deckNumber - 1] = engine.beginTimer(JOGWHEEL_STOP_POLL_TIME, () => TraktorMX2.jogStopper(field), true);
    }
};

TraktorMX2.jogHandler = function (field) {

    if (field.name === "jog_timer") {
        // Update internal timer value
        TraktorMX2.jogTimer[field.group] = field.value;
        return;
    }

    const deckNumber = TraktorMX2.controller.resolveDeck(field.group);
    const velocity = TraktorMX2.wheelVelocity(deckNumber, field.value);


    if (TraktorMX2.jogModeState[field.group] === 0) {
        //Turntable
        if (TraktorMX2.shiftPressed[field.group] && !engine.getValue(field.group, "play")) {
            // Skip through track
            engine.setValue(field.group, "beatjump", velocity);
        } else {
            if (engine.getValue(field.group, "scratch2_enable")) {
                engine.setValue(field.group, "scratch2", velocity * VELOCITY_TO_SCRATCH);

                // Cancel any existing decay timers
                if ((TraktorMX2.jogDecayTimerId[deckNumber - 1] !== null) && (TraktorMX2.jogDecayTimerId[deckNumber - 1] !== undefined)) {
                    engine.stopTimer(TraktorMX2.jogDecayTimerId[deckNumber - 1]);
                    TraktorMX2.jogDecayTimerId[deckNumber - 1] = null;
                }
                // Start timer to manually decay the velocity after a while
                TraktorMX2.jogDecayTimerId[deckNumber - 1] = engine.beginTimer(JOGWHEEL_DECAY_POLL_TIME, () => {
                    TraktorMX2.jogDecayer(field);
                }, true);

            } else {
                engine.setValue(field.group, "jog", velocity * VELOCITY_TO_JOG);
            }
        }
    } else {
        // Jog
        engine.setValue(field.group, "jog", velocity * VELOCITY_TO_JOG);
    }
};

// Called continuously after jogwheel stops sending packets. Gradually slows the jogwheel.
TraktorMX2.jogDecayer = function(field) {
    const deckNumber = TraktorMX2.controller.resolveDeck(field.group);

    // If wheel is slow enough, immediately set scratch2 to 0
    if (Math.abs(engine.getValue(field.group, "scratch2")) <= JOGWHEEL_EPSILON * VELOCITY_TO_SCRATCH) {
        TraktorMX2.lastVelocity[deckNumber - 1] = 0;
        engine.setValue(field.group, "scratch2", 0);
        TraktorMX2.jogDecayTimerId[deckNumber - 1] = null;
        // Otherwise, decay the velocity and call itself again after a while
    } else {
        const decayedVelocity = TraktorMX2.lastVelocity[deckNumber - 1] * (1 - JOGWHEEL_ALPHA);
        TraktorMX2.lastVelocity[deckNumber - 1] = decayedVelocity;
        engine.setValue(field.group, "scratch2", decayedVelocity * VELOCITY_TO_SCRATCH);
        TraktorMX2.jogDecayTimerId[deckNumber - 1] = engine.beginTimer(JOGWHEEL_DECAY_POLL_TIME, () => TraktorMX2.jogDecayer(field), true);
    }
};

TraktorMX2.wheelVelocity = function(deckNumber, value) {

    // Get current timer value - 32bit
    let timeval = TraktorMX2.jogTimer[deckNumber === 1 ? "[Channel1]" : "[Channel2]"];

    // Current tick value (wheel position) - 32bit
    const tickval = value;

    // Group 1 and 2 -> Array index 0 and 1
    const prevTick = this.lastTickVal[deckNumber - 1];
    const prevTime = this.lastTimestamp[deckNumber - 1];
    const prevWallClock = this.lastWallClock[deckNumber - 1];
    this.lastTickVal[deckNumber - 1] = tickval;
    this.lastTimestamp[deckNumber - 1] = timeval;
    this.lastWallClock[deckNumber - 1] = Date.now();

    // If the user hasn't touched the jog wheel for a long time, the
    // internal timer may have looped around more than once. We have nothing
    // to go by so return 0
    if (this.lastWallClock[deckNumber - 1] - prevWallClock > 40000) {
        this.lastVelocity[deckNumber - 1] = 0;
        return 0;
    }

    if (prevTime > timeval) {
        // We looped around.  Adjust current time so that subtraction works.
        timeval += 0x400000;
    }
    let timeDelta = timeval - prevTime;
    if (timeDelta === 0) {
        // Spinning too fast to detect speed!  By not dividing we are guessing it took 10us.
        timeDelta = 1;
    }

    let tickDelta = tickval - prevTick;
    // Check if we looped around
    if (tickDelta > 512) {
        // Looped around from 0 to max
        tickDelta -= 1024;
    } else if (tickDelta < -512) {
        // Looped around from max to 0
        tickDelta += 1024;
    }

    // Velocity smoothing
    const velocity = tickDelta / timeDelta;
    const prevVelocity = this.lastVelocity[deckNumber - 1];
    let nextVelocity = null;
    // Check if the jogwheel is currently stopped or changing directions.
    // If so, set the velocity to the new value instantly.
    if ((Math.abs(prevVelocity) < JOGWHEEL_EPSILON) || (velocity * prevVelocity < 0)) {
        nextVelocity = velocity;
        //  Otherwise, smooth the velocity.
    } else {
        nextVelocity = JOGWHEEL_ALPHA * velocity + (1 - JOGWHEEL_ALPHA) * prevVelocity;
    }
    this.lastVelocity[deckNumber - 1] = nextVelocity;

    return nextVelocity;
};


TraktorMX2.fxHandler = function (field) {
    if (field.value === 0 || field.name === "misc") {
        return;
    }

    let group = `[EffectRack1_${field.group}_${field.name}]`
    if (!TraktorMX2.shiftPressed[`[Channel${field.group[field.group.length - 1]}]`]) {
        script.toggleControl(group, "enabled");
    } else {
        script.triggerControl(group, "next_effect");
    }


};

TraktorMX2.fxSelectHandler = function (field) {
    if (field.value === 0) {
        return;
    }

    let group = "[EffectRack1_" + field.name + "]"
    let control = "group_" + field.group + "_enable"

    script.toggleControl(group, control);
};

TraktorMX2.gfxToggleHandler = function (field) {
    if (field.value === 0) {
        return;
    }
    let group = "[QuickEffectRack1_" + field.group + "]"
    let control = "enabled"

    script.toggleControl(group, control);
};

TraktorMX2.globalfxHandler = function (field) {
    /* We support 8 effects in total by having 2 effects per fx button. *
     * First press of the button will load the preset at the index of the quick effect preset list *
     * Second press will load the preset index + 4 *
     * Arrange your quick effect preset list from 1 to 8 like this: 1/5, 2/6, 3/7, 4/8 */
    if (field.value === 0) {
        return;
    }

    const availableGroups = ["[Channel1]", "[Channel2]"];
    const fxButtonCount = 4;
    const fxNumber = parseInt(field.id[field.id.length - 1]);

    const fxToApply = {};
    const activeFx = {};
    // Detect which fx should be enabled
    for (const group of availableGroups) {
        activeFx[group] = engine.getValue(`[QuickEffectRack1_${group}]`, "loaded_chain_preset") - 1;

        if (activeFx[group] === fxNumber && fxNumber !== 0) {
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

    const [deck1, deck2] = availableGroups;
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

    // Now apply the new fx value
    for (const group of availableGroups) {
        engine.setValue(`[QuickEffectRack1_${group}]`, "loaded_chain_preset", fxToApply[group] + 1);
    }
};

TraktorMX2.reverseHandler = function (field) {
    if (TraktorMX2.shiftPressed[field.group]) {
        engine.setValue(field.group, "reverseroll", field.value);
    } else {
        engine.setValue(field.group, "reverse", field.value);
    }

    TraktorMX2.outputHandler(field.value, field.group, "reverse");
};

TraktorMX2.fluxHandler = function (field) {
    if (field.value === 0) {
        return;
    }
    script.toggleControl(field.group, "slip_enabled");
};

TraktorMX2.registerOutputPackets = function () {
    const output = new HIDPacket("output", 0x80);

    // Channel 1
    output.addOutput("[Channel1]", "fx_misc", 0x01, "B");
    output.addOutput("[Channel1]", "fx_1", 0x02, "B");
    output.addOutput("[Channel1]", "fx_2", 0x03, "B");
    output.addOutput("[Channel1]", "fx_3", 0x04, "B");

    output.addOutput("[Channel1]", "favorites", 0x05, "B");
    output.addOutput("[Channel1]", "addTrack", 0x06, "B");
    output.addOutput("[PreviewDeck1]", "play_indicator", 0x07, "B");
    output.addOutput("[Channel1]", "maximizeLibrary", 0x08, "B");

    output.addOutput("[Channel1]", "slip_enabled", 0x09, "B");
    output.addOutput("[Channel1]", "reverse", 0x0a, "B");

    output.addOutput("[Channel1]", "tt", 0x0b, "B");
    output.addOutput("[Channel1]", "jog", 0x0c, "B");

    output.addOutput("[Channel1]", "shift", 0x0d, "B");

    output.addOutput("[Channel1]", "sync_enabled", 0x0e, "B");
    output.addOutput("[Channel1]", "sync_leader", 0x0f, "B");

    output.addOutput("[Channel1]", "keylock", 0x10, "B");

    output.addOutput("[Channel1]", "hotcues", 0x11, "B");
    output.addOutput("[Channel1]", "stems", 0x12, "B");
    output.addOutput("[Channel1]", "patterns", 0x13, "B");
    output.addOutput("[Channel1]", "loops", 0x14, "B");

    output.addOutput("[Channel1]", "pad_1", 0x15, "B");
    output.addOutput("[Channel1]", "pad_2", 0x16, "B");
    output.addOutput("[Channel1]", "pad_3", 0x17, "B");
    output.addOutput("[Channel1]", "pad_4", 0x18, "B");
    output.addOutput("[Channel1]", "pad_5", 0x19, "B");
    output.addOutput("[Channel1]", "pad_6", 0x1a, "B");
    output.addOutput("[Channel1]", "pad_7", 0x1b, "B");
    output.addOutput("[Channel1]", "pad_8", 0x1c, "B");

    output.addOutput("[Channel1]", "cue_indicator", 0x1d, "B");
    output.addOutput("[Channel1]", "play_indicator", 0x1e, "B");


    // Channel 2
    output.addOutput("[Channel2]", "fx_misc", 0x1f, "B");
    output.addOutput("[Channel2]", "fx_1", 0x20, "B");
    output.addOutput("[Channel2]", "fx_2", 0x21, "B");
    output.addOutput("[Channel2]", "fx_3", 0x22, "B");

    output.addOutput("[Channel2]", "favorites", 0x23, "B");
    output.addOutput("[Channel2]", "addTrack", 0x24, "B");
    output.addOutput("[PreviewDeck2]", "play_indicator", 0x25, "B");
    output.addOutput("[Channel2]", "maximizeLibrary", 0x26, "B");

    output.addOutput("[Channel2]", "slip_enabled", 0x27, "B");
    output.addOutput("[Channel2]", "reverse", 0x28, "B");

    output.addOutput("[Channel2]", "tt", 0x29, "B");
    output.addOutput("[Channel2]", "jog", 0x2a, "B");

    output.addOutput("[Channel2]", "shift", 0x2b, "B");

    output.addOutput("[Channel2]", "sync_enabled", 0x2c, "B");
    output.addOutput("[Channel2]", "sync_leader", 0x2d, "B");

    output.addOutput("[Channel2]", "keylock", 0x2e, "B");

    output.addOutput("[Channel2]", "hotcues", 0x2f, "B");
    output.addOutput("[Channel2]", "stems", 0x30, "B");
    output.addOutput("[Channel2]", "patterns", 0x31, "B");
    output.addOutput("[Channel2]", "loops", 0x32, "B");

    output.addOutput("[Channel2]", "pad_1", 0x33, "B");
    output.addOutput("[Channel2]", "pad_2", 0x34, "B");
    output.addOutput("[Channel2]", "pad_3", 0x35, "B");
    output.addOutput("[Channel2]", "pad_4", 0x36, "B");
    output.addOutput("[Channel2]", "pad_5", 0x37, "B");
    output.addOutput("[Channel2]", "pad_6", 0x38, "B");
    output.addOutput("[Channel2]", "pad_7", 0x39, "B");
    output.addOutput("[Channel2]", "pad_8", 0x3a, "B");

    output.addOutput("[Channel2]", "cue_indicator", 0x3b, "B");
    output.addOutput("[Channel2]", "play_indicator", 0x3c, "B");

    // Mixer and Effects
    output.addOutput("[Channel1]", "fx_select_1", 0x3d, "B");
    output.addOutput("[Channel1]", "fx_select_2", 0x3e, "B");
    output.addOutput("[Channel1]", "gfx_toggle", 0x3f, "B");
    output.addOutput("[Channel1]", "pfl", 0x40, "B");

    output.addOutput("[Channel2]", "fx_select_1", 0x41, "B");
    output.addOutput("[Channel2]", "fx_select_2", 0x42, "B");
    output.addOutput("[Channel2]", "gfx_toggle", 0x43, "B");
    output.addOutput("[Channel2]", "pfl", 0x44, "B");

    output.addOutput("[ChannelX]", "gfx_1", 0x45, "B");
    output.addOutput("[ChannelX]", "gfx_2", 0x46, "B");
    output.addOutput("[ChannelX]", "gfx_3", 0x47, "B");
    output.addOutput("[ChannelX]", "gfx_4", 0x48, "B");
    output.addOutput("[ChannelX]", "gfx_0", 0x49, "B");

    output.addOutput("[Microphone]", "talkover", 0x4a, "B");

    output.addOutput("[Channel1]", "bottom_led_1", 0x4b, "B");
    output.addOutput("[Channel1]", "bottom_led_2", 0x4c, "B");
    output.addOutput("[Channel1]", "bottom_led_3", 0x4d, "B");
    output.addOutput("[Channel1]", "bottom_led_4", 0x4e, "B");
    output.addOutput("[Channel1]", "bottom_led_5", 0x4f, "B");
    output.addOutput("[Channel1]", "bottom_led_6", 0x50, "B");

    output.addOutput("[Channel2]", "bottom_led_1", 0x51, "B");
    output.addOutput("[Channel2]", "bottom_led_2", 0x52, "B");
    output.addOutput("[Channel2]", "bottom_led_3", 0x53, "B");
    output.addOutput("[Channel2]", "bottom_led_4", 0x54, "B");
    output.addOutput("[Channel2]", "bottom_led_5", 0x55, "B");
    output.addOutput("[Channel2]", "bottom_led_6", 0x56, "B");

    output.addOutput("[Main]", "peak_indicator", 0x57, "B");

    output.addOutput("[Channel1]", "vu-18", 0x58, "B");
    output.addOutput("[Channel1]", "vu-15", 0x59, "B");
    output.addOutput("[Channel1]", "vu-12", 0x5a, "B");
    output.addOutput("[Channel1]", "vu-9", 0x5b, "B");
    output.addOutput("[Channel1]", "vu-6", 0x5c, "B");
    output.addOutput("[Channel1]", "vu-3", 0x5d, "B");
    output.addOutput("[Channel1]", "vu0", 0x5e, "B");
    output.addOutput("[Channel1]", "vu6", 0x5f, "B");
    output.addOutput("[Channel1]", "peak_indicator", 0x60, "B");

    output.addOutput("[Channel2]", "vu-18", 0x61, "B");
    output.addOutput("[Channel2]", "vu-15", 0x62, "B");
    output.addOutput("[Channel2]", "vu-12", 0x63, "B");
    output.addOutput("[Channel2]", "vu-9", 0x64, "B");
    output.addOutput("[Channel2]", "vu-6", 0x65, "B");
    output.addOutput("[Channel2]", "vu-3", 0x66, "B");
    output.addOutput("[Channel2]", "vu0", 0x67, "B");
    output.addOutput("[Channel2]", "vu6", 0x68, "B");
    output.addOutput("[Channel2]", "peak_indicator", 0x69, "B");

    this.controller.registerOutputPacket(output);


    // Link outputs to handlers

    // Channel 1
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect1]", "enabled", this.fxOutputHandler);
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect2]", "enabled", this.fxOutputHandler);
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect3]", "enabled", this.fxOutputHandler);

    // Favorites
    // Add to List
    engine.makeConnection("[PreviewDeck1]", "play_indicator", this.previewOutputHandler);
    // Maximize Library

    this.linkOutput("[Channel1]", "slip_enabled", this.outputHandler);
    // Reverse

    // Shift

    this.linkOutput("[Channel1]", "sync_enabled", this.outputHandler);
    this.linkOutput("[Channel1]", "sync_leader", this.outputHandler);

    this.linkOutput("[Channel1]", "keylock", this.outputHandler);

    this.linkOutput("[Channel1]", "play_indicator", this.outputHandler);
    this.linkOutput("[Channel1]", "cue_indicator", this.outputHandler);


    // Channel 2

    engine.makeConnection("[EffectRack1_EffectUnit2_Effect1]", "enabled", this.fxOutputHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2_Effect2]", "enabled", this.fxOutputHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2_Effect3]", "enabled", this.fxOutputHandler);

    // Favorites
    // Add to List
    // Same as Channel 1: engine.makeConnection("[PreviewDeck1]", "play_indicator", this.previewOutputHandler)
    // Maximize Library

    this.linkOutput("[Channel2]", "slip_enabled", this.outputHandler)
    // Reverse

    // Shift

    this.linkOutput("[Channel2]", "sync_enabled", this.outputHandler);
    this.linkOutput("[Channel2]", "sync_leader", this.outputHandler);
    this.linkOutput("[Channel2]", "keylock", this.outputHandler);

    this.linkOutput("[Channel2]", "play_indicator", this.outputHandler);
    this.linkOutput("[Channel2]", "cue_indicator", this.outputHandler);


    // Mixer and Effects
    engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", this.fxSelectOutputHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", this.fxSelectOutputHandler);

    engine.makeConnection("[QuickEffectRack1_[Channel1]]", "enabled", this.gfxToggleOutputHandler);

    engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", this.fxSelectOutputHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", this.fxSelectOutputHandler);

    this.linkOutput("[Channel1]", "pfl", this.outputHandler);
    this.linkOutput("[Channel2]", "pfl", this.outputHandler);

    engine.makeConnection("[QuickEffectRack1_[Channel2]]", "enabled", this.gfxToggleOutputHandler);

    this.fxCallbacks = [];
    for (const group of ["[Channel1]", "[Channel2]"]) {
        this.fxCallbacks.push(engine.makeConnection(`[QuickEffectRack1_${group}]`, "loaded_chain_preset", this.gfxOutputHandler));
    }

    this.linkOutput("[Microphone]", "talkover", this.outputHandler);

    for (let i = 1; i <= 8; ++i) {
        engine.makeConnection("[Channel1]", `hotcue_${i}_status`, this.hotcueOutputHandler);
        engine.makeConnection("[Channel2]", `hotcue_${i}_status`, this.hotcueOutputHandler);

        engine.makeConnection("[Channel1]", `hotcue_${i}_color`, this.hotcueColorHandler);
        engine.makeConnection("[Channel2]", `hotcue_${i}_color`, this.hotcueColorHandler);
    }

    engine.makeConnection("[Channel1]", `stem_count`, this.patternOutputHandler);
    engine.makeConnection("[Channel2]", `stem_count`, this.patternOutputHandler);

    // Bottom LEDs

    engine.makeConnection("[App]", "indicator_500ms", this.bottomLedOutputHandler);


    // VuMeter
    this.vuLeftConnection = engine.makeUnbufferedConnection("[Channel1]", "vu_meter", this.vuMeterOutputHandler);
    this.vuRightConnection = engine.makeUnbufferedConnection("[Channel2]", "vu_meter", this.vuMeterOutputHandler);

    this.clipLeftConnection = engine.makeConnection("[Channel1]", "peak_indicator", this.peakOutputHandler.bind(this));
    this.clipRightConnection = engine.makeConnection("[Channel2]", "peak_indicator", this.peakOutputHandler.bind(this));

    this.clipMainConnection = engine.makeConnection("[Main]", "peak_indicator", this.peakOutputHandler.bind(this));

    TraktorMX2.lightDeck(false);
};

/* Helper function to link output in a short form */
TraktorMX2.linkOutput = function (group, name, callback) {
    TraktorMX2.controller.linkOutput(group, name, group, name, callback);
};

TraktorMX2.outputHandler = function (value, group, key) {
    // Custom value for multi-colored LEDs

    let ledValue = value;

    // Look up color from map
    const colorConfig = TraktorMX2.OutputColorMap[group]?.[key];

    if (colorConfig) {
        if (value === 0 || value === false) {
            ledValue = colorConfig.dim;
        } else if (value === 1 || value === true) {
            ledValue = colorConfig.full;
        }
    }

    TraktorMX2.controller.setOutput(group, key, ledValue, true);
};

TraktorMX2.colorOutputHandler = function (value, group, key) {
    // Custom value for multi-colored LEDs
    const colorValue = TraktorMX2.PadColorMap.getValueForNearestColor(value);
    TraktorMX2.controller.setOutput(group, key, colorValue, true);
};

TraktorMX2.vuMeterOutputHandler = function (value, group, _key) {
    const vuKeys = Object.keys(TraktorMX2.vuMeterThresholds);
    for (let i = 0; i < vuKeys.length; ++i) {
        // Avoid spamming HID by only sending last LED update
        const last = i === vuKeys.length - 1;
        if (TraktorMX2.vuMeterThresholds[vuKeys[i]] > value) {
            TraktorMX2.controller.setOutput(group, vuKeys[i], 0x00, last);
        } else {
            TraktorMX2.controller.setOutput(group, vuKeys[i], 0x7e, last);
        }
    }
};

TraktorMX2.peakOutputHandler = function (value, group, key) {
    let ledValue = 0x00;

    if (value) {
        ledValue = 0x7e;
        if (group === "[Main]") {
            ledValue = TraktorMX2.baseColors.red;
        }
    }

    TraktorMX2.controller.setOutput(group, key, ledValue, true);
};

TraktorMX2.previewOutputHandler = function (_value, group, name) {
    const value = engine.getValue(group, name);
    TraktorMX2.outputHandler(value, `[PreviewDeck1]`, "play_indicator");
    TraktorMX2.outputHandler(value, `[PreviewDeck2]`, "play_indicator");
}

TraktorMX2.gfxToggleOutputHandler = function (_value, group, name) {
    TraktorMX2.outputHandler(engine.getValue(group, name), `[Channel${group[group.length - 3]}]`, "gfx_toggle");
}

TraktorMX2.fxSelectOutputHandler = function (_value, group, name) {
    TraktorMX2.outputHandler(engine.getValue(group, name), `[Channel${name[name.length - 9]}]`, `fx_select_${group[group.length - 2]}`);
}

TraktorMX2.fxOutputHandler = function (_value, group, name) {
    TraktorMX2.outputHandler(engine.getValue(group, name), `[Channel${group[group.length - 10]}]`, `fx_${group[group.length - 2]}`);
}

TraktorMX2.gfxOutputHandler = function () {
    const fxButtonCount = 4;

    let presetNum = engine.getValue(`[QuickEffectRack1_[Channel1]]`, "loaded_chain_preset") - 1;

    for (let fxButton = 0; fxButton <= fxButtonCount; fxButton++) {
        let active = (presetNum === fxButton || (fxButton !== 0 && (fxButton === presetNum - fxButtonCount)));
        TraktorMX2.outputHandler(active, "[ChannelX]", "gfx_" + fxButton);
    }
};

TraktorMX2.hotcueOutputHandler = function (value, group, name) {
    // Light button LED only when we are in hotcue mode
    if (TraktorMX2.padModeState[group] === 0) {
        const colorKey = name.replace("_status", "_color");
        const color = engine.getValue(group, colorKey);
        const padNum = name[7];
        if (value > 0) {
            TraktorMX2.colorOutputHandler(color, group, `pad_${padNum}`);
        } else {
            TraktorMX2.outputHandler(TraktorMX2.baseColors.dimmedWhite, group, `pad_${padNum}`);
        }
    }
};

TraktorMX2.hotcueColorHandler = function (value, group, name) {
    // Light button LED only when we are in hotcue mode
    const padNum = name[7];
    if (TraktorMX2.padModeState[group] === 0) {
        TraktorMX2.colorOutputHandler(value, group, `pad_${padNum}`);
    }
};

TraktorMX2.patternOutputHandler = function (value, group, name) {
    if (TraktorMX2.padModeState[group] === 1) {
        for (let i = 1; i <= engine.getValue(group, name); i++) {
            const color = engine.getValue(`[Channel${group[group.length - 2]}_Stem${i}]`, "color");
            const status = engine.getValue(`[Channel${group[group.length - 2]}_Stem${i}]`, "mute");
            const colorValue = status ? TraktorMX2.baseColors.dimmedRed : TraktorMX2.PadColorMap.getValueForNearestColor(color);
            TraktorMX2.outputHandler(colorValue, group, `pad_${i}`);
        }
    }
};

TraktorMX2.bottomLedOutputHandler = function () {

    for (const channel of ["[Channel1]", "[Channel2]"]) {
        let eot = engine.getValue(channel, "end_of_track")
        let loop = engine.getValue(channel, "loop_enabled")
        let playing = engine.getValue(channel, "play")
        for (let i = 1; i <= 6; i++) {
            if (eot) {
                if (TraktorMX2.bottomLedState[channel][i] === TraktorMX2.baseColors.red) {
                    TraktorMX2.bottomLedState[channel][i] = TraktorMX2.baseColors.off;
                } else {
                    TraktorMX2.bottomLedState[channel][i] = TraktorMX2.baseColors.red;
                }
            } else if (loop) {
                TraktorMX2.bottomLedState[channel][i] = playing ? TraktorMX2.baseColors.green : TraktorMX2.baseColors.dimmedGreen;
            } else {
                TraktorMX2.bottomLedState[channel][i] = playing ? TraktorMX2.baseColors.blue : TraktorMX2.baseColors.dimmedBlue;
            }
        }

        for (let i = 1; i <= 5; i++) {
            TraktorMX2.controller.setOutput(channel, `bottom_led_${i}`, TraktorMX2.bottomLedState[channel][i], false);
        }
        TraktorMX2.controller.setOutput(channel, "bottom_led_" + 6, TraktorMX2.bottomLedState[channel][6], true);
    }

}

TraktorMX2.lightDeck = function (switchOff) {

    const getColorValue = (group, key, active) => {
        if (switchOff) {
            return 0x00;
        }
        const colorConfig = TraktorMX2.OutputColorMap[group]?.[key];
        if (colorConfig) {
            return active ? colorConfig.full : colorConfig.dim;
        }
        // Fallback to white if not in map
        return active ? 0x7e : 0x7c;
    };

    let current = engine.getValue("[Channel1]", "play_indicator");
    TraktorMX2.controller.setOutput("[Channel1]", "play_indicator", getColorValue("[Channel1]", "play_indicator", current), false);
    current = engine.getValue("[Channel2]", "play_indicator");
    TraktorMX2.controller.setOutput("[Channel2]", "play_indicator", getColorValue("[Channel2]", "play_indicator", current), false);

    current = engine.getValue("[Channel1]", "cue_indicator");
    TraktorMX2.controller.setOutput("[Channel1]", "cue_indicator", getColorValue("[Channel1]", "cue_indicator", current), false);
    current = engine.getValue("[Channel2]", "cue_indicator");
    TraktorMX2.controller.setOutput("[Channel2]", "cue_indicator", getColorValue("[Channel2]", "cue_indicator", current), false);

    TraktorMX2.controller.setOutput("[Channel1]", "shift", getColorValue("[Channel1]", "shift", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "shift", getColorValue("[Channel2]", "shift", false), false);

    current = engine.getValue("[Channel1]", "sync_enabled");
    TraktorMX2.controller.setOutput("[Channel1]", "sync_enabled", getColorValue("[Channel1]", "sync_enabled", current), false);
    current = engine.getValue("[Channel2]", "sync_enabled");
    TraktorMX2.controller.setOutput("[Channel2]", "sync_enabled", getColorValue("[Channel2]", "sync_enabled", current), false);

    current = engine.getValue("[Channel1]", "sync_leader");
    TraktorMX2.controller.setOutput("[Channel1]", "sync_leader", getColorValue("[Channel1]", "sync_leader", current), false);
    current = engine.getValue("[Channel2]", "sync_leader");
    TraktorMX2.controller.setOutput("[Channel2]", "sync_leader", getColorValue("[Channel2]", "sync_leader", current), false);

    // Hotcues mode is default start value
    TraktorMX2.controller.setOutput("[Channel1]", "hotcues", getColorValue("[Channel1]", "hotcues", true), false);
    TraktorMX2.controller.setOutput("[Channel2]", "hotcues", getColorValue("[Channel2]", "hotcues", true), false);

    TraktorMX2.controller.setOutput("[Channel1]", "stems", getColorValue("[Channel1]", "stems", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "stems", getColorValue("[Channel2]", "stems", false), false);

    TraktorMX2.controller.setOutput("[Channel1]", "patterns", getColorValue("[Channel1]", "patterns", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "patterns", getColorValue("[Channel2]", "patterns", false), false);

    TraktorMX2.controller.setOutput("[Channel1]", "loops", getColorValue("[Channel1]", "loops", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "loops", getColorValue("[Channel2]", "loops", false), false);

    current = engine.getValue("[Channel1]", "keylock");
    TraktorMX2.controller.setOutput("[Channel1]", "keylock", getColorValue("[Channel1]", "keylock", current), false);
    current = engine.getValue("[Channel2]", "keylock");
    TraktorMX2.controller.setOutput("[Channel2]", "keylock", getColorValue("[Channel2]", "keylock", current), false);

    for (let i = 1; i <= 8; ++i) {
        if (switchOff) {
            // do not dim but turn completely off
            TraktorMX2.outputHandler(0x00, "[Channel1]", "pad_" + i);
            TraktorMX2.outputHandler(0x00, "[Channel2]", "pad_" + i);
        } else {
            current = engine.getValue("[Channel1]", `hotcue_${i}_status`) ? TraktorMX2.baseColors.blue : TraktorMX2.baseColors.dimmed;
            TraktorMX2.outputHandler(current, "[Channel1]", `pad_${i}`);
            current = engine.getValue("[Channel2]", `hotcue_${i}_status`) ? TraktorMX2.baseColors.blue : TraktorMX2.baseColors.dimmed;
            TraktorMX2.outputHandler(current, "[Channel2]", `pad_${i}`);
        }
    }

    current = engine.getValue("[Channel1]", "pfl");
    TraktorMX2.controller.setOutput("[Channel1]", "pfl", getColorValue("[Channel1]", "pfl", current), false);
    current = engine.getValue("[Channel2]", "pfl");
    TraktorMX2.controller.setOutput("[Channel2]", "pfl", getColorValue("[Channel2]", "pfl", current), false);

    current = engine.getValue("[QuickEffectRack1_[Channel1]]", "enabled");
    TraktorMX2.controller.setOutput("[Channel1]", "gfx_toggle", getColorValue("[Channel1]", "gfx_toggle", current), false);
    current = engine.getValue("[QuickEffectRack1_[Channel2]]", "enabled");
    TraktorMX2.controller.setOutput("[Channel2]", "gfx_toggle", getColorValue("[Channel2]", "gfx_toggle", current), false);

    TraktorMX2.controller.setOutput("[ChannelX]", "gfx_0", getColorValue("[ChannelX]", "gfx_0", false), false);
    TraktorMX2.controller.setOutput("[ChannelX]", "gfx_1", getColorValue("[ChannelX]", "gfx_1", false), false);
    TraktorMX2.controller.setOutput("[ChannelX]", "gfx_2", getColorValue("[ChannelX]", "gfx_2", false), false);
    TraktorMX2.controller.setOutput("[ChannelX]", "gfx_3", getColorValue("[ChannelX]", "gfx_3", false), false);
    TraktorMX2.controller.setOutput("[ChannelX]", "gfx_4", getColorValue("[ChannelX]", "gfx_4", false), false);

    current = engine.getValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable");
    TraktorMX2.controller.setOutput("[Channel1]", "fx_select_1", getColorValue("[Channel1]", "fx_select_1", current), false);
    current = engine.getValue("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable");
    TraktorMX2.controller.setOutput("[Channel1]", "fx_select_2", getColorValue("[Channel1]", "fx_select_2", current), false);

    current = engine.getValue("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable");
    TraktorMX2.controller.setOutput("[Channel2]", "fx_select_1", getColorValue("[Channel2]", "fx_select_1", current), false);
    current = engine.getValue("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable");
    TraktorMX2.controller.setOutput("[Channel2]", "fx_select_2", getColorValue("[Channel2]", "fx_select_2", current), false);

    TraktorMX2.controller.setOutput("[Channel1]", "fx_misc", getColorValue("[Channel1]", "fx_misc", false), false);
    TraktorMX2.controller.setOutput("[Channel1]", "fx_1", getColorValue("[Channel1]", "fx_1", false), false);
    TraktorMX2.controller.setOutput("[Channel1]", "fx_2", getColorValue("[Channel1]", "fx_2", false), false);
    TraktorMX2.controller.setOutput("[Channel1]", "fx_3", getColorValue("[Channel1]", "fx_3", false), false);

    TraktorMX2.controller.setOutput("[Channel2]", "fx_misc", getColorValue("[Channel2]", "fx_misc", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "fx_1", getColorValue("[Channel2]", "fx_1", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "fx_2", getColorValue("[Channel2]", "fx_2", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "fx_3", getColorValue("[Channel2]", "fx_3", false), false);

    // Set FX button LED state according to active quick effects on start-up
    if (!switchOff) {
        TraktorMX2.gfxOutputHandler();
    }

    TraktorMX2.controller.setOutput("[Channel1]", "reverse", getColorValue("[Channel1]", "reverse", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "reverse", getColorValue("[Channel2]", "reverse", false), false);

    current = engine.getValue("[Channel1]", "slip_enabled");
    TraktorMX2.controller.setOutput("[Channel1]", "slip_enabled", getColorValue("[Channel1]", "slip_enabled", current), false);
    current = engine.getValue("[Channel2]", "slip_enabled");
    TraktorMX2.controller.setOutput("[Channel2]", "slip_enabled", getColorValue("[Channel2]", "slip_enabled", current), false);

    current = TraktorMX2.jogModeState["[Channel1]"];
    TraktorMX2.controller.setOutput("[Channel1]", "tt", getColorValue("[Channel1]", "tt", !current), false);
    TraktorMX2.controller.setOutput("[Channel1]", "jog", getColorValue("[Channel1]", "jog", current), false);
    current = TraktorMX2.jogModeState["[Channel2]"];
    TraktorMX2.controller.setOutput("[Channel2]", "tt", getColorValue("[Channel2]", "tt", !current), false);
    TraktorMX2.controller.setOutput("[Channel2]", "jog", getColorValue("[Channel2]", "jog", current), false);

    TraktorMX2.controller.setOutput("[PreviewDeck1]", "play_indicator", getColorValue("[PreviewDeck1]", "play_indicator", false), false);
    TraktorMX2.controller.setOutput("[PreviewDeck2]", "play_indicator", getColorValue("[PreviewDeck2]", "play_indicator", false), false);

    TraktorMX2.controller.setOutput("[Channel1]", "maximizeLibrary", getColorValue("[Channel1]", "maximizeLibrary", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "maximizeLibrary", getColorValue("[Channel2]", "maximizeLibrary", false), false);

    TraktorMX2.controller.setOutput("[Channel1]", "addTrack", getColorValue("[Channel1]", "addTrack", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "addTrack", getColorValue("[Channel2]", "addTrack", false), false);

    TraktorMX2.controller.setOutput("[Channel1]", "favorites", getColorValue("[Channel1]", "favorites", false), false);
    TraktorMX2.controller.setOutput("[Channel2]", "favorites", getColorValue("[Channel2]", "favorites", false), false);

    // For the last output we should send the packet finally
    current = engine.getValue("[Microphone]", "talkover");
    TraktorMX2.controller.setOutput("[Microphone]", "talkover", getColorValue("[Microphone]", "talkover", current), true);
};

TraktorMX2.messageCallback = function (packet, data) {
    for (const name in data) {
        if (Object.hasOwnProperty.call(data, name)) {
            TraktorMX2.controller.processButton(data[name]);
        }
    }
};

TraktorMX2.shutdown = function () {
    // Deactivate all LEDs
    TraktorMX2.lightDeck(true);

    console.log("HID: TraktorMX2: Shutdown done!");
};

TraktorMX2.incomingData = function (data, length) {
    TraktorMX2.controller.parsePacket(data, length);
};

TraktorMX2.baseColors = {
    off: 0x00,

    // Used for single-colored LED like vumeters -> white on multicolored LEDs
    dimmed: 0x7c, full: 0x7e,

    // White - used for multi-colored LEDs
    dimmedWhite: 0x48, white: 0x4a,

    dimmedRed: 0x04, red: 0x06,

    dimmedGreen: 0x1c, green: 0x1e,

    dimmedBlue: 0x2c, blue: 0x2e,

    dimmedYellow: 0x14, yellow: 0x16,

    dimmedOrange: 0x0c, orange: 0x0e,
}

TraktorMX2.OutputColorMap = {
    // Channel 1
    "[Channel1]": {
        "fx_misc": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "fx_1": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "fx_2": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "fx_3": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "favorites": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "addTrack": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "maximizeLibrary": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "slip_enabled": {dim: TraktorMX2.baseColors.dimmedRed, full: TraktorMX2.baseColors.red},
        "reverse": {dim: TraktorMX2.baseColors.dimmedRed, full: TraktorMX2.baseColors.red},
        "tt": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "jog": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "shift": {dim: TraktorMX2.baseColors.dimmedWhite, full: TraktorMX2.baseColors.white},
        "sync_enabled": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "sync_leader": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "keylock": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "hotcues": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "stems": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "patterns": {dim: TraktorMX2.baseColors.off, full: TraktorMX2.baseColors.red},
        "loops": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "cue_indicator": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "play_indicator": {dim: TraktorMX2.baseColors.dimmedGreen, full: TraktorMX2.baseColors.green},
        "fx_select_1": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "fx_select_2": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "gfx_toggle": {dim: TraktorMX2.baseColors.dimmedYellow, full: TraktorMX2.baseColors.yellow},
        "pfl": {dim: TraktorMX2.baseColors.dimmedWhite, full: TraktorMX2.baseColors.white},
    }, // Channel 2 (same structure)
    "[Channel2]": {
        "fx_misc": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "fx_1": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "fx_2": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "fx_3": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "favorites": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "addTrack": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "maximizeLibrary": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "slip_enabled": {dim: TraktorMX2.baseColors.dimmedRed, full: TraktorMX2.baseColors.red},
        "reverse": {dim: TraktorMX2.baseColors.dimmedRed, full: TraktorMX2.baseColors.red},
        "tt": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "jog": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "shift": {dim: TraktorMX2.baseColors.dimmedWhite, full: TraktorMX2.baseColors.white},
        "sync_enabled": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "sync_leader": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "keylock": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "hotcues": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "stems": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "patterns": {dim: TraktorMX2.baseColors.off, full: TraktorMX2.baseColors.red},
        "loops": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "cue_indicator": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "play_indicator": {dim: TraktorMX2.baseColors.dimmedGreen, full: TraktorMX2.baseColors.green},
        "fx_select_1": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "fx_select_2": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "gfx_toggle": {dim: TraktorMX2.baseColors.dimmedYellow, full: TraktorMX2.baseColors.yellow},
        "pfl": {dim: TraktorMX2.baseColors.dimmedWhite, full: TraktorMX2.baseColors.white},
    }, "[ChannelX]": {
        "gfx_0": {dim: TraktorMX2.baseColors.dimmedOrange, full: TraktorMX2.baseColors.orange},
        "gfx_1": {dim: TraktorMX2.baseColors.dimmedRed, full: TraktorMX2.baseColors.red},
        "gfx_2": {dim: 0x20, full: 0x22},
        "gfx_3": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
        "gfx_4": {dim: TraktorMX2.baseColors.dimmedYellow, full: TraktorMX2.baseColors.yellow},
    }, "[Microphone]": {
        "talkover": {dim: TraktorMX2.baseColors.dimmedWhite, full: TraktorMX2.baseColors.white},
    }, "[PreviewDeck1]": {
        "play_indicator": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
    }, "[PreviewDeck2]": {
        "play_indicator": {dim: TraktorMX2.baseColors.dimmedBlue, full: TraktorMX2.baseColors.blue},
    },
};

TraktorMX2.PadColorMap = new ColorMapper({
    0x755771: 0x00,
    0xe9c9da: 0x01,
    0x372c2b: 0x02,
    0x2b2b21: 0x03,
    0xaf262e: 0x04,
    0xcb242c: 0x05,
    0xff2916: 0x06,
    0xfe354f: 0x07,
    0xb8332d: 0x08,
    0xd1322c: 0x09,
    0xfd5921: 0x0a,
    0xf95364: 0x0b,
    0xa53a2e: 0x0c,
    0xb03e2b: 0x0d,
    0xfd7bab: 0x0e,
    0xfb8553: 0x0f,
    0xbe712f: 0x10,
    0xc57831: 0x11,
    0xf8ab36: 0x12,
    0xf6aa80: 0x13,
    0x9c7132: 0x14,
    0xb07b31: 0x15,
    0xe9ba3d: 0x16,
    0xecbc92: 0x17,
    0x626341: 0x18,
    0x6d6c43: 0x19,
    0xb6b959: 0x1a,
    0xa9bb91: 0x1b,
    0x2e5c45: 0x1c,
    0x2b7446: 0x1d,
    0x2ccf68: 0x1e,
    0x92c7b0: 0x1f,
    0x409654: 0x20,
    0x39a97f: 0x21,
    0x3fd2b0: 0x22,
    0xb0c6c9: 0x23,
    0x2c8fe4: 0x24,
    0x7f97de: 0x25,
    0x0de1ff: 0x26,
    0xb3d5ff: 0x27,
    0x059bf3: 0x28,
    0x6b97ef: 0x29,
    0x00c3ff: 0x2a,
    0x63d3ff: 0x2b,
    0x1860dd: 0x2c,
    0x0482f8: 0x2d,
    0x00a1ff: 0x2e,
    0x42b3ff: 0x2f,
    0x8d52dd: 0x30,
    0xb171ea: 0x31,
    0xe298fe: 0x32,
    0xceb4fd: 0x33,
    0xeb4bd3: 0x34,
    0xeb5ddb: 0x35,
    0xf263e3: 0x36,
    0xf08cea: 0x37,
    0xc33196: 0x38,
    0xc944a1: 0x39,
    0xfc4dc0: 0x3a,
    0xfa6ddc: 0x3b,
    0xa82c6b: 0x3c,
    0xb13a74: 0x3d,
    0xfb359f: 0x3e,
    0xf94eae: 0x3f,
    0xd12a40: 0x40,
    0xda394f: 0x41,
    0xfb3672: 0x42,
    0xf8487e: 0x43,
    0x6d5a77: 0x44,
    0x836295: 0x45,
    0xe5bada: 0x46,
    0xe1bce3: 0x47,
    0x7f6487: 0x48,
    0x916ca0: 0x49,
    0xdcb6d4: 0x4a,
    0xd6b3db: 0x4b,
    0x795f82: 0x4c,
    0x8e689e: 0x4d,
    0xddb9d4: 0x4e,
    0xd6b7d9: 0x4f,
});

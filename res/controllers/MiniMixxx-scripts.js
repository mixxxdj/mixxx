///////////////////////////////////////////////////////////////////////////////////
//
// MiniMixxx controller script v1.00
// Author: Owen Williams (owilliams@mixxx.org)
//
///////////////////////////////////////////////////////////////////////////////////
//
// TODO:
//
// * Library mode
// * Hotcue Layer
// * Sampler Layer
// * Pretty colors for meters?
//
///////////////////////////////////////////////////////////////////////////////////

var MiniMixxx = {};

// ==== Friendly User Configuration ====

// Set to true to output debug messages and debug light outputs.
MiniMixxx.DebugMode = true;

// Mixxx's javascript doesn't support .bind natively, so here's a simple version.
MiniMixxx.bind = function (fn, obj) {
    return function () {
        return fn.apply(obj, arguments);
    };
};

// Mode is the parent class of all the Mode objects below (both for encoders and
// buttons).
MiniMixxx.Mode = function (parent, modeName, channel, idx) {
    this.parent = parent;
    this.modeName = modeName;
    this.channel = channel;
    this.idx = idx;
}

// Jog Mode Encoder:
// Input:
// * Spin: adjust jog
// * Shift + spin: quick seek
// Output: spinny angle display.
MiniMixxx.EncoderModeJog = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "JOG", channel, idx);
    this.positionUpdate = false;
    this.curPosition = 0;
    this.trackDurationSec = 0;
    this.color = 13;

    engine.connectControl(this.channel, "track_loaded", MiniMixxx.bind(MiniMixxx.EncoderModeJog.prototype.trackLoadedHandler, this));
    engine.connectControl(this.channel, "playposition", MiniMixxx.bind(MiniMixxx.EncoderModeJog.prototype.playpositionChanged, this));
}
MiniMixxx.EncoderModeJog.prototype.handleSpin = function (velo) {
    if (MiniMixxx.kontrol.shiftActive()) {
        var playPosition = engine.getValue(this.channel, "playposition");
        playPosition += velo / 256.0;
        playPosition = Math.max(Math.min(playPosition, 1.0), 0.0);
        engine.setValue(this.channel, "playposition", playPosition);
    } else {
        engine.setValue(this.channel, "jog", velo / 2.0);
    }
}
MiniMixxx.EncoderModeJog.prototype.handlePress = function (_value) {
}
MiniMixxx.EncoderModeJog.prototype.playpositionChanged = function (value) {
    if (this.trackDurationSec === 0) {
        var samples = engine.getValue(this.channel, "track_loaded");
        if (samples > 0) {
            this.trackLoadedHandler();
        } else {
            // No track loaded, abort
            return;
        }
    }
    this.curPosition = value * this.trackDurationSec;
    this.positionUpdated = true;
};
MiniMixxx.EncoderModeJog.prototype.trackLoadedHandler = function () {
    var trackSamples = engine.getValue(this.channel, "track_samples");
    if (trackSamples === 0) {
        this.trackDurationSec = 0;
        return;
    }
    var trackSampleRate = engine.getValue(this.channel, "track_samplerate");
    // Assume stereo.
    this.trackDurationSec = trackSamples / 2.0 / trackSampleRate;
};
MiniMixxx.EncoderModeJog.prototype.lightSpinny = function () {
    if (!this.positionUpdated) {
        return;
    }
    this.positionUpdated = false;
    var rotations = this.curPosition * (1 / 1.8);  // 1/1.8 is rotations per second (33 1/3 RPM)
    // Calculate angle from 0-1.0
    var angle = rotations - Math.floor(rotations);

    // Angles between 0 and .4 and .6 and 1.0 can be mapped to the indicator.
    // The others have to go on the pfl light.
    if (angle >= 0.6) {
        var midival = (angle - 0.6) * (64.0 / 0.4);
        midi.sendShortMsg(0xB0, this.idx, midival);
        // switch light off
        midi.sendShortMsg(0x90, this.idx, 0x00);
    } else if (angle < 0.4) {
        var midival = angle * (64.0 / 0.4) + 64;
        midi.sendShortMsg(0xB0, this.idx, midival);
        // switch light off
        midi.sendShortMsg(0x90, this.idx, 0x00);
    } else {
        midi.sendShortMsg(0x90, this.idx, this.color);
        // rotary light off
        midi.sendShortMsg(0xb0, this.idx, 0x00);
    }
}
MiniMixxx.EncoderModeJog.prototype.setLights = function () {
    midi.sendShortMsg(0xBF, this.idx, this.color);
    this.lightSpinny();
}

// Gain Mode Encoder:
// Input:  spinning adjusts gain, pressing toggles pfl.
// Output:
// * When idle, displays vu meters
// * when adjusting gain, shows gain.
// * switch shows pfl status.
MiniMixxx.EncoderModeGain = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "GAIN", channel, idx);

    this.color = 0x60;
    this.idleTimer = 0;
    this.showGain = false;
    engine.connectControl(this.channel, "pregain", MiniMixxx.bind(MiniMixxx.EncoderModeGain.prototype.pregainIndicator, this));
    engine.connectControl(this.channel, "pfl", MiniMixxx.bind(MiniMixxx.EncoderModeGain.prototype.pflIndicator, this));
    engine.connectControl(this.channel, "VuMeter", MiniMixxx.bind(MiniMixxx.EncoderModeGain.prototype.vuIndicator, this));
    engine.connectControl(this.channel, "PeakIndicator", MiniMixxx.bind(MiniMixxx.EncoderModeGain.prototype.peakIndicator, this));
}
MiniMixxx.EncoderModeGain.prototype.handleSpin = function (velo) {
    engine.setValue(this.channel, "pregain", engine.getValue(this.channel, "pregain") + velo / 50.0);
    this.pregainIndicator(engine.getValue(this.channel, "pregain"));
}
MiniMixxx.EncoderModeGain.prototype.handlePress = function (value) {
    if (value > 0) {
        script.toggleControl(this.channel, "pfl");
    }
}
MiniMixxx.EncoderModeGain.prototype.vuIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode || this.showGain) {
        return;
    }
    var color = engine.getValue(this.channel, "PeakIndicator") > 0 ? 0x01 : this.color;
    var midiValue = value * 127.0;
    midi.sendShortMsg(0xBF, this.idx, color);
    midi.sendShortMsg(0xB0, this.idx, midiValue);
}
MiniMixxx.EncoderModeGain.prototype.peakIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode || this.showGain) {
        return;
    }
    this.vuIndicator(engine.getValue(this.channel, "VuMeter"));
}
MiniMixxx.EncoderModeGain.prototype.pregainIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    if (this.idleTimer !== 0) {
        engine.stopTimer(this.idleTimer);
    }
    this.showGain = true;
    this.idleTimer = engine.beginTimer(1000, MiniMixxx.bind(function () {
        this.showGain = false;
        this.vuIndicator(engine.getValue(this.channel, "VuMeter"));
    }, this), true);
    midi.sendShortMsg(0xBF, this.idx, this.color);
    midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
}
MiniMixxx.EncoderModeGain.prototype.pflIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    var color = value > 0 ? this.color : 0;
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.EncoderModeGain.prototype.setLights = function () {
    this.vuIndicator(engine.getValue(this.channel, "VuMeter"));
    this.pflIndicator(engine.getValue(this.channel, "pfl"));
}

// Loop Encoder:
// Input:
// * Spin: adjust loop size
// * Shift + Spin: move loop
// * Press: activate if no loop, reloop toggle if there is
// * Shift + Press: always reloop toggle.
// Output: TODO: display loop size, and loop status.
MiniMixxx.EncoderModeLoop = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "LOOP", channel, idx);

    this.color = 46;
    // engine.connectControl(this.channel, "pregain", MiniMixxx.bind(MiniMixxx.EncoderModeLoop.prototype.spinIndicator, this));
    engine.connectControl(this.channel, "loop_enabled", MiniMixxx.bind(MiniMixxx.EncoderModeLoop.prototype.switchIndicator, this));
}
MiniMixxx.EncoderModeLoop.prototype.handleSpin = function (velo) {
    if (MiniMixxx.kontrol.shiftActive()) {
        var beatjumpSize = engine.getValue(this.channel, "beatjump_size");
        if (velo > 0) {
            script.triggerControl(this.channel, "loop_move_" + beatjumpSize + "_forward");
        } else {
            script.triggerControl(this.channel, "loop_move_" + beatjumpSize + "_backward");
        }
    } else {
        if (velo > 0) {
            script.triggerControl(this.channel, "loop_double");
        } else {
            script.triggerControl(this.channel, "loop_halve");
        }
    }
}
MiniMixxx.EncoderModeLoop.prototype.handlePress = function (value) {
    if (value === 0) {
        return;
    }
    var isLoopActive = engine.getValue(this.channel, "loop_enabled");

    if (MiniMixxx.kontrol.shiftActive()) {
        engine.setValue(this.channel, "reloop_toggle", value);
    } else {
        if (isLoopActive) {
            engine.setValue(this.channel, "reloop_toggle", value);
        } else {
            engine.setValue(this.channel, "beatloop_activate", value);
        }
    }
}
// MiniMixxx.EncoderModeLoop.prototype.spinIndicator = function (value, _group, _control) {
//     if (this !== this.parent.activeMode) {
//         return;
//     }
//     midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
// }
MiniMixxx.EncoderModeLoop.prototype.switchIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    var color = value > 0 ? this.color : 0;
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.EncoderModeLoop.prototype.setLights = function () {
    midi.sendShortMsg(0xBF, this.idx, 0);
    this.switchIndicator(engine.getValue(this.channel, "loop_enabled"));
}

// Beat Jump Encoder:
// Input:
//   * Spin: Jump forward / backward
//   * Shift + Spin: Adjust beatjump size
//   * Press: beatloop roll
//   * Shift + Press: reloop and stop
// Output: on if mode is active (I guess?).
MiniMixxx.BeatJump = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "BEATJUMP", channel, idx);

    this.color = 46;
}
MiniMixxx.BeatJump.prototype.handleSpin = function (velo) {
    if (MiniMixxx.kontrol.shiftActive()) {
        var beatjumpSize = engine.getValue(this.channel, "beatjump_size");
        if (velo > 0) {
            engine.setValue(this.channel, "beatjump_size", beatjumpSize * 2);
        } else {
            engine.setValue(this.channel, "beatjump_size", beatjumpSize / 2);
        }
    } else {
        if (velo > 0) {
            script.triggerControl(this.channel, "beatjump_forward");
        } else {
            script.triggerControl(this.channel, "beatjump_backward");
        }
    }
}
MiniMixxx.BeatJump.prototype.handlePress = function (value) {
    if (MiniMixxx.kontrol.shiftActive()) {
        engine.setValue(this.channel, "reloop_andstop", value);
    } else {
        engine.setValue(this.channel, "beatlooproll_activate", value);
    }
}
MiniMixxx.BeatJump.prototype.switchIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    var color = value > 0 ? this.color : 0;
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.BeatJump.prototype.setLights = function () {
    midi.sendShortMsg(0xBF, this.idx, this.color);
    this.switchIndicator(engine.getValue(this.channel, "loop_enabled"));
}

// An Encoder represents a single encoder knob and tracks the active mode.
MiniMixxx.Encoder = function (channel, idx, layerConfig) {
    this.channel = channel;
    this.idx = idx;
    this.encoders = {};
    this.layers = {};
    this.activeMode = {};

    for (var layerName in layerConfig) {
        mode = layerConfig[layerName];
        if (mode === "JOG") {
            this.encoders[mode] = new MiniMixxx.EncoderModeJog(this, channel, idx);
        } else if (mode === "GAIN") {
            this.encoders[mode] = new MiniMixxx.EncoderModeGain(this, channel, idx);
        } else if (mode === "LOOP") {
            this.encoders[mode] = new MiniMixxx.EncoderModeLoop(this, channel, idx);
        } else if (mode === "BEATJUMP") {
            this.encoders[mode] = new MiniMixxx.BeatJump(this, channel, idx);
        } else {
            print("Ignoring unknown encoder mode: " + mode);
            continue;
        }
        this.layers[layerName] = this.encoders[mode];
        // // In library mode, press: loads track (shift: unload)
        // "LIBRARY": new MiniMixxx.EncoderModeLibrary("LIBRARY", channel, idx),

        // // In FX mode, press: toggles
        // "FX": new MiniMixxx.EncoderMode("", channel, idx),
    }
    this.activateLayer("NONE");
}

MiniMixxx.Encoder.prototype.activateLayer = function (layerName) {
    // Only the active mode object should drive lights.
    var mode = this.layers[layerName];
    if (!mode) {
        print("encoder mode not found for layer: " + layerName);
        return;
    }
    this.activeMode = mode;
    this.activeMode.setLights();
}

MiniMixxx.encoderHandler = function (_midino, control, velo, _status, _group) {
    if (velo >= 64) {
        velo -= 128;
    }
    this.kontrol.encoders[control].activeMode.handleSpin(velo);
}

MiniMixxx.encoderButtonHandler = function (_midino, control, value, _status, _group) {
    this.kontrol.encoders[control].activeMode.handlePress(value);
}

// ButtonMode defines some extra state useful for buttons.
MiniMixxx.ButtonMode = function (parent, modeName, channel, idx, colors) {
    MiniMixxx.Mode.call(this, parent, modeName, channel, idx);
    this.colors = colors;
}
MiniMixxx.lightButton = function (idx, value, colors) {
    var color = value > 0 ? colors[1] : colors[0];
    midi.sendShortMsg(0x90, idx, color);
}

MiniMixxx.ButtonModeEmpty = function (parent, channel, idx) {
    MiniMixxx.ButtonMode.call(this, parent, "EMPTY", channel, idx, [0, 0]);
}
MiniMixxx.ButtonModeEmpty.prototype.handlePress = function (value) {
}
MiniMixxx.ButtonModeEmpty.prototype.indicator = function (value, _group, _control) {
    MiniMixxx.lightButton(this.idx, value, this.colors);
}
MiniMixxx.ButtonModeEmpty.prototype.setLights = function () {
    this.indicator(0);
}

MiniMixxx.ButtonModeKeylock = function (parent, channel, idx) {
    MiniMixxx.ButtonMode.call(this, parent, "KEYLOCK", channel, idx, [0, 108]);
    this.keylockPressed = false;
    engine.connectControl(this.channel, "keylock", MiniMixxx.bind(MiniMixxx.ButtonModeKeylock.prototype.indicator, this));
}
MiniMixxx.ButtonModeKeylock.prototype.handlePress = function (value) {
    // shift + keylock resets pitch (in either mode).
    if (MiniMixxx.kontrol.shiftActive()) {
        if (value) {
            engine.setValue(this.channel, "pitch_adjust_set_default", 1);
        }
    } else {
        if (value) {
            // In relative mode on down-press, reset the values and note that
            // the button is pressed.
            this.keylockPressed = true;
            MiniMixxx.kontrol.keyAdjusted[this.channel] = false;
        } else {
            // On release, note that the button is released, and if the key *wasn't* adjusted,
            // activate keylock.
            this.keylockPressed = false;
            if (!MiniMixxx.kontrol.keyAdjusted[this.channel]) {
                script.toggleControl(this.channel, "keylock");
            }
        }
    }

    // Adjust the light on release depending on keylock status.  Down-press is always lit.
    if (!value) {
        var val = engine.getValue(this.channel, "keylock");
        this.indicator(val);
    } else {
        this.indicator(1);
    }
}
MiniMixxx.ButtonModeKeylock.prototype.indicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    MiniMixxx.lightButton(this.idx, value, this.colors);
}
MiniMixxx.ButtonModeKeylock.prototype.setLights = function () {
    this.indicator(engine.getValue(this.channel, "keylock"));
}

MiniMixxx.ButtonModeSync = function (parent, channel, idx) {
    MiniMixxx.ButtonMode.call(this, parent, "SYNC", channel, idx, [0, 84]);
    this.syncPressedTimer = 0;

    engine.connectControl(this.channel, "sync_enabled", MiniMixxx.bind(MiniMixxx.ButtonModeSync.prototype.indicator, this));
}
MiniMixxx.ButtonModeSync.prototype.handlePress = function (value) {
    if (value) {
        // We have to reimplement push-to-lock because it's only defined in the midi code
        // in Mixxx.
        if (engine.getValue(this.channel, "sync_enabled") === 0) {
            engine.setValue(this.channel, "sync_enabled", 1);
            // Start timer to measure how long button is pressed
            this.syncPressedTimer = engine.beginTimer(300, MiniMixxx.bind(function () {
                engine.setValue(this.channel, "sync_enabled", 1);
                // Reset sync button timer state if active
                if (this.syncPressedTimer !== 0) {
                    this.syncPressedTimer = 0;
                }
            }, this), true);
        } else {
            // Deactivate sync lock
            // LED is turned off by the callback handler for sync_enabled
            engine.setValue(this.channel, "sync_enabled", 0);
        }
    } else if (this.syncPressedTimer !== 0) {
        // Timer still running -> stop it and unlight LED
        engine.stopTimer(this.syncPressedTimer);
        engine.setValue(this.channel, "sync_enabled", 0);
    }
}
MiniMixxx.ButtonModeSync.prototype.indicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    MiniMixxx.lightButton(this.idx, value, this.colors);
}
MiniMixxx.ButtonModeSync.prototype.setLights = function () {
    this.indicator(engine.getValue(this.channel, "sync_enabled"));
}

MiniMixxx.ButtonModeShift = function (parent, channel, idx) {
    MiniMixxx.ButtonMode.call(this, parent, "SHIFT", channel, idx, [0, 127]);
    this.shiftActive = false;
}
MiniMixxx.ButtonModeShift.prototype.handlePress = function (value) {
    this.shiftActive = value > 0;
    this.indicator(this.shiftActive);
}
MiniMixxx.ButtonModeShift.prototype.indicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    MiniMixxx.lightButton(this.idx, value, this.colors);
}
MiniMixxx.ButtonModeShift.prototype.setLights = function () {
    this.indicator(this.shiftActive);
}

MiniMixxx.ButtonModeLoopLayer = function (parent, channel, idx) {
    MiniMixxx.ButtonMode.call(this, parent, "LOOPLAYER", channel, idx, [0, 46]);
    this.layerActive = false;
}
MiniMixxx.ButtonModeLoopLayer.prototype.handlePress = function (value) {
    if (value > 0) {
        this.layerActive = !this.layerActive;
    }
    if (this.layerActive) {
        MiniMixxx.kontrol.activateLayer(this.channel, this.modeName);
    } else {
        MiniMixxx.kontrol.activateLayer(this.channel, "NONE");
    }
    this.indicator(this.layerActive);
}
MiniMixxx.ButtonModeLoopLayer.prototype.indicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    MiniMixxx.lightButton(this.idx, value, this.colors);
}
MiniMixxx.ButtonModeLoopLayer.prototype.setLights = function () {
    this.indicator(this.layerActive);
}

MiniMixxx.ButtonModeSamplerLayer = function (parent, channel, idx) {
    MiniMixxx.ButtonMode.call(this, parent, "SAMPLERLAYER", channel, idx, [0, 21]);
    this.layerActive = false;
}
MiniMixxx.ButtonModeSamplerLayer.prototype.handlePress = function (value) {
    if (value > 0) {
        this.layerActive = !this.layerActive;
    }
    if (this.layerActive) {
        MiniMixxx.kontrol.activateLayer(this.channel, this.modeName);
    } else {
        MiniMixxx.kontrol.activateLayer(this.channel, "NONE");
    }
    this.indicator(this.layerActive);
}
MiniMixxx.ButtonModeSamplerLayer.prototype.indicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    MiniMixxx.lightButton(this.idx, value, this.colors);
}
MiniMixxx.ButtonModeSamplerLayer.prototype.setLights = function () {
    this.indicator(this.layerActive);
}

// ButtonModeSampler is the button mode for playing individual sampler decks.
MiniMixxx.ButtonModeSampler = function (parent, channel, idx, samplerNum) {
    MiniMixxx.ButtonMode.call(this, parent, "SAMPLER-" + samplerNum, channel, idx, [0, 22]);
    this.samplerGroup = "[Sampler" + samplerNum + "]";
    engine.connectControl(this.samplerGroup, "track_loaded", MiniMixxx.bind(MiniMixxx.ButtonModeSampler.prototype.indicator, this));
}
MiniMixxx.ButtonModeSampler.prototype.handlePress = function (value) {
    // shift + keylock resets pitch (in either mode).
    if (MiniMixxx.kontrol.shiftActive()) {
        if (value) {
            engine.setValue(this.samplerGroup, "eject", 1);
        }
    } else {
        if (value) {
            engine.setValue(this.samplerGroup, "cue_gotoandplay", 1);
        }
    }
}
MiniMixxx.ButtonModeSampler.prototype.indicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    MiniMixxx.lightButton(this.idx, value, this.colors);
}
MiniMixxx.ButtonModeSampler.prototype.setLights = function () {
    this.indicator(engine.getValue(this.samplerGroup, "track_loaded"));
}

MiniMixxx.Button = function (channel, idx, layerConfig) {
    this.channel = channel;
    this.idx = idx;
    this.buttons = {};
    this.layers = {};
    this.activeMode = {};

    for (var layerName in layerConfig) {
        mode = layerConfig[layerName];
        if (mode === "EMPTY") {
            this.buttons[mode] = new MiniMixxx.ButtonModeEmpty(this, channel, idx);
        } else if (mode === "SYNC") {
            this.buttons[mode] = new MiniMixxx.ButtonModeSync(this, channel, idx);
        } else if (mode === "KEYLOCK") {
            this.buttons[mode] = new MiniMixxx.ButtonModeKeylock(this, channel, idx);
        } else if (mode === "SHIFT") {
            this.buttons[mode] = new MiniMixxx.ButtonModeShift(this, channel, idx);
        } else if (mode === "LOOPLAYER") {
            this.buttons[mode] = new MiniMixxx.ButtonModeLoopLayer(this, channel, idx);
        } else if (mode === "SAMPLERLAYER") {
            this.buttons[mode] = new MiniMixxx.ButtonModeSamplerLayer(this, channel, idx);
        } else if (mode.startsWith("SAMPLER-")) {
            var splitted = mode.split("-");
            var samplerNum = splitted[1];
            this.buttons[mode] = new MiniMixxx.ButtonModeSampler(this, channel, idx, samplerNum);
        } else {
            print("Ignoring unknown button mode: " + mode);
            continue;
        }
        this.layers[layerName] = this.buttons[mode];
    }
    this.activateLayer("NONE");
}
MiniMixxx.Button.prototype.activateLayer = function (layerName) {
    var mode = this.layers[layerName];
    if (!mode) {
        print("button mode not found for layer: " + layerName);
        return;
    }
    this.activeMode = mode;
    this.activeMode.setLights();
}

MiniMixxx.buttonHandler = function (_midino, control, value, _status, _group) {
    button = this.kontrol.buttons[control];
    if (!button) {
        print("unhandled button: " + control);
        return;
    }

    button.activeMode.handlePress(value);
}

MiniMixxx.pitchSliderHandler = function (_midino, _control, value, _status, group) {
    MiniMixxx.kontrol.pitchSliderHandler(value, group);
}

MiniMixxx.Controller = function () {
    this.encoders = {
        0x00: new MiniMixxx.Encoder("[Channel1]", 0, {
            "NONE": "JOG",
            "LOOPLAYER": "LOOP"
        }),
        0x01: new MiniMixxx.Encoder("[Channel1]", 1, {
            "NONE": "GAIN",
            "LOOPLAYER": "BEATJUMP"
        }),
        0x02: new MiniMixxx.Encoder("[Channel2]", 2, {
            "NONE": "GAIN",
            "LOOPLAYER": "LOOP"
        }),
        0x03: new MiniMixxx.Encoder("[Channel2]", 3, {
            "NONE": "JOG",
            "LOOPLAYER": "BEATJUMP"
        })
    };

    this.jogEncoders = {
        "[Channel1]": this.encoders[0x00],
        "[Channel2]": this.encoders[0x03]
    };

    this.buttons = {
        0x05: new MiniMixxx.Button("[Channel1]", 0x05, {
            "NONE": "KEYLOCK",
        }),
        // 0x06: new MiniMixxx.Button("[Channel1]", 0x06, {
        // }),
        // 0x07: new MiniMixxx.Button("[Channel1]", 0x07, {
        // }),
        0x0D: new MiniMixxx.Button("[Channel1]", 0x0D, {
            "NONE": "SYNC"
        }),
        0x0E: new MiniMixxx.Button("[Channel1]", 0x0E, {
            "NONE": "LOOPLAYER",
        }),
        // 0x0F: new MiniMixxx.Button("[Channel1]", 0x0F, {
        // }),

        0x09: new MiniMixxx.Button("[Channel2]", 0x09, {
            "NONE": "KEYLOCK",
            "SAMPLERLAYER": "SAMPLER-1"
        }),
        0x0A: new MiniMixxx.Button("[Channel2]", 0x0A, {
            "NONE": "EMPTY",
            "SAMPLERLAYER": "SAMPLER-2"
        }),
        0x0B: new MiniMixxx.Button("[Channel2]", 0x0B, {
            "NONE": "SAMPLERLAYER"
        }),
        0x11: new MiniMixxx.Button("[Channel2]", 0x11, {
            "NONE": "SYNC",
            "SAMPLERLAYER": "SAMPLER-3"
        }),
        0x12: new MiniMixxx.Button("[Channel2]", 0x12, {
            "NONE": "LOOPLAYER",
            "SAMPLERLAYER": "SAMPLER-4"
        }),
        0x13: new MiniMixxx.Button("[Channel2]", 0x13, {
            "NONE": "SHIFT"
        }),
    }

    this.shiftButton = this.buttons[0x13];

    this.keylockButtons = {
        "[Channel1]": this.buttons[0x05],
        "[Channel2]": this.buttons[0x09]
    };

    this.pitchSliderLastValue = {
        "[Channel1]": -1,
        "[Channel2]": -1
    };

    this.keyAdjusted = {
        "[Channel1]": false,
        "[Channel2]": false
    };

    for (var name in this.buttons) {
        print("setting lights " + name);
        this.buttons[name].activeMode.setLights();
    }

    this.guiTickConnection = engine.makeConnection("[Master]", "guiTick50ms", MiniMixxx.bind(MiniMixxx.Controller.prototype.guiTickHandler, this));
};

MiniMixxx.Controller.prototype.shiftActive = function (channel) {
    return this.shiftButton.buttons["SHIFT"].shiftActive;
}

MiniMixxx.Controller.prototype.keylockPressed = function (channel) {
    return this.keylockButtons[channel].buttons["KEYLOCK"].keylockPressed;
}

MiniMixxx.Controller.prototype.activateLayer = function (channel, layerName) {
    for (var name in this.encoders) {
        encoder = this.encoders[name];
        if (encoder.channel === channel) {
            encoder.activateLayer(layerName);
        }
    }
    for (var name in this.buttons) {
        button = this.buttons[name];
        if (button.channel === channel) {
            button.activateLayer(layerName);
        }
    }
}

MiniMixxx.Controller.prototype.pitchSliderHandler = function (value, group) {
    // Adapt HID value to rate control range.
    value = -1.0 + ((value / 127.0) * 2.0);
    if (this.pitchSliderLastValue[group] === -1) {
        this.pitchSliderLastValue[group] = value;
    } else {
        // If shift is pressed, don't update any values.
        if (this.shiftActive()) {
            this.pitchSliderLastValue[group] = value;
            return;
        }

        // var relVal;
        if (this.keylockPressed(group)) {
            relVal = 1.0 - engine.getValue(group, "pitch_adjust");
        } else {
            relVal = engine.getValue(group, "rate");
        }

        // This can result in values outside -1 to 1, but that is valid for the
        // rate control. This means the entire swing of the rate slider can be
        // outside the range of the widget, but that's ok because the slider still
        // works.
        relVal += value - this.pitchSliderLastValue[group];
        this.pitchSliderLastValue[group] = value;

        if (this.keylockPressed(group)) {
            // To match the pitch change from adjusting the rate, flip the pitch
            // adjustment.
            engine.setValue(group, "pitch_adjust", 1.0 - relVal);
            this.keyAdjusted[group] = true;
        } else {
            engine.setValue(group, "rate", relVal);
        }
    }
}

MiniMixxx.Controller.prototype.guiTickHandler = function () {
    this.jogEncoders["[Channel1]"].encoders["JOG"].lightSpinny();
    this.jogEncoders["[Channel2]"].encoders["JOG"].lightSpinny();
};

MiniMixxx.init = function (_id) {
    this.kontrol = new MiniMixxx.Controller();

    print("MiniMixxx: Init done!");

    if (MiniMixxx.DebugMode) {
        MiniMixxx.debugLights();
    }
};

MiniMixxx.debugLights = function () {
    // midi.sendShortMsg(0xB0, 0x00, 117);
};

MiniMixxx.shutdown = function () {
    for (var i = 0; i < 0x13; i++) {
        if (i < 0x04) {
            midi.sendShortMsg(0xBF, i, 0x00);
        }
        midi.sendShortMsg(0x90, i, 0x00);
    }
};

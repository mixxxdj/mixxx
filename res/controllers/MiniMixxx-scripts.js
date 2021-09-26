///////////////////////////////////////////////////////////////////////////////////
//
// MiniMixxx controller script v1.00
// Author: Owen Williams
//
///////////////////////////////////////////////////////////////////////////////////
//
// TODO:
//
// * Library mode
// * relative pitch slider / key slider
// * wheel spinnies
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
// Input:  spinning adjust jog, pressing has no effect.
// Output: TODO: spinny angle display.
MiniMixxx.EncoderModeJog = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "JOG", channel, idx);
}
MiniMixxx.EncoderModeJog.prototype.handleSpin = function (velo) {
    engine.setValue(this.channel, "jog", velo);
}
MiniMixxx.EncoderModeJog.prototype.handlePress = function (_value) {
}
MiniMixxx.EncoderModeJog.prototype.setLights = function () {
    midi.sendShortMsg(0xbf, this.idx, 0);
    midi.sendShortMsg(0x90, this.idx, 0);
}

// Gain Mode Encoder:
// Input:  spinning adjusts gain, pressing toggles pfl.
// Output: displays gain level and pfl status.
MiniMixxx.EncoderModeGain = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "GAIN", channel, idx);

    this.color = 0x60;
    engine.connectControl(this.channel, "pregain", MiniMixxx.bind(MiniMixxx.EncoderModeGain.prototype.pregainIndicator, this));
    engine.connectControl(this.channel, "pfl", MiniMixxx.bind(MiniMixxx.EncoderModeGain.prototype.pflIndicator, this));
}
MiniMixxx.EncoderModeGain.prototype.handleSpin = function (velo) {
    engine.setValue(this.channel, "pregain", engine.getValue(this.channel, "pregain") + velo / 50.0);
}
MiniMixxx.EncoderModeGain.prototype.handlePress = function (value) {
    if (value > 0) {
        script.toggleControl(this.channel, "pfl");
    }
}
MiniMixxx.EncoderModeGain.prototype.pregainIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    midi.sendShortMsg(0xbf, this.idx, this.color);
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
    this.pregainIndicator(engine.getValue(this.channel, "pregain"));
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
    if (MiniMixxx.kontrol.shiftActive(this.channel)) {
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

    if (MiniMixxx.kontrol.shiftActive(this.channel)) {
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
    // this.pregainIndicator(engine.getValue(this.channel, "pregain"));
    midi.sendShortMsg(0xbf, this.idx, 0);
    this.switchIndicator(engine.getValue(this.channel, "loop_enabled"));
}

// Beat Jump Encoder:
// Input:
//   * Spin: Jump forward / backward
//   * Shift + Spin: Adjust beatjump size
//   * Press: beatloop roll
//   * Shift + Press: reloop and stop
// Output: ??.
MiniMixxx.BeatJump = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "BEATJUMP", channel, idx);

    this.color = 46;
    // engine.connectControl(this.channel, "pregain", MiniMixxx.bind(MiniMixxx.BeatJump.prototype.spinIndicator, this));
    engine.connectControl(this.channel, "loop_enabled", MiniMixxx.bind(MiniMixxx.BeatJump.prototype.switchIndicator, this));
}
MiniMixxx.BeatJump.prototype.handleSpin = function (velo) {
    if (MiniMixxx.kontrol.shiftActive(this.channel)) {
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
    if (MiniMixxx.kontrol.shiftActive(this.channel)) {
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
    midi.sendShortMsg(0xbf, this.idx, 0);
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

MiniMixxx.ButtonModeKeylock = function (parent, channel, idx) {
    MiniMixxx.ButtonMode.call(this, parent, "KEYLOCK", channel, idx, [0, 108]);

    engine.connectControl(this.channel, "keylock", MiniMixxx.bind(MiniMixxx.ButtonModeKeylock.prototype.indicator, this));
}
MiniMixxx.ButtonModeKeylock.prototype.handlePress = function (value) {
    if (value > 0) {
        script.toggleControl(this.channel, "keylock");
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
    MiniMixxx.ButtonMode.call(this, parent, "SHIFT", channel, idx, [0, 108]);
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

MiniMixxx.Button = function (channel, idx, layerConfig) {
    this.channel = channel;
    this.idx = idx;
    this.buttons = {};
    this.layers = {};
    this.activeMode = {};

    for (var layerName in layerConfig) {
        mode = layerConfig[layerName];
        if (mode === "SYNC") {
            this.buttons[mode] = new MiniMixxx.ButtonModeSync(this, channel, idx);
        } else if (mode === "KEYLOCK") {
            this.buttons[mode] = new MiniMixxx.ButtonModeKeylock(this, channel, idx);
        } else if (mode === "SHIFT") {
            this.buttons[mode] = new MiniMixxx.ButtonModeShift(this, channel, idx);
        } else if (mode === "LOOPLAYER") {
            this.buttons[mode] = new MiniMixxx.ButtonModeLoopLayer(this, channel, idx);
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

MiniMixxx.debugLights = function () {
};

MiniMixxx.shutdown = function () {
};

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

    this.buttons = {
        0x05: new MiniMixxx.Button("[Channel1]", 0x05, {
            "NONE": "KEYLOCK"
        }),
        0x0D: new MiniMixxx.Button("[Channel1]", 0x0D, {
            "NONE": "SYNC"
        }),
        0x0E: new MiniMixxx.Button("[Channel1]", 0x0E, {
            "NONE": "LOOPLAYER"
        }),
        0x0F: new MiniMixxx.Button("[Channel1]", 0x0F, {
            "NONE": "SHIFT"
        }),
        0x09: new MiniMixxx.Button("[Channel2]", 0x09, {
            "NONE": "KEYLOCK"
        }),
        0x11: new MiniMixxx.Button("[Channel2]", 0x11, {
            "NONE": "SYNC"
        }),
        0x12: new MiniMixxx.Button("[Channel2]", 0x12, {
            "NONE": "LOOPLAYER"
        }),
        0x13: new MiniMixxx.Button("[Channel2]", 0x13, {
            "NONE": "SHIFT"
        }),
    }

    this.shiftButtons = {
        "[Channel1]": this.buttons[0x0F],
        "[Channel2]": this.buttons[0x13]
    };

    for (var name in this.buttons) {
        this.buttons[name].activeMode.setLights();
    }
};

MiniMixxx.Controller.prototype.shiftActive = function (channel) {
    return this.shiftButtons[channel].buttons["SHIFT"].shiftActive;
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

MiniMixxx.init = function (_id) {
    this.kontrol = new MiniMixxx.Controller();

    print("MiniMixxx: Init done!");

    if (MiniMixxx.DebugMode) {
        MiniMixxx.debugLights();
    }
};

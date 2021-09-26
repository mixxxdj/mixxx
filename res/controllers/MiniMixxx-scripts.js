///////////////////////////////////////////////////////////////////////////////////
//
// MiniMixxx controller script v1.00
// Author: Owen Williams
//
///////////////////////////////////////////////////////////////////////////////////
//
// TODO:
//
// * Implement loop mode
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

MiniMixxx.EncoderModeJog = function (modeName, channel, idx) {
    this.modeName = modeName;
    this.channel = channel;
    this.active = false;
    this.idx = idx;
}
MiniMixxx.EncoderModeJog.prototype.handleSpin = function (velo) {
    engine.setValue(this.channel, "jog", velo);
}
MiniMixxx.EncoderModeJog.prototype.handlePress = function (_value) {
}
MiniMixxx.EncoderModeJog.prototype.setLights = function () {
}

MiniMixxx.EncoderModeGain = function (modeName, channel, idx) {
    this.modeName = modeName;
    this.channel = channel;
    this.active = false;
    this.idx = idx;
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
    if (!this.active) {
        return;
    }
    midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
}
MiniMixxx.EncoderModeGain.prototype.pflIndicator = function (value, _group, _control) {
    if (!this.active) {
        return;
    }
    if (value > 0) {
        midi.sendShortMsg(0x90, this.idx, this.color);
    } else {
        midi.sendShortMsg(0x90, this.idx, 0);
    }
}
MiniMixxx.EncoderModeGain.prototype.setLights = function () {
    this.pregainIndicator(engine.getValue(this.channel, "pregain"));
    this.pflIndicator(engine.getValue(this.channel, "pfl"));
}


MiniMixxx.Encoder = function (channel, idx, availableModes) {
    this.channel = channel;
    this.idx = idx;
    this.modes = {};
    this.activeMode = {};

    for (var i in availableModes) {
        mode = availableModes[i];
        if (mode === "JOG") {
            this.modes[mode] = new MiniMixxx.EncoderModeJog(mode, channel, idx);
        } else if (mode === "GAIN") {
            this.modes[mode] = new MiniMixxx.EncoderModeGain(mode, channel, idx);
        }
        // // In loop size mode, press: toggle loop
        // "LOOPSIZE": new MiniMixxx.EncoderModeLoopSize("LOOPSIZE", channel, idx),

        // // In loop move mode, press: ???
        // "LOOPMOVE": new MiniMixxx.EncoderModeLoopMove("LOOPMOVE", channel, idx),

        // // In library mode, press: loads track (shift: unload)
        // "LIBRARY": new MiniMixxx.EncoderModeLibrary("LIBRARY", channel, idx),

        // // In FX mode, press: toggles
        // "FX": new MiniMixxx.EncoderMode("", channel, idx),
    }
    this.activateMode(availableModes[0]);
}

MiniMixxx.Encoder.prototype.activateMode = function (mode) {
    // Only the active mode object should drive lights.
    for (var name in this.modes) {
        m = this.modes[name];
        m.active = (m.modeName == mode);
    }
    this.activeMode = this.modes[mode];
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

MiniMixxx.ButtonModeKeylock = function (modeName, channel, idx) {
    this.modeName = modeName;
    this.channel = channel;
    this.idx = idx;
    this.active = true;
    this.colors = [0, 108];

    engine.connectControl(this.channel, "keylock", MiniMixxx.bind(MiniMixxx.ButtonModeKeylock.prototype.indicator, this));
}
MiniMixxx.ButtonModeKeylock.prototype.handlePress = function (value) {
    if (value > 0) {
        script.toggleControl(this.channel, "keylock");
    }
}
MiniMixxx.ButtonModeKeylock.prototype.indicator = function (value, _group, _control) {
    if (!this.active) {
        return;
    }
    var color = value > 0 ? this.colors[1] : this.colors[0];
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.ButtonModeKeylock.prototype.setLights = function () {
    this.indicator(engine.getValue(this.channel, "keylock"));
}

MiniMixxx.ButtonModeSync = function (modeName, channel, idx) {
    this.modeName = modeName;
    this.channel = channel;
    this.idx = idx;
    this.active = true;
    this.syncPressedTimer = 0;
    this.colors = [0, 84];

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
    if (!this.active) {
        return;
    }

    var color = value > 0 ? this.colors[1] : this.colors[0];
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.ButtonModeSync.prototype.setLights = function () {
    this.indicator(engine.getValue(this.channel, "sync_enabled"));
}

MiniMixxx.ButtonModeShift = function (modeName, channel, idx) {
    this.modeName = modeName;
    this.channel = channel;
    this.idx = idx;
    this.active = true;
    this.colors = [0, 108];

    engine.connectControl("[Controls]", "touch_shift", MiniMixxx.bind(MiniMixxx.ButtonModeKeylock.prototype.indicator, this));
}
MiniMixxx.ButtonModeShift.prototype.handlePress = function (value) {
    engine.setValue("[Controls]", "touch_shift", value);
}
MiniMixxx.ButtonModeShift.prototype.indicator = function (value, _group, _control) {
    if (!this.active) {
        return;
    }
    var color = value > 0 ? this.colors[1] : this.colors[0];
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.ButtonModeShift.prototype.setLights = function () {
    this.indicator(engine.getValue("[Controls]", "touch_shift"));
}

MiniMixxx.Button = function (channel, idx, availableModes) {
    this.channel = channel;
    this.idx = idx;
    this.modes = {};
    this.activeMode = {};

    for (var i in availableModes) {
        mode = availableModes[i];
        print("add " + mode);
        if (mode === "SYNC") {
            this.modes[mode] = new MiniMixxx.ButtonModeSync(mode, channel, idx);
        } else if (mode === "KEYLOCK") {
            this.modes[mode] = new MiniMixxx.ButtonModeKeylock(mode, channel, idx);
        } else if (mode === "SHIFT") {
            this.modes[mode] = new MiniMixxx.ButtonModeShift(mode, channel, idx);
        }
    }
    this.activateMode(availableModes[0]);
}
MiniMixxx.Button.prototype.activateMode = function (mode) {
    print("activate " + mode);
    for (var name in this.modes) {
        m = this.modes[name];
        m.active = (m.modeName == mode);
    }
    this.activeMode = this.modes[mode];
    print("yeah ? " + this.activeMode);
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
        0x00: new MiniMixxx.Encoder("[Channel1]", 0, ["JOG"]),
        0x01: new MiniMixxx.Encoder("[Channel1]", 1, ["GAIN"]),
        0x02: new MiniMixxx.Encoder("[Channel2]", 2, ["GAIN"]),
        0x03: new MiniMixxx.Encoder("[Channel2]", 3, ["JOG"])
    };

    this.buttons = {
        0x05: new MiniMixxx.Button("[Channel1]", 0x05, ["KEYLOCK"]),
        0x0D: new MiniMixxx.Button("[Channel1]", 0x0D, ["SYNC"]),
        0x0F: new MiniMixxx.Button("[Channel1]", 0x0F, ["SHIFT"]),
        0x09: new MiniMixxx.Button("[Channel2]", 0x09, ["KEYLOCK"]),
        0x11: new MiniMixxx.Button("[Channel2]", 0x11, ["SYNC"]),
        0x13: new MiniMixxx.Button("[Channel2]", 0x13, ["SHIFT"]),
    }

    for (var name in this.buttons) {
        this.buttons[name].activeMode.setLights();
    }
};

MiniMixxx.init = function (_id) {
    this.kontrol = new MiniMixxx.Controller();

    print("MiniMixxx: Init done!");

    if (MiniMixxx.DebugMode) {
        MiniMixxx.debugLights();
    }
};

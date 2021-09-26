///////////////////////////////////////////////////////////////////////////////////
//
// MiniMixxx controller script v1.00
// Author: Owen Williams
//
///////////////////////////////////////////////////////////////////////////////////
//
// TODO:
//
///////////////////////////////////////////////////////////////////////////////////

var MiniMixxx = {};

// ==== Friendly User Configuration ====

// Set to true to output debug messages and debug light outputs.
MiniMixxx.DebugMode = true;

MiniMixxx.Controller = function () {
    this.encoders = {
        0x00: new MiniMixxx.Encoder("[Channel1]", 0),
        0x01: new MiniMixxx.Encoder("[Channel1]", 1),
        0x02: new MiniMixxx.Encoder("[Channel2]", 2),
        0x03: new MiniMixxx.Encoder("[Channel2]", 3)
    };
};

// Mixxx's javascript doesn't support .bind natively, so here's a simple version.
MiniMixxx.bind = function (fn, obj) {
    return function () {
        return fn.apply(obj, arguments);
    };
};

MiniMixxx.EncoderModeJog = function (modeName, channel, idx) {
    this.modeName = modeName
    this.channel = channel
    this.active = false
    this.idx = idx
    // engine.connectControl(this.channel, "pregain", MiniMixxx.bind(MiniMixxx.EncoderModeJog.prototype.pregainIndicator, this));
    // engine.connectControl(this.channel, "pfl", MiniMixxx.bind(MiniMixxx.EncoderModeJog.prototype.pflIndicator, this));
}
MiniMixxx.EncoderModeJog.prototype.handleSpin = function (velo) {
    engine.setValue(this.channel, "jog", velo);
}
MiniMixxx.EncoderModeJog.prototype.handlePress = function (value) {
}
MiniMixxx.EncoderModeJog.prototype.pregainIndicator = function (value, _group, _control) {
    if (!this.active) {
        return;
    }
    // midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
}
MiniMixxx.EncoderModeJog.prototype.pflIndicator = function (value, _group, _control) {
    if (!this.active) {
        return;
    }
    // if (value > 0) {
    //     midi.sendShortMsg(0x90, this.idx, 0x60);
    // } else {
    //     midi.sendShortMsg(0x90, this.idx, 0x00);
    // }
}

MiniMixxx.EncoderModeGain = function (modeName, channel, idx) {
    this.modeName = modeName
    this.channel = channel
    this.active = false
    this.idx = idx
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
        midi.sendShortMsg(0x90, this.idx, 0x60);
    } else {
        midi.sendShortMsg(0x90, this.idx, 0x00);
    }
}

MiniMixxx.Encoder = function (channel, idx) {
    this.channel = channel;
    this.idx = idx;
    this.modes = {
        "JOG": new MiniMixxx.EncoderModeJog("JOG", channel, idx),

    //     // In gain mode, press: toggles pfl
        "GAIN": new MiniMixxx.EncoderModeGain("GAIN", channel, idx)

    //     // // In library mode, press: loads track (shift: unload)
    //     // this.LIBRARY = 2,

    //     // // In loop size mode, press: toggle loop
    //     // this.LOOPSIZE = 3,

    //     // // In loop move mode, press: ???
    //     // this.LOOPMOVE = 4,

    //     // // In FX mode, press: toggles
    //     // this.FX = 5,
    };

    switch (this.idx) {
        case 0:
        case 3:
            this.activateMode("JOG");
            break;
        case 1:
        case 2:
            this.activateMode("GAIN");
            break;
    }
    print("mode? : " + this.modes["GAIN"]);
}

MiniMixxx.Encoder.prototype.activateMode = function (mode) {
    for (var name in this.modes) {
        m = this.modes[name];
        print("activating: " + mode + " " + m.modeName);
        m.active = (m.modeName == mode);
    }
    this.activeMode = this.modes[mode];
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

MiniMixxx.debugLights = function () {
    // Position of encoders
    // midi.sendShortMsg(0xB0, 0x00, 0x11);
    // midi.sendShortMsg(0xB0, 0x01, 0x40);
    // midi.sendShortMsg(0xB0, 0x02, 0x33);
    // midi.sendShortMsg(0xB0, 0x03, 0x60);

    // // Colors of encoders
    // midi.sendShortMsg(0xBF, 0x00, 0x11);
    // midi.sendShortMsg(0xBF, 0x01, 0x40);
    // midi.sendShortMsg(0xBF, 0x02, 0x33);
    // midi.sendShortMsg(0xBF, 0x03, 0x60);

    // // Colors of encoder switches?
    // midi.sendShortMsg(0x90, 0x00, 0x60);
    // midi.sendShortMsg(0x90, 0x01, 0x60);
};

MiniMixxx.shutdown = function () {
};

MiniMixxx.init = function (_id) {
    this.kontrol = new MiniMixxx.Controller();

    print("MiniMixxx: Init done!");

    if (MiniMixxx.DebugMode) {
        MiniMixxx.debugLights();
    }
};

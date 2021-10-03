///////////////////////////////////////////////////////////////////////////////////
//
// MiniMixxx controller script v1.00
// Author: Owen Williams (owilliams@mixxx.org)
//
///////////////////////////////////////////////////////////////////////////////////
//
// TODO:
//  * Issue when activating shifted layer and shifted layer is already active.
//
///////////////////////////////////////////////////////////////////////////////////

var MiniMixxx = {};

MiniMixxx.FXColor = 61;           // Cyan
MiniMixxx.LoopColor = 46;         // Green
MiniMixxx.LibraryColor = 104;     // Purple
MiniMixxx.LibraryFocusColor = 97; // More Purple
MiniMixxx.GainColor = 96;         // Pale Blue
MiniMixxx.JogColor = 13;          // Orange
MiniMixxx.PeakColor = 1;          // Red
MiniMixxx.HotcueColor = 20;       // Light Orange
MiniMixxx.UnsetSamplerColor = 82; // Blue
MiniMixxx.SamplerColor = 70;      // Light Blue
MiniMixxx.MainGainColor = 50;     // Kind of a light green

// Set to true to output debug messages and debug light outputs.
MiniMixxx.DebugMode = false;

// Mixxx's javascript doesn't support .bind natively, so here's a simple version.
MiniMixxx.bind = function (fn, obj) {
    return function () {
        return fn.apply(obj, arguments);
    };
};

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
            this.encoders[mode] = new MiniMixxx.EncoderModeBeatJump(this, channel, idx);
        } else if (mode === "LIBRARY") {
            this.encoders[mode] = new MiniMixxx.EncoderModeLibrary(this, channel, idx);
        } else if (mode === "LIBRARYFOCUS") {
            this.encoders[mode] = new MiniMixxx.EncoderModeLibraryFocus(this, channel, idx);
        } else if (mode.startsWith("EFFECT-")) {
            var splitted = mode.split("-");
            var effectNum = splitted[1];
            this.encoders[mode] = new MiniMixxx.EncoderModeFX(this, channel, idx, effectNum);
        } else if (mode === "MAINGAIN") {
            this.encoders[mode] = new MiniMixxx.EncoderModeMainGain(this, "[Master]", idx);
        } else if (mode === "BALANCE") {
            this.encoders[mode] = new MiniMixxx.EncoderModeBalance(this, "[Master]", idx);
        } else if (mode === "HEADGAIN") {
            this.encoders[mode] = new MiniMixxx.EncoderModeHeadGain(this, "[Master]", idx);
        } else if (mode === "HEADMIX") {
            this.encoders[mode] = new MiniMixxx.EncoderModeHeadMix(this, "[Master]", idx);
        } else {
            print("Ignoring unknown encoder mode: " + mode);
            continue;
        }
        this.layers[layerName] = this.encoders[mode];
        // // In FX mode, press: toggles
        // "FX": new MiniMixxx.EncoderMode("", channel, idx),
    }
    this.activateLayer("NONE", "");
}

MiniMixxx.Encoder.prototype.activateLayer = function (layerName, channel) {
    // Only the active mode object should drive lights.
    var mode = this.layers[layerName];
    if (!mode || (channel && this.channel !== channel)) {
        mode = this.layers["NONE"];
    }
    this.activeMode = mode;
    this.activeMode.setLights();
}

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
// * Press: activate loop or toggle loop if active
// * Shift + Press: loop roll
// Output:
// * spinny angle
// * switch indicates loop active
MiniMixxx.EncoderModeJog = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "JOG", channel, idx);
    this.positionUpdate = false;
    this.curPosition = 0;
    this.trackDurationSec = 0;
    this.baseColor = MiniMixxx.JogColor;
    this.loopColor = MiniMixxx.LoopColor;
    this.color = this.baseColor;

    engine.connectControl(this.channel, "track_loaded", MiniMixxx.bind(MiniMixxx.EncoderModeJog.prototype.trackLoadedHandler, this));
    engine.connectControl(this.channel, "playposition", MiniMixxx.bind(MiniMixxx.EncoderModeJog.prototype.playpositionChanged, this));
    engine.connectControl(this.channel, "loop_enabled", MiniMixxx.bind(MiniMixxx.EncoderModeJog.prototype.loopEnabledChanged, this));
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
MiniMixxx.EncoderModeJog.prototype.handlePress = function (value) {
    var isLoopActive = engine.getValue(this.channel, "loop_enabled");

    if (MiniMixxx.kontrol.shiftActive()) {
        engine.setValue(this.channel, "beatlooproll_activate", value);
    } else {
        if (value === 0) {
            return;
        }
        if (isLoopActive) {
            engine.setValue(this.channel, "reloop_toggle", value);
        } else {
            engine.setValue(this.channel, "beatloop_activate", value);
        }
    }
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
    if (!this.positionUpdated || this !== this.parent.activeMode) {
        return;
    }
    this.positionUpdated = false;
    if (!engine.getValue(this.channel, "track_loaded")) {
        midi.sendShortMsg(0xB0, this.idx, 0x00);
        return;
    }
    var rotations = this.curPosition * (1 / 1.8);  // 1/1.8 is rotations per second (33 1/3 RPM)
    // Calculate angle from 0-1.0
    var angle = rotations - Math.floor(rotations);

    if (engine.getValue(this.channel, "loop_enabled") > 0) {
        var midiColor = this.loopColor;
    } else {
        midiColor = MiniMixxx.vuMeterColor(engine.getValue(this.channel, "VuMeter"));
    }

    // Angles between 0 and .4 and .6 and 1.0 can be mapped to the indicator.
    // The others have to go on the pfl light.
    if (angle >= 0.6) {
        var midival = (angle - 0.6) * (64.0 / 0.4);
        midi.sendShortMsg(0xBF, this.idx, midiColor);
        midi.sendShortMsg(0xB0, this.idx, midival);
    } else if (angle < 0.4) {
        var midival = angle * (64.0 / 0.4) + 64;
        midi.sendShortMsg(0xBF, this.idx, midiColor);
        midi.sendShortMsg(0xB0, this.idx, midival);
    } else {
        // rotary light off
        midi.sendShortMsg(0xB0, this.idx, 0x00);
    }
}
MiniMixxx.EncoderModeJog.prototype.loopEnabledChanged = function () {
    this.setLights();
};
MiniMixxx.EncoderModeJog.prototype.setLights = function () {
    this.positionUpdated = true;
    this.lightSpinny();
    var color = engine.getValue(this.channel, "loop_enabled") > 0 ? this.loopColor : 0x00;
    midi.sendShortMsg(0x90, this.idx, color);
}

// Gain Mode Encoder:
// Input:  spinning adjusts gain, pressing toggles pfl.
// Output:
// * When idle, displays vu meters
// * when adjusting gain, shows gain.
// * switch shows pfl status.
MiniMixxx.EncoderModeGain = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "GAIN", channel, idx);

    this.color = MiniMixxx.GainColor;
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

    var color = MiniMixxx.vuMeterColor(value);
    color = engine.getValue(this.channel, "PeakIndicator") > 0 ? MiniMixxx.PeakColor : color;
    var midiValue = value * 127.0;
    midi.sendShortMsg(0xBF, this.idx, color);
    midi.sendShortMsg(0xB0, this.idx, midiValue);
}
MiniMixxx.EncoderModeGain.prototype.peakIndicator = function (_value, _group, _control) {
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

    this.color = MiniMixxx.LoopColor;
    engine.connectControl(this.channel, "beatloop_size", MiniMixxx.bind(MiniMixxx.EncoderModeLoop.prototype.spinIndicator, this));
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
MiniMixxx.EncoderModeLoop.prototype.spinIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    value = Math.log2(value);
    // log values go from -5 to 9 (14 steps)
    // And we should map from 9 to 127 (118 steps)
    value = Math.floor((value + 5) / 14.0 * 118 + 9);
    midi.sendShortMsg(0xB0, this.idx, value);
}
MiniMixxx.EncoderModeLoop.prototype.switchIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    var color = value > 0 ? this.color : 0;
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.EncoderModeLoop.prototype.setLights = function () {
    midi.sendShortMsg(0xBF, this.idx, this.color);
    this.spinIndicator(engine.getValue(this.channel, "beatloop_size"));
    this.switchIndicator(engine.getValue(this.channel, "loop_enabled"));
}

// Beat Jump Encoder:
// Input:
//   * Spin: Adjust beatjump size
//   * Shift + Spin: Jump forward / backward
//   * Press: beatloop roll
//   * Shift + Press: reloop and stop
// Output: on if mode is active (I guess?).
MiniMixxx.EncoderModeBeatJump = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "BEATJUMP", channel, idx);

    this.color = MiniMixxx.LoopColor;
    engine.connectControl(this.channel, "beatjump_size", MiniMixxx.bind(MiniMixxx.EncoderModeBeatJump.prototype.spinIndicator, this));
}
MiniMixxx.EncoderModeBeatJump.prototype.handleSpin = function (velo) {
    if (MiniMixxx.kontrol.shiftActive()) {
        if (velo > 0) {
            script.triggerControl(this.channel, "beatjump_forward");
        } else {
            script.triggerControl(this.channel, "beatjump_backward");
        }
    } else {
        var beatjumpSize = engine.getValue(this.channel, "beatjump_size");
        if (velo > 0) {
            beatjumpSize *= 2;
        } else {
            beatjumpSize /= 2;
        }
        engine.setValue(this.channel, "beatjump_size", Math.max(Math.min(beatjumpSize, 512), 1.0 / 32.0));
    }
}
MiniMixxx.EncoderModeBeatJump.prototype.handlePress = function (value) {
    if (MiniMixxx.kontrol.shiftActive()) {
        engine.setValue(this.channel, "reloop_andstop", value);
    } else {
        engine.setValue(this.channel, "beatlooproll_activate", value);
    }
}
MiniMixxx.EncoderModeBeatJump.prototype.spinIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    value = Math.log2(value);
    // log values go from -5 to 9 (14 steps)
    // And we should map from 9 to 127 (118 steps)
    value = Math.floor((value + 5) / 14.0 * 118 + 9);
    midi.sendShortMsg(0xB0, this.idx, value);
}
MiniMixxx.EncoderModeBeatJump.prototype.switchIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    var color = value > 0 ? this.color : 0;
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.EncoderModeBeatJump.prototype.setLights = function () {
    midi.sendShortMsg(0xBF, this.idx, this.color);
    this.spinIndicator(engine.getValue(this.channel, "beatjump_size"));
    midi.sendShortMsg(0x90, this.idx, this.color);
}

// Library Encoder:
// Input:
//   * Spin: scroll up/down
//   * Shift + Spin: scroll horizontal
//   * Press: load track
//   * Shift + Press: eject track
// Output: ???
MiniMixxx.EncoderModeLibrary = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "LIBRARY", channel, idx);
    this.color = MiniMixxx.LibraryColor;
}
MiniMixxx.EncoderModeLibrary.prototype.handleSpin = function (velo) {
    if (MiniMixxx.kontrol.shiftActive()) {
        engine.setValue("[Library]", "MoveHorizontal", velo);
    } else {
        engine.setValue("[Library]", "MoveVertical", velo);
    }
}
MiniMixxx.EncoderModeLibrary.prototype.handlePress = function (value) {
    if (MiniMixxx.kontrol.shiftActive()) {
        engine.setValue(this.channel, "eject", value);
    } else {
        engine.setValue(this.channel, "LoadSelectedTrack", value);
    }
}
MiniMixxx.EncoderModeLibrary.prototype.setLights = function () {
    midi.sendShortMsg(0xBF, this.idx, this.color);
    midi.sendShortMsg(0xB0, this.idx, 0x7F);
    midi.sendShortMsg(0x90, this.idx, this.color);
}

// Library Focus Encoder:
// Input:
//   * Spin: move focus forward / back
//   * Shift + Spin: scroll through track
//   * Press: ?
//   * Shift + Press: search history select?
// Output: ???
MiniMixxx.EncoderModeLibraryFocus = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "LIBRARYFOCUS", channel, idx);
    this.color = MiniMixxx.LibraryFocusColor;
}
MiniMixxx.EncoderModeLibraryFocus.prototype.handleSpin = function (velo) {
    if (MiniMixxx.kontrol.shiftActive()) {
        var playPosition = engine.getValue(this.channel, "playposition");
        playPosition += velo / 256.0;
        playPosition = Math.max(Math.min(playPosition, 1.0), 0.0);
        engine.setValue(this.channel, "playposition", playPosition);
    } else {
        if (velo > 0) {
            engine.setValue("[Library]", "MoveFocusForward", 1);
        } else if (velo < 0) {
            engine.setValue("[Library]", "MoveFocusBackward", 1);
        }
    }
}
MiniMixxx.EncoderModeLibraryFocus.prototype.handlePress = function (value) {
    if (MiniMixxx.kontrol.shiftActive()) {
        engine.setValue("[Library]", "search_history_selector", value);
    } else {
        engine.setValue(this.channel, "LoadSelectedTrack", value);
    }
}
MiniMixxx.EncoderModeLibraryFocus.prototype.setLights = function () {
    midi.sendShortMsg(0xBF, this.idx, this.color);
    midi.sendShortMsg(0xB0, this.idx, 0x00);
    midi.sendShortMsg(0x90, this.idx, this.color);
}

// FX Encoder:
// Input:
//   * Spin: Adjust meta knob
//   * Shift + Spin: n/a
//   * Press: activate
//   * Shift + Press: n/a
// Output:
// * fx unit meta
// * switch on if fx is active.
MiniMixxx.EncoderModeFX = function (parent, channel, idx, effectNum) {
    MiniMixxx.Mode.call(this, parent, "FX", channel, idx);

    this.color = MiniMixxx.FXColor;
    if (effectNum === "SUPER") {
        this.effectGroup = "[EffectRack1_EffectUnit1]";
        this.effectKey = "super1";
    } else {
        this.effectGroup = "[EffectRack1_EffectUnit1_Effect" + effectNum + "]";
        this.effectKey = "meta";
    }

    engine.connectControl(this.effectGroup, this.effectKey, MiniMixxx.bind(MiniMixxx.EncoderModeFX.prototype.spinIndicator, this));
    engine.connectControl(this.effectGroup, "enabled", MiniMixxx.bind(MiniMixxx.EncoderModeFX.prototype.switchIndicator, this));
}
MiniMixxx.EncoderModeFX.prototype.handleSpin = function (velo) {
    engine.setValue(this.effectGroup, this.effectKey, engine.getValue(this.effectGroup, this.effectKey) + velo / 50.0);
}
MiniMixxx.EncoderModeFX.prototype.handlePress = function (value) {
    if (value > 0) {
        script.toggleControl(this.effectGroup, "enabled");
    }
}
MiniMixxx.EncoderModeFX.prototype.spinIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    midi.sendShortMsg(0xB0, this.idx, Math.floor(value * 127.0));
}
MiniMixxx.EncoderModeFX.prototype.switchIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    var color = value > 0 ? this.color : 0;
    midi.sendShortMsg(0x90, this.idx, color);
}
MiniMixxx.EncoderModeFX.prototype.setLights = function () {
    midi.sendShortMsg(0xBF, this.idx, this.color);
    this.spinIndicator(engine.getValue(this.effectGroup, this.effectKey));
    this.switchIndicator(engine.getValue(this.effectGroup, "enabled"));
}

// Primary Gain:
// Input:
//   * Spin: Adjust primary output up/down
//   * Press: reset primary output
// Output: gain
MiniMixxx.EncoderModeMainGain = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "MAINGAIN", channel, idx);
    this.color = MiniMixxx.MainGainColor;

    engine.connectControl("[Master]", "gain", MiniMixxx.bind(MiniMixxx.EncoderModeMainGain.prototype.gainIndicator, this));
}
MiniMixxx.EncoderModeMainGain.prototype.handleSpin = function (velo) {
    engine.setValue("[Master]", "gain", engine.getValue("[Master]", "gain") + .02 *velo);
}
MiniMixxx.EncoderModeMainGain.prototype.handlePress = function (value) {
    if (value === 0) {
        return;
    }
    engine.setValue("[Master]", "gain", 1.0);
}
MiniMixxx.EncoderModeMainGain.prototype.gainIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }

    midi.sendShortMsg(0xBF, this.idx, this.color);
    midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
}
MiniMixxx.EncoderModeMainGain.prototype.setLights = function () {
    midi.sendShortMsg(0x90, this.idx, 0x00);
    this.gainIndicator(engine.getValue("[Master]", "gain"));
}

// Headphone Gain:
// Input:
//   * Spin: Adjust headphone output up/down
//   * Press: reset headphone output
// Output: gain
MiniMixxx.EncoderModeHeadGain = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "HEADGAIN", channel, idx);
    this.color = MiniMixxx.MainGainColor;

    engine.connectControl("[Master]", "headGain", MiniMixxx.bind(MiniMixxx.EncoderModeHeadGain.prototype.gainIndicator, this));
}
MiniMixxx.EncoderModeHeadGain.prototype.handleSpin = function (velo) {
    engine.setValue("[Master]", "headGain", engine.getValue("[Master]", "headGain") + .02 *velo);
}
MiniMixxx.EncoderModeHeadGain.prototype.handlePress = function (value) {
    if (value === 0) {
        return;
    }
    engine.setValue("[Master]", "headGain", 1.0);
}
MiniMixxx.EncoderModeHeadGain.prototype.gainIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }

    midi.sendShortMsg(0xBF, this.idx, this.color);
    midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
}
MiniMixxx.EncoderModeHeadGain.prototype.setLights = function () {
    midi.sendShortMsg(0x90, this.idx, 0x00);
    this.gainIndicator(engine.getValue("[Master]", "headGain"));
}

// Balance:
// Input:
//   * Spin: Adjust balance
//   * Press: reset balance
// Output: value
MiniMixxx.EncoderModeBalance = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "BALANCE", channel, idx);
    this.color = MiniMixxx.MainGainColor;

    engine.connectControl("[Master]", "balance", MiniMixxx.bind(MiniMixxx.EncoderModeBalance.prototype.balIndicator, this));
}
MiniMixxx.EncoderModeBalance.prototype.handleSpin = function (velo) {
    engine.setValue("[Master]", "balance", engine.getValue("[Master]", "balance") + .01 * velo);
}
MiniMixxx.EncoderModeBalance.prototype.handlePress = function (value) {
    if (value === 0) {
        return;
    }
    engine.setValue("[Master]", "balance", 0.0);
}
MiniMixxx.EncoderModeBalance.prototype.balIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }

    midi.sendShortMsg(0xBF, this.idx, this.color);
    midi.sendShortMsg(0xB0, this.idx, (value + 1.0) * 64.0 - 1.0);
}
MiniMixxx.EncoderModeBalance.prototype.setLights = function () {
    this.balIndicator(engine.getValue("[Master]", "balance"));
    midi.sendShortMsg(0x90, this.idx, this.color);
}

// Headphone Mix:
// Input:
//   * Spin: Adjust headphone mix
//   * Press: reset headphone mix
// Output: value
MiniMixxx.EncoderModeHeadMix = function (parent, channel, idx) {
    MiniMixxx.Mode.call(this, parent, "HEADMIX", channel, idx);
    this.color = MiniMixxx.MainGainColor;

    engine.connectControl("[Master]", "headMix", MiniMixxx.bind(MiniMixxx.EncoderModeHeadMix.prototype.mixIndicator, this));
}
MiniMixxx.EncoderModeHeadMix.prototype.handleSpin = function (velo) {
    engine.setValue("[Master]", "headMix", engine.getValue("[Master]", "headMix") + .01 * velo);
}
MiniMixxx.EncoderModeHeadMix.prototype.handlePress = function (value) {
    if (value === 0) {
        return;
    }
    engine.setValue("[Master]", "headMix", 0.0);
}
MiniMixxx.EncoderModeHeadMix.prototype.mixIndicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }

    midi.sendShortMsg(0xBF, this.idx, this.color);
    midi.sendShortMsg(0xB0, this.idx, (value + 1.0) * 64.0 - 1.0);
}
MiniMixxx.EncoderModeHeadMix.prototype.setLights = function () {
    this.mixIndicator(engine.getValue("[Master]", "headMix"));
    midi.sendShortMsg(0x90, this.idx, this.color);
}

// Button represents a single physical button and contains all of the mode objects that
// drive its behavior.
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
            this.buttons[mode] = new MiniMixxx.ButtonModeShift(this, "", idx);
        } else if (mode.startsWith("SAMPLER-")) {
            var splitted = mode.split("-");
            var samplerNum = splitted[1];
            this.buttons[mode] = new MiniMixxx.ButtonModeSampler(this, "", idx, samplerNum);
        } else if (mode.startsWith("HOTCUE-")) {
            var splitted = mode.split("-");
            var hotcueNum = splitted[1];
            this.buttons[mode] = new MiniMixxx.ButtonModeHotcue(this, channel, idx, hotcueNum);
        } else if (mode.startsWith("FX-")) {
            // We only use FX 1 for now.
            this.buttons[mode] = new MiniMixxx.ButtonModeFX(this, channel, idx);
        } else if (mode === "LOOPLAYER") {
            this.buttons[mode] = new MiniMixxx.ButtonModeLayer(this, "LOOPLAYER", channel, idx, [0, MiniMixxx.LoopColor]);
        } else if (mode === "SAMPLERLAYER-HOTCUE2LAYER") {
            this.buttons[mode] = new MiniMixxx.ButtonModeLayer(this, "SAMPLERLAYER", "", idx, [0, MiniMixxx.SamplerColor]);
            this.buttons[mode].addShiftedButton(this, "HOTCUELAYER", channel, idx, [0, MiniMixxx.HotcueColor]);
            var shiftedButton = this.buttons[mode].shiftedButton;
            this.layers[shiftedButton.layerName] = shiftedButton;
        } else if (mode === "LIBRARYLAYER-MAINGAINLAYER") {
            this.buttons[mode] = new MiniMixxx.ButtonModeLayer(this, "LIBRARYLAYER", "", idx, [0, MiniMixxx.LibraryColor]);
            this.buttons[mode].addShiftedButton(this, "MAINGAINLAYER", "", idx, [0, MiniMixxx.MainGainColor]);
            shiftedButton = this.buttons[mode].shiftedButton;
            this.layers[shiftedButton.layerName] = shiftedButton;
        } else if (mode === "FXLAYER-HOTCUE1LAYER") {
            this.buttons[mode] = new MiniMixxx.ButtonModeLayer(this, "FXLAYER", "", idx, [0, MiniMixxx.FXColor]);
            this.buttons[mode].addShiftedButton(this, "HOTCUELAYER", channel, idx, [0, MiniMixxx.HotcueColor]);
            shiftedButton = this.buttons[mode].shiftedButton;
            this.layers[shiftedButton.layerName] = shiftedButton;
        } else {
            print("Ignoring unknown button mode: " + mode);
            continue;
        }
        this.layers[layerName] = this.buttons[mode];
    }
    this.activateLayer("NONE", "");
}
MiniMixxx.Button.prototype.activateLayer = function (layerName, channel) {
    // We need to go through and update all the layer buttons that we might own.
    for (var name in this.buttons) {
        var button = this.buttons[name];
        if (button instanceof MiniMixxx.ButtonModeLayer) {
            button.setActive(layerName, channel);
        }
        if (button.shiftedButton instanceof MiniMixxx.ButtonModeLayer) {
            button.shiftedButton.setActive(layerName, channel);
        }
    }

    var mode = this.layers[layerName];
    if (mode) {
        if (channel === "" || mode.channel === channel) {
            this.activeMode = mode;
        }
    } else if (channel === "") {
        this.activeMode = this.layers["NONE"];
    }
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

// ButtonMode defines some extra state useful for buttons.
MiniMixxx.ButtonMode = function (parent, modeName, channel, idx, colors) {
    MiniMixxx.Mode.call(this, parent, modeName, channel, idx);
    this.colors = colors;
}
MiniMixxx.lightButton = function (idx, value, colors) {
    var color = value > 0 ? colors[1] : colors[0];
    midi.sendShortMsg(0x90, idx, color);
}

// ButtonModeEmpty is a placeholder that does nothing.
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

// ButtonModeKeylock enables and disables keylock.
// If held, this button causes the pitch slider to act as a musical key adjustment.
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

// ButtonModeSync
// Input: press to do a one-off beatsync.  Press and hold to enable Sync Lock.
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

// ButtonModeShift: the shift button
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

// ButtonModeLayer activates the layer with the given name.
// channel can be empty if it should apply to both channels.
MiniMixxx.ButtonModeLayer = function (parent, layerName, channel, idx, colors) {
    MiniMixxx.ButtonMode.call(this, parent, layerName, channel, idx, colors);
    // A ButtonModeLayer can contain a child of itself that is activated when Shift is held.
    this.shiftedButton = null;
    this.requireShift = false;
    this.layerActive = false;
}
// Add a second layer option to the shift-press for this button.
MiniMixxx.ButtonModeLayer.prototype.addShiftedButton = function (parent, layerName, channel, idx, colors) {
    // A shifted button could hypothetically have its own shifted button but let's not go there.
    this.shiftedButton = new MiniMixxx.ButtonModeLayer(parent, layerName, channel, idx, colors);
    this.shiftedButton.requireShift = true;
}
MiniMixxx.ButtonModeLayer.prototype.handlePress = function (value) {
    // If we have a shiftedButton, pass off to it if shift is held OR if that shifted
    // layer is active.  That way a simple press of the active shifted layer will turn it off
    // instead of activating the unshifted layer.
    if (this.shiftedButton) {
        if (MiniMixxx.kontrol.shiftActive() || this.shiftedButton.layerActive) {
            this.shiftedButton.handlePress(value);
            return;
        }
    }
    if (value === 0) {
        return;
    }
    this.layerActive = !this.layerActive;
    this.setLights();
    if (this.layerActive) {
        MiniMixxx.kontrol.activateLayer(this.modeName, this.channel);
    } else {
        MiniMixxx.kontrol.activateLayer("NONE", this.channel);
    }
}
MiniMixxx.ButtonModeLayer.prototype.setActive = function (layerName, channel) {
    // if it's got a channel, and so do we, no need to change if channel mismatch
    if (channel && this.channel && this.channel !== channel) {
        return;
    }

    // Set based on layer name match.
    this.layerActive = (this.modeName === layerName);
    this.setLights();
}
MiniMixxx.ButtonModeLayer.prototype.indicator = function (value, _group, _control) {
    MiniMixxx.lightButton(this.idx, this.layerActive, this.colors);
}
MiniMixxx.ButtonModeLayer.prototype.setLights = function () {
    if (this.shiftedButton && this.shiftedButton.layerActive) {
        this.shiftedButton.indicator();
        return;
    }
    this.indicator();
}

// ButtonModeSampler is the button mode for playing individual sampler decks.
// Shift+Press to eject the sample.
MiniMixxx.ButtonModeSampler = function (parent, channel, idx, samplerNum) {
    MiniMixxx.ButtonMode.call(this, parent, "SAMPLER-" + samplerNum, channel, idx, [MiniMixxx.UnsetSamplerColor, MiniMixxx.SamplerColor]);
    this.samplerGroup = "[Sampler" + samplerNum + "]";
    engine.connectControl(this.samplerGroup, "track_loaded", MiniMixxx.bind(MiniMixxx.ButtonModeSampler.prototype.indicator, this));
}
MiniMixxx.ButtonModeSampler.prototype.handlePress = function (value) {
    // shift+press ejects the sample.
    if (MiniMixxx.kontrol.shiftActive()) {
        if (value) {
            script.toggleControl(this.samplerGroup, "eject");
        }
    } else {
        if (value) {
            script.toggleControl(this.samplerGroup, "cue_gotoandplay");
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

// ButtonModeHotcue is the button mode for playing individual sampler decks.
// Shift+Press to eject the sample.
MiniMixxx.ButtonModeHotcue = function (parent, channel, idx, hotcueNum) {
    MiniMixxx.ButtonMode.call(this, parent, "HOTCUE-" + hotcueNum, channel, idx, [0x7f, MiniMixxx.HotcueColor]);
    this.hotcueNum = hotcueNum;
    this.keyPrefix = "hotcue_" + hotcueNum + "_";
    engine.connectControl(this.channel, this.keyPrefix + "enabled", MiniMixxx.bind(MiniMixxx.ButtonModeHotcue.prototype.indicator, this));
    engine.connectControl(this.channel, this.keyPrefix + "color", MiniMixxx.bind(MiniMixxx.ButtonModeHotcue.prototype.indicator, this));
}
MiniMixxx.ButtonModeHotcue.prototype.handlePress = function (value) {
    // shift + press unsets the hotcue
    if (MiniMixxx.kontrol.shiftActive()) {
        if (value) {
            script.toggleControl(this.channel, this.keyPrefix + "clear");
        }
    } else {
        engine.setValue(this.channel, this.keyPrefix + "activate", value);
    }
}
MiniMixxx.ButtonModeHotcue.prototype.indicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    if (!engine.getValue(this.channel, this.keyPrefix + "enabled")) {
        midi.sendShortMsg(0x90, this.idx, 0x7f);
        return;
    }
    var colorCode = engine.getValue(this.channel, this.keyPrefix + "color");
    var midiColor = MiniMixxx.kontrol.colorMapper.getValueForNearestColor(colorCode);
    midi.sendShortMsg(0x90, this.idx, midiColor);
}
MiniMixxx.ButtonModeHotcue.prototype.setLights = function () {
    this.indicator(engine.getValue(this.channel, this.keyPrefix + "enabled"));
}

// ButtonModeFX enables the FX unit for this deck.
MiniMixxx.ButtonModeFX = function (parent, channel, idx) {
    MiniMixxx.ButtonMode.call(this, parent, "FX", channel, idx, [0, MiniMixxx.FXColor]);
    engine.connectControl("[EffectRack1_EffectUnit1]", "group_" + this.channel + "_enable", MiniMixxx.bind(MiniMixxx.ButtonModeFX.prototype.indicator, this));
}
MiniMixxx.ButtonModeFX.prototype.handlePress = function (value) {
    if (value > 0) {
        script.toggleControl("[EffectRack1_EffectUnit1]", "group_" + this.channel + "_enable");
    }
}
MiniMixxx.ButtonModeFX.prototype.indicator = function (value, _group, _control) {
    if (this !== this.parent.activeMode) {
        return;
    }
    MiniMixxx.lightButton(this.idx, value, this.colors);
}
MiniMixxx.ButtonModeFX.prototype.setLights = function () {
    this.indicator(engine.getValue("[EffectRack1_EffectUnit1]", "group_" + this.channel + "_enable"));
}

MiniMixxx.Controller = function () {
    this.encoders = {
        0x00: new MiniMixxx.Encoder("[Channel1]", 0, {
            "NONE": "JOG",
            "LOOPLAYER": "LOOP",
            "LIBRARYLAYER": "LIBRARYFOCUS",
            "FXLAYER": "EFFECT-1",
            "MAINGAINLAYER": "BALANCE",
        }),
        0x01: new MiniMixxx.Encoder("[Channel1]", 1, {
            "NONE": "GAIN",
            "LOOPLAYER": "BEATJUMP",
            "LIBRARYLAYER": "LIBRARY",
            "FXLAYER": "EFFECT-2",
            "MAINGAINLAYER": "MAINGAIN",
        }),
        0x02: new MiniMixxx.Encoder("[Channel2]", 2, {
            "NONE": "GAIN",
            "LOOPLAYER": "LOOP",
            "LIBRARYLAYER": "LIBRARY",
            "FXLAYER": "EFFECT-3",
            "MAINGAINLAYER": "HEADGAIN",
        }),
        0x03: new MiniMixxx.Encoder("[Channel2]", 3, {
            "NONE": "JOG",
            "LOOPLAYER": "BEATJUMP",
            "LIBRARYLAYER": "LIBRARYFOCUS",
            "FXLAYER": "EFFECT-SUPER",
            "MAINGAINLAYER": "HEADMIX",
        })
    };

    this.jogEncoders = {
        "[Channel1]": this.encoders[0x00],
        "[Channel2]": this.encoders[0x03]
    };

    this.buttons = {
        // Deck 1
        // 0x04: hard-coded CUE1
        0x05: new MiniMixxx.Button("[Channel1]", 0x05, {
            "NONE": "KEYLOCK",
            "SAMPLERLAYER": "SAMPLER-1",
            "HOTCUELAYER": "HOTCUE-1",
        }),
        0x06: new MiniMixxx.Button("[Channel1]", 0x06, {
            "NONE": "FX-1",
            "SAMPLERLAYER": "SAMPLER-2",
            "HOTCUELAYER": "HOTCUE-2",
        }),
        //0x0C: hard-coded PLAY1
        0x0D: new MiniMixxx.Button("[Channel1]", 0x0D, {
            "NONE": "SYNC",
            "SAMPLERLAYER": "SAMPLER-5",
            "HOTCUELAYER": "HOTCUE-3",
        }),
        0x0E: new MiniMixxx.Button("[Channel1]", 0x0E, {
            "NONE": "LOOPLAYER",
            "SAMPLERLAYER": "SAMPLER-6",
            "HOTCUELAYER": "HOTCUE-4",
        }),

        // 0x07: hard-coded CUE2
        0x08: new MiniMixxx.Button("[Channel2]", 0x08, {
            "NONE": "KEYLOCK",
            "SAMPLERLAYER": "SAMPLER-3",
            "HOTCUELAYER": "HOTCUE-1",
        }),
        0x09: new MiniMixxx.Button("[Channel2]", 0x09, {
            "NONE": "FX-1",
            "SAMPLERLAYER": "SAMPLER-4",
            "HOTCUELAYER": "HOTCUE-2",
        }),
        //0x0F: hard coded PLAY2
        0x10: new MiniMixxx.Button("[Channel2]", 0x10, {
            "NONE": "SYNC",
            "SAMPLERLAYER": "SAMPLER-7",
            "HOTCUELAYER": "HOTCUE-3",
        }),
        0x11: new MiniMixxx.Button("[Channel2]", 0x11, {
            "NONE": "LOOPLAYER",
            "SAMPLERLAYER": "SAMPLER-8",
            "HOTCUELAYER": "HOTCUE-4",
        }),

        // Righthand mode buttons
        0x0A: new MiniMixxx.Button("[Channel1]", 0x0A, {
            "NONE": "FXLAYER-HOTCUE1LAYER",
        }),
        0x0B: new MiniMixxx.Button("[Channel2]", 0x0B, {
            "NONE": "SAMPLERLAYER-HOTCUE2LAYER"
        }),
        0x12: new MiniMixxx.Button("", 0x12, {
            "NONE": "LIBRARYLAYER-MAINGAINLAYER",
        }),
        0x13: new MiniMixxx.Button("", 0x13, {
            "NONE": "SHIFT"
        }),
    }

    this.shiftButton = this.buttons[0x13];

    this.keylockButtons = {
        "[Channel1]": this.buttons[0x05],
        "[Channel2]": this.buttons[0x08]
    };

    this.pitchSliderLastValue = {
        "[Channel1]": -1,
        "[Channel2]": -1
    };

    this.keyAdjusted = {
        "[Channel1]": false,
        "[Channel2]": false
    };

    // Default palette from: https://gitlab.com/yaeltex/ytx-controller/-/blob/develop/ytx-controller/headers/types.h#L128
    var palette = [
        0x000000,         // 00
        0xf2c0c0,
        0xf25858,
        0xf2a5a5,
        0xf22cc0,
        0xf26e58,
        0xf2b0a5,
        0xf24dc0,
        0xf28458,
        0xf2bba5,
        0xf26ec0,          // 10
        0xf29a58,
        0xf2c6a5,
        0xf28fc0,
        0xf2b058,
        0xf2d1a5,
        0xf2b0c0,
        0xf2c658,
        0xf2dca5,
        0xf2d1c0,
        0xf2dc58,         // 20
        0xf2e7a5,
        0xf2f2c0,
        0xf2f258,
        0xf2f2a5,
        0xd1f2c0,
        0xdcf258,
        0xd1f2a5,
        0xb0f2c0,
        0xc6f258,
        0xb0f2a5,         // 30
        0x8ff2c0,
        0xb0f258,
        0x8ff2a5,
        0x6ef2c0,
        0x9af258,
        0x6ef2a5,
        0x4df2c0,
        0x84f258,
        0x4df2a5,
        0x2cf2c0,          // 40
        0x6ef258,
        0x2cf2a5,
        0xc0f2c0,
        0x58f258,
        0xc0f2a5,
        0xc0f22c,
        0x58f26e,
        0xc0f2b0,
        0xc0f24d,
        0x58f284,         // 50
        0xc0f2bb,
        0xc0f26e,
        0x58f29a,
        0xc0f2c6,
        0xc0f28f,
        0x58f2b0,
        0xc0f2d1,
        0xc0f2b0,
        0x58f2c6,
        0xc0f2dc,          // 60
        0xc0f2d1,
        0x58f2dc,
        0xc0f2e7,
        0xc0f2f2,
        0x58f2f2,
        0xc0f2f2,
        0xc0d1f2,
        0x58dcf2,
        0xc0e7f2,
        0xc0b0f2,          // 70
        0x58c6f2,
        0xc0dcf2,
        0xc08ff2,
        0x58b0f2,
        0xc0d1f2,
        0xc06ef2,
        0x589af2,
        0xc0c6f2,
        0xc04df2,
        0x5884f2,         // 80
        0xc0bbf2,
        0xc02cf2,
        0x586ef2,
        0xc0b0f2,
        0xc0c0f2,
        0x5858f2,
        0xc0a5f2,
        0x2cc0f2,
        0x6e58f2,
        0x2ca5f2,         // 90
        0x4dc0f2,
        0x8458f2,
        0x4da5f2,
        0x6ec0f2,
        0x9a58f2,
        0x6ea5f2,
        0x8fc0f2,
        0xb058f2,
        0x8fa5f2,
        0xb0c0f2,          // 100
        0xc658f2,
        0xb0a5f2,
        0xd1c0f2,
        0xdc58f2,
        0xd1a5f2,
        0xf2c0f2,
        0xf258f2,
        0xf2a5f2,
        0xf2c0d1,
        0xf258dc,         // 110
        0xf2a5e7,
        0xf2c0b0,
        0xf258c6,
        0xf2a5dc,
        0xf2c08f,
        0xf258b0,
        0xf2a5d1,
        0xf2c06e,
        0xf2589a,
        0xf2a5c6,         // 120
        0xf2c04d,
        0xf25884,
        0xf2a5bb,
        0xf2c02c,
        0xf2586e,
        0xf2a5b0,
        0xf0f0f0
    ];

    var colorMap = {};
    for (var i = 0; i < 128; i++) {
        colorMap[palette[i]] = i;
    }
    this.colorMapper = new ColorMapper(colorMap);

    for (var name in this.buttons) {
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

MiniMixxx.Controller.prototype.activateLayer = function (layerName, channel) {
    for (var name in this.encoders) {
        encoder = this.encoders[name];
        encoder.activateLayer(layerName, channel);
    }
    for (var name in this.buttons) {
        button = this.buttons[name];
        button.activateLayer(layerName, channel);
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

MiniMixxx.encoderHandler = function (_midino, control, velo, _status, _group) {
    if (velo >= 64) {
        velo -= 128;
    }
    this.kontrol.encoders[control].activeMode.handleSpin(velo);
}

MiniMixxx.encoderButtonHandler = function (_midino, control, value, _status, _group) {
    this.kontrol.encoders[control].activeMode.handlePress(value);
}

MiniMixxx.pitchSliderHandler = function (_midino, _control, value, _status, group) {
    MiniMixxx.kontrol.pitchSliderHandler(value, group);
}

MiniMixxx.vuMeterColor = function (value) {
    value = Math.max(Math.min(value, 1.0), 0.0);
    // Color: 85 to 22, multiples of 3. That's 21 levels.
    return Math.round((1.0 - value) * 21.0) * 3 + 22;
}

MiniMixxx.debugLights = function () {
    midi.sendShortMsg(0xBF, 0x00, 20);
    midi.sendShortMsg(0xB0, 0x00, 9);
    midi.sendShortMsg(0xBF, 0x03, 20);
    midi.sendShortMsg(0xB0, 0x03, 117);
};

MiniMixxx.shutdown = function () {
    for (var i = 0; i < 0x13; i++) {
        if (i < 0x04) {
            midi.sendShortMsg(0xBF, i, 0x00);
        }
        midi.sendShortMsg(0x90, i, 0x00);
    }
};

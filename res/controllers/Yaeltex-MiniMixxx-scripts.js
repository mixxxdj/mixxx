///////////////////////////////////////////////////////////////////////////////////
//
// MiniMixxx controller script v1.00
// Author: Owen Williams (owilliams@mixxx.org)
//
///////////////////////////////////////////////////////////////////////////////////
//
// NOTE: This config uses a unique, custom javascript framework.  Please don't copy
// it into other controller configs. Instead, use the midi-components-X.X.js
// library.
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
MiniMixxx.MainOutColor = 50;      // Kind of a light green
MiniMixxx.HeadOutColor = 26;      // Kind of a light yellow

// Set to true to output debug messages and debug light outputs.
MiniMixxx.DebugMode = false;

// Set to true to only allow adjustments to the pitch sliders if Shift is held.
// This can prevent accidental adjustments.
MiniMixxx.ShiftPitch = false;

// An Encoder represents a single encoder knob and tracks the active mode.
MiniMixxx.Encoder = class {
    constructor(channel, idx, layerConfig) {
        this.channel = channel;
        this.idx = idx;
        this.encoders = {};
        this.layers = {};
        this.activeMode = {};

        for (const layerName in layerConfig) {
            const mode = layerConfig[layerName];
            if (mode.startsWith("EFFECT-")) {
                const tokens = mode.split("-");
                const effectNum = tokens[1];
                this.encoders[mode] = new MiniMixxx.EncoderModeFX(this, channel, idx, effectNum);
            } else {
                switch (mode) {
                case "JOG":
                    this.encoders[mode] = new MiniMixxx.EncoderModeJog(this, channel, idx);
                    break;
                case "GAIN":
                    this.encoders[mode] = new MiniMixxx.EncoderModeGain(this, channel, idx);
                    break;
                case "LOOP":
                    this.encoders[mode] = new MiniMixxx.EncoderModeLoop(this, channel, idx);
                    break;
                case "BEATJUMP":
                    this.encoders[mode] = new MiniMixxx.EncoderModeBeatJump(this, channel, idx);
                    break;
                case "LIBRARY":
                    this.encoders[mode] = new MiniMixxx.EncoderModeLibrary(this, channel, idx);
                    break;
                case "LIBRARYFOCUS":
                    this.encoders[mode] = new MiniMixxx.EncoderModeLibraryFocus(this, channel, idx);
                    break;
                case "LIBRARYHORIZ":
                    this.encoders[mode] = new MiniMixxx.EncoderModeLibraryHorizontal(this, channel, idx);
                    break;
                case "MAINGAIN":
                    this.encoders[mode] = new MiniMixxx.EncoderModeMainGain(this, "[Master]", idx);
                    break;
                case "BALANCE":
                    this.encoders[mode] = new MiniMixxx.EncoderModeBalance(this, "[Master]", idx);
                    break;
                case "HEADGAIN":
                    this.encoders[mode] = new MiniMixxx.EncoderModeHeadGain(this, "[Master]", idx);
                    break;
                case "HEADMIX":
                    this.encoders[mode] = new MiniMixxx.EncoderModeHeadMix(this, "[Master]", idx);
                    break;
                default:
                    console.log("Ignoring unknown encoder mode: " + mode);
                    continue;
                }
            }

            this.layers[layerName] = this.encoders[mode];
        }
        this.activateLayer("NONE", "");
    }

    activateLayer(layerName, channel) {
        // Only the active mode object should drive lights.
        let mode = this.layers[layerName];
        if (!mode || (channel && this.channel !== channel)) {
            mode = this.layers.NONE;
        }
        this.activeMode = mode;
        this.activeMode.setLights();
    }
};

// Mode is the parent class of all the Mode objects below (both for encoders and
// buttons).
MiniMixxx.Mode = class {
    constructor(parent, modeName, channel, idx) {
        this.parent = parent;
        this.modeName = modeName;
        this.channel = channel;
        this.idx = idx;
    }
};

// Jog Mode Encoder:
// Input:
// * Spin: adjust jog
// * Shift + spin: quick seek
// * Press: activate loop or toggle loop if active
// * Shift + Press: loop roll
// Output:
// * spinny angle
// * switch indicates loop active
MiniMixxx.EncoderModeJog = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "JOG", channel, idx);
        this.positionUpdate = false;
        this.curPosition = 0;
        this.trackDurationSec = 0;
        this.baseColor = MiniMixxx.JogColor;
        this.loopColor = MiniMixxx.LoopColor;
        this.color = this.baseColor;

        engine.makeConnection(this.channel, "track_loaded", this.trackLoadedHandler.bind(this));
        engine.makeConnection(this.channel, "playposition", this.playpositionChanged.bind(this));
        engine.makeConnection(this.channel, "loop_enabled", this.loopEnabledChanged.bind(this));
    }
    handleSpin(velo) {
        if (MiniMixxx.kontrol.shiftActive()) {
            let playPosition = engine.getValue(this.channel, "playposition");
            playPosition += velo / 256.0;
            playPosition = Math.max(Math.min(playPosition, 1.0), 0.0);
            engine.setValue(this.channel, "playposition", playPosition);
        } else {
            if (engine.getValue(this.channel, "play")) {
                velo /= 2;
            } else {
                velo *= 2;
            }
            engine.setValue(this.channel, "jog", velo);
        }
    }
    handlePress(value) {
        const isLoopActive = engine.getValue(this.channel, "loop_enabled");

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
    playpositionChanged(value) {
        if (this.trackDurationSec === 0) {
            const samples = engine.getValue(this.channel, "track_loaded");
            if (samples > 0) {
                this.trackLoadedHandler();
            } else {
                // No track loaded, abort
                return;
            }
        }
        this.curPosition = value * this.trackDurationSec;
        this.positionUpdated = true;
    }
    trackLoadedHandler() {
        const trackSamples = engine.getValue(this.channel, "track_samples");
        if (trackSamples === 0) {
            this.trackDurationSec = 0;
            return;
        }
        const trackSampleRate = engine.getValue(this.channel, "track_samplerate");
        // Assume stereo.
        this.trackDurationSec = trackSamples / 2.0 / trackSampleRate;
    }
    lightSpinny() {
        if (!this.positionUpdated || this !== this.parent.activeMode) {
            return;
        }
        this.positionUpdated = false;
        if (!engine.getValue(this.channel, "track_loaded")) {
            midi.sendShortMsg(0xB0, this.idx, 0x00);
            return;
        }
        const rotations = this.curPosition * (1 / 1.8);  // 1/1.8 is rotations per second (33 1/3 RPM)
        // Calculate angle from 0-1.0
        const angle = rotations - Math.floor(rotations);

        let midiColor = this.loopColor;
        if (engine.getValue(this.channel, "loop_enabled") === 0) {
            midiColor = MiniMixxx.vuMeterColor(engine.getValue(this.channel, "vu_meter"));
        }

        // Angles between 0 and .4 and .6 and 1.0 are in the indicator so no output.
        // The others have to go on the pfl light.
        if (angle >= 0.6) {
            const midival = (angle - 0.6) * (64.0 / 0.4);
            midi.sendShortMsg(0xBF, this.idx, midiColor);
            midi.sendShortMsg(0xB0, this.idx, midival);
        } else if (angle < 0.4) {
            const midival = angle * (64.0 / 0.4) + 64;
            midi.sendShortMsg(0xBF, this.idx, midiColor);
            midi.sendShortMsg(0xB0, this.idx, midival);
        } else {
            // rotary light off
            midi.sendShortMsg(0xB0, this.idx, 0x00);
        }
    }
    loopEnabledChanged() {
        this.setLights();
    }
    setLights() {
        this.positionUpdated = true;
        this.lightSpinny();
        const color = engine.getValue(this.channel, "loop_enabled") > 0 ? this.loopColor : 0x00;
        midi.sendShortMsg(0x90, this.idx, color);
    }
};

// Gain Mode Encoder:
// Input:
//  * Spin: adjust gain
//  * Press: toggle pfl.
//  * Shift + Press: reset gain adjust
// Output:
// * When idle, displays vu meters
// * when adjusting gain, shows gain.
// * switch shows pfl status.
MiniMixxx.EncoderModeGain = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "GAIN", channel, idx);

        this.color = MiniMixxx.GainColor;
        this.idleTimer = 0;
        this.showGain = false;
        engine.makeConnection(this.channel, "pregain", this.pregainIndicator.bind(this));
        engine.makeConnection(this.channel, "pfl", this.pflIndicator.bind(this));
        engine.makeUnbufferedConnection(this.channel, "vu_meter", this.vuIndicator.bind(this));
        engine.makeConnection(this.channel, "peak_indicator", this.peakIndicator.bind(this));
    }
    handleSpin(velo) {
        engine.setValue(this.channel, "pregain", engine.getValue(this.channel, "pregain") + velo / 50.0);
        this.pregainIndicator(engine.getValue(this.channel, "pregain"));
    }
    handlePress(value) {
        if (value === 0) {
            return;
        }
        if (MiniMixxx.kontrol.shiftActive()) {
            engine.setValue(this.channel, "pregain", 1.0);
        } else {
            script.toggleControl(this.channel, "pfl");
        }
    }
    vuIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode || this.showGain) {
            return;
        }

        let color = MiniMixxx.vuMeterColor(value);
        color = engine.getValue(this.channel, "peak_indicator") > 0 ? MiniMixxx.PeakColor : color;
        const midiValue = value * 127.0;
        midi.sendShortMsg(0xBF, this.idx, color);
        midi.sendShortMsg(0xB0, this.idx, midiValue);
    }
    peakIndicator(_value, _group, _control) {
        if (this !== this.parent.activeMode || this.showGain) {
            return;
        }
        this.vuIndicator(engine.getValue(this.channel, "vu_meter"));
    }
    pregainIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        if (this.idleTimer !== 0) {
            engine.stopTimer(this.idleTimer);
        }
        this.showGain = true;
        this.idleTimer = engine.beginTimer(1000, () => {
            this.showGain = false;
            this.vuIndicator(engine.getValue(this.channel, "vu_meter"));
        }, true);
        midi.sendShortMsg(0xBF, this.idx, this.color);
        midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
    }
    pflIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        const color = value > 0 ? this.color : 0;
        midi.sendShortMsg(0x90, this.idx, color);
    }
    setLights() {
        this.vuIndicator(engine.getValue(this.channel, "vu_meter"));
        this.pflIndicator(engine.getValue(this.channel, "pfl"));
    }
};

// Loop Encoder:
// Input:
// * Spin: adjust loop size
// * Shift + Spin: move loop
// * Press: activate if no loop, reloop toggle if there is
// * Shift + Press: always reloop toggle.
// Output:
// * Ring: display loop size
// * switch: loop status.
MiniMixxx.EncoderModeLoop = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "LOOP", channel, idx);

        this.color = MiniMixxx.LoopColor;
        engine.makeConnection(this.channel, "beatloop_size", this.spinIndicator.bind(this));
        engine.makeConnection(this.channel, "loop_enabled", this.switchIndicator.bind(this));
    }
    handleSpin(velo) {
        if (MiniMixxx.kontrol.shiftActive()) {
            const beatjumpSize = engine.getValue(this.channel, "beatjump_size");
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
    handlePress(value) {
        if (value === 0) {
            return;
        }
        const isLoopActive = engine.getValue(this.channel, "loop_enabled");

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
    spinIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        value = Math.log2(value);
        // log values go from -5 to 9 (14 steps)
        // And we should map from 9 to 127 (118 steps)
        value = Math.floor((value + 5) / 14.0 * 118 + 9);
        midi.sendShortMsg(0xB0, this.idx, value);
    }
    switchIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        const color = value > 0 ? this.color : 0;
        midi.sendShortMsg(0x90, this.idx, color);
    }
    setLights() {
        midi.sendShortMsg(0xBF, this.idx, this.color);
        this.spinIndicator(engine.getValue(this.channel, "beatloop_size"));
        this.switchIndicator(engine.getValue(this.channel, "loop_enabled"));
    }
};

// Beat Jump Encoder:
// Input:
//   * Spin: Adjust beatjump size
//   * Shift + Spin: Jump forward / backward
//   * Press: beatloop roll
//   * Shift + Press: reloop and stop
// Output:
//   * Ring: beatjump size
//   * Switch: on if mode is active.
MiniMixxx.EncoderModeBeatJump = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "BEATJUMP", channel, idx);

        this.color = MiniMixxx.LoopColor;
        engine.makeConnection(this.channel, "beatjump_size", this.spinIndicator.bind(this));
    }
    handleSpin(velo) {
        if (MiniMixxx.kontrol.shiftActive()) {
            if (velo > 0) {
                script.triggerControl(this.channel, "beatjump_forward");
            } else {
                script.triggerControl(this.channel, "beatjump_backward");
            }
        } else {
            let beatjumpSize = engine.getValue(this.channel, "beatjump_size");
            if (velo > 0) {
                beatjumpSize *= 2;
            } else {
                beatjumpSize /= 2;
            }
            engine.setValue(this.channel, "beatjump_size", Math.max(Math.min(beatjumpSize, 512), 1.0 / 32.0));
        }
    }
    handlePress(value) {
        if (MiniMixxx.kontrol.shiftActive()) {
            engine.setValue(this.channel, "reloop_andstop", value);
        } else {
            engine.setValue(this.channel, "beatlooproll_activate", value);
        }
    }
    spinIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        value = Math.log2(value);
        // log values go from -5 to 9 (14 steps)
        // And we should map from 9 to 127 (118 steps)
        value = Math.floor((value + 5) / 14.0 * 118 + 9);
        midi.sendShortMsg(0xB0, this.idx, value);
    }
    switchIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        const color = value > 0 ? this.color : 0;
        midi.sendShortMsg(0x90, this.idx, color);
    }
    setLights() {
        midi.sendShortMsg(0xBF, this.idx, this.color);
        this.spinIndicator(engine.getValue(this.channel, "beatjump_size"));
        midi.sendShortMsg(0x90, this.idx, this.color);
    }
};

// Library Encoder:
// Input:
//   * Spin: scroll up/down
//   * Shift + Spin: scroll horizontal
//   * Press: load track
//   * Shift + Press: eject track
// Output: All on.
MiniMixxx.EncoderModeLibrary = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "LIBRARY", channel, idx);
        this.color = MiniMixxx.LibraryColor;
    }
    handleSpin(velo) {
        if (MiniMixxx.kontrol.shiftActive()) {
            engine.setValue("[Library]", "MoveHorizontal", velo);
        } else {
            engine.setValue("[Library]", "MoveVertical", velo);
        }
    }
    handlePress(value) {
        if (MiniMixxx.kontrol.shiftActive()) {
            engine.setValue(this.channel, "eject", value);
        } else {
            engine.setValue(this.channel, "LoadSelectedTrack", value);
        }
    }
    setLights() {
        midi.sendShortMsg(0xBF, this.idx, this.color);
        midi.sendShortMsg(0xB0, this.idx, 0x7F);
        midi.sendShortMsg(0x90, this.idx, this.color);
    }
};

// Library Focus Encoder:
// Input:
//   * Spin: move focus forward / back
//   * Shift + Spin: scroll through track
//   * Press: "Go To Item" (opens trees)
//   * Shift + Press: search history select
// Output: Switch on.
MiniMixxx.EncoderModeLibraryFocus = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "LIBRARYFOCUS", channel, idx);
        this.color = MiniMixxx.LibraryFocusColor;
    }
    handleSpin(velo) {
        if (MiniMixxx.kontrol.shiftActive()) {
            let playPosition = engine.getValue(this.channel, "playposition");
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
    handlePress(value) {
        if (MiniMixxx.kontrol.shiftActive()) {
            engine.setValue("[Library]", "search_history_selector", value);
        } else {
            engine.setValue("[Library]", "GoToItem", value);
        }
    }
    setLights() {
        midi.sendShortMsg(0xBF, this.idx, this.color);
        midi.sendShortMsg(0xB0, this.idx, 0x00);
        midi.sendShortMsg(0x90, this.idx, this.color);
    }
};

// Library Horizontal Focus:
// Input:
//   * Spin: Scroll horizontal
//   * Shift + Spin: scroll through track
//   * Press: "Go To Item" (opens trees)
//   * Shift + Press: search history select
// Output: Switch on.
MiniMixxx.EncoderModeLibraryHorizontal = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "LIBRARYHORIZ", channel, idx);
        this.color = MiniMixxx.LibraryFocusColor;
    }
    handleSpin(velo) {
        if (MiniMixxx.kontrol.shiftActive()) {
            let playPosition = engine.getValue(this.channel, "playposition");
            playPosition += velo / 256.0;
            playPosition = Math.max(Math.min(playPosition, 1.0), 0.0);
            engine.setValue(this.channel, "playposition", playPosition);
        } else {
            engine.setValue("[Library]", "MoveHorizontal", velo);
        }
    }
    handlePress(value) {
        if (MiniMixxx.kontrol.shiftActive()) {
            engine.setValue("[Library]", "search_history_selector", value);
        } else {
            engine.setValue("[Library]", "GoToItem", value);
        }
    }
    setLights() {
        midi.sendShortMsg(0xBF, this.idx, this.color);
        midi.sendShortMsg(0xB0, this.idx, 0x00);
        midi.sendShortMsg(0x90, this.idx, this.color);
    }
};

// FX Encoder:
// Input:
//   * Spin: Adjust meta knob
//   * Shift + Spin: n/a
//   * Press: activate
//   * Shift + Press: n/a
// Output:
// * fx unit meta
// * switch on if fx is active.
MiniMixxx.EncoderModeFX = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx, effectNum) {
        super(parent, "FX", channel, idx);

        this.color = MiniMixxx.FXColor;
        if (effectNum === "SUPER") {
            this.effectGroup = "[EffectRack1_EffectUnit1]";
            this.effectKey = "super1";
        } else {
            this.effectGroup = "[EffectRack1_EffectUnit1_Effect" + effectNum + "]";
            this.effectKey = "meta";
        }

        engine.makeConnection(this.effectGroup, this.effectKey, this.spinIndicator.bind(this));
        engine.makeConnection(this.effectGroup, "enabled", this.switchIndicator.bind(this));
    }
    handleSpin(velo) {
        engine.setValue(this.effectGroup, this.effectKey, engine.getValue(this.effectGroup, this.effectKey) + velo / 50.0);
    }
    handlePress(value) {
        if (value > 0) {
            script.toggleControl(this.effectGroup, "enabled");
        }
    }
    spinIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        midi.sendShortMsg(0xB0, this.idx, Math.floor(value * 127.0));
    }
    switchIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        const color = value > 0 ? this.color : 0;
        midi.sendShortMsg(0x90, this.idx, color);
    }
    setLights() {
        midi.sendShortMsg(0xBF, this.idx, this.color);
        this.spinIndicator(engine.getValue(this.effectGroup, this.effectKey));
        this.switchIndicator(engine.getValue(this.effectGroup, "enabled"));
    }
};

// Primary Gain:
// Input:
//   * Spin: Adjust primary output up/down
//   * Shift + Spin: Adjust booth
//   * Press: reset primary output
//   * Shift + Press: reset booth
// Output: gain
MiniMixxx.EncoderModeMainGain = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "MAINGAIN", channel, idx);
        this.color = MiniMixxx.MainOutColor;

        engine.makeConnection("[Master]", "gain", this.gainIndicator.bind(this));
        engine.makeConnection("[Master]", "booth_gain", this.gainIndicator.bind(this));
    }
    handleSpin(velo) {
        if (MiniMixxx.kontrol.shiftActive()) {
            engine.setValue("[Master]", "booth_gain", engine.getValue("[Master]", "booth_gain") + .02 * velo);
        } else {
            engine.setValue("[Master]", "gain", engine.getValue("[Master]", "gain") + .02 * velo);
        }
    }
    handlePress(value) {
        if (value === 0) {
            return;
        }
        if (MiniMixxx.kontrol.shiftActive()) {
            engine.setValue("[Master]", "booth_gain", 1.0);
        } else {
            engine.setValue("[Master]", "gain", 1.0);
        }
    }
    gainIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }

        midi.sendShortMsg(0xBF, this.idx, this.color);
        midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
    }
    setLights() {
        midi.sendShortMsg(0x90, this.idx, this.color);
        this.gainIndicator(engine.getValue("[Master]", "gain"));
    }
};

// Headphone Gain:
// Input:
//   * Spin: Adjust headphone output up/down
//   * Press: reset headphone output
// Output: gain
MiniMixxx.EncoderModeHeadGain = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "HEADGAIN", channel, idx);
        this.color = MiniMixxx.HeadOutColor;

        engine.makeConnection("[Master]", "headGain", this.gainIndicator.bind(this));
    }
    handleSpin(velo) {
        engine.setValue("[Master]", "headGain", engine.getValue("[Master]", "headGain") + .02 * velo);
    }
    handlePress(value) {
        if (value === 0) {
            return;
        }
        engine.setValue("[Master]", "headGain", 1.0);
    }
    gainIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }

        midi.sendShortMsg(0xBF, this.idx, this.color);
        midi.sendShortMsg(0xB0, this.idx, script.absoluteNonLinInverse(value, 0, 1.0, 4.0));
    }
    setLights() {
        midi.sendShortMsg(0x90, this.idx, this.color);
        this.gainIndicator(engine.getValue("[Master]", "headGain"));
    }
};

// Balance:
// Input:
//   * Spin: Adjust balance
//   * Press: reset balance
// Output: value
MiniMixxx.EncoderModeBalance = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "BALANCE", channel, idx);
        this.color = MiniMixxx.MainOutColor;

        engine.makeConnection("[Master]", "balance", this.balIndicator.bind(this));
    }
    handleSpin(velo) {
        engine.setValue("[Master]", "balance", engine.getValue("[Master]", "balance") + .01 * velo);
    }
    handlePress(value) {
        if (value === 0) {
            return;
        }
        engine.setValue("[Master]", "balance", 0.0);
    }
    balIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }

        midi.sendShortMsg(0xBF, this.idx, this.color);
        midi.sendShortMsg(0xB0, this.idx, (value + 1.0) * 64.0 - 1.0);
    }
    setLights() {
        this.balIndicator(engine.getValue("[Master]", "balance"));
        midi.sendShortMsg(0x90, this.idx, this.color);
    }
};

// Headphone Mix:
// Input:
//   * Spin: Adjust headphone mix
//   * Press: reset headphone mix
// Output: value
MiniMixxx.EncoderModeHeadMix = class extends MiniMixxx.Mode {
    constructor(parent, channel, idx) {
        super(parent, "HEADMIX", channel, idx);
        this.color = MiniMixxx.HeadOutColor;

        engine.makeConnection("[Master]", "headMix", this.mixIndicator.bind(this));
    }
    handleSpin(velo) {
        engine.setValue("[Master]", "headMix", engine.getValue("[Master]", "headMix") + .01 * velo);
    }
    handlePress(value) {
        if (value === 0) {
            return;
        }
        engine.setValue("[Master]", "headMix", 0.0);
    }
    mixIndicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }

        midi.sendShortMsg(0xBF, this.idx, this.color);
        midi.sendShortMsg(0xB0, this.idx, (value + 1.0) * 64.0 - 1.0);
    }
    setLights() {
        this.mixIndicator(engine.getValue("[Master]", "headMix"));
        midi.sendShortMsg(0x90, this.idx, this.color);
    }
};

// Button represents a single physical button and contains all of the mode objects that
// drive its behavior.
MiniMixxx.Button = class {
    constructor(channel, idx, layerConfig) {
        this.channel = channel;
        this.idx = idx;
        this.buttons = {};
        this.layers = {};
        this.activeMode = {};

        for (const layerName in layerConfig) {
            const mode = layerConfig[layerName];

            if (mode.startsWith("SAMPLER-")) {
                const tokens = mode.split("-");
                const samplerNum = tokens[1];
                this.buttons[mode] = new MiniMixxx.ButtonModeSampler(this, "", idx, samplerNum);
            } else if (mode.startsWith("HOTCUE-")) {
                const tokens = mode.split("-");
                const hotcueNum = tokens[1];
                this.buttons[mode] = new MiniMixxx.ButtonModeHotcue(this, channel, idx, hotcueNum);
            } else if (mode.startsWith("FX-")) {
                // We only use FX 1 for now.
                this.buttons[mode] = new MiniMixxx.ButtonModeFX(this, channel, idx);
            } else {
                let shiftedButton = {};
                switch (mode) {
                case "EMPTY":
                    this.buttons[mode] = new MiniMixxx.ButtonModeEmpty(this, channel, idx);
                    break;
                case "SYNC":
                    this.buttons[mode] = new MiniMixxx.ButtonModeSync(this, channel, idx);
                    break;
                case "KEYLOCK":
                    this.buttons[mode] = new MiniMixxx.ButtonModeKeylock(this, channel, idx);
                    break;
                case "SHIFT":
                    this.buttons[mode] = new MiniMixxx.ButtonModeShift(this, "", idx);
                    break;
                case "LOOPLAYER":
                    this.buttons[mode] = new MiniMixxx.ButtonModeLayer(this, "LOOPLAYER", channel, idx, [0, MiniMixxx.LoopColor]);
                    break;
                case "SAMPLERLAYER-HOTCUE2LAYER":
                    this.buttons[mode] = new MiniMixxx.ButtonModeLayer(this, "HOTCUELAYER", channel, idx, [0, MiniMixxx.HotcueColor]);
                    this.buttons[mode].addShiftedButton(this, "SAMPLERLAYER", "", idx, [0, MiniMixxx.SamplerColor]);
                    shiftedButton = this.buttons[mode].shiftedButton;
                    this.layers[shiftedButton.layerName] = shiftedButton;
                    break;
                case "LIBRARYLAYER-MAINGAINLAYER":
                    this.buttons[mode] = new MiniMixxx.ButtonModeLayer(this, "LIBRARYLAYER", "", idx, [0, MiniMixxx.LibraryColor]);
                    this.buttons[mode].addShiftedButton(this, "MAINGAINLAYER", "", idx, [0, MiniMixxx.MainOutColor]);
                    shiftedButton = this.buttons[mode].shiftedButton;
                    this.layers[shiftedButton.layerName] = shiftedButton;
                    break;
                case "FXLAYER-HOTCUE1LAYER":
                    this.buttons[mode] = new MiniMixxx.ButtonModeLayer(this, "HOTCUELAYER", channel, idx, [0, MiniMixxx.HotcueColor]);
                    this.buttons[mode].addShiftedButton(this, "FXLAYER", "", idx, [0, MiniMixxx.FXColor]);
                    shiftedButton = this.buttons[mode].shiftedButton;
                    this.layers[shiftedButton.layerName] = shiftedButton;
                    break;
                default:
                    console.log("Ignoring unknown button mode: " + mode);
                    continue;
                }
            }
            this.layers[layerName] = this.buttons[mode];
        }
        this.activateLayer("NONE", "");
    }
    activateLayer(layerName, channel) {
        // We need to go through and update all the layer buttons that we might own.
        for (const name in this.buttons) {
            const button = this.buttons[name];
            if (button instanceof MiniMixxx.ButtonModeLayer) {
                button.setActive(layerName, channel);
            }
            if (button.shiftedButton instanceof MiniMixxx.ButtonModeLayer) {
                button.shiftedButton.setActive(layerName, channel);
            }
        }

        const mode = this.layers[layerName];
        if (mode) {
            // Only activate the mode if it's universal (no channel set) or the channel matches.
            if (channel === "" || mode.channel === channel) {
                this.activeMode = mode;
            }
        } else if (channel === "") {
            // If we didn't have the desired mode, and it's universal, forcibly change to the NONE
            // layer.
            this.activeMode = this.layers.NONE;
        }
        this.activeMode.setLights();
    }
};

MiniMixxx.buttonHandler = function(_midino, control, value, _status, _group) {
    const button = this.kontrol.buttons[control];
    if (!button) {
        console.log("unhandled button: " + control);
        return;
    }

    button.activeMode.handlePress(value);
};


// ButtonMode defines some extra state useful for buttons.
MiniMixxx.ButtonMode = class extends MiniMixxx.Mode {
    constructor(parent, modeName, channel, idx, colors) {
        super(parent, modeName, channel, idx);
        this.colors = colors;
    }
    lightButton(idx, value, colors) {
        const color = value > 0 ? colors[1] : colors[0];
        midi.sendShortMsg(0x90, idx, color);
    }
};

// ButtonModeEmpty is a placeholder that does nothing.
MiniMixxx.ButtonModeEmpty = class extends MiniMixxx.ButtonMode {
    constructor(parent, channel, idx) {
        super(parent, "EMPTY", channel, idx, [0, 0]);
    }
    handlePress(_value) {
    }
    indicator(value, _group, _control) {
        super.lightButton(this.idx, value, this.colors);
    }
    setLights() {
        this.indicator(0);
    }
};

// ButtonModeKeylock enables and disables keylock.
// If held, this button causes the pitch slider to act as a musical key adjustment.
MiniMixxx.ButtonModeKeylock = class extends MiniMixxx.ButtonMode {
    constructor(parent, channel, idx) {
        super(parent, "KEYLOCK", channel, idx, [0, 108]);
        this.keylockPressed = false;
        engine.makeConnection(this.channel, "keylock", this.indicator.bind(this));
    }
    handlePress(value) {
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
            const val = engine.getValue(this.channel, "keylock");
            this.indicator(val);
        } else {
            this.indicator(1);
        }
    }
    indicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        super.lightButton(this.idx, value, this.colors);
    }
    setLights() {
        this.indicator(engine.getValue(this.channel, "keylock"));
    }
};

// ButtonModeSync
// Input: press to do a one-off beatsync.  Press and hold to enable Sync Lock.
MiniMixxx.ButtonModeSync = class extends MiniMixxx.ButtonMode {
    constructor(parent, channel, idx) {
        super(parent, "SYNC", channel, idx, [0, 84]);
        this.syncPressedTimer = 0;

        engine.makeConnection(this.channel, "sync_enabled", this.indicator.bind(this));
    }
    handlePress(value) {
        if (value) {
            // We have to reimplement push-to-lock because it's only defined in the midi code
            // in Mixxx.
            if (engine.getValue(this.channel, "sync_enabled") === 0) {
                engine.setValue(this.channel, "sync_enabled", 1);
                // Start timer to measure how long button is pressed
                this.syncPressedTimer = engine.beginTimer(300, () => {
                    engine.setValue(this.channel, "sync_enabled", 1);
                    // Reset sync button timer state if active
                    if (this.syncPressedTimer !== 0) {
                        this.syncPressedTimer = 0;
                    }
                }, true);
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
    indicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        super.lightButton(this.idx, value, this.colors);
    }
    setLights() {
        this.indicator(engine.getValue(this.channel, "sync_enabled"));
    }
};

// ButtonModeShift: the shift button
MiniMixxx.ButtonModeShift = class extends MiniMixxx.ButtonMode {
    constructor(parent, channel, idx) {
        super(parent, "SHIFT", channel, idx, [0, 127]);
        this.shiftActive = false;
    }
    handlePress(value) {
        this.shiftActive = value > 0;
        this.indicator(this.shiftActive);
    }
    indicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        super.lightButton(this.idx, value, this.colors);
    }
    setLights() {
        this.indicator(this.shiftActive);
    }
};

// ButtonModeLayer activates the layer with the given name.
// channel can be empty if it should apply to both channels.
// These buttons can also have a shift function to select a different layer.
MiniMixxx.ButtonModeLayer = class extends MiniMixxx.ButtonMode {
    constructor(parent, layerName, channel, idx, colors) {
        super(parent, layerName, channel, idx, colors);
        // A ButtonModeLayer can contain a child of itself that is activated when Shift is held.
        this.shiftedButton = null;
        this.requireShift = false;
        this.layerActive = false;
    }
    // Add a second layer option to the shift-press for this button.
    addShiftedButton(parent, layerName, channel, idx, colors) {
        // A shifted button could hypothetically have its own shifted button but let's not go there.
        this.shiftedButton = new MiniMixxx.ButtonModeLayer(parent, layerName, channel, idx, colors);
        this.shiftedButton.requireShift = true;
    }
    handlePress(value) {
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
    setActive(layerName, channel) {
        // if it's got a channel, and so do we, no need to change if channel mismatch
        if (channel && this.channel && this.channel !== channel) {
            return;
        }

        // Set based on layer name match.
        this.layerActive = (this.modeName === layerName);
        this.setLights();
    }
    indicator(_value, _group, _control) {
        super.lightButton(this.idx, this.layerActive, this.colors);
    }
    setLights() {
        if (this.shiftedButton && this.shiftedButton.layerActive) {
            this.shiftedButton.indicator();
            return;
        }
        this.indicator();
    }
};

// ButtonModeSampler is the button mode for playing individual sampler decks.
// Shift+Press to eject the sample.
MiniMixxx.ButtonModeSampler = class extends MiniMixxx.ButtonMode {
    constructor(parent, channel, idx, samplerNum) {
        super(parent, "SAMPLER-" + samplerNum, channel, idx, [MiniMixxx.UnsetSamplerColor, MiniMixxx.SamplerColor]);
        this.samplerGroup = "[Sampler" + samplerNum + "]";
        engine.makeConnection(this.samplerGroup, "track_loaded", this.indicator.bind(this));
    }
    handlePress(value) {
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
    indicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        super.lightButton(this.idx, value, this.colors);
    }
    setLights() {
        this.indicator(engine.getValue(this.samplerGroup, "track_loaded"));
    }
};

// ButtonModeHotcue is the button mode for playing individual sampler decks.
// Shift+Press to eject the sample.
MiniMixxx.ButtonModeHotcue = class extends MiniMixxx.ButtonMode {
    constructor(parent, channel, idx, hotcueNum) {
        super(parent, "HOTCUE-" + hotcueNum, channel, idx, [0x7f, MiniMixxx.HotcueColor]);
        this.hotcueNum = hotcueNum;
        this.keyPrefix = "hotcue_" + hotcueNum + "_";
        engine.makeConnection(this.channel, this.keyPrefix + "enabled", this.indicator.bind(this));
        engine.makeConnection(this.channel, this.keyPrefix + "color", this.indicator.bind(this));
    }
    handlePress(value) {
        // shift + press unsets the hotcue
        if (MiniMixxx.kontrol.shiftActive()) {
            if (value) {
                script.toggleControl(this.channel, this.keyPrefix + "clear");
            }
        } else {
            engine.setValue(this.channel, this.keyPrefix + "activate", value);
        }
    }
    indicator(_value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        if (!engine.getValue(this.channel, this.keyPrefix + "enabled")) {
            midi.sendShortMsg(0x90, this.idx, 0x7f);
            return;
        }
        const colorCode = engine.getValue(this.channel, this.keyPrefix + "color");
        const midiColor = MiniMixxx.kontrol.colorMapper.getValueForNearestColor(colorCode);
        midi.sendShortMsg(0x90, this.idx, midiColor);
    }
    setLights() {
        this.indicator(engine.getValue(this.channel, this.keyPrefix + "enabled"));
    }
};

// ButtonModeFX enables the FX unit for this deck.
MiniMixxx.ButtonModeFX = class extends MiniMixxx.ButtonMode {
    constructor(parent, channel, idx) {
        super(parent, "FX", channel, idx, [0, MiniMixxx.FXColor]);
        engine.makeConnection("[EffectRack1_EffectUnit1]", "group_" + this.channel + "_enable", this.indicator.bind(this));
    }
    handlePress(value) {
        if (value > 0) {
            script.toggleControl("[EffectRack1_EffectUnit1]", "group_" + this.channel + "_enable");
        }
    }
    indicator(value, _group, _control) {
        if (this !== this.parent.activeMode) {
            return;
        }
        super.lightButton(this.idx, value, this.colors);
    }
    setLights() {
        this.indicator(engine.getValue("[EffectRack1_EffectUnit1]", "group_" + this.channel + "_enable"));
    }
};

MiniMixxx.Controller = class {
    constructor() {
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
                "LIBRARYLAYER": "LIBRARYHORIZ",
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
        };

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
        const palette = [
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

        const colorMap = {};
        for (let i = 0; i < 128; i++) {
            colorMap[palette[i]] = i;
        }
        this.colorMapper = new ColorMapper(colorMap);

        for (const name in this.buttons) {
            this.buttons[name].activeMode.setLights();
        }

        this.guiTickConnection = engine.makeConnection("[App]", "gui_tick_50ms_period_s", this.guiTickHandler.bind(this));
    }


    shiftActive(_channel) {
        return this.shiftButton.buttons.SHIFT.shiftActive;
    }

    keylockPressed(channel) {
        return this.keylockButtons[channel].buttons.KEYLOCK.keylockPressed;
    }

    activateLayer(layerName, channel) {
        for (const name in this.encoders) {
            const encoder = this.encoders[name];
            encoder.activateLayer(layerName, channel);
        }
        for (const name in this.buttons) {
            const button = this.buttons[name];
            button.activateLayer(layerName, channel);
        }
    }

    pitchSliderHandler(value, group) {
        // Adapt HID value to rate control range.
        value = -1.0 + ((value / 127.0) * 2.0);
        if (this.pitchSliderLastValue[group] === -1) {
            this.pitchSliderLastValue[group] = value;
            return;
        }

        // If shift is pressed, don't update any values.
        if (!this.shiftActive() && !this.keylockPressed(group)) {
            this.pitchSliderLastValue[group] = value;
            return;
        }

        let relVal = 0.0;
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

    guiTickHandler() {
        this.jogEncoders["[Channel1]"].encoders.JOG.lightSpinny();
        this.jogEncoders["[Channel2]"].encoders.JOG.lightSpinny();
    }
};

MiniMixxx.init = function(_id) {
    if (engine.getValue("[App]", "num_samplers") < 8) {
        engine.setValue("[App]", "num_samplers", 8);
    }

    this.kontrol = new MiniMixxx.Controller();

    console.log("MiniMixxx: Init done!");

    if (MiniMixxx.DebugMode) {
        MiniMixxx.debugLights();
    }
};

MiniMixxx.encoderHandler = function(_midino, control, velo, _status, _group) {
    if (velo >= 64) {
        velo -= 128;
    }
    this.kontrol.encoders[control].activeMode.handleSpin(velo);
};

MiniMixxx.encoderButtonHandler = function(_midino, control, value, _status, _group) {
    this.kontrol.encoders[control].activeMode.handlePress(value);
};

MiniMixxx.pitchSliderHandler = function(_midino, _control, value, _status, group) {
    MiniMixxx.kontrol.pitchSliderHandler(value, group);
};

MiniMixxx.vuMeterColor = function(value) {
    value = Math.max(Math.min(value, 1.0), 0.0);
    // Color: 85 to 16, multiples of 3. That's 23 levels.
    return Math.round((1.0 - value) * 23.0) * 3 + 16;
};

MiniMixxx.debugLights = function() {
    midi.sendShortMsg(0xBF, 0x00, 20);
    midi.sendShortMsg(0xB0, 0x00, 9);
    midi.sendShortMsg(0xBF, 0x03, 20);
    midi.sendShortMsg(0xB0, 0x03, 117);
};

MiniMixxx.shutdown = function() {
    for (let i = 0; i < 0x13; i++) {
        if (i < 0x04) {
            midi.sendShortMsg(0xBF, i, 0x00);
        }
        midi.sendShortMsg(0x90, i, 0x00);
    }
};

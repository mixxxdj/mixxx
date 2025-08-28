/****************************************************************/
/*      Traktor Kontrol S2 MK1 HID controller script            */
/*      Copyright (C) 2023, leifhelm                            */
/*      Initially Based on:                                     */
/*      Traktor Kontrol S2 MK2 HID controller script v1.00      */
/*      Copyright (C) 2020, Be <be@mixxx.org>                   */
/*      Copyright (C) 2017, z411 <z411@omaera.org>              */
/*      but feel free to tweak this to your heart's content!    */
/****************************************************************/

// ==== Friendly User Configuration ====
// The Cue button, when Shift is also held, can have two possible functions:
// 1. "REWIND": seeks to the very start of the track.
// 2. "REVERSEROLL": performs a temporary reverse or "censor" effect, where the track
//    is momentarily played in reverse until the button is released.
const ShiftCueButtonAction = "REWIND";

// Set the brightness of button LEDs which are off and on. This uses a scale from 0 to 0x1F (31).
// If you don't have the optional power adapter and are using the controller with USB bus power,
const ButtonBrightnessOff = 0x00;
const ButtonBrightnessOn = 0x1F;

const padModes = {
    "hotcue": 0,
    "introOutro": 1,
    "sampler": 2
};


class DeckClass {
    constructor(parent, number) {
        this.parent = parent;
        this.controller = this.parent.controller;
        this.number = number;
        this.channel = "[Channel" + number + "]";
        this.gainEncoder = new Encoder();
        this.leftEncoder = new Encoder();
        this.rightEncoder = new Encoder();
        this.wheelPressInertiaTimer = 0;
        this.gainEncoderPressed = false;
        this.leftEncoderPressed = false;
        this.rightEncoderPressed = false;
        this.shiftPressed = false;
        this.currentPadMode = padModes.hotcue;
        this.pads = [
            new PadButton(this, 1),
            new PadButton(this, 2),
            new PadButton(this, 3),
            new PadButton(this, 4),
        ];
        this.eq = new Equalizer(this);
        this.lastTickTime = 0;
        this.lastTickValue = 0;
        this.syncEnabledTime = NaN;
        this.calibration = null;
    }
    registerInputs(config) {
        // InputReport 0x01
        this.registerButton("!gain_encoder_press", config.gainEncoderPress, this.gainEncoderPress);
        this.registerButton("!shift", config.shift, this.shift);
        this.registerButton("!sync_enabled", config.sync, this.syncButton);
        this.registerButton("!cue_default", config.cue, this.cueButton);
        this.registerButton("!play", config.play, this.playButton);
        for (let i = 0; i < 4; i++) {
            this.pads[i].registerInputs(config.pads[i]);
        }
        this.registerButton("!loop_in", config.loopIn, this.loopInButton);
        this.registerButton("!loop_out", config.loopOut, this.loopOutButton);
        this.registerButton("!samples_button", config.samples, this.samplerModeButton);
        this.registerButton("!reset_button", config.reset, this.introOutroModeButton);
        this.registerButton("!left_encoder_press", config.leftEncoderPress, this.leftEncoderPress);
        this.registerButton("!right_encoder_press", config.rightEncoderPress, this.rightEncoderPress);
        this.registerButton("!load_track", config.loadTrack, this.loadTrackButton);
        this.registerButton("!pfl", config.pfl, this.pflButton);
        config.jogWheel.hidReport.addControl(this.channel, "!jog_wheel", config.jogWheel.offset, "I", 0xFFFFFFFF, false, this.jogMove.bind(this));
        // InputReport 0x02
        this.registerScalar("rate", config.rate);
        this.registerEncoder("!left_encoder", config.leftEncoder, this.leftEncoderCallback);
        this.registerEncoder("!right_encoder", config.rightEncoder, this.rightEncoderCallback);
        this.registerScalar("!volume", config.volume, this.volume);
        this.registerEncoder("!pregain", config.gain, this.gainEncoderCallback);
        this.registerScalar("!jog_press", config.jogPress, this.jogPress);
        this.eq.registerInputs(config.eq);
    }
    registerOutputs(config) {
        this.registerLed("track_loaded", config.trackLoaded);
        for (let i = 0; i < 4; i++) {
            this.registerLed("!VuMeter" + i, {hidReport: config.vuMeter.hidReport, offset: config.vuMeter.offset + i});
        }
        this.registerLed("peak_indicator", config.peak);
        this.registerLed("!reset_button", config.reset);
        this.registerLed("loop_in", config.loopIn);
        this.registerLed("loop_out",  config.loopOut);
        this.registerLed("pfl", config.pfl);
        this.registerLed("!samples_button", config.samples);
        this.registerLed("!shift", config.shift);
        this.registerLed("sync_enabled", config.sync);
        this.registerLed("cue_indicator", config.cue);
        this.registerLed("play_indicator", config.play);
        for (let i = 0; i < 4; i++) {
            this.pads[i].registerOutputs(config.pads[i]);
        }
    }
    linkOutputs() {
        this.linkLed("sync_enabled", this.outputCallback);
        this.linkLed("cue_indicator", this.outputCallback);
        this.linkLed("play_indicator", this.outputCallback);
        this.linkLed("loop_in", this.outputCallbackLoop);
        this.linkLed("loop_out", this.outputCallbackLoop);
        this.linkLed("pfl", this.outputCallback);
        this.linkLed("track_loaded", this.outputCallback);
        this.linkLed("peak_indicator", this.outputCallback);
        engine.makeConnection(this.channel, "vu_meter", this.onVuMeterChanged.bind(this)).trigger();
        engine.makeConnection(this.channel, "loop_enabled", this.onLoopEnabledChanged.bind(this));
    }
    calibrate(calibration) {
        this.calibration = calibration;
        this.eq.calibrate(calibration.eq);
    }
    enableSoftTakeover() {
        engine.softTakeover(this.channel, "rate", true);
        engine.softTakeover(this.channel, "volume", true);
        this.eq.enableSoftTakeover();
    }
    registerButton(name, config, callback) {
        if (callback !==undefined) {
            callback = callback.bind(this);
        }
        config.hidReport.addControl(this.channel, name, config.offset, "B", config.mask, false, callback);
    }
    registerScalar(name, config, callback) {
        if (callback !==undefined) {
            callback= callback.bind(this);
        }
        config.hidReport.addControl(this.channel, name, config.offset, "H", 0xFFFF, false, callback);
    }
    registerEncoder(name, config, callback) {
        if (callback !==undefined) {
            callback= callback.bind(this);
        }
        config.hidReport.addControl(this.channel, name, config.offset, "B", config.mask, false, callback);
    }
    registerLed(name, config) {
        config.hidReport.addOutput(this.channel, name, config.offset, "B");
    }
    linkLed(name, callback) {
        engine.makeConnection(this.channel, name, callback.bind(this));
    }
    volume(field) {
        setFaderParameter(this.channel, "volume", field.value, this.calibration.volume);
    }
    gainEncoderPress(field) {
        if (field.value > 0) {
            this.gainEncoderPressed = true;
            if (this.shiftPressed) {
                script.triggerControl(this.channel, "pregain_set_default");
            } else {
                script.triggerControl("[QuickEffectRack1_" + this.channel + "]", "super1_set_default");
            }
        } else {
            this.gainEncoderPressed = false;
        }
    }
    shift(field) {
        const shiftPressed = field.value > 0;
        this.shiftPressed = shiftPressed;
        this.controller.setOutput(this.channel, "!shift",
            shiftPressed ? ButtonBrightnessOn : ButtonBrightnessOff,
            !this.parent.batchingLEDUpdate);
    }
    syncButton(field) {
        const now = Date.now();

        // If shifted, just toggle.
        // TODO(later version): actually make this enable explicit master.
        if (this.shiftPressed) {
            if (field.value === 0) {
                return;
            }
            const synced = engine.getValue(this.channel, "sync_enabled");
            engine.setValue(this.channel, "sync_enabled", !synced);
        } else {
            if (field.value === 1) {
                this.syncEnabledTime = now;
                engine.setValue(this.channel, "sync_enabled", 1);
            } else {
                if (!engine.getValue(this.channel, "sync_enabled")) {
                // If disabled, and switching to disable... stay disabled.
                    engine.setValue(this.channel, "sync_enabled", 0);
                    return;
                }
                // was enabled, and button has been let go.  maybe latch it.
                if (now - this.syncEnabledTime > 300) {
                    engine.setValue(this.channel, "sync_enabled", 1);
                    return;
                }
                engine.setValue(this.channel, "sync_enabled", 0);
            }
        }
    }
    cueButton(field) {
        if (this.shiftPressed) {
            if (ShiftCueButtonAction === "REWIND") {
                if (field.value === 0) {
                    return;
                }
                engine.setValue(this.channel, "start_stop", 1);
            } else if (ShiftCueButtonAction === "REVERSEROLL") {
                engine.setValue(this.channel, "reverseroll", field.value);
            }
        } else {
            engine.setValue(this.channel, "cue_default", field.value);
        }
    }
    playButton(field) {
        if (field.value === 0) {
            return;
        }
        if (this.shiftPressed) {
            const locked = engine.getValue(this.channel, "keylock");
            engine.setValue(this.channel, "keylock", !locked);
        } else {
            const playing = engine.getValue(this.channel, "play");
            // Failsafe to disable scratching in case the finishJogPress timer has not executed yet
            // after a backspin.
            if (engine.isScratching(this.number)) {
                engine.scratchDisable(this.number, false);
            }
            engine.setValue(this.channel, "play", !playing);
        }
    }
    loopInButton(field) {
        engine.setValue(this.channel, "loop_in", field.value);
    }

    loopOutButton(field) {
        engine.setValue(this.channel, "loop_out", field.value);
    }

    samplerModeButton(field) {
        if (field.value === 0) {
            return;
        }
        const padMode = this.currentPadMode;
        if (padMode !== padModes.sampler) {
            this.setPadMode(padModes.sampler);
            this.controller.setOutput(this.channel, "!samples_button", ButtonBrightnessOn, false);
            this.controller.setOutput(this.channel, "!reset_button", ButtonBrightnessOff, !this.parent.batchingLEDUpdate);
        } else {
            this.setPadMode(padModes.hotcue);
            this.controller.setOutput(this.channel, "!samples_button", ButtonBrightnessOff, !this.parent.batchingLEDUpdate);
        }
    }
    introOutroModeButton(field) {
        if (field.value === 0) {
            return;
        }
        const padMode = this.currentPadMode;
        if (padMode !== padModes.introOutro) {
            this.setPadMode(padModes.introOutro);
            this.controller.setOutput(this.channel, "!reset_button", ButtonBrightnessOn, false);
            this.controller.setOutput(this.channel, "!samples_button", ButtonBrightnessOff, !this.parent.batchingLEDUpdate);
        } else {
            this.setPadMode(padModes.hotcue);
            this.controller.setOutput(this.channel, "!reset_button", ButtonBrightnessOff, !this.parent.batchingLEDUpdate);
        }
    }
    leftEncoderPress(field) {
        this.leftEncoderPressed = field.value > 0;
        if (this.shiftPressed && this.leftEncoderPressed) {
            script.triggerControl(this.channel, "pitch_adjust_set_default");
        }
    }
    rightEncoderPress(field) {
        if (field.value === 0) {
            return;
        }
        const loopEnabled = engine.getValue(this.channel, "loop_enabled");
        // The actions triggered below change the state of loop_enabled,
        // so to simplify the logic, use script.triggerControl to only act
        // on press rather than resetting ControlObjects to 0 on release.
        if (this.shiftPressed) {
            if (loopEnabled) {
                script.triggerControl(this.channel, "reloop_andstop");
            } else {
                script.triggerControl(this.channel, "reloop_toggle");
            }
        } else {
            if (loopEnabled) {
                script.triggerControl(this.channel, "reloop_toggle");
            } else {
                script.triggerControl(this.channel, "beatloop_activate");
            }
        }
    }
    loadTrackButton(field) {
        if (this.shiftPressed) {
            engine.setValue(this.channel, "CloneFromDeck", 0);
        } else {
            engine.setValue(this.channel, "LoadSelectedTrack", field.value);
        }
    }
    pflButton(field) {
        if (field.value > 0) {
            if (this.shiftPressed) {
                script.toggleControl(this.channel, "quantize");
            } else {
                script.toggleControl(this.channel, "pfl");
            }
        }
    }
    jogMove(field) {
        const deltas = this.wheelDeltas(field.value);
        let tickDelta = deltas[0];
        const timeDelta = deltas[1];

        if (engine.getValue(this.channel, "scratch2_enable")) {
            if (this.shiftPressed) {
                tickDelta *= 10;
            }
            engine.scratchTick(this.number, tickDelta);
        } else {
            const velocity = this.scalerJog(tickDelta, timeDelta);
            engine.setValue(this.channel, "jog", velocity);
        }
    }
    leftEncoderCallback(field) {
        const delta = this.leftEncoder.delta(field.value);

        if (this.shiftPressed) {
            if (delta === 1) {
                script.triggerControl(this.channel, "pitch_up_small");
            } else if (delta === -1) {
                script.triggerControl(this.channel, "pitch_down_small");
            }
        } else {
            if (this.leftEncoderPressed) {
                let beatjumpSize = engine.getValue(this.channel, "beatjump_size");
                if (delta === 1) {
                    beatjumpSize *= 2;
                } else if (delta === -1) {
                    beatjumpSize /= 2;
                }
                engine.setValue(this.channel, "beatjump_size", beatjumpSize);
            } else {
                if (delta === 1) {
                    script.triggerControl(this.channel, "beatjump_forward");
                } else if (delta === -1) {
                    script.triggerControl(this.channel, "beatjump_backward");
                }
            }
        }
    }
    rightEncoderCallback(field) {
        const delta = this.rightEncoder.delta(field.value);

        if (this.shiftPressed) {
            if (delta === 1) {
                script.triggerControl(this.channel, "beatjump_1_forward");
            } else if (delta === -1) {
                script.triggerControl(this.channel, "beatjump_1_backward");
            }
        } else {
            if (delta === 1) {
                script.triggerControl(this.channel, "loop_double");
            } else if (delta === -1) {
                script.triggerControl(this.channel, "loop_halve");
            }
        }
    }
    gainEncoderCallback(field) {
        const delta = 0.03333 * this.gainEncoder.delta(field.value);

        if (this.shiftPressed) {
            const currentPregain = engine.getParameter(this.channel, "pregain");
            engine.setParameter(this.channel, "pregain", currentPregain + delta);
        } else {
            const quickEffectGroup = "[QuickEffectRack1_" + this.channel + "]";
            if (this.gainEncoderPressed) {
                if (delta === 1) {
                    script.triggerControl(quickEffectGroup, "next_chain");
                } else if (delta === -1) {
                    script.triggerControl(quickEffectGroup, "prev_chain");
                }
            } else {
                const currentQuickEffectSuperKnob = engine.getParameter(quickEffectGroup, "super1");
                engine.setParameter(quickEffectGroup, "super1", currentQuickEffectSuperKnob + delta);
            }
        }
    }
    jogPress(field) {
        if (this.wheelPressInertiaTimer !== 0) {
            // The wheel was touched again, reset the timer.
            engine.stopTimer(this.wheelPressInertiaTimer);
            this.wheelPressInertiaTimer = 0;
        }
        if (field.value > this.calibration.jogPress.pressed) {
            engine.scratchEnable(this.number, 1024, 33.3333, 0.125, 0.125/8, true);
        } else {
            // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
            // Depending on how fast the platter was moving, lengthen the time we'll wait.
            const scratchRate = Math.abs(engine.getValue(this.channel, "scratch2"));
            // inertiaTime was experimentally determined. It should be enough time to allow the user to
            // press play after a backspin without normal playback starting before they can press the
            // button, but not so long that there is an awkward delay before stopping scratching after
            // a backspin.
            let inertiaTime;
            if (this.shiftPressed) {
                inertiaTime = Math.pow(1.7, scratchRate / 10) / 1.6;
            } else {
                inertiaTime = Math.pow(1.7, scratchRate) / 1.6;
            }
            if (inertiaTime < 100) {
            // Just do it now.
                this.finishJogPress();
            } else {
                this.wheelTouchInertiaTimer = engine.beginTimer(
                    inertiaTime,
                    this.finishJogPress.bind(this),
                    true);
            }
        }
    }
    outputCallback(value, group, key) {
        let ledValue = ButtonBrightnessOff;
        if (value) {
            ledValue = ButtonBrightnessOn;
        }
        this.controller.setOutput(group, key, ledValue, !this.parent.batchingLEDUpdate);
    }
    outputCallbackLoop(value, group, key) {
        let ledValue = ButtonBrightnessOff;
        if (engine.getValue(group, "loop_enabled")) {
            ledValue = 0x1F;
        }
        this.controller.setOutput(group, key, ledValue, !this.parent.batchingLEDUpdate);
    }
    outputCallbackDark(value, group, key) {
        let ledValue = 0x00;
        if (value) {
            ledValue = 0x1F;
        }
        this.controller.setOutput(group, key, ledValue, !this.parent.batchingLEDUpdate);
    }
    onVuMeterChanged(value, group, _key) {
    // This handler is called a lot so it should be as fast as possible.

        // Figure out number of fully-illuminated segments.
        const scaledValue = value * 4.0;
        const fullIllumCount = Math.floor(scaledValue);

        // Figure out how much the partially-illuminated segment is illuminated.
        const partialIllum = (scaledValue - fullIllumCount) * 0x1F;

        for (let i = 0; i <= 3; i++) {
            const key = "!VuMeter" + i;
            if (i < fullIllumCount) {
            // Don't update lights until they're all done, so the last term is false.
                this.controller.setOutput(group, key, 0x1F, false);
            } else if (i === fullIllumCount) {
                this.controller.setOutput(group, key, partialIllum, false);
            } else {
                this.controller.setOutput(group, key, 0x00, false);
            }
        }
        this.controller.OutputPackets.OutputReport0x80.send();
    }

    onLoopEnabledChanged(value, group, _key) {
        this.outputCallbackLoop(value, group, "loop_in");
        this.outputCallbackLoop(value, group, "loop_out");
    }
    setPadMode(padMode) {
        this.currentPadMode = padMode;
        for (const padButton in this.pads) {
            this.pads[padButton].padModeChanged();
        }
    }
    wheelDeltas(value) {
    // When the wheel is touched, four bytes change, but only the first behaves predictably.
    // It looks like the wheel is 1024 ticks per revolution.
        const tickval = value & 0xFF;
        let timeValue = value >>> 16;
        const previousTick =              this.lastTickValue;
        const previousTime = this.lastTickTime;
        this.lastTickValue = tickval;
        this.lastTickTime = timeValue;

        if (previousTime > timeValue) {
            // We looped around.  Adjust current time so that subtraction works.
            timeValue += 0x10000;
        }
        let timeDelta = timeValue - previousTime;
        if (timeDelta === 0) {
            // Spinning too fast to detect speed!  By not dividing we are guessing it took 1ms.
            timeDelta = 1;
        }

        let tickDelta = 0;
        if (previousTick >= 200 && tickval <= 100) {
            tickDelta = tickval + 256 - previousTick;
        } else if (previousTick <= 100 && tickval >= 200) {
            tickDelta = tickval - previousTick - 256;
        } else {
            tickDelta = tickval - previousTick;
        }
        return [tickDelta, timeDelta];
    }

    scalerJog(tickDelta, timeDelta) {
        if (engine.getValue(this.channel, "play")) {
            return (tickDelta / timeDelta) / 3;
        } else {
            return (tickDelta / timeDelta) * 2.0;
        }
    }

    finishJogPress() {
        this.wheelPressInertiaTimer = 0;
        const play = engine.getValue(this.channel, "play");
        if (play !== 0) {
            // If we are playing, just hand off to the engine.
            engine.scratchDisable(this.number, true);
        } else {
            // If things are paused, there will be a non-smooth handoff between scratching and jogging.
            // Instead, keep scratch on until the platter is not moving.
            const scratchRate = Math.abs(engine.getValue(this.channel, "scratch2"));
            if (scratchRate < 0.01) {
            // The platter is basically stopped, now we can disable scratch and hand off to jogging.
                engine.scratchDisable(this.number, true);
            } else {
            // Check again soon.
                this.wheelPressInertiaTimer = engine.beginTimer(20, this.finishJogPress.bind(this), true);
            }
        }
    }
}

class PadButton {
    constructor(deck, number) {
        this.deck = deck;
        this.controller = deck.controller;
        this.number = number;
        const samplerNumber = (this.deck.number -1) * 4 + this.number;
        this.samplerGroup = "[Sampler" + samplerNumber + "]";
        this.connections = [];
    }
    registerInputs(config) {
        config.hidReport.addControl(this.deck.channel, "!pad" + this.number, config.offset, "B", config.mask, false, this.pressHandler.bind(this));
    }
    registerOutputs(config) {
        this.registerLed("!pad_" + this.number + "_G", config.green);
        this.registerLed("!pad_" + this.number + "_B", config.blue);
    }
    registerLed(name, config) {
        config.hidReport.addOutput(this.deck.channel, name, config.offset, "B");
    }
    pressHandler(field) {
        const padMode = this.deck.currentPadMode;

        if (padMode === padModes.hotcue) {
            this.hotcueButton(field.value);
        } else if (padMode === padModes.introOutro) {
            this.introOutroButton(field.value);
        } else if (padMode === padModes.sampler) {
            this.samplerButton(field.value);
        }
    }
    hotcueButton(value) {
        if (this.deck.shiftPressed) {
            engine.setValue(this.deck.channel, "hotcue_" + this.number + "_clear", value);
        } else {
            engine.setValue(this.deck.channel, "hotcue_" + this.number + "_activate", value);
        }
    }
    introOutroButton(value) {
        if (this.deck.shiftPressed) {
            engine.setValue(this.deck.channel, introOutroKeys[this.number-1] + "_clear", value);
        } else {
            engine.setValue(this.deck.channel, introOutroKeys[this.number-1] + "_activate", value);
        }
    }
    samplerButton(value) {
        if (value === 0) {
            return;
        }
        if (this.deck.shiftPressed) {
            if (engine.getValue(this.samplerGroup, "play") === 1) {
                engine.setValue(this.samplerGroup, "play", 0);
            } else {
                script.triggerControl(this.samplerGroup, "eject");
            }
        } else {
            if (engine.getValue(this.samplerGroup, "track_loaded") === 0) {
                script.triggerControl(this.samplerGroup, "LoadSelectedTrack");
            } else {
                script.triggerControl(this.samplerGroup, "cue_gotoandplay");
            }
        }
    }
    outputHotcueCallback() {
        let color;
        const status = engine.getValue(this.deck.channel, `hotcue_${  this.number  }_status`);
        if (status === 1 || status === 2) {
            color = {green: 0, blue: 0x1F};
        } else {
            color = {green: 0, blue: 0};
        }
        this.sendPadColor(color);
    }
    outputIntroOutroCallback(value) {
        if (value > 0) {
            this.sendPadColor(introOutroColors[this.number-1]);
        } else {
            this.sendPadColor(introOutroColorsDim[this.number-1]);
        }
    }
    outputSamplerCallback() {
        if (engine.getValue(this.samplerGroup, "track_loaded")) {
            if (engine.getValue(this.samplerGroup, "play") === 1) {
                if (engine.getValue(this.samplerGroup, "repeat") === 1) {
                    this.sendPadColor({green: 0x1F, blue: 0x1F});
                } else {
                    this.sendPadColor({green: 0x1F, blue: 0});
                }
            } else {
                this.sendPadColor({green: 0x05, blue: 0x00});
            }
        } else {
            this.sendPadColor({green: 0, blue: 0});
        }
    }
    sendPadColor(color) {
        const padKey = "!pad_" + this.number + "_";
        const ColorBrightnessScaler = ButtonBrightnessOn / 0x1f;
        let green = color.green * ColorBrightnessScaler;
        let blue = color.blue * ColorBrightnessScaler;
        if (color.green === 0 && color.blue === 0) {
            green = ButtonBrightnessOff;
            blue = ButtonBrightnessOff;
        }
        this.controller.setOutput(this.deck.channel, padKey + "G", green, false);
        this.controller.setOutput(this.deck.channel, padKey + "B", blue, !this.deck.parent.batchingLEDUpdate);
    }
    padModeChanged() {
        const padMode = this.deck.currentPadMode;

        this.connections.forEach(function(connection) {
            connection.disconnect();
        });
        this.connections = [];

        if (padMode === padModes.hotcue) {
            this.connections.push(
                engine.makeConnection(this.deck.channel, `hotcue_${  this.number  }_status`, this.outputHotcueCallback.bind(this)));
        } else if (padMode === padModes.introOutro) {
            this.connections.push(engine.makeConnection(
                this.deck.channel, introOutroKeys[this.number-1] + "_enabled", this.outputIntroOutroCallback.bind(this)));
        } else if (padMode === padModes.sampler) {
            this.connections.push(engine.makeConnection(
                this.samplerGroup, "track_loaded", this.outputSamplerCallback.bind(this)));
            this.connections.push(engine.makeConnection(
                this.samplerGroup, "play", this.outputSamplerCallback.bind(this)));
            this.connections.push(engine.makeConnection(
                this.samplerGroup, "repeat", this.outputSamplerCallback.bind(this)));
        }

        this.connections.forEach(function(connection) {
            connection.trigger();
        });
    }
}

class Equalizer {
    constructor(deck) {
        this.deck = deck;
        this.controller = this.deck.controller;
        this.group = "[EqualizerRack1_" + this.deck.channel + "_Effect1]";
        this.params = {
            hi: new EqualizerParameter(this, 3),
            mid: new EqualizerParameter(this, 2),
            low: new EqualizerParameter(this, 1),
        };
    }
    registerInputs(config) {
        for (const param in this.params) {
            this.params[param].registerInputs(config[param]);
        }
    }
    calibrate(calibration) {
        for (const param in this.params) {
            this.params[param].calibrate(calibration[param]);
        }
    }
    enableSoftTakeover() {
        for (const param in this.params) {
            this.params[param].enableSoftTakeover();
        }
    }
}
class EqualizerParameter {
    constructor(equalizer, number) {
        this.equalizer = equalizer;
        this.group = equalizer.group;
        this.number = number;
        this.calibration = null;
    }
    registerInputs(config) {
        this.registerKnob("!parameter" + this.number, config, this.knob);
    }
    calibrate(calibration) {
        this.calibration = calibration;
    }
    enableSoftTakeover() {
        engine.softTakeover(this.group, "parameter" + this.number, true);
    }
    registerKnob(name, config, callback) {
        config.hidReport.addControl(this.group, name, config.offset, "H", 0xFFFF, false, callback.bind(this));
    }
    knob(field) {
        setKnobParameter(this.group, "parameter" + this.number, field.value, this.calibration);
    }
}

const longPressTimeoutMilliseconds = 275;
class EffectUnit {
    constructor(parent, number) {
        this.parent = parent;
        this.controller = parent.controller;
        this.number = number;
        this.group = "[EffectRack1_EffectUnit" + number + "]";
        this.effectButtonLongPressTimer= [0, 0, 0, 0];
        this.effectButtonIsLongPressed = [false, false, false, false];
        this.effectFocusLongPressTimer = 0;
        this.effectFocusChooseModeActive = false;
        this.effectFocusButtonPressedWhenParametersHidden =false;
        this.previouslyFocusedEffect = null;
        this.params = [
            new EffectParameter(this, 1),
            new EffectParameter(this, 2),
            new EffectParameter(this, 3),
        ];
        this.calibration = null;
    }
    registerInputs(config) {
        this.registerButton("!effect_focus_button", config.focus, this.effectFocusButton);
        this.registerButton("group_[Channel1]_enable", config.channel1);
        this.registerButton("group_[Channel2]_enable", config.channel2);
        this.registerKnob("!mix", config.mix, this.mixKnob);
        for (let i = 0; i < 3; i++) {
            this.params[i].registerInputs(config.params[i]);
        }
    }
    registerOutputs(config) {
        this.registerLed("!effect_focus_button", config.focus);
        this.registerLed("group_[Channel1]_enable", config.channel1);
        this.registerLed("group_[Channel2]_enable", config.channel2);
        for (let i = 0; i < 3; i++) {
            this.params[i].registerOutputs(config.params[i]);
        }
    }
    linkOutputs() {
        engine.makeConnection(this.group, "show_parameters", this.onShowParametersChange.bind(this));
        engine.makeConnection(this.group, "focused_effect", this.onFocusedEffectChange.bind(this)).trigger();
        engine.makeConnection(this.group, "group_[Channel1]_enable", this.outputCallback.bind(this)).trigger();
        engine.makeConnection(this.group, "group_[Channel2]_enable", this.outputCallback.bind(this)).trigger();
        this.connectEffectButtonLedsNormal();
    }
    calibrate(calibration) {
        this.calibration = calibration.mix;
        for (let i = 0; i < 3; i++) {
            this.params[i].calibrate(calibration.params[i]);
        }
    }
    enableSoftTakeover() {
        engine.softTakeover(this.group, "mix", true);
        for (let i = 0; i < 3; i++) {
            this.params[i].enableSoftTakeover();
        }
    }
    registerButton(name, config, callback) {
        if (callback !==undefined) {
            callback= callback.bind(this);
        }
        config.hidReport.addControl(this.group, name, config.offset, "B", config.mask, false, callback);
    }
    registerKnob(name, config, callback) {
        config.hidReport.addControl(this.group, name, config.offset, "H", 0xFFFF, false, callback.bind(this));
    }
    registerLed(name, config) {
        config.hidReport.addOutput(this.group, name, config.offset, "B");
    }
    effectFocusButton(field) {
        const showParameters = engine.getValue(this.group, "show_parameters");
        if (field.value > 0) {
            if (this.parent.decks[this.number -1].shiftPressed) {
                engine.setValue(this.group, "loaded_chain_preset", 1);
                return;
            }
            this.effectFocusLongPressTimer = engine.beginTimer(longPressTimeoutMilliseconds, () => {
                this.effectFocusChooseModeActive = true;
                this.connectEffectButtonLedsFocused();
            });
            if (!showParameters) {
                engine.setValue(this.group, "show_parameters", 1);
                this.effectFocusButtonPressedWhenParametersHidden = true;
            }
        } else {
            if (this.effectFocusLongPressTimer !== 0) {
                engine.stopTimer(this.effectFocusLongPressTimer);
                this.effectFocusLongPressTimer = 0;
            }

            if (this.effectFocusChooseModeActive) {
                this.effectFocusChooseModeActive = false;
                this.connectEffectButtonLedsNormal();
            } else if (showParameters && !this.effectFocusButtonPressedWhenParametersHidden) {
                engine.setValue(this.group, "show_parameters", 0);
            }

            this.effectFocusButtonPressedWhenParametersHidden = false;
        }
    }
    mixKnob(field) {
        setKnobParameter(this.group, "mix", field.value, this.calibration);
    }
    onShowParametersChange(value, group, _control) {
        if (value === 0) {
            if (engine.getValue(group, "show_focus") > 0) {
                engine.setValue(group, "show_focus", 0);
                this.previouslyFocusedEffect = engine.getValue(group, "focused_effect");
                engine.setValue(group, "focused_effect", 0);
            }
        } else {
            engine.setValue(group, "show_focus", 1);
            if (this.previouslyFocusedEffect !== null) {
                engine.setValue(group, "focused_effect", this.previouslyFocusedEffect);
            }
        }
        this.connectEffectButtonLedsNormal();
    }
    onFocusedEffectChange(value, group, _control) {
        this.controller.setOutput(this.group, "!effect_focus_button", value > 0 ? ButtonBrightnessOn : ButtonBrightnessOff, !this.parent.batchingLEDUpdate);
        if (value === 0) {
            for (let i = 1; i <= 2; i++) {
            // The previously focused effect is not available here, so iterate over all effects' parameter knobs.
                for (let j = 1; j < 3; j++) {
                    engine.softTakeoverIgnoreNextValue(group.slice(0, -1) + "_Effect" + i + "]", "parameter" + j);
                }
            }
        } else {
            for (let i = 1; i <= 2; i++) {
                engine.softTakeoverIgnoreNextValue(group.slice(0, -1) + "_Effect" + i + "]", "meta");
            }
        }
    }
    outputCallback(value, group, key) {
        let ledValue = ButtonBrightnessOff;
        if (value) {
            ledValue = ButtonBrightnessOn;
        }
        this.controller.setOutput(group, key, ledValue, !this.parent.batchingLEDUpdate);
    }
    shiftPressed() {
        return this.parent.decks[this.number - 1].shiftPressed;
    }
    connectEffectButtonLedsNormal() {
        this.connectEffectButtonLeds(this.params[0].connectLedNormal);
    }
    connectEffectButtonLedsFocused() {
        this.connectEffectButtonLeds(this.params[0].connectLedFocused);
    }
    connectEffectButtonLeds(fn) {
        this.parent.batchingLEDUpdate = true;
        for (let i = 0; i < 2; i++) {
            fn.bind(this.params[i])();
        }
        this.parent.batchingLEDUpdate = false;
        fn.bind(this.params[2])();
    }
}

class EffectParameter {
    constructor(effectUnit, number) {
        this.effectUnit = effectUnit;
        this.groupPrefix = effectUnit.group.slice(0, -1);
        this.group = effectUnit.group;
        this.controller = effectUnit.controller;
        this.number = number;
        this.longPressTimer = 0;
        this.isLongPressed = false;
        this.ledConnection = null;
        this.calibration = null;
    }
    registerInputs(config) {
        this.registerButton("!effectbutton" + this.number, config.button, this.effectButton);
        this.registerKnob("!effectknob" + this.number, config.knob, this.effectKnob);
    }
    registerOutputs(config) {
        config.hidReport.addOutput(this.group, "!effectbutton" + this.number, config.offset, "B");
    }
    calibrate(calibration) {
        this.calibration = calibration;
    }
    enableSoftTakeover() {
        const group = this.groupPrefix + "_Effect" + this.number + "]";
        engine.softTakeover(group, "meta", true);
        for (let i = 1; i <= 3; i++) {
            engine.softTakeover(group, "parameter" + i, true);
        }
    }
    registerButton(name, config, callback) {
        config.hidReport.addControl(this.group, name, config.offset, "B", config.mask, false, callback.bind(this));
    }
    registerKnob(name, config, callback) {
        config.hidReport.addControl(this.group, name, config.offset, "H", 0xFFFF, false, callback.bind(this));
    }
    effectButton(field) {
        const focusedEffect = engine.getValue(this.group, "focused_effect");

        if (field.value > 0) {
            if (this.effectUnit.shiftPressed()) {
                engine.setValue(this.group, "loaded_chain_preset", this.number+1);
            } else {
                if (this.effectUnit.effectFocusChooseModeActive) {
                    if (focusedEffect === this.number) {
                        engine.setValue(this.group, "focused_effect", 0);
                    } else {
                        engine.setValue(this.group, "focused_effect", this.number);
                    }
                    this.effectUnit.effectFocusChooseModeActive = false;
                } else {
                    this.toggle();
                    this.longPressTimer = engine.beginTimer(longPressTimeoutMilliseconds,
                        () => {
                            this.isLongPressed = true;
                            this.longPressTimer= 0;
                        },
                        true
                    );
                }
            }
        } else {
            engine.stopTimer(this.longPressTimer);
            this.longPressTimer = 0;
            if (this.isLongPressed) {
                this.toggle();
            }
            this.isLongPressed = false;
        }
    }
    toggle() {
        const button = this.getButtonGroupAndKey();
        script.toggleControl(button.group, button.key);
    }
    effectKnob(field) {
        const knob = this.getKnobGroupAndKey();
        setKnobParameter(knob.group, knob.key, field.value, this.calibration);
    }
    connectLedNormal() {
        const button = this.getButtonGroupAndKey();
        this.connectLed(button.group, button.key, this.ledCallbackNormal);
    }
    connectLedFocused() {
        this.connectLed(this.group, "focused_effect", this.ledCallbackFocused);
    }
    connectLed(group, key, callback) {
        if (this.ledConnection !== null) {
            this.ledConnection.disconnect();
        }
        this.ledConnection = engine.makeConnection(group, key, callback.bind(this));
        this.ledConnection.trigger();
    }
    getButtonGroupAndKey() {
        const focusedEffect = engine.getValue(this.group, "focused_effect");
        if (focusedEffect === 0) {
            return {
                group: this.groupPrefix + "_Effect" + this.number + "]",
                key: "enabled",
            };
        } else {
            return {
                group: this.groupPrefix + "_Effect" + focusedEffect + "]",
                key: "button_parameter" + this.number,
            };
        }
    }
    getKnobGroupAndKey() {
        const focusedEffect = engine.getValue(this.group, "focused_effect");
        if (focusedEffect === 0) {
            return {
                group: this.groupPrefix + "_Effect" + this.number + "]",
                key: "meta",
            };
        } else {
            return {
                group: this.groupPrefix + "_Effect" + focusedEffect + "]",
                key: "parameter" + this.number,
            };
        }
    }
    ledCallbackNormal(value) {
        this.ledCallback(value === 1);
    }
    ledCallbackFocused(value) {
        this.ledCallback(value === this.number);
    }
    ledCallback(value) {
        this.controller.setOutput(this.group, "!effectbutton" + this.number,
            value ? ButtonBrightnessOn : ButtonBrightnessOff, !this.effectUnit.parent.batchingLEDUpdate);
    }
}

class TraktorS2MK1Class {
    constructor() {
        this.controller = new HIDController();
        if (engine.getValue("[App]", "num_samplers") < 8) {
            engine.setValue("[App]", "num_samplers", 8);
        }

        // When true, packets will not be sent to the controller.
        // Used when updating multiple LEDs simultaneously.
        this.batchingLEDUpdate = false;

        this.browseEncoder = new Encoder();

        this.decks = [
            new DeckClass(this, 1),
            new DeckClass(this, 2),
        ];
        this.effectUnits = [
            new EffectUnit(this, 1),
            new EffectUnit(this, 2),
        ];
        this.rawCalibration = {};
        this.calibration = null;
    }
    registerInputPackets() {
        // Values in input report 0x01 are all buttons, except the jog wheels.
        // An exclamation point indicates a specially-handled function.  Everything else is a standard
        // Mixxx control object name.
        const InputReport0x01 = new HIDPacket("InputReport0x01", 0x01, this.inputReport0x01Callback.bind(this));
        // Most items in the input report 0x02 are controls that go from 0-4095.
        // There are also some 4 bit encoders.
        const InputReport0x02 = new HIDPacket("InputReport0x02", 0x02, this.inputReport0x02Callback.bind(this));

        this.decks[0].registerInputs({
            gainEncoderPress: {hidReport: InputReport0x01, offset: 0x0E, mask: 0x01},
            shift: {hidReport: InputReport0x01, offset: 0x0D, mask: 0x80},
            sync: {hidReport: InputReport0x01, offset: 0x0D, mask: 0x40},
            cue: {hidReport: InputReport0x01, offset: 0x0D, mask: 0x20},
            play: {hidReport: InputReport0x01, offset: 0x0D, mask: 0x10},
            pads: [
                {hidReport: InputReport0x01, offset: 0x0D, mask: 0x08},
                {hidReport: InputReport0x01, offset: 0x0D, mask: 0x04},
                {hidReport: InputReport0x01, offset: 0x0D, mask: 0x02},
                {hidReport: InputReport0x01, offset: 0x0D, mask: 0x01},
            ],
            loopIn: {hidReport: InputReport0x01, offset: 0x09, mask: 0x40},
            loopOut: {hidReport: InputReport0x01, offset: 0x09, mask: 0x20},
            samples: {hidReport: InputReport0x01, offset: 0x0B, mask: 0x02},
            reset: {hidReport: InputReport0x01, offset: 0x09, mask: 0x10},
            leftEncoderPress: {hidReport: InputReport0x01, offset: 0x0E, mask: 0x02},
            rightEncoderPress: {hidReport: InputReport0x01, offset: 0x0E, mask: 0x04},
            jogWheel: {hidReport: InputReport0x01, offset: 0x01},
            loadTrack: {hidReport: InputReport0x01, offset: 0x0B, mask: 0x08},
            pfl: {hidReport: InputReport0x01, offset: 0x09, mask: 0x80},
            rate: {hidReport: InputReport0x02, offset: 0x0F},
            leftEncoder: {hidReport: InputReport0x02, offset: 0x01, mask: 0xF0},
            rightEncoder: {hidReport: InputReport0x02, offset: 0x02, mask: 0x0F},
            volume: {hidReport: InputReport0x02, offset: 0x2B},
            gain: {hidReport: InputReport0x02, offset: 0x01, mask: 0x0F},
            jogPress: {hidReport: InputReport0x02, offset: 0x0D},
            eq: {
                hi: {hidReport: InputReport0x02, offset: 0x11},
                mid: {hidReport: InputReport0x02, offset: 0x25},
                low: {hidReport: InputReport0x02, offset: 0x27},
            }
        });
        this.decks[1].registerInputs({
            gainEncoderPress: {hidReport: InputReport0x01, offset: 0x0E, mask: 0x10},
            shift: {hidReport: InputReport0x01, offset: 0x0C, mask: 0x80},
            sync: {hidReport: InputReport0x01, offset: 0x0C, mask: 0x40},
            cue: {hidReport: InputReport0x01, offset: 0x0C, mask: 0x20},
            play: {hidReport: InputReport0x01, offset: 0x0C, mask: 0x10},
            pads: [
                {hidReport: InputReport0x01, offset: 0x0C, mask: 0x08},
                {hidReport: InputReport0x01, offset: 0x0C, mask: 0x04},
                {hidReport: InputReport0x01, offset: 0x0C, mask: 0x02},
                {hidReport: InputReport0x01, offset: 0x0C, mask: 0x01},
            ],
            loopIn: {hidReport: InputReport0x01, offset: 0x0B, mask: 0x40},
            loopOut: {hidReport: InputReport0x01, offset: 0x0B, mask: 0x20},
            samples: {hidReport: InputReport0x01, offset: 0x0B, mask: 0x01},
            reset: {hidReport: InputReport0x01, offset: 0x0B, mask: 0x10},
            leftEncoderPress: {hidReport: InputReport0x01, offset: 0x0E, mask: 0x20},
            rightEncoderPress: {hidReport: InputReport0x01, offset: 0x0E, mask: 0x40},
            jogWheel: {hidReport: InputReport0x01, offset: 0x05},
            loadTrack: {hidReport: InputReport0x01, offset: 0x0B, mask: 0x04},
            pfl: {hidReport: InputReport0x01, offset: 0x0B, mask: 0x80},
            rate: {hidReport: InputReport0x02, offset: 0x1F},
            leftEncoder: {hidReport: InputReport0x02, offset: 0x03, mask: 0xF0},
            rightEncoder: {hidReport: InputReport0x02, offset: 0x04, mask: 0x0F},
            volume: {hidReport: InputReport0x02, offset: 0x2D},
            gain: {hidReport: InputReport0x02, offset: 0x03, mask: 0x0F},
            jogPress: {hidReport: InputReport0x02, offset: 0x1D},
            eq: {
                hi: {hidReport: InputReport0x02, offset: 0x21},
                mid: {hidReport: InputReport0x02, offset: 0x23},
                low: {hidReport: InputReport0x02, offset: 0x29},
            }
        });
        this.effectUnits[0].registerInputs({
            focus: {hidReport: InputReport0x01, offset: 0x09, mask: 0x08},
            mix: {hidReport: InputReport0x02, offset: 0x0B},
            params: [
                {
                    button: {hidReport: InputReport0x01, offset: 0x09, mask: 0x04},
                    knob: {hidReport: InputReport0x02, offset: 0x09},
                },
                {
                    button: {hidReport: InputReport0x01, offset: 0x09, mask: 0x02},
                    knob: {hidReport: InputReport0x02, offset: 0x07},
                },
                {
                    button: {hidReport: InputReport0x01, offset: 0x09, mask: 0x01},
                    knob: {hidReport: InputReport0x02, offset: 0x05},
                },
            ],
            channel1: {hidReport: InputReport0x01, offset: 0x0A, mask: 0x02},
            channel2: {hidReport: InputReport0x01, offset: 0x0A, mask: 0x08},
        });
        this.effectUnits[1].registerInputs({
            focus: {hidReport: InputReport0x01, offset: 0x0A, mask: 0x80},
            mix: {hidReport: InputReport0x02, offset: 0x1B},
            params: [
                {
                    button: {hidReport: InputReport0x01, offset: 0x0A, mask: 0x40},
                    knob: {hidReport: InputReport0x02, offset: 0x19},
                },
                {
                    button: {hidReport: InputReport0x01, offset: 0x0A, mask: 0x20},
                    knob: {hidReport: InputReport0x02, offset: 0x17},
                },
                {
                    button: {hidReport: InputReport0x01, offset: 0x0A, mask: 0x10},
                    knob: {hidReport: InputReport0x02, offset: 0x15},
                },
            ],
            channel1: {hidReport: InputReport0x01, offset: 0x0A, mask: 0x01},
            channel2: {hidReport: InputReport0x01, offset: 0x0A, mask: 0x04},
        });
        InputReport0x01.addControl("[Master]", "!browse_encoder_press", 0x0E, "B", 0x08, false, this.browseEncoderPress.bind(this));

        InputReport0x02.addControl("[Master]", "!crossfader", 0x2F, "H", 0xFFFF, false, this.crossfader.bind(this));
        InputReport0x02.addControl("[Master]", "headMix", 0x31, "H");
        InputReport0x02.addControl("[Master]", "!samplerGain", 0x13, "H", 0xFFFF, false, this.samplerGainKnob.bind(this));
        InputReport0x02.addControl("[Library]", "!browse", 0x02, "B", 0xF0, false, this.browseEncoderCallback.bind(this));

        // Soft takeover for knobs

        // Set scalers
        this.controller.setScaler("headMix", this.scalerSlider);
        this.controller.setScaler("rate", this.scalerSlider);

        // Register packet
        this.controller.registerInputPacket(InputReport0x01);
        this.controller.registerInputPacket(InputReport0x02);
    }
    registerOutputPackets() {
        const OutputReport0x80 = new HIDPacket("OutputReport0x80", 0x80);

        this.decks[0].registerOutputs({
            trackLoaded: {hidReport: OutputReport0x80, offset: 0x1F},
            vuMeter: {hidReport: OutputReport0x80, offset: 0x15},
            peak: {hidReport: OutputReport0x80, offset: 0x01},
            reset: {hidReport: OutputReport0x80, offset: 0x06},
            loopIn: {hidReport: OutputReport0x80, offset: 0x02},
            loopOut: {hidReport: OutputReport0x80, offset: 0x05},
            pfl: {hidReport: OutputReport0x80, offset: 0x20},
            samples: {hidReport: OutputReport0x80, offset: 0x35},
            shift: {hidReport: OutputReport0x80, offset: 0x08},
            sync: {hidReport: OutputReport0x80, offset: 0x04},
            cue: {hidReport: OutputReport0x80, offset: 0x07},
            play: {hidReport: OutputReport0x80, offset: 0x03},
            pads: [
                {
                    green: {hidReport: OutputReport0x80, offset: 0x0C},
                    blue: {hidReport: OutputReport0x80, offset: 0x10},
                },
                {
                    green: {hidReport: OutputReport0x80, offset: 0x0B},
                    blue: {hidReport: OutputReport0x80, offset: 0x0F},
                },
                {
                    green: {hidReport: OutputReport0x80, offset: 0x0A},
                    blue: {hidReport: OutputReport0x80, offset: 0x0E},
                },
                {
                    green: {hidReport: OutputReport0x80, offset: 0x09},
                    blue: {hidReport: OutputReport0x80, offset: 0x0D},
                },
            ],
        });
        this.decks[1].registerOutputs({
            trackLoaded: {hidReport: OutputReport0x80, offset: 0x1E},
            vuMeter: {hidReport: OutputReport0x80, offset: 0x11},
            peak: {hidReport: OutputReport0x80, offset: 0x25},
            reset: {hidReport: OutputReport0x80, offset: 0x26},
            loopIn: {hidReport: OutputReport0x80, offset: 0x22},
            loopOut: {hidReport: OutputReport0x80, offset: 0x21},
            pfl: {hidReport: OutputReport0x80, offset: 0x1D},
            samples: {hidReport: OutputReport0x80, offset: 0x34},
            shift: {hidReport: OutputReport0x80, offset: 0x28},
            sync: {hidReport: OutputReport0x80, offset: 0x24},
            cue: {hidReport: OutputReport0x80, offset: 0x27},
            play: {hidReport: OutputReport0x80, offset: 0x23},
            pads: [
                {
                    green: {hidReport: OutputReport0x80, offset: 0x2C},
                    blue: {hidReport: OutputReport0x80, offset: 0x30},
                },
                {
                    green: {hidReport: OutputReport0x80, offset: 0x2B},
                    blue: {hidReport: OutputReport0x80, offset: 0x2F},
                },
                {
                    green: {hidReport: OutputReport0x80, offset: 0x2A},
                    blue: {hidReport: OutputReport0x80, offset: 0x2E},
                },
                {
                    green: {hidReport: OutputReport0x80, offset: 0x29},
                    blue: {hidReport: OutputReport0x80, offset: 0x2D},
                },
            ],
        });
        this.effectUnits[0].registerOutputs({
            focus: {hidReport: OutputReport0x80, offset: 0x1C},
            params: [
                {hidReport: OutputReport0x80, offset: 0x1B},
                {hidReport: OutputReport0x80, offset: 0x1A},
                {hidReport: OutputReport0x80, offset: 0x19},
            ],
            channel1: {hidReport: OutputReport0x80, offset: 0x3D},
            channel2: {hidReport: OutputReport0x80, offset: 0x3B},
        });
        this.effectUnits[1].registerOutputs({
            focus: {hidReport: OutputReport0x80, offset: 0x39},
            params: [
                {hidReport: OutputReport0x80, offset: 0x38},
                {hidReport: OutputReport0x80, offset: 0x37},
                {hidReport: OutputReport0x80, offset: 0x36},
            ],
            channel1: {hidReport: OutputReport0x80, offset: 0x3C},
            channel2: {hidReport: OutputReport0x80, offset: 0x3A},
        });

        OutputReport0x80.addOutput("[Master]", "!warninglight", 0x33, "B");

        this.controller.registerOutputPacket(OutputReport0x80);

        this.decks.forEach(function(deck) {
            deck.linkOutputs();
        });
        this.effectUnits.forEach(function(effectUnit) {
            effectUnit.linkOutputs();
        });
        this.decks.forEach(function(deck) {
            deck.setPadMode(padModes.hotcue);
        });
        this.controller.setOutput("[Master]", "!warninglight", 0x00, true);
    }
    calibrate() {
        this.rawCalibration.faders = new Uint8Array(controller.getFeatureReport(0xD0));
        this.rawCalibration.knobs = new Uint8Array(0x20 * 3);
        this.rawCalibration.knobs.set(new Uint8Array(controller.getFeatureReport(0xD1)), 0x00);
        this.rawCalibration.knobs.set(new Uint8Array(controller.getFeatureReport(0xD2)), 0x20);
        this.rawCalibration.knobs.set(new Uint8Array(controller.getFeatureReport(0xD3)), 0x40);
        this.rawCalibration.jogWheels = new Uint8Array(controller.getFeatureReport(0xD4));
        this.calibration = this.parseRawCalibration();
        for (let i = 0; i < 2; i++) {
            this.decks[i].calibrate(this.calibration.decks[i]);
        }
        for (let i = 0; i < 2; i++) {
            this.effectUnits[i].calibrate(this.calibration.effectUnits[i]);
        }
    }
    readCurrentPosition() {
        const report0x01 = new Uint8Array(controller.getInputReport(0x01));
        this.controller.parsePacket([0x01, ...Array.from(report0x01)]);
        const report0x02 = new Uint8Array(controller.getInputReport(0x02));
        // The first packet is ignored by HIDConstroller
        this.controller.parsePacket([0x02, ...Array.from(report0x02.map(x => x ^ 0xFF))]);
        this.controller.parsePacket([0x02, ...Array.from(report0x02)]);
    }
    enableSoftTakeover() {
        engine.softTakeover("[Master]", "crossfader", true);
        engine.softTakeover("[Master]", "headMix", true);
        for (let i = 1; i <= 8; i++) {
            engine.softTakeover("[Sampler" + i + "]", "pregain", true);
        }

        for (let i = 0; i < 2; i++) {
            this.decks[i].enableSoftTakeover();
        }
        for (let i = 0; i < 2; i++) {
            this.effectUnits[i].enableSoftTakeover();
        }
    }
    init() {
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

        this.calibrate();
        this.registerInputPackets();
        this.readCurrentPosition();
        this.enableSoftTakeover();

        const debugLEDs = false;
        if (debugLEDs) {
            const data = [];
            for (let i = 0; i < 61; i++) {
                data[i] = ButtonBrightnessOn;
            }
            data[0x31 - 1] = 0;
            data[0x32 - 1] = 0;
            controller.send(data, data.length, 0x80);
        } else {
            this.registerOutputPackets();
        }
    }

    shutdown() {
        const data = [];
        for (let i = 0; i < 61; i++) {
            data[i] = 0;
        }
        // light up warning light
        data[0x33 - 1] = ButtonBrightnessOn;
        controller.send(data, data.length, 0x80);
    }

    incomingData(data, length) {
        this.controller.parsePacket(data, length);
    }
    // The input report 0x01 handles buttons and jog wheels.
    inputReport0x01Callback(packet, data) {
        for (const name in data) {
            const field = data[name];
            if (field.name === "!jog_wheel") {
                this.controller.processControl(field);
                continue;
            }

            this.controller.processButton(field);
        }
    }
    // There are no buttons handled by input report 0x02, so this is a little simpler.
    inputReport0x02Callback(packet, data) {
        for (const name in data) {
            const field = data[name];
            this.controller.processControl(field);
        }
    }
    samplerGainKnob(field) {
        for (let i = 1; i <= 8; i++) {
            setKnobParameter("[Sampler" + i + "]", "pregain", field.value, this.calibration.sampler);
        }
    }

    toggleButton(field) {
        if (field.value > 0) {
            script.toggleControl(field.group, field.name);
        }
    }

    browseEncoderCallback(field) {
        const delta = this.browseEncoder.delta(field.value);

        if (this.shiftPressed()) {
            engine.setValue("[Library]", "ScrollVertical", delta);
        } else {
            engine.setValue("[Library]", "MoveVertical", delta);
        }
    }

    browseEncoderPress(field) {
        if (this.shiftPressed()) {
            engine.setValue("[Library]", "MoveFocusBackward", field.value);
        } else {
            engine.setValue("[Library]", "GoToItem", field.value);
        }

    }
    crossfader(field) {
        setFaderParameter("[Master]", "crossfader", field.value, this.calibration.crossfader);
    }

    scalerParameter(group, name, value) {
        const scaledValue =  script.absoluteLin(value, 0, 1, 16, 4080);
        return scaledValue;
    }

    scalerVolume(group, name, value) {
        if (group === "[Master]") {
            return script.absoluteNonLin(value, 0, 1, 4, 16, 4080);
        } else {
            return script.absoluteNonLin(value, 0, 0.25, 1, 16, 4080);
        }
    }

    scalerSlider(group, name, value) {
        return script.absoluteLin(value, -1, 1, 16, 4080);
    }

    parseRawCalibration() {
        return {
            decks: [
                {
                    volume: this.parseFaderCalibration(0x0C),
                    eq: {
                        hi: this.parseKnobCalibration(0x18),
                        mid: this.parseKnobCalibration(0x1E),
                        low: this.parseKnobCalibration(0x24),
                    },
                    jogPress: this.parseJogPressCalibration(0x00),
                },
                {
                    volume: this.parseFaderCalibration(0x10),
                    eq: {
                        hi: this.parseKnobCalibration(0x2A),
                        mid: this.parseKnobCalibration(0x30),
                        low: this.parseKnobCalibration(0x36),
                    },
                    jogPress: this.parseJogPressCalibration(0x04),
                },
            ],
            effectUnits: [
                {
                    mix: this.parseKnobCalibration(0x00),
                    params: [
                        this.parseKnobCalibration(0x06),
                        this.parseKnobCalibration(0x0C),
                        this.parseKnobCalibration(0x12),
                    ],
                },
                {
                    mix: this.parseKnobCalibration(0x42),
                    params: [
                        this.parseKnobCalibration(0x48),
                        this.parseKnobCalibration(0x4E),
                        this.parseKnobCalibration(0x54),
                    ]
                }
            ],
            crossfader: this.parseFaderCalibration(0x14),
            sampler: this.parseKnobCalibration(0x3C),
        };
    }
    parseKnobCalibration(index) {
        const data = this.rawCalibration.knobs;
        return {
            min: this.parseUint16Le(data, index),
            center: this.parseUint16Le(data, index+2),
            max: this.parseUint16Le(data, index+4),
        };
    }
    parseFaderCalibration(index) {
        const data = this.rawCalibration.faders;
        return {
            min: this.parseUint16Le(data, index),
            max: this.parseUint16Le(data, index+2),
        };
    }
    parseJogPressCalibration(index) {
        const data = this.rawCalibration.jogWheels;
        return {
            unpressed: this.parseUint16Be(data, index),
            pressed: this.parseUint16Be(data, index+2),
        };
    }
    parseUint16Le(data, index) {
        return data[index] + (data[index+1]<<8);
    }
    parseUint16Be(data, index) {
        return (data[index]<<8) + data[index+1];
    }

    shiftPressed() {
        return this.decks[0].shiftPressed || this.decks[1].shiftPressed;
    }
}

class Encoder {
    constructor() {
        this.previousValue = -1;
    }
    /// return value 1 === right turn
    /// return value -1 === left turn
    /// return value 0 when something weird happens/first delta
    delta(value) {
        if (this.previousValue === -1) {
            this.previousValue = value;
            return 0;
        }
        let dir = 0;
        if ((value + 1) % 16 === this.previousValue) {
            dir = -1;
        } else if ((this.previousValue + 1) % 16 === value) {
            dir = 1;
        }
        this.previousValue = value;
        return dir;
    }
}

const introOutroKeys = [
    "intro_start",
    "intro_end",
    "outro_start",
    "outro_end"
];

const introOutroColors = [
    {green: 0x1F, blue: 0},
    {green: 0x1F, blue: 0},
    {green: 0, blue: 0x1F},
    {green: 0, blue: 0x1F}
];

const introOutroColorsDim = [
    {green: 0x05, blue: 0},
    {green: 0x05, blue: 0},
    {green: 0, blue: 0x05},
    {green: 0, blue: 0x05}
];

const setKnobParameter = function(group, key, value, calibration) {
    let calibratedValue;
    if (value <= calibration.center) {
        calibratedValue = script.absoluteLin(value, 0, 0.5, calibration.min, calibration.center);
    } else {
        calibratedValue = script.absoluteLin(value, 0.5, 1, calibration.center, calibration.max);
    }
    engine.setParameter(group, key, calibratedValue);
};

const setFaderParameter = function(group, key, value, calibration) {
    const calibratedValue = script.absoluteLin(value, 0, 1, calibration.min, calibration.max);
    engine.setParameter(group, key, calibratedValue);
};


var TraktorS2MK1 = new TraktorS2MK1Class(); // eslint-disable-line no-var, no-unused-vars

// # Feature Report Description
//
// Feature Report `208` (`0xd0`)
// | Byte   | Endianness | Description                                               |
// |--------|------------|-----------------------------------------------------------|
// | 0      | -          | Always `d0`                                               |
// | 1..12  | -          | I don't know/always `01 00 00 00 10 00 f0 0f 10 00 f0 0f` |
// | 13..14 | LE         | Channel 1 volume fader bottom                             |
// | 15..16 | LE         | Channel 1 volume fader top                                |
// | 17..18 | LE         | Channel 2 volume fader bottom                             |
// | 19..20 | LE         | Channel 2 volume fader top                                |
// | 21..22 | LE         | Crossfader left                                           |
// | 23..24 | LE         | Crossfader right                                          |
// | 25..32 | -          | Padding (`ff`)                                            |

// Feature Report `209` (`0xd1`)
// | Byte   | Endianness | Description                      |
// |--------|------------|----------------------------------|
// | 0      | -          | Always `d1`                      |
// | 1..2   | LE         | Channel 1 FX dry/wet knob left   |
// | 3..4   | LE         | Channel 1 FX dry/wet knob center |
// | 5..6   | LE         | Channel 1 FX dry/wet knob right  |
// | 7..8   | LE         | Channel 1 FX 1 knob left         |
// | 9..10  | LE         | Channel 1 FX 1 knob center       |
// | 11.12  | LE         | Channel 1 FX 1 knob right        |
// | 13..14 | LE         | Channel 1 FX 2 knob left         |
// | 15..16 | LE         | Channel 1 FX 2 knob center       |
// | 17..18 | LE         | Channel 1 FX 2 knob right        |
// | 19..20 | LE         | Channel 1 FX 3 knob left         |
// | 21..22 | LE         | Channel 1 FX 3 knob center       |
// | 23..24 | LE         | Channel 2 FX 3 knob right        |
// | 25..26 | LE         | Channel 1 EQ hi knob left        |
// | 27..28 | LE         | Channel 1 EQ hi knob center      |
// | 29..30 | LE         | Channel 1 EQ hi knob right       |
// | 31..32 | LE         | Channel 1 EQ mid knob left       |

// Feature Report `210` (`0xd2`)
// | Byte   | Endianness | Description                  |
// |--------|------------|------------------------------|
// | 0      | -          | Always `d2`                  |
// | 1..2   | LE         | Channel 1 EQ mid knob center |
// | 3..4   | LE         | Channel 1 EQ mid knob right  |
// | 5..6   | LE         | Channel 1 EQ low knob left   |
// | 7..8   | LE         | Channel 1 EQ low knob center |
// | 9..10  | LE         | Channel 1 EQ low knob right  |
// | 11..12 | LE         | Channel 2 EQ hi knob left    |
// | 13..14 | LE         | Channel 2 EQ hi knob center  |
// | 15..16 | LE         | Channel 2 EQ hi knob right   |
// | 17..18 | LE         | Channel 2 EQ mid knob left   |
// | 19..20 | LE         | Channel 2 EQ mid knob center |
// | 21..22 | LE         | Channel 2 EQ mid knob right  |
// | 23..24 | LE         | Channel 2 EQ low knob left   |
// | 25..26 | LE         | Channel 2 EQ low knob center |
// | 27..28 | LE         | Channel 2 EQ low knob right  |
// | 29..30 | LE         | Sample knob left             |
// | 31..32 | LE         | Sample knob center           |

// Feature Report `211` (`0xd3`)
// | Byte   | Endianness | Description                      |
// |--------|------------|----------------------------------|
// | 0      | -          | Always `d3`                      |
// | 1..2   | LE         | Sample knob right                |
// | 3..4   | LE         | Channel 2 FX dry/wet knob left   |
// | 5..6   | LE         | Channel 2 FX dry/wet knob center |
// | 7..8   | LE         | Channel 2 FX dry/wet knob right  |
// | 9..10  | LE         | Channel 2 FX 1 knob left         |
// | 11..12 | LE         | Channel 2 FX 1 knob center       |
// | 13..14 | LE         | Channel 2 FX 1 knob right        |
// | 15..16 | LE         | Channel 2 FX 2 knob left         |
// | 17..18 | LE         | Channel 2 FX 2 knob center       |
// | 19..20 | LE         | Channel 2 FX 2 knob right        |
// | 21..22 | LE         | Channel 2 FX 3 knob left         |
// | 23..24 | LE         | Channel 2 FX 3 knob center       |
// | 25..26 | LE         | Channel 2 FX 3 knob right        |
// | 27..32 | -          | Padding (`ff`)                   |

// Feature Report `212` (`0xd4`)
// | Byte   | Endianness | Description                                               |
// |--------|------------|-----------------------------------------------------------|
// | 0      | -          | Always `d4`                                               |
// | 1..2   | BE         | Left jogwheel unpressed                                   |
// | 3..4   | BE         | Left jogwheel pressed                                     |
// | 5..6   | BE         | Right jogwheel unpressed                                  |
// | 7..8   | BE         | Right jogwheel pressed                                    |
// | 9      | -          | Left jogwhell calibration type (`00` user, `ff` factory)  |
// | 10     | -          | Right jogwhell calibration type (`00` user, `ff` factory) |
// | 11..32 | -          | Padding (`ff`)                                            |

// Feature Report `215` (`0xd7`) contains the factory calibration data for jogwheels:

// Restore of both jogwheels to factory settings:
// Read 215:  `d7 00 00 00 00 4e c2 26 82 0c 1b 0d b2 0c 9a 0e 16 2a ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff`
// Write 212: `d4 0c 1b 0c e6 0c 9a 0d 58 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff`

// | Byte   | Endianness | Description                  |
// |--------|------------|------------------------------|
// | 0      | -          | Always `d7`                  |
// | 1..8   | -          | I don't know                 |
// | 9..10  | BE         | Left jogwheel unpressed      |
// | 11..12 | BE         | Left jogwheel fully pressed  |
// | 13..14 | BE         | Right jogwheel unpressed     |
// | 15..16 | BE         | Right jogwheel fully pressed |
// | 17     | -          | `42` i guess                 |
// | 18..32 | -          | Padding (`ff`)               |

// `212`'s unpressed seems to be the same as unpressed. `212`'s pressed is `(pressed + unpressed) / 2`.

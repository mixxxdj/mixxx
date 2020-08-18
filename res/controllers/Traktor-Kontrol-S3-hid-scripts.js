///////////////////////////////////////////////////////////////////////////////////
// JSHint configuration                                                          //
///////////////////////////////////////////////////////////////////////////////////
/* global controller                                                             */
/* global HIDDebug                                                               */
/* global HIDPacket                                                              */
/* global HIDController                                                          */
/* jshint -W016                                                                  */
///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* Traktor Kontrol S3 HID controller script v1.00                                */
/* Last modification: August 2020                                                */
/* Author: Owen Williams                                                         */
/* https://www.mixxx.org/wiki/doku.php/native_instruments_traktor_kontrol_s3     */
/*                                                                               */
///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* TODO:                                                                         */
/*   * should we be lighting things inside input handlers? no because we want    */
/*   things to light up if activated in GUI, not controller.                     */
/*   * touch for track browse, loop control, beatjump?                           */
/*   * jog button                                                                */
/*   * star button                                                               */
/*                                                                               */
///////////////////////////////////////////////////////////////////////////////////

var TraktorS3 = new function() {
    this.controller = new HIDController();

    // ==== Friendly User Configuration ====
    // The pitch slider can operate either in absolute or relative mode.
    // In absolute mode:
    // * Moving the pitch slider works like normal
    // * Mixxx will use soft-takeover
    // * Pressing shift will adjust musical pitch instead of rate
    // * Keylock toggles on with down-press.
    //
    // In relative mode:
    // * The slider always moves, unless it has hit the end of the range inside Mixxx
    // * No soft-takeover
    // * Hold shift to move the pitch slider without adjusting the rate
    // * Hold keylock and move the pitch slider to adjust musical pitch
    // * keylock will still toggle on, but on release, not press.
    this.pitchSliderRelativeMode = true;

    // State for relative mode

    // "5" is the "filter" button below the other 4. It starts on but the
    // others start off.
    this.fxButtonState = {1: false, 2: false, 3: false, 4: false, 5: true};

    // When true, packets will not be sent to the controller.  Good for doing mass updates.
    this.batchingOutputs = false;

    // Microphone button
    this.microphonePressedTimer = 0; // Timer to distinguish between short and long press

    // VuMeter
    this.masterVuConnections = {
        "VuMeterL": {},
        "VuMeterR": {}
    };

    // The S3 has a set of predefined colors for many buttons. They are not
    // mapped by RGB, but 16 colors, each with 4 levels of brightness, plus white.
    this.controller.LEDColors = {
        OFF: 0x00,
        RED: 0x04,
        CARROT: 0x08,
        ORANGE: 0x0C,
        HONEY: 0x10,
        YELLOW: 0x14,
        LIME: 0x18,
        GREEN: 0x1C,
        AQUA: 0x20,
        CELESTE: 0x24,
        SKY: 0x28,
        BLUE: 0x2C,
        PURPLE: 0x30,
        FUSCHIA: 0x34,
        MAGENTA: 0x38,
        AZALEA: 0x3C,
        SALMON: 0x40,
        WHITE: 0x44
    };

    // Each color has four brightnesses.
    this.LEDDimValue = 0x00;
    this.LEDBrightValue = 0x02;

    // More User-friendly config: you can choose whatever colors you like for each deck!
    this.deckOutputColors = {
        "[Channel1]": "CARROT",
        "[Channel2]": "CARROT",
        "[Channel3]": "BLUE",
        "[Channel4]": "BLUE"
    };

    // FX 5 is the Filter
    this.fxLEDValue = {
        1: this.controller.LEDColors.RED,
        2: this.controller.LEDColors.GREEN,
        3: this.controller.LEDColors.BLUE,
        4: this.controller.LEDColors.YELLOW,
        5: this.controller.LEDColors.PURPLE,
    };

    this.colorMap = new ColorMapper({
        0xCC0000: this.controller.LEDColors.RED,
        0xCC5E00: this.controller.LEDColors.CARROT,
        0xCC7800: this.controller.LEDColors.ORANGE,
        0xCC9200: this.controller.LEDColors.HONEY,

        0xCCCC00: this.controller.LEDColors.YELLOW,
        0x81CC00: this.controller.LEDColors.LIME,
        0x00CC00: this.controller.LEDColors.GREEN,
        0x00CC49: this.controller.LEDColors.AQUA,

        0x00CCCC: this.controller.LEDColors.CELESTE,
        0x0091CC: this.controller.LEDColors.SKY,
        0x0000CC: this.controller.LEDColors.BLUE,
        0xCC00CC: this.controller.LEDColors.PURPLE,

        0xCC0091: this.controller.LEDColors.FUSCHIA,
        0xCC0079: this.controller.LEDColors.MAGENTA,
        0xCC477E: this.controller.LEDColors.AZALEA,
        0xCC4761: this.controller.LEDColors.SALMON,

        0xCCCCCC: this.controller.LEDColors.WHITE,
    });

    // callbacks
    this.samplerCallbacks = [];
};

TraktorS3.bind = function(fn, obj) {
    return function() {
        return fn.apply(obj, arguments);
    };
};

//// Deck Objects ////
// Decks are the physical controllers on either side of the controller.
// Each Deck can control 2 channels.
TraktorS3.Deck = function(deckNumber, group) {
    this.deckNumber = deckNumber;
    this.group = group;
    this.activeChannel = "[Channel" + deckNumber + "]";
    this.shiftPressed = false;

    // State for pitch slider relative mode
    this.pitchSliderLastValue = -1;
    this.keylockPressed = false;
    this.keyAdjusted = false;

    // Various states
    this.syncPressedTimer = 0;
    this.previewPressed = false;
    // state 0 is hotcues, 1 is samplers
    this.padModeState = 0;

    // Jog wheel state
    // tickReceived is used to detect when the platter has stopped moving.
    this.tickReceived = false;
    this.lastTickVal = 0;
    this.lastTickTime = 0;
    this.wheelTouchInertiaTimer = 0;

    // Knob encoder states (hold values between 0x0 and 0xF)
    // Rotate to the right is +1 and to the left is means -1
    this.browseKnobEncoderState = 0;
    this.loopKnobEncoderState = 0;
    this.moveKnobEncoderState = 0;
};

TraktorS3.Deck.prototype.activateChannel = function(channel) {
    if (channel.parentDeck !== this) {
        HIDDebug("Programming ERROR: tried to activate a channel with a deck that is not its parent");
        return;
    }
    this.activeChannel = channel.group;
    engine.softTakeoverIgnoreNextValue(this.activeChannel, "rate");
    TraktorS3.lightDeck(this.activeChannel);
};

// defineButton allows us to configure either the right deck or the left deck, depending on which
// is appropriate.  This avoids extra logic in the function where we define all the magic numbers.
// We use a similar approach in the other define funcs.
TraktorS3.Deck.prototype.defineButton = function(msg, name, deckOffset, deckBitmask, deck2Offset, deck2Bitmask, fn) {
    if (this.deckNumber === 2) {
        deckOffset = deck2Offset;
        deckBitmask = deck2Bitmask;
    }
    TraktorS3.registerInputButton(msg, this.group, name, deckOffset, deckBitmask, TraktorS3.bind(fn, this));
};

TraktorS3.Deck.prototype.defineJog = function(message, name, deckOffset, deck2Offset, callback) {
    // Jog wheels have 4 byte input
    if (this.deckNumber === 2) {
        deckOffset = deck2Offset;
    }
    message.addControl(this.group, name, deckOffset, "I", 0xFFFFFFFF);
    message.setCallback(this.group, name, TraktorS3.bind(callback, this));
};

// defineScaler configures ranged controls like knobs and sliders.
TraktorS3.Deck.prototype.defineScaler = function(msg, name, deckOffset, deckBitmask, deck2Offset, deck2Bitmask, fn) {
    if (this.deckNumber === 2) {
        deckOffset = deck2Offset;
        deckBitmask = deck2Bitmask;
    }
    TraktorS3.registerInputScaler(msg, this.group, name, deckOffset, deckBitmask, TraktorS3.bind(fn, this));
};

TraktorS3.Deck.prototype.registerInputs = function(messageShort, messageLong) {
    var deckFn = TraktorS3.Deck.prototype;
    this.defineButton(messageShort, "!play", 0x03, 0x01, 0x06, 0x02, deckFn.playHandler);
    this.defineButton(messageShort, "!cue_default", 0x02, 0x80, 0x06, 0x01, deckFn.cueHandler);
    this.defineButton(messageShort, "!shift", 0x01, 0x01, 0x04, 0x02, deckFn.shiftHandler);
    this.defineButton(messageShort, "!sync", 0x02, 0x08, 0x05, 0x10, deckFn.syncHandler);
    this.defineButton(messageShort, "!keylock", 0x02, 0x10, 0x05, 0x20, deckFn.keylockHandler);
    this.defineButton(messageShort, "!hotcues", 0x02, 0x20, 0x05, 0x40, deckFn.padModeHandler);
    this.defineButton(messageShort, "!samples", 0x02, 0x40, 0x05, 0x80, deckFn.padModeHandler);

    this.defineButton(messageShort, "!pad_1", 0x03, 0x02, 0x06, 0x04, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_2", 0x03, 0x04, 0x06, 0x08, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_3", 0x03, 0x08, 0x06, 0x10, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_4", 0x03, 0x10, 0x06, 0x20, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_5", 0x03, 0x20, 0x06, 0x40, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_6", 0x03, 0x40, 0x06, 0x80, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_7", 0x03, 0x80, 0x07, 0x01, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_8", 0x04, 0x01, 0x07, 0x02, deckFn.numberButtonHandler);

    // TODO: bind touch: 0x09/0x40, 0x0A/0x02
    this.defineButton(messageShort, "!SelectTrack", 0x0B, 0x0F, 0x0C, 0xF0, deckFn.selectTrackHandler);
    this.defineButton(messageShort, "!LoadSelectedTrack", 0x09, 0x01, 0x09, 0x08, deckFn.loadTrackHandler);
    this.defineButton(messageShort, "!PreviewTrack", 0x01, 0x08, 0x04, 0x10, deckFn.previewTrackHandler);
    this.defineButton(messageShort, "!LibraryFocus", 0x01, 0x40, 0x04, 0x80, deckFn.LibraryFocusHandler);
    this.defineButton(messageShort, "!QueueAutoDJ", 0x01, 0x20, 0x04, 0x40, deckFn.cueAutoDJHandler);

    // Loop control
    // TODO: bind touch detections: 0x0A/0x01, 0x0A/0x08
    this.defineButton(messageShort, "!SelectLoop", 0x0C, 0x0F, 0x0D, 0xF0, deckFn.selectLoopHandler);
    this.defineButton(messageShort, "!ActivateLoop", 0x09, 0x04, 0x09, 0x20, deckFn.activateLoopHandler);

    // Rev / FLUX / GRID
    this.defineButton(messageShort, "!reverse", 0x01, 0x04, 0x04, 0x08, deckFn.reverseHandler);
    this.defineButton(messageShort, "!slip_enabled", 0x01, 0x02, 0x04, 0x04, deckFn.fluxHandler);
    this.defineButton(messageShort, "quantize", 0x01, 0x80, 0x05, 0x01, deckFn.quantizeHandler);

    // Beatjump
    // TODO: bind touch detections: 0x09/0x80, 0x0A/0x04
    this.defineButton(messageShort, "!SelectBeatjump", 0x0B, 0xF0, 0x0D, 0x0F, deckFn.selectBeatjumpHandler);
    this.defineButton(messageShort, "!ActivateBeatjump", 0x09, 0x02, 0x09, 0x10, deckFn.activateBeatjumpHandler);

    // Jog wheels
    this.defineButton(messageShort, "!jog_touch", 0x0A, 0x10, 0x0A, 0x20, deckFn.jogTouchHandler);
    this.defineJog(messageShort, "!jog", 0x0E, 0x12, deckFn.jogHandler);

    this.defineScaler(messageLong, "rate", 0x01, 0xFFFF, 0x0D, 0xFFFF, deckFn.pitchSliderHandler);
};

TraktorS3.Deck.prototype.playHandler = function(field) {
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "start_stop", field.value);
    } else if (field.value === 1) {
        script.toggleControl(this.activeChannel, "play");
    }
};

TraktorS3.Deck.prototype.cueHandler = function(field) {
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "cue_gotoandstop", field.value);
    } else {
        engine.setValue(this.activeChannel, "cue_default", field.value);
    }
};

TraktorS3.Deck.prototype.shiftHandler = function(field) {
    // Mixxx only knows about one shift value, but this controller has two shift buttons.
    // This control object could get confused if both physical buttons are pushed at the same
    // time.
    engine.setValue("[Controls]", "touch_shift", field.value);
    this.shiftPressed = field.value;
    TraktorS3.basicOutputHandler(field.value, field.group, "!shift");
};

TraktorS3.Deck.prototype.syncHandler = function(field) {
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "beatsync_phase", field.value);
        // Light LED while pressed
        this.colorOutput(field.value, "sync_enabled");
        return;
    }

    // Unshifted
    if (field.value) {
        // We have to reimplement push-to-lock because it's only defined in the midi code
        // in Mixxx.
        if (engine.getValue(this.activeChannel, "sync_enabled") === 0) {
            script.triggerControl(this.activeChannel, "beatsync");
            // Start timer to measure how long button is pressed
            this.syncPressedTimer = engine.beginTimer(300, function() {
                engine.setValue(this.activeChannel, "sync_enabled", 1);
                // Reset sync button timer state if active
                if (this.syncPressedTimer !== 0) {
                    this.syncPressedTimer = 0;
                }
            }, true);

            // Light corresponding LED when button is pressed
            this.colorOutput(1, "sync_enabled");
        } else {
            // Deactivate sync lock
            // LED is turned off by the callback handler for sync_enabled
            engine.setValue(this.activeChannel, "sync_enabled", 0);
        }
    } else {
        if (this.syncPressedTimer !== 0) {
            // Timer still running -> stop it and unlight LED
            engine.stopTimer(this.syncPressedTimer);
            this.colorOutput(0, "sync_enabled");
        }
    }
};

TraktorS3.Deck.prototype.keylockHandler = function(field) {
    // shift + keylock resets pitch (in either mode).
    if (this.shiftPressed) {
        if (field.value) {
            engine.setValue(this.activeChannel, "pitch_adjust_set_default", 1);
        }
        return;
    }
    if (TraktorS3.pitchSliderRelativeMode) {
        if (field.value) {
            // In relative mode on down-press, reset the values and note that
            // the button is pressed.
            this.keylockPressed = true;
            this.keyAdjusted = false;
            return;
        }
        // On release, note that the button is released, and if the key *wasn't* adjusted,
        // activate keylock.
        this.keylockPressed = false;
        if (!this.keyAdjusted) {
            script.toggleControl(this.activeChannel, "keylock");
        }
        return;
    }

    // By default, do a basic press-to-toggle action.
    if (field.value === 0) {
        return;
    }
    script.toggleControl(this.activeChannel, "keylock");
};

// This handles when the mode buttons for the pads is pressed.
TraktorS3.Deck.prototype.padModeHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    if (this.padModeState === 0 && field.name === "!samples") {
        // If we are in hotcues mode and samples mode is activated
        engine.setValue("[Samplers]", "show_samplers", 1);
        this.padModeState = 1;
    } else if (field.name === "!hotcues") {
        // If we are in samples mode and hotcues mode is activated
        this.padModeState = 0;
    }
    this.lightPads();
};

TraktorS3.Deck.prototype.numberButtonHandler = function(field) {
    var padNumber = parseInt(field.id[field.id.length - 1]);
    var action = "";

    // Hotcues mode
    if (this.padModeState === 0) {
        // XXX: Should we be lighting things here?  I think not.
        // TraktorS3.lightHotcue(padNumber, field.value);
        if (field.value) {
            if (this.shiftPressed) {
                action = "_clear";
            } else {
                action = "_activate";
            }
            engine.setValue(this.activeChannel, "hotcue_" + padNumber + action, field.value);
        }
        return;
    }

    // Samples mode
    var sampler = padNumber;
    if (field.group === "deck2") {
        sampler += 8;
    }

    // var ledValue = field.value;
    // if (!field.value) {
    //     ledValue = engine.getValue("[Sampler" + sampler + "]", "track_loaded");
    // }
    // XXX: Again, should we be lighting?
    // this.colorOutput(ledValue, "!pad_" + padNumber);

    if (this.shiftPressed) {
        var playing = engine.getValue("[Sampler" + sampler + "]", "play");
        if (playing) {
            action = "cue_default";
        } else {
            action = "eject";
        }
        engine.setValue("[Sampler" + sampler + "]", action, field.value);
        return;
    }
    var loaded = engine.getValue("[Sampler" + sampler + "]", "track_loaded");
    if (loaded) {
        if (field.value) {
            action = "cue_gotoandplay";
        } else {
            action = "stop";
        }
        engine.setValue("[Sampler" + sampler + "]", action, 1);
        return;
    }
    engine.setValue("[Sampler" + sampler + "]", "LoadSelectedTrack", field.value);
};

TraktorS3.Deck.prototype.selectTrackHandler = function(field) {
    var delta = 1;
    if ((field.value + 1) % 16 === this.browseKnobEncoderState) {
        delta = -1;
    }
    this.browseKnobEncoderState = field.value;

    // When preview is held, rotating the library encoder scrolls through the previewing track.
    if (this.previewPressed) {
        var playPosition = engine.getValue("[PreviewDeck1]", "playposition");
        if (delta > 0) {
            playPosition += 0.0125;
        } else {
            playPosition -= 0.0125;
        }
        engine.setValue("[PreviewDeck1]", "playposition", playPosition);
        return;
    }

    if (this.shiftPressed) {
        engine.setValue("[Library]", "MoveHorizontal", delta);
    } else {
        engine.setValue("[Library]", "MoveVertical", delta);
    }
};

TraktorS3.Deck.prototype.loadTrackHandler = function(field) {
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "eject", field.value);
    } else {
        engine.setValue(this.activeChannel, "LoadSelectedTrack", field.value);
    }
};

TraktorS3.Deck.prototype.previewTrackHandler = function(field) {
    this.colorOutput(field.value, "!PreviewTrack");
    if (field.value === 1) {
        this.previewPressed = true;
        engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
    } else {
        this.previewPressed = false;
        engine.setValue("[PreviewDeck1]", "play", 0);
    }
};

TraktorS3.Deck.prototype.LibraryFocusHandler = function(field) {
    this.colorOutput(field.value, "!LibraryFocus");
    if (field.value === 0) {
        return;
    }

    script.toggleControl("[Library]", "MoveFocus");
};

TraktorS3.Deck.prototype.cueAutoDJHandler = function(field) {
    this.colorOutput(field.value, "!QueueAutoDJ");
    if (this.shiftPressed) {
        engine.setValue("[Library]", "AutoDjAddTop", field.value);
    } else {
        engine.setValue("[Library]", "AutoDjAddBottom", field.value);
    }
};


TraktorS3.Deck.prototype.selectLoopHandler = function(field) {
    if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
        script.triggerControl(this.activeChannel, "loop_halve");
    } else {
        script.triggerControl(this.activeChannel, "loop_double");
    }

    this.loopKnobEncoderState = field.value;
};

TraktorS3.Deck.prototype.activateLoopHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    var isLoopActive = engine.getValue(this.activeChannel, "loop_enabled");

    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "reloop_toggle", field.value);
    } else {
        if (isLoopActive) {
            engine.setValue(this.activeChannel, "reloop_toggle", field.value);
        } else {
            engine.setValue(this.activeChannel, "beatloop_activate", field.value);
        }
    }
};

TraktorS3.Deck.prototype.selectBeatjumpHandler = function(field) {
    var delta = 1;
    if ((field.value + 1) % 16 === this.moveKnobEncoderState) {
        delta = -1;
    }

    if (this.shiftPressed) {
        var beatjumpSize = engine.getValue(this.activeChannel, "beatjump_size");
        if (delta > 0) {
            engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize * 2);
        } else {
            engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize / 2);
        }
    } else {
        if (delta < 0) {
            script.triggerControl(this.activeChannel, "beatjump_backward");
        } else {
            script.triggerControl(this.activeChannel, "beatjump_forward");
        }
    }

    this.moveKnobEncoderState = field.value;
};

TraktorS3.Deck.prototype.activateBeatjumpHandler = function(field) {
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "reloop_andstop", field.value);
    } else {
        engine.setValue(this.activeChannel, "beatlooproll_activate", field.value);
    }
};

TraktorS3.Deck.prototype.reverseHandler = function(field) {
    this.basicOutput(field.value, "reverse");
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "reverseroll", field.value);
    } else {
        engine.setValue(this.activeChannel, "reverse", field.value);
    }
};

TraktorS3.Deck.prototype.fluxHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    script.toggleControl(this.activeChannel, "slip_enabled");
};

TraktorS3.Deck.prototype.quantizeHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "beats_translate_curpos", field.value);
    } else {
        script.toggleControl(this.activeChannel, "quantize");
    }
};

TraktorS3.Deck.prototype.jogTouchHandler = function(field) {
    if (this.wheelTouchInertiaTimer !== 0) {
        // The wheel was touched again, reset the timer.
        engine.stopTimer(this.wheelTouchInertiaTimer);
        this.wheelTouchInertiaTimer = 0;
    }
    if (field.value !== 0) {
        engine.setValue(this.activeChannel, "scratch2_enable", true);
    } else {
        // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
        // Depending on how fast the platter was moving, lengthen the time we'll wait.
        var scratchRate = Math.abs(engine.getValue(this.activeChannel, "scratch2"));
        // Note: inertiaTime multiplier is controller-specific and should be factored out.
        var inertiaTime = Math.pow(1.8, scratchRate) * 2;
        if (inertiaTime < 100) {
            // Just do it now.
            this.finishJogTouch();
        } else {
            this.wheelTouchInertiaTimer = engine.beginTimer(
                inertiaTime, this.finishJogTouch, true);
        }
    }
};

TraktorS3.Deck.prototype.wheelDeltas = function(value) {
    // When the wheel is touched, 1 byte measures distance ticks, the other
    // three represent a timer value. We can use the amount of time required for
    // the number of ticks to elapse to get a velocity.
    var tickval = value & 0xFF;
    var timeval = value >>> 8;
    var prevTick = 0;
    var prevTime = 0;

    prevTick = this.lastTickVal;
    prevTime = this.lastTickTime;
    this.lastTickVal = tickval;
    this.lastTickTime = timeval;

    if (prevTime > timeval) {
        // We looped around.  Adjust current time so that subtraction works.
        timeval += 0x100000;
    }
    var timeDelta = timeval - prevTime;
    if (timeDelta === 0) {
        // Spinning too fast to detect speed!  By not dividing we are guessing it took 1ms.
        // (This is almost certainly not going to happen on this controller.)
        timeDelta = 1;
    }

    var tickDelta = 0;

    // Very generous 8bit loop-around detection.
    if (prevTick >= 200 && tickval <= 100) {
        tickDelta = tickval + 256 - prevTick;
    } else if (prevTick <= 100 && tickval >= 200) {
        tickDelta = tickval - prevTick - 256;
    } else {
        tickDelta = tickval - prevTick;
    }

    return [tickDelta, timeDelta];
};

TraktorS3.Deck.prototype.finishJogTouch = function() {
    this.wheelTouchInertiaTimer = 0;

    // If we've received no ticks since the last call, we are stopped.
    if (!this.tickReceived) {
        engine.setValue(this.activeChannel, "scratch2", 0.0);
        engine.setValue(this.activeChannel, "scratch2_enable", false);
        this.playIndicatorHandler(0, this.activeChannel);
    } else {
        // Check again soon.
        this.wheelTouchInertiaTimer = engine.beginTimer(
            100, this.finishJogTouch, true);
    }
    this.tickReceived = false;
};

TraktorS3.Deck.prototype.jogHandler = function(field) {
    this.tickReceived = true;
    var deltas = this.wheelDeltas(field.value);
    var tickDelta = deltas[0];
    var timeDelta = deltas[1];

    // The scratch rate is the ratio of the wheel's speed to "regular" speed,
    // which we're going to call 33.33 RPM.  It's 768 ticks for a circle, and
    // 400000 ticks per second, and 33.33 RPM is 1.8 seconds per rotation, so
    // the standard speed is 768 / (400000 * 1.8)
    var thirtyThree = 768 / 720000;

    // Our actual speed is tickDelta / timeDelta.  Take the ratio of those to get the
    // rate ratio.
    var velocity = (tickDelta / timeDelta) / thirtyThree;

    // The Mixxx scratch code tries to do accumulation and time calculation itself.
    // This controller is better, so just use its values.
    if (engine.getValue(this.activeChannel, "scratch2_enable")) {
        engine.setValue(this.activeChannel, "scratch2", velocity);
    } else {
        // If we're playing, just nudge.
        if (engine.getValue(this.activeChannel, "play")) {
            velocity /= 4;
        } else {
            velocity *= 2;
        }
        engine.setValue(this.activeChannel, "jog", velocity);
    }
};

TraktorS3.Deck.prototype.pitchSliderHandler = function(field) {
    var value = field.value / 4095;
    if (TraktorS3.pitchSliderRelativeMode) {
        if (this.pitchSliderLastValue === -1) {
            this.pitchSliderLastValue = value;
        } else {
            // If shift is pressed, don't update any values.
            if (this.shiftPressed) {
                this.pitchSliderLastValue = value;
                return;
            }

            var relVal;
            if (this.keylockPressed) {
                relVal = 1.0 - engine.getParameter(this.activeChannel, "pitch_adjust");
            } else {
                relVal = engine.getParameter(this.activeChannel, "rate");
            }
            relVal += value - this.pitchSliderLastValue;
            this.pitchSliderLastValue = value;
            value = Math.max(0.0, Math.min(1.0, relVal));

            if (this.keylockPressed) {
                // To match the pitch change from adjusting the rate, flip the pitch
                // adjustment.
                engine.setParameter(this.activeChannel, "pitch_adjust", 1.0 - value);
                this.keyAdjusted = true;
            } else {
                engine.setParameter(this.activeChannel, "rate", value);
            }
        }
        return;
    }

    if (this.shiftPressed) {
        // To match the pitch change from adjusting the rate, flip the pitch
        // adjustment.
        engine.setParameter(this.activeChannel, "pitch_adjust", 1.0 - value);
    } else {
        engine.setParameter(this.activeChannel, "rate", value);
    }
};

//// Deck Outputs ////

TraktorS3.Deck.prototype.defineOutput = function(packet, name, offsetA, offsetB) {
    switch (this.deckNumber) {
    case 1:
        packet.addOutput(this.group, name, offsetA, "B");
        break;
    case 2:
        packet.addOutput(this.group, name, offsetB, "B");
        break;
    }
};

TraktorS3.Deck.prototype.registerOutputs = function(outputA, _outputB) {
    this.defineOutput(outputA, "!shift", 0x01, 0x1A);
    this.defineOutput(outputA, "slip_enabled", 0x02, 0x1B);
    this.defineOutput(outputA, "reverse", 0x03, 0x1C);
    this.defineOutput(outputA, "!PreviewTrack", 0x04,  0x1D);
    this.defineOutput(outputA, "!QueueAutoDJ", 0x06, 0x1F);
    this.defineOutput(outputA, "!LibraryFocus", 0x07, 0x20);
    this.defineOutput(outputA, "quantize", 0x08, 0x21);
    this.defineOutput(outputA, "sync_enabled", 0x0C, 0x25);
    this.defineOutput(outputA, "keylock", 0x0D, 0x26);
    this.defineOutput(outputA, "hotcues", 0x0E, 0x27);
    this.defineOutput(outputA, "samples", 0x0F, 0x28);
    this.defineOutput(outputA, "cue_indicator", 0x10, 0x29);
    this.defineOutput(outputA, "play_indicator", 0x11, 0x2A);

    this.defineOutput(outputA, "!pad_1", 0x12, 0x2B);
    this.defineOutput(outputA, "!pad_2", 0x13, 0x2C);
    this.defineOutput(outputA, "!pad_3", 0x14, 0x2D);
    this.defineOutput(outputA, "!pad_4", 0x15, 0x2E);
    this.defineOutput(outputA, "!pad_5", 0x16, 0x2F);
    this.defineOutput(outputA, "!pad_6", 0x17, 0x30);
    this.defineOutput(outputA, "!pad_7", 0x18, 0x31);
    this.defineOutput(outputA, "!pad_8", 0x19, 0x32);

    // this.defineOutput(outputA, "addTrack", 0x03, 0x2A);

    var wheelOffsets = [0x43, 0x4B];
    for (var i = 0; i < 8; i++) {
        this.defineOutput(outputA, "!" + "wheel" + i, wheelOffsets[0] + i, wheelOffsets[1] + i);
    }
};

TraktorS3.Deck.prototype.defineLink = function(key, callback) {
    switch (this.deckNumber) {
    case 1:
        TraktorS3.controller.linkOutput("deck1", key, "[Channel1]", key, callback);
        engine.connectControl("[Channel3]", key, callback);
        break;
    case 2:
        TraktorS3.controller.linkOutput("deck2", key, "[Channel2]", key, callback);
        engine.connectControl("[Channel4]", key, callback);
        break;
    }
};

TraktorS3.Deck.prototype.linkOutputs = function() {
    var deckFn = TraktorS3.Deck.prototype;
    this.defineLink("play_indicator", TraktorS3.bind(deckFn.playIndicatorHandler, this));
    this.defineLink("cue_indicator", TraktorS3.bind(deckFn.colorOutputHandler, this));
    this.defineLink("sync_enabled", TraktorS3.bind(deckFn.colorOutputHandler, this));
    this.defineLink("keylock", TraktorS3.bind(deckFn.colorOutputHandler, this));
    this.defineLink("slip_enabled", TraktorS3.bind(deckFn.colorOutputHandler, this));
    this.defineLink("quantize", TraktorS3.bind(deckFn.colorOutputHandler, this));
    this.defineLink("reverse", TraktorS3.bind(deckFn.basicOutputHandler, this));
};

TraktorS3.Deck.prototype.deckBaseColor = function() {
    return TraktorS3.controller.LEDColors[TraktorS3.deckOutputColors[this.activeChannel]];
};

// outputHandler drives lights that only have one color.
TraktorS3.Deck.prototype.basicOutputHandler = function(value, _group, key) {
    this.basicOutput(value, key);
};

TraktorS3.Deck.prototype.basicOutput = function(value, key) {
    // incoming value will be a channel, we have to resolve back to
    // deck.
    var ledValue = 0x20;
    if (value === 1 || value === true) {
        // On value
        ledValue = 0x77;
    }
    TraktorS3.controller.setOutput(this.group, key, ledValue, !TraktorS3.batchingOutputs);
};

TraktorS3.Deck.prototype.colorOutputHandler = function(value, _group, key) {
    this.colorOutput(value, key);
};

// colorOutput drives lights that have the palettized multicolor lights.
TraktorS3.Deck.prototype.colorOutput = function(value, key) {
    var ledValue = this.deckBaseColor();

    if (value === 1 || value === true) {
        ledValue += TraktorS3.LEDBrightValue;
    } else {
        ledValue += TraktorS3.LEDDimValue;
    }
    TraktorS3.controller.setOutput(this.group, key, ledValue, !TraktorS3.batchingOutputs);
};

TraktorS3.Deck.prototype.playIndicatorHandler = function(value, group, key) {
    // Also call regular handler
    this.basicOutput(value, key);
    this.wheelOutputByValue(group, value);
};

TraktorS3.Deck.prototype.colorForHotcue = function(num) {
    var colorCode = engine.getValue(this.activeChannel, "hotcue_" + num + "_color");
    return TraktorS3.colorMap.getValueForNearestColor(colorCode);
};

TraktorS3.Deck.prototype.lightHotcue = function(number) {
    var loaded = engine.getValue(this.activeChannel, "hotcue_" + number + "_enabled");
    var active = engine.getValue(this.activeChannel, "hotcue_" + number + "_activate");
    var ledValue = TraktorS3.controller.LEDColors.WHITE;
    if (loaded) {
        ledValue = this.colorForHotcue(number);
        ledValue += TraktorS3.LEDDimValue;
    }
    if (active) {
        ledValue += TraktorS3.LEDBrightValue;
    } else {
        ledValue += TraktorS3.LEDDimValue;
    }
    TraktorS3.controller.setOutput(this.group, "!pad_" + number, ledValue, !TraktorS3.batchingOutputs);
};

TraktorS3.Deck.prototype.lightPads = function() {
    // Samplers
    if (this.padModeState === 1) {
        this.colorOutput(0, "hotcues");
        this.colorOutput(1, "samples");
        for (var i = 1; i <= 8; i++) {
            var idx = i;
            if (this.group === "deck2") {
                idx += 8;
            }
            var loaded = engine.getValue("[Sampler" + idx + "]", "track_loaded");
            this.colorOutput(loaded, "!pad_" + i);
        }
    } else {
        this.colorOutput(1, "hotcues");
        this.colorOutput(0, "samples");
        for (i = 1; i <= 8; ++i) {
            this.lightHotcue(i);
        }
    }
};

TraktorS3.Deck.prototype.wheelOutputByValue = function(group, value) {
    if (group !== this.activeChannel) {
        return;
    }

    var ledValue = this.deckBaseColor();

    if (value === 1 || value === true) {
        ledValue += TraktorS3.LEDBrightValue;
    } else {
        ledValue = 0x00;
    }
    this.wheelOutputHandler(group,
        [ledValue, ledValue, ledValue, ledValue, ledValue, ledValue, ledValue, ledValue]);
};

TraktorS3.Deck.prototype.wheelOutputHandler = function(group, valueArray) {
    if (group !== this.activeChannel) {
        return;
    }

    var sendPacket = !TraktorS3.batchingOutputs;
    for (var i = 0; i < 8; i++) {
        TraktorS3.controller.setOutput(this.group, "!wheel" + i, valueArray[i], false);
    }
    if (sendPacket) {
        for (var packetName in TraktorS3.controller.OutputPackets) {
            TraktorS3.controller.OutputPackets[packetName].send();
        }
    }
};

/////////////////////////
//// Channel Objects ////
////
//// Channels don't have much state, just the fx button state.
TraktorS3.Channel = function(parentDeck, group) {
    this.parentDeck = parentDeck;
    this.group = group;
    this.fxEnabledState = true;

    this.trackDurationSec = 0;
    this.endOfTrackTimer = 0;
    this.endOfTrack = false;
    this.endOfTrackBlinkState = 0;

    this.vuConnection = {};
    this.clipConnection = {};
    this.hotcueCallbacks = [];
};

TraktorS3.Channel.prototype.fxEnableHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    this.fxEnabledState = !this.fxEnabledState;
    this.colorOutput(this.fxEnabledState, "!fxEnabled");
    TraktorS3.toggleFX();
};

// Finds the shortest distance between two angles on the wheel, assuming
// 0-8.0 angle value.
TraktorS3.wheelSegmentDistance = function(segNum, angle) {
    // Account for wraparound
    if (Math.abs(segNum - angle) > 4) {
        if (angle > segNum) {
            segNum += 8;
        } else {
            angle += 8;
        }
    }
    return Math.abs(angle - segNum);
};

TraktorS3.Channel.prototype.trackLoadedHandler = function() {
    var trackSamples = engine.getValue(this.group, "track_samples");
    if (trackSamples === 0) {
        this.trackDurationSec = 0;
        return;
    }
    var trackSampleRate = engine.getValue(this.group, "track_samplerate");
    // Assume stereo.
    this.trackDurationSec = trackSamples / 2.0 / trackSampleRate;
};

TraktorS3.Channel.prototype.endOfTrackHandler = function(value) {
    this.endOfTrack = value;
    if (!value) {
        if (this.endOfTrackTimer) {
            engine.stopTimer(this.endOfTrackTimer);
            this.endOfTrackTimer = 0;
        }
        return;
    }
    this.endOfTrackTimer = engine.beginTimer(400, function() {
        this.endOfTrackBlinkState = !this.endOfTrackBlinkState;
    }, false);
};

TraktorS3.Channel.prototype.playpositionChanged = function(value) {
    if (this.parentDeck.activeChannel !== this.group) {
        return;
    }

    // How many segments away from the actual angle should we light?
    // (in both directions, so "2" will light up to four segments)
    if (this.trackDurationSec === 0) {
        var samples = engine.getValue(this.group, "track_samples");
        if (samples > 0) {
            this.trackLoadedHandler();
        } else {
            // No track loaded, abort
            return;
        }
    }
    var elapsed = value * this.trackDurationSec;

    var rotations = elapsed * (1 / 1.8);  // 1/1.8 is rotations per second
    // Calculate angle from 0-1.0
    var angle = rotations - Math.floor(rotations);
    // The wheel has 8 segments
    var wheelAngle = 8.0 * angle;
    var baseLedValue = this.channelBaseColor();
    // Reduce the dimming distance at the end of track.
    var dimDistance = this.endOfTrack ? 2.5 : 1.5;
    var segValues = [0, 0, 0, 0, 0, 0, 0, 0];
    for (var seg = 0; seg < 8; seg++) {
        var distance = TraktorS3.wheelSegmentDistance(seg, wheelAngle);
        var brightVal = Math.round(4 * (1.0 - (distance / dimDistance)));
        if (this.endOfTrack) {
            dimDistance = 1.5;
            brightVal = Math.round(4 * (1.0 - (distance / dimDistance)));
            if (this.endOfTrackBlinkState) {
                brightVal = brightVal > 0x03 ? 0x04 : 0x02;
            } else {
                brightVal = brightVal > 0x02 ? 0x04 : 0x00;
            }
        }
        if (brightVal <= 0) {
            segValues[seg] = 0x00;
        } else {
            segValues[seg] = baseLedValue + brightVal - 1;
        }
    }
    this.parentDeck.wheelOutputHandler(this.group, segValues);
};

TraktorS3.Channel.prototype.linkOutputs = function() {
    this.vuConnection = engine.makeConnection(this.group, "VuMeter", TraktorS3.bind(TraktorS3.Channel.prototype.channelVuMeterHandler, this));
    this.clipConnection = engine.makeConnection(this.group, "PeakIndicator", TraktorS3.peakOutputHandler);
    for (var j = 1; j <= 8; j++) {
        this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_enabled",
            TraktorS3.bind(TraktorS3.Channel.prototype.hotcuesOutputHandler, this)));
        this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_activate",
            TraktorS3.bind(TraktorS3.Channel.prototype.hotcuesOutputHandler, this)));
        TraktorS3.linkChannelOutput(this.group, "pfl", TraktorS3.basicOutputHandler);
    }
};

TraktorS3.Channel.prototype.channelBaseColor = function() {
    return TraktorS3.controller.LEDColors[TraktorS3.deckOutputColors[this.group]];
};

// colorOutput drives lights that have the palettized multicolor lights.
TraktorS3.Channel.prototype.colorOutput = function(value, key) {
    var ledValue = this.channelBaseColor();
    if (value === 1 || value === true) {
        ledValue += TraktorS3.LEDBrightValue;
    } else {
        ledValue  += TraktorS3.LEDDimValue;
    }
    TraktorS3.controller.setOutput(this.group, key, ledValue, !TraktorS3.batchingOutputs);
};

TraktorS3.Channel.prototype.hotcuesOutputHandler = function(_value, group, key) {
    var deck = TraktorS3.Channels[group].parentDeck;
    if (deck.activeChannel !== group) {
        // Not active, ignore
        return;
    }
    var matches = key.match(/hotcue_(\d+)_/);
    if (matches.length !== 2) {
        HIDDebug("Didn't get expected hotcue number from string: " + key);
        return;
    }
    var cueNum = matches[1];
    deck.lightHotcue(cueNum);
};

TraktorS3.Channel.prototype.channelVuMeterHandler = function(value, _group, key) {
    this.vuMeterHandler(value, key, 14);
};

TraktorS3.Channel.prototype.masterVuMeterHandler = function(value, _group, key) {
    this.vuMeterHandler(value, key, 8);
};

TraktorS3.Channel.prototype.vuMeterHandler = function(value, key, segments) {
    // return;
    // This handler is called a lot so it should be as fast as possible.
    var scaledValue = value * segments;
    var fullIllumCount = Math.floor(scaledValue);

    // Figure out how much the partially-illuminated segment is illuminated.
    var partialIllum = (scaledValue - fullIllumCount) * 0x7F;

    for (var i = 0; i < segments; i++) {
        var segmentKey = "!" + key + i;
        if (i < fullIllumCount) {
            // Don't update lights until they're all done, so the last term is false.
            TraktorS3.controller.setOutput(this.group, segmentKey, 0x7F, false);
        } else if (i === fullIllumCount) {
            TraktorS3.controller.setOutput(this.group, segmentKey, partialIllum, false);
        } else {
            TraktorS3.controller.setOutput(this.group, segmentKey, 0x00, false);
        }
    }
    TraktorS3.controller.OutputPackets["outputB"].send();
};

TraktorS3.registerInputPackets = function() {
    var messageShort = new HIDPacket("shortmessage", 0x01, this.messageCallback);
    var messageLong = new HIDPacket("longmessage", 0x02, this.messageCallback);

    for (var idx in TraktorS3.Decks) {
        var deck = TraktorS3.Decks[idx];
        deck.registerInputs(messageShort, messageLong);
    }

    this.registerInputButton(messageShort, "[Channel1]", "!switchDeck", 0x02, 0x02, this.deckSwitchHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!switchDeck", 0x05, 0x04, this.deckSwitchHandler);
    this.registerInputButton(messageShort, "[Channel3]", "!switchDeck", 0x02, 0x04, this.deckSwitchHandler);
    this.registerInputButton(messageShort, "[Channel4]", "!switchDeck", 0x05, 0x08, this.deckSwitchHandler);

    var group = "[Channel3]";
    TraktorS3.registerInputButton(messageShort, group, "!fxEnabled", 0x07, 0x08,
        TraktorS3.bind(TraktorS3.Channel.prototype.fxEnableHandler, this.Channels[group]));
    group = "[Channel1]";
    TraktorS3.registerInputButton(messageShort, group, "!fxEnabled", 0x07, 0x10,
        TraktorS3.bind(TraktorS3.Channel.prototype.fxEnableHandler, this.Channels[group]));
    group = "[Channel2]";
    TraktorS3.registerInputButton(messageShort, group, "!fxEnabled", 0x07, 0x20,
        TraktorS3.bind(TraktorS3.Channel.prototype.fxEnableHandler, this.Channels[group]));
    group = "[Channel4]";
    TraktorS3.registerInputButton(messageShort, group, "!fxEnabled", 0x07, 0x40,
        TraktorS3.bind(TraktorS3.Channel.prototype.fxEnableHandler, this.Channels[group]));

    // Headphone buttons
    this.registerInputButton(messageShort, "[Channel1]", "pfl", 0x08, 0x01, this.buttonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "pfl", 0x08, 0x02, this.buttonHandler);
    this.registerInputButton(messageShort, "[Channel3]", "pfl", 0x07, 0x80, this.buttonHandler);
    this.registerInputButton(messageShort, "[Channel4]", "pfl", 0x08, 0x04, this.buttonHandler);

    // FX Buttons
    this.registerInputButton(messageShort, "[ChannelX]", "!fx1", 0x08, 0x08, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx2", 0x08, 0x10, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx3", 0x08, 0x20, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx4", 0x08, 0x40, this.fxHandler);
    this.registerInputButton(messageShort, "[ChannelX]", "!fx5", 0x08, 0x80, this.fxHandler);

    this.controller.registerInputPacket(messageShort);

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

    this.registerInputScaler(messageLong, "[Channel1]", "!super", 0x39, 0xFFFF, this.superHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "!super", 0x3B, 0xFFFF, this.superHandler);
    this.registerInputScaler(messageLong, "[Channel3]", "!super", 0x37, 0xFFFF, this.superHandler);
    this.registerInputScaler(messageLong, "[Channel4]", "!super", 0x3D, 0xFFFF, this.superHandler);

    this.registerInputScaler(messageLong, "[Master]", "crossfader", 0x0B, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "gain", 0x17, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headMix", 0x1D, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headGain", 0x1B, 0xFFFF, this.parameterHandler);

    this.controller.registerInputPacket(messageLong);

    // Soft takeovers
    for (var ch = 1; ch <= 4; ch++) {
        group = "[Channel" + ch + "]";
        if (!TraktorS3.pitchSliderRelativeMode) {
            engine.softTakeover(group, "rate", true);
        }
        engine.softTakeover(group, "pitch_adjust", true);
        // engine.softTakeover(group, "volume", true);
        // engine.softTakeover(group, "pregain", true);
        engine.softTakeover("[QuickEffectRack1_" + group + "]", "super1", true);
    }

    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]", "parameter3", true);

    // engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[Master]", "gain", true);
    engine.softTakeover("[Master]", "headMix", true);
    engine.softTakeover("[Master]", "headGain", true);

    for (var i = 1; i <= 16; ++i) {
        engine.softTakeover("[Sampler" + i + "]", "pregain", true);
    }

    for (ch in TraktorS3.Channels) {
        var chanob = TraktorS3.Channels[ch];
        engine.connectControl(ch, "playposition",
            TraktorS3.bind(TraktorS3.Channel.prototype.playpositionChanged, chanob));
        engine.connectControl(ch, "track_loaded",
            TraktorS3.bind(TraktorS3.Channel.prototype.trackLoadedHandler, chanob));
        engine.connectControl(ch, "end_of_track",
            TraktorS3.bind(TraktorS3.Channel.prototype.endOfTrackHandler, chanob));
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

TraktorS3.parameterHandler = function(field) {
    engine.setParameter(field.group, field.name, field.value / 4095);
};

TraktorS3.headphoneHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    script.toggleControl(field.group, "pfl");
};

TraktorS3.superHandler = function(field) {
    // The super knob drives all the supers!
    var group = field.group;
    var value = field.value / 4095.;
    engine.setParameter("[QuickEffectRack1_" + group + "]", "super1", value);
    for (var fxNumber = 1; fxNumber <= 4; fxNumber++) {
        if (TraktorS3.fxButtonState[fxNumber]) {
            engine.setParameter("[EffectRack1_EffectUnit" + fxNumber + "]", "super1", value);
        }
    }
};

TraktorS3.deckSwitchHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    var channel = TraktorS3.Channels[field.group];
    var deck = channel.parentDeck;
    deck.activateChannel(channel);
};

TraktorS3.fxHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    var fxNumber = parseInt(field.id[field.id.length - 1]);

    // Toggle effect unit
    TraktorS3.fxButtonState[fxNumber] = !TraktorS3.fxButtonState[fxNumber];
    var ledValue = TraktorS3.fxLEDValue[fxNumber];
    if (TraktorS3.fxButtonState[fxNumber]) {
        ledValue += TraktorS3.LEDBrightValue;
    } else {
        ledValue += TraktorS3.LEDDimValue;
    }
    TraktorS3.controller.setOutput("[ChannelX]", "!fxButton" + fxNumber, ledValue, !TraktorS3.batchingOutputs);
    TraktorS3.toggleFX();
};

TraktorS3.toggleFX = function() {
    // This is an AND operation.  We go through each channel, and if
    // the fitler button is ON and the fx is ON, we turn the effect ON.
    // We turn OFF if either is false.
    for (var fxNumber = 1; fxNumber <= 5; fxNumber++) {
        for (var ch = 1; ch <= 4; ch++) {
            var channel = TraktorS3.Channels["[Channel" + ch + "]"];
            var fxGroup = "[EffectRack1_EffectUnit" + fxNumber + "]";
            var fxKey = "group_[Channel" + ch + "]_enable";
            if (fxNumber === 5) {
                fxGroup = "[QuickEffectRack1_[Channel" + ch + "]_Effect1]";
                fxKey = "enabled";
            }

            var newState = channel.fxEnabledState && TraktorS3.fxButtonState[fxNumber];
            engine.setValue(fxGroup, fxKey, newState);
        }
    }
};

TraktorS3.registerOutputPackets = function() {
    var outputA = new HIDPacket("outputA", 0x80);
    var outputB = new HIDPacket("outputB", 0x81);

    for (var idx in TraktorS3.Decks) {
        var deck = TraktorS3.Decks[idx];
        deck.registerOutputs(outputA, outputB);
    }

    outputA.addOutput("[Channel1]", "!deck_A", 0x0A, "B");
    outputA.addOutput("[Channel2]", "!deck_B", 0x23, "B");
    outputA.addOutput("[Channel3]", "!deck_C", 0x0B, "B");
    outputA.addOutput("[Channel4]", "!deck_D", 0x24, "B");

    outputA.addOutput("[Channel1]", "pfl", 0x39, "B");
    outputA.addOutput("[Channel2]", "pfl", 0x3A, "B");
    outputA.addOutput("[Channel3]", "pfl", 0x38, "B");
    outputA.addOutput("[Channel4]", "pfl", 0x3B, "B");

    outputA.addOutput("[ChannelX]", "!fxButton1", 0x3C, "B");
    outputA.addOutput("[ChannelX]", "!fxButton2", 0x3D, "B");
    outputA.addOutput("[ChannelX]", "!fxButton3", 0x3E, "B");
    outputA.addOutput("[ChannelX]", "!fxButton4", 0x3F, "B");
    outputA.addOutput("[ChannelX]", "!fxButton5", 0x40, "B");

    outputA.addOutput("[Channel3]", "!fxEnabled", 0x34, "B");
    outputA.addOutput("[Channel1]", "!fxEnabled", 0x35, "B");
    outputA.addOutput("[Channel2]", "!fxEnabled", 0x36, "B");
    outputA.addOutput("[Channel4]", "!fxEnabled", 0x37, "B");

    this.controller.registerOutputPacket(outputA);

    var VuOffsets = {
        "[Channel3]": 0x01,
        "[Channel1]": 0x10,
        "[Channel2]": 0x1F,
        "[Channel4]": 0x2E
    };
    for (var ch in VuOffsets) {
        for (var i = 0; i < 14; i++) {
            outputB.addOutput(ch, "!" + "VuMeter" + i, VuOffsets[ch] + i, "B");
        }
    }

    var MasterVuOffsets = {
        "VuMeterL": 0x3D,
        "VuMeterR": 0x46
    };
    for (i = 0; i < 8; i++) {
        outputB.addOutput("[Master]", "!" + "VuMeterL" + i, MasterVuOffsets["VuMeterL"] + i, "B");
        outputB.addOutput("[Master]", "!" + "VuMeterR" + i, MasterVuOffsets["VuMeterR"] + i, "B");
    }

    outputB.addOutput("[Master]", "PeakIndicatorL", 0x45, "B");
    outputB.addOutput("[Master]", "PeakIndicatorR", 0x4E, "B");

    outputB.addOutput("[Channel3]", "PeakIndicator", 0x0F, "B");
    outputB.addOutput("[Channel1]", "PeakIndicator", 0x1E, "B");
    outputB.addOutput("[Channel2]", "PeakIndicator", 0x2D, "B");
    outputB.addOutput("[Channel4]", "PeakIndicator", 0x3C, "B");

    this.controller.registerOutputPacket(outputB);

    for (idx in TraktorS3.Decks) {
        deck = TraktorS3.Decks[idx];
        deck.linkOutputs();
    }

    for (idx in TraktorS3.Channels) {
        var chan = TraktorS3.Channels[idx];
        chan.linkOutputs();
    }

    // Master VuMeters
    this.masterVuConnections["VuMeterL"] = engine.makeConnection("[Master]", "VuMeterL", this.masterVuMeterHandler);
    this.masterVuConnections["VuMeterR"] = engine.makeConnection("[Master]", "VuMeterR", this.masterVuMeterHandler);
    this.linkChannelOutput("[Master]", "PeakIndicatorL", this.peakOutputHandler);
    this.linkChannelOutput("[Master]", "PeakIndicatorR", this.peakOutputHandler);

    // Sampler callbacks
    for (i = 1; i <= 16; ++i) {
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "track_loaded", this.samplesOutputHandler));
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "play", this.samplesOutputHandler));
    }
};

TraktorS3.linkChannelOutput = function(group, name, callback) {
    TraktorS3.controller.linkOutput(group, name, group, name, callback);
};

TraktorS3.wrapOutput = function(callback) {
    return function(value, _group, name) {
        callback(value, name);
    };
};

// outputHandler drives lights that only have one color.
TraktorS3.basicOutputHandler = function(value, group, key) {
    var ledValue = value;
    if (value === 0 || value === false) {
        // Off value
        ledValue = 0x04;
    } else if (value === 1 || value === true) {
        // On value
        ledValue = 0xFF;
    }

    TraktorS3.controller.setOutput(group, key, ledValue, !TraktorS3.batchingOutputs);
};

TraktorS3.peakOutputHandler = function(value, group, key) {
    var ledValue = 0x00;
    if (value) {
        ledValue = 0x7E;
    }

    TraktorS3.controller.setOutput(group, key, ledValue, !TraktorS3.batchingOutputs);
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

TraktorS3.samplesOutputHandler = function(value, group, key) {
    // Sampler 1-8 -> Channel1
    // Samples 9-16 -> Channel2
    var sampler = TraktorS3.resolveSampler(group);
    var deck = TraktorS3.Decks["deck1"];
    var num = sampler;
    if (sampler === undefined) {
        return;
    } else if (sampler > 8 && sampler < 17) {
        deck = TraktorS3.Decks["deck2"];
        num = sampler - 8;
    }

    // If we are in samples modes light corresponding LED
    if (this.padModeState === 1) {
        if (key === "play" && engine.getValue(group, "track_loaded")) {
            if (value) {
                // Green light on play
                deck.colorOutput(0x9E, "!pad_" + num);
            } else {
                // Reset LED to full white light
                deck.colorOutput(1, "!pad_" + num);
            }
        } else if (key === "track_loaded") {
            deck.colorOutput(value, "!pad_" + num);
        }
    }
};

TraktorS3.lightGroup = function(packet, outputGroupName, coGroupName) {
    var groupOb = packet.groups[outputGroupName];
    for (var fieldName in groupOb) {
        var field = groupOb[fieldName];
        if (field.name[0] === "!") {
            continue;
        }
        if (field.mapped_callback !== undefined) {
            var value = engine.getValue(coGroupName, field.name);
            field.mapped_callback(value, coGroupName, field.name);
        }
    // No callback, no light!
    }
};

TraktorS3.lightFX = function() {
    for (var ch in TraktorS3.Channels) {
        var chanob = TraktorS3.Channels[ch];
        chanob.colorOutput(chanob.fxEnabledState, "!fxEnabled");
    }
    for (var fxNumber = 1; fxNumber <= 5; fxNumber++) {
        var ledValue = TraktorS3.fxLEDValue[fxNumber];
        if (TraktorS3.fxButtonState[fxNumber]) {
            ledValue += TraktorS3.LEDBrightValue;
        } else {
            ledValue += TraktorS3.LEDDimValue;
        }
        TraktorS3.controller.setOutput("[ChannelX]", "!fxButton" + fxNumber, ledValue, !TraktorS3.batchingOutputs);
    }
};

TraktorS3.lightDeck = function(group, sendPackets) {
    if (sendPackets === undefined) {
        sendPackets = true;
    }
    // Freeze the lights while we do this update so we don't spam HID.
    TraktorS3.batchingOutputs = true;
    for (var packetName in this.controller.OutputPackets) {
        var packet = this.controller.OutputPackets[packetName];
        var deckGroupName = "deck1";
        if (group === "[Channel2]" || group === "[Channel4]") {
            deckGroupName = "deck2";
        }

        var deck = TraktorS3.Decks[deckGroupName];

        TraktorS3.lightGroup(packet, deckGroupName, group);
        TraktorS3.lightGroup(packet, group, group);

        deck.lightPads();

        // These lights are different because either they aren't associated with a CO, or
        // there are two buttons that point to the same CO.
        deck.basicOutput(0, "!shift");
        deck.colorOutput(0, "!PreviewTrack");
        deck.colorOutput(0, "!QueueAutoDJ");
        deck.colorOutput(0, "!LibraryFocus");
    }
    TraktorS3.lightFX();

    // Selected deck lights
    var ctrlr = TraktorS3.controller;
    if (group === "[Channel1]") {
        ctrlr.setOutput("[Channel1]", "!deck_A", ctrlr.LEDColors[TraktorS3.deckOutputColors["[Channel1]"]] + TraktorS3.LEDBrightValue, false);
        ctrlr.setOutput("[Channel3]", "!deck_C", ctrlr.LEDColors[TraktorS3.deckOutputColors["[Channel3]"]] + TraktorS3.LEDDimValue, false);
    } else if (group === "[Channel2]") {
        ctrlr.setOutput("[Channel2]", "!deck_B", ctrlr.LEDColors[TraktorS3.deckOutputColors["[Channel2]"]] + TraktorS3.LEDBrightValue, false);
        ctrlr.setOutput("[Channel4]", "!deck_D", ctrlr.LEDColors[TraktorS3.deckOutputColors["[Channel4]"]] + TraktorS3.LEDDimValue, false);
    } else if (group === "[Channel3]") {
        ctrlr.setOutput("[Channel3]", "!deck_C", ctrlr.LEDColors[TraktorS3.deckOutputColors["[Channel3]"]] + TraktorS3.LEDBrightValue, false);
        ctrlr.setOutput("[Channel1]", "!deck_A", ctrlr.LEDColors[TraktorS3.deckOutputColors["[Channel1]"]] + TraktorS3.LEDDimValue, false);
    } else if (group === "[Channel4]") {
        ctrlr.setOutput("[Channel4]", "!deck_D", ctrlr.LEDColors[TraktorS3.deckOutputColors["[Channel4]"]] + TraktorS3.LEDBrightValue, false);
        ctrlr.setOutput("[Channel2]", "!deck_B", ctrlr.LEDColors[TraktorS3.deckOutputColors["[Channel2]"]] + TraktorS3.LEDDimValue, false);
    }

    TraktorS3.batchingOutputs = false;
    // And now send them all.
    if (sendPackets) {
        for (packetName in this.controller.OutputPackets) {
            this.controller.OutputPackets[packetName].send();
        }
    }
};

TraktorS3.messageCallback = function(_packet, data) {
    for (var name in data) {
        if (Object.prototype.hasOwnProperty.call(data, name)) {
            TraktorS3.controller.processButton(data[name]);
        }
    }
};

TraktorS3.incomingData = function(data, length) {
    TraktorS3.controller.parsePacket(data, length);
};

TraktorS3.debugLights = function() {
    // Call this if you want to just send raw packets to the controller (good for figuring out what
    // bytes do what).
    var dataStrings = [
        "      7C 00  35 2C 2C FF  2C 39 FF 00  FF FF 00 35 " +
        "00 2C 7E 00  00 FF FF FF  2C 2C 20 7C  7C 00 FF FF " +
        "FF 00 00 00  FF FF FF 2C  00 FF 2C 7C  FF 00 00 00 " +
        "00 00 00 00  7E 0C FF FF  0C FF FF FF  FF FF FF FF " +
        "FF FF 40 FF  FF FF 00 FF  FF 2E FF 00  00 FF 00 00 " +
        "00 00 FF 00 ",
        "      00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  FF FF FF 00  FF 00 00 00  FF 00 00",
    ];

    var data = [Object(), Object()];


    for (var i = 0; i < data.length; i++) {
        var ok = true;
        var splitted = dataStrings[i].split(/\s+/);
        HIDDebug("i " + i + " " + splitted);
        data[i].length = splitted.length;
        for (var j = 0; j < splitted.length; j++) {
            var byteStr = splitted[j];
            if (byteStr.length === 0) {
                continue;
            }
            if (byteStr.length !== 2) {
                ok = false;
                HIDDebug("not two characters?? " + byteStr);
            }
            var b = parseInt(byteStr, 16);
            if (b < 0 || b > 255) {
                ok = false;
                HIDDebug("number out of range: " + byteStr + " " + b);
            }
            data[i][j] = b;
        }
        // if (i === 0) {
        //     for (k = 0; k < data[0].length; k++) {
        //         data[0][k] = 0x30;
        //     }
        // }
        // for (d = 0; d < 8; d++) {
        //     data[0][0x11+d] = (d+1) * 4 + 2;
        // }
        // for (d = 0; d < 8; d++) {
        //     data[0][0x2A + d] = (d + 1) * 4 + 32 + 2;
        // }
        if (ok) {
            controller.send(data[i], data[i].length, 0x80 + i);
        }
    }
};

TraktorS3.shutdown = function() {
    // Deactivate all LEDs
    var dataStrings = [
        "      00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00 ",
        "      00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00",
    ];

    var data = [Object(), Object()];


    for (var i = 0; i < data.length; i++) {
        var splitted = dataStrings[i].split(/\s+/);
        data[i].length = splitted.length;
        for (var j = 0; j < splitted.length; j++) {
            var byteStr = splitted[j];
            if (byteStr.length === 0) {
                continue;
            }
            data[i][j] = parseInt(byteStr, 16);
        }
        controller.send(data[i], data[i].length, 0x80 + i);
    }

    HIDDebug("TraktorS3: Shutdown done!");
};

TraktorS3.init = function(_id) {
    this.Decks = {
        "deck1": new TraktorS3.Deck(1, "deck1"),
        "deck2": new TraktorS3.Deck(2, "deck2"),
    };

    this.Channels = {
        "[Channel1]": new TraktorS3.Channel(this.Decks["deck1"], "[Channel1]"),
        "[Channel2]": new TraktorS3.Channel(this.Decks["deck2"], "[Channel2]"),
        "[Channel3]": new TraktorS3.Channel(this.Decks["deck1"], "[Channel3]"),
        "[Channel4]": new TraktorS3.Channel(this.Decks["deck2"], "[Channel4]")
    };

    TraktorS3.registerInputPackets();
    TraktorS3.registerOutputPackets();
    HIDDebug("TraktorS3: Init done!");

    TraktorS3.lightDeck("[Channel3]", false);
    TraktorS3.lightDeck("[Channel4]", false);
    TraktorS3.lightDeck("[Channel1]", false);
    TraktorS3.lightDeck("[Channel2]", true);

    // TraktorS3.debugLights();
};

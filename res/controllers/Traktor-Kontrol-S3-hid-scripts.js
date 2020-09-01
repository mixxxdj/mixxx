///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* Traktor Kontrol S3 HID controller script v1.00                                */
/* Last modification: August 2020                                                */
/* Author: Owen Williams                                                         */
/* https://www.mixxx.org/wiki/doku.php/native_instruments_traktor_kontrol_s3     */
/*                                                                               */
/* For linter:                                                                   */
/* global HIDController, HIDDebug, HIDPacket, controller                         */
///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* TODO:                                                                         */
/*   * jog button                                                                */
/*   * star button                                                               */
/*                                                                               */
///////////////////////////////////////////////////////////////////////////////////

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
var TraktorS3PitchSliderRelativeMode = true;

// You can choose the colors you want for each channel. The list of colors is:
// RED, CARROT, ORANGE, HONEY, YELLOW, LIME, GREEN, AQUA, CELESTE, SKY, BLUE,
// PURPLE, FUCHSIA, MAGENTA, AZALEA, SALMON, WHITE
// Some colors may look odd because of how they are encoded inside the controller.
var TraktorS3ChannelColors = {
    "[Channel1]": "CARROT",
    "[Channel2]": "CARROT",
    "[Channel3]": "BLUE",
    "[Channel4]": "BLUE"
};

// Each color has four brightnesses, so these values can be between 0 and 3.
var TraktorS3LEDDimValue = 0x00;
var TraktorS3LEDBrightValue = 0x02;

// Set to true to output debug messages and debug light outputs.
var TraktorS3DebugMode = false;


var TraktorS3 = new function() {
    this.controller = new HIDController();

    // When true, packets will not be sent to the controller.  Good for doing mass updates.
    this.batchingOutputs = false;

    // "5" is the "filter" button below the other 4.
    this.fxButtonState = {1: false, 2: false, 3: false, 4: false, 5: false};

    this.masterVuMeter = {
        "VuMeterL": {
            connection: null,
            updated: false,
            value: 0
        },
        "VuMeterR": {
            connection: null,
            updated: false,
            value: 0
        }
    };

    this.guiTickConnection = {};

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
        FUCHSIA: 0x34,
        MAGENTA: 0x38,
        AZALEA: 0x3C,
        SALMON: 0x40,
        WHITE: 0x44
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

        0xCC0091: this.controller.LEDColors.FUCHSIA,
        0xCC0079: this.controller.LEDColors.MAGENTA,
        0xCC477E: this.controller.LEDColors.AZALEA,
        0xCC4761: this.controller.LEDColors.SALMON,

        0xCCCCCC: this.controller.LEDColors.WHITE,
    });

    // State for controller input loudness setting
    this.inputModeLine = false;

    // If true, channel 4 is in input mode
    this.channel4InputMode = false;

    this.inputFxEnabledState = false;

    // callbacks
    this.samplerCallbacks = [];
};

// Mixxx's javascript doesn't support .bind natively, so here's a simple version.
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
    // padModeState 0 is hotcues, 1 is samplers
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
    if (this.deckNumber === 2) {
        deckOffset = deck2Offset;
    }
    // Jog wheels have four byte input: 1 byte for distance ticks, and 3 bytes for a timecode.
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

    // Rev / Flux / Grid / Jog
    this.defineButton(messageShort, "!reverse", 0x01, 0x04, 0x04, 0x08, deckFn.reverseHandler);
    this.defineButton(messageShort, "!slip_enabled", 0x01, 0x02, 0x04, 0x04, deckFn.fluxHandler);
    // Grid button
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

TraktorS3.Deck.prototype.shiftHandler = function(field) {
    // Mixxx only knows about one shift value, but this controller has two shift buttons.
    // This control object could get confused if both physical buttons are pushed at the same
    // time.
    engine.setValue("[Controls]", "touch_shift", field.value);
    this.shiftPressed = field.value;
    if (field.value) {
        engine.softTakeoverIgnoreNextValue("[Master]", "gain");
    }
    TraktorS3.basicOutput(field.value, field.group, "!shift");
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
    } else if (this.syncPressedTimer !== 0) {
        // Timer still running -> stop it and unlight LED
        engine.stopTimer(this.syncPressedTimer);
        this.colorOutput(0, "sync_enabled");
    }
};

TraktorS3.Deck.prototype.keylockHandler = function(field) {
    // shift + keylock resets pitch (in either mode).
    if (this.shiftPressed) {
        if (field.value) {
            engine.setValue(this.activeChannel, "pitch_adjust_set_default", 1);
        }
    } else if (TraktorS3PitchSliderRelativeMode) {
        if (field.value) {
            // In relative mode on down-press, reset the values and note that
            // the button is pressed.
            this.keylockPressed = true;
            this.keyAdjusted = false;
        } else {
            // On release, note that the button is released, and if the key *wasn't* adjusted,
            // activate keylock.
            this.keylockPressed = false;
            if (!this.keyAdjusted) {
                script.toggleControl(this.activeChannel, "keylock");
            }
        }
    } else if (field.value) {
        // In absolute mode, do a simple toggle on down-press.
        script.toggleControl(this.activeChannel, "keylock");
    }

    // Adjust the light on release depending on keylock status.  Down-press is always lit.
    if (!field.value) {
        var val = engine.getValue(this.activeChannel, "keylock");
        this.colorOutput(val, "keylock");
    } else {
        this.colorOutput(1, "keylock");
    }
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
    var padNumber = parseInt(field.name[field.name.length - 1]);
    var action = "";

    // Hotcues mode
    if (this.padModeState === 0) {
        if (this.shiftPressed) {
            action = "_clear";
        } else {
            action = "_activate";
        }
        engine.setValue(this.activeChannel, "hotcue_" + padNumber + action, field.value);
        return;
    }

    // Samples mode
    var sampler = padNumber;
    if (field.group === "deck2") {
        sampler += 8;
    }

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
    // this.basicOutput(field.value, "reverse");
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "reverse", field.value);
    } else {
        engine.setValue(this.activeChannel, "reverseroll", field.value);
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
    if (TraktorS3PitchSliderRelativeMode) {
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

    var colorOutput = function(value, _group, key) {
        this.colorOutput(value, key);
    };

    var basicOutput = function(value, _group, key) {
        this.basicOutput(value, key);
    };

    this.defineLink("play_indicator", TraktorS3.bind(deckFn.playIndicatorHandler, this));
    this.defineLink("cue_indicator", TraktorS3.bind(colorOutput, this));
    this.defineLink("sync_enabled", TraktorS3.bind(colorOutput, this));
    this.defineLink("keylock", TraktorS3.bind(colorOutput, this));
    this.defineLink("slip_enabled", TraktorS3.bind(colorOutput, this));
    this.defineLink("quantize", TraktorS3.bind(colorOutput, this));
    this.defineLink("reverse", TraktorS3.bind(basicOutput, this));
};

TraktorS3.Deck.prototype.deckBaseColor = function() {
    return TraktorS3.controller.LEDColors[TraktorS3ChannelColors[this.activeChannel]];
};

// basicOutput drives lights that only have one color.
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

// colorOutput drives lights that have the palettized multicolor lights.
TraktorS3.Deck.prototype.colorOutput = function(value, key) {
    var ledValue = this.deckBaseColor();

    if (value === 1 || value === true) {
        ledValue += TraktorS3LEDBrightValue;
    } else {
        ledValue += TraktorS3LEDDimValue;
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
        ledValue += TraktorS3LEDDimValue;
    }
    if (active) {
        ledValue += TraktorS3LEDBrightValue;
    } else {
        ledValue += TraktorS3LEDDimValue;
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
        ledValue += TraktorS3LEDBrightValue;
    } else {
        ledValue = 0x00;
    }
    this.wheelOutput(group,
        [ledValue, ledValue, ledValue, ledValue, ledValue, ledValue, ledValue, ledValue]);
};

TraktorS3.Deck.prototype.wheelOutput = function(group, valueArray) {
    if (group !== this.activeChannel) {
        return;
    }

    for (var i = 0; i < 8; i++) {
        TraktorS3.controller.setOutput(this.group, "!wheel" + i, valueArray[i], false);
    }
    if (!TraktorS3.batchingOutputs) {
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
    this.fxEnabledState = false;

    this.trackDurationSec = 0;
    this.positionUpdated = false;
    this.curPosition = -1;
    this.endOfTrackTimer = 0;
    this.endOfTrack = false;
    this.endOfTrackBlinkState = 0;
    this.vuMeterUpdated = false;
    this.vuMeterValue = 0;

    this.vuConnection = {};
    this.clipConnection = {};
    this.hotcueCallbacks = [];
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
    this.curPosition = value * this.trackDurationSec;
    this.positionUpdated = true;
};

TraktorS3.Channel.prototype.vuMeterHandler = function(value) {
    this.vuMeterUpdated = true;
    this.vuMeterValue = value;
};

TraktorS3.Channel.prototype.linkOutputs = function() {
    this.vuConnection = engine.makeConnection(this.group, "VuMeter", TraktorS3.bind(TraktorS3.Channel.prototype.vuMeterHandler, this));
    this.clipConnection = engine.makeConnection(this.group, "PeakIndicator", TraktorS3.peakOutput);
    TraktorS3.linkChannelOutput(this.group, "pfl", TraktorS3.pflOutput);
    for (var j = 1; j <= 8; j++) {
        this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_enabled",
            TraktorS3.bind(TraktorS3.Channel.prototype.hotcuesOutput, this)));
        this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_activate",
            TraktorS3.bind(TraktorS3.Channel.prototype.hotcuesOutput, this)));
    }
};

TraktorS3.Channel.prototype.channelBaseColor = function() {
    if (this.group === "[Channel4]" && TraktorS3.channel4InputMode) {
        return TraktorS3.controller.LEDColors[TraktorS3.controller.LEDColors.OFF];
    }
    return TraktorS3.controller.LEDColors[TraktorS3ChannelColors[this.group]];
};

// colorOutput drives lights that have the palettized multicolor lights.
TraktorS3.Channel.prototype.colorOutput = function(value, key) {
    var ledValue = this.channelBaseColor();
    if (value === 1 || value === true) {
        ledValue += TraktorS3LEDBrightValue;
    } else {
        ledValue += TraktorS3LEDDimValue;
    }
    TraktorS3.controller.setOutput(this.group, key, ledValue, !TraktorS3.batchingOutputs);
};

TraktorS3.Channel.prototype.hotcuesOutput = function(_value, group, key) {
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

// Returns true if there was an update.
TraktorS3.Channel.prototype.lightWheelPosition = function() {
    if (!this.positionUpdated) {
        return false;
    }
    this.positionUpdated = false;
    var rotations = this.curPosition * (1 / 1.8);  // 1/1.8 is rotations per second
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
    this.parentDeck.wheelOutput(this.group, segValues);
    return true;
};

// FXControl is an object that manages the gray area in the middle of the
// controller: the fx control knobs, fxenable buttons, and fx select buttons.
TraktorS3.FXControl = function() {
    // 0 is filter, 1-4 are FX Units 1-4
    this.FILTER_EFFECT = 0;
    this.activeFX = this.FILTER_EFFECT;

    this.enablePressed = {
        "[Channel1]": false,
        "[Channel2]": false,
        "[Channel3]": false,
        "[Channel4]": false
    };
    this.selectPressed = {
        0: false,
        1: false,
        2: false,
        3: false,
        4: false
    };

    // States
    this.STATE_FILTER = 0;
    this.STATE_EFFECT = 1;
    this.STATE_FOCUS = 2;

    this.currentState = this.STATE_FILTER;
};

TraktorS3.FXControl.prototype.registerInputs = function(messageShort, messageLong) {
    // FX Buttons
    var fxFn = TraktorS3.FXControl.prototype;
    TraktorS3.registerInputButton(messageShort, "[ChannelX]", "!fx1", 0x08, 0x08, TraktorS3.bind(fxFn.fxSelectHandler, this));
    TraktorS3.registerInputButton(messageShort, "[ChannelX]", "!fx2", 0x08, 0x10, TraktorS3.bind(fxFn.fxSelectHandler, this));
    TraktorS3.registerInputButton(messageShort, "[ChannelX]", "!fx3", 0x08, 0x20, TraktorS3.bind(fxFn.fxSelectHandler, this));
    TraktorS3.registerInputButton(messageShort, "[ChannelX]", "!fx4", 0x08, 0x40, TraktorS3.bind(fxFn.fxSelectHandler, this));
    TraktorS3.registerInputButton(messageShort, "[ChannelX]", "!fx0", 0x08, 0x80, TraktorS3.bind(fxFn.fxSelectHandler, this));

    TraktorS3.registerInputButton(messageShort, "[Channel3]", "!fxEnabled", 0x07, 0x08, TraktorS3.bind(fxFn.fxEnableHandler, this));
    TraktorS3.registerInputButton(messageShort, "[Channel1]", "!fxEnabled", 0x07, 0x10, TraktorS3.bind(fxFn.fxEnableHandler, this));
    TraktorS3.registerInputButton(messageShort, "[Channel2]", "!fxEnabled", 0x07, 0x20, TraktorS3.bind(fxFn.fxEnableHandler, this));
    TraktorS3.registerInputButton(messageShort, "[Channel4]", "!fxEnabled", 0x07, 0x40, TraktorS3.bind(fxFn.fxEnableHandler, this));

    TraktorS3.registerInputScaler(messageLong, "[Channel1]", "!fxKnob", 0x39, 0xFFFF, TraktorS3.bind(fxFn.fxKnobHandler, this));
    TraktorS3.registerInputScaler(messageLong, "[Channel2]", "!fxKnob", 0x3B, 0xFFFF, TraktorS3.bind(fxFn.fxKnobHandler, this));
    TraktorS3.registerInputScaler(messageLong, "[Channel3]", "!fxKnob", 0x37, 0xFFFF, TraktorS3.bind(fxFn.fxKnobHandler, this));
    TraktorS3.registerInputScaler(messageLong, "[Channel4]", "!fxKnob", 0x3D, 0xFFFF, TraktorS3.bind(fxFn.fxKnobHandler, this));
};

TraktorS3.FXControl.prototype.channelToIndex = function(group) {
    var result = group.match(script.channelRegEx);
    if (result === null) {
        HIDDebug("barf" + group);
        return undefined;
    }
    // Unmap from channel number to button index.
    switch (result[1]) {
    case "1":
        return 2;
    case "2":
        return 3;
    case "3":
        return 1;
    case "4":
        return 4;
    }
    return undefined;
};

TraktorS3.FXControl.prototype.StatusDebug = function() {
    HIDDebug("active: " + this.activeFX +
        " enablepressed? " + this.enablePressed +
        " selectpressed? " + this.selectPressed);
    for (var i = 1; i <= 4; i++) {
        var focus = engine.getValue("[EffectRack1_EffectUnit" + i + "]", "focused_effect");
        if (focus) {
            HIDDebug("FX" + i + " focus");
        }
    }
};

// TraktorS3.FXControl.prototype.toggleFocusEnable = function(fxNum) {
//     var group = "[EffectRack1_EffectUnit" + fxNum + "]";
//     var newState = !engine.getValue(group, "focused_effect");

//     if (!newState) {
//         engine.setValue(group, "focused_effect", 0);
//         return;
//     }
//     // If we are setting a different unit to be enabled, the others become
//     // disabled.
//     for (var i = 1; i <= 4; i++) {
//         group = "[EffectRack1_EffectUnit" + i + "]";
//         engine.setValue(group, "focused_effect", i === fxNum);
//     }
// };

TraktorS3.FXControl.prototype.firstPressedSelect = function() {
    for (var idx in this.selectPressed) {
        if (this.selectPressed[idx]) {
            return idx;
        }
    }
    return undefined;
};

TraktorS3.FXControl.prototype.anyEnablePressed = function() {
    for (var key in this.enablePressed) {
        if (this.enablePressed[key]) {
            return true;
        }
    }
    return false;
};

// pressing FX SELECT buttons changes the activeFX
// press again to focus, press again to unfocus.  (focus should blink)
TraktorS3.FXControl.prototype.fxSelectHandler = function(field) {
    var fxNumber = parseInt(field.name[field.name.length - 1]);
    this.selectPressed[fxNumber] = field.value;

    if (!field.value) {
        // do lights?
        return;
    }

    switch (this.currentState) {
    case this.STATE_FILTER:
        // If any fxEnable button is pressed, we are toggling fx unit assignment.
        if (this.anyEnablePressed()) {
            var fxGroup = "[EffectRack1_EffectUnit" + fxNumber + "]";
            for (var key in this.enablePressed) {
                if (this.enablePressed[key]) {
                    HIDDebug("key? " + key);
                    var fxKey = "group_" + key + "_enable";
                    script.toggleControl(fxGroup, fxKey);
                }
            }
            //     this.StatusDebug();
            return;
        }

        // If we push filter and filter is already pushed, do nothing.
        if (fxNumber === 0) {
            this.currentState = this.STATE_FILTER;
        } else {
            // Select this filter instead.
            // initiate state change
            // this.activateState(this.STATE_EFFECT, fxNumber);
            this.currentState = this.STATE_EFFECT;
        }
        this.activeFX = fxNumber;
        break;
    case this.STATE_EFFECT:
        if (fxNumber === 0) {
            this.currentState = this.STATE_FILTER;
        } else {
            // if (fxNumber === this.activeFX) {
            //     this.currentState = this.STATE_FOCUS;
            // } else {
            //     // Select this filter instead.
            //     // initiate state change
            //     // this.activateState(this.STATE_EFFECT, fxNumber);
            this.currentState = this.STATE_EFFECT;
            // }
        }
        this.activeFX = fxNumber;
        break;
    case this.STATE_FOCUS:
        if (fxNumber === 0) {
            this.currentState = this.STATE_FILTER;
        } else {
            this.currentState = this.STATE_EFFECT;
        }
        this.activeFX = fxNumber;
        break;
    }
    // var fxGroup = "[EffectRack1_EffectUnit" + fxNumber + "]";
};

// in unfocus mode, tap.... does nothing?  Just highlights which FX are enabled for that deck while
// held.
TraktorS3.FXControl.prototype.fxEnableHandler = function(field) {
    HIDDebug("we are here: " + field.group + " " + field.value);
    this.enablePressed[field.group] = field.value;

    if (!field.value) {
        // do lights?
        return;
    }

    HIDDebug("eh?-------------");
    var fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
    var buttonNumber = this.channelToIndex(field.group);
    switch (this.currentState) {
    case this.STATE_FILTER:
        HIDDebug("filter");
        break;
    case this.STATE_EFFECT:
        HIDDebug("effect mode");
        if (this.firstPressedSelect()) {
            HIDDebug("change to focus");
            // Choose the first pressed select button only.
            this.currentState = this.STATE_FOCUS;
            HIDDebug("focusing " + fxGroupPrefix + " " + buttonNumber);
            // var focusedEffect = engine.getValue(effectUnitGroup, "focused_effect");
            engine.setValue(fxGroupPrefix + "]", "focused_effect", buttonNumber);
        } else {
            HIDDebug("effect togg? " + field.group);
            var group = fxGroupPrefix + "_Effect" + buttonNumber + "]";
            var key = "enabled";
            HIDDebug("toggling " + group + " " + key);
            script.toggleControl(group, key);
        }
        break;
    case this.STATE_FOCUS:
        HIDDebug("focus mode");
        var focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
        fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
        group = fxGroupPrefix + "_Effect" + focusedEffect + "]";
        key = "parameter" + buttonNumber;
        HIDDebug("toggling " + group + " " + key);
        script.toggleControl(group, key);
        break;
    }

    // HIDDebug("FX ENABLE " + field.group + " " + field.name + " " + field.value);
    // if (field.value === 0) {
    //     if (this.enablePressed === field.group) {
    //         this.enablePressed = "";
    //     }
    //     this.StatusDebug();
    //     return;
    // }

    // // in unfocus mode, preess and hold + tap fxselect to enable/disable per channel
    // // if select was pressed first though, ignore pressing enable as a press-and-hold.
    // if (this.selectPressed === -1) {
    //     HIDDebug("TOGGLE");
    //     this.enablePressed = field.group;
    // }

    // // in focus mode, tap fxenable enables/disables individual fx in units.
    // // var fxGroupPrefix = TraktorS3.fxGroupPrefix(this.);
    // // if (fxGroupPrefix === undefined) {
    // //     HIDDebug("Programming Error: Didn't match channel number in fxSelectHandler: " + field.group);
    // //     return;
    // // }
    // var fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
    // var focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
    // if (focusedEffect > 0) {
    //     HIDDebug("toggle subeffect!");
    //     var buttonNumber = this.channelToIndex(field.group);
    //     if (buttonNumber === undefined) {
    //         HIDDebug("Programming Error: unexpectedly couldn't parse group: " + field.group);
    //         return;
    //     }

    //     var group = fxGroupPrefix + "_Effect" + buttonNumber + "]";
    //     var key = "enabled";
    //     HIDDebug("flipping " + group + " " + key);
    //     script.toggleControl(group, key);
    // }
    // this.StatusDebug();
};

TraktorS3.fxGroupPrefix = function(group) {
    var channelMatch = group.match(script.channelRegEx);
    if (channelMatch === undefined) {
        return undefined;
    }
    return "[EffectRack1_EffectUnit" + channelMatch[1];
};

TraktorS3.FXControl.prototype.fxKnobHandler = function(field) {
    HIDDebug("FX KNOB " + field.group + " " + field.name + " " + field.value);
    var value = field.value / 4095.;
    var fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
    var knobIdx = this.channelToIndex(field.group);

    switch (this.currentState) {
    case this.STATE_FILTER:
        HIDDebug("filter");
        if (field.group === "[Channel4]" && TraktorS3.channel4InputMode) {
            // There is no quickeffect for the microphone, do nothing.
            // this.StatusDebug();
            return;
        }
        HIDDebug("HERE?2 " + "[QuickEffectRack1_" + field.group + "]");
        engine.setParameter("[QuickEffectRack1_" + field.group + "]", "super1", value);
        break;
    case this.STATE_EFFECT:
        // if (TraktorS3.anyShiftPressed()) {
        //     HIDDebug("load preset");
        //     engine.setValue(effectUnitGroup, "load_preset", buttonNumber+1);
        // } else {
        HIDDebug("effect");
        if (knobIdx === 4) {
            engine.setParameter(fxGroupPrefix + "]", "mix", value);
        } else {
            HIDDebug("set " + group + " meta");
            var group = fxGroupPrefix + "_Effect" + knobIdx + "]";
            engine.setParameter(group, "meta", value);
        }
        // }
        break;
    case this.STATE_FOCUS:
        var focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
        HIDDebug("focus " + focusedEffect);
        group = fxGroupPrefix + "_Effect" + focusedEffect + "]";
        var key = "parameter" + knobIdx;
        HIDDebug("set " + group + " " + key);
        engine.setParameter(group, key, value);
        break;
    }

    // var fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
    // HIDDebug("asking for: " + fxGroupPrefix + "]");
    // var focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
    // if (focusedEffect > 0) {
    //     // focus: shift + adjust selects effect
    //     // if (TraktorS3.anyShiftPressed()) {
    //     //     // engine.setValue(field.group, "load_preset", knobIdx+1);
    //     // }
    //     HIDDebug("focused?? ");

    //     // focus: adjusts params for that effect
    //     HIDDebug("PARAM: " + fxGroupPrefix + "_Effect" + focusedEffect + "]");
    //     engine.setParameter(fxGroupPrefix + "_Effect" + focusedEffect + "]",
    //         "parameter" + knobIdx,
    //         field.value / 4096);
    //     this.StatusDebug();
    //     return;
    // }

    // // unfocus: other fx selected: adjust meta knob per channel
    // //XXXXX there's only meta knob per effect, not channel!
    // HIDDebug("HERE?");
    // engine.setParameter(fxGroupPrefix + "]",
    //     "super1",
    //     field.value / 4096);
    // this.StatusDebug();
};

// FX LIGHTS:
// if a button is pressed, definitely it should be on
// if enable is pressed,
//   light the selects with what that channel has enabled.
// else if enable is not pressed,
//   if focused, select blinks for selected effect
//   if unfocused, selects is solid for selected effect
// if unfocused, enables are lit depending on which channels have that effect enabled. (for filter,
//   that's all)
// if focused, enables are lit depending on which units are active.



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

    // Headphone buttons
    this.registerInputButton(messageShort, "[Channel1]", "pfl", 0x08, 0x01, this.headphoneHandler);
    this.registerInputButton(messageShort, "[Channel2]", "pfl", 0x08, 0x02, this.headphoneHandler);
    this.registerInputButton(messageShort, "[Channel3]", "pfl", 0x07, 0x80, this.headphoneHandler);
    this.registerInputButton(messageShort, "[Channel4]", "pfl", 0x08, 0x04, this.headphoneHandler);

    // EXT Button
    this.registerInputButton(messageShort, "[Master]", "!extButton", 0x07, 0x04, this.extModeHandler);

    this.fxController.registerInputs(messageShort, messageLong);

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

    this.registerInputScaler(messageLong, "[Master]", "crossfader", 0x0B, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "gain", 0x17, 0xFFFF, this.masterGainHandler);
    this.registerInputScaler(messageLong, "[Master]", "headMix", 0x1D, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headGain", 0x1B, 0xFFFF, this.parameterHandler);

    this.controller.registerInputPacket(messageLong);

    // Soft takeovers
    for (var ch = 1; ch <= 4; ch++) {
        var group = "[Channel" + ch + "]";
        if (!TraktorS3PitchSliderRelativeMode) {
            engine.softTakeover(group, "rate", true);
        }
        engine.softTakeover(group, "pitch_adjust", true);
        engine.softTakeover(group, "volume", true);
        engine.softTakeover(group, "pregain", true);
        engine.softTakeover("[QuickEffectRack1_" + group + "]", "super1", true);
    }

    engine.softTakeover("[Microphone]", "volume", true);
    engine.softTakeover("[Microphone]", "pregain", true);

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
    // engine.softTakeover("[Master]", "headMix", true);
    // engine.softTakeover("[Master]", "headGain", true);

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
    if (field.group === "[Channel4]" && TraktorS3.channel4InputMode) {
        engine.setParameter("[Microphone]", field.name, field.value / 4095);
    } else {
        engine.setParameter(field.group, field.name, field.value / 4095);
    }
};

TraktorS3.anyShiftPressed = function() {
    return TraktorS3.Decks["deck1"].shiftPressed || TraktorS3.Decks["deck2"].shiftPressed;
};

TraktorS3.masterGainHandler = function(field) {
    // Only adjust if shift is held.  This will still adjust the sound card
    // volume but it at least allows for control of Mixxx's master gain.
    if (TraktorS3.anyShiftPressed()) {
        engine.setParameter(field.group, field.name, field.value / 4095);
    }
};

TraktorS3.headphoneHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    if (field.group === "[Channel4]" && TraktorS3.channel4InputMode) {
        script.toggleControl("[Microphone]", "pfl");
    } else {
        script.toggleControl(field.group, "pfl");
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

TraktorS3.toggleFX = function() {
    // This is an AND operation.  We go through each channel, and if
    // the filter button is ON and the fx is ON, we turn the effect ON.
    // We turn OFF if either is false.

    // The only exception is the Filter effect.  If the channel fxenable
    // is off, the Filter effect is still automatically enabled.
    // If the fxenable button is on, the Filter effect is only enabled if
    // the Filter FX button is enabled.
    for (var ch = 1; ch <= 4; ch++) {
        var channel = TraktorS3.Channels["[Channel" + ch + "]"];
        var chEnabled = channel.fxEnabledState;
        if (ch === 4 && TraktorS3.channel4InputMode) {
            chEnabled = TraktorS3.inputFxEnabledState;
        } else {
            // There is no quickeffect for the microphone
            var newState = !chEnabled || TraktorS3.fxButtonState[5];
            engine.setValue("[QuickEffectRack1_[Channel" + ch + "]]", "enabled",
                newState);
        }
        for (var fxNumber = 1; fxNumber <= 4; fxNumber++) {
            var fxGroup = "[EffectRack1_EffectUnit" + fxNumber + "]";
            var fxKey = "group_[Channel" + ch + "]_enable";
            newState = chEnabled && TraktorS3.fxButtonState[fxNumber];
            if (ch === 4 && TraktorS3.channel4InputMode) {
                fxKey = "group_[Microphone]_enable";
            }
            engine.setValue(fxGroup, fxKey, newState);
        }
    }
};

TraktorS3.extModeHandler = function(field) {
    if (!field.value) {
        TraktorS3.basicOutput(TraktorS3.channel4InputMode, field.group, field.name);
        return;
    }
    if (TraktorS3.anyShiftPressed()) {
        TraktorS3.basicOutput(field.value, field.group, field.name);
        TraktorS3.inputModeLine = !TraktorS3.inputModeLine;
        TraktorS3.setInputLineMode(TraktorS3.inputModeLine);
        return;
    }
    TraktorS3.channel4InputMode = !TraktorS3.channel4InputMode;
    if (TraktorS3.channel4InputMode) {
        engine.softTakeoverIgnoreNextValue("[Microphone]", "volume");
        engine.softTakeoverIgnoreNextValue("[Microphone]", "pregain");
    } else {
        engine.softTakeoverIgnoreNextValue("[Channel4]", "volume");
        engine.softTakeoverIgnoreNextValue("[Channel4]", "pregain");
    }
    TraktorS3.lightDeck("[Channel4]");
    TraktorS3.basicOutput(TraktorS3.channel4InputMode, field.group, field.name);
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

    outputA.addOutput("[Master]", "!extButton", 0x33, "B");

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

    engine.connectControl("[Microphone]", "pfl", this.pflOutput);

    // Master VuMeters
    this.masterVuMeter["VuMeterL"].connection = engine.makeConnection("[Master]", "VuMeterL", this.masterVuMeterHandler);
    this.masterVuMeter["VuMeterR"].connection = engine.makeConnection("[Master]", "VuMeterR", this.masterVuMeterHandler);
    this.linkChannelOutput("[Master]", "PeakIndicatorL", this.peakOutput);
    this.linkChannelOutput("[Master]", "PeakIndicatorR", this.peakOutput);
    this.guiTickConnection = engine.makeConnection("[Master]", "guiTick50ms", this.guiTickHandler);

    // Sampler callbacks
    for (i = 1; i <= 16; ++i) {
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "track_loaded", this.samplesOutput));
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "play", this.samplesOutput));
    }
};

TraktorS3.linkChannelOutput = function(group, name, callback) {
    TraktorS3.controller.linkOutput(group, name, group, name, callback);
};

TraktorS3.pflOutput = function(value, group, key) {
    if (group === "[Microphone]" && TraktorS3.channel4InputMode) {
        TraktorS3.basicOutput(value, "[Channel4]", key);
        return;
    }
    if (group === "[Channel4]" && !TraktorS3.channel4InputMode) {
        TraktorS3.basicOutput(value, group, key);
        return;
    }
    if (group.match(/^\[Channel[123]\]$/)) {
        TraktorS3.basicOutput(value, group, key);
    }
    // Unhandled case, ignore.
};

// Output drives lights that only have one color.
TraktorS3.basicOutput = function(value, group, key) {
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

TraktorS3.peakOutput = function(value, group, key) {
    var ledValue = 0x00;
    if (value) {
        ledValue = 0x7E;
    }

    TraktorS3.controller.setOutput(group, key, ledValue, !TraktorS3.batchingOutputs);
};

TraktorS3.masterVuMeterHandler = function(value, _group, key) {
    TraktorS3.masterVuMeter[key].updated = true;
    TraktorS3.masterVuMeter[key].value = value;
};

TraktorS3.vuMeterOutput = function(value, group, key, segments) {
    // This handler is called a lot so it should be as fast as possible.
    var scaledValue = value * segments;
    var fullIllumCount = Math.floor(scaledValue);

    // Figure out how much the partially-illuminated segment is illuminated.
    var partialIllum = (scaledValue - fullIllumCount) * 0x7F;

    for (var i = 0; i < segments; i++) {
        var segmentKey = "!" + key + i;
        if (i < fullIllumCount) {
            // Don't update lights until they're all done, so the last term is false.
            TraktorS3.controller.setOutput(group, segmentKey, 0x7F, false);
        } else if (i === fullIllumCount) {
            TraktorS3.controller.setOutput(group, segmentKey, partialIllum, false);
        } else {
            TraktorS3.controller.setOutput(group, segmentKey, 0x00, false);
        }
    }
    if (!TraktorS3.batchingOutputs) {
        TraktorS3.controller.OutputPackets["outputB"].send();
    }
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

TraktorS3.samplesOutput = function(value, group, key) {
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
        if (ch === "[Channel4]" && TraktorS3.channel4InputMode) {
            chanob.colorOutput(TraktorS3.inputFxEnabledState, "!fxEnabled");
        } else {
            chanob.colorOutput(chanob.fxEnabledState, "!fxEnabled");
        }
    }
    for (var fxNumber = 1; fxNumber <= 5; fxNumber++) {
        var ledValue = TraktorS3.fxLEDValue[fxNumber];
        if (TraktorS3.fxButtonState[fxNumber]) {
            ledValue += TraktorS3LEDBrightValue;
        } else {
            ledValue += TraktorS3LEDDimValue;
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
        if (group === "[Channel4]") {
            TraktorS3.basicOutput(0, "[Master]", "!extButton");
        }
    }
    TraktorS3.lightFX();

    // Selected deck lights
    var ctrlr = TraktorS3.controller;
    if (group === "[Channel1]") {
        ctrlr.setOutput("[Channel1]", "!deck_A", ctrlr.LEDColors[TraktorS3ChannelColors["[Channel1]"]] + TraktorS3LEDBrightValue, false);
        ctrlr.setOutput("[Channel3]", "!deck_C", ctrlr.LEDColors[TraktorS3ChannelColors["[Channel3]"]] + TraktorS3LEDDimValue, false);
    } else if (group === "[Channel2]") {
        ctrlr.setOutput("[Channel2]", "!deck_B", ctrlr.LEDColors[TraktorS3ChannelColors["[Channel2]"]] + TraktorS3LEDBrightValue, false);
        ctrlr.setOutput("[Channel4]", "!deck_D", ctrlr.LEDColors[TraktorS3ChannelColors["[Channel4]"]] + TraktorS3LEDDimValue, false);
    } else if (group === "[Channel3]") {
        ctrlr.setOutput("[Channel3]", "!deck_C", ctrlr.LEDColors[TraktorS3ChannelColors["[Channel3]"]] + TraktorS3LEDBrightValue, false);
        ctrlr.setOutput("[Channel1]", "!deck_A", ctrlr.LEDColors[TraktorS3ChannelColors["[Channel1]"]] + TraktorS3LEDDimValue, false);
    } else if (group === "[Channel4]") {
        ctrlr.setOutput("[Channel4]", "!deck_D", ctrlr.LEDColors[TraktorS3ChannelColors["[Channel4]"]] + TraktorS3LEDBrightValue, false);
        ctrlr.setOutput("[Channel2]", "!deck_B", ctrlr.LEDColors[TraktorS3ChannelColors["[Channel2]"]] + TraktorS3LEDDimValue, false);
    }

    TraktorS3.batchingOutputs = false;
    // And now send them all.
    if (sendPackets) {
        for (packetName in this.controller.OutputPackets) {
            this.controller.OutputPackets[packetName].send();
        }
    }
};

// Render wheel positions, channel VU meters, and master vu meters
TraktorS3.guiTickHandler = function() {
    TraktorS3.batchingOutputs = true;
    var gotUpdate = false;
    gotUpdate |= TraktorS3.Channels[TraktorS3.Decks["deck1"].activeChannel].lightWheelPosition();
    gotUpdate |= TraktorS3.Channels[TraktorS3.Decks["deck2"].activeChannel].lightWheelPosition();

    for (var vu in TraktorS3.masterVuMeter) {
        if (TraktorS3.masterVuMeter[vu].updated) {
            TraktorS3.vuMeterOutput(TraktorS3.masterVuMeter[vu].value, "[Master]", vu, 8);
            TraktorS3.masterVuMeter[vu].updated = false;
            gotUpdate = true;
        }
    }
    for (var ch = 1; ch <= 4; ch++) {
        var chan = TraktorS3.Channels["[Channel" + ch + "]"];
        if (chan.vuMeterUpdated) {
            TraktorS3.vuMeterOutput(chan.vuMeterValue, chan.group, "VuMeter", 14);
            chan.vuMeterUpdated = false;
            gotUpdate = true;
        }
    }

    TraktorS3.batchingOutputs = false;

    if (gotUpdate) {
        for (var packetName in TraktorS3.controller.OutputPackets) {
            TraktorS3.controller.OutputPackets[packetName].send();
        }
    }
};

// A special packet sent to the controller switches between mic and line
// input modes.  if lineMode is true, sets input to line. Otherwise, mic.
TraktorS3.setInputLineMode = function(lineMode) {
    var packet = Object();
    packet.length = 33;
    packet[0] = 0x20;
    if (!lineMode) {
        packet[1] = 0x08;
    }
    controller.send(packet, packet.length, 0xF4);
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
        "00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "22 22 22 22  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "22 22 22 22  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "FF FF FF FF  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00 ",
        "00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00",
        "20 08 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00"
    ];

    var data = [Object(), Object(), Object()];


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
        if (ok) {
            var header = 0x80 + i;
            if (i === 2) {
                header = 0xF4;
            }
            controller.send(data[i], data[i].length, header);
        }
    }
    TraktorS3.setInputLineMode(false);
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

    this.fxController = new TraktorS3.FXControl();

    TraktorS3.registerInputPackets();
    TraktorS3.registerOutputPackets();
    HIDDebug("TraktorS3: Init done!");

    if (TraktorS3DebugMode) {
        TraktorS3.debugLights();
    } else {
        TraktorS3.lightDeck("[Channel3]", false);
        TraktorS3.lightDeck("[Channel4]", false);
        TraktorS3.lightDeck("[Channel1]", false);
        TraktorS3.lightDeck("[Channel2]", true);
    }

    TraktorS3.setInputLineMode(TraktorS3.inputModeLine);
};

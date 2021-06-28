///////////////////////////////////////////////////////////////////////////////////
//
// Traktor Kontrol S3 HID controller script v1.00
// Last modification: August 2020
// Author: Owen Williams
// https://www.mixxx.org/wiki/doku.php/native_instruments_traktor_kontrol_s3
//
///////////////////////////////////////////////////////////////////////////////////
//
// TODO:
//   * star button
//
///////////////////////////////////////////////////////////////////////////////////

var TraktorS3 = {};

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
TraktorS3.PitchSliderRelativeMode = true;

// The Samplers can operate two ways.
// With SamplerModePressAndHold = false, tapping a Sampler button will start the
// sample playing.  Pressing the button again will stop playback.
// With SamplerModePressAndHold = true, a Sample will play while you hold the
// button down.  Letting go will stop playback.
TraktorS3.SamplerModePressAndHold = false;

// When this option is true, start up with the jog button lit, which means touching the job wheel
// enables scratch mode.
TraktorS3.JogDefaultOn = true;

// You can choose the colors you want for each channel. The list of colors is:
// RED, CARROT, ORANGE, HONEY, YELLOW, LIME, GREEN, AQUA, CELESTE, SKY, BLUE,
// PURPLE, FUCHSIA, MAGENTA, AZALEA, SALMON, WHITE
// Some colors may look odd because of how they are encoded inside the controller.
TraktorS3.ChannelColors = {
    "[Channel1]": "CARROT",
    "[Channel2]": "CARROT",
    "[Channel3]": "BLUE",
    "[Channel4]": "BLUE"
};

// Each color has four brightnesses, so these values can be between 0 and 3.
TraktorS3.LEDDimValue = 0x00;
TraktorS3.LEDBrightValue = 0x02;

// Set to true to output debug messages and debug light outputs.
TraktorS3.DebugMode = false;

TraktorS3.Controller = function() {
    this.hid = new HIDController();

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
    this.hid.LEDColors = {
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
        0: this.hid.LEDColors.PURPLE,
        1: this.hid.LEDColors.RED,
        2: this.hid.LEDColors.GREEN,
        3: this.hid.LEDColors.CELESTE,
        4: this.hid.LEDColors.YELLOW,
    };

    this.colorMap = new ColorMapper({
        0xCC0000: this.hid.LEDColors.RED,
        0xCC5E00: this.hid.LEDColors.CARROT,
        0xCC7800: this.hid.LEDColors.ORANGE,
        0xCC9200: this.hid.LEDColors.HONEY,

        0xCCCC00: this.hid.LEDColors.YELLOW,
        0x81CC00: this.hid.LEDColors.LIME,
        0x00CC00: this.hid.LEDColors.GREEN,
        0x00CC49: this.hid.LEDColors.AQUA,

        0x00CCCC: this.hid.LEDColors.CELESTE,
        0x0091CC: this.hid.LEDColors.SKY,
        0x0000CC: this.hid.LEDColors.BLUE,
        0xCC00CC: this.hid.LEDColors.PURPLE,

        0xCC0091: this.hid.LEDColors.FUCHSIA,
        0xCC0079: this.hid.LEDColors.MAGENTA,
        0xCC477E: this.hid.LEDColors.AZALEA,
        0xCC4761: this.hid.LEDColors.SALMON,

        0xCCCCCC: this.hid.LEDColors.WHITE,
    });

    // State for controller input loudness setting
    this.inputModeLine = false;

    // If true, channel 4 is in input mode
    this.channel4InputMode = false;

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
TraktorS3.Deck = function(controller, deckNumber, group) {
    this.controller = controller;
    this.deckNumber = deckNumber;
    this.group = group;
    this.activeChannel = "[Channel" + deckNumber + "]";
    // When true, touching the wheel enables scratch mode.  When off, touching the wheel
    // has no special effect
    this.jogToggled = TraktorS3.JogDefaultOn;
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
    this.controller.lightDeck(this.activeChannel);
};

// defineButton allows us to configure either the right deck or the left deck, depending on which
// is appropriate.  This avoids extra logic in the function where we define all the magic numbers.
// We use a similar approach in the other define funcs.
TraktorS3.Deck.prototype.defineButton = function(msg, name, deckOffset, deckBitmask, deck2Offset, deck2Bitmask, fn) {
    if (this.deckNumber === 2) {
        deckOffset = deck2Offset;
        deckBitmask = deck2Bitmask;
    }
    this.controller.registerInputButton(msg, this.group, name, deckOffset, deckBitmask, TraktorS3.bind(fn, this));
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
    this.controller.registerInputScaler(msg, this.group, name, deckOffset, deckBitmask, TraktorS3.bind(fn, this));
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
    // There is no control object to mark / unmark a track as played.
    // this.defineButton(messageShort, "!SetPlayed", 0x01, 0x10, 0x04, 0x20, deckFn.SetPlayedHandler);
    this.defineButton(messageShort, "!LibraryFocus", 0x01, 0x20, 0x04, 0x40, deckFn.LibraryFocusHandler);
    this.defineButton(messageShort, "!MaximizeLibrary", 0x01, 0x40, 0x04, 0x80, deckFn.MaximizeLibraryHandler);

    // Loop control
    // TODO: bind touch detections: 0x0A/0x01, 0x0A/0x08
    this.defineButton(messageShort, "!SelectLoop", 0x0C, 0x0F, 0x0D, 0xF0, deckFn.selectLoopHandler);
    this.defineButton(messageShort, "!ActivateLoop", 0x09, 0x04, 0x09, 0x20, deckFn.activateLoopHandler);

    // Rev / Flux / Grid / Jog
    this.defineButton(messageShort, "!reverse", 0x01, 0x04, 0x04, 0x08, deckFn.reverseHandler);
    this.defineButton(messageShort, "!slip_enabled", 0x01, 0x02, 0x04, 0x04, deckFn.fluxHandler);
    this.defineButton(messageShort, "quantize", 0x01, 0x80, 0x05, 0x01, deckFn.quantizeHandler);
    this.defineButton(messageShort, "!jogButton", 0x02, 0x01, 0x05, 0x02, deckFn.jogButtonHandler);

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
    this.controller.basicOutput(field.value, field.group, "!shift");
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
            this.syncPressedTimer = engine.beginTimer(300, TraktorS3.bind(function() {
                engine.setValue(this.activeChannel, "sync_enabled", 1);
                // Reset sync button timer state if active
                if (this.syncPressedTimer !== 0) {
                    this.syncPressedTimer = 0;
                }
            }, this), true);

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
    } else if (TraktorS3.PitchSliderRelativeMode) {
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

    // Hotcues mode
    if (this.padModeState === 0) {
        var action = this.shiftPressed ? "_clear" : "_activate";
        engine.setValue(this.activeChannel, "hotcue_" + padNumber + action, field.value);
        return;
    }

    // Samples mode
    var sampler = padNumber;
    if (field.group === "deck2") {
        sampler += 8;
    }

    var playing = engine.getValue("[Sampler" + sampler + "]", "play");
    if (this.shiftPressed) {
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
        if (TraktorS3.SamplerModePressAndHold) {
            if (field.value) {
                action = "cue_gotoandplay";
            } else {
                action = "stop";
            }
            engine.setValue("[Sampler" + sampler + "]", action, 1);
        } else {
            if (field.value) {
                if (playing) {
                    action = "stop";
                } else {
                    action = "cue_gotoandplay";
                }
                engine.setValue("[Sampler" + sampler + "]", action, 1);
            }
        }
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

    engine.setValue("[Library]", "MoveFocus", field.value);
};

TraktorS3.Deck.prototype.MaximizeLibraryHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    script.toggleControl("[Master]", "maximize_library");
};

TraktorS3.Deck.prototype.selectLoopHandler = function(field) {
    var delta = 1;
    if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
        delta = -1;
    }

    if (this.shiftPressed) {
        var beatjumpSize = engine.getValue(this.activeChannel, "beatjump_size");
        if (delta > 0) {
            script.triggerControl(this.activeChannel, "loop_move_" + beatjumpSize + "_forward");
        } else {
            script.triggerControl(this.activeChannel, "loop_move_" + beatjumpSize + "_backward");
        }
    } else {
        if (delta > 0) {
            script.triggerControl(this.activeChannel, "loop_double");
        } else {
            script.triggerControl(this.activeChannel, "loop_halve");
        }
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
        if (delta > 0) {
            script.triggerControl(this.activeChannel, "beatjump_forward");
        } else {
            script.triggerControl(this.activeChannel, "beatjump_backward");
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

TraktorS3.Deck.prototype.jogButtonHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    this.jogToggled = !this.jogToggled;
    this.colorOutput(this.jogToggled, "!jogButton");
};

TraktorS3.Deck.prototype.jogTouchHandler = function(field) {
    if (!this.jogToggled) {
        return;
    }
    if (this.wheelTouchInertiaTimer !== 0) {
        // The wheel was touched again, reset the timer.
        engine.stopTimer(this.wheelTouchInertiaTimer);
        this.wheelTouchInertiaTimer = 0;
    }
    if (field.value !== 0) {
        engine.setValue(this.activeChannel, "scratch2_enable", true);
        return;
    }
    // If shift is pressed, reset right away.
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "scratch2", 0.0);
        engine.setValue(this.activeChannel, "scratch2_enable", false);
        this.playIndicatorHandler(0, this.activeChannel);
        return;
    }
    // The wheel keeps moving after the user lifts their finger, so don't release scratch mode
    // right away.
    this.tickReceived = false;
    this.wheelTouchInertiaTimer = engine.beginTimer(
        100, TraktorS3.bind(TraktorS3.Deck.prototype.checkJogInertia, this), false);
};

TraktorS3.Deck.prototype.checkJogInertia = function() {
    // If we've received no ticks since the last call we are stopped.
    // In jog mode we always stop right away.
    if (!this.tickReceived) {
        engine.setValue(this.activeChannel, "scratch2", 0.0);
        engine.setValue(this.activeChannel, "scratch2_enable", false);
        this.playIndicatorHandler(0, this.activeChannel);
        engine.stopTimer(this.wheelTouchInertiaTimer);
        this.wheelTouchInertiaTimer = 0;
    }
    this.tickReceived = false;
};

TraktorS3.Deck.prototype.jogHandler = function(field) {
    this.tickReceived = true;
    var deltas = this.wheelDeltas(field.value);

    // If shift button is held, do a simple seek.
    if (this.shiftPressed) {
        // But if we're in the inertial period, ignore any wheel motion.
        if (this.wheelTouchInertiaTimer !== 0) {
            return;
        }
        var playPosition = engine.getValue(this.activeChannel, "playposition");
        playPosition += deltas[0] / 2048.0;
        playPosition = Math.max(Math.min(playPosition, 1.0), 0.0);
        engine.setValue(this.activeChannel, "playposition", playPosition);
        return;
    }
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

TraktorS3.Deck.prototype.pitchSliderHandler = function(field) {
    // Adapt HID value to rate control range.
    var value = -1.0 + ((field.value / 4095) * 2.0);
    if (TraktorS3.PitchSliderRelativeMode) {
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
                relVal = 1.0 - engine.getValue(this.activeChannel, "pitch_adjust");
            } else {
                relVal = engine.getValue(this.activeChannel, "rate");
            }
            // This can result in values outside -1 to 1, but that is valid for the
            // rate control. This means the entire swing of the rate slider can be
            // outside the range of the widget, but that's ok because the slider still
            // works.
            relVal += value - this.pitchSliderLastValue;
            this.pitchSliderLastValue = value;

            if (this.keylockPressed) {
                // To match the pitch change from adjusting the rate, flip the pitch
                // adjustment.
                engine.setValue(this.activeChannel, "pitch_adjust", 1.0 - relVal);
                this.keyAdjusted = true;
            } else {
                engine.setValue(this.activeChannel, "rate", relVal);
            }
        }
        return;
    }

    if (this.shiftPressed) {
        // To match the pitch change from adjusting the rate, flip the pitch
        // adjustment.
        engine.setValue(this.activeChannel, "pitch_adjust", 1.0 - value);
    } else {
        engine.setValue(this.activeChannel, "rate", value);
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
    this.defineOutput(outputA, "!LibraryFocus", 0x06, 0x1F);
    this.defineOutput(outputA, "!MaximizeLibrary", 0x07, 0x20);
    this.defineOutput(outputA, "quantize", 0x08, 0x21);
    this.defineOutput(outputA, "!jogButton", 0x09, 0x22);
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
        this.controller.hid.linkOutput("deck1", key, "[Channel1]", key, callback);
        engine.connectControl("[Channel3]", key, callback);
        break;
    case 2:
        this.controller.hid.linkOutput("deck2", key, "[Channel2]", key, callback);
        engine.connectControl("[Channel4]", key, callback);
        break;
    }
};

TraktorS3.Deck.prototype.linkOutputs = function() {
    var colorOutput = function(value, _group, key) {
        this.colorOutput(value, key);
    };

    var basicOutput = function(value, _group, key) {
        this.basicOutput(value, key);
    };

    this.defineLink("play_indicator", TraktorS3.bind(TraktorS3.Deck.prototype.playIndicatorHandler, this));
    this.defineLink("cue_indicator", TraktorS3.bind(colorOutput, this));
    this.defineLink("sync_enabled", TraktorS3.bind(colorOutput, this));
    this.defineLink("keylock", TraktorS3.bind(colorOutput, this));
    this.defineLink("slip_enabled", TraktorS3.bind(colorOutput, this));
    this.defineLink("quantize", TraktorS3.bind(colorOutput, this));
    this.defineLink("reverse", TraktorS3.bind(basicOutput, this));
    this.defineLink("scratch2_enable", TraktorS3.bind(colorOutput, this));
};

TraktorS3.Deck.prototype.deckBaseColor = function() {
    return this.controller.hid.LEDColors[TraktorS3.ChannelColors[this.activeChannel]];
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
    this.controller.hid.setOutput(this.group, key, ledValue, !TraktorS3.batchingOutputs);
};

// colorOutput drives lights that have the palettized multicolor lights.
TraktorS3.Deck.prototype.colorOutput = function(value, key) {
    var ledValue = this.deckBaseColor();

    if (value === 1 || value === true) {
        ledValue += TraktorS3.LEDBrightValue;
    } else {
        ledValue += TraktorS3.LEDDimValue;
    }
    this.controller.hid.setOutput(this.group, key, ledValue, !this.controller.batchingOutputs);
};

TraktorS3.Deck.prototype.playIndicatorHandler = function(value, group, _key) {
    // Also call regular handler
    this.basicOutput(value, "play_indicator");
    this.wheelOutputByValue(group, value);
};

TraktorS3.Deck.prototype.colorForHotcue = function(num) {
    var colorCode = engine.getValue(this.activeChannel, "hotcue_" + num + "_color");
    return this.controller.colorMap.getValueForNearestColor(colorCode);
};

TraktorS3.Deck.prototype.lightHotcue = function(number) {
    var loaded = engine.getValue(this.activeChannel, "hotcue_" + number + "_enabled");
    var active = engine.getValue(this.activeChannel, "hotcue_" + number + "_activate");
    var ledValue = this.controller.hid.LEDColors.WHITE;
    if (loaded) {
        ledValue = this.colorForHotcue(number);
        ledValue += TraktorS3.LEDDimValue;
    }
    if (active) {
        ledValue += TraktorS3.LEDBrightValue;
    } else {
        ledValue += TraktorS3.LEDDimValue;
    }
    this.controller.hid.setOutput(this.group, "!pad_" + number, ledValue, !TraktorS3.batchingOutputs);
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
    this.wheelOutput(group,
        [ledValue, ledValue, ledValue, ledValue, ledValue, ledValue, ledValue, ledValue]);
};

TraktorS3.Deck.prototype.wheelOutput = function(group, valueArray) {
    if (group !== this.activeChannel) {
        return;
    }

    for (var i = 0; i < 8; i++) {
        this.controller.hid.setOutput(this.group, "!wheel" + i, valueArray[i], false);
    }
    if (!TraktorS3.batchingOutputs) {
        for (var packetName in this.controller.hid.OutputPackets) {
            this.controller.hid.OutputPackets[packetName].send();
        }
    }
};

/////////////////////////
//// Channel Objects ////
////
//// Channels don't have much state, just the fx button state.
TraktorS3.Channel = function(controller, parentDeck, group) {
    this.controller = controller;
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
        var samples = engine.getValue(this.group, "track_loaded");
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
    this.clipConnection = engine.makeConnection(this.group, "PeakIndicator", TraktorS3.bind(TraktorS3.Controller.prototype.peakOutput, this.controller));
    this.controller.linkChannelOutput(this.group, "pfl", TraktorS3.bind(TraktorS3.Controller.prototype.pflOutput, this.controller));
    for (var j = 1; j <= 8; j++) {
        this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_enabled",
            TraktorS3.bind(TraktorS3.Channel.prototype.hotcuesOutput, this)));
        this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_activate",
            TraktorS3.bind(TraktorS3.Channel.prototype.hotcuesOutput, this)));
    }
};

TraktorS3.Channel.prototype.channelBaseColor = function() {
    if (this.group === "[Channel4]" && this.controller.channel4InputMode) {
        return this.controller.hid.LEDColors[this.controller.hid.LEDColors.OFF];
    }
    return this.controller.hid.LEDColors[TraktorS3.ChannelColors[this.group]];
};

// colorOutput drives lights that have the palettized multicolor lights.
TraktorS3.Channel.prototype.colorOutput = function(value, key) {
    var ledValue = this.channelBaseColor();
    if (value === 1 || value === true) {
        ledValue += TraktorS3.LEDBrightValue;
    } else {
        ledValue += TraktorS3.LEDDimValue;
    }
    this.controller.hid.setOutput(this.group, key, ledValue, !this.controller.batchingOutputs);
};

TraktorS3.Channel.prototype.hotcuesOutput = function(_value, group, key) {
    var deck = this.controller.Channels[group].parentDeck;
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
    var rotations = this.curPosition * (1 / 1.8);  // 1/1.8 is rotations per second (33 1/3 RPM)
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
TraktorS3.FXControl = function(controller) {
    // 0 is filter, 1-4 are FX Units 1-4
    this.FILTER_EFFECT = 0;
    this.activeFX = this.FILTER_EFFECT;
    this.controller = controller;

    this.enablePressed = {
        "[Channel1]": false,
        "[Channel2]": false,
        "[Channel3]": false,
        "[Channel4]": false
    };
    this.selectPressed = [
        false,
        false,
        false,
        false,
        false
    ];
    this.selectBlinkState = [
        false,
        false,
        false,
        false,
        false
    ];

    // States
    this.STATE_FILTER = 0;
    // State for when an effect select has been pressed, but not released yet.
    this.STATE_EFFECT_INIT = 1;
    // State for when an effect select has been pressed and released.
    this.STATE_EFFECT = 2;
    this.STATE_FOCUS = 3;

    this.currentState = this.STATE_FILTER;

    // Light states
    this.LIGHT_OFF = 0;
    this.LIGHT_DIM = 1;
    this.LIGHT_BRIGHT = 2;

    this.focusBlinkState = false;
    this.focusBlinkTimer = 0;
};

TraktorS3.FXControl.prototype.registerInputs = function(messageShort, messageLong) {
    // FX Buttons
    var fxFn = TraktorS3.FXControl.prototype;
    this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx1", 0x08, 0x08, TraktorS3.bind(fxFn.fxSelectHandler, this));
    this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx2", 0x08, 0x10, TraktorS3.bind(fxFn.fxSelectHandler, this));
    this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx3", 0x08, 0x20, TraktorS3.bind(fxFn.fxSelectHandler, this));
    this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx4", 0x08, 0x40, TraktorS3.bind(fxFn.fxSelectHandler, this));
    this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx0", 0x08, 0x80, TraktorS3.bind(fxFn.fxSelectHandler, this));

    this.controller.registerInputButton(messageShort, "[Channel3]", "!fxEnabled", 0x07, 0x08, TraktorS3.bind(fxFn.fxEnableHandler, this));
    this.controller.registerInputButton(messageShort, "[Channel1]", "!fxEnabled", 0x07, 0x10, TraktorS3.bind(fxFn.fxEnableHandler, this));
    this.controller.registerInputButton(messageShort, "[Channel2]", "!fxEnabled", 0x07, 0x20, TraktorS3.bind(fxFn.fxEnableHandler, this));
    this.controller.registerInputButton(messageShort, "[Channel4]", "!fxEnabled", 0x07, 0x40, TraktorS3.bind(fxFn.fxEnableHandler, this));

    this.controller.registerInputScaler(messageLong, "[Channel1]", "!fxKnob", 0x39, 0xFFFF, TraktorS3.bind(fxFn.fxKnobHandler, this));
    this.controller.registerInputScaler(messageLong, "[Channel2]", "!fxKnob", 0x3B, 0xFFFF, TraktorS3.bind(fxFn.fxKnobHandler, this));
    this.controller.registerInputScaler(messageLong, "[Channel3]", "!fxKnob", 0x37, 0xFFFF, TraktorS3.bind(fxFn.fxKnobHandler, this));
    this.controller.registerInputScaler(messageLong, "[Channel4]", "!fxKnob", 0x3D, 0xFFFF, TraktorS3.bind(fxFn.fxKnobHandler, this));
};

TraktorS3.FXControl.prototype.channelToIndex = function(group) {
    var result = group.match(script.channelRegEx);
    if (result === null) {
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

TraktorS3.FXControl.prototype.firstPressedSelect = function() {
    for (var idx in this.selectPressed) {
        if (this.selectPressed[idx]) {
            return idx;
        }
    }
    return undefined;
};

TraktorS3.FXControl.prototype.firstPressedEnable = function() {
    for (var ch in this.enablePressed) {
        if (this.enablePressed[ch]) {
            return ch;
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

TraktorS3.FXControl.prototype.changeState = function(newState) {
    if (newState === this.currentState) {
        return;
    }

    // Ignore next values for all knob actions. This is safe to do for all knobs
    // even if we're ignoring knobs that aren't active in the new state.
    for (var ch = 1; ch <= 4; ch++) {
        var group = "[Channel" + ch + "]";
        engine.softTakeoverIgnoreNextValue("[QuickEffectRack1_" + group + "]", "super1");
    }
    for (var unit = 1; unit <= 4; unit++) {
        group = "[EffectRack1_EffectUnit" + unit + "]";
        key = "mix";
        engine.softTakeoverIgnoreNextValue(group, key);
        for (var effect = 1; effect <= 4; effect++) {
            group = "[EffectRack1_EffectUnit" + unit + "_Effect" + effect + "]";
            key = "meta";
            engine.softTakeoverIgnoreNextValue(group, key);
            for (var param = 1; param <= 4; param++) {
                var key = "parameter" + param;
                engine.softTakeoverIgnoreNextValue(group, key);
            }
        }
    }

    var oldState = this.currentState;
    this.currentState = newState;
    if (oldState === this.STATE_FOCUS) {
        engine.stopTimer(this.focusBlinkTimer);
        this.focusBlinkTimer = 0;
    }
    switch (newState) {
    case this.STATE_FILTER:
        break;
    case this.STATE_EFFECT_INIT:
        break;
    case this.STATE_EFFECT:
        break;
    case this.STATE_FOCUS:
        this.focusBlinkTimer = engine.beginTimer(150, function() {
            TraktorS3.kontrol.fxController.focusBlinkState = !TraktorS3.kontrol.fxController.focusBlinkState;
            TraktorS3.kontrol.fxController.lightFX();
        }, false);
    }
};

TraktorS3.FXControl.prototype.fxSelectHandler = function(field) {
    var fxNumber = parseInt(field.name[field.name.length - 1]);
    // Coerce to boolean
    this.selectPressed[fxNumber] = !!field.value;

    if (!field.value) {
        if (fxNumber === this.activeFX) {
            if (this.currentState === this.STATE_EFFECT) {
                this.changeState(this.STATE_FILTER);
            } else if (this.currentState === this.STATE_EFFECT_INIT) {
                this.changeState(this.STATE_EFFECT);
            }
        }
        this.lightFX();
        return;
    }

    switch (this.currentState) {
    case this.STATE_FILTER:
        // If any fxEnable button is pressed, we are toggling fx unit assignment.
        if (this.anyEnablePressed()) {
            for (var key in this.enablePressed) {
                if (this.enablePressed[key]) {
                    if (fxNumber === 0) {
                        var fxGroup = "[QuickEffectRack1_" + key + "_Effect1]";
                        var fxKey = "enabled";
                    } else {
                        fxGroup = "[EffectRack1_EffectUnit" + fxNumber + "]";
                        fxKey = "group_" + key + "_enable";
                    }
                    script.toggleControl(fxGroup, fxKey);
                }
            }
        } else {
            if (fxNumber === 0) {
                this.changeState(this.STATE_FILTER);
            } else {
                this.changeState(this.STATE_EFFECT_INIT);
            }
            this.activeFX = fxNumber;
        }
        break;
    case this.STATE_EFFECT_INIT:
        // Fallthrough intended
    case this.STATE_EFFECT:
        if (fxNumber === 0) {
            this.changeState(this.STATE_FILTER);
        } else if (fxNumber !== this.activeFX) {
            this.changeState(this.STATE_EFFECT_INIT);
        }
        this.activeFX = fxNumber;
        break;
    case this.STATE_FOCUS:
        if (fxNumber === 0) {
            this.changeState(this.STATE_FILTER);
        } else {
            this.changeState(this.STATE_EFFECT_INIT);
        }
        this.activeFX = fxNumber;
        break;
    }
    this.lightFX();
};

TraktorS3.FXControl.prototype.fxEnableHandler = function(field) {
    // Coerce to boolean
    this.enablePressed[field.group] = !!field.value;

    if (!field.value) {
        this.lightFX();
        return;
    }

    var fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
    var buttonNumber = this.channelToIndex(field.group);
    switch (this.currentState) {
    case this.STATE_FILTER:
        break;
    case this.STATE_EFFECT_INIT:
        // Fallthrough intended
    case this.STATE_EFFECT:
        if (this.firstPressedSelect()) {
            // Choose the first pressed select button only.
            this.changeState(this.STATE_FOCUS);
            engine.setValue(fxGroupPrefix + "]", "focused_effect", buttonNumber);
        } else {
            var group = fxGroupPrefix + "_Effect" + buttonNumber + "]";
            var key = "enabled";
            script.toggleControl(group, key);
        }
        break;
    case this.STATE_FOCUS:
        var focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
        group = fxGroupPrefix + "_Effect" + focusedEffect + "]";
        key = "button_parameter" + buttonNumber;
        script.toggleControl(group, key);
        break;
    }
    this.lightFX();
};

TraktorS3.FXControl.prototype.fxKnobHandler = function(field) {
    var value = field.value / 4095.;
    var fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
    var knobIdx = this.channelToIndex(field.group);

    switch (this.currentState) {
    case this.STATE_FILTER:
        if (field.group === "[Channel4]" && this.controller.channel4InputMode) {
            // There is no quickeffect for the microphone, do nothing.
            return;
        }
        engine.setParameter("[QuickEffectRack1_" + field.group + "]", "super1", value);
        break;
    case this.STATE_EFFECT_INIT:
        // Fallthrough intended
    case this.STATE_EFFECT:
        if (knobIdx === 4) {
            engine.setParameter(fxGroupPrefix + "]", "mix", value);
        } else {
            var group = fxGroupPrefix + "_Effect" + knobIdx + "]";
            engine.setParameter(group, "meta", value);
        }
        break;
    case this.STATE_FOCUS:
        var focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
        group = fxGroupPrefix + "_Effect" + focusedEffect + "]";
        var key = "parameter" + knobIdx;
        engine.setParameter(group, key, value);
        break;
    }
};

TraktorS3.FXControl.prototype.getFXSelectLEDValue = function(fxNumber, status) {
    var ledValue = this.controller.fxLEDValue[fxNumber];
    switch (status) {
    case this.LIGHT_OFF:
        return 0x00;
    case this.LIGHT_DIM:
        return ledValue;
    case this.LIGHT_BRIGHT:
        return ledValue + 0x02;
    }
};

TraktorS3.FXControl.prototype.getChannelColor = function(group, status) {
    var ledValue = this.controller.hid.LEDColors[TraktorS3.ChannelColors[group]];
    switch (status) {
    case this.LIGHT_OFF:
        return 0x00;
    case this.LIGHT_DIM:
        return ledValue;
    case this.LIGHT_BRIGHT:
        return ledValue + 0x02;
    }
};

TraktorS3.FXControl.prototype.lightFX = function() {
    this.controller.batchingOutputs = true;

    // Loop through select buttons
    // Idx zero is filter button
    for (var idx = 0; idx < 5; idx++) {
        this.lightSelect(idx);
    }
    for (var ch = 1; ch <= 4; ch++) {
        var channel = "[Channel" + ch + "]";
        this.lightEnable(channel);
    }

    this.controller.batchingOutputs = false;
    for (var packetName in this.controller.hid.OutputPackets) {
        this.controller.hid.OutputPackets[packetName].send();
    }
};

TraktorS3.FXControl.prototype.lightSelect = function(idx) {
    var status = this.LIGHT_OFF;
    var ledValue = 0x00;
    switch (this.currentState) {
    case this.STATE_FILTER:
        // Always light when pressed
        if (this.selectPressed[idx]) {
            status = this.LIGHT_BRIGHT;
        } else {
            // select buttons on if fx unit enabled for the pressed channel,
            // otherwise disabled.
            status = this.LIGHT_DIM;
            var pressed = this.firstPressedEnable();
            if (pressed) {
                if (idx === 0) {
                    var fxGroup = "[QuickEffectRack1_" + pressed + "_Effect1]";
                    var fxKey = "enabled";
                } else {
                    fxGroup = "[EffectRack1_EffectUnit" + idx + "]";
                    fxKey = "group_" + pressed + "_enable";
                }
                if (engine.getParameter(fxGroup, fxKey)) {
                    status = this.LIGHT_BRIGHT;
                } else {
                    status = this.LIGHT_OFF;
                }
            }
            ledValue = this.getFXSelectLEDValue(idx, status);
        }
        break;
    case this.STATE_EFFECT_INIT:
        // Fallthrough intended
    case this.STATE_EFFECT:
        // Highlight if pressed, disable if active effect.
        // Otherwise off.
        if (this.selectPressed[idx]) {
            status = this.LIGHT_BRIGHT;
        } else if (idx === this.activeFX) {
            status = this.LIGHT_BRIGHT;
        }
        break;
    case this.STATE_FOCUS:
        // if blink state is false, only like active fx bright
        // if blink state is true, active fx is bright and selected effect
        // is dim.  if those are the same, active fx is dim
        if (this.selectPressed[idx]) {
            status = this.LIGHT_BRIGHT;
        } else {
            if (idx === this.activeFX) {
                status = this.LIGHT_BRIGHT;
            }
            if (this.focusBlinkState) {
                var fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
                var focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
                if (idx === focusedEffect) {
                    status = this.LIGHT_DIM;
                }
            }
        }
        break;
    }
    ledValue = this.getFXSelectLEDValue(idx, status);
    this.controller.hid.setOutput("[ChannelX]", "!fxButton" + idx, ledValue, false);
};

TraktorS3.FXControl.prototype.lightEnable = function(channel) {
    var status = this.LIGHT_OFF;
    var ledValue = 0x00;
    var buttonNumber = this.channelToIndex(channel);
    switch (this.currentState) {
    case this.STATE_FILTER:
        // enable buttons highlighted if pressed or if any fx unit enabled for channel.
        // Highlight if pressed.
        status = this.LIGHT_DIM;
        if (this.enablePressed[channel]) {
            status = this.LIGHT_BRIGHT;
        } else {
            for (var idx = 1; idx <= 4 && status === this.LIGHT_OFF; idx++) {
                var group = "[EffectRack1_EffectUnit" + idx + "]";
                var key = "group_" + channel + "_enable";
                if (engine.getParameter(group, key)) {
                    status = this.LIGHT_DIM;
                }
            }
        }
        // Enable buttons have regular deck colors
        ledValue = this.getChannelColor(channel, status);
        break;
    case this.STATE_EFFECT_INIT:
        // Fallthrough intended
    case this.STATE_EFFECT:
        if (this.enablePressed[channel]) {
            status = this.LIGHT_BRIGHT;
        } else {
            // off if nothing loaded, dim if loaded, bright if enabled.
            group = "[EffectRack1_EffectUnit" + this.activeFX + "_Effect" + buttonNumber + "]";
            if (engine.getParameter(group, "loaded")) {
                status = this.LIGHT_DIM;
            }
            if (engine.getParameter(group, "enabled")) {
                status = this.LIGHT_BRIGHT;
            }
        }
        // Colors match effect colors so it's obvious we're in a different mode
        ledValue = this.getFXSelectLEDValue(this.activeFX, status);
        break;
    case this.STATE_FOCUS:
        if (this.enablePressed[channel]) {
            status = this.LIGHT_BRIGHT;
        } else {
            var fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
            var focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
            group = fxGroupPrefix + "_Effect" + focusedEffect + "]";
            key = "button_parameter" + buttonNumber;
            // Off if not loaded, dim if loaded, bright if enabled.
            if (engine.getParameter(group, key + "_loaded")) {
                status = this.LIGHT_DIM;
            }
            if (engine.getParameter(group, key)) {
                status = this.LIGHT_BRIGHT;
            }
        }
        // Colors match effect colors so it's obvious we're in a different mode
        ledValue = this.getFXSelectLEDValue(this.activeFX, status);
        break;
    }
    this.controller.hid.setOutput(channel, "!fxEnabled", ledValue, false);
};

TraktorS3.Controller.prototype.registerInputPackets = function() {
    var messageShort = new HIDPacket("shortmessage", 0x01, TraktorS3.messageCallback);
    var messageLong = new HIDPacket("longmessage", 0x02, TraktorS3.messageCallback);

    for (var idx in this.Decks) {
        var deck = this.Decks[idx];
        deck.registerInputs(messageShort, messageLong);
    }

    this.registerInputButton(messageShort, "[Channel1]", "!switchDeck", 0x02, 0x02, TraktorS3.bind(TraktorS3.Controller.prototype.deckSwitchHandler, this));
    this.registerInputButton(messageShort, "[Channel2]", "!switchDeck", 0x05, 0x04, TraktorS3.bind(TraktorS3.Controller.prototype.deckSwitchHandler, this));
    this.registerInputButton(messageShort, "[Channel3]", "!switchDeck", 0x02, 0x04, TraktorS3.bind(TraktorS3.Controller.prototype.deckSwitchHandler, this));
    this.registerInputButton(messageShort, "[Channel4]", "!switchDeck", 0x05, 0x08, TraktorS3.bind(TraktorS3.Controller.prototype.deckSwitchHandler, this));

    // Headphone buttons
    this.registerInputButton(messageShort, "[Channel1]", "pfl", 0x08, 0x01, TraktorS3.bind(TraktorS3.Controller.prototype.headphoneHandler, this));
    this.registerInputButton(messageShort, "[Channel2]", "pfl", 0x08, 0x02, TraktorS3.bind(TraktorS3.Controller.prototype.headphoneHandler, this));
    this.registerInputButton(messageShort, "[Channel3]", "pfl", 0x07, 0x80, TraktorS3.bind(TraktorS3.Controller.prototype.headphoneHandler, this));
    this.registerInputButton(messageShort, "[Channel4]", "pfl", 0x08, 0x04, TraktorS3.bind(TraktorS3.Controller.prototype.headphoneHandler, this));

    // EXT Button
    this.registerInputButton(messageShort, "[Master]", "!extButton", 0x07, 0x04, TraktorS3.bind(TraktorS3.Controller.prototype.extModeHandler, this));

    this.fxController.registerInputs(messageShort, messageLong);

    this.hid.registerInputPacket(messageShort);

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
    this.registerInputScaler(messageLong, "[Master]", "gain", 0x17, 0xFFFF, TraktorS3.bind(TraktorS3.Controller.prototype.masterGainHandler, this));
    this.registerInputScaler(messageLong, "[Master]", "headMix", 0x1D, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headGain", 0x1B, 0xFFFF, this.parameterHandler);

    this.hid.registerInputPacket(messageLong);

    // Soft takeovers
    for (var ch = 1; ch <= 4; ch++) {
        var group = "[Channel" + ch + "]";
        if (!TraktorS3.PitchSliderRelativeMode) {
            engine.softTakeover(group, "rate", true);
        }
        engine.softTakeover(group, "pitch_adjust", true);
        engine.softTakeover(group, "volume", true);
        engine.softTakeover(group, "pregain", true);
        engine.softTakeover("[QuickEffectRack1_" + group + "]", "super1", true);
    }
    for (var unit = 1; unit <= 4; unit++) {
        group = "[EffectRack1_EffectUnit" + unit + "]";
        var key = "mix";
        engine.softTakeover(group, key, true);
        for (var effect = 1; effect <= 4; effect++) {
            group = "[EffectRack1_EffectUnit" + unit + "_Effect" + effect + "]";
            key = "meta";
            engine.softTakeover(group, key, true);
            for (var param = 1; param <= 4; param++) {
                key = "parameter" + param;
                engine.softTakeover(group, key, true);
            }
        }
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

    for (ch in this.Channels) {
        var chanob = this.Channels[ch];
        engine.makeConnection(ch, "playposition",
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

TraktorS3.Controller.prototype.registerInputJog = function(message, group, name, offset, bitmask, callback) {
    // Jog wheels have 4 byte input
    message.addControl(group, name, offset, "I", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS3.Controller.prototype.registerInputScaler = function(message, group, name, offset, bitmask, callback) {
    message.addControl(group, name, offset, "H", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS3.Controller.prototype.registerInputButton = function(message, group, name, offset, bitmask, callback) {
    message.addControl(group, name, offset, "B", bitmask);
    message.setCallback(group, name, callback);
};

TraktorS3.Controller.prototype.parameterHandler = function(field) {
    if (field.group === "[Channel4]" && this.channel4InputMode) {
        engine.setParameter("[Microphone]", field.name, field.value / 4095);
    } else {
        engine.setParameter(field.group, field.name, field.value / 4095);
    }
};

TraktorS3.Controller.prototype.anyShiftPressed = function() {
    return this.Decks["deck1"].shiftPressed || this.Decks["deck2"].shiftPressed;
};

TraktorS3.Controller.prototype.masterGainHandler = function(field) {
    // Only adjust if shift is held.  This will still adjust the sound card
    // volume but it at least allows for control of Mixxx's master gain.
    if (this.anyShiftPressed()) {
        engine.setParameter(field.group, field.name, field.value / 4095);
    }
};

TraktorS3.Controller.prototype.headphoneHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    if (field.group === "[Channel4]" && this.channel4InputMode) {
        script.toggleControl("[Microphone]", "pfl");
    } else {
        script.toggleControl(field.group, "pfl");
    }
};

TraktorS3.Controller.prototype.deckSwitchHandler = function(field) {
    if (field.value === 0) {
        return;
    }

    var channel = this.Channels[field.group];
    var deck = channel.parentDeck;
    deck.activateChannel(channel);
};

TraktorS3.Controller.prototype.extModeHandler = function(field) {
    if (!field.value) {
        this.basicOutput(this.channel4InputMode, field.group, field.name);
        return;
    }
    if (this.anyShiftPressed()) {
        this.basicOutput(field.value, field.group, field.name);
        this.inputModeLine = !this.inputModeLine;
        this.setInputLineMode(this.inputModeLine);
        return;
    }
    this.channel4InputMode = !this.channel4InputMode;
    if (this.channel4InputMode) {
        engine.softTakeoverIgnoreNextValue("[Microphone]", "volume");
        engine.softTakeoverIgnoreNextValue("[Microphone]", "pregain");
    } else {
        engine.softTakeoverIgnoreNextValue("[Channel4]", "volume");
        engine.softTakeoverIgnoreNextValue("[Channel4]", "pregain");
    }
    this.lightDeck("[Channel4]");
    this.basicOutput(this.channel4InputMode, field.group, field.name);
};

TraktorS3.Controller.prototype.registerOutputPackets = function() {
    var outputA = new HIDPacket("outputA", 0x80);
    var outputB = new HIDPacket("outputB", 0x81);

    for (var idx in this.Decks) {
        var deck = this.Decks[idx];
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
    outputA.addOutput("[ChannelX]", "!fxButton0", 0x40, "B");

    outputA.addOutput("[Channel3]", "!fxEnabled", 0x34, "B");
    outputA.addOutput("[Channel1]", "!fxEnabled", 0x35, "B");
    outputA.addOutput("[Channel2]", "!fxEnabled", 0x36, "B");
    outputA.addOutput("[Channel4]", "!fxEnabled", 0x37, "B");

    outputA.addOutput("[Master]", "!extButton", 0x33, "B");

    this.hid.registerOutputPacket(outputA);

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

    this.hid.registerOutputPacket(outputB);

    for (idx in this.Decks) {
        deck = this.Decks[idx];
        deck.linkOutputs();
    }

    for (idx in this.Channels) {
        var chan = this.Channels[idx];
        chan.linkOutputs();
    }

    engine.connectControl("[Microphone]", "pfl", this.pflOutput);

    engine.connectControl("[Master]", "maximize_library", TraktorS3.bind(TraktorS3.Controller.prototype.maximizeLibraryOutput, this));

    // Master VuMeters
    this.masterVuMeter["VuMeterL"].connection = engine.makeConnection("[Master]", "VuMeterL", TraktorS3.bind(TraktorS3.Controller.prototype.masterVuMeterHandler, this));
    this.masterVuMeter["VuMeterR"].connection = engine.makeConnection("[Master]", "VuMeterR", TraktorS3.bind(TraktorS3.Controller.prototype.masterVuMeterHandler, this));
    this.linkChannelOutput("[Master]", "PeakIndicatorL", TraktorS3.bind(TraktorS3.Controller.prototype.peakOutput, this));
    this.linkChannelOutput("[Master]", "PeakIndicatorR", TraktorS3.bind(TraktorS3.Controller.prototype.peakOutput, this));
    this.guiTickConnection = engine.makeConnection("[Master]", "guiTick50ms", TraktorS3.bind(TraktorS3.Controller.prototype.guiTickHandler, this));

    // Sampler callbacks
    for (i = 1; i <= 8; ++i) {
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "track_loaded", TraktorS3.bind(TraktorS3.Controller.prototype.samplesOutput, this)));
        this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "play", TraktorS3.bind(TraktorS3.Controller.prototype.samplesOutput, this)));
    }
};

TraktorS3.Controller.prototype.linkChannelOutput = function(group, name, callback) {
    this.hid.linkOutput(group, name, group, name, callback);
};

TraktorS3.Controller.prototype.pflOutput = function(value, group, key) {
    if (group === "[Microphone]" && this.channel4InputMode) {
        this.basicOutput(value, "[Channel4]", key);
        return;
    }
    if (group === "[Channel4]" && !this.channel4InputMode) {
        this.basicOutput(value, group, key);
        return;
    }
    if (group.match(/^\[Channel[123]\]$/)) {
        this.basicOutput(value, group, key);
    }
    // Unhandled case, ignore.
};

TraktorS3.Controller.prototype.maximizeLibraryOutput = function(value, _group, _key) {
    this.Decks["deck1"].colorOutput(value, "!MaximizeLibrary");
    this.Decks["deck2"].colorOutput(value, "!MaximizeLibrary");
};

// Output drives lights that only have one color.
TraktorS3.Controller.prototype.basicOutput = function(value, group, key) {
    var ledValue = value;
    if (value === 0 || value === false) {
        // Off value
        ledValue = 0x04;
    } else if (value === 1 || value === true) {
        // On value
        ledValue = 0xFF;
    }

    this.hid.setOutput(group, key, ledValue, !this.batchingOutputs);
};

TraktorS3.Controller.prototype.peakOutput = function(value, group, key) {
    var ledValue = 0x00;
    if (value) {
        ledValue = 0x7E;
    }

    this.hid.setOutput(group, key, ledValue, !this.batchingOutputs);
};

TraktorS3.Controller.prototype.masterVuMeterHandler = function(value, _group, key) {
    this.masterVuMeter[key].updated = true;
    this.masterVuMeter[key].value = value;
};

TraktorS3.Controller.prototype.vuMeterOutput = function(value, group, key, segments) {
    // This handler is called a lot so it should be as fast as possible.
    var scaledValue = value * segments;
    var fullIllumCount = Math.floor(scaledValue);

    // Figure out how much the partially-illuminated segment is illuminated.
    var partialIllum = (scaledValue - fullIllumCount) * 0x7F;

    for (var i = 0; i < segments; i++) {
        var segmentKey = "!" + key + i;
        if (i < fullIllumCount) {
            // Don't update lights until they're all done, so the last term is false.
            this.hid.setOutput(group, segmentKey, 0x7F, false);
        } else if (i === fullIllumCount) {
            this.hid.setOutput(group, segmentKey, partialIllum, false);
        } else {
            this.hid.setOutput(group, segmentKey, 0x00, false);
        }
    }
    if (!this.batchingOutputs) {
        this.hid.OutputPackets["outputB"].send();
    }
};

TraktorS3.Controller.prototype.resolveSampler = function(group) {
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

TraktorS3.Controller.prototype.samplesOutput = function(value, group, key) {
    // Sampler 1-8 -> Channel1
    // Samples 9-16 -> Channel2
    var sampler = this.resolveSampler(group);
    var deck = this.Decks["deck1"];
    var num = sampler;
    if (sampler === undefined) {
        return;
    } else if (sampler > 8 && sampler < 17) {
        deck = this.Decks["deck2"];
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

TraktorS3.Controller.prototype.lightGroup = function(packet, outputGroupName, coGroupName) {
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

TraktorS3.Controller.prototype.lightDeck = function(group, sendPackets) {
    if (sendPackets === undefined) {
        sendPackets = true;
    }
    // Freeze the lights while we do this update so we don't spam HID.
    this.batchingOutputs = true;
    for (var packetName in this.hid.OutputPackets) {
        var packet = this.hid.OutputPackets[packetName];
        var deckGroupName = "deck1";
        if (group === "[Channel2]" || group === "[Channel4]") {
            deckGroupName = "deck2";
        }

        var deck = this.Decks[deckGroupName];

        this.lightGroup(packet, deckGroupName, group);
        this.lightGroup(packet, group, group);

        deck.lightPads();

        // These lights are different because either they aren't associated with a CO, or
        // there are two buttons that point to the same CO.
        deck.basicOutput(0, "!shift");
        deck.colorOutput(0, "!PreviewTrack");
        deck.colorOutput(0, "!LibraryFocus");
        deck.colorOutput(0, "!MaximizeLibrary");
        deck.colorOutput(deck.jogToggled, "!jogButton");
        if (group === "[Channel4]") {
            this.basicOutput(0, "[Master]", "!extButton");
        }
    }
    // this.lightFX();

    // Selected deck lights
    if (group === "[Channel1]") {
        this.hid.setOutput("[Channel1]", "!deck_A", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel1]"]] + TraktorS3.LEDBrightValue, false);
        this.hid.setOutput("[Channel3]", "!deck_C", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel3]"]] + TraktorS3.LEDDimValue, false);
    } else if (group === "[Channel2]") {
        this.hid.setOutput("[Channel2]", "!deck_B", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel2]"]] + TraktorS3.LEDBrightValue, false);
        this.hid.setOutput("[Channel4]", "!deck_D", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel4]"]] + TraktorS3.LEDDimValue, false);
    } else if (group === "[Channel3]") {
        this.hid.setOutput("[Channel3]", "!deck_C", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel3]"]] + TraktorS3.LEDBrightValue, false);
        this.hid.setOutput("[Channel1]", "!deck_A", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel1]"]] + TraktorS3.LEDDimValue, false);
    } else if (group === "[Channel4]") {
        this.hid.setOutput("[Channel4]", "!deck_D", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel4]"]] + TraktorS3.LEDBrightValue, false);
        this.hid.setOutput("[Channel2]", "!deck_B", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel2]"]] + TraktorS3.LEDDimValue, false);
    }

    this.batchingOutputs = false;
    // And now send them all.
    if (sendPackets) {
        for (packetName in this.hid.OutputPackets) {
            this.hid.OutputPackets[packetName].send();
        }
    }
};

// Render wheel positions, channel VU meters, and master vu meters
TraktorS3.Controller.prototype.guiTickHandler = function() {
    this.batchingOutputs = true;
    var gotUpdate = false;
    gotUpdate |= this.Channels[this.Decks["deck1"].activeChannel].lightWheelPosition();
    gotUpdate |= this.Channels[this.Decks["deck2"].activeChannel].lightWheelPosition();

    for (var vu in this.masterVuMeter) {
        if (this.masterVuMeter[vu].updated) {
            this.vuMeterOutput(this.masterVuMeter[vu].value, "[Master]", vu, 8);
            this.masterVuMeter[vu].updated = false;
            gotUpdate = true;
        }
    }
    for (var ch = 1; ch <= 4; ch++) {
        var chan = this.Channels["[Channel" + ch + "]"];
        if (chan.vuMeterUpdated) {
            this.vuMeterOutput(chan.vuMeterValue, chan.group, "VuMeter", 14);
            chan.vuMeterUpdated = false;
            gotUpdate = true;
        }
    }

    this.batchingOutputs = false;

    if (gotUpdate) {
        for (var packetName in this.hid.OutputPackets) {
            this.hid.OutputPackets[packetName].send();
        }
    }
};

// A special packet sent to the controller switches between mic and line
// input modes.  if lineMode is true, sets input to line. Otherwise, mic.
TraktorS3.Controller.prototype.setInputLineMode = function(lineMode) {
    var packet = Array();
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
            TraktorS3.kontrol.hid.processButton(data[name]);
        }
    }
};

TraktorS3.incomingData = function(data, length) {
    TraktorS3.kontrol.hid.parsePacket(data, length);
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

    var data = [Array(), Array(), Array()];


    for (var i = 0; i < data.length; i++) {
        var ok = true;
        var tokens = dataStrings[i].split(/\s+/);
        HIDDebug("i " + i + " " + tokens);
        data[i].length = tokens.length;
        for (var j = 0; j < tokens.length; j++) {
            var byteStr = tokens[j];
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
    TraktorS3.kontrol.setInputLineMode(false);
};

TraktorS3.shutdown = function() {
    // Deactivate all LEDs
    var packet = Array(267);
    for (var i = 0; i < packet.length; i++) {
        packet[i] = 0;
    }
    controller.send(packet, packet.length, 0x80);
    packet = Array(251);
    for (i = 0; i < packet.length; i++) {
        packet[i] = 0;
    }
    controller.send(packet, packet.length, 0x81);

    HIDDebug("TraktorS3: Shutdown done!");
};

TraktorS3.init = function(_id) {
    this.kontrol = new TraktorS3.Controller();
    this.kontrol.Decks = {
        "deck1": new TraktorS3.Deck(this.kontrol, 1, "deck1"),
        "deck2": new TraktorS3.Deck(this.kontrol, 2, "deck2"),
    };

    this.kontrol.Channels = {
        "[Channel1]": new TraktorS3.Channel(this.kontrol, this.kontrol.Decks["deck1"], "[Channel1]"),
        "[Channel3]": new TraktorS3.Channel(this.kontrol, this.kontrol.Decks["deck1"], "[Channel3]"),
        "[Channel4]": new TraktorS3.Channel(this.kontrol, this.kontrol.Decks["deck2"], "[Channel4]"),
        "[Channel2]": new TraktorS3.Channel(this.kontrol, this.kontrol.Decks["deck2"], "[Channel2]"),
    };

    this.kontrol.fxController = new TraktorS3.FXControl(this.kontrol);

    this.kontrol.registerInputPackets();
    this.kontrol.registerOutputPackets();
    HIDDebug("TraktorS3: Init done!");

    if (TraktorS3.DebugMode) {
        TraktorS3.debugLights();
    } else {
        // Light secondary decks first so that the primary decks overwrite their values where
        // needed.  This way the controller looks correct on startup.
        this.kontrol.lightDeck("[Channel3]", false);
        this.kontrol.lightDeck("[Channel4]", false);
        this.kontrol.lightDeck("[Channel1]", false);
        this.kontrol.lightDeck("[Channel2]", true);
        this.kontrol.fxController.lightFX();
    }

    this.kontrol.setInputLineMode(TraktorS3.inputModeLine);
};

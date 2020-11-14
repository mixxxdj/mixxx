///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* Traktor Kontrol Z2 HID controller script v1.00                                */
/* Last modification: August 2020                                                */
/* Author: Jörg Wartenberg (based on the Traktor S3 mapping by Owen Williams)    */
/*                                                                               */
/* To inhibit false 'Undeclared variable' warnings by codefactor:                */
/* global HIDController, HIDDebug, HIDPacket, controller                         */
///////////////////////////////////////////////////////////////////////////////////

// Each color has 8 brightnesses, so these values can be between 0 and 7.
var LedOff    = 0x00;
var LedDimmed = 0x02;
var LedBright = 0x07;

var TraktorZ2 = new function() {
    this.controller = new HIDController();

    this.shiftPressed = false;

    // 0x00: shift mode off / and not active pressed
    // 0x01: shift mode off / but active pressed
    // 0x02: shift mode on  / and not active pressed
    // 0x03: shift mode on  / and active pressed
    this.shiftState  = 0x00;

    // Knob encoder states (hold values between 0x0 and 0xF)
    // Rotate to the right is +1 and to the left is means -1
    this.browseKnobEncoderState = 0;

    this.displayBrightness = [];
    
    this.pregainCh3Timer = 0;
    this.pregainCh4Timer = 0;
    
    this.eqValueStorage = [];

    this.chTimer = [];
    for (var chidx = 1; chidx <= 4; chidx++) {
        var ch = "[Channel" + chidx + "]";

        this.displayBrightness[ch] = LedDimmed;

        this.chTimer[ch] = [];
        for (var timerIdx = 1; timerIdx <= 5; timerIdx++) {
            this.chTimer[ch][timerIdx] = -1;
        }
    }
};

// Mixxx's javascript doesn't support .bind natively, so here's a simple version.
TraktorZ2.bind = function(fn, obj) {
    return function() {
        return fn.apply(obj, arguments);
    };
};

TraktorZ2.fxOnClickHandler = function(field) {
    HIDDebug("TraktorZ2: fxOnClickHandler");
    var numOfLoadedandEnabledEffects = 0;
    for (var effectIdx = 1; effectIdx <= engine.getValue(field.group, "num_effects"); effectIdx++) {
        if (engine.getValue(field.group.substr(0, field.group.length-1) + "_Effect" + effectIdx + "]", "loaded") === 1) {
            if (engine.getValue(field.group.substr(0, field.group.length-1) + "_Effect" + effectIdx + "]", "enabled") === 1) {
                numOfLoadedandEnabledEffects++;
            }
        }
    }

    if (field.value !== 0) {
        if (numOfLoadedandEnabledEffects === 0) {
            for (effectIdx = 1; effectIdx <= engine.getValue(field.group, "num_effects"); effectIdx++) {
                if (engine.getValue(field.group.substr(0, field.group.length-1) + "_Effect" + effectIdx + "]", "loaded") === 1) {
                    engine.setValue(field.group.substr(0, field.group.length-1) + "_Effect" + effectIdx + "]", "enabled", 1);
                }
            }
        } else
            for (effectIdx = 1; effectIdx <= engine.getValue(field.group, "num_effects"); effectIdx++) {
                engine.setValue(field.group.substr(0, field.group.length-1) + "_Effect" + effectIdx + "]", "enabled", 0);
            }
    }
};

TraktorZ2.fxOnLedHandler = function() {
    HIDDebug("TraktorZ2: fxOnLedHandler");
    for (var macroFxUnitIdx = 1; macroFxUnitIdx <= 2; macroFxUnitIdx++) {
        var numOfLoadedButDisabledEffects = 0;
        var numOfLoadedandEnabledEffects = 0;
        for (var effectIdx = 1; effectIdx <= engine.getValue("[EffectRack1_EffectUnit" + macroFxUnitIdx +"]", "num_effects"); effectIdx++) {
            if (engine.getValue("[EffectRack1_EffectUnit" + macroFxUnitIdx +"_Effect" + effectIdx + "]", "loaded") === 1) {
                if (engine.getValue("[EffectRack1_EffectUnit" + macroFxUnitIdx +"_Effect" + effectIdx + "]", "enabled") === 1) {
                    numOfLoadedandEnabledEffects++;
                } else {
                    numOfLoadedButDisabledEffects++;
                }
            }
        }
        if (numOfLoadedandEnabledEffects === 0) {
            TraktorZ2.controller.setOutput("[EffectRack1_EffectUnit" + macroFxUnitIdx +"]", "!On", LedOff,    macroFxUnitIdx === 2);
        } else if (numOfLoadedandEnabledEffects > 0 && numOfLoadedButDisabledEffects > 0) {
            TraktorZ2.controller.setOutput("[EffectRack1_EffectUnit" + macroFxUnitIdx +"]", "!On", LedDimmed, macroFxUnitIdx === 2);
        } else {
            TraktorZ2.controller.setOutput("[EffectRack1_EffectUnit" + macroFxUnitIdx +"]", "!On", LedBright, macroFxUnitIdx === 2);
        }
    }
};

//// Deck Objects ////
// Decks are the loop controls and the 4 hotcue buttons on either side of the controller.
// Each Deck can control 2 channels a + c and b + d, which can be mapped.
TraktorZ2.Deck = function(deckNumber, group) {
    this.deckNumber = deckNumber;
    this.group = group;
    this.activeChannel = "[Channel" + deckNumber + "]";

    // Various states
    TraktorZ2.syncPressedTimer = 0;
    TraktorZ2.vinylcontrolTimer = 0;

    // Knob encoder states (hold values between 0x0 and 0xF)
    // Rotate to the right is +1 and to the left is means -1
    this.loopKnobEncoderState = 0;
};

TraktorZ2.Deck.prototype.registerInputs = function(messageShort) {
    HIDDebug("TraktorZ2: Deck.prototype.registerInputs");
    var deckFn = TraktorZ2.Deck.prototype;

    this.defineButton(messageShort, "!pad_1", 0x06, 0x04, 0x07, 0x08, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_2", 0x06, 0x08, 0x07, 0x10, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_3", 0x06, 0x10, 0x07, 0x20, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_4", 0x06, 0x20, 0x07, 0x40, deckFn.numberButtonHandler);

    // Vinyl control mode (REL / INTL)
    this.defineButton(messageShort, "vinylcontrol_mode", 0x04, 0x10, 0x04, 0x20, deckFn.vinylcontrolHandler);
    this.defineButton(messageShort, "!sync", 0x04, 0x40, 0x04, 0x80, deckFn.syncHandler);

    // Load/Duplicate buttons
    this.defineButton(messageShort, "!LoadSelectedTrack", 0x04, 0x01, 0x04, 0x02, deckFn.loadTrackHandler);

    // Loop control
    this.defineButton(messageShort, "!SelectLoop", 0x01, 0xF0, 0x02, 0x0F, deckFn.selectLoopHandler);
    this.defineButton(messageShort, "!ActivateLoop", 0x05, 0x40, 0x08, 0x20, deckFn.activateLoopHandler);

    // Flux / Tap
    this.defineButton(messageShort, "!slip_enabled", 0x06, 0x40, 0x07, 0x80, deckFn.fluxHandler);

};

TraktorZ2.deckSwitchHandler = function(field) {
    HIDDebug("TraktorZ2: deckSwitchHandler: "+field.group + " " +field.value);
    if (field.value === 1) {
        if (field.group === "[Channel1]") {
            TraktorZ2.controller.setOutput("[Channel3]", "!deck", LedOff,       true);
            TraktorZ2.deckSwitchHandler["[Channel3]"] = 0;
        } else if (field.group === "[Channel2]") {
            TraktorZ2.controller.setOutput("[Channel4]", "!deck", LedOff,       true);
            TraktorZ2.deckSwitchHandler["[Channel4]"] = 0;
        } else if (field.group === "[Channel3]") {
            TraktorZ2.controller.setOutput("[Channel1]", "!deck", LedOff,       true);
            TraktorZ2.deckSwitchHandler["[Channel1]"] = 0;
        } else if (field.group === "[Channel4]") {
            TraktorZ2.controller.setOutput("[Channel2]", "!deck", LedOff,       true);
            TraktorZ2.deckSwitchHandler["[Channel2]"] = 0;
        }

        if (TraktorZ2.deckSwitchHandler[field.group] !== 1) {
            TraktorZ2.deckSwitchHandler[field.group] = 1;
            TraktorZ2.controller.setOutput(field.group, "!deck", LedBright,       true);

        } else if (engine.getValue("[Skin]", "show_8_hotcues")) {
            TraktorZ2.deckSwitchHandler[field.group] = 2;
            TraktorZ2.controller.setOutput(field.group, "!deck", LedDimmed,       true);
        }
        TraktorZ2.hotcueOutputHandler(); // Set new hotcue button colors
    }
};

TraktorZ2.Deck.prototype.numberButtonHandler = function(field) {

    var sideChannel = ["[Channel1]", "[Channel2]"];
    var sideOffset= [0, 0];

    if (TraktorZ2.deckSwitchHandler["[Channel1]"] === 2) {
        sideChannel[1] = "[Channel1]";
        sideOffset[1] = 4; // Second 4 hotcues mapped to the pads
    } else if (TraktorZ2.deckSwitchHandler["[Channel3]"] === 1) {
        sideChannel[1] = "[Channel3]";
        sideOffset[1] = 0; // First 4 hotcues mapped to the pads
    } else if (TraktorZ2.deckSwitchHandler["[Channel3]"] === 2) {
        sideChannel[1] = "[Channel3]";
        sideOffset[1] = 4; // Second 4 hotcues mapped to the pads
    } else {
        sideChannel[1] = "[Channel1]";
        sideOffset[1] = 0; // First 4 hotcues mapped to the pads
    }
    if (TraktorZ2.deckSwitchHandler["[Channel2]"] === 2) {
        sideChannel[2] = "[Channel2]";
        sideOffset[2] = 4; // Second 4 hotcues mapped to the pads
    } else if (TraktorZ2.deckSwitchHandler["[Channel4]"] === 1) {
        sideChannel[2] = "[Channel4]";
        sideOffset[2] = 0; // First 4 hotcues mapped to the pads
    } else if (TraktorZ2.deckSwitchHandler["[Channel4]"] === 2) {
        sideChannel[2] = "[Channel4]";
        sideOffset[2] = 4; // Second 4 hotcues mapped to the pads
    } else {
        sideChannel[2] = "[Channel2]";
        sideOffset[2] = 0; // First 4 hotcues mapped to the pads
    }
    var chIdx;
    if (this.activeChannel === "[Channel1]") {
        chIdx = 1;
    } else {
        chIdx = 2;
    }

    var padNumber = parseInt(field.id[field.id.length - 1]);
    var action = "";

    // Hotcues mode clear when shift button is active pressed
    if (TraktorZ2.shiftState & 0x01) {
        action = "_clear";
    } else {
        action = "_activate";
    }
    HIDDebug("setting " + "hotcue_" + padNumber + action + " " + field.value);
    engine.setValue(sideChannel[chIdx], "hotcue_" + (sideOffset[chIdx] + padNumber) + action, field.value);
    return;
};

TraktorZ2.Deck.prototype.fluxHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    script.toggleControl(this.activeChannel, "slip_enabled");
};

TraktorZ2.Deck.prototype.vinylcontrolHandler = function(field) {
    HIDDebug("TraktorZ2: vinylcontrolHandler" + " this.activeChannel:" + this.activeChannel + " field:" + field);
    var vinylControlMode = engine.getValue(this.activeChannel, "vinylcontrol_mode");
    var ch = this.activeChannel; // Use global variable in timer function, because this.activeChannel can change until the timer is active
    TraktorZ2.vinylcontrolTimer = engine.beginTimer(300, function() {
        if (vinylControlMode >= 2) {
            vinylControlMode = 0;
        } else {
            vinylControlMode++;
        }
        engine.setValue(ch, "vinylcontrol_mode", vinylControlMode);
        // Reset vinylcontrol button timer state if active
        if (TraktorZ2.vinylcontrolTimer !== 0) {
            TraktorZ2.vinylcontrolTimer = 0;
        }
    }, true);
};


TraktorZ2.vinylcontrolOutputHandler = function() {
    HIDDebug("TraktorZ2: vinylcontrolOutputHandler");
    for (var chidx = 1; chidx <= 2; chidx++) {
        var ch = "[Channel" + chidx + "]";
        if (engine.getValue(ch, "vinylcontrol_status") === 0) {
            // Vinyl control disabled
            TraktorZ2.controller.setOutput(ch, "!vinylcontrol_green", LedOff, chidx === 2);
            TraktorZ2.controller.setOutput(ch, "!vinylcontrol_orange", LedOff, chidx === 2);
        } else if (engine.getValue(ch, "vinylcontrol_status") === 1) {
            // Vinyl control signal
            TraktorZ2.controller.setOutput(ch, "!vinylcontrol_green", LedBright, chidx === 2);
            TraktorZ2.controller.setOutput(ch, "!vinylcontrol_orange", LedOff, chidx === 2);
        } else if (engine.getValue(ch, "vinylcontrol_status") === 2) {
            // Track end section of control vinyl
            TraktorZ2.controller.setOutput(ch, "!vinylcontrol_green", LedOff, chidx === 2);
            TraktorZ2.controller.setOutput(ch, "!vinylcontrol_orange", LedBright, chidx === 2);
        } else {
            // Passthrough
            // Track end section of control vinyl
            TraktorZ2.controller.setOutput(ch, "!vinylcontrol_green", LedDimmed, chidx === 2);
            TraktorZ2.controller.setOutput(ch, "!vinylcontrol_orange", LedDimmed, chidx === 2);
        }
    }
};

TraktorZ2.Deck.prototype.syncHandler = function(field) {
    HIDDebug("TraktorZ2: syncHandler" + " this.activeChannel:" + this.activeChannel + " field:" + field);
    if (TraktorZ2.shiftState & 0x01) {
        engine.setValue(this.activeChannel, "beatsync_phase", field.value);
        // Light LED while pressed
        //this.colorOutput(field.value, "sync_enabled");
        return;
    }

    // Unshifted
    if (field.value) {
        // We have to reimplement push-to-lock because it's only defined in the midi code
        // in Mixxx.
        if (engine.getValue(this.activeChannel, "sync_enabled") === 0) {
            script.triggerControl(this.activeChannel, "beatsync");
            // Start timer to measure how long button is pressed
            var ch = this.activeChannel; // Use global variable in timer function, because this.activeChannel can change until the timer is active
            TraktorZ2.syncPressedTimer = engine.beginTimer(300, function() {
                engine.setValue(ch, "sync_enabled", 1);
                // Reset sync button timer state if active
                if (TraktorZ2.syncPressedTimer !== 0) {
                    TraktorZ2.syncPressedTimer = 0;
                }
            }, true);

            // Light corresponding LED when button is pressed
            //this.colorOutput(1, "sync_enabled");
        } else {
            // Deactivate sync lock
            // LED is turned off by the callback handler for sync_enabled
            engine.setValue(this.activeChannel, "sync_enabled", 0);
        }
    } else {
        if (TraktorZ2.syncPressedTimer !== 0) {
            // Timer still running -> stop it and unlight LED
            engine.stopTimer(TraktorZ2.syncPressedTimer);
            //this.colorOutput(0, "sync_enabled");
        }
    }
};

TraktorZ2.selectTrackHandler = function(field) {
    HIDDebug("TraktorZ2: selectTrackHandler");
    var delta = 1;
    if ((field.value + 1) % 16 === TraktorZ2.browseKnobEncoderState) {
        delta = -1;
    }
    TraktorZ2.browseKnobEncoderState = field.value;

    // If shift mode is locked
    if (TraktorZ2.shiftState === 0x02) {
        engine.setValue("[Library]", "MoveHorizontal", delta);
    } else {
        engine.setValue("[Library]", "MoveVertical", delta);
    }
};

TraktorZ2.LibraryFocusHandler = function(field) {
    HIDDebug("TraktorZ2: LibraryFocusHandler");
    if (field.value) {
        // If shift mode is locked
        if (TraktorZ2.shiftState === 0x02) {
            engine.setValue("[Library]", "sort_column_toggle", 0);
        } else {
            engine.setValue("[Library]", "MoveFocusForward", true);
        }
    }
};

TraktorZ2.Deck.prototype.loadTrackHandler = function(field) {
    // If shift mode is locked or active pressed
    if (TraktorZ2.shiftState) {
        if (this.activeChannel === "[Channel1]") {
            engine.setValue("[Channel1]", "CloneFromDeck", 2);
        } else if (this.activeChannel === "[Channel2]") {
            engine.setValue("[Channel2]", "CloneFromDeck", 1);
        }
    } else {
        engine.setValue(this.activeChannel, "LoadSelectedTrack", field.value);
    }
};


// defineButton allows us to configure either the right deck or the left deck, depending on which
// is appropriate.  This avoids extra logic in the function where we define all the magic numbers.
// We use a similar approach in the other define funcs.
TraktorZ2.Deck.prototype.defineButton = function(msg, name, deckOffset, deckBitmask, deck2Offset, deck2Bitmask, fn) {
    if (this.deckNumber === 2) {
        deckOffset = deck2Offset;
        deckBitmask = deck2Bitmask;
    }
    TraktorZ2.registerInputButton(msg, this.group, name, deckOffset, deckBitmask, TraktorZ2.bind(fn, this));
};

TraktorZ2.Deck.prototype.selectLoopHandler = function(field) {
    HIDDebug("TraktorZ2: selectLoopHandler");

    // If shift mode is locked
    if (TraktorZ2.shiftState === 0x02) {
        // Adjust beatjump size
        var beatjumpSize = engine.getValue(this.activeChannel, "beatjump_size");
        if ((field.value + 1) % 16  === this.moveKnobEncoderState) {
            engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize * 2);
        } else {
            engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize / 2);
        }
        // } else {
        // if (delta < 0) {
        // script.triggerControl(this.activeChannel, "beatjump_backward");
        // } else {
        // script.triggerControl(this.activeChannel, "beatjump_forward");
        // }
        // }

        this.moveKnobEncoderState = field.value;
    } else {
        // Adjust loop size
        if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
            script.triggerControl(this.activeChannel, "loop_halve");
        } else {
            script.triggerControl(this.activeChannel, "loop_double");
        }
        this.loopKnobEncoderState = field.value;

        TraktorZ2.displayLoopCount(this.activeChannel);
    }
};

TraktorZ2.Deck.prototype.activateLoopHandler = function(field) {
    HIDDebug("TraktorZ2: activateLoopHandler");
    if (field.value === 1) {
        var isLoopActive = engine.getValue(this.activeChannel, "loop_enabled");

        // Shift state ??
        if (TraktorZ2.shiftState) {
            engine.setValue(this.activeChannel, "reloop_toggle", field.value);
        } else {
            if (isLoopActive) {
                engine.setValue(this.activeChannel, "reloop_toggle", field.value);
            } else {
                engine.setValue(this.activeChannel, "beatloop_activate", field.value);
            }
        }
    }
    TraktorZ2.displayLoopCount(this.activeChannel);
};

TraktorZ2.crossfaderReverseHandler = function(field) {
    HIDDebug("TraktorZ2: LibraryFocusHandler");
    if (field.value) {
        TraktorZ2.controller.setOutput("[Master]", "!crossfaderReverse", LedBright,  true);
        if (engine.getValue("[Channel1]", "orientation") === 0) {engine.setValue("[Channel1]", "orientation", 2);}
        if (engine.getValue("[Channel3]", "orientation") === 0) {engine.setValue("[Channel3]", "orientation", 2);}
        if (engine.getValue("[Channel2]", "orientation") === 2) {engine.setValue("[Channel2]", "orientation", 0);}
        if (engine.getValue("[Channel4]", "orientation") === 2) {engine.setValue("[Channel4]", "orientation", 0);}
        
    } else {
        TraktorZ2.controller.setOutput("[Master]", "!crossfaderReverse", LedOff,  true);
        if (engine.getValue("[Channel1]", "orientation") === 2) {engine.setValue("[Channel1]", "orientation", 0);}
        if (engine.getValue("[Channel3]", "orientation") === 2) {engine.setValue("[Channel3]", "orientation", 0);}
        if (engine.getValue("[Channel2]", "orientation") === 0) {engine.setValue("[Channel2]", "orientation", 2);}
        if (engine.getValue("[Channel4]", "orientation") === 0) {engine.setValue("[Channel4]", "orientation", 2);}
    }
};

TraktorZ2.buttonHandler = function(field) {
    HIDDebug("TraktorZ2: buttonHandler");
    if (field.value === 0) {
        return;
    }
    script.toggleControl(field.group, field.name);
};

TraktorZ2.traktorbuttonHandler = function(field) {
    HIDDebug("TraktorZ2: traktorbuttonHandler" + " field: " + field + " field.value: " + field.value);
    if (field.value === 1) {
        TraktorZ2.controller.setOutput(field.group, "traktorbutton", LedBright,  true); // Controller internal state ON -> Switch LED to represent this state
    } else {
        TraktorZ2.controller.setOutput(field.group, "traktorbutton", LedOff,     true); // Controller internal state OFF -> Switch LED to represent this state
    }
};

TraktorZ2.registerInputPackets = function() {
    var messageShort = new HIDPacket("shortmessage", 0x01, this.messageCallback);

    HIDDebug("TraktorZ2: registerInputPackets");
    for (var idx in TraktorZ2.Decks) {
        var deck = TraktorZ2.Decks[idx];
        deck.registerInputs(messageShort);
    }

    this.registerInputButton(messageShort, "[Channel1]", "switchDeck", 0x06, 0x02, this.deckSwitchHandler);
    this.registerInputButton(messageShort, "[Channel2]", "switchDeck", 0x07, 0x02, this.deckSwitchHandler);
    this.registerInputButton(messageShort, "[Channel3]", "switchDeck", 0x06, 0x01, this.deckSwitchHandler);
    this.registerInputButton(messageShort, "[Channel4]", "switchDeck", 0x07, 0x04, this.deckSwitchHandler);

    // this.registerInputButton(messageShort, "[Channel1]", "!traktorbutton", 0x03, 0x01, this.traktorbuttonHandler);
    // this.registerInputButton(messageShort, "[Channel2]", "!traktorbutton", 0x03, 0x02, this.traktorbuttonHandler);

    this.registerInputButton(messageShort, "[Channel1]", "!traktorbutton", 0x09, 0x08, this.traktorbuttonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "!traktorbutton", 0x09, 0x10, this.traktorbuttonHandler);

    this.registerInputButton(messageShort, "[Master]", "skin_settings", 0x03, 0x08, this.buttonHandler);

    this.registerInputButton(messageShort, "[Channel1]", "quantize", 0x03, 0x04, this.buttonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "quantize", 0x03, 0x10, this.buttonHandler);

    // Mic button
    this.registerInputButton(messageShort, "[Microphone]", "talkover", 0x05, 0x01, this.buttonHandler);

    // Headphone buttons
    this.registerInputButton(messageShort, "[Channel1]", "pfl", 0x04, 0x04, this.buttonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "pfl", 0x04, 0x08, this.buttonHandler);

    this.registerInputButton(messageShort, "[Master]", "shift", 0x07, 0x01, this.shiftHandler);

    this.registerInputButton(messageShort, "[Master]", "!SelectTrack", 0x01, 0x0F, this.selectTrackHandler);
    this.registerInputButton(messageShort, "[Master]", "!LibraryFocus", 0x03, 0x80, this.LibraryFocusHandler);


    this.registerInputButton(messageShort, "[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x05, 0x04, this.buttonHandler);
    this.registerInputButton(messageShort, "[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x05, 0x08, this.buttonHandler);
    this.registerInputButton(messageShort, "[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x08, 0x02, this.buttonHandler);
    this.registerInputButton(messageShort, "[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x08, 0x04, this.buttonHandler);

    this.registerInputButton(messageShort, "[EffectRack1_EffectUnit1]", "!enabled", 0x05, 0x02, this.fxOnClickHandler);
    this.registerInputButton(messageShort, "[EffectRack1_EffectUnit2]", "!enabled", 0x08, 0x01, this.fxOnClickHandler);
        
    this.registerInputButton(messageShort, "Master", "!crossfaderReverse", 0x08, 0x80, this.crossfaderReverseHandler);


    this.controller.registerInputPacket(messageShort);


    var messageLong = new HIDPacket("longmessage", 0x02, this.messageCallback);

    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit1]", "mix", 0x0D, 0xFFFF, this.parameterHandler); // MACRO FX1 D/W
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit1]", "super1", 0x0F, 0xFFFF, this.parameterHandler); // MACRO FX1 FX
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit2]", "mix", 0x1B, 0xFFFF, this.parameterHandler); // MACRO FX2 D/W
    this.registerInputScaler(messageLong, "[EffectRack1_EffectUnit2]", "super1", 0x1D, 0xFFFF, this.parameterHandler); // MACRO FX2 FX

    this.registerInputScaler(messageLong, "[Channel1]", "volume", 0x2D, 0xFFFF, this.faderHandler); // Fader Deck A
    this.registerInputScaler(messageLong, "[Channel2]", "volume", 0x2F, 0xFFFF, this.faderHandler); // Fader Deck B

    this.registerInputScaler(messageLong, "[Master]", "duckStrengh", 0x03, 0xFFFF, this.parameterHandler); // Mic/Aux Tone knob, where no 1:1 mapping is available
    this.registerInputScaler(messageLong, "[Microphone]", "pregain", 0x01, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Channel1]", "pregain", 0x11, 0xFFFF, this.pregainHandler); // Rotary knob Deck A
    this.registerInputScaler(messageLong, "[Channel2]", "pregain", 0x1F, 0xFFFF, this.pregainHandler); // Rotary knob Deck B
    this.registerInputScaler(messageLong, "[Channel3]", "pregain", 0x29, 0xFFFF, this.pregainHandler); // Rotary knob Deck C
    this.registerInputScaler(messageLong, "[Channel4]", "pregain", 0x2B, 0xFFFF, this.pregainHandler); // Rotary knob Deck D

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x13, 0xFFFF, this.eqKnobHandler); // High
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x15, 0xFFFF, this.eqKnobHandler); // Mid
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x17, 0xFFFF, this.eqKnobHandler); // Low

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x21, 0xFFFF, this.eqKnobHandler); // High
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x23, 0xFFFF, this.eqKnobHandler); // Mid
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x25, 0xFFFF, this.eqKnobHandler); // Low

    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel1]]", "super1", 0x19, 0xFFFF, this.eqKnobHandler);
    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel2]]", "super1", 0x27, 0xFFFF, this.eqKnobHandler);

    this.registerInputScaler(messageLong, "[Master]", "crossfader", 0x31, 0xFFFF, this.faderHandler);
    this.registerInputScaler(messageLong, "[Master]", "gain", 0x09, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headMix", 0x07, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headGain", 0x05, 0xFFFF, this.parameterHandler);

    this.controller.registerInputPacket(messageLong);

    // Soft takeovers
    for (var ch = 1; ch <= 2; ch++) {
        var group = "[Channel" + ch + "]";
        engine.softTakeover("[QuickEffectRack1_" + group + "]", "super1", true);
    }

    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", true);

    // engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[Master]", "gain", true);
    engine.softTakeover("[Master]", "headMix", true);
    engine.softTakeover("[Master]", "headGain", true);

    // Dirty hack to set initial values in the packet parser
    var data = [0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
        0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
        0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
        0, 0, 0, 0,   0];
    TraktorZ2.incomingData(data);
};

TraktorZ2.registerInputScaler = function(message, group, name, offset, bitmask, callback) {
    HIDDebug("TraktorZ2: registerInputScaler");
    message.addControl(group, name, offset, "H", bitmask);
    message.setCallback(group, name, callback);
};

TraktorZ2.registerInputButton = function(message, group, name, offset, bitmask, callback) {
    HIDDebug("TraktorZ2: registerInputButton");
    message.addControl(group, name, offset, "B", bitmask);
    message.setCallback(group, name, callback);
};

TraktorZ2.shiftHandler = function(field) {
    HIDDebug("TraktorZ2: shiftHandler");

    // This function sets TraktorZ2.shiftState as follows:
    // 0x00: shift mode off / and not active pressed
    // 0x01: shift mode off / but active pressed
    // 0x02: shift mode on  / and not active pressed
    // 0x03: shift mode on  / and active pressed

    if (TraktorZ2.shiftPressed === false && field.value === 1) {
        TraktorZ2.shiftPressed = true;
        TraktorZ2.shiftState |= 0x01;
        TraktorZ2.controller.setOutput("[Master]", "shift",  LedBright,  true);

        TraktorZ2.shiftPressedTimer = engine.beginTimer(200, function() {
            // Reset sync button timer state if active
            if (TraktorZ2.shiftPressedTimer !== 0) {
                TraktorZ2.shiftPressedTimer = 0;
            }
            HIDDebug("TraktorZ2: shift unlocked");
        }, true);

        HIDDebug("TraktorZ2: shift pressed");
    } else if (TraktorZ2.shiftPressed === true && field.value === 0) {

        TraktorZ2.shiftPressed = false;

        HIDDebug("TraktorZ2: shift button released"  + TraktorZ2.shiftState);
        if (TraktorZ2.shiftPressedTimer !== 0) {
            if (TraktorZ2.shiftState & 0x02) {
                // Timer still running -> stop it and set LED depending on previous lock state
                TraktorZ2.shiftState = 0x00;
                TraktorZ2.controller.setOutput("[Master]", "shift",  LedOff,  true);
            } else {
                TraktorZ2.shiftState = 0x02;
                TraktorZ2.controller.setOutput("[Master]", "shift",  LedDimmed,  true);
            }
            engine.stopTimer(TraktorZ2.shiftPressedTimer);
            HIDDebug("TraktorZ2: static shift state changed to: "  + TraktorZ2.shiftState);
        } else {
            if (TraktorZ2.shiftState & 0x02) {
                TraktorZ2.shiftState = 0x02;
                TraktorZ2.controller.setOutput("[Master]", "shift",  LedDimmed,  true);
            } else {
                TraktorZ2.shiftState = 0x00;
                TraktorZ2.controller.setOutput("[Master]", "shift",  LedOff,  true);
            }
            HIDDebug("TraktorZ2: back to static shift state: " + TraktorZ2.shiftState);
        }
        // Apply stored EQ and filter settings
         var eqGroups = {
            "1" : "[EqualizerRack1_[Channel1]_Effect1]",
            "2" : "[EqualizerRack1_[Channel1]_Effect1]",
            "3" : "[EqualizerRack1_[Channel1]_Effect1]",
            "4" : "[QuickEffectRack1_[Channel1]]",
            "5" : "[EqualizerRack1_[Channel2]_Effect1]",
            "6" : "[EqualizerRack1_[Channel2]_Effect1]",
            "7" : "[EqualizerRack1_[Channel2]_Effect1]",
            "8" : "[QuickEffectRack1_[Channel2]]"
        };
        var eqParameters = {
            "1" : "parameter1",
            "2" : "parameter2",
            "3" : "parameter3",
            "4" : "super1",
            "5" : "parameter1",
            "6" : "parameter2",
            "7" : "parameter3",
            "8" : "super1"
        };

        for (var idx in eqGroups) {
            
            if (TraktorZ2.eqValueStorage[eqGroups[idx] + eqParameters[idx] + "changed"] === true) {
                TraktorZ2.eqExecute(eqGroups[idx], eqParameters[idx], TraktorZ2.eqValueStorage[eqGroups[idx] + eqParameters[idx] + "value"]);
            }
        }
    }
};

TraktorZ2.parameterHandler = function(field) {
    HIDDebug("TraktorZ2: parameterHandler");
    engine.setParameter(field.group, field.name, field.value / 4095);
};

TraktorZ2.eqKnobHandler = function(field) {
    HIDDebug("TraktorZ2: eqKnobHandler");
    
    if (TraktorZ2.shiftPressed === false) {
        TraktorZ2.eqExecute(field.group, field.name, field.value);
    } else {
        // Store value until Shift button will be released
        TraktorZ2.eqValueStorage[field.group + field.name + "changed"] = true;
        TraktorZ2.eqValueStorage[field.group + field.name + "value"] = field.value;
    }
}

TraktorZ2.eqExecute = function(group, name, value) {
    HIDDebug("TraktorZ2: eqExecute");
    if ((group === "[EqualizerRack1_[Channel1]_Effect1]") ||
         (group === "[QuickEffectRack1_[Channel1]]")) {        
            engine.stopTimer(TraktorZ2.pregainCh3Timer);
            TraktorZ2.pregainCh3Timer = 0;
    } else if ((group === "[EqualizerRack1_[Channel2]_Effect1]") ||
        (group === "[QuickEffectRack1_[Channel2]]")) {        
            engine.stopTimer(TraktorZ2.pregainCh4Timer);
            TraktorZ2.pregainCh4Timer = 0;
    }
    engine.setParameter(group, name, value / 4095);
    TraktorZ2.eqValueStorage[group + name + "changed"] = false;
};

TraktorZ2.pregainHandler = function(field) {
    HIDDebug("TraktorZ2: pregainHandler");
    engine.setParameter(field.group, field.name, field.value / 4095);
    if ((field.group === "[Channel1]")  && (TraktorZ2.pregainCh3Timer !== 0)) {        
            engine.stopTimer(TraktorZ2.pregainCh3Timer);
            TraktorZ2.pregainCh3Timer = 0;
    }
    if (field.group === "[Channel3]") {  
        engine.stopTimer(TraktorZ2.pregainCh3Timer);
        TraktorZ2.pregainCh3Timer = engine.beginTimer(2500, function() {
            TraktorZ2.pregainCh3Timer = 0;
        }, true);
    }
    if ((field.group === "[Channel2]")  && (TraktorZ2.pregainCh4Timer !== 0)) {        
            engine.stopTimer(TraktorZ2.pregainCh4Timer);
            TraktorZ2.pregainCh4Timer = 0;
    }
    if (field.group === "[Channel4]") {  
        engine.stopTimer(TraktorZ2.pregainCh4Timer);
        TraktorZ2.pregainCh4Timer = engine.beginTimer(2500, function() {
            TraktorZ2.pregainCh4Timer = 0;
        }, true);
    }
};

TraktorZ2.faderHandler = function(field) {
    engine.setParameter(field.group, field.name, script.absoluteLin(field.value, 0, 1, 100, 3095));
};

TraktorZ2.messageCallback = function(_packet, data) {
    for (var name in data) {
        if (Object.prototype.hasOwnProperty.call(data, name)) {
            TraktorZ2.controller.processButton(data[name]);
        }
    }
};

TraktorZ2.incomingData = function(data, length) {
    TraktorZ2.controller.parsePacket(data, length);
};

TraktorZ2.shutdown = function() {

    // Switch software mixing mode of and given LED control to mixer hardware
    var data = [0x00, 0x40];
    controller.sendFeatureReport(data, 0xF1);

    data = [0xFF, 0x40];
    controller.sendFeatureReport(data, 0xF3);

    TraktorZ2.controller.setOutput("[Master]", "!usblight", LedBright, true);

    HIDDebug("TraktorZ2: Shutdown done!");
};

TraktorZ2.debugLights = function() {

    HIDDebug("TraktorZ2: debugLights");
    // Call this if you want to just send raw packets to the controller (good for figuring out what
    // bytes do what).
    var dataA = [
        /* 0x80 */
        0x00,  // 0x01 3 bits (0x40, 0x20, 0x10) control Warning Symbol on top left brightness (orange)
        0x00,  // 0x02 3 bits (0x40, 0x20, 0x10) control Timecode-Vinyl Symbol on top right brightness (orange)
        0x00,  // 0x03 3 bits (0x40, 0x20, 0x10) control Snap-Button S brightness (blue)
        0x00,  // 0x04 3 bits (0x40, 0x20, 0x10) control Quantize-Button Q brightness (blue)
        0x00,  // 0x05 3 bits (0x40, 0x20, 0x10) control Settings-Button (Gear-Wheel-Symbol) brightness (orange)
        0x00,  // 0x06 3 bits (0x40, 0x20, 0x10) control SHIFT-Button brightness (white)
        0x00,  // 0x07 3 bits (0x40, 0x20, 0x10) control Deck A button brightness (blue)
        0x00,  // 0x08 3 bits (0x40, 0x20, 0x10) control Deck B button brightness (blue)
        0x00,  // 0x09 3 bits (0x40, 0x20, 0x10) control Deck C button brightness (white)
        0x00,  // 0x0A 3 bits (0x40, 0x20, 0x10) control Deck D button brightness (white)

        0x00,  // 0x0B 3 bits (0x40, 0x20, 0x10) control Deck C volume text label backlight brightness (white)
        0x00,  // 0x0C 3 bits (0x40, 0x20, 0x10) control Deck D volume text label backlight brightness (white)

        0x00,  // 0x0D 3 bits (0x40, 0x20, 0x10) control Macro FX1 On button brightness (orange)
        0x00,  // 0x0E 3 bits (0x40, 0x20, 0x10) control Deck 1 Flux button brightness (orange)
        0x00,  // 0x0F 3 bits (0x40, 0x20, 0x10) control Channel 1 FX1 select button brightness (orange)
        0x00,  // 0x10 3 bits (0x40, 0x20, 0x10) control Channel 1 FX2 select button brightness (orange)
        0x00,  // 0x11 3 bits (0x40, 0x20, 0x10) control Load A button brightness (orange)
        0x70,  // 0x12 3 bits (0x40, 0x20, 0x10) control vinylcontrol Rel/Intl A button brightness (orange)
        0x00,  // 0x13 3 bits (0x40, 0x20, 0x10) control vinylcontrol Rel/Intl A button brightness (green)
        0x00,  // 0x14 3 bits (0x40, 0x20, 0x10) control vinylcontrol Sync A button brightness (orange)

        0x00,  // 0x15 3 bits (0x40, 0x20, 0x10) control Macro FX2 On button brightness (orange)
        0x00,  // 0x16 3 bits (0x40, 0x20, 0x10) control Deck 2 Flux button brightness (orange)
        0x00,  // 0x17 3 bits (0x40, 0x20, 0x10) control Channel 2 FX1 select button brightness (orange)
        0x00,  // 0x18 3 bits (0x40, 0x20, 0x10) control Channel 2 FX2 select button brightness (orange)
        0x00,  // 0x19 3 bits (0x40, 0x20, 0x10) control Load B button brightness (orange)
        0x70,  // 0x1A 3 bits (0x40, 0x20, 0x10) control vinylcontrol Rel/Intl B button brightness (orange)
        0x30,  // 0x1B 3 bits (0x40, 0x20, 0x10) control vinylcontrol Rel/Intl B button brightness (green)
        0x00,  // 0x1C 3 bits (0x40, 0x20, 0x10) control vinylcontrol Sync B button brightness (orange)
        0x00, 0x00, 0x00, // 0x1D HotCue 1 Deck 1 RGB
        0x00, 0x00, 0x00, // 0x20 HotCue 2 Deck 1 RGB
        0x00, 0x00, 0x00, // 0x23 HotCue 3 Deck 1 RGB
        0x00, 0x00, 0x00, // 0x26 HotCue 4 Deck 1 RGB
        0x00, 0x00, 0x00, // 0x29 HotCue 2 Deck 2 RGB
        0x00, 0x00, 0x00, // 0x2C HotCue 2 Deck 2 RGB
        0x00, 0x00, 0x00, // 0x2F HotCue 3 Deck 2 RGB
        0x00, 0x00, 0x00, // 0x32 HotCue 4 Deck 2 RGB

        0x00,  // 0x35 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x36 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x37 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x38 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x39 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x3A 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x3B 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x3C 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x3D 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x3E 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x3F 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x40 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x41 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x42 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x43 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x44 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x45 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x46 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x47 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x48 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x49 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x4A 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x4B 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x4C 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x4D 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x4E 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x4F 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x50 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x51 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x52 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x53 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x54 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x55 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x56 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x57 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x58 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x59 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x5A 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x5B 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x5C 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x5D 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment lower left vertical bar brightness (orange)
        0x00   // 0x5E 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment lower horizontal bar brightness (orange)
    ];
    controller.send(dataA, dataA.length, 0x80);

    var dataB = [
        /* 0x81 */
        0x00,  // 0x01 3 bits (0x40, 0x20, 0x10) control VU meter label "A"  (white)
        0x00,  // 0x02 3 bits (0x40, 0x20, 0x10) control VU meter -15dBa ChA (blue)
        0x00,  // 0x03 3 bits (0x40, 0x20, 0x10) control VU meter  -6dBa ChA (blue)
        0x00,  // 0x04 3 bits (0x40, 0x20, 0x10) control VU meter  -3dBa ChA (blue)
        0x00,  // 0x05 3 bits (0x40, 0x20, 0x10) control VU meter   0dBa ChA (blue)
        0x00,  // 0x06 3 bits (0x40, 0x20, 0x10) control VU meter  +3dBa ChA (orange)
        0x00,  // 0x07 3 bits (0x40, 0x20, 0x10) control VU meter  +6dBa ChA (orange)
        0x00,  // 0x08 3 bits (0x40, 0x20, 0x10) control VU meter   CLIP ChA (orange)

        0x00,  // 0x09 3 bits (0x40, 0x20, 0x10) control VU meter label "B"  (white)
        0x00,  // 0x0A 3 bits (0x40, 0x20, 0x10) control VU meter -15dBa ChB (blue)
        0x00,  // 0x0B 3 bits (0x40, 0x20, 0x10) control VU meter  -6dBa ChB (blue)
        0x00,  // 0x0C 3 bits (0x40, 0x20, 0x10) control VU meter  -3dBa ChB (blue)
        0x00,  // 0x0D 3 bits (0x40, 0x20, 0x10) control VU meter   0dBa ChB (blue)
        0x00,  // 0x0E 3 bits (0x40, 0x20, 0x10) control VU meter  +3dBa ChB (orange)
        0x00,  // 0x0F 3 bits (0x40, 0x20, 0x10) control VU meter  +6dBa ChB (orange)
        0x00,  // 0x10 3 bits (0x40, 0x20, 0x10) control VU meter   CLIP ChB (orange)

        0x00,  // 0x11 3 bits (0x40, 0x20, 0x10) control VU meter label "C"  (white)
        0x00,  // 0x12 3 bits (0x40, 0x20, 0x10) control VU meter -15dBa ChC/MasterLeft (blue)
        0x00,  // 0x13 3 bits (0x40, 0x20, 0x10) control VU meter  -6dBa ChC/MasterLeft (blue)
        0x00,  // 0x14 3 bits (0x40, 0x20, 0x10) control VU meter  -3dBa ChC/MasterLeft (blue)
        0x00,  // 0x15 3 bits (0x40, 0x20, 0x10) control VU meter   0dBa ChC/MasterLeft (blue)
        0x00,  // 0x16 3 bits (0x40, 0x20, 0x10) control VU meter  +3dBa ChC/MasterLeft (orange)
        0x00,  // 0x17 3 bits (0x40, 0x20, 0x10) control VU meter  +6dBa ChC/MasterLeft (orange)
        0x00,  // 0x18 3 bits (0x40, 0x20, 0x10) control VU meter   CLIP ChC/MasterLeft (orange)

        0x00,  // 0x19 3 bits (0x40, 0x20, 0x10) control VU meter label "D"  (white)
        0x00,  // 0x1A 3 bits (0x40, 0x20, 0x10) control VU meter -15dBa ChD/MasterRight (blue)
        0x00,  // 0x1B 3 bits (0x40, 0x20, 0x10) control VU meter  -6dBa ChD/MasterRight (blue)
        0x00,  // 0x1C 3 bits (0x40, 0x20, 0x10) control VU meter  -3dBa ChD/MasterRight (blue)
        0x00,  // 0x1D 3 bits (0x40, 0x20, 0x10) control VU meter   0dBa ChD/MasterRight (blue)
        0x00,  // 0x1E 3 bits (0x40, 0x20, 0x10) control VU meter  +3dBa ChD/MasterRight (orange)
        0x00,  // 0x1F 3 bits (0x40, 0x20, 0x10) control VU meter  +6dBa ChD/MasterRight (orange)
        0x00,  // 0x20 3 bits (0x40, 0x20, 0x10) control VU meter   CLIP ChD/MasterRight (orange)

        0x00,  // 0x21 3 bits (0x40, 0x20, 0x10) control VU meter label "MST"  (white)
        0x00,  // 0x22 3 bits (0x40, 0x20, 0x10) control Microphone-Button (orange)
        0x00,  // 0x23 3 bits (0x40, 0x20, 0x10) control Headphone-Button A (blue)
        0x00,  // 0x24 3 bits (0x40, 0x20, 0x10) control Headphone-Button B (blue)
        0x00,  // 0x25 3 bits (0x40, 0x20, 0x10) control Traktor-Button ChA (orange)
        0x00,  // 0x26 3 bits (0x40, 0x20, 0x10) control Traktor-Button ChB (orange)
        0x00,  // 0x27 3 bits (0x40, 0x20, 0x10) control USB-symbol on top (orange)
        0x00   // 0x28 3 bits (0x40, 0x20, 0x10) control VU meter label "XF REVERSE" (orange)
    ];
    controller.send(dataB, dataB.length, 0x81);

};


// outputHandler drives lights that only have one color.
TraktorZ2.basicOutputHandler = function(value, group, key) {
    var ledValue = value;
    if (value === 0 || value === false) {
        // Off value
        ledValue = LedOff;
    } else if (value === 1 || value === true) {
        // On value
        ledValue = LedBright;
    }

    TraktorZ2.controller.setOutput(group, key, ledValue, true);
};


TraktorZ2.hotcueOutputHandler = function() {

    var sideChannel = ["[Channel1]", "[Channel2]"];
    var sideOffset= [0, 0];

    if (TraktorZ2.deckSwitchHandler["[Channel1]"] === 2) {
        sideChannel[1] = "[Channel1]";
        sideOffset[1] = 4; // Second 4 hotcues mapped to the pads
    } else if (TraktorZ2.deckSwitchHandler["[Channel3]"] === 1) {
        sideChannel[1] = "[Channel3]";
        sideOffset[1] = 0; // First 4 hotcues mapped to the pads
    } else if (TraktorZ2.deckSwitchHandler["[Channel3]"] === 2) {
        sideChannel[1] = "[Channel3]";
        sideOffset[1] = 4; // Second 4 hotcues mapped to the pads
    } else {
        sideChannel[1] = "[Channel1]";
        sideOffset[1] = 0; // First 4 hotcues mapped to the pads
    }
    if (TraktorZ2.deckSwitchHandler["[Channel2]"] === 2) {
        sideChannel[2] = "[Channel2]";
        sideOffset[2] = 4; // Second 4 hotcues mapped to the pads
    } else if (TraktorZ2.deckSwitchHandler["[Channel4]"] === 1) {
        sideChannel[2] = "[Channel4]";
        sideOffset[2] = 0; // First 4 hotcues mapped to the pads
    } else if (TraktorZ2.deckSwitchHandler["[Channel4]"] === 2) {
        sideChannel[2] = "[Channel4]";
        sideOffset[2] = 4; // Second 4 hotcues mapped to the pads
    } else {
        sideChannel[2] = "[Channel2]";
        sideOffset[2] = 0; // First 4 hotcues mapped to the pads
    }


    for (var chidx = 1; chidx <= 2; chidx++) {
        var ch = "[Channel" + chidx + "]";
        for (var i = 1; i <= 4; i++) {

            var colorCode = engine.getValue(sideChannel[chidx], "hotcue_" + (sideOffset[chidx] + i) + "_color");
            if (engine.getValue(sideChannel[chidx], "hotcue_" + (sideOffset[chidx] + i) + "_enabled") === 0) colorCode = 0;
            var red =   ((colorCode & 0xFF0000) >> 16) / 0x20;
            var green = ((colorCode & 0x00FF00) >>  8) / 0x20;
            var blue =  ((colorCode & 0x0000FF)) / 0x20;
            //HIDDebug("Channel: " + ch + " Hotcue: " + i + " Colorcode: " + colorCode + " Red: " + red + " Green: " + green + " Blue: " + blue);
            TraktorZ2.controller.setOutput(ch, "Hotcue" + i + "Red",   red,   false);
            TraktorZ2.controller.setOutput(ch, "Hotcue" + i + "Green", green, false);
            TraktorZ2.controller.setOutput(ch, "Hotcue" + i + "Blue",  blue,  true);
        }
    }
};

TraktorZ2.trackLoadedHandler = function(value, group) {
    if (value === 0) {
        // Stop runnig beat timer for the unloaded deck
        for (var timerIdx = 1; timerIdx <= 2; timerIdx++) {
            if (TraktorZ2.chTimer[group][timerIdx] !== -1) {
                engine.stopTimer(TraktorZ2.chTimer[group][timerIdx]);
            }
        }
        TraktorZ2.displayBrightness[group] = LedOff;
        TraktorZ2.displayBeatLeds(group);
    } else {
        TraktorZ2.displayBrightness[group] = 0x02;
        TraktorZ2.displayBeatLeds(group);
    }
}

TraktorZ2.beatOutputHandler = function(value, group) {
    if (value === 1) {
        for (var timerIdx = 1; timerIdx <= 2; timerIdx++) {
            if (TraktorZ2.chTimer[group][timerIdx] !== -1) {
                engine.stopTimer(TraktorZ2.chTimer[group][timerIdx]);
            }
        }

        TraktorZ2.displayBrightness[group] = 0x07; //LedBright
        TraktorZ2.displayBeatLeds(group);

        var beatPeriodMillis = 60 / engine.getValue(group, "bpm") * 1000;

        if (engine.getValue(group, "loop_enabled") && engine.getValue(group, "play_indicator") && (engine.getValue(group, "beatloop_size") < 1)) {
            // If beatloop_size is < 1, it can be, that the loop is in between two beats. Than beat_active will never trigger this function.
            var timerPeriodMillis = 0.5 * beatPeriodMillis * engine.getValue(group, "beatloop_size");
            if (timerPeriodMillis <= 20) {
                timerPeriodMillis = 20;
            }
            TraktorZ2.chTimer[group][2] = engine.beginTimer(timerPeriodMillis, function() {
                if (TraktorZ2.chTimer[group][1] === -1) {
                    if (TraktorZ2.displayBrightness[group] === 0x02) {
                        TraktorZ2.displayBrightness[group] = 0x07;
                        TraktorZ2.displayBeatLeds(group);
                    } else {
                        TraktorZ2.displayBrightness[group] = 0x02;
                        TraktorZ2.displayBeatLeds(group);
                    }
                }
            }, false);
        } else {
            TraktorZ2.chTimer[group][1] = engine.beginTimer(0.25 * beatPeriodMillis / 5 * 1, function() {
                TraktorZ2.displayBrightness[group]--;
                TraktorZ2.displayBeatLeds(group);
                if (TraktorZ2.displayBrightness[group] <= 0x02) {
                    engine.stopTimer(TraktorZ2.chTimer[group][1]);
                    TraktorZ2.chTimer[group][1] = -1;
                }
            }, false);
        }
    }
};

TraktorZ2.displayBeatLeds = function(group) {
    if ((group === "[Channel1]") || (group === "[Channel2]")) {
        TraktorZ2.displayLoopCount(group);
    } else {
        TraktorZ2.controller.setOutput(group, "!beatIndicator", TraktorZ2.displayBrightness[group], true);
    }
}

TraktorZ2.displayLoopCount = function(group) {
    // @param group may be either[Channel1] or [Channel2]
    // @param TraktorZ2.displayBrightness[group] may be aninteger value from 0x00 to 0x07
    var beatloopSize = engine.getValue(group, "beatloop_size");

    var led2DigitModulus = {
        "[Digit3]": 10,
        "[Digit2]": 100
    };

    var led3DigitModulus = {
        "[Digit3]": 10,
        "[Digit2]": 100,
        "[Digit1]": 1000
    };

    var displayBrightness;

    if (engine.getValue(group, "loop_enabled")) {
        var playposition = engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
        if (
            (playposition >= engine.getValue(group, "loop_start_position")) &&
            (playposition <= engine.getValue(group, "loop_end_position"))
        ) {
            displayBrightness = TraktorZ2.displayBrightness[group];
        } else {
            displayBrightness = LedDimmed;
        }
    } else if (engine.getValue(group, "track_loaded") === 0) {
        displayBrightness = LedOff;
    } else {
        displayBrightness = LedBright;
    }

    if (beatloopSize < 1) {
        // Fraction of a beat
        var beatloopSizeRemainder = 1 / beatloopSize;
        for (var digit in led2DigitModulus) {
            var leastSignificiantDigit = (beatloopSizeRemainder % 10);
            beatloopSizeRemainder = beatloopSizeRemainder - leastSignificiantDigit;
            //HIDDebug(leastSignificiantDigit + " " + beatloopSizeRemainder + " " + group + " " + digit);
            if (digit === "[Digit2]" && beatloopSize > .1) {
                leastSignificiantDigit = -1; // Leading ero -> Show special symbol of number 1 and the fraction stroke combined in left digit
            }
            TraktorZ2.displayLoopCountDigit(group + digit, leastSignificiantDigit, displayBrightness);
            beatloopSizeRemainder /= 10;
        }
        if (beatloopSize > .1) {
            TraktorZ2.displayLoopCountDigit(group + "[Digit1]", -2, displayBrightness);  // Leading ero -> Blank
        } else {
            TraktorZ2.displayLoopCountDigit(group + "[Digit1]", -1, displayBrightness); // Show special symbol of number 1 and the fraction stroke combined in left digit
        }
    } else {
        // Beat integer
        beatloopSizeRemainder = beatloopSize;
        for (digit in led3DigitModulus) {
            leastSignificiantDigit = (beatloopSizeRemainder % 10);
            beatloopSizeRemainder = beatloopSizeRemainder - leastSignificiantDigit;
            //HIDDebug(leastSignificiantDigit + " " + beatloopSizeRemainder + " " + group + " " + digit);
            if ((digit === "[Digit1]" && beatloopSize < 100) || (digit === "[Digit2]" && beatloopSize < 10)) {
                leastSignificiantDigit = -2; // Leading ero -> Blank
            }
            TraktorZ2.displayLoopCountDigit(group + digit, leastSignificiantDigit, displayBrightness);
            beatloopSizeRemainder /= 10;
        }
    }
};

TraktorZ2.displayLoopCountDigit = function(group, digit, brightness) {
    // @param offset of the first LED (center horizontal bar) of the digit
    // @param digit to display (-2 represents all OFF, -1 represents "1/" )
    // @param brightness may be aninteger value from 0x00 to 0x07
    // HIDDebug("Offset:" + " Digit:" + digit + " Brightness:" + brightness);

    // Segment a (upper horizontal bar)
    if (digit === 0 || digit === 2 || digit === 3 || digit === 5 || digit === 6 || digit === 7  || digit === 8  || digit === 9) {
        TraktorZ2.controller.setOutput(group, "segment_a", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_a", LedOff,       false); // OFF
    }

    // Segment b (upper right vertical bar)
    if (digit === 0 || digit === 1 || digit === 2 || digit === 3 || digit === 4 || digit === 7  || digit === 8  || digit === 9 || digit === -1) {
        TraktorZ2.controller.setOutput(group, "segment_b", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_b", LedOff,       false); // OFF
    }

    // Segment c (lower right vertical bar)
    if (digit === 0 || digit === 1 || digit === 3 || digit === 4  || digit === 5  || digit === 6  || digit === 7  || digit === 8  || digit === 9) {
        TraktorZ2.controller.setOutput(group, "segment_c", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_c", LedOff,       false); // OFF
    }

    // Segment d (lower horizontal bar)
    if (digit === 0 || digit === 2 || digit === 3 || digit === 5  || digit === 6 || digit === 8  || digit === 9) {
        TraktorZ2.controller.setOutput(group, "segment_d", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_d", LedOff,     false); // OFF
    }

    // Segment e (lower left vertical bar)
    if (digit === 0 || digit === 2 || digit === 6 || digit === 8 || digit === -1) {
        TraktorZ2.controller.setOutput(group, "segment_e", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_e", LedOff,     false); // OFF
    }

    // Segment f (upper left vertical bar)
    if (digit === 0 || digit === 4 || digit === 5 || digit === 6 || digit === 8  || digit === 9 || digit === -1) {
        TraktorZ2.controller.setOutput(group, "segment_f", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_f", LedOff,     false); // OFF
    }

    // Send HID packet at last digit of last digit
    var batching = (group === "[Channel1][Digit1]" || group === "[Channel2][Digit1]");

    // Segment g (center horizontal bar)
    if (digit === 2 || digit === 3  || digit === 4 || digit === 5 || digit === 6 || digit === 8 || digit === 9) {
        TraktorZ2.controller.setOutput(group, "segment_g", brightness, batching); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_g", LedOff,     batching); // OFF
    }
};


TraktorZ2.registerOutputPackets = function() {
    HIDDebug("TraktorZ2: registerOutputPackets");
    var outputA = new HIDPacket("outputA", 0x80);
    var outputB = new HIDPacket("outputB", 0x81);

    for (var ch = 1; ch <= 4; ch++) {
        var group = "[Channel" + ch + "]";
        for (var hotcue = 1; hotcue <= 8; hotcue++) {
            engine.connectControl(group, "hotcue_" + hotcue + "_color", TraktorZ2.bind(TraktorZ2.hotcueOutputHandler, this));
            engine.connectControl(group, "hotcue_" + hotcue + "_enabled", TraktorZ2.bind(TraktorZ2.hotcueOutputHandler, this));
        }
    }
    for (ch = 1; ch <= 2; ch++) {
        group = "[Channel" + ch + "]";
        for (hotcue = 1; hotcue <= 4; hotcue++) {
            var address = 0x1D + ((ch-1) * 4 * 3) + ((hotcue-1) * 3);
            outputA.addOutput(group, "Hotcue" + hotcue + "Red",   address,   "B", 0x70);
            outputA.addOutput(group, "Hotcue" + hotcue + "Green", address+1, "B", 0x70);
            outputA.addOutput(group, "Hotcue" + hotcue + "Blue",  address+2, "B", 0x70);
        }
    }


    outputA.addOutput("[Channel1]", "quantize", 0x03, "B", 0x70);
    engine.connectControl("[Channel1]", "quantize", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));
    outputA.addOutput("[Channel2]", "quantize", 0x04, "B", 0x70);
    engine.connectControl("[Channel2]", "quantize", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputA.addOutput("[Master]", "skin_settings", 0x05, "B", 0x70);
    engine.connectControl("[Master]", "skin_settings", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));
    outputA.addOutput("[Master]", "shift", 0x06, "B", 0x70);

    outputA.addOutput("[Channel1]", "!deck", 0x07, "B", 0x70);
    outputA.addOutput("[Channel2]", "!deck", 0x08, "B", 0x70);
    outputA.addOutput("[Channel3]", "!deck", 0x09, "B", 0x70);
    outputA.addOutput("[Channel4]", "!deck", 0x0A, "B", 0x70);

    outputA.addOutput("[Channel3]", "!beatIndicator", 0x0B, "B", 0x70);
    outputA.addOutput("[Channel4]", "!beatIndicator", 0x0C, "B", 0x70);

    outputA.addOutput("[EffectRack1_EffectUnit1]", "!On", 0x0D, "B", 0x70);
    engine.connectControl("[EffectRack1_EffectUnit1_Effect1]", "enabled", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit1_Effect1]", "loaded", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit1_Effect2]", "enabled", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit1_Effect2]", "loaded", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit1_Effect3]", "enabled", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit1_Effect3]", "loaded", TraktorZ2.bind(this.fxOnLedHandler, this));

    outputA.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x0F, "B", 0x70);
    engine.connectControl("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));
    outputA.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x10, "B", 0x70);
    engine.connectControl("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputA.addOutput("[EffectRack1_EffectUnit2]", "!On", 0x15, "B", 0x70);
    engine.connectControl("[EffectRack1_EffectUnit2_Effect1]", "enabled", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit2_Effect1]", "loaded", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit2_Effect2]", "enabled", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit2_Effect2]", "loaded", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit2_Effect3]", "enabled", TraktorZ2.bind(this.fxOnLedHandler, this));
    engine.connectControl("[EffectRack1_EffectUnit2_Effect3]", "loaded", TraktorZ2.bind(this.fxOnLedHandler, this));

    outputA.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x17, "B", 0x70);
    engine.connectControl("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));
    outputA.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x18, "B", 0x70);
    engine.connectControl("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputA.addOutput("[Channel1]", "slip_enabled", 0x0E, "B", 0x70);
    engine.connectControl("[Channel1]", "slip_enabled", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));
    outputA.addOutput("[Channel1]", "!vinylcontrol_orange", 0x12, "B", 0x70);
    outputA.addOutput("[Channel1]", "!vinylcontrol_green", 0x13, "B", 0x70);
    engine.connectControl("[Channel1]", "vinylcontrol_status", TraktorZ2.bind(TraktorZ2.vinylcontrolOutputHandler, this));
    outputA.addOutput("[Channel1]", "sync_enabled", 0x14, "B", 0x70);
    engine.connectControl("[Channel1]", "sync_enabled", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputA.addOutput("[Channel2]", "slip_enabled", 0x16, "B", 0x70);
    engine.connectControl("[Channel2]", "slip_enabled", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));
    outputA.addOutput("[Channel2]", "!vinylcontrol_orange", 0x1A, "B", 0x70);
    outputA.addOutput("[Channel2]", "!vinylcontrol_green", 0x1B, "B", 0x70);
    engine.connectControl("[Channel2]", "vinylcontrol_status", TraktorZ2.bind(TraktorZ2.vinylcontrolOutputHandler, this));
    outputA.addOutput("[Channel2]", "sync_enabled", 0x1C, "B", 0x70);
    engine.connectControl("[Channel2]", "sync_enabled", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    engine.connectControl("[Channel1]", "beat_active", TraktorZ2.bind(TraktorZ2.beatOutputHandler, this));
    engine.connectControl("[Channel2]", "beat_active", TraktorZ2.bind(TraktorZ2.beatOutputHandler, this));
    engine.connectControl("[Channel3]", "beat_active", TraktorZ2.bind(TraktorZ2.beatOutputHandler, this));
    engine.connectControl("[Channel4]", "beat_active", TraktorZ2.bind(TraktorZ2.beatOutputHandler, this));

    engine.connectControl("[Channel1]", "track_loaded", TraktorZ2.bind(TraktorZ2.trackLoadedHandler, this));
    engine.connectControl("[Channel2]", "track_loaded", TraktorZ2.bind(TraktorZ2.trackLoadedHandler, this));
    engine.connectControl("[Channel3]", "track_loaded", TraktorZ2.bind(TraktorZ2.trackLoadedHandler, this));
    engine.connectControl("[Channel4]", "track_loaded", TraktorZ2.bind(TraktorZ2.trackLoadedHandler, this));

    var ledChannelOffsets = {
        "[Channel1]": 0x35,
        "[Channel2]": 0x4A
    };
    var ledDigitOffsets = {
        "[Digit1]": 0x00,
        "[Digit2]": 0x07,
        "[Digit3]": 0x0E
    };

    for (ch in ledChannelOffsets) {
        for (var digit in ledDigitOffsets) {
            outputA.addOutput(ch + digit, "segment_g", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x00, "B", 0x70); // 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment center horizontal bar brightness (orange)
            outputA.addOutput(ch + digit, "segment_c", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x01, "B", 0x70); // 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower right vertical bar brightness (orange)
            outputA.addOutput(ch + digit, "segment_b", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x02, "B", 0x70); // 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper right vertical bar brightness (orange)
            outputA.addOutput(ch + digit, "segment_a", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x03, "B", 0x70); // 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper horizontal bar brightness (orange)
            outputA.addOutput(ch + digit, "segment_f", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x04, "B", 0x70); // 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper left vertical bar brightness (orange)
            outputA.addOutput(ch + digit, "segment_e", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x05, "B", 0x70); // 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower left vertical bar brightness (orange)
            outputA.addOutput(ch + digit, "segment_d", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x06, "B", 0x70); // 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower horizontal bar brightness (orange)
        }
    }

    this.controller.registerOutputPacket(outputA);

    // Headphone buttons
    outputB.addOutput("[Channel1]", "pfl", 0x23, "B", 0x70);
    engine.connectControl("[Channel1]", "pfl", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputB.addOutput("[Channel2]", "pfl", 0x24, "B", 0x70);
    engine.connectControl("[Channel2]", "pfl", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputB.addOutput("[Channel1]", "traktorbutton", 0x25, "B", 0x70);
    // engine.connectControl("[Channel1]", "passthrough", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputB.addOutput("[Channel2]", "traktorbutton", 0x26, "B", 0x70);
    // engine.connectControl("[Channel2]", "passthrough", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputB.addOutput("[Master]", "!usblight", 0x27, "B", 0x70);
    
    outputB.addOutput("[Master]", "!VuLabelMst", 0x21, "B", 0x70);
    outputB.addOutput("[Master]", "!crossfaderReverse", 0x28, "B", 0x70);

    
    var VuOffsets = {
        "[Channel1]": 0x01, // ChA
        "[Channel2]": 0x09, // ChB
        "[Channel3]": 0x11, // ChC/MasterL
        "[Channel4]": 0x19  // ChD_MasterR
    };

    for (ch in VuOffsets) {
        outputB.addOutput(ch, "!VuLabel", VuOffsets[ch], "B", 0x70);
        for (var i = 0; i < 6; i++) {
            outputB.addOutput(ch, "!VuMeter" + i, VuOffsets[ch] + i + 0x01, "B", 0x70);
        }
        outputB.addOutput(ch, "!PeakIndicator", VuOffsets[ch] + 0x07, "B", 0x70);
    }

    this.controller.registerOutputPacket(outputB);

};

TraktorZ2.displayVuMeter = function() {

    var VuMeters = {
        "[Channel1]": "", // ChA
        "[Channel2]": "", // ChB
        "[Channel3]": "", // ChC/MasterL
        "[Channel4]": ""  // ChD_MasterR
    };

    for (var ch in VuMeters) {

        var VuValue;
        var PeakIndicator;

        if  (ch === "[Channel3]") {
            // MasterL
            VuValue = engine.getValue("[Master]", "VuMeterL") * 6;
            PeakIndicator = engine.getValue("[Master]", "PeakIndicatorL");
        } else if  (ch === "[Channel4]") {
            // MasterR
            VuValue = engine.getValue("[Master]", "VuMeterR") * 6;
            PeakIndicator = engine.getValue("[Master]", "PeakIndicatorR");
        } else if  ((ch === "[Channel1]") && (TraktorZ2.pregainCh3Timer === 0)) {
            // ChA
            VuValue = engine.getValue(ch, "VuMeter") * 6;
            PeakIndicator = engine.getValue(ch, "PeakIndicator");
            TraktorZ2.controller.setOutput("[Channel1]", "!VuLabel", LedBright, false);
            TraktorZ2.controller.setOutput("[Channel3]", "!VuLabel", LedOff, false);
        } else if  ((ch === "[Channel1]") && (TraktorZ2.pregainCh3Timer !== 0)) {
            // ChC
            VuValue = engine.getValue("[Channel3]", "VuMeter") * 6;
            PeakIndicator = engine.getValue("[Channel3]", "PeakIndicator");
            TraktorZ2.controller.setOutput("[Channel1]", "!VuLabel", LedOff, false);
            TraktorZ2.controller.setOutput("[Channel3]", "!VuLabel", LedBright, false);
        } else if  ((ch === "[Channel2]") && (TraktorZ2.pregainCh4Timer === 0)) {
            // ChB
            VuValue = engine.getValue(ch, "VuMeter") * 6;
            PeakIndicator = engine.getValue(ch, "PeakIndicator");
            TraktorZ2.controller.setOutput("[Channel2]", "!VuLabel", LedBright, false);
            TraktorZ2.controller.setOutput("[Channel4]", "!VuLabel", LedOff, false);
        } else if  ((ch === "[Channel2]") && (TraktorZ2.pregainCh4Timer !== 0)) {
            // ChD
            VuValue = engine.getValue("[Channel4]", "VuMeter") * 6;
            PeakIndicator = engine.getValue("[Channel4]", "PeakIndicator");
            TraktorZ2.controller.setOutput("[Channel2]", "!VuLabel", LedOff, false);
            TraktorZ2.controller.setOutput("[Channel4]", "!VuLabel", LedBright, false);
        }

        for (var i = 0; i < 6; i++) {
            var brightness = (VuValue - i) * LedBright;
            if (brightness < LedOff) {
                brightness = LedOff;
            }
            if (brightness > LedBright) {
                brightness = LedBright;
            }
            TraktorZ2.controller.setOutput(ch, "!VuMeter" + i, brightness, false);
        }

        if (PeakIndicator) {
            TraktorZ2.controller.setOutput(ch, "!PeakIndicator", LedBright, (ch === "[Channel4]"));
        } else {
            TraktorZ2.controller.setOutput(ch, "!PeakIndicator", LedOff,    (ch === "[Channel4]"));
        }
    }
};

TraktorZ2.init = function(_id) {
    this.Decks = {
        "deck1": new TraktorZ2.Deck(1, "deck1"),
        "deck2": new TraktorZ2.Deck(2, "deck2"),
    };

    // Traktor Z2 can be switched per channel from internal mixing to external mixing
    // This is done by USB HID: Set Reports (Feature) 0xF1
    // Bit 0x10 Must be set to see any LED output
    // 0xF1 9n 40  -> Bit 0x01 of n means  Ch1 (internal) mixing
    // 0xF1 9n 40  -> Bit 0x02 of n means  Ch2 (internal) mixing
    // 0xF1 9n 40  -> Bit 0x04 of n means  MasterCh (internal) mixing
    // 0xF1 9n 40  -> Bit 0x08 of n means  Mic/Aux (internal) mixing

    var data
    data = [0xFF, 0x40];
    controller.sendFeatureReport(data, 0xF1);
    data = [0xFF, 0x40];
    controller.sendFeatureReport(data, 0xF3);

    //TraktorZ2.debugLights();

    TraktorZ2.registerInputPackets();
    TraktorZ2.registerOutputPackets();

    TraktorZ2.controller.setOutput("[Master]", "!usblight", LedDimmed, true);

    TraktorZ2.deckSwitchHandler["[Channel1]"] = 1;
    TraktorZ2.controller.setOutput("[Channel1]", "!deck", LedBright,       true);
    TraktorZ2.deckSwitchHandler["[Channel2]"] = 1;
    TraktorZ2.controller.setOutput("[Channel2]", "!deck", LedBright,       true);
    TraktorZ2.deckSwitchHandler["[Channel3]"] = 0;
    TraktorZ2.controller.setOutput("[Channel3]", "!deck", LedOff,       true);
    TraktorZ2.deckSwitchHandler["[Channel4]"] = 0;
    TraktorZ2.controller.setOutput("[Channel4]", "!deck", LedOff,       true);

    TraktorZ2.controller.setOutput("[Master]", "!VuLabelMst", LedBright, false);

    TraktorZ2.hotcueOutputHandler();
    HIDDebug("TraktorZ2: Init done!");

    this.guiTickConnection = engine.makeConnection("[Master]", "guiTick50ms", this.displayVuMeter);
};

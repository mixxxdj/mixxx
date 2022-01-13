///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* Traktor Kontrol Z2 HID controller script v1.00                                */
/* Last modification: January 2022                                                */
/* Author: Jörg Wartenberg (based on the Traktor S3 mapping by Owen Williams)    */
/*                                                                               */
///////////////////////////////////////////////////////////////////////////////////

"use strict";

// Each color has 7Bit brightnesses, so these values can be between 0 and 128.
const kLedOff = 0x00;
const kLedDimmed = 0x27;
const kLedVuMeterBrightness = 0x37;
const kLedBright = 0x7F;

var TraktorZ2 = {};

TraktorZ2 = new function() {
    this.controller = new HIDController();

    this.shiftPressed = false;

    // 0x00: shift mode off / and not active pressed
    // 0x01: shift mode off / but active pressed
    // 0x02: shift mode on  / and not active pressed
    // 0x03: shift mode on  / and active pressed
    this.shiftState = 0x00;

    this.syncPressedTimer = [];
    this.syncPressed = [];
    this.snapQuantizePressedTimer = [];
    this.snapQuantizePressed = [];

    this.microphoneButtonStatus;
    this.traktorButtonStatus = [];

    this.dataF1 = new Uint8Array;

    // Knob encoder states (hold values between 0x0 and 0xF)
    // Rotate to the right is +1 and to the left is means -1
    this.browseKnobEncoderState = 0;

    this.lastsendTimestamp = 0;
    this.lastBeatTimestamp = [];
    this.beatLoopFractionCounter = [];
    this.displayBrightness = [];

    this.pregainCh3Timer = 0;
    this.pregainCh4Timer = 0;

    this.eqValueStorage = [];

    this.chTimer = [];
    for (let chidx = 1; chidx <= 4; chidx++) {
        this.lastBeatTimestamp["[Channel" + chidx + "]"] = 0;
        this.beatLoopFractionCounter["[Channel" + chidx + "]"] = 0;
        this.displayBrightness["[Channel" + chidx + "]"] = kLedDimmed;
    }

    this.inputReport01 = new HIDPacket;
    this.inputReport02 = new HIDPacket;

    this.outputReport80 = new HIDPacket;
    this.outputReport81 = new HIDPacket;
};

// Mixxx's javascript doesn't support .bind natively, so here's a simple version.
TraktorZ2.bind = function(fn, obj) {
    return function() {
        return fn.apply(obj, arguments);
    };
};

TraktorZ2.fxOnClickHandler = function(field) {
    HIDDebug("TraktorZ2: fxOnClickHandler");
    let numOfLoadedandEnabledEffects = 0;
    for (let effectIdx = 1; effectIdx <= engine.getValue(field.group, "num_effectslots"); effectIdx++) {
        if (engine.getValue(field.group.substr(0, field.group.length - 1) + "_Effect" + effectIdx + "]", "loaded") === 1) {
            if (engine.getValue(field.group.substr(0, field.group.length - 1) + "_Effect" + effectIdx + "]", "enabled") === 1) {
                numOfLoadedandEnabledEffects++;
            }
        }
    }

    if (field.value !== 0) {
        if (numOfLoadedandEnabledEffects === 0) {
            for (let effectIdx = 1; effectIdx <= engine.getValue(field.group, "num_effectslots"); effectIdx++) {
                if (engine.getValue(field.group.substr(0, field.group.length - 1) + "_Effect" + effectIdx + "]", "loaded") === 1) {
                    engine.setValue(field.group.substr(0, field.group.length - 1) + "_Effect" + effectIdx + "]", "enabled", 1);
                }
            }
        } else {
            for (let effectIdx = 1; effectIdx <= engine.getValue(field.group, "num_effectslots"); effectIdx++) {
                engine.setValue(field.group.substr(0, field.group.length - 1) + "_Effect" + effectIdx + "]", "enabled", 0);
            }
        }
    }
};

TraktorZ2.fxOnLedHandler = function() {
    HIDDebug("TraktorZ2: fxOnLedHandler");
    for (let macroFxUnitIdx = 1; macroFxUnitIdx <= 2; macroFxUnitIdx++) {
        let numOfLoadedButDisabledEffects = 0;
        let numOfLoadedandEnabledEffects = 0;
        for (let effectIdx = 1; effectIdx <= engine.getValue("[EffectRack1_EffectUnit" + macroFxUnitIdx + "]", "num_effectslots"); effectIdx++) {
            if (engine.getValue("[EffectRack1_EffectUnit" + macroFxUnitIdx + "_Effect" + effectIdx + "]", "loaded") === 1) {
                if (engine.getValue("[EffectRack1_EffectUnit" + macroFxUnitIdx + "_Effect" + effectIdx + "]", "enabled") === 1) {
                    numOfLoadedandEnabledEffects++;
                } else {
                    numOfLoadedButDisabledEffects++;
                }
            }
        }
        if (numOfLoadedandEnabledEffects === 0) {
            TraktorZ2.controller.setOutput("[EffectRack1_EffectUnit" + macroFxUnitIdx + "]", "!On", kLedOff, macroFxUnitIdx === 2);
        } else if (numOfLoadedandEnabledEffects > 0 && numOfLoadedButDisabledEffects > 0) {
            TraktorZ2.controller.setOutput("[EffectRack1_EffectUnit" + macroFxUnitIdx + "]", "!On", kLedDimmed, macroFxUnitIdx === 2);
        } else {
            TraktorZ2.controller.setOutput("[EffectRack1_EffectUnit" + macroFxUnitIdx + "]", "!On", kLedBright, macroFxUnitIdx === 2);
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
    TraktorZ2.syncPressedTimer[this.activeChannel] = 0;
    TraktorZ2.syncPressed[this.activeChannel] = false;
    TraktorZ2.snapQuantizePressedTimer[this.activeChannel] = 0;
    TraktorZ2.snapQuantizePressed[this.activeChannel] = false;

    // Knob encoder states (hold values between 0x0 and 0xF)
    // Rotate to the right is +1 and to the left is means -1
    this.loopKnobEncoderState = 0;
};


TraktorZ2.deckSwitchHandler = function(field) {
    HIDDebug("TraktorZ2: deckSwitchHandler: " + field.group + " " + field.value);
    if (field.value === 1) {
        if (field.group === "[Channel1]") {
            TraktorZ2.controller.setOutput("[Channel3]", "!deck", kLedOff, true);
            TraktorZ2.deckSwitchHandler["[Channel3]"] = 0;
        } else if (field.group === "[Channel2]") {
            TraktorZ2.controller.setOutput("[Channel4]", "!deck", kLedOff, true);
            TraktorZ2.deckSwitchHandler["[Channel4]"] = 0;
        } else if (field.group === "[Channel3]") {
            TraktorZ2.controller.setOutput("[Channel1]", "!deck", kLedOff, true);
            TraktorZ2.deckSwitchHandler["[Channel1]"] = 0;
        } else if (field.group === "[Channel4]") {
            TraktorZ2.controller.setOutput("[Channel2]", "!deck", kLedOff, true);
            TraktorZ2.deckSwitchHandler["[Channel2]"] = 0;
        }

        if (TraktorZ2.deckSwitchHandler[field.group] !== 1) {
            TraktorZ2.deckSwitchHandler[field.group] = 1;
            TraktorZ2.controller.setOutput(field.group, "!deck", kLedBright, true);

        } else if (engine.getValue("[Skin]", "show_8_hotcues")) {
            TraktorZ2.deckSwitchHandler[field.group] = 2;
            TraktorZ2.controller.setOutput(field.group, "!deck", kLedDimmed, true);
        }
        TraktorZ2.hotcueOutputHandler(); // Set new hotcue button colors
    }
};

TraktorZ2.Deck.prototype.numberButtonHandler = function(field) {

    const sideChannel = ["[Channel1]", "[Channel2]"];
    const sideOffset = [0, 0];

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
    let chIdx;
    if (this.activeChannel === "[Channel1]") {
        chIdx = 1;
    } else {
        chIdx = 2;
    }

    const padNumber = parseInt(field.id[field.id.length - 1]);
    let action = "";

    // Hotcues mode clear when shift button is active pressed
    if (TraktorZ2.shiftState & 0x01) {
        action = "_clear";
    } else {
        action = "_activate";
    }
    HIDDebug("setting " + "hotcue_" + padNumber + action + " " + field.value);
    engine.setValue(sideChannel[chIdx], "hotcue_" + (sideOffset[chIdx] + padNumber) + action, field.value);
};

TraktorZ2.Deck.prototype.fluxHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    script.toggleControl(this.activeChannel, "slip_enabled");
};

TraktorZ2.Deck.prototype.vinylcontrolHandler = function(field) {
    HIDDebug("TraktorZ2: vinylcontrolHandler" + " this.activeChannel:" + this.activeChannel + " field.value:" + field.value);
    if (field.value === 0 || (engine.getValue(this.activeChannel, "passthrough") === 1)) {
        return;
    }

    if ((TraktorZ2.shiftState & 0x01) === 0x01) {
        // Shift button hold down -> Toggle between Internal Playback mode / Vinyl Control
        script.toggleControl(this.activeChannel, "vinylcontrol_enabled");
    } else {
        if (engine.getValue(this.activeChannel, "vinylcontrol_enabled") === 0) {
            // Internal Playback mode -> Vinyl Control Off -> Orange
            if ((TraktorZ2.shiftState & 0x02) !== 0x02) {
                // Shift mode isn't locked -> Mapped to PLAY button
                script.toggleControl(this.activeChannel, "play");
            } else {
                // Shift mode isn't locked -> Mapped to CUE button
                script.toggleControl(this.activeChannel, "cue_default");
            }
        } else {
            let vinylControlMode = engine.getValue(this.activeChannel, "vinylcontrol_mode");
            vinylControlMode++;
            if (vinylControlMode > 2) {
                vinylControlMode = 0;
            }
            engine.setValue(this.activeChannel, "vinylcontrol_mode", vinylControlMode);
        }
    }

};

TraktorZ2.vinylcontrolOutputHandler = function(value, group, key) {
    HIDDebug("TraktorZ2: vinylcontrolOutputHandler" + " group:" + group + " key:" + key);
    TraktorZ2.traktorButtonOutputHandler(group); // Sets TaktorButton state, depending on Passthrough state
    if (engine.getValue(group, "passthrough") === 1) {
        // REL /INTL button has no function in Passthrough mode -> LED Off
        TraktorZ2.controller.setOutput(group, "!vinylcontrol_green", kLedOff, false);
        TraktorZ2.controller.setOutput(group, "!vinylcontrol_orange", kLedOff, true);
        return;
    }

    if (engine.getValue(group, "vinylcontrol_enabled") === 0) {
        // Internal Playback mode -> Vinyl Control Off -> Orange
        TraktorZ2.controller.setOutput(group, "!vinylcontrol_green", kLedOff);
        if ((TraktorZ2.shiftState & 0x02) !== 0x02) {
            // Shift mode isn't locked -> Show PLAY indicator
            if (engine.getValue(group, "play_indicator") === 0) {
                // Dim only to signal visualize Internal Playback mode by Orange color
                TraktorZ2.controller.setOutput(group, "!vinylcontrol_orange", kLedDimmed, true);
            } else {
                TraktorZ2.controller.setOutput(group, "!vinylcontrol_orange", kLedBright, true);
            }
        } else {
            // Shift mode is locked -> Show CUE indicator
            if (engine.getValue(group, "cue_indicator") === 0) {
                // Dim only to signal visualize Internal Playback mode by Orange color
                TraktorZ2.controller.setOutput(group, "!vinylcontrol_orange", kLedDimmed, true);
            } else {
                TraktorZ2.controller.setOutput(group, "!vinylcontrol_orange", kLedBright, true);
            }
        }
    } else {
        if (engine.getValue(group, "vinylcontrol_mode") === 0) {
            // Absolute Mode (track position equals needle position and speed)
            TraktorZ2.controller.setOutput(group, "!vinylcontrol_green", kLedBright, false);
            TraktorZ2.controller.setOutput(group, "!vinylcontrol_orange", kLedOff, true);
        } else if (engine.getValue(group, "vinylcontrol_mode") === 1) {
            // Relative Mode (track tempo equals needle speed regardless of needle position)
            TraktorZ2.controller.setOutput(group, "!vinylcontrol_green", kLedDimmed, false);
            TraktorZ2.controller.setOutput(group, "!vinylcontrol_orange", kLedOff, true);
        } else if (engine.getValue(group, "vinylcontrol_mode") === 2) {
            // Constant Mode (track tempo equals last known-steady tempo regardless of needle input
            // Both LEDs on -> Values result in a dirty yellow
            TraktorZ2.controller.setOutput(group, "!vinylcontrol_green", 0x37, false);
            TraktorZ2.controller.setOutput(group, "!vinylcontrol_orange", 0x57, true);
        }
    }
};
TraktorZ2.vinylcontrolStatusOutputHandler = function(vfalue, group, key) {
    HIDDebug("TraktorZ2: vinylcontrolOutputHandler" + " group:" + group + " key:" + key);
    // Z2 has only one vinylcontrol status LED for both channels -> merge information of both
    if ((engine.getValue("[Channel1]", "vinylcontrol_status") === 3) ||
        (engine.getValue("[Channel2]", "vinylcontrol_status") === 3) ||
        (engine.getValue("[Channel1]", "vinylcontrol_status") === 2) ||
        (engine.getValue("[Channel2]", "vinylcontrol_status") === 2)) {
        TraktorZ2.controller.setOutput("[Master]", "!vinylcontrolstatus", kLedBright, true);
    } else if ((engine.getValue("[Channel1]", "vinylcontrol_status") === 1) || (engine.getValue("[Channel2]", "vinylcontrol_status") === 1)) {
        TraktorZ2.controller.setOutput("[Master]", "!vinylcontrolstatus", kLedDimmed, true);
    } else {
        TraktorZ2.controller.setOutput("[Master]", "!vinylcontrolstatus", kLedOff, true);
    }
};
TraktorZ2.Deck.prototype.syncHandler = function(field) {
    HIDDebug("TraktorZ2: syncHandler" + " this.activeChannel:" + this.activeChannel + " field:" + field + "key:" + engine.getValue(this.activeChannel, "key"));

    // Shift not hold down
    if (TraktorZ2.shiftState === 0) {
        if (field.value === 1) {
            engine.setValue(this.activeChannel, "sync_enabled", 1);
            // Start timer to measure how long button is pressed
            const ch = this.activeChannel; // Use variable in timer function, because this.activeChannel can change until the timer is active
            TraktorZ2.syncPressedTimer[ch] = engine.beginTimer(300, function() {
                // Reset sync button timer state if active
                if (TraktorZ2.syncPressedTimer[ch] !== 0) {
                    TraktorZ2.syncPressedTimer[ch] = 0;
                }
            }, true);
        } else {
            if (TraktorZ2.syncPressedTimer[this.activeChannel] !== 0) {
                // Timer still running -> stop it and unlight LED
                engine.stopTimer(TraktorZ2.syncPressedTimer[this.activeChannel]);
                engine.setValue(this.activeChannel, "sync_enabled", 0);
            }
        }
        return;
    }

    // Shift presed and hold
    if (((TraktorZ2.shiftState & 0x01) === 0x01) && (field.value === 1)) {
        script.toggleControl(this.activeChannel, "keylock");
        return;
    }

    // Shift locked
    // Depending on long or short press, sync beat or go to key sync mode
    if (field.value === 1) {
        // Start timer to measure how long button is pressed
        const ch = this.activeChannel; // Use variable in timer function, because this.activeChannel can change until the timer is active
        TraktorZ2.syncPressedTimer[ch] = engine.beginTimer(300, function() {
            TraktorZ2.syncPressed[ch] = true;
            // Change display values to key notation
            TraktorZ2.displayLoopCount("[Channel1]", false);
            TraktorZ2.displayLoopCount("[Channel2]", true);

            // Reset sync button timer state if active
            if (TraktorZ2.syncPressedTimer[ch] !== 0) {
                TraktorZ2.syncPressedTimer[ch] = 0;
            }
        }, true);
    } else {
        TraktorZ2.syncPressed[this.activeChannel] = false;
        // Change display values to loop/beatjump
        TraktorZ2.displayLoopCount("[Channel1]", false);
        TraktorZ2.displayLoopCount("[Channel2]", true);

        if (TraktorZ2.syncPressedTimer[this.activeChannel] !== 0) {
            // Timer still running -> stop it and unlight LED
            engine.stopTimer(TraktorZ2.syncPressedTimer[this.activeChannel]);
            script.triggerControl(this.activeChannel, "sync_key");
        }
    }
};

TraktorZ2.selectTrackHandler = function(field) {
    HIDDebug("TraktorZ2: selectTrackHandler");
    let delta = 1;
    if ((field.value + 1) % 16 === TraktorZ2.browseKnobEncoderState) {
        delta = -1;
    }
    TraktorZ2.browseKnobEncoderState = field.value;

    if (TraktorZ2.snapQuantizePressed["[Channel1]"] !== TraktorZ2.snapQuantizePressed["[Channel2]"]
    ) {
        let ch;
        // Snap / Quantize button is hold for one channel
        if (TraktorZ2.snapQuantizePressed["[Channel1]"]) {
            ch = "[Channel1]";
        } else {
            ch = "[Channel2]";
        }

        if (TraktorZ2.shiftState === 0x02) {
            // If shift mode is locked scale beatgrid
            if (delta < 0) {
                script.triggerControl(ch, "beats_adjust_faster");
            } else {
                script.triggerControl(ch, "beats_adjust_slower");
            }
        } else {
            // Shift is not locked zoom waveform
            if (delta < 0) {
                script.triggerControl(ch, "waveform_zoom_up");
            } else {
                script.triggerControl(ch, "waveform_zoom_down");
            }
        }
        return;
    }

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


// defineButton and defineLED2 allows us to configure either the right deck or the left deck, depending on which
// is appropriate.  This avoids extra logic in the function where we define all the magic numbers.
TraktorZ2.Deck.prototype.defineButton = function(msg, name, deck1Offset, deck1Bitmask, deck2Offset, deck2Bitmask, fn) {
    switch (this.deckNumber) {
    case 1:
        TraktorZ2.registerInputButton(msg, this.group, name, deck1Offset, deck1Bitmask, TraktorZ2.bind(fn, this));
        break;
    case 2:
        TraktorZ2.registerInputButton(msg, this.group, name, deck2Offset, deck2Bitmask, TraktorZ2.bind(fn, this));
        break;
    }
};

TraktorZ2.Deck.prototype.defineLED2 = function(hidReport, name, deck1Offset, deck2Offset) {
    // All LEDs of the Traktor Z2 have 7Bit outputs
    HIDDebug("TraktorZ2: defineLED2 hidReport:" + hidReport + " this.deckNumber" + this.deckNumber + " name:" + name);

    switch (this.deckNumber) {
    case 1:
        hidReport.addOutput("[Channel1]", name, deck1Offset, "B", 0x7F);
        break;
    case 2:
        hidReport.addOutput("[Channel2]", name, deck2Offset, "B", 0x7F);
        break;
    }
};

TraktorZ2.Deck.prototype.defineLED4 = function(hidReport, name, deck1Offset, deck2Offset, deck3Offset, deck4Offset) {
    // All LEDs of the Traktor Z2 have 7Bit outputs
    HIDDebug("TraktorZ2: defineLED4 hidReport:" + hidReport + " this.deckNumber" + this.deckNumber + " name:" + name);

    switch (this.deckNumber) {
    case 1:
        hidReport.addOutput("[Channel1]", name, deck1Offset, "B", 0x7F);
        break;
    case 2:
        hidReport.addOutput("[Channel2]", name, deck2Offset, "B", 0x7F);
        break;
    case 3:
        hidReport.addOutput("[Channel3]", name, deck3Offset, "B", 0x7F);
        break;
    case 4:
        hidReport.addOutput("[Channel4]", name, deck4Offset, "B", 0x7F);
        break;
    }
};

TraktorZ2.Deck.prototype.selectLoopHandler = function(field) {
    HIDDebug("TraktorZ2: selectLoopHandler" + this.activeChannel + "  field.value:" + field.value);

    if (
        ((TraktorZ2.syncPressed["[Channel1]"] === true) && (this.activeChannel !== "[Channel1]")) ||
        ((TraktorZ2.syncPressed["[Channel2]"] === true) && (this.activeChannel !== "[Channel2]"))
    ) {
        // Display shows key not loop or beatjump -> Ignore input
        return;
    }

    if (TraktorZ2.syncPressed[this.activeChannel] === true) {
        // Sync hold down -> Adjust key
        if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
            script.triggerControl(this.activeChannel, "pitch_down");
        } else {
            script.triggerControl(this.activeChannel, "pitch_up");
        }
    } else if (TraktorZ2.shiftState === 0x00) {
        // Shift mode not set, and shift button not pressed -> Adjust loop size
        const beatloopSize = engine.getValue(this.activeChannel, "beatloop_size");
        if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
            if (beatloopSize >= (1 / 16)) {
                engine.setValue(this.activeChannel, "beatloop_size", beatloopSize / 2);
            }
        } else {
            if (beatloopSize < 256) {
                engine.setValue(this.activeChannel, "beatloop_size", beatloopSize * 2);
            }
        }
    } else if (TraktorZ2.shiftState === 0x01) {
        // Shift mode not set, but shift button is pressed ->  Move loop
        if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
            engine.setValue(this.activeChannel, "loop_move", engine.getValue(this.activeChannel, "beatloop_size") * -1);
        } else {
            engine.setValue(this.activeChannel, "loop_move", engine.getValue(this.activeChannel, "beatloop_size"));
        }
    } else if (TraktorZ2.shiftState === 0x02) {
        // Shift mode is set, but shift button not pressed ->  Adjust beatjump size
        const beatjumpSize = engine.getValue(this.activeChannel, "beatjump_size");
        if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
            engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize / 2);
        } else {
            engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize * 2);
        }
    } else if (TraktorZ2.shiftState === 0x03) {
        // Shift mode is set, and shift button is pressed ->  Move beatjump
        if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
            engine.setValue(this.activeChannel, "beatjump", engine.getValue(this.activeChannel, "beatjump_size") * -1);
        } else {
            engine.setValue(this.activeChannel, "beatjump", engine.getValue(this.activeChannel, "beatjump_size"));
        }
    }
    this.loopKnobEncoderState = field.value;
};

TraktorZ2.Deck.prototype.activateLoopHandler = function(field) {
    HIDDebug("TraktorZ2: activateLoopHandler");
    if (field.value === 1) {
        const isLoopActive = engine.getValue(this.activeChannel, "loop_enabled");

        if (TraktorZ2.syncPressed[this.activeChannel] === true) {
            // Sync hold down -> Sync key
            script.triggerControl(this.activeChannel, "reset_key");
        } else if (TraktorZ2.shiftState) {
            engine.setValue(this.activeChannel, "reloop_toggle", field.value);
        } else {
            if (isLoopActive) {
                engine.setValue(this.activeChannel, "reloop_toggle", field.value);
            } else {
                engine.setValue(this.activeChannel, "beatloop_activate", field.value);
            }
        }
    }
};

TraktorZ2.crossfaderReverseHandler = function(field) {
    HIDDebug("TraktorZ2: LibraryFocusHandler");
    if (field.value) {
        TraktorZ2.controller.setOutput("[Master]", "!crossfaderReverse", kLedBright, true);
        if (engine.getValue("[Channel1]", "orientation") === 0) { engine.setValue("[Channel1]", "orientation", 2); }
        if (engine.getValue("[Channel3]", "orientation") === 0) { engine.setValue("[Channel3]", "orientation", 2); }
        if (engine.getValue("[Channel2]", "orientation") === 2) { engine.setValue("[Channel2]", "orientation", 0); }
        if (engine.getValue("[Channel4]", "orientation") === 2) { engine.setValue("[Channel4]", "orientation", 0); }

    } else {
        TraktorZ2.controller.setOutput("[Master]", "!crossfaderReverse", kLedOff, true);
        if (engine.getValue("[Channel1]", "orientation") === 2) { engine.setValue("[Channel1]", "orientation", 0); }
        if (engine.getValue("[Channel3]", "orientation") === 2) { engine.setValue("[Channel3]", "orientation", 0); }
        if (engine.getValue("[Channel2]", "orientation") === 0) { engine.setValue("[Channel2]", "orientation", 2); }
        if (engine.getValue("[Channel4]", "orientation") === 0) { engine.setValue("[Channel4]", "orientation", 2); }
    }
};

TraktorZ2.buttonHandler = function(field) {
    HIDDebug("TraktorZ2: buttonHandler");
    if (field.value === 0) {
        return; // Button released
    }
    script.toggleControl(field.group, field.name);
};

TraktorZ2.Deck.prototype.snapQuantizeButtonHandler = function(field) {
    HIDDebug("TraktorZ2: snapQuantizeButtonHandler");

    // Depending on long or short press, sync beat or go to key sync mode
    if (field.value === 1) {   // Start timer to measure how long button is pressed
        const ch = this.activeChannel; // Use variable in timer function, because this.activeChannel can change until the timer is active
        TraktorZ2.snapQuantizePressedTimer[ch] = engine.beginTimer(300, function() {
            TraktorZ2.snapQuantizePressed[ch] = true;

            // Reset sync button timer state if active
            if (TraktorZ2.snapQuantizePressedTimer[ch] !== 0) {
                TraktorZ2.snapQuantizePressedTimer[ch] = 0;
            }
        }, true);
    } else {
        TraktorZ2.snapQuantizePressed[this.activeChannel] = false;
        // Change display values to loop/beatjump

        if (TraktorZ2.snapQuantizePressedTimer[this.activeChannel] !== 0) {
            // Timer still running -> stop it
            engine.stopTimer(TraktorZ2.snapQuantizePressedTimer[this.activeChannel]);

            if (TraktorZ2.shiftState !== 0) {
                // Adjust Beatgrid to current trackposition
                script.triggerControl(this.activeChannel, "beats_translate_curpos");
            } else {
                script.toggleControl(this.activeChannel, "quantize");
            }
        }
    }

};

TraktorZ2.Deck.prototype.pflButtonHandler = function(field) {
    HIDDebug("TraktorZ2: pflButtonHandler");
    if (field.value === 0) {
        return; // Button released
    }

    let group;
    if (TraktorZ2.shiftState !== 0) {
        // Shift mode on  -> DeckC / DeckD
        if (this.activeChannel === "[Channel1]") {
            group = "[Channel3]";
        } else {
            group = "[Channel4]";
        }
    } else {
        // Shift mode off -> DeckA / DeckB
        group = this.activeChannel;
    }

    script.toggleControl(group, field.name);
};

TraktorZ2.pflOutputHandler = function(value, group, key) {

    // TODO: Implement Channel A/C B/D switch here

    let ledValue = value;
    if (value === 0 || value === false) {
        // Off value
        ledValue = kLedOff;
    } else if (value === 1 || value === true) {
        // On value
        ledValue = kLedBright;
    }

    TraktorZ2.controller.setOutput(group, key, ledValue);
};

TraktorZ2.microphoneButtonHandler = function(field) {
    HIDDebug("TraktorZ2: microphoneButtonHandler" + " field.value: " + field.value);
    if (field.value === 1) {
        // Microphone button pressed
        if (TraktorZ2.shiftState & 0x01) {
            if (TraktorZ2.dataF1[1] & 0x40) {
                TraktorZ2.dataF1[1] &= ~0x40;
            } else {
                TraktorZ2.dataF1[1] |= 0x40;
            }
            HIDDebug(TraktorZ2.dataF1[1]);
            controller.sendFeatureReport(TraktorZ2.dataF1, 0xF1);
        }
        TraktorZ2.microphoneButtonOutputHandler();
    }
};

TraktorZ2.microphoneButtonStatusHandler = function(field) {
    HIDDebug("TraktorZ2: microphoneButtonStatusHandler" + " field.value: " + field.value);
    TraktorZ2.microphoneButtonStatus = field.value;
    TraktorZ2.microphoneButtonOutputHandler();
};

TraktorZ2.microphoneButtonOutputHandler = function() {
    HIDDebug("TraktorZ2: microphoneButtonOutputHandler");
    if (TraktorZ2.dataF1[1] & 0x40) {
        // Mic/Aux in internal mixing mode
        HIDDebug("TraktorZ2: microphoneButtonStatusHandler: internal");
        if (TraktorZ2.microphoneButtonStatus === 0) {
            TraktorZ2.controller.setOutput("[Master]", "!microphonebuttonled", kLedOff, true); // Controller internal state OFF -> Switch LED to represent this state
        } else {
            TraktorZ2.controller.setOutput("[Master]", "!microphonebuttonled", kLedDimmed, true); // Controller internal state ON -> Switch LED to represent this state
        }
    } else {

        HIDDebug("TraktorZ2: microphoneButtonStatusHandler: external");
        if (TraktorZ2.microphoneButtonStatus === 0) {
            TraktorZ2.controller.setOutput("[Master]", "!microphonebuttonled", kLedOff, true); // Controller internal state OFF -> Switch LED to represent this state
        } else {
            TraktorZ2.controller.setOutput("[Master]", "!microphonebuttonled", kLedBright, true); // Controller internal state ON -> Switch LED to represent this state
        }
    }
};

TraktorZ2.Deck.prototype.traktorButtonHandler = function(field) {
    HIDDebug("TraktorZ2: traktorButtonHandler" + " this.activeChannel: " + this.activeChannel + " field.value: " + field.value);
    if (field.value === 1) {
        // Taktor button pressed
        if (TraktorZ2.shiftState & 0x01) {
            script.toggleControl(this.activeChannel, "passthrough");
            TraktorZ2.traktorButtonOutputHandler(this.activeChannel);
        }
    }
};

TraktorZ2.Deck.prototype.traktorButtonStatusHandler = function(field) {
    HIDDebug("TraktorZ2: traktorButtonStatusHandler" + " this.activeChannel: " + this.activeChannel + " field.value: " + field.value);
    TraktorZ2.traktorButtonStatus[this.activeChannel] = field.value;
    TraktorZ2.traktorButtonOutputHandler(this.activeChannel);
};

TraktorZ2.traktorButtonOutputHandler = function(group) {
    HIDDebug("TraktorZ2: traktorButtonOutputHandler");

    if (TraktorZ2.traktorButtonStatus[group] === 1) {
        if (engine.getValue(group, "passthrough") === 1) {
            TraktorZ2.controller.setOutput(group, "!traktorbuttonled", kLedDimmed, true); // Controller internal state ON -> Switch LED to represent this state
        } else {
            TraktorZ2.controller.setOutput(group, "!traktorbuttonled", kLedBright, true); // Controller internal state OFF -> Switch LED to represent this state
        }
    } else {
        // Channel in standalone mixing mode. No external control possible until traktor button is pressed
        TraktorZ2.controller.setOutput(group, "!traktorbuttonled", kLedOff, true); // Controller internal state OFF -> Switch LED to represent this state
    }
};

TraktorZ2.Deck.prototype.registerInputs2Decks = function() {
    HIDDebug("TraktorZ2: Deck.prototype.registerInputs2Decks");
    const deckFn = TraktorZ2.Deck.prototype;

    this.defineButton(TraktorZ2.inputReport01, "!pad_1", 0x06, 0x04, 0x07, 0x08, deckFn.numberButtonHandler);
    this.defineButton(TraktorZ2.inputReport01, "!pad_2", 0x06, 0x08, 0x07, 0x10, deckFn.numberButtonHandler);
    this.defineButton(TraktorZ2.inputReport01, "!pad_3", 0x06, 0x10, 0x07, 0x20, deckFn.numberButtonHandler);
    this.defineButton(TraktorZ2.inputReport01, "!pad_4", 0x06, 0x20, 0x07, 0x40, deckFn.numberButtonHandler);

    // Traktor buttons
    this.defineButton(TraktorZ2.inputReport01, "!traktorbutton", 0x03, 0x01, 0x03, 0x02, deckFn.traktorButtonHandler);
    this.defineButton(TraktorZ2.inputReport01, "!stateOfTraktorbutton", 0x09, 0x08, 0x09, 0x10, deckFn.traktorButtonStatusHandler);

    // Quantize buttons
    this.defineButton(TraktorZ2.inputReport01, "quantize", 0x03, 0x04, 0x03, 0x10, deckFn.snapQuantizeButtonHandler);

    // PFL Headphone CUE buttons
    this.defineButton(TraktorZ2.inputReport01, "pfl", 0x04, 0x04, 0x04, 0x08, deckFn.pflButtonHandler);

    // Vinyl control mode (REL / INTL)
    this.defineButton(TraktorZ2.inputReport01, "vinylcontrol_mode", 0x04, 0x10, 0x04, 0x20, deckFn.vinylcontrolHandler);
    this.defineButton(TraktorZ2.inputReport01, "!sync", 0x04, 0x40, 0x04, 0x80, deckFn.syncHandler);

    // Load/Duplicate buttons
    this.defineButton(TraktorZ2.inputReport01, "!LoadSelectedTrack", 0x04, 0x01, 0x04, 0x02, deckFn.loadTrackHandler);

    // Loop control
    this.defineButton(TraktorZ2.inputReport01, "!SelectLoop", 0x01, 0xF0, 0x02, 0x0F, deckFn.selectLoopHandler);
    this.defineButton(TraktorZ2.inputReport01, "!ActivateLoop", 0x05, 0x40, 0x08, 0x20, deckFn.activateLoopHandler);

    // Flux / Tap
    this.defineButton(TraktorZ2.inputReport01, "!slip_enabled", 0x06, 0x40, 0x07, 0x80, deckFn.fluxHandler);

};


TraktorZ2.registerInputPackets = function() {
    HIDDebug("TraktorZ2: registerInputPackets");

    this.inputReport01 = new HIDPacket("InputReport01", 0x01, this.messageCallback);
    this.inputReport02 = new HIDPacket("InputReport02", 0x02, this.messageCallback);

    // Register inputs, which only exist on the 2 main decks
    TraktorZ2.Decks.deck1.registerInputs2Decks();
    TraktorZ2.Decks.deck2.registerInputs2Decks();

    this.registerInputButton(TraktorZ2.inputReport01, "[Channel1]", "switchDeck", 0x06, 0x02, this.deckSwitchHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[Channel2]", "switchDeck", 0x07, 0x02, this.deckSwitchHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[Channel3]", "switchDeck", 0x06, 0x01, this.deckSwitchHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[Channel4]", "switchDeck", 0x07, 0x04, this.deckSwitchHandler);

    this.registerInputButton(TraktorZ2.inputReport01, "[Master]", "maximize_library", 0x03, 0x08, this.buttonHandler);

    this.registerInputButton(TraktorZ2.inputReport01, "[Master]", "!microphoneButton", 0x05, 0x01, this.microphoneButtonHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[Master]", "!stateOfMicrophoneButton", 0x09, 0x01, this.microphoneButtonStatusHandler);

    this.registerInputButton(TraktorZ2.inputReport01, "[Master]", "shift", 0x07, 0x01, this.shiftHandler);

    this.registerInputButton(TraktorZ2.inputReport01, "[Master]", "!SelectTrack", 0x01, 0x0F, this.selectTrackHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[Master]", "!LibraryFocus", 0x03, 0x80, this.LibraryFocusHandler);


    this.registerInputButton(TraktorZ2.inputReport01, "[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x05, 0x04, this.buttonHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x05, 0x08, this.buttonHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x08, 0x02, this.buttonHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x08, 0x04, this.buttonHandler);

    this.registerInputButton(TraktorZ2.inputReport01, "[EffectRack1_EffectUnit1]", "!enabled", 0x05, 0x02, this.fxOnClickHandler);
    this.registerInputButton(TraktorZ2.inputReport01, "[EffectRack1_EffectUnit2]", "!enabled", 0x08, 0x01, this.fxOnClickHandler);

    this.registerInputButton(TraktorZ2.inputReport01, "[Master]", "!crossfaderReverse", 0x08, 0x80, this.crossfaderReverseHandler);


    this.controller.registerInputPacket(TraktorZ2.inputReport01);


    this.registerInputScaler(TraktorZ2.inputReport02, "[EffectRack1_EffectUnit1]", "mix", 0x0D, 0xFFFF, this.parameterHandler); // MACRO FX1 D/W
    this.registerInputScaler(TraktorZ2.inputReport02, "[EffectRack1_EffectUnit1]", "super1", 0x0F, 0xFFFF, this.parameterHandler); // MACRO FX1 FX
    this.registerInputScaler(TraktorZ2.inputReport02, "[EffectRack1_EffectUnit2]", "mix", 0x1B, 0xFFFF, this.parameterHandler); // MACRO FX2 D/W
    this.registerInputScaler(TraktorZ2.inputReport02, "[EffectRack1_EffectUnit2]", "super1", 0x1D, 0xFFFF, this.parameterHandler); // MACRO FX2 FX

    this.registerInputScaler(TraktorZ2.inputReport02, "[Channel1]", "volume", 0x2D, 0xFFFF, this.faderHandler); // Fader Deck A
    this.registerInputScaler(TraktorZ2.inputReport02, "[Channel2]", "volume", 0x2F, 0xFFFF, this.faderHandler); // Fader Deck B

    //this.registerInputScaler(TraktorZ2.inputReport02, "[Master]", "duckStrengh", 0x03, 0xFFFF, this.parameterHandler); // Mic/Aux Tone knob, where no 1:1 mapping is available
    this.registerInputScaler(TraktorZ2.inputReport02, "[Microphone]", "pregain", 0x01, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(TraktorZ2.inputReport02, "[Channel1]", "pregain", 0x11, 0xFFFF, this.pregainHandler); // Rotary knob Deck A
    this.registerInputScaler(TraktorZ2.inputReport02, "[Channel2]", "pregain", 0x1F, 0xFFFF, this.pregainHandler); // Rotary knob Deck B
    this.registerInputScaler(TraktorZ2.inputReport02, "[Channel3]", "pregain", 0x29, 0xFFFF, this.pregainHandler); // Rotary knob Deck C
    this.registerInputScaler(TraktorZ2.inputReport02, "[Channel4]", "pregain", 0x2B, 0xFFFF, this.pregainHandler); // Rotary knob Deck D

    this.registerInputScaler(TraktorZ2.inputReport02, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x13, 0xFFFF, this.eqKnobHandler); // High
    this.registerInputScaler(TraktorZ2.inputReport02, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x15, 0xFFFF, this.eqKnobHandler); // Mid
    this.registerInputScaler(TraktorZ2.inputReport02, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x17, 0xFFFF, this.eqKnobHandler); // Low

    this.registerInputScaler(TraktorZ2.inputReport02, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x21, 0xFFFF, this.eqKnobHandler); // High
    this.registerInputScaler(TraktorZ2.inputReport02, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x23, 0xFFFF, this.eqKnobHandler); // Mid
    this.registerInputScaler(TraktorZ2.inputReport02, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x25, 0xFFFF, this.eqKnobHandler); // Low

    this.registerInputScaler(TraktorZ2.inputReport02, "[QuickEffectRack1_[Channel1]]", "super1", 0x19, 0xFFFF, this.eqKnobHandler);
    this.registerInputScaler(TraktorZ2.inputReport02, "[QuickEffectRack1_[Channel2]]", "super1", 0x27, 0xFFFF, this.eqKnobHandler);

    this.registerInputScaler(TraktorZ2.inputReport02, "[Master]", "crossfader", 0x31, 0xFFFF, this.faderHandler);
    //this.registerInputScaler(TraktorZ2.inputReport02, "[Master]", "gain", 0x09, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(TraktorZ2.inputReport02, "[Master]", "headMix", 0x07, 0xFFFF, this.parameterHandler);
    //this.registerInputScaler(TraktorZ2.inputReport02, "[Master]", "headGain", 0x05, 0xFFFF, this.parameterHandler);

    this.controller.registerInputPacket(TraktorZ2.inputReport02);
};


TraktorZ2.enableSoftTakeover = function() {
    // Soft takeovers
    for (let ch = 1; ch <= 2; ch++) {
        const group = "[Channel" + ch + "]";
        engine.softTakeover("[QuickEffectRack1_" + group + "]", "super1", true);
    }

    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", true);

    // engine.softTakeover("[Master]", "crossfader", true); // softTakeover might be a performance issue
    engine.softTakeover("[Master]", "gain", true);
    engine.softTakeover("[Master]", "headMix", true);
    engine.softTakeover("[Master]", "headGain", true);
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
        TraktorZ2.controller.setOutput("[Master]", "shift", kLedBright, true);

        TraktorZ2.shiftPressedTimer = engine.beginTimer(200, function() {
            // Reset sync button timer state if active
            if (TraktorZ2.shiftPressedTimer !== 0) {
                TraktorZ2.shiftPressedTimer = 0;
            }
            // Change display values to beatloopsize
            TraktorZ2.displayLoopCount("[Channel1]", false);
            TraktorZ2.displayLoopCount("[Channel2]", true);
            HIDDebug("TraktorZ2: shift unlocked");
        }, true);

        HIDDebug("TraktorZ2: shift pressed");
    } else if (TraktorZ2.shiftPressed === true && field.value === 0) {

        TraktorZ2.shiftPressed = false;

        HIDDebug("TraktorZ2: shift button released" + TraktorZ2.shiftState);
        if (TraktorZ2.shiftPressedTimer !== 0) {
            if (TraktorZ2.shiftState & 0x02) {
                // Timer still running -> stop it and set LED depending on previous lock state
                TraktorZ2.shiftState = 0x00;
                TraktorZ2.controller.setOutput("[Master]", "shift", kLedOff, false);
                TraktorZ2.vinylcontrolOutputHandler(0, "[Channel1]", "Shift");
                TraktorZ2.vinylcontrolOutputHandler(0, "[Channel2]", "Shift");
            } else {
                TraktorZ2.shiftState = 0x02;
                TraktorZ2.controller.setOutput("[Master]", "shift", kLedDimmed, true);
                TraktorZ2.vinylcontrolOutputHandler(1, "[Channel1]", "Shift");
                TraktorZ2.vinylcontrolOutputHandler(1, "[Channel2]", "Shift");
            }
            engine.stopTimer(TraktorZ2.shiftPressedTimer);
            // Change display values beatjumpsize / beatloopsize
            TraktorZ2.displayLoopCount("[Channel1]", false);
            TraktorZ2.displayLoopCount("[Channel2]", true);
            HIDDebug("TraktorZ2: static shift state changed to: " + TraktorZ2.shiftState);
        } else {
            if (TraktorZ2.shiftState & 0x02) {
                TraktorZ2.shiftState = 0x02;
                TraktorZ2.controller.setOutput("[Master]", "shift", kLedDimmed, true);
            } else {
                TraktorZ2.shiftState = 0x00;
                TraktorZ2.controller.setOutput("[Master]", "shift", kLedOff, true);
            }
            HIDDebug("TraktorZ2: back to static shift state: " + TraktorZ2.shiftState);
        }
        // Apply stored EQ and filter settings
        const eqGroups = {
            "1": "[EqualizerRack1_[Channel1]_Effect1]",
            "2": "[EqualizerRack1_[Channel1]_Effect1]",
            "3": "[EqualizerRack1_[Channel1]_Effect1]",
            "4": "[QuickEffectRack1_[Channel1]]",
            "5": "[EqualizerRack1_[Channel2]_Effect1]",
            "6": "[EqualizerRack1_[Channel2]_Effect1]",
            "7": "[EqualizerRack1_[Channel2]_Effect1]",
            "8": "[QuickEffectRack1_[Channel2]]"
        };
        const eqParameters = {
            "1": "parameter1",
            "2": "parameter2",
            "3": "parameter3",
            "4": "super1",
            "5": "parameter1",
            "6": "parameter2",
            "7": "parameter3",
            "8": "super1"
        };

        for (const idx in eqGroups) {

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
};

TraktorZ2.eqExecute = function(group, name, value) {
    HIDDebug("TraktorZ2: eqExecute");
    if ((group === "[EqualizerRack1_[Channel1]_Effect1]") ||
        (group === "[QuickEffectRack1_[Channel1]]")) {
        if (TraktorZ2.pregainCh3Timer !== 0) {
            engine.stopTimer(TraktorZ2.pregainCh3Timer);
            TraktorZ2.pregainCh3Timer = 0;
            TraktorZ2.displayVuValue(engine.getValue("[Channel1]", "VuMeter"), "[Channel1]", "VuMeter");
            TraktorZ2.displayPeakIndicator(engine.getValue("[Channel1]", "PeakIndicator"), "[Channel1]", "PeakIndicator");
        }
    } else if ((group === "[EqualizerRack1_[Channel2]_Effect1]") ||
        (group === "[QuickEffectRack1_[Channel2]]")) {
        if (TraktorZ2.pregainCh4Timer !== 0) {
            engine.stopTimer(TraktorZ2.pregainCh4Timer);
            TraktorZ2.pregainCh4Timer = 0;
            TraktorZ2.displayVuValue(engine.getValue("[Channel2]", "VuMeter"), "[Channel2]", "VuMeter");
            TraktorZ2.displayPeakIndicator(engine.getValue("[Channel2]", "PeakIndicator"), "[Channel2]", "PeakIndicator");
        }
    }
    engine.setParameter(group, name, value / 4095);
    TraktorZ2.eqValueStorage[group + name + "changed"] = false;
};

TraktorZ2.pregainHandler = function(field) {
    HIDDebug("TraktorZ2: pregainHandler");
    engine.setParameter(field.group, field.name, field.value / 4095);
    if ((field.group === "[Channel1]") && (TraktorZ2.pregainCh3Timer !== 0)) {
        engine.stopTimer(TraktorZ2.pregainCh3Timer);
        TraktorZ2.pregainCh3Timer = 0;
        TraktorZ2.displayVuValue(engine.getValue("[Channel1]", "VuMeter"), "[Channel1]", "VuMeter");
        TraktorZ2.displayPeakIndicator(engine.getValue("[Channel1]", "PeakIndicator"), "[Channel1]", "PeakIndicator");
    }
    if (field.group === "[Channel3]") {
        if (TraktorZ2.pregainCh3Timer !== 0) {
            engine.stopTimer(TraktorZ2.pregainCh3Timer);
        }
        TraktorZ2.displayVuValue(engine.getValue("[Channel3]", "VuMeter"), "[Channel3]", "VuMeter");
        TraktorZ2.displayPeakIndicator(engine.getValue("[Channel3]", "PeakIndicator"), "[Channel3]", "PeakIndicator");
        TraktorZ2.pregainCh3Timer = engine.beginTimer(2500, function() {
            TraktorZ2.pregainCh3Timer = 0;
            TraktorZ2.displayVuValue(engine.getValue("[Channel1]", "VuMeter"), "[Channel1]", "VuMeter");
            TraktorZ2.displayPeakIndicator(engine.getValue("[Channel1]", "PeakIndicator"), "[Channel1]", "PeakIndicator");
        }, true);
    }
    if ((field.group === "[Channel2]") && (TraktorZ2.pregainCh4Timer !== 0)) {
        engine.stopTimer(TraktorZ2.pregainCh4Timer);
        TraktorZ2.pregainCh4Timer = 0;
        TraktorZ2.displayVuValue(engine.getValue("[Channel2]", "VuMeter"), "[Channel2]", "VuMeter");
        TraktorZ2.displayPeakIndicator(engine.getValue("[Channel2]", "PeakIndicator"), "[Channel2]", "PeakIndicator");
    }
    if (field.group === "[Channel4]") {
        if (TraktorZ2.pregainCh4Timer !== 0) {
            engine.stopTimer(TraktorZ2.pregainCh4Timer);
        }
        TraktorZ2.displayVuValue(engine.getValue("[Channel4]", "VuMeter"), "[Channel4]", "VuMeter");
        TraktorZ2.displayPeakIndicator(engine.getValue("[Channel4]", "PeakIndicator"), "[Channel4]", "PeakIndicator");
        TraktorZ2.pregainCh4Timer = engine.beginTimer(2500, function() {
            TraktorZ2.pregainCh4Timer = 0;
            TraktorZ2.displayVuValue(engine.getValue("[Channel2]", "VuMeter"), "[Channel2]", "VuMeter");
            TraktorZ2.displayPeakIndicator(engine.getValue("[Channel2]", "PeakIndicator"), "[Channel2]", "PeakIndicator");
        }, true);
    }
};

TraktorZ2.faderHandler = function(field) {
    // To ensure that the faders always shut completely,
    // all values below 100 are rated as zero,
    // and all values from 3995 to 4095 are rated as one.
    // Todo: NI provides a tool, to calibrate the Z2 faders.
    //       If the interaction between this calibration tool
    //       and the values in Mixxx is understood,
    //       this implementation might be improved
    engine.setParameter(field.group, field.name, script.absoluteLin(field.value, 0, 1, 100, 3995));
};

TraktorZ2.messageCallback = function(_packet, data) {
    for (const name in data) {
        if (Object.prototype.hasOwnProperty.call(data, name)) {
            TraktorZ2.controller.processButton(data[name]);
        }
    }
};

TraktorZ2.incomingData = function(data, length) {
    //HIDDebug("TraktorZ2: incomingData data:" + data + "   length:" + length);
    TraktorZ2.controller.parsePacket(data, length);
};

TraktorZ2.shutdown = function() {

    TraktorZ2.controller.setOutput("[Master]", "!usblight", kLedBright, true);

    // Switch software mixing mode of and given LED control to mixer hardware
    TraktorZ2.dataF1[0] = 0x00;
    controller.sendFeatureReport(TraktorZ2.dataF1, 0xF1);

    HIDDebug("TraktorZ2: Shutdown done!");
};

TraktorZ2.debugLights = function() {

    HIDDebug("TraktorZ2: debugLights");
    // Call this if you want to just send raw packets to the controller (good for figuring out what
    // bytes do what).
    const dataA = [
        /* 0x80 */
        0x00,  // 0x01 7 bits control Warning Symbol on top left brightness (orange)
        0x00,  // 0x02 7 bits control Timecode-Vinyl Symbol on top right brightness (orange)
        0x00,  // 0x03 7 bits control Snap-Button S brightness (blue)
        0x00,  // 0x04 7 bits control Quantize-Button Q brightness (blue)
        0x00,  // 0x05 7 bits control Settings-Button (Gear-Wheel-Symbol) brightness (orange)
        0x00,  // 0x06 7 bits control SHIFT-Button brightness (white)
        0x00,  // 0x07 7 bits control Deck A button brightness (blue)
        0x00,  // 0x08 7 bits control Deck B button brightness (blue)
        0x00,  // 0x09 7 bits control Deck C button brightness (white)
        0x00,  // 0x0A 7 bits control Deck D button brightness (white)

        0x00,  // 0x0B 7 bits control Deck C volume text label backlight brightness (white)
        0x00,  // 0x0C 7 bits control Deck D volume text label backlight brightness (white)

        0x00,  // 0x0D 7 bits control Macro FX1 On button brightness (orange)
        0x00,  // 0x0E 7 bits control Deck 1 Flux button brightness (orange)
        0x00,  // 0x0F 7 bits control Channel 1 FX1 select button brightness (orange)
        0x00,  // 0x10 7 bits control Channel 1 FX2 select button brightness (orange)
        0x00,  // 0x11 7 bits control Load A button brightness (orange)
        0x70,  // 0x12 7 bits control vinylcontrol Rel/Intl A button brightness (orange)
        0x00,  // 0x13 7 bits control vinylcontrol Rel/Intl A button brightness (green)
        0x00,  // 0x14 7 bits control Sync A button brightness (orange)

        0x00,  // 0x15 7 bits control Macro FX2 On button brightness (orange)
        0x00,  // 0x16 7 bits control Deck 2 Flux button brightness (orange)
        0x00,  // 0x17 7 bits control Channel 2 FX1 select button brightness (orange)
        0x00,  // 0x18 7 bits control Channel 2 FX2 select button brightness (orange)
        0x00,  // 0x19 7 bits control Load B button brightness (orange)
        0x70,  // 0x1A 7 bits control vinylcontrol Rel/Intl B button brightness (orange)
        0x30,  // 0x1B 7 bits control vinylcontrol Rel/Intl B button brightness (green)
        0x00,  // 0x1C 7 bits control Sync B button brightness (orange)
        0x00, 0x10, 0x00, // 0x1D HotCue 1 Deck 1 RGB
        0x00, 0x1F, 0x00, // 0x20 HotCue 2 Deck 1 RGB
        0x00, 0x20, 0x00, // 0x23 HotCue 3 Deck 1 RGB
        0x00, 0x2F, 0x00, // 0x26 HotCue 4 Deck 1 RGB
        0x00, 0x00, 0x00, // 0x29 HotCue 1 Deck 2 RGB
        0x00, 0x00, 0x00, // 0x2C HotCue 2 Deck 2 RGB
        0x00, 0x00, 0x00, // 0x2F HotCue 3 Deck 2 RGB
        0x00, 0x00, 0x00, // 0x32 HotCue 4 Deck 2 RGB

        0x00,  // 0x35 7 bits control Deck 1 1st 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x36 7 bits control Deck 1 1st 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x37 7 bits control Deck 1 1st 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x38 7 bits control Deck 1 1st 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x39 7 bits control Deck 1 1st 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x3A 7 bits control Deck 1 1st 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x3B 7 bits control Deck 1 1st 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x3C 7 bits control Deck 1 2nd 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x3D 7 bits control Deck 1 2nd 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x3E 7 bits control Deck 1 2nd 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x3F 7 bits control Deck 1 2nd 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x40 7 bits control Deck 1 2nd 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x41 7 bits control Deck 1 2nd 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x42 7 bits control Deck 1 2nd 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x43 7 bits control Deck 1 3rd 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x44 7 bits control Deck 1 3rd 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x45 7 bits control Deck 1 3rd 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x46 7 bits control Deck 1 3rd 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x47 7 bits control Deck 1 3rd 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x48 7 bits control Deck 1 3rd 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x49 7 bits control Deck 1 3rd 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x4A 7 bits control Deck 2 1st 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x4B 7 bits control Deck 2 1st 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x4C 7 bits control Deck 2 1st 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x4D 7 bits control Deck 2 1st 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x4E 7 bits control Deck 2 1st 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x4F 7 bits control Deck 2 1st 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x50 7 bits control Deck 2 1st 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x51 7 bits control Deck 2 2nd 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x52 7 bits control Deck 2 2nd 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x53 7 bits control Deck 2 2nd 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x54 7 bits control Deck 2 2nd 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x55 7 bits control Deck 2 2nd 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x56 7 bits control Deck 2 2nd 7 segment lower left vertical bar brightness (orange)
        0x00,  // 0x57 7 bits control Deck 2 2nd 7 segment lower horizontal bar brightness (orange)

        0x00,  // 0x58 7 bits control Deck 2 3rd 7 segment center horizontal bar brightness (orange)
        0x00,  // 0x59 7 bits control Deck 2 3rd 7 segment lower right vertical bar brightness (orange)
        0x00,  // 0x5A 7 bits control Deck 2 3rd 7 segment upper right vertical bar brightness (orange)
        0x00,  // 0x5B 7 bits control Deck 2 3rd 7 segment upper horizontal bar brightness (orange)
        0x00,  // 0x5C 7 bits control Deck 2 3rd 7 segment upper left vertical bar brightness (orange)
        0x00,  // 0x5D 7 bits control Deck 2 3rd 7 segment lower left vertical bar brightness (orange)
        0x00   // 0x5E 7 bits control Deck 2 3rd 7 segment lower horizontal bar brightness (orange)
    ];
    controller.send(dataA, dataA.length, 0x80);

    const dataB = [
        /* 0x81 */
        0x00,  // 0x01 7 bits control VU meter label "A"  (white)
        0x00,  // 0x02 7 bits control VU meter -15dBa ChA (blue)
        0x00,  // 0x03 7 bits control VU meter  -6dBa ChA (blue)
        0x00,  // 0x04 7 bits control VU meter  -3dBa ChA (blue)
        0x00,  // 0x05 7 bits control VU meter   0dBa ChA (blue)
        0x00,  // 0x06 7 bits control VU meter  +3dBa ChA (orange)
        0x00,  // 0x07 7 bits control VU meter  +6dBa ChA (orange)
        0x00,  // 0x08 7 bits control VU meter   CLIP ChA (orange)

        0x00,  // 0x09 7 bits control VU meter label "B"  (white)
        0x00,  // 0x0A 7 bits control VU meter -15dBa ChB (blue)
        0x00,  // 0x0B 7 bits control VU meter  -6dBa ChB (blue)
        0x00,  // 0x0C 7 bits control VU meter  -3dBa ChB (blue)
        0x00,  // 0x0D 7 bits control VU meter   0dBa ChB (blue)
        0x00,  // 0x0E 7 bits control VU meter  +3dBa ChB (orange)
        0x00,  // 0x0F 7 bits control VU meter  +6dBa ChB (orange)
        0x00,  // 0x10 7 bits control VU meter   CLIP ChB (orange)

        0x00,  // 0x11 7 bits control VU meter label "C"  (white)
        0x00,  // 0x12 7 bits control VU meter -15dBa ChC/MasterLeft (blue)
        0x00,  // 0x13 7 bits control VU meter  -6dBa ChC/MasterLeft (blue)
        0x00,  // 0x14 7 bits control VU meter  -3dBa ChC/MasterLeft (blue)
        0x00,  // 0x15 7 bits control VU meter   0dBa ChC/MasterLeft (blue)
        0x00,  // 0x16 7 bits control VU meter  +3dBa ChC/MasterLeft (orange)
        0x00,  // 0x17 7 bits control VU meter  +6dBa ChC/MasterLeft (orange)
        0x00,  // 0x18 7 bits control VU meter   CLIP ChC/MasterLeft (orange)

        0x00,  // 0x19 7 bits control VU meter label "D"  (white)
        0x00,  // 0x1A 7 bits control VU meter -15dBa ChD/MasterRight (blue)
        0x00,  // 0x1B 7 bits control VU meter  -6dBa ChD/MasterRight (blue)
        0x00,  // 0x1C 7 bits control VU meter  -3dBa ChD/MasterRight (blue)
        0x00,  // 0x1D 7 bits control VU meter   0dBa ChD/MasterRight (blue)
        0x00,  // 0x1E 7 bits control VU meter  +3dBa ChD/MasterRight (orange)
        0x00,  // 0x1F 7 bits control VU meter  +6dBa ChD/MasterRight (orange)
        0x00,  // 0x20 7 bits control VU meter   CLIP ChD/MasterRight (orange)

        0x00,  // 0x21 7 bits control VU meter label "MST"  (white)
        0x00,  // 0x22 7 bits control Microphone-Button (orange)
        0x00,  // 0x23 7 bits control Headphone-Button A (blue)
        0x00,  // 0x24 7 bits control Headphone-Button B (blue)
        0x00,  // 0x25 7 bits control Traktor-Button ChA (orange)
        0x00,  // 0x26 7 bits control Traktor-Button ChB (orange)
        0x00,  // 0x27 7 bits control USB-symbol on top (orange)
        0x00   // 0x28 7 bits control VU meter label "XF REVERSE" (orange)
    ];
    controller.send(dataB, dataB.length, 0x81);

};


// outputHandler drives lights that only have one color.
TraktorZ2.basicOutputHandler = function(value, group, key) {
    HIDDebug("TraktorZ2: basicOutputHandler" + " group:" + group + " key:" + key + " value:" + value);
    if (value === 0 || value === false) {
        // Off value
        TraktorZ2.controller.setOutput(group, key, kLedOff, true);
    } else {
        // On value
        TraktorZ2.controller.setOutput(group, key, kLedBright, true);
    }
};


TraktorZ2.hotcueOutputHandler = function() {

    const sideChannel = ["[Channel1]", "[Channel2]"];
    const sideOffset = [0, 0];

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


    for (let chidx = 1; chidx <= 2; chidx++) {
        const ch = "[Channel" + chidx + "]";
        for (let i = 1; i <= 4; i++) {

            let colorCode = engine.getValue(sideChannel[chidx], "hotcue_" + (sideOffset[chidx] + i) + "_color");
            if (engine.getValue(sideChannel[chidx], "hotcue_" + (sideOffset[chidx] + i) + "_enabled") === 0) { colorCode = 0; }
            let red = ((colorCode & 0xFF0000) >> 16);
            let green = ((colorCode & 0x00FF00) >> 8);
            let blue = ((colorCode & 0x0000FF));
            // Scale color up to 100% brightness
            let brightnessCorrectionFactor;
            if ((red > green) && (red > blue)) {
                brightnessCorrectionFactor = kLedBright / red;
            } else if ((green > red) && (green > blue)) {
                brightnessCorrectionFactor = kLedBright / green;
            } else if ((blue > red) && (blue > green)) {
                brightnessCorrectionFactor = kLedBright / blue;
            }
            red *= brightnessCorrectionFactor;
            green *= brightnessCorrectionFactor;
            blue *= brightnessCorrectionFactor;

            // Scale color down to 30% if a saved loop is assigned
            if (engine.getValue(sideChannel[chidx], "hotcue_" + (sideOffset[chidx] + i) + "_type") === 4) {
                red = Math.ceil(red * 0.3);
                green = Math.ceil(green * 0.3);
                blue = Math.ceil(blue * 0.25); // Blue LED is dominant -> dim it slightly
            }
            //HIDDebug("Channel: " + ch + " Hotcue: " + i + " Colorcode: " + colorCode + " Red: " + red + " Green: " + green + " Blue: " + blue);
            TraktorZ2.controller.setOutput(ch, "Hotcue" + i + "Red", red, false);
            TraktorZ2.controller.setOutput(ch, "Hotcue" + i + "Green", green, false);
            TraktorZ2.controller.setOutput(ch, "Hotcue" + i + "Blue", blue, true);
        }
    }
};


TraktorZ2.beatOutputHandler = function(value, group) {
    if (engine.getValue(group, "loop_enabled") && engine.getValue(group, "play")) {
        const playposition = engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
        if (
            (playposition >= engine.getValue(group, "loop_start_position")) &&
            (playposition <= engine.getValue(group, "loop_end_position"))
        ) {
            if (engine.getValue(group, "beatloop_size") >= 0.125) {
                if (value !== 0) {
                    TraktorZ2.displayBrightness[group] = kLedBright;
                } else {
                    TraktorZ2.displayBrightness[group] = kLedDimmed;
                }
            } else if (engine.getValue(group, "beatloop_size") >= 0.0625) {
                if (TraktorZ2.displayBrightness[group] === 0x57) {
                    return;
                } else {
                    TraktorZ2.displayBrightness[group] = 0x57;
                }
            } else {
                if (TraktorZ2.displayBrightness[group] === 0x40) {
                    return;
                } else {
                    TraktorZ2.displayBrightness[group] = 0x40;
                }
            }
            TraktorZ2.displayBeatLeds(group);
            return;
        }
    }

    if (value !== 0) {
        TraktorZ2.displayBrightness[group] = kLedBright;
    } else {
        TraktorZ2.displayBrightness[group] = kLedDimmed;
    }
    TraktorZ2.displayBeatLeds(group);
};

TraktorZ2.updateDisplayOutputHandler = function(value, group) {
    HIDDebug("updateDisplayOutputHandler");
    TraktorZ2.displayLoopCount(group, true);
};

TraktorZ2.displayBeatLeds = function(group) {

    if ((group === "[Channel1]") || (group === "[Channel2]")) {
        TraktorZ2.displayLoopCount(group, false);
    }
    const sendNow = true;
    if (engine.getValue(group, "track_loaded") === 0) {
        TraktorZ2.controller.setOutput(group, "!beatIndicator", kLedOff, sendNow);
    } else {
        TraktorZ2.controller.setOutput(group, "!beatIndicator", TraktorZ2.displayBrightness[group], sendNow);
    }
};

TraktorZ2.displayLoopCount = function(group, sendMessage) {
    // @param group may be either[Channel1] or [Channel2]
    // sendMessage: if true, send HID package immediateley
    // @param TraktorZ2.displayBrightness[group] may be an integer value from 0x00 to 0x07
    let numberToDisplay;

    let displayBrightness;

    if (engine.getValue(group, "track_loaded") === 0) {
        displayBrightness = kLedOff;
    } else if (engine.getValue(group, "loop_enabled") && !(TraktorZ2.shiftState & 0x02)) {
        const playposition = engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
        if (
            (playposition >= engine.getValue(group, "loop_start_position")) &&
            (playposition <= engine.getValue(group, "loop_end_position"))
        ) {
            displayBrightness = TraktorZ2.displayBrightness[group];
        } else {
            displayBrightness = kLedDimmed;
        }
    } else {
        displayBrightness = kLedBright;
    }

    const ledKeyDigitModulus = {
        "[Digit2]": 10,
        "[Digit1]": 100
    };

    const led2DigitModulus = {
        "[Digit3]": 10,
        "[Digit2]": 100
    };

    const led3DigitModulus = {
        "[Digit3]": 10,
        "[Digit2]": 100,
        "[Digit1]": 1000
    };

    if ((TraktorZ2.syncPressed["[Channel1]"] === true) || (TraktorZ2.syncPressed["[Channel2]"] === true)) {
        const key = engine.getValue(group, "key");
        HIDDebug("TraktorZ2: ################ Key:" + key);

        let majorMinor;

        if (key === 1) {
            numberToDisplay = 8; // 1d
            majorMinor = 0x0B;
        } else if (key === 2) {
            numberToDisplay = 3; // 8d
            majorMinor = 0x0B;
        } else if (key === 3) {
            numberToDisplay = 10; // 3d
            majorMinor = 0x0B;
        } else if (key === 4) {
            numberToDisplay = 5; // 10d
            majorMinor = 0x0B;
        } else if (key === 5) {
            numberToDisplay = 12; // 5d
            majorMinor = 0x0B;
        } else if (key === 6) {
            numberToDisplay = 7; // 12d
            majorMinor = 0x0B;
        } else if (key === 7) {
            numberToDisplay = 2; // 7d
            majorMinor = 0x0B;
        } else if (key === 8) {
            numberToDisplay = 9; // 2d
            majorMinor = 0x0B;
        } else if (key === 9) {
            numberToDisplay = 4; // 9d
            majorMinor = 0x0B;
        } else if (key === 10) {
            numberToDisplay = 11; // 4d
            majorMinor = 0x0B;
        } else if (key === 11) {
            numberToDisplay = 6; // 11d
            majorMinor = 0x0B;
        } else if (key === 12) {
            numberToDisplay = 1; // 6d
            majorMinor = 0x0B;
        } else if (key === 13) {
            numberToDisplay = 5; // 10m
            majorMinor = 0x0A;
        } else if (key === 14) {
            numberToDisplay = 12; // 5m
            majorMinor = 0x0A;
        } else if (key === 15) {
            numberToDisplay = 7; // 12m
            majorMinor = 0x0A;
        } else if (key === 16) {
            numberToDisplay = 2; // 7m
            majorMinor = 0x0A;
        } else if (key === 17) {
            numberToDisplay = 9; // 2m
            majorMinor = 0x0A;
        } else if (key === 18) {
            numberToDisplay = 4; // 9m
            majorMinor = 0x0A;
        } else if (key === 19) {
            numberToDisplay = 11; // 4m
            majorMinor = 0x0A;
        } else if (key === 20) {
            numberToDisplay = 6; // 11m
            majorMinor = 0x0A;
        } else if (key === 21) {
            numberToDisplay = 1; // 6m
            majorMinor = 0x0A;
        } else if (key === 22) {
            numberToDisplay = 8; // 1m
            majorMinor = 0x0A;
        } else if (key === 23) {
            numberToDisplay = 3; // 8m
            majorMinor = 0x0A;
        } else if (key === 24) {
            numberToDisplay = 10; // 3m
            majorMinor = 0x0A;
        }

        TraktorZ2.displayLoopCountDigit(group + "[Digit3]", majorMinor, displayBrightness, false);

        // Lancelot key notation integer
        let numberToDisplayRemainder = numberToDisplay;
        for (const digit in ledKeyDigitModulus) {
            let leastSignificiantDigit = (numberToDisplayRemainder % 10);
            numberToDisplayRemainder = numberToDisplayRemainder - leastSignificiantDigit;
            //HIDDebug(leastSignificiantDigit + " " + numberToDisplayRemainder + " " + group + " " + digit);
            if ((digit === "[Digit1]" && numberToDisplay < 10)) {
                leastSignificiantDigit = -2; // Leading zero -> Blank
            }
            if (digit !== "[Digit1]") {
                TraktorZ2.displayLoopCountDigit(group + digit, leastSignificiantDigit, displayBrightness, false);
            } else {
                TraktorZ2.displayLoopCountDigit(group + digit, leastSignificiantDigit, displayBrightness, sendMessage);
            }
            numberToDisplayRemainder /= 10;
        }
        return;
    } else if (TraktorZ2.shiftState & 0x02) {
        numberToDisplay = engine.getValue(group, "beatjump_size");
    } else {
        numberToDisplay = engine.getValue(group, "beatloop_size");
    }

    if (numberToDisplay < 1) {
        // Fraction of a beat
        let numberToDisplayRemainder = 1 / numberToDisplay;
        for (const digit in led2DigitModulus) {
            let leastSignificiantDigit = (numberToDisplayRemainder % 10);
            numberToDisplayRemainder = numberToDisplayRemainder - leastSignificiantDigit;
            //HIDDebug(leastSignificiantDigit + " " + numberToDisplayRemainder + " " + group + " " + digit);
            if (digit === "[Digit2]" && numberToDisplay > .1) {
                leastSignificiantDigit = -1; // Leading zero -> Show special symbol of number 1 and the fraction stroke combined in left digit
            }
            TraktorZ2.displayLoopCountDigit(group + digit, leastSignificiantDigit, displayBrightness, false);
            numberToDisplayRemainder /= 10;
        }
        if (numberToDisplay > .1) {
            TraktorZ2.displayLoopCountDigit(group + "[Digit1]", -2, displayBrightness, sendMessage);  // Leading zero -> Blank
        } else {
            TraktorZ2.displayLoopCountDigit(group + "[Digit1]", -1, displayBrightness, sendMessage); // Show special symbol of number 1 and the fraction stroke combined in left digit
        }
    } else {
        // Beat integer
        let numberToDisplayRemainder = numberToDisplay;
        for (const digit in led3DigitModulus) {
            let leastSignificiantDigit = (numberToDisplayRemainder % 10);
            numberToDisplayRemainder = numberToDisplayRemainder - leastSignificiantDigit;
            //HIDDebug(leastSignificiantDigit + " " + numberToDisplayRemainder + " " + group + " " + digit);
            if ((digit === "[Digit1]" && numberToDisplay < 100) || (digit === "[Digit2]" && numberToDisplay < 10)) {
                leastSignificiantDigit = -2; // Leading zero -> Blank
            }
            if (digit !== "[Digit1]") {
                TraktorZ2.displayLoopCountDigit(group + digit, leastSignificiantDigit, displayBrightness, false);
            } else {
                TraktorZ2.displayLoopCountDigit(group + digit, leastSignificiantDigit, displayBrightness, sendMessage);
            }
            numberToDisplayRemainder /= 10;
        }
    }
};

TraktorZ2.displayLoopCountDigit = function(group, digit, brightness, sendMessage) {
    // @param offset of the first LED (center horizontal bar) of the digit
    // @param digit to display (-2 represents all OFF, -1 represents "1/" )
    // @param brightness may be aninteger value from 0x00 to 0x07
    // HIDDebug("Offset:" + " Digit:" + digit + " Brightness:" + brightness);

    // Segment a (upper horizontal bar)
    if (digit === 0 || digit === 2 || digit === 3 || digit === 5 || digit === 6 || digit === 7 || digit === 8 || digit === 9 || digit === 0x0A) {
        TraktorZ2.controller.setOutput(group, "segment_a", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_a", kLedOff, false); // OFF
    }

    // Segment b (upper right vertical bar)
    if (digit === 0 || digit === 1 || digit === 2 || digit === 3 || digit === 4 || digit === 7 || digit === 8 || digit === 9 || digit === -1 || digit === 0x0A) {
        TraktorZ2.controller.setOutput(group, "segment_b", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_b", kLedOff, false); // OFF
    }

    // Segment c (lower right vertical bar)
    if (digit === 0 || digit === 1 || digit === 3 || digit === 4 || digit === 5 || digit === 6 || digit === 7 || digit === 8 || digit === 9 || digit === 0x0A || digit === 0x0B) {
        TraktorZ2.controller.setOutput(group, "segment_c", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_c", kLedOff, false); // OFF
    }

    // Segment d (lower horizontal bar)
    if (digit === 0 || digit === 2 || digit === 3 || digit === 5 || digit === 6 || digit === 8 || digit === 9 || digit === 0x0B) {
        TraktorZ2.controller.setOutput(group, "segment_d", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_d", kLedOff, false); // OFF
    }

    // Segment e (lower left vertical bar)
    if (digit === 0 || digit === 2 || digit === 6 || digit === 8 || digit === -1 || digit === 0x0A || digit === 0x0B) {
        TraktorZ2.controller.setOutput(group, "segment_e", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_e", kLedOff, false); // OFF
    }

    // Segment f (upper left vertical bar)
    if (digit === 0 || digit === 4 || digit === 5 || digit === 6 || digit === 8 || digit === 9 || digit === -1 || digit === 0x0A || digit === 0x0B) {
        TraktorZ2.controller.setOutput(group, "segment_f", brightness, false); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_f", kLedOff, false); // OFF
    }

    // Segment g (center horizontal bar)
    if (digit === 2 || digit === 3 || digit === 4 || digit === 5 || digit === 6 || digit === 8 || digit === 9 || digit === 0x0A || digit === 0x0B) {
        TraktorZ2.controller.setOutput(group, "segment_g", brightness, sendMessage); // ON
    } else {
        TraktorZ2.controller.setOutput(group, "segment_g", kLedOff, sendMessage); // OFF
    }
};

TraktorZ2.Deck.prototype.registerOutputs2Decks = function() {
    HIDDebug("TraktorZ2: Deck.prototype.registerOutputs2Decks");

    this.defineLED2(TraktorZ2.outputReport80, "slip_enabled", 0x0E, 0x16);
    engine.makeConnection(this.activeChannel, "slip_enabled", TraktorZ2.basicOutputHandler);
    engine.trigger(this.activeChannel, "slip_enabled");

    this.defineLED2(TraktorZ2.outputReport80, "!vinylcontrol_orange", 0x12, 0x1A);
    this.defineLED2(TraktorZ2.outputReport80, "!vinylcontrol_green", 0x13, 0x1B);
    engine.makeConnection(this.activeChannel, "vinylcontrol_status", TraktorZ2.vinylcontrolStatusOutputHandler);
    engine.makeConnection(this.activeChannel, "vinylcontrol_mode", TraktorZ2.vinylcontrolOutputHandler);
    engine.makeConnection(this.activeChannel, "cue_indicator", TraktorZ2.vinylcontrolOutputHandler);
    engine.makeConnection(this.activeChannel, "play_indicator", TraktorZ2.vinylcontrolOutputHandler);
    engine.makeConnection(this.activeChannel, "vinylcontrol_enabled", TraktorZ2.vinylcontrolOutputHandler);
    engine.trigger(this.activeChannel, "vinylcontrol_status");
    engine.trigger(this.activeChannel, "vinylcontrol_mode");
    engine.trigger(this.activeChannel, "cue_indicator");
    engine.trigger(this.activeChannel, "play_indicator");
    engine.trigger(this.activeChannel, "vinylcontrol_enabled");

    this.defineLED2(TraktorZ2.outputReport80, "sync_mode", 0x14, 0x1C);
    engine.makeConnection(this.activeChannel, "sync_mode", TraktorZ2.basicOutputHandler);
    engine.trigger(this.activeChannel, "sync_mode");

    // Headphone button LEDs
    this.defineLED2(TraktorZ2.outputReport81, "pfl", 0x23, 0x24);
    engine.makeConnection(this.activeChannel, "pfl", TraktorZ2.pflOutputHandler);
    engine.trigger(this.activeChannel, "pfl");


    this.defineLED2(TraktorZ2.outputReport81, "!traktorbuttonled", 0x25, 0x26);
    engine.makeConnection(this.activeChannel, "passthrough", TraktorZ2.vinylcontrolOutputHandler);

    engine.makeConnection(this.activeChannel, "key", TraktorZ2.updateDisplayOutputHandler);
    engine.trigger(this.activeChannel, "key");
    engine.makeConnection(this.activeChannel, "beatloop_size", TraktorZ2.updateDisplayOutputHandler);
    engine.makeConnection(this.activeChannel, "beatjump_size", TraktorZ2.updateDisplayOutputHandler);
    engine.makeConnection(this.activeChannel, "loop_enabled", TraktorZ2.updateDisplayOutputHandler);
    engine.trigger(this.activeChannel, "beatloop_size");

    // Define LEDs of the RGB LED pads
    const ledPadOffsets = {
        "Hotcue1": 0,
        "Hotcue2": 3,
        "Hotcue3": 6,
        "Hotcue4": 9
    };
    for (const hotcue in ledPadOffsets) {
        this.defineLED2(TraktorZ2.outputReport80, hotcue + "Red", 0x1D + ledPadOffsets[hotcue], 0x29 + ledPadOffsets[hotcue]);
        this.defineLED2(TraktorZ2.outputReport80, hotcue + "Green", 0x1E + ledPadOffsets[hotcue], 0x2A + ledPadOffsets[hotcue]);
        this.defineLED2(TraktorZ2.outputReport80, hotcue + "Blue", 0x1F + ledPadOffsets[hotcue], 0x2B + ledPadOffsets[hotcue]);
    }

};


TraktorZ2.Deck.prototype.registerOutputs4Decks = function() {
    HIDDebug("TraktorZ2: Deck.prototype.registerOutputs4Decks  this.activeChannel:" + this.activeChannel);

    for (let hotcue = 1; hotcue <= 8; hotcue++) {
        engine.makeConnection(this.activeChannel, "hotcue_" + hotcue + "_color", TraktorZ2.hotcueOutputHandler);
        engine.makeConnection(this.activeChannel, "hotcue_" + hotcue + "_enabled", TraktorZ2.hotcueOutputHandler);
    }

    // Deck Switch for Hotcue selection
    this.defineLED4(TraktorZ2.outputReport80, "!deck", 0x07, 0x08, 0x09, 0x0A);

    // ChA / ChB -> "Load/Duplicate" LED
    // ChC / ChD -> "Deck C" / "Deck" D LED
    this.defineLED4(TraktorZ2.outputReport80, "!beatIndicator", 0x11, 0x19, 0x0B, 0x0C);
    engine.makeUnbufferedConnection(this.activeChannel, "beat_active", TraktorZ2.beatOutputHandler);


    this.defineLED4(TraktorZ2.outputReport81, "!VuLabel", 0x01, 0x09, 0x11, 0x19);
    for (let i = 0; i < 6; i++) {
        this.defineLED4(TraktorZ2.outputReport81, "!VuMeter" + i, 0x02 + i, 0x0A + i, 0x12 + i, 0x1A + i);
    }
    this.defineLED4(TraktorZ2.outputReport81, "!PeakIndicator", 0x08, 0x10, 0x18, 0x20);

    // Ch3 / Ch4 VUMeter usage depends on context -> ChC / MasterL and ChD / MasterR
    engine.makeUnbufferedConnection(this.activeChannel, "VuMeter", TraktorZ2.displayVuValue);
    engine.makeConnection(this.activeChannel, "PeakIndicator", TraktorZ2.displayPeakIndicator);

};

TraktorZ2.registerOutputPackets = function() {
    HIDDebug("TraktorZ2: registerOutputPackets");

    this.outputReport80 = new HIDPacket("OutputReport80", 0x80);
    this.outputReport81 = new HIDPacket("OutputReport81", 0x81);

    // Register outputs, which only exist on the 2 main decks
    TraktorZ2.Decks.deck1.registerOutputs2Decks();
    TraktorZ2.Decks.deck2.registerOutputs2Decks();

    // Register outputs, which exist on all 4 decks
    for (const idx in TraktorZ2.Decks) {
        TraktorZ2.Decks[idx].registerOutputs4Decks();
    }

    TraktorZ2.outputReport80.addOutput("[Master]", "!vinylcontrolstatus", 0x02, "B", 0x7F);

    TraktorZ2.outputReport80.addOutput("[Channel1]", "quantize", 0x03, "B", 0x7F);
    engine.makeConnection("[Channel1]", "quantize", TraktorZ2.basicOutputHandler);
    engine.trigger("[Channel1]", "quantize");
    TraktorZ2.outputReport80.addOutput("[Channel2]", "quantize", 0x04, "B", 0x7F);
    engine.makeConnection("[Channel2]", "quantize", TraktorZ2.basicOutputHandler);
    engine.trigger("[Channel2]", "quantize");

    TraktorZ2.outputReport80.addOutput("[Master]", "skin_settings", 0x05, "B", 0x7F);
    //engine.makeConnection("[Master]", "skin_settings", TraktorZ2.basicOutputHandler);
    //engine.trigger("[Master]", "skin_settings");
    TraktorZ2.outputReport80.addOutput("[Master]", "shift", 0x06, "B", 0x7F);

    TraktorZ2.outputReport80.addOutput("[EffectRack1_EffectUnit1]", "!On", 0x0D, "B", 0x7F);
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect1]", "enabled", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect1]", "loaded", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect2]", "enabled", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect2]", "loaded", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect3]", "enabled", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit1_Effect3]", "loaded", TraktorZ2.fxOnLedHandler);
    engine.trigger("[EffectRack1_EffectUnit1_Effect1]", "enabled");

    TraktorZ2.outputReport80.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x0F, "B", 0x7F);
    engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", TraktorZ2.basicOutputHandler);
    engine.trigger("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable");
    TraktorZ2.outputReport80.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x10, "B", 0x7F);
    engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", TraktorZ2.basicOutputHandler);
    engine.trigger("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable");

    TraktorZ2.outputReport80.addOutput("[EffectRack1_EffectUnit2]", "!On", 0x15, "B", 0x7F);
    engine.makeConnection("[EffectRack1_EffectUnit2_Effect1]", "enabled", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2_Effect1]", "loaded", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2_Effect2]", "enabled", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2_Effect2]", "loaded", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2_Effect3]", "enabled", TraktorZ2.fxOnLedHandler);
    engine.makeConnection("[EffectRack1_EffectUnit2_Effect3]", "loaded", TraktorZ2.fxOnLedHandler);
    engine.trigger("[EffectRack1_EffectUnit2_Effect1]", "enabled");

    TraktorZ2.outputReport80.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x17, "B", 0x7F);
    engine.makeConnection("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", TraktorZ2.basicOutputHandler);
    engine.trigger("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable");
    TraktorZ2.outputReport80.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x18, "B", 0x7F);
    engine.makeConnection("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", TraktorZ2.basicOutputHandler);
    engine.trigger("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable");






    const ledChannelOffsets = {
        "[Channel1]": 0x35,
        "[Channel2]": 0x4A
    };
    const ledDigitOffsets = {
        "[Digit1]": 0x00,
        "[Digit2]": 0x07,
        "[Digit3]": 0x0E
    };

    for (const ch in ledChannelOffsets) {
        for (const digit in ledDigitOffsets) {
            TraktorZ2.outputReport80.addOutput(ch + digit, "segment_g", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x00, "B", 0x7F); // 7 bits control Deck 1 3rd 7 segment center horizontal bar brightness (orange)
            TraktorZ2.outputReport80.addOutput(ch + digit, "segment_c", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x01, "B", 0x7F); // 7 bits control Deck 1 3rd 7 segment lower right vertical bar brightness (orange)
            TraktorZ2.outputReport80.addOutput(ch + digit, "segment_b", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x02, "B", 0x7F); // 7 bits control Deck 1 3rd 7 segment upper right vertical bar brightness (orange)
            TraktorZ2.outputReport80.addOutput(ch + digit, "segment_a", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x03, "B", 0x7F); // 7 bits control Deck 1 3rd 7 segment upper horizontal bar brightness (orange)
            TraktorZ2.outputReport80.addOutput(ch + digit, "segment_f", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x04, "B", 0x7F); // 7 bits control Deck 1 3rd 7 segment upper left vertical bar brightness (orange)
            TraktorZ2.outputReport80.addOutput(ch + digit, "segment_e", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x05, "B", 0x7F); // 7 bits control Deck 1 3rd 7 segment lower left vertical bar brightness (orange)
            TraktorZ2.outputReport80.addOutput(ch + digit, "segment_d", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x06, "B", 0x7F); // 7 bits control Deck 1 3rd 7 segment lower horizontal bar brightness (orange)
        }
    }

    this.controller.registerOutputPacket(TraktorZ2.outputReport80);

    // Microphone button LED
    TraktorZ2.outputReport81.addOutput("[Master]", "!microphonebuttonled", 0x22, "B", 0x7F);

    TraktorZ2.outputReport81.addOutput("[Master]", "!usblight", 0x27, "B", 0x7F);

    // Ch3 / Ch4 VUMeter usage depends on context ->  ChC / MasterL and ChD / MasterR
    engine.makeUnbufferedConnection("[Master]", "VuMeterL", TraktorZ2.displayVuValue);
    engine.makeUnbufferedConnection("[Master]", "VuMeterR", TraktorZ2.displayVuValue);
    engine.makeConnection("[Master]", "PeakIndicatorL", TraktorZ2.displayPeakIndicator);
    engine.makeConnection("[Master]", "PeakIndicatorR", TraktorZ2.displayPeakIndicator);
    TraktorZ2.outputReport81.addOutput("[Master]", "!VuLabelMst", 0x21, "B", 0x7F);

    TraktorZ2.outputReport81.addOutput("[Master]", "!crossfaderReverse", 0x28, "B", 0x7F);

    this.controller.registerOutputPacket(TraktorZ2.outputReport81);

};

TraktorZ2.displayVuValue = function(value, group, key) {

    let ch;

    if ((group === "[Master]") && (key === "VuMeterL")) {
        // MasterL
        ch = "[Channel3]";
    } else if ((group === "[Master]") && (key === "VuMeterR")) {
        // MasterR
        ch = "[Channel4]";
    } else if ((group === "[Channel1]") && (key === "VuMeter") && (TraktorZ2.pregainCh3Timer === 0)) {
        // ChA
        ch = "[Channel1]";
    } else if ((group === "[Channel3]") && (key === "VuMeter") && (TraktorZ2.pregainCh3Timer !== 0)) {
        // ChC
        ch = "[Channel1]";
    } else if ((group === "[Channel2]") && (key === "VuMeter") && (TraktorZ2.pregainCh4Timer === 0)) {
        // ChB
        ch = "[Channel2]";
    } else if ((group === "[Channel4]") && (key === "VuMeter") && (TraktorZ2.pregainCh4Timer !== 0)) {
        // ChD
        ch = "[Channel2]";
    } else {
        return; // Hidden Channel of the pairs A/C or B/D
    }

    for (let i = 0; i < 6; i++) {
        let brightness = ((value * 6) - i) * kLedVuMeterBrightness;
        if (brightness < kLedOff) {
            brightness = kLedOff;
        }
        if (brightness > kLedVuMeterBrightness) {
            brightness = kLedVuMeterBrightness;
        }
        TraktorZ2.controller.setOutput(ch, "!VuMeter" + i, brightness, false);
    }
};

TraktorZ2.displayPeakIndicator = function(value, group, key) {

    let ch;

    if ((group === "[Master]") && (key === "PeakIndicatorL")) {
        // MasterL
        ch = "[Channel3]";
    } else if ((group === "[Master]") && (key === "PeakIndicatorR")) {
        // MasterR
        ch = "[Channel4]";
    } else if ((group === "[Channel1]") && (key === "PeakIndicator") && (TraktorZ2.pregainCh3Timer === 0)) {
        // ChA
        ch = "[Channel1]";
        TraktorZ2.controller.setOutput("[Channel1]", "!VuLabel", kLedVuMeterBrightness, false);
        TraktorZ2.controller.setOutput("[Channel3]", "!VuLabel", kLedOff, false);
    } else if ((group === "[Channel3]") && (key === "PeakIndicator") && (TraktorZ2.pregainCh3Timer !== 0)) {
        // ChC
        ch = "[Channel1]";
        TraktorZ2.controller.setOutput("[Channel1]", "!VuLabel", kLedOff, false);
        TraktorZ2.controller.setOutput("[Channel3]", "!VuLabel", kLedVuMeterBrightness, false);
    } else if ((group === "[Channel2]") && (key === "PeakIndicator") && (TraktorZ2.pregainCh4Timer === 0)) {
        // ChB
        ch = "[Channel2]";
        TraktorZ2.controller.setOutput("[Channel2]", "!VuLabel", kLedVuMeterBrightness, false);
        TraktorZ2.controller.setOutput("[Channel4]", "!VuLabel", kLedOff, false);
    } else if ((group === "[Channel4]") && (key === "PeakIndicator") && (TraktorZ2.pregainCh4Timer !== 0)) {
        // ChD
        ch = "[Channel2]";
        TraktorZ2.controller.setOutput("[Channel2]", "!VuLabel", kLedOff, false);
        TraktorZ2.controller.setOutput("[Channel4]", "!VuLabel", kLedVuMeterBrightness, false);
    } else {
        return; // Hidden Channel of the pairs A/C or B/D
    }

    if (value !== 0) {
        TraktorZ2.controller.setOutput(ch, "!PeakIndicator", kLedBright, false);
    } else {
        TraktorZ2.controller.setOutput(ch, "!PeakIndicator", kLedOff, false);
    }
};


TraktorZ2.displayLEDs = function() {

    TraktorZ2.controller.setOutput("[Master]", "skin_settings", kLedOff, true);
};

TraktorZ2.enableLEDsPerChannel = function() {
    HIDDebug("TraktorZ2: enableLEDsPerChannel");
    // Traktor Z2 can be switched per channel from internal mixing to external mixing
    // This is done by USB HID: Set Reports (Feature) 0xF1
    // 2x8Bit Logical 0...255
    // 0xF1 nn xx  -> Bit 0x01 of nn means  Software control for LEDs Ch1
    // 0xF1 nn xx  -> Bit 0x02 of nn means  Software control for LEDs Ch2
    // 0xF1 nn xx  -> Bit 0x04 of nn means  Software control for LEDs MasterCh
    // 0xF1 nn xx  -> Bit 0x08 of nn means  Software control for LED  Mic/Aux
    // 0xF1 nn xx  -> Bit 0x10 of nn means  Must be set to see any LED output??? or only 7 Segment displays for both channels???
    // 0xF1 nn xx  -> Bit 0x20 of nn means  Software control for LED USB-Symbol
    // 0xF1 nn xx  -> Bit 0x40 of nn means  ???
    // 0xF1 nn xx  -> Bit 0x80 of nn means  Software control for LEDs in Browse section

    // 0xF1 nn xx  -> Bit 0x40 of xx means  Mic/Aux (internal) mixing
    // 0xF1 nn xx  -> Any other bit of xx set means  Booth depends on Master level, otherwise both regulators are independent from each other
    const data = new Uint8Array(controller.getFeatureReport(0xF1));
    data[0] = 0xBF;
    controller.sendFeatureReport(data.buffer, 0xF1);

    TraktorZ2.dataF1 = controller.getFeatureReport(0xF1);/*
    TraktorZ2.dataF1[0] = 0x9F;
    TraktorZ2.dataF1 = [0xFF, 0x40];
    controller.sendFeatureReport(TraktorZ2.dataF1, 0xF1);*/

    HIDDebug(controller.getFeatureReport(0xF1));  // 2x8Bit Logical 0...255
};

TraktorZ2.init = function(_id) {
    this.Decks = {
        "deck1": new TraktorZ2.Deck(1, "deck1"),
        "deck2": new TraktorZ2.Deck(2, "deck2"),
        "deck3": new TraktorZ2.Deck(3, "deck3"),
        "deck4": new TraktorZ2.Deck(4, "deck4")
    };


    HIDDebug(new Uint8Array(controller.getFeatureReport(0xF1)));  // 2x8Bit Logical 0...255
    // Enabling "Route mic/aux input through Traktor" in Traktor Pro sends raw data "F1 93 00".
    // Disabling "Route mic/aux input through Traktor" in Traktor Pro sends raw data "F1 93 40".

    HIDDebug(new Uint8Array(controller.getFeatureReport(0xF3)));  // 2x8Bit Logical 0...127
    // The 1st 7 bit word defines the brightness of the LEDs in Off-State when the Z2 is not controlled by Mixxx
    // The 2nd 7 bit word defines the brightness of the LEDs in ON-State when the Z2 is not controlled by Mixxx


    const featureRptF1 = new Uint8Array([0x20, 0x80]);
    controller.sendFeatureReport(featureRptF1.buffer, 0xF1);
    const featureRptF3 = new Uint8Array([0x55, 0x7F]);
    controller.sendFeatureReport(featureRptF3.buffer, 0xF3);
    //TraktorZ2.debugLights();

    TraktorZ2.registerOutputPackets();
    TraktorZ2.registerInputPackets();

    // Read and apply initial state for two HID InputReports:
    // 10 Byte InputReport with ReportID 0x01
    // 53 Byte InputReport with ReportID 0x02

    // Set each InputReport to the bitwise inverted state first,
    // and than apply the non-inverted initial state.
    // This is done, because the common-hid-packet-parser only triggers
    // the callback functions in case of a delta to the previous data.
    for (let inputReportIdx = 0x01; inputReportIdx <= 0x02; ++inputReportIdx) {
        const data = new Uint8Array(controller.getInputReport(inputReportIdx));
        const dataInverted = new Uint8Array(data);
        for (let byteIdx = 1; byteIdx < data.byteLength; ++byteIdx) {
            dataInverted[byteIdx] = ~data[byteIdx];
        }
        TraktorZ2.incomingData(dataInverted);
        TraktorZ2.incomingData(data);
    }


    const inputRpt01 = new Uint8Array(controller.getInputReport(0x01));
    HIDDebug("inputRpt01" + inputRpt01 + "   " + inputRpt01[9]);
    if ((inputRpt01[9] & 0x02) !== 0) {
        engine.setValue("[Channel1]", "pfl", 1);
        HIDDebug("11");
    } else {
        engine.setValue("[Channel1]", "pfl", 0);
        HIDDebug("10");
    }
    if ((inputRpt01[9] & 0x04) !== 0) {
        engine.setValue("[Channel2]", "pfl", 1);
        HIDDebug("21");
    } else {
        engine.setValue("[Channel2]", "pfl", 0);
        HIDDebug("20");
    }

    // Now read the status of both InputReports
    // -> The normal callback function incomingData is executed direct here
    TraktorZ2.incomingData(controller.getInputReport(0x01));
    TraktorZ2.incomingData(controller.getInputReport(0x02));

    TraktorZ2.enableSoftTakeover();

    TraktorZ2.controller.setOutput("[Master]", "!usblight", kLedDimmed, false);

    TraktorZ2.deckSwitchHandler["[Channel1]"] = 1;
    TraktorZ2.controller.setOutput("[Channel1]", "!deck", kLedBright, false);
    TraktorZ2.deckSwitchHandler["[Channel2]"] = 1;
    TraktorZ2.controller.setOutput("[Channel2]", "!deck", kLedBright, false);
    TraktorZ2.deckSwitchHandler["[Channel3]"] = 0;
    TraktorZ2.controller.setOutput("[Channel3]", "!deck", kLedOff, false);
    TraktorZ2.deckSwitchHandler["[Channel4]"] = 0;
    TraktorZ2.controller.setOutput("[Channel4]", "!deck", kLedOff, false);

    TraktorZ2.controller.setOutput("[Master]", "!VuLabelMst", kLedVuMeterBrightness, true);

    // Initialize VU-Labels A and B
    TraktorZ2.displayPeakIndicator(engine.getValue("[Channel1]", "PeakIndicator"), "[Channel1]", "PeakIndicator");
    TraktorZ2.displayPeakIndicator(engine.getValue("[Channel2]", "PeakIndicator"), "[Channel2]", "PeakIndicator");

    TraktorZ2.hotcueOutputHandler();

    // Set LED control to software control not before  initializing all LED values, to reduce visual glitches
    TraktorZ2.enableLEDsPerChannel();

    HIDDebug("TraktorZ2: Init done!");

    //engine.beginTimer(20, this.displayLEDs);

    engine.beginTimer(50, function() {
        TraktorZ2.controller.setOutput("[Master]", "!VuLabelMst", kLedVuMeterBrightness, true);
    });

    HIDDebug("TraktorZ2: Init done!");
};

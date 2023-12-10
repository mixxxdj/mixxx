///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* Traktor Kontrol Z2 HID controller script v1.00                                */
/* Last modification: January 2023                                                */
/* Author: Jörg Wartenberg (based on the Traktor S3 mapping by Owen Williams)    */
/*                                                                               */
///////////////////////////////////////////////////////////////////////////////////

"use strict";

// Each color has 7Bit brightnesses, so these values can be between 0 and 127.
const kLedOff = 0x00;
const kLedDimmed = 0x27;
const kLedVuMeterBrightness = 0x37;
const kLedBright = 0x7F;

/**
 * Decks are the loop controls and the 4 hotcue buttons on either side of the controller.
 * Each Deck can control 2 channels a + c and b + d, which can be mapped.
 */
class DeckClass {
    /**
     * Creates an instance of DeckClass.
     * @param {TraktorZ2Class} parent
     * @param {number} deckNumber
     * @param {string} group
     */
    constructor(parent, deckNumber, group) {
        /** @type {TraktorZ2Class} */
        this.parent = parent;
        this.deckNumber = deckNumber;
        this.group = group;
        this.activeChannel = "[Channel" + deckNumber + "]";

        // Timer states to distinguish short and long press
        this.syncPressedTimerId = 0;
        this.syncPressed = false;
        this.snapQuantizePressedTimerId = 0;
        this.snapQuantizePressed = false;

        /**
         * Knob encoder states (hold values between 0x0 and 0xF)
         * Rotate to the right is +1 and to the left is means -1
         */
        this.loopKnobEncoderState = undefined;
    }

    numberButtonHandler(field) {
        const sideChannel = ["[Channel1]", "[Channel2]"];
        const sideOffset = [0, 0];

        if (this.parent.deckSwitch["[Channel1]"] === 1) {
            sideChannel[1] = "[Channel1]";
            sideOffset[1] = 0;  // First 4 hotcues mapped to the pads
        } else if (this.parent.deckSwitch["[Channel1]"] === 2) {
            sideChannel[1] = "[Channel1]";
            sideOffset[1] = 4;  // Second 4 hotcues mapped to the pads
        } else if (this.parent.deckSwitch["[Channel1]"] === 3) {
            sideChannel[1] = "[Channel1]";
            sideOffset[1] = -1;  // IntroBegin/End and OutroBegin/End mapped to the pads
        } else if (this.parent.deckSwitch["[Channel3]"] === 1) {
            sideChannel[1] = "[Channel3]";
            sideOffset[1] = 0;  // First 4 hotcues mapped to the pads
        } else if (this.parent.deckSwitch["[Channel3]"] === 2) {
            sideChannel[1] = "[Channel3]";
            sideOffset[1] = 4;  // Second 4 hotcues mapped to the pads
        } else if (this.parent.deckSwitch["[Channel3]"] === 3) {
            sideChannel[1] = "[Channel3]";
            sideOffset[1] = -1;  // IntroBegin/End and OutroBegin/End mapped to the pads
        }
        if (this.parent.deckSwitch["[Channel2]"] === 1) {
            sideChannel[2] = "[Channel2]";
            sideOffset[2] = 0;  // First 4 hotcues mapped to the pads
        } else if (this.parent.deckSwitch["[Channel2]"] === 2) {
            sideChannel[2] = "[Channel2]";
            sideOffset[2] = 4;  // Second 4 hotcues mapped to the pads
        } else if (this.parent.deckSwitch["[Channel2]"] === 3) {
            sideChannel[2] = "[Channel2]";
            sideOffset[2] = -1;  // IntroBegin/End and OutroBegin/End mapped to the pads
        } else if (this.parent.deckSwitch["[Channel4]"] === 1) {
            sideChannel[2] = "[Channel4]";
            sideOffset[2] = 0;  // First 4 hotcues mapped to the pads
        } else if (this.parent.deckSwitch["[Channel4]"] === 2) {
            sideChannel[2] = "[Channel4]";
            sideOffset[2] = 4;  // Second 4 hotcues mapped to the pads
        } else if (this.parent.deckSwitch["[Channel4]"] === 3) {
            sideChannel[2] = "[Channel4]";
            sideOffset[2] = -1;  // IntroBegin/End an
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
        if (this.parent.shiftState & 0x01) {
            action = "_clear";
        } else {
            action = "_activate";
        }
        if (sideOffset[chIdx] === -1) {
            switch (padNumber) {
            case 1 :
                engine.setValue(
                    sideChannel[chIdx], "intro_start" + action, field.value);
                break;
            case 2 :
                engine.setValue(
                    sideChannel[chIdx], "intro_end" + action, field.value);
                break;
            case 3 :
                engine.setValue(
                    sideChannel[chIdx], "outro_start" + action, field.value);
                break;
            case 4 :
                engine.setValue(
                    sideChannel[chIdx], "outro_end" + action, field.value);
                break;
            }
        } else {
            console.log("setting " + "hotcue_" + padNumber + action + " " + field.value);
            engine.setValue(
                sideChannel[chIdx], "hotcue_" + (sideOffset[chIdx] + padNumber) + action, field.value);
        }
    }

    fluxHandler(field) {
        if (field.value === 0) {
            return;
        }
        script.toggleControl(this.activeChannel, "slip_enabled");
    }

    vinylcontrolHandler(field) {
        console.log(
            "TraktorZ2: vinylcontrolHandler " +
            "this.activeChannel:" + this.activeChannel + " field.value:" + field.value);
        if (field.value === 0 || (engine.getValue(this.activeChannel, "passthrough") === 1)) {
            return;
        }

        if ((this.parent.shiftState & 0x01) === 0x01) {
            // Shift button hold down -> Toggle between Internal Playback mode / Vinyl Control
            script.toggleControl(this.activeChannel, "vinylcontrol_enabled");
        } else {
            if (engine.getValue(this.activeChannel, "vinylcontrol_enabled") === 0) {
                // Internal Playback mode -> Vinyl Control Off -> Orange
                if ((this.parent.shiftState & 0x02) !== 0x02) {
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
    }

    syncHandler(field) {
        console.log(
            "TraktorZ2: syncHandler " +
            "this.activeChannel:" + this.activeChannel + " field:" + field +
            "key:" + engine.getValue(this.activeChannel, "key"));

        // Shift not hold down
        if (this.parent.shiftState === 0) {
            if (field.value === 1) {
                engine.setValue(this.activeChannel, "sync_enabled", 1);
                // Start timer to measure how long button is pressed
                this.syncPressedTimerId = engine.beginTimer(300, () => {
                    // Reset sync button timer state if active
                    if (this.syncPressedTimerId !== 0) {
                        this.syncPressedTimerId = 0;
                    }
                }, true);
            } else {
                if (this.syncPressedTimerId !== 0) {
                    // Timer still running -> stop it and unlight LED
                    engine.stopTimer(this.syncPressedTimerId);
                    engine.setValue(this.activeChannel, "sync_enabled", 0);
                }
            }
            return;
        }

        // Shift pressed and hold
        if (((this.parent.shiftState & 0x01) === 0x01) && (field.value === 1)) {
            script.toggleControl(this.activeChannel, "keylock");
            return;
        }

        // Shift locked
        // Depending on long or short press, sync beat or go to key sync mode
        if (field.value === 1) {
            // Start timer to measure how long button is pressed
            this.syncPressedTimerId = engine.beginTimer(300, () => {
                this.syncPressed = true;
                // Change display values to key notation
                this.parent.displayLoopCount("[Channel1]", false);
                this.parent.displayLoopCount("[Channel2]", true);

                // Reset sync button timer state if active
                if (this.syncPressedTimerId !== 0) {
                    this.syncPressedTimerId = 0;
                }
            }, true);
        } else {
            this.syncPressed = false;
            // Change display values to loop/beatjump
            this.parent.displayLoopCount("[Channel1]", false);
            this.parent.displayLoopCount("[Channel2]", true);

            if (this.syncPressedTimerId !== 0) {
                // Timer still running -> stop it and unlight LED
                engine.stopTimer(this.syncPressedTimerId);
                script.triggerControl(this.activeChannel, "sync_key");
            }
        }
    }

    loadTrackHandler(field) {
        // If shift mode is locked or active pressed
        if (this.parent.shiftState) {
            if (this.activeChannel === "[Channel1]") {
                engine.setValue("[Channel1]", "CloneFromDeck", 2);
            } else if (this.activeChannel === "[Channel2]") {
                engine.setValue("[Channel2]", "CloneFromDeck", 1);
            }
        } else {
            engine.setValue(this.activeChannel, "LoadSelectedTrack", field.value);
        }
    }

    /**
     * defineButton2 allows us to configure input buttons for the two main decks of the 2+2 deck
     * mixer layout, depending on which is appropriate.  This avoids extra logic in the function
     * where we define all the magic numbers.
     * @param hidReport
     * @param name
     * @param deck1Offset
     * @param deck1Bitmask
     * @param deck2Offset
     * @param deck2Bitmask
     * @param fn
     */
    defineButton2(hidReport, name, deck1Offset, deck1Bitmask, deck2Offset, deck2Bitmask, fn) {
        switch (this.deckNumber) {
        case 1:
            this.parent.registerInputButton(
                hidReport, this.group, name, deck1Offset, deck1Bitmask, fn.bind(this));
            break;
        case 2:
            this.parent.registerInputButton(
                hidReport, this.group, name, deck2Offset, deck2Bitmask, fn.bind(this));
            break;
        }
    }

    /**
     * defineLED2 allows us to configure output LEDs for the two main decks of the 2+2 deck mixer
     * layout, depending on which is appropriate.  This avoids extra logic in the function where we
     * define all the magic numbers.
     * @param hidReport
     * @param name
     * @param deck1Offset
     * @param deck2Offset
     */
    defineLED2(hidReport, name, deck1Offset, deck2Offset) {
        // All LEDs of the Traktor Z2 have 7Bit outputs
        console.log(
            "TraktorZ2: defineLED2 " +
            "hidReport:" + hidReport + " this.deckNumber" + this.deckNumber + " name:" + name);

        switch (this.deckNumber) {
        case 1:
            hidReport.addOutput("[Channel1]", name, deck1Offset, "B", 0x7F);
            break;
        case 2:
            hidReport.addOutput("[Channel2]", name, deck2Offset, "B", 0x7F);
            break;
        }
    }

    /**
     * defineLED2 allows us to configure output LEDs for all 4 decks of the 2+2 deck mixer layout,
     * depending on which is appropriate.  This avoids extra logic in the function where we define
     * all the magic numbers.
     * @param hidReport
     * @param name
     * @param deck1Offset
     * @param deck2Offset
     * @param deck3Offset
     * @param deck4Offset
     */
    defineLED4(hidReport, name, deck1Offset, deck2Offset, deck3Offset, deck4Offset) {
        // All LEDs of the Traktor Z2 have 7Bit outputs
        console.log(
            "TraktorZ2: defineLED4 " +
            "hidReport:" + hidReport + " this.deckNumber" + this.deckNumber + " name:" + name);

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
    }

    selectLoopHandler(field) {
        console.log(
            "TraktorZ2: selectLoopHandler " + this.activeChannel + " field.value:" + field.value);

        let delta = 0;
        if ((field.value + 15) % 16 === this.loopKnobEncoderState) {
            delta = +1;
        } else if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
            delta = -1;
        }
        this.loopKnobEncoderState = field.value;

        if (((this.parent.Decks.deck1.syncPressed === true) &&
             (this.activeChannel !== "[Channel1]")) ||
            ((this.parent.Decks.deck2.syncPressed === true) &&
             (this.activeChannel !== "[Channel2]"))) {
            // Display shows key not loop or beatjump -> Ignore input
            return;
        }

        if (this.syncPressed) {
            // Sync hold down -> Adjust key
            if (delta === -1) {
                script.triggerControl(this.activeChannel, "pitch_down");
            } else if (delta === +1) {
                script.triggerControl(this.activeChannel, "pitch_up");
            }
        } else if (this.parent.shiftState === 0x00) {
            // Shift mode not set, and shift button not pressed -> Adjust loop size
            const beatloopSize = engine.getValue(this.activeChannel, "beatloop_size");
            if (delta === -1) {
                engine.setValue(this.activeChannel, "beatloop_size", beatloopSize / 2);
            } else if (delta === +1) {
                engine.setValue(this.activeChannel, "beatloop_size", beatloopSize * 2);
            }
        } else if (this.parent.shiftState === 0x01) {
            // Shift mode not set, but shift button is pressed ->  Move loop
            if (delta === -1) {
                engine.setValue(
                    this.activeChannel, "loop_move",
                    engine.getValue(this.activeChannel, "beatloop_size") * -1);
            } else if (delta === +1) {
                engine.setValue(
                    this.activeChannel, "loop_move",
                    engine.getValue(this.activeChannel, "beatloop_size"));
            }
        } else if (this.parent.shiftState === 0x02) {
            // Shift mode is set, but shift button not pressed ->  Adjust beatjump size
            const beatjumpSize = engine.getValue(this.activeChannel, "beatjump_size");
            if (delta === -1) {
                engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize / 2);
            } else if (delta === +1) {
                engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize * 2);
            }
        } else if (this.parent.shiftState === 0x03) {
            // Shift mode is set, and shift button is pressed ->  Move beatjump
            if (delta === -1) {
                engine.setValue(
                    this.activeChannel, "beatjump",
                    engine.getValue(this.activeChannel, "beatjump_size") * -1);
            } else if (delta === +1) {
                engine.setValue(
                    this.activeChannel, "beatjump",
                    engine.getValue(this.activeChannel, "beatjump_size"));
            }
        }
    }

    activateLoopHandler(field) {
        console.log("TraktorZ2: activateLoopHandler");
        if (field.value === 1) {
            const isLoopActive = engine.getValue(this.activeChannel, "loop_enabled");

            if (this.syncPressed) {
                // Sync hold down -> Sync key
                script.triggerControl(this.activeChannel, "reset_key");
            } else if (this.parent.shiftState) {
                engine.setValue(this.activeChannel, "reloop_toggle", field.value);
            } else {
                if (isLoopActive) {
                    engine.setValue(this.activeChannel, "reloop_toggle", field.value);
                } else {
                    engine.setValue(this.activeChannel, "beatloop_activate", field.value);
                }
            }
        }
    }

    snapQuantizeButtonHandler(field) {
        console.log("TraktorZ2: snapQuantizeButtonHandler");

        // Depending on long or short press, sync beat or go to key sync mode
        if (field.value === 1) {  // Start timer to measure how long button is pressed
            this.snapQuantizePressedTimerId = engine.beginTimer(300, () => {
                this.snapQuantizePressed = true;

                // Reset sync button timer state if active
                if (this.snapQuantizePressedTimerId !== 0) {
                    this.snapQuantizePressedTimerId = 0;
                }
            }, true);
        } else {
            this.snapQuantizePressed = false;
            // Change display values to loop/beatjump

            if (this.snapQuantizePressedTimerId !== 0) {
                // Timer still running -> stop it
                engine.stopTimer(this.snapQuantizePressedTimerId);

                if (this.parent.shiftState !== 0) {
                    // Adjust Beatgrid to current trackposition
                    script.triggerControl(this.activeChannel, "beats_translate_curpos");
                } else {
                    script.toggleControl(this.activeChannel, "quantize");
                }
            }
        }
    }

    pflButtonHandler(field) {
        console.log("TraktorZ2: pflButtonHandler");
        if (field.value === 0) {
            return;  // Button released
        }

        let group;
        if (this.parent.shiftState !== 0) {
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
    }

    traktorButtonHandler(field) {
        console.log(
            "TraktorZ2: traktorButtonHandler " +
            "this.activeChannel: " + this.activeChannel + " field.value: " + field.value);
        if (field.value === 1) {
            // Taktor button pressed
            if (this.parent.shiftState & 0x01) {
                script.toggleControl(this.activeChannel, "passthrough");
                this.parent.traktorButtonOutputHandler(this.activeChannel);
            }
        }
    }

    traktorButtonStatusHandler(field) {
        console.log(
            "TraktorZ2: traktorButtonStatusHandler " +
            "this.activeChannel: " + this.activeChannel + " field.value: " + field.value);
        this.parent.traktorButtonStatus[this.activeChannel] = field.value;
        this.parent.traktorButtonOutputHandler(this.activeChannel);
    }

    registerInputs2Decks() {
        console.log("TraktorZ2: registerInputs2Decks");

        this.defineButton2(
            this.parent.inputReport01, "!pad_1", 0x06, 0x04, 0x07, 0x08,
            this.numberButtonHandler.bind(this));
        this.defineButton2(
            this.parent.inputReport01, "!pad_2", 0x06, 0x08, 0x07, 0x10,
            this.numberButtonHandler.bind(this));
        this.defineButton2(
            this.parent.inputReport01, "!pad_3", 0x06, 0x10, 0x07, 0x20,
            this.numberButtonHandler.bind(this));
        this.defineButton2(
            this.parent.inputReport01, "!pad_4", 0x06, 0x20, 0x07, 0x40,
            this.numberButtonHandler.bind(this));

        // Traktor buttons
        this.defineButton2(
            this.parent.inputReport01, "!traktorbutton", 0x03, 0x01, 0x03, 0x02,
            this.traktorButtonHandler.bind(this));
        this.defineButton2(
            this.parent.inputReport01, "!stateOfTraktorbutton", 0x09, 0x08, 0x09, 0x10,
            this.traktorButtonStatusHandler.bind(this));

        // Quantize buttons
        this.defineButton2(
            this.parent.inputReport01, "quantize", 0x03, 0x04, 0x03, 0x10,
            this.snapQuantizeButtonHandler.bind(this));

        // PFL Headphone CUE buttons
        this.defineButton2(
            this.parent.inputReport01, "pfl", 0x04, 0x04, 0x04, 0x08,
            this.pflButtonHandler.bind(this));

        // Vinyl control mode (REL / INTL)
        this.defineButton2(
            this.parent.inputReport01, "vinylcontrol_mode", 0x04, 0x10, 0x04, 0x20,
            this.vinylcontrolHandler.bind(this));
        this.defineButton2(
            this.parent.inputReport01, "!sync", 0x04, 0x40, 0x04, 0x80,
            this.syncHandler.bind(this));

        // Load/Duplicate buttons
        this.defineButton2(
            this.parent.inputReport01, "!LoadSelectedTrack", 0x04, 0x01, 0x04, 0x02,
            this.loadTrackHandler.bind(this));

        // Loop control
        this.defineButton2(
            this.parent.inputReport01, "!SelectLoop", 0x01, 0xF0, 0x02, 0x0F,
            this.selectLoopHandler.bind(this));
        this.defineButton2(
            this.parent.inputReport01, "!ActivateLoop", 0x05, 0x40, 0x08, 0x20,
            this.activateLoopHandler.bind(this));

        // Flux / Tap
        this.defineButton2(
            this.parent.inputReport01, "!slip_enabled", 0x06, 0x40, 0x07, 0x80,
            this.fluxHandler.bind(this));
    }

    registerOutputs2Decks() {
        console.log("TraktorZ2: registerOutputs2Decks");

        this.defineLED2(this.parent.outputReport80, "slip_enabled", 0x0E, 0x16);
        engine.makeConnection(
            this.activeChannel, "slip_enabled", this.parent.basicOutputHandler.bind(this.parent));
        engine.trigger(this.activeChannel, "slip_enabled");

        this.defineLED2(this.parent.outputReport80, "!vinylcontrol_orange", 0x12, 0x1A);
        this.defineLED2(this.parent.outputReport80, "!vinylcontrol_green", 0x13, 0x1B);
        engine.makeConnection(
            this.activeChannel, "vinylcontrol_status",
            this.parent.vinylcontrolStatusOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "vinylcontrol_mode",
            this.parent.vinylcontrolOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "cue_indicator",
            this.parent.vinylcontrolOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "play_indicator",
            this.parent.vinylcontrolOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "vinylcontrol_enabled",
            this.parent.vinylcontrolOutputHandler.bind(this.parent));
        engine.trigger(this.activeChannel, "vinylcontrol_status");
        engine.trigger(this.activeChannel, "vinylcontrol_mode");
        engine.trigger(this.activeChannel, "cue_indicator");
        engine.trigger(this.activeChannel, "play_indicator");
        engine.trigger(this.activeChannel, "vinylcontrol_enabled");

        this.defineLED2(this.parent.outputReport80, "sync_mode", 0x14, 0x1C);
        engine.makeConnection(
            this.activeChannel, "sync_mode",
            this.parent.basicOutputHandler.bind(this.parent));
        engine.trigger(this.activeChannel, "sync_mode");

        // Headphone button LEDs
        this.defineLED2(this.parent.outputReport81, "pfl", 0x23, 0x24);
        engine.makeConnection(
            this.activeChannel, "pfl",
            this.parent.pflOutputHandler.bind(this.parent));
        engine.trigger(this.activeChannel, "pfl");


        this.defineLED2(this.parent.outputReport81, "!traktorbuttonled", 0x25, 0x26);
        engine.makeConnection(
            this.activeChannel, "passthrough",
            this.parent.vinylcontrolOutputHandler.bind(this.parent));

        engine.makeConnection(
            this.activeChannel, "key",
            this.parent.updateDisplayOutputHandler.bind(this.parent));
        engine.trigger(this.activeChannel, "key");
        engine.makeConnection(
            this.activeChannel, "beatloop_size",
            this.parent.updateDisplayOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "beatjump_size",
            this.parent.updateDisplayOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "loop_enabled",
            this.parent.updateDisplayOutputHandler.bind(this.parent));
        engine.trigger(this.activeChannel, "beatloop_size");

        // Define LEDs of the RGB LED pads
        const ledPadOffsets = {
            "Hotcue1": 0,
            "Hotcue2": 3,
            "Hotcue3": 6,
            "Hotcue4": 9
        };
        for (const hotcue in ledPadOffsets) {
            this.defineLED2(
                this.parent.outputReport80, hotcue + "Red", 0x1D + ledPadOffsets[hotcue],
                0x29 + ledPadOffsets[hotcue]);
            this.defineLED2(
                this.parent.outputReport80, hotcue + "Green", 0x1E + ledPadOffsets[hotcue],
                0x2A + ledPadOffsets[hotcue]);
            this.defineLED2(
                this.parent.outputReport80, hotcue + "Blue", 0x1F + ledPadOffsets[hotcue],
                0x2B + ledPadOffsets[hotcue]);
        }
    }

    registerOutputs4Decks() {
        console.log("TraktorZ2: registerOutputs4Decks  this.activeChannel:" + this.activeChannel);

        engine.makeConnection(
            this.activeChannel, "intro_start_enabled",
            this.parent.hotcueOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "intro_end_enabled",
            this.parent.hotcueOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "outro_start_enabled",
            this.parent.hotcueOutputHandler.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "outro_end_enabled",
            this.parent.hotcueOutputHandler.bind(this.parent));

        for (let hotcue = 1; hotcue <= 8; hotcue++) {
            engine.makeConnection(
                this.activeChannel, "hotcue_" + hotcue + "_color",
                this.parent.hotcueOutputHandler.bind(this.parent));
            engine.makeConnection(
                this.activeChannel, "hotcue_" + hotcue + "_status",
                this.parent.hotcueOutputHandler.bind(this.parent));
        }

        // Deck Switch for Hotcue selection
        this.defineLED4(this.parent.outputReport80, "!deck", 0x07, 0x08, 0x09, 0x0A);

        // ChA / ChB -> "Load/Duplicate" LED
        // ChC / ChD -> "Deck C" / "Deck" D LED
        this.defineLED4(this.parent.outputReport80, "!beatIndicator", 0x11, 0x19, 0x0B, 0x0C);
        engine.makeUnbufferedConnection(
            this.activeChannel, "beat_active",
            this.parent.beatOutputHandler.bind(this.parent));


        this.defineLED4(this.parent.outputReport81, "!vu_label", 0x01, 0x09, 0x11, 0x19);
        for (let i = 0; i < 6; i++) {
            this.defineLED4(
                this.parent.outputReport81, "!vu_meter" + i, 0x02 + i, 0x0A + i, 0x12 + i, 0x1A + i);
        }
        this.defineLED4(this.parent.outputReport81, "!peak_indicator", 0x08, 0x10, 0x18, 0x20);

        // Ch3 / Ch4 VuMeter usage depends on context -> ChC / MasterL and ChD / MasterR
        engine.makeUnbufferedConnection(
            this.activeChannel, "vu_meter",
            this.parent.displayVuValue.bind(this.parent));
        engine.makeConnection(
            this.activeChannel, "peak_indicator",
            this.parent.displayPeakIndicator.bind(this.parent));
    }
}

class TraktorZ2Class {
    constructor() {
        this.controller = new HIDController();

        this.shiftPressedTimer = undefined;
        this.shiftPressed = false;

        /**
         * - 0x00: shift mode off / and not active pressed
         *  - 0x01: shift mode off / but active pressed
         *  - 0x02: shift mode on  / and not active pressed
         *  - 0x03: shift mode on  / and active pressed
         */
        this.shiftState = 0x00;

        this.microphoneButtonStatus = undefined;
        this.traktorButtonStatus = [];

        this.dataF1 = new ArrayBuffer(2);

        /**
         * Knob encoder states (hold values between 0x0 and 0xF)
         * Rotate to the right is +1 and to the left is means -1
         */
        this.browseKnobEncoderState = undefined;

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

        this.inputReport01 = new HIDPacket("InputReport01", 0x01, this.messageCallback.bind(this));
        this.inputReport02 = new HIDPacket("InputReport02", 0x02, this.messageCallback.bind(this));

        this.outputReport80 = new HIDPacket("OutputReport80", 0x80);
        this.outputReport81 = new HIDPacket("OutputReport81", 0x81);

        this.deckSwitch = {
            "[Channel1]": 1,
            "[Channel2]": 2,
            "[Channel3]": 3,
            "[Channel4]": 4
        };

        this.Decks = {
            "deck1": new DeckClass(this, 1, "deck1"),
            "deck2": new DeckClass(this, 2, "deck2"),
            "deck3": new DeckClass(this, 3, "deck3"),
            "deck4": new DeckClass(this, 4, "deck4")
        };
    }

    init(_id) {
        console.log(new Uint8Array(controller.getFeatureReport(0xF1)));  // 2x8Bit Logical 0...255
        // HotPluging with only NIHardwareService.exe running raw data "F1 90 40".
        // Enabling "Route mic/aux input through Traktor" in Traktor Pro sends raw data "F1 93 00".
        // Disabling "Route mic/aux input through Traktor" in Traktor Pro sends raw data "F1 93 40".

        console.log(new Uint8Array(controller.getFeatureReport(0xF3)));  // 2x8Bit Logical 0...127
        // The 1st 7 bit word defines the brightness of the LEDs in Off-State when the Z2 is not
        // controlled by Mixxx The 2nd 7 bit word defines the brightness of the LEDs in ON-State
        // when the Z2 is not controlled by Mixxx

        const featureRptF1 = new Uint8Array([0x20, 0x80]);
        controller.sendFeatureReport(0xF1, featureRptF1.buffer);
        //const featureRptF3 = new Uint8Array([0x7F, 0x55]);
        //controller.sendFeatureReport(0xF3, featureRptF3.buffer);
        // this.debugLights();

        this.registerOutputPackets();
        this.registerInputPackets();

        // Read and apply initial state for two HID InputReports:
        // 10 Byte InputReport with ReportID 0x01
        // 53 Byte InputReport with ReportID 0x02

        // Set each InputReport to the bitwise inverted state first,
        // and than apply the non-inverted initial state.
        // This is done, because the common-hid-packet-parser only triggers
        // the callback functions in case of a delta to the previous data.
        for (let inputReportIdx = 0x01; inputReportIdx <= 0x02; ++inputReportIdx) {
            const reportData = new Uint8Array(controller.getInputReport(inputReportIdx));

            this.incomingData([inputReportIdx, ...reportData.map(x => ~x)]);
            this.incomingData([inputReportIdx, ...reportData]);
        }

        const inputRpt01 = new Uint8Array(controller.getInputReport(0x01));
        console.log("inputRpt01" + inputRpt01 + "   " + inputRpt01[8]);
        if ((inputRpt01[8] & 0x02) !== 0) {
            engine.setValue("[Channel1]", "pfl", 1);
            console.log("PFL 11");
        } else {
            engine.setValue("[Channel1]", "pfl", 0);
            console.log("PFL 10");
        }
        if ((inputRpt01[8] & 0x04) !== 0) {
            engine.setValue("[Channel2]", "pfl", 1);
            console.log("PFL 21");
        } else {
            engine.setValue("[Channel2]", "pfl", 0);
            console.log("PFL 20");
        }

        this.enableSoftTakeover();

        this.controller.setOutput("[Main]", "!usblight", kLedDimmed, false);

        this.deckSwitch["[Channel1]"] = 1;
        this.controller.setOutput("[Channel1]", "!deck", kLedBright, false);
        this.deckSwitch["[Channel2]"] = 1;
        this.controller.setOutput("[Channel2]", "!deck", kLedBright, false);
        this.deckSwitch["[Channel3]"] = 0;
        this.controller.setOutput("[Channel3]", "!deck", kLedOff, false);
        this.deckSwitch["[Channel4]"] = 0;
        this.controller.setOutput("[Channel4]", "!deck", kLedOff, false);

        this.controller.setOutput("[Main]", "!vu_labelMst", kLedVuMeterBrightness, true);

        this.controller.setOutput("[Master]", "skin_settings", kLedOff, true);

        // Initialize VU-Labels A and B
        this.displayPeakIndicator(
            engine.getValue("[Channel1]", "peak_indicator"), "[Channel1]", "peak_indicator");
        this.displayPeakIndicator(
            engine.getValue("[Channel2]", "peak_indicator"), "[Channel2]", "peak_indicator");

        this.hotcueOutputHandler();

        // Set LED control to software control not before  initializing all LED values, to reduce
        // visual glitches
        this.enableLEDsPerChannel();

        console.log("TraktorZ2: Init done!");

        engine.beginTimer(50, () => {
            this.controller.setOutput("[Main]", "!vu_labelMst", kLedVuMeterBrightness, true);
        });

        console.log("TraktorZ2: Init done!");
    }

    fxOnClickHandler(field) {
        console.log("TraktorZ2: fxOnClickHandler");
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
    }

    fxOnLedHandler() {
        console.log("TraktorZ2: fxOnLedHandler");
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
                this.controller.setOutput(
                    "[EffectRack1_EffectUnit" + macroFxUnitIdx + "]", "!On", kLedOff, macroFxUnitIdx === 2);
            } else if (numOfLoadedandEnabledEffects > 0 && numOfLoadedButDisabledEffects > 0) {
                this.controller.setOutput(
                    "[EffectRack1_EffectUnit" + macroFxUnitIdx + "]", "!On", kLedDimmed, macroFxUnitIdx === 2);
            } else {
                this.controller.setOutput(
                    "[EffectRack1_EffectUnit" + macroFxUnitIdx + "]", "!On", kLedBright, macroFxUnitIdx === 2);
            }
        }
    }

    deckSwitchHandler(field) {
        console.log("TraktorZ2: deckSwitchHandler: " + field.group + " " + field.value);
        if (field.value === 1) {
            if (field.group === "[Channel1]") {
                this.controller.setOutput("[Channel3]", "!deck", kLedOff, true);
                this.deckSwitch["[Channel3]"] = 0;
            } else if (field.group === "[Channel2]") {
                this.controller.setOutput("[Channel4]", "!deck", kLedOff, true);
                this.deckSwitch["[Channel4]"] = 0;
            } else if (field.group === "[Channel3]") {
                this.controller.setOutput("[Channel1]", "!deck", kLedOff, true);
                this.deckSwitch["[Channel1]"] = 0;
            } else if (field.group === "[Channel4]") {
                this.controller.setOutput("[Channel2]", "!deck", kLedOff, true);
                this.deckSwitch["[Channel2]"] = 0;
            }

            if (this.shiftPressed === false) {
                if (this.deckSwitch[field.group] !== 1) {
                    // Show HotCues 1...4
                    this.deckSwitch[field.group] = 1;
                    this.controller.setOutput(field.group, "!deck", kLedBright, true);

                } else if (engine.getValue("[Skin]", "show_8_hotcues")) {
                    // Show HotCues 5...8
                    this.deckSwitch[field.group] = 2;
                    this.controller.setOutput(field.group, "!deck", kLedDimmed, true);
                }
            } else {
                // Show IntroBegin/End and OutroBegin/End
                this.deckSwitch[field.group] = 3;
                this.controller.setOutput(field.group, "!deck", kLedBright, true);
            }
            this.hotcueOutputHandler();  // Set new hotcue button colors
        }
    }

    vinylcontrolOutputHandler(value, group, key) {
        console.log(
            "TraktorZ2: vinylcontrolOutputHandler " +
            "group:" + group + " key:" + key);

        // Sets TaktorButton state, depending on Passthrough state
        this.traktorButtonOutputHandler(group);

        if (engine.getValue(group, "passthrough") === 1) {
            // REL /INTL button has no function in Passthrough mode -> LED Off
            this.controller.setOutput(group, "!vinylcontrol_green", kLedOff, false);
            this.controller.setOutput(group, "!vinylcontrol_orange", kLedOff, true);
            return;
        }

        if (engine.getValue(group, "vinylcontrol_enabled") === 0) {
            // Internal Playback mode -> Vinyl Control Off -> Orange
            this.controller.setOutput(group, "!vinylcontrol_green", kLedOff);
            if ((this.shiftState & 0x02) !== 0x02) {
                // Shift mode isn't locked -> Show PLAY indicator
                if (engine.getValue(group, "play_indicator") === 0) {
                    // Dim only to signal visualize Internal Playback mode by Orange color
                    this.controller.setOutput(group, "!vinylcontrol_orange", kLedDimmed, true);
                } else {
                    this.controller.setOutput(group, "!vinylcontrol_orange", kLedBright, true);
                }
            } else {
                // Shift mode is locked -> Show CUE indicator
                if (engine.getValue(group, "cue_indicator") === 0) {
                    // Dim only to signal visualize Internal Playback mode by Orange color
                    this.controller.setOutput(group, "!vinylcontrol_orange", kLedDimmed, true);
                } else {
                    this.controller.setOutput(group, "!vinylcontrol_orange", kLedBright, true);
                }
            }
        } else {
            if (engine.getValue(group, "vinylcontrol_mode") === 0) {
                // Absolute Mode (track position equals needle position and speed)
                this.controller.setOutput(group, "!vinylcontrol_green", kLedBright, false);
                this.controller.setOutput(group, "!vinylcontrol_orange", kLedOff, true);
            } else if (engine.getValue(group, "vinylcontrol_mode") === 1) {
                // Relative Mode (track tempo equals needle speed regardless of needle position)
                this.controller.setOutput(group, "!vinylcontrol_green", kLedDimmed, false);
                this.controller.setOutput(group, "!vinylcontrol_orange", kLedOff, true);
            } else if (engine.getValue(group, "vinylcontrol_mode") === 2) {
                // Constant Mode (track tempo equals last known-steady tempo regardless of needle
                // input Both LEDs on -> Values result in a dirty yellow
                this.controller.setOutput(group, "!vinylcontrol_green", 0x37, false);
                this.controller.setOutput(group, "!vinylcontrol_orange", 0x57, true);
            }
        }
    }

    vinylcontrolStatusOutputHandler(vfalue, group, key) {
        console.log(
            "TraktorZ2: vinylcontrolOutputHandler " +
            "group:" + group + " key:" + key);
        // Z2 has only one vinylcontrol status LED for both channels -> merge information of both
        if ((engine.getValue("[Channel1]", "vinylcontrol_status") === 3) ||
            (engine.getValue("[Channel2]", "vinylcontrol_status") === 3) ||
            (engine.getValue("[Channel1]", "vinylcontrol_status") === 2) ||
            (engine.getValue("[Channel2]", "vinylcontrol_status") === 2)) {
            this.controller.setOutput("[Main]", "!vinylcontrolstatus", kLedBright, true);
        } else if (
            (engine.getValue("[Channel1]", "vinylcontrol_status") === 1) ||
            (engine.getValue("[Channel2]", "vinylcontrol_status") === 1)) {
            this.controller.setOutput("[Main]", "!vinylcontrolstatus", kLedDimmed, true);
        } else {
            this.controller.setOutput("[Main]", "!vinylcontrolstatus", kLedOff, true);
        }
    }

    selectTrackHandler(field) {
        console.log("TraktorZ2: selectTrackHandler");
        let delta = 0;
        if ((field.value + 15) % 16 === this.browseKnobEncoderState) {
            delta = +1;
        } else if ((field.value + 1) % 16 === this.browseKnobEncoderState) {
            delta = -1;
        }
        this.browseKnobEncoderState = field.value;

        if (this.Decks.deck1.snapQuantizePressed !== this.Decks.deck2.snapQuantizePressed) {
            let ch;
            // Snap / Quantize button is hold for one channel
            if (this.Decks.deck1.snapQuantizePressed) {
                ch = "[Channel1]";
            } else {
                ch = "[Channel2]";
            }

            if (this.shiftState === 0x02) {
                // If shift mode is locked scale beatgrid
                if (delta === -1) {
                    script.triggerControl(ch, "beats_adjust_faster");
                } else if (delta === +1) {
                    script.triggerControl(ch, "beats_adjust_slower");
                }
            } else {
                // Shift is not locked zoom waveform
                if (delta === -1) {
                    script.triggerControl(ch, "waveform_zoom_up");
                } else if (delta === +1) {
                    script.triggerControl(ch, "waveform_zoom_down");
                }
            }
            return;
        }

        // If shift mode is locked
        if (this.shiftState === 0x02) {
            engine.setValue("[Library]", "MoveHorizontal", delta);
        } else {
            engine.setValue("[Library]", "MoveVertical", delta);
        }
    }

    LibraryFocusHandler(field) {
        console.log("TraktorZ2: LibraryFocusHandler");
        if (field.value) {
            // If shift mode is locked
            if (this.shiftState === 0x02) {
                engine.setValue("[Library]", "sort_column_toggle", 0);
            } else {
                engine.setValue("[Library]", "MoveFocusForward", 1);
            }
        }
    }

    crossfaderReverseHandler(field) {
        console.log("TraktorZ2: LibraryFocusHandler");
        const busSelector = {
            Left: 0,
            Center: 1,
            Right: 2
        };

        if (field.value) {
            // XF REVERSE (Hamster mode)
            this.controller.setOutput("[Main]", "!crossfaderReverse", kLedBright, true);
            if (engine.getValue("[Channel1]", "orientation") !== busSelector.Center) {
                engine.setValue("[Channel1]", "orientation", busSelector.Right);
            }
            if (engine.getValue("[Channel3]", "orientation") !== busSelector.Center) {
                engine.setValue("[Channel3]", "orientation", busSelector.Right);
            }
            if (engine.getValue("[Channel2]", "orientation") !== busSelector.Center) {
                engine.setValue("[Channel2]", "orientation", busSelector.Left);
            }
            if (engine.getValue("[Channel4]", "orientation") !== busSelector.Center) {
                engine.setValue("[Channel4]", "orientation", busSelector.Left);
            }

        } else {
            // Normal mode
            this.controller.setOutput("[Main]", "!crossfaderReverse", kLedOff, true);
            if (engine.getValue("[Channel1]", "orientation") !== busSelector.Center) {
                engine.setValue("[Channel1]", "orientation", busSelector.Left);
            }
            if (engine.getValue("[Channel3]", "orientation") !== busSelector.Center) {
                engine.setValue("[Channel3]", "orientation", busSelector.Left);
            }
            if (engine.getValue("[Channel2]", "orientation") !== busSelector.Center) {
                engine.setValue("[Channel2]", "orientation", busSelector.Right);
            }
            if (engine.getValue("[Channel4]", "orientation") !== busSelector.Center) {
                engine.setValue("[Channel4]", "orientation", busSelector.Right);
            }
        }
    }

    buttonHandler(field) {
        console.log("TraktorZ2: buttonHandler");
        if (field.value === 0) {
            return;  // Button released
        }
        script.toggleControl(field.group, field.name);
    }

    pflOutputHandler(value, group, key) {
        // TODO: Implement Channel A/C B/D switch here

        let ledValue = value;
        if (value === 0 || value === false) {
            // Off value
            ledValue = kLedOff;
        } else if (value === 1 || value === true) {
            // On value
            ledValue = kLedBright;
        }

        this.controller.setOutput(group, key, ledValue);
    }

    microphoneButtonHandler(field) {
        console.log(
            "TraktorZ2: microphoneButtonHandler " +
            "field.value: " + field.value);
        if (field.value === 1) {
            // Microphone button pressed
            if (this.shiftState & 0x01) {
                if (this.dataF1[1] & 0x40) {
                    this.dataF1[1] &= ~0x40;
                } else {
                    this.dataF1[1] |= 0x40;
                }
                console.log(this.dataF1[1]);
                controller.sendFeatureReport(0xF1, this.dataF1);
            }
            this.microphoneButtonOutputHandler();
        }
    }

    microphoneButtonStatusHandler(field) {
        console.log(
            "TraktorZ2: microphoneButtonStatusHandler " +
            "field.value: " + field.value);
        this.microphoneButtonStatus = field.value;
        this.microphoneButtonOutputHandler();
    }

    microphoneButtonOutputHandler() {
        console.log("TraktorZ2: microphoneButtonOutputHandler");
        if (this.dataF1[1] & 0x40) {
            // Mic/Aux in internal mixing mode
            console.log("TraktorZ2: microphoneButtonStatusHandler: internal");
            if (this.microphoneButtonStatus === 0) {
                // Controller internal state OFF -> Switch LED to represent this state
                this.controller.setOutput("[Main]", "!microphonebuttonled", kLedOff, true);
            } else {
                // Controller internal state ON -> Switch LED to represent this state
                this.controller.setOutput("[Main]", "!microphonebuttonled", kLedDimmed, true);
            }
        } else {
            console.log("TraktorZ2: microphoneButtonStatusHandler: external");
            if (this.microphoneButtonStatus === 0) {
                // Controller internal state OFF -> Switch LED to represent this state
                this.controller.setOutput("[Main]", "!microphonebuttonled", kLedOff,
                    true);
            } else {
                // Controller internal state ON -> Switch LED to represent this state
                this.controller.setOutput("[Main]", "!microphonebuttonled", kLedBright, true);
            }
        }
    }

    traktorButtonOutputHandler(group) {
        console.log("TraktorZ2: traktorButtonOutputHandler");

        if (this.traktorButtonStatus[group] === 1) {
            if (engine.getValue(group, "passthrough") === 1) {
                // Controller internal state ON -> Switch LED to represent this state
                this.controller.setOutput(group, "!traktorbuttonled", kLedDimmed, true);
            } else {
                // Controller internal state OFF -> Switch LED to represent this state
                this.controller.setOutput(group, "!traktorbuttonled", kLedBright, true);
            }
        } else {
            // Channel in standalone mixing mode. No external control possible until traktor button
            // is pressed

            // Controller internal state OFF -> Switch LED to represent this state
            this.controller.setOutput(group, "!traktorbuttonled", kLedOff, true);
        }
    }

    registerInputPackets() {
        console.log("TraktorZ2: registerInputPackets");

        // Register inputs, which only exist on the 2 main decks
        this.Decks.deck1.registerInputs2Decks();
        this.Decks.deck2.registerInputs2Decks();

        this.registerInputButton(
            this.inputReport01, "[Channel1]", "switchDeck", 0x06, 0x02,
            this.deckSwitchHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[Channel2]", "switchDeck", 0x07, 0x02,
            this.deckSwitchHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[Channel3]", "switchDeck", 0x06, 0x01,
            this.deckSwitchHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[Channel4]", "switchDeck", 0x07, 0x04,
            this.deckSwitchHandler.bind(this));

        this.registerInputButton(
            this.inputReport01, "[Master]", "maximize_library", 0x03, 0x08,
            this.buttonHandler.bind(this));

        this.registerInputButton(
            this.inputReport01, "[Main]", "!microphoneButton", 0x05, 0x01,
            this.microphoneButtonHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[Main]", "!stateOfMicrophoneButton", 0x09, 0x01,
            this.microphoneButtonStatusHandler.bind(this));

        this.registerInputButton(
            this.inputReport01, "[Master]", "shift", 0x07, 0x01,
            this.shiftHandler.bind(this));

        this.registerInputButton(
            this.inputReport01, "[Master]", "!SelectTrack", 0x01, 0x0F,
            this.selectTrackHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[Master]", "!LibraryFocus", 0x03, 0x80,
            this.LibraryFocusHandler.bind(this));


        this.registerInputButton(
            this.inputReport01, "[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x05, 0x04,
            this.buttonHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x05, 0x08,
            this.buttonHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x08, 0x02,
            this.buttonHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x08, 0x04,
            this.buttonHandler.bind(this));

        this.registerInputButton(
            this.inputReport01, "[EffectRack1_EffectUnit1]", "!enabled", 0x05, 0x02,
            this.fxOnClickHandler.bind(this));
        this.registerInputButton(
            this.inputReport01, "[EffectRack1_EffectUnit2]", "!enabled", 0x08, 0x01,
            this.fxOnClickHandler.bind(this));

        this.registerInputButton(
            this.inputReport01, "[Main]", "!crossfaderReverse", 0x08, 0x80,
            this.crossfaderReverseHandler.bind(this));


        this.controller.registerInputPacket(this.inputReport01);


        this.registerInputScaler(
            this.inputReport02, "[EffectRack1_EffectUnit1]", "mix", 0x0D, 0xFFFF,
            this.parameterHandler.bind(this));  // MACRO FX1 D/W
        this.registerInputScaler(
            this.inputReport02, "[EffectRack1_EffectUnit1]", "super1", 0x0F, 0xFFFF,
            this.parameterHandler.bind(this));  // MACRO FX1 FX
        this.registerInputScaler(
            this.inputReport02, "[EffectRack1_EffectUnit2]", "mix", 0x1B, 0xFFFF,
            this.parameterHandler.bind(this));  // MACRO FX2 D/W
        this.registerInputScaler(
            this.inputReport02, "[EffectRack1_EffectUnit2]", "super1", 0x1D, 0xFFFF,
            this.parameterHandler.bind(this));  // MACRO FX2 FX

        this.registerInputScaler(
            this.inputReport02, "[Channel1]", "volume", 0x2D, 0xFFFF,
            this.faderHandler.bind(this));  // Fader Deck A
        this.registerInputScaler(
            this.inputReport02, "[Channel2]", "volume", 0x2F, 0xFFFF,
            this.faderHandler.bind(this));  // Fader Deck B

        // Mic/Aux Tone knob, where no 1:1 mapping is available
        // this.registerInputScaler(
        //     this.inputReport02, "[Master]", "duckStrengh", 0x03, 0xFFFF,
        //     this.parameterHandler.bind(this));
        this.registerInputScaler(
            this.inputReport02, "[Microphone]", "pregain", 0x01, 0xFFFF,
            this.parameterHandler.bind(this));

        this.registerInputScaler(
            this.inputReport02, "[Channel1]", "pregain", 0x11, 0xFFFF,
            this.pregainHandler.bind(this));  // Rotary knob Deck A
        this.registerInputScaler(
            this.inputReport02, "[Channel2]", "pregain", 0x1F, 0xFFFF,
            this.pregainHandler.bind(this));  // Rotary knob Deck B
        this.registerInputScaler(
            this.inputReport02, "[Channel3]", "pregain", 0x29, 0xFFFF,
            this.pregainHandler.bind(this));  // Rotary knob Deck C
        this.registerInputScaler(
            this.inputReport02, "[Channel4]", "pregain", 0x2B, 0xFFFF,
            this.pregainHandler.bind(this));  // Rotary knob Deck D

        this.registerInputScaler(
            this.inputReport02, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x13, 0xFFFF,
            this.eqKnobHandler.bind(this));  // High
        this.registerInputScaler(
            this.inputReport02, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x15, 0xFFFF,
            this.eqKnobHandler.bind(this));  // Mid
        this.registerInputScaler(
            this.inputReport02, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x17, 0xFFFF,
            this.eqKnobHandler.bind(this));  // Low

        this.registerInputScaler(
            this.inputReport02, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x21, 0xFFFF,
            this.eqKnobHandler.bind(this));  // High
        this.registerInputScaler(
            this.inputReport02, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x23, 0xFFFF,
            this.eqKnobHandler.bind(this));  // Mid
        this.registerInputScaler(
            this.inputReport02, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x25, 0xFFFF,
            this.eqKnobHandler.bind(this));  // Low

        this.registerInputScaler(
            this.inputReport02, "[QuickEffectRack1_[Channel1]]", "super1", 0x19, 0xFFFF,
            this.eqKnobHandler.bind(this));
        this.registerInputScaler(
            this.inputReport02, "[QuickEffectRack1_[Channel2]]", "super1", 0x27, 0xFFFF,
            this.eqKnobHandler.bind(this));

        this.registerInputScaler(
            this.inputReport02, "[Master]", "crossfader", 0x31, 0xFFFF,
            this.faderHandler.bind(this));

        // Master Gain is a function of the hardware mixer -> Don't map this knob
        // this.registerInputScaler(this.inputReport02, "[Master]", "gain", 0x09, 0xFFFF,
        // this.parameterHandler.bind(this));

        this.registerInputScaler(
            this.inputReport02, "[Master]", "headMix", 0x07, 0xFFFF,
            this.parameterHandler.bind(this));

        // Headphone Gain is a function of the hardware mixer -> Don't map this knob
        // this.registerInputScaler(this.inputReport02, "[Master]", "headGain", 0x05, 0xFFFF,
        // this.parameterHandler.bind(this));

        this.controller.registerInputPacket(this.inputReport02);
    }

    enableSoftTakeover() {
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

        // engine.softTakeover("[Master]", "crossfader", true); // softTakeover might be a
        // performance issue
        engine.softTakeover("[Master]", "gain", true);
        engine.softTakeover("[Master]", "headMix", true);
        engine.softTakeover("[Master]", "headGain", true);
    }


    registerInputScaler(message, group, name, offset, bitmask, callback) {
        console.log("TraktorZ2: registerInputScaler");
        message.addControl(group, name, offset, "H", bitmask);
        message.setCallback(group, name, callback);
    }

    registerInputButton(message, group, name, offset, bitmask, callback) {
        console.log("TraktorZ2: registerInputButton");
        message.addControl(group, name, offset, "B", bitmask);
        message.setCallback(group, name, callback);
    }

    shiftHandler(field) {
        console.log("TraktorZ2: shiftHandler");

        // This function sets this.shiftState as follows:
        // 0x00: shift mode off / and not active pressed
        // 0x01: shift mode off / but active pressed
        // 0x02: shift mode on  / and not active pressed
        // 0x03: shift mode on  / and active pressed

        if (this.shiftPressed === false && field.value === 1) {
            this.shiftPressed = true;
            this.shiftState |= 0x01;
            this.controller.setOutput("[Master]", "shift", kLedBright, true);

            this.shiftPressedTimer = engine.beginTimer(200, () => {
                // Reset sync button timer state if active
                if (this.shiftPressedTimer !== 0) {
                    this.shiftPressedTimer = 0;
                }
                // Change display values to beatloopsize
                this.displayLoopCount("[Channel1]", false);
                this.displayLoopCount("[Channel2]", true);
                console.log("TraktorZ2: shift unlocked");
            }, true);

            console.log("TraktorZ2: shift pressed");
        } else if (this.shiftPressed === true && field.value === 0) {
            this.shiftPressed = false;

            console.log("TraktorZ2: shift button released" + this.shiftState);
            if (this.shiftPressedTimer !== 0) {
                if (this.shiftState & 0x02) {
                    // Timer still running -> stop it and set LED depending on previous lock state
                    this.shiftState = 0x00;
                    this.controller.setOutput("[Master]", "shift", kLedOff, false);
                    this.vinylcontrolOutputHandler(0, "[Channel1]", "Shift");
                    this.vinylcontrolOutputHandler(0, "[Channel2]", "Shift");
                } else {
                    this.shiftState = 0x02;
                    this.controller.setOutput("[Master]", "shift", kLedDimmed, true);
                    this.vinylcontrolOutputHandler(1, "[Channel1]", "Shift");
                    this.vinylcontrolOutputHandler(1, "[Channel2]", "Shift");
                }
                engine.stopTimer(this.shiftPressedTimer);
                // Change display values beatjumpsize / beatloopsize
                this.displayLoopCount("[Channel1]", false);
                this.displayLoopCount("[Channel2]", true);
                console.log("TraktorZ2: static shift state changed to: " + this.shiftState);
            } else {
                if (this.shiftState & 0x02) {
                    this.shiftState = 0x02;
                    this.controller.setOutput("[Master]", "shift", kLedDimmed, true);
                } else {
                    this.shiftState = 0x00;
                    this.controller.setOutput("[Master]", "shift", kLedOff, true);
                }
                console.log("TraktorZ2: back to static shift state: " + this.shiftState);
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
                if (this.eqValueStorage[eqGroups[idx] + eqParameters[idx] + "changed"] === true) {
                    this.eqExecute(
                        eqGroups[idx], eqParameters[idx],
                        this.eqValueStorage[eqGroups[idx] + eqParameters[idx] + "value"]);
                }
            }
        }
    }

    parameterHandler(field) {
        console.log("TraktorZ2: parameterHandler");
        engine.setParameter(field.group, field.name, field.value / 4095);
    }

    eqKnobHandler(field) {
        console.log("TraktorZ2: eqKnobHandler");

        if (this.shiftPressed === false) {
            this.eqExecute(field.group, field.name, field.value);
        } else {
            // Store value until Shift button will be released
            this.eqValueStorage[field.group + field.name + "changed"] = true;
            this.eqValueStorage[field.group + field.name + "value"] = field.value;
        }
    }

    eqExecute(group, name, value) {
        console.log("TraktorZ2: eqExecute");
        if ((group === "[EqualizerRack1_[Channel1]_Effect1]") ||
            (group === "[QuickEffectRack1_[Channel1]]")) {
            if (this.pregainCh3Timer !== 0) {
                engine.stopTimer(this.pregainCh3Timer);
                this.pregainCh3Timer = 0;
                this.displayVuValue(
                    engine.getValue("[Channel1]", "vu_meter"), "[Channel1]", "vu_meter");
                this.displayPeakIndicator(
                    engine.getValue("[Channel1]", "peak_indicator"), "[Channel1]", "peak_indicator");
            }
        } else if ((group === "[EqualizerRack1_[Channel2]_Effect1]") ||
            (group === "[QuickEffectRack1_[Channel2]]")) {
            if (this.pregainCh4Timer !== 0) {
                engine.stopTimer(this.pregainCh4Timer);
                this.pregainCh4Timer = 0;
                this.displayVuValue(
                    engine.getValue("[Channel2]", "vu_meter"), "[Channel2]", "vu_meter");
                this.displayPeakIndicator(
                    engine.getValue("[Channel2]", "peak_indicator"), "[Channel2]", "peak_indicator");
            }
        }
        engine.setParameter(group, name, value / 4095);
        this.eqValueStorage[group + name + "changed"] = false;
    }

    /// The Traktor Z2 has a dedicated pre-gain knob for all 4 channels,
    /// but only 2 multiplexed channel VU-Meters.
    /// An additional LED indicates if the left VU-Meter shows the signal from with channel A or C.
    /// The same applies for the right VU-Meter an the letters B or D
    pregainHandler(field) {
        console.log("TraktorZ2: pregainHandler");
        engine.setParameter(field.group, field.name, field.value / 4095);
        if ((field.group === "[Channel1]") && (this.pregainCh3Timer !== 0)) {
            engine.stopTimer(this.pregainCh3Timer);
            this.pregainCh3Timer = 0;
            this.displayVuValue(engine.getValue("[Channel1]", "vu_meter"), "[Channel1]", "vu_meter");
            this.displayPeakIndicator(
                engine.getValue("[Channel1]", "peak_indicator"), "[Channel1]", "peak_indicator");
        }
        if (field.group === "[Channel3]") {
            if (this.pregainCh3Timer !== 0) {
                engine.stopTimer(this.pregainCh3Timer);
            }
            this.displayVuValue(engine.getValue("[Channel3]", "vu_meter"), "[Channel3]", "vu_meter");
            this.displayPeakIndicator(
                engine.getValue("[Channel3]", "peak_indicator"), "[Channel3]", "peak_indicator");
            this.pregainCh3Timer = engine.beginTimer(2500, () => {
                this.pregainCh3Timer = 0;
                this.displayVuValue(
                    engine.getValue("[Channel1]", "vu_meter"), "[Channel1]", "vu_meter");
                this.displayPeakIndicator(
                    engine.getValue("[Channel1]", "peak_indicator"), "[Channel1]", "peak_indicator");
            }, true);
        }
        if ((field.group === "[Channel2]") && (this.pregainCh4Timer !== 0)) {
            engine.stopTimer(this.pregainCh4Timer);
            this.pregainCh4Timer = 0;
            this.displayVuValue(engine.getValue("[Channel2]", "vu_meter"), "[Channel2]", "vu_meter");
            this.displayPeakIndicator(
                engine.getValue("[Channel2]", "peak_indicator"), "[Channel2]", "peak_indicator");
        }
        if (field.group === "[Channel4]") {
            if (this.pregainCh4Timer !== 0) {
                engine.stopTimer(this.pregainCh4Timer);
            }
            this.displayVuValue(engine.getValue("[Channel4]", "vu_meter"), "[Channel4]", "vu_meter");
            this.displayPeakIndicator(
                engine.getValue("[Channel4]", "peak_indicator"), "[Channel4]", "peak_indicator");
            this.pregainCh4Timer = engine.beginTimer(2500, () => {
                this.pregainCh4Timer = 0;
                this.displayVuValue(
                    engine.getValue("[Channel2]", "vu_meter"), "[Channel2]", "vu_meter");
                this.displayPeakIndicator(
                    engine.getValue("[Channel2]", "peak_indicator"), "[Channel2]", "peak_indicator");
            }, true);
        }
    }

    faderHandler(field) {
        // To ensure that the faders always shut completely,
        // all values below 100 are rated as zero,
        // and all values from 3995 to 4095 are rated as one.
        // Todo: NI provides a tool, to calibrate the Z2 faders.
        //       If the interaction between this calibration tool
        //       and the values in Mixxx is understood,
        //       this implementation might be improved
        engine.setParameter(
            field.group, field.name, script.absoluteLin(field.value, 0, 1, 100, 3995));
    }

    messageCallback(_packet, data) {
        HIDController.fastForIn(data, (fieldName) => {
            this.controller.processButton(data[fieldName]);
        }
        );
    }

    incomingData(data, length) {
        // console.log("TraktorZ2: incomingData data:" + data + "   length:" + length);
        this.controller.parsePacket(data, length);
    }

    /**
     * Call this if you want to just send raw packets to the controller (good for figuring out what
     *  bytes do what).
     */
    debugLights() {
        console.log("TraktorZ2: debugLights");
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
            0x00, 0x10, 0x00,  // 0x1D HotCue 1 Deck 1 RGB
            0x00, 0x1F, 0x00,  // 0x20 HotCue 2 Deck 1 RGB
            0x00, 0x20, 0x00,  // 0x23 HotCue 3 Deck 1 RGB
            0x00, 0x2F, 0x00,  // 0x26 HotCue 4 Deck 1 RGB
            0x00, 0x00, 0x00,  // 0x29 HotCue 1 Deck 2 RGB
            0x00, 0x00, 0x00,  // 0x2C HotCue 2 Deck 2 RGB
            0x00, 0x00, 0x00,  // 0x2F HotCue 3 Deck 2 RGB
            0x00, 0x00, 0x00,  // 0x32 HotCue 4 Deck 2 RGB

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
    }

    /**
     basicOutputHandler drives lights that only have one color.
     * @param value
     * @param group
     * @param key
     */
    basicOutputHandler(value, group, key) {
        console.log(
            "TraktorZ2: basicOutputHandler " +
            "group:" + group + " key:" + key + " value:" + value);
        if (value === 0 || value === false) {
            // Off value
            this.controller.setOutput(group, key, kLedOff, true);
        } else {
            // On value
            this.controller.setOutput(group, key, kLedBright, true);
        }
    }

    hotcueOutputHandler() {
        const sideChannel = ["[Channel1]", "[Channel2]"];
        const sideOffset = [0, 0];
        if (this.deckSwitch["[Channel1]"] === 1) {
            sideChannel[1] = "[Channel1]";
            sideOffset[1] = 0;  // First 4 hotcues mapped to the pads
        } else if (this.deckSwitch["[Channel1]"] === 2) {
            sideChannel[1] = "[Channel1]";
            sideOffset[1] = 4;  // Second 4 hotcues mapped to the pads
        } else if (this.deckSwitch["[Channel1]"] === 3) {
            sideChannel[1] = "[Channel1]";
            sideOffset[1] = -1;  // IntroBegin/End and OutroBegin/End mapped to the pads
        } else if (this.deckSwitch["[Channel3]"] === 1) {
            sideChannel[1] = "[Channel3]";
            sideOffset[1] = 0;  // First 4 hotcues mapped to the pads
        } else if (this.deckSwitch["[Channel3]"] === 2) {
            sideChannel[1] = "[Channel3]";
            sideOffset[1] = 4;  // Second 4 hotcues mapped to the pads
        } else if (this.deckSwitch["[Channel3]"] === 3) {
            sideChannel[1] = "[Channel3]";
            sideOffset[1] = -1;  // IntroBegin/End and OutroBegin/End mapped to the pads
        }
        if (this.deckSwitch["[Channel2]"] === 1) {
            sideChannel[2] = "[Channel2]";
            sideOffset[2] = 0;  // First 4 hotcues mapped to the pads
        } else if (this.deckSwitch["[Channel2]"] === 2) {
            sideChannel[2] = "[Channel2]";
            sideOffset[2] = 4;  // Second 4 hotcues mapped to the pads
        } else if (this.deckSwitch["[Channel2]"] === 3) {
            sideChannel[2] = "[Channel2]";
            sideOffset[2] = -1;  // IntroBegin/End and OutroBegin/End mapped to the pads
        } else if (this.deckSwitch["[Channel4]"] === 1) {
            sideChannel[2] = "[Channel4]";
            sideOffset[2] = 0;  // First 4 hotcues mapped to the pads
        } else if (this.deckSwitch["[Channel4]"] === 2) {
            sideChannel[2] = "[Channel4]";
            sideOffset[2] = 4;  // Second 4 hotcues mapped to the pads
        } else if (this.deckSwitch["[Channel4]"] === 3) {
            sideChannel[2] = "[Channel4]";
            sideOffset[2] = -1;  // IntroBegin/End and OutroBegin/End mapped to the pads
        }

        for (let chidx = 1; chidx <= 2; chidx++) {
            const ch = "[Channel" + chidx + "]";
            for (let i = 1; i <= 4; i++) {
                let colorCode = engine.getValue(
                    sideChannel[chidx], "hotcue_" + (sideOffset[chidx] + i) + "_color");
                if (engine.getValue(
                    sideChannel[chidx], "hotcue_" + (sideOffset[chidx] + i) + "_status") === 0) {
                    colorCode = 0;
                }
                if (sideOffset[chidx] === -1) {
                    // Show IntroBegin/End and OutroBegin/End
                    colorCode = 0x0000FF; // Blue
                }
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
                // or unset Intro/Outro marker is assigned
                if ((sideOffset[chidx] === -1 &&
                    ((i === 1 && engine.getValue(sideChannel[chidx], "intro_start_enabled") === 0) ||
                    (i === 2 && engine.getValue(sideChannel[chidx], "intro_end_enabled") === 0) ||
                    (i === 3 && engine.getValue(sideChannel[chidx], "outro_start_enabled") === 0) ||
                    (i === 4 && engine.getValue(sideChannel[chidx], "outro_end_enabled") === 0)
                    )) || engine.getValue(sideChannel[chidx], "hotcue_" + (sideOffset[chidx] + i) + "_type") === 4) {
                    red = Math.ceil(red * 0.3);
                    green = Math.ceil(green * 0.3);
                    blue = Math.ceil(blue * 0.25);  // Blue LED is dominant -> dim it slightly
                }
                // console.log("Channel: " + ch + " Hotcue: " + i + " Colorcode: " + colorCode +
                // " Red: " + red + " Green: " + green + " Blue: " + blue);
                this.controller.setOutput(ch, "Hotcue" + i + "Red", red, false);
                this.controller.setOutput(ch, "Hotcue" + i + "Green", green, false);
                this.controller.setOutput(ch, "Hotcue" + i + "Blue", blue, true);
            }
        }
    }

    beatOutputHandler(value, group) {
        if (engine.getValue(group, "loop_enabled") && engine.getValue(group, "play")) {
            const playposition =
                engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
            if ((playposition >= engine.getValue(group, "loop_start_position")) &&
                (playposition <= engine.getValue(group, "loop_end_position"))) {
                if (engine.getValue(group, "beatloop_size") >= 0.125) {
                    if (value !== 0) {
                        this.displayBrightness[group] = kLedBright;
                    } else {
                        this.displayBrightness[group] = kLedDimmed;
                    }
                } else if (engine.getValue(group, "beatloop_size") >= 0.0625) {
                    if (this.displayBrightness[group] === 0x57) {
                        return;
                    } else {
                        this.displayBrightness[group] = 0x57;
                    }
                } else {
                    if (this.displayBrightness[group] === 0x40) {
                        return;
                    } else {
                        this.displayBrightness[group] = 0x40;
                    }
                }
                this.displayBeatLeds(group);
                return;
            }
        }

        if (value !== 0) {
            this.displayBrightness[group] = kLedBright;
        } else {
            this.displayBrightness[group] = kLedDimmed;
        }
        this.displayBeatLeds(group);
    }

    updateDisplayOutputHandler(value, group) {
        console.log("updateDisplayOutputHandler");
        this.displayLoopCount(group, true);
    }

    displayBeatLeds(group) {
        if ((group === "[Channel1]") || (group === "[Channel2]")) {
            this.displayLoopCount(group, false);
        }
        const sendNow = true;
        if (engine.getValue(group, "track_loaded") === 0) {
            this.controller.setOutput(group, "!beatIndicator", kLedOff, sendNow);
        } else {
            this.controller.setOutput(
                group, "!beatIndicator", this.displayBrightness[group], sendNow);
        }
    }

    /**
     * @param group may be either[Channel1] or [Channel2]
     * @param {boolean} sendMessage if true, send HID package immediateley
     */
    displayLoopCount(group, sendMessage) {
        let numberToDisplay;

        let displayBrightness;

        if (engine.getValue(group, "track_loaded") === 0) {
            displayBrightness = kLedOff;
        } else if (engine.getValue(group, "loop_enabled") && !(this.shiftState & 0x02)) {
            const playposition =
                engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
            if ((playposition >= engine.getValue(group, "loop_start_position")) &&
                (playposition <= engine.getValue(group, "loop_end_position"))) {
                displayBrightness = this.displayBrightness[group];
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

        if (this.Decks.deck1.syncPressed || this.Decks.deck2.syncPressed) {
            const key = engine.getValue(group, "key");
            console.log("TraktorZ2: ################ Key:" + key);

            let majorMinor;

            if (key === 1) {
                numberToDisplay = 8;  // 1d
                majorMinor = "b";
            } else if (key === 2) {
                numberToDisplay = 3;  // 8d
                majorMinor = "b";
            } else if (key === 3) {
                numberToDisplay = 10;  // 3d
                majorMinor = "b";
            } else if (key === 4) {
                numberToDisplay = 5;  // 10d
                majorMinor = "b";
            } else if (key === 5) {
                numberToDisplay = 12;  // 5d
                majorMinor = "b";
            } else if (key === 6) {
                numberToDisplay = 7;  // 12d
                majorMinor = "b";
            } else if (key === 7) {
                numberToDisplay = 2;  // 7d
                majorMinor = "b";
            } else if (key === 8) {
                numberToDisplay = 9;  // 2d
                majorMinor = "b";
            } else if (key === 9) {
                numberToDisplay = 4;  // 9d
                majorMinor = "b";
            } else if (key === 10) {
                numberToDisplay = 11;  // 4d
                majorMinor = "b";
            } else if (key === 11) {
                numberToDisplay = 6;  // 11d
                majorMinor = "b";
            } else if (key === 12) {
                numberToDisplay = 1;  // 6d
                majorMinor = "b";
            } else if (key === 13) {
                numberToDisplay = 5;  // 10m
                majorMinor = "A";
            } else if (key === 14) {
                numberToDisplay = 12;  // 5m
                majorMinor = "A";
            } else if (key === 15) {
                numberToDisplay = 7;  // 12m
                majorMinor = "A";
            } else if (key === 16) {
                numberToDisplay = 2;  // 7m
                majorMinor = "A";
            } else if (key === 17) {
                numberToDisplay = 9;  // 2m
                majorMinor = "A";
            } else if (key === 18) {
                numberToDisplay = 4;  // 9m
                majorMinor = "A";
            } else if (key === 19) {
                numberToDisplay = 11;  // 4m
                majorMinor = "A";
            } else if (key === 20) {
                numberToDisplay = 6;  // 11m
                majorMinor = "A";
            } else if (key === 21) {
                numberToDisplay = 1;  // 6m
                majorMinor = "A";
            } else if (key === 22) {
                numberToDisplay = 8;  // 1m
                majorMinor = "A";
            } else if (key === 23) {
                numberToDisplay = 3;  // 8m
                majorMinor = "A";
            } else if (key === 24) {
                numberToDisplay = 10;  // 3m
                majorMinor = "A";
            } else {
                if (sendMessage === true) {
                    // Display empty string
                    this.displayLoopCountDigit(group + "[Digit1]", -2, displayBrightness);
                    this.displayLoopCountDigit(group + "[Digit2]", -2, displayBrightness);
                    this.displayLoopCountDigit(group + "[Digit3]", -2, displayBrightness);

                    this.controller.getOutputField(group + "[Digit1]", "segment_a").packet.send();
                }
                return;
            }

            this.displayLoopCountDigit(group + "[Digit3]", majorMinor, displayBrightness);

            // Lancelot key notation integer
            let numberToDisplayRemainder = numberToDisplay;
            for (const digit in ledKeyDigitModulus) {
                let leastSignificiantDigit = (numberToDisplayRemainder % 10);
                numberToDisplayRemainder = numberToDisplayRemainder - leastSignificiantDigit;
                // console.log(leastSignificiantDigit + " " + numberToDisplayRemainder + " " + group + " " + digit);
                if ((digit === "[Digit1]" && numberToDisplay < 10)) {
                    leastSignificiantDigit = -2;  // Leading zero -> Blank
                }
                if (digit !== "[Digit1]") {
                    this.displayLoopCountDigit(
                        group + digit, leastSignificiantDigit, displayBrightness);
                } else {
                    this.displayLoopCountDigit(
                        group + digit, leastSignificiantDigit, displayBrightness);
                }
                numberToDisplayRemainder /= 10;
            }
            if (sendMessage === true) {
                this.controller.getOutputField(group + "[Digit1]", "segment_a").packet.send();
            }
            return;
        } else if (this.shiftState & 0x02) {
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
                // console.log(leastSignificiantDigit + " " + numberToDisplayRemainder + " " + group + " " + digit);
                if (digit === "[Digit2]" && numberToDisplay > .1) {
                    leastSignificiantDigit = -1;  // Leading zero -> Show special symbol of number 1
                    // and the fraction stroke combined in left digit
                }
                this.displayLoopCountDigit(
                    group + digit, leastSignificiantDigit, displayBrightness);
                numberToDisplayRemainder /= 10;
            }
            if (numberToDisplay > .1) {
                this.displayLoopCountDigit(
                    group + "[Digit1]", -2, displayBrightness);  // Leading zero -> Blank
            } else {
                this.displayLoopCountDigit(
                    group + "[Digit1]", -1,
                    displayBrightness);  // Show special symbol of number 1 and the fraction stroke
                // combined in left digit
            }
        } else {
            // Beat integer
            let numberToDisplayRemainder = numberToDisplay;
            for (const digit in led3DigitModulus) {
                let leastSignificiantDigit = (numberToDisplayRemainder % 10);
                numberToDisplayRemainder = numberToDisplayRemainder - leastSignificiantDigit;
                // console.log(leastSignificiantDigit + " " + numberToDisplayRemainder + " " + group +
                // " " + digit);
                if ((digit === "[Digit1]" && numberToDisplay < 100) ||
                    (digit === "[Digit2]" && numberToDisplay < 10)) {
                    leastSignificiantDigit = -2;  // Leading zero -> Blank
                }
                if (digit !== "[Digit1]") {
                    this.displayLoopCountDigit(
                        group + digit, leastSignificiantDigit, displayBrightness);
                } else {
                    this.displayLoopCountDigit(
                        group + digit, leastSignificiantDigit, displayBrightness);
                }
                numberToDisplayRemainder /= 10;
            }
        }
        if (sendMessage === true) {
            this.controller.getOutputField(group + "[Digit1]", "segment_a").packet.send();
        }
    }

    displayLoopCountDigit(group, digit, brightness) {
        // @param offset of the first LED (center horizontal bar) of the digit
        // @param digit to display (-2 represents all OFF, -1 represents "1/" )
        // @param brightness may be aninteger value from 0x00 to 0x07
        // console.log("Offset:" + " Digit:" + digit + " Brightness:" + brightness);

        //  -- a --
        // |       |
        // f       b
        // |       |
        //  -- g --
        // |       |
        // e       c
        // |       |
        //  -- d --

        const symbolArray = {
            "-2": [[], ["a", "b", "c", "d", "e", "f", "g"]],  // All off
            "1": [["b", "c"], ["a", "d", "e", "f", "g"]],
            "2": [["a", "b", "d", "e", "g"], ["c", "f"]],
            "3": [["a", "b", "c", "d", "g"], ["e", "f"]],
            "4": [["b", "c", "f", "g"], ["a", "d", "e"]],
            "5": [["a", "c", "d", "f", "g"], ["b", "e"]],
            "6": [["a", "c", "d", "e", "f", "g"], ["b"]],
            "7": [["a", "b", "c"], ["d", "e", "f", "g"]],
            "8": [["a", "b", "c", "d", "e", "f", "g"], []],
            "9": [["a", "b", "c", "d", "f", "g"], ["e"]],
            "0": [["a", "b", "c", "d", "e", "f"], ["g"]],
            "A": [["a", "b", "c", "e", "f", "g"], ["d"]],
            "b": [["c", "d", "e", "f", "g"], ["a", "b"]],
            "-1": [["b", "e", "f"], ["a", "c", "d", "g"]],
        };
        // Switch LEDs On
        HIDController.fastForIn(symbolArray[digit][0], (segmentName) => {
            this.controller.setOutput(group, "segment_" + symbolArray[digit][0][segmentName], brightness, false);
        }
        );
        // Switch LEDs Off

        HIDController.fastForIn(symbolArray[digit][1], (segmentName) => {
            this.controller.setOutput(group, "segment_" + symbolArray[digit][1][segmentName], kLedOff, false);
        }
        );
    }

    registerOutputPackets() {
        console.log("TraktorZ2: registerOutputPackets");

        // Register outputs, which only exist on the 2 main decks
        this.Decks.deck1.registerOutputs2Decks();
        this.Decks.deck2.registerOutputs2Decks();

        // Register outputs, which exist on all 4 decks
        for (const idx in this.Decks) {
            this.Decks[idx].registerOutputs4Decks();
        }

        this.outputReport80.addOutput("[Main]", "!vinylcontrolstatus", 0x02, "B", 0x7F);

        this.outputReport80.addOutput("[Channel1]", "quantize", 0x03, "B", 0x7F);
        engine.makeConnection("[Channel1]", "quantize", this.basicOutputHandler.bind(this));
        engine.trigger("[Channel1]", "quantize");
        this.outputReport80.addOutput("[Channel2]", "quantize", 0x04, "B", 0x7F);
        engine.makeConnection("[Channel2]", "quantize", this.basicOutputHandler.bind(this));
        engine.trigger("[Channel2]", "quantize");

        this.outputReport80.addOutput("[Master]", "skin_settings", 0x05, "B", 0x7F);
        // engine.makeConnection("[Master]", "skin_settings", this.basicOutputHandler.bind(this));
        // engine.trigger("[Master]", "skin_settings");
        this.outputReport80.addOutput("[Master]", "shift", 0x06, "B", 0x7F);

        this.outputReport80.addOutput("[EffectRack1_EffectUnit1]", "!On", 0x0D, "B", 0x7F);
        engine.makeConnection(
            "[EffectRack1_EffectUnit1_Effect1]", "enabled", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit1_Effect1]", "loaded", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit1_Effect2]", "enabled", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit1_Effect2]", "loaded", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit1_Effect3]", "enabled", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit1_Effect3]", "loaded", this.fxOnLedHandler.bind(this));
        engine.trigger("[EffectRack1_EffectUnit1_Effect1]", "enabled");

        this.outputReport80.addOutput(
            "[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x0F, "B", 0x7F);
        engine.makeConnection(
            "[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", this.basicOutputHandler.bind(this));
        engine.trigger("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable");
        this.outputReport80.addOutput(
            "[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x10, "B", 0x7F);
        engine.makeConnection(
            "[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", this.basicOutputHandler.bind(this));
        engine.trigger("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable");

        this.outputReport80.addOutput("[EffectRack1_EffectUnit2]", "!On", 0x15, "B", 0x7F);
        engine.makeConnection(
            "[EffectRack1_EffectUnit2_Effect1]", "enabled", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit2_Effect1]", "loaded", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit2_Effect2]", "enabled", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit2_Effect2]", "loaded", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit2_Effect3]", "enabled", this.fxOnLedHandler.bind(this));
        engine.makeConnection(
            "[EffectRack1_EffectUnit2_Effect3]", "loaded", this.fxOnLedHandler.bind(this));
        engine.trigger("[EffectRack1_EffectUnit2_Effect1]", "enabled");

        this.outputReport80.addOutput(
            "[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x17, "B", 0x7F);
        engine.makeConnection(
            "[EffectRack1_EffectUnit1]", "group_[Channel2]_enable",
            this.basicOutputHandler.bind(this));
        engine.trigger("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable");
        this.outputReport80.addOutput(
            "[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x18, "B", 0x7F);
        engine.makeConnection(
            "[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", this.basicOutputHandler.bind(this));
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
                // 7 bits control Deck 1 3rd 7 segment center horizontal bar brightness (orange)
                this.outputReport80.addOutput(
                    ch + digit, "segment_g", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x00, "B", 0x7F);

                // 7 bits control Deck 1 3rd 7 segment lower right vertical bar brightness (orange)
                this.outputReport80.addOutput(
                    ch + digit, "segment_c", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x01,
                    "B", 0x7F);

                // 7 bits control Deck 1 3rd 7 segment upper right vertical bar brightness (orange)
                this.outputReport80.addOutput(
                    ch + digit, "segment_b", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x02, "B", 0x7F);

                // 7 bits control Deck 1 3rd 7 segment upper horizontal bar brightness (orange)
                this.outputReport80.addOutput(
                    ch + digit, "segment_a", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x03, "B", 0x7F);

                // 7 bits control Deck 1 3rd 7 segment upper left vertical bar brightness (orange)
                this.outputReport80.addOutput(
                    ch + digit, "segment_f", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x04, "B", 0x7F);

                // 7 bits control Deck 1 3rd 7 segment lower left vertical bar brightness (orange)
                this.outputReport80.addOutput(
                    ch + digit, "segment_e", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x05, "B", 0x7F);

                // 7 bits control Deck 1 3rd 7 segment lower horizontal bar brightness (orange)
                this.outputReport80.addOutput(
                    ch + digit, "segment_d", ledChannelOffsets[ch] + ledDigitOffsets[digit] + 0x06, "B", 0x7F);
            }
        }

        this.controller.registerOutputPacket(this.outputReport80);

        // Microphone button LED
        this.outputReport81.addOutput("[Main]", "!microphonebuttonled", 0x22, "B", 0x7F);

        this.outputReport81.addOutput("[Main]", "!usblight", 0x27, "B", 0x7F);

        // Ch3 / Ch4 VuMeter usage depends on context ->  ChC / MasterL and ChD / MasterR
        engine.makeUnbufferedConnection("[Main]", "vu_meter_left", this.displayVuValue.bind(this));
        engine.makeUnbufferedConnection("[Main]", "vu_meter_right", this.displayVuValue.bind(this));
        engine.makeConnection("[Main]", "peak_indicator_left", this.displayPeakIndicator.bind(this));
        engine.makeConnection("[Main]", "peak_indicator_right", this.displayPeakIndicator.bind(this));
        this.outputReport81.addOutput("[Main]", "!vu_labelMst", 0x21, "B", 0x7F);

        this.outputReport81.addOutput("[Main]", "!crossfaderReverse", 0x28, "B", 0x7F);

        this.controller.registerOutputPacket(this.outputReport81);
    }

    displayVuValue(value, group, key) {
        let ch;

        if ((group === "[Main]") && (key === "vu_meter_left")) {
            // MasterL
            ch = "[Channel3]";
        } else if ((group === "[Main]") && (key === "vu_meter_right")) {
            // MasterR
            ch = "[Channel4]";
        } else if (
            (group === "[Channel1]") && (key === "vu_meter") && (this.pregainCh3Timer === 0)) {
            // ChA
            ch = "[Channel1]";
        } else if (
            (group === "[Channel3]") && (key === "vu_meter") && (this.pregainCh3Timer !== 0)) {
            // ChC
            ch = "[Channel1]";
        } else if (
            (group === "[Channel2]") && (key === "vu_meter") && (this.pregainCh4Timer === 0)) {
            // ChB
            ch = "[Channel2]";
        } else if (
            (group === "[Channel4]") && (key === "vu_meter") && (this.pregainCh4Timer !== 0)) {
            // ChD
            ch = "[Channel2]";
        } else {
            return;  // Hidden Channel of the pairs A/C or B/D
        }

        for (let i = 0; i < 6; i++) {
            let brightness = ((value * 6) - i) * kLedVuMeterBrightness;
            if (brightness < kLedOff) {
                brightness = kLedOff;
            }
            if (brightness > kLedVuMeterBrightness) {
                brightness = kLedVuMeterBrightness;
            }
            this.controller.setOutput(ch, "!vu_meter" + i, brightness, false);
        }
    }

    displayPeakIndicator(value, group, key) {
        let ch;

        if ((group === "[Main]") && (key === "peak_indicator_left")) {
            // MasterL
            ch = "[Channel3]";
        } else if ((group === "[Main]") && (key === "peak_indicator_right")) {
            // MasterR
            ch = "[Channel4]";
        } else if (
            (group === "[Channel1]") && (key === "peak_indicator") && (this.pregainCh3Timer === 0)) {
            // ChA
            ch = "[Channel1]";
            this.controller.setOutput("[Channel1]", "!vu_label", kLedVuMeterBrightness, false);
            this.controller.setOutput("[Channel3]", "!vu_label", kLedOff, false);
        } else if (
            (group === "[Channel3]") && (key === "peak_indicator") && (this.pregainCh3Timer !== 0)) {
            // ChC
            ch = "[Channel1]";
            this.controller.setOutput("[Channel1]", "!vu_label", kLedOff, false);
            this.controller.setOutput("[Channel3]", "!vu_label", kLedVuMeterBrightness, false);
        } else if (
            (group === "[Channel2]") && (key === "peak_indicator") && (this.pregainCh4Timer === 0)) {
            // ChB
            ch = "[Channel2]";
            this.controller.setOutput("[Channel2]", "!vu_label", kLedVuMeterBrightness, false);
            this.controller.setOutput("[Channel4]", "!vu_label", kLedOff, false);
        } else if (
            (group === "[Channel4]") && (key === "peak_indicator") && (this.pregainCh4Timer !== 0)) {
            // ChD
            ch = "[Channel2]";
            this.controller.setOutput("[Channel2]", "!vu_label", kLedOff, false);
            this.controller.setOutput("[Channel4]", "!vu_label", kLedVuMeterBrightness, false);
        } else {
            return;  // Hidden Channel of the pairs A/C or B/D
        }

        if (value !== 0) {
            this.controller.setOutput(ch, "!peak_indicator", kLedBright, false);
        } else {
            this.controller.setOutput(ch, "!peak_indicator", kLedOff, false);
        }
    }

    enableLEDsPerChannel() {
        console.log("TraktorZ2: enableLEDsPerChannel");
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
        controller.sendFeatureReport(0xF1, data.buffer);

        this.dataF1 = controller.getFeatureReport(0xF1);
        /*
         this.dataF1[0] = 0x9F;
         this.dataF1 = [0xFF, 0x40];
         controller.sendFeatureReport(0xF1, this.dataF1);*/

        console.log(controller.getFeatureReport(0xF1));  // 2x8Bit Logical 0...255
    }

    shutdown() {
        this.controller.setOutput("[Main]", "!usblight", kLedBright, true);

        // Switch software mixing mode of and given LED control to mixer hardware
        const featureRptF1 = new Uint8Array([0x00, 0x00]);
        controller.sendFeatureReport(0xF1, featureRptF1.buffer);

        console.log("TraktorZ2: Shutdown done!");
    }
}

var TraktorZ2 = new TraktorZ2Class;  // eslint-disable-line no-var, no-unused-vars

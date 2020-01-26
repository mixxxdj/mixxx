////////////////////////////////////////////////////////////////////////
// Controller: Reloop Digital Jockey 2
// URL:        na razie nic
// Author:     DJ aK
// Credits:    Uwe Klotz a/k/a tapir (baseline: Denon MC6000MK2 script)
////////////////////////////////////////////////////////////////////////

var RDJ2 = {};


////////////////////////////////////////////////////////////////////////
// Tunable constants                                                  //
////////////////////////////////////////////////////////////////////////

RDJ2.JOG_SPIN_CUE_PEAK = 0.2; // [0.0, 1.0]
RDJ2.JOG_SPIN_CUE_EXPONENT = 0.7; // 1.0 = linear response

RDJ2.JOG_SPIN_PLAY_PEAK = 0.3; // [0.0, 1.0]
RDJ2.JOG_SPIN_PLAY_EXPONENT = 0.7; // 1.0 = linear response

RDJ2.JOG_SCRATCH_RPM = 33.333333; // 33 1/3
RDJ2.JOG_SCRATCH_ALPHA = 0.125; // 1/8
RDJ2.JOG_SCRATCH_BETA = RDJ2.JOG_SCRATCH_ALPHA / 32.0;
RDJ2.JOG_SCRATCH_RAMP = true; // required for back spins

// Seeking: Number of revolutions needed to seek from the beginning
// to the end of the track.
RDJ2.JOG_SEEK_REVOLUTIONS = 2;


////////////////////////////////////////////////////////////////////////
// Fixed constants                                                    //
////////////////////////////////////////////////////////////////////////

// Controller constants
RDJ2.DECK_COUNT = 2;
RDJ2.JOG_RESOLUTION = 128; // measured/estimated


// Jog constants
RDJ2.MIDI_JOG_DELTA_BIAS = 0x40; // center value of relative movements
RDJ2.MIDI_JOG_DELTA_RANGE = 0x3F; // both forward (= positive) and reverse (= negative)


// Mixxx constants
RDJ2.MIXXX_JOG_RANGE = 3.0;


////////////////////////////////////////////////////////////////////////
// Logging functions                                                  //
////////////////////////////////////////////////////////////////////////

RDJ2.logDebug = function (msg) {
    if (RDJ2.debug) {
        print("[" + RDJ2.id + " DEBUG] " + msg);
    }
};

RDJ2.logInfo = function (msg) {
    print("[" + RDJ2.id + " INFO] " + msg);
};

RDJ2.logWarning = function (msg) {
    print("[" + RDJ2.id + " WARNING] " + msg);
};

RDJ2.logError = function (msg) {
    print("[" + RDJ2.id + " ERROR] " + msg);
};


////////////////////////////////////////////////////////////////////////
// Buttons                                                            //
////////////////////////////////////////////////////////////////////////

RDJ2.MIDI_BUTTON_ON = 0x7F;
RDJ2.MIDI_BUTTON_OFF = 0x00;

RDJ2.isButtonPressed = function (midiValue) {
    switch (midiValue) {
        case RDJ2.MIDI_BUTTON_ON:
            return true;
        case RDJ2.MIDI_BUTTON_OFF:
            return false;
        default:
            RDJ2.logError("Unexpected MIDI button value: " + midiValue);
            return undefined;
    }
};

/* This map is necessary as Reloop has designed the controller in such
   a way that not all buttons/knobs have the same offset comparing
   CH0 and CH1. By looking at the MIDI messages sent by the controller,
   we can see that the hardware is designed as symmetric halves.
   
   In other words, constant offset in hardware corresponds to symmetric
   halves, but the controller layout is not fully symmetric.
   (e.x. ACTIVATE 1 buttons)

   Thus we need a map to preserve object oriented approach .*/
RDJ2.BUTTONMAP_CH0_CH1 = {
    play: [0x19, 0x55],
    cue: [0x18, 0x54],
    sync: [0x01, 0x3D],
    scratch: [0x1B, 0x57]
}

/* Custom buttons */
RDJ2.ScratchButton = function (options) {
    this.state = undefined;
    components.Button.call(this, options);
    this.init();
};
RDJ2.ScratchButton.prototype = new components.Button({
    type: components.Button.prototype.types.toggle,
    init: function () {
        this.state = false;
        this.send(this.outValueScale(this.state));
    },
    input: function (isButtonPressed) {
        if (isButtonPressed) {
            this.state = !this.state;
            this.send(this.outValueScale(this.state));
        }
    },
    isActive: function () {
        return this.state;
    }
});

RDJ2.ShiftButton = function (options) {
    this.state = false;
    this.connectedContainers = [];
    components.Button.call(this, options);
};
RDJ2.ShiftButton.prototype = new components.Button({
    input: function (channel, control, value) {
        //update shift state
        this.state = RDJ2.isButtonPressed(value);
        this.send(this.outValueScale(this.state));

        //call shift()/unshift() for each connected container
        if (this.state) {
            //RDJ2.logDebug("Shifting containers");
            this.connectedContainers.forEach(function (container) {
                container.shift();
            });
        } else {
            //RDJ2.logDebug("Unshifting containers");
            this.connectedContainers.forEach(function (container) {
                container.unshift();
            });
        }
    },
    isActive: function () {
        return this.state;
    },
    connectContainer: function (container) {
        if (container instanceof components.ComponentContainer === false) {
            RDJ2.logError("Container type mismatch");
        }
        else
        {
            RDJ2.logDebug("Connecting container to shift button");
            this.connectedContainers.push(container);
        }
    }
});

////////////////////////////////////////////////////////////////////////
// Knobs                                                              //
////////////////////////////////////////////////////////////////////////

RDJ2.MIDI_KNOB_INC = 0x41;
RDJ2.MIDI_KNOB_DEC = 0x3F;
RDJ2.MIDI_KNOB_DELTA_BIAS = 0x40; // center value of relative movements
RDJ2.MIDI_KNOB_STEPS = 20;

//currently RDJ2.getKnobDeltaOld is not used
RDJ2.getKnobDeltaOld = function (midiValue) {
    switch (midiValue) {
        case RDJ2.MIDI_KNOB_INC:
            return 1;
        case RDJ2.MIDI_KNOB_DEC:
            return -1;
        default:
            RDJ2.logError("Unexpected MIDI knob value: " + midiValue);
            return 0;
    }
};

RDJ2.getKnobDelta = function (midiValue) {
        return midiValue - RDJ2.MIDI_KNOB_DELTA_BIAS;
};

RDJ2.knobInput = function (channel, control, value, status, group) {
        var knobDelta = RDJ2.getKnobDelta(value);
        this.inSetParameter(this.inGetParameter() + knobDelta / RDJ2.MIDI_KNOB_STEPS);
};

////////////////////////////////////////////////////////////////////////
// Controls                                                           //
////////////////////////////////////////////////////////////////////////

//code removed - looks as deprecated

////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

/* Management */

//TODO REMOVE
// RDJ2.decksByGroup = {};

// RDJ2.getDeckByGroup = function (group) {
//     var deck = RDJ2.decksByGroup[group];
//     if (undefined === deck) {
//         RDJ2.logError("No deck found for " + group);
//     }
//     return deck;
// };

/* Constructor */

RDJ2.Deck = function (number) {
    RDJ2.logDebug("Creating Deck " + number);

    this.number = number;
    this.group = "[Channel" + number + "]";
    this.filterGroup = "[QuickEffectRack1_" + this.group + "_Effect1]";
    this.rateDirBackup = this.getValue("rate_dir");
    this.setValue("rate_dir", -1);
    this.vinylMode = false;
    this.jogTouchState = false;
    this.syncMode = undefined;

    components.Deck.call(this, number);

    this.playButton = new components.PlayButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.play[number - 1]]);
    this.cueButton = new components.CueButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.cue[number - 1]]);
    this.syncButton = new components.SyncButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.sync[number - 1]]);
    this.scratchButton = new RDJ2.ScratchButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.scratch[number - 1]]);

    // Set the group properties of the above Components and connect their output callback functions
    // Without this, the group property for each Component would have to be specified to its
    // constructor.
    this.reconnectComponents(function (component) {
        if (component.group === undefined) {
            // 'this' inside a function passed to reconnectComponents refers to the ComponentContainer
            // so 'this' refers to the custom Deck object being constructed
            component.group = this.currentDeck;
        }
    });
};

// give our custom Deck all the methods of the generic Deck in the Components library
RDJ2.Deck.prototype = Object.create(components.Deck.prototype);

/* Values & Parameters */

RDJ2.Deck.prototype.getValue = function (key) {
    return engine.getValue(this.group, key);
};

RDJ2.Deck.prototype.setValue = function (key, value) {
    engine.setValue(this.group, key, value);
};

RDJ2.Deck.prototype.toggleValue = function (key) {
    this.setValue(key, !this.getValue(key));
};

RDJ2.Deck.prototype.setParameter = function (key, param) {
    engine.setParameter(this.group, key, param);
};

RDJ2.Deck.prototype.triggerValue = function (key) {
    engine.trigger(this.group, key);
};

/* Cue & Play */

RDJ2.Deck.prototype.isPlaying = function () {
    return this.getValue("play");
};

/* Pitch Bend / Track Search */

RDJ2.Deck.prototype.onBendPlusButton = function (isButtonPressed) {
    if (this.isPlaying()) {
        this.setValue("fwd", false);
        /*if (this.getShiftState()) {
            this.setValue("rate_temp_up_small", isButtonPressed);
        } else*/ {
            this.setValue("rate_temp_up", isButtonPressed);
        }
    } else {
        this.setValue("fwd", isButtonPressed);
    }
};

RDJ2.Deck.prototype.onBendMinusButton = function (isButtonPressed) {
    if (this.isPlaying()) {
        this.setValue("back", false);
        /*if (this.getShiftState()) {
            this.setValue("rate_temp_down_small", isButtonPressed);
        } else*/ {
            this.setValue("rate_temp_down", isButtonPressed);
        }
    } else {
        this.setValue("back", isButtonPressed);
    }
};

/* Vinyl Mode (Scratching) */

RDJ2.Deck.prototype.onScratchButton = function (isButtonPressed) {
    this.scratchButton.input(isButtonPressed);
    this.vinylMode = this.scratchButton.isActive();

};

/* Jog Wheel */

RDJ2.Deck.prototype.onJogTouch = function (isJogTouched) {    
    this.jogTouchState = isJogTouched;
    
    if (this.vinylMode && isJogTouched) {
        engine.scratchEnable(this.number,
            RDJ2.JOG_RESOLUTION,
            RDJ2.JOG_SCRATCH_RPM,
            RDJ2.JOG_SCRATCH_ALPHA,
            RDJ2.JOG_SCRATCH_BETA,
            RDJ2.JOG_SCRATCH_RAMP);
    } else if (!isJogTouched && engine.isScratching(this.number)) {
        engine.scratchDisable(this.number, RDJ2.JOG_SCRATCH_RAMP);
    }
};

RDJ2.Deck.prototype.onJogSpin = function (jogDelta) {
    if (engine.isScratching(this.number)) {
        engine.scratchTick(this.number, jogDelta);
    } else if (this.jogTouchState && !this.isPlaying()) {
        // fast track seek (strip search)
        var playPos = engine.getValue(this.group, "playposition");
        if (undefined !== playPos) {
            var seekPos = playPos + (jogDelta / (RDJ2.JOG_RESOLUTION * RDJ2.JOG_SEEK_REVOLUTIONS));
            this.setValue("playposition", Math.max(0.0, Math.min(1.0, seekPos)));
        }
    } else {
        var normalizedDelta = jogDelta / RDJ2.MIDI_JOG_DELTA_RANGE;
        var scaledDelta;
        var jogExponent;
        if (this.isPlaying()) {
            // bending
            scaledDelta = normalizedDelta / RDJ2.JOG_SPIN_PLAY_PEAK;
            jogExponent = RDJ2.JOG_SPIN_PLAY_EXPONENT;
        } else {
            // cueing
            scaledDelta = normalizedDelta / RDJ2.JOG_SPIN_CUE_PEAK;
            jogExponent = RDJ2.JOG_SPIN_CUE_EXPONENT;
        }
        var direction;
        var scaledDeltaAbs;
        if (scaledDelta < 0.0) {
            direction = -1.0;
            scaledDeltaAbs = -scaledDelta;
        } else {
            direction = 1.0;
            scaledDeltaAbs = scaledDelta;
        }
        var scaledDeltaPow = direction * Math.pow(scaledDeltaAbs, jogExponent);
        var jogValue = RDJ2.MIXXX_JOG_RANGE * scaledDeltaPow;
        this.setValue("jog", jogValue);
    }
};

/* MIDI Input Callbacks */

RDJ2.Deck.prototype.recvBendPlusButton = function (channel, control, value) {
    this.onBendPlusButton(RDJ2.isButtonPressed(value));
};

RDJ2.Deck.prototype.recvBendMinusButton = function (channel, control, value) {
    this.onBendMinusButton(RDJ2.isButtonPressed(value));
};

RDJ2.Deck.prototype.recvJogTouch = function (channel, control, value) {
    this.onJogTouch(RDJ2.isButtonPressed(value));
};

RDJ2.Deck.prototype.recvJogSpin = function (channel, control, value) {
    this.onJogSpin(RDJ2.getJogDeltaValue(value));
};

RDJ2.Deck.prototype.recvScratchButton = function (channel, control, value) {
    this.onScratchButton(RDJ2.isButtonPressed(value));
};

////////////////////////////////////////////////////////////////////////
// Effects                                                            //
////////////////////////////////////////////////////////////////////////

//function for overriding default unshift/shift functions of efx unit knobs
RDJ2.efxUnitKnobUnshift = function () {
    this.input = function (channel, control, value, status, group) {
        var knobDelta = RDJ2.getKnobDelta(value);
        this.inSetParameter(this.inGetParameter() + knobDelta / RDJ2.MIDI_KNOB_STEPS);

        if (this.previousValueReceived === undefined) {
            var effect = '[EffectRack1_EffectUnit' + this.eu.currentUnitNumber +
                        '_Effect' + this.number + ']';
            engine.softTakeover(effect, 'meta', true);
            engine.softTakeover(effect, 'parameter1', true);
            engine.softTakeover(effect, 'parameter2', true);
            engine.softTakeover(effect, 'parameter3', true);
        }
        this.previousValueReceived = value;
    };
};

////////////////////////////////////////////////////////////////////////
// Controller functions                                               //
////////////////////////////////////////////////////////////////////////

RDJ2.group = "[Master]";

RDJ2.getValue = function (key) {
    return engine.getValue(RDJ2.group, key);
};

RDJ2.setValue = function (key, value) {
    engine.setValue(RDJ2.group, key, value);
};

RDJ2.getJogDeltaValue = function (value) {
    if (0x00 === value) {
        return 0x00;
    } else {
        return value - RDJ2.MIDI_JOG_DELTA_BIAS;
    }
};


////////////////////////////////////////////////////////////////////////
// MIDI callback functions without a group                            //
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// MIDI [Channel<n>] callback functions                               //
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// Mixxx connected controls callback functions                        //
////////////////////////////////////////////////////////////////////////

//some out of context code was here (?)

////////////////////////////////////////////////////////////////////////
// Mixxx Callback Functions                                           //
////////////////////////////////////////////////////////////////////////

RDJ2.init = function (id, debug) {
    RDJ2.id = id;
    RDJ2.debug = debug;
    
    RDJ2.logInfo("Initializing controller");

    //left and right shift buttons
    RDJ2.leftShiftButton = new RDJ2.ShiftButton([0x90, 0x0A]);
    RDJ2.rightShiftButton = new RDJ2.ShiftButton([0x90, 0x46]);
    
    // left and right deck
    RDJ2.leftDeck = new RDJ2.Deck(1);
    RDJ2.rightDeck = new RDJ2.Deck(2);

    //efx unit
    RDJ2.fx1 = new components.EffectUnit(1);
    RDJ2.fx1.EffectUnitKnob.prototype.unshift = RDJ2.efxUnitKnobUnshift;
    RDJ2.fx1.EffectUnitKnob.prototype.eu = RDJ2.fx1;    //hack for use by reimplemented unshift/shift
    RDJ2.fx1.enableButtons[1].midi = [0x90, 0x07];
    RDJ2.fx1.enableButtons[2].midi = [0x90, 0x0C];
    RDJ2.fx1.enableButtons[3].midi = [0x90, 0x09];
    RDJ2.fx1.knobs[1].midi = [0xB0, 0x07];
    RDJ2.fx1.knobs[2].midi = [0xB0, 0x08];
    RDJ2.fx1.knobs[3].midi = [0xB0, 0x09];
    RDJ2.fx1.dryWetKnob.midi = [0xB0, 0x0A];
    RDJ2.fx1.dryWetKnob.input = RDJ2.knobInput;
    RDJ2.fx1.effectFocusButton.midi = [0x90, 0x0E];
    // We need to call unshift() again for each EffectUnitKnob as we
    // swapped its implementation after fx object construction (when 
    // it is called automatically)
    for (var n = 1; n <= 3; n++) {
        RDJ2.fx1.knobs[n].unshift();
    }
    // Now init the fx unit
    RDJ2.fx1.init();

    //connect decks and efx units to shift buttons
    RDJ2.leftShiftButton.connectContainer(RDJ2.leftDeck);
    RDJ2.leftShiftButton.connectContainer(RDJ2.fx1);
    RDJ2.rightShiftButton.connectContainer(RDJ2.rightDeck);
};

RDJ2.shutdown = function () {
    RDJ2.logInfo("Shutting down controller");
};
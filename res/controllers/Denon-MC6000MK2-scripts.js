////////////////////////////////////////////////////////////////////////
// Controller: Denon MC6000MK2
// URL:        http://www.mixxx.org/wiki/doku.php/denon_mc6000mk2
// Author:     Uwe Klotz <uklotz@mixxx.org>
////////////////////////////////////////////////////////////////////////

var DenonMC6000MK2 = {};

////////////////////////////////////////////////////////////////////////
// Tunable constants                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.SAMPLER_MODE = {
    HOLD: {
        value: 1,
        name: "Hold",
        description: "Play sample from beginning to end while pressing the Sample button. Playback continues when Shift is pressed while releasing the Sample button."
    },
    TRIGGER: {
        value: 2,
        name: "Trigger",
        description: "Play sample from beginning to end until stopped by pressing Shift + Sample button."
    }
};

DenonMC6000MK2.DEFAULT_SAMPLER_MODE = DenonMC6000MK2.SAMPLER_MODE.HOLD;

DenonMC6000MK2.JOG_SPIN_CUE_PEAK = 0.2; // [0.0, 1.0]
DenonMC6000MK2.JOG_SPIN_CUE_EXPONENT = 0.7; // 1.0 = linear response

DenonMC6000MK2.JOG_SPIN_PLAY_PEAK = 0.3; // [0.0, 1.0]
DenonMC6000MK2.JOG_SPIN_PLAY_EXPONENT = 0.7; // 1.0 = linear response

DenonMC6000MK2.JOG_SCRATCH_RPM = 33.333333; // 33 1/3
DenonMC6000MK2.JOG_SCRATCH_ALPHA = 0.125; // 1/8
DenonMC6000MK2.JOG_SCRATCH_BETA = DenonMC6000MK2.JOG_SCRATCH_ALPHA / 32.0;
DenonMC6000MK2.JOG_SCRATCH_RAMP = true; // required for back spins

DenonMC6000MK2.EFX_MIX_ENCODER_STEPS = 20;

// Seeking: Number of revolutions needed to seek from the beginning
// to the end of the track.
DenonMC6000MK2.JOG_SEEK_REVOLUTIONS = 2;

DenonMC6000MK2.GLOBAL_SHIFT_STATE = true;

////////////////////////////////////////////////////////////////////////
// Fixed constants                                                    //
////////////////////////////////////////////////////////////////////////

// Controller constants
DenonMC6000MK2.BRAND = "Denon";
DenonMC6000MK2.MODEL = "MC6000MK2";
DenonMC6000MK2.SIDE_COUNT = 2;
DenonMC6000MK2.DECK_COUNT = 4;
DenonMC6000MK2.SAMPLER_COUNT_PER_SIDE = 4;
DenonMC6000MK2.SAMPLE_RATE = 44100;
DenonMC6000MK2.JOG_RESOLUTION = 600; // delta ticks per revolution (measured/estimated)

DenonMC6000MK2.EFX_RACK = "EffectRack1";
DenonMC6000MK2.LEFT_EFX_UNIT = "EffectUnit1";
DenonMC6000MK2.RIGHT_EFX_UNIT = "EffectUnit2";

DenonMC6000MK2.LEFT_EFX_UNIT_GROUP = "[EffectRack1_EffectUnit1]";
DenonMC6000MK2.LEFT_EFX_PARAM_GROUP = "[EffectRack1_EffectUnit1_Effect1]";
DenonMC6000MK2.RIGHT_EFX_UNIT_GROUP = "[EffectRack1_EffectUnit2]";
DenonMC6000MK2.RIGHT_EFX_PARAM_GROUP = "[EffectRack1_EffectUnit2_Effect1]";

// Jog constants
DenonMC6000MK2.MIDI_JOG_DELTA_BIAS = 0x40; // center value of relative movements
DenonMC6000MK2.MIDI_JOG_DELTA_RANGE = 0x3F; // both forward (= positive) and reverse (= negative)

// Mixxx constants
DenonMC6000MK2.MIXXX_JOG_RANGE = 3.0;
DenonMC6000MK2.MIXXX_SYNC_NONE = 0;
DenonMC6000MK2.MIXXX_XFADER_LEFT = 0;
DenonMC6000MK2.MIXXX_XFADER_CENTER = 1;
DenonMC6000MK2.MIXXX_XFADER_RIGHT = 2;
DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED = -1;

// MIDI constants
DenonMC6000MK2.MIDI_CH0 = 0x00;
DenonMC6000MK2.MIDI_CH1 = 0x01;
DenonMC6000MK2.MIDI_CH2 = 0x02;
DenonMC6000MK2.MIDI_CH3 = 0x03;

////////////////////////////////////////////////////////////////////////
// Globals                                                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.sidesByGroup = {};

DenonMC6000MK2.getSideByGroup = function(group) {
    var side = DenonMC6000MK2.sidesByGroup[group];
    if (side === undefined) {
        DenonMC6000MK2.logError("No side found for " + group);
    } else {
        if (side instanceof DenonMC6000MK2.Side === false) {
            DenonMC6000MK2.logError("Unexpected type: " + typeof side);
        }
    }
    return side;
};

DenonMC6000MK2.oldSidesByGroup = {};

DenonMC6000MK2.getOldSideByGroup = function(group) {
    var side = DenonMC6000MK2.oldSidesByGroup[group];
    if (undefined === side) {
        DenonMC6000MK2.logError("No side found for " + group);
    } else {
        if (side instanceof DenonMC6000MK2.OldSide === false) {
            DenonMC6000MK2.logError("Unexpected type: " + typeof side);
        }
    }
    return side;
};

DenonMC6000MK2.oldDecksByGroup = {};

DenonMC6000MK2.getOldDeckByGroup = function(group) {
    var deck = DenonMC6000MK2.oldDecksByGroup[group];
    if (deck === undefined) {
        DenonMC6000MK2.logError("No deck found for " + group);
    } else {
        if (deck instanceof DenonMC6000MK2.OldDeck === false) {
            DenonMC6000MK2.logError("Unexpected type: " + typeof deck);
        }
    }
    return deck;
};

////////////////////////////////////////////////////////////////////////
// Logging functions                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.logDebug = function(msg) {
    if (DenonMC6000MK2.debug) {
        print("[" + DenonMC6000MK2.id + " DEBUG] " + msg);
    }
};

DenonMC6000MK2.logInfo = function(msg) {
    print("[" + DenonMC6000MK2.id + " INFO] " + msg);
};

DenonMC6000MK2.logWarning = function(msg) {
    print("[" + DenonMC6000MK2.id + " WARNING] " + msg);
};

DenonMC6000MK2.logError = function(msg) {
    print("[" + DenonMC6000MK2.id + " ERROR] " + msg);
};


////////////////////////////////////////////////////////////////////////
// Buttons                                                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_BUTTON_ON = 0x40;
DenonMC6000MK2.MIDI_BUTTON_OFF = 0x00;

DenonMC6000MK2.isButtonPressed = function(midiValue) {
    switch (midiValue) {
    case DenonMC6000MK2.MIDI_BUTTON_ON:
        return true;
    case DenonMC6000MK2.MIDI_BUTTON_OFF:
        return false;
    default:
        DenonMC6000MK2.logError("Unexpected MIDI button value: " + midiValue);
        return undefined;
    }
};


////////////////////////////////////////////////////////////////////////
// Knobs                                                              //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_KNOB_INC = 0x00;
DenonMC6000MK2.MIDI_KNOB_DEC = 0x7F;

DenonMC6000MK2.getKnobDelta = function(midiValue) {
    switch (midiValue) {
    case DenonMC6000MK2.MIDI_KNOB_INC:
        return 1;
    case DenonMC6000MK2.MIDI_KNOB_DEC:
        return -1;
    default:
        DenonMC6000MK2.logError("Unexpected MIDI knob value: " + midiValue);
        return 0;
    }
};


////////////////////////////////////////////////////////////////////////
// LEDs                                                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_LED_ON = 0x50;
DenonMC6000MK2.MIDI_LED_OFF = 0x51;

DenonMC6000MK2.LedState = function(midiCtrl) {
    this.midiCtrl = midiCtrl;
};

DenonMC6000MK2.LED_ON = new DenonMC6000MK2.LedState(DenonMC6000MK2.MIDI_LED_ON);
DenonMC6000MK2.LED_OFF = new DenonMC6000MK2.LedState(DenonMC6000MK2.MIDI_LED_OFF);

DenonMC6000MK2.Led = function(midiChannel, midiValue) {
    this.midiChannel = midiChannel;
    this.midiValue = midiValue;
    this.state = undefined;
};

DenonMC6000MK2.Led.prototype.setState = function(ledState) {
    this.state = ledState;
    this.trigger();
};

DenonMC6000MK2.Led.prototype.setStateBoolean = function(booleanValue) {
    if (booleanValue) {
        this.setState(DenonMC6000MK2.LED_ON);
    } else {
        this.setState(DenonMC6000MK2.LED_OFF);
    }
};

DenonMC6000MK2.Led.prototype.trigger = function() {
    if (this.state === undefined) {
        return;
    }
    midi.sendShortMsg(0xB0 + this.midiChannel, this.state.midiCtrl, this.midiValue);
};

DenonMC6000MK2.Led.prototype.reset = function() {
    this.setStateBoolean(false);
};

DenonMC6000MK2.MIDI_TRI_LED_ON = 0x4A;
DenonMC6000MK2.MIDI_TRI_LED_OFF = 0x4B;
DenonMC6000MK2.MIDI_TRI_LED_BLINK = 0x4C;

DenonMC6000MK2.TriLedState = function(midiCtrl) {
    this.midiCtrl = midiCtrl;
};

DenonMC6000MK2.TRI_LED_ON = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_ON);
DenonMC6000MK2.TRI_LED_OFF = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_OFF);
DenonMC6000MK2.TRI_LED_BLINK = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_BLINK);

DenonMC6000MK2.TriLed = function(midiChannel, midiValue) {
    this.midiChannel = midiChannel;
    this.midiValue = midiValue;
    this.state = undefined;
};

DenonMC6000MK2.TriLed.prototype.setTriState = function(ledState) {
    this.state = ledState;
    this.trigger();
};

DenonMC6000MK2.TriLed.prototype.setStateBoolean = function(booleanValue) {
    if (booleanValue) {
        this.setTriState(DenonMC6000MK2.TRI_LED_ON);
    } else {
        this.setTriState(DenonMC6000MK2.TRI_LED_OFF);
    }
};

DenonMC6000MK2.TriLed.prototype.trigger = function() {
    midi.sendShortMsg(0xB0 + this.midiChannel, this.state.midiCtrl, this.midiValue);
};

DenonMC6000MK2.TriLed.prototype.reset = function() {
    this.setStateBoolean(false);
};

DenonMC6000MK2.connectedLeds = [];

DenonMC6000MK2.connectLed = function(midiChannel, midiValue) {
    var led = new DenonMC6000MK2.Led(midiChannel, midiValue);
    led.reset();
    DenonMC6000MK2.connectedLeds.push(led);
    return led;
};

DenonMC6000MK2.connectTriLed = function(midiChannel, midiValue) {
    var led = new DenonMC6000MK2.TriLed(midiChannel, midiValue);
    led.reset();
    DenonMC6000MK2.connectedLeds.push(led);
    return led;
};

DenonMC6000MK2.disconnectLeds = function() {
    for (var index in DenonMC6000MK2.connectedLeds) {
        DenonMC6000MK2.connectedLeds[index].reset();
    }
    DenonMC6000MK2.connectedLeds = [];
};


////////////////////////////////////////////////////////////////////////
// Controls                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.Control = function(group, ctrl, func) {
    this.group = group;
    this.ctrl = ctrl;
    this.func = func;
    this.isConnected = false;
};

DenonMC6000MK2.Control.prototype.connect = function() {
    if (this.isConnected) {
        DenonMC6000MK2.logWarning("Control is already connected: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
        return true;
    }
    if (engine.connectControl(this.group, this.ctrl, this.func)) {
        this.isConnected = true;
        this.trigger();
    } else {
        DenonMC6000MK2.logError("Failed to connect control: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
    }
    return this.isConnected;
};

DenonMC6000MK2.Control.prototype.disconnect = function() {
    if (this.isConnected) {
        if (engine.connectControl(this.group, this.ctrl, this.func, true)) {
            this.isConnected = false;
        } else {
            DenonMC6000MK2.logError("Failed to disconnect control: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
        }
    } else {
        DenonMC6000MK2.logWarning("Control is not connected: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
    }
};

DenonMC6000MK2.Control.prototype.trigger = function() {
    engine.trigger(this.group, this.ctrl);
};

DenonMC6000MK2.connectedControls = [];

DenonMC6000MK2.connectControl = function(group, ctrl, func) {
    var control = new DenonMC6000MK2.Control(group, ctrl, func);
    if (control.connect()) {
        DenonMC6000MK2.connectedControls.push(control);
        return control;
    } else {
        return undefined;
    }
};

DenonMC6000MK2.disconnectControls = function() {
    for (var index in DenonMC6000MK2.connectedControls) {
        DenonMC6000MK2.connectedControls[index].disconnect();
    }
    DenonMC6000MK2.connectedControls = [];
};


////////////////////////////////////////////////////////////////////////
// Custom components                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.LoadButton = function() {
    components.Button.call(this, {
        type: components.Button.toggle,
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
        },
        shift: function() {
            this.inKey = "eject";
        },
        input: function(_channel, _control, value, _status, _group) {
            this.inSetParameter(this.inValueScale(value));
            // Smart PFL control
            if (this.inKey === "LoadSelectedTrack" && !engine.getValue(this.group, "play")) {
                for (var deckIndex in DenonMC6000MK2.allDecks) {
                    var deck = DenonMC6000MK2.allDecks[deckIndex];
                    var deckGroup = deck.currentDeck;
                    engine.setValue(deckGroup, "pfl", this.group === deckGroup);
                }
            }
        },
    });
};

DenonMC6000MK2.LoadButton.prototype = Object.create(components.Button.prototype);
DenonMC6000MK2.LoadButton.prototype.constructor = DenonMC6000MK2.LoadButton;


DenonMC6000MK2.Deck = function(number, channel) {
    DenonMC6000MK2.logDebug("Creating deck: " + number);

    components.Deck.call(this, number);

    this.loadButton = new DenonMC6000MK2.LoadButton();

    this.cueButton = new components.CueButton([0xB0 + channel, 0x26]);
    this.playButton = new components.PlayButton([0xB0 + channel, 0x27]);
    this.syncButton = new components.SyncButton([0xB0 + channel, 0x09]);

    this.hotcueButtons = [];
    for (var i = 1; i <= 4; i++) {
        this.hotcueButtons[i] = new components.HotcueButton({
            midi: [0xB0 + channel, 0x11 + 2 * (i - 1)],
            number: i,
        });
    }

    // Set the group properties of the above Components and connect their output callback functions
    var thisDeck = this;
    this.reconnectComponents(function(component) {
        if (component.group === undefined) {
            component.group = thisDeck.currentDeck;
        }
    });
};

DenonMC6000MK2.Deck.prototype = Object.create(components.Deck.prototype);
DenonMC6000MK2.Deck.prototype.constructor = DenonMC6000MK2.Deck;

DenonMC6000MK2.Deck.prototype.loopInButtonInput = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onLoopInButton(isButtonPressed);
};

DenonMC6000MK2.Deck.prototype.loopOutButtonInput = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onLoopOutButton(isButtonPressed);
};


DenonMC6000MK2.Side = function(name, effectUnitNumbers, oldSide) {
    DenonMC6000MK2.logDebug("Creating Side: " + name);

    components.ComponentContainer.call(this);

    this.name = name;

    this.decksByGroup = {};
    this.currentDeck = undefined;

    this.effectUnit = new components.EffectUnit(effectUnitNumbers);
    this.effectUnit.dryWetKnob.input = function(_channel, _control, value, _status, _group) {
        var knobDelta = DenonMC6000MK2.getKnobDelta(value);
        this.inSetParameter(this.inGetParameter() + knobDelta / DenonMC6000MK2.EFX_MIX_ENCODER_STEPS);
    };
    this.effectUnit.init();

    // TODO: Eliminate oldSide
    DenonMC6000MK2.sidesByGroup[oldSide.efxUnit.group] = this;
    for (var samplerIndex in oldSide.samplers) {
        var sampler = oldSide.samplers[samplerIndex];
        DenonMC6000MK2.sidesByGroup[sampler.group] = this;
    }
};

DenonMC6000MK2.Side.prototype = Object.create(components.ComponentContainer.prototype);
DenonMC6000MK2.Side.prototype.constructor = DenonMC6000MK2.Side;

DenonMC6000MK2.Side.prototype.getShiftState = function() {
    if (DenonMC6000MK2.GLOBAL_SHIFT_STATE) {
        return DenonMC6000MK2.getShiftState();
    } else {
        return this.isShifted;
    }
};

DenonMC6000MK2.Side.prototype.connectDeck = function(deck) {
    if (deck instanceof DenonMC6000MK2.Deck === false) {
        DenonMC6000MK2.logError("Type mismatch: " + deck);
        return;
    }
    var group = deck.currentDeck;
    DenonMC6000MK2.logDebug("Connecting deck " + group + " to " + this.name + " side");
    DenonMC6000MK2.sidesByGroup[group] = this;
    this.decksByGroup[group] = deck;
};

DenonMC6000MK2.Side.prototype.shiftButtonInput = function(channel, control, value, status) {
    var isButtonPressed = components.Button.prototype.isPress(channel, control, value, status);
    if (isButtonPressed) {
        this.shift();
    } else {
        this.unshift();
    }
    for (var deckGroup in this.decksByGroup) {
        var deck = this.decksByGroup[deckGroup];
        if (isButtonPressed) {
            deck.shift();
        } else {
            deck.unshift();
        }
    }
};


////////////////////////////////////////////////////////////////////////
// Samplers                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.samplerCount = 0;

DenonMC6000MK2.samplersByGroup = {};

DenonMC6000MK2.getSamplerByGroup = function(group) {
    var sampler = DenonMC6000MK2.samplersByGroup[group];
    if (undefined === sampler) {
        DenonMC6000MK2.logError("No sampler found for " + group);
    }
    return sampler;
};

DenonMC6000MK2.Sampler = function(midiChannel, midiLedValue, midiDimmerLedValue) {
    this.number = ++DenonMC6000MK2.samplerCount;
    this.group = "[Sampler" + this.number + "]";
    DenonMC6000MK2.samplersByGroup[this.group] = this;
    this.midiChannel = midiChannel;
    this.midiLedValue = midiLedValue;
    this.midiDimmerLedValue = midiDimmerLedValue;
};

DenonMC6000MK2.Sampler.prototype.isTrackLoaded = function() {
    return DenonMC6000MK2.isTrackLoaded(engine.getValue(this.group, "track_samples"));
};

DenonMC6000MK2.Sampler.prototype.loadSelectedTrack = function() {
    engine.setValue(this.group, "LoadSelectedTrack", true);
};

DenonMC6000MK2.Sampler.prototype.loadSelectedTrackAndPlay = function() {
    engine.setValue(this.group, "LoadSelectedTrackAndPlay", true);
};

DenonMC6000MK2.Sampler.prototype.isPlaying = function() {
    return engine.getValue(this.group, "play");
};

DenonMC6000MK2.Sampler.prototype.onButton = function(isButtonPressed) {
    var side;
    switch (DenonMC6000MK2.DEFAULT_SAMPLER_MODE) {
    case DenonMC6000MK2.SAMPLER_MODE.TRIGGER:
        if (isButtonPressed) {
            if (this.isTrackLoaded()) {
                side = DenonMC6000MK2.getSideByGroup(this.group);
                if (side.getShiftState()) {
                    engine.setValue(this.group, "start_stop", true);
                } else {
                    engine.setValue(this.group, "start_play", true);
                }
            } else {
                this.loadSelectedTrack();
            }
        }
        break;
    case DenonMC6000MK2.SAMPLER_MODE.HOLD:
        if (this.isTrackLoaded()) {
            side = DenonMC6000MK2.getSideByGroup(this.group);
            if (isButtonPressed) {
                if (side.getShiftState()) {
                    engine.setValue(this.group, "eject", true);
                } else {
                    engine.setValue(this.group, "start_play", true);
                }
            } else {
                // continue playing if shift is pressed when
                // releasing the pressed button
                if (!side.getShiftState()) {
                    engine.setValue(this.group, "start_stop", true);
                }
            }
        } else {
            if (isButtonPressed) {
                this.loadSelectedTrack();
            }
        }
        break;
    }
};

DenonMC6000MK2.Sampler.prototype.connectLeds = function() {
    this.led = DenonMC6000MK2.connectTriLed(this.midiChannel, this.midiLedValue);
    this.dimmerLed = DenonMC6000MK2.connectTriLed(this.midiChannel, this.midiDimmerLedValue);
};

DenonMC6000MK2.Sampler.prototype.updateLeds = function() {
    if (this.isPlaying()) {
        this.led.setStateBoolean(true);
    } else {
        this.dimmerLed.setStateBoolean(this.isTrackLoaded());
    }
};

DenonMC6000MK2.Sampler.prototype.connectControls = function() {
    DenonMC6000MK2.connectControl(this.group, "play", DenonMC6000MK2.ctrlSampler);
    DenonMC6000MK2.connectControl(this.group, "track_samples", DenonMC6000MK2.ctrlSampler);
};


////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

/* Constructor */

DenonMC6000MK2.OldDeck = function(number, midiChannel) {
    this.number = number;
    this.group = "[Channel" + number + "]";
    this.filterGroup = "[QuickEffectRack1_" + this.group + "_Effect1]";
    this.midiChannel = midiChannel;
    this.jogTouchState = false;
    DenonMC6000MK2.oldDecksByGroup[this.group] = this;
    this.rateDirBackup = this.getValue("rate_dir");
    this.setValue("rate_dir", -1);
    this.vinylMode = undefined;
    this.syncMode = undefined;
};

/* Shift */

DenonMC6000MK2.OldDeck.prototype.getShiftState = function() {
    var side = DenonMC6000MK2.getSideByGroup(this.group);
    return side.getShiftState();
};

/* Values & Parameters */

DenonMC6000MK2.OldDeck.prototype.getValue = function(key) {
    return engine.getValue(this.group, key);
};

DenonMC6000MK2.OldDeck.prototype.setValue = function(key, value) {
    engine.setValue(this.group, key, value);
};

DenonMC6000MK2.OldDeck.prototype.toggleValue = function(key) {
    this.setValue(key, !this.getValue(key));
};

DenonMC6000MK2.OldDeck.prototype.setParameter = function(key, param) {
    engine.setParameter(this.group, key, param);
};

DenonMC6000MK2.OldDeck.prototype.triggerValue = function(key) {
    engine.trigger(this.group, key);
};

/* Xfader */

DenonMC6000MK2.OldDeck.prototype.assignXfaderLeft = function() {
    this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_LEFT);
};

DenonMC6000MK2.OldDeck.prototype.assignXfaderCenter = function() {
    this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_CENTER);
};

DenonMC6000MK2.OldDeck.prototype.assignXfaderRight = function() {
    this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_RIGHT);
};

/* Tracks */

DenonMC6000MK2.OldDeck.prototype.loadSelectedTrack = function() {
    this.setValue("LoadSelectedTrack", true);
    if (!this.isPlaying()) {
        this.setCueMixSolo(); // just for convenience ;)
    }
};

DenonMC6000MK2.OldDeck.prototype.loadSelectedTrackAndPlay = function() {
    this.setValue("LoadSelectedTrackAndPlay", true);
};

DenonMC6000MK2.OldDeck.prototype.unloadTrack = function() {
    this.setValue("eject", true);
};

/* Key Lock Mode */

DenonMC6000MK2.OldDeck.prototype.onKeyLockButton = function(isButtonPressed) {
    if (isButtonPressed) {
        this.toggleValue("keylock");
    }
};

DenonMC6000MK2.OldDeck.prototype.enableKeyLock = function() {
    this.setValue("keylock", true);
};

DenonMC6000MK2.OldDeck.prototype.disableKeyLock = function() {
    this.setValue("keylock", false);
};

DenonMC6000MK2.OldDeck.prototype.onKeyLockValue = function(value) {
    this.keyLockLed.setStateBoolean(value);
};

/* Key Control */

DenonMC6000MK2.OldDeck.prototype.resetKey = function() {
    this.setValue("reset_key", true);
};

/* Sync Mode */

DenonMC6000MK2.OldDeck.prototype.disableSyncMode = function() {
    this.setValue("sync_mode", DenonMC6000MK2.MIXXX_SYNC_NONE);
};

/* Cue Mix */

DenonMC6000MK2.OldDeck.prototype.setCueMixSolo = function() {
    for (var index in DenonMC6000MK2.oldDecksByGroup) {
        var deck = DenonMC6000MK2.oldDecksByGroup[index];
        deck.setValue("pfl", this === deck);
    }
};

DenonMC6000MK2.OldDeck.prototype.onCueMixButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            this.setCueMixSolo();
        } else {
            this.toggleValue("pfl");
        }
    }
};

DenonMC6000MK2.OldDeck.prototype.updateCueMixValue = function(pflValue, isTrackLoaded) {
    if (pflValue) {
        this.cueMixLed.setStateBoolean(pflValue);
    } else {
        this.cueMixDimmerLed.setStateBoolean(isTrackLoaded);
    }
};

DenonMC6000MK2.OldDeck.prototype.onCueMixValue = function(pflValue) {
    this.updateCueMixValue(pflValue, this.isTrackLoaded());
};

/* Track Load */

DenonMC6000MK2.isTrackLoaded = function(trackSamples) {
    return 0 < trackSamples;
};

DenonMC6000MK2.OldDeck.prototype.isTrackLoaded = function() {
    return DenonMC6000MK2.isTrackLoaded(this.getValue("track_samples"));
};

DenonMC6000MK2.OldDeck.prototype.onTrackSamplesValue = function(value) {
    this.updateCueMixValue(this.getValue("pfl"), DenonMC6000MK2.isTrackLoaded(value));
};

/* Cue & Play */

DenonMC6000MK2.OldDeck.prototype.isPlaying = function() {
    return this.getValue("play");
};

/* Pitch Bend / Track Search */

DenonMC6000MK2.OldDeck.prototype.onBendPlusButton = function(isButtonPressed) {
    if (this.isPlaying()) {
        this.setValue("fwd", false);
        if (this.getShiftState()) {
            this.setValue("rate_temp_up_small", isButtonPressed);
        } else {
            this.setValue("rate_temp_up", isButtonPressed);
        }
    } else {
        this.setValue("fwd", isButtonPressed);
    }
};

DenonMC6000MK2.OldDeck.prototype.onBendMinusButton = function(isButtonPressed) {
    if (this.isPlaying()) {
        this.setValue("back", false);
        if (this.getShiftState()) {
            this.setValue("rate_temp_down_small", isButtonPressed);
        } else {
            this.setValue("rate_temp_down", isButtonPressed);
        }
    } else {
        this.setValue("back", isButtonPressed);
    }
};


/* Censor / Slip Mode */

DenonMC6000MK2.OldDeck.prototype.onCensorButton = function(buttonPressed) {
    if (this.getShiftState()) {
        // Please note that reverseroll seems to have side effects on
        // slip_enabled so better leave it alone while shift is pressed!
        if (buttonPressed) {
            this.toggleValue("slip_enabled");
        }
    } else {
        this.setValue("reverseroll", buttonPressed);
    }
};

DenonMC6000MK2.OldDeck.prototype.onSlipModeValue = function(value) {
    this.slipModeLed.setStateBoolean(value);
};

/* Vinyl Mode (Scratching) */

DenonMC6000MK2.OldDeck.prototype.onVinylModeValue = function() {
    this.vinylModeLed.setStateBoolean(this.vinylMode);
};

DenonMC6000MK2.OldDeck.prototype.enableScratching = function() {
};

DenonMC6000MK2.OldDeck.prototype.disableScratching = function() {
};

DenonMC6000MK2.OldDeck.prototype.updateVinylMode = function() {
    if (this.vinylMode && this.jogTouchState) {
        engine.scratchEnable(this.number,
            DenonMC6000MK2.JOG_RESOLUTION,
            DenonMC6000MK2.JOG_SCRATCH_RPM,
            DenonMC6000MK2.JOG_SCRATCH_ALPHA,
            DenonMC6000MK2.JOG_SCRATCH_BETA,
            DenonMC6000MK2.JOG_SCRATCH_RAMP);
    } else {
        engine.scratchDisable(this.number,
            DenonMC6000MK2.JOG_SCRATCH_RAMP);
    }
    this.onVinylModeValue();
};

DenonMC6000MK2.OldDeck.prototype.setVinylMode = function(vinylMode) {
    this.vinylMode = vinylMode;
    this.updateVinylMode();
};

DenonMC6000MK2.OldDeck.prototype.toggleVinylMode = function() {
    this.setVinylMode(!this.vinylMode);
};

DenonMC6000MK2.OldDeck.prototype.enableVinylMode = function() {
    this.setVinylMode(true);
};

DenonMC6000MK2.OldDeck.prototype.disableVinylMode = function() {
    this.setVinylMode(false);
};

DenonMC6000MK2.OldDeck.prototype.onVinylButton = function(_isButtonPressed) {
    this.toggleVinylMode();
};

/* Jog Wheel */

DenonMC6000MK2.OldDeck.prototype.touchJog = function(isJogTouched) {
    this.jogTouchState = isJogTouched;
    this.updateVinylMode();
};

DenonMC6000MK2.OldDeck.prototype.spinJog = function(jogDelta) {
    if (this.getShiftState() && this.jogTouchState && !this.isPlaying()) {
        // fast track seek (strip search)
        var playPos = engine.getValue(this.group, "playposition");
        if (undefined !== playPos) {
            var seekPos = playPos + (jogDelta / (DenonMC6000MK2.JOG_RESOLUTION * DenonMC6000MK2.JOG_SEEK_REVOLUTIONS));
            this.setValue("playposition", Math.max(0.0, Math.min(1.0, seekPos)));
        }
    } else {
        if (engine.isScratching(this.number)) {
            engine.scratchTick(this.number, jogDelta);
        } else {
            var normalizedDelta = jogDelta / DenonMC6000MK2.MIDI_JOG_DELTA_RANGE;
            var scaledDelta;
            var jogExponent;
            if (this.isPlaying()) {
                // bending
                scaledDelta = normalizedDelta / DenonMC6000MK2.JOG_SPIN_PLAY_PEAK;
                jogExponent = DenonMC6000MK2.JOG_SPIN_PLAY_EXPONENT;
            } else {
                // cueing
                scaledDelta = normalizedDelta / DenonMC6000MK2.JOG_SPIN_CUE_PEAK;
                jogExponent = DenonMC6000MK2.JOG_SPIN_CUE_EXPONENT;
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
            var jogValue = DenonMC6000MK2.MIXXX_JOG_RANGE * scaledDeltaPow;
            this.setValue("jog", jogValue);
        }
    }
};

/* Filter */

DenonMC6000MK2.OldDeck.prototype.applyFilter = function() {
    var side = DenonMC6000MK2.getOldSideByGroup(this.group);
    engine.setValue(this.filterGroup, "enabled", side.filterEnabled);
    engine.setParameter(this.filterGroup, "meta", side.filterParam);
};

/* Loops */

DenonMC6000MK2.OldDeck.prototype.hasLoopStart = function() {
    return DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED !== this.getValue("loop_start_position");
};

DenonMC6000MK2.OldDeck.prototype.hasLoopEnd = function() {
    return DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED !== this.getValue("loop_end_position");
};

DenonMC6000MK2.OldDeck.prototype.hasLoop = function() {
    return this.hasLoopStart() && this.hasLoopEnd() && this.getValue("loop_start_position") < this.getValue("loop_end_position");
};

DenonMC6000MK2.OldDeck.prototype.deleteLoopStart = function() {
    this.setValue("loop_in", false);
    this.setValue("loop_start_position", DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED);
};

DenonMC6000MK2.OldDeck.prototype.deleteLoopEnd = function() {
    this.setValue("loop_out", false);
    this.setValue("loop_end_position", DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED);
};

DenonMC6000MK2.OldDeck.prototype.deleteLoop = function() {
    // loop end has to be deleted before loop start
    this.deleteLoopEnd();
    this.deleteLoopStart();
};

DenonMC6000MK2.OldDeck.prototype.onAutoLoopButton = function(isButtonPressed) {
    if (this.hasLoop() || this.getValue("loop_enabled")) {
        if (this.getShiftState()) {
            // Delete loop
            this.setValue("reloop_toggle", false);
            if (isButtonPressed) {
                this.deleteLoop();
            }
        } else {
            // Reloop
            this.setValue("reloop_toggle", isButtonPressed);
        }
    } else {
        // Autoloop
        this.setValue("reloop_toggle", false);
        this.deleteLoop(); // cleanup
        if (isButtonPressed) {
            if (this.getShiftState()) {
                this.setValue("beatlooproll_activate", true);
            } else {
                this.setValue("beatloop_activate", true);
            }
        }
    }
};

DenonMC6000MK2.OldDeck.prototype.onLoopInButton = function(isButtonPressed) {
    if (this.getShiftState()) {
        this.setValue("loop_in", false);
        if (isButtonPressed) {
            this.deleteLoop(); // whole loop, i.e. both start and end
        }
    } else {
        this.setValue("loop_in", isButtonPressed);
    }
};

DenonMC6000MK2.OldDeck.prototype.onLoopOutButton = function(isButtonPressed) {
    if (this.getShiftState()) {
        this.setValue("loop_out", false);
        if (isButtonPressed) {
            this.deleteLoopEnd(); // only end
        }
    } else {
        this.setValue("loop_out", isButtonPressed);
    }
};

DenonMC6000MK2.OldDeck.prototype.onLoopCutMinusButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            var stepSize = this.getValue("beatloop_size");
            if (stepSize <= 0.0) {
                stepSize = 1;
            }
            this.setValue("loop_move_" + stepSize + "_backward", true);
        } else {
            this.setValue("loop_halve", true);
        }
    }
};

DenonMC6000MK2.OldDeck.prototype.onLoopCutPlusButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            var stepSize = this.getValue("beatloop_size");
            if (stepSize <= 0.0) {
                stepSize = 1;
            }
            this.setValue("loop_move_" + stepSize + "_forward", true);
        } else {
            this.setValue("loop_double", true);
        }
    }
};

DenonMC6000MK2.OldDeck.prototype.updateLoopLeds = function(_value) {
    if (this.getValue("loop_enabled")) {
        this.loopInLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
        this.loopOutLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
        this.autoLoopLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
    } else {
        this.loopInLed.setStateBoolean(this.hasLoopStart());
        this.loopOutLed.setStateBoolean(this.hasLoopEnd());
        this.autoLoopDimmerLed.setStateBoolean(this.hasLoop());
    }
};

/* Deck LEDs */

DenonMC6000MK2.OldDeck.prototype.connectLed = function(midiValue) {
    return DenonMC6000MK2.connectLed(this.midiChannel, midiValue);
};

DenonMC6000MK2.OldDeck.prototype.connectTriLed = function(midiValue) {
    return DenonMC6000MK2.connectTriLed(this.midiChannel, midiValue);
};

/* Startup */

DenonMC6000MK2.OldDeck.prototype.connectLeds = function() {
    this.vinylModeLed = this.connectTriLed(0x06);
    this.keyLockLed = this.connectTriLed(0x08);
    this.loopInLed = this.connectTriLed(0x24);
    this.loopInDimmerLed = this.connectTriLed(0x3E);
    this.loopOutLed = this.connectTriLed(0x40);
    this.loopOutDimmerLed = this.connectTriLed(0x2A);
    this.autoLoopLed = this.connectTriLed(0x2B);
    this.autoLoopDimmerLed = this.connectTriLed(0x53);
    this.slipModeLed = this.connectTriLed(0x64);
};

DenonMC6000MK2.OldDeck.prototype.connectControl = function(ctrl, func) {
    return DenonMC6000MK2.connectControl(this.group, ctrl, func);
};

DenonMC6000MK2.OldDeck.prototype.connectControls = function() {
    this.connectControl("keylock", DenonMC6000MK2.ctrlKeyLock);
    this.connectControl("pfl", DenonMC6000MK2.ctrlCueMix);
    this.connectControl("track_samples", DenonMC6000MK2.ctrlTrackSamples);
    this.connectControl("slip_enabled", DenonMC6000MK2.ctrlSlipModeValue);
    this.connectControl("loop_enabled", DenonMC6000MK2.ctrlLoopEnabled);
    this.connectControl("loop_start_position", DenonMC6000MK2.ctrlLoopStartPosition);
    this.connectControl("loop_end_position", DenonMC6000MK2.ctrlLoopEndPosition);
    DenonMC6000MK2.oldLeftSide.efxUnit.connectDeckControls(this, DenonMC6000MK2.oldLeftSide.efxUnit.ctrlDeck);
    DenonMC6000MK2.oldRightSide.efxUnit.connectDeckControls(this, DenonMC6000MK2.oldRightSide.efxUnit.ctrlDeck);
    // default settings
    this.enableKeyLock();
    this.enableVinylMode();
    this.disableSyncMode();
};

/* Shutdown */

DenonMC6000MK2.OldDeck.prototype.restoreValues = function() {
    this.setValue("rate_dir", this.rateDirBackup);
};


////////////////////////////////////////////////////////////////////////
// Efx Params                                                         //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.efxParamsByGroup = {};

DenonMC6000MK2.ctrlEfxParamEnabled = function(value, group, _control) {
    var param = DenonMC6000MK2.efxParamsByGroup[group];
    if (param !== undefined) {
        param.onLed.setStateBoolean(value !== 0.0);
    }
};

DenonMC6000MK2.EfxParam = function(group) {
    DenonMC6000MK2.efxParamsByGroup[group] = this;
    DenonMC6000MK2.connectControl(group, "enabled", DenonMC6000MK2.ctrlEfxParamEnabled);
};


////////////////////////////////////////////////////////////////////////
// Efx Units                                                          //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.EfxUnit = function(unit) {
    var groupName = DenonMC6000MK2.EFX_RACK + "_" + unit;
    this.group = "[" + groupName + "]";
    this.params = [];
    for (var i = 1; i <= 3; ++i) {
        this.params[i] = new DenonMC6000MK2.EfxParam("[" + groupName + "_Effect" + i + "]");
    }
};

DenonMC6000MK2.EfxUnit.prototype.getShiftState = function() {
    var side = DenonMC6000MK2.getSideByGroup(this.group);
    return side.getShiftState();
};

DenonMC6000MK2.EfxUnit.prototype.isEnabled = function() {
    return engine.getValue(this.group, "enabled");
};

DenonMC6000MK2.EfxUnit.prototype.onEnableButton = function(isButtonPressed) {
    if (isButtonPressed) {
        script.toggleControl(this.group, "enabled");
    }
};

DenonMC6000MK2.EfxUnit.prototype.getDeckAssignKey = function(deck) {
    return "group_" + deck.group + "_enable";
};

DenonMC6000MK2.EfxUnit.prototype.connectDeckControls = function(deck, assignCB) {
    DenonMC6000MK2.connectControl(this.group, this.getDeckAssignKey(deck), assignCB);
};

DenonMC6000MK2.EfxUnit.prototype.isDeckAssigned = function(deck) {
    return engine.getValue(this.group, this.getDeckAssignKey(deck));
};

DenonMC6000MK2.EfxUnit.prototype.isActiveDeckAssigned = function() {
    var side = DenonMC6000MK2.getOldSideByGroup(this.group);
    return engine.getValue(this.group, this.getDeckAssignKey(side.activeDeck));
};

DenonMC6000MK2.EfxUnit.prototype.isAnyDeckAssigned = function() {
    for (var deckGroup in DenonMC6000MK2.oldDecksByGroup) {
        var deck = DenonMC6000MK2.oldDecksByGroup[deckGroup];
        if (this.isDeckAssigned(deck)) {
            return true;
        }
    }
    return false;
};

DenonMC6000MK2.EfxUnit.prototype.assignDeckToggle = function(deck) {
    script.toggleControl(this.group, this.getDeckAssignKey(deck));
};

DenonMC6000MK2.EfxUnit.prototype.assignDeckExlusively = function(deck) {
    for (var deckGroup in DenonMC6000MK2.oldDecksByGroup) {
        var varDeck = DenonMC6000MK2.oldDecksByGroup[deckGroup];
        engine.setValue(this.group, this.getDeckAssignKey(varDeck), varDeck === deck);
    }
};

DenonMC6000MK2.EfxUnit.prototype.onEnabled = function() {
    this.tapLed.setTriState(this.isEnabled() ? DenonMC6000MK2.TRI_LED_ON : DenonMC6000MK2.TRI_LED_OFF);
};

DenonMC6000MK2.EfxUnit.prototype.onDeckButton = function(deckGroup, isButtonPressed) {
    if (isButtonPressed) {
        var deck = DenonMC6000MK2.getOldDeckByGroup(deckGroup);
        if (this.getShiftState()) {
            this.assignDeckExlusively(deck);
        } else {
            this.assignDeckToggle(deck);
        }
    }
};

////////////////////////////////////////////////////////////////////////
// Sides                                                              //
////////////////////////////////////////////////////////////////////////

/* Constructor */

DenonMC6000MK2.OldSide = function(decks, efxUnit, samplerMidiChannel) {
    this.decksByGroup = {};
    for (var deckIndex in decks) {
        var deck = decks[deckIndex];
        DenonMC6000MK2.logInfo("Adding deck " + deck.group + " to side");
        DenonMC6000MK2.oldSidesByGroup[deck.group] = this;
        this.decksByGroup[deck.group] = deck;
    }
    this.activeDeck = decks[0];
    this.efxUnit = new DenonMC6000MK2.EfxUnit(efxUnit);
    DenonMC6000MK2.oldSidesByGroup[this.efxUnit.group] = this;
    this.samplers = [];
    this.samplers[1] = new DenonMC6000MK2.Sampler(samplerMidiChannel, 0x19, 0x1A);
    this.samplers[2] = new DenonMC6000MK2.Sampler(samplerMidiChannel, 0x1B, 0x1C);
    this.samplers[3] = new DenonMC6000MK2.Sampler(samplerMidiChannel, 0x1D, 0x1F);
    this.samplers[4] = new DenonMC6000MK2.Sampler(samplerMidiChannel, 0x20, 0x21);
    for (var samplerIndex in this.samplers) {
        var sampler = this.samplers[samplerIndex];
        DenonMC6000MK2.oldSidesByGroup[sampler.group] = this;
    }
    this.filterLed = undefined;
};

/* Decks */

DenonMC6000MK2.OldSide.prototype.onDeckButton = function(deckGroup, isButtonPressed) {
    if (isButtonPressed) {
        this.activeDeck = this.decksByGroup[deckGroup];
    }
};


/* Startup */

DenonMC6000MK2.OldSide.prototype.connectLeds = function() {
    for (var deckGroup in this.decksByGroup) {
        var deck = this.decksByGroup[deckGroup];
        deck.connectLeds();
    }
    for (var samplerIndex in this.samplers) {
        this.samplers[samplerIndex].connectLeds();
    }
};

DenonMC6000MK2.OldSide.prototype.connectControls = function() {
    for (var deckGroup in this.decksByGroup) {
        var deck = this.decksByGroup[deckGroup];
        deck.connectControls();
    }
    for (var samplerIndex in this.samplers) {
        this.samplers[samplerIndex].connectControls();
    }
};

/* Shutdown */

DenonMC6000MK2.OldSide.prototype.restoreValues = function() {
    for (var group in this.decksByGroup) {
        var deck = this.decksByGroup[group];
        deck.restoreValues();
    }
};

// Filter

DenonMC6000MK2.OldSide.prototype.applyFilter = function() {
    for (var group in this.decksByGroup) {
        var deck = this.decksByGroup[group];
        deck.applyFilter();
    }
};

DenonMC6000MK2.OldSide.prototype.initFilter = function() {
    this.filterEnabled = true;
    this.filterParam = 0.5; // centered
    this.applyFilter();
};

DenonMC6000MK2.OldSide.prototype.toggleFilter = function() {
    this.filterEnabled = !this.filterEnabled;
    this.applyFilter();
};

DenonMC6000MK2.OldSide.prototype.onFilterButton = function(isButtonPressed) {
    if (isButtonPressed) {
        this.toggleFilter();
    }
};

DenonMC6000MK2.OldSide.prototype.onFilterMidiValue = function(value) {
    this.filterParam = script.absoluteLin(value, 0.0, 1.0);
    this.applyFilter();
};


////////////////////////////////////////////////////////////////////////
// Controller functions                                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.id = undefined;
DenonMC6000MK2.debug = undefined;
DenonMC6000MK2.group = "[Master]";

// left side
DenonMC6000MK2.oldDeck1 = new DenonMC6000MK2.OldDeck(1, DenonMC6000MK2.MIDI_CH0);
DenonMC6000MK2.oldDeck3 = new DenonMC6000MK2.OldDeck(3, DenonMC6000MK2.MIDI_CH1);
DenonMC6000MK2.oldLeftSide = new DenonMC6000MK2.OldSide(
    [DenonMC6000MK2.oldDeck1, DenonMC6000MK2.oldDeck3],
    DenonMC6000MK2.LEFT_EFX_UNIT,
    DenonMC6000MK2.MIDI_CH0);

// right side
DenonMC6000MK2.oldDeck2 = new DenonMC6000MK2.OldDeck(2, DenonMC6000MK2.MIDI_CH2);
DenonMC6000MK2.oldDeck4 = new DenonMC6000MK2.OldDeck(4, DenonMC6000MK2.MIDI_CH3);
DenonMC6000MK2.oldRightSide = new DenonMC6000MK2.OldSide(
    [DenonMC6000MK2.oldDeck2, DenonMC6000MK2.oldDeck4],
    DenonMC6000MK2.RIGHT_EFX_UNIT,
    DenonMC6000MK2.MIDI_CH2);

DenonMC6000MK2.oldSides = [DenonMC6000MK2.oldLeftSide, DenonMC6000MK2.oldRightSide];

DenonMC6000MK2.getShiftState = function() {
    // Direct access to prevent recursion
    return DenonMC6000MK2.leftSide.isShifted || DenonMC6000MK2.rightSide.isShifted;
};

DenonMC6000MK2.getValue = function(key) {
    return engine.getValue(DenonMC6000MK2.group, key);
};

DenonMC6000MK2.setValue = function(key, value) {
    engine.setValue(DenonMC6000MK2.group, key, value);
};

DenonMC6000MK2.getJogDeltaValue = function(value) {
    if (0x00 === value) {
        return 0x00;
    } else {
        return value - DenonMC6000MK2.MIDI_JOG_DELTA_BIAS;
    }
};

DenonMC6000MK2.initValues = function() {
    DenonMC6000MK2.backupSampleRate = engine.getValue(DenonMC6000MK2.group, "samplerate");
    if (DenonMC6000MK2.backupSampleRate !== DenonMC6000MK2.SAMPLE_RATE) {
        DenonMC6000MK2.logInfo(
            "Adjusting sample rate: " +
            DenonMC6000MK2.backupSampleRate +
            " -> " +
            DenonMC6000MK2.SAMPLE_RATE);
        DenonMC6000MK2.setValue("samplerate", DenonMC6000MK2.SAMPLE_RATE);
    }
    DenonMC6000MK2.backupNumDecks = DenonMC6000MK2.getValue("num_decks");
    if (DenonMC6000MK2.backupNumDecks !== DenonMC6000MK2.DECK_COUNT) {
        DenonMC6000MK2.logInfo(
            "Adjusting number of decks: " +
            DenonMC6000MK2.backupNumDecks +
            " -> " +
            DenonMC6000MK2.DECK_COUNT);
        DenonMC6000MK2.setValue("num_decks", DenonMC6000MK2.DECK_COUNT);
    }
    DenonMC6000MK2.backupNumSamplers = DenonMC6000MK2.getValue("num_samplers");
    var numSamplers = DenonMC6000MK2.SIDE_COUNT * DenonMC6000MK2.SAMPLER_COUNT_PER_SIDE;
    if (DenonMC6000MK2.backupNumSamplers !== numSamplers) {
        DenonMC6000MK2.logInfo(
            "Adjusting number of samplers: " +
            DenonMC6000MK2.backupNumSamplers +
            " -> " +
            numSamplers);
        DenonMC6000MK2.setValue("num_samplers", numSamplers);
    }
};

DenonMC6000MK2.connectLeds = function() {
    DenonMC6000MK2.oldDeck1.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x45);
    DenonMC6000MK2.oldDeck1.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x46);
    DenonMC6000MK2.oldDeck1.leftEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x56);
    DenonMC6000MK2.oldDeck1.rightEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x5A);
    DenonMC6000MK2.oldDeck2.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x4B);
    DenonMC6000MK2.oldDeck2.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x4C);
    DenonMC6000MK2.oldDeck2.leftEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x57);
    DenonMC6000MK2.oldDeck2.rightEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x5B);
    DenonMC6000MK2.oldDeck3.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x51);
    DenonMC6000MK2.oldDeck3.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x52);
    DenonMC6000MK2.oldDeck3.leftEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x54);
    DenonMC6000MK2.oldDeck3.rightEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x58);
    DenonMC6000MK2.oldDeck4.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x57);
    DenonMC6000MK2.oldDeck4.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x58);
    DenonMC6000MK2.oldDeck4.leftEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x55);
    DenonMC6000MK2.oldDeck4.rightEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x59);
    DenonMC6000MK2.oldLeftSide.efxUnit.params[1].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x5C);
    DenonMC6000MK2.oldLeftSide.efxUnit.params[2].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x5D);
    DenonMC6000MK2.oldLeftSide.efxUnit.params[3].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x5E);
    DenonMC6000MK2.oldLeftSide.efxUnit.tapLed = DenonMC6000MK2.connectTriLed(0x00, 0x5F);
    DenonMC6000MK2.oldLeftSide.filterLed = DenonMC6000MK2.connectLed(0x00, 0x65);
    DenonMC6000MK2.oldRightSide.efxUnit.params[1].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x60);
    DenonMC6000MK2.oldRightSide.efxUnit.params[2].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x61);
    DenonMC6000MK2.oldRightSide.efxUnit.params[3].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x62);
    DenonMC6000MK2.oldRightSide.efxUnit.tapLed = DenonMC6000MK2.connectTriLed(0x00, 0x63);
    DenonMC6000MK2.oldRightSide.filterLed = DenonMC6000MK2.connectLed(0x00, 0x66);
    for (var index in DenonMC6000MK2.oldSides) {
        var oldSide = DenonMC6000MK2.oldSides[index];
        oldSide.connectLeds();
    }
};

DenonMC6000MK2.connectControls = function() {
    for (var index in DenonMC6000MK2.oldSides) {
        var oldSide = DenonMC6000MK2.oldSides[index];
        oldSide.connectControls();
        DenonMC6000MK2.connectControl(oldSide.efxUnit.group, "enabled", oldSide.efxUnit.ctrlEnabled);
        for (var deckGroup in oldSide.decksByGroup) {
            var deck = oldSide.decksByGroup[deckGroup];
            DenonMC6000MK2.connectControl(deck.filterGroup, "enabled", oldSide.ctrlFilterEnabled);
        }
    }
};

DenonMC6000MK2.restoreValues = function() {
    for (var index in DenonMC6000MK2.oldSides) {
        var oldSide = DenonMC6000MK2.oldSides[index];
        oldSide.restoreValues();
    }
    var numSamplers = DenonMC6000MK2.getValue("num_samplers");
    if (numSamplers !== DenonMC6000MK2.backupNumSamplers) {
        DenonMC6000MK2.logInfo(
            "Restoring number of samplers: " +
            numSamplers +
            " -> " +
            DenonMC6000MK2.backupNumSamplers);
        DenonMC6000MK2.setValue("num_samplers", DenonMC6000MK2.backupNumSamplers);
    }
    var numDecks = DenonMC6000MK2.getValue("num_decks");
    if (numDecks !== DenonMC6000MK2.backupNumDecks) {
        DenonMC6000MK2.logInfo(
            "Restoring number of decks: " +
            numDecks +
            " -> " +
            DenonMC6000MK2.backupNumDecks);
        DenonMC6000MK2.setValue("num_decks", DenonMC6000MK2.backupNumDecks);
    }
    var sampleRate = DenonMC6000MK2.getValue("samplerate");
    if (sampleRate !== DenonMC6000MK2.backupSampleRate) {
        DenonMC6000MK2.logInfo(
            "Restoring sample rate: " +
            sampleRate +
            " -> " +
            DenonMC6000MK2.backupSampleRate);
        DenonMC6000MK2.setValue("samplerate", DenonMC6000MK2.backupSampleRate);
    }
};


////////////////////////////////////////////////////////////////////////
// MIDI callback functions without a group                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.recvTrackSelectKnob = function(_channel, _control, value, _status) {
    var knobDelta = DenonMC6000MK2.getKnobDelta(value);
    var ctrlName = DenonMC6000MK2.getShiftState() ? "ScrollVertical" : "MoveVertical";
    engine.setValue("[Library]", ctrlName, knobDelta);
};

DenonMC6000MK2.recvTrackSelectButton = function(_channel, _control, value, _status) {
    var buttonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (!buttonPressed) {
        return;
    }
    var ctrlName = DenonMC6000MK2.getShiftState() ? "MoveFocusBackward" : "GoToItem";
    engine.setValue("[Library]", ctrlName, true);
};

DenonMC6000MK2.recvBackButton = function(_channel, _control, value, _status) {
    var buttonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (!buttonPressed) {
        return;
    }
    var ctrlName = DenonMC6000MK2.getShiftState() ? "MoveFocusBackward" : "ScrollUp";
    engine.setValue("[Library]", ctrlName, true);
};

DenonMC6000MK2.recvFwdButton = function(_channel, _control, value, _status) {
    var buttonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (!buttonPressed) {
        return;
    }
    var ctrlName = DenonMC6000MK2.getShiftState() ? "MoveFocusForward" : "ScrollDown";
    engine.setValue("[Library]", ctrlName, true);
};

DenonMC6000MK2.recvXfaderContourKnob = function(_channel, _control, value, _status) {
    script.crossfaderCurve(value);
};


////////////////////////////////////////////////////////////////////////
// MIDI [Channel<n>] callback functions                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.recvXfaderAssignLeftButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (!isButtonPressed) {
        return;
    }
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.assignXfaderLeft();
};

DenonMC6000MK2.recvXfaderAssignThruButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (!isButtonPressed) {
        return;
    }
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.assignXfaderCenter();
};

DenonMC6000MK2.recvXfaderAssignRightButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (!isButtonPressed) {
        return;
    }
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.assignXfaderRight();
};

DenonMC6000MK2.recvPanelButton = function(_channel, _control, value, _status, _group) {
    // TODO
    this.logDebug("Panel button: " + DenonMC6000MK2.isButtonPressed(value));
};

DenonMC6000MK2.recvListButton = function(_channel, _control, value, _status, _group) {
    // TODO
    this.logDebug("List button: " + DenonMC6000MK2.isButtonPressed(value));
};

DenonMC6000MK2.recvViewButton = function(_channel, _control, value, _status, _group) {
    // TODO
    this.logDebug("View button: " + DenonMC6000MK2.isButtonPressed(value));
};

DenonMC6000MK2.recvAreaButton = function(_channel, _control, value, _status, _group) {
    // TODO
    this.logDebug("Area button: " + DenonMC6000MK2.isButtonPressed(value));
};

DenonMC6000MK2.recvVinylButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onVinylButton(isButtonPressed);
};

DenonMC6000MK2.recvKeyLockButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onKeyLockButton(isButtonPressed);
};

DenonMC6000MK2.recvCueMixButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onCueMixButton(isButtonPressed);
};

DenonMC6000MK2.recvBendPlusButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onBendPlusButton(isButtonPressed);
};

DenonMC6000MK2.recvBendMinusButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onBendMinusButton(isButtonPressed);
};

DenonMC6000MK2.recvJogTouch = function(_channel, _control, value, _status, group) {
    var isJogTouched = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.touchJog(isJogTouched);
};

DenonMC6000MK2.recvJogTouchVinyl = function(_channel, _control, value, _status, group) {
    var isJogTouched = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.touchJog(isJogTouched);
};

DenonMC6000MK2.recvJogSpin = function(_channel, _control, value, _status, group) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    var jogDelta = DenonMC6000MK2.getJogDeltaValue(value);
    deck.spinJog(jogDelta);
};

DenonMC6000MK2.recvJogSpinVinyl = function(_channel, _control, value, _status, group) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    var jogDelta = DenonMC6000MK2.getJogDeltaValue(value);
    deck.spinJog(jogDelta);
};

DenonMC6000MK2.recvAutoLoopButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onAutoLoopButton(isButtonPressed);
};

DenonMC6000MK2.recvLoopCutMinusButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onLoopCutMinusButton(isButtonPressed);
};

DenonMC6000MK2.recvLoopCutPlusButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onLoopCutPlusButton(isButtonPressed);
};

DenonMC6000MK2.recvCensorButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onCensorButton(isButtonPressed);
};

DenonMC6000MK2.recvSamplerButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var sampler = DenonMC6000MK2.getSamplerByGroup(group);
    sampler.onButton(isButtonPressed);
};

DenonMC6000MK2.oldLeftSide.recvFilterButton = function(_channel, _control, value, _status, _group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.oldLeftSide.onFilterButton(isButtonPressed);
};

DenonMC6000MK2.oldRightSide.recvFilterButton = function(_channel, _control, value, _status, _group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.oldRightSide.onFilterButton(isButtonPressed);
};

DenonMC6000MK2.oldLeftSide.recvFilterKnob = function(_channel, _control, value, _status, _group) {
    DenonMC6000MK2.oldLeftSide.onFilterMidiValue(value);
};

DenonMC6000MK2.oldRightSide.recvFilterKnob = function(_channel, _control, value, _status, _group) {
    DenonMC6000MK2.oldRightSide.onFilterMidiValue(value);
};

DenonMC6000MK2.oldLeftSide.efxUnit.recvDeckButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.oldLeftSide.efxUnit.onDeckButton(group, isButtonPressed);
};

DenonMC6000MK2.oldRightSide.efxUnit.recvDeckButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.oldRightSide.efxUnit.onDeckButton(group, isButtonPressed);
};

DenonMC6000MK2.oldLeftSide.efxUnit.recvTapButton = function(_channel, _control, value, _status, _group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.oldLeftSide.efxUnit.onEnableButton(isButtonPressed);
};

DenonMC6000MK2.oldRightSide.efxUnit.recvTapButton = function(_channel, _control, value, _status, _group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.oldRightSide.efxUnit.onEnableButton(isButtonPressed);
};

DenonMC6000MK2.oldLeftSide.recvDeckButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.oldLeftSide.onDeckButton(group, isButtonPressed);
};

DenonMC6000MK2.oldRightSide.recvDeckButton = function(_channel, _control, value, _status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.oldRightSide.onDeckButton(group, isButtonPressed);
};


////////////////////////////////////////////////////////////////////////
// Mixxx connected controls callback functions                        //
////////////////////////////////////////////////////////////////////////

// Deck controls

DenonMC6000MK2.ctrlKeyLock = function(value, group, _control) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onKeyLockValue(value);
};

DenonMC6000MK2.ctrlSlipModeValue = function(value, group, _control) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onSlipModeValue(value);
};

DenonMC6000MK2.ctrlCueMix = function(value, group, _control) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onCueMixValue(value);
};

DenonMC6000MK2.ctrlTrackSamples = function(value, group, _control) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.onTrackSamplesValue(value);
};

// Loop controls

DenonMC6000MK2.ctrlLoopStartPosition = function(value, group, _control) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.updateLoopLeds();
};

DenonMC6000MK2.ctrlLoopEndPosition = function(value, group, _control) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.updateLoopLeds();
};

DenonMC6000MK2.ctrlLoopEnabled = function(value, group, _control) {
    var deck = DenonMC6000MK2.getOldDeckByGroup(group);
    deck.updateLoopLeds();
};

// Sampler controls

DenonMC6000MK2.ctrlSampler = function(value, group, _control) {
    var sampler = DenonMC6000MK2.getSamplerByGroup(group);
    sampler.updateLeds();
};

// Filter controls (shared between 2 decks on each side)

DenonMC6000MK2.oldLeftSide.ctrlFilterEnabled = function(value, _group, _control) {
    DenonMC6000MK2.oldLeftSide.filterLed.setStateBoolean(value);
};

DenonMC6000MK2.oldRightSide.ctrlFilterEnabled = function(value, _group, _control) {
    DenonMC6000MK2.oldRightSide.filterLed.setStateBoolean(value);
};

// Efx controls

DenonMC6000MK2.oldLeftSide.efxUnit.ctrlDeck = function(value, _group, control) {
    var deckGroup = control.substring(6, 16);
    var deck = DenonMC6000MK2.getOldDeckByGroup(deckGroup);
    deck.leftEfxLed.setStateBoolean(value);
};

DenonMC6000MK2.oldRightSide.efxUnit.ctrlDeck = function(value, _group, control) {
    var deckGroup = control.substring(6, 16);
    var deck = DenonMC6000MK2.getOldDeckByGroup(deckGroup);
    deck.rightEfxLed.setStateBoolean(value);
};

DenonMC6000MK2.oldLeftSide.efxUnit.ctrlEnabled = function(_value, _group, _control) {
    DenonMC6000MK2.oldLeftSide.efxUnit.onEnabled();
};

DenonMC6000MK2.oldRightSide.efxUnit.ctrlEnabled = function(_value, _group, _control) {
    DenonMC6000MK2.oldRightSide.efxUnit.onEnabled();
};

////////////////////////////////////////////////////////////////////////
// Mixxx Callback Functions                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.init = function(id, debug) {
    DenonMC6000MK2.id = id;
    DenonMC6000MK2.debug = debug;

    DenonMC6000MK2.logInfo("Initializing controller");

    // Customize components
    components.Button.prototype.isPress = function(_channel, _control, _value, status) {
        return (status & 0xF0) === 0x90;
    };
    components.Button.prototype.off = DenonMC6000MK2.MIDI_TRI_LED_OFF;
    components.Button.prototype.on = DenonMC6000MK2.MIDI_TRI_LED_ON;
    components.Button.prototype.onShifted = DenonMC6000MK2.MIDI_TRI_LED_BLINK;
    components.Button.prototype.outValueScale = function(value) {
        if (value > 0) {
            if (this.sendShifted) {
                return this.onShifted;
            } else {
                return this.on;
            }
        } else {
            return this.off;
        }
    };
    components.Button.prototype.send = function(value) {
        if (this.midi === undefined || this.midi[0] === undefined || this.midi[1] === undefined) {
            return;
        }
        // For Denon hardware we need to swap the 2 midi bytes (= 2nd/3rd param) in sendShortMsg()!
        midi.sendShortMsg(this.midi[0], value, this.midi[1]);
        if (this.sendShifted) {
            if (this.shiftChannel) {
                midi.sendShortMsg(this.midi[0] + this.shiftOffset, value, this.midi[1]);
            } else if (this.shiftControl) {
                midi.sendShortMsg(this.midi[0], value, this.midi[1] + this.shiftOffset);
            }
        }
    };

    // Init both sides (left/right)
    DenonMC6000MK2.leftSide = new DenonMC6000MK2.Side("left", [1], DenonMC6000MK2.oldLeftSide);
    DenonMC6000MK2.rightSide = new DenonMC6000MK2.Side("right", [2], DenonMC6000MK2.oldRightSide);

    // Init left side decks (1/3)
    DenonMC6000MK2.leftDeck1 = new DenonMC6000MK2.Deck(1, DenonMC6000MK2.MIDI_CH0);
    DenonMC6000MK2.leftSide.connectDeck(DenonMC6000MK2.leftDeck1);
    DenonMC6000MK2.leftDeck3 = new DenonMC6000MK2.Deck(3, DenonMC6000MK2.MIDI_CH1);
    DenonMC6000MK2.leftSide.connectDeck(DenonMC6000MK2.leftDeck3);

    // Init right side decks (2/4)
    DenonMC6000MK2.rightDeck2 = new DenonMC6000MK2.Deck(2, DenonMC6000MK2.MIDI_CH2);
    DenonMC6000MK2.rightSide.connectDeck(DenonMC6000MK2.rightDeck2);
    DenonMC6000MK2.rightDeck4 = new DenonMC6000MK2.Deck(4, DenonMC6000MK2.MIDI_CH3);
    DenonMC6000MK2.rightSide.connectDeck(DenonMC6000MK2.rightDeck4);

    DenonMC6000MK2.allDecks = [
        DenonMC6000MK2.leftDeck1,
        DenonMC6000MK2.rightDeck2,
        DenonMC6000MK2.leftDeck3,
        DenonMC6000MK2.rightDeck4,
    ];

    try {
        DenonMC6000MK2.initValues();
        DenonMC6000MK2.connectLeds();
        DenonMC6000MK2.connectControls();
        for (var index in DenonMC6000MK2.oldSides) {
            var oldSide = DenonMC6000MK2.oldSides[index];
            oldSide.initFilter();
        }
    } catch (ex) {
        DenonMC6000MK2.logError("Exception during controller initialization: " + ex);
    }
};

DenonMC6000MK2.shutdown = function() {
    DenonMC6000MK2.logInfo("Shutting down controller");

    try {
        DenonMC6000MK2.disconnectControls();
        DenonMC6000MK2.disconnectLeds();
        DenonMC6000MK2.restoreValues();
    } catch (ex) {
        DenonMC6000MK2.logError("Exception during controller shutdown: " + ex);
    }
};

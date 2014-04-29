////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// Controller: Denon MC6000MK2
// Author: Uwe Klotz a/k/a tapir
// Revision: 2014-04-29
//
// Changelog:
// 2014-04-29 Many MIDI mapping fixes
//   - Many MIDI mapping fixes, mainly for [Channel3/4]
//   - AutoDJ changes
//     - Shift + Cue triggers skip_next
//     - Shift + Play triggers fade_now
//   - Internal refactorings & cleanup
// 2014-04-28 Polishing
//   - Global shift state
//   - Slip mode fixes
//   - Looping improvements
//   - Cue button controls AutoDJ fade/skip
//   - Shift + play triggers stutter play
//   - Internal refactoring of hotcues
//   - Delete obsolete constants & triggers
//   - Minor changes
// 2014-04-27 Samplers and various improvements
//   - 16 samplers (4 per deck)
//   - Hotcue fixes
//   - Sync mode switching improvements
//   - Use cue mix LEDs as track load indicator
//   - Delete obsolete filter controls
// 2014-04-17 Track loading
//   - Solo cue mix when loading a track (can also triggered manually
//     by pressing Shift + Cue Mix)
//   - Slip mode fixes (still does not always work as expected)
// 2014-04-16 Looping
//   - Loops: Manual, beat and beatroll
//   - Censor: Use reverseroll instead of reverse
//   - Scratching: Don't modify keylock mode
// 2014-04-14 Direct mapping
//   - Map some controls and the VU meter LEDs directly
// 2014-04-13 Initial revision
//   - Just the basics
//   - No loops
//   - No samplers
//   - No filters and effects
//   - No microphone support
////////////////////////////////////////////////////////////////////////

function DenonMC6000MK2 () {}


////////////////////////////////////////////////////////////////////////
// Tunable constants                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.JOG_SPIN_CUE_PEAK = 0.7; // [0.0, 1.0]
DenonMC6000MK2.JOG_SPIN_CUE_EXPONENT = 0.5; // 1.0 = linear response

DenonMC6000MK2.JOG_SPIN_PLAY_PEAK = 0.5; // [0.0, 1.0]
DenonMC6000MK2.JOG_SPIN_PLAY_EXPONENT = 0.5; // 1.0 = linear response

DenonMC6000MK2.JOG_SCRATCH_RPM = 33.333333; // 33 1/3
DenonMC6000MK2.JOG_SCRATCH_ALPHA = 0.125; // 1/8
DenonMC6000MK2.JOG_SCRATCH_BETA = DenonMC6000MK2.JOG_SCRATCH_ALPHA / 32.0;
DenonMC6000MK2.JOG_SCRATCH_RAMP = true;

// Seeking: Number of revolutions needed to seek from the beginning
// to the end of the track.
DenonMC6000MK2.JOG_SEEK_REVOLUTIONS = 3;


////////////////////////////////////////////////////////////////////////
// Fixed constants                                                    //
////////////////////////////////////////////////////////////////////////

// Controller constants
DenonMC6000MK2.BRAND = "Denon";
DenonMC6000MK2.MODEL = "MC6000MK2";
DenonMC6000MK2.DECK_COUNT = 4;
DenonMC6000MK2.SAMPLER_COUNT = 4 * DenonMC6000MK2.DECK_COUNT; // 4 per deck
DenonMC6000MK2.SAMPLE_RATE = 44100;
DenonMC6000MK2.JOG_RESOLUTION = 600; // measured/estimated

// Jog constants
DenonMC6000MK2.MIDI_JOG_DELTA_BIAS = 0x40; // center value of relative movements
DenonMC6000MK2.MIDI_JOG_DELTA_RANGE = 0x3F; // both forward (= positive) and reverse (= negative)

DenonMC6000MK2.JOG_SPIN_CUE_SCALE = 1.0 / (DenonMC6000MK2.MIDI_JOG_DELTA_RANGE * DenonMC6000MK2.JOG_SPIN_CUE_PEAK);
DenonMC6000MK2.JOG_SPIN_PLAY_SCALE = 1.0 / (DenonMC6000MK2.MIDI_JOG_DELTA_RANGE * DenonMC6000MK2.JOG_SPIN_PLAY_PEAK);
DenonMC6000MK2.JOG_SEEK_SCALE = 1.0 / (DenonMC6000MK2.JOG_SEEK_REVOLUTIONS * DenonMC6000MK2.JOG_RESOLUTION);

// Mixxx constants
DenonMC6000MK2.MIXXX_JOG_RANGE = 3.0;
DenonMC6000MK2.MIXXX_SYNC_NONE = 0;
DenonMC6000MK2.MIXXX_SYNC_SLAVE = 1;
DenonMC6000MK2.MIXXX_SYNC_MASTER = 2;
DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED = -1;


////////////////////////////////////////////////////////////////////////
// Globale variables                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.shiftState = false;


////////////////////////////////////////////////////////////////////////
// Logging functions                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.logDebug = function (msg) {
	if (DenonMC6000MK2.debug) {
		print("[" + DenonMC6000MK2.id + " DEBUG] " + msg);
	}
};

DenonMC6000MK2.logInfo = function (msg) {
	print("[" + DenonMC6000MK2.id + " INFO] " + msg);
};

DenonMC6000MK2.logWarning = function (msg) {
	print("[" + DenonMC6000MK2.id + " WARNING] " + msg);
};

DenonMC6000MK2.logError = function (msg) {
	print("[" + DenonMC6000MK2.id + " ERROR] " + msg);
};


////////////////////////////////////////////////////////////////////////
// Buttons                                                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_BUTTON_ON = 0x40;
DenonMC6000MK2.MIDI_BUTTON_OFF = 0x00;

DenonMC6000MK2.isButtonPressed = function (midiValue) {
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

DenonMC6000MK2.getKnobDelta = function (midiValue) {
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

DenonMC6000MK2.LedState = function (midiCtrl) {
	this.midiCtrl = midiCtrl;
};

DenonMC6000MK2.LED_ON = new DenonMC6000MK2.LedState(DenonMC6000MK2.MIDI_LED_ON);
DenonMC6000MK2.LED_OFF = new DenonMC6000MK2.LedState(DenonMC6000MK2.MIDI_LED_OFF);

DenonMC6000MK2.Led = function (midiChannel, midiValue) {
	this.midiChannel = midiChannel;
	this.midiValue = midiValue;
	this.state = undefined;
};

DenonMC6000MK2.Led.prototype.setState = function (ledState) {
	this.state = ledState;
	this.trigger();
};

DenonMC6000MK2.Led.prototype.setStateBoolean = function (booleanValue) {
	if (booleanValue) {
		this.setState(DenonMC6000MK2.LED_ON);
	} else {
		this.setState(DenonMC6000MK2.LED_OFF);
	}
};

DenonMC6000MK2.Led.prototype.trigger = function () {
	midi.sendShortMsg(0xB0 + this.midiChannel, this.state.midiCtrl, this.midiValue);
};

DenonMC6000MK2.Led.prototype.reset = function () {
	this.setStateBoolean(false);
};

DenonMC6000MK2.MIDI_TRI_LED_ON = 0x4A;
DenonMC6000MK2.MIDI_TRI_LED_OFF = 0x4B;
DenonMC6000MK2.MIDI_TRI_LED_BLINK = 0x4C;

DenonMC6000MK2.TriLedState = function (midiCtrl) {
	this.midiCtrl = midiCtrl;
};

DenonMC6000MK2.TRI_LED_ON = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_ON);
DenonMC6000MK2.TRI_LED_OFF = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_OFF);
DenonMC6000MK2.TRI_LED_BLINK = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_BLINK);

DenonMC6000MK2.TriLed = function (midiChannel, midiValue) {
	this.midiChannel = midiChannel;
	this.midiValue = midiValue;
	this.state = undefined;
};

DenonMC6000MK2.TriLed.prototype.setTriState = function (ledState) {
	this.state = ledState;
	this.trigger();
};

DenonMC6000MK2.TriLed.prototype.setStateBoolean = function (booleanValue) {
	if (booleanValue) {
		this.setTriState(DenonMC6000MK2.TRI_LED_ON);
	} else {
		this.setTriState(DenonMC6000MK2.TRI_LED_OFF);
	}
};

DenonMC6000MK2.TriLed.prototype.trigger = function () {
	midi.sendShortMsg(0xB0 + this.midiChannel, this.state.midiCtrl, this.midiValue);
};

DenonMC6000MK2.TriLed.prototype.reset = function () {
	this.setStateBoolean(false);
};

DenonMC6000MK2.connectedLeds = [];

DenonMC6000MK2.connectLed = function (midiChannel, midiValue) {
	var led = new DenonMC6000MK2.Led(midiChannel, midiValue);
	led.reset();
	DenonMC6000MK2.connectedLeds.push(led);
	return led;
};

DenonMC6000MK2.connectTriLed = function (midiChannel, midiValue) {
	var led = new DenonMC6000MK2.TriLed(midiChannel, midiValue);
	led.reset();
	DenonMC6000MK2.connectedLeds.push(led);
	return led;
};

DenonMC6000MK2.disconnectLeds = function () {
	for (var connectedLed in DenonMC6000MK2.connectedLeds) {
		connectedLed.reset();
	}
	DenonMC6000MK2.connectedLeds = [];
};


////////////////////////////////////////////////////////////////////////
// Controls                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.Control = function (group, ctrl, func) {
	this.group = group;
	this.ctrl = ctrl;
	this.func = func;
	this.isConnected = false;
};

DenonMC6000MK2.Control.prototype.connect = function () {
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

DenonMC6000MK2.Control.prototype.disconnect = function () {
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

DenonMC6000MK2.Control.prototype.trigger = function () {
	engine.trigger(this.group, this.ctrl);
};

DenonMC6000MK2.connectedControls = [];

DenonMC6000MK2.connectControl = function (group, ctrl, func) {
	var control = new DenonMC6000MK2.Control(group, ctrl, func);
	if (control.connect()) {
		DenonMC6000MK2.connectedControls.push(control);
		return control;
	} else {
		return undefined;
	}
};

DenonMC6000MK2.disconnectControls = function () {
	for (var connectedControl in DenonMC6000MK2.connectedControls) {
		connectedControl.disconnect();
	}
	DenonMC6000MK2.connectedControls = [];
};


////////////////////////////////////////////////////////////////////////
// Hotcues                                                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.Hotcue = function (deck, number, midiLedValue, midiDimmerLedValue) {
	this.deck = deck;
	this.number = number;
	this.ctrlPrefix = "hotcue_" + number;
	this.midiLedValue = midiLedValue;
	this.midiDimmerLedValue = midiDimmerLedValue;
};

DenonMC6000MK2.Hotcue.prototype.connectControls = function (callbackFunc) {
	this.deck.connectControl(this.ctrlPrefix + "_enabled", callbackFunc);
};

DenonMC6000MK2.Hotcue.prototype.isEnabled = function () {
	return this.deck.getValue(this.ctrlPrefix + "_enabled");
};

DenonMC6000MK2.Hotcue.prototype.onButton = function (isButtonPressed) {
	if (isButtonPressed && DenonMC6000MK2.shiftState) {
		this.deck.setValue(this.ctrlPrefix + "_clear", true);
	} else {
		this.deck.setValue(this.ctrlPrefix + "_activate", isButtonPressed);
	}
};

DenonMC6000MK2.Hotcue.prototype.connectLeds = function () {
	this.led = this.deck.connectTriLed(this.midiLedValue);
	this.dimmerLed = this.deck.connectTriLed(this.midiDimmerLedValue);
};

DenonMC6000MK2.Hotcue.prototype.updateLeds = function () {
	this.led.setStateBoolean(this.isEnabled());
};


////////////////////////////////////////////////////////////////////////
// Samplers                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.samplersByGroup = {};

DenonMC6000MK2.getSamplerByGroup = function (group) {
	var sampler = DenonMC6000MK2.samplersByGroup[group];
	if (undefined === sampler) {
		DenonMC6000MK2.logError("No sampler found for " + group);
	}
	return sampler;
};

DenonMC6000MK2.Sampler = function (deck, number, midiLedValue, midiDimmerLedValue) {
	this.deck = deck;
	this.number = number;
	this.group = "[Sampler" + number + "]";
	this.midiLedValue = midiLedValue;
	this.midiDimmerLedValue = midiDimmerLedValue;
	DenonMC6000MK2.samplersByGroup[this.group] = this;
};

DenonMC6000MK2.Sampler.prototype.isLoaded = function () {
	return 0 < engine.getValue(this.group, "track_samples");
};

DenonMC6000MK2.Sampler.prototype.isPlaying = function () {
	return engine.getValue(this.group, "play");
};

DenonMC6000MK2.Sampler.prototype.onButtonPressed = function() {
	if (DenonMC6000MK2.shiftState) {
		engine.setValue(this.group, "start_stop", true);
	} else {
		engine.setValue(this.group, "start_play", true);
	}
};

DenonMC6000MK2.Sampler.prototype.connectLeds = function () {
	this.led = this.deck.connectTriLed(this.midiLedValue);
	this.dimmerLed = this.deck.connectTriLed(this.midiDimmerLedValue);
};

DenonMC6000MK2.Sampler.prototype.updateLeds = function () {
	if (this.isPlaying()) {
		this.led.setStateBoolean(true);
	} else {
		this.dimmerLed.setStateBoolean(this.isLoaded());
	}
};

DenonMC6000MK2.onSamplerValue = function (value, group, control) {
	var sampler = DenonMC6000MK2.getSamplerByGroup(group);
	sampler.updateLeds();
};

DenonMC6000MK2.Sampler.prototype.connectControls = function () {
	DenonMC6000MK2.connectControl(this.group, "play", DenonMC6000MK2.onSamplerValue);
	DenonMC6000MK2.connectControl(this.group, "track_samples", DenonMC6000MK2.onSamplerValue);
};


////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

/* Management */

DenonMC6000MK2.decksByGroup = {};

DenonMC6000MK2.getDeckByGroup = function (group) {
	var deck = DenonMC6000MK2.decksByGroup[group];
	if (undefined === deck) {
		DenonMC6000MK2.logError("No deck found for " + group);
	}
	return deck;
};

/* Constructor */

DenonMC6000MK2.Deck = function (number, midiChannel) {
	this.side = undefined;
	this.number = number;
	this.group = "[Channel" + number + "]";
	this.midiChannel = midiChannel;
	this.jogTouchState = false;
	this.jogTouchVinylState = false;
	DenonMC6000MK2.decksByGroup[this.group] = this;
	this.rateDirBackup = this.getValue("rate_dir");
	this.setValue("rate_dir", -1);
	this.vinylMode = undefined;
	this.syncMode = undefined;
	this.hotcues = [];
	this.hotcues[1] = new DenonMC6000MK2.Hotcue(this, 1, 0x11, 0x12);
	this.hotcues[2] = new DenonMC6000MK2.Hotcue(this, 2, 0x13, 0x14);
	this.hotcues[3] = new DenonMC6000MK2.Hotcue(this, 3, 0x15, 0x16);
	this.hotcues[4] = new DenonMC6000MK2.Hotcue(this, 4, 0x17, 0x18);
	this.samplers = [];
	var samplerNumberOffset = (this.number - 1) * DenonMC6000MK2.DECK_COUNT;
	this.samplers[1] = new DenonMC6000MK2.Sampler(this, samplerNumberOffset + 1, 0x19, 0x1A);
	this.samplers[2] = new DenonMC6000MK2.Sampler(this, samplerNumberOffset + 2, 0x1B, 0x1C);
	this.samplers[3] = new DenonMC6000MK2.Sampler(this, samplerNumberOffset + 3, 0x1D, 0x1F);
	this.samplers[4] = new DenonMC6000MK2.Sampler(this, samplerNumberOffset + 4, 0x20, 0x21);
};

/* Values */

DenonMC6000MK2.Deck.prototype.getValue = function (key) {
	return engine.getValue(this.group, key);
};

DenonMC6000MK2.Deck.prototype.setValue = function (key, value) {
	engine.setValue(this.group, key, value);
};

DenonMC6000MK2.Deck.prototype.toggleValue = function (key) {
	this.setValue(key, !this.getValue(key));
};

DenonMC6000MK2.Deck.prototype.triggerValue = function (key) {
	engine.trigger(this.group, key);
};

/* Tracks */

DenonMC6000MK2.Deck.prototype.loadSelectedTrack = function () {
	this.setValue("LoadSelectedTrack", true);
	this.setCueMixSolo(); // just for convenience ;)
};

DenonMC6000MK2.Deck.prototype.unloadTrack = function () {
	this.setValue("eject", true);
};

/* Key Lock Mode */

DenonMC6000MK2.Deck.prototype.onKeyLockButtonPressed = function () {
	this.toggleValue("keylock");
};

DenonMC6000MK2.Deck.prototype.enableKeyLock = function () {
	this.setValue("keylock", true);
};

DenonMC6000MK2.Deck.prototype.disableKeyLock = function () {
	this.setValue("keylock", false);
};

DenonMC6000MK2.Deck.prototype.onKeyLockValue = function (value) {
	this.keyLockLed.setStateBoolean(value);
};

DenonMC6000MK2.onKeyLockValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onKeyLockValue(value);
};

/* Sync Mode */

DenonMC6000MK2.Deck.prototype.onSyncButtonPressed = function () {
	var syncMode;
	if (DenonMC6000MK2.shiftState) {
		// disable sync mode
		syncMode = DenonMC6000MK2.MIXXX_SYNC_NONE;
	} else {
		var deck, deckGroup;
		// Rationale: Only decks that are currently playing can become
		// the new sync master. Otherwise all sync slaves would stop
		// playing! An exception of this rule is when none of the
		// other decks is in sync mode.
		syncMode = this.getValue("sync_mode");
		switch (syncMode) {
		case DenonMC6000MK2.MIXXX_SYNC_NONE:
			// become the new master if none of the other decks is in
			// sync mode
			syncMode = DenonMC6000MK2.MIXXX_SYNC_MASTER;
			for (deckGroup in DenonMC6000MK2.decksByGroup) {
				deck = DenonMC6000MK2.decksByGroup[deckGroup];
				if (this !== deck) {
					if (DenonMC6000MK2.MIXXX_SYNC_NONE != deck.getValue("sync_mode")) {
						syncMode = DenonMC6000MK2.MIXXX_SYNC_SLAVE;
						// exit loop
						break;
					}
				}
			}
			break;
		case DenonMC6000MK2.MIXXX_SYNC_SLAVE:
			// become the new master if playing or if none of the other
			// decks is in sync mode
			syncMode = DenonMC6000MK2.MIXXX_SYNC_MASTER;
			if (!this.isPlaying()) {
				for (deckGroup in DenonMC6000MK2.decksByGroup) {
					deck = DenonMC6000MK2.decksByGroup[deckGroup];
					if (this !== deck) {
						if (DenonMC6000MK2.MIXXX_SYNC_NONE != deck.getValue("sync_mode")) {
							syncMode = DenonMC6000MK2.MIXXX_SYNC_SLAVE;
							// exit loop
							break;
						}
					}
				}
			}
			break;
		case DenonMC6000MK2.MIXXX_SYNC_MASTER:
			// nothing to do
			break;
		default:
			DenonMC6000MK2.logError("Unknown sync_mode value: " + syncMode);
		}
	}
	this.setValue("sync_mode", syncMode);
};

DenonMC6000MK2.Deck.prototype.disableSyncMode = function () {
	this.setValue("sync_mode", DenonMC6000MK2.MIXXX_SYNC_NONE);
};

DenonMC6000MK2.Deck.prototype.onSyncModeValue = function (value) {
	switch (value) {
	case DenonMC6000MK2.MIXXX_SYNC_NONE:
		this.syncModeLed.setTriState(DenonMC6000MK2.TRI_LED_OFF);
		break;
	case DenonMC6000MK2.MIXXX_SYNC_SLAVE:
		this.syncModeLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
		break;
	case DenonMC6000MK2.MIXXX_SYNC_MASTER:
		this.syncModeLed.setTriState(DenonMC6000MK2.TRI_LED_ON);
		break;
	default:
		DenonMC6000MK2.logError("Unknown sync_mode value: " + value);
	}
};

DenonMC6000MK2.onSyncModeValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onSyncModeValue(value);
};

/* Cue Mix */

DenonMC6000MK2.Deck.prototype.setCueMixSolo = function () {
	for (var deckGroup in DenonMC6000MK2.decksByGroup) {
		var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
		deck.setValue("pfl", this === deck);
	}
};

DenonMC6000MK2.Deck.prototype.onCueMixButtonPressed = function () {
	if (DenonMC6000MK2.shiftState) {
		this.setCueMixSolo();
	} else {
		this.toggleValue("pfl");
	}
};

DenonMC6000MK2.Deck.prototype.updateCueMixValue = function (pflValue, isTrackLoaded) {
	if (pflValue) {
		this.cueMixLed.setStateBoolean(pflValue);
	} else {
		this.cueMixDimmerLed.setStateBoolean(isTrackLoaded);
	}
};

DenonMC6000MK2.Deck.prototype.onCueMixValue = function (pflValue) {
	this.updateCueMixValue(pflValue, this.isTrackLoaded());
};

DenonMC6000MK2.onCueMixValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCueMixValue(value);
};

/* Track Load */

DenonMC6000MK2.isTrackLoaded = function (trackSamples) {
	return 0 < trackSamples;
};

DenonMC6000MK2.Deck.prototype.isTrackLoaded = function () {
	return DenonMC6000MK2.isTrackLoaded(this.getValue("track_samples"));
};

DenonMC6000MK2.Deck.prototype.onTrackSamplesValue = function (value) {
	this.updateCueMixValue(this.getValue("pfl"), DenonMC6000MK2.isTrackLoaded(value));
};

DenonMC6000MK2.onTrackSamplesValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onTrackSamplesValue(value);
};

/* Cue & Play */

DenonMC6000MK2.Deck.prototype.onCueButton = function (isButtonPressed) {
	if (DenonMC6000MK2.shiftState && engine.getValue("[AutoDJ]", "enabled")) {
		if (isButtonPressed) {
			engine.setValue("[AutoDJ]", "skip_next", true);
		}
	} else {
		if (DenonMC6000MK2.shiftState) {
			if (isButtonPressed) {
				if (this.isPlaying()) {
					// move cue point
					this.setValue("cue_set", true);
				} else {
					// clear cue point
					if (0.0 < this.getValue("cue_point")) {
						this.setValue("cue_point", 0.0);
					} else {
						// jump to beginning of track
						this.setValue("playposition", 0.0);
					}
				}
			}
		} else {
			this.setValue("cue_default", isButtonPressed);
		}
	}
};

DenonMC6000MK2.Deck.prototype.onCueIndicatorValue = function (value) {
	this.cueLed.setStateBoolean(value);
};

DenonMC6000MK2.onCueIndicatorValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCueIndicatorValue(value);
};

DenonMC6000MK2.Deck.prototype.onPlayButtonPressed = function () {
	if (DenonMC6000MK2.shiftState && engine.getValue("[AutoDJ]", "enabled")) {
		engine.setValue("[AutoDJ]", "fade_now", true);
	} else {
		if (DenonMC6000MK2.shiftState) {
			this.setValue("play_stutter", true);
		} else {
			this.toggleValue("play");
		}
	}
};

DenonMC6000MK2.Deck.prototype.isPlaying = function () {
	return this.getValue("play");
};

DenonMC6000MK2.Deck.prototype.onPlayIndicatorValue = function (value) {
	this.playLed.setStateBoolean(value);
};

DenonMC6000MK2.onPlayIndicatorValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onPlayIndicatorValue(value);
};

/* Pitch Bend / Track Search */

DenonMC6000MK2.Deck.prototype.onBendPlusButton = function (isButtonPressed) {
	if (DenonMC6000MK2.shiftState) {
		this.setValue("fwd", isButtonPressed);
	} else {
		this.setValue("fwd", false);
		if (this.isPlaying()) {
			this.setValue("rate_temp_up", isButtonPressed);
		}
	}
};

DenonMC6000MK2.Deck.prototype.onBendMinusButton = function (isButtonPressed) {
	if (DenonMC6000MK2.shiftState) {
		this.setValue("back", isButtonPressed);
	} else {
		this.setValue("back", false);
		if (this.isPlaying()) {
			this.setValue("rate_temp_down", isButtonPressed);
		}
	}
};

/* Hotcues */

DenonMC6000MK2.onHotcue1Value = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.hotcues[1].updateLeds();
};

DenonMC6000MK2.onHotcue2Value = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.hotcues[2].updateLeds();
};

DenonMC6000MK2.onHotcue3Value = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.hotcues[3].updateLeds();
};

DenonMC6000MK2.onHotcue4Value = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.hotcues[4].updateLeds();
};


/* Censor / Slip Mode */

DenonMC6000MK2.Deck.prototype.onCensorButton = function (isButtonPressed) {
	if (DenonMC6000MK2.shiftState) {
		// Please note that "reverseroll" seems to have side effects on
		// "slip_enabled" so better leave it alone while shift is pressed!
		if (isButtonPressed) {
			this.toggleValue("slip_enabled");
		}
	} else {
		this.setValue("reverseroll", isButtonPressed);
	}
};

DenonMC6000MK2.Deck.prototype.onSlipModeValue = function (value) {
	this.slipModeLed.setStateBoolean(value);
};

DenonMC6000MK2.onSlipModeValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onSlipModeValue(value);
};

/* Jog Wheel */

DenonMC6000MK2.Deck.prototype.touchJog = function (isJogTouched) {
	this.jogTouchState = isJogTouched;
};

DenonMC6000MK2.Deck.prototype.spinJog = function (jogDelta) {
	if (DenonMC6000MK2.shiftState) {
		// seeking
		var playPos = engine.getValue(this.group, "playposition");
		if (undefined !== playPos) {
			var seekPos = playPos + (jogDelta * DenonMC6000MK2.JOG_SEEK_SCALE);
			this.setValue("playposition", Math.max(0.0, Math.min(1.0, seekPos)));
		}
	} else {
		var scaledDelta;
		var jogExponent;
		if (this.isPlaying()) {
			scaledDelta = jogDelta * DenonMC6000MK2.JOG_SPIN_PLAY_SCALE;
			jogExponent = DenonMC6000MK2.JOG_SPIN_PLAY_EXPONENT;
		} else {
			scaledDelta = jogDelta * DenonMC6000MK2.JOG_SPIN_CUE_SCALE;
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
};

/* Vinyl Mode (Scratching) */

DenonMC6000MK2.Deck.prototype.isScratching = function () {
	return engine.isScratching(this.number);
};

DenonMC6000MK2.Deck.prototype.enableScratching = function () {
	if (!this.isScratching()) {
		engine.scratchEnable(this.number,
			DenonMC6000MK2.JOG_RESOLUTION,
			DenonMC6000MK2.JOG_SCRATCH_RPM,
			DenonMC6000MK2.JOG_SCRATCH_ALPHA,
			DenonMC6000MK2.JOG_SCRATCH_BETA,
			DenonMC6000MK2.JOG_SCRATCH_RAMP);
	}
};

DenonMC6000MK2.Deck.prototype.disableScratching = function () {
	if (this.isScratching()) {
		engine.scratchDisable(this.number, DenonMC6000MK2.JOG_SCRATCH_RAMP);
	}
};

DenonMC6000MK2.Deck.prototype.scratchJog = function (jogDelta) {
	engine.scratchTick(this.number, jogDelta);
};

DenonMC6000MK2.Deck.prototype.onVinylModeValue = function () {
	this.vinylModeLed.setStateBoolean(this.vinylMode);
	engine.setValue("[Spinny" + this.number + "]", "show_spinny", this.vinylMode);
};

DenonMC6000MK2.Deck.prototype.updateVinylMode = function () {
	if (this.vinylMode && this.jogTouchVinylState) {
		this.enableScratching();
	} else {
		this.disableScratching();
	}
	this.onVinylModeValue();
};

DenonMC6000MK2.Deck.prototype.setVinylMode = function (vinylMode) {
	this.vinylMode = vinylMode;
	this.updateVinylMode();
};

DenonMC6000MK2.Deck.prototype.onVinylButtonPressed = function () {
	this.setVinylMode(!this.vinylMode);
};

DenonMC6000MK2.Deck.prototype.enableVinylMode = function () {
	this.setVinylMode(true);
};

DenonMC6000MK2.Deck.prototype.disableVinylMode = function () {
	this.setVinylMode(false);
};

DenonMC6000MK2.Deck.prototype.touchJogVinyl = function (isJogTouched) {
	this.jogTouchVinylState = isJogTouched;
	this.updateVinylMode();
};

DenonMC6000MK2.Deck.prototype.spinJogVinyl = function (jogDelta) {
	if (this.isScratching()) {
		this.scratchJog(jogDelta);
	} else {
		this.spinJog(jogDelta);
	}
};

/* Loops */

DenonMC6000MK2.Deck.prototype.hasLoopStart = function () {
	return DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED != this.getValue("loop_start_position");
};

DenonMC6000MK2.Deck.prototype.hasLoopEnd = function () {
	return DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED != this.getValue("loop_end_position");
};

DenonMC6000MK2.Deck.prototype.hasLoop = function () {
	return this.hasLoopStart() && this.hasLoopEnd();
};

DenonMC6000MK2.Deck.prototype.deleteLoopStart = function () {
	this.setValue("loop_start_position", DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED);
};

DenonMC6000MK2.Deck.prototype.deleteLoopEnd = function () {
	this.setValue("loop_end_position", DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED);
};

DenonMC6000MK2.Deck.prototype.deleteLoop = function () {
	this.deleteLoopStart();
	this.deleteLoopEnd();
};

DenonMC6000MK2.Deck.prototype.toggleLoop = function () {
	this.setValue("reloop_exit", true);
};

DenonMC6000MK2.Deck.prototype.onAutoLoopButtonPressed = function () {
	if (this.hasLoop()) {
		if (DenonMC6000MK2.shiftState) {
			this.deleteLoop();
		} else {
			this.toggleLoop();
		}
	} else {
		if (DenonMC6000MK2.shiftState) {
			this.setValue("beatlooproll_4_activate", true);
		} else {
			this.setValue("beatloop_4_activate", true);
		}
	}
};

DenonMC6000MK2.Deck.prototype.onLoopInButtonPressed = function () {
	if (DenonMC6000MK2.shiftState) {
		this.deleteLoop(); // both start & end
	} else {
		this.setValue("loop_in", true);
	}
};

DenonMC6000MK2.Deck.prototype.onLoopOutButtonPressed = function () {
	if (DenonMC6000MK2.shiftState) {
		this.deleteLoopEnd();
	} else {
		this.setValue("loop_out", true);
	}
};

DenonMC6000MK2.Deck.prototype.onLoopCutMinusButtonPressed = function () {
	this.setValue("loop_halve", true);
};

DenonMC6000MK2.Deck.prototype.onLoopCutPlusButtonPressed = function () {
	this.setValue("loop_double", true);
};

DenonMC6000MK2.Deck.prototype.updateLoopLeds = function (value) {
	if (this.getValue("loop_enabled")) {
		this.loopInLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
		this.loopOutLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
		this.autoLoopLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
	} else {
		this.loopInLed.setStateBoolean(this.hasLoopStart());
		this.loopOutLed.setStateBoolean(this.hasLoop()); // both start & end
		if (this.hasLoop()) {
			this.autoLoopDimmerLed.setStateBoolean(true);
		} else {
			this.autoLoopDimmerLed.setStateBoolean(false);
		}
	}
};

DenonMC6000MK2.onLoopEnabledValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.updateLoopLeds();
};

DenonMC6000MK2.onLoopStartPositionValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.updateLoopLeds();
};

DenonMC6000MK2.onLoopEndPositionValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.updateLoopLeds();
};

/* Filter */

DenonMC6000MK2.Deck.prototype.onFilterButtonPressed = function () {
	// TODO
};

DenonMC6000MK2.Deck.prototype.onFilterKnobValue = function (value) {
	// TODO
};

/* Deck LEDs */

DenonMC6000MK2.Deck.prototype.connectLed = function (midiValue) {
	return DenonMC6000MK2.connectLed(this.midiChannel, midiValue);
};

DenonMC6000MK2.Deck.prototype.connectTriLed = function (midiValue) {
	return DenonMC6000MK2.connectTriLed(this.midiChannel, midiValue);
};

/* Startup */

DenonMC6000MK2.Deck.prototype.connectLeds = function () {
	this.vinylModeLed = this.connectTriLed(0x06);
	this.keyLockLed = this.connectTriLed(0x08);
	this.syncModeLed = this.connectTriLed(0x09);
	this.cueLed = this.connectTriLed(0x26);
	this.playLed = this.connectTriLed(0x27);
	this.loopInLed = this.connectTriLed(0x24);
	this.loopInDimmerLed = this.connectTriLed(0x3E);
	this.loopOutLed = this.connectTriLed(0x40);
	this.loopOutDimmerLed = this.connectTriLed(0x2A);
	this.autoLoopLed = this.connectTriLed(0x2B);
	this.autoLoopDimmerLed = this.connectTriLed(0x53);
	this.slipModeLed = this.connectTriLed(0x64);
	this.hotcues[1].connectLeds();
	this.hotcues[2].connectLeds();
	this.hotcues[3].connectLeds();
	this.hotcues[4].connectLeds();
	this.samplers[1].connectLeds();
	this.samplers[2].connectLeds();
	this.samplers[3].connectLeds();
	this.samplers[4].connectLeds();
};

DenonMC6000MK2.Deck.prototype.connectControl = function (ctrl, func) {
	return DenonMC6000MK2.connectControl(this.group, ctrl, func);
};

DenonMC6000MK2.Deck.prototype.connectControls = function () {
	this.connectControl("cue_indicator", DenonMC6000MK2.onCueIndicatorValue);
	this.connectControl("play_indicator", DenonMC6000MK2.onPlayIndicatorValue);
	this.connectControl("sync_mode", DenonMC6000MK2.onSyncModeValue);
	this.connectControl("keylock", DenonMC6000MK2.onKeyLockValue);
	this.connectControl("pfl", DenonMC6000MK2.onCueMixValue);
	this.connectControl("track_samples", DenonMC6000MK2.onTrackSamplesValue);
	this.connectControl("slip_enabled", DenonMC6000MK2.onSlipModeValue);
	this.connectControl("loop_enabled", DenonMC6000MK2.onLoopEnabledValue);
	this.connectControl("loop_start_position", DenonMC6000MK2.onLoopStartPositionValue);
	this.connectControl("loop_end_position", DenonMC6000MK2.onLoopEndPositionValue);
	this.hotcues[1].connectControls(DenonMC6000MK2.onHotcue1Value);
	this.hotcues[2].connectControls(DenonMC6000MK2.onHotcue2Value);
	this.hotcues[3].connectControls(DenonMC6000MK2.onHotcue3Value);
	this.hotcues[4].connectControls(DenonMC6000MK2.onHotcue4Value);
	this.samplers[1].connectControls();
	this.samplers[2].connectControls();
	this.samplers[3].connectControls();
	this.samplers[4].connectControls();
	// default settings
	this.enableKeyLock();
	this.enableVinylMode();
	this.disableSyncMode();
};

/* Shutdown */

DenonMC6000MK2.Deck.prototype.restoreValues = function () {
	this.setValue("rate_dir", this.rateDirBackup);
};


////////////////////////////////////////////////////////////////////////
// Sides                                                              //
////////////////////////////////////////////////////////////////////////

/* Management */

DenonMC6000MK2.sidesByGroup = {};

DenonMC6000MK2.getSideByGroup = function (group) {
	var side = DenonMC6000MK2.sidesByGroup[group];
	if (undefined === side) {
		DenonMC6000MK2.logError("No side found for " + group);
	}
	return side;
};

/* Constructor */

DenonMC6000MK2.Side = function (decks) {
	this.decksByGroup = {};
	for (var deckIndex in decks) {
		var deck = decks[deckIndex];
		deck.side = this;
		this.decksByGroup[deck.group] = deck;
		DenonMC6000MK2.sidesByGroup[deck.group] = this;
	}
};

/* Decks */

DenonMC6000MK2.Side.prototype.getDeckByGroup = function (group) {
	var deck = this.decksByGroup[group];
	if (undefined === deck) {
		DenonMC6000MK2.logError("No deck found for " + group);
	}
	return deck;
};

/* Filter */

DenonMC6000MK2.Side.prototype.onFilterButtonPressed = function () {
	for (var group in this.decksByGroup) {
		var deck = this.decksByGroup[group];
		deck.onFilterButtonPressed();
	}
};

DenonMC6000MK2.Side.prototype.onFilterKnobValue = function (value) {
	for (var group in this.decksByGroup) {
		var deck = this.decksByGroup[group];
		deck.onFilterKnobValue(value);
	}
};

/* Startup */

DenonMC6000MK2.Side.prototype.connectLeds = function () {
	for (var group in this.decksByGroup) {
		var deck = this.decksByGroup[group];
		deck.connectLeds();
	}
};

DenonMC6000MK2.Side.prototype.connectControls = function () {
	for (var group in this.decksByGroup) {
		var deck = this.decksByGroup[group];
		deck.connectControls();
	}
};

/* Shutdown */

DenonMC6000MK2.Side.prototype.restoreValues = function () {
	for (var group in this.decksByGroup) {
		var deck = this.decksByGroup[group];
		deck.restoreValues();
	}
};


////////////////////////////////////////////////////////////////////////
// Controller functions                                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.id = undefined;
DenonMC6000MK2.debug = undefined;
DenonMC6000MK2.group = "[Master]";


DenonMC6000MK2.getValue = function (key) {
	return engine.getValue(DenonMC6000MK2.group, key);
};

DenonMC6000MK2.setValue = function (key, value) {
	engine.setValue(DenonMC6000MK2.group, key, value);
};

DenonMC6000MK2.triggerValue = function (key) {
	engine.trigger(DenonMC6000MK2.group, key);
};

DenonMC6000MK2.onFxEfxButtonPressed = function (fx, efx) {
	// TODO
};

DenonMC6000MK2.onFxEfxKnobValue = function (fx, efx, value) {
	// TODO
};

DenonMC6000MK2.getJogDeltaValue = function (value) {
	if (0x00 === value) {
		return 0;
	} else {
		return value - DenonMC6000MK2.MIDI_JOG_DELTA_BIAS;
	}
};


DenonMC6000MK2.initValues = function () {
	DenonMC6000MK2.backupSampleRate = engine.getValue(DenonMC6000MK2.group, "samplerate");
	DenonMC6000MK2.setValue("samplerate", DenonMC6000MK2.SAMPLE_RATE);
	DenonMC6000MK2.backupNumDecks = DenonMC6000MK2.getValue("num_decks");
	DenonMC6000MK2.setValue("num_decks", DenonMC6000MK2.DECK_COUNT);
	DenonMC6000MK2.backupNumSamplers = DenonMC6000MK2.getValue("num_samplers");
	DenonMC6000MK2.setValue("num_samplers", DenonMC6000MK2.SAMPLER_COUNT);
	// left side
	DenonMC6000MK2.deck1 = new DenonMC6000MK2.Deck(1, 0x00);
	DenonMC6000MK2.deck3 = new DenonMC6000MK2.Deck(3, 0x01);
	DenonMC6000MK2.leftDecks = [ DenonMC6000MK2.deck1, DenonMC6000MK2.deck3 ];
	DenonMC6000MK2.leftSide = new DenonMC6000MK2.Side(DenonMC6000MK2.leftDecks);
	// right side
	DenonMC6000MK2.deck2 = new DenonMC6000MK2.Deck(2, 0x02);
	DenonMC6000MK2.deck4 = new DenonMC6000MK2.Deck(4, 0x03);
	DenonMC6000MK2.rightDecks = [ DenonMC6000MK2.deck2, DenonMC6000MK2.deck4 ];
	DenonMC6000MK2.rightSide = new DenonMC6000MK2.Side(DenonMC6000MK2.rightDecks);
};

DenonMC6000MK2.connectLeds = function () {
	DenonMC6000MK2.deck1.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x45);
	DenonMC6000MK2.deck1.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x46);
	DenonMC6000MK2.deck2.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x4B);
	DenonMC6000MK2.deck2.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x4C);
	DenonMC6000MK2.deck3.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x51);
	DenonMC6000MK2.deck3.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x52);
	DenonMC6000MK2.deck4.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x57);
	DenonMC6000MK2.deck4.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x58);
	DenonMC6000MK2.leftSide.filterLed = DenonMC6000MK2.connectLed(0x00, 0x63);
	DenonMC6000MK2.leftSide.connectLeds();
	DenonMC6000MK2.rightSide.filterLed = DenonMC6000MK2.connectLed(0x00, 0x64);
	DenonMC6000MK2.rightSide.connectLeds();
};

DenonMC6000MK2.connectControls = function () {
	DenonMC6000MK2.leftSide.connectControls();
	DenonMC6000MK2.rightSide.connectControls();
};

DenonMC6000MK2.restoreValues = function () {
	DenonMC6000MK2.leftSide.restoreValues();
	DenonMC6000MK2.rightSide.restoreValues();
	DenonMC6000MK2.setValue("num_samplers", DenonMC6000MK2.backupNumSamplers);
	DenonMC6000MK2.setValue("num_decks", DenonMC6000MK2.backupNumDecks);
	DenonMC6000MK2.setValue("samplerate", DenonMC6000MK2.backupSampleRate);
};


////////////////////////////////////////////////////////////////////////
// Mixxx controller callback functions                                //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.init = function (id, debug) {
	DenonMC6000MK2.id = id;
	DenonMC6000MK2.debug = debug;
	try {
		DenonMC6000MK2.initValues();
		DenonMC6000MK2.connectLeds();
		DenonMC6000MK2.connectControls();
	} catch (ex) {
		DenonMC6000MK2.logError("Exception during controller initialization: " + ex);
	}
};

DenonMC6000MK2.shutdown = function () {
	try {
		DenonMC6000MK2.disconnectControls();
		DenonMC6000MK2.disconnectLeds();
		DenonMC6000MK2.restoreValues();
	} catch (ex) {
		DenonMC6000MK2.logError("Exception during controller shutdown: " + ex);
	}
};


////////////////////////////////////////////////////////////////////////
// MIDI [Master] callback functions                                   //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.onTrackSelectKnob = function (channel, control, value, status, group) {
	var knobDelta = DenonMC6000MK2.getKnobDelta(value);
	engine.setValue("[Playlist]", "SelectPrevTrack", 0 > knobDelta);
	engine.setValue("[Playlist]", "SelectNextTrack", 0 < knobDelta);
};

DenonMC6000MK2.onFilterButtonLeft = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.leftSide.onFilterButtonPressed();
	}
};

DenonMC6000MK2.onFilterButtonRight = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.rightSide.onFilterButtonPressed();
	}
};

DenonMC6000MK2.onFilterKnobLeft = function (channel, control, value, status, group) {
	DenonMC6000MK2.leftSide.onFilterKnobValue(value);
};

DenonMC6000MK2.onFilterKnobRight = function (channel, control, value, status, group) {
	DenonMC6000MK2.rightSide.onFilterKnobValue(value);
};

DenonMC6000MK2.onXfaderContour = function (channel, control, value, status, group) {
	// TODO
};

DenonMC6000MK2.onBoothLevel = function (channel, control, value, status, group) {
	// TODO
};


////////////////////////////////////////////////////////////////////////
// MIDI [Channel<n>] callback functions                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.onShiftButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	// global shift state for all decks
	DenonMC6000MK2.shiftState = isButtonPressed;
};

DenonMC6000MK2.onLoadButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.loadSelectedTrack();
	}
};

DenonMC6000MK2.onVinylButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onVinylButtonPressed();
	}
};

DenonMC6000MK2.onKeyLockButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onKeyLockButtonPressed();
	}
};

DenonMC6000MK2.onCueMixButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onCueMixButtonPressed();
	}
};

DenonMC6000MK2.onCueButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCueButton(isButtonPressed);
};

DenonMC6000MK2.onPlayButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onPlayButtonPressed();
	}
};

DenonMC6000MK2.onBendPlusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onBendPlusButton(isButtonPressed);
};

DenonMC6000MK2.onBendMinusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onBendMinusButton(isButtonPressed);
};

DenonMC6000MK2.onSyncButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onSyncButtonPressed();
	}
};

DenonMC6000MK2.onJogTouch = function (channel, control, value, status, group) {
	var isJogTouched = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.touchJog(isJogTouched);
};

DenonMC6000MK2.onJogTouchVinyl = function (channel, control, value, status, group) {
	var isJogTouched = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.touchJogVinyl(isJogTouched);
};

DenonMC6000MK2.onJogSpin = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	// Serato DJ/Intro: Jog wheel movements are recognized wether or
	// not touching the sensor plate when not scratching. This behavior
	// can easily be adjusted by uncommenting the following lines of
	// code.
	//if (deck.jogTouchState) {
		var jogDelta = DenonMC6000MK2.getJogDeltaValue(value);
		deck.spinJog(jogDelta);
	//}
};

DenonMC6000MK2.onJogSpinVinyl = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var jogDelta = DenonMC6000MK2.getJogDeltaValue(value);
	deck.spinJogVinyl(jogDelta);
};

DenonMC6000MK2.onHotcue1Button = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	deck.hotcues[1].onButton(isButtonPressed);
};

DenonMC6000MK2.onHotcue2Button = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	deck.hotcues[2].onButton(isButtonPressed);
};

DenonMC6000MK2.onHotcue3Button = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	deck.hotcues[3].onButton(isButtonPressed);
};

DenonMC6000MK2.onHotcue4Button = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	deck.hotcues[4].onButton(isButtonPressed);
};

DenonMC6000MK2.onSampler1Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.samplers[1].onButtonPressed();
	}
};

DenonMC6000MK2.onSampler2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.samplers[2].onButtonPressed();
	}
};

DenonMC6000MK2.onSampler3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.samplers[3].onButtonPressed();
	}
};

DenonMC6000MK2.onSampler4Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.samplers[4].onButtonPressed();
	}
};

DenonMC6000MK2.onAutoLoopButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onAutoLoopButtonPressed();
	}
};

DenonMC6000MK2.onLoopInButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onLoopInButtonPressed();
	}
};

DenonMC6000MK2.onLoopOutButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onLoopOutButtonPressed();
	}
};

DenonMC6000MK2.onLoopCutMinusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onLoopCutMinusButtonPressed();
	}
};

DenonMC6000MK2.onLoopCutPlusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onLoopCutPlusButtonPressed();
	}
};

DenonMC6000MK2.onCensorButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCensorButton(isButtonPressed);
};

DenonMC6000MK2.onFx1Efx1Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.onFxEfxButtonPressed(1, 1);
	}
};

DenonMC6000MK2.onFx1Efx2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.onFxEfxButtonPressed(1, 2);
	}
};

DenonMC6000MK2.onFx1Efx3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.onFxEfxButtonPressed(1, 3);
	}
};

DenonMC6000MK2.onFx2Efx1Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.onFxEfxButtonPressed(2, 1);
	}
};

DenonMC6000MK2.onFx2Efx2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.onFxEfxButtonPressed(2, 2);
	}
};

DenonMC6000MK2.onFx2Efx3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.onFxEfxButtonPressed(2, 3);
	}
};

DenonMC6000MK2.onFx1Efx1Knob = function (channel, control, value, status, group) {
	DenonMC6000MK2.onFxEfxKnobValue(1, 1, value);
};

DenonMC6000MK2.onFx1Efx2Knob = function (channel, control, value, status, group) {
	DenonMC6000MK2.onFxEfxKnobValue(1, 2, value);
};

DenonMC6000MK2.onFx1Efx3Knob = function (channel, control, value, status, group) {
	DenonMC6000MK2.onFxEfxKnobValue(1, 3, value);
};

DenonMC6000MK2.onFx2Efx1Knob = function (channel, control, value, status, group) {
	DenonMC6000MK2.onFxEfxKnobValue(2, 1, value);
};

DenonMC6000MK2.onFx2Efx2Knob = function (channel, control, value, status, group) {
	DenonMC6000MK2.onFxEfxKnobValue(2, 2, value);
};

DenonMC6000MK2.onFx2Efx3Knob = function (channel, control, value, status, group) {
	DenonMC6000MK2.onFxEfxKnobValue(2, 3, value);
};

DenonMC6000MK2.onFx1BeatsKnob = function (channel, control, value, status, group) {
	// TODO
};

DenonMC6000MK2.onFx2BeatsKnob = function (channel, control, value, status, group) {
	// TODO
};

DenonMC6000MK2.onFx1TapButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		// TODO
	}
};

DenonMC6000MK2.onFx2TapButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		// TODO
	}
};

DenonMC6000MK2.onFx1AssignButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		// TODO
	}
};

DenonMC6000MK2.onFx2AssignButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		// TODO
	}
};

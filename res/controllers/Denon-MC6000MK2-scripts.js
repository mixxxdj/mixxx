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
// Revision: 2014-04-13
//
// Changelog:
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

// VU meter: 1st green LED
DenonMC6000MK2.METER_BIAS = 0.2; // [0.0, 1.0]
// VU meter: 1st yellow LED
DenonMC6000MK2.METER_SPLIT = 0.8; // [DenonMC6000MK2.METER_BIAS, 1.0]


////////////////////////////////////////////////////////////////////////
// Fixed constants                                                    //
////////////////////////////////////////////////////////////////////////

// Controller constants
DenonMC6000MK2.BRAND = "Denon";
DenonMC6000MK2.MODEL = "MC6000MK2";
DenonMC6000MK2.DECK_COUNT = 4;
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

DenonMC6000MK2.MIDI_METER_LED_ON = 0x01;
DenonMC6000MK2.MIDI_METER_LED_OFF = 0x00;

DenonMC6000MK2.MeterLedState = function (midiValue) {
	this.midiValue = midiValue;
};

DenonMC6000MK2.METER_LED_ON = new DenonMC6000MK2.MeterLedState(DenonMC6000MK2.MIDI_METER_LED_ON);
DenonMC6000MK2.METER_LED_OFF = new DenonMC6000MK2.MeterLedState(DenonMC6000MK2.MIDI_METER_LED_OFF);

DenonMC6000MK2.MeterLed = function (midiChannel, midiCtrl) {
	this.midiChannel = midiChannel;
	this.midiCtrl = midiCtrl;
	this.state = undefined;
};

DenonMC6000MK2.MeterLed.prototype.setState = function (ledState) {
	this.state = ledState;
	this.trigger();
};

DenonMC6000MK2.MeterLed.prototype.setStateBoolean = function (booleanValue) {
	if (booleanValue) {
		this.setState(DenonMC6000MK2.METER_LED_ON);
	} else {
		this.setState(DenonMC6000MK2.METER_LED_OFF);
	}
};

DenonMC6000MK2.MeterLed.prototype.trigger = function () {
	midi.sendShortMsg(0xB0 + this.midiChannel, this.midiCtrl, this.state.midiValue);
};

DenonMC6000MK2.MeterLed.prototype.reset = function () {
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

DenonMC6000MK2.connectMeterLed = function (midiChannel, midiCtrl) {
	var led = new DenonMC6000MK2.MeterLed(midiChannel, midiCtrl);
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
// Hot cues                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.HotCue = function (number, midiLedValue, midiDimmerLedValue) {
	this.number = number;
	this.ctrlPrefix = "hotcue_" + number;
	this.midiLedValue = midiLedValue;
	this.midiDimmerLedValue = midiDimmerLedValue;
};

DenonMC6000MK2.HOT_CUE_1 = new DenonMC6000MK2.HotCue(1, 0x11, 0x12);
DenonMC6000MK2.HOT_CUE_2 = new DenonMC6000MK2.HotCue(2, 0x13, 0x14);
DenonMC6000MK2.HOT_CUE_3 = new DenonMC6000MK2.HotCue(3, 0x15, 0x16);
DenonMC6000MK2.HOT_CUE_4 = new DenonMC6000MK2.HotCue(4, 0x17, 0x18);


////////////////////////////////////////////////////////////////////////
// Samplers                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.Sampler = function (number, midiLedValue, midiDimmerLedValue) {
	this.number = number;
	this.midiLedValue = midiLedValue;
	this.midiDimmerLedValue = midiDimmerLedValue;
};

DenonMC6000MK2.SAMPLER_1 = new DenonMC6000MK2.Sampler(1, 0x19, 0x1A);
DenonMC6000MK2.SAMPLER_2 = new DenonMC6000MK2.Sampler(2, 0x1B, 0x1C);
DenonMC6000MK2.SAMPLER_3 = new DenonMC6000MK2.Sampler(3, 0x1D, 0x1F);
DenonMC6000MK2.SAMPLER_4 = new DenonMC6000MK2.Sampler(4, 0x20, 0x21);


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
	this.setValue("volume", 0.0);
	this.jogTouchState = false;
	this.jogTouchVinylState = false;
	DenonMC6000MK2.decksByGroup[this.group] = this;
	this.rateDirBackup = this.getValue("rate_dir");
	this.setValue("rate_dir", -1);
	this.disableScratchingTimer = undefined;
	this.hotCues = [];
	this.hotCues[1] = new DenonMC6000MK2.HotCue(this, 1, 0x11, 0x12);
	this.hotCues[2] = new DenonMC6000MK2.HotCue(this, 2, 0x13, 0x14);
	this.hotCues[3] = new DenonMC6000MK2.HotCue(this, 3, 0x15, 0x16);
	this.hotCues[4] = new DenonMC6000MK2.HotCue(this, 4, 0x17, 0x18);
	this.samplers = [];
	this.samplers[1] = new DenonMC6000MK2.Sampler(this, 1, 0x19, 0x1A);
	this.samplers[2] = new DenonMC6000MK2.Sampler(this, 2, 0x1B, 0x1C);
	this.samplers[3] = new DenonMC6000MK2.Sampler(this, 3, 0x1D, 0x1F);
	this.samplers[4] = new DenonMC6000MK2.Sampler(this, 4, 0x20, 0x21);
};

/* Shift State */

DenonMC6000MK2.Deck.prototype.getShiftState = function () {
	return this.side.shiftState;
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
};

DenonMC6000MK2.Deck.prototype.unloadTrack = function () {
	this.setValue("eject", true);
};

/* Key Lock Mode */

DenonMC6000MK2.Deck.prototype.toggleKeyLock = function () {
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

DenonMC6000MK2.Deck.prototype.toggleSyncMode = function () {
	var syncMode;
	if (this.getShiftState()) {
		syncMode = DenonMC6000MK2.MIXXX_SYNC_NONE;
	} else {
		// Rationale: Only decks that are currently playing can become
		// the new sync master. Otherwise all sync slaves will stop
		// playing!
		syncMode = this.getValue("sync_mode");
		switch (syncMode) {
		case DenonMC6000MK2.MIXXX_SYNC_NONE:
			if (this.isPlaying()) {
				syncMode = DenonMC6000MK2.MIXXX_SYNC_MASTER;
				for (var deckGroup in DenonMC6000MK2.decksByGroup) {
					var deck = DenonMC6000MK2.decksByGroup[deckGroup];
					if (DenonMC6000MK2.MIXXX_SYNC_MASTER == deck.getValue("sync_mode")) {
						// follow the current master
						syncMode = DenonMC6000MK2.MIXXX_SYNC_SLAVE;
						// exit loop
						break;
					}
				}
			} else {
				syncMode = DenonMC6000MK2.MIXXX_SYNC_SLAVE;
			}
			break;
		case DenonMC6000MK2.MIXXX_SYNC_SLAVE:
			if (this.isPlaying()) {
				syncMode = DenonMC6000MK2.MIXXX_SYNC_MASTER;
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

DenonMC6000MK2.Deck.prototype.toggleCueMix = function () {
	this.toggleValue("pfl");
};

DenonMC6000MK2.Deck.prototype.onCueMixValue = function (value) {
	this.cueMixLed.setStateBoolean(value);
};

DenonMC6000MK2.onCueMixValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCueMixValue(value);
};

/* Cue & Play */

DenonMC6000MK2.Deck.prototype.onCueButton = function (isButtonPressed) {
	if (this.getShiftState()) {
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
};

DenonMC6000MK2.Deck.prototype.onCueIndicatorValue = function (value) {
	this.cueLed.setStateBoolean(value);
};

DenonMC6000MK2.onCueIndicatorValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCueIndicatorValue(value);
};

DenonMC6000MK2.Deck.prototype.togglePlay = function () {
	this.toggleValue("play");
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
	if (this.getShiftState()) {
		this.setValue("fwd", isButtonPressed);
	} else {
		this.setValue("fwd", false);
		if (this.isPlaying()) {
			this.setValue("rate_temp_up", isButtonPressed);
		}
	}
};

DenonMC6000MK2.Deck.prototype.onBendMinusButton = function (isButtonPressed) {
	if (this.getShiftState()) {
		this.setValue("back", isButtonPressed);
	} else {
		this.setValue("back", false);
		if (this.isPlaying()) {
			this.setValue("rate_temp_down", isButtonPressed);
		}
	}
};

/* Hot Cues */

DenonMC6000MK2.Deck.prototype.toggleHotCue = function (hotCue) {
	if (this.getShiftState()) {
		this.setValue(hotCue.ctrlPrefix + "_clear", true);
	} else {
		if (this.getValue(hotCue.ctrlPrefix + "_enabled")) {
			this.setValue(hotCue.ctrlPrefix + "_goto", true);
		} else {
			this.setValue(hotCue.ctrlPrefix + "_set", true);
		}
	}
};

DenonMC6000MK2.Deck.prototype.onHotCue1Value = function (value) {
	this.hotCue1Led.setStateBoolean(value);
};

DenonMC6000MK2.Deck.prototype.onHotCue2Value = function (value) {
	this.hotCue2Led.setStateBoolean(value);
};

DenonMC6000MK2.Deck.prototype.onHotCue3Value = function (value) {
	this.hotCue3Led.setStateBoolean(value);
};

DenonMC6000MK2.Deck.prototype.onHotCue4Value = function (value) {
	this.hotCue4Led.setStateBoolean(value);
};

DenonMC6000MK2.onHotCue1Value = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onHotCue1Value(value);
};

DenonMC6000MK2.onHotCue2Value = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onHotCue2Value(value);
};

DenonMC6000MK2.onHotCue3Value = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onHotCue3Value(value);
};

DenonMC6000MK2.onHotCue4Value = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onHotCue4Value(value);
};

/* Censor / Slip Mode */

DenonMC6000MK2.Deck.prototype.onCensorButton = function (isButtonPressed) {
	if (this.getShiftState()) {
		if (isButtonPressed) {
			this.toggleValue("slip_enabled");
		} else {
			this.setValue("reverse", false);
		}
	} else {
		this.setValue("slip_enabled", isButtonPressed);
		this.setValue("reverse", isButtonPressed);
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
	if (this.getShiftState()) {
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
		this.disableKeyLock();
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
		// Leave keylock disabled to avoid unnatural playback and glitches!
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

DenonMC6000MK2.Deck.prototype.toggleVinylMode = function () {
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

/* Filter */

DenonMC6000MK2.Deck.prototype.toggleFilter = function () {
	this.toggleValue("filter");
};

DenonMC6000MK2.Deck.prototype.setFilterDepth = function (filterDepth) {
	this.setValue("filterDepth", filterDepth);
};

DenonMC6000MK2.Deck.prototype.onFilterValue = function (value) {
	this.side.filterLed.setStateBoolean(value);
};

DenonMC6000MK2.onFilterValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onFilterValue(value);
};

/* Loops */

DenonMC6000MK2.Deck.prototype.toggleAutoLoop = function () {
	// TODO
};

DenonMC6000MK2.Deck.prototype.toggleLoopIn = function () {
	// TODO
};

DenonMC6000MK2.Deck.prototype.toggleLoopOut = function () {
	// TODO
};

DenonMC6000MK2.Deck.prototype.toggleLoopCutMinus = function () {
	// TODO
};

DenonMC6000MK2.Deck.prototype.toggleLoopCutPlus = function () {
	// TODO
};

/* VU Meter */

DenonMC6000MK2.Deck.prototype.onPeakIndicatorValue = function (value) {
	this.redMeterLed.setStateBoolean(value);
};

DenonMC6000MK2.onPeakIndicatorValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onPeakIndicatorValue(value);
};

DenonMC6000MK2.Deck.prototype.onVuMeterValue = function (value) {
	var level;
	var bias;
	for (level = 0; level < this.greenMeterLeds.length; ++level) {
		bias = this.greenMeterBias + level * this.greenMeterScale;
		this.greenMeterLeds[level].setStateBoolean(value > bias);
	}
	for (level = 0; level < this.yellowMeterLeds.length; ++level) {
		bias = this.yellowMeterBias + level * this.yellowMeterScale;
		this.yellowMeterLeds[level].setStateBoolean(value > bias);
	}
};

DenonMC6000MK2.onVuMeterValue = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onVuMeterValue(value);
};

/* Deck LEDs */

DenonMC6000MK2.Deck.prototype.connectLed = function (midiValue) {
	return DenonMC6000MK2.connectLed(this.midiChannel, midiValue);
};

DenonMC6000MK2.Deck.prototype.connectTriLed = function (midiValue) {
	return DenonMC6000MK2.connectTriLed(this.midiChannel, midiValue);
};

DenonMC6000MK2.Deck.prototype.connectMeterLed = function (midiCtrl) {
	return DenonMC6000MK2.connectMeterLed(this.midiChannel, midiCtrl);
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
	this.hotCue1Led = this.connectTriLed(DenonMC6000MK2.HOT_CUE_1.midiLedValue);
	this.hotCue1DimmerLed = this.connectTriLed(DenonMC6000MK2.HOT_CUE_1.midiDimmerLedValue);
	this.hotCue2Led = this.connectTriLed(DenonMC6000MK2.HOT_CUE_2.midiLedValue);
	this.hotCue2DimmerLed = this.connectTriLed(DenonMC6000MK2.HOT_CUE_2.midiDimmerLedValue);
	this.hotCue3Led = this.connectTriLed(DenonMC6000MK2.HOT_CUE_3.midiLedValue);
	this.hotCue3DimmerLed = this.connectTriLed(DenonMC6000MK2.HOT_CUE_3.midiDimmerLedValue);
	this.hotCue4Led = this.connectTriLed(DenonMC6000MK2.HOT_CUE_4.midiLedValue);
	this.hotCue4DimmerLed = this.connectTriLed(DenonMC6000MK2.HOT_CUE_4.midiDimmerLedValue);
	this.greenMeterLeds = [
		this.connectMeterLed(0x53),
		this.connectMeterLed(0x54),
		this.connectMeterLed(0x55) ];
	this.yellowMeterLeds = [
		this.connectMeterLed(0x56),
		this.connectMeterLed(0x57),
		this.connectMeterLed(0x58) ];
	this.redMeterLed =
		this.connectMeterLed(0x59);
	this.greenMeterBias = DenonMC6000MK2.METER_BIAS;
	this.greenMeterScale = (DenonMC6000MK2.METER_SPLIT - this.greenMeterBias) / this.greenMeterLeds.length;
	this.yellowMeterBias = DenonMC6000MK2.METER_SPLIT;
	this.yellowMeterScale = (1.0 - this.yellowMeterBias) / this.yellowMeterLeds.length;
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
	this.connectControl("slip_enabled", DenonMC6000MK2.onSlipModeValue);
	this.connectControl("filter", DenonMC6000MK2.onFilterValue);
	this.connectControl(DenonMC6000MK2.HOT_CUE_1.ctrlPrefix + "_enabled", DenonMC6000MK2.onHotCue1Value);
	this.connectControl(DenonMC6000MK2.HOT_CUE_2.ctrlPrefix + "_enabled", DenonMC6000MK2.onHotCue2Value);
	this.connectControl(DenonMC6000MK2.HOT_CUE_3.ctrlPrefix + "_enabled", DenonMC6000MK2.onHotCue3Value);
	this.connectControl(DenonMC6000MK2.HOT_CUE_4.ctrlPrefix + "_enabled", DenonMC6000MK2.onHotCue4Value);
	this.connectControl("VuMeter", DenonMC6000MK2.onVuMeterValue);
	this.connectControl("PeakIndicator", DenonMC6000MK2.onPeakIndicatorValue);
	// default settings
	this.enableKeyLock();
	this.disableSyncMode();
	this.disableVinylMode();
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
	this.shiftState = false;
	this.decksByGroup = {};
	this.selectedDeck = undefined;
	for (var deckIndex in decks) {
		var deck = decks[deckIndex];
		deck.side = this;
		this.decksByGroup[deck.group] = deck;
		DenonMC6000MK2.sidesByGroup[deck.group] = this;
		if (undefined === this.selectedDeck) {
			this.selectedDeck = deck;
		}
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

DenonMC6000MK2.Side.prototype.selectDeckByGroup = function (group) {
	var deck = this.getDeckByGroup(group);
	// TODO
	return deck;
};

DenonMC6000MK2.selectDeckByGroup = function (group) {
	var side = DenonMC6000MK2.getSideByGroup(group);
	return side.selectDeckByGroup(group);
};

/* Tracks */

DenonMC6000MK2.Side.prototype.loadSelectedTrack = function () {
	this.selectedDeck.loadSelectedTrack();
};

/* Filter */

DenonMC6000MK2.Side.prototype.toggleFilter = function () {
	this.selectedDeck.toggleFilter();
};

DenonMC6000MK2.Side.prototype.setFilterDepth = function (filterDepth) {
	this.selectedDeck.setFilterDepth(filterDepth);
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

DenonMC6000MK2.toggleFxEfx = function (fx, efx) {
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
	DenonMC6000MK2.backupNumPreviewDecks = DenonMC6000MK2.getValue("num_preview_decks");
	DenonMC6000MK2.setValue("num_preview_decks", 0); // TODO
	DenonMC6000MK2.backupNumSamplers = DenonMC6000MK2.getValue("num_samplers");
	DenonMC6000MK2.setValue("num_samplers", 0); // TODO
	DenonMC6000MK2.setValue("volume", 0.0);
	DenonMC6000MK2.setValue("headMix", 0.0);
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
	DenonMC6000MK2.setValue("num_preview_decks", DenonMC6000MK2.backupNumPreviewDecks);
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

DenonMC6000MK2.onTrackSelectButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	engine.setValue("[Playlist]", "ToggleSelectedSidebarItem", isButtonPressed);
};

DenonMC6000MK2.onFwdButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	engine.setValue("[Playlist]", "SelectNextPlaylist", isButtonPressed);
};

DenonMC6000MK2.onBackButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	engine.setValue("[Playlist]", "SelectPrevPlaylist", isButtonPressed);
};

DenonMC6000MK2.onLoadLeftButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.leftSide.loadSelectedTrack();
	}
};

DenonMC6000MK2.onLoadRightButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.rightSide.loadSelectedTrack();
	}
};

DenonMC6000MK2.onShiftLeftButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.leftSide.shiftState = isButtonPressed;
};

DenonMC6000MK2.onShiftRightButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.rightSide.shiftState = isButtonPressed;
};

DenonMC6000MK2.onFilterLeftButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.leftSide.toggleFilter();
	}
};

DenonMC6000MK2.onFilterRightButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.rightSide.toggleFilter();
	}
};

DenonMC6000MK2.onFilterLeftKnob = function (channel, control, value, status, group) {
	var filterDepth = value / 0x7F;
	DenonMC6000MK2.leftSide.setFilterDepth(filterDepth);
};

DenonMC6000MK2.onFilterRightKnob = function (channel, control, value, status, group) {
	var filterDepth = value / 0x7F;
	DenonMC6000MK2.rightSide.setFilterDepth(filterDepth);
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

DenonMC6000MK2.onDeckButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.selectDeckByGroup(group);
	}
};

DenonMC6000MK2.onVinylModeButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleVinylMode();
	}
};

DenonMC6000MK2.onKeyLockButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleKeyLock();
	}
};

DenonMC6000MK2.onCueMixButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleCueMix();
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
		deck.togglePlay();
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
		deck.toggleSyncMode();
	}
};

DenonMC6000MK2.onPitchSlider = function (channel, control, value, status, group) {
	var rate = script.pitch(control, value, status);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.setValue("rate", rate);
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

DenonMC6000MK2.onHotCue1Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleHotCue(DenonMC6000MK2.HOT_CUE_1);
	}
};

DenonMC6000MK2.onHotCue2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleHotCue(DenonMC6000MK2.HOT_CUE_2);
	}
};

DenonMC6000MK2.onHotCue3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleHotCue(DenonMC6000MK2.HOT_CUE_3);
	}
};

DenonMC6000MK2.onHotCue4Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleHotCue(DenonMC6000MK2.HOT_CUE_4);
	}
};

DenonMC6000MK2.onSampler1Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	// TODO
};

DenonMC6000MK2.onSampler2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	// TODO
};

DenonMC6000MK2.onSampler3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	// TODO
};

DenonMC6000MK2.onSampler4Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	// TODO
};

DenonMC6000MK2.onAutoLoopButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleAutoLoop();
	}
};

DenonMC6000MK2.onLoopInButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleLoopIn();
	}
};

DenonMC6000MK2.onLoopOutButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleLoopOut();
	}
};

DenonMC6000MK2.onLoopCutMinusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleLoopCutMinus();
	}
};

DenonMC6000MK2.onLoopCutPlusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.toggleLoopCutPlus();
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
		DenonMC6000MK2.toggleFxEfx(1, 1);
	}
};

DenonMC6000MK2.onFx1Efx2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.toggleFxEfx(1, 2);
	}
};

DenonMC6000MK2.onFx1Efx3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.toggleFxEfx(1, 3);
	}
};

DenonMC6000MK2.onFx2Efx1Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.toggleFxEfx(2, 1);
	}
};

DenonMC6000MK2.onFx2Efx2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.toggleFxEfx(2, 2);
	}
};

DenonMC6000MK2.onFx2Efx3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.toggleFxEfx(2, 3);
	}
};

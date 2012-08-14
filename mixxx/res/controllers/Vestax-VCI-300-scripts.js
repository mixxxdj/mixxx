/******************************************************************************
 * Vestax VCI-300 controller script
 * Author: Uwe Klotz
 *
 * Description
 * -----------
 * - High-res jog wheels (1664 steps per revolution) for scratching and pitch bending
 * - High-res pitch sliders (9-bit = 512 steps)
 * - VU meters display left/right channel volume (incl. peak indicator)
 * - 6 hotcues (instead of 3 hotcues and 3 manual loops)
 * - Beatloop starting with 4 beats + Half/Double buttons
 * - Shift + Half/Double jumps to start/end of the loaded track (disabled while playing)
 * - Keylock is disabled permanently when starting to scratch
 * - Pitch Shift buttons for fine tuning the pitch +/-0.01
 * - Navigation buttons work
 * - Crates/Files/Browse buttons are connected but unused
 *
 * Revision history
 * ----------------
 * 2012-08-12: Initial revision for Mixxx 1.11.0
 * 2012-08-13: Show spinning wheels in scratch mode
 *             Fast track search while not playing: Shift + Jog
 * 2012-08-14: Fix typo to avoid crash when pressing Crates button
 *             Fix typo to correctly disconnect controls upon shutdown
 *             Fix array of auto loop beat lengths
 *             Connect and synchronize rateRange with preference settings
 *             Adjust behaviour of Auto Tempo button:
 *               -         Auto Tempo = trigger "beatsync"
 *               - Shift + Auto Tempo = toggle "quantize"
 *             Manipulate crossfader curve via C.F.CURVE control
 *             Use Scroll + Jog to scroll the playlist
 *             Shift + Play = toggle "repeat"
 * ...to be continued...
 *****************************************************************************/


function VestaxVCI300() {}

VestaxVCI300.group = "[Master]";

VestaxVCI300.pitchFineTuneStepPercent = 0.01;

VestaxVCI300.scratchRPM = 33.0 + (1.0 / 3.0); // 33 1/3
VestaxVCI300.scratchAlpha = 1.0 / 8.0;
VestaxVCI300.scratchBeta = this.scratchAlpha / 32.0;
VestaxVCI300.disableScratchingTimeoutMillisec = 20;
VestaxVCI300.jogResolution = 1664; // steps per revolution

VestaxVCI300.jogInputRange = this.jogResolution / this.scratchRPM;
VestaxVCI300.jogOutputRange = 3.0; // -3.0 <= "jog" <= 3.0
VestaxVCI300.jogPitchBendSensitivity = 0.8; // 1.0 = linear
VestaxVCI300.jogSearchScale = this.jogOutputRange / this.jogInputRange;
VestaxVCI300.jogScrollDeltaStepsPerTrack = 8; // 1664 / 8 = 208 tracks per revolution
VestaxVCI300.jogScrollDeltaAdjustment = this.jogScrollDeltaStepsPerTrack - 1;
VestaxVCI300.numberOfHotcues = 6;

VestaxVCI300.autoLoopBeatsArray = [
	"0.0625",
	"0.125",
	"0.25",
	"0.5",
	"1",
	"2",
	"4",
	"8",
	"16",
	"32",
	"64" ];
VestaxVCI300.defaultAutoLoopBeatsIndex = 6; // 4 beats
VestaxVCI300.vuMeterOffThreshold = 0.025;
VestaxVCI300.vuMeterYellowThreshold = 0.9;

VestaxVCI300.decksByGroup = {};
	
VestaxVCI300.allLEDs = [];

VestaxVCI300.updatePitchValue = function (group, pitchHigh, pitchLow) {
	// 0x00 <= pitchHigh <= 0x7F
	// pitchLow = 0x00/0x20/0x40/0x60 -> 2 out of 7 bits
	var pitchValue = (pitchHigh << 7) | pitchLow;
	// pitchValueMin    = 0
	// pitchValueCenter = 8192 = 0x2000
	// pitchValueMax    = 16352
	if (pitchValue <= 8192) {
		engine.setValue(group, "rate", (8192 - pitchValue) / 8192.0);
	} else {
		// 8160 = 16352 - 8192
		engine.setValue(group, "rate", (8192 - pitchValue) / 8160.0);
	}
}

VestaxVCI300.initValues = function () {
	VestaxVCI300.scrollState = false;
	VestaxVCI300.numDecksBackup = engine.getValue(VestaxVCI300.group, "num_decks");
	engine.setValue(VestaxVCI300.group, "num_decks", 2);
	engine.setValue(VestaxVCI300.group, "crossfader", 0.0);
	engine.softTakeover(VestaxVCI300.group, "crossfader", true);
	engine.setValue(VestaxVCI300.group, "headMix", 0.0);
	engine.softTakeover(VestaxVCI300.group, "headMix", true);
}

VestaxVCI300.resetoreValues = function () {
	engine.setValue(VestaxVCI300.group, "num_decks", VestaxVCI300.numDecksBackup);
}

VestaxVCI300.connectControl = function (group, ctrl, func) {
	engine.connectControl(group, ctrl, func);
	engine.trigger(group, ctrl);
}

VestaxVCI300.connectControls = function () {
}

VestaxVCI300.disconnectControls = function () {
}

VestaxVCI300.updateScrollState = function () {
	VestaxVCI300.scrollLED.trigger(VestaxVCI300.scrollState);
}


//
// Buttons
//

VestaxVCI300.getButtonPressed = function (value) {
	switch (value) {
		case 0x00:
			return false;
		case 0x7F:
			return true;
	}
}

VestaxVCI300.toggleBinaryValue = function (group, control) {
	engine.setValue(group, control, !engine.getValue(group, control));
}

VestaxVCI300.onToggleButton = function (group, control, value) {
	if (VestaxVCI300.getButtonPressed(value)) {
		VestaxVCI300.toggleBinaryValue(group, control);
	} else {
		engine.trigger(group, control);
	}
}


//
// LEDs
//

VestaxVCI300.LED = function (control) {
	this.control = control;
	VestaxVCI300.allLEDs.push(this);
}

VestaxVCI300.LED.trigger = function (control, state) {
	midi.sendShortMsg(0x90, control, state ? 0x7F : 0x00);
}

VestaxVCI300.LED.prototype.trigger = function (state) {
	VestaxVCI300.LED.trigger(this.control, state);
}

VestaxVCI300.turnOffAllLEDs = function () {
	for (var led in VestaxVCI300.allLEDs) {
		led.trigger(false);
	}
}


//
// Decks
//

VestaxVCI300.Deck = function (number) {
	this.number = number;
	this.group = "[Channel" + this.number + "]";
	VestaxVCI300.decksByGroup[this.group] = this;
	this.onHotcueValueCB = [];
}

VestaxVCI300.leftDeck = new VestaxVCI300.Deck(1);
VestaxVCI300.rightDeck = new VestaxVCI300.Deck(2);

VestaxVCI300.Deck.prototype.resetScratchingTimer = function () {
	if (undefined != this.disableScratchingTimer) {
		engine.stopTimer(this.disableScratchingTimer);
		this.disableScratchingTimer = undefined
	}
}

VestaxVCI300.Deck.prototype.enableScratching = function () {
	this.resetScratchingTimer();
	engine.setValue(this.group, "keylock", false);
	engine.scratchEnable(this.number, VestaxVCI300.jogResolution, VestaxVCI300.scratchRPM, VestaxVCI300.scratchAlpha, VestaxVCI300.scratchBeta, /*ramp*/ true);
}

VestaxVCI300.Deck.prototype.disableScratching = function () {
	this.resetScratchingTimer();
	if (engine.isScratching(this.number)) {
		engine.scratchDisable(this.number, /*ramp*/ true);
		this.jogMoveLED.trigger(false);
		// leave keylock turned off to avoid unnatural playback
	}
}

VestaxVCI300.leftDeck.disableScratchingTimerCB = function () {
	VestaxVCI300.leftDeck.disableScratching();
}

VestaxVCI300.rightDeck.disableScratchingTimerCB = function () {
	VestaxVCI300.rightDeck.disableScratching();
}

VestaxVCI300.Deck.prototype.disableScratchingLazy = function () {
	this.resetScratchingTimer();
	if (engine.getValue(this.group, "play") || (Math.abs(this.jogDelta) < 1.0)) {
		this.disableScratching();
	} else {
		if (engine.isScratching(this.number)) {
			this.disableScratchingTimer = engine.beginTimer(
				VestaxVCI300.disableScratchingTimeoutMillisec,
				this.disableScratchingTimerCB);
		}
	}
}

VestaxVCI300.Deck.prototype.updateJogValue = function (jogHigh, jogLow) {
	// 0x00 <= jogHigh/jogLow <= 0x7F
	var jogValue = (jogHigh << 7) | jogLow;
	// 0x0000 <= jogValue <= 0x3FFF (14-bit, cyclic)
	// 1 revolution = 0x0D00 = 1664 steps
	if (undefined != this.jogValue) {
		this.jogDelta = jogValue - this.jogValue;
		if (this.jogDelta >= 0x2000) {
			// cyclic overflow
			this.jogDelta -= 0x4000;
		} else if (this.jogDelta < -0x2000) {
			// cyclic overflow
			this.jogDelta += 0x4000;
		}
		if (this.jogDelta != 0) {
			if (engine.isScratching(this.number)) {
				engine.scratchTick(this.number, this.jogDelta);
				this.jogMoveLED.trigger(0.0 < Math.abs(this.jogDelta));
			} else {
				if (engine.getValue(this.group,"play")) {
					// pitch bend
					engine.setValue(this.group, "jog", ((this.jogDelta < 0.0) ? -1.0 : 1.0) * VestaxVCI300.jogOutputRange * Math.pow(Math.abs(this.jogDelta) / VestaxVCI300.jogInputRange, VestaxVCI300.jogPitchBendSensitivity));
				} else {
					if (this.shiftState) {
						var playposition = engine.getValue(this.group, "playposition");
						if (undefined != playposition) {
							// fast track search
							var searchpos = engine.getValue(this.group, "playposition") + (this.jogDelta / VestaxVCI300.jogResolution);
							engine.setValue(this.group, "playposition", Math.min(0.0, Math.max(1.0, searchpos)));
						}
					} else if (VestaxVCI300.scrollState) {
						// scroll playlist
						var adjustedJogDelta;
						if (this.jogDelta < 0) {
							adjustedJogDelta = this.jogDelta - VestaxVCI300.jogScrollDeltaAdjustment;
						} else {
							adjustedJogDelta = this.jogDelta + VestaxVCI300.jogScrollDeltaAdjustment;
						}
						engine.setValue(
							"[Playlist]",
							"SelectTrackKnob",
							Math.round(adjustedJogDelta / VestaxVCI300.jogScrollDeltaStepsPerTrack));
					} else {
						// jog search
						engine.setValue(this.group, "jog", this.jogDelta * VestaxVCI300.jogSearchScale);
					}
				}
			}
		 }
	}
	this.jogValue = jogValue;
	if (undefined != this.disableScratchingTimer) {
		this.disableScratchingLazy();
	}
}

VestaxVCI300.Deck.prototype.updateShiftState = function () {
	this.shiftLED.trigger(this.shiftState);
}

VestaxVCI300.Deck.prototype.updateScratchState = function () {
	this.scratchLED.trigger(this.scratchState);
	engine.setValue("[Spinny" + this.number + "]", "show_spinny", this.scratchState);
	if (this.scratchState && this.jogTouchState) {
		this.enableScratching();
	} else {
		this.disableScratchingLazy();
	}
}

VestaxVCI300.Deck.prototype.updateRateRange = function () {
	this.pitchFineTuneStep = VestaxVCI300.pitchFineTuneStepPercent / (100.0 * engine.getValue(this.group, "rateRange"));
}

VestaxVCI300.Deck.prototype.initValues = function () {
	this.shiftState = false;
	this.scratchState = false;
	this.jogTouchState = false;
	this.autoLoopBeatIndex = VestaxVCI300.defaultAutoLoopBeatsIndex;
	this.greenVUMeterOffset = VestaxVCI300.vuMeterOffThreshold;
	this.greenVUMeterScale = (VestaxVCI300.vuMeterYellowThreshold - this.greenVUMeterOffset) / this.greenVUMeterLEDs.length;
	this.redVUMeterOffset = VestaxVCI300.vuMeterYellowThreshold;
	this.redVUMeterScale = (1.0 - this.redVUMeterOffset) / this.redVUMeterLEDs.length;
	engine.setValue(this.group, "volume", 0.0);
	engine.softTakeover(this.group, "volume", true);
	engine.setValue(this.group, "filterHigh", 1.0);
	engine.softTakeover(this.group, "filterHigh", true);
	engine.setValue(this.group, "filterMid", 1.0);
	engine.softTakeover(this.group, "filterMid", true);
	engine.setValue(this.group, "filterLow", 1.0);
	engine.softTakeover(this.group, "filterLow", true);
	engine.setValue(this.group, "pregain", 1.0);
	engine.softTakeover(this.group, "pregain", true);
	this.rateDirBackup = engine.getValue(this.group, "rate_dir");
	engine.setValue(this.group, "rate_dir", -1);
	engine.setValue(this.group, "rate", 0.0);
	engine.softTakeover(this.group, "rate", true);
}

VestaxVCI300.Deck.prototype.restoreValues = function () {
	engine.setValue(this.group, "rate_dir", this.rateDirBackup);
}

VestaxVCI300.Deck.prototype.connectControls = function () {
	VestaxVCI300.connectControl(this.group, "pfl", this.onPFLValueCB);
	VestaxVCI300.connectControl(this.group, "rateRange", this.onRateRangeValueCB);
	VestaxVCI300.connectControl(this.group, "cue_default", this.onCueValueCB);
	VestaxVCI300.connectControl(this.group, "play", this.onPlayValueCB);
	VestaxVCI300.connectControl(this.group, "loop_halve", this.onLoopHalveValueCB);
	VestaxVCI300.connectControl(this.group, "loop_double", this.onLoopDoubleValueCB);
	VestaxVCI300.connectControl(this.group, "keylock", this.onKeylockValueCB);
	VestaxVCI300.connectControl(this.group, "beatsync", this.onBeatsyncValueCB);
	VestaxVCI300.connectControl(this.group, "reverse", this.onReverseValueCB);
	VestaxVCI300.connectControl(this.group, "PeakIndicator", this.onPeakIndicatorValueCB);
	VestaxVCI300.connectControl(this.group, "VuMeter", this.onVUMeterValueCB);
	for (var hotcueIndex = 0; hotcueIndex < VestaxVCI300.numberOfHotcues; ++hotcueIndex) {
		VestaxVCI300.connectControl(
			this.group,
			"hotcue_" + (hotcueIndex  + 1) + "_enabled",
			this.onHotcueValueCB[hotcueIndex]);
	}
	for (var beatsIndex in VestaxVCI300.autoLoopBeatsArray) {
		VestaxVCI300.connectControl(this.group, "beatloop_" + VestaxVCI300.autoLoopBeatsArray[beatsIndex] + "_enabled", this.onAutoLoopValueCB);
	}
}

VestaxVCI300.Deck.prototype.disconnectControls = function () {
	VestaxVCI300.disconnectControl(this.group, "pfl");
	VestaxVCI300.disconnectControl(this.group, "rateRange");
	VestaxVCI300.disconnectControl(this.group, "cue_default");
	VestaxVCI300.disconnectControl(this.group, "play");
	VestaxVCI300.disconnectControl(this.group, "loop_halve");
	VestaxVCI300.disconnectControl(this.group, "loop_double");
	VestaxVCI300.disconnectControl(this.group, "keylock");
	VestaxVCI300.disconnectControl(this.group, "beatsync");
	VestaxVCI300.disconnectControl(this.group, "reverse");
	VestaxVCI300.disconnectControl(this.group, "PeakIndicator");
	VestaxVCI300.disconnectControl(this.group, "VuMeter");
	for (var hotcueIndex = 0; hotcueIndex < VestaxVCI300.numberOfHotcues; ++hotcueIndex) {
		VestaxVCI300.disconnectControl(
			this.group,
			"hotcue_" + (hotcueIndex  + 1) + "_enabled");
	}
	for (var beats in VestaxVCI300.autoLoopBeatsArray) {
		VestaxVCI300.disconnectControl(this.group, "beatloop_" + VestaxVCI300.autoLoopBeatsArray[beatsIndex] + "_enabled");
	}
}

VestaxVCI300.Deck.prototype.updateVUMeterState = function (value) {
	var level;
	for (level = 0; level < this.greenVUMeterLEDs.length; ++level) {
		this.greenVUMeterLEDs[level].trigger(value > (this.greenVUMeterOffset + level * this.greenVUMeterScale));
	}
	for (level = 0; level < this.redVUMeterLEDs.length; ++level) {
		this.redVUMeterLEDs[level].trigger(value > (this.redVUMeterOffset + level * this.redVUMeterScale));
	}
}

VestaxVCI300.Deck.prototype.updateAutoLoopState = function () {
	this.autoLoopLED.trigger(engine.getValue(this.group, "beatloop_" + VestaxVCI300.autoLoopBeatsArray[this.autoLoopBeatIndex] + "_enabled"));
}


//
// Application callback functions
// 

VestaxVCI300.init = function (id, debug) {
	VestaxVCI300.id = id;
	VestaxVCI300.debug = debug;
	VestaxVCI300.scrollLED =
		new VestaxVCI300.LED(0x2F);
	VestaxVCI300.cratesLED =
		new VestaxVCI300.LED(0x2C);
	VestaxVCI300.filesLED =
		new VestaxVCI300.LED(0x2E);
	VestaxVCI300.browseLED =
		new VestaxVCI300.LED(0x30);
	VestaxVCI300.leftDeck.shiftLED =
		new VestaxVCI300.LED(0x27);
	VestaxVCI300.leftDeck.scratchLED =
		new VestaxVCI300.LED(0x26);
	VestaxVCI300.leftDeck.syncLED =
		new VestaxVCI300.LED(0x25);
	VestaxVCI300.leftDeck.cueLED =
		new VestaxVCI300.LED(0x71);
	VestaxVCI300.leftDeck.playLED =
		new VestaxVCI300.LED(0x72);
	VestaxVCI300.leftDeck.jogMoveLED =
		new VestaxVCI300.LED(0x75);
	VestaxVCI300.leftDeck.jogTouchLED =
		new VestaxVCI300.LED(0x76);
	VestaxVCI300.leftDeck.reverseLED =
		new VestaxVCI300.LED(0x28);
	VestaxVCI300.leftDeck.autoLoopLED =
		new VestaxVCI300.LED(0x29);
	VestaxVCI300.leftDeck.pitchShiftDownLED =
		new VestaxVCI300.LED(0x3C);
	VestaxVCI300.leftDeck.pitchShiftUpLED =
		new VestaxVCI300.LED(0x3B);
	VestaxVCI300.leftDeck.pflLED =
		new VestaxVCI300.LED(0x2D);
	VestaxVCI300.leftDeck.keylockLED =
		new VestaxVCI300.LED(0x24);
	VestaxVCI300.leftDeck.autoLoopHalfLED =
		new VestaxVCI300.LED(0x2A);
	VestaxVCI300.leftDeck.autoLoopDoubleLED =
		new VestaxVCI300.LED(0x2B);
	VestaxVCI300.leftDeck.greenVUMeterLEDs = [
		new VestaxVCI300.LED(0x64),
		new VestaxVCI300.LED(0x63),
		new VestaxVCI300.LED(0x62),
		new VestaxVCI300.LED(0x61),
		new VestaxVCI300.LED(0x60),
		new VestaxVCI300.LED(0x5F),
		new VestaxVCI300.LED(0x5E),
		new VestaxVCI300.LED(0x5D),
		new VestaxVCI300.LED(0x5C)
	];
	VestaxVCI300.leftDeck.redVUMeterLEDs = [
		new VestaxVCI300.LED(0x5B),
		new VestaxVCI300.LED(0x5A)
	];
	VestaxVCI300.leftDeck.peakIndicatorLED =
		new VestaxVCI300.LED(0x59);
	VestaxVCI300.leftDeck.hotcueLEDs = [
		[ new VestaxVCI300.LED(0x41) /*red*/, new VestaxVCI300.LED(0x47) /*green*/ ],
		[ new VestaxVCI300.LED(0x43) /*yellow*/, new VestaxVCI300.LED(0x49) /*green*/ ],
		[ new VestaxVCI300.LED(0x45) /*blue*/, new VestaxVCI300.LED(0x4B) /*green*/ ],
		[ new VestaxVCI300.LED(0x42) /*red*/, new VestaxVCI300.LED(0x48) /*green*/ ],
		[ new VestaxVCI300.LED(0x44) /*yellow*/, new VestaxVCI300.LED(0x4A) /*green*/ ],
		[ new VestaxVCI300.LED(0x46) /*blue*/, new VestaxVCI300.LED(0x4C) /*green*/ ]
	];
	VestaxVCI300.rightDeck.shiftLED =
		new VestaxVCI300.LED(0x37);
	VestaxVCI300.rightDeck.scratchLED =
		new VestaxVCI300.LED(0x38);
	VestaxVCI300.rightDeck.syncLED =
		new VestaxVCI300.LED(0x39);
	VestaxVCI300.rightDeck.cueLED =
		new VestaxVCI300.LED(0x73);
	VestaxVCI300.rightDeck.playLED =
		new VestaxVCI300.LED(0x74);
	VestaxVCI300.rightDeck.jogMoveLED =
		new VestaxVCI300.LED(0x77);
	VestaxVCI300.rightDeck.jogTouchLED =
		new VestaxVCI300.LED(0x78);
	VestaxVCI300.rightDeck.reverseLED =
		new VestaxVCI300.LED(0x34);
	VestaxVCI300.rightDeck.autoLoopLED =
		new VestaxVCI300.LED(0x35);
	VestaxVCI300.rightDeck.pitchShiftDownLED =
		new VestaxVCI300.LED(0x3E);
	VestaxVCI300.rightDeck.pitchShiftUpLED =
		new VestaxVCI300.LED(0x3D);
	VestaxVCI300.rightDeck.pflLED =
		new VestaxVCI300.LED(0x31);
	VestaxVCI300.rightDeck.keylockLED =
		new VestaxVCI300.LED(0x33);
	VestaxVCI300.rightDeck.autoLoopHalfLED =
		new VestaxVCI300.LED(0x32);
	VestaxVCI300.rightDeck.autoLoopDoubleLED =
		new VestaxVCI300.LED(0x36);
	VestaxVCI300.rightDeck.greenVUMeterLEDs = [
		new VestaxVCI300.LED(0x70),
		new VestaxVCI300.LED(0x6F),
		new VestaxVCI300.LED(0x6E),
		new VestaxVCI300.LED(0x6D),
		new VestaxVCI300.LED(0x6C),
		new VestaxVCI300.LED(0x6B),
		new VestaxVCI300.LED(0x6A),
		new VestaxVCI300.LED(0x69),
		new VestaxVCI300.LED(0x68) ];
	VestaxVCI300.rightDeck.redVUMeterLEDs = [
		new VestaxVCI300.LED(0x67),
		new VestaxVCI300.LED(0x66) ];
	VestaxVCI300.rightDeck.peakIndicatorLED =
		new VestaxVCI300.LED(0x65);
	VestaxVCI300.rightDeck.hotcueLEDs = [
		[ new VestaxVCI300.LED(0x4D) /*red*/, new VestaxVCI300.LED(0x53) /*green*/ ],
		[ new VestaxVCI300.LED(0x4F) /*yellow*/, new VestaxVCI300.LED(0x55) /*green*/ ],
		[ new VestaxVCI300.LED(0x51) /*blue*/, new VestaxVCI300.LED(0x57) /*green*/ ],
		[ new VestaxVCI300.LED(0x4E) /*red*/, new VestaxVCI300.LED(0x54) /*green*/ ],
		[ new VestaxVCI300.LED(0x50) /*yellow*/, new VestaxVCI300.LED(0x56) /*green*/ ],
		[ new VestaxVCI300.LED(0x52) /*blue*/, new VestaxVCI300.LED(0x58) /*green*/ ]
	];
	VestaxVCI300.initValues();
	VestaxVCI300.connectControls();
	VestaxVCI300.updateScrollState();
	for (var group in VestaxVCI300.decksByGroup) {
		var deck = VestaxVCI300.decksByGroup[group];
		deck.initValues();
		deck.connectControls();
		deck.updateShiftState();
		deck.updateScratchState();
	}
}

VestaxVCI300.shutdown = function () {
	VestaxVCI300.disconnectControls();
	VestaxVCI300.restoreValues();
	for (var group in VestaxVCI300.decksByGroup) {
		var deck = VestaxVCI300.decksByGroup[group];
		deck.resetScratchingTimer();
		deck.disconnectControls();
		deck.restoreValues();
	}
	VestaxVCI300.turnOffAllLEDs();
}


//
// Controller callback functions (see also: Vestax-VCI-300-midi.xml)
// 

VestaxVCI300.onHotcueButton = function (group, hotcue, value) {
	if (VestaxVCI300.getButtonPressed(value)) {
		var deck = VestaxVCI300.decksByGroup[group];
		deck.disableScratching();
		if (deck.shiftState) {
			engine.setValue(group, "hotcue_" + hotcue + "_clear", true);
		} else {
			engine.setValue(group, "hotcue_" + hotcue + "_activate", true);
		}
	}
}

VestaxVCI300.onScrollButton = function (channel, control, value, status, group) {
	VestaxVCI300.scrollState = VestaxVCI300.getButtonPressed(value);
	VestaxVCI300.updateScrollState();
}

VestaxVCI300.onShiftButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.shiftState = VestaxVCI300.getButtonPressed(value);
	deck.updateShiftState();
}

VestaxVCI300.onCueButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.disableScratching();
	engine.setValue(group, "cue_default", VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onPlayButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		if (VestaxVCI300.getButtonPressed(value)) {
			// repeat
			VestaxVCI300.toggleBinaryValue(group, "repeat");
			deck.playLED.trigger(true);
		} else {
			engine.trigger(group, "play");
		}
	} else {
		deck.disableScratching();
		VestaxVCI300.onToggleButton(group, "play", value);
	}
}

VestaxVCI300.onCensorReverseButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		// Reverse
		deck.disableScratching();
		VestaxVCI300.onToggleButton(group, "reverse", value);
		deck.censorPosition = undefined
	} else {
		if (engine.getValue(group, "play")) {
			if (VestaxVCI300.getButtonPressed(value)) {
				if (engine.getValue(group, "reverse")) {
					deck.disableScratching();
					VestaxVCI300.onToggleButton(group, "reverse", value);
				} else {
					deck.disableScratching();
					deck.censorPosition = engine.getValue(group, "playposition");
					engine.setValue(group, "reverse", true);
				}
			} else if ((deck.censorPosition != undefined) && engine.getValue(group, "reverse")) {
				deck.disableScratching();
				var deltaPosition = deck.censorPosition - engine.getValue(group, "playposition");
				engine.setValue(group, "playposition", deck.censorPosition + deltaPosition);
				engine.setValue(group, "reverse", false);
			}
		}
	}
}

VestaxVCI300.onKeyLockButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		if (VestaxVCI300.getButtonPressed(value)) {
			// quartz lock
			engine.setValue(group, "rate", 0.0);
			deck.keylockLED.trigger(true);
		} else {
			engine.trigger(group, "keylock");
		}
	} else {
		if (VestaxVCI300.getButtonPressed(value)) {
			VestaxVCI300.toggleBinaryValue(group, "keylock");
		}
	}
}

VestaxVCI300.onAutoTempoButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		if (VestaxVCI300.getButtonPressed(value)) {
			// quantize
			VestaxVCI300.toggleBinaryValue(group, "quantize");
			deck.syncLED.trigger(true);
		} else {
			engine.trigger(group, "beatsync");
		}
	} else {
		// beatsync
		deck.onBeatsyncValueCB(VestaxVCI300.getButtonPressed(value));
		engine.setValue(group, "beatsync", VestaxVCI300.getButtonPressed(value));
	}
}

VestaxVCI300.onPFLButton = function (channel, control, value, status, group) {
	if (VestaxVCI300.scrollState) {
		// load selected track into deck
		engine.setValue(group, "LoadSelectedTrack", VestaxVCI300.getButtonPressed(value));
		if (VestaxVCI300.getButtonPressed(value)) {
			var deck = VestaxVCI300.decksByGroup[group];
			deck.pflLED.trigger(VestaxVCI300.getButtonPressed(true));
		} else {
			engine.trigger(group, "pfl");
		}
	} else {
		VestaxVCI300.onToggleButton(group, "pfl", value);
	}
}

VestaxVCI300.onHalfPrevButton = function (channel, control, value, status, group) {
	if (VestaxVCI300.scrollState) {
		// TODO
	} else {
		var deck = VestaxVCI300.decksByGroup[group];
		if (deck.shiftState) {
			if (!engine.getValue(group, "play")) {
				// jump to track start
				engine.setValue(group, "start", VestaxVCI300.getButtonPressed(value));
				if (VestaxVCI300.getButtonPressed(value)) {
					deck.autoLoopHalfLED.trigger(true);
				} else {
					engine.trigger(group, "loop_halve");
				}
			}
		} else {
			if (engine.getValue(group, "play")) {
				if (VestaxVCI300.getButtonPressed(value) && (0 < deck.autoLoopBeatIndex)) {
					engine.setValue(group, "loop_halve", true);
					--deck.autoLoopBeatIndex;
				} else {
					engine.setValue(group, "loop_halve", false);
				}
			} else {
				engine.setValue(group, "back", VestaxVCI300.getButtonPressed(value));
			}
		}
	}
}

VestaxVCI300.onDoubleNextButton = function (channel, control, value, status, group) {
	if (VestaxVCI300.scrollState) {
		// TODO
	} else {
		var deck = VestaxVCI300.decksByGroup[group];
		if (deck.shiftState) {
			if (!engine.getValue(group, "play")) {
				// jump to track end
				engine.setValue(group, "end", VestaxVCI300.getButtonPressed(value));
				if (VestaxVCI300.getButtonPressed(value)) {
					deck.autoLoopDoubleLED.trigger(true);
				} else {
					engine.trigger(group, "loop_double");
				}
			}
		} else {
			if (engine.getValue(group, "play")) {
				if (VestaxVCI300.getButtonPressed(value) && (deck.autoLoopBeatIndex < (VestaxVCI300.autoLoopBeatsArray.length - 1))) {
					engine.setValue(group, "loop_double", true);
					++deck.autoLoopBeatIndex;
				} else {
					engine.setValue(group, "loop_double", false);
				}
			} else {
				engine.setValue(group, "fwd", VestaxVCI300.getButtonPressed(value));
			}
		}
	}
}

VestaxVCI300.onPitchHighValue = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.pitchHighValue = value;
	// defer adjusting the pitch until onPitchLowValue() arrives
}

VestaxVCI300.onPitchLowValue = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	VestaxVCI300.updatePitchValue(group, deck.pitchHighValue, value);
}

VestaxVCI300.onPitchShiftDownButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		// TODO
	} else {
		if (VestaxVCI300.getButtonPressed(value)) {
			engine.setValue(group, "rate", engine.getValue(group, "rate") - deck.pitchFineTuneStep);
		}
	}
	deck.pitchShiftDownLED.trigger(VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onPitchShiftUpButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		// TODO
	} else {
		if (VestaxVCI300.getButtonPressed(value)) {
			engine.setValue(group, "rate", engine.getValue(group, "rate") + deck.pitchFineTuneStep);
		}
	}
	deck.pitchShiftUpLED.trigger(VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onScratchButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (VestaxVCI300.getButtonPressed(value)) {
		deck.scratchState = !deck.scratchState;
	}
	deck.updateScratchState();
}

VestaxVCI300.onJogTouch = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.jogTouchState = VestaxVCI300.getButtonPressed(value);
	deck.jogTouchLED.trigger(deck.jogTouchState);
	deck.updateScratchState();
}

VestaxVCI300.onJogHighValue = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.jogHighValue = value;
	// defer adjusting the jog position until onJogLowValue() arrives
}

VestaxVCI300.onJogLowValue = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.updateJogValue(deck.jogHighValue, value);
}

VestaxVCI300.onCue1InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onHotcueButton(group, 1, value);
}

VestaxVCI300.onCue2InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onHotcueButton(group, 2, value);
}

VestaxVCI300.onCue3InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onHotcueButton(group, 3, value);
}

VestaxVCI300.onOut1LoopButton = function (channel, control, value, status, group) {
	VestaxVCI300.onHotcueButton(group, 4, value);
}

VestaxVCI300.onOut2LoopButton = function (channel, control, value, status, group) {
	VestaxVCI300.onHotcueButton(group, 5, value);
}

VestaxVCI300.onOut3LoopButton = function (channel, control, value, status, group) {
	VestaxVCI300.onHotcueButton(group, 6, value);
}

VestaxVCI300.onAutoLoopButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	var beatloopPrefix = "beatloop_" + VestaxVCI300.autoLoopBeatsArray[deck.autoLoopBeatIndex];
	if (VestaxVCI300.getButtonPressed(value)) {
		if (engine.getValue(group, beatloopPrefix + "_enabled")) {
			// reset size upon deactivation
			deck.autoLoopBeatIndex = VestaxVCI300.defaultAutoLoopBeatsIndex;
		}
	}
	engine.setValue(group, beatloopPrefix + "_toggle", VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onCrossfaderCurve = function (channel, control, value, status, group) {
	script.crossfaderCurve(value);
}

VestaxVCI300.onLinefaderCurve = function (channel, control, value, status, group) {
	// TODO
}

VestaxVCI300.onCratesButton = function (channel, control, value, status, group) {
	// TODO
	VestaxVCI300.cratesLED.trigger(VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onFilesButton = function (channel, control, value, status, group) {
	// TODO
	VestaxVCI300.filesLED.trigger(VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onBrowseButton = function (channel, control, value, status, group) {
	// TODO
	VestaxVCI300.browseLED.trigger(VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onNavigationUpButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "SelectPrevTrack", VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onNavigationDownButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "SelectNextTrack", VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onNavigationBackButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "SelectPrevPlaylist", VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onNavigationFwdButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "SelectNextPlaylist", VestaxVCI300.getButtonPressed(value));
}

VestaxVCI300.onNavigationTabButton = function (channel, control, value, status, group) {
	if (VestaxVCI300.getButtonPressed(value)) {
		// TODO
	}
}


//
// Engine callback functions for connected controls
// 

VestaxVCI300.leftDeck.onRateRangeValueCB = function (value) {
	VestaxVCI300.leftDeck.updateRateRange(value);
}

VestaxVCI300.rightDeck.onRateRangeValueCB = function (value) {
	VestaxVCI300.rightDeck.updateRateRange(value);
}

VestaxVCI300.leftDeck.onVUMeterValueCB = function (value) {
	VestaxVCI300.leftDeck.updateVUMeterState(value);
}

VestaxVCI300.rightDeck.onVUMeterValueCB = function (value) {
	VestaxVCI300.rightDeck.updateVUMeterState(value);
}

VestaxVCI300.leftDeck.onPFLValueCB = function (value) {
	VestaxVCI300.leftDeck.pflLED.trigger(value);
}

VestaxVCI300.rightDeck.onPFLValueCB = function (value) {
	VestaxVCI300.rightDeck.pflLED.trigger(value);
}

VestaxVCI300.leftDeck.onLoopHalveValueCB = function (value) {
	VestaxVCI300.leftDeck.autoLoopHalfLED.trigger(value);
}

VestaxVCI300.rightDeck.onLoopHalveValueCB = function (value) {
	VestaxVCI300.rightDeck.autoLoopHalfLED.trigger(value);
}

VestaxVCI300.leftDeck.onLoopDoubleValueCB = function (value) {
	VestaxVCI300.leftDeck.autoLoopDoubleLED.trigger(value);
}

VestaxVCI300.rightDeck.onLoopDoubleValueCB = function (value) {
	VestaxVCI300.rightDeck.autoLoopDoubleLED.trigger(value);
}

VestaxVCI300.leftDeck.onKeylockValueCB = function (value) {
	VestaxVCI300.leftDeck.keylockLED.trigger(value);
}

VestaxVCI300.rightDeck.onKeylockValueCB = function (value) {
	VestaxVCI300.rightDeck.keylockLED.trigger(value);
}

VestaxVCI300.leftDeck.onPeakIndicatorValueCB = function (value) {
	VestaxVCI300.leftDeck.peakIndicatorLED.trigger(value);
}

VestaxVCI300.rightDeck.onPeakIndicatorValueCB = function (value) {
	VestaxVCI300.rightDeck.peakIndicatorLED.trigger(value);
};

VestaxVCI300.leftDeck.onCueValueCB = function (value) {
	VestaxVCI300.leftDeck.cueLED.trigger(value);
}

VestaxVCI300.rightDeck.onCueValueCB = function (value) {
	VestaxVCI300.rightDeck.cueLED.trigger(value);
}

VestaxVCI300.leftDeck.onPlayValueCB = function (value) {
	VestaxVCI300.leftDeck.playLED.trigger(value);
}

VestaxVCI300.rightDeck.onPlayValueCB = function (value) {
	VestaxVCI300.rightDeck.playLED.trigger(value);
}

VestaxVCI300.leftDeck.onBeatsyncValueCB = function (value) {
	VestaxVCI300.leftDeck.syncLED.trigger(value);
}

VestaxVCI300.rightDeck.onBeatsyncValueCB = function (value) {
	VestaxVCI300.rightDeck.syncLED.trigger(value);
}

VestaxVCI300.leftDeck.onReverseValueCB = function (value) {
	VestaxVCI300.leftDeck.reverseLED.trigger(value);
}

VestaxVCI300.rightDeck.onReverseValueCB = function (value) {
	VestaxVCI300.rightDeck.reverseLED.trigger(value);
}

VestaxVCI300.leftDeck.onAutoLoopValueCB = function (value) {
	VestaxVCI300.leftDeck.updateAutoLoopState();
}

VestaxVCI300.rightDeck.onAutoLoopValueCB = function (value) {
	VestaxVCI300.rightDeck.updateAutoLoopState();
}

VestaxVCI300.leftDeck.onHotcueValueCB[0] = function (value) {
	VestaxVCI300.leftDeck.hotcueLEDs[0][0].trigger(value);
	//VestaxVCI300.leftDeck.hotcueLEDs[0][1].trigger(value);
}

VestaxVCI300.rightDeck.onHotcueValueCB[0] = function (value) {
	VestaxVCI300.rightDeck.hotcueLEDs[0][0].trigger(value);
	//VestaxVCI300.rightDeck.hotcueLEDs[0][1].trigger(value);
}

VestaxVCI300.leftDeck.onHotcueValueCB[1] = function (value) {
	VestaxVCI300.leftDeck.hotcueLEDs[1][0].trigger(value);
	//VestaxVCI300.leftDeck.hotcueLEDs[1][1].trigger(value);
}

VestaxVCI300.rightDeck.onHotcueValueCB[1] = function (value) {
	VestaxVCI300.rightDeck.hotcueLEDs[1][0].trigger(value);
	//VestaxVCI300.rightDeck.hotcueLEDs[1][1].trigger(value);
}

VestaxVCI300.leftDeck.onHotcueValueCB[2] = function (value) {
	VestaxVCI300.leftDeck.hotcueLEDs[2][0].trigger(value);
	//VestaxVCI300.leftDeck.hotcueLEDs[2][1].trigger(value);
}

VestaxVCI300.rightDeck.onHotcueValueCB[2] = function (value) {
	VestaxVCI300.rightDeck.hotcueLEDs[2][0].trigger(value);
	//VestaxVCI300.rightDeck.hotcueLEDs[2][1].trigger(value);
}

VestaxVCI300.leftDeck.onHotcueValueCB[3] = function (value) {
	//VestaxVCI300.leftDeck.hotcueLEDs[3][0].trigger(value);
	VestaxVCI300.leftDeck.hotcueLEDs[3][1].trigger(value);
}

VestaxVCI300.rightDeck.onHotcueValueCB[3] = function (value) {
	//VestaxVCI300.rightDeck.hotcueLEDs[3][0].trigger(value);
	VestaxVCI300.rightDeck.hotcueLEDs[3][1].trigger(value);
}

VestaxVCI300.leftDeck.onHotcueValueCB[4] = function (value) {
	VestaxVCI300.leftDeck.hotcueLEDs[4][0].trigger(value);
	VestaxVCI300.leftDeck.hotcueLEDs[4][1].trigger(value);
}

VestaxVCI300.rightDeck.onHotcueValueCB[4] = function (value) {
	VestaxVCI300.rightDeck.hotcueLEDs[4][0].trigger(value);
	VestaxVCI300.rightDeck.hotcueLEDs[4][1].trigger(value);
}

VestaxVCI300.leftDeck.onHotcueValueCB[5] = function (value) {
	VestaxVCI300.leftDeck.hotcueLEDs[5][0].trigger(value);
	VestaxVCI300.leftDeck.hotcueLEDs[5][1].trigger(value);
}

VestaxVCI300.rightDeck.onHotcueValueCB[5] = function (value) {
	VestaxVCI300.rightDeck.hotcueLEDs[5][0].trigger(value);
	VestaxVCI300.rightDeck.hotcueLEDs[5][1].trigger(value);
}


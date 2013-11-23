function NumarkTotalControl() {}

NumarkTotalControl.init = function(id) {	// called when the MIDI device is opened & set up
	NumarkTotalControl.id = id;	// Store the ID of this device for later use
	
	NumarkTotalControl.directoryMode = false;
	
	NumarkTotalControl.scratchMode = false;
	NumarkTotalControl.scratchTimer = [-1, -1];
	
	NumarkTotalControl.simpleCue = false;
	
	NumarkTotalControl.extendedLooping = false;
	NumarkTotalControl.oldLoopStart = [-1, -1];
	NumarkTotalControl.extendedLoopingType = { "None": 0, "SetBegin": 1, "SetLength": 2 };
	NumarkTotalControl.extendedLoopingState = [NumarkTotalControl.extendedLoopingType.None, NumarkTotalControl.extendedLoopingType.None];
	NumarkTotalControl.extendedLoopingChanged = [false, false];
	NumarkTotalControl.extendedLoopingLEDState = [false, false];
	NumarkTotalControl.extendedLoopingLEDTimer = [-1, -1];
	NumarkTotalControl.extendedLoopingJogCarryOver = [0, 0];
	
	NumarkTotalControl.quantizeLEDState = false;
	NumarkTotalControl.quantizeLEDTimer = -1;
	
	NumarkTotalControl.leds = [
		// Common
		{ "directory": 0x56, "simpleCue": 0x33, "scratchMode": 0x31, "extendedLooping": 0x44, "quantize": 0x45 },
		// Deck 1
		{ "rate": 0x34, "tap": 0x30, "loopIn": 0x3a, "loopOut": 0x3b, "loopHalve": 0x38, "loopDouble": 0x39 },
		// Deck 2
		{ "rate": 0x43, "tap": 0x47, "loopIn": 0x4a, "loopOut": 0x4b, "loopHalve": 0x48, "loopDouble": 0x49 }
	];
	
	// Doesn't work ?!?
	engine.softTakeover("[Channel1]", "rate", true);
	engine.softTakeover("[Channel2]", "rate", true);
	
	NumarkTotalControl.setLED(NumarkTotalControl.leds[1]["rate"], true);	// Turn on 0 rate lights
	NumarkTotalControl.setLED(NumarkTotalControl.leds[2]["rate"], true);	// Turn on 0 rate lights
	
	engine.connectControl("[Channel1]", "loop_enabled", "NumarkTotalControl.loopLEDs");
	engine.connectControl("[Channel2]", "loop_enabled", "NumarkTotalControl.loopLEDs");
	
	engine.connectControl("[Channel1]", "quantize", "NumarkTotalControl.quantizeLED");
	engine.connectControl("[Channel2]", "quantize", "NumarkTotalControl.quantizeLED");
}

NumarkTotalControl.shutdown = function(id) {	// called when the MIDI device is closed
	engine.connectControl("[Channel1]", "loop_enabled", "NumarkTotalControl.loopLEDs", true);
	engine.connectControl("[Channel2]", "loop_enabled", "NumarkTotalControl.loopLEDs", true);
	
	var lowestLED = 0x30;
	var highestLED = 0x56;
	for (var i=lowestLED; i<=highestLED; i++) {
		NumarkTotalControl.setLED(i, false);	// Turn off all the lights
	}
}

NumarkTotalControl.groupToDeck = function(group) {
	var matches = group.match(/^\[Channel(\d+)\]$/);
	if (matches == null) {
		return -1;
	} else {
		return matches[1];
	}
}

NumarkTotalControl.samplesPerBeat = function(group) {
	var sampleRate = engine.getValue(group, "track_samplerate");
	// FIXME: Get correct channel count for current deck
	var channels = 2;
	var bpm = engine.getValue(group, "file_bpm");
	return channels * sampleRate * 60 / bpm;
}

NumarkTotalControl.setLED = function(value, status) {
	if (status) {
		status = 0x64;
	} else {
		status = 0x00;
	}
	midi.sendShortMsg(0x90, value, status);
}

NumarkTotalControl.selectKnob = function(channel, control, value, status, group) {
	if (value > 63) {
		value = value - 128;
	}
	if (NumarkTotalControl.directoryMode) {
		if (value > 0) {
			for (var i = 0; i < value; i++) {
				engine.setValue(group, "SelectNextPlaylist", 1);
			}
		} else {
			for (var i = 0; i < -value; i++) {
				engine.setValue(group, "SelectPrevPlaylist", 1);
			}
		}
	} else {
		engine.setValue(group, "SelectTrackKnob", value);
	}
}

NumarkTotalControl.loopIn = function(channel, control, value, status, group) {
	if (value) {
		if (engine.getValue(group, "loop_enabled")) {
			engine.setValue(group, "reloop_exit", 1);
		}
		engine.setValue(group, "loop_in", 1);
		engine.setValue(group, "loop_end_position", -1);
	}
}

NumarkTotalControl.loopOut = function(channel, control, value, status, group) {
	if (value) {
		var start = engine.getValue(group, "loop_start_position");
		var end = engine.getValue(group, "loop_end_position");
		if (start != -1) {
			if (end != -1) {
				// Loop In and Out set -> call Reloop/Exit
				engine.setValue(group, "reloop_exit", 1);
			} else {
				// Loop In set -> call Loop Out
				if (NumarkTotalControl.extendedLooping) {
					var deck = NumarkTotalControl.groupToDeck(group);
					if (NumarkTotalControl.oldLoopStart[deck-1] == -1) {
						// Get current position by temporary setting loop start
						NumarkTotalControl.oldLoopStart[deck-1] = start;
						engine.setValue(group, "loop_in", 1);
						engine.beginTimer(20, "NumarkTotalControl.loopExtendedAdjustment('" + group + "')", true);
					}
				} else {
					engine.setValue(group, "loop_out", 1);
				}
			}
		}
	}
}

// Adjust loop length
NumarkTotalControl.loopExtendedAdjustment = function(group) {
	var deck = NumarkTotalControl.groupToDeck(group);
	// Check if temporary loop start is already set
	var start = engine.getValue(group, "loop_start_position");
	if (start == NumarkTotalControl.oldLoopStart[deck-1]) {
		// Still old loop start -> retry later
		engine.beginTimer(20, "NumarkTotalControl.loopExtendedAdjustment('" + group + "')", true);
		return;
	}
	
	// Restore loop start position
	var currentPosition = start;
	start = NumarkTotalControl.oldLoopStart[deck-1];
	engine.setValue(group, "loop_start_position", start);
	NumarkTotalControl.oldLoopStart[deck-1] = -1;
	var len = currentPosition - start;
	
	// Calculate nearest beat
	var beatSamples = NumarkTotalControl.samplesPerBeat(group);
	var lenInBeats = len / beatSamples;
	if (lenInBeats > 1) {
		//print("Full: " + Math.ceil(lenInBeats));
		//print("Mult2: " + (Math.ceil(lenInBeats / 2) * 2));
		//print("Pot2: " + Math.pow(2, Math.ceil(Math.log(lenInBeats) / Math.log(2))));
		
		// Round to full beats
		//lenInBeats = Math.ceil(lenInBeats);
		
		// Round to 2*x beats
		//lenInBeats = Math.ceil(lenInBeats / 2) * 2;
		
		// Round to 2^x beats
		lenInBeats = Math.pow(2, Math.ceil(Math.log(lenInBeats) / Math.log(2)));
	} else {
		//print("Full: 1 / " + Math.floor(1 / lenInBeats));
		//print("Mult2: 1 / " + (Math.floor(1 / lenInBeats / 2) * 2));
		//print("Pot2: 1 / " + Math.pow(2, Math.floor(Math.log(1 / lenInBeats) / Math.log(2))));
		
		// Round to beat fragments
		//lenInBeats = 1 / Math.floor(1 / lenInBeats);
		
		// Round to fragments of 2*x beats
		//lenInBeats = 1 / (Math.floor(1 / lenInBeats / 2) * 2);
		
		// Round to fragments of 2^x beats
		lenInBeats = 1 / Math.pow(2, Math.floor(Math.log(1 / lenInBeats) / Math.log(2)));
	}
	len = lenInBeats * beatSamples;
	
	// Set calculated loop end
	engine.setValue(group, "loop_end_position", start + len);
	
	// Start looping
	engine.setValue(group, "reloop_exit", 1);
}

// Activates alternative function
// Called by timer after button was 1sec pressed or by jog wheel movement
NumarkTotalControl.loopExtendedChange = function(group, timerCall) {
	var deck = NumarkTotalControl.groupToDeck(group);
	if (!NumarkTotalControl.extendedLoopingChanged[deck-1]) {
		if (!timerCall) {
			// Stop extended loop change timer
			engine.stopTimer(NumarkTotalControl.extendedLoopingLEDTimer[deck-1]);
		}
		NumarkTotalControl.extendedLoopingChanged[deck-1] = true;
		// Get current LED status
		NumarkTotalControl.extendedLoopingLEDState[deck-1] = engine.getValue(group, "loop_enabled");
		// Start LED blink timer
		NumarkTotalControl.loopLEDBlink(deck);
		NumarkTotalControl.extendedLoopingLEDTimer[deck-1] = engine.beginTimer(333, "NumarkTotalControl.loopLEDBlink(" + deck + ")");
	}
}

// Set LEDs to current loop status
NumarkTotalControl.loopLEDs = function(value, group, key) {
	var status = false;
	var deck = NumarkTotalControl.groupToDeck(group);
	if (value) {
		status = true;
	}
	NumarkTotalControl.setLED(NumarkTotalControl.leds[deck]["loopOut"], status);
	
	if (!NumarkTotalControl.extendedLooping) {
		status = false;
	}
	NumarkTotalControl.setLED(NumarkTotalControl.leds[deck]["loopHalve"], status);
	NumarkTotalControl.setLED(NumarkTotalControl.leds[deck]["loopDouble"], status);
}

// Let LED blink on alternative function
NumarkTotalControl.loopLEDBlink = function(deck) {
	var led;
	switch (NumarkTotalControl.extendedLoopingState[deck-1]) {
		case NumarkTotalControl.extendedLoopingType.SetBegin:
			led = NumarkTotalControl.leds[deck]["loopHalve"];
			break;
		case NumarkTotalControl.extendedLoopingType.SetLength:
			led = NumarkTotalControl.leds[deck]["loopDouble"];
			break;
		default:
			return;
	}
	if (NumarkTotalControl.extendedLoopingLEDState[deck-1]) {
		NumarkTotalControl.extendedLoopingLEDState[deck-1] = false;
	} else {
		NumarkTotalControl.extendedLoopingLEDState[deck-1] = true;
	}
	NumarkTotalControl.setLED(led, NumarkTotalControl.extendedLoopingLEDState[deck-1]);
}

NumarkTotalControl.extendedFunctionButton = function(normalFunction, extendedLoopingFactor, extendedLoopingType, group, value) {
	var deck = NumarkTotalControl.groupToDeck(group);
	if (NumarkTotalControl.extendedLooping) {
		var start = engine.getValue(group, "loop_start_position");
		var end = engine.getValue(group, "loop_end_position");
		if (value) {
			if ((start != -1) && (end != -1) && (NumarkTotalControl.extendedLoopingState[deck-1] == NumarkTotalControl.extendedLoopingType.None)) {
				NumarkTotalControl.extendedLoopingState[deck-1] = extendedLoopingType;
				NumarkTotalControl.extendedLoopingChanged[deck-1] = false;
				NumarkTotalControl.extendedLoopingJogCarryOver[deck-1] = 0;
				// Start alternative function timer -> activated after button was 500msec pressed or jog wheel movement (see jogWheel function)
				NumarkTotalControl.extendedLoopingLEDTimer[deck-1] = engine.beginTimer(500, "NumarkTotalControl.loopExtendedChange('" + group + "', true)", true);
			}
		} else {
			// Check if alternative function wasn't used
			if (!NumarkTotalControl.extendedLoopingChanged[deck-1] && (start != -1) && (end != -1)) {
				// Call default function
				engine.setValue(group, "loop_end_position", start + (end - start) * extendedLoopingFactor);
			}
			// Stop LED blink or extended loop change timer
			engine.stopTimer(NumarkTotalControl.extendedLoopingLEDTimer[deck-1]);
			NumarkTotalControl.extendedLoopingLEDTimer[deck-1] = -1;
			// Reset LEDs
			NumarkTotalControl.loopLEDs(engine.getValue(group, "loop_enabled"), group, "loop_enabled");
			// Reset alternative function variables
			NumarkTotalControl.extendedLoopingState[deck-1] = NumarkTotalControl.extendedLoopingType.None;
			NumarkTotalControl.extendedLoopingChanged[deck-1] = false;
			NumarkTotalControl.extendedLoopingJogCarryOver[deck-1] = 0;
		}
	} else {
		if (value) {
			engine.setValue(group, normalFunction, 1);
		} else {
			engine.setValue(group, normalFunction, 0);
		}
	}
}

NumarkTotalControl.leftFunction = function(channel, control, value, status, group) {
	NumarkTotalControl.extendedFunctionButton("rate_temp_down", 0.5, NumarkTotalControl.extendedLoopingType.SetBegin, group, value);
}

NumarkTotalControl.rightFunction = function(channel, control, value, status, group) {
	NumarkTotalControl.extendedFunctionButton("rate_temp_up", 2, NumarkTotalControl.extendedLoopingType.SetLength, group, value);
}

// Set LED to current quantize status
NumarkTotalControl.quantizeLED = function(value, group, key) {
	var deck1 = engine.getValue("[Channel1]", "quantize");
	var deck2 = engine.getValue("[Channel2]", "quantize");
	if (deck1 == deck2) {
		if (NumarkTotalControl.quantizeLEDTimer != -1) {
			engine.stopTimer(NumarkTotalControl.quantizeLEDTimer);
			NumarkTotalControl.quantizeLEDTimer = -1;
		}
		NumarkTotalControl.setLED(NumarkTotalControl.leds[0]["quantize"], deck1);
	} else if (NumarkTotalControl.quantizeLEDTimer == -1) {
		NumarkTotalControl.quantizeLEDBlink();
		NumarkTotalControl.quantizeLEDTimer = engine.beginTimer(333, "NumarkTotalControl.quantizeLEDBlink()");
	}
}

// Let LED blink on unequal quantize status
NumarkTotalControl.quantizeLEDBlink = function() {
	NumarkTotalControl.quantizeLEDStatus = !NumarkTotalControl.quantizeLEDStatus;
	NumarkTotalControl.setLED(NumarkTotalControl.leds[0]["quantize"], NumarkTotalControl.quantizeLEDStatus);
}

NumarkTotalControl.finePitch = function(channel, control, value, status, group) {
	if (value > 63) {
		value = value - 128;
	}
	engine.setValue(group, "rate", engine.getValue(group, "rate") + value / 512);
}

// Fixes cue_set glitches
NumarkTotalControl.setCue = function(channel, control, value, status, group) {
	if (value) {
		engine.setValue(group, "cue_set", 1);
	}
}

// If playing, stutters from cuepoint; otherwise jumps to cuepoint and stops
NumarkTotalControl.playFromCue = function(channel, control, value, status, group) {
	if (NumarkTotalControl.simpleCue) {
		if (value) {
			if (engine.getValue(group, "play")) {
				engine.setValue(group, "cue_goto", 1);
			} else {
				engine.setValue(group, "cue_gotoandstop", 1);
			}
		}
	} else {
		if (value) {
			if (engine.getValue(group, "play")) {
				engine.setValue(group, "play", 0);
				engine.setValue(group, "cue_gotoandstop", 1);
			} else {
				engine.setValue(group, "cue_preview", 1);
			}
		} else {
			engine.setValue(group, "cue_preview", 0);
		}
	}
}

// Jog values: (counter) fast slow still slow fast (clockwise)
// Jog values:            064  127   -   001  063
NumarkTotalControl.jogWheel = function(channel, control, value, status, group) {
	var deck = NumarkTotalControl.groupToDeck(group);
	var adjustedJog = parseFloat(value);
	var posNeg = 1;
	if (adjustedJog > 63) {	// Counter-clockwise
		posNeg = -1;
		adjustedJog = value - 128;
	}
	
	if (NumarkTotalControl.extendedLoopingState[deck-1] == NumarkTotalControl.extendedLoopingType.SetBegin) {
		var start = engine.getValue(group, "loop_start_position");
		var end = engine.getValue(group, "loop_end_position");
		if ((start != -1) && (end != -1)) {
			// Activate alternative function SetBegin
			NumarkTotalControl.loopExtendedChange(group, false);
			// Adjust jog speed
			// FIXME: Get correct channel count from deck
			var channels = 2;
			var sampleRate = engine.getValue(group, "track_samplerate");
			adjustedJog = adjustedJog * channels * sampleRate / 600;
			// Move loop
			engine.setValue(group, "loop_start_position", start + adjustedJog);
			engine.setValue(group, "loop_end_position", end + adjustedJog);
		}
	} else if (NumarkTotalControl.extendedLoopingState[deck-1] == NumarkTotalControl.extendedLoopingType.SetLength) {
		var start = engine.getValue(group, "loop_start_position");
		var end = engine.getValue(group, "loop_end_position");
		if ((start != -1) && (end != -1)) {
			// Activate alternative function SetLength
			NumarkTotalControl.loopExtendedChange(group, false);
			// Adjust jog speed and add remaining value from last jog change
			adjustedJog = adjustedJog / 40 + NumarkTotalControl.extendedLoopingJogCarryOver[deck-1];
			var beats;
			// Round to full beats
			if (adjustedJog > 0) {
				beats = Math.floor(adjustedJog);
			} else {
				beats = Math.ceil(adjustedJog);
			}
			// Save remaining value for next jog change
			NumarkTotalControl.extendedLoopingJogCarryOver[deck-1] = adjustedJog - beats;
			// Set new loop end
			engine.setValue(group, "loop_end_position", end + beats * NumarkTotalControl.samplesPerBeat(group));
		}
	} else if (NumarkTotalControl.scratchMode) {
		if (NumarkTotalControl.scratchTimer[deck-1] == -1) {
			engine.scratchEnable(deck, 128, 33+1/3, 1.0/8, (1.0/8)/32);
		} else {
			engine.stopTimer(NumarkTotalControl.scratchTimer[deck-1]);
		}
		engine.scratchTick(deck, adjustedJog);
		NumarkTotalControl.scratchTimer[deck-1] = engine.beginTimer(20, "NumarkTotalControl.jogWheelStopScratch(" + deck + ")", true);
	} else {
		var gammaInputRange = 64;	// Max jog speed
		var maxOutFraction = 0.5;	// Where on the curve it should peak; 0.5 is half-way
		var sensitivity = 0.5;		// Adjustment gamma
		var gammaOutputRange = 3;	// Max rate change
		if (engine.getValue(group,"play")) {
			adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
		} else {
			adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
		}
		engine.setValue(group, "jog", adjustedJog);
	}
}

NumarkTotalControl.jogWheelStopScratch = function(deck) {
	NumarkTotalControl.scratchTimer[deck-1] = -1;
	engine.scratchDisable(deck);
}

NumarkTotalControl.tap = function(channel, control, value, status, group) {
	var deck = NumarkTotalControl.groupToDeck(group);
	if (value) {
		NumarkTotalControl.setLED(NumarkTotalControl.leds[deck]["tap"], true);
		bpm.tapButton(deck);
	} else {
		NumarkTotalControl.setLED(NumarkTotalControl.leds[deck]["tap"], false);
	}
}

NumarkTotalControl.toggleDirectoryMode = function(channel, control, value, status, group) {
	// Toggle setting and light
	if (value) {
		NumarkTotalControl.directoryMode = !NumarkTotalControl.directoryMode;
		NumarkTotalControl.setLED(NumarkTotalControl.leds[0]["directory"], NumarkTotalControl.directoryMode);
	}
}

NumarkTotalControl.toggleScratchMode = function(channel, control, value, status, group) {
	// Toggle setting and light
	if (value) {
		NumarkTotalControl.scratchMode = !NumarkTotalControl.scratchMode;
		NumarkTotalControl.setLED(NumarkTotalControl.leds[0]["scratchMode"], NumarkTotalControl.scratchMode);
	}
}

NumarkTotalControl.toggleSimpleCue = function(channel, control, value, status, group) {
	// Toggle setting and light
	if (value) {
		NumarkTotalControl.simpleCue = !NumarkTotalControl.simpleCue;
		NumarkTotalControl.setLED(NumarkTotalControl.leds[0]["simpleCue"], NumarkTotalControl.simpleCue);
	}
}

NumarkTotalControl.toggleExtendedLooping = function(channel, control, value, status, group) {
	// Toggle setting and light
	if (value) {
		NumarkTotalControl.extendedLooping = !NumarkTotalControl.extendedLooping;
		NumarkTotalControl.setLED(NumarkTotalControl.leds[0]["extendedLooping"], NumarkTotalControl.extendedLooping);
		NumarkTotalControl.loopLEDs(engine.getValue("[Channel1]", "loop_enabled"), "[Channel1]", "loop_enabled");
		NumarkTotalControl.loopLEDs(engine.getValue("[Channel2]", "loop_enabled"), "[Channel2]", "loop_enabled");
	}
}

NumarkTotalControl.toggleQuantize = function(channel, control, value, status, group) {
	// Toggle setting
	if (value) {
		var newValue = !(engine.getValue("[Channel1]", "quantize") && engine.getValue("[Channel2]", "quantize"));
		engine.setValue("[Channel1]", "quantize", newValue);
		engine.setValue("[Channel2]", "quantize", newValue);
	}
}


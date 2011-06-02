// Numark Mixtrack Mapping Script Functions
// 1/11/2010 - v0.1
// Matteo <matteo@magm3.com>
function NumarkMixTrack() {}

NumarkMixTrack.init = function(id) {	// called when the MIDI device is opened & set up
	NumarkMixTrack.id = id;	// Store the ID of this device for later use
	
	NumarkMixTrack.directoryMode = false;
	
	NumarkMixTrack.scratchMode = [false, false];
	NumarkMixTrack.scratchTimer = [-1, -1];
	
	NumarkMixTrack.manualLooping = [false, false];
	
	NumarkMixTrack.leds = [
		// Common
		{ "directory": 0x73, "file": 0x72 },
		// Deck 1
		{ "rate": 0x70, "scratchMode": 0x48, "manualLoop": 0x61, "loopIn": 0x53, "loopOut": 0x54, "reLoop": 0x55 },
		// Deck 2
		{ "rate": 0x71, "scratchMode": 0x50, "manualLoop": 0x62, "loopIn": 0x56, "loopOut": 0x57, "reLoop": 0x58 }
	];
	NumarkMixTrack.setLED(NumarkMixTrack.leds[0]["file"], true);
}

NumarkMixTrack.shutdown = function(id) {	// called when the MIDI device is closed
	var lowestLED = 0x30;
	var highestLED = 0x73;
	for (var i=lowestLED; i<=highestLED; i++) {
		NumarkMixTrack.setLED(i, false);	// Turn off all the lights
	}
}

NumarkMixTrack.groupToDeck = function(group) {
	var matches = group.match(/^\[Channel(\d+)\]$/);
	if (matches == null) {
		return -1;
	} else {
		return matches[1];
	}
}

NumarkMixTrack.samplesPerBeat = function(group) {
	// FIXME: Get correct samplerate and channels for current deck
	var sampleRate = 44100;
	var channels = 2;
	var bpm = engine.getValue(group, "file_bpm");
	return channels * sampleRate * 60 / bpm;
}

NumarkMixTrack.setLED = function(value, status) {
	if (status) {
		status = 0x64;
	} else {
		status = 0x00;
	}
	midi.sendShortMsg(0x90, value, status);
}

NumarkMixTrack.selectKnob = function(channel, control, value, status, group) {
	if (value > 63) {
		value = value - 128;
	}
	if (NumarkMixTrack.directoryMode) {
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

NumarkMixTrack.loopIn = function(channel, control, value, status, group) {
        var deck = NumarkMixTrack.groupToDeck(group);
	if (value) {
		if(NumarkMixTrack.manualLooping[deck-1]) {
				// Cut loop to Half
				var start = engine.getValue(group, "loop_start_position");
				var end = engine.getValue(group, "loop_end_position");
				if((start != -1) && (end != -1)) {
					var len = (end - start) / 2;
					engine.setValue(group, "loop_end_position", start + len);
				}
		} else {
			if (engine.getValue(group, "loop_enabled")) {
				engine.setValue(group, "reloop_exit", 1);
				NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["reLoop"],false);
			}
			engine.setValue(group, "loop_in", 1);
			engine.setValue(group, "loop_end_position", -1);
			NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["loopIn"],true);
			NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["loopOut"],false);
		}
	}
}

NumarkMixTrack.loopOut = function(channel, control, value, status, group) {
        var deck = NumarkMixTrack.groupToDeck(group);
	if (value) {
		var start = engine.getValue(group, "loop_start_position");
		var end = engine.getValue(group, "loop_end_position");
		if(NumarkMixTrack.manualLooping[deck-1]) {
			// Set loop to current Bar (very approximative and would need to get fixed !!!)
			var bar = NumarkMixTrack.samplesPerBeat(group);
			engine.setValue(group,"loop_in",1);
			var start = engine.getValue(group, "loop_start_position");
			engine.setValue(group,"loop_end_position", start + bar);
			NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["loopIn"],true);
                        NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["loopOut"],true);
		} else {
			if (start != -1) {
				if (end != -1) {
					// Loop In and Out set -> call Reloop/Exit
					engine.setValue(group, "reloop_exit", 1);
					NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["loopIn"],true);
					NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["loopOut"],true);
					NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["reLoop"],true);
				} else {
					// Loop In set -> call Loop Out
					engine.setValue(group, "loop_out", 1);
					NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["loopOut"],true);
				}
			}
		}
	}
}

NumarkMixTrack.reLoop = function(channel, control, value, status, group) {
        var deck = NumarkMixTrack.groupToDeck(group);
	if (value) {
		if(NumarkMixTrack.manualLooping[deck-1]) {
				// Multiply Loop by Two 
				var start = engine.getValue(group, "loop_start_position");
				var end = engine.getValue(group, "loop_end_position");
				if((start != -1) && (end != -1)) {
					var len = (end - start) * 2;
					engine.setValue(group, "loop_end_position", start + len);
				}
		} else {
			if (engine.getValue(group, "loop_enabled")) {
				NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["reLoop"],false);
			} else {
				var start = engine.getValue(group, "loop_start_position");
				var end = engine.getValue(group, "loop_end_position");
				if( (start != -1) && (end != -1)) {
					// Loop is set ! Light the led
					NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["reLoop"],true);
				}
			}
			engine.setValue(group, "reloop_exit", 1);
		}
	}
}


// If playing, stutters from cuepoint; otherwise jumps to cuepoint and stops
NumarkMixTrack.playFromCue = function(channel, control, value, status, group) {
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

NumarkMixTrack.jogWheel = function(channel, control, value, status, group) {
	var deck = NumarkMixTrack.groupToDeck(group);
	var adjustedJog = parseFloat(value);
	var posNeg = 1;
	if (adjustedJog > 63) {	// Counter-clockwise
		posNeg = -1;
		adjustedJog = value - 128;
	}
	
        if (NumarkMixTrack.scratchMode[deck-1]) {
		if (NumarkMixTrack.scratchTimer[deck-1] == -1) {
			engine.scratchEnable(deck, 128, 33+1/3, 1.0/8, (1.0/8)/32);
		} else {
			engine.stopTimer(NumarkMixTrack.scratchTimer[deck-1]);
		}
		engine.scratchTick(deck, adjustedJog);
		NumarkMixTrack.scratchTimer[deck-1] = engine.beginTimer(20, "NumarkMixTrack.jogWheelStopScratch(" + deck + ")", true);
	} else {
		var gammaInputRange = 23;	// Max jog speed
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

NumarkMixTrack.jogWheelStopScratch = function(deck) {
	NumarkMixTrack.scratchTimer[deck-1] = -1;
	engine.scratchDisable(deck);
}

NumarkMixTrack.toggleDirectoryMode = function(channel, control, value, status, group) {
	// Toggle setting and light
	if (value) {
		if (NumarkMixTrack.directoryMode) {
			NumarkMixTrack.directoryMode = false;
		} else {
			NumarkMixTrack.directoryMode = true;
		}
		NumarkMixTrack.setLED(NumarkMixTrack.leds[0]["directory"], NumarkMixTrack.directoryMode);
		NumarkMixTrack.setLED(NumarkMixTrack.leds[0]["file"], !NumarkMixTrack.directoryMode);
	}
}

NumarkMixTrack.toggleScratchMode = function(channel, control, value, status, group) {
	var deck = NumarkMixTrack.groupToDeck(group);
	// Toggle setting and light
	if (value) {
		if (NumarkMixTrack.scratchMode[deck-1]) {
			NumarkMixTrack.scratchMode[deck-1] = false;
		} else {
			NumarkMixTrack.scratchMode[deck-1] = true;
		}
		NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["scratchMode"], NumarkMixTrack.scratchMode[deck-1]);
	}
}

NumarkMixTrack.toggleManualLooping = function(channel, control, value, status, group) {
	var deck = NumarkMixTrack.groupToDeck(group);
	// Toggle setting and light
	if (value) {
		if (NumarkMixTrack.manualLooping[deck-1]) {
			NumarkMixTrack.manualLooping[deck-1] = false;
		} else {
			NumarkMixTrack.manualLooping[deck-1] = true;
		}
		NumarkMixTrack.setLED(NumarkMixTrack.leds[deck]["manualLoop"], NumarkMixTrack.manualLooping[deck-1]);
	}
}

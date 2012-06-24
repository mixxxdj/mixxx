// Numark DJ2Go Mapping Script 2/05/12 by Jeff Mission <beatfixstudios@gmail.com>
// based on Numark MixTrack Script v0.1 1/11/2010 by Matteo <matteo@magm3.com>
function NumarkDJ2Go() {}

NumarkDJ2Go.init = function(id) {	// called when the MIDI device is opened & set up
	NumarkDJ2Go.id = id;	// Store the ID of this device for later use
	
	NumarkDJ2Go.directoryMode = false;
	
	NumarkDJ2Go.scratchMode = [false, false];
	NumarkDJ2Go.scratchTimer = [-1, -1];
	
	NumarkDJ2Go.manualLooping = [false, false];
	
	NumarkDJ2Go.leds = [
		// Common
		{ "directory": 0x73, "file": 0x72 },
		// Deck 1
		{ "rate": 0x70, "scratchMode": 0x48, "manualLoop": 0x61, "loopIn": 0x53, "loopOut": 0x54, "reLoop": 0x55 },
		// Deck 2
		{ "rate": 0x71, "scratchMode": 0x50, "manualLoop": 0x62, "loopIn": 0x56, "loopOut": 0x57, "reLoop": 0x58 }
	];
	NumarkDJ2Go.setLED(NumarkDJ2Go.leds[0]["file"], true);
}

NumarkDJ2Go.shutdown = function(id) {	// called when the MIDI device is closed
	var lowestLED = 0x30;
	var highestLED = 0x73;
	for (var i=lowestLED; i<=highestLED; i++) {
		NumarkDJ2Go.setLED(i, false);	// Turn off all the lights
	}
}

NumarkDJ2Go.groupToDeck = function(group) {
	var matches = group.match(/^\[Channel(\d+)\]$/);
	if (matches == null) {
		return -1;
	} else {
		return matches[1];
	}
}

NumarkDJ2Go.samplesPerBeat = function(group) {
	// FIXME: Get correct samplerate and channels for current deck
	var sampleRate = 44100;
	var channels = 2;
	var bpm = engine.getValue(group, "file_bpm");
	return channels * sampleRate * 60 / bpm;
}

NumarkDJ2Go.setLED = function(value, status) {
	if (status) {
		status = 0x64;
	} else {
		status = 0x00;
	}
	midi.sendShortMsg(0x90, value, status);
}

NumarkDJ2Go.selectKnob = function(channel, control, value, status, group) {
	if (value > 63) {
		value = value - 128;
	}
	if (NumarkDJ2Go.directoryMode) {
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


NumarkDJ2Go.jogWheel = function(channel, control, value, status, group) {
	var deck = NumarkDJ2Go.groupToDeck(group);
	var adjustedJog = parseFloat(value);
	var posNeg = 1;
	if (adjustedJog > 63) {	// Counter-clockwise
		posNeg = -1;
		adjustedJog = value - 128;
	}
	
        if (NumarkDJ2Go.scratchMode[deck-1]) {
		if (NumarkDJ2Go.scratchTimer[deck-1] == -1) {
			engine.scratchEnable(deck, 128, 33+1/3, 1.0/8, (1.0/8)/32);
		} else {
			engine.stopTimer(NumarkDJ2Go.scratchTimer[deck-1]);
		}
		engine.scratchTick(deck, adjustedJog);
		NumarkDJ2Go.scratchTimer[deck-1] = engine.beginTimer(20, "NumarkDJ2Go.jogWheelStopScratch(" + deck + ")", true);
	} else {
		var gammaInputRange = 12;	// Max jog speed
		var maxOutFraction = 0.5;	// Where on the curve it should peak; 0.5 is half-way
		var sensitivity = 0.9;		// .5 Adjustment gamma
		var gammaOutputRange = 3;	// Max rate change
		if (engine.getValue(group,"play")) {
			adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
		} else {
			adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
		}
		engine.setValue(group, "jog", adjustedJog);
	}
}

NumarkDJ2Go.jogWheelStopScratch = function(deck) {
	NumarkDJ2Go.scratchTimer[deck-1] = -1;
	engine.scratchDisable(deck);
}

NumarkDJ2Go.pitchBendDown = function(channel, control, value, status, group) {
	engine.setValue(group, "jog", -3.0);
}

NumarkDJ2Go.pitchBendUp = function(channel, control, value, status, group) {
	engine.setValue(group, "jog", 3.0);
}
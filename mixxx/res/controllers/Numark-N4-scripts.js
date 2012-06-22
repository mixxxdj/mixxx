NumarkN4 = new Controller();

NumarkN4.currentDeck = function (group) {
	if (group == "[Channel1]")
		return 0;
	else if (group == "[Channel2]")
		return 1;
	print("Invalid group : " + group);
	return -1; // error
}

NumarkN4.currentDeck2 = function (group) {
	if (group == "[Channel1]")
		return "[Channel1]";
	else if (group == "[Channel2]")
		return "[Channel2]";
	
	print("Invalid group : " + group);
	return -1; // error
}

NumarkN4.init = function(id) {	// called when the MIDI device is opened & set up
	NumarkN4.id = id;	// Store the ID of this device for later use
	midi.sendShortMsg(0xb0, 0x39, 0x01);
	//midi.sendShortMsg(0xB0, 0x39, 0x00);
	NumarkN4.directoryMode = false;
	
	NumarkN4.scratchMode = [false, false];
	NumarkN4.scratchTimer = [-1, -1];
	
	NumarkN4.manualLooping = [false, false];
	NumarkN4.touched = [false, false];
	
	NumarkN4.leds = [
		// Common
		{ "directory": 0x73, "file": 0x72 },
		// Deck 1
//0x11 = deck 3 light
//0x1e = deck 1 triangle light
//0x1f = deck 2 triangle light
//0x20 = deck 3 triangle light
//0x28 = deck 4 light
//0x39 = all lights around the outside
//0x40 = headphone cue button 1
//0x41 = headphone cue button 2
//0x42 = headphone cue button 3
//0x44 = vid fade 1 deck 1
//0x45 = vid fade 1 deck 2
//0x46 = vid fade 2 deck 1
//0x47 = vid fade 2 deck 2
//0x48 = vid fade 3 deck 1
//0x49 = vid fade 3 deck 2
//0x4a = vid fade 4 deck 1
//0x4b = vid fade 4 deck 2
//0x50 = deck 1/3
//0x51 = deck 2/4
//0x52 = 

		{ "rate": 0x70, "scratchMode": 0x39, "manualLoop": 0x61, "loopIn": 0x53, "loopOut": 0x54, "reLoop": 0x55 },
		// Deck 2
		{ "rate": 0x71, "scratchMode": 0x30, "manualLoop": 0x62, "loopIn": 0x56, "loopOut": 0x57, "reLoop": 0x58 }
	];
	NumarkN4.setLED(NumarkN4.leds[0]["file"], true);
}

NumarkN4.setLED = function(value, status) {
	if (status) {
		status = 0x64;
	} else {
		status = 0x00;
	}
	midi.sendShortMsg(0xB0, value, status);
}

NumarkN4.light = function(channel, control, value, status, group) {

midi.sendShortMsg(0xbf, value, 0x7f);

}

NumarkN4.scratch_sensitivity = 80;

NumarkN4.jogWheel = function(channel, control, value, status, group) {
	var deck = NumarkN4.groupToDeck(group);
	var adjustedJog = parseFloat(value);
	var posNeg = 1;
	if (adjustedJog > 63) {	// Counter-clockwise
		posNeg = -1;
		adjustedJog = value - 128;
	}
	
        if (NumarkN4.scratchMode[deck-1]) {
		if (NumarkN4.scratchTimer[deck-1] == -1) {
			engine.scratchEnable(deck, 128, 33+1/3, 1.0/8, (1.0/8)/32);
		} else {
			engine.stopTimer(NumarkN4.scratchTimer[deck-1]);
		}
		engine.scratchTick(deck, adjustedJog);
		NumarkN4.scratchTimer[deck-1] = engine.beginTimer(20, "NumarkN4.jogWheelStopScratch(" + deck + ")", true);
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

NumarkN4.jogWheelStopScratch = function(deck) {
	NumarkN4.scratchTimer[deck-1] = -1;
	engine.scratchDisable(deck);
}

NumarkN4.toggleScratchMode = function(channel, control, value, status, group) {
	var deck = NumarkN4.groupToDeck(group);
	// Toggle setting and light
	if (value) {
		if (NumarkN4.scratchMode[deck-1]) {
			NumarkN4.scratchMode[deck-1] = false;
		} else {
			NumarkN4.scratchMode[deck-1] = true;
		}
		
		//NumarkN4.setLED(NumarkN4.leds[deck]["scratchMode"], NumarkN4.scratchMode[deck-1]);
	}

}


NumarkN4.groupToDeck = function(group) {
	var matches = group.match(/^\[Channel(\d+)\]$/);
	if (matches == null) {
		return -1;
	} else {
		return matches[1];
	}
}

NumarkN4.SelectTrack = function(channel, control, value, status, group) {
	if (value == 0x01) {
		engine.setValue("[Playlist]","SelectNextTrack",1);
	}
	if (value == 0x7F) {
		engine.setValue("[Playlist]","SelectPrevTrack",1);
	}
}

NumarkN4.menuNext = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		engine.setValue("[Playlist]","SelectNextPlaylist",1);
	}

}
NumarkN4.menuPrev = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		engine.setValue("[Playlist]","SelectPrevPlaylist",1);
	}
}

NumarkN4.cue = function (channel, control, value, status, group) {
	var currentlyPlaying = engine.getValue(NumarkN4.currentDeck2(group),"play");
	if ((currentlyPlaying == 1) & (value == 0x7F)) {
		engine.setValue(NumarkN4.currentDeck2(group),"cue_default",1);
	}
	else if ((currentlyPlaying == 0) & (value == 0x7F)) {
		engine.setValue(NumarkN4.currentDeck2(group),"cue_default",1);
	}
	else {
		engine.setValue(NumarkN4.currentDeck2(group),"cue_default",0);
	}
}

NumarkN4.touch = function (channel, control, value, status, group) {
var deck = NumarkN4.groupToDeck(group);
var currentlyPlaying = engine.getValue(NumarkN4.currentDeck2(group),"play");
	if ((currentlyPlaying == 1) & (value == 0x7F & (NumarkN4.touched[deck-1] == false))) {
		engine.setValue(NumarkN4.currentDeck2(group),"play",0);
		NumarkN4.touched[deck-1] = true;
	}
	if ((currentlyPlaying == 0) & (value == 0x00) & (NumarkN4.touched[deck-1] == true)) {
		engine.setValue(NumarkN4.currentDeck2(group),"play",1);
		NumarkN4.touched[deck-1] = false;
	}
}

//0x40 = headphone cue button 1
//0x41 = headphone cue button 2
NumarkN4.headcue = function (channel, control, value, status, group) {
var deck = NumarkN4.groupToDeck(group);
//head 1 on
if ((status == 0x90) & (control == 0x31)){
midi.sendShortMsg(0xB0, 0x41, 0x00);
engine.setValue(NumarkN4.currentDeck2(group),"pfl",1);
}
//head 1 off
if ((status == 0x80) & (control == 0x31)){
engine.setValue(NumarkN4.currentDeck2(group),"pfl",0);
}
//head 2
if ((status == 0x90) & (control == 0x32)){
midi.sendShortMsg(0xB0, 0x40, 0x00);
engine.setValue(NumarkN4.currentDeck2(group),"pfl",1);
}
//head 2 off
if ((status == 0x80) & (control == 0x32)){
engine.setValue(NumarkN4.currentDeck2(group),"pfl",0);
}
}

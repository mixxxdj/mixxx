function HerculesMP3e2 () {}

// Control schema: http://blog.ebruni.it/blog/wp-content/uploads/2010/01/Hercules-mp3e2-schema-comandi.jpg

// Image: http://www.hablarcom.com.br/Imagens/arquivos/jptech/DJ_control_MP3_2_frente.jpg

// Explaination: http://www.mixxx.org/wiki/doku.php/hercules_dj_control_mp3_e2



// Number of the standard RPM value. Lower values increase de sensitivity as the really records.
standardRpm = 33.33;

// The alpha value for the filter (start with 1/8 (0.125) and tune from there)
alpha = 1/8;

// The beta value for the filter (start with alpha/32 and tune from there)
beta = alpha/20;

// Timer to disable the scratch if the "jog wheel" is stopped for "x" milliseconds (default = 60)
scratchResetTime = 60;

// Seconds to the end of track after which cue button blink (default = 30)
secondsBlink = 30;  

// Tune the jog sensitivity when the scratch mode is disabled (default = 1, increase for increase the sensitivity
jogSensitivity = 0.8;



superButtonHold = 0;
scratchButton = 0;
scratchMode = 0;
scratchTimer = 0;
wheelMove = [0,0];
pitchIncrementRelative = 0;
//scratchFactor = 0;
//jogPitchFactor = 0;

/*HerculesMP3e2.controls = {
    "inputs": {
	0x11: { "channel": 1, "name": "loadA", 		"type": "button" },
	0x25: { "channel": 2, "name": "loadB", 		"type": "button" },
	0x0B: { "channel": 1, "name": "pitchbend+", 	"type": "button" },
	0x0A: { "channel": 1, "name": "pitchbend-", 	"type": "button" },
	0x1F: { "channel": 2, "name": "pitchbend+", 	"type": "button" },
	0x1E: { "channel": 2, "name": "pitchbend-", 	"type": "button" },
	0x12: { "channel": 1, "name": "sync", 			"type": "button" },
	0x26: { "channel": 2, "name": "sync", 			"type": "button" },
	0x13: { "channel": 1, "name": "mastertempo", "type": "button" },
	0x27: { "channel": 2, "name": "mastertempo", "type": "button" },
	0x0F: { "channel": 1, "name": "play", 			"type": "button" },
	0x23: { "channel": 2, "name": "play", 			"type": "button" },
	0x0E: { "channel": 1, "name": "cue", 			"type": "button" },
	0x22: { "channel": 2, "name": "cue", 			"type": "button" },
	0x2D: { "channel": 1, "name": "scratch", 		"type": "button" },
	0x2E: { "channel": 1, "name": "automix", 		"type": "button" },
	0x01: { "channel": 1, "name": "K1", 			"type": "button" },
	0x02: { "channel": 1, "name": "K2", 			"type": "button" },
	0x03: { "channel": 1, "name": "K3", 			"type": "button" },
	0x04: { "channel": 1, "name": "K4", 			"type": "button" },
	0x05: { "channel": 1, "name": "K5", 			"type": "button" },
	0x06: { "channel": 1, "name": "K6", 			"type": "button" },
	0x07: { "channel": 1, "name": "K7", 			"type": "button" },
	0x08: { "channel": 1, "name": "K8", 			"type": "button" },
	0x15: { "channel": 2, "name": "K1", 			"type": "button" },
	0x16: { "channel": 2, "name": "K2", 			"type": "button" },
	0x17: { "channel": 2, "name": "K3", 			"type": "button" },
	0x18: { "channel": 2, "name": "K4", 			"type": "button" },
	0x19: { "channel": 2, "name": "K5", 			"type": "button" },
	0x1A: { "channel": 2, "name": "K6", 			"type": "button" },
	0x1B: { "channel": 2, "name": "K7", 			"type": "button" },
	0x1C: { "channel": 2, "name": "K8", 			"type": "button" },
	0x30: { "channel": 1, "name": "wheel", 		"type": "pot" },
	0x31: { "channel": 2, "name": "wheel", 		"type": "pot" },
	0x2C: { "channel": 1, "name": "folder", 		"type": "button" },
	0x2B: { "channel": 1, "name": "files", 		"type": "button" },
    },
}; */

HerculesMP3e2.init = function (id) 
{ 
	// Switch off all LEDs
	for (i=1; i<95; i++) 
	{
		midi.sendShortMsg(0x90, i, 0x00);
	}
	
	midi.sendShortMsg(0xB0,0x7F,0x7F);
	
	// Switch-on some LEDs for improve the usability
	midi.sendShortMsg(0x90, 46, 0x7F);	// Automix LED
	midi.sendShortMsg(0x90, 14, 0x7F);	// Cue deck A LED
	midi.sendShortMsg(0x90, 34, 0x7F);	// Cue deck B LED
	
	engine.connectControl("[Channel1]", "playposition", "HerculesMP3e2.playPositionCue");
	engine.connectControl("[Channel2]", "playposition", "HerculesMP3e2.playPositionCue");
	engine.connectControl("[Channel1]", "loop_start_position", "HerculesMP3e2.loopStartSetLeds");
	engine.connectControl("[Channel2]", "loop_start_position", "HerculesMP3e2.loopStartSetLeds");
	engine.connectControl("[Channel1]", "loop_end_position", "HerculesMP3e2.loopEndSetLeds");
	engine.connectControl("[Channel2]", "loop_end_position", "HerculesMP3e2.loopEndSetLeds");
};


HerculesMP3e2.shutdown = function (id) 
{
	// Switch off all LEDs
	for (i=1; i<95; i++)
	{
		midi.sendShortMsg(0x90, i, 0x00);
	}
};

HerculesMP3e2.automix = function (midino, control, value, status, group) 
{
	// SHIFT BUTTON	
	// The "Automix" button is used like a shift button. When this is hold
	//	down, many commands of the console has another functions
	
	// Button pressed
	if (value) 
	{
		superButtonHold = 1;
		// Switch-on some LEDs
		midi.sendShortMsg(0x90, 30, 0x7F);	// Pitchbend - DB
		midi.sendShortMsg(0x90, 31, 0x7F);  // Pitchbend + DB
		midi.sendShortMsg(0x90, 10, 0x7F);  // Pitchbend - DA
		midi.sendShortMsg(0x90, 11, 0x7F);  // Pitchbend + DA
		midi.sendShortMsg(0x90, 19, 0x7F);	// Master tempo DA 
		midi.sendShortMsg(0x90, 39, 0x7F);  // Master tempo DB
	}
	// Button released
	else
	{
		superButtonHold = 0;
		// Switch-off some LEDs	
		midi.sendShortMsg(0x90, 30, 0x00);  // Pitchbend - DB
		midi.sendShortMsg(0x90, 31, 0x00);  // Pitchbend + DB
		midi.sendShortMsg(0x90, 10, 0x00);  // Pitchbend - DA
		midi.sendShortMsg(0x90, 11, 0x00);  // Pitchbend + DA
		midi.sendShortMsg(0x90, 19, 0x00);	// Master tempo DA 
		midi.sendShortMsg(0x90, 39, 0x00);  // Master tempo DB
	}
};


// Enable/disable the flanger effect or enable/disable the keylock tempo if shifted
HerculesMP3e2.masterTempo = function (midino, control, value, status, group) 
{
	if (superButtonHold == 1 && value && scratchMode == 0)
	{
	engine.setValue(group, "keylock", (engine.getValue(group, "keylock") == 0) ? 1 : 0);
	}
	if (superButtonHold == 0 && value)
	{
	engine.setValue(group, "flanger", (engine.getValue(group, "flanger") == 0) ? 1 : 0);
	}
};

HerculesMP3e2.loadTrack = function (midino, control, value, status, group) 
{
	// Load the selected track in the corresponding deck only if the track is 
	// paused

	if(value && engine.getValue(group, "play") != 1) 
	{
		engine.setValue(group, "LoadSelectedTrack", 1);
		engine.setValue(group, "rate", 0);
	}
	else engine.setValue(group, "LoadSelectedTrack", 0);
};

HerculesMP3e2.scroll = function (midino, control, value, status, group) 
{
	// Button "Files": up 10 tracks
	// Button "Folder": down 10 tracks
	// This function scroll up or down 10 tracks on the playlist, like the mouse
	// scroll.

	if(control == 0x2C && value == 0x7F) 
	{
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "1");
		engine.setValue("[Playlist]", "SelectPrevTrack", "0");
	}
	if (control == 0x2B && value == 0x7F) 
	{
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "1");
		engine.setValue("[Playlist]", "SelectNextTrack", "0");
	}
};

/*
// NOT USED

HerculesMP3e2.holdTimer = function (group, first, second) {

	holdButtonFlag = 1;
	engine.setValue(group, first, 0);	// Set "Off" the first function
	engine.setValue(group, second, 1);	// Set "On" the second function
};


HerculesMP3e2.holdButton = function (group, value, first, second) {
	// This feature allows you to perform a different function if a button is 
	// pressed for 2 seconds. When the button is pressed, the first function is
	// performed. If the button is hold down for 2 seconds, the second function
	// is performed and the first function is disables.
	
	if (value) {
		engine.setValue(group, first, 1);	// Set "On" the first function
		holdTimerID = engine.beginTimer(2000, "HerculesMP3e2.holdTimer(\""+group+"\", \""+first+"\", \""+second+"\")", true);
		}
	else {
		if (holdButtonFlag) {
			engine.setValue(group, second, 0);	// Set "Off" the second function
			holdButtonFlag = 0;
		}
		else {
			engine.stopTimer(holdTimerID);
			engine.setValue(group, first, 0);	// Set "Off" the first function
		}
	}
}; 

*/

HerculesMP3e2.keyButton = function (midino, control, value, status, group) 
{
	// Loop command for the first 4 Key, Hotcues command for the latest 4
	
	switch (control) 
	{
		// Loop buttons
		case 0x01: case 0x15:  	// K1, Loop in
			if (superButtonHold == 1 && value) 
			{
			engine.setValue(group, "loop_start_position", -1);
			engine.setValue(group, "loop_end_position", -1);
			}
			else engine.setValue(group, "loop_in", value ? 1 : 0);
			break;
		case 0x02: case 0x16:	// K2, Loop out
			if (superButtonHold == 1 && value) 
			{
			engine.setValue(group, "loop_start_position", -1);
			engine.setValue(group, "loop_end_position", -1);
			}
			else engine.setValue(group, "loop_out", value ? 1 : 0);
			break;
		case 0x03: case 0x17:	// K3, Reloop/Exit
			engine.setValue(group, "reloop_exit", value ? 1 : 0); break;
			break;
		case 0x04: case 0x18:	// K4, Reloop/Exit
			engine.setValue(group, "reloop_exit", value ? 1 : 0);
			break;

		// Hotcue buttons:
		// Simple press: go to the hotcue position
		// Shift (hold down "Automix"): clear the hotcue
		case 0x05: case 0x19 :	// K5
			if (superButtonHold == 1) 
			{
				//HerculesMP3e2.holdButton(group, value, "hotcue_1_set", "hotcue_1_clear");
				engine.setValue(group, "hotcue_1_clear", value ? 1 : 0);
			}
			else 
			{
				engine.setValue(group, "hotcue_1_activate", value ? 1 : 0);
			}
			break;

		case 0x06: case 0x1A:	// K6
			if (superButtonHold == 1) 
			{
				//HerculesMP3e2.holdButton(group, value, "hotcue_2_set", "hotcue_2_clear");
				engine.setValue(group, "hotcue_2_clear", value ? 1 : 0);
			}
			else
			{
				engine.setValue(group, "hotcue_2_activate", value ? 1 : 0);
			}
			break;

		case 0x07: case 0x1B:	// K7
			if (superButtonHold == 1) 
			{
				//HerculesMP3e2.holdButton(group, value, "hotcue_3_set", "hotcue_3_clear");
				engine.setValue(group, "hotcue_3_clear", value ? 1 : 0);
			}
			else
			{
				engine.setValue(group, "hotcue_3_activate", value ? 1 : 0);
			}
			break;

		case 0x08: case 0x1C:	// K8
			if (superButtonHold == 1) 
			{
				//HerculesMP3e2.holdButton(group, value, "hotcue_4_set", "hotcue_4_clear");
				engine.setValue(group, "hotcue_4_clear", value ? 1 : 0);
			}
			else
			{
				engine.setValue(group, "hotcue_4_activate", value ? 1 : 0);
			}
			break;
		}
};

HerculesMP3e2.knobIncrement = function (group, action, minValue, maxValue, centralValue, step, sign) 
{
	// This function allows you to increment a non-linear value like the volume's knob
	// sign must be 1 for positive increment, -1 for negative increment
	semiStep = step/2;
	rangeWidthLeft = centralValue-minValue;
	rangeWidthRight = maxValue-centralValue;
	actual = engine.getValue(group, action);
	
	if (actual < 1) 
	{
		increment = ((rangeWidthLeft)/semiStep)*sign;
	}
	else if (actual > 1) 
	{
		increment = ((rangeWidthRight)/semiStep)*sign;
	}
	else if (actual == 1) 
	{
		increment = (sign == 1) ? rangeWidthRight/semiStep : (rangeWidthLeft/semiStep)*sign;
	}

	if (sign == 1 && actual < maxValue)
	{
		newValue = actual + increment;
	}
	else if (sign == -1 && actual > minValue)
	{
		newValue = actual + increment;
	}
	
	return newValue;
};

HerculesMP3e2.pitch = function (midino, control, value, status, group) 
{
	// Simple: pitch slider
	// Shifted: Headphone volume and pre/main
	
	if (superButtonHold == 1) 
	{
		sign = (value == 0x01) ? 1 : -1;
		
		if (group == "[Channel1]") 
		{
			newValue = HerculesMP3e2.knobIncrement("[Master]", "headVolume", 0, 5, 1, 30, sign);
			engine.setValue("[Master]", "headVolume", newValue);
		}
		if (group == "[Channel2]") 
		{
			newValue = HerculesMP3e2.knobIncrement("[Master]", "headMix", -1, 1, 0, 20, sign);
			engine.setValue("[Master]", "headMix", newValue);
		}
	}
	else
	{
		engine.setValue(group, (value==1) ? "rate_perm_up" : "rate_perm_down", 1);
		engine.setValue(group, (value==1) ? "rate_perm_up" : "rate_perm_down", 0);
	}
	
};

HerculesMP3e2.pitchbend = function (midino, control, value, status, group) 
{
	// Simple: temporary pitch adjust
	// Shift:  pregain adjust
	if (superButtonHold == 1 && value) 
	{
		// Pitchbend +
		if (control == 0x0B || control == 0x1F) 
		{
			newValue = HerculesMP3e2.knobIncrement(group, "pregain", 0, 4, 1, 20, 1);
			engine.setValue(group, "pregain", newValue);
		}
		// Pitchbend -
		else 
		{
			newValue = HerculesMP3e2.knobIncrement(group, "pregain", 0, 4, 1, 20, -1);
			engine.setValue(group, "pregain", newValue);
		}
	}
	else
	{
		// Pitchbend +
		if (control == 0x0B || control == 0x1F) 
		{
			engine.setValue(group, "rate_temp_up", value ? 1 : 0);
		}
		// Pitchbend -
		else 
		{
			engine.setValue(group, "rate_temp_down", value ? 1 : 0);
		}
	}
};

		
HerculesMP3e2.cue = function (midino, control, value, status, group) 
{
	// Don't set Cue accidentaly at the end of the song
	if(engine.getValue(group, "playposition") <= 0.97) 
	{
		engine.setValue(group, "cue_default", value ? 1 : 0);
	}
	else
	{
		engine.setValue(group, "cue_preview", value ? 1 : 0);
	}
};

HerculesMP3e2.scratch = function (midino, control, value, status, group) 
{
	if (value) 
	{
		if(scratchMode == 0) 
		{
		// Enable the scratch mode on the corrisponding deck and start the timer
			scratchMode = 1;
			scratchTimer = engine.beginTimer(scratchResetTime, "HerculesMP3e2.wheelOnOff()");	
			midi.sendShortMsg(0x90, 45, 0x7F); // Switch-on the sync led
			engine.setValue("[Channel1]", "keylock", 0);
			engine.setValue("[Channel2]", "keylock", 0);
		
		}
		else 
		{
		// Disable the scratch mode on the corrisponding deck and stop the timer
			scratchMode = 0;
			engine.stopTimer(scratchTimer);
			midi.sendShortMsg(0x90, 45, 0x00); // Switch-off the sync led
		}
	
	}
};

HerculesMP3e2.sync = function (midino, control, value, status, group) 
{
		engine.setValue(group, "beatsync", value ? 1 : 0);
};


// This function is called every "scratchResetTime" seconds and checks if the wheel was moved in the previous interval 
// (every interval last "scratchResetTime" seconds). If the wheel was moved enables the scratch mode, else disables it.
// In this way I have made a simple workaround to simulate the touch-sensitivity of the other controllers.

HerculesMP3e2.wheelOnOff = function () {
	
	// Wheel Deck A
	if (wheelMove[0]) engine.scratchEnable(1, 128, standardRpm, alpha, beta);
	else engine.scratchDisable(1);
	wheelMove[0] = 0;
	//Wheel Deck B
	if (wheelMove[1]) engine.scratchEnable(2, 128, standardRpm, alpha, beta);
	else engine.scratchDisable(2);
	wheelMove[1] = 0;
};


HerculesMP3e2.jogWheel = function (midino, control, value, status, group) 
{
	var deck = (group == "[Channel1]") ? 1 : 2;
	
	// This function is called everytime the jog is moved
	if (value == 0x01) 
	{
		if (scratchMode) {
			engine.scratchTick(deck, 1);
			wheelMove[deck-1] = 1;
		}
		else
			engine.setValue(group, "jog", jogSensitivity);
	}
	else 
	{
		if (scratchMode) {
			engine.scratchTick(deck, -1);
			wheelMove[deck-1] = 1;
		}
		else
			engine.setValue(group, "jog", -jogSensitivity); 
	}
};


// This function switch-on the blinking of the cue led when the track is going to end and switch off the led 
// when the track is ended
HerculesMP3e2.playPositionCue = function (playposition, group) {
	
    var secondsToEnd = engine.getValue(group, "duration") * (1-playposition);
	
	if (secondsToEnd > secondsBlink) { 
		if (group == "[Channel1]") {
			midi.sendShortMsg(0x90,14,0x7F); // Switch-on Cue Led
			midi.sendShortMsg(0x90,62,0x00); // Switch-off  Cue Blink
		}
		else {
			midi.sendShortMsg(0x90,34,0x7F);
			midi.sendShortMsg(0x90,82,0x00);
		}
		
	}

	if (secondsToEnd < secondsBlink && secondsToEnd > 1) { // The song is going to end
		if (group == "[Channel1]") {
			midi.sendShortMsg(0x90,14,0x00);  // Switch-off Cue Led
			midi.sendShortMsg(0x90,62,0x7F);  // Switch-on  Cue Blink
		}
		else {
			midi.sendShortMsg(0x90,34,0x00);
			midi.sendShortMsg(0x90,82,0x7F);
		}
	}
	
	if (secondsToEnd < 1) { // The song is finished
		if (group == "[Channel1]") {
			midi.sendShortMsg(0x90,14,0x00); // Switch-off Cue Led and blink
			midi.sendShortMsg(0x90,62,0x00);
		}
		else {
			midi.sendShortMsg(0x90,34,0x00);
			midi.sendShortMsg(0x90,82,0x00);
		}
	}
		

};

// Switch-on the K1 Led if the loop start is set
HerculesMP3e2.loopStartSetLeds = function (loopStartPos, group) 
{
	if (group == "[Channel1]") 
	{
		if (loopStartPos != -1) midi.sendShortMsg(0x90,1,0x7F);
		else midi.sendShortMsg(0x90,1,0x00);
	}
	else	
	{
		if (loopStartPos != -1) midi.sendShortMsg(0x90,21,0x7F);
		else midi.sendShortMsg(0x90,21,0x00);
	}
}

// Switch-on the K2 Led if the loop end is set
HerculesMP3e2.loopEndSetLeds = function (loopEndPos, group) 
{
	if (group == "[Channel1]") 
	{
		if (loopEndPos != -1) midi.sendShortMsg(0x90,2,0x7F);
		else midi.sendShortMsg(0x90,2,0x00);
	}
	else	
	{
		if (loopEndPos != -1) midi.sendShortMsg(0x90,22,0x7F);
		else midi.sendShortMsg(0x90,22,0x00);
	}
}








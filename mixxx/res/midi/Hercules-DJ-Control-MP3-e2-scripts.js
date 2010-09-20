function HerculesMP3e2 () {}

// Control schema: http://blog.ebruni.it/blog/wp-content/uploads/2010/01/Hercules-mp3e2-schema-comandi.jpg

// Image: http://www.hablarcom.com.br/Imagens/arquivos/jptech/DJ_control_MP3_2_frente.jpg



// Number of the standard RPM value. Lower values increase de sensitivity as the really records.
standardRpm = 33.33;

// The alpha value for the filter (start with 1/8 (0.125) and tune from there)
alpha = 1/8;

// The beta value for the filter (start with alpha/32 and tune from there)
beta = alpha/20;


superButtonHold = 0;
//holdButtonFlag = 0;
//holdTimerID = 0;
scratchButton = 0;
scratchMode = [ 0, 0 ];
pitchShiftIncrement = 0.01;
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
	
	// Switch-on the LEDs for improve the usability
	midi.sendShortMsg(0x90, 45, 0x7F);	// Scratch LED
	midi.sendShortMsg(0x90, 46, 0x7F);	// Automix LED
	midi.sendShortMsg(0x90, 14, 0x7F);	// Cue deck A LED
	midi.sendShortMsg(0x90, 34, 0x7F);	// Cue deck B LED
	
	HerculesMP3e2.setPitchIncrement();
	engine.connectControl("[Channel1]","rateRange",
	                "HerculesMP3e2.setPitchIncrement");
};

HerculesMP3e2.setPitchIncrement = function()
{
    var rRange = engine.getValue("[Channel1]", "rateRange");
    var range = Math.round(rRange * 100);
	if (( range % 10 ) && (range != 8))
		range += ( range - (range % 10 ));
	
    pitchShiftIncrement = 0.1 / range;
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
		midi.sendShortMsg(0x90, 30, 0x7F);	// Switch on some LEDs
		midi.sendShortMsg(0x90, 31, 0x7F);
		midi.sendShortMsg(0x90, 10, 0x7F);
		midi.sendShortMsg(0x90, 11, 0x7F);
	}
	// Button released
	else
	{
		superButtonHold = 0;		
		midi.sendShortMsg(0x90, 30, 0x00);
		midi.sendShortMsg(0x90, 31, 0x00);
		midi.sendShortMsg(0x90, 10, 0x00);
		midi.sendShortMsg(0x90, 11, 0x00);
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
			engine.setValue(group, "loop_in", value ? 1 : 0);
			break;
		case 0x02: case 0x16:	// K2, Loop out
			engine.setValue(group, "loop_out", value ? 1 : 0);
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
		//increment = 0.0166;
		print("Pitch Shift Increment = " + pitchShiftIncrement);
		increment = pitchShiftIncrement;
		increment = (value == 0x01) ? increment : increment * -1;
		engine.setValue(group, "rate", engine.getValue(group, "rate") + increment);
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
	// The "scratch" button is used like a shift button only for enable the scratch mode on the 
	// deck A and/or B with the "sync" buttons
	if (value) 
	{
		scratchButton = 1;
		//Switch-on the LEDs of Sync buttons while the "scratch" button is hold down
		midi.sendShortMsg(0x90, 18, 0x7F); 
		midi.sendShortMsg(0x90, 38, 0x7F);
	}
	else 
	{
		scratchButton = 0;
		midi.sendShortMsg(0x90, 18, 0x00);
		midi.sendShortMsg(0x90, 38, 0x00);
	}
};

HerculesMP3e2.sync = function (midino, control, value, status, group) 
{
	// If the "scratch" button is hold down
	if (scratchButton && value) 
	{
		deck = (group == "[Channel1]") ? 1 : 2;
		
		print("DECK = " + deck + " ScratchMode = " + scratchMode[(deck-1)]);
		
		if(scratchMode[(deck-1)]==0) 
		{
			engine.scratchEnable(deck, 128, standardRpm, alpha, beta); // Enable the scratch mode on Deck A
			scratchMode[(deck-1)] = 1;
			midi.sendShortMsg(0x90, (deck == 1 ? 66 : 86), 0x7F); // Blinks the sync led
		}
		else 
		{
			engine.scratchDisable(deck);
			scratchMode[(deck-1)] = 0;
			midi.sendShortMsg(0x90, (deck == 1 ? 66 : 86), 0x00); // Switch-off the sync led
		}
		
	}
	else
	{
	        engine.setValue(group, "beatsync", value ? 1 : 0);
	}
};

HerculesMP3e2.jogWheel = function (midino, control, value, status, group) 
{
        var deck = (group == "[Channel1]") ? 1 : 2;
        
        
	// This function is called everytime the jog is moved
	if (value == 0x01) 
	{
		engine.scratchTick(deck, 1);
		engine.setValue(group, "jog", 1);
	}
	else 
	{
		engine.scratchTick(deck, -1);
		engine.setValue(group, "jog", -1); 
	}
};

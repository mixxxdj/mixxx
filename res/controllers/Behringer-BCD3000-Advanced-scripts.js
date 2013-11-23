function BehringerBCD3000 () {}

//sensitivity setting
BehringerBCD3000.UseAcceleration = true;
BehringerBCD3000.JogSensivity = 0.5;

BehringerBCD3000.init = function (id) { // called when the device is opened & set up
	
	BehringerBCD3000.reset();

	// Ask BCD to send the current values of all rotary knobs and sliders
	midi.sendShortMsg(0xB0,0x64,0x7F);

	// Set jog acceleration
	if (BehringerBCD3000.UseAcceleration)
		midi.sendShortMsg(0xB0, 0x63, 0x7F);
	else
		midi.sendShortMsg(0xB0, 0x63, 0x0);
};

BehringerBCD3000.shutdown = function () {

	BehringerBCD3000.reset();

	// Reenable jog acceleration 
	if (!BehringerBCD3000.UseAcceleration)
		midi.sendShortMsg(0xB0, 0x63, 0x7F);
};

BehringerBCD3000.reset = function (id) {

	// Turn off all the lights
	for (i = 0; i <= 25; i++) {
		midi.sendShortMsg(0xB0, i, 0);
	}
	// reset global variables
	BehringerBCD3000.id = id;
	BehringerBCD3000.onKey = false;
	BehringerBCD3000.actionKey = false;
	BehringerBCD3000.keyKey = false;
	BehringerBCD3000.alt = false;
	BehringerBCD3000.inputA = false;
	BehringerBCD3000.inputB = false;
	BehringerBCD3000.scratchMode = [false, false];
	BehringerBCD3000.scratchTimer = [-1, -1];

};

BehringerBCD3000.getDeck = function (group) {
	if (group == "[Channel1]")
		return 0;
	else if (group == "[Channel2]")
		return 1;
	
	print("Invalid group : " + group);
	return -1; // error
}

BehringerBCD3000.actionKey = function (value) {
	if (value)
		midi.sendShortMsg(0xB0, 28, 0x7F)

}



//Scratch, cue search and pitch bend function, browse
BehringerBCD3000.jogWheel = function (channel, control, value, status, group) {


	deck = BehringerBCD3000.getDeck(group);

	if (BehringerBCD3000.scratchMode[deck]) {

		scratchValue = (value - 0x40);
		engine.scratchEnable(deck + 1, 100, 33+1/3, 1.0/8, (1.0/8)/32);
		engine.scratchTick(deck + 1, scratchValue);
		BehringerBCD3000.scratchTimer[deck] = engine.beginTimer(20, "BehringerBCD3000.stopScratch(" + deck + ")", true);

	} else if (BehringerBCD3000.onKey) {
		if (value > 0x40){
			engine.setValue("[Playlist]","SelectNextTrack",1);
		} else {
			engine.setValue("[Playlist]","SelectPrevTrack",1);
		}

	} else if (BehringerBCD3000.actionKey) {
		if (value > 0x40){
			engine.setValue("[Playlist]","SelectNextTrack",1);
		} else {
			engine.setValue("[Playlist]","SelectPrevTrack",1);
		}

	} else {

		jogValue = (value - 0x40) * BehringerBCD3000.JogSensivity;
		engine.setValue(group, "jog", jogValue);

	}
};

//Scratch button function => set scratch flag
BehringerBCD3000.scratchButton = function (channel, control, value, status, group) {

	deck = BehringerBCD3000.getDeck(group);

	if (value) BehringerBCD3000.scratchMode[deck] = !BehringerBCD3000.scratchMode[deck];

	if (BehringerBCD3000.scratchMode[deck]) {
		// Turn on the scratch light
		if (!deck)
			midi.sendShortMsg(0xB0, 0x13, 0x7F);
		else
			midi.sendShortMsg(0xB0, 0x0B, 0x7F);
	} else {
		// Turn off the scratch light
		if (!deck)	
			midi.sendShortMsg(0xB0, 0x13, 0x00);
		else
			midi.sendShortMsg(0xB0, 0x0B, 0x00);
		engine.scratchDisable(deck + 1);
	}
};

// Stop scratch by timer
BehringerBCD3000.stopScratch = function(deck) {
        BehringerBCD3000.scratchTimer[deck] = -1;
        engine.scratchDisable(deck + 1);
}


//Set loop function 
BehringerBCD3000.loop = function (channel, control, value, status, group) {
	if (value)
		action = "loop_in";
	else
		action = "loop_out";
		engine.setValue(group, action, 1);
};

//On button function 
BehringerBCD3000.On = function (channel, control, value, status, group) {

	if (BehringerBCD3000.actionKey){
		BehringerBCD3000.actionKey = false;
		midi.sendShortMsg(0xB0, 0x3, 0x00);
	}
		deck = BehringerBCD3000.getDeck(group);

	if (value) BehringerBCD3000.onKey = !BehringerBCD3000.onKey;
		if (BehringerBCD3000.onKey) {
		// Turn on the On light
		midi.sendShortMsg(0xB0, 0x6, 0x7F);
	} else {
		// Turn off the On light
		midi.sendShortMsg(0xB0, 0x6, 0x00);
		engine.setValue("[Channel1]","LoadSelectedTrack",1);
	};
};

//Action button function 
BehringerBCD3000.Action = function (channel, control, value, status, group) {
	if (BehringerBCD3000.onKey){
		BehringerBCD3000.onKey = false;
		midi.sendShortMsg(0xB0, 0x6, 0x00);
	}

	deck = BehringerBCD3000.getDeck(group);

	if (value) BehringerBCD3000.actionKey = !BehringerBCD3000.actionKey;

	if (BehringerBCD3000.actionKey) {
		// Turn on the Action light
		midi.sendShortMsg(0xB0, 0x3, 0x7F);
	} else {
		// Turn off the Action light
		midi.sendShortMsg(0xB0, 0x3, 0x00);
		engine.setValue("[Channel2]","LoadSelectedTrack",1);
	}
};

//Key button function 
BehringerBCD3000.keykey = function (channel, control, value, status, group) {
	// toggle "alt" flag
	if (value) BehringerBCD3000.alt = !BehringerBCD3000.alt;

	if (BehringerBCD3000.alt) {
		// Turn on the Key light
		midi.sendShortMsg(0xB0, 0x19, 0x7F);
		engine.setValue("[Channel1]","keylock",1);
		engine.setValue("[Channel2]","keylock",1);
	} else {
		// Turn off the Key light
		midi.sendShortMsg(0xB0, 0x19, 0x00);
		engine.setValue("[Channel1]","keylock",0);
		engine.setValue("[Channel2]","keylock",0);
	}
};


//Ext-A button function 
//BehringerBCD3000.extakey = function (channel, control, value, status, group) {
//	// toggle inputA flag
//	if (value) BehringerBCD3000.inputA = !BehringerBCD3000.inputA;
//	if (BehringerBCD3000.inputA) {
//		// Turn on the Key light
//		midi.sendShortMsg(0xB0, 0x08, 0x7F);
//	} else {
//		// Turn off the Key light
//		midi.sendShortMsg(0xB0, 0x08, 0x00);
//	}
//};

//Ext-B button function 
//BehringerBCD3000.extbkey = function (channel, control, value, status, group) {
//	// toggle inputB flag
//	if (value) BehringerBCD3000.inputB = !BehringerBCD3000.inputB;
//	if (BehringerBCD3000.inputB) {
//		// Turn on the Key light
//		midi.sendShortMsg(0xB0, 0x07, 0x7F);
//	} else {
//		// Turn off the Key light
//		midi.sendShortMsg(0xB0, 0x07, 0x00);
//	}
//};


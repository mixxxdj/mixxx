function BehringerBCD3000 () {}
BehringerBCD3000.debug = true;
BehringerBCD3000.escratch1 = false;
BehringerBCD3000.escratch2 = false;

//sensitivity setting
BehringerBCD3000.UseAcceleration = true;
if (BehringerBCD3000.UseAcceleration) {
	BehringerBCD3000.JogSensivity = 0.3;
	BehringerBCD3000.ScratchSensivity = 10;
} else {
	BehringerBCD3000.JogSensivity = 0.5;
	BehringerBCD3000.ScratchSensivity = 15;
}

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

BehringerBCD3000.reset = function () {

	// Turn off all the lights
	for (i = 0; i <= 25; i++) {
		midi.sendShortMsg(0xB0, i, 0);
	}

};


//Scratch, cue search and pitch bend function for channel 1
BehringerBCD3000.jog_wheel1 = function (group, control, value, status) {


	if (BehringerBCD3000.escratch1) {

		scratchValue = (value - 0x40) * BehringerBCD3000.ScratchSensivity;
		engine.setValue("[Channel1]", "jog", scratchValue);

		if (BehringerBCD3000.debug)
			print("Channel 1 scratch jog adjust : " + scratchValue);

	} else {

		jogValue = (value - 0x40) * BehringerBCD3000.JogSensivity;
		engine.setValue("[Channel1]", "jog", jogValue);

		if (BehringerBCD3000.debug)
			print("Channel 1 pitching jog adjust : " + jogValue);

	}
};

//Scratch button function for channel 1
BehringerBCD3000.scratch1 = function (group, control, value, status) {

	if (value != 0x7F)
		return;

	BehringerBCD3000.escratch1 = !BehringerBCD3000.escratch1;

	if (BehringerBCD3000.debug)
		print("Channel 1 scratch enabled :" + BehringerBCD3000.escratch1);

	if (BehringerBCD3000.escratch1) {
		// Turn on the scratch light
		midi.sendShortMsg(0xB0, 0x13, 0x7F);
	} else {
		// Turn off the scratch light
		midi.sendShortMsg(0xB0, 0x13, 0x00);
	}
};


//Scratch, cue search and pitch bend function for channel 2
BehringerBCD3000.jog_wheel2 = function (group, control, value, status) {


	if (BehringerBCD3000.escratch2) {

		scratchValue = (value - 0x40) * BehringerBCD3000.ScratchSensivity;
		engine.setValue("[Channel2]", "jog", scratchValue);

		if (BehringerBCD3000.debug)
			print("Channel 2 scratch jog adjust : " + scratchValue);

	} else {

		jogValue = (value - 0x40) * BehringerBCD3000.JogSensivity;
		engine.setValue("[Channel2]", "jog", jogValue);

		if (BehringerBCD3000.debug)
			print("Channel 2 pitching jog adjust : " + jogValue);

	}
};

//Scratch button function for channel 2
BehringerBCD3000.scratch2 = function (group, control, value, status) {

	if (value != 0x7F)
		return;

	BehringerBCD3000.escratch2 = !BehringerBCD3000.escratch2;

	if (BehringerBCD3000.debug)
		print("Channel 2 scratch enabled :" + BehringerBCD3000.escratch2);

	if (BehringerBCD3000.escratch2) {
		// Turn on the scratch light
		midi.sendShortMsg(0xB0, 0x0B, 0x7F);
	} else {
		// Turn off the scratch light
		midi.sendShortMsg(0xB0, 0x0B, 0x00);
	}
};

//Set loop function for channel 1
BehringerBCD3000.loop1 = function (group, control, value, status) {
	if (value)
		action = "loop_in";
	else
		action = "loop_out";

	if (BehringerBCD3000.debug)
		print("Channel 1 " + action);

	 engine.setValue("[Channel1]", action, 1);
};

//Set loop function for channel 2
BehringerBCD3000.loop2 = function (group, control, value, status) {
	if (value)
		action = "loop_in";
	else
		action = "loop_out";

	if (BehringerBCD3000.debug)
		print("Channel 2 " + action);

	 engine.setValue("[Channel2]", action, 1);
};

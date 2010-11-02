function HerculesMP3 () {}

HerculesMP3.leds = { 
"ch 2 blink play":0x80,
"ch 2 play":0x2,
"ch 2 cue button":0x3,
"ch 2 autobeat":0x4,
"ch 1 blink play":0x5,
"ch 1 play":0x8,
"ch 1 cue button":0x9,
"ch 1 autobeat":0xa,
"ch1 loop":0xd,
"ch1 cue":0xe,
"ch 1 fx":0xf,
"ch 2 fx":0x10,
"ch2 cue":0x11,
"ch2 loop":0x12,
"ch1 tempo":0x16,
"ch2 tempo":0x1a,
"pfl2" : 0x7d, //125
"pfl1" : 0x7e //126
};

HerculesMP3.blinking1 = false;
HerculesMP3.blinking2 = false;
HerculesMP3.playing1 = false;
HerculesMP3.playing2 = false;
HerculesMP3.blink2toggle = false;
HerculesMP3.blink2timer = null;

HerculesMP3.EXPO = 1.75;
HerculesMP3.RATEPERM = 0.005;
HerculesMP3.NORMTICK = 1.0/20.0;
HerculesMP3.NORMEXPO = HerculesMP3.NORMTICK / HerculesMP3.EXPO;

HerculesMP3.rate1startval = 0;
HerculesMP3.rate2startval = 0;
HerculesMP3.rate1time = 0;
HerculesMP3.rate2time = 0;
HerculesMP3.rate1changing = 0; //-1 down, 0 steady, 1 up
HerculesMP3.rate2changing = 0;

HerculesMP3.init = function (id) {    // called when the MIDI device is opened & set up
    print ("HerculesMP3 id: \""+id+"\" initialized.");

	//doesn't work (not started yet?)
	//HerculesMP3.clearlights();
	
	engine.connectControl("[Channel1]","play","HerculesMP3.play1");
    engine.connectControl("[Channel2]","play","HerculesMP3.play2");
    engine.connectControl("[Channel1]","playposition","HerculesMP3.playpos1");
    engine.connectControl("[Channel2]","playposition","HerculesMP3.playpos2");
    
    engine.beginTimer(100,"HerculesMP3.rate_changer()");
    //print ("stuff hooked up");
}

HerculesMP3.shutdown = function(id) {
	HerculesMP3.clearlights();
}

HerculesMP3.clearlights = function () {
	for ( var LED in HerculesMP3.leds ) {
        print("Clear LED: " + LED+" "+HerculesMP3.leds[LED]);
        midi.sendShortMsg(0xB1, HerculesMP3.leds[LED], 0x0);
    }
}	  
	  
HerculesMP3.play1 = function (value) { 
	//print ("play1 "+value);
	HerculesMP3.playing1 = value;
	HerculesMP3.maybelightplay1();
}

HerculesMP3.play2 = function (value) { 
	//print ("play2 "+value);
	HerculesMP3.playing2 = value;
	HerculesMP3.maybelightplay2();
}

HerculesMP3.maybelightplay1 = function () { 
	if (HerculesMP3.playing1)
	{
		//print ("light play1");
		midi.sendShortMsg(0xb1, HerculesMP3.leds["ch 1 play"], 0x7f);
	}
	else
	{
		//print ("dark play1");
		midi.sendShortMsg(0xb1, HerculesMP3.leds["ch 1 play"], 0x0);
	}
}

HerculesMP3.maybelightplay2 = function () { 
	if (HerculesMP3.playing2)
	{
		//print ("light play2");
		midi.sendShortMsg(0xb1, HerculesMP3.leds["ch 2 play"], 0x7f);
	}
	else
	{
		//print ("dark play2");
		midi.sendShortMsg(0xb1, HerculesMP3.leds["ch 2 play"], 0x0);
	}
}

HerculesMP3.playpos1 = function (value) {
	duration = engine.getValue("[Channel1]","duration");
	//print("playpos1 " + value + " " + duration);
	if (value > 0.9 && duration > 0)
	{
		if (!HerculesMP3.blinking1)
		{
			midi.sendShortMsg(0xb1, HerculesMP3.leds["ch 1 blink play"], 0x7f);
		}
		HerculesMP3.blinking1 = true;
	}
	else
	{
		if (HerculesMP3.blinking1)
		{
			midi.sendShortMsg(0xb1, HerculesMP3.leds["ch 1 blink play"], 0x0);
		}
		HerculesMP3.blinking1 = false;
	}			
	//engine.trigger("[Channel1]","playposition");
	//engine.trigger("[Channel1]","play");
}

HerculesMP3.playpos2 = function (value) {
	//print("playpos2 " + value);
	duration = engine.getValue("[Channel2]","duration");
	if (value > 0.9 && duration > 0)
	{
		//print ("Running blink workaround");
		//HerculesMP3.workaroundblink2(value);
		if (!HerculesMP3.blinking2)
			HerculesMP3.blink2timer = engine.beginTimer(1000,"HerculesMP3.workaroundblink2()");
		HerculesMP3.blinking2 = true;
	}
	else
	{
		if (HerculesMP3.blinking2)
		{
			HerculesMP3.maybelightplay2();
			engine.stopTimer(HerculesMP3.blink2timer);
		}
		HerculesMP3.blinking2 = false;
	}	
	//engine.trigger("[Channel2]","playposition");
	//engine.trigger("[Channel2]","play");
}

HerculesMP3.workaroundblink2 = function(value) {
	// for some reason, activating the blinker on deck 2 turns
	// off the light on deck 1.  so, blink manually!
	//print ("change state");
	if (HerculesMP3.blink2toggle)
		midi.sendShortMsg(0xB1, HerculesMP3.leds["ch 2 play"], 0x0);
	else
		midi.sendShortMsg(0xB1, HerculesMP3.leds["ch 2 play"], 0x7f);
	HerculesMP3.blink2toggle = !HerculesMP3.blink2toggle;
}

HerculesMP3.rate_perm_up_small1 = function (group, control, value, status) {
	if (value == 127)
	{
		HerculesMP3.rate1changing = 1;
		HerculesMP3.rate1startval = engine.getValue("[Channel1]","rate");
		HerculesMP3.rate1time = 0;
	}
	else
	{
		HerculesMP3.rate1changing = 0;
		HerculesMP3.rate1startval = 0;
		HerculesMP3.rate1time = 0;
	}
}

HerculesMP3.rate_perm_up_small2 = function (group, control, value, status) {
	if (value == 127)
	{
		HerculesMP3.rate2changing = 1;
		HerculesMP3.rate2startval = engine.getValue("[Channel2]","rate");
		HerculesMP3.rate2time = 0;
	}
	else
	{
		HerculesMP3.rate2changing = 0;
		HerculesMP3.rate2startval = 0;
		HerculesMP3.rate2time = 0;
	}
}

HerculesMP3.rate_perm_down_small1 = function (group, control, value, status) {
	if (value == 127)
	{
		HerculesMP3.rate1changing = -1;
		HerculesMP3.rate1startval = engine.getValue("[Channel1]","rate");
		HerculesMP3.rate1time = 0;
	}
	else
	{
		HerculesMP3.rate1changing = 0;
		HerculesMP3.rate1startval = 0;
		HerculesMP3.rate1time = 0;
	}
}

HerculesMP3.rate_perm_down_small2 = function (group, control, value, status) {
	if (value == 127)
	{
		HerculesMP3.rate2changing = -1;
		HerculesMP3.rate2startval = engine.getValue("[Channel2]","rate");
		HerculesMP3.rate2time = 0;
	}
	else
	{
		HerculesMP3.rate2changing = 0;
		HerculesMP3.rate2startval = 0;
		HerculesMP3.rate2time = 0;
	}
}

HerculesMP3.rate_changer = function () {
	if(HerculesMP3.rate1changing != 0)
		HerculesMP3.change_rate1();
	if(HerculesMP3.rate2changing != 0)
		HerculesMP3.change_rate2();
}

HerculesMP3.change_rate1 = function () {
	if (HerculesMP3.rate1time < 10)
		new_rate = HerculesMP3.rate1time * 0.001;
	else
		new_rate = 0.01 + Math.pow((HerculesMP3.rate1time-10)*HerculesMP3.NORMTICK, HerculesMP3.EXPO)*HerculesMP3.NORMEXPO;
	new_rate *= HerculesMP3.rate1changing;
	new_rate += HerculesMP3.rate1startval;
	new_rate = Math.min(new_rate, 1);
	new_rate = Math.max(new_rate, -1);
	engine.setValue("[Channel1]", "rate", new_rate);
	HerculesMP3.rate1time++; //tick
}

HerculesMP3.change_rate2 = function () {
	if (HerculesMP3.rate2time < 10)
		new_rate = HerculesMP3.rate2time * 0.001;
	else
		new_rate = 0.01 + Math.pow((HerculesMP3.rate2time-10)*HerculesMP3.NORMTICK, HerculesMP3.EXPO)*HerculesMP3.NORMEXPO;
	new_rate *= HerculesMP3.rate2changing;
	new_rate += HerculesMP3.rate2startval;
	new_rate = Math.min(new_rate, 1);
	new_rate = Math.max(new_rate, -1);
	engine.setValue("[Channel2]", "rate", new_rate);
	HerculesMP3.rate2time++; //tick
}

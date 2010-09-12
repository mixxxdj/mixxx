function DM2() {}

DM2.playlist2deck =	{	"65" : "SelectNextTrack", // Wheel Clockwise
						"66" : "SelectPrevTrack", // Wheel Counter Clockwise
						"67" : 1, //Last wheel to stop moving on middle up
						"68" : 2  //Last wheel to stop moving on middle up
};

DM2.knob2deck =		{	"1"  : 1,	//Wheel:		Left Wheel
						"3"  : 2,	//Wheel:		Right Wheel
						"18" : 1,	//Jog:			Left Wheel
						"36" : 2,	//Jog:			Right Wheel
						"23" : 1,	//Rate:			Left  Wheel
						"39" : 2,	//Rate:			Right Wheel
						"22" : 1,	//filterHigh:	Left  Wheel
						"32" : 2,	//filterHigh:	Right Wheel
						"21" : 1,	//filterMid:	Left  Wheel
						"33" : 2,	//filterMid:	Right Wheel
						"20" : 1,	//filterLow:	Left  Wheel
						"34" : 2,	//filterLow:	Right Wheel
						"16" : 1,	//Gain:			Left Wheel
						"38" : 2,	//Gain:			Right Wheel
						"17" : 1,	//Volume:		Left Wheel
						"37" : 2	//Volume:		Right Wheel
};

DM2.button2mixxx = { 
						"22" : "filterHigh",	//filterHigh:	Left  Wheel
						"32" : "filterHigh",	//filterHigh:	Right Wheel
						"21" : "filterMid",		//filterMid:	Left  Wheel
						"33" : "filterMid",		//filterMid:	Right Wheel
						"20" : "filterLow",		//filterLow:	Left  Wheel
						"34" : "filterLow",		//filterLow:	Right Wheel
						"16" : "pregain",		//Gain:			Left Wheel
						"38" : "pregain",		//Gain:			Right Wheel
						"23" : "flanger",		// Right
						"31" : "flanger"		// Left
};

DM2.buttonStatus = { 
						"35" : false,			//Button 1
						"34" : false,			//Button 2
						"33" : false			//Button 3
}


DM2.deck1leds =  [ 64, 65, 66, 67, 68, 69, 70, 71 ];
DM2.deck2leds =  [ 80, 81, 82, 83, 84, 85, 86, 87 ];

DM2.deck1idle = 88;
DM2.deck2idle = 89;

DM2.wheelMode = 1;


DM2.init = function (id) {   // called when the MIDI device is opened & set up
	print ("======DM2: initialized. " + id );

	//midi.sendShortMsg(0x80, 17, 0xff);
	engine.setValue( "[Channel1]", "volume", 0.5 );
	engine.setValue( "[Channel2]", "volume", 0.5 );
	
	//engine.connectControl("[Channel1]","playposition","DM2.wheelDecay");
	//engine.connectControl("[Channel2]","playposition","DM2.wheelDecay");

}

DM2.shutdown = function () {   // called when the MIDI device is closed
	print ("======DM2: shutdown...");
}

DM2.ledOn = function(led) {
	midi.sendShortMsg(0x90, led, 0x7f);
}

DM2.ledOff = function(led) {
	midi.sendShortMsg(0x80, led, 0);
}

DM2.unused = function (channel, control, value, status) {
	script.debug(channel, control, value, status)
	nop();
}

DM2.button = function (channel, control, value, status) {
	DM2.buttonStatus[control] = value > 0;
}

DM2.joystick = function (channel, control, value, status) {
	script.debug(channel, control, value, status)
	//volume Up Down
	
	//balance Left Right
}

DM2.wheel = function (channel, control, value, status) {
	value = value - 64;
	value = script.absoluteNonLin(value, -3, 0, 3);
	engine.setValue( "[Channel" + DM2.knob2deck[control] + "]", "wheel", value );
}

DM2.playlist = function (channel, control, value, status) {
	engine.setValue( "[Playlist]", DM2.playlist2deck[control], 1 );
}

DM2.middle = function (channel, control, value, status) {
	//engine.setValue( "[Playlist]", "LoadSelectedIntoFirstStopped", 1 );
	engine.setValue( "[Channel" + DM2.playlist2deck[control] + "]", "LoadSelectedTrack", 1 );
}

DM2.volume = function (channel, control, value, status) {
	//value = script.absoluteNonLin(value, 0, 1, 2) / 2;
	//engine.setValue( "[Channel" + DM2.knob2deck[control] + "]", "volume", value );
	script.absoluteSlider( "[Channel" + DM2.knob2deck[control] + "]", "volume", value, 0, 1 );
}

DM2.rate = function (channel, control, value, status) {
	//7bit values (0x00 - 0x7F)
	value = script.absoluteNonLin(value, 0, 1, 2) - 1;
	engine.setValue( "[Channel" + DM2.knob2deck[control] + "]", "rate", value );
}

DM2.filter = function (channel, control, value, status) {
	//7bit values (0x00 - 0x7F)
	value = script.absoluteNonLin(value, 0, 1, 4);
	engine.setValue( "[Channel" + DM2.knob2deck[control] + "]", DM2.button2mixxx[control], value );
}

DM2.filterKill = function( channel, control, value, status ) {
	var toggle = ~ engine.getValue( "[Channel" + DM2.knob2deck[control] + "]", DM2.button2mixxx[control] + "Kill" ) & 1;

	print("setting: " + DM2.button2mixxx[control] + " = " + (toggle) );
	engine.setValue( "[Channel" + DM2.knob2deck[control] + "]", DM2.button2mixxx[control] + "Kill", toggle );
}

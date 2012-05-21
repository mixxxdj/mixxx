
function DM2() {}

DM2.pitch2deck =   {  "0" : 1, // Left  Wheel
                      "1" : 2  // Right Wheel
};

DM2.button2deck =  {  "1" : 1,
                      "1" : 2,
                     "30" : 1, // Left  filterHighKill
                     "29" : 1, // Left  filterMidKill
                     "28" : 1, // Left  filterlowKill
                     "16" : 2, // Right filterHighKill
                     "17" : 2, // Right filterMidKill
                     "18" : 2, // Right filterLowKill
                     "23" : 2, // Right flanger
                     "31" : 1, // Left  flanger
                     "19" : 2, // Right cue_default
                     "27" : 1, // Left  cue_default
                     "22" : 2, // Right rate_perm_up_small
                     "24" : 1, // Left  rate_perm_up_small
                     "20" : 2, // Right rate_perm_down_small
                     "26" : 1  // Left  rate_perm_down_small
};

DM2.button2mixxx = { "30" : "filterHighKill", // Left
                     "29" : "filterMidKill",  // Left
                     "28" : "filterLowKill",  // Left
                     "16" : "filterHighKill", // Right
                     "17" : "filterMidKill",  // Right
                     "18" : "filterLowKill",  // Right
                     "23" : "flanger",        // Right
                     "31" : "flanger",        // Left
                     "19" : "cue_default",    // Right
                     "27" : "cue_default",    // Left
                     "22" : "rate_perm_up_small",    // Right
                     "24" : "rate_perm_up_small",    // Left
                     "20" : "rate_perm_down_small",    // Right
                     "26" : "rate_perm_down_small"     // Left
};

DM2.cc2deck =      {  "3" : 1, // Left  filterHighKill
                      "4" : 2, // Right filterHighKill
                      "5" : 1, // Left  filterMidKill
                      "6" : 2, // Right filterMidKill
                      "7" : 1, // Left  filterLowKill
                      "8" : 2, // Right filterLowKill
                      "9" : 1, // Left  gain
                     "10" : 2, // Right gain
                     "11" : 1, // Left  volume
                     "12" : 2, // Right volume
                     "13" : 1, // left  rate
                     "14" : 2  // Right rate
};

DM2.cc2mixxx =    {   "3" : "filterHigh", // Left
                      "4" : "filterHigh", // Right
                      "5" : "filterMid",  // Left
                      "6" : "filterMid",  // Right
                      "7" : "filterLow",  // Left
                      "8" : "filterLow",  // Right
                      "9" : "pregain",    // Left
                     "10" : "pregain",    // Right
                     "11" : "volume",     // Left
                     "12" : "volume",     // Right
                     "13" : "rate",       // left
                     "14" : "rate"        // Right
};


DM2.acc = 0;
DM2.lastacc = 0;
DM2.playlist_channel = -1;
DM2.filter_active = 0;

DM2.init = function (id) {   // called when the MIDI device is opened & set up
    print ("======DM2: initialized. " + id );
}

DM2.shutdown = function () {   // called when the MIDI device is closed
    print ("======DM2: shutdown...");
}

DM2.wheel = function (channel, control, value, status ) {
	// 14bit values
	var newValue = ( value << 7 ) | control;
	if( newValue >= 8192 )
		newValue = newValue - 16384;
// 	print( "14bit: " + newValue );

// 	print( "setting wheel("+channel +"): " + "[Channel" + DM2.pitch2deck[channel] + "]" + " to " + newValue/512.0 );
	engine.setValue( "[Channel" + DM2.pitch2deck[channel] + "]", "jog", newValue/512.0 );

// 	if( channel == 0 ) {
// 		if( engine.getValue("[Channel2]","VuMeter") > 0.6 )
// 			print( "*" );
// 		else
// 			print( "" );
// 		print ("playpos: " + engine.getValue("[Channel2]","VuMeter"));
// 	}
/*	print( "=====DM2:" );
	print( " - channel: "  + channel  );
	print( " - control: "  + control  );
	print( " - value: "    + value    );
	print( " - status: "   + status   );
	print( "setting jog: " + newValue   );*/
}

DM2.playlist = function (channel, control, value, status) {
	// 7bit values
	var newValue=value;
	if( value >= 64 )
		newValue = newValue - 128;

	DM2.acc += newValue;

	if( DM2.acc > DM2.lastacc + 20 )
	{
		DM2.playlist_channel = channel;
		engine.setValue( "[Playlist]", "SelectNextTrack", 1 );
		DM2.lastacc = DM2.acc;
	}
	else if( DM2.acc < DM2.lastacc - 20 )
	{
		DM2.playlist_channel = channel;
		engine.setValue( "[Playlist]", "SelectPrevTrack", 1 );
		DM2.lastacc = DM2.acc;
	}
}

DM2.middle = function (channel, control, value, status) {
	if( DM2.playlist_channel != -1 )
	{
		engine.setValue( "[Playlist]", "LoadSelectedIntoFirstStopped", 1 );
// 		engine.setValue( "[Channel" + DM2.playlist_channel + "]", "LoadSelectedTrack", 1 );
		DM2.playlist_channel = -1;
	}
}

DM2.filter = function (channel, control, value, status) {
	var deck = DM2.cc2deck[control];
	// 7bit values
	if( value >= 64 )
		value = value - 128;

	var f = engine.getValue( "[Channel" + deck + "]", DM2.cc2mixxx[control] );
	f += value / 256;
	if( f > 4.0 )
		f = 4.0;
	else if( f < 0.0 )
		f = 0.0;

// 	print( "settiing [Channel" + deck + "] " + DM2.cc2mixxx[control] + " to " + f );
	engine.setValue( "[Channel" + deck + "]", DM2.cc2mixxx[control], f );
	DM2.filter_active = 1;
}

DM2.volume = function (channel, control, value, status) {
	var deck = DM2.cc2deck[control];
	// 7bit values
	if( value >= 64 )
		value = value - 128;

	var f = engine.getValue( "[Channel" + deck + "]", DM2.cc2mixxx[control] );
	f += value / 512;
	if( f > 1.0 )
		f = 1.0;
	else if( f < 0.0 )
		f = 0.0;

	engine.setValue( "[Channel" + deck + "]", DM2.cc2mixxx[control], f );
	DM2.filter_active = 1;
}

DM2.rate = function (channel, control, value, status) {
	var deck = DM2.cc2deck[control];
	// 7bit values
	if( value >= 64 )
		value = value - 128;

	var f = engine.getValue( "[Channel" + deck + "]", DM2.cc2mixxx[control] );
	f += value / 8192;
	if( f > 1.0 )
		f = 1.0;
	else if( f < -1.0 )
		f = -1.0;

	engine.setValue( "[Channel" + deck + "]", DM2.cc2mixxx[control], f );
	DM2.filter_active = 1;
}

DM2.filterKill = function( channel, control, value, status )
{
	print( "control "+control );
	if( DM2.filter_active )
		DM2.filter_active = 0;
	else
	{
		print( "setting: " + DM2.button2mixxx[control] + " = " + ( ~ engine.getValue( "[Channel" + DM2.button2deck[control] + "]",
				DM2.button2mixxx[control] ) & 1 ) );
		engine.setValue( "[Channel" + DM2.button2deck[control] + "]",
			DM2.button2mixxx[control],
			( ~ engine.getValue( "[Channel" + DM2.button2deck[control] + "]",
				DM2.button2mixxx[control] ) & 1 ) );
	}
}


// TODO: does not work
DM2.trigger = function( channel, control, value, status )
{
	print( "trigger "+control );
	if( DM2.filter_active )
		DM2.filter_active = 0;
	else
	{
		var f = engine.getValue( "[Channel" + DM2.button2deck[control] + "]", DM2.button2mixxx[control] );
		engine.setValue( "[Channel" + DM2.button2deck[control] + "]",
			DM2.button2mixxx[control], 1 );
		var i;
		for( i = 0; i < 10000; i++ ) ;
		engine.setValue( "[Channel" + DM2.button2deck[control] + "]",
			DM2.button2mixxx[control], 0 );
	}
}

function MaudioXponent () {}

// ----------   Global variables    ----------

MaudioXponent.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
MaudioXponent.off = 0x80;
MaudioXponent.on = 0x90;
MaudioXponent.mastermeter = 0;  //1 for master meter, 0 for channel meter
MaudioXponent.flashprogress = 8; //flash duration on progress meters
MaudioXponent.leds = {
    "play": 0x24,
    "cue": 0x23,
    "back": 0x21,
    "fwd": 0x22,
    "in": 0x29,
    "out": 0x2B,
    "loop": 0x2A,
    "loop1": 0x25,
    "loop2": 0x26,
    "loop4": 0x27,
    "loop8": 0x28,
    "leftkey": 0x1C,
    "rightkey": 0x1D,
    "key": 0x1E,
    "pluskey": 0x1F,
    "minkey": 0x20,
    "cue1": 0x17,
    "cue2": 0x18,
    "cue3": 0x19,
    "cue4": 0x1A,
    "cue5": 0x1B,
    "fx1": 0x0C,
    "fx2": 0x0D,
    "fx3": 0x0E,
    "fx4": 0x0F,
    "leftp": 0x10,
    "rightp": 0x11,
    "bigx": 0x12,
    "big-": 0x13,
    "head": 0x14,
    "scratch": 0x15,
    "fadarbutton": 0x07,
    "sync": 0x02,
    "low": 0x08,
    "middle": 0x09,
    "hight": 0x0A,
    "gain": 0x0B,
    "shift": 0x2C
};
MaudioXponent.binleds = {
    8:  "filterLowKill",
    9:  "filterMidKill",
    10: "filterHighKill",
    18: "keylock",
    19: "reverse",
    20: "pfl",
    33: "back",
    34: "fwd",
    36: "play"
};
MaudioXponent.hotcues = {
    23: 1,
    24: 2,
    25: 3,
    26: 4,
    27: 5
};
MaudioXponent.timer = [-1, -1];  // Temporary storage of timer IDs
MaudioXponent.state = { 
    "flashes" : 0, 
    "faderpos" : 0, 
    "shift1" : 0, 
    "shift2" : 0, 
    "scrmode1" : 0, 
    "scrmode2" : 0, 
    "plnumberpos" : 0, 
    "plnumberneg" : 0,
    "scratching1" : 0,
    "scratching2" : 0,
}; // Temporary state variables

// ----------   Functions    ----------

MaudioXponent.init = function (id) {    // called when the MIDI device is opened & set up
    
    //This code light on all leds and then light off
    midi.sendShortMsg(0xB3,0x14,0x00);
    midi.sendShortMsg(0xB3,0x15,0x00);
    for (var led in MaudioXponent.leds) {
	midi.sendShortMsg(MaudioXponent.on,MaudioXponent.leds[led],0x01);
	midi.sendShortMsg(MaudioXponent.on + 1,MaudioXponent.leds[led],0x01);
	MaudioXponent.pauseScript(15);
    }
    midi.sendShortMsg(0xB3,0x12,0x7F);
    midi.sendShortMsg(0xB3,0x13,0x7F);
    midi.sendShortMsg(0xB3,0x14,0x7F);
    midi.sendShortMsg(0xB3,0x15,0x7F);
    MaudioXponent.pauseScript(500);
    for (var led in MaudioXponent.leds) {
	midi.sendShortMsg(MaudioXponent.on,MaudioXponent.leds[led],0x00);
	midi.sendShortMsg(MaudioXponent.on + 1,MaudioXponent.leds[led],0x00);
	MaudioXponent.pauseScript(15);
    }
    midi.sendShortMsg(0xB3,0x14,0x00);
    midi.sendShortMsg(0xB3,0x15,0x00);
    //end of light on all leds

    MaudioXponent.flashdur1 = 0;
    MaudioXponent.flashdur2 = 0;
    engine.connectControl("[Channel1]","visual_playposition","MaudioXponent.playPositionMeter1");
    engine.connectControl("[Channel2]","visual_playposition","MaudioXponent.playPositionMeter2");
    if (MaudioXponent.mastermeter == 1) {
	engine.connectControl("[Master]","VuMeterL","MaudioXponent.volumeLEDs1");
	engine.connectControl("[Master]","VuMeterR","MaudioXponent.volumeLEDs2");
    } else {
	engine.connectControl("[Channel1]","VuMeter","MaudioXponent.volumeLEDs1");
	engine.connectControl("[Channel2]","VuMeter","MaudioXponent.volumeLEDs2");
    }

    engine.connectControl("[Channel1]","hotcue_1_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel1]","hotcue_2_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel1]","hotcue_3_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel1]","hotcue_4_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel1]","hotcue_5_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel2]","hotcue_1_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel2]","hotcue_2_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel2]","hotcue_3_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel2]","hotcue_4_enabled","MaudioXponent.hotcue");
    engine.connectControl("[Channel2]","hotcue_5_enabled","MaudioXponent.hotcue");

    engine.connectControl("[Channel1]", "loop_enabled", "MaudioXponent.onLoopExit");
    engine.connectControl("[Channel1]", "loop_start_position", "MaudioXponent.onLoopIn");
    engine.connectControl("[Channel1]", "loop_end_position", "MaudioXponent.onLoopOut");
    engine.connectControl("[Channel2]", "loop_enabled", "MaudioXponent.onLoopExit");
    engine.connectControl("[Channel2]", "loop_start_position", "MaudioXponent.onLoopIn");
    engine.connectControl("[Channel2]", "loop_end_position", "MaudioXponent.onLoopOut");

    //engine.connectControl("[Channel1]","bpm","MaudioXponent.bpmsync");

};

MaudioXponent.wheel = function (channel, control, value, status) {
    var currentdeck = channel+1;
    
    // In either case, register the movement

    if (MaudioXponent.state["shift"+currentdeck] == 1) {
        if (value > 64) {
            MaudioXponent.state["plnumberpos"]++;
            if (MaudioXponent.state["plnumberpos"] % 12 == 0) {
                engine.setValue("[Playlist]","SelectTrackKnob",1);
            }
        } else if (value < 64) {
            MaudioXponent.state["plnumberneg"]++;
            if (MaudioXponent.state["plnumberneg"] % 12 == 0) {
                engine.setValue("[Playlist]","SelectTrackKnob",-1);
            }  
        }
    }else{
        if (MaudioXponent.state["scratching"+currentdeck] == 1) { //scratch mode on
	    engine.scratchTick(currentdeck, value-64);
        } else {  //normal wheel mode
            engine.setValue("[Channel"+currentdeck+"]", "jog", (value-64)/8);
        }
    }
};

MaudioXponent.wheelbuton = function(channel, control, value, status) {
    var currentdeck = channel+1;
    if (MaudioXponent.state["scrmode"+currentdeck] == 1) { //scratch mode on
	engine.scratchEnable(currentdeck, 3*128, 33+1/3, 1.0/8, (1.0/8)/32);
	MaudioXponent.state["scratching"+currentdeck] = true;
    }
};

MaudioXponent.wheelbutoff = function(channel, control, value, status) {
    var currentdeck = channel+1;
    engine.scratchDisable (currentdeck);
    MaudioXponent.state["scratching"+currentdeck] = false;
};

MaudioXponent.pauseScript = function(ms) {
    startDate = new Date();
    currentDate = null;
    while(currentDate-startDate < ms) currentDate = new Date();
};

MaudioXponent.playPositionMeter1 = function(value) {
    if (value >= 0.75) {
	MaudioXponent.flashdur1++;;
	if (MaudioXponent.flashdur1 == MaudioXponent.flashprogress) {
	    midi.sendShortMsg(0xB3,0x14,0x00);
	}
	if (MaudioXponent.flashdur1 >= MaudioXponent.flashprogress*2) {
	    midi.sendShortMsg(0xB3,0x14,MaudioXponent.convert(value));
	    MaudioXponent.flashdur1 = 0;
	}
    }else{
	midi.sendShortMsg(0xB3,0x14,MaudioXponent.convert(value));
    }
};

MaudioXponent.playPositionMeter2 = function(value) {
    if (value >= 0.75) {
	MaudioXponent.flashdur2++;;
	if (MaudioXponent.flashdur2 == MaudioXponent.flashprogress) {
	    midi.sendShortMsg(0xB3,0x15,0x00);
	}
	if (MaudioXponent.flashdur2 >= MaudioXponent.flashprogress*2) {
	    midi.sendShortMsg(0xB3,0x15,MaudioXponent.convert(value));
	    MaudioXponent.flashdur2 = 0;
	}
    }else{
	midi.sendShortMsg(0xB3,0x15,MaudioXponent.convert(value));
    }
};

MaudioXponent.volumeLEDs1 = function(value) {
    midi.sendShortMsg(0xB3,0x12,MaudioXponent.convert(value));
};

MaudioXponent.volumeLEDs2 = function(value) {
    midi.sendShortMsg(0xB3,0x13,MaudioXponent.convert(value));
};

MaudioXponent.convert = function(value) {
    value = value*127;
    value = value.toFixed(0);
    value = value.toString(16);
    return (value);
};

MaudioXponent.actbin = function(channel, control, value, status) {    // Generic binary button
    script.debug(channel, control, value, status);
    var currentdeck = channel+1;
    var activenow = engine.getValue("[Channel"+currentdeck+"]", MaudioXponent.binleds[control]);
    if (activenow) {    // If currently active
        engine.setValue("[Channel"+currentdeck+"]",MaudioXponent.binleds[control], 0);    // Stop
        midi.sendShortMsg(MaudioXponent.off + channel,control, 0x00);    // Turn off the LED
    }else {    // If not currently active
        engine.setValue("[Channel"+currentdeck+"]",MaudioXponent.binleds[control], 1);    // Start
        midi.sendShortMsg(MaudioXponent.on + channel,control, 0x01);    // Turn on the LED
    }
};

MaudioXponent.actbinstop = function(channel, control, value, status) {    // Generic binary button
    script.debug(channel, control, value, status);
    var currentdeck = channel+1;
    engine.setValue("[Channel"+currentdeck+"]",MaudioXponent.binleds[control],0);    // Stop
    midi.sendShortMsg(MaudioXponent.off + channel,control,0x00);    // Turn off the LED
};

MaudioXponent.hotcue = function(channel, control, value, status) {    // Generic binary button
    var channelnow = 0;
    if (control == "[Channel2]") {
	channelnow = 1;
    }
    switch (value) {
    case "hotcue_1_enabled":
        if (channel == 1){
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x17,0x01);    // Turn on the LED
        }else{
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x17,0x00);    // Turn off the LED
        }
	break;
    case "hotcue_2_enabled":
        if (channel == 1){
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x18,0x01);    // Turn on the LED
        }else{
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x18,0x00);    // Turn off the LED
        }
	break;
    case "hotcue_3_enabled":
        if (channel == 1){
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x19,0x01);    // Turn on the LED
        }else{
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x19,0x00);    // Turn off the LED
        }
	break;
    case "hotcue_4_enabled":
        if (channel == 1){
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x1A,0x01);    // Turn on the LED
        }else{
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x1A,0x00);    // Turn off the LED
        }
	break;
    case "hotcue_5_enabled":
        if (channel == 1){
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x1B,0x01);    // Turn on the LED
        }else{
            midi.sendShortMsg(MaudioXponent.on + channelnow,0x1B,0x00);    // Turn off the LED
        }
	break;
    }
};

MaudioXponent.hotcueset = function(channel, control, value, status) {
    //MaudioXponent.debug(channel, control, value, status)
    var currentdeck = channel+1;
    if (MaudioXponent.state["shift"+currentdeck] == 1) {
        engine.setValue("[Channel"+currentdeck+"]","hotcue_"+MaudioXponent.hotcues[control]+"_clear",1);
    }else{
        engine.setValue("[Channel"+currentdeck+"]","hotcue_"+MaudioXponent.hotcues[control]+"_activate",1);
    }
};

MaudioXponent.onLoopIn = function(channel, control, value, status) 
{
    channelnow = control == "[Channel2]" ? 1 : 0;
    midi.sendShortMsg(MaudioXponent.on + channelnow, 0x29, 
		      engine.getValue (control, value) != -1);
    /* TODO.
    if (-1 == engine.getValue(control, "loop_end_position")) 
    {
	MaudioXponent.state["flashes"] = 0;  // initialize number of flashes
	MaudioXponent.timer[channelnow] = engine.beginTimer(
	    250, "MaudioXponent.flashled("+ channelnow +",\"0x2B\")");
    }
    */
}

MaudioXponent.onLoopOut = function(channel, control, value, status) 
{
    channelnow = control == "[Channel2]" ? 1 : 0;
    /*
    if (MaudioXponent.timer[channelnow] != -1) { 
	engine.stopTimer (MaudioXponent.timer[channelnow]);
	MaudioXponent.timer[channelnow] = -1;
    }
    */
    midi.sendShortMsg(MaudioXponent.on + channelnow, 0x2B, 
		      engine.getValue (control, value) != -1);
}

MaudioXponent.onLoopExit = function(channel, control, value, status) 
{
    channelnow = control == "[Channel2]" ? 1 : 0;
    midi.sendShortMsg(MaudioXponent.on + channelnow, 0x2A, 
		      engine.getValue (control, value) == 1);
}

MaudioXponent.loopin = function(channel, control, value, status) {
    //MaudioXponent.debug(channel, control, value, status)
    var currentdeck = channel+1;
    engine.setValue("[Channel"+currentdeck+"]","loop_in", 1);
};

MaudioXponent.loopout = function(channel, control, value, status) {
    //MaudioXponent.debug(channel, control, value, status)
    var currentdeck = channel+1;
    engine.setValue("[Channel"+currentdeck+"]", "loop_out", 1);
};

MaudioXponent.loopexit = function(channel, control, value, status) {
    var currentdeck = channel+1;
    var activenow = engine.getValue("[Channel"+currentdeck+"]", "reloop_exit");
    engine.setValue("[Channel"+currentdeck+"]", "reloop_exit", 1);
};

MaudioXponent.flashled = function (channel, control) {
    MaudioXponent.state["flashes"]++;
    if (MaudioXponent.state["flashes"] % 2 == 0) {
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
    }else {
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x00);    // Turn off the LED
    }
};

MaudioXponent.faderbuttonon = function(channel, control, value, status) {
    //script.debug(channel, control, value, status);
    MaudioXponent.state["faderpos"] = engine.getValue("[Master]","crossfader");
    if (MaudioXponent.state["faderpos"] <= -0.90 && channel == 1) {
	midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
	engine.setValue("[Master]","crossfader",0);
    }else if (MaudioXponent.state["faderpos"] >= 0.90 && channel == 0) {
	midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
	engine.setValue("[Master]","crossfader",0);
    }
};

MaudioXponent.faderbuttonoff = function(channel, control, value, status) {
    //script.debug(channel, control, value, status);
    if (MaudioXponent.state["faderpos"] <= -0.90 && channel == 1) {
	midi.sendShortMsg(MaudioXponent.on + channel,control,0x00);    // Turn off the LED
	engine.setValue("[Master]","crossfader",MaudioXponent.state["faderpos"]);
    }else if (MaudioXponent.state["faderpos"] >= 0.90 && channel == 0) {
	midi.sendShortMsg(MaudioXponent.on + channel,control,0x00);    // Turn off the LED
	engine.setValue("[Master]","crossfader",MaudioXponent.state["faderpos"]);
    }
};

MaudioXponent.bpmsync = function(channel, control, value) {
    print (channel);
    print (control);
};

MaudioXponent.pitch = function(channel, control, value, status) {
    var currentdeck = channel+1;
    engine.setValue("[Channel"+currentdeck+"]","rate",script.pitch(control, value, status));
};

MaudioXponent.secondaryon = function(channel, control, value, status) {
    var currentdeck = channel+1;
    midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
    MaudioXponent.state["shift"+currentdeck] = 1;
};

MaudioXponent.secondaryoff = function(channel, control, value, status) {
    var currentdeck = channel+1;
    midi.sendShortMsg(MaudioXponent.on + channel,control,0x00);    // Turn on the LED
    MaudioXponent.state["shift"+currentdeck] = 0;
};

MaudioXponent.flanger = function(channel, control, value, status) {
    //script.debug(channel, control, value, status);
    if (control == 12) {
        var activenow = engine.getValue("[Channel1]","flanger");
        if (activenow == 1) {    // If currently active
            midi.sendShortMsg(MaudioXponent.on+1,control,0x00);    // Turn off the LED
            engine.setValue("[Channel1]","flanger",0);
        }else{
            midi.sendShortMsg(MaudioXponent.on+1,control,0x01);    // Turn on the LED
            engine.setValue("[Channel1]","flanger",1);
        }
    }else if (control == 13) {
        var activenow = engine.getValue("[Channel2]","flanger");
        if (activenow == 1) {    // If currently active
            midi.sendShortMsg(MaudioXponent.on+1,control,0x00);    // Turn off the LED
            engine.setValue("[Channel2]","flanger",0);
        }else{
            midi.sendShortMsg(MaudioXponent.on+1,control,0x01);    // Turn on the LED
            engine.setValue("[Channel2]","flanger",1);
        }
    }
};

MaudioXponent.scrmode = function(channel, control, value, status) {
    var currentdeck = channel+1;
    if (MaudioXponent.state["scrmode"+currentdeck] == 1) {
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x00);    // Turn off the LED
        MaudioXponent.state["scrmode"+currentdeck] = 0;
    }else{
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
        MaudioXponent.state["scrmode"+currentdeck] = 1;
    }
};

MaudioXponent.playlist = function(channel, control, value, status) {
    var currentdeck = channel+1;
    switch (control) {
    case 28:
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
        engine.setValue("[Playlist]","SelectPrevTrack",1);
	break;
    case 29:
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
        engine.setValue("[Playlist]","SelectNextTrack",1);
	break;
    case 30:
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
        var activenow = engine.getValue("[Channel"+currentdeck+"]","play");
        if (activenow == 1) {    // If currently active
            engine.setValue("[Playlist]","LoadSelectedIntoFirstStopped",1);
        }else{
            engine.setValue("[Channel"+currentdeck+"]","LoadSelectedTrack",1);
        }
	break;
    case 31:
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
        engine.setValue("[Playlist]","SelectPrevPlaylist",1);
	break;
    case 32:
        midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
        engine.setValue("[Playlist]","SelectNextPlaylist",1);
	break;
    }
};

MaudioXponent.playlistoff = function(channel, control, value, status) {
    midi.sendShortMsg(MaudioXponent.off + channel,control,0x00);    // Turn off the LED
};

MaudioXponent.cuedefon = function(channel, control, value, status) {
    var currentdeck = channel+1;
    engine.setValue("[Channel"+currentdeck+"]","cue_default",1);
    midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
};

MaudioXponent.cuedefoff = function(channel, control, value, status) {
    var currentdeck = channel+1;
    engine.setValue("[Channel"+currentdeck+"]","cue_default",0);
    midi.sendShortMsg(MaudioXponent.off + channel,control,0x00);    // Turn off the LED
};

MaudioXponent.volbuttonon = function(channel, control, value, status) {
    //script.debug(channel, control, value, status);
    var currentdeck = channel+1;
    engine.setValue("[Channel"+currentdeck+"]","volume",0);
    midi.sendShortMsg(MaudioXponent.on + channel,control,0x01);    // Turn on the LED
};

MaudioXponent.volbuttonoff = function(channel, control, value, status) {
    //script.debug(channel, control, value, status);
    var currentdeck = channel+1;
    engine.setValue("[Channel"+currentdeck+"]","volume",1);
    midi.sendShortMsg(MaudioXponent.off + channel,control,0x00);    // Turn off the LED
};

MaudioXponent.shutdown = function (id) {    // called when the MIDI device is closed

    for (var led in MaudioXponent.leds) {
	midi.sendShortMsg(MaudioXponent.on,MaudioXponent.leds[led],0x00); // Turn off deck 1 lights
	midi.sendShortMsg(MaudioXponent.on + 1,MaudioXponent.leds[led],0x00); // Turn off deck 2 lights
    }
    midi.sendShortMsg(0xB3,0x14,0x00);
    midi.sendShortMsg(0xB3,0x15,0x00);

};


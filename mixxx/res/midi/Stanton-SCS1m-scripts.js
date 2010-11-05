/****************************************************************/
/*      Stanton SCS.1m MIDI controller script v1.1              */
/*          Copyright (C) 2009, Sean M. Pappalardo              */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.8.x                                 */
/****************************************************************/

function StantonSCS1m() {}

// ----------   Customization variables ----------
//      See http://mixxx.org/wiki/doku.php/stanton_SCS.1m_mixxx_user_guide  for details

// The below values vary with latency and are preset for 10ms. For 2ms, use 0.8, 0.8 and 1.5.

StantonSCS1m.scratchKnob = {    "slippage":0.2,             // Slipperiness of the virtual slipmat when scratching with the select knob (higher=slower response, 0<n<1)
                                "sensitivity":0.5,          // How much the audio moves for a given knob arc (higher=faster response, 0<n<1)
                                "stoppedMultiplier":2.2 };  // Correction for when the deck is stopped (set higher for higher latencies)

// ----------   Other global variables    ----------
StantonSCS1m.debug = false; // Enable/disable debugging messages to the console
StantonSCS1m.faderStart = true; // Allows decks to start when their channel or cross fader is opened (toggleable with the top button)
StantonSCS1m.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
StantonSCS1m.channel = 0;   // MIDI channel the device is on
StantonSCS1m.swVersion = "1.8";   // Mixxx version for display
StantonSCS1m.sysex = [0xF0, 0x00, 0x01, 0x02];  // Preamble for all SysEx messages for this device
StantonSCS1m.modifier = { };    // Modifier buttons (allowing alternate controls) defined on-the-fly if needed
StantonSCS1m.selectKnobMode = "browse"; // Current mode for the gray select knob
StantonSCS1m.inSetup = false; // Flag for if the device is in setup mode
StantonSCS1m.revtime = 1.8;   // Time in seconds for the virtual record to spin once. Used for calculating the position LEDs (1.8 for 33 1/3 RPM)
StantonSCS1m.trackDuration = [0,0]; // Duration of the song on each deck (used for vinyl LEDs)
StantonSCS1m.lastLight = [-1,-1];   // Last circle LED values
StantonSCS1m.lastTime = ["-99:99","-99:99"];    // Last time remaining values
StantonSCS1m.lastColor = [0,0]; // Last color of display flash
StantonSCS1m.displayFlash = [-1,-1];  // Temp storage for display flash timeouts
StantonSCS1m.lastCrossFader = 0;  // Last value of the cross fader
StantonSCS1m.lastFader = [0,0];   // Last value of each channel fader
StantonSCS1m.scratchDeck = 1;   // Current deck being scratched with the select knob
StantonSCS1m.hotCueDeck = 1;    // Current hot cue page
StantonSCS1m.cuePoints =  {     1:{ 35:-0.1, 36:-0.1, 37:-0.1, 38:-0.1 },
                                2:{ 35:-0.1, 36:-0.1, 37:-0.1, 38:-0.1 }    };

// ----------   Functions   ----------

StantonSCS1m.init = function (id) {    // called when the MIDI device is opened & set up
    
    StantonSCS1m.id = id;   // Store the ID of this device for later use
    
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 17, 0xF7]),7); // Extinguish all LEDs

//    var CC = 0xB0 + StantonSCS1m.channel;
    var No = 0x90 + StantonSCS1m.channel;

    // Welcome message
    var message = "Welcome";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 1],message.toInt(), 0xF7),7+message.length);   // Set LCD1
    midi.sendShortMsg(No,49,127);   // to orange
    message = "to";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 2],message.toInt(), 0xF7),7+message.length);   // Set LCD2
    midi.sendShortMsg(No,49+1,127); // to orange
    message = "Mixxx  v";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 3],message.toInt(), 0xF7),7+message.length);   // Set LCD3
    midi.sendShortMsg(No,49+2,127); // to orange
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 4],StantonSCS1m.swVersion.toInt(), 0xF7),7+StantonSCS1m.swVersion.length);   // Set LCD4
    midi.sendShortMsg(No,49+3,32);  // to red
    
//    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 16, 0xF7]),7); // Light all LEDs

//    for (i=0x48; i<=0x5c; i++) midi.sendShortMsg(No,i,0x40); // Set surface LEDs to black default

    // Connect signals
        // Vu Meters
    engine.connectControl("[Master]","VuMeterL","StantonSCS1m.MasterLVu");
    engine.connectControl("[Master]","VuMeterR","StantonSCS1m.MasterRVu");
    engine.connectControl("[Channel1]","VuMeter","StantonSCS1m.Channel1Vu");
    engine.connectControl("[Channel2]","VuMeter","StantonSCS1m.Channel2Vu");
    
        // Pitch displays
    engine.connectControl("[Channel1]","rate","StantonSCS1m.pitchDisplay1");
    engine.connectControl("[Channel2]","rate","StantonSCS1m.pitchDisplay2");
    
        // Pitch display colors
    engine.connectControl("[Channel1]","rateRange","StantonSCS1m.pitchColor1");
    engine.connectControl("[Channel2]","rateRange","StantonSCS1m.pitchColor2");
    
        // Virtual platter LEDs & time displays
    engine.connectControl("[Channel1]","visual_playposition","StantonSCS1m.positionUpdates1");
    engine.connectControl("[Channel2]","visual_playposition","StantonSCS1m.positionUpdates2");
    engine.connectControl("[Channel1]","duration","StantonSCS1m.durationChange1");
    engine.connectControl("[Channel2]","duration","StantonSCS1m.durationChange2");
    
        // Faders
    engine.connectControl("[Master]","crossfader","StantonSCS1m.crossFaderStart");
    engine.connectControl("[Channel1]","volume","StantonSCS1m.ch1FaderStart");
    engine.connectControl("[Channel2]","volume","StantonSCS1m.ch2FaderStart");
    
	//  Initialize the vinyl LEDs if the mapping is loaded after a song is
    StantonSCS1m.durationChange1(engine.getValue("[Channel1]","duration"));
    StantonSCS1m.durationChange2(engine.getValue("[Channel2]","duration"));
    
    // Set LCD text
    StantonSCS1m.initLCDs();

    StantonSCS1m.browseButton(StantonSCS1m.channel, 0x20, 0x7F, 0x90+StantonSCS1m.channel); // Force into browse mode

    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 34, 32, 0xF7]),8); // Get position of all pots
    
    // Button LEDs
    if (StantonSCS1m.faderStart) midi.sendShortMsg(No,8,1);
    
    var LEDInit = [ ["Channels","rate_temp_up"],
                    ["Channels","rate_temp_down"],
                    ["Channels","pfl"],
                    ["Channels","beatsync"],
                    ["Channels","play"],
                    ["Channels","cue_default"]     ];
    
    for (i=0; i<LEDInit.length; i++) {
        var group = LEDInit[i][0];
        var name = LEDInit[i][1];
        if (group=="Channels") {
            engine.trigger("[Channel1]",name);
            engine.trigger("[Channel2]",name);
            }
        else engine.trigger(group,name);
    }

    print ("StantonSCS1m: \""+StantonSCS1m.id+"\" on MIDI channel "+(StantonSCS1m.channel+1)+" initialized.");
}

StantonSCS1m.shutdown = function () {   // called when the MIDI device is closed

    // Graffiti :)
    var message = "Mixxx";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 1],message.toInt(), 0xF7),7+message.length);   // Set LCD1
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 2],StantonSCS1m.swVersion.toInt(), 0xF7),7+StantonSCS1m.swVersion.length);   // Set LCD2
    message = "was here";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 3],message.toInt(), 0xF7),7+message.length);   // Set LCD3
    message = "";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 4],message.toInt(), 0xF7),7+message.length);   // Set LCD4
    
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 17, 0xF7]),7); // Extinguish all LEDs
    print ("StantonSCS1m: \""+StantonSCS1m.id+"\" on MIDI channel "+(StantonSCS1m.channel+1)+" shut down.");
}

StantonSCS1m.initLCDs = function () {
    var No = 0x90 + StantonSCS1m.channel;
    var message = "Pitch 1";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 1],message.toInt(), 0xF7),7+message.length);   // Set LCD1
    StantonSCS1m.pitchColor1(engine.getValue("[Channel1]","rateRange"));
    message = " Pitch 2";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 4],message.toInt(), 0xF7),7+message.length);   // Set LCD4
    StantonSCS1m.pitchColor2(engine.getValue("[Channel2]","rateRange"));
    message = " Deck 1";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 2],message.toInt(), 0xF7),7+message.length);   // Set LCD2
    midi.sendShortMsg(No,49+1,32);   // to red
    message = " Deck 2";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 3],message.toInt(), 0xF7),7+message.length);   // Set LCD3
    midi.sendShortMsg(No,49+2,32);   // to red
}

StantonSCS1m.checkInSetup = function () {
  if (StantonSCS1m.inSetup) print ("StantonSCS1m: In setup mode, ignoring command.");
  return StantonSCS1m.inSetup;
}

StantonSCS1m.playButton1 = function (channel, control, value, status) {
    StantonSCS1m.playButton(channel, control, value, status, 1);
}

StantonSCS1m.playButton2 = function (channel, control, value, status) {
    StantonSCS1m.playButton(channel, control, value, status, 2);
}

StantonSCS1m.playButton = function (channel, control, value, status, deck) {
    if (StantonSCS1m.debug) print("Play button"+deck);
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {    // If button down
        StantonSCS1m.modifier["play"+deck]=1;
        if (StantonSCS1m.modifier["cue"+deck]==1) engine.setValue("[Channel"+deck+"]","play",1);
        else {
            var currentlyPlaying = engine.getValue("[Channel"+deck+"]","play");
            if (currentlyPlaying && engine.getValue("[Channel"+deck+"]","cue_default")==1) engine.setValue("[Channel"+deck+"]","cue_default",0);
            engine.setValue("[Channel"+deck+"]","play", !currentlyPlaying);
        }
        return;
    }
    StantonSCS1m.modifier["play"+deck]=0;
}

StantonSCS1m.cueButton1 = function (channel, control, value, status) {
    StantonSCS1m.cueButton(channel, control, value, status, 1);
}

StantonSCS1m.cueButton2 = function (channel, control, value, status) {
    StantonSCS1m.cueButton(channel, control, value, status, 2);
}

StantonSCS1m.cueButton = function (channel, control, value, status, deck) {
    if (StantonSCS1m.debug) print("Cue button"+deck);
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) != 0x80) {    // If button down
        engine.setValue("[Channel"+deck+"]","cue_default",1);
        StantonSCS1m.modifier["cue"+deck]=1;   // Set button modifier flag
        return;
    }
    if (StantonSCS1m.modifier["play"+deck]==0) engine.setValue("[Channel"+deck+"]","cue_default",0);
    StantonSCS1m.modifier["cue"+deck]=0;   // Clear button modifier flag
}

StantonSCS1m.setupButton = function (channel, control, value, status) {
    if ((status & 0XF0) == 0x90) StantonSCS1m.inSetup = !StantonSCS1m.inSetup;
    else if (StantonSCS1m.inSetup) {  // If entering setup, change the LCD back light colors to green
        var No = 0x90 + channel;
        midi.sendShortMsg(No,49,96);
        midi.sendShortMsg(No,49+3,96);
        midi.sendShortMsg(No,49+1,96);
        midi.sendShortMsg(No,49+2,96);
    }
    else {
        StantonSCS1m.initLCDs(); // If out of setup & the button was released, restore the displays 
        StantonSCS1m.lastTime = ["-99:99","-99:99"];    // Force time displays to update
        engine.trigger("[Channel1]","rate");    // Force pitch displays to update
        engine.trigger("[Channel2]","rate");
    }
}

StantonSCS1m.controlButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
    var byte1 = 0x90 + channel;
    if ((status & 0XF0) == 0x90) {    // If button down
        midi.sendShortMsg(byte1,control,64);  // Light 'er up
        StantonSCS1m.selectKnobMode = "control";
        midi.sendShortMsg(0x80+channel,32,0);  // turn off the "browse" mode button
        
        if (StantonSCS1m.scratchDeck==1) {
            midi.sendShortMsg(byte1,24,32); // Light left (cancel) button green
            midi.sendShortMsg(0x80+channel,26,0);   // Exinguish right (enter) button
        }
        else {
            midi.sendShortMsg(byte1,26,32); // Light right (enter) button green
            midi.sendShortMsg(0x80+channel,24,0);   // Exinguish left (cancel) button
        }
        return;
    }
}

StantonSCS1m.browseButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
    var byte1 = 0x90 + channel;
    if ((status & 0XF0) == 0x90) {    // If button down
        midi.sendShortMsg(byte1,control,64);  // Light 'er up
        StantonSCS1m.selectKnobMode = "browse";
        midi.sendShortMsg(0x80+channel,30,0);  // turn off the "control" mode button
        
        midi.sendShortMsg(0x80+channel,24,0);   // Exinguish left (cancel) button
        midi.sendShortMsg(0x80+channel,26,0);   // Exinguish right (enter) button
        return;
    }
}

StantonSCS1m.selectKnob = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
  
  jogValue = (value-64); // -64 to +63, - = CCW, + = CW
  
    switch (StantonSCS1m.selectKnobMode) {
        case "control":
            group = "[Channel"+StantonSCS1m.scratchDeck+"]";
            if (engine.getValue(group,"play")==1 && engine.getValue(group,"reverse")==1) jogValue= -(jogValue);
            multiplier = StantonSCS1m.scratchKnob["sensitivity"] * (engine.getValue(group,"play") ? 1 : StantonSCS1m.scratchKnob["stoppedMultiplier"] );
//            if (StantonSCS1m.debug) print("do scratching VALUE:" + value + " jogValue: " + jogValue );
            engine.setValue(group,"scratch", (engine.getValue(group,"scratch") + (jogValue * multiplier)).toFixed(2));

            break;
        case "browse":
            engine.setValue("[Playlist]","SelectTrackKnob", jogValue);
            break;
    }
}

StantonSCS1m.pressSelectKnob = function () {
  if (StantonSCS1m.checkInSetup()) return;
    switch (StantonSCS1m.selectKnobMode) {
        case "control":
            //engine.setValue("[Channel"+StantonSCS1m.scratchDeck+"]","scratch",0);
            break;
        case "browse":
            engine.setValue("[Playlist]","LoadSelectedIntoFirstStopped",1);
            break;
    }
}

StantonSCS1m.cancelButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
    switch (StantonSCS1m.selectKnobMode) {
        case "control":
            if (StantonSCS1m.scratchDeck!=1) {
                StantonSCS1m.scratchDeck=1; // Scratch deck 1
                midi.sendShortMsg(0x90+channel,24,32); // Light left (cancel) button green
                midi.sendShortMsg(0x80+channel,26,0);   // Exinguish right (enter) button
            }
            break;
        case "browse":
            break;
    }
}

StantonSCS1m.enterButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
    switch (StantonSCS1m.selectKnobMode) {
        case "control":
            if (StantonSCS1m.scratchDeck!=2) {
                StantonSCS1m.scratchDeck=2; // Scratch deck 1
                midi.sendShortMsg(0x90+channel,26,32); // Light right (enter) button green
                midi.sendShortMsg(0x80+channel,24,0);   // Exinguish left (cancel) button
            }
            break;
        case "browse":
            break;
    }
}

StantonSCS1m.encoderJog1 = function (channel, control, value, status) {
    StantonSCS1m.encoderJog(value,1);
}

StantonSCS1m.encoderJog2 = function (channel, control, value, status) {
    StantonSCS1m.encoderJog(value,2);
}

StantonSCS1m.encoderJog = function (value,deck) {
    if (StantonSCS1m.checkInSetup()) return;
    jogValue=(value-64);
    if (engine.getValue("[Channel"+deck+"]","play")==1 && engine.getValue("[Channel"+deck+"]","reverse")==1) jogValue= -(jogValue);
    engine.setValue("[Channel"+deck+"]","jog",jogValue*4);
}

StantonSCS1m.pitchRangeKnob1 = function (channel, control, value, status) {
    StantonSCS1m.pitchRangeKnob(value,1);
}

StantonSCS1m.pitchRangeKnob2 = function (channel, control, value, status) {
    StantonSCS1m.pitchRangeKnob(value,2);
}

StantonSCS1m.pitchRangeKnob = function (value,deck) {
    if (StantonSCS1m.checkInSetup()) return;
    var currentValue = engine.getValue("[Channel"+deck+"]","rateRange");
    var newValue = Math.round(currentValue*100+(value-64))*0.01;
    if (newValue<=0.01) newValue=0.01;
    if (newValue>1) newValue=1;
    engine.setValue("[Channel"+deck+"]","rateRange",newValue);
    engine.trigger("[Channel"+deck+"]","rate"); // Force GUI to update
    StantonSCS1m.pitchDisplay(engine.getValue("[Channel"+deck+"]","rate"),deck); // Force LCDs to update
}

StantonSCS1m.faderStartToggle = function (channel, control, value, status) {
    StantonSCS1m.faderStart = !StantonSCS1m.faderStart;
    var No = 0x90 + channel;
    if (StantonSCS1m.faderStart) midi.sendShortMsg(No,8,1);
    else midi.sendShortMsg(No,8,0);
}

StantonSCS1m.hotCueDeckToggle = function (channel, control, value, status) {
    if (StantonSCS1m.checkInSetup()) return;
    if ((status & 0XF0) == 0x90) {    // If button down
        StantonSCS1m.modifier["hotCueToggle"]=1;    // Set modifier flag (to allow cue deletion)
        StantonSCS1m.modifier["hotCueToggleTime"] = new Date();  // Store the current time in milliseconds
    }
    else {    // If button up
        StantonSCS1m.modifier["hotCueToggle"]=0;
        // If the button was held down for over 1/3 of a second, stay on the current cue page
        if (StantonSCS1m.modifier["hotCueToggleTime"] != 0.0 && ((new Date() - StantonSCS1m.modifier["hotCueToggleTime"])>300)) return;
        
        if (StantonSCS1m.hotCueDeck==2) {
            StantonSCS1m.hotCueDeck--;
            midi.sendShortMsg(0x80 + channel,34,0);
            }
        else {
            StantonSCS1m.hotCueDeck++;
            midi.sendShortMsg(0x90 + channel,34,64);
        }
        // Light the Preset buttons if any cues are set
        for (i=35; i<=38; i++) {
            if (StantonSCS1m.cuePoints[StantonSCS1m.hotCueDeck][i] != -0.1) 
                midi.sendShortMsg(0x90 + channel,i,64);
            else midi.sendShortMsg(0x80 + channel,i,0);
        }
    }
}

StantonSCS1m.presetButton = function (channel, control, value, status) {
    if (StantonSCS1m.checkInSetup()) return;
    if ((status & 0xF0) == 0x90) {    // If button down
        midi.sendShortMsg(0x90 + channel,control,1); // Turn on button light
        // Multiple cue points
        if (StantonSCS1m.modifier["hotCueToggle"]==1) {
            StantonSCS1m.cuePoints[StantonSCS1m.hotCueDeck][control] = -0.1;
            midi.sendShortMsg(0x80 + channel,control,0);    // Turn off button light
        }
        else {
            if (StantonSCS1m.cuePoints[StantonSCS1m.hotCueDeck][control] == -0.1)
                StantonSCS1m.cuePoints[StantonSCS1m.hotCueDeck][control] = engine.getValue("[Channel"+StantonSCS1m.hotCueDeck+"]","visual_playposition");
            else engine.setValue("[Channel"+StantonSCS1m.hotCueDeck+"]","playposition",StantonSCS1m.cuePoints[StantonSCS1m.hotCueDeck][control]);
        }
    }
}

// ----------   Slot functions  ----------

StantonSCS1m.crossFaderStart = function (value) {

  if (!StantonSCS1m.faderStart) return;
  if (StantonSCS1m.lastCrossFader==value) return;

  if (value==-1.0 && engine.getValue("[Channel2]","play")==1) {
    engine.setValue("[Channel2]","cue_default",1);
    engine.setValue("[Channel2]","cue_default",0);
    }
  if (value==1.0 && engine.getValue("[Channel1]","play")==1) {
    engine.setValue("[Channel1]","cue_default",1);
    engine.setValue("[Channel1]","cue_default",0);
    }
  if (StantonSCS1m.lastCrossFader==-1.0) engine.setValue("[Channel2]","play",1);
  if (StantonSCS1m.lastCrossFader==1.0) engine.setValue("[Channel1]","play",1);
  
  StantonSCS1m.lastCrossFader=value;
}

StantonSCS1m.ch1FaderStart = function (value) {
  StantonSCS1m.channelFaderStart(value,1);
}

StantonSCS1m.ch2FaderStart = function (value) {
  StantonSCS1m.channelFaderStart(value,2);
}

StantonSCS1m.channelFaderStart = function (value,deck) {

  if (!StantonSCS1m.faderStart) return;
  if (StantonSCS1m.lastFader[deck]==value) return;

  if (value==0 && engine.getValue("[Channel"+deck+"]","play")==1) {
    engine.setValue("[Channel"+deck+"]","cue_default",1);
    engine.setValue("[Channel"+deck+"]","cue_default",0);
    }
  if (StantonSCS1m.lastFader[deck]==0) engine.setValue("[Channel"+deck+"]","play",1);
  
  StantonSCS1m.lastFader[deck]=value;
}

// ----------   LED slot functions  ----------

StantonSCS1m.buttonLED = function (value, note, on, off) {
    var byte1 = 0x90 + StantonSCS1m.channel;
    if (value>0) midi.sendShortMsg(byte1,note,on);
    else midi.sendShortMsg(byte1,note,off);
}


// Vu Meters

StantonSCS1m.Channel1Vu = function (value) {
    StantonSCS1m.VuMeter(value,93);
}

StantonSCS1m.Channel2Vu = function (value) {
    StantonSCS1m.VuMeter(value,94);
}

StantonSCS1m.MasterLVu = function (value) {
    StantonSCS1m.VuMeter(value,96);
}

StantonSCS1m.MasterRVu = function (value) {
    StantonSCS1m.VuMeter(value,97);
}

StantonSCS1m.VuMeter = function (value,note) {
    var on = 0x90 + StantonSCS1m.channel;
    var off = 0x80 + StantonSCS1m.channel;
    var range = 0.125;  // 1/8
    if (value>0.001) midi.sendShortMsg(on,note,0);
    else midi.sendShortMsg(off,note,0);
    if (value>range) midi.sendShortMsg(on,note,1);
    else midi.sendShortMsg(off,note,1);
    if (value>range*2) midi.sendShortMsg(on,note,2);
    else midi.sendShortMsg(off,note,2);
    if (value>range*3) midi.sendShortMsg(on,note,3);
    else midi.sendShortMsg(off,note,3);
    if (value>range*4) midi.sendShortMsg(on,note,4);
    else midi.sendShortMsg(off,note,4);
    if (value>range*5) midi.sendShortMsg(on,note,5);
    else midi.sendShortMsg(off,note,5);
    if (value>range*6) midi.sendShortMsg(on,note,6);
    else midi.sendShortMsg(off,note,6);
    if (value>range*7) midi.sendShortMsg(on,note,7);
    else midi.sendShortMsg(off,note,7);
    if (value>=range*8) midi.sendShortMsg(on,note,8);
    else midi.sendShortMsg(off,note,8);
}

// LCD displays ("scribble strips")

StantonSCS1m.pitchDisplay1 = function (value) {
    if (StantonSCS1m.inSetup) return;
    StantonSCS1m.pitchDisplay(value,1);
}

StantonSCS1m.pitchDisplay2 = function (value) {
    if (StantonSCS1m.inSetup) return;
    StantonSCS1m.pitchDisplay(value,2);
}

StantonSCS1m.pitchDisplay = function (value,deck) {
    var data = (value * engine.getValue("[Channel"+deck+"]","rateRange") * 100);
    if (engine.getValue("[Channel"+deck+"]","rate_dir") == -1) data=-data;  // Show the correct sign depending on the preferences
    data = data.toFixed(2); // Always two decimal places
    if (data>0) data = "+"+data;
    data = data + "%";
    var message = "";
    for (i=data.length; i<8; i++) message += " ";  // Pad the left side with spaces so the decimal point doesn't move around
    message += data;
    var LCD;
    switch (deck) {
        case 1: LCD=1; break;
        case 2: LCD=4; break;
    }
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, LCD],message.toInt(), 0xF7),7+message.length);   // Set LCD with pitch value
}

StantonSCS1m.pitchColor1 = function (value) {
    StantonSCS1m.pitchColor(value,1);
}

StantonSCS1m.pitchColor2 = function (value) {
    StantonSCS1m.pitchColor(value,2);
}

StantonSCS1m.pitchColor = function (value, deck) {
    var No = 0x90 + StantonSCS1m.channel;
    var offset=0;
    if (deck==2) offset=3;

    if (value<=0.25) midi.sendShortMsg(No,49+offset,96);
    else if (value<=0.5) midi.sendShortMsg(No,49+offset,127);
    else if (value>0.5) midi.sendShortMsg(No,49+offset,32);
    engine.trigger("[Channel"+deck+"]","rate");    // Force pitch display to update
}

// Virtual platter rings & time displays

StantonSCS1m.durationChange1 = function (value) {
    StantonSCS1m.trackDuration[1]=value;
    StantonSCS1m.displayFlash[1]=-1;
}

StantonSCS1m.durationChange2 = function (value) {
    StantonSCS1m.trackDuration[2]=value;
    StantonSCS1m.displayFlash[2]=-1;
}

StantonSCS1m.positionUpdates1 = function (value) {
    StantonSCS1m.positionUpdates(value,1);
}

StantonSCS1m.positionUpdates2 = function (value) {
    StantonSCS1m.positionUpdates(value,2);
}

StantonSCS1m.positionUpdates = function (value,deck) {
    StantonSCS1m.wheelDecay(value); // Take care of scratching
    
    // Revolution time of the imaginary record in seconds
    var revtime = StantonSCS1m.revtime;
    var currentTrackPos = value * StantonSCS1m.trackDuration[deck];
    
    var revolutions = currentTrackPos/revtime;
    var light = ((revolutions-((revolutions)|0))*18) | 0; // OR with 0 replaces Math.floor and is faster
//     print("----------Light="+light+", Revolutions="+revolutions+", Remainder="+(revolutions-Math.floor(revolutions)));

    if (StantonSCS1m.lastLight[deck]!=light) {  // Don't send light commands if there's no visible change
        var byte1 = 0xB0 + StantonSCS1m.channel;
        if (deck==1) midi.sendShortMsg(byte1,0x7E,light+1);
        else midi.sendShortMsg(byte1,0x7D,light+1);
        StantonSCS1m.lastLight[deck]=light;
    }   // End platter lights

    
    if (!StantonSCS1m.inSetup) {    // If not in setup mode
        // Show track time remaining
        var trackTimeRemaining = ((1-value) * StantonSCS1m.trackDuration[deck]) | 0;    // OR with 0 replaces Math.floor and is faster
        var message = "-"+secondstominutes(trackTimeRemaining);
        var No = 0x90 + StantonSCS1m.channel;
        if (StantonSCS1m.lastTime[deck]!=message) { // Only send the message if its different
            if (trackTimeRemaining>30) midi.sendShortMsg(No,49+deck,32);    // Set backlight to red
            midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, deck+1],message.toInt(), 0xF7),7+message.length);
            StantonSCS1m.lastTime[deck]=message;
        }
        // Flash near the end of the track
        // TODO: Use a timer for this
        if (trackTimeRemaining<=30 && trackTimeRemaining>15) {   // If <30s left, flash the LCD slowly
            if (StantonSCS1m.displayFlash[deck]==-1) StantonSCS1m.displayFlash[deck] = new Date();
            if (new Date() - StantonSCS1m.displayFlash[deck]>500) {
                StantonSCS1m.displayFlash[deck] = new Date();
                if (StantonSCS1m.lastColor[deck]==32) StantonSCS1m.lastColor[deck]=96; // green
                else StantonSCS1m.lastColor[deck]=32;  // red
                midi.sendShortMsg(No,49+deck,StantonSCS1m.lastColor[deck]);
            }
        } else if (trackTimeRemaining<=15 && trackTimeRemaining>0) { // If <15s left, flash quickly
            if (StantonSCS1m.displayFlash[deck]==-1) StantonSCS1m.displayFlash[deck] = new Date();
            if (new Date() - StantonSCS1m.displayFlash[deck]>125) {
                StantonSCS1m.displayFlash[deck] = new Date();
                if (StantonSCS1m.lastColor[deck]==32) StantonSCS1m.lastColor[deck]=96; // green
                else StantonSCS1m.lastColor[deck]=32;  // red
                midi.sendShortMsg(No,49+deck,StantonSCS1m.lastColor[deck]);
            }
        }   // End flashing
    }
}

StantonSCS1m.wheelDecay = function (value) {

     if (StantonSCS1m.selectKnobMode=="control") {    // do some scratching
        
        scratch = engine.getValue("[Channel"+StantonSCS1m.scratchDeck+"]","scratch");
        jogDecayRate = StantonSCS1m.scratchKnob["slippage"] * (engine.getValue("[Channel"+StantonSCS1m.scratchDeck+"]","play") ? 1 : 1.1 );
        
        if (StantonSCS1m.debug) print("Scratch deck"+StantonSCS1m.scratchDeck+": " + scratch + ", Jog decay rate="+jogDecayRate);
         
        if (scratch != 0) {
            if (Math.abs(scratch) > jogDecayRate*0.001) {  
                  engine.setValue("[Channel"+StantonSCS1m.scratchDeck+"]","scratch", (scratch * jogDecayRate).toFixed(4));
               } else {
                  engine.setValue("[Channel"+StantonSCS1m.scratchDeck+"]","scratch", 0);
               }
            }
     }
}

/* TODO:
- Add cueplay fix?
*/
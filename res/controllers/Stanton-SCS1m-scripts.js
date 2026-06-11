/****************************************************************/
/*      Stanton SCS.1m MIDI controller script v1.20             */
/*          Copyright (C) 2009-2011, Sean M. Pappalardo         */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.9.x                                 */
/****************************************************************/

function StantonSCS1m() {}

// ----------   Customization variables ----------
//      See http://mixxx.org/wiki/doku.php/stanton_SCS.1m_mixxx_user_guide for details

StantonSCS1m.faderStart = false;    // Allows decks to start when their channel or cross fader is opened (toggleable with the top button)
StantonSCS1m.scratchFactor = 2;     // Adjusts the speed of scratching with the select knob

// ----------   Other global variables    ----------
StantonSCS1m.debug = false; // Enable/disable debugging messages to the console
StantonSCS1m.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
StantonSCS1m.channel = 0;   // MIDI channel the device is on
StantonSCS1m.swVersion = "1.9";   // Mixxx version for display
StantonSCS1m.sysex = [0xF0, 0x00, 0x01, 0x02];  // Preamble for all SysEx messages for this device
StantonSCS1m.modifier = { };    // Modifier buttons (allowing alternate controls) defined on-the-fly if needed
StantonSCS1m.selectKnobMode = "browse"; // Current mode for the gray select knob
StantonSCS1m.inSetup = false; // Flag for if the device is in setup mode
StantonSCS1m.revtime = 1.8;   // Time in seconds for the virtual record to spin once. Used for calculating the position LEDs (1.8 for 33 1/3 RPM)
StantonSCS1m.trackDuration = [0,0]; // Duration of the song on each deck (used for vinyl LEDs)
StantonSCS1m.state = { };   // Temporary state variables
StantonSCS1m.timer = { };   // Temporary storage of timer IDs
StantonSCS1m.lastLight = [-1,-1];   // Last circle LED values
StantonSCS1m.lastTime = ["-99:99","-99:99"];    // Last time remaining values
StantonSCS1m.lastVu = { 92:0, 93:0, 94:0, 95:0, 96:0, 97:0 };  // Last VU meter values
StantonSCS1m.lastCrossFader = 0;  // Last value of the cross fader
StantonSCS1m.lastFader = [0,0];   // Last value of each channel fader
StantonSCS1m.scratchDeck = 0;   // Current deck being scratched with the select knob
StantonSCS1m.scratch = {    "rpm":(33+1/3)*StantonSCS1m.scratchFactor,
                            "resolution":30, "alpha":1.0/8, "beta":(1.0/8)/32 };
StantonSCS1m.hotCueDeck = 1;    // Current hot cue page
StantonSCS1m.hotCues =  {   1:{ 35:1, 36:2, 37:3, 38:4 },
                            2:{ 35:1, 36:2, 37:3, 38:4 }    };

// Signals to (dis)connect: Group, Key, Function name
StantonSCS1m.hotcueSignals = [  ["CurrentChannel", "hotcue_1_enabled", "StantonSCS1m.Preset1LED"],
                                ["CurrentChannel", "hotcue_2_enabled", "StantonSCS1m.Preset2LED"],
                                ["CurrentChannel", "hotcue_3_enabled", "StantonSCS1m.Preset3LED"],
                                ["CurrentChannel", "hotcue_4_enabled", "StantonSCS1m.Preset4LED"] ];

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
    engine.connectControl("[Main]", "vu_meter_left", "StantonSCS1m.MasterLVu");
    engine.connectControl("[Main]", "vu_meter_right", "StantonSCS1m.MasterRVu");
    engine.connectControl("[Channel1]","vu_meter","StantonSCS1m.Channel1Vu");
    engine.connectControl("[Channel2]","vu_meter","StantonSCS1m.Channel2Vu");

        // Clipping LED
    engine.connectControl("[Master]","peak_indicator","StantonSCS1m.MasterClip");
    engine.connectControl("[Channel1]","peak_indicator","StantonSCS1m.Channel1Clip");
    engine.connectControl("[Channel2]","peak_indicator","StantonSCS1m.Channel2Clip");

        // Pitch displays
    engine.connectControl("[Channel1]","rate","StantonSCS1m.pitchDisplay1");
    engine.connectControl("[Channel2]","rate","StantonSCS1m.pitchDisplay2");

        // Pitch display colors
    engine.connectControl("[Channel1]","rateRange","StantonSCS1m.pitchColor1");
    engine.connectControl("[Channel2]","rateRange","StantonSCS1m.pitchColor2");

    // Virtual platter LEDs & time displays
    engine.connectControl("[Channel1]", "playposition", "StantonSCS1m.positionUpdates1");
    engine.connectControl("[Channel2]", "playposition", "StantonSCS1m.positionUpdates2");
    engine.connectControl("[Channel1]", "duration", "StantonSCS1m.durationChange1");
    engine.connectControl("[Channel2]", "duration", "StantonSCS1m.durationChange2");

        // Faders
    engine.connectControl("[Master]","crossfader","StantonSCS1m.crossFaderStart");
    engine.connectControl("[Channel1]","volume","StantonSCS1m.ch1FaderStart");
    engine.connectControl("[Channel2]","volume","StantonSCS1m.ch2FaderStart");

	//  Initialize the vinyl LEDs if the mapping is loaded after a song is
    StantonSCS1m.durationChange1(engine.getValue("[Channel1]","duration"));
    StantonSCS1m.durationChange2(engine.getValue("[Channel2]","duration"));

    // Force change to first deck, initializing the LEDs and connecting signals in the process
    // Set active deck to last available so the below will switch to #1.
    StantonSCS1m.hotCueDeck = engine.getValue("[App]", "num_decks");
    StantonSCS1m.hotCueDeckChange(StantonSCS1m.channel, 34, 0x7F, 0x90+StantonSCS1m.channel);
    StantonSCS1m.hotCueDeckChange(StantonSCS1m.channel, 34, 0x00, 0x80+StantonSCS1m.channel);

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

// (Dis)connects the appropriate Mixxx control signals to/from functions based on the currently controlled deck and what mode the controller section is in
StantonSCS1m.connectPresetSignals = function (channel, disconnect) {

    var signalList = StantonSCS1m.hotcueSignals;
    for (i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        if (group=="CurrentChannel") group = "[Channel"+StantonSCS1m.hotCueDeck+"]";
        engine.connectControl(group,signalList[i][1],signalList[i][2],disconnect);

        // If connecting a signal, cause it to fire (by setting it to the same value) to update the LEDs
//         if (!disconnect) engine.trigger(group,signalList[i][1]);  // Commented because there's no sense in wasting queue length
        if (!disconnect) {
            // Alternate:
            var command = signalList[i][2]+"("+engine.getValue(group,signalList[i][1])+")";
//             print("StantonSCS1m: command="+command);
            eval(command);
        }
        if (StantonSCS1m.debug) {
            if (disconnect) print("StantonSCS1m: "+group+","+signalList[i][1]+" disconnected from "+signalList[i][2]);
            else print("StantonSCS1m: "+group+","+signalList[i][1]+" connected to "+signalList[i][2]);
        }
    }
    // If disconnecting signals, darken the LEDs on the preset buttons
    if (disconnect) {
        var No = 0x80 + channel;
        for (i=35; i<=38; i++) midi.sendShortMsg(No,i,0);
    }
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
        var currentlyPlaying = engine.getValue("[Channel"+deck+"]","play");
        engine.setValue("[Channel"+deck+"]","play", !currentlyPlaying);
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
    engine.setValue("[Channel"+deck+"]","cue_default",0);
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

        midi.sendShortMsg(byte1,24,32); // Cancel and Enter buttons green
        midi.sendShortMsg(byte1,26,32);
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
            var group = "[Channel"+StantonSCS1m.scratchDeck+"]";
            if (StantonSCS1m.modifier["enterButton"]==1 || StantonSCS1m.modifier["cancelButton"]==1)
                engine.scratchTick(StantonSCS1m.scratchDeck,jogValue);
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
            break;
        case "browse":
            engine.setValue("[Playlist]","LoadSelectedIntoFirstStopped",1); // This doesn't work in 1.9.x!
            break;
    }
}

StantonSCS1m.cancelButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
  if ((status & 0XF0) == 0x90) {    // If button down
    StantonSCS1m.modifier["cancelButton"]=1;
    switch (StantonSCS1m.selectKnobMode) {
        case "control":
            if (StantonSCS1m.scratchDeck==0) {
                StantonSCS1m.scratchDeck=1; // Scratch deck 1
                engine.scratchEnable(StantonSCS1m.scratchDeck, StantonSCS1m.scratch["resolution"],
                    StantonSCS1m.scratch["rpm"], StantonSCS1m.scratch["alpha"], StantonSCS1m.scratch["beta"]);
                midi.sendShortMsg(0x90+channel,control,127); // Light it orange
            } else midi.sendShortMsg(0x90+channel,control,64); // Light it red
            break;
        case "browse":
            // If the deck is playing and the cross-fader is not completely toward the other deck...
            if (engine.getValue("[Channel1]","play")==1 && engine.getValue("[Master]","crossfader")<1.0) {
                // ...light the button red to show acknowledgement of the press but don't load
                midi.sendShortMsg(0x90+channel,control,64);
                print ("StantonSCS1m: Not loading into deck 1 because it's playing to the Master output.");
            }
            else {
                midi.sendShortMsg(0x90+channel,control,127); // Orange
                engine.setValue("[Channel1]","LoadSelectedTrack",1);
            }
            break;
    }
    return;
  }
  // Button up
  switch (StantonSCS1m.selectKnobMode) {
    case "control":
        if (StantonSCS1m.scratchDeck==1) {
            engine.scratchDisable(StantonSCS1m.scratchDeck);
            StantonSCS1m.scratchDeck=0;
        }
        midi.sendShortMsg(0x90+channel,control,32); // green
        break;
    case "browse":
        midi.sendShortMsg(0x80+channel,control,0); // Off
        engine.setValue("[Channel1]","LoadSelectedTrack",0);
        break;
  }
  StantonSCS1m.modifier["cancelButton"]=0;
}

StantonSCS1m.enterButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
  if ((status & 0XF0) == 0x90) {    // If button down
    StantonSCS1m.modifier["enterButton"]=1;
    switch (StantonSCS1m.selectKnobMode) {
        case "control":
            if (StantonSCS1m.scratchDeck==0) {
                StantonSCS1m.scratchDeck=2;
                engine.scratchEnable(StantonSCS1m.scratchDeck, StantonSCS1m.scratch["resolution"],
                    StantonSCS1m.scratch["rpm"], StantonSCS1m.scratch["alpha"], StantonSCS1m.scratch["beta"]);
                midi.sendShortMsg(0x90+channel,control,127); // Light it orange
            } else midi.sendShortMsg(0x90+channel,control,64); // Light it red
            break;
        case "browse":
            // If the deck is playing and the cross-fader is not completely toward the other deck...
            if (engine.getValue("[Channel2]","play")==1 && engine.getValue("[Master]","crossfader")>-1.0) {
                // ...light the button red to show acknowledgement of the press but don't load
                midi.sendShortMsg(0x90+channel,control,64);
                print ("StantonSCS1m: Not loading into deck 2 because it's playing to the Master output.");
            }
            else {
                midi.sendShortMsg(0x90+channel,control,127); // Orange
                engine.setValue("[Channel2]","LoadSelectedTrack",1);
            }
            break;
    }
    return;
  }
  // Button up
  switch (StantonSCS1m.selectKnobMode) {
    case "control":
        if (StantonSCS1m.scratchDeck==2) {
            engine.scratchDisable(StantonSCS1m.scratchDeck);
            StantonSCS1m.scratchDeck=0;
        }
        midi.sendShortMsg(0x90+channel,control,32); // green
        break;
    case "browse":
        midi.sendShortMsg(0x80+channel,control,0); // Off
        engine.setValue("[Channel2]","LoadSelectedTrack",0);
        break;
  }
  StantonSCS1m.modifier["enterButton"]=0;
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

StantonSCS1m.hotCueDeckChange = function (channel, control, value, status) {
    if (StantonSCS1m.checkInSetup()) return;
    if ((status & 0XF0) == 0x90) {    // If button down
        StantonSCS1m.modifier["hotCueToggle"]=1;    // Set modifier flag (to allow cue deletion)
        StantonSCS1m.modifier["hotCueToggleTime"] = new Date();  // Store the current time in milliseconds
    }
    else {    // If button up
        StantonSCS1m.modifier["hotCueToggle"]=0;
        // If the button was held down for over 1/3 of a second, stay on the current cue page
        if (StantonSCS1m.modifier["hotCueToggleTime"] != 0.0 && ((new Date() - StantonSCS1m.modifier["hotCueToggleTime"])>300)) return;

        StantonSCS1m.connectPresetSignals(channel,true);    // Disconnect previous ones
        if (StantonSCS1m.hotCueDeck == engine.getValue("[App]", "num_decks")) StantonSCS1m.hotCueDeck=1;
        else StantonSCS1m.hotCueDeck++;
        // Change bank button color
        if (StantonSCS1m.hotCueDeck % 2 == 0) midi.sendShortMsg(0x90 + channel,34,64);   // On
        else midi.sendShortMsg(0x80 + channel,34,0);   // Off
        StantonSCS1m.connectPresetSignals(channel,false);   // Connect new ones
    }
}

StantonSCS1m.presetButton = function (channel, control, value, status) {
    if (StantonSCS1m.checkInSetup()) return;
    var deck = StantonSCS1m.hotCueDeck;
    if ((status & 0xF0) == 0x90) {    // If button down
        // Multiple cue points
        if (StantonSCS1m.modifier["hotCueToggle"]==1) { // Delete cue point
            engine.setValue("[Channel"+deck+"]","hotcue_"+StantonSCS1m.hotCues[deck][control]+"_clear",1);
            engine.setValue("[Channel"+deck+"]","hotcue_"+StantonSCS1m.hotCues[deck][control]+"_clear",0);
        }
        else {
            // If hotcue X is set, seeks the player to hotcue X's position.
            // If hotcue X is not set, sets hotcue X to the current play position.
            engine.setValue("[Channel"+deck+"]","hotcue_"+StantonSCS1m.hotCues[deck][control]+"_activate",1);
        }
        return;
    }
    // Button up
    engine.setValue("[Channel"+deck+"]","hotcue_"+StantonSCS1m.hotCues[deck][control]+"_activate",0);
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

StantonSCS1m.Preset1LED = function(value) { StantonSCS1m.PresetLED(value,35);   }
StantonSCS1m.Preset2LED = function(value) { StantonSCS1m.PresetLED(value,36);   }
StantonSCS1m.Preset3LED = function(value) { StantonSCS1m.PresetLED(value,37);   }
StantonSCS1m.Preset4LED = function(value) { StantonSCS1m.PresetLED(value,38);   }

StantonSCS1m.PresetLED = function(value,control) {
    if (value>0) midi.sendShortMsg(0x90 + StantonSCS1m.channel,control,64);
    else midi.sendShortMsg(0x80 + StantonSCS1m.channel,control,0);
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

StantonSCS1m.Channel1Clip = function (value) {
    StantonSCS1m.clipLED(value,93);
}

StantonSCS1m.Channel2Clip = function (value) {
    StantonSCS1m.clipLED(value,94);
}

StantonSCS1m.MasterClip = function (value) {
    StantonSCS1m.clipLED(value,96);
    StantonSCS1m.clipLED(value,97);
}

StantonSCS1m.clipLED = function (value, note) {
    var on = 0x90 + StantonSCS1m.channel;
    var off = 0x80 + StantonSCS1m.channel;
    if (value>0) midi.sendShortMsg(on,note,8);
    else midi.sendShortMsg(off,note,8);
}

StantonSCS1m.VuMeter = function (value,note) {
    var range = 1/7;    // keep the highest one for the clip signal
    var newLEDs=0;
    if (value>=range*7) newLEDs = 255;
    else if (value>range*6) newLEDs = 127;
    else if (value>range*5) newLEDs = 63;
    else if (value>range*4) newLEDs = 31;
    else if (value>range*3) newLEDs = 15;
    else if (value>range*2) newLEDs = 7;
    else if (value>range) newLEDs = 3;
    else if (value>0.001) newLEDs = 1;

    // Last one XOR this one gives the LEDs that changed state
    var change = StantonSCS1m.lastVu[note] ^ newLEDs;
    StantonSCS1m.lastVu[note] = newLEDs;
    if (change == 0) return;

    var on = 0x90 + StantonSCS1m.channel;
    var off = 0x80 + StantonSCS1m.channel;
    var compare=1;
    for (i=0; i<=7; i++) {
        if((change & compare) !=0) { // If this LED changed
            if ((newLEDs & compare) !=0) {  // If the new value is not 0
                midi.sendShortMsg(on,note,i);   // Light it up
                }
            else midi.sendShortMsg(off,note,i); // Turn it off
        }
        compare <<= 1;
    }
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
    // Stop any leftover end-of-track-flash timers
    var deck = 1;
    if (StantonSCS1m.timer["15s-d"+deck] != -1) {
        engine.stopTimer(StantonSCS1m.timer["15s-d"+deck]);
        StantonSCS1m.timer["15s-d"+deck] = -1;
    }
    if (StantonSCS1m.timer["30s-d"+deck] != -1) {
        engine.stopTimer(StantonSCS1m.timer["30s-d"+deck]);
        StantonSCS1m.timer["30s-d"+deck] = -1;
    }
    // Make sure back light is red
    if (StantonSCS1m.state["flash"+deck]!=false) {
        StantonSCS1m.state["flash"+deck]=false;
        midi.sendShortMsg(0x90 + StantonSCS1m.channel,49+deck,32);   //red
    }

    StantonSCS1m.trackDuration[1]=value;
    StantonSCS1m.displayFlash[1]=-1;
}

StantonSCS1m.durationChange2 = function (value) {
    // Stop any leftover end-of-track-flash timers
    var deck = 1;
    if (StantonSCS1m.timer["15s-d"+deck] != -1) {
        engine.stopTimer(StantonSCS1m.timer["15s-d"+deck]);
        StantonSCS1m.timer["15s-d"+deck] = -1;
    }
    if (StantonSCS1m.timer["30s-d"+deck] != -1) {
        engine.stopTimer(StantonSCS1m.timer["30s-d"+deck]);
        StantonSCS1m.timer["30s-d"+deck] = -1;
    }
    // Make sure back light is red
    if (StantonSCS1m.state["flash"+deck]!=false) {
        StantonSCS1m.state["flash"+deck]=false;
        midi.sendShortMsg(0x90 + StantonSCS1m.channel,49+deck,32);   //red
    }

    StantonSCS1m.trackDuration[2]=value;
    StantonSCS1m.displayFlash[2]=-1;
}

StantonSCS1m.positionUpdates1 = function (value) {
    StantonSCS1m.positionUpdates(value,1);
}

StantonSCS1m.positionUpdates2 = function (value) {
    StantonSCS1m.positionUpdates(value,2);
}

StantonSCS1m.displayFlash = function (deck) {
    if (StantonSCS1m.checkInSetup()) return;
    var No = 0x90 + StantonSCS1m.channel;
    if (!StantonSCS1m.state["flash"+deck]) {
        StantonSCS1m.state["flash"+deck]=true;
        midi.sendShortMsg(No,49+deck,96);   //green
    }
    else {
        StantonSCS1m.state["flash"+deck]=false;
        midi.sendShortMsg(No,49+deck,32);   //red
    }
}

StantonSCS1m.positionUpdates = function (value,deck) {
    var No = 0x90 + StantonSCS1m.channel;
    var trackTimeRemaining = ((1-value) * StantonSCS1m.trackDuration[deck]) | 0;    // OR with 0 replaces Math.floor and is faster

    // Flash near the end of the track if the track is longer than 30s
    if (StantonSCS1m.trackDuration[deck]>30) {
        if (trackTimeRemaining<=30 && trackTimeRemaining>15) {   // If <30s left, flash slowly
            if (StantonSCS1m.timer["30s-d"+deck] == -1) {
                // Start timer
                StantonSCS1m.timer["30s-d"+deck] = engine.beginTimer(500, () => { StantonSCS1m.displayFlash(deck); });
                if (StantonSCS1m.timer["15s-d"+deck] != -1) {
                    // Stop the 15s timer if it was running
                    engine.stopTimer(StantonSCS1m.timer["15s-d"+deck]);
                    StantonSCS1m.timer["15s-d"+deck] = -1;
                }
            }
        } else if (trackTimeRemaining<=15 && trackTimeRemaining>0) { // If <15s left, flash quickly
            if (StantonSCS1m.timer["15s-d"+deck] == -1) {
                // Start timer
                StantonSCS1m.timer["15s-d"+deck] = engine.beginTimer(125,() => { StantonSCS1m.displayFlash(deck); });
                if (StantonSCS1m.timer["30s-d"+deck] != -1) {
                    // Stop the 30s timer if it was running
                    engine.stopTimer(StantonSCS1m.timer["30s-d"+deck]);
                    StantonSCS1m.timer["30s-d"+deck] = -1;
                }
            }
        } else {    // Stop flashing
            if (StantonSCS1m.timer["15s-d"+deck] != -1) {
                engine.stopTimer(StantonSCS1m.timer["15s-d"+deck]);
                StantonSCS1m.timer["15s-d"+deck] = -1;
            }
            if (StantonSCS1m.timer["30s-d"+deck] != -1) {
                engine.stopTimer(StantonSCS1m.timer["30s-d"+deck]);
                StantonSCS1m.timer["30s-d"+deck] = -1;
            }
            // Make sure back light is red
            if (StantonSCS1m.state["flash"+deck]!=false) {
                StantonSCS1m.state["flash"+deck]=false;
                midi.sendShortMsg(No,49+deck,32);   //red
            }
        }
    }

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
        var message = "-"+secondstominutes(trackTimeRemaining);
        if (StantonSCS1m.lastTime[deck]!=message) { // Only send the message if its different
            if (trackTimeRemaining>30) midi.sendShortMsg(No,49+deck,32);    // Set backlight to red
            midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, deck+1],message.toInt(), 0xF7),7+message.length);
            StantonSCS1m.lastTime[deck]=message;
        }
    }
}

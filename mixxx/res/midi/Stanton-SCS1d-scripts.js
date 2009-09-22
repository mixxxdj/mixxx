/****************************************************************/
/*      Stanton SCS.1d MIDI controller script vPre              */
/*          Copyright (C) 2009, Sean M. Pappalardo              */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.7.0                                 */
/****************************************************************/

function StantonSCS1d() {}

// ----------   Customization variables ----------
//      See http://mixxx.org/wiki/doku.php/stanton_scs.1d_mixxx_user_guide  for details
StantonSCS1d.pitchRanges = [ 0.08, 0.16, 0.25, 0.5 ];   // Pitch ranges (can add more, but the .rangeButton function would need to be extended.)
StantonSCS1d.fastDeckChange = true;     // Skip the flashy lights if true, for juggling
StantonSCS1d.globalMode = false;        // Stay in the current modes on deck changes if true
StantonSCS1d.platterSpeed = 0;          // Speed of the platter at 0% pitch: 0=33 RPM, 1=45 RPM
StantonSCS1d.deckChangeWait = 1000;     // Time in milliseconds to hold the Deck Change button down to avoid changing decks
StantonSCS1d.padVelocity = true;        // Use the velocity values when recalling cues on the trigger pads

// These values are heavily latency-dependent. They're preset for 10ms and will need tuning for other latencies. (For 2ms, try 0.885, 0.15, and 1.5.)
StantonSCS1d.scratching = {     "slippage":1,             // Slipperiness of the virtual slipmat when scratching with the circle (higher=slower response, 0<n<1)
                                "sensitivity":0.11,          // How much the audio moves for a given circle arc (higher=faster response, 0<n<1)
                                "stoppedMultiplier":1.7 };  // Correction for when the deck is stopped (set higher for higher latencies)

// ----------   Other global variables    ----------
StantonSCS1d.debug = true;  // Enable/disable debugging messages to the console
StantonSCS1d.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
StantonSCS1d.channel = 0;   // MIDI channel the device is on
StantonSCS1d.swVersion = "1.7.0";   // Mixxx version for display
StantonSCS1d.buttons = { "control":27, "browse":28, "vinyl":29, "deckSelect":64 };

StantonSCS1d.platterMode = { "[Channel1]":"vinyl", "[Channel2]":"vinyl" };   // Set vinyl mode on both decks
StantonSCS1d.knobMode = { "[Channel1]":1, "[Channel2]":1 }; // 1=Low,Mid,High EQ,Vol; 2=Depth,Delay,LFO,Gain; 3=Cuemix,Headvol,Balance,MasterVol
StantonSCS1d.padBank = { "deck":1, "bank1":1, "bank2":1 };
StantonSCS1d.deck = 1;  // Currently active virtual deck
StantonSCS1d.trackDuration = [0,0]; // Duration of the song on each deck (used for jog LCD and displays)
StantonSCS1d.modifier = { "cue":0, "play":0 };  // Modifier buttons (allowing alternate controls) defined on-the-fly if needed
StantonSCS1d.state = { "pitchAbs":0, "jog":0, "dontMove":0, "platterGrabbed":false };   // Temporary state variables
StantonSCS1d.prevValues = { };  // Temporary previous value storage
StantonSCS1d.inSetup = false;   // Flag for if the device is in setup mode
StantonSCS1d.sysex = [0xF0, 0x00, 0x01, 0x02];  // Preamble for all SysEx messages for this device
// Variables used in the scratching alpha-beta filter: (revtime = 1.8 to start)
StantonSCS1d.scratch = { "revtime":1.8, "alpha":0.1, "beta":1.0 };

// Pitch values for key change mode
StantonSCS1d.pitchPoints = {    1:{ 8:-0.1998, 9:-0.1665, 10:-0.1332, 11:-0.0999, 12:-0.0666, 13:-0.0333,
                                    14:0.0333, 15:0.0666, 18:0.0999, 19:0.1332, 20:0.1665, 21:0.1998 }, // 3.33% increments
                                2:{ 8:-0.5, 9:-0.4043, 10:-0.2905, 11:-0.1567, 12:-0.1058, 13:-0.0548, 
                                    14:0.06, 15:0.12, 18:0.181, 19:0.416, 20:0.688, 21:1.0 },  // Key changes
                                3:{ 8:-0.4370, 9:-0.3677, 10:-0.3320, 11:-0.2495, 12:-0.1567, 13:-0.0548, 
                                    14:0.12, 15:0.263, 18:0.338, 19:0.506, 20:0.688, 21:0.895 } };  // Notes
// Multiple banks of multiple cue points:
StantonSCS1d.cuePoints =  {     1:{ 8:-0.1, 9:-0.1, 10:-0.1, 11:-0.1, 12:-0.1, 13:-0.1,
                                    14:-0.1, 15:-0.1, 18:-0.1, 19:-0.1, 20:-0.1, 21:-0.1 },
                                2:{ 8:-0.1, 9:-0.1, 10:-0.1, 11:-0.1, 12:-0.1, 13:-0.1,
                                    14:-0.1, 15:-0.1, 18:-0.1, 19:-0.1, 20:-0.1, 21:-0.1 }  };
                                    
StantonSCS1d.padPoints =  {     1:{ // Deck
                                    1:{ 0x20:-0.1, 0x21:-0.1, 0x22:-0.1, 0x23:-0.1 },   // Bank
                                    2:{ 0x20:-0.1, 0x21:-0.1, 0x22:-0.1, 0x23:-0.1 },
                                    3:{ 0x20:-0.1, 0x21:-0.1, 0x22:-0.1, 0x23:-0.1 } },
                                2:{ // Deck
                                    1:{ 0x20:-0.1, 0x21:-0.1, 0x22:-0.1, 0x23:-0.1 },   // Bank
                                    2:{ 0x20:-0.1, 0x21:-0.1, 0x22:-0.1, 0x23:-0.1 },
                                    3:{ 0x20:-0.1, 0x21:-0.1, 0x22:-0.1, 0x23:-0.1 } }
                            };

// Signals to (dis)connect by mode: Group, Key, Function name
StantonSCS1d.platterSignals = { "vinyl":[ ["CurrentChannel", "rate", "StantonSCS1d.platterSpeed"] ],
                                "control":[],
                                "browse":[],
                                "none":[]  // To avoid an error on forced mode changes
                            };
StantonSCS1d.deckSignals = [    ["CurrentChannel", "rateRange", "StantonSCS1d.pitchRangeLEDs"],
                                ["CurrentChannel", "rate", "StantonSCS1d.pitchChange"],
                                ["CurrentChannel", "play", "StantonSCS1d.playLED"],
                                ["CurrentChannel", "reverse", "StantonSCS1d.reverse"],
                                ["CurrentChannel", "cue_default", "StantonSCS1d.cueLED"],
                                ["CurrentChannel", "beatsync", "StantonSCS1d.syncLED"],
                                ["CurrentChannel", "back", "StantonSCS1d.backLED"],
                                ["CurrentChannel", "fwd", "StantonSCS1d.fwdLED"],
                                ["CurrentChannel", "pfl", "StantonSCS1d.headphoneLED"]
                            ];
StantonSCS1d.knobText =    [  ["Low EQ","Mid EQ","High EQ","Volume"],
                              ["Depth","Delay","Period","PF Gain"],
                              ["Pre/Main","Head Vol","Balance","M.Volume"]
                            ];
StantonSCS1d.knobSignals = [  [ ["CurrentChannel", "filterLow", "StantonSCS1d.encoder1EQLEDs"],
                                ["CurrentChannel", "filterMid", "StantonSCS1d.encoder2EQLEDs"],
                                ["CurrentChannel", "filterHigh", "StantonSCS1d.encoder3EQLEDs"],
                                ["CurrentChannel", "volume", "StantonSCS1d.encoder4VolumeLEDs"] ],
                              [ ["[Flanger]", "lfoDepth", "StantonSCS1d.FXDepthLEDs"],
                                ["[Flanger]", "lfoDelay", "StantonSCS1d.FXDelayLEDs"],
                                ["[Flanger]", "lfoPeriod", "StantonSCS1d.FXPeriodLEDs"],
                                ["CurrentChannel", "pregain", "StantonSCS1d.encoder4EQLEDs"] ],
                              [ ["[Master]", "headMix", "StantonSCS1d.encoder1BalanceLEDs"],
                                ["[Master]", "headVolume", "StantonSCS1d.encoder2MVolumeLEDs"],
                                ["[Master]", "balance", "StantonSCS1d.encoder3BalanceLEDs"],
                                ["[Master]", "volume", "StantonSCS1d.encoder4MVolumeLEDs"] ],
                            ];

// ----------   Functions   ----------

StantonSCS1d.init = function (id) {    // called when the MIDI device is opened & set up

    // Welcome message
    var message = "Welcome";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 1],message.toInt(), 0xF7),7+message.length);   // Set LCD1
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 6],message.toInt(), 0xF7),7+message.length);   // Set LCD6
    //midi.sendShortMsg(No,49,127);   // to orange
    message = "to";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 2],message.toInt(), 0xF7),7+message.length);   // Set LCD2
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 7],message.toInt(), 0xF7),7+message.length);   // Set LCD7
    //midi.sendShortMsg(No,49+1,127); // to orange
    message = "Mixxx  v";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 3],message.toInt(), 0xF7),7+message.length);   // Set LCD3
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 8],message.toInt(), 0xF7),7+message.length);   // Set LCD8
    //midi.sendShortMsg(No,49+2,127); // to orange
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 4],StantonSCS1d.swVersion.toInt(), 0xF7),7+StantonSCS1d.swVersion.length);   // Set LCD4
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 9],StantonSCS1d.swVersion.toInt(), 0xF7),7+StantonSCS1d.swVersion.length);   // Set LCD9
    //midi.sendShortMsg(No,49+3,32);  // to red
    
    StantonSCS1d.id = id;   // Store the ID of this device for later use
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 17, 0xF7]),7); // Extinguish all LEDs
    var CC = 0xB0 + StantonSCS1d.channel;
    var No = 0x90 + StantonSCS1d.channel;

    midi.sendShortMsg(CC,1,'x'.toInt());   // Stop platter
    if (StantonSCS1d.platterSpeed==1) midi.sendShortMsg(CC,1,'2'.toInt());   // 45 RPM
    else midi.sendShortMsg(CC,1,'1'.toInt());   // 33 RPM
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 14, 0, 0xF7]),8);  // Clear Passive mode

    //for (var i=0x48; i<=0x5c; i++) midi.sendShortMsg(No,i,0x40); // Set surface LEDs to black default
    
    // Force change to first deck, initializing the LEDs and connecting signals in the process
    StantonSCS1d.state["Oldknob"]=1;
    StantonSCS1d.deck = 2;  // Set active deck to right (#2) so the below will switch to #1.
    StantonSCS1d.DeckChange(StantonSCS1d.channel, StantonSCS1d.buttons["deckSelect"], "", 0x80+StantonSCS1d.channel);
    
    // Connect the playposition functions permanently since they disrupt playback if connected on the fly
    engine.connectControl("[Channel1]","visual_playposition","StantonSCS1d.circleBars1");
    engine.connectControl("[Channel2]","visual_playposition","StantonSCS1d.circleBars2");
    engine.connectControl("[Channel1]","duration","StantonSCS1d.durationChange1");
    engine.connectControl("[Channel2]","duration","StantonSCS1d.durationChange2");
    engine.connectControl("[Channel2]","play", "StantonSCS1d.playLED");
    
    //midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 16, 0xF7]),7); // Light all LEDs

    print ("StantonSCS1d: \""+StantonSCS1d.id+"\" on MIDI channel "+(StantonSCS1d.channel+1)+" initialized.");
}

StantonSCS1d.shutdown = function () {   // called when the MIDI device is closed
    var CC = 0xB0 + StantonSCS1d.channel;
    midi.sendShortMsg(CC,1,'x'.toInt());   // Stop the platter

    // Graffiti :)
    var message = "Mixxx";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 1],message.toInt(), 0xF7),7+message.length);   // Set LCD1
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 6],message.toInt(), 0xF7),7+message.length);   // Set LCD6
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 2],StantonSCS1d.swVersion.toInt(), 0xF7),7+StantonSCS1d.swVersion.length);   // Set LCD2
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 7],StantonSCS1d.swVersion.toInt(), 0xF7),7+StantonSCS1d.swVersion.length);   // Set LCD7
    message = "was here";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 3],message.toInt(), 0xF7),7+message.length);   // Set LCD3
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 8],message.toInt(), 0xF7),7+message.length);   // Set LCD8
    message = "";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 4],message.toInt(), 0xF7),7+message.length);   // Set LCD4
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 9],message.toInt(), 0xF7),7+message.length);   // Set LCD9
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 36, 0x20, 0xF7]),8);    // clear jog LCD character (set to space)
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 17, 0xF7]),7); // Extinguish all LEDs
    print ("StantonSCS1d: \""+StantonSCS1d.id+"\" on MIDI channel "+(StantonSCS1d.channel+1)+" shut down.");
}

StantonSCS1d.checkInSetup = function () {
  if (StantonSCS1d.inSetup) print ("StantonSCS1d: In setup mode, ignoring command.");
  return StantonSCS1d.inSetup;
}

StantonSCS1d.setupButton = function (channel, control, value, status) {
    if ((status & 0XF0) == 0x90) {
        StantonSCS1d.inSetup = !StantonSCS1d.inSetup;
        if (StantonSCS1d.inSetup) StantonSCS1d.connectKnobSignals(channel,true);   // Disconnect knob signals & turn off their LEDs
        }
    else if (!StantonSCS1d.inSetup) StantonSCS1d.connectKnobSignals(channel);   // Reconnect knob signals
}

StantonSCS1d.controlButton = function (channel, control, value, status) {
  if (StantonSCS1d.checkInSetup()) return;
    var byte1 = 0x90 + channel;
    if ((status & 0XF0) == 0x90) {    // If button down
        midi.sendShortMsg(0xB0 + channel,1,'x'.toInt());   // Stop platter
        midi.sendShortMsg(byte1,control,1);  // Light 'er up
        StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = "control";
        midi.sendShortMsg(0x80+channel,0x1C,0);  // turn off the "browse" mode button
        midi.sendShortMsg(0x80+channel,0x1D,0);  // turn off the "vinyl" mode button
        
        return;
    }
}

StantonSCS1d.browseButton = function (channel, control, value, status) {
  if (StantonSCS1d.checkInSetup()) return;
    var byte1 = 0x90 + channel;
    if ((status & 0XF0) == 0x90) {    // If button down
        midi.sendShortMsg(0xB0 + channel,1,'x'.toInt());   // Stop platter
        midi.sendShortMsg(byte1,control,1);  // Light 'er up
        StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = "browse";
        midi.sendShortMsg(0x80+channel,0x1B,0);  // turn off the "control" mode button
        midi.sendShortMsg(0x80+channel,0x1D,0);  // turn off the "vinyl" mode button
        
        return;
    }
}

StantonSCS1d.vinylButton = function (channel, control, value, status) {
  if (StantonSCS1d.checkInSetup()) return;
    var byte1 = 0x90 + channel;
    if ((status & 0XF0) == 0x90) {    // If button down
        midi.sendShortMsg(byte1,control,1);  // Light 'er up
        StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = "vinyl";
        midi.sendShortMsg(0x80+channel,0x1B,0);  // turn off the "control" mode button
        midi.sendShortMsg(0x80+channel,0x1C,0);  // turn off the "browse" mode button
        
        StantonSCS1d.pitchChange(engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate")); // So the platter speed is updated
        StantonSCS1d.playLED(engine.getValue("[Channel"+StantonSCS1d.deck+"]","play")); // So the platter begins spinning if applicable

        return;
    }
}

// (Dis)connects the appropriate Mixxx control signals to/from functions based on the currently controlled deck and what mode the encoder knobs are in
StantonSCS1d.connectKnobSignals = function (channel, disconnect) {

    var signalList = StantonSCS1d.knobSignals[StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"]-1];
    for (var i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        if (group=="CurrentChannel") group = "[Channel"+StantonSCS1d.deck+"]";
        engine.connectControl(group,signalList[i][1],signalList[i][2],disconnect);
        
        // If connecting a signal, cause it to fire (by setting it to the same value) to update the LEDs
//         if (!disconnect) engine.trigger(group,signalList[i][1]);  // Commented because there's no sense in wasting queue length
        if (!disconnect) {
            // Alternate:
            var command = signalList[i][2]+"("+engine.getValue(group,signalList[i][1])+")";
            //print("StantonSCS1d: command="+command);
            eval(command);
            
            // Set text
            var message = StantonSCS1d.knobText[StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"]-1][i];
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, i+1],message.toInt(), 0xF7),7+message.length);   // Set LCD
        }
        if (StantonSCS1d.debug) {
            if (disconnect) print("StantonSCS1d: "+group+","+signalList[i][1]+" disconnected from "+signalList[i][2]);
            else print("StantonSCS1d: "+group+","+signalList[i][1]+" connected to "+signalList[i][2]);
        }
    }
    // If disconnecting signals, darken the LEDs on the knobs
    if (disconnect) {
        var CC = 0xB0 + channel;
        midi.sendShortMsg(CC,0x7F,0x00);  // Encoder 1 LEDs off
        midi.sendShortMsg(CC,0x7E,0x00);  // Encoder 2 LEDs off
        midi.sendShortMsg(CC,0x7D,0x00);  // Encoder 3 LEDs off
        midi.sendShortMsg(CC,0x7C,0x00);  // Encoder 4 LEDs off
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 1],0x20, 0xF7),8);   // Blank LCD text
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 2],0x20, 0xF7),8);   // Blank LCD text
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 3],0x20, 0xF7),8);   // Blank LCD text
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 4],0x20, 0xF7),8);   // Blank LCD text
    }
}

// (Dis)connects the mode-independent Mixxx control signals to/from functions based on the currently controlled virtual deck
StantonSCS1d.connectDeckSignals = function (channel, disconnect) {
    var signalList = StantonSCS1d.deckSignals;
    for (var i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        var name = signalList[i][1];
        if (group=="CurrentChannel") group = "[Channel"+StantonSCS1d.deck+"]";
        engine.connectControl(group,name,signalList[i][2],disconnect);
//         print("StantonSCS1d: (dis)connected "+group+","+name+" to/from "+signalList[i][2]);
        
        // If connecting a signal, update the LEDs
        if (!disconnect) {
            switch (name) {
                case "play":
                        var currentValue = engine.getValue(group,name);
//                         print("StantonSCS1d: current value="+currentValue);
                        StantonSCS1d.playLED(currentValue);
                        break;
                case "cue_default":
                case "beatsync": break;
                default:    // Cause the signal to fire to update LEDs
//                         engine.trigger(group,name);  // No sense in wasting queue length if we can do this another way
                    // Alternate:
                        var command = signalList[i][2]+"("+engine.getValue(group,name)+")";
//                         print("StantonSCS1d: command="+command);
                        eval(command);
                        break;
            }
        }
        
        if (StantonSCS1d.debug) {
            if (disconnect) print("StantonSCS1d: "+group+","+signalList[i][1]+" disconnected from "+signalList[i][2]);
            else print("StantonSCS1d: "+group+","+signalList[i][1]+" connected to "+signalList[i][2]);
        }
    }
    // If disconnecting signals, darken the corresponding LEDs
    if (disconnect) {
        var CC = 0xB0 + channel;
        var No = 0x90 + channel;
        midi.sendShortMsg(No,55,0x00);    // Headphone button off
        midi.sendShortMsg(No,0x29,0x00);  // PLAY button off
        midi.sendShortMsg(No,0x2B,0x00);  // CUE button off
        midi.sendShortMsg(No,0x2A,0x00);  // SYNC button off
        midi.sendShortMsg(No,0x28,0x00);  // BPM button off
    }
}

StantonSCS1d.playButton = function (channel, control, value, status) {
    if ((status & 0xF0) != 0x80) {    // If button down
        StantonSCS1d.modifier["play"]=1;
        if (StantonSCS1d.modifier["cue"]==1) engine.setValue("[Channel"+StantonSCS1d.deck+"]","play",1);
        else {
            if (StantonSCS1d.modifier["pad"]==1) {
                // Continue playing (or stop playing) if play is pressed while a pad is held down
                midi.sendShortMsg(0x90+channel,control,127);    // Make it orange
                StantonSCS1d.state["padWasPlaying"]=!StantonSCS1d.state["padWasPlaying"];
                return;
            }
            
            var currentlyPlaying = engine.getValue("[Channel"+StantonSCS1d.deck+"]","play");
            if (currentlyPlaying && engine.getValue("[Channel"+StantonSCS1d.deck+"]","cue_default")==1) engine.setValue("[Channel"+StantonSCS1d.deck+"]","cue_default",0);
            engine.setValue("[Channel"+StantonSCS1d.deck+"]","play", !currentlyPlaying);
        }
        return;
    }
    engine.trigger("[Channel"+StantonSCS1d.deck+"]","play");
    StantonSCS1d.modifier["play"]=0;
}

StantonSCS1d.cueButton = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) != 0x80) {    // If button down
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","cue_default",1);
        StantonSCS1d.modifier["cue"]=1;   // Set button modifier flag
        return;
    }
    if (StantonSCS1d.modifier["play"]==0) engine.setValue("[Channel"+StantonSCS1d.deck+"]","cue_default",0);
    StantonSCS1d.modifier["cue"]=0;   // Clear button modifier flag
}

StantonSCS1d.syncButton = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) != 0x80) {    // If button down
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","beatsync",1);
        return;
    }
    midi.sendShortMsg(byte1,control,0x00);  // SYNC light off
    engine.setValue("[Channel"+StantonSCS1d.deck+"]","beatsync",0);
}

StantonSCS1d.bpmButton = function (channel, control, value, status) {
    StantonSCS1d.buttonLED(value,control,64,0);
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {    // If button down
//         print("StantonSCS1d: TAP");
        bpm.tapButton(StantonSCS1d.deck);
        return;
    }
}

StantonSCS1d.pfl = function (channel, control, value, status) {
    if ((status & 0xF0) != 0x80) {    // If button down
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","pfl",!engine.getValue("[Channel"+StantonSCS1d.deck+"]","pfl"));
    }
}

StantonSCS1d.rew = function (channel, control, value, status) {
    if ((status & 0xF0) == 0x90) {    // If button down
        midi.sendShortMsg(0xB0+channel,1,'4'.toInt());   // 45 RPM backward
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 35, 1.5, 5, 0, 0, 0xF7]),11);  // Motor full speed
        midi.sendShortMsg(0xB0+channel,1,'o'.toInt());   // Start platter
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","back",1);
        return;
    }
    engine.setValue("[Channel"+StantonSCS1d.deck+"]","back",0);
    engine.trigger("[Channel"+StantonSCS1d.deck+"]","reverse"); // Return to correct direction
    StantonSCS1d.pitchChange(engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate")); // and speed
    engine.trigger("[Channel"+StantonSCS1d.deck+"]","play");    // and playback status
}

StantonSCS1d.ffwd = function (channel, control, value, status) {
    if ((status & 0xF0) == 0x90) {    // If button down
        midi.sendShortMsg(0xB0+channel,1,'2'.toInt());   // 45 RPM foreward
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 35, 1.5, 5, 0, 0, 0xF7]),11);  // Motor full speed
        midi.sendShortMsg(0xB0+channel,1,'o'.toInt());   // Start platter
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","fwd",1);
        return;
    }
    engine.setValue("[Channel"+StantonSCS1d.deck+"]","fwd",0);
    engine.trigger("[Channel"+StantonSCS1d.deck+"]","reverse"); // Return to correct direction
    StantonSCS1d.pitchChange(engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate")); // and speed
    engine.trigger("[Channel"+StantonSCS1d.deck+"]","play");    // and playback status
}

StantonSCS1d.rangeButton = function (channel, control, value, status) {
    if ((status & 0xF0) == 0x90) {    // If button down
        midi.sendShortMsg(0x90+StantonSCS1d.channel,control,0x7F);  // Light button LED
        StantonSCS1d.modifier["pitchRange"]=1;   // Set button modifier flag
        // Move to cross-fader position
        StantonSCS1d.pitchRangeLEDs(0); // darken range LEDs
        var xfader = engine.getValue("[Master]","crossfader")*63+64;
        if (StantonSCS1d.debug) print ("Moving slider to "+xfader);
        midi.sendShortMsg(0xB0+StantonSCS1d.channel,0x00,xfader);
        StantonSCS1d.state["crossfaderAdjusted"]=false;
    }
    else {
        midi.sendShortMsg(0x80+StantonSCS1d.channel,control,0); // Darken button LED
        StantonSCS1d.modifier["pitchRange"]=0; // Clear button modifier flag
        midi.sendShortMsg(0xB0+StantonSCS1d.channel,0x00,engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate")*63+64);    // Move to pitch position
        
        if (!StantonSCS1d.state["crossfaderAdjusted"]) {
            // Change the range
            var currentRange = engine.getValue("[Channel"+StantonSCS1d.deck+"]","rateRange");
            switch (true) {
                case (currentRange<=StantonSCS1d.pitchRanges[0]):
                        engine.setValue("[Channel"+StantonSCS1d.deck+"]","rateRange",StantonSCS1d.pitchRanges[1]);
                    break;
                case (currentRange<=StantonSCS1d.pitchRanges[1]):
                        engine.setValue("[Channel"+StantonSCS1d.deck+"]","rateRange",StantonSCS1d.pitchRanges[2]);
                    break;
                case (currentRange<=StantonSCS1d.pitchRanges[2]):
                        engine.setValue("[Channel"+StantonSCS1d.deck+"]","rateRange",StantonSCS1d.pitchRanges[3]);
                    break;
                case (currentRange>=StantonSCS1d.pitchRanges[3]):
                        engine.setValue("[Channel"+StantonSCS1d.deck+"]","rateRange",StantonSCS1d.pitchRanges[0]);
                    break;
            }
            // Update the screen display
            engine.trigger("[Channel"+StantonSCS1d.deck+"]","rate");
        }
        StantonSCS1d.pitchRangeLEDs(engine.getValue("[Channel"+StantonSCS1d.deck+"]","rateRange")); // Light the LEDs again
    }
}

StantonSCS1d.pitchReset = function (channel, control, value, status) {
    if ((status & 0xF0) == 0x90) midi.sendShortMsg(0x90+StantonSCS1d.channel,control,0x7F); // Light button LED
    else {
        midi.sendShortMsg(0x80+StantonSCS1d.channel,control,0); // Darken button LED
        if (StantonSCS1d.modifier["pitchRange"]==1) {
            engine.setValue("[Master]","crossfader",0);
            StantonSCS1d.state["crossfaderAdjusted"]=true;
        }
        else engine.setValue("[Channel"+StantonSCS1d.deck+"]","rate",0);
    }
}

StantonSCS1d.platterGrabbed = function (channel, control, value, status) {
    if (StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] != "vinyl") return;  // Skip if not in vinyl mode
    if (value == 0x7f) {
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","wheel",0);
        StantonSCS1d.state["platterGrabbed"]=true;
        if (engine.getValue("[Channel"+StantonSCS1d.deck+"]","play")==1) {
            engine.setValue("[Channel"+StantonSCS1d.deck+"]","play",0);
            StantonSCS1d.scratch["wasPlaying"] = true;
        }
    }
    if (value == 0) {
        //engine.setValue("[Channel"+StantonSCS1d.deck+"]","wheel",0);
        //engine.setValue("[Channel"+StantonSCS1d.deck+"]","scratch",0);
        StantonSCS1d.state["platterGrabbed"]=false;
    }
}

StantonSCS1d.platterBend = function (channel, control, value, status) {
    //if (!StantonSCS1d.state["platterGrabbed"])
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","wheel",(value-64)/63);
}

StantonSCS1d.platterScratch = function (channel, control, value, status) {
    var currentMode = StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"];
    switch (currentMode) {
        case "browse":
            if ((value-64)>0) {
                engine.setValue("[Playlist]","SelectNextTrack",1);
            }
            else {
                engine.setValue("[Playlist]","SelectPrevTrack",1);
            }
            break;
        case "vinyl":
            var group = "[Channel"+StantonSCS1d.deck+"]";
            var jogValue = (value-64)/3;
            if (engine.getValue(group,"play")==1 && engine.getValue(group,"reverse")==1) jogValue= -(jogValue);
            
            var multiplier = StantonSCS1d.scratching["sensitivity"] * (engine.getValue(group,"play") ? 1 : StantonSCS1d.scratching["stoppedMultiplier"] );
//              if (StantonSCS1d.debug) print("do scratching VALUE:" + value + " jogValue: " + jogValue );
            engine.setValue(group,"scratch", (engine.getValue(group,"scratch") + (jogValue * multiplier)).toFixed(2));
            break;
    }
    return;
    //print("Play="+engine.getValue("[Channel"+StantonSCS1d.deck+"]","play")+ " Wheel="+engine.getValue("[Channel"+StantonSCS1d.deck+"]","wheel")+ " Jog="+engine.getValue("[Channel"+StantonSCS1d.deck+"]","jog")+ " Scratch="+engine.getValue("[Channel"+StantonSCS1d.deck+"]","scratch"));
    //engine.setValue("[Channel"+StantonSCS1d.deck+"]","wheel",engine.getValue("[Channel"+StantonSCS1d.deck+"]","wheel")+(value-64)/63);
    
    // From SCS.1m
    //var jogValue = (value-64)/128; // -64 to +63, - = CCW, + = CW

    //var group = "[Channel"+StantonSCS1d.deck+"]";
    //if (engine.getValue(group,"play")==1 && engine.getValue(group,"reverse")==1) jogValue= -(jogValue);
    //var multiplier = 0.18 * (engine.getValue("[Channel"+StantonSCS1d.deck+"]","play") ? 1 : 2 );
    //if (StantonSCS1d.debug) print("do scratching VALUE:" + value + " jogValue: " + jogValue + " scratch="+ engine.getValue(group,"scratch"));
    //engine.setValue(group,"scratch", (engine.getValue(group,"scratch") + (jogValue * multiplier)).toFixed(2));
    
    // (33+1/3)/60 /1000
    
    var add = ((33+1/3)/60)/100;
    
    if (value==0x3d) add = -add;

    var newPosition = engine.getValue("[Channel"+StantonSCS1d.deck+"]","playposition") + add / engine.getValue("[Channel"+StantonSCS1d.deck+"]",    "duration");
    engine.setValue("[Channel"+StantonSCS1d.deck+"]","playposition",newPosition);
}

StantonSCS1d.scratchDecay = function (value) {

    // do some scratching
    //if (StantonSCS1d.debug) print("Scratch deck"+StantonSCS1d.deck+": " + engine.getValue("[Channel"+StantonSCS1d.deck+"]","scratch"));
    
    //scratch = engine.getValue("[Channel"+StantonSCS1d.deck+"]","scratch");
    //jogDecayRate = StantonSCS1d.slippage * (engine.getValue("[Channel"+StantonSCS1d.deck+"]","play") ? 1 : 0.2 );
     
    //if (scratch != 0) {
    //    if (Math.abs(scratch) > jogDecayRate*0.01) {  
    //          engine.setValue("[Channel"+StantonSCS1d.deck+"]","scratch", (scratch * jogDecayRate).toFixed(4));
    //       } else {
    //          engine.setValue("[Channel"+StantonSCS1d.deck+"]","scratch", 0);
    //       }
    //    }
    
    var scratch = engine.getValue("[Channel"+StantonSCS1d.deck+"]","scratch");
    var jogDecayRate = (engine.getValue("[Channel"+StantonSCS1d.deck+"]","play") ? 0.8 : 0.8 );
    //print ("Latency="+engine.getValue("[Soundcard]","latency"));
    
    //if (StantonSCS1d.debug) print("Scratch deck"+StantonSCS1d.deck+": " + scratch + ", Jog decay rate="+jogDecayRate);
    
    // If it was playing, ramp back to playback speed
    if (StantonSCS1d.scratch["wasPlaying"] && !StantonSCS1d.state["platterGrabbed"]) {
        var rate = engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate") * engine.getValue("[Channel"+StantonSCS1d.deck+"]","rateRange");
        var convergeTo = 1+rate;
        //jogDecayRate = StantonSCS1d.scratching["slippage"] * 0.2;
        if (scratch != convergeTo) { // Thanks to jusics on IRC for help with this part
            if (Math.abs(scratch-convergeTo) > jogDecayRate*0.001) {  
                engine.setValue("[Channel"+StantonSCS1d.deck+"]","scratch", (convergeTo + (scratch-convergeTo) * jogDecayRate).toFixed(5));
                //engine.setValue("[Channel"+StantonSCS1d.deck+"]","scratch", (scratch + (convergeTo - scratch) / jogDecayRate).toFixed(5));
            } else {
                // Once "scratch" has gotten close enough to the play speed, just resume normal playback
                engine.setValue("[Channel"+StantonSCS1d.deck+"]","scratch", 0);
                engine.setValue("[Channel"+StantonSCS1d.deck+"]","play",1);
                StantonSCS1d.scratch["wasPlaying"] = false;
            }
        }
    } else
    if (scratch != 0) { // For regular scratching when stopped or if playing (and ramp down...touch functions set scratch=1 and play=0)
        if (Math.abs(scratch) > jogDecayRate*0.001) {  
              engine.setValue("[Channel"+StantonSCS1d.deck+"]","scratch", (scratch * jogDecayRate).toFixed(4));
           } else {
              engine.setValue("[Channel"+StantonSCS1d.deck+"]","scratch", 0);
           }
    }
}

StantonSCS1d.lightDelay = function () {
    var date = new Date();
    var curDate = null;
    
    do { curDate = new Date(); }
    while(curDate-date < 60);
}

StantonSCS1d.DeckChange = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {  // If button down
        midi.sendShortMsg(byte1,control,127); // Make button orange
        StantonSCS1d.modifier["DeckSelect"]=1;   // Set button modifier flag
        StantonSCS1d.modifier["deckTime"] = new Date();  // Store the current time in milliseconds
        StantonSCS1d.connectKnobSignals(channel,true);   // Disconnect old knob signals & turn off their LEDs
        StantonSCS1d.state["Oldknob"]=StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];   // Store old knob mode
        midi.sendShortMsg(0x80+channel,4,0);  // Darken the knob bank button
        StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"]=3;   // Global controls
        StantonSCS1d.connectKnobSignals(channel);   // Connect knob signals
        return;
    }
    StantonSCS1d.modifier["DeckSelect"]=0;   // Clear button modifier flag
    StantonSCS1d.connectKnobSignals(channel,true);   // Disconnect old knob signals & turn off their LEDs
    StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"]=StantonSCS1d.state["Oldknob"];   // Restore previous mode

    var newPlatterMode;
    // If the button's been held down for over a second, stay on the current deck
    if (new Date() - StantonSCS1d.modifier["deckTime"]>StantonSCS1d.deckChangeWait) {
        //StantonSCS1d.connectKnobSignals(channel);   // Re-connect (restored) knob signals
        // Return to appropriate color
        if (StantonSCS1d.deck==2) midi.sendShortMsg(byte1,control,64); // Deck select button red
        else midi.sendShortMsg(byte1,control,32); // Deck select button green
    }
    else {
        StantonSCS1d.connectDeckSignals(channel,true);    // Disconnect static signals
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 17, 0xF7]),7); // Extinguish all LEDs
        
        if (StantonSCS1d.globalMode) newPlatterMode = StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"];
        if (StantonSCS1d.deck == 2) {
            if (StantonSCS1d.debug) print("StantonSCS1d: Switching to deck 1");
            StantonSCS1d.deck--;
            midi.sendShortMsg(0x80 + channel,control,0); // Deck select button off
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 0, 0xF7]),8); // Jog backlight off
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, 0x20, 0xF7]),8); // Blank jog character
            if (!StantonSCS1d.fastDeckChange) { // Make flashy lights to signal a deck change
                var number='1'.toInt();
                StantonSCS1d.lightDelay();
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 1, 0xF7]),8); // Jog backlight on
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, number, 0xF7]),8); // 2 jog character
                midi.sendShortMsg(byte1,control,32); // Deck select button green
                StantonSCS1d.lightDelay();
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 0, 0xF7]),8); // Jog backlight off
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, 0x20, 0xF7]),8); // Blank jog character
                midi.sendShortMsg(0x80 + channel,control,0); // Deck select button off
                StantonSCS1d.lightDelay();
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 1, 0xF7]),8); // Jog backlight on
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, number, 0xF7]),8); // 2 jog character
                midi.sendShortMsg(byte1,control,32); // Deck select button green
                StantonSCS1d.lightDelay();
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 0, 0xF7]),8); // Jog backlight off
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, 0x20, 0xF7]),8); // Blank jog character
                midi.sendShortMsg(0x80 + channel,control,0); // Deck select button off
                StantonSCS1d.lightDelay();
            }
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 1, 0xF7]),8); // Jog backlight on
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, number, 0xF7]),8); // 2 jog character
                midi.sendShortMsg(byte1,control,32); // Deck select button green
        }
        else {
            if (StantonSCS1d.debug) print("StantonSCS1d: Switching to deck 2");
            StantonSCS1d.deck++;
            midi.sendShortMsg(0x80 + channel,control,0); // Deck select button off
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 0, 0xF7]),8); // Jog backlight off
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, 0x20, 0xF7]),8); // Blank jog character
            if (!StantonSCS1d.fastDeckChange) { // Make flashy lights to signal a deck change
                var number='2'.toInt();
                StantonSCS1d.lightDelay();
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 1, 0xF7]),8); // Jog backlight on
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, number, 0xF7]),8); // 2 jog character
                midi.sendShortMsg(byte1,control,64); // Deck select button red
                StantonSCS1d.lightDelay();
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 0, 0xF7]),8); // Jog backlight off
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, 0x20, 0xF7]),8); // Blank jog character
                midi.sendShortMsg(0x80 + channel,control,0); // Deck select button off
                StantonSCS1d.lightDelay();
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 1, 0xF7]),8); // Jog backlight on
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, number, 0xF7]),8); // 2 jog character
                midi.sendShortMsg(byte1,control,64); // Deck select button red
                StantonSCS1d.lightDelay();
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 0, 0xF7]),8); // Jog backlight off
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, 0x20, 0xF7]),8); // Blank jog character
                midi.sendShortMsg(0x80 + channel,control,0); // Deck select button off
                StantonSCS1d.lightDelay();
            }
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 1, 0xF7]),8); // Jog backlight on
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, number, 0xF7]),8); // 2 jog character
                midi.sendShortMsg(byte1,control,64); // Deck select button red
        }
        StantonSCS1d.connectDeckSignals(channel);    // Connect static signals
        StantonSCS1d.padRefresh();  // Light pad section correctly
    }
    if (StantonSCS1d.globalMode) StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"] = StantonSCS1d.state["Oldknob"];
    else newPlatterMode = StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"];
    
    StantonSCS1d.connectKnobSignals(channel);   // Connect new knob signals & light LEDs & displays
    StantonSCS1d.encoderBank(channel, 4, 0, 0x80);  // Light the bank button the correct color for the mode
    
    switch(newPlatterMode) {
        case "control": StantonSCS1d.controlButton(channel, StantonSCS1d.buttons["control"], value, 0x90 + channel); break;
        case "browse": StantonSCS1d.browseButton(channel, StantonSCS1d.buttons["browse"], value, 0x90 + channel); break;
        case "vinyl": StantonSCS1d.vinylButton(channel, StantonSCS1d.buttons["vinyl"], value, 0x90 + channel); break;
    }
    
}   // End Deck Change function

// ----------   Encoders  ----------

StantonSCS1d.encoderBank = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup() || StantonSCS1d.modifier["DeckSelect"]==1) return;
    
    var newMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    
    if ((status & 0xF0) == 0x80) {  // If button up
        print("Newmode="+newMode);
        switch (newMode) {
            case 1:
                midi.sendShortMsg(0x90+channel,control,32);    // Make button green
                break;
            case 2: // FX params
                midi.sendShortMsg(0x90+channel,control,64);    // Make button red
                break;
            default:
                print("Stanton SCS.1d: Unhandled knob mode #"+newMode);
                midi.sendShortMsg(0x80+channel,control,0);    // Turn button off
                break;
        }
    }
    
    if ((status & 0xF0) == 0x90) {  // If button down
        midi.sendShortMsg(0x90+channel,control,127);    // Make button orange
        StantonSCS1d.connectKnobSignals(channel,true);  // Disconnect old knob signals & turn off their LEDs
        newMode++;
        if (newMode>2) newMode=1;   // Wrap
        StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"] = newMode;
        StantonSCS1d.connectKnobSignals(channel);   // Connect new knob signals & light LEDs & displays
    }
    
    // Use button 1 to toggle the flanger effect if in mode 2 (which adjusts FX params)
    if (newMode==2) {
        engine.connectControl("[Channel"+StantonSCS1d.deck+"]","flanger","StantonSCS1d.FXLED");
        engine.trigger("[Channel"+StantonSCS1d.deck+"]","flanger");
    }
    else {
        engine.connectControl("[Channel"+StantonSCS1d.deck+"]","flanger","StantonSCS1d.FXLED",true);
        midi.sendShortMsg(0x80+channel,0x19,0);    // Turn button off
    }
}

StantonSCS1d.encoder1 = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var offset = (value-64);
    var knobMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    switch(knobMode) {
        case 1: // Low EQ
            switch (status & 0xF0) {
                case 0x90:  // Reset to center
                    StantonSCS1d.encoderSetAbs(knobMode,1,1);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.1;
                    StantonSCS1d.encoderSet(knobMode,1,offset,0,4);
                    break;
            }
            break;
        case 2: // Flanger depth
            switch (status & 0xF0) {
                case 0x90:  // Reset to center
                    StantonSCS1d.encoderSetAbs(knobMode,1,0.5);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.03;
                    StantonSCS1d.encoderSet(knobMode,1,offset,0,1);
                    break;
            }
            break;
        case 3: // Pre/Main mix
            switch (status & 0xF0) {
                case 0x90:  // Reset to Left
                    StantonSCS1d.encoderSetAbs(knobMode,1,-1);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.1;
                    StantonSCS1d.encoderSet(knobMode,1,offset,-1,1);
                    break;
            }
            break;
    }
}

StantonSCS1d.display1button = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var knobMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    switch(knobMode) {
        case 1: // Low EQ momentary kill
            switch (status & 0xF0) {
                case 0x90:
                    midi.sendShortMsg(0x90+channel,control,1);
                    StantonSCS1d.encoderSetAbs(knobMode,1,0,true);
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,1,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    break;
            }
            break;
        case 2: // Toggle Flanger
            switch (status & 0xF0) {
                case 0x90:
                    engine.setValue("[Channel"+StantonSCS1d.deck+"]","flanger",!engine.getValue("[Channel"+StantonSCS1d.deck+"]","flanger"));
                    break;
            }
            break;
        case 3: // Momentary Main mix
            switch (status & 0xF0) {
                case 0x90:
                    midi.sendShortMsg(0x90+channel,control,1);
                    StantonSCS1d.encoderSetAbs(knobMode,1,1,true);
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,1,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    break;
            }
            break;
    }
}

StantonSCS1d.encoder2 = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var offset = (value-64);
    var knobMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    switch(knobMode) {
        case 1: // Mid EQ
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,2,1);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.1;
                    StantonSCS1d.encoderSet(knobMode,2,offset,0,4);
                    break;
            }
            break;
        case 2: // Flanger delay
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,2,4950);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*100;
                    StantonSCS1d.encoderSet(knobMode,2,offset,50,10000);
                    break;
            }
            break;
        case 3: // Headphone volume
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,2,1);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.1;
                    StantonSCS1d.encoderSet(knobMode,2,offset,0,5);
                    break;
            }
            break;
    }
}

StantonSCS1d.display2button = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var knobMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    switch(knobMode) {
        case 1: // Mid EQ momentary kill
            switch (status & 0xF0) {
                case 0x90:
                    midi.sendShortMsg(0x90+channel,control,1);
                    StantonSCS1d.encoderSetAbs(knobMode,2,0,true);
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,2,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    break;
            }
            break;
        case 2: // Flanger delay
            break;
        case 3: // Headphone volume
            break;
    }
}

StantonSCS1d.encoder3 = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var offset = (value-64);
    var knobMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    switch(knobMode) {
        case 1: // High EQ
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,3,1);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.1;
                    StantonSCS1d.encoderSet(knobMode,3,offset,0,4);
                    break;
            }
            break;
        case 2: // Flanger period (LFO)
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,3,1025000);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*10000;
                    StantonSCS1d.encoderSet(knobMode,3,offset,50000,2000000);
                    break;
            }
            break;
        case 3: // Master Balance
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,3,0);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.08;
                    StantonSCS1d.encoderSet(knobMode,3,offset,-1,1);
                    break;
            }
            break;
    }
}

StantonSCS1d.display3button = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var knobMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    switch(knobMode) {
        case 1: // High EQ momentary kill
            switch (status & 0xF0) {
                case 0x90:
                    midi.sendShortMsg(0x90+channel,control,1);
                    StantonSCS1d.encoderSetAbs(knobMode,3,0,true);
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,3,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    break;
            }
            break;
        case 2: // Flanger period
            break;
        case 3: // Master Balance
            switch (status & 0xF0) {
                case 0x90:  // Full right
                    midi.sendShortMsg(0x90+channel,control,1);
                    StantonSCS1d.encoderSetAbs(knobMode,3,1);
                    break;
                case 0x80:  // Full left
                    StantonSCS1d.encoderSetAbs(knobMode,3,-1);
                    midi.sendShortMsg(0x90+channel,control,0);
                    break;
            }
            break;
    }
}

StantonSCS1d.encoder4 = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var offset = (value-64);
    var knobMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    switch(knobMode) {
        case 1: // Channel volume
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,4,1);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.025;
                    StantonSCS1d.encoderSet(knobMode,4,offset,0,1);
                    break;
            }
            break;
        case 2: // Pre-fader gain
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,4,1);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.05;
                    StantonSCS1d.encoderSet(knobMode,4,offset,0,4);
                    break;
            }
            break;
        case 3: // Master volume
            switch (status & 0xF0) {
                case 0x90:  // Reset
                    StantonSCS1d.encoderSetAbs(knobMode,4,1);
                    break;
                case 0xb0:  // Adjust
                    offset = offset*0.05;
                    StantonSCS1d.encoderSet(knobMode,4,offset,0,5);
                    break;
            }
            break;
    }
}

StantonSCS1d.display4button = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup()) return;
    var knobMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    switch(knobMode) {
        case 1: // Channel volume momentary kill
            switch (status & 0xF0) {
                case 0x90:
                    midi.sendShortMsg(0x90+channel,control,1);
                    StantonSCS1d.encoderSetAbs(knobMode,4,0,true);
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,4,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    break;
            }
            break;
        case 2: // Pre-fader gain
            break;
        case 3: // Master volume momentary kill
            switch (status & 0xF0) {
                case 0x90:
                    midi.sendShortMsg(0x90+channel,control,1);
                    StantonSCS1d.encoderSetAbs(knobMode,4,0,true);
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,4,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    break;
            }
            break;
    }
}

StantonSCS1d.encoderSet = function (mode,knob,offset,min,max) {
    var signalList = StantonSCS1d.knobSignals[mode-1];
    var group = signalList[knob-1][0];
    if (group=="CurrentChannel") group = "[Channel"+StantonSCS1d.deck+"]";
    var newValue=engine.getValue(group,signalList[knob-1][1])+offset;
    if (newValue<min) newValue=min;
    if (newValue>max) newValue=max;
    engine.setValue(group,signalList[knob-1][1],newValue);
}

StantonSCS1d.encoderSetAbs = function (mode,knob,value,save) {
    var signalList = StantonSCS1d.knobSignals[mode-1];
    var group = signalList[knob-1][0];
    if (group=="CurrentChannel") group = "[Channel"+StantonSCS1d.deck+"]";
    var key=signalList[knob-1][1];
    // Save value for possible use later
    if (save) StantonSCS1d.prevValues[key]=engine.getValue(group,key);
    // Restore to previous value (for kill buttons)
    if (value=="previous") engine.setValue(group,key,StantonSCS1d.prevValues[key]);
    else engine.setValue(group,key,value)
}

// ----------   Sliders  ----------

StantonSCS1d.pitchSlider = function (channel, control, value) {
    var currentValue = engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate");
    var newValue = (value-64)/63;
    if (newValue<-1) newValue=-1.0;
    if (newValue>1) newValue=1.0;
    StantonSCS1d.state["dontMove"]=new Date();
    if (StantonSCS1d.modifier["pitchRange"]==1) {
        engine.setValue("[Master]","crossfader",newValue);
        StantonSCS1d.state["crossfaderAdjusted"]=true;
    }
    else engine.setValue("[Channel"+StantonSCS1d.deck+"]","rate",newValue);
}

// ----------   Surface buttons  ----------

StantonSCS1d.SurfaceButton = function (channel, control, value, status) {
    var currentMode = StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"];
    var byte1 = 0x90 + channel;
    
    var index = currentMode.charAt(currentMode.length-1);
    if (index != "2" && index != "3") index = "1";
    
    if ((status & 0xF0) != 0x80) {    // If button down
        midi.sendShortMsg(byte1,control,0x01); // Turn on button lights
        midi.sendShortMsg(byte1,StantonSCS1d.buttonLEDs[control],0x01);
        switch (currentMode) {
            case "loop":
            case "loop2":
            case "loop3":
                // Multiple pitch points
                
//                 if (StantonSCS1d.modifier[currentMode]==1) {    // Delete a pitch point if the mode button is held
//                     StantonSCS1d.pitchPoints[index][button] = -0.1;
//                     break;
//                 }
                if (StantonSCS1d.pitchPoints[index][control] == -0.1)
                    StantonSCS1d.pitchPoints[index][control] = engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate");
                else {
                    // Need 100% range for values to be correct
                    engine.setValue("[Channel"+StantonSCS1d.deck+"]","rateRange",1.0);
                    engine.setValue("[Channel"+StantonSCS1d.deck+"]","rate",StantonSCS1d.pitchPoints[index][control]); 
                }
                break;
            case "trig":
            case "trig2":
            case "trig3":
                    // Multiple cue points
                    if (StantonSCS1d.modifier[currentMode]==1) {
                        if (StantonSCS1d.deck==1) StantonSCS1d.triggerPoints1[index][control] = -0.1;
                        else StantonSCS1d.triggerPoints2[index][control] = -0.1;
                        break;
                    }
                    if (StantonSCS1d.deck==1) {
                        if (StantonSCS1d.triggerPoints1[index][control] == -0.1)
                            StantonSCS1d.triggerPoints1[index][control] = engine.getValue("[Channel"+StantonSCS1d.deck+"]","playposition");
                        else engine.setValue("[Channel"+StantonSCS1d.deck+"]","playposition",StantonSCS1d.triggerPoints1[index][control]); 
                    break;
                    }   // End deck 1
                    else {
                        if (StantonSCS1d.triggerPoints2[index][control] == -0.1)
                            StantonSCS1d.triggerPoints2[index][control] = engine.getValue("[Channel"+StantonSCS1d.deck+"]","playposition");
                        else engine.setValue("[Channel"+StantonSCS1d.deck+"]","playposition",StantonSCS1d.triggerPoints2[index][control]); 
                    break;
                    }
        }
        return;
    }
    var marker;
    if (StantonSCS1d.markHotCues == "red") marker = StantonSCS1d.buttonLEDs[control];
    else marker = control;
    midi.sendShortMsg(byte1,marker,0x00); // Turn off activated button LED
    // Don't extinguish the marker LED if a cue point isn't set on that button
    if (currentMode.substring(0,4)=="trig") {
        if ((StantonSCS1d.deck==1 && StantonSCS1d.triggerPoints1[index][control] != -0.1) ||
            (StantonSCS1d.deck==2 && StantonSCS1d.triggerPoints2[index][control] != -0.1)) return;
    }
    if (StantonSCS1d.markHotCues == "red") marker = control;
    else marker = StantonSCS1d.buttonLEDs[control];
    midi.sendShortMsg(byte1,marker,0x00);
}

StantonSCS1d.EnterButton = function (channel, control, value, status) {
    StantonSCS1d.buttonLED(value,control,127,0);
    if ((status & 0xF0) == 0x90) {    // If button down
        // If the deck is playing and the cross-fader is not completely toward the other deck...
        if (engine.getValue("[Channel"+StantonSCS1d.deck+"]","play")==1 &&
        ((StantonSCS1d.deck==1 && engine.getValue("[Master]","crossfader")<1.0) || 
        (StantonSCS1d.deck==2 && engine.getValue("[Master]","crossfader")>-1.0))) {
            // ...light the button red to show acknowledgement of the press but don't load
            StantonSCS1d.buttonLED(value,control,64,0);
            print ("StantonSCS1d: Not loading into deck "+StantonSCS1d.deck+" because it's playing to the Master output.");
        }
        else {
            engine.setValue("[Channel"+StantonSCS1d.deck+"]","LoadSelectedTrack",1);
        }
    }
}

// ----------   Pad section  ----------

StantonSCS1d.padRefresh = function () {     // Refresh the LEDs and LCDs in the pad section
    StantonSCS1d.state["padCleared"]=true;
    StantonSCS1d.velocityButton(StantonSCS1d.channel,0x34,0,0x80);
    
    // For each pad
    var deck = StantonSCS1d.padBank["deck"];
    var bank = StantonSCS1d.padBank["bank"+deck];
    for (i=0; i<=3; i++) {
        StantonSCS1d.pad(StantonSCS1d.channel,0x20+i,0,0x80);
        
        // Displays
        var message = "Empty";
        if (StantonSCS1d.padPoints[deck][bank][0x20+i] != -0.1) {
            var trackTime = ((StantonSCS1d.padPoints[deck][bank][0x20+i] * StantonSCS1d.trackDuration[deck])*1000) | 0;  // OR with 0 replaces Math.floor and is faster
            message = msecondstominutes(trackTime);
        }
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 6+i],message.toInt(), 0xF7),7+message.length);   // Set LCD
    }
    
    StantonSCS1d.padBankButton(StantonSCS1d.channel, 0x35, 0, 0x80);
    StantonSCS1d.padBankButton(StantonSCS1d.channel, 0x36, 0, 0x80);
}

StantonSCS1d.pad = function (channel, control, value, status) {
    var deck = StantonSCS1d.padBank["deck"];
    var bank = StantonSCS1d.padBank["bank"+deck];
    var byte1 = 0x90 + channel;

    if ((status & 0xF0) == 0x90) {    // If button down
        midi.sendShortMsg(byte1,control,127); // Light it orange
        StantonSCS1d.modifier["pad"]=1;

        // Multiple cue points
        if (StantonSCS1d.modifier["velocityToggle"]==1) {
            StantonSCS1d.padPoints[deck][bank][control] = -0.1;   // Erase
            var message = "Empty";
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, control-26],message.toInt(), 0xF7),7+message.length);   // Clear LCD
            StantonSCS1d.state["padCleared"]=true;
            return;
        }
        
        if (StantonSCS1d.padPoints[deck][bank][control] == -0.1) { // Set if blank
            StantonSCS1d.padPoints[deck][bank][control] = engine.getValue("[Channel"+deck+"]","playposition");
            // Set display with cue point time
            // Track time in milliseconds
            var trackTime = ((StantonSCS1d.padPoints[deck][bank][control] * StantonSCS1d.trackDuration[deck])*1000) | 0;  // OR with 0 replaces Math.floor and is faster
            var message = msecondstominutes(trackTime);
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, control-26],message.toInt(), 0xF7),7+message.length);   // Set LCD
        }
        else {
            engine.setValue("[Channel"+deck+"]","playposition",StantonSCS1d.padPoints[deck][bank][control]);  // Recall
            if (StantonSCS1d.padVelocity) engine.setValue("[Channel"+deck+"]","volume",(value/0x7A));   // 0x7A seems the highest # the unit sends
            if (engine.getValue("[Channel"+deck+"]","play") != 1) { // Start the deck playing if it isn't already
                engine.setValue("[Channel"+deck+"]","play",1);
                if (StantonSCS1d.state["padWasPlaying"]==null) StantonSCS1d.state["padWasPlaying"]=false;
            }
            else if (StantonSCS1d.state["padWasPlaying"]==null) StantonSCS1d.state["padWasPlaying"]=true;
        }
        return;
    }
    
    // If button up
    if (StantonSCS1d.state["padWasPlaying"]==false) engine.setValue("[Channel"+deck+"]","play",0);
    StantonSCS1d.state["padWasPlaying"]=null;
    StantonSCS1d.modifier["pad"]=0;
    
    // If a cue point is set, make the pad light red
    if (StantonSCS1d.padPoints[deck][bank][control] != -0.1) midi.sendShortMsg(byte1,control,32);
    else midi.sendShortMsg(byte1,control,0x00); // If not, darken the pad
}

StantonSCS1d.velocityButton = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;

    if ((status & 0xF0) == 0x90) {  // If button down
        midi.sendShortMsg(byte1,control,127); // Light it orange
        StantonSCS1d.modifier["velocityToggle"]=1;
        return;
    }
    
    // On button up
    StantonSCS1d.modifier["velocityToggle"]=0;
    if (!StantonSCS1d.state["padCleared"]) StantonSCS1d.padVelocity = !StantonSCS1d.padVelocity;
    StantonSCS1d.state["padCleared"]=false;
    if (StantonSCS1d.padVelocity) midi.sendShortMsg(byte1,control,64);  //red
    else midi.sendShortMsg(0x80 + channel,control,0);   // off
}

StantonSCS1d.padBankButton = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    var deck=control-0x34;
    
    if ((status & 0xF0) == 0x90) {  // If button down
        midi.sendShortMsg(byte1,control,127); // Light it orange
        StantonSCS1d.modifier["padBank"+deck]=1;
    
        if (StantonSCS1d.padBank["deck"]!=deck) {
            StantonSCS1d.padBank["deck"]=deck;
            if (deck==1) midi.sendShortMsg(0x80 + channel,0x36,0);   // turn off deck 2 button
            else midi.sendShortMsg(0x80 + channel,0x35,0);  // turn off deck 1 button
        }
        else {
            if (StantonSCS1d.padBank["bank"+deck]==3) StantonSCS1d.padBank["bank"+deck]=1;
            else {
                StantonSCS1d.padBank["bank"+deck]++;
                print("PadBank="+StantonSCS1d.padBank["bank"+deck]);
                }
        }
        StantonSCS1d.padRefresh();
        return;
    }
    else StantonSCS1d.modifier["padBank"+deck]=0;
    
    // In either case
    if (StantonSCS1d.padBank["deck"]!=deck) midi.sendShortMsg(0x80 + channel,control,0);    // off
    else switch (StantonSCS1d.padBank["bank"+deck]) {
        case 1:
            midi.sendShortMsg(byte1,control,32);    // green
            break;
        case 2:
            midi.sendShortMsg(byte1,control,64);    // red
            break;
        case 3:
            midi.sendShortMsg(byte1,control,127);   // orange
            break;
    }
}

// ----------   Slot functions  ----------

StantonSCS1d.reverse = function (value) {
    // Ignore if fast-forwarding or rewinding
    if (engine.getValue("[Channel"+StantonSCS1d.deck+"]","fwd")>0 || engine.getValue("[Channel"+StantonSCS1d.deck+"]","back")>0) return;
    var CC = 0xB0 + StantonSCS1d.channel;
    if (value>0) {
        if (StantonSCS1d.platterSpeed==1) midi.sendShortMsg(CC,1,'4'.toInt());   // 45 RPM backward
        else midi.sendShortMsg(CC,1,'3'.toInt());   // 33 RPM backward
        return;
    }
    if (StantonSCS1d.platterSpeed==1) midi.sendShortMsg(CC,1,'2'.toInt());   // 45 RPM foreward
    else midi.sendShortMsg(CC,1,'1'.toInt());   // 33 RPM foreward
}

StantonSCS1d.pitchChange = function (value) {
    if (StantonSCS1d.modifier["pitchRange"]==1) return; // Skip if adjusting the cross-fader
    if (value < -1 || value > 1) return;  // FIXME: This sometimes happens after using the BPM button to set the tempo and changing the pitch range. We should find out why.
    var now=new Date();
    // Move slider if applicable
    if (now - StantonSCS1d.state["dontMove"]>100) {
        if (StantonSCS1d.debug) print ("Moving slider to "+(value*63+64));
        midi.sendShortMsg(0xB0+StantonSCS1d.channel,0x00,value*63+64);
        }
    
    // Skip motor speed change if fast-forwarding or rewinding
    if (engine.getValue("[Channel"+StantonSCS1d.deck+"]","fwd")>0 || engine.getValue("[Channel"+StantonSCS1d.deck+"]","back")>0) return;
    
    var multiplier = value * engine.getValue("[Channel"+StantonSCS1d.deck+"]","rateRange");
    var iMotorPitch = 1000+(1000*multiplier);
    if (iMotorPitch < 500) iMotorPitch = 500;
	if (iMotorPitch > 2000) iMotorPitch = 2000;
    //print("Motor pitch: "+iMotorPitch+" multiplier: "+multiplier);
    
    // Convert for AVR protocol
    iMotorPitch -= 500;
	p1 = iMotorPitch  / 1000; 
	p2 = (iMotorPitch / 100) % 10;
	p3 = (iMotorPitch /  10) % 10;
	p4 = (iMotorPitch      ) % 10;
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 35, p1, p2, p3, p4, 0xF7]),11);  // Change motor speed
}

// --------   LEDs  --------

StantonSCS1d.buttonLED = function (value, note, on, off) {
    if (value>0) midi.sendShortMsg(0x90 + StantonSCS1d.channel,note,on);
    else midi.sendShortMsg(0x80 + StantonSCS1d.channel,note,off);
}

// Transport buttons

StantonSCS1d.playLED = function (value) {
    var CC = 0xB0 + StantonSCS1d.channel;
    if (StantonSCS1d.debug) print ("PlatterGrabbed="+StantonSCS1d.state["platterGrabbed"]);
    if (StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] == "vinyl" && !StantonSCS1d.state["platterGrabbed"]) {
        if (value==0) midi.sendShortMsg(CC,1,'x'.toInt());   // Stop platter
        else midi.sendShortMsg(CC,1,'o'.toInt());   // Start platter
    }
    StantonSCS1d.buttonLED(value, 41, 32, 0x00);    //green
}

StantonSCS1d.cueLED = function (value) {
    StantonSCS1d.buttonLED(value, 43, 127, 0x00);  //orange
}

StantonSCS1d.syncLED = function (value) {
    StantonSCS1d.buttonLED(value, 42, 64, 0x00);    //red
}

StantonSCS1d.backLED = function (value) {
    StantonSCS1d.buttonLED(value, 47, 0x7f, 0x00);
}

StantonSCS1d.fwdLED = function (value) {
    StantonSCS1d.buttonLED(value, 46, 0x7f, 0x00);
}

StantonSCS1d.headphoneLED = function (value) {
    StantonSCS1d.buttonLED(value, 55, 0x7f, 32);
}

StantonSCS1d.FXLED = function (value) {
    StantonSCS1d.buttonLED(value, 0x19, 0x7f, 0);
}

// Encoders

// -- 0..1..4 controls --

StantonSCS1d.encoder1EQLEDs = function (value) {
    StantonSCS1d.encoderEQLEDs(value,127);
}

StantonSCS1d.encoder2EQLEDs = function (value) {
    StantonSCS1d.encoderEQLEDs(value,126);
}

StantonSCS1d.encoder3EQLEDs = function (value) {
    StantonSCS1d.encoderEQLEDs(value,125);
}

StantonSCS1d.encoder4EQLEDs = function (value) {
    StantonSCS1d.encoderEQLEDs(value,124);
}

// -- 0..1..5 controls --

StantonSCS1d.encoder1MVolumeLEDs = function (value) {
    StantonSCS1d.encoderMVolumeLEDs(value,127);
}

StantonSCS1d.encoder2MVolumeLEDs = function (value) {
    StantonSCS1d.encoderMVolumeLEDs(value,126);
}

StantonSCS1d.encoder3MVolumeLEDs = function (value) {
    StantonSCS1d.encoderMVolumeLEDs(value,125);
}

StantonSCS1d.encoder4MVolumeLEDs = function (value) {
    StantonSCS1d.encoderMVolumeLEDs(value,124);
}

// -- 0..1 controls --

StantonSCS1d.encoder4VolumeLEDs = function (value) {
    StantonSCS1d.encoderVolumeLEDs(value,124);
}

// -- -1..0..1 controls --

StantonSCS1d.encoder1BalanceLEDs = function (value) {
    StantonSCS1d.encoderBalanceLEDs(value,127);
}

StantonSCS1d.encoder3BalanceLEDs = function (value) {
    StantonSCS1d.encoderBalanceLEDs(value,125);
}

// ---

StantonSCS1d.encoderEQLEDs = function (value,control) {
    var LEDs=StantonSCS1d.EncoderBoostCut(value, 0, 1, 4, 8, 8);
    midi.sendShortMsg(0xB0+StantonSCS1d.channel,control,21+LEDs);
}

StantonSCS1d.encoderBalanceLEDs = function (value,control) {
    var LEDs=StantonSCS1d.EncoderBoostCut(value, -1, 0, 1, 8, 8);
    midi.sendShortMsg(0xB0+StantonSCS1d.channel,control,21+LEDs);
}

StantonSCS1d.EncoderBoostCut = function (value, low, mid, high, lowMidSteps, midHighSteps) {
    var LEDs = 0;
    var lowMidInterval = (mid-low)/(lowMidSteps*2);     // Half the actual interval so the LEDs light in the middle of the interval
    var midHighInterval = (high-mid)/(midHighSteps*2);  // Half the actual interval so the LEDs light in the middle of the interval
    value=value.toFixed(4);
    if (value>low) LEDs++;
    for (var i=1; i<(lowMidSteps*2); i+=2) {
        if (value>low+lowMidInterval*i) LEDs++;
        }
    for (var i=1; i<(midHighSteps*2); i+=2) {
        if (value>mid+midHighInterval*i) LEDs++;
        }
    if (value>=high) LEDs++;
    return LEDs;
}

StantonSCS1d.encoderMVolumeLEDs = function (value,control) {
    //var LEDs=StantonSCS1d.EncoderPeak(value, 0, 5, 18);
    // 0..1..5
    var low=0;
    var mid=1;
    var high=5;
    var max=18;
    
    var top=max/2;
    var add;
    if (value<mid) add=(value*(top+1))/(mid-low);
    else add=((value*top)/(high-mid))+(top-2);
    midi.sendShortMsg(0xB0+StantonSCS1d.channel,control,40+add);
}

StantonSCS1d.encoderVolumeLEDs = function (value,control) {
    var LEDs=StantonSCS1d.EncoderPeak(value, 0, 1, 18);
    midi.sendShortMsg(0xB0+StantonSCS1d.channel,control,40+LEDs);
}

StantonSCS1d.EncoderPeak = function (value, low, high, number) {
    var LEDs = 0;
    var range = (high-low)/number;
    for (var i=0; i<(number-1); i++)
        if (value>low+range*i) LEDs++;  // When i==0, this becomes if (value>low) LEDs++;
    if (value>=high) LEDs++;
    return LEDs;
}

StantonSCS1d.FXDepthLEDs = function (value) {
    var add = StantonSCS1d.EncoderPeak(value,0,1,18);
    midi.sendShortMsg(0xB0+StantonSCS1d.channel,127,40+add);
}

StantonSCS1d.FXDelayLEDs = function (value) {
    var add = StantonSCS1d.EncoderPeak(value,50,10000,18);
    midi.sendShortMsg(0xB0+StantonSCS1d.channel,126,40+add);
}

StantonSCS1d.FXPeriodLEDs = function (value) {
    var add = StantonSCS1d.EncoderPeak(value,50000,2000000,18);
    midi.sendShortMsg(0xB0+StantonSCS1d.channel,125,40+add);
}

StantonSCS1d.MasterVolumeLEDs = function (value) {
    var LEDs = 0;
    var mid = 1.0;
    var lowMidRange = 1/4;
    var midHighRange = 4/4;
    if (value>0.0) LEDs++;
    if (value>lowMidRange) LEDs++;
    if (value>lowMidRange*2) LEDs++;
    if (value>lowMidRange*3) LEDs++;
//     if (value>lowMidRange*4) LEDs++;
    if (value>mid) LEDs++;
    if (value>mid+midHighRange) LEDs++;
    if (value>mid+midHighRange*2) LEDs++;
    if (value>mid+midHighRange*3) LEDs++;
    if (value>=5.0) LEDs++;
//     print("Value="+value+", LEDs="+LEDs);
    var byte1 = 0xB0 + StantonSCS1d.channel;
    midi.sendShortMsg(byte1,0x07,0x28+LEDs);
}

StantonSCS1d.pitchRangeLEDs = function (value) {
    StantonSCS1d.pitchChange(engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate")); // So the platter speed is updated
    var on = 0x90 + StantonSCS1d.channel;
    var off = 0x80 + StantonSCS1d.channel;
    switch (true) {
        case (value<0.08):
                midi.sendShortMsg(off,65,0);   // 8%
                midi.sendShortMsg(off,66,0);   // 16%
                midi.sendShortMsg(off,67,0);   // 25%
                midi.sendShortMsg(off,68,0);   // 50%
            break;
        case (value==0.08):
                midi.sendShortMsg(on,65,1);   // 8%
                midi.sendShortMsg(off,66,0);   // 16%
                midi.sendShortMsg(off,67,0);   // 25%
                midi.sendShortMsg(off,68,0);   // 50%
            break;
        case (value>0.08 && value<0.16):
                midi.sendShortMsg(on,65,1);   // 8%
                midi.sendShortMsg(on,66,1);   // 16%
                midi.sendShortMsg(off,67,0);   // 25%
                midi.sendShortMsg(off,68,0);   // 50%
            break;
        case (value==0.16):
                midi.sendShortMsg(off,65,0);   // 8%
                midi.sendShortMsg(on,66,1);   // 16%
                midi.sendShortMsg(off,67,0);   // 25%
                midi.sendShortMsg(off,68,0);   // 50%
            break;
        case (value>0.16 && value<0.25):
                midi.sendShortMsg(off,65,0);   // 8%
                midi.sendShortMsg(on,66,1);   // 16%
                midi.sendShortMsg(on,67,1);   // 25%
                midi.sendShortMsg(off,68,0);   // 50%
            break;
        case (value==0.25):
                midi.sendShortMsg(off,65,0);   // 8%
                midi.sendShortMsg(off,66,0);   // 16%
                midi.sendShortMsg(on,67,1);   // 25%
                midi.sendShortMsg(off,68,0);   // 50%
            break;
        case (value>0.25 && value<0.5):
                midi.sendShortMsg(off,65,0);   // 8%
                midi.sendShortMsg(off,66,0);   // 16%
                midi.sendShortMsg(on,67,1);   // 25%
                midi.sendShortMsg(on,68,1);   // 50%
            break;
        case (value==0.5):
                midi.sendShortMsg(off,65,0);   // 8%
                midi.sendShortMsg(off,66,0);   // 16%
                midi.sendShortMsg(off,67,0);   // 25%
                midi.sendShortMsg(on,68,1);   // 50%
            break;
        case (value>0.5):
                midi.sendShortMsg(on,65,1);   // 8%
                midi.sendShortMsg(on,66,1);   // 16%
                midi.sendShortMsg(on,67,1);   // 25%
                midi.sendShortMsg(on,68,1);   // 50%
            break;
    }
}

StantonSCS1d.circleBars1 = function (value) {
    if (StantonSCS1d.deck!=1) return;
    StantonSCS1d.circleBars(value);
}

StantonSCS1d.circleBars2 = function (value) {
    if (StantonSCS1d.deck!=2) return;
    StantonSCS1d.circleBars(value);
}

StantonSCS1d.durationChange1 = function (value) {
    StantonSCS1d.trackDuration[1]=value;
}

StantonSCS1d.durationChange2 = function (value) {
    StantonSCS1d.trackDuration[2]=value;
}

StantonSCS1d.circleBars = function (value) {
    StantonSCS1d.scratchDecay();    // take care of scratching
    
    // Revolution time of the imaginary record in seconds
//     var revtime = StantonSCS1d.scratch["revtime"]/2;    // Use this for two lights
    var revtime = StantonSCS1d.scratch["revtime"];
    var currentTrackPos = value * StantonSCS1d.trackDuration[StantonSCS1d.deck];
    
    var revolutions = currentTrackPos/revtime;
//     var light = ((revolutions-(revolutions|0))*18)|0;    // Use this for two lights
    var light = ((revolutions-(revolutions|0))*36)|0;   // OR with 0 replaces Math.floor and is faster

    if (StantonSCS1d.lastLight[StantonSCS1d.deck]==light) return;   // Don't send light commands if there's no visible change
    
    var byte1 = 0xB0 + StantonSCS1d.channel;
    //midi.sendShortMsg(byte1,2,0);     // Clear circle markers
    StantonSCS1d.lastLight[StantonSCS1d.deck]=light;
    midi.sendShortMsg(byte1,2,light);
//     midi.sendShortMsg(byte1,2,18+light);   // Add this for two lights
}

/*
TODO:
- hot cue/loop/pitch section
*/
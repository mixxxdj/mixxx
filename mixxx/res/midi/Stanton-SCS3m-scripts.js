/****************************************************************/
/*      Stanton SCS.3m MIDI controller script v1.05             */
/*          Copyright (C) 2010-11, Sean M. Pappalardo           */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.9.x                                 */
/****************************************************************/

function StantonSCS3m() {}

//      See http://mixxx.org/wiki/doku.php/stanton_scs.3m_mixxx_user_guide  for details

// ----------   Other global variables    ----------
StantonSCS3m.debug = false;  // Enable/disable debugging messages to the console
StantonSCS3m.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
StantonSCS3m.channel = 0;   // MIDI channel the device is on
StantonSCS3m.sysex = [0xF0, 0x00, 0x01, 0x60];    // Preamble for all SysEx messages for this device
StantonSCS3m.modifier = { };    // Modifier buttons (allowing alternate controls) defined on-the-fly if needed
StantonSCS3m.mode_store = { "[Channel1]":"fx", "[Channel2]":"fx", "Master":"alt" };   // Set to opposite default on both

// Signals to (dis)connect by mode: Group, Key, Function name
StantonSCS3m.modeSignals = {"fx":[ ["[Flanger]", "lfoDepth", "StantonSCS3m.FXDepthLEDs"],
                                   ["[Flanger]", "lfoDelay", "StantonSCS3m.FXDelayLEDs"],
                                   ["[Flanger]", "lfoPeriod", "StantonSCS3m.FXPeriodLEDs"] ],
                            "eq":[ ["CurrentChannel", "filterLow", "StantonSCS3m.EQLowLEDs"],
                                   ["CurrentChannel", "filterMid", "StantonSCS3m.EQMidLEDs"],
                                   ["CurrentChannel", "filterHigh", "StantonSCS3m.EQHighLEDs"] ],
                            "none":[]  // To avoid an error on forced mode changes
                            };
StantonSCS3m.masterSignals = {  "main":[ ["CurrentChannel", "rate", "StantonSCS3m.pitchLEDs"],
                                         ["CurrentChannel", "volume", "StantonSCS3m.volumeLEDs"],
                                         ["CurrentChannel", "VuMeter", "StantonSCS3m.Vu"] ],
                                "alt":[  ["[Master]","VuMeterL","StantonSCS3m.VuL"],
                                         ["[Master]","VuMeterR","StantonSCS3m.VuR"],
                                         ["[Master]","headMix","StantonSCS3m.pitchLEDsL"],
                                         ["[Master]","headVolume","StantonSCS3m.masterVolumeLEDsL"],
                                         ["[Master]","balance","StantonSCS3m.pitchLEDsR"],
                                         ["[Master]","volume","StantonSCS3m.masterVolumeLEDsR"] ]
                            };

StantonSCS3m.masterSliders = {  "slider":[ 0x00, 0x01, 0x02, 0x04, 0x06, 0x03, 0x05, 0x07, 0x08, 0x09, 0x0A ],
                                "main":  [ 0x71, 0x71, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70 ],
                                "alt":   [ 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x71, 0x71, 0x70 ]
};  // 0x70 = absolute, 0x71 = relative

// ----------   Functions   ----------

StantonSCS3m.init = function (id) {
    
    StantonSCS3m.id = id;   // Store the ID of this device for later use
    
    // Set the device to flat mode
    midi.sendSysexMsg(StantonSCS3m.sysex.concat([0x15, 0x01, 0xF7]),7);
    
    var CC = 0xB0 + StantonSCS3m.channel;
    var No = 0x90 + StantonSCS3m.channel;
//     midi.sendShortMsg(CC,0x7B,0x00);  // Extinguish all LEDs

    // Connect cross-fader LEDs & light them
    StantonSCS3m.doConnectSignal("[Master]","crossfader","StantonSCS3m.crossfaderLEDs");

    // Connect master signals
    // Hack: Have to register a down press first to get LEDs to light correctly
    StantonSCS3m.Master(StantonSCS3m.channel, 0x0E, 0, 0x90+StantonSCS3m.channel);
    StantonSCS3m.Master(StantonSCS3m.channel, 0x0E, 0, 0x80+StantonSCS3m.channel);

    // Force change to EQ mode on both sides
    // Hack: Have to force FX mode first to get LEDs to light correctly
    StantonSCS3m.FXL(StantonSCS3m.channel, 0x0C, 0, 0x80+StantonSCS3m.channel);
    StantonSCS3m.FXR(StantonSCS3m.channel, 0x0D, 0, 0x80+StantonSCS3m.channel);
    // ----
    StantonSCS3m.EQL(StantonSCS3m.channel, 0x0C, 0, 0x80+StantonSCS3m.channel);
    StantonSCS3m.EQR(StantonSCS3m.channel, 0x0D, 0, 0x80+StantonSCS3m.channel);

    print ("Stanton SCS.3m: \""+StantonSCS3m.id+"\" on MIDI channel "+(StantonSCS3m.channel+1)+" initialized.");
}

StantonSCS3m.shutdown = function () {
    var CC = 0xB0 + StantonSCS3m.channel;
    var No = 0x90 + StantonSCS3m.channel;

    midi.sendShortMsg(CC,0x7B,0x00);  // Extinguish all LEDs
    midi.sendShortMsg(No,0x69,0x01);  // Light the Stanton LED so you know it's still on
    
    print ("Stanton SCS.3m: \""+StantonSCS3m.id+"\" on MIDI channel "+(StantonSCS3m.channel+1)+" shut down.");
}

// (Dis)connects the appropriate Mixxx control signals to/from functions based on what mode the controller section is in
StantonSCS3m.connectModeSignals = function (midiChannel, side, disconnect) {
    var deck = StantonSCS3m.SideToDeck(side);
    var signalList = StantonSCS3m.modeSignals[StantonSCS3m.mode_store["[Channel"+deck+"]"]];
    for (i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        var calledFunction = signalList[i][2]+side;
        if (group=="CurrentChannel") group = "[Channel"+deck+"]";
        StantonSCS3m.doConnectSignal(group,signalList[i][1],calledFunction,disconnect);
    }
    // If disconnecting signals, darken the LEDs on the controls
    if (disconnect) {
        var CC = 0xB0 + midiChannel;
        switch (side) {
            case "L":
                midi.sendShortMsg(CC,0x02,0x00);  // S3 LEDs off
                midi.sendShortMsg(CC,0x04,0x00);  // S4 LEDs off
                midi.sendShortMsg(CC,0x06,0x00);  // S5 LEDs off
                break;
            case "R":
                midi.sendShortMsg(CC,0x03,0x00);  // S6 LEDs off
                midi.sendShortMsg(CC,0x05,0x00);  // S7 LEDs off
                midi.sendShortMsg(CC,0x07,0x00);  // S8 LEDs off
                break;
        }
    }
}

// (Dis)connects the appropriate Mixxx Master control signals to/from functions
StantonSCS3m.connectMasterSignals = function (midiChannel, disconnect) {
    
    var signalList = StantonSCS3m.masterSignals[StantonSCS3m.mode_store["Master"]];
    for (i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        var calledFunction = signalList[i][2];
        if (group=="CurrentChannel") {
            var sides = ["L","R"];
            var side, deck;
            for (j=0; j<2; j++) {
                side=sides[j];
                switch (side) {
                    case "L":
                        deck=1; break;
                    case "R":
                        deck=2; break;
                }
                group = "[Channel"+deck+"]";
                calledFunction = signalList[i][2]+side;
                StantonSCS3m.doConnectSignal(group,signalList[i][1],calledFunction,disconnect);
            }
        }
        else StantonSCS3m.doConnectSignal(group,signalList[i][1],calledFunction,disconnect);
    }
    // If disconnecting signals, darken the LEDs on the controls
    if (disconnect) {
        var CC = 0xB0 + midiChannel;
        midi.sendShortMsg(CC,0x00,0x00);  // S1 LEDs off
        midi.sendShortMsg(CC,0x08,0x00);  // S9 LEDs off
        midi.sendShortMsg(CC,0x0C,0x00);  // V1 LEDs off
        midi.sendShortMsg(CC,0x01,0x00);  // S2 LEDs off
        midi.sendShortMsg(CC,0x09,0x00);  // S10 LEDs off
        midi.sendShortMsg(CC,0x0D,0x00);  // V2 LEDs off
    }
}

StantonSCS3m.doConnectSignal = function (group, key, calledFunction, disconnect) {
    engine.connectControl(group,key,calledFunction,disconnect);

    // If connecting a signal, cause it to fire (by setting it to the same value) to update the LEDs
//     if (!disconnect) engine.trigger(group,key);  // Commented because there's no sense in wasting queue length
    // Alternate method:
    if (!disconnect) {
        var command = calledFunction+"("+engine.getValue(group,key)+")";
//         if (StantonSCS3m.debug) print("Stanton SCS.3m: command="+command);
        eval(command);
    }
    if (StantonSCS3m.debug) {
        if (disconnect) print("Stanton SCS.3m: "+group+","+key+" disconnected from "+calledFunction);
        else print("Stanton SCS.3m: "+group+","+key+" connected to "+calledFunction);
    }
}

StantonSCS3m.SideToDeck = function (side) { // For future n-deck support
    var deck;
    switch (side) {
        case "L":
            deck=1; break;
        case "R":
            deck=2; break;
    }
    return deck;
}

StantonSCS3m.sliderMode = function (control,value) {
    // FIXME: This is a hack, present due to the fact that the SCS.3m
    // wasn't designed to have its slider modes changed quickly on-the-fly
    // It needs about 10ms between slider change commands
    midi.sendShortMsg(0xBF,control,value);
    
    // 10ms pause - TODO: This needs a timer in 1.8
    var date = new Date();
    var curDate = null;
    
    do { curDate = new Date(); }
    while(curDate-date < 10);
}

// ------------------- Surface Controls -----------------------

StantonSCS3m.Deck = function (channel, control, value, status) {
    if (StantonSCS3m.modifier["Master"]) return;    // Ignore if MASTER is held down
    var decks = { 0x10:1, 0x0F:2 };
    var deck = decks[control];
    
    var sides = { 0x10:"L", 0x0F:"R" };
    var side = sides[control];
    
    var volsliders = { 0x10:0x08, 0x0F:0x09 };
    var volslider = volsliders[control];
    
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {   // If button down
        midi.sendShortMsg(byte1,control,3); // Light up C/D too
        StantonSCS3m.modifier["Deck"+side] = true;
        // Use the sliders for other functions, so disconnect previous ones
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]", "volume", "StantonSCS3m.volumeLEDs"+side, true);
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]", "volume", "StantonSCS3m.volumeLEDs"+side, true);
        StantonSCS3m.doConnectSignal("[Master]","crossfader","StantonSCS3m.crossfaderLEDs",true);
        // Connect new ones
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]", "pregain", "StantonSCS3m.gainLEDs"+side);
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]", "playposition", "StantonSCS3m.needleDropLEDs");
        // Change the slider mode - 0x70 = absolute, 0x71 = relative
        StantonSCS3m.sliderMode(volslider,0x71);  // Relative gain control
        StantonSCS3m.sliderMode(0,0x7F);    // End of sequence
        // Use the mode buttons for other functions
        //  so connect applicable signals
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]","beatsync","StantonSCS3m.EQ"+side+"ButtonLED");
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]","flanger","StantonSCS3m.FX"+side+"ButtonLED");
    }
    else {
        // Disconnect special signals
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]","beatsync","StantonSCS3m.EQ"+side+"ButtonLED",true);
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]","flanger","StantonSCS3m.FX"+side+"ButtonLED",true);
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]", "pregain", "StantonSCS3m.gainLEDs"+side, true);
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]", "playposition", "StantonSCS3m.needleDropLEDs", true);
        // Reconnect original ones & trigger
        StantonSCS3m.doConnectSignal("[Channel"+deck+"]", "volume", "StantonSCS3m.volumeLEDs"+side);
        StantonSCS3m.doConnectSignal("[Master]","crossfader","StantonSCS3m.crossfaderLEDs");
        // Change the slider mode - 0x70 = absolute, 0x71 = relative
        StantonSCS3m.sliderMode(volslider,0x70);  // Absolute volume control
        StantonSCS3m.sliderMode(0,0x7F);    // End of sequence
        // Restore the button LEDs
        var fx = { "L":0x0A, "R":0x0B };
        var eq = { "L":0x0C, "R":0x0D };
        var currentMode = StantonSCS3m.mode_store["[Channel"+deck+"]"];
        StantonSCS3m.modifier["Deck"+side] = false;
        switch (currentMode) {
            case "fx": StantonSCS3m.modeButton(channel, fx[side], 0x80+channel, "fx", side); break;
            case "eq": StantonSCS3m.modeButton(channel, eq[side], 0x80+channel, "eq", side); break;
        }
        midi.sendShortMsg(byte1,control,1); // Light just A/B
    }
}

StantonSCS3m.Master = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {   // If button down
        midi.sendShortMsg(byte1,control,2); // Make it red
        StantonSCS3m.modifier["Master"] = true;
        StantonSCS3m.connectMasterSignals(channel,true);    // Disconnect previous signals
        StantonSCS3m.mode_store["Master"]="alt";
        StantonSCS3m.connectMasterSignals(channel); // Connect new ones
    }
    else {
        StantonSCS3m.connectMasterSignals(channel,true);    // Disconnect previous signals
        StantonSCS3m.mode_store["Master"]="main";
        StantonSCS3m.connectMasterSignals(channel); // Connect new ones
        StantonSCS3m.modifier["Master"] = false;
        midi.sendShortMsg(byte1,control,1); // Make it blue
    }
    // Set the sliders to abs/rel mode as needed
    var sliderList = StantonSCS3m.masterSliders["slider"];
    var sliderValues = StantonSCS3m.masterSliders[StantonSCS3m.mode_store["Master"]];
    for (i=0; i<sliderList.length; i++) {
//         print("Stanton SCS.3m: slider number="+sliderList[i]+", value="+sliderValues[i]);
        StantonSCS3m.sliderMode(sliderList[i],sliderValues[i]);
    }
    StantonSCS3m.sliderMode(0,0x7F);    // End of sequence
}

StantonSCS3m.PitchL = function (channel, control, value, status) {
    var deck = StantonSCS3m.SideToDeck("L");
    var currentMode = StantonSCS3m.mode_store["[Channel"+deck+"]"];
    // If the current mode button is held down, reset the control to center
    if (StantonSCS3m.modifier[currentMode+"L"]) {
        switch (StantonSCS3m.mode_store["Master"]) {
            case "main": engine.setValue("[Channel"+deck+"]","rate",0); break;
            case "alt": engine.setValue("[Master]","headMix",0); break;
        }
    }
    else switch (StantonSCS3m.mode_store["Master"]) {
        case "main": StantonSCS3m.PitchRel(1,value,"L"); break;
        case "alt": engine.setValue("[Master]","headMix",(value-64)/64); break;
    }
}

StantonSCS3m.PitchR = function (channel, control, value, status) {
    var deck = StantonSCS3m.SideToDeck("R");
    var currentMode = StantonSCS3m.mode_store["[Channel"+deck+"]"];
    // If the current mode button is held down, reset the control to center
    if (StantonSCS3m.modifier[currentMode+"R"]) {
        switch (StantonSCS3m.mode_store["Master"]) {
            case "main": engine.setValue("[Channel"+deck+"]","rate",0); break;
            case "alt": engine.setValue("[Master]","balance",0); break;
        }
    }
    else switch (StantonSCS3m.mode_store["Master"]) {
        case "main": StantonSCS3m.PitchRel(2,value,"R"); break;
        case "alt": engine.setValue("[Master]","balance",(value-64)/64); break;
    }
}

StantonSCS3m.PitchRel = function (deck, value, side) {
    var currentValue = engine.getValue("[Channel"+deck+"]","rate");
    var newValue;
    if (StantonSCS3m.modifier["Deck"+side])
        newValue = currentValue+(value-64)/1024;    // Fine pitch adjust
    else newValue = currentValue+(value-64)/256;
    if (newValue<-1) newValue=-1.0;
    if (newValue>1) newValue=1.0;
    engine.setValue("[Channel"+deck+"]","rate",newValue);
}

StantonSCS3m.PitchLbL = function (channel, control, value, status) {
    StantonSCS3m.PitchBendButton(status,"L",-1);
}

StantonSCS3m.PitchLbR = function (channel, control, value, status) {
    StantonSCS3m.PitchBendButton(status,"L",1);
}

StantonSCS3m.PitchRbL = function (channel, control, value, status) {
    StantonSCS3m.PitchBendButton(status,"R",-1);
}

StantonSCS3m.PitchRbR = function (channel, control, value, status) {
    StantonSCS3m.PitchBendButton(status,"R",1);
}

StantonSCS3m.PitchBendButton = function (status, side, comp) {
    // Skip if Master button is held or fine-tuning the pitch
    if (StantonSCS3m.modifier["Master"] || StantonSCS3m.modifier["Deck"+side]) return;
    var deck = StantonSCS3m.SideToDeck(side);
    if (engine.getValue("[Channel"+deck+"]","rate_dir") == comp) {   // Go in the appropriate direction
        if ((status & 0xF0) == 0x90)    // If button down
            engine.setValue("[Channel"+deck+"]","rate_temp_up",1);
        else 
            engine.setValue("[Channel"+deck+"]","rate_temp_up",0);
    }
    else {
        if ((status & 0xF0) == 0x90)    // If button down
            engine.setValue("[Channel"+deck+"]","rate_temp_down",1);
        else 
            engine.setValue("[Channel"+deck+"]","rate_temp_down",0);
    }
}


// Multi-function sliders

//      Function select buttons
StantonSCS3m.FXL = function (channel, control, value, status) {
    StantonSCS3m.modeButton(channel, control, status, "fx", "L");
}

StantonSCS3m.FXR = function (channel, control, value, status) {
    StantonSCS3m.modeButton(channel, control, status, "fx", "R");
}

StantonSCS3m.EQL = function (channel, control, value, status) {
    StantonSCS3m.modeButton(channel, control, status, "eq", "L");
}

StantonSCS3m.EQR = function (channel, control, value, status) {
    StantonSCS3m.modeButton(channel, control, status, "eq", "R");
}

//      Actual sliders
StantonSCS3m.TripleL1 = function (channel, control, value, status) {
    StantonSCS3m.Triple1(channel, control, value, status, "L");
}

StantonSCS3m.TripleL2 = function (channel, control, value, status) {
    StantonSCS3m.Triple2(channel, control, value, status, "L");
}

StantonSCS3m.TripleL3 = function (channel, control, value, status) {
    StantonSCS3m.Triple3(channel, control, value, status, "L");
}

StantonSCS3m.TripleR1 = function (channel, control, value, status) {
    StantonSCS3m.Triple1(channel, control, value, status, "R");
}

StantonSCS3m.TripleR2 = function (channel, control, value, status) {
    StantonSCS3m.Triple2(channel, control, value, status, "R");
}

StantonSCS3m.TripleR3 = function (channel, control, value, status) {
    StantonSCS3m.Triple3(channel, control, value, status, "R");
}

//      Work functions
StantonSCS3m.Triple1 = function (channel, control, value, status, side) {
    var deck = StantonSCS3m.SideToDeck(side);
    var currentMode = StantonSCS3m.mode_store["[Channel"+deck+"]"];
    // If mode button is held down, reset the control to center
    if (StantonSCS3m.modifier[currentMode+side]) {
        switch (currentMode) {
            case "fx": engine.setValue("[Flanger]","lfoDepth",0.5); break;
            case "eq": engine.setValue("[Channel"+deck+"]","filterLow",1); break;
        }
    }
    else switch (currentMode) {
        case "fx": script.absoluteSlider("[Flanger]","lfoDepth",value,0,1); break;
        case "eq": engine.setValue("[Channel"+deck+"]","filterLow",script.absoluteNonLin(value,0,1,4)); break;
    }
}

StantonSCS3m.Triple2 = function (channel, control, value, status, side) {
    var deck = StantonSCS3m.SideToDeck(side);
    var currentMode = StantonSCS3m.mode_store["[Channel"+deck+"]"];
    // If mode button is held down, reset the control to center
    if (StantonSCS3m.modifier[currentMode+side]) {
        switch (currentMode) {
            case "fx": engine.setValue("[Flanger]","lfoDelay",4950); break; // reset the control to center
            case "eq": engine.setValue("[Channel"+deck+"]","filterMid",1); break;  // reset the control to center
        }
    }
    else switch (currentMode) {
        case "fx": script.absoluteSlider("[Flanger]","lfoDelay",value,50,10000); break;
        case "eq": engine.setValue("[Channel"+deck+"]","filterMid",script.absoluteNonLin(value,0,1,4)); break;
    }
}

StantonSCS3m.Triple3 = function (channel, control, value, status, side) {
    var deck = StantonSCS3m.SideToDeck(side);
    var currentMode = StantonSCS3m.mode_store["[Channel"+deck+"]"];
    // If mode button is held down, reset the control to center
    if (StantonSCS3m.modifier[currentMode+side]) {
        switch (currentMode) {
            case "fx": engine.setValue("[Flanger]","lfoPeriod",1025000); break;
            case "eq": engine.setValue("[Channel"+deck+"]","filterHigh",1); break;
        }
    }
    else switch (currentMode) {
        case "fx": script.absoluteSlider("[Flanger]","lfoPeriod",value,50000,2000000); break;
        case "eq": engine.setValue("[Channel"+deck+"]","filterHigh",script.absoluteNonLin(value,0,1,4)); break;
    }
}


StantonSCS3m.modeButton = function (channel, control, status, modeName, side) {
    var deck = StantonSCS3m.SideToDeck(side);
    var currentMode = StantonSCS3m.mode_store["[Channel"+deck+"]"];
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {    // If button down
        // If the deck change button for the side in question is held down,
        //  do special functions
        if (StantonSCS3m.modifier["Deck"+side]) {
            switch (modeName) {
                case "fx": engine.setValue("[Channel"+deck+"]","flanger",!engine.getValue("[Channel"+deck+"]","flanger")); break;
                case "eq": engine.setValue("[Channel"+deck+"]","beatsync",1); break;
            }
            return;
        }
        midi.sendShortMsg(byte1,control,0x03); // Make button purple
        StantonSCS3m.modifier[modeName+side]=true;   // Set mode modifier flag
        if (currentMode == modeName) {
            StantonSCS3m.modifier["time"+modeName+side] = new Date();  // Store the current time in milliseconds
        }
        else StantonSCS3m.modifier["time"+modeName+side] = 0.0;
        return;
    }
    
    // If button up
    
    // If the deck change button for the side in question is held down,
    //  do special functions
    if (StantonSCS3m.modifier["Deck"+side]) {
        switch (modeName) {
            case "eq": engine.setValue("[Channel"+deck+"]","beatsync",0); break;
        }
        return;
    }
    
    StantonSCS3m.modifier[currentMode+side] = StantonSCS3m.modifier[modeName+side] = 0;   // Clear mode modifier flags
    switch (side) { // Make both mode buttons blue on that side
        case "L":
            midi.sendShortMsg(byte1,0x0A,0x01);
            midi.sendShortMsg(byte1,0x0C,0x01);
            break;
        case "R":
            midi.sendShortMsg(byte1,0x0B,0x01);
            midi.sendShortMsg(byte1,0x0D,0x01);
            break;
    }
    // If trying to switch to the same mode, or the same physical button was held down for over 1/3 of a second, stay in the current mode
    if (currentMode == modeName || (StantonSCS3m.modifier["time"+modeName+side] != 0.0 && 
        ((new Date() - StantonSCS3m.modifier["time"+modeName+side])>300))) {
        switch (currentMode.charAt(currentMode.length-1)) {   // Return the button to its original color
            case "2": midi.sendShortMsg(byte1,control,0x03); break; // Make button purple
            case "3": midi.sendShortMsg(byte1,control,0x00); break; // Make button black
            default:  midi.sendShortMsg(byte1,control,0x02); break; // Make button red
        }
        // Re-trigger signals
        var signalList = StantonSCS3m.modeSignals[StantonSCS3m.mode_store["[Channel"+deck+"]"]];
        for (i=0; i<signalList.length; i++) {
            var group = signalList[i][0];
            var calledFunction = signalList[i][2]+side;
            if (group=="CurrentChannel") group = "[Channel"+deck+"]";
            engine.trigger(group,signalList[i][1]);
        }
        return;
    }
    
    if (StantonSCS3m.debug) print("Stanton SCS.3m: Switching to "+modeName.toUpperCase()+" mode on the "+side+" side");
    switch (modeName.charAt(modeName.length-1)) {   // Set the button to its new color
        case "2": midi.sendShortMsg(byte1,control,0x03); break;   // Make button purple
        case "3": midi.sendShortMsg(byte1,control,0x00); break;   // Make button black
        default:  midi.sendShortMsg(byte1,control,0x02); break;  // Make button red
    }
    StantonSCS3m.connectModeSignals(channel,side,true);  // Disconnect previous ones
    StantonSCS3m.mode_store["[Channel"+deck+"]"] = modeName;
    StantonSCS3m.connectModeSignals(channel,side);  // Connect new ones
}

StantonSCS3m.VolumeL = function (channel, control, value, status) {
    var deck = StantonSCS3m.SideToDeck("L");
    switch (StantonSCS3m.mode_store["Master"]) {
        case "main": 
            if (StantonSCS3m.modifier["DeckL"]) {    // Adjust gain
                var newValue = engine.getValue("[Channel"+deck+"]","pregain")+(value-64)/128;
                if (newValue<0.0) newValue=0.0;
                if (newValue>4.0) newValue=4.0;
                engine.setValue("[Channel"+deck+"]","pregain",newValue);
            }
            else engine.setValue("[Channel"+deck+"]","volume",value/127);
            break;
        case "alt":
            var newValue = engine.getValue("[Master]","headVolume")+(value-64)/128;
            if (newValue<0.0) newValue=0.0;
            if (newValue>5.0) newValue=5.0;
            engine.setValue("[Master]","headVolume",newValue);
            break;
    }
}

StantonSCS3m.VolumeR = function (channel, control, value, status) {
    var deck = StantonSCS3m.SideToDeck("R");
    switch (StantonSCS3m.mode_store["Master"]) {
        case "main": 
            if (StantonSCS3m.modifier["DeckR"]) {    // Adjust gain
                var newValue = engine.getValue("[Channel"+deck+"]","pregain")+(value-64)/128;
                if (newValue<0.0) newValue=0.0;
                if (newValue>4.0) newValue=4.0;
                engine.setValue("[Channel"+deck+"]","pregain",newValue);
            }
            else engine.setValue("[Channel"+deck+"]","volume",value/127);
            break;
        case "alt": 
            var newValue = engine.getValue("[Master]","volume")+(value-64)/128;
            if (newValue<0.0) newValue=0.0;
            if (newValue>5.0) newValue=5.0;
            engine.setValue("[Master]","volume",newValue);
            break;
    }
}

StantonSCS3m.crossfader = function (channel, control, value) {
    var deck;
    // Needle dropping a la Numark's "Strip Search"
    if (StantonSCS3m.modifier["DeckL"]) deck = StantonSCS3m.SideToDeck("L");
    if (StantonSCS3m.modifier["DeckR"]) deck = StantonSCS3m.SideToDeck("R");
    if (deck) {
        engine.setValue("[Channel"+deck+"]","playposition",value/127);
        return;
    }
    // Otherwise regular cross-fader
    engine.setValue("[Master]","crossfader",(value-64)/63);
}

StantonSCS3m.cueL = function (channel, control, value, status) {
    StantonSCS3m.cue(status,"L");
}

StantonSCS3m.cueR = function (channel, control, value, status) {
    StantonSCS3m.cue(status,"R");
}

StantonSCS3m.cue = function (status, side) {
    var deck = StantonSCS3m.SideToDeck(side);
    if ((status & 0xF0) == 0x90) {  // If button down
        StantonSCS3m.modifier["cue"+side] = true;   // Set button modifier flag
        engine.setValue("[Channel"+deck+"]","cue_default",1);
    }
    else {
        StantonSCS3m.modifier["cue"+side] = false;
        engine.setValue("[Channel"+deck+"]","cue_default",0);
    }
}


StantonSCS3m.playL = function (channel, control, value, status) {
    StantonSCS3m.play(status,"L");
}

StantonSCS3m.playR = function (channel, control, value, status) {
    StantonSCS3m.play(status,"R");
}

StantonSCS3m.play = function (status, side) {
    var deck = StantonSCS3m.SideToDeck(side);
    if ((status & 0xF0) == 0x90) {  // If button down
        StantonSCS3m.modifier["play"+side]=true;
        var currentlyPlaying = engine.getValue("[Channel"+deck+"]","play");
        engine.setValue("[Channel"+deck+"]","play", !currentlyPlaying);
        return;
    }
    StantonSCS3m.modifier["play"+side]=false;
}

// ------------------- Slot Functions -----------------------

StantonSCS3m.crossfaderLEDs = function (value) {
    var add = StantonSCS3m.BoostCut(11, value, -1, 0, 1, 5, 5);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    midi.sendShortMsg(byte1,0x0A,0x15+add);
}

StantonSCS3m.needleDropLEDs = function (value) {
    var add = StantonSCS3m.Peak(10,value,0,1);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    midi.sendShortMsg(byte1,0x0A,1+add);
}

StantonSCS3m.buttonLED = function (value, note, on, off) {
    var byte1 = 0x90 + StantonSCS3m.channel;
    if (value>0) midi.sendShortMsg(byte1,note,on);
    else midi.sendShortMsg(byte1,note,off);
}

StantonSCS3m.EQLButtonLED = function(value) {
    StantonSCS3m.buttonLED(value, 0x0C, 0x02, 0x00);
}

StantonSCS3m.EQRButtonLED = function(value) {
    StantonSCS3m.buttonLED(value, 0x0D, 0x02, 0x00);
}

StantonSCS3m.FXLButtonLED = function(value) {
    StantonSCS3m.buttonLED(value, 0x0A, 0x02, 0x00);
}

StantonSCS3m.FXRButtonLED = function(value) {
    StantonSCS3m.buttonLED(value, 0x0B, 0x02, 0x00);
}

StantonSCS3m.EQLowLEDsL = function (value) {
    StantonSCS3m.EQLowLEDs(value,"L");
}

StantonSCS3m.EQLowLEDsR = function (value) {
    StantonSCS3m.EQLowLEDs(value,"R");
}

StantonSCS3m.EQLowLEDs = function (value, side) {
    var add = StantonSCS3m.BoostCut(7,value, 0, 1, 4, 3, 3);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    switch (side) {
        case "L": midi.sendShortMsg(byte1,0x02,0x15+add); break;
        case "R": midi.sendShortMsg(byte1,0x03,0x15+add); break;
    }
}

StantonSCS3m.EQMidLEDsL = function (value) {
    StantonSCS3m.EQMidLEDs(value,"L");
}

StantonSCS3m.EQMidLEDsR = function (value) {
    StantonSCS3m.EQMidLEDs(value,"R");
}

StantonSCS3m.EQMidLEDs = function (value, side) {
    var add = StantonSCS3m.BoostCut(7,value, 0, 1, 4, 3, 3);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    switch (side) {
        case "L": midi.sendShortMsg(byte1,0x04,0x15+add); break;
        case "R": midi.sendShortMsg(byte1,0x05,0x15+add); break;
    }
}

StantonSCS3m.EQHighLEDsL = function (value) {
    StantonSCS3m.EQHighLEDs(value,"L");
}

StantonSCS3m.EQHighLEDsR = function (value) {
    StantonSCS3m.EQHighLEDs(value,"R");
}

StantonSCS3m.EQHighLEDs = function (value, side) {
    var add = StantonSCS3m.BoostCut(7,value, 0, 1, 4, 3, 3);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    switch (side) {
        case "L": midi.sendShortMsg(byte1,0x06,0x15+add); break;
        case "R": midi.sendShortMsg(byte1,0x07,0x15+add); break;
    }
}

StantonSCS3m.FXDepthLEDsL = function (value) {
    StantonSCS3m.FXDepthLEDs(value,"L");
}

StantonSCS3m.FXDepthLEDsR = function (value) {
    StantonSCS3m.FXDepthLEDs(value,"R");
}

StantonSCS3m.FXDepthLEDs = function (value, side) {
    var add = StantonSCS3m.Peak(7,value,0,1);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    switch (side) {
        case "L": midi.sendShortMsg(byte1,0x02,0x28+add); break;
        case "R": midi.sendShortMsg(byte1,0x03,0x28+add); break;
    }
}

StantonSCS3m.FXDelayLEDsL = function (value) {
    StantonSCS3m.FXDelayLEDs(value,"L");
}

StantonSCS3m.FXDelayLEDsR = function (value) {
    StantonSCS3m.FXDelayLEDs(value,"R");
}

StantonSCS3m.FXDelayLEDs = function (value, side) {
    var add = StantonSCS3m.Peak(7,value,50,10000);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    switch (side) {
        case "L": midi.sendShortMsg(byte1,0x04,0x28+add); break;
        case "R": midi.sendShortMsg(byte1,0x05,0x28+add); break;
    }
}

StantonSCS3m.FXPeriodLEDsL = function (value) {
    StantonSCS3m.FXPeriodLEDs(value,"L");
}

StantonSCS3m.FXPeriodLEDsR = function (value) {
    StantonSCS3m.FXPeriodLEDs(value,"R");
}

StantonSCS3m.FXPeriodLEDs = function (value, side) {
    var add = StantonSCS3m.Peak(7,value,50000,2000000);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    switch (side) {
        case "L": midi.sendShortMsg(byte1,0x06,0x28+add); break;
        case "R": midi.sendShortMsg(byte1,0x07,0x28+add); break;
    }
}

StantonSCS3m.pitchLEDsL = function (value) {
    StantonSCS3m.pitchLEDs(value,"L");
}

StantonSCS3m.pitchLEDsR = function (value) {
    StantonSCS3m.pitchLEDs(value,"R");
}

StantonSCS3m.pitchLEDs = function (value, side) {
    var add = StantonSCS3m.BoostCut(7, value, -1, 0, 1, 3, 3);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    switch (side) {
        case "L": midi.sendShortMsg(byte1,0x00,0x15+add); break;
        case "R": midi.sendShortMsg(byte1,0x01,0x15+add); break;
    }
}

StantonSCS3m.gainLEDsL = function (value) {
    StantonSCS3m.gainLEDs(value,0x08);
}

StantonSCS3m.gainLEDsR = function (value) {
    StantonSCS3m.gainLEDs(value,0x09);
}

StantonSCS3m.gainLEDs = function (value, CC) {
    var add = StantonSCS3m.BoostCut(7, value, 0.0, 1.0, 4.0, 3, 3);
    midi.sendShortMsg(0xB0 + StantonSCS3m.channel,CC,0x15+add);
}

StantonSCS3m.volumeLEDsL = function (value) {
    StantonSCS3m.volumeLEDs(value,0x08);
}

StantonSCS3m.volumeLEDsR = function (value) {
    StantonSCS3m.volumeLEDs(value,0x09);
}

StantonSCS3m.VuL = function (value) {
    StantonSCS3m.volumeLEDs(value,0x0C);
}

StantonSCS3m.VuR = function (value) {
    StantonSCS3m.volumeLEDs(value,0x0D);
}

StantonSCS3m.volumeLEDs = function (value, CC) {
    var add = StantonSCS3m.Peak(7, value, 0, 1);
    midi.sendShortMsg(0xB0 + StantonSCS3m.channel,CC,0x28+add);
}

StantonSCS3m.masterVolumeLEDsL = function (value) {
    StantonSCS3m.masterVolumeLEDs(value,0x08);
}

StantonSCS3m.masterVolumeLEDsR = function (value) {
    StantonSCS3m.masterVolumeLEDs(value,0x09);
}

StantonSCS3m.masterVolumeLEDs = function (value, CC) {
    var add = StantonSCS3m.BoostCut(7, value, 0.0, 1.0, 5.0, 3, 3);
    var byte1 = 0xB0 + StantonSCS3m.channel;
    midi.sendShortMsg(byte1,CC,0x15+add);
}

StantonSCS3m.BoostCut = function (numberLights, value, low, mid, high, lowMidSteps, midHighSteps) {
    var LEDs = 0;
    var lowMidInterval = (mid-low)/(lowMidSteps*2);     // Half the actual interval so the LEDs light in the middle of the interval
    var midHighInterval = (high-mid)/(midHighSteps*2);  // Half the actual interval so the LEDs light in the middle of the interval
    value=value.toFixed(4);
    if (value>low) LEDs++;
    if (value>low+lowMidInterval) LEDs++;
    if (value>low+lowMidInterval*3) LEDs++;
    if (numberLights>=9 && value>low+lowMidInterval*5) LEDs++;
    if (numberLights>=11 && value>low+lowMidInterval*7) LEDs++;
    if (value>mid+midHighInterval) LEDs++;
    if (value>mid+midHighInterval*3) LEDs++;
    if (numberLights>=9 && value>mid+midHighInterval*5) LEDs++;
    if (numberLights>=11 && value>mid+midHighInterval*7) LEDs++;
    if (value>=high) LEDs++;
    return LEDs;
}

StantonSCS3m.Peak = function (numberLights, value, low, high) {
    var LEDs = 0;
    var halfInterval = (high-low)/((numberLights-1)*2);    // Half the actual interval so the LEDs light in the middle of the interval
    value=value.toFixed(4);
    if (value>low) LEDs++;
    if (value>low+halfInterval) LEDs++;
    if (value>low+halfInterval*3) LEDs++;
    if (value>low+halfInterval*5) LEDs++;
    if (value>low+halfInterval*7) LEDs++;
    if (value>low+halfInterval*9) LEDs++;
    if (numberLights>=8 && value>low+halfInterval*11) LEDs++;
    if (numberLights>=9 && value>low+halfInterval*13) LEDs++;
    if (numberLights>=10 && value>low+halfInterval*15) LEDs++;
    if (numberLights>=11 && value>low+halfInterval*17) LEDs++;
    if (value>=high) LEDs++;
    return LEDs;
}

/*
Possibly add:
- (Deck + Play = Reverse?)
- Reset Master & head vols?

TODO:
- Rework slider mode changes to use presets or timers
- Don't use the hardware buttons for pitch bending. Switch the sliders to absolute mode & use the formula from the SCS.3d script.
*/

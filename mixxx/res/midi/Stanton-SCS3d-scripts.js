/****************************************************************/
/*      Stanton SCS.3d MIDI controller script                   */
/*          Copyright (C) 2009, Sean M. Pappalardo              */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.6.2                                 */
/****************************************************************/

function StantonSCS3d() {}

// ----------   Customization variables ----------
StantonSCS3d.pitchRanges = [ 0.08, 0.12, 0.5, 1.0 ];    // Pitch ranges for LED off, blue, purple, red
StantonSCS3d.fastDeckChange = false;    // Skip the flashy lights if true

// ----------   Other global variables    ----------

StantonSCS3d.buttons = { "fx":0x20, "eq":0x26, "loop":0x22, "trig":0x28, "vinyl":0x24, "vinyl2":0x24, "deck":0x2A };
StantonSCS3d.buttonLEDs = { 0x48:0x62, 0x4A:0x61, 0x4C:0x60, 0x4e:0x5f, 0x4f:0x67, 0x51:0x68, 0x53:0x69, 0x55:0x6a }; // Maps surface buttons to corresponding circle LEDs
StantonSCS3d.mode_store = { "[Channel1]":"vinyl", "[Channel2]":"vinyl" };   // Set vinyl mode on both decks
StantonSCS3d.deck = 1;  // Currently active virtual deck
StantonSCS3d.modifier = { };  // Modifier buttons (allowing alternate controls) defined on-the-fly if needed
StantonSCS3d.state = { "pitchAbs":0 };   // Temporary state variables
StantonSCS3d.modeSurface = { "fx":"S3+S5", "eq":"S3+S5", "loop":"Buttons", "trig":"Buttons", "vinyl":"C1", "vinyl2":"C1"};
StantonSCS3d.surface = { "C1":0x00, "S5":0x01, "S3":0x02, "S3+S5":0x03, "Buttons":0x04 };
StantonSCS3d.sysex = [0xF0, 0x00, 0x01, 0x60];  // Preamble for all SysEx messages for this device
// Variables used in the scratching alpha-beta filter: (revtime = 1.8 to start)
StantonSCS3d.scratch = { "revtime":1.8, "alpha":0.1, "beta":1.0 };
// StantonSCS3d.scratch = { "time":0.0, "track":0.0, "trackInitial":0.0, "slider":0, "scratch":0.0, "revtime":3.6, "alpha":0.1, "beta":1.0 };
// Multiple cue points:
StantonSCS3d.triggerPoints1 = { 0x48:-0.1, 0x4A:-0.1, 0x4C:-0.1, 0x4E:-0.1, 0x4F:-0.1, 0x51:-0.1, 0x53:-0.1, 0x55:-0.1 };
StantonSCS3d.triggerPoints2 = { 0x48:-0.1, 0x4A:-0.1, 0x4C:-0.1, 0x4E:-0.1, 0x4F:-0.1, 0x51:-0.1, 0x53:-0.1, 0x55:-0.1 }; 

// Signals to (dis)connect by mode: Group, Key, Function name
StantonSCS3d.modeSignals = {"fx":[ ["[Flanger]", "lfoDepth", "StantonSCS3d.FXDepthLEDs"],
                                   ["[Flanger]", "lfoDelay", "StantonSCS3d.FXDelayLEDs"],
                                   ["[Flanger]", "lfoPeriod", "StantonSCS3d.FXPeriodLEDs"],
                                   ["CurrentChannel", "reverse", "StantonSCS3d.B11LED"],
                                   ["CurrentChannel", "flanger", "StantonSCS3d.B12LED"] ],
                            "eq":[ ["CurrentChannel", "filterLow", "StantonSCS3d.EQLowLEDs"],
                                   ["CurrentChannel", "filterMid", "StantonSCS3d.EQMidLEDs"],
                                   ["CurrentChannel", "filterHigh", "StantonSCS3d.EQHighLEDs"] ],
                            "loop":[],
                            "trig":[],
                            "vinyl":[ ["CurrentChannel", "pfl", "StantonSCS3d.B11LED"] ],
                            "vinyl2":[],
                            "none":[]  // To avoid an error on forced mode changes
                            };
StantonSCS3d.deckSignals = [    ["CurrentChannel", "rate", "StantonSCS3d.pitchLEDs"],
                                ["CurrentChannel", "rateRange", "StantonSCS3d.pitchSliderLED"],
                                ["CurrentChannel", "volume", "StantonSCS3d.gainLEDs"],
                                ["CurrentChannel", "play", "StantonSCS3d.playLED"],
                                ["CurrentChannel", "cue_default", "StantonSCS3d.cueLED"],
                                ["CurrentChannel", "beatsync", "StantonSCS3d.syncLED"],
//                                 ["CurrentChannel", "bpm", "StantonSCS3d.tapLED"],
                                ["CurrentChannel", "back", "StantonSCS3d.B13LED"],
                                ["CurrentChannel", "fwd", "StantonSCS3d.B14LED"]
                            ];

StantonSCS3d.temp = { "channel":1, "device":"SCS.3d MIDI 1" };

// ----------   Functions   ----------

/*
StantonSCS3d.signalsInit = function () {    // We don't need this anymore! :)
    // Signal connections work fine at the beginning
    // Seems like they all need to be initialized by being connected to something here first in order to use them later
    var ChannelKeys = ["play", "cue_default", "beatsync", "bpm", "volume", "rate", "rateRange", "pfl", "playposition", "back", "fwd", "reverse", "filterLow", "filterMid", "filterHigh", "flanger"];
    for (i=0; i<ChannelKeys.length; i++) {
        for (j=1; j<=2; j++) {  // Number of decks
            engine.connectControl("[Channel"+j+"]",ChannelKeys[i],"nop");    // Connect the signal ("nop" is defined in the default script file)
            engine.connectControl("[Channel"+j+"]",ChannelKeys[i],"nop",true);   // Disconnect it
        }
    }
    
    var Groups = [ "[Flanger]" ];
    var Keys = [ ["lfoDepth", "lfoDelay", "lfoPeriod"] ];
    for (i=0; i<Groups.length; i++) {
        for (j=0; j<Keys[i].length; j++) {
            engine.connectControl(Groups[i],Keys[i][j],"nop");    // Connect the signal ("nop" is defined in the default script file)
            engine.connectControl(Groups[i],Keys[i][j],"nop",true);   // Disconnect it
        }
    }
}
*/

StantonSCS3d.init = function () {   // called when the MIDI device is opened & set up
//     StantonSCS3d.signalsInit();
    
    var CC = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    var No = 0x90 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(CC,0x7B,0x00,StantonSCS3d.temp["device"]);  // Extinguish all LEDs
    midi.sendShortMsg(No,0x3E,0x01,StantonSCS3d.temp["device"]);  // Pitch LED blue
    for (i=0x48; i<=0x5c; i++) midi.sendShortMsg(No,i,0x40,StantonSCS3d.temp["device"]); // Set surface LEDs to black default
    
    // Force change to first deck, initializing the control surface & LEDs and connecting signals in the process
    StantonSCS3d.deck = 2;  // Set active deck to right (#2) so the below will switch to #1.
    StantonSCS3d.DeckChange(StantonSCS3d.temp["channel"], StantonSCS3d.temp["device"], StantonSCS3d.buttons["deck"], "", 0x80+StantonSCS3d.temp["channel"]-1);
    
    print ("StantonSCS3d: \""+StantonSCS3d.temp["device"]+"\" initialized.");
}

StantonSCS3d.shutdown = function () {   // called when the MIDI device is closed
    var byte1 = 0xB0 + (midi.channel-1);
    midi.sendShortMsg(byte1,0x7B,0x00,StantonSCS3d.temp["device"]);  // Extinguish all LEDs
}

// (Dis)connects the appropriate Mixxx control signals to/from functions based on the currently controlled deck and what mode the controller is in
StantonSCS3d.connectSurfaceSignals = function (disconnect) {

    var signalList = StantonSCS3d.modeSignals[StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]];
    for (i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        if (group=="CurrentChannel") group = "[Channel"+StantonSCS3d.deck+"]";
        engine.connectControl(group,signalList[i][1],signalList[i][2],disconnect);
        
        // If connecting a signal, cause it to fire (by setting it to the same value) to update the LEDs
//         if (!disconnect) engine.setValue(group,signalList[i][1],engine.getValue(group,signalList[i][1]));
        if (!disconnect) {
// //             engine.trigger(group,signalList[i][1]); // This is what should be used but it can segfault due to ControlObject::queueFromThread()
//             // Workaround:
            var command = signalList[i][2]+"("+engine.getValue(group,signalList[i][1])+")";
//             print("StantonSCS3d: command="+command);
            eval(command);
        }
//         if (disconnect) print("StantonSCS3d: "+group+","+signalList[i][1]+" disconnected from "+signalList[i][2]);
//         else print("StantonSCS3d: "+group+","+signalList[i][1]+" connected to "+signalList[i][2]);
    }
    // If disconnecting signals, darken the LEDs on the control surface & soft buttons
    if (disconnect) {
        var CC = 0xB0 + (StantonSCS3d.temp["channel"]-1);
        midi.sendShortMsg(CC,0x62,0x00,StantonSCS3d.temp["device"]);  // C1 LEDs off
        midi.sendShortMsg(CC,0x0C,0x00,StantonSCS3d.temp["device"]);  // S3 LEDs off
        midi.sendShortMsg(CC,0x01,0x00,StantonSCS3d.temp["device"]);  // S4 LEDs off
        midi.sendShortMsg(CC,0x0E,0x00,StantonSCS3d.temp["device"]);  // S5 LEDs off
    }
}

// (Dis)connects the mode-independent Mixxx control signals to/from functions based on the currently controlled virtual deck
StantonSCS3d.connectDeckSignals = function (disconnect) {
    var signalList = StantonSCS3d.deckSignals;
    for (i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        var name = signalList[i][1];
        if (group=="CurrentChannel") group = "[Channel"+StantonSCS3d.deck+"]";
        engine.connectControl(group,name,signalList[i][2],disconnect);
//         print("StantonSCS3d: (dis)connected "+group+","+name+" to/from "+signalList[i][2]);
        
        // If connecting a signal, update the LEDs
        if (!disconnect) {
            switch (name) {
                case "play":
                        var currentValue = engine.getValue(group,name);
//                         print("StantonSCS3d: current value="+currentValue);
                        StantonSCS3d.playLED(currentValue);
                        break;
                case "cue_default":
                case "beatsync":
                case "bpm": break;
                default:    // Cause the signal to fire to update LEDs
//                         engine.setValue(group,name,engine.getValue(group,name));
//                         engine.trigger(group,name);  // This should be used but it can segfault due to slotSet
//                     Workaround:
                        var command = signalList[i][2]+"("+engine.getValue(group,name)+")";
//                         print("StantonSCS3d: command="+command);
                        eval(command);
                        break;
            }
        }
        
//         if (disconnect) print("StantonSCS3d: "+group+","+signalList[i][1]+" disconnected from "+signalList[i][2]);
//         else print("StantonSCS3d: "+group+","+signalList[i][1]+" connected to "+signalList[i][2]);
    }
    // If disconnecting signals, darken the corresponding LEDs
    if (disconnect) {
        var CC = 0xB0 + (StantonSCS3d.temp["channel"]-1);
        var No = 0x90 + (StantonSCS3d.temp["channel"]-1);
        midi.sendShortMsg(CC,0x07,0x00,StantonSCS3d.temp["device"]);  // Gain LEDs off
        midi.sendShortMsg(CC,0x03,0x00,StantonSCS3d.temp["device"]);  // Pitch LEDs off
        midi.sendShortMsg(No,0x6D,0x00,StantonSCS3d.temp["device"]);  // PLAY button blue
        midi.sendShortMsg(No,0x6E,0x00,StantonSCS3d.temp["device"]);  // CUE button blue
        midi.sendShortMsg(No,0x6F,0x00,StantonSCS3d.temp["device"]);  // SYNC button blue
        midi.sendShortMsg(No,0x70,0x00,StantonSCS3d.temp["device"]);  // TAP button blue
    }
}

// Sets all mode buttons except Deck to the same color
StantonSCS3d.modeButtonsColor = function (color) {
    var channel = StantonSCS3d.temp["channel"];
    var byte1 = 0x90 + (channel-1);
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["fx"],color,StantonSCS3d.temp["device"]); // Set FX button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["eq"],color,StantonSCS3d.temp["device"]); // Set EQ button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["loop"],color,StantonSCS3d.temp["device"]); // Set Loop button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["trig"],color,StantonSCS3d.temp["device"]); // Set Trig button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["vinyl"],color,StantonSCS3d.temp["device"]); // Set Vinyl button color
}

// Sets all four soft buttons to the same color
StantonSCS3d.softButtonsColor = function (color) {
    var channel = StantonSCS3d.temp["channel"];
    var byte1 = 0x90 + (channel-1);
    midi.sendShortMsg(byte1,0x2c,color,StantonSCS3d.temp["device"]); // Set B11 button color
    midi.sendShortMsg(byte1,0x2e,color,StantonSCS3d.temp["device"]); // Set B12 button color
    midi.sendShortMsg(byte1,0x30,color,StantonSCS3d.temp["device"]); // Set B13 button color
    midi.sendShortMsg(byte1,0x32,color,StantonSCS3d.temp["device"]); // Set B14 button color
}

// Sets color of side circle LEDs (used for deck change effect)
StantonSCS3d.circleLEDsColor = function (color,side) {
    var channel = StantonSCS3d.temp["channel"];
    var byte1 = 0x90 + (channel-1);
    var start;
    var end;
    if (side=="left") { start = 0x5e; end = 0x63; }
    else { start = 0x66; end = 0x6b; }
    for (i=start; i<=end; i++) midi.sendShortMsg(byte1,i,color,StantonSCS3d.temp["device"]);
}

StantonSCS3d.pitch = function (channel, device, control, value) {   // Lower the sensitivity of the pitch slider
    if (StantonSCS3d.modifier[StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]]==1) return;
    var currentValue = engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate");
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate",currentValue+(value-64)/256);
}

StantonSCS3d.pitchAbsolute = function (channel, device, control, value) {   // Pitch bending at the edges of the slider
    if (StantonSCS3d.state["pitchAbs"]==0) StantonSCS3d.state["pitchAbs"]=value;    // Log the initial value
    
    // Ignore if the slider was first touched in the middle
    if (StantonSCS3d.state["pitchAbs"]>=10 && StantonSCS3d.state["pitchAbs"]<=117) return;
    
    if (value<10) engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down",1);
    if (value>117) engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up",1);
}

// Reset the pitch to 0 if the slider is touched while "vinyl" is held down
StantonSCS3d.pitchTouch = function (channel, device, control, value, category) {
    if (category == (0x80 + channel-1)) {   // If button up
        StantonSCS3d.state["pitchAbs"]=0;   // Clear the initial value
        if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down") != 0)
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down",0);
        if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up") != 0)
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up",0);
    }
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1)
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate",0);
}

StantonSCS3d.gain = function (channel, device, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1) return;
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","volume",value/127);
}

StantonSCS3d.gainRelative = function (channel, device, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1)  {   // If mode button held, set pre-gain level
        var newValue = engine.getValue("[Channel"+StantonSCS3d.deck+"]","pregain")+(value-64)/64;
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","pregain",newValue);
        var add = StantonSCS3d.BoostCut9(newValue, 0.0, 1.0, 4.0, 5, 4);
        var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
        midi.sendShortMsg(byte1,0x07,0x15+add,StantonSCS3d.temp["device"]);
    }
}

StantonSCS3d.playButton = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","play",!engine.getValue("[Channel"+StantonSCS3d.deck+"]","play"));
        return;
    }
}

StantonSCS3d.cueButton = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","cue_default",1);
        StantonSCS3d.modifier["cue"]=1;   // Set button modifier flag
        return;
    }
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","cue_default",0);
    StantonSCS3d.modifier["cue"]=0;   // Clear button modifier flag
}

StantonSCS3d.syncButton = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","beatsync",1);
        return;
    }
    midi.sendShortMsg(byte1,control,0x00,StantonSCS3d.temp["device"]);  // SYNC button blue
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","beatsync",0);
}

StantonSCS3d.tapButton = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        print("StantonSCS3d: Switchable deck TAP functionality not working yet.");
//         engine.setValue("[Channel"+StantonSCS3d.deck+"]","bpm",1);
        return;
    }
//     engine.setValue("[Channel"+StantonSCS3d.deck+"]","bpm",0);
}

StantonSCS3d.B11 = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        StantonSCS3d.modifier["B11"]=1;   // Set button modifier flag
    }
    else {
        StantonSCS3d.modifier["B11"]=0;   // Clear button modifier flag
    }
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    switch (currentMode) {
        case "fx":
//                 if (category != (0x80 + channel-1))     // If button down
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","reverse",!engine.getValue("[Channel"+StantonSCS3d.deck+"]","reverse"));
                break;
        case "vinyl":
                if (category != (0x80 + channel-1)) {    // If button down
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","pfl",!engine.getValue("[Channel"+StantonSCS3d.deck+"]","pfl"));
                }
                break;
        case "vinyl2":
                if (category != (0x80 + channel-1)) {    // If button down
                    engine.setValue("[Playlist]","SelectPrevPlaylist",1);
                }
                else engine.setValue("[Playlist]","SelectPrevPlaylist",0);
                break;
    }
}

StantonSCS3d.B12 = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (category != (0x80 + channel-1)) {    // If button down
        StantonSCS3d.modifier["B12"]=1;   // Set button modifier flag
        if (currentMode == "vinyl" || StantonSCS3d.modifier["Deck"]==1)
            midi.sendShortMsg(byte1,control,0x01,StantonSCS3d.temp["device"]); // Make button red
        if (StantonSCS3d.modifier["Deck"]==1) {
            engine.setValue("[Master]","crossfader",0.0); // Reset cross-fader to center
            midi.sendShortMsg(0xB0+(channel-1),0x01,0x18,StantonSCS3d.temp["device"]);  // Show it centered on S4
            return;
        }
    }
    else {  // If button up
        StantonSCS3d.modifier["B12"]=0;   // Clear button modifier flag
        if (currentMode == "vinyl" || StantonSCS3d.modifier["Deck"]==1)
            midi.sendShortMsg(byte1,control,0x02,StantonSCS3d.temp["device"]); // Make button blue
    }
    switch (currentMode) {
        case "fx":
                if (category != (0x80 + channel-1))     // If button down
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","flanger",!engine.getValue("[Channel"+StantonSCS3d.deck+"]","flanger"));
                break;
        case "vinyl":
                if (category != (0x80 + channel-1)) {    // If button down
                    var currentRange = engine.getValue("[Channel"+StantonSCS3d.deck+"]","rateRange");
                    switch (true) {
                        case (currentRange<=StantonSCS3d.pitchRanges[0]):
                                engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[1]);
                            break;
                        case (currentRange<=StantonSCS3d.pitchRanges[1]):
                                engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[2]);
                            break;
                        case (currentRange<=StantonSCS3d.pitchRanges[2]):
                                engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[3]);
                            break;
                        case (currentRange>=StantonSCS3d.pitchRanges[3]):
                                engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[0]);
                            break;
                    }
                    // Set the pitch slider to the same value to update the screen display
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate",engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate"));
                }
                break;
        case "vinyl2":
                if (category != (0x80 + channel-1)) {    // If button down
                    engine.setValue("[Playlist]","SelectNextPlaylist",1);
                }
                else engine.setValue("[Playlist]","SelectNextPlaylist",0);
                break;
    }
}

StantonSCS3d.B13 = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","back",1);
        return;
    }
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","back",0);
}

StantonSCS3d.B14 = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","fwd",1);
        return;
    }
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","fwd",0);
}

// ----------   Mode buttons  ----------

StantonSCS3d.modeButton = function (channel, control, category, modeName) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        midi.sendShortMsg(byte1,control,0x03,StantonSCS3d.temp["device"]); // Make button purple
        StantonSCS3d.modifier[modeName]=1;   // Set mode modifier flag
        if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == modeName) {
            // Set Gain LEDs to pregain value
            var add = StantonSCS3d.BoostCut9(engine.getValue("[Channel"+StantonSCS3d.deck+"]","pregain"), 0.0, 1.0, 4.0, 5, 4);
            midi.sendShortMsg(0xB0+(channel-1),0x07,0x15+add,StantonSCS3d.temp["device"]);
        }
        return;
    }
    StantonSCS3d.modifier[modeName]=0;   // Clear mode modifier flag
    StantonSCS3d.gainLEDs(engine.getValue("[Channel"+StantonSCS3d.deck+"]","volume"));  // Restore Gain LEDs
    StantonSCS3d.modeButtonsColor(0x02);  // Make all mode buttons blue
    midi.sendShortMsg(byte1,control,0x01,StantonSCS3d.temp["device"]); // Make button red
    if (modeName=="vinyl2") midi.sendShortMsg(byte1,control,0x00,StantonSCS3d.temp["device"]); // Make button black
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == modeName) return;
    print("StantonSCS3d: Switching to "+modeName.toUpperCase()+" mode on deck "+StantonSCS3d.deck);
    StantonSCS3d.softButtonsColor(0x02);  // Make the soft buttons blue
    StantonSCS3d.connectSurfaceSignals(true);  // Disconnect previous ones
    switch (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]) {    // Special recovery from certain modes
        case "loop":
        case "trig":
                var redButtonLEDs = [0x48, 0x4a, 0x4c, 0x4e, 0x4f, 0x51, 0x53, 0x55];
                for (i=0; i<redButtonLEDs.length; i++)
                    midi.sendShortMsg(byte1,redButtonLEDs[i],0x40,StantonSCS3d.temp["device"]); // Set them to black
                for (i=0x56; i<=0x5c; i++)
                    midi.sendShortMsg(byte1,i,0x40,StantonSCS3d.temp["device"]); // Set center slider to black
            break;
    }
    midi.sendSysexMsg(StantonSCS3d.sysex.concat([0x01, StantonSCS3d.surface[StantonSCS3d.modeSurface[modeName]], 0xF7]),7);  // Configure surface
    switch (modeName) {    // Prep LEDs for certain modes
        case "loop":
        case "trig":
                var redButtonLEDs = [0x48, 0x4a, 0x4c, 0x4e, 0x4f, 0x51, 0x53, 0x55];
                for (i=0; i<redButtonLEDs.length; i++)
                    midi.sendShortMsg(byte1,redButtonLEDs[i],0x41,StantonSCS3d.temp["device"]); // Set them to red dim
                for (i=0x56; i<=0x5c; i++)
                    midi.sendShortMsg(byte1,i,0x41,StantonSCS3d.temp["device"]); // Set center slider to red dim
                if (modeName!="trig") break;
                // Light the blue circle LEDs for any cues currently set
                for (i=0; i<redButtonLEDs.length; i++) {
                    if (StantonSCS3d.deck==1) {
                        if (StantonSCS3d.triggerPoints1[redButtonLEDs[i]] != -0.1) 
                            midi.sendShortMsg(byte1,StantonSCS3d.buttonLEDs[redButtonLEDs[i]],0x01,StantonSCS3d.temp["device"]);
                    }
                    else {
                        if (StantonSCS3d.triggerPoints2[redButtonLEDs[i]] != -0.1) 
                            midi.sendShortMsg(byte1,StantonSCS3d.buttonLEDs[redButtonLEDs[i]],0x01,StantonSCS3d.temp["device"]);
                    }
                }
            break;
    }
    StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = modeName;
    StantonSCS3d.connectSurfaceSignals();  // Connect new ones
}

StantonSCS3d.FX = function (channel, device, control, value, category) {
    StantonSCS3d.modeButton(channel, control, category, "fx");
}

StantonSCS3d.EQ = function (channel, device, control, value, category) {
    StantonSCS3d.modeButton(channel, control, category, "eq");
}

StantonSCS3d.Loop = function (channel, device, control, value, category) {
    StantonSCS3d.modeButton(channel, control, category, "loop");
}

StantonSCS3d.Trig = function (channel, device, control, value, category) {
    StantonSCS3d.modeButton(channel, control, category, "trig");
}

StantonSCS3d.Vinyl = function (channel, device, control, value, category) {
    // If in vinyl mode
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]=="vinyl") {
        // On button up, switch to vinyl2 mode. Else (on button down) stay in vinyl mode.
        if (category == (0x80 + channel-1)) StantonSCS3d.modeButton(channel, control, category, "vinyl2");
        else StantonSCS3d.modeButton(channel, control, category, "vinyl");
        return;
    }
    // If in vinyl2 mode
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]=="vinyl2") {
        // On button up, switch to vinyl mode. Else (on button down) stay in vinyl2 mode.
        if (category == (0x80 + channel-1)) StantonSCS3d.modeButton(channel, control, category, "vinyl");
        else StantonSCS3d.modeButton(channel, control, category, "vinyl2");
        return;
    }
    // In all other cases, switch to vinyl mode
    StantonSCS3d.modeButton(channel, control, category, "vinyl");
}

StantonSCS3d.lightDelay = function () {
    var date = new Date();
    var curDate = null;
    
    do { curDate = new Date(); }
    while(curDate-date < 60);
}

StantonSCS3d.DeckChange = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // Only respond to button up events
        StantonSCS3d.modeButtonsColor(0x02);  // Make all mode buttons blue
        midi.sendShortMsg(byte1,control,0x01,StantonSCS3d.temp["device"]); // Make button red
        StantonSCS3d.modifier["Deck"]=1;   // Set button modifier flag
        // Show the current crossfader position on S4
        midi.sendShortMsg(0xB0+(channel-1),0x0C,0x00,StantonSCS3d.temp["device"]);  // Darken S3
        midi.sendShortMsg(0xB0+(channel-1),0x0E,0x00,StantonSCS3d.temp["device"]);  // Darken S5
        var add = StantonSCS3d.BoostCut7(engine.getValue("[Master]","crossfader"), -1.0, 0.0, 1.0, 4, 3);
        midi.sendShortMsg(0xB0+(channel-1),0x01,0x15+add,StantonSCS3d.temp["device"]);  // Show position on S4
        return;
    }
    StantonSCS3d.modifier["Deck"]=0;   // Clear button modifier flag
    StantonSCS3d.connectSurfaceSignals(true);   // Disconnect signals from the outgoing deck & turn off surface LEDs
    StantonSCS3d.connectDeckSignals(true);    // Disconnect static signals
    StantonSCS3d.softButtonsColor(0x00);  // Darken the soft buttons
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == "trig" || StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == "loop")
        for (i=0x48; i<=0x5c; i++) midi.sendShortMsg(byte1,i,0x40,StantonSCS3d.temp["device"]); // Set surface LEDs to black
    if (StantonSCS3d.deck == 1) {
        print("StantonSCS3d: Switching to deck 2");
        StantonSCS3d.deck++;
        midi.sendShortMsg(byte1,StantonSCS3d.buttons["deck"],0x03,StantonSCS3d.temp["device"]); // Deck button purple
        midi.sendShortMsg(byte1,0x71,0x00,StantonSCS3d.temp["device"]);  // Deck A light off
        if (!StantonSCS3d.fastDeckChange) { // Make flashy lights to signal a deck change
            StantonSCS3d.circleLEDsColor(0x00,"right");
            StantonSCS3d.lightDelay();
            midi.sendShortMsg(byte1,0x72,0x01,StantonSCS3d.temp["device"]);  // Deck B light on
            StantonSCS3d.circleLEDsColor(0x01,"right");
            StantonSCS3d.lightDelay();
            midi.sendShortMsg(byte1,0x72,0x00,StantonSCS3d.temp["device"]);  // Deck B light off
            StantonSCS3d.circleLEDsColor(0x00,"right");
            StantonSCS3d.lightDelay();
            midi.sendShortMsg(byte1,0x72,0x01,StantonSCS3d.temp["device"]);  // Deck B light on
            StantonSCS3d.circleLEDsColor(0x01,"right");
            StantonSCS3d.lightDelay();
            midi.sendShortMsg(byte1,0x72,0x00,StantonSCS3d.temp["device"]);  // Deck B light off
            StantonSCS3d.circleLEDsColor(0x00,"right");
            StantonSCS3d.lightDelay();
        }
            midi.sendShortMsg(byte1,0x72,0x01,StantonSCS3d.temp["device"]);  // Deck B light on
    }
    else {
        print("StantonSCS3d: Switching to deck 1");
        StantonSCS3d.deck--;
        midi.sendShortMsg(byte1,StantonSCS3d.buttons["deck"],0x02,StantonSCS3d.temp["device"]); // Deck button blue
        midi.sendShortMsg(byte1,0x72,0x00,StantonSCS3d.temp["device"]);  // Deck B light off
        if (!StantonSCS3d.fastDeckChange) {
            StantonSCS3d.circleLEDsColor(0x00,"left");
            StantonSCS3d.lightDelay();
            midi.sendShortMsg(byte1,0x71,0x01,StantonSCS3d.temp["device"]);  // Deck A light on
            StantonSCS3d.circleLEDsColor(0x01,"left");
            StantonSCS3d.lightDelay();
            midi.sendShortMsg(byte1,0x71,0x00,StantonSCS3d.temp["device"]);  // Deck A light off
            StantonSCS3d.circleLEDsColor(0x00,"left");
            StantonSCS3d.lightDelay();
            midi.sendShortMsg(byte1,0x71,0x01,StantonSCS3d.temp["device"]);  // Deck A light on
            StantonSCS3d.circleLEDsColor(0x01,"left");
            StantonSCS3d.lightDelay();
            midi.sendShortMsg(byte1,0x71,0x00,StantonSCS3d.temp["device"]);  // Deck A light off
            StantonSCS3d.circleLEDsColor(0x00,"left");
            StantonSCS3d.lightDelay();
        }
        midi.sendShortMsg(byte1,0x71,0x01,StantonSCS3d.temp["device"]);  // Deck A light on
    }
    var newMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "none"; // Forces a mode change when a function is called
    StantonSCS3d.connectDeckSignals();    // Connect static signals
    switch (newMode) {    // Call the appropriate mode change function to set the control surface & connect signals on the now-current deck
        case "fx":      StantonSCS3d.FX(channel, device, StantonSCS3d.buttons["fx"], value, 0x80 + channel-1); break;
        case "eq":      StantonSCS3d.EQ(channel, device, StantonSCS3d.buttons["eq"], value, 0x80 + channel-1); break;
        case "loop":    StantonSCS3d.Loop(channel, device, StantonSCS3d.buttons["loop"], value, 0x80 + channel-1); break;
        case "trig":    StantonSCS3d.Trig(channel, device, StantonSCS3d.buttons["trig"], value, 0x80 + channel-1); break;
        case "vinyl":   
        case "vinyl2":   StantonSCS3d.Vinyl(channel, device, StantonSCS3d.buttons["vinyl"], value, 0x80 + channel-1); break;
    }
}   // End Deck Change function

// ----------   Sliders  ----------

StantonSCS3d.S4relative = function (channel, device, control, value) {
        return;
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] != "vinyl") return;   // Skip if not in vinyl mode

//     print("S4="+value);
//     value -= 64;
//     value = -value/5;

    // From asantoni:
    // value = 1.0f + (value-64)/64.0f <strike>
    //
    // x = (val-64)/64
    // vel = C*(x-oldX)/(time-oldTime)
    // set scratch to 1+vel if stopped, just vel if playing
    //
    // Init oldX to 1.0, oldTime to when first touched?
    
    var oldX = StantonSCS3d.scratch["track"];
    var oldTime = StantonSCS3d.scratch["time"];
    StantonSCS3d.scratch["time"] = new Date()/1000;
    
    StantonSCS3d.scratch["track"] = (value-64)/64;
    StantonSCS3d.scratch["scratch"] = StantonSCS3d.scratch["revtime"] * (StantonSCS3d.scratch["track"] - oldX) / (StantonSCS3d.scratch["time"] - oldTime);
    
    print("scratch="+StantonSCS3d.scratch["scratch"]+", current time="+StantonSCS3d.scratch["time"]);
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","scratch",StantonSCS3d.scratch["scratch"]);
}

StantonSCS3d.S3absolute = function (channel, device, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    switch (currentMode) {
        case "fx": script.absoluteSlider("[Flanger]","lfoDepth",value,0,1); break;
        case "eq": script.absoluteEQ("[Channel"+StantonSCS3d.deck+"]","filterLow",value); break;
    }
}

StantonSCS3d.S4absolute = function (channel, device, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier["Deck"]==1) { // Adjust the cross-fader if "Deck" is held down
        var add = StantonSCS3d.BoostCut7((value-64)/63, -1.0, 0.0, 1.0, 4, 3);
        var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
        midi.sendShortMsg(byte1,0x01,0x15+add,StantonSCS3d.temp["device"]);
        engine.setValue("[Master]","crossfader",(value-64)/63);
        return;
    }
    switch (currentMode) {
        case "fx": script.absoluteSlider("[Flanger]","lfoDelay",value,50,10000); break;
        case "eq": script.absoluteEQ("[Channel"+StantonSCS3d.deck+"]","filterMid",value); break;
        case "vinyl":   // Scratching
//         break;
                // Set slider lights
                var add = StantonSCS3d.Peak7(value,-1,128);
                var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
                midi.sendShortMsg(byte1,0x01,add,StantonSCS3d.temp["device"]); //S4 LEDs
                midi.sendShortMsg(byte1,0x0C,add,StantonSCS3d.temp["device"]); //S3 LEDs
                midi.sendShortMsg(byte1,0x0E,add,StantonSCS3d.temp["device"]); //S5 LEDs
                
                // Call global scratch slider function
                print("StantonSCS3d: Calling scratch.slider");
                var newScratchValue = scratch.slider(StantonSCS3d.deck, value, StantonSCS3d.scratch["revtime"], StantonSCS3d.scratch["alpha"], StantonSCS3d.scratch["beta"]);
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","scratch",newScratchValue);
            break;
    }
}

StantonSCS3d.S5absolute = function (channel, device, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    switch (currentMode) {
        case "fx": script.absoluteSlider("[Flanger]","lfoPeriod",value,50000,2000000); break;
        case "eq": script.absoluteEQ("[Channel"+StantonSCS3d.deck+"]","filterHigh",value); break;
    }
}

StantonSCS3d.C1touch = function (channel, device, control, value, category) {
    var byte1 = 0xB0 + (channel-1);
    if (category == (0x80 + channel-1)) {    // If button up
        midi.sendShortMsg(byte1,0x62,0x00,StantonSCS3d.temp["device"]); // Turn off C1 lights
    }
}

StantonSCS3d.S3touch = function () {
    // Reset the value to center if the slider is touched while the mode button is held down
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1){
        switch (currentMode) {
            case "fx": engine.setValue("[Flanger]","lfoDepth",0.5); break;
            case "eq": engine.setValue("[Channel"+StantonSCS3d.deck+"]","filterLow",1); break;
        }
    }
}

StantonSCS3d.S4touch = function (channel, device, control, value, category) {
    if (StantonSCS3d.modifier["Deck"]==1) return;   // If we're modifying the cross-fader, ignore this touch
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1){ // If the current mode button is held down, reset the control to center
        switch (currentMode) {
            case "fx": engine.setValue("[Flanger]","lfoDelay",4950); break;
            case "eq": engine.setValue("[Channel"+StantonSCS3d.deck+"]","filterMid",1); break;
        }
    }
    if (category != (0x80 + channel-1)) {    // If button down
        switch (currentMode) {
            case "vinyl":   // Store scratch info the point it was touched
                scratch.enable(StantonSCS3d.deck);
                break;
            case "loop":
                StantonSCS3d.S4buttonLights(true); break;
            case "trig":
                StantonSCS3d.S4buttonLights(true); break;
            case "vinyl2":
                StantonSCS3d.S4buttonLights(true);
                engine.setValue("[Playlist]","LoadSelectedIntoFirstStopped",1);
                break;
        }
        return;
    }
    // If button up
    switch (currentMode) {
        case "vinyl":   // Reset the triggers
            scratch.disable(StantonSCS3d.deck);
            var byte1a = 0xB0 + (StantonSCS3d.temp["channel"]-1);
            midi.sendShortMsg(byte1a,0x01,0x00,StantonSCS3d.temp["device"]); //S4 LEDs off
            midi.sendShortMsg(byte1a,0x0C,0x00,StantonSCS3d.temp["device"]); //S3 LEDs off
            midi.sendShortMsg(byte1a,0x0E,0x00,StantonSCS3d.temp["device"]); //S5 LEDs off
            break;
        case "loop":
        case "trig":
        case "vinyl2":
            StantonSCS3d.S4buttonLights(false); break;
    }
}

// Reset the value to center if the slider is touched while the mode button is held down
StantonSCS3d.S5touch = function () {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1){
        switch (currentMode) {
            case "fx": engine.setValue("[Flanger]","lfoPeriod",1025000); break;
            case "eq": engine.setValue("[Channel"+StantonSCS3d.deck+"]","filterHigh",1); break;
        }
    }
}

StantonSCS3d.S4buttonLights = function (light) {     // Turn on/off button lights
    var byte1 = 0x90 + (StantonSCS3d.temp["channel"]-1);
    var color=0x00; // Off
    if (light) color=0x01;  // On
    midi.sendShortMsg(byte1,0x64,color,StantonSCS3d.temp["device"]);
    midi.sendShortMsg(byte1,0x65,color,StantonSCS3d.temp["device"]);
    midi.sendShortMsg(byte1,0x5d,color,StantonSCS3d.temp["device"]);
    midi.sendShortMsg(byte1,0x6c,color,StantonSCS3d.temp["device"]);
    midi.sendShortMsg(byte1,0x56,color,StantonSCS3d.temp["device"]);
    midi.sendShortMsg(byte1,0x5c,color,StantonSCS3d.temp["device"]);
}

StantonSCS3d.C1relative = function (channel, device, control, value, category) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    switch (currentMode) {
        case "vinyl":
            var newValue=(value-64)*2+engine.getValue("[Channel"+StantonSCS3d.deck+"]","jog");
            print("C1="+value+", jog="+newValue);
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","jog",newValue);
            break;
        case "vinyl2":
            if ((value-64)>0) {
                print("C1="+value+", NextTrack");
                engine.setValue("[Playlist]","SelectNextTrack",1);
            }
            else {
                print("C1="+value+", PrevTrack");
                engine.setValue("[Playlist]","SelectPrevTrack",1);
            }
            break;
    }
}

StantonSCS3d.C1absolute = function (channel, device, control, value, category) {
    // Light the LEDs
    var byte1 = 0xB0 + (channel-1);
    var light = Math.floor(value/8)+1;
//     print("light="+light);
    midi.sendShortMsg(byte1,0x62,light,StantonSCS3d.temp["device"]);
}

// ----------   Surface buttons  ----------

StantonSCS3d.SurfaceButton = function (channel, device, control, value, category) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        midi.sendShortMsg(byte1,control,0x01,StantonSCS3d.temp["device"]); // Turn on button lights
        midi.sendShortMsg(byte1,StantonSCS3d.buttonLEDs[control],0x01,StantonSCS3d.temp["device"]);
        switch (currentMode) {
            case "loop": break;
            case "trig":    // Multiple cue points. Multidimensional array didn't work.
                    if (StantonSCS3d.modifier[currentMode]==1) {
                        if (StantonSCS3d.deck==1) StantonSCS3d.triggerPoints1[control] = -0.1;
                        else StantonSCS3d.triggerPoints2[control] = -0.1;
                        break;
                    }
                    if (StantonSCS3d.deck==1) {
                        if (StantonSCS3d.triggerPoints1[control] == -0.1)
                            StantonSCS3d.triggerPoints1[control] = engine.getValue("[Channel"+StantonSCS3d.deck+"]","playposition");
                        else engine.setValue("[Channel"+StantonSCS3d.deck+"]","playposition",StantonSCS3d.triggerPoints1[control]); 
                    break;
                    }   // End deck 1
                    else {
                        if (StantonSCS3d.triggerPoints2[control] == -0.1)
                            StantonSCS3d.triggerPoints2[control] = engine.getValue("[Channel"+StantonSCS3d.deck+"]","playposition");
                        else engine.setValue("[Channel"+StantonSCS3d.deck+"]","playposition",StantonSCS3d.triggerPoints2[control]); 
                    break;
                    }
        }
        return;
    }
    midi.sendShortMsg(byte1,control,0x00,StantonSCS3d.temp["device"]); // Turn off red button LED
    // Don't extinguish the blue LED if a cue point isn't set on that button
    if (currentMode=="trig" && ((StantonSCS3d.deck==1 && StantonSCS3d.triggerPoints1[control] != -0.1) ||
        (StantonSCS3d.deck==2 && StantonSCS3d.triggerPoints2[control] != -0.1)) ) return;
    midi.sendShortMsg(byte1,StantonSCS3d.buttonLEDs[control],0x00,StantonSCS3d.temp["device"]);
}

// Workarounds for not being able to map multiple buttons to the same Mixxx control:
StantonSCS3d.B15 = function (channel, device, control, value, category) {
    StantonSCS3d.SurfaceButton(channel, device, control, value, category);
}

StantonSCS3d.B16 = function (channel, device, control, value, category) {
    StantonSCS3d.SurfaceButton(channel, device, control, value, category);
}

StantonSCS3d.B17 = function (channel, device, control, value, category) {
    StantonSCS3d.SurfaceButton(channel, device, control, value, category);
}

StantonSCS3d.B18 = function (channel, device, control, value, category) {
    StantonSCS3d.SurfaceButton(channel, device, control, value, category);
}

StantonSCS3d.B19 = function (channel, device, control, value, category) {
    StantonSCS3d.SurfaceButton(channel, device, control, value, category);
}

StantonSCS3d.B20 = function (channel, device, control, value, category) {
    StantonSCS3d.SurfaceButton(channel, device, control, value, category);
}

StantonSCS3d.B21 = function (channel, device, control, value, category) {
    StantonSCS3d.SurfaceButton(channel, device, control, value, category);
}

StantonSCS3d.B22 = function (channel, device, control, value, category) {
    StantonSCS3d.SurfaceButton(channel, device, control, value, category);
}

// ----------   LED slot functions  ----------

StantonSCS3d.buttonLED = function (value, note, on, off) {
    var byte1 = 0x90 + (StantonSCS3d.temp["channel"]-1);
    if (value>0) midi.sendShortMsg(byte1,note,on,StantonSCS3d.temp["device"]);
    else midi.sendShortMsg(byte1,note,off,StantonSCS3d.temp["device"]);
}

// Transport buttons

StantonSCS3d.playLED = function (value) {
    StantonSCS3d.buttonLED(value, 0x6D, 0x01, 0x00);
}

StantonSCS3d.cueLED = function (value) {
    StantonSCS3d.buttonLED(value, 0x6E, 0x01, 0x00);
}

StantonSCS3d.syncLED = function (value) {
    StantonSCS3d.buttonLED(value, 0x6F, 0x01, 0x00);
}

StantonSCS3d.tapLED = function (value) {
//     StantonSCS3d.buttonLED(value, 0x70, 0x01, 0x00);
}

// Soft buttons

StantonSCS3d.B11LED = function (value) {
    StantonSCS3d.buttonLED(value, 0x2C, 0x01, 0x02);
}

StantonSCS3d.B12LED = function (value) {
    StantonSCS3d.buttonLED(value, 0x2E, 0x01, 0x02);
}

StantonSCS3d.B13LED = function (value) {
    StantonSCS3d.buttonLED(value, 0x30, 0x01, 0x02);
}

StantonSCS3d.B14LED = function (value) {
    StantonSCS3d.buttonLED(value, 0x32, 0x01, 0x02);
}

StantonSCS3d.BoostCut7 = function (value, low, mid, high, lowMidSteps, midHighSteps) {
    var LEDs = 0;
    var lowMidRange = (mid-low)/lowMidSteps;
    var midHighRange = (high-mid)/midHighSteps;
//     if (value>low)
    if (value>low+lowMidRange) LEDs++;
    if (value>low+lowMidRange*2) LEDs++;
    if (value>low+lowMidRange*3) LEDs++;
    if (value>mid) LEDs++;
    if (value>mid+midHighRange) LEDs++;
    if (value>mid+midHighRange*2) LEDs++;
    if (value>=high) LEDs++;
//     print("Value="+value+", LEDs="+LEDs);
    return LEDs;
}

StantonSCS3d.BoostCut9 = function (value, low, mid, high, lowMidSteps, midHighSteps) {
    var LEDs = 0;
    var lowMidRange = (mid-low)/lowMidSteps;
    var midHighRange = (high-mid)/midHighSteps;
    if (value>low) LEDs++;
    if (value>lowMidRange) LEDs++;
    if (value>lowMidRange*2) LEDs++;
    if (value>lowMidRange*3) LEDs++;
//     if (value>lowMidRange*4) LEDs++;
    if (value>mid) LEDs++;
    if (value>mid+midHighRange) LEDs++;
    if (value>mid+midHighRange*2) LEDs++;
    if (value>mid+midHighRange*3) LEDs++;
    if (value>=high) LEDs++;
//     print("Value="+value+", LEDs="+LEDs);
    return LEDs;
}

StantonSCS3d.Peak7 = function (value, low, high) {
    var LEDs = 0;
    var range = (high-low)/7;
    if (value>low) LEDs++;
    if (value>low+range) LEDs++;
    if (value>low+range*2) LEDs++;
    if (value>low+range*3) LEDs++;
    if (value>low+range*4) LEDs++;
    if (value>low+range*5) LEDs++;
    if (value>low+range*6) LEDs++;
    if (value>=high) LEDs++;
    return LEDs;
}

StantonSCS3d.EQLowLEDs = function (value) {
    var add = StantonSCS3d.BoostCut7(value, 0, 1, 4, 4, 3);
    var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x0C,0x15+add,StantonSCS3d.temp["device"]);
}

StantonSCS3d.EQMidLEDs = function (value) {
    var add = StantonSCS3d.BoostCut7(value, 0, 1, 4, 4, 3);
    var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x01,0x15+add,StantonSCS3d.temp["device"]);
}

StantonSCS3d.EQHighLEDs = function (value) {
    var add = StantonSCS3d.BoostCut7(value, 0, 1, 4, 4, 3);
    var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x0E,0x15+add,StantonSCS3d.temp["device"]);
}

StantonSCS3d.FXDepthLEDs = function (value) {
    var add = StantonSCS3d.Peak7(value,0,1);
    var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x0C,0x28+add,StantonSCS3d.temp["device"]);
}

StantonSCS3d.FXDelayLEDs = function (value) {
    var add = StantonSCS3d.Peak7(value,50,10000);
    var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x01,0x28+add,StantonSCS3d.temp["device"]);
}

StantonSCS3d.FXPeriodLEDs = function (value) {
    var add = StantonSCS3d.Peak7(value,50000,2000000);
    var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x0E,0x28+add,StantonSCS3d.temp["device"]);
}


StantonSCS3d.pitchLEDs = function (value) {
    var LEDs = 0;
    if (value>=-1) LEDs++;
    if (value>-0.78) LEDs++;
    if (value>-0.56) LEDs++;
    if (value>-0.33) LEDs++;
    if (value>-0.11) LEDs++;
    if (value>0.11) LEDs++;
    if (value>0.33) LEDs++;
    if (value>0.56) LEDs++;
    if (value>0.78) LEDs++;
    var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x03,0x14+LEDs,StantonSCS3d.temp["device"]);
}

StantonSCS3d.gainLEDs = function (value) {
    var LEDs = 0;
    if (value>0.01) LEDs++;
    if (value>0.13) LEDs++;
    if (value>0.26) LEDs++;
    if (value>0.38) LEDs++;
    if (value>0.50) LEDs++;
    if (value>0.63) LEDs++;
    if (value>0.75) LEDs++;
    if (value>0.88) LEDs++;
    if (value>=1) LEDs++;
    var byte1 = 0xB0 + (StantonSCS3d.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x07,0x28+LEDs,StantonSCS3d.temp["device"]);
}

StantonSCS3d.pitchSliderLED = function (value) {
    var byte1 = 0x90 + (StantonSCS3d.temp["channel"]-1);
    switch (true) {
        case (value<=StantonSCS3d.pitchRanges[0]):
//                 midi.sendShortMsg(byte1,0x2E,0x00,StantonSCS3d.temp["device"]); // Make button black
                midi.sendShortMsg(byte1,0x3D,0x00,StantonSCS3d.temp["device"]);  // Pitch LED black
                midi.sendShortMsg(byte1,0x3E,0x00,StantonSCS3d.temp["device"]);
            break;
        case (value<=StantonSCS3d.pitchRanges[1]):
//                 midi.sendShortMsg(byte1,0x2E,0x02,StantonSCS3d.temp["device"]); // Make button blue
                midi.sendShortMsg(byte1,0x3D,0x00,StantonSCS3d.temp["device"]);  // Pitch LED blue
                midi.sendShortMsg(byte1,0x3E,0x01,StantonSCS3d.temp["device"]);
            break;
        case (value<=StantonSCS3d.pitchRanges[2]):
//                 midi.sendShortMsg(byte1,0x2E,0x03,StantonSCS3d.temp["device"]); // Make button purple
                midi.sendShortMsg(byte1,0x3D,0x01,StantonSCS3d.temp["device"]);  // Pitch LED purple
                midi.sendShortMsg(byte1,0x3E,0x01,StantonSCS3d.temp["device"]);
            break;
        case (value>=StantonSCS3d.pitchRanges[3]):
//                 midi.sendShortMsg(byte1,0x2E,0x01,StantonSCS3d.temp["device"]); // Make button red
                midi.sendShortMsg(byte1,0x3D,0x01,StantonSCS3d.temp["device"]);  // Pitch LED red
                midi.sendShortMsg(byte1,0x3E,0x00,StantonSCS3d.temp["device"]);
            break;
    }
}

StantonSCS3d.circleLEDs = function (value) {
//     print("StantonSCS3d: playposition="+value);
}

StantonSCS3d.VUMeter = function (value) {
//     print("StantonSCS3d: VuMeter="+value);
}
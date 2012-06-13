/****************************************************************/
/*      Stanton SCS.1d MIDI controller script v1.10             */
/*          Copyright (C) 2009-2012, Sean M. Pappalardo         */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.11.x, controller firmware v1.25     */
/****************************************************************/

function StantonSCS1d() {}

// ----------   Customization variables ----------
//      See http://mixxx.org/wiki/doku.php/stanton_scs.1d_mixxx_user_guide  for details
StantonSCS1d.pitchRanges = [ 0.08, 0.16, 0.25, 0.5 ];   // Available pitch ranges. You can add more (in ascending order.)
StantonSCS1d.fastDeckChange = false;    // Skip the flashy lights if true, for juggling
StantonSCS1d.globalMode = false;        // Stay in the current modes on deck changes if true
StantonSCS1d.platterSpeed = 0;          // Speed of the platter at 0% pitch: 0=33 RPM, 1=45 RPM
StantonSCS1d.deckChangeWait = 1000;     // Time in milliseconds to hold the Deck Change button down to avoid changing decks
StantonSCS1d.padVelocity = true;        // Use the velocity values when recalling cues on the trigger pads
StantonSCS1d.crossFader = true;         // Use the pitch slider to adjust cross-fader when Range is held down
StantonSCS1d.browseDamp = 3;            // Number of platter ticks to move the highlight one item when browsing the library
StantonSCS1d.looseLoops = true;         // Re-define loop points while set (for loop rolls)

// ----------   Other global variables    ----------
StantonSCS1d.debug = false;  // Enable/disable debugging messages to the console
StantonSCS1d.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
StantonSCS1d.channel = 0;   // MIDI channel the device is on
StantonSCS1d.swVersion = "1.11";   // Mixxx version for display
StantonSCS1d.platterMode = { "default":"vinyl" };   // Set vinyl mode on all decks
StantonSCS1d.padBank = { "deck":1, "default":1 };
StantonSCS1d.triggerBank = { "default":1 };    // Default trigger button bank

StantonSCS1d.deck = 1;  // Currently active virtual deck
StantonSCS1d.buttons = { "control":27, "browse":28, "vinyl":29, "deckSelect":64 };
StantonSCS1d.newPlatterMode;
StantonSCS1d.knobMode = { }; // 1=Low,Mid,High EQ,Vol; 2=Depth,Delay,LFO,Gain; 3=Cuemix,Headvol,Balance,MasterVol
StantonSCS1d.trackDuration = [ ]; // Duration of the song on each deck (used for jog LCD and displays)
StantonSCS1d.lastLight = [ ];    // Last circle LCD values
StantonSCS1d.modifier = { "cue":0, "play":0, "trigger":-1 };  // Modifier buttons (allowing alternate controls) defined on-the-fly if needed
// Temporary state variables
StantonSCS1d.state = { "pitchAbs":0, "dontMove":0, "browseTicks":0, "loopFlash":0, "control":false };
StantonSCS1d.timer = { "loop":-1, "deckChange":-1 };  // Temporary storage of timer IDs
StantonSCS1d.loopActive = [ ]; // Mark which loop is active on each deck, if any
StantonSCS1d.mutex = { };   // Temporary mutual exclusion variables
StantonSCS1d.prevValues = { };  // Temporary previous value storage
StantonSCS1d.inSetup = false;   // Flag for if the device is in setup mode
StantonSCS1d.sysex = [0xF0, 0x00, 0x01, 0x02];  // Preamble for all SysEx messages for this device
StantonSCS1d.sysexCorrect = [0xF0, 0x00, 0x01, 0x60];  // Correct preamble for all SysEx messages for this device (ALSA's HSS1394 translation uses this)
StantonSCS1d.initialized = false;
StantonSCS1d.rpm = [33+1/3,45];    // RPM values for StantonSCS1d.platterSpeed - DO NOT CHANGE!
// Variables used in the scratching alpha-beta filter: (revtime = 1.8 to start)
StantonSCS1d.scratch = { "revtime":(60/StantonSCS1d.rpm[StantonSCS1d.platterSpeed]),
                         "resolution":4000, "alpha":1.0/8, "beta":(1.0/8)/32,
                         "prevTimeStamp":0, "prevState":0 };
// Pitch values for key change mode
StantonSCS1d.pitchPoints = {    1:{ 8:-0.1998, 9:-0.1665, 10:-0.1332, 11:-0.0999, 12:-0.0666, 13:-0.0333,
                                    14:0.0333, 15:0.0666, 18:0.0999, 19:0.1332, 20:0.1665, 21:0.1998 }, // 3.33% increments
                                2:{ 8:-0.5, 9:-0.4043, 10:-0.2905, 11:-0.1567, 12:-0.1058, 13:-0.0548, 
                                    14:0.06, 15:0.12, 18:0.181, 19:0.416, 20:0.688, 21:1.0 },  // Key changes
                                3:{ 8:-0.4370, 9:-0.3677, 10:-0.3320, 11:-0.2495, 12:-0.1567, 13:-0.0548, 
                                    14:0.12, 15:0.263, 18:0.338, 19:0.506, 20:0.688, 21:0.895 } };  // Notes
// Multiple banks of multiple cue points:
StantonSCS1d.hotCues = {    1:{ 0x20: 1, 0x21: 2, 0x22: 3, 0x23: 4 },
                            2:{ 0x20: 5, 0x21: 6, 0x22: 7, 0x23: 8 },
                            3:{ 0x20: 9, 0x21: 10, 0x22: 11, 0x23: 12 } };
// Button-to-index mapping for trigger section
StantonSCS1d.trigButtons = {8:0,  9:1,  10:2,  11:3,
                            12:4, 13:5, 14:6,  15:7,
                            18:8, 19:9, 20:10, 21:11 };
// Button-to-reloop index mapping for trigger section
StantonSCS1d.trigButtonsLoopNr = {  8:1,  9:1,  10:2, 11:2,
                                    12:3, 13:3, 14:4, 15:4,
                                    18:5, 19:5, 20:6, 21:6 };

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
                                ["CurrentChannel", "pfl", "StantonSCS1d.headphoneLED"],
                                ["CurrentChannel", "loop_enabled", "StantonSCS1d.loopEnabled"],
                                ["CurrentChannel", "reloop_exit", "StantonSCS1d.activateLoop"],
                                ["CurrentChannel", "bpm_tap", "StantonSCS1d.bpmLED"],
                                ["CurrentChannel", "visual_playposition", "StantonSCS1d.circleBars"],
                                ["CurrentChannel", "duration","StantonSCS1d.durationChange"]
                            ];
// When Mixxx gets multiple loop capability, just change the key values here for the respective buttons
StantonSCS1d.trigSignals = [ [],    // Bank 0 (non-existent)
                             [  ["CurrentChannel", "loop_start_position", "StantonSCS1d.Trig1ILED"],
                                ["CurrentChannel", "loop_end_position", "StantonSCS1d.Trig1OLED"],
                                ["CurrentChannel", "loop_start_position", "StantonSCS1d.Trig2ILED"],
                                ["CurrentChannel", "loop_end_position", "StantonSCS1d.Trig2OLED"],
                                ["CurrentChannel", "loop_start_position", "StantonSCS1d.Trig3ILED"],
                                ["CurrentChannel", "loop_end_position", "StantonSCS1d.Trig3OLED"],
                                ["CurrentChannel", "loop_start_position", "StantonSCS1d.Trig4ILED"],
                                ["CurrentChannel", "loop_end_position", "StantonSCS1d.Trig4OLED"],
                                ["CurrentChannel", "loop_start_position", "StantonSCS1d.Trig5ILED"],
                                ["CurrentChannel", "loop_end_position", "StantonSCS1d.Trig5OLED"],
                                ["CurrentChannel", "loop_start_position", "StantonSCS1d.Trig6ILED"],
                                ["CurrentChannel", "loop_end_position", "StantonSCS1d.Trig6OLED"]
                            ], [], [] ];    // Only one bank right now
StantonSCS1d.knobText =    [  ["Low EQ","Mid EQ","High EQ","Volume"],
                              ["Depth","Delay","Period","PF Gain"],
                              ["Pre/Main","Head Vol","Balance","M.Volume"]
                            ];
StantonSCS1d.knobSignals = [  [ ["CurrentChannel", "filterLow", "StantonSCS1d.encoder1EQLEDs"],
                                ["CurrentChannel", "filterMid", "StantonSCS1d.encoder2EQLEDs"],
                                ["CurrentChannel", "filterHigh", "StantonSCS1d.encoder3EQLEDs"],
                                ["CurrentChannel", "volume", "StantonSCS1d.encoder4VolumeLEDs"]
                              ],
                              [ ["[Flanger]", "lfoDepth", "StantonSCS1d.FXDepthLEDs"],
                                ["[Flanger]", "lfoDelay", "StantonSCS1d.FXDelayLEDs"],
                                ["[Flanger]", "lfoPeriod", "StantonSCS1d.FXPeriodLEDs"],
                                ["CurrentChannel", "pregain", "StantonSCS1d.encoder4EQLEDs"]
                              ],
                              [ ["[Master]", "headMix", "StantonSCS1d.encoder1BalanceLEDs"],
                                ["[Master]", "headVolume", "StantonSCS1d.encoder2MVolumeLEDs"],
                                ["[Master]", "balance", "StantonSCS1d.encoder3BalanceLEDs"],
                                ["[Master]", "volume", "StantonSCS1d.encoder4MVolumeLEDs"]
                              ]
                           ];
StantonSCS1d.padSignals = [    [],    // Bank 0 (non-existent)
                            [   ["CurrentChannel", "hotcue_1_position", "StantonSCS1d.Pad1LCD"],
                                ["CurrentChannel", "hotcue_2_position", "StantonSCS1d.Pad2LCD"],
                                ["CurrentChannel", "hotcue_3_position", "StantonSCS1d.Pad3LCD"],
                                ["CurrentChannel", "hotcue_4_position", "StantonSCS1d.Pad4LCD"],
                                ["CurrentChannel", "hotcue_1_activate", "StantonSCS1d.Pad1aLED"],
                                ["CurrentChannel", "hotcue_2_activate", "StantonSCS1d.Pad2aLED"],
                                ["CurrentChannel", "hotcue_3_activate", "StantonSCS1d.Pad3aLED"],
                                ["CurrentChannel", "hotcue_4_activate", "StantonSCS1d.Pad4aLED"] ],
                            [   ["CurrentChannel", "hotcue_5_position", "StantonSCS1d.Pad1LCD"],
                                ["CurrentChannel", "hotcue_6_position", "StantonSCS1d.Pad2LCD"],
                                ["CurrentChannel", "hotcue_7_position", "StantonSCS1d.Pad3LCD"],
                                ["CurrentChannel", "hotcue_8_position", "StantonSCS1d.Pad4LCD"],
                                ["CurrentChannel", "hotcue_5_activate", "StantonSCS1d.Pad1aLED"],
                                ["CurrentChannel", "hotcue_6_activate", "StantonSCS1d.Pad2aLED"],
                                ["CurrentChannel", "hotcue_7_activate", "StantonSCS1d.Pad3aLED"],
                                ["CurrentChannel", "hotcue_8_activate", "StantonSCS1d.Pad4aLED"] ],
                            [   ["CurrentChannel", "hotcue_9_position", "StantonSCS1d.Pad1LCD"],
                                ["CurrentChannel", "hotcue_10_position", "StantonSCS1d.Pad2LCD"],
                                ["CurrentChannel", "hotcue_11_position", "StantonSCS1d.Pad3LCD"],
                                ["CurrentChannel", "hotcue_12_position", "StantonSCS1d.Pad4LCD"],
                                ["CurrentChannel", "hotcue_9_activate", "StantonSCS1d.Pad1aLED"],
                                ["CurrentChannel", "hotcue_10_activate", "StantonSCS1d.Pad2aLED"],
                                ["CurrentChannel", "hotcue_11_activate", "StantonSCS1d.Pad3aLED"],
                                ["CurrentChannel", "hotcue_12_activate", "StantonSCS1d.Pad4aLED"] ] ];

// ----------   Functions   ----------

StantonSCS1d.init = function (id,debug) {
    StantonSCS1d.debug = debug;

    // Welcome message
    var message = "Welcome";
    // Set LCD1
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 1],message.toInt(), 0xF7),7+message.length);
    // Set LCD6
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 6],message.toInt(), 0xF7),7+message.length);
    //midi.sendShortMsg(No,49,127);   // to orange
    
    message = "to";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 2],message.toInt(), 0xF7),7+message.length);
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 7],message.toInt(), 0xF7),7+message.length);
    //midi.sendShortMsg(No,49+1,127); // to orange
    
    message = "Mixxx  v";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 3],message.toInt(), 0xF7),7+message.length);
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 8],message.toInt(), 0xF7),7+message.length);
    //midi.sendShortMsg(No,49+2,127); // to orange
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 4],
        StantonSCS1d.swVersion.toInt(), 0xF7),7+StantonSCS1d.swVersion.length);
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 9],
        StantonSCS1d.swVersion.toInt(), 0xF7),7+StantonSCS1d.swVersion.length);
    //midi.sendShortMsg(No,49+3,32);  // to red
    
    StantonSCS1d.id = id;   // Store the ID of this device for later use
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 17, 0xF7]),7); // Extinguish all LEDs
    var CC = 0xB0 + StantonSCS1d.channel;
    var No = 0x90 + StantonSCS1d.channel;

    midi.sendShortMsg(CC,1,'x'.toInt());   // Stop platter
    if (StantonSCS1d.platterSpeed==1) midi.sendShortMsg(CC,1,'2'.toInt());   // 45 RPM
    else midi.sendShortMsg(CC,1,'1'.toInt());   // 33 RPM
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 14, 0, 0xF7]),8);  // Clear Passive mode

    // Ask for firmware version
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 0x0C, 0xF7]),7);
}

StantonSCS1d.init2 = function () {
    
    // Force change to first deck, initializing the LEDs and connecting signals in the process
    StantonSCS1d.state["Oldknob"]=1;
    // Set active deck to last available so the below will switch to #1.
    StantonSCS1d.deck = engine.getValue("[Master]","num_decks");
    // Set the default platter mode for this last deck
    if (!StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"])
        StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = StantonSCS1d.platterMode["default"];
    StantonSCS1d.DeckChange(StantonSCS1d.channel, StantonSCS1d.buttons["deckSelect"], 0x7F, 0x90+StantonSCS1d.channel);
    StantonSCS1d.DeckChange(StantonSCS1d.channel, StantonSCS1d.buttons["deckSelect"], 0x00, 0x80+StantonSCS1d.channel);
    
    //midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 16, 0xF7]),7); // Light all LEDs

    StantonSCS1d.initialized = true;
    print ("StantonSCS1d: \""+StantonSCS1d.id+"\" on MIDI channel "+(StantonSCS1d.channel+1)+" initialized.");
}

StantonSCS1d.shutdown = function () {   // called when the MIDI device is closed
    
    var CC = 0xB0 + StantonSCS1d.channel;
    midi.sendShortMsg(CC,1,'x'.toInt());   // Stop the platter

    // Graffiti :)
    var message = "Mixxx";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 1],message.toInt(), 0xF7),7+message.length);
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 6],message.toInt(), 0xF7),7+message.length);
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 2],
        StantonSCS1d.swVersion.toInt(), 0xF7),7+StantonSCS1d.swVersion.length);
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 7],
        StantonSCS1d.swVersion.toInt(), 0xF7),7+StantonSCS1d.swVersion.length);
    
    message = "was here";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 3],message.toInt(), 0xF7),7+message.length);
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 8],message.toInt(), 0xF7),7+message.length);
    
    message = "";
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 4],message.toInt(), 0xF7),7+message.length);
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 9],message.toInt(), 0xF7),7+message.length);
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 13, 0, 0xF7]),8); // Jog backlight off
    // clear jog LCD character (set to space)
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 36, 0x20, 0xF7]),8);
    midi.sendShortMsg(CC,2,0x00);    // Clear jog circle
    
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 17, 0xF7]),7); // Extinguish all LEDs

    print ("StantonSCS1d: \""+StantonSCS1d.id+"\" on MIDI channel "+(StantonSCS1d.channel+1)+" shut down.");
}

StantonSCS1d.inboundSysex = function (data, length) {
    if (data[3]==StantonSCS1d.sysex[3]) {
        // The only specified sysex from the hardware itself is the firmware
        //  version response
        
        // Assemble the firmware string
        var i=0;
        var out="";
        while (i<(length-6)) {
            out+=String.fromCharCode(data[i+5]);
            i++;
        }
        print("SCS.1d firmware string: "+out);

        // TODO: Check firmware version if possible. Platter behaves very differently after v1.25!
        //StantonSCS1d.fwVersion =

        StantonSCS1d.init2();
    } else if (StantonSCS1d.initialized && data[3]==StantonSCS1d.sysexCorrect[3]) {
        // Check for ALSA's 'HSS' preamble
        if (data[4]==0x48 && data[5]==0x53 && data[6]==0x53) {
            // We're on Linux under ALSA.
            // Parse the long-ass Sysex message into what libhss1394 sends
            //  so the rest of this script code can use it the same way.

            // Platter message
            var offset=2;   // TODO: once Clemens removes the bogus 00, remove this
            var message = [];
            for (i=0; i<=7; i+=2) {  // 4 bytes in the 0xF9 message
                message[i/2] = (data[7+i+offset] << 4) | data[8+i+offset];
//                 print(data[7+i+offset]+" "+data[8+i+offset]);
            }
//             print(message[3]);
            StantonSCS1d.vinylMoved(message,4);
        }
    }
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
        engine.scratchDisable(StantonSCS1d.deck);
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
        if (StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] == "browse") {
            // If already in browse mode, change library view category
            engine.setValue("[Playlist]","SelectNextPlaylist",1);
        }
        else {
            engine.scratchDisable(StantonSCS1d.deck);
            StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = "";    // Avoid problems coming from vinyl mode
            midi.sendShortMsg(byte1,control,1);  // Light 'er up
            midi.sendShortMsg(0x80+channel,0x1B,0);  // turn off the "control" mode button
            midi.sendShortMsg(0x80+channel,0x1D,0);  // turn off the "vinyl" mode button
            midi.sendShortMsg(0xB0 + channel,1,'x'.toInt());   // Stop platter
            engine.setValue("[Playlist]","SelectPrevPlaylist",1);
        }
        return;
    }
    
    // On button up
    if (StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] == "browse") {
        engine.setValue("[Playlist]","SelectNextPlaylist",0);
    }
    else {
        // Switch modes on button up to give the motor a chance to stop
        engine.setValue("[Playlist]","SelectPrevPlaylist",0);
        StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = "browse";
    }
}

StantonSCS1d.vinylButton = function (channel, control, value, status) {
  if (StantonSCS1d.checkInSetup()) return;
    var byte1 = 0x90 + channel;
    if ((status & 0XF0) == 0x90) {    // If button down
        midi.sendShortMsg(byte1,control,1);  // Light 'er up
        midi.sendShortMsg(0x80+channel,0x1B,0);  // turn off the "control" mode button
        midi.sendShortMsg(0x80+channel,0x1C,0);  // turn off the "browse" mode button
        StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = "vinyl";
        
        // Update the platter speed and start it spinning if applicable
        StantonSCS1d.pitchChange(engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate"),true);
        return;
    }
    // Enable direct platter control on button up to give the motor a chance to get to speed
    if (!StantonSCS1d.state["outsideMotor"] && StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] == "vinyl")    
        engine.scratchEnable(StantonSCS1d.deck, StantonSCS1d.scratch["resolution"],
            StantonSCS1d.rpm[StantonSCS1d.platterSpeed],StantonSCS1d.scratch["alpha"],
            StantonSCS1d.scratch["beta"]);
}

// (Dis)connects the appropriate Mixxx control signals to/from functions based on 
//      the currently controlled deck and what mode the encoder knobs are in
StantonSCS1d.connectKnobSignals = function (channel, disconnect) {
    if (StantonSCS1d.debug) print("     StantonSCS1d: "+(disconnect?"DISCONNECT":"connect")+"KnobSignals:");

    // Set default mode
    if (!StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"]) StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"] = 1;
    
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

// (Dis)connects the appropriate Mixxx control signals to/from functions based on the currently controlled deck
StantonSCS1d.connectPadSignals = function (channel, disconnect) {
    if (StantonSCS1d.debug) print("     StantonSCS1d: "+(disconnect?"DISCONNECT":"connect")+"PadSignals:");
    var deck = StantonSCS1d.padBank["deck"];
    var bank = StantonSCS1d.padBank["bank"+deck];
    // Set the default bank if not present
    if (!bank)
        bank = StantonSCS1d.padBank["bank"+deck] = StantonSCS1d.padBank["default"];
    
    var signalList = StantonSCS1d.padSignals[bank];
    for (var i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        if (group=="CurrentChannel") group = "[Channel"+deck+"]";
        engine.connectControl(group,signalList[i][1],signalList[i][2],disconnect);
        
        // If connecting a signal, cause it to fire (by setting it to the same value) to update the LEDs
//         if (!disconnect) engine.trigger(group,signalList[i][1]);  // Commented because there's no sense in wasting queue length
        if (!disconnect) {
            // Alternate:
            var command = signalList[i][2]+"("+engine.getValue(group,signalList[i][1])+")";
            //print("StantonSCS1d: command="+command);
            eval(command);
        }
        if (StantonSCS1d.debug) {
            if (disconnect) print("StantonSCS1d: "+group+","+signalList[i][1]+" disconnected from "+signalList[i][2]);
            else print("StantonSCS1d: "+group+","+signalList[i][1]+" connected to "+signalList[i][2]);
        }
    }
    // If disconnecting signals, darken the LEDs on the pads & clear the displays
    if (disconnect) {
        var CC = 0xB0 + channel;
        midi.sendShortMsg(CC,0x20,0x00);  // Pad 1 LEDs off
        midi.sendShortMsg(CC,0x21,0x00);  // Pad 2 LEDs off
        midi.sendShortMsg(CC,0x22,0x00);  // Pad 3 LEDs off
        midi.sendShortMsg(CC,0x23,0x00);  // Pad 4 LEDs off
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 5],0x20, 0xF7),8);   // Blank LCD text
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 6],0x20, 0xF7),8);   // Blank LCD text
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 7],0x20, 0xF7),8);   // Blank LCD text
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 8],0x20, 0xF7),8);   // Blank LCD text
    }
}

// (Dis)connects the appropriate Mixxx control signals to/from functions based on the currently controlled deck
StantonSCS1d.connectTrigSignals = function (channel, disconnect) {
    if (StantonSCS1d.debug) print("     StantonSCS1d: "+(disconnect?"DISCONNECT":"connect")+"TrigSignals:");
    var bank = StantonSCS1d.triggerBank[StantonSCS1d.deck];
    // Set the default bank if not present
    if (!bank)
        bank = StantonSCS1d.triggerBank[StantonSCS1d.deck] = StantonSCS1d.triggerBank["default"];
    
    var signalList = StantonSCS1d.trigSignals[bank];
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
        }
        if (StantonSCS1d.debug) {
            if (disconnect) print("StantonSCS1d: "+group+","+signalList[i][1]+" disconnected from "+signalList[i][2]);
            else print("StantonSCS1d: "+group+","+signalList[i][1]+" connected to "+signalList[i][2]);
        }
    }
    // If disconnecting signals, darken the button LEDs
    if (disconnect) {
        var byte1 = 0x80 + channel;
        for (var i=8; i<=15; i++)
            midi.sendShortMsg(byte1,i,0x00);
        for (var i=18; i<=21; i++)
            midi.sendShortMsg(byte1,i,0x00);
    }
}

// (Dis)connects the mode-independent Mixxx control signals to/from functions based on the currently controlled virtual deck
StantonSCS1d.connectDeckSignals = function (channel, disconnect) {
    if (StantonSCS1d.debug) print("     StantonSCS1d: "+(disconnect?"DISCONNECT":"connect")+"DeckSignals:");
    var signalList = StantonSCS1d.deckSignals;
    for (var i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        var name = signalList[i][1];
        if (group=="CurrentChannel") group = "[Channel"+StantonSCS1d.deck+"]";
        engine.connectControl(group,name,signalList[i][2],disconnect);
        
        // If connecting a signal, update the LEDs
        if (!disconnect) {
            switch (name) {
                case "play":
                    var currentValue = engine.getValue(group,name);
//                    print("StantonSCS1d: current value="+currentValue);
                    StantonSCS1d.playLED(currentValue);
                    break;
                case "cue_default":
                case "beatsync": break;
                default:    // Cause the signal to fire to update LEDs
                    //engine.trigger(group,name);  // No sense in wasting queue length if we can do this another way
                    // Alternate:
                    var command = signalList[i][2]+"("+engine.getValue(group,name)+")";
//                    print("StantonSCS1d: command="+command);
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
        if (StantonSCS1d.modifier["pad"]==1 || StantonSCS1d.modifier["cue"]==1)
            midi.sendShortMsg(0x90+channel,control,127);    // Make it orange
        
        var currentlyPlaying = engine.getValue("[Channel"+StantonSCS1d.deck+"]","play");
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","play", !currentlyPlaying);
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
    engine.setValue("[Channel"+StantonSCS1d.deck+"]","cue_default",0);
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
        //bpm.tapButton(StantonSCS1d.deck);
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","bpm_tap",1);
        return;
    }
    // Button up
    engine.setValue("[Channel"+StantonSCS1d.deck+"]","bpm_tap",0);
}

StantonSCS1d.pfl = function (channel, control, value, status) {
    if ((status & 0xF0) != 0x80) {    // If button down
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","pfl",!engine.getValue("[Channel"+StantonSCS1d.deck+"]","pfl"));
    }
}

StantonSCS1d.rew = function (channel, control, value, status) {
    // If in vinyl mode and button down
    if ((status & 0xF0) == 0x90) {
        if (StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] == "vinyl") {
            midi.sendShortMsg(0xB0+channel,1,'4'.toInt());   // 45 RPM backward
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 35, 1.5, 5, 0, 0, 0xF7]),11);  // Motor full speed
            midi.sendShortMsg(0xB0+channel,1,'o'.toInt());   // Start platter
        }
        engine.setValue("[Channel"+StantonSCS1d.deck+"]","back",1);
        return;
    }
    engine.setValue("[Channel"+StantonSCS1d.deck+"]","back",0);
    engine.trigger("[Channel"+StantonSCS1d.deck+"]","reverse"); // Return to correct direction
    StantonSCS1d.pitchChange(engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate")); // and speed
    engine.trigger("[Channel"+StantonSCS1d.deck+"]","play");    // and playback status
}

StantonSCS1d.ffwd = function (channel, control, value, status) {
    // If in vinyl mode and button down
    if ((status & 0xF0) == 0x90) {
        if (StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] == "vinyl") {
            midi.sendShortMsg(0xB0+channel,1,'2'.toInt());   // 45 RPM foreward
            midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 35, 1.5, 5, 0, 0, 0xF7]),11);  // Motor full speed
            midi.sendShortMsg(0xB0+channel,1,'o'.toInt());   // Start platter
        }
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
        if (StantonSCS1d.crossFader) {
            // Move to cross-fader position
            StantonSCS1d.pitchRangeLEDs(0); // darken range LEDs
            var xfader = engine.getValue("[Master]","crossfader")*63+64;
            if (StantonSCS1d.debug) print ("Moving slider to "+xfader+" for cross-fader");
            midi.sendShortMsg(0xB0+StantonSCS1d.channel,0x00,xfader);
            StantonSCS1d.state["crossfaderAdjusted"]=false;
        }
    }
    else {
        midi.sendShortMsg(0x80+StantonSCS1d.channel,control,0); // Darken button LED
        StantonSCS1d.modifier["pitchRange"]=0; // Clear button modifier flag
        midi.sendShortMsg(0xB0+StantonSCS1d.channel,0x00,engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate")*63+64);    // Move to pitch position
        
        if (!StantonSCS1d.state["crossfaderAdjusted"]) {
            // Change the range
            var currentRange = engine.getValue("[Channel"+StantonSCS1d.deck+"]","rateRange");
            var high = StantonSCS1d.pitchRanges.length;
            if (currentRange<StantonSCS1d.pitchRanges[0] || currentRange>=StantonSCS1d.pitchRanges[high-1])
                engine.setValue("[Channel"+StantonSCS1d.deck+"]","rateRange",StantonSCS1d.pitchRanges[0]);
            else {
                var i=1;
                while (i<high) {
                    if (currentRange>=StantonSCS1d.pitchRanges[i-1] &&
                        currentRange<StantonSCS1d.pitchRanges[i]) {
                        engine.setValue("[Channel"+StantonSCS1d.deck+"]","rateRange",StantonSCS1d.pitchRanges[i]);
                        break;
                    }
                    i++;
                }
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

StantonSCS1d.vinylMoved = function (data, length) {
    // Re-construct the 32-bit word
    var iInfo = (data[0] << 24) | (data[1] << 16) | 
                (data[2] << 8) | data[3];
    
    //// Unpack the data - Firmware v1.24 and lower
    //var iTimeStamp = (iInfo >>> 5);
    //var iQuad = 3 & ((((iInfo >>> 1) & 1) | (iInfo << 1)) ^ (iInfo & 1)); 
    //var iDirection = ((iInfo >>> 2) & 1) ^ 1;
    //iDirection = 1 - (iDirection << 1);
    //
    //if (StantonSCS1d.debug) print("Timestamp="+iTimeStamp+" Quad="+iQuad+" Dir="+iDirection+" Info="+iInfo+" Delta="+(iTimeStamp - StantonSCS1d.scratch["prevTimeStamp"]));
    //
    // // Timestamp range: 131071812 - 130547712 = 524,100
    
    // Unpack the data - Firmware 1.25 and higher
    var iTimeStamp = (iInfo >>> 8);
    var iState = data[3];
    if (isNaN(iState)) return;
    var iMoved = (iState - StantonSCS1d.scratch["prevState"] + 384) % 256 - 128;
    var iSpeed = iMoved/(iTimeStamp - StantonSCS1d.scratch["prevTimeStamp"]);
    
    //if (StantonSCS1d.debug)
    //    print("Timestamp="+iTimeStamp+" State="+iState+" Moved="+iMoved+" Speed="+(iSpeed*60000|0));
    
    StantonSCS1d.scratch["prevTimeStamp"] = iTimeStamp;
    StantonSCS1d.scratch["prevState"] = iState;
    
    // Process the data
    
    var platterMode = StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"];

    switch(platterMode) {
        case "control":
            break;
        case "browse":
            if (StantonSCS1d.state["browseTicks"]==StantonSCS1d.browseDamp) {
                StantonSCS1d.state["browseTicks"]=0;
                if (iMoved>0) engine.setValue("[Playlist]","SelectNextTrack",1);
                else if (iMoved<0) engine.setValue("[Playlist]","SelectPrevTrack",1);
            }
            else StantonSCS1d.state["browseTicks"]++;
            break;
        case "vinyl":    // Scratching
            // Ignore if the music speed is outside the motor abilities and the platter is stopped
            if (StantonSCS1d.state["outsideMotor"]) return;
            engine.scratchTick(StantonSCS1d.deck,iMoved);
            break;
    }
}

StantonSCS1d.deckChangeFlash = function (channel, value) {
    var byte1 = 0x90 + channel;
    var color=32;   // Green
    
    if (StantonSCS1d.deck % 2 == 0) color=64; // Deck select button red
    
    StantonSCS1d.state["flashes"]++;
    
    if (StantonSCS1d.state["flashes"] % 2 == 0) {
        midi.sendShortMsg(byte1,64,color); // Deck select button on
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 1, 0xF7]),8); // Jog backlight on
    }
    else {
        midi.sendShortMsg(0x80 + channel,64,0); // Deck select button off
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 0, 0xF7]),8); // Jog backlight off
    }
    
    if (StantonSCS1d.state["flashes"]>=7) {
        engine.stopTimer(StantonSCS1d.timer["deckChange"]);
        StantonSCS1d.timer["deckChange"] = -1;
        
        // Finish the deck change
        StantonSCS1d.DeckChangeFinish(channel);
        StantonSCS1d.DeckChangeP2(channel, value);  // Call part 2
    }
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

    // If the button's been held down for over a second, stay on the current deck
    if (new Date() - StantonSCS1d.modifier["deckTime"]>StantonSCS1d.deckChangeWait) {
        //StantonSCS1d.connectKnobSignals(channel);   // Re-connect (restored) knob signals
        // Return to appropriate color
        if (StantonSCS1d.deck % 2 == 0) midi.sendShortMsg(byte1,control,64); // Deck select button red
        else midi.sendShortMsg(byte1,control,32); // Deck select button green
    }
    else {
        engine.scratchDisable(StantonSCS1d.deck);    // To avoid accidentally stopping the outgoing deck
        StantonSCS1d.loopEnabled(0);
        StantonSCS1d.connectDeckSignals(channel,true);    // Disconnect static signals
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 17, 0xF7]),7); // Extinguish all LEDs
        StantonSCS1d.padRefresh();  // Re-light pad section. It's not affected by deck changes.
        midi.sendShortMsg(0xB0 + channel,2,0x00);    // Clear jog circle
        if (StantonSCS1d.globalMode)
            StantonSCS1d.newPlatterMode = StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"];
        
        // Supports n-decks
        if (StantonSCS1d.deck == engine.getValue("[Master]","num_decks")) StantonSCS1d.deck=1;
        else StantonSCS1d.deck++;
        
        if (StantonSCS1d.debug) print("StantonSCS1d: Switching to deck "+StantonSCS1d.deck);
        midi.sendShortMsg(0x80 + channel,control,0); // Deck select button off
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 0, 0xF7]),8); // Jog backlight off
        // Blank jog character
        //midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, 0x20, 0xF7]),8);
        // Deck number jog character
        var deckNr = StantonSCS1d.deck+"";
        deckNr = deckNr.toInt();
        if (StantonSCS1d.deck>9) deckNr = 64+(StantonSCS1d.deck-9);
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 36, deckNr, 0xF7]),8);
        
        if (StantonSCS1d.globalMode) StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"] = StantonSCS1d.state["Oldknob"];
        else {
            // Set the platter mode to default if not present
            if (!StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"])
                StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = StantonSCS1d.platterMode["default"];
            StantonSCS1d.newPlatterMode = StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"];
        }
        
        StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] = ""; // Force mode change
        
        // 1st half
        switch(StantonSCS1d.newPlatterMode) {
            case "control": 
                StantonSCS1d.controlButton(channel, StantonSCS1d.buttons["control"], value, 0x90 + channel);
                break;
            case "browse": 
                StantonSCS1d.browseButton(channel, StantonSCS1d.buttons["browse"], value, 0x90 + channel);
                break;
            case "vinyl": 
                StantonSCS1d.vinylButton(channel, StantonSCS1d.buttons["vinyl"], value, 0x90 + channel);
                break;
        }
        
        // Make flashy lights to signal a deck change
        if (!StantonSCS1d.fastDeckChange) { 
            if (StantonSCS1d.timer["deckChange"] != -1) engine.stopTimer(StantonSCS1d.timer["deckChange"]);
            StantonSCS1d.state["flashes"] = 0;  // initialize number of flashes
            StantonSCS1d.timer["deckChange"] = engine.beginTimer(60,"StantonSCS1d.deckChangeFlash("+channel+","+value+")");
            return;
        }
        // No flashy lights
        StantonSCS1d.DeckChangeFinish(channel);
    }
    // Do this even if the deck has not changed
    StantonSCS1d.DeckChangeP2(channel, value); // Go to part 2.
}

StantonSCS1d.DeckChangeFinish = function(channel) {
    var byte1 = 0x90 + channel;
    // Finish the deck change
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([channel, 13, 1, 0xF7]),8); // Jog backlight on
    if (StantonSCS1d.deck % 2 == 0) midi.sendShortMsg(byte1,64,64); // Deck select button red
    else midi.sendShortMsg(byte1,64,32); // Deck select button green
    StantonSCS1d.connectDeckSignals(channel);    // Connect static signals
    // Update jog circle
    StantonSCS1d.durationChange(engine.getValue("[Channel"+StantonSCS1d.deck+"]","duration"));
    StantonSCS1d.lastLight[StantonSCS1d.deck]=-1;
    StantonSCS1d.circleBars(engine.getValue("[Channel"+StantonSCS1d.deck+"]","visual_playposition"));
}

StantonSCS1d.DeckChangeP2 = function (channel, value) {
    
    
    StantonSCS1d.connectKnobSignals(channel);   // Connect new knob signals & light LEDs & displays
    StantonSCS1d.encoderBank(channel, 4, 0, 0x80);  // Light the bank button the correct color for the mode
    
    // 2nd half
    switch(StantonSCS1d.newPlatterMode) {
        case "control": 
            StantonSCS1d.controlButton(channel, StantonSCS1d.buttons["control"], value, 0x80 + channel);
            break;
        case "browse": 
            StantonSCS1d.browseButton(channel, StantonSCS1d.buttons["browse"], value, 0x80 + channel);
            break;
        case "vinyl": 
            StantonSCS1d.vinylButton(channel, StantonSCS1d.buttons["vinyl"], value, 0x80 + channel);
            break;
    }
    
    switch (StantonSCS1d.triggerBank[StantonSCS1d.deck]) {
        case 2: 
            StantonSCS1d.triggerBankSelect(channel, 23, 1, 0x90);
            StantonSCS1d.triggerBankSelect(channel, 23, 1, 0x80);
            break;
        case 3: 
            StantonSCS1d.triggerBankSelect(channel, 5, 1, 0x90);
            StantonSCS1d.triggerBankSelect(channel, 5, 1, 0x80);
            break;
        case 1:
        default:
            StantonSCS1d.triggerBank[StantonSCS1d.deck]=-1; // Force LEDs to update
            StantonSCS1d.triggerBankSelect(channel, 22, 1, 0x90);
            StantonSCS1d.triggerBankSelect(channel, 22, 1, 0x80);
            break;
    }
    
}   // End Deck Change function

// ----------   Encoders  ----------

StantonSCS1d.encoderBank = function (channel, control, value, status) {
    if (StantonSCS1d.checkInSetup() || StantonSCS1d.modifier["DeckSelect"]==1) return;
    
    var newMode = StantonSCS1d.knobMode["[Channel"+StantonSCS1d.deck+"]"];
    
    if ((status & 0xF0) == 0x80) {  // If button up
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
    
    if (newMode==2) {
        // Use button 1 to toggle the flanger effect if in mode 2 (which adjusts FX params)
        engine.connectControl("[Channel"+StantonSCS1d.deck+"]","flanger","StantonSCS1d.knobB1LED");
        engine.trigger("[Channel"+StantonSCS1d.deck+"]","flanger");
        // Use button 4 to toggle key lock if in mode 2 (which adjusts FX params)
        engine.connectControl("[Channel"+StantonSCS1d.deck+"]","keylock","StantonSCS1d.knobB4LED");
        engine.trigger("[Channel"+StantonSCS1d.deck+"]","keylock");
    }
    else {
        engine.connectControl("[Channel"+StantonSCS1d.deck+"]","flanger","StantonSCS1d.knobB1LED",true);
        midi.sendShortMsg(0x80+channel,25,0);    // Turn button off
        engine.connectControl("[Channel"+StantonSCS1d.deck+"]","keylock","StantonSCS1d.knobB4LED",true);
        midi.sendShortMsg(0x80+channel,17,0);    // Turn button off
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
                    if (StantonSCS1d.mutex[control]) break;
                    StantonSCS1d.encoderSetAbs(knobMode,1,0,true);
                    StantonSCS1d.mutex[control]=true;
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,1,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    StantonSCS1d.mutex[control]=false;
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
                    if (StantonSCS1d.mutex[control]) break;
                    StantonSCS1d.encoderSetAbs(knobMode,1,1,true);
                    StantonSCS1d.mutex[control]=true;
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,1,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    StantonSCS1d.mutex[control]=false;
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
                    if (StantonSCS1d.mutex[control]) break;
                    StantonSCS1d.encoderSetAbs(knobMode,2,0,true);
                    StantonSCS1d.mutex[control]=true;
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,2,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    StantonSCS1d.mutex[control]=false;
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
                    if (StantonSCS1d.mutex[control]) break;
                    StantonSCS1d.encoderSetAbs(knobMode,3,0,true);
                    StantonSCS1d.mutex[control]=true;
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,3,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    StantonSCS1d.mutex[control]=false;
                    break;
            }
            break;
        case 2: // Flanger period
            break;
        case 3: // Master Balance
            switch (status & 0xF0) {
                case 0x90:  // Full right
                    midi.sendShortMsg(0x90+channel,control,1);
                    if (StantonSCS1d.mutex[control]) break;
                    StantonSCS1d.encoderSetAbs(knobMode,3,1);
                    StantonSCS1d.mutex[control]=true;
                    break;
                case 0x80:  // Full left
                    StantonSCS1d.encoderSetAbs(knobMode,3,-1);
                    midi.sendShortMsg(0x90+channel,control,0);
                    StantonSCS1d.mutex[control]=false;
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
                    if (StantonSCS1d.mutex[control]) break;
                    StantonSCS1d.encoderSetAbs(knobMode,4,0,true);
                    StantonSCS1d.mutex[control]=true;
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,4,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    StantonSCS1d.mutex[control]=false;
                    break;
            }
            break;
        case 2: // Toggle key lock
            switch (status & 0xF0) {
                case 0x90:
                    engine.setValue("[Channel"+StantonSCS1d.deck+"]","keylock",!engine.getValue("[Channel"+StantonSCS1d.deck+"]","keylock"));
                    break;
            }
            break;
        case 3: // Master volume momentary kill
            switch (status & 0xF0) {
                case 0x90:
                    midi.sendShortMsg(0x90+channel,control,1);
                    if (StantonSCS1d.mutex[control]) break;
                    StantonSCS1d.encoderSetAbs(knobMode,4,0,true);
                    StantonSCS1d.mutex[control]=true;
                    break;
                case 0x80:
                    StantonSCS1d.encoderSetAbs(knobMode,4,"previous");
                    midi.sendShortMsg(0x90+channel,control,0);
                    StantonSCS1d.mutex[control]=false;
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
    if (StantonSCS1d.crossFader && StantonSCS1d.modifier["pitchRange"]==1) {
        engine.setValue("[Master]","crossfader",newValue);
        StantonSCS1d.state["crossfaderAdjusted"]=true;
    }
    else engine.setValue("[Channel"+StantonSCS1d.deck+"]","rate",newValue);
}

// ----------   Trigger buttons  ----------

StantonSCS1d.triggerBankSelect = function (channel, control, value, status) {
    if ((status & 0xF0) == 0x90) {    // If button down
        var byte1 = 0x80 + channel;
        midi.sendShortMsg(byte1,22,0x00); // Turn off bank button lights
        midi.sendShortMsg(byte1,23,0x00);
        midi.sendShortMsg(byte1,5,0x00);
        
        var lastBank = StantonSCS1d.triggerBank[StantonSCS1d.deck];
        switch (control) {
            case 22: StantonSCS1d.triggerBank[StantonSCS1d.deck]=1; break;
            case 23: StantonSCS1d.triggerBank[StantonSCS1d.deck]=2; break;
            case 5: StantonSCS1d.triggerBank[StantonSCS1d.deck]=3; break;
        }
        
        byte1 = 0x90 + channel;
        midi.sendShortMsg(byte1,control,0x01); // Turn on active bank button light
        StantonSCS1d.modifier["trigger"] = StantonSCS1d.triggerBank[StantonSCS1d.deck];
        
        // If we've changed banks and the new one is #1, connect the loop signals
        if (lastBank != StantonSCS1d.triggerBank[StantonSCS1d.deck] 
            && StantonSCS1d.triggerBank[StantonSCS1d.deck]==1) {
            StantonSCS1d.connectTrigSignals(channel,false);
            StantonSCS1d.loopEnabled(1);
        }
        // If we've changed banks and the new one is other than #1,
        //  disconnect the loop signals & set the LEDs
        if (lastBank != StantonSCS1d.triggerBank[StantonSCS1d.deck] 
            && StantonSCS1d.triggerBank[StantonSCS1d.deck]!=1) {  // Pitch points
            StantonSCS1d.connectTrigSignals(channel,true);
            StantonSCS1d.loopEnabled(0);
            // Light all the buttons since they're all pre-set
            for (var i=8; i<=15; i++)
                midi.sendShortMsg(byte1,i,0x01); // Turn on button light
            for (var i=18; i<=21; i++)
                midi.sendShortMsg(byte1,i,0x01); // Turn on button light
        }
        
        return;
    }
    
    // If button up
    StantonSCS1d.modifier["trigger"] = -1;
}

StantonSCS1d.triggerButton = function (channel, control, value, status) {
    
    var byte1 = 0x90 + channel;
    var deck = StantonSCS1d.deck;
    
    var index = StantonSCS1d.triggerBank[deck];
    
    if ((status & 0xF0) == 0x90) {    // If button down
        
        if (index == 1) { // bank 1 = Looping
            var loopNumber = StantonSCS1d.trigButtonsLoopNr[control];   // For when we get multiple loops
            loopNumber = 1; // REMOVE ME for multiple loops
            var prevLoopNumber = StantonSCS1d.modifier["loop"+loopNumber];
            StantonSCS1d.modifier["loop"+loopNumber] = 1;
            // If the current bank select button is held down 
            //  when this trigger button is pressed, erase the loop marker for this button
            if (!StantonSCS1d.looseLoops && StantonSCS1d.modifier["trigger"]==index) {
                if (control % 2 == 0) engine.setValue("[Channel"+deck+"]","loop_start_position",-1);
                else engine.setValue("[Channel"+deck+"]","loop_end_position",-1);
            }
            else if ((!StantonSCS1d.looseLoops && prevLoopNumber == loopNumber)
                        || (StantonSCS1d.looseLoops && StantonSCS1d.modifier["trigger"]==index)) {
                // If another button is held down corresponding to the same loop as this one,
                //  and the in and out points are set, toggle the loop
                // Unless in looseLoops mode, where the bank button + a loop button will toggle theloop
                if (engine.getValue("[Channel"+deck+"]","loop_start_position") != -1
                    && engine.getValue("[Channel"+deck+"]","loop_end_position") != -1) {
                    engine.setValue("[Channel"+deck+"]","reloop_exit",1);
                    engine.setValue("[Channel"+deck+"]","reloop_exit",0);
                }
            }
            else {
                // Set loop marker
                // If not in looseLoops mode, only set if it isn't currently
                // If in looseLoops mode, set it regardless
                if (control % 2 == 0) {
                    if (!StantonSCS1d.looseLoops 
                        && engine.getValue("[Channel"+deck+"]","loop_start_position") != -1)
                        return;
                    engine.setValue("[Channel"+deck+"]","loop_in",1);
                }
                if (control % 2 != 0) {
                    if (!StantonSCS1d.looseLoops 
                        && engine.getValue("[Channel"+deck+"]","loop_end_position") != -1)
                        return;
                    engine.setValue("[Channel"+deck+"]","loop_out",1);
                }
            }
        }
        else {
            midi.sendShortMsg(byte1,control,0x01); // Turn off activated button LED

            // Multiple pitch points
            
            //if (StantonSCS1d.modifier[currentMode]==1) {    // Delete a pitch point if the mode button is held
            //    StantonSCS1d.pitchPoints[index][button] = -0.1;
            //    break;
            //}
            if (StantonSCS1d.pitchPoints[index][control] == -0.1)
                StantonSCS1d.pitchPoints[index][control] = engine.getValue("[Channel"+deck+"]","rate");
            else {
                // Need 100% range for values to be correct
                engine.setValue("[Channel"+deck+"]","rateRange",1.0);
                engine.setValue("[Channel"+deck+"]","rate",StantonSCS1d.pitchPoints[index][control]); 
            }
        }
        return;
    }
    
    // If button up
    
    if (index == 1) { // bank 1 = Looping
        var loopNumber = StantonSCS1d.trigButtonsLoopNr[control];
        loopNumber = 1; // REMOVE ME for multiple loops
        StantonSCS1d.modifier["loop"+loopNumber] = -1;
        if (StantonSCS1d.looseLoops || (!StantonSCS1d.looseLoops && StantonSCS1d.modifier["trigger"]!=index)) {
            if (control % 2 == 0) engine.setValue("[Channel"+deck+"]","loop_in",0);
            else engine.setValue("[Channel"+deck+"]","loop_out",0);
        }
    }
    else {
        midi.sendShortMsg(byte1,control,0x01); // Turn on activated button LED
    }
}

// ----------   Other buttons  ----------

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
    
    // Darken both pad bank buttons
    StantonSCS1d.padBankButton(StantonSCS1d.channel, 0x35, 0, 0x80);
    StantonSCS1d.padBankButton(StantonSCS1d.channel, 0x36, 0, 0x80);
    
    //StantonSCS1d.connectPadSignals(StantonSCS1d.channel,true);
    StantonSCS1d.connectPadSignals(StantonSCS1d.channel);
}

StantonSCS1d.pad = function (channel, control, value, status) {
    var deck = StantonSCS1d.padBank["deck"];
    var bank = StantonSCS1d.padBank["bank"+deck];
    var hotCue = StantonSCS1d.hotCues[bank][control];
    var byte1 = 0x90 + channel;

    if ((status & 0xF0) == 0x90) {    // If button down
        StantonSCS1d.modifier["pad"]=1;

        // Multiple cue points
        // Mixxx has 31 hot cues. Skip any above that.
        
        if (hotCue == -1) return;

        if (StantonSCS1d.modifier["velocityToggle"]==1) { // Delete a cue point
            engine.setValue("[Channel"+deck+"]","hotcue_"+hotCue+"_clear",1);
            engine.setValue("[Channel"+deck+"]","hotcue_"+hotCue+"_clear",0);
            StantonSCS1d.state["padCleared"]=true;
            return;
        }
        
        if (StantonSCS1d.padVelocity && engine.getValue("[Channel"+deck+"]","hotcue_"+hotCue+"_enabled"))
            engine.setValue("[Channel"+deck+"]","volume",(value/0x7A));   // 0x7A seems the highest # the unit sends
            
        // If hotcue X is set, seeks the player to hotcue X's position.
        // If hotcue X is not set, sets hotcue X to the current play position.
        engine.setValue("[Channel"+deck+"]","hotcue_"+hotCue+"_activate",1);
        return;
    }
    
    // If button up
    engine.setValue("[Channel"+deck+"]","hotcue_"+hotCue+"_activate",0);
    StantonSCS1d.modifier["pad"]=0;
}


StantonSCS1d.padTop1 = function (channel, control, value, status) {
    // Future use
}

StantonSCS1d.padTop2 = function (channel, control, value, status) {

}

StantonSCS1d.padTop3 = function (channel, control, value, status) {

}

StantonSCS1d.padTop4 = function (channel, control, value, status) {

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
        
        StantonSCS1d.connectPadSignals(channel,true);    // Disconnect existing signals
    
        if (StantonSCS1d.padBank["deck"]!=deck) {
            StantonSCS1d.padBank["deck"]=deck;
            if (deck==1) midi.sendShortMsg(0x80 + channel,0x36,0);   // turn off deck 2 button
            else midi.sendShortMsg(0x80 + channel,0x35,0);  // turn off deck 1 button
        }
        else {
            if (StantonSCS1d.padBank["bank"+deck]==3) StantonSCS1d.padBank["bank"+deck]=1;
            else StantonSCS1d.padBank["bank"+deck]++;
            
            if (StantonSCS1d.debug) print("PadBank="+StantonSCS1d.padBank["bank"+deck]);
        }
        StantonSCS1d.padRefresh();    // Signals connected here
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

StantonSCS1d.pitchChange = function (value,noScratchEnable) {
    var CC = 0xB0 + StantonSCS1d.channel;
    var group = "[Channel"+StantonSCS1d.deck+"]";
    
    if (StantonSCS1d.crossFader && StantonSCS1d.modifier["pitchRange"]==1) return; // Skip if adjusting the cross-fader
    if (value < -1 || value > 1) return;  // FIXME: This sometimes happens after using the BPM button to set the tempo and changing the pitch range. We should find out why.
    var now=new Date();
    // Move slider if applicable
    if (now - StantonSCS1d.state["dontMove"]>100) {
        if (StantonSCS1d.debug) print ("Moving slider to "+(value*63+64)+" for pitch");
        midi.sendShortMsg(0xB0+StantonSCS1d.channel,0x00,value*63+64);
        }
    
    // Skip motor speed change if fast-forwarding or rewinding
    if (engine.getValue(group,"fwd")>0 || engine.getValue(group,"back")>0) return;
    
    if (engine.getValue(group,"rate_dir") == -1) value = -value; // Take slider direction into account
    var multiplier = value * engine.getValue(group,"rateRange");
    var iMotorPitch = 1000+(1000*multiplier);
    if (iMotorPitch < 500 || iMotorPitch > 2000) {
        if (engine.getValue(group,"play")>0 && !StantonSCS1d.state["outsideMotor"]) {
            if (StantonSCS1d.debug) print ("Stopping platter motor: music speed is outside its abilities");
            engine.scratchDisable(StantonSCS1d.deck);    // Disable direct platter control
            midi.sendShortMsg(CC,1,'x'.toInt());    // Stop platter
        }
        StantonSCS1d.state["outsideMotor"]=true;
        return;
    } else {    // Start the platter since the music speed is within the ability of the motor
        StantonSCS1d.state["outsideMotor"]=false;
        if (StantonSCS1d.platterMode[group] == "vinyl" && engine.getValue(group,"play")>0) {
                midi.sendShortMsg(CC,1,'o'.toInt());
                // Re-enable direct platter control
                if (noScratchEnable != true)
                    engine.scratchEnable(StantonSCS1d.deck, StantonSCS1d.scratch["resolution"],
                        StantonSCS1d.rpm[StantonSCS1d.platterSpeed],StantonSCS1d.scratch["alpha"],
                        StantonSCS1d.scratch["beta"]);
        }
    }
    if (iMotorPitch < 500) iMotorPitch = 500;    // Or stop
    if (iMotorPitch > 2000) iMotorPitch = 2000;    // Or stop
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
    if (StantonSCS1d.platterMode["[Channel"+StantonSCS1d.deck+"]"] == "vinyl") {
        if (value==0) midi.sendShortMsg(CC,1,'x'.toInt());   // Stop platter
        else if (!StantonSCS1d.state["outsideMotor"]) midi.sendShortMsg(CC,1,'o'.toInt());   // Start platter
    }
    StantonSCS1d.buttonLED(value, 41, 32, 0);    //green
}

StantonSCS1d.cueLED = function (value) {
    StantonSCS1d.buttonLED(value, 43, 127, 0);  //orange
}

StantonSCS1d.syncLED = function (value) {
    StantonSCS1d.buttonLED(value, 42, 64, 0);    //red
}

StantonSCS1d.bpmLED = function (value) {
    StantonSCS1d.buttonLED(value, 40, 64, 0);
}

StantonSCS1d.backLED = function (value) {
    StantonSCS1d.buttonLED(value, 47, 0x7f, 0);
}

StantonSCS1d.fwdLED = function (value) {
    StantonSCS1d.buttonLED(value, 46, 0x7f, 0);
}

StantonSCS1d.headphoneLED = function (value) {
    StantonSCS1d.buttonLED(value, 55, 0x7f, 32);
}

StantonSCS1d.knobB1LED = function (value) {
    StantonSCS1d.buttonLED(value, 25, 0x7f, 0);
}

StantonSCS1d.knobB4LED = function (value) {
    StantonSCS1d.buttonLED(value, 17, 0x7f, 0);
}

// Triggers/Loops
StantonSCS1d.activateLoop = function (value) {
    // In the future for multiple loops, add a loopNum parameter and store that value below.
    if (value>0) 
        if (engine.getValue("[Channel"+StantonSCS1d.deck+"]","loop_enabled"))
            StantonSCS1d.loopActive[StantonSCS1d.deck]=1;
        else StantonSCS1d.loopActive[StantonSCS1d.deck]=-1;
}

StantonSCS1d.loopEnabled = function (value) {
    if (value>0 && StantonSCS1d.timer["loop"]==-1) {
        if (!StantonSCS1d.loopActive[StantonSCS1d.deck] || 
            StantonSCS1d.loopActive[StantonSCS1d.deck]==-1) StantonSCS1d.activateLoop(1);
        StantonSCS1d.timer["loop"] = engine.beginTimer(500,"StantonSCS1d.loopFlash()");
    }
    
    if (value<=0 && StantonSCS1d.timer["loop"]!=-1) {
        engine.stopTimer(StantonSCS1d.timer["loop"]);
        StantonSCS1d.timer["loop"]=-1;
        StantonSCS1d.connectTrigSignals(StantonSCS1d.channel);
    }
}

StantonSCS1d.loopFlash = function (end) {
    var buttons = [0,8,10,12,14,18,20];
    var target;
    
    var activeLoop = StantonSCS1d.loopActive[StantonSCS1d.deck];
    if (!activeLoop || activeLoop==-1) return;
    target = buttons[activeLoop];
    if (StantonSCS1d.state["loopFlash"] < 1) {
        StantonSCS1d.state["loopFlash"]++;
        for (var i=1; i<=6; i++) {  // REMOVE for multiple loops
            target = buttons[i];     // REMOVE for multiple loops
            StantonSCS1d.buttonLED(1, target, 0x7f, 0);
            StantonSCS1d.buttonLED(1, target+1, 0x7f, 0);
        }   // REMOVE for multiple loops
    }
    else {
        StantonSCS1d.state["loopFlash"]=0;
        for (var i=1; i<=6; i++) {  // REMOVE for multiple loops
            target = buttons[i];     // REMOVE for multiple loops
            StantonSCS1d.buttonLED(0, target, 0x7f, 0);
            StantonSCS1d.buttonLED(0, target+1, 0x7f, 0);
        }   // REMOVE for multiple loops
    }
    
}

StantonSCS1d.trigLED = function (value,control) {
    if (value == -1) StantonSCS1d.buttonLED(0, control, 0x7f, 0);
    else StantonSCS1d.buttonLED(1, control, 0x7f, 0);
}

StantonSCS1d.Trig1ILED = function (value) {    StantonSCS1d.trigLED(value, 8); }
StantonSCS1d.Trig1OLED = function (value) {    StantonSCS1d.trigLED(value, 9); }
StantonSCS1d.Trig2ILED = function (value) {    StantonSCS1d.trigLED(value, 10);}
StantonSCS1d.Trig2OLED = function (value) {    StantonSCS1d.trigLED(value, 11);}
StantonSCS1d.Trig3ILED = function (value) {    StantonSCS1d.trigLED(value, 12);}
StantonSCS1d.Trig3OLED = function (value) {    StantonSCS1d.trigLED(value, 13);}
StantonSCS1d.Trig4ILED = function (value) {    StantonSCS1d.trigLED(value, 14);}
StantonSCS1d.Trig4OLED = function (value) {    StantonSCS1d.trigLED(value, 15);}
StantonSCS1d.Trig5ILED = function (value) {    StantonSCS1d.trigLED(value, 18);}
StantonSCS1d.Trig5OLED = function (value) {    StantonSCS1d.trigLED(value, 19);}
StantonSCS1d.Trig6ILED = function (value) {    StantonSCS1d.trigLED(value, 20);}
StantonSCS1d.Trig6OLED = function (value) {    StantonSCS1d.trigLED(value, 21);}

// ---- Hot cues ----

// Pad LCDs

StantonSCS1d.PadDisplay = function (value, pad) {
    // value = hotcue position
    var deck = StantonSCS1d.padBank["deck"];
    var bank = StantonSCS1d.padBank["bank"+deck];
    var hotCue = StantonSCS1d.hotCues[bank][pad];
    
    var message= "Hotcue"+(hotCue < 10 ? " " + hotCue : hotCue);    // If empty, just display hot cue #
    
    if (value!=-1) {
        // Set display with cue point time
        var samplerate = engine.getValue("[Channel"+deck+"]","track_samplerate");
        var msecs = ((value/2/samplerate)*1000) | 0;    // OR with 0 replaces Math.floor and is faster
        
        // Track time in milliseconds
        message = msecondstominutes(msecs);
    }
    else {
        // Light pad LED on hot cue erase
        midi.sendShortMsg(0x90 + StantonSCS1d.channel,pad,127);
    }
    
    // Display value
    midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, pad-26],message.toInt(), 0xF7),7+message.length);
}

StantonSCS1d.Pad1LCD = function (value) { StantonSCS1d.PadDisplay(value,0x20); }
StantonSCS1d.Pad2LCD = function (value) { StantonSCS1d.PadDisplay(value,0x21); }
StantonSCS1d.Pad3LCD = function (value) { StantonSCS1d.PadDisplay(value,0x22); }
StantonSCS1d.Pad4LCD = function (value) { StantonSCS1d.PadDisplay(value,0x23); }

// Pad LEDs

StantonSCS1d.PadLED = function (value, pad) {
    var deck = StantonSCS1d.padBank["deck"];
    var bank = StantonSCS1d.padBank["bank"+deck];
    var hotCue = StantonSCS1d.hotCues[bank][pad];
    
    var color = 0;
    
    if (value != 0) {    // Activated
        color = 127;
    }
    else {    // Deactivated
        if (engine.getValue("[Channel"+deck+"]","hotcue_"+hotCue+"_enabled")==1) color = 32;    // Hot cue set (green)
    }
    
    midi.sendShortMsg(0x90 + StantonSCS1d.channel,pad,color);
}

// Hot cue activated LEDs
StantonSCS1d.Pad1aLED = function (value) { StantonSCS1d.PadLED(value,0x20); }
StantonSCS1d.Pad2aLED = function (value) { StantonSCS1d.PadLED(value,0x21); }
StantonSCS1d.Pad3aLED = function (value) { StantonSCS1d.PadLED(value,0x22); }
StantonSCS1d.Pad4aLED = function (value) { StantonSCS1d.PadLED(value,0x23); }

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

StantonSCS1d.pitchRangeLEDs = function (value) {
    StantonSCS1d.pitchChange(engine.getValue("[Channel"+StantonSCS1d.deck+"]","rate")); // So the platter speed is updated
    var on = 0x90 + StantonSCS1d.channel;
    var off = 0x80 + StantonSCS1d.channel;
    switch (true) {
        case (value<=0.07):
                midi.sendShortMsg(off,65,0);   // 8%
                midi.sendShortMsg(off,66,0);   // 16%
                midi.sendShortMsg(off,67,0);   // 25%
                midi.sendShortMsg(off,68,0);   // 50%
            break;
        case (value>0.07 && value<=0.08):
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

StantonSCS1d.durationChange = function (value) {
    // Stop any leftover end-of-track-flash timers
    var deck = StantonSCS1d.deck;
    if (StantonSCS1d.timer["15s-d"+deck] != -1) {
        engine.stopTimer(StantonSCS1d.timer["15s-d"+deck]);
        StantonSCS1d.timer["15s-d"+deck] = -1;
    }
    if (StantonSCS1d.timer["30s-d"+deck] != -1) {
        engine.stopTimer(StantonSCS1d.timer["30s-d"+deck]);
        StantonSCS1d.timer["30s-d"+deck] = -1;
    }
    // Make sure jog is lit
    if (StantonSCS1d.state["jogFlash"]!=false) {
        StantonSCS1d.state["jogFlash"]=false;
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 13, 1, 0xF7]),8); // Jog backlight on
    }
    
    StantonSCS1d.trackDuration[deck]=value;
    StantonSCS1d.padRefresh();    // Update hot cues
}

StantonSCS1d.circleFlash = function (deck) {
    if (StantonSCS1d.deck != deck) return;  // Only do this for the current deck
    if (!StantonSCS1d.state["jogFlash"]) {
        StantonSCS1d.state["jogFlash"]=true;
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 13, 0, 0xF7]),8); // Jog backlight off
    }
    else {
        StantonSCS1d.state["jogFlash"]=false;
        midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 13, 1, 0xF7]),8); // Jog backlight on
    }
}

StantonSCS1d.circleBars = function (value) {
    var deck = StantonSCS1d.deck;

    // Flash the jog back light near the end of the track if the track is longer than 30s
    if (StantonSCS1d.trackDuration[deck]>30) {
        var trackTimeRemaining = ((1-value) * StantonSCS1d.trackDuration[deck]) | 0;
        if (trackTimeRemaining<=30 && trackTimeRemaining>15) {   // If <30s left, flash slowly
            if (StantonSCS1d.timer["30s-d"+deck] == -1) {
                // Start timer
                StantonSCS1d.timer["30s-d"+deck] = engine.beginTimer(500,"StantonSCS1d.circleFlash("+deck+")");
                if (StantonSCS1d.timer["15s-d"+deck] != -1) {
                    // Stop the 15s timer if it was running
                    engine.stopTimer(StantonSCS1d.timer["15s-d"+deck]);
                    StantonSCS1d.timer["15s-d"+deck] = -1;
                }
            }
        } else if (trackTimeRemaining<=15 && trackTimeRemaining>0) { // If <15s left, flash quickly
            if (StantonSCS1d.timer["15s-d"+deck] == -1) {
                // Start timer
                StantonSCS1d.timer["15s-d"+deck] = engine.beginTimer(125,"StantonSCS1d.circleFlash("+deck+")");
                if (StantonSCS1d.timer["30s-d"+deck] != -1) {
                    // Stop the 30s timer if it was running
                    engine.stopTimer(StantonSCS1d.timer["30s-d"+deck]);
                    StantonSCS1d.timer["30s-d"+deck] = -1;
                }
            }
        } else {    // Stop flashing
            if (StantonSCS1d.timer["15s-d"+deck] != -1) {
                engine.stopTimer(StantonSCS1d.timer["15s-d"+deck]);
                StantonSCS1d.timer["15s-d"+deck] = -1;
            }
            if (StantonSCS1d.timer["30s-d"+deck] != -1) {
                engine.stopTimer(StantonSCS1d.timer["30s-d"+deck]);
                StantonSCS1d.timer["30s-d"+deck] = -1;
            }
            // Make sure jog is lit
            if (StantonSCS1d.state["jogFlash"]!=false) {
                StantonSCS1d.state["jogFlash"]=false;
                midi.sendSysexMsg(StantonSCS1d.sysex.concat([StantonSCS1d.channel, 13, 1, 0xF7]),8); // Jog backlight on
            }
        }
    }
    
    // Revolution time of the imaginary record in seconds
//     var revtime = StantonSCS1d.scratch["revtime"]/2;    // Use this for two bars
    var revtime = StantonSCS1d.scratch["revtime"];
    var currentTrackPos = value * StantonSCS1d.trackDuration[deck];
    
    var revolutions = currentTrackPos/revtime;
//     var light = ((revolutions-(revolutions|0))*18)|0;    // Use this for two bars
    var light = ((revolutions-(revolutions|0))*36)|0;   // OR with 0 replaces Math.floor and is faster

    if (!StantonSCS1d.lastLight[deck]) {
        // Set default value
        StantonSCS1d.lastLight[StantonSCS1d.deck]=-1;
        return;
    }
    
    if (StantonSCS1d.lastLight[deck]==light) return;   // Don't send light commands if there's no visible change
    
    var byte1 = 0xB0 + StantonSCS1d.channel;
    //midi.sendShortMsg(byte1,2,0);     // Clear circle markers
    StantonSCS1d.lastLight[deck]=light;
    midi.sendShortMsg(byte1,2,light+1);
//     midi.sendShortMsg(byte1,2,18+light);   // Add this for two bars
}

/*
TODO:
- Motor calibration option (would be really nice to have GUI interaction for this)
- Wait for motor to stop in-between mode/deck changes
- Wait for motor to get to speed before latching on when pressing CUE or PLAY (soft-takeover! :) )
- Stop motor on FF/REW? If not, FF/REW only at motor speed?
- Connect cross-fader signal?

BUGS:
- Dragging the window/tooltips screw up speed - use timestamps
- Sticker drift - timestamps?
- Manipulating other controls in vinyl mode makes speed jiggly - timestamps??
*/
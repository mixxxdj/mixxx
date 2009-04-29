/****************************************************************/
/*      Stanton SCS.1m MIDI controller script vPre              */
/*          Copyright (C) 2009, Sean M. Pappalardo              */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.7.0                                 */
/****************************************************************/

function StantonSCS1m() {}

// ----------   Customization variables ----------
//      See http://mixxx.org/wiki/doku.php/stanton_SCS.1m_mixxx_user_guide  for details
StantonSCS1m.faderStart = true;

// ----------   Other global variables    ----------
StantonSCS1m.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
StantonSCS1m.channel = 0;   // MIDI channel the device is on
StantonSCS1m.swVersion = "1.7.0-b1";    // Mixxx version for display
StantonSCS1m.sysex = [0xF0, 0x00, 0x01, 0x02];  // Preamble for all SysEx messages for this device
StantonSCS1m.selectKnobMode = "browse"; // Current mode for the gray select knob
StantonSCS1m.inSetup = false; // Flag for if the device is in setup mode
StantonSCS1m.scratch = { "revtime":1.8, "alpha":0.1, "beta":1.0 };  // Variables used in the scratching alpha-beta filter: (revtime = 1.8 to start)
StantonSCS1m.trackDuration = [0,0]; // Duration of the song on each deck (used for vinyl LEDs)
StantonSCS1m.lastLight = [-1,-1];   // Last circle LED values
StantonSCS1m.lastTime = ["-99:99","-99:99"];    // Last time remaining values
StantonSCS1m.lastColor = [0,0]; // Last color of display flash
StantonSCS1m.displayFlash = [-1,-1];  // Temp storage for display flash timeouts
StantonSCS1m.lastCrossFader = 0;  // Last value of the cross fader

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
    
        // Virtual platter LEDs & time displays
    engine.connectControl("[Channel1]","playposition","StantonSCS1m.positionUpdates1");
    engine.connectControl("[Channel2]","playposition","StantonSCS1m.positionUpdates2");
    engine.connectControl("[Channel1]","duration","StantonSCS1m.durationChange1");
    engine.connectControl("[Channel2]","duration","StantonSCS1m.durationChange2");
    
        // Faders
    engine.connectControl("[Master]","crossfader","StantonSCS1m.crossFaderStart");
    
    // Set LCD text
    StantonSCS1m.initLCDs();

    StantonSCS1m.browseButton(StantonSCS1m.channel, 0x20, 0x7F, 0x90+StantonSCS1m.channel); // Force into browse mode

    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 34, 32, 0xF7]),8); // Get position of all pots

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
    midi.sendShortMsg(No,49,96);   // to green
    message = " Pitch 2";
    midi.sendSysexMsg(StantonSCS1m.sysex.concat([StantonSCS1m.channel, 4],message.toInt(), 0xF7),7+message.length);   // Set LCD4
    midi.sendShortMsg(No,49+3,96);   // to green
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

StantonSCS1m.browseButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
    var byte1 = 0x90 + channel;
    if ((status & 0XF0) == 0x90) {    // If button down
        midi.sendShortMsg(byte1,control,64);  // Light 'er up
        StantonSCS1m.selectKnobMode = "browse";
        midi.sendShortMsg(0x80+channel,30,0);  // turn off the "control" mode button
        return;
    }
}

StantonSCS1m.setupButton = function (channel, control, value, status) {
    if ((status & 0XF0) == 0x90) StantonSCS1m.inSetup = !StantonSCS1m.inSetup;
    else if (StantonSCS1m.inSetup) {  // If entering setup, change the LCD back light colors to green
        var No = 0x90 + channel;
        midi.sendShortMsg(No,49,96);    // to green
        midi.sendShortMsg(No,49+3,96);  // to green
        midi.sendShortMsg(No,49+1,96);  // to red
        midi.sendShortMsg(No,49+2,96);  // to red
    }
    else {
        StantonSCS1m.initLCDs(); // If out of setup & the button was released, restore the displays 
        StantonSCS1m.lastTime = ["-99:99","-99:99"];    // Force time displays to update
        engine.trigger("[Channel1]","rate");    // Force pitch displays to update
        engine.trigger("[Channel2]","rate");
    }
}

StantonSCS1m.selectKnob = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
    switch (StantonSCS1m.selectKnobMode) {
        case "control":
            var newValue=(value-64);
//             print("C1="+value+", jog="+newValue);
            engine.setValue("[Channel"+StantonSCS1m.deck+"]","jog",newValue);
            break;
        case "browse":
            if (value>0x40) {
                engine.setValue("[Playlist]","SelectNextTrack",1);
            }
            else {
                engine.setValue("[Playlist]","SelectPrevTrack",1);
            }
            break;
    }
}

StantonSCS1m.cancelButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
}

StantonSCS1m.enterButton = function (channel, control, value, status) {
  if (StantonSCS1m.checkInSetup()) return;
}

StantonSCS1m.encoderJog1 = function (channel, control, value, status) {
    StantonSCS1m.encoderJog(value,1);
}

StantonSCS1m.encoderJog2 = function (channel, control, value, status) {
    StantonSCS1m.encoderJog(value,2);
}

StantonSCS1m.encoderJog = function (value,deck) {
    if (StantonSCS1m.checkInSetup()) return;
    engine.setValue("[Channel"+deck+"]","jog",(value-64)*4);
}

// ----------   Slot functions  ----------

StantonSCS1m.crossFaderStart = function (value) {
  if (!StantonSCS1m.faderStart) return;
  if (StantonSCS1m.lastCrossFader==value) return;

  if (value==-1.0) engine.setValue("[Channel2]","cue_default",0);
  if (value==1.0) engine.setValue("[Channel1]","cue_default",0);
  if (StantonSCS1m.lastCrossFader==-1.0) engine.setValue("[Channel2]","play",1);
  if (StantonSCS1m.lastCrossFader==1.0) engine.setValue("[Channel1]","play",1);
  StantonSCS1m.lastCrossFader=value;
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
    var range = 1/8;
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
    var data = (value * engine.getValue("[Channel"+deck+"]","rateRange") * 100).toFixed(2) + "%";
    if (value>0) data = "+"+data;
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
    // Revolution time of the imaginary record in seconds
    var revtime = StantonSCS1m.scratch["revtime"];
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

/* TODO:
- Channel fader start
- Cross- (& channel?) fader start assign buttons
- Jog wheel scratching, control button
- Pitch range change
*/
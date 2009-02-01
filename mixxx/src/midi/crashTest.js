function crashTest() {}

// ----------   Customization variables ----------
crashTest.fastDeckChange = false;    // Skip the flashy lights if true

// ----------   Global variables    ----------
crashTest.deck = 1;  // Currently active virtual deck
crashTest.deckSignals = [    ["CurrentChannel", "rate", "crashTest.pitchLEDs"],
                                ["CurrentChannel", "rateRange", "crashTest.pitchSliderLED"],
                                ["CurrentChannel", "volume", "crashTest.gainLEDs"],
                                ["CurrentChannel", "play", "crashTest.playLED"],
                                ["CurrentChannel", "cue_default", "crashTest.cueLED"],
                                ["CurrentChannel", "back", "crashTest.B13LED"],
                                ["CurrentChannel", "fwd", "crashTest.B14LED"],
                                ["CurrentChannel", "filterLow", "crashTest.EQLowLEDs"],
                                ["CurrentChannel", "filterMid", "crashTest.EQMidLEDs"],
                                ["CurrentChannel", "filterHigh", "crashTest.EQHighLEDs"]
                            ];

crashTest.temp = { "channel":1, "device":"CrashTest MIDI 1" };

// ----------   Functions   ----------

crashTest.init = function () {   // called when the MIDI device is opened & set up
    crashTest.connectSignals(); // Connect initial set of signals
    print("CrashTest: Init successful");
}

// (Dis)connects the mode-independent Mixxx control signals to/from functions based on the currently controlled virtual deck
crashTest.connectSignals = function (disconnect) {
    var signalList = crashTest.deckSignals;
    for (i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        var name = signalList[i][1];
        if (group=="CurrentChannel") group = "[Channel"+crashTest.deck+"]";
        engine.connectControl(group,name,signalList[i][2],disconnect);
//         print("crashTest: (dis)connected "+group+","+name+" to/from "+signalList[i][2]);
        
        // If connecting a signal, update the LEDs
        if (!disconnect) {
            // Cause the signal to fire by setting it to the same value
            engine.setValue(group,name,engine.getValue(group,name));
        }  
    }
    // If disconnecting signals, darken the corresponding LEDs
    if (disconnect) {
        var CC = 0xB0 + (crashTest.temp["channel"]-1);
        var No = 0x90 + (crashTest.temp["channel"]-1);
        midi.sendShortMsg(CC,0x07,0x00,crashTest.temp["device"]);  // Gain LEDs off
        midi.sendShortMsg(CC,0x03,0x00,crashTest.temp["device"]);  // Pitch LEDs off
        midi.sendShortMsg(No,0x6D,0x00,crashTest.temp["device"]);  // PLAY button blue
        midi.sendShortMsg(No,0x6E,0x00,crashTest.temp["device"]);  // CUE button blue
        midi.sendShortMsg(No,0x6F,0x00,crashTest.temp["device"]);  // SYNC button blue
        midi.sendShortMsg(No,0x70,0x00,crashTest.temp["device"]);  // TAP button blue
    }
}

// Sets color of side circle LEDs (used for deck change effect)
crashTest.circleLEDsColor = function (color,side) {
    var channel = crashTest.temp["channel"];
    var byte1 = 0x90 + (channel-1);
    var start;
    var end;
    if (side=="left") { start = 0x5e; end = 0x63; }
    else { start = 0x66; end = 0x6b; }
    for (i=start; i<=end; i++) midi.sendShortMsg(byte1,i,color,crashTest.temp["device"]);
}

crashTest.lightDelay = function () {
    var date = new Date();
    var curDate = null;
    
    do { curDate = new Date(); }
    while(curDate-date < 60);
}

crashTest.swapSignals = function (channel, device, control, value, category) {
    if (category != (0x80 + channel-1)) return;  // Only respond to button up events
    var byte1 = 0x90 + (channel-1);
    crashTest.connectSignals(true);    // Disconnect signals for outgoing deck
    if (crashTest.deck == 1) {
        print("crashTest: Switching to deck 2");
        crashTest.deck++;
        midi.sendShortMsg(byte1,0x71,0x00,crashTest.temp["device"]);  // Deck A light off
        if (!crashTest.fastDeckChange) { // Make flashy lights to signal a deck change
            crashTest.circleLEDsColor(0x00,"right");
            crashTest.lightDelay();
            midi.sendShortMsg(byte1,0x72,0x01,crashTest.temp["device"]);  // Deck B light on
            crashTest.circleLEDsColor(0x01,"right");
            crashTest.lightDelay();
            midi.sendShortMsg(byte1,0x72,0x00,crashTest.temp["device"]);  // Deck B light off
            crashTest.circleLEDsColor(0x00,"right");
            crashTest.lightDelay();
            midi.sendShortMsg(byte1,0x72,0x01,crashTest.temp["device"]);  // Deck B light on
            crashTest.circleLEDsColor(0x01,"right");
            crashTest.lightDelay();
            midi.sendShortMsg(byte1,0x72,0x00,crashTest.temp["device"]);  // Deck B light off
            crashTest.circleLEDsColor(0x00,"right");
            crashTest.lightDelay();
        }
            midi.sendShortMsg(byte1,0x72,0x01,crashTest.temp["device"]);  // Deck B light on
    }
    else {
        print("crashTest: Switching to deck 1");
        crashTest.deck--;
        midi.sendShortMsg(byte1,0x72,0x00,crashTest.temp["device"]);  // Deck B light off
        if (!crashTest.fastDeckChange) {
            crashTest.circleLEDsColor(0x00,"left");
            crashTest.lightDelay();
            midi.sendShortMsg(byte1,0x71,0x01,crashTest.temp["device"]);  // Deck A light on
            crashTest.circleLEDsColor(0x01,"left");
            crashTest.lightDelay();
            midi.sendShortMsg(byte1,0x71,0x00,crashTest.temp["device"]);  // Deck A light off
            crashTest.circleLEDsColor(0x00,"left");
            crashTest.lightDelay();
            midi.sendShortMsg(byte1,0x71,0x01,crashTest.temp["device"]);  // Deck A light on
            crashTest.circleLEDsColor(0x01,"left");
            crashTest.lightDelay();
            midi.sendShortMsg(byte1,0x71,0x00,crashTest.temp["device"]);  // Deck A light off
            crashTest.circleLEDsColor(0x00,"left");
            crashTest.lightDelay();
        }
        midi.sendShortMsg(byte1,0x71,0x01,crashTest.temp["device"]);  // Deck A light on
    }
    crashTest.connectSignals();    // Connect signals for incoming deck
}

crashTest.rewind = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        engine.setValue("[Channel"+crashTest.deck+"]","back",1);
        return;
    }
    engine.setValue("[Channel"+crashTest.deck+"]","back",0);
}

crashTest.ffwd = function (channel, device, control, value, category) {
    var byte1 = 0x90 + (channel-1);
    if (category != (0x80 + channel-1)) {    // If button down
        engine.setValue("[Channel"+crashTest.deck+"]","fwd",1);
        return;
    }
    engine.setValue("[Channel"+crashTest.deck+"]","fwd",0);
}

// ----------   "Slot" functions  ----------

crashTest.pitchLEDs = function (value) {
    print("crashTest: pitchLEDs      value="+value);
}

crashTest.pitchSliderLED = function (value) {
    print("crashTest: pitchSliderLED value="+value);
}

crashTest.gainLEDs = function (value) {
    print("crashTest: gainLEDs       value="+value);
}

crashTest.playLED = function (value) {
    print("crashTest: playLED        value="+value);
}

crashTest.cueLED = function (value) {
    print("crashTest: cueLED         value="+value);
}

crashTest.EQLowLEDs = function (value) {
    print("crashTest: EQLowLEDs      value="+value);
}

crashTest.EQMidLEDs = function (value) {
    print("crashTest: EQMidLEDs      value="+value);
}

crashTest.EQHighLEDs = function (value) {
    print("crashTest: EQHighLEDs     value="+value);
}

crashTest.B13LED = function (value) {
    print("crashTest: B13LED         value="+value);
}

crashTest.B14LED = function (value) {
    print("crashTest: B14LED         value="+value);
}

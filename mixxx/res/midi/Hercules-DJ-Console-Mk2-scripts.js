function HerculesMk2 () {}

HerculesMk2.ledOn = 0x7F;
HerculesMk2.ledOff = 0x00;

HerculesMk2.leds = { 
    "[Channel1] fx":0x0F,
    "[Channel2] fx":0x10,
    "[Channel1] cue 123":0x0E,
    "[Channel2] cue 123":0x11,
    "[Channel1] loop":0x0D,
    "[Channel2] loop":0x12,
    "[Channel1] master tempo":0x16,
    "[Channel2] master tempo":0x1A,
    "[Channel1] auto beat":0x0A,
    "[Channel2] auto beat":0x04,
    "[Channel1] cue":0x09,
    "[Channel2] cue":0x03,
    "[Channel1] play blink":0x00,
    "[Channel2] play blink":0x05,
    "[Channel1] play":0x08,
    "[Channel2] play":0x02

    // Audio source LEDs accept message 'B0 7E Value' for both left
    // and right decks. i.e. the middle byte remains 7E for both.
    //
    // Values are as follows:
    // 00 = Deck A Computer mode + Deck B Computer mode
    // 04 = Deck A Computer mode + Deck B Input mode
    // 08 = Deck A Input mode + Deck B Computer mode
    // 0C = Deck A Input mode + Deck B Input mode

    // "audio source":0x7E
};

HerculesMk2.getGroup = function (control){ 
    // Get the "group" that used to be provided in group, this is not reusable 
    // across devices and also breaks remapping of these functions to other 
    // buttons.  
    controlToGroup = { 
	0x08:"[Channel1]", // Play
	0x02:"[Channel2]",
	0x09:"[Channel1]", // Cue 
	0x03:"[Channel2]", 
	0x36:"[Channel1]", // Jog Wheels 
	0x37:"[Channel2]",
	0x1B:"[Channel1]", // Load selected track 
	0x1C:"[Channel2]" 
   }
   return controlToGroup[control];
}

HerculesMk2.debug = false; 
HerculesMk2.scratchMode = false;
HerculesMk2.decayLast = new Date().getTime();
HerculesMk2.decayInterval = 300;
HerculesMk2.decayRate = 1.5;
HerculesMk2.cueLedOn = { "[Channel1]": false, "[Channel2]": false };
// HerculesMk2.cueButton = { "[Channel1]": false, "[Channel2]": false };
// HerculesMk2.cuePlay = { "[Channel1]": false, "[Channel2]": false };

// TODO HerculesMk2 controls should be divided into groups, then signals 
// should directed to each group without thinking about specific controls 
// to allow for easy rebinding.

HerculesMk2.init = function (id) { // called when the device is opened & set up
    HerculesMk2.setLeds('on');
    HerculesMk2.setLeds('off');
    
    // Set controls in Mixxx to reflect settings on the device
    midi.sendShortMsg(0xB0,0x7F,0x7F);
    
    engine.connectControl("[Channel1]","playposition","HerculesMk2.wheelDecay");
    engine.connectControl("[Channel2]","playposition","HerculesMk2.wheelDecay");
    
    print ("HerculesMk2 id: \""+id+"\" initialized.");
};
    
HerculesMk2.shutdown = function (id) {
    HerculesMk2.setLeds('off');
};

HerculesMk2.setLeds = function (onOff) {
    for (LED in HerculesMk2.leds) {
        if (HerculesMk2.debug) 
	    print("Set LED: " + LED + " Value: " + onOff.toString(16));
	HerculesMk2.setLed(LED,onOff);
	// Seems that if midi messages are sent too quickly, leds don't behave
	// as expected. Pause of 6ms rectifies this.
	HerculesMk2.pauseScript(6);
    }
};

HerculesMk2.setLed = function (led, onOff) {
    value = onOff=='on' ?  HerculesMk2.ledOn : HerculesMk2.ledOff;
    midi.sendShortMsg(0xB0,HerculesMk2.leds[led],value);
};

HerculesMk2.pauseScript = function(ms) {
    startDate = new Date();
    currentDate = null;

    while(currentDate-startDate < ms) currentDate = new Date();
};

HerculesMk2.pfl = function (group, control, value, status) {
    if (value) { // Act on given mode being selected, not deselected. 
	if (HerculesMk2.debug) print("HerculesMk2.pfl: " + control.toString(16) + " " + value.toString(16));
	switch(control) {
	case 0x24: // Headphones Mix
	case 0x23: // Headphones Split
	    if (HerculesMk2.debug) print("HerculesMk2.pfl: Mix/Split");
	    engine.setValue("[Channel1]", "pfl", 1);
	    engine.setValue("[Channel2]", "pfl", 1);
	    break;
	case 0x21: // Headphones Deck A
	    if (HerculesMk2.debug) print("HerculesMk2.pfl: Deck A");
	    engine.setValue("[Channel1]", "pfl", 1);	    
	    engine.setValue("[Channel2]", "pfl", 0);
	    break;
	case 0x22: // Headphones Deck B
	    if (HerculesMk2.debug) print("HerculesMk2.pfl: Deck B");
	    engine.setValue("[Channel1]", "pfl", 0);	    
	    engine.setValue("[Channel2]", "pfl", 1);	    
	}
    }
};

HerculesMk2.cue = function (group, control, value, status) {
    group = HerculesMk2.getGroup(control);
    
    if ((engine.getValue(group, "duration") == 0) && (value)) { 
	print("No song on " + group);
	return; 
    }
    
    if (value) { // Down
	engine.setValue(group,"cue_default",1);
	HerculesMk2.setLed(group + ' cue', 'on');
	HerculesMk2.cueLedOn[group] = true;
    } else { // Release
	if (HerculesMk2.debug) print("R. Play: " + engine.getValue(group,"play") + " PlayPosition: " + engine.getValue(group,"playposition") + " cue_default: "+ engine.getValue(group,"cue_default"));
	engine.setValue(group,"cue_default",0);
    }
};

HerculesMk2.play = function (group, control, value, status) {
    if (value) { // Only do stuff when play is pushed, not released.
	group = HerculesMk2.getGroup(control);
	
	if (engine.getValue(group, "duration") == 0) { 
	    print("No song on " + group);
	    return; 
	}
	
	engine.setValue(group,"play", !engine.getValue(group,"play"));
	HerculesMk2.setLed(group + ' cue', 'off');
    }
};

HerculesMk2.loadSelectedTrack = function (group, control, value, status) {
    if (value) { // Only do stuff when pushed, not released.
	group = HerculesMk2.getGroup(control);
	engine.setValue(group, "LoadSelectedTrack", 1);
	HerculesMk2.setLed(group + " cue", 'on');
	HerculesMk2.cueLedOn[group] = true;
    }
}

HerculesMk2.jog_wheel = function (group, control, value, status) {
    //  7F > 40: CCW Slow > Fast - 127 > 64 
    //  01 > 3F: CW Slow > Fast - 0 > 63
    group = HerculesMk2.getGroup(control);
    
    if (HerculesMk2.cueLedOn[group]) {
	HerculesMk2.setLed(group + ' cue', 'off');
	HerculesMk2.cueLedOn[group] = false;
    }

    jogValue = value >=0x40 ? value - 0x80 : value; // -64 to +63, - = CCW, + = CW
    
    if (HerculesMk2.scratchMode) { // do some scratching
	if (HerculesMk2.debug) print("Do scratching value:" + value + " jogValue: " + jogValue );
	engine.setValue(group,"scratch", (engine.getValue(group,"scratch") + (jogValue/64)).toFixed(2));
    } else { // do pitch adjustment
	newValue = jogValue; 
	if (HerculesMk2.debug) print("do pitching adjust " + jogValue + " new Value: " + newValue);
	engine.setValue(group,"jog", newValue);
    }
};
    
HerculesMk2.wheelDecay = function (value) {    
    currentDate = new Date().getTime();
    
    if (currentDate > HerculesMk2.decayLast + HerculesMk2.decayInterval) {
	HerculesMk2.decayLast = currentDate;
	
	if (HerculesMk2.debug) print(" new playposition: " + value + " decayLast: "+ HerculesMk2.decayLast);
	if (HerculesMk2.scratchMode) { // do some scratching
	    if (HerculesMk2.debug) print("Scratch deck1: " + engine.getValue("[Channel1]","scratch") + " deck2: "+ engine.getValue("[Channel2]","scratch"));
	    
	    jog1DecayRate = HerculesMk2.decayRate * (engine.getValue("[Channel1]","play") ? 1 : 5);
	    jog1 = engine.getValue("[Channel1]","scratch"); 
	    if (jog1 != 0) {
		if (Math.abs(jog1) > jog1DecayRate) {  
		    engine.setValue("[Channel1]","scratch", (jog1 / jog1DecayRate).toFixed(2));
		} else {
		    engine.setValue("[Channel1]","scratch", 0);
		}
	    }
	    jog2DecayRate = HerculesMk2.decayRate * (engine.getValue("[Channel2]","play") ? 1 : 5);
	    jog2 = engine.getValue("[Channel2]","scratch"); 
	    if (jog2 != 0) {
		if (Math.abs(jog2) > jog2DecayRate) {  
		    engine.setValue("[Channel2]","scratch", (jog2 / jog2DecayRate).toFixed(2));
		} else {
		    engine.setValue("[Channel2]","scratch", 0);
		}
	    }
	}
    }
}

function HerculesMk2 () {}

HerculesMk2.ledOn = 0x7F;
HerculesMk2.ledOff = 0x00;

HerculesMk2.debug = false;
HerculesMk2.scratchMode = false;
HerculesMk2.decayLast = new Date().getTime();
HerculesMk2.decayInterval = 300;
HerculesMk2.decayRate = 1.5;

HerculesMk2.buttons123Modes = ["kill", "fx", "cue", "loop"];
HerculesMk2.buttons123used = {"[Channel1]": false, "[Channel1]": false};

// TODO HerculesMk2 controls should be divided into groups, then signals 
// should directed to each group without thinking about specific controls 
// to allow for easy rebinding.

HerculesMk2.cueLastDate1 = {"[Channel1]": 0, "[Channel2]" : 0};
HerculesMk2.cueLastDate2 = {"[Channel1]": 0, "[Channel2]" : 0};
HerculesMk2.cueLastDate3 = {"[Channel1]": 0, "[Channel2]" : 0};
HerculesMk2.cueLongTime = 500;

HerculesMk2.controls = {
    "inputs": {
	0x09: { "channel": 1, "name": "cue", "type": "button" },
	0x03: { "channel": 2, "name": "cue", "type": "button" },
	0x08: { "channel": 1, "name": "play", "type": "button" },
	0x02: { "channel": 2, "name": "play", "type": "button" },
	0x07: { "channel": 1, "name": "fx select", "type": "button", 
		"mode": 0 },
	0x01: { "channel": 2, "name": "fx select", "type": "button", 
		"mode": 0 },
	0x0F: { "channel": 1, "name": "fx 1", "type": "button", "used": false },
	0x10: { "channel": 2, "name": "fx 1", "type": "button", "used": false },
	0x0E: { "channel": 1, "name": "fx 2", "type": "button", "used": false },
	0x11: { "channel": 2, "name": "fx 2", "type": "button", "used": false },
	0x0D: { "channel": 1, "name": "fx 3", "type": "button", "used": false },
	0x12: { "channel": 2, "name": "fx 3", "type": "button", "used": false },
	0x1B: { "channel": 1, "name": "mouse", "type": "button" },
	0x1C: { "channel": 2, "name": "mouse", "type": "button" },
	0x34: { "channel": 1, "name": "pitch", "type": "pot" },
	0x35: { "channel": 2, "name": "pitch", "type": "pot" },
	0x36: { "channel": 1, "name": "wheel", "type": "pot" },
	0x37: { "channel": 2, "name": "wheel", "type": "pot" }
    },
    "outputs": {
	0x0F: { "channel": 1, "name": "fx mode", "type": "led" },
	0x10: { "channel": 2, "name": "fx mode", "type": "led" },
	0x0E: { "channel": 1, "name": "cue mode", "type": "led" },
	0x11: { "channel": 2, "name": "cue mode", "type": "led" },
	0x0D: { "channel": 1, "name": "loop mode", "type": "led" },
	0x12: { "channel": 2, "name": "loop mode", "type": "led" },
	0x16: { "channel": 1, "name": "master tempo", "type": "led" },
	0x1A: { "channel": 2, "name": "master tempo", "type": "led" },
	0x0A: { "channel": 1, "name": "auto beat", "type": "led" },
	0x04: { "channel": 2, "name": "auto beat", "type": "led" },
	0x09: { "channel": 1, "name": "cue", "type": "led" },
	0x03: { "channel": 2, "name": "cue", "type": "led" },
	0x00: { "channel": 1, "name": "play blink", "type": "led" },
	0x05: { "channel": 2, "name": "play blink", "type": "led" },
	0x08: { "channel": 1, "name": "play", "type": "led" },
	0x02: { "channel": 2, "name": "play", "type": "led" }
    }
};

HerculesMk2.leds = { 
};

HerculesMk2.init = function (id) { // called when the device is opened & set up
    HerculesMk2.initializeControls();
    
    engine.connectControl("[Channel1]","playposition","HerculesMk2.wheelDecay");
    engine.connectControl("[Channel2]","playposition","HerculesMk2.wheelDecay");
    
    print ("HerculesMk2 id: \""+id+"\" initialized.");
};
    
HerculesMk2.initializeControls = function () {
    for (control in HerculesMk2.controls.outputs) {
	if (HerculesMk2.controls.outputs[control].type == 'led') {
	    key = "[Channel" + HerculesMk2.controls.outputs[control].channel + "] " + HerculesMk2.controls.outputs[control].name;
	    HerculesMk2.leds[key] = control;
	}
    }

    HerculesMk2.setLeds("on");
    HerculesMk2.setLeds("off");
    
    // Set controls in Mixxx to reflect settings on the device
    midi.sendShortMsg(0xB0,0x7F,0x7F);
};

HerculesMk2.shutdown = function (id) {
    HerculesMk2.setLeds("off");
};

HerculesMk2.getGroup = function (control){ 
    // Get the "group" that used to be provided in group, this is not reusable 
    // across devices and also breaks remapping of these functions to other 
    // buttons.  

    return "[Channel" + HerculesMk2.controls.inputs[control].channel + "]";
};

HerculesMk2.getControl = function (io, channel, name) { 
    // Accept channel in form 'N' or '[ChannelN]'
    channel = channel.replace(/\[Channel(\d)\]/, "$1");

    for (control in HerculesMk2.controls.inputs) {
	if (HerculesMk2.controls.inputs[control].channel == channel && 
	    HerculesMk2.controls.inputs[control].name == name
	    ) return HerculesMk2.controls.inputs[control];
    }

    print ("HerculesMk2.getControl: Control not found: io=" + io + ": channel=" + channel + ": name=" + name);
};

HerculesMk2.setLeds = function (onOff) {
    for (LED in HerculesMk2.leds) {
	HerculesMk2.setLed(LED,onOff);
	// Seems that if midi messages are sent too quickly, leds don't behave
	// as expected. A pause rectifies this.
	HerculesMk2.pauseScript(10);
    }
};

HerculesMk2.setLed = function (led, onOff) {
    value = onOff=="on" ?  HerculesMk2.ledOn : HerculesMk2.ledOff;
    if (HerculesMk2.debug) print ("HerculesMk2.setLed: Setting " + led + " led " + onOff);
    if (HerculesMk2.debug) print ("HerculesMk2.setLed: midi.sendShortMsg(0xB0," + HerculesMk2.leds[led].toString(16) + "," + value + ")");
    midi.sendShortMsg(0xB0,HerculesMk2.leds[led],value);
    HerculesMk2.controls.outputs[HerculesMk2.leds[led]].isOn = onOff=="on" ? true : false;
};

HerculesMk2.pauseScript = function(ms) {
    startDate = new Date();
    currentDate = null;

    while(currentDate-startDate < ms) currentDate = new Date();
};

HerculesMk2.pfl = function (group, control, value, status) {
    if (value) { // Act on given mode being selected, not deselected. 
	if (HerculesMk2.debug) print("HerculesMk2.pfl: " + control.toString(16) + " " + value.toString(16));
	switch (control) {
	case 0x23: // Headphones Split
	case 0x24: // Headphones Mix
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
	HerculesMk2.setLed(group + " cue", "on");
    } else { // Release
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
	HerculesMk2.setLed(group + " cue", "off");
    }
};

HerculesMk2.loadSelectedTrack = function (group, control, value, status) {
    if (value) { // Only do stuff when pushed, not released.
	group = HerculesMk2.getGroup(control);
	engine.setValue(group, "LoadSelectedTrack", 1);
	HerculesMk2.setLed(group + " cue", "on");
    }
};

HerculesMk2.buttons123 = function (group, control, value, status) {
    group = HerculesMk2.getGroup(control);

    if (value) { // Button pressed.
	HerculesMk2.controls.inputs[control].isDown = true;
    } else { //Button released.
	HerculesMk2.controls.inputs[control].isDown = false;
    }

    mode = HerculesMk2.getControl("inputs", group, "fx select").mode;
    mode = HerculesMk2.buttons123Modes[mode];
    
    switch (mode) {

    case "kill": // Kill mode
	if (value) { // Button pressed.
	    switch (HerculesMk2.controls.inputs[control].name) {
	    case "fx 1":
		engine.setValue(group, "filterLowKill", !engine.getValue(group, "filterLowKill"));
		break;
	    case "fx 2": 
		engine.setValue(group, "filterMidKill", !engine.getValue(group, "filterMidKill"));
		break;
	    case "fx 3":
		engine.setValue(group, "filterHighKill", !engine.getValue(group, "filterHighKill"));
		break;
	    } 
	}
	break; // End kill mode

    case "fx": // Fx mode
	// Were only turning off the flanger if any of the 123 buttons are
	// released, not pushed. This is so we can have any of the 123 buttons
	// held down, then use the pitch pot to modify the effect settings
	if (!value) { // Button released.
	    if (HerculesMk2.controls.inputs[control].used) {
		// Button was used to turn the pitch control into a modifier 
		// for the effect settings, so don't go on and turn the flanger
		// on/off
		HerculesMk2.controls.inputs[control].used = false;
		return;
	    }

	    switch (HerculesMk2.controls.inputs[control].name) {
	    case "fx 1":
	    case "fx 2":
	    case "fx 3":
		engine.setValue(group, "flanger", !engine.getValue(group, "flanger"));
		break;
	    } 
	}
	break; // End fx mode

    case "cue": 
	if (value) { // onPress
		switch (HerculesMk2.controls.inputs[control].name) {
		    case "fx 1":
			HerculesMk2.cueLastDate1[group] = new Date().getTime();
			break;
		    case "fx 2":
			HerculesMk2.cueLastDate2[group] = new Date().getTime();
			break;
		    case "fx 3":
			HerculesMk2.cueLastDate3[group] = new Date().getTime();
			break;
		}
	} else { // onRelease
		currentDate = new Date().getTime();
		switch (HerculesMk2.controls.inputs[control].name) {
		    case "fx 1":
			if (currentDate - HerculesMk2.cueLastDate1[group] < HerculesMk2.cueLongTime) {
				engine.setValue(group,"hotcue_1_activate",1);
				engine.setValue(group,"hotcue_1_activate",0);
			} else {
				engine.setValue(group,"hotcue_1_clear",1);
				engine.setValue(group,"hotcue_1_clear",0);
			}
			break;
		    case "fx 2":
			if (currentDate - HerculesMk2.cueLastDate2[group] < HerculesMk2.cueLongTime) {
				engine.setValue(group,"hotcue_2_activate",1);
				engine.setValue(group,"hotcue_2_activate",0);
			} else {
				engine.setValue(group,"hotcue_2_clear",1);
				engine.setValue(group,"hotcue_2_clear",0);
			}
			break;
		    case "fx 3":
			if (currentDate - HerculesMk2.cueLastDate3[group] < HerculesMk2.cueLongTime) {
				engine.setValue(group,"hotcue_3_activate",1);
				engine.setValue(group,"hotcue_3_activate",0);
			} else {
				engine.setValue(group,"hotcue_3_clear",1);
				engine.setValue(group,"hotcue_3_clear",0);
			}
			break;
		}

	}
	break;

    case "loop":
        if (value) { // Button pressed.
            switch (HerculesMk2.controls.inputs[control].name) {
            case "fx 1": // "fx 1,2,3" should be globally renamed as said before?...
                // trigger loop in
                engine.setValue(group,"loop_in", !engine.getValue(group,"start")); // Am I correct?
                break;
            case "fx 2":
                // trigger loop out
                engine.setValue(group,"loop_out", !engine.getValue(group,"end")); // Am I correct?
                break;
            case "fx 3":
                // trigger loop exit
                engine.setValue(group,"reloop_exit", !engine.getValue(group,"loop")); // Am I correct?
                break;
           }
        }
	break;

    default: 
	print("HerculesMk2.buttons123: " + mode + " mode unsupported");
    }
};

HerculesMk2.buttons123mode = function (group, control, value, status) {
    group = HerculesMk2.getGroup(control);
    if (value) { // Only do stuff when pushed, not released.
	currentMode = HerculesMk2.controls.inputs[control].mode;
	nextMode = currentMode < HerculesMk2.buttons123Modes.length-1 ? currentMode+1 : 0;
	currentLed = group + " " + HerculesMk2.buttons123Modes[currentMode] + " mode";
	nextLed = group + " " + HerculesMk2.buttons123Modes[nextMode] + " mode";

	sNextMode = HerculesMk2.buttons123Modes[nextMode]; 
	switch (sNextMode) {
	case "kill":
	case "fx": 
	    print("HerculesMk2.buttons123mode: Switching to " + sNextMode + " mode");
	    break;
	case "cue": 
	    print("HerculesMk2.buttons123mode: Switching to " + sNextMode + " mode");
	    break;
	case "loop":
	    print("HerculesMk2.buttons123mode: Switching to " + sNextMode + " mode");
	    break;
	default:
	    print("HerculesMk2.buttons123mode: Switching to " + sNextMode + " mode");
	}

	// Only turn on/off leds for non-zero modes as 0 is kill mode which
	// has no corresponding LED. i.e. all LEDs off for kill mode.
	if (currentMode) HerculesMk2.setLed(currentLed, "off");
	// Seems that if midi messages are sent too quickly, leds don't behave
	// as expected. A pause rectifies this.
	HerculesMk2.pauseScript(10);
	if (nextMode) HerculesMk2.setLed(nextLed, "on");

	HerculesMk2.controls.inputs[control].mode = nextMode;
    }
};

HerculesMk2.pitch = function (group, control, value, status) {
    //  7F > 40: CCW Slow > Fast - 127 > 64 
    //  01 > 3F: CW Slow > Fast - 0 > 63

    group = HerculesMk2.getGroup(control);
    pitchControl = HerculesMk2.getControl("inputs", group, "pitch");
    done = false;

    currentMode = HerculesMk2.getControl("inputs", group, "fx select").mode;
    currentMode = HerculesMk2.buttons123Modes[currentMode];

    // If in fx mode and one or more of buttons 123 are pressed, use pitch
    // pot to adjust the relevant flanger parameters instead of changing
    // the rate (as is normal operation for the pitch pot)

    if (currentMode == "fx") { 
	potStep = 25; // How many clicks round the pot from min to max values

	if (HerculesMk2.getControl("inputs", group, "fx 1").isDown) {
	    min = 0; max = 1;
	    increment = (max-min)/potStep;
	    increment = (value <= 0x3F) ? increment : increment * -1;

	    currentValue = engine.getValue("[Flanger]", "lfoDepth");
	    newValue = currentValue + increment;
	    newValue = newValue > max ? max : newValue < min ? min : newValue;
	    if (HerculesMk2.debug) print ("HerculesMk2.pitch: value= " + newValue);
	    if (newValue != currentValue) 
		engine.setValue("[Flanger]", "lfoDepth", newValue);
	    
	    HerculesMk2.getControl("inputs", group, "fx 1").used = true;
	    done = true;
	} 

	if (HerculesMk2.getControl("inputs", group, "fx 2").isDown) {
	    min = 50; max = 10000;
	    increment = (max-min)/potStep;
	    increment = (value <= 0x3F) ? increment : increment * -1;
	
	    currentValue = engine.getValue("[Flanger]", "lfoDelay");
	    newValue = currentValue + increment;
	    newValue = newValue > max ? max : newValue < min ? min : newValue;
	    if (HerculesMk2.debug) print ("HerculesMk2.pitch: value= " + newValue);
	    if (newValue != currentValue) 
		engine.setValue("[Flanger]", "lfoDelay", newValue);
	    
	    HerculesMk2.getControl("inputs", group, "fx 2").used = true;
	    done = true;
	}
	
	if (HerculesMk2.getControl("inputs", group, "fx 3").isDown) {
	    min = 50000; max = 2000000
	    increment = (max-min)/potStep;
	    increment = (value <= 0x3F) ? increment : increment * -1;

	    currentValue = engine.getValue("[Flanger]", "lfoPeriod");
	    newValue = currentValue + increment;
	    newValue = newValue > max ? max : newValue < min ? min : newValue;
	    if (HerculesMk2.debug) print ("HerculesMk2.pitch: value= " + newValue);
	    if (newValue != currentValue) 
		engine.setValue("[Flanger]", "lfoPeriod", newValue);
	    
	    HerculesMk2.getControl("inputs", group, "fx 3").used = true;
	    done = true;
	}
    }

    if (done) return;

    increment = 0.016;
    increment = (value <= 0x3F) ? increment : increment * -1;

    if (HerculesMk2.debug) print ("HerculesMk2.pitch: value=" + value);
    engine.setValue(group, "rate", engine.getValue(group, "rate") + increment);
};

HerculesMk2.jog_wheel = function (group, control, value, status) {
    //  7F > 40: CCW Slow > Fast - 127 > 64 
    //  01 > 3F: CW Slow > Fast - 0 > 63
    group = HerculesMk2.getGroup(control);

    if (HerculesMk2.controls.outputs[HerculesMk2.leds[group + " cue"]].isOn == true) {
	HerculesMk2.setLed(group + " cue", "off");
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
};

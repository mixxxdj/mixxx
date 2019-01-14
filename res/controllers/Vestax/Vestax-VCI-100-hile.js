//
// Mapping for Vestax VCI-100 controller, modified to use JS for all 
// controls.
// Copyright (C) 2012, Ilkka Tuohela
// For Mixxx version 1.11.x
// 

function HileVci100() {};

HileVci100.debugging = true;

// Virtual buttons we don't want to set LED for automatically, 
// or button inputs which don't have a LED 
IGNORE_LED_UPDATES = [
    "jog_touch", "cuestop", "loop_in", "loop_out", "reloop_exit",
    "active_loops_1", "active_loops_2", 
    "beatloop_1_enabled", "beatloop_2_enabled", 
    "beatloop_4_enabled", "beatloop_8_enabled",
    "rate_temp_up", "rate_temp_down"
]

HileVci100.NOTE_ON_CONTROLS = {
  0xc: "fader1_reset",
  0xd: "fader2_reset",
  0x2e: "jog1_touch",
  0x2f: "jog2_touch",
  0x32: "play1",
  0x33: "cup1",
  0x34: "cus1",
  0x35: "cueset1",
  0x36: "play2",
  0x37: "cup2",
  0x38: "cus2",
  0x39: "cueset2",
  0x3a: "nudge1_fwd",
  0x3b: "nudge1_back",
  0x3c: "nudge2_fwd",
  0x3d: "nudge2_back",
  0x42: "loop1",
  0x43: "loop2",
  0x44: "key1",
  0x45: "key2",
  0x46: "sync1",
  0x47: "sync2",
  0x48: "pfl1",
  0x49: "pfl2",
  0x4a: "effect1",
  0x4b: "effect2",
  // Conflicts with effects_button_4
  // 0x4c: "effect_select",
  0x4c: "effects_button_4",
  0x4f: "bank_up",
  0x52: "bank_down",
  0x55: "effects_button_1",
  0x58: "effects_button_2",
  0x5b: "effects_button_3",
  0x5c: "up",
  0x5d: "down",
  0x5e: "center",
  0x60: "left",
  0x61: "right",
  0x62: "loop_button_1",
  0x63: "loop_button_2",
  0x64: "loop_button_3",
  0x65: "loop_button_4",
  0x66: "loop_button_5",
  0x67: "loop_button_6",
  0x68: "loop_button_7",
  0x6b: "deck1_activate",
  0x6c: "deck2_activate"
};

HileVci100.FADER_CONTROLS = {
  0x7: "trim_1",
  0x8: "crossfader",
  0xc: "volume_left",
  0xd: "volume_right",
  0xe: "fader1",
  0xf: "fader2",
  0x10: "jog1_wheel",
  0x11: "jog2_wheel",
  0x12: "jog1_seek",
  0x13: "jog2_seek",
  0x14: "deck1_eq1",
  0x15: "deck1_eq2",
  0x16: "deck1_eq3",
  0x17: "deck1_eq4",
  0x18: "deck2_eq1",
  0x19: "deck2_eq2",
  0x1a: "deck2_eq3",
  0x1b: "deck2_eq4",
  0x1c: "deck1_trim",
  0x1d: "deck2_trim",
  0x1e: "deck1_balance",
  0x1f: "deck2_balance",
  0x54: "effects_knob_1",
  0x55: "effects_knob_2",
  0x56: "effects_knob_3",
  0x57: "effects_knob_4",
  0x58: "trim_2",
  0x59: "trim_3"
};

HileVci100.debug = function(message) {
    if (!HileVci100.debugging)
        return;
    print("VCI-100 " + message);
}

HileVci100.init = function(id) {
    HileVci100.id = id;

    HileVci100.buttonStates = {released:0x00, pressed:0x7F};
    HileVci100.beatloopSizes = { 1: 1, 2: 2, 3: 4, 4: 8 };
 
    HileVci100.scratchintervalsPerRev = 256;
    HileVci100.scratchRPM = 33+1/3;
    HileVci100.scratchAlpha = 1.0/8;
    HileVci100.scratchBeta = (1.0/8) / 32;
    HileVci100.rampedScratchEnable = false; 
    HileVci100.rampedScratchDisable = true;

    HileVci100.isScratchEnabled = new Object();
    HileVci100.isScratchEnabled["[Channel1]"] = false;
    HileVci100.isScratchEnabled["[Channel2]"] = false;

    // Set to 1 to invert pitch slider direction
    HileVci100.pitchDirection = -1;
    HileVci100.registerScalers();

    HileVci100.registerInputs();

    HileVci100.activeLooperGroup = "[Channel1]";

    for (var control in HileVci100.NOTE_ON_CONTROLS) {
        var name = HileVci100.NOTE_ON_CONTROLS[control];
        if (name in HileVci100.buttonMap) {
            var button = HileVci100.buttonMap[name];
            if (IGNORE_LED_UPDATES.indexOf(button.control)!=-1) 
                continue;
            value = engine.getValue(button.group,button.control);
            engine.connectControl(
                button.group,button.control,HileVci100.LEDCallback
            );
        } else {
            value = 0x0;
        }
        midi.sendShortMsg(0x90,control,value);
    }

    for (i=1;i<=7;i++) {
        var group = HileVci100.activeLooperGroup;
        var button_name = "loop_button_" + i;
        var button = HileVci100.buttonMap[button_name];
        engine.connectControl(group, button.control, HileVci100.updateLooperLEDs);
    }

    HileVci100.updateLooperLEDs();

    HileVci100.debug("Controller Initialized");
}

HileVci100.shutdown = function(id) {
    for (var button_name in HileVci100.buttonMap) {
        var button = HileVci100.buttonMap[name];
        if (IGNORE_LED_UPDATES.indexOf(button.control)!=-1) 
            continue;
        engine.connectControl(
            button.group,button.control,HileVci100.LEDCallback,true
        );
    }
}

HileVci100.registerScalers = function() {
    HileVci100.scalers = new Object();
    HileVci100.scalers.eq = function(value) {
        return script.absoluteNonLin(value,0,1,5,0,127);       
    } 
    HileVci100.scalers.pregain = function(value) {
        return script.absoluteNonLin(value,0,1,4,0,127);
    }
    HileVci100.scalers.master = function(value) {
        return script.absoluteLin(value,0,5,0,127);       
    }
    HileVci100.scalers.volume = function(value) {
        return script.absoluteLin(value,0,1,0,127);       
    }
    HileVci100.scalers.crossfader = function(value) {
        return script.absoluteLin(value,-1,1,0,127);       
    }
    HileVci100.scalers.rate = function(value) {
        return HileVci100.pitchDirection * script.absoluteLin(value,-1,1,0,127); 
    }
    HileVci100.scalers.jogticks = function(value) { return value-0x40; }
}

HileVci100.registerInputs = function() {
    HileVci100.modifiers = new Object();
    HileVci100.modifiers["[Master]"] = new Object();
    HileVci100.modifiers["[Channel1]"] = new Object();
    HileVci100.modifiers["[Channel2]"] = new Object();

    HileVci100.buttonMap = new Object();
    HileVci100.faderMap = new Object();

    HileVci100.linkButton("bank_up","[Playlist]","SelectPrevPlaylist");
    HileVci100.linkButton("bank_down","[Playlist]","SelectNextPlaylist");
    HileVci100.linkButton("up","[Playlist]","SelectPrevTrack");
    HileVci100.linkButton("down","[Playlist]","SelectNextTrack");
    HileVci100.linkButton("center","[Playlist]","LoadSelectedIntoFirstStopped");
    HileVci100.linkButton("left","[Channel1]","LoadSelectedTrack");
    HileVci100.linkButton("right","[Channel2]","LoadSelectedTrack");

    HileVci100.linkButton("effect1","[Channel1]","quantize",true);
    HileVci100.linkButton("effect2","[Channel2]","quantize",true);
    HileVci100.linkButton("pfl1","[Channel1]","pfl",true);
    HileVci100.linkButton("pfl2","[Channel2]","pfl",true);
    HileVci100.linkButton("play1","[Channel1]","play",true,HileVci100.togglePlay);
    HileVci100.linkButton("play2","[Channel2]","play",true,HileVci100.togglePlay);
    HileVci100.linkButton("sync1","[Channel1]","beatsync");
    HileVci100.linkButton("sync2","[Channel2]","beatsync");
    HileVci100.linkButton("cup1","[Channel1]","cue_gotoandplay");
    HileVci100.linkButton("cup2","[Channel2]","cue_gotoandplay");
    HileVci100.linkButton("cus1","[Channel1]","cuestop",HileVci100.cuestop);
    HileVci100.linkButton("cus2","[Channel2]","cuestop",false,HileVci100.cuestop);
    HileVci100.linkButton("cueset1","[Channel1]","cue_set");
    HileVci100.linkButton("cueset2","[Channel2]","cue_set");

    HileVci100.linkButton("jog1_touch","[Channel1]","jog_touch",false,HileVci100.scratchEnable);
    HileVci100.linkButton("jog2_touch","[Channel2]","jog_touch",false,HileVci100.scratchEnable);
    HileVci100.linkFader("jog1_wheel","[Channel1]","jog_wheel","jogticks",HileVci100.jog_wheel);
    HileVci100.linkFader("jog2_wheel","[Channel2]","jog_wheel","jogticks",HileVci100.jog_wheel);
    HileVci100.linkFader("jog1_seek","[Channel1]","jog_seek","jogticks",HileVci100.jog_wheel);
    HileVci100.linkFader("jog2_seek","[Channel2]","jog_seek","jogticks",HileVci100.jog_wheel);

    HileVci100.linkButton("nudge1_back","[Channel1]","rate_temp_down");
    HileVci100.linkButton("nudge1_fwd","[Channel1]","rate_temp_up");
    HileVci100.linkButton("nudge2_back","[Channel2]","rate_temp_down");
    HileVci100.linkButton("nudge2_fwd","[Channel2]","rate_temp_up");

    HileVci100.linkButton("loop1","[Channel1]","active_loops_1",true,HileVci100.active);
    HileVci100.linkButton("loop2","[Channel2]","active_loops_2",true,HileVci100.active);
    HileVci100.linkButton("loop_button_1","looper","loop_in",false,HileVci100.looper);
    HileVci100.linkButton("loop_button_2","looper","loop_out",false,HileVci100.looper);
    HileVci100.linkButton("loop_button_3","looper","reloop_exit",false,HileVci100.looper);
    HileVci100.linkButton("loop_button_4","beatloop","beatloop_1_enabled",true,HileVci100.beatloop);
    HileVci100.linkButton("loop_button_5","beatloop","beatloop_2_enabled",true,HileVci100.beatloop);
    HileVci100.linkButton("loop_button_6","beatloop","beatloop_4_enabled",true,HileVci100.beatloop);
    HileVci100.linkButton("loop_button_7","beatloop","beatloop_8_enabled",true,HileVci100.beatloop);

    HileVci100.linkFader("fader1","[Channel1]","rate");
    HileVci100.linkFader("fader2","[Channel2]","rate");
    HileVci100.linkFader("volume_left","[Channel1]","volume");
    HileVci100.linkFader("volume_right","[Channel2]","volume");
    HileVci100.linkFader("deck1_eq1","[Channel1]","pregain");
    HileVci100.linkFader("deck1_eq2","[Channel1]","filterHigh","eq");
    HileVci100.linkFader("deck1_eq3","[Channel1]","filterMid","eq");
    HileVci100.linkFader("deck1_eq4","[Channel1]","filterLow","eq");
    HileVci100.linkFader("deck2_eq1","[Channel2]","pregain");
    HileVci100.linkFader("deck2_eq2","[Channel2]","filterHigh","eq");
    HileVci100.linkFader("deck2_eq3","[Channel2]","filterMid","eq");
    HileVci100.linkFader("deck2_eq4","[Channel2]","filterLow","eq");
    HileVci100.linkFader("crossfader","[Master]","crossfader");
    HileVci100.linkFader("deck1_trim","[Master]","headVolume","master");
    HileVci100.linkFader("deck2_trim","[Master]","headMix","crossfader");

};

HileVci100.linkButton = function(name,group,control,toggle,callback) {
    if (name in HileVci100.buttonMap) {
        HileVci100.debug("Button name already linked: " + name);
        return;
    }
    var button = new Object();
    button.name = name;
    button.group = group;
    button.control = control;
    button.toggle = toggle;
    button.callback = callback;
    button.midino = undefined;
    for (var n in HileVci100.NOTE_ON_CONTROLS) {
        if (name == HileVci100.NOTE_ON_CONTROLS[n]) {
            button.midino = n;
            break;
        }
    }
    HileVci100.buttonMap[name] = button;
}

HileVci100.LEDCallback = function(value,group,control) { 
    for (var button_name in HileVci100.buttonMap) {
        var button = HileVci100.buttonMap[button_name];
        if (button.group!=group || button.control!=control) 
            continue;
        var status = (value) ? 0x7f : 0x0;
        midi.sendShortMsg(0x90,button.midino,status);
        return;
    }
    HileVci100.debug("LED to update not found: " + group+" "+control);
}

HileVci100.linkFader = function(name,group,control,scaler,callback) {
    if (name in HileVci100.faderMap) {
        HileVci100.debug("Fader name already linked: " + name);
        return;
    }
    if (scaler==undefined)
        scaler = control;
    if (!(scaler in HileVci100.scalers)) {
        HileVci100.debug("Unknown scaler: " + name);
        return; 
    }
    var fader = new Object();
    fader.name = name;
    fader.group = group;
    fader.control = control;
    fader.scaler = HileVci100.scalers[scaler];
    fader.callback = callback;
    HileVci100.faderMap[name] = fader;
}

HileVci100.faders = function (channel,control,value,status,group) {
    var name;
    if (status==0xb0) {
        name = HileVci100.FADER_CONTROLS[control];
    } else {
        HileVci100.debug("Received unexpected fader message code " + status);
        return;
    }
    if (control==undefined) {
        HileVci100.debug("Received unknown fader: " + name);
        return;
    }
    var fader = HileVci100.faderMap[name];
    if (fader==undefined) {
        HileVci100.debug("FADER " + name + " value " + value);
        return;
    }
    if (fader.scaler==undefined) {
        HileVci100.debug("FADER missing scaling function " + name);
        return;
    }
    value = fader.scaler(value);
    if (fader.callback)
        fader.callback(fader,value);
    else
        engine.setValue(fader.group,fader.control,value);
}

HileVci100.buttons = function (channel,control,value,status,group) {
    var name;
    var value;
    if (status==0x90) {
        name = HileVci100.NOTE_ON_CONTROLS[control];
    } else if (status==0x80) {
        name = HileVci100.NOTE_OFF_CONTROLS[control];
    } else {
        HileVci100.debug("Received unexpected button message code " + status);
        return;
    }
    if (control==undefined) {
        HileVci100.debug("Received unknown fader: " + control);
        return;
    }
    var activate = /deck[0-9]+_activate/;
    if (name.match(activate)) {
        // Activate toggles for decks: ignore for now
        return;
    }
    var button = HileVci100.buttonMap[name];

    if (button==undefined) {
        HileVci100.debug("UNMAPPED BUTTON " + name+" "+group+" "+control);
        return;
    }

    if (button.toggle) {
        if (value==HileVci100.buttonStates.released)
            return;
        if (IGNORE_LED_UPDATES.indexOf(button.control)!=-1) {
            value = true;
        } else {
            value = (engine.getValue(button.group,button.control)) ? false : true;
        }
    } else { 
        value = (value==HileVci100.buttonStates.pressed) ? true : false;
    }

    midi.sendShortMsg(0x90,button.midino,value);
    if (button.callback) {
        button.callback(button,value);
    } else {
        engine.setValue(button.group,button.control,value);
    }
}

HileVci100.cuestop = function(button,value) {
    var modifiers = HileVci100.modifiers[button.group];
    if (value) {
        engine.setValue(button.group,"cue_default",true);
        modifiers["cue_stop"] = value;
    } else if ("cue_stop" in modifiers) {
        engine.setValue(button.group,"cue_default",false);
        delete modifiers["cue_stop"];
    }
}

HileVci100.togglePlay = function(button,value) {
    var modifiers = HileVci100.modifiers[button.group];
    if ("cue_stop" in modifiers) {
        if (!value) {
            engine.setValue(button.group,"play",true);
            midi.sendShortMsg(0x90,button.midino,0x7f);
            delete modifiers["cue_stop"];
        }
    } else if (engine.getValue(button.group,"play")) {
        engine.setValue(button.group,"play",false);
        engine.setValue(button.group,"stop",true);
        midi.sendShortMsg(0x90,button.midino,0x0);
    } else {
        engine.setValue(button.group,"stop",false);
        engine.setValue(button.group,"play",true);
        midi.sendShortMsg(0x90,button.midino,0x7f);
    }
}

// Update looper related LEDs
HileVci100.updateLooperLEDs = function() {
    var group = HileVci100.activeLooperGroup;
    print("UPDATE LOOOPER LEDs");
    for (i=1;i<=4;i++) {
        var name = "beatloop_" + HileVci100.beatloopSizes[i];
        var buttonname = "loop_button_" + (i+3);
        var button = HileVci100.buttonMap[buttonname];
        if (button==undefined) {
            HileVci100.debug("BUTTON NOT FOUND " + buttonname);
            continue;
        }
        var status = (engine.getValue(group,name+"_enabled")) ? 0x7f : 0x0;
        HileVci100.debug("BEATLOOP " + button.midino +" "+status);
        midi.sendShortMsg(0x90,button.midino,status);
    }
    var button = HileVci100.buttonMap["loop1"];
    var status = (button.group==group) ? 0x7f : 0x0;
    midi.sendShortMsg(0x90,button.midino,status);

    var button = HileVci100.buttonMap["loop2"];
    var status = (button.group==group) ? 0x7f : 0x0;
    midi.sendShortMsg(0x90,button.midino,status);
}

HileVci100.active = function(button,value) {
    if (!value) return;
    if (button.group==HileVci100.activeLooperGroup)
        return;
    var modifiers = HileVci100.modifiers[button.group];
    HileVci100.activeLooperGroup = button.group;
    var group = HileVci100.activeLooperGroup;
    var inactive_group = (group=="[Channel1]") ? "[Channel2]" : "[Channel1]";
    HileVci100.debug("ACTIVE LOOPER: " + HileVci100.activeLooperGroup);
    for (i=1;i<=7;i++) {
        var button_name = "loop_button_" + i;
        var button = HileVci100.buttonMap[button_name];
        engine.connectControl(
            inactive_group, button.control, HileVci100.updateLooperLEDs, true
        );
        engine.connectControl(
            group, button.control, HileVci100.updateLooperLEDs
        );
    }
    HileVci100.updateLooperLEDs();
}
 
HileVci100.looper = function(button,value) {
    if (!value) {
        if (button.control!="reloop_exit") {
            engine.setValue(group,button.control,false);
        }
        return;
    }
    var modifiers = HileVci100.modifiers[button.group];
    var group = HileVci100.activeLooperGroup;
    HileVci100.debug("LOOPER: " + group+" "+ button.control);
    engine.setValue(group,button.control,true);

    // Activate LOOP button when we have active loop
    if (button.control=="loop_out") {
        var button = HileVci100.buttonMap["loop_button_3"];
        var status = (button.group==group) ? 0x7f : 0x0;
        midi.sendShortMsg(0x90,button.midino,status);
    } else if (button.control=="reloop_exit") {
        engine.setValue(group,"loop_in",false);
        engine.setValue(group,"loop_out",false);
    }
}

HileVci100.beatloop = function(button,value) {
    if (!value) return;
    var modifiers = HileVci100.modifiers[button.group];
    var group = HileVci100.activeLooperGroup;
    var control = button.control.replace('_enabled','_activate');
    HileVci100.debug("BEATLOOP: " + group+" "+ button.control);
    midi.sendShortMsg(0x90,button.midino,0x7f);
    engine.setValue(group,control,true);
}

HileVci100.scratchEnable = function(button,value) {
    var deck;
    if (button.group=="[Channel1]") deck=1;
    if (button.group=="[Channel2]") deck=2;
    if (value) {
        if (HileVci100.isScratchEnabled[button.group]) return;
        HileVci100.isScratchEnabled[button.group] = true;
        engine.scratchEnable(deck,
            HileVci100.scratchintervalsPerRev,
            HileVci100.scratchRPM,
            HileVci100.scratchAlpha,
            HileVci100.scratchBeta,
            HileVci100.rampedScratchEnable
        );
    } else {
        if (!HileVci100.isScratchEnabled[button.group]) 
            return;
        HileVci100.isScratchEnabled[button.group] = false;
        engine.scratchDisable(deck,HileVci100.rampedScratchDisable);
    }
}

HileVci100.jog_wheel = function(button,value) {
    var deck;
    if (HileVci100.isScratchEnabled[button.group]) {
        if (button.group=="[Channel1]") deck=1;
        if (button.group=="[Channel2]") deck=2;
        engine.scratchTick(deck,value);
    } else {
        engine.setValue(button.group,"jog",value);
    }
}


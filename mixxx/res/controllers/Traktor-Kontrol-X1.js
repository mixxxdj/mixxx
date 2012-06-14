
function KontrolX1() {};

KontrolX1.ledStates = { off: 0x0, on: 0x7f };
KontrolX1.leds = new Object();
KontrolX1.knobs = new Object();
KontrolX1.buttons = new Object();
KontrolX1.encoders = new Object();

// Value scaling functions for certain default mixxx ranges
KontrolX1.volume = function(value) {
    return script.absoluteNonLin(value, 0, 1, 5, 0, 127);
}
KontrolX1.eq = function(value) {
    return script.absoluteNonLin(value, 0, 1, 4, 0, 127);
}

// Define output LEDs
KontrolX1.addLED = function(index,group,control) {
    led_id = group+'.'+control;
    KontrolX1.leds[led_id] = {
        'id': led_id,
        'group': group,
        'control': control,
        'index': index,
    };
    engine.connectControl(group,control,KontrolX1.updateLEDState);
}

// Define named input button and engine control
KontrolX1.addButton = function(index,group,control,hold,callback) {
    KontrolX1.buttons[index] = {
        'id': group+'.'+control,
        'group': group,
        'control': control,
        'hold': hold,
        'callback': callback
    };
}

// Define named input knob and engine control
KontrolX1.addKnob = function(index,group,control,scaler,callback) {
    KontrolX1.knobs[index] = {
        'id': group+'.'+control,
        'group': group,
        'control': control,
        'scaler': scaler,
        'callback': callback
    };
}

// Defined named input encoder and engine control
KontrolX1.addEncoder = function(index,group,control,scaler,callback) {
    KontrolX1.encoders[index] = {
        'id': group+'.'+control,
        'group': group,
        'control': control,
        'scaler': scaler,
        'callback': callback
    };
}

KontrolX1.init = function(id) {
    KontrolX1.id = id;
    KontrolX1.timers = new Object();

    KontrolX1.addKnob(0x0,"[Channel1]","pregain",KontrolX1.volume);
    KontrolX1.addKnob(0x1,"[Channel2]","pregain",KontrolX1.volume);
    KontrolX1.addKnob(0x2,"[Channel1]","filterHigh",KontrolX1.eq);
    KontrolX1.addKnob(0x3,"[Channel2]","filterHigh",KontrolX1.eq);
    KontrolX1.addKnob(0x4,"[Channel1]","filterMid",KontrolX1.eq);
    KontrolX1.addKnob(0x5,"[Channel2]","filterMid",KontrolX1.eq);
    KontrolX1.addKnob(0x6,"[Channel1]","filterLow",KontrolX1.eq);
    KontrolX1.addKnob(0x7,"[Channel2]","filterLow",KontrolX1.eq);

    KontrolX1.addKnob(0x2c,"[Channel1]","mixer_1_shift");
    KontrolX1.addKnob(0x2d,"[Channel2]","mixer_1_shift");
    KontrolX1.addKnob(0x2e,"[Channel1]","mixer_2_shift");
    KontrolX1.addKnob(0x2f,"[Channel2]","mixer_2_shift");
    KontrolX1.addKnob(0x30,"[Channel1]","mixer_3_shift");
    KontrolX1.addKnob(0x31,"[Channel2]","mixer_3_shift");
    KontrolX1.addKnob(0x32,"[Channel1]","mixer_4_shift");
    KontrolX1.addKnob(0x33,"[Channel2]","mixer_4_shift");

    KontrolX1.addButton(0x8,"[Channel1]","pfl");
    KontrolX1.addLED(0x8,"[Channel1]","pfl");
    KontrolX1.addButton(0x9,"[Channel2]","pfl");
    KontrolX1.addLED(0x9,"[Channel2]","pfl");
    KontrolX1.addButton(0xa,"[Channel1]","filterHighKill");
    KontrolX1.addLED(0xa,"[Channel1]","filterHighKill");
    KontrolX1.addButton(0xb,"[Channel2]","filterHighKill");
    KontrolX1.addLED(0xb,"[Channel2]","filterHighKill");
    KontrolX1.addButton(0xc,"[Channel1]","filterMidKill");
    KontrolX1.addLED(0xc,"[Channel1]","filterMidKill");
    KontrolX1.addButton(0xd,"[Channel2]","filterMidKill");
    KontrolX1.addLED(0xd,"[Channel2]","filterMidKill");
    KontrolX1.addButton(0xe,"[Channel1]","filterLowKill");
    KontrolX1.addLED(0xe,"[Channel1]","filterLowKill");
    KontrolX1.addButton(0xf,"[Channel2]","filterLowKill");
    KontrolX1.addLED(0xf,"[Channel2]","filterLowKill");

    KontrolX1.addButton(0x34,"[Channel1]","mixer_1_shift");
    KontrolX1.addLED(0x34,"[Channel1]","mixer_1_shift");
    KontrolX1.addButton(0x35,"[Channel2]","mixer_1_shift");
    KontrolX1.addLED(0x35,"[Channel2]","mixer_1_shift");
    KontrolX1.addButton(0x36,"[Channel1]","mixer_2_shift");
    KontrolX1.addLED(0x36,"[Channel1]","mixer_2_shift");
    KontrolX1.addButton(0x37,"[Channel2]","mixer_2_shift");
    KontrolX1.addLED(0x37,"[Channel2]","mixer_2_shift");
    KontrolX1.addButton(0x38,"[Channel1]","mixer_3_shift");
    KontrolX1.addLED(0x38,"[Channel1]","mixer_3_shift");
    KontrolX1.addButton(0x39,"[Channel2]","mixer_3_shift");
    KontrolX1.addLED(0x39,"[Channel2]","mixer_3_shift");
    KontrolX1.addButton(0x3a,"[Channel1]","mixer_4_shift");
    KontrolX1.addLED(0x3a,"[Channel1]","mixer_4_shift");
    KontrolX1.addButton(0x3b,"[Channel2]","mixer_4_shift");
    KontrolX1.addLED(0x3b,"[Channel2]","mixer_4_shift");

    KontrolX1.addEncoder(0x10,"[Channel1]","encoder_1");
    KontrolX1.addEncoder(0x11,"[Playlist]","SelectTrackKnob");
    KontrolX1.addButton(0x12,"[Channel1]","encoder_1_button");
    KontrolX1.addButton(0x13,"[Playlist]","LoadSelectedIntoFirstStopped",true);
    KontrolX1.addButton(0x18,"[Channel1]","encoder_2_button");
    KontrolX1.addButton(0x19,"[Channel2]","encoder_2_button");
    KontrolX1.addEncoder(0x1a,"[Channel1]","encoder_2");
    KontrolX1.addEncoder(0x1b,"[Channel2]","encoder_2");

    KontrolX1.addEncoder(0x3c,"[Channel1]","encoder_1_shift");
    KontrolX1.addEncoder(0x3d,"[Channel2]","encoder_1_shift");
    KontrolX1.addButton(0x3e,"[Channel1]","encoder_1_button_shift");
    KontrolX1.addButton(0x3f,"[Channel2]","encoder_1_button_shift");
    KontrolX1.addButton(0x44,"[Channel1]","encoder_2_button_shift");
    KontrolX1.addButton(0x45,"[Channel2]","encoder_2_button_shift");
    KontrolX1.addEncoder(0x46,"[Channel1]","rate_encoder");
    KontrolX1.addEncoder(0x47,"[Channel2]","rate_encoder");

    KontrolX1.addButton(0x14,"[Channel1]","fx_1");
    KontrolX1.addLED(0x14,"[Channel1]","fx_1");
    KontrolX1.addButton(0x15,"[Channel2]","fx_1");
    KontrolX1.addLED(0x15,"[Channel2]","fx_1");
    KontrolX1.addButton(0x16,"[Channel1]","fx_2");
    KontrolX1.addLED(0x16,"[Channel1]","fx_2");
    KontrolX1.addButton(0x17,"[Channel2]","fx_2");
    KontrolX1.addLED(0x17,"[Channel2]","fx_2");

    KontrolX1.addButton(0x1c,"[Channel1]","loop_in");
    KontrolX1.addLED(0x1c,"[Channel1]","loop_in");
    KontrolX1.addButton(0x1d,"[Channel2]","loop_in");
    KontrolX1.addLED(0x1d,"[Channel2]","loop_in");
    KontrolX1.addButton(0x1e,"[Channel1]","loop_out");
    KontrolX1.addLED(0x1e,"[Channel1]","loop_out");
    KontrolX1.addButton(0x1f,"[Channel2]","loop_out");
    KontrolX1.addLED(0x1f,"[Channel2]","loop_out");
    KontrolX1.addButton(0x20,"[Channel1]","nudge_down",true,KontrolX1.nudge);
    KontrolX1.addLED(0x20,"[Channel1]","nudge_down",true,KontrolX1.nudge);
    KontrolX1.addButton(0x21,"[Channel2]","nudge_down",true,KontrolX1.nudge);
    KontrolX1.addLED(0x21,"[Channel2]","nudge_down",true,KontrolX1.nudge);
    KontrolX1.addButton(0x22,"[Channel1]","nudge_up",true,KontrolX1.nudge);
    KontrolX1.addLED(0x22,"[Channel1]","nudge_up",true,KontrolX1.nudge);
    KontrolX1.addButton(0x23,"[Channel2]","nudge_up",true,KontrolX1.nudge);
    KontrolX1.addLED(0x23,"[Channel2]","nudge_up",true,KontrolX1.nudge);
    KontrolX1.addButton(0x24,"[Channel1]","cue_default",true);
    KontrolX1.addLED(0x24,"[Channel1]","cue_default");
    KontrolX1.addButton(0x25,"[Channel2]","cue_default",true);
    KontrolX1.addLED(0x25,"[Channel2]","cue_default");
    KontrolX1.addButton(0x26,"[Channel1]","reloop_exit",true);
    KontrolX1.addLED(0x26,"[Channel1]","reloop_exit",true);
    KontrolX1.addButton(0x27,"[Channel2]","reloop_exit",true);
    KontrolX1.addLED(0x27,"[Channel2]","reloop_exit",true);
    KontrolX1.addButton(0x28,"[Channel1]","play");
    KontrolX1.addLED(0x28,"[Channel1]","play");
    KontrolX1.addButton(0x29,"[Channel2]","play");
    KontrolX1.addLED(0x29,"[Channel2]","play");
    KontrolX1.addButton(0x2a,"[Channel1]","sync");
    KontrolX1.addLED(0x2a,"[Channel1]","sync");
    KontrolX1.addButton(0x2b,"[Channel2]","sync");
    KontrolX1.addLED(0x2b,"[Channel2]","sync");

    KontrolX1.addButton(0x40,"[Channel1]","fx_1_shift");
    KontrolX1.addLED(0x40,"[Channel1]","fx_1_shift");
    KontrolX1.addButton(0x41,"[Channel2]","fx_1_shift");
    KontrolX1.addLED(0x41,"[Channel2]","fx_2_shift");
    KontrolX1.addButton(0x42,"[Channel1]","fx_2_shift");
    KontrolX1.addLED(0x42,"[Channel1]","fx_2_shift");
    KontrolX1.addButton(0x43,"[Channel2]","fx_2_shift");
    KontrolX1.addLED(0x43,"[Channel2","fx_2_shift");

    KontrolX1.addButton(0x48,"[Channel1]","tempo_minus");
    KontrolX1.addLED(0x48,"[Channel1]","tempo_minus");
    KontrolX1.addButton(0x49,"[Channel2]","tempo_plus");
    KontrolX1.addLED(0x49,"[Channel2]","tempo_minus");
    KontrolX1.addButton(0x4a,"[Channel1]","tempo_plus");
    KontrolX1.addLED(0x4a,"[Channel1]","tempo_plus");
    KontrolX1.addButton(0x4b,"[Channel2]","tempo_plus");
    KontrolX1.addLED(0x4b,"[Channel2]","tempo_plus");
    KontrolX1.addButton(0x4c,"[Channel1]","previous");
    KontrolX1.addLED(0x4c,"[Channel1]","previous");
    KontrolX1.addButton(0x4d,"[Channel2]","previous");
    KontrolX1.addLED(0x4d,"[Channel2]","previous");
    KontrolX1.addButton(0x4e,"[Channel1]","next");
    KontrolX1.addLED(0x4e,"[Channel1]","next");
    KontrolX1.addButton(0x4f,"[Channel2]","next");
    KontrolX1.addLED(0x4f,"[Channel2]","next");
    KontrolX1.addButton(0x50,"[Channel1]","cue_rel_shift");
    KontrolX1.addLED(0x50,"[Channel1]","cue_rel_shift");
    KontrolX1.addButton(0x51,"[Channel2]","cue_rel_shift");
    KontrolX1.addLED(0x51,"[Channel2]","cue_rel_shift");
    KontrolX1.addButton(0x52,"[Channel1]","reloop_exit_shift");
    KontrolX1.addLED(0x52,"[Channel1]","reloop_exit_shift");
    KontrolX1.addButton(0x53,"[Channel2]","reloop_exit_shift");
    KontrolX1.addLED(0x53,"[Channel2]","reloop_exit_shift");
    KontrolX1.addButton(0x54,"[Channel1]","keylock");
    KontrolX1.addLED(0x54,"[Channel1]","keylock");
    KontrolX1.addButton(0x55,"[Channel2]","keylock");
    KontrolX1.addLED(0x55,"[Channel2]","keylock");
    KontrolX1.addButton(0x56,"[Channel1]","tap");
    KontrolX1.addLED(0x56,"[Channel1]","tap");
    KontrolX1.addButton(0x57,"[Channel2]","tap");
    KontrolX1.addLED(0x57,"[Channel2]","tap");

    KontrolX1.resetLEDs();
    print("Kontrol X1 " + KontrolX1.id + " initialized");
}

KontrolX1.shutdown = function() {
    print("Kontrol X1 " + KontrolX1.id + " shutdown");
}

// Set all LEDs to disabled state, both normal and shifted mode
KontrolX1.resetLEDs = function() {
    var led;
    for (var led_name in KontrolX1.leds) {
        print("LED " + led_name);
        led = KontrolX1.leds[led_name];
        midi.sendShortMsg(0xb0,led.index,KontrolX1.ledStates["off"]);
        midi.sendShortMsg(0xb0,led.index+0x2c,KontrolX1.ledStates["off"]);
    }
}
KontrolX1.updateLEDState = function(value,group,key) {
    var led_id = group+'.'+key;
    var led = KontrolX1.leds[led_id];
    if (led==undefined) {
        print("Unknown LED to update: " + led_id);
        return;
    }
    midi.sendShortMsg(0xb0,led.index,value);
    print("Update LED " +group+'.'+key+ " value " + value);
}

// Button callback for nudge buttons
KontrolX1.nudge = function(button,value) {
    print("NUDGE " + button.id + " value " + value);
}

// Callback from MIDI mapping for all knobs in device
KontrolX1.knobs = function(channel,control,value,status,group) {
    var knob = KontrolX1.knobs[control];
    var shift = (control>=0x2c) ? true : false;
    if (knob==undefined) {
        print("Knob not defined in mapping: " + control);
        return;
    }
    if (knob.scaler==undefined) {
        print("No valid scaling function registered for " + knob.id); 
        return;
    }   
    if (knob.callback!=undefined) {
        knob.callback(channel,control,value,status,group);
        return;
    }
    value = knob.scaler(value);
    print("KNOB " + knob.id + " value " + value + " shift " + shift);
    engine.setValue(knob.group,knob.control,value);
}

// Callback from MIDI mapping for all encoders in device
KontrolX1.encoders = function(channel,control,value,status,group) {
    var encoder = KontrolX1.encoders[control];
    var shift = (control>=0x2c) ? true : false;

    if (encoder==undefined) { 
        print("ENCODER not defined in mapping: " + control);
        return;
    }
    // Encoders in F1 send 127 for -1, 1 for +1
    value = (value==127)?value=-1:value=+1;
    if (encoder.scaler!=undefined) 
        value = encoder.scaler(value);
    if (encoder.callback!=undefined) {
        encoder.callback(channel,control,value,status,group);
        return;
    }
    engine.setValue(encoder.group,encoder.control,value);
    print("ENCODER " + encoder.id + " value " + value + " shift " + shift);
}

// Callback from MIDI mapping for all buttons in device
KontrolX1.buttons = function(channel,control,value,status,group) {
    var button = KontrolX1.buttons[control];
    var shift = (control>=0x2c) ? true : false;

    if (button==undefined) {
        print("BUTTON not defined in mapping: " + control);
        return;
    }
    if (button.callback!=undefined) {
        button.callback(button,value);
        return;
    }
    if (!button.hold && value==0)
        return;

    if (button.control=="play") {
        if (engine.getValue(button.group,'play'))
            engine.setValue(button.group,'stop',true);
        else
            engine.setValue(button.group,'play',true);
        return;
    }

    if (!button.hold)
        value = engine.getValue(button.group,button.control) ? false : true;
    print("BUTTON" + button.id + " value " + value + " shift " + shift);
    engine.setValue(button.group,button.control,value);
}


//
// Pioneer CDJ cd player HID script v0.01
// Copyright (C) 2012, Ilkka Tuohela, Sean M. Pappalardo
// but feel free to tweak this to your heart's content!
// For Mixxx version 1.11.x
//

PioneerCDJHID = new HIDController();

// Valid values: 1 for mouse mode, 0 for xy-pad mode
PioneerCDJHID.LEDUpdateInterval = undefined;
PioneerCDJHID.version_major = undefined;
PioneerCDJHID.version_minor = undefined;

// Timers and toggles
PioneerCDJHID.deckSwitchClicked = false;
PioneerCDJHID.deckSwitchClickTimer = undefined;

PioneerCDJHID.LEDColors = { off: 0x0, on: 0x01 }

PioneerCDJHID.ignoredControlChanges = [ ];
PioneerCDJHID.WheelLEDCount = 0;
PioneerCDJHID.ButtonLEDCount = 22;

PioneerCDJHID.activeDeck = 1;

// Scratch parameters
PioneerCDJHID.scratchintervalsPerRev = 256;
PioneerCDJHID.scratchAlpha = 1.0/8;
PioneerCDJHID.rampedScratchEnable = true;
PioneerCDJHID.rampedScratchEnable = true;

//
// Functions to modify be end user
//

// Initialize device state
PioneerCDJHID.init = function (id) {
    PioneerCDJHID.id = id;
    PioneerCDJHID.updateLEDs();

    PioneerCDJHID.registerInputPackets();
    PioneerCDJHID.registerOutputPackets();
    PioneerCDJHID.registerScalers();
    PioneerCDJHID.registerCallbacks();

    PioneerCDJHID.toggleButtons = [ 'play', 'keylock' ]

    if (PioneerCDJHID.LEDUpdateInterval!=undefined) {
        PioneerCDJHID.LEDTimer = engine.beginTimer(
            PioneerCDJHID.LEDUpdateInterval,
            "PioneerCDJHID.updateLEDs(true)"
        );
    }
}

// Device cleanup function
PioneerCDJHID.shutdown = function() {
    if (EksOtus.LEDTimer!=undefined) {
        engine.stopTimer(EksOtus.LEDTimer);
        EksOtus.LEDTimer = undefined;
    }
    EksOtus.setLEDControlMode(2);
    EksOtus.setTrackpadMode(1);
    script.HIDDebug("EKS "+EksOtus.id+" shut down");
}

// Mandatory default handler for incoming packets
PioneerCDJHID.incomingData = function(data,length) {
    PioneerCDJHID.parsePacket(data,length);
}

// Register value scaling functions
PioneerCDJHID.registerScalers = function() {
    // Register functions to scale value
    PioneerCDJHID.registerScalingFunction('crossfader',PioneerCDJHID.plusMinus1Scaler);
    PioneerCDJHID.registerScalingFunction('jog',PioneerCDJHID.jogScaler);
    PioneerCDJHID.registerScalingFunction('jog_scratch',PioneerCDJHID.jogScratchScaler);
}

// Register input/output field callback functions
PioneerCDJHID.registerCallbacks = function() {
    // PioneerCDJHID.registerInputCallback('control','hid','deck_switch',PioneerCDJHID.deckSwitch);
    // PioneerCDJHID.registerInputCallback('control','[Master]','beat_align',PioneerCDJHID.beat_align);

    // PioneerCDJHID.registerInputCallback('control','deck','slider_scale',PioneerCDJHID.pitchSlider);
    // PioneerCDJHID.registerInputCallback('control','deck','slider_value',PioneerCDJHID.pitchSlider);
    // PioneerCDJHID.registerInputCallback('control','deck','slider_position',PioneerCDJHID.pitchSlider);
    // PioneerCDJHID.registerInputCallback('control','deck','slider_pos_1',PioneerCDJHID.pitchSlider);
    // PioneerCDJHID.registerInputCallback('control','deck','slider_pos_2',PioneerCDJHID.pitchSlider);

    // PioneerCDJHID.registerInputCallback('control','deck','beatloop_1',PioneerCDJHID.beatloop);
    // PioneerCDJHID.registerInputCallback('control','deck','beatloop_2',PioneerCDJHID.beatloop);
    // PioneerCDJHID.registerInputCallback('control','deck','beatloop_4',PioneerCDJHID.beatloop);
    // PioneerCDJHID.registerInputCallback('control','deck','beatloop_8',PioneerCDJHID.beatloop);

}

// Jog wheel seek event scaler
PioneerCDJHID.jogScaler = function(group,name,value) {
    return value/256*3;
}

// Jog wheel scratch event scaler
PioneerCDJHID.jogScratchScaler = function(group,name,value) {
    if (engine.getValue(group,'play')) {
        if (value>0) return 1;
        else return -1;
    } else {
        // TODO - do different scaling for stopped scratching
        if (value>0) return 1;
        else return -1;
    }
}

// Volume slider scaling for 0..1..5 scaling
PioneerCDJHID.volumeScaler = function(group,name,value) {
    return script.absoluteNonLin(value, 0, 1, 5, 0, 65536);
}

// Generic unsigned short to -1..0..1 range scaling
PioneerCDJHID.plusMinus1Scaler = function(group,name,value) {
    if (value<32768)
        return value/32768-1;
    else
        return (value-32768)/32768;
}

// Hotcues activated with normal press, cleared with shift
PioneerCDJHID.hotcue = function (field) {
    var command;
    if (field.value==PioneerCDJHID.buttonStates.released)
        return;
    if (PioneerCDJHID.activeDeck==undefined)
        return;

    var active_group = PioneerCDJHID.resolveGroup(field.group);
    if (PioneerCDJHID.modifiers['shift']) {
        command = field.name + '_clear';
    } else {
        command = field.name + '_activate';
    }
    // print ("HOTCUE group " + active_group + " name " + field.name + " value " + command);
    engine.setValue(active_group,command,true);
}


// Beatloops activated with normal presses to beatloop_1 - beatloop_8
PioneerCDJHID.beatloop = function (field) {
    var command;
    if (field.value==PioneerCDJHID.buttonStates.released)
        return;
    if (PioneerCDJHID.activeDeck==undefined)
        return;
    var active_group = PioneerCDJHID.resolveGroup(field.group);
    command = field.name + '_activate';
    engine.setValue(active_group,command,true);
}

PioneerCDJHID.beat_align = function (field) {
    if (PioneerCDJHID.activeDeck==undefined)
        return;
    var active_group = PioneerCDJHID.resolveGroup(field.group);
    if (PioneerCDJHID.modifiers['shift']) {
        // if (field.value==PioneerCDJHID.buttonStates.released) return;
        engine.setValue(active_group,'beats_translate_curpos',field.value);
    } else {
        engine.setValue(active_group,'quantize',field.value);
    }
}

// Pitch slider modifies track speed directly
// TODO - make this relative to the touch position
PioneerCDJHID.pitchSlider = function (field) {
    if (PioneerCDJHID.activeDeck==undefined)
        return;
    if (PioneerCDJHID.modifiers['pitchslider']) {
        var active_group = PioneerCDJHID.resolveGroup(field.group);
        if (field.name=='slider_position') {
            if (field.value==0)
                return;
            var value = PioneerCDJHID.plusMinus1Scaler(
                active_group,field.name,field.value
            );
            // print ("PITCH group " + active_group + " name " + field.name + " value " + value);
            engine.setValue(active_group,'rate',value);
        }
    }
}

// Use pregain if modifier shift is active, volume otherwise
PioneerCDJHID.pregain = function (field) {
    if (PioneerCDJHID.activeDeck==undefined)
        return;
    var active_group = PioneerCDJHID.resolveGroup(field.group);
    var value;
    if (PioneerCDJHID.modifiers['shift']) {
        value = script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536);
        engine.setValue(active_group,'pregain',value);
    } else {
        value = field.value / 65536;
        engine.setValue(active_group,'volume',value);
    }
}

// Function called when the special 'Deck Switch' button is pressed
PioneerCDJHID.deckSwitch = function(field) {
    if (PioneerCDJHID.initialized==false)
        return;
    if (field.value == PioneerCDJHID.buttonStates.released) {
        if (PioneerCDJHID.deckSwitchClicked==false) {
            PioneerCDJHID.deckSwitchClicked=true;
            PioneerCDJHID.deckSwitchClickTimer = engine.beginTimer(
                250,"PioneerCDJHID.deckSwitchClickedClear()"
            );
        } else {
            PioneerCDJHID.deckSwitchDoubleClick();
        }
    }
    // TODO - add code for 'hold deck_switch and press hot_cue[1-4] to select deck 1-4
}

// Timer to clear the double click status for deck switch
PioneerCDJHID.deckSwitchClickedClear = function() {
    PioneerCDJHID.deckSwitchClicked = false;
    if (PioneerCDJHID.deckSwitchClickTimer!=undefined) {
        engine.stopTimer(PioneerCDJHID.deckSwitchClickTimer);
        PioneerCDJHID.deckSwitchClickTimer = undefined;
    }
}

// Function to handle case when 'deck_switch' button was double clicked
PioneerCDJHID.deckSwitchDoubleClick = function() {
    PioneerCDJHID.deckSwitchClicked = false;
    if (PioneerCDJHID.deckSwitchClickTimer!=undefined) {
        engine.stopTimer(PioneerCDJHID.deckSwitchClickTimer);
        PioneerCDJHID.deckSwitchClickTimer = undefined;
    }
    if (PioneerCDJHID.activeDeck!=undefined)
        PioneerCDJHID.disconnectDeckLEDs();
    switch (PioneerCDJHID.activeDeck) {
        case 1:
            PioneerCDJHID.activeDeck = 2;
            PioneerCDJHID.setLED('hid','deck_switch','green');
            break;
        case 2:
            PioneerCDJHID.activeDeck = 1;
            PioneerCDJHID.setLED('hid','deck_switch','red');
            break;
        case 3:
            PioneerCDJHID.activeDeck = 4;
            PioneerCDJHID.setLED('hid','deck_switch','green');
            break;
        case 4:
            PioneerCDJHID.activeDeck = 3;
            PioneerCDJHID.setLED('hid','deck_switch','red');
            break;
        case undefined:
            PioneerCDJHID.activeDeck = 1;
            PioneerCDJHID.setLED('hid','deck_switch','red');
            break;
    }
    PioneerCDJHID.connectDeckLEDs();
    this.updateActiveDeckLEDs();
    script.HIDDebug('Active EKS Otus deck now ' + PioneerCDJHID.activeDeck);
}

PioneerCDJHID.activeLEDUpdateWrapper = function() {
    PioneerCDJHID.updateActiveDeckLEDs();
}

// Initialize control fields, buttons and LEDs
// Group name 'deck' is dynamically modified to active deck, since
// Otus is a dualdeck controller.
PioneerCDJHID.registerInputPackets = function() {
    var packet = undefined;
    var name = undefined;
    var offset = 0;

    // addControl(group,name,offset,pack,bitmask,isEncoder,callback)
    // Input controller state packet - 'control' is a special name
    packet = new HIDPacket('control',[],20);
    packet.addControl('deck','previous',0,'B',2);
    packet.addControl('deck','next',0,'B',3);
    packet.addControl('deck','seek_back',0,'B',4);
    packet.addControl('deck','seek_forward',0,'B',5);
    packet.addControl('deck','cue',0,'B',6);
    packet.addControl('deck','play',0,'B',7);

    packet.addControl('deck','reloop_exit',1,'B',5);
    packet.addControl('deck','cue_out',1,'B',6);
    packet.addControl('deck','cue_in',1,'B',7);

    packet.addControl('deck','tempo_mode',2,'B',3);
    packet.addControl('deck','cue_delete',2,'B',4);
    packet.addControl('deck','cue_previous',2,'B',5);
    packet.addControl('deck','cue_next',2,'B',6);
    packet.addControl('deck','cue_memory',2,'B',7);

    packet.addControl('deck','jog_mode',3,'B',1);
    packet.addControl('deck','master_tempo',3,'B',3);
    packet.addControl('deck','tempo',3,'B',4);
    packet.addControl('deck','browse_push',3,'B',5);

    packet.addControl('deck','beat_select',4,'B',3);
    packet.addControl('deck','hotcue_16',4,'B',4);
    packet.addControl('deck','hotcue_8',4,'B',5);
    packet.addControl('deck','hotcue_4',4,'B',6);
    packet.addControl('deck','hotcue_2',4,'B',7);

    packet.addControl('deck','reverse',7,'B',0);
    packet.addControl('deck','jog_touch_1',7,'B',5);
    packet.addControl('deck','jog_touch_2',7,'B',6);
    packet.addControl('deck','jog_touch_3',7,'B',7);

    packet.addControl('deck','vinyl_speed',8,'B');
    packet.addControl('deck','browse',10,'B');
    packet.addControl('deck','pitch',12,'H');
    packet.addControl('deck','jog_wheel',14,'I');

    // Adjust minimum deltas from unstable potentiometers
    packet.setMinDelta('deck','jog_wheel',4);
    PioneerCDJHID.registerInputPacket(packet);
}

// Register output packets we send to the controller
PioneerCDJHID.registerOutputPackets = function() {
    var packet = undefined;
    var name = undefined;
    var offset = 0;

    // Control packet for button LEDs
    packet = new HIDPacket('button_leds',[],20);

    packet.addLED('deck','previous',0,'B',2);
    packet.addLED('deck','next',0,'B',3);
    packet.addLED('deck','cue',0,'B',6);
    packet.addLED('deck','cue',0,'B',7);
    PioneerCDJHID.registerOutputPacket(packet);

}


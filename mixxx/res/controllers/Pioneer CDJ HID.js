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

PioneerCDJHID.beatLoopSizeMap = { 
    beatloop_2: '8', beatloop_2_shift: '0.5',
    beatloop_4: '4', beatloop_4_shift: '0.25',
    beatloop_8: '2', beatloop_8_shift: '0.125',
    beatloop_16: '1', beatloop_16_shift: '0.0625'
}

PioneerCDJHID.ignoredControlChanges = [ ];
PioneerCDJHID.WheelLEDCount = 0;
PioneerCDJHID.ButtonLEDCount = 22;

PioneerCDJHID.activeDeck = 1;

// Scratch parameters
PioneerCDJHID.scratchintervalsPerRev = 1024;
PioneerCDJHID.scratchAlpha = 1.0/8;
PioneerCDJHID.rampedScratchEnable = true;
PioneerCDJHID.rampedScratchEnable = true;

//
// Functions to modify be end user
//

// Initialize device state
PioneerCDJHID.init = function (id) {
    PioneerCDJHID.id = id;

    PioneerCDJHID.registerInputPackets();
    PioneerCDJHID.registerOutputPackets();
    PioneerCDJHID.registerScalers();
    PioneerCDJHID.registerCallbacks();

    PioneerCDJHID.initializeHIDMode();
    PioneerCDJHID.updateLEDs();

    PioneerCDJHID.toggleButtons = [ 'play', 'keylock', 'pfl' ]
    engine.softTakeover('[Channel1]','pregain',true);
    engine.softTakeover('[Channel2]','pregain',true);

    if (PioneerCDJHID.LEDUpdateInterval!=undefined) {
        PioneerCDJHID.LEDTimer = engine.beginTimer(
            PioneerCDJHID.LEDUpdateInterval,
            "PioneerCDJHID.updateLEDs(true)"
        );
    }
}

// Device cleanup function
PioneerCDJHID.shutdown = function() {
    if (PioneerCDJHID.LEDTimer!=undefined) {
        engine.stopTimer(PioneerCDJHID.LEDTimer);
        PioneerCDJHID.LEDTimer = undefined;
    }
    script.HIDDebug("Pioneer CDJ Deck "+PioneerCDJHID.id+" shut down");
}

PioneerCDJHID.initializeHIDMode = function() {
    var packet = PioneerCDJHID.OutputPackets['request_hid_mode'];
    script.HIDDebug("Sending HID mode init packet");
    packet.send();
}

// Mandatory default handler for incoming packets
PioneerCDJHID.incomingData = function(data,length) {
    PioneerCDJHID.parsePacket(data,length);
}

PioneerCDJHID.ignore = function(field) { }
PioneerCDJHID.dump = function(field) {
    script.HIDDebug(
        'Field  ' + field.id 
        + ' value ' + field.value 
        + ' delta ' + field.delta
    );
}

// Register value scaling functions
PioneerCDJHID.registerScalers = function() {
    // Register functions to scale value
    PioneerCDJHID.registerScalingFunction('rate',PioneerCDJHID.pitchScaler);
    PioneerCDJHID.registerScalingFunction('pregain',PioneerCDJHID.pregainScaler);
    PioneerCDJHID.registerScalingFunction('jog',PioneerCDJHID.jogScaler);
    PioneerCDJHID.registerScalingFunction('jog_scratch',PioneerCDJHID.jogPositionDelta);
}

// Register input/output field callback functions
PioneerCDJHID.registerCallbacks = function() {
    PioneerCDJHID.registerInputCallback('control','hid','deck_switch',PioneerCDJHID.deckSwitch);
    PioneerCDJHID.registerInputCallback('control','deck','beatloop_16',PioneerCDJHID.beatloop);
    PioneerCDJHID.registerInputCallback('control','deck','beatloop_8',PioneerCDJHID.beatloop);
    PioneerCDJHID.registerInputCallback('control','deck','beatloop_4',PioneerCDJHID.beatloop);
    PioneerCDJHID.registerInputCallback('control','deck','beatloop_2',PioneerCDJHID.beatloop);
    PioneerCDJHID.registerInputCallback('control','deck','jog_direction',PioneerCDJHID.ignore);
    PioneerCDJHID.registerInputCallback('control','deck','jog_move',PioneerCDJHID.ignore);
    PioneerCDJHID.registerInputCallback('control','deck','jog_ticks',PioneerCDJHID.ignore);
    PioneerCDJHID.registerInputCallback('control','deck','needle_search',PioneerCDJHID.dump);

}

// Jog wheel seek event scaler
PioneerCDJHID.jogScaler = function(group,name,value) {
    return value/6;
}

// Jog wheel scratch event scaler
PioneerCDJHID.jogPositionDelta = function(group,name,value) {
    return value/3; 
}

// Pitch on CDJ sends -1000 to 1000, reset at 0, swap direction
PioneerCDJHID.pitchScaler = function(gruop,name,value) {
    return -(value/1000);
}

// Volume slider scaling for 0..1..5 scaling from 0-255
PioneerCDJHID.pregainScaler = function(group,name,value) {
    return script.absoluteNonLin(value, 0, 1, 5, 0, 256);
}

// Generic unsigned short to -1..0..1 range scaling
PioneerCDJHID.plusMinus1Scaler = function(group,name,value) {
    if (value<32768)
        return value/32768-1;
    else
        return (value-32768)/32768;
}

PioneerCDJHID.seek_fwd = function (field) {
    var active_group = PioneerCDJHID.resolveGroup(field.group);
    if (field.value==PioneerCDJHID.buttonStates.released)
        engine.setValue(active_group,'fwd',false);
    if (field.value==PioneerCDJHID.buttonStates.pressed)
        engine.setValue(active_group,'fwd',true);
}

// Hotcues activated with normal press, cleared with shift
PioneerCDJHID.beatloop = function (field) {
    var command;
    if (field.value==PioneerCDJHID.buttonStates.released)
        return;
    if (PioneerCDJHID.activeDeck==undefined)
        return;

    var active_group = PioneerCDJHID.resolveGroup(field.group);
    if (PioneerCDJHID.modifiers['beatloop_size']) {
        size = PioneerCDJHID.beatLoopSizeMap[field.name+'_shift'] ;
    } else {
        size = PioneerCDJHID.beatLoopSizeMap[field.name];
    }
    var command = 'beatloop_' + size + '_toggle';
    print ("BEATLOOP SIZE" + active_group + " name " + command);
    engine.setValue(active_group,command,true);
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
    script.HIDDebug('DECK SWITCH pressed');
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
    script.HIDDebug('Active CDJ deck now ' + PioneerCDJHID.activeDeck);
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
    packet.addControl('hid','deck_switch',0,'B',0x1);
    packet.addControl('[Playlist]','SelectPrevTrack',0,'B',0x4);

    packet.addControl('[Playlist]','SelectNextTrack',0,'B',0x8);
    packet.addControl('deck','back',0,'B',0x10);
    packet.addControl('deck','fwd',0,'B',0x20);
    packet.addControl('deck','cue_default',0,'B',0x40);
    packet.addControl('deck','play',0,'B',0x80);

    packet.addControl('deck','reloop_exit',1,'B',0x20);
    packet.addControl('deck','loop_out',1,'B',0x40);
    packet.addControl('deck','loop_in',1,'B',0x80);

    packet.addControl('deck','tempo_mode',2,'B',0x8);
    packet.addControl('deck','keylock',2,'B',0x10);
    packet.addControl('deck','loop_halve',2,'B',0x20);
    packet.addControl('deck','loop_double',2,'B',0x40);
    packet.addControl('deck','quantize',2,'B',0x80);

    packet.addControl('deck','pfl',3,'B',0x2);
    packet.addControl('deck','beatsync',3,'B',0x8);
    packet.addControl('deck','beats_translate_curpos',3,'B',0x10);
    packet.addControl('deck','LoadSelectedTrack',3,'B',0x20);

    packet.addControl('modifiers','beatloop_size',4,'B',0x8);
    packet.addControl('deck','beatloop_16',4,'B',0x10);
    packet.addControl('deck','beatloop_8',4,'B',0x20);
    packet.addControl('deck','beatloop_4',4,'B',0x40);
    packet.addControl('deck','beatloop_2',4,'B',0x80);

    packet.addControl('deck','reverse',7,'B',0x1);
    packet.addControl('deck','jog_touch',7,'B',0x20);
    packet.addControl('deck','jog_direction',7,'B',0x40);
    packet.addControl('deck','jog_move',7,'B',0x80);

    packet.addControl('deck','pregain',8,'B');
    packet.addControl('[Playlist]','SelectTrackKnob',10,'H',undefined,true);
    // 0xFC18 to 0x03E8
    packet.addControl('deck','rate',12,'h');
    packet.addControl('deck','jog_wheel',14,'h',undefined,true);
    packet.addControl('deck','jog_ticks',16,'h');

    packet.addControl('deck','needle_search',18,'h');

    // Adjust minimum deltas from unstable potentiometers
    // packet.setMinDelta('deck','pitch',4);
    // packet.setMinDelta('deck','jog_ticks',4);
    PioneerCDJHID.registerInputPacket(packet);
}

// Register output packets we send to the controller
PioneerCDJHID.registerOutputPackets = function() {
    var packet = undefined;
    var name = undefined;
    var offset = 0;

    // Control packet to initialize HID mode on CDJ
    packet = new HIDPacket('request_hid_mode',[0x1],0x20);
    packet.addControl('hid','mode',0,'B',1);
    PioneerCDJHID.registerOutputPacket(packet);

    // Control packet for button LEDs
    packet = new HIDPacket('button_leds',[],20);
    packet.addLED('deck','previous',0,'B',2);
    packet.addLED('deck','next',0,'B',3);
    packet.addLED('deck','cue',0,'B',6);
    PioneerCDJHID.registerOutputPacket(packet);

}


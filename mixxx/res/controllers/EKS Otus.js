//
// EKS Otus HID controller script v0.01
// Copyright (C) 2012, Sean M. Pappalardo, Ilkka Tuohela
// but feel free to tweak this to your heart's content!
// For Mixxx version 1.11.x
//

EksOtus = new HIDController();

// Valid values: 1 for mouse mode, 0 for xy-pad mode
EksOtus.trackpadMode = 0;
EksOtus.LEDUpdateInterval = 300;
EksOtus.version_major = undefined;
EksOtus.version_minor = undefined;

// Timers and toggles
EksOtus.deckSwitchClicked = false;
EksOtus.deckSwitchClickTimer = undefined;
EksOtus.initAnimationTimer = undefined;

EksOtus.LEDColors = { off: 0x0, red: 0x0f, green: 0xf0, amber: 0xff };
EksOtus.deckLEDColors = { 1: 'red', 2: 'green', 3: 'red', 4: 'green'}

EksOtus.ignoredControlChanges = [
    'mask','timestamp','packet_number','deck_status', 'wheel_position',
    // These return the Otus slider position scaled by the 'slider scale'
    'slider_pos_1','slider_pos_2', 'slider_value'
];
EksOtus.WheelLEDCount = 60;
EksOtus.ButtonLEDCount = 22;
EksOtus.SliderLedCount = 20;

// Dual deck controller, active deck is not selected by default
EksOtus.activeDeck = undefined;

// Scratch parameters
EksOtus.scratchintervalsPerRev = 256;
EksOtus.scratchAlpha = 1.0/8;
EksOtus.rampedScratchEnable = true;
EksOtus.rampedScratchEnable = true;

//
// Functions to modify be end user
//

// Initialize device state
EksOtus.init = function (id) {
    EksOtus.id = id;
    EksOtus.initializePacketData();
    EksOtus.updateLEDs();

    EksOtus.registerInputPackets();
    EksOtus.registerOutputPackets();
    EksOtus.registerScalers();
    EksOtus.registerCallbacks();

    EksOtus.toggleButtons = [ 'play', 'pfl', 'keylock',
        'filterLowKill','filterMidKill', 'filterHighKill'
    ]

    EksOtus.setTrackpadMode(EksOtus.trackpadMode);
    EksOtus.requestFirmwareVersion();
    if (EksOtus.LEDUpdateInterval!=undefined) {
        EksOtus.LEDTimer = engine.beginTimer(
            EksOtus.LEDUpdateInterval,
            "EksOtus.updateLEDs(true)"
        );
    }

}

// Device cleanup function
EksOtus.shutdown = function() {
    if (EksOtus.LEDTimer!=undefined) {
        engine.stopTimer(EksOtus.LEDTimer);
        EksOtus.LEDTimer = undefined;
    }
    EksOtus.setLEDControlMode(2);
    EksOtus.setTrackpadMode(1);
    script.HIDDebug("EKS "+EksOtus.id+" shut down");
}

// Mandatory default handler for incoming packets
EksOtus.incomingData = function(data,length) {
    EksOtus.parsePacket(data,length);
}

// Register value scaling functions
EksOtus.registerScalers = function() {
    // Register functions to scale value
    EksOtus.registerScalingFunction('crossfader',EksOtus.plusMinus1Scaler);
    EksOtus.registerScalingFunction('filterLow',EksOtus.eqScaler);
    EksOtus.registerScalingFunction('filterMid',EksOtus.eqScaler);
    EksOtus.registerScalingFunction('filterHigh',EksOtus.eqScaler);
    EksOtus.registerScalingFunction('jog',EksOtus.jogScaler);
    EksOtus.registerScalingFunction('jog_scratch',EksOtus.jogScratchScaler);
}

// Register input/output field callback functions
EksOtus.registerCallbacks = function() {
    EksOtus.registerInputCallback('control','hid','deck_switch',EksOtus.deckSwitch);
    EksOtus.registerInputCallback('control','hid','jog_ne',EksOtus.corner_wheel);
    EksOtus.registerInputCallback('control','hid','jog_sw',EksOtus.corner_wheel);
    EksOtus.registerInputCallback('control','hid','jog_nw',EksOtus.corner_wheel);
    EksOtus.registerInputCallback('control','deck1','pregain',EksOtus.pregain);
    EksOtus.registerInputCallback('control','deck2','pregain',EksOtus.pregain);
    EksOtus.registerInputCallback('control','[Master]','headphones',EksOtus.headphones);
    EksOtus.registerInputCallback('control','[Master]','beat_align',EksOtus.beat_align);


    EksOtus.registerInputCallback('control','deck','slider_scale',EksOtus.pitchSlider);
    EksOtus.registerInputCallback('control','deck','slider_value',EksOtus.pitchSlider);
    EksOtus.registerInputCallback('control','deck','slider_position',EksOtus.pitchSlider);
    EksOtus.registerInputCallback('control','deck','slider_pos_1',EksOtus.pitchSlider);
    EksOtus.registerInputCallback('control','deck','slider_pos_2',EksOtus.pitchSlider);

    EksOtus.registerInputCallback('control','[Effects]','trackpad_x',EksOtus.xypad);
    EksOtus.registerInputCallback('control','[Effects]','trackpad_y',EksOtus.xypad);
    EksOtus.registerInputCallback('control','[Effects]','trackpad_left',EksOtus.xypad);
    EksOtus.registerInputCallback('control','[Effects]','trackpad_right',EksOtus.xypad);
    EksOtus.registerInputCallback('control','[Effects]','touch_trackpad',EksOtus.xypad);

    EksOtus.registerInputCallback('control','deck','hotcue_1',EksOtus.hotcue);
    EksOtus.registerInputCallback('control','deck','hotcue_2',EksOtus.hotcue);
    EksOtus.registerInputCallback('control','deck','hotcue_3',EksOtus.hotcue);
    EksOtus.registerInputCallback('control','deck','hotcue_4',EksOtus.hotcue);
    EksOtus.registerInputCallback('control','deck','hotcue_5',EksOtus.hotcue);
    EksOtus.registerInputCallback('control','deck','hotcue_6',EksOtus.hotcue);

    EksOtus.registerInputCallback('control','deck','beatloop_1',EksOtus.beatloop);
    EksOtus.registerInputCallback('control','deck','beatloop_2',EksOtus.beatloop);
    EksOtus.registerInputCallback('control','deck','beatloop_4',EksOtus.beatloop);
    EksOtus.registerInputCallback('control','deck','beatloop_8',EksOtus.beatloop);

}

// Jog wheel seek event scaler
EksOtus.jogScaler = function(group,name,value) {
    return value/256*3;
}

// Jog wheel scratch event scaler
EksOtus.jogScratchScaler = function(group,name,value) {
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
EksOtus.volumeScaler = function(group,name,value) {
    return script.absoluteNonLin(value, 0, 1, 5, 0, 65536);
}

// EQ scaling function for 0..1..4 scaling
EksOtus.eqScaler = function(group,name,value) {
    return script.absoluteNonLin(value, 0, 1, 4, 0, 65536);
}

// Generic unsigned short to -1..0..1 range scaling
EksOtus.plusMinus1Scaler = function(group,name,value) {
    if (value<32768)
        return value/32768-1;
    else
        return (value-32768)/32768;
}

// Rotation of the Otus 'corner' wheels.
// Note right bottom wheel is library browser encoder and not handled here
EksOtus.corner_wheel = function(field) {
    // TODO - attach some functionality these corner wheels!
    print("CORNER " + field.name + " delta " + field.delta);
}

// Hotcues activated with normal press, cleared with shift
EksOtus.hotcue = function (field) {
    var command;
    if (field.value==EksOtus.buttonStates.released)
        return;
    if (EksOtus.activeDeck==undefined)
        return;
    var active_group = EksOtus.resolveGroup(field.group);
    if (EksOtus.modifiers['shift']) {
        command = field.name + '_clear';
    } else {
        command = field.name + '_activate';
    }
    // print ("HOTCUE group " + active_group + " name " + field.name + " value " + command);
    engine.setValue(active_group,command,true);
}


// Beatloops activated with normal presses to beatloop_1 - beatloop_8
EksOtus.beatloop = function (field) {
    var command;
    if (field.value==EksOtus.buttonStates.released)
        return;
    if (EksOtus.activeDeck==undefined)
        return;
    var active_group = EksOtus.resolveGroup(field.group);
    command = field.name + '_activate';
    engine.setValue(active_group,command,true);
}

EksOtus.beat_align = function (field) {
    if (EksOtus.activeDeck==undefined)
        return;
    var active_group = EksOtus.resolveGroup(field.group);
    if (EksOtus.modifiers['shift']) {
        // if (field.value==EksOtus.buttonStates.released) return;
        engine.setValue(active_group,'beats_translate_curpos',field.value);
    } else {
        engine.setValue(active_group,'quantize',field.value);
    }
}

// Pitch slider modifies track speed directly
// TODO - make this relative to the touch position
EksOtus.pitchSlider = function (field) {
    if (EksOtus.activeDeck==undefined)
        return;
    if (EksOtus.modifiers['pitchslider']) {
        var active_group = EksOtus.resolveGroup(field.group);
        if (field.name=='slider_position') {
            if (field.value==0)
                return;
            var value = EksOtus.plusMinus1Scaler(
                active_group,field.name,field.value
            );
            // print ("PITCH group " + active_group + " name " + field.name + " value " + value);
            engine.setValue(active_group,'rate',value);
        }
    }
}

// Use pregain if modifier shift is active, volume otherwise
EksOtus.pregain = function (field) {
    if (EksOtus.activeDeck==undefined)
        return;
    var active_group = EksOtus.resolveGroup(field.group);
    var value;
    if (EksOtus.modifiers['shift']) {
        value = script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536);
        engine.setValue(active_group,'pregain',value);
    } else {
        value = field.value / 65536;
        engine.setValue(active_group,'volume',value);
    }
}

// Use headphones volume, if modifier shift is active, pre/main mix otherwise
EksOtus.headphones = function (field) {

    if (EksOtus.modifiers['shift']) {
        value = script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536);
        engine.setValue(field.group,'headVolume',value);
    } else {
        value = EksOtus.plusMinus1Scaler(field.group,field.name,field.value);
        engine.setValue(field.group,'headMix',value);
    }
}

// Control effects or somethig with XY pad
EksOtus.xypad = function(field) {
    if (EksOtus.activeDeck==undefined)
        return;
    print ("XYPAD group " + field.group + " name " + field.name + " value " + field.value);
}

// Function called when the special 'Deck Switch' button is pressed
EksOtus.deckSwitch = function(field) {
    if (EksOtus.initialized==false)
        return;
    if (field.value == EksOtus.buttonStates.released) {
        if (EksOtus.deckSwitchClicked==false) {
            EksOtus.deckSwitchClicked=true;
            EksOtus.deckSwitchClickTimer = engine.beginTimer(
                250,"EksOtus.deckSwitchClickedClear()"
            );
        } else {
            EksOtus.deckSwitchDoubleClick();
        }
    }
    // TODO - add code for 'hold deck_switch and press hot_cue[1-4] to select deck 1-4
}

// Timer to clear the double click status for deck switch
EksOtus.deckSwitchClickedClear = function() {
    EksOtus.deckSwitchClicked = false;
    if (EksOtus.deckSwitchClickTimer!=undefined) {
        engine.stopTimer(EksOtus.deckSwitchClickTimer);
        EksOtus.deckSwitchClickTimer = undefined;
    }
}

// Function to handle case when 'deck_switch' button was double clicked
EksOtus.deckSwitchDoubleClick = function() {
    EksOtus.deckSwitchClicked = false;
    if (EksOtus.deckSwitchClickTimer!=undefined) {
        engine.stopTimer(EksOtus.deckSwitchClickTimer);
        EksOtus.deckSwitchClickTimer = undefined;
    }
    if (EksOtus.activeDeck!=undefined)
        EksOtus.disconnectDeckLEDs();
    switch (EksOtus.activeDeck) {
        case 1:
            EksOtus.activeDeck = 2;
            EksOtus.setLED('hid','deck_switch','green');
            break;
        case 2:
            EksOtus.activeDeck = 1;
            EksOtus.setLED('hid','deck_switch','red');
            break;
        case 3:
            EksOtus.activeDeck = 4;
            EksOtus.setLED('hid','deck_switch','green');
            break;
        case 4:
            EksOtus.activeDeck = 3;
            EksOtus.setLED('hid','deck_switch','red');
            break;
        case undefined:
            EksOtus.activeDeck = 1;
            EksOtus.setLED('hid','deck_switch','red');
            break;
    }
    EksOtus.connectDeckLEDs();
    this.updateActiveDeckLEDs();
    script.HIDDebug('Active EKS Otus deck now ' + EksOtus.activeDeck);
}

EksOtus.activeLEDUpdateWrapper = function() {
    EksOtus.updateActiveDeckLEDs();
}

// Silly little function for wheel LEDs to indicate device is initialized
// Triggers itself with a timer to reverse the LED states to off.
EksOtus.wheelLEDInitAnimation = function (state) {
    var i;
    var name = undefined;
    if (state=='off') {
        print("DISABLE wheel animation");
        if (EksOtus.initAnimationTimer!=undefined) {
            print("STOP wheel animation timer");
            engine.stopTimer(EksOtus.initAnimationTimer);
            EksOtus.initAnimationTimer = undefined;
        }
        for (i=EksOtus.WheelLEDCount;i>0;i--) {
            name = 'wheel_' + i;
            EksOtus.setLED('jog',name,state);
        }
    } else {
        print("ENABLE wheel animation");
        for (i=1;i<=EksOtus.WheelLEDCount;i++) {
            name = 'wheel_' + i;
            EksOtus.setLED('jog',name,state);
        }
        EksOtus.initAnimationTimer = engine.beginTimer(
            1000, "EksOtus.wheelLEDInitAnimation('off')"
        );
    }
}

// Initialize control fields, buttons and LEDs
// Group name 'deck' is dynamically modified to active deck, since
// Otus is a dualdeck controller.
EksOtus.registerInputPackets = function() {
    var packet = undefined;
    var name = undefined;
    var offset = 0;

    // addControl(group,name,offset,pack,bitmask,isEncoder,callback)
    // Input controller state packet - 'control' is a special name
    packet = new HIDPacket('control',[0x0,0x35],64);
    packet.addControl('deck','wheel_position',2,'H');
    packet.addControl('deck','jog_wheel',4,'h');
    packet.addControl('hid','timestamp',6,'I');
    packet.addControl('deck','slider_value',10,'H');
    packet.addControl('deck','slider_position',12,'H');
    packet.addControl('hid','jog_ne',14,'B',undefined,true);
    packet.addControl('[Playlist]','SelectTrackKnob',15,'B',undefined,true);
    packet.addControl('hid','jog_sw',16,'B',undefined,true);
    packet.addControl('hid','jog_nw',17,'B',undefined,true);
    packet.addControl('deck1','pregain',18,'H');
    packet.addControl('deck2','pregain',20,'H');
    packet.addControl('deck1','filterHigh',22,'H');
    packet.addControl('deck2','filterHigh',24,'H');
    packet.addControl('deck1','filterMid',26,'H');
    packet.addControl('deck2','filterMid',28,'H');
    packet.addControl('deck1','filterLow',30,'H');
    packet.addControl('deck2','filterLow',32,'H');
    packet.addControl('[Master]','crossfader',34,'H');
    packet.addControl('[Master]','headphones',36,'H');
    packet.addControl('[Effects]','trackpad_x',38,'H');
    packet.addControl('[Effects]','trackpad_y',40,'H');
    packet.addControl('deck','slider_pos_2',42,'H');
    packet.addControl('deck','slider_pos_1',44,'H');
    packet.addControl('hid','jog_ne_button',46,'I',0);
    packet.addControl('deck','beatloop_8',46,'I',1);
    packet.addControl('deck','beatloop_4',46,'I',2);
    packet.addControl('deck','beatloop_2',46,'I',3);
    packet.addControl('deck','beatloop_1',46,'I',4);
    packet.addControl('deck','loop_in',46,'I',5);
    packet.addControl('deck','loop_out',46,'I',6);
    packet.addControl('deck','reloop_exit',46,'I',7);
    packet.addControl('deck','slider_scale',46,'I',8);
    packet.addControl('deck','LoadSelectedTrack',46,'I',9);
    packet.addControl('modifiers','shift',46,'I',10);
    packet.addControl('hid','deck_switch',46,'I',11);
    packet.addControl('deck','pfl',46,'I',12);
    packet.addControl('hid','jog_sw_button',46,'I',13);
    packet.addControl('deck','filterLowKill',46,'I',14);
    packet.addControl('deck','play',46,'I',15);
    packet.addControl('deck','cue_default',46,'I',16);
    packet.addControl('deck','filterMidKill',46,'I',17);
    packet.addControl('deck','filterHighKill',46,'I',18);
    packet.addControl('deck','beat_align',46,'I',19);
    packet.addControl('hid','jog_nw_button',46,'I',20);
    packet.addControl('deck','jog_touch',46,'I',21);
    packet.addControl('hid','trackpad_left',46,'I',22);
    packet.addControl('hid','trackpad_right',46,'I',23);
    packet.addControl('deck','hotcue_1',46,'I',24);
    packet.addControl('deck','hotcue_2',46,'I',25);
    packet.addControl('deck','hotcue_3',46,'I',26);
    packet.addControl('deck','hotcue_4',46,'I',27);
    packet.addControl('deck','hotcue_5',46,'I',28);
    packet.addControl('deck','hotcue_6',46,'I',29);
    packet.addControl('modifiers','pitchslider',46,'I',30)
    packet.addControl('deck','touch_trackpad',46,'I',31);
    packet.addControl('hid','packet_number',51,'B');
    packet.addControl('hid','deck_status',52,'B');

    // Set certain packet fields as ignored
    packet.setIgnored('hid','timestamp',true);
    packet.setIgnored('hid','packet_number',true);
    packet.setIgnored('hid','deck_status',true);

    // Adjust minimum deltas from unstable potentiometers
    packet.setMinDelta('deck','jog_wheel',4);
    packet.setMinDelta('deck1','pregain',128);
    packet.setMinDelta('deck2','pregain',128);
    packet.setMinDelta('deck1','filterHigh',128);
    packet.setMinDelta('deck2','filterHigh',128);
    packet.setMinDelta('deck1','filterMid',128);
    packet.setMinDelta('deck2','filterMid',128);
    packet.setMinDelta('deck1','filterLow',128);
    packet.setMinDelta('deck2','filterLow',128);
    packet.setMinDelta('[Master]','crossfader',128);
    packet.setMinDelta('[Master]','headphones',128);
    EksOtus.registerInputPacket(packet);

    // Input packet to receive device firmware version
    packet = new HIDPacket('firmware_version',[0xa,0x4],64,EksOtus.FirmwareVersionResponse);
    packet.addControl('hid','major',2,'B');
    packet.addControl('hid','minor',3,'B');
    EksOtus.registerInputPacket(packet);

    // Input packet to receive trackpad mode change response
    packet = new HIDPacket('trackpad_mode',[0x5,0x3],64,EksOtus.TrackpadModeResponse);
    packet.addControl('hid','status',2,'B');
    EksOtus.registerInputPacket(packet);
}

// Register output packets we send to the controller
EksOtus.registerOutputPackets = function() {
    var packet = undefined;
    var name = undefined;
    var offset = 0;

    // Control packet for button LEDs
    packet = new HIDPacket('button_leds',[0x16,0x18],32);
    offset = 2;
    packet.addLED('hid','jog_ne',offset++,'B');
    packet.addLED('hid','jog_nw',offset++,'B');
    packet.addLED('[Playlist]','SelectTrackKnob',offset++,'B');
    packet.addLED('hid','jog_se',offset++,'B');
    packet.addLED('deck','beatloop_8_enabled',offset++,'B');
    packet.addLED('deck','beatloop_4_enabled',offset++,'B');
    packet.addLED('deck','beatloop_2_enabled',offset++,'B');
    packet.addLED('deck','beatloop_1_enabled',offset++,'B');
    packet.addLED('deck','loop_in',offset++,'B');
    packet.addLED('deck','loop_out',offset++,'B');
    packet.addLED('deck','reloop_exit',offset++,'B');
    packet.addLED('modifiers','shift',offset++,'B');
    packet.addLED('hid','deck_switch',offset++,'B');
    packet.addLED('hid','trackpad_right',offset++,'B');
    packet.addLED('hid','trackpad_left',offset++,'B');
    packet.addLED('deck','pfl',offset++,'B');
    packet.addLED('deck','filterLowKill',offset++,'B');
    packet.addLED('deck','play',offset++,'B');
    packet.addLED('deck','filterMidKill',offset++,'B');
    packet.addLED('deck','cue_default',offset++,'B');
    packet.addLED('deck','filterHighKill',offset++,'B');
    packet.addLED('deck','beats_translate_curpos',offset++,'B');
    EksOtus.registerOutputPacket(packet);

    // Slider LEDs
    packet = new HIDPacket('slider_leds',[0x17,0x16],32);
    offset = 2;
    packet.addLED('pitch',"slider_1",offset++,'B');
    packet.addLED('pitch',"slider_2",offset++,'B');
    packet.addLED('pitch',"slider_3",offset++,'B');
    packet.addLED('pitch',"slider_4",offset++,'B');
    packet.addLED('pitch',"slider_5",offset++,'B');
    packet.addLED('pitch',"slider_6",offset++,'B');
    packet.addLED('pitch',"slider_7",offset++,'B');
    packet.addLED('pitch',"slider_8",offset++,'B');
    packet.addLED('pitch',"slider_9",offset++,'B');
    packet.addLED('pitch',"slider_10",offset++,'B');
    packet.addLED('pitch',"slider_11",offset++,'B');
    packet.addLED('pitch',"slider_12",offset++,'B');
    packet.addLED('pitch',"slider_13",offset++,'B');
    packet.addLED('pitch',"slider_14",offset++,'B');
    packet.addLED('pitch',"slider_15",offset++,'B');
    packet.addLED('pitch',"slider_16",offset++,'B');
    packet.addLED('pitch',"slider_17",offset++,'B');
    packet.addLED('pitch',"slider_scale_1",offset++,'B');
    packet.addLED('pitch',"slider_scale_2",offset++,'B');
    packet.addLED('pitch',"slider_scale_3",offset++,'B');
    EksOtus.registerOutputPacket(packet);

    // Control packet for left wheel LEDs
    packet = new HIDPacket('led_wheel_left',[0x14,0x20],32);
    offset = 2;
    for (var led_index=1;led_index<=EksOtus.WheelLEDCount/2;led_index++) {
        var led_name = 'wheel_' + led_index;
        packet.addLED('jog',led_name,offset++,'B');
    }
    EksOtus.registerOutputPacket(packet);

    // Control packet for right wheel LEDs
    packet = new HIDPacket('led_wheel_right',[0x15,0x20],32);
    offset = 2;
    for (var led_index=EksOtus.WheelLEDCount/2+1;led_index<=EksOtus.WheelLEDCount;led_index++) {
        var led_name = 'wheel_' + led_index;
        packet.addLED('jog',led_name,offset++,'B');
    }
    EksOtus.registerOutputPacket(packet);

    // Output packet to request firmware version
    packet = new HIDPacket('request_firmware_version',[0xa,0x2],32);
    EksOtus.registerOutputPacket(packet);

    // Output packet to set trackpad mode
    packet = new HIDPacket('set_trackpad_mode',[0x5,0x3],32);
    packet.addControl('hid','mode',2,'B');
    EksOtus.registerOutputPacket(packet);

    // Output packet to set LED control mode
    packet = new HIDPacket('set_ledcontrol_mode',[0x1d,0x3],32);
    packet.addControl('hid','mode',2,'B');
    EksOtus.registerOutputPacket(packet);
}

// Otus specific output packet to request device firmware version
EksOtus.requestFirmwareVersion = function() {
    var packet = EksOtus.resolveOutputPacket('request_firmware_version');
    if (packet==undefined) {
        return;
    }
    script.HIDDebug("Requesting firmware version " + packet.name);
    packet.send();
}

// Firmware version response. Required to finish device INIT
EksOtus.FirmwareVersionResponse = function(packet,delta) {
    var field_major = packet.lookupField('hid','major');
    var field_minor = packet.lookupField('hid','minor');
    if (field_major==undefined) {
        script.HIDDebug("Error parsing field major from packet");
        return;
    }
    if (field_minor==undefined) {
        script.HIDDebug("Error parsing field minor from packet");
        return;
    }
    EksOtus.initialized=true;
    EksOtus.version_major = field_major.value;
    EksOtus.version_minor = field_minor.value;
    EksOtus.setLEDControlMode(1);

    EksOtus.updateLEDs(false);
    // Start blinking 'deck switch' button to indicate we are ready.
    EksOtus.setLEDBlink('hid','deck_switch','amber');
    script.HIDDebug("EKS " + EksOtus.id +
        " v"+EksOtus.version_major+"."+EksOtus.version_minor+
        " initialized"
    );
    // Indicate we are initialized with a little animation
    EksOtus.wheelLEDInitAnimation('amber');
}

// Otus specific output packet to set the trackpad control mode
EksOtus.setTrackpadMode = function(mode) {
    if (mode!=0 && mode!=1) {
        script.HIDDebug("Unsupported trackpad mode value: " + mode);
        return;
    }
    var packet = EksOtus.resolveOutputPacket('set_trackpad_mode');
    if (packet==undefined) {
        script.HIDDebug("Output not registered: set_trackpad_mode");
        return;
    }
    var field = packet.lookupField('hid','mode');
    if (field==undefined) {
        script.HIDDebug("EksOtus.setTrackpadMode error fetching field mode");
        return;
    }
    field.value = mode;
    packet.send();
}

// Response to above trackpad mode packet
EksOtus.TrackpadModeResponse = function(packet,delta) {
    field = packet.lookupField('hid','status');
    if (field==undefined) {
        script.HIDDebug("Error parsing field status from packet");
        return;
    }
    if (field.value==1) {
        script.HIDDebug("Trackpad mode successfully set");
    } else {
        script.HIDDebug("Trackpad mode change failed");
    }
}

// Set LED Control Mode on Otus firmware versions > 1.6. Major and minor must
// contain the version numbers for firmware as received from response.
// Valid modes are:
//      0   disable all LEDs
//      1   Re-enable LEDs
//      2   Revert to built-in light functionality
EksOtus.setLEDControlMode = function(mode) {
    if (EksOtus.version_major<=1 && EksOtus.version_minor<6) {
        // Firmware version does not support LED Control Mode Setting
        return;
    }
    if (mode!=0 && mode!=1 && mode!=2) {
        script.HIDDebug("Unknown value for LED Control Mode Setting: " + mode);
        return;
    }
    var packet = EksOtus.OutputPackets['set_ledcontrol_mode'];
    var field = packet.lookupField('hid','mode');
    if (field==undefined) {
        script.HIDDebug("EksOtus.setLEDControlMode error fetching field mode");
        return;
    }
    field.value = mode;
    packet.send();
}

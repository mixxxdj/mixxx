//
// EKS Otus HID controller script v0.01
// Copyright (C) 2012, Sean M. Pappalardo, Ilkka Tuohela
// but feel free to tweak this to your heart's content!
// For Mixxx version 1.11.x
//

EksOtus = new HIDController();

// Valid values: 1 for mouse mode, 0 for xy-pad mode

EksOtus.activeDeck = 1;
EksOtus.trackpadMode = 0;
EksOtus.LEDUpdateInterval = 300;
EksOtus.version_major = undefined;
EksOtus.version_minor = undefined;

// Timers and toggles
EksOtus.deckSwitchClicked = false;
EksOtus.deckSwitchClickTimer = undefined;
EksOtus.initAnimationTimer = undefined;

// Wheel spin animation details
EksOtus.activeTrackDuration = undefined;
// Group registered to update spinning platter details
EksOtus.activeSpinningPlatterGroup = undefined;
// Virtual record spin time, 1.8 for 33 1/3 RPM, 1.33 for 45 RPM
EksOtus.revTime = 1.8;
// Wheel LED index, range 1-60
EksOtus.activeSpinningPlatterLED = undefined;

EksOtus.LEDColors = { off: 0x0, red: 0x0f, green: 0xf0, amber: 0xff };
EksOtus.deckLEDColors = { 1: 'red', 2: 'green', 3: 'red', 4: 'green'}
EksOtus.deckSwitchMap = { 1: 2, 2: 1, 3: 4, 4: 3, undefined: 1 }

EksOtus.ignoredControlChanges = [
    'mask','timestamp','packet_number','deck_status', 'wheel_position',
    // These return the Otus slider position scaled by the 'slider scale'
    'slider_pos_1','slider_pos_2', 'slider_value'
];

// Scratch parameters
EksOtus.scratchintervalsPerRev = 512;
EksOtus.scratchAlpha = 1.0/8;
EksOtus.rampedScratchEnable = true;
EksOtus.rampedScratchEnable = true;

// Static variables for HID specs
EksOtus.wheelLEDCount = 60;
EksOtus.buttonLEDCount = 22;
EksOtus.sliderLEDCount = 20;

//
// Functions to modify be end user
//

// Initialize device state, send request for firmware. Otus is not
// usable before we receive a valid firmware version response.
EksOtus.init = function (id) {
    EksOtus.id = id;

    // Register controller details
    EksOtus.registerInputPackets();
    EksOtus.registerOutputPackets();
    EksOtus.registerScalers();
    EksOtus.registerCallbacks();

    EksOtus.toggleButtons = [ 
        'play', 'pfl', 'keylock', 
        'filterLowKill','filterMidKill', 'filterHighKill'
    ]

    EksOtus.updateLEDs();
    EksOtus.setTrackpadMode(EksOtus.trackpadMode);

    if (EksOtus.LEDUpdateInterval!=undefined) {
        EksOtus.LEDTimer = engine.beginTimer(
            EksOtus.LEDUpdateInterval,
            "EksOtus.updateLEDs(true)"
        );
    }

    engine.softTakeover('[Master]','headVolume',true);
    engine.softTakeover('[Master]','headMix',true);
    for (var deck in EksOtus.deckLEDColors) {
        engine.softTakeover('[Channel'+deck+']','pregain',true);
        engine.softTakeover('[Channel'+deck+']','volume',true);
    }

    // Note: Otus is not considered initialized before we get response
    // to this packet
    EksOtus.requestFirmwareVersion();

}

// Device cleanup function
EksOtus.shutdown = function() {
    if (EksOtus.LEDTimer!=undefined) {
        engine.stopTimer(EksOtus.LEDTimer);
        EksOtus.LEDTimer = undefined;
    }
    EksOtus.setLEDControlMode(2);
    EksOtus.setTrackpadMode(1);

    engine.softTakeover('[Master]','headVolume',false);
    engine.softTakeover('[Master]','headMix',false);
    for (var deck in EksOtus.deckLEDColors) {
        engine.softTakeover('[Channel'+deck+']','pregain',false);
        engine.softTakeover('[Channel'+deck+']','volume',false);
    }

    script.HIDDebug("EKS "+EksOtus.id+" shut down");
}

// Mandatory default handler for incoming packets
EksOtus.incomingData = function(data,length) {
    EksOtus.parsePacket(data,length);
}

// Mandatory LED update callback handler
EksOtus.activeLEDUpdateWrapper = function() {
    EksOtus.updateActiveDeckLEDs();
}

// Register value scaling functions
EksOtus.registerScalers = function() {
    // Register functions to scale value
    EksOtus.registerScalingFunction('volume',EksOtus.volumeScaler);
    EksOtus.registerScalingFunction('pregain',EksOtus.volumeScaler);
    EksOtus.registerScalingFunction('crossfader',EksOtus.plusMinus1Scaler);
    EksOtus.registerScalingFunction('filterLow',EksOtus.eqScaler);
    EksOtus.registerScalingFunction('filterMid',EksOtus.eqScaler);
    EksOtus.registerScalingFunction('filterHigh',EksOtus.eqScaler);
    EksOtus.registerScalingFunction('jog',EksOtus.jogScaler);
    EksOtus.registerScalingFunction('jog_scratch',EksOtus.jogScratchScaler);
}

// Dummy callback for unregistered controls
EksOtus.dump = function(field) {
    script.HIDDebug("FIELD " + field.id + " VALUE " + field.value + " DELTA " +
    field.delta);
}

// Register input/output field callback functions
EksOtus.registerCallbacks = function() {
    EksOtus.registerInputCallback('control','hid','deck_switch',EksOtus.deckSwitch);
    EksOtus.registerInputCallback('control','hid','jog_sw',EksOtus.dump);
    EksOtus.registerInputCallback('control','hid','jog_sw_button',EksOtus.dump);
    EksOtus.registerInputCallback('control','deck1','pregain',EksOtus.volume_pregain);
    EksOtus.registerInputCallback('control','deck1','rate_encoder',EksOtus.rate_wheel);
    EksOtus.registerInputCallback('control','deck2','pregain',EksOtus.volume_pregain);
    EksOtus.registerInputCallback('control','deck2','rate_encoder',EksOtus.rate_wheel);
    EksOtus.registerInputCallback('control','[Master]','headphones',EksOtus.headphones);
    EksOtus.registerInputCallback('control','deck','beat_align',EksOtus.beat_align);

    EksOtus.registerInputCallback('control','deck','slider_scale',EksOtus.pitchSlider);
    EksOtus.registerInputCallback('control','deck','slider_value',EksOtus.pitchSlider);
    EksOtus.registerInputCallback('control','deck','slider_position',EksOtus.pitchSlider);
    EksOtus.registerInputCallback('control','deck','slider_pos_1',EksOtus.pitchSlider);
    EksOtus.registerInputCallback('control','deck','slider_pos_2',EksOtus.pitchSlider);

    EksOtus.registerInputCallback('control','hid','trackpad_x',EksOtus.xypad);
    EksOtus.registerInputCallback('control','hid','trackpad_y',EksOtus.xypad);
    EksOtus.registerInputCallback('control','hid','trackpad_left',EksOtus.xypad);
    EksOtus.registerInputCallback('control','hid','trackpad_right',EksOtus.xypad);
    EksOtus.registerInputCallback('control','hid','touch_trackpad',EksOtus.xypad);

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
EksOtus.jogScaler = function(group,name,value) { return value/256*3; }

// Jog wheel scratch event scaler
EksOtus.jogScratchScaler = function(group,name,value) {
    var ticks = undefined;
    if (engine.getValue(group,'play')) {
        if (value>0) 
            ticks = value/64;
        else 
            ticks = value/64;
    } else {
        // TODO - do different scaling for stopped scratching
        if (value>0) 
            ticks = 1;
        else 
            ticks = -1;
    }
    // print("JOG SCRATCH SCALER TICKS " + ticks + " FROM VALUE " + value);
    return ticks;
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

// Deck rate adjustment with top corner wheels
EksOtus.rate_wheel = function(field) {
    if (EksOtus.activeDeck==undefined)
        return;
    var active_group = EksOtus.resolveGroup(field.group);
    var current = engine.getValue(active_group,'rate');
    if (field.delta<0)
        engine.setValue(active_group,'rate',current+0.003);
    else
        engine.setValue(active_group,'rate',current-0.003);
}

// Callback to set current loaded trakc's duration for wheel led animation
EksOtus.loadedTrackDuration = function(value) {
    EksOtus.activeTrackDuration = value;
}

// Switch the visual LED feedback on platter LEDs to active deck
// NOTE this is now disabled, it causes HID input errors in firmware!
EksOtus.activateSpinningPlatterLEDs = function() {
    var active_group = EksOtus.resolveDeckGroup(EksOtus.activeDeck);
    script.HIDDebug("ACTIVE GROUP " + active_group);
    if (active_group==undefined)
        return;
    if (!(EksOtus.activeDeck in EksOtus.deckLEDColors)) {
        script.HIDDebug("LED color not mapped to deck " % EksOtus.activeDeck);
        return;
    }
    if (active_group==undefined) {
        EksOtus.disableSpinningPlatterLEDs();
        return;
    }
    if (EksOtus.activeSpinningPlatterGroup !=undefined) {
        EksOtus.disableSpinningPlatterLEDs();
    }
    EksOtus.activeSpinningPlatterGroup = active_group;
    EksOtus.loadedTrackDuration(engine.getValue(active_group,"duration"));
    EksOtus.enableSpinningPlatterLEDs();
}

// Enable spinning platter LED functionality for active virtual deck
EksOtus.enableSpinningPlatterLEDs = function() {
    if (EksOtus.activeSpinningPlatterGroup==undefined)
        return;
    script.HIDDebug("Activating platter spin LEDs " + EksOtus.activeSpinningPlatterGroup);
    engine.connectControl(
        EksOtus.activeSpinningPlatterGroup,
        'visual_playposition',
        "EksOtus.circleLEDs"
    );
    engine.connectControl(
        EksOtus.activeSpinningPlatterGroup,
        'duration',
        "EksOtus.loadedTrackDuration"
    )
    EksOtus.resetWheelLEDs('off',false);
}

// Disable spinning platter LED functionality for active virtual deck
EksOtus.disableSpinningPlatterLEDs = function() {
    if (EksOtus.activeSpinningPlatterGroup==undefined)
        return;
    script.HIDDebug("Disabling platter spin LEDs " + EksOtus.activeSpinningPlatterGroup);
    engine.connectControl(
        EksOtus.activeSpinningPlatterGroup,
        'visual_playposition',
        "EksOtus.circleLEDs",
        true
    );
    engine.connectControl(
        EksOtus.activeSpinningPlatterGroup,
        'duration',
        "EksOtus.loadedTrackDuration",
        true
    )
    EksOtus.resetWheelLEDs('off',false);
}

// Callback from engine to set every third LED in circling pattern according to
// the track position. Careful not to enable sending all 60 positions, it may
// cause too much HID traffic!
EksOtus.circleLEDs = function(position) {
    if (position<0 || position>1) {
        EksOtus.resetWheelLEDs('off',false);
        return;
    }
    // Only update every third LED to save HID packet bandwidth
    var wheelLEDSplit = 3;
    var wheelLEDGroups = EksOtus.wheelLEDCount/wheelLEDSplit;
    var timeRemaining = ((1-position)*EksOtus.activeTrackDuration) | 0; 
    var track_pos = position * EksOtus.activeTrackDuration;
    var revolutions = track_pos / EksOtus.revTime;
    var led_index = (((revolutions-(revolutions|0))*wheelLEDGroups)|0)*wheelLEDSplit;
    led_index++;
    if (led_index==EksOtus.activeSpinningPlatterLED) 
        return;
    EksOtus.activeSpinningPlatterLED = led_index;
    var led_color = EksOtus.deckLEDColors[EksOtus.activeDeck];
    EksOtus.resetWheelLEDs('off',false);
    EksOtus.setLED('jog','wheel_'+(led_index),led_color);
    EksOtus.updateLEDs();
}

// Reset all wheel LEDs to given color. If color is undefined,
// use 'off'
EksOtus.resetWheelLEDs = function (color) {
    if (color==undefined || !(color in EksOtus.LEDColors))
        color = 'off';
    for (i=1;i<=EksOtus.wheelLEDCount;i++) 
        EksOtus.setLED('jog','wheel_'+i,color,false);
    EksOtus.updateLEDs(true);
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
        if (field.value!=0)
            if (!engine.getValue(active_group,'quantize'))
                engine.setValue(active_group,'quantize',true);
            else
                engine.setValue(active_group,'quantize',false);
    }
}

// Pitch slider modifies track speed directly
// TODO - make this relative to the touch position
EksOtus.pitchSlider = function (field) {
    // Use the top jogs now for pitch
    return;
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

// Use headphones volume, if modifier shift is active, pre/main mix otherwise
EksOtus.volume_pregain = function (field) {
    if (EksOtus.activeDeck==undefined)
        return;
    var active_group = EksOtus.resolveGroup(field.group);
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
    if (!(EksOtus.activeDeck in EksOtus.deckSwitchMap))
        EksOtus.activeDeck = 1;
    EksOtus.activeDeck = EksOtus.deckSwitchMap[EksOtus.activeDeck];
    EksOtus.setLED('hid','deck_switch',EksOtus.deckLEDColors[EksOtus.activeDeck]);
    EksOtus.connectDeckLEDs();
    this.updateActiveDeckLEDs();
    // EksOtus.activateSpinningPlatterLEDs();
    script.HIDDebug('Active EKS Otus deck now ' + EksOtus.activeDeck);
}

// Silly little function for wheel LEDs to indicate device is initialized
// Triggers itself with a timer to reverse the LED states to off.
EksOtus.wheelLEDInitAnimation = function (state) {
    var i;
    var name = undefined;
    if (state=='off') {
        //print("DISABLE wheel animation");
        if (EksOtus.initAnimationTimer!=undefined) {
            engine.stopTimer(EksOtus.initAnimationTimer);
            EksOtus.initAnimationTimer = undefined;
        }
        EksOtus.resetWheelLEDs(state);
        // if (EksOtus.activeDeck)
        //    EksOtus.activateSpinningPlatterLEDs();
    } else {
        //print("ENABLE wheel animation");
        EksOtus.resetWheelLEDs(state);
        EksOtus.initAnimationTimer = engine.beginTimer(
            1000, "EksOtus.wheelLEDInitAnimation('off')"
        );
    }
}

//
// HID Packet registration details for Otus
//

// Initialize control fields, buttons and LEDs
// Group name 'deck' is dynamically modified to active deck, since
// Otus is a dualdeck controller.
EksOtus.registerInputPackets = function() {
    var packet = undefined;
    var name = undefined;
    var offset = 0;

    packet = new HIDPacket('control',[0x0,0x35],64);
    packet.addControl('deck','wheel_position',2,'H');
    packet.addControl('deck','jog_wheel',4,'h');
    packet.addControl('hid','timestamp',6,'I');
    packet.addControl('deck','slider_value',10,'H');
    packet.addControl('deck','slider_position',12,'H');
    packet.addControl('deck2','rate_encoder',14,'B',undefined,true);
    packet.addControl('[Playlist]','SelectTrackKnob',15,'B',undefined,true);
    packet.addControl('hid','jog_sw',16,'B',undefined,true);
    packet.addControl('deck1','rate_encoder',17,'B',undefined,true);
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
    packet.addControl('hid','trackpad_x',38,'H');
    packet.addControl('hid','trackpad_y',40,'H');
    packet.addControl('deck','slider_pos_2',42,'H');
    packet.addControl('deck','slider_pos_1',44,'H');
    packet.addControl('deck2','keylock',46,'I',0x1);
    packet.addControl('deck','beatloop_8',46,'I',0x2);
    packet.addControl('deck','beatloop_4',46,'I',0x4);
    packet.addControl('deck','beatloop_2',46,'I',0x8);
    packet.addControl('deck','beatloop_1',46,'I',0x10);
    packet.addControl('deck','loop_in',46,'I',0x20);
    packet.addControl('deck','loop_out',46,'I',0x40);
    packet.addControl('deck','reloop_exit',46,'I',0x80);
    packet.addControl('deck','slider_scale',46,'I',0x100);
    packet.addControl('deck','LoadSelectedTrack',46,'I',0x200);
    packet.addControl('modifiers','shift',46,'I',0x400);
    packet.addControl('hid','deck_switch',46,'I',0x800);
    packet.addControl('deck','pfl',46,'I',0x1000);
    packet.addControl('hid','jog_sw_button',46,'I',0x2000);
    packet.addControl('deck','filterLowKill',46,'I',0x4000);
    packet.addControl('deck','play',46,'I',0x8000);
    packet.addControl('deck','cue_default',46,'I',0x10000);
    packet.addControl('deck','filterMidKill',46,'I',0x20000);
    packet.addControl('deck','filterHighKill',46,'I',0x40000);
    packet.addControl('deck','beat_align',46,'I',0x80000);
    packet.addControl('deck1','keylock',46,'I',0x100000);
    packet.addControl('deck','jog_touch',46,'I',0x200000);
    packet.addControl('hid','trackpad_left',46,'I',0x400000);
    packet.addControl('hid','trackpad_right',46,'I',0x800000);
    packet.addControl('deck','hotcue_1',46,'I',0x1000000);
    packet.addControl('deck','hotcue_2',46,'I',0x2000000);
    packet.addControl('deck','hotcue_3',46,'I',0x4000000);
    packet.addControl('deck','hotcue_4',46,'I',0x8000000);
    packet.addControl('deck','hotcue_5',46,'I',0x10000000);
    packet.addControl('deck','hotcue_6',46,'I',0x20000000);
    packet.addControl('modifiers','pitchslider',46,'I',0x40000000)
    packet.addControl('hid','touch_trackpad',46,'I',0x80000000);
    packet.addControl('hid','packet_number',51,'B');
    packet.addControl('hid','deck_status',52,'B');

    packet.setIgnored('hid','timestamp',true);
    packet.setIgnored('hid','packet_number',true);
    packet.setIgnored('hid','deck_status',true);

    packet.setMinDelta('deck','jog_wheel',8);
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

    packet = new HIDPacket('firmware_version',[0xa,0x4],64,EksOtus.FirmwareVersionResponse);
    packet.addControl('hid','major',2,'B');
    packet.addControl('hid','minor',3,'B');
    EksOtus.registerInputPacket(packet);

    packet = new HIDPacket('trackpad_mode',[0x5,0x3],64,EksOtus.TrackpadModeResponse);
    packet.addControl('hid','status',2,'B');
    EksOtus.registerInputPacket(packet);
}

// Register output packets we send to the controller
EksOtus.registerOutputPackets = function() {
    var packet = undefined;
    var name = undefined;
    var offset = 0;

    packet = new HIDPacket('button_leds',[0x16,0x18],32);
    offset = 2;
    packet.addLED('deck1','keylock',offset++,'B');
    packet.addLED('deck2','keylock',offset++,'B');
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
    packet.addLED('deck','quantize',offset++,'B');
    EksOtus.registerOutputPacket(packet);

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

    packet = new HIDPacket('led_wheel_left',[0x14,0x20],32);
    offset = 2;
    for (var led_index=1;led_index<=EksOtus.wheelLEDCount/2;led_index++) 
        packet.addLED('jog','wheel_' + led_index,offset++,'B');
    EksOtus.registerOutputPacket(packet);

    packet = new HIDPacket('led_wheel_right',[0x15,0x20],32);
    offset = 2;
    for (var led_index=EksOtus.wheelLEDCount/2+1;led_index<=EksOtus.wheelLEDCount;led_index++) 
        packet.addLED('jog','wheel_' + led_index,offset++,'B');
    EksOtus.registerOutputPacket(packet);

    packet = new HIDPacket('request_firmware_version',[0xa,0x2],32);
    EksOtus.registerOutputPacket(packet);

    packet = new HIDPacket('set_trackpad_mode',[0x5,0x3],32);
    packet.addControl('hid','mode',2,'B');
    EksOtus.registerOutputPacket(packet);

    packet = new HIDPacket('set_ledcontrol_mode',[0x1d,0x3],32);
    packet.addControl('hid','mode',2,'B');
    EksOtus.registerOutputPacket(packet);
}

// Otus specific output packet to request device firmware version
EksOtus.requestFirmwareVersion = function() {
    var packet = EksOtus.resolveOutputPacket('request_firmware_version');
    if (packet==undefined)
        return;
    script.HIDDebug("Requesting firmware version " + packet.name);
    packet.send();
}

// Firmware version response. Required to finish device INIT
EksOtus.FirmwareVersionResponse = function(packet,delta) {
    var field_major = packet.lookupField('hid','major');
    var field_minor = packet.lookupField('hid','minor');
    if (field_major==undefined || field_minor==undefined) {
        script.HIDDebug("Error parsing response version packet");
        return;
    }
    EksOtus.initialized=true;
    EksOtus.version_major = field_major.value;
    EksOtus.version_minor = field_minor.value;
    EksOtus.setLEDControlMode(1);

    EksOtus.updateLEDs();
    if (EksOtus.activeDeck!=undefined) {
        EksOtus.connectDeckLEDs();
        EksOtus.updateActiveDeckLEDs();
        //EksOtus.activateSpinningPlatterLEDs();
    } else {
        // Start blinking 'deck switch' button to indicate we are ready.
        EksOtus.setLEDBlink('hid','deck_switch','amber');
        // Indicate we are initialized with a little animation
        EksOtus.wheelLEDInitAnimation('amber');
    }
    // EksOtus.wheelLEDInitAnimation('amber');
    script.HIDDebug("EKS " + EksOtus.id +
        " v"+EksOtus.version_major+"."+EksOtus.version_minor+
        " initialized"
    );
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

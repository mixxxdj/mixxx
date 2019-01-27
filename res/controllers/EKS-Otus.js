//
// EKS Otus HID controller script v1.0
// Copyright (C) 2012, Sean M. Pappalardo, Ilkka Tuohela
// but feel free to tweak this to your heart's content!
// For Mixxx version 1.11.x
//

// EKS Otus HID interface specification
function EKSOtusController() {
    this.controller = new HIDController();

    // Initialized to firmware version by version response packet
    this.version_major = undefined;
    this.version_minor = undefined;
    this.controller.activeDeck = 1;

    this.controller.LEDColors = { off: 0x0, red: 0x0f, green: 0xf0, amber: 0xff };
    this.controller.deckOutputColors = { 1: "red", 2: "green", 3: "red", 4: "green"};

    // Static variables for HID specs
    this.wheelLEDCount = 60;
    this.buttonLEDCount = 22;
    this.sliderLEDCount = 20;

    this.registerInputPackets = function() {
        var packet = undefined;
        var name = undefined;
        var offset = 0;

        packet = new HIDPacket("control", 0, undefined, [0x35]);
        packet.addControl("hid","wheel_position",2,"H");
        packet.addControl("hid","wheel_speed",4,"h");
        packet.addControl("hid","timestamp",6,"I");
        packet.addControl("hid","slider_value",10,"H");
        packet.addControl("hid","slider_position",12,"H");
        packet.addControl("hid","rate_encoder",14,"B",undefined,true);
        packet.addControl("hid","jog_se",15,"B",undefined,true);
        packet.addControl("hid","jog_sw",16,"B",undefined,true);
        packet.addControl("hid","rate_encoder",17,"B",undefined,true);
        packet.addControl("hid","gain_1",18,"H");
        packet.addControl("hid","gain_2",20,"H");
        packet.addControl("hid","eq_high_1",22,"H");
        packet.addControl("hid","eq_high_2",24,"H");
        packet.addControl("hid","eq_mid_1",26,"H");
        packet.addControl("hid","eq_mid_2",28,"H");
        packet.addControl("hid","eq_low_1",30,"H");
        packet.addControl("hid","eq_low_2",32,"H");
        packet.addControl("hid","crossfader",34,"H");
        packet.addControl("hid","headphones",36,"H");
        packet.addControl("hid","trackpad_x",38,"H");
        packet.addControl("hid","trackpad_y",40,"H");
        packet.addControl("hid","slider_pos_2",42,"H");
        packet.addControl("hid","slider_pos_1",44,"H");
        packet.addControl("hid","keylock",46,"I",0x1);
        packet.addControl("hid","beatloop_8",46,"I",0x2);
        packet.addControl("hid","beatloop_4",46,"I",0x4);
        packet.addControl("hid","beatloop_2",46,"I",0x8);
        packet.addControl("hid","beatloop_1",46,"I",0x10);
        packet.addControl("hid","loop_in",46,"I",0x20);
        packet.addControl("hid","loop_out",46,"I",0x40);
        packet.addControl("hid","reloop_exit",46,"I",0x80);
        packet.addControl("hid","slider_scale",46,"I",0x100);
        packet.addControl("hid","jog_se_button",46,"I",0x200);
        packet.addControl("hid","eject_right",46,"I",0x400);
        packet.addControl("hid","deck_switch",46,"I",0x800);
        packet.addControl("hid","eject_left",46,"I",0x1000);
        packet.addControl("hid","jog_sw_button",46,"I",0x2000);
        packet.addControl("hid","stop",46,"I",0x4000);
        packet.addControl("hid","play",46,"I",0x8000);
        packet.addControl("hid","cue",46,"I",0x10000);
        packet.addControl("hid","reverse",46,"I",0x20000);
        packet.addControl("hid","brake",46,"I",0x40000);
        packet.addControl("hid","fastforward",46,"I",0x80000);
        packet.addControl("hid","jog_nw_button",46,"I",0x100000);
        packet.addControl("hid","jog_touch",46,"I",0x200000);
        packet.addControl("hid","trackpad_left",46,"I",0x400000);
        packet.addControl("hid","trackpad_right",46,"I",0x800000);
        packet.addControl("hid","hotcue_1",46,"I",0x1000000);
        packet.addControl("hid","hotcue_2",46,"I",0x2000000);
        packet.addControl("hid","hotcue_3",46,"I",0x4000000);
        packet.addControl("hid","hotcue_4",46,"I",0x8000000);
        packet.addControl("hid","hotcue_5",46,"I",0x10000000);
        packet.addControl("hid","hotcue_6",46,"I",0x20000000);
        packet.addControl("hid","touch_slider",46,"I",0x40000000)
        packet.addControl("hid","touch_trackpad",46,"I",0x80000000);
        packet.addControl("hid","packet_number",51,"B");
        packet.addControl("hid","deck_status",52,"B"); 
        this.controller.registerInputPacket(packet);

        packet = new HIDPacket("firmware_version", 0xa, undefined, [0x4]);
        packet.addControl("hid","major",2,"B");
        packet.addControl("hid","minor",3,"B");
        this.controller.registerInputPacket(packet);

        packet = new HIDPacket("trackpad_mode", 0x5, undefined, [0x3]);
        packet.addControl("hid","status",2,"B");
        this.controller.registerInputPacket(packet);

    }

    this.registerOutputPackets = function() { 
        var packet = undefined;
        var name = undefined;
        var offset = 0;

        packet = new HIDPacket("button_leds", 0x16, undefined, [0x18]);
        offset = 1;
        packet.addOutput("hid","jog_nw",offset++,"B");
        packet.addOutput("hid","jog_ne",offset++,"B");
        packet.addOutput("hid","jog_se",offset++,"B");
        packet.addOutput("hid","jog_sw",offset++,"B");
        packet.addOutput("hid","beatloop_8",offset++,"B");
        packet.addOutput("hid","beatloop_4",offset++,"B");
        packet.addOutput("hid","beatloop_2",offset++,"B");
        packet.addOutput("hid","beatloop_1",offset++,"B");
        packet.addOutput("hid","loop_in",offset++,"B");
        packet.addOutput("hid","loop_out",offset++,"B");
        packet.addOutput("hid","reloop_exit",offset++,"B");
        packet.addOutput("hid","eject_right",offset++,"B");
        packet.addOutput("hid","deck_switch",offset++,"B");
        packet.addOutput("hid","trackpad_right",offset++,"B");
        packet.addOutput("hid","trackpad_left",offset++,"B");
        packet.addOutput("hid","eject_left",offset++,"B");
        packet.addOutput("hid","stop",offset++,"B");
        packet.addOutput("hid","play",offset++,"B");
        packet.addOutput("hid","reverse",offset++,"B");
        packet.addOutput("hid","cue",offset++,"B");
        packet.addOutput("hid","brake",offset++,"B");
        packet.addOutput("hid","fastforward",offset++,"B");
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("slider_leds", 0x17, undefined, [0x16]);
        offset = 1;
        packet.addOutput("pitch","slider_1",offset++,"B");
        packet.addOutput("pitch","slider_2",offset++,"B");
        packet.addOutput("pitch","slider_3",offset++,"B");
        packet.addOutput("pitch","slider_4",offset++,"B");
        packet.addOutput("pitch","slider_5",offset++,"B");
        packet.addOutput("pitch","slider_6",offset++,"B");
        packet.addOutput("pitch","slider_7",offset++,"B");
        packet.addOutput("pitch","slider_8",offset++,"B");
        packet.addOutput("pitch","slider_9",offset++,"B");
        packet.addOutput("pitch","slider_10",offset++,"B");
        packet.addOutput("pitch","slider_11",offset++,"B");
        packet.addOutput("pitch","slider_12",offset++,"B");
        packet.addOutput("pitch","slider_13",offset++,"B");
        packet.addOutput("pitch","slider_14",offset++,"B");
        packet.addOutput("pitch","slider_15",offset++,"B");
        packet.addOutput("pitch","slider_16",offset++,"B");
        packet.addOutput("pitch","slider_17",offset++,"B");
        packet.addOutput("pitch","slider_scale_1",offset++,"B");
        packet.addOutput("pitch","slider_scale_2",offset++,"B");
        packet.addOutput("pitch","slider_scale_3",offset++,"B");
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("led_wheel_left", 0x14, undefined, [0x20]);
        offset = 1;
        for (var led_index=1;led_index<=this.wheelLEDCount/2;led_index++) 
            packet.addOutput("hid","wheel_" + led_index,offset++,"B");
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("led_wheel_right", 0x15, undefined, [0x20]);
        offset = 1;
        for (var led_index=this.wheelLEDCount/2+1;led_index<=this.wheelLEDCount;led_index++) 
            packet.addOutput("hid","wheel_" + led_index,offset++,"B");
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("request_firmware_version", 0xa, undefined, [0x2]);
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("set_trackpad_mode", 0x5, undefined, [0x3]);
        packet.addControl("hid","mode",2,"B");
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("set_ledcontrol_mode", 0x1d, undefined, [0x3]);
        packet.addControl("hid","mode",2,"B");
        this.controller.registerOutputPacket(packet);
    }

    // Otus specific output packet to request device firmware version
    this.requestFirmwareVersion = function() {
        var packet = this.controller.getOutputPacket("request_firmware_version");
        if (packet==undefined)
            return;
        HIDDebug("Requesting firmware version " + packet.name);
        packet.send();
    }

    // Set LED Control Mode on Otus firmware versions > 1.6. Major and minor must
    // contain the version numbers for firmware as received from response.
    // Valid modes are:
    //      0   disable all LEDs
    //      1   Re-enable LEDs
    //      2   Revert to built-in light functionality
    this.setLEDControlMode = function(mode) {
        var controller = this.controller;
        if (this.version_major<=1 && this.version_minor<6) {
            // Firmware version does not support LED Control Mode Setting
            return;
        }
        if (mode!=0 && mode!=1 && mode!=2) {
            HIDDebug("Unknown value for LED Control Mode Setting: " + mode);
            return;
        }
        var packet = controller.getOutputPacket("set_ledcontrol_mode");
        var field = packet.getField("hid","mode");
        if (field==undefined) {
            HIDDebug("EksOtus.setLEDControlMode error fetching field mode");
            return;
        }
        field.value = mode;
        packet.send();
    }

    // Firmware version response. Required to finish device INIT
    this.FirmwareVersionResponse = function(packet,delta) {
        var controller = this.controller;
        var field_major = packet.getField("hid","major");
        var field_minor = packet.getField("hid","minor");
        if (field_major==undefined || field_minor==undefined) {
            HIDDebug("Error parsing response version packet");
            return;
        }
        this.version_major = field_major.value;
        this.version_minor = field_minor.value;
        controller.initialized = true;

        this.setLEDControlMode(1);
        if (controller.activeDeck!=undefined) {
            controller.setOutput("hid","deck_switch", controller.LEDColors[controller.deckOutputColors[controller.activeDeck]]);
            controller.switchDeck(controller.activeDeck);
        } else {
            var value = controller.LEDColors["amber"];
            this.controller.setOutputToggle("hid","deck_switch",value);
        }
        this.updateLEDs();
        HIDDebug("EKS " + EksOtus.id +
            " v"+EksOtus.version_major+"."+EksOtus.version_minor+
            " initialized"
        );
    }

    // Otus specific output packet to set the trackpad control mode
    this.setTrackpadMode = function(mode) {
        if (mode!=0 && mode!=1) {
            HIDDebug("Unsupported trackpad mode value: " + mode);
            return;
        }
        var packet = this.controller.getOutputPacket("set_trackpad_mode");
        if (packet==undefined) {
            HIDDebug("Output not registered: set_trackpad_mode");
            return;
        }
        var field = packet.getField("hid","mode");
        if (field==undefined) {
            HIDDebug("EksOtus.setTrackpadMode error fetching field mode");
            return;
        }
        field.value = mode;
        packet.send();
    }

    // Response to above trackpad mode packet
    this.TrackpadModeResponse = function(packet,delta) {
        field = packet.getField("hid","status");
        if (field==undefined) {
            HIDDebug("Error parsing field status from packet");
            return;
        }
        if (field.value==1) {
            HIDDebug("Trackpad mode successfully set");
        } else {
            HIDDebug("Trackpad mode change failed");
        }
    }

    // Generic unsigned short to -1..0..1 range scaling
    this.plusMinus1Scaler = function(group,name,value) {
        if (value<32768)
            return value/32768-1;
        else
            return (value-32768)/32768;
    }

    // Volume slider scaling for 0..1..5 scaling
    this,volumeScaler = function(group,name,value) {
        return script.absoluteNonLin(value, 0, 1, 5, 0, 65536);
    }

    // EQ scaling function for 0..1..4 scaling
    this.eqScaler = function(group,name,value) {
        return script.absoluteNonLin(value, 0, 1, 4, 0, 65536);
    }

    // Mandatory call from init() to initialize hardware
    this.initializeHIDController = function() {
        this.registerInputPackets();
        this.registerOutputPackets();
    }

    this.shutdownHardware = function() {
        this.setLEDControlMode(2);
        this.setTrackpadMode(1);
    }

}

EksOtus = new EKSOtusController(); 

// Initialize device state, send request for firmware. Otus is not
// usable before we receive a valid firmware version response.
EksOtus.init = function (id) {
    EksOtus.id = id;

    EksOtus.LEDUpdateInterval = 250;
    // Valid values: 1 for mouse mode, 0 for xy-pad mode
    EksOtus.trackpadMode = 0;

    EksOtus.deckSwitchClicked = false;

    // Wheel absolute position value
    EksOtus.wheelPosition = undefined;

    // Wheel spin animation details
    EksOtus.activeTrackDuration = undefined;
    // Group registered to update spinning platter details
    EksOtus.activeSpinningPlatterGroup = undefined;
    // Virtual record spin time, 1.8 for 33 1/3 RPM, 1.33 for 45 RPM
    EksOtus.revTime = 1.8;
    // Wheel LED index, range 1-60
    EksOtus.activeSpinningPlatterLED = undefined;

    // Call the HID packet parser initializers
    EksOtus.initializeHIDController();
    var controller = EksOtus.controller;
    // Set callbacks for packets here to avoid issues in callback handling
    controller.setPacketCallback("firmware_version",EksOtus.FirmwareVersionWrapper);
    controller.setPacketCallback("trackpad_mode",EksOtus.TrackpadModeWrapper);

    controller.ignoredControlChanges = [
        "mask","timestamp","packet_number","deck_status", "wheel_speed",
        // These return the Otus slider position scaled by the 'slider scale'
        "slider_pos_1","slider_pos_2", "slider_value"
    ];

    // Scratch parameters
    controller.scratchintervalsPerRev = 1024;
    controller.scratchAlpha = 1.0/8;
    controller.rampedScratchEnable = true;

    EksOtus.setTrackpadMode(this.trackpadMode);
    // Note: Otus is not considered initialized before we get 
    // response to this packet
    EksOtus.requestFirmwareVersion();
    // Link controls and register callbacks
    EksOtus.registerCallbacks();

    engine.softTakeover("[Master]","headVolume",true);
    engine.softTakeover("[Master]","headMix",true);
    for (var deck in controller.deckOutputColors) {
        engine.softTakeover("[Channel"+deck+"]","pregain",true);
        engine.softTakeover("[Channel"+deck+"]","volume",true);
    }

    if (EksOtus.LEDUpdateInterval!=undefined) {
        controller.timers["led_update"] = engine.beginTimer(
            EksOtus.LEDUpdateInterval,
            "EksOtus.updateLEDs(true)"
        );
    }

}

EksOtus.outputCallback = function(value,group,key) {
    var controller = EksOtus.controller;
    if (group=="deck") {
        if (controller.activeDeck==undefined)
            return;
        group = controller.resolveGroup("deck");
    }
    if (value==1) 
        EksOtus.controller.setOutput("deck",key,
            controller.LEDColors[controller.deckOutputColors[controller.activeDeck]],
            true
        );
    else
        EksOtus.controller.setOutput("deck",key,controller.LEDColors.off,true);
}

EksOtus.updateLEDs = function(from_timer) {
    var controller = EksOtus.controller;
    controller.getOutputPacket("button_leds").send();
    controller.getOutputPacket("slider_leds").send();
    controller.getOutputPacket("led_wheel_left").send();
    controller.getOutputPacket("led_wheel_right").send();
}

// Device cleanup function
EksOtus.shutdown = function() {
    engine.softTakeover("[Master]","headVolume",false);
    engine.softTakeover("[Master]","headMix",false);
    for (var deck in controller.deckOutputColors) {
        engine.softTakeover("[Channel"+deck+"]","pregain",false);
        engine.softTakeover("[Channel"+deck+"]","volume",false);
    }
    EksOtus.shutdownHardware(2);
    HIDDebug("EKS "+EksOtus.id+" shut down");
}

// Mandatory default handler for incoming packets
EksOtus.incomingData = function(data,length) {
    EksOtus.controller.parsePacket(data,length);
}

EksOtus.FirmwareVersionWrapper = function(packet,data) {
    return EksOtus.FirmwareVersionResponse(packet,data); 
}

EksOtus.TrackpadModeWrapper = function(packet,data) {
    return EksOtus.TrackpadModeResponse(packet,data); 
}

// Callback to set current loaded track's duration for wheel led animation
EksOtus.loadedTrackDuration = function(value) {
    EksOtus.activeTrackDuration = value;
}

// Link virtual HID naming of input and LED controls to mixxx
// Note: HID specification has more fields than we map here. 
EksOtus.registerCallbacks = function() {
    var controller = EksOtus.controller;

    controller.linkModifier("hid","eject_right","shift");
    controller.linkModifier("hid","touch_slider","pitch");

    controller.linkControl("hid","play","deck","play");
    controller.linkControl("hid","cue","deck","cue_default");
    controller.linkControl("hid","reverse","deck","reverse");
    controller.linkControl("hid","eject_left","deck","pfl");
    controller.linkControl("hid","jog_touch","deck","jog_touch");
    controller.linkControl("hid","wheel_position","deck","jog_wheel");

    controller.linkControl("hid","jog_se_button","deck","LoadSelectedTrack");
    controller.linkControl("hid","jog_se","[Playlist]","SelectTrackKnob");

    controller.linkControl("hid","crossfader","[Master]","crossfader");
    controller.linkControl("hid","gain_1","deck1","pregain");
    controller.linkControl("hid","gain_2","deck2","pregain");
    controller.linkControl("hid","eq_high_1","deck1","filterHigh");
    controller.linkControl("hid","eq_high_2","deck2","filterHigh");
    controller.linkControl("hid","eq_mid_1","deck1","filterMid");
    controller.linkControl("hid","eq_mid_2","deck2","filterMid");
    controller.linkControl("hid","eq_low_1","deck1","filterLow");
    controller.linkControl("hid","eq_low_2","deck2","filterLow");

    controller.setScaler("jog",EksOtus.jogScaler);
    controller.setScaler("jog_scratch",EksOtus.wheelScaler);
    controller.setScaler("crossfader",EksOtus.plusMinus1Scaler);
    controller.setScaler("pregain",EksOtus.eqScaler);
    controller.setScaler("filterHigh",EksOtus.eqScaler);
    controller.setScaler("filterMid",EksOtus.eqScaler);
    controller.setScaler("filterLow",EksOtus.eqScaler);

    controller.setCallback("control","hid","hotcue_1",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_2",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_3",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_4",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_5",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_6",EksOtus.hotcue);

    controller.setCallback("control","hid","beatloop_1",EksOtus.beatloop);
    controller.setCallback("control","hid","beatloop_2",EksOtus.beatloop);
    controller.setCallback("control","hid","beatloop_4",EksOtus.beatloop);
    controller.setCallback("control","hid","beatloop_8",EksOtus.beatloop);
    controller.linkControl("hid","loop_in","deck","loop_in");
    controller.linkControl("hid","loop_out","deck","loop_out");
    controller.linkControl("hid","reloop_exit","deck","reloop_exit");

    controller.setCallback("control","hid","deck_switch",EksOtus.deckSwitch);

    //controller.linkControl("hid","headphones","[Master]","headphones");
    controller.setCallback("control","hid","headphones",EksOtus.headphones);

    controller.setCallback("control","hid","slider_scale",EksOtus.pitchSlider);
    controller.setCallback("control","hid","slider_value",EksOtus.pitchSlider);
    controller.setCallback("control","hid","slider_position",EksOtus.pitchSlider);
    controller.setCallback("control","hid","slider_pos_1",EksOtus.pitchSlider);
    controller.setCallback("control","hid","slider_pos_2",EksOtus.pitchSlider);

    controller.linkOutput("hid","beatloop_8","deck","beatloop_8_enabled",EksOtus.outputCallback);
    controller.linkOutput("hid","beatloop_4","deck","beatloop_4_enabled",EksOtus.outputCallback);
    controller.linkOutput("hid","beatloop_2","deck","beatloop_2_enabled",EksOtus.outputCallback);
    controller.linkOutput("hid","beatloop_1","deck","beatloop_1_enabled",EksOtus.outputCallback);
    controller.linkOutput("hid","loop_in","deck","loop_in",EksOtus.outputCallback);
    controller.linkOutput("hid","loop_out","deck","loop_out",EksOtus.outputCallback);
    controller.linkOutput("hid","reloop_exit","deck","reloop_exit",EksOtus.outputCallback);
    controller.linkOutput("hid","eject_left","deck","pfl",EksOtus.outputCallback);
    controller.linkOutput("hid","play","deck","play",EksOtus.outputCallback);
    controller.linkOutput("hid","reverse","deck","reverse",EksOtus.outputCallback);
    controller.linkOutput("hid","cue","deck","cue_default",EksOtus.outputCallback);

}

// Default scaler for jog values
EksOtus.wheelScaler = function(group,name,value) { 
    if (EksOtus.wheelPosition==undefined) {
        EksOtus.wheelPosition = value;
        return 0;
    }
    var delta = EksOtus.wheelPosition - value; 
    if (delta>32768)
        return 0; 
    EksOtus.wheelPosition = value;
    if (delta>-8 && delta<8)
        return -delta/4;
    return -delta/16; 
}

EksOtus.jogScaler = function(group,name,value) { 
    if (EksOtus.wheelPosition==undefined) {
        EksOtus.wheelPosition = value;
        return 0;
    }
    var delta = EksOtus.wheelPosition - value; 
    if (delta>32768)
        return 0; 
    EksOtus.wheelPosition = value;
    return -delta/64; 
}

// Deck rate adjustment with top corner wheels
EksOtus.rate_wheel = function(field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    var active_group = controller.resolveGroup(field.group);
    var current = engine.getValue(active_group,"rate");
    if (field.delta<0)
        engine.setValue(active_group,"rate",current+0.003);
    else
        engine.setValue(active_group,"rate",current-0.003);
}

// Reset all wheel LEDs to given color. If color is undefined,
// use 'off'
EksOtus.resetWheelLEDs = function (color) {
    var controller = EksOtus.controller;
    if (color==undefined || !(color in controller.LEDColors))
        color = "off";
    for (i=1;i<=EksOtus.wheelLEDCount;i++) 
        controller.setOutput("jog","wheel_"+i,color,false);
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
    var controller = EksOtus.controller;
    var command;
    if (controller.activeDeck==undefined ||
        field.value==controller.buttonStates.released)
        return;
    var active_group = controller.resolveDeckGroup(controller.activeDeck);
    if (controller.modifiers.get("shift")) 
        command = field.name + "_clear";
    else 
        command = field.name + "_activate";
    engine.setValue(active_group,command,true);
}

// Beatloops activated with normal presses to beatloop_1 - beatloop_8
EksOtus.beatloop = function (field) {
    var controller = EksOtus.controller;
    var command;
    if (controller.activeDeck==undefined ||
        field.value==controller.buttonStates.released)
        return;
    var active_group = controller.resolveDeckGroup(controller.activeDeck);
    command = field.name + "_activate";
    engine.setValue(active_group,command,true);
}

EksOtus.beat_align = function (field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    var active_group = controller.resolveGroup(field.group);
    if (controller.modifiers.get("shift")) {
        // if (field.value==controller.buttonStates.released) return;
        engine.setValue(active_group,"beats_translate_curpos",field.value);
    } else {
        if (field.value==controller.buttonStates.released)
            return;
        if (!engine.getValue(active_group,"quantize"))
            engine.setValue(active_group,"quantize",true);
        else
            engine.setValue(active_group,"quantize",false);
    }
}

// Pitch slider modifies track speed directly
EksOtus.pitchSlider = function (field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    if (controller.modifiers.get("pitch")) {
        var active_group = controller.resolveDeckGroup(controller.activeDeck);
        if (field.name=="slider_position") {
            if (field.value==0)
                return;
            var value = EksOtus.plusMinus1Scaler(
                active_group,field.name,field.value
            );
            engine.setValue(active_group,"rate",value);
        }
    }
}

// Set pregain, if modifier shift is active, deck volume otherwise
EksOtus.volume_pregain = function (field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    var active_group = controller.resolveGroup(field.group);
    if (controller.modifiers.get("shift")) {
        value = script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536);
        engine.setValue(active_group,"pregain",value);
    } else {
        value = field.value / 65536;
        engine.setValue(active_group,"volume",value);
    }
}

// Set headphones volume, if modifier shift is active, pre/main mix otherwise
EksOtus.headphones = function (field) {
    var controller = EksOtus.controller;
    if (controller.modifiers.get("shift")) {
        value = script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536);
        engine.setValue("[Master]","headVolume",value);
    } else {
        value = EksOtus.plusMinus1Scaler(field.group,field.name,field.value);
        engine.setValue("[Master]","headMix",value);
    }
}

// Control effects or something with XY pad
EksOtus.xypad = function(field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    print ("XYPAD group " + field.group + 
        " name " + field.name + " value " + field.value
    );
}

// Function called when the special 'Deck Switch' button is pressed
// TODO - add code for 'hold deck_switch and press hot_cue[1-4] 
// to select deck 1-4
EksOtus.deckSwitch = function(field) {
    var controller = EksOtus.controller;
    if (EksOtus.initialized==false)
        return;
    if (field.value == controller.buttonStates.released) {
        if (EksOtus.deckSwitchClicked==false) {
            EksOtus.deckSwitchClicked=true;
            controller.timers["deck_switch"] = engine.beginTimer(
                250,"EksOtus.deckSwitchClickedClear()"
            );
        } else {
            EksOtus.deckSwitchDoubleClick();
        }
    }
}

// Timer to clear the double click status for deck switch
EksOtus.deckSwitchClickedClear = function() {
    EksOtus.deckSwitchClicked = false;
    var controller = EksOtus.controller;
    if (controller.timers["deck_switch"]!=undefined) {
        engine.stopTimer(controller.timers["deck_switch"]);
        delete controller.timers["deck_switch"];
    }
}

// Function to handle case when 'deck_switch' button was double clicked
EksOtus.deckSwitchDoubleClick = function() {
    var controller = EksOtus.controller;
    EksOtus.deckSwitchClicked = false;
    if (controller.timers["deck_switch"]!=undefined) {
        engine.stopTimer(controller.timers["deck_switch"]);
        delete controller.timers["deck_switch"];
    }
    controller.switchDeck();
    controller.setOutput("hid","deck_switch", controller.LEDColors[controller.deckOutputColors[controller.activeDeck]]);
    EksOtus.updateLEDs();
    HIDDebug("Active EKS Otus deck now " + controller.activeDeck);
}

// Switch the visual LED feedback on platter LEDs to active deck
// NOTE this is now disabled, it causes HID input errors in firmware!
EksOtus.activateSpinningPlatterLEDs = function() {
    var controller = EksOtus.controller;
    var active_group = controller.resolveDeckGroup(controller.activeDeck);
    if (active_group==undefined)
        return;
    if (!(controller.activeDeck in controller.deckOutputColors)) {
        HIDDebug("LED color not mapped to deck " % controller.activeDeck);
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
    engine.connectControl(
        EksOtus.activeSpinningPlatterGroup,
        "visual_playposition",
        "EksOtus.circleLEDs"
    );
    engine.connectControl(
        EksOtus.activeSpinningPlatterGroup,
        "duration",
        "EksOtus.loadedTrackDuration"
    )
    EksOtus.resetWheelLEDs("off",false);
}

// Disable spinning platter LED functionality for active virtual deck
EksOtus.disableSpinningPlatterLEDs = function() {
    if (EksOtus.activeSpinningPlatterGroup==undefined)
        return;
    engine.connectControl(
        EksOtus.activeSpinningPlatterGroup,
        "visual_playposition",
        "EksOtus.circleLEDs",
        true
    );
    engine.connectControl(
        EksOtus.activeSpinningPlatterGroup,
        "duration",
        "EksOtus.loadedTrackDuration",
        true
    )
    EksOtus.resetWheelLEDs("off",false);
}

// Callback from engine to set every third LED in circling pattern according to
// the track position. Careful not to enable sending all 60 positions, it may
// cause too much HID traffic!
EksOtus.circleLEDs = function(position) {
    var controller = EksOtus.controller;
    if (position<0 || position>1) {
        EksOtus.resetWheelLEDs("off",false);
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
    EksOtus.resetWheelLEDs("off",false);
    var led_color = controller.deckOutputColors[controller.activeDeck];
    controller.setOutput("jog","wheel_"+(led_index),led_color);
    EksOtus.updateLEDs();
}


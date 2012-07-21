//
// Native Instruments Traktor Kontrol F1 HID controller script v0.9
// Copyright (C) 2012, Ilkka Tuohela
// For Mixxx version 1.11.x
//

// Native Instruments Traktor Kontrol F1 HID interface specification
function KontrolF1Controller() {
    this.controller = new HIDController();

    // Initialized to firmware version by version response packet
    this.version_major = undefined;
    this.version_minor = undefined;
    this.controller.activeDeck = 1;

    // F1 PAD LEDs are special, it has full RGB control with three bytes, 
    // we can't use normal LEDColors directly. Define here some basic
    // colors, allow using full RGB values with script
    this.padColorSet = { 
        1: [0x0,0x40,0x0], 
        2: [0x0,0x40,0x40],
        3: [0x40,0x0,0x40],
        4: [0x40,0x40,0x0], 
    };
    this.padColorActive = { 
        1: [0x0,0x7f,0x0], 
        2: [0x0,0x7f,0x7f],
        3: [0x7f,0x0,0x7f],
        4: [0x7f,0x7f,0x0], 
    };
    this.deckLEDColors = { 1: "red", 2: "green", 3: "red", 4: "green"};
    this.deckSwitchMap = { 1: 2, 2: 1, 3: 4, 4: 3, undefined: 1 };

    this.buttonNames = [
        "sync","quant","capture","shift",
        "reverse","type","size","browse",
        "stop_1_1","stop_1_2","stop_2_1","stop_2_2",
        "stop_3_1","stop_3_2","stop_4_1","stop_4_2"
    ]; 

    this.registerInputPackets = function() {
        var packet = undefined;

        packet = new HIDPacket("control",[0x1],22);
        packet.addControl("hid","pad_8_button",1,"I",0x1);
        packet.addControl("hid","pad_7_button",1,"I",0x2);
        packet.addControl("hid","pad_6_button",1,"I",0x4);
        packet.addControl("hid","pad_5_button",1,"I",0x8);
        packet.addControl("hid","pad_4_button",1,"I",0x10);
        packet.addControl("hid","pad_3_button",1,"I",0x20);
        packet.addControl("hid","pad_2_button",1,"I",0x40);
        packet.addControl("hid","pad_1_button",1,"I",0x80);
        packet.addControl("hid","pad_16_button",1,"I",0x100);
        packet.addControl("hid","pad_15_button",1,"I",0x200);
        packet.addControl("hid","pad_14_button",1,"I",0x400);
        packet.addControl("hid","pad_13_button",1,"I",0x800);
        packet.addControl("hid","pad_12_button",1,"I",0x1000);
        packet.addControl("hid","pad_11_button",1,"I",0x2000);
        packet.addControl("hid","pad_10_button",1,"I",0x4000);
        packet.addControl("hid","pad_9_button",1,"I",0x8000);
        packet.addControl("hid","shift",1,"I",0x800000);
        packet.addControl("hid","reverse",1,"I",0x400000);
        packet.addControl("hid","size",1,"I",0x100000);
        packet.addControl("hid","type",1,"I",0x200000);
        packet.addControl("hid","select_push",1,"I",0x40000);
        packet.addControl("hid","browse",1,"I",0x80000);
        packet.addControl("hid","stop_1",1,"I",0x80000000);
        packet.addControl("hid","stop_2",1,"I",0x40000000);
        packet.addControl("hid","stop_3",1,"I",0x20000000);
        packet.addControl("hid","stop_4",1,"I",0x10000000);
        packet.addControl("hid","sync",1,"I",0x8000000);
        packet.addControl("hid","quant",1,"I",0x4000000);
        packet.addControl("hid","capture",1,"I",0x2000000);
        packet.addControl("hid","select_encoder",5,"B",undefined,true);

        // Knobs have value range 0-4092, so while some controls are
        // -1..0..1 range, hidparser will return same data as with h
        // packing (16 bit range while we only use 12 bits)
        packet.addControl("hid","knob_1",6,"H");
        packet.addControl("hid","knob_2",8,"H");
        packet.addControl("hid","knob_3",10,"H");
        packet.addControl("hid","knob_4",12,"H");
        packet.addControl("hid","knob_5",14,"H");
        packet.addControl("hid","knob_6",16,"H");
        packet.addControl("hid","knob_7",18,"H");
        packet.addControl("hid","knob_8",20,"H");
        this.controller.registerInputPacket(packet);

    }

    this.registerOutputPackets = function() { 
        var packet = undefined;

        packet = new HIDPacket("lights",[0x80],81);
        // Right 7-segment element - 0x0 off, 0x40 on
        packet.addControl("hid","right_segment_dp",1,"B");
        packet.addControl("hid","right_segment_a",2,"B");
        packet.addControl("hid","right_segment_b",3,"B");
        packet.addControl("hid","right_segment_c",4,"B");
        packet.addControl("hid","right_segment_d",5,"B");
        packet.addControl("hid","right_segment_e",6,"B");
        packet.addControl("hid","right_segment_f",7,"B");
        packet.addControl("hid","right_segment_g",8,"B");

        // Left 7-segment element - 0x0 off, 0x40 on
        packet.addControl("hid","left_segment_dp",9,"B");
        packet.addControl("hid","left_segment_a",10,"B");
        packet.addControl("hid","left_segment_b",11,"B");
        packet.addControl("hid","left_segment_c",12,"B");
        packet.addControl("hid","left_segment_d",13,"B");
        packet.addControl("hid","left_segment_e",14,"B");
        packet.addControl("hid","left_segment_f",15,"B");
        packet.addControl("hid","left_segment_g",16,"B");

        // Button led brightness, 0-0xff
        packet.addControl("hid","browse_brightness",17,"B");
        packet.addControl("hid","size_brightness",18,"B");
        packet.addControl("hid","type_brightness",19,"B");
        packet.addControl("hid","reverse_brightness",20,"B");
        packet.addControl("hid","shift_brightness",21,"B");
        packet.addControl("hid","capture_brightness",22,"B");
        packet.addControl("hid","quant_brightness",23,"B");
        packet.addControl("hid","sync_brightness",24,"B");

        // Pad RGB color button controls, 3 bytes per pad
        packet.addControl("hid","pad_1_blue",25,"B")
        packet.addControl("hid","pad_1_red",26,"B")
        packet.addControl("hid","pad_1_green",27,"B")
        packet.addControl("hid","pad_2_blue",28,"B")
        packet.addControl("hid","pad_2_red",29,"B")
        packet.addControl("hid","pad_2_green",30,"B")
        packet.addControl("hid","pad_3_blue",31,"B")
        packet.addControl("hid","pad_3_red",32,"B")
        packet.addControl("hid","pad_3_green",33,"B")
        packet.addControl("hid","pad_4_blue",34,"B")
        packet.addControl("hid","pad_4_red",35,"B")
        packet.addControl("hid","pad_4_green",36,"B")
        packet.addControl("hid","pad_5_blue",37,"B")
        packet.addControl("hid","pad_5_red",38,"B")
        packet.addControl("hid","pad_5_green",39,"B")
        packet.addControl("hid","pad_6_blue",40,"B")
        packet.addControl("hid","pad_6_red",41,"B")
        packet.addControl("hid","pad_6_green",42,"B")
        packet.addControl("hid","pad_7_blue",43,"B")
        packet.addControl("hid","pad_7_red",44,"B")
        packet.addControl("hid","pad_7_green",45,"B")
        packet.addControl("hid","pad_8_blue",46,"B")
        packet.addControl("hid","pad_8_red",47,"B")
        packet.addControl("hid","pad_8_green",48,"B")
        packet.addControl("hid","pad_9_blue",49,"B")
        packet.addControl("hid","pad_9_red",50,"B")
        packet.addControl("hid","pad_9_green",51,"B")
        packet.addControl("hid","pad_10_blue",52,"B")
        packet.addControl("hid","pad_10_red",53,"B")
        packet.addControl("hid","pad_10_green",54,"B")
        packet.addControl("hid","pad_11_blue",55,"B")
        packet.addControl("hid","pad_11_red",56,"B")
        packet.addControl("hid","pad_11_green",57,"B")
        packet.addControl("hid","pad_12_blue",58,"B")
        packet.addControl("hid","pad_12_red",59,"B")
        packet.addControl("hid","pad_12_green",60,"B")
        packet.addControl("hid","pad_13_blue",61,"B")
        packet.addControl("hid","pad_13_red",62,"B")
        packet.addControl("hid","pad_13_green",63,"B")
        packet.addControl("hid","pad_14_blue",64,"B")
        packet.addControl("hid","pad_14_red",65,"B")
        packet.addControl("hid","pad_14_green",66,"B")
        packet.addControl("hid","pad_15_blue",67,"B")
        packet.addControl("hid","pad_15_red",68,"B")
        packet.addControl("hid","pad_15_green",69,"B")
        packet.addControl("hid","pad_16_blue",70,"B")
        packet.addControl("hid","pad_16_red",71,"B")
        packet.addControl("hid","pad_16_green",72,"B")

        // Stop key brightness control, 0-0xff
        packet.addControl("hid","stop_4_1_brightness",73,"B");
        packet.addControl("hid","stop_4_2_brightness",74,"B");
        packet.addControl("hid","stop_3_1_brightness",75,"B");
        packet.addControl("hid","stop_3_2_brightness",76,"B");
        packet.addControl("hid","stop_2_1_brightness",77,"B");
        packet.addControl("hid","stop_2_2_brightness",78,"B");
        packet.addControl("hid","stop_1_1_brightness",79,"B");
        packet.addControl("hid","stop_1_2_brightness",80,"B");

        this.controller.registerOutputPacket(packet);

    }

    this.initializeHIDController = function() {
        this.registerInputPackets();
        this.registerOutputPackets();
    }

    // Volume slider scaling for 0..1..5 scaling
    this.volumeScaler = function(group,name,value) {
        return script.absoluteLin(value, 0, 5, 0, 4096);
    }

    // Rate knob scaler
    this.rateScaler = function(group,name,value) {
        return -script.absoluteLin(value,-1,1,0,4096);
    }

    // EQ scaling function for 0..1..4 scaling
    this.eqScaler = function(group,name,value) {
        return script.absoluteNonLin(value, 0, 1, 4, 0, 4096);
    }

    // Set brightness for single color buttons with brigthness adjustment
    // Valid adjustment range is 0-0x7f
    this.setButtonBrightness = function(name,value) {
        var controller = this.controller;
        var packet = controller.getOutputPacket("lights");
        if (name.match(/pad_/)) {
            HIDDebug("ERROR: set PAD colors with setPADColor");
            return;
        }
        if (!name.match(/.*_brightness$/))
            name = name + "_brightness";
        var field = packet.getField("hid",name);
        if (field==undefined) {
            HIDDebug("button field not found: " + name);
            return;
        }
        if (value<0) 
            value = 0;
        if (value>0x7f)
            value = 0x7f;
        field.value = value;
    }

    // Set the 8 bytes in left or right 7-segment display. DP is the dot.
    this.set7SegmentValue = function(name,dp,v1,v2,v3,v4,v5,v6,v7,v8) {
        var controller = this.controller;
        var packet = controller.getOutputPacket("lights");
        var field = undefined;
        field = packet.getField("hid",name+"_segment_dp");
        field.value = dp;
        field = packet.getField("hid",name+"_segment_a");
        field.value = v1;
        field = packet.getField("hid",name+"_segment_b");
        field.value = v2;
        field = packet.getField("hid",name+"_segment_c");
        field.value = v3;
        field = packet.getField("hid",name+"_segment_d");
        field.value = v4;
        field = packet.getField("hid",name+"_segment_e");
        field.value = v5;
        field = packet.getField("hid",name+"_segment_f");
        field.value = v6;
        field = packet.getField("hid",name+"_segment_g");
        field.value = v7;
    }

    // Set RGB color for one of the 16 pads.
    // Index is pad number index as 1-16.
    // Valid range for each color is 0-0x7f.
    this.setPADColor = function(index,red,green,blue) {
        var controller = this.controller;
        var packet = controller.getOutputPacket("lights");
        var field = undefined;
        if (index<0 || index>16) {
            HIDDebug("Invalid pad index" + index);
            return;
        }
        if (red==undefined) 
            red=0;
        if (red>0x7f) 
            red=0x7f;
        if (green==undefined) 
            green=0;
        if (green>0x7f) 
            green=0x7f;
        if (blue==undefined) 
            blue=0;
        if (blue>0x7f) 
            blue=0x7f;
        field = packet.getField("hid","pad_"+index+"_red");
        field.value = red;
        field = packet.getField("hid","pad_"+index+"_green");
        field.value = green;
        field = packet.getField("hid","pad_"+index+"_blue");
        field.value = blue;
    }

    // reset all lights to off state
    this.resetLights = function() {
        var controller = this.controller;
        var packet = controller.getOutputPacket("lights");
        for (var group_name in packet.groups) {
            var group = packet.groups[group_name];
            for (var field_name in group) {
                var field = group[field_name]; 
                field.value = 0;
            } 
        }
        packet.send();
    }

    // Send update for LED packets after LED state modifications
    this.updateLEDs = function() {
        var packet = this.controller.getOutputPacket("lights");
        packet.send();
    }

}

KontrolF1Samplers = new KontrolF1Controller();

KontrolF1Samplers.init = function (id) {
    KontrolF1Samplers.id = id;
    KontrolF1Samplers.initializeHIDController();

    var controller = KontrolF1Samplers.controller;

    // Scratch parameters
    controller.scratchintervalsPerRev = 256;
    controller.scratchAlpha = 1.0/8;
    controller.rampedScratchEnable = true;
    controller.rampedScratchEnable = true;

    controller.postProcessDelta = KontrolF1Samplers.ButtonLEDPressUpdate;

    // Link controls and register callbacks
    KontrolF1Samplers.registerCallbacks();

    // Reset all lights to off state
    KontrolF1Samplers.resetLights();
    KontrolF1Samplers.initializeLEDs();

    //engine.softTakeover("[Master]","headVolume",true);
    //engine.softTakeover("[Master]","headMix",true);
    //for (var deck in KontrolF1Samplers.deckLEDColors) {
    //    engine.softTakeover("[Channel"+deck+"]","pregain",true);
    //    engine.softTakeover("[Channel"+deck+"]","volume",true);
    //}

    // Timers can't be defined in prototype with this.
    if (KontrolF1Samplers.LEDUpdateInterval!=undefined) {
        KontrolF1Samplers.LEDTimer = engine.beginTimer(
            KontrolF1Samplers.LEDUpdateInterval,
            "KontrolF1Samplers.controller.updateLEDs(true)"
        );
    }
    HIDDebug("NI Traktor F1 "+KontrolF1Samplers.id+" initialized");
}

// Device cleanup function
KontrolF1Samplers.shutdown = function() {
    if (KontrolF1Samplers.LEDTimer!=undefined) {
        engine.stopTimer(KontrolF1Samplers.LEDTimer);
        KontrolF1Samplers.LEDTimer = undefined;
    }

    //engine.softTakeover("[Master]","headVolume",false);
    //engine.softTakeover("[Master]","headMix",false);
    //for (var deck in KontrolF1Samplers.deckLEDColors) {
    //    engine.softTakeover("[Channel"+deck+"]","pregain",false);
    //    engine.softTakeover("[Channel"+deck+"]","volume",false);
    //}

    KontrolF1Samplers.shutdownHardware(2);
    HIDDebug("NI Traktor F1 "+KontrolF1Samplers.id+" shut down");
}

// Mandatory default handler for incoming packets
KontrolF1Samplers.incomingData = function(data,length) {
    KontrolF1Samplers.controller.parsePacket(data,length);
}

// Handle button LED updates after packet receive if required:
// F1 will reset LEDs to a default state without this
KontrolF1Samplers.ButtonLEDPressUpdate = function(packet,changed_data) {
    var send_led_update = false;
    for (var field in changed_data) {
        var delta = changed_data[field];
        var name = field.split(".")[1]
        // Select encoder also resets LEDs for some reason
        if (field=="select_encoder") {
            send_led_update = true;
            break;
        }
        // Update leds if any of these buttons were modified in packet
        if (KontrolF1Samplers.buttonNames.indexOf(name)!=-1) {
            if (delta.value==1)
                KontrolF1Samplers.setButtonBrightness(name,0x7f);
            if (delta.value==0)
                KontrolF1Samplers.setButtonBrightness(name,0);
            send_led_update = true;
            break;
        }
        // Update leds if any of pads was pressed
        if (/pad_[0-9]+_button/.test(name)) {
            send_led_update = true;
            break;
        }
    }
    if (send_led_update)
        KontrolF1Samplers.updateLEDs();
}

// Initialize LED states from mixxx sampler details
KontrolF1Samplers.initializeLEDs = function() {
    var value;
    for (var i=1;i<=4;i++) {
        var sampler = "[Sampler"+i+"]";
        value = (engine.getValue(sampler,"play")==1) ? 0x7f :0;
        KontrolF1Samplers.setButtonBrightness("stop_"+i+"_1",value);
        KontrolF1Samplers.setButtonBrightness("stop_"+i+"_2",value);

        for (var j=1;j<=4;j++) {
            var button = 4*(j-1)+i;
            if (engine.getValue(sampler,"hotcue_"+j+"_enabled"))
                value = KontrolF1Samplers.padColorSet[j];
            else    
                value = [0,0,0];
            KontrolF1Samplers.setPADColor(button,value[0],value[1],value[2]);
        }
    }

    KontrolF1Samplers.updateLEDs();
}

// Mandatory LED update callback handler
KontrolF1Samplers.activeLEDUpdateWrapper = function() {
    KontrolF1Samplers.controller.updateActiveDeckLEDs();
}

// Link virtual HID naming of input and LED controls to mixxx
// Note: HID specification has more fields than we map here. 
KontrolF1Samplers.registerCallbacks = function() {
    var controller = KontrolF1Samplers.controller;

    print("Registering HID callbacks");
    controller.linkControl("hid","knob_1","[Sampler1]","rate");
    controller.linkControl("hid","knob_2","[Sampler2]","rate");
    controller.linkControl("hid","knob_3","[Sampler3]","rate");
    controller.linkControl("hid","knob_4","[Sampler4]","rate");
    controller.linkControl("hid","knob_5","[Sampler1]","pregain");
    controller.linkControl("hid","knob_6","[Sampler2]","pregain");
    controller.linkControl("hid","knob_7","[Sampler3]","pregain");
    controller.linkControl("hid","knob_8","[Sampler4]","pregain");
    controller.setScaler("rate",KontrolF1Samplers.rateScaler);
    controller.setScaler("pregain",KontrolF1Samplers.volumeScaler);

    controller.linkControl("hid","select_encoder","[Playlist]","SelectTrackKnob");
    controller.linkControl("hid","select_push","[Playlist]","LoadSelectedIntoFirstStopped");
    controller.linkModifier("hid","shift","shift");

    controller.linkControl("hid","stop_1","[Sampler1]","play",KontrolF1Samplers.play_load);
    controller.linkControl("hid","stop_2","[Sampler2]","play",KontrolF1Samplers.play_load);
    controller.linkControl("hid","stop_3","[Sampler3]","play",KontrolF1Samplers.play_load);
    controller.linkControl("hid","stop_4","[Sampler4]","play",KontrolF1Samplers.play_load);

    controller.setCallback("control","hid","pad_1_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_2_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_3_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_4_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_5_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_6_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_7_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_8_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_9_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_10_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_11_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_12_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_13_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_14_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_15_button",KontrolF1Samplers.hotcue);
    controller.setCallback("control","hid","pad_16_button",KontrolF1Samplers.hotcue);

    engine.connectControl("[Sampler1]","play",KontrolF1Samplers.updateButtonLEDState);
    engine.connectControl("[Sampler1]","hotcue_1_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler1]","hotcue_2_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler1]","hotcue_3_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler1]","hotcue_4_enabled",KontrolF1Samplers.updatePadLEDState);

    engine.connectControl("[Sampler2]","play",KontrolF1Samplers.updateButtonLEDState);
    engine.connectControl("[Sampler2]","hotcue_1_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler2]","hotcue_2_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler2]","hotcue_3_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler2]","hotcue_4_enabled",KontrolF1Samplers.updatePadLEDState);

    engine.connectControl("[Sampler3]","play",KontrolF1Samplers.updateButtonLEDState);
    engine.connectControl("[Sampler3]","hotcue_1_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler3]","hotcue_2_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler3]","hotcue_3_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler3]","hotcue_4_enabled",KontrolF1Samplers.updatePadLEDState);

    engine.connectControl("[Sampler4]","play",KontrolF1Samplers.updateButtonLEDState);
    engine.connectControl("[Sampler4]","hotcue_1_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler4]","hotcue_2_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler4]","hotcue_3_enabled",KontrolF1Samplers.updatePadLEDState);
    engine.connectControl("[Sampler4]","hotcue_4_enabled",KontrolF1Samplers.updatePadLEDState);

}

KontrolF1Samplers.updateButtonLEDState = function(value,group,key) {
    if (! /^\[Sampler[0-9]+\]$/.test(group)) 
        return;
    var sampler = parseInt(group.substring(8,group.length-1));
    if (key=="play") {
        value = (value==1) ? 0x7f : 0;
        KontrolF1Samplers.setButtonBrightness("stop_"+sampler+"_1",value);
        KontrolF1Samplers.setButtonBrightness("stop_"+sampler+"_2",value);
        // Seek end of track decks back to start
        if (value==0 && engine.getValue(group,"playposition")>0.99) {
            engine.setValue(group,"playposition",0); 
        }
    }
    KontrolF1Samplers.updateLEDs();
}

KontrolF1Samplers.updatePadLEDState = function(value,group,key) {
    if (! /^\[Sampler[0-9]+\]$/.test(group)) 
        return;
    if (! /^hotcue_[0-9]_enabled$/.test(key))
        return;
    var sampler = parseInt(group.substring(8,group.length-1));
    var hotcue = parseInt(key.split("_")[1]);
    var button = 4*(hotcue-1) + sampler;
    var color;

    if (value==1)
        color = KontrolF1Samplers.padColorSet[hotcue];
    if (value==0)
        color = [0,0,0];
    if (color==undefined)
        return;
    KontrolF1Samplers.setPADColor(button,color[0],color[1],color[2]);
    KontrolF1Samplers.updateLEDs();
}

KontrolF1Samplers.play_load = function(field) {
    var controller = KontrolF1Samplers.controller;
    if (controller.modifiers.get("shift")) 
        engine.setValue(field.mapped_group,"LoadSelectedTrack",true);
    else 
        controller.togglePlay(field.mapped_group,field);
}

// Hotcues activated with normal press, cleared with shift
KontrolF1Samplers.hotcue = function (field) {
    var controller = KontrolF1Samplers.controller;
    var command;

    // Only valid for 'pad' buttons named pad_<number>_button
    if (! /pad_[0-9]+_button/.test(field.name))
        return;

    var color = undefined;
    var hotcue = undefined;
    var button = field.name.split("_")[1];
    var sampler = "[Sampler"+ (1+(button-1)%4) + "]";
    if (button>12) hotcue = 4;
    else if (button>8) hotcue = 3;
    else if (button>4) hotcue = 2;
    else hotcue = 1;
 
    if (controller.modifiers.get("shift")) {
        command = "hotcue_" + hotcue + "_clear";
    } else {
        if (field.value==1)
            color = KontrolF1Samplers.padColorActive[hotcue];
        if (field.value==0)
            if  (engine.getValue(sampler,"hotcue_"+hotcue+"_enabled"))
                color = KontrolF1Samplers.padColorSet[hotcue];
            else
                color = [0,0,0];
        if (color!=undefined)
            KontrolF1Samplers.setPADColor(button,color[0],color[1],color[2]);
        command = "hotcue_" + hotcue + "_activate";
    }

    if (field.value==1)
        engine.setValue(sampler,command,true);
}


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
    
    this.buttonNames = [
        "sync","quant","capture","shift",
        "reverse","type","size","browse",
        "play_1_1","play_1_2","play_2_1","play_2_2",
        "play_3_1","play_3_2","play_4_1","play_4_2"
    ]; 

    this.registerInputPackets = function() {
        var packet = undefined;

        packet = new HIDPacket("control",[0x1],22);
        packet.addControl("hid","grid_8",1,"I",0x1);
        packet.addControl("hid","grid_7",1,"I",0x2);
        packet.addControl("hid","grid_6",1,"I",0x4);
        packet.addControl("hid","grid_5",1,"I",0x8);
        packet.addControl("hid","grid_4",1,"I",0x10);
        packet.addControl("hid","grid_3",1,"I",0x20);
        packet.addControl("hid","grid_2",1,"I",0x40);
        packet.addControl("hid","grid_1",1,"I",0x80);
        packet.addControl("hid","grid_16",1,"I",0x100);
        packet.addControl("hid","grid_15",1,"I",0x200);
        packet.addControl("hid","grid_14",1,"I",0x400);
        packet.addControl("hid","grid_13",1,"I",0x800);
        packet.addControl("hid","grid_12",1,"I",0x1000);
        packet.addControl("hid","grid_11",1,"I",0x2000);
        packet.addControl("hid","grid_10",1,"I",0x4000);
        packet.addControl("hid","grid_9",1,"I",0x8000);
        packet.addControl("hid","shift",1,"I",0x800000);
        packet.addControl("hid","reverse",1,"I",0x400000);
        packet.addControl("hid","size",1,"I",0x100000);
        packet.addControl("hid","type",1,"I",0x200000);
        packet.addControl("hid","select_push",1,"I",0x40000);
        packet.addControl("hid","browse",1,"I",0x80000);
        packet.addControl("hid","play_1",1,"I",0x80000000);
        packet.addControl("hid","play_2",1,"I",0x40000000);
        packet.addControl("hid","play_3",1,"I",0x20000000);
        packet.addControl("hid","play_4",1,"I",0x10000000);
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
        packet.addControl("hid","fader_1",14,"H");
        packet.addControl("hid","fader_2",16,"H");
        packet.addControl("hid","fader_3",18,"H");
        packet.addControl("hid","fader_4",20,"H");
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
        packet.addControl("hid","grid_1_blue",25,"B")
        packet.addControl("hid","grid_1_red",26,"B")
        packet.addControl("hid","grid_1_green",27,"B")
        packet.addControl("hid","grid_2_blue",28,"B")
        packet.addControl("hid","grid_2_red",29,"B")
        packet.addControl("hid","grid_2_green",30,"B")
        packet.addControl("hid","grid_3_blue",31,"B")
        packet.addControl("hid","grid_3_red",32,"B")
        packet.addControl("hid","grid_3_green",33,"B")
        packet.addControl("hid","grid_4_blue",34,"B")
        packet.addControl("hid","grid_4_red",35,"B")
        packet.addControl("hid","grid_4_green",36,"B")
        packet.addControl("hid","grid_5_blue",37,"B")
        packet.addControl("hid","grid_5_red",38,"B")
        packet.addControl("hid","grid_5_green",39,"B")
        packet.addControl("hid","grid_6_blue",40,"B")
        packet.addControl("hid","grid_6_red",41,"B")
        packet.addControl("hid","grid_6_green",42,"B")
        packet.addControl("hid","grid_7_blue",43,"B")
        packet.addControl("hid","grid_7_red",44,"B")
        packet.addControl("hid","grid_7_green",45,"B")
        packet.addControl("hid","grid_8_blue",46,"B")
        packet.addControl("hid","grid_8_red",47,"B")
        packet.addControl("hid","grid_8_green",48,"B")
        packet.addControl("hid","grid_9_blue",49,"B")
        packet.addControl("hid","grid_9_red",50,"B")
        packet.addControl("hid","grid_9_green",51,"B")
        packet.addControl("hid","grid_10_blue",52,"B")
        packet.addControl("hid","grid_10_red",53,"B")
        packet.addControl("hid","grid_10_green",54,"B")
        packet.addControl("hid","grid_11_blue",55,"B")
        packet.addControl("hid","grid_11_red",56,"B")
        packet.addControl("hid","grid_11_green",57,"B")
        packet.addControl("hid","grid_12_blue",58,"B")
        packet.addControl("hid","grid_12_red",59,"B")
        packet.addControl("hid","grid_12_green",60,"B")
        packet.addControl("hid","grid_13_blue",61,"B")
        packet.addControl("hid","grid_13_red",62,"B")
        packet.addControl("hid","grid_13_green",63,"B")
        packet.addControl("hid","grid_14_blue",64,"B")
        packet.addControl("hid","grid_14_red",65,"B")
        packet.addControl("hid","grid_14_green",66,"B")
        packet.addControl("hid","grid_15_blue",67,"B")
        packet.addControl("hid","grid_15_red",68,"B")
        packet.addControl("hid","grid_15_green",69,"B")
        packet.addControl("hid","grid_16_blue",70,"B")
        packet.addControl("hid","grid_16_red",71,"B")
        packet.addControl("hid","grid_16_green",72,"B")

        // Play key brightness control, 0-0xff
        packet.addControl("hid","play_4_1_brightness",73,"B");
        packet.addControl("hid","play_4_2_brightness",74,"B");
        packet.addControl("hid","play_3_1_brightness",75,"B");
        packet.addControl("hid","play_3_2_brightness",76,"B");
        packet.addControl("hid","play_2_1_brightness",77,"B");
        packet.addControl("hid","play_2_2_brightness",78,"B");
        packet.addControl("hid","play_1_1_brightness",79,"B");
        packet.addControl("hid","play_1_2_brightness",80,"B");

        this.controller.registerOutputPacket(packet);

    }

    this.initializeHIDController = function() {
        this.scalers = new Object();
        this.scalers["volume"] = function(value) {
            return script.absoluteLin(value, 0, 1, 0, 4096);
        }
        this.scalers["pregain"] = function(value) {
            return script.absoluteNonLin(value,0,1,5,0,4096);
        }
        this.scalers["rate"] = function(value) {
            return script.absoluteLin(value,-1,1,0,4096);
        }
        this.scalers["ratereversed"] = function(value) {
            return -script.absoluteLin(value,-1,1,0,4096);
        }
        this.scalers["plusminus"] = function(value) {
            return script.absoluteLin(value,-1,1,0,4096);
        }
        this.scalers["eq"] = function(value) {
            return script.absoluteNonLin(value, 0, 1, 4, 0, 4096);
        }

        this.registerInputPackets();
        this.registerOutputPackets();
    }

    // Set brightness for single color buttons with brigthness adjustment
    // Valid adjustment range is 0-0x7f
    this.setButtonBrightness = function(name,value) {
        var controller = this.controller;
        var packet = controller.getOutputPacket("lights");
        if (name.match(/grid_/)) {
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
        if (index<=0 || index>16) {
            HIDDebug("Invalid grid index" + index);
            return;
        }
        if (red==undefined) 
            red=0;
        if (red>0x7f) 
            red=0x7f;
        field = packet.getField("hid","grid_"+index+"_red");
        field.value = red;
        if (green==undefined) 
            green=0;
        if (green>0x7f) 
            green=0x7f;
        field = packet.getField("hid","grid_"+index+"_green");
        field.value = green;
        if (blue==undefined) 
            blue=0;
        if (blue>0x7f) 
            blue=0x7f;
        field = packet.getField("hid","grid_"+index+"_blue");
        field.value = blue;
    }

    // reset all lights to off state
    this.resetLEDs = function() {
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

KontrolF1 = new KontrolF1Controller();

KontrolF1.init = function (id) {
    KontrolF1.id = id;

    KontrolF1.controlModeButtons = { "decks": "capture", "samplers": "quant" };
    KontrolF1.defaultControlMode = "decks";

    KontrolF1.initializeHIDController();
    var controller = KontrolF1.controller;

    KontrolF1.knobs = new Object();
    KontrolF1.faders = new Object();
    KontrolF1.grids = new Object();
    KontrolF1.playbuttons = new Object();

    controller.postProcessDelta = KontrolF1.ButtonLEDPressUpdate;

    KontrolF1.registerCallbacks();

    KontrolF1.resetLEDs();
    KontrolF1.setControlMode(KontrolF1.defaultControlMode);

    // Timers can't be defined in prototype with this.
    if (KontrolF1.LEDUpdateInterval!=undefined) {
        KontrolF1.LEDTimer = engine.beginTimer(
            KontrolF1.LEDUpdateInterval,
            "KontrolF1.controller.updateLEDs(true)"
        );
    }
    HIDDebug("NI Traktor F1 "+KontrolF1.id+" initialized");
}

// Device cleanup function
KontrolF1.shutdown = function() {
    if (KontrolF1.LEDTimer!=undefined) {
        engine.stopTimer(KontrolF1.LEDTimer);
        KontrolF1.LEDTimer = undefined;
    }

    KontrolF1.shutdownHardware(2);
    HIDDebug("NI Traktor F1 "+KontrolF1.id+" shut down");
}

// Mandatory default handler for incoming packets
KontrolF1.incomingData = function(data,length) {
    KontrolF1.controller.parsePacket(data,length);
}
 
// Mandatory LED update callback handler
KontrolF1.activeLEDUpdateWrapper = function() {
    KontrolF1.controller.updateActiveDeckLEDs();
}

// Handle button LED updates after packet receive if required:
// F1 will reset LEDs to a default state without this
// Registered as packet post processing callback in init.
KontrolF1.ButtonLEDPressUpdate = function(packet,changed_data) {
    var send_led_update = false;
    for (var field in changed_data) {
        var delta = changed_data[field];
        var name = field.split(".")[1]
        // Select encoder also resets LEDs for some reason
        if (field=="select_encoder") {
            send_led_update = true;
            break;
        }
        // Check if this is one of permanently lit LEDs
        var controlmode = false;
        for (mode in KontrolF1.controlModeButtons) {
            if (KontrolF1.controlModeButtons[mode]==name) {
                controlmode = true;
                break;
            }
        }
        if (controlmode) {
            send_led_update = true;
            break;
        }

        // Update leds if any of these buttons were modified in packet
        if (KontrolF1.buttonNames.indexOf(name)!=-1) {
            if (delta.value==1)
                KontrolF1.setButtonBrightness(name,0x7f);
            if (delta.value==0)
                KontrolF1.setButtonBrightness(name,0);
            send_led_update = true;
            break;
        }
        // Update leds if any of pads was pressed
        if (/grid_[0-9]/.test(name)) {
            send_led_update = true;
            break;
        }
    }
    if (send_led_update)
        KontrolF1.updateLEDs();
}

KontrolF1.disconnectModeLEDs = function(mode) {
    var grid = KontrolF1.grids[mode];
    var buttons = KontrolF1.playbuttons[mode];
    for (var name in grid) {
        var button = grid[name];
        if (button.ledname==undefined)
            continue
        engine.connectControl(button.group, button.ledname,
            KontrolF1.setLED, false
        );
    }
    for (var name in buttons) {
        var button = buttons[name];
        var button_index = parseInt(name.split("_")[1]);
        if (button.ledname==undefined)
            continue;
        engine.connectControl(button.group, button.ledname,
            KontrolF1.setLED, false
        );
    }
}

KontrolF1.connectModeLEDs = function(mode) {
    var grid = KontrolF1.grids[mode];
    var buttons = KontrolF1.playbuttons[mode];

    var color;
    for (var name in grid) {
        var button = grid[name];
        var button_index = parseInt(name.split("_")[1]);
        if (button.ledname==undefined)
            continue;
        engine.connectControl(button.group, button.ledname,
            KontrolF1.setLED
        );
        if (engine.getValue(button.group, button.ledname))
            value = button.ledcolor;
        else
            value = 0,0,0; 
        KontrolF1.setPADColor(button_index,value[0],value[1],value[2]);
    }
    for (var name in buttons) {
        var button = buttons[name];
        var button_index = parseInt(name.split("_")[1]);
        if (button.ledname==undefined)
            continue;
        engine.connectControl(button.group, button.ledname,
            KontrolF1.setLED
        );
        value = (engine.getValue(button.group,button.name)==1) ? 0x7f :0;
        KontrolF1.setButtonBrightness("play_"+button_index+"_1",value);
        KontrolF1.setButtonBrightness("play_"+button_index+"_2",value);
    }
    KontrolF1.updateLEDs();
}

KontrolF1.setLED = function(value,group,key) {
    var grid = KontrolF1.grids[KontrolF1.controlMode];
    var buttons = KontrolF1.playbuttons[KontrolF1.controlMode];
    var matched = false;
    
    for (var name in grid) {
        var button = grid[name];
        var button_index = parseInt(name.split("_")[1]);
        if (button.group!=group || button.ledname!=key) 
            continue;
        if (value)
            value = button.ledcolor;
        else
            value = 0,0,0; 
        KontrolF1.setPADColor(button_index,value[0],value[1],value[2]);
        matched = true;
        break;
    }
    for (var name in buttons) { 
        var button = buttons[name];
        var button_index = parseInt(name.split("_")[1]);
        if (button.group!=group || button.name!=key) 
            continue;
        value = (value==1) ? 0x7f : 0;
        KontrolF1.setButtonBrightness("play_"+button_index+"_1",value);
        KontrolF1.setButtonBrightness("play_"+button_index+"_2",value);
        matched = true;
        break;
    }
    KontrolF1.updateLEDs();
}

KontrolF1.linkKnob = function(mode,knob,group,name,scaler) {
    if (!(mode in KontrolF1.knobs))
        KontrolF1.knobs[mode] = new Object();
    var mapping = new Object();
    mapping.mode = mode;
    mapping.knob = knob;
    mapping.group = group;
    mapping.name = name;
    mapping.scaler = scaler;
    KontrolF1.knobs[mode][knob] = mapping;
}

KontrolF1.knob = function(field) {
    var controller = KontrolF1.controller;
    var mode = KontrolF1.knobs[KontrolF1.controlMode];
    if (mode==undefined) {
        HIDDebug("Knob group not mapped in mode " + KontrolF1.controlMode);
        return;
    }
    var knob = mode[field.name];
    if (knob==undefined) {
        HIDDebug("Fader "+field.name+ " not mapped in " + KontrolF1.controlMode);
        return;
    }
    return KontrolF1.control(knob,field);
}

KontrolF1.linkFader = function(mode,fader,group,name,scaler,callback) {
    if (!(mode in KontrolF1.faders))
        KontrolF1.faders[mode] = new Object();
    var mapping = new Object();
    mapping.mode = mode;
    mapping.fader = fader;
    mapping.group = group;
    mapping.name = name;
    mapping.scaler = scaler;
    mapping.callback = callback;
    KontrolF1.faders[mode][fader] = mapping;
}

KontrolF1.fader = function(field) {
    var controller = KontrolF1.controller;
    var mode = KontrolF1.faders[KontrolF1.controlMode];
    if (mode==undefined) {
        HIDDebug("Fader group not mapped in mode " + KontrolF1.controlMode);
        return;
    }
    var fader = mode[field.name];
    if (fader==undefined) {
        HIDDebug("Fader "+field.name+ " not mapped in " + KontrolF1.controlMode);
        return;
    }
    return KontrolF1.control(fader,field);
}

KontrolF1.linkGrid = function(mode,button,group,name,toggle,callback,ledcolor,ledname) {
    if (!(mode in KontrolF1.grids)) 
        KontrolF1.grids[mode] = new Object();
    if (ledname==undefined) {
        if (name.match(/hotcue_[0-9]/))
            ledname = name + '_enabled';
        else
            ledname = name;
    }
    if (ledcolor==undefined) {
        ledcolor = [0x7f,0x7f,0x7f];
    }
    var mapping = new Object();
    mapping.mode = mode;
    mapping.button = button;
    mapping.group = group;
    mapping.name = name;
    mapping.toggle = toggle;
    mapping.ledname = ledname;
    mapping.ledcolor = ledcolor;
    mapping.callback = callback;
    KontrolF1.grids[mode][button] = mapping;
}

KontrolF1.grid = function(field) {
    var mode = KontrolF1.grids[KontrolF1.controlMode];
    if (mode==undefined) {
        HIDDebug("Grid button group not mapped in " + KontrolF1.controlMode);
        return;
    }
    var button = mode[field.name];
    if (button==undefined) {
        HIDDebug("Grid "+field.name+ " not mapped in " + KontrolF1.controlMode);
        return;
    }
    return KontrolF1.button(button,field);
}

KontrolF1.linkPlay = function(mode,button,group,name,toggle,callback,ledname) {
    if (!(mode in KontrolF1.playbuttons))
        KontrolF1.playbuttons[mode] = new Object();

    if (ledname==undefined) {
        if (name.match(/hotcue_[0-9]/))
            ledname = name + '_enabled';
        else
            ledname = name;
    }
    var mapping = new Object();
    mapping.mode = mode;
    mapping.button = button;
    mapping.group = group;
    mapping.name = name;
    mapping.toggle = toggle;
    mapping.ledname = ledname;
    mapping.callback = callback;
    KontrolF1.playbuttons[mode][button] = mapping;
}

KontrolF1.play = function(field) {
    var mode = KontrolF1.playbuttons[KontrolF1.controlMode];
    if (mode==undefined) {
        HIDDebug("Play button group not mapped in " + KontrolF1.controlMode);
        return;
    }
    var button = mode[field.name];
    if (button==undefined) {
        HIDDebug("Play button "+field.name+ " not mapped in " + KontrolF1.controlMode);
        return;
    }
    return KontrolF1.button(button,field);
}

KontrolF1.control = function(control,field) {
    if (control.callback!=undefined) {
        control.callback(control,field);
        return;
    }
    var scaler = KontrolF1.scalers[control.scaler];
    engine.setValue(control.group,control.name,scaler(field.value));
}

KontrolF1.button = function(button,field) {
    var controller = KontrolF1.controller;
    if (button.callback!=undefined) {
        button.callback(button,field);
        return;
    }
    if (button.toggle) {
        if (button.name=='play')
            controller.togglePlay(button.group,field);
        else
            controller.toggle(button.group,button.name,field.value);
    } else {
        var value = (field.value==1) ? true : false;
        engine.setValue(button.group,button.name,value);
    }
}

KontrolF1.switchControlMode = function(field) {
    if (field.name=='quant') {
        KontrolF1.setControlMode("samplers");
    } else if (field.name=='capture') {
        KontrolF1.setControlMode("decks");
    } else {
        HIDDebug("Unconfigured mode selector button: " + field.name);
        return;
    }
}

KontrolF1.setControlMode = function(mode) {
    if (mode==KontrolF1.controlMode) 
        return;
    if (!(mode in KontrolF1.controlModeButtons)) {
        HIDDebug("Unconfigured control mode: " + mode);
        return;
    } 
    
    if (KontrolF1.controlMode!=undefined) {
        KontrolF1.disconnectModeLEDs(KontrolF1.controlMode);
        KontrolF1.setButtonBrightness(
            KontrolF1.controlModeButtons[KontrolF1.controlMode],0
        );
    }
    KontrolF1.controlMode = mode;
    led = KontrolF1.controlModeButtons[mode];
    KontrolF1.connectModeLEDs(KontrolF1.controlMode);
    KontrolF1.setButtonBrightness(
        KontrolF1.controlModeButtons[KontrolF1.controlMode],0x7f
    );
    KontrolF1.updateLEDs();
}

KontrolF1.registerCallbacks = function() {
    var controller = KontrolF1.controller;

    HIDDebug("Registering HID callbacks");

    controller.linkControl("hid","select_encoder","[Playlist]","SelectTrackKnob");
    controller.linkControl("hid","select_push","[Playlist]","LoadSelectedIntoFirstStopped");
    controller.linkModifier("hid","shift","shift");

    controller.setCallback("control","hid","capture",KontrolF1.switchControlMode);
    controller.setCallback("control","hid","quant",KontrolF1.switchControlMode);

    controller.setCallback("control","hid","knob_1",KontrolF1.knob);
    controller.setCallback("control","hid","knob_2",KontrolF1.knob);
    controller.setCallback("control","hid","knob_3",KontrolF1.knob);
    controller.setCallback("control","hid","knob_4",KontrolF1.knob);

    controller.setCallback("control","hid","fader_1",KontrolF1.fader);
    controller.setCallback("control","hid","fader_2",KontrolF1.fader);
    controller.setCallback("control","hid","fader_3",KontrolF1.fader);
    controller.setCallback("control","hid","fader_4",KontrolF1.fader);

    controller.setCallback("control","hid","grid_1",KontrolF1.grid);
    controller.setCallback("control","hid","grid_2",KontrolF1.grid);
    controller.setCallback("control","hid","grid_3",KontrolF1.grid);
    controller.setCallback("control","hid","grid_4",KontrolF1.grid);
    controller.setCallback("control","hid","grid_5",KontrolF1.grid);
    controller.setCallback("control","hid","grid_6",KontrolF1.grid);
    controller.setCallback("control","hid","grid_7",KontrolF1.grid);
    controller.setCallback("control","hid","grid_8",KontrolF1.grid);
    controller.setCallback("control","hid","grid_9",KontrolF1.grid);
    controller.setCallback("control","hid","grid_10",KontrolF1.grid);
    controller.setCallback("control","hid","grid_11",KontrolF1.grid);
    controller.setCallback("control","hid","grid_12",KontrolF1.grid);
    controller.setCallback("control","hid","grid_13",KontrolF1.grid);
    controller.setCallback("control","hid","grid_14",KontrolF1.grid);
    controller.setCallback("control","hid","grid_15",KontrolF1.grid);
    controller.setCallback("control","hid","grid_16",KontrolF1.grid);

    controller.setCallback("control","hid","play_1",KontrolF1.play);
    controller.setCallback("control","hid","play_2",KontrolF1.play);
    controller.setCallback("control","hid","play_3",KontrolF1.play);
    controller.setCallback("control","hid","play_4",KontrolF1.play);

    KontrolF1.linkKnob("decks","knob_1","[Master]","headVolume","pregain");
    KontrolF1.linkKnob("decks","knob_2","[Master]","headMix","plusminus");
    KontrolF1.linkKnob("decks","knob_3","[Master]","balance","plusminus");
    KontrolF1.linkKnob("decks","knob_4","[Master]","volume","pregain");
    KontrolF1.linkFader("decks","fader_1","[Channel1]","rate","rate");
    KontrolF1.linkFader("decks","fader_2","[Channel1]","volume","volume");
    KontrolF1.linkFader("decks","fader_3","[Channel2]","volume","volume");
    KontrolF1.linkFader("decks","fader_4","[Channel2]","rate","rate");
    KontrolF1.linkGrid("decks","grid_1","[Channel1]","hotcue_1",false,KontrolF1.hotcue,[0x7f,0x7f,0x0]);
    KontrolF1.linkGrid("decks","grid_2","[Channel1]","hotcue_2",false,KontrolF1.hotcue,[0x7f,0x0,0x7f]);
    KontrolF1.linkGrid("decks","grid_3","[Channel2]","hotcue_1",false,KontrolF1.hotcue,[0x7f,0x7f,0x0]);
    KontrolF1.linkGrid("decks","grid_4","[Channel2]","hotcue_2",false,KontrolF1.hotcue,[0x7f,0x0,0x7f]);
    KontrolF1.linkGrid("decks","grid_5","[Channel1]","hotcue_3",false,KontrolF1.hotcue,[0x0,0x40,0x7f]);
    KontrolF1.linkGrid("decks","grid_6","[Channel1]","hotcue_4",false,KontrolF1.hotcue,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("decks","grid_7","[Channel2]","hotcue_3",false,KontrolF1.hotcue,[0x0,0x40,0x7f]);
    KontrolF1.linkGrid("decks","grid_8","[Channel2]","hotcue_4",false,KontrolF1.hotcue,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("decks","grid_9","[Channel1]","loop_in",false,undefined,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("decks","grid_10","[Channel1]","loop_out",false,undefined,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("decks","grid_11","[Channel2]","loop_in",false,undefined,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("decks","grid_12","[Channel2]","loop_out",false,undefined,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("decks","grid_13","[Channel1]","quantize",true,undefined,[0x10,0x10,0x40]);
    KontrolF1.linkGrid("decks","grid_14","[Channel1]","reloop_exit",false,undefined,[0x7f,0x0,0x0]);
    KontrolF1.linkGrid("decks","grid_15","[Channel2]","quantize",true,undefined,[0x10,0x10,0x40]);
    KontrolF1.linkGrid("decks","grid_16","[Channel2]","reloop_exit");
    KontrolF1.linkPlay("decks","play_1","[Channel1]","play",true);
    KontrolF1.linkPlay("decks","play_2","[Channel1]","cue_default");
    KontrolF1.linkPlay("decks","play_3","[Channel2]","play",true);
    KontrolF1.linkPlay("decks","play_4","[Channel2]","cue_default");

    KontrolF1.linkKnob("samplers","knob_1","[Sampler1]","rate","ratereversed");
    KontrolF1.linkKnob("samplers","knob_2","[Sampler2]","rate","ratereversed");
    KontrolF1.linkKnob("samplers","knob_3","[Sampler3]","rate","ratereversed");
    KontrolF1.linkKnob("samplers","knob_4","[Sampler4]","rate","ratereversed");
    KontrolF1.linkFader("samplers","fader_1","[Sampler1]","pregain","pregain");
    KontrolF1.linkFader("samplers","fader_2","[Sampler2]","pregain","pregain");
    KontrolF1.linkFader("samplers","fader_3","[Sampler3]","pregain","pregain");
    KontrolF1.linkFader("samplers","fader_4","[Sampler4]","pregain","pregain");
    KontrolF1.linkGrid("samplers","grid_1","[Sampler1]","hotcue_1",false,KontrolF1.hotcue,[0x7f,0x0,0x0]);
    KontrolF1.linkGrid("samplers","grid_2","[Sampler2]","hotcue_1",false,KontrolF1.hotcue,[0x7f,0x0,0x0]);
    KontrolF1.linkGrid("samplers","grid_3","[Sampler3]","hotcue_1",false,KontrolF1.hotcue,[0x7f,0x0,0x0]);
    KontrolF1.linkGrid("samplers","grid_4","[Sampler4]","hotcue_1",false,KontrolF1.hotcue,[0x7f,0x0,0x0]);
    KontrolF1.linkGrid("samplers","grid_5","[Sampler1]","hotcue_2",false,KontrolF1.hotcue,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("samplers","grid_6","[Sampler2]","hotcue_2",false,KontrolF1.hotcue,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("samplers","grid_7","[Sampler3]","hotcue_2",false,KontrolF1.hotcue,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("samplers","grid_8","[Sampler4]","hotcue_2",false,KontrolF1.hotcue,[0x0,0x7f,0x0]);
    KontrolF1.linkGrid("samplers","grid_9","[Sampler1]","hotcue_3",false,KontrolF1.hotcue,[0x0,0x0,0x7f]);
    KontrolF1.linkGrid("samplers","grid_10","[Sampler2]","hotcue_3",false,KontrolF1.hotcue,[0x0,0x0,0x7f]);
    KontrolF1.linkGrid("samplers","grid_11","[Sampler3]","hotcue_3",false,KontrolF1.hotcue,[0x0,0x0,0x7f]);
    KontrolF1.linkGrid("samplers","grid_12","[Sampler4]","hotcue_3",false,KontrolF1.hotcue,[0x0,0x0,0x7f]);
    KontrolF1.linkGrid("samplers","grid_13","[Sampler1]","hotcue_4",false,KontrolF1.hotcue,[0x0,0x7f,0x7f]);
    KontrolF1.linkGrid("samplers","grid_14","[Sampler2]","hotcue_4",false,KontrolF1.hotcue,[0x0,0x7f,0x7f]);
    KontrolF1.linkGrid("samplers","grid_15","[Sampler3]","hotcue_4",false,KontrolF1.hotcue,[0x0,0x7f,0x7f]);
    KontrolF1.linkGrid("samplers","grid_16","[Sampler4]","hotcue_4",false,KontrolF1.hotcue,[0x0,0x7f,0x7f]);
    KontrolF1.linkPlay("samplers","play_1","[Sampler1]","play",true);
    KontrolF1.linkPlay("samplers","play_2","[Sampler2]","play",true);
    KontrolF1.linkPlay("samplers","play_3","[Sampler3]","play",true);
    KontrolF1.linkPlay("samplers","play_4","[Sampler4]","play",true);
}

KontrolF1.hotcue = function(button,field) { 
    var controller = KontrolF1.controller;
    var name;
    if (field.value == controller.buttonStates.released)
        return;
    if (controller.modifiers.get("shift")) 
        name = button.name + '_clear';
    else 
        name = button.name + '_activate';
    engine.setValue(button.group,name,true);
}


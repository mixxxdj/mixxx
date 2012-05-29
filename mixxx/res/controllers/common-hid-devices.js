
// Sony playstation SixxAxis controller example
function SonySixxAxisController() {
    this.controller = new HIDController();

    this.registerInputPackets = function() {
        packet = new HIDPacket('control',[],49,this.dump);

        // Toggle buttons
        packet.addControl('hid','select',2,'B',0x1);
        packet.addControl('hid','jog_buttons',2,'B',0x6);
        //packet.addControl('hid','jog_left_button',2,'B',0x2);
        //packet.addControl('hid','jog_right_button',2,'B',0x4);
        packet.addControl('hid','start',2,'B',0x8);
        packet.addControl('hid','arrow_button_up',2,'B',0x10);
        packet.addControl('hid','arrow_button_right',2,'B',0x20);
        packet.addControl('hid','arrow_button_down',2,'B',0x40);
        packet.addControl('hid','arrow_button_left',2,'B',0x80);
        packet.addControl('hid','front_button_bottom_left',3,'B',0x1);
        packet.addControl('hid','front_button_bottom_right',3,'B',0x2);
        packet.addControl('hid','front_button_top_left',3,'B',0x4);
        packet.addControl('hid','front_button_top_right',3,'B',0x8);
        packet.addControl('hid','button_triangle',3,'B',0x10);
        packet.addControl('hid','button_circle',3,'B',0x20);
        packet.addControl('hid','button_cross',3,'B',0x40);
        packet.addControl('hid','button_square',3,'B',0x80);

        // 0x0 left, 0xff right
        packet.addControl('hid','jog_left_x',0x6,'B');
        packet.addControl('hid','jog_right_x',0x8,'B');
        // 0x0 top, 0xff bottom
        packet.addControl('hid','jog_left_y',0x7,'B');
        packet.addControl('hid','jog_right_y',0x9,'B');

        // Toggle button pressure sensitivity 0x00-0xff
        packet.addControl('hid','arrow_pressure_up',0x0e,'B');
        packet.addControl('hid','arrow_pressure_right',0x0f,'B');
        packet.addControl('hid','arrow_pressure_down',0x10,'B');
        packet.addControl('hid','arrow_pressure_left',0x11,'B');
        packet.addControl('hid','front_pressure_bottom_left',0x12,'B');
        packet.addControl('hid','front_pressure_bottom_right',0x13,'B');
        packet.addControl('hid','front_pressure_top_left',0x14,'B');
        packet.addControl('hid','front_pressure_top_right',0x15,'B');
        packet.addControl('hid','pressure_triangle',0x16,'B');
        packet.addControl('hid','pressure_circle',0x17,'B');
        packet.addControl('hid','pressure_cross',0x18,'B');
        packet.addControl('hid','pressure_square',0x19,'B');

        // These controls send lots of +-1 delta when not touched
        packet.setMinDelta('hid','jog_right_x',2);
        packet.setMinDelta('hid','jog_right_y',2);
        packet.setMinDelta('hid','jog_left_x',2);
        packet.setMinDelta('hid','jog_left_y',2);

        packet.setMinDelta('hid','arrow_pressure_up',1);
        packet.setMinDelta('hid','arrow_pressure_right',1);
        packet.setMinDelta('hid','arrow_pressure_down',1);
        packet.setMinDelta('hid','arrow_pressure_left',1);
        packet.setMinDelta('hid','front_pressure_bottom_left',1);
        packet.setMinDelta('hid','front_pressure_bottom_right',1);
        packet.setMinDelta('hid','front_pressure_top_left',1);
        packet.setMinDelta('hid','pressure_triangle',1);
        packet.setMinDelta('hid','pressure_circle',1);
        packet.setMinDelta('hid','pressure_cross',1);
        packet.setMinDelta('hid','pressure_square',1);

        this.controller.registerInputPacket(packet);
    }

    // HID trackpads have no output controls
    this.registerOutputPackets = function() { }

    // No need to do scaling for keyboard presses
    this.registerScalers = function() { }

    // No need for callbacks here, we bound whole input packet to mouseInput
    this.registerCallbacks = function() { }

    this.dump = function(packet,delta) {
        for (field_name in delta) {
            var field = delta[field_name];
            if (field.bitmask==undefined)
                continue;
            script.HIDDebug('SixxAxis ' + field.id + ' VALUE ' + field.value);
        }   
    }
}

// Generic HID trackpad implementation
function HIDTrackpad() {
    this.controller = new HIDController();

    this.registerInputPackets = function() {
        // Example how to register a callback directly on a packet
        packet = new HIDPacket('control',[],4,this.mouseInput);
        packet.addControl('hid','byte_1',3,'B');
        packet.addControl('hid','byte_2',4,'B');
        packet.addControl('hid','byte_3',5,'B');
        packet.addControl('hid','byte_4',6,'B');
        this.controller.registerInputPacket(packet);
    }

    // HID trackpads have no output controls
    this.registerOutputPackets = function() { }

    // No need to do scaling for keyboard presses
    this.registerScalers = function() { }

    // No need for callbacks here, we bound whole input packet to mouseInput
    this.registerCallbacks = function() { }

    // Example to process the mouse input packet all yourself
    this.mouseInput = function(packet,delta) {
        script.HIDDebug("Trackpad INPUT " + packet.name);
        if (!delta.length) {
            script.HIDDebug("No changed data received in HID packet");
            return;
        }
        for (var field_name in delta) {
            var field = delta[field_name];
            script.HIDDebug("FIELD " + field.id + " VALUE " + value);
        }
    }
}

// Generic HID keyboard implementation 
function HIDKeyboard() {
    this.controller = new HIDController();

    this.registerInputPackets = function() {
        packet = new HIDPacket('control',[0x1,0x0,0x0],9);
        packet.addControl('hid','keycode_1',3,'B');
        packet.addControl('hid','keycode_2',4,'B');
        packet.addControl('hid','keycode_3',5,'B');
        packet.addControl('hid','keycode_4',6,'B');
        packet.addControl('hid','keycode_5',7,'B');
        packet.addControl('hid','keycode_6',8,'B');
        this.controller.registerInputPacket(packet);
    }

    // HID keyboards have no output controls
    this.registerOutputPackets = function() { }

    // No need to do scaling for keyboard presses
    this.registerScalers = function() { }

    // Example to bind the bytes to a callback
    this.registerCallbacks = function() { 
        this.controller.registerInputCallback('control','hid','keycode_1',this.keyPress);
        this.controller.registerInputCallback('control','hid','keycode_2',this.keyPress);
        this.controller.registerInputCallback('control','hid','keycode_3',this.keyPress);
        this.controller.registerInputCallback('control','hid','keycode_4',this.keyPress);
        this.controller.registerInputCallback('control','hid','keycode_5',this.keyPress);
        this.controller.registerInputCallback('control','hid','keycode_6',this.keyPress);
    }

    // Example to do something with the keycodes received
    this.keyPress = function(field) {
        if (field.value!=0) 
            script.HIDDebug("KEY PRESS " + field.id + " CODE " + field.value);
        else
            script.HIDDebug("KEY RELEASE " + field.id); 
    }
}


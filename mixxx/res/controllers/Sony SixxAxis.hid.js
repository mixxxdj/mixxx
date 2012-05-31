// 
// Sony playstation SixxAxis controller HID packet declaration
// 
// This model does not register any of the packet fields directly to
// mixxx controls. Please register callbacks to fields named here in
// the script using SonySixxAxisController() object and parse events
// yourself in callbacks.
//

function SonySixxAxisController() {
    this.controller = new HIDController();

    this.registerInputPackets = function() {
        // Example call for debugging: calling packet level 
        // callback ignores all field scalers and callbacks
        // packet = new HIDPacket('control',[],49,this.dump);
        packet = new HIDPacket('control',[],49);

        // Toggle buttons
        packet.addControl('hid','select',2,'B',0x1);
        packet.addControl('hid','jog_left_button',2,'B',0x2);
        packet.addControl('hid','jog_right_button',2,'B',0x4);
        packet.addControl('hid','start',2,'B',0x8);
        packet.addControl('hid','arrow_button_up',2,'B',0x10);
        packet.addControl('hid','arrow_button_right',2,'B',0x20);
        packet.addControl('hid','arrow_button_down',2,'B',0x40);
        packet.addControl('hid','arrow_button_left',2,'B',0x80);
        packet.addControl('hid','button_bottom_left',3,'B',0x1);
        packet.addControl('hid','button_bottom_right',3,'B',0x2);
        packet.addControl('hid','button_top_left',3,'B',0x4);
        packet.addControl('hid','button_top_right',3,'B',0x8);
        packet.addControl('hid','button_triangle',3,'B',0x10);
        packet.addControl('hid','button_circle',3,'B',0x20);
        packet.addControl('hid','button_cross',3,'B',0x40);
        packet.addControl('hid','button_square',3,'B',0x80);

        // 0x0 left, 0xff right
        packet.addControl('hid','jog_left_x',0x6,'B');
        packet.setMinDelta('hid','jog_left_x',2);
        packet.addControl('hid','jog_right_x',0x8,'B');
        packet.setMinDelta('hid','jog_right_x',2);

        // 0x0 top, 0xff bottom
        packet.addControl('hid','jog_left_y',0x7,'B');
        packet.setMinDelta('hid','jog_left_y',2);
        packet.addControl('hid','jog_right_y',0x9,'B');
        packet.setMinDelta('hid','jog_right_y',2);

        // Toggle button pressure sensitivity 0x00-0xff
        packet.addControl('hid','arrow_pressure_up',0x0e,'B');
        packet.setMinDelta('hid','arrow_pressure_up',1);
        packet.addControl('hid','arrow_pressure_right',0x0f,'B');
        packet.setMinDelta('hid','arrow_pressure_right',1);
        packet.addControl('hid','arrow_pressure_down',0x10,'B');
        packet.setMinDelta('hid','arrow_pressure_down',1);
        packet.addControl('hid','arrow_pressure_left',0x11,'B');
        packet.setMinDelta('hid','arrow_pressure_left',1);
        packet.addControl('hid','pressure_bottom_left',0x12,'B');
        packet.setMinDelta('hid','pressure_bottom_left',4);
        packet.addControl('hid','pressure_bottom_right',0x13,'B');
        packet.setMinDelta('hid','pressure_bottom_right',4);
        packet.addControl('hid','pressure_top_left',0x14,'B');
        packet.setMinDelta('hid','pressure_top_left',4);
        packet.addControl('hid','pressure_top_right',0x15,'B');
        packet.setMinDelta('hid','pressure_top_right',4);
        packet.addControl('hid','pressure_triangle',0x16,'B');
        packet.setMinDelta('hid','pressure_triangle',1);
        packet.addControl('hid','pressure_circle',0x17,'B');
        packet.setMinDelta('hid','pressure_circle',1);
        packet.addControl('hid','pressure_cross',0x18,'B');
        packet.setMinDelta('hid','pressure_cross',1);
        packet.addControl('hid','pressure_square',0x19,'B');
        packet.setMinDelta('hid','pressure_square',1);

        // Gyro pitch (sideways movement)
        packet.addControl('hid','gyro_pitch',0x2a,'B');
        packet.registerCallback('hid','gyro_pitch',this.gyro);
        packet.setMinDelta('hid','gyro_pitch',4);
        // Gyro roll (forward movement)
        packet.addControl('hid','gyro_roll',0x2c,'B');
        packet.setMinDelta('hid','gyro_roll',4);
        packet.registerCallback('hid','gyro_roll',this.gyro);
        // Gyro gravity position
        packet.addControl('hid','gyro_gravity',0x2e,'B');
        packet.setMinDelta('hid','gyro_gravity',4);
        packet.registerCallback('hid','gyro_gravity',this.gyro);

        this.controller.registerInputPacket(packet);
    }

    // Sixxaxis has no output controls
    this.registerOutputPackets = function() { }

    // No default scalers: all controls done with callbacks anyway
    this.registerScalers = function() { }

    // Register your own callbacks in caller by overriding this
    this.registerCallbacks = function() { }

    // Default dummy callback for gyro events
    this.gyro = function(field) { return; }

    // Example of packet callback: 
    // dumps changes (delta) in all packet fields
    this.dump = function(packet,delta) {
        for (field_name in delta) {
            var field = delta[field_name];
            script.HIDDebug('SixxAxis ' + field.id + ' VALUE ' + field.value);
        }   
    }

}


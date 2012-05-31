// 
// Nintendo Wii Remote controller HID packet declaration
// 
// This model does not register any of the packet fields directly to
// mixxx controls. Please register callbacks to fields named here in
// the script using WiimoteController() object and parse events
// yourself in callbacks.
// 
// Details for Wiimote HID packet data can be found in:
// http://wiibrew.org/wiki/Wiimote
//
// Note: until I figure out why my Wii does not accept output packets
// for LED and mode control, this mapping is for the basic mode only
// with buttons: no rumble, LED control or accelerometer data.
//

function WiimoteController() {
    this.controller = new HIDController();

    this.registerInputPackets = function() {
        // Example call for debugging: calling packet level 
        // callback ignores all field scalers and callbacks
        //packet = new HIDPacket('control',[],3,this.dump);

        // Basic button toggle input packet: packet is longer when
        // we figure out how to enable other data than buttons
        packet = new HIDPacket('control',[],3);

        packet.addControl('hid','arrow_left',1,'B',0x1);
        packet.addControl('hid','arrow_right',1,'B',0x2);
        packet.addControl('hid','arrow_down',1,'B',0x4);
        packet.addControl('hid','arrow_up',1,'B',0x8);
        packet.addControl('hid','button_plus',1,'B',0x10);
        packet.addControl('hid','button_1',2,'B',0x2);
        packet.addControl('hid','button_2',2,'B',0x1);
        packet.addControl('hid','button_bottom',2,'B',0x4);
        packet.addControl('hid','button_a',2,'B',0x8);
        packet.addControl('hid','button_minus',2,'B',0x10);
        packet.addControl('hid','button_home',2,'B',0x80);

        this.controller.registerInputPacket(packet);
    }

    // Wiimote has output controls, but I could not figure out how
    // to send the packets, I'm just getting a general error response.
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
            script.HIDDebug('Wiimote ' + field.id + ' VALUE ' + field.value);
        }   
    }

}


//
// Nintendo Wii Remote Game Controller HID mapping
// Copyright (C) 2012, Ilkka Tuohela
// For Mixxx version 1.11.x
//
// Details for Wiimote HID packet data can be found in:
// http://wiibrew.org/wiki/Wiimote
//

function WiimoteController() {
    this.controller = new HIDController();

    this.registerInputPackets = function() {

        // Valid control packet modes. Strings here are also prefixes 
        // for functions to setup callbacks for this mode.
        this.controlModes = [
            "buttons",
            "coreaccel",
            "coreaccel_ext8", 
            "coreaccel_ir12",
            "corebuttons_ext19",
            "coreaccel_ext16",
            "corebuttons_ir10_ext9",
            "coreaccel_ir10_ext6",
            "ext_21",
            "coreaccel_interleaved"
        ];
        // Select the default control packet to parse from above
        this.controller.defaultPacket = "coreaccel";

        // Core buttons input packet
        packet = new HIDPacket("buttons",[0x30],3);
        packet.addControl("buttons","arrow_left",1,"B",0x1);
        packet.addControl("buttons","arrow_right",1,"B",0x2);
        packet.addControl("buttons","arrow_down",1,"B",0x4);
        packet.addControl("buttons","arrow_up",1,"B",0x8);
        packet.addControl("buttons","button_plus",1,"B",0x10);
        packet.addControl("buttons","button_1",2,"B",0x2);
        packet.addControl("buttons","button_2",2,"B",0x1);
        packet.addControl("buttons","button_bottom",2,"B",0x4);
        packet.addControl("buttons","button_a",2,"B",0x8);
        packet.addControl("buttons","button_minus",2,"B",0x10);
        packet.addControl("buttons","button_home",2,"B",0x80);
        this.controller.registerInputPacket(packet);

        // Core buttons and accelerometer data
        packet = new HIDPacket("coreaccel",[0x31],6);
        packet.addControl("coreaccel","arrow_left",1,"B",0x1);
        packet.addControl("coreaccel","arrow_right",1,"B",0x2);
        packet.addControl("coreaccel","arrow_down",1,"B",0x4);
        packet.addControl("coreaccel","arrow_up",1,"B",0x8);
        packet.addControl("coreaccel","button_plus",1,"B",0x10);
        packet.addControl("coreaccel","button_1",2,"B",0x2);
        packet.addControl("coreaccel","button_2",2,"B",0x1);
        packet.addControl("coreaccel","button_bottom",2,"B",0x4);
        packet.addControl("coreaccel","button_a",2,"B",0x8);
        packet.addControl("coreaccel","button_minus",2,"B",0x10);
        packet.addControl("coreaccel","button_home",2,"B",0x80);
        packet.addControl("coreaccel","accelerometer_x",3,"B");
        packet.addControl("coreaccel","accelerometer_y",4,"B");
        packet.addControl("coreaccel","accelerometer_z",5,"B");
        this.controller.registerInputPacket(packet);

        // Core buttons and accelerometer data with 8 bytes
        // from extension module
        packet = new HIDPacket("coreaccel_ext8",[0x32],14);
        packet.addControl("coreaccel_ext8","arrow_left",1,"B",0x1);
        packet.addControl("coreaccel_ext8","arrow_right",1,"B",0x2);
        packet.addControl("coreaccel_ext8","arrow_down",1,"B",0x4);
        packet.addControl("coreaccel_ext8","arrow_up",1,"B",0x8);
        packet.addControl("coreaccel_ext8","button_plus",1,"B",0x10);
        packet.addControl("coreaccel_ext8","button_1",2,"B",0x2);
        packet.addControl("coreaccel_ext8","button_2",2,"B",0x1);
        packet.addControl("coreaccel_ext8","button_bottom",2,"B",0x4);
        packet.addControl("coreaccel_ext8","button_a",2,"B",0x8);
        packet.addControl("coreaccel_ext8","button_minus",2,"B",0x10);
        packet.addControl("coreaccel_ext8","button_home",2,"B",0x80);
        packet.addControl("coreaccel_ext8","accelerometer_x",3,"B");
        packet.addControl("coreaccel_ext8","accelerometer_y",4,"B");
        packet.addControl("coreaccel_ext8","accelerometer_z",5,"B");
        packet.addControl("coreaccel_ext8","extension_1",6,"B");
        packet.addControl("coreaccel_ext8","extension_2",7,"B");
        packet.addControl("coreaccel_ext8","extension_3",8,"B");
        packet.addControl("coreaccel_ext8","extension_4",9,"B");
        packet.addControl("coreaccel_ext8","extension_5",10,"B");
        packet.addControl("coreaccel_ext8","extension_6",11,"B");
        packet.addControl("coreaccel_ext8","extension_7",12,"B");
        packet.addControl("coreaccel_ext8","extension_8",13,"B");
        this.controller.registerInputPacket(packet);

        // Core buttons and accelerometer data with 12 bytes
        // from IR camera 
        packet = new HIDPacket("coreaccel_ir12",[0x33],18);
        packet.addControl("coreaccel_ir12","arrow_left",1,"B",0x1);
        packet.addControl("coreaccel_ir12","arrow_right",1,"B",0x2);
        packet.addControl("coreaccel_ir12","arrow_down",1,"B",0x4);
        packet.addControl("coreaccel_ir12","arrow_up",1,"B",0x8);
        packet.addControl("coreaccel_ir12","button_plus",1,"B",0x10);
        packet.addControl("coreaccel_ir12","button_1",2,"B",0x2);
        packet.addControl("coreaccel_ir12","button_2",2,"B",0x1);
        packet.addControl("coreaccel_ir12","button_bottom",2,"B",0x4);
        packet.addControl("coreaccel_ir12","button_a",2,"B",0x8);
        packet.addControl("coreaccel_ir12","button_minus",2,"B",0x10);
        packet.addControl("coreaccel_ir12","button_home",2,"B",0x80);
        packet.addControl("coreaccel_ir12","accelerometer_x",3,"B");
        packet.addControl("coreaccel_ir12","accelerometer_y",4,"B");
        packet.addControl("coreaccel_ir12","accelerometer_z",5,"B");
        packet.addControl("coreaccel_ir12","ir_1",6,"B");
        packet.addControl("coreaccel_ir12","ir_2",7,"B");
        packet.addControl("coreaccel_ir12","ir_3",8,"B");
        packet.addControl("coreaccel_ir12","ir_4",9,"B");
        packet.addControl("coreaccel_ir12","ir_5",10,"B");
        packet.addControl("coreaccel_ir12","ir_6",11,"B");
        packet.addControl("coreaccel_ir12","ir_7",12,"B");
        packet.addControl("coreaccel_ir12","ir_8",13,"B");
        packet.addControl("coreaccel_ir12","ir_9",14,"B");
        packet.addControl("coreaccel_ir12","ir_10",15,"B");
        packet.addControl("coreaccel_ir12","ir_11",16,"B");
        packet.addControl("coreaccel_ir12","ir_12",17,"B");
        this.controller.registerInputPacket(packet);

        // Core buttons and 19 bytes from extension module,
        // no accelerometer data
        packet = new HIDPacket("corebuttons_ext19",[0x34],22);
        packet.addControl("corebuttons_ext19","arrow_left",1,"B",0x1);
        packet.addControl("corebuttons_ext19","arrow_right",1,"B",0x2);
        packet.addControl("corebuttons_ext19","arrow_down",1,"B",0x4);
        packet.addControl("corebuttons_ext19","arrow_up",1,"B",0x8);
        packet.addControl("corebuttons_ext19","button_plus",1,"B",0x10);
        packet.addControl("corebuttons_ext19","button_1",2,"B",0x2);
        packet.addControl("corebuttons_ext19","button_2",2,"B",0x1);
        packet.addControl("corebuttons_ext19","button_bottom",2,"B",0x4);
        packet.addControl("corebuttons_ext19","button_a",2,"B",0x8);
        packet.addControl("corebuttons_ext19","button_minus",2,"B",0x10);
        packet.addControl("corebuttons_ext19","button_home",2,"B",0x80);
        packet.addControl("corebuttons_ext19","extension_1",3,"B");
        packet.addControl("corebuttons_ext19","extension_2",4,"B");
        packet.addControl("corebuttons_ext19","extension_3",5,"B");
        packet.addControl("corebuttons_ext19","extension_4",6,"B");
        packet.addControl("corebuttons_ext19","extension_5",7,"B");
        packet.addControl("corebuttons_ext19","extension_6",8,"B");
        packet.addControl("corebuttons_ext19","extension_7",9,"B");
        packet.addControl("corebuttons_ext19","extension_8",10,"B");
        packet.addControl("corebuttons_ext19","extension_9",11,"B");
        packet.addControl("corebuttons_ext19","extension_10",12,"B");
        packet.addControl("corebuttons_ext19","extension_11",13,"B");
        packet.addControl("corebuttons_ext19","extension_12",14,"B");
        packet.addControl("corebuttons_ext19","extension_13",15,"B");
        packet.addControl("corebuttons_ext19","extension_14",16,"B");
        packet.addControl("corebuttons_ext19","extension_15",17,"B");
        packet.addControl("corebuttons_ext19","extension_16",18,"B");
        packet.addControl("corebuttons_ext19","extension_17",19,"B");
        packet.addControl("corebuttons_ext19","extension_18",20,"B");
        packet.addControl("corebuttons_ext19","extension_19",21,"B");
        this.controller.registerInputPacket(packet);

        // Core buttons, accelerometer and 16 bytes from 
        // extension module
        packet = new HIDPacket("coreaccel_ext16",[0x35],22);
        packet.addControl("coreaccel_ext16","arrow_left",1,"B",0x1);
        packet.addControl("coreaccel_ext16","arrow_right",1,"B",0x2);
        packet.addControl("coreaccel_ext16","arrow_down",1,"B",0x4);
        packet.addControl("coreaccel_ext16","arrow_up",1,"B",0x8);
        packet.addControl("coreaccel_ext16","button_plus",1,"B",0x10);
        packet.addControl("coreaccel_ext16","button_1",2,"B",0x2);
        packet.addControl("coreaccel_ext16","button_2",2,"B",0x1);
        packet.addControl("coreaccel_ext16","button_bottom",2,"B",0x4);
        packet.addControl("coreaccel_ext16","button_a",2,"B",0x8);
        packet.addControl("coreaccel_ext16","button_minus",2,"B",0x10);
        packet.addControl("coreaccel_ext16","button_home",2,"B",0x80);
        packet.addControl("coreaccel_ext16","accelerometer_x",3,"B");
        packet.addControl("coreaccel_ext16","accelerometer_y",4,"B");
        packet.addControl("coreaccel_ext16","accelerometer_z",5,"B");
        packet.addControl("coreaccel_ext16","extension_1",6,"B");
        packet.addControl("coreaccel_ext16","extension_2",7,"B");
        packet.addControl("coreaccel_ext16","extension_3",8,"B");
        packet.addControl("coreaccel_ext16","extension_4",9,"B");
        packet.addControl("coreaccel_ext16","extension_5",10,"B");
        packet.addControl("coreaccel_ext16","extension_6",11,"B");
        packet.addControl("coreaccel_ext16","extension_7",12,"B");
        packet.addControl("coreaccel_ext16","extension_8",13,"B");
        packet.addControl("coreaccel_ext16","extension_9",14,"B");
        packet.addControl("coreaccel_ext16","extension_10",15,"B");
        packet.addControl("coreaccel_ext16","extension_11",16,"B");
        packet.addControl("coreaccel_ext16","extension_12",17,"B");
        packet.addControl("coreaccel_ext16","extension_13",18,"B");
        packet.addControl("coreaccel_ext16","extension_14",19,"B");
        packet.addControl("coreaccel_ext16","extension_15",20,"B");
        packet.addControl("coreaccel_ext16","extension_16",21,"B");
        this.controller.registerInputPacket(packet);

        // Core buttons, no accelerometer and 10 IR bytes and
        // 9 bytes from extension module
        packet = new HIDPacket("corebuttons_ir10_ext9",[0x36],22);
        packet.addControl("corebuttons_ir10_ext9","arrow_left",1,"B",0x1);
        packet.addControl("corebuttons_ir10_ext9","arrow_right",1,"B",0x2);
        packet.addControl("corebuttons_ir10_ext9","arrow_down",1,"B",0x4);
        packet.addControl("corebuttons_ir10_ext9","arrow_up",1,"B",0x8);
        packet.addControl("corebuttons_ir10_ext9","button_plus",1,"B",0x10);
        packet.addControl("corebuttons_ir10_ext9","button_1",2,"B",0x2);
        packet.addControl("corebuttons_ir10_ext9","button_2",2,"B",0x1);
        packet.addControl("corebuttons_ir10_ext9","button_bottom",2,"B",0x4);
        packet.addControl("corebuttons_ir10_ext9","button_a",2,"B",0x8);
        packet.addControl("corebuttons_ir10_ext9","button_minus",2,"B",0x10);
        packet.addControl("corebuttons_ir10_ext9","button_home",2,"B",0x80);
        packet.addControl("corebuttons_ir10_ext9","ir_1",3,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_2",4,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_3",5,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_4",6,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_5",7,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_6",8,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_7",9,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_8",10,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_9",11,"B");
        packet.addControl("corebuttons_ir10_ext9","ir_10",12,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_1",13,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_2",14,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_3",15,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_4",16,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_5",17,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_6",18,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_7",19,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_8",20,"B");
        packet.addControl("corebuttons_ir10_ext9","extension_9",21,"B");
        this.controller.registerInputPacket(packet);

        // Core buttons, accelerometer and 10 IR bytes and
        // 6 bytes from extension module
        packet = new HIDPacket("coreaccel_ir10_ext6",[0x37],22);
        packet.addControl("coreaccel_ir10_ext6","arrow_left",1,"B",0x1);
        packet.addControl("coreaccel_ir10_ext6","arrow_right",1,"B",0x2);
        packet.addControl("coreaccel_ir10_ext6","arrow_down",1,"B",0x4);
        packet.addControl("coreaccel_ir10_ext6","arrow_up",1,"B",0x8);
        packet.addControl("coreaccel_ir10_ext6","button_plus",1,"B",0x10);
        packet.addControl("coreaccel_ir10_ext6","button_1",2,"B",0x2);
        packet.addControl("coreaccel_ir10_ext6","button_2",2,"B",0x1);
        packet.addControl("coreaccel_ir10_ext6","button_bottom",2,"B",0x4);
        packet.addControl("coreaccel_ir10_ext6","button_a",2,"B",0x8);
        packet.addControl("coreaccel_ir10_ext6","button_minus",2,"B",0x10);
        packet.addControl("coreaccel_ir10_ext6","button_home",2,"B",0x80);
        packet.addControl("coreaccel_ir10_ext6","accelerometer_x",3,"B");
        packet.addControl("coreaccel_ir10_ext6","accelerometer_y",4,"B");
        packet.addControl("coreaccel_ir10_ext6","accelerometer_z",5,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_1",6,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_2",7,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_3",8,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_4",9,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_5",10,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_6",11,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_7",12,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_8",13,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_9",14,"B");
        packet.addControl("coreaccel_ir10_ext6","ir_10",15,"B");
        packet.addControl("coreaccel_ir10_ext6","extension_1",16,"B");
        packet.addControl("coreaccel_ir10_ext6","extension_2",17,"B");
        packet.addControl("coreaccel_ir10_ext6","extension_3",18,"B");
        packet.addControl("coreaccel_ir10_ext6","extension_4",19,"B");
        packet.addControl("coreaccel_ir10_ext6","extension_5",20,"B");
        packet.addControl("coreaccel_ir10_ext6","extension_6",21,"B");
        this.controller.registerInputPacket(packet);

        // No core buttons, no accelerometer, 21 bytes from
        // extension module
        packet = new HIDPacket("ext_21",[0x3d],22);
        packet.addControl("ext_21","extension_1",1,"B");
        packet.addControl("ext_21","extension_2",2,"B");
        packet.addControl("ext_21","extension_3",3,"B");
        packet.addControl("ext_21","extension_4",4,"B");
        packet.addControl("ext_21","extension_5",5,"B");
        packet.addControl("ext_21","extension_6",6,"B");
        packet.addControl("ext_21","extension_7",7,"B");
        packet.addControl("ext_21","extension_8",8,"B");
        packet.addControl("ext_21","extension_9",9,"B");
        packet.addControl("ext_21","extension_10",10,"B");
        packet.addControl("ext_21","extension_11",11,"B");
        packet.addControl("ext_21","extension_12",12,"B");
        packet.addControl("ext_21","extension_13",13,"B");
        packet.addControl("ext_21","extension_14",14,"B");
        packet.addControl("ext_21","extension_15",15,"B");
        packet.addControl("ext_21","extension_16",16,"B");
        packet.addControl("ext_21","extension_17",17,"B");
        packet.addControl("ext_21","extension_18",18,"B");
        packet.addControl("ext_21","extension_19",19,"B");
        packet.addControl("ext_21","extension_20",20,"B");
        packet.addControl("ext_21","extension_21",21,"B");
        this.controller.registerInputPacket(packet);

        // Interleaved packet 1: core buttons, accelerometer,
        // first 16 bytes from IR camera
        packet = new HIDPacket("coreaccel_interleaved_1",[0x3e],22);
        packet.addControl("coreaccel_interleaved_1","arrow_left",1,"B",0x1);
        packet.addControl("coreaccel_interleaved_1","arrow_right",1,"B",0x2);
        packet.addControl("coreaccel_interleaved_1","arrow_down",1,"B",0x4);
        packet.addControl("coreaccel_interleaved_1","arrow_up",1,"B",0x8);
        packet.addControl("coreaccel_interleaved_1","button_plus",1,"B",0x10);
        packet.addControl("coreaccel_interleaved_1","button_1",2,"B",0x2);
        packet.addControl("coreaccel_interleaved_1","button_2",2,"B",0x1);
        packet.addControl("coreaccel_interleaved_1","button_bottom",2,"B",0x4);
        packet.addControl("coreaccel_interleaved_1","button_a",2,"B",0x8);
        packet.addControl("coreaccel_interleaved_1","button_minus",2,"B",0x10);
        packet.addControl("coreaccel_interleaved_1","button_home",2,"B",0x80);
        packet.addControl("coreaccel_interleaved_1","accelerometer_x",3,"B");
        packet.addControl("coreaccel_interleaved_1","accelerometer_y",4,"B");
        packet.addControl("coreaccel_interleaved_1","accelerometer_z",5,"B");
        packet.addControl("coreaccel_interleaved_1","ir_1",6,"B");
        packet.addControl("coreaccel_interleaved_1","ir_2",7,"B");
        packet.addControl("coreaccel_interleaved_1","ir_3",8,"B");
        packet.addControl("coreaccel_interleaved_1","ir_4",9,"B");
        packet.addControl("coreaccel_interleaved_1","ir_5",10,"B");
        packet.addControl("coreaccel_interleaved_1","ir_6",11,"B");
        packet.addControl("coreaccel_interleaved_1","ir_7",12,"B");
        packet.addControl("coreaccel_interleaved_1","ir_8",13,"B");
        packet.addControl("coreaccel_interleaved_1","ir_9",14,"B");
        packet.addControl("coreaccel_interleaved_1","ir_10",15,"B");
        packet.addControl("coreaccel_interleaved_1","ir_11",16,"B");
        packet.addControl("coreaccel_interleaved_1","ir_12",17,"B");
        packet.addControl("coreaccel_interleaved_1","ir_13",18,"B");
        packet.addControl("coreaccel_interleaved_1","ir_14",19,"B");
        packet.addControl("coreaccel_interleaved_1","ir_15",20,"B");
        packet.addControl("coreaccel_interleaved_1","ir_16",21,"B");
        this.controller.registerInputPacket(packet);

        // Interleaved packet 2: core buttons, accelerometer,
        // last 16 bytes from IR camera
        packet = new HIDPacket("coreaccel_interleaved_2",[0x3f],22);
        packet.addControl("coreaccel_interleaved_2","arrow_left",1,"B",0x1);
        packet.addControl("coreaccel_interleaved_2","arrow_right",1,"B",0x2);
        packet.addControl("coreaccel_interleaved_2","arrow_down",1,"B",0x4);
        packet.addControl("coreaccel_interleaved_2","arrow_up",1,"B",0x8);
        packet.addControl("coreaccel_interleaved_2","button_plus",1,"B",0x10);
        packet.addControl("coreaccel_interleaved_2","button_1",2,"B",0x2);
        packet.addControl("coreaccel_interleaved_2","button_2",2,"B",0x1);
        packet.addControl("coreaccel_interleaved_2","button_bottom",2,"B",0x4);
        packet.addControl("coreaccel_interleaved_2","button_a",2,"B",0x8);
        packet.addControl("coreaccel_interleaved_2","button_minus",2,"B",0x10);
        packet.addControl("coreaccel_interleaved_2","button_home",2,"B",0x80);
        packet.addControl("coreaccel_interleaved_2","accelerometer_x",3,"B");
        packet.addControl("coreaccel_interleaved_2","accelerometer_y",4,"B");
        packet.addControl("coreaccel_interleaved_2","accelerometer_z",5,"B");
        packet.addControl("coreaccel_interleaved_2","ir_17",6,"B");
        packet.addControl("coreaccel_interleaved_2","ir_18",7,"B");
        packet.addControl("coreaccel_interleaved_2","ir_19",8,"B");
        packet.addControl("coreaccel_interleaved_2","ir_20",9,"B");
        packet.addControl("coreaccel_interleaved_2","ir_21",10,"B");
        packet.addControl("coreaccel_interleaved_2","ir_22",11,"B");
        packet.addControl("coreaccel_interleaved_2","ir_23",12,"B");
        packet.addControl("coreaccel_interleaved_2","ir_24",13,"B");
        packet.addControl("coreaccel_interleaved_2","ir_25",14,"B");
        packet.addControl("coreaccel_interleaved_2","ir_26",15,"B");
        packet.addControl("coreaccel_interleaved_2","ir_27",16,"B");
        packet.addControl("coreaccel_interleaved_2","ir_28",17,"B");
        packet.addControl("coreaccel_interleaved_2","ir_29",18,"B");
        packet.addControl("coreaccel_interleaved_2","ir_30",19,"B");
        packet.addControl("coreaccel_interleaved_2","ir_31",20,"B");
        packet.addControl("coreaccel_interleaved_2","ir_32",21,"B");
        this.controller.registerInputPacket(packet);
    }

    this.registerOutputPackets = function() {
        packet = new HIDPacket("feedback",[0x11],2);
        packet.addControl("state","rumble",1,"B",0x1);
        packet.addControl("state","led_1",1,"B",0x10);
        packet.addControl("state","led_2",1,"B",0x20);
        packet.addControl("state","led_3",1,"B",0x40);
        packet.addControl("state","led_4",1,"B",0x80);
        this.controller.registerOutputPacket(packet); 

        packet = new HIDPacket("setreportmode",[0x12],3);
        packet.addControl("reportmode","continuous",1,"B",0x4);
        packet.addControl("reportmode","code",2,"B");
        this.controller.registerOutputPacket(packet); 

        packet = new HIDPacket("ircamera",[0x13],3);
        packet.addControl("ircontrol","enabled",1,"B",0x4);
        this.controller.registerOutputPacket(packet); 

        packet = new HIDPacket("ircamerastate",[0x1a],3);
        packet.addControl("irstate","enabled",1,"B",0x4);
        this.controller.registerOutputPacket(packet); 
    }

    // No default scalers: all controls done with callbacks anyway
    this.registerScalers = function() { }

    // Register your own callbacks in caller by overriding this
    this.registerCallbacks = function() { }

}

// WiiMoteController is defined in Wiimote.hid.js
Wiimote = new WiimoteController();

// Mandatory function for mixxx controllers
Wiimote.init = function(id) {
    Wiimote.id = id;
    Wiimote.continuousReporting = true;

    var controller = Wiimote.controller;
    controller.activeDeck = 1;
    controller.auto_repeat_interval = 50;
   
    Wiimote.registerInputPackets();
    Wiimote.registerOutputPackets();

    controller.startAutoRepeatTimer = function(timer_id,interval) {
        if (controller.timers[timer_id])
            return;
        controller.timers[timer_id] = engine.beginTimer(
            interval,
            "Wiimote.controller.autorepeatTimer()"
        )
    }
    controller.setOutput("state","rumble",0);
    controller.setOutput("state","led_1",0);
    controller.setOutput("state","led_2",0);
    controller.setOutput("state","led_3",0);
    controller.setOutput("state","led_4",0);

    Wiimote.selectInputMode(controller.defaultPacket);
    Wiimote.toggleLED(controller.activeDeck);
    HIDDebug("Wiimote controller initialized: " + Wiimote.id);
}

// Mandatory function for mixxx controllers
Wiimote.shutdown = function() {
    Wiimote.controller.close();
    HIDDebug("Wiimote controller shutdown: " + Wiimote.id);
}

// Mandatory function to receive anything from HID
Wiimote.incomingData = function(data,length) {
    Wiimote.controller.parsePacket(data,length);
}

// Select callback mode based on packet type
Wiimote.selectInputMode = function(mode) {
    var controller = Wiimote.controller;
    if (Wiimote.controlModes.indexOf(mode)==-1) {
        HIDDebug("Unknown input mode name: " + mode);
        return;
    }
    var packet = Wiimote.controller.getInputPacket(mode);

    var callbacks = Wiimote[mode+"_Callbacks"];
    if (callbacks==undefined) {
        HIDDebug("Callbacks for mode not found: " + mode);
        return;
    }
    callbacks(mode);
    controller.defaultPacket = mode;
    Wiimote.updateReportMode(packet.header[0]);
}

Wiimote.updateReportMode = function(code,continuous) {
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getOutputPacket("setreportmode");
    if (packet==undefined) {
        HIDDebug("No output packet "+packetname+" defined");
        return;
    }
    if (code!=undefined)
        Wiimote.reportMode = code;
    if (continuous!=undefined)
        Wiimote.continuosReporting = (continuous) ? true : false;
    controller.setOutput("reportmode","continuous",(Wiimote.continuosReporting)?1:0);
    controller.setOutput("reportmode","code",Wiimote.reportMode);
    packet.send();
}

// Register callbacks for default buttons output report
Wiimote.buttons_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons default mode");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }

    controller.linkModifier("buttons","button_bottom","shift");
    controller.setCallback(packetname,"buttons","button_home",Wiimote.home);
    controller.setCallback(packetname,"buttons","button_a",Wiimote.playcue);

    controller.setCallback(packetname,"buttons","button_minus",Wiimote.jog);
    controller.setCallback(packetname,"buttons","button_plus",Wiimote.jog);

    controller.setCallback(packetname,"buttons","arrow_left",Wiimote.seek_loop);
    controller.setCallback(packetname,"buttons","arrow_right",Wiimote.seek_loop);

    controller.setCallback(packetname,"buttons","arrow_up",Wiimote.select);
    controller.setCallback(packetname,"buttons","arrow_down",Wiimote.select);

    controller.setCallback(packetname,"buttons","button_1",Wiimote.hotcue);
    controller.setCallback(packetname,"buttons","button_2",Wiimote.hotcue);
};

// Register callbacks for buttons + accelerometer output report
Wiimote.coreaccel_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons + accelerometer");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    // Drop accelerometer data which differs by only one
    packet.setMinDelta("coreaccel","accelerometer_x",2);
    packet.setMinDelta("coreaccel","accelerometer_y",2);
    packet.setMinDelta("coreaccel","accelerometer_z",2);

    controller.linkModifier("coreaccel","button_bottom","shift");
    controller.setCallback(packetname,"coreaccel","button_home",Wiimote.home);
    controller.setCallback(packetname,"coreaccel","button_a",Wiimote.playcue);

    controller.setCallback(packetname,"coreaccel","button_minus",Wiimote.jog);
    controller.setCallback(packetname,"coreaccel","button_plus",Wiimote.jog);

    controller.setCallback(packetname,"coreaccel","arrow_left",Wiimote.seek_loop);
    controller.setCallback(packetname,"coreaccel","arrow_right",Wiimote.seek_loop);

    controller.setCallback(packetname,"coreaccel","arrow_up",Wiimote.select);
    controller.setCallback(packetname,"coreaccel","arrow_down",Wiimote.select);

    controller.setCallback(packetname,"coreaccel","button_1",Wiimote.hotcue);
    controller.setCallback(packetname,"coreaccel","button_2",Wiimote.hotcue);

    controller.setCallback(packetname,"coreaccel","accelerometer_x",Wiimote.accel);
    controller.setCallback(packetname,"coreaccel","accelerometer_y",Wiimote.accel);
    controller.setCallback(packetname,"coreaccel","accelerometer_z",Wiimote.accel);
}

// Register callbacks for buttons + accelerometer output report
// with 8 bytes from extension module
Wiimote.coreaccel_ext8_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons + accelerometer 8 bytes extension");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    // TODO - if you wish to use this mode, fill in the controls as above
}

// Register callbacks for buttons + accelerometer output report
// with 8 bytes from extension module
// NOTE - this code does not acually work, it freezes my wiimote
Wiimote.coreaccel_ir12_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons + accelerometer / 12 bytes IR mode");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    packet.setMinDelta("coreaccel_ir12","accelerometer_x",2);
    packet.setMinDelta("coreaccel_ir12","accelerometer_y",2);
    packet.setMinDelta("coreaccel_ir12","accelerometer_z",2);

    controller.linkModifier("coreaccel_ir12","button_bottom","shift");
    controller.setCallback(packetname,"coreaccel_ir12","button_home",Wiimote.home);
    controller.setCallback(packetname,"coreaccel_ir12","button_a",Wiimote.playcue);

    controller.setCallback(packetname,"coreaccel_ir12","button_minus",Wiimote.jog);
    controller.setCallback(packetname,"coreaccel_ir12","button_plus",Wiimote.jog);

    controller.setCallback(packetname,"coreaccel_ir12","arrow_left",Wiimote.seek_loop);
    controller.setCallback(packetname,"coreaccel_ir12","arrow_right",Wiimote.seek_loop);

    controller.setCallback(packetname,"coreaccel_ir12","arrow_up",Wiimote.select);
    controller.setCallback(packetname,"coreaccel_ir12","arrow_down",Wiimote.select);

    controller.setCallback(packetname,"coreaccel_ir12","button_1",Wiimote.hotcue);
    controller.setCallback(packetname,"coreaccel_ir12","button_2",Wiimote.hotcue);

    controller.setCallback(packetname,"coreaccel_ir12","accelerometer_x",Wiimote.accel);
    controller.setCallback(packetname,"coreaccel_ir12","accelerometer_y",Wiimote.accel);
    controller.setCallback(packetname,"coreaccel_ir12","accelerometer_z",Wiimote.accel);

    for (var i=1;i<=12;i++) {
        var control = "ir_"+i;
        controller.setCallback(packetname,"coreaccel_ir12",control,Wiimote.irdata);
    }
    Wiimote.infraredTracking(true);
}

// Register callbacks for buttons + accelerometer output report
// with 8 bytes from extension module
Wiimote.corebuttons_ext19_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons / 19 bytes extension mode");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    // TODO - if you wish to use this mode, fill in the controls as above
}

// Register callbacks for buttons + accelerometer output report
// with 8 bytes from extension module
Wiimote.coreaccel_ext16_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons + accelerometer / 16 bytes extension mode");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    // TODO - if you wish to use this mode, fill in the controls as above
}

// Register callbacks for buttons + accelerometer output report
// with 8 bytes from extension module
Wiimote.corebuttons_ir10_ext9_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons 10 bytes IR data, 9 bytes extension mode");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    // TODO - if you wish to use this mode, fill in the controls as above
}

// Register callbacks for buttons + accelerometer output report
// with 8 bytes from extension module
Wiimote.coreaccel_ir10_ext6_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons + accelerometer / 10 byte IR 6 byte extension mode");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    // TODO - if you wish to use this mode, fill in the controls as above
}

// Register callbacks for buttons + accelerometer output report
// with 8 bytes from extension module
Wiimote.ext_21_Callbacks = function(packetname) {
    HIDDebug("Activating 21 byte extension data mode");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    // TODO - if you wish to use this mode, fill in the controls as above
}

// Register callbacks for buttons + accelerometer output report
// with 8 bytes from extension module
Wiimote.coreaccel_interleaved_Callbacks = function(packetname) {
    HIDDebug("Activating core buttons + accelerometer 16 byte IR interleaved mode");
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket(packetname);
    if (packet==undefined) {
        HIDDebug("No input packet "+packetname+" defined");
        return;
    }
    // TODO - if you wish to use this mode, fill in the controls as above
    // Also you need to process both of the 2 interleaved packets yourself,
    // ask for help if you really think you want it
}

Wiimote.infraredTracking = function(state) {
    var controller = Wiimote.controller;
    var packet;
    state = (state) ? 1 : 0;
    packet = Wiimote.controller.getOutputPacket("ircamera");
    controller.setOutput("ircontrol","enabled",state);
    packet.send();
    packet = Wiimote.controller.getOutputPacket("ircamerastate");
    controller.setOutput("irstate","enabled",state);
    packet.send();
}

// Light the LEDs to indicate which deck we are controlling
Wiimote.toggleLED = function(deck) {
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getOutputPacket("feedback");
    var active_led = "led_" + deck;
    controller.setOutput("state","led_1",0);
    controller.setOutput("state","led_2",0);
    controller.setOutput("state","led_3",0);
    controller.setOutput("state","led_4",0);
    controller.setOutput("state",active_led,1);
    packet.send();
}

Wiimote.rumble = function(rumble,milliseconds) { 
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getOutputPacket("feedback");
    if (rumble!=undefined)
        rumble = (rumble) ? 1 : 0;
    controller.setOutput("state","rumble",rumble);
    packet.send();
    if (milliseconds!=undefined && rumble) {
        controller.timers["rumble"] = engine.beginTimer(
            milliseconds,
            "Wiimote.rumble(false)"
        );
    } else if ("rumble" in controller.timers) {
        engine.stopTimer(controller.timers["rumble"]);
        delete controller.timers["rumble"]
    }
}

Wiimote.seek_loop = function(field) { 
    var controller = Wiimote.controller;
    var value;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (group==undefined)
        return;
    if (field.value==controller.buttonStates.released)
        value = false;
    else
        value = true;
    if (controller.modifiers.get("shift")) {
        if (field.name=="arrow_left") 
            engine.setValue(group,"back",value);
        if (field.name=="arrow_right") 
            engine.setValue(group,"fwd",value);
    } else {
        // Reset back/gwd states (can be left on if you release shift)
        if (engine.getValue(group,"back"))
            engine.setValue(group,"back",false);
        if (engine.getValue(group,"fwd"))
            engine.setValue(group,"fwd",false);
        if (field.name=="arrow_left") 
            engine.setValue(group,"loop_in",value);
        if (field.name=="arrow_right") {
            if (engine.getValue(group,"loop_enabled")) {
                engine.setValue(group,"reloop_exit",value);
                engine.setValue(group,"loop_in",false);
                engine.setValue(group,"loop_out",false);
            }
            else
                engine.setValue(group,"loop_out",value);
        }
    }
};

Wiimote.select = function(field) { 
    var controller = Wiimote.controller;
    if (!controller.modifiers.get("shift"))  {
        if (field.value==controller.buttonStates.released)
            return;
        if (field.name=="arrow_up") {
            callback = function() {
                engine.setValue("[Playlist]","SelectPrevTrack",true);
            }
        }
        if (field.name=="arrow_down") {
            callback = function() {
                engine.setValue("[Playlist]","SelectNextTrack",true);
            }
        }
        controller.setAutoRepeat(controller.defaultPacket,field.name,callback,100);
    } else {
        var value;
        var change;
        var callback = undefined;
        var group = controller.resolveDeckGroup(controller.activeDeck);
        if (group==undefined)
            return;
        if (field.name=="arrow_up") {
            callback = function(field) {
                var group = Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck)
                engine.setValue(group,"rate",engine.getValue(group,"rate")+0.02);
            }
        }
        if (field.name=="arrow_down") {
            callback = function(field) {
                var group = Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck)
                engine.setValue(group,"rate",engine.getValue(group,"rate")-0.02);
            }
        }
        controller.setAutoRepeat(controller.defaultPacket,field.name,callback,100);
    }
}

// Call cue_default, or select deck if shift (bottom button) is pressed
Wiimote.home = function(field) { 
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (!controller.modifiers.get("shift"))  {
        if (engine.getValue(group,"play")) {
            Wiimote.rumble(true,200);
        } else {
            engine.setValue(group,"LoadSelectedTrack",true);
        }
    } else {
        if (field.value==controller.buttonStates.released)
            return;
        controller.switchDeck();
        Wiimote.toggleLED(controller.activeDeck);
    }
}

Wiimote.playcue = function(field) { 
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (group==undefined)
        return;
    if (!controller.modifiers.get("shift"))  {
        if (field.value==controller.buttonStates.released)
            return;
        controller.togglePlay(group,field);
    } else {
        var group = controller.resolveDeckGroup(controller.activeDeck);
        if (field.value==controller.buttonStates.released) {
            engine.setValue(group,"cue_default",false);
        } else {
            engine.setValue(group,"cue_default",true);
        }
    }
}

Wiimote.jog = function(field) {
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (group==undefined)
        return;
    if (field.value==controller.buttonStates.released)
        return;
    if (field.name=="button_minus") {
        if (!controller.modifiers.get("shift")) {
            engine.setValue(group,"jog",-2);
            callback = function(field) { 
                engine.setValue(
                  Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck),
                  "jog",-2
                );
            };
        } else {
            engine.setValue(group,"jog",-6);
            callback = function(field) { 
                engine.setValue(
                  Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck),
                  "jog",-6
                );
            };
        }
        controller.setAutoRepeat(controller.defaultPacket,"button_minus",callback,20);
    }
    if (field.name=="button_plus") {
        if (!controller.modifiers.get("shift")) {
            engine.setValue(group,"jog",2);
            callback = function(field) { 
                engine.setValue(
                  Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck),
                  "jog",2
                );
            };
        } else {
            engine.setValue(group,"jog",6);
            callback = function(field) { 
                engine.setValue(
                  Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck),
                  "jog",6
                );
            };
        }
        controller.setAutoRepeat(controller.defaultPacket,"button_plus",callback,20);
    }
}

Wiimote.hotcue = function(field) { 
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (group==undefined)
        return;
    if (controller.modifiers.get("shift")) {
        if (field.value==controller.buttonStates.released)
            return;
        if (field.name=="button_1") 
            engine.setValue(group,"hotcue_1_clear",true);
        if (field.name=="button_2") 
            engine.setValue(group,"hotcue_2_clear",true);
    } else {
        if (field.value==controller.buttonStates.released)
            return;
        if (field.name=="button_1") 
            engine.setValue(group,"hotcue_1_activate",true);
        if (field.name=="button_2") 
            engine.setValue(group,"hotcue_2_activate",true);
    }
}

Wiimote.irdata = function(field) { 
    HIDDebug(field.name + " DELTA " + field.delta);
}

Wiimote.accel = function(field) { 
    // Disabled until we do something useful this data
    return;
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (group==undefined)
        return;
    if (!controller.modifiers.get("shift")) {
        return;
    }
    if (field.name == 'accelerometer_x') {
        HIDDebug(field.name + " DELTA " + field.delta);
    } else if (field.name == 'accelerometer_y') {
        HIDDebug(field.name + " DELTA " + field.delta);
    } else if (field.name == 'accelerometer_z') {
        HIDDebug(field.name + " DELTA " + field.delta);
    }
}


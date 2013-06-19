//
// Sony SixxAxis PS3 Game Controller HID mapping
// Copyright (C) 2012, Ilkka Tuohela
// For Mixxx version 1.11.x
//

function SonySixxAxisController() {
    this.controller = new HIDController();

    this.controller.activeDeck = 1;

    this.registerInputPackets = function() {
        packet = new HIDPacket("control",[],49);

        // Toggle buttons
        packet.addControl("hid","select",2,"B",0x1);
        packet.addControl("hid","jog_left_button",2,"B",0x2);
        packet.addControl("hid","jog_right_button",2,"B",0x4);
        packet.addControl("hid","start",2,"B",0x8);
        packet.addControl("hid","arrow_button_up",2,"B",0x10);
        packet.addControl("hid","arrow_button_right",2,"B",0x20);
        packet.addControl("hid","arrow_button_down",2,"B",0x40);
        packet.addControl("hid","arrow_button_left",2,"B",0x80);
        packet.addControl("hid","button_bottom_left",3,"B",0x1);
        packet.addControl("hid","button_bottom_right",3,"B",0x2);
        packet.addControl("hid","button_top_left",3,"B",0x4);
        packet.addControl("hid","button_top_right",3,"B",0x8);
        packet.addControl("hid","button_triangle",3,"B",0x10);
        packet.addControl("hid","button_circle",3,"B",0x20);
        packet.addControl("hid","button_cross",3,"B",0x40);
        packet.addControl("hid","button_square",3,"B",0x80);

        // 0x0 left, 0xff right
        packet.addControl("hid","jog_left_x",0x6,"B");
        packet.setMinDelta("hid","jog_left_x",2);
        packet.addControl("hid","jog_right_x",0x8,"B");
        packet.setMinDelta("hid","jog_right_x",2);

        // 0x0 top, 0xff bottom
        packet.addControl("hid","jog_left_y",0x7,"B");
        packet.setMinDelta("hid","jog_left_y",2);
        packet.addControl("hid","jog_right_y",0x9,"B");
        packet.setMinDelta("hid","jog_right_y",2);

        // Toggle button pressure sensitivity 0x00-0xff
        //packet.addControl("hid","arrow_pressure_up",0x0e,"B");
        //packet.setMinDelta("hid","arrow_pressure_up",1);
        //packet.addControl("hid","arrow_pressure_right",0x0f,"B");
        //packet.setMinDelta("hid","arrow_pressure_right",1);
        //packet.addControl("hid","arrow_pressure_down",0x10,"B");
        //packet.setMinDelta("hid","arrow_pressure_down",1);
        //packet.addControl("hid","arrow_pressure_left",0x11,"B");
        //packet.setMinDelta("hid","arrow_pressure_left",1);
        packet.addControl("hid","pressure_bottom_left",0x12,"B");
        packet.setMinDelta("hid","pressure_bottom_left",4);
        packet.addControl("hid","pressure_bottom_right",0x13,"B");
        packet.setMinDelta("hid","pressure_bottom_right",4);
        packet.addControl("hid","pressure_top_left",0x14,"B");
        packet.setMinDelta("hid","pressure_top_left",4);
        packet.addControl("hid","pressure_top_right",0x15,"B");
        packet.setMinDelta("hid","pressure_top_right",4);
        //packet.addControl("hid","pressure_triangle",0x16,"B");
        //packet.setMinDelta("hid","pressure_triangle",1);
        //packet.addControl("hid","pressure_circle",0x17,"B");
        //packet.setMinDelta("hid","pressure_circle",1);
        //packet.addControl("hid","pressure_cross",0x18,"B");
        //packet.setMinDelta("hid","pressure_cross",1);
        //packet.addControl("hid","pressure_square",0x19,"B");
        //packet.setMinDelta("hid","pressure_square",1);

        // Gyro pitch (sideways movement)
        packet.addControl("hid","gyro_pitch",0x2a,"B");
        packet.setCallback("hid","gyro_pitch",this.gyro);
        packet.setMinDelta("hid","gyro_pitch",4);
        // Gyro roll (forward movement)
        packet.addControl("hid","gyro_roll",0x2c,"B");
        packet.setMinDelta("hid","gyro_roll",4);
        packet.setCallback("hid","gyro_roll",this.gyro);
        // Gyro gravity position
        packet.addControl("hid","gyro_gravity",0x2e,"B");
        packet.setMinDelta("hid","gyro_gravity",4);
        packet.setCallback("hid","gyro_gravity",this.gyro);

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
}

// SonySixxAxisController is defined in SonySixxAxis.hid.js
SonySixxAxis = new SonySixxAxisController();

// Mandatory function for mixxx controllers
SonySixxAxis.init = function(id) {
    SonySixxAxis.id = id;
    var controller = SonySixxAxis.controller;
    SonySixxAxis.registerInputPackets();
    SonySixxAxis.registerCallbacks();

    controller.startAutoRepeatTimer = function(timer_id,interval) {
        if (controller.timers[timer_id])
            return;
        controller.timers[timer_id] = engine.beginTimer(
            interval,
            "SonySixxAxis.controller.autorepeatTimer()"
        )
    }
    HIDDebug("Sony SixxAxis controller initialized: " + SonySixxAxis.id);
}

// Mandatory function for mixxx controllers
SonySixxAxis.shutdown = function() {
    SonySixxAxis.controller.close();
    HIDDebug("Sony SixxAxis controller shutdown: " + SonySixxAxis.id);
}

// Mandatory function to receive anything from HID
SonySixxAxis.incomingData = function(data,length) {
    SonySixxAxis.controller.parsePacket(data,length);
}

// Register callbacks for "hid" controls defined in SonySixxAxisController
SonySixxAxis.registerCallbacks = function(id) {
    var controller = SonySixxAxis.controller;
    var packet = SonySixxAxis.controller.getInputPacket("control");
    if (packet==undefined) {
        HIDDebug("No input packet " +control+ " defined");
        return;
    }
    if (controller==undefined) {
        HIDDebug("Error registrering callbacks: controller is undefined");
        return;
    }

    // Change field types to modifier and register the modifiers
    controller.linkModifier("hid","arrow_button_down","shift_left");
    controller.linkModifier("hid","button_cross","shift_right");

    // Link HID buttons to deck1 and deck2 play buttons
    controller.linkControl("hid","arrow_button_up","deck1","LoadSelectedTrack");
    controller.linkControl("hid","arrow_button_left","deck1","cue_default");
    controller.linkControl("hid","arrow_button_right","deck1","play");
    controller.linkControl("hid","button_triangle","deck2","LoadSelectedTrack");
    controller.linkControl("hid","button_square","deck2","cue_default");
    controller.linkControl("hid","button_circle","deck2","play");
    controller.linkControl("hid","jog_left_button","deck1","beatsync");
    controller.linkControl("hid","jog_right_button","deck2","beatsync");

    // Callbacks for toggle buttons front of controller
    controller.setCallback("control","hid","button_bottom_left",SonySixxAxis.front_left);
    controller.setCallback("control","hid","button_top_left",SonySixxAxis.front_left);
    controller.setCallback("control","hid","button_bottom_right",SonySixxAxis.front_right);
    controller.setCallback("control","hid","button_top_right",SonySixxAxis.front_right);

    // Callbacks for pressure sensitive buttons front of controller
    controller.setCallback("control","hid","pressure_bottom_left",SonySixxAxis.left_bend);
    controller.setCallback("control","hid","pressure_top_left",SonySixxAxis.left_bend);
    controller.setCallback("control","hid","pressure_bottom_right",SonySixxAxis.right_bend);
    controller.setCallback("control","hid","pressure_top_right",SonySixxAxis.right_bend);

    // Callbacks for jog controls
    controller.setCallback("control","hid","jog_left_y",SonySixxAxis.left_jog);
    controller.setCallback("control","hid","jog_right_y",SonySixxAxis.right_jog);

    controller.setCallback("control","hid","jog_left_x",SonySixxAxis.left_jog);
    controller.setCallback("control","hid","jog_right_x",SonySixxAxis.right_jog);

};

SonySixxAxis.front_left = function(field) {
    var controller = SonySixxAxis.controller;
    if (field.value==controller.buttonStates.released)
        return;
    if (!controller.modifiers.get("shift_left"))
        return;
    if (field.name=="button_top_left") 
        engine.setValue("[Playlist]","SelectPrevPlaylist",true);
    if (field.name=="button_bottom_left") 
        engine.setValue("[Playlist]","SelectNextPlaylist",true);
}

SonySixxAxis.front_right = function(field) {
    var controller = SonySixxAxis.controller;
    if (field.value==controller.buttonStates.released)
        return;
    if (!controller.modifiers.get("shift_right")) 
        return;
    if (field.name=="button_top_right") {
        engine.setValue("[Playlist]","SelectPrevTrack",true);
        var callback = function(field) {
            engine.setValue("[Playlist]","SelectPrevTrack",true);
        }
        controller.setAutoRepeat("hid","button_top_right",callback,150);
    }
    if (field.name=="button_bottom_right") {
        engine.setValue("[Playlist]","SelectNextTrack",true);
        var callback = function(field) {
            engine.setValue("[Playlist]","SelectNextTrack",true);
        }
        controller.setAutoRepeat("hid","button_bottom_right",callback,150);
    }
}

SonySixxAxis.left_jog = function(field) {
    var controller = SonySixxAxis.controller;
    var group = controller.resolveGroup("deck1");
    if (group==undefined)
        return;
    if (field.name=="jog_left_y") {
        if (field.delta<=0) 
            return;
        old_value = engine.getValue(group,"rate");
        if (field.value<115)
            engine.setValue(group,"rate",old_value+0.005);
        else if (field.value>135)
            engine.setValue(group,"rate",old_value-0.005);
    }
    if (field.name=="jog_left_x") {

    }
}

SonySixxAxis.right_jog = function(field) {
    var controller = SonySixxAxis.controller;
    var group = controller.resolveGroup("deck2");
    var old_value = undefined;
    if (group==undefined)
        return;
    if (field.name=="jog_right_y") {
        if (field.delta<=0) 
            return;
        old_value = engine.getValue(group,"rate");
        if (field.value<115)
            engine.setValue(group,"rate",old_value+0.005);
        else if (field.value>135)
            engine.setValue(group,"rate",old_value-0.005);
    }
    if (field.name=="jog_right_x") {

    }
}

SonySixxAxis.left_bend = function(field) {
    var controller = SonySixxAxis.controller;
    if (controller.modifiers.get("shift_left")) 
        return;
    if (field.name=="pressure_top_left") {
        SonySixxAxis.jog_bend("deck1","down",field.value);
        var callback = function(field) {
            SonySixxAxis.jog_bend("deck1","down",64);
        }
        controller.setAutoRepeat("hid","pressure_top_left",callback,50);
    }
    if (field.name=="pressure_bottom_left") {
        SonySixxAxis.jog_bend("deck1","up",field.value);
        var callback = function() {
            SonySixxAxis.jog_bend("deck1","up",64);
        }
        controller.setAutoRepeat("hid","pressure_bottom_left",callback,50);
    }
}

SonySixxAxis.right_bend = function(field) {
    var controller = SonySixxAxis.controller;
    if (controller.modifiers.get("shift_right")) 
        return;
    if (field.name=="pressure_top_right") {
        SonySixxAxis.jog_bend("deck2","down",field);
        var callback = function() {
            SonySixxAxis.jog_bend("deck2","down",64);
        }
        controller.setAutoRepeat("hid","pressure_top_right",callback,50);
    }
    if (field.name=="pressure_bottom_right") {
        SonySixxAxis.jog_bend("deck2","up",field);
        var callback = function() {
            SonySixxAxis.jog_bend("deck2","up",64);
        }
        controller.setAutoRepeat("hid","pressure_bottom_right",callback,50);
    }
}

SonySixxAxis.jog_bend = function(group,direction,value) {
    var controller = SonySixxAxis.controller;
    var jog_value = value/32;
    group = controller.resolveGroup(group);
    if (group==undefined)
        return;
    if (direction=="up")
        engine.setValue(group,"jog",6);
    if (direction=="down")
        engine.setValue(group,"jog",-6);
}



//
// Sony SixxAxis PS3 Game Controller HID mapping
// Copyright (C) 2012, Ilkka Tuohela
// For Mixxx version 1.11.x
//

// SonySixxAxisController is defined in SonySixxAxis.hid.js
SonySixxAxis = new SonySixxAxisController();

SonySixxAxis.activeDeck = 1;

// Mandatory function for mixxx controllers
SonySixxAxis.init = function(id) {
    SonySixxAxis.id = id;
    // Required to make virtual deck resolution work
    SonySixxAxis.controller.activeDeck = 1;
    SonySixxAxis.registerInputPackets();
    SonySixxAxis.registerCallbacks();

    var controller = SonySixxAxis.controller;
    controller.registerAutoRepeatTimer = function() {
        controller.auto_repeat_timer = engine.beginTimer(
            controller.auto_repeat_interval,
            "SonySixxAxis.controller.controlAutoRepeat()"
        )
    }

    script.HIDDebug("Sony SixxAxis controller initialized: " + SonySixxAxis.id);
}

// Mandatory function for mixxx controllers
SonySixxAxis.shutdown = function() {
    SonySixxAxis.controller.close();
    script.HIDDebug("Sony SixxAxis controller shutdown: " + SonySixxAxis.id);
}

// Mandatory function to receive anything from HID
SonySixxAxis.incomingData = function(data,length) {
    SonySixxAxis.controller.parsePacket(data,length);
}

// Register callbacks for 'hid' controls defined in SonySixxAxisController
SonySixxAxis.registerCallbacks = function(id) {
    var packet = SonySixxAxis.controller.resolveInputPacket('control');
    if (packet==undefined) {
        script.HIDDebug("No input packet 'control' defined");
        return;
    }
    var controller = SonySixxAxis.controller;
    if (controller==undefined) {
        script.HIDDebug("Error registrering callbacks: controller is undefined");
        return;
    }

    // Change field types to modifier and register the modifiers
    controller.linkModifier('hid','arrow_button_down','shift_left');
    controller.linkModifier('hid','button_cross','shift_right');

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
    controller.registerInputCallback("control","hid","button_bottom_left",SonySixxAxis.front_left);
    controller.registerInputCallback("control","hid","button_top_left",SonySixxAxis.front_left);
    controller.registerInputCallback("control","hid","button_bottom_right",SonySixxAxis.front_right);
    controller.registerInputCallback("control","hid","button_top_right",SonySixxAxis.front_right);
    controller.enableBitAutoRepeat("hid","button_bottom_right");
    controller.enableBitAutoRepeat("hid","button_top_right");

    // Callbacks for pressure sensitive buttons front of controller
    controller.registerInputCallback("control","hid","pressure_bottom_left",SonySixxAxis.left_bend);
    controller.enableBitAutoRepeat("hid","pressure_bottom_left");
    controller.registerInputCallback("control","hid","pressure_top_left",SonySixxAxis.left_bend);
    controller.enableBitAutoRepeat("hid","pressure_top_left");
    controller.registerInputCallback("control","hid","pressure_bottom_right",SonySixxAxis.right_bend);
    controller.enableBitAutoRepeat("hid","pressure_bottom_right");
    controller.registerInputCallback("control","hid","pressure_top_right",SonySixxAxis.right_bend);
    controller.enableBitAutoRepeat("hid","pressure_top_right");

    // Callbacks for jog controls
    controller.registerInputCallback("control","hid","jog_left_y",SonySixxAxis.left_jog);
    controller.enableBitAutoRepeat("hid","jog_left_y");
    controller.registerInputCallback("control","hid","jog_right_y",SonySixxAxis.right_jog);
    controller.enableBitAutoRepeat("hid","jog_right_y");

    controller.registerInputCallback("control","hid","jog_left_x",SonySixxAxis.left_jog);
    controller.registerInputCallback("control","hid","jog_right_x",SonySixxAxis.right_jog);

};

SonySixxAxis.play = function(field) {
    script.HIDDebug("PLAY from field " + field.id);
}

SonySixxAxis.front_left = function(field) {
    var controller = SonySixxAxis.controller;
    if (field.value==controller.buttonStates.released)
        return;
    if (!controller.modifierIsSet('shift_left'))
        return;
    if (field.name=='button_top_left') 
        engine.setValue('[Playlist]','SelectPrevPlaylist',true);
    if (field.name=='button_bottom_left') 
        engine.setValue('[Playlist]','SelectNextPlaylist',true);
}

SonySixxAxis.front_right = function(field) {
    var controller = SonySixxAxis.controller;
    if (field.value==controller.buttonStates.released)
        return;
    if (!controller.modifierIsSet('shift_right')) 
        return;
    if (field.name=='button_top_right') 
        engine.setValue('[Playlist]','SelectPrevTrack',true);
    if (field.name=='button_bottom_right') 
        engine.setValue('[Playlist]','SelectNextTrack',true);
}

SonySixxAxis.left_jog = function(field) {
    var controller = SonySixxAxis.controller;
    var group = controller.resolveGroup('deck1');
    if (group==undefined)
        return;
    if (field.name=='jog_left_y') {
        if (field.delta<=0) 
            return;
        old_value = engine.getValue(group,'rate');
        if (field.value<115)
            engine.setValue(group,'rate',old_value+0.005);
        else if (field.value>135)
            engine.setValue(group,'rate',old_value-0.005);
    }
    if (field.name=='jog_left_x') {

    }
}

SonySixxAxis.right_jog = function(field) {
    var controller = SonySixxAxis.controller;
    var group = controller.resolveGroup('deck2');
    var old_value = undefined;
    if (group==undefined)
        return;
    if (field.name=='jog_right_y') {
        if (field.delta<=0) 
            return;
        old_value = engine.getValue(group,'rate');
        if (field.value<115)
            engine.setValue(group,'rate',old_value+0.005);
        else if (field.value>135)
            engine.setValue(group,'rate',old_value-0.005);
    }
    if (field.name=='jog_right_x') {

    }
}

SonySixxAxis.left_bend = function(field) {
    var controller = SonySixxAxis.controller;
    if (controller.modifierIsSet('shift_left')) 
        return;
    if (field.name=='pressure_top_left') 
        SonySixxAxis.jog_bend('deck1','down',field);
    if (field.name=='pressure_bottom_left') 
        SonySixxAxis.jog_bend('deck1','up',field);
}

SonySixxAxis.right_bend = function(field) {
    var controller = SonySixxAxis.controller;
    if (controller.modifierIsSet('shift_right')) 
        return;
    if (field.name=='pressure_top_right')
        SonySixxAxis.jog_bend('deck2','down',field);
    if (field.name=='pressure_bottom_right')
        SonySixxAxis.jog_bend('deck2','up',field);
}

SonySixxAxis.jog_bend = function(group,direction,field) {
    var controller = SonySixxAxis.controller;
    var jog_value = field.value/32;
    if (field.value==0 || field.delta<=0)
        return;
    group = controller.resolveGroup(group);
    if (group==undefined)
        return;
    if (direction=='up')
        engine.setValue(group,'jog',jog_value);
    if (direction=='down')
        engine.setValue(group,'jog',-jog_value);
}



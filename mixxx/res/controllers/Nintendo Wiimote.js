//
// Nintendo Wii Remote Game Controller HID mapping
// Copyright (C) 2012, Ilkka Tuohela
// For Mixxx version 1.11.x
//

// WiiMoteController is defined in Wiimote.hid.js
Wiimote = new WiimoteController();

Wiimote.activeDeck = 1;

// Mandatory function for mixxx controllers
Wiimote.init = function(id) {
    Wiimote.id = id;
    // Required to make virtual deck resolution work
    Wiimote.controller.activeDeck = 1;
    Wiimote.controller.auto_repeat_interval = 100;
    Wiimote.registerInputPackets();
    Wiimote.registerCallbacks();

    var controller = Wiimote.controller;
    controller.registerAutoRepeatTimer = function() {
        controller.auto_repeat_timer = engine.beginTimer(
            controller.auto_repeat_interval,
            "Wiimote.controller.controlAutoRepeat()"
        )
    }

    script.HIDDebug("Wiimote controller initialized: " + Wiimote.id);
}

// Mandatory function for mixxx controllers
Wiimote.shutdown = function() {
    Wiimote.controller.close();
    script.HIDDebug("Wiimote controller shutdown: " + Wiimote.id);
}

// Mandatory function to receive anything from HID
Wiimote.incomingData = function(data,length) {
    Wiimote.controller.parsePacket(data,length);
}

// Register callbacks for 'hid' controls defined in WiimoteController
Wiimote.registerCallbacks = function(id) {
    var packet = Wiimote.controller.resolveInputPacket('control');
    if (packet==undefined) {
        script.HIDDebug("No input packet 'control' defined");
        return;
    }
    var controller = Wiimote.controller;
    if (controller==undefined) {
        script.HIDDebug("Error registrering callbacks: controller is undefined");
        return;
    }

    // Link HID buttons to deck1 and deck2 play buttons
    controller.linkModifier("hid","button_bottom","shift");
    controller.registerInputCallback("control","hid","button_home",Wiimote.deck_cue);
    controller.registerInputCallback("control","hid","button_a",Wiimote.play_load);

    controller.registerInputCallback("control","hid","button_minus",Wiimote.jog);
    controller.registerInputCallback("control","hid","button_plus",Wiimote.jog);
    controller.enableBitAutoRepeat("hid","button_minus");
    controller.enableBitAutoRepeat("hid","button_plus");

    controller.registerInputCallback("control","hid","arrow_left",Wiimote.seek_loop);
    controller.registerInputCallback("control","hid","arrow_right",Wiimote.seek_loop);

    controller.registerInputCallback("control","hid","arrow_up",Wiimote.select);
    controller.registerInputCallback("control","hid","arrow_down",Wiimote.select);
    controller.enableBitAutoRepeat("hid","arrow_up");
    controller.enableBitAutoRepeat("hid","arrow_down");

    controller.registerInputCallback("control","hid","button_1",Wiimote.hotcue);
    controller.registerInputCallback("control","hid","button_2",Wiimote.hotcue);


};

Wiimote.seek_loop = function(field) { 
    var controller = Wiimote.controller;
    var value;
    var group = controller.resolveDeckGroup(Wiimote.activeDeck);
    if (group==undefined)
        return;
    if (field.value==controller.buttonStates.released)
        value = false;
    else
        value = true;
    if (controller.modifierIsSet('shift')) {
        if (field.name=="arrow_left") 
            engine.setValue(group,'back',value);
        if (field.name=="arrow_right") 
            engine.setValue(group,'fwd',value);
    } else {
        // Reset back/gwd states (can be left on if you release shift)
        if (engine.getValue(group,'back'))
            engine.setValue(group,'back',false);
        if (engine.getValue(group,'fwd'))
            engine.setValue(group,'fwd',false);
        if (field.value==controller.buttonStates.released)
            return;
        if (field.name=="arrow_left") 
            engine.setValue(group,'loop_in',value);
        if (field.name=="arrow_right") 
            if (engine.getValue(group,'loop_enabled'))
                engine.setValue(group,'reloop_exit',true);
            else
                engine.setValue(group,'loop_out',true);
    }
};

Wiimote.select = function(field) { 
    var controller = Wiimote.controller;
    if (!controller.modifierIsSet('shift'))  {
        if (field.value==controller.buttonStates.released)
            return;
        if (field.name=="arrow_up") 
            engine.setValue("[Playlist]","SelectPrevTrack",true);
        if (field.name=="arrow_down") 
            engine.setValue("[Playlist]","SelectNextTrack",true);
    } else {
        var group = controller.resolveDeckGroup(Wiimote.activeDeck);
        var value;
        if (group==undefined)
            return;
        if (field.name=="arrow_up") {
            value = engine.getValue(group,'rate')+0.02;
            engine.setValue(group,"rate",value);
        }
        if (field.name=="arrow_down") {
            value = engine.getValue(group,'rate')-0.02;
            engine.setValue(group,"rate",value);
        }
    }
}

// Call cue_default, or select deck if shift (bottom button) is pressed
Wiimote.deck_cue = function(field) { 
    var controller = Wiimote.controller;
    if (!controller.modifierIsSet('shift'))  {
        var group = controller.resolveDeckGroup(Wiimote.activeDeck);
        if (field.value==controller.buttonStates.released)
            engine.setValue(group,'cue_default',false);
        else
            engine.setValue(group,'cue_default',true);
    } else {
        if (field.value==controller.buttonStates.released)
            return;
        switch (Wiimote.activeDeck) {
            case 1: Wiimote.activeDeck = 2; break;
            case 2: Wiimote.activeDeck = 1; break;
            default: Wiimote.activeDeck = 1; break;
        } 
        script.HIDDebug("Active deck now " + Wiimote.activeDeck);
    }
}

Wiimote.play_load = function(field) { 
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(Wiimote.activeDeck);
    if (group==undefined)
        return;
    if (field.value==controller.buttonStates.released)
        return;
    if (!controller.modifierIsSet('shift'))  {
        controller.togglePlay(group,field);
    } else {
        engine.setValue(group,"LoadSelectedTrack",true);
    }
}

Wiimote.jog = function(field) {
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(Wiimote.activeDeck);
    if (group==undefined)
        return;
    if (field.name=='button_minus') 
        if (field.value==controller.buttonStates.released)
            engine.setValue(group,'jog',0);
        else
            engine.setValue(group,'jog',-4);
    if (field.name=='button_plus') 
        if (field.value==controller.buttonStates.released)
            engine.setValue(group,'jog',0);
        else
            engine.setValue(group,'jog',4);
}

Wiimote.hotcue = function(field) { 
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(Wiimote.activeDeck);
    if (group==undefined)
        return;
    if (controller.modifierIsSet('shift'))  {
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


//
// Nintendo Wii Remote Game Controller HID mapping
// Copyright (C) 2012, Ilkka Tuohela
// For Mixxx version 1.11.x
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
        //packet = new HIDPacket("control",[],3,this.dump);

        // Basic button toggle input packet: packet is longer when
        // we figure out how to enable other data than buttons
        packet = new HIDPacket("control",[],3);

        packet.addControl("deck","arrow_left",1,"B",0x1);
        packet.addControl("deck","arrow_right",1,"B",0x2);
        packet.addControl("deck","arrow_down",1,"B",0x4);
        packet.addControl("deck","arrow_up",1,"B",0x8);
        packet.addControl("deck","button_plus",1,"B",0x10);
        packet.addControl("deck","button_1",2,"B",0x2);
        packet.addControl("deck","button_2",2,"B",0x1);
        packet.addControl("deck","button_bottom",2,"B",0x4);
        packet.addControl("deck","button_a",2,"B",0x8);
        packet.addControl("deck","button_minus",2,"B",0x10);
        packet.addControl("deck","button_home",2,"B",0x80);

        this.controller.registerInputPacket(packet);
    }

    // Wiimote has output controls, but I could not figure out how
    // to send the packets, I"m just getting a general error response.
    this.registerOutputPackets = function() { }

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
    var controller = Wiimote.controller;
    controller.activeDeck = 1;
    controller.auto_repeat_interval = 50;
    Wiimote.registerInputPackets();
    Wiimote.registerCallbacks();

    controller.startAutoRepeatTimer = function(timer_id,interval) {
        if (controller.timers[timer_id])
            return;
        controller.timers[timer_id] = engine.beginTimer(
            interval,
            "Wiimote.controller.autorepeatTimer()"
        )
    }
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

// Register callbacks for deck controls defined in WiimoteController
Wiimote.registerCallbacks = function(id) {
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getInputPacket("control");
    if (packet==undefined) {
        HIDDebug("No input packet "+control+" defined");
        return;
    }

    // Link HID buttons to deck1 and deck2 play buttons
    controller.linkModifier("deck","button_bottom","shift");
    controller.setCallback("control","deck","button_home",Wiimote.deck_cue);
    controller.setCallback("control","deck","button_a",Wiimote.play_load);

    controller.setCallback("control","deck","button_minus",Wiimote.jog);
    controller.setCallback("control","deck","button_plus",Wiimote.jog);

    controller.setCallback("control","deck","arrow_left",Wiimote.seek_loop);
    controller.setCallback("control","deck","arrow_right",Wiimote.seek_loop);

    controller.setCallback("control","deck","arrow_up",Wiimote.select);
    controller.setCallback("control","deck","arrow_down",Wiimote.select);

    controller.setCallback("control","deck","button_1",Wiimote.hotcue);
    controller.setCallback("control","deck","button_2",Wiimote.hotcue);
};

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
        if (field.name=="arrow_up") 
            callback = function() {
                engine.setValue("[Playlist]","SelectPrevTrack",true);
            }
        if (field.name=="arrow_down") 
            callback = function() {
                engine.setValue("[Playlist]","SelectNextTrack",true);
            }
        controller.setAutoRepeat("deck",field.name,callback,50);
    } else {
        var group = controller.resolveDeckGroup(controller.activeDeck);
        var value;
        var change;
        var callback = undefined;
        if (group==undefined)
            return;
        if (field.name=="arrow_up") 
            callback = function(field) {
                var group = Wiimote.controller.resolveGroup(field.group);
                engine.setValue(group,"rate",engine.getValue(group,"rate")+0.02);
            }
        if (field.name=="arrow_down") 
            callback = function(field) {
                var group = Wiimote.controller.resolveGroup(field.group);
                engine.setValue(group,"rate",engine.getValue(group,"rate")-0.02);
            }
        controller.setAutoRepeat("deck",field.name,callback,50);
    }
}

// Call cue_default, or select deck if shift (bottom button) is pressed
Wiimote.deck_cue = function(field) { 
    var controller = Wiimote.controller;
    if (!controller.modifiers.get("shift"))  {
        var group = controller.resolveDeckGroup(controller.activeDeck);
        if (field.value==controller.buttonStates.released)
            engine.setValue(group,"cue_default",false);
        else
            engine.setValue(group,"cue_default",true);
    } else {
        if (field.value==controller.buttonStates.released)
            return;
        controller.switchDeck();
        HIDDebug("Active deck now " + controller.activeDeck);
    }
}

Wiimote.play_load = function(field) { 
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (group==undefined)
        return;
    if (field.value==controller.buttonStates.released)
        return;
    if (!controller.modifiers.get("shift"))  {
        controller.togglePlay(group,field);
    } else {
        engine.setValue(group,"LoadSelectedTrack",true);
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
            callback = function(field) { 
                var group = Wiimote.controller.resolveGroup(field.group);
                engine.setValue(group,"jog",-1);
            };
        } else {
            callback = function(field) { 
                var group = Wiimote.controller.resolveGroup(field.group);
                engine.setValue(group,"jog",-6);
            };
        }
        controller.setAutoRepeat("deck","button_minus",callback,50);
    }
    if (field.name=="button_plus") {
        if (!controller.modifiers.get("shift")) {
            callback = function(field) { 
                var group = Wiimote.controller.resolveGroup(field.group);
                engine.setValue(group,"jog",1);
            };
        } else {
            callback = function(field) { 
                var group = Wiimote.controller.resolveGroup(field.group);
                engine.setValue(group,"jog",6);
            };
        }
        controller.setAutoRepeat("deck","button_plus",callback,50);
    }
}

Wiimote.hotcue = function(field) { 
    var controller = Wiimote.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (group==undefined)
        return;
    if (controller.modifiers.get("shift"))  {
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


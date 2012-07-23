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

        packet.addControl("hid","arrow_left",1,"B",0x1);
        packet.addControl("hid","arrow_right",1,"B",0x2);
        packet.addControl("hid","arrow_down",1,"B",0x4);
        packet.addControl("hid","arrow_up",1,"B",0x8);
        packet.addControl("hid","button_plus",1,"B",0x10);
        packet.addControl("hid","button_1",2,"B",0x2);
        packet.addControl("hid","button_2",2,"B",0x1);
        packet.addControl("hid","button_bottom",2,"B",0x4);
        packet.addControl("hid","button_a",2,"B",0x8);
        packet.addControl("hid","button_minus",2,"B",0x10);
        packet.addControl("hid","button_home",2,"B",0x80);
        this.controller.registerInputPacket(packet);
    }

    this.registerOutputPackets = function() {
        packet = new HIDPacket("leds",[0x11],2);
        packet.addControl("hid","led_1",1,"B",0x10);
        packet.addControl("hid","led_2",1,"B",0x20);
        packet.addControl("hid","led_3",1,"B",0x40);
        packet.addControl("hid","led_4",1,"B",0x80);
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
    var controller = Wiimote.controller;
    controller.activeDeck = 1;
    controller.auto_repeat_interval = 50;
    Wiimote.registerInputPackets();
    Wiimote.registerOutputPackets();
    Wiimote.registerCallbacks();

    controller.startAutoRepeatTimer = function(timer_id,interval) {
        if (controller.timers[timer_id])
            return;
        controller.timers[timer_id] = engine.beginTimer(
            interval,
            "Wiimote.controller.autorepeatTimer()"
        )
    }
    Wiimote.setActiveDeckLED(controller.activeDeck);
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

// Light the LEDs to indicate which deck we are controlling
Wiimote.setActiveDeckLED = function(deck) {
    var controller = Wiimote.controller;
    var packet = Wiimote.controller.getOutputPacket("leds");
    var active_led = "led_" + deck;
    controller.setOutput("hid","led_1",0);
    controller.setOutput("hid","led_2",0);
    controller.setOutput("hid","led_3",0);
    controller.setOutput("hid","led_4",0);
    controller.setOutput("hid",active_led,1);
    HIDDebug("Sending packet " + packet.length + " bytes");
    packet.send();
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
    controller.linkModifier("hid","button_bottom","shift");
    controller.setCallback("control","hid","button_home",Wiimote.deck_cue);
    controller.setCallback("control","hid","button_a",Wiimote.play_load);

    controller.setCallback("control","hid","button_minus",Wiimote.jog);
    controller.setCallback("control","hid","button_plus",Wiimote.jog);

    controller.setCallback("control","hid","arrow_left",Wiimote.seek_loop);
    controller.setCallback("control","hid","arrow_right",Wiimote.seek_loop);

    controller.setCallback("control","hid","arrow_up",Wiimote.select);
    controller.setCallback("control","hid","arrow_down",Wiimote.select);

    controller.setCallback("control","hid","button_1",Wiimote.hotcue);
    controller.setCallback("control","hid","button_2",Wiimote.hotcue);
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
        controller.setAutoRepeat("hid",field.name,callback,100);
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
        controller.setAutoRepeat("hid",field.name,callback,100);
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
        Wiimote.setActiveDeckLED(controller.activeDeck);
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
            engine.setValue(group,"jog",-2);
            callback = function(field) { 
                engine.setValue(
                  Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck),
                  "jog",-4
                );
            };
        } else {
            engine.setValue(group,"jog",-6);
            callback = function(field) { 
                engine.setValue(
                  Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck),
                  "jog",-1
                );
            };
        }
        controller.setAutoRepeat("hid","button_minus",callback,100);
    }
    if (field.name=="button_plus") {
        if (!controller.modifiers.get("shift")) {
            engine.setValue(group,"jog",2);
            callback = function(field) { 
                engine.setValue(
                  Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck),
                  "jog",4
                );
            };
        } else {
            engine.setValue(group,"jog",6);
            callback = function(field) { 
                engine.setValue(
                  Wiimote.controller.resolveDeckGroup(Wiimote.controller.activeDeck),
                  "jog",1
                );
            };
        }
        controller.setAutoRepeat("hid","button_plus",callback,100);
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


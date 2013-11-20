//
// Demo script to print events from apple bluetooth keyboard on OS/X
// Copyright (C) 2012, Ilkka Tuohela
// Feel free to use whatever way wish1
//

HIDKeyboard = new HIDKeyboardDevice();

HIDKeyboard.init = function(id) {
    HIDKeyboard.id = id;
    HIDKeyboard.registerInputPackets();
    HIDKeyboard.registerOutputPackets();
    HIDKeyboard.registerScalers();
    HIDKeyboard.registerCallbacks();
    HIDDebug("HID Keyboard Initialized: " + HIDKeyboard.id);
}

HIDKeyboard.shutdown = function() {
    HIDDebug("HID Keyboard Shutdown: " + HIDKeyboard.id);
}

HIDKeyboard.incomingData = function(data,length) {
    var controller = HIDKeyboard.controller;
    if (controller==undefined) {
        HIDDebug("Error in script initialization: controller not found");
        return;
    }
    controller.parsePacket(data,length);
}


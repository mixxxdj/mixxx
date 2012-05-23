//
// Demo script to print events from apple bluetooth keyboard on OS/X
// Copyright (C) 2012, Ilkka Tuohela
// Feel free to use whatever way wish1
//

AppleBluetoothKeyboard = new HIDKeyboard();

AppleBluetoothKeyboard.init = function(id) {
    AppleBluetoothKeyboard.id = id;
    AppleBluetoothKeyboard.registerInputPackets();
    AppleBluetoothKeyboard.registerOutputPackets();
    AppleBluetoothKeyboard.registerScalers();
    AppleBluetoothKeyboard.registerCallbacks();
    script.HIDDebug("HID Keyboard Initialized: " + AppleBluetoothKeyboard.id);
}

AppleBluetoothKeyboard.shutdown = function() {
    script.HIDDebug("HID Keyboard Shutdown: " + AppleBluetoothKeyboard.id);
}

AppleBluetoothKeyboard.incomingData = function(data,length) {
    var controller = AppleBluetoothKeyboard.controller;
    if (controller==undefined) {
        script.HIDDebug("Error in script initialization: controller not found");
        return;
    }
    controller.parsePacket(data,length);
}


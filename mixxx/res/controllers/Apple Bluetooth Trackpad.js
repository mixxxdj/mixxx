//
// Demo script to print events from apple bluetooth trackpad on OS/X
// NOTE: the trackpad doesn't seem to actually send events to us. This
// is just a silly example anyway
// Copyright (C) 2012, Ilkka Tuohela
// Feel free to use whatever way wish1
//

AppleBluetoothTrackpad = new HIDTrackpad();

AppleBluetoothTrackpad.init = function(id) {
    AppleBluetoothTrackpad.id = id;
    AppleBluetoothTrackpad.registerInputPackets();
    AppleBluetoothTrackpad.registerOutputPackets();
    AppleBluetoothTrackpad.registerScalers();
    AppleBluetoothTrackpad.registerCallbacks();
    script.HIDDebug("Apple Bluetooth Trackpad Initialized: " + AppleBluetoothTrackpad.id);
}

AppleBluetoothTrackpad.shutdown = function() {
    script.HIDDebug("Apple Bluetooth Trackpad Shutdown: " + AppleBluetoothTrackpad.id);
}

AppleBluetoothTrackpad.incomingData = function(data,length) {
    var controller = AppleBluetoothTrackpad.controller;
    if (controller==undefined) {
        script.HIDDebug("Error in script initialization: controller not found");
        return;
    }
    controller.parsePacket(data,length);
}


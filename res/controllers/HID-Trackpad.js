//
// Demo script to print events from apple bluetooth trackpad on OS/X
// NOTE: the trackpad doesn't seem to actually send events to us. This
// is just a silly example anyway
// Copyright (C) 2012, Ilkka Tuohela
// Feel free to use whatever way wish1
//

HIDTrackpad = new HIDTrackpadDevice();

HIDTrackpad.init = function(id) {
    HIDTrackpad.id = id;
    HIDTrackpad.registerInputPackets();
    HIDTrackpad.registerOutputPackets();
    HIDTrackpad.registerScalers();
    HIDTrackpad.registerCallbacks();
    HIDDebug("HID Trackpad Initialized: " + HIDTrackpad.id);
}

HIDTrackpad.shutdown = function() {
    HIDDebug("HID Trackpad Shutdown: " + HIDTrackpad.id);
}

HIDTrackpad.incomingData = function(data,length) {
    var controller = HIDTrackpad.controller;
    if (controller==undefined) {
        HIDDebug("Error in script initialization: controller not found");
        return;
    }
    controller.parsePacket(data,length);
}


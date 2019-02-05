// Generic HID trackpad implementation
HIDTrackpadDevice = function() {
    this.controller = new HIDController()

    this.registerInputPackets = function() {
        // Example how to register a callback directly on a packet
        packet = new HIDPacket("control", 0, this.mouseInput)
        packet.addControl("hid", "byte_1", 3, "B")
        packet.addControl("hid", "byte_2", 4, "B")
        packet.addControl("hid", "byte_3", 5, "B")
        packet.addControl("hid", "byte_4", 6, "B")
        this.controller.registerInputPacket(packet)
    }

    // HID trackpads have no output controls
    this.registerOutputPackets = function() { }

    // No need to do scaling for keyboard presses
    this.registerScalers = function() { }

    // No need for callbacks here, we bound whole input packet to mouseInput
    this.registerCallbacks = function() { }

    // Example to process the mouse input packet all yourself
    this.mouseInput = function(packet, delta) {
        HIDDebug("Trackpad INPUT " + packet.name)
        if (!delta.length) {
            HIDDebug("No changed data received in HID packet")
            return
        }
        for (var field_name in delta) {
            var field = delta[field_name]
            HIDDebug("FIELD " + field.id + " VALUE " + value)
        }
    }
}

// Generic HID keyboard implementation 
HIDKeyboardDevice = function() {
    this.controller = new HIDController()

    this.registerInputPackets = function() {
        packet = new HIDPacket("control", 0x1)
        packet.addControl("hid", "keycode_1", 3, "B")
        packet.addControl("hid", "keycode_2", 4, "B")
        packet.addControl("hid", "keycode_3", 5, "B")
        packet.addControl("hid", "keycode_4", 6, "B")
        packet.addControl("hid", "keycode_5", 7, "B")
        packet.addControl("hid", "keycode_6", 8, "B")
        this.controller.registerInputPacket(packet)
    }

    // HID keyboards have no output controls
    this.registerOutputPackets = function() { }

    // No need to do scaling for keyboard presses
    this.registerScalers = function() { }

    // Example to bind the bytes to a callback
    this.registerCallbacks = function() {
        this.controller.setCallback("control", "hid", "keycode_1", this.keyPress)
        this.controller.setCallback("control", "hid", "keycode_2", this.keyPress)
        this.controller.setCallback("control", "hid", "keycode_3", this.keyPress)
        this.controller.setCallback("control", "hid", "keycode_4", this.keyPress)
        this.controller.setCallback("control", "hid", "keycode_5", this.keyPress)
        this.controller.setCallback("control", "hid", "keycode_6", this.keyPress)
    }

    // Example to do something with the keycodes received
    this.keyPress = function(field) {
        if (field.value != 0)
            HIDDebug("KEY PRESS " + field.id + " CODE " + field.value)
        else
            HIDDebug("KEY RELEASE " + field.id)
    }
}


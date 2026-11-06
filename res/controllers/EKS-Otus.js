//
// EKS Otus HID controller script v1.0
// Copyright (C) 2012, Sean M. Pappalardo, Ilkka Tuohela
// but feel free to tweak this to your heart's content!
// For Mixxx version 1.11.x
//

// --- Compatibility shim for Mixxx 2.5's common-hid-packet-parser.js -------
// This script's output linking (controller.linkOutput()/EksOtus.outputCallback,
// unchanged since Mixxx 2.2 - verified against the 2.2.0 release source)
// depends on HIDController.getOutputField(m_group, m_name) being able to find
// an output field by its *mapped* Mixxx group/name (e.g. "deck","play"), not
// just its raw HID group/name ("hid","play").
//
// In Mixxx 2.2 this worked because getOutputField() did a live linear scan
// over every registered output field on every call, checking both
// field.mapped_group/mapped_name AND field.group/name - so it kept working
// no matter when a field got linked.
//
// In Mixxx 2.5 this was rewritten as an O(1) OutputFieldLookup Map for
// performance. registerOutputPacket() populates that Map from both the raw
// and mapped identities, but only once, at packet registration time - before
// any linkOutput() call has run, so mapped_group/mapped_name don't exist yet.
// The Map is never updated afterwards, so every lookup by mapped identity
// (linkOutput()'s own initial LED sync, and every call to outputCallback()
// on a subsequent control change) misses and logs
// "HIDController.setOutput - Unknown field: deck.X". This is a genuine
// engine regression, not a bug in this driver - restore the old fallback
// behavior here (and cache the result back into the Map, so this only
// costs a scan once per field).
if (typeof HIDController !== "undefined" &&
        HIDController.prototype.getOutputField &&
        !HIDController.prototype._eksOtusGetOutputFieldPatched) {
    var _eksOtusOriginalGetOutputField = HIDController.prototype.getOutputField;
    HIDController.prototype.getOutputField = function(m_group, m_name) {
        var field = _eksOtusOriginalGetOutputField.call(this, m_group, m_name);
        if (field !== undefined) {
            return field;
        }
        // Fall back to a Mixxx-2.2-style linear scan by mapped identity.
        for (var packet_name in this.OutputPackets) {
            var packet = this.OutputPackets[packet_name];
            for (var group_name in packet.groups) {
                var group = packet.groups[group_name];
                for (var field_name in group) {
                    var candidate = group[field_name];
                    if (candidate.type === "bitvector") {
                        for (var bit_id in candidate.value.bits) {
                            var bit = candidate.value.bits[bit_id];
                            if (bit.mapped_group === m_group && bit.mapped_name === m_name) {
                                this.OutputFieldLookup.set([m_group, m_name].toString(), bit);
                                return bit;
                            }
                        }
                        continue;
                    }
                    if (candidate.mapped_group === m_group && candidate.mapped_name === m_name) {
                        this.OutputFieldLookup.set([m_group, m_name].toString(), candidate);
                        return candidate;
                    }
                }
            }
        }
        return undefined;
    };
    HIDController.prototype._eksOtusGetOutputFieldPatched = true;
}
// ---------------------------------------------------------------------------

// EKS Otus HID interface specification
function EKSOtusController() {
    this.controller = new HIDController();

    // Initialized to firmware version by version response packet
    this.version_major = undefined;
    this.version_minor = undefined;
    this.controller.activeDeck = 1;

    this.controller.LEDColors = { off: 0x0, red: 0x0f, green: 0xf0, amber: 0xff };
    this.controller.deckOutputColors = { 1: "red", 2: "green", 3: "red", 4: "green"};

    // Static variables for HID specs
    this.wheelLEDCount = 60;
    this.buttonLEDCount = 22;
    this.sliderLEDCount = 20;

    this.registerInputPackets = function() {
        var packet = undefined;
        var name = undefined;
        var offset = 0;

        packet = new HIDPacket("control", 0, undefined, [0x00, 0x35]);
        packet.addControl("hid","wheel_position",2,"H");
        packet.addControl("hid","wheel_speed",4,"h");
        packet.addControl("hid","timestamp",6,"I");
        packet.addControl("hid","slider_value",10,"H");
        packet.addControl("hid","slider_position",12,"H");
        packet.addControl("hid","rate_encoder",14,"B",undefined,true);
        packet.addControl("hid","jog_se",15,"B",undefined,true);
        packet.addControl("hid","jog_sw",16,"B",undefined,true);
        packet.addControl("hid","rate_encoder",17,"B",undefined,true);
        packet.addControl("hid","gain_1",18,"H");
        packet.addControl("hid","gain_2",20,"H");
        packet.addControl("hid","eq_high_1",22,"H");
        packet.addControl("hid","eq_high_2",24,"H");
        packet.addControl("hid","eq_mid_1",26,"H");
        packet.addControl("hid","eq_mid_2",28,"H");
        packet.addControl("hid","eq_low_1",30,"H");
        packet.addControl("hid","eq_low_2",32,"H");
        packet.addControl("hid","crossfader",34,"H");
        packet.addControl("hid","headphones",36,"H");
        packet.addControl("hid","trackpad_x",38,"H");
        packet.addControl("hid","trackpad_y",40,"H");
        packet.addControl("hid","slider_pos_2",42,"H");
        packet.addControl("hid","slider_pos_1",44,"H");
        packet.addControl("hid","keylock",46,"I",0x1);
        packet.addControl("hid","beatloop_8",46,"I",0x2);
        packet.addControl("hid","beatloop_4",46,"I",0x4);
        packet.addControl("hid","beatloop_2",46,"I",0x8);
        packet.addControl("hid","beatloop_1",46,"I",0x10);
        packet.addControl("hid","loop_in",46,"I",0x20);
        packet.addControl("hid","loop_out",46,"I",0x40);
        packet.addControl("hid","reloop_exit",46,"I",0x80);
        packet.addControl("hid","slider_scale",46,"I",0x100);
        packet.addControl("hid","jog_se_button",46,"I",0x200);
        packet.addControl("hid","eject_right",46,"I",0x400);
        packet.addControl("hid","deck_switch",46,"I",0x800);
        packet.addControl("hid","eject_left",46,"I",0x1000);
        packet.addControl("hid","jog_sw_button",46,"I",0x2000);
        packet.addControl("hid","stop",46,"I",0x4000);
        packet.addControl("hid","play",46,"I",0x8000);
        packet.addControl("hid","cue",46,"I",0x10000);
        packet.addControl("hid","reverse",46,"I",0x20000);
        packet.addControl("hid","brake",46,"I",0x40000);
        packet.addControl("hid","fastforward",46,"I",0x80000);
        packet.addControl("hid","jog_nw_button",46,"I",0x100000);
        packet.addControl("hid","jog_touch",46,"I",0x200000);
        packet.addControl("hid","trackpad_left",46,"I",0x400000);
        packet.addControl("hid","trackpad_right",46,"I",0x800000);
        packet.addControl("hid","hotcue_1",46,"I",0x1000000);
        packet.addControl("hid","hotcue_2",46,"I",0x2000000);
        packet.addControl("hid","hotcue_3",46,"I",0x4000000);
        packet.addControl("hid","hotcue_4",46,"I",0x8000000);
        packet.addControl("hid","hotcue_5",46,"I",0x10000000);
        packet.addControl("hid","hotcue_6",46,"I",0x20000000);
        packet.addControl("hid","touch_slider",46,"I",0x40000000)
        packet.addControl("hid","touch_trackpad",46,"I",0x80000000);
        packet.addControl("hid","packet_number",51,"B");
        packet.addControl("hid","deck_status",52,"B");
        this.controller.registerInputPacket(packet);
        
        packet = new HIDPacket("firmware_version", 0xa, undefined, [0x0a, 0x04]);
        packet.addControl("hid","major",2,"B");
        packet.addControl("hid","minor",3,"B");
        this.controller.registerInputPacket(packet);

        packet = new HIDPacket("trackpad_mode", 0x5, undefined, [0x05, 0x03]);
        packet.addControl("hid","status",2,"B");
        this.controller.registerInputPacket(packet);


    }

    this.registerOutputPackets = function() {
        var packet = undefined;
        var name = undefined;
        var offset = 0;

        packet = new HIDPacket("button_leds", 0x16, undefined, [0x18]);
        offset = 2; // matches original 2.2.0 raw offset; addOutput()'s internal -1 shim then lands this at data[1], right after the 1-byte header
        packet.addOutput("hid","jog_nw",offset++,"B");
        packet.addOutput("hid","jog_ne",offset++,"B");
        packet.addOutput("hid","jog_se",offset++,"B");
        packet.addOutput("hid","jog_sw",offset++,"B");
        packet.addOutput("hid","beatloop_8",offset++,"B");
        packet.addOutput("hid","beatloop_4",offset++,"B");
        packet.addOutput("hid","beatloop_2",offset++,"B");
        packet.addOutput("hid","beatloop_1",offset++,"B");
        packet.addOutput("hid","loop_in",offset++,"B");
        packet.addOutput("hid","loop_out",offset++,"B");
        packet.addOutput("hid","reloop_exit",offset++,"B");
        packet.addOutput("hid","eject_right",offset++,"B");
        packet.addOutput("hid","deck_switch",offset++,"B");
        packet.addOutput("hid","trackpad_right",offset++,"B");
        packet.addOutput("hid","trackpad_left",offset++,"B");
        packet.addOutput("hid","eject_left",offset++,"B");
        packet.addOutput("hid","stop",offset++,"B");
        packet.addOutput("hid","play",offset++,"B");
        packet.addOutput("hid","reverse",offset++,"B");
        packet.addOutput("hid","cue",offset++,"B");
        packet.addOutput("hid","brake",offset++,"B");
        packet.addOutput("hid","fastforward",offset++,"B");
        packet.length = 31; // match stock 2.2.0's fixed 32-byte total report size (31 + 1 reportID)
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("slider_leds", 0x17, undefined, [0x16]);
        offset = 2;
        packet.addOutput("pitch","slider_1",offset++,"B");
        packet.addOutput("pitch","slider_2",offset++,"B");
        packet.addOutput("pitch","slider_3",offset++,"B");
        packet.addOutput("pitch","slider_4",offset++,"B");
        packet.addOutput("pitch","slider_5",offset++,"B");
        packet.addOutput("pitch","slider_6",offset++,"B");
        packet.addOutput("pitch","slider_7",offset++,"B");
        packet.addOutput("pitch","slider_8",offset++,"B");
        packet.addOutput("pitch","slider_9",offset++,"B");
        packet.addOutput("pitch","slider_10",offset++,"B");
        packet.addOutput("pitch","slider_11",offset++,"B");
        packet.addOutput("pitch","slider_12",offset++,"B");
        packet.addOutput("pitch","slider_13",offset++,"B");
        packet.addOutput("pitch","slider_14",offset++,"B");
        packet.addOutput("pitch","slider_15",offset++,"B");
        packet.addOutput("pitch","slider_16",offset++,"B");
        packet.addOutput("pitch","slider_17",offset++,"B");
        packet.addOutput("pitch","slider_scale_1",offset++,"B");
        packet.addOutput("pitch","slider_scale_2",offset++,"B");
        packet.addOutput("pitch","slider_scale_3",offset++,"B");
        packet.length = 31;
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("led_wheel_left", 0x14, undefined, [0x20]);
        offset = 2;
        for (var led_index=1;led_index<=this.wheelLEDCount/2;led_index++)
            packet.addOutput("hid","wheel_" + led_index,offset++,"B");
        packet.length = 31;
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("led_wheel_right", 0x15, undefined, [0x20]);
        offset = 2;
        for (var led_index=this.wheelLEDCount/2+1;led_index<=this.wheelLEDCount;led_index++)
            packet.addOutput("hid","wheel_" + led_index,offset++,"B");
        packet.length = 31;
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("request_firmware_version", 0xa, undefined, [0x2]);
        packet.length = 31;
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("set_trackpad_mode", 0x5, undefined, [0x3]);
        packet.addOutput("hid","mode",2,"B");
        packet.length = 31;
        this.controller.registerOutputPacket(packet);

        packet = new HIDPacket("set_ledcontrol_mode", 0x1d, undefined, [0x3]);
        packet.addOutput("hid","mode",2,"B");
        packet.length = 31;
        this.controller.registerOutputPacket(packet);
    }

    // Otus specific output packet to request device firmware version
    this.requestFirmwareVersion = function() {
        var packet = this.controller.getOutputPacket("request_firmware_version");
        if (packet==undefined)
            return;
        HIDDebug("Requesting firmware version " + packet.name);
        packet.send();
    }

    // Set LED Control Mode on Otus firmware versions > 1.6. Major and minor must
    // contain the version numbers for firmware as received from response.
    // Valid modes are:
    //      0   disable all LEDs
    //      1   Re-enable LEDs
    //      2   Revert to built-in light functionality
    this.setLEDControlMode = function(mode) {
        var controller = this.controller;
        if (this.version_major<=1 && this.version_minor<6) {
            // Firmware version does not support LED Control Mode Setting
            return;
        }
        if (mode!=0 && mode!=1 && mode!=2) {
            HIDDebug("Unknown value for LED Control Mode Setting: " + mode);
            return;
        }
        var packet = controller.getOutputPacket("set_ledcontrol_mode");
        var field = packet.getField("hid","mode");
        if (field==undefined) {
            HIDDebug("EksOtus.setLEDControlMode error fetching field mode");
            return;
        }
        field.value = mode;
        packet.send();
    }

    // Firmware version response. Required to finish device INIT
    this.FirmwareVersionResponse = function(packet,delta) {
        var controller = this.controller;
        var field_major = packet.getField("hid","major");
        var field_minor = packet.getField("hid","minor");
        if (field_major==undefined || field_minor==undefined) {
            HIDDebug("Error parsing response version packet");
            return;
        }
        this.version_major = field_major.value;
        this.version_minor = field_minor.value;
        controller.initialized = true;

        this.setLEDControlMode(1);
        if (controller.activeDeck!=undefined) {
            controller.setOutput("hid","deck_switch", controller.LEDColors[controller.deckOutputColors[controller.activeDeck]]);
            controller.switchDeck(controller.activeDeck);
        } else {
            var value = controller.LEDColors["amber"];
            this.controller.setOutputToggle("hid","deck_switch",value);
        }
        this.updateLEDs();
        HIDDebug("EKS " + EksOtus.id +
            " v"+EksOtus.version_major+"."+EksOtus.version_minor+
            " initialized"
        );
    }

    // Otus specific output packet to set the trackpad control mode
    this.setTrackpadMode = function(mode) {
        if (mode!=0 && mode!=1) {
            HIDDebug("Unsupported trackpad mode value: " + mode);
            return;
        }
        var packet = this.controller.getOutputPacket("set_trackpad_mode");
        if (packet==undefined) {
            HIDDebug("Output not registered: set_trackpad_mode");
            return;
        }
        var field = packet.getField("hid","mode");
        if (field==undefined) {
            HIDDebug("EksOtus.setTrackpadMode error fetching field mode");
            return;
        }
        field.value = mode;
        packet.send();
    }

    // Response to above trackpad mode packet
    this.TrackpadModeResponse = function(packet,delta) {
        field = packet.getField("hid","status");
        if (field==undefined) {
            HIDDebug("Error parsing field status from packet");
            return;
        }
        if (field.value==1) {
            HIDDebug("Trackpad mode successfully set");
        } else {
            HIDDebug("Trackpad mode change failed");
        }
    }

    // Generic unsigned short to -1..0..1 range scaling
    this.plusMinus1Scaler = function(group,name,value) {
        if (value<32768)
            return value/32768-1;
        else
            return (value-32768)/32768;
    }

    // Volume slider scaling for 0..1..5 scaling
    this.volumeScaler = function(group,name,value) {
        return script.absoluteNonLin(value, 0, 1, 5, 0, 65536);
    }

    // EQ scaling function for 0..1..4 scaling
    this.eqScaler = function(group,name,value) {
        return script.absoluteNonLin(value, 0, 1, 4, 0, 65536);
    }

    // Mandatory call from init() to initialize hardware
    this.initializeHIDController = function() {
        this.registerInputPackets();
        this.registerOutputPackets();
    }

    this.shutdownHardware = function() {
        this.setLEDControlMode(2);
        this.setTrackpadMode(1);
    }

}

EksOtus = new EKSOtusController();

// Initialize device state, send request for firmware. Otus is not
// usable before we receive a valid firmware version response.
EksOtus.init = function (id) {
    EksOtus.id = id;

    EksOtus.LEDUpdateInterval = 250;
    // Valid values: 1 for mouse mode, 0 for xy-pad mode
    EksOtus.trackpadMode = 0;
    EksOtus.deckSwitchHeld = false;
    EksOtus.deckSwitchHoldPending = false;
    EksOtus.deckSwitchDebounceIgnoring = false;
    // Wheel absolute position value
    EksOtus.wheelPosition = undefined;
    // Wheel spin animation details
    EksOtus.activeTrackDuration = undefined;
    // Group registered to update spinning platter details
    EksOtus.activeSpinningPlatterGroup = undefined;
    // Virtual record spin time, 1.8 for 33 1/3 RPM, 1.33 for 45 RPM
    EksOtus.revTime = 1.8;
    EksOtus.pitchModifierActive = false;
    // Wheel LED index, range 1-60
    EksOtus.activeSpinningPlatterLED = undefined;

    // Call the HID packet parser initializers
    EksOtus.initializeHIDController();
    var controller = EksOtus.controller;
    // Set callbacks for packets here to avoid issues in callback handling
    controller.setPacketCallback("firmware_version",EksOtus.FirmwareVersionWrapper);
    controller.setPacketCallback("trackpad_mode",EksOtus.TrackpadModeWrapper);

    // NOTE: the engine keys every changed field as "<group>.<name>" (see
    // HIDPacket.parse/parseBitVector in common-hid-packet-parser.js), so an
    // entry here must include the "hid." group prefix to actually match and
    // be skipped. "deck_status", "slider_pos_1", "slider_pos_2", and
    // "slider_value" were missing that prefix and were therefore never
    // actually ignored. In practice the three slider fields have their own
    // setCallback() handler so they short-circuit before this matters, but
    // "deck_status" has none, so its value changes fell through to the
    // generic engine.setValue("hid", "deck_status", ...) fallback - and
    // since "hid" isn't a real Mixxx control group, that logs
    // "ControlDoublePrivate::getControl returning NULL for ("hid","deck_status")".
    controller.ignoredControlChanges = [
        "mask","hid.timestamp","hid.packet_number","hid.deck_status", "hid.wheel_speed",
        // These return the Otus slider position scaled by the 'slider scale'
        "hid.slider_pos_1","hid.slider_pos_2", "hid.slider_value"
    ];

    // Scratch parameters
    controller.scratchintervalsPerRev = 1024;
    controller.scratchAlpha = 1.0/8;
    // NOTE: 'rampedScratchEnable' is not a real HIDController property (the
    // engine reads scratchRampOnEnable/scratchRampOnDisable), so this line
    // never had any effect since the script was written - scratch start/stop
    // has always defaulted to an instant jump instead of ramping. Setting
    // the real properties makes engaging/releasing the wheel ramp smoothly,
    // which is part of what should make scratching feel less abrupt/jerky.
    controller.scratchRampOnEnable = true;
    controller.scratchRampOnDisable = true;

    EksOtus.setTrackpadMode(this.trackpadMode);
    // Note: Otus is not considered initialized before we get
    // response to this packet
    EksOtus.requestFirmwareVersion();
    // Link controls and register callbacks
    EksOtus.registerCallbacks();

    // CHANGED: headVolume -> headGain for Mixxx 2.5+
    engine.softTakeover("[Master]","headGain",true);
    engine.softTakeover("[Master]","headMix",true);
    for (var deck in controller.deckOutputColors) {
        engine.softTakeover("[Channel"+deck+"]","pregain",true);
        engine.softTakeover("[Channel"+deck+"]","volume",true);
    }

    if (EksOtus.LEDUpdateInterval!=undefined) {
        controller.timers["led_update"] = engine.beginTimer(
            EksOtus.LEDUpdateInterval,
            () => EksOtus.updateLEDs(true)
        );
    }

}

// Callback bound to each of the 11 deck-tied outputs below via linkOutput().
// Unchanged from the Mixxx 2.2 original - setOutput("deck", key, ...) only
// resolves correctly now because of the getOutputField compatibility shim
// at the top of this file (Mixxx 2.5's engine broke mapped-identity lookups).
EksOtus.outputCallback = function(value, group, key) {
    var controller = EksOtus.controller;
    if (group=="deck") {
        if (controller.activeDeck==undefined)
            return;
        group = controller.resolveGroup("deck");
    }
    if (value==1)
        EksOtus.controller.setOutput("deck",key,
            controller.LEDColors[controller.deckOutputColors[controller.activeDeck]],
            true
        );
    else
        EksOtus.controller.setOutput("deck",key,controller.LEDColors.off,true);
}

// Mixxx's HIDPacket.send() (common-hid-packet-parser.js) always calls
// controller.sendOutputReport(reportId, data, useNonSkippingFIFO=false).
// With that default, Mixxx's native HID I/O thread silently SKIPS writing
// a report to the device if its bytes are identical to the last data it
// queued for that report ID - a deliberate throughput optimization, not a
// bug (see src/controllers/hid/hidcontroller.h). On this device that skip
// logic appears to also suppress the very first real write, so the
// hardware never receives the report and the LEDs never light. Passing
// useNonSkippingFIFO=true forces every write through unconditionally.
// This re-implements HIDPacket.send()'s packing step locally so we can
// call controller.sendOutputReport() ourselves with that flag set,
// without needing to modify the shared library file.
EksOtus.forceSendPacket = function(packet) {
    // NOTE: do NOT shadow the name "controller" here. Mixxx injects a
    // global "controller" object (the native HidControllerJSProxy with
    // sendOutputReport/send/etc). EksOtus.controller is a DIFFERENT
    // object - our own JS HIDController wrapper instance (getOutputPacket,
    // setOutput, ...) - and has no sendOutputReport method at all.
    var data = new Uint8Array(packet.length);
    if (packet.header !== undefined) {
        for (var header_byte = 0; header_byte < packet.header.length; header_byte++) {
            data[header_byte] = packet.header[header_byte];
        }
    }
    for (var group_name in packet.groups) {
        var group = packet.groups[group_name];
        for (var field_name in group) {
            packet.pack(data, group[field_name]);
        }
    }
    controller.sendOutputReport(packet.reportId, data.buffer, true);
}

EksOtus.updateLEDs = function(from_timer) {
    var controller = EksOtus.controller;
    EksOtus.forceSendPacket(controller.getOutputPacket("button_leds"));
    EksOtus.forceSendPacket(controller.getOutputPacket("slider_leds"));
    EksOtus.forceSendPacket(controller.getOutputPacket("led_wheel_left"));
    EksOtus.forceSendPacket(controller.getOutputPacket("led_wheel_right"));
}

// Device cleanup function
EksOtus.shutdown = function() {
    // CHANGED: headVolume -> headGain for Mixxx 2.5+
    engine.softTakeover("[Master]","headGain",false);
    engine.softTakeover("[Master]","headMix",false);
    for (var deck in EksOtus.controller.deckOutputColors) {
        engine.softTakeover("[Channel"+deck+"]","pregain",false);
        engine.softTakeover("[Channel"+deck+"]","volume",false);
    }
    EksOtus.shutdownHardware(2);
    HIDDebug("EKS "+EksOtus.id+" shut down");
}

// Mandatory default handler for incoming packets
EksOtus.incomingData = function(data,length) {
    EksOtus.controller.parsePacket(data,length);
}

EksOtus.FirmwareVersionWrapper = function(packet,data) {
    return EksOtus.FirmwareVersionResponse(packet,data);
}

EksOtus.TrackpadModeWrapper = function(packet,data) {
    return EksOtus.TrackpadModeResponse(packet,data);
}

// Callback to set current loaded track's duration for wheel led animation
EksOtus.loadedTrackDuration = function(value) {
    EksOtus.activeTrackDuration = value;
}

// Link virtual HID naming of input and LED controls to mixxx
// Note: HID specification has more fields than we map here.
EksOtus.registerCallbacks = function() {
    var controller = EksOtus.controller;

    controller.modifiers.add("shift");
    controller.modifiers.add("shift");
    controller.linkModifier("hid","eject_right","shift");
    controller.setCallback("control","hid","touch_slider",function(field) { EksOtus.pitchModifierActive = (field.value == 1); });

    controller.linkControl("hid","play","deck","play");
    controller.linkControl("hid","cue","deck","cue_default");
    controller.linkControl("hid","reverse","deck","reverse");
    controller.linkControl("hid","eject_left","deck","pfl");
    controller.linkControl("hid","jog_touch","deck","jog_touch");
    controller.linkControl("hid","wheel_position","deck","jog_wheel");

    controller.linkControl("hid","jog_se_button","deck","LoadSelectedTrack");
    controller.linkControl("hid","jog_se","[Playlist]","SelectTrackKnob");

    controller.linkControl("hid","crossfader","[Master]","crossfader");
    controller.linkControl("hid","gain_1","deck1","pregain");
    controller.linkControl("hid","gain_2","deck2","pregain");
    controller.linkControl("hid","eq_high_1","deck1","filterHigh");
    controller.linkControl("hid","eq_high_2","deck2","filterHigh");
    controller.linkControl("hid","eq_mid_1","deck1","filterMid");
    controller.linkControl("hid","eq_mid_2","deck2","filterMid");
    controller.linkControl("hid","eq_low_1","deck1","filterLow");
    controller.linkControl("hid","eq_low_2","deck2","filterLow");

    controller.setScaler("jog",EksOtus.jogScaler);
    controller.setScaler("jog_scratch",EksOtus.wheelScaler);
    controller.setScaler("crossfader",EksOtus.plusMinus1Scaler);
    controller.setScaler("pregain",EksOtus.eqScaler);
    controller.setScaler("filterHigh",EksOtus.eqScaler);
    controller.setScaler("filterMid",EksOtus.eqScaler);
    controller.setScaler("filterLow",EksOtus.eqScaler);

    controller.setCallback("control","hid","hotcue_1",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_2",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_3",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_4",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_5",EksOtus.hotcue);
    controller.setCallback("control","hid","hotcue_6",EksOtus.hotcue);

    controller.setCallback("control","hid","beatloop_1",EksOtus.beatloop);
    controller.setCallback("control","hid","beatloop_2",EksOtus.beatloop);
    controller.setCallback("control","hid","beatloop_4",EksOtus.beatloop);
    controller.setCallback("control","hid","beatloop_8",EksOtus.beatloop);
    controller.linkControl("hid","loop_in","deck","loop_in");
    controller.linkControl("hid","loop_out","deck","loop_out");
    controller.linkControl("hid","reloop_exit","deck","reloop_exit");

    controller.setCallback("control","hid","deck_switch",EksOtus.deckSwitch);

    //controller.linkControl("hid","headphones","[Master]","headphones");
    controller.setCallback("control","hid","headphones",EksOtus.headphones);

    controller.setCallback("control","hid","slider_scale",EksOtus.pitchSlider);
    controller.setCallback("control","hid","slider_value",EksOtus.pitchSlider);
    controller.setCallback("control","hid","slider_position",EksOtus.pitchSlider);
    controller.setCallback("control","hid","slider_pos_1",EksOtus.pitchSlider);
    controller.setCallback("control","hid","slider_pos_2",EksOtus.pitchSlider);

    controller.linkOutput("hid","beatloop_8","deck","beatloop_8_enabled",EksOtus.outputCallback);
    controller.linkOutput("hid","beatloop_4","deck","beatloop_4_enabled",EksOtus.outputCallback);
    controller.linkOutput("hid","beatloop_2","deck","beatloop_2_enabled",EksOtus.outputCallback);
    controller.linkOutput("hid","beatloop_1","deck","beatloop_1_enabled",EksOtus.outputCallback);
    controller.linkOutput("hid","loop_in","deck","loop_in",EksOtus.outputCallback);
    controller.linkOutput("hid","loop_out","deck","loop_out",EksOtus.outputCallback);
    controller.linkOutput("hid","reloop_exit","deck","reloop_exit",EksOtus.outputCallback);
    controller.linkOutput("hid","eject_left","deck","pfl",EksOtus.outputCallback);
    controller.linkOutput("hid","play","deck","play",EksOtus.outputCallback);
    controller.linkOutput("hid","reverse","deck","reverse",EksOtus.outputCallback);
    controller.linkOutput("hid","cue","deck","cue_default",EksOtus.outputCallback);

}

// Default scaler for jog values
EksOtus.wheelScaler = function(group,name,value) {
    if (EksOtus.wheelPosition==undefined) {
        EksOtus.wheelPosition = value;
        return 0;
    }
    var delta = EksOtus.wheelPosition - value;
    // wheel_position wraps around as a 16-bit counter (0-65535). The old
    // check only caught the wrap in one direction (delta>32768), so
    // scratching backward across the wrap point produced delta values
    // like -65525 that fell straight into the "large movement" branch
    // below and were returned as a huge, wrong tick (~+4096) instead of
    // being discarded like the forward-wrap case already was.
    if (delta>32768 || delta<-32768)
        return 0;
    EksOtus.wheelPosition = value;
    if (delta>-8 && delta<8)
        return -delta/4;
    return -delta/16;
}

EksOtus.jogScaler = function(group,name,value) {
    if (EksOtus.wheelPosition==undefined) {
        EksOtus.wheelPosition = value;
        return 0;
    }
    var delta = EksOtus.wheelPosition - value;
    // Same wrap-around fix as wheelScaler above - catch both directions.
    if (delta>32768 || delta<-32768)
        return 0;
    EksOtus.wheelPosition = value;
    return -delta/64;
}

// Deck rate adjustment with top corner wheels
EksOtus.rate_wheel = function(field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    var active_group = controller.resolveGroup(field.group);
    var current = engine.getValue(active_group,"rate");
    if (field.delta<0)
        engine.setValue(active_group,"rate",current+0.003);
    else
        engine.setValue(active_group,"rate",current-0.003);
}

// Reset all wheel LEDs to given color. If color is undefined,
// use 'off'
EksOtus.resetWheelLEDs = function (color) {
    var controller = EksOtus.controller;
    if (color==undefined || !(color in controller.LEDColors))
        color = "off";
    // setOutput() needs the numeric LED value, not the color name string
    var color_value = controller.LEDColors[color];
    for (i=1;i<=EksOtus.wheelLEDCount;i++)
        controller.setOutput("hid","wheel_"+i,color_value,false);
    EksOtus.updateLEDs(true);
}

// Rotation of the Otus 'corner' wheels.
// Note right bottom wheel is library browser encoder and not handled here
EksOtus.corner_wheel = function(field) {
    // TODO - attach some functionality these corner wheels!
    print("CORNER " + field.name + " delta " + field.delta);
}

// Hotcues activated with normal press, cleared with shift
EksOtus.hotcue = function (field) {
    var controller = EksOtus.controller;
    var command;
    if (controller.activeDeck==undefined ||
        field.value==controller.buttonStates.released)
        return;
    var active_group = controller.resolveDeckGroup(controller.activeDeck);
    if (controller.modifiers.get("shift"))
        command = field.name + "_clear";
    else
        command = field.name + "_activate";
    engine.setValue(active_group,command,true);
}

// Beatloops activated with normal presses to beatloop_1 - beatloop_8
EksOtus.beatloop = function (field) {
    var controller = EksOtus.controller;
    var command;
    if (controller.activeDeck==undefined ||
        field.value==controller.buttonStates.released)
        return;
    var active_group = controller.resolveDeckGroup(controller.activeDeck);
    command = field.name + "_activate";
    engine.setValue(active_group,command,true);
}

EksOtus.beat_align = function (field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    var active_group = controller.resolveGroup(field.group);
    if (controller.modifiers.get("shift")) {
        // if (field.value==controller.buttonStates.released) return;
        engine.setValue(active_group,"beats_translate_curpos",field.value);
    } else {
        if (field.value==controller.buttonStates.released)
            return;
        if (!engine.getValue(active_group,"quantize"))
            engine.setValue(active_group,"quantize",true);
        else
            engine.setValue(active_group,"quantize",false);
    }
}

// Pitch slider modifies track speed directly
EksOtus.pitchSlider = function (field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    if (EksOtus.pitchModifierActive) {
        var active_group = controller.resolveDeckGroup(controller.activeDeck);
        if (field.name=="slider_position") {
            if (field.value==0)
                return;
            var value = EksOtus.plusMinus1Scaler(
                active_group,field.name,field.value
            );
            engine.setValue(active_group,"rate",value);
        }
    }
}

// Set pregain, if modifier shift is active, deck volume otherwise
EksOtus.volume_pregain = function (field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    var active_group = controller.resolveGroup(field.group);
    if (controller.modifiers.get("shift")) {
        value = script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536);
        engine.setValue(active_group,"pregain",value);
    } else {
        value = field.value / 65536;
        engine.setValue(active_group,"volume",value);
    }
}

// Set headphones volume, if modifier shift is active, pre/main mix otherwise
EksOtus.headphones = function (field) {
    var controller = EksOtus.controller;
    if (controller.modifiers.get("shift")) {
        value = script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536);
        // CHANGED: headVolume -> headGain for Mixxx 2.5+
        engine.setValue("[Master]","headGain",value);
    } else {
        value = EksOtus.plusMinus1Scaler(field.group,field.name,field.value);
        engine.setValue("[Master]","headMix",value);
    }
}

// Control effects or something with XY pad
EksOtus.xypad = function(field) {
    var controller = EksOtus.controller;
    if (controller.activeDeck==undefined)
        return;
    print ("XYPAD group " + field.group +
        " name " + field.name + " value " + field.value
    );
}

// How long (ms) 'deck_switch' must be held before it is treated as a
// long press that temporarily switches deck controls until released.
EksOtus.deckSwitchHoldTime = 400;

// How long (ms) to ignore a new 'deck_switch' press right after a tap was
// already handled. A real hardware log capture showed a single physical
// tap being reported as TWO complete press/release cycles back-to-back
// (switch contact bounce) - each one is individually a valid, fast tap, so
// the debounce guard above alone can't tell them apart. The first cycle
// switches decks, the second (spurious) cycle immediately switches back,
// so nothing visibly changes (or - as observed - the LED flashes the new
// color for an instant and then flips right back). 150ms was not always
// enough: on some taps the bounce runs a bit longer than that, so the
// trailing bounce cycle arrives just after the window closed and gets
// treated as a brand-new tap, switching the deck straight back. Widened
// to 300ms, which still leaves 100ms of clearance under deckSwitchHoldTime
// (400ms) so it can't interfere with a genuine hold.
EksOtus.deckSwitchDebounceTime = 300;
EksOtus.deckSwitchDebounceUntil = 0;

// Function called when the special 'Deck Switch' button is pressed
// TODO - add code for 'hold deck_switch and press hot_cue[1-4]
// to select deck 1-4
//
// Behaviour:
// - quick press/release (tap) -> persistent deck switch
// - press and hold for deckSwitchHoldTime -> temporary deck switch,
//   reverted automatically when the button is released
EksOtus.deckSwitch = function(field) {
    var controller = EksOtus.controller;
    if (EksOtus.initialized==false)
        return;

    if (field.value == controller.buttonStates.pressed) {
        if (Date.now() < EksOtus.deckSwitchDebounceUntil) {
            // Spurious repeated press (contact bounce) right after a tap
            // was already handled - ignore this whole press/release cycle.
            // Slide the window forward on every bounce we see so a longer
            // or multi-cycle bounce burst is fully swallowed instead of
            // only the first extra cycle.
            EksOtus.deckSwitchDebounceIgnoring = true;
            EksOtus.deckSwitchDebounceUntil = Date.now() + EksOtus.deckSwitchDebounceTime;
            HIDDebug("EksOtus.deckSwitch - ignoring bounced press");
            return;
        }
        EksOtus.deckSwitchDebounceIgnoring = false;
        // Start the long-press timer. If it fires while the button is
        // still held, EksOtus.deckSwitchHoldTrigger() will temporarily
        // switch decks. deckSwitchHoldPending is an explicit guard flag,
        // checked inside the timer callback itself - engine.stopTimer()
        // alone was not reliably cancelling this timer, so a released tap
        // was cleared from controller.timers but the timer fired anyway
        // ~deckSwitchHoldTime later and reverted the deck, silently
        // cancelling the tap's switch. The flag makes a late/duplicate
        // firing a guaranteed no-op regardless of whether stopTimer worked.
        EksOtus.deckSwitchHoldPending = true;
        controller.timers["deck_switch_hold"] = engine.beginTimer(
            EksOtus.deckSwitchHoldTime, EksOtus.deckSwitchHoldTrigger, true
        );
        return;
    }

    // field.value == controller.buttonStates.released
    if (EksOtus.deckSwitchDebounceIgnoring) {
        // Release half of a bounced press we already ignored above.
        EksOtus.deckSwitchDebounceIgnoring = false;
        return;
    }

    if (EksOtus.deckSwitchHoldPending) {
        // Released before the hold threshold - this was a quick tap,
        // switch decks persistently. Clear the pending flag FIRST so that
        // even if the hold timer still fires later (stopTimer unreliable),
        // EksOtus.deckSwitchHoldTrigger() will see it is no longer pending
        // and do nothing.
        EksOtus.deckSwitchHoldPending = false;
        if (controller.timers["deck_switch_hold"] != undefined) {
            engine.stopTimer(controller.timers["deck_switch_hold"]);
            delete controller.timers["deck_switch_hold"];
        }
        EksOtus.deckSwitchPersistent();
        EksOtus.deckSwitchDebounceUntil = Date.now() + EksOtus.deckSwitchDebounceTime;
        return;
    }

    if (EksOtus.deckSwitchHeld) {
        // The long press already triggered a temporary deck switch -
        // releasing the button reverts back to the original deck.
        EksOtus.deckSwitchHeld = false;
        controller.switchDeck();
        controller.setOutput("hid","deck_switch", controller.LEDColors[controller.deckOutputColors[controller.activeDeck]]);
        EksOtus.updateLEDs();
        EksOtus.deckSwitchDebounceUntil = Date.now() + EksOtus.deckSwitchDebounceTime;
        HIDDebug("Active EKS Otus deck reverted to " + controller.activeDeck);
    }
}

// Timer callback fired when 'deck_switch' has been held down continuously
// for EksOtus.deckSwitchHoldTime - temporarily switches deck controls
// until the button is released.
EksOtus.deckSwitchHoldTrigger = function() {
    var controller = EksOtus.controller;
    delete controller.timers["deck_switch_hold"];
    if (!EksOtus.deckSwitchHoldPending) {
        // The button was already released (a quick tap) and handled -
        // this firing is stale/late, ignore it.
        return;
    }
    EksOtus.deckSwitchHoldPending = false;
    EksOtus.deckSwitchHeld = true;
    controller.switchDeck();
    controller.setOutput("hid","deck_switch", controller.LEDColors[controller.deckOutputColors[controller.activeDeck]]);
    EksOtus.updateLEDs();
    HIDDebug("Active EKS Otus deck temporarily switched to " + controller.activeDeck);
}

// Function to handle a quick tap of 'deck_switch' - switches decks
// persistently (stays switched until tapped or held again).
EksOtus.deckSwitchPersistent = function() {
    var controller = EksOtus.controller;
    controller.switchDeck();
    controller.setOutput("hid","deck_switch", controller.LEDColors[controller.deckOutputColors[controller.activeDeck]]);
    EksOtus.updateLEDs();
    HIDDebug("Active EKS Otus deck now " + controller.activeDeck);
}

// Switch the visual LED feedback on platter LEDs to active deck
// NOTE this is now disabled, it causes HID input errors in firmware!
EksOtus.activateSpinningPlatterLEDs = function() {
    var controller = EksOtus.controller;
    var active_group = controller.resolveDeckGroup(controller.activeDeck);
    if (active_group==undefined)
        return;
    if (!(controller.activeDeck in controller.deckOutputColors)) {
        HIDDebug("LED color not mapped to deck " % controller.activeDeck);
        return;
    }
    if (active_group==undefined) {
        EksOtus.disableSpinningPlatterLEDs();
        return;
    }
    if (EksOtus.activeSpinningPlatterGroup !=undefined) {
        EksOtus.disableSpinningPlatterLEDs();
    }
    EksOtus.activeSpinningPlatterGroup = active_group;
    EksOtus.loadedTrackDuration(engine.getValue(active_group,"duration"));
    EksOtus.enableSpinningPlatterLEDs();
}

// Enable spinning platter LED functionality for active virtual deck
EksOtus.enableSpinningPlatterLEDs = function() {
    if (EksOtus.activeSpinningPlatterGroup==undefined)
        return;
    engine.makeConnection(
        EksOtus.activeSpinningPlatterGroup,
        "playposition",
        "EksOtus.circleLEDs"
    );
    engine.makeConnection(
        EksOtus.activeSpinningPlatterGroup,
        "duration",
        "EksOtus.loadedTrackDuration"
    )
    EksOtus.resetWheelLEDs("off",false);
}

// Disable spinning platter LED functionality for active virtual deck
EksOtus.disableSpinningPlatterLEDs = function() {
    if (EksOtus.activeSpinningPlatterGroup==undefined)
        return;
    engine.makeConnection(
        EksOtus.activeSpinningPlatterGroup,
        "playposition",
        "EksOtus.circleLEDs",
        true
    );
    engine.makeConnection(
        EksOtus.activeSpinningPlatterGroup,
        "duration",
        "EksOtus.loadedTrackDuration",
        true
    )
    EksOtus.resetWheelLEDs("off",false);
}

// Callback from engine to set every third LED in circling pattern according to
// the track position. Careful not to enable sending all 60 positions, it may
// cause too much HID traffic!
EksOtus.circleLEDs = function(position) {
    var controller = EksOtus.controller;
    if (position<0 || position>1) {
        EksOtus.resetWheelLEDs("off",false);
        return;
    }
    // Only update every third LED to save HID packet bandwidth
    var wheelLEDSplit = 3;
    var wheelLEDGroups = EksOtus.wheelLEDCount/wheelLEDSplit;
    var timeRemaining = ((1-position)*EksOtus.activeTrackDuration) | 0;
    var track_pos = position * EksOtus.activeTrackDuration;
    var revolutions = track_pos / EksOtus.revTime;
    var led_index = (((revolutions-(revolutions|0))*wheelLEDGroups)|0)*wheelLEDSplit;
    led_index++;
    if (led_index==EksOtus.activeSpinningPlatterLED)
        return;
    EksOtus.activeSpinningPlatterLED = led_index;
    EksOtus.resetWheelLEDs("off",false);
    // setOutput() needs the numeric LED value, not the color name string
    var led_color = controller.LEDColors[controller.deckOutputColors[controller.activeDeck]];
    controller.setOutput("hid","wheel_"+(led_index),led_color);
    EksOtus.updateLEDs();
}

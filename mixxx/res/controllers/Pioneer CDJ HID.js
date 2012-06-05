//
// Pioneer CDJ cd player HID script v0.01
// Copyright (C) 2012, Ilkka Tuohela, Sean M. Pappalardo
// but feel free to tweak this to your heart's content!
// For Mixxx version 1.11.x
//

function PioneerCDJController() {
    this.controller = new HIDController();

    // Valid values: 1 for mouse mode, 0 for xy-pad mode
    this.version_major = undefined;
    this.version_minor = undefined;

    this.ButtonLEDCount = 22;
    this.LEDColors = { off: 0x0, on: 0x01 };

    this.registerInputPackets = function() {
        var packet = undefined;
        var name = undefined;
        var offset = 0;

        // Register input controls with actual deck name, link in 
        // mapping using this.
        packet = new HIDPacket("control",[],20); // ,this.dump);
        packet.addControl("hid","eject",0,"B",0x1);
        packet.addControl("hid","previous_track",0,"B",0x4);
        packet.addControl("hid","next_track",0,"B",0x8);
        packet.addControl("hid","seek_back",0,"B",0x10);
        packet.addControl("hid","seek_forward",0,"B",0x20);
        packet.addControl("hid","cue",0,"B",0x40);
        packet.addControl("hid","play",0,"B",0x80);
        packet.addControl("hid","reloop_exit",1,"B",0x20);
        packet.addControl("hid","cue_out",1,"B",0x40);
        packet.addControl("hid","cue_in",1,"B",0x80);
        packet.addControl("hid","time_mode",2,"B",0x8);
        packet.addControl("hid","cue_delete",2,"B",0x10);
        packet.addControl("hid","cue_previous",2,"B",0x20);
        packet.addControl("hid","cue_next",2,"B",0x40);
        packet.addControl("hid","cue_memory",2,"B",0x80);
        packet.addControl("hid","jog_mode",3,"B",0x2);
        packet.addControl("hid","master_tempo",3,"B",0x8);
        packet.addControl("hid","tempo_range",3,"B",0x10);
        packet.addControl("hid","browse_press",3,"B",0x20);
        packet.addControl("hid","menu_back",3,"B",0x40);
        packet.addControl("hid","beat_select",4,"B",0x8);
        packet.addControl("hid","beatloop_16",4,"B",0x10);
        packet.addControl("hid","beatloop_8",4,"B",0x20);
        packet.addControl("hid","beatloop_4",4,"B",0x40);
        packet.addControl("hid","beatloop_2",4,"B",0x80);
        packet.addControl("hid","tag_track",6,"B",0x1);
        packet.addControl("hid","reverse",7,"B",0x1);
        packet.addControl("hid","jog_touch",7,"B",0x20);
        packet.addControl("hid","jog_direction",7,"B",0x40);
        packet.addControl("hid","jog_move",7,"B",0x80);
        packet.addControl("hid","vinyl_speed_knob",8,"B");
        packet.addControl("hid","browse_knob",10,"H",undefined,true);
        packet.addControl("hid","pitch_slider",12,"h");
        packet.addControl("hid","jog_wheel",14,"h",undefined,true);
        packet.addControl("hid","jog_ticks",16,"h");
        packet.addControl("hid","needle_search",18,"h");
        this.controller.registerInputPacket(packet);

    }

    // Register output packets we send to the controller
    this.registerOutputPackets = function() {
        var packet = undefined;
        var name = undefined;
        var offset = 0;

        // Control packet to initialize HID mode on CDJ. 
        // TODO - Sean: This is arbitrary example packet, fix the 
        // bytes to get it working. Need to add a response packet
        // to input packets as well, if we receive acknowledgement
        packet = new HIDPacket("request_hid_mode",[0x1],0x20);
        packet.addControl("hid","mode",0,"B",1);
        this.controller.registerOutputPacket(packet);

        // Control packet for button LEDs
        // TODO - Sean: this is just example, fill in correct packet
        // size, header bytes control field offesets but bits to make
        // it work.
        packet = new HIDPacket("button_leds",[],20);
        packet.addLED("hid","play",0,"B",6);
        packet.addLED("hid","cue",0,"B",6);
        packet.addLED("hid","previous_track",0,"B",2);
        packet.addLED("hid","seek_back",0,"B",3);
        packet.addLED("hid","seek_forward",0,"B",3);
        packet.addLED("hid","reloop_exit",0,"B",3);
        packet.addLED("hid","cue_out",0,"B",3);
        packet.addLED("hid","cue_in",0,"B",3);
        packet.addLED("hid","time_mode",0,"B",3);
        packet.addLED("hid","cue_delete",0,"B",3);
        packet.addLED("hid","cue_previous",0,"B",3);
        packet.addLED("hid","cue_next",0,"B",3);
        packet.addLED("hid","cue_memory",0,"B",3);
        packet.addLED("hid","jog_mode",0,"B",3);
        packet.addLED("hid","master_tempo",0,"B",3);
        packet.addLED("hid","tempo_range",0,"B",3);
        packet.addLED("hid","beat_select",0,"B",3);
        packet.addLED("hid","beat_16",0,"B",3);
        packet.addLED("hid","beat_8",0,"B",3);
        packet.addLED("hid","beat_4",0,"B",3);
        packet.addLED("hid","beat_2",0,"B",3);
        this.controller.registerOutputPacket(packet);

        // Control packet for screen text control
        // TODO - Sean: this is just a silly example how to register
        // another packet, this must be handled completely separately
        // anyway when implemented. When I know what data is sent, I 
        // will add a helper function to actually do something with it!
        var textlines = 1; 
        var chars = 1; 
        var offset = 2;
        // Register 2 bytes for each letter, I expect UTF-8 output
        packet = new HIDPacket("display",[0x2,0x2],2+textlines*chars*2);
        for (var i=0;i<textlines;i++) {
            for (var c=0;c<chars;c++) {
                var name = "line_"+(i+1)+"letter_"+(c+1);
                packet.addControl("hid",name,offset++,"H");
            }
        }
        this.controller.registerOutputPacket(packet);

        // Control packet for waveform display
        // TODO - Sean: this is just a silly example how to register
        // arbitrary byte packet, this must be handled completely separately
        // anyway when implemented. When I know what data is sent, I 
        // will add a helper function to actually do something with it!
        var waveform_packet_datalen = 400;
        packet = new HIDPacket("waveform",[0x1,0x2],2+waveform_packet_datalen);;
        for (var i=0;i<waveform_packet_datalen;i++)
            packet.addControl("hid","byte_"+i,i,"B");
        this.controller.registerOutputPacket(packet);

    }

    // Test callback for control packet (enabled above if needed) to dump 
    // input field name and value to log and discard any controls
    this.dump = function(packet,delta) {
        for (field_name in delta) {
            var field = delta[field_name];
            script.HIDDebug("CDJ" + field.id + " VALUE " + field.value);
        }
    }

    // Jog wheel seek event scaler
    this.jogScaler = function(group,name,value) { return value/12; }

    // Jog wheel scratch event (ticks) scaler
    this.jogPositionDelta = function(group,name,value) { return value/3; }

    // Pitch on CDJ sends -1000 to 1000, reset at 0, swap direction
    this.pitchScaler = function(gruop,name,value) { return -(value/1000); }

    // Volume slider scaling for 0..1..5 scaling from 0-255
    this.volumeScaler = function(group,name,value) {
        return script.absoluteNonLin(value, 0, 1, 5, 0, 256);
    }

    // Generic unsigned short to -1..0..1 range scaling
    this.plusMinus1Scaler = function(group,name,value) {
        if (value<32768)
            return value/32768-1;
        else
            return (value-32768)/32768;
    }

    // Generic wrapper call to initialize this functions methods
    this.initializeHIDController = function() {
        this.registerInputPackets();
        this.registerOutputPackets();
        var packet = this.controller.OutputPackets["request_hid_mode"];
        if (packet!=undefined) {
            script.HIDDebug("Sending HID mode init packet");
            packet.send();
        }
    }

}

PioneerCDJHID = new PioneerCDJController();

// Map beatloop sizes to actual mixxx values
PioneerCDJHID.beatLoopSizeMap = {
    beatloop_2: "8", beatloop_2_shift: "0.5",
    beatloop_4: "4", beatloop_4_shift: "0.25",
    beatloop_8: "2", beatloop_8_shift: "0.125",
    beatloop_16: "1", beatloop_16_shift: "0.0625"
}

// Timers and toggles
PioneerCDJHID.deckSwitchClicked = false;
PioneerCDJHID.deckSwitchClickTimer = undefined;

// Initialize device state
PioneerCDJHID.init = function(id) {
    PioneerCDJHID.id = id;

    // Call the HID packet parser initializers
    PioneerCDJHID.initializeHIDController();
    // Link controls and register callbacks
    PioneerCDJHID.registerCallbacks();
    var controller = PioneerCDJHID.controller;

    // By default we control first deck. See deckSwitch for swapping decks
    controller.activeDeck = 1;
    // Enable to create a timer to update LEDs (for blinking)
    controller.LEDUpdateInterval = undefined;

    // Scratch parameters
    controller.scratchintervalsPerRev = 2048;
    controller.scratchAlpha = 1.0/8;
    controller.rampedScratchEnable = true;
    controller.rampedScratchEnable = true;
    controller.toggleButtons = [ "play", "keylock", "pfl" ];

    // Declare some fields as soft takeover mode
    engine.softTakeover("[Channel1]","pregain",true);
    engine.softTakeover("[Channel2]","pregain",true);

    if (this.LEDUpdateInterval!=undefined) {
        this.LEDTimer = engine.beginTimer(
            this.LEDUpdateInterval,
            "PioneerCDJHID.updateLEDs(true)"
        );
    }

    script.HIDDebug("Pioneer CDJ Deck "+PioneerCDJHID.id+" initialized");
}

// Device cleanup function
PioneerCDJHID.shutdown = function() {
    if (PioneerCDJHID.LEDTimer!=undefined) {
        engine.stopTimer(PioneerCDJHID.LEDTimer);
        PioneerCDJHID.LEDTimer = undefined;
    }
}

// Mandatory default handler for incoming packets
PioneerCDJHID.incomingData = function(data,length) {
    PioneerCDJHID.controller.parsePacket(data,length);
}

// Callback wrapper to update active LEDs on group change
PioneerCDJHID.activeLEDUpdateWrapper = function() {
    PioneerCDJHID.controller.updateActiveDeckLEDs();
}

// Link virtual HID naming of input and LED controls to mixxx
// Note: HID specification has more fields than we map here. 
PioneerCDJHID.registerCallbacks = function() {
    var controller = PioneerCDJHID.controller;

    script.HIDDebug("Registring controls and callbacks");

    // Play/cue/reverse buttons
    controller.linkControl("hid","play","deck","play");
    controller.linkControl("hid","cue","deck","cue_default");
    controller.linkControl("hid","reverse","deck","reverse");

    // Seek buttons top of play/cue
    controller.linkControl("hid","seek_back","deck","back");
    controller.linkControl("hid","seek_forward","deck","fwd");
    controller.linkControl("hid","previous_track","[Playlist]","SelectPrevTrack");
    controller.linkControl("hid","next_track","[Playlist]","SelectNextTrack");

    // Knob to browse tracks right of screen
    controller.linkControl("hid","browse_press","deck","LoadSelectedTrack");
    controller.linkControl("hid","browse_knob","[Playlist]","SelectTrackKnob");

    // Pitch slider is range -1000..0..1000
    controller.linkControl("hid","pitch_slider","deck","rate");
    controller.addScaler("rate",PioneerCDJHID.pitchScaler);

    // Loop in/out buttons, loop exit button
    controller.linkControl("hid","cue_in","deck","loop_in");
    controller.linkControl("hid","cue_out","deck","loop_out");
    controller.linkControl("hid","reloop_exit","deck","reloop_exit");

    // Handle beatloop buttons with modifier + callback
    controller.linkModifier("hid","beat_select","beatloop_size");
    controller.addCallback("control","hid","beatloop_16",PioneerCDJHID.beatloop);
    controller.addCallback("control","hid","beatloop_8",PioneerCDJHID.beatloop);
    controller.addCallback("control","hid","beatloop_4",PioneerCDJHID.beatloop);
    controller.addCallback("control","hid","beatloop_2",PioneerCDJHID.beatloop);

    // Link normal jog touch and delta to HIDController default jog 
    // and scratch functions. CDJ reports more than these fields from job, 
    // we ignore the other input fields for jog control. Feel free to adopt.
    controller.linkControl("hid","jog_touch","deck","jog_touch");
    controller.linkControl("hid","jog_wheel","deck","jog_wheel");

    // Standard HIDController scalers for jog functionality. 
    // Not related to field specifications names above!
    controller.addScaler("jog",PioneerCDJHID.jogScaler);
    controller.addScaler("jog_scratch",PioneerCDJHID.jogPositionDelta);

    // TODO Fix these if we want to actually follow CDJ controls strictly.
    // Misuse some buttons for something more useful in mixxx
    controller.linkControl("hid","cue_previous","deck","loop_halve");
    controller.linkControl("hid","cue_next","deck","loop_double");
    controller.linkControl("hid","cue_memory","deck","quantize");
    controller.linkControl("hid","master_tempo","deck","beatsync");
    controller.linkControl("hid","tempo_range","deck","beats_translate_curpos");
    controller.linkControl("hid","jog_mode","deck","pfl");

    // Use 'eject' button for deck switching
    controller.addCallback("control","hid","eject",PioneerCDJHID.deckSwitch);

    // Use vinyl speed adjustment for pregain
    controller.linkControl("hid","vinyl_speed_knob","deck","pregain");
    controller.addScaler("pregain",PioneerCDJHID.pregainScaler);
    // Unused, useful buttons to map - MAP to something!
    // controller.linkControl("hid","time_mode","deck","");
    // controller.linkControl("hid","menu_back","deck","");
    // controller.linkControl("hid","tag_track","deck","");

    script.HIDDebug("Registering controls and callbacks finished");
}

// Hotcues activated with normal press, cleared with shift
PioneerCDJHID.beatloop = function (field) {
    var controller = PioneerCDJHID.controller;
    var command;
    if (field.value==controller.buttonStates.released)
        return;
    if (controller.activeDeck==undefined)
        return;

    var active_group = controller.resolveGroup('deck');
    if (controller.modifiers.get("beatloop_size")) {
        size = PioneerCDJHID.beatLoopSizeMap[field.name+"_shift"] ;
    } else {
        size = PioneerCDJHID.beatLoopSizeMap[field.name];
    }
    var command = "beatloop_" + size + "_toggle";
    engine.setValue(active_group,command,true);
}

// Use pregain if modifier shift is active, volume otherwise
// NOTE: right now no shift button is registered so this is always
// pregain
PioneerCDJHID.pregain = function (field) {
    var controller = PioneerCDJHID.controller;
    if (controller.activeDeck==undefined)
        return;
    var active_group = controller.resolveGroup(field.group);
    var value;
    if (controller.modifiers.get("shift")) {
        value = script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536);
        engine.setValue(active_group,"pregain",value);
    } else {
        value = field.value / 65536;
        engine.setValue(active_group,"volume",value);
    }
}

// Function called when the special 'Deck Switch' button is pressed
// TODO - add code for 'hold deck_switch and press hot_cue[1-4]
// to select deck 1-4
PioneerCDJHID.deckSwitch = function(field) {
    var controller = PioneerCDJHID.controller;
    if (field.value == controller.buttonStates.released) {
        if (PioneerCDJHID.deckSwitchClicked==false) {
            PioneerCDJHID.deckSwitchClicked=true;
            PioneerCDJHID.deckSwitchClickTimer = engine.beginTimer(
                250,"PioneerCDJHID.deckSwitchClickedClear()"
            );
        } else {
            PioneerCDJHID.deckSwitchDoubleClick();
        }
    }
}

// Timer to clear the double click status for deck switch
PioneerCDJHID.deckSwitchClickedClear = function() {
    PioneerCDJHID.deckSwitchClicked = false;
    if (PioneerCDJHID.deckSwitchClickTimer!=undefined) {
        engine.stopTimer(PioneerCDJHID.deckSwitchClickTimer);
        PioneerCDJHID.deckSwitchClickTimer = undefined;
    }
}

// Function to handle case when 'deck_switch' button was double clicked
PioneerCDJHID.deckSwitchDoubleClick = function() {
    var controller = PioneerCDJHID.controller;
    PioneerCDJHID.deckSwitchClicked = false;
    if (PioneerCDJHID.deckSwitchClickTimer!=undefined) {
        engine.stopTimer(PioneerCDJHID.deckSwitchClickTimer);
        PioneerCDJHID.deckSwitchClickTimer = undefined;
    }
    if (PioneerCDJHID.activeDeck!=undefined)
        controller.disconnectDeckLEDs();
    switch (controller.activeDeck) {
        case 1:
            controller.activeDeck = 2;
            controller.setLED('hid','deck_switch','green');
            break;
        case 2:
            controller.activeDeck = 1;
            controller.setLED('hid','deck_switch','red');
            break;
        case 3:
            controller.activeDeck = 4;
            controller.setLED('hid','deck_switch','green');
            break;
        case 4:
            controller.activeDeck = 3;
            controller.setLED('hid','deck_switch','red');
            break;
        case undefined:
            controller.activeDeck = 1;
            controller.setLED('hid','deck_switch','red');
            break;
    }
    controller.connectDeckLEDs();
    controller.updateActiveDeckLEDs();
    script.HIDDebug('Active CDJ deck now ' + controller.activeDeck);
}


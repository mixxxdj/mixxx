//
// Pioneer CDJ cd player HID script v0.9
// Copyright (C) 2012, Ilkka Tuohela, Sean M. Pappalardo
// but feel free to tweak this to your heart's content!
// For Mixxx version 1.11.x
//

function PioneerCDJController() {
    this.controller = new HIDController();

    // Valid values: 1 for mouse mode, 0 for xy-pad mode
    this.version_major = undefined;
    this.version_minor = undefined;
    this.remain_mode = true;

    this.controller.LEDColors = {on:1, off:0};
    this.ButtonOutputCount = 22;

    this.registerInputPackets = function() {
        var packet = undefined;
        var name = undefined;
        var offset = 0;

        packet = new HIDPacket("control",[],20);
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
        //packet.addControl("hid","jog_direction",7,"B",0x40);
        //packet.addControl("hid","jog_move",7,"B",0x80);
        packet.addControl("hid","vinyl_speed_knob",8,"B");
        packet.addControl("hid","browse_knob",10,"H",undefined,true);
        packet.addControl("hid","pitch_slider",12,"h");
        packet.addControl("hid","jog_wheel",14,"h",undefined,true);
        //packet.addControl("hid","jog_ticks",16,"h");
        packet.addControl("hid","needle_search",18,"h");
        this.controller.registerInputPacket(packet);

    }

    // Register output packets we send to the controller
    this.registerOutputPackets = function() {
        var packet = undefined;
        var name = undefined;
        var offset = 0;

        // Control packet for button Outputs
        // TODO - Sean: this is just example, fill in correct packet
        // size, header bytes control field offesets but bits to make
        // it work.
        packet = new HIDPacket("button_leds", [0x1],36);
        packet.addOutput("hid","screen_acue",4,"B",0x1);
        packet.addOutput("hid","remain",4,"B",0x2);
        packet.addOutput("hid","reloop_exit",4,"B",0x8);
        packet.addOutput("hid","cue_out",4,"B",0x10);
        packet.addOutput("hid","cue_in",4,"B",0x20);
        packet.addOutput("hid","cue",4,"B",0x40);
        packet.addOutput("hid","play",4,"B",0x80);
        packet.addOutput("hid","screen_memory",5,"B",0x1);
        packet.addOutput("hid","screen_wide",5,"B",0x8);
        packet.addOutput("hid","screen_16_percent",5,"B",0x10);
        packet.addOutput("hid","screen_10_percent",5,"B",0x20);
        packet.addOutput("hid","screen_6_percent",5,"B",0x40);
        packet.addOutput("hid","master_tempo",5,"B",0x80);
        packet.addOutput("hid","beatloop_16",6,"B",0x1);
        packet.addOutput("hid","beatloop_8",6,"B",0x2);
        packet.addOutput("hid","beatloop_4",6,"B",0x4);
        packet.addOutput("hid","beatloop_2",6,"B",0x8);
        packet.addOutput("hid","reverse",6,"B",0x10);
        packet.addOutput("hid","jog_dashed_circle",6,"B",0x20);
        packet.addOutput("hid","jog_vinyl_logo",6,"B",0x40);
        packet.addOutput("hid","browse",8,"B",0x1);
        packet.addOutput("hid","info",8,"B",0x4);
        packet.addOutput("hid","menu",8,"B",0x40);
        packet.addOutput("hid","taglist",8,"B",0x80);
        packet.addOutput("hid","set_time_mode",11,"B");
        packet.addOutput("hid","time_minutes",12,"B");
        packet.addOutput("hid","time_seconds",13,"B");
        packet.addOutput("hid","time_frames",14,"B");
        packet.addOutput("hid","tracknumber",18,"B");
        packet.addOutput("hid","bpm",20,"H");
        packet.addOutput("hid","rate",22,"H");
        this.controller.registerOutputPacket(packet);

        // Control packet to initialize HID mode on CDJ. 
        // TODO - Sean: This is arbitrary example packet, fix the 
        // bytes to get it working. Need to add a response packet
        // to input packets as well, if we receive acknowledgement
        packet = new HIDPacket("request_hid_mode",[0x1],0x20);
        packet.addControl("hid","mode",0,"B",1);
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
        //this.controller.registerOutputPacket(packet);

        // Control packet for waveform display
        // TODO - Sean: this is just a silly example how to register
        // arbitrary byte packet, this must be handled completely separately
        // anyway when implemented. When I know what data is sent, I 
        // will add a helper function to actually do something with it!
        var waveform_packet_datalen = 400;
        packet = new HIDPacket("waveform",[0x1,0x2],2+waveform_packet_datalen);;
        for (var i=0;i<waveform_packet_datalen;i++)
            packet.addControl("hid","byte_"+i,i,"B");
        //this.controller.registerOutputPacket(packet);

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
        var controller = this.controller;
        this.registerInputPackets();
        this.registerOutputPackets();
        var packet = controller.OutputPackets["request_hid_mode"];
        if (packet!=undefined) {
            HIDDebug("Sending HID mode init packet");
            packet.send();
        }

        // Set this value permanently to 0xc0
        controller.setOutput("hid","set_time_mode",0xc0);
    }

}

PioneerCDJHID = new PioneerCDJController();


// Initialize device state
PioneerCDJHID.init = function(id) {
    PioneerCDJHID.id = id;

    var controller = PioneerCDJHID.controller;
    
    // Map beatloop sizes to actual mixxx values
    PioneerCDJHID.beatLoopSizeMap = {
        beatloop_2: "16", beatloop_2_shift: "1",
        beatloop_4: "8", beatloop_4_shift: "0.5",
        beatloop_8: "4", beatloop_8_shift: "0.25",
        beatloop_16: "2", beatloop_16_shift: "0.125"
    }

    controller.activeDeck = 1;

    // Call the HID packet parser initializers
    PioneerCDJHID.initializeHIDController();
    // Link controls and register callbacks
    PioneerCDJHID.registerCallbacks();
   
    // Scratch parameters
    controller.scratchintervalsPerRev = 2048;
    controller.scratchAlpha = 1.0/8;
    controller.rampedScratchEnable = true;
    controller.rampedScratchEnable = true;
    controller.toggleButtons = [ "play", "quantize", "keylock", "pfl" ];

    // Declare some fields as soft takeover mode
    engine.softTakeover("[Channel1]","pregain",true);
    engine.softTakeover("[Channel2]","pregain",true);
   
    // Set initial deck number
    controller.setOutput("hid","tracknumber",controller.activeDeck);
    PioneerCDJHID.connectDeckControls();

    controller.getOutputPacket("button_leds").send();
    HIDDebug("Pioneer CDJ Deck "+PioneerCDJHID.id+" initialized");
}

// Device cleanup function
PioneerCDJHID.shutdown = function() {
}

// Mandatory default handler for incoming packets
PioneerCDJHID.incomingData = function(data,length) {
    PioneerCDJHID.controller.parsePacket(data,length);
}

PioneerCDJHID.disconnectDeckControls = function() {
    var controller = PioneerCDJHID.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    if (!engine.connectControl(group,"duration",PioneerCDJHID.updateTrackLength,true))
        HIDDebug("Error disconnecting duration from group " + group);
    if (!engine.connectControl(group,"playposition",PioneerCDJHID.updateTime,true))
        HIDDebug("Error disconnecting playposition from group " + group);
}

PioneerCDJHID.connectDeckControls = function() {
    var controller = PioneerCDJHID.controller;
    engine.connectControl("[Channel1]","duration",PioneerCDJHID.updateTrackLength);
    engine.connectControl("[Channel2]","duration",PioneerCDJHID.updateTrackLength);
    engine.connectControl("[Channel1]","playposition",PioneerCDJHID.updateTime);
    engine.connectControl("[Channel2]","playposition",PioneerCDJHID.updateTime);

    var group = controller.resolveDeckGroup(controller.activeDeck);
    PioneerCDJHID.updateTrackLength(engine.getValue(group,"duration"));

    if (PioneerCDJHID.remain_mode) 
        PioneerCDJHID.controller.setOutput("hid","remain",1);
    else
        PioneerCDJHID.controller.setOutput("hid","remain",0);
    PioneerCDJHID.updateBPM();
    //PioneerCDJHID.updateRate();
    PioneerCDJHID.updateTime();

}
 
// Link virtual HID naming of input and Output controls to mixxx
// Note: HID specification has more fields than we map here. 
PioneerCDJHID.registerCallbacks = function() {
    var controller = PioneerCDJHID.controller;

    HIDDebug("Registring controls and callbacks");

    // Play/cue/reverse buttons
    controller.linkControl("hid","play","deck","play");
    controller.linkControl("hid","cue","deck","cue_default");
    controller.linkControl("hid","reverse","deck","reverse");
    controller.linkControl("hid","reloop_exit","deck","reloop_exit");
    controller.linkControl("hid","reloop_exit","deck","reloop_exit");

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
    controller.setScaler("rate",PioneerCDJHID.pitchScaler);

    // Loop in/out buttons, loop exit button
    controller.linkControl("hid","cue_in","deck","loop_in");
    controller.linkControl("hid","cue_out","deck","loop_out");
    controller.setCallback("control","hid","reloop_exit",PioneerCDJHID.reloop_exit);

    // Handle beatloop buttons with modifier + callback
    controller.linkModifier("hid","beat_select","beatloop_size");
    controller.setCallback("control","hid","beatloop_16",PioneerCDJHID.beatloop);
    controller.setCallback("control","hid","beatloop_8",PioneerCDJHID.beatloop);
    controller.setCallback("control","hid","beatloop_4",PioneerCDJHID.beatloop);
    controller.setCallback("control","hid","beatloop_2",PioneerCDJHID.beatloop);
    controller.setCallback("control","hid","time_mode",PioneerCDJHID.toggleTimeDisplayMode);

    // Link normal jog touch and delta to HIDController default jog 
    // and scratch functions. CDJ reports more than these fields from job, 
    // we ignore the other input fields for jog control. Feel free to adopt.
    controller.linkControl("hid","jog_touch","deck","jog_touch");
    controller.linkControl("hid","jog_wheel","deck","jog_wheel");

    // Standard HIDController scalers for jog functionality. 
    // Not related to field specifications names above!
    controller.setScaler("jog",PioneerCDJHID.jogScaler);
    controller.setScaler("jog_scratch",PioneerCDJHID.jogPositionDelta);

    // TODO Fix these if we want to actually follow CDJ controls strictly.
    // Misuse some buttons for something more useful in mixxx
    controller.linkControl("hid","cue_previous","deck","loop_halve");
    controller.linkControl("hid","cue_next","deck","loop_double");
    controller.linkControl("hid","master_tempo","deck","beatsync");
    controller.linkControl("hid","tempo_range","deck","beats_translate_curpos");
    controller.linkControl("hid","jog_mode","deck","pfl");

    // Use 'eject' button for deck switching
    controller.setCallback("control","hid","eject",PioneerCDJHID.switchDeck);

    // Use vinyl speed adjustment for pregain
    controller.linkControl("hid","vinyl_speed_knob","deck","pregain");
    controller.setScaler("pregain",PioneerCDJHID.pregainScaler);

    // Unused, useful buttons to map - MAP to something!
    // controller.linkControl("hid","menu_back","deck","");
    // controller.linkControl("hid","tag_track","deck","");

    controller.linkOutput("hid","play","deck","play",PioneerCDJHID.updateOutput);
    controller.linkOutput("hid","cue","deck","cue_default",PioneerCDJHID.updateOutput);
    controller.linkOutput("hid","reverse","deck","reverse",PioneerCDJHID.updateOutput);
    controller.linkOutput("hid","cue_in","deck","loop_in",PioneerCDJHID.updateOutput);
    controller.linkOutput("hid","cue_out","deck","loop_out",PioneerCDJHID.updateOutput);
    controller.linkOutput("hid","bpm","deck","bpm",PioneerCDJHID.updateBPM);
    //controller.linkOutput("hid","rate","deck","rate",PioneerCDJHID.updateRate);

    HIDDebug("Registering controls and callbacks finished");
}

PioneerCDJHID.updateTrackLength = function(value,group,key) {
    PioneerCDJHID.track_length = value;
    // This should be triggered automatically, but since it doesn't ...
    PioneerCDJHID.updateBPM();
    //PioneerCDJHID.updateRate();
}

PioneerCDJHID.toggleTimeDisplayMode = function(field) {
    var controller = PioneerCDJHID.controller;
    if (field.value==controller.buttonStates.released)
        return;
    PioneerCDJHID.remain_mode = (PioneerCDJHID.remain_mode!=true) ? true : false;
    if (PioneerCDJHID.remain_mode) {
        PioneerCDJHID.controller.setOutput("hid","remain",1);
    } else {
        PioneerCDJHID.controller.setOutput("hid","remain",0);
    }
}

PioneerCDJHID.updateTime = function(value,group,key) {
    var controller = PioneerCDJHID.controller;
    if (controller.resolveDeck(group)!=controller.activeDeck)
        return;
    var position; 
    var output_packet = controller.getOutputPacket("button_leds");
    if (PioneerCDJHID.remain_mode==true) {
        // This is some weird reverse engineered bit magic: don't ask
        if (value==undefined || value==1) {
            controller.setOutput("hid","time_minutes",0x76);
            controller.setOutput("hid","time_seconds",0x8d);
            controller.setOutput("hid","time_frames",0x79);
        } else {
            position = PioneerCDJHID.track_length - PioneerCDJHID.track_length*value;
            controller.setOutput("hid","time_minutes",0x77-(position/60));
            controller.setOutput("hid","time_seconds",0x8d-(position%60));
            controller.setOutput("hid","time_frames",
                0x79-Math.floor((position-Math.floor(position))*75) 
            );
        }
    } else {
        if (value==undefined || value<=0) {
            controller.setOutput("hid","time_minutes",0);
            controller.setOutput("hid","time_seconds",0);
            controller.setOutput("hid","time_frames",0);
        } else {
            position = PioneerCDJHID.track_length*value;
            controller.setOutput("hid","time_minutes",position/60);
            controller.setOutput("hid","time_seconds",position%60);
            controller.setOutput("hid","time_frames",
                Math.floor((position-Math.floor(position))*75) 
            );
        }
    }
    output_packet.send();
}

PioneerCDJHID.updateBPM = function() {
    var controller = PioneerCDJHID.controller;
    if (controller.activeDeck==undefined)
        return;
    var value = engine.getValue(
        controller.resolveDeckGroup(controller.activeDeck),
        "bpm"
    );
    HIDDebug("DECK " + controller.activeDeck + " BPM " + Math.floor(value)); 
    PioneerCDJHID.controller.setOutput("deck","bpm",Math.floor(value));
}

PioneerCDJHID.updateRate = function() {
    var controller = PioneerCDJHID.controller;
    if (controller.activeDeck==undefined)
        return;
    var value = engine.getValue(
        controller.resolveDeckGroup(controller.activeDeck),
        "rate"
    );
    HIDDebug("DECK " + controller.activeDeck + " RATE " + Math.floor(value)); 
    PioneerCDJHID.controller.setOutput("deck","rate",Math.floor(value));
}

// Callback to update Outputs from engine.connectControl
PioneerCDJHID.updateOutput = function(value,group,key) {
    var controller = PioneerCDJHID.controller;
    if (controller.activeDeck==undefined)
        return;
    if (value==1) 
        PioneerCDJHID.controller.setOutput("deck",key,controller.LEDColors.on,true);
    else
        PioneerCDJHID.controller.setOutput("deck",key,controller.LEDColors.off,true);
}

// Scaling of pregain (0-0xff) to gain value
PioneerCDJHID.pregainScaler = function (group,name,value) {
    return script.absoluteLin(value, 0, 4, 0, 0xff);
}

PioneerCDJHID.reloop_exit = function(field) {
    var controller = PioneerCDJHID.controller;
    var active_group = controller.resolveDeckGroup(controller.activeDeck);
    var output_packet = controller.getOutputPacket("button_leds");

    controller.setOutput("hid","beatloop_16",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_8",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_4",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_2",controller.LEDColors.off);
    output_packet.send();
    engine.setValue(active_group,"reloop_exit",field.value);
}

// Hotcues activated with normal press, cleared with shift
PioneerCDJHID.beatloop = function (field) {
    var controller = PioneerCDJHID.controller;
    var command;
    if (field.value==controller.buttonStates.released)
        return;
    if (controller.activeDeck==undefined)
        return;
    var active_group = controller.resolveGroup("deck");
    if (controller.modifiers.get("beatloop_size")) {
        size = PioneerCDJHID.beatLoopSizeMap[field.name+"_shift"] ;
    } else {
        size = PioneerCDJHID.beatLoopSizeMap[field.name];
    }
    var output_packet = controller.getOutputPacket("button_leds");
    controller.setOutput("hid","beatloop_16",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_8",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_4",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_2",controller.LEDColors.off);
    controller.setOutput("hid",field.name,controller.LEDColors.on);
    output_packet.send();

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
PioneerCDJHID.switchDeck = function(field) {
    var controller = PioneerCDJHID.controller;
    if (field.value==controller.buttonStates.released)
        return;
    controller.switchDeck();
    controller.setOutput("hid","tracknumber",controller.activeDeck);
    controller.getOutputPacket("button_leds").send();
    HIDDebug("Active CDJ deck now " + controller.activeDeck);
}



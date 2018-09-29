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
        packet.addControl("hid","tempo_reset",3,"B",0x4);
        packet.addControl("hid","master_tempo",3,"B",0x8);
        packet.addControl("hid","tempo_range",3,"B",0x10);
        packet.addControl("hid","browse_press",3,"B",0x20);
        packet.addControl("hid","menu_back",3,"B",0x40);
        packet.addControl("hid","beat_select",4,"B",0x8);
        packet.addControl("hid","beatloop_16",4,"B",0x10);
        packet.addControl("hid","beatloop_8",4,"B",0x20);
        packet.addControl("hid","beatloop_4",4,"B",0x40);
        packet.addControl("hid","beatloop_2",4,"B",0x80);
        packet.addControl("hid","hotcue_rec",5,"B",0x1);
        packet.addControl("hid","hotcue_3",5,"B",0x20);
        packet.addControl("hid","hotcue_2",5,"B",0x40);
        packet.addControl("hid","hotcue_1",5,"B",0x80);
        packet.addControl("hid","tag_track",6,"B",0x1);
        packet.addControl("hid","lock_unlock",6,"B",0x40);
        packet.addControl("hid","browse_taglist",6,"B",0x80);
        packet.addControl("hid","reverse",7,"B",0x1);
        packet.addControl("hid","jog_touch",7,"B",0x20);
        //packet.addControl("hid","jog_direction",7,"B",0x40);
        //packet.addControl("hid","jog_move",7,"B",0x80);
        packet.addControl("hid","vinyl_speed_knob",8,"B");
        packet.addControl("hid","release_start_knob",9,"B");
        packet.addControl("hid","browse_knob",10,"H",undefined,true);
        packet.addControl("hid","pitch_slider",12,"h");
        packet.addControl("hid","jog_wheel",14,"h",undefined,true);
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
        packet = new HIDPacket("lights", [0x1],36);
        packet.addOutput("hid","screen_acue",4,"B",0x1);
        packet.addOutput("hid","remain",4,"B",0x2);
        packet.addOutput("hid","screen_flag_1",4,"B",0x4);
        packet.addOutput("hid","reloop_exit",4,"B",0x8);
        packet.addOutput("hid","cue_out",4,"B",0x10);
        packet.addOutput("hid","cue_in",4,"B",0x20);
        packet.addOutput("hid","cue",4,"B",0x40);
        packet.addOutput("hid","play",4,"B",0x80);
        packet.addOutput("hid","screen_memory",5,"B",0x1);
        packet.addOutput("hid","screen_flag_2",5,"B",0x2);
        packet.addOutput("hid","screen_flag_3",5,"B",0x4);
        packet.addOutput("hid","screen_wide",5,"B",0x8);
        packet.addOutput("hid","screen_flag_4",5,"B",0x1);
        packet.addOutput("hid","screen_flag_5",5,"B",0x2);
        packet.addOutput("hid","screen_flag_6",5,"B",0x4);
        packet.addOutput("hid","screen_flag_7",5,"B",0x8);
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
        packet.addOutput("hid","jog_mode",6,"B",0x80);
        packet.addOutput("hid","jog_touch_highlight",7,"B",0x1);
        packet.addOutput("hid","screen_flag_9",7,"B",0x2);
        packet.addOutput("hid","hotcue_3_green",7,"B",0x4);
        packet.addOutput("hid","hotcue_3_red",7,"B",0x8);
        packet.addOutput("hid","hotcue_2_green",7,"B",0x10);
        packet.addOutput("hid","hotcue_2_red",7,"B",0x20);
        packet.addOutput("hid","hotcue_1_green",7,"B",0x40);
        packet.addOutput("hid","hotcue_1_red",7,"B",0x80);
        packet.addOutput("hid","browse",8,"B",0x1);
        packet.addOutput("hid","screen_flag_10",8,"B",0x2);
        packet.addOutput("hid","info",8,"B",0x4);
        packet.addOutput("hid","screen_flag_11",8,"B",0x8);
        packet.addOutput("hid","screen_flag_12",8,"B",0x10);
        packet.addOutput("hid","screen_flag_13",8,"B",0x20);
        packet.addOutput("hid","menu",8,"B",0x40);
        packet.addOutput("hid","taglist",8,"B",0x80);
        packet.addOutput("hid","flags_9",9,"B");
        packet.addOutput("hid","flags_10",10,"B");
        packet.addOutput("hid","screen_flags",11,"B");
        packet.addOutput("hid","time_minutes",12,"B");
        packet.addOutput("hid","time_seconds",13,"B");
        packet.addOutput("hid","time_frames",14,"B");
        packet.addOutput("hid","wave_summary_1",15,"B");
        packet.addOutput("hid","wave_summary_2",16,"B");
        packet.addOutput("hid","wave_summary_3",17,"B");
        packet.addOutput("hid","tracknumber",18,"B");
        packet.addOutput("hid","bpm",20,"H");
        packet.addOutput("hid","rate",22,"h");
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
    this.jogPositionDelta = function(group,name,value) {
        // We sometimes receive invalid events with value > 32000, ignore those
        if (value>=8192)
            return 0;
        return value/3;
    }

    // Pitch on CDJ sends -1000 to 1000, reset at 0, swap direction
    this.pitchScaler = function(group,name,value) { return -(value/1000); }

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
            packet.send();
        }

        controller.setOutput("hid","screen_flag_1",0);
        controller.setOutput("hid","screen_flag_2",0);
        controller.setOutput("hid","screen_flag_3",0);
        controller.setOutput("hid","screen_flag_4",0); // screen_memory_2000
        controller.setOutput("hid","screen_flag_5",0);
        controller.setOutput("hid","screen_flag_6",0);
        controller.setOutput("hid","screen_flag_7",0); // screen_wide_2000
        controller.setOutput("hid","screen_flag_8",0); // jog_touch_highlight
        controller.setOutput("hid","screen_flag_9",0);
        controller.setOutput("hid","screen_flag_10",0);
        controller.setOutput("hid","screen_flag_11",0);
        controller.setOutput("hid","screen_flag_12",0);

        controller.setOutput("hid","flag_6_1",0);
        controller.setOutput("hid","flag_6_2",0);
        controller.setOutput("hid","flag_6_4",0);
        controller.setOutput("hid","flag_6_8",0);

        controller.setOutput("hid","flags_9",0x0
            //0x1|0x2|0x4|0x8|0x10|0x20|0x40|0x80
        );
        controller.setOutput("hid","flags_10",0x0
            //0x1|0x2|0x4|0x8|0x10|0x20|0x40|0x80
        );
        controller.setOutput("hid","jog_vinyl_logo",1);

        if ( PioneerCDJHID.id=="PIONEER CDJ-900" ||
             PioneerCDJHID.id=="PIONEER CDJ-2000") {

            //tracknumber frames/ms control 0x1
            //rate control 0x10
            //bpm control 0x20
            //tracknumber control 0x40
            //jog control 0x80
            controller.setOutput("hid","screen_flags",
                0x1|0x2|0x4|0x8|0x10|0x20|0x40|0x80
            );

        } else if ( PioneerCDJHID.id=="PIONEER CDJ-850") {
            controller.setOutput("hid","screen_flags",0x1|0x10|0x20|0x40|0x80);
        } else {
            HIDDebug("Unsupported CDJ device name: " +  PioneerCDJHID.id);
        }
    }

}

PioneerCDJHID = new PioneerCDJController();

// Initialize device state
PioneerCDJHID.init = function(id) {
    id = id.replace(/(^\s*)|(\s*$)/gi,"");
    id = id.replace(/[ ]{2,}/gi," ");
    id = id.replace(/\n /,"\n");
    PioneerCDJHID.id = id;

    var controller = PioneerCDJHID.controller;
    controller.activeDeck = 1;
    controller.enableScratchCallback = PioneerCDJHID.enableScratchCallback;

    // Map beatloop sizes to actual mixxx values
    PioneerCDJHID.beatLoopSizeMap = {
        beatloop_2: "16", beatloop_2_shift: "1",      beatloop_2_hotcue: "4",
        beatloop_4: "8",  beatloop_4_shift: "0.5",    beatloop_4_hotcue: "3",
        beatloop_8: "4",  beatloop_8_shift: "0.25",   beatloop_8_hotcue: "2",
        beatloop_16: "2", beatloop_16_shift: "0.125", beatloop_16_hotcue: "1",
    }

    // Call the HID packet parser initializers
    PioneerCDJHID.initializeHIDController();
    // Link controls and register callbacks
    PioneerCDJHID.registerCallbacks();

    // Offset of play position, when 'previous track' will seek to beginning
    // of track instead of jumping to previous track
    PioneerCDJHID.previousJumpStartSeconds = 0;

    // Scratch parameters
    controller.scratchintervalsPerRev = 2048;
    controller.scratchAlpha = 1.0/6;
    controller.rampedScratchEnable = false;
    controller.toggleButtons = [ "play", "quantize", "keylock", "pfl" ];

    // Set deck switch local callbacks
    controller.disconnectDeck = PioneerCDJHID.disconnectDeck;
    controller.connectDeck = PioneerCDJHID.connectDeck;
    PioneerCDJHID.connectDeck();

    HIDDebug("Pioneer CDJ Deck "+PioneerCDJHID.id+" initialized");
}

// Device cleanup function
PioneerCDJHID.shutdown = function() {
}

PioneerCDJHID.enableScratchCallback = function(value) {
    HIDDebug("SCRATCH " + value);
    var controller = PioneerCDJHID.controller;
    controller.setOutput("hid","jog_touch_highlight",value);

}

// Scaling of pregain (0-0xff) to gain value
PioneerCDJHID.pregainScaler = function (group,name,value) {
    return script.absoluteNonLin(value,0,1,4,0,0xff);
}

// Mandatory default handler for incoming packets
PioneerCDJHID.incomingData = function(data,length) {
    PioneerCDJHID.controller.parsePacket(data,length);
}

// Link virtual HID naming of input and Output controls to mixxx
// Note: HID specification has more fields than we map here.
PioneerCDJHID.registerCallbacks = function() {
    var controller = PioneerCDJHID.controller;

    HIDDebug("Registering controls and callbacks");

    // Play/cue/reverse buttons
    controller.linkControl("hid","play","deck","play");
    controller.linkControl("hid","cue","deck","cue_default");
    controller.linkControl("hid","reverse","deck","reverse");
    controller.linkControl("hid","reloop_exit","deck","reloop_exit");
    controller.linkControl("hid","reloop_exit","deck","reloop_exit");

    // Seek buttons top of play/cue
    controller.linkControl("hid","seek_back","deck","back");
    controller.linkControl("hid","seek_forward","deck","fwd");
    controller.setCallback("control","hid","previous_track",PioneerCDJHID.track);
    controller.setCallback("control","hid","next_track",PioneerCDJHID.track);
    controller.setCallback("control","hid","tempo_reset",PioneerCDJHID.resetrate);

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
    controller.setCallback("control","hid","needle_search",PioneerCDJHID.needlesearch);

    // Handle beatloop buttons with modifier + callback
    controller.linkModifier("hid","beat_select","beatloop_size");
    controller.linkModifier("hid","hotcue_rec","hotcue_rec");
    controller.linkModifier("hid","cue_memory","hotcue_set");
    controller.linkModifier("hid","cue_delete","hotcue_delete");
    controller.setCallback("control","hid","hotcue_1",PioneerCDJHID.hotcue);
    controller.setCallback("control","hid","hotcue_2",PioneerCDJHID.hotcue);
    controller.setCallback("control","hid","hotcue_3",PioneerCDJHID.hotcue);
    controller.setCallback("control","hid","beatloop_16",PioneerCDJHID.beatloop);
    controller.setCallback("control","hid","beatloop_8",PioneerCDJHID.beatloop);
    controller.setCallback("control","hid","beatloop_4",PioneerCDJHID.beatloop);
    controller.setCallback("control","hid","beatloop_2",PioneerCDJHID.beatloop);
    controller.setCallback("control","hid","time_mode",PioneerCDJHID.timeMode);

    // Link normal jog touch and delta to HIDController default jog
    // and scratch functions. CDJ reports more than these fields from job,
    // we ignore the other input fields for jog control. Feel free to adopt.
    controller.linkControl("hid","jog_touch","deck","jog_touch");
    controller.linkControl("hid","jog_wheel","deck","jog_wheel");

    // Standard HIDController scalers for jog functionality.
    // Not related to field specifications names above!
    controller.setScaler("jog",PioneerCDJHID.jogScaler);
    controller.setScaler("jog_scratch",PioneerCDJHID.jogPositionDelta);

    // Misuse some buttons for something more useful in mixxx
    controller.linkControl("hid","cue_previous","deck","loop_halve");
    controller.linkControl("hid","cue_next","deck","loop_double");
    controller.linkControl("hid","master_tempo","deck","beatsync");
    controller.linkControl("hid","tempo_range","deck","beats_translate_curpos");
    controller.linkControl("hid","jog_mode","deck","pfl");

    // Unused buttons
    // controller.linkControl("hid","menu_back","deck","");
    // controller.linkControl("hid","tag_track","deck","");

    controller.linkOutput("hid","play","deck","play","PioneerCDJHID.updateLED");
    controller.linkOutput("hid","cue","deck","cue_default","PioneerCDJHID.updateLED");
    controller.linkOutput("hid","reverse","deck","reverse","PioneerCDJHID.updateLED");
    controller.linkOutput("hid","cue_in","deck","loop_in","PioneerCDJHID.updateLED");
    controller.linkOutput("hid","cue_out","deck","loop_out","PioneerCDJHID.updateLED");
    controller.linkOutput("hid","bpm","deck","bpm","PioneerCDJHID.bpmCallback");
    controller.linkOutput("hid","rate","deck","rate","PioneerCDJHID.rateCallback");

    // Use 'eject' button for deck switching
    controller.setCallback("control","hid","eject",PioneerCDJHID.switchDeck);

    // Use vinyl speed adjustment for pregain
    controller.setCallback("control","hid","vinyl_speed_knob",PioneerCDJHID.touchrelease);

    HIDDebug("CDJ Controls And Callbacks Registered");
}

// Callback to update LEDs from engine.connectControl
PioneerCDJHID.updateLED = function(value,group,key) {
    var controller = PioneerCDJHID.controller;
    if (value==1)
        controller.setOutput("deck",key,controller.LEDColors.on,true);
    else
        controller.setOutput("deck",key,controller.LEDColors.off,true);
}

// Switch active deck between 1 and 2
PioneerCDJHID.switchDeck = function(field) {
    var controller = PioneerCDJHID.controller;
    if (field.value==controller.buttonStates.released)
        return;
    controller.switchDeck();
    HIDDebug("Active CDJ deck now " + controller.activeDeck);
}

// Disconnect controls before deck during deck switching
PioneerCDJHID.disconnectDeck = function() {
    var controller = PioneerCDJHID.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    engine.connectControl(group,"duration","PioneerCDJHID.durationCallback",true);
    engine.connectControl(group,"playposition","PioneerCDJHID.positionCallback",true);
}

// Connect controls during init and after deck during deck switching
PioneerCDJHID.connectDeck = function() {
    var controller = PioneerCDJHID.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    var output_packet = controller.getOutputPacket("lights");

    engine.connectControl(group,"duration","PioneerCDJHID.durationCallback");
    engine.connectControl(group,"playposition","PioneerCDJHID.positionCallback");

    if (PioneerCDJHID.remain_mode) {
        controller.setOutput("hid","remain",1);
    } else {
        controller.setOutput("hid","remain",0);
    }

    PioneerCDJHID.setBPM();
    PioneerCDJHID.setRate();
    PioneerCDJHID.setDuration();
    PioneerCDJHID.setTime();
    controller.setOutput("hid","tracknumber",controller.activeDeck);
    output_packet.send();
}

// Toggle CDJ 'remain/elapsed' track display mode
PioneerCDJHID.timeMode = function(field) {
    var controller = PioneerCDJHID.controller;
    if (field.value==controller.buttonStates.released)
        return;
    PioneerCDJHID.remain_mode = (PioneerCDJHID.remain_mode!=true) ? true : false;
    if (PioneerCDJHID.remain_mode) {
        controller.setOutput("hid","remain",1);
    } else {
        controller.setOutput("hid","remain",0);
    }
}

// Set internal track duration variable
PioneerCDJHID.setDuration = function(value) {
    var controller = PioneerCDJHID.controller;
    if (value==undefined)
        PioneerCDJHID.track_length = engine.getValue(
            controller.resolveDeckGroup(controller.activeDeck),
            "duration"
        );
    else
        PioneerCDJHID.track_length = value;
}

// Set current bpm to packet
PioneerCDJHID.setBPM = function(value) {
    var controller = PioneerCDJHID.controller;
    controller.setOutput("deck","bpm", Math.floor(engine.getValue(
        controller.resolveDeckGroup(controller.activeDeck), "bpm"
    )));
}

// Set current rate to packet
PioneerCDJHID.setRate = function(value) {
    var controller = PioneerCDJHID.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    var range = Math.floor(100*engine.getValue(group,"rateRange"));

    // We still need to multiply actual rate with 100
    controller.setOutput("hid","rate",
        100 * range * engine.getValue(group,"rate_dir") * engine.getValue(group,"rate")
    );
}

// Set current time value to packet
PioneerCDJHID.setTime = function(value) {
    var controller = PioneerCDJHID.controller;
    var minutes,seconds,frames;
    if (value==undefined)
        value = engine.getValue(
            controller.resolveDeckGroup(controller.activeDeck),
            "playposition"
        );

    // CDJ 850 time control
    if (PioneerCDJHID.id=="PIONEER CDJ-850") {

        if (PioneerCDJHID.remain_mode==true) {
            if (value>=1) {
                minutes = 0x76; seconds = 0x8d; frames = 0x79;
            } else {
                position = PioneerCDJHID.track_length - PioneerCDJHID.track_length*value;
                minutes = Math.floor(0x77-(position/60));
                seconds = Math.floor(0x8d-(position%60));
                frames = 0x79-Math.floor((position-Math.floor(position))*100);
            }
        } else {
            if (value<=0) {
                position = 0; minutes = 0; seconds = 0;
            } else {
                position = PioneerCDJHID.track_length*value;
                minutes = Math.floor(position/60);
                seconds = Math.floor(position%60);
                frames = Math.floor((position-Math.floor(position))*100);
            }
        }

        controller.setOutput("hid","time_minutes",minutes);
        controller.setOutput("hid","time_seconds",seconds);
        controller.setOutput("hid","time_frames",frames);
        //controller.setOutput("hid","wave_summary_3",minutes);
        //controller.setOutput("hid","wave_summary_2",seconds);
        //controller.setOutput("hid","wave_summary_1",frames);

    } else if (PioneerCDJHID.id=="PIONEER CDJ-900" ||
               PioneerCDJHID.id=="PIONEER CDJ-2000") {

        if (PioneerCDJHID.remain_mode==true) {
            controller.setOutput("hid","wave_summary_1",0x80);
            controller.setOutput("hid","wave_summary_2",0x20);
            controller.setOutput("hid","wave_summary_3",0x10);
            if (value>=1) {
                minutes = 0x74; seconds = 0x8d; frames = 0x79;
            } else {
                position = PioneerCDJHID.track_length - PioneerCDJHID.track_length*value;
                minutes = Math.floor(0x77-(position/60));
                seconds = Math.floor(0x8d-(position%60));
                frames = 0x79-Math.floor((position-Math.floor(position))*100);
            }
            controller.setOutput("hid","wave_summary_1",0x76);
            controller.setOutput("hid","wave_summary_2",0x8d);
            controller.setOutput("hid","wave_summary_3",0x79);
        } else {
            controller.setOutput("hid","wave_summary_1",0);
            controller.setOutput("hid","wave_summary_2",0);
            controller.setOutput("hid","wave_summary_3",0);
            if (value<=0) {
                minutes = 0; seconds = 0; frames = 0;
            } else {
                position = PioneerCDJHID.track_length*value;
                minutes = Math.floor(position/60);
                seconds = Math.floor(position%60);
                frames = Math.floor((position-Math.floor(position))*100);
            }
        }
        //HIDDebug("TIME " +minutes+" "+seconds+" "+frames);
        controller.setOutput("hid","time_minutes",minutes);
        controller.setOutput("hid","time_seconds",seconds);
        controller.setOutput("hid","time_frames",frames);

    }
}

// Control callback when track duration changes (new track is loaded)
// Update all other outputs as side effect, because track changed
PioneerCDJHID.durationCallback = function(value,group,key) {
    var output_packet = PioneerCDJHID.controller.getOutputPacket("lights");
    PioneerCDJHID.setDuration(value);
    PioneerCDJHID.setBPM();
    PioneerCDJHID.setRate();
    PioneerCDJHID.setTime();
    output_packet.send();
}

// Update current track location on CDJ display
PioneerCDJHID.positionCallback = function(value,group,key) {
    var output_packet = PioneerCDJHID.controller.getOutputPacket("lights");
    PioneerCDJHID.setTime(value);
    output_packet.send();
}

// Control callback to update track BPM value
PioneerCDJHID.bpmCallback = function(value) {
    var output_packet = PioneerCDJHID.controller.getOutputPacket("lights");
    PioneerCDJHID.setBPM(value);
    output_packet.send();
}

// Control callback to update track BPM value
PioneerCDJHID.rateCallback = function(value) {
    var output_packet = PioneerCDJHID.controller.getOutputPacket("lights");
    PioneerCDJHID.setRate(value);
    output_packet.send();
}

// HID callback for previous/next track buttons.
PioneerCDJHID.track = function(field) {
    var controller = PioneerCDJHID.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);

    if (field.value==controller.buttonStates.released)
        return;

    if (field.name=='previous_track') {
        var position = PioneerCDJHID.track_length -
            PioneerCDJHID.track_length * engine.getValue(group,"playposition");
        if (position<PioneerCDJHID.previousJumpStartSeconds) {
            // Move to beginning of track if play position was far enough
            // TODO - implement this with timer so that multiple presses
            // still jump to previous track.
            // Currently just disabled by setting the previousJumpStartSeconds
            // value to 0
            engine.setValue(group,"playposition",0);
        } else {
            // Jump to previous track
            engine.setValue("[Playlist]","SelectPrevTrack",true);
            // Disable until audio bug with noise when we do this is fixed
            //engine.setValue(group,"stop",true);
            //engine.setValue(group,"eject",true);
            engine.setValue(group,"LoadSelectedTrack",true);
        }
    } else if (field.name=='next_track') {
        // Jump to next track
        engine.setValue("[Playlist]","SelectNextTrack",true);
        // Disable until audio bug with noise when we do this is fixed
        //engine.setValue(group,"stop",true);
        //engine.setValue(group,"eject",true);
        engine.setValue(group,"LoadSelectedTrack",true);
    } else {
        HIDDebug("track: Unknown track control field " + field.id);
        return;
    }
}

PioneerCDJHID.needlesearch = function(field) {
    var controller = PioneerCDJHID.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    var value = field.value / 400;
    if (value==0)
        return;
    engine.setValue(group,"play",false);
    engine.setValue(group,"playposition",value);

}

PioneerCDJHID.hotcue = function(field) {
    var controller = PioneerCDJHID.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    var control;
    if (field.value==controller.buttonStates.released) {
        controller.setOutput("hid",field.name+"_red",0);
        return;
    }

    if (controller.modifiers.get("hotcue_delete")) {
        control = field.name + '_activate';
        controller.setOutput("hid",field.name+"_red",0);
        controller.setOutput("hid",field.name+"_green",0);
        control = field.name + '_clear';
    } else if (controller.modifiers.get("hotcue_rec")) {
        control = field.name + '_set';
        controller.setOutput("hid",field.name+"_red",1);
        controller.setOutput("hid",field.name+"_green",1);
    } else {
        control = field.name + '_activate';
        controller.setOutput("hid",field.name+"_red",0);
        controller.setOutput("hid",field.name+"_green",1);
    }
    engine.setValue(group,control,true);
}

// Set given size beatloop or a hotcue, light LED for beatloop
PioneerCDJHID.beatloop = function(field) {
    var controller = PioneerCDJHID.controller;
    var group = controller.resolveDeckGroup(controller.activeDeck);
    var size;

    if (field.value==controller.buttonStates.released)
        return;

    if (controller.modifiers.get("hotcue_set")) {
        size = PioneerCDJHID.beatLoopSizeMap[field.name+"_hotcue"];
        control = "hotcue_" + size + "_activate";
    } else if (controller.modifiers.get("hotcue_delete")) {
        size = PioneerCDJHID.beatLoopSizeMap[field.name+"_hotcue"];
        control = "hotcue_" + size + "_clear";
    } else if (controller.modifiers.get("beatloop_size")) {
        size = PioneerCDJHID.beatLoopSizeMap[field.name+"_shift"] ;
        control = "beatloop_" + size + "_activate";
        engine.setValue(group,control,true);
    } else {
        size = PioneerCDJHID.beatLoopSizeMap[field.name];
        control = "beatloop_" + size + "_activate";
    }
    engine.setValue(group,control,true);

    var output_packet = controller.getOutputPacket("lights");
    controller.setOutput("hid","beatloop_16",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_8",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_4",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_2",controller.LEDColors.off);
    controller.setOutput("hid",field.name,controller.LEDColors.on);
    output_packet.send();

}

// Exit current beatloop or manual loop, reset beatloop LEDs
PioneerCDJHID.reloop_exit = function(field) {
    var controller = PioneerCDJHID.controller;
    if (field.value==controller.buttonStates.released) {
        controller.setOutput("hid",field.name,controller.LEDColors.off);
        return;
    }

    engine.setValue(
        controller.resolveDeckGroup(controller.activeDeck),
        "reloop_exit",
        true
    );
    var output_packet = controller.getOutputPacket("lights");
    controller.setOutput("hid","beatloop_16",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_8",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_4",controller.LEDColors.off);
    controller.setOutput("hid","beatloop_2",controller.LEDColors.off);
    controller.setOutput("hid",field.name,controller.LEDColors.on);
    output_packet.send();
}

PioneerCDJHID.resetrate = function (field) {
    var controller = PioneerCDJHID.controller;
    engine.setValue(
        controller.resolveDeckGroup(controller.activeDeck),
        "rate",
        0
    )
}

// Adjust deck touch/release rate: note, since mixxx doesn't yet have function
// to adjust scratch2 ramp rate, we just have a stub waiting for this call
PioneerCDJHID.touchrelease = function (field) {
    var controller = PioneerCDJHID.controller;
    HIDDebug("TOUCH_RELEASE " + controller.activeDeck + " " + field.value);
}

// Adjust pregain with deck mode knob
PioneerCDJHID.pregain = function (field) {
    var controller = PioneerCDJHID.controller;
    engine.setValue(
        controller.resolveDeckGroup(controller.activeDeck),
        "pregain",
        script.absoluteNonLin(field.value, 0, 1, 5, 0, 65536)
    );
}


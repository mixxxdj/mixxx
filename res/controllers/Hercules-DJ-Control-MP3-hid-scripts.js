/****************************************************************/
/*      Hercules DJ Control MP3 HID controller script           */
/*      For Mixxx version 1.11                                  */
/*      Author: Shalty                                          */
/*              Adaptation for the MP3 reusing the              */
/*              work on the Mk1 version by zestoi/ewanuno       */
/*                                                              */
/****************************************************************/

HerculesMP3Hid = new Controller();
HerculesMP3Hid.controls = [];
HerculesMP3Hid.leds = [];
HerculesMP3Hid.cache_in = [];
HerculesMP3Hid.cache_out = [];
HerculesMP3Hid.callbacks = [];
HerculesMP3Hid.feedbacks = [];
HerculesMP3Hid.layer = [ "fx", "fx" ]; // fx, hotcue, loop, kill
HerculesMP3Hid.tempo_scaling = 400;
HerculesMP3Hid.beatjump_size = 8;
HerculesMP3Hid.kills = [ { filterHighKill: 0, filterMidKill: 0, filterLowKill: 0 }, { filterHighKill: 0, filterMidKill: 0, filterLowKill: 0 } ];
HerculesMP3Hid.loop_lengths = [ 0.25, 0.5, 1 ]; // edit to loop size needed
HerculesMP3Hid.kill_order = [ "filterHighKill", "filterMidKill", "filterLowKill" ]; // edit if needed
HerculesMP3Hid.scratch_enabled = { "[Channel1]": false, "[Channel2]": false };
HerculesMP3Hid.jog_skip =  { "[Channel1]": true, "[Channel2]": true };
HerculesMP3Hid.shift = false; // either autobeat button

//
// the actual mapping is defined in this function
//

HerculesMP3Hid.init = function() {

    var c = HerculesMP3Hid;

    //
    // define the hid packet
    //

    c.define_hid_format();

    //
    // create the actual mapping
    // deck controls (will work for any decks as the group is passed in)
    //

    c.capture("crossfader", "all", function(g, e, v) { engine.setValue(g, e, (v - 128) / 128); });

    c.capture("play", "press", function(g, e, v) { engine.setValue(g, e, !engine.getValue(g, e)); });
    c.capture("cue_default", "all", function(g, e, v) { engine.setValue(g, e, v); });
    c.capture("beatsync", "all", function(g, e, v) { HerculesMP3Hid.shift = v > 0; engine.setValue(g, e, v); });

    c.capture("volume", "all", function(g, e, v) { engine.setValue(g, e, v / 256); });
    c.capture("filterHigh", "all", function(g, e, v) { engine.setValue(g, e, v / 128); });
    c.capture("filterMid", "all", function(g, e, v) { engine.setValue(g, e, v / 128); });
    c.capture("filterLow", "all", function(g, e, v) { engine.setValue(g, e, v / 128); });

    c.capture("jog", "all", function(g, e, v, ctrl) { 
        // skip initial jog values
        if (HerculesMP3Hid.jog_skip[g]) {
            HerculesMP3Hid.jog_skip[g] = false;
            return;
        }

        // scratch mode
        if (HerculesMP3Hid.scratch_enabled[g]) {
            engine.scratchTick(parseInt(g.substring(8,9)), ctrl.relative);
        }

        // fine jog mode when playing
        else if (engine.getValue(g, "play")) {
            engine.setValue(g, e, ctrl.relative); 
        }

        // track browsing when shift held (sync) and not playing
        else if (HerculesMP3Hid.shift) {
            engine.setValue("[Playlist]", "SelectTrackKnob", ctrl.relative);
        }

        // normal jog mode when not playing
        else {
            engine.setValue(g, e, ctrl.relative); 
        }
    });

    //
    // double up pitch bend buttons as beatjumps when the track is stopped
    //

    c.capture("pitchbend_down", "all", function(g, e, v) { 
        if (engine.getValue(g, "play") == 0) {
            engine.setValue(g, "back", v > 0 ? 1 : 0);
        }
        else if (v > 0) {
            engine.setValue(g, "jog", -3);
        }
    });

    c.capture("pitchbend_up", "all", function(g, e, v) { 
        if (engine.getValue(g, "play") == 0) {
            engine.setValue(g, "fwd", v > 0 ? 1 : 0);
        }
        else if (v > 0) {
            engine.setValue(g, "jog", 3);
        }
    });

    //
    // track browsing - don't use the joystick or track load buttons as they function as a normal mouse too
    // (map joystick_y/joystick_x/load if you want to use the joystick)
    //

    c.capture("track_previous_a", "all", c.scroll_tracks);
    c.capture("track_next_a", "all", c.scroll_tracks);
    c.capture("track_previous_b", "press", function(g, e, v) { engine.setValue("[Channel1]", "LoadSelectedTrack", 1); });
    c.capture("track_next_b", "press", function(g, e, v) { engine.setValue("[Channel2]", "LoadSelectedTrack", 1); });

    //
    // uncomment this code and the function HerculesMP3Hid.scroll_tracks_joystick() below if you really want the joystick 
    // to be used for track browsing and the left/right joystick buttons for track loading
    //

    /*
    c.capture("load", "press", function(g, e, v) { engine.setValue(g, "LoadSelectedTrack", 1); });

    c.capture("joystick_y", "all", function(g, e, v) { 
        if (v == 128) {
            HerculesMP3Hid.direction = 0;
            if (HerculesMP3Hid.track_timer) {
                engine.stopTimer(HerculesMP3Hid.track_timer);
                HerculesMP3Hid.track_timer = null;
            }
        }
        else {
            if (v > 128) {
                HerculesMP3Hid.direction = 1;
                v -= 128;
            }
            else {
                HerculesMP3Hid.direction = -1;
                v = 128 - v;
            }

            if (v < 30) v = 30;
            if (HerculesMP3Hid.track_timer) engine.stopTimer(HerculesMP3Hid.track_timer);
            HerculesMP3Hid.track_timer = engine.beginTimer(parseInt(5120 / v), 'HerculesMP3Hid.scroll_tracks_joystick');
        }
    });
    */

    //
    // tempo encoder 
    //

    c.capture("rate", "all", function(g, e, v, ctrl) { 
        var rate = engine.getValue(g, "rate") + ctrl.relative / c.tempo_scaling;
        if (rate > 1) rate = 1; else if (rate < -1) rate = -1;
        engine.setValue(g, e, rate);
    });

    //
    // enable/disable scratching with the beatlock buttons (as jogs are non touch sensitive)
    //

    c.capture("beatlock", "press", function(g, e, v) { 

        HerculesMP3Hid.scratch_enabled[g] = !HerculesMP3Hid.scratch_enabled[g];
        
        if (HerculesMP3Hid.scratch_enabled[g]) {
            engine.scratchEnable(parseInt(g.substring(8,9)), 64, 45, 0.125, 0.125/32);
        }
        else {
            engine.scratchDisable(parseInt(g.substring(8,9)));
        }

        c.send(g, e, HerculesMP3Hid.scratch_enabled[g] ? 1 : 0);
    });

    //
    // toggle between fx, hotcue and loop modes and update leds
    //

    c.capture("layer_select", "press", function(g, e, v) {
        var deck = parseInt(g.substring(8,9));
        c.send(g, "fx", 0);
        c.send(g, "hotcue", 0);
        c.send(g, "loop", 0);
        switch (c.layer[deck-1]) {
            case "fx": c.layer[deck-1] = "hotcue"; break;
            case "hotcue": c.layer[deck-1] = "loop"; break;
            case "loop": 
                c.layer[deck-1] = "kill"; 
                c.send(g, "fx", !c.kills[deck-1]['filterHighKill']);
                c.send(g, "hotcue", !c.kills[deck-1]['filterMidKill']);
                c.send(g, "loop", !c.kills[deck-1]['filterLowKill']);
                break;
            case "kill": 
                c.layer[deck-1] = "fx";
        }
        if (c.layer[deck-1] != "kill") {
            c.send(g, c.layer[deck-1], 1);
        }
    });

    c.capture("layer_btn1", "all", function(g, e, v) { c.layer_btn(g, e, v); });
    c.capture("layer_btn2", "all", function(g, e, v) { c.layer_btn(g, e, v); });
    c.capture("layer_btn3", "all", function(g, e, v) { c.layer_btn(g, e, v); });

    //
    // headphone cue
    //

    c.capture("monitor_a", "all", function(g, e, v) {
        if(v==1){
            var pflstatusa = engine.getValue("[Channel1]", "pfl");
            //print("pflstatusa " + pflstatusa);
            if (pflstatusa == 1){
            engine.setValue("[Channel1]", "pfl", 0);}
            else {engine.setValue("[Channel1]", "pfl", 1); 
            }
        }
    });

    
    c.capture("monitor_b", "all", function(g, e, v) {
        if(v==1){
            var pflstatusb = engine.getValue("[Channel2]", "pfl");
            //print("pflstatusb " + pflstatusb);
            if (pflstatusb == 1){
            engine.setValue("[Channel2]", "pfl", 0);}
            else {engine.setValue("[Channel2]", "pfl", 1); 
            }
        }
    });    


    c.feedback("[Channel1]", "play", function(g, e, v) { c.send(g, e, v); });
    c.feedback("[Channel2]", "play", function(g, e, v) { c.send(g, e, v); });
    c.feedback("[Channel1]", "cue_default", function(g, e, v) { c.send(g, e, v); });
    c.feedback("[Channel2]", "cue_default", function(g, e, v) { c.send(g, e, v); });
    c.feedback("[Channel1]", "beatsync", function(g, e, v) { c.send(g, e, v); });
    c.feedback("[Channel2]", "beatsync", function(g, e, v) { c.send(g, e, v); });

    
    c.feedback("[Channel1]", "pfl", function(g, e, v) { c.send(g, e, v); });
    c.feedback("[Channel2]", "pfl", function(g, e, v) { c.send(g, e, v); });
    c.feedback("[Channel1]", "beat_active", function(g, e, v) { c.send(g, e, v); });
    c.feedback("[Channel2]", "beat_active", function(g, e, v) { c.send(g, e, v); });

    //
    // kill status
    //

    c.feedback("[Channel1]", "filterHighKill", c.kill_status);
    c.feedback("[Channel1]", "filterMidKill", c.kill_status);
    c.feedback("[Channel1]", "filterLowKill", c.kill_status);
    c.feedback("[Channel2]", "filterHighKill", c.kill_status);
    c.feedback("[Channel2]", "filterMidKill", c.kill_status);
    c.feedback("[Channel2]", "filterLowKill", c.kill_status);

    //
    // clear/setup any initial leds
    //

    for (id in c.leds) {
        c.send(c.leds[id].group, c.leds[id].name, 0);
    }

    c.send("[Channel1]", "fx", 1);
    c.send("[Channel2]", "fx", 1);
}


HerculesMP3Hid.shutdown = function() {

    var c = HerculesMP3Hid;
    
    // Elvis has left the building...
    for (id in c.leds) {
        c.send(c.leds[id].group, c.leds[id].name, 0);
    }

}
//
// map the 6 buttons that control either effects, hotcues, loops or kills
//

HerculesMP3Hid.layer_btn = function(g, e, v) {
    var deck = parseInt(g.substring(8,9));
    var btn = parseInt(e.substring(9,10));
    switch (HerculesMP3Hid.layer[deck-1]) {
        case "fx":
            print("fxmode");
            switch (btn) {
                case 1:
                    engine.setValue("[Flanger]", "lfoDepth", 1);
                    engine.setValue("[Flanger]", "lfoPeriod", 500000);
                    engine.setValue("[Flanger]", "lfoDelay", 666);
                    engine.setValue(g, "flanger", v > 0);
                    break;
                case 2:
                    engine.spinback(parseInt(g.substring(8,9)), v > 0);
                    break;
                case 3:
                    engine.brake(parseInt(g.substring(8,9)), v > 0);
            }
            break;
        case "hotcue":
            print("hotcuemode");
            engine.setValue(g, "hotcue_" + btn + "_activate", v > 0 ? 1 : 0);
            break;
        case "loop":
            print("loopmode");
            var len = HerculesMP3Hid.loop_lengths[btn-1];
            engine.setValue(g, "beatloop_" + len + "_toggle", 1);
            break;
        case 'kill':
            print("killmode");
            if (v > 0) {
                engine.setValue(g, HerculesMP3Hid.kill_order[btn-1], !engine.getValue(g, HerculesMP3Hid.kill_order[btn-1]));
            }
    }
}

//
// beatjump - will get out of sync if called while deck is playing
//

HerculesMP3Hid.beatjump = function(group, jump) {
    jump = jump * 120 / engine.getValue(group, "bpm") / engine.getValue(group, "track_samples") * engine.getValue(group, "track_samplerate");
    engine.setValue(group, "playposition", engine.getValue(group, "playposition") + jump);
}

//
// playlist scroll nex/previous with auto-repeat when held
//

HerculesMP3Hid.scroll_tracks = function(g, e, v) {
    if (v > 0) {
        engine.setValue("[Playlist]", e == "track_next_a" ? "SelectNextTrack" : "SelectPrevTrack", 1);
        if (!HerculesMP3Hid.scroll_timer) {
            HerculesMP3Hid.scroll_timer = engine.beginTimer(150, 'HerculesMP3Hid.scroll_tracks("[Playlist]","' + e + '",' + v + ')');
        }
    }
    else {
        if (HerculesMP3Hid.scroll_timer) {
            engine.stopTimer(HerculesMP3Hid.scroll_timer);
            HerculesMP3Hid.scroll_timer = null;
        }
    }
}

//
// eq kill status
//

HerculesMP3Hid.kill_status = function(g, e, v) {
    var deck = parseInt(g.substring(8,9));
    HerculesMP3Hid.kills[deck-1][e] = v;

    //
    // update leds with kill status if we're on that layer
    //

    if (HerculesMP3Hid.layer[deck-1] == "kill") {
        
        switch (e) {
            case 'filterHighKill': HerculesMP3Hid.send(g, "fx", !v); break;
            case 'filterMidKill': HerculesMP3Hid.send(g, "hotcue", !v); break;
            case 'filterLowKill': HerculesMP3Hid.send(g, "loop", !v); 
        }
    }
}

/*
HerculesMP3Hid.scroll_tracks_joystick = function() {
    engine.setValue("[Playlist]", HerculesMP3Hid.direction > 0 ? "SelectNextTrack" : "SelectPrevTrack", 1);
}
*/


////////////////////////////////////////////////////////////////////////////////////////////////////
// define the hid packet to event mapping, could be defined via xml so can be used in multiple mappings
// naming the controls as much as possible inline with the mixxx engine names makes most mappings trivial
//

HerculesMP3Hid.define_hid_format = function() {

    var c = HerculesMP3Hid;
    var pid = 0x1;

    // deck 1

    c.add_control(pid, "play", "[Channel1]", "button", 1, 0x80)
    c.add_control(pid, "cue_default", "[Channel1]", "button", 2, 0x01)
    c.add_control(pid, "track_previous_a", "[Channel1]", "button", 2, 0x04)
    c.add_control(pid, "track_next_a", "[Channel1]", "button", 2, 0x08)
    c.add_control(pid, "beatsync", "[Channel1]", "button", 2, 0x02)
    c.add_control(pid, "pitchbend_down", "[Channel1]", "button", 3, 0x08)
    c.add_control(pid, "pitchbend_up", "[Channel1]", "button", 3, 0x04)
    c.add_control(pid, "load", "[Channel1]", "button", 4, 0x04)
    c.add_control(pid, "beatlock", "[Channel1]", "button", 3, 0x20)
    c.add_control(pid, "source", "[Channel1]", "button", 4, 0x10)
    c.add_control(pid, "filterLow", "[Channel1]", "fader", 8, 0xff)
    c.add_control(pid, "filterMid", "[Channel1]", "fader", 9, 0xff)
    c.add_control(pid, "filterHigh", "[Channel1]", "fader", 10, 0xff)
    c.add_control(pid, "volume", "[Channel1]", "fader", 12, 0xff)
    c.add_control(pid, "rate", "[Channel1]", "encoder", 14, 0xff)
    c.add_control(pid, "jog", "[Channel1]", "encoder", 16, 0xff)
    c.add_control(pid, "layer_select", "[Channel1]", "button", 1, 0x40)
    c.add_control(pid, "layer_btn1", "[Channel1]", "button", 2, 0x40)
    c.add_control(pid, "layer_btn2", "[Channel1]", "button", 2, 0x20)
    c.add_control(pid, "layer_btn3", "[Channel1]", "button", 2, 0x10)

    // deck 2

    c.add_control(pid, "play", "[Channel2]", "button", 1, 0x02)
    c.add_control(pid, "cue_default", "[Channel2]", "button", 1, 0x04)
    c.add_control(pid, "track_previous_b", "[Channel2]", "button", 1, 0x10)
    c.add_control(pid, "track_next_b", "[Channel2]", "button", 1, 0x20)
    c.add_control(pid, "beatsync", "[Channel2]", "button", 1, 0x08)
    c.add_control(pid, "pitchbend_down", "[Channel2]", "button", 3, 0x80)
    c.add_control(pid, "pitchbend_up", "[Channel2]", "button", 3, 0x40)
    c.add_control(pid, "load", "[Channel2]", "button", 4, 0x08)
    c.add_control(pid, "beatlock", "[Channel2]", "button", 4, 0x02)
    c.add_control(pid, "source", "[Channel2]", "button", 4, 0x20)
    c.add_control(pid, "filterLow", "[Channel2]", "fader", 5, 0xff)
    c.add_control(pid, "filterMid", "[Channel2]", "fader", 6, 0xff)
    c.add_control(pid, "filterHigh", "[Channel2]", "fader", 7, 0xff)
    c.add_control(pid, "volume", "[Channel2]", "fader", 13, 0xff)
    c.add_control(pid, "rate", "[Channel2]", "encoder", 15, 0xff)
    c.add_control(pid, "jog", "[Channel2]", "encoder", 17, 0xff)
    c.add_control(pid, "layer_select", "[Channel2]", "button", 1, 0x01)
    c.add_control(pid, "layer_btn1", "[Channel2]", "button", 2, 0x80)
    c.add_control(pid, "layer_btn2", "[Channel2]", "button", 3, 0x01)
    c.add_control(pid, "layer_btn3", "[Channel2]", "button", 3, 0x02)

    // master

    c.add_control(pid, "crossfader", "[Master]", "fader", 11, 0xff)
    //c.add_control(pid, "joystick_x", "[Master]", "fader", 19, 0xff)
    //c.add_control(pid, "joystick_y", "[Master]", "fader", 20, 0xff)

    // headphone cue

    c.add_control(pid, "monitor_a", "[Master]", "button", 3, 0x10);
    c.add_control(pid, "monitor_b", "[Master]", "button", 4, 0x01);
    //c.add_control(pid, "monitor_both", "[Master]", "button", 5, 0x4); 
    c.add_control(pid, "monitor_both", "[Master]", "button", 5, 0x8);

    // define led feedback
    
    pid = 0;    
    c.cache_out[pid] = [ 0x0, 0x0, 0x0 ];  // the first byte is not the reportID, that's the raw packet... reportID will be appended later

    c.add_control(pid, "play",      "[Channel1]", "led", 2, 0x02); // blinking: 3, 0x2
    c.add_control(pid, "cue_default","[Channel1]", "led", 1, 0x08); 
    c.add_control(pid, "beat_active","[Channel1]", "led", 1, 0x10);
    c.add_control(pid, "beatlock",      "[Channel1]", "led", 0, 0x01);
    c.add_control(pid, "fx",      "[Channel1]", "led", 0, 0x04);
    c.add_control(pid, "hotcue",      "[Channel1]", "led", 0, 0x80);
    c.add_control(pid, "loop",      "[Channel1]", "led", 1, 0x40);
    c.add_control(pid, "pfl",      "[Channel1]", "led", 1, 0x02);

    c.add_control(pid, "play",      "[Channel2]", "led", 2, 0x01); // blinking: 3, 0x1
    c.add_control(pid, "cue_default","[Channel2]", "led", 0, 0x20);
    c.add_control(pid, "beat_active","[Channel2]", "led", 1, 0x20);
    c.add_control(pid, "beatlock",      "[Channel2]", "led", 0, 0x02);
    c.add_control(pid, "fx",      "[Channel2]", "led", 0, 0x08);
    c.add_control(pid, "hotcue",      "[Channel2]", "led", 0, 0x10);    
    c.add_control(pid, "loop",      "[Channel2]", "led", 1, 0x80);
    c.add_control(pid, "pfl",      "[Channel2]", "led", 1, 0x04);
            

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// non-specific controller framework to allow hid packets to be defined and processed via
// callback functions - could/should be in a shared file
//

HerculesMP3Hid.add_control = function(packetid, name, group, type, offset, mask) {
    if (type == "led") {
        HerculesMP3Hid.leds[group + name] = new HerculesMP3Hid.control(packetid, name, group, type, offset, mask);
    }
    else {
        if (HerculesMP3Hid.controls[offset] == undefined) {
            HerculesMP3Hid.controls[offset] = [];
        }
        HerculesMP3Hid.controls[offset].push(new HerculesMP3Hid.control(packetid, name, group, type, offset, mask));
    }
}

//
// bind a function to a modified controller value
//

HerculesMP3Hid.capture = function(name, values, func) {
    if (HerculesMP3Hid.callbacks[name] == undefined) {
        HerculesMP3Hid.callbacks[name] = [ [ values, func ] ];
    }
    else {
        HerculesMP3Hid.callbacks[name].push([ values, func ]);
    }
}

//
// bind a function to feedback from mixxx, callbacks accept args in same order as from capture()
//

HerculesMP3Hid.feedback = function(g, e, f) {
    engine.connectControl(g, e, "HerculesMP3Hid.feedbackData");
    if (HerculesMP3Hid.feedbacks[g + e] == undefined) {
        HerculesMP3Hid.feedbacks[g + e] = [];
    }
    HerculesMP3Hid.feedbacks[g + e].push(f);
}

//
// controller feedback: send data to the controller by name and automatically send out the full hid packet needed
//

HerculesMP3Hid.send = function(g, e, v) {
    if ((ctrl = this.leds[g + e]) != undefined) {

        //
        // for the byte in the hid packet that this led control affects, mask out it's old value
        // and then add in it's new one
        //

        this.cache_out[ctrl.packetid][ctrl.offset] = this.cache_out[ctrl.packetid][ctrl.offset] & ctrl.maskinv | (v << ctrl.bitshift);

        //
        // send complete hid packet and update our cache
        //

        controller.send(this.cache_out[ctrl.packetid], this.cache_out[ctrl.packetid].length, 1);  // packetID = 1!
        this.cache_out[ctrl.packetid] = this.cache_out[ctrl.packetid];
    }
}

//
// process incoming data from mixxx and call any callbacks
//

HerculesMP3Hid.feedbackData = function(v, g, e) {
    if (HerculesMP3Hid.feedbacks[g + e] != undefined) {
        for (func in HerculesMP3Hid.feedbacks[g + e]) {
            if (typeof(HerculesMP3Hid.feedbacks[g + e][func]) == "function") {
                HerculesMP3Hid.feedbacks[g + e][func](g, e, v);
            }
        }
    }
}

//
// a single hid control, store last known value and offset/mask to work out the new value from incoming data
//

HerculesMP3Hid.control = function(packetid, name, group, type, offset, mask) {
    this.packetid = packetid;
    this.name = name;
    this.group = group;
    this.type = type;
    this.value = 0;
    this.relative = 0;
    this.offset = offset;
    this.mask = mask;
    this.maskinv = ~mask;
    this.bitshift = 0;
    this.maxval = 255; // needed for encoder, could guess from the mask
    this.changed = function(value) {
        value = (value & this.mask) >> this.bitshift; 
        if (this.value == value) {
            return false;
        }
        else {
            // map to a relative value if it's an encoder, usually +1 or -1
            if (this.type == 'encoder') {
                this.relative = value - this.value;
                if (this.relative > 100) {
                    this.relative -= this.maxval;
                }
                else if (this.relative < -100) {
                    this.relative += this.maxval;
                }
            }
            this.value = value;
            return true;
        }
    };
    while (mask != 0 && (mask & 0x1) == 0) {
        mask = mask >> 1;
        this.bitshift++;
    }
}

//
// process incoming data and call any callbacks if their bound controls have changed
//

HerculesMP3Hid.incomingData = function (data, length) {

    var c = HerculesMP3Hid;
    var packetid = data[0];

    //
    // iterate thru each byte and only check controls for that byte if the byte has changed
    //

    for (i=1; i<length; i++) {
        if ((c.cache_in[packetid] == undefined || data[i] != c.cache_in[packetid][i]) && c.controls[i] != undefined) {

            //
            // a byte has changed, check any controls defined in that byte, more efficient
            // than checking old+new values for all controls
            //

            for (key in c.controls[i]) {
                var control = c.controls[i][key];
                if (typeof(control) == 'object' && control.packetid == data[0] && control.changed(data[i])) {

                    //
                    // we found a hid control that has changed value within that byte, check for callbacks
                    //

                    var callbacks = c.callbacks[control.name];
                    if (callbacks != undefined) {
                        for (var i=0; i<callbacks.length; i++) {
                            if (typeof(callbacks[i][1]) == 'function') {

                                //
                                // check we need to call for this value change: all, press, release
                                //

                                if (callbacks[i][0] == "all" ||
                                    (callbacks[i][0] == "press" && control.value > 0) ||
                                    (callbacks[i][0] == "release" && control.value == 0)) {

                                    //
                                    // call a callback function for this control
                                    //

                                    callbacks[i][1](control.group, control.name, control.value, control);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // store the new raw data
    c.cache_in[data[0]] = data;
}

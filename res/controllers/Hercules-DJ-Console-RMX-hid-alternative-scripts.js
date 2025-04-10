/****************************************************************/
/*      Hercules DJ Console RMX HID controller script           */
/*      For Mixxx version 2.1                                   */
/*      Author: RichterSkala                                    */
/*      Contributors: Markus Kohlhase, Jesse French             */
/*      Based on zestoi's script                                */
/****************************************************************/

RMX = new Controller();

RMX.controls  = [];
RMX.leds      = [];
RMX.cache_in  = [];
RMX.cache_out = [];
RMX.callbacks = [];
RMX.feedbacks = [];

//State variables:
RMX.scratch_enabled = false;

RMX.scratching = {
  "[Channel1]": false,
  "[Channel2]": false
};

RMX.jog_skip =  {
  "[Channel1]": true,
  "[Channel2]": true
};

RMX.shift = false; // controlled via stop button

//Config:
RMX.eq_max = 4; //eq max value. Full range =4, minimal value (=linear) =2

// the actual mapping is defined in this function
RMX.init = function() {

  var c = RMX;

  // define the hid packet
  c.define_hid_format();

  // create the actual mapping
  // deck controls (will work for any decks as the group is passed in)

  c.capture("crossfader", "all", function(g, e, v) {
    engine.setValue(g, e, (v - 128) / 126);
  });

  c.capture("balance", "all", function(g, e, v) {
    engine.setValue(g, e, (v-128) / 128);
  });

  // brake / soft start on shift (stop) + play
  c.capture("play", "press", function(g, e, v) {

    if(RMX.shift){ // if stop button pressed
      var deck = parseInt(g.substring(8,9)); // work out which deck we are using
      if(!engine.getValue(g, 'play')){ // if not playing
        engine.softStart(deck, true);
      } else {
        engine.brake(deck, true);
      }
    } else { 
      engine.setValue(g, e, !engine.getValue(g, e)); // toggle play
    }
  });

  c.capture("play_indicator", "all", function(g, e, v) {
    engine.setValue(g, e, v);
  });

  c.capture("cue_default", "all", function(g, e, v) {
    engine.setValue(g, e, v);
  });

  c.capture("cue_indicator", "all", function(g, e, v) {
    engine.setValue(g, e, v);
  });

  // TODO: Think about adapting to master tempo for 1.12
  c.capture("beatsync", "all", function(g, e, v) {
    engine.setValue(g, e, v);
  });

  c.capture("volume", "all", function(g, e, v) {
    if (g === '[Master]'){
      engine.setValue(g,e,Math.pow(v,2)/13107.2);
    } else {
      engine.setValue(g, e, v/256);
    }
  });

  // gain & filters go from 0..1..4 in mixxx, scale in fct.
  c.capture("pregain", "all", c.filter_scale);


  // when shift key is pressed, filter knobs control FX meta knobs
  c.capture("filterHigh", "all", function(g,e,v){
    if(RMX.shift){
      if(g==="[Channel1]"){
        engine.setParameter("[EffectRack1_EffectUnit1_Effect1]", "meta", v/256);
      } else if (g==="[Channel2]"){
        engine.setParameter("[EffectRack1_EffectUnit2_Effect1]", "meta", v/256);
      }
    }
    else {c.filter_scale(g,e,v);}
  });
  c.capture("filterMid",  "all", function(g,e,v){
    if(RMX.shift){
      if(g==="[Channel1]"){
        engine.setParameter("[EffectRack1_EffectUnit1_Effect2]", "meta", v/256);
      } else if (g==="[Channel2]"){
        engine.setParameter("[EffectRack1_EffectUnit2_Effect2]", "meta", v/256);
      }
    }
    else {c.filter_scale(g,e,v);}
  });
  c.capture("filterLow",  "all", function(g,e,v){
    if(RMX.shift){
      if(g==="[Channel1]"){
        engine.setParameter("[EffectRack1_EffectUnit1_Effect3]", "meta", v/256);
      } else if (g==="[Channel2]"){
        engine.setParameter("[EffectRack1_EffectUnit2_Effect3]", "meta", v/256);
      }
    }
    else {c.filter_scale(g,e,v);}
  });

  // Handle Jog-wheel in own function
  c.capture("jog", "all", c.jog);

  // stop toggle for shift
  c.capture("stop", "all", function(g,e,v) { RMX.shift = v > 0? 1 : 0; });

  c.capture("previous", "all", function(g, e, v) {
      if (RMX.shift) {
        engine.setValue(g, "loop_halve", !engine.getValue(g, "loop_halve"));     //halve the loop when shift + rew pressed
      }
      
      else if (engine.getValue(g, "loop_enabled")){
        engine.setValue (g, "loop_end_position", engine.getValue(g, "loop_end_position") - 450); //adjust loop out position when loop is enabled
      }
      
      else {
        engine.setValue(g, "back", v > 0 ? 1 : 0);
      }
    }
  );

  c.capture("next", "all", function(g, e, v) { 
    if (RMX.shift) {
      engine.setValue(g, "loop_double", !engine.getValue(g, "loop_double"));      //double the loop when shift + next pressed
    }
    else if (engine.getValue(g, "loop_enabled")){
      engine.setValue (g, "loop_end_position", 450 + engine.getValue(g, "loop_end_position"));  //adjust loop out position when loop is enabled
    }
    else{
      engine.setValue(g, "fwd", v > 0 ? 1 : 0);
    }
  }  
  );

  // track browsing

  c.capture("menu_up", "all", c.scroll_tracks);

  c.capture("menu_down", "all", c.scroll_tracks);

  c.capture("menu_left", "all", function(g, e, v) {
    engine.setValue("[Playlist]", "SelectPrevPlaylist", v);
  });

  c.capture("menu_right", "all", function(g,e,v) {
    engine.setValue("[Playlist]", "SelectNextPlaylist",v);
  });

  c.capture("LoadSelectedTrack", "press", function(g, e, v) {
    engine.setValue(g, e, 1);
  });

  // fader

  c.capture("rate", "all", function(g, e, v) {
    // TODO: Figure out if we can still use full range
    // and not lose information at top.
    engine.setValue(g, e, (v-127)/128);
    // TODO: Reimplement rate magnification.
  });

  // enable/disable scratching
  c.capture("scratch", "press", function(g, e, v) {
    RMX.scratch_enabled = !RMX.scratch_enabled;
    c.send(g, e, RMX.scratch_enabled ? 1: 0);
  });

  c.capture("beatlock", "press", function(g, e, v) {
    engine.setValue(g,"keylock",!engine.getValue(g, "keylock"));
  });

  c.capture("pitch_reset", "press", function(g, e, v) {
    engine.setValue(g,"rate",0);
  });

  // headphone cue

  c.capture("headphone_cue", "press", function(g, e, v) {
    engine.setValue(g, "pfl", !engine.getValue(g, "pfl"));
  });

  c.capture("headMix", "all", function(g, e, v) {
    engine.setValue(g, e, (v-128)/128);
  });

  var print_unhandled_button = function(g,e,v) { print("Unhandled button");};

  c.capture("keypad1","all", function(g, e, v) {
    engine.setValue(g,"hotcue_1_activate",v)
  } );
  c.capture("keypad2","all", function(g, e, v) {
    engine.setValue(g,"hotcue_2_activate",v)
  } );
  c.capture("keypad3","all", function(g, e, v) {
    engine.setValue(g,"hotcue_3_activate",v)
  } );
  c.capture("keypad4","all", function(g, e, v) {
    engine.setValue(g,"hotcue_4_activate",v)
  } );
  
  c.capture("keypad5","all", function(g, e, v) {
    if (RMX.shift){
      engine.setValue(g,"beatlooproll_activate",v)
    }
    else engine.setValue(g,"loop_in",v) 
  } );
  c.capture("keypad6","all", function(g, e, v) {
    if(RMX.shift){
      engine.setValue(g, "reloop_exit", !engine.getValue(g, "reloop_exit"));
    }
    else engine.setValue(g,"loop_out",!engine.getValue(g, "loop_out"))
  } );

  var filterKill = function(g, e, v) {
    engine.setValue(g, e, !engine.getValue(g, e));
  };

  // when shift enabled, filter kill buttons turn on / off FX
  c.capture("filterHighKill","press", function(g, e, v){
    if(!RMX.shift){
      filterKill(g,e,v);
    }
    else {
      if(g==="[Channel1]"){
        engine.setValue("[EffectRack1_EffectUnit1_Effect1]", "enabled", !engine.getValue("[EffectRack1_EffectUnit1_Effect1]", "enabled", v))
      } else if(g==="[Channel2]"){
        engine.setValue("[EffectRack1_EffectUnit2_Effect1]", "enabled", !engine.getValue("[EffectRack1_EffectUnit2_Effect1]", "enabled", v))
      }
    }
  });
  c.capture("filterMidKill", "press", function(g, e, v){
    if(!RMX.shift){
      filterKill(g,e,v);
    }
    else {
      if(g==="[Channel1]"){
        engine.setValue("[EffectRack1_EffectUnit1_Effect2]", "enabled", !engine.getValue("[EffectRack1_EffectUnit1_Effect2]", "enabled", v))
      } else if(g==="[Channel2]"){
        engine.setValue("[EffectRack1_EffectUnit2_Effect2]", "enabled", !engine.getValue("[EffectRack1_EffectUnit2_Effect2]", "enabled", v))
      }
    }
  });
  c.capture("filterLowKill", "press", function(g, e, v){
    if(!RMX.shift){
      filterKill(g,e,v);
    }
    else {
      if(g==="[Channel1]"){
        engine.setValue("[EffectRack1_EffectUnit1_Effect3]", "enabled", !engine.getValue("[EffectRack1_EffectUnit1_Effect3]", "enabled", v))
      } else if(g==="[Channel2]"){
        engine.setValue("[EffectRack1_EffectUnit2_Effect3]", "enabled", !engine.getValue("[EffectRack1_EffectUnit2_Effect3]", "enabled", v))
      }
    }
  });

  var pipe = function(g, e, v) { c.send(g,e,v); };

  // led feedback
  c.feedback("[Channel1]", "play",        pipe);
  c.feedback("[Channel1]", "play_indicator",        pipe);
  c.feedback("[Channel2]", "play",        pipe);
  c.feedback("[Channel2]", "play_indicator",        pipe);
  c.feedback("[Channel1]", "cue_default", pipe);
  c.feedback("[Channel1]", "cue_indicator", pipe);
  c.feedback("[Channel2]", "cue_default", pipe);
  c.feedback("[Channel2]", "cue_indicator", pipe);
  c.feedback("[Channel1]", "beatsync",    pipe);
  c.feedback("[Channel2]", "beatsync",    pipe);

  c.feedback("[Channel1]", "keylock",
    function(g, e, v) { c.send(g, "beatlock", v);
  });

  c.feedback("[Channel2]", "keylock",
    function(g, e, v) { c.send(g, "beatlock", v);
  });

  c.feedback("[Channel1]", "pfl",
    function(g, e, v) { c.send(g, "headphone_cue", v);
  });

  c.feedback("[Channel2]", "pfl",
    function(g, e, v) { c.send(g, "headphone_cue", v);
  });


  RMX.shutdown = function() {
    var c = RMX; 
    for (id in c.leds) {
      c.send(c.leds[id].group, c.leds[id].name, 0);
    }
  }

  // Enable soft-takeover for all direct hardware controls
  // engine.softTakeover("[Channel1]","rate",true);
  engine.softTakeover("[Channel1]","volume",true);
  engine.softTakeover("[Channel1]","pregain",true);
  engine.softTakeover("[Channel1]","filterHigh",true);
  engine.softTakeover("[Channel1]","filterMid",true);
  engine.softTakeover("[Channel1]","filterLow",true);
  // engine.softTakeover("[Channel2]","rate",true);
  engine.softTakeover("[Channel2]","volume",true);
  engine.softTakeover("[Channel2]","pregain",true);
  engine.softTakeover("[Channel2]","filterHigh",true);
  engine.softTakeover("[Channel2]","filterMid",true);
  engine.softTakeover("[Channel2]","filterLow",true);
  // engine.softTakeover("[Master]","crossfader",true);
  // engine.softTakeover("[Master]","headVolume",true);
  // engine.softTakeover("[Master]","headMix",true);
  // engine.softTakeover("[Master]","volume",true);
  // engine.softTakeover("[Master]","balance",true);



};

// playlist scroll nex/previous with auto-repeat when held

RMX.scroll_tracks = function(g, e, v) {
  if (v > 0) {

    var direction = e == "menu_down" ? "SelectNextTrack" : "SelectPrevTrack";
    engine.setValue("[Playlist]", direction, 1);

    if (!RMX.scroll_timer) {
      var callback = 'RMX.scroll_tracks("[Playlist]","' + e + '",' + v + ')';
      RMX.scroll_timer = engine.beginTimer(140, callback);
    }
  }
  else {
    if (RMX.scroll_timer) {
      engine.stopTimer(RMX.scroll_timer);
      RMX.scroll_timer = null;
    }
  }
};

// scale the knobs according to RMX.eq_max

RMX.filter_scale = function(g, e, v) {
  if (v>128) {
    engine.setValue(g, e, (RMX.eq_max-1) * ((v / 128)-1)+1);
  } else {
    engine.setValue(g, e, v /128);
  }

};

// jog
RMX.jog = function(g, e, v, ctrl) {

  // skip initial jog values
  if (RMX.jog_skip[g]) {
    RMX.jog_skip[g] = false;
    return;
  }

  // scratch mode
  if (RMX.scratch_enabled) {

    var deck = parseInt(g.substring(8,9));

    if (!RMX.scratching[g]) {

      RMX.scratching[g] = true;
      var ScratchRPM;

      if(engine.getValue(g,"play")) {
        ScratchRPM = 45;
      } else {
        ScratchRPM = 80;
      }

      //Don't make any sound when trimming loops
      engine.scratchEnable(deck, 128, ScratchRPM, 1.0/8, (1.0/8)/32);
    } else {
      engine.stopTimer(RMX.scratchTimer);
    }
    engine.scratchTick(deck, ctrl.relative);

    var callback = 'RMX.stopScratching(\"' + g + '\")';
    RMX.scratchTimer = engine.beginTimer(20, callback, true);
  }

  // fine jog mode when playing
  else if (engine.getValue(g, "play")) {
    engine.setValue(g, e, ctrl.relative/2);
  }

  // track browsing when shift held and not playing
  else if (RMX.shift) {
    engine.setValue("[Playlist]", "SelectTrackKnob", ctrl.relative);
  }

  // normal jog mode when not playing
  else {
    engine.setValue(g, e, ctrl.relative);
  }
};

RMX.stopScratching = function(g) {
  var deck = parseInt(g.substring(8,9));
  RMX.scratching[g] = false;
  engine.scratchDisable(deck);
};

/**
 * define the hid packet to event mapping, could be defined via xml so can be
 * used in multiple mappings
 * naming the controls as much as possible inline with the mixxx engine names
 * makes most mappings trivial
 */

RMX.define_hid_format = function() {

  var c = RMX;
  var pid = 0x1;
  // Order as in manual
  // deck 1 - buttons
  c.add_control(pid, "keypad1",           "[Channel1]", "button", 1, 0x01);
  c.add_control(pid, "keypad2",           "[Channel1]", "button", 1, 0x02);
  c.add_control(pid, "keypad3",           "[Channel1]", "button", 1, 0x04);
  c.add_control(pid, "keypad4",           "[Channel1]", "button", 1, 0x08);
  c.add_control(pid, "keypad5",           "[Channel1]", "button", 1, 0x10);
  c.add_control(pid, "keypad6",           "[Channel1]", "button", 1, 0x20);
  c.add_control(pid, "beatsync",          "[Channel1]", "button", 1, 0x40);
  c.add_control(pid, "beatlock",          "[Channel1]", "button", 1, 0x80);
  c.add_control(pid, "previous",          "[Channel1]", "button", 2, 0x01);
  c.add_control(pid, "next",              "[Channel1]", "button", 2, 0x02);
  c.add_control(pid, "play",              "[Channel1]", "button", 2, 0x04);
  c.add_control(pid, "cue_default",       "[Channel1]", "button", 2, 0x08);
  c.add_control(pid, "stop",              "[Channel1]", "button", 2, 0x10);
  c.add_control(pid, "filterHighKill",    "[Channel1]", "button", 2, 0x20);
  c.add_control(pid, "filterMidKill",     "[Channel1]", "button", 2, 0x40);
  c.add_control(pid, "filterLowKill",     "[Channel1]", "button", 2, 0x80);
  c.add_control(pid, "pitch_reset",       "[Channel1]", "button", 3, 0x01);
  c.add_control(pid, "LoadSelectedTrack", "[Channel1]", "button", 3, 0x02);
  c.add_control(pid, "source",            "[Channel1]", "button", 3, 0x04);
  c.add_control(pid, "headphone_cue",     "[Channel1]", "button", 3, 0x08);

  // deck 2 - buttons
  c.add_control(pid, "keypad1",           "[Channel2]", "button", 4, 0x01);
  c.add_control(pid, "keypad2",           "[Channel2]", "button", 4, 0x02);
  c.add_control(pid, "keypad3",           "[Channel2]", "button", 4, 0x04);
  c.add_control(pid, "keypad4",           "[Channel2]", "button", 4, 0x08);
  c.add_control(pid, "keypad5",           "[Channel2]", "button", 4, 0x10);
  c.add_control(pid, "keypad6",           "[Channel2]", "button", 4, 0x20);
  c.add_control(pid, "beatsync",          "[Channel2]", "button", 4, 0x40);
  c.add_control(pid, "beatlock",          "[Channel2]", "button", 3, 0x10);
  c.add_control(pid, "previous",          "[Channel2]", "button", 5, 0x01);
  c.add_control(pid, "next",              "[Channel2]", "button", 5, 0x02);
  c.add_control(pid, "play",              "[Channel2]", "button", 5, 0x04);
  c.add_control(pid, "cue_default",       "[Channel2]", "button", 5, 0x08);
  c.add_control(pid, "stop",              "[Channel2]", "button", 5, 0x10);
  c.add_control(pid, "filterHighKill",    "[Channel2]", "button", 5, 0x20);
  c.add_control(pid, "filterMidKill",     "[Channel2]", "button", 5, 0x40);
  c.add_control(pid, "filterLowKill",     "[Channel2]", "button", 5, 0x80);
  c.add_control(pid, "pitch_reset",       "[Channel2]", "button", 4, 0x80);
  c.add_control(pid, "LoadSelectedTrack", "[Channel2]", "button", 3, 0x20);
  c.add_control(pid, "source",            "[Channel2]", "button", 3, 0x40);
  c.add_control(pid, "headphone_cue",     "[Channel2]", "button", 3, 0x80);

  // master buttons
  c.add_control(pid, "scratch",           "[Master]",   "button", 6, 0x01);
  c.add_control(pid, "menu_up",           "[Master]",   "button", 6, 0x02);
  c.add_control(pid, "menu_down",         "[Master]",   "button", 6, 0x04);
  c.add_control(pid, "menu_left",         "[Master]",   "button", 6, 0x08);
  c.add_control(pid, "menu_right",        "[Master]",   "button", 6, 0x10);
  c.add_control(pid, "mic_toggle",        "[Master]",   "button", 6, 0x20);

  // wheels
  c.add_control(pid, "jog",               "[Channel1]", "encoder", 7, 0xff);
  c.add_control(pid, "jog",               "[Channel2]", "encoder", 8, 0xff);

  // faders
  c.add_control(pid, "rate",              "[Channel1]", "fader", 9,  0xff);
  c.add_control(pid, "volume",            "[Channel1]", "fader", 10, 0xff);
  c.add_control(pid, "pregain",           "[Channel1]", "fader", 11, 0xff);
  c.add_control(pid, "filterHigh",        "[Channel1]", "fader", 12, 0xff);
  c.add_control(pid, "filterMid",         "[Channel1]", "fader", 13, 0xff);
  c.add_control(pid, "filterLow",         "[Channel1]", "fader", 14, 0xff);

  c.add_control(pid, "balance",           "[Master]",   "fader", 15, 0xff);
  c.add_control(pid, "volume",            "[Master]",   "fader", 16, 0xff);
  c.add_control(pid, "crossfader",        "[Master]",   "fader", 17, 0xff);
  c.add_control(pid, "headMix",           "[Master]",   "fader", 18, 0xff);

  c.add_control(pid, "rate",              "[Channel2]", "fader", 19, 0xff);
  c.add_control(pid, "volume",            "[Channel2]", "fader", 20, 0xff);
  c.add_control(pid, "pregain",           "[Channel2]", "fader", 21, 0xff);
  c.add_control(pid, "filterHigh",        "[Channel2]", "fader", 22, 0xff);
  c.add_control(pid, "filterMid",         "[Channel2]", "fader", 23, 0xff);
  c.add_control(pid, "filterLow",         "[Channel2]", "fader", 24, 0xff);


  // define led feedback

  pid = 0x00;
  c.cache_out[pid] = [ pid, 0x0, 0x0, 0x0 ];

  // Last argument (hex) selects which LED, the integer before selects deck - figured out by RichterSkala
  // But I (Jesse) can't figure out how to make rhem blink! Says it's supported by controller's MIDI implementation...
  // but HID != MIDI
  //
  // Forum user 'neale' suggests 'try twiddling the value you send to the control and the control number',
  // but I can't figure out how to do that here.
  //
  // However, I switched to "play_indicator" and "cue_indicator" instead of "play" and "cue_default" respectively,
  // and original blinking play button issue appears to be fixed...there is now some response to changing cue mode in Mixxx prefs
  // - Jesse

  c.add_control(pid, "scratch",       "[Master]",   "led", 1, 0x01); 
  c.add_control(pid, "play_indicator","[Channel1]", "led", 1, 0x02); 
  c.add_control(pid, "cue_indicator", "[Channel1]", "led", 1, 0x04);
  c.add_control(pid, "headphone_cue", "[Channel1]", "led", 1, 0x08);
  c.add_control(pid, "source",        "[Channel1]", "led", 1, 0x10);
  c.add_control(pid, "beatsync",      "[Channel1]", "led", 1, 0x20);
  c.add_control(pid, "beatlock",      "[Channel1]", "led", 1, 0x40);
  c.add_control(pid, "pitch_reset",   "[Channel1]", "led", 1, 0x80);

  c.add_control(pid, "play_indicator","[Channel2]", "led", 2, 0x02);
  c.add_control(pid, "cue_indicator", "[Channel2]", "led", 2, 0x04);
  c.add_control(pid, "headphone_cue", "[Channel2]", "led", 2, 0x08);
  c.add_control(pid, "source",        "[Channel2]", "led", 2, 0x10);
  c.add_control(pid, "beatsync",      "[Channel2]", "led", 2, 0x20);
  c.add_control(pid, "beatlock",      "[Channel2]", "led", 2, 0x80);
  c.add_control(pid, "pitch_reset",   "[Channel2]", "led", 2, 0x40);

};

/**
 * non-specific controller framework to allow hid packets to be defined and
 * processed via callback functions - could/should be in a shared file
 */

RMX.add_control = function(packetid, name, group, type, offset, mask) {
  if (type == "led") {
    RMX.leds[group + name] =
      new RMX.control(packetid, name, group, type, offset, mask);
  }
  else {
    if (RMX.controls[offset] === undefined) {
      RMX.controls[offset] = [];
    }
    RMX.controls[offset].push(
      new RMX.control(packetid, name, group, type, offset, mask)
    );
  }
};

// bind a function to a modified controller value

RMX.capture = function(name, values, func) {
  if (RMX.callbacks[name] === undefined) {
    RMX.callbacks[name] = [ [ values, func ] ];
  }
  else {
    RMX.callbacks[name].push([ values, func ]);
  }
};

// bind a function to feedback from mixxx, callbacks accept args in same order
// as from capture()

RMX.feedback = function(g, e, f) {
  engine.makeConnection(g, e, "RMX.feedbackData");
  if (RMX.feedbacks[g + e] === undefined) {
    RMX.feedbacks[g + e] = [];
  }
  RMX.feedbacks[g + e].push(f);
};

// controller feedback: send data to the controller by name and automatically
// send out the full hid packet needed

RMX.send = function(g, e, v) {
  if ((ctrl = this.leds[g + e]) !== undefined) {

    // for the byte in the hid packet that this led control affects, mask out
    // it's old value
    // and then add in it's new one

    var tmp = this.cache_out[ctrl.packetid];

    tmp[ctrl.offset] = tmp[ctrl.offset] & ctrl.maskinv | (v << ctrl.bitshift);

    // send complete hid packet
    controller.send(tmp, tmp.length, 0);
  }
};

// process incoming data from mixxx and call any callbacks

RMX.feedbackData = function(v, g, e) {
  if (RMX.feedbacks[g + e] !== undefined) {
    for (var func in RMX.feedbacks[g + e]) {
      if (typeof(RMX.feedbacks[g + e][func]) == "function") {
        RMX.feedbacks[g + e][func](g, e, v);
      }
    }
  }
};

// a single hid control, store last known value and offset/mask to work out the
// new value from incoming data

RMX.control = function(packetid, name, group, type, offset, mask) {
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
  while (mask !== 0 && (mask & 0x1) === 0) {
    mask = mask >> 1;
    this.bitshift++;
  }
};

// process incoming data and call any callbacks if their bound controls have
// changed

RMX.incomingData = function (data, length) {

  var c = RMX;
  var packetid = data[0];
  var p = c.cache_in[packetid];

  // iterate thru each byte and only check controls for that byte if the byte
  // has changed
  for (var i=1; i<length; i++) {

    if ((p === undefined || data[i] != p[i]) && c.controls[i] !== undefined) {

      // a byte has changed, check any controls defined in that byte, more
      // efficient than checking old+new values for all controls

      for (var key in c.controls[i]) {
        var control = c.controls[i][key];
        if (typeof(control) == 'object' &&
            control.packetid == data[0] &&
            control.changed(data[i]) ) {

          // we found a hid control that has changed value within that byte,
          // check for callbacks
          var callbacks = c.callbacks[control.name];

          if (callbacks !== undefined) {
            for (var j=0; j<callbacks.length; j++) {

              var cb = callbacks[j][1];

              if (typeof(cb) == 'function') {

                // check we need to call for this value change:
                // all, press, release
                var v       = callbacks[j][0];
                var all     = v == "all";
                var press   = v == "press"   && control.value  >  0;
                var release = v == "release" && control.value === 0;
                if ( all || press || release ) {
                  // call a callback function for this control
                  cb(control.group, control.name, control.value, control);
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
};

/****************************************************************/
/*      Hercules DJ Console RMX HID controller script           */
/*      For Mixxx version 1.11                                  */
/*      Author: RichterSkala                                    */
/*      Based on zestoi's script                                */
/****************************************************************/

RMX = new Controller();

RMX.controls  = [];
RMX.leds      = [];
RMX.cache_in  = [];
RMX.cache_out = [];
RMX.callbacks = [];
RMX.feedbacks = [];

//RMX.loop_lengths = [ 0.25, 0.5, 1 ]; // edit to loop size needed

//State variables:
RMX.scratch_enabled = false;
RMX.scratching =  { "[Channel1]": false, "[Channel2]": false};
//RMX.beatlock_tmp =  { "[Channel1]": false, "[Channel2]": false};
RMX.jog_skip =  { "[Channel1]": true, "[Channel2]": true };
RMX.shift = false; // controlled via stop button
RMX.caps = false; // extra shift, toggled via mic button.
//RMX.breaks = { "[Channel1]": false, "[Channel2]": false};
RMX.select = { "[Channel1]": false, "[Channel2]": false}; // extra extra shift per dec, toggled via source button.

//Config:
RMX.eq_max = 4; //eq max value. Full range =4, minimal value (=linear) =2

//
// the actual mapping is defined in this function
//

RMX.init = function() {

  var c = RMX;

  //
  // define the hid packet
  //

  c.define_hid_format();

  //
  // create the actual mapping
  // deck controls (will work for any decks as the group is passed in)
  //

  c.capture("crossfader", "all", function(g, e, v) { engine.setValue(g, e, (v - 128) / 128); });

  c.capture("balance", "all", function(g,e,v) { engine.setValue(g,e,(v-128)/128);});

  // TO DO: Brake on shift + play ... or find a better button.
  c.capture("play", "press", function(g, e, v) { engine.setValue(g, e, !engine.getValue(g, e)); });

  c.capture("cue_default", "all", function(g, e, v) { engine.setValue(g, e, v); });

  //TO DO: Think about adapting to master tempo for 1.12
  c.capture("beatsync", "all", function(g, e, v) { engine.setValue(g, e, v); });

  c.capture("volume", "all", function(g, e, v) { engine.setValue(g, e, v / 256); });

  c.capture("pregain", "all", c.filter_scale); // gain & filters go from 0..1..4 in mixxx, scale in fct.

  //TO DO: Think about adapting to effects branch, or at least flanger.
  c.capture("filterHigh", "all", c.filter_scale);
  c.capture("filterMid", "all", c.filter_scale);
  c.capture("filterLow", "all", c.filter_scale);

  // Handle Jog-wheel in own function
  c.capture("jog", "all", c.jog);

  // stop and mic toggle for shift
  c.capture("stop", "all", function(g,e,v) { RMX.shift = v > 0? 1 : 0; });
  c.capture("mic_toggle", "all", function(g,e,v) { RMX.caps = v > 0? 1:0; });
    //mic button is toggled in hardware: value doesn't change on release
  // source toggle for shift
  c.capture("source","press", function(g,e,v) {
    RMX.select[g] = !RMX.select[g];
    c.send(g, e, RMX.select[g] ? 1:0);
  });

  c.capture("previous", "all", function(g, e, v) {
      engine.setValue(g, "back", v > 0 ? 1 : 0);
    }
  );

  c.capture("next", "all", function(g, e, v) {
      engine.setValue(g, "fwd", v > 0 ? 1 : 0);

  });


  //
  // track browsing
  // TO DO: Maybe use shift for fx selection?
  //

  c.capture("menu_up", "all", c.scroll_tracks);
  c.capture("menu_down", "all", c.scroll_tracks);
  c.capture("menu_left", "all", function(g, e, v) { engine.setValue("[Playlist]", "SelectPrevPlaylist", v);});
  c.capture("menu_right", "all", function(g,e,v) {engine.setValue("[Playlist]", "SelectNextPlaylist",v);});
  c.capture("LoadSelectedTrack", "press", function(g, e, v) { engine.setValue(g, e, 1); });


  //
  // fader
  //

  c.capture("rate", "all", function(g, e, v) {
    engine.setValue(g, e, (v-127)/128); // TO DO: Figure out if we can still use full range and not lose information at top.
    // TO DO: Reimplement rate magnification.
  });

  //
  // enable/disable scratching
  //
  c.capture("scratch", "press", function(g, e, v) {
    RMX.scratch_enabled = !RMX.scratch_enabled;
     c.send(g, e, RMX.scratch_enabled ? 1: 0);
  });

  c.capture("beatlock", "press", function(g, e, v) {  engine.setValue(g,"keylock",!engine.getValue(g, "keylock"));});
  c.capture("pitch_reset", "press", function(g,e,v) { engine.setValue(g,"rate",0)});


  //
  // headphone cue
  //

  c.capture("headphone_cue", "press", function(g, e, v) { engine.setValue(g, "pfl", !engine.getValue(g, "pfl")); });
  c.capture("headMix", "all", function(g,e,v) { engine.setValue(g, e, (v-128)/128); });


  c.capture("keypad1","all", function(g,e,v) { print("Unhandled button");});
  c.capture("keypad2","all", function(g,e,v) { print("Unhandled button");});
  c.capture("keypad3","all", function(g,e,v) { print("Unhandled button");});
  c.capture("keypad4","all", function(g,e,v) { print("Unhandled button");});
  c.capture("keypad5","all", function(g,e,v) { print("Unhandled button");});
  c.capture("keypad6","all", function(g,e,v) { print("Unhandled button");});

  c.capture("filterHighKill","press", function(g,e,v) { engine.setValue(g, e, !engine.getValue(g, e));});
  c.capture("filterMidKill","press", function(g,e,v) { engine.setValue(g, e, !engine.getValue(g, e));});
  c.capture("filterLowKill","press", function(g,e,v) { engine.setValue(g, e, !engine.getValue(g, e));});




  //
  // led feedback
  //

  c.feedback("[Channel1]", "play", function(g, e, v) { c.send(g, e, v); });
  c.feedback("[Channel2]", "play", function(g, e, v) { c.send(g, e, v); });
  c.feedback("[Channel1]", "cue_default", function(g, e, v) { c.send(g, e, v); });
  c.feedback("[Channel2]", "cue_default", function(g, e, v) { c.send(g, e, v); });
  c.feedback("[Channel1]", "beatsync", function(g, e, v) { c.send(g, e, v); });
  c.feedback("[Channel2]", "beatsync", function(g, e, v) { c.send(g, e, v); });
  c.feedback("[Channel1]", "keylock", function(g, e, v) { c.send(g, "beatlock", v); });
  c.feedback("[Channel2]", "keylock", function(g, e, v) { c.send(g, "beatlock", v); });
  c.feedback("[Channel1]", "pfl", function(g, e, v) { c.send(g, "headphone_cue", v); });
  c.feedback("[Channel2]", "pfl", function(g, e, v) { c.send(g, "headphone_cue", v); });


  //
  // clear/setup any initial leds
  //
  /*
  for (id in c.leds) {
    c.send(c.leds[id].group, c.leds[id].name, 0);
  }
  */

}

//
// playlist scroll nex/previous with auto-repeat when held
//

RMX.scroll_tracks = function(g, e, v) {
  if (v > 0) {
    engine.setValue("[Playlist]", e == "menu_down" ? "SelectNextTrack" : "SelectPrevTrack", 1);
    if (!RMX.scroll_timer) {
      RMX.scroll_timer = engine.beginTimer(150, 'RMX.scroll_tracks("[Playlist]","' + e + '",' + v + ')');
    }
  }
  else {
    if (RMX.scroll_timer) {
      engine.stopTimer(RMX.scroll_timer);
      RMX.scroll_timer = null;
    }
  }
}

//
// scale the knobs according to RMX.eq_max
//
RMX.filter_scale = function(g, e, v) {
  if (v>128) {
    engine.setValue(g, e, (RMX.eq_max-1) * ((v / 128)-1)+1);
  } else {
    engine.setValue(g, e, v /128);
  }

}

//
// jog
//
RMX.jog = function(g, e, v, ctrl) {
    // skip initial jog values
    if (RMX.jog_skip[g]) {
      RMX.jog_skip[g] = false;
      return;
    }

    // scratch mode
    if (RMX.scratch_enabled) {
      if (!RMX.scratching[g]) {
              RMX.scratching[g] = true;
              var ScratchRPM
              if(engine.getValue(g,"play")) {
                ScratchRPM = 45;
              } else {
                ScratchRPM = 80;
              }
        engine.scratchEnable(parseInt(g.substring(8,9)), 128, ScratchRPM, 1.0/8, (1.0/8)/32);         //Don't make any sound when trimming loops
          } else {
            engine.stopTimer(RMX.scratchTimer);
          }
          engine.scratchTick(parseInt(g.substring(8,9)), ctrl.relative);

         RMX.scratchTimer = engine.beginTimer(20, 'RMX.stopScratching(\"' + g + '\")', true);

    }

    // fine jog mode when playing
    else if (engine.getValue(g, "play")) {
      engine.setValue(g, e, ctrl.relative/2);
    }

    // track browsing when shift held (sync) and not playing
    else if (RMX.shift) {
      engine.setValue("[Playlist]", "SelectTrackKnob", ctrl.relative);
    }

    // normal jog mode when not playing
    else {
      engine.setValue(g, e, ctrl.relative);
    }
  }

RMX.stopScratching = function(g) {
  RMX.scratching[g] = false;
  engine.scratchDisable(parseInt(g.substring(8,9)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// define the hid packet to event mapping, could be defined via xml so can be used in multiple mappings
// naming the controls as much as possible inline with the mixxx engine names makes most mappings trivial
//

RMX.define_hid_format = function() {

  var c = RMX;
  var pid = 0x1;
  // Order as in manual
  // deck 1 - buttons
  c.add_control(pid, "keypad1", "[Channel1]", "button", 1, 0x01);
  c.add_control(pid, "keypad2", "[Channel1]", "button", 1, 0x02);
  c.add_control(pid, "keypad3", "[Channel1]", "button", 1, 0x04);
  c.add_control(pid, "keypad4", "[Channel1]", "button", 1, 0x08);
  c.add_control(pid, "keypad5", "[Channel1]", "button", 1, 0x10);
  c.add_control(pid, "keypad6", "[Channel1]", "button", 1, 0x20);
  c.add_control(pid, "beatsync", "[Channel1]", "button", 1, 0x40);
  c.add_control(pid, "beatlock", "[Channel1]", "button", 1, 0x80);
  c.add_control(pid, "previous", "[Channel1]", "button", 2, 0x01);
  c.add_control(pid, "next", "[Channel1]", "button", 2, 0x02);
  c.add_control(pid, "play", "[Channel1]", "button", 2, 0x04);
  c.add_control(pid, "cue_default", "[Channel1]", "button", 2, 0x08);
  c.add_control(pid, "stop", "[Channel1]", "button", 2, 0x10);
  c.add_control(pid, "filterHighKill", "[Channel1]", "button", 2, 0x20);
  c.add_control(pid, "filterMidKill", "[Channel1]", "button", 2, 0x40);
  c.add_control(pid, "filterLowKill", "[Channel1]", "button", 2, 0x80);
  c.add_control(pid, "pitch_reset", "[Channel1]", "button", 3, 0x01);
  c.add_control(pid, "LoadSelectedTrack", "[Channel1]", "button", 3, 0x02);
  c.add_control(pid, "source", "[Channel1]", "button", 3, 0x04);
  c.add_control(pid, "headphone_cue", "[Channel1]", "button", 3, 0x08);

  // deck 2 - buttons
  c.add_control(pid, "beatlock", "[Channel2]", "button", 3, 0x10);
  c.add_control(pid, "LoadSelectedTrack", "[Channel2]", "button", 3, 0x20);
  c.add_control(pid, "source", "[Channel2]", "button", 3, 0x40);
  c.add_control(pid, "headphone_cue", "[Channel2]", "button", 3, 0x80);
  c.add_control(pid, "keypad1", "[Channel2]", "button", 4, 0x01);
  c.add_control(pid, "keypad2", "[Channel2]", "button", 4, 0x02);
  c.add_control(pid, "keypad3", "[Channel2]", "button", 4, 0x04);
  c.add_control(pid, "keypad4", "[Channel2]", "button", 4, 0x08);
  c.add_control(pid, "keypad5", "[Channel2]", "button", 4, 0x10);
  c.add_control(pid, "keypad6", "[Channel2]", "button", 4, 0x20);
  c.add_control(pid, "beatsync", "[Channel2]", "button", 4, 0x40);
  c.add_control(pid, "pitch_reset", "[Channel2]", "button", 4, 0x80);
  c.add_control(pid, "previous", "[Channel2]", "button", 5, 0x01);
  c.add_control(pid, "next", "[Channel2]", "button", 5, 0x02);
  c.add_control(pid, "play", "[Channel2]", "button", 5, 0x04);
  c.add_control(pid, "cue_default", "[Channel2]", "button", 5, 0x08);
  c.add_control(pid, "stop", "[Channel2]", "button", 5, 0x10);
  c.add_control(pid, "filterHighKill", "[Channel2]", "button", 5, 0x20);
  c.add_control(pid, "filterMidKill", "[Channel2]", "button", 5, 0x40);
  c.add_control(pid, "filterLowKill", "[Channel2]", "button", 5, 0x80);

  // master buttons
  c.add_control(pid, "scratch", "[Master]", "button", 6, 0x01);
  c.add_control(pid, "menu_up", "[Master]", "button", 6, 0x02);
  c.add_control(pid, "menu_down", "[Master]", "button", 6, 0x04);
  c.add_control(pid, "menu_left", "[Master]", "button", 6, 0x08);
  c.add_control(pid, "menu_right", "[Master]", "button", 6, 0x10);
  c.add_control(pid, "mic_toggle", "[Master]", "button", 6, 0x20);

  // wheels
  c.add_control(pid, "jog", "[Channel1]", "encoder", 7, 0xff);
  c.add_control(pid, "jog", "[Channel2]", "encoder", 8, 0xff);

  // faders
  c.add_control(pid, "rate", "[Channel1]", "fader", 9, 0xff);
  c.add_control(pid, "volume", "[Channel1]", "fader", 10, 0xff);
  c.add_control(pid, "pregain", "[Channel1]", "fader", 11, 0xff);
  c.add_control(pid, "filterHigh", "[Channel1]", "fader", 12, 0xff);
  c.add_control(pid, "filterMid", "[Channel1]", "fader", 13, 0xff);
  c.add_control(pid, "filterLow", "[Channel1]", "fader", 14, 0xff);

  c.add_control(pid, "balance", "[Master]", "fader", 15, 0xff);
  c.add_control(pid, "volume", "[Master]", "fader", 16, 0xff);
  c.add_control(pid, "crossfader", "[Master]", "fader", 17, 0xff);
  c.add_control(pid, "headMix", "[Master]", "fader", 18, 0xff);

  c.add_control(pid, "rate", "[Channel2]", "fader", 19, 0xff);
  c.add_control(pid, "volume", "[Channel2]", "fader", 20, 0xff);
  c.add_control(pid, "pregain", "[Channel2]", "fader", 21, 0xff);
  c.add_control(pid, "filterHigh", "[Channel2]", "fader", 22, 0xff);
  c.add_control(pid, "filterMid", "[Channel2]", "fader", 23, 0xff);
  c.add_control(pid, "filterLow", "[Channel2]", "fader", 24, 0xff);


  // define led feedback

  pid = 0x00;
  c.cache_out[pid] = [ pid, 0x0, 0x0, 0x0 ];


  c.add_control(pid, "scratch", "[Master]", "led", 1, 0x01); // blinking: 3, 0x2
  c.add_control(pid, "play", "[Channel1]", "led", 1, 0x02); // blinking: 3, 0x2
  c.add_control(pid, "cue_default", "[Channel1]", "led", 1, 0x04);
  c.add_control(pid, "headphone_cue", "[Channel1]", "led", 1, 0x08);
  c.add_control(pid, "source", "[Channel1]", "led", 1, 0x10);
  c.add_control(pid, "beatsync", "[Channel1]", "led", 1, 0x20);
  c.add_control(pid, "beatlock", "[Channel1]", "led", 1, 0x40);
  c.add_control(pid, "pitch_reset", "[Channel1]", "led", 1, 0x80);

  c.add_control(pid, "play", "[Channel2]", "led", 2, 0x02); // blinking: 2, 0x02
  c.add_control(pid, "cue_default", "[Channel2]", "led", 2, 0x04); // blinking 2, 0x04
  c.add_control(pid, "headphone_cue", "[Channel2]", "led", 2, 0x08);  //blinking 2, 0x08
  c.add_control(pid, "source", "[Channel2]", "led", 2, 0x10);
  c.add_control(pid, "beatsync", "[Channel2]", "led", 2, 0x20); // blinking 2, 0x20
  c.add_control(pid, "beatlock", "[Channel2]", "led", 2, 0x80); // blinking 2, 0x80
  c.add_control(pid, "pitch_reset", "[Channel2]", "led", 2, 0x40);  //blinking 2, 0x40

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// non-specific controller framework to allow hid packets to be defined and processed via
// callback functions - could/should be in a shared file
//

RMX.add_control = function(packetid, name, group, type, offset, mask) {
  if (type == "led") {
    RMX.leds[group + name] = new RMX.control(packetid, name, group, type, offset, mask);
  }
  else {
    if (RMX.controls[offset] == undefined) {
      RMX.controls[offset] = [];
    }
    RMX.controls[offset].push(new RMX.control(packetid, name, group, type, offset, mask));
  }
}

//
// bind a function to a modified controller value
//

RMX.capture = function(name, values, func) {
  if (RMX.callbacks[name] == undefined) {
    RMX.callbacks[name] = [ [ values, func ] ];
  }
  else {
    RMX.callbacks[name].push([ values, func ]);
  }
}

//
// bind a function to feedback from mixxx, callbacks accept args in same order as from capture()
//

RMX.feedback = function(g, e, f) {
  engine.connectControl(g, e, "RMX.feedbackData");
  if (RMX.feedbacks[g + e] == undefined) {
    RMX.feedbacks[g + e] = [];
  }
  RMX.feedbacks[g + e].push(f);
}

//
// controller feedback: send data to the controller by name and automatically send out the full hid packet needed
//

RMX.send = function(g, e, v) {
  if ((ctrl = this.leds[g + e]) != undefined) {

    //
    // for the byte in the hid packet that this led control affects, mask out it's old value
    // and then add in it's new one
    //

    this.cache_out[ctrl.packetid][ctrl.offset] = this.cache_out[ctrl.packetid][ctrl.offset] & ctrl.maskinv | (v << ctrl.bitshift);

    //
    // send complete hid packet and update our cache
    //

    controller.send(this.cache_out[ctrl.packetid], this.cache_out[ctrl.packetid].length, 0);
    this.cache_out[ctrl.packetid] = this.cache_out[ctrl.packetid];
  }
}

//
// process incoming data from mixxx and call any callbacks
//

RMX.feedbackData = function(v, g, e) {
  if (RMX.feedbacks[g + e] != undefined) {
    for (func in RMX.feedbacks[g + e]) {
      if (typeof(RMX.feedbacks[g + e][func]) == "function") {
        RMX.feedbacks[g + e][func](g, e, v);
      }
    }
  }
}

//
// a single hid control, store last known value and offset/mask to work out the new value from incoming data
//

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
  while (mask != 0 && (mask & 0x1) == 0) {
    mask = mask >> 1;
    this.bitshift++;
  }
}

//
// process incoming data and call any callbacks if their bound controls have changed
//

RMX.incomingData = function (data, length) {

  var c = RMX;
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

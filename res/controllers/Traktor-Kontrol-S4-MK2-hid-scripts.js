// TODO:
// fx knobs
// snap / master / quant buttons?
// ******lights******

TraktorS4MK2 = new function() {
   this.controller = new HIDController();
   this.partial_packet = Object();
   this.divisor_map = Object();
   this.controller.left_deck_C = false;
   this.controller.right_deck_D = false;
   this.controller.prev_pregain = {"[Channel1]" : 0,
                                   "[Channel2]" : 0,
                                   "[Channel3]" : 0,
                                   "[Channel4]" : 0};
   this.controller.prev_browse = 0;
   // last tick times for the left and right wheels.
   this.controller.last_tick_val = [0, 0];
   this.controller.last_tick_time = [0.0, 0.0];
}

TraktorS4MK2.callbackPregain = function(field) {
  // TODO: common-hid-packet-parser looks like it should do deltas, but I can't get them to work.
  prev_pregain = TraktorS4MK2.controller.prev_pregain[field.group];
  TraktorS4MK2.controller.prev_pregain[field.group] = field.value;
  var delta = 0;
  if (prev_pregain === 15 && field.value === 0) {
    delta = 0.05;
  } else if (prev_pregain === 0 && field.value === 15) {
    delta = -0.05;
  } else if (field.value > prev_pregain) {
    delta = 0.05;
  } else {
    delta = -0.05;
  }

  var cur_pregain = engine.getValue(group, "pregain");
  engine.setValue(group, "pregain", cur_pregain + delta);
}

TraktorS4MK2.callbackBrowse = function(field) {
  // TODO: common-hid-packet-parser looks like it should do deltas, but I can't get them to work.
  prev_browse = TraktorS4MK2.controller.prev_browse;
  TraktorS4MK2.controller.prev_browse = field.value;
  var delta = 0;
  if (prev_browse === 15 && field.value === 0) {
    delta = 1;
  } else if (prev_browse === 0 && field.value === 15) {
    delta = -1;
  } else if (field.value > prev_browse) {
    delta = 1;
  } else {
    delta = -1;
  }

  engine.setValue("[Playlist]", "SelectTrackKnob", delta);
}

TraktorS4MK2.scalerEq = function(group, name, value) {
  return script.absoluteNonLin(value, 0, 1, 4, 16, 4080);
}

TraktorS4MK2.scalerVolume = function(group, name, value) {
  if (group === "[Master]") {
    return script.absoluteNonLin(value, 0, 1, 4, 16, 4080);
  } else {
    return script.absoluteNonLin(value, 0, 0.25, 1, 16, 4080);
  }
}

TraktorS4MK2.scalerQuickKnob = function(group, name, value) {
  return script.absoluteLin(value, 0, 1, 16, 4080);
}

TraktorS4MK2.scalerSlider = function(group, name, value) {
  return script.absoluteLin(value, -1, 1, 16, 4080);
}

TraktorS4MK2.registerInputPackets = function() {
  MessageShort = new HIDPacket("shortmessage",[0x01],21, this.shortMessageCallback);
  MessageLong = new HIDPacket("longmessage",[0x02],79, this.longMessageCallback);

  // Values in the short message are all buttons.
  MessageShort.addControl("buttons_left", "!shift", 0x0D, "B", 0x08);
  MessageShort.addControl("buttons_left", "sync_enabled", 0x0D, "B", 0x04);
  MessageShort.addControl("buttons_left", "cue_default", 0x0D, "B", 0x02);
  MessageShort.addControl("buttons_left", "play", 0x0D, "B", 0x01);
  MessageShort.addControl("buttons_left", "hotcue_1_activate", 0x0D, "B", 0x80);
  MessageShort.addControl("buttons_left", "hotcue_2_activate", 0x0D, "B", 0x40);
  MessageShort.addControl("buttons_left", "hotcue_3_activate", 0x0D, "B", 0x20);
  MessageShort.addControl("buttons_left", "hotcue_4_activate", 0x0D, "B", 0x10);
  MessageShort.addControl("buttons_left", "!play1", 0x0E, "B", 0x80);
  MessageShort.addControl("buttons_left", "!play2", 0x0E, "B", 0x40);
  MessageShort.addControl("buttons_left", "!play3", 0x0E, "B", 0x20);
  MessageShort.addControl("buttons_left", "!play4", 0x0E, "B", 0x10);
  MessageShort.addControl("buttons_left", "loop_out", 0x0E, "B", 0x08);
  MessageShort.addControl("buttons_left", "loop_in", 0x0E, "B", 0x04);
  MessageShort.addControl("buttons_left", "!flux", 0x0E, "B", 0x02);
  MessageShort.addControl("buttons_left", "!reset", 0x0E, "B", 0x01);
  MessageShort.addControl("buttons_left", "!loopsize", 0x13, "B", 0x02);
  MessageShort.addControl("buttons_left", "!loopmove", 0x13, "B", 0x01);
  MessageShort.addControl("buttons_left", "!wheeltouch", 0x11, "B", 0x01);
  MessageShort.addControl("buttons_left", "jog_wheel", 0x01, "I");
  MessageShort.addControl("buttons_left", "!deckswitch", 0x0F, "B", 0x20);
  MessageShort.addControl("buttons_left", "LoadSelectedTrack", 0x0F, "B", 0x10);
  MessageShort.addControl("buttons_left", "!FX1", 0x12, "B", 0x80);
  MessageShort.addControl("buttons_left", "!FX2", 0x12, "B", 0x40);
  MessageShort.addControl("buttons_left", "!FX3", 0x12, "B", 0x20);
  MessageShort.addControl("buttons_left", "!FX4", 0x12, "B", 0x10);
  MessageShort.addControl("buttons_left", "!FXMode", 0x11, "B", 0x08);

  MessageShort.addControl("buttons_right", "!shift", 0x0C, "B", 0x08);
  MessageShort.addControl("buttons_right", "sync_enabled", 0x0C, "B", 0x04);
  MessageShort.addControl("buttons_right", "cue_default", 0x0C, "B", 0x02);
  MessageShort.addControl("buttons_right", "play", 0x0C, "B", 0x01);
  MessageShort.addControl("buttons_right", "hotcue_1_activate", 0x0C, "B", 0x80);
  MessageShort.addControl("buttons_right", "hotcue_2_activate", 0x0C, "B", 0x40);
  MessageShort.addControl("buttons_right", "hotcue_3_activate", 0x0C, "B", 0x20);
  MessageShort.addControl("buttons_right", "hotcue_4_activate", 0x0C, "B", 0x10);
  MessageShort.addControl("buttons_right", "!play1", 0x0B, "B", 0x80);
  MessageShort.addControl("buttons_right", "!play2", 0x0B, "B", 0x40);
  MessageShort.addControl("buttons_right", "!play3", 0x0B, "B", 0x20);
  MessageShort.addControl("buttons_right", "!play4", 0x0B, "B", 0x10);
  MessageShort.addControl("buttons_right", "loop_out", 0x0B, "B", 0x08);
  MessageShort.addControl("buttons_right", "loop_in", 0x0B, "B", 0x04);
  MessageShort.addControl("buttons_right", "!flux", 0x0B, "B", 0x02);
  MessageShort.addControl("buttons_right", "!reset", 0x0B, "B", 0x01);
  MessageShort.addControl("buttons_right", "!loopsize", 0x13, "B", 0x10);
  MessageShort.addControl("buttons_right", "!loopmove", 0x13, "B", 0x08);
  MessageShort.addControl("buttons_right", "!wheeltouch", 0x11, "B", 0x02);
  MessageShort.addControl("buttons_right", "jog_wheel", 0x05, "I");
  MessageShort.addControl("buttons_right", "!deckswitch", 0x0A, "B", 0x20);
  MessageShort.addControl("buttons_right", "LoadSelectedTrack", 0x0A, "B", 0x10);
  MessageShort.addControl("buttons_right", "!FX1", 0x10, "B", 0x08);
  MessageShort.addControl("buttons_right", "!FX2", 0x10, "B", 0x04);
  MessageShort.addControl("buttons_right", "!FX3", 0x10, "B", 0x02);
  MessageShort.addControl("buttons_right", "!FX4", 0x10, "B", 0x01);
  MessageShort.addControl("buttons_right", "!FXMode", 0x11, "B", 0x04);

  MessageShort.addControl("[Channel1]", "pfl", 0x0F, "B", 0x40);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel1]_enable", 0x12, "B", 0x02);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel1]_enable", 0x12, "B", 0x01);
  MessageShort.addControl("[Channel1]", "pregain_set_default", 0x11, "B", 0x20);

  MessageShort.addControl("[Channel2]", "pfl", 0x0A, "B", 0x40);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel2]_enable", 0x10, "B", 0x80);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel2]_enable", 0x10, "B", 0x40);
  MessageShort.addControl("[Channel2]", "pregain_set_default", 0x11, "B", 0x40);

  MessageShort.addControl("[Channel3]", "pfl", 0x0F, "B", 0x80);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel3]_enable", 0x12, "B", 0x08);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel3]_enable", 0x12, "B", 0x04);
  MessageShort.addControl("[Channel3]", "pregain_set_default", 0x11, "B", 0x10);

  MessageShort.addControl("[Channel4]", "pfl", 0x0A, "B", 0x80);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel4]_enable", 0x10, "B", 0x20);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel4]_enable", 0x10, "B", 0x10);
  MessageShort.addControl("[Channel4]", "pregain_set_default", 0x11, "B", 0x80);

  MessageShort.addControl("[Playlist]", "LoadSelectedIntoFirstStopped", 0x13, "B", 0x04);

  this.controller.setScaler("jog", this.scalerJog);
  this.controller.setScaler("jog_scratch", this.scalerScratch);
  this.controller.registerInputPacket(MessageShort);

  // Most items in the long message are controls that go from 0-4096.  There are also some encoders.
  MessageLong.addControl("buttons_left", "rate", 0x09, "H");
  MessageLong.addControl("buttons_right", "rate", 0x0B, "H");

  MessageLong.addControl("[Channel1]", "volume", 0x37, "H");
  MessageLong.addControl("[QuickEffectRack1_[Channel1]]", "super1", 0x1D, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x1B, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x19, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x17, "H");
  MessageLong.addControl("[Channel1]", "pregain", 0x03, "B", 0xF0, undefined, true);
  MessageLong.setCallback("[Channel1]", "pregain", this.callbackPregain);

  MessageLong.addControl("[Channel2]", "volume", 0x39, "H");
  MessageLong.addControl("[QuickEffectRack1_[Channel2]]", "super1", 0x25, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x23, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x21, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x1F, "H");
  MessageLong.addControl("[Channel2]", "pregain", 0x04, "B", 0x0F, undefined, true);
  MessageLong.setCallback("[Channel2]", "pregain", this.callbackPregain);

  MessageLong.addControl("[Channel3]", "volume", 0x3B, "H");
  MessageLong.addControl("[QuickEffectRack1_[Channel3]]", "super1", 0x2D, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel3]_Effect1]", "parameter1", 0x2B, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel3]_Effect1]", "parameter2", 0x29, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel3]_Effect1]", "parameter3", 0x27, "H");
  MessageLong.addControl("[Channel3]", "pregain", 0x04, "B", 0xF0, undefined, true);
  MessageLong.setCallback("[Channel3]", "pregain", this.callbackPregain);

  MessageLong.addControl("[Channel4]", "volume", 0x3D, "H");
  MessageLong.addControl("[QuickEffectRack1_[Channel4]]", "super1", 0x35, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel4]_Effect1]", "parameter1", 0x33, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel4]_Effect1]", "parameter2", 0x31, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel4]_Effect1]", "parameter3", 0x2F, "H");
  MessageLong.addControl("[Channel4]", "pregain", 0x05, "B", 0x0F, undefined, true);
  MessageLong.setCallback("[Channel4]", "pregain", this.callbackPregain);

  MessageLong.addControl("[Master]", "volume", 0x11, "H");
  MessageLong.addControl("[Master]", "crossfader", 0x07, "H");
  MessageLong.addControl("[Master]", "headMix", 0x0D, "H");
  MessageLong.addControl("[Playlist]", "!browse", 0x02, "B", 0x0F, undefined, true);
  MessageLong.setCallback("[Playlist]", "!browse", this.callbackBrowse);

  this.controller.setScaler("volume", this.scalerVolume);
  this.controller.setScaler("headMix", this.scalerSlider);
  this.controller.setScaler("parameter1", this.scalerEq);
  this.controller.setScaler("parameter2", this.scalerEq);
  this.controller.setScaler("parameter3", this.scalerEq);
  this.controller.setScaler("super1", this.scalerQuickKnob);
  this.controller.setScaler("crossfader", this.scalerSlider);
  this.controller.setScaler("rate", this.scalerSlider);
  this.controller.registerInputPacket(MessageLong);
}

TraktorS4MK2.init = function(id) {
  // initial lights setup etc?
  TraktorS4MK2.registerInputPackets()
}

TraktorS4MK2.shutdown = function() {
}

// Mandatory function to receive anything from HID
TraktorS4MK2.incomingData = function(data, length) {
  // Packets of 21 bytes are message 0x01 and can be handed off right away
  if (length === 21) {
    TraktorS4MK2.controller.parsePacket(data, length);
    return;
  }

  // Packets of 64 bytes and 15 bytes are partials.  We have to save the 64 byte portion and then
  // append the 15 bytes when we get it
  if (length === 64) {
    this.partial_packet = data;
    return;
  }

  if (length === 15) {
    if (this.partial_packet.length !== 64) {
      HIDDebug("Received second half of message but don't have first half, ignoring");
      return;
    }
    partial_length = this.partial_packet.length;
    // packet data is a javascript Object with properties that are integers (!).  So it's actually
    // unordered data.  So "appending" is just setting more properties.
    for (var i = 0; i < length; i++) {
      this.partial_packet[partial_length + i] = data[i];
    }
    TraktorS4MK2.controller.parsePacket(this.partial_packet, partial_length + length);
    // Clear out the partial packet
    this.partial_packet = Object();
    return;
  }

  HIDDebug("Unhandled packet size: " + length);
}

TraktorS4MK2.getGroupFromButton = function(name) {
  //HIDDebug("deckswitch status " + this.controller.left_deck_C + " " + this.controller.right_deck_D);
  splitted = name.split(".");
  if (splitted.length !== 2) {
    HIDDebug("Tried to set from simple packet but not exactly one period in name: " + name);
    return;
  }

  if (splitted[0][0] === "[") {
    return splitted[0];
  } else if (splitted[0] === "buttons_left") {
    if (TraktorS4MK2.controller.left_deck_C) {
      return "[Channel3]";
    } else {
      return "[Channel1]";
    }
  } else if (splitted[0] === "buttons_right") {
    if (TraktorS4MK2.controller.right_deck_D) {
      return "[Channel4]";
    } else {
      return "[Channel2]";
    }
  } else {
    HIDDebug("Unrecognized packet group: " + splitted[0]);
    return "";
  }
}

// Many buttons can just be handled by the regular Mixxx code, but others require some special
// handling.  Those others start with a "!" to indicate they are special.
// This is the HID equivalent of saying <script/> for a control.
TraktorS4MK2.getButtonHandler = function(name) {
  splitted = name.split(".");
  if (splitted.length !== 2) {
    HIDDebug("Tried to set from simple packet but not exactly one period in name");
    return null;
  }

  var button_name = splitted[1];

  if (button_name[0] !== "!") {
    // Standard Mixxx control, no handler needed;
    return null;
  }
  if (button_name === "!deckswitch") {
    return TraktorS4MK2.deckSwitchHandler;
  }
  if (button_name === "!wheel") {
    return TraktorS4MK2.wheelHandler;
  }

  HIDDebug("Unhandled special button: " + button_name);
  return undefined;
}

TraktorS4MK2.deckSwitchHandler = function(field) {
  if (field.value === 0) {
    return;
  }

  // The group is the currently-assigned value, so set the variable to the opposite.
  if (field.group === "[Channel1]") {
    TraktorS4MK2.controller.left_deck_C = true;
  } else if (field.group === "[Channel3]") {
    TraktorS4MK2.controller.left_deck_C = false;
  } else if (field.group === "[Channel2]") {
    TraktorS4MK2.controller.right_deck_D = true;
  } else if (field.group === "[Channel4]") {
    TraktorS4MK2.controller.right_deck_D = false;
  } else {
    HIDDebug("Unrecognized packet group: " + field.group);
  }
}

TraktorS4MK2.scalerJog = function(group, name, value) {
  // When the wheel is touched, four bytes change, but only the first behaves predictably.
  // It looks like the wheel is 1024 ticks per revolution.
  var tickval = value & 0xFF;
  var timeval = value >>> 16;
  var prev_tick = 0;
  var prev_time = 0;
  if (group[8] === "1" || group[8] === "3") {
    prev_tick = TraktorS4MK2.controller.last_tick_val[0];
    prev_time = TraktorS4MK2.controller.last_tick_time[0];
    TraktorS4MK2.controller.last_tick_val[0] = tickval;
    TraktorS4MK2.controller.last_tick_time[0] = timeval;
  } else {
    prev_tick = TraktorS4MK2.controller.last_tick_val[1];
    prev_time = TraktorS4MK2.controller.last_tick_time[1];
    TraktorS4MK2.controller.last_tick_val[1] = tickval;
    TraktorS4MK2.controller.last_tick_time[1] = timeval;
  }

  var delta = 0;
  if (prev_tick >= 250 && tickval <= 5) {
    HIDDebug("tick loop " + prev_tick + " " + tickval);
    delta = tickval + 256 - prev_tick;
    HIDDebug("delta " + delta);
  } else if (prev_tick <= 5 && tickval >= 250) {
    delta = tickval - prev_tick - 256;
    HIDDebug("tick loop " + prev_tick + " " + tickval + " " + delta);
  } else {
    delta = tickval - prev_tick;
  }

  if (prev_time > timeval) {
    // We looped around.
    timeval += 0x10000;
    HIDDebug("loop around " + timeval + " " + prev_time);
  }

  var velocity = 0.0;
  if (timeval === prev_time) {
    // Spinning too fast to detect speed!
    velocity = delta * 8;
    HIDDebug("super speed! " + delta);
  } else {
    velocity = delta * 5.0 / (timeval - prev_time);
  }

  //HIDDebug(group + " " + name + " eh " + prev_time + " " + delta + " " + timeval + " " + velocity);
  return velocity;
}

TraktorS4MK2.scalerScratch = function(group, name, value) {
  // When the wheel is touched, four bytes change, but only the first behaves predictably.
  // It looks like the wheel is 1024 ticks per revolution.
  HIDDebug("scratch" + group + " " + name + " " + value);
  return 0;
}

TraktorS4MK2.shortMessageCallback = function(packet, data) {
  for (name in data) {
    field = data[name];
    //HIDDebug("that happened " + name + " " + field.group);
    group = TraktorS4MK2.getGroupFromButton(field.id);
    field.group = group;
    if (field.name === "jog_wheel") {
      TraktorS4MK2.controller.processControl(field);
      continue;
    }
    var handler = TraktorS4MK2.getButtonHandler(field.id);
    if (handler !== null) {
      if (handler !== undefined) {
        handler(field);
      }
    } else {
      TraktorS4MK2.controller.processButton(field);
    }
  }
  //TraktorS4MK2.controller.processIncomingPacket(packet, data);
}

TraktorS4MK2.longMessageCallback = function(packet, data) {
  for (name in data) {
    field = data[name];
    //HIDDebug("that happened " + name + " ");
    group = TraktorS4MK2.getGroupFromButton(field.id);
    field.group = group;
    TraktorS4MK2.controller.processControl(field);
  }
  //TraktorS4MK2.controller.processIncomingPacket(packet, data);
}

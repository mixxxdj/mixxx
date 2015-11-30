TraktorS4MK2 = new function() {
   this.controller = new HIDController();
}

TraktorS4MK2.registerInputPackets = function() {
  MessageShort = new HIDPacket("shortmessage",[0x01],21, this.shortMessageCallback);
  MessageLong = new HIDPacket("longmessage",[0x02],79);

  //group,name,offset,pack,bitmask,isEncoder
  MessageShort.addControl("buttons_left", "shift", 0x0D, "B", 0x08);
  MessageShort.addControl("buttons_left", "sync", 0x0D, "B", 0x04);
  MessageShort.addControl("buttons_left", "cue", 0x0D, "B", 0x02);
  MessageShort.addControl("buttons_left", "play", 0x0D, "B", 0x01);
  MessageShort.addControl("buttons_left", "button1", 0x0D, "B", 0x80);
  MessageShort.addControl("buttons_left", "button2", 0x0D, "B", 0x40);
  MessageShort.addControl("buttons_left", "button3", 0x0D, "B", 0x20);
  MessageShort.addControl("buttons_left", "button4", 0x0D, "B", 0x10);
  MessageShort.addControl("buttons_left", "play1", 0x0E, "B", 0x80);
  MessageShort.addControl("buttons_left", "play2", 0x0E, "B", 0x40);
  MessageShort.addControl("buttons_left", "play3", 0x0E, "B", 0x20);
  MessageShort.addControl("buttons_left", "play4", 0x0E, "B", 0x10);
  MessageShort.addControl("buttons_left", "loopout", 0x0E, "B", 0x08);
  MessageShort.addControl("buttons_left", "loopin", 0x0E, "B", 0x04);
  MessageShort.addControl("buttons_left", "flux", 0x0E, "B", 0x02);
  MessageShort.addControl("buttons_left", "reset", 0x0E, "B", 0x01);
  MessageShort.addControl("buttons_left", "loopsize", 0x13, "B", 0x02);
  MessageShort.addControl("buttons_left", "loopmove", 0x13, "B", 0x01);
  MessageShort.addControl("buttons_left", "deckswitch", 0x0F, "B", 0x20);
  MessageShort.addControl("buttons_left", "load", 0x0F, "B", 0x10);
  MessageShort.addControl("buttons_left", "FX1", 0x12, "B", 0x80);
  MessageShort.addControl("buttons_left", "FX2", 0x12, "B", 0x40);
  MessageShort.addControl("buttons_left", "FX3", 0x12, "B", 0x20);
  MessageShort.addControl("buttons_left", "FX4", 0x12, "B", 0x10);
  MessageShort.addControl("buttons_left", "FXMode", 0x11, "B", 0x08);

  MessageShort.addControl("buttons_right", "shift", 0x0C, "B", 0x08);
  MessageShort.addControl("buttons_right", "sync", 0x0C, "B", 0x04);
  MessageShort.addControl("buttons_right", "cue", 0x0C, "B", 0x02);
  MessageShort.addControl("buttons_right", "play", 0x0C, "B", 0x01);
  MessageShort.addControl("buttons_right", "button1", 0x0C, "B", 0x80);
  MessageShort.addControl("buttons_right", "button2", 0x0C, "B", 0x40);
  MessageShort.addControl("buttons_right", "button3", 0x0C, "B", 0x20);
  MessageShort.addControl("buttons_right", "button4", 0x0C, "B", 0x10);
  MessageShort.addControl("buttons_right", "play1", 0x0B, "B", 0x80);
  MessageShort.addControl("buttons_right", "play2", 0x0B, "B", 0x40);
  MessageShort.addControl("buttons_right", "play3", 0x0B, "B", 0x20);
  MessageShort.addControl("buttons_right", "play4", 0x0B, "B", 0x10);
  MessageShort.addControl("buttons_right", "loopout", 0x0B, "B", 0x08);
  MessageShort.addControl("buttons_right", "loopin", 0x0B, "B", 0x04);
  MessageShort.addControl("buttons_right", "flux", 0x0B, "B", 0x02);
  MessageShort.addControl("buttons_right", "reset", 0x0B, "B", 0x01);
  MessageShort.addControl("buttons_right", "loopsize", 0x13, "B", 0x10);
  MessageShort.addControl("buttons_right", "loopmove", 0x13, "B", 0x08);
  MessageShort.addControl("buttons_right", "deckswitch", 0x0A, "B", 0x20);
  MessageShort.addControl("buttons_right", "load", 0x0A, "B", 0x10);
  MessageShort.addControl("buttons_right", "FX1", 0x10, "B", 0x08);
  MessageShort.addControl("buttons_right", "FX2", 0x10, "B", 0x04);
  MessageShort.addControl("buttons_right", "FX3", 0x10, "B", 0x02);
  MessageShort.addControl("buttons_right", "FX4", 0x10, "B", 0x01);
  MessageShort.addControl("buttons_right", "FXMode", 0x11, "B", 0x04);

  MessageShort.addControl("[Channel1]", "pfl", 0x0F, "B", 0x40);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel1]_enable", 0x12, "B", 0x02);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel1]_enable", 0x12, "B", 0x01);
  MessageShort.addControl("[Channel1]", "pregain_push", 0x11, "B", 0x20);

  MessageShort.addControl("[Channel2]", "pfl", 0x0A, "B", 0x40);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel2]_enable", 0x10, "B", 0x80);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel2]_enable", 0x10, "B", 0x40);
  MessageShort.addControl("[Channel2]", "pregain_push", 0x11, "B", 0x40);

  MessageShort.addControl("[Channel3]", "pfl", 0x0F, "B", 0x80);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel3]_enable", 0x12, "B", 0x08);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel3]_enable", 0x12, "B", 0x04);
  MessageShort.addControl("[Channel3]", "pregain_push", 0x11, "B", 0x10);

  MessageShort.addControl("[Channel4]", "pfl", 0x0A, "B", 0x80);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel4]_enable", 0x10, "B", 0x20);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel4]_enable", 0x10, "B", 0x10);
  MessageShort.addControl("[Channel4]", "pregain_push", 0x11, "B", 0x80);



  this.controller.registerInputPacket(MessageShort);
}

TraktorS4MK2.init = function(id) {
  // initial lights setup etc?
  TraktorS4MK2.registerInputPackets()
}

TraktorS4MK2.shutdown = function() {
}

// Mandatory function to receive anything from HID
TraktorS4MK2.incomingData = function(data, length) {
  TraktorS4MK2.controller.parsePacket(data, length);
}

TraktorS4MK2.shortMessageCallback = function(packet, data) {
  HIDDebug("that happened " + Object.keys(data));
}

//controller = new TraktorS4MK2();

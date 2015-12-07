// TODO:
// fx knobs / lights
// loop knobs
// shift button stuff
// (loop size readout :()
// snap / master / quant buttons?
// scratch handoff like in vestax

TraktorS4MK2 = new function() {
  this.controller = new HIDController();
  this.partial_packet = Object();
  this.divisor_map = Object();
  // When true, packets will not be sent to the controller.  Good for doing mass updates.
  this.controller.freeze_lights = false;
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
  this.controller.sync_enabled_time = Object();

  // scratch overrides
  this.controller.scratchintervalsPerRev = 1024;
  this.controller.scratchRPM = 33+1/3;
  this.controller.scratchAlpha = 1.0 / 8;
  this.controller.scratchBeta = this.controller.scratchAlpha / 8;
  this.controller.scratchRampOnEnable = true;
  this.controller.scratchRampOnDisable = true;
}

TraktorS4MK2.registerInputPackets = function() {
  MessageShort = new HIDPacket("shortmessage", [0x01], 21, this.shortMessageCallback);
  MessageLong = new HIDPacket("longmessage", [0x02], 79, this.longMessageCallback);

  // Values in the short message are all buttons, except the jog wheels.
  // An exclamation point indicates a specially-handled function.  Everything else is a standard
  // Mixxx control object name.
  MessageShort.addControl("deck1", "!shift", 0x0D, "B", 0x08);
  MessageShort.addControl("deck1", "!sync_enabled", 0x0D, "B", 0x04);
  MessageShort.addControl("deck1", "cue_default", 0x0D, "B", 0x02);
  MessageShort.addControl("deck1", "play", 0x0D, "B", 0x01);
  MessageShort.addControl("deck1", "hotcue_1_activate", 0x0D, "B", 0x80);
  MessageShort.addControl("deck1", "hotcue_2_activate", 0x0D, "B", 0x40);
  MessageShort.addControl("deck1", "hotcue_3_activate", 0x0D, "B", 0x20);
  MessageShort.addControl("deck1", "hotcue_4_activate", 0x0D, "B", 0x10);
  MessageShort.addControl("deck1", "!play1", 0x0E, "B", 0x80);
  MessageShort.addControl("deck1", "!play2", 0x0E, "B", 0x40);
  MessageShort.addControl("deck1", "!play3", 0x0E, "B", 0x20);
  MessageShort.addControl("deck1", "!play4", 0x0E, "B", 0x10);
  MessageShort.addControl("deck1", "loop_out", 0x0E, "B", 0x08);
  MessageShort.addControl("deck1", "loop_in", 0x0E, "B", 0x04);
  MessageShort.addControl("deck1", "slip_enabled", 0x0E, "B", 0x02);
  MessageShort.addControl("deck1", "!reset", 0x0E, "B", 0x01);
  MessageShort.addControl("deck1", "!loopsize", 0x13, "B", 0x02);
  MessageShort.addControl("deck1", "!loopmove", 0x13, "B", 0x01);
  MessageShort.addControl("deck1", "jog_touch", 0x11, "B", 0x01);
  MessageShort.addControl("deck1", "jog_wheel", 0x01, "I");
  MessageShort.addControl("deck1", "!deckswitch", 0x0F, "B", 0x20);
  MessageShort.addControl("deck1", "LoadSelectedTrack", 0x0F, "B", 0x10);
  MessageShort.addControl("deck1", "!FX1", 0x12, "B", 0x80);
  MessageShort.addControl("deck1", "!FX2", 0x12, "B", 0x40);
  MessageShort.addControl("deck1", "!FX3", 0x12, "B", 0x20);
  MessageShort.addControl("deck1", "!FX4", 0x12, "B", 0x10);
  MessageShort.addControl("deck1", "!FXMode", 0x11, "B", 0x08);

  MessageShort.addControl("deck2", "!shift", 0x0C, "B", 0x08);
  MessageShort.addControl("deck2", "!sync_enabled", 0x0C, "B", 0x04);
  MessageShort.addControl("deck2", "cue_default", 0x0C, "B", 0x02);
  MessageShort.addControl("deck2", "play", 0x0C, "B", 0x01);
  MessageShort.addControl("deck2", "hotcue_1_activate", 0x0C, "B", 0x80);
  MessageShort.addControl("deck2", "hotcue_2_activate", 0x0C, "B", 0x40);
  MessageShort.addControl("deck2", "hotcue_3_activate", 0x0C, "B", 0x20);
  MessageShort.addControl("deck2", "hotcue_4_activate", 0x0C, "B", 0x10);
  MessageShort.addControl("deck2", "!play1", 0x0B, "B", 0x80);
  MessageShort.addControl("deck2", "!play2", 0x0B, "B", 0x40);
  MessageShort.addControl("deck2", "!play3", 0x0B, "B", 0x20);
  MessageShort.addControl("deck2", "!play4", 0x0B, "B", 0x10);
  MessageShort.addControl("deck2", "loop_out", 0x0B, "B", 0x08);
  MessageShort.addControl("deck2", "loop_in", 0x0B, "B", 0x04);
  MessageShort.addControl("deck2", "slip_enabled", 0x0B, "B", 0x02);
  MessageShort.addControl("deck2", "!reset", 0x0B, "B", 0x01);
  MessageShort.addControl("deck2", "!loopsize", 0x13, "B", 0x10);
  MessageShort.addControl("deck2", "!loopmove", 0x13, "B", 0x08);
  MessageShort.addControl("deck2", "jog_touch", 0x11, "B", 0x02);
  MessageShort.addControl("deck2", "jog_wheel", 0x05, "I");
  MessageShort.addControl("deck2", "!deckswitch", 0x0A, "B", 0x20);
  MessageShort.addControl("deck2", "LoadSelectedTrack", 0x0A, "B", 0x10);
  MessageShort.addControl("deck2", "!FX1", 0x10, "B", 0x08);
  MessageShort.addControl("deck2", "!FX2", 0x10, "B", 0x04);
  MessageShort.addControl("deck2", "!FX3", 0x10, "B", 0x02);
  MessageShort.addControl("deck2", "!FX4", 0x10, "B", 0x01);
  MessageShort.addControl("deck2", "!FXMode", 0x11, "B", 0x04);

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
  MessageShort.setCallback("deck1", "!deckswitch", this.deckSwitchHandler);
  MessageShort.setCallback("deck2", "!deckswitch", this.deckSwitchHandler);
  MessageShort.setCallback("deck1", "!sync_enabled", this.syncEnabledHandler);
  MessageShort.setCallback("deck2", "!sync_enabled", this.syncEnabledHandler);
  // TODO: the rest of the "!" controls.
  this.controller.registerInputPacket(MessageShort);

  // Most items in the long message are controls that go from 0-4096.
  // There are also some 4 bit encoders.
  MessageLong.addControl("deck1", "rate", 0x09, "H");
  MessageLong.addControl("deck2", "rate", 0x0B, "H");

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

TraktorS4MK2.registerOutputPackets = function() {
  Output1 = new HIDPacket("output1", [0x80], 53);
  Output2 = new HIDPacket("output2", [0x81], 63);
  Output3 = new HIDPacket("output3", [0x82], 61);

  Output1.addOutput("[Channel1]", "track_samples", 0x10, "B");
  Output1.addOutput("[Channel2]", "track_samples", 0x18, "B");
  Output1.addOutput("[Channel3]", "track_samples", 0x08, "B");
  Output1.addOutput("[Channel4]", "track_samples", 0x20, "B");

  var VuOffsets = {"[Channel1]" : 0x09,
                   "[Channel2]" : 0x11,
                   "[Channel3]" : 0x01,
                   "[Channel4]" : 0x19};
  for (ch in VuOffsets) {
    for (i = 0; i < 0x06; i++) {
      Output1.addOutput(ch, "!" + "VuMeter" + i, VuOffsets[ch] + i, "B");
    }
  }

  Output1.addOutput("[Channel1]", "PeakIndicator", 0x0F, "B");
  Output1.addOutput("[Channel2]", "PeakIndicator", 0x17, "B");
  Output1.addOutput("[Channel3]", "PeakIndicator", 0x07, "B");
  Output1.addOutput("[Channel4]", "PeakIndicator", 0x1F, "B");

  Output1.addOutput("[Master]", "!usblight", 0x2A, "B");
  this.controller.registerOutputPacket(Output1);

  Output2.addOutput("deck1", "sync_enabled", 0x1E, "B");
  Output2.addOutput("deck1", "cue_indicator", 0x1F, "B");
  Output2.addOutput("deck1", "play_indicator", 0x20, "B");
  Output2.addOutput("deck1", "hotcue_1_enabled", 0x01, "B");
  Output2.addOutput("deck1", "!hotcue_1_enabled_G", 0x02, "B");
  Output2.addOutput("deck1", "!hotcue_1_enabled_B", 0x03, "B");
  Output2.addOutput("deck1", "hotcue_2_enabled", 0x04, "B");
  Output2.addOutput("deck1", "!hotcue_2_enabled_G", 0x05, "B");
  Output2.addOutput("deck1", "!hotcue_2_enabled_B", 0x06, "B");
  Output2.addOutput("deck1", "hotcue_3_enabled", 0x07, "B");
  Output2.addOutput("deck1", "!hotcue_3_enabled_G", 0x08, "B");
  Output2.addOutput("deck1", "!hotcue_3_enabled_B", 0x09, "B");
  Output2.addOutput("deck1", "hotcue_4_enabled", 0x0A, "B");
  Output2.addOutput("deck1", "!hotcue_4_enabled_G", 0x0B, "B");
  Output2.addOutput("deck1", "!hotcue_4_enabled_B", 0x0C, "B");
  Output2.addOutput("deck1", "loop_in", 0x29, "B");
  Output2.addOutput("deck1", "loop_out", 0x2A, "B");
  Output2.addOutput("deck1", "keylock", 0x2F, "B");
  Output2.addOutput("deck1", "slip_enabled", 0x39, "B");

  Output2.addOutput("deck2", "sync_enabled", 0x26, "B");
  Output2.addOutput("deck2", "cue_indicator", 0x27, "B");
  Output2.addOutput("deck2", "play_indicator", 0x28, "B");
  Output2.addOutput("deck2", "hotcue_1_enabled", 0x0D, "B");
  Output2.addOutput("deck2", "!hotcue_1_enabled_G", 0x0E, "B");
  Output2.addOutput("deck2", "!hotcue_1_enabled_B", 0x0F, "B");
  Output2.addOutput("deck2", "hotcue_2_enabled", 0x10, "B");
  Output2.addOutput("deck2", "!hotcue_2_enabled_G", 0x11, "B");
  Output2.addOutput("deck2", "!hotcue_2_enabled_B", 0x12, "B");
  Output2.addOutput("deck2", "hotcue_3_enabled", 0x13, "B");
  Output2.addOutput("deck2", "!hotcue_3_enabled_G", 0x14, "B");
  Output2.addOutput("deck2", "!hotcue_3_enabled_B", 0x15, "B");
  Output2.addOutput("deck2", "hotcue_4_enabled", 0x16, "B");
  Output2.addOutput("deck2", "!hotcue_4_enabled_G", 0x17, "B");
  Output2.addOutput("deck2", "!hotcue_4_enabled_B", 0x18, "B");
  Output2.addOutput("deck2", "loop_in", 0x2B, "B");
  Output2.addOutput("deck2", "loop_out", 0x2C, "B");
  Output2.addOutput("deck2", "keylock", 0x35, "B");
  Output2.addOutput("deck2", "slip_enabled", 0x3B, "B");

  Output2.addOutput("[Channel1]", "!deck_A", 0x2E, "B");
  Output2.addOutput("[Channel2]", "!deck_B", 0x34, "B");
  Output2.addOutput("[Channel3]", "!deck_C", 0x31, "B");
  Output2.addOutput("[Channel4]", "!deck_D", 0x37, "B");

  this.controller.registerOutputPacket(Output2);

  Output3.addOutput("[Channel3]", "pfl", 0x17, "B");
  Output3.addOutput("[Channel1]", "pfl", 0x18, "B");
  Output3.addOutput("[Channel2]", "pfl", 0x19, "B");
  Output3.addOutput("[Channel4]", "pfl", 0x1A, "B");

  Output3.addOutput("[Master]", "PeakIndicatorL", 0x3B, "B");
  Output3.addOutput("[Master]", "PeakIndicatorR", 0x3C, "B");

  Output3.addOutput("deck1", "!deckLight", 0x13, "B");
  Output3.addOutput("deck1", "LoadSelectedTrack", 0x14, "B");

  Output3.addOutput("deck2", "!deckLight", 0x16, "B");
  Output3.addOutput("deck2", "LoadSelectedTrack", 0x15, "B");
  this.controller.registerOutputPacket(Output3);

  // Link up control objects to their outputs
  TraktorS4MK2.linkDeckOutputs("sync_enabled", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("cue_indicator", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("play_indicator", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("hotcue_1_enabled", TraktorS4MK2.outputCueCallback);
  TraktorS4MK2.linkDeckOutputs("hotcue_2_enabled", TraktorS4MK2.outputCueCallback);
  TraktorS4MK2.linkDeckOutputs("hotcue_3_enabled", TraktorS4MK2.outputCueCallback);
  TraktorS4MK2.linkDeckOutputs("hotcue_4_enabled", TraktorS4MK2.outputCueCallback);
  TraktorS4MK2.linkDeckOutputs("loop_in", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("loop_out", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("keylock", TraktorS4MK2.outputCallbackDark);
  TraktorS4MK2.linkDeckOutputs("LoadSelectedTrack", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("slip_enabled", TraktorS4MK2.outputCallback);
  TraktorS4MK2.controller.linkOutput("[Channel1]", "pfl", "[Channel1]", "pfl", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.controller.linkOutput("[Channel2]", "pfl", "[Channel2]", "pfl", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.controller.linkOutput("[Channel3]", "pfl", "[Channel3]", "pfl", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.controller.linkOutput("[Channel4]", "pfl", "[Channel4]", "pfl", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.controller.linkOutput("[Channel1]", "track_samples", "[Channel1]", "track_samples", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.controller.linkOutput("[Channel2]", "track_samples", "[Channel2]", "track_samples", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.controller.linkOutput("[Channel3]", "track_samples", "[Channel3]", "track_samples", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.controller.linkOutput("[Channel4]", "track_samples", "[Channel4]", "track_samples", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.controller.linkOutput("[Channel1]", "PeakIndicator", "[Channel1]", "PeakIndicator", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.controller.linkOutput("[Channel2]", "PeakIndicator", "[Channel2]", "PeakIndicator", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.controller.linkOutput("[Channel3]", "PeakIndicator", "[Channel3]", "PeakIndicator", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.controller.linkOutput("[Channel4]", "PeakIndicator", "[Channel4]", "PeakIndicator", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.controller.linkOutput("[Master]", "PeakIndicatorL", "[Master]", "PeakIndicatorL", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.controller.linkOutput("[Master]", "PeakIndicatorR", "[Master]", "PeakIndicatorR", TraktorS4MK2.outputChannelCallbackDark);

  // VU meters get special attention
  engine.connectControl("[Channel1]", "VuMeter", "TraktorS4MK2.onVuMeterChanged");
  engine.connectControl("[Channel2]", "VuMeter", "TraktorS4MK2.onVuMeterChanged");
  engine.connectControl("[Channel3]", "VuMeter", "TraktorS4MK2.onVuMeterChanged");
  engine.connectControl("[Channel4]", "VuMeter", "TraktorS4MK2.onVuMeterChanged");
}

TraktorS4MK2.linkDeckOutputs = function(key, callback) {
  // Linking outputs is a little tricky, the library doesn't quite do what I want.  But this
  // method works.
  TraktorS4MK2.controller.linkOutput("deck1", key, "[Channel1]", key, callback);
  engine.connectControl("[Channel3]", key, callback);
  TraktorS4MK2.controller.linkOutput("deck2", key, "[Channel2]", key, callback);
  engine.connectControl("[Channel4]", key, callback);
}

TraktorS4MK2.lightGroup = function(packet, output_group_name, co_group_name) {
  var group_ob = packet.groups[output_group_name];
  for (var field_name in group_ob) {
    field = group_ob[field_name];
    if (field.name[0] === "!") {
      continue;
    }
    var value = engine.getValue(co_group_name, field.name);
    if (field.mapped_callback) {
      field.mapped_callback(value, co_group_name, field.name);
    }
    // No callback, no light!
  }
}

TraktorS4MK2.lightDeck = function(group) {
  // Freeze the lights while we do this update.
  this.controller.freeze_lights = true;
  for (var packet_name in this.controller.OutputPackets) {
    packet = this.controller.OutputPackets[packet_name];
    var deck_group_name = "deck1";
    if (group === "[Channel2]" || group === "[Channel4]") {
      deck_group_name = "deck2";
    }

    TraktorS4MK2.lightGroup(packet, deck_group_name, group);
    TraktorS4MK2.lightGroup(packet, group, group);
  }

  if (group === "[Channel1]") {
    TraktorS4MK2.controller.setOutput("[Channel1]", "!deck_A", 0x7F, false);
    TraktorS4MK2.controller.setOutput("[Channel3]", "!deck_C", 0x00, false);
    TraktorS4MK2.controller.setOutput("deck1", "!deckLight", 0x05, false);
  } else if (group === "[Channel2]") {
    TraktorS4MK2.controller.setOutput("[Channel2]", "!deck_B", 0x7F, false);
    TraktorS4MK2.controller.setOutput("[Channel4]", "!deck_D", 0x00, false);
    TraktorS4MK2.controller.setOutput("deck2", "!deckLight", 0x05, false);
  } else if (group === "[Channel3]") {
    TraktorS4MK2.controller.setOutput("[Channel3]", "!deck_C", 0x7F, false);
    TraktorS4MK2.controller.setOutput("[Channel1]", "!deck_A", 0x00, false);
    TraktorS4MK2.controller.setOutput("deck1", "!deckLight", 0x7F, false);
  } else if (group === "[Channel4]") {
    TraktorS4MK2.controller.setOutput("[Channel4]", "!deck_D", 0x7F, false);
    TraktorS4MK2.controller.setOutput("[Channel2]", "!deck_B", 0x00, false);
    TraktorS4MK2.controller.setOutput("deck2", "!deckLight", 0x7F, false);
  }

  this.controller.freeze_lights = false;
  // And now send them all.
  for (packet_name in this.controller.OutputPackets) {
    var packet_ob = this.controller.OutputPackets[packet_name];
    packet_ob.send();
  }
}

TraktorS4MK2.init = function(id) {
  // initial lights setup etc?
  TraktorS4MK2.registerInputPackets()
  TraktorS4MK2.registerOutputPackets()
  //var data_strings = ["80 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 0A 0A 0A 0A 0A 0A 0A 0A 00 7F 00 00 00 00 0A 0A 0A 0A 0A 0A",
  //                    "81 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 7F 0A 0A 0A 0A 0A 7F 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A",
  //                    "82 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 00 00 7F 7F 7F 7F 7F 7F 00 00 7F 7F 7F 7F 7F 00 00 00 7F 7F 7F 7F 7F 7F 00 00 7F 7F 7F 7F 7F 00 00 00"];
  //                   00 01 02 03  04 05 06 07  08 09 0A 0B  0C 0D 0E 0F
  /*var data_strings = ["80 00 00 00  00 00 00 00  0A 00 00 00  00 00 00 00  \n" +
                      "0A 00 00 00  00 00 00 00  0A 00 00 00  00 00 00 00  \n" +
                      "0A 0A 0A 0A  0A 0A 0A 0A  0A 00 7F 00  00 00 00 0A  \n" +
                      "0A 0A 0A 0A  0A",
                      "81 00 00 7F  7F 03 7F 0B  03 7F 0B 03  7F 0B 03 7F  \n" +
                      "0B 03 7F 0B  03 00 7f 03  7F 0A 0A 0A  00 7f 0A 0A  \n" +
                      "00 7f 0A 0A  0A 0A 0A 0A  7F 0A 0a 0A  0a 0a 7f 0a  \n" +
                      "0a 0a 0a 0a  7F 0A 0A 0A  0a 0a 0a 0a  0a 0a 0a",
                      "82 0a 0A 0A  0a 0a 0a 0A  0A 0A 0A 0A  0a 0a 0A 0A  \n" +
                      "0a 0a 0a 0a  0a 0a 0a 0a  0A 0A 0a 0a  00 00 00 00  \n" +
                      "7f 00 00 7f  00 7F 7F 7F  7F 7F 7f 7f  00 7F 7F 7F  \n" +
                      "7F 7F 7F 00  00 7F 7F 7F  7F 7F 00 7f  00"];
  var data = [Object(), Object(), Object()];

  HIDDebug("here2");

  for (i = 0; i < 3; i++) {
    var ok = true;
    var splitted = data_strings[i].split(/\s+/);
    HIDDebug("i" + i + " " + splitted);
    data[i].length = splitted.length;
    for (j = 0; j < splitted.length; j++) {
      var byte_str = splitted[j];
      if (byte_str.length !== 2) {
        ok = false;
        HIDDebug("not two characters?? " + byte_str);
      }
      var b = parseInt(byte_str, 16);
      if (b < 0 || b > 255) {
        ok = false;
        HIDDebug("number out of range: " + byte_str + " " + b);
      }
      data[i][j] = b;
    }
    if (ok) {
      HIDDebug("so what the length? " + data[i].length);
      controller.send(data[i], data[i].length, 0);
    }
  }*/

  HIDDebug("---------");
  // Light 3 and 4 first so we get the mixer lights on, then do 1 and 2 since those are active
  // on startup.
  TraktorS4MK2.controller.setOutput("[Master]", "!usblight", 0x7F, true);
  TraktorS4MK2.lightDeck("[Channel3]");
  TraktorS4MK2.lightDeck("[Channel4]");
  TraktorS4MK2.lightDeck("[Channel1]");
  TraktorS4MK2.lightDeck("[Channel2]");

  HIDDebug("done init");
}

TraktorS4MK2.shutdown = function() {
  var packet_lengths = [53, 63, 61];
  for (i = 0; i < packet_lengths.length; i++) {
    var packet_length = packet_lengths[i];
    var data = Object();
    data.length = packet_length;
    data[0] = 0x80 + i;
    for (j = 1; j < packet_length; j++) {
      data[j] = 0;
    }
    controller.send(data, packet_length, 0);
  }
  // Key USB light on though.
  TraktorS4MK2.controller.setOutput("[Master]", "!usblight", 0x7F, true);
}

// Mandatory function to receive anything from HID
TraktorS4MK2.incomingData = function(data, length) {
  // Packets of 21 bytes are message 0x01 and can be handed off right away
  if (length === 21) {
    TraktorS4MK2.controller.parsePacket(data, length);
    return;
  }

  // Packets of 64 bytes and 15 bytes are partials.  We have to save the 64 byte portion and then
  // append the 15 bytes when we get it.
  if (length === 64) {
    this.partial_packet = data;
    return;
  }

  if (length === 15) {
    if (this.partial_packet.length !== 64) {
      HIDDebug("Received second half of message but don't have first half, ignoring");
      return;
    }
    // packet data is a javascript Object with properties that are integers (!).  So it's actually
    // unordered data (!!).  So "appending" is just setting more properties.
    partial_length = this.partial_packet.length;
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

// The short message handles buttons and jog wheels.
TraktorS4MK2.shortMessageCallback = function(packet, data) {
  for (name in data) {
    field = data[name];
    //HIDDebug("that happened " + name + " " + field.group);
    // Rewrite group name from "buttons_X" to "[ChannelY]"
    group = TraktorS4MK2.getGroupFromButton(field.id);
    field.group = group;
    if (field.name === "jog_wheel") {
      TraktorS4MK2.controller.processControl(field);
      continue;
    }

    TraktorS4MK2.controller.processButton(field);
  }
}

// There are no buttons handled by the long message, so this is a little simpler.  (Even though
// this is very similar to the other handler, it's easier to keep them separate to know what's
// a control and what's a button.
TraktorS4MK2.longMessageCallback = function(packet, data) {
  for (name in data) {
    field = data[name];
    //HIDDebug("that happened " + name + " ");
    // Rewrite group name from "buttons_X" to "[ChannelY]"
    group = TraktorS4MK2.getGroupFromButton(field.id);
    field.group = group;
    TraktorS4MK2.controller.processControl(field);
  }
}

// Utility function for converting mappings like "deck1.play" into "[ChannelX].play" based
// on the state of the deck switches.
TraktorS4MK2.getGroupFromButton = function(name) {
  //HIDDebug("deckswitch status " + this.controller.left_deck_C + " " + this.controller.right_deck_D);
  splitted = name.split(".");
  if (splitted.length !== 2) {
    HIDDebug("Tried to set from simple packet but not exactly one period in name: " + name);
    return;
  }

  if (splitted[0][0] === "[") {
    return splitted[0];
  } else if (splitted[0] === "deck1") {
    if (TraktorS4MK2.controller.left_deck_C) {
      return "[Channel3]";
    } else {
      return "[Channel1]";
    }
  } else if (splitted[0] === "deck2") {
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

TraktorS4MK2.deckSwitchHandler = function(field) {
  if (field.value === 0) {
    return;
  }

  // The group is the currently-assigned value, so set the variable to the opposite.
  if (field.group === "[Channel1]") {
    TraktorS4MK2.controller.left_deck_C = true;
    TraktorS4MK2.lightDeck("[Channel3]");
  } else if (field.group === "[Channel3]") {
    TraktorS4MK2.controller.left_deck_C = false;
    TraktorS4MK2.lightDeck("[Channel1]");
  } else if (field.group === "[Channel2]") {
    TraktorS4MK2.controller.right_deck_D = true;
    TraktorS4MK2.lightDeck("[Channel4]");
  } else if (field.group === "[Channel4]") {
    TraktorS4MK2.controller.right_deck_D = false;
    TraktorS4MK2.lightDeck("[Channel2]");
  } else {
    HIDDebug("Unrecognized packet group: " + field.group);
  }
}

TraktorS4MK2.syncEnabledHandler = function(field) {
  var now = Date.now();
  if (field.value === 1) {
    // The group is the currently-assigned value, so set the variable to the opposite.
    TraktorS4MK2.controller.sync_enabled_time[field.group] = now;
    engine.setValue(field.group, "sync_enabled", 1);
  } else {
    var cur_enabled = engine.getValue(field.group, "sync_enabled");
    if (!cur_enabled) {
      // If disabled, and switching to disable... stay disabled.
      engine.setValue(field.group, "sync_enabled", 0);
      return;
    }
    // was enabled, and button has been let go.  maybe latch it.
    if (now - TraktorS4MK2.controller.sync_enabled_time[field.group] > 300) {
      engine.setValue(field.group, "sync_enabled", 1);
      return;
    }
    engine.setValue(field.group, "sync_enabled", 0);
  }
}

TraktorS4MK2.wheelDeltas = function(group, value) {
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

  if (prev_time > timeval) {
    // We looped around.  Adjust current time so that subtraction works.
    timeval += 0x10000;
  }
  var time_delta = timeval - prev_time;
  if (time_delta === 0) {
    // Spinning too fast to detect speed!  By not dividing we are guessing it took 1ms.
    time_delta = 1;
  }

  var tick_delta = 0;
  if (prev_tick >= 200 && tickval <= 100) {
    tick_delta = tickval + 256 - prev_tick;
  } else if (prev_tick <= 100 && tickval >= 200) {
    tick_delta = tickval - prev_tick - 256;
  } else {
    tick_delta = tickval - prev_tick;
  }
//  HIDDebug(tickval + " " + prev_tick + " " + tick_delta);
  return [tick_delta, time_delta];
}

TraktorS4MK2.wheelVelocity = function(group, value) {
  var deltas = TraktorS4MK2.wheelDeltas(group, value);
  var tick_delta = deltas[0];
  var time_delta = deltas[1];

  var velocity = tick_delta / time_delta;

  //HIDDebug(group + " " + name + " eh " + prev_time + " " + delta + " " + timeval + " " + velocity);
  return velocity;
}

TraktorS4MK2.scalerJog = function(group, name, value) {
  if (engine.getValue(group, "play")) {
    return TraktorS4MK2.wheelVelocity(group, value) / 2;
  } else {
    return TraktorS4MK2.wheelVelocity(group, value) * 2.5;
  }
}

TraktorS4MK2.scalerScratch = function(group, name, value) {
  // Return raw tick delta
  return TraktorS4MK2.wheelDeltas(group, value)[0];
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

TraktorS4MK2.resolveDeckIfActive = function(group) {
  var controller = TraktorS4MK2.controller;
  if (group === "[Channel1]") {
    if (controller.left_deck_C) {
      return undefined;
    }
    return "deck1";
  } else if (group === "[Channel3]") {
    if (!controller.left_deck_C) {
      return undefined;
    }
    return "deck1";
  } else if (group === "[Channel2]") {
    if (controller.right_deck_D) {
      return undefined;
    }
    return "deck2";
  } else if (group === "[Channel4]") {
    if (!controller.right_deck_D) {
      return undefined;
    }
    return "deck2";
  }
  return undefined;
}

TraktorS4MK2.outputChannelCallback = function(value,group,key) {
  var controller = TraktorS4MK2.controller;
  //HIDDebug("output channel? " + group + " " + key + " " + value + " " + TraktorS4MK2.controller.left_deck_C);
  var led_value = 0x05;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS4MK2.controller.setOutput(group, key, led_value, !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.outputChannelCallbackDark = function(value,group,key) {
  var controller = TraktorS4MK2.controller;
  var led_value = 0x00;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS4MK2.controller.setOutput(group, key, led_value, !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.outputCallback = function(value,group,key) {
  var controller = TraktorS4MK2.controller;
  //HIDDebug("output2? " + group + " " + key + " " + value + " " + TraktorS4MK2.controller.left_deck_C);
  var deck_group = TraktorS4MK2.resolveDeckIfActive(group);
  if (deck_group === undefined) {
    return;
  }

  var led_value = 0x09;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS4MK2.controller.setOutput(deck_group, key, led_value, !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.outputCallbackDark = function(value,group,key) {
  var controller = TraktorS4MK2.controller;
  var deck_group = TraktorS4MK2.resolveDeckIfActive(group);
  if (deck_group === undefined) {
    return;
  }

  var led_value = 0x00;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS4MK2.controller.setOutput(deck_group, key, led_value, !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.outputCueCallback = function(value, group, key) {
 var deck_group = TraktorS4MK2.resolveDeckIfActive(group);
  if (deck_group === undefined) {
    HIDDebug("nope " + group);
    return;
  }

  var RGB_value = [0, 0, 0];
  if (group === "[Channel1]" || group === "[Channel2]") {
    RGB_value = [0x40, 0x02, 0x02];
  } else {
    RGB_value = [0x0A, 0x35, 0x35];
  }

  TraktorS4MK2.controller.setOutput(deck_group, key, RGB_value[0], false);
  TraktorS4MK2.controller.setOutput(deck_group, "!" + key + "_G", RGB_value[1], false);
  TraktorS4MK2.controller.setOutput(deck_group, "!" + key + "_B", RGB_value[2], !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.onVuMeterChanged = function(value, group, key) {
  // VU is drawn on 6 segments, the 7th indicates clip.

  var VuOffsets = {"[Channel1]" : 0x09,
                   "[Channel2]" : 0x11,
                   "[Channel3]" : 0x01,
                   "[Channel4]" : 0x19};

  // Figure out number of fully-illuminated segments.
  var fullIllum = Math.floor(value * 6.0);

  // Figure out how much the partially-illuminated segment is illuminated.
  var partialIllum = Math.floor((value % 6) * 0x7F);

  var packet_ob = TraktorS4MK2.controller.OutputPackets["output1"];
  for (i = 0; i < 6; i++) {
    var key = "!" + "VuMeter" + i;
    if (i < fullIllum) {
      // Don't update lights until they're all done.
      TraktorS4MK2.controller.setOutput(group, key, 0x7F, false);
    } else if (i == fullIllum) {
      TraktorS4MK2.controller.setOutput(group, key, partialIllum, false);
    } else {
      TraktorS4MK2.controller.setOutput(group, key, 0x00, false);
    }
  }
  var packet_ob = TraktorS4MK2.controller.OutputPackets["output1"];
  packet_ob.send();
}

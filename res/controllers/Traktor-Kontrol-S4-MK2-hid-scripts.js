/****************************************************************/
/*      Traktor Kontrol S4 MK2 HID controller script v1.00      */
/*      Copyright (C) 2015, the Mixxx Team                      */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 2.0                                   */
/****************************************************************/

// TODO:
// * Microphone volume
// * Sampler volume
// * On Air indicator for shoutcast or set recording
// * Loop size readout / selection
// * Find a use for snap / master / other unused buttons?


// ==== Friendly User Configuration ====
// The Remix Slot buttons can have two possible functions:
// 1. Sample launchers: pressing the buttons activates sample players in Mixxx
// 2. Loop Roll: Holding the buttons activates a short loop, and when the button is
//    released playback will resume where the track would have been without the loop.
// To choose sample launching, set the word in quotes below to SAMPLES, all caps.  For
// rolls, change the word to LOOPROLLS (no space).
RemixSlotButtonAction = "SAMPLES";
// The Cue button, when Shift is also held, can have two possible functions:
// 1. "REWIND": seeks to the very start of the track.
// 2. "REVERSEROLL": performs a temporary reverse or "censor" effect, where the track
//    is momentarily played in reverse until the button is released.
ShiftCueButtonAction = "REWIND";


TraktorS4MK2 = new function() {
  this.controller = new HIDController();
  // TODO: Decide if these should be part of this.controller instead.
  this.partial_packet = Object();
  this.divisor_map = Object();
  this.RemixSlotButtonAction = RemixSlotButtonAction;
  this.ShiftCueButtonAction = ShiftCueButtonAction;

  // When true, packets will not be sent to the controller.  Good for doing mass updates.
  this.controller.freeze_lights = false;

  // Active deck switches -- common-hid-packet-parser only has one active deck status per
  // Controller object.
  this.controller.left_deck_C = false;
  this.controller.right_deck_D = false;
  // The controller has a single quantize button, and we remember its state independent of
  // other channels.  (The user may toggle channel quantize in the GUI)
  this.controller.master_quantize = false;
  // Previous values, used for calculating deltas for encoder knobs.
  this.controller.prev_pregain = {"[Channel1]" : 0,
                                 "[Channel2]" : 0,
                                 "[Channel3]" : 0,
                                 "[Channel4]" : 0};
  this.controller.prev_browse = 0;
  this.controller.prev_loopmove = {"deck1" : 0,
                                   "deck2" : 0};
  this.controller.prev_loopsize = {"deck1" : 0,
                                   "deck2" : 0};
  this.controller.shift_pressed = {"deck1" : false,
                                   "deck2" : false};
  this.controller.wheelTouchInertiaTimer = {"[Channel1]" : 0,
                                            "[Channel2]" : 0,
                                            "[Channel3]" : 0,
                                            "[Channel4]" : 0};

  // TODO: convert to Object()s for easier logic.
  this.controller.last_tick_val = [0, 0];
  this.controller.last_tick_time = [0.0, 0.0];
  this.controller.sync_enabled_time = Object();

  // scratch overrides
  // TODO: these can probably be removed, or should be used in my custom scratch code.
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
  // "deck1" and "deck2" refer to the left deck or right deck, and may be Channel1 or 3 depending
  // on the deck switch state.  These are keywords in the HID library.
  MessageShort.addControl("deck1", "!shift", 0x0D, "B", 0x08);
  MessageShort.addControl("deck1", "!sync_enabled", 0x0D, "B", 0x04);
  MessageShort.addControl("deck1", "!cue_default", 0x0D, "B", 0x02);
  MessageShort.addControl("deck1", "!play", 0x0D, "B", 0x01);
  MessageShort.addControl("deck1", "!hotcue1", 0x0D, "B", 0x80);
  MessageShort.addControl("deck1", "!hotcue2", 0x0D, "B", 0x40);
  MessageShort.addControl("deck1", "!hotcue3", 0x0D, "B", 0x20);
  MessageShort.addControl("deck1", "!hotcue4", 0x0D, "B", 0x10);
  MessageShort.addControl("deck1", "!remix1", 0x0E, "B", 0x80);
  MessageShort.addControl("deck1", "!remix2", 0x0E, "B", 0x40);
  MessageShort.addControl("deck1", "!remix3", 0x0E, "B", 0x20);
  MessageShort.addControl("deck1", "!remix4", 0x0E, "B", 0x10);
  MessageShort.addControl("deck1", "loop_out", 0x0E, "B", 0x08);
  MessageShort.addControl("deck1", "loop_in", 0x0E, "B", 0x04);
  MessageShort.addControl("deck1", "slip_enabled", 0x0E, "B", 0x02);
  MessageShort.addControl("deck1", "!reset", 0x0E, "B", 0x01);
  MessageShort.addControl("deck1", "beatloop_8_activate", 0x13, "B", 0x02);
  MessageShort.addControl("deck1", "!loop_activate", 0x13, "B", 0x01);
  MessageShort.addControl("deck1", "!jog_touch", 0x11, "B", 0x01);
  MessageShort.addControl("deck1", "!jog_wheel", 0x01, "I");
  MessageShort.addControl("deck1", "!deckswitch", 0x0F, "B", 0x20);
  MessageShort.addControl("deck1", "!load_track", 0x0F, "B", 0x10);
  MessageShort.addControl("deck1", "!FX1", 0x12, "B", 0x80);
  MessageShort.addControl("deck1", "!FX2", 0x12, "B", 0x40);
  MessageShort.addControl("deck1", "!FX3", 0x12, "B", 0x20);
  MessageShort.addControl("deck1", "!FX4", 0x12, "B", 0x10);
  MessageShort.addControl("[EffectRack1_EffectUnit1]", "next_chain", 0x11, "B", 0x08);

  MessageShort.addControl("deck2", "!shift", 0x0C, "B", 0x08);
  MessageShort.addControl("deck2", "!sync_enabled", 0x0C, "B", 0x04);
  MessageShort.addControl("deck2", "!cue_default", 0x0C, "B", 0x02);
  MessageShort.addControl("deck2", "!play", 0x0C, "B", 0x01);
  MessageShort.addControl("deck2", "!hotcue1", 0x0C, "B", 0x80);
  MessageShort.addControl("deck2", "!hotcue2", 0x0C, "B", 0x40);
  MessageShort.addControl("deck2", "!hotcue3", 0x0C, "B", 0x20);
  MessageShort.addControl("deck2", "!hotcue4", 0x0C, "B", 0x10);
  MessageShort.addControl("deck2", "!remix1", 0x0B, "B", 0x80);
  MessageShort.addControl("deck2", "!remix2", 0x0B, "B", 0x40);
  MessageShort.addControl("deck2", "!remix3", 0x0B, "B", 0x20);
  MessageShort.addControl("deck2", "!remix4", 0x0B, "B", 0x10);
  MessageShort.addControl("deck2", "loop_out", 0x0B, "B", 0x08);
  MessageShort.addControl("deck2", "loop_in", 0x0B, "B", 0x04);
  MessageShort.addControl("deck2", "slip_enabled", 0x0B, "B", 0x02);
  MessageShort.addControl("deck2", "!reset", 0x0B, "B", 0x01);
  MessageShort.addControl("deck2", "beatloop_8_activate", 0x13, "B", 0x10);
  MessageShort.addControl("deck2", "!loop_activate", 0x13, "B", 0x08);
  MessageShort.addControl("deck2", "!jog_touch", 0x11, "B", 0x02);
  MessageShort.addControl("deck2", "!jog_wheel", 0x05, "I");
  MessageShort.addControl("deck2", "!deckswitch", 0x0A, "B", 0x20);
  MessageShort.addControl("deck2", "!load_track", 0x0A, "B", 0x10);
  MessageShort.addControl("deck2", "!FX1", 0x10, "B", 0x08);
  MessageShort.addControl("deck2", "!FX2", 0x10, "B", 0x04);
  MessageShort.addControl("deck2", "!FX3", 0x10, "B", 0x02);
  MessageShort.addControl("deck2", "!FX4", 0x10, "B", 0x01);
  MessageShort.addControl("[EffectRack1_EffectUnit2]", "next_chain", 0x11, "B", 0x04);

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
  MessageShort.addControl("[PreviewDeck1]", "!previewdeck", 0x0F, "B", 0x01);

  MessageShort.addControl("[Master]", "!quantize", 0x0A, "B", 0x08);

  MessageShort.setCallback("deck1", "!shift", this.shiftHandler);
  MessageShort.setCallback("deck2", "!shift", this.shiftHandler);
  MessageShort.setCallback("deck1", "!cue_default", this.cueHandler);
  MessageShort.setCallback("deck2", "!cue_default", this.cueHandler);
  MessageShort.setCallback("deck1", "!play", this.playHandler);
  MessageShort.setCallback("deck2", "!play", this.playHandler);
  MessageShort.setCallback("deck1", "!hotcue1", this.hotcueHandler);
  MessageShort.setCallback("deck1", "!hotcue2", this.hotcueHandler);
  MessageShort.setCallback("deck1", "!hotcue3", this.hotcueHandler);
  MessageShort.setCallback("deck1", "!hotcue4", this.hotcueHandler);
  MessageShort.setCallback("deck2", "!hotcue1", this.hotcueHandler);
  MessageShort.setCallback("deck2", "!hotcue2", this.hotcueHandler);
  MessageShort.setCallback("deck2", "!hotcue3", this.hotcueHandler);
  MessageShort.setCallback("deck2", "!hotcue4", this.hotcueHandler);
  MessageShort.setCallback("deck1", "!deckswitch", this.deckSwitchHandler);
  MessageShort.setCallback("deck2", "!deckswitch", this.deckSwitchHandler);
  MessageShort.setCallback("deck1", "!load_track", this.loadTrackHandler);
  MessageShort.setCallback("deck2", "!load_track", this.loadTrackHandler);
  MessageShort.setCallback("deck1", "!sync_enabled", this.syncEnabledHandler);
  MessageShort.setCallback("deck2", "!sync_enabled", this.syncEnabledHandler);
  MessageShort.setCallback("deck1", "!loop_activate", this.loopActivateHandler);
  MessageShort.setCallback("deck2", "!loop_activate", this.loopActivateHandler);
  MessageShort.setCallback("deck1", "!jog_touch", this.jogTouchHandler);
  MessageShort.setCallback("deck2", "!jog_touch", this.jogTouchHandler);
  MessageShort.setCallback("deck1", "!jog_wheel", this.jogMoveHandler);
  MessageShort.setCallback("deck2", "!jog_wheel", this.jogMoveHandler);
  MessageShort.setCallback("deck1", "!remix1", this.remixHandler);
  MessageShort.setCallback("deck1", "!remix2", this.remixHandler);
  MessageShort.setCallback("deck1", "!remix3", this.remixHandler);
  MessageShort.setCallback("deck1", "!remix4", this.remixHandler);
  MessageShort.setCallback("deck2", "!remix1", this.remixHandler);
  MessageShort.setCallback("deck2", "!remix2", this.remixHandler);
  MessageShort.setCallback("deck2", "!remix3", this.remixHandler);
  MessageShort.setCallback("deck2", "!remix4", this.remixHandler);
  MessageShort.setCallback("[PreviewDeck1]", "!previewdeck", this.previewDeckHandler);
  MessageShort.setCallback("[Master]", "!quantize", this.quantizeHandler);
  // TODO: the rest of the "!" controls.
  this.controller.registerInputPacket(MessageShort);

  // Most items in the long message are controls that go from 0-4096.
  // There are also some 4 bit encoders.
  MessageLong.addControl("deck1", "rate", 0x09, "H");
  MessageLong.addControl("deck2", "rate", 0x0B, "H");
  engine.softTakeover("[Channel1]", "rate", true);
  engine.softTakeover("[Channel2]", "rate", true);
  engine.softTakeover("[Channel3]", "rate", true);
  engine.softTakeover("[Channel4]", "rate", true);
  MessageLong.addControl("deck1", "!loopmove", 0x01, "B", 0x0F, undefined, true);
  MessageLong.addControl("deck2", "!loopmove", 0x02, "B", 0xF0, undefined, true);
  MessageLong.setCallback("deck1", "!loopmove", this.callbackLoopMove);
  MessageLong.setCallback("deck2", "!loopmove", this.callbackLoopMove);
  MessageLong.addControl("deck1", "!loopsize", 0x01, "B", 0xF0, undefined, true);
  MessageLong.addControl("deck2", "!loopsize", 0x03, "B", 0x0F, undefined, true);
  MessageLong.setCallback("deck1", "!loopsize", this.callbackLoopSize);
  MessageLong.setCallback("deck2", "!loopsize", this.callbackLoopSize);

  MessageLong.addControl("[EffectRack1_EffectUnit1]", "mix", 0x3F, "H");
  MessageLong.addControl("[EffectRack1_EffectUnit1_Effect1]", "parameter1", 0x41, "H");
  MessageLong.addControl("[EffectRack1_EffectUnit1_Effect1]", "parameter2", 0x43, "H");
  MessageLong.addControl("[EffectRack1_EffectUnit1_Effect1]", "parameter3", 0x45, "H");

  MessageLong.addControl("[EffectRack1_EffectUnit2]", "mix", 0x47, "H");
  MessageLong.addControl("[EffectRack1_EffectUnit2_Effect1]", "parameter1", 0x49, "H");
  MessageLong.addControl("[EffectRack1_EffectUnit2_Effect1]", "parameter2", 0x4B, "H");
  MessageLong.addControl("[EffectRack1_EffectUnit2_Effect1]", "parameter3", 0x4D, "H");

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

  // The physical master button controls the internal sound card volume, so if we hook this
  // up the adjustment is double-applied.
  //MessageLong.addControl("[Master]", "volume", 0x11, "H");
  MessageLong.addControl("[Master]", "crossfader", 0x07, "H");
  MessageLong.addControl("[Master]", "headMix", 0x0D, "H");
  MessageLong.addControl("[Playlist]", "!browse", 0x02, "B", 0x0F, undefined, true);
  MessageLong.setCallback("[Playlist]", "!browse", this.callbackBrowse);

  this.controller.setScaler("volume", this.scalerVolume);
  this.controller.setScaler("headMix", this.scalerSlider);
  this.controller.setScaler("parameter1", this.scalerParameter);
  this.controller.setScaler("parameter2", this.scalerParameter);
  this.controller.setScaler("parameter3", this.scalerParameter);
  this.controller.setScaler("super1", this.scalerParameter);
  this.controller.setScaler("crossfader", this.scalerSlider);
  this.controller.setScaler("rate", this.scalerSlider);
  this.controller.setScaler("mix", this.scalerParameter);
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
  Output1.addOutput("[Master]", "!quantize", 0x31, "B");
  Output1.addOutput("[InternalClock]", "sync_master", 0x30, "B");
  this.controller.registerOutputPacket(Output1);

  Output2.addOutput("deck1", "!shift", 0x1D, "B");
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

  Output2.addOutput("deck2", "!shift", 0x25, "B");
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

  Output2.addOutput("[PreviewDeck1]", "play_indicator", 0x3D, "B");

  // Note: this logic means remix button actions are not switchable without reloading the script.
  // Once we have support for controller preferences, this can be changed.
  if (TraktorS4MK2.RemixSlotButtonAction === "SAMPLES") {
    Output2.addOutput("[Sampler1]", "play_indicator", 0x19, "B");
    Output2.addOutput("[Sampler2]", "play_indicator", 0x1A, "B");
    Output2.addOutput("[Sampler3]", "play_indicator", 0x1B, "B");
    Output2.addOutput("[Sampler4]", "play_indicator", 0x1C, "B");
    Output2.addOutput("[Sampler5]", "play_indicator", 0x21, "B");
    Output2.addOutput("[Sampler6]", "play_indicator", 0x22, "B");
    Output2.addOutput("[Sampler7]", "play_indicator", 0x23, "B");
    Output2.addOutput("[Sampler8]", "play_indicator", 0x24, "B");
  } else if (TraktorS4MK2.RemixSlotButtonAction === "LOOPROLLS") {
    Output2.addOutput("deck1", "beatlooproll_0.125_activate", 0x19, "B");
    Output2.addOutput("deck1", "beatlooproll_0.25_activate", 0x1A, "B");
    Output2.addOutput("deck1", "beatlooproll_0.5_activate", 0x1B, "B");
    Output2.addOutput("deck1", "beatlooproll_1_activate", 0x1C, "B");
    Output2.addOutput("deck2", "beatlooproll_0.125_activate", 0x21, "B");
    Output2.addOutput("deck2", "beatlooproll_0.25_activate", 0x22, "B");
    Output2.addOutput("deck2", "beatlooproll_0.5_activate", 0x23, "B");
    Output2.addOutput("deck2", "beatlooproll_1_activate", 0x24, "B");
  }

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

  Output3.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel3]_enable", 0x05, "B");
  Output3.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel3]_enable", 0x06, "B");
  Output3.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x07, "B");
  Output3.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x08, "B");
  Output3.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x09, "B");
  Output3.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x0A, "B");
  Output3.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel4]_enable", 0x0B, "B");
  Output3.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel4]_enable", 0x0C, "B");

  Output3.addOutput("[EffectRack1_EffectUnit1]", "next_chain", 0x11, "B");
  Output3.addOutput("[EffectRack1_EffectUnit2]", "next_chain", 0x12, "B");

  this.controller.registerOutputPacket(Output3);

  // Link up control objects to their outputs
  TraktorS4MK2.linkDeckOutputs("sync_enabled", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("cue_indicator", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("play_indicator", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("hotcue_1_enabled", TraktorS4MK2.outputCueCallback);
  TraktorS4MK2.linkDeckOutputs("hotcue_2_enabled", TraktorS4MK2.outputCueCallback);
  TraktorS4MK2.linkDeckOutputs("hotcue_3_enabled", TraktorS4MK2.outputCueCallback);
  TraktorS4MK2.linkDeckOutputs("hotcue_4_enabled", TraktorS4MK2.outputCueCallback);
  TraktorS4MK2.linkDeckOutputs("loop_in", TraktorS4MK2.outputCallbackLoop);
  TraktorS4MK2.linkDeckOutputs("loop_out", TraktorS4MK2.outputCallbackLoop);
  TraktorS4MK2.linkDeckOutputs("keylock", TraktorS4MK2.outputCallbackDark);
  TraktorS4MK2.linkDeckOutputs("LoadSelectedTrack", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkDeckOutputs("slip_enabled", TraktorS4MK2.outputCallback);
  TraktorS4MK2.linkChannelOutput("[Channel1]", "pfl", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[Channel2]", "pfl", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[Channel3]", "pfl", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[Channel4]", "pfl", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[Channel1]", "track_samples", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[Channel2]", "track_samples", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[Channel3]", "track_samples", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[Channel4]", "track_samples", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[Channel1]", "PeakIndicator", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.linkChannelOutput("[Channel2]", "PeakIndicator", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.linkChannelOutput("[Channel3]", "PeakIndicator", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.linkChannelOutput("[Channel4]", "PeakIndicator", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.linkChannelOutput("[Master]", "PeakIndicatorL", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.linkChannelOutput("[Master]", "PeakIndicatorR", TraktorS4MK2.outputChannelCallbackDark);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit1]", "group_[Channel3]_enable", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit2]", "group_[Channel3]_enable", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit1]", "group_[Channel4]_enable", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit2]", "group_[Channel4]_enable", TraktorS4MK2.outputChannelCallback);

  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit1]", "next_chain", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[EffectRack1_EffectUnit2]", "next_chain", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[PreviewDeck1]", "play_indicator", TraktorS4MK2.outputChannelCallback);
  TraktorS4MK2.linkChannelOutput("[InternalClock]", "sync_master", TraktorS4MK2.outputChannelCallback);

  if (TraktorS4MK2.RemixSlotButtonAction === "SAMPLES") {
    TraktorS4MK2.linkChannelOutput("[Sampler1]", "play_indicator", TraktorS4MK2.outputChannelCallback);
    TraktorS4MK2.linkChannelOutput("[Sampler2]", "play_indicator", TraktorS4MK2.outputChannelCallback);
    TraktorS4MK2.linkChannelOutput("[Sampler3]", "play_indicator", TraktorS4MK2.outputChannelCallback);
    TraktorS4MK2.linkChannelOutput("[Sampler4]", "play_indicator", TraktorS4MK2.outputChannelCallback);
    TraktorS4MK2.linkChannelOutput("[Sampler5]", "play_indicator", TraktorS4MK2.outputChannelCallback);
    TraktorS4MK2.linkChannelOutput("[Sampler6]", "play_indicator", TraktorS4MK2.outputChannelCallback);
    TraktorS4MK2.linkChannelOutput("[Sampler7]", "play_indicator", TraktorS4MK2.outputChannelCallback);
    TraktorS4MK2.linkChannelOutput("[Sampler8]", "play_indicator", TraktorS4MK2.outputChannelCallback);
  } else if (TraktorS4MK2.RemixSlotButtonAction === "LOOPROLLS") {
    TraktorS4MK2.linkDeckOutputs("beatlooproll_0.125_activate", TraktorS4MK2.outputCallback);
    TraktorS4MK2.linkDeckOutputs("beatlooproll_0.25_activate", TraktorS4MK2.outputCallback);
    TraktorS4MK2.linkDeckOutputs("beatlooproll_0.5_activate", TraktorS4MK2.outputCallback);
    TraktorS4MK2.linkDeckOutputs("beatlooproll_1_activate", TraktorS4MK2.outputCallback);
  }

  // VU meters get special attention
  engine.connectControl("[Channel1]", "VuMeter", "TraktorS4MK2.onVuMeterChanged");
  engine.connectControl("[Channel2]", "VuMeter", "TraktorS4MK2.onVuMeterChanged");
  engine.connectControl("[Channel3]", "VuMeter", "TraktorS4MK2.onVuMeterChanged");
  engine.connectControl("[Channel4]", "VuMeter", "TraktorS4MK2.onVuMeterChanged");

  engine.connectControl("[Channel1]", "loop_enabled", "TraktorS4MK2.onLoopEnabledChanged");
  engine.connectControl("[Channel2]", "loop_enabled", "TraktorS4MK2.onLoopEnabledChanged");
  engine.connectControl("[Channel3]", "loop_enabled", "TraktorS4MK2.onLoopEnabledChanged");
  engine.connectControl("[Channel4]", "loop_enabled", "TraktorS4MK2.onLoopEnabledChanged");
}

TraktorS4MK2.linkDeckOutputs = function(key, callback) {
  // Linking outputs is a little tricky because the library doesn't quite do what I want.  But this
  // method works.
  TraktorS4MK2.controller.linkOutput("deck1", key, "[Channel1]", key, callback);
  engine.connectControl("[Channel3]", key, callback);
  TraktorS4MK2.controller.linkOutput("deck2", key, "[Channel2]", key, callback);
  engine.connectControl("[Channel4]", key, callback);
}

TraktorS4MK2.linkChannelOutput = function(group, key, callback) {
  TraktorS4MK2.controller.linkOutput(group, key, group, key, callback);
}

TraktorS4MK2.lightGroup = function(packet, output_group_name, co_group_name) {
  var group_ob = packet.groups[output_group_name];
  for (var field_name in group_ob) {
    field = group_ob[field_name];
    if (field.name[0] === "!") {
      continue;
    }
    if (field.mapped_callback) {
      var value = engine.getValue(co_group_name, field.name);
      field.mapped_callback(value, co_group_name, field.name);
    }
    // No callback, no light!
  }
}

TraktorS4MK2.lightDeck = function(group) {
  // Freeze the lights while we do this update so we don't spam HID.
  this.controller.freeze_lights = true;
  for (var packet_name in this.controller.OutputPackets) {
    packet = this.controller.OutputPackets[packet_name];
    var deck_group_name = "deck1";
    if (group === "[Channel2]" || group === "[Channel4]") {
      deck_group_name = "deck2";
    }

    TraktorS4MK2.lightGroup(packet, deck_group_name, group);
    TraktorS4MK2.lightGroup(packet, group, group);
    // Shift is a weird key because there's no CO that it is actually associated with.
    TraktorS4MK2.outputCallback(0, group, "!shift");
  }

  // FX buttons
  var packet = this.controller.OutputPackets["output3"];
  TraktorS4MK2.lightGroup(packet, "[EffectRack1_EffectUnit1]", "[EffectRack1_EffectUnit1]");
  TraktorS4MK2.lightGroup(packet, "[EffectRack1_EffectUnit2]", "[EffectRack1_EffectUnit2]");

  // Selected deck lights
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

TraktorS4MK2.pointlessLightShow = function() {
  var packets = [Object(), Object(), Object()];

  packets[0].length = 53;
  packets[1].length = 63;
  packets[2].length = 61;

  // Fade up all lights evenly from 0 to 0x7F
  for (k = 0; k < 0x7F; k+=0x05) {
    for (var i = 0; i < packets.length; i++) {
      // Packet header
      packets[i][0] = 0x80 + i;
      for (j = 1; j < packets[i].length; j++) {
        packets[i][j] = k;
      }
    }
    controller.send(packets[0], packets[0].length, 0);
    controller.send(packets[1], packets[1].length, 0);
    controller.send(packets[2], packets[2].length, 0);
    // "sleep"
    var then = Date.now();
    while (true) {
      var now = Date.now();
      if (now - then > 25) {
        break;
      }
    }
  }
}

TraktorS4MK2.init = function(id) {
  TraktorS4MK2.pointlessLightShow()
  TraktorS4MK2.registerInputPackets()
  TraktorS4MK2.registerOutputPackets()

  // Initialize master quantize based on the state of Channel1.  It's the best we can do for now
  // until we have controller preferences.
  TraktorS4MK2.master_quantize = engine.getValue("[Channel1]", "quantize");
  engine.setValue("[Channel1]", "quantize", TraktorS4MK2.master_quantize);
  engine.setValue("[Channel2]", "quantize", TraktorS4MK2.master_quantize);
  engine.setValue("[Channel3]", "quantize", TraktorS4MK2.master_quantize);
  engine.setValue("[Channel4]", "quantize", TraktorS4MK2.master_quantize);
  TraktorS4MK2.controller.setOutput("[Master]", "!quantize", 0x7F * TraktorS4MK2.master_quantize, true);

  TraktorS4MK2.controller.setOutput("[Master]", "!usblight", 0x7F, true);
  TraktorS4MK2.outputChannelCallback(engine.getValue("[InternalClock]", "sync_master"), "[InternalClock]", "sync_master");
  TraktorS4MK2.lightDeck("[PreviewDeck1]");
  // Light 3 and 4 first so we get the mixer lights on, then do 1 and 2 since those are active
  // on startup.
  TraktorS4MK2.lightDeck("[Channel3]");
  TraktorS4MK2.lightDeck("[Channel4]");
  TraktorS4MK2.lightDeck("[Channel1]");
  TraktorS4MK2.lightDeck("[Channel2]");

  HIDDebug("TraktorS4MK2: done init");
}

TraktorS4MK2.debugLights = function() {
  // Call this if you want to just send raw packets to the controller (good for figuring out what
  // bytes do what).
  //var data_strings = ["80 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 00 00 00 00 00 00 00 0A 0A 0A 0A 0A 0A 0A 0A 0A 00 7F 00 00 00 00 0A 0A 0A 0A 0A 0A",
  //                    "81 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0B 03 00 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 7F 0A 0A 0A 0A 0A 7F 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A",
  //                    "82 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 0A 00 00 7F 7F 7F 7F 7F 7F 00 00 7F 7F 7F 7F 7F 00 00 00 7F 7F 7F 7F 7F 7F 00 00 7F 7F 7F 7F 7F 00 00 00"];
  //                   00 01 02 03  04 05 06 07  08 09 0A 0B  0C 0D 0E 0F
  var data_strings = ["80 00 00 00  00 00 00 00  0A 00 00 00  00 00 00 00  \n" +
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
      controller.send(data[i], data[i].length, 0);
    }
  }
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
    // Keep USB light on though.
    if (i === 0) {
      data[0x2A] = 0x7F;
    }
    controller.send(data, packet_length, 0);
  }
}

// Called by Mixxx -- mandatory function to receive anything from HID
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
      HIDDebug("Traktor S4MK2: Received second half of message but don't have first half, ignoring");
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

  HIDDebug("Traktor S4MK2: Unhandled packet size: " + length);
}

// The short message handles buttons and jog wheels.
TraktorS4MK2.shortMessageCallback = function(packet, data) {
  for (name in data) {
    field = data[name];
    // Rewrite group name from "deckX" to "[ChannelY]"
    group = TraktorS4MK2.getGroupFromButton(field.id);
    field.group = group;
    if (field.name === "!jog_wheel") {
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
    // Rewrite group name from "deckX" to "[ChannelY]"
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
    HIDDebug("Traktor S4MK2: Tried to set from simple packet but not exactly one period in name: " + name);
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
    HIDDebug("Traktor S4MK2: Unrecognized packet group: " + splitted[0]);
    return "";
  }
}

TraktorS4MK2.shiftHandler = function(field) {
  var group = field.id.split(".")[0];
  TraktorS4MK2.controller.shift_pressed[group] = field.value;
  TraktorS4MK2.outputCallback(field.value, field.group, "!shift");
}

TraktorS4MK2.deckSwitchHandler = function(field) {
  if (field.value === 0) {
    return;
  }

  // The group is the currently-assigned value, so set the variable to the opposite.
  if (field.group === "[Channel1]") {
    TraktorS4MK2.controller.left_deck_C = true;
    TraktorS4MK2.lightDeck("[Channel3]");
    engine.softTakeoverIgnoreNextValue("[Channel3]", "rate");
  } else if (field.group === "[Channel3]") {
    TraktorS4MK2.controller.left_deck_C = false;
    TraktorS4MK2.lightDeck("[Channel1]");
    engine.softTakeoverIgnoreNextValue("[Channel1]", "rate");
  } else if (field.group === "[Channel2]") {
    TraktorS4MK2.controller.right_deck_D = true;
    TraktorS4MK2.lightDeck("[Channel4]");
    engine.softTakeoverIgnoreNextValue("[Channel4]", "rate");
  } else if (field.group === "[Channel4]") {
    TraktorS4MK2.controller.right_deck_D = false;
    TraktorS4MK2.lightDeck("[Channel2]");
    engine.softTakeoverIgnoreNextValue("[Channel2]", "rate");
  } else {
    HIDDebug("Traktor S4MK2: Unrecognized packet group: " + field.group);
  }
}

TraktorS4MK2.loadTrackHandler = function(field) {
  var splitted = field.id.split(".");
  var group = splitted[0];
  if (TraktorS4MK2.controller.shift_pressed[group]) {
    engine.setValue(field.group, "eject", field.value);
  } else {
    engine.setValue(field.group, "LoadSelectedTrack", field.value);
  }
}

TraktorS4MK2.syncEnabledHandler = function(field) {
  var now = Date.now();

  var splitted = field.id.split(".");
  var group = splitted[0];
  // If shifted, just toggle.
  // TODO(later version): actually make this enable explicit master.
  if (TraktorS4MK2.controller.shift_pressed[group]) {
    if (field.value === 0) {
      return;
    }
    var synced = engine.getValue(field.group, "sync_enabled");
    engine.setValue(field.group, "sync_enabled", !synced);
  } else {
    if (field.value === 1) {
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
}

TraktorS4MK2.loopActivateHandler = function(field) {
  var splitted = field.id.split(".");
  var group = splitted[0];
  if (TraktorS4MK2.controller.shift_pressed[group]) {
    engine.setValue(field.group, "pitch_adjust_set_default", field.value);
  } else {
    engine.setValue(field.group, "reloop_exit", field.value);
  }
}

TraktorS4MK2.cueHandler = function(field) {
  var splitted = field.id.split(".");
  var group = splitted[0];
  if (TraktorS4MK2.controller.shift_pressed[group]) {
    if (TraktorS4MK2.ShiftCueButtonAction == "REWIND") {
      if (field.value === 0) {
        return;
      }
      engine.setValue(field.group, "start_stop", 1);
    } else if (TraktorS4MK2.ShiftCueButtonAction == "REVERSEROLL") {
      engine.setValue(field.group, "reverseroll", field.value);
    } else {
      print ("Traktor S4 WARNING: Invalid ShiftCueButtonAction picked.  Must be either REWIND " +
           "or REVERSEROLL");
    }
  } else {
    engine.setValue(field.group, "cue_default", field.value);
  }
}

TraktorS4MK2.playHandler = function(field) {
  if (field.value === 0) {
    return;
  }
  var splitted = field.id.split(".");
  var group = splitted[0];
  if (TraktorS4MK2.controller.shift_pressed[group]) {
    var locked = engine.getValue(field.group, "keylock");
    engine.setValue(field.group, "keylock", !locked);
  } else {
    var playing = engine.getValue(field.group, "play");
    engine.setValue(field.group, "play", !playing);
  }
}

TraktorS4MK2.previewDeckHandler = function(field) {
  if (field.value === 0) {
    return;
  }
  // TODO: figure out a way to know if the browse position has changed.  If not, the preview
  // button should pause / resume. If it has changed, preview button loads new track.
  /*if (engine.getValue("[PreviewDeck1]", "play")) {
    engine.setValue("[PreviewDeck1]", "cue_gotoandstop", 1);
    engine.setValue("[PreviewDeck1]", "cue_gotoandstop", 0);
  } else {*/
  engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
  engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 0);
  //}
}

// Jog wheel touch code taken from VCI400.  It should be moved into common-hid-packet-parser.js
TraktorS4MK2.jogTouchHandler = function(field) {
  if (TraktorS4MK2.controller.wheelTouchInertiaTimer[field.group] != 0) {
    // The wheel was touched again, reset the timer.
    engine.stopTimer(TraktorS4MK2.controller.wheelTouchInertiaTimer[field.group]);
    TraktorS4MK2.controller.wheelTouchInertiaTimer[field.group] = 0;
  }
  if (field.value !== 0) {
    var deckNumber = TraktorS4MK2.controller.resolveDeck(group);
    engine.scratchEnable(deckNumber, 1024, 33.3333, 0.125, 0.125/8, true);
  } else {
    // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
    // Depending on how fast the platter was moving, lengthen the time we'll wait.
    var scratchRate = Math.abs(engine.getValue(field.group, "scratch2"));
    // Note: inertiaTime multiplier is controller-specific and should be factored out.
    var inertiaTime = Math.pow(1.8, scratchRate) * 2;
    if (inertiaTime < 100) {
      // Just do it now.
      TraktorS4MK2.finishJogTouch(field.group);
    } else {
      TraktorS4MK2.controller.wheelTouchInertiaTimer[field.group] = engine.beginTimer(
          inertiaTime, "TraktorS4MK2.finishJogTouch(\"" + field.group + "\")", true);
    }
  }
}

TraktorS4MK2.finishJogTouch = function(group) {
  TraktorS4MK2.controller.wheelTouchInertiaTimer[group] = 0;
  var deckNumber = TraktorS4MK2.controller.resolveDeck(group);
  // No vinyl button (yet)
  /*if (this.vinylActive) {
    // Vinyl button still being pressed, don't disable scratch mode yet.
    this.wheelTouchInertiaTimer[group] = engine.beginTimer(
        100, "VestaxVCI400.Decks." + this.deckIdentifier + ".finishJogTouch()", true);
    return;
  }*/
  var play = engine.getValue(group, "play");
  if (play != 0) {
    // If we are playing, just hand off to the engine.
    engine.scratchDisable(deckNumber, true);
  } else {
    // If things are paused, there will be a non-smooth handoff between scratching and jogging.
    // Instead, keep scratch on until the platter is not moving.
    var scratchRate = Math.abs(engine.getValue(group, "scratch2"));
    if (scratchRate < 0.01) {
      // The platter is basically stopped, now we can disable scratch and hand off to jogging.
      engine.scratchDisable(deckNumber, false);
    } else {
      // Check again soon.
      TraktorS4MK2.controller.wheelTouchInertiaTimer[group] = engine.beginTimer(
              100, "TraktorS4MK2.finishJogTouch(\"" + group + "\")", true);
    }
  }
}

TraktorS4MK2.jogMoveHandler = function(field) {
  var deltas = TraktorS4MK2.wheelDeltas(field.group, field.value);
  var tick_delta = deltas[0];
  var time_delta = deltas[1];

  var velocity = TraktorS4MK2.scalerJog(tick_delta, time_delta);
  engine.setValue(field.group, "jog", velocity);
  if (engine.getValue(field.group, "scratch2_enable")) {
    var deckNumber = TraktorS4MK2.controller.resolveDeck(group);
    engine.scratchTick(deckNumber, tick_delta);
  }
};

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
  //HIDDebug(group + " " + tickval + " " + prev_tick + " " + tick_delta);
  return [tick_delta, time_delta];
}

TraktorS4MK2.scalerJog = function(tick_delta, time_delta) {
  if (engine.getValue(group, "play")) {
    return (tick_delta / time_delta) / 3;
  } else {
    return (tick_delta / time_delta) * 2.0;
  }
}

TraktorS4MK2.hotcueHandler = function(field) {
  var group = field.id.split(".")[0];
  var buttonNumber = parseInt(field.name[field.name.length - 1]);
  if (TraktorS4MK2.controller.shift_pressed[group]) {
    engine.setValue(field.group, "hotcue_" + buttonNumber + "_clear", field.value);
  } else {
    engine.setValue(field.group, "hotcue_" + buttonNumber + "_activate", field.value);
  }
}

TraktorS4MK2.remixHandler = function(field) {
  var group = field.id.split(".")[0];

  if (TraktorS4MK2.RemixSlotButtonAction === "SAMPLES") {
    if (field.value !== 1) {
      return;
    }
    var buttonNumber = parseInt(field.name[field.name.length - 1]);
    if (group === "deck2") {
      buttonNumber = buttonNumber + 4;
    }
    if (TraktorS4MK2.controller.shift_pressed[group]) {
      engine.setValue("[Sampler" + buttonNumber + "]", "eject", 1);
    } else {
      engine.setValue("[Sampler" + buttonNumber + "]", "cue_gotoandplay", 1);
    }
  } else if (TraktorS4MK2.RemixSlotButtonAction === "LOOPROLLS") {
    var buttonNumber = parseInt(field.name[field.name.length - 1]);
    // If shift is held, create a quick loop of convenient size
    if (TraktorS4MK2.controller.shift_pressed[group]) {
      if (field.value !== 1) {
        return;
      }
      var loop_size = Math.pow(2, buttonNumber + 1);
      engine.setValue(field.group, "beatloop_" + loop_size + "_activate", field.value);
    } else {
      var loop_size = Math.pow(2, buttonNumber - 4);
      engine.setValue(field.group, "beatlooproll_" + loop_size + "_activate", field.value);
    }
  } else {
    print ("Traktor S4 WARNING: Invalid RemixSlotButtonAction picked.  Must be either SAMPLES " +
           "or LOOPROLLS");
  }
}

TraktorS4MK2.quantizeHandler = function(field) {
  if (field.value === 0) {
    return;
  }
  TraktorS4MK2.master_quantize = !TraktorS4MK2.master_quantize;
  engine.setValue("[Channel1]", "quantize", TraktorS4MK2.master_quantize);
  engine.setValue("[Channel2]", "quantize", TraktorS4MK2.master_quantize);
  engine.setValue("[Channel3]", "quantize", TraktorS4MK2.master_quantize);
  engine.setValue("[Channel4]", "quantize", TraktorS4MK2.master_quantize);
  TraktorS4MK2.controller.setOutput("[Master]", "!quantize", 0x7F * TraktorS4MK2.master_quantize, true);
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

TraktorS4MK2.callbackLoopMove = function(field) {
  // TODO: common-hid-packet-parser looks like it should do deltas, but I can't get them to work.
  var splitted = field.id.split(".");
  var group = splitted[0]
  prev_loopmove = TraktorS4MK2.controller.prev_loopmove[group];
  TraktorS4MK2.controller.prev_loopmove[group] = field.value;
  var delta = 0;
  if (prev_loopmove === 15 && field.value === 0) {
    delta = 1;
  } else if (prev_loopmove === 0 && field.value === 15) {
    delta = -1;
  } else if (field.value > prev_loopmove) {
    delta = 1;
  } else {
    delta = -1;
  }

  // Shift mode: adjust musical key
  if (TraktorS4MK2.controller.shift_pressed[group]) {
    if (delta == 1) {
      engine.setValue(field.group, "pitch_up_small", 1);
      engine.setValue(field.group, "pitch_up_small", 0);
    } else {
      engine.setValue(field.group, "pitch_down_small", 1);
      engine.setValue(field.group, "pitch_down_small", 0);
    }
  } else {
    engine.setValue(field.group, "loop_move", delta);
  }
}

TraktorS4MK2.callbackLoopSize = function(field) {
  var splitted = field.id.split(".");
  var group = splitted[0]
  prev_loopsize = TraktorS4MK2.controller.prev_loopsize[group];
  TraktorS4MK2.controller.prev_loopsize[group] = field.value;
  var delta = 0;
  if (prev_loopsize === 15 && field.value === 0) {
    delta = 1;
  } else if (prev_loopsize === 0 && field.value === 15) {
    delta = -1;
  } else if (field.value > prev_loopsize) {
    delta = 1;
  } else {
    delta = -1;
  }

  if (TraktorS4MK2.controller.shift_pressed[group]) {
    var playPosition = engine.getValue(field.group, "playposition")
    if (delta == 1) {
      playPosition += 0.0125;
    } else {
      playPosition -= 0.0125;
    }
    engine.setValue(field.group, "playposition", playPosition);
  } else {
    if (delta == 1) {
      engine.setValue(field.group, "loop_double", 1);
      engine.setValue(field.group, "loop_double", 0);
    } else {
      engine.setValue(field.group, "loop_halve", 1);
      engine.setValue(field.group, "loop_halve", 0);
    }
  }
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

TraktorS4MK2.scalerParameter = function(group, name, value) {
  return script.absoluteLin(value, 0, 1, 16, 4080);
}
// Tell the HIDController script to use setParameter instead of setValue.
TraktorS4MK2.scalerParameter.useSetParameter = true;

TraktorS4MK2.scalerVolume = function(group, name, value) {
  if (group === "[Master]") {
    return script.absoluteNonLin(value, 0, 1, 4, 16, 4080);
  } else {
    return script.absoluteNonLin(value, 0, 0.25, 1, 16, 4080);
  }
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
  var led_value = 0x05;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS4MK2.controller.setOutput(group, key, led_value, !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.outputChannelCallbackDark = function(value,group,key) {
  var led_value = 0x00;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS4MK2.controller.setOutput(group, key, led_value, !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.outputCallback = function(value,group,key) {
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

TraktorS4MK2.outputCallbackLoop = function(value,group,key) {
  var deck_group = TraktorS4MK2.resolveDeckIfActive(group);
  if (deck_group === undefined) {
    return;
  }

  var led_value = 0x09;
  if (engine.getValue(group, "loop_enabled")) {
    led_value = 0x7F;
  }
  TraktorS4MK2.controller.setOutput(deck_group, key, led_value, !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.outputCallbackDark = function(value,group,key) {
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
    return;
  }

  var RGB_value = [0, 0, 0];
  // Use different colors for decks 1/2 and 3/4 that match LateNight (red and blue).
  if (group === "[Channel1]" || group === "[Channel2]") {
    if (value === 1) {
      RGB_value = [0x40, 0x02, 0x02];
    } else {
      RGB_value = [0x08, 0x01, 0x01];
    }
  } else {
    if (value === 1) {
      RGB_value = [0x0A, 0x35, 0x35];
    } else {
      RGB_value = [0x02, 0x10, 0x10];
    }
  }

  TraktorS4MK2.controller.setOutput(deck_group, key, RGB_value[0], false);
  TraktorS4MK2.controller.setOutput(deck_group, "!" + key + "_G", RGB_value[1], false);
  TraktorS4MK2.controller.setOutput(deck_group, "!" + key + "_B", RGB_value[2], !TraktorS4MK2.controller.freeze_lights);
}

TraktorS4MK2.onVuMeterChanged = function(value, group, key) {
  // This handler is called a lot so it should be as fast as possible.

  // VU is drawn on 6 segments, the 7th indicates clip.
  var VuOffsets = {"[Channel1]" : 0x09,
                   "[Channel2]" : 0x11,
                   "[Channel3]" : 0x01,
                   "[Channel4]" : 0x19};

  // Figure out number of fully-illuminated segments.
  var scaledValue = value * 6.0;
  var fullIllumCount = Math.floor(scaledValue);

  // Figure out how much the partially-illuminated segment is illuminated.
  var partialIllum = (scaledValue - fullIllumCount) * 0x7F

  for (i = 0; i < 6; i++) {
    var key = "!" + "VuMeter" + i;
    if (i < fullIllumCount) {
      // Don't update lights until they're all done, so the last term is false.
      TraktorS4MK2.controller.setOutput(group, key, 0x7F, false);
    } else if (i == fullIllumCount) {
      TraktorS4MK2.controller.setOutput(group, key, partialIllum, false);
    } else {
      TraktorS4MK2.controller.setOutput(group, key, 0x00, false);
    }
  }
  TraktorS4MK2.controller.OutputPackets["output1"].send();
}

TraktorS4MK2.onLoopEnabledChanged = function(value, group, key) {
  TraktorS4MK2.outputCallbackLoop(value, group, "loop_in");
  TraktorS4MK2.outputCallbackLoop(value, group, "loop_out");
}

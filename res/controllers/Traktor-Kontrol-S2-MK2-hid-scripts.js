/****************************************************************/
/*      Traktor Kontrol S2 MK2 HID controller script            */
/*      Copyright (C) 2020, Be <be@mixxx.org>                   */
/*      Copyright (C) 2017, douteiful                           */
/*      Based on:                                               */
/*      Traktor Kontrol S4 MK2 HID controller script v1.00      */
/*      Copyright (C) 2015, the Mixxx Team                      */
/*      but feel free to tweak this to your heart's content!    */
/****************************************************************/

// ==== Friendly User Configuration ====
// The Cue button, when Shift is also held, can have two possible functions:
// 1. "REWIND": seeks to the very start of the track.
// 2. "REVERSEROLL": performs a temporary reverse or "censor" effect, where the track
//    is momentarily played in reverse until the button is released.
ShiftCueButtonAction = "REWIND";

TraktorS2MK2 = new function() {
  this.controller = new HIDController();

  // When true, packets will not be sent to the controller.
  // Used when updating multiple LEDs simultaneously.
  this.freeze_lights = false;

  // Previous values, used for calculating deltas for encoder knobs.
  this.previous_browse = 0;
  this.previous_pregain = {
    "[Channel1]": 0,
    "[Channel2]": 0
  };
  this.previous_leftencoder = {
    "[Channel1]": 0,
    "[Channel2]": 0
  };
  this.previous_leftencoder = {
    "[Channel1]": 0,
    "[Channel2]": 0
  };
  this.wheelTouchInertiaTimer = {
    "[Channel1]": 0,
    "[Channel2]": 0
  };
  
  this.top_encoder_pressed = {
    "[Channel1]": false,
    "[Channel2]": false
  };
  this.left_encoder_pressed = {
    "[Channel1]": false,
    "[Channel2]": false
  };
  this.shift_pressed = {
    "[Channel1]": false,
    "[Channel2]": false
  };
                                   
  this.pad_modes = {
    "hotcue": 0,
    "intro_outro": 1,
    "sampler": 2
  };
  this.current_pad_mode = {
    "[Channel1]": this.pad_modes.hotcue,
    "[Channel2]": this.pad_modes.hotcue
  };
  this.pad_connections = {
    "[Channel1]": [],
    "[Channel2]": []
  };

  this.last_tick_val = [0, 0];
  this.last_tick_time = [0.0, 0.0];
  this.sync_enabled_time = {};
  
  this.longPressTimeout = 275; // milliseconds
  
  this.effectButtonLongPressTimer = {
    "[EffectRack1_EffectUnit1]": [0, 0, 0, 0],
    "[EffectRack1_EffectUnit2]": [0, 0, 0, 0]
  };
  this.effectButtonIsLongPressed = {
    "[EffectRack1_EffectUnit1]": [false, false, false, false],
    "[EffectRack1_EffectUnit2]": [false, false, false, false]
  };
  this.effectFocusLongPressTimer = {
    "[EffectRack1_EffectUnit1]": 0,
    "[EffectRack1_EffectUnit2]": 0
  };
  this.effectFocusChooseModeActive = {
    "[EffectRack1_EffectUnit1]": false,
    "[EffectRack1_EffectUnit2]": false
  };
  this.effectFocusButtonPressedWhenParametersHidden = {
    "[EffectRack1_EffectUnit1]": false,
    "[EffectRack1_EffectUnit2]": false
  };
  this.previouslyFocusedEffect = {
    "[EffectRack1_EffectUnit1]": null,
    "[EffectRack1_EffectUnit2]": null
  };
  this.effectButtonLEDconnections = {
    "[EffectRack1_EffectUnit1]": [],
    "[EffectRack1_EffectUnit2]": []
  };
}

TraktorS2MK2.registerInputPackets = function() {
  var MessageShort = new HIDPacket("shortmessage", 0x01, this.shortMessageCallback);
  var MessageLong = new HIDPacket("longmessage", 0x02, this.longMessageCallback);

  // Values in the short message are all buttons, except the jog wheels.
  // An exclamation point indicates a specially-handled function.  Everything else is a standard
  // Mixxx control object name.
  // "[Channel1]" and "[Channel2]" refer to the left deck or right deck, and may be Channel1 or 3 depending
  // on the deck switch state.  These are keywords in the HID library.
  
  MessageShort.addControl("[Channel1]", "!top_encoder_press", 0x0D, "B", 0x40);
  MessageShort.addControl("[Channel2]", "!top_encoder_press", 0x0D, "B", 0x80);
  
  MessageShort.addControl("[Channel1]", "!shift", 0x0B, "B", 0x08);
  MessageShort.addControl("[Channel1]", "!sync_enabled", 0x0B, "B", 0x04);
  MessageShort.addControl("[Channel1]", "!cue_default", 0x0B, "B", 0x02);
  MessageShort.addControl("[Channel1]", "!play", 0x0B, "B", 0x01);
  MessageShort.addControl("[Channel1]", "!pad1", 0x0B, "B", 0x80);
  MessageShort.addControl("[Channel1]", "!pad2", 0x0B, "B", 0x40);
  MessageShort.addControl("[Channel1]", "!pad3", 0x0B, "B", 0x20);
  MessageShort.addControl("[Channel1]", "!pad4", 0x0B, "B", 0x10);
  MessageShort.addControl("[Channel1]", "!loop_in", 0x0C, "B", 0x40);
  MessageShort.addControl("[Channel1]", "!loop_out", 0x0C, "B", 0x80);
  MessageShort.addControl("[Channel1]", "!remix_button", 0x0C, "B", 0x02);
  MessageShort.addControl("[Channel1]", "!flux_button", 0x0C, "B", 0x20);
  MessageShort.addControl("[Channel1]", "!left_encoder_press", 0x0F, "B", 0x01);
  MessageShort.addControl("[Channel1]", "!right_encoder_press", 0x0F, "B", 0x02);
  MessageShort.addControl("[Channel1]", "!jog_touch", 0x0A, "B", 0x01);
  MessageShort.addControl("[Channel1]", "!jog_wheel", 0x01, "I");
  MessageShort.addControl("[Channel1]", "!load_track", 0x0C, "B", 0x08);
  MessageShort.addControl("[EffectRack1_EffectUnit1]", "!effect_focus_button", 0x0E, "B", 0x10);
  // closing "]" intentionally left off here to make string manipulation a bit easier
  MessageShort.addControl("[EffectRack1_EffectUnit1", "!effectbutton1", 0x0E, "B", 0x80);
  MessageShort.addControl("[EffectRack1_EffectUnit1", "!effectbutton2", 0x0E, "B", 0x40);
  MessageShort.addControl("[EffectRack1_EffectUnit1", "!effectbutton3", 0x0E, "B", 0x20);

  MessageShort.addControl("[Channel2]", "!shift", 0x09, "B", 0x08);
  MessageShort.addControl("[Channel2]", "!sync_enabled", 0x09, "B", 0x04);
  MessageShort.addControl("[Channel2]", "!cue_default", 0x09, "B", 0x02);
  MessageShort.addControl("[Channel2]", "!play", 0x09, "B", 0x01);
  MessageShort.addControl("[Channel2]", "!pad1", 0x09, "B", 0x80);
  MessageShort.addControl("[Channel2]", "!pad2", 0x09, "B", 0x40);
  MessageShort.addControl("[Channel2]", "!pad3", 0x09, "B", 0x20);
  MessageShort.addControl("[Channel2]", "!pad4", 0x09, "B", 0x10);
  MessageShort.addControl("[Channel2]", "!loop_in", 0x0A, "B", 0x40);
  MessageShort.addControl("[Channel2]", "!loop_out", 0x0A, "B", 0x80);
  MessageShort.addControl("[Channel2]", "!remix_button", 0x0C, "B", 0x01);
  MessageShort.addControl("[Channel2]", "!flux_button", 0x0A, "B", 0x20);
  MessageShort.addControl("[Channel2]", "!left_encoder_press", 0x0F, "B", 0x08);
  MessageShort.addControl("[Channel2]", "!right_encoder_press", 0x0F, "B", 0x10);
  MessageShort.addControl("[Channel2]", "!jog_touch", 0x0A, "B", 0x02);
  MessageShort.addControl("[Channel2]", "!jog_wheel", 0x05, "I");
  MessageShort.addControl("[Channel2]", "!load_track", 0x0C, "B", 0x04);
  MessageShort.addControl("[EffectRack1_EffectUnit2]", "!effect_focus_button", 0xD, "B", 0x04);
  MessageShort.addControl("[EffectRack1_EffectUnit2", "!effectbutton1", 0xD, "B", 0x20);
  MessageShort.addControl("[EffectRack1_EffectUnit2", "!effectbutton2", 0xD, "B", 0x10);
  MessageShort.addControl("[EffectRack1_EffectUnit2", "!effectbutton3", 0xD, "B", 0x08);

  MessageShort.addControl("[Channel1]", "!pfl", 0x0C, "B", 0x10);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel1]_enable", 0x0E, "B", 0x08);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel1]_enable", 0x0E, "B", 0x04);

  MessageShort.addControl("[Channel2]", "!pfl", 0x0A, "B", 0x10);
  MessageShort.addControl("[EffectRack1_EffectUnit1]","group_[Channel2]_enable", 0x0E, "B", 0x02);
  MessageShort.addControl("[EffectRack1_EffectUnit2]","group_[Channel2]_enable", 0x0E, "B", 0x01);
  
  MessageShort.addControl("[Master]", "maximize_library", 0x0F, "B", 0x04);
  
  MessageShort.addControl("[Microphone]", "talkover", 0x0A, "B", 0x08);

  MessageShort.setCallback("[Channel1]", "!top_encoder_press", this.topEncoderPress);
  MessageShort.setCallback("[Channel2]", "!top_encoder_press", this.topEncoderPress);
  
  MessageShort.setCallback("[Channel1]", "!pfl", this.pflButton);
  MessageShort.setCallback("[Channel2]", "!pfl", this.pflButton);  
  
  MessageShort.setCallback("[Channel1]", "!shift", this.shift);
  MessageShort.setCallback("[Channel2]", "!shift", this.shift);
  MessageShort.setCallback("[Channel1]", "!cue_default", this.cue);
  MessageShort.setCallback("[Channel2]", "!cue_default", this.cue);
  MessageShort.setCallback("[Channel1]", "!play", this.play);
  MessageShort.setCallback("[Channel2]", "!play", this.play);
  MessageShort.setCallback("[Channel1]", "!pad1", this.pad);
  MessageShort.setCallback("[Channel1]", "!pad2", this.pad);
  MessageShort.setCallback("[Channel1]", "!pad3", this.pad);
  MessageShort.setCallback("[Channel1]", "!pad4", this.pad);
  MessageShort.setCallback("[Channel1]", "!remix_button", this.samplerModeButton);
  MessageShort.setCallback("[Channel1]", "!flux_button", this.introOutroModeButton);
  MessageShort.setCallback("[Channel2]", "!pad1", this.pad);
  MessageShort.setCallback("[Channel2]", "!pad2", this.pad);
  MessageShort.setCallback("[Channel2]", "!pad3", this.pad);
  MessageShort.setCallback("[Channel2]", "!pad4", this.pad);
  MessageShort.setCallback("[Channel2]", "!remix_button", this.samplerModeButton);
  MessageShort.setCallback("[Channel2]", "!flux_button", this.introOutroModeButton);
  MessageShort.setCallback("[Channel1]", "!loop_in", this.loopIn);
  MessageShort.setCallback("[Channel1]", "!loop_out", this.loopOut);
  MessageShort.setCallback("[Channel2]", "!loop_in", this.loopIn);
  MessageShort.setCallback("[Channel2]", "!loop_out", this.loopOut);
  MessageShort.setCallback("[Channel1]", "!load_track", this.loadTrack);
  MessageShort.setCallback("[Channel2]", "!load_track", this.loadTrack);
  MessageShort.setCallback("[Channel1]", "!sync_enabled", this.syncEnabled);
  MessageShort.setCallback("[Channel2]", "!sync_enabled", this.syncEnabled);
  MessageShort.setCallback("[Channel1]", "!left_encoder_press", this.leftEncoderPress);
  MessageShort.setCallback("[Channel2]", "!left_encoder_press", this.leftEncoderPress);
  MessageShort.setCallback("[Channel1]", "!right_encoder_press", this.rightEncoderPress);
  MessageShort.setCallback("[Channel2]", "!right_encoder_press", this.rightEncoderPress);
  MessageShort.setCallback("[Channel1]", "!jog_touch", this.jogTouch);
  MessageShort.setCallback("[Channel2]", "!jog_touch", this.jogTouch);
  MessageShort.setCallback("[Channel1]", "!jog_wheel", this.jogMove);
  MessageShort.setCallback("[Channel2]", "!jog_wheel", this.jogMove);
  
  MessageShort.setCallback("[EffectRack1_EffectUnit1]", "!effect_focus_button", this.effectFocusButton);
  MessageShort.setCallback("[EffectRack1_EffectUnit1", "!effectbutton1", this.effectButton);
  MessageShort.setCallback("[EffectRack1_EffectUnit1", "!effectbutton2", this.effectButton);
  MessageShort.setCallback("[EffectRack1_EffectUnit1", "!effectbutton3", this.effectButton);
 
  MessageShort.setCallback("[EffectRack1_EffectUnit2]", "!effect_focus_button", this.effectFocusButton);
  MessageShort.setCallback("[EffectRack1_EffectUnit2", "!effectbutton1", this.effectButton);
  MessageShort.setCallback("[EffectRack1_EffectUnit2", "!effectbutton2", this.effectButton);
  MessageShort.setCallback("[EffectRack1_EffectUnit2", "!effectbutton3", this.effectButton);
  
  MessageShort.setCallback("[Master]", "maximize_library", this.toggleButton);
  
  MessageShort.setCallback("[Microphone]", "talkover", this.toggleButton);
  
  engine.makeConnection("[EffectRack1_EffectUnit1]", "show_parameters", TraktorS2MK2.onShowParametersChange);
  engine.makeConnection("[EffectRack1_EffectUnit2]", "show_parameters", TraktorS2MK2.onShowParametersChange);
  
  this.controller.registerInputPacket(MessageShort);

  // Most items in the long message are controls that go from 0-4096.
  // There are also some 4 bit encoders.
  MessageLong.addControl("[Channel1]", "rate", 0x07, "H");
  MessageLong.addControl("[Channel2]", "rate", 0x09, "H");
  engine.softTakeover("[Channel1]", "rate", true);
  engine.softTakeover("[Channel2]", "rate", true);
  MessageLong.addControl("[Channel1]", "!left_encoder", 0x01, "B", 0x0F, undefined, true);
  MessageLong.addControl("[Channel2]", "!left_encoder", 0x02, "B", 0xF0, undefined, true);
  MessageLong.setCallback("[Channel1]", "!left_encoder", this.leftEncoder);
  MessageLong.setCallback("[Channel2]", "!left_encoder", this.leftEncoder);
  MessageLong.addControl("[Channel1]", "!right_encoder", 0x01, "B", 0xF0, undefined, true);
  MessageLong.addControl("[Channel2]", "!right_encoder", 0x03, "B", 0x0F, undefined, true);
  MessageLong.setCallback("[Channel1]", "!right_encoder", this.rightEncoder);
  MessageLong.setCallback("[Channel2]", "!right_encoder", this.rightEncoder);

  MessageLong.addControl("[EffectRack1_EffectUnit1]", "mix", 0x17, "H");
  MessageLong.addControl("[EffectRack1_EffectUnit1", "!effectknob1", 0x19, "H");
  MessageLong.setCallback("[EffectRack1_EffectUnit1", "!effectknob1", this.effectKnob);
  MessageLong.addControl("[EffectRack1_EffectUnit1", "!effectknob2", 0x1B, "H");
  MessageLong.setCallback("[EffectRack1_EffectUnit1", "!effectknob2", this.effectKnob);
  MessageLong.addControl("[EffectRack1_EffectUnit1", "!effectknob3", 0x1D, "H");
  MessageLong.setCallback("[EffectRack1_EffectUnit1", "!effectknob3", this.effectKnob);

  MessageLong.addControl("[EffectRack1_EffectUnit2]", "mix", 0x1F, "H");
  MessageLong.addControl("[EffectRack1_EffectUnit2", "!effectknob1", 0x21, "H");
  MessageLong.setCallback("[EffectRack1_EffectUnit2", "!effectknob1", this.effectKnob);
  MessageLong.addControl("[EffectRack1_EffectUnit2", "!effectknob2", 0x23, "H");
  MessageLong.setCallback("[EffectRack1_EffectUnit2", "!effectknob2", this.effectKnob);
  MessageLong.addControl("[EffectRack1_EffectUnit2", "!effectknob3", 0x25, "H");
  MessageLong.setCallback("[EffectRack1_EffectUnit2", "!effectknob3", this.effectKnob);
  
  for (var i = 1; i < 3; i++) {
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect" + i + "]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit2_Effect" + i + "]", "meta", true);
    for (var j = 1; j < 3; j++) {
      engine.softTakeover("[EffectRack1_EffectUnit1_Effect" + i + "]", "parameter" + j, true);
      engine.softTakeover("[EffectRack1_EffectUnit2_Effect" + i + "]", "parameter" + j, true);
    }
  }

  MessageLong.addControl("[Channel1]", "volume", 0x13, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x27, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x29, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x2B, "H");
  MessageLong.addControl("[Channel1]", "pregain", 0x03, "B", 0xF0, undefined, true);
  MessageLong.setCallback("[Channel1]", "pregain", this.topEncoder);

  MessageLong.addControl("[Channel2]", "volume", 0x15, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x2D, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x2F, "H");
  MessageLong.addControl("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x31, "H");
  MessageLong.addControl("[Channel2]", "pregain", 0x04, "B", 0x0F, undefined, true);
  MessageLong.setCallback("[Channel2]", "pregain", this.topEncoder);

  // The master gain knob controls the internal sound card volume, so if this was mapped
  // the gain would be double-applied.
  //MessageLong.addControl("[Master]", "volume", 0x11, "H");
  MessageLong.addControl("[Master]", "crossfader", 0x05, "H");
  MessageLong.addControl("[Master]", "headMix", 0x0B, "H");
  MessageLong.addControl("[Master]", "!samplerGain", 0xD, "H");
  MessageLong.setCallback("[Master]", "!samplerGain", this.samplerGainKnob);
  MessageLong.addControl("[Playlist]", "!browse", 0x02, "B", 0x0F, undefined, true);
  MessageLong.setCallback("[Playlist]", "!browse", this.callbackBrowse);

  this.controller.setScaler("volume", this.scalerVolume);
  this.controller.setScaler("headMix", this.scalerSlider);
  this.controller.setScaler("meta", this.scalerParameter);
  this.controller.setScaler("parameter1", this.scalerParameter);
  this.controller.setScaler("parameter2", this.scalerParameter);
  this.controller.setScaler("parameter3", this.scalerParameter);
  this.controller.setScaler("super1", this.scalerParameter);
  this.controller.setScaler("crossfader", this.scalerSlider);
  this.controller.setScaler("rate", this.scalerSlider);
  this.controller.setScaler("mix", this.scalerParameter);
  this.controller.registerInputPacket(MessageLong);
}

TraktorS2MK2.registerOutputPackets = function() {
  var OutputTop = new HIDPacket("outputTop", 0x80);
  var OutputBottom = new HIDPacket("outputBottom", 0x81);
  
  OutputTop.addOutput("[Channel1]", "track_loaded", 0x19, "B");
  OutputTop.addOutput("[Channel2]", "track_loaded", 0x1A, "B");

  var VuOffsets = {"[Channel1]" : 0x01,
                   "[Channel2]" : 0x06};
  for (ch in VuOffsets) {
    for (i = 0; i < 0x04; i++) {
      OutputTop.addOutput(ch, "!" + "VuMeter" + i, VuOffsets[ch] + i, "B");
    }
  }

  OutputTop.addOutput("[Channel1]", "PeakIndicator", 0x05, "B");
  OutputTop.addOutput("[Channel2]", "PeakIndicator", 0x0A, "B");
  
  OutputTop.addOutput("[Channel1]", "!flux_button", 0x20, "B");
  OutputTop.addOutput("[Channel1]", "loop_in", 0x21, "B");
  OutputTop.addOutput("[Channel1]", "loop_out", 0x22, "B");

  OutputTop.addOutput("[Channel2]", "!flux_button", 0x25, "B");  
  OutputTop.addOutput("[Channel2]", "loop_in", 0x23, "B");
  OutputTop.addOutput("[Channel2]", "loop_out", 0x24, "B");
  
  OutputTop.addOutput("[Channel1]", "pfl", 0x1B, "B");
  OutputTop.addOutput("[Master]", "!usblight", 0x1D, "B");
  OutputTop.addOutput("[Channel2]", "pfl", 0x1F, "B");
  
  OutputTop.addOutput("[EffectRack1_EffectUnit1]", "!effect_focus_button", 0xB, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit1]", "!effectbutton1", 0xC, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit1]", "!effectbutton2", 0xD, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit1]", "!effectbutton3", 0xE, "B");

  OutputTop.addOutput("[EffectRack1_EffectUnit2]", "!effect_focus_button", 0x13, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit2]", "!effectbutton1", 0x14, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit2]", "!effectbutton2", 0x15, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit2]", "!effectbutton3", 0x16, "B");
  
  OutputTop.addOutput("[Channel1]", "!remix_button", 0x17, "B");
  OutputTop.addOutput("[Channel2]", "!remix_button", 0x18, "B");

  OutputTop.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0x0F, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0x10, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0x11, "B");
  OutputTop.addOutput("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0x12, "B");
  
  this.controller.registerOutputPacket(OutputTop);
  
  OutputBottom.addOutput("[Channel1]", "!shift", 0x19, "B");
  OutputBottom.addOutput("[Channel1]", "sync_enabled", 0x1A, "B");
  OutputBottom.addOutput("[Channel1]", "cue_indicator", 0x1B, "B");
  OutputBottom.addOutput("[Channel1]", "play_indicator", 0x1C, "B");
  
  OutputBottom.addOutput("[Channel1]", "!pad_1_R", 0x01, "B");
  OutputBottom.addOutput("[Channel1]", "!pad_1_G", 0x02, "B");
  OutputBottom.addOutput("[Channel1]", "!pad_1_B", 0x03, "B");
  
  OutputBottom.addOutput("[Channel1]", "!pad_2_R", 0x04, "B");
  OutputBottom.addOutput("[Channel1]", "!pad_2_G", 0x05, "B");
  OutputBottom.addOutput("[Channel1]", "!pad_2_B", 0x06, "B");
  
  OutputBottom.addOutput("[Channel1]", "!pad_3_R", 0x07, "B");
  OutputBottom.addOutput("[Channel1]", "!pad_3_G", 0x08, "B");
  OutputBottom.addOutput("[Channel1]", "!pad_3_B", 0x09, "B");
  
  OutputBottom.addOutput("[Channel1]", "!pad_4_R", 0x0A, "B");
  OutputBottom.addOutput("[Channel1]", "!pad_4_G", 0x0B, "B");
  OutputBottom.addOutput("[Channel1]", "!pad_4_B", 0x0C, "B");
  
  OutputBottom.addOutput("[Channel2]", "!shift", 0x1D, "B");
  OutputBottom.addOutput("[Channel2]", "sync_enabled", 0x1E, "B");
  OutputBottom.addOutput("[Channel2]", "cue_indicator", 0x1F, "B");
  OutputBottom.addOutput("[Channel2]", "play_indicator", 0x20, "B");
  
  OutputBottom.addOutput("[Channel2]", "!pad_1_R", 0x0D, "B");
  OutputBottom.addOutput("[Channel2]", "!pad_1_G", 0x0E, "B");
  OutputBottom.addOutput("[Channel2]", "!pad_1_B", 0x0F, "B");
  
  OutputBottom.addOutput("[Channel2]", "!pad_2_R", 0x10, "B");
  OutputBottom.addOutput("[Channel2]", "!pad_2_G", 0x11, "B");
  OutputBottom.addOutput("[Channel2]", "!pad_2_B", 0x12, "B");
  
  OutputBottom.addOutput("[Channel2]", "!pad_3_R", 0x13, "B");
  OutputBottom.addOutput("[Channel2]", "!pad_3_G", 0x14, "B");
  OutputBottom.addOutput("[Channel2]", "!pad_3_B", 0x15, "B");
  
  OutputBottom.addOutput("[Channel2]", "!pad_4_R", 0x16, "B");
  OutputBottom.addOutput("[Channel2]", "!pad_4_G", 0x17, "B");
  OutputBottom.addOutput("[Channel2]", "!pad_4_B", 0x18, "B");

  this.controller.registerOutputPacket(OutputBottom);

  // Link up control objects to their outputs
  TraktorS2MK2.linkDeckOutputs("sync_enabled", TraktorS2MK2.outputCallback);
  TraktorS2MK2.linkDeckOutputs("cue_indicator", TraktorS2MK2.outputCallback);
  TraktorS2MK2.linkDeckOutputs("play_indicator", TraktorS2MK2.outputCallback);
  
  TraktorS2MK2.setPadMode("[Channel1]", TraktorS2MK2.pad_modes.hotcue);
  TraktorS2MK2.setPadMode("[Channel2]", TraktorS2MK2.pad_modes.hotcue);
  
  TraktorS2MK2.linkDeckOutputs("loop_in", TraktorS2MK2.outputCallbackLoop);
  TraktorS2MK2.linkDeckOutputs("loop_out", TraktorS2MK2.outputCallbackLoop);
  TraktorS2MK2.linkDeckOutputs("keylock", TraktorS2MK2.outputCallbackDark);
  TraktorS2MK2.linkDeckOutputs("LoadSelectedTrack", TraktorS2MK2.outputCallback);
  TraktorS2MK2.linkDeckOutputs("slip_enabled", TraktorS2MK2.outputCallback);
  TraktorS2MK2.linkChannelOutput("[Channel1]", "pfl", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[Channel2]", "pfl", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[Channel1]", "track_loaded", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[Channel2]", "track_loaded", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[Channel1]", "PeakIndicator", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[Channel2]", "PeakIndicator", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", TraktorS2MK2.outputChannelCallback);
  TraktorS2MK2.linkChannelOutput("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", TraktorS2MK2.outputChannelCallback);

  engine.makeConnection("[EffectRack1_EffectUnit1]", "focused_effect", TraktorS2MK2.onFocusedEffectChange);
  engine.makeConnection("[EffectRack1_EffectUnit2]", "focused_effect", TraktorS2MK2.onFocusedEffectChange);  
  TraktorS2MK2.connectEffectButtonLEDs("[EffectRack1_EffectUnit1]");
  TraktorS2MK2.connectEffectButtonLEDs("[EffectRack1_EffectUnit2]");
  
  engine.makeConnection("[Channel1]", "VuMeter", TraktorS2MK2.onVuMeterChanged);
  engine.makeConnection("[Channel2]", "VuMeter", TraktorS2MK2.onVuMeterChanged);

  engine.makeConnection("[Channel1]", "loop_enabled", TraktorS2MK2.onLoopEnabledChanged);
  engine.makeConnection("[Channel2]", "loop_enabled", TraktorS2MK2.onLoopEnabledChanged);
}

TraktorS2MK2.linkDeckOutputs = function(key, callback) {
  // Linking outputs is a little tricky because the library doesn't quite do what I want.  But this
  // method works.
  TraktorS2MK2.controller.linkOutput("[Channel1]", key, "[Channel1]", key, callback);
  engine.connectControl("[Channel3]", key, callback);
  TraktorS2MK2.controller.linkOutput("[Channel2]", key, "[Channel2]", key, callback);
  engine.connectControl("[Channel4]", key, callback);
}

TraktorS2MK2.linkChannelOutput = function(group, key, callback) {
  TraktorS2MK2.controller.linkOutput(group, key, group, key, callback);
}

TraktorS2MK2.lightGroup = function(packet, output_group_name, co_group_name) {
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

TraktorS2MK2.lightDeck = function(group) {
  // Freeze the lights while we do this update so we don't spam HID.
  this.freeze_lights = true;
  for (var packet_name in this.controller.OutputPackets) {
    packet = this.controller.OutputPackets[packet_name];
    TraktorS2MK2.lightGroup(packet, group, group);
    // Shift is a weird key because there's no CO that it is actually associated with.
    TraktorS2MK2.outputCallback(0, group, "!shift");
  }

  this.freeze_lights = false;
  // And now send them all.
  for (packet_name in this.controller.OutputPackets) {
    var packet_ob = this.controller.OutputPackets[packet_name];
    packet_ob.send();
  }
}

TraktorS2MK2.init = function(id) {
  TraktorS2MK2.registerInputPackets();
  TraktorS2MK2.registerOutputPackets();
//   var data = [0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f];
//   controller.send(data, data.length, 0x80);

  TraktorS2MK2.controller.setOutput("[Master]", "!usblight", 0x7F, true);
  TraktorS2MK2.lightDeck("[Channel1]");
  TraktorS2MK2.lightDeck("[Channel2]");
  TraktorS2MK2.lightDeck("[EffectRack1_EffectUnit1]");
  TraktorS2MK2.lightDeck("[EffectRack1_EffectUnit2]");
}

TraktorS2MK2.shutdown = function() {
  var data = [];
  for (var i = 0; i < 38; i++) {
    data[i] = 0;
  }
  // Leave USB plug indicator light on.
  data[0x1C] = 0x7f;
  controller.send(data, data.length, 0x80);
  
  for (i = 0; i < 33; i++) {
    data[i] = 0;
  }
  controller.send(data, data.length, 0x81);
}

TraktorS2MK2.incomingData = function(data, length) {
  TraktorS2MK2.controller.parsePacket(data, length);
}

// The short message handles buttons and jog wheels.
TraktorS2MK2.shortMessageCallback = function(packet, data) {
  for (name in data) {
    field = data[name];
    if (field.name === "!jog_wheel") {
      TraktorS2MK2.controller.processControl(field);
      continue;
    }

    TraktorS2MK2.controller.processButton(field);
  }
}

// There are no buttons handled by the long message, so this is a little simpler.  (Even though
// this is very similar to the other handler, it's easier to keep them separate to know what's
// a control and what's a button.
TraktorS2MK2.longMessageCallback = function(packet, data) {
  for (name in data) {
    field = data[name];
    TraktorS2MK2.controller.processControl(field);
  }
}

TraktorS2MK2.samplerGainKnob = function(field) {
  for (var i = 1; i <= 8; i++) {
    engine.setParameter("[Sampler" + i + "]", "pregain", field.value / 4096);
  }
}

TraktorS2MK2.toggleButton = function(field) {
  if (field.value > 0) {
    script.toggleControl(field.group, field.name);
  }
}

TraktorS2MK2.shift = function(field) {
  var group = field.id.split(".")[0];
  TraktorS2MK2.shift_pressed[group] = field.value;
  TraktorS2MK2.outputCallback(field.value, field.group, "!shift");
}

TraktorS2MK2.loadTrack = function(field) {
  var splitted = field.id.split(".");
  var group = splitted[0];
  if (TraktorS2MK2.shift_pressed[group]) {
    engine.setValue(field.group, "eject", field.value);
  } else {
    engine.setValue(field.group, "LoadSelectedTrack", field.value);
  }
}

TraktorS2MK2.syncEnabled = function(field) {
  var now = Date.now();

  var splitted = field.id.split(".");
  var group = splitted[0];
  // If shifted, just toggle.
  // TODO(later version): actually make this enable explicit master.
  if (TraktorS2MK2.shift_pressed[group]) {
    if (field.value === 0) {
      return;
    }
    var synced = engine.getValue(field.group, "sync_enabled");
    engine.setValue(field.group, "sync_enabled", !synced);
  } else {
    if (field.value === 1) {
      TraktorS2MK2.sync_enabled_time[field.group] = now;
      engine.setValue(field.group, "sync_enabled", 1);
    } else {
      var cur_enabled = engine.getValue(field.group, "sync_enabled");
      if (!cur_enabled) {
        // If disabled, and switching to disable... stay disabled.
        engine.setValue(field.group, "sync_enabled", 0);
        return;
      }
      // was enabled, and button has been let go.  maybe latch it.
      if (now - TraktorS2MK2.sync_enabled_time[field.group] > 300) {
        engine.setValue(field.group, "sync_enabled", 1);
        return;
      }
      engine.setValue(field.group, "sync_enabled", 0);
    }
  }
}

TraktorS2MK2.cue = function(field) {
  var splitted = field.id.split(".");
  var group = splitted[0];
  if (TraktorS2MK2.shift_pressed[group]) {
    if (ShiftCueButtonAction == "REWIND") {
      if (field.value === 0) {
        return;
      }
      engine.setValue(field.group, "start_stop", 1);
    } else if (ShiftCueButtonAction == "REVERSEROLL") {
      engine.setValue(field.group, "reverseroll", field.value);
    } else {
      print ("Traktor S2 Mk2 WARNING: Invalid ShiftCueButtonAction picked.  Must be either REWIND " +
           "or REVERSEROLL");
    }
  } else {
    engine.setValue(field.group, "cue_default", field.value);
  }
}

TraktorS2MK2.play = function(field) {
  if (field.value === 0) {
    return;
  }
  var splitted = field.id.split(".");
  var group = splitted[0];
  if (TraktorS2MK2.shift_pressed[group]) {
    var locked = engine.getValue(field.group, "keylock");
    engine.setValue(field.group, "keylock", !locked);
  } else {
    var playing = engine.getValue(field.group, "play");
    var deckNumber = TraktorS2MK2.controller.resolveDeck(group);
    // Failsafe to disable scratching in case the finishJogTouch timer has not executed yet
    // after a backspin.
    if (engine.isScratching(deckNumber)) {
      engine.scratchDisable(deckNumber, false);
    }
    engine.setValue(field.group, "play", !playing);
  }
}

TraktorS2MK2.jogTouch = function(field) {
  if (TraktorS2MK2.wheelTouchInertiaTimer[field.group] != 0) {
    // The wheel was touched again, reset the timer.
    engine.stopTimer(TraktorS2MK2.wheelTouchInertiaTimer[field.group]);
    TraktorS2MK2.wheelTouchInertiaTimer[field.group] = 0;
  }
  if (field.value !== 0) {
    var deckNumber = TraktorS2MK2.controller.resolveDeck(group);
    engine.scratchEnable(deckNumber, 1024, 33.3333, 0.125, 0.125/8, true);
  } else {
    // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
    // Depending on how fast the platter was moving, lengthen the time we'll wait.
    var scratchRate = Math.abs(engine.getValue(field.group, "scratch2"));
    // inertiaTime was experimentally determined. It should be enough time to allow the user to
    // press play after a backspin without normal playback starting before they can press the
    // button, but not so long that there is an awkward delay before stopping scratching after
    // a backspin.
    var inertiaTime;
    if (TraktorS2MK2.shift_pressed[field.group]) {
      inertiaTime = Math.pow(1.7, scratchRate / 10) / 1.6;
    } else {
      inertiaTIme = Math.pow(1.7, scratchRate) / 1.6;
    }
    if (inertiaTime < 100) {
      // Just do it now.
      TraktorS2MK2.finishJogTouch(field.group);
    } else {
      TraktorS2MK2.wheelTouchInertiaTimer[field.group] = engine.beginTimer(
        inertiaTime, function () {
          TraktorS2MK2.finishJogTouch(field.group)
        }, true);
    }
  }
}

TraktorS2MK2.finishJogTouch = function(group) {
  TraktorS2MK2.wheelTouchInertiaTimer[group] = 0;
  var deckNumber = TraktorS2MK2.controller.resolveDeck(group);
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
      engine.scratchDisable(deckNumber, true);
    } else {
      // Check again soon.
      TraktorS2MK2.wheelTouchInertiaTimer[group] = engine.beginTimer(
        1, function() {
          TraktorS2MK2.finishJogTouch(group);
        }, true);
    }
  }
}

TraktorS2MK2.jogMove = function(field) {
  var deltas = TraktorS2MK2.wheelDeltas(field.group, field.value);
  var tick_delta = deltas[0];
  var time_delta = deltas[1];

  if (engine.getValue(field.group, "scratch2_enable")) {
    var deckNumber = TraktorS2MK2.controller.resolveDeck(group);
    if (TraktorS2MK2.shift_pressed[group]) {
      tick_delta *= 10;
    }
    engine.scratchTick(deckNumber, tick_delta);
  } else { 
    var velocity = TraktorS2MK2.scalerJog(tick_delta, time_delta);
    engine.setValue(field.group, "jog", velocity);
  }
};

TraktorS2MK2.wheelDeltas = function(group, value) {
  // When the wheel is touched, four bytes change, but only the first behaves predictably.
  // It looks like the wheel is 1024 ticks per revolution.
  var tickval = value & 0xFF;
  var timeval = value >>> 16;
  var prev_tick = 0;
  var prev_time = 0;

  if (group[8] === "1" || group[8] === "3") {
    prev_tick = TraktorS2MK2.last_tick_val[0];
    prev_time = TraktorS2MK2.last_tick_time[0];
    TraktorS2MK2.last_tick_val[0] = tickval;
    TraktorS2MK2.last_tick_time[0] = timeval;
  } else {
    prev_tick = TraktorS2MK2.last_tick_val[1];
    prev_time = TraktorS2MK2.last_tick_time[1];
    TraktorS2MK2.last_tick_val[1] = tickval;
    TraktorS2MK2.last_tick_time[1] = timeval;
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

TraktorS2MK2.scalerJog = function(tick_delta, time_delta) {
  if (engine.getValue(group, "play")) {
    return (tick_delta / time_delta) / 3;
  } else {
    return (tick_delta / time_delta) * 2.0;
  }
}

var intro_outro_keys = [
  "intro_start",
  "intro_end",
  "outro_start",
  "outro_end"
];

var intro_outro_colors = [
  {red: 0,   green: 255, blue: 0},
  {red: 0,   green: 255, blue: 0},
  {red: 255, green: 0,   blue: 0},
  {red: 255, green: 0,   blue: 0}
];

TraktorS2MK2.setPadMode = function(group, padMode) {
  TraktorS2MK2.pad_connections[group].forEach(function(connection) {
    connection.disconnect();
  });
  TraktorS2MK2.pad_connections[group] = [];
  
  if (padMode === TraktorS2MK2.pad_modes.hotcue) {
    for (var i = 1; i <= 4; i++) {
      TraktorS2MK2.pad_connections[group].push(
        engine.makeConnection(group, "hotcue_" + i + "_enabled", TraktorS2MK2.outputHotcueCallback));          
      TraktorS2MK2.pad_connections[group].push(
        engine.makeConnection(group, "hotcue_" + i + "_color", TraktorS2MK2.outputHotcueCallback));
    }
  } else if (padMode === TraktorS2MK2.pad_modes.intro_outro) {
    for (var i = 1; i <= 4; i++) {
      // This function to create callback functions is needed so the loop index variable
      // i does not get captured in a closure within the callback.
      var makeIntroOutroCallback = function(padNumber) {
        return function(value, group, control) {
          if (value > 0) {
            TraktorS2MK2.sendPadColor(group, padNumber, intro_outro_colors[padNumber-1]);
          } else {
            TraktorS2MK2.sendPadColor(group, padNumber, {red: 0, green: 0, blue: 0});
          }
        };
      };
      TraktorS2MK2.pad_connections[group].push(engine.makeConnection(
        group, intro_outro_keys[i-1] + "_enabled", makeIntroOutroCallback(i)));
    }
  } else if (padMode === TraktorS2MK2.pad_modes.sampler) {
    for (var i = 1; i <= 4; i++) {
      var makeSamplerCallback = function(deckGroup, padNumber) {
        var samplerNumber = deckGroup === "[Channel1]" ? padNumber : padNumber + 4;
        var samplerGroup = "[Sampler" + samplerNumber + "]";
        return function(value, group, control) {
          if (engine.getValue(samplerGroup, "track_loaded")) {
            if (engine.getValue(samplerGroup, "play") === 1) {
              if (engine.getValue(samplerGroup, "repeat") === 1) {
                TraktorS2MK2.sendPadColor(deckGroup, padNumber, {red: 255, green: 255, blue: 0});
              } else {
                TraktorS2MK2.sendPadColor(deckGroup, padNumber, {red: 0xAF, green: 0x00, blue: 0xCC});
              }
            } else {
              TraktorS2MK2.sendPadColor(deckGroup, padNumber, {red: 255, green: 255, blue: 255});
            }
          } else {
            TraktorS2MK2.sendPadColor(deckGroup, padNumber, {red: 0, green: 0, blue: 0});
          }
        };
      }
      
      var sNumber = group === "[Channel1]" ? i : i + 4;
      var sGroup = "[Sampler" + sNumber + "]";
      TraktorS2MK2.pad_connections[group].push(engine.makeConnection(
        sGroup, "track_loaded", makeSamplerCallback(group, i)));
      TraktorS2MK2.pad_connections[group].push(engine.makeConnection(
        sGroup, "play", makeSamplerCallback(group, i)));
      TraktorS2MK2.pad_connections[group].push(engine.makeConnection(
        sGroup, "repeat", makeSamplerCallback(group, i)));
    }
  }
  
  TraktorS2MK2.pad_connections[group].forEach(function(connection) {
    connection.trigger();
  });
  
  TraktorS2MK2.current_pad_mode[group] = padMode;
}

TraktorS2MK2.pad = function(field) {
  var group = field.id.split(".")[0];
  var buttonNumber = parseInt(field.name[field.name.length - 1]);
  var padMode = TraktorS2MK2.current_pad_mode[group];
  
  if (padMode === TraktorS2MK2.pad_modes.hotcue) {
    if (TraktorS2MK2.shift_pressed[group]) {
      engine.setValue(field.group, "hotcue_" + buttonNumber + "_clear", field.value);
    } else {
      engine.setValue(field.group, "hotcue_" + buttonNumber + "_activate", field.value);
    }
  } else if (padMode === TraktorS2MK2.pad_modes.intro_outro) {
    if (TraktorS2MK2.shift_pressed[group]) {
      engine.setValue(field.group, intro_outro_keys[buttonNumber-1] + "_clear", field.value);
    } else {
      engine.setValue(field.group, intro_outro_keys[buttonNumber-1] + "_activate", field.value);
    }
  } else if (padMode === TraktorS2MK2.pad_modes.sampler) {
    if (field.value === 0) {
      return;
    }
    var samplerNumber = group === "[Channel1]" ? buttonNumber : buttonNumber + 4;
    var samplerGroup = "[Sampler" + samplerNumber + "]";
    if (TraktorS2MK2.shift_pressed[group]) {
      if (engine.getValue(samplerGroup, "play") === 1) {
        engine.setValue(samplerGroup, "play", 0);
      } else {
        script.triggerControl(samplerGroup, "eject");
      }
    } else {
      if (engine.getValue(samplerGroup, "track_loaded") === 0) {
        script.triggerControl(samplerGroup, "LoadSelectedTrack");
      } else {
        script.triggerControl(samplerGroup, "cue_gotoandplay");
      }
    }
  }
}

TraktorS2MK2.samplerModeButton = function(field) {
  if (field.value === 0) {
    return;
  }
  var padMode = TraktorS2MK2.current_pad_mode[field.group];
  if (padMode !== TraktorS2MK2.pad_modes.sampler) {
    TraktorS2MK2.setPadMode(field.group, TraktorS2MK2.pad_modes.sampler);
    TraktorS2MK2.controller.setOutput(field.group, "!remix_button", 0x7f, false);
    TraktorS2MK2.controller.setOutput(field.group, "!flux_button", 0x00, !TraktorS2MK2.freeze_lights);
  } else {
    TraktorS2MK2.setPadMode(field.group, TraktorS2MK2.pad_modes.hotcue);
    TraktorS2MK2.controller.setOutput(field.group, "!remix_button", 0x00, !TraktorS2MK2.freeze_lights);
  }
}

TraktorS2MK2.introOutroModeButton = function(field) {
  if (field.value === 0) {
    return;
  }
  var padMode = TraktorS2MK2.current_pad_mode[field.group];
  if (padMode !== TraktorS2MK2.pad_modes.intro_outro) {
    TraktorS2MK2.setPadMode(field.group, TraktorS2MK2.pad_modes.intro_outro);
    TraktorS2MK2.controller.setOutput(field.group, "!flux_button", 0x7f, false);
    TraktorS2MK2.controller.setOutput(field.group, "!remix_button", 0x00, !TraktorS2MK2.freeze_lights);
  } else {
    TraktorS2MK2.setPadMode(field.group, TraktorS2MK2.pad_modes.hotcue);
    TraktorS2MK2.controller.setOutput(field.group, "!flux_button", 0x00, !TraktorS2MK2.freeze_lights);
  }
}

TraktorS2MK2.loopIn = function(field) {
  var group = field.id.split(".")[0];
  engine.setValue(group, "loop_in", field.value);
}

TraktorS2MK2.loopOut = function(field) {
  var group = field.id.split(".")[0];
  engine.setValue(group, "loop_out", field.value);
}

TraktorS2MK2.samplerActivateButton = function(field) {
  if (field.value > 0) {
    TraktorS2Mk2.controller.samplers_active[field.group] = !TraktorS2Mk2.controller.samplers_active[field.group];
  }
}

TraktorS2MK2.connectEffectButtonLEDs = function(effectUnitGroup) {
  TraktorS2MK2.effectButtonLEDconnections[effectUnitGroup].forEach(function(connection) {
    connection.disconnect();
  });
  
  var focusedEffect = engine.getValue(effectUnitGroup, "focused_effect");
  var makeButtonLEDcallback = function(effectNumber) {
    return function(value, group, control) {
      TraktorS2MK2.controller.setOutput(effectUnitGroup, "!effectbutton" + effectNumber,
        value === 1 ? 0x7f : 0, !TraktorS2MK2.freeze_lights);
    };
  };
  
  // FIXME: Why do the LEDs flicker?
  TraktorS2MK2.freeze_lights = true;
  for (var i = 0; i < 3; i++) {
    var effectGroup;
    var key;
    if (focusedEffect === 0) {
      effectGroup = effectUnitGroup.slice(0, -1) + "_Effect" + (i+1) + "]";
      key = "enabled";
    } else {
      effectGroup = effectUnitGroup.slice(0, -1) + "_Effect" + focusedEffect + "]";
      key = "button_parameter" + (i+1);
    }
    TraktorS2MK2.effectButtonLEDconnections[effectUnitGroup][i] = engine.makeConnection(
      effectGroup, key, makeButtonLEDcallback(i+1));
    TraktorS2MK2.effectButtonLEDconnections[effectUnitGroup][i].trigger();
  }
  TraktorS2MK2.freeze_lights = false;
  TraktorS2MK2.effectButtonLEDconnections[effectUnitGroup][2].trigger();
}

TraktorS2MK2.onShowParametersChange = function(value, group, control) {
  if (value === 0) {
    if (engine.getValue(group, "show_focus") > 0) {
      engine.setValue(group, "show_focus", 0);
      TraktorS2MK2.previouslyFocusedEffect[group] = engine.getValue(group, "focused_effect");
      engine.setValue(group, "focused_effect", 0);
    }
  } else {
    engine.setValue(group, "show_focus", 1);
    if (TraktorS2MK2.previouslyFocusedEffect[group] !== null) {
      engine.setValue(group, "focused_effect", TraktorS2MK2.previouslyFocusedEffect[group]);
    }
  }
  TraktorS2MK2.connectEffectButtonLEDs(group);
}

TraktorS2MK2.onFocusedEffectChange = function(value, group, control) {
  TraktorS2MK2.controller.setOutput(group, "!effect_focus_button", value > 0 ? 0x7f : 0, !TraktorS2MK2.freeze_lights);
  if (value === 0) {
    for (var i = 1; i < 3; i++) {
      // The previously focused effect is not available here, so iterate over all effects' parameter knobs.
      for (var j = 1; j < 3; j++) {
        engine.softTakeoverIgnoreNextValue(group.slice(0, -1) + "_Effect" + i + "]", "parameter" + j);
      }
    }
  } else {
    for (var i = 1; i < 3; i++) {
      engine.softTakeoverIgnoreNextValue(group.slice(0, -1) + "_Effect" + i + "]", "meta");
    }
  }
}

TraktorS2MK2.effectFocusButton = function(field) {
  var showParameters = engine.getValue(field.group, "show_parameters");
  if (field.value > 0) {
    var effectUnitNumber = field.group.slice(-2, -1);
    if (TraktorS2MK2.shift_pressed["[Channel" + effectUnitNumber + "]"]) {
      engine.setValue(field.group, "load_preset", 1);
      return;
    }
    TraktorS2MK2.effectFocusLongPressTimer[field.group] = engine.beginTimer(TraktorS2MK2.longPressTimeout, function() {
      TraktorS2MK2.effectFocusChooseModeActive[field.group] = true;
      TraktorS2MK2.effectButtonLEDconnections[field.group].forEach(function(connection) {
        connection.disconnect();
      });
      var makeButtonLEDcallback = function(buttonNumber) {
          return function(value, group, control) {
            TraktorS2MK2.controller.setOutput(group, "!effectbutton" + buttonNumber,
              value === buttonNumber ? 0x7f : 0, !TraktorS2MK2.freeze_lights);
          };
      };
      TraktorS2MK2.freeze_lights = true;
      for (var i = 0; i < 3; i++) {
        TraktorS2MK2.effectButtonLEDconnections[i] = engine.makeConnection(
          field.group, "focused_effect", makeButtonLEDcallback(i+1));
        TraktorS2MK2.effectButtonLEDconnections[i].trigger();
      }
      TraktorS2MK2.freeze_lights = false;
      TraktorS2MK2.effectButtonLEDconnections[2].trigger();
    });
    if (!showParameters) {
      engine.setValue(field.group, "show_parameters", 1);
      TraktorS2MK2.effectFocusButtonPressedWhenParametersHidden[field.group] = true;
    }
  } else {
    if (TraktorS2MK2.effectFocusLongPressTimer[field.group] !== 0) {
      engine.stopTimer(TraktorS2MK2.effectFocusLongPressTimer[field.group]);
      TraktorS2MK2.effectFocusLongPressTimer[field.group] = 0;
    }
    
    if (TraktorS2MK2.effectFocusChooseModeActive[field.group]) {
      TraktorS2MK2.effectFocusChooseModeActive[field.group] = false;
      TraktorS2MK2.connectEffectButtonLEDs(field.group);
    } else if (showParameters && !TraktorS2MK2.effectFocusButtonPressedWhenParametersHidden[field.group]) {
      engine.setValue(this.group, "show_parameters", 0);
    }
    
    TraktorS2MK2.effectFocusButtonPressedWhenParametersHidden[field.group] = false;
  }
}

TraktorS2MK2.effectKnob = function(field) {
  var knobNumber = parseInt(field.id.slice(-1));
  var effectUnitGroup = field.group + "]";
  var focusedEffect = engine.getValue(effectUnitGroup, "focused_effect");
  if (focusedEffect > 0) {
    engine.setParameter(field.group + "_Effect" + focusedEffect + "]", "parameter" + knobNumber, field.value / 4096);
  } else {
    engine.setParameter(field.group + "_Effect" + knobNumber + "]", "meta", field.value / 4096);
  }
}

TraktorS2MK2.effectButton = function(field) {
  var buttonNumber = parseInt(field.id.slice(-1));
  var effectUnitGroup = field.group + "]";
  var effectUnitNumber = field.group.slice(-1);
  var focusedEffect = engine.getValue(effectUnitGroup, "focused_effect");
    
  var toggle = function () {
    var group;
    var key;
    if (focusedEffect === 0) {
      group = field.group + "_Effect" + buttonNumber + "]";
      key = "enabled";
    } else {
      group = field.group + "_Effect" + focusedEffect + "]"
      key = "button_parameter" + buttonNumber;
    }
    script.toggleControl(group, key);
  };
  
  if (field.value > 0) {
    if (TraktorS2MK2.shift_pressed["[Channel" + effectUnitNumber + "]"]) {
      engine.setValue(effectUnitGroup, "load_preset", buttonNumber+1);
    } else {
      if (TraktorS2MK2.effectFocusChooseModeActive[effectUnitGroup]) {
        if (focusedEffect === buttonNumber) {
          engine.setValue(effectUnitGroup, "focused_effect", 0);
        } else {
          engine.setValue(effectUnitGroup, "focused_effect", buttonNumber);
        }
        TraktorS2MK2.effectFocusChooseModeActive[effectUnitGroup] = false;
      } else {
        toggle();
        TraktorS2MK2.effectButtonLongPressTimer[effectUnitGroup][buttonNumber] =
          engine.beginTimer(TraktorS2MK2.longPressTimeout,
            function() {
              TraktorS2MK2.effectButtonIsLongPressed[effectUnitGroup][buttonNumber] = true;
              TraktorS2MK2.effectButtonLongPressTimer[effectUnitGroup][buttonNumber] = 0;
            },
            true
          );
      }
    }
  } else {
    engine.stopTimer(TraktorS2MK2.effectButtonLongPressTimer[effectUnitGroup][buttonNumber]);
    TraktorS2MK2.effectButtonLongPressTimer[effectUnitGroup][buttonNumber] = 0;
    if (TraktorS2MK2.effectButtonIsLongPressed[effectUnitGroup][buttonNumber]) {
      toggle();
    }
    TraktorS2MK2.effectButtonIsLongPressed[effectUnitGroup][buttonNumber] = false;
  }
}

TraktorS2MK2.topEncoder = function(field) {
  // TODO: common-hid-packet-parser looks like it should do deltas, but I can't get them to work.
  prev_pregain = TraktorS2MK2.previous_pregain[field.group];
  TraktorS2MK2.previous_pregain[field.group] = field.value;
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

  if (TraktorS2MK2.shift_pressed[field.group]) {
    var cur_pregain = engine.getValue(group, "pregain");
    engine.setValue(group, "pregain", cur_pregain + delta);
  } else {
    var quickeffect_group = "[QuickEffectRack1_" + field.group + "]";
    if (TraktorS2MK2.top_encoder_pressed[field.group]) {
      script.triggerControl(quickeffect_group, delta > 0 ? "next_chain" : "prev_chain");
    } else {
      var cur_quickeffect = engine.getValue(quickeffect_group, "super1");
      engine.setParameter(quickeffect_group, "super1", cur_quickeffect + delta / 1.5);
    }
  }
}

TraktorS2MK2.topEncoderPress = function(field) {
  if (field.value > 0) {
    TraktorS2MK2.top_encoder_pressed[field.group] = true;
    if (TraktorS2MK2.shift_pressed[field.group]) {
      script.triggerControl(field.group, "pregain_set_default");
    } else {
      script.triggerControl("[QuickEffectRack1_" + field.group + "]", "super1_set_default");
    }
  } else {
    TraktorS2MK2.top_encoder_pressed[field.group] = false;
  }
}

TraktorS2MK2.leftEncoder = function(field) {
  // TODO: common-hid-packet-parser looks like it should do deltas, but I can't get them to work.
  var splitted = field.id.split(".");
  var group = splitted[0]
  previous_leftencoder = TraktorS2MK2.previous_leftencoder[group];
  TraktorS2MK2.previous_leftencoder[group] = field.value;
  var delta = 0;
  if (previous_leftencoder === 15 && field.value === 0) {
    delta = 1;
  } else if (previous_leftencoder === 0 && field.value === 15) {
    delta = -1;
  } else if (field.value > previous_leftencoder) {
    delta = 1;
  } else {
    delta = -1;
  }

  if (TraktorS2MK2.shift_pressed[group]) {
    if (delta == 1) {
      script.triggerControl(group, "pitch_up_small");
    } else {
      script.triggerControl(group, "pitch_down_small");
    }
  } else {
    if (TraktorS2MK2.left_encoder_pressed[group]) {
      var beatjump_size = engine.getValue(group, "beatjump_size");
      if (delta === 1) {
        beatjump_size *= 2;
      } else {
        beatjump_size /= 2;
      }
      engine.setValue(group, "beatjump_size", beatjump_size);
    } else {
      if (delta === 1) {
        script.triggerControl(group, "beatjump_forward");
      } else {
        script.triggerControl(group, "beatjump_backward");
      }
    }
  }
}

TraktorS2MK2.leftEncoderPress = function(field) {
  var splitted = field.id.split(".");
  var group = splitted[0];
  TraktorS2MK2.left_encoder_pressed[group] = (field.value > 0) ? true : false;
  if (TraktorS2MK2.shift_pressed[group] && field.value > 0) {
      script.triggerControl(group, "pitch_adjust_set_default");
  }
}

TraktorS2MK2.rightEncoder = function(field) {
  var splitted = field.id.split(".");
  var group = splitted[0]
  previous_rightencoder = TraktorS2MK2.previous_leftencoder[group];
  TraktorS2MK2.previous_leftencoder[group] = field.value;
  var delta = 0;
  if (previous_rightencoder === 15 && field.value === 0) {
    delta = 1;
  } else if (previous_rightencoder === 0 && field.value === 15) {
    delta = -1;
  } else if (field.value > previous_rightencoder) {
    delta = 1;
  } else {
    delta = -1;
  }

  if (TraktorS2MK2.shift_pressed[group]) {
    if (delta == 1) {
      script.triggerControl(group, "beatjump_1_forward");
    } else {
      script.triggerControl(group, "beatjump_1_backward");
    }
  } else {
    if (delta == 1) {
      script.triggerControl(group, "loop_double");
    } else {
      script.triggerControl(group, "loop_halve");
    }
  }
}

TraktorS2MK2.rightEncoderPress = function(field) {
  if (field.value === 0) {
      return;
  }
  var splitted = field.id.split(".");
  var group = splitted[0];
  var loop_enabled = engine.getValue(group, "loop_enabled");
  // The actions triggered below change the state of loop_enabled,
  // so to simplify the logic, use script.triggerControl to only act
  // on press rather than resetting ControlObjects to 0 on release.
  if (TraktorS2MK2.shift_pressed[group]) {
    if (loop_enabled) {
      script.triggerControl(group, "reloop_andstop");
    } else {
      script.triggerControl(group, "reloop_toggle");
    }
  } else {
    if (loop_enabled) {
      script.triggerControl(group, "reloop_toggle");
    } else {
      script.triggerControl(group, "beatloop_activate");
    }
  }
}

TraktorS2MK2.callbackBrowse = function(field) {
  // TODO: common-hid-packet-parser looks like it should do deltas, but I can't get them to work.
  prev_browse = TraktorS2MK2.previous_browse;
  TraktorS2MK2.previous_browse = field.value;
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

TraktorS2MK2.scalerParameter = function(group, name, value) {
  return script.absoluteLin(value, 0, 1, 16, 4080);
}
// Tell the HIDController script to use setParameter instead of setValue.
TraktorS2MK2.scalerParameter.useSetParameter = true;

TraktorS2MK2.scalerVolume = function(group, name, value) {
  if (group === "[Master]") {
    return script.absoluteNonLin(value, 0, 1, 4, 16, 4080);
  } else {
    return script.absoluteNonLin(value, 0, 0.25, 1, 16, 4080);
  }
}

TraktorS2MK2.scalerSlider = function(group, name, value) {
  return script.absoluteLin(value, -1, 1, 16, 4080);
}

TraktorS2MK2.outputChannelCallback = function(value,group,key) {
  var led_value = 0x05;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS2MK2.controller.setOutput(group, key, led_value, !TraktorS2MK2.freeze_lights);
}

TraktorS2MK2.outputChannelCallbackDark = function(value,group,key) {
  var led_value = 0x00;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS2MK2.controller.setOutput(group, key, led_value, !TraktorS2MK2.freeze_lights);
}

TraktorS2MK2.outputCallback = function(value,group,key) {
  var led_value = 0x09;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS2MK2.controller.setOutput(group, key, led_value, !TraktorS2MK2.freeze_lights);
}

TraktorS2MK2.outputCallbackLoop = function(value,group,key) {
  var led_value = 0x09;
  if (engine.getValue(group, "loop_enabled")) {
    led_value = 0x7F;
  }
  TraktorS2MK2.controller.setOutput(group, key, led_value, !TraktorS2MK2.freeze_lights);
}

TraktorS2MK2.outputCallbackDark = function(value,group,key) {
  var led_value = 0x00;
  if (value) {
    led_value = 0x7F;
  }
  TraktorS2MK2.controller.setOutput(group, key, led_value, !TraktorS2MK2.freeze_lights);
}

TraktorS2MK2.pflButton = function(field) {
  if (field.value > 0) {
    if (TraktorS2MK2.shift_pressed[field.group]) {
      script.toggleControl(field.group, "quantize");
    } else {
      script.toggleControl(field.group, "pfl");
    }
  }
}

TraktorS2MK2.sendPadColor = function(group, padNumber, color) {
  var padKey = "!pad_" + padNumber + "_";
  TraktorS2MK2.controller.setOutput(group, padKey + "R", color.red, false);
  TraktorS2MK2.controller.setOutput(group, padKey + "G", color.green, false);
  TraktorS2MK2.controller.setOutput(group, padKey + "B", color.blue, !TraktorS2MK2.freeze_lights);
}

TraktorS2MK2.outputHotcueCallback = function(value, group, key) {
  var hotcueNumber = key.charAt(7);
  var color;
  if (engine.getValue(group, 'hotcue_' + hotcueNumber + '_enabled')) {
    color = colorCodeToObject(engine.getValue(group, 'hotcue_' + hotcueNumber + '_color'));
  } else {
    color = {red: 0, green: 0, blue: 0};
  }
  TraktorS2MK2.sendPadColor(group, hotcueNumber, color);
}

TraktorS2MK2.onVuMeterChanged = function(value, group, key) {
  // This handler is called a lot so it should be as fast as possible.

  // VU is drawn on 6 segments, the 7th indicates clip.
  var VuOffsets = {"[Channel1]" : 0x01,
                   "[Channel2]" : 0x06};

  // Figure out number of fully-illuminated segments.
  var scaledValue = value * 4.0;
  var fullIllumCount = Math.floor(scaledValue);

  // Figure out how much the partially-illuminated segment is illuminated.
  var partialIllum = (scaledValue - fullIllumCount) * 0x7F

  for (i = 0; i < 4; i++) {
    var key = "!" + "VuMeter" + i;
    if (i < fullIllumCount) {
      // Don't update lights until they're all done, so the last term is false.
      TraktorS2MK2.controller.setOutput(group, key, 0x7F, false);
    } else if (i == fullIllumCount) {
      TraktorS2MK2.controller.setOutput(group, key, partialIllum, false);
    } else {
      TraktorS2MK2.controller.setOutput(group, key, 0x00, false);
    }
  }
  TraktorS2MK2.controller.OutputPackets["outputTop"].send();
}

TraktorS2MK2.onLoopEnabledChanged = function(value, group, key) {
  TraktorS2MK2.outputCallbackLoop(value, group, "loop_in");
  TraktorS2MK2.outputCallbackLoop(value, group, "loop_out");
}

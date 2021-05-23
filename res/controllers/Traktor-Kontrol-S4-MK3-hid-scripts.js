// Copyright 2021 Google LLC.
// SPDX-Licence-Identifier: GPL-2.0-or-later
//
// WARNING: This is DRAFT code. It is incomplete and does not follow
// the expected coding practices. This is done deliberately for
// exploratory and educational purposes. Also, I don't really know
// JavaScript, any suggestions on making the code more idiomatic are
// welcome.
//
// This code allows Mixxx to communicate with the the Traktor S4 MK3
// controller over the USB HID interface.
//
// The main design idea is a separation between parsing bits, identifying
// S4 controls (such as buttons and knobs), mapping S4 controls to
// Mixxx controls, and handling the state of the controller (such as
// decks assignment).

TraktorS4MK3_alex = new function() {
    this.controller = new HIDController();
    this.partial_packet = Object();
    this.encoderValues = Object();
    this.decksMapping = {
        // The dynamic assignment of pseudo decks "left" and "right"
        // can be changed by the "deck select" buttons.
        left: "Channel1",
        right: "Channel2",
        // The deckN to ChannelM mapping is fixed.
        deckA: "Channel1",
        deckB: "Channel2",
        deckC: "Channel3",
        deckD: "Channel4",
    }

    // Constants
    this.encoderMax = 15;
    this.encoderMin = 0;
    this.potentiometerMax = 4095;
    this.maxWheelTick = 4294967296;
    this.maxWheelValue = 65536;
    this.wheelCycleLength = 3170;
    this.wheelSpeedMultiplier = 4.0;

    // When the weel is being operated, it changes its position few
    // times a second. If the position has changed after a long pause,
    // then this change should be ignored.
    this.maxWheelWait = 2 * 1000; // 2 seconds.

    // The state of wheels
    this.prevWheelValue = {"left": undefined, "right": undefined};
    this.currWheelValue = {"left": undefined, "right": undefined};

    // The time is expressed both in S4 ticks and as a UNIX timestamp,
    // because the tick counter can wraps around each ~minute.
    this.prevWheelTime = {"left": undefined, "right": undefined};
    this.prevWheelTick = {"left": undefined, "right": undefined};
    this.currWheelTick = undefined;
    this.currTime = new Date().getTime();
}

// Creates a lookup table for translating between S4 control names and
// Mixxx control names.
TraktorS4MK3_alex.registerS4ToMixxxMapping = function() {
    this.s4ToMixxx = {
        // Deck controls
        browse_encoder_rotate: { control: "MoveVertical", group: "Library" },
        browse_encoder_press: {
            control: "LoadSelectedTrack",
            group: "${Channel}",
        },
        tempo: {
            control: "rate",
            group: "${Channel}",
            inverse: true,
        },
        cue: {
            control: "cue_default",
            group: "${Channel}",
        },
        play_pause: {
            control: "play",
            group: "${Channel}",
            toggle: true,
        },
        
        // Mixer channel controls
        gain: { control: "pregain", group: "${Channel}" },
        eq_high_knob: {
            control: "parameter3",
            group: "EqualizerRack1_[${Channel}]_Effect1",
        },
        eq_mid_knob: {
            control: "parameter2",
            group: "EqualizerRack1_[${Channel}]_Effect1",
        },
        eq_low_knob: {
            control: "parameter1",
            group: "EqualizerRack1_[${Channel}]_Effect1",
        },
        headphone_cue: {
            control: "pfl",
            group: "${Channel}",
            toggle: true,
        },
        level: {
            control: "volume",
            group: "${Channel}",
        },
        
        // Mixer global controls
        
        // The `master_level` remains unmapped, because it physically
        // controls the output level from S4 audio interface. Therefore
        // mapping `master_level` to a Mixxx control would simply result
        // in a steeper control curve.
        
        // TODO: check if booth_level needs a mapping.
        
        headphone_mix: { control: "headMix", group: "Master" },
        headphone_vol: { control: "headGain", group: "Master" },
        crossfader: { control: "crossfader", group: "Master" },
    }
}

// Some S4 controls cannot be directly mapped to Mixxx controls and
// need to be specially handled by the code.
TraktorS4MK3_alex.registerSpecialInputHandlers = function() {
    var s4 = TraktorS4MK3_alex
    this.specialInputHandlers = {
        select_deck_A: function(field) {
            s4.assignDeck("left", "deckA", field);
        },
        select_deck_C: function(field) {
            s4.assignDeck("left", "deckC", field);
        },
        select_deck_B: function(field) {
            s4.assignDeck("right", "deckB", field);
        },
        select_deck_D: function(field) {
            s4.assignDeck("right", "deckD", field);
        },
    }
}

// This is the mapping from the bits on the wire to S4 controls. It
// does not map directly to Mixxx on purpose.
TraktorS4MK3_alex.registerInputPackets = function() {
    var m1 = new HIDPacket("message1", 0x01, this.messageCallback);
    var m2 = new HIDPacket("message2", 0x02, this.messageCallback);
    var m3 = new HIDPacket("message3", 0x03, this.wheelsCallback);

    // Message1 contains the state of buttons and encoders.
    this.addButton(m1, "left", "master_or_reset", 0x01, 0x01);
    this.addButton(m1, "left", "browse_encoder_press", 0x01, 0x02);
    this.addButton(m1, "left", "favourites", 0x01, 0x10);
    this.addButton(m1, "left", "preview_play", 0x01, 0x20);
    
    this.addButton(m1, "left", "browser_view", 0x02, 0x01);
    this.addButton(m1, "left", "prep_list", 0x02, 0x02);
    this.addButton(m1, "left", "rev", 0x02, 0x10);
    this.addButton(m1, "left", "flx", 0x02, 0x20);
    this.addButton(m1, "left", "fx1_on", 0x02, 0x40);
    this.addButton(m1, "left", "fx2_on", 0x02, 0x80);
    this.addButton(m1, "left", "fx3_on", 0x02, 0x08);
    this.addButton(m1, "left", "fx4_on", 0x02, 0x04);

    this.addButton(m1, "deckC", "ext", 0x03, 0x02);
    this.addButton(m1, "deckA", "fx_assign_prev", 0x03, 0x08);
    this.addButton(m1, "deckA", "fx_assign_next", 0x03, 0x10);
    this.addButton(m1, "deckB", "fx_assign_prev", 0x03, 0x20);
    this.addButton(m1, "deckB", "fx_assign_next", 0x03, 0x40);
    this.addButton(m1, "deckC", "fx_assign_prev", 0x03, 0x01);
    this.addButton(m1, "deckC", "fx_assign_next", 0x03, 0x04);
    this.addButton(m1, "deckD", "fx_assign_prev", 0x03, 0x80);
    
    this.addButton(m1, "left", "pad1", 0x04, 0x20);
    this.addButton(m1, "left", "pad2", 0x04, 0x10);
    this.addButton(m1, "left", "pad3", 0x04, 0x80);
    this.addButton(m1, "left", "pad4", 0x04, 0x40);
    this.addButton(m1, "left", "pad5", 0x04, 0x08);
    this.addButton(m1, "left", "pad6", 0x04, 0x04);
    this.addButton(m1, "left", "pad7", 0x04, 0x02);
    this.addButton(m1, "left", "pad8", 0x04, 0x01);
    
    this.addButton(m1, "left", "play_pause", 0x05, 0x01);
    this.addButton(m1, "left", "cue", 0x05, 0x02);
    this.addButton(m1, "left", "pad_hotcues", 0x05, 0x04);
    this.addButton(m1, "left", "pad_record", 0x05, 0x08);
    this.addButton(m1, "left", "pad_samples", 0x05, 0x10);
    this.addButton(m1, "left", "pad_mute", 0x05, 0x20);
    
    this.addButton(m1, "left", "pad_stems", 0x06, 0x01);
    this.addButton(m1, "left", "shift", 0x06, 0x02);
    this.addButton(m1, "left", "select_deck_A", 0x06, 0x04);
    this.addButton(m1, "left", "select_deck_C", 0x06, 0x08);
    this.addButton(m1, "left", "jog_mode", 0x06, 0x10);
    this.addButton(m1, "left", "tt_mode", 0x06, 0x20);
    this.addButton(m1, "left", "grid_mode", 0x06, 0x40);
    this.addButton(m1, "left", "sync_lock", 0x06, 0x80);

    this.addButton(m1, "left", "move_encoder_press", 0x07, 0x04);
    this.addButton(m1, "left", "move_encoder_touch", 0x07, 0x08);
    this.addButton(m1, "left", "loop_encoder_touch", 0x07, 0x10);
    this.addButton(m1, "left", "loop_encoder_press", 0x07, 0x20);

    this.addButton(m1, "deckA", "mixerfx_on", 0x08, 0x01);
    this.addButton(m1, "deckB", "mixerfx_on", 0x08, 0x20);
    this.addButton(m1, "deckC", "mixerfx_on", 0x08, 0x02);
    this.addButton(m1, "deckD", "mixerfx_on", 0x08, 0x10);
    this.addButton(m1, "deckA", "headphone_cue", 0x08, 0x08);
    this.addButton(m1, "deckB", "headphone_cue", 0x08, 0x40);
    this.addButton(m1, "deckC", "headphone_cue", 0x08, 0x04);
    this.addButton(m1, "deckD", "headphone_cue", 0x08, 0x80);

    this.addButton(m1, "mixer", "select_fx_1", 0x09, 0x20);
    this.addButton(m1, "mixer", "select_fx_2", 0x09, 0x02);
    this.addButton(m1, "mixer", "select_fx_3", 0x09, 0x40);
    this.addButton(m1, "mixer", "select_fx_4", 0x09, 0x01);
    this.addButton(m1, "mixer", "select_fx_filter", 0x09, 0x80);
    
    this.addButton(m1, "right", "browser_view", 0x0a, 0x01);
    this.addButton(m1, "right", "prep_list", 0x0a, 0x08);
    this.addButton(m1, "right", "favourites", 0x0a, 0x02);
    this.addButton(m1, "right", "preview_play", 0x0a, 0x04);
    this.addButton(m1, "right", "fx1_on", 0x0a, 0x10);
    this.addButton(m1, "right", "fx2_on", 0x0a, 0x20);
    this.addButton(m1, "right", "fx3_on", 0x0a, 0x40);
    this.addButton(m1, "right", "fx4_on", 0x0a, 0x80);

    this.addButton(m1, "right", "master_reset", 0x0b, 0x01);
    this.addButton(m1, "right", "browse_encoder_press", 0x0b, 0x02);
    this.addButton(m1, "right", "rev", 0x0b, 0x10);
    this.addButton(m1, "right", "flx", 0x0b, 0x20);

    this.addButton(m1, "deckA", "ext", 0x0c, 0x01);
    this.addButton(m1, "deckB", "ext", 0x0c, 0x02);
    this.addButton(m1, "deckD", "ext", 0x0c, 0x04);
    this.addButton(m1, "mixer", "quantise", 0x0c, 0x40);
    this.addButton(m1, "deckD", "fx_assign_next", 0x0c, 0x80);

    this.addButton(m1, "right", "pad_hotcues", 0x0d, 0x04);
    this.addButton(m1, "right", "pad_record", 0x0d, 0x08);
    this.addButton(m1, "right", "pad_samples", 0x0d, 0x10);
    this.addButton(m1, "right", "pad_mute", 0x0d, 0x20);
    this.addButton(m1, "right", "pad_stems", 0x0d, 0x02);
    this.addButton(m1, "right", "play_pause", 0x0d, 0x01);

    this.addButton(m1, "right", "pad1", 0x0e, 0x20);
    this.addButton(m1, "right", "pad2", 0x0e, 0x10);
    this.addButton(m1, "right", "pad3", 0x0e, 0x80);
    this.addButton(m1, "right", "pad4", 0x0e, 0x40);
    this.addButton(m1, "right", "pad5", 0x0e, 0x08);
    this.addButton(m1, "right", "pad6", 0x0e, 0x04);
    this.addButton(m1, "right", "pad7", 0x0e, 0x02);
    this.addButton(m1, "right", "pad8", 0x0e, 0x01);

    this.addButton(m1, "right", "sync_lock", 0x0f, 0x10);
    this.addButton(m1, "right", "cue", 0x0f, 0x20);
    this.addButton(m1, "right", "shift", 0x0f, 0x02);
    this.addButton(m1, "right", "select_deck_B", 0x0f, 0x04);
    this.addButton(m1, "right", "select_deck_D", 0x0f, 0x08);
    this.addButton(m1, "right", "jog_mode", 0x0f, 0x01);
    this.addButton(m1, "right", "tt_mode", 0x0f, 0x40);
    this.addButton(m1, "right", "grid_mode", 0x0f, 0x80);

    this.addButton(m1, "right", "move_encoder_touch", 0x10, 0x10);
    this.addButton(m1, "right", "loop_encoder_touch", 0x10, 0x08);
    this.addButton(m1, "right", "move_encoder_press", 0x10, 0x20);
    this.addButton(m1, "right", "loop_encoder_press", 0x10, 0x04);

    this.addButton(m1, "left", "wheel_touch", 0x11, 0x10);
    this.addButton(m1, "right", "wheel_touch", 0x11, 0x20);
    
    m1.addControl("message1", "unknown_0x12", 0x12, "B");
    m1.addControl("message1", "unknown_0x13", 0x13, "B");

    this.addEncoder(m1, "left", "move_encoder_rotate", 0x14, 0x0f);
    this.addEncoder(m1, "left", "loop_encoder_rotate", 0x14, 0xf0);

    this.addEncoder(m1, "left", "browse_encoder_rotate", 0x15, 0x0f);
    this.addEncoder(m1, "right", "move_encoder_rotate", 0x15, 0xf0);

    this.addEncoder(m1, "right", "loop_encoder_rotate", 0x16, 0x0f);
    this.addEncoder(m1, "right", "browse_encoder_rotate", 0x16, 0xf0);
    
    m1.addControl("message1", "unknown_0x17", 0x17, "B");
    this.controller.registerInputPacket(m1);

    // Message2 contains the positions of knobs and faders.
    this.addPotentiometer(m2, "mixer", "crossfader", 0x01);
    this.addPotentiometer(m2, "deckA", "level", 0x03);
    this.addPotentiometer(m2, "deckB", "level", 0x05);
    this.addPotentiometer(m2, "deckC", "level", 0x07);
    this.addPotentiometer(m2, "deckD", "level", 0x09);
    this.addPotentiometer(m2, "right", "tempo", 0x0b);
    this.addPotentiometer(m2, "left", "tempo", 0x0d);
    this.addPotentiometer(m2, "deckC", "gain", 0x0f);
    this.addPotentiometer(m2, "deckA", "gain", 0x11);
    this.addPotentiometer(m2, "deckB", "gain", 0x13);
    this.addPotentiometer(m2, "deckD", "gain", 0x15);
    this.addPotentiometer(m2, "mixer", "master_level", 0x17);
    this.addPotentiometer(m2, "mixer", "booth_level", 0x19);
    this.addPotentiometer(m2, "mixer", "headphone_vol", 0x1b);
    this.addPotentiometer(m2, "mixer", "headphone_mix", 0x1d);
    this.addPotentiometer(m2, "left", "fx_knob1", 0x1f);
    this.addPotentiometer(m2, "left", "fx_knob2", 0x21);
    this.addPotentiometer(m2, "left", "fx_knob3", 0x23);
    this.addPotentiometer(m2, "left", "fx_knob4", 0x25);
    this.addPotentiometer(m2, "deckC", "eq_high_knob", 0x27);
    this.addPotentiometer(m2, "deckC", "eq_mid_knob", 0x29);
    this.addPotentiometer(m2, "deckC", "eq_low_knob", 0x2b);
    this.addPotentiometer(m2, "deckA", "eq_high_knob", 0x2d);
    this.addPotentiometer(m2, "deckA", "eq_mid_knob", 0x2f);
    this.addPotentiometer(m2, "deckA", "eq_low_knob", 0x31);
    this.addPotentiometer(m2, "deckB", "eq_high_knob", 0x33);
    this.addPotentiometer(m2, "deckB", "eq_mid_knob", 0x35);
    this.addPotentiometer(m2, "deckB", "eq_low_knob", 0x37);
    this.addPotentiometer(m2, "deckD", "eq_high_knob", 0x39);
    this.addPotentiometer(m2, "deckD", "eq_mid_knob", 0x3b);
    this.addPotentiometer(m2, "deckD", "eq_low_knob", 0x3d);
    this.addPotentiometer(m2, "deckC", "mixerfx_knob", 0x3f);
    this.addPotentiometer(m2, "deckA", "mixerfx_knob", 0x41);
    this.addPotentiometer(m2, "deckB", "mixerfx_knob", 0x43);
    this.addPotentiometer(m2, "deckD", "mixerfx_knob", 0x45);
    this.addPotentiometer(m2, "right", "fx_knob1", 0x47);
    this.addPotentiometer(m2, "right", "fx_knob2", 0x49);
    this.addPotentiometer(m2, "right", "fx_knob3", 0x4b);
    this.addPotentiometer(m2, "right", "fx_knob4", 0x4d);
    m2.addControl("message2", "unknown_0x4f", 0x4f, "B");
    this.controller.registerInputPacket(m2);

    
    // Message3 is a periodic message, sent each 256 milliseconds.
    // It contains timestamps and wheel positions.
    m3.addControl("message3", "unknown_0x01", 0x01, "B");
    m3.addControl("message3", "unknown_0x02", 0x02, "B");
    m3.addControl("message3", "unknown_0x03", 0x03, "B");
    m3.addControl("message3", "unknown_0x04", 0x04, "B");
    m3.addControl("message3", "unknown_0x05", 0x05, "B");
    m3.addControl("message3", "unknown_0x06", 0x06, "B");
    m3.addControl("message3", "unknown_0x07", 0x07, "B");

    // Time ticks, 100_000 per 1 ms.
    m3.addControl("message3", "clock1", 0x08, "I");

    // Position of the left jog wheel.
    // 3170 ticks per revolution, range: 0-65535
    m3.addControl("left", "wheel_long", 0x0c, "H");

    m3.addControl("message3", "unknown_0x0e", 0x0e, "B");
    m3.addControl("message3", "unknown_0x0f", 0x0f, "B");

    // Position of the left jog wheel.
    // 3170 ticks per revolution, range: 0-2879
    m3.addControl("left", "wheel_short", 0x10, "H");

    m3.addControl("message3", "unknown_0x12", 0x12, "B");
    m3.addControl("message3", "unknown_0x13", 0x13, "B");
    m3.addControl("message3", "unknown_0x14", 0x14, "B");
    m3.addControl("message3", "unknown_0x15", 0x15, "B");
    m3.addControl("message3", "unknown_0x16", 0x16, "B");
    m3.addControl("message3", "unknown_0x17", 0x17, "B");

    m3.addControl("message3", "noisy_0x18", 0x18, "H");

    m3.addControl("message3", "noisy_0x1a", 0x1a, "B");

    // This bytes changes the value with a delta of +/-1 from time to
    // time. More often so when left wheel is being operated.
    m3.addControl("message3", "flipflop_0x1b", 0x1b, "B");

    m3.addControl("message3", "unknown_0x1c", 0x1c, "B");
    m3.addControl("message3", "unknown_0x1d", 0x1d, "B");
    m3.addControl("message3", "unknown_0x1e", 0x1e, "B");
    m3.addControl("message3", "unknown_0x1f", 0x1f, "B");
    m3.addControl("message3", "unknown_0x20", 0x20, "B");
    m3.addControl("message3", "unknown_0x21", 0x21, "B");
    m3.addControl("message3", "unknown_0x22", 0x22, "B");
    m3.addControl("message3", "unknown_0x23", 0x23, "B");

    // Time ticks, 100_000 per 1 ms.
    m3.addControl("message3", "clock2", 0x24, "I");

    // Position of the right jog wheel.
    // 3170 ticks per revolution, range: 0-65535
    m3.addControl("right", "wheel_long", 0x28, "H");

    m3.addControl("message3", "unknown_0x2a", 0x2a, "B");
    m3.addControl("message3", "unknown_0x2b", 0x2b, "B");

    // Position of the right jog wheel.
    // 3170 ticks per revolution, range: 0-2879
    m3.addControl("right", "wheel_short", 0x2c, "H");

    m3.addControl("message3", "unknown_0x2e", 0x2e, "B");
    m3.addControl("message3", "unknown_0x2f", 0x2f, "B");
    m3.addControl("message3", "unknown_0x30", 0x30, "B");
    m3.addControl("message3", "unknown_0x31", 0x31, "B");
    m3.addControl("message3", "unknown_0x32", 0x32, "B");
    m3.addControl("message3", "unknown_0x33", 0x33, "B");

    m3.addControl("message3", "noisy_0x34", 0x34, "I");

    m3.addControl("message3", "unknown_0x38", 0x38, "B");
    m3.addControl("message3", "unknown_0x39", 0x39, "B");
    m3.addControl("message3", "unknown_0x3a", 0x3a, "B");
    m3.addControl("message3", "unknown_0x3b", 0x3b, "B");
    m3.addControl("message3", "unknown_0x3c", 0x3c, "B");
    this.controller.registerInputPacket(m3);
}

// A wrapper around engine.setParameter() for debug purposes.
TraktorS4MK3_alex.setParameter = function(group, key, value) {
    this.debug("SetParameter group=" + group + ", key=" + key +
               ", value=" + value);
    engine.setParameter(group, key, value);
}

// A wrapper around engine.SetValue() for debug purposes.
TraktorS4MK3_alex.setValue = function(group, key, value) {
    this.debug("SetValue group=" + group + ", key=" + key +
               ", value=" + value);
    engine.setValue(group, key, value);
}

// A wrapper around script.toggleControl() for debug purposes.
TraktorS4MK3_alex.toggleControl = function(group, key) {
    this.debug("script.toggleControl group=" + group + ", key=" + key);
    script.toggleControl(group, key);
}

// Performs string interpolation by replacing the placeholder `${Channel}
// within the `template` with the name of the Mixxx channel mapped
// to the specified S4 deck.
TraktorS4MK3_alex.translateGroupNames = function(template, deck) {
    if (template.indexOf("${Channel}") === -1) {
        return template // No translation needed.
    }
    if (!(deck in this.decksMapping)) {
        this.unmappedDeckError(deck)
        return undefined
    }
    return template.replace("${Channel}", this.decksMapping[deck])
}

// Given a name of the S4 control and the S4 deck name produces a
// description of the corresponding Mixxx group and control.
TraktorS4MK3_alex.mapFromS4ToMixxx = function(s4ControlName, deck) {
    if (!(s4ControlName in this.s4ToMixxx)) {
        return undefined
    }
    var control = this.s4ToMixxx[s4ControlName]
    var copy = Object();
    copy["control"] = control.control

    if ("toggle" in control) {
        copy["toggle"] = true
    }

    if ("inverse" in control) {
        copy["inverse"] = true
    }

    if (control.group !== undefined) {
        mixxxGroup = this.translateGroupNames(control.group, deck)
        copy["group"] = "[" + mixxxGroup + "]"
    } else {
        this.unknownGroupError(s4ControlName, control.control)
    }
    return copy
}

// Updates the dynamic mapping of the physical left and right decks to
// the virtual decks A,B,C, and D. The 'pseudoDeck' and 'actualDeck'
// are S4 names such as 'left' and 'deckA'.
TraktorS4MK3_alex.assignDeck = function(pseudoDeck, actualDeck, field) {
    if (field.value !== 1) {
        return; // Respond only to when the button is pushed,
                // and ignore when it is released.
    }
    if (!(pseudoDeck in this.decksMapping)) {
        this.unmappedDeckError(pseudoDeck)
    }
    if (!(actualDeck in this.decksMapping)) {
        this.unmappedDeckError(actualDeck)
    }
    this.decksMapping[pseudoDeck] = this.decksMapping[actualDeck]
}

// Performs actions associated with a change in the state of the S4
// button control identified by the 'field' parameter.
TraktorS4MK3_alex.handleButton = function(field) {
    if (field.name in this.specialInputHandlers) {
        this.specialInputHandlers[field.name](field);
        return;
    }
    var control = this.mapFromS4ToMixxx(field.name, field.group)
    if (control === undefined) {
        this.noMappingError(field.name)
        return
    }

    if ("toggle" in control) {
        if (field.value === 1) {
            this.toggleControl(control.group, control.control);
        }
    } else {
        this.setValue(control.group, control.control, field.value);
    }
}

TraktorS4MK3_alex.buttonCallback = function(field) {
    // Using wrapped call to set the correct object receiver.
    TraktorS4MK3_alex.handleButton(field)
}

// Decodes the change in the encoder state based on the 'field'
// parameter. Returns -1 if the encoder was rotated counterclockwise
// or 1 if it was rotated clockwise. If the direction cannot be
// determined, returns zero.
TraktorS4MK3_alex.updateEncoderValueAndCalculateDirection = function (field) {
    var prev_value = this.encoderValues[field.id]
    var direction = 0
    if (prev_value != undefined) {
        if (field.value === this.encoderMax &&
            prev_value === this.encoderMin) {
            direction = -1
        } else if (field.value === this.encoderMin &&
                   prev_value === this.encoderMax) {
            diretion = 1
        } else {
            var delta = field.value - prev_value
            if (delta > 0) {
                direction = 1
            }
            if (delta < 0) {
                direction = -1
            }
        }
    }
    this.encoderValues[field.id] = field.value
    return direction
}

// Performs actions associated with a change in the state of the S4
// encoder control identified by the 'field' parameter.
TraktorS4MK3_alex.handleEncoder = function(field) {
    var direction = this.updateEncoderValueAndCalculateDirection(field)
    var control = this.mapFromS4ToMixxx(field.name, field.group)
    if (control === undefined) {
        this.noMappingError(field.name)
        return
    }
    if (direction !== 0) {
        this.setValue(control.group, control.control, direction);
    }
}

TraktorS4MK3_alex.encoderCallback = function(field) {
    // Using wrapped call to set the correct object receiver.
    TraktorS4MK3_alex.handleEncoder(field)
}

// Performs actions associated with a change in the state of the S4
// fader or knob control identified by the 'field' parameter.
TraktorS4MK3_alex.handlePotentiometer = function(field) {
    var control = this.mapFromS4ToMixxx(field.name, field.group)
    if (control === undefined) {
        this.noMappingError(field.name)
        return
    }

    var value = 1.0 * field.value / this.potentiometerMax;
    if ("inverse" in control) {
        value = 1.0 - value
    }
    
    this.setParameter(control.group, control.control, value);
}

TraktorS4MK3_alex.potentiometerCallback = function(field) {
    // Using wrapped call to set the correct object receiver.
    TraktorS4MK3_alex.handlePotentiometer(field)
}

// The helper functions addButton, addEncoder, and addPotentiometer
// exploit the fact that the mapping from each type of an S4 control
// to the bits on the wire follows the same format.

TraktorS4MK3_alex.addButton = function(message, group, name, offset, bitmask) {
    message.addControl(group, name, offset, "B", bitmask);
    message.setCallback(group, name, this.buttonCallback);
}

TraktorS4MK3_alex.addEncoder = function(message, group, name, offset, bitmask) {
    message.addControl(group, name, offset, "B", bitmask, true); // is encoder
    message.setCallback(group, name, this.encoderCallback);
}

TraktorS4MK3_alex.addPotentiometer = function(message, group, name, offset) {
    message.addControl(group, name, offset, "H");
    message.setCallback(group, name, this.potentiometerCallback);
}

// This function is called by the HID packet parser when it has
// recognised a particular message based on the bits provided to it.
TraktorS4MK3_alex.messageCallback = function(packet, data) {
    var s4 = TraktorS4MK3_alex
    for (var name in data) {
        if (data.hasOwnProperty(name)) {
            var field = data[name]
            if (field.callback != undefined) {
                field.callback(field)
            } else {
                s4.error("ERROR: field " + name + " is missing callback!")
            }
        }
    }
}

// An interval between two integers that includes both the lower and
// the upper boundary.
TraktorS4MK3_alex.Interval = function(boundaries) {
    return {
        from: boundaries.from,
        to: boundaries.to,
        includes: function(value) {
            return (this.from <= value) && (value <= this.to);
        },
    };
}

// Computes the time delta for wheel input. Validates the data,
// suppresses the step changes following periods of inactivity.
//
// TODO: Is this logic even needed? Can the code instead rely on the
// fact that the report 0x03 is periodic and provides both the new
// clock value and wheel positions?
TraktorS4MK3_alex.updateTimeDeltaForWheel = function(group) {
    var prevTick = this.prevWheelTick[group];
    var prevTime = this.prevWheelTime[group];

    this.prevWheelTick[group] = this.currWheelTick;
    this.prevWheelTime[group] = this.currTime;

    if (prevTick === undefined || this.currWheelTick === undefined) {
        // Ignoring the first update, delta can not be calculated.
        return 0;
    }
    if (this.currTime - prevTime > this.maxWheelWait) {
        // Ignoring an update after a long pause. The wheel is
        // expected to be sending few updates per second when it is
        // being operated.
        return 0;
    }

    var delta = this.currWheelTick - prevTick;
    if (delta < 0) {
        // The tick counter wrapped around.
        delta = this.maxWheelTick - prevTick + this.currWheelTick;
    }
    return delta;
}

// Computes the delta between the previous and the current position
// of the wheel, accounting for the wraparound conditions.
TraktorS4MK3_alex.calculateWheelDelta = function(previous, current) {
    var firstCycle = this.Interval({
        from:0,
        to: this.wheelCycleLength,
    });
    var lastCycle = this.Interval({
        from: this.maxWheelValue - this.wheelCycleLength,
        to: this.maxWheelValue,
    });

    var delta = 0;

    if (lastCycle.includes(previous) && firstCycle.includes(current)) {
        // Counter wrapped clockwise.
        delta = (this.maxWheelValue - previous) + current;
    } else if (firstCycle.includes(previous) && lastCycle.includes(current)) {
        // Counter wrapped counter-clockwise.
        delta = 0 - (previous + (this.maxWheelValue - current));
    } else {
        // The regular case.
        delta = current - previous;
    }

    return delta;
}

// Calculates wheel speed based on tick and time deltas, updates Mixxx
// if the speed is not-zero. The allowed values of the 'group'
// parameter are 'left' and 'right'.
TraktorS4MK3_alex.updateWheelSpeed = function(group, value) {
    this.prevWheelValue[group] = this.currWheelValue[group];
    this.currWheelValue[group] = value;

    // TODO: move into the calculate() function.
    if (this.prevWheelValue[group] === undefined) {
        this.prevWheelValue[group] = this.currWheelValue[group];
    }

    if (value < 0 || value > this.maxWheelValue) {
        this.error("wheel value " + value + " is out of range");
        return;
    }

    var tickDelta = this.updateTimeDeltaForWheel(group);
    var wheelDelta = this.calculateWheelDelta(
        this.prevWheelValue[group], value);

    var speed = 0;
    if (tickDelta > 0) {
        speed = wheelDelta / (tickDelta / 10000.0);
    }
    speed *= this.wheelSpeedMultiplier;

    mixxxGroup = "[" + this.translateGroupNames("${Channel}", group) + "]";
    this.setValue(mixxxGroup, "jog", speed);
}

// This function is called by the HID packet parser when it has
// recognised a particular message based on the bits provided to it.
TraktorS4MK3_alex.wheelsCallback = function(packet, data) {
    var s4 = TraktorS4MK3_alex
    for (var name in data) {
        if (data.hasOwnProperty(name)) {
            var field = data[name]
            if (field.id === "message3.clock1") {
                s4.currWheelTick = field.value
                s4.currTime = new Date().getTime();
            }
            if (field.name == "wheel_long") {
                s4.updateWheelSpeed(field.group, field.value);
            }
        }
    }
}

// This function is called by the Mixxx host before it starts sending
// data packets from the wire to this code.
TraktorS4MK3_alex.init = function(id) {
    this.info("initialising...");
    this.registerS4ToMixxxMapping();
    this.registerSpecialInputHandlers();
    this.registerInputPackets()
    // this.registerOutputPackets()
    this.info("done init");
}

// This function is called by the Mixxx host when it is terminating or
// when it is about to unload this mapping code.
TraktorS4MK3_alex.shutdown = function() {
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

// This function is called by the Mixxx host when it has received new
// data from the USB HID interface.
TraktorS4MK3_alex.incomingData = function(data, length) {
    // Packets of 23 bytes are message 0x01 and can be handed off immediately.
    if (length === 23) {
        this.controller.parsePacket(data, length);
        return;
    }

    // Windows seems to get the packet of length 79, so parse as one:
    if (length == 79) {
        this.controller.parsePacket(data, data.length);
        return;
    }

    // Packets of 64 bytes and 15 bytes are partials. We have to save
    // the 64 byte portion and then append the 15 bytes when we get it.
    if (length === 64) {
        this.partial_packet = data; return;
    }

    if (length === 15) {
        if (this.partial_packet.length !== 64) {
            this.outOfOrderBitsWarning();
            return;
        }
        // Packet data is a javascript Object with properties that are
        // integers (!). So it's actually unordered data (!!). Therefore
        // "appending" is just setting more properties.
        partial_length = this.partial_packet.length;
        for (var i = 0; i < length; i++) {
            this.partial_packet[partial_length + i] = data[i];
        }
        this.controller.parsePacket(this.partial_packet,
                                    partial_length + length);
        // Clear out the partial packet
        this.partial_packet = Object();
        return;
    }
    
    // This is the periodic report 0x03, can be parsed immediately.
    if (length === 60 && data[0] === 3) {
        this.controller.parsePacket(data, data.length);
        return;
    }
    
    this.error("Unhandled packet size: " + length + ", data[0] = " + data[0]);
}

// The logging functions, useful for debugging.

TraktorS4MK3_alex.error = function(text) {
    this.debug("ERROR: " + text)
}

TraktorS4MK3_alex.warning = function(text) {
    this.debug("WARNING: " + text)
}

TraktorS4MK3_alex.info = function(text) {
  HIDDebug("TraktorS4MK3: " + text)
}

TraktorS4MK3_alex.debug = function(text) {
  HIDDebug("TraktorS4MK3: " + text)
}

TraktorS4MK3_alex.outOfOrderBitsWarning = function() {
    this.warning("Received 15 bits. " +
                 "Assuming this is the second half of the message. " +
                 "But the first half has not been received yet, ignoring");
}

TraktorS4MK3_alex.noMappingError = function(s4ControlName) {
    this.error("Unable to find mapping from Traktor S4 control " +
               "'" + s4ControlName + "' to a Mixxx control")
}

TraktorS4MK3_alex.unknownGroupError = function(s4ControlName, mixxxControlName) {
    this.error("Unable to identify the control group for the S4 control " +
               "'" + s4ControlName + "' mapped to the Mixxx control " +
               "'" + mixxxControlName + "'")
}

TraktorS4MK3_alex.unmappedDeckError = function(deck) {
    this.error("Unable to map S4 deck '" + deck + "' to a Mixxx deck")
}

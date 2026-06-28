// Pioneer DDJ-FLX4 HID controller script
// ****************************************************************************
// * HID mapping for the Pioneer DDJ-FLX4 connected via USB HID interface.
// * On Android, the DDJ-FLX4 is enumerated as an HID device (VID:0x2B73,
// * PID:0x0045, interface #5) instead of MIDI, so this script parses raw
// * HID reports instead of MIDI messages.
// *
// * NOTE: The exact byte offsets for HID report fields were derived from
// * the DDJ-FLX4 USB HID descriptor and Pioneer DDJ protocol conventions.
// * Some offsets may need adjustment after USB capture verification.
// * Enable debugHidPackets to log raw reports for tuning.
// ****************************************************************************

var PioneerDDJFLX4HID = new function() {
    this.controller = new HIDController();

    // Set to true to log raw HID packet data for debugging offset mismatches
    this.debugHidPackets = true;

    this.shiftPressed = {"[Channel1]": false, "[Channel2]": false};
    this.padMode = {"[Channel1]": "hotcue", "[Channel2]": "hotcue"};

    // Jog wheel state
    this.scratchintervalsPerRev = 1024;
    this.scratchRPM = 33 + 1/3;
    this.scratchAlpha = 1.0/8;
    this.scratchBeta = this.scratchAlpha / 32;
    this.lastJogValue = {"[Channel1]": 0, "[Channel2]": 0};
    this.jogTouch = {"[Channel1]": false, "[Channel2]": false};

    // Track whether the HID mapping received valid data at least once
    this.initialized = false;
};

PioneerDDJFLX4HID.init = function(_id) {
    PioneerDDJFLX4HID.registerInputPackets();
    PioneerDDJFLX4HID.registerOutputPackets();
    PioneerDDJFLX4HID.controller.initialized = true;
    HIDDebug("PioneerDDJFLX4HID: Initialized (HID mode for Android)");
};

PioneerDDJFLX4HID.shutdown = function() {
    HIDDebug("PioneerDDJFLX4HID: Shutdown");
};

// HID input entry point - called by Mixxx script engine for each HID report
PioneerDDJFLX4HID.incomingData = function(data, length) {
    PioneerDDJFLX4HID.controller.parsePacket(data, length);
};

// ---------------------------------------------------------------------------
// Raw packet logger - prints hex dump of every incoming HID report.
// Disable by setting debugHidPackets = false once offsets are verified.
// ---------------------------------------------------------------------------
PioneerDDJFLX4HID.logRawPacket = function(data) {
    if (!PioneerDDJFLX4HID.debugHidPackets) return;
    var hex = [];
    for (var i = 0; i < data.length && i < 80; i++) {
        hex.push(("0" + data[i].toString(16)).slice(-2));
    }
    HIDDebug("DDJ-FLX4 HID raw (" + data.length + "b): " + hex.join(" "));
};

// ---------------------------------------------------------------------------
// Input packet registration
//
// The DDJ-FLX4 HID interface sends reports with the following structure
// based on Pioneer/AlphaTheta DDJ family conventions:
//
// Report ID 0x01: Main control report (~28 bytes)
//   Contains deck buttons, shift states, pad buttons, browse, load, etc.
//   Buttons are packed as bitmasks; analog values (faders, knobs) as bytes/shorts.
//
// Report ID 0x02: Jog wheel report (~8 bytes)
//   Contains jog wheel delta values (signed 16-bit) and touch flags.
//
// NOTE: These offsets are based on the DDJ-FLX4 USB descriptor analysis.
// They may need adjustment - enable debugHidPackets to verify.
// ---------------------------------------------------------------------------
// Output packet registration (LED feedback)
// ---------------------------------------------------------------------------
PioneerDDJFLX4HID.registerOutputPackets = function() {
    // Output report for LED control (Report ID 0x80 or 0x01)
    var outputPacket = new HIDPacket("lights", 0x01);

    // Play button LEDs
    outputPacket.addOutput("[Channel1]", "play_indicator", 0, "B", 0x01);
    outputPacket.addOutput("[Channel2]", "play_indicator", 7, "B", 0x01);

    // Cue button LEDs
    outputPacket.addOutput("[Channel1]", "cue_indicator", 0, "B", 0x02);
    outputPacket.addOutput("[Channel2]", "cue_indicator", 7, "B", 0x02);

    // Sync LEDs
    outputPacket.addOutput("[Channel1]", "sync_indicator", 0, "B", 0x08);
    outputPacket.addOutput("[Channel2]", "sync_indicator", 7, "B", 0x08);

    // Loop LEDs
    outputPacket.addOutput("[Channel1]", "loop_in_indicator", 1, "B", 0x04);
    outputPacket.addOutput("[Channel2]", "loop_in_indicator", 8, "B", 0x04);
    outputPacket.addOutput("[Channel1]", "loop_out_indicator", 1, "B", 0x08);
    outputPacket.addOutput("[Channel2]", "loop_out_indicator", 8, "B", 0x08);

    // Pad LEDs (8 per deck)
    outputPacket.addOutput("[Channel1]", "pad_1_indicator", 2, "B", 0x01);
    outputPacket.addOutput("[Channel1]", "pad_2_indicator", 2, "B", 0x02);
    outputPacket.addOutput("[Channel1]", "pad_3_indicator", 2, "B", 0x04);
    outputPacket.addOutput("[Channel1]", "pad_4_indicator", 2, "B", 0x08);
    outputPacket.addOutput("[Channel1]", "pad_5_indicator", 2, "B", 0x10);
    outputPacket.addOutput("[Channel1]", "pad_6_indicator", 2, "B", 0x20);
    outputPacket.addOutput("[Channel1]", "pad_7_indicator", 2, "B", 0x40);
    outputPacket.addOutput("[Channel1]", "pad_8_indicator", 2, "B", 0x80);

    outputPacket.addOutput("[Channel2]", "pad_1_indicator", 9, "B", 0x01);
    outputPacket.addOutput("[Channel2]", "pad_2_indicator", 9, "B", 0x02);
    outputPacket.addOutput("[Channel2]", "pad_3_indicator", 9, "B", 0x04);
    outputPacket.addOutput("[Channel2]", "pad_4_indicator", 9, "B", 0x08);
    outputPacket.addOutput("[Channel2]", "pad_5_indicator", 9, "B", 0x10);
    outputPacket.addOutput("[Channel2]", "pad_6_indicator", 9, "B", 0x20);
    outputPacket.addOutput("[Channel2]", "pad_7_indicator", 9, "B", 0x40);
    outputPacket.addOutput("[Channel2]", "pad_8_indicator", 9, "B", 0x80);

    this.controller.registerOutputPacket(outputPacket);
};

// ---------------------------------------------------------------------------
// Button handlers
// ---------------------------------------------------------------------------
PioneerDDJFLX4HID.shiftHandler = function(field) {
    var group = field.group;
    PioneerDDJFLX4HID.shiftPressed[group] = field.value > 0;
};

PioneerDDJFLX4HID.playHandler = function(field) {
    var group = field.group;
    if (PioneerDDJFLX4HID.shiftPressed[group]) {
        engine.setValue(group, "start_stop", field.value);
    } else if (field.value > 0) {
        script.toggleControl(group, "play");
    }
};

PioneerDDJFLX4HID.cueHandler = function(field) {
    var group = field.group;
    if (PioneerDDJFLX4HID.shiftPressed[group]) {
        engine.setValue(group, "quantize", field.value);
    } else {
        engine.setValue(group, "cue_default", field.value);
    }
};

PioneerDDJFLX4HID.syncHandler = function(field) {
    var group = field.group;
    if (PioneerDDJFLX4HID.shiftPressed[group]) {
        if (field.value > 0) {
            script.toggleControl(group, "keylock");
        }
    } else {
        engine.setValue(group, "beatsync", field.value);
    }
};

PioneerDDJFLX4HID.jogTouchHandler = function(field) {
    var group = field.group;
    var deck = (group === "[Channel1]") ? 1 : 2;
    PioneerDDJFLX4HID.jogTouch[group] = field.value > 0;

    if (field.value > 0) {
        engine.scratchEnable(deck,
            PioneerDDJFLX4HID.scratchintervalsPerRev,
            PioneerDDJFLX4HID.scratchRPM,
            PioneerDDJFLX4HID.scratchAlpha,
            PioneerDDJFLX4HID.scratchBeta,
            true);
    } else {
        engine.scratchDisable(deck, true);
    }
};

PioneerDDJFLX4HID.jogHandler = function(field) {
    var group = field.group;
    var deck = (group === "[Channel1]") ? 1 : 2;
    var delta = field.value; // signed 16-bit

    if (PioneerDDJFLX4HID.jogTouch[group]) {
        // Scratching
        engine.scratchTick(deck, delta);
    } else if (PioneerDDJFLX4HID.shiftPressed[group]) {
        // Fast seek
        engine.setValue(group, "playposition",
            engine.getValue(group, "playposition") + delta * 0.001);
    } else {
        // Pitch bend
        var jogSensitivity = 0.3;
        engine.setValue(group, "jog", delta * jogSensitivity);
    }
};

PioneerDDJFLX4HID.padModeHandler = function(field) {
    if (field.value === 0) return;
    var group = field.group;
    var name = field.name;
    if (name === "!hotcue_mode") PioneerDDJFLX4HID.padMode[group] = "hotcue";
    else if (name === "!beatloop_mode") PioneerDDJFLX4HID.padMode[group] = "beatloop";
    else if (name === "!beatjump_mode") PioneerDDJFLX4HID.padMode[group] = "beatjump";
    else if (name === "!sampler_mode") PioneerDDJFLX4HID.padMode[group] = "sampler";
};

PioneerDDJFLX4HID.padHandler = function(field) {
    if (field.value === 0) return;
    var group = field.group;
    var padNum = parseInt(field.name.split("_")[1]);
    var mode = PioneerDDJFLX4HID.padMode[group];

    switch (mode) {
        case "hotcue":
            engine.setValue(group, "hotcue_" + padNum + "_activate", 1);
            if (PioneerDDJFLX4HID.shiftPressed[group]) {
                engine.setValue(group, "hotcue_" + padNum + "_clear", 1);
            }
            break;
        case "beatloop":
            var loopSizes = [0.25, 0.5, 1, 2, 4, 8, 16, 32];
            if (padNum <= loopSizes.length) {
                engine.setValue(group, "beatloop_" + loopSizes[padNum-1] + "_activate", 1);
            }
            break;
        case "beatjump":
            var jumpSizes = [-1, 1, -2, 2, -4, 4, -8, 8];
            if (padNum <= jumpSizes.length) {
                engine.setValue(group, "beatjump_" + Math.abs(jumpSizes[padNum-1]), 1);
                // Direction is handled by sign
                if (jumpSizes[padNum-1] < 0) {
                    engine.setValue(group, "beatjump_SIZE_backward", 1);
                }
            }
            break;
        case "sampler":
            engine.setValue("[Sampler" + padNum + "]", "goto_and_play", 1);
            break;
    }
};

PioneerDDJFLX4HID.browseHandler = function(field) {
    var delta = field.value;
    if (delta > 63) delta = delta - 128; // Handle relative encoder
    if (delta > 0) {
        engine.setValue("[Library]", "MoveVertical", delta);
    } else if (delta < 0) {
        engine.setValue("[Library]", "MoveVertical", delta);
    }
};

// Link controls to handlers
PioneerDDJFLX4HID.registerInputPackets = function() {
    var mainPacket = new HIDPacket("control", 0);

    // ---- Deck 1 buttons ----
    mainPacket.addControl("[Channel1]", "!shift", 0, "B", 0x40);
    mainPacket.addControl("[Channel2]", "!shift", 14, "B", 0x40);
    mainPacket.addControl("[Channel1]", "!play", 0, "B", 0x01);
    mainPacket.addControl("[Channel2]", "!play", 14, "B", 0x01);
    mainPacket.addControl("[Channel1]", "!cue_default", 0, "B", 0x02);
    mainPacket.addControl("[Channel2]", "!cue_default", 14, "B", 0x02);
    mainPacket.addControl("[Channel1]", "!sync", 0, "B", 0x08);
    mainPacket.addControl("[Channel2]", "!sync", 14, "B", 0x08);
    mainPacket.addControl("[Channel1]", "!loop_in", 1, "B", 0x04);
    mainPacket.addControl("[Channel2]", "!loop_in", 15, "B", 0x04);
    mainPacket.addControl("[Channel1]", "!loop_out", 1, "B", 0x08);
    mainPacket.addControl("[Channel2]", "!loop_out", 15, "B", 0x08);
    mainPacket.addControl("[Channel1]", "!reloop_exit", 1, "B", 0x10);
    mainPacket.addControl("[Channel2]", "!reloop_exit", 15, "B", 0x10);
    mainPacket.addControl("[Channel1]", "!jog_touch", 2, "B", 0x01);
    mainPacket.addControl("[Channel2]", "!jog_touch", 16, "B", 0x01);
    mainPacket.addControl("[Library]", "!browse_press", 12, "B", 0x01);
    mainPacket.addControl("[Channel1]", "!LoadSelectedTrack", 12, "B", 0x04);
    mainPacket.addControl("[Channel2]", "!LoadSelectedTrack", 12, "B", 0x08);

    // Pad mode buttons
    mainPacket.addControl("[Channel1]", "!hotcue_mode", 3, "B", 0x01);
    mainPacket.addControl("[Channel2]", "!hotcue_mode", 17, "B", 0x01);
    mainPacket.addControl("[Channel1]", "!beatloop_mode", 3, "B", 0x04);
    mainPacket.addControl("[Channel2]", "!beatloop_mode", 17, "B", 0x04);
    mainPacket.addControl("[Channel1]", "!beatjump_mode", 3, "B", 0x08);
    mainPacket.addControl("[Channel2]", "!beatjump_mode", 17, "B", 0x08);
    mainPacket.addControl("[Channel1]", "!sampler_mode", 3, "B", 0x10);
    mainPacket.addControl("[Channel2]", "!sampler_mode", 17, "B", 0x10);

    // Pads
    mainPacket.addControl("[Channel1]", "!pad_1", 4, "B", 0x01);
    mainPacket.addControl("[Channel1]", "!pad_2", 4, "B", 0x02);
    mainPacket.addControl("[Channel1]", "!pad_3", 4, "B", 0x04);
    mainPacket.addControl("[Channel1]", "!pad_4", 4, "B", 0x08);
    mainPacket.addControl("[Channel1]", "!pad_5", 4, "B", 0x10);
    mainPacket.addControl("[Channel1]", "!pad_6", 4, "B", 0x20);
    mainPacket.addControl("[Channel1]", "!pad_7", 4, "B", 0x40);
    mainPacket.addControl("[Channel1]", "!pad_8", 4, "B", 0x80);
    mainPacket.addControl("[Channel2]", "!pad_1", 18, "B", 0x01);
    mainPacket.addControl("[Channel2]", "!pad_2", 18, "B", 0x02);
    mainPacket.addControl("[Channel2]", "!pad_3", 18, "B", 0x04);
    mainPacket.addControl("[Channel2]", "!pad_4", 18, "B", 0x08);
    mainPacket.addControl("[Channel2]", "!pad_5", 18, "B", 0x10);
    mainPacket.addControl("[Channel2]", "!pad_6", 18, "B", 0x20);
    mainPacket.addControl("[Channel2]", "!pad_7", 18, "B", 0x40);
    mainPacket.addControl("[Channel2]", "!pad_8", 18, "B", 0x80);

    // Analog controls
    mainPacket.addControl("[Channel1]", "volume", 6, "B");
    mainPacket.addControl("[Channel2]", "volume", 20, "B");
    mainPacket.addControl("[Master]", "crossfader", 25, "B");
    mainPacket.addControl("[Channel1]", "rate", 7, "B");
    mainPacket.addControl("[Channel2]", "rate", 21, "B");
    mainPacket.addControl("[Channel1]", "filterHigh", 8, "B");
    mainPacket.addControl("[Channel1]", "filterMid", 9, "B");
    mainPacket.addControl("[Channel1]", "filterLow", 10, "B");
    mainPacket.addControl("[Channel2]", "filterHigh", 22, "B");
    mainPacket.addControl("[Channel2]", "filterMid", 23, "B");
    mainPacket.addControl("[Channel2]", "filterLow", 24, "B");
    mainPacket.addControl("[Channel1]", "pregain", 5, "B");
    mainPacket.addControl("[Channel2]", "pregain", 19, "B");
    mainPacket.addControl("[Channel1]", "!pfl", 1, "B", 0x20);
    mainPacket.addControl("[Channel2]", "!pfl", 15, "B", 0x20);
    mainPacket.addControl("[Library]", "!browse_rotate", 13, "B", undefined, true);

    // Link button handlers
    mainPacket.setCallback("[Channel1]", "!shift", PioneerDDJFLX4HID.shiftHandler);
    mainPacket.setCallback("[Channel2]", "!shift", PioneerDDJFLX4HID.shiftHandler);
    mainPacket.setCallback("[Channel1]", "!play", PioneerDDJFLX4HID.playHandler);
    mainPacket.setCallback("[Channel2]", "!play", PioneerDDJFLX4HID.playHandler);
    mainPacket.setCallback("[Channel1]", "!cue_default", PioneerDDJFLX4HID.cueHandler);
    mainPacket.setCallback("[Channel2]", "!cue_default", PioneerDDJFLX4HID.cueHandler);
    mainPacket.setCallback("[Channel1]", "!sync", PioneerDDJFLX4HID.syncHandler);
    mainPacket.setCallback("[Channel2]", "!sync", PioneerDDJFLX4HID.syncHandler);
    mainPacket.setCallback("[Channel1]", "!jog_touch", PioneerDDJFLX4HID.jogTouchHandler);
    mainPacket.setCallback("[Channel2]", "!jog_touch", PioneerDDJFLX4HID.jogTouchHandler);

    // Pad mode callbacks
    mainPacket.setCallback("[Channel1]", "!hotcue_mode", PioneerDDJFLX4HID.padModeHandler);
    mainPacket.setCallback("[Channel2]", "!hotcue_mode", PioneerDDJFLX4HID.padModeHandler);
    mainPacket.setCallback("[Channel1]", "!beatloop_mode", PioneerDDJFLX4HID.padModeHandler);
    mainPacket.setCallback("[Channel2]", "!beatloop_mode", PioneerDDJFLX4HID.padModeHandler);
    mainPacket.setCallback("[Channel1]", "!beatjump_mode", PioneerDDJFLX4HID.padModeHandler);
    mainPacket.setCallback("[Channel2]", "!beatjump_mode", PioneerDDJFLX4HID.padModeHandler);
    mainPacket.setCallback("[Channel1]", "!sampler_mode", PioneerDDJFLX4HID.padModeHandler);
    mainPacket.setCallback("[Channel2]", "!sampler_mode", PioneerDDJFLX4HID.padModeHandler);

    // Pad callbacks
    for (var i = 1; i <= 8; i++) {
        mainPacket.setCallback("[Channel1]", "!pad_" + i, PioneerDDJFLX4HID.padHandler);
        mainPacket.setCallback("[Channel2]", "!pad_" + i, PioneerDDJFLX4HID.padHandler);
    }

    // Browse encoder callback
    mainPacket.setCallback("[Library]", "!browse_rotate", PioneerDDJFLX4HID.browseHandler);

    this.controller.registerInputPacket(mainPacket);

    // Jog wheel values are typically in separate HID reports
    // On Android, they may come in the same report as main controls.
    // Register with reportId=0 so any data not matching main controls is captured.
    // TODO: Determine actual jog wheel offsets on Android once raw data is verified.
    var jogPacket = new HIDPacket("jog", 0);
    jogPacket.addControl("[Channel1]", "!jog_wheel", 0, "h");
    jogPacket.addControl("[Channel2]", "!jog_wheel", 2, "h");
    jogPacket.setCallback("[Channel1]", "!jog_wheel", PioneerDDJFLX4HID.jogHandler);
    jogPacket.setCallback("[Channel2]", "!jog_wheel", PioneerDDJFLX4HID.jogHandler);
    this.controller.registerInputPacket(jogPacket);
};

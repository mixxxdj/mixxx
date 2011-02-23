// This is the companion script file for the 
// Mixman DM2 mapping (Linux driver, dm2linux.sourceforge.net)

function DM2() {}

// No specific init. MIDI reset would be nice here, to recalibrate...
DM2.init = function () {}
DM2.shutdown = function () {}

// Scratch wheel needs severe rescaling.
DM2.scratch1 = function (channel, control, value, status ) {
    engine.setValue("[Channel1]", "scratch", (value - 0x40)*0.05);
}
DM2.scratch2 = function (channel, control, value, status ) {
    engine.setValue("[Channel2]", "scratch", (value - 0x40)*0.05);
}

// For the flanger, we use one key to shift one axis into lfoPeriod mode.
DM2.flanger_shifted = 0;
DM2.shift = function (channel, control, value, status) {
    DM2.flanger_shifted = (value < 0x40) ? 0 : 1;
}
DM2.delay_period = function (channel, control, value, status) {
    if (DM2.flanger_shifted) {
	// period mode
	value = 50000.0 + (2000000.0 - 50000.0) * value / 127.0;
	engine.setValue("[Flanger]", "lfoPeriod", value);
    } else {
	// delay mode
	value = 50.0 + (10000.0 - 50.0) * value / 127.0;
	engine.setValue("[Flanger]", "lfoDelay", value);
    }
}

// Beatsync only on the platter currently not playing.
DM2.beatsync = function (channel, control, value, status) {
    if (!engine.getValue("[Channel1]", "play")) {
        engine.setValue("[Channel1]", "beatsync", value);
	return;
    }
    if (!engine.getValue("[Channel2]", "play")) {
        engine.setValue("[Channel2]", "beatsync", value);
	return;
    }
}

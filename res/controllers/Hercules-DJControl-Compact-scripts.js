// Mode button does give out midi, but the selection itself is done inside the
// controller -- no control over which mode is selected for instance.

HercDJCompact = function() {
   this.group = "[Master]";
};

HercDJCompact.init = function(id) {
    engine.setValue("[Master]", "num_samplers", 8);

    scratch = false;
    scratch_timer = [];
    scratch_timer_on = [];

    engine.connectControl("[Recording]", "status",
            "HercDJCompact.OnRecordingStatusChange");

    // Tell controller to send midi to update knob and slider positions.
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);

    // But the rate values get messed up, so reset them
    engine.setValue("[Channel1]", "rate_set_default", 1.0);
    engine.setValue("[Channel2]", "rate_set_default", 1.0);
};

HercDJCompact.shutdown = function() {
    // toggle all lights off.
    for (i = 0x01; i < 0x57; i++) {
        midi.sendShortMsg(0x90, i, 0x00);
    }
};

HercDJCompact.controls = {
    // "name" is just for reference in the code.
    "inputs": {
        0x30: { "name": "jog", "channel": 1, "group": "[Channel1]"},
        0x31: { "name": "jog", "channel": 2, "group": "[Channel2]"},
        0x37: { "name": "pitch", "channel": 1, "group": "[Channel1]"},
        0x38: { "name": "pitch", "channel": 2, "group": "[Channel2]"},
    }
};

HercDJCompact.scratch = function (group, control, value, status) {
    scratch = value > 0;
};

HercDJCompact.jog_wheel = function (group, control, value, status) {
    var input = HercDJCompact.controls.inputs[control];
    // If the high bit is 1, convert to a negative number
    if (value & 0x40) {
        value = value - 0x80;
    }
    if (scratch) {
        if (value !== 0) {
            if (scratch_timer_on[input.channel]) {
                engine.stopTimer(scratch_timer[input.channel]);
                scratch_timer_on[input.channel] = false;
            }
            if (!engine.getValue(input.group, "scratch2_enable")) {
                engine.scratchEnable(input.channel, 256, 33+1/3, 1.0/8*(0.500), (1.0/8)*(0.500)/32);
            }
            else {
                engine.scratchTick(input.channel, value);
            }
        }

        if(engine.getValue(input.group, "scratch2_enable")) {
            //when not moved for 200 msecs, probably we are not touching the wheel anymore
            scratch_timer[input.channel] =
                engine.beginTimer(200, "HercDJCompact.jog_wheelhelper("+input.channel+")", true);
            scratch_timer_on[input.channel] = true;
        }
    } else {
        if (value !== 0) {
            engine.setValue(input.group, "jog", value);
        }
    }
};

HercDJCompact.jog_wheelhelper = function(n) {
    engine.scratchDisable(n);
    scratch_timer_on[n] = false;
};

// Pitch is adjusted by holding down shift and turning the jog wheel.
HercDJCompact.pitch = function (group, control, value, status) {
    var input = HercDJCompact.controls.inputs[control];
    if (value & 0x40) {
        value = value - 0x80;
    }
    var delta = Math.pow(Math.abs(value), 2) / 1000.0;
    if (value < 0) {
        delta = -delta;
    }
    var pitch = engine.getValue(input.group, "rate") + delta;
    if (pitch > 1.0) {
        pitch = 1.0;
    }
    if (pitch < -1.0) {
        pitch = -1.0;
    }
    engine.setValue(input.group, "rate", pitch);
};

HercDJCompact.OnRecordingStatusChange = function(value, group, control) {
    // Not sure why this doesn't work with a regular midi output in the xml.
    if (value == 2) {
        midi.sendShortMsg(0x90, 0x2B, 0x7F);
        midi.sendShortMsg(0x90, 0x2C, 0x7F);
    } else {
        midi.sendShortMsg(0x90, 0x2B, 0x0);
        midi.sendShortMsg(0x90, 0x2C, 0x0);
    }
};

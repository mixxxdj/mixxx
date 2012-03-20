
function PioneerCDJ2000() {}

PioneerCDJ2000.debug = false; 
PioneerCDJ2000.jog_sensitivity = 3.0;
PioneerCDJ2000.scratch_sensitivity = 0.5;

PioneerCDJ2000.init = function(id) {}
PioneerCDJ2000.shutdown = function(id) {}

PioneerCDJ2000.jog_wheel = function(channel, control, value, status, group) {
    value = (value-0x40) / PioneerCDJ2000.jog_sensitivity;
    engine.setValue(group,'jog',value);
};

PioneerCDJ2000.jog_scratch = function(channel, control, value, status, group) {
    value = (value-0x40) / PioneerCDJ2000.scratch_sensitivity;
    engine.setValue(group,'jog',value);
};

PioneerCDJ2000.select_track_knob = function(channel, control, value, status, group) {
    if (value >= 0x01 && value <= 0x1e) {
        value = value;
    } else if (value >= 0x62 && value <= 0x7f) {
        value = 0 - (0x7f-value+1);
    } else {
        return;
    }
    engine.setValue(group,'SelectTrackKnob',value);
};


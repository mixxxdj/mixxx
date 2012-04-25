
function PioneerCDJ850() {}

PioneerCDJ850.debug = false; 
PioneerCDJ850.reverse_play = false; 
PioneerCDJ850.last_hotcue = 0; 

PioneerCDJ850.jog_sensitivity = 2.0;
PioneerCDJ850.scratch_sensitivity = 1.0;

PioneerCDJ850.init = function(id) {}
PioneerCDJ850.shutdown = function(id) {}

PioneerCDJ850.hotcue_activate = function(group,hotcue,value) {
    PioneerCDJ850.last_hotcue = hotcue;
    key = 'hotcue_' + hotcue + '_activate';
    engine.setValue(group,key,value);
}

PioneerCDJ850.reverse_toggle = function(channel, control, value, status, group) {
    if (value == 0)
        return;
    if (PioneerCDJ850.reverse_play)
       PioneerCDJ850.reverse_play = false;
    else
       PioneerCDJ850.reverse_play = true;
    engine.setValue(group,'reverse',PioneerCDJ850.reverse_play);
}

PioneerCDJ850.jog_wheel = function(channel, control, value, status, group) {
    value = (value-0x40) / PioneerCDJ850.jog_sensitivity;
    engine.setValue(group,'jog',value);
};

PioneerCDJ850.jog_scratch = function(channel, control, value, status, group) {
    value = (value-0x40) / PioneerCDJ850.scratch_sensitivity;
    engine.setValue(group,'jog',value);
};

PioneerCDJ850.select_track_knob = function(channel, control, value, status, group) {
    if (value >= 0x01 && value <= 0x1e) {
        value = value;
    } else if (value >= 0x62 && value <= 0x7f) {
        value = 0 - (0x7f-value+1);
    } else {
        return;
    }
    engine.setValue(group,'SelectTrackKnob',value);
};

PioneerCDJ850.hotcue_1_activate = function(channel, control, value, status, group) {
    PioneerCDJ850.hotcue_activate(group,1,value);
}
PioneerCDJ850.hotcue_2_activate = function(channel, control, value, status, group) {
    PioneerCDJ850.hotcue_activate(group,2,value);
}
PioneerCDJ850.hotcue_3_activate = function(channel, control, value, status, group) {
    PioneerCDJ850.hotcue_activate(group,3,value);
}
PioneerCDJ850.hotcue_4_activate = function(channel, control, value, status, group) {
    PioneerCDJ850.hotcue_activate(group,4,value);
}

PioneerCDJ850.hotcue_last_delete = function(channel, control, value, status, group) {
    if (PioneerCDJ850.last_hotcue < 1 || PioneerCDJ850.last_hotcue > 4)
        return;
    key = 'hotcue_' + PioneerCDJ850.last_hotcue + '_clear'; 
    engine.setValue(group,key,value);
    PioneerCDJ850.last_hotcue = 0;
}


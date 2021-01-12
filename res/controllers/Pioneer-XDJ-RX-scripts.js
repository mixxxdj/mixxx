function PioneerXDJRX() {}

PioneerXDJRX.debug = true;
PioneerXDJRX.jog_sensitivity = 2.0;
PioneerXDJRX.scratch_sensitivity = 1.0;

PioneerXDJRX.init = function() {
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);
    engine.softTakeover("[Channel1]", "cue_default", true);
    engine.softTakeover("[Channel2]", "cue_default", true);
    engine.softTakeover("[Channel1]", "jog", true);
    engine.softTakeover("[Channel2]", "jog", true);
    engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[Master]", "volume", true);
    engine.softTakeover("[Channel1]", "play", true);
    engine.softTakeover("[Channel2]", "play", true);
    engine.softTakeover("[Channel1]", "SelectTrackKnob", true);
    engine.softTakeover("[Channel2]", "SelectTrackKnob", true);
    engine.softTakeover("[Channel1]", "keylock", true);
    engine.softTakeover("[Channel2]", "keylock", true);
    engine.softTakeover("[Channel1]", "beatsync", true);
    engine.softTakeover("[Channel2]", "beatsync", true);
};

PioneerXDJRX.reverse_toggle = function(channel, control, value, status, group) {
    if (value === 0)
        return;
    if (PioneerXDJRX.reverse_play)
        PioneerXDJRX.reverse_play = false;
    else
        PioneerXDJRX.reverse_play = true;
    engine.setValue(group, "reverse", PioneerXDJRX.reverse_play);
};

PioneerXDJRX.tempo_btn = function(channel, control, value, status, group) {
    if (value === 0x00) {
        return;
    }

    var oldValue = engine.getValue(group, "rateRange");
    var newValue = 0.06;

    if (oldValue > 0.11) {
        newValue = 0.03;
    } else if (oldValue > 0.05) {
        newValue = 0.12;
    }

    engine.setValue(group, "rateRange", newValue);
};

PioneerXDJRX.jog_wheel = function(channel, control, value, status, group) {
    value = (value - 0x40) / PioneerXDJRX.jog_sensitivity;
    engine.setValue(group, "jog", value);

};
//Inactive
PioneerXDJRX.jog_scratch = function(channel, control, value, status, group) {
    value = (value - 0x40) / PioneerXDJRX.scratch_sensitivity;
    engine.setValue(group, "jog", value);

};

PioneerXDJRX.ClearCue1 = function(channel, control, value, status, group) {
    engine.setValue("[Channel1]", "cue_point", 0);
};

PioneerXDJRX.ClearCue2 = function(channel, control, value, status, group) {
    engine.setValue("[Channel2]", "cue_point", 0);
};

PioneerXDJRX.select_track_knob = function(channel, control, value, status, group) {
    if (value >= 0x01 && value <= 0x1e) {
        value = value;
    } else if (value >= 0x62 && value <= 0x7F) {
        value = value - 0x80;
    } else {
        return;
    }
    engine.setValue(group, "SelectTrackKnob", value);
};

var headPhone1On = false;
var headPhone1lastclicked;
PioneerXDJRX.toogleHeadPhone1 = function(channel, control, value, status) {
    var thisClick = new Date().getTime();
    if ((headPhone1lastclicked + 250) > thisClick) {
        return;
    }
    headPhone1lastclicked = thisClick;
    if (!headPhone1On) {
        midi.sendShortMsg(0x96, 0x54, 127); //headphone button 1 lights on
        engine.setValue("[Channel1]", "pfl", 1);
        headPhone1On = true;
    } else {
        midi.sendShortMsg(0x96, 0x54, 0); //headphone button 1 lights off
        engine.setValue("[Channel1]", "pfl", 0);
        headPhone1On = false;
    }
};

var headPhone2On = false;
var headPhone2lastclicked;
PioneerXDJRX.toogleHeadPhone2 = function(channel, control, value, status) {
    var thisClick = new Date().getTime();
    if ((headPhone2lastclicked + 250) > thisClick) {
        return;
    }
    headPhone2lastclicked = thisClick;
    if (!headPhone2On) {
        midi.sendShortMsg(0x96, 0x55, 127); //headphone button 2 lights on
        engine.setValue("[Channel2]", "pfl", 1);
        headPhone2On = true;
    } else {
        midi.sendShortMsg(0x96, 0x55, 0); //headphone button 2 lights off
        engine.setValue("[Channel2]", "pfl", 0);
        headPhone2On = false;
    }
};


PioneerXDJRX.PlayDecks = function(channel, control, value, status, group) {
    if (value === 0x7F) return;
    if (!engine.getValue(group, "play")) {
        engine.setValue(group, "play", 1);
    } else {
        engine.setValue(group, "play", 0);
    }
};


PioneerXDJRX.CueDecks = function(channel, control, value, status, group) {
    if (!engine.getValue(group, "cue_default")) {
        engine.setValue(group, "cue_default", 1); //Cue Pressed
        midi.sendShortMsg(status, 0x0C, 127); //Cue Light On
        midi.sendShortMsg(status, 0x0B, 127); //Play lights On
    } else {
        engine.setValue(group, "cue_default", 0); //Cue Release
        midi.sendShortMsg(status, 0x0C, 0); //Cue Light Off
        midi.sendShortMsg(status, 0x0B, 0); //Play lights Off

    }

};

PioneerXDJRX.Sync1 = function(channel, control, value, status, group) {
    if (!engine.getValue(group, "beatsync")) {
        engine.setValue(group, "beatsync", 1);
        midi.sendShortMsg(status, 0x58, 1); //Sync Light On
    } else {
        engine.setValue(group, "beatsync", 0);
        midi.sendShortMsg(status, 0x58, 0); //Sync Light Off
    }
};
PioneerXDJRX.Sync2 = function(channel, control, value, status, group) {
    if (!engine.getValue(group, "beatsync")) {
        engine.setValue(group, "beatsync", 1);
        midi.sendShortMsg(status, 0x58, 1); //Sync Light On
    } else {
        engine.setValue(group, "beatsync", 0);
        midi.sendShortMsg(status, 0x58, 0); //Sync Light Off
    }
};

var KeyLock1On = false;
var KeyLockLClk1;
PioneerXDJRX.MasterTempo1 = function(channel, control, value, status) {
    var thisClick = new Date().getTime();
    if ((KeyLockLClk1 + 250) > thisClick) {
        return;
    }
    KeyLockLClk1 = thisClick;
    if (!KeyLock1On) {
        midi.sendShortMsg(0x90, 0x1A, 127); //MasterTempo lights on
        engine.setValue("[Channel1]", "keylock", 1);
        KeyLock1On = true;
    } else {
        midi.sendShortMsg(0x90, 0x1A, 0); //MasterTempo lights off
        engine.setValue("[Channel1]", "keylock", 0);
        KeyLock1On = false;
    }
};

var KeyLock2On = false;
var KeyLockLClk2;
PioneerXDJRX.MasterTempo2 = function(channel, control, value, status) {
    var thisClick = new Date().getTime();
    if ((KeyLockLClk2 + 250) > thisClick) {
        return;
    }
    KeyLockLClk2 = thisClick;
    if (!KeyLock2On) {
        midi.sendShortMsg(0x91, 0x1A, 127); //headphone button 2 lights on
        engine.setValue("[Channel2]", "keylock", 1);
        KeyLock2On = true;
    } else {
        midi.sendShortMsg(0x91, 0x1A, 0); //headphone button 2 lights off
        engine.setValue("[Channel2]", "keylock", 0);
        KeyLock2On = false;
    }
};














PioneerXDJRX.select_turn = function(channel, control, value, status, group) {
    // CCW:  98 -> 127
    // CW:    1 ->  30

    if (value > 64) {
        value -= 128;
    }

    if (PioneerXDJRX.playlistMode) // playlist
    {
        engine.setValue("[Playlist]", "ScrollVertical", value);
    } else // tree
    {
        if (value > 0) {
            engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
        } else if (value < 0) {
            engine.setValue("[Playlist]", "SelectPrevPlaylist", 1);
        }
    }
};


PioneerXDJRX.toggle_playlist = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        engine.setValue(group, "ToggleSelectedSidebarItem", 1);
        midi.sendShortMsg(0x96, 0x55, 127);
        return (true);

    }
    PioneerXDJRX.playlistMode = !PioneerXDJRX.playlistMode;
};

PioneerXDJRX.exit_playlist = function(channel, control, value, status, group) {
    PioneerXDJRX.playlistMode = false;
};

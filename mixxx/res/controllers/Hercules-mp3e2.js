secondsBlink = 30;

MP3e2 = new Object();

function sendMsg(status, a, b) {
    midi.sendShortMsg(status, a, b);
    //controller.send([status, a, b], 3);
}

blink = new Object();

function light(id, enable) {
    if (enable == blink) {
        bval = 0x7f;
        val = 0;
    } else {
        bval = 0;
        val = enable?0x7f:0;
    }
    sendMsg(0x90, id, val);
    sendMsg(0x90, id + 0x30, bval);
}

function simpleButton(id) {
    return function(val, group) {
        var btn = (group == "[Channel1]")?(id):(id+20);

        light(btn, !!val);
    }
}

function toggleButton(id, val, group) {
    var btn = (group == "[Channel1]")?(id):(id + 20);

    light(btn, !!val);
}

function btn_loop_enabled(val, group) {
    toggleButton(3, val, group);
    toggleButton(4, val, group);
}

function btn_playposition(pos, group) {
    var secondsToEnd = engine.getValue(group, "duration") * (1 - pos);
    var btn = (group == "[Channel1]")?14:34;

    if (secondsToEnd < 1) {
        light(btn, false);
    } else if (secondsToEnd < secondsBlink) {
        light(btn, blink);
    } else {
        light(btn, true);
    }
}

binds = [
    ["loop_start_position", simpleButton(1)],
    ["loop_end_position", simpleButton(2)],
    ["hotcue_1_enabled", simpleButton(5)],
    ["hotcue_2_enabled", simpleButton(6)],
    ["hotcue_3_enabled", simpleButton(7)],
    ["hotcue_4_enabled", simpleButton(8)],
    ["play", simpleButton(15)],
    ["pfl", simpleButton(16)],
    ["playposition", btn_playposition],
    ["loop_enabled", btn_loop_enabled],
];

meta = false;
ctl_id = "";

MP3e2.init = function(id) {
    print("MP3e2 controller initialized: " + id);

    ctl_id = id;

    for (var i = 0; i < 2; i += 1) {
        group = "[Channel" + (i+1) + "]";


        for (var j in binds) {
            var control = binds[j][0];
            var func = binds[j][1];

            engine.connectControl(group, control, func);
            engine.trigger(group, control);
        }
    }

    // Turn everything off
    for (i = 1; i < 46; i += 1) {
        light(i, false);
    }

    // Request all faders report position
    sendMsg(0xb0, 0x7f, 0x7f);

    // Turn on some lights
    light(46, true);    // Automix
    //light(14, true);    // Cue A
    //light(34, true);    // Cue B
}

MP3e2.shutdown = function() {
    controller.close();
    print("MP3e2 controller shutdown: " + ctl_id);
}

meta_buttons = {
     1: "loop_start_position",
     2: "loop_end_position",
     5: "hotcue_1_clear",
     6: "hotcue_2_clear",
     7: "hotcue_3_clear",
     8: "hotcue_4_clear",
};

buttons = {
     1: "loop_in",
     2: "loop_out",
     3: "reloop_exit",
     4: "reloop_exit",
     5: "hotcue_1_activate",
     6: "hotcue_2_activate",
     7: "hotcue_3_activate",
     8: "hotcue_4_activate",
    10: "rate_temp_down",
    11: "rate_temp_up",
    12: "back",
    13: "fwd",
    14: "cue_default",
    15: "play",
    16: "pfl",
    17: "LoadSelectedTrack",
    18: "beatsync",

    41: "SelectPrevTrack",
    42: "SelectNextTrack",
};

function handleButton(id, pressed) {
    var val = pressed ? 1 : 0;
    var did = id;
    var group, ctl;

    if (id > 40) {
        group = '[Playlist]';
    } else if (id > 20) {
        group = '[Channel2]';
        did -= 20;
    } else {
        group = '[Channel1]';
    }

    if (meta) {
        ctl = meta_buttons[did];
    }
    if (! ctl) {
        ctl = buttons[did];
    }

    print(id + ": " + ctl);
    if (ctl) {
        switch (did) {
            case 1:
            case 2:
            case 5:
            case 6:
            case 7:
            case 8:
                if (meta) {
                    val = -pressed;
                }
                break;
            case 15:
            case 16:
                // Ignore button release, toggle value
                if (! pressed) {
                    return;
                }
                val = ! engine.getValue(group, ctl);
                break;
        }

        engine.setValue(group, ctl, val);
        return;
    }

    switch (id) {
        case 46:
            meta = !!pressed;
            light(30, meta);
            light(31, meta);
            light(10, meta);
            light(11, meta);
            light(19, meta);
            light(39, meta);
            break;
    }
}


faders = {
    0x35: "filterHigh",
    0x36: "filterMid",
    0x37: "filterLow"
};

function handleFader(id, val) {
    if (id < 0x34) {
        group = '[Channel' + ((id & 1) + 1) + ']';
        id = id & 0xfe;
    } else if (id < 0x39) {
        group = '[Channel1]';
    } else {
        group = '[Channel2]';
        id -= 5;
    }

    ctl = faders[id];

    switch (id) {
        case 0x30: // Jog wheel
            if (val == 127) {
                dir = -1;
            } else {
                dir = 1;
            }
            engine.setValue(group, "jog", dir);
            break;
        case 0x32: // Pitch adjust
            if (val == 127) {
                ctl = "rate_perm_down";
            } else {
                ctl = "rate_perm_up";
            }
            engine.setValue(group, ctl, 1);
            engine.setValue(group, ctl, 0);
            break;
        case 0x34:
            fval = script.absoluteLin(val, 0, 1);
            engine.setValue(group, "volume", fval);
            return;
        case 0x35:
        case 0x36:
        case 0x37:
            fval = script.absoluteNonLin(val, 0, 1, 4);
            engine.setValue(group, ctl, fval);
            break;
        case 0x38: // Crossfader
            fval = script.absoluteLin(val, -1, 1);
            engine.setValue('[Master]', 'crossfader', fval);
            break;
    }
}

MP3e2.incomingData = function(data, length) {
    for (i = 0; i < length; i += 3) {
        switch (data[i]) {
            case 0x90:
                handleButton(data[i+1], data[i+2] == 0x7f);
                break;
            case 0xb0:
                handleFader(data[i+1], data[i+2]);
                break;
            default:
                print("Unhandled packet: " + 
                      " 0x" + data[i].toString(16) +
                      " 0x" + data[i+1].toString(16) +
                      " 0x" + data[i+2].teString(16))
                  break;
        }
    }
}


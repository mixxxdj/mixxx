/****************************************************************/
/*             Xone K2 MIDI controller script v1.20             */
/*          Copyright (C) 2012, Owen Williams                   */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.11                                  */
/****************************************************************/

var XoneK2 = {};

XoneK2.shift_status = false;

// USER OPTION:
// shift_lock = true; means that pressing shift will turn on shift and it will
// stay on.
// shift_lock = false; means that shift is only active when the button is held
XoneK2.shift_lock = false;
XoneK2.deck_order = [3, 1, 2, 4];

XoneK2.leds =  [0x34,   0x35,   0x36,   0x37,
                0x58,	0x59,   0x5A,	0x5B,
                0x7C,	0x7d,	0x7e,	0x7f,
                0x30,	0x31,	0x32,	0x33,
                0x54,	0x55,	0x56,	0x57,
                0x78,	0x79,	0x7A,	0x7b,
                0x2c,	0x2d,	0x2e,	0x2f,
                0x50,	0x51,	0x52,	0x53,
                0x74,	0x75,	0x76,	0x77,
                0x28,	0x29,	0x2a,	0x2b,
                0x4c,	0x4d,	0x4e,	0x4f,
                0x70,	0x71,	0x72,	0x73,
                0x24,	0x25,	0x26,	0x27,
                0x48,	0x49,	0x4a,	0x4b,
                0x6c,	0x6d,	0x6e,	0x6f,
                0x20,	0x21,	0x22,	0x23,
                0x44,	0x45,	0x46,	0x47,
                0x68,	0x69,	0x6a,	0x6b,
                0x1c,	0x1d,	0x1e,	0x1f,
                0x40,	0x41,	0x42,	0x43,
                0x64,	0x65,	0x66,	0x67,
                0x18,	0x19,	0x1A,	0x1b,
                0x3c,	0x3d,	0x3e,	0x3f,
                0x60,	0x61,	0x62,	0x63];

XoneK2.init = function (id) {    // called when the MIDI device is opened & set up
    print ("XoneK2 id: \""+id+"\" initialized.");
    //one-shot to clear lights.  If we don't delay this call, nothing happens
    if (engine.beginTimer(2000,"XoneK2.clearlights()",true) == 0) {
        print("Clearlights timer setup failed");
    }
    XoneK2.clearlights();
}

XoneK2.shutdown = function(id) {
    XoneK2.clearlights();
}

XoneK2.clearlights = function () {
    for ( var LED in XoneK2.leds ) {
        midi.sendShortMsg(0x9F, XoneK2.leds[LED], 0x0);
    }
}

XoneK2.encoderJog = function (midichannel, control, value, status) {
    deck = XoneK2.IndexToDeck(control);
    if (value == 127) {
        jogValue = -1;
    } else {
        jogValue = 1;
    }

    if (XoneK2.shift_status) {
        //// faster seek with shift held
        //jogValue *= 5;
        //}
        //pregain = engine.getValue("[Channel" + deck + "]", "pregain");
        //engine.setValue("[Channel" + deck + "]", "pregain", pregain + (.05 * jogValue));
        rate = engine.getValue("[Channel" + deck + "]", "rate");
        engine.setValue("[Channel" + deck + "]", "rate", rate + (.005 * jogValue));
    } else {
        if (engine.getValue("[Channel" + deck + "]", "play") == 1 &&
            engine.getValue("[Channel" + deck + "]", "reverse") == 1) {
            jogValue= -(jogValue);
        }
        engine.setValue("[Channel"+deck+"]","jog",jogValue);
    }
}

XoneK2.encoderButton = function (midichannel, control, value, status) {
    deck = XoneK2.IndexToDeck(control - 52);
    if (!value) return;

    channel = "[Channel" + deck + "]"

    if (XoneK2.shift_status) {

    } else {
        engine.setValue(channel, "rate", 0.0);
    }
}


XoneK2.shift_on = function (midichannel, control, value, status) {
    if (XoneK2.shift_lock) {
        if (value == 127) {
            XoneK2.shift_status = !XoneK2.shift_status;
        }
    }
    else
    {
        if (value == 127) {
            XoneK2.shift_status = true;
        } else {
            XoneK2.shift_status = false;
        }
    }

    if (XoneK2.shift_status) {
        midi.sendShortMsg(0x9F, 0xF, 0x7F);
    } else {
        midi.sendShortMsg(0x9F, 0xF, 0x0);
    }
}

XoneK2.leftBottomKnob = function (midichannel, control, value, status) {
    if (XoneK2.shift_status)
    {
        cur_vol = engine.getValue("[Master]", "headMix");
        if (value == 1)
            cur_vol += 0.02;
        else
            cur_vol -= 0.02;
        engine.setValue("[Master]", "headMix", cur_vol);
    } else {
        cur_vol = engine.getValue("[InternalClock]", "bpm");
        if (value == 1)
            cur_vol += 0.1;
        else
            cur_vol -= 0.1;
        engine.setValue("[InternalClock]", "bpm", cur_vol);
    }
}

XoneK2.rightBottomKnob = function (midichannel, control, value, status) {
    if (XoneK2.shift_status)
    {
        cur_vol = engine.getValue("[Master]", "volume");
        if (value == 1) {
            cur_vol += 0.02;
        } else {
            cur_vol -= 0.02;
        }
        print ("set volume to " +cur_vol);
        engine.setValue("[Master]", "volume", cur_vol);
    }
    else
    {
        if (value == 1) {
            engine.setValue("[Playlist]", "SelectNextTrack", 1);
        } else {
            engine.setValue("[Playlist]", "SelectPrevTrack", 1);
        }
    }
}

XoneK2.PlayButton = function (midichannel, control, value, status) {
    deck = XoneK2.IndexToDeck(control - 24);
    if (!value) return;

    channel = "[Channel" + deck + "]"

    if (XoneK2.shift_status) {
        engine.setValue(channel, "cue_default", 1.0);
        engine.setValue(channel, "cue_default", 0.0);
    } else {
        engine.setValue(channel, "play", !engine.getValue(channel, "play"));
    }
}

XoneK2.Vinyl = function (midichannel, control, value, status) {
    deck = XoneK2.IndexToDeck(control - 28);
    if (!value) return;

    channel = "[Channel" + deck + "]"

    if (XoneK2.shift_status) {
        curval = engine.getValue(channel, "vinylcontrol_mode");
        curval = (curval + 1) % 3
        engine.setValue(channel, "vinylcontrol_mode", curval);
    } else {
        curval = engine.getValue(channel, "vinylcontrol_enabled");
        engine.setValue(channel, "vinylcontrol_enabled", !curval);
    }
}

XoneK2.IndexToDeck = function (index) {
    return XoneK2.deck_order[index];
}

// The Xone K2 uses different control numbers (second MIDI byte) to distinguish between
// different colors for the LEDs. The baseline control number sets the LED to red. Adding
// these offsets to the control number sets the LED to a different color.
XoneK2.color = {
    red: 0,
    amber: 36,
    green: 72
};
components.Component.prototype.send =  function (value) {
    if (this.midi === undefined || this.midi[0] === undefined || this.midi[1] === undefined) {
        return;
    }
    // The LEDs are turned on with a Note On MIDI message (first nybble of first byte 9)
    // and turned off with a Note Offf MIDI message (first nybble of first byte 8).
    if (value > 0) {
        midi.sendShortMsg(this.midi[0] + 0x10, this.midi[1], value);
    } else {
        midi.sendShortMsg(this.midi[0], this.midi[1], 0x7F);
    }
};

XoneK2.setColumnMidi = function (columnObject, columnNumber, midiChannel) {
    midiChannel--;
    columnObject.encoderPress.midi = [0x80 + midiChannel, 0x34 + (columnNumber-1)];

    for (var b = 1; b <= 3; b++) {
        columnObject.topButtons[b].midi = [0x80 + midiChannel,
                                           0x30 - (b-1)*4 + (columnNumber-1)];
    }

    for (var c = 1; c <= 4; c++) {
        columnObject.bottomButtons[c].midi = [0x80 + midiChannel,
                                              0x24 - (c-1)*4 + (columnNumber-1)];
    }
};

XoneK2.effectUnit = function (unitNumber, column, midiChannel) {
    components.EffectUnit.call(this, [unitNumber]);

    this.encoder = new components.Component();
    // TODO: figure out a use for this
    this.encoder.input = function () {};
    this.encoderPress = this.effectFocusButton;

    this.topButtons = [];
    for (var b = 0; b <= 3; b++) {
        this.topButtons[b] = this.enableButtons[b];
    }

    this.fader = this.dryWetKnob;

    this.bottomButtons = [];
    var channelString;
    for (var c = 1; c <= 4; c++) {
        channelString = "Channel" + c;
        this.enableOnChannelButtons.addButton(channelString);
        this.bottomButtons[c] = this.enableOnChannelButtons[channelString];
    }

    XoneK2.setColumnMidi(this, column, midiChannel);
    this.init();
};
XoneK2.effectUnit.prototype = new components.ComponentContainer();

XoneK2.midiChannels = [];
for (var ch = 0; ch <= 16; ++ch) {
    XoneK2.midiChannels[ch] = [];
}
XoneK2.midiChannels[14].columns = [];
for (var z = 1; z <= 4; z++) {
    XoneK2.midiChannels[14].columns[z] = new XoneK2.effectUnit(z, z, 14);
}

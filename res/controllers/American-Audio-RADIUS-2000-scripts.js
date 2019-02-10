/**
 * American Audio Radius 2000 controller script v1.10.0
 * Written by Markus Steinbauer 2011
 *
 * Special Thanks to the Programmer of the Behringer BCD 3000 Mapping.
 * The Mapping for the Jog Wheel and Scratch function is great.
 **/

RADIUS2000 = new Controller();

RADIUS2000.currentDeck = function (group) {
    if (group == "[Channel1]")
        return 0;
    else if (group == "[Channel2]")
        return 1;
    print("Invalid group : " + group);
    return -1; // error
}

RADIUS2000.currentDeck2 = function (group) {
    if (group == "[Channel1]")
        return "[Channel1]";
    else if (group == "[Channel2]")
        return "[Channel2]";
    
    print("Invalid group : " + group);
    return -1; // error
}

RADIUS2000.debug = false;

RADIUS2000.escratch = [false, false];

RADIUS2000.UseAcceleration = true;

RADIUS2000.JogSensitivity = 0.5;

RADIUS2000.init = function (channel, control, value, status, group) {
    midi.sendShortMsg(0x90,0x2A,0x00);
    midi.sendShortMsg(0x90,0x0B,0x7F);
    midi.sendShortMsg(0x90,0x05,0x7F);
    midi.sendShortMsg(0x90,0x30,0x7F);
    midi.sendShortMsg(0x90,0x04,0x7F);
    midi.sendShortMsg(0x90,0x02,0x7F);
    midi.sendShortMsg(0x90,0x06,0x00);
    midi.sendShortMsg(0x90,0x16,0x7F);
    midi.sendShortMsg(0x90,0x10,0x7F);
    midi.sendShortMsg(0x90,0x0A,0x7F);
    midi.sendShortMsg(0x90,0x22,0x7F);
    midi.sendShortMsg(0x90,0x19,0x7F);
    midi.sendShortMsg(0x90,0x1F,0x7F);
    midi.sendShortMsg(0x90,0x31,0x7F);
    midi.sendShortMsg(0x90,0x01,0x7F);
}

RADIUS2000.shutdown = function () {
    midi.sendShortMsg(0x90,0x2A,0x00);
    midi.sendShortMsg(0x90,0x0B,0x00);
    midi.sendShortMsg(0x90,0x05,0x00);
    midi.sendShortMsg(0x90,0x30,0x00);
    midi.sendShortMsg(0x90,0x04,0x00);
    midi.sendShortMsg(0x90,0x02,0x00);
    midi.sendShortMsg(0x90,0x06,0x00);
    midi.sendShortMsg(0x90,0x16,0x00);
    midi.sendShortMsg(0x90,0x10,0x00);
    midi.sendShortMsg(0x90,0x0A,0x00);
    midi.sendShortMsg(0x90,0x22,0x00);
    midi.sendShortMsg(0x90,0x1E,0x00);
    midi.sendShortMsg(0x90,0x19,0x00);
    midi.sendShortMsg(0x90,0x1F,0x00);
    midi.sendShortMsg(0x90,0x0C,0x00);
    midi.sendShortMsg(0x90,0x31,0x00);
    midi.sendShortMsg(0x90,0x32,0x00);
    midi.sendShortMsg(0x90,0x33,0x00);
    midi.sendShortMsg(0x90,0x01,0x00);
}

RADIUS2000.trackSearch = function (channel, control, value, status, group) {
    if (value == 0x41) {
        engine.setValue("[Playlist]","SelectNextTrack",1);
    }
    if (value == 0x3F) {
        engine.setValue("[Playlist]","SelectPrevTrack",1);
    }
}

RADIUS2000.menuSearch = function (channel, control, value, status, group) {
    if (value == 0x41) {
        engine.setValue("[Playlist]","SelectNextPlaylist",1);
    }
    if (value == 0x3F) {
        engine.setValue("[Playlist]","SelectPrevPlaylist",1);
    }
}

RADIUS2000.play = function (channel, control, value, status, group) {
    var currentlyPlaying = engine.getValue(RADIUS2000.currentDeck2(group),"play");
    if ((currentlyPlaying == 1) & (value == 0x00)) {
        engine.setValue(RADIUS2000.currentDeck2(group),"play",0);
        midi.sendShortMsg(0x90,0x2A,0x00);
    }
    if ((currentlyPlaying == 0) & (value == 0x00)) {
        engine.setValue(RADIUS2000.currentDeck2(group),"play",1);
        midi.sendShortMsg(0x90,0x2A,0x7F);
        midi.sendShortMsg(0x90,0x30,0x00);
    }
}

RADIUS2000.cue = function (channel, control, value, status, group) {
    var currentlyPlaying = engine.getValue(RADIUS2000.currentDeck2(group),"play");
    if ((currentlyPlaying == 1) & (value == 0x7F)) {
        engine.setValue(RADIUS2000.currentDeck2(group),"cue_default",1);
        midi.sendShortMsg(0x90,0x2A,0x00);
        midi.sendShortMsg(0x90,0x30,0x7F);
    }
    else if ((currentlyPlaying == 0) & (value == 0x7F)) {
        engine.setValue(RADIUS2000.currentDeck2(group),"cue_default",1);
        midi.sendShortMsg(0x90,0x30,0x7F);
    }
    else {
        engine.setValue(RADIUS2000.currentDeck2(group),"cue_default",0);
        midi.sendShortMsg(0x90,0x30,0x7F);
    }
}

RADIUS2000.keylock = function (channel, control, value, status, group) {
    var keylockStat = engine.getValue(RADIUS2000.currentDeck2(group),"keylock"); 
    if (value == 0x7F & keylockStat == 1) {
        engine.setValue(RADIUS2000.currentDeck2(group),"keylock",0);
        midi.sendShortMsg(0x90,0x06,0x00);
    }
    if (value == 0x7F & keylockStat == 0) {
        engine.setValue(RADIUS2000.currentDeck2(group),"keylock",1);
        midi.sendShortMsg(0x90,0x06,0x7F);
    }
}

RADIUS2000.pitchRateRange = function (channel, control, value, status, group) {
    var pitchRateRange = engine.getValue(RADIUS2000.currentDeck2(group),"rateRange")
    if (value == 0x7F) {
        switch (pitchRateRange) {
            case 0.04:
                engine.setValue(RADIUS2000.currentDeck2(group),"rateRange",0.08);
                midi.sendShortMsg(0x90,0x0C,0x00);
                midi.sendShortMsg(0x90,0x31,0x7F);
                midi.sendShortMsg(0x90,0x32,0x00);
                midi.sendShortMsg(0x90,0x33,0x00);
                midi.sendShortMsg(0x90,0x01,0x7F);
                break;
            case 0.08:
                engine.setValue(RADIUS2000.currentDeck2(group),"rateRange",0.16);
                midi.sendShortMsg(0x90,0x0C,0x00);
                midi.sendShortMsg(0x90,0x31,0x00);
                midi.sendShortMsg(0x90,0x32,0x7F);
                midi.sendShortMsg(0x90,0x33,0x00);
                break;
            case 0.16:
                engine.setValue(RADIUS2000.currentDeck2(group),"rateRange",1.00);
                midi.sendShortMsg(0x90,0x0C,0x00);
                midi.sendShortMsg(0x90,0x31,0x00);
                midi.sendShortMsg(0x90,0x32,0x00);
                midi.sendShortMsg(0x90,0x33,0x7F);
                midi.sendShortMsg(0x90,0x01,0x7F);
                break;
            case 1.00:
                engine.setValue(RADIUS2000.currentDeck2(group),"rateRange",0.04);
                midi.sendShortMsg(0x90,0x0C,0x7F);
                midi.sendShortMsg(0x90,0x31,0x00);
                midi.sendShortMsg(0x90,0x32,0x00);
                midi.sendShortMsg(0x90,0x33,0x00);
                midi.sendShortMsg(0x90,0x01,0x7F);
                break;
            default:
                engine.setValue(RADIUS2000.currentDeck2(group),"rateRange",0.16);
                midi.sendShortMsg(0x90,0x0C,0x00);
                midi.sendShortMsg(0x90,0x31,0x7F);
                midi.sendShortMsg(0x90,0x32,0x00);
                midi.sendShortMsg(0x90,0x33,0x00);
                midi.sendShortMsg(0x90,0x01,0x7F);
                break;
        }
    }
}

RADIUS2000.pitchRate = function (channel, control, value, status, group) {
    var pitchRate = engine.getValue(RADIUS2000.currentDeck2(group),"rateRange");
    if ((value == 0x7F) & (pitchRate != 0)) {
        engine.setValue(RADIUS2000.currentDeck2(group),"rateRange",0);
        midi.sendShortMsg(0x90,0x01,0x00);
        midi.sendShortMsg(0x90,0x0C,0x00);
        midi.sendShortMsg(0x90,0x31,0x00);
        midi.sendShortMsg(0x90,0x32,0x00);
        midi.sendShortMsg(0x90,0x33,0x00);
    }
    if ((value == 0x7F) & (pitchRate == 0)) {
        engine.setValue(RADIUS2000.currentDeck2(group),"rateRange",0.08);
        midi.sendShortMsg(0x90,0x01,0x7F);
        midi.sendShortMsg(0x90,0x31,0x7F);
    }
}

RADIUS2000.flanger = function (channel, control, value, status, group) {
    var flangerFX = engine.getValue(RADIUS2000.currentDeck2(group),"flanger");
    if ((value == 0x7F) & (flangerFX == 0)) {
        engine.setValue(RADIUS2000.currentDeck2(group),"flanger",1);
        midi.sendShortMsg(0x90,0x2D,0x7F);
    }
    if ((value == 0x7F) & (flangerFX == 1)) {
        engine.setValue(RADIUS2000.currentDeck2(group),"flanger",0);
        midi.sendShortMsg(0x90,0x2D,0x00);
    }

}

RADIUS2000.flangerDelay = function (channel, control, value, status, group) {
    var delayLevel = engine.getValue(RADIUS2000.currentDeck2(group),"lfoDelay");
    if (value == 0x41) {
        engine.setValue("Flanger","lfoDelay",10000);
    }
    if (value == 0x3F){
        engine.setValue("Flanger","lfoDelay",-10000);
    }

}


RADIUS2000.wheelTurn = function (channel, control, value, status, group) {
    deck = RADIUS2000.currentDeck(group);
    if (RADIUS2000.escratch[deck]) {
        scratchValue = (value - 0x40);
        engine.scratchTick(deck + 1, scratchValue);
    } else {

        jogValue = (value - 0x40) * RADIUS2000.JogSensitivity;
        engine.setValue(group, "jog", jogValue);
    }
}

RADIUS2000.wheelTouch = function (channel, control, value, status, group) {
    if (value == 0x7F) {
        deck = RADIUS2000.currentDeck(group);
        RADIUS2000.escratch[deck] = true;
        print(RADIUS2000.escratch[deck]);
        engine.scratchEnable(deck + 1, 100, 330, 1.0/8, (1.0/8)/32);
    }
    else {
        deck = RADIUS2000.currentDeck(group);
        RADIUS2000.escratch[deck] = false;
        engine.scratchDisable(deck + 1);
        }
}
/*
Not jet in config:
Scratch Modi: (Normal/ Scratch / A.CUE Scratch)
BPM TAP
Memory
Sample
All Effects but Flanger (Because they are not available in Mixxx)
Time
SGL/CTN
Parameter Buttons
ADV.
Folder Knob (Turning works, but pressing does not.)
*/

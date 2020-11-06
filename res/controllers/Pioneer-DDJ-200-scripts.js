var DDJ200 = {
    fourDeckMode: false,
    vDeckNo: [0, 1, 2],
    vDeck: {},
    shiftPressed: {left: false, right: false},
    jogCounter: 0
};

DDJ200.init = function() {
    for (var i = 1; i <= 4; i++) {

        // create associative arrays for 4 virtual decks
        this.vDeck[i] = {
            syncEnabled: false,
            volMSB: 0,
            rateMSB: 0,
            jogEnabled: true,
        };

        var vgroup = "[Channel" + i + "]";

        // run onTrackLoad after every track load to set LEDs accordingly
        engine.makeConnection(vgroup, "track_loaded", function(ch, vgroup) {
            DDJ200.onTrackLoad(ch, vgroup);
        });

        // set Pioneer CDJ cue mode for all decks
        engine.setValue(vgroup, "cue_cdj", true);
    }

    DDJ200.LEDsOff();

    // start with focus on library for selecting tracks (delay seems required)
    engine.beginTimer(500, function() {
        engine.setValue("[Library]", "MoveFocus", 1);
    }, true);
};

DDJ200.shutdown = function() {
    DDJ200.LEDsOff();
};

DDJ200.LEDsOff = function() {                         // turn off LED buttons:

    for (var i = 0; i <= 1; i++) {
        midi.sendShortMsg(0x96 + i, 0x63, 0x00);      // set headphone master
        midi.sendShortMsg(0x90 + i, 0x54, 0x00);      // pfl headphone
        midi.sendShortMsg(0x90 + i, 0x58, 0x00);      // beat sync
        midi.sendShortMsg(0x90 + i, 0x0B, 0x00);      // play
        midi.sendShortMsg(0x90 + i, 0x0C, 0x00);      // cue
        for (var j = 0; j <= 8; j++) {
            midi.sendShortMsg(0x97 + 2 * i, j, 0x00); // hotcue
        }
    }
};

DDJ200.onTrackLoad = function(channel, vgroup) {
    // set LEDs (hotcues, etc.) for the loaded deck
    // if controller is switched to this deck
    var vDeckNo = script.deckFromGroup(vgroup);
    var deckNo = (vDeckNo % 2) ? 1 : 2;
    if (vDeckNo === DDJ200.vDeckNo[deckNo]) {
        DDJ200.switchLEDs(vDeckNo);
    }
};

DDJ200.LoadSelectedTrack = function(channel, control, value, status, group) {
    if (value) { // only if button pressed, not releases, i.e. value === 0
        var deckNo = script.deckFromGroup(group);
        var vDeckNo = DDJ200.vDeckNo[deckNo];
        var vgroup = "[Channel" + vDeckNo + "]";
        script.triggerControl(vgroup, "LoadSelectedTrack", true);
    }
};

DDJ200.browseTracks = function(value) {
    DDJ200.jogCounter += value - 64;
    if (DDJ200.jogCounter > 9) {
        engine.setValue("[Library]", "MoveDown", true);
        DDJ200.jogCounter = 0;
    } else if (DDJ200.jogCounter < -9) {
        engine.setValue("[Library]", "MoveUp", true);
        DDJ200.jogCounter = 0;
    }
};

DDJ200.shiftLeft = function() {
    // toggle shift left pressed variable
    DDJ200.shiftPressed["left"] = ! DDJ200.shiftPressed["left"];
};

DDJ200.shiftRight = function() {
    // toggle shift right pressed variable
    DDJ200.shiftPressed["right"] = ! DDJ200.shiftPressed["right"];
};

DDJ200.jog = function(channel, control, value, status, group) {
    // For a control that centers on 0x40 (64):
    // Convert value down to +1/-1
    // Register the movement
    if (DDJ200.shiftPressed["left"]) {
        DDJ200.browseTracks(value);
    } else {
        var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
        if (DDJ200.vDeck[vDeckNo]["jogEnabled"]) {
            var vgroup = "[Channel" + vDeckNo + "]";
            engine.setValue(vgroup, "jog", value - 64);
        }
    }
};

DDJ200.scratch = function(channel, control, value, status, group) {
    // For a control that centers on 0x40 (64):
    // Convert value down to +1/-1
    // Register the movement
    engine.scratchTick(DDJ200.vDeckNo[script.deckFromGroup(group)],
        value - 64);
};

DDJ200.touch = function(channel, control, value, status, group) {
    var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
    if (value) {
        // enable scratch
        var alpha = 1.0 / 8;
        engine.scratchEnable(vDeckNo, 128, 33 + 1 / 3, alpha, alpha / 32);
        // disable jog not to prevent track alignment
        DDJ200.vDeck[vDeckNo]["jogEnabled"] = false;
    } else {
        // enable jog after 900 ms again
        engine.beginTimer(900, function() {
            DDJ200.vDeck[vDeckNo]["jogEnabled"] = true;
        }, true);
        // disable scratch
        engine.scratchDisable(vDeckNo);
    }
};

DDJ200.seek = function(channel, control, value, status, group) {
    var oldPos = engine.getValue(group, "playposition");
    // Since ‘playposition’ is normalized to unity, we need to scale by
    // song duration in order for the jog wheel to cover the same amount
    // of time given a constant turning angle.
    var duration = engine.getValue(group, "duration");
    var newPos = Math.max(0, oldPos + ((value - 64) * 0.2 / duration));

    var deckNo = script.deckFromGroup(group);
    var vgroup = "[Channel" + DDJ200.vDeckNo[deckNo] + "]";
    engine.setValue(vgroup, "playposition", newPos); // Strip search
};

DDJ200.headmix = function(channel, control, value) {
    // toggle headMix knob between -1 to 1
    if (value) { // do nothing if button is released, i.e. value === 0
        var masterMixEnabled = (engine.getValue("[Master]", "headMix") > 0);
        engine.setValue("[Master]", "headMix", masterMixEnabled ? -1 : 1);
        midi.sendShortMsg(0x96, 0x63, masterMixEnabled ? 0x7F : 0); // set LED
    }
};

DDJ200.toggleFourDeckMode = function(channel, control, value) {
    if (value) { // only if button pressed, not releases, i.e. value === 0
        DDJ200.fourDeckMode = !DDJ200.fourDeckMode;
        if (DDJ200.fourDeckMode) {
            midi.sendShortMsg(0x90, 0x54, 0x00);
            midi.sendShortMsg(0x91, 0x54, 0x00);
        } else {
            DDJ200.vDeckNo[1] = 1;
            DDJ200.vDeckNo[2] = 2;
            DDJ200.switchLEDs(1); // set LEDs of controller deck
            DDJ200.switchLEDs(2); // set LEDs of controller deck
        }
    }
};

DDJ200.play = function(channel, control, value, status, group) {
    if (value) { // only if button pressed, not releases, i.e. value === 0
        var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
        var vgroup = "[Channel" + vDeckNo + "]";
        var playing = engine.getValue(vgroup, "play");
        engine.setValue(vgroup, "play", ! playing);
        if (engine.getValue(vgroup, "play") === playing) {
            engine.setValue(vgroup, "play", !playing);
        }
        midi.sendShortMsg(status, 0x0B, engine.getValue(vgroup, "play") ? 0x7F : 0);
    }
};

DDJ200.syncEnabled = function(channel, control, value, status, group) {
    if (value) { // only if button pressed, not releases, i.e. value === 0
        var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
        var vgroup = "[Channel" + vDeckNo + "]";
        var syncEnabled = ! engine.getValue(vgroup, "sync_enabled");
        DDJ200.vDeck[vDeckNo]["syncEnabled"] = syncEnabled;
        engine.setValue(vgroup, "sync_enabled", syncEnabled);
        midi.sendShortMsg(status, control, 0x7F * syncEnabled); // set LED
    }
};

DDJ200.rateMSB = function(channel, control, value, status, group) {
    // store most significant byte value of rate
    var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
    DDJ200.vDeck[vDeckNo]["rateMSB"] = value;
};

DDJ200.rateLSB = function(channel, control, value, status, group) {
    var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
    var vgroup = "[Channel" + vDeckNo + "]";
    // calculte rate value from its most and least significant bytes
    var rateMSB = DDJ200.vDeck[vDeckNo]["rateMSB"];
    var rate = 1 - (((rateMSB << 7) + value) / 0x1FFF);
    engine.setValue(vgroup, "rate", rate);
};

DDJ200.volumeMSB = function(channel, control, value, status, group) {
    // store most significant byte value of volume
    var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
    DDJ200.vDeck[vDeckNo]["volMSB"] = value;
};

DDJ200.volumeLSB = function(channel, control, value, status, group) {
    var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
    var vgroup = "[Channel" + vDeckNo + "]";
    // calculte volume value from its most and least significant bytes
    var volMSB = DDJ200.vDeck[vDeckNo]["volMSB"];
    var vol = ((volMSB << 7) + value) / 0x3FFF;
    //var vol = ((volMSB << 7) + value); // use for linear correction
    //vol = script.absoluteNonLin(vol, 0, 0.25, 1, 0, 0x3FFF);
    engine.setValue(vgroup, "volume", vol);
};

DDJ200.eq = function(channel, control, value, status, group) {
    var val = script.absoluteNonLin(value, 0, 1, 4);
    var eq = (control === 0x0B) ? 2 : 1;
    if (control === 0x07) {
        eq = 3;
    }
    var deckNo = group.substring(24, 25);
    // var deckNo = group.match("hannel.")[0].substring(6); // more general
    // var deckNo = script.deckFromGroup(group); // working after fix
    // https://github.com/mixxxdj/mixxx/pull/3178 only
    var vDeckNo = DDJ200.vDeckNo[deckNo];
    var vgroup = group.replace("Channel" + deckNo, "Channel" + vDeckNo);
    engine.setValue(vgroup, "parameter" + eq, val);
};

DDJ200.super1 = function(channel, control, value, status, group) {
    var val = script.absoluteNonLin(value, 0, 0.5, 1);
    var deckNo = group.substring(26, 27);
    //var deckNo = group.match("hannel.")[0].substring(6); // more general
    //var deckNo = script.deckFromGroup(group); // working after fix
    // https://github.com/mixxxdj/mixxx/pull/3178 only
    var vDeckNo = DDJ200.vDeckNo[deckNo];
    var vgroup = group.replace("Channel" + deckNo, "Channel" + vDeckNo);
    engine.setValue(vgroup, "super1", val);
};

DDJ200.cueDefault = function(channel, control, value, status, group) {
    if (value) { // only if button pressed, not releases, i.e. value === 0
        var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
        var vgroup = "[Channel" + vDeckNo + "]";
        if (!DDJ200.vDeck[vDeckNo]["jogEnabled"]) {  // if jog top is touched
            engine.setValue(vgroup, "cue_set", true);
        } else {
            engine.setValue(vgroup, "cue_gotoandplay", true);
        }
        var cueSet = (engine.getValue(vgroup, "cue_point") !== -1);
        midi.sendShortMsg(status, 0x0C, 0x7F * cueSet);      // set cue LED
        midi.sendShortMsg(status, 0x0B, 0x7F *               // set play LED
                          engine.getValue(vgroup, "play"));
    }
};

DDJ200.cueGotoandstop = function(channel, control, value, status, group) {
    if (value) { // only if button pressed, not releases, i.e. value === 0
        var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
        var vgroup = "[Channel" + vDeckNo + "]";
        engine.setValue(vgroup, "cue_gotoandstop", true);
        //engine.setValue(vgroup, "start_stop", true); // go to start if preferred
        midi.sendShortMsg(status, 0x0B, 0x7F * engine.getValue(vgroup, "play"));
    }
};

DDJ200.hotcueNActivate = function(channel, control, value, status, group) {
    var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
    var vgroup = "[Channel" + vDeckNo + "]";
    var hotcue = "hotcue_" + (control + 1);
    engine.setValue(vgroup, hotcue + "_activate", true);
    midi.sendShortMsg(status, control,
        0x7F * engine.getValue(vgroup, hotcue + "_enabled"));
    var deckNo = script.deckFromGroup(group);
    midi.sendShortMsg(0x90 + deckNo - 1, 0x0B, 0x7F *
                      engine.getValue(vgroup, "play")); // set play LED
};

DDJ200.hotcueNClear = function(channel, control, value, status, group) {
    var vDeckNo = DDJ200.vDeckNo[script.deckFromGroup(group)];
    var vgroup = "[Channel" + vDeckNo + "]";
    engine.setValue(vgroup, "hotcue_" + (control + 1) + "_clear", true);
    midi.sendShortMsg(status-1, control, 0x00);        // set hotcue LEDs
};

DDJ200.pfl = function(channel, control, value, status, group) {
    if (value) { // only if button pressed, not releases, i.e. value === 0
        var deckNo = script.deckFromGroup(group);
        var vDeckNo = DDJ200.vDeckNo[deckNo];
        var vgroup = "[Channel" + vDeckNo + "]";
        var pfl = ! engine.getValue(vgroup, "pfl");
        engine.setValue(vgroup, "pfl", pfl);
        if (!DDJ200.fourDeckMode) {
            midi.sendShortMsg(status, 0x54, 0x7F * pfl);  // switch pfl LED
        }
    }
};

DDJ200.switchLEDs = function(vDeckNo) {
    // set LEDs of controller deck 1 or 2 according to virtual deck
    var d = (vDeckNo % 2) ? 0 : 1;           // d = deckNo - 1
    var vgroup = "[Channel" + vDeckNo + "]";
    midi.sendShortMsg(0x90 + d, 0x0B, 0x7F * engine.getValue(vgroup, "play"));
    midi.sendShortMsg(0x90 + d, 0x0C, 0x7F *
                      (engine.getValue(vgroup, "cue_point") !== -1));
    midi.sendShortMsg(0x90 + d, 0x58, 0x7F * engine.getValue(vgroup,
        "sync_enabled"));
    if (!DDJ200.fourDeckMode) {
        midi.sendShortMsg(0x90 + d, 0x54,
            0x7F * engine.getValue(vgroup, "pfl"));
    }

    for (var i = 1; i <= 8; i++) {
        midi.sendShortMsg(0x97 + 2 * d, i - 1, 0x7F * engine.getValue(
            vgroup, "hotcue_" + i + "_enabled"));
    }
};

DDJ200.toggleDeck = function(channel, control, value, status, group) {
    if (value) { // only if button pressed, not releases, i.e. value === 0
        if (DDJ200.shiftPressed["left"]) {
            // left shift + pfl 1/2 does not toggle decks but loads track
            DDJ200.LoadSelectedTrack(channel, control, value, status, group);
        } else if (DDJ200.fourDeckMode) { //right shift + pfl 1/2 toggles
            var deckNo = script.deckFromGroup(group);
            var vDeckNo;
            var led = 0x7F;
            if (deckNo === 1) {
                // toggle virtual deck of controller deck 1
                DDJ200.vDeckNo[1] = 4 - DDJ200.vDeckNo[1];
                if (DDJ200.vDeckNo[1] === 1) {
                    led = 0;
                }
                vDeckNo = DDJ200.vDeckNo[1];
            } else { // deckNo === 2
                // toggle virtual deck of controller deck 2
                DDJ200.vDeckNo[2] = 6 - DDJ200.vDeckNo[2];
                if (DDJ200.vDeckNo[2] === 2) {
                    led = 0;
                }
                vDeckNo = DDJ200.vDeckNo[2];
            }
            midi.sendShortMsg(status, 0x54, led); // toggle virtual deck LED
            DDJ200.switchLEDs(vDeckNo); // set LEDs of controller deck
        }
    }
};

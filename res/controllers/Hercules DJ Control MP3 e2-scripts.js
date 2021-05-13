var HerculesMP3e2 = function() {};

// Control schema: http://blog.ebruni.it/blog/wp-content/uploads/2010/01/Hercules-mp3e2-schema-comandi.jpg

// Number of the standard RPM value. Lower values increase de sensitivity as the really records.
var standardRpm = 33.33;

// The alpha value for the filter (start with 1/8 (0.125) and tune from there)
var alpha = 1 / 8;

// The beta value for the filter (start with alpha/32 and tune from there)
var beta = alpha / 20;

// Timer to disable the scratch if the "jog wheel" is stopped for "x" milliseconds (default = 60)
var scratchResetTime = 60;

// Tune the jog sensitivity when the scratch mode is disabled (default = 1, increase for increase the sensitivity
var jogSensitivity = 0.8;

// Debug switch. set to true to print debug log messages in console.
var debug = false;

// 4-decks variables
var deckA = 1;
var deckB = 2;

var scratchMode = 0;
var scratchTimer = 0;
var wheelMove = [0, 0, 0, 0];

var playlist = false;

// HerculesMP3e2.controls is completely useless but I kept it here
// to be used as a reference of MP3e2 MIDI controls
/*HerculesMP3e2.controls = {
    "inputs": {
    0x11: { "channel": 1, "name": "loadA",         "type": "button" },
    0x25: { "channel": 2, "name": "loadB",         "type": "button" },
    0x0B: { "channel": 1, "name": "pitchbend+",     "type": "button" },
    0x0A: { "channel": 1, "name": "pitchbend-",     "type": "button" },
    0x1F: { "channel": 2, "name": "pitchbend+",     "type": "button" },
    0x1E: { "channel": 2, "name": "pitchbend-",     "type": "button" },
    0x12: { "channel": 1, "name": "sync",             "type": "button" },
    0x26: { "channel": 2, "name": "sync",             "type": "button" },
    0x13: { "channel": 1, "name": "mastertempo", "type": "button" },
    0x27: { "channel": 2, "name": "mastertempo", "type": "button" },
    0x0F: { "channel": 1, "name": "play",             "type": "button" },
    0x23: { "channel": 2, "name": "play",             "type": "button" },
    0x0E: { "channel": 1, "name": "cue",             "type": "button" },
    0x22: { "channel": 2, "name": "cue",             "type": "button" },
    0x2D: { "channel": 1, "name": "scratch",         "type": "button" },
    0x2E: { "channel": 1, "name": "automix",         "type": "button" },
    0x01: { "channel": 1, "name": "K1",             "type": "button" },
    0x02: { "channel": 1, "name": "K2",             "type": "button" },
    0x03: { "channel": 1, "name": "K3",             "type": "button" },
    0x04: { "channel": 1, "name": "K4",             "type": "button" },
    0x05: { "channel": 1, "name": "K5",             "type": "button" },
    0x06: { "channel": 1, "name": "K6",             "type": "button" },
    0x07: { "channel": 1, "name": "K7",             "type": "button" },
    0x08: { "channel": 1, "name": "K8",             "type": "button" },
    0x15: { "channel": 2, "name": "K1",             "type": "button" },
    0x16: { "channel": 2, "name": "K2",             "type": "button" },
    0x17: { "channel": 2, "name": "K3",             "type": "button" },
    0x18: { "channel": 2, "name": "K4",             "type": "button" },
    0x19: { "channel": 2, "name": "K5",             "type": "button" },
    0x1A: { "channel": 2, "name": "K6",             "type": "button" },
    0x1B: { "channel": 2, "name": "K7",             "type": "button" },
    0x1C: { "channel": 2, "name": "K8",             "type": "button" },
    0x30: { "channel": 1, "name": "wheel",         "type": "pot" },
    0x31: { "channel": 2, "name": "wheel",         "type": "pot" },
    0x2C: { "channel": 1, "name": "folder",         "type": "button" },
    0x2B: { "channel": 1, "name": "files",         "type": "button" },
    },

    // ADD 0x30 to hexa for blinking LED

    Ex: 0x2C => folder fixed led
        0x5C => folder blinking led

}; */

// This function is executed when mixxx is starting
HerculesMP3e2.init = function(id, debugging) {
    if (debugging) {
        debug = true;
    }

    if (debug) {
        print("*** Hercules MP3 e2 id " + id + " initialization begins");
    }

    // Switch off all LEDs
    for (var i = 1; i < 95; i++) {
        midi.sendShortMsg(0x90, i, 0x00);
    }

    // Switch-on some LEDs for improve the usability
    midi.sendShortMsg(0x90, 0x2B, 0x7F); // Files LED

    if (debug) {
        print("*** Connecting controls of deck A to [Channel" + deckA + "]");
    }
    HerculesMP3e2.connectControl(deckA);
    HerculesMP3e2.updateLeds(deckA);

    if (debug) {
        print("*** Connecting controls of deck B to [Channel" + deckB + "]");
    }
    HerculesMP3e2.connectControl(deckB);
    HerculesMP3e2.updateLeds(deckB);

    if (debug) {
        print("*** Hercules MP3 e2 initialization complete");
    }

};

HerculesMP3e2.shutdown = function() {
    if (debug) {
        print("*** Hercules MP3 e2 shutdown begins. Swhitching off all leds");
    }

    // Switch off all LEDs
    for (var i = 1; i < 95; i++) {
        midi.sendShortMsg(0x90, i, 0x00);
    }

    if (debug) {
        print("*** Hercules MP3 e2 shutdown complete, Bye...");
    }
};

// TODO: Include function to activate switching
// This function return group with 1/3 and 2/4 modifier.
HerculesMP3e2.switchDeck = function(group) {
    var deck = group.replace("Channel1", "Channel" + deckA);
    deck = deck.replace("Channel2", "Channel" + deckB);
    return deck;
};

HerculesMP3e2.connectControl = function(deck, remove) {
    remove = (typeof remove !== "undefined") ? remove : false; // default value for remove is false

    engine.connectControl("[Channel" + deck + "]", "cue_indicator", "HerculesMP3e2.cueLed", remove);
    engine.connectControl("[Channel" + deck + "]", "play_indicator", "HerculesMP3e2.playLed", remove);
    engine.connectControl("[Channel" + deck + "]", "sync_enabled", "HerculesMP3e2.syncmode", remove);
    engine.connectControl("[Channel" + deck + "]", "pfl", "HerculesMP3e2.pflLeds", remove);
    if (debug) {
        print("*** " + (remove === true) ? "Disable" : "Enable" + " soft takeover for [EqualizerRack1_[Channel" + deck + "]_Effect1].parameter[1-3]");
    }
    engine.softTakeover("[EqualizerRack1_[Channel" + deck + "]_Effect1]", "parameter1", remove !== true);
    engine.softTakeover("[EqualizerRack1_[Channel" + deck + "]_Effect1]", "parameter2", remove !== true);
    engine.softTakeover("[EqualizerRack1_[Channel" + deck + "]_Effect1]", "parameter3", remove !== true);
    if (debug) {
        print("*** " + (remove === true) ? "Disable" : "Enable" + " soft takeover for [Channel" + deck + "].volume");
    }
    engine.softTakeover("[Channel" + deck + "]", "volume", remove !== true);
};

// This function calls engine.trigger to update Leds of DeckA or DeckB after a deck change (switch between 1/3 ou 2/4)
HerculesMP3e2.updateLeds = function(deck) {
    engine.trigger("[Channel" + deck + "]", "cue_indicator");
    engine.trigger("[Channel" + deck + "]", "play_indicator");
    engine.trigger("[Channel" + deck + "]", "sync_enabled");
    engine.trigger("[Channel" + deck + "]", "pfl");
};

HerculesMP3e2.selectTrack = function(_midino, control, value, _status, _group) {
    //normal: goes 1 track down or up

    if (playlist === true) {
        if (control === 0x29 && value) {
            engine.setValue("[Playlist]", "SelectPrevPlaylist", 1);
        } else if (control === 0x2A && value) {
            engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
        }
    } else {
        if (control === 0x29 && value) {
            engine.setValue("[Playlist]", "SelectPrevTrack", 1);
        } else if (control === 0x2A && value) {
            engine.setValue("[Playlist]", "SelectNextTrack", 1);
        }
    }

};

HerculesMP3e2.keyButton = function(_midino, control, value, _status, group) {
    // Loops/FX
    // FIX: Loops led on after changing beatloop

    var deck = (group === "[Channel1]") ? deckA : deckB;

    switch (control) {
    case 0x01:
    case 0x15:
        // K1
        // Normal : Loop 1
        script.toggleControl("[Channel" + deck + "]", "beatloop_1_toggle");
        if (engine.getValue("[Channel" + deck + "]", "beatloop_1_enabled") === 1) {
            midi.sendShortMsg(0x90, control, 0x7F); // Led ON
        } else {
            midi.sendShortMsg(0x90, control, 0x00); // Led OFF
        }
        break;

    case 0x02:
    case 0x16:
        // K2
        // Normal : Loop 2
        script.toggleControl("[Channel" + deck + "]", "beatloop_2_toggle");
        if (engine.getValue("[Channel" + deck + "]", "beatloop_2_enabled") === 1) {
            midi.sendShortMsg(0x90, control, 0x7F); // Led ON
        } else {
            midi.sendShortMsg(0x90, control, 0x00); // Led OFF
        }
        break;
    case 0x03:
    case 0x17:
        // K3
        // Normal : Loop 4
        script.toggleControl("[Channel" + deck + "]", "beatloop_4_toggle");
        if (engine.getValue("[Channel" + deck + "]", "beatloop_4_enabled") === 1) {
            midi.sendShortMsg(0x90, control, 0x7F); // Led ON
        } else {
            midi.sendShortMsg(0x90, control, 0x00); // Led OFF
        }
        break;
    case 0x04:
    case 0x18:
        // K4 : Loop 8
        script.toggleControl("[Channel" + deck + "]", "beatloop_8_toggle");
        if (engine.getValue("[Channel" + deck + "]", "beatloop_8_enabled") === 1) {
            midi.sendShortMsg(0x90, control, 0x7F); // Led ON
        } else {
            midi.sendShortMsg(0x90, control, 0x00); // Led OFF
        }
        break;

    case 0x05:
    case 0x19:
        // K5 : Effect 1
        if (value) {
            script.toggleControl("[EffectRack1_EffectUnit" + deck + "_Effect1]", "enabled"); // Switch on/off
            if (engine.getValue("[EffectRack1_EffectUnit" + deck + "_Effect1]", "enabled") === 1) {
                midi.sendShortMsg(0x90, control, 0x7F); // Led ON
            } else {
                midi.sendShortMsg(0x90, control, 0x00); // Led OFF
            }
        }
        break;
    case 0x06:
    case 0x1A:
        // K6 : Effect 2
        if (value) {
            script.toggleControl("[EffectRack1_EffectUnit" + deck + "_Effect2]", "enabled"); // Switch on/off
            if (engine.getValue("[EffectRack1_EffectUnit" + deck + "_Effect2]", "enabled") === 1) {
                midi.sendShortMsg(0x90, control, 0x7F); // Led ON
            } else {
                midi.sendShortMsg(0x90, control, 0x00); // Led OFF
            }
        }
        break;
    case 0x07:
    case 0x1B:
        // K7 : Effect 3
        if (value) {
            script.toggleControl("[EffectRack1_EffectUnit" + deck + "_Effect3]", "enabled"); // Switch on/off
            if (engine.getValue("[EffectRack1_EffectUnit" + deck + "_Effect3]", "enabled") === 1) {
                midi.sendShortMsg(0x90, control, 0x7F); // Led ON
            } else {
                midi.sendShortMsg(0x90, control, 0x00); // Led OFF
            }
        }
        break;
    case 0x08:
    case 0x1C:
        // K8 : Nothing
        break;

    }
};

HerculesMP3e2.scroll = function(_midino, control, value, _status, _group) {
    // Folder/Files buttons
    // Normal: Toggle selected sidebar item

    if (control === 0x2C && value) {
        // Go to folder side
        playlist = true;
        midi.sendShortMsg(0x90, 0x2C, 0x7F); // Folder LED ON
        midi.sendShortMsg(0x90, 0x2B, 0x00); // Files LED OFF
        engine.setValue("[Playlist]", "ToggleSelectedSidebarItem", 1);

    } else if (control === 0x2B && value) {
        // Go to files side
        playlist = false;
        midi.sendShortMsg(0x90, 0x2C, 0x00); // Folder LED OFF
        midi.sendShortMsg(0x90, 0x2B, 0x7F); // Files LED ON
    }
};

HerculesMP3e2.masterTempo = function(_midino, _control, value, _status, group) {

    var deck = HerculesMP3e2.switchDeck(group);

    if (value) {
        engine.setValue(deck, "sync_enabled", (engine.getValue(deck, "sync_enabled") === 0) ? 1 : 0);
    }

};

HerculesMP3e2.wheelOnOff = function() {
    // Wheel Deck A / Channel 1
    if (wheelMove[0]) {
        engine.scratchEnable(1, 128, standardRpm, alpha, beta);
    } else engine.scratchDisable(1);
    wheelMove[0] = 0;

    //Wheel Deck B / Channel 2
    if (wheelMove[1]) {
        engine.scratchEnable(2, 128, standardRpm, alpha, beta);
    } else engine.scratchDisable(2);
    wheelMove[1] = 0;

    // Wheel Deck A / Channel 3
    if (wheelMove[2]) {
        engine.scratchEnable(3, 128, standardRpm, alpha, beta);
    } else engine.scratchDisable(3);
    wheelMove[2] = 0;

    //Wheel Deck B / Channel 4
    if (wheelMove[3]) {
        engine.scratchEnable(4, 128, standardRpm, alpha, beta);
    } else engine.scratchDisable(4);
    wheelMove[3] = 0;
};

HerculesMP3e2.sync = function(_midino, control, value, _status, group) {
    //Normal: Sync

    var deck = HerculesMP3e2.switchDeck(group);
    engine.setValue(deck, "beatsync", value ? 1 : 0);
    midi.sendShortMsg(0x90, control, (value) ? 0x7F : 0x00);

};

HerculesMP3e2.scratch = function(_midino, _control, value, _status, _group) {
    // Normal scratch function
    if (value) {
        if (scratchMode === 0) {
            // Enable the scratch mode on the corresponding deck and start the timer
            scratchMode = 1;
            scratchTimer = engine.beginTimer(scratchResetTime, "HerculesMP3e2.wheelOnOff()");
            midi.sendShortMsg(0x90, 45, 0x7F); // Switch-on the Scratch led
        } else {
            // Disable the scratch mode on the corresponding deck and stop the timer
            scratchMode = 0;
            engine.stopTimer(scratchTimer);
            midi.sendShortMsg(0x90, 45, 0x00); // Switch-off the Scratch led
        }
    }
};

HerculesMP3e2.jogWheel = function(_midino, _control, value, _status, group) {
    var deck = (group === "[Channel1]") ? deckA : deckB;

    // This function is called everytime the jog is moved
    if (value === 0x01) {
        if (scratchMode) {
            engine.scratchTick(deck, 1);
            wheelMove[deck - 1] = 1;
        } else
            engine.setValue("[Channel" + deck + "]", "jog", jogSensitivity);
    } else {
        if (scratchMode) {
            engine.scratchTick(deck, -1);
            wheelMove[deck - 1] = 1;
        } else
            engine.setValue("[Channel" + deck + "]", "jog", -jogSensitivity);
    }
};

HerculesMP3e2.loadTrack = function(_midino, _control, value, _status, group) {
    // Normal : Load track to deckA or deckB
    // FIX: pfl starting on both

    var deck = HerculesMP3e2.switchDeck(group);

    if (value && engine.getValue(deck, "play") !== 1) { // Load the selected track in the corresponding deck only if the track is paused
        engine.setValue(deck, "LoadSelectedTrack", 1);
        engine.setValue(deck, "rate", 0);
        print("Deck : " + deck);
        if (deck === "[Channel1]" && !engine.getValue(deck, "pfl")) {
            HerculesMP3e2.pfl(null, null, 0x7F, null, deck); // enable pfl deckA
            HerculesMP3e2.pfl(null, null, 0x7F, null, "[Channel2]"); // disable pfl deckB
        } else if (deck === "[Channel2]" && !engine.getValue(deck, "pfl")) {
            HerculesMP3e2.pfl(null, null, 0x7F, null, deck); // enable pfl deckB
            HerculesMP3e2.pfl(null, null, 0x7F, null, "[Channel1]"); // disable pfl deckA
        }
    } else {
        engine.setValue(deck, "LoadSelectedTrack", 0);
    }

};

HerculesMP3e2.pitchbend = function(_midino, control, value, _status, group) {
    // Simple: temporary pitch adjust

    var deck = HerculesMP3e2.switchDeck(group);

    // Pitchbend +
    if (control === 0x0B || control === 0x1F) {
        engine.setValue(deck, "rate_temp_up", value ? 1 : 0);
        midi.sendShortMsg(0x90, control, (value) ? 0x7F : 0x00);
    } else { // Pitchbend -
        engine.setValue(deck, "rate_temp_down", value ? 1 : 0);
        midi.sendShortMsg(0x90, control, (value) ? 0x7F : 0x00);
    }
};

HerculesMP3e2.wind = function(_midino, control, value, _status, group) {
    //normal: fwd and rwd

    var deck = HerculesMP3e2.switchDeck(group);

    if (control === 0x21 || control === 0x0D) { //forward
        engine.setValue(deck, "fwd", value ? 1 : 0);
    } else { // Backward
        engine.setValue(deck, "back", value ? 1 : 0);
    }
};

HerculesMP3e2.pfl = function(_midino, _control, value, _status, group) {
    //normal: pfl / Headphones

    var deck = HerculesMP3e2.switchDeck(group);

    if (value) {
        engine.setValue(deck, "pfl", !(engine.getValue(deck, "pfl")));
    }
};

HerculesMP3e2.cue = function(_channel, _control, value, _status, group) {
    var deck = HerculesMP3e2.switchDeck(group);

    // Don't set Cue accidentally at the end of the song
    if (engine.getValue(deck, "playposition") <= 0.97) {
        engine.setValue(deck, "cue_default", value ? 1 : 0);
    } else {
        engine.setValue(deck, "cue_preview", value ? 1 : 0);
    }
};

HerculesMP3e2.volume = function(_midino, _control, value, _status, group) {
    var deck = HerculesMP3e2.switchDeck(group);
    var newValue = script.absoluteLin(value, 0, 1);

    engine.setValue(deck, "volume", newValue);
};

HerculesMP3e2.play = function(_midino, _control, value, _status, group) {
    //normal: play

    var deck = HerculesMP3e2.switchDeck(group);

    if (value) {
        engine.setValue(deck, "play", !(engine.getValue(deck, "play")));
    }
};

// Cue Led
HerculesMP3e2.cueLed = function(value, group, _control) {
    if (((group === "[Channel1]") && (deckA === 1)) || ((group === "[Channel3]") && (deckA === 3))) {
        midi.sendShortMsg(0x90, 14, (value) ? 0x7F : 0x00); // Switch-on Cue Led
    } else if (((group === "[Channel2]") && (deckB === 2)) || ((group === "[Channel4]") && (deckB === 4))) {
        midi.sendShortMsg(0x90, 34, (value) ? 0x7F : 0x00); // Switch-on Cue Led
    }
};

// Play led
HerculesMP3e2.playLed = function(value, group, _control) {
    if (((group === "[Channel1]") && (deckA === 1)) || ((group === "[Channel3]") && (deckA === 3))) {
        midi.sendShortMsg(0x90, 15, (value) ? 0x7F : 0x00); // Switch-on Play Led
    } else if (((group === "[Channel2]") && (deckB === 2)) || ((group === "[Channel4]") && (deckB === 4))) {
        midi.sendShortMsg(0x90, 35, (value) ? 0x7F : 0x00); // Switch-on Play Led
    }
};

// Switch-on the pfl / headphone Led if pfl is set
HerculesMP3e2.pflLeds = function(value, group, _control) {
    if (((group === "[Channel1]") && (deckA === 1)) || ((group === "[Channel3]") && (deckA === 3))) {
        if (value) {
            midi.sendShortMsg(0x90, 16, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 16, 0x00);
        }
    } else if (((group === "[Channel2]") && (deckB === 2)) || ((group === "[Channel4]") && (deckB === 4))) {
        if (value) {
            midi.sendShortMsg(0x90, 36, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 36, 0x00);
        }
    }
};

HerculesMP3e2.syncmode = function(value, group, _control) {

    if (value === 1) { // Sync_enabled => fixed
        if (((group === "[Channel1]") && (deckA === 1)) || ((group === "[Channel3]") && (deckA === 3))) {
            midi.sendShortMsg(0x90, 67, 0x00); //blink off
            midi.sendShortMsg(0x90, 19, 0x7F); //fixed on
        } else if (((group === "[Channel2]") && (deckB === 2)) || ((group === "[Channel4]") && (deckB === 4))) {
            midi.sendShortMsg(0x90, 87, 0x00); //blink off
            midi.sendShortMsg(0x90, 39, 0x7F); //fixed on
        }
    } else if (value === 0) { //None => led off
        if (((group === "[Channel1]") && (deckA === 1)) || ((group === "[Channel3]") && (deckA === 3))) {
            midi.sendShortMsg(0x90, 67, 0x00); //blink off
            midi.sendShortMsg(0x90, 19, 0x00); //fixed off
        } else if (((group === "[Channel2]") && (deckB === 2)) || ((group === "[Channel4]") && (deckB === 4))) {
            midi.sendShortMsg(0x90, 87, 0x00); //blink off
            midi.sendShortMsg(0x90, 39, 0x00); //fixed off
        }
    }
};

HerculesMP3e2.pitch = function(_midino, _control, value, _status, group) {
    // Simple: pitch slider
    var deck = HerculesMP3e2.switchDeck(group);
    engine.setValue(deck, (value === 1) ? "rate_perm_up" : "rate_perm_down", 1);
    engine.setValue(deck, (value === 1) ? "rate_perm_up" : "rate_perm_down", 0);
};

HerculesMP3e2.filterKnob = function(group, control, value) {
    var deck = HerculesMP3e2.switchDeck(group);
    engine.setValue("[EqualizerRack1_" + deck + "_Effect1]", control, script.absoluteNonLin(value, 0, 1, 4));
};

HerculesMP3e2.filterHigh = function(_midino, _control, value, _status, group) {
    HerculesMP3e2.filterKnob(group, "parameter3", value);
};

HerculesMP3e2.filterMid = function(_midino, _control, value, _status, group) {
    HerculesMP3e2.filterKnob(group, "parameter2", value);
};

HerculesMP3e2.filterLow = function(_midino, _control, value, _status, group) {
    HerculesMP3e2.filterKnob(group, "parameter1", value);
};

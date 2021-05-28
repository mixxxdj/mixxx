////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////
function HerculesMP3e2() {}

// Control schema: http://blog.ebruni.it/blog/wp-content/uploads/2010/01/Hercules-mp3e2-schema-comandi.jpg
// Image: http://www.hablarcom.com.br/Imagens/arquivos/jptech/DJ_control_MP3_2_frente.jpg
// Explanation: http://www.mixxx.org/wiki/doku.php/hercules_dj_control_mp3_e2

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

// Lower jog sensitivity when selecting playlists
// Count each step until jogPlaylistSensitivityDivider is reached, then change Playlist
//
// TODO(XXX): add a jogPlaylistCounter reset timer if no jogWheel move is done for some time
//
var jogPlaylistSensitivityDivider = 3;
var jogPlaylistSense = 0;
var jogPlaylistCounter = 0;

var superButtonHold = 0;
var automixPressed = false;
var scratchMode = 0;
var scratchTimer = 0;
var wheelMove = [0, 0, 0, 0];

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
}; */


// This function return group with 1/3 and 2/4 modifier.
HerculesMP3e2.switchDeck = function(group) {
    var deck = group.replace("Channel1", "Channel" + deckA);
    deck = deck.replace("Channel2", "Channel" + deckB);
    return deck;
};


// This function connect a control or remove the connection if the remove parameter is set to true.
HerculesMP3e2.connectControl = function(deck, remove) {
    remove = (typeof remove !== 'undefined') ? remove : false; // default value for remove is false

    engine.connectControl("[Channel" + deck + "]", "cue_indicator", "HerculesMP3e2.cueLed", remove);
    engine.connectControl("[Channel" + deck + "]", "play_indicator", "HerculesMP3e2.playLed", remove);
    engine.connectControl("[Channel" + deck + "]", "loop_start_position", "HerculesMP3e2.loopStartSetLeds", remove);
    engine.connectControl("[Channel" + deck + "]", "loop_end_position", "HerculesMP3e2.loopEndSetLeds", remove);
    engine.connectControl("[Channel" + deck + "]", "loop_enabled", "HerculesMP3e2.loopEnabledLeds", remove);
    engine.connectControl("[Channel" + deck + "]", "hotcue_1_enabled", "HerculesMP3e2.hotcueLeds", remove);
    engine.connectControl("[Channel" + deck + "]", "hotcue_2_enabled", "HerculesMP3e2.hotcueLeds", remove);
    engine.connectControl("[Channel" + deck + "]", "hotcue_3_enabled", "HerculesMP3e2.hotcueLeds", remove);
    engine.connectControl("[Channel" + deck + "]", "hotcue_4_enabled", "HerculesMP3e2.hotcueLeds", remove);
    engine.connectControl("[Channel" + deck + "]", "sync_enabled", "HerculesMP3e2.syncmode", remove);
    engine.connectControl("[Channel" + deck + "]", "pfl", "HerculesMP3e2.pflLeds", remove);
    if (debug) {
        print("*** " + (remove === true) ? "Disable" : "Enable" + " soft takeover for [EqualizerRack1_[Channel" + deck + "]_Effect1].parameter[1-3]");
    }
    engine.softTakeover("[EqualizerRack1_[Channel" + deck + "]_Effect1]", "parameter1", (remove === true) ? false : true);
    engine.softTakeover("[EqualizerRack1_[Channel" + deck + "]_Effect1]", "parameter2", (remove === true) ? false : true);
    engine.softTakeover("[EqualizerRack1_[Channel" + deck + "]_Effect1]", "parameter3", (remove === true) ? false : true);
    if (debug) {
        print("*** " + (remove === true) ? "Disable" : "Enable" + " soft takeover for [Channel" + deck + "].volume");
    }
    engine.softTakeover("[Channel" + deck + "]", "volume", (remove === true) ? false : true);
};

// This function prints its argument values for debug purposes
// Use it like this :     engine.connectControl("[Channel1]", "volume", "HerculesMP3e2.debugControl");
HerculesMP3e2.debugControl = function(value, group, control) {
    print("*** Received control: value=" + value + ", group=" + group + ", control=" + control + ";");
};

// This function calls engine.trigger to update Leds of DeckA or DeckB after a deck change (switch between 1/3 ou 2/4)
HerculesMP3e2.updateLeds = function(deck) {
    engine.trigger("[Channel" + deck + "]", "cue_indicator");
    engine.trigger("[Channel" + deck + "]", "play_indicator");
    engine.trigger("[Channel" + deck + "]", "loop_start_position");
    engine.trigger("[Channel" + deck + "]", "loop_end_position");
    engine.trigger("[Channel" + deck + "]", "loop_enabled");
    engine.trigger("[Channel" + deck + "]", "hotcue_1_enabled");
    engine.trigger("[Channel" + deck + "]", "hotcue_2_enabled");
    engine.trigger("[Channel" + deck + "]", "hotcue_3_enabled");
    engine.trigger("[Channel" + deck + "]", "hotcue_4_enabled");
    engine.trigger("[Channel" + deck + "]", "sync_enabled");
    engine.trigger("[Channel" + deck + "]", "pfl");
};

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

    midi.sendShortMsg(0xB0, 0x7F, 0x7F);

    // Switch-on some LEDs for improve the usability
    midi.sendShortMsg(0x90, 46, 0x7F); // Automix LED

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

HerculesMP3e2.automix = function(midino, control, value, status, group) {
    // SHIFT BUTTON
    // The "Automix" button is used like a shift button. When this is hold
    //    down, many commands of the console has another functions

    // Button pressed
    if (value) {
        superButtonHold = 1;
        automixPressed = true;
        // Switch-on some LEDs
        midi.sendShortMsg(0x90, 30, 0x7F); // Pitchbend - DB
        midi.sendShortMsg(0x90, 31, 0x7F); // Pitchbend + DB
        midi.sendShortMsg(0x90, 10, 0x7F); // Pitchbend - DA
        midi.sendShortMsg(0x90, 11, 0x7F); // Pitchbend + DA
        midi.sendShortMsg(0x90, 18, 0x7F); // Sync DA
        midi.sendShortMsg(0x90, 38, 0x7F); // Sync DB
    }
    // Button released
    else {
        if (superButtonHold != 2) {
            superButtonHold = 0;
        }
        automixPressed = false;
        // Switch-off some LEDs
        midi.sendShortMsg(0x90, 30, 0x00); // Pitchbend - DB
        midi.sendShortMsg(0x90, 31, 0x00); // Pitchbend + DB
        midi.sendShortMsg(0x90, 10, 0x00); // Pitchbend - DA
        midi.sendShortMsg(0x90, 11, 0x00); // Pitchbend + DA
        midi.sendShortMsg(0x90, 18, 0x00); // Sync DA
        midi.sendShortMsg(0x90, 38, 0x00); // Sync DB
    }
};


// Enable/disable the magnet or enable/disable the keylock tempo if shifted or define a loop if supershifted
HerculesMP3e2.masterTempo = function(midino, control, value, status, group) {

    var deck = HerculesMP3e2.switchDeck(group);

    if (superButtonHold == 2) {
        if (value) {
            engine.setValue(deck, "quantize", !(engine.getValue(deck, "quantize")));
        }
    } else if (superButtonHold == 1) {
        if (value && scratchMode === 0) {
            engine.setValue(deck, "keylock", (engine.getValue(deck, "keylock") === 0) ? 1 : 0);
        }
    } else {
        if (value) {
            engine.setValue(deck, "sync_enabled", (engine.getValue(deck, "sync_enabled") === 0) ? 1 : 0);
        }
    }
};


HerculesMP3e2.loadTrack = function(midino, control, value, status, group) {
    // Normal : Load track to deckA or deckB
    // Shifted : Load into sampler
    // Supershifted : Deck switch 1/3 or 2/4

    if (superButtonHold == 2) {
        if (value) {
            // Deck switch between 1/3 or 2/4
            if (control == 0x11) {
                if (debug) {
                    print("*** remove Deck A controls from [Channel" + deckA + "]");
                }

                HerculesMP3e2.connectControl(deckA, true); // remove connected controls for deckA

                deckA = (deckA == 1) ? 3 : 1; // Switch Deck
                midi.sendShortMsg(0x90, 44, (deckA == 3) ? 0x7F : 0x00); // Folder Led On if DeckA = [Channel3] or Off if DeckA = [Channel1]

                HerculesMP3e2.connectControl(deckA); // Connect new controls for deckA
                HerculesMP3e2.updateLeds(deckA); // Update all leds for deckA according to new Channel

                if (debug) {
                    print("*** Switched Deck A to [Channel" + deckA + "]");
                }
            } else {
                HerculesMP3e2.connectControl(deckB, true); // remove connected controls for deckB

                deckB = (deckB == 2) ? 4 : 2; // Switch Deck
                midi.sendShortMsg(0x90, 43, (deckB == 4) ? 0x7F : 0x00); // Folder Led On if DeckB = [Channel4] or Off if DeckB = [Channel2]

                HerculesMP3e2.connectControl(deckB); // Connect new controls for deckB
                HerculesMP3e2.updateLeds(deckB); // Update all leds for DeckB according to new Channel

                if (debug) {
                    print("*** Switched Deck B to [Channel" + deckB + "]");
                }
            }
        }
    } else {
        var deck = HerculesMP3e2.switchDeck(group);
        if (superButtonHold == 1) {
            deck = deck.replace("Channel", "Sampler");
        }

        if (value && engine.getValue(deck, "play") != 1) { // Load the selected track in the corresponding deck only if the track is paused
            engine.setValue(deck, "LoadSelectedTrack", 1);
            engine.setValue(deck, "rate", 0);
        } else {
            engine.setValue(deck, "LoadSelectedTrack", 0);
        }
    }
};

HerculesMP3e2.scroll = function(midino, control, value, status, group) {
    // Folder/Files buttons
    // Normal: Toggle selected sidebar item
    // Shifted: play sampler
    // Supershifted: stop sampler

    var deck = "";
    if (control == 0x2C) {
        deck = "[Sampler" + deckA + "]";
    } else {
        deck = "[Sampler" + deckB + "]";
    }

    if (value) {
        if (superButtonHold == 2) {
            engine.setValue(deck, "start_stop", 1);
        } else if (superButtonHold == 1) {
            engine.setValue(deck, "cue_gotoandplay", 1);
        } else {
            engine.setValue("[Playlist]", "ToggleSelectedSidebarItem", 1);
        }
    }
};

/*
// NOT USED

HerculesMP3e2.holdTimer = function (group, first, second) {

    holdButtonFlag = 1;
    engine.setValue(group, first, 0);    // Set "Off" the first function
    engine.setValue(group, second, 1);    // Set "On" the second function
};


HerculesMP3e2.holdButton = function (group, value, first, second) {
    // This feature allows you to perform a different function if a button is
    // pressed for 2 seconds. When the button is pressed, the first function is
    // performed. If the button is hold down for 2 seconds, the second function
    // is performed and the first function is disables.

    if (value) {
        engine.setValue(group, first, 1);    // Set "On" the first function
        holdTimerID = engine.beginTimer(2000, "HerculesMP3e2.holdTimer(\""+group+"\", \""+first+"\", \""+second+"\")", true);
        }
    else {
        if (holdButtonFlag) {
            engine.setValue(group, second, 0);    // Set "Off" the second function
            holdButtonFlag = 0;
        }
        else {
            engine.stopTimer(holdTimerID);
            engine.setValue(group, first, 0);    // Set "Off" the first function
        }
    }
};

*/

HerculesMP3e2.keyButton = function(midino, control, value, status, group) {
    // Loop command for the first 4 Key, Hotcues command for the latest 4

    var deck = HerculesMP3e2.switchDeck(group);

    switch (control) {
        // Loop buttons
        // normal: loop in, loop out, reloop/exit, loop 4
        // shift: loop 1, loop 1/2, loop 1/4, loop 2
        // supershift: loop 8, loop 16, loop 1/8, clear loop
        case 0x01:
        case 0x15: // K1, Loop in
            if (superButtonHold == 2 && value) {
                engine.setValue(deck, "loop_start_position", -1);
                engine.setValue(deck, "loop_end_position", -1);
            } else if (superButtonHold == 1 && value) {
                engine.setValue(deck, "beatloop_0.25_toggle", 1);
            } else engine.setValue(deck, "loop_in", value ? 1 : 0);
            break;
        case 0x02:
        case 0x16: // K2, Loop out
            if (superButtonHold == 2 && value) {
                engine.setValue(deck, "beatloop_0.125_toggle", 1);
            } else if (superButtonHold == 1 && value) {
                engine.setValue(deck, "beatloop_0.5_toggle", 1);
            } else engine.setValue(deck, "loop_out", value ? 1 : 0);
            break;
        case 0x03:
        case 0x17: // K3, Reloop/Exit
            if (superButtonHold == 2 && value) {
                engine.setValue(deck, "beatloop_8_toggle", 1);
            } else if (superButtonHold == 1 && value) {
                engine.setValue(deck, "beatloop_1_toggle", 1);
            } else engine.setValue(deck, "reloop_exit", value ? 1 : 0);
            break;
        case 0x04:
        case 0x18: // K4, Reloop/Exit
            if (superButtonHold == 2 && value) {
                engine.setValue(deck, "beatloop_16_toggle", 1);
            } else if (superButtonHold == 1 && value) {
                engine.setValue(deck, "beatloop_2_toggle", 1);
            } else if (value) {
                engine.setValue(deck, "beatloop_4_toggle", 1);
            }
            break;

            // Hotcue buttons:
            // Simple press: go to the hotcue position
            // Shift (hold down "Automix"): clear the hotcue
        case 0x05:
        case 0x19: // K5
            if (superButtonHold == 2 && value) {
                //Nothing
            } else if (superButtonHold == 1) {
                engine.setValue(deck, "hotcue_1_clear", value ? 1 : 0);
            } else {
                engine.setValue(deck, "hotcue_1_activate", value ? 1 : 0);
            }
            break;

        case 0x06:
        case 0x1A: // K6
            if (superButtonHold == 2 && value) {
                //Nothing
            } else if (superButtonHold == 1) {
                engine.setValue(deck, "hotcue_2_clear", value ? 1 : 0);
            } else {
                engine.setValue(deck, "hotcue_2_activate", value ? 1 : 0);
            }
            break;

        case 0x07:
        case 0x1B: // K7
            if (superButtonHold == 2 && value) {
                //Nothing
            } else if (superButtonHold == 1) {
                engine.setValue(deck, "hotcue_3_clear", value ? 1 : 0);
            } else {
                engine.setValue(deck, "hotcue_3_activate", value ? 1 : 0);
            }
            break;

        case 0x08:
        case 0x1C: // K8
            if (superButtonHold == 2 && value) {
                //Nothing
            } else if (superButtonHold == 1) {
                engine.setValue(deck, "hotcue_4_clear", value ? 1 : 0);
            } else {
                engine.setValue(deck, "hotcue_4_activate", value ? 1 : 0);
            }
            break;
    }
};

HerculesMP3e2.pitch = function(midino, control, value, status, group) {
    // Simple: pitch slider
    // Shifted: Headphone volume and pre/main (these are 4-deck independent)
    // Supershifted: QuickEffect Filter knob

    var sign;
    var newValue;

    if (superButtonHold == 2) {
        sign = (value == 0x01) ? 1 : -1;

        if (group == "[Channel1]") {
            newValue = HerculesMP3e2.knobIncrement("[QuickEffectRack1_[Channel" + deckA + "]]", "super1", 0, 1, 0.5, 20, sign);
            engine.setValue("[QuickEffectRack1_[Channel" + deckA + "]]", "super1", newValue);
        }
        if (group == "[Channel2]") {
            newValue = HerculesMP3e2.knobIncrement("[QuickEffectRack1_[Channel" + deckB + "]]", "super1", 0, 1, 0.5, 20, sign);
            engine.setValue("[QuickEffectRack1_[Channel" + deckB + "]]", "super1", newValue);
        }
    } else if (superButtonHold == 1) {
        sign = (value == 0x01) ? 1 : -1;

        if (group == "[Channel1]") {
            newValue = HerculesMP3e2.knobIncrement("[Master]", "headVolume", 0, 5, 1, 30, sign);
            engine.setValue("[Master]", "headVolume", newValue);
        }
        if (group == "[Channel2]") {
            newValue = HerculesMP3e2.knobIncrement("[Master]", "headMix", -1, 1, 0, 20, sign);
            engine.setValue("[Master]", "headMix", newValue);
        }
    } else {
        var deck = HerculesMP3e2.switchDeck(group);
        engine.setValue(deck, (value == 1) ? "rate_perm_up" : "rate_perm_down", 1);
        engine.setValue(deck, (value == 1) ? "rate_perm_up" : "rate_perm_down", 0);
    }
};

HerculesMP3e2.knobIncrement = function(group, action, minValue, maxValue, centralValue, step, sign) {
    // This function allows you to increment a non-linear value like the volume's knob
    // sign must be 1 for positive increment, -1 for negative increment
    var semiStep = step / 2;
    var rangeWidthLeft = centralValue - minValue;
    var rangeWidthRight = maxValue - centralValue;
    var actual = engine.getValue(group, action);
    var newValue;
    var increment;

    if (actual < 1) {
        increment = ((rangeWidthLeft) / semiStep) * sign;
    } else if (actual > 1) {
        increment = ((rangeWidthRight) / semiStep) * sign;
    } else if (actual == 1) {
        increment = (sign == 1) ? rangeWidthRight / semiStep : (rangeWidthLeft / semiStep) * sign;
    }

    if (sign == 1 && actual < maxValue) {
        newValue = actual + increment;
    } else if (sign == -1 && actual > minValue) {
        newValue = actual + increment;
    }

    return newValue;
};

HerculesMP3e2.pitchbend = function(midino, control, value, status, group) {
    // Simple: temporary pitch adjust
    // Shift:  old: pregain adjust, new: double/half loop
    // Supershift: disable low/high

    var deck = HerculesMP3e2.switchDeck(group);

    if (superButtonHold == 2 && value) {
        // disable high
        if (control == 0x0B || control == 0x1F) {
            engine.setValue(deck, "filterHighKill", !(engine.getValue(deck, "filterHighKill")));
        }
        // disable low
        else {
            engine.setValue(deck, "filterLowKill", !(engine.getValue(deck, "filterLowKill")));
        }
    } else if (superButtonHold == 1) {
        // double loop
        if (control == 0x0B || control == 0x1F) {
            engine.setValue(deck, "loop_double", value ? 1 : 0);
        }
        // half loop
        else {
            engine.setValue(deck, "loop_halve", value ? 1 : 0);
        }
    } else {
        // Pitchbend +
        if (control == 0x0B || control == 0x1F) {
            engine.setValue(deck, "rate_temp_up", value ? 1 : 0);
        }
        // Pitchbend -
        else {
            engine.setValue(deck, "rate_temp_down", value ? 1 : 0);
        }
    }
};


HerculesMP3e2.cue = function(channel, control, value, status, group) {
    var deck = HerculesMP3e2.switchDeck(group);

    // Don't set Cue accidentally at the end of the song
    if (engine.getValue(deck, "playposition") <= 0.97) {
        engine.setValue(deck, "cue_default", value ? 1 : 0);
    } else {
        engine.setValue(deck, "cue_preview", value ? 1 : 0);
    }
};

HerculesMP3e2.scratch = function(midino, control, value, status, group) {
    if (superButtonHold >= 1) {
        // Add an second Control Button if Automix and Scratch are pressed - Supershift
        if (value) {
            superButtonHold = 2;
            midi.sendShortMsg(0x90, 78, 0x7F); // Blink Pitchbend - DB
            midi.sendShortMsg(0x90, 79, 0x7F); // Blink Pitchbend + DB
            midi.sendShortMsg(0x90, 58, 0x7F); // Blink Pitchbend - DA
            midi.sendShortMsg(0x90, 59, 0x7F); // Blink Pitchbend + DA
            midi.sendShortMsg(0x90, 66, 0x7F); // Blink Sync DA
            midi.sendShortMsg(0x90, 86, 0x7F); // Blink Sync DB
        } else {
            midi.sendShortMsg(0x90, 78, 0x00); // Blink Pitchbend - DB
            midi.sendShortMsg(0x90, 79, 0x00); // Blink Pitchbend + DB
            midi.sendShortMsg(0x90, 58, 0x00); // Blink Pitchbend - DA
            midi.sendShortMsg(0x90, 59, 0x00); // Blink Pitchbend + DA
            midi.sendShortMsg(0x90, 66, 0x00); // Blink Sync DA
            midi.sendShortMsg(0x90, 86, 0x00); // Blink Sync DB
            if (automixPressed) {
                superButtonHold = 1;
            } else {
                superButtonHold = 0;
            }
        }
    } else {
        // Normal scratch function
        if (value) {
            if (scratchMode === 0) {
                // Enable the scratch mode on the corresponding deck and start the timer
                scratchMode = 1;
                scratchTimer = engine.beginTimer(scratchResetTime, "HerculesMP3e2.wheelOnOff()");
                midi.sendShortMsg(0x90, 45, 0x7F); // Switch-on the Scratch led
                engine.setValue("[Channel1]", "keylock", 0);
                engine.setValue("[Channel2]", "keylock", 0);
                engine.setValue("[Channel3]", "keylock", 0);
                engine.setValue("[Channel4]", "keylock", 0);
            } else {
                // Disable the scratch mode on the corresponding deck and stop the timer
                scratchMode = 0;
                engine.stopTimer(scratchTimer);
                midi.sendShortMsg(0x90, 45, 0x00); // Switch-off the Scratch led
            }
        }
    }
};

HerculesMP3e2.sync = function(midino, control, value, status, group) {
    //Normal: Sync
    //Shifted: Adjust Beatgrid
    //Supershifted: Kill Mid

    var deck = HerculesMP3e2.switchDeck(group);

    if (superButtonHold == 2) {
        if (value) {
            engine.setValue(deck, "filterMidKill", !(engine.getValue(deck, "filterMidKill")));
        }
    } else if (superButtonHold == 1) {
        engine.setValue(deck, "beats_translate_curpos", value ? 1 : 0);
    } else {
        engine.setValue(deck, "beatsync", value ? 1 : 0);
    }
};


// This function is called every "scratchResetTime" seconds and checks if the wheel was moved in the previous interval
// (every interval last "scratchResetTime" seconds). If the wheel was moved enables the scratch mode, else disables it.
// In this way I have made a simple workaround to simulate the touch-sensitivity of the other controllers.

HerculesMP3e2.wheelOnOff = function() {
    // Wheel Deck A / Channel 1
    if (wheelMove[0]) {
        engine.scratchEnable(1, 128, standardRpm, alpha, beta);
    }
    else engine.scratchDisable(1);
    wheelMove[0] = 0;

    //Wheel Deck B / Channel 2
    if (wheelMove[1]) {
        engine.scratchEnable(2, 128, standardRpm, alpha, beta);
    }
    else engine.scratchDisable(2);
    wheelMove[1] = 0;

    // Wheel Deck A / Channel 3
    if (wheelMove[2]) {
        engine.scratchEnable(3, 128, standardRpm, alpha, beta);
    }
    else engine.scratchDisable(3);
    wheelMove[2] = 0;

    //Wheel Deck B / Channel 4
    if (wheelMove[3]) {
        engine.scratchEnable(4, 128, standardRpm, alpha, beta);
    }
    else engine.scratchDisable(4);
    wheelMove[3] = 0;
};


HerculesMP3e2.jogWheel = function(midino, control, value, status, group) {
    var deck = (group == "[Channel1]") ? deckA : deckB;

    // This function is called everytime the jog is moved
    if (value == 0x01) {
        if (superButtonHold == 2) {
            if ((deck == 1) || (deck == 3)) {
                if (jogPlaylistSense != 1) {
                    jogPlaylistSense = 1;
                    jogPlaylistCounter = jogPlaylistSensitivityDivider; // Force doing it NOW as we changes rotation sense
                } else {
                    jogPlaylistCounter += 1;
                }
                if (jogPlaylistCounter >= jogPlaylistSensitivityDivider) {
                    jogPlaylistCounter = 0;
                    engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
                }
            } else {
                engine.setValue("[Playlist]", "SelectNextTrack", 1);
            }
        } else {
            if (scratchMode) {
                engine.scratchTick(deck, 1);
                wheelMove[deck - 1] = 1;
            } else
                engine.setValue("[Channel" + deck + "]", "jog", jogSensitivity);
        }
    } else {
        if (superButtonHold == 2) {
            if ((deck == 1) || (deck == 3)) {
                if (jogPlaylistSense != -1) {
                    jogPlaylistSense = -1;
                    jogPlaylistCounter = jogPlaylistSensitivityDivider; // Force doing it NOW as we changes rotation sense
                } else {
                    jogPlaylistCounter += 1;
                }
                if (jogPlaylistCounter >= jogPlaylistSensitivityDivider) {
                    jogPlaylistCounter = 0;
                    engine.setValue("[Playlist]", "SelectPrevPlaylist", 1);
                }
            } else {
                engine.setValue("[Playlist]", "SelectPrevTrack", 1);
            }
        } else {
            if (scratchMode) {
                engine.scratchTick(deck, -1);
                wheelMove[deck - 1] = 1;
            } else
                engine.setValue("[Channel" + deck + "]", "jog", -jogSensitivity);
        }
    }
};

// drive master tempo led connected to sync_mode
HerculesMP3e2.syncmode = function(value, group, control) {
    // Following code was used for sync_leader control.
    // Deactivated for now due to https://bugs.launchpad.net/mixxx/+bug/1456801
    // currently (2015-05-20) explicit master mode is not supported.
    // Switched to sync_enabled (binary) control

    /*    if (value == 2) //Master => Blink
        {
            if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3)))
            {
                midi.sendShortMsg(0x90,19,0x00); //fixed off
                midi.sendShortMsg(0x90,67,0x7F); //blink on
            }
            else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4)))
            {
                midi.sendShortMsg(0x90,39,0x00); //fixed off
                midi.sendShortMsg(0x90,87,0x7F); //blink on
            }
        }
        else */

    if (value == 1) { // Sync_enabled => fixed
        if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
            midi.sendShortMsg(0x90, 67, 0x00); //blink off
            midi.sendShortMsg(0x90, 19, 0x7F); //fixed on
        } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
            midi.sendShortMsg(0x90, 87, 0x00); //blink off
            midi.sendShortMsg(0x90, 39, 0x7F); //fixed on
        }
    } else if (value === 0) { //None => led off
        if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
            midi.sendShortMsg(0x90, 67, 0x00); //blink off
            midi.sendShortMsg(0x90, 19, 0x00); //fixed off
        } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
            midi.sendShortMsg(0x90, 87, 0x00); //blink off
            midi.sendShortMsg(0x90, 39, 0x00); //fixed off
        }
    }
};

HerculesMP3e2.cueLed = function(value, group, control) {
    if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
        midi.sendShortMsg(0x90, 14, (value) ? 0x7F : 0x00); // Switch-on Cue Led
    } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
        midi.sendShortMsg(0x90, 34, (value) ? 0x7F : 0x00); // Switch-on Cue Led
    }
};

// Play led
HerculesMP3e2.playLed = function(value, group, control) {
    if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
        midi.sendShortMsg(0x90, 15, (value) ? 0x7F : 0x00); // Switch-on Play Led
    } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
        midi.sendShortMsg(0x90, 35, (value) ? 0x7F : 0x00); // Switch-on Play Led
    }
};

// Switch on the hotcue leds
HerculesMP3e2.hotcueLeds = function(value, group, control) {
    var ledStatus = (value === 0) ? 0x00 : 0x7F;
    var ledNo = 0;
    if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
        switch (control) {
            case "hotcue_1_enabled":
                ledNo = 5;
                break;
            case "hotcue_2_enabled":
                ledNo = 6;
                break;
            case "hotcue_3_enabled":
                ledNo = 7;
                break;
            case "hotcue_4_enabled":
                ledNo = 8;
                break;
        }
        midi.sendShortMsg(0x90, ledNo, ledStatus);
    } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
        switch (control) {
            case "hotcue_1_enabled":
                ledNo = 25;
                break;
            case "hotcue_2_enabled":
                ledNo = 26;
                break;
            case "hotcue_3_enabled":
                ledNo = 27;
                break;
            case "hotcue_4_enabled":
                ledNo = 28;
                break;
        }
        midi.sendShortMsg(0x90, ledNo, ledStatus);
    }
};

// Switch-on the K1 Led if the loop start is set
HerculesMP3e2.loopStartSetLeds = function(loopStartPos, group) {
    if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
        if (loopStartPos != -1) {
            midi.sendShortMsg(0x90, 1, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 1, 0x00);
        }
    } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
        if (loopStartPos != -1) {
            midi.sendShortMsg(0x90, 21, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 21, 0x00);
        }
    }
};

// Switch-on the K2 Led if the loop end is set
HerculesMP3e2.loopEndSetLeds = function(loopEndPos, group) {
    if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
        if (loopEndPos != -1) {
            midi.sendShortMsg(0x90, 2, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 2, 0x00);
        }
    } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
        if (loopEndPos != -1) {
            midi.sendShortMsg(0x90, 22, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 22, 0x00);
        }
    }
};

HerculesMP3e2.wind = function(midino, control, value, status, group) {
    //normal: fwd and rwd
    //shift: adjust pregain
    //supershift: same as shift

    var deck = HerculesMP3e2.switchDeck(group);
    var newValue = 0;

    if (control == 0x21 || control == 0x0D) { //forward

        if (superButtonHold == 2) {
            if (debug) {
                print("**** BRAKE deck " + (control == 0x0D ? deckA.toString() : deckB.toString()) + " : " + (value ? "true" : "false"));
            }
            engine.brake(control == 0x0D ? deckA : deckB, value ? true : false);
        } else if (superButtonHold == 1) {
            if (value) {
                newValue = HerculesMP3e2.knobIncrement(deck, "pregain", 0, 4, 1, 20, 1);
                engine.setValue(deck, "pregain", newValue);
            }
        } else {
            engine.setValue(deck, "fwd", value ? 1 : 0);
        }
    } else { // Backward
        if (superButtonHold == 2) {
            if (debug) {
                print("**** SPINBACK deck " + (control == 0x0C ? deckA.toString() : deckB.toString()) + " : " + (value ? "true" : "false"));
            }
            engine.spinback(control == 0x0C ? deckA : deckB, value ? true : false);
        } else if (superButtonHold == 1) {
            if (value) {
                newValue = HerculesMP3e2.knobIncrement(deck, "pregain", 0, 4, 1, 20, -1);
                engine.setValue(deck, "pregain", newValue);
            }
        } else {
            engine.setValue(deck, "back", value ? 1 : 0);
        }
    }
};

HerculesMP3e2.pfl = function(midino, control, value, status, group) {
    //normal: pfl / Headphones
    //shift: pfl / Headphones
    //supershift: pfl / Headphones

    var deck = HerculesMP3e2.switchDeck(group);

    if (value) {
        engine.setValue(deck, "pfl", !(engine.getValue(deck, "pfl")));
    }
};

HerculesMP3e2.play = function(midino, control, value, status, group) {
    //normal: play
    //shift: backwards play
    //supershift: replay button

    var deck = HerculesMP3e2.switchDeck(group);

    if (superButtonHold == 2 && value) {
        engine.setValue(deck, "repeat", !(engine.getValue(deck, "repeat")));
    } else if (superButtonHold == 1) {
        engine.setValue(deck, "reverse", value ? 1 : 0);
    } else if (value) {
        engine.setValue(deck, "play", !(engine.getValue(deck, "play")));
    }
};

// Switch-on the pfl / headphone Led if pfl is set
HerculesMP3e2.pflLeds = function(value, group, control) {
    if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
        if (value) {
            midi.sendShortMsg(0x90, 16, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 16, 0x00);
        }
    } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
        if (value) {
            midi.sendShortMsg(0x90, 36, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 36, 0x00);
        }
    }
};

// Blink loop mod 3/4 leds when loop enabled
HerculesMP3e2.loopEnabledLeds = function(value, group, control) {
    if (((group == "[Channel1]") && (deckA == 1)) || ((group == "[Channel3]") && (deckA == 3))) {
        if (value) {
            midi.sendShortMsg(0x90, 51, 0x7F);
            midi.sendShortMsg(0x90, 52, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 51, 0x00);
            midi.sendShortMsg(0x90, 52, 0x00);
        }
    } else if (((group == "[Channel2]") && (deckB == 2)) || ((group == "[Channel4]") && (deckB == 4))) {
        if (value) {
            midi.sendShortMsg(0x90, 71, 0x7F);
            midi.sendShortMsg(0x90, 72, 0x7F);
        } else {
            midi.sendShortMsg(0x90, 71, 0x00);
            midi.sendShortMsg(0x90, 72, 0x00);
        }
    }
};

HerculesMP3e2.selectTrack = function(midino, control, value, status, group) {
    //normal: goes 1 track down
    //shift: goes 1 track up
    if (superButtonHold >= 1 && value) {
        engine.setValue("[Playlist]", "SelectPrevTrack", 1);
    } else if (value) {
        engine.setValue("[Playlist]", "SelectNextTrack", 1);
    }

};

HerculesMP3e2.mic = function(midino, control, value, status, group) {
    //Activates talk with mic.
    engine.setValue("[Microphone]", "talkover", value ? 1 : 0);
};

HerculesMP3e2.volume = function(midino, control, value, status, group) {
    var deck = HerculesMP3e2.switchDeck(group);
    var newValue = script.absoluteLin(value, 0, 1);

    engine.setValue(deck, "volume", newValue);
};

HerculesMP3e2.filterKnob = function(group, control, value) {
    var deck = HerculesMP3e2.switchDeck(group);
    engine.setValue("[EqualizerRack1_" + deck + "_Effect1]", control, script.absoluteNonLin(value, 0, 1, 4));
};

HerculesMP3e2.filterHigh = function(midino, control, value, status, group) {
    HerculesMP3e2.filterKnob(group, "parameter3", value);
};

HerculesMP3e2.filterMid = function(midino, control, value, status, group) {
    HerculesMP3e2.filterKnob(group, "parameter2", value);
};

HerculesMP3e2.filterLow = function(midino, control, value, status, group) {
    HerculesMP3e2.filterKnob(group, "parameter1", value);
};
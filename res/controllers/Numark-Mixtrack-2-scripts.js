// Based on Numark Mixtrack Mapping Script Functions
// 1/11/2010 - v0.1 - Matteo <matteo@magm3.com>
//
// 5/18/2011 - Changed by James Ralston
// 05/26/2012 to 06/27/2012 - Changed by Darío José Freije <dario2004@gmail.com>
//
// Prepared by Thomas Preston for MixTrack Pro II
// Completed by Armen Rizal to work as close as in the manual.
//
//
// Bug Fixes:
// - When top row of pads flash, it's not maintaining original state of the leds.
// - KeyLock is not turning back to original state after scratching. Solution: always turn KeyLock ON by default.
//
// Known Issue:
// - Cue Point Led address is missing
// - ToggleSelectedSidebarItem control is not working
// - After activating brake, Play button is not working

// 10/26/2016 - Changed by Shaun O'Neill
//              Updated the flanger effects rack. Removed deprecated XML controls and added new JS functions to modify the FX knobs
// 10/10/2017 - Changed by Shaun O'Neill
//              Added super button control via shift + fx1 knob.
//              Low frequency filter now doubles as filter effect control via shift key.
// 03/02/2018 - Changed by ninomp
//              Adapt the mapping for Mixxx 2.1


function NumarkMixTrackII() {}

NumarkMixTrackII.init = function(id) {   // called when the MIDI device is opened & set up
    NumarkMixTrackII.id = id;    // Store the ID of this device for later use

    // [deck 1, deck 2]
    NumarkMixTrackII.directory_mode = false;
    NumarkMixTrackII.scratch_mode = [false, false];
    NumarkMixTrackII.touch = [false, false];
    NumarkMixTrackII.scratch_timer = [-1, -1];

    NumarkMixTrackII.shift_is_pressed = [false, false];

    NumarkMixTrackII.cue_delete_mode = [false, false];
    NumarkMixTrackII.pitch_slider_ranges = [0.04, 0.08, 0.16, 0.50];
    NumarkMixTrackII.pitch_slider_range_index = 0;

    // LED addresses
    NumarkMixTrackII.leds = [
        // Common
        {
            "directory": 0x34,
            "file": 0x4B
        },
        // Deck 1
        {
            //"rate": 0x28,
            "scratch_mode": 0x48,
            "loop_in": 0x53,
            "loop_out": 0x54,
            "reloop": 0x55,
            "loop_halve": 0x63,
            "sample_1": 0x65,
            "sample_2": 0x66,
            "sample_3": 0x67,
            "sample_4": 0x68,
            "hotcue_1": 0x6D,
            "hotcue_2": 0x6E,
            "hotcue_3": 0x6F,
            "hotcue_delete": 0x70,
            "fx1": 0x59,
            "fx2": 0x5A,
            "fx3": 0x5B,
            "tap": 0x5C,
            "sync": 0x40,
            "cue": 0x33,
            "play_pause": 0x3B,
            "stutter": 0x4A
        },
        // Deck 2
        {
            //"rate": 0x29,
            "scratch_mode": 0x50,
            "loop_in": 0x56,
            "loop_out": 0x57,
            "reloop": 0x58,
            "loop_halve": 0x64,
            "sample_1": 0x69,
            "sample_2": 0x6A,
            "sample_3": 0x6B,
            "sample_4": 0x6C,
            "hotcue_1": 0x71,
            "hotcue_2": 0x72,
            "hotcue_3": 0x73,
            "hotcue_delete": 0x74,
            "fx1": 0x5D,
            "fx2": 0x5E,
            "fx3": 0x5F,
            "tap": 0x60,
            "sync": 0x47,
            "cue": 0x3C,
            "play_pause": 0x42,
            "stutter": 0x4C
         }
    ];

    NumarkMixTrackII.turnOffAllLeds();

    if (engine.getValue("[App]", "num_samplers") < 8) {
        engine.setValue("[App]", "num_samplers", 8);
    }

    NumarkMixTrackII.updateDirectoryAndFileLeds();
}

NumarkMixTrackII.shutdown = function(id) {   // called when the MIDI device is closed
    NumarkMixTrackII.turnOffAllLeds();
}

NumarkMixTrackII.turnOffAllLeds = function() {
    // Turn off all the leds
    for (var deck_index in NumarkMixTrackII.leds) {
        for (var led in NumarkMixTrackII.leds[deck_index]) {
            NumarkMixTrackII.setLED(NumarkMixTrackII.leds[deck_index][led], false);
        }
    }
}

NumarkMixTrackII.updateDirectoryAndFileLeds = function() {
    NumarkMixTrackII.setLED(NumarkMixTrackII.leds[0]["directory"], NumarkMixTrackII.directory_mode);
    NumarkMixTrackII.setLED(NumarkMixTrackII.leds[0]["file"], !NumarkMixTrackII.directory_mode);
}

NumarkMixTrackII.setLED = function(control, status) {
    midi.sendShortMsg(0x90, control, status ? 0x64 : 0x00);
}

NumarkMixTrackII.groupToDeck = function(group) {
    var matches = group.match(/^\[Channel(\d+)\]$/);
    if (matches == null) {
        return -1;
    } else {
        return matches[1];
    }
}

NumarkMixTrackII.selectKnob = function(channel, control, value, status, group) {
    if (value > 63) {
        value = value - 128;
    }

    if (NumarkMixTrackII.directory_mode) {
        if (value > 0) {
            engine.setValue(group, "SelectNextPlaylist", 1);
        } else {
            engine.setValue(group, "SelectPrevPlaylist", 1);
        }
    } else {
        engine.setValue(group, "SelectTrackKnob", value);
    }
}

NumarkMixTrackII.toggleDirectoryMode = function(channel, control, value, status, group) {
    // Toggle setting and light
    if (value) {
        NumarkMixTrackII.directory_mode = !NumarkMixTrackII.directory_mode;
        NumarkMixTrackII.updateDirectoryAndFileLeds();
    }
}

NumarkMixTrackII.jogWheel = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);

    //if (!NumarkMixTrackPro.touch[deck - 1] && !engine.getValue(group, "play")) return;

    var adjustedJog = parseFloat(value);
    var posNeg = 1;
    if (adjustedJog > 63) { // Counter-clockwise
        posNeg = -1;
        adjustedJog = value - 128;
    }

    if (engine.getValue(group, "play")) {
        if (NumarkMixTrackII.scratch_mode[deck - 1] && posNeg == -1 && !NumarkMixTrackII.touch[deck - 1]) {
            if (NumarkMixTrackII.scratch_timer[deck - 1] != -1) {
                engine.stopTimer(NumarkMixTrackII.scratch_timer[deck - 1]);
            }
            NumarkMixTrackII.scratch_timer[deck - 1] = engine.beginTimer(20, () => NumarkMixTrackII.jogWheelStopScratch(deck), true);
        }
    } else {
        if (!NumarkMixTrackII.touch[deck - 1]) {
            if (NumarkMixTrackII.scratch_timer[deck - 1] != -1) {
                engine.stopTimer(NumarkMixTrackII.scratch_timer[deck - 1]);
            }
            NumarkMixTrackII.scratch_timer[deck - 1] = engine.beginTimer(20, () => NumarkMixTrackII.jogWheelStopScratch(deck), true);
        }
    }

    engine.scratchTick(deck, adjustedJog);

    if (engine.getValue(group,"play")) {
        var gammaInputRange = 13;  // Max jog speed
        var maxOutFraction = 0.8;  // Where on the curve it should peak; 0.5 is half-way
        var sensitivity = 0.5;     // Adjustment gamma
        var gammaOutputRange = 2;  // Max rate change

        adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
        engine.setValue(group, "jog", adjustedJog);
    }
}

NumarkMixTrackII.jogWheelStopScratch = function(deck) {
    NumarkMixTrackII.scratch_timer[deck - 1] = -1;
    engine.scratchDisable(deck);
}

NumarkMixTrackII.wheelTouch = function(channel, control, value, status, group){
    var deck = NumarkMixTrackII.groupToDeck(group);

    if (!value) {
        NumarkMixTrackII.touch[deck - 1] = false;

        // paro el timer (si no existe da error mmmm) y arranco un nuevo timer.
        // Si en 20 milisegundos no se mueve el plato, desactiva el scratch

        if (NumarkMixTrackII.scratch_timer[deck - 1] != -1) {
            engine.stopTimer(NumarkMixTrackII.scratch_timer[deck - 1]);
        }

        NumarkMixTrackII.scratch_timer[deck - 1] = engine.beginTimer(20, () => NumarkMixTrackII.jogWheelStopScratch(deck), true);
    } else {
        if (!NumarkMixTrackII.scratch_mode[deck - 1] && engine.getValue(group, "play")) {
            return;
        }

        if (NumarkMixTrackII.scratch_timer[deck - 1] != -1) {
            engine.stopTimer(NumarkMixTrackII.scratch_timer[deck - 1]);
        }

        // change the 600 value for sensibility
        engine.scratchEnable(deck, 600, 33 + 1 / 3, 1.0 / 8, (1.0 / 8) / 32);

        NumarkMixTrackII.touch[deck - 1] = true;
    }
}

NumarkMixTrackII.toggleScratchMode = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = NumarkMixTrackII.groupToDeck(group);

    // Toggle setting and light
    NumarkMixTrackII.scratch_mode[deck - 1] = !NumarkMixTrackII.scratch_mode[deck - 1];
    NumarkMixTrackII.setLED(NumarkMixTrackII.leds[deck]["scratch_mode"], NumarkMixTrackII.scratch_mode[deck - 1]);
}

/* Shift key pressed/unpressed - toggle shift status in controller object
 * so that other buttons can detect if shift button is currently held down
 */
NumarkMixTrackII.shift = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);
    NumarkMixTrackII.shift_is_pressed[deck - 1] = value == 0x7f ? true : false;
}

/* if shift is held down: toggle keylock
 * else: temporarily bend the pitch down
 */
NumarkMixTrackII.pitch_bend_down_or_keylock = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);
    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        // toggle keylock (only on press down)
        if (value > 0) {
            var current_keylock_value = engine.getValue(group, 'keylock');
            engine.setValue(group, 'keylock', !current_keylock_value);
        }
    } else {
        // temp pitch down
        engine.setValue(group, 'rate_temp_down', value == 0 ? 0 : 1);
    }
}

NumarkMixTrackII.pitch_bend_up_or_range = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);
    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        // cycle slider range
        if (value > 0) {
            var psri = NumarkMixTrackII.pitch_slider_range_index;
            var psr = NumarkMixTrackII.pitch_slider_ranges;
            NumarkMixTrackII.pitch_slider_range_index = (psri + 1) % psr.length;
            //print("setting rate to " + psr[psri]);
            engine.setValue(group, 'rateRange', psr[psri]);
        }
    } else {
        // temp pitch up
        engine.setValue(group, 'rate_temp_up', value > 0 ? 1 : 0);
    }
}

/* All hotcue buttons come here, enable/disable hotcues 1 to 3, toggle delete
 * with the fourth button.
 * LED comes on when there is a hotcue set for that pad.
 * It would be nice if they flashed when the delete button was turned on.
 */
NumarkMixTrackII.hotcue = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);
    var cue_midi_controls = [[0x6D, 0x6E, 0x6F], [0x71, 0x72, 0x73]];
    var cue_num = cue_midi_controls[deck - 1].indexOf(control) + 1;

    if (value && NumarkMixTrackII.cue_delete_mode[deck - 1]) {
        // clear the cue and exit delete mode
        engine.setValue(group, 'hotcue_' + cue_num + '_clear', value);
        NumarkMixTrackII.cue_delete_mode[deck - 1] = false;
        NumarkMixTrackII.setLED(NumarkMixTrackII.leds[deck]["hotcue_delete"], false);
    } else if (cue_num >= 1) {
        engine.setValue(group, 'hotcue_' + cue_num + '_activate', value);
    }
}

NumarkMixTrackII.toggleDeleteHotcueMode = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);

    if (value) {
        // toggle cue delete mode and its' LED
        NumarkMixTrackII.cue_delete_mode[deck - 1] = !NumarkMixTrackII.cue_delete_mode[deck - 1];
        NumarkMixTrackII.setLED(NumarkMixTrackII.leds[deck]["hotcue_delete"],
                                NumarkMixTrackII.cue_delete_mode[deck - 1]);
    }
}

/* loop_halve unless shift_is_pressed, then double loop */
NumarkMixTrackII.loop_halve = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = NumarkMixTrackII.groupToDeck(group);
    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        engine.setValue(group, 'loop_double', 1);
    } else {
        engine.setValue(group, 'loop_halve', 1);
    }
}

/* fx enable/disable unless shift_is_pressed, then auto-loop */
NumarkMixTrackII.fx1_or_auto1 = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = NumarkMixTrackII.groupToDeck(group);
    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        engine.setValue(group, 'beatloop', 1);
    } else {
        var fxGroup = null;
        var c = "enabled";

        if (control == 0x59) {
            fxGroup = "[EffectRack1_EffectUnit1_Effect1]";
        } else if (control == 0x5D) {
            fxGroup = "[EffectRack1_EffectUnit2_Effect1]";
        }

        var paramVal = !engine.getValue(fxGroup, c);
        engine.setValue(fxGroup, c, paramVal);
    }
}

NumarkMixTrackII.fx2_or_auto2 = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = NumarkMixTrackII.groupToDeck(group);
    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        engine.setValue(group, 'beatloop', 2);
    } else {
        var c = "enabled";
        if (control == 0x5A) {
            group = "[EffectRack1_EffectUnit1_Effect2]";
        } else if (control == 0x5E) {
            group = "[EffectRack1_EffectUnit2_Effect2]";
        }
        var paramVal = !engine.getValue(group, c);
        engine.setValue(group, c, paramVal);
    }
}

NumarkMixTrackII.fx3_or_auto4 = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = NumarkMixTrackII.groupToDeck(group);
    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        engine.setValue(group, 'beatloop', 4);
    } else {
        var c = "enabled";
        if (control == 0x5B) {
            group = "[EffectRack1_EffectUnit1_Effect3]";
        } else if (control == 0x5F) {
            group = "[EffectRack1_EffectUnit2_Effect3]";
        }
        var paramVal = !engine.getValue(group, c);
        engine.setValue(group, c, paramVal);
    }
}

NumarkMixTrackII.tap_or_auto16 = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);
    if (value > 0 && NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        engine.setValue(group, 'beatloop', 16);
    } else {
        engine.setValue(group, 'bpm_tap', value);
    }
}

/* effect parameter unless shift_is_pressed, then effect select */
NumarkMixTrackII.fxKnobs = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);

    var fxGroup = null;
    if (control == 0x1B) {
        fxGroup = "[EffectRack1_EffectUnit1_Effect1]";
    } else if (control == 0x1C) {
        fxGroup = "[EffectRack1_EffectUnit1_Effect2]";
    } else if (control == 0x1D) {
        fxGroup = "[EffectRack1_EffectUnit1_Effect3]";
    } else if (control == 0x1E) {
        fxGroup = "[EffectRack1_EffectUnit2_Effect1]";
    } else if (control == 0x1F) {
        fxGroup = "[EffectRack1_EffectUnit2_Effect2]";
    } else if (control == 0x20) {
        fxGroup = "[EffectRack1_EffectUnit2_Effect3]";
    }

    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        if (value == 1) {
            engine.setValue(fxGroup, 'effect_selector', +1);
        } else {
            engine.setValue(fxGroup, 'effect_selector', -1);
        }
    } else {
        var c = "meta";
        var paramVal = engine.getParameter(fxGroup, c);

        if (value == 1) {
            paramVal = paramVal + 0.05;
        } else {
            paramVal = paramVal - 0.05;
        }

        engine.setParameter(fxGroup, c, paramVal);
    }
}

/* fx dry/wet unless shift_is_pressed, then quick effect */
NumarkMixTrackII.beatsKnob = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);

    var fxGroup = null;
    var c = null;

    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        fxGroup = "[QuickEffectRack1_" + group + "]";
        c = "super1";
    } else {
        fxGroup = "[EffectRack1_EffectUnit" + deck + "]";
        c = "mix";
    }

    var paramVal = engine.getParameter(fxGroup, c);
    if (value == 1) {
        paramVal = paramVal + 0.05;
    } else {
        paramVal = paramVal - 0.05;
    }
    engine.setParameter(fxGroup, c, paramVal);
}

/* cue_default unless shift_is_pressed, then cue_gotoandstop */
NumarkMixTrackII.cue = function(channel, control, value, status, group) {
    var deck = NumarkMixTrackII.groupToDeck(group);

    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        engine.setValue(group, 'cue_gotoandstop', value);
    } else {
        engine.setValue(group, 'cue_default', value);
    }
}

/* play/pause unless shift_is_pressed, then softStart/brake */
NumarkMixTrackII.play = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = NumarkMixTrackII.groupToDeck(group);
    var isPlaying = engine.getValue(group, "play");

    if (NumarkMixTrackII.shift_is_pressed[deck - 1]) {
        if (isPlaying) {
            engine.brake(deck, true);
        } else {
            engine.softStart(deck, true);
        }
    } else {
        if (isPlaying) {
            engine.setValue(group, 'play', 0);
        } else {
            engine.setValue(group, 'play', 1);
        }
    }
}

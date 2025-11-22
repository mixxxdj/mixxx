/*******************************************************************
 * Soundless studio.
 * Author: Ozzy Chiu (ozzy.chc@gmail.com)
 * Version: 2020-01-17
 *
 * Variables on joyMIDI.userSetting can be modified by users to suit their preferences
 *******************************************************************/
var joyMIDI = {

    // User Setting
    "userSetting": {
        // Enable the knob(button+jog) function
        "knobEnable": true,
        // Enable the beatloop with roll(1) function otherwise toggle(0)
        "beatLoopRollEnable": true,
        // Enable the samplers played from cue(1) otherwise from track start(0)
        "samplerCueModeEnable": true,
    },

    // Channel1 variable
    "[Channel1]": {
        btnShift: 0,
        btnSync: 0,     syncTimerID: null,     syncLongPress: 0,
        btnScratch: 0,  isScratchPermanent: 0,  scratchTimerID: 0,
        btnKey: 0,
        btnBeatgrid: 0,
        btnVolume: 0,
        btnFilter: {"low": 0, "middle": 0, "high": 0},
        btnFx: {"1": 0, "2": 0},

        connBeatIndicator: {},
    },

    // Channel2 variable
    "[Channel2]": {
        btnShift: 0,
        btnSync: 0,     syncTimerID: null,     syncLongPress: 0,
        btnScratch: 0,  isScratchPermanent: 0,  scratchTimerID: 0,
        btnKey: 0,
        btnBeatgrid: 0,
        btnVolume: 0,
        btnFilter: {"low": 0, "middle": 0, "high": 0},
        btnFx: {"1": 0, "2": 0},

        connBeatIndicator: {},
    },
};

// Constant
var RELEASED = 0,
    PRESSED  = 1,
    ADJUSTED = 2;

//----------------------------
// Constants for scratching
//----------------------------

/* Internal jog-wheel */
var forwardFinetune  = 0;
var backwardFinetune = 0;

var intervalPerRev = 64;
var rpm = 33 + 1 / 3;
var alpha = 1 / 16;
var beta = (1 / 12) / 32;

var scratchDisableTime = 100;
var enableAccVal = true;

/* external jog-wheel */
// var forwardFinetune  = 0;
// var backwardFinetune = 1;

// var intervalPerRev = 1000;
// var rpm = 33 + 1 / 3;
// var alpha = 1.0 / 8;
// var beta = alpha / 32;

// var scratchDisableTime = 150;
// var enableAccVal = false;

//----------------------------
// Common variable
//----------------------------
var joystickDelay = false;
var fsrDelay = false;




//==== Init, Shutdown  ========================================================
joyMIDI.init = function(_id, _debug) {
    joyMIDI["[Channel1]"].connBeatIndicator = engine.makeConnection("[Channel1]", "beat_active", joyMIDI.onBeatIndicator);
    joyMIDI["[Channel2]"].connBeatIndicator = engine.makeConnection("[Channel2]", "beat_active", joyMIDI.onBeatIndicator);
};

joyMIDI.shutdown = function() {
    joyMIDI["[Channel1]"].connBeatIndicator.disconnect();
    joyMIDI["[Channel2]"].connBeatIndicator.disconnect();
};

//==== Callback  ==============================================================
joyMIDI.onBeatIndicator = function(value, group, _control) {
    var fader = engine.getParameter("[Mixer]", "crossfader");
    if (fader < 0.5) {
        // Left
        if (group === "[Channel1]") {
            midi.sendShortMsg(0x91, 0x00, (value) ? 0x7F : 0x00);
        }
    } else {
        // Right
        if (group === "[Channel2]") {
            midi.sendShortMsg(0x92, 0x00, (value) ? 0x7F : 0x00);
        }
    }
};

//==== Button  ================================================================
joyMIDI.shiftButton = function(channel, control, value, status, group) {
    joyMIDI[group].btnShift = (value > 0) ? PRESSED : RELEASED;
};

joyMIDI.syncButton = function(channel, control, value, status, group) {
    if (!joyMIDI.userSetting.knobEnable) {
        // knob disabled
        if (value > 0) {
            // press
            if (!joyMIDI[group].btnShift) {
                if (engine.getValue(group, "sync_enabled") > 0) {
                    engine.setValue(group, "sync_enabled", 0);
                } else {
                    engine.setValue(group, "sync_enabled", 1);
                    if (joyMIDI[group].syncTimerID !== null) {
                        engine.stopTimer(joyMIDI[group].syncTimerID);
                    }
                    joyMIDI[group].syncLongPress = 0;
                    joyMIDI[group].syncTimerID =
                        engine.beginTimer(1000, () => { joyMIDI.syncTimerHandler(group); }, true);
                }
            } else {
                engine.setValue(group, "rate", 0);
            }
        } else {
            // release
            if (!joyMIDI[group].btnShift) {
                if (joyMIDI[group].syncTimerID !== null) {
                    if (joyMIDI[group].syncLongPress === 0 && engine.getValue(group, "sync_enabled") === 1) {
                        engine.stopTimer(joyMIDI[group].syncTimerID);
                        engine.setValue(group, "sync_enabled", 0);
                    }
                    joyMIDI[group].syncTimerID = null;
                }
            }
        }
    } else {
        // knob enabled
        if (value > 0) {
            // press
            joyMIDI[group].btnSync = PRESSED;
        } else {
            // release
            if (joyMIDI[group].btnSync !== ADJUSTED) {
                if (!joyMIDI[group].btnShift) {
                    engine.setValue(group, "beatsync", 1);
                } else {
                    engine.setValue(group, "rate", 0);
                }
            }
            joyMIDI[group].btnSync = RELEASED;
        }
    }
};

joyMIDI.syncTimerHandler = function(group) {
    joyMIDI[group].syncLongPress = 1;
};

joyMIDI.cueButton = function(channel, control, value, status, group) {
    var sft = joyMIDI[group].btnShift;
    var cmd = sft ? "start" : "cue_default";
    var val = (value > 0) ? 1 : 0;

    engine.setValue(group, cmd, val);
};

joyMIDI.playButton = function(channel, control, value, status, group) {
    var sft = joyMIDI[group].btnShift;
    var cmd = sft ? "reverse" : "play";

    if (value > 0) {
        script.toggleControl(group, cmd);
    }
};

joyMIDI.scratchButton = function(channel, control, value, status, group) {
    if (value > 0) {
        joyMIDI[group].isScratchPermanent = !joyMIDI[group].isScratchPermanent;
    }
};

joyMIDI.keyButton = function(channel, control, value, status, group) {
    if (!joyMIDI.userSetting.knobEnable) {
        // knob disabled
        if (value > 0) {
            // press
            if (!joyMIDI[group].btnShift) {
                engine.setValue(group, "sync_key", 1);
            } else {
                engine.setValue(group, "reset_key", 1);
            }
        } else {
            // release
        }
    } else {
        // knob enabled
        if (value > 0) {
            // press
            joyMIDI[group].btnKey = PRESSED;
        } else {
            // release
            if (joyMIDI[group].btnKey !== ADJUSTED) {
                if (joyMIDI[group].btnShift === RELEASED) {
                    engine.setValue(group, "sync_key", 1);
                } else {
                    engine.setValue(group, "reset_key", 1);
                }
            }
            joyMIDI[group].btnKey = RELEASED;
        }
    }
};

joyMIDI.beatgridButton = function(channel, control, value, status, group) {
    if (!joyMIDI.userSetting.knobEnable) {
        // knob disabled
        if (value > 0) {
            // press
            if (!joyMIDI[group].btnShift) {
                engine.setValue(group, "beats_translate_curpos", 1);
                engine.setValue(group, "beats_translate_curpos", 0);
            } else {
                engine.setValue(group, "beats_translate_match_alignment", 1);
                engine.setValue(group, "beats_translate_match_alignment", 0);
            }
        }
    } else {
        // knob enabled
        if (value > 0) {
            // press
            joyMIDI[group].btnBeatgrid = PRESSED;
        } else {
            // release
            if (joyMIDI[group].btnBeatgrid !== ADJUSTED) {
                if (joyMIDI[group].btnShift === RELEASED) {
                    engine.setValue(group, "beats_translate_curpos", 1);
                    engine.setValue(group, "beats_translate_curpos", 0);
                } else {
                    engine.setValue(group, "beats_translate_match_alignment", 1);
                    engine.setValue(group, "beats_translate_match_alignment", 0);
                }
            }
            joyMIDI[group].btnBeatgrid = RELEASED;
        }
    }
};

joyMIDI.volumeButton = function(channel, control, value, status, group) {
    if (!joyMIDI.userSetting.knobEnable) {
        // knob disabled
        if (value > 0) {
            // press
            if (!joyMIDI[group].btnShift) {
                engine.setValue(group, "volume", 1.0);
            }
        }
    } else {
        // knob enabled
        if (value > 0) {
            // press
            joyMIDI[group].btnVolume = PRESSED;
        } else {
            // release
            if (joyMIDI[group].btnVolume !== ADJUSTED) {
                if (joyMIDI[group].btnShift === RELEASED) {
                    engine.setValue(group, "volume", 1.0);
                }
            }
            joyMIDI[group].btnVolume = RELEASED;
        }
    }
};

joyMIDI.filterLowButton = function(channel, control, value, status, group) {
    joyMIDI.filterButton(channel, control, value, status, group,
        "[EqualizerRack1_" + group + "_Effect1]", "button_parameter1", "low");
};

joyMIDI.filterMiddleButton = function(channel, control, value, status, group) {
    joyMIDI.filterButton(channel, control, value, status, group,
        "[EqualizerRack1_" + group + "_Effect1]", "button_parameter2", "middle");
};

joyMIDI.filterHighButton = function(channel, control, value, status, group) {
    joyMIDI.filterButton(channel, control, value, status, group,
        "[EqualizerRack1_" + group + "_Effect1]", "button_parameter3", "high");
};

joyMIDI.filterButton = function(channel, control, value, status, group, group2, param, select) {
    if (!joyMIDI.userSetting.knobEnable) {
        // knob disabled
        if (value > 0) {
            // press
            if (!joyMIDI[group].btnShift) {
                script.toggleControl(group2, param);
            }
        }
    } else {
        // knob enabled
        if (value > 0) {
            // press
            joyMIDI[group].btnFilter[select] = PRESSED;
        } else {
            // release
            if (joyMIDI[group].btnFilter[select] !== ADJUSTED) {
                if (joyMIDI[group].btnShift === RELEASED) {
                    script.toggleControl(group2, param);
                }
            }
            joyMIDI[group].btnFilter[select] = RELEASED;
        }
    }
};

joyMIDI.hotcueButton = function(channel, control, value, status, group) {
    var num = control - ((group === "[Channel1]") ? 0x3B : 0x3F);
    var sft = joyMIDI[group].btnShift;
    var cmd = "hotcue_" + num + (sft ? "_clear" : "_activate");
    var val = (value > 0) ? 1 : 0;

    engine.setValue(group, cmd, val);
};

joyMIDI.reloopToggleButton = function(channel, control, value, status, group) {
    var usr = joyMIDI.userSetting.beatLoopRollEnable;
    var sft = joyMIDI[group].btnShift;
    var cmd = sft ? (usr?"beatlooproll_activate":"beatloop_activate") : "reloop_toggle";
    var val = (value > 0) ? 1 : 0;

    engine.setValue(group, cmd, val);
};

joyMIDI.loopInButton = function(channel, control, value, status, group) {
    var sft = joyMIDI[group].btnShift;
    var cmd = sft ? "loop_halve" : "loop_in";
    var val = (value > 0) ? 1 : 0;

    engine.setValue(group, cmd, val);
};

joyMIDI.loopOutButton = function(channel, control, value, status, group) {
    var sft = joyMIDI[group].btnShift;
    var cmd = sft ? "loop_double" : "loop_out";
    var val = (value > 0) ? 1 : 0;

    engine.setValue(group, cmd, val);
};

joyMIDI.loop0p125Button = function(channel, control, value, status, group) {
    joyMIDI.loopXButton(channel, control, value, status, group,
        ["beatloop_0.125_toggle",       "beatloop_2_toggle",
          "beatlooproll_0.125_activate", "beatlooproll_2_activate"]);
};

joyMIDI.loop0p25Button = function(channel, control, value, status, group) {
    joyMIDI.loopXButton(channel, control, value, status, group,
        ["beatloop_0.25_toggle",       "beatloop_4_toggle",
          "beatlooproll_0.25_activate", "beatlooproll_4_activate"]);
};

joyMIDI.loop0p5Button = function(channel, control, value, status, group) {
    joyMIDI.loopXButton(channel, control, value, status, group,
        ["beatloop_0.5_toggle",       "beatloop_8_toggle",
          "beatlooproll_0.5_activate", "beatlooproll_8_activate"]);
};

joyMIDI.loop1Button = function(channel, control, value, status, group) {
    joyMIDI.loopXButton(channel, control, value, status, group,
        ["beatloop_1_toggle",       "beatloop_16_toggle",
          "beatlooproll_1_activate", "beatlooproll_16_activate"]);
};

joyMIDI.loopXButton = function(channel, control, value, status, group, commands) {
    var usr = joyMIDI.userSetting.beatLoopRollEnable;
    var sft = joyMIDI[group].btnShift;
    var cmd = commands[sft + (usr?2:0)];
    var val = (value > 0) ? 1 : 0;

    engine.setValue(group, cmd, val);
};

joyMIDI.loadButton = function(channel, control, value, status, group) {
    var sft = joyMIDI[group].btnShift;
    var cmd = sft ? "eject" : "LoadSelectedTrack";
    var val = (value > 0) ? 1 : 0;

    engine.setValue(group, cmd, val);
};

joyMIDI.sampler1Button = function(channel, control, value, status, group) {
    joyMIDI.samplerButton(channel, control, value, status, group, 1);
};

joyMIDI.sampler2Button = function(channel, control, value, status, group) {
    joyMIDI.samplerButton(channel, control, value, status, group, 2);
};

joyMIDI.sampler3Button = function(channel, control, value, status, group) {
    joyMIDI.samplerButton(channel, control, value, status, group, 3);
};

joyMIDI.sampler4Button = function(channel, control, value, status, group) {
    joyMIDI.samplerButton(channel, control, value, status, group, 4);
};

joyMIDI.samplerButton = function(channel, control, value, status, group, number) {
    var commands = ["start_play", "start_stop", "cue_gotoandplay", "cue_default"];
    var usr = joyMIDI.userSetting.samplerCueModeEnable;
    var sft = joyMIDI["[Channel"+ ((number > 2) ? 2 : 1) +"]"].btnShift;
    var cmd = commands[sft + (usr?2:0)];
    var val = (value > 0) ? 1 : 0;

    engine.setValue(group, cmd, val);
};

joyMIDI.fx1Button = function(channel, control, value, status, group) {
    joyMIDI.fxButton(channel, control, value, status, group,
        "[EffectRack1_EffectUnit1]", "group_"+group+"_enable", 1);
};

joyMIDI.fx2Button = function(channel, control, value, status, group) {
    joyMIDI.fxButton(channel, control, value, status, group,
        "[EffectRack1_EffectUnit2]", "group_"+group+"_enable", 2);
};

joyMIDI.fxButton = function(channel, control, value, status, group, group2, param, select) {
    if (!joyMIDI.userSetting.knobEnable) {
        // knob disabled
        if (value > 0) {
            // press
            if (!joyMIDI[group].btnShift) {
                script.toggleControl(group2, param);
            }
        }
    } else {
        // knob enabled
        if (value > 0) {
            // press
            joyMIDI[group].btnFx[select] = PRESSED;
        } else {
            // release
            if (joyMIDI[group].btnFx[select] !== ADJUSTED) {
                if (joyMIDI[group].btnShift === RELEASED) {
                    script.toggleControl(group2, param);
                }
            }
            joyMIDI[group].btnFx[select] = RELEASED;
        }
    }
};

//==== Wheel  ============================================================
joyMIDI.wheel = function(channel, control, value, status, group) {
    if        (joyMIDI[group].btnSync >= PRESSED) {
        joyMIDI.wheelSync(channel, control, value, status, group);
        joyMIDI[group].btnSync = ADJUSTED;
    } else if (joyMIDI[group].btnKey >= PRESSED) {
        joyMIDI.wheelKey(channel, control, value, status, group);
        joyMIDI[group].btnKey = ADJUSTED;
    } else if (joyMIDI[group].btnBeatgrid >= PRESSED) {
        joyMIDI.wheelBeatgrid(channel, control, value, status, group);
        joyMIDI[group].btnBeatgrid = ADJUSTED;
    } else if (joyMIDI[group].btnVolume >= PRESSED) {
        joyMIDI.wheelVolume(channel, control, value, status, group);
        joyMIDI[group].btnVolume = ADJUSTED;

    } else if (joyMIDI[group].btnFilter["low"] >= PRESSED) {
        joyMIDI.wheelFilterLow(channel, control, value, status, group);
        joyMIDI[group].btnFilter["low"] = ADJUSTED;
    } else if (joyMIDI[group].btnFilter["middle"] >= PRESSED) {
        joyMIDI.wheelFilterMiddle(channel, control, value, status, group);
        joyMIDI[group].btnFilter["middle"] = ADJUSTED;
    } else if (joyMIDI[group].btnFilter["high"] >= PRESSED) {
        joyMIDI.wheelFilterHigh(channel, control, value, status, group);
        joyMIDI[group].btnFilter["high"] = ADJUSTED;

    } else if (joyMIDI[group].btnFx["1"] >= PRESSED) {
        joyMIDI.wheelFx1(channel, control, value, status, group);
        joyMIDI[group].btnFx["1"] = ADJUSTED;
    } else if (joyMIDI[group].btnFx["2"] >= PRESSED) {
        joyMIDI.wheelFx2(channel, control, value, status, group);
        joyMIDI[group].btnFx["2"] = ADJUSTED;
    } else if (joyMIDI[group].btnFx["3"] >= PRESSED) {
        joyMIDI.wheelFx3(channel, control, value, status, group);
        joyMIDI[group].btnFx["3"] = ADJUSTED;

    } else if (joyMIDI[group].isScratchPermanent) {
        joyMIDI.wheelScratch(channel, control, value, status, group);
    } else {
        joyMIDI.wheelPitchBend(channel, control, value, status, group);
    }
};

joyMIDI.wheelPitchBend = function(channel, control, value, status, group) {
    var newValue = joyMIDI.helperAccel(value - 64);

    engine.setValue(group, "jog", newValue);
    print(group + "jog=" + newValue);
};

joyMIDI.wheelScratch = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var newValue = joyMIDI.helperAccel(value - 64);

    if (newValue > 0) newValue = newValue + forwardFinetune;
    else              newValue = newValue - backwardFinetune;

    if (joyMIDI[group].scratchTimerID) {
        engine.stopTimer(joyMIDI[group].scratchTimerID);
    }

    // if (!engine.isScratching(deck)) {
        engine.scratchEnable(deck, intervalPerRev, rpm, alpha, beta);
    // }

    engine.scratchTick(deck, newValue);

    joyMIDI[group].scratchTimerID =
        engine.beginTimer(scratchDisableTime, () => { joyMIDI.ScratchTimerHandler(deck, group); }, true);
};

joyMIDI.ScratchTimerHandler = function(deck, group) {
    engine.scratchDisable(deck);
    joyMIDI[group].scratchTimerID = 0;
};

joyMIDI.wheelSync = function(channel, control, value, status, group) {
    var delta    = joyMIDI.helperAccel(value - 64) * 0.01;
    var oldValue = engine.getValue(group, "rate");
    var newValue = oldValue + delta;

    newValue = joyMIDI.helperLimit(newValue, 1.0, -1.0);
    engine.setValue(group, "rate", newValue);
    print(group + "rate=" + newValue);
};

joyMIDI.wheelKey = function(channel, control, value, status, group) {
    var delta    = /*joyMIDI.helperAccel*/(value - 64) * 0.2;
    var oldValue = engine.getValue(group, "pitch");
    var newValue = oldValue + delta;

    newValue = joyMIDI.helperLimit(newValue, 6.0, -6.0);
    engine.setValue(group, "pitch", newValue);
    print(group + "pitch=" + newValue);
};

joyMIDI.wheelBeatgrid = function(channel, control, value, status, group) {
    var delta = joyMIDI.helperAccel(value - 64);
    var count = Math.abs(delta);

    for (var i=0; i<count; i++) {
        if (delta > 0) {
            engine.setValue(group, "beats_translate_earlier", 1);
            print(group + "beats_translate_earlier");
        } else {
            engine.setValue(group, "beats_translate_later", 1);
            print(group + "beats_translate_later");
        }
    }
};

joyMIDI.wheelVolume = function(channel, control, value, status, group) {
    var delta    = joyMIDI.helperAccel(value - 64) * 0.01;
    var oldValue = engine.getValue(group, "volume");
    var newValue = oldValue + delta;

    newValue = joyMIDI.helperLimit(newValue, 1.0, 0);
    engine.setValue(group, "volume", newValue);
    print(group + "volume=" + newValue);
};

joyMIDI.wheelFilterLow = function(channel, control, value, status, group) {
    joyMIDI.wheelFilter(channel, control, value, status, group,
        "[EqualizerRack1_" + group + "_Effect1]", "parameter1", "low");
};

joyMIDI.wheelFilterMiddle = function(channel, control, value, status, group) {
    joyMIDI.wheelFilter(channel, control, value, status, group,
        "[EqualizerRack1_" + group + "_Effect1]", "parameter2", "middle");
};

joyMIDI.wheelFilterHigh = function(channel, control, value, status, group) {
    joyMIDI.wheelFilter(channel, control, value, status, group,
        "[EqualizerRack1_" + group + "_Effect1]", "parameter3", "high");
};

joyMIDI.wheelFilter = function(channel, control, value, status, group, group2, param, select) {
    var delta    = joyMIDI.helperAccel(value - 64) * 0.01;
    var oldValue = engine.getParameter(group2, param);
    var newValue = oldValue + delta;

    newValue = joyMIDI.helperLimit(newValue, 1.0, 0);
    engine.setParameter(group2, param, newValue);
    print(group + "EQ_" + select + "=" + newValue);
};

joyMIDI.wheelFx1 = function(channel, control, value, status, group) {
    if (group === "[Channel1]") {
        joyMIDI.wheelFx(channel, control, value, status, group,
            "[EffectRack1_EffectUnit1]", "mix", 1);
    } else {
        joyMIDI.wheelFx(channel, control, value, status, group,
            "[EffectRack1_EffectUnit2]", "mix", 1);
    }
};

joyMIDI.wheelFx2 = function(channel, control, value, status, group) {
    if (group === "[Channel1]") {
        joyMIDI.wheelFx(channel, control, value, status, group,
            "[EffectRack1_EffectUnit1]", "super1", 2);
    } else {
        joyMIDI.wheelFx(channel, control, value, status, group,
            "[EffectRack1_EffectUnit2]", "super1", 2);
    }
};

joyMIDI.wheelFx = function(channel, control, value, status, group, group2, param, _select) {
    var delta    = joyMIDI.helperAccel(value - 64) * 0.01;
    var oldValue = engine.getParameter(group2, param);
    var newValue = oldValue + delta;

    newValue = joyMIDI.helperLimit(newValue, 1.0, 0);
    engine.setParameter(group2, param, newValue);
};


//==== Jojstick ============================================================
joyMIDI.joystick = function(channel, control, _value, _status, _group) {

    if (joystickDelay === false) {
        /* Delay a while */
        joystickDelay = true;
        engine.beginTimer(300, () => { joystickDelay = false; }, true);

        if (!joyMIDI["[Channel1]"].btnShift) {
            /* Joystick only */
            switch (control) {
                case 0x10:  engine.setValue("[Library]", "MoveLeft", true);     break;
                case 0x11:  engine.setValue("[Library]", "MoveRight", true);    break;
                case 0x12:  engine.setValue("[Library]", "MoveDown", true);     break;
                case 0x13:  engine.setValue("[Library]", "MoveUp", true);       break;
            }
        } else {
            /* Joystick + shift */
            switch (control) {
                case 0x10:  /* Do nothing */                                    break;
                case 0x11:  /* Do nothing */                                    break;
                case 0x12:  engine.setValue("[Library]", "ScrollDown", true);   break;
                case 0x13:  engine.setValue("[Library]", "ScrollUp", true);     break;
            }
        }
    }
};


//==== FSR ============================================================
joyMIDI.fsr = function(_channel, _control, _value, _status, _group) {

    if (fsrDelay === false) {
        /* Delay a while */
        fsrDelay = true;
        engine.beginTimer(300, () => { fsrDelay = false; }, true);

        var isShift = joyMIDI["[Channel1]"].btnShift === true || joyMIDI["[Channel2]"].btnShift === true;
        if (! isShift) {
            /* FSR only */
            engine.setValue("[Library]", "MoveFocusForward", true);
        } else {
            /* FSR + shift */
            engine.setValue("[Library]", "MoveFocusBackward", true);
        }
    }
};

//==== Helper =============================================================
joyMIDI.helperLimit = function(input, max, min) {
    input = input > max ? max : input;
    input = input < min ? min : input;
    return input;
};

joyMIDI.helperAccel = function(input) {
    var acc = 0;
    if (enableAccVal === true) {
        acc = Math.pow(Math.abs(input), 2);
        acc = (input > 0) ? acc : -acc;
    } else {
        acc = input;
    }
    return acc;
};

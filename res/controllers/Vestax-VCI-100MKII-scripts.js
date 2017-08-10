// name: Vestax VCI-100MKII
// author: Takeshi Soejima
// description: 2017-8-9
// wiki: <http://www.mixxx.org/wiki/doku.php/vestax_vci-100mkii>

// JSHint Configuration
// global engine
// global midi

var VCI102 = {};

VCI102.deck = ["[Channel1]", "[Channel2]", "[Channel3]", "[Channel4]"];
VCI102.fx12 = ["[EffectRack1_EffectUnit1]", "[EffectRack1_EffectUnit2]"];
VCI102.fx34 = ["[EffectRack1_EffectUnit3]", "[EffectRack1_EffectUnit4]"];
VCI102.fxButton = [VCI102.fx12, VCI102.fx12];
VCI102.lockButton = ["keylock", "keylock"];
VCI102.sizeButton = ["beatloop_size", "beatloop_size"];

VCI102.refreshLED = function(ch) {
    var deck = VCI102.deck[ch];
    VCI102.fxButton[ch % 2].forEach(function(fx) {
        engine.trigger(fx, "group_" + deck + "_enable");
    });
    engine.trigger(deck, VCI102.lockButton[ch % 2]);
    engine.trigger(deck, VCI102.sizeButton[ch % 2]);
};

VCI102.shift = [0, 0];

VCI102.setShift = function(ch, midino, value, status, group) {
    ch = VCI102.deck.indexOf(group);  // override channel by group
    VCI102.shift[ch] = value;
    if (value) {
        VCI102.fxButton[ch] = VCI102.fx34;
        VCI102.lockButton[ch] = "quantize";
        VCI102.sizeButton[ch] = "beatjump_size";
    } else {
        VCI102.fxButton[ch] = VCI102.fx12;
        VCI102.lockButton[ch] = "keylock";
        VCI102.sizeButton[ch] = "beatloop_size";
    }
    VCI102.refreshLED(ch);
    VCI102.refreshLED(ch + 2);
};

VCI102.MoveFocus = function(ch, midino, value, status, group) {
    if (VCI102.shift[0] + VCI102.shift[1]) {
        engine.setValue(group, "ChooseItem", value > 0);
    } else {
        engine.setValue(group, "MoveFocusForward", value > 0);
    }
};

VCI102.selectTimer = 0;

VCI102.selectIter = function(select) {
    if (select) {
        select();
        VCI102.selectTimer = engine.beginTimer(500, function() {
            VCI102.selectTimer = engine.beginTimer(40, select);
        }, true);
    } else {
        engine.stopTimer(VCI102.selectTimer);
    }
};

VCI102.SelectPrevTrack = function(ch, midino, value, status, group) {
    VCI102.selectIter(value && function() {
        engine.setValue(group, "MoveUp", 1);
    });
};

VCI102.SelectNextTrack = function(ch, midino, value, status, group) {
    VCI102.selectIter(value && function() {
        engine.setValue(group, "MoveDown", 1);
    });
};

VCI102.slip = function(value, group, key) {
    // resume after the effect when the track is [re]played
    if (key == "play" ? value : !value && engine.getValue(group, "play")) {
        if (engine.getValue(group, "slip_enabled")) {
            engine.setValue(group, "slip_enabled", 0);
            engine.beginTimer(40, function() {
                engine.setValue(group, "slip_enabled", 1);
            }, true);
        }
    }
};

VCI102.scratchTimer = [0, 0, 0, 0];

VCI102.scratchEnable = function(ch, midino, value, status, group) {
    var deck = ch + 1;
    if (value) {
        if (VCI102.scratchTimer[ch]) {
            engine.stopTimer(VCI102.scratchTimer[ch]);
            VCI102.scratchTimer[ch] = 0;
        } else if (VCI102.shift[ch % 2]) {
            engine.brake(deck, true);
        } else {
            engine.scratchEnable(deck, 2400, 100 / 3, 1 / 8, 1 / 256);
        }
    } else if (engine.isScratching(deck)) {
        VCI102.scratchTimer[ch] = engine.beginTimer(20, function() {
            var vel = Math.abs(engine.getValue(group, "scratch2"));
            if (vel < 1 && (vel < 1e-9 || engine.getValue(group, "play"))) {
                if (VCI102.scratchTimer[ch]) {
                    engine.stopTimer(VCI102.scratchTimer[ch]);
                    VCI102.scratchTimer[ch] = 0;
                    engine.scratchDisable(
                        deck, !engine.getValue(group, "slip_enabled"));
                    VCI102.slip(value, group);
                }
            }
        });
    } else {
        // terminate brake even without shift if not scratching
        engine.brake(deck, false);
        VCI102.slip(value, group);
    }
};

VCI102.jog = function(ch, midino, value, status, group) {
    var deck = ch + 1;
    value -= 64;
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, value);
    } else if (!engine.getValue(group, "slip_enabled")) {
        engine.setValue(group, "jog", value * value * value / 1024);
    }
};

VCI102.rateEnable = [true, true, true, true];
VCI102.rateValueMSB = [64, 64, 64, 64];  // defaults are at center

VCI102.rateMSB = function(ch, midino, value, status, group) {
    // unlock rate control if the change is not caused by a mechanical drift
    if (!VCI102.rateEnable[ch]) {
        if (Math.abs(value - VCI102.rateValueMSB[ch]) > 1) {
            VCI102.rateEnable[ch] = true;
        } else {
            return;
        }
    }
    VCI102.rateValueMSB[ch] = value;
};

VCI102.rateQuantizedMSB = function(ch, midino, value, status, group) {
    // lock rate control against a mechanical drift
    if (VCI102.rateEnable[ch]) {
        VCI102.rateEnable[ch] = false;
    }
    VCI102.rateValueMSB[ch] = value;
};

VCI102.rate = function(ch, lsb) {
    return ((64 - VCI102.rateValueMSB[ch]) * 128 - lsb) / 8192;
};

VCI102.rateLSB = function(ch, midino, value, status, group) {
    // inhibit rate control after the change by quantized bpm until unlocked
    if (VCI102.rateEnable[ch]) {
        engine.setValue(group, "rate", VCI102.rate(ch, value));
    }
};

VCI102.rateQuantizedLSB = function(ch, midino, value, status, group) {
    // not change "bpm" direct but by "rate" to go through soft takeover
    var bpm = engine.getValue(group, "file_bpm");
    var range = engine.getValue(group, "rateRange");
    engine.setValue(group, "rate", (Math.round(
        (VCI102.rate(ch, value) * range + 1) * bpm) / bpm - 1) / range);
};

VCI102.pitch = function(ch, midino, value, status, group) {
    value = (value - 64) / 21;  // up and down to 3 semitones
    if (engine.getValue(group, "keylock")) {
        value = Math.round(value);  // discrete change on keylock
    }
    engine.setValue(group, "pitch_adjust", value);
};

VCI102.slot = function(group, n) {
    // convert effectUnit group to effectSlot group
    return group.slice(0, -1) + "_Effect" + n + "]";
};

VCI102.parameter = function(ch, value, group, key) {
    if (VCI102.shift[ch % 2]) {
        // link to meta, inverse variants -> none -> direct variants
        if (engine.getValue(group, key + "_loaded")) {
            value = Math.round(value / 16) - 4;
            engine.setValue(group, key + "_link_type", Math.abs(value));
            engine.setValue(group, key + "_link_inverse", value < 0);
        }
    } else {
        engine.setParameter(group, key, value < 127 ? value / 128 : 1);
    }
};

VCI102.meta = function(value, group, key) {
    engine.setValue(group, key, value < 127 ? value / 128 : 1);
};

VCI102.parameter1 = function(ch, midino, value, status, group) {
    var n = engine.getValue(group, "focused_effect");
    if (n) {
        VCI102.parameter(ch, value, VCI102.slot(group, n), "parameter1");
    } else {
        VCI102.meta(value, VCI102.slot(group, 1), "meta");
    }
};

VCI102.parameter2 = function(ch, midino, value, status, group) {
    var n = engine.getValue(group, "focused_effect");
    if (n) {
        VCI102.parameter(ch, value, VCI102.slot(group, n), "parameter2");
    } else {
        VCI102.meta(value, VCI102.slot(group, 2), "meta");
    }
};

VCI102.parameter3 = function(ch, midino, value, status, group) {
    var n = engine.getValue(group, "focused_effect");
    if (n) {
        VCI102.parameter(ch, value, VCI102.slot(group, n), "parameter3");
    } else {
        VCI102.meta(value, VCI102.slot(group, 3), "meta");
    }
};

VCI102.parameter4 = function(ch, midino, value, status, group) {
    var n = engine.getValue(group, "focused_effect");
    if (n) {
        VCI102.parameter(ch, value, VCI102.slot(group, n), "parameter4");
    } else {
        VCI102.meta(value, group, "super1");
    }
};

VCI102.prev_effect = function(ch, midino, value, status, group) {
    var n = engine.getValue(group, "focused_effect");
    if (n) {
        engine.setValue(VCI102.slot(group, n), "prev_effect", value > 0);
    } else {
        engine.setValue(group, "prev_chain", value > 0);
    }
};

VCI102.next_effect = function(ch, midino, value, status, group) {
    var n = engine.getValue(group, "focused_effect");
    if (n) {
        engine.setValue(VCI102.slot(group, n), "next_effect", value > 0);
    } else {
        engine.setValue(group, "next_chain", value > 0);
    }
};

[0, 1, 2, 3].forEach(function(n) {
    VCI102["focusEffect" + n] = function(ch, midino, value, status, group) {
        if (value) {
            engine.setValue(group, "focused_effect", n);
        }
    };
});

VCI102.loop = function(ch, midino, value, status, group) {
    if (value) {
        if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "reloop_toggle", 1);
        } else {
            engine.setValue(group, "beatloop_activate", 1);
            engine.setValue(group, "beatloop_activate", 0);
        }
    }
};

VCI102.reloop = function(ch, midino, value, status, group) {
    if (value) {
        if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "loop_out", 1);
            engine.setValue(group, "loop_out", 0);
        } else {
            engine.setValue(group, "reloop_toggle", 1);
        }
    }
};

VCI102.loop_scale = function(ch, group, scale) {
    if (VCI102.shift[1 - ch % 2]) {
        // scale beatjump_size of the other Deck if shift of it
        group = VCI102.Deck[1 - ch % 2];
        engine.setValue(group, "beatjump_size",
                        engine.getValue(group, "beatjump_size") * scale);
    } else if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_scale", scale);
    } else {
        engine.setValue(group, "beatloop_size",
                        engine.getValue(group, "beatloop_size") * scale);
    }
};

VCI102.loop_halve = function(ch, midino, value, status, group) {
    if (value) {
        VCI102.loop_scale(ch, group, 1 / 2);
    }
};

VCI102.loop_double = function(ch, midino, value, status, group) {
    if (value) {
        VCI102.loop_scale(ch, group, 2);
    }
};

VCI102.Deck = ["[Channel1]", "[Channel2]"];

VCI102.setDeck = function(ch, midino, value, status, group) {
    if (value) {
        VCI102.Deck[ch] = group;
    }
};

VCI102.solo = function(ch, midino, value, status, group) {
    if (value) {
        VCI102.deck.forEach(function(deck, i) {
            engine.setValue(deck, "pfl", i == ch);
        });
    }
};

VCI102.shutdown = function() {
    var i, j;
    for (i = 0x90; i < 0x94; i++) {
        for (j = 0x22; j < 0x3B; j++) {
            midi.sendShortMsg(i, j, 0);
        }
    }
};

VCI102.init = function(id, debug) {
    var LEDfx = [0x3A, 0x38];
    var LEDlock = 0x2D;
    var LEDsize = [
        [0x33, 0x29],
        [0x29, 0x26]
    ];

    function headMix(value, group, key) {
        if (value) {
            if (engine.getValue("[Master]", "headMix") == 1) {
                engine.setValue("[Master]", "headMix", -1);
            }
        } else if (engine.getValue("[Master]", "headMix") == -1) {
            if (VCI102.deck.every(function(deck) {
                return !engine.getValue(deck, "pfl");
            })) {
                engine.setValue("[Master]", "headMix", 1);
            }
        }
    }

    function makeLEDfx(ch, i) {
        return function(value, group, key) {
            if (group == VCI102.fxButton[ch % 2][i]) {
                midi.sendShortMsg(0x90 + ch, LEDfx[i], value * 127);
            }
        };
    }

    function makeLEDlock(ch) {
        return function(value, group, key) {
            if (key == VCI102.lockButton[ch % 2]) {
                midi.sendShortMsg(0x90 + ch, LEDlock, value * 127);
            }
        };
    }

    function makeLEDsize(ch) {
        return function(value, group, key) {
            if (key == VCI102.sizeButton[ch % 2]) {
                [value < 4 || value > 64, value > 4 || value < 1 / 4
                ].forEach(function(on, i) {
                    midi.sendShortMsg(0x90 + ch, LEDsize[ch % 2][i], on * 127);
                });
            }
        };
    }

    VCI102.deck.forEach(function(deck, i) {
        var enable = "group_" + deck + "_enable";
        var lock = makeLEDlock(i);
        var size = makeLEDsize(i);
        [makeLEDfx(i, 0), makeLEDfx(i, 1)].forEach(function(led, j) {
            engine.connectControl(VCI102.fx12[j], enable, led);
            engine.connectControl(VCI102.fx34[j], enable, led);
        });
        ["keylock", "quantize"].forEach(function(key) {
            engine.connectControl(deck, key, lock);
        });
        ["beatloop_size", "beatjump_size"].forEach(function(key) {
            engine.connectControl(deck, key, size);
        });
        ["loop_enabled", "play", "reverse"].forEach(function(key) {
            engine.connectControl(deck, key, VCI102.slip);
        });
        engine.connectControl(deck, "pfl", headMix);
        engine.softTakeover(deck, "rate", true);
        engine.softTakeover(deck, "pitch_adjust", true);
        VCI102.refreshLED(i);
    });
    VCI102.fx12.concat(VCI102.fx34).forEach(function(fx) {
        engine.setValue(fx, "show_focus", 1);
    });
};

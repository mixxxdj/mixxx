const DDJFLX2 = {
    fourDeckMode: false,
    vDeckNo: [0, 1, 2],
    vDeck: {},
    shiftPressed: {left: false, right: false},
    jogCounter: 0,
};

DDJFLX2.init = function() {
    for (let i = 1; i <= 4; i++) {
    // create associative arrays for 4 virtual decks
        this.vDeck[i] = {
            syncEnabled: false,
            volMSB: 0,
            rateMSB: 0,
            jogEnabled: true,
        };

        const vgroup = `[Channel${  i  }]`;

        // run updateDeckLeds after every track load to set LEDs accordingly
        engine.makeConnection(vgroup, "track_loaded", function(ch, vgroup) {
            DDJFLX2.updateDeckLeds(vgroup);
        });

        // run switchPlayLED after play/pause track to set LEDs accordingly
        engine.makeConnection(vgroup, "play", function(ch, vgroup) {
            const vDeckNo = script.deckFromGroup(vgroup);
            const d = vDeckNo % 2 ? 0 : 1;
            DDJFLX2.switchPlayLED(d, ch);
        });

        // run switchSyncLED after sync toggle to set LEDs accordingly
        engine.makeConnection(vgroup, "sync_enabled", function(ch, vgroup) {
            const vDeckNo = script.deckFromGroup(vgroup);
            const d = vDeckNo % 2 ? 0 : 1;
            DDJFLX2.switchSyncLED(d, ch);
        });

        // listen to changes on hotcues
        for (let j = 1; j <= 8; j++) {
            // run switchPadLED after every hotcue update
            engine.makeConnection(
                vgroup,
                `hotcue_${  j  }_enabled`,
                function(ch, vgroup, control) {
                    const pad = Number(control.split("_")[1]);
                    const vDeckNo = script.deckFromGroup(vgroup);
                    const d = vDeckNo % 2 ? 0 : 1; // d = deckNo - 1
                    DDJFLX2.switchPadLED(d, pad, ch);
                }
            );
        }

        // set Pioneer CDJ cue mode for all decks
        engine.setValue(vgroup, "cue_cdj", true);
    }

    DDJFLX2.LEDsOff();

    // start with focus on library for selecting tracks (delay seems required)
    engine.beginTimer(
        500,
        () => {
            engine.setValue("[Library]", "MoveFocus", 1);
        },
        true
    );

    // query the controller for current control positions on startup
    midi.sendSysexMsg(
        [0xf0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x0a, 0x00, 0x03, 0x01, 0xf7],
        12
    );
};

DDJFLX2.shutdown = function() {
    DDJFLX2.LEDsOff();
};

DDJFLX2.LEDsOff = function() {
    // turn off LED buttons:

    for (let i = 0; i <= 1; i++) {
        midi.sendShortMsg(0x96 + i, 0x63, 0x00); // set headphone master
        midi.sendShortMsg(0x90 + i, 0x54, 0x00); // pfl headphone
        midi.sendShortMsg(0x90 + i, 0x58, 0x00); // beat sync
        midi.sendShortMsg(0x90 + i, 0x0b, 0x00); // play
        midi.sendShortMsg(0x90 + i, 0x0c, 0x00); // cue
        for (let j = 0; j <= 8; j++) {
            midi.sendShortMsg(0x97 + 2 * i, j, 0x00); // hotcue
        }
    }
};

DDJFLX2.updateDeckLeds = function(vgroup) {
    // set LEDs (hotcues, etc.) for the loaded deck
    // if controller is switched to this deck
    const vDeckNo = script.deckFromGroup(vgroup);
    const deckNo = vDeckNo % 2 ? 1 : 2;
    if (vDeckNo === DDJFLX2.vDeckNo[deckNo]) {
        DDJFLX2.switchLEDs(vDeckNo);
    }
};

DDJFLX2.LoadSelectedTrack = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        const deckNo = script.deckFromGroup(group);
        const vDeckNo = DDJFLX2.vDeckNo[deckNo];
        const vgroup = `[Channel${  vDeckNo  }]`;
        script.triggerControl(vgroup, "LoadSelectedTrack", true);
    }
};

DDJFLX2.browseTracks = function(value) {
    DDJFLX2.jogCounter += value - 64;
    if (DDJFLX2.jogCounter > 9) {
        engine.setValue("[Library]", "MoveDown", true);
        DDJFLX2.jogCounter = 0;
    } else if (DDJFLX2.jogCounter < -9) {
        engine.setValue("[Library]", "MoveUp", true);
        DDJFLX2.jogCounter = 0;
    }
};

DDJFLX2.shiftLeft = function() {
    // toggle shift left pressed variable
    DDJFLX2.shiftPressed.left = !DDJFLX2.shiftPressed.left;
};

DDJFLX2.shiftRight = function() {
    // toggle shift right pressed variable
    DDJFLX2.shiftPressed.right = !DDJFLX2.shiftPressed.right;
};

DDJFLX2.jog = function(channel, control, value, status, group) {
    // For a control that centers on 0x40 (64):
    // Convert value down to +1/-1
    // Register the movement
    if (DDJFLX2.shiftPressed.left) {
        DDJFLX2.browseTracks(value);
    } else {
        const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
        if (DDJFLX2.vDeck[vDeckNo].jogEnabled) {
            const vgroup = `[Channel${  vDeckNo  }]`;
            engine.setValue(vgroup, "jog", value - 64);
        }
    }
};

DDJFLX2.scratch = function(channel, control, value, status, group) {
    // For a control that centers on 0x40 (64):
    // Convert value down to +1/-1
    // Register the movement
    engine.scratchTick(DDJFLX2.vDeckNo[script.deckFromGroup(group)], value - 64);
};

DDJFLX2.touch = function(channel, control, value, status, group) {
    const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
    if (value) {
    // enable scratch
        const alpha = 1.0 / 8;
        engine.scratchEnable(vDeckNo, 480, 33 + 1 / 3, alpha, alpha / 32);
        // disable jog not to prevent track alignment
        DDJFLX2.vDeck[vDeckNo].jogEnabled = false;
    } else {
    // enable jog after 900 ms again
        engine.beginTimer(
            900,
            () => {
                DDJFLX2.vDeck[vDeckNo].jogEnabled = true;
            },
            true
        );
        // disable scratch
        engine.scratchDisable(vDeckNo);
    }
};

DDJFLX2.seek = function(channel, control, value, status, group) {
    const oldPos = engine.getValue(group, "playposition");
    // Since ‘playposition’ is normalized to unity, we need to scale by
    // song duration in order for the jog wheel to cover the same amount
    // of time given a constant turning angle.
    const duration = engine.getValue(group, "duration");
    const newPos = Math.max(0, oldPos + ((value - 64) * 0.2) / duration);

    const deckNo = script.deckFromGroup(group);
    const vgroup = `[Channel${  DDJFLX2.vDeckNo[deckNo]  }]`;
    engine.setValue(vgroup, "playposition", newPos); // Strip search
};

DDJFLX2.headmix = function(channel, control, value) {
    // toggle headMix knob between -1 to 1
    if (value) {
    // do nothing if button is released, i.e. value === 0
        const masterMixEnabled = engine.getValue("[Master]", "headMix") > 0;
        engine.setValue("[Master]", "headMix", masterMixEnabled ? -1 : 1);
        midi.sendShortMsg(0x96, 0x63, masterMixEnabled ? 0 : 0x7f); // set LED
    }
};

DDJFLX2.toggleFourDeckMode = function(channel, control, value) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        DDJFLX2.fourDeckMode = !DDJFLX2.fourDeckMode;
        if (DDJFLX2.fourDeckMode) {
            midi.sendShortMsg(0x90, 0x54, 0x00);
            midi.sendShortMsg(0x91, 0x54, 0x00);
        } else {
            DDJFLX2.vDeckNo[1] = 1;
            DDJFLX2.vDeckNo[2] = 2;
            DDJFLX2.switchLEDs(1); // set LEDs of controller deck
            DDJFLX2.switchLEDs(2); // set LEDs of controller deck
        }
    }
};

DDJFLX2.play = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
        const vgroup = `[Channel${  vDeckNo  }]`;
        const playing = engine.getValue(vgroup, "play");
        engine.setValue(vgroup, "play", !playing);
        if (engine.getValue(vgroup, "play") === playing) {
            engine.setValue(vgroup, "play", !playing);
        }
        midi.sendShortMsg(status, 0x0b, engine.getValue(vgroup, "play") ? 0x7f : 0);
    }
};

DDJFLX2.syncEnabled = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
        const vgroup = `[Channel${  vDeckNo  }]`;
        const syncEnabled = !engine.getValue(vgroup, "sync_enabled");
        DDJFLX2.vDeck[vDeckNo].syncEnabled = syncEnabled;
        engine.setValue(vgroup, "sync_enabled", syncEnabled);
        midi.sendShortMsg(status, control, 0x7f * syncEnabled); // set LED
    }
};

DDJFLX2.rateMSB = function(channel, control, value, status, group) {
    // store most significant byte value of rate
    const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
    DDJFLX2.vDeck[vDeckNo].rateMSB = value;
};

DDJFLX2.rateLSB = function(channel, control, value, status, group) {
    const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
    const vgroup = `[Channel${  vDeckNo  }]`;
    // calculate rate value from its most and least significant bytes
    const rateMSB = DDJFLX2.vDeck[vDeckNo].rateMSB;
    const rate = 1 - ((rateMSB << 7) + value) / 0x1fff;
    engine.setValue(vgroup, "rate", rate);
};

DDJFLX2.volumeMSB = function(channel, control, value, status, group) {
    // store most significant byte value of volume
    const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
    DDJFLX2.vDeck[vDeckNo].volMSB = value;
};

DDJFLX2.volumeLSB = function(channel, control, value, status, group) {
    const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
    const vgroup = `[Channel${  vDeckNo  }]`;
    // calculate volume value from its most and least significant bytes
    const volMSB = DDJFLX2.vDeck[vDeckNo].volMSB;
    const vol = ((volMSB << 7) + value) / 0x3fff;
    //var vol = ((volMSB << 7) + value); // use for linear correction
    //vol = script.absoluteNonLin(vol, 0, 0.25, 1, 0, 0x3FFF);
    engine.setValue(vgroup, "volume", vol);
};

DDJFLX2.eq = function(channel, control, value, status, group) {
    const val = script.absoluteNonLin(value, 0, 1, 4);
    let eq = control === 0x0b ? 2 : 1;
    if (control === 0x07) {
        eq = 3;
    }
    const deckNo = group.substring(24, 25);
    // var deckNo = group.match("hannel.")[0].substring(6); // more general
    // var deckNo = script.deckFromGroup(group); // working after fix
    // https://github.com/mixxxdj/mixxx/pull/3178 only
    const vDeckNo = DDJFLX2.vDeckNo[deckNo];
    const vgroup = group.replace(`Channel${  deckNo}`, `Channel${  vDeckNo}`);
    engine.setValue(vgroup, `parameter${  eq}`, val);
};

DDJFLX2.super1 = function(channel, control, value, status, group) {
    const val = script.absoluteNonLin(value, 0, 0.5, 1);
    const deckNo = group.substring(26, 27);
    //var deckNo = group.match("hannel.")[0].substring(6); // more general
    //var deckNo = script.deckFromGroup(group); // working after fix
    // https://github.com/mixxxdj/mixxx/pull/3178 only
    const vDeckNo = DDJFLX2.vDeckNo[deckNo];
    const vgroup = group.replace(`Channel${  deckNo}`, `Channel${  vDeckNo}`);
    engine.setValue(vgroup, "super1", val);
};

DDJFLX2.cueDefault = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
        const vgroup = `[Channel${  vDeckNo  }]`;
        if (!DDJFLX2.vDeck[vDeckNo].jogEnabled) {
            // if jog top is touched
            engine.setValue(vgroup, "cue_set", true);
        } else {
            engine.setValue(vgroup, "cue_gotoandplay", true);
        }
        const cueSet = engine.getValue(vgroup, "cue_point") !== -1;
        midi.sendShortMsg(status, 0x0c, 0x7f * cueSet); // set cue LED
        midi.sendShortMsg(
            status,
            0x0b,
            0x7f * // set play LED
        engine.getValue(vgroup, "play")
        );
    }
};

DDJFLX2.cueGotoandstop = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
        const vgroup = `[Channel${  vDeckNo  }]`;
        engine.setValue(vgroup, "cue_gotoandstop", true);
        //engine.setValue(vgroup, "start_stop", true); // go to start if preferred
        midi.sendShortMsg(status, 0x0b, 0x7f * engine.getValue(vgroup, "play"));
    }
};

DDJFLX2.hotcueNActivate = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
        const vgroup = `[Channel${  vDeckNo  }]`;
        const hotcue = `hotcue_${  control + 1}`;
        engine.setValue(vgroup, `${hotcue  }_activate`, true);
        midi.sendShortMsg(
            status,
            control,
            0x7f * engine.getValue(vgroup, `${hotcue  }_enabled`)
        );
        const deckNo = script.deckFromGroup(group);
        midi.sendShortMsg(
            0x90 + deckNo - 1,
            0x0b,
            0x7f * engine.getValue(vgroup, "play")
        ); // set play LED
    }
};

DDJFLX2.hotcueNClear = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        const vDeckNo = DDJFLX2.vDeckNo[script.deckFromGroup(group)];
        const vgroup = `[Channel${  vDeckNo  }]`;
        engine.setValue(vgroup, `hotcue_${  control + 1  }_clear`, true);
        midi.sendShortMsg(status - 1, control, 0x00); // set hotcue LEDs
    }
};

DDJFLX2.pfl = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        const deckNo = script.deckFromGroup(group);
        const vDeckNo = DDJFLX2.vDeckNo[deckNo];
        const vgroup = `[Channel${  vDeckNo  }]`;
        const pfl = !engine.getValue(vgroup, "pfl");
        engine.setValue(vgroup, "pfl", pfl);
        if (!DDJFLX2.fourDeckMode) {
            midi.sendShortMsg(status, 0x54, 0x7f * pfl); // switch pfl LED
        }
    }
};

DDJFLX2.switchLEDs = function(vDeckNo) {
    // set LEDs of controller deck 1 or 2 according to virtual deck
    const d = vDeckNo % 2 ? 0 : 1; // d = deckNo - 1
    const vgroup = `[Channel${  vDeckNo  }]`;
    DDJFLX2.switchPlayLED(d, engine.getValue(vgroup, "play"));
    midi.sendShortMsg(
        0x90 + d,
        0x0c,
        0x7f * (engine.getValue(vgroup, "cue_point") !== -1)
    );
    DDJFLX2.switchSyncLED(d, engine.getValue(vgroup, "sync_enabled"));
    if (!DDJFLX2.fourDeckMode) {
        midi.sendShortMsg(0x90 + d, 0x54, 0x7f * engine.getValue(vgroup, "pfl"));
    }

    for (let i = 1; i <= 8; i++) {
        const isButtonEnabled = engine.getValue(vgroup, `hotcue_${  i  }_enabled`);
        DDJFLX2.switchPadLED(d, i, isButtonEnabled);
    }
};

DDJFLX2.switchPlayLED = function(deck, enabled) {
    midi.sendShortMsg(0x90 + deck, 0x0b, 0x7f * enabled);
};

DDJFLX2.switchSyncLED = function(deck, enabled) {
    midi.sendShortMsg(0x90 + deck, 0x58, 0x7f * enabled);
};

DDJFLX2.switchPadLED = function(deck, pad, enabled) {
    midi.sendShortMsg(0x97 + 2 * deck, pad - 1, 0x7f * enabled);
};

DDJFLX2.toggleDeck = function(channel, control, value, status, group) {
    if (value) {
    // only if button pressed, not releases, i.e. value === 0
        if (DDJFLX2.shiftPressed.left) {
            // left shift + pfl 1/2 does not toggle decks but loads track
            DDJFLX2.LoadSelectedTrack(channel, control, value, status, group);
        } else if (DDJFLX2.fourDeckMode) {
            //right shift + pfl 1/2 toggles
            const deckNo = script.deckFromGroup(group);
            let vDeckNo;
            let led = 0x7f;
            if (deckNo === 1) {
                // toggle virtual deck of controller deck 1
                DDJFLX2.vDeckNo[1] = 4 - DDJFLX2.vDeckNo[1];
                if (DDJFLX2.vDeckNo[1] === 1) {
                    led = 0;
                }
                vDeckNo = DDJFLX2.vDeckNo[1];
            } else {
                // deckNo === 2
                // toggle virtual deck of controller deck 2
                DDJFLX2.vDeckNo[2] = 6 - DDJFLX2.vDeckNo[2];
                if (DDJFLX2.vDeckNo[2] === 2) {
                    led = 0;
                }
                vDeckNo = DDJFLX2.vDeckNo[2];
            }
            midi.sendShortMsg(status, 0x54, led); // toggle virtual deck LED
            DDJFLX2.switchLEDs(vDeckNo); // set LEDs of controller deck
        }
    }
};

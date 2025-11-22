// Based on Numark Mixtrack Mapping Script Functions
// 1/11/2010 - v0.1 - Matteo <matteo@magm3.com>
//
// 5/18/2011 - Changed by James Ralston
//
// 3/11/2024 - Changed by Josh Patten    // codespell:ignore patten
// 2025-04-19 - engine.softTakeover("[Channel1]", "rate", true); by vespadj
//            - waveform_zoom _up _down: press hold and rotate FX Select Deck1
//            - Filter: PFL(cue) + Mid
//            - Original "PITCH BEND" buttons remapped as beatjump_forward and beatjump_forward
//            - Fast forward/rewind: hold beatjump_forward + jogWheel (or beatjump_backward)
//
// Known Bugs:
//	Mixxx complains about an undefined variable on 1st load of the mapping (ignore it, then restart Mixxx)
//	Each slide/knob needs to be moved on Mixxx startup to match levels with the Mixxx UI
//
// 05/26/2012 to 06/27/2012 - Changed by Darío José Freije <dario2004@gmail.com>
//
//	Almost all work like expected. Resume and Particularities:
//
// ************* Script now is Only for 1.11.0 and above *************
//
//  "Delete", aka "VIEW" and "TICK" buttons, used as [Shift].
//
//	Delete + Hotcues: Clear Hotcues (First press Delete, then Hotcue).
//	Delete + Reloop:  Clear Loop.
//	Delete + Manual:  Set Quantize ON (for best manual loop) or OFF.
//	Delete + Sync: 	  Set Pitch to Zero.
//
// 	Load track: 	Only if the track is paused. Put the pitch in 0 at load.
//
//  	Gain: 	The 3rd knob of the "effect" section is "Gain" (up to clip).
//
//	Effect:	Flanger. 1st and 2nd knob modify Depth and Delay.
//
//	Cue: 	Don't set Cue accidentally at the end of the song (return to the latest cue).
//		LED ON when stopped. LED OFF when playing.
//		LED Blink at Beat time in the ultimates 30 seconds of song.
//
// 	Stutter: Adjust BeatGrid in the correct place (useful to sync well).
//		 LED Blink at each Beat of the grid.
//
//	Sync:	If the other deck is stopped, only sync tempo (not phase).
//		LED Blink at Clip Gain (Peak indicator).
//
// 	Pitch: 	Up, Up; Down, Down. Pitch slide are inverted, to match with the screen (otherwise is very confusing).
//		Soft-takeover to prevent sudden wide parameter changes when the on-screen control diverges from a hardware control.
//		The control will have no effect until the position is close to that of the software,
//		at which point it will take over and operate as usual.
//
// 	Auto Loop (LED ON): 	Active at program Start.
//				"1 Bar" button: Active an Instant 4 beat Loop. Press again to exit loop.
//
//	Scratch:
//	In Stop mode, with Scratch OFF or ON: 	Scratch at touch, and Stop moving when the wheel stop moving.
//	In Play mode, with Scratch OFF: 	Only Pitch bend.
// 	In Play mode, with Scratch ON: 		Scratch at touch and, in Backwards Stop Scratch when the wheel stop moving for 20ms -> BACKSPIN EFFECT!!!!.
//						In Fordward Stop Scratch when the touch is released > Play Immediately (without breaks for well mix).
//						Border of the wheels: Pitch Bend.
//

// eslint-disable-next-line no-var
var NumarkMixTrackPro = {};

NumarkMixTrackPro.init = function(id) {
    // called when the MIDI device is opened & set up

    // Store the ID of this device for later use
    NumarkMixTrackPro.id = id;

    NumarkMixTrackPro.directoryMode = false;
    NumarkMixTrackPro.scratchMode = [false, false];
    NumarkMixTrackPro.manualLoop = [true, true];
    NumarkMixTrackPro.shiftKey = [false, false]; // used as [Shift], aka "VIEW" and "TICK" buttons
    NumarkMixTrackPro.touch = [false, false];
    NumarkMixTrackPro.scratchTimer = [-1, -1];
    NumarkMixTrackPro.fxKnobPressed = [0, 0];
    NumarkMixTrackPro.syncLastTimestamp = [0, 0];
    NumarkMixTrackPro.loopEditIn = [-1, -1];

    NumarkMixTrackPro.isBeatJumpBackwardOn = [0, 0]; // glossary: forward, backward
    engine.makeConnection("[Channel1]", "beatjump_backward", (value) => {
        NumarkMixTrackPro.isBeatJumpBackwardOn[0] = value;
    });
    engine.makeConnection("[Channel2]", "beatjump_backward", (value) => {
        NumarkMixTrackPro.isBeatJumpBackwardOn[1] = value;
    });

    NumarkMixTrackPro.isBeatJumpForwardOn = [0, 0]; // glossary: forward, backward
    engine.makeConnection("[Channel1]", "beatjump_forward", (value) => {
        NumarkMixTrackPro.isBeatJumpForwardOn[0] = value;
    });
    engine.makeConnection("[Channel2]", "beatjump_forward", (value) => {
        NumarkMixTrackPro.isBeatJumpForwardOn[1] = value;
    });

    NumarkMixTrackPro.isPflOn = [0, 0];
    engine.makeConnection("[Channel1]", "pfl", (value) => {
        NumarkMixTrackPro.isPflOn[0] = value;
        console.log("NumarkMixTrackPro.isPflOn[0]", NumarkMixTrackPro.isPflOn[0]);
    });
    engine.makeConnection("[Channel2]", "pfl", (value) => {
        NumarkMixTrackPro.isPflOn[1] = value;
    });

    NumarkMixTrackPro.isFxOn = [0, 0];
    engine.makeConnection(
        "[EffectRack1_EffectUnit1_Effect1]",
        "enabled",
        (value) => {
            NumarkMixTrackPro.isFxOn[0] = value;
        }
    );
    engine.makeConnection(
        "[EffectRack1_EffectUnit2_Effect1]",
        "enabled",
        (value) => {
            NumarkMixTrackPro.isFxOn[1] = value;
        }
    );

    NumarkMixTrackPro.leds = [
        // Common
        {directory: 0x73, file: 0x72},
        // Deck 1
        {
            effect: 0x63,
            headphone: 0x65,
            rate: 0x70,
            scratchMode: 0x48,
            manualLoop: 0x61,
            keylock: 0x51,
            // eslint-disable-next-line camelcase
            loop_start_position: 0x53,
            // eslint-disable-next-line camelcase
            loop_end_position: 0x54,
            reloopExit: 0x55,
            shiftKey: 0x59,
            hotCue1: 0x5a,
            hotCue2: 0x5b,
            hotCue3: 0x5c,
            stutter: 0x4a,
            Cue: 0x33,
            sync: 0x40,
        },
        // Deck 2
        {
            effect: 0x64,
            headphone: 0x66,
            rate: 0x71,
            scratchMode: 0x50,
            manualLoop: 0x62,
            keylock: 0x52,
            // eslint-disable-next-line camelcase
            loop_start_position: 0x56,
            // eslint-disable-next-line camelcase
            loop_end_position: 0x57,
            reloopExit: 0x58,
            shiftKey: 0x5d,
            hotCue1: 0x5e,
            hotCue2: 0x5f,
            hotCue3: 0x60,
            stutter: 0x4c,
            Cue: 0x3c,
            sync: 0x47,
        },
    ];

    NumarkMixTrackPro.ledTimers = {};

    NumarkMixTrackPro.LedTimer = function(id, led, count, state) {
        this.id = id;
        this.led = led;
        this.count = count;
        this.state = state;
    };

    // Turn off all the lights
    for (let i = 0x30; i <= 0x73; i++) {
        midi.sendShortMsg(0x90, i, 0x00);
    }

    NumarkMixTrackPro.hotCue = {
        //Deck 1
        0x5a: "1",
        0x5b: "2",
        0x5c: "3",
        //Deck 2
        0x5e: "1",
        0x5f: "2",
        0x60: "3",
    };

    //Add event listeners
    for (let i = 1; i < 3; i++) {
        for (let x = 1; x < 4; x++) {
            engine.makeConnection(
                `[Channel${i}]`,
                `hotcue_${x}_status`,
                NumarkMixTrackPro.onHotCueChange
            );
        }
        NumarkMixTrackPro.setLoopMode(i, false);
    }

    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[0].file, true);

    NumarkMixTrackPro.setLED(
        NumarkMixTrackPro.leds[1].keylock,
        engine.getParameter("[Channel1]", "keylock")
    );

    NumarkMixTrackPro.setLED(
        NumarkMixTrackPro.leds[2].keylock,
        engine.getParameter("[Channel2]", "keylock")
    );

    // Enable soft-takeover for Pitch slider
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);

    // Clipping LED
    engine.makeConnection(
        "[Channel1]",
        "peak_indicator",
        NumarkMixTrackPro.Channel1Clip
    );
    engine.makeConnection(
        "[Channel2]",
        "peak_indicator",
        NumarkMixTrackPro.Channel2Clip
    );

    // Stutter beat light
    engine.makeConnection(
        "[Channel1]",
        "beat_active",
        NumarkMixTrackPro.Stutter1Beat
    );
    engine.makeConnection(
        "[Channel2]",
        "beat_active",
        NumarkMixTrackPro.Stutter2Beat
    );

    // Settings
    NumarkMixTrackPro.settings = {};

    // get and map
    const settingsOptions = [
        "brakeEnabled",
        "meanPitchBendBtns",
        "editLoopByWheelEnabled",
        "quickFxActivator",
    ];
    settingsOptions.forEach((item) => {
        NumarkMixTrackPro.settings[item] =
            engine.getSetting(`numarkMixTrackPro_${item}`) || 0;
    });
};

NumarkMixTrackPro.pitchBendBtns = function(group, value, isPlus) {
    switch (NumarkMixTrackPro.settings.meanPitchBendBtns) {
    case "beatjump":
        engine.setParameter(
            group,
            `beatjump${isPlus ? "_forward" : "_backward"}`,
            !!value
        );
        break;

    case "rate_temp":
        engine.setParameter(
            group,
            `rate_temp${isPlus ? "_up" : "_down"}`,
            !!value
        );
        break;

    case "pitch":
        if (
        // if also the other button is pressed
            value &&
                engine.getParameter(group, `pitch${!isPlus ? "_up" : "_down"}`)
        ) {
            // reset pitch
            engine.setParameter(group, "pitch", 0.5);
        } else {
            engine.setParameter(group, `pitch${isPlus ? "_up" : "_down"}`, !!value);
        }
        break;

    default:
        console.error(
            "NumarkMixTrackPro.settings.meanPitchBendBtns",
            NumarkMixTrackPro.settings.meanPitchBendBtns
        );
        break;
    }
};

NumarkMixTrackPro.pitchBendMinus = function(
    channel,
    control,
    value,
    status,
    group
) {
    const isPlus = false;
    NumarkMixTrackPro.pitchBendBtns(group, value, isPlus);
};

NumarkMixTrackPro.pitchBendPlus = function(
    channel,
    control,
    value,
    status,
    group
) {
    const isPlus = true;
    NumarkMixTrackPro.pitchBendBtns(group, value, isPlus);
};

NumarkMixTrackPro.pitchFader = function(
    channel,
    control,
    value,
    status,
    group
) {
    let newValue = value / 127;
    if (value === 63 || value === 64) {
        newValue = 0.5;
    }
    engine.setParameter(group, "rate", newValue);
};

NumarkMixTrackPro.Channel1Clip = function(value) {
    NumarkMixTrackPro.clipLED(value, NumarkMixTrackPro.leds[1].sync);
};

NumarkMixTrackPro.Channel2Clip = function(value) {
    NumarkMixTrackPro.clipLED(value, NumarkMixTrackPro.leds[2].sync);
};

NumarkMixTrackPro.setStutterBeat = function(deck, value) {
    const secondsBlink = 30;
    const secondsToEnd =
        engine.getParameter(`[Channel${deck}]`, "duration") *
        (1 - engine.getParameter(`[Channel${deck}]`, "playposition"));

    if (
        secondsToEnd < secondsBlink &&
        secondsToEnd > 1 &&
        engine.getParameter(`[Channel${deck}]`, "play")
    ) {
        // The song is going to end
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].Cue, value);
    }

    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].stutter, value);
};

NumarkMixTrackPro.Stutter1Beat = function(value) {
    NumarkMixTrackPro.setStutterBeat(1, value);
};

NumarkMixTrackPro.Stutter2Beat = function(value) {
    NumarkMixTrackPro.setStutterBeat(2, value);
};

NumarkMixTrackPro.clipLED = function(value, note) {
    if (value > 0) {
        NumarkMixTrackPro.flashLED(note, 1);
    } else {
        NumarkMixTrackPro.setLED(note, value);
    }
};

NumarkMixTrackPro.shutdown = function() {
    // called when the MIDI device is closed
    const lowestLED = 0x30;
    const highestLED = 0x73;
    for (let i = lowestLED; i <= highestLED; i++) {
        NumarkMixTrackPro.setLED(i, false); // Turn off all the lights
    }
};

NumarkMixTrackPro.setLED = function(value, status) {
    status = status ? 0x64 : 0x00;
    midi.sendShortMsg(0x90, value, status);
};

NumarkMixTrackPro.flashLED = function(led, veces) {
    const ndx = Math.random();
    const id = engine.beginTimer(120, NumarkMixTrackPro.doFlash(ndx, veces));
    NumarkMixTrackPro.ledTimers[ndx] = new NumarkMixTrackPro.LedTimer(
        id,
        led,
        0,
        false
    );
};

NumarkMixTrackPro.doFlash = function(ndx, veces) {
    const ledTimer = NumarkMixTrackPro.ledTimers[ndx];

    if (!ledTimer) {
        return;
    }

    if (ledTimer.count > veces) {
        // how many times blink the button
        engine.stopTimer(ledTimer.id);
        delete NumarkMixTrackPro.ledTimers[ndx];
    } else {
        ledTimer.count++;
        ledTimer.state = !ledTimer.state;
        NumarkMixTrackPro.setLED(ledTimer.led, ledTimer.state);
    }
};

NumarkMixTrackPro.selectKnob = function(
    channel,
    control,
    value
) {
    if (value > 63) {
        value = value - 128;
    }
    if (NumarkMixTrackPro.directoryMode) {
        // [Playlist]SelectNextPlaylist and SelectPrevPlaylist are deprecaded
        // but it respect LED on controller, [Library]MoveDown and MoveUp don't.
        if (value > 0) {
            for (let i = 0; i < value; i++) {
                script.triggerControl("[Playlist]", "SelectNextPlaylist", 50);
            }
        } else {
            for (let i = 0; i < -value; i++) {
                script.triggerControl("[Playlist]", "SelectPrevPlaylist", 50);
            }
        }
    } else {
        engine.setParameter("[Playlist]", "SelectTrackKnob", value);
    }
};

NumarkMixTrackPro.LoadTrack = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (value) {
        script.triggerControl(group, "LoadSelectedTrack", 50);

        // reset rate/pitch to 0%
        engine.softTakeover(group, "rate", false);
        engine.setParameter(group, "rate", 0.5);
        engine.softTakeover(group, "rate", true);
    }
};

NumarkMixTrackPro.cuebutton = function(
    channel,
    control,
    value,
    status,
    group
) {
    // Don't set Cue accidentally at the end of the song
    if (engine.getParameter(group, "playposition") <= 0.97) {
        engine.setParameter(group, "cue_default", value ? 1 : 0);
    } else {
        engine.setParameter(group, "cue_preview", value ? 1 : 0);
    }
};

NumarkMixTrackPro.beatsync = function(channel, control, value, status, group) {
    const deck = script.deckFromGroup(group);

    if (NumarkMixTrackPro.shiftKey[deck - 1]) {
    // Shift + SYNC = reset rate to 0, at half (0.5)
        engine.softTakeover(group, "rate", false);
        engine.setParameter(group, "rate", 0.5);
        engine.softTakeover(group, "rate", true);

        // Reset shift key state
        NumarkMixTrackPro.toggleShiftKey(channel, control, value, status, group);
    } else {
        // New sync_enabled. Fix for Leader persistent, same as GUI
        // define an Handler and link to global var
        const lastTimestampHandler = (deck) => {
            const index = deck - 1;
            return {
                get: () => NumarkMixTrackPro.syncLastTimestamp[index],
                set: (value) => {
                    NumarkMixTrackPro.syncLastTimestamp[index] = value;
                },
            };
        };
        const lastTimestamp = lastTimestampHandler(deck);

        const syncLeader = engine.getParameter(group, "sync_leader");
        const timestamp = Date.now();

        if (value) {
            if (!syncLeader) {
                engine.setParameter(group, "sync_enabled", true);
                lastTimestamp.set(timestamp);
            } else {
                engine.setParameter(group, "sync_enabled", false);
                lastTimestamp.set(0);
            }
        } else {
            if (lastTimestamp.get() === 0) {
                return;
            }
            if (timestamp - lastTimestamp.get() < 250) {
                engine.setParameter(group, "sync_enabled", false);
            }
            // else for long press > 250 ms, do nothing
        }
    }
};

NumarkMixTrackPro.playbutton = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (!value) {
        return;
    } // (NoteOff, 0x00, button up)
    const deck = script.deckFromGroup(group);

    if (!NumarkMixTrackPro.settings.brakeEnabled) {
        // Play/Pause standard
        if (engine.getParameter(group, "play")) {
            engine.setParameter(group, "play", 0);
        } else {
            engine.setParameter(group, "play", 1);
        }
    } else {
        // Brake and Soft Start if scratch led is on
        // Mixxx v.2.6+ required
        if (engine.getParameter(group, "play") && !engine.isBrakeActive(deck)) {
            if (NumarkMixTrackPro.scratchMode[deck - 1]) {
                engine.brake(deck, true);
            } else {
                engine.setParameter(group, "play", 0);
            }
        } else {
            if (NumarkMixTrackPro.scratchMode[deck - 1]) {
                engine.softStart(deck, true);
            } else {
                engine.setParameter(group, "play", 1);
            }
        }
    }
};

NumarkMixTrackPro.toggleManualLooping = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (!value) { return; }

    const deck = script.deckFromGroup(group);

    if (NumarkMixTrackPro.shiftKey[deck - 1]) {
        // If Shift is on, toggle quantize
        if (engine.getParameter(group, "quantize")) {
            engine.setParameter(group, "quantize", 0);
        } else {
            engine.setParameter(group, "quantize", 1);
        }

        NumarkMixTrackPro.toggleShiftKey(channel, control, value, status, group);
    } else {
        NumarkMixTrackPro.setLoopMode(
            deck,
            !NumarkMixTrackPro.manualLoop[deck - 1]
        );
    }
};

NumarkMixTrackPro.loopIn = function(channel, control, value, status, group) {
    if (!value) {
        return;
    }

    const deck = script.deckFromGroup(group);

    if (NumarkMixTrackPro.manualLoop[deck - 1]) {
        // Manual Mode
        script.triggerControl(group, "loop_in", 100);
    } else {
        // Auto Mode
        const loopActive = engine.getParameter(group, "loop_enabled");
        if (loopActive) {
            script.triggerControl(group, "loop_halve", 1);
        } else {
            engine.setParameter(group, "beatloop_1_activate", 1);
        }
    }
};

NumarkMixTrackPro.loopOut = function(channel, control, value, status, group) {
    if (!value) {
        return;
    }

    const deck = script.deckFromGroup(group);

    if (NumarkMixTrackPro.manualLoop[deck - 1]) {
        // Manual Mode: set LoopOut point
        script.triggerControl(group, "loop_out", 100);
    } else {
        // Auto mode: loop of 4 beat (1 bar)
        const isLoopActive = engine.getParameter(group, "loop_enabled");

        if (!isLoopActive) {
            engine.setParameter(group, "beatloop_4_activate", 1);
        } else {
            engine.setParameter(group, "loop_enabled", 0);
        }
    }
};

NumarkMixTrackPro.reLoop = function(channel, control, value, status, group) {
    if (!value) { return; }

    const deck = script.deckFromGroup(group);

    if (NumarkMixTrackPro.shiftKey[deck - 1]) {
        // Shift: exit loop, reset Shift
        script.triggerControl(group, "loop_remove", 50);
        NumarkMixTrackPro.toggleShiftKey(channel, control, value, status, group);
    } else if (NumarkMixTrackPro.manualLoop[deck - 1]) {
        // Manual mode: recall last loop or exit current loop
        script.triggerControl(group, "reloop_toggle", 1);
    } else {
        // Auto mode: 2x
        const loopActive = engine.getParameter(group, "loop_enabled");
        if (loopActive) {
            script.triggerControl(group, "loop_double", 1);
        } else {
            // recall last loop
            script.triggerControl(group, "reloop_toggle", 1);
        }
    }
};

NumarkMixTrackPro.setLoopMode = function(deck, manual) {
    NumarkMixTrackPro.manualLoop[deck - 1] = manual;
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].manualLoop, manual);

    const group = `[Channel${deck}]`;

    if (NumarkMixTrackPro.connections === undefined) {
        NumarkMixTrackPro.connections = {};
    }

    if (NumarkMixTrackPro.connections[deck] === undefined) {
        NumarkMixTrackPro.connections[deck] = {};
    }

    for (const conn in NumarkMixTrackPro.connections[deck]) {
        if (NumarkMixTrackPro.connections[deck][conn]) {
            NumarkMixTrackPro.connections[deck][conn].disconnect();
            NumarkMixTrackPro.connections[deck][conn] = null;
        }
    }

    // Create
    if (manual) {
        NumarkMixTrackPro.connections[deck].loopStart = engine.makeConnection(
            group,
            "loop_start_position",
            NumarkMixTrackPro.onLoopChange
        );

        NumarkMixTrackPro.connections[deck].loopEnd = engine.makeConnection(
            group,
            "loop_end_position",
            NumarkMixTrackPro.onLoopChange
        );

        NumarkMixTrackPro.connections[deck].loopEnabled = engine.makeConnection(
            group,
            "loop_enabled",
            NumarkMixTrackPro.onReloopExitChange
        );

        // Update LED to the actual state
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loop_start_position,
            engine.getParameter(group, "loop_start_position") > -1
        );
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loop_end_position,
            engine.getParameter(group, "loop_end_position") > -1
        );
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].reloopExit,
            engine.getParameter(group, "loop_enabled")
        );
    } else {
        // Auto Mode
        NumarkMixTrackPro.connections[deck].loopEnabled = engine.makeConnection(
            group,
            "loop_enabled",
            NumarkMixTrackPro.onReloopExitChangeAuto
        );

        // Update LED to Auto Mode
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loop_start_position,
            false
        );
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loop_end_position,
            engine.getParameter(group, "loop_enabled")
        );
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].reloopExit,
            false
        );
    }
};

NumarkMixTrackPro.onLoopChange = function(value, group, key) {
    const deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(
        NumarkMixTrackPro.leds[deck][key],
        value > -1
    );
};

NumarkMixTrackPro.onReloopExitChange = function(value, group) {
    const deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(
        NumarkMixTrackPro.leds[deck].reloopExit,
        value
    );
};

NumarkMixTrackPro.onReloopExitChangeAuto = function(value, group) {
    const deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(
        NumarkMixTrackPro.leds[deck].loop_end_position,
        value
    );
};

// Stutters adjust BeatGrid
NumarkMixTrackPro.playFromCue = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = script.deckFromGroup(group);

    if (engine.getParameter(group, "beats_translate_curpos")) {
        engine.setParameter(group, "beats_translate_curpos", 0);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].stutter, 0);
    } else {
        engine.setParameter(group, "beats_translate_curpos", 1);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].stutter, 1);
    }
};



NumarkMixTrackPro.jogWheel = function(channel, control, value, status, group) {
    const deck = script.deckFromGroup(group);

    let adjustedJog = parseFloat(value); // 1; 2; ...; 13 or 120; ...; 127
    let posNeg = 1;
    if (adjustedJog > 63) {
        // Counter-clockwise
        posNeg = -1;
        adjustedJog = value - 128; // -13 .. +13
    }
    // Convert to float (+/- 0.6...)
    const gammaInputRange = 13; // Max jog speed
    const maxOutFraction = 0.8; // Where on the curve it should peak; 0.5 is half-way
    const sensitivity = 0.5; // Adjustment gamma
    const gammaOutputRange = 2; // Max rate change

    const adjustedJogAdv =
        posNeg *
        gammaOutputRange *
        Math.pow(
            Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction),
            sensitivity
        );

    // Fast forward/rewind if beatjump_forward is pressed
    if (
        NumarkMixTrackPro.isBeatJumpForwardOn[deck - 1] ||
        NumarkMixTrackPro.isBeatJumpBackwardOn[deck - 1]
    ) {
        // const x = parseFloat(value);
        // console.log("x; adjustedJog; adjustedJogAdv", x, adjustedJog, adjustedJogAdv);
        engine.setParameter(group, "beatjump", adjustedJog);
        return;
    }

    // Loop editing by Wheel
    const loopActive = engine.getParameter(group, "loop_enabled");

    if (
        NumarkMixTrackPro.settings.editLoopByWheelEnabled &&
        loopActive &&
        !NumarkMixTrackPro.scratchMode[deck - 1] &&
        NumarkMixTrackPro.touch[deck - 1]
    ) {
        if (NumarkMixTrackPro.loopEditIn[deck - 1] === -1) {
            NumarkMixTrackPro.loopEditIn[deck - 1] = adjustedJog < 0 ? 0 : 1;
        }

        if (NumarkMixTrackPro.loopEditIn[deck - 1] === 1) {
            // begin a loop move
            // loop_move_X... is very raw...
            const x = engine.getParameter(group, "loop_start_position");
            const y = x + adjustedJog * 300; // 44200 / 300 = 147 parts per second
            engine.setParameter(group, "loop_start_position", y);
            return;
        }

        if (NumarkMixTrackPro.loopEditIn[deck - 1] === 0) {
            const scale = 1 + adjustedJog * 0.0009;
            engine.setParameter(group, "loop_scale", scale);
            return;
        }
    }

    if (engine.getParameter(group, "play")) {
        if (
            NumarkMixTrackPro.scratchMode[deck - 1] &&
            posNeg === -1 &&
            !NumarkMixTrackPro.touch[deck - 1]
        ) {
            if (NumarkMixTrackPro.scratchTimer[deck - 1] !== -1) {
                engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck - 1]);
            }
            NumarkMixTrackPro.scratchTimer[deck - 1] = engine.beginTimer(
                20,
                () => {
                    NumarkMixTrackPro.jogWheelStopScratch(deck);
                },
                true
            );
        }
    } else {
        // stop scratching
        if (!NumarkMixTrackPro.touch[deck - 1]) {
            if (NumarkMixTrackPro.scratchTimer[deck - 1] !== -1) {
                engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck - 1]);
            }
            NumarkMixTrackPro.scratchTimer[deck - 1] = engine.beginTimer(
                20,
                () => {
                    NumarkMixTrackPro.jogWheelStopScratch(deck);
                },
                true
            );
        }
    }

    engine.scratchTick(deck, adjustedJog);

    if (engine.getParameter(group, "play")) {
        engine.setParameter(group, "jog", adjustedJogAdv);
    }
};

NumarkMixTrackPro.jogWheelStopScratch = function(deck) {
    NumarkMixTrackPro.scratchTimer[deck - 1] = -1;
    engine.scratchDisable(deck);
};

NumarkMixTrackPro.wheelTouch = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = script.deckFromGroup(group);

    if (!value) {
        // Untouch
        NumarkMixTrackPro.touch[deck - 1] = false;
        NumarkMixTrackPro.loopEditIn[deck - 1] = -1;

        // Stop the timer (if it doesn't exist, it gives an error) and start a new timer.
        // If the wheel doesn't move within 20 milliseconds, it disables scratching.

        if (NumarkMixTrackPro.scratchTimer[deck - 1] !== -1) {
            engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck - 1]);
        }

        NumarkMixTrackPro.scratchTimer[deck - 1] = engine.beginTimer(
            20,
            () => {
                NumarkMixTrackPro.jogWheelStopScratch(deck);
            },
            true
        );
    } else {
        NumarkMixTrackPro.touch[deck - 1] = true;

        // if playing and scratch mode is disabled, do nothing on press
        if (
            !NumarkMixTrackPro.scratchMode[deck - 1] &&
            engine.getParameter(group, "play")
        ) {
            return;
        }

        // scratch disables braking, so stop if is braking to avoid re-play
        if (NumarkMixTrackPro.settings.brakeEnabled) {
            // Mixxx v.2.6+ required
            if (engine.isBrakeActive(deck)) {
                engine.setParameter(group, "play", 0); // Stop
            }

            // TODO: little bug if engine.isSoftStartActive(deck), workaround: double-touch
            // I don't understand why.
            /*
            if (engine.isSoftStartActive(deck)) {
                // force exit from SoftStart
                engine.setParameter(group, "play", 0); // Stop
                // not work
                // engine.setParameter(group, "play", 1); // Play
            }
            */
        }

        if (NumarkMixTrackPro.scratchTimer[deck - 1] !== -1) {
            engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck - 1]);
        }

        // change the 600 value for sensibility
        engine.scratchEnable(deck, 600, 33 + 1 / 3, 1.0 / 8, 1.0 / 8 / 32);
    }
};

NumarkMixTrackPro.toggleDirectoryMode = function(
    channel,
    control,
    value
) {
    // Toggle setting and light
    if (value) {
        NumarkMixTrackPro.directoryMode = !NumarkMixTrackPro.directoryMode;
        // https://manual.mixxx.org/latest/chapters/appendix/mixxx_controls.html#control-[Library]-focused_widget
        /*
        if (NumarkMixTrackPro.directoryMode) {
            script.triggerControl('[Library]', 'MoveFocusBackward', 50); // [Shift+TAB]; [TAB]: MoveFocusForward
        }
        */
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[0].directory,
            NumarkMixTrackPro.directoryMode
        );
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[0].file,
            !NumarkMixTrackPro.directoryMode
        );
    }
};

NumarkMixTrackPro.onCentralKnobPress = function(
    channel,
    control,
    value
) {
    if (!value) { return; }

    // DEPRECADED mode but better
    if (NumarkMixTrackPro.directoryMode) {
        script.triggerControl("[Playlist]", "ToggleSelectedSidebarItem", 50);
    } else {
        script.triggerControl("[Playlist]", "LoadSelectedIntoFirstStopped", 50);
    }

    // New Mode: TODO: fix leds in toggleDirectoryMode
    // script.triggerControl('[Library]', 'GoToItem', 50);
};

NumarkMixTrackPro.toggleScratchMode = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (!value) {
        return;
    }

    const deck = script.deckFromGroup(group);
    // Toggle setting and light
    NumarkMixTrackPro.scratchMode[deck - 1] =
        !NumarkMixTrackPro.scratchMode[deck - 1];
    NumarkMixTrackPro.setLED(
        NumarkMixTrackPro.leds[deck].scratchMode,
        NumarkMixTrackPro.scratchMode[deck - 1]
    );
};

NumarkMixTrackPro.onHotCueChange = function(value, group, key) {
    const deck = script.deckFromGroup(group);
    const hotCueNum = key[7];
    NumarkMixTrackPro.setLED(
        NumarkMixTrackPro.leds[deck][`hotCue${hotCueNum}`],
        !!value
    );
};

NumarkMixTrackPro.changeHotCue = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = script.deckFromGroup(group);
    const hotCue = NumarkMixTrackPro.hotCue[control];

    // onHotCueChange called automatically
    if (NumarkMixTrackPro.shiftKey[deck - 1]) {
        if (engine.getParameter(group, `hotcue_${hotCue}_enabled`)) {
            script.triggerControl(group, `hotcue_${hotCue}_clear`, 50);
        }
        NumarkMixTrackPro.toggleShiftKey(channel, control, value, status, group);
    } else {
        // Press and realise button (no trigger)
        if (value) {
            engine.setParameter(group, `hotcue_${hotCue}_activate`, 1);
        } else {
            engine.setParameter(group, `hotcue_${hotCue}_activate`, 0);
        }
    }
};

NumarkMixTrackPro.toggleShiftKey = function(
    channel,
    control,
    value,
    status,
    group
) {
    // used as [Shift], aka "VIEW" and "TICK" buttons
    // was toggleDeleteKey

    if (!value) {
        return;
    }

    const deck = script.deckFromGroup(group);
    NumarkMixTrackPro.shiftKey[deck - 1] = !NumarkMixTrackPro.shiftKey[deck - 1];
    NumarkMixTrackPro.setLED(
        NumarkMixTrackPro.leds[deck].shiftKey,
        NumarkMixTrackPro.shiftKey[deck - 1]
    );
};

NumarkMixTrackPro.mid = function(channel, control, value, status, group) {
    // used as Mid or Filter if Shift* is on
    if (isNaN(value)) { return; }

    const deck = script.deckFromGroup(group);
    const _value = script.absoluteLin(value, 0, 1);

    if (
        (NumarkMixTrackPro.settings.quickFxActivator === "cue" &&
            NumarkMixTrackPro.isPflOn[deck - 1]) ||
        (NumarkMixTrackPro.settings.quickFxActivator === "shift" &&
            NumarkMixTrackPro.shiftKey[deck - 1]) ||
        (NumarkMixTrackPro.settings.quickFxActivator === "effect" &&
            NumarkMixTrackPro.isFxOn[deck - 1])
    ) {
        // Filter, or the selected FX in Quick Effect Rack
        // TODO: Add soft takeover?
        engine.setParameter(`[QuickEffectRack1_${group}]`, "super1", _value);
    } else {
        // Mid
        engine.setParameter(
            `[EqualizerRack1_${group}_Effect1]`,
            "parameter2",
            _value
        );
    }
};

NumarkMixTrackPro.fxSelectKnobPress = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = script.deckFromGroup(group);

    // [QuickEffectRack1_[Channel1]] or [QuickEffectRack1_[Channel2]]
    if (value === 127) {
        script.triggerControl(
            `[QuickEffectRack1_${group}]`,
            "super1_set_default",
            100
        );
        NumarkMixTrackPro.fxKnobPressed[deck - 1] = true;
    } else {
        NumarkMixTrackPro.fxKnobPressed[deck - 1] = false;
    }
};

NumarkMixTrackPro.fxSelectKnobRotate = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = script.deckFromGroup(group);
    if (NumarkMixTrackPro.fxKnobPressed[deck - 1]) {
        if (deck === 1) {
            if (value === 0x7f) {
                script.triggerControl(group, "waveform_zoom_down", 100);
            } else {
                script.triggerControl(group, "waveform_zoom_up", 100);
            }
        }
    } else {
        // Select FX
        const deck = script.deckFromGroup(group);
        const fxGroup = `[EffectRack1_EffectUnit${deck}_Effect1]`;
        if (value === 1) {
            engine.setParameter(fxGroup, "effect_selector", -1);
        } else {
            engine.setParameter(fxGroup, "effect_selector", +1);
        }
    }
};

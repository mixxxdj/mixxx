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


// Global constants/variables
/*
const ON = 0x7f;
const OFF = 0x00;
const DOWN = 0x7f;
const FW = 0x01; // wheel forward
const RW = 0x7f; // wheel rewind
*/

/* eslint no-undef: "error" */
function NumarkMixTrackPro() {}

NumarkMixTrackPro.init = function(id) {
    // called when the MIDI device is opened & set up

    // Store the ID of this device for later use
    NumarkMixTrackPro.id = id;

    NumarkMixTrackPro.directoryMode = false;
    NumarkMixTrackPro.scratchMode = [false, false];
    NumarkMixTrackPro.manualLoop = [true, true];
    NumarkMixTrackPro.shiftKey = [false, false]; // used as [Shift], aka "VIEW" and "TICK" buttons
    NumarkMixTrackPro.isKeyLocked = [0, 0]; // TODO: delete unused
    NumarkMixTrackPro.touch = [false, false];
    NumarkMixTrackPro.scratchTimer = [-1, -1];

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
            loopStartPosition: 0x53,
            loopEndPosition: 0x54,
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
            loopStartPosition: 0x56,
            loopEndPosition: 0x57,
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

NumarkMixTrackPro.Stutter1Beat = function(value) {
    const secondsBlink = 30;
    const secondsToEnd =
    engine.getParameter("[Channel1]", "duration") *
    (1 - engine.getParameter("[Channel1]", "playposition"));

    if (
        secondsToEnd < secondsBlink &&
    secondsToEnd > 1 &&
    engine.getParameter("[Channel1]", "play")
    ) {
    // The song is going to end

        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[1].Cue, value);
    }
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[1].stutter, value);
};

NumarkMixTrackPro.Stutter2Beat = function(value) {
    const secondsBlink = 30;
    const secondsToEnd =
    engine.getParameter("[Channel2]", "duration") *
    (1 - engine.getParameter("[Channel2]", "playposition"));

    if (
        secondsToEnd < secondsBlink &&
    secondsToEnd > 1 &&
    engine.getParameter("[Channel2]", "play")
    ) {
    // The song is going to end

        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[2].Cue, value);
    }

    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[2].stutter, value);
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

    // First Remove event listeners
    for (let i = 1; i < 2; i++) {
        for (let x = 1; x < 4; x++) {
            // TODO: Check
            engine.makeConnection(
                `[Channel${i}]`,
                `hotcue_${x}_status`, // was _enabled
                NumarkMixTrackPro.onHotCueChange,
                true
            );
        }
        NumarkMixTrackPro.setLoopMode(i, false);
    }

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
    value,
    status,
    group
) {
    if (value > 63) {
        value = value - 128;
    }
    if (NumarkMixTrackPro.directoryMode) {
        if (value > 0) {
            for (let i = 0; i < value; i++) {
                engine.setParameter(group, "SelectNextPlaylist", 1);
            }
        } else {
            for (let i = 0; i < -value; i++) {
                engine.setParameter(group, "SelectPrevPlaylist", 1);
            }
        }
    } else {
        engine.setParameter(group, "SelectTrackKnob", value);
    }
};

NumarkMixTrackPro.LoadTrack = function(
    channel,
    control,
    value,
    status,
    group
) {
    // Load the selected track in the corresponding deck only if the track is paused

    if (value && engine.getParameter(group, "play") !== 1) {
        engine.setParameter(group, "LoadSelectedTrack", 1);

        // cargar el tema con el pitch en 0
        engine.softTakeover(group, "rate", false);
        engine.setParameter(group, "rate", 0.5);
        engine.softTakeover(group, "rate", true);
    } else {
        engine.setParameter(group, "LoadSelectedTrack", 0);
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
        if (deck === 1) {
            // si la otra deck esta en stop, sincronizo sólo el tempo (no el golpe)
            if (!engine.getParameter("[Channel2]", "play")) {
                engine.setParameter(group, "beatsync_tempo", value ? 1 : 0);
            } else {
                engine.setParameter(group, "beatsync", value ? 1 : 0);
            }
        }

        if (deck === 2) {
            // si la otra deck esta en stop, sincronizo sólo el tempo (no el golpe)
            if (!engine.getParameter("[Channel1]", "play")) {
                engine.setParameter(group, "beatsync_tempo", value ? 1 : 0);
            } else {
                engine.setParameter(group, "beatsync", value ? 1 : 0);
            }
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
    }

    if (engine.getParameter(group, "play")) {
        engine.setParameter(group, "play", 0);
    } else {
        engine.setParameter(group, "play", 1);
    }
};

NumarkMixTrackPro.loopIn = function(channel, control, value, status, group) {
    const deck = script.deckFromGroup(group);

    if (NumarkMixTrackPro.manualLoop[deck - 1]) {
        if (!value) {
            return;
        }
        // Act like the Mixxx UI
        engine.setParameter(group, "loop_in", status ? 1 : 0);
        return;
    }

    // Auto Loop: 1/2 loop size
    const start = engine.getParameter(group, "loop_start_position");
    const end = engine.getParameter(group, "loop_end_position");
    if (start < 0 || end < 0) {
        NumarkMixTrackPro.flashLED(
            NumarkMixTrackPro.leds[deck].loopStartPosition,
            4
        );
        return;
    }

    if (value) {
        const start = engine.getParameter(group, "loop_start_position");
        const end = engine.getParameter(group, "loop_end_position");
        const len = (end - start) / 2;
        engine.setParameter(group, "loop_end_position", start + len);
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loopStartPosition,
            true
        );
    } else {
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loopStartPosition,
            false
        );
    }
};

NumarkMixTrackPro.loopOut = function(channel, control, value, status, group) {
    const deck = script.deckFromGroup(group);

    if (!value) {
        return;
    }

    if (NumarkMixTrackPro.manualLoop[deck - 1]) {
    // Act like the Mixxx UI
        engine.setParameter(group, "loop_out", status ? 1 : 0);
        return;
    }

    const isLoopActive = engine.getParameter(group, "loop_enabled");

    // Set a 4 beat auto loop or exit the loop

    if (!isLoopActive) {
        engine.setParameter(group, "beatloop_4", 1);
    } else {
        engine.setParameter(group, "beatloop_4", 0);
    }
};

NumarkMixTrackPro.reLoop = function(channel, control, value, status, group) {
    const deck = script.deckFromGroup(group);

    if (NumarkMixTrackPro.manualLoop[deck - 1]) {
    // Act like the Mixxx UI (except for working delete)
        if (!value) {
            return;
        }
        if (NumarkMixTrackPro.shiftKey[deck - 1]) {
            engine.setParameter(group, "reloop_exit", 0);
            engine.setParameter(group, "loop_start_position", -1);
            engine.setParameter(group, "loop_end_position", -1);
            NumarkMixTrackPro.toggleShiftKey(channel, control, value, status, group);
        } else {
            engine.setParameter(group, "reloop_exit", status ? 1 : 0);
        }
        return;
    }

    // Auto Loop: Double Loop Size
    const start = engine.getParameter(group, "loop_start_position");
    const end = engine.getParameter(group, "loop_end_position");
    if (start < 0 || end < 0) {
        NumarkMixTrackPro.flashLED(NumarkMixTrackPro.leds[deck].reloopExit, 4);
        return;
    }

    if (value) {
        const len = (end - start) * 2;
        engine.setParameter(group, "loop_end_position", start + len);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].reloopExit, true);
    } else {
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].reloopExit, false);
    }
};

NumarkMixTrackPro.setLoopMode = function(deck, manual) {
    NumarkMixTrackPro.manualLoop[deck - 1] = manual;
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].manualLoop, !manual);
    engine.connectControl(
        `[Channel${deck}]`,
        "loop_start_position",
        "NumarkMixTrackPro.onLoopChange",
        !manual
    );
    engine.connectControl(
        `[Channel${deck}]`,
        "loop_end_position",
        "NumarkMixTrackPro.onLoopChange",
        !manual
    );
    engine.connectControl(
        `[Channel${deck}]`,
        "loop_enabled",
        "NumarkMixTrackPro.onReloopExitChange",
        !manual
    );
    engine.connectControl(
        `[Channel${deck}]`,
        "loop_enabled",
        "NumarkMixTrackPro.onReloopExitChangeAuto",
        manual
    );

    const group = `[Channel${deck}]`;
    if (manual) {
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loopStartPosition,
            engine.getParameter(group, "loop_start_position") > -1
        );
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loopEndPosition,
            engine.getParameter(group, "loop_end_position") > -1
        );
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].reloopExit,
            engine.getParameter(group, "loop_enabled")
        );
    } else {
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loopStartPosition,
            false
        );
        NumarkMixTrackPro.setLED(
            NumarkMixTrackPro.leds[deck].loopEndPosition,
            engine.getParameter(group, "loop_enabled")
        );
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].reloopExit, false);
    }
};

NumarkMixTrackPro.toggleManualLooping = function(
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

    if (NumarkMixTrackPro.shiftKey[deck - 1]) {
    // activar o desactivar quantize

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

NumarkMixTrackPro.onLoopChange = function(value, group, key) {
    const deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck][key], value > -1);
};

NumarkMixTrackPro.onReloopExitChange = function(value, group) {
    const deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].reloopExit, value);
};

NumarkMixTrackPro.onReloopExitChangeAuto = function(value, group) {
    const deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck].loopEndPosition, value);
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

    // 	if (!NumarkMixTrackPro.touch[deck-1] && !engine.getParameter(group, "play")) return;

    let adjustedJog = parseFloat(value);
    let posNeg = 1;
    if (adjustedJog > 63) {
    // Counter-clockwise
        posNeg = -1;
        adjustedJog = value - 128;
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
                    NumarkMixTrackPro.jogWheelStopScratch();
                },
                true
            );
        }
    }

    engine.scratchTick(deck, adjustedJog);

    if (engine.getParameter(group, "play")) {
        const gammaInputRange = 13; // Max jog speed
        const maxOutFraction = 0.8; // Where on the curve it should peak; 0.5 is half-way
        const sensitivity = 0.5; // Adjustment gamma
        const gammaOutputRange = 2; // Max rate change

        adjustedJog =
      posNeg *
      gammaOutputRange *
      Math.pow(
          Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction),
          sensitivity
      );
        engine.setParameter(group, "jog", adjustedJog);
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
        NumarkMixTrackPro.touch[deck - 1] = false;

        // 	paro el timer (si no existe da error mmmm) y arranco un nuevo timer.
        // 	Si en 20 milisegundos no se mueve el plato, desactiva el scratch

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
    // if playing and scratch mode is disabled, do nothing on press
        if (
            !NumarkMixTrackPro.scratchMode[deck - 1] &&
      engine.getParameter(group, "play")
        ) {
            return;
        }

        if (NumarkMixTrackPro.scratchTimer[deck - 1] !== -1) {
            engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck - 1]);
        }

        // change the 600 value for sensibility
        engine.scratchEnable(deck, 600, 33 + 1 / 3, 1.0 / 8, 1.0 / 8 / 32);

        NumarkMixTrackPro.touch[deck - 1] = true;
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
        // https://manual.mixxx.org/latest/it/chapters/appendix/mixxx_controls.html#control-[Library]-focused_widget
        /*
        if (NumarkMixTrackPro.directoryMode) {
            engine.setParameter('[Library]', 'MoveFocusBackward', 1); // [Shift+TAB]; [TAB]: MoveFocusForward
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
            engine.setParameter(group, `hotcue_${hotCue}_clear`, 1);
        }
        NumarkMixTrackPro.toggleShiftKey(channel, control, value, status, group);
    } else {
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

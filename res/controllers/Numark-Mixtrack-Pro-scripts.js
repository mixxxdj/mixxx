/*
	Numark MixTrack Pro mapping for Mixxx 2.3

	Changelog:
		- 2010-01-11 Matteo <matteo@magm3.com> created v0.1 based on Numark Mixtrack mapping script functions
		- 2011-05-08 James Ralston changed
		- 2012-05-26 Darío José Freije <dario2004@gmail.com> changed
		- 2021-02-21 Quint Guvernator <quint@guvernator.net> updated for version 2.3, fixed some broken behavior, mapped some new features

	Known Bugs:
		- Each slide/knob needs to be moved on Mixxx startup to match levels with the Mixxx UI

	Beat jump:
		The Select knob can be used to jump around the track in time with the music.

		key                     | function
		---                     | --------
		Select (turn)           | jump a set number of beats forward or backward
		Select (press and turn) | set jump amount (powers of 2)

	Effects:
		Press the Effect button to toggle effects mode.

		key                       | function
		---                       | --------
		Select (turn)             | focus effect slot
		Select (press and turn)   | switch effect in focused slot
		Select (push and release) | enable/disable effect in focused slot
		Control                   | control focused effect's Meta knob

		Outside of effects mode, the Control knob controls the first effect's meta knob.

	"Delete" keys:
		"Delete" refers to the View button on the left deck and the Tick
		button on the right deck. These buttons don't do anything alone, but
		allow access to additional functions on other buttons. You can hold
		Delete while pressing the second key; or you can press Delete first,
		followed by the second key.

		chord           | function
		-----           | --------
		Delete + Hotcue | delete hotcue
		Delete + Sync   | restore original song tempo
		Delete + Manual | enable/disable quantization
		Delete + Cue    | start playback from the beginning of the track
		Delete + Play   | start/stop the player with a vinyl speed-up/slow-down effect
		Delete + Delete | do nothing

	Pitch bend:
		The pitch bend buttons increase or decrease the song pitch by one
		semitone without affecting the set tempo. Press both buttons together
		to reset to the original song pitch.

	Load track: 	Only if the track is paused. Put the pitch in 0 at load.

	Gain: 	The 3rd knob of the "effect" section is "Gain" (up to clip).

	Cue: 	Don't set Cue accidentally at the end of the song (return to the lastest cue).
		LED ON when stopped. LED OFF when playing.
		LED Blink at Beat time in the ultimates 30 seconds of song.

	Stutter: Adjust BeatGrid in the correct place (useful to sync well).
		 LED Blink at each Beat of the grid.

	Sync:	If the other deck is stopped, only sync tempo (not fase).
		LED Blink at Clip Gain (Peak indicator).

	Pitch: 	Up, Up; Down, Down. Pitch slide are inverted, to match with the screen (otherwise is very confusing).
		Soft-takeover to prevent sudden wide parameter changes when the on-screen control diverges from a hardware control.
		The control will have no effect until the position is close to that of the software,
		at which point it will take over and operate as usual.

	Auto Loop (LED ON): 	Active at program Start.
				"1 Bar" button: Active an Instant 4 beat Loop. Press again to exit loop.

	Scratch:
	In Stop mode, with Scratch OFF or ON: 	Scratch at touch, and Stop moving when the wheel stop moving.
	In Play mode, with Scratch OFF: 	Only Pitch bend.
	In Play mode, with Scratch ON: 		Scratch at touch and, in Backwards Stop Scratch when the wheel stop moving for 20ms -> BACKSPIN EFFECT!!!!.
						In Fordward Stop Scratch when the touch is released > Play Inmediatly (without breaks for well mix).
						Border of the wheels: Pitch Bend.
*/


var NumarkMixTrackPro = function() {};

NumarkMixTrackPro.init = function(id) {	// called when the MIDI device is opened & set up
    NumarkMixTrackPro.id = id;	// Store the ID of this device for later use

    NumarkMixTrackPro.directoryMode = false;
    NumarkMixTrackPro.scratchMode = [false, false];
    NumarkMixTrackPro.manualLoop = [true, true];

    NumarkMixTrackPro.effectMode = [false, false];
    NumarkMixTrackPro.topSelectHeld = [false, false];
    NumarkMixTrackPro.effectSwitched = [false, false];
    NumarkMixTrackPro.isKeyLocked = [0, 0];
    NumarkMixTrackPro.touch = [false, false];
    NumarkMixTrackPro.scratchTimer = [-1, -1];
    NumarkMixTrackPro.plusKeyPressed = [false, false];
    NumarkMixTrackPro.minusKeyPressed = [false, false];

    // inactive, held_unused, held_unused, active
    NumarkMixTrackPro.deleteKey = ["inactive", "inactive"];

    NumarkMixTrackPro.leds = [
        // Common
        {"directory": 0x73, "file": 0x72},
        // Deck 1
        {
            "rate": 0x70, "scratchMode": 0x48, "manualLoop": 0x61,
            "loop_start_position": 0x53, "loop_end_position": 0x54, "reloop_exit": 0x55,
            "deleteKey": 0x59, "hotCue1": 0x5a, "hotCue2": 0x5b, "hotCue3": 0x5c,
            "effectKey": 0x63, "stutter": 0x4a, "Cue": 0x33, "sync": 0x40
        },
        // Deck 2
        {
            "rate": 0x71, "scratchMode": 0x50, "manualLoop": 0x62,
            "loop_start_position": 0x56, "loop_end_position": 0x57, "reloop_exit": 0x58,
            "deleteKey": 0x5d, "hotCue1": 0x5e, "hotCue2": 0x5f, "hotCue3": 0x60,
            "effectKey": 0x64, "stutter": 0x4c, "Cue": 0x3c, "sync": 0x47
        }
    ];

    /*
		33 cue L
		3c cue R

		40 sync L
		47 sync R

		48 scratch L
		4a stutter L
		4c stutter R
		50 scratch R

		53 start L
		54 end L
		55 reloop L
		56 start R
		57 end R
		58 reloop R

		59 delete L
		5a cue1 L
		5b cue2 L
		5c cue3 L
		5d delete R
		5e cue1 R
		5f cue2 R
		60 cue3 R

		61 manual L
		62 manual R

		63 effect L
		64 effect R

		70 rate L
		71 rate R

		72 file
		73 directory
	*/

    NumarkMixTrackPro.ledTimers = {};

    NumarkMixTrackPro.LedTimer = function(id, led, count, state) {
        this.id = id;
        this.led = led;
        this.count = count;
        this.state = state;
    };

    for (i=0x30; i<=0x73; i++) midi.sendShortMsg(0x90, i, 0x00); 	// Turn off all the lights

    NumarkMixTrackPro.hotCue = {
        //Deck 1
        0x5a: "1", 0x5b: "2", 0x5c: "3",
        //Deck 2
        0x5e: "1", 0x5f: "2", 0x60: "3"
    };

    //Add event listeners
    for (var i=1; i<3; i++) {
        for (var x=1; x<4; x++) {
            engine.connectControl("[Channel" + i +"]", "hotcue_"+ x +"_enabled", "NumarkMixTrackPro.onHotCueChange");
        }
        NumarkMixTrackPro.setLoopMode(i, false);
    }

    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[0]["file"], true);


    // Enable soft-takeover for Pitch slider

    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);


    // Clipping LED
    engine.connectControl("[Channel1]", "PeakIndicator", "NumarkMixTrackPro.Channel1Clip");
    engine.connectControl("[Channel2]", "PeakIndicator", "NumarkMixTrackPro.Channel2Clip");

    // Stutter beat light
    engine.connectControl("[Channel1]", "beat_active", "NumarkMixTrackPro.Stutter1Beat");
    engine.connectControl("[Channel2]", "beat_active", "NumarkMixTrackPro.Stutter2Beat");


};

NumarkMixTrackPro.deleteMode = function(deck) {
    var deleteKey = NumarkMixTrackPro.deleteKey[deck - 1];
    var chorded = deleteKey !== "inactive";

    switch (deleteKey) {
    case "active":
        deleteKey = "inactive";
        break;
    case "held_unused":
        deleteKey = "held_used";
        break;
    }

    NumarkMixTrackPro.deleteKey[deck - 1] = deleteKey;
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["deleteKey"], deleteKey !== "inactive");
    return chorded;
};

NumarkMixTrackPro.Channel1Clip = function(value) {
    NumarkMixTrackPro.clipLED(value, NumarkMixTrackPro.leds[1]["sync"]);

};

NumarkMixTrackPro.Channel2Clip = function(value) {
    NumarkMixTrackPro.clipLED(value, NumarkMixTrackPro.leds[2]["sync"]);

};

NumarkMixTrackPro.Stutter1Beat = function(value) {

    var secondsBlink = 30;
    var secondsToEnd = engine.getValue("[Channel1]", "duration") * (1-engine.getValue("[Channel1]", "playposition"));

    if (secondsToEnd < secondsBlink && secondsToEnd > 1 && engine.getValue("[Channel1]", "play")) { // The song is going to end

        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[1]["Cue"], value);
    }
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[1]["stutter"], value);

};

NumarkMixTrackPro.Stutter2Beat = function(value) {

    var secondsBlink = 30;
    var secondsToEnd = engine.getValue("[Channel2]", "duration") * (1-engine.getValue("[Channel2]", "playposition"));

    if (secondsToEnd < secondsBlink && secondsToEnd > 1 && engine.getValue("[Channel2]", "play")) { // The song is going to end

        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[2]["Cue"], value);
    }

    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[2]["stutter"], value);

};

NumarkMixTrackPro.clipLED = function(value, note) {

    if (value>0) NumarkMixTrackPro.flashLED(note, 1);

};

NumarkMixTrackPro.shutdown = function(_id) {	// called when the MIDI device is closed

    // First Remove event listeners
    var i;
    for (i=1; i<2; i++) {
        for (var x=1; x<4; x++) {
            engine.connectControl("[Channel" + i +"]", "hotcue_"+ x +"_enabled", "NumarkMixTrackPro.onHotCueChange", true);
        }
        NumarkMixTrackPro.setLoopMode(i, false);
    }

    var lowestLED = 0x30;
    var highestLED = 0x73;
    for (i=lowestLED; i<=highestLED; i++) {
        NumarkMixTrackPro.setLED(i, false);	// Turn off all the lights
    }

};

NumarkMixTrackPro.samplesPerBeat = function(group) {
    // FIXME: Get correct samplerate and channels for current deck
    var sampleRate = 44100;
    var channels = 2;
    var bpm = engine.getValue(group, "file_bpm");
    return channels * sampleRate * 60 / bpm;
};

NumarkMixTrackPro.groupToDeck = function(group) {

    var matches = group.match(/^\[Channel(\d+)\]$/);

    if (matches === null) {
        return -1;
    } else {
        return matches[1];
    }

};

NumarkMixTrackPro.setLED = function(value, status) {

    status = status ? 0x64 : 0x00;
    midi.sendShortMsg(0x90, value, status);
};

NumarkMixTrackPro.flashLED = function(led, veces) {
    var ndx = Math.random();
    var func = "NumarkMixTrackPro.doFlash(" + ndx + ", " + veces + ")";
    var id = engine.beginTimer(120, func);
    NumarkMixTrackPro.ledTimers[ndx] =  new NumarkMixTrackPro.LedTimer(id, led, 0, false);
};

NumarkMixTrackPro.doFlash = function(ndx, veces) {
    var ledTimer = NumarkMixTrackPro.ledTimers[ndx];

    if (!ledTimer) return;

    if (ledTimer.count > veces) { // how many times blink the button
        engine.stopTimer(ledTimer.id);
        delete NumarkMixTrackPro.ledTimers[ndx];
    } else {
        ledTimer.count++;
        ledTimer.state = !ledTimer.state;
        NumarkMixTrackPro.setLED(ledTimer.led, ledTimer.state);
    }
};

NumarkMixTrackPro.selectKnob = function(channel, control, value, status, group) {
    var i;
    if (value > 63) {
        value = value - 128;
    }
    if (NumarkMixTrackPro.directoryMode) {
        if (value > 0) {
            for (i = 0; i < value; i++) {
                engine.setValue(group, "SelectNextPlaylist", 1);
            }
        } else {
            for (i = 0; i < -value; i++) {
                engine.setValue(group, "SelectPrevPlaylist", 1);
            }
        }
    } else {
        engine.setValue(group, "SelectTrackKnob", value);

    }
};

NumarkMixTrackPro.LoadTrack = function(channel, control, value, status, group) {

    // Load the selected track in the corresponding deck only if the track is paused

    if (value && engine.getValue(group, "play") !== 1) {
        engine.setValue(group, "LoadSelectedTrack", 1);

        // cargar el tema con el pitch en 0
        engine.softTakeover(group, "rate", false);
        engine.setValue(group, "rate", 0);
        engine.softTakeover(group, "rate", true);
    } else engine.setValue(group, "LoadSelectedTrack", 0);

};

NumarkMixTrackPro.cuebutton = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);

    if (NumarkMixTrackPro.deleteMode(deck)) {
        if (value) {
            script.triggerControl(group, "start_play");
        }

        // Don't set Cue accidentally at the end of the song
    } else if (engine.getValue(group, "playposition") <= 0.97) {
        engine.setValue(group, "cue_default", value ? 1 : 0);

    } else {
        engine.setValue(group, "cue_preview", value ? 1 : 0);
    }

};

NumarkMixTrackPro.beatsync = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);

    // Delete + SYNC = vuelve pitch a 0
    if (NumarkMixTrackPro.deleteMode(deck)) {
        engine.softTakeover(group, "rate", false);
        engine.setValue(group, "rate", 0);
        engine.softTakeover(group, "rate", true);

        // si la otra deck esta en stop, sincronizo sólo el tempo (no el golpe)
    } else if (!engine.getValue("[Channel"+ (deck === 1 ? 2 : 1) + "]", "play")) {
        engine.setValue(group, "beatsync_tempo", value ? 1 : 0);

        // sincronizo todos
    } else {
        engine.setValue(group, "beatsync", value ? 1 : 0);
    }
};


NumarkMixTrackPro.playbutton = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = script.deckFromGroup(group);
    var playing = engine.getValue(group, "play");

    if (NumarkMixTrackPro.deleteMode(deck)) {
        if (playing) {
            engine.brake(deck, true, 20);
        } else {
            engine.softStart(deck, true, 20);
        }

    } else {
        engine.setValue(group, "play", !playing);
    }

};

NumarkMixTrackPro.loop = function(type, pressed, group) {
    var deck = script.deckFromGroup(group);

    switch (type) {
    case "half":
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], pressed);
        if (pressed) {
            script.triggerControl(group, "loop_halve");
        }
        break;

    case "1bar":
        if (pressed) {
            var isLooping = engine.getValue(group, "loop_enabled");
            if (isLooping) {
                script.triggerControl(group, "reloop_toggle");
            } else {
                engine.setValue(group, "beatloop_size", 4);
                script.triggerControl(group, "beatloop_activate");
            }
        }
        break;

    case "double":
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], pressed);
        if (pressed) {
            script.triggerControl(group, "loop_double");
        }
        break;

    case "in":
        if (pressed) {
            script.triggerControl(group, "loop_in");
        }
        break;
    case "out":
        if (pressed) {
            script.triggerControl(group, "loop_out");
        }
        break;
    case "reloop":
        if (pressed) {
            script.triggerControl(group, "reloop_toggle");
        }
        break;
    }
};

NumarkMixTrackPro.loopIn = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var manualMode = NumarkMixTrackPro.manualLoop[deck-1];
    NumarkMixTrackPro.loop(manualMode ? "in" : "half", !!value, group);
};

NumarkMixTrackPro.loopOut = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var manualMode = NumarkMixTrackPro.manualLoop[deck-1];
    NumarkMixTrackPro.loop(manualMode ? "out" : "1bar", !!value, group);
};

NumarkMixTrackPro.repositionHack = function(group, oldPosition) {
    // see if the value has been updated
    if (engine.getValue(group, "loop_start_position") === oldPosition) {
        if (NumarkMixTrackPro.hackCount[group]++ < 9) {
            engine.beginTimer(20, "NumarkMixTrackPro.repositionHack('" + group + "', " + oldPosition + ")", true);
        } else {
            var deck = script.deckFromGroup(group);
            NumarkMixTrackPro.flashLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], 4);
        }
        return;
    }
    var bar = NumarkMixTrackPro.samplesPerBeat(group);
    var start = engine.getValue(group, "loop_start_position");
    engine.setValue(group, "loop_end_position", start + bar);
};

NumarkMixTrackPro.reLoop = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var manualMode = NumarkMixTrackPro.manualLoop[deck-1];
    NumarkMixTrackPro.loop(manualMode ? "reloop" : "double", !!value, group);
};

NumarkMixTrackPro.setLoopMode = function(deck, manual) {

    NumarkMixTrackPro.manualLoop[deck-1] = manual;
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["manualLoop"], !manual);
    engine.connectControl("[Channel" + deck + "]", "loop_start_position", "NumarkMixTrackPro.onLoopChange", !manual);
    engine.connectControl("[Channel" + deck + "]", "loop_end_position", "NumarkMixTrackPro.onLoopChange", !manual);
    engine.connectControl("[Channel" + deck + "]", "loop_enabled", "NumarkMixTrackPro.onReloopExitChange", !manual);
    engine.connectControl("[Channel" + deck + "]", "loop_enabled", "NumarkMixTrackPro.onReloopExitChangeAuto", manual);

    var group = "[Channel" + deck + "]";
    if (manual) {
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], engine.getValue(group, "loop_start_position")>-1);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_end_position"], engine.getValue(group, "loop_end_position")>-1);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], engine.getValue(group, "loop_enabled"));
    } else {
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], false);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_end_position"], engine.getValue(group, "loop_enabled"));
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], false);
    }
};

NumarkMixTrackPro.toggleManualLooping = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = script.deckFromGroup(group);

    // activar o desactivar quantize
    if (NumarkMixTrackPro.deleteMode(deck)) {
        if (engine.getValue(group, "quantize")) {
            engine.setValue(group, "quantize", 0);
        } else {
            engine.setValue(group, "quantize", 1);
        }

    } else {
        NumarkMixTrackPro.setLoopMode(deck, !NumarkMixTrackPro.manualLoop[deck-1]);
    }

};

NumarkMixTrackPro.onLoopChange = function(value, group, key) {
    var deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck][key], value>-1);
};

NumarkMixTrackPro.onReloopExitChange = function(value, group, _key) {
    var deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], value);
};

NumarkMixTrackPro.onReloopExitChangeAuto = function(value, group, _key) {
    var deck = script.deckFromGroup(group);
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_end_position"], value);
};

// Stutters adjust BeatGrid
NumarkMixTrackPro.playFromCue = function(channel, control, value, status, group) {

    var deck = script.deckFromGroup(group);

    if (engine.getValue(group, "beats_translate_curpos")) {

        engine.setValue(group, "beats_translate_curpos", 0);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["stutter"], 0);
    } else {
        engine.setValue(group, "beats_translate_curpos", 1);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["stutter"], 1);
    }

};

NumarkMixTrackPro.pitch = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);

    var pitchValue = 0;

    if (value < 64) pitchValue = (value-64) /64;
    if (value > 64) pitchValue = (value-64) /63;

    engine.setValue("[Channel"+deck+"]", "rate", pitchValue);
};


NumarkMixTrackPro.jogWheel = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);

    // 	if (!NumarkMixTrackPro.touch[deck-1] && !engine.getValue(group, "play")) return;

    var adjustedJog = parseFloat(value);
    var posNeg = 1;
    if (adjustedJog > 63) {	// Counter-clockwise
        posNeg = -1;
        adjustedJog = value - 128;
    }

    if (engine.getValue(group, "play")) {

        if (NumarkMixTrackPro.scratchMode[deck-1] && posNeg === -1 && !NumarkMixTrackPro.touch[deck-1]) {

            if (NumarkMixTrackPro.scratchTimer[deck-1] !== -1) engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck-1]);
            NumarkMixTrackPro.scratchTimer[deck-1] = engine.beginTimer(20, "NumarkMixTrackPro.jogWheelStopScratch(" + deck + ")", true);
        }

    } else { // en stop hace scratch siempre

        if (!NumarkMixTrackPro.touch[deck-1]) {

            if (NumarkMixTrackPro.scratchTimer[deck-1] !== -1) engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck-1]);
            NumarkMixTrackPro.scratchTimer[deck-1] = engine.beginTimer(20, "NumarkMixTrackPro.jogWheelStopScratch(" + deck + ")", true);
        }

    }

    engine.scratchTick(deck, adjustedJog);

    if (engine.getValue(group, "play")) {
        var gammaInputRange = 13;	// Max jog speed
        var maxOutFraction = 0.8;	// Where on the curve it should peak; 0.5 is half-way
        var sensitivity = 0.5;		// Adjustment gamma
        var gammaOutputRange = 2;	// Max rate change

        adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
        engine.setValue(group, "jog", adjustedJog);
    }

};


NumarkMixTrackPro.jogWheelStopScratch = function(deck) {
    NumarkMixTrackPro.scratchTimer[deck-1] = -1;
    engine.scratchDisable(deck);
};

NumarkMixTrackPro.wheelTouch = function(channel, control, value, status, group) {

    var deck = script.deckFromGroup(group);

    if (!value) {

        NumarkMixTrackPro.touch[deck-1]= false;

        // 	paro el timer (si no existe da error mmmm) y arranco un nuevo timer.
        // 	Si en 20 milisegundos no se mueve el plato, desactiva el scratch

        if (NumarkMixTrackPro.scratchTimer[deck-1] !== -1) engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck-1]);

        NumarkMixTrackPro.scratchTimer[deck-1] = engine.beginTimer(20, "NumarkMixTrackPro.jogWheelStopScratch(" + deck + ")", true);

    } else {

        // si esta en play y el modo scratch desactivado, al presionar el touch no hace nada
        if (!NumarkMixTrackPro.scratchMode[deck-1] && engine.getValue(group, "play")) return;

        if (NumarkMixTrackPro.scratchTimer[deck-1] !== -1) engine.stopTimer(NumarkMixTrackPro.scratchTimer[deck-1]);

        // change the 600 value for sensibility
        engine.scratchEnable(deck, 600, 33+1/3, 1.0/8, (1.0/8)/32);

        NumarkMixTrackPro.touch[deck-1]= true;
    }
};

NumarkMixTrackPro.toggleDirectoryMode = function(channel, control, value, _status, _group) {
    // Toggle setting and light
    if (value) {
        NumarkMixTrackPro.directoryMode = !NumarkMixTrackPro.directoryMode;

        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[0]["directory"], NumarkMixTrackPro.directoryMode);
        NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[0]["file"], !NumarkMixTrackPro.directoryMode);
    }
};

NumarkMixTrackPro.toggleScratchMode = function(channel, control, value, status, group) {
    if (!value) return;

    var deck = script.deckFromGroup(group);
    // Toggle setting and light
    NumarkMixTrackPro.scratchMode[deck-1] = !NumarkMixTrackPro.scratchMode[deck-1];
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["scratchMode"], NumarkMixTrackPro.scratchMode[deck-1]);
};


NumarkMixTrackPro.onHotCueChange = function(value, group, key) {
    var deck = script.deckFromGroup(group);
    var hotCueNum = key[7];
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["hotCue" + hotCueNum], !!value);
};

NumarkMixTrackPro.changeHotCue = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var hotCue = NumarkMixTrackPro.hotCue[control];

    // onHotCueChange called automatically
    if (NumarkMixTrackPro.deleteMode(deck)) {
        if (engine.getValue(group, "hotcue_" + hotCue + "_enabled")) {
            engine.setValue(group, "hotcue_" + hotCue + "_clear", 1);
        }

    } else {
        engine.setValue(group, "hotcue_" + hotCue + "_activate", value ? 1 : 0);
    }
};

NumarkMixTrackPro.effectPress = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var effectMode = NumarkMixTrackPro.effectMode[deck - 1];

    if (value) {
        effectMode = !effectMode;
    }

    NumarkMixTrackPro.effectMode[deck - 1] = effectMode;
    engine.setParameter("[EffectRack1_EffectUnit" + deck + "]", "show_focus", effectMode);
    engine.setParameter("[EffectRack1_EffectUnit" + deck + "]", "focused_effect", 1);
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["effectKey"], effectMode);
};

NumarkMixTrackPro.toggleDeleteKey = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var deleteKey = NumarkMixTrackPro.deleteKey[deck - 1];
    switch (deleteKey) {
    case "inactive":
        if (value) {
            deleteKey = "held_unused";
        }
        break;

    case "held_unused":
        if (!value) {
            deleteKey = "active";
        }
        break;

    case "active":
        if (value) {
            deleteKey = "inactive";
        }
        break;

    case "held_used":
        if (!value) {
            deleteKey = "inactive";
        }
        break;
    }

    NumarkMixTrackPro.deleteKey[deck - 1] = deleteKey;
    NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["deleteKey"], deleteKey !== "inactive");
};

NumarkMixTrackPro.topSelectClick = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var effectMode = NumarkMixTrackPro.effectMode[deck - 1];
    var effect = engine.getParameter("[EffectRack1_EffectUnit" + deck + "]", "focused_effect");

    if (!effectMode && value) {
        engine.setValue(group, "beatjump_size", 4);
    }

    NumarkMixTrackPro.topSelectHeld[deck - 1] = !!value;

    if (value) {
        NumarkMixTrackPro.effectSwitched[deck - 1] = false;
    }

    // if it's been released before rotating, then enable the current effect
    if (effectMode && !value && !NumarkMixTrackPro.effectSwitched[deck - 1]) {
        script.toggleControl("[EffectRack1_EffectUnit" + deck + "_Effect" + effect + "]", "enabled");
    }
};

NumarkMixTrackPro.topSelectTurn = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var held = NumarkMixTrackPro.topSelectHeld[deck - 1];
    var effectMode = NumarkMixTrackPro.effectMode[deck - 1];

    if (value > 63) {
        value = value - 128;
    }

    if (effectMode) {
        var effect = engine.getParameter("[EffectRack1_EffectUnit" + deck + "]", "focused_effect");

        // change effect type
        if (held) {
            var key = "[EffectRack1_EffectUnit" + deck + "_Effect" + effect + "]";
            script.triggerControl(key, value > 0 ? "prev_effect" : "next_effect");

            // select effect
        } else {
            var numEffects = engine.getValue("[EffectRack1_EffectUnit" + deck + "]", "num_effects");
            effect += value > 0 ? -1 : 1;

            if (effect > numEffects) {
                effect = numEffects;
            } else if (effect < 1) {
                effect = 1;
            }

            NumarkMixTrackPro.effectSwitched[deck - 1] = true;
            engine.setParameter("[EffectRack1_EffectUnit" + deck + "]", "focused_effect", effect);
        }

    } else {
        if (held) {
            var beatjumpOld = engine.getValue(group, "beatjump_size");
            engine.setValue(group, "beatjump_size", value > 0 ? beatjumpOld / 2 : beatjumpOld * 2);
        } else {
            script.triggerControl(group, value > 0 ? "beatjump_backward" : "beatjump_forward");
        }
    }
};

NumarkMixTrackPro.songPitch = function(group, newMinus, newPlus) {
    var deck = script.deckFromGroup(group);
    var oldMinus = NumarkMixTrackPro.minusKeyPressed[deck - 1];
    var oldPlus = NumarkMixTrackPro.plusKeyPressed[deck - 1];
    var pitch = engine.getValue(group, "pitch");

    if (newMinus === null) {
        newMinus = oldMinus;
    }
    if (newPlus === null) {
        newPlus = oldPlus;
    }

    if (newMinus && newPlus) {
        engine.setValue(group, "pitch", 0);
        newMinus = false;
        newPlus = false;

    } else if (oldMinus && !newMinus) {
        engine.setValue(group, "pitch", pitch - 0.5);

    } else if (oldPlus && !newPlus) {
        engine.setValue(group, "pitch", pitch + 0.5);
    }

    NumarkMixTrackPro.minusKeyPressed[deck - 1] = newMinus;
    NumarkMixTrackPro.plusKeyPressed[deck - 1] = newPlus;
};

NumarkMixTrackPro.plus = function(channel, control, value, status, group) {
    return NumarkMixTrackPro.songPitch(group, null, value);
};

NumarkMixTrackPro.minus = function(channel, control, value, status, group) {
    return NumarkMixTrackPro.songPitch(group, value, null);
};

NumarkMixTrackPro.controlKnob = function(channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);

    if (value > 63) {
        value = value - 128;
    }

    var effect = engine.getParameter("[EffectRack1_EffectUnit" + deck + "]", "focused_effect");
    var key = "[EffectRack1_EffectUnit" + deck + "_Effect" + effect + "]";
    for (var i = 0; i < Math.abs(value); i++) {
        script.triggerControl(key, value > 0 ? "meta_up_small" : "meta_down_small");
    }
};

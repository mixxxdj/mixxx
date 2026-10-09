////////////////////////////////////////////////////////////////////////
// Controller: Denon HC1000S
////////////////////////////////////////////////////////////////////////
var DenonHC1000S = {};

////////////////////////////////////////////////////////////////////////

// SERATO-Certification ->
//The SysEx message to send to the controller to force the midi controller
// to send the status of every item on the control surface.
var ControllerStatusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];

/*
DN-HC1000S todo notes:
4 Deck support (with C+D)
optional (local):
DVS control w/TraktorSoundCard Ctrl (THRU)
------
[done] LED for Sampler/Play/Cue if 2nd Shift are pressed
[done] DVS control software (place these functions somewhere)
[done] 	Play & Cue: 	use LoadSample button as 2nd Shift with Cue1/2 buttons
[done] RecordButton (shift TrackSelect)
[done]SamplerLEDs: overwrite HotCue Lights with SamplerLoaded-Status if "load" buttons are pressed
[done]AutoLoop: make via script to handle LED (off if exiting via Loop);
[done]LEDs: explore binding hotcues and loop buttons so the lights react on the actual actual status (e.g. if changed via screen UI)
[done]LoopLED: reset on new track
[done]LED blinking if r/l-shift pressed and sample loaded
[done] LoopCut Wheel: add secondary dial (if pressed) to move loop right/left
[done] Samples: use LoadSample buttons as 2nd Shift for Cue3/4/5
[done] Tap
[done] Fwd/Back button
[done] Instant Double (shift load track support)
 */

/////INIT///////

DenonHC1000S.init = function(id, debug) {
    DenonHC1000S.id = id;
    DenonHC1000S.debug = debug;
    // create an instance of your custom Deck object for each side of your controller
    DenonHC1000S.leftDeck = new DenonHC1000S.Deck([1, 3], 1);
    DenonHC1000S.rightDeck = new DenonHC1000S.Deck([2, 4], 2);

    engine.setValue("[Master]", "num_decks", 4);
    var leds = [0x11, 0x13, 0x15, 0x17, 0x19, /* cues deckA*/
        0x24, 0x40, 0x2B, /* loops deckA*/
        0x09, /* sync deckA*/
        0x61, 0x63, 0x65, 0x67, 0x69, /* cues deckB*/
        0x6B, 0x6D, 0x6F, /* loops deckB*/
        0x60 /* sync deckB*/
    ];
    for (var index = 0, count = leds.length; index < count; index++) {

        midi.sendShortMsg(0xB0, 0x4C, leds[index]); //Channel1
        midi.sendShortMsg(0xB1, 0x4C, leds[index]); //Channel2
        midi.sendShortMsg(0xB2, 0x4C, leds[index]); //Channel3
        midi.sendShortMsg(0xB3, 0x4C, leds[index]); //Channel4

        midi.sendShortMsg(0xB0, 0x4B, leds[index]); //Channel1
        midi.sendShortMsg(0xB1, 0x4B, leds[index]); //Channel2
        midi.sendShortMsg(0xB2, 0x4B, leds[index]); //Channel3
        midi.sendShortMsg(0xB3, 0x4B, leds[index]); //Channel4
    }

    for (var i = 1; i <= 4; i++) {
        engine.makeConnection("[Channel" + i + "]", "beat_active", DenonHC1000S.onBeatFlash);
        engine.makeConnection("[Channel" + i + "]", "beatloop_activate", DenonHC1000S.ledLoop);
        engine.makeConnection("[Channel" + i + "]", "beatlooproll_activate", DenonHC1000S.ledLoop);
        engine.makeConnection("[Channel" + i + "]", "loop_enabled", DenonHC1000S.ledLoop);
        engine.makeConnection("[Channel" + i + "]", "loop_in", DenonHC1000S.ledLoop);
        engine.makeConnection("[Channel" + i + "]", "loop_out", DenonHC1000S.ledLoop);
        engine.makeConnection("[Channel" + i + "]", "track_loaded", DenonHC1000S.newTrackLoaded);
        for (var j = 1; j <= 5; j++) {
            engine.makeConnection("[Channel" + i + "]", "hotcue_" + j + "_enabled", DenonHC1000S.ledHotCueAndSampler);
        }
    }

    for (i = 1; i <= 6; i++) {
        engine.makeConnection("[Sampler" + i + "]", "track_loaded", DenonHC1000S.ledHotCueAndSampler);
    }
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
};

////////////////////////////////////////////////////////////////////////
// Buttons                                                            //
////////////////////////////////////////////////////////////////////////

DenonHC1000S.MIDI_BUTTON_ON = 0x40;
DenonHC1000S.MIDI_BUTTON_OFF = 0x00;

DenonHC1000S.isButtonPressed = function(midiValue) {
    switch (midiValue) {
    case DenonHC1000S.MIDI_BUTTON_ON:
        return true;
    case DenonHC1000S.MIDI_BUTTON_OFF:
        return false;
    default:
        DenonHC1000S.logError("Unexpected MIDI button value: " + midiValue);
        return undefined;
    }
};

////////////////////////////////////////////////////////////////////////
// Knobs                                                              //
////////////////////////////////////////////////////////////////////////

DenonHC1000S.MIDI_KNOB_INC = 0x00;
DenonHC1000S.MIDI_KNOB_DEC = 0x7F;

DenonHC1000S.getKnobDelta = function(midiValue) {
    switch (midiValue) {
    case DenonHC1000S.MIDI_KNOB_INC:
        return 1;
    case DenonHC1000S.MIDI_KNOB_DEC:
        return -1;
    default:
        DenonHC1000S.logError("Unexpected MIDI knob value: " + midiValue);
        return 0;
    }
};

DenonHC1000S.shift = false;
DenonHC1000S.shiftButton = function(channel, control, value, _status, _group) {
    // Note that there is no 'if (value === 127)' here so this executes both when the shift button is pressed and when it is released.
    // Therefore, DenonHC1000S.shift will only be true while the shift button is held down
    DenonHC1000S.shift = !DenonHC1000S.shift; //'!' inverts a boolean (true/false) value
    print("Shift is pressed " + DenonHC1000S.shift);
    if ((DenonHC1000S.shiftSampleLeft && DenonHC1000S.shiftSampleRight && DenonHC1000S.shift) && (value === 64)) {
        var samplersShow = engine.getValue("[Samplers]", "show_samplers");
        print("want to show Samplers in UI; samplersShow is " + samplersShow);
        engine.setValue("[Samplers]", "show_samplers", !samplersShow);
    }
};

DenonHC1000S.shiftSampleLeft = false;
DenonHC1000S.shiftSampleLeftButton = function(channel, control, value, status, group) {
    DenonHC1000S.shiftSampleLeft = !DenonHC1000S.shiftSampleLeft; // '!' inverts a boolean (true/false) value
    print("shiftSampleLeft is pressed " + DenonHC1000S.shiftSampleLeft);
    DenonHC1000S.ledHotCueAndSampler(channel, control, value, status, group);
    //DenonHC1000S.LED_Play_and_Cue(channel, control, value, status, group);
    if ((DenonHC1000S.shiftSampleLeft && DenonHC1000S.shiftSampleRight && DenonHC1000S.shift) && (value === 64)) {
        var samplersShow = engine.getValue("[Samplers]", "show_samplers");
        print("want to show Samplers in UI; samplersShow is " + samplersShow);
        engine.setValue("[Samplers]", "show_samplers", !samplersShow);
    }
};

DenonHC1000S.shiftSampleRight = false;
DenonHC1000S.shiftSampleRightButton = function(channel, control, value, status, group) {
    DenonHC1000S.shiftSampleRight = !DenonHC1000S.shiftSampleRight; // '!' inverts a boolean (true/false) value
    print("shiftSampleRight is pressed " + DenonHC1000S.shiftSampleRight);
    DenonHC1000S.ledHotCueAndSampler(channel, control, value, status, group);
    if ((DenonHC1000S.shiftSampleLeft && DenonHC1000S.shiftSampleRight && DenonHC1000S.shift) && (value === 64)) {
        var samplersShow = engine.getValue("[Samplers]", "show_samplers");
        print("want to show Samplers in UI; samplersShow is " + samplersShow);
        engine.setValue("[Samplers]", "show_samplers", !samplersShow);
    }
};

///////

DenonHC1000S.getValue = function(key) {
    return engine.getValue(DenonHC1000S.group, key);
};

DenonHC1000S.setValue = function(key, value) {
    engine.setValue(DenonHC1000S.group, key, value);
};

DenonHC1000S.samplesPerBeat = function(group) {
    return 2 * engine.getValue(group, "track_samplerate") * 60 / engine.getValue(group, "file_bpm");
};

//==TrackSelect===================================================================

DenonHC1000S.recvTrackSelectKnob = function(_channel, _control, value, _status) {
    var knobDelta = DenonHC1000S.getKnobDelta(value);
    var ctrlName = DenonHC1000S.shift ? "ScrollVertical" : "MoveVertical";
    engine.setValue("[Library]", ctrlName, knobDelta);
};

DenonHC1000S.recvTrackSelectButton = function(_channel, _control, value, _status) {
    var buttonPressed = DenonHC1000S.isButtonPressed(value);
    if (!buttonPressed) {
        return;
    }
    if ((DenonHC1000S.shiftSampleLeft && DenonHC1000S.shiftSampleRight) && (value === 64)) {
        print("want to toggle maximize Library");
        var libraryMaximize = engine.getValue("[Master]", "maximize_library");
        engine.setValue("[Master]", "maximize_library", !libraryMaximize);
    } else {
        if (DenonHC1000S.shift) {
            engine.setValue("[Recording]", "toggle_recording", true);
        } else {
            var ctrlName = "GoToItem";
            engine.setValue("[Library]", ctrlName, true);
        }
    }
};
DenonHC1000S.resizeLoopButton = function(channel, control, value, status, group) {
    DenonHC1000S.resizeLoopButtonPressed = !DenonHC1000S.resizeLoopButtonPressed; // '!' inverts a boolean (true/false) value
    if (DenonHC1000S.resizeLoopButtonPressed && (value === 64)) {
        DenonHC1000S.resizeLoopButtonPressed = true; // sets boolean
        print(group + " resizeLoop Wheel-Button pressed is " + DenonHC1000S.resizeLoopButtonPressed);
    }
};

DenonHC1000S.getDeckByGroup = function(group) {
    for (var index = 0, count = decks.length; index < count; index++) {
        if (decks[index].group === group) {
            return decks[index];
        }
    }
    return null;
};

DenonHC1000S.groupToDeck = function(group) {
    var matches = group.match(/^\[Channel(\d+)\]$/);
    if (matches === null) {
        return -1;
    } else {
        return matches[1];
    }
};

DenonHC1000S.getDeckByInputValue = function(group) {
    for (var index = 0, count = decks.length; index < count; index++) {
        if (decks[index].group === group) {
            return decks[index];
        }
    }
    return null;
};

DenonHC1000S.Deck = function(deckNumber, group) {
    this.deckNumber = deckNumber;
    this.group = group;
    this.scratchMode = false;
    this.controlPressed = -1;
};

DenonHC1000S.loopOrHotcues = function(midino, control, value, status, group) {
    var deck = DenonHC1000S.getDeckByGroup(group);
    var shift = DenonHC1000S.shift;
    print("loopOrHotcues shift " + shift);
    var playStatus = engine.getValue(group, "play"); //get Play status
    print("ButtonStatus loopOrHotcue is " + (status & 0xF0));
    if ((status & 0xF0) === (0x80 || 0x81)) {
        print("deck.controlPressedIF before is " + (deck.controlPressed));
        deck.controlPressed = -1;
        print("deck.controlPressedIF is " + (deck.controlPressed));
    } else {
        deck.controlPressed = control;
        print("deck.controlPressedELSE is " + (deck.controlPressed));
        if (DenonHC1000S.shiftSampleLeft || DenonHC1000S.shiftSampleRight) {
            switch (control) {
            // deck A //
            // Samplers
            case 0x17:
                print((group) + ": play status before was " + (playStatus));
                engine.setValue(group, "play", !playStatus); //Play toggle
                break;
            case 0x18:
                if (playStatus) {
                    engine.setValue(group, "cue_gotoandplay", 1); //Cue and Play
                } else {
                    engine.setValue(group, "cue_default", 1); //set Cue if paused
                }
                break;
            case 0x19:
                DenonHC1000S.sampler(1, group, value, shift);
                break;
            case 0x20:
                DenonHC1000S.sampler(2, group, value, shift);
                break;
            case 0x21:
                DenonHC1000S.sampler(3, group, value, shift);
                break;
                // loop A
            case 0x37:
                DenonHC1000S.loopIn(group, value, shift); // DVS control?
                break;
            case 0x40:
                DenonHC1000S.reloop(group, value, shift); // DVS control?
                break;
                // deck B //
                // hotcue B
            case 0x75:
                print((group) + ": play status before was " + (playStatus));
                engine.setValue(group, "play", !playStatus); //Play toggle
                break;
            case 0x76:
                if (playStatus) {
                    engine.setValue(group, "cue_gotoandplay", 1); //Cue and Play
                } else {
                    engine.setValue(group, "cue_default", 1); //set Cue if paused
                }
                break;
            case 0x77:
                DenonHC1000S.sampler(4, group, value, shift);
                break;
            case 0x78:
                DenonHC1000S.sampler(5, group, value, shift);
                break;
            case 0x79:
                DenonHC1000S.sampler(6, group, value, shift);
                break;
                // loop B
            case 0x7B:
                DenonHC1000S.loopIn(group, value, shift); // DVS control?
                break;
            case 0x7D:
                DenonHC1000S.reloop(group, value, shift); // DVS control?
                break;
            }
        } else {
            switch (control) {
            // deck A //
            // hotcue A
            case 0x17:
                DenonHC1000S.hotcue(1, group, value, shift);
                break;
            case 0x18:
                DenonHC1000S.hotcue(2, group, value, shift);
                break;
            case 0x19:
                DenonHC1000S.hotcue(3, group, value, shift);
                break;
            case 0x20:
                DenonHC1000S.hotcue(4, group, value, shift);
                break;
            case 0x21:
                DenonHC1000S.hotcue(5, group, value, shift);
                break;
                // loop A
            case 0x37:
                DenonHC1000S.loopIn(group, value, shift);
                break;
            case 0x40:
                DenonHC1000S.reloop(group, value, shift);
                break;
                // deck B //
                // hotcue B
            case 0x75:
                DenonHC1000S.hotcue(1, group, value, shift);
                break;
            case 0x76:
                DenonHC1000S.hotcue(2, group, value, shift);
                break;
            case 0x77:
                DenonHC1000S.hotcue(3, group, value, shift);
                break;
            case 0x78:
                DenonHC1000S.hotcue(4, group, value, shift);
                break;
            case 0x79:
                DenonHC1000S.hotcue(5, group, value, shift);
                break;
                // loop B
            case 0x7B:
                DenonHC1000S.loopIn(group, value, shift);
                break;
            case 0x7D:
                DenonHC1000S.reloop(group, value, shift);
                break;
            }
        }
    }
};

DenonHC1000S.hotcue = function(cueIndex, group, value, shift) {
    if (!shift) {
        engine.setValue(group, "hotcue_" + cueIndex + "_activate", 1);
    } else {
        if (engine.getValue(group, "hotcue_" + cueIndex + "_enabled") === 0) {
            var samplesPerBeat = DenonHC1000S.samplesPerBeat(group);
            var positionInBeats = (engine.getValue(group, "playposition") * engine.getValue(group, "track_samples")) / samplesPerBeat;
            if ((positionInBeats - Math.floor(positionInBeats)) > 0.5) {
                positionInBeats = Math.floor(0.5 + positionInBeats) * samplesPerBeat;
            } else {
                positionInBeats = Math.floor(positionInBeats) * samplesPerBeat;
                positionInBeats = Math.floor(0.5 + positionInBeats);
                positionInBeats = Math.max(0, positionInBeats - positionInBeats % 2);
                engine.setValue(group, "hotcue_" + cueIndex + "_activate", 1);
                engine.setValue(group, "hotcue_" + cueIndex + "_position", positionInBeats);
            }
        } else {
            engine.setValue(group, "hotcue_" + cueIndex + "_clear", 1);
        }
    }
};

DenonHC1000S.sampler = function(cueIndex, group, value, shift) {
    if (!shift) {
        if (DenonHC1000S.shiftSampleLeft && DenonHC1000S.shiftSampleRight) { //shiftSampleRight AND shiftSampleLeft buttons are pressed -> load currently selected track from library
            engine.setValue("[Sampler" + cueIndex + "]", "LoadSelectedTrack", 1);
        } else { // sample playback
            print(group + ": Sampler#" + cueIndex + " selected; shiftLeft is " + DenonHC1000S.shiftSampleLeft + "; shiftRight is " + DenonHC1000S.shiftSampleRight);
            engine.setValue("[Sampler" + cueIndex + "]", "cue_gotoandplay", 1);
        }
    } else { // shift pressed; stop or load sample

        print("Shift pressed -> stopping " + group + ": Sampler#" + cueIndex + " shiftLeft is " + DenonHC1000S.shiftSampleLeft + "; shiftRight is " + DenonHC1000S.shiftSampleRight);
        engine.setValue("[Sampler" + cueIndex + "]", "start_stop", 1);
    }
};
DenonHC1000S.loopIn = function(group, value, shift) {
    if (shift) {
        engine.setValue(group, "loop_start_position", -1);
    } else {
        if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "loop_in_goto", 1); // to fix LoopIn hanging
        } else {
            engine.setValue(group, "loop_in", 1);
        }
    }
};

DenonHC1000S.loopOut = function(midino, control, value, status, group) {
    print("ButtonStatusLoopOut triggered");
    var shift = DenonHC1000S.shift;
    if (shift) {
        print("loopOut: shift is pressed");
        engine.setValue(group, "loop_out", -1);
        engine.setValue(group, "loop_end_position", -1);
    } else {
        if (value === 64) { //only on press not on release to avoid doubles
            engine.setValue(group, "loop_end_position", -1); //removes current to fix hanging out
            engine.setValue(group, "loop_out", 1); //and sets again
            print("value is " + value);
        }
    }
};

DenonHC1000S.reloop = function(group, value, shift) {
    var loopInPosition = engine.getValue(group, "loop_start_position");
    var loopOutPosition = engine.getValue(group, "loop_end_position");
    if (loopInPosition !== -1 && loopOutPosition !== -1) {
        engine.setValue(group, "reloop_toggle", 1);
    } else {
        var samplesPerBeat = DenonHC1000S.samplesPerBeat(group);
        loopInPosition = engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
        if (shift) {
            var loopInPositionInBeats = loopInPosition / samplesPerBeat;
            if ((loopInPositionInBeats - Math.floor(loopInPositionInBeats)) > 0.5) {
                loopInPosition = Math.floor(0.5 + loopInPositionInBeats) * samplesPerBeat;
            } else {
                loopInPosition = Math.floor(loopInPositionInBeats) * samplesPerBeat;
            }
        } else {
            loopInPosition = engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
        }
        loopInPosition = Math.floor(0.5 + loopInPosition);
        loopInPosition = Math.max(0, loopInPosition - loopInPosition % 2);
        loopOutPosition = loopInPosition + Math.floor(0.5 + samplesPerBeat);
        loopOutPosition = Math.max(0, loopOutPosition - loopOutPosition % 2);
        if (loopInPosition + samplesPerBeat < engine.getValue(group, "track_samples")) {
            engine.setValue(group, "loop_start_position", loopInPosition);
            engine.setValue(group, "loop_end_position", loopOutPosition);
            engine.setValue(group, "reloop_toggle", 1);
        }
    }
};

DenonHC1000S.resizeLoop = function(midino, control, value, status, group) {
    var knobDelta = DenonHC1000S.getKnobDelta(value);
    print(group + " loop_scale " + knobDelta);
    var scaleFactor = 1;
    if (!DenonHC1000S.shift) {
        if (knobDelta > 0) { //make scale bigger (right turn)
            if (!DenonHC1000S.resizeLoopButtonPressed) {
                scaleFactor = (scaleFactor * scaleFactor + 1 / 2);
                print("scale up by factor " + scaleFactor);
                engine.setValue(group, "loop_scale", scaleFactor);
            } else {
                //move loop to right
                DenonHC1000S.moveLoopRight(midino, control, value, status, group);
                print("moving loop right");
            }
        } else { //make scale smaller (left turn)
            if (!DenonHC1000S.resizeLoopButtonPressed) {
                scaleFactor = (scaleFactor - scaleFactor * 1 / 2);
                print("scale down by factor " + scaleFactor);
                engine.setValue(group, "loop_scale", scaleFactor);
            } else {
                //move loop to left
                DenonHC1000S.moveLoopLeft(midino, control, value, status, group);
                print("moving loop left");
            }
        }
    } else { //shift pressed -> change beatloop size with wheel
        var beatloopSizeFactor = engine.getValue(group, "beatloop_size");
        if (knobDelta > 0) { //make beatloop_size bigger (right turn)
            if ((DenonHC1000S.resizeLoopButtonPressed) && (beatloopSizeFactor > 1)) {
                scaleFactor = Math.round((beatloopSizeFactor + 1)); //single increment
            } else {
                scaleFactor = (beatloopSizeFactor * 2); //double
                if (scaleFactor > 1) {
                    scaleFactor = Math.round(scaleFactor);
                }
            }
            print("change beatloop_size by factor " + scaleFactor);
            engine.setValue(group, "beatloop_size", scaleFactor);
        } else { //make beatloop_size  smaller (left turn)
            if ((DenonHC1000S.resizeLoopButtonPressed) && (beatloopSizeFactor > 1)) {
                scaleFactor = Math.round((beatloopSizeFactor - 1));
            } else {
                scaleFactor = (beatloopSizeFactor * 1 / 2);
                if (scaleFactor > 1) {
                    scaleFactor = Math.round(scaleFactor);
                }
            }
            print("change beatloop_size by factor " + scaleFactor);
            engine.setValue(group, "beatloop_size", scaleFactor);
        }
    }
};

DenonHC1000S.moveLoopLeft = function(midino, control, value, status, group) {
    var samplesPerBeat = Math.floor(0.5 + DenonHC1000S.samplesPerBeat(group));
    var loopInPosition = engine.getValue(group, "loop_start_position");
    var loopOutPosition = engine.getValue(group, "loop_end_position");
    if (loopInPosition !== -1 && loopOutPosition !== -1 && loopInPosition > samplesPerBeat) {
        loopInPosition = loopInPosition - samplesPerBeat;
        loopInPosition = Math.max(0, loopInPosition - loopInPosition % 2);
        loopOutPosition = loopOutPosition - samplesPerBeat;
        loopOutPosition = Math.max(0, loopOutPosition - loopOutPosition % 2);
        engine.setValue(group, "loop_start_position", loopInPosition);
        engine.setValue(group, "loop_end_position", loopOutPosition);
    }
};

DenonHC1000S.moveLoopRight = function(midino, control, value, status, group) {
    var samplesPerBeat = Math.floor(0.5 + DenonHC1000S.samplesPerBeat(group));
    var loopInPosition = engine.getValue(group, "loop_start_position");
    var loopOutPosition = engine.getValue(group, "loop_end_position");
    if (loopInPosition !== -1 && loopOutPosition !== -1 && loopOutPosition + samplesPerBeat < engine.getValue(group, "track_samples")) {
        loopInPosition = loopInPosition + samplesPerBeat;
        loopInPosition = Math.max(0, loopInPosition - loopInPosition % 2);
        loopOutPosition = loopOutPosition + samplesPerBeat;
        loopOutPosition = Math.max(0, loopOutPosition - loopOutPosition % 2);
        engine.setValue(group, "loop_start_position", loopInPosition);
        engine.setValue(group, "loop_end_position", loopOutPosition);
    }
};

DenonHC1000S.autoLoop = function(midino, control, value, status, group) {
    if (!DenonHC1000S.shift) {
        engine.setValue(group, "beatloop_activate", false);
        engine.setValue(group, "beatloop_activate", true);
    } else {
        engine.setValue(group, "beatloop_activate", false);
        engine.setValue(group, "reloop_exit", true);
    }
};

DenonHC1000S.beatLoopRoll = function(midino, control, value, status, group) {
    DenonHC1000S.beatroll = !DenonHC1000S.beatroll; // '!' inverts a boolean (true/false) value
    print("beatroll is pressed " + DenonHC1000S.beatroll);
    engine.setValue(group, "beatlooproll_activate", DenonHC1000S.beatroll);
};

DenonHC1000S.pitchBendDown = function(midino, control, value, status, group) {
    if (DenonHC1000S.shiftSampleLeft || DenonHC1000S.shiftSampleRight) { //with Sample-shift use this button for DVS Ctrl
        var vinylcontrolStatus = engine.getValue(group, "vinylcontrol_enabled");
        if ((status & 0xF0) === 0x90) {
            engine.setValue(group, "vinylcontrol_enabled", (!vinylcontrolStatus)); //DVS VinylControl Toggle (software)
        }
    } else {
        if (!DenonHC1000S.shift) {
            if ((status & 0xF0) === 0x90) {
                engine.setValue(group, "rate_temp_down", 1);
            } else {
                engine.setValue(group, "rate_temp_down", 0);
            }
        } else {
            if ((status & 0xF0) !== 0x90) {
                engine.setValue(group, "back", false);
            } else {
                engine.setValue(group, "back", true);
            }
        }
    }
};

DenonHC1000S.pitchBendUp = function(midino, control, value, status, group) {
    if (DenonHC1000S.shiftSampleLeft || DenonHC1000S.shiftSampleRight) { //with Sample-shift use this button for DVS Ctrl
        var passthroughStatus = engine.getValue(group, "passthrough");
        if ((status & 0xF0) === 0x90) {
            engine.setValue(group, "passthrough", (!passthroughStatus)); //DVS Passthrough (software)
        }
    } else {
        if (!DenonHC1000S.shift) {
            if ((status & 0xF0) === 0x90) {
                engine.setValue(group, "rate_temp_up", 1);
            } else {
                engine.setValue(group, "rate_temp_up", 0);
            }
        } else {
            if ((status & 0xF0) !== 0x90) {
                engine.setValue(group, "fwd", false);
            } else {
                engine.setValue(group, "fwd", true);
            }
        }
    }
};

DenonHC1000S.jumpBack = function(midino, control, value, status, group) {
    if (DenonHC1000S.shiftSampleLeft || DenonHC1000S.shiftSampleRight) { //with Sample-shift use this button for DVS Ctrl
        var vinylcontrolModeBefore = engine.getValue(group, "vinylcontrol_mode");
        if (vinylcontrolModeBefore < 2) {
            var vinylcontrolModeNext = (vinylcontrolModeBefore + 1);
        } else {
            vinylcontrolModeNext = 0;
        }
        if ((status & 0xF0) === 0x90) {
            print("DVS changing vinyl control mode to " + vinylcontrolModeNext);
            engine.setValue(group, "vinylcontrol_mode", vinylcontrolModeNext); //DVS VinylControl Mode
        }
    } else {
        if (!DenonHC1000S.shift) {
            engine.setValue(group, "beatjump_size", engine.getValue(group, "beatloop_size")); //using beatLoopScale as JumpScale (could be different)
            if ((status & 0xF0) === 0x90) {
                engine.setValue(group, "beatjump_backward", 1);
            } else {
                engine.setValue(group, "beatjump_backward", 0);
            }
        } else {
            if ((status & 0xF0) !== 0x90) {
                engine.setValue(group, "start", false);
            } else {
                engine.setValue(group, "start", true);
            }
        }
    }
};

DenonHC1000S.jumpFwd = function(midino, control, value, status, group) {
    if (DenonHC1000S.shiftSampleLeft || DenonHC1000S.shiftSampleRight) { //with Sample-shift use this button for DVS Ctrl
        var vinylcontrolCueBefore = engine.getValue(group, "vinylcontrol_cueing");
        if (vinylcontrolCueBefore < 2) {
            var vinylcontrolCueNext = (vinylcontrolCueBefore + 1);
        } else {
            vinylcontrolCueNext = 0;
        }
        if ((status & 0xF0) === 0x90) {
            print("DVS changing vinyl cueing to " + vinylcontrolCueNext);
            engine.setValue(group, "vinylcontrol_cueing", vinylcontrolCueNext); //DVS VinylControl Cueing
        }
    } else {
        if (!DenonHC1000S.shift) {
            engine.setValue(group, "beatjump_size", engine.getValue(group, "beatloop_size")); //using beatLoopScale as JumpScale (could be different)
            if ((status & 0xF0) === 0x90) {
                engine.setValue(group, "beatjump_forward", 1);
            } else {
                engine.setValue(group, "beatjump_forward", 0);
            }
        } else {
            if ((status & 0xF0) !== 0x90) {
                engine.setValue(group, "end", false);
            } else {
                engine.setValue(group, "end", true);
            }
        }
    }
};

DenonHC1000S.bpmTabButton = function(midino, control, value, status, group) {
    if (DenonHC1000S.shiftSampleLeft || DenonHC1000S.shiftSampleRight) { //with Sample-shift use this button for DVS Ctrl
        var showVinylcontrolStatus = engine.getValue("[VinylControl]", "show_vinylcontrol");
        engine.setValue("[VinylControl]", "show_vinylcontrol", !showVinylcontrolStatus);
    } else {
        if ((status & 0xF0) === 0x90) {
            engine.setValue(group, "bpm_tap", true);
        } else {
            engine.setValue(group, "bpm_tap", false);
        }
    }
};

DenonHC1000S.reverseRoll = function(midino, control, value, status, group) {
    if (DenonHC1000S.shiftSampleLeft || DenonHC1000S.shiftSampleRight) { //with Sample-shift use this button for DVS Ctrl
        if ((status & 0xF0) === 0x90) {
            engine.setValue("[VinylControl]", "Toggle", true);
            print("VinylControl Toggle is triggered");
        } else {
            engine.setValue("[VinylControl]", "Toggle", false);
        }
    } else {
        if ((status & 0xF0) === 0x90) {
            engine.setValue(group, "reverseroll", true);
        } else {
            engine.setValue(group, "reverseroll", false);
        }
    }
};

DenonHC1000S.LoadTrack = function(midino, control, value, status, group) {
    if (!DenonHC1000S.shift) {
        print("group is " + (group));
        engine.setValue(group, "LoadSelectedTrack", 1);
    } else {
        print("shift & trackLoad => InstantDouble");
        switch (group) {
        case "[Channel1]":
            engine.setValue(group, "CloneFromDeck", 2);
            break;
        case "[Channel2]":
            engine.setValue(group, "CloneFromDeck", 1);
            break;
        case "[Channel3]":
            engine.setValue(group, "CloneFromDeck", 4);
            break;
        case "[Channel4]":
            engine.setValue(group, "CloneFromDeck", 3);
            break;
        }
    }
};

DenonHC1000S.ledHotCueAndSampler = function(channel, control, value, status, group) {
    var ledChannel = DenonHC1000S.getLedChannelByGroup(group); // 0xB0 or
    if (DenonHC1000S.shiftSampleLeft || DenonHC1000S.shiftSampleRight) { //show Sampler/Play/Cue info instead of HotCue
        var samplerLeds = [0x15, 0x17, 0x19, 0x65, 0x67, 0x69]; //,0x1B,0x40,0x20
        print("ledChannel = " + ledChannel);
        for (var index = 0, count = samplerLeds.length; index < count; index++) {
            DenonHC1000S.handleLed(ledChannel, (engine.getValue("[Sampler" + (index + 1) + "]", "track_loaded") === 1), samplerLeds[index], 0x4C, 0x4B);
            print("LED sampler is " + (index));
        }
        //Play Button on Shift-Cue-1
        ledChannel = DenonHC1000S.getLedChannelByGroup(group);
        //Channel 1+3
        DenonHC1000S.handleLed((ledChannel), (engine.getValue("[Channel1]", "track_loaded") === 1), 0x11, 0x4C, 0x4B);
        DenonHC1000S.handleLed((ledChannel), (engine.getValue("[Channel1]", "cue_indicator") === 1), 0x13, 0x4C, 0x4B);
        //Channel 2+4
        DenonHC1000S.handleLed((ledChannel), (engine.getValue("[Channel2]", "track_loaded") === 1), 0x61, 0x4C, 0x4B);
        DenonHC1000S.handleLed((ledChannel), (engine.getValue("[Channel2]", "cue_indicator") === 1), 0x63, 0x4C, 0x4B);

        // hotcues
    } else {
        ledChannel = DenonHC1000S.getLedChannelByGroup(group);
        //Channel 1+3
        var cueLeds = [0x11, 0x13, 0x15, 0x17, 0x19]; //,0x1B,0x40,0x20
        print("LED group1 is " + (group));
        for (index = 0, count = cueLeds.length; index < count; index++) {
            if (group !== "[Channel3]") {
                DenonHC1000S.handleLed(0xB0, (engine.getValue("[Channel1]", "hotcue_" + (index + 1) + "_position") !== -1), cueLeds[index], 0x4A, 0x4B);
            } else {
                DenonHC1000S.handleLed(0xB1, (engine.getValue("[Channel3]", "hotcue_" + (index + 1) + "_position") !== -1), cueLeds[index], 0x4A, 0x4B);
            }
        }
        // Channel 2+4
        cueLeds = [0x61, 0x63, 0x65, 0x67, 0x69]; //,0x1B,0x7D,0x78
        print("LED group2 is " + (group));
        for (index = 0, count = cueLeds.length; index < count; index++) {
            if (group !== "[Channel4]") {
                DenonHC1000S.handleLed(0xB0, (engine.getValue("[Channel2]", "hotcue_" + (index + 1) + "_position") !== -1), cueLeds[index], 0x4A, 0x4B);
            } else {
                DenonHC1000S.handleLed(0xB1, (engine.getValue("[Channel4]", "hotcue_" + (index + 1) + "_position") !== -1), cueLeds[index], 0x4A, 0x4B);
            }
        }
    }
};

DenonHC1000S.newTrackLoaded = function(channel, control, value, status, group) {
    //to update LEDs (e.g. loop buttons) when new track is loaded
    DenonHC1000S.ledHotCueAndSampler(channel, control, value, status, group);
    DenonHC1000S.ledLoop(channel, control, value, status, group);
};

DenonHC1000S.ledLoop = function(value, group) {
    var ledChannel = DenonHC1000S.getLedChannelByGroup(group);
    if (engine.getValue(group, "loop_enabled") === 1) { //LED fully on, or blinking?
        var ledOnTypeLoop = 0x4C; //blinking
        print(group + " Loop is enabled");
    } else {
        ledOnTypeLoop = 0x4A; //fully on
        print(group + " Loop is disabled");
    }
    if (engine.getValue(group, "beatlooproll_activate") === 1) { //LED fully on, or blinking?
        var ledOnTypeAutoLoop = 0x4C; //blinking
        print(group + " BeatRoll is enabled");
    } else {
        ledOnTypeAutoLoop = 0x4A; //fully on
        print(group + " BeatRoll is disabled");
    }
    print(group + "LED ledOnTypeLoop is " + ledOnTypeLoop);
    print(group + "LED ledOnTypeAutoLoop is " + ledOnTypeAutoLoop);

    if (group !== "[Channel2]" && group !== "[Channel4]") {
        ledChannel = DenonHC1000S.getLedChannelByGroup(group);
        DenonHC1000S.handleLed(ledChannel, (engine.getValue(group, "loop_start_position") !== -1), 0x24, ledOnTypeLoop, 0x4B); //IN
        DenonHC1000S.handleLed(ledChannel, (engine.getValue(group, "loop_end_position") !== -1), 0x40, ledOnTypeLoop, 0x4B); //OUT
        DenonHC1000S.handleLed(ledChannel, ((engine.getValue(group, "beatloop_activate") === 1) || (engine.getValue(group, "beatlooproll_activate") === 1)), 0x2B, ledOnTypeAutoLoop, 0x4B); //AUTOLOOP or Beatroll
    } else {
        ledChannel = DenonHC1000S.getLedChannelByGroup(group);
        DenonHC1000S.handleLed(ledChannel, (engine.getValue(group, "loop_start_position") !== -1), 0x6B, ledOnTypeLoop, 0x4B); //IN
        DenonHC1000S.handleLed(ledChannel, (engine.getValue(group, "loop_end_position") !== -1), 0x6D, ledOnTypeLoop, 0x4B); //OUT
        DenonHC1000S.handleLed(ledChannel, ((engine.getValue(group, "beatloop_activate") === 1) || (engine.getValue(group, "beatlooproll_activate") === 1)), 0x6F, ledOnTypeAutoLoop, 0x4B); //AUTOLOOP
    }
};

DenonHC1000S.handleLed = function(ledChannel, test, led, stateTrue, stateFalse) {
    if (test) {
        midi.sendShortMsg(ledChannel, stateTrue, led);
    } else {
        midi.sendShortMsg(ledChannel, stateFalse, led);
    }
};

DenonHC1000S.getLedChannelByGroup = function(group) {
    print("ledChannelByGroup reads channel as " + group);
    if (group === "[Channel1]" || group === "[Channel2]") {
        return 0xB0; // Midi-Ch#1 deck A+B
    } else {
        return 0xB1; // Midi-Ch#2 deck C+D
    }
};

DenonHC1000S.toggleBinaryValue = function(group, key) {
    engine.setValue(group, key, engine.getValue(group, key) * -1 + 1);
};

//Beat flashing changed state/////////////
DenonHC1000S.onBeatFlash = function(value, group, _control) {
    var val = (value) ? 0x4A : 0x4B;
    var deck = parseInt(DenonHC1000S.groupToDeck(group));
    //print ("beat_active selector for deck " + deck + " value=" + val); //debug
    switch (deck) {
    case 1:
        midi.sendShortMsg(0xB0, val, 0x09);
        break;
    case 2:
        midi.sendShortMsg(0xB0, val, 0x60);
        break;
    case 3:
        midi.sendShortMsg(0xB1, val, 0x09);
        break;
    case 4:
        midi.sendShortMsg(0xB1, val, 0x60);
        break;
    }
};

/*******************************************************************************/
var decks = [new DenonHC1000S.Deck(1, "[Channel1]"),
    new DenonHC1000S.Deck(2, "[Channel2]"),
    new DenonHC1000S.Deck(3, "[Channel3]"),
    new DenonHC1000S.Deck(4, "[Channel4]"),
];

DenonHC1000S.shutdown = function() {
    // send whatever MIDI messages you need to turn off the lights of your controller
    var leds = [0x11, 0x13, 0x15, 0x17, 0x19, /* cues deckA*/
        0x24, 0x40, 0x2B, /* loops deckA*/
        0x09, /* sync deckA*/
        0x61, 0x63, 0x65, 0x67, 0x69, /* cues deckB*/
        0x6B, 0x6D, 0x6F, /* loops deckB*/
        0x60 /* sync deckB*/
    ];
    for (var index = 0, count = leds.length; index < count; index++) {

        midi.sendShortMsg(0xB0, 0x4B, leds[index]); //Channel1
        midi.sendShortMsg(0xB1, 0x4B, leds[index]); //Channel2
        midi.sendShortMsg(0xB2, 0x4B, leds[index]); //Channel3
        midi.sendShortMsg(0xB3, 0x4B, leds[index]); //Channel4
    }
};

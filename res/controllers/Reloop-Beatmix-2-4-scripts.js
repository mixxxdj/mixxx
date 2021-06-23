/***********************************************************************
 * ==============             User Options             =================
 * Track End Warning
 * ---------------
 * By default, when you reach the end of the track, the jog wheel is flashing.
 * It flashes slowly when there is less than JogFlashWarningTime seconds left,
 * and quickly in the last JogFlashCriticalTime seconds.
 * You can adjust these values to suit your needs by setting the two variables below.
 * Set them to -1 if you want to completely disable flashing at the end of track.
 **************************/
 var JogFlashWarningTime = 30; // number of seconds to slowly blink at the end of track
 var JogFlashCriticalTime = 15; // number of seconds to quickly blink at the end of track


/************************  GPL v2 licence  *****************************
 * Reloop Beatmix 2/4 controller script
 * Author: Sébastien Blaisot <sebastien@blaisot.org>
 *
 **********************************************************************
 * User References
 * ---------------
 * Wiki/manual : http://www.mixxx.org/wiki/doku.php/reloop_beatmix_2
 * Wiki/manual : http://www.mixxx.org/wiki/doku.php/reloop_beatmix_4
 * support forum : http://mixxx.org/forums/viewtopic.php?f=7&t=8428
 *
 * Thanks
 * ----------------
 * Thanks to Be.Ing for mapping review
 *
 * Revision history
 * ----------------
 * 2016-07-28 - v1.0 - Initial revision for Mixxx 2.1.0
 * 2016-07-31 - v1.1 - fix some bugs, Improved pad mapping, and lots of small improvements
 * 2016-08-13 - v1.2 - Improved jog leds
 * 2016-08-15 - v1.2.1 - fix small typos and bugs
 * 2016-08-17 - v1.3 - sync each item on the controller with Mixxx at launch
 * 2016-08-19 - v1.3.1 - fix nex/prev effect button release and superknob responsiveness
 ***********************************************************************
 *                           GPL v2 licence
 *                           --------------
 * Reloop Beatmix controller script script 1.3.1 for Mixxx 2.1.0
 * Copyright (C) 2016 Sébastien Blaisot
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***********************************************************************/
////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////

// Global variables and declarations.
// ========================================================
var ReloopBeatmix24 = {};

var RateRangeArray = [0.08, 0.10, 0.12, 0.16];

// Timers & long press state
var jogWheelTimers = [];
var loadButtonTimers = [];
var loadButtonLongPressed = [];
var FxModeTimers = [];
var FxModeLongPressed = [];

// A variable to store previous value of some connectControls
var previousValue = [];

// Trax mode
// 1 for playlist mode
// 2 for track mode
// 3 for preview mode
var traxMode = 2;

// Effects mode
// 1 for single effect mode (1 effect controlled in each EffectUnit)
// 2 for multi-effect mode (3 effects controlled in each EffectUnit)
// SHIFT + long press on pitchbend +/- to change mode
var FxMode = 1; // Single effect mode by default

// Jog Led variables
// set JogFlashWarningTime and JogFlashCriticalTime to -1 to disable jog wheel flash
var JogRPM = 33.0 + 1/3, // Jog Wheel simulates a 33.3RPM turntable
    RoundTripTime = 60.0 / JogRPM, // Time in seconds for a complete turn
    JogLedNumber = 16, // number of leds (sections) on the jog wheel
    JogBaseLed = 0x3f, // Midino of last led (we count backward to turn in the right side)
    JogFlashWarningInterval = 400, // number of ms to wait when flashing slowly
    JogFlashCriticalInterval = 200; // number of ms to wait when flashing quickly

var JogLedLit = [];
var channelPlaying = []; // Keeping track of channel playing
var JogBlinking = [];

// Buttons and Led Variables
var ON = 0x7F,
    OFF = 0x00,
    RED = 0x7F,
    BLUE = 0x55,
    VIOLET = 0x2A,
    SHIFT = 0x40,
    DOWN = 0x7F,
    UP = 0x00;

// The SysEx message to send to the controller to force the midi controller
// to send the status of every item on the control surface.
var ControllerStatusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];

// Some useful regex
var channelRegEx = /\[Channel(\d+)\]/;
var samplerRegEx = /\[Sampler(\d+)\]/;

// Initialise and shutdown stuff.
// ========================================================
ReloopBeatmix24.TurnLEDsOff = function() {
    // Turn all LEDS off
    var i, j;
    for (i = 0x91; i <= 0x94; i++) { // 4 decks

        // Pads
        for (j = 0x00; j <= 0x0F; j++) {
            midi.sendShortMsg(i, j, OFF);
            midi.sendShortMsg(i, j + SHIFT, OFF);
        }
        for (j = 0x10; j <= 0x1F; j++) {
            midi.sendShortMsg(i, j, OFF);
        }

        // Play/cue/cup/sync/pitch bend
        for (j = 0x20; j <= 0x25; j++) {
            midi.sendShortMsg(i, j, OFF);
            midi.sendShortMsg(i, j + SHIFT, OFF);
        }

        // Jog Wheel leds
        ReloopBeatmix24.AllJogLEDsToggle(i, OFF);

        // Load + Cue
        midi.sendShortMsg(i, 0x50, OFF);
        midi.sendShortMsg(i, 0x52, OFF);
        midi.sendShortMsg(i, 0x72, OFF);
    }
};

ReloopBeatmix24.connectControls = function() {
    var group;

    // Channels controls
    for (var i = 1; i <= 4; i++) {
        group = "[Channel" + i + "]";
        engine.connectControl(group, "track_samples",
            "ReloopBeatmix24.deckLoaded");
        engine.connectControl(group, "play",
            "ReloopBeatmix24.ChannelPlay");
        engine.connectControl(group, "playposition",
            "ReloopBeatmix24.JogLed");
        engine.connectControl(group, "loop_end_position",
            "ReloopBeatmix24.loopDefined");
        engine.softTakeover(group, "rate", true);
        engine.setValue("[EffectRack1_EffectUnit1]",
            "group_" + group + "_enable", 0);
        engine.setValue("[EffectRack1_EffectUnit2]",
            "group_" + group + "_enable", 0);
        channelPlaying[group] = engine.getValue(group, "play") ? true : false;
        JogBlinking[group] = false;
    }

    // Samplers controls
    for (i = 1; i <= 8; i++) {
        group = "[Sampler" + i + "]";
        engine.connectControl(group, "track_samples",
            "ReloopBeatmix24.deckLoaded");
        engine.connectControl(group, "play",
            "ReloopBeatmix24.SamplerPlay");
    }

    // Effects reset
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Master]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Master]_enable", 0);
};

ReloopBeatmix24.init = function(id, debug) {
    ReloopBeatmix24.id = id;
    ReloopBeatmix24.TurnLEDsOff(); // Turn off all LEDs
    ReloopBeatmix24.connectControls(false);

    for (var i = 1; i <= 4; i++) {
        engine.trigger("[Channel" + i + "]", "loop_end_position");
    }

    // After midi controller receive this Outbound Message request SysEx Message,
    // midi controller will send the status of every item on the
    // control surface. (Mixxx will be initialized with current values)
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);

    print("Reloop Beatmix: " + id + " initialized.");
};

ReloopBeatmix24.shutdown = function() {
    ReloopBeatmix24.TurnLEDsOff(); // Turn off all LEDs
    print("Reloop Beatmix: " + ReloopBeatmix24.id + " shut down.");
};

// Button functions.
// ========================================================
ReloopBeatmix24.GetNextRange = function(previous) {
    var len = RateRangeArray.length;
    var pos = RateRangeArray.indexOf(previous);
    // If 'previous' is not found in the array, pos == -1 and (pos+1) == 0,
    // so this function will return the first element
    return RateRangeArray[(pos + 1) % len];
};

ReloopBeatmix24.Range = function(channel, control, value, status, group) {
    if (value === DOWN) {
        var oldvalue = engine.getValue(group, "rateRange");
        engine.setValue(group, "rateRange", ReloopBeatmix24.GetNextRange(
            oldvalue));
        engine.softTakeoverIgnoreNextValue(group, "rate");
    }
};

ReloopBeatmix24.MasterSync = function(channel, control, value, status, group) {
    if (value === DOWN) {
        script.toggleControl(group, "sync_enabled");
    }
};

ReloopBeatmix24.LoopSet = function(channel, control, value, status, group) {
    if (value === DOWN) {
        engine.setValue(group, "loop_in", 1);
    } else {
        engine.setValue(group, "loop_out", 1);
    }
};

ReloopBeatmix24.PitchSlider = function(channel, control, value, status, group) {
    engine.setValue(group, "rate", -script.midiPitch(control, value, status));
};

// Trax navigation functions
// ========================================================
ReloopBeatmix24.traxSelect = function(value, step) {
    switch (traxMode) {
        case 1: // Playlist mode
            var i, j;
            for (i = 0; i < Math.abs(value); i++) {
                for (j = 0; j < step; j++) {
                    if (value < 0) {
                        engine.setValue("[Playlist]", "SelectPrevPlaylist",
                            true);
                    } else {
                        engine.setValue("[Playlist]", "SelectNextPlaylist",
                            true);
                    }
                }
            }
            break;
        case 2: // Track mode
            engine.setValue("[Playlist]", "SelectTrackKnob", value * step);
            break;
        case 3: // Preview mode
            engine.setValue("[PreviewDeck1]", "playposition", Math.max(0,
                Math.min(1, engine.getValue("[PreviewDeck1]",
                    "playposition") + 0.02 * value * step)));
            break;
    }
};

ReloopBeatmix24.TraxTurn = function(channel, control, value, status, group) {
    ReloopBeatmix24.traxSelect(value - 0x40, 1);
};

ReloopBeatmix24.ShiftTraxTurn = function(channel, control, value, status, group) {
    ReloopBeatmix24.traxSelect(value - 0x40, 10);
};

ReloopBeatmix24.TraxPush = function(channel, control, value, status, group) {
    switch (traxMode) {
        case 1: // Playlist mode
            engine.setValue("[Playlist]", "ToggleSelectedSidebarItem",
                value);
            break;
        case 2: // Track mode
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay",
                value);
            traxMode = 3;
            break;
        case 3: // Preview mode
            if (value == DOWN) {
                script.toggleControl("[PreviewDeck1]", "play");
            }
            break;
    }
};

ReloopBeatmix24.BackButton = function(channel, control, value, status, group) {
    if (value === DOWN) {
        switch (traxMode) {
            case 1: // Playlist mode
                traxMode = 2; // Switch to track mode
                break;
            case 2: // Track mode
                traxMode = 1; // Switch to playlist mode
                break;
            case 3: // Preview mode
                traxMode = 2; // Switch to track mode
                break;
        }
    }
};

ReloopBeatmix24.LoadButtonEject = function(group) {
    loadButtonLongPressed[group] = true;
    engine.setValue(group, "eject", 1);
    delete loadButtonTimers[group];
};

ReloopBeatmix24.LoadButton = function(channel, control, value, status, group) {
    if (value === DOWN) {
        loadButtonLongPressed[group] = false;
        loadButtonTimers[group] = engine.beginTimer(1000,
            "ReloopBeatmix24.LoadButtonEject(\"" + group + "\")", true);
    } else { // UP
        if (!loadButtonLongPressed[group]) { // Short press
            engine.stopTimer(loadButtonTimers[group]);
            delete loadButtonTimers[group];
            engine.setValue(group, "LoadSelectedTrack", 1);
        } else {
            // Set eject back to 0 to turn off the eject button on screen
            engine.setValue(group, "eject", 0);
            loadButtonLongPressed[group] = false;
        }
    }
};

// Sampler functions
// ========================================================
ReloopBeatmix24.SamplerPad = function(channel, control, value, status, group) {
    if (value === DOWN) {
        if (engine.getValue(group, "track_samples")) { //Sampler loaded (playing or not)
            engine.setValue(group, "cue_gotoandplay", 1);
        } else {
            engine.setValue(group, "LoadSelectedTrack", 1);
        }
    }
};

ReloopBeatmix24.ShiftSamplerPad = function(channel, control, value, status,
    group) {
    if (value === DOWN) {
        if (engine.getValue(group, "track_samples")) { //Sampler loaded (playing or not)
            if (engine.getValue(group, "play")) { // Sampler is playing
                engine.setValue(group, "cue_gotoandstop", 1);
            } else {
                engine.setValue(group, "eject", 1);
            }
        } else {
            engine.setValue(group, "LoadSelectedTrack", 1);
        }
    } else { // UP
        if (!engine.getValue(group, "track_samples")) { // if empty
            // Set eject back to 0 to turn off the eject button on screen
            engine.setValue(group, "eject", 0);
        }
    }
};

ReloopBeatmix24.SamplerVol = function(channel, control, value, status, group) {
    for (var i = 1; i <= engine.getValue("[Master]", "num_samplers"); i++) {
        engine.setValue("[Sampler" + i + "]", "volume", value / 127.0);
    }
};

// Jog Wheel functions
// ========================================================

ReloopBeatmix24.WheelTouch = function(channel, control, value, status, group) {
    var deck = parseInt(group.substr(8, 1), 10);
    if (value === DOWN) {
        var alpha = 1.0 / 8;
        var beta = alpha / 32;
        engine.scratchEnable(deck, 800, JogRPM, alpha, beta);
    } else {
        engine.scratchDisable(deck);
    }
};

ReloopBeatmix24.WheelTurn = function(channel, control, value, status, group) {
    var newValue = value - 64;
    var deck = parseInt(group.substr(8, 1), 10);

    // In either case, register the movement
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, newValue); // Scratch!
    } else {
        engine.setValue(group, 'jog', newValue / 5); // Pitch bend
    }
};

// Led Feedback functions
// ========================================================
ReloopBeatmix24.AllJogLEDsToggle = function(deck, state, step) {
    step = typeof step !== 'undefined' ? step : 1; // default value
    for (var j = 0x30; j <= 0x3F; j += step) {
        midi.sendShortMsg(deck, j, state);
    }
};

ReloopBeatmix24.deckLoaded = function(value, group, control) {
    var i;
    switch (group.substr(1, 7)) {
        case "Channel":
            var channelChan = parseInt(channelRegEx.exec(group)[1]);
            if (channelChan <= 4) {
                // shut down load button
                midi.sendShortMsg(0x90 + channelChan, 0x50,
                    value ? ON : OFF);

                // shut down jog led on unload
                if ((JogLedLit[group] !== undefined) && !value) {
                    midi.sendShortMsg(0x90 + channelChan,
                        JogBaseLed - (JogLedLit[group] + JogLedNumber - 1) %
                        JogLedNumber, OFF);
                    delete JogLedLit[group];
                }
            }
            break;
        case "Sampler":
            var samplerChan = parseInt(samplerRegEx.exec(group)[1]);
            if (samplerChan <= 8) { // We only handle 8 samplers (1 per pad)
                for (i = 0x91; i <= 0x94; i++) {
                    // PAD1 Mode A
                    midi.sendShortMsg(i, 0x08 - 1 + samplerChan, value ?
                        RED : OFF);
                    // SHIFT+PAD1 Mode A
                    midi.sendShortMsg(i, 0x48 - 1 + samplerChan, value ?
                        RED : OFF);
                    if (samplerChan <= 4) { // Handle first 4 samplers in split mode
                        // PAD5 Mode A+B (sampler 1 in split mode)
                        midi.sendShortMsg(i, 0x14 - 1 + samplerChan, value ?
                            RED : OFF);
                        // SHIFT+PAD5 Mode A+B (sampler 1 in split mode)
                        midi.sendShortMsg(i, 0x1C - 1 + samplerChan, value ?
                            RED : OFF);
                    }
                }
            }
            break;
    }
};

ReloopBeatmix24.SamplerPlay = function(value, group, control) {
    var samplerChan = parseInt(samplerRegEx.exec(group)[1]);
    if (samplerChan <= 8) { // We only handle 8 samplers (1 per pad)
        var ledColor;
        if (value) {
            ledColor = VIOLET;
        } else {
            ledColor = engine.getValue(group, "track_samples") ? RED : OFF;
        }

        // We need to switch off pad lights before changing color otherwise
        // VIOLET to RED transition does not work well. (???)
        for (var i = 0x91; i <= 0x94; i++) {
            // PAD1 Mode A
            midi.sendShortMsg(i, 0x08 - 1 + samplerChan, OFF);
            midi.sendShortMsg(i, 0x08 - 1 + samplerChan, ledColor);
            // SHIFT+PAD1 Mode A
            midi.sendShortMsg(i, 0x48 - 1 + samplerChan, OFF);
            midi.sendShortMsg(i, 0x48 - 1 + samplerChan, ledColor);
            if (samplerChan <= 4) { // Handle first 4 samplers in split mode
                // PAD5 Mode A+B (sampler 1 in split mode)
                midi.sendShortMsg(i, 0x14 - 1 + samplerChan, OFF);
                midi.sendShortMsg(i, 0x14 - 1 + samplerChan, ledColor);
                // SHIFT+PAD5 Mode A+B (sampler 1 in split mode)
                midi.sendShortMsg(i, 0x1C - 1 + samplerChan, OFF);
                midi.sendShortMsg(i, 0x1C - 1 + samplerChan, ledColor);
            }
        }
    }
};

ReloopBeatmix24.loopDefined = function(value, group, control) {
    var channelChan = parseInt(channelRegEx.exec(group)[1]);
    if (channelChan <= 4) {
        midi.sendShortMsg(0x90 + channelChan, 0x44, value < 0 ? OFF :
            VIOLET);
    }
};

ReloopBeatmix24.ChannelPlay = function(value, group, control) {
    // Keep track of the playing state of each channel to avoid
    // calling engine.getValue(group, "play") too often
    if (value) {
        channelPlaying[group] = true;
    } else {
        // Stop JogWheel blinking when we stop playing and resume virtual needle
        if (JogBlinking[group]) {
            engine.stopTimer(jogWheelTimers[group]);
            delete jogWheelTimers[group];
            JogBlinking[group] = false;
            var channelChan = parseInt(channelRegEx.exec(group)[1]);
            ReloopBeatmix24.AllJogLEDsToggle(0x90 + channelChan, OFF, 2);
            engine.trigger(group, "playposition"); // light up jog position led
        }
        channelPlaying[group] = false;
    }
};

// This function will light jog led and is connected to playposition,
// so value here represent the position in the track in the range [-0.14..1.14]
// (0 = beginning, 1 = end)
ReloopBeatmix24.JogLed = function(value, group, control) {

    // time computation
    var trackDuration = engine.getValue(group, "duration");
    var timeLeft = trackDuration * (1.0 - value);
    var channelChan = parseInt(channelRegEx.exec(group)[1]);

    // Start JogWheel blinking if playing and time left < warning time
    if (channelPlaying[group] && timeLeft <= JogFlashWarningTime) {
        if (!JogBlinking[group]) { // only if not already blinking
            // turn jog single led off
            if (JogLedLit[group] !== undefined) { // if some led on, shut it down
                midi.sendShortMsg(0x90 + channelChan,
                    JogBaseLed - (JogLedLit[group] + JogLedNumber - 1) %
                    JogLedNumber, OFF);
                delete JogLedLit[group];
            }
            // light all jog leds
            ReloopBeatmix24.AllJogLEDsToggle(0x90 + channelChan, ON, 2);
            // Set timer for shut off leds
            jogWheelTimers[group] = engine.beginTimer(
                timeLeft <= JogFlashCriticalTime ?
                    JogFlashCriticalInterval : JogFlashWarningInterval,
                "ReloopBeatmix24.jogLedFlash(\"" + group + "\", " + ON + ")",
                true);
            JogBlinking[group] = true;
        }
        return;
    }

    var timePosition = trackDuration * value;
    var rotationNumber = timePosition / RoundTripTime; // number of turn since beginning
    var positionInCircle = rotationNumber - Math.floor(rotationNumber); // decimal part
    var ledToLight = Math.round(positionInCircle * JogLedNumber);
    if (JogLedLit[group] === ledToLight) { // exit if there is no change
        return;
    }
    if (JogLedLit[group] !== undefined) { // if other led on, shut it down
        midi.sendShortMsg(0x90 + channelChan,
            JogBaseLed - (JogLedLit[group] + JogLedNumber - 1) % JogLedNumber,
            OFF);
    }
    midi.sendShortMsg(0x90 + channelChan,
        JogBaseLed - (ledToLight + JogLedNumber - 1) % JogLedNumber, ON);
    JogLedLit[group] = ledToLight; // save last led lit
};

ReloopBeatmix24.jogLedFlash = function(group, state) {
    var chan = parseInt(group.substr(8, 1), 10);

    // toggle all jog leds
    ReloopBeatmix24.AllJogLEDsToggle(0x90 + chan, state ? OFF : ON, 2);

    var timeleft = engine.getValue(group, "duration") * (1.0 - engine.getValue(
        group, "playposition"));

    if (timeleft < JogFlashWarningTime) {
        // Set timer for leds shut off
        var nextTime = (timeleft < JogFlashCriticalTime ?
            JogFlashCriticalInterval : JogFlashWarningInterval);
        jogWheelTimers[group] = engine.beginTimer(nextTime,
            "ReloopBeatmix24.jogLedFlash(\"" + group + "\", " +
            (state ? OFF : ON) + ")", true);
    } else { // Back in time ?
        // shut off all jog leds
        ReloopBeatmix24.AllJogLEDsToggle(0x90 + chan, OFF);
        delete jogWheelTimers[group];
        // JogLed callback will restart led cycling
        JogBlinking[group] = false;
    }
};

// Effects functions
// ========================================================
ReloopBeatmix24.FxModeLedFlash = function(step, mode) {
    var i;
    var ledValue = (step % 2) ? ON : OFF;
    if (step >= 7) {
        for (i = 1; i <= 4; i++) {
            // engine.trigger should be sufficient, but...
            engine.trigger("[EffectRack1_EffectUnit1]", "group_[Channel" +
                i + "]_enable");
            engine.trigger("[EffectRack1_EffectUnit2]", "group_[Channel" +
                i + "]_enable");
            // Workaround for bug #1607277 as engine.trigger doesn't work well
            var newValue = engine.getValue("[EffectRack1_EffectUnit1]",
                "group_[Channel" + i + "]_enable");
            midi.sendShortMsg(0x90 + i, 0x25, newValue ? ON : OFF);
            midi.sendShortMsg(0x90 + i, 0x25 + SHIFT, newValue ? ON : OFF);
            newValue = engine.getValue("[EffectRack1_EffectUnit2]",
                "group_[Channel" + i + "]_enable");
            midi.sendShortMsg(0x90 + i, 0x24, newValue ? ON : OFF);
            midi.sendShortMsg(0x90 + i, 0x24 + SHIFT, newValue ? ON : OFF);
        }
    } else {
        for (i = 0x91; i <= 0x94; i++) {
            midi.sendShortMsg(i, 0x26 - mode, ledValue);
            midi.sendShortMsg(i, 0x26 + SHIFT - mode, ledValue);
        }
        engine.beginTimer(150, "ReloopBeatmix24.FxModeLedFlash(" + (step + 1) +
            ", " + mode + ")", true);
    }
};

ReloopBeatmix24.FxModeCallback = function(group, mode) {
    FxMode = mode;
    FxModeLongPressed[group] = true;
    delete FxModeTimers[group];
    // give some visual feedback (blink led 3 times)
    for (var i = 0x91; i <= 0x94; i++) {
        midi.sendShortMsg(i, 0x26 - mode, OFF);
        midi.sendShortMsg(i, 0x26 + SHIFT - mode, OFF);
    }
    engine.beginTimer(150, "ReloopBeatmix24.FxModeLedFlash(1, " + mode + ")",
        true);
};

// This function activate Fx Unit 1 or 2 for the selected Channel on short press
// and toggle Fx Mode on long press (>1s)
// It is mapped to SHIFT + PITCHBEND+/- (FX1 and FX2)
ReloopBeatmix24.ActivateFx = function(channel, control, value, status, group) {
    // Calculate Fx num based on midi control (0x66 for Fx1 and 0x67 for Fx2)
    var FxNum = control - 0x65;
    if (value === DOWN) {
        if (FxModeTimers[group]) {
            engine.stopTimer(FxModeTimers[group]);
            delete FxModeTimers[group];
        }
        FxModeLongPressed[group] = false;
        FxModeTimers[group] = engine.beginTimer(1000,
            "ReloopBeatmix24.FxModeCallback(\"" + group + "\", " + FxNum + ")",
            true);
    } else { // UP
        if (FxModeLongPressed[group]) { // long press
            // Nothing to do, this has already been done in callback function
            FxModeLongPressed[group] = false;
        } else { // short press
            // stop & delete timer
            engine.stopTimer(FxModeTimers[group]);
            delete FxModeTimers[group];
            script.toggleControl("[EffectRack1_EffectUnit" + FxNum + "]",
                "group_" + group + "_enable");
        }
    }
};

ReloopBeatmix24.FxKnobTurn = function(channel, control, value, status, group) {
    if (FxMode == 1) {
        var parameter = control;
        engine.setParameter(group, "parameter" + parameter.toString(),
            script.absoluteLin(value, 0, 1));
    }
    // Nothing in multi-effect mode
};

ReloopBeatmix24.ShiftFxKnobTurn = function(channel, control, value, status,
    group) {
    if (FxMode == 1) {
        var parameter = 3 + control - SHIFT;
        engine.setParameter(group, "parameter" + parameter.toString(),
            script.absoluteLin(value, 0, 1));
    } else {
        var effectUnit = parseInt(group.substr(23, 1), 10);
        var Effect = control - SHIFT;
        var storeIndex = "FX" + Effect.toString() + "U" + effectUnit.toString();
        if (storeIndex in previousValue) {
            if (value - previousValue[storeIndex] > 5) {
                engine.setValue("[EffectRack1_EffectUnit" + effectUnit +
                    "_Effect" + Effect + "]", "next_effect", 1);
                engine.setValue("[EffectRack1_EffectUnit" + effectUnit +
                    "_Effect" + Effect + "]", "next_effect", 0);
                previousValue[storeIndex] = value;
            } else if (value - previousValue[storeIndex] < -5) {
                engine.setValue("[EffectRack1_EffectUnit" + effectUnit +
                    "_Effect" + Effect + "]", "prev_effect", 1);
                engine.setValue("[EffectRack1_EffectUnit" + effectUnit +
                    "_Effect" + Effect + "]", "prev_effect", 0);
                previousValue[storeIndex] = value;
            }
        } else {
            previousValue[storeIndex] = value;
        }
    }
};

// Fx knobs send Note-Off MIDI signal when at 0 and Note-On when leaving zero.
// These 0x9 MIDI signals are mapped to this function
ReloopBeatmix24.FxKnobOnOff = function(channel, control, value, status, group) {
    if (FxMode !== 1) {
        engine.setValue(group, "enabled", value ? 1 : 0);
    }
    // Nothing in single-effect mode
};

ReloopBeatmix24.EffectClearTimerCallBack = function(group) {
    engine.setValue(group, "clear", 0);
};

ReloopBeatmix24.ShiftFxKnobOnOff = function(channel, control, value, status, group) {
    if (FxMode !== 1) {
        if (value === UP) {
            engine.setValue(group, "clear", 1);
            engine.beginTimer(100,
                "ReloopBeatmix24.EffectClearTimerCallBack(\"" + group +
                "\")", true);
        }
    }
    // Nothing in single-effect mode
};

ReloopBeatmix24.FxEncoderTurn = function(channel, control, value, status, group) {
    var newValue = value - 0x40;
    if (FxMode == 1) {
        engine.setValue(group, newValue > 0 ? "mix_up" : "mix_down", 1);
    } else {
        engine.setValue(group, newValue > 0 ? "super1_up" : "super1_down", 1);
    }
};

ReloopBeatmix24.ShiftFxEncoderTurn = function(channel, control, value, status,
    group) {
    var newValue = value - 0x40;
    if (FxMode == 1) {
        engine.setValue(group, "chain_selector", newValue);
    } else {
        engine.setValue(group, newValue > 0 ? "mix_up" : "mix_down", 1);
    }
};

ReloopBeatmix24.FxEncoderPush = function(channel, control, value, status, group) {
    if (value === DOWN) {
        script.toggleControl(group, "enabled");
    }
};

ReloopBeatmix24.ShiftFxEncoderPush = function(channel, control, value, status,
    group) {
    engine.setValue(group, "clear", value);
};

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
 * Revision history
 * ----------------
 * 2016-07-31 - Initial revision for Mixxx 2.1.0
 ***********************************************************************
 *                           GPL v2 licence
 *                           --------------
 * Reloop Beatmix controller script script 1.1 for Mixxx 2.1.0
 * Copyright (C) 2015 Sébastien Blaisot
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

var ON = 0x7F,
    OFF = 0x00,
    RED = 0x7F,
    BLUE = 0x55,
    VIOLET = 0x2A,
    SHIFT = 0x40,
    DOWN = 0x7F,
    UP = 0x00;

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

ReloopBeatmix24.connectControls = function(disconnect) {

    // Channels controls
    for (var i = 1; i <= 4; i++) {
        engine.connectControl("[Channel" + i + "]", "track_samples",
            "ReloopBeatmix24.deckLoaded", disconnect);
        engine.connectControl("[Channel" + i + "]", "play",
            "ReloopBeatmix24.startJogLedSpinnie", disconnect);
        engine.connectControl("[Channel" + i + "]", "loop_end_position",
            "ReloopBeatmix24.loopDefined", disconnect);
        engine.softTakeover("[Channel" + i + "]", "rate", disconnect ?
            false : true);
        engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel" + i +
            "]_enable", 0);
        engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel" + i +
            "]_enable", 0);
    }

    // Samplers controls
    for (i = 1; i <= 8; i++) {
        engine.connectControl("[Sampler" + i + "]", "track_samples",
            "ReloopBeatmix24.deckLoaded", disconnect);
        engine.connectControl("[Sampler" + i + "]", "play",
            "ReloopBeatmix24.SamplerPlay", disconnect);
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

    print("Reloop Beatmix: " + id + " initialized.");
};

ReloopBeatmix24.shutdown = function() {
    ReloopBeatmix24.TurnLEDsOff(); // Turn off all LEDs
    ReloopBeatmix24.connectControls(true);
    print("Reloop Beatmix: " + ReloopBeatmix24.id + " shut down.");
};

// Button functions.
// ========================================================
ReloopBeatmix24.GetNextRange = function(OV) {
    var len = RateRangeArray.length;
    var pos = RateRangeArray.indexOf(OV);
    // If OV is not found in the array, pos == -1 and (pos+1) == 0,
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
    engine.setValue(group, "rate", -script.pitch(control, value, status));
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
        engine.scratchEnable(deck, 1600, 33 + 1 / 3, alpha, beta);
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
        engine.setValue(group, 'jog', newValue); // Pitch bend
    }
};

// Led Feedback functions
// ========================================================
ReloopBeatmix24.AllJogLEDsToggle = function(deck, state, step) {
    var step = typeof step !== 'undefined' ? step : 1; // default value
    for (var j = 0x30; j <= 0x3F; j += step) {
        midi.sendShortMsg(deck, j, state);
    }
};

ReloopBeatmix24.deckLoaded = function(value, group, control) {
    var i;
    switch (group.substr(1, 7)) {
        case "Channel":
            var channelRegEx = /\[Channel(\d+)\]/;
            var channelChan = parseInt(channelRegEx.exec(group)[1]);
            if (channelChan <= 4) {
                midi.sendShortMsg(0x90 + channelChan, 0x50, value ? ON :
                    OFF);
            }
            break;
        case "Sampler":
            var samplerRegEx = /\[Sampler(\d+)\]/;
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
    var samplerRegEx = /\[Sampler(\d+)\]/;
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
    var channelRegEx = /\[Channel(\d+)\]/;
    var channelChan = parseInt(channelRegEx.exec(group)[1]);
    if (channelChan <= 4) {
        midi.sendShortMsg(0x90 + channelChan, 0x44, value < 0 ? OFF :
            VIOLET);
    }
};

ReloopBeatmix24.startJogLedSpinnie = function(value, group, control) {

    var chan = parseInt(group.substr(8, 1), 10);

    // Stop and remove reference to old timer, shut off all leds,
    // we reached the next beat
    if (jogWheelTimers[group]) {
        engine.stopTimer(jogWheelTimers[group]);
        delete jogWheelTimers[group];
    }
    ReloopBeatmix24.AllJogLEDsToggle(0x90 + chan, OFF);

    if (value) {
        // Lit top led
        midi.sendShortMsg(0x90 + chan, 0x30, ON);

        // Set timer for next led in 2 seconds / 16 led intervals
        jogWheelTimers[group] = engine.beginTimer(2000 / 16,
            "ReloopBeatmix24.nextJogLedSpinnie(\"" + group +
            "\", 0x30)",
            true);
    }
};

ReloopBeatmix24.nextJogLedSpinnie = function(group, prevLed) {
    var chan = parseInt(group.substr(8, 1), 10);

    // cleanup timer reference
    delete jogWheelTimers[group];

    // shut off previous led
    midi.sendShortMsg(0x90 + chan, prevLed, OFF);

    if (engine.getValue(group, "play")) {

        var timeleft = engine.getValue(group, "duration") *
            (1.0 - engine.getValue(group, "playposition"));

        if (timeleft > 30) {
            // lit next led
            if (prevLed == 0x30) { //cycling
                prevLed = 0x40;
            }

            midi.sendShortMsg(0x90 + chan, prevLed - 1, ON);

            // Set timer for next led
            jogWheelTimers[group] = engine.beginTimer(2000 / 16,
                "ReloopBeatmix24.nextJogLedSpinnie(\"" + group + "\", " +
                (prevLed - 1) + ")", true);
        } else {
            // lit all jog leds
            ReloopBeatmix24.AllJogLEDsToggle(0x90 + chan, ON, 2);
            // Set timer for shut off leds
            jogWheelTimers[group] = engine.beginTimer(500,
                "ReloopBeatmix24.jogLedFlash(\"" + group + "\", " + ON +
                ")",
                true);
        }
    }
};

ReloopBeatmix24.jogLedFlash = function(group, state) {
    var chan = parseInt(group.substr(8, 1), 10);

    // toggle all jog leds
    ReloopBeatmix24.AllJogLEDsToggle(0x90 + chan, state ? OFF : ON, 2);

    var timeleft = engine.getValue(group, "duration") * (1.0 - engine.getValue(
        group, "playposition"));
    var nextTime = (timeleft < 15 ? 200 : 400);

    if (timeleft < 30) {
        // Set timer for leds shut off
        jogWheelTimers[group] = engine.beginTimer(nextTime,
            "ReloopBeatmix24.jogLedFlash(\"" + group + "\", " +
            (state ? OFF : ON) + ")", true);
    } else { // Back in time ?
        // shut off all jog leds
        ReloopBeatmix24.AllJogLEDsToggle(0x90 + chan, OFF);
        // Lit top led
        midi.sendShortMsg(0x90 + chan, 0x30, ON);

        // Set timer for next led in 2 seconds / 16 led intervals
        jogWheelTimers[group] = engine.beginTimer(2000 / 16,
            "ReloopBeatmix24.nextJogLedSpinnie(\"" + group +
            "\", 0x30)",
            true);
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

// This function activate Fx Unit 1 or 2 for the selected Channel.
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
                previousValue[storeIndex] = value;
            } else if (value - previousValue[storeIndex] < -5) {
                engine.setValue("[EffectRack1_EffectUnit" + effectUnit +
                    "_Effect" + Effect + "]", "prev_effect", 1);
                previousValue[storeIndex] = value;
            }
        } else {
            previousValue[storeIndex] = value;
        }
    }
};

ReloopBeatmix24.FxOff = function(channel, control, value, status, group) {
    if (FxMode !== 1) {
        engine.setValue(group, "enabled", value ? 1 : 0);
    }
    // Nothing in single-effect mode
};

ReloopBeatmix24.EffectClearTimerCallBack = function(group) {
    engine.setValue(group, "clear", 0);
};

ReloopBeatmix24.ShiftFxOff = function(channel, control, value, status, group) {
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

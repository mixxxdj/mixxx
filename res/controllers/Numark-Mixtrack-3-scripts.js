/***********************************************************************
 * ==============      Configuration Options           =================
 * valid values are "true" or "false" unless specified
 **********************************************************************/
// TrackEndWarning: "true": when you reach the end of the track,
// the jog wheel Button will flash. "false": No flash of Jog Wheel Button
var TrackEndWarning = true;

//iCutEnabled: iCut mode will automatically cut your track with the cross fader
// when SHIFT enabled and scratching with the jog wheel
var iCutEnabled = true;

//activate PFL of deck on track load
var smartPFL = true;

//Disable Play on Sync button Double Press
var noPlayOnSyncDoublePress = false;

// Shift+Filter control behavior
// true (default) - FX parameter 4 (when the FX is focused)
// false - Channel Gain
var ShiftFilterFX4 = true;

// allow pitch bend with wheel when wheel is not active
var PitchBendOnWheelOff = true;

/**************************
 *  scriptpause
 * ---------------
 * period (in ms) while the script will be paused when sending messages
 * to the controller in order to avoid too much data flow at once in the same time.
 *  - default value : 5 ms
 *  - To disable : 0;
 **************************/
var scriptpause = 0;

/**************************
 * Constants for scratching :
 **************************/
var intervalsPerRev = 1200;
var rpm = 33 + 1 / 3;   // Like a real vinyl !!! :)
var alpha = 1.0 / 8;    // Adjust to suit.
var beta = alpha / 32;  // Adjust to suit.

/**************************
 * Loop Size array
 * first 4 values used for Autoloop Not shifted
 * last 4 values used for Autoloop Shifted
 **************************/
var loopsize = [2, 4, 8, 16, 0.125, 0.25, 0.5, 1];

/************************  GPL v2 licence  *****************************
 * Numark Mixtrack Pro 3 controller script
 * Author: Stéphane Morin
 * Modified by: Radu Suciu
 *
 * Key features
 * ------------
 * - ICUT effect for scratching
 * - Fader Start
 * - press/double press/long press handling
 * - Smart PFL
 * - 4 deck support
 * - Full effect chains support from Deere Skin
 **********************************************************************
 * User References
 * ---------------
 * Wiki/manual : http://mixxx.org/wiki/doku.php/numark_mixtrack_pro_3
 * support forum : http://mixxx.org/forums/viewtopic.php?f=7&p=27984#p27984
 * e-mail : steph@smorin.com
 *
 * Thanks
 * ----------------
 * Thanks to Chloé AVRILLON (DJ Chloé) and authors of other scripts and particularly
 * to authors of Numark Dj2Go, KANE QuNeo, Vestax-VCI-400
 *
 * Revision history
 * ----------------
 * 2016-01-12 (V0.9) to 2016-01-15 (1.0 beta 3) - Chloé AVRILLON
 * 2016-02-17 (1.0 beta 4) 2016-04-08 (V1.3 )- Stéphane Morin
 * 2016-04-08 (1.3) - Stéphane Morin - https://github.com/mixxxdj/mixxx/pull/905
 * 2016-09-14 (1.31) - Stefan Mikolajczyk - https://github.com/mixxxdj/mixxx/pull/1012
 * 2016-04-08 (1.4) to 2017-01-05 (2.2) - Stéphane Morin - https://github.com/mixxxdj/mixxx/pull/1014
 * 2017-02-10 (2.3) - Radu Suciu - https://github.com/mixxxdj/mixxx/pull/1180
 * 2018-01-20 (2.4) - NTMusic - Shift+Filter can control either FX4 or channel gain
 *
 ***********************************************************************
 *                           GPL v2 licence
 *                           --------------
 * Numark Mixtrack Pro 3 controller script 2.4 for Mixxx 2.1+
 * Copyright (C) 2016 Stéphane Morin
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
/* global print                                                       */
/* global midi                                                        */
/* global script                                                      */
/* jshint sub:true                                                    */
/* jshint shadow:true                                                 */
////////////////////////////////////////////////////////////////////////


var NumarkMixtrack3 = {
    group: "[Master]",
    decks: [],
    samplers: []
};

// Array of Objects can be created

// Global constants/variables
var ON = 0x7F,
    OFF = 0x00,
    DOWN = 0x7F;
var QUICK_PRESS = 1,
    DOUBLE_PRESS = 2,
    LONG_PRESS = 3;

//LEDs constants
var ledCategories = {
    "master": 0,
    "channel1": 1,
    "channel2": 2,
    "meters": 32
};

var leds = {
    // Master: all are first byte 0x90 ( = 0x90+ledcatecories.master )
    "headphones1": 0x0e,
    "headphones2": 0x0f,
    "all": 0x75,
    // Deck 1: first byte 0x91 ( = 0x90+ledcatecories.channel1 = 0x90+1 )
    // Deck 2: first byte 0x92 ( = 0x90+ledcatecories.channel2 = 0x90+2 )
    "jogWheelsInScratchMode": 0x06,
    "loopin": 0x13,
    "loopout": 0x14,
    "reloopExit": 0x15,
    "loopHalve": 0x16,
    "hotCue1": 0x1b,
    "hotCue2": 0x1c,
    "hotCue3": 0x1d,
    "hotCue4": 0x1e,
    "cue": 0x03,
    "sync": 0x02,
    "play": 0x01,
    "fx1": 0x07,
    "fx2": 0x08,
    "fx3": 0x09,
    "tap": 0x0a,
    "padLoop1": 0x17,
    "padLoop2": 0x18,
    "padLoop3": 0x19,
    "padLoop4": 0x1A,
    "padSampler1": 0x20,
    "padSampler2": 0x21,
    "padSampler3": 0x22,
    "padSampler4": 0x23,
    "padSampler5": 0x20,
    "padSampler6": 0x21,
    "padSampler7": 0x22,
    "padSampler8": 0x23,

    // Meters: first byte 0xb0 ( = 0x90+ledcatecories.meters )
    "meter1": 0x02,
    "meter2": 0x03
};

var PADcolors = {
    "black": 0,
    "blue": 32,
    "yellow": 96,
    "purple": 127
};

// Utilities
// =====================================================================

function pauseScript(ms) {
    if (ms > 0) {
        var startDate = new Date();
        var currentDate = null;
        while (currentDate - startDate < ms) {
            currentDate = new Date();
        }
    }
}

Math.sign = Math.sign || function(x) {
    x = +x; // convert the parameter into a number
    if (x === 0 || isNaN(x)) {
        return x;
    }
    return x > 0 ? 1 : -1;
};

function sendShortMsg(control, midino, value) {
    midi.sendShortMsg(control, midino, value);
}

function parameterSoftTakeOver(group, control, value) {
    var threshold = 0.07; //on the CMD Studio 4a this threshold got the right balance
    //between smooth takeover and keeping up with quick turns, but you can adjust the value to suit your needs
    var currentKnobVal = value / 127;
    var currentParamVal = engine.getParameter(group, control);
    var spread = Math.abs(currentParamVal - currentKnobVal);

    if (spread < threshold) {
        engine.setParameter(group, control, currentKnobVal); //set the new value
    } else {
        return; //do nothing until we get close
    }
}

// =====================================================================
// Reusable Objects (special buttons handling, LEDs, iCUT and Jog wheels)
// =====================================================================

// LED class object
var LED = function(control, midino) {
    this.control = control;
    this.midino = midino;
    this.lit = 0;
    this.flashTimer = 0;
    this.flashTimer2 = 0;
    this.flashOnceTimer = 0;
    this.flashDuration = 0;
    this.flashOnceDuration = 0;
    this.num_ms_on = 0;
    this.valueon = 0;
    this.num_ms_off = 0;
    this.flashCount = 0;
    this.relight = 0;
    this.valueoff = 0;
};

// public : light on/off
LED.prototype.onOff = function(value) {
    // stop pending flashing effects now
    if (this.flashTimer !== 0) {
        engine.stopTimer(this.flashTimer);
        this.flashTimer = 0;
        this.flashDuration = 0;
    }

    if (this.flashTimer2 !== 0) {
        engine.stopTimer(this.flashTimer2);
        this.flashTimer2 = 0;
        this.flashDuration = 0;
    }

    if (this.flashOnceTimer !== 0) {
        engine.stopTimer(this.flashOnceTimer);
        this.flashOnceTimer = 0;
        this.flashOnceDuration = 0;
    }

    sendShortMsg(this.control, this.midino, value);
    pauseScript(scriptpause);
    this.lit = value;
};

// public : make a light flashing
// ------------------------------
// num_ms_on : number of ms the light should stay enlighted when blinking
// value : value to send to the controller to lit it up,
//         generally 0x00 means OFF, 0x7F means ON, but the light
//         can receive some other values if it can have various colors
// num_ms_off : number of ms the light should be switched off when blinking
// flashcount : number of time the light should blink (3 times ? 10 times ? only once (1) ?
//              if set to 0 or not set, flashes for ever, can be stopped with flashOff()
// relight : once the light has finished to blink, should we restore it in its original state (true) or must it be switched off (false).
//           if not set, it considers it as a switch off (default=false)
// valueoff : like "value". That permits for instance with two colors (once red(on), once blue(off), once red(on), etc...)

LED.prototype.flashOn = function(num_ms_on, value, num_ms_off, flashCount) {
    // stop pending timers
    this.flashOff();

    // init
    this.flashDuration = num_ms_on;
    this.num_ms_on = num_ms_on;
    this.valueon = value;
    this.num_ms_off = num_ms_off;
    this.flashCount = flashCount;

    // 1st flash
    // This is because the permanent timer below takes
    // num_ms_on milisecs before first flash.
    this.flashOnceOn(num_ms_on, value);

    if (flashCount !== 1) {
        // flashcount == 0 means permanent flash,
        // flashcount > 0 , means temporary flash, first flash already done,
        // so we don't need this part  if flashcount == 1

        // permanent timer
        this.flashTimer = engine.beginTimer(num_ms_on + num_ms_off, ()=>this.flashOnceOn(false));
    }

    if (flashCount > 1) {
        // flashcount>0 , means temporary flash, first flash already done,
        // so we don't need this part  if flashcount=1
        // temporary timer. The end of this timer stops the permanent flashing

        if (flashCount > 0) {
            this.flashTimer2 = engine.beginTimer(flashCount * (num_ms_on + num_ms_off) - num_ms_off, ()=>this.stopflash(this.relight), true);
        }
    }
};

// private : relight=true : restore light state before it was flashing
// this is a call back function (called in flashon() )
LED.prototype.flashOff = function(relight) {
    // stop permanent timer if any
    if (this.flashTimer !== 0) {
        engine.stopTimer(this.flashTimer);
        // reset flash variables to 0
        this.flashTimer = 0;
    }
    if (this.flashTimer2 !== 0) {
        engine.stopTimer(this.flashTimer2);
        // reset flash variables to 0
        this.flashTimer2 = 0;
    }

    this.flashDuration = 0;

    if (relight) {
        this.onOff(this.lit);
    } else {
        this.onOff(OFF);
    }
};

// private : relight=true : restore light state before it was flashing
// this is a call back function (called in flashon() )
LED.prototype.stopflash = function(relight) {
    // stop permanent timer
    if (this.flashTimer !== 0) {
        engine.stopTimer(this.flashTimer);
    }
    // reset flash variables to 0
    this.flashTimer = 0;
    this.flashTimer2 = 0;
    this.flashDuration = 0;
    this.flashOff(relight);
};

// private : call back function (called in flashon() )
LED.prototype.flashOnceOn = function(relight) {
    sendShortMsg(this.control, this.midino, this.valueon);
    pauseScript(scriptpause);
    this.flashOnceDuration = this.num_ms_on;
    this.flashOnceTimer = engine.beginTimer(this.num_ms_on - scriptpause, ()=>this.flashOnceOff(relight), true);
};

// private :call back function (called in flashOnceOn() )
LED.prototype.flashOnceOff = function(relight) {
    this.flashOnceTimer = 0;
    this.flashOnceDuration = 0;

    if (relight) {
        sendShortMsg(this.control, this.midino, this.lit);
        pauseScript(scriptpause);
    } else {
        sendShortMsg(this.control, this.midino, this.valueoff);
        pauseScript(scriptpause);
        this.lit = OFF;
    }
};

// ********* special buttons handlers (SHIFT ,LOAD, PFL and SYNC buttons)
// =======================  SingleDoubleBtn
// callback           : Callback function you have to provide (see end of
//                      the code), that will return the original event
//                      parameters (channel, control, value, status, group)
//                      and the kind of press event affecting your button
//                      (eventkind).
//                      This callback will be triggered as soon as you
//                      press the button a second time (Value will be
//                      equal to DOWN), or the Long press is asserted
//                      (value = DOWN because you are still holding down
//                      the button or value=UP because you have released
//                      the button only once before it becomes a long press).
// doublePressTimeOut : delay in ms above which a second press on the
//                      button will not be considered as a potential double
//                      but as a new press cycle event (default = 400ms).
var SingleDoubleBtn = function(callback, doublePressTimeOut) {
    this.channel = 0;
    this.control = 0;
    this.value = 0;
    this.status = 0;
    this.group = "";
    this.callback = callback;
    this.doublePressTimeOut = doublePressTimeOut || 400;
    this.buttonCount = 0;
    this.buttonTimer = 0;
};

// Button pressed
SingleDoubleBtn.prototype.buttonDown = function(channel, control, value, status, group) {
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;

    if (this.buttonTimer === 0) { // first press
        this.buttonTimer = engine.beginTimer(this.doublePressTimeOut, this.buttonDecide.bind(this), true);
        this.buttonCount = 1;
    } else { // 2nd press (before timer's out)
        engine.stopTimer(this.buttonTimer);
        this.buttonTimer = 0;
        this.buttonCount = 2;
        this.buttonDecide();
    }
};

// Take action
SingleDoubleBtn.prototype.buttonDecide = function() {
    this.buttonTimer = 0;
    this.callback(this.channel, this.control, this.value, this.status, this.group, this.buttonCount);
    this.buttonCount = 0;
};

// =======================  LongShortBtn
// callback           : callback function you have to provide (see end of the code), that will return
//                      the original event parameters (channel, control, value, status, group)
//                      and the kind of press event affecting your button (eventkind)
//                      This callback will be called once you release the button
//                      (Value will be equal to UP). You must provide this parameter.
// longPressThreshold : delay in ms above which a first press on the
//                      button will be considered as a Long press (default = 500ms).
//                      This parameter is optional.
// callBackOKLongPress : This callback will give you the same values than the first one
//                       but it will be triggered as soon as the Long press is taken
//                       into account ( at this moment, value = DOWN because you are still
//                       holding down the button). This permits for instance to lit up a light indicating
//                       the user that he/she can release the button. This callback occurs before the first one.
//                       This parameter is optional.
// Like that, you can decide to put the code for the long press in either callback function
var LongShortBtn = function(callback, longPressThreshold, callBackOKLongPress) {
    this.callback = callback;
    this.channel = 0;
    this.control = 0;
    this.value = 0;
    this.status = 0;
    this.group = "";
    this.callBackOKLongPress = callBackOKLongPress;
    this.longPressThreshold = longPressThreshold || 500;
    this.buttonLongPress = false;
    this.buttonLongPressTimer = 0;
};

// Timer's call back for long press
LongShortBtn.prototype.buttonAssertLongPress = function() {
    this.buttonLongPress = true;
    //the timer was stopped, we set it to zero
    this.buttonLongPressTimer = 0;

    if (typeof this.callBackOKLongPress === "function") {
        this.callBackOKLongPress(this.channel, this.control, this.value, this.status, this.group, LONG_PRESS);
    }
};

LongShortBtn.prototype.buttonDown = function(channel, control, value, status, group) {
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;
    this.buttonLongPress = false;
    this.buttonLongPressTimer = engine.beginTimer(this.longPressThreshold, this.buttonAssertLongPress.bind(this), true);
};

LongShortBtn.prototype.buttonUp = function() {
    if (this.buttonLongPressTimer !== 0) {
        engine.stopTimer(this.buttonLongPressTimer);
        this.buttonLongPressTimer = 0;
    }

    if (this.buttonLongPress) {
        this.callback(this.channel, this.control, this.value, this.status, this.group, LONG_PRESS);
    } else {
        this.callback(this.channel, this.control, this.value, this.status, this.group, QUICK_PRESS);
    }
};

// =======================  LongShortDoubleBtn
// callback           : Callback function you have to provide (see end of
//                      the code), that will return the original event
//                      parameters (channel, control, value, status, group)
//                      and the kind of press event affecting your button
//                      (eventkind).
//                      This callback will be triggered as soon as you
//                      press the button a second time (Value will be
//                      equal to DOWN), or the Long press is asserted
//                      (value = DOWN because you are still holding down
//                      the button or value=UP because you have released
//                      the button only once before it becomes a long press).
// longPressThreshold : delay in ms above which a first press on the
//                      button will be considered as a Long press (default = 500ms).
// doublePressTimeOut : delay in ms above which a second press on the
//                      button will not be considered as a potential double
//                      but as a new press cycle event (default = 400ms).

var LongShortDoubleBtn = function(callback, longPressThreshold, doublePressTimeOut) {
    this.callback = callback;
    this.channel = 0;
    this.control = 0;
    this.value = 0;
    this.status = 0;
    this.group = "";
    this.longPressThreshold = longPressThreshold || 500;
    this.doublePressTimeOut = doublePressTimeOut || 400;
    this.buttonTimer = 0;
    this.buttonLongPress = false;
    this.buttonLongPressTimer = 0;
    this.buttonCount = 0;
};

// Timer's call back for long press
LongShortDoubleBtn.prototype.buttonAssertLongPress = function() {
    this.buttonLongPress = true;
    // the timer was stopped, we set it to zero
    this.buttonLongPressTimer = 0;
    // let's take action of the long press
    this.buttonDecide();
};

// Timer's callback for single press/double press
LongShortDoubleBtn.prototype.buttonAssert1Press = function() {
    // Short Timer ran out before it was manually stopped by release
    // of the button (buttonUp):
    // for sure it is a single click (short or long), we will know
    // when button will be released or when longtimer will stop by itself

    // the timer was stopped, we set it to zero
    this.buttonTimer = 0;
    this.buttonCount = 1;
    if (this.buttonLongPressTimer === 0) {
        // long press timer was stopped (short press)
        //take action
        this.buttonDecide();
    }
};

// Button pressed (function called by mapper's code)
LongShortDoubleBtn.prototype.buttonDown = function(channel, control, value, status, group) {
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;

    if (this.buttonCount === 0) { //first press (inits)
        // 1st press
        this.buttonCount = 1;
        // and short press
        this.buttonLongPress = false;

        this.buttonLongPressTimer = engine.beginTimer(
            this.longPressThreshold, this.buttonAssertLongPress.bind(this), true
        );

        this.buttonTimer = engine.beginTimer(
            this.doublePressTimeOut, this.buttonAssert1Press.bind(this), true
        );

    } else if (this.buttonCount === 1) { // 2nd press (before short timer's out)
        // stop timers...
        if (this.buttonLongPressTimer !== 0) {
            engine.stopTimer(this.buttonLongPressTimer);
            this.buttonLongPressTimer = 0;
        }
        // we stopped the timer, we have to set it to zero.
        // You must have this reflex : "stopTimer(timer)/timer=0" in mind
        // so that you can test later on if it is active or not. Other else
        // it's value stays with the one given by engine.beginTimer

        // "stopTimer(timer)/timer=0"
        if (this.buttonTimer !== 0) {
            engine.stopTimer(this.buttonTimer);
            this.buttonTimer = 0;
        }

        // 2nd press
        this.buttonCount = 2;

        // ...and take action immediately
        this.buttonDecide();
    } // else :
    // 2nd press after short timer's out, this cannot happen,
    // do nothing
};

// Button released  (function called by mapper's code)
LongShortDoubleBtn.prototype.buttonUp = function() {
    // button released
    if (this.buttonLongPress === false) {
        // long press was not asserted by timer (buttonAssertLongPress)
        // Button is released before timer's out

        // If first Buttun up, long timer is still running
        // stop long timer if it is still running, keep short timer,
        // longpress will never happen
        if (this.buttonLongPressTimer !== 0) {
            engine.stopTimer(this.buttonLongPressTimer);
            this.buttonLongPressTimer = 0;
        }
    } // else :
    // longpressed is confirmed, we already took action in buttonAssertLongPress
};

// Take actions and call callback
LongShortDoubleBtn.prototype.buttonDecide = function() {
    if (this.buttonLongPressTimer !== 0) {
        engine.stopTimer(this.buttonLongPressTimer);
    }

    this.buttonLongPressTimer = 0;
    this.buttonTimer = 0;

    if (this.buttonLongPress) {
        this.callback(this.channel, this.control, this.value, this.status, this.group, LONG_PRESS);
    } else {
        if (this.buttonCount === 2) {
            this.callback(this.channel, this.control, this.value, this.status, this.group, DOUBLE_PRESS);
        } else { // We pressed sync only once
            this.callback(this.channel, this.control, this.value, this.status, this.group, QUICK_PRESS);
        }
    }
    // re-init
    this.buttonCount = 0;
    this.buttonLongPress = false;
};

// *************************************************
// iCut mode management
// *************************************************
// this mode simulates a scratch routine. When the jog wheel is turned back
// the crossfader closes, when the jog wheel is turned forward the crossfader
// will open.

var AutoCut = function(decknum) {
    this.decknum = decknum;
    this.timer = 0;
    this.enabled = false;
};

AutoCut.prototype.On = function() {
    if (!this.enabled) {
        this.enabled = true;
        engine.softTakeover("[Master]", "crossfader", false);
    }
};

AutoCut.prototype.FaderCut = function(jogValue, decknum) {
    if (this.enabled) {
        var direction = Math.sign(jogValue); //Get Jog wheel direction
        // Backward=-1 (close), forward =0 (open)
        if (direction > 0) {
            direction = 0;
        }
        // Left Deck direction = 0 (open : crossfader to zero) or 1 (close : crossfader to the right)
        // Right Deck direction = 0 (open : crossfader to zero) or -1 (close : crossfader to the left)
        if (decknum === 1) {
            direction = -direction;
        } // else direction is of the good sign
        engine.setValue("[Master]", "crossfader", direction);
    }
};

AutoCut.prototype.Off = function() {
    if (this.enabled) {
        this.enabled = false;
        engine.setValue("[Master]", "crossfader", 0);
        engine.softTakeover("[Master]", "crossfader", true);
    }
};

// *****************************************************************
// Jog wheel management (scratching, bending, ...)
// *************************************************

var Jogger = function(group, decknum) {
    this.decknum = decknum;
    this.group = group;
    this.wheelTouchInertiaTimer = 0;
    this.iCUT = new AutoCut(decknum);
};

// ******************************************************************
// Decks
// ******************************************************************
NumarkMixtrack3.deck = function(decknum) {
    this.decknum = decknum;
    this.group = "[Channel" + decknum + "]";
    this.loaded = false;
    this.jogWheelsInScratchMode = false;
    this.PADMode = false; //false = not pressed; true = pressed
    this.shiftKey = false;
    this.touch = false;
    this.faderstart = false;
    this.PitchFaderHigh = 0;
    this.lastfadervalue = 0;
    this.scratchTimer = 0;
    this.seekingfast = false;
    this.iCutStatus = false;
    this.LEDs = [];
    this.TapDown = false;
    this.InstantFX = [];
    this.Jog = new Jogger(this.group, this.decknum);
    this.duration = 0;

    engine.setValue("[EffectRack1_EffectUnit" + decknum + "]", "show_focus", true);

    // buttons
    this.LoadButtonControl = new LongShortBtn(NumarkMixtrack3.OnLoadButton);
    this.SyncButtonControl = new LongShortDoubleBtn(NumarkMixtrack3.OnSyncButton);
    this.PADLoopButtonHold = new LongShortBtn(NumarkMixtrack3.onPADLoopButtonHold);

    engine.softTakeover(this.group, "rate", true); // Enable soft-takeover for Pitch slider
    engine.softTakeover(this.group, "volume", true); // Enable soft-takeover for volume
    engine.setParameter(this.group, "volume", 0); // Set volume to zero for each deck (initial load only)

    // only have two physical sets of buttons for our 4 virtual decks
    var j = (decknum + 1) % 2 + 1;
    this.LEDs.headphones = new LED(0x90 + ledCategories.master, leds.headphones1 - 1 + j);
    this.LEDs.jogWheelsInScratchMode = new LED(0x90 + j, leds.jogWheelsInScratchMode);
    this.LEDs.loopin = new LED(0x90 + j, leds.loopin);
    this.LEDs.loopout = new LED(0x90 + j, leds.loopout);
    this.LEDs.reloopExit = new LED(0x90 + j, leds.reloopExit);
    this.LEDs.loopHalve = new LED(0x90 + j, leds.loopHalve);
    this.LEDs.hotCue1 = new LED(0x90 + j, leds.hotCue1);
    this.LEDs.hotCue2 = new LED(0x90 + j, leds.hotCue2);
    this.LEDs.hotCue3 = new LED(0x90 + j, leds.hotCue3);
    this.LEDs.hotCue4 = new LED(0x90 + j, leds.hotCue4);
    this.LEDs.cue = new LED(0x90 + j, leds.cue);
    this.LEDs.sync = new LED(0x90 + j, leds.sync);
    this.LEDs.play = new LED(0x90 + j, leds.play);
    this.LEDs.fx1 = new LED(0x90 + j, leds.fx1);
    this.LEDs.fx2 = new LED(0x90 + j, leds.fx2);
    this.LEDs.fx3 = new LED(0x90 + j, leds.fx3);
    this.LEDs.tap = new LED(0x90 + j, leds.tap);
    this.LEDs.padLoop1 = new LED(0x90 + j, leds.padLoop1);
    this.LEDs.padLoop2 = new LED(0x90 + j, leds.padLoop2);
    this.LEDs.padLoop3 = new LED(0x90 + j, leds.padLoop3);
    this.LEDs.padLoop4 = new LED(0x90 + j, leds.padLoop4);
    this.LEDs.meter = new LED(0x90 + ledCategories.meters, leds.meter1 - 1 + j);

    for (var led in this.LEDs) {
        if (led.hasOwnProperty("onOff")) {
            led.onOff(OFF);
        }
    }

    for (var j = 1; j <= 4; j++) {
        this.LEDs["padLoop" + j].onOff(PADcolors.black);
    }
};


NumarkMixtrack3.deck.prototype.trackLoaded = function() {
    return engine.getValue(this.group, "track_loaded");
};

NumarkMixtrack3.deck.prototype.getFocusedEffect = function() {
    var effectGroup = "[EffectRack1_EffectUnit" + this.decknum + "]";
    return engine.getValue(effectGroup, "focused_effect");
};

NumarkMixtrack3.deck.prototype.focusEffect = function(effectNum) {
    var effectGroup = "[EffectRack1_EffectUnit" + this.decknum + "]";
    engine.setValue(effectGroup, "focused_effect", effectNum);
};

NumarkMixtrack3.deck.prototype.focusNextEffect = function() {
    var effectsPerUnit = 3;
    var nextEffect = (this.getFocusedEffect() + 1) % (effectsPerUnit + 1);
    this.focusEffect(nextEffect);
};

// ******************************************************************
// Samplers - object
// ******************************************************************

NumarkMixtrack3.sampler = function(decknum) {
    this.decknum = decknum;
    this.group = "[Sampler" + decknum + "]";
    this.loaded = false;
    this.PitchFaderHigh = 0;
    this.lastfadervalue = 0;
    this.LEDs = [];

    this.PADSampleButtonHold = new LongShortBtn(NumarkMixtrack3.onPADSampleButtonHold);

    engine.connectControl(this.group, "play", "NumarkMixtrack3.OnSamplePlayStop");

    // sampler LED, first 4 are 0x91, next 4 are 0x92
    this.led = new LED(0x91 + Math.round(decknum / 9), leds["padSampler" + decknum]);
    this.led.onOff(PADcolors.black);
};

// =====================================================================
// Initialization of the mapping
// =====================================================================
NumarkMixtrack3.init = function(id, debug) {
    // Set up the controller to manipulate decks 1 & 2 when this script is loaded
    print("********* Initialisation process engaged *****************");
    print("              Mapping initialization");
    print("");

    print("==========================================================");
    print("               Initialize variables");
    print("");

    NumarkMixtrack3.id = id; // Store the ID of this device for later use
    NumarkMixtrack3.debug = debug;

    engine.setValue("[Skin]", "show_superknobs", 1);

    NumarkMixtrack3.deckGroup = {
        "[Channel1]": "[Channel1]",
        "[Channel2]": "[Channel2]",
        "[Channel3]": "[Channel3]",
        "[Channel4]": "[Channel4]"
    };

    NumarkMixtrack3.fxControls = { // used to determine FX Button
        "[EffectRack1_EffectUnit1_Effect1]": 1,
        "[EffectRack1_EffectUnit1_Effect2]": 2,
        "[EffectRack1_EffectUnit1_Effect3]": 3,
        "[EffectRack1_EffectUnit2_Effect1]": 1,
        "[EffectRack1_EffectUnit2_Effect2]": 2,
        "[EffectRack1_EffectUnit2_Effect3]": 3,
        "[EffectRack1_EffectUnit3_Effect1]": 1,
        "[EffectRack1_EffectUnit3_Effect2]": 2,
        "[EffectRack1_EffectUnit3_Effect3]": 3,
        "[EffectRack1_EffectUnit4_Effect1]": 1,
        "[EffectRack1_EffectUnit4_Effect2]": 2,
        "[EffectRack1_EffectUnit4_Effect3]": 3
    };

    NumarkMixtrack3.fxGroups = { //Used to determine deck
        "[EffectRack1_EffectUnit1_Effect1]": 1,
        "[EffectRack1_EffectUnit1_Effect2]": 1,
        "[EffectRack1_EffectUnit1_Effect3]": 1,
        "[EffectRack1_EffectUnit2_Effect1]": 2,
        "[EffectRack1_EffectUnit2_Effect2]": 2,
        "[EffectRack1_EffectUnit2_Effect3]": 2,
        "[EffectRack1_EffectUnit3_Effect1]": 3,
        "[EffectRack1_EffectUnit3_Effect2]": 3,
        "[EffectRack1_EffectUnit3_Effect3]": 3,
        "[EffectRack1_EffectUnit4_Effect1]": 4,
        "[EffectRack1_EffectUnit4_Effect2]": 4,
        "[EffectRack1_EffectUnit4_Effect3]": 4
    };

    NumarkMixtrack3.Autoloop = {
        "beatloop_2_enabled": 1,
        "beatloop_4_enabled": 2,
        "beatloop_8_enabled": 3,
        "beatloop_16_enabled": 4,
        "beatloop_0.125_enabled": 1,
        "beatloop_0.25_enabled": 2,
        "beatloop_0.5_enabled": 3,
        "beatloop_1_enabled": 4
    };

    // initialize 4 decks
    for (var i = 1; i <= 4; i++) {
        NumarkMixtrack3.decks["D" + i] = new NumarkMixtrack3.deck(i);
    }

    if (engine.getValue("[App]", "num_samplers") < 8) {
        engine.setValue("[App]", "num_samplers", 8);
    }
    // initialize 8 samplers
    for (var i = 1; i <= 8; i++) {
        NumarkMixtrack3.samplers["S" + i] = new NumarkMixtrack3.sampler(i);
    }

    NumarkMixtrack3.AllLeds = new LED(0x90 + ledCategories.master, leds.all);
    // Turn ON all the lights: the only way PADMode Leds light up
    NumarkMixtrack3.AllLeds.onOff(ON);

    NumarkMixtrack3.initDeck("[Channel1]", false); //Initial load, "remove" is set to false
    NumarkMixtrack3.initDeck("[Channel2]", false);

    print("*********      Controller is ready      *******************");
    print("********* End of Initialisation process *******************");
};

NumarkMixtrack3.initDeck = function(group, remove) {
    var disconnectDeck = parseInt(NumarkMixtrack3.channelRegEx.exec(group)[1]);
    var connectedLED = disconnectDeck;

    if (disconnectDeck <= 2) {
        disconnectDeck += 2;
    } else {
        disconnectDeck -= 2;
    }

    print("==========================================================");
    print("                initDeck " + group);
    print("");

    // If "remove" = true, disconnect old deck's Mixxx controls from LEDs.
    // We always connect new deck's Mixxx controls to LEDs
    NumarkMixtrack3.connectDeckControls(group, remove);

    // Toggle LED that indicates which deck is being controlled
    if (connectedLED <= 2) {
        NumarkMixtrack3.decks["D" + disconnectDeck].LEDs.tap.onOff((OFF));
    } else {
        NumarkMixtrack3.decks["D" + connectedLED].LEDs.tap.onOff((ON));
    }
};

NumarkMixtrack3.connectDeckControls = function(group, remove) {
    // If the 'remove' parameter is not passed to this function, set remove = false
    remove = remove || false;
    var onDeckNum = parseInt(NumarkMixtrack3.channelRegEx.exec(group)[1]);
    var offDeckNum = (onDeckNum + 1) % 4 + 1;
    var onDeck = NumarkMixtrack3.decks["D" + onDeckNum];
    var offDeck = NumarkMixtrack3.decks["D" + offDeckNum];

    if (remove) {
        print("==========================================================");
        print("           Disconnect controls from deck " + offDeckNum);
        print("");

        // make sure that the shift is no longer active on either deck to prevent confusion
        offDeck.shiftKey = false;
        onDeck.shiftKey = false;

        for (var led in offDeck.LEDs) {
            if (led.hasOwnProperty("onOff")) {
                led.onOff(OFF);
            }
        }

        for (var i = 1; i <= 4; i++) {
            onDeck.LEDs["padLoop" + i].onOff(PADcolors.yellow);
        }
    }

    print("==========================================================");
    print("         Connect controls and triggers deck " + onDeckNum);
    print("");

    var controlsToFunctions = {
        "hotcue_1_enabled": "NumarkMixtrack3.OnHotcueChange",
        "hotcue_2_enabled": "NumarkMixtrack3.OnHotcueChange",
        "hotcue_3_enabled": "NumarkMixtrack3.OnHotcueChange",
        "hotcue_4_enabled": "NumarkMixtrack3.OnHotcueChange",
        "vu_meter": "NumarkMixtrack3.OnVuMeterChange",
        "playposition": "NumarkMixtrack3.OnPlaypositionChange",
        "volume": "NumarkMixtrack3.OnVolumeChange",
        "pfl": "NumarkMixtrack3.OnPFLStatusChange",
        "duration": "NumarkMixtrack3.OnLoadSelectedTrack",
        "play_indicator": "NumarkMixtrack3.OnPlayIndicatorChange",
        "cue_indicator": "NumarkMixtrack3.OnCuePointChange",
        "loop_start_position": "NumarkMixtrack3.OnLoopInOutChange",
        "loop_end_position": "NumarkMixtrack3.OnLoopInOutChange",
        "loop_enabled": "NumarkMixtrack3.OnLoopInOutChange",
        "sync_enabled": "NumarkMixtrack3.OnSyncButtonChange"
    };

    for (var i = 0, n = loopsize.length; i < n; i ++) {
        controlsToFunctions["beatloop_" + loopsize[i] + "_enabled"] = "NumarkMixtrack3.OnPADLoopButtonChange";
    }

    engine.connectControl("[EffectRack1_EffectUnit" + onDeckNum + "_Effect1]", "enabled",
        "NumarkMixtrack3.OnEffectEnabled");
    engine.connectControl("[EffectRack1_EffectUnit" + onDeckNum + "_Effect2]", "enabled",
        "NumarkMixtrack3.OnEffectEnabled");
    engine.connectControl("[EffectRack1_EffectUnit" + onDeckNum + "_Effect3]", "enabled",
        "NumarkMixtrack3.OnEffectEnabled");

    engine.trigger("[EffectRack1_EffectUnit" + onDeckNum + "_Effect1]", "enabled");
    engine.trigger("[EffectRack1_EffectUnit" + onDeckNum + "_Effect2]", "enabled");
    engine.trigger("[EffectRack1_EffectUnit" + onDeckNum + "_Effect3]", "enabled");


    // Set InstantFX LEDs to flash if required
    var arrayLength = onDeck.InstantFX.length;

    for (var i = 0; i < arrayLength; i++) {
        var ButtonNum = onDeck.InstantFX[i];
        onDeck.LEDs["fx" + ButtonNum].flashOn(250, ON, 250);
    }

    for (var control in controlsToFunctions) {
        if (controlsToFunctions.hasOwnProperty(control)) {
            if (remove) {
                engine.connectControl(offDeck.group, control, controlsToFunctions[control], true);
            }
            engine.connectControl(group, control, controlsToFunctions[control]);
            engine.trigger(group, control);
        }
    }

    if (!remove) {
        for (var i = 1; i <= 4; i++) {
            engine.setValue("[EffectRack1_EffectUnit" + i + "_Effect1]", "enabled", false);
            engine.setValue("[EffectRack1_EffectUnit" + i + "_Effect2]", "enabled", false);
            engine.setValue("[EffectRack1_EffectUnit" + i + "_Effect3]", "enabled", false);
        }
    }

    print("");
    print("               Initialisation completed");
    print("==========================================================");
    print("");
};

NumarkMixtrack3.shutdown = function() {
    print("********* Starting Controller Shutdown ********** ");
    print("               Turning off LEDs");
    NumarkMixtrack3.AllLeds.onOff(OFF);
    print("********* Controller shutdown completed********* ");
};

NumarkMixtrack3.channelRegEx = /(\d+)/;

NumarkMixtrack3.deckFromGroup = function(group) { // DFG // for easy find
    group = NumarkMixtrack3.deckGroup[group];
    var decknum = parseInt(group.substring(8, 9));
    return this.decks["D" + decknum];
};

NumarkMixtrack3.ShiftButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    deck.shiftKey = (value === DOWN);
};

/******************     Play Button :
 * - Press         : to Play / pause the track. If no track is loaded,
 *                   Load the selected track (if any) and play.
 * - SHIFT+ press : Go to Cue point and play (stutter).
 * *********************************************************************/
NumarkMixtrack3.PlayButton = function(channel, control, value, status, group) {
    if (!value) return;
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        if (!deck.shiftKey) {
            // play/pause
            if (!deck.trackLoaded()) {
                engine.setValue(deck.group, "LoadSelectedTrackAndPlay", true);
            } else {
                script.toggleControl(deck.group, "play");
            }
        } else {
            // shifted: stutter
            engine.setValue(deck.group, "play_stutter", true);
        }
    }
};

NumarkMixtrack3.BrowseButton = function(channel, control, value, status, group) {
    var shifted = (NumarkMixtrack3.decks.D1.shiftKey || NumarkMixtrack3.decks.
        D2.shiftKey || NumarkMixtrack3.decks.D3.shiftKey || NumarkMixtrack3.decks.D4.shiftKey);

    if (value === ON) {
	    if (shifted) {
	        // SHIFT + BROWSE push : directory mode -- > Open/Close selected side bar item
	        engine.setValue("[Library]", "GoToItem", true);
	    } else {
	        // Browse push : maximize/minimize library view
	        if (value === ON) {
	            script.toggleControl("[Skin]", "show_maximized_library");
	        }
	    }
    }
};

NumarkMixtrack3.BrowseKnob = function(channel, control, value, status, group) {
    var shifted = (
        NumarkMixtrack3.decks.D1.shiftKey || NumarkMixtrack3.decks.D2.shiftKey ||
        NumarkMixtrack3.decks.D3.shiftKey || NumarkMixtrack3.decks.D4.shiftKey
    );

    // value = 1 / 2 / 3 ... for positive //value = 1 / 2 / 3
    var nval = (value > 0x40 ? value - 0x80 : value);

    // SHIFT+Turn BROWSE Knob : directory mode --> select Play List/Side bar item
    if (shifted) {
        engine.setValue("[Playlist]", "SelectPlaylist", nval);
    } else {
        engine.setValue("[Playlist]", "SelectTrackKnob", nval);
    }
};

NumarkMixtrack3.PadModeButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    deck.PADMode = false;

    if (value === DOWN) {
        deck.PADMode = true;
        //ensure all LEDs are ON (default)
        for (var i = 1; i <= 8; i++) {
            NumarkMixtrack3.samplers["S" + i].led.onOff(PADcolors.purple);
        }

        deck.LEDs["padLoop1"].onOff(PADcolors.yellow);
        deck.LEDs["padLoop2"].onOff(PADcolors.yellow);
        deck.LEDs["padLoop3"].onOff(PADcolors.yellow);
        deck.LEDs["padLoop4"].onOff(PADcolors.yellow);
    }

    // Now check which one should be blinking
    // Need to check if loop is enabled; if yes, stop it , else start it
    //Autoloop
    if (value === DOWN) {
        for (var i = 0; i < loopsize.length; i++) {
            var index = i + 1;
            if (index > 4) {
                index = index - 4;
            }

            if (engine.getValue(deck.group, "beatloop_" + loopsize[i] + "_enabled")) {
                deck.LEDs["padLoop" + index].flashOn(300, PADcolors.yellow, 300);
            }
        }

        //Sampler
        for (var i = 1; i <= 8; i++) {
            engine.trigger("[Sampler" + i + "]", "play");
        }
    }
};

/******************     Load button :
 * - Load a track : Press these buttons to load the selected track from
 *  (short press)   the Browser to left or right deck. The LED of the
 *                  button will be on if the deck is loaded.
 * - Eject        : Hold the same button for more than half of a second
 *  (long press)    to unload the same deck.
 ***********************************************************************/
NumarkMixtrack3.LoadButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        deck.faderstart = false;

        if (smartPFL) {
            for (var i = 1; i <= 4; i++) {
                //Change headphone cue (pfl) to the deck on which the song loaded.
                engine.setValue("[Channel" + i + "]", "pfl", deck.decknum === i);
            }
        }

        if (deck.shiftKey) {
            // SHIFT + Load = fader start activated
            deck.faderstart = true;

            if (!deck.trackLoaded()) {
                engine.setValue(deck.group, "LoadSelectedTrack", true);
            }
        }

        deck.LoadButtonControl.buttonDown(channel, control, value, status, deck.group);
    } else {
        deck.LoadButtonControl.buttonUp();
    }
};

// Callback for the Load Button
NumarkMixtrack3.OnLoadButton = function(channel, control, value, status, group, eventkind) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (eventkind === LONG_PRESS) {
        engine.setValue(deck.group, "eject", true);
        engine.setValue(deck.group, "eject", false);
    } else {
        engine.setValue(deck.group, "LoadSelectedTrack", true);
    }
};

NumarkMixtrack3.OnLoadSelectedTrack = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var trackDuration = engine.getValue(deck.group, "duration");

    if (smartPFL && deck.duration !== trackDuration && trackDuration !== 0) {
        for (var i = 1; i <= 4; i++) {
            // change headphone cue (pfl) to the deck on which the song loaded.
            engine.setValue("[Channel" + i + "]", "pfl", deck.decknum === i);
        }

        // update the jog light
        engine.trigger(group, "playposition");
    }

    deck.duration = trackDuration;

};

/******************     Sync button :
 * - Short Press  : Press once to synchronize the tempo (BPM) and phase
 *                  to that of to that of the other track.
 * - Double Press : press twice QUICKLY to play the track immediately,
 *                  synchronized to the tempo (BPM) and to the phase of
 *                 the other track, if the track was paused.
 * - Long Press (Sync Lock) :
 *                 Hold for at least half of a second to enable sync lock
 *                 for this deck. Decks with sync locked will all play at
 *                 the same tempo, and decks that also have quantize
 *                 enabled will always have their beats lined up.
 * If Sync Lock was previously activated, it just deactivates it,
 * regardless of the Short press/Double Press
 *
 * - SHIFT + Press : Toggle Key Lock
 ***********************************************************************/
NumarkMixtrack3.SyncButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (!deck.shiftKey) {
        if (value === DOWN) {
            deck.SyncButtonControl.buttonDown(channel, control, value, status, deck.group);
        } else {
            deck.SyncButtonControl.buttonUp();
        }
    } else {
        if (value === DOWN) {
            script.toggleControl(deck.group, "keylock");
        }
    }
};

// Callback for the SYNC Button
NumarkMixtrack3.OnSyncButton = function(channel, control, value, status, group, eventkind) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (eventkind === LONG_PRESS) {
        deck.LEDs.sync.onOff(ON);
        engine.setValue(group, "sync_enabled", true);
    } else {
        if (engine.getValue(group, "sync_enabled")) {
            // if sync lock is enabled, simply disable sync lock
            engine.setValue(group, "sync_enabled", false);
            deck.LEDs.sync.onOff(OFF);
        } else {
            if (eventkind === DOUBLE_PRESS && !noPlayOnSyncDoublePress) {
                // double press : Sync and play (if the track was paused
                // the playback starts, synchronized to the other track
                engine.setValue(group, "play", true);
                engine.setValue(group, "beatsync", true);
                deck.LEDs.sync.flashOn(100, ON, 100, 3);
            } else {
                // we pressed sync only once, we sync the track
                // with the other track (eventkind === QUICK_PRESS
                engine.setValue(group, "beatsync", true);
                deck.LEDs.sync.flashOn(100, ON, 100, 3);
            }
        }
    }
};

NumarkMixtrack3.OnSyncButtonChange = function(value, group, key) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (engine.getValue(group, "sync_enabled")) {
        deck.LEDs.sync.onOff(ON);
    } else {
        deck.LEDs.sync.onOff(OFF);
    }
};

/******************     Cue button :
 * - press         : Well, it is the Cue Button :)
 * - SHIFT + press : Go to start of the track
 ***********************************************************************/
NumarkMixtrack3.CueButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (!deck.shiftKey) {
        // Don't set Cue accidentally at the end of the song
        if (engine.getValue(deck.group, "playposition") <= 0.97) {
            engine.setValue(deck.group, "cue_default", value ? 1 : 0);
        } else {
            engine.setValue(deck.group, "cue_preview", value ? 1 : 0);
        }
    } else {
        engine.setValue(deck.group, "start", true);
    }
};

NumarkMixtrack3.OnCuePointChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    deck.LEDs.cue.onOff((value) ? ON : OFF);
};

// Pitch faders send 2*7bits
NumarkMixtrack3.PitchFaderHighValue = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    deck.PitchFaderHigh = value;
};

NumarkMixtrack3.PitchFaderLowValue = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var calcvalue = (8192 - ((deck.PitchFaderHigh * 128) + value)) / 8192;
    engine.setValue(deck.group, "rate", calcvalue);
};

NumarkMixtrack3.toggleJogMode = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        if (deck.shiftKey) {
            script.toggleControl(group, "slip_enabled");
        } else {
            deck.jogWheelsInScratchMode = !deck.jogWheelsInScratchMode;
            deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ? ON : OFF);
        }
    }
};

NumarkMixtrack3.WheelTouch = function(channel, control, value, status, group) {
    /*
    This function sets the variable to assign the wheel move action
    - Pitch bend / jog = default
    - fast seek - deck.seekingfast = true
    - iCut = deck.iCutStatus = true
    - Scratching = deck.touch = true
    */
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var isPlaying = engine.getValue(deck.group, "play");

    deck.touch = false;
    deck.iCutStatus = false;
    deck.seekingfast = false;

    if (value === DOWN) {
        if (deck.jogWheelsInScratchMode || !isPlaying) {
            engine.scratchEnable(deck.decknum, intervalsPerRev, rpm, alpha, beta);

            // Wheel is On - test for Shift Key");
            if (deck.shiftKey && iCutEnabled) {
                deck.iCutStatus = true;
                deck.Jog.iCUT.On();
            } else {
                deck.iCutStatus = false;
                deck.touch = true;
                deck.Jog.iCUT.Off();
            }
        } else {
            if (deck.shiftKey) {
                deck.seekingfast = true;
            }
        }
    } else {
        engine.scratchDisable(deck.decknum, true);
        deck.seekingfast = false;
        deck.Jog.iCUT.Off();
    }
};

NumarkMixtrack3.WheelMove = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var adjustedJog = parseFloat(value); // set jog value
    var direction = 1; // 1 = clockwise, -1 = counter-clockwise

    // NMTP3 is a "Model A" controller for scratching, it centers on 0.
    // See http://www.mixxx.org/wiki/doku.php/midi_scripting#scratching
    if (adjustedJog > 63) { // Counter-clockwise
        direction = -1;
        adjustedJog = value - 128;
    }

    /*  This function performs that actions defined by wheel touch
        - Pitch bend / jog = default
        - fast seek - deck.seekingfast = true
        - iCut = deck.iCutStatus = true
        - Scratching = deck.touch = true */
    if (deck.iCutStatus) {
        deck.Jog.iCUT.On();
        deck.Jog.iCUT.FaderCut(adjustedJog, deck.decknum);
    }

    if (deck.seekingfast) {
        engine.setValue(deck.Jog.group, "beatjump", adjustedJog * 2);
    }

    engine.scratchTick(deck.decknum, adjustedJog);

    var isPlaying = engine.getValue(deck.Jog.group, "play");

    // pitch bend when playing - side or platter have same effect
    if (isPlaying && (PitchBendOnWheelOff || deck.jogWheelsInScratchMode)) {
        var gammaInputRange = 13; // Max jog speed
        var maxOutFraction = 0.8; // Where on the curve it should peak; 0.5 is half-way
        var sensitivity = 0.5; // Adjustment gamma
        var gammaOutputRange = 0.75; // Max rate change

        adjustedJog = direction * gammaOutputRange * Math.pow(
            Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction),
            sensitivity
        );

        engine.setValue(deck.Jog.group, "jog", adjustedJog);
    }
};

NumarkMixtrack3.HotCueButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var hotCue = parseInt(control - leds.hotCue1 + 1);

    if (deck.shiftKey) {
        if (value === DOWN) {
            engine.setValue(deck.group, "hotcue_" + hotCue + "_clear", true);
            deck.LEDs["hotCue" + hotCue].onOff(OFF);
        }
    } else if (deck.TapDown) {
        // special effects with tap + hot cues
        if (hotCue === 1) {
            engine.brake(deck.decknum, value === DOWN);
        } else if (hotCue === 2) {
            engine.spinback(deck.decknum, value === DOWN);
        }
    } else {
        engine.setValue(deck.group, "hotcue_" + hotCue + "_activate", value === DOWN);
    }
};

NumarkMixtrack3.OnHotcueChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var padIndex = parseInt(NumarkMixtrack3.channelRegEx.exec(control)[1]);

    deck.LEDs["hotCue" + padIndex].onOff((value) ? ON : OFF);
};

NumarkMixtrack3.SamplerButton = function(channel, control, value, status, group) {
    var isPlaying = engine.getValue(group, "play");
    var isLoaded = engine.getValue(group, "track_loaded");
    var padIndex = parseInt(group.substring(8, 9));
    var sampler = NumarkMixtrack3.samplers["S" + padIndex];
    var decknum = 1;

    if (padIndex > 4) {
        decknum = 2;
        engine.setValue("[Deere]", "sampler_bank_2", true);
    } else {
        engine.setValue("[Deere]", "sampler_bank_1", true);
    }

    var deck = NumarkMixtrack3.deckFromGroup("[Channel" + decknum + "]");

    if (value === DOWN) {
        sampler.PADSampleButtonHold.buttonDown(channel, control, value, status, group);

        if (!isPlaying) {
            if (deck.shiftKey) {
                // shift is on, play sampler with no Sync
                engine.setValue(group, "beatsync", 0);
                engine.setValue(group, "cue_gotoandplay", 1);
            } else if (!isLoaded) {
                engine.setValue(group, "LoadSelectedTrack", 1);
            } else if (deck.TapDown) {
                engine.setValue(group, "eject", 1);
                engine.setValue(group, "eject", 0);
            } else {
                engine.setValue(group, "cue_gotoandplay", 1);
                engine.setValue(group, "beatsync", 1);
            }

            sampler.led.flashOn(300, PADcolors.purple, 300);
        } else {
            if (deck.shiftKey) {
                engine.setValue(group, "stop", 1);
                sampler.led.onOff(ON);
            } else {
                engine.setValue(group, "cue_gotoandplay", 1);
            }
        }
    }

    if (value === OFF) {
        sampler.PADSampleButtonHold.buttonUp();
    }
};

NumarkMixtrack3.onPADSampleButtonHold = function(channel, control, value, status, group, eventkind) {
    var padIndex = parseInt(group.substring(8, 9));
    var sampler = NumarkMixtrack3.samplers["S" + padIndex];

    // the event is a Long Press, LONG_PRESS is true, we set a variable so that when the
    // pad button is lifted, the Sampler stops
    if (eventkind === LONG_PRESS) {
        engine.setValue(group, "stop", 1);
        sampler.led.onOff(ON);
    }
};

NumarkMixtrack3.OnSamplePlayStop = function(value, group, control) {
    var padIndex = parseInt(group.substring(8, 9));
    var sampler = NumarkMixtrack3.samplers["S" + padIndex];

    if (value === 1) {
        sampler.led.flashOn(300, PADcolors.purple, 300);
    } else {
        sampler.led.onOff(ON);
    }
};

NumarkMixtrack3.PADLoopButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    // if pad mode is being held, don't act as if we pressed the pad
    if (deck.PADMode) {
        return;
    }

    var padIndex = control - leds.padLoop1 + 1;
    var loopsizeNew;

    if (deck.shiftKey) {
        loopsizeNew = loopsize[padIndex + 3];
    } else {
        loopsizeNew = loopsize[padIndex - 1];
    }

    if (value === DOWN && deck.trackLoaded()) {
        // make sure all LED are ON
        deck.LEDs.padLoop1.onOff(PADcolors.yellow);
        deck.LEDs.padLoop2.onOff(PADcolors.yellow);
        deck.LEDs.padLoop3.onOff(PADcolors.yellow);
        deck.LEDs.padLoop4.onOff(PADcolors.yellow);

        if (engine.getValue(group, "beatloop_" + loopsizeNew + "_enabled")) {
            // Loop is active, turn it off
            engine.setValue(deck.group, "reloop_toggle", true);
            deck.LEDs["padLoop" + padIndex].onOff(PADcolors.yellow);
        } else {
            // Loop is not active, turn it on
            deck.LEDs["padLoop" + padIndex].flashOn(250, PADcolors.yellow, 250);
            engine.setValue(deck.group, "beatloop_" + loopsizeNew + "_toggle", true);
        }

        // Event if long press
        deck.PADLoopButtonHold.buttonDown(channel, control, value, status, deck.group);
    }

    if (value === OFF && deck.trackLoaded()) { //This triggers the callback function for "PADLoopButtonHold"
        deck.PADLoopButtonHold.buttonUp();
    }
};

NumarkMixtrack3.onPADLoopButtonHold = function(channel, control, value, status, group, eventkind) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var padIndex = control - leds.padLoop1 + 1;
    var loopsizeNew;

    if (deck.shiftKey) {
        loopsizeNew = loopsize[padIndex + 3];
    } else {
        loopsizeNew = loopsize[padIndex - 1];
    }

    if (eventkind === LONG_PRESS) {
        engine.setValue(deck.group, "reloop_toggle", true);
    }
};

NumarkMixtrack3.OnPADLoopButtonChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var padIndex = NumarkMixtrack3.Autoloop[control];

    if (value === 1) {
        deck.LEDs["padLoop" + padIndex].flashOn(300, PADcolors.yellow, 300);
    } else {
        deck.LEDs["padLoop" + padIndex].onOff(PADcolors.yellow);
    }

    // on initialization of deck, the value "0" would cause the pad LED to stop blinking on the 2nd pass
    // of triggers. This gives ensures that the PAD mode reflect the proper state of AutoLoop
    for (var i = 0; i < loopsize.length; i++) {
        var index = i + 1;

        if (index > 4) {
            index = index - 4;
        }

        var test = engine.getValue(deck.group, "beatloop_" + loopsize[i] + "_enabled");

        if (test) {
            deck.LEDs["padLoop" + index].flashOn(300, PADcolors.yellow, 300);
        }
    }
};

NumarkMixtrack3.StripTouchEffect = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var focusedEffect = deck.getFocusedEffect();

    if (deck.shiftKey) {
        engine.setValue(deck.group, "playposition", value / 127);
    } else {
        if (deck.InstantFX.length) {
            for (var i = 0; i < deck.InstantFX.length; i++) {
                var fxGroup = "[EffectRack1_EffectUnit" + deck.decknum + "_Effect" + deck.InstantFX[i] + "]";
                engine.setValue(fxGroup, "enabled", true);
                engine.setValue(fxGroup, "meta", value / 127);
            }
        } else if (focusedEffect) {
            var focusedGroup = "[EffectRack1_EffectUnit" + deck.decknum + "_Effect" + focusedEffect + "]";
            engine.setValue(focusedGroup, "meta", value / 127);
        } else {
            engine.setValue("[EffectRack1_EffectUnit" + deck.decknum + "]", "super1", value / 127);
        }
    }
};

NumarkMixtrack3.InstantFXOff = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    for (var i = 0, n = deck.InstantFX.length; i < n; i++) {
        var buttonNum = deck.InstantFX[i];
        engine.setValue("[EffectRack1_EffectUnit" + deck.decknum + "_Effect" + buttonNum + "]", "enabled", false);
    }
};

NumarkMixtrack3.FXButton = function(channel, control, value, status, group) {
    if (value !== DOWN) {
        return;
    }

    var deck = NumarkMixtrack3.deckFromGroup(group);
    var decknum = deck.decknum;
    var effectNum = control - leds.fx1 + 1;
    var effectGroup = "[EffectRack1_EffectUnit" + decknum + "_Effect" + effectNum + "]";

    if (deck.PADMode) {
        if (deck.InstantFX.indexOf(effectNum) > -1) {
            // remove effect from InstantFX and set LED to reflect whether or not effect is enabled
            var index = deck.InstantFX.indexOf(effectNum);
            deck.InstantFX.splice(index, 1);
            deck.LEDs["fx" + effectNum].onOff(engine.getValue(effectGroup, "enabled") ? ON : OFF);
        } else {
            // add new effect to InstantFX
            deck.InstantFX.push(effectNum);

            // get all LEDs to flash in sync
            for (var i = 0, n = deck.InstantFX.length; i < n; i++) {
                effectNum = deck.InstantFX[i];
                deck.LEDs["fx" + effectNum].flashOn(250, ON, 250);
            }
        }
    } else if (deck.shiftKey) {
        // load next effect and make sure the unit is enabled
        engine.setValue(effectGroup, "next_effect", true);
    } else if (deck.TapDown) {
        if (deck.getFocusedEffect() === effectNum) {
            deck.focusEffect(0);
        } else {
            deck.focusEffect(effectNum);
        }
    } else {
        // toggle effect if InstantFX is not active
        if (deck.InstantFX.indexOf(effectNum) === -1) {
            script.toggleControl(effectGroup, "enabled");
        }
    }
};

NumarkMixtrack3.OnEffectEnabled = function(value, group, control) {
    var index = NumarkMixtrack3.fxControls[group];
    var decknum = NumarkMixtrack3.fxGroups[group];
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (deck.InstantFX.indexOf(index) === -1) {
        var new_value = engine.getValue(group, control);
        deck.LEDs["fx" + index].onOff(new_value ? ON : OFF);
    }
};

/******************     Shift Button :
 * - Press                : toggle PFL
 * - SHIFT + press        : toggle quantize mode
 * *********************************************************************/
NumarkMixtrack3.PFLButton = function(channel, control, value, status, group) {
    if (!value) return;
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        if (deck.shiftKey) {
            script.toggleControl(group, "quantize");
        } else {
            script.toggleControl(deck.group, "pfl");
            for (var i = 1; i <= 4; i++) {
                if (i !== deck.decknum) {
                    engine.setValue("[Channel" + i + "]", "pfl", false);
                }
            }
        }
    }
};

NumarkMixtrack3.OnPFLStatusChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    deck.LEDs.headphones.onOff((value) ? ON : OFF);
};

NumarkMixtrack3.PitchBendMinusButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        if (deck.shiftKey) {
            engine.setValue(deck.group, "beatjump_backward", true);
        } else {
            engine.setValue(deck.group, "rate_temp_down", true);
        }
    } else if (!deck.shiftKey) {
        engine.setValue(deck.group, "rate_temp_down", false);
    }
};

NumarkMixtrack3.PitchBendPlusButton = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        if (deck.shiftKey) {
            engine.setValue(deck.group, "beatjump_forward", true);
        } else {
            engine.setValue(deck.group, "rate_temp_up", true);
        }
    } else if (!deck.shiftKey) {
        engine.setValue(deck.group, "rate_temp_up", false);
    }
};

NumarkMixtrack3.BeatKnob = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    // beat knobs sends 1 or 127 as value. If value = 127, turn is counterclockwise
    var increase = (value === 1);
    var increment = 1 / 20;

    if (!increase) {
        increment = -increment;
    }


    // direct interaction with knob, without any button combination
    if (!deck.PADMode && !deck.shiftKey && !deck.TapDown) {
        var mixValue = engine.getParameter("[EffectRack1_EffectUnit" + deck.decknum + "]", "mix");
        engine.setParameter("[EffectRack1_EffectUnit" + deck.decknum + "]", "mix", mixValue + increment);
    }

    // shift to change beatjump_size
    if (deck.shiftKey) {
        var jumpSize = engine.getValue(deck.group, "beatjump_size");
        var maxJumpSize = 512;
        var minJumpSize = 1/32;

        if (increase) {
            jumpSize = jumpSize * 2;
        } else {
            jumpSize = jumpSize / 2;
        }

        if (jumpSize > maxJumpSize) {
            jumpSize = maxJumpSize;
        } else if (jumpSize < minJumpSize) {
            jumpSize = minJumpSize;
        }

        engine.setValue(deck.group, "beatjump_size", jumpSize);
    }

    // tap to move beat grid
    if (deck.TapDown) {
        if (increase) {
            engine.setValue(deck.group, "beats_translate_later", true);
        } else {
            engine.setValue(deck.group, "beats_translate_earlier", true);
        }
    }

    // pad to change sampler gains
    if (deck.PADMode) {
        var startingSampler = (deck.decknum < 3) ? 1 : 5;

        for (var i = 0; i <= startingSampler + 3; i++) {
            var samplerGroup = "[Sampler" + i + "]";
            var gainValue = engine.getParameter(samplerGroup, "pregain");
            engine.setParameter(samplerGroup, "pregain", gainValue + increment);
        }
    }
};

NumarkMixtrack3.bpmTap = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);

    deck.TapDown = false;

    if (value === DOWN) {
        deck.TapDown = true; //to use TAP button as a "shift" key (e.g. InstantFX)

        if (deck.shiftKey) {
            var newDeckNum = (deck.decknum + 1) % 4 + 1; // toggle decks 1 -> 3, 2 -> 4 and vice versa
            NumarkMixtrack3.deckGroup[group] = "[Channel" + newDeckNum + "]";
            NumarkMixtrack3.initDeck(NumarkMixtrack3.deckGroup[group], true);
        } else {
            engine.setValue(deck.group, "bpm_tap", true);
            engine.setValue(deck.group, "bpm_tap", false);
        }
    }
};

NumarkMixtrack3.EQKnob = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var decknum = deck.decknum;
    var focusedEffect = deck.getFocusedEffect();
    var EQp = 4 - control; // convert control number to parameter number in mixxx
    var FXp = control; // control number matches effect param order

    // default behavior is to control EQ
    // when shifted, change parameters of focused effect
    if (deck.shiftKey && focusedEffect) {
        parameterSoftTakeOver(
            "[EffectRack1_EffectUnit" + decknum + "_Effect" + focusedEffect +"]", "parameter" + FXp, value
        );
    } else {
        parameterSoftTakeOver("[EqualizerRack1_[Channel" + decknum + "]_Effect1]", "parameter" + EQp, value);
    }
};

NumarkMixtrack3.FilterKnob = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup("[Channel" + group.substring(26, 27) + "]");
    var decknum = deck.decknum;
    var focusedEffect = deck.getFocusedEffect();

    // default behavior is to control filter
    // when shifted, change parameters of focused effect
    if (deck.shiftKey) {
        // Default behavior for Shift+Filter is to change FX4
        // for the currently focused effect
        if (focusedEffect && ShiftFilterFX4) {
            parameterSoftTakeOver(
                "[EffectRack1_EffectUnit" + decknum + "_Effect" + focusedEffect + "]", "parameter4", value
            );
        } else {
            // Shift+Filter is mapped to channel gain otherwise
            parameterSoftTakeOver("[Channel" + decknum + "]", "pregain", value);
        }
    } else {
        parameterSoftTakeOver("[QuickEffectRack1_[Channel" + decknum + "]]", "super1", value);
    }
};

NumarkMixtrack3.loop_in = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    engine.setValue(deck.group, "loop_in", value === DOWN);
};

NumarkMixtrack3.loop_out = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    engine.setValue(deck.group, "loop_out", value === DOWN);
};

NumarkMixtrack3.reloop_exit = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    engine.setValue(deck.group, "reloop_toggle", value === DOWN);
};

NumarkMixtrack3.LoopHalveButton = function(channel, control, value, status, group) {
    if (value === DOWN) {
        var deck = NumarkMixtrack3.deckFromGroup(group);

        if (deck.shiftKey) {
            engine.setValue(deck.group, "loop_double", true);
        } else {
            engine.setValue(deck.group, "loop_halve", true);
        }
    }
};

NumarkMixtrack3.OnLoopInOutChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var valIn = engine.getValue(deck.group, "loop_start_position");
    var valOut = engine.getValue(deck.group, "loop_end_position");
    var valEnabled = engine.getValue(deck.group, "loop_enabled");

    if (valIn == -1) {
        if (deck.LEDs.loopin.flashDuration !== 300) {
            deck.LEDs.loopin.flashOn(300, PADcolors.blue, 300);
        }

        deck.LEDs.loopout.onOff(OFF);
        deck.LEDs.reloopExit.onOff(OFF);
        deck.LEDs.loopHalve.onOff(OFF);
    } else if (valOut == -1 && deck.trackLoaded()) {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        if (deck.LEDs.loopout.flashDuration !== 300) {
            deck.LEDs.loopout.flashOn(300, PADcolors.blue, 300);
        }
        deck.LEDs.reloopExit.onOff(OFF);
        deck.LEDs.loopHalve.onOff(OFF);
    } else if (!valEnabled) {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        deck.LEDs.loopout.onOff(PADcolors.blue);
        if (deck.LEDs.reloopExit.flashDuration !== 300) {
            deck.LEDs.reloopExit.flashOn(300, PADcolors.blue, 300);
        }
        deck.LEDs.loopHalve.onOff(PADcolors.blue);
    } else {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        deck.LEDs.loopout.onOff(PADcolors.blue);
        deck.LEDs.reloopExit.onOff(PADcolors.blue);
        deck.LEDs.loopHalve.onOff(PADcolors.blue);
    }
};

NumarkMixtrack3.volume = function(channel, control, value, status, group) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    engine.setValue(deck.group, "volume", value / 127);
};

NumarkMixtrack3.OnVolumeChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var delta = value - deck.lastfadervalue;

    if (deck.faderstart) {
        if (value <= 0.01 && deck.isplaying) {
            engine.setValue(group, "play", 0);
            deck.isplaying = false;
        } else {
            if (delta > 0 && !deck.isplaying) {
                engine.setValue(group, "play", 1);
                deck.isplaying = true;
            }
        }
    }

    deck.lastfadervalue = value;
};

NumarkMixtrack3.OnVuMeterChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    deck.LEDs.meter.onOff(82 * value);
};

NumarkMixtrack3.OnPlaypositionChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    var duration = engine.getValue(group, "duration");

    if (deck.trackLoaded() && TrackEndWarning) {
        var timeRemaining = duration * (1 - value);

        if (timeRemaining <= 30) {
            // flashing slowly
            if (deck.LEDs.jogWheelsInScratchMode.flashDuration !== 1000) {
                deck.LEDs.jogWheelsInScratchMode.flashOn(1000, ON, 1000);
            }
        } else if (timeRemaining <= 10) {
            // flashing fast
            if (deck.LEDs.jogWheelsInScratchMode.flashDuration !== 300) {
                deck.LEDs.jogWheelsInScratchMode.flashOn(300, ON, 300);
            }
        } else {
            deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ? ON : OFF);
        }
    } else {
        deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ? ON : OFF);
    }
};

NumarkMixtrack3.OnPlayIndicatorChange = function(value, group, control) {
    var deck = NumarkMixtrack3.deckFromGroup(group);
    deck.LEDs.play.onOff((value) ? ON : OFF);
};

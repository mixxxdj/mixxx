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

// fastSeekEnabled: enable fast seek with Jog Wheel with Wheel Off and Shift ON 
// Shift can be locked or not
var fastSeekEnabled = true;

//activate PFL of deck on track load
var smartPFL = true;

// use beatlooproll instead of beatloop
var beatlooprollActivate = false;

// If Dark Metal Skin is used, set this to true: 
// this is required to expand library view with DM skin.
var DarkMetalSkin = false;

// Effects mode
// 1 for multi-effect mode (3 effects controlled in each EffectUnit, best used with Deere Skin)
// 2 for single effect mode (1 effect controlled in each EffectUnit, best used with skins other than Deere)
var FXmode = 1; // multi-effect mode by default

//Disable Play on Sync button Double Press
var noPlayOnSyncDoublePress = false;

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
var intervalsPerRev = 1200,
    rpm = 33 + 1 / 3, //Like a real vinyl !!! :)
    alpha = 1.0 / 8, //Adjust to suit.
    beta = alpha / 32; //Adjust to suit.

/**************************
 * Loop Size array 
 * first 4 values used for Autoloop Not shifted
 * last 4 values used for Autoloop Shifted
 **************************/
var loopsize = [2, 4, 8, 16, 0.125, 0.25, 0.5, 1];

/************************  GPL v2 licence  *****************************
 * Numark Mixtrack Pro 3 controller script
 * Author: Stéphane Morin
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
 * 2016-04-08 (1.4) to 2016-09-26 (1.8) - Stéphane Morin - https://github.com/mixxxdj/mixxx/pull/1014
 * 2016-09-26 (2.0) - Stéphane Morin - 4 deck support
 *              - Added 4 deck support (Shift + Tap button = deck toggle)
 *              - Remove TAP LED flashing on BeatActive to prevent confusion with deck selection
 *              - Change Shift Lock visual indicator from TAP LED to 
 *                Wheel Button LED. It will flash quickly on Shift lock
 *                with the use of LONG_PRESS (stop on release) and Single press (on/off behavior)
 *              - Add brake effect: PADMode + FX button 1
 *              - Add spinback effect: PADMode + FX button 2
 * 2016-11-03 (2.1) - Stéphane Morin - FX Mode selection based on skin used
 * 2016-12-05 (2.2) - Stéphane Morin 
 *              - Always allow use of wheel to position song when song is not playing 
 *              - Add configuration option for to choose if pitch bend is allowed when wheel is off
 *
 ***********************************************************************
 *                           GPL v2 licence
 *                           -------------- 
 * Numark Mixtrack Pro 3 controller script 2.1 for Mixxx 2.0+
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
/* global bpm                                                        */
////////////////////////////////////////////////////////////////////////

// Line below to remove warnings from JSHINT page regarding dot notation
/*jshint sub:true*/

function NumarkMixtrack3() {}

NumarkMixtrack3(); // Very important ! Initializes some reusable objects.

// Array of Objects can be created
NumarkMixtrack3.group = "[Master]";
NumarkMixtrack3.decknum = 0;
NumarkMixtrack3.decks = [];
NumarkMixtrack3.samplers = [];

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
    "reloop_exit": 0x15,
    "loop_halve": 0x16,
    "hotCue1": 0x1b,
    "hotCue2": 0x1c,
    "hotCue3": 0x1d,
    "hotCue4": 0x1e,
    "Cue": 0x03,
    "sync": 0x02,
    "play": 0x01,
    "fx1": 0x07,
    "fx2": 0x08,
    "fx3": 0x09,
    "tap": 0x0a,
    "PADloop1": 0x17,
    "PADloop2": 0x18,
    "PADloop3": 0x19,
    "PADloop4": 0x1A,
    "PADsampler1": 0x20,
    "PADsampler2": 0x21,
    "PADsampler3": 0x22,
    "PADsampler4": 0x23,
    "PADsampler5": 0x20,
    "PADsampler6": 0x21,
    "PADsampler7": 0x22,
    "PADsampler8": 0x23,

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

function toggleValue(group, key) {
    engine.setValue(group, key, !engine.getValue(group, key));
}

function TrackIsLoaded(group) {
    return (engine.getValue(group, "track_samples") > 0) ? true : false;
}

function RealDuration(group) {
    var ts = engine.getValue(group, "track_samples");
    if (ts <= 0) {
        return 0;
    } else {
        // this is an integer :
        var d1 = engine.getValue(group, "duration");
        //this is a real value :
        var d2 = engine.getValue(group, "track_samples") / engine.getValue(
            group, "track_samplerate");

        if (d1 === d2) {
            //it is mono
            return d2;
        } else {
            if ((d1 > Math.floor(d2)) && (d1 < Math.ceil(d2))) {
                //It is mono
                return d2;
            } else {
                //It is stereo
                return d2 / 2;
            }
        }
    }
}

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

LED.prototype.flashOn = function(num_ms_on, value, num_ms_off, flashCount,
    relight, valueoff) {
    var myself = this;

    // stop pending timers
    this.flashOff();

    // init     
    this.flashDuration = num_ms_on;
    this.num_ms_on = num_ms_on;
    this.valueon = value;
    this.num_ms_off = num_ms_off;
    this.flashCount = flashCount;
    this.relight = relight;
    this.valueoff = valueoff;

    // 1st flash
    // This is because the permanent timer below takes
    // num_ms_on milisecs before first flash.
    this.flashOnceOn(num_ms_on, value);

    if (flashCount !== 1) {
        // flashcount =0 means permanent flash,
        // flashcount>0 , means temporary flash, first flash already done,
        // so we don't need this part  if flashcount=1
        // permanent timer

        this.flashTimer = engine.beginTimer(num_ms_on + num_ms_off,
            function() {
                myself.flashOnceOn(false);
            });
    }
    if (flashCount > 1) {
        // flashcount>0 , means temporary flash, first flash already done,
        // so we don't need this part  if flashcount=1
        // temporary timer. The end of this timer stops the permanent flashing

        this.flashTimer2 = engine.beginTimer(flashCount * (num_ms_on +
            num_ms_off) - num_ms_off, function() {
            myself.Stopflash(relight);
        }, true);
    }
};

// public
LED.prototype.getFlashDuration = function() {
    return this.flashDuration;
};

LED.prototype.checkOn = function() {
    return this.lit;
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
LED.prototype.Stopflash = function(relight) {
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
    var myself = this;
    sendShortMsg(this.control, this.midino, this.valueon);
    pauseScript(scriptpause);
    this.flashOnceDuration = this.num_ms_on;
    this.flashOnceTimer = engine.beginTimer(this.num_ms_on - scriptpause,
        function() {
            myself.flashOnceOff(relight);
        }, true);
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
// Callback           : Callback function you have to provide (see end of
//                      the code), that will return the original event
//                      parameters (channel, control, value, status, group)
//                      and the kind of press event affecting your button
//                      (eventkind).
//                      This callback will be triggered as soon as you
//                      press the button a second time (Value will be
//                      equal to DOWN), or the Long press is asserted
//                      (value = DOWN because you are still holding down
//                      the button or value=UP because you have realeased
//                      the button only once before it becomes a long press).
// DoublePressTimeOut : delay in ms above wich a second press on the
//                      button will not be considered as a potential double
//                      but as a new press cycle event (default = 400ms).   
var SingleDoubleBtn = function(Callback, DoublePressTimeOut) {
    this.channel = 0;
    this.control = 0;
    this.value = 0;
    this.status = 0;
    this.group = "";
    this.Callback = Callback;
    if (DoublePressTimeOut) {
        this.DoublePressTimeOut = DoublePressTimeOut;
    } else {
        //Sets a default value of 400 ms
        this.DoublePressTimeOut = 400;
    }
    this.ButtonCount = 0;
    this.ButtonTimer = 0;
};

// Button pressed
SingleDoubleBtn.prototype.ButtonDown = function(channel, control, value,
    status, group) {
    var myself = this;
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;
    if (this.ButtonTimer === 0) { // first press

        this.ButtonTimer =
            engine.beginTimer(this.DoublePressTimeOut,
                function() {
                    myself.ButtonDecide();
                }, true);
        this.ButtonCount = 1;
    } else { // 2nd press (before timer's out)
        engine.stopTimer(this.ButtonTimer);
        this.ButtonTimer = 0;
        this.ButtonCount = 2;
        this.ButtonDecide();
    }
};

// Take action
SingleDoubleBtn.prototype.ButtonDecide = function() {
    this.ButtonTimer = 0;
    this.Callback(this.channel, this.control, this.value, this.status, this
        .group, this.ButtonCount);
    this.ButtonCount = 0;
};

// =======================  LongShortBtn    
// Callback           : Callback function you have to provide (see end of the code), that will return
//                      the original event parameters (channel, control, value, status, group)
//                      and the kind of press event affecting your button (eventkind)
//                      This callback will be called once you release the button
//                      (Value will be equal to UP). You must provide this parameter.
// LongPressThreshold : delay in ms above which a firts press on the
//                      button will be considered as a Long press (default = 500ms).
//                      This parameter is optional.
// CallBackOKLongPress : This callback will give you the same values than the first one
//                       but it will be triggered as soon as the Long press is taken
//                       into account ( at this moment, value = DOWN because you are still
//                       holding down the button). This permits for instance to lit up a light indicating
//                       the user that he/she can release the button. This callback occurs before the first one.
//                       This parameter is optional.
// Like that, you can decide to put the code for the long press in either callback function
var LongShortBtn = function(Callback, LongPressThreshold, CallBackOKLongPress) {
    this.Callback = Callback;
    this.channel = 0;
    this.control = 0;
    this.value = 0;
    this.status = 0;
    this.group = "";
    this.CallBackOKLongPress = CallBackOKLongPress;
    if (LongPressThreshold) {
        this.LongPressThreshold = LongPressThreshold;
    } else {
        //Sets a default value of 500 ms
        this.LongPressThreshold = 500;
    }

    this.ButtonLongPress = false;
    this.ButtonLongPressTimer = 0;
};

// Timer's call back for long press
LongShortBtn.prototype.ButtonAssertLongPress = function() {
    this.ButtonLongPress = true;
    //the timer was stopped, we set it to zero
    this.ButtonLongPressTimer = 0;
    // let's take action of the long press
    // Make sure the callback is a function​ and exist
    if (typeof callback === "function") {
        // Call it, since we have confirmed it is callable​
        this.CallBackOKLongPress(this.channel, this.control, this.value,
            this.status, this.group, LONG_PRESS);
    }
};

LongShortBtn.prototype.ButtonDown = function(channel, control, value, status,
    group) {
    var myself = this;
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;
    this.ButtonLongPress = false;
    this.ButtonLongPressTimer = engine.beginTimer(this.LongPressThreshold,
        function() {
            myself.ButtonAssertLongPress();
        }, true);
};

LongShortBtn.prototype.ButtonUp = function() {
    if (this.ButtonLongPressTimer !== 0) {
        engine.stopTimer(this.ButtonLongPressTimer);
        this.ButtonLongPressTimer = 0;
    }
    if (this.ButtonLongPress) {
        this.Callback(this.channel, this.control, this.value, this.status,
            this.group, LONG_PRESS);
    } else {
        this.Callback(this.channel, this.control, this.value, this.status,
            this.group, QUICK_PRESS);
    }
};

// =======================  LongShortDoubleBtn
// Callback           : Callback function you have to provide (see end of
//                      the code), that will return the original event
//                      parameters (channel, control, value, status, group)
//                      and the kind of press event affecting your button
//                      (eventkind).
//                      This callback will be triggered as soon as you
//                      press the button a second time (Value will be
//                      equal to DOWN), or the Long press is asserted
//                      (value = DOWN because you are still holding down
//                      the button or value=UP because you have realeased
//                      the button only once before it becomes a long press).
// LongPressThreshold : delay in ms above which a firts press on the
//                      button will be considered as a Long press (default = 500ms).
// DoublePressTimeOut : delay in ms above wich a second press on the
//                      button will not be considered as a potential double
//                      but as a new press cycle event (default = 400ms).

var LongShortDoubleBtn = function(Callback, LongPressThreshold,
    DoublePressTimeOut) {
    this.Callback = Callback;
    this.channel = 0;
    this.control = 0;
    this.value = 0;
    this.status = 0;
    this.group = "";
    if (LongPressThreshold) {
        this.LongPressThreshold = LongPressThreshold;
    } else {
        // Sets a default value of 500 ms
        this.LongPressThreshold = 500;
    }
    if (DoublePressTimeOut) {
        this.DoublePressTimeOut = DoublePressTimeOut;
    } else {
        // Sets a default value of 400 ms
        this.DoublePressTimeOut = 400;
    }
    this.ButtonTimer = 0;
    this.ButtonLongPress = false;
    this.ButtonLongPressTimer = 0;
    this.ButtonCount = 0;
};

// Timer's call back for long press
LongShortDoubleBtn.prototype.ButtonAssertLongPress = function() {
    this.ButtonLongPress = true;
    // the timer was stopped, we set it to zero
    this.ButtonLongPressTimer = 0;
    // let's take action of the long press
    this.ButtonDecide();
};

// Timer's callback for single press/double press
LongShortDoubleBtn.prototype.ButtonAssert1Press = function() {
    // Short Timer ran out before it was manually stopped by release
    // of the button (ButtonUp):
    // for sure it is a single click (short or long), we will know
    // when button will be released or when longtimer will stop by itself

    // the timer was stopped, we set it to zero
    this.ButtonTimer = 0;
    this.ButtonCount = 1;
    if (this.ButtonLongPressTimer === 0) {
        // long press timer was stopped (short press)
        //take action
        this.ButtonDecide();
    }
};

// Button pressed (function called by mapper's code)
LongShortDoubleBtn.prototype.ButtonDown = function(channel, control, value,
    status, group) {
    var myself = this;
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;

    if (this.ButtonCount === 0) { //first press (inits)
        // 1st press
        this.ButtonCount = 1;
        // and short press
        this.ButtonLongPress = false;
        this.ButtonLongPressTimer =
            engine.beginTimer(this.LongPressThreshold,
                function() {
                    myself.ButtonAssertLongPress();
                },
                true);
        this.ButtonTimer =
            engine.beginTimer(this.DoublePressTimeOut,
                function() {
                    myself.ButtonAssert1Press();
                },
                true);
    } else if (this.ButtonCount === 1) { // 2nd press (before short timer's out)
        // stop timers...           
        if (this.ButtonLongPressTimer !== 0) {
            engine.stopTimer(this.ButtonLongPressTimer);
            this.ButtonLongPressTimer = 0;
        }
        // we stopped the timer, we have to set it to zero.
        // You must have this reflex : "stopTimer(timer)/timer=0" in mind
        // so that you can test later on if it is active or not. Other else
        // it's value stays with the one given by engine.beginTimer

        // "stopTimer(timer)/timer=0"
        if (this.ButtonTimer !== 0) {
            engine.stopTimer(this.ButtonTimer);
            this.ButtonTimer = 0;
        }

        // 2nd press
        this.ButtonCount = 2;

        // ...and take action immediatly
        this.ButtonDecide();
    } // else :
    // 2nd press after short timer's out, this cannot happen,
    // do nothing
};

// Button released  (function called by mapper's code)
LongShortDoubleBtn.prototype.ButtonUp = function() {
    // button released
    if (this.ButtonLongPress === false) {
        // long press was not asserted by timer (ButtonAssertLongPress)
        // Button is released before timer's out

        // If first Buttun up, long timer is still running
        // stop long timer if it is still running, keep short timer,
        // longpress will never happen
        if (this.ButtonLongPressTimer !== 0) {
            engine.stopTimer(this.ButtonLongPressTimer);
            this.ButtonLongPressTimer = 0;
        }
    } // else :
    // longpressed is confirmed, we already took action in ButtonAssertLongPress
};

// Take actions and call callback
LongShortDoubleBtn.prototype.ButtonDecide = function() {
    if (this.ButtonLongPressTimer !== 0) {
        engine.stopTimer(this.ButtonLongPressTimer);
    }
    this.ButtonLongPressTimer = 0;
    this.ButtonTimer = 0;

    if (this.ButtonLongPress) {
        this.Callback(this.channel, this.control, this.value, this.status,
            this.group, LONG_PRESS);
    } else {
        if (this.ButtonCount === 2) {
            this.Callback(this.channel, this.control, this.value, this.status,
                this.group, DOUBLE_PRESS);
        } else { // We pressed sync only once
            this.Callback(this.channel, this.control, this.value, this.status,
                this.group, QUICK_PRESS);
        }
    }
    // re-init
    this.ButtonCount = 0;
    this.ButtonLongPress = false;
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
    this.delay = 20;
    this.fadersave = 0;
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
        engine.setValue('[Master]', 'crossfader', direction);
    }
};

AutoCut.prototype.Off = function() {
    if (this.enabled) {
        this.enabled = false;
        engine.setValue('[Master]', 'crossfader', 0);
        engine.softTakeover("[Master]", "crossfader", true);
    }
};

// *****************************************************************
// Jog wheel management (scratching, bending, ...)
// *************************************************

var Jogger = function(group, decknum, model) {
    this.decknum = decknum;
    this.group = group;
    this.wheelTouchInertiaTimer = 0;
    this.iCUT = new AutoCut(decknum);
    this.model = model;
};

// ******************************************************************
// Decks
// ******************************************************************
NumarkMixtrack3.deck = function(decknum) {
    this.decknum = decknum;
    this.group = "[Channel" + decknum + "]";
    this.loaded = false;
    this.LoadInitiated = false;
    this.jogWheelsInScratchMode = false;
    this.PADMode = false; //false = not pressed; true = pressed
    this.shiftKey = false;
    this.shiftLock = false;
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
    this.Jog = new Jogger(this.group, this.decknum, "A");
    this.duration = 0;
    // NMTP3 is a "Model A" controller for scratching, it centers on 0.
    // See http://www.mixxx.org/wiki/doku.php/midi_scripting#scratching
    // and see "Jogger" object constructor
};

// ******************************************************************
// Samplers - object
// ******************************************************************

NumarkMixtrack3.sampler = function(decknum) {
    this.decknum = decknum;
    this.group = "[Sampler" + decknum + "]";
    this.loaded = false;
    this.LoadInitiated = false;
    this.PitchFaderHigh = 0;
    this.lastfadervalue = 0;
    this.LEDs = [];
};

NumarkMixtrack3.deck.prototype.TrackIsLoaded = function() {
    return TrackIsLoaded(this.group);
};

NumarkMixtrack3.deck.prototype.StripEffect = function(value, decknum) {
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var ButtonNum;
    var arrayLength = deck.InstantFX.length;
    var i;

    if (FXmode === 1) { //3 FX in 4 FX Units

        if (!deck.shiftKey) {
            // if deck.shiftKey is true, we are fast seeking thru the track
            for (i = 0; i < arrayLength; i++) {
                ButtonNum = deck.InstantFX[i];
                engine.setValue("[EffectRack1_EffectUnit" + decknum + "_Effect" + ButtonNum + "]",
                    "enabled", true);
            }
        }

        engine.setValue("[EffectRack1_EffectUnit" + decknum + "]", "super1", value / 127);

    } else { // 1 FX in 3 FX unit

        if (!deck.shiftKey) {
            // if deck.shiftKey is true, we are fast seeking thru the track
            for (i = 0; i < arrayLength; i++) {
                ButtonNum = deck.InstantFX[i];
                engine.setValue("[EffectRack1_EffectUnit" + ButtonNum + "]",
                    "group_[Channel" + decknum + "]_enable", true);
            }
        }
        engine.setValue("[EffectRack1_EffectUnit1]", "super1", value / 127);
        engine.setValue("[EffectRack1_EffectUnit2]", "super1", value / 127);
        engine.setValue("[EffectRack1_EffectUnit3]", "super1", value / 127);
    }
};

// =====================================================================
// Initialization of the mapping
// =====================================================================

NumarkMixtrack3.decks.D1 = new NumarkMixtrack3.deck("1");
NumarkMixtrack3.decks.D2 = new NumarkMixtrack3.deck("2");
NumarkMixtrack3.decks.D3 = new NumarkMixtrack3.deck("3");
NumarkMixtrack3.decks.D4 = new NumarkMixtrack3.deck("4");
NumarkMixtrack3.samplers.S1 = new NumarkMixtrack3.sampler("1");
NumarkMixtrack3.samplers.S2 = new NumarkMixtrack3.sampler("2");
NumarkMixtrack3.samplers.S3 = new NumarkMixtrack3.sampler("3");
NumarkMixtrack3.samplers.S4 = new NumarkMixtrack3.sampler("4");
NumarkMixtrack3.samplers.S5 = new NumarkMixtrack3.sampler("5");
NumarkMixtrack3.samplers.S6 = new NumarkMixtrack3.sampler("6");
NumarkMixtrack3.samplers.S7 = new NumarkMixtrack3.sampler("7");
NumarkMixtrack3.samplers.S8 = new NumarkMixtrack3.sampler("8");

NumarkMixtrack3.initLEDsObjects = function() {
    var i;
    // Lets create some LEDs
    NumarkMixtrack3.AllLeds =
        new LED(0x90 + ledCategories.master, leds.all);
    NumarkMixtrack3.samplers["S1"].LEDs.PADsampler1 =
        new LED(0x91, leds.PADsampler1);
    NumarkMixtrack3.samplers["S2"].LEDs.PADsampler2 =
        new LED(0x91, leds.PADsampler2);
    NumarkMixtrack3.samplers["S3"].LEDs.PADsampler3 =
        new LED(0x91, leds.PADsampler3);
    NumarkMixtrack3.samplers["S4"].LEDs.PADsampler4 =
        new LED(0x91, leds.PADsampler4);
    NumarkMixtrack3.samplers["S5"].LEDs.PADsampler5 =
        new LED(0x92, leds.PADsampler5);
    NumarkMixtrack3.samplers["S6"].LEDs.PADsampler6 =
        new LED(0x92, leds.PADsampler6);
    NumarkMixtrack3.samplers["S7"].LEDs.PADsampler7 =
        new LED(0x92, leds.PADsampler7);
    NumarkMixtrack3.samplers["S8"].LEDs.PADsampler8 =
        new LED(0x92, leds.PADsampler8);

    for (i = 1; i <= 2; i++) {
        NumarkMixtrack3.decks["D" + i].LEDs.headphones =
            new LED(0x90 + ledCategories.master, leds.headphones1 - 1 + i);
        NumarkMixtrack3.decks["D" + i].LEDs.jogWheelsInScratchMode =
            new LED(0x90 + i, leds.jogWheelsInScratchMode);
        NumarkMixtrack3.decks["D" + i].LEDs.loopin =
            new LED(0x90 + i, leds.loopin);
        NumarkMixtrack3.decks["D" + i].LEDs.loopout =
            new LED(0x90 + i, leds.loopout);
        NumarkMixtrack3.decks["D" + i].LEDs.reloop_exit =
            new LED(0x90 + i, leds.reloop_exit);
        NumarkMixtrack3.decks["D" + i].LEDs.loop_halve =
            new LED(0x90 + i, leds.loop_halve);
        NumarkMixtrack3.decks["D" + i].LEDs.hotCue1 =
            new LED(0x90 + i, leds.hotCue1);
        NumarkMixtrack3.decks["D" + i].LEDs.hotCue2 =
            new LED(0x90 + i, leds.hotCue2);
        NumarkMixtrack3.decks["D" + i].LEDs.hotCue3 =
            new LED(0x90 + i, leds.hotCue3);
        NumarkMixtrack3.decks["D" + i].LEDs.hotCue4 =
            new LED(0x90 + i, leds.hotCue4);
        NumarkMixtrack3.decks["D" + i].LEDs.Cue =
            new LED(0x90 + i, leds.Cue);
        NumarkMixtrack3.decks["D" + i].LEDs.sync =
            new LED(0x90 + i, leds.sync);
        NumarkMixtrack3.decks["D" + i].LEDs.play =
            new LED(0x90 + i, leds.play);
        NumarkMixtrack3.decks["D" + i].LEDs.fx1 =
            new LED(0x90 + i, leds.fx1);
        NumarkMixtrack3.decks["D" + i].LEDs.fx2 =
            new LED(0x90 + i, leds.fx2);
        NumarkMixtrack3.decks["D" + i].LEDs.fx3 =
            new LED(0x90 + i, leds.fx3);
        NumarkMixtrack3.decks["D" + i].LEDs.tap =
            new LED(0x90 + i, leds.tap);
        NumarkMixtrack3.decks["D" + i].LEDs.PADloop1 =
            new LED(0x90 + i, leds.PADloop1);
        NumarkMixtrack3.decks["D" + i].LEDs.PADloop2 =
            new LED(0x90 + i, leds.PADloop2);
        NumarkMixtrack3.decks["D" + i].LEDs.PADloop3 =
            new LED(0x90 + i, leds.PADloop3);
        NumarkMixtrack3.decks["D" + i].LEDs.PADloop4 =
            new LED(0x90 + i, leds.PADloop4);
        NumarkMixtrack3.decks["D" + i].LEDs.meter =
            new LED(0x90 + ledCategories.meters, leds.meter1 - 1 + i);
    }
    // LEDs for Deck 3 and 4
    for (i = 1; i <= 2; i++) {

        var j = i + 2;
        NumarkMixtrack3.decks["D" + j].LEDs.headphones =
            new LED(0x90 + ledCategories.master, leds.headphones1 - 1 + i);
        NumarkMixtrack3.decks["D" + j].LEDs.jogWheelsInScratchMode =
            new LED(0x90 + i, leds.jogWheelsInScratchMode);
        NumarkMixtrack3.decks["D" + j].LEDs.loopin =
            new LED(0x90 + i, leds.loopin);
        NumarkMixtrack3.decks["D" + j].LEDs.loopout =
            new LED(0x90 + i, leds.loopout);
        NumarkMixtrack3.decks["D" + j].LEDs.reloop_exit =
            new LED(0x90 + i, leds.reloop_exit);
        NumarkMixtrack3.decks["D" + j].LEDs.loop_halve =
            new LED(0x90 + i, leds.loop_halve);
        NumarkMixtrack3.decks["D" + j].LEDs.hotCue1 =
            new LED(0x90 + i, leds.hotCue1);
        NumarkMixtrack3.decks["D" + j].LEDs.hotCue2 =
            new LED(0x90 + i, leds.hotCue2);
        NumarkMixtrack3.decks["D" + j].LEDs.hotCue3 =
            new LED(0x90 + i, leds.hotCue3);
        NumarkMixtrack3.decks["D" + j].LEDs.hotCue4 =
            new LED(0x90 + i, leds.hotCue4);
        NumarkMixtrack3.decks["D" + j].LEDs.Cue =
            new LED(0x90 + i, leds.Cue);
        NumarkMixtrack3.decks["D" + j].LEDs.sync =
            new LED(0x90 + i, leds.sync);
        NumarkMixtrack3.decks["D" + j].LEDs.play =
            new LED(0x90 + i, leds.play);
        NumarkMixtrack3.decks["D" + j].LEDs.fx1 =
            new LED(0x90 + i, leds.fx1);
        NumarkMixtrack3.decks["D" + j].LEDs.fx2 =
            new LED(0x90 + i, leds.fx2);
        NumarkMixtrack3.decks["D" + j].LEDs.fx3 =
            new LED(0x90 + i, leds.fx3);
        NumarkMixtrack3.decks["D" + j].LEDs.tap =
            new LED(0x90 + i, leds.tap);
        NumarkMixtrack3.decks["D" + j].LEDs.PADloop1 =
            new LED(0x90 + i, leds.PADloop1);
        NumarkMixtrack3.decks["D" + j].LEDs.PADloop2 =
            new LED(0x90 + i, leds.PADloop2);
        NumarkMixtrack3.decks["D" + j].LEDs.PADloop3 =
            new LED(0x90 + i, leds.PADloop3);
        NumarkMixtrack3.decks["D" + j].LEDs.PADloop4 =
            new LED(0x90 + i, leds.PADloop4);
        NumarkMixtrack3.decks["D" + j].LEDs.meter =
            new LED(0x90 + ledCategories.meters, leds.meter1 - 1 + i);
    }
};

NumarkMixtrack3.initButtonsObjects = function() {
    var i;

    for (i = 1; i <= 4; i++) {
        NumarkMixtrack3.decks["D" + i].LoadButtonControl =
            new LongShortBtn(NumarkMixtrack3.OnLoadButton);
        NumarkMixtrack3.decks["D" + i].SyncButtonControl =
            new LongShortDoubleBtn(NumarkMixtrack3.OnSyncButton);
        NumarkMixtrack3.decks["D" + i].ShiftedPFLButtonControl =
            new SingleDoubleBtn(NumarkMixtrack3.OnShiftedPFLButton);
        NumarkMixtrack3.decks["D" + i].PADLoopButtonHold =
            new LongShortBtn(NumarkMixtrack3.onPADLoopButtonHold);
    }
    for (i = 1; i <= 8; i++) {
            NumarkMixtrack3.samplers["S" + i].PADSampleButtonHold =
            new LongShortBtn(NumarkMixtrack3.onPADSampleButtonHold);
    }
};

NumarkMixtrack3.init = function(id, debug) {
    // Set up the controller to manipulate decks 1 & 2 when this script is loaded 

    print("********* Initialisation process engaged *****************");
    print("              Mapping initialization");
    print("");

    print("==========================================================");
    print("               Initialize variables");
    print("");

    NumarkMixtrack3.libraryMode = false;
    NumarkMixtrack3.id = id; // Store the ID of this device for later use
    NumarkMixtrack3.debug = debug;
    NumarkMixtrack3.deckGroup = {
        '[Channel1]': '[Channel1]',
        '[Channel2]': '[Channel2]',
        '[Channel3]': '[Channel3]',
        '[Channel4]': '[Channel4]'
    };

    NumarkMixtrack3.fxControls = { // used to determine FX Button
        '[EffectRack1_EffectUnit1_Effect1]': 1,
        '[EffectRack1_EffectUnit1_Effect2]': 2,
        '[EffectRack1_EffectUnit1_Effect3]': 3,
        '[EffectRack1_EffectUnit2_Effect1]': 1,
        '[EffectRack1_EffectUnit2_Effect2]': 2,
        '[EffectRack1_EffectUnit2_Effect3]': 3,
        '[EffectRack1_EffectUnit3_Effect1]': 1,
        '[EffectRack1_EffectUnit3_Effect2]': 2,
        '[EffectRack1_EffectUnit3_Effect3]': 3,
        '[EffectRack1_EffectUnit4_Effect1]': 1,
        '[EffectRack1_EffectUnit4_Effect2]': 2,
        '[EffectRack1_EffectUnit4_Effect3]': 3,
        '[EffectRack1_EffectUnit1]': 1, //FXmode !== 1
        '[EffectRack1_EffectUnit2]': 2, //FXmode !== 1
        '[EffectRack1_EffectUnit3]': 3, //FXmode !== 1
        '[EffectRack1_EffectUnit4]': 4  //FXmode !== 1
    };

    NumarkMixtrack3.fxGroups = { //Used to determine deck
        '[EffectRack1_EffectUnit1_Effect1]': 1,
        '[EffectRack1_EffectUnit1_Effect2]': 1,
        '[EffectRack1_EffectUnit1_Effect3]': 1,
        '[EffectRack1_EffectUnit2_Effect1]': 2,
        '[EffectRack1_EffectUnit2_Effect2]': 2,
        '[EffectRack1_EffectUnit2_Effect3]': 2,
        '[EffectRack1_EffectUnit3_Effect1]': 3,
        '[EffectRack1_EffectUnit3_Effect2]': 3,
        '[EffectRack1_EffectUnit3_Effect3]': 3,
        '[EffectRack1_EffectUnit4_Effect1]': 4,
        '[EffectRack1_EffectUnit4_Effect2]': 4,
        '[EffectRack1_EffectUnit4_Effect3]': 4,
        'group_[Channel1]_enable': 1, //FXmode !== 1
        'group_[Channel2]_enable': 2, //FXmode !== 1
        'group_[Channel3]_enable': 3, //FXmode !== 1
        'group_[Channel4]_enable': 4  //FXmode !== 1
    };

    NumarkMixtrack3.Autoloop = {
        'beatloop_2_enabled': 1,
        'beatloop_4_enabled': 2,
        'beatloop_8_enabled': 3,
        'beatloop_16_enabled': 4,
        'beatloop_0.125_enabled': 1,
        'beatloop_0.25_enabled': 2,
        'beatloop_0.5_enabled': 3,
        'beatloop_1_enabled': 4
    };

    var i, j, k;

    print("==========================================================");
    print("                  Initialize LEDs");
    print("");

    // Create LEDs Objects
    NumarkMixtrack3.initLEDsObjects();

    // Turn ON all the lights: the only way PADMode Leds light up 
    NumarkMixtrack3.AllLeds.onOff(ON);

    // Initialise some others (PAD LEDs)
    for (k = 1; k <= 8; k++) {
        NumarkMixtrack3.samplers["S" + k].LEDs["PADsampler" + k].onOff(
            PADcolors.black);
    }

    for (i = 1; i <= 4; i++) {
        for (j = 1; j <= 4; j++) {
            NumarkMixtrack3.decks["D" + i].LEDs["PADloop" + j].onOff(
                PADcolors.black);
        }

        NumarkMixtrack3.decks["D" + i].LEDs.jogWheelsInScratchMode.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.headphones.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.loopin.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.loopout.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.reloop_exit.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.loop_halve.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.hotCue1.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.hotCue2.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.hotCue3.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.hotCue4.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.Cue.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.sync.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.play.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.fx1.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.fx2.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.fx3.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.tap.onOff(OFF);
        NumarkMixtrack3.decks["D" + i].LEDs.meter.onOff(OFF);

        print("   LEDs state set for deck " + "D" + i);
    }

    print("==========================================================");
    print("                 Initialize Buttons");
    print("");
    NumarkMixtrack3.initButtonsObjects();

    print("==========================================================");
    print("                Init Soft Takeovers");
    print("");

    // Set soft-takeover for all Sampler volumes
    for (i = engine.getValue("[Master]", "num_samplers"); i >= 1; i--) {
        engine.softTakeover("[Sampler" + i + "]", "pregain", true);
    }

    for (i = engine.getValue("[Master]", "num_decks"); i >= 1; i--) {
        engine.softTakeover("[Channel" + i + "]", "rate", true); // Enable soft-takeover for Pitch slider
        engine.softTakeover("[Channel" + i + "]", "volume", true); // Enable soft-takeover for volume
        engine.setParameter("[Channel" + i + "]", "volume", 0); // Set volume to zero for each deck (initial load only)
    }

    for (i = 1; i <= 8; i++) {
        engine.connectControl("[Sampler" + i + "]", "play",
            "NumarkMixtrack3.OnSamplePlayStop");
    }

    NumarkMixtrack3.initDeck('[Channel1]', false); //Initial load, "remove" is set to false
    NumarkMixtrack3.initDeck('[Channel2]', false);

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
    remove = (typeof remove !== 'undefined') ? remove : false;
    var OnDeck = parseInt(NumarkMixtrack3.channelRegEx.exec(group)[1]); 
    var OffDeck = OnDeck;
    var i; 

    if (OffDeck <= 2) {
        OffDeck += 2;
    } else {
        OffDeck -= 2;
    }

    if (remove) {
        print("==========================================================");
        print("           Disconnect controls from deck " + OffDeck);
        print("");

        // make sure that the shift is no longer active on either deck to prevent confusion
        NumarkMixtrack3.decks["D" + OffDeck].shiftKey = false;
        NumarkMixtrack3.decks["D" + OnDeck].shiftKey = false; 
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.jogWheelsInScratchMode.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.headphones.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.loopin.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.loopout.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.reloop_exit.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.loop_halve.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.hotCue1.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.hotCue2.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.hotCue3.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.hotCue4.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.Cue.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.sync.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.play.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.fx1.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.fx2.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.fx3.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.tap.onOff(OFF);
        NumarkMixtrack3.decks["D" + OffDeck].LEDs.meter.onOff(OFF);
        
        for (i = 0; i < 4; i++) {
        var index = i + 1;

        NumarkMixtrack3.decks["D" + OffDeck].LEDs["PADloop" + index].onOff(
            OFF);
        NumarkMixtrack3.decks["D" + OnDeck].LEDs["PADloop" + index].onOff(
            PADcolors.yellow);
        }
    } 
        
        print("==========================================================");
        print("         Connect controls and triggers deck "+ OnDeck);
        print("");
    
    var controlsToFunctions = {
        'hotcue_1_enabled': 'NumarkMixtrack3.OnHotcueChange',
        'hotcue_2_enabled': 'NumarkMixtrack3.OnHotcueChange',
        'hotcue_3_enabled': 'NumarkMixtrack3.OnHotcueChange',
        'hotcue_4_enabled': 'NumarkMixtrack3.OnHotcueChange',
        'track_samples': 'NumarkMixtrack3.OnTrackLoaded',
        'VuMeter': 'NumarkMixtrack3.OnVuMeterChange',
        'playposition': 'NumarkMixtrack3.OnPlaypositionChange',
        'volume': 'NumarkMixtrack3.OnVolumeChange',
        'pfl': 'NumarkMixtrack3.OnPFLStatusChange',
        'duration': 'NumarkMixtrack3.OnLoadSelectedTrack',
        'play_indicator': 'NumarkMixtrack3.OnPlayIndicatorChange',
        'cue_indicator': 'NumarkMixtrack3.OnCuePointChange',
        'loop_start_position': 'NumarkMixtrack3.OnLoopInOutChange',
        'loop_end_position': 'NumarkMixtrack3.OnLoopInOutChange',
        'loop_enabled': 'NumarkMixtrack3.OnLoopInOutChange',
        'sync_enabled': 'NumarkMixtrack3.OnSyncButtonChange',
        'beatloop_2_enabled': 'NumarkMixtrack3.OnPADLoopButtonChange',
        'beatloop_4_enabled': 'NumarkMixtrack3.OnPADLoopButtonChange',
        'beatloop_8_enabled': 'NumarkMixtrack3.OnPADLoopButtonChange',
        'beatloop_16_enabled': 'NumarkMixtrack3.OnPADLoopButtonChange',
        'beatloop_1_enabled': 'NumarkMixtrack3.OnPADLoopButtonChange',
        'beatloop_0.5_enabled': 'NumarkMixtrack3.OnPADLoopButtonChange',
        'beatloop_0.25_enabled': 'NumarkMixtrack3.OnPADLoopButtonChange',
        'beatloop_0.125_enabled': 'NumarkMixtrack3.OnPADLoopButtonChange'
    };

   if (FXmode === 1) { //3 FX in 4 FX Units
        engine.connectControl("[EffectRack1_EffectUnit" + OnDeck + "_Effect1]", "enabled",
            "NumarkMixtrack3.OnEffectEnabled");
        engine.connectControl("[EffectRack1_EffectUnit" + OnDeck + "_Effect2]", "enabled",
            "NumarkMixtrack3.OnEffectEnabled");
        engine.connectControl("[EffectRack1_EffectUnit" + OnDeck + "_Effect3]", "enabled",
            "NumarkMixtrack3.OnEffectEnabled");

        engine.trigger("[EffectRack1_EffectUnit" + OnDeck + "_Effect1]", "enabled");
        engine.trigger("[EffectRack1_EffectUnit" + OnDeck + "_Effect2]", "enabled");
        engine.trigger("[EffectRack1_EffectUnit" + OnDeck + "_Effect3]", "enabled");

    } else { // 1 FX in 3 FX unit

        for (i = 0; i <= 4; i++) {
            engine.connectControl("[EffectRack1_EffectUnit1]",
                "group_[Channel" + OnDeck + "]_enable", "NumarkMixtrack3.OnEffectEnabled");

            engine.connectControl("[EffectRack1_EffectUnit2]",
                "group_[Channel" + OnDeck + "]_enable", "NumarkMixtrack3.OnEffectEnabled");

            engine.connectControl("[EffectRack1_EffectUnit3]",
                "group_[Channel" + OnDeck + "]_enable", "NumarkMixtrack3.OnEffectEnabled");
        }
    }


    // Set InstantFX LEDs to flash if required
    var arrayLength = NumarkMixtrack3.decks["D" + OnDeck].InstantFX.length;

    for (i = 0; i < arrayLength; i++) {
        var ButtonNum = NumarkMixtrack3.decks["D" + OnDeck].InstantFX[i];
        NumarkMixtrack3.decks["D" + OnDeck].LEDs["fx" + ButtonNum].flashOn(250, ON, 250);
    }

    for (var control in controlsToFunctions) {
        if (controlsToFunctions.hasOwnProperty(control)) {
            if (remove){
                engine.connectControl("[Channel" + OffDeck + "]", control, controlsToFunctions[control], true);
            }
            engine.connectControl(group, control, controlsToFunctions[control]);
            engine.trigger(group, control);
        }
    }

    if (!remove && FXmode === 1) { 
        for (i = 1; i <= 4; i++) {
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
    return (decknum);
};

NumarkMixtrack3.ShiftButton = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (value === DOWN) {
        deck.shiftKey = true;
    } else {
        deck.shiftKey = false;
    }
};

/******************     Play Button :
 * - Press         : to Play / pause the track. If no track is loaded,
 *                   Load the selected track (if any) and play.
 * - SHIFT+ press : Go to Cue point and play (stutter).
 * *********************************************************************/
NumarkMixtrack3.PlayButton = function(channel, control, value, status, group) {
    if (!value) return;
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (value === DOWN) {
        if (!deck.shiftKey) {
            // play/pause

            if (!deck.TrackIsLoaded()) {
                // If a track is not loaded, load the selected track (if any) and play
                engine.setValue("[Channel" + decknum + "]", "LoadSelectedTrackAndPlay", true);
            } else {
                // Else play/pause
                toggleValue("[Channel" + decknum + "]", "play");
            }
        } else {
            // shifted : stutter
            engine.setValue("[Channel" + decknum + "]", "play_stutter", true);
        }
    }
};

/******************     Browse Button/Knob :
 * Track list mode.....:
 * - Turn         : Select a track in the play list
 * - Push         : Load Selected track into first stopped deck
 * Directory mode...... :
 * - SHIFT + Turn : Select Play List/Side bar item
 * - SHIFT + Push : Open/Close selected side bar item.
 *                   Load the selected track (if any) and play.
 * *********************************************************************/
NumarkMixtrack3.BrowseButton = function(channel, control, value, status, group) {
    var shifted = (NumarkMixtrack3.decks.D1.shiftKey || NumarkMixtrack3.decks
        .D2.shiftKey || NumarkMixtrack3.decks.D3.shiftKey || NumarkMixtrack3.decks.D4.shiftKey);

    var maxview = !NumarkMixtrack3.libraryMode;
    var LibraryCommand;
    var LibraryGroup;
    var expand;
    var contract;

    if (DarkMetalSkin) {
        LibraryGroup = "[Hifi]";
        LibraryCommand = "show";
        expand = 0;
        contract = 1;
    } else {
        LibraryGroup = "[Master]";
        LibraryCommand = "maximize_library";
        expand = 1;
        contract = 0;
    }

    if (shifted && value === ON) {
        // SHIFT+ BROWSE push : directory mode -- > Open/Close selected side bar item
        engine.setValue(group, "ToggleSelectedSidebarItem", true);
    } else {
        // Browse push : maximize/minimize library view
        if (value === ON) {

            NumarkMixtrack3.libraryMode = !NumarkMixtrack3.libraryMode;
            if (maxview) {
                engine.setValue(LibraryGroup, LibraryCommand, expand);

            } else {
                engine.setValue(LibraryGroup, LibraryCommand, contract);
            }
        }
    }
};

NumarkMixtrack3.BrowseKnob = function(channel, control, value, status, group) {
    var i;
    var shifted = (NumarkMixtrack3.decks.D1.shiftKey || NumarkMixtrack3.decks
        .D2.shiftKey || NumarkMixtrack3.decks.D3.shiftKey || NumarkMixtrack3.decks.D4.shiftKey);
    // value = 1 / 2 / 3 ... for positive //value = 1 / 2 / 3  
    var nval = (value > 0x40 ? value - 0x80 : value);

    if (shifted) {
        // SHIFT+Turn BROWSE Knob : directory mode --> select Play List/Side bar item
        if (nval > 0) {
            for (i = 0; i < nval; i++) {
                engine.setValue(group, "SelectNextPlaylist", 1);
            }
        } else {
            for (i = 0; i < -nval; i++) {
                engine.setValue(group, "SelectPrevPlaylist", 1);
            }
        }
    } else {
        // Turn BROWSE Knob : track list mode -->  select track
        engine.setValue(group, "SelectTrackKnob", nval);
    }
};

NumarkMixtrack3.PadModeButton = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var i;
    deck.PADMode = false;

    if (value === DOWN) {
        deck.PADMode = true;
        //ensure all LEDs are ON (default)
        for (i = 1; i <= 8; i++) {
            NumarkMixtrack3.samplers["S" + i + ""].LEDs["PADsampler" + i + ""].onOff(PADcolors
                .purple);
        }
        NumarkMixtrack3.decks["D" + decknum].LEDs["PADloop1"].onOff(PADcolors.yellow);
        NumarkMixtrack3.decks["D" + decknum].LEDs["PADloop2"].onOff(PADcolors.yellow);
        NumarkMixtrack3.decks["D" + decknum].LEDs["PADloop3"].onOff(PADcolors.yellow);
        NumarkMixtrack3.decks["D" + decknum].LEDs["PADloop4"].onOff(PADcolors.yellow);
    }

    // Now check which one should be blinking
    // Need to check if loop is enabled; if yes, stop it , else start it 
    //Autoloop
    if (value === DOWN) {
        for (i = 0; i < loopsize.length; i++) {
            var index = i + 1;
            if (index > 4) {
                index = index - 4;
            }

            if (engine.getValue("[Channel" + decknum + "]", "beatloop_" + loopsize[i] +
                    "_enabled")) {
                deck.LEDs["PADloop" + index].flashOn(300, PADcolors.yellow,
                    300);
            }
        }

        //Sampler
        for (i = 1; i <= 8; i++) {
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
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var i;

    if (value === DOWN) {
        deck.LEDs["headphones"].onOff(ON);
        deck.faderstart = false;

        if (smartPFL) {
            for (i = 1; i <= 4; i++) {
                //Change headphone cue (pfl) to the deck on which the song loaded.
                engine.setValue("[Channel" + i + "]", "pfl", (decknum === i) ? 1 : 0);
            }
        }

        if (deck.shiftKey) {
            // SHIFT + Load = fader start activated

            deck.faderstart = true;
            deck.LEDs["headphones"].flashOn(250, ON, 250);

            if (!deck.TrackIsLoaded) {
                engine.setValue("[Channel" + decknum + "]", 'LoadSelectedTrack', true);
            }
        }

        deck.LoadButtonControl.ButtonDown(channel, control, value, status,
            "[Channel" + decknum + "]");
    } else {

        deck.LoadButtonControl.ButtonUp();
    }
};

// Callback for the Load Button
NumarkMixtrack3.OnLoadButton = function(channel, control, value, status, group,
    eventkind) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);

    if (eventkind === LONG_PRESS) {
        engine.setValue("[Channel" + decknum + "]", 'eject', true);
    } else {
        engine.setValue("[Channel" + decknum + "]", 'LoadSelectedTrack', true);
    }
};

NumarkMixtrack3.OnLoadSelectedTrack = function(value, group, control) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var TrackDuration = engine.getValue("[Channel" + decknum + "]", "duration");
    var i;

    if (smartPFL && deck.duration !== TrackDuration && TrackDuration !== 0) {
        for (i = 1; i <= 4; i++) {
            //Change headphone cue (pfl) to the deck on which the song loaded.
            engine.setValue("[Channel" + i + "]", "pfl", (decknum === i) ? 1 : 0);
        }
    }
    deck.duration = engine.getValue("[Channel" + decknum + "]", "duration");
};

/******************     Sync button :
 * - Short Press  : Press once to synchronize the tempo (BPM) and phase
 *                  to that of to that of the other track.
 * - Double Press : press twice QUICKLY to play the track immediatly,
 *                  synchronized to the tempo (BPM) and to the phase of
 *                 the other track, if the track was paused.
 * - Long Press (Sync Locck) :
 *                 Hold for at least half of a second to enable sync lock
 *                 for this deck. Decks with sync locked will all play at
 *                 the same tempo, and decks that also have quantize
 *                 enabled will always have their beats lined up.
 * If the Sync Loack was previously activated, it just desactivate it,
 * regardless of the Short press/Double Press
 *
 * - SHIFT + Press : Toggle Key Lock
 ***********************************************************************/
NumarkMixtrack3.SyncButton = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (!deck.shiftKey) {
        if (value === DOWN) {

            deck.SyncButtonControl.ButtonDown(channel, control, value,
                status, "[Channel" + decknum + "]");
        } else {

            deck.SyncButtonControl.ButtonUp();
        }
    } else {
        if (value === DOWN) {
            toggleValue("[Channel" + decknum + "]", "keylock");
        }
    }
};

// Callback for the SYNC Button
NumarkMixtrack3.OnSyncButton = function(channel, control, value, status, group,
    eventkind) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];


    if (eventkind === LONG_PRESS) {
        deck.LEDs.sync.onOff(ON);
        engine.setValue(group, 'sync_enabled', true);
    } else {
        if (engine.getValue(group, 'sync_enabled')) {
            // If sync lock is enabled, simply disable sync lock
            engine.setValue(group, 'sync_enabled', false);
            deck.LEDs.sync.onOff(OFF);
        } else {
            if (eventkind === DOUBLE_PRESS && !noPlayOnSyncDoublePress) {
                // Double press : Sync and play (if the track was paused
                // the playback starts, synchronized to the other track
                engine.setValue(group, 'play', true);
                engine.setValue(group, 'beatsync', true);
                deck.LEDs.sync.flashOn(100, ON, 100, 3);

            } else { // We pressed sync only once, we sync the track
                // with the other track (eventkind === QUICK_PRESS
                engine.setValue(group, 'beatsync', true);
                deck.LEDs.sync.flashOn(100, ON, 100, 3);
            }
        }
    }
};

NumarkMixtrack3.OnSyncButtonChange = function(value, group, key) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var valIn = engine.getValue(group, 'sync_enabled');

    if (valIn) {
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
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (!deck.shiftKey) {
        // Don't set Cue accidentaly at the end of the song
        if (engine.getValue("[Channel" + decknum + "]", "playposition") <= 0.97) {
            engine.setValue("[Channel" + decknum + "]", "cue_default", value ? 1 : 0);
        } else {
            engine.setValue("[Channel" + decknum + "]", "cue_preview", value ? 1 : 0);
        }
    } else {
        engine.setValue("[Channel" + decknum + "]", "start", true);
    }
};

NumarkMixtrack3.OnCuePointChange = function(value, group, control) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.LEDs.Cue.onOff((value) ? ON : OFF);
};

// Pitch faders send 2*7bits
NumarkMixtrack3.PitchFaderHighValue = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.PitchFaderHigh = value;
};

NumarkMixtrack3.PitchFaderLowValue = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var calcvalue = (8192 - ((deck.PitchFaderHigh * 128) + value)) / 8192;
    engine.setValue("[Channel" + decknum + "]", "rate", calcvalue);
};

NumarkMixtrack3.toggleJogMode = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (value === DOWN) {
        // Toggle setting and light
        deck.jogWheelsInScratchMode = !deck.jogWheelsInScratchMode;
        deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ?
            ON : OFF);
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
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var isplaying = engine.getValue("[Channel" + decknum + "]", "play");
    
    deck.touch = false;
    deck.iCutStatus = false;
    deck.seekingfast = false;

    if (value === DOWN) {

        if (deck.jogWheelsInScratchMode || !isplaying) {

            engine.scratchEnable(decknum, intervalsPerRev, rpm, alpha, beta);

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

            if (fastSeekEnabled && deck.shiftKey) {
                deck.seekingfast = true;
            }
        }
    } else {
        engine.scratchDisable(decknum, true);
        deck.seekingfast = false;
        deck.Jog.iCUT.Off();
    }
};

NumarkMixtrack3.WheelMove = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];


    // Set jog value
    var adjustedJog = parseFloat(value);
    var posNeg = 1;

    if (adjustedJog > 63) { // Counter-clockwise
        posNeg = -1;
        adjustedJog = value - 128;
    }

    /*  This function performs that actions defined by wheel touch 
        - Pitch bend / jog = default
        - fast seek - deck.seekingfast = true
        - iCut = deck.iCutStatus = true
        - Scratching = deck.touch = true */

    if (deck.iCutStatus) {
        deck.Jog.iCUT.On();
        deck.Jog.iCUT.FaderCut(adjustedJog, decknum);
    }

    if (!deck.seekingfast && !deck.iCutStatus && !deck.touch && deck.jogWheelsInScratchMode) {
    }

    if (deck.seekingfast) {
        engine.setValue(deck.Jog.group, "beatjump", adjustedJog * 2);
    }

    engine.scratchTick(decknum, adjustedJog);
    
    if(PitchBendOnWheelOff) {
        //Pitch bend when playing - side or platter have same effect        
        if (engine.getValue(deck.Jog.group, "play")) {
            var gammaInputRange = 13; // Max jog speed
            var maxOutFraction = 0.8; // Where on the curve it should peak; 0.5 is half-way
            var sensitivity = 0.5; // Adjustment gamma
            var gammaOutputRange = 0.75; // Max rate change

            adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(
                    adjustedJog) / (gammaInputRange * maxOutFraction),
                sensitivity);
            engine.setValue(deck.Jog.group, "jog", adjustedJog);
        }
    } else {
        //Pitch bend when playing - side or platter have same effect        
        if (deck.jogWheelsInScratchMode) {
            if (engine.getValue(deck.Jog.group, "play")) {
                var gammaInputRange = 13; // Max jog speed
                var maxOutFraction = 0.8; // Where on the curve it should peak; 0.5 is half-way
                var sensitivity = 0.5; // Adjustment gamma
                var gammaOutputRange = 0.75; // Max rate change

                adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(
                        adjustedJog) / (gammaInputRange * maxOutFraction),
                    sensitivity);
                engine.setValue(deck.Jog.group, "jog", adjustedJog);
            }
        } 
    }
};

NumarkMixtrack3.HotCueButton = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var hotCue = control - leds.hotCue1 + 1;

    if (deck.shiftKey) {
        if (value === DOWN) {
            engine.setValue("[Channel" + decknum + "]", "hotcue_" + hotCue + "_clear", true);
            deck.LEDs["hotCue" + hotCue].onOff(OFF);
        }
    } else {
        if (value === DOWN) {
            engine.setValue("[Channel" + decknum + "]", "hotcue_" + hotCue + "_activate", 1);
        } else {
            engine.setValue("[Channel" + decknum + "]", "hotcue_" + hotCue + "_activate", 0);
        }
    }
};

NumarkMixtrack3.OnHotcueChange = function(value, group, control) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var padindex = parseInt(NumarkMixtrack3.channelRegEx.exec(control)[1]);

    deck.LEDs["hotCue" + padindex].onOff((value) ? ON : OFF);
};

NumarkMixtrack3.SamplerButton = function(channel, control, value, status,
    group) {
    var isplaying = engine.getValue(group, "play");
    var padindex = parseInt(group.substring(8, 9));
    var decknum = padindex;
    
    if (decknum > 4) {
        decknum = 2;
        engine.setValue("[Deere]","sampler_bank_2", true);
    } else {
        decknum = 1;
        engine.setValue("[Deere]","sampler_bank_1", true);
    }

    decknum = NumarkMixtrack3.deckFromGroup("[Channel" + decknum + "]");
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var sampler = NumarkMixtrack3.samplers["S" + padindex];

    if (value === DOWN) {


    sampler.PADSampleButtonHold.ButtonDown(channel, control, value, status, group);

        if (!isplaying) {
            if (deck.shiftKey) {
                //Shift is on, play sampler with no Sync
                engine.setValue(group, "beatsync", 0);
                engine.setValue(group, "cue_gotoandplay", 1);
                sampler.LEDs["PADsampler" + padindex].flashOn(300,
                    PADcolors.purple, 300);
            } else {
                //play sampler with Sync
                engine.setValue(group, "cue_gotoandplay", 1);
                engine.setValue(group, "beatsync", 1);
                sampler.LEDs["PADsampler" + padindex].flashOn(300,
                    PADcolors.purple, 300);
            }
        } else {
            engine.setValue(group, "stop", 1);
            sampler.LEDs["PADsampler" + padindex].onOff(ON);
        }
    }

    if (value === OFF) {
       sampler.PADSampleButtonHold.ButtonUp();
    }
};

NumarkMixtrack3.onPADSampleButtonHold = function(channel, control, value, status,
    group, eventkind) {
    var decknum = parseInt(group.substring(8, 9));
    var sampler = NumarkMixtrack3.samplers["S" + decknum];
    var padindex = parseInt(group.substring(8, 9));

    if (decknum > 4) {
        decknum = 2;
    } else {
        decknum = 1;
    }

    //The event is a Long Press, LONG_PRESS is true, we set a variable so that when the 
    // pad button is lifted, the Sampler stops
    if (eventkind === LONG_PRESS) {
        engine.setValue(group, "stop", 1);
        sampler.LEDs["PADsampler" + padindex].onOff(ON);
    }
};

NumarkMixtrack3.OnSamplePlayStop = function(value, group, control) {
    var decknum = parseInt(group.substring(8, 9));
    var sampler = NumarkMixtrack3.samplers["S" + decknum];

    if (value === 1) {
        sampler.LEDs["PADsampler" + decknum].flashOn(300, PADcolors.purple,
            300);
    } else {
        sampler.LEDs["PADsampler" + decknum].onOff(ON);
    }
};

NumarkMixtrack3.PADLoopButton = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var padindex = control - leds.PADloop1 + 1;
    var trueFalse;
    var loopsizeNew;

    if (deck.shiftKey) {
        loopsizeNew = loopsize[padindex + 3];
    } else {
        loopsizeNew = loopsize[padindex - 1];
    }

    var loopCommand1; //verify if loop is active
    var loopCommand2; //enable loop
    var loopCommand3; //stop loop

    if (beatlooprollActivate) {
        loopCommand1 = "beatlooproll_" + loopsizeNew + "_activate";
        loopCommand2 = "beatlooproll_" + loopsizeNew + "_activate";
        loopCommand3 = "beatlooproll_" + loopsizeNew + "_activate";
        trueFalse = false;
    } else {
        loopCommand1 = "beatloop_" + loopsizeNew + "_enabled";
        loopCommand2 = "beatloop_" + loopsizeNew + "_toggle";
        loopCommand3 = "reloop_exit";
        trueFalse = true;
    }

    if (value === DOWN && deck.duration !== 0) {
        // make sure all LED are ON
        deck.LEDs["PADloop1"].onOff(PADcolors.yellow);
        deck.LEDs["PADloop2"].onOff(PADcolors.yellow);
        deck.LEDs["PADloop3"].onOff(PADcolors.yellow);
        deck.LEDs["PADloop4"].onOff(PADcolors.yellow);

        if (engine.getValue(group, loopCommand1)) {
            // Loop is active, turn it off
            engine.setValue("[Channel" + decknum + "]", loopCommand3, trueFalse);
            deck.LEDs["PADloop" + padindex].onOff(PADcolors.yellow);

        } else {
            // Loop is not active, turn it on
            deck.LEDs["PADloop" + padindex].flashOn(250, PADcolors.yellow,
                250);
            engine.setValue("[Channel" + decknum + "]", loopCommand2, true);
        }
    // Event if long press
        deck.PADLoopButtonHold.ButtonDown(channel, control, value, status,
            "[Channel" + decknum + "]");
    }

    if (value === OFF && deck.duration !== 0) { //This triggers the callback function for "PADLoopButtonHold"
        deck.PADLoopButtonHold.ButtonUp();
    }
};

NumarkMixtrack3.onPADLoopButtonHold = function(channel, control, value, status,
    group, eventkind) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var padindex = control - leds.PADloop1 + 1;
    var loopsizeNew;

    if (deck.shiftKey) {
        loopsizeNew = loopsize[padindex + 3];
    } else {
        loopsizeNew = loopsize[padindex - 1];
    }
    
    if (eventkind === LONG_PRESS) {
        if (beatlooprollActivate) {
            engine.setValue("[Channel" + decknum + "]", "reloop_exit", true);
        } else {
            engine.setValue("[Channel" + decknum + "]", "beatlooproll_" + loopsizeNew + "_activate", false);
        }
    }
};

NumarkMixtrack3.OnPADLoopButtonChange = function(value, group, control) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var padindex = NumarkMixtrack3.Autoloop[control];
    var i;
    var index;

    if (value === 1) {
        deck.LEDs["PADloop" + padindex].flashOn(300, PADcolors.yellow, 300);
    } else {
        deck.LEDs["PADloop" + padindex].onOff(PADcolors.yellow);
    }
    
    // on initialization of deck, the value "0" would cause the pad LED to stop blinking on the 2nd pass
    // of triggers. This gives ensures that the PAD mode reflect the proper state of AutoLoop
    for (i = 0; i < loopsize.length; i++) {
        index = i + 1;
        if (index > 4) {
            index = index - 4;
        }

        var test = engine.getValue("[Channel" + decknum + "]", "beatloop_" + loopsize[i] +
                "_enabled");

        if (test) {
            NumarkMixtrack3.decks["D" + decknum].LEDs["PADloop" + index].flashOn(300, PADcolors.yellow, 300);
        }
    }
};

NumarkMixtrack3.StripTouchEffect = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (deck.shiftKey) {
        engine.setValue("[Channel" + decknum + "]", "playposition", value / 127);
    } else {
        deck.StripEffect(value, decknum);
    }
};

NumarkMixtrack3.InstantFXOff = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var i;
    var ButtonNum;
    var arrayLength = deck.InstantFX.length;

    for (i = 0; i < arrayLength; i++) {
        ButtonNum = deck.InstantFX[i];

        if (FXmode === 1) { //3 FX in 4 FX Units
            engine.setValue("[EffectRack1_EffectUnit" + decknum + "_Effect" + ButtonNum + "]",
                "enabled", false);
        } else { // 1 FX in 3 FX unit
            engine.setValue("[EffectRack1_EffectUnit" + ButtonNum + "]",
                "group_[Channel" + decknum + "]_enable", false);
        }
    }
};

NumarkMixtrack3.FXButton = function(channel, control, value, status, group) {
    //if (!value) return; //not sure why this is there
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var ButtonNum = control - leds.fx1 + 1;
    var i;
    var new_value;

    if (value === DOWN && deck.TapDown && !deck.PADMode) {

        if (deck.InstantFX.indexOf(ButtonNum) > -1) {
            // we are removing the instantFX option from the selected FX button
            // Find and remove item from an array
            i = deck.InstantFX.indexOf(ButtonNum);
            if (i != -1) {
                deck.InstantFX.splice(i, 1);
            }

            if (deck.InstantFX.indexOf(ButtonNum) === -1) {
                new_value = engine.getValue("[EffectRack1_EffectUnit" + decknum + "_Effect" +
                    ButtonNum + "]", "enabled");
                deck.LEDs["fx" + ButtonNum].onOff(new_value ? ON : OFF);
            }

        } else {

            // we are adding the effect to the InstantFX list, or removing it
            // tap + FX button enables/disables InstantFX, we check deck.TapDown to know if tap is pressed
            //in order for FX LEDs to flas in sync, we need to loop thru the array to reset the LEDs
            deck.InstantFX.push(ButtonNum);

            // Get all LEDs to flash in sync
            var arrayLength = deck.InstantFX.length;
            for (i = 0; i < arrayLength; i++) {

                ButtonNum = deck.InstantFX[i];
                deck.LEDs["fx" + ButtonNum].flashOn(250, ON, 250);
            }
        }
    } else if (value === DOWN && !deck.TapDown && !deck.PADMode) {
        if (deck.shiftKey) {
            // Select Effect 
            if (FXmode === 1) { //3 FX in 4 FX Units
                engine.setValue("[EffectRack1_EffectUnit" + decknum + "_Effect" + ButtonNum + "]",
                    "next_effect", true); // Load FX, but not active
                engine.setValue("[EffectRack1_EffectUnit" + decknum + "_Effect" + ButtonNum + "]",
                    "enabled", false); // Load FX, but not active
                engine.setValue("[EffectRack1_EffectUnit" + decknum + "]",
                    "group_[Channel" + decknum + "]_enable", true); // An FX is loaded, activate Effect Unit
            } else {
                engine.setValue("[EffectRack1_EffectUnit" + ButtonNum + "]", "prev_chain", true);
            }
        } else {
            // Toggle effect if InstantFX is not active
            if (deck.InstantFX.indexOf(ButtonNum) === -1) {
                if (FXmode === 1) { //3 FX in 4 FX Units
                    new_value = !engine.getValue("[EffectRack1_EffectUnit" + decknum + "_Effect" + ButtonNum + "]",
                        "enabled");
                    engine.setValue("[EffectRack1_EffectUnit" + decknum + "_Effect" + ButtonNum + "]", "enabled",
                        new_value);
                } else {
                    new_value = !engine.getValue("[EffectRack1_EffectUnit" +
                        ButtonNum + "]", "group_[Channel" + decknum + "]_enable");
                    engine.setValue("[EffectRack1_EffectUnit" + ButtonNum + "]",
                        "group_[Channel" + decknum + "]_enable", new_value);
                }
                deck.LEDs["fx" + ButtonNum].onOff(new_value ? ON : OFF);
            }
        }
    }

    // Standard FX Control done, now deal with extra features

    if (value === DOWN && deck.PADMode && ButtonNum === 2) {
        engine.spinback(decknum, true); // enable spinback effect
    } else {
        engine.spinback(decknum, false); // disable spinback effect
    }

    if (deck.PADMode && ButtonNum === 1) {
        NumarkMixtrack3.brake_button(decknum, value);
    }
};

NumarkMixtrack3.brake_button = function(decknum, value) {
   var activate = value > 0;

    if (activate) {
        engine.brake(decknum, true); // enable brake effect
    } else {
        engine.brake(decknum, false); // disable brake effect
    }   
};

NumarkMixtrack3.OnEffectEnabled = function(value, group, control) {
  var index;
  var decknum;
    if (FXmode === 1) {
        index = NumarkMixtrack3.fxControls[group];
        decknum = NumarkMixtrack3.fxGroups[group];
    } else {
        index = NumarkMixtrack3.fxControls[group];
        decknum = NumarkMixtrack3.fxGroups[control];
    }

    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (deck.InstantFX.indexOf(index) === -1) {
        var new_value = engine.getValue(group, control);
        deck.LEDs["fx" + index].onOff(new_value ? ON : OFF);
    }
};

/******************     Shift Button :
 * - Press                : toggle PFL
 * - SHIFT + press        : toggle slip mode
 * - SHIFT + double press : toggle quantize mode
 * *********************************************************************/
NumarkMixtrack3.PFLButton = function(channel, control, value, status, group) {
    if (!value) return;
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var i;
    
    if (value === DOWN) {
        if (deck.shiftKey) {
            deck.ShiftedPFLButtonControl.ButtonDown(channel, control, value,
                status, "[Channel" + decknum + "]");
        } else {
            toggleValue("[Channel" + decknum + "]", "pfl");
            for (i = 1; i <= 4 ; i++) {
                if (i != decknum) { 
                    engine.setValue("[Channel" + i + "]", "pfl", false);
                }
            }
        }
    }
};

NumarkMixtrack3.OnPFLStatusChange = function(value, group, control) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    deck.LEDs.headphones.onOff((value) ? ON : OFF);
};

// Callback for the PFL Button
NumarkMixtrack3.OnShiftedPFLButton = function(channel, control, value, status,
    group, eventkind) {
    if (eventkind === DOUBLE_PRESS) {
        // Double press : toggle slip mode
        toggleValue(group, "slip_enabled");
    } else {
        // Single press : toggle quantize mode
        toggleValue(group, "quantize");
    }
};

NumarkMixtrack3.PitchBendMinusButton = function(channel, control, value,
    status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (value === DOWN) {
        if (deck.shiftKey) {
            engine.setValue("[Channel" + decknum + "]", "beatjump_1_backward", true);
        } else {
            engine.setValue("[Channel" + decknum + "]", "rate_temp_down", true);
        }
    } else if (!deck.shiftKey) {
        engine.setValue("[Channel" + decknum + "]", "rate_temp_down", false);
    }
};

NumarkMixtrack3.PitchBendPlusButton = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (value === DOWN) {

        if (deck.shiftKey) {
            engine.setValue("[Channel" + decknum + "]", "beatjump_1_forward", true);
        } else {
            engine.setValue("[Channel" + decknum + "]", "rate_temp_up", true);
        }
    } else if (!deck.shiftKey) {
        engine.setValue("[Channel" + decknum + "]", "rate_temp_up", false);
    }
};

NumarkMixtrack3.BeatKnob = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var gainIncrement;
    var gainValue = [];
    var i;
    var knobValue;

    if (!deck.PADMode) {

        // Default mode, direct interaction with knob, without any button combination
        for (i = 1; i <= 4; i++) {

            gainValue[i - 1] = engine.getParameter(
                "[EffectRack1_EffectUnit" + [i] + "]", "mix");

            // increment value between 0 and 1
            gainIncrement = 1 / 20; // 20 increments in one full knob turn   

            // beat knobs sends 1 or 127 as value. If value = 127, turn is counterclockwise, we reduce gain
            if (value === 127) {
                gainIncrement = -gainIncrement;
            }

            gainValue[i - 1] = gainValue[i - 1] + gainIncrement;

            if ((gainValue[i - 1] + gainIncrement) < 0) {
                gainValue[i - 1] = 0;
            }
            if ((gainValue[i - 1] + gainIncrement) > 1) {
                gainValue[i - 1] = 1;
            }

        }

        // we adjust pregain with adjusted value
        for (i = 1; i <= 5; i++) {
            if (gainValue[i - 1] >= 0 && gainValue[i - 1] <= 4) {
                if (deck.TapDown) {
                    engine.setParameter("[EffectRack1_EffectUnit" + i +
                        "_Effect1]", "parameter2", gainValue[i - 1]);
                } else {
                    if (FXmode === 1){
                        if (decknum === i) {
                            engine.setParameter("[EffectRack1_EffectUnit" + i +
                                "]", "mix", gainValue[i - 1]);
                        }
                    } else { //FX Mode = 1 FX 3 Units
                            engine.setParameter("[EffectRack1_EffectUnit" + i +
                                "]", "mix", gainValue[i - 1]);
                    }
                }
            }
        }

    }

    if (deck.shiftKey) {
        if (value - 64 > 0) {
            knobValue = value - 128;
        } else {
            knobValue = value;
        }

        if (knobValue < 0) {
            engine.setValue("[Channel" + decknum + "]", "beats_translate_earlier", true);
        } else {
            engine.setValue("[Channel" + decknum + "]", "beats_translate_later", true);
        }
    }

    if (deck.PADMode) {
        // PadMode button is used, Shift key is not used, we adjust Sampler volumes
        // Define new value of sampler pregain
        // Off = 1, centered = 1, max = 4

        var startingSampler;
        if (decknum === 1 || decknum === 3) {
            startingSampler = 1;
        } else {
            startingSampler = 5;
        }

        for (i = startingSampler; i <= startingSampler + 3; i++) {

            gainValue[i - 1] = engine.getValue("[Sampler" + [i] + "]",
                "pregain");

            if (gainValue[i - 1] <= 1) {
                // increment value between 0 and 1
                gainIncrement = 1 / 20; // 20 increments in one full knob turn   
            } else {
                // increment value between 1 and 4
                gainIncrement = 3 / 20; // 20 increments in one full knob turn
            }

            // beat knobs sends 1 or 127 as value. If value = 127, turn is counterclockwise, we reduce gain
            if (value === 127) {
                gainIncrement = -gainIncrement;
            }

            gainValue[i - 1] = gainValue[i - 1] + gainIncrement;

            if ((gainValue[i - 1] + gainIncrement) < 0) {
                gainValue[i - 1] = 0;
            }

            if ((gainValue[i - 1] + gainIncrement) > 4) {
                gainValue[i - 1] = 4;
            }

            if (gainValue[i - 1] >= 0 && gainValue[i - 1] <= 4) {
                engine.setValue("[Sampler" + i + "]", "pregain",
                    gainValue[i - 1]);
            }
        }
    }
};

NumarkMixtrack3.bpmTap = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    deck.TapDown = false;

    if (value === DOWN) {

        deck.TapDown = true; //to use TAP button as a "shift" key (e.g. InstantFX)

        if (deck.shiftKey) { // Toggle decks
            if (decknum <= 2) {
                decknum += 2;
            } else {
                decknum -= 2;
            }
            //determine the deck that we want to switch to:
            NumarkMixtrack3.deckGroup[group] = '[Channel' + decknum + ']';
            NumarkMixtrack3.initDeck(NumarkMixtrack3.deckGroup[group], true);
        } else { //Either use the button as a modifier, or as TAP button
            bpm.tapButton(decknum);
            engine.setValue("[Channel" + decknum + "]", "bpm_tap", true);
        }
    }
    engine.setValue("[Channel" + decknum + "]", "bpm_tap", false);
};

NumarkMixtrack3.EQKnob = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var EQp, FXp;

    switch (control) {
        case 1: // Treble
            EQp = 3;
            FXp = 1;
            break;
        case 2: // Mid
            EQp = 2;
            FXp = 2;
            break;
        case 3: // Bass
            EQp = 1;
            FXp = 3;
            break;
    }

    if (!deck.shiftKey && !deck.PADMode && !deck.TapDown) {
        parameterSoftTakeOver("[EqualizerRack1_[Channel" + decknum +
            "]_Effect1]", "parameter" + EQp, value);
    }

    if (FXmode === 1) {
        if (deck.shiftKey) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit" + decknum +
                "_Effect1]", "parameter" + FXp, value);
        }
        if (deck.PADMode) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit" + decknum +
                "_Effect2]", "parameter" + FXp, value);
        }
        if (deck.TapDown) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit" + decknum +
                "_Effect3]", "parameter" + FXp, value);
        }
    } else {

        if (deck.shiftKey) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit1_Effect1]", "parameter" + FXp, value);
        }
        if (deck.PADMode) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit2_Effect1]", "parameter" + FXp, value);
        }
        if (deck.TapDown) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit3_Effect1]", "parameter" + FXp, value);
        }
    }
};

NumarkMixtrack3.FilterKnob = function(channel, control, value, status, group) {
    var decknum = group.substring(26, 27);
    decknum = NumarkMixtrack3.deckFromGroup("[Channel" + decknum + "]");
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (!deck.shiftKey && !deck.PADMode && !deck.TapDown) {
        parameterSoftTakeOver("[QuickEffectRack1_[Channel" + decknum + "]]", "super1", value);
    }

    if (FXmode === 1) {
        if (deck.shiftKey) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit" + decknum +
                "_Effect1]", "parameter4", value);
        }
        if (deck.PADMode) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit" + decknum +
                "_Effect2]", "parameter4", value);
        }
        if (deck.TapDown) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit" + decknum +
                "_Effect3]", "parameter4", value);
        }
    } else {
        if (deck.shiftKey) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit1_Effect1]", "parameter4", value);
        }
        if (deck.PADMode) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit2_Effect1]", "parameter4", value);
        }
        if (deck.TapDown) {
            parameterSoftTakeOver("[EffectRack1_EffectUnit3_Effect1]", "parameter4", value);
        }
    }
};

NumarkMixtrack3.loop_in = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        engine.setValue("[Channel" + decknum + "]", "loop_in", value ? 1 : 0);
    }
};

NumarkMixtrack3.loop_out = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        engine.setValue("[Channel" + decknum + "]", "loop_out", value ? 1 : 0);
    }
};

NumarkMixtrack3.reloop_exit = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);

    if (value === DOWN) {
        engine.setValue("[Channel" + decknum + "]", "reloop_exit", value ? 1 : 0);
    }
};

NumarkMixtrack3.LoopHalveButton = function(channel, control, value, status,
    group) {
    if (value === DOWN) {
        var decknum = NumarkMixtrack3.deckFromGroup(group);
        var deck = NumarkMixtrack3.decks["D" + decknum];

        if (deck.shiftKey) {
            engine.setValue("[Channel" + decknum + "]", "loop_double", true);
        } else {
            engine.setValue("[Channel" + decknum + "]", "loop_halve", true);
        }
    }
};

NumarkMixtrack3.OnLoopInOutChange = function(value, group, control) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var valIn = engine.getValue("[Channel" + decknum + "]", "loop_start_position");
    var valOut = engine.getValue("[Channel" + decknum + "]", "loop_end_position");
    var valEnabled = engine.getValue("[Channel" + decknum + "]", "loop_enabled");

    if (valIn == -1) {
        if (deck.LEDs.loopin.getFlashDuration() !== 300) {
        deck.LEDs.loopin.flashOn(300, PADcolors.blue, 300);
        } 
        deck.LEDs.loopout.onOff(OFF);
        deck.LEDs.reloop_exit.onOff(OFF);
        deck.LEDs.loop_halve.onOff(OFF);

    } else if (valOut == -1 && deck.loaded) {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        if (deck.LEDs.loopout.getFlashDuration() !== 300) {
            deck.LEDs.loopout.flashOn(300, PADcolors.blue, 300);
        }
        deck.LEDs.reloop_exit.onOff(OFF);
        deck.LEDs.loop_halve.onOff(OFF);

    } else if (!valEnabled) {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        deck.LEDs.loopout.onOff(PADcolors.blue);
        if (deck.LEDs.reloop_exit.getFlashDuration() !== 300) {
            deck.LEDs.reloop_exit.flashOn(300, PADcolors.blue, 300);
        }
        deck.LEDs.loop_halve.onOff(PADcolors.blue);

    } else {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        deck.LEDs.loopout.onOff(PADcolors.blue);
        deck.LEDs.reloop_exit.onOff(PADcolors.blue);
        deck.LEDs.loop_halve.onOff(PADcolors.blue);
    }
};

NumarkMixtrack3.volume = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);

    engine.setValue("[Channel" + decknum + "]", "volume", value / 127);
};

NumarkMixtrack3.OnVolumeChange = function(value, group, control) {
    var decknum = parseInt(group.substring(8, 9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
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
    var decknum = parseInt(group.substring(8, 9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.LEDs.meter.onOff(120 * value);
};

NumarkMixtrack3.OnPlaypositionChange = function(value, group, control) {

    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (deck.loaded) {
        var timeremaining = RealDuration(group) * (1 - value);
        var trackwarning = 0;
        if (timeremaining <= 30) {
            trackwarning = 1;
        }
        if (timeremaining <= 10) {
            trackwarning = 2;
        }
        if (timeremaining <= 0) {
            trackwarning = 3;
        }
        if (!TrackEndWarning) {
            trackwarning = 0;
        }

        switch (trackwarning) {
            case 0:
                deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ?
                    ON : OFF);
                break;

            case 1: // if less than 30 seconds before end of track : flashing slowly
                if (deck.LEDs.jogWheelsInScratchMode.getFlashDuration() !==
                    1000) {
                    deck.LEDs.jogWheelsInScratchMode.flashOn(1000, ON, 1000);
                }
                break;

            case 2: // if less than 10 seconds before end of track : flashing fast
                if (deck.LEDs.jogWheelsInScratchMode.getFlashDuration() !== 300) {
                    deck.LEDs.jogWheelsInScratchMode.flashOn(300, ON, 300);
                }
                break;

            case 3: // end of strack : full ring lit
                deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ?
                    ON : OFF);
                break;
            default:
                break;
        }
    } else {
        deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ?
            ON : OFF);
    }
};

NumarkMixtrack3.OnTrackLoaded = function(value, group, control) {
    var decknum = parseInt(group.substring(8, 9));
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (value !== 0) {
        if (!deck.faderstart) {
            // Light up the PFL light indicating that a track is loaded
            deck.LEDs["headphones"].onOff(ON);
        } else {
            // Flash up the PFL light button indicating that a track is loaded with fader start
            deck.LEDs["headphones"].flashOn(300, ON, 300);
        }
    } else {
        // Switch off the PFL light indicating that a track is ejected
        deck.LEDs["headphones"].onOff(OFF);
    }

    var oldloaded = deck.loaded;
    deck.loaded = (value !== 0);
    if (oldloaded !== deck.loaded) { // if this value changed we update the jog light
        engine.trigger(group, "playposition");
    }
};

NumarkMixtrack3.OnPlayIndicatorChange = function(value, group, control) {
    var decknum = script.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    deck.LEDs.play.onOff((value) ? ON : OFF);
};
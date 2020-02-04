/***********************************************************************
 * ==============             User Options             =================
 *******************
 * TrackEndWarning *
 *******************
 * By default, when you reach the end of the track, the jog wheel are flashing.
 * set this variable just below to "false" instead of "true"
 * in order to disable this behaviour by default.
 * This can be toggled from the controller also (see the Wiki)
 * (idea by be.ing, member of the Mixxx team)
 **************************/
var TrackEndWarning = true;
/****************
 *  scriptpause *
 ****************
 * period (in ms) while the script will be paused when sending messages
 * to the controller in order to avoid too much data flow at once in the same time.
 *  - default value : 5 ms
 *  - To disable : 0;
 **************************/
var scriptpause = 5;

/****************************
 * Constants for scratching *
 ****************************
 * Beatpad jog wheel is 800 intervals per revolution.
 * but the value has to be multiplied by 2 because
 * the scratch takes into account the track_samples in background
 * which is multiplied by 2 when the track is stereo
 ***************************/
var intervalsPerRev = 1600,
    rpm = 33 + 1 / 3,  //Like a real vinyl !!! :)
    alpha = 1.0 / 8,   //Adjust to suit.
    beta = alpha / 32; //Adjust to suit.

/*****************************
 * Constants for Jog Bending *
 *****************************
 * benConst is the acceleration parameter
 ************************ ***/
var bendConst = 1/4; // Adjust to suit.


/************************  GPL v2 licence  *****************************
 * Reloop Beatpad controller script
 * Author: Chloé AVRILLON (DJ Chloé)
 *
 * Key features
 * ------------
 * - Light and Jog wheel light handling
 *-  shift+wheelturn in "Jog Scratch" mode do automatic cut of the fader while scratching
 * - press/double press/long press handling
 **********************************************************************
 * User References
 * ---------------
 * Wiki/manual : http://www.mixxx.org/wiki/doku.php/reloop_beatpad
 * support forum : http://www.mixxx.org/forums/viewtopic.php?f=7&amp;t=7581
 * e-mail : chloe.avrillon@gmail.com
 *
 * Thanks
 * ----------------
 * Thanks to authors of other scripts and particularly to authors of
 * Numark Dj2Go, KANE QuNeo, Vestax-VCI-400
 *
 * Revision history
 * ----------------
 * 2015-10-22 - Initial revision for Mixxx 1.12.0
 * 2015-10-22 - GPL v2 licence, rework of this header, JSHint.com quality check,
 *              a few comments, minor changes, typos
 * 2015-11-24 - Make some code reusable (lights : LED object; Special Buttons, iCUT)
 *            - pfl hidden bug fixed
 *            - removed line duplicate in JumpBtn : deck.controls.jump.onOff(true);
 *              (Mixxx bug was fixed : https://bugs.launchpad.net/mixxx/+bug/1504503)
 *            - More comment in code
 *            - Moved scratching constants with the global constants
 *            - Fixed Jog Bending and fast search
 *            - Sysex identification of the controller (nice print out in midiDebug).
 *
 * 2016-01-12 - Fixed FX effect selection on deck 2 (was selecting the entire chain instead (SHIFT+FX Select)
 *            - Fixed SHIFT+PFL on the right deck
 * 2016-01-13 - removed a few unused variables, a useless return statement, corrected typos,
 *            - modified "Jogger" object
 *                  --> Autocut feature (made it more "reusable")
 *                  --> model "A" and model "B" controller parameter
 *            - move scratching and jog bending constant in user parameters section
 *            - comments formatting
 *
 * This is a neverending story...
 ***********************************************************************
 *                           GPL v2 licence
 *                           --------------
 * Reloop Beatpad controller script script 1.3 for Mixxx 2.0+
 * Copyright (C) 2015-2016 Chloé AVRILLON
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

function ReloopBeatpad() {}

ReloopBeatpad(); // Very important ! Initializes rusable objects.

// Array of Objects can be created
ReloopBeatpad.decks = [];
ReloopBeatpad.playlist = [];
ReloopBeatpad.master = [];

// Global constants/variables
var ON = 0x7F,
    OFF = 0x00,
    DOWN = 0x7F,
    // UP = 0x00, // unused in the script, may be one day...
    SHIFT = -0x40,
    LBtn = 0x90,
    RBtn = 0x91,
    MBtn = 0x94,
    // Performance modes
    CUEMODE = 0,
    LOOPMODE = 1,
    FXMODE = 2,
    // 4 "Sampler" modes
    SAMPLERMODE = 3,
    SAMPLERBANKSTATUSMODE = 4,
    LOOPMODESTATUS = 5,
    FXRACKSELECTMODE = 6,
    // loop kind : simple normal loop mode/Loop roll mode
    SIMPLE = 1,
    ROLL = 2,
    HardwareLight = false,
    // Constant for special handling of some buttons
    QUICK_PRESS = 1, DOUBLE_PRESS = 2, LONG_PRESS = 3,

    // Sysex messages :
    // This Sysex message permits to ask the Reloop Beatpad for a complete
    // status of it's buttons, knobs and faders.
    // It is not used in the script anymore, Mixxx does it automatically.
    // just after the call to the init() function.
    // I leave it as a reference, commented.
    // ControllerStatusSysex = [0xF0, 0x26, 0x2D, 0x65, 0x22, 0xF7],

    // This Sysex message asks the controller (it's very generic and works
    // for a lot of MIDI controllers). Sending this to the controller will
    // be followed by a Sysex sent by the controllers giving some
    // values that permits to identify it (have a look far at the end
    // of this mapping script
    SysexIDRequest = [0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7];

// Utilities
//======================================================================
function pauseScript(ms) {
    if (ms>0) {
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

function intpart(n) {
    if (n<0) {
        return Math.ceil(n);
    } else {
        return Math.floor(n);
    }
}

function toggleValue(group,key) {
    engine.setValue(group,key,!engine.getValue(group,key));
}

function IsStereo(group) {
    // this is an integer
    var d1 = engine.getValue(group, "duration");
    // this is a real value
    var d2 = engine.getValue(group, "track_samples") / engine.getValue(group, "track_samplerate");
    if (d1==d2) {
        return false;
    } else {
        if  ( (d1 < Math.floor(d2)) && (d1 < Math.ceil(d2)) ) {
            return false;
        } else {
            return true;
        }
    }
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
        // this is a real value :
        var d2 = engine.getValue(group, "track_samples") / engine.getValue(group, "track_samplerate");

        if (d1==d2) {
            // it is mono
            return d2;
        } else {
            if  ( (d1 > Math.floor(d2)) && (d1 < Math.ceil(d2)) ) {
                // It is mono
                return d2;
            } else {
                // It is stereo
                return d2/2;
            }
        }
    }
}

function TurnLEDsOff() {
    // Turn all LEDS off
    var i,j;
    for (i = LBtn; i <= RBtn; i++) { // 2 decks
        midi.sendShortMsg(i, 0x41, OFF);
        midi.sendShortMsg(i, 0x42, OFF);
        midi.sendShortMsg(i, 0x44, OFF);
        for (j = 0x47; j <= 0x62; j++) {
            midi.sendShortMsg(i, j, OFF);
            midi.sendShortMsg(i, j + SHIFT, OFF);
        }
        for (j = 0x66; j <= 0x6C; j++) {
            midi.sendShortMsg(i, j, OFF);
        }
    }
}

// Constants
// ======================================================================
ReloopBeatpad.MIDI = {
    rec: 0x41,
    Trackpush: 0x46,
    BendMinus: 0x41,
    BendPlus: 0x42,
    // Jog modes
    Vinyl: 0x45,
    iScratch: 0x46,
    // Pitch+FX switches
    Loop: 0x44,
    Loop_size_push: 0x43,
    FX_ON: 0x47,
    // Performance mode
    Cue: 0x48,
    Loop_Bounce: 0x49,
    InstantFX: 0x4A,
    Sampler: 0x4B,
    CuePad: 0x4C,      // (4 Buttons 0x4C~0x4F (+0--+4)
    LoopPad: 0x50,     // (4 Buttons 0x50~0x53 (+0--+4)
    FXPad: 0x54,       // (4 Buttons 0x54~0x57 (+0--+4)
    SamplerPad: 0x58,  // (4 Sampler Buttons 0x58~0x5B (+0--+4)  (RGB color leds)
    Shift: 0x5C,
    mSync: 0x5D,
    Set: 0x5E,         // "Cue Play" in the Beatpad documentation
    Jump: 0x5F,        // "MainCue" in the Beatpad documentation
    Play: 0x60,
    pfl: 0x61,
    Load: 0x62,
    VUMeter: 0x66,     // values : 0x00~0x08
    // Vinyl RIM Leds
    RIM_Red: 0x67,    // 1st behaviour 0x01-0x18 ; 2nd Behavior = 1st +24 ;3d behavior ON/OFF
    RIM_Blue: 0x68,   // 1st behaviour 0x05-0x08 ; 2nd Behavior = 1st -4;3d behavior ON/OFF
    RIM_RGB: 0x69     // 4 RGB Leds 0x69~0x6C (+0--+4) : for values, see ReloopBeatpad.RGB just below
};

// Colors used by the jogwheels
ReloopBeatpad.RGB = {
    black: 0x00,
    red: 0x01,
    green: 0x02,
    blue: 0x03,
    yellow: 0x04,
    magenta: 0x05,
    cyan: 0x06,
    white: 0x07
};

// Colors used by the PADs (in Sampler mode)
ReloopBeatpad.PadColor = {
    black: 0x00,
    blue: 0x01,
    magenta: 0x02,
    uv: 0x03,
    purple: 0x04,
    indigo: 0x05,
    fushia: 0x06,
    lilac: 0x07,
    orange: 0x08
};

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
    midi.sendShortMsg(this.control, this.midino, value);
    pauseScript(scriptpause);
    this.lit = value;
};

// public : make a light flashing
//-------------------------------
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
LED.prototype.flashOn = function(num_ms_on, value, num_ms_off, flashCount, relight, valueoff) {
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

        this.flashTimer = engine.beginTimer( num_ms_on + num_ms_off, function(){ myself.flashOnceOn(false); } );
    }
    if (flashCount > 1) {
        // flashcount>0 , means temporary flash, first flash already done,
        // so we don't need this part  if flashcount=1
        // temporary timer. The end of this timer stops the permanent flashing

        this.flashTimer2 = engine.beginTimer(flashCount * (num_ms_on + num_ms_off) - num_ms_off, function(){ myself.Stopflash(relight); }, true);
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
    midi.sendShortMsg(this.control, this.midino, this.valueon);
    pauseScript(scriptpause);
    this.flashOnceDuration = this.num_ms_on;
    this.flashOnceTimer = engine.beginTimer(this.num_ms_on - scriptpause, function(){ myself.flashOnceOff(relight); }, true);
};

// private :call back function (called in flashOnceOn() )
LED.prototype.flashOnceOff = function(relight) {
    this.flashOnceTimer = 0;
    this.flashOnceDuration = 0;

    if (relight) {
        midi.sendShortMsg(this.control, this.midino, this.lit);
        pauseScript(scriptpause);
    } else {
        midi.sendShortMsg(this.control, this.midino, this.valueoff);
        pauseScript(scriptpause);
        this.lit = OFF;
    }
};

// ********* special buttons handlers (SHIFT ,LOAD and SYNC buttons)
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
//                      the button or value=UP because you have released
//                      the button only once before it becomes a long press).
// DoublePressTimeOut : delay in ms above which a second press on the
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
        // Sets a default value of 400 ms
        this.DoublePressTimeOut = 400;
    }
    this.ButtonCount = 0;
    this.ButtonTimer = 0;
};

// Button pressed
SingleDoubleBtn.prototype.ButtonDown = function(channel, control, value, status, group) {
    var myself = this;
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;
    if (this.ButtonTimer === 0) { // first press

        this.ButtonTimer =
            engine.beginTimer(this.DoublePressTimeOut,
                              function(){ myself.ButtonDecide(); }, true);
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
    this.Callback(this.channel, this.control, this.value, this.status, this.group, this.ButtonCount);
    this.ButtonCount = 0;
};

// =======================  LongShortBtn
// Callback           : Callback function you have to provide (see end of the code), that will return
//                      the original event parameters (channel, control, value, status, group)
//                      and the kind of press event affecting your button (eventkind)
//                      This callback will be called once you release the button
//                      (Value will be equal to UP). You must provide this parameter.
// LongPressThreshold : delay in ms above which a first press on the
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
        // Sets a default value of 500 ms
        this.LongPressThreshold = 500;
    }

    this.ButtonLongPress = false;
    this.ButtonLongPressTimer = 0;
};

// Timer's call back for long press
LongShortBtn.prototype.ButtonAssertLongPress = function() {
    this.ButtonLongPress = true;
    // the timer was stopped, we set it to zero
    this.ButtonLongPressTimer = 0;
    // let's take action of the long press
    // Make sure the callback is a function​ and exist
    if (typeof callback === "function") {
        // Call it, since we have confirmed it is callable​
        this.CallBackOKLongPress(this.channel, this.control, this.value, this.status, this.group, LONG_PRESS);
    }
};

LongShortBtn.prototype.ButtonDown = function(channel, control, value, status, group) {
    var myself = this;
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;
    this.ButtonLongPress = false;
    this.ButtonLongPressTimer = engine.beginTimer(this.LongPressThreshold, function(){ myself.ButtonAssertLongPress(); }, true);
};

LongShortBtn.prototype.ButtonUp = function() {
    if (this.ButtonLongPressTimer !== 0) {
        engine.stopTimer(this.ButtonLongPressTimer);
        this.ButtonLongPressTimer = 0;
    }
    if (this.ButtonLongPress) {
        this.Callback(this.channel, this.control, this.value, this.status, this.group, LONG_PRESS);
    } else {
        this.Callback(this.channel, this.control, this.value, this.status, this.group, QUICK_PRESS);
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
//                      the button or value=UP because you have released
//                      the button only once before it becomes a long press).
// LongPressThreshold : delay in ms above which a first press on the
//                      button will be considered as a Long press (default = 500ms).
// DoublePressTimeOut : delay in ms above which a second press on the
//                      button will not be considered as a potential double
//                      but as a new press cycle event (default = 400ms).
var LongShortDoubleBtn = function(Callback, LongPressThreshold, DoublePressTimeOut) {
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
        // take action
        this.ButtonDecide();
    }
};

// Button pressed (function called by mapper's code)
LongShortDoubleBtn.prototype.ButtonDown = function(channel, control, value, status, group) {
    var myself = this;
    this.channel = channel;
    this.control = control;
    this.value = value;
    this.status = status;
    this.group = group;

    if (this.ButtonCount === 0) { // first press (inits)
        // 1st press
        this.ButtonCount = 1;
        // and short press
        this.ButtonLongPress = false;
        this.ButtonLongPressTimer =
            engine.beginTimer(this.LongPressThreshold,
                              function(){ myself.ButtonAssertLongPress(); },
                              true);
        this.ButtonTimer =
            engine.beginTimer(this.DoublePressTimeOut,
                              function(){ myself.ButtonAssert1Press(); },
                              true);
    } else if (this.ButtonCount == 1) { // 2nd press (before short timer's out)
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
            this.ButtonTimer = 0 ;
        }

        // 2nd press
        this.ButtonCount = 2;

        // ...and take action immediately
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
    } //else :
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
        this.Callback(this.channel, this.control, this.value, this.status, this.group, LONG_PRESS);
    } else {
        if (this.ButtonCount == 2) {
            this.Callback(this.channel, this.control, this.value, this.status, this.group, DOUBLE_PRESS);
        } else { // We pressed sync only once
            this.Callback(this.channel, this.control, this.value, this.status, this.group, QUICK_PRESS);
        }
    }
    // re-init
    this.ButtonCount = 0;
    this.ButtonLongPress = false;
};


// *************************************************
// iCut mode management
// ****
// According to the Quick start guide :
//      iCut MODE :
//      DJAY will automatically cut your track with the
//      cross fader when holding SHIFT and scratching
//      with the jog wheel
// According to Reloop Website : http://www.reloop.com/reloop-beatpad (Explorer tab)
//      iCut :
//      "this mode simulates a scratch routine. When the jog wheel is turned back
//      the crossfader closes, when the jog wheel is turned forward the crossfader
//      will open."
// In Practice : DJAY software is closing/opening the crossfader
//      quickly without taking into account the direction of the wheel.
//      Here I am trying to stick with the reloop explanation :
//      it is the "as it is supposed to be done"
var AutoCut = function (deckNum) {
    this.deckNum = deckNum;
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

AutoCut.prototype.FaderCut = function(jogValue) {
    if (this.enabled) {
        var direction = Math.sign(jogValue); //Get Jog wheel direction
        // Backward=-1 (close), forward =0 (open)
        if (direction > 0) {
            direction = 0;
        }
        //  Left Deck ? direction = 0 (open : crossfader to zéro) or 1 (close : crossfader to the right)
        // Right Deck ? direction = 0 (open : crossfader to zéro) or -1 (close : crossfader to the left)
        if (this.deckNum == 1) {
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
// ******
// Thank you to the authors of the Vestax VCI 400 mapping script
// model : model of your controller, should be "A" or "B".
// Your controller is a "Model A" controller for scratching,
// if it centers on 0.
// Your controller is a "Model B" controller for scratching,
// if it centers on 0x40 (64)
// See http://www.mixxx.org/wiki/doku.php/midi_scripting#scratching
var Jogger = function (group, deckNum, model) {
    this.deckNum = deckNum;
    this.group = group;
    this.wheelTouchInertiaTimer = 0;
    this.iCUT = new AutoCut(deckNum);
    this.model = model;
};

Jogger.prototype.finishWheelTouch = function() {
    var myself = this;
    this.wheelTouchInertiaTimer = 0;
    var play = engine.getValue(this.group, "play");
    if (play !== 0) {
        // If we are playing, just hand off to the engine.
        this.iCUT.Off();
        engine.scratchDisable(this.deckNum, true);
    } else {
        // If things are paused, there will be a non-smooth handoff
        // between scratching and jogging.
        // Instead, keep scratch on until the platter is not moving.
        var scratchRate = Math.abs(engine.getValue(this.group, "scratch2"));
        if (scratchRate < 0.01) {
            // The platter is basically stopped, now we can disable
            // scratch and hand off to jogging.
            this.iCUT.Off();
            engine.scratchDisable(this.deckNum, false);
        } else {
            // Check again soon.
            this.wheelTouchInertiaTimer =
                engine.beginTimer(100,
                                function(){ myself.finishWheelTouch(); },
                                true);
        }
    }
};

Jogger.prototype.onWheelTouch = function(value,Do_iCut) {
    var myself = this;
    if (Do_iCut) {
        this.iCUT.On();
    } else {
        this.iCUT.Off();
    }

    if (this.wheelTouchInertiaTimer !== 0) {
        // The wheel was touched again, reset the timer.
        engine.stopTimer(this.wheelTouchInertiaTimer);
        this.wheelTouchInertiaTimer = 0;
    }

    if (value == DOWN) {
        // Hand on the Jog wheel, scratch activated
        engine.scratchEnable(this.deckNum, intervalsPerRev, rpm, alpha, beta);
    } else {
        // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
        // Depending on how fast the platter was moving, lengthen the time we'll wait.
        var scratchRate = Math.abs(engine.getValue(this.group, "scratch2"));
        var inertiaTime = Math.pow(1.8, scratchRate) * 50;
        if (inertiaTime < 100) {
            // Just do it now.
            this.finishWheelTouch();
        } else { // If button up
            this.wheelTouchInertiaTimer =
                engine.beginTimer(inertiaTime,
                                function(){ myself.finishWheelTouch(); },
                                true);
        }
    }
};

Jogger.prototype.onWheelMove = function(value, Do_iCut) {
    var jogValue;
    if (this.model=="A") {
        if (value-64 > 0) jogValue = value - 128;
        else jogValue = value;
    } else { // Model B controller
      jogValue = value - 0x40;
    }

    // Note that we always set the jog value even if scratching is active.  This seems
    // to create a better handoff between scratching and not-scratching.
    if (engine.getValue(this.group, "play")) {
        engine.setValue(this.group, "jog", jogValue / (bendConst*10));
    } else {
        engine.setValue(this.group, "jog", jogValue / (bendConst*2.5));
    }
    if (engine.getValue(this.group, "scratch2_enable")) {
        // if "control"<0x40, then the DJ is doing a SHIFT+JogWheel (iCut with Dejay from Algorriddim)
        // if "Jog Scratch" mode is activated on the Beatpad, we have for the "control" value
        // Left JogWheel  --> 0x63, +SHIFT --> 0x23
        // Right JogWheel --> 0x65, +SHIFT --> 0x25
        if (Do_iCut) {
            this.iCUT.On();
        } else {
            this.iCUT.Off();
        }
        this.iCUT.FaderCut(jogValue);
        // Register the movement :
        engine.scratchTick(this.deckNum, jogValue);
    }
};

// =====================================================================
// Specific objects for the Reloop Beatpad
// =====================================================================

// ******************************************************************
// RGB Jog Leds management
// *********
// Some explanation about this part which control the lights (colors, flashing effect, and so forth)
// of the jog wheels. It is based on the generic LED object code above.
// It is a layered structure like layers in paint programs.
// The lowest level (0) is the background and the highest is the foreground (show 6)
// Each of the layer permits to act like a mask for the lower ones.
// Each show corresponds to a light show to indicate whether you apply an effect, the lp/hp filter , loops, etc
// The jogwheel has 4 RGB leds , so there is 4 slots for each show.
// let's that we have toggle an Effect of the effect rack (show 1), the jog wheel
// will illuminate magenta, yellow, cyan or green depending of the effect.
// If above that we turn the lp/hp filter button to the right (High passs),
// the right part will illuminate white (show 2), leaving the left part with the color of the effect.
// The left part of (slots 1 and 2), set to "null", will be considered as being transparent for the layer show n°2,
// leaving the corresponding slots below (show1) in order to be displayed.
// Shows 5 and 6 are reserved for temporary blinking : blinking off can be transparent or full black
// This technique prevents throwing too many messages to the controller to lit on or off the leds.
ReloopBeatpad.rgbLEDs = function(control, deckID) {
    this.control = control;
    this.midino = 0x69;
    this.deckID = deckID;
    this.layers = {
        "show0": {
            activated: true,
            colors: [OFF, OFF, OFF, OFF]
        }, //OFFF state
        "show1": {
            activated: false,
            colors: [null, null, null, null]
        },
        "show2": {
            activated: false,
            colors: [null, null, null, null]
        },
        "show3": {
            activated: false,
            colors: [null, null, null, null]
        },
        "show4": {
            activated: false,
            colors: [null, null, null, null]
        },
        "show5": {
            activated: true,
            colors: [null, null, null, null]
        }, // reserved for blinking  (blink Off values)
        "show6": {
            activated: false,
            colors: [null, null, null, null]
        } // reserved for blinking (blink On values)
    };
    this.lit = [OFF, OFF, OFF, OFF];
    this.flashTimer = 0;
    this.flashTimer2 = 0;
    this.flashOnceTimer = 0;
    this.flashDuration = 0;
};

ReloopBeatpad.rgbLEDs.prototype.setshow = function(showname, color1, color2, color3, color4) {
    if (showname !== "show0") {
        if (color1 === undefined) {
            this.layers[showname].colors[0] = null;
        } else {
            this.layers[showname].colors[0] = color1;
        }
        if (color2 === undefined) {
            this.layers[showname].colors[1] = this.layers[showname].colors[0];
            this.layers[showname].colors[2] = this.layers[showname].colors[0];
            this.layers[showname].colors[3] = this.layers[showname].colors[0];
        } else {
            this.layers[showname].colors[1] = color2;
            this.layers[showname].colors[2] = color3;
            this.layers[showname].colors[3] = color4;
        }
    }
};

ReloopBeatpad.rgbLEDs.prototype.updatecontroller = function() {
    // null="transparent"
    var tosend = [null, null, null, null];
    var showname, i, j, k;
    var pausecount = 0;
    for (k = 0; k <= 3; k++) {
        for (j = 6; tosend[k] === null; j--) {
            showname = "show" + j;
            if (this.layers[showname].activated) {
                tosend[k] = this.layers[showname].colors[k];
            }
        }
    }
    for (i = 0; i <= 3; i++) {
        if (tosend[i] !== this.lit[i]) {
            midi.sendShortMsg(this.control, this.midino + i, tosend[i]);
            pauseScript(scriptpause);
            pausecount++;
        }
    }
    this.lit = tosend.slice(0);
    return pausecount;
};

ReloopBeatpad.rgbLEDs.prototype.activateshow = function(showname, activate) {
    var pausecount = 0;
    this.layers[showname].activated = activate;
    if (showname !== "show5") {
        pausecount = this.updatecontroller();
    }
    return pausecount;
};

// public : light on/off
ReloopBeatpad.rgbLEDs.prototype.onOff = function(showname, value) {
    if (this.flashTimer !== 0) {
        engine.stopTimer(this.flashTimer);
        this.flashTimer = 0;
        this.flashDuration = 0;
        this.layers.show5.activate = false;
        this.layers.show6.activate = false;
    }

    if (this.flashTimer2 !== 0) {
        engine.stopTimer(this.flashTimer2);
        this.flashTimer2 = 0;
        this.flashDuration = 0;
        this.layers.show5.activate = false;
        this.layers.show6.activate = false;
    }

    if (this.flashOnceTimer !== 0) {
        engine.stopTimer(this.flashOnceTimer);
        this.flashOnceTimer = 0;
        this.flashOnceDuration = 0;
        this.layers.show5.activate = false;
        this.layers.show6.activate = false;
    }

    this.activateshow(showname, (value) ? true : false);
};

// public : make a rgb light flashing
ReloopBeatpad.rgbLEDs.prototype.flashOn = function(num_ms_on, RGBColor, num_ms_off, flashCount) {
    var myself = this;
    this.setshow("show6", RGBColor);

    // stop pending timers
    this.flashOff();

    // inits
    this.layers.show5.activate = true;
    this.flashDuration = num_ms_on;

    // 1st flash
    // This is because the permanent timer below takes num_ms_on milisecs before first flash.
    this.flashOnceOn(true);

    if (flashCount !== 1) {
        // flashcount =0 means permanent flash,
        // flashcount>0 , means temporary flash, first flash already done,
        // so we don't need this part  if flashcount=1
        // permanent timer
        this.flashTimer = engine.beginTimer( num_ms_on + num_ms_off,
                                    function(){ myself.flashOnceOn(true); } );
    }
    if (flashCount > 1) {
        // flashcount>0 , means temporary flash, first flash already done,
        // so we don't need this part  if flashcount=1
        // temporary timer. The end of this timer stops the permanent flashing

        this.flashTimer2 = engine.beginTimer(flashCount * (num_ms_on + num_ms_off) - num_ms_off, function(){ myself.Stopflash(); }, true);
    }
};

// private : relight=true : restore light state before it was flashing
// this is a call back function (called in flashon() )
ReloopBeatpad.rgbLEDs.prototype.flashOff = function() {
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
    this.layers.show5.activate = false;
    this.activateshow("show6", false);
};

// private : relight=true : restore light state before it was flashing
// this is a call back function (called in flashon() )
ReloopBeatpad.rgbLEDs.prototype.Stopflash = function() {
    // stop permanent timer
    if (this.flashTimer !== 0) {
        engine.stopTimer(this.flashTimer);
    }
    // reset flash variables to 0
    this.flashTimer = 0;
    this.flashTimer2 = 0;
    this.flashDuration = 0;
    this.flashOff();
};

// public
ReloopBeatpad.rgbLEDs.prototype.getFlashDuration = function() {
    return this.flashDuration;
};

// private : call back function (called in flashon() )
ReloopBeatpad.rgbLEDs.prototype.flashOnceOn = function(relight) {
    var myself = this;
    this.layers.show5.activate = true;
    var pausecount = this.activateshow("show6", true);
    this.flashOnceDuration = this.num_ms_on;
    this.flashOnceTimer = engine.beginTimer(this.flashDuration - scriptpause * pausecount,  function(){ myself.flashOnceOff(relight); }, true);
};

// private :call back function (called in flashOnceOn() )
ReloopBeatpad.rgbLEDs.prototype.flashOnceOff = function(relight) {
    this.flashOnceTimer = 0;
    this.flashOnceDuration = 0;
    if (relight) {
        this.layers.show5.activate = true;
    } else {
        this.layers.show5.activate = false;
    }
    this.activateshow("show6", false);
};

// RGB light showtime
// we use a dedicated layer for each light show to display
// Light show n°1 when an effect is turned on, color depending on the effect. (No value = off)
ReloopBeatpad.rgbLEDs.prototype.effects = function(value) {
    var RGBColor;
    switch (value) {
        case 1:
            RGBColor = ReloopBeatpad.RGB.green;
            break;
        case 2:
            RGBColor = ReloopBeatpad.RGB.yellow;
            break;
        case 3:
            RGBColor = ReloopBeatpad.RGB.cyan;
            break;
        case 4:
            RGBColor = ReloopBeatpad.RGB.magenta;
            break;
        default:
            RGBColor = OFF;
            break;
    }
    this.setshow("show1", RGBColor);
    this.activateshow("show1", (RGBColor !== 0));
};

// Light show n°6 when selecting an effect (blinking)
ReloopBeatpad.rgbLEDs.prototype.effectsblink = function(value) {
    var RGBColor;
    switch (value) {
        case 1:
            RGBColor = ReloopBeatpad.RGB.green;
            break;
        case 2:
            RGBColor = ReloopBeatpad.RGB.yellow;
            break;
        case 3:
            RGBColor = ReloopBeatpad.RGB.cyan;
            break;
        case 4:
            RGBColor = ReloopBeatpad.RGB.magenta;
            break;
        default:
            RGBColor = OFF;
            break;
    }
    this.setshow("show6", RGBColor);
    this.flashOn(200, RGBColor, 200, 3, true);
    this.setshow("show1", RGBColor);
};

// Light show n°2 when applying a lp/hp filter
ReloopBeatpad.rgbLEDs.prototype.filter = function(value) {
    var RGBColor1 = (value > 0) ? ReloopBeatpad.RGB.white : null;
    var RGBColor2 = (value > 0) ? ReloopBeatpad.RGB.white : null;
    var RGBColor3 = (value < 0) ? ReloopBeatpad.RGB.white : null;
    var RGBColor4 = (value < 0) ? ReloopBeatpad.RGB.white : null;
    var activate = (value !== 0);
    this.setshow("show2", RGBColor1, RGBColor2, RGBColor3, RGBColor4);
    this.activateshow("show2", activate);
};

// Light show n°3 for loops activated
ReloopBeatpad.rgbLEDs.prototype.loops = function(value) {
    var RGBColor = (value !== 0) ? ReloopBeatpad.RGB.magenta : null;
    var activate = (value !== 0);
    this.setshow("show3", RGBColor);
    this.activateshow("show3", activate);
    ReloopBeatpad.decks[this.deckID].leds.RimBlue.onOff((activate) ? 0x7F : 0);
};

// Light show n°4 to indicate when a deck is not loaded with a track
ReloopBeatpad.rgbLEDs.prototype.notloaded = function(value) {
    var RGBColor = (value) ? ReloopBeatpad.RGB.red : null;
    this.setshow("show4", RGBColor);
    this.activateshow("show4", value);
};

// ******************************************************************
// Sampler bank management
// *********
// This take care of sampler PAD lights when a Bank of Samples or only one sample is changed or loaded,
// or when the sampler mode is changed to display other status
ReloopBeatpad.SamplerBank = function() {
    this.bankactive = 1;
    this.loaded = [];
    this.loaded.length = 17;
};

// This will update only one Sampler PAD light, only if it is necessary
ReloopBeatpad.SamplerBank.prototype.LedUpdate = function(padindex) {
    var color, isloaded, isplaying, deck, decknum;
    var samplerbaseindex, samplerindex;
    for (decknum = 1; decknum <= 2; decknum++) {
        deck = ReloopBeatpad.decks["D" + decknum];
        switch (deck.PadMode) {
            // Sampler bank select status
            case SAMPLERBANKSTATUSMODE:
                color = (padindex == this.bankactive) ? ReloopBeatpad.PadColor.magenta : 0;
                deck.leds["SamplerPad" + padindex].onOff(color);
                break;
                // Loop mode Status
            case LOOPMODESTATUS:
                break;
                // FX rack Select status
            case FXRACKSELECTMODE:
                color = (padindex == deck.CurrentEffectRack) ? ReloopBeatpad.PadColor.purple : 0;
                deck.leds["SamplerPad" + padindex].onOff(color);
                break;
                // Normal sampler mode
            default:
                samplerbaseindex = (this.bankactive - 1) * 4;
                samplerindex = samplerbaseindex + padindex;
                isloaded = this.loaded[samplerindex];
                isplaying = engine.getValue("[Sampler" + samplerindex + "]", "play");
                color = ReloopBeatpad.PadColor.orange;
                if (isplaying) {
                    color = ReloopBeatpad.PadColor.uv;
                }
                deck.leds["SamplerPad" + padindex].onOff(isloaded ? color : OFF);
                break;
        }
    }
};

// This will update only one Sampler PAD light, only if it is necessary
// This is used to filter the led change in normal sampler mode, depending
// if we display the sample bank of the requested Sample or not
ReloopBeatpad.SamplerBank.prototype.LedUpdateSampler = function(samplernum) {
    var samplerbaseindex = (this.bankactive - 1) * 4;
    if ((samplerbaseindex < samplernum) && (samplernum <= (samplerbaseindex + 4))) {
        var padindex = samplernum - samplerbaseindex;
        this.LedUpdate(padindex);
    }
};

// Will update, if necessary the four Sampler PAD at once
ReloopBeatpad.SamplerBank.prototype.LedsUpdate = function() {
    var samplerbaseindex = (this.bankactive - 1) * 4;
    var samplerindex = 0;
    var color = 0;
    var isloaded, isplaying, deck, decknum,i;

    for (decknum = 1; decknum <= 2; decknum++) {
        deck = ReloopBeatpad.decks["D" + decknum];
        switch (deck.PadMode) {
            // Sampler bank select status
            case SAMPLERBANKSTATUSMODE:
                for (i = 1; i <= 4; i++) {
                    color = (i == this.bankactive) ? ReloopBeatpad.PadColor.fushia : 0;
                    deck.leds["SamplerPad" + i].onOff(color);
                    //deck.leds["sSamplerPad" + i].onOff(color);
                }
                break;
                // Loop mode Status
            case LOOPMODESTATUS:
                deck.leds.SamplerPad1.onOff(ReloopBeatpad.PadColor.magenta);
                deck.leds.SamplerPad2.onOff(OFF);
                deck.leds.SamplerPad3.onOff(OFF);
                color = (deck.loopkind == SIMPLE) ? OFF : ReloopBeatpad.PadColor.magenta;
                deck.leds.SamplerPad4.onOff(color);
                break;
                // FX rack Select status
            case FXRACKSELECTMODE:
                for (i = 1; i <= 4; i++) {
                    color = (i == deck.CurrentEffectRack) ? ReloopBeatpad.PadColor.purple : 0;
                    deck.leds["SamplerPad" + i].onOff(color);
                }
                break;
                // Normal sampler mode
            default:
                for (i = 1; i <= 4; i++) {
                    samplerindex = samplerbaseindex + i;
                    isloaded = this.loaded[samplerindex];
                    isplaying = engine.getValue("[Sampler" + samplerindex + "]", "play");
                    color = ReloopBeatpad.PadColor.orange;
                    if (isplaying) {
                        color = ReloopBeatpad.PadColor.fushia;
                    }
                    deck.leds["SamplerPad" + i].onOff(isloaded ? color : OFF);
                }
                break;
        }
    }
};


ReloopBeatpad.SamplerBank.prototype.SetLoaded = function(samplernum, value) {
    this.loaded[samplernum] = value;
    this.LedUpdateSampler(samplernum);
};

ReloopBeatpad.SamplerBank.prototype.LoadBank = function(deck, banknum) {
    this.bankactive = banknum;
    deck.controls["sSamplerPad" + banknum].onOff(true);
};

ReloopBeatpad.SamplerBank.prototype.play = function(padnum, value) {
    var samplerindex = (this.bankactive - 1) * 4 + padnum;
    var isplaying = engine.getValue("[Sampler" + samplerindex + "]", "play");
    if (value) {
        if (!isplaying) {
            engine.setValue("[Sampler" + samplerindex + "]", "cue_gotoandplay", 1);
            engine.setValue("[Sampler" + samplerindex + "]", "beatsync", 1);
        } else {
            engine.setValue("[Sampler" + samplerindex + "]", "stop", 1);
        }
    } else {
        engine.setValue("[Sampler" + samplerindex + "]", "stop", 1);
    }
};

ReloopBeatpad.samplers = new ReloopBeatpad.SamplerBank();

// ******************************************************************
// Controls
// *********
// Control class for control objects, e.g. play button. Objects for the
// two jog wheels are not created this way, instead they are
// represented directly by the deck objects. This was just an easier
// way to do it, and there will only ever be one jog wheel
// per deck anyway.
ReloopBeatpad.control = function(key, control, midino, group) {
    this.key = key;
    this.control = control;
    this.midino = midino;
    this.group = group;
};

ReloopBeatpad.control.prototype.onOff = function(value, light) {
    engine.setValue(this.group, this.key, value);
    if ((typeof light !== "undefined") && (typeof this.led != "undefined")) {
        this.led.onOff(light);
    }
};

ReloopBeatpad.control.prototype.checkOn = function() {
    var checkOn = engine.getValue(this.group, this.key);
    return checkOn;
};

ReloopBeatpad.control.prototype.toggle = function(value) {
    toggleValue(this.group, this.key);
};

// ******************************************************************
// Decks
// *********


ReloopBeatpad.deck = function(deckNum) {
    this.deckNum = deckNum;
    this.group = "[Channel" + deckNum + "]";
    this.Shifted = false;
    this.firstbeatpos = 0;
    this.beatpos = 0;
    this.CurrentEffectRack = 1;
    this.loaded = false;
    this.timers = [];
    this.JogScratchStatus = false;
    this.JogSeekStatus = false;
    this.FX_ONStatus = false;
    this.LoopStatus = false;
    this.PadMode = CUEMODE;
    this.seekingfast = true;
    this.filterligthshowstatus = 0;
    this.looplightshowstatus = 0;

    this.loopkind = SIMPLE;
    this.loopsize = 1;
    this.looppadstatus = 0;
    this.InstantFXBtnDown = false;

    //The reloop Beatpad is a model "B" controller (see the Jogger declaration/constructor
    this.Jog = new Jogger(this.group, this.deckNum,"B");

    // for the deck--buttons, sliders, etc--are associated with
    // the deck using this array.
    this.controls = [];
    this.leds = [];
    this.RGBShow = new ReloopBeatpad.rgbLEDs(0x90+deckNum, "D"+deckNum);
};

ReloopBeatpad.deck.prototype.SelectEffectRack = function(newindex) {
    this.CurrentEffectRack = newindex;
    this.RGBShow.effectsblink(newindex);
    ReloopBeatpad.samplers.LedsUpdate();
};

// toggle loop mode into SIMPLE/ROLL
ReloopBeatpad.deck.prototype.ToggleLoopKind = function() {
    var isLoopActive = engine.getValue(this.group, "loop_enabled");
    this.loopkind = 3 - this.loopkind;
    if (isLoopActive) {
        if (this.loopkind == SIMPLE) {
            engine.setValue(this.group, "beatloop_" + this.loopsize+ "_activate", 1);
        } else {
            engine.setValue(this.group, "beatlooproll_" + this.loopsize+ "_activate", 1);
        }
    }
    ReloopBeatpad.samplers.LedsUpdate();
};

ReloopBeatpad.deck.prototype.TrackIsLoaded = function() {
    return TrackIsLoaded(this.group);
};

// trigger some controls if shift is pressed in order to update
// some LEDs in SHIFT mode/non SHIFT mode.
// some LEDs of the Beatpad buttons can have two states, one in SHIFT
// mode, one in Regular mode and can be updated only once SHIFT is
// pressed or not.
ReloopBeatpad.deck.prototype.triggershift = function() {
    var i;
    if (this.Shifted) {
        this.leds.brake.onOff(OFF);
        this.leds.censor.onOff(OFF);
    }
    engine.trigger(this.group, "track_samples");
    engine.trigger(this.group, "play_indicator");
    engine.trigger(this.group, "cue_point");
    engine.trigger(this.group, "sync_enabled");
    engine.trigger(this.group, "keylock");
    engine.trigger(this.group, "pfl");

    for (i = 1; i <= 4; i++) {
        engine.trigger(this.group, "hotcue_" + i + "_position");
    }

    for (i = 1; i <= 4; i++) {
        engine.trigger("[Deere]", "sampler_bank_" + i);
    }
};

// Constructor for creating control/led objects
ReloopBeatpad.deck.prototype.addControl = function(arrID, ID, controlObj, addLED) {
    var arrAdd = this[arrID];
    if (addLED) {
        // If the button can illuminate, a led object is created for it (see above).

        controlObj.led = new LED(controlObj.control, controlObj.midino);
        this.leds[ID] = controlObj.led;
    }
    arrAdd[ID] = controlObj;
};


// =====================================================================
// Initialization of the mapping
// =====================================================================
// LED creator
ReloopBeatpad.add2LEDs = function(ID, midiname, complement, addLight) {
    var midino = ReloopBeatpad.MIDI[midiname]+complement;
    ReloopBeatpad.decks.D1.leds[ID] = new LED(LBtn, midino);
    ReloopBeatpad.decks.D2.leds[ID] = new LED(RBtn, midino);
};

// Creating the two deck objects.
ReloopBeatpad.decks.D1 = new ReloopBeatpad.deck("1");
ReloopBeatpad.decks.D2 = new ReloopBeatpad.deck("2");

ReloopBeatpad.recordingled = new LED(MBtn, ReloopBeatpad.MIDI.rec);

// ----------   Other global variables    ----------
ReloopBeatpad.initobjects = function() {
    var i;
    // control creators
    this.cc1 = function(arrID, ID, key, midiname, complement, group, addLight) {
        var midino = ReloopBeatpad.MIDI[midiname]+complement;
        var NewControl = new ReloopBeatpad.control(key, LBtn, midino,group);
        var deck = ReloopBeatpad.decks.D1;
        deck.addControl(arrID, ID, NewControl, addLight);
    };

    this.cc2 = function(arrID, ID, key, midiname, complement, group, addLight) {
        var midino = ReloopBeatpad.MIDI[midiname]+complement;
        var NewControl = new ReloopBeatpad.control(key, RBtn, midino,group);
        var deck = ReloopBeatpad.decks.D2;
        deck.addControl(arrID, ID, NewControl, addLight);
    };

    this.cc = function(arrID, ID, key, midiname, complement, addLight) {
        this.cc1(arrID, ID, key, midiname, complement, "[Channel1]", addLight);
        this.cc2(arrID, ID, key, midiname, complement, "[Channel2]", addLight);
    };

    // All controls below associated with left or right deck.
    this.cc("controls", "load", "LoadSelectedTrack", "Load", 0, true);
    this.cc("controls", "sync", "beatsync", "mSync", 0, true);
    this.cc("controls", "start", "start", "mSync", SHIFT, true);
    this.cc("controls", "pfl", "pfl", "pfl", 0, true);
    this.cc("controls", "slip", "slip_enabled", "pfl", SHIFT, true);
    this.cc("controls", "quantize", "quantize", "pfl", SHIFT, true);
    this.cc("controls", "Set", "cue_default", "Set", 0, true);
    this.cc("controls", "keylock", "keylock", "Set", SHIFT, true);
    this.cc("controls", "jump", "cue_gotoandplay", "Jump", 0, true);
    this.cc("controls", "play", "play", "Play", 0, true);
    this.cc("controls", "LoadAndPlay", "LoadSelectedTrackAndPlay", "Play", 0, true);
    this.cc("controls", "censor", "reverseroll", "Play", SHIFT, true);
    this.cc("controls", "bendMinus", "rate_temp_down", "BendMinus", 0, true);
    this.cc("controls", "beatjumpMinus", "beatjump_1_backward", "BendMinus", SHIFT, true);
    this.cc("controls", "bendPlus", "rate_temp_up", "BendPlus", 0, true);
    this.cc("controls", "beatjumpPlus", "beatjump_1_forward", "BendPlus", SHIFT, true);

    for (i = 1; i <= 4; i++) {
        this.cc1("controls", "FXPad" + i, "group_[Channel1]_enable", "FXPad", i - 1, "[EffectRack1_EffectUnit" + i + "]", HardwareLight);
        this.cc2("controls", "FXPad" + i, "group_[Channel2]_enable", "FXPad", i - 1, "[EffectRack1_EffectUnit" + i + "]", HardwareLight);
    }

    for (i = 1; i <= 4; i++) {
        this.cc("controls", "LoopPad" + i, "beatlooproll_" + Math.pow(2, i - 4) + "_activate", "LoopPad", i - 1, HardwareLight);
    }

    for (i = 1; i <= 4; i++) {
        this.cc("controls", "sLoopPad" + i, "beatlooproll_" + Math.pow(2, i) + "_activate", "LoopPad", i - 1 + SHIFT, HardwareLight);
    }

    for (i = 1; i <= 4; i++) {
        this.cc1("controls", "sSamplerPad" + i, "sampler_bank_" + i, "SamplerPad", i - 1 + SHIFT, "[Deere]", false);
        this.cc2("controls", "sSamplerPad" + i, "sampler_bank_" + i, "SamplerPad", i - 1 + SHIFT, "[Deere]", false);
    }

    // LEDs


    ReloopBeatpad.add2LEDs("brake", "Jump", SHIFT, true);
    ReloopBeatpad.add2LEDs("RimRed", "RIM_Red", 0, true);
    ReloopBeatpad.add2LEDs("RimBlue", "RIM_Blue", 0, true);


    ReloopBeatpad.add2LEDs("Loop", "Loop", 0, true);
    ReloopBeatpad.add2LEDs("FX_ON", "FX_ON", 0, true);

    for (i = 1; i <= 4; i++) {
        ReloopBeatpad.add2LEDs("CuePad" + i, "CuePad", i - 1, true);
        ReloopBeatpad.add2LEDs("sCuePad" + i, "CuePad", i - 1 + SHIFT, true);
    }

    for (i = 1; i <= 4; i++) {
        ReloopBeatpad.add2LEDs("SamplerPad" + i, "SamplerPad", i - 1, true);
    }
    ReloopBeatpad.add2LEDs("VUMeter", "VUMeter", 0, true);
};

ReloopBeatpad.initButtonsObjects = function() {
    var i;
    for(i=1;i<=2;i++) {
        ReloopBeatpad.decks["D"+i].LoadButtonControl =
            new LongShortBtn(ReloopBeatpad.OnLoadButton);
        ReloopBeatpad.decks["D"+i].SyncButtonControl =
            new LongShortDoubleBtn(ReloopBeatpad.OnSyncButton);
        ReloopBeatpad.decks["D"+i].ShiftedPFLButtonControl =
            new SingleDoubleBtn(ReloopBeatpad.OnShiftedPFLButton);
    }
};

// =====================================================================
// Init an shutdown main entries functions
// =====================================================================
ReloopBeatpad.init = function(id, debug) {
    var i;
    // Connect button lights to equivalent controls. The track_samples control
    // is being used to tell if a track has successfully loaded (whereupon lights
    // flash twice. Best way I could think of in the absence of a proper control
    // for checking whether a track is loaded.
    ReloopBeatpad.id = id;
    print("********* Initialisation process engaged *****************");
    print("1/3 : Mapping initialization");
    print("============================");
    TurnLEDsOff();

    ReloopBeatpad.initButtonsObjects();
    ReloopBeatpad.initobjects();

    // Set soft-takeover for all Sampler volumes
    for (i = engine.getValue("[Master]", "num_samplers"); i >= 1; i--) {
        engine.softTakeover("[Sampler" + i + "]", "pregain", true);
    }
    // Set soft-takeover for all applicable Deck controls
    for (i = engine.getValue("[Master]", "num_decks"); i >= 1; i--) {
        engine.softTakeover("[Channel" + i + "]", "volume", true);
        engine.softTakeover("[Channel" + i + "]", "filterHigh", true);
        engine.softTakeover("[Channel" + i + "]", "filterMid", true);
        engine.softTakeover("[Channel" + i + "]", "filterLow", true);
    }

    engine.softTakeover("[Master]", "crossfader", true);

    for (i = 1; i <= 4; i++) {
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "]", "super1", true);
    }



    engine.connectControl("[Channel1]", "play_indicator", "ReloopBeatpad.OnPlayIndicatorChange");
    engine.connectControl("[Channel2]", "play_indicator", "ReloopBeatpad.OnPlayIndicatorChange");
    engine.connectControl("[Channel1]", "beat_active", "ReloopBeatpad.OnBeatActive");
    engine.connectControl("[Channel2]", "beat_active", "ReloopBeatpad.OnBeatActive");
    engine.connectControl("[Channel1]", "cue_point", "ReloopBeatpad.OnCuePointChange");
    engine.connectControl("[Channel2]", "cue_point", "ReloopBeatpad.OnCuePointChange");
    engine.connectControl("[Channel1]", "sync_enabled", "ReloopBeatpad.OnSyncEnabledChange");
    engine.connectControl("[Channel2]", "sync_enabled", "ReloopBeatpad.OnSyncEnabledChange");
    engine.connectControl("[Channel1]", "track_samples", "ReloopBeatpad.OnTrackLoaded");
    engine.connectControl("[Channel2]", "track_samples", "ReloopBeatpad.OnTrackLoaded");
    engine.connectControl("[Channel1]", "keylock", "ReloopBeatpad.OnKeylock");
    engine.connectControl("[Channel2]", "keylock", "ReloopBeatpad.OnKeylock");
    engine.connectControl("[Channel1]", "VuMeter", "ReloopBeatpad.OnVuMeterChange");
    engine.connectControl("[Channel2]", "VuMeter", "ReloopBeatpad.OnVuMeterChange");
    engine.connectControl("[Channel1]", "playposition", "ReloopBeatpad.OnPlaypositionChange");
    engine.connectControl("[Channel2]", "playposition", "ReloopBeatpad.OnPlaypositionChange");
    engine.connectControl("[Channel1]", "duration", "ReloopBeatpad.OnDurationChange");
    engine.connectControl("[Channel2]", "duration", "ReloopBeatpad.OnDurationChange");
    engine.connectControl("[Channel1]", "pfl", "ReloopBeatpad.OnPFLStatusChange");
    engine.connectControl("[Channel2]", "pfl", "ReloopBeatpad.OnPFLStatusChange");

    engine.connectControl("[EffectRack1_EffectUnit1_Effect1]", "loaded", function(value, group, control) {
        ReloopBeatpad.OnEffectLoaded(value, group, control, 1);
    });
    engine.connectControl("[EffectRack1_EffectUnit2_Effect1]", "loaded", function(value, group, control) {
        ReloopBeatpad.OnEffectLoaded(value, group, control, 2);
    });
    engine.connectControl("[EffectRack1_EffectUnit3_Effect1]", "loaded", function(value, group, control) {
        ReloopBeatpad.OnEffectLoaded(value, group, control, 3);
    });
    engine.connectControl("[EffectRack1_EffectUnit4_Effect1]", "loaded", function(value, group, control) {
        ReloopBeatpad.OnEffectLoaded(value, group, control, 4);
    });

    engine.connectControl("[Recording]", "status", "ReloopBeatpad.OnRecordingStatusChange");

    engine.connectControl("[Channel1]", "hotcue_1_position", function(value, group, control) {
        ReloopBeatpad.OnHotcuePositionChange(value, group, control, 1);
    });
    engine.connectControl("[Channel2]", "hotcue_1_position", function(value, group, control) {
        ReloopBeatpad.OnHotcuePositionChange(value, group, control, 1);
    });
    engine.connectControl("[Channel1]", "hotcue_2_position", function(value, group, control) {
        ReloopBeatpad.OnHotcuePositionChange(value, group, control, 2);
    });
    engine.connectControl("[Channel2]", "hotcue_2_position", function(value, group, control) {
        ReloopBeatpad.OnHotcuePositionChange(value, group, control, 2);
    });
    engine.connectControl("[Channel1]", "hotcue_3_position", function(value, group, control) {
        ReloopBeatpad.OnHotcuePositionChange(value, group, control, 3);
    });
    engine.connectControl("[Channel2]", "hotcue_3_position", function(value, group, control) {
        ReloopBeatpad.OnHotcuePositionChange(value, group, control, 3);
    });
    engine.connectControl("[Channel1]", "hotcue_4_position", function(value, group, control) {
        ReloopBeatpad.OnHotcuePositionChange(value, group, control, 4);
    });
    engine.connectControl("[Channel2]", "hotcue_4_position", function(value, group, control) {
        ReloopBeatpad.OnHotcuePositionChange(value, group, control, 4);
    });

    for (i = 1; i <= 16; i++) {
        engine.connectControl("[Sampler" + i + "]", "track_samples", "ReloopBeatpad.OnSampleLoaded");
        engine.connectControl("[Sampler" + i + "]", "play", "ReloopBeatpad.OnSamplePlayStop");
    }

    engine.connectControl("[Deere]", "sampler_bank_1", function(value, group, control) {
        ReloopBeatpad.OnBankLoaded(value, group, control, 1);
    });
    engine.connectControl("[Deere]", "sampler_bank_2", function(value, group, control) {
        ReloopBeatpad.OnBankLoaded(value, group, control, 2);
    });
    engine.connectControl("[Deere]", "sampler_bank_3", function(value, group, control) {
        ReloopBeatpad.OnBankLoaded(value, group, control, 3);
    });
    engine.connectControl("[Deere]", "sampler_bank_4", function(value, group, control) {
        ReloopBeatpad.OnBankLoaded(value, group, control, 4);
    });

    // After midi controller receive this Outbound Message request SysEx Message,
    // midi controller will send the status of every item on the
    // control surface. (Mixxx will be initialized with current values)


    // check if there is already something loaded on each deck (when script reinitialize)
    engine.trigger("[Channel1]", "track_samples");
    engine.trigger("[Channel2]", "track_samples");
    engine.trigger("[Channel1]", "play_indicator");
    engine.trigger("[Channel2]", "play_indicator");
    engine.trigger("[Channel1]", "beat_active");
    engine.trigger("[Channel2]", "beat_active");
    engine.trigger("[Channel1]", "cue_point");
    engine.trigger("[Channel2]", "cue_point");
    engine.trigger("[Channel1]", "sync_enabled");
    engine.trigger("[Channel2]", "sync_enabled");
    engine.trigger("[Channel1]", "keylock");
    engine.trigger("[Channel2]", "keylock");
    engine.trigger("[Channel1]", "VuMeter");
    engine.trigger("[Channel2]", "VuMeter");
    engine.trigger("[Channel1]", "playposition");
    engine.trigger("[Channel2]", "playposition");
    engine.trigger("[Channel1]", "bpm");
    engine.trigger("[Channel2]", "bpm");
    engine.trigger("[Channel1]", "duration");
    engine.trigger("[Channel2]", "duration");
    engine.trigger("[Channel1]", "pfl");
    engine.trigger("[Channel2]", "pfl");
    engine.trigger("[Recording]", "status");

    for (i = 1; i <= 4; i++) {
        engine.trigger("[Channel1]", "hotcue_" + i + "_position");
        engine.trigger("[Channel2]", "hotcue_" + i + "_position");
    }

    for (i = 1; i <= 16; i++) {
        engine.trigger("[Sampler" + i + "]", "track_samples");
    }

    for (i = 1; i <= 4; i++) {
        engine.trigger("[Deere]", "sampler_bank_" + i);
    }

    print("Mapping script initialized.");
    print();
    print("2/3 : Checking controller");
    print("=========================");
    print("Sysex ID request... :");
    // has to be send twice to the controller in order to work. (???)
    midi.sendSysexMsg(SysexIDRequest, SysexIDRequest.length);
    pauseScript(20);
};

ReloopBeatpad.shutdown = function() {
    var i;
    // Stop all timers
    for (i = 0; i < ReloopBeatpad.timers.length; i++) {
        engine.stopTimer(ReloopBeatpad.timers[i]);
    }

    // Extinguish all LEDs
    TurnLEDsOff();
    print("Reloop Beatpad: " + ReloopBeatpad.id + " shut down.");
};

// =====================================================================
// Buttons, Jogs mappings
// (All functions declared in the xml part of the mapping)
// =====================================================================


// The Jog Scratch touch that enables/disables scratching
ReloopBeatpad.WheelScratchTouch = function(channel, control, value, status, group) {
    var decknum = script.deckFromGroup(group);
    var deck = ReloopBeatpad.decks["D" + decknum]; // works out which deck we are using
    // if "control"<0x40, then the DJ is doing a SHIFT+JogWheel and we enable the iCut
    // if "Jog Scratch" mode is activated on the Beatpad, we have for the "control" value
    // Left JogWheel --> 0x63, +SHIFT --> 0x23
    // Right JogWheel --> 0x65, +SHIFT --> 0x25
    deck.Jog.onWheelTouch(value, (control<0x40));
};


// The Jog Scratch that actually controls the scratching
ReloopBeatpad.WheelScratch = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    deck.Jog.onWheelMove(value, (control<0x40));
};

ReloopBeatpad.WheelSeekTouch = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    if (value == DOWN) {
        // Hand on the Jog wheel, fast seek activated
        deck.seekingfast = true;
    } else {
        // Hand off the Jog wheel, deactivate fast seek
        deck.seekingfast = false;
    }
};

ReloopBeatpad.WheelSeek = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    // Test if we are "seeking fast". If not, it means that the DJ
    // is using the border ring, we then navigate into the track
    // slowly (beatjump function id working way better than "fwd"or "back"
    // functions with the jogwheels for seeking the track)
    if (!deck.seekingfast) {
        engine.setValue(group, "beatjump", (value - 0x40) / 4);
    } else {
        engine.setValue(group, "beatjump", (value - 0x40));
    }

};

ReloopBeatpad.WheelBendTouch = function(channel, control, value, status, group) {
    // Useless, but room for future idea
};

ReloopBeatpad.WheelBend = function(channel, control, value, status, group) {
    engine.setValue(group, "jog", (value - 0x40) / bendConst);
};



// ********************** Buttons and Co ****************
ReloopBeatpad.ShiftBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    deck.Shifted = (value === DOWN);
    // trigger() : lights in different mode if the deck is shifted or not
    // examples : SYNC/SET/JUMP and Pause/play buttons have a second function
    deck.triggershift();
};

ReloopBeatpad.Brake = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    if (value == DOWN) {
        deck.leds.brake.onOff(ON);
        engine.brake(decknum, true); // enable brake effect
    } else {
        deck.leds.brake.onOff(OFF);
        engine.brake(decknum, false); // disable brake effect
    }
};

// Censor
ReloopBeatpad.ReverseRoll = function(channel, control, value, status, group) {

    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    if (value == DOWN) {
        deck.controls.censor.onOff(true, ON);
    } else {
        deck.controls.censor.onOff(false,OFF);
    }
};

ReloopBeatpad.SelectPlayList = function(channel, control, value, status, group) {
    var i;
    value = value - 0x40;
    if (value < 0) {
        for (i = 0; i < -value; i++) {
            engine.setValue(group, "SelectPrevPlaylist", true);
        }
    } else {
        for (i = 0; i < value; i++) {
            engine.setValue(group, "SelectNextPlaylist", true);
        }
    }
};

ReloopBeatpad.RecBtn = function(channel, control, value, status, group) {
    if (value == DOWN) {
        engine.setValue(group, "toggle_recording", true);
    }
};

ReloopBeatpad.sRecBtn = function(channel, control, value, status, group) {
    print("RecBtn ");
    if (value == DOWN) {
        TrackEndWarning = !TrackEndWarning;
        print("TrackEndWarning " + TrackEndWarning);
        ReloopBeatpad.recordingled.flashOn(200, ON, 200, 3, true);
    }
};

ReloopBeatpad.pflBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        deck.controls.pfl.toggle();
    }
};

ReloopBeatpad.spflBtn = function(channel, control, value, status, group) {
    // simple press : slip mode togggle ; double press : quantize toggle
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        deck.ShiftedPFLButtonControl.ButtonDown(channel, control, value, status, group);
    }
};

// Callback for the PFL Button
ReloopBeatpad.OnShiftedPFLButton = function(channel, control, value, status, group, eventkind) {
    var decknum = script.deckFromGroup(group);
    var deck = ReloopBeatpad.decks["D" + decknum];
    if (eventkind == DOUBLE_PRESS) {
        // Double press : toggle slip mode
        deck.controls.slip.toggle();
    } else {
        // Single press : toggle quantize mode
        deck.controls.quantize.toggle();
    }
};

ReloopBeatpad.LoadBtn = function(channel, control, value, status, group) {
    // LOAD hold <500ms : load track, >500ms : eject
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        deck.LoadButtonControl.ButtonDown(channel, control, value, status, group);
    } else {
        deck.LoadButtonControl.ButtonUp();
    }
};

// Callback for the Load Button
ReloopBeatpad.OnLoadButton = function(channel, control, value, status, group, eventkind) {
    var decknum = script.deckFromGroup(group);
    var deck = ReloopBeatpad.decks["D" + decknum];

    if (eventkind == LONG_PRESS) {
        engine.setValue(group, 'eject', true);
        deck.leds.load.onOff(OFF);
    } else {
        engine.setValue(group, 'LoadSelectedTrack', true);
        deck.leds.load.onOff(ON);
    }
};

ReloopBeatpad.SyncBtn = function(channel, control, value, status, group) {
    // SYNC pressed once :sync  , pressed twice : play, long press SYNC Lock

    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        deck.SyncButtonControl.ButtonDown(channel, control, value, status, group);
    } else {
        deck.SyncButtonControl.ButtonUp();
    }
};

// Callback for the SYNC Button
ReloopBeatpad.OnSyncButton = function(channel, control, value, status, group, eventkind) {
    if (eventkind == LONG_PRESS) {
        engine.setValue(group, 'sync_enabled', true);
    } else {
        if (engine.getValue(group, 'sync_enabled')) {
            // If sync lock is enabled, simply disable sync lock
            engine.setValue(group, 'sync_enabled', false);
        } else {
            if (eventkind == DOUBLE_PRESS) {
                // Double press : Sync and play (if the track was paused
                // the playback starts, synchronized to the other track
                engine.setValue(group, 'play', true);
                engine.setValue(group, 'beatsync', true);

            } else {
                // We pressed sync only once, we sync the track
                // with the other track (eventkind == QUICK_PRESS
                engine.setValue(group, 'beatsync', true);
            }
        }
    }
};

ReloopBeatpad.StartBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        deck.controls.start.onOff(ON,ON);

    } else {
        deck.controls.start.onOff(OFF,OFF);
    }
};

ReloopBeatpad.PlayBtn = function(channel, control, value, status, group) {
    // toggle play. If no track is loaded, load the one which is selected and play
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        if (!deck.TrackIsLoaded()) {
            deck.controls.LoadAndPlay.onOff(true);
        } else {
            deck.controls.play.toggle(); // toggle play/pause
        }
    }
};

ReloopBeatpad.JumpBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        if (!deck.TrackIsLoaded()) {
            deck.controls.load.onOff(true);
        }
        deck.controls.jump.onOff(true);
    }
};


ReloopBeatpad.JogSeekBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    deck.JogSeekStatus = (value == DOWN);
    deck.beatpos = 0;
    engine.trigger(group, "playposition");
};

ReloopBeatpad.JogScratchBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    deck.JogScratchStatus = (value == DOWN);
    deck.beatpos = 0;
    engine.trigger(group, "playposition");
};

ReloopBeatpad.FX_ONBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        if (deck.leds.FX_ON.checkOn() == OFF) {
            deck.leds.FX_ON.onOff(ON);
            deck.controls["FXPad" + deck.CurrentEffectRack].onOff(ON);
            deck.RGBShow.effects(deck.CurrentEffectRack);
        } else {
            deck.controls["FXPad" + deck.CurrentEffectRack].onOff(OFF);
            deck.leds.FX_ON.onOff(OFF);
            deck.RGBShow.effects(OFF);
        }
    }
};

// Pads Performance mode
ReloopBeatpad.ShowSamplersAndEffects = function() {
    var PadMode1 = ReloopBeatpad.decks.D1.PadMode;
    var PadMode2 = ReloopBeatpad.decks.D2.PadMode;
    // SAMPLERMODE/ SAMPLERBANKSTATUSMODE / FXRACKSELECTMODE
    var OpenFX = ((PadMode1 == FXMODE) || (PadMode2 == FXMODE) || (PadMode1 == FXRACKSELECTMODE) || (PadMode2 == FXRACKSELECTMODE));
    var OpenSampler = ((PadMode1 == SAMPLERMODE) || (PadMode2 == SAMPLERMODE) || (PadMode1 == SAMPLERBANKSTATUSMODE) || (PadMode2 == SAMPLERBANKSTATUSMODE));
    engine.setValue("[Samplers]", "show_samplers", OpenSampler);
    engine.setValue("[EffectRack1]", "show", OpenFX);
};

ReloopBeatpad.CueBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        deck.PadMode = CUEMODE;
        ReloopBeatpad.ShowSamplersAndEffects();
    }
};

ReloopBeatpad.InstantFXBtn = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];

    if (control <= 0x30) { // (SHIFT+Btn)<=0x30
        if (value == DOWN) {
            // enable spinback effect
            engine.spinback(decknum, true);
        } else {
            // disable spinback effect
            engine.spinback(decknum, false);
        }
    } else {
        deck.InstantFXBtnDown = (value == DOWN);
        if (value == DOWN) {
            deck.PadMode = FXMODE;
            ReloopBeatpad.ShowSamplersAndEffects();
        }
    }
};

ReloopBeatpad.FXSelectPush = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    // quick button for the current fx

    // deactivate previous pending effect
    if (value == DOWN) {
        deck.controls["FXPad" + deck.CurrentEffectRack].onOff(ON);
        deck.leds.FX_ON.onOff(ON);
        deck.RGBShow.effects(deck.CurrentEffectRack);

    } else {
        if (!deck.leds.FX_ON.checkOn()) {
            /* We did it already :
             * -------------------
             * deck.controls["FXPad"+padindex].onOff(OFF);
             */
            deck.RGBShow.effects(OFF);
            deck.controls["FXPad" + deck.CurrentEffectRack].onOff(OFF);
            deck.leds.FX_ON.onOff(OFF);
        } else {
            deck.controls["FXPad" + deck.CurrentEffectRack].onOff(ON);
            deck.RGBShow.effects(deck.CurrentEffectRack);
        }
    }
};

ReloopBeatpad.sFXSelectPush = function(channel, control, value, status, group) {
    // quick button for Instant fx : ENABLE/DISABLE

    // deactivate previous pending effect
    if (value == DOWN) {
        toggleValue("[QuickEffectRack1_" + group + "_Effect1]", "enabled");
    }
};

ReloopBeatpad.InstantFXPad = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    var padindex = control - ReloopBeatpad.MIDI.FXPad + 1;
    // pads are quick buttons
    if (deck.InstantFXBtnDown) {
        if (value == DOWN) {
            // InstantFX down+InstantFX Pad = SHIFT + InstantFX Pad
            // select/unselect current effectunits for settings
            deck.SelectEffectRack(padindex);
        }
    } else {
        // deactivate previous pending effect
        deck.controls["FXPad" + deck.CurrentEffectRack].onOff(OFF);
        if (value == DOWN) {
            deck.controls["FXPad" + padindex].onOff(ON);
            deck.RGBShow.effects(deck.CurrentEffectRack);
            deck.leds.FX_ON.onOff(ON);
        } else {
            if (!deck.leds.FX_ON.checkOn()) {
                /* We did it already :
                 * -------------------
                 * deck.controls["FXPad"+padindex].onOff(OFF);
                 */
                deck.RGBShow.effects(OFF);
                deck.leds.FX_ON.onOff(OFF);
            } else {
                deck.controls["FXPad" + deck.CurrentEffectRack].onOff(ON);
                deck.RGBShow.effects(deck.CurrentEffectRack);
            }
        }
    }
};

ReloopBeatpad.sInstantFXPad = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    var padindex = control - ReloopBeatpad.MIDI.FXPad + 1 - SHIFT;
    // SHIFT + InstantFX Pad
    // select/unselect current effectunits for settings
    if (value == DOWN) {
        deck.SelectEffectRack(padindex);
    }
};

ReloopBeatpad.BounceBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        deck.PadMode = LOOPMODE;
        ReloopBeatpad.ShowSamplersAndEffects();
    }
};

ReloopBeatpad.LoopBtn = function(channel, control, value, status, group) {
    var isLoopActive = engine.getValue(group, "loop_enabled");
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        if (deck.loopkind == SIMPLE) {
            if (!isLoopActive) {
                // activate loop
                engine.setValue(group, "beatloop_" + deck.loopsize.toString() + "_activate", 1);
                deck.leds.Loop.onOff(ON);
                deck.RGBShow.loops(ON);
            } else {
                //deactivate loop
                engine.setValue(group, "reloop_exit", true);
                deck.leds.Loop.onOff(OFF);
                deck.RGBShow.loops(OFF);
            }
        } else {
            // activate roll
            engine.setValue(group, "beatlooproll_" + deck.loopsize.toString() + "_activate", 1);
            deck.leds.Loop.onOff(ON);
            deck.RGBShow.loops(ON);
        }
    } else {
        if (deck.loopkind == ROLL) {
            // deactivate roll
            engine.setValue(group, "beatlooproll_" + deck.loopsize.toString() + "_activate", 0);
            deck.leds.Loop.onOff(OFF);
            deck.RGBShow.loops(OFF);
        }
    }
};

ReloopBeatpad.LoopPad = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    var padindex = control - ReloopBeatpad.MIDI.LoopPad + 1;
    if (value == DOWN) {
        deck.loopsize = Math.pow(2, padindex - 4);
        deck.looppadstatus += 1;
        deck.controls["LoopPad" + padindex].onOff(ON);
        deck.leds.Loop.onOff(OFF);
        deck.RGBShow.loops(ON);
    } else {
        if (!deck.leds.Loop.checkOn()) {
            deck.looppadstatus -= 1;
            if (deck.looppadstatus === 0) {
                deck.controls["LoopPad" + padindex].onOff(OFF);
                deck.RGBShow.loops(OFF);
            }
        } else {
            deck.loopsize = Math.pow(2, padindex - 4);
            deck.RGBShow.loops(ON);
        }
    }
};

ReloopBeatpad.sLoopPad = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    var padindex = control - ReloopBeatpad.MIDI.LoopPad + 1 - SHIFT;
    if (value == DOWN) {
        deck.loopsize = Math.pow(2, padindex);
        deck.looppadstatus += 1;
        deck.controls["sLoopPad" + padindex].onOff(ON);
        deck.leds.Loop.onOff(OFF);
        deck.RGBShow.loops(ON);
    } else {
        if (!deck.leds.Loop.checkOn()) {
            deck.looppadstatus -= 1;
            if (deck.looppadstatus === 0) {
                deck.controls["LoopPad" + padindex].onOff(OFF);
                deck.RGBShow.loops(OFF);
            }
        } else {
            deck.loopsize = Math.pow(2, padindex);
            deck.RGBShow.loops(ON);
        }
    }
};

ReloopBeatpad.LoopSizeBtn = function(channel, control, value, status, group) {
    var isLoopActive = engine.getValue(group, "loop_enabled");
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        if (deck.loopkind == SIMPLE) {
            if (!isLoopActive) {
                // activate loop
                engine.setValue(group, "beatloop_" + deck.loopsize.toString() + "_activate", 1);
                deck.leds.Loop.onOff(ON);
                deck.RGBShow.loops(ON);
            } else {
                // deactivate loop
                engine.setValue(group, "reloop_exit", true);
                deck.leds.Loop.onOff(OFF);
                deck.RGBShow.loops(OFF);
            }
        } else {
            // activate roll
            engine.setValue(group, "beatlooproll_" + deck.loopsize.toString() + "_activate", 1);
            deck.leds.Loop.onOff(ON);
            deck.RGBShow.loops(ON);
        }
    } else {
        if (deck.loopkind == ROLL) {
            // deactivate roll
            engine.setValue(group, "beatlooproll_" + deck.loopsize.toString() + "_activate", 0);
            deck.leds.Loop.onOff(OFF);
            deck.RGBShow.loops(OFF);
        }
    }
};

ReloopBeatpad.sLoopSizeBtn = function(channel, control, value, status, group) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        // toggle loop mode into SIMPLE/ROLL
        deck.ToggleLoopKind();
    }
};

ReloopBeatpad.LoopSizeKnob = function(channel, control, value, status, group) {
    var isLoopActive = engine.getValue(group, "loop_enabled");
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    var knobValue = value - 0x40;

    if (isLoopActive) {
        if (control < 0x30) {
            if (knobValue > 0) {
                engine.setValue(group, "loop_move", 1);
            } else {
                engine.setValue(group, "loop_move", -1);
            }
        } else {
            if (knobValue > 0) {
                // Because loop_halve is supposed to be a pushbutton, we have to
                // fake the button-off event to clear out the "pressed" status.
                engine.setValue(group, "loop_double", 1);
                engine.setValue(group, "loop_double", 0);
                deck.loopsize *= 2;
            } else {
                engine.setValue(group, "loop_halve", 1);
                engine.setValue(group, "loop_halve", 0);
                deck.loopsize /= 2.0;
            }
        }
    }
};

ReloopBeatpad.SamplerBtn = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    if (value == DOWN) {
        if (deck.PadMode < SAMPLERMODE) {
            deck.PadMode = SAMPLERMODE;
        } else {
            // Cycle the Sampler Pads in three modes : SAMPLERMODE/ SAMPLERBANKSTATUSMODE / FXRACKSELECTMODE
            deck.PadMode += 1;
            if (deck.PadMode > FXRACKSELECTMODE) {
                deck.PadMode = SAMPLERMODE;
            }
            switch (deck.PadMode) {
            case SAMPLERMODE:
                print("Sampler mode");
                break;
            case SAMPLERBANKSTATUSMODE:
                print("Sampler bank status mode");
                break;
            case FXRACKSELECTMODE:
                print("FX Rack select mode");
                break;
            case LOOPMODESTATUS:
                print("Loop mode status");
                break;
            default:
                break;
        }
        }
        ReloopBeatpad.ShowSamplersAndEffects();
        ReloopBeatpad.samplers.LedsUpdate();
    }
};

ReloopBeatpad.SamplerPad = function(channel, control, value, status, group) {
    var padindex = control - ReloopBeatpad.MIDI.SamplerPad + 1;
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        switch (deck.PadMode) {
            case SAMPLERMODE:
                ReloopBeatpad.samplers.play(padindex, true);
                break;
            case SAMPLERBANKSTATUSMODE:
                ReloopBeatpad.samplers.LoadBank(deck, padindex);
                break;
            case FXRACKSELECTMODE:
                deck.SelectEffectRack(padindex);
                break;
            case LOOPMODESTATUS:
                deck.ToggleLoopKind();
                break;
            default:
                break;
        }
    }
};

ReloopBeatpad.sSamplerPad = function(channel, control, value, status, group) {
    var padindex = control - ReloopBeatpad.MIDI.SamplerPad + 1 - SHIFT;
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (value == DOWN) {
        switch (deck.PadMode) {
            case SAMPLERMODE:
                ReloopBeatpad.samplers.play(padindex, false);
                break;
            case SAMPLERBANKSTATUSMODE:
                ReloopBeatpad.samplers.LoadBank(deck, padindex);
                break;
            case FXRACKSELECTMODE:
                deck.SelectEffectRack(padindex);
                break;
            case LOOPMODESTATUS:
                deck.ToggleLoopKind();
                break;
            default:
                break;
        }
    }
};

ReloopBeatpad.FXSelectKnob = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    value = value - 0x40;
    var n = deck.CurrentEffectRack;
    engine.setValue("[EffectRack1_EffectUnit" + n + "]", "chain_selector", value);
};

// Does not work for the moment, not implemented in Mixxx
ReloopBeatpad.sFXSelectKnob = function(channel, control, value, status, group) {
    value = value - 0x40;
    if (value > 0) {
        // engine.setValue("[QuickEffectRack1]", "effect_selector", value);
    }
};

ReloopBeatpad.FXParam = function(channel, control, value, status, group) {
    // Super for EffectRack
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    engine.setValue("[EffectRack1_EffectUnit" + deck.CurrentEffectRack + "]", "super1", value / 128);
};

ReloopBeatpad.FXParamShift = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    engine.setValue("[EffectRack1_EffectUnit" + deck.CurrentEffectRack + "]", "mix", value / 128);
};

ReloopBeatpad.FilterMid = function(channel, control, value, status, group) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    engine.setValue("[QuickEffectRack1_" + group + "]", "super1_set_default", value);
    deck.leds.RimBlue.onOff(OFF);
    deck.RGBShow.filter(0);
    deck.filterligthshowstatus = 0;
};

ReloopBeatpad.FilterKnob = function(channel, control, value, status, group) {
    // Super for QuickEffectRack1_[Channel1]
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    var newstatus = (value > 0x40) ? 2 : 4;
    engine.setValue("[QuickEffectRack1_" + group + "]", "super1", value / 128);
    if (deck.filterligthshowstatus !== newstatus) {
        deck.leds.RimBlue.onOff(newstatus);
        if (deck.filterligthshowstatus === 0) {
            deck.RGBShow.filter(value - 0x40);
        }
    }
    deck.filterligthshowstatus = newstatus;
};

// =====================================================================
// LED Output callback functions
// =====================================================================

ReloopBeatpad.OnPlayIndicatorChange = function(value, group, control) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (!deck.Shifted) {
        deck.leds.play.onOff((value) ? ON : OFF);
    }
};

ReloopBeatpad.OnCuePointChange = function(value, group, control) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    if (!deck.Shifted) {
        deck.leds.Set.onOff((value > 0) ? ON : OFF);
        deck.leds.jump.onOff((value > 0) ? ON : OFF);
    }
};

ReloopBeatpad.OnSyncEnabledChange = function(value, group, control) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    deck.leds.sync.onOff((value) ? ON : OFF);
};

ReloopBeatpad.OnTrackLoaded = function(value, group, control) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    deck.leds.load.onOff((value) ? ON : OFF);
    var oldloaded = deck.loaded;
    deck.loaded = (value !== 0);
    if (oldloaded != deck.loaded) { // if this value changed we update the jog lights
        engine.trigger(group, "playposition");
    }
};

ReloopBeatpad.OnKeylock = function(value, group, control) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    var dispValue = (value) ? ON : OFF;
    deck.leds.keylock.onOff(dispValue); // calling only the light object is very important ! (infinite loop triggered)
};

ReloopBeatpad.OnVuMeterChange = function(value, group, control) {
    var deck = ReloopBeatpad.decks["D" + group.substring(8, 9)];
    var dispValue = Math.round(8.0 * value);
    deck.leds.VUMeter.onOff(dispValue);
};

ReloopBeatpad.OnPlaypositionChange = function(value, group, control) {
    // Rim Red : 1st behaviour 0x01-0x18 ; 2nd Behavior = 1st +24 ;3d behavior ON/OFF
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    if (deck.loaded) {
        deck.RGBShow.notloaded(false);
        deck.leds.start.onOff((value) ? OFF : ON);
        var timeremaining = RealDuration(group) * (1 - value);
        var trackwarning = 0;
        var ledindex = 0;
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
                var isLoopActive = engine.getValue(group, "loop_enabled");
                if (isLoopActive) { // loop_position clock
                    var ts = engine.getValue(group, "track_samples");
                    var loopstart = engine.getValue(group, "loop_start_position");
                    var loopend = engine.getValue(group, "loop_end_position");
                    var loop_position = (value * ts - loopstart) / (loopend - loopstart);
                    ledindex = Math.round(23.0 * loop_position) + 1;
                    deck.leds.RimRed.onOff(ledindex);
                } else {
                    if (deck.JogScratchStatus) { // Spinny
                        var revolutions = value * RealDuration(group) / 1.8; //33+1/3 rev/mn=1.8 s/rev
                        var needle = intpart( (revolutions - intpart(revolutions)) * 24);
                        ledindex = Math.floor(needle) + 1;
                        deck.leds.RimRed.onOff(ledindex);
                    } else if (deck.JogSeekStatus) { // Track position
                        // Track position/elapsed time
                        ledindex = Math.round(24.0 * value);
                        if (ledindex !== 0) {
                            ledindex += 24;
                        }
                        deck.leds.RimRed.onOff(ledindex);
                    }
                }
                break;

            case 1: // if less than 30 seconds before end of track : flashing slowly
                if (deck.leds.RimRed.getFlashDuration() !== 1000) {
                    deck.leds.RimRed.flashOn(1000, ON, 1000);
                }
                break;

            case 2: // if less than 10 seconds before end of track : flashing fast
                if (deck.leds.RimRed.getFlashDuration() !== 300) {
                    deck.leds.RimRed.flashOn(300, ON, 300);
                }
                break;

            case 3: // end of track : full ring lit
                deck.leds.RimRed.onOff(ON);
                break;
            default:
                break;
        }
    } else {
        deck.RGBShow.notloaded(true);
    }
};

ReloopBeatpad.OnBeatActive = function(value, group, control) {
    // OnPlayPosition n°2 !!!
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    if (!(deck.JogScratchStatus || deck.JogSeekStatus)) {
        if (value == 1) {
            var timeremaining = RealDuration(group) * (1 - engine.getValue(group, "playposition"));
            if ((timeremaining <= 30) && (TrackEndWarning)) { // flashing end of track
                engine.trigger(group, "playposition");
            } else {
                deck.leds.RimRed.onOff(deck.beatpos * 3 + 1);
                deck.beatpos += 1;
                if (deck.beatpos >= 8) {
                    deck.beatpos = 0;
                }
            }
        }
    }
};

ReloopBeatpad.OnRecordingStatusChange = function(value, group, control) {
    ReloopBeatpad.recordingled.onOff((value) ? ON : OFF);
};

ReloopBeatpad.OnPFLStatusChange = function(value, group, control) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    deck.leds.pfl.onOff((value) ? ON : OFF);
};

ReloopBeatpad.OnDurationChange = function(value, group, control) {
    engine.trigger(group, "playposition");
};

ReloopBeatpad.OnHotcuePositionChange = function(value, group, control, padindex) {
    var decknum = parseInt(group.substring(8,9));
    var deck = ReloopBeatpad.decks["D" + decknum];
    deck.leds["CuePad" + padindex].onOff((value == -1) ? OFF : ON);
    deck.leds["sCuePad" + padindex].onOff((value == -1) ? OFF : ON);
};

ReloopBeatpad.OnEffectLoaded = function(value, group, control, index) {
    var i;
    var parameterlinkedcount = 0;
    var linktype = 0;
    if (value) {
        var numparams = engine.getValue("[EffectRack1_EffectUnit" + index + "_Effect1]", "num_parameters");
        if (numparams > 0) {
            for (i = 1; i <= numparams; i++) {
                linktype = engine.getValue("[EffectRack1_EffectUnit" + index + "_Effect1]", "parameter" + i + "_link_type");
                if (linktype !== 0) {
                    parameterlinkedcount += 1;
                }
            }
        }
        if (parameterlinkedcount === 0) {
            for (i = 1; i <= numparams; i++) {
                engine.setValue("[EffectRack1_EffectUnit" + index + "_Effect1]", "parameter" + i + "_link_type", 1);
            }

        }
    }
};

ReloopBeatpad.OnSampleLoaded = function(value, group, control) {
    var samplenum = parseInt(group.replace(/[^0-9\.]/g, ''), 10);
    ReloopBeatpad.samplers.SetLoaded(samplenum, (value !== 0));
};

ReloopBeatpad.OnSamplePlayStop = function(value, group, control) {
    var samplenum = parseInt(group.replace(/[^0-9\.]/g, ''), 10);
    ReloopBeatpad.samplers.LedUpdateSampler(samplenum);
};

ReloopBeatpad.OnBankLoaded = function(value, group, control, index) {
    if (value == 1) {
        ReloopBeatpad.samplers.bankactive = index;
        ReloopBeatpad.samplers.LedsUpdate();
    }
};

// =====================================================================
// Sysex messages handling
// =====================================================================
ReloopBeatpad.InboundSysex = function(data, length) {
    //for testing
    var s = "";
    var i;
    for(i=0; i<length; i++) {
        s = s + ("00"+data[i].toString(16)).substr(-2);
    }
    /******************************************************************
     * Reloop Beatpad gives : F0 7E 00 06 02 00 20 6E 26 2D 65 22 00 00 00 21 F7
     * The SysEx answer for an Identity Request should start with
     * " F07E??0602 " (0xF0, 0x7E, <channel>, 0x06, 0x02),
     * then the Manufacturer's ID (on 3 bytes, see some examples here),
     * followed by the Device Family Code, the Device Family Member Code,
     * the Software Revision Level, ..
     *
     * Manufacturer's ID : 00 20 6E (Ya Horng Electronic Co LTD)
     * Device Family Code : 26 2D (USB VID = family code; 26 2D =Reloop )
     * Device Family Member Code :  65 22 (USB PID = model number; 6522 = Beatpad)
     * Software Revision Level :  00 00 00 21 (firmware 0.21)
     * ****************************************************************/

    if (s=="f07e00060200206e262d652200000021f7") {
         print("Manufacturer : Ya Horng Electronic Co LTD");
         print("Vendor...... : Reloop");
         print("Model....... : Beatpad");
         print("firmware ... : v0.21");
         print("Welcome !");
    } else if (s=="f07e00060200206e262d652200000018f7") {
         print("Manufacturer : Ya Horng Electronic Co LTD");
         print("Vendor...... : Reloop");
         print("Model....... : Beatpad");
         print("firmware ... : v0.18");
         print("Please update the firmware of your controller");
         print("to the latest version (0.21)");
    } else {
        print("Your controller won't work with this mapping, designed");
        print("for the Reloop Beatpad.");
    }
    print();
    print(ReloopBeatpad.id + " initialized.");
    print();
    print("3/3 : Mixxx initialization");
    print("==========================");
    print("Request Beatpad Status by Mixxx.. (controller is ready) :");
    // Automatically done by Mixxx :
    // midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
};

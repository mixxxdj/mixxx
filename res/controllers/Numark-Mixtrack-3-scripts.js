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

// PAD Loop button behavior: "true": Loop stops when finger release. 
//"false" will force start loop on press and stop on 2nd press
var PADLoopButtonHold = false;

// PAD Sample button behavior:"true": Sampler stops when finger release. 
//"false" will force start Sampler on press and stop on 2nd press
var PADSampleButtonHold = false; 

// LED Flash on Beat Active : "true": TAP LED will flash to the beat
// if Shift Lock is enable TAP LED will remain ON
var OnBeatActiveFlash = true; 

// If Dark Metal Skin is used, set this to true: 
// this is required to expand library view with DM skin.
var DarkMetalSkin = false;

// Use Beat knob to adjust Sampler Volume. 
// If "true": Deck 1 adjusts Samplers 1-4 ; Deck 2 adjusts Samplers 5-8 
// Shift + Beat knob moves beat grid 
// If false; beat knob will adjust beatgrid, shift + knob will adjust grid size
var BeatKnobAsSamplerVolume = true;

//Disable Play on Sync button Double Press
var noPlayOnSyncDoublePress = false; 

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
    rpm = 33 + 1 / 3,  //Like a real vinyl !!! :)
    alpha = 1.0 / 8,   //Adjust to suit.
    beta = alpha / 32; //Adjust to suit.

/************************  GPL v2 licence  *****************************
 * Numark Mixtrack Pro 3 controller script
 * Author: Stéphane Morin, largely based on script from Chloé AVRILLON (DJ Chloé)
 *
 * Key features
 * ------------
 * - ICUT effect for scratching
 * - Fader Start
 * - press/double press/long press handling
 * - Smart PFL
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
 * 2016-01-12 (0.9) - Chloé AVRILLON
 *            - Initial revision for Mixxx 2.0+
 *            - GPL v2 licence, rework of this header, JSHint.com quality check,
 *              a few comments, minor changes, typos
 *            - Make some code reusable (lights : LED object; Special Buttons, iCUT)
 * 2016-01-14 (1.0 beta 1) - Chloé AVRILLON
 *            - Fixed Pitch Bend Button
 *            - Fixed Syntax error at line 1625, column 1
 *            - Fixed line 907 
 *            - Samplers management integration
 * 2016-01-14 (1.0 beta 2) - Chloé AVRILLON
 *            - Debugging session working with Emulator Pro as a midi controller
 *              (moved midi.sendmessage calls to a util function 
 *              easy to comment/uncomment
 *              to avoid midi messages looping over in Mixx)
 * 2016-01-15 (1.0 beta 3) - Chloé AVRILLON
 *            - Fixed bug line 1963
 *            - Pitch bend buttons and strip were not bind
 *              to the javascript (<script-binding/>)
 *2016-02-17 (1.0 beta 4) - Stéphane Morin
 *            - rewrite of wheel functions (wheel move and wheel touch)
 *            - ensured Padmode LEDs are lit
 *            - added global variables for preferences
 *            - added print statement to aid troubleshooting (these could be removed later)
 *            - Broke sync LEDs... they don't light up anymore.. can someone fix this?
 *2016-02-18 (1.0 beta 5) - Stéphane Morin
 *            - Sync LEDs fixed
 *2016-02-23 (1.0 beta 6) - Stéphane Morin
 *            - AutoLoop fixed, including LEDs management
 *            - Samplers fixed, including LEDs management
 *            - Added Smart PFL
 *2016-02-25 (1.0 beta 7) - Stéphane Morin
 *            - Faderstart corrected
 *            - Implement PADLoopButtonPressed 
 *2016-02-25 (1.0 beta 8) - Stéphane Morin
 *            - replaced a script function (script.deckFromGroup) by 
 *              (NumarkMixtrack3.deckFromGroup)
 *              a portion of script.deckFromGroup is commented in the common file 
 *              and not usable for this script sampler implementation
 *2016-03-07 (1.0 ) - Stéphane Morin - https://github.com/mixxxdj/mixxx/pull/905
 *            - Code Clean up 
 *            - Add Maximize Library function to TAP button
 *            - Added function: Sampler + Shift Key: play sample with NO Sync
 *            - Fixed Super Effect button
 *            - Fixed Sampler Shift - Sync now removed if present
 *2016-03-07 (1.1 ) - Stéphane Morin - https://github.com/mixxxdj/mixxx/pull/905
 *            - Corrected Pitch Bend rate of wheel for smoother operation
 *            - Add option (noPlayOnSyncDoublePress) to disable Play on Double press of Sync button
 *2016-04-08 (1.2 ) - Stéphane Morin - https://github.com/mixxxdj/mixxx/pull/905
 *            - Renamed user options: PADLoopButtonPressed to PADLoopButtonHold 
 *            - Renamed user options: PADSampleButtonPressed to PADSampleButtonHold
 *            - TapExpandLibrary moved from Tap button to Browse Button push
 *            - Linked printComments to debug value of init function: The debugging parameter is set to 'true' 
 *              if the user specified the --mididebug parameter on the command line 
 *            - Cleaned NumarkMixtrack3.shutdown function
 *2016-04-08 (1.3 ) - Stéphane Morin - https://github.com/mixxxdj/mixxx/pull/905
 *            - changed skin option to use boolean for 
 *              DarkMetalSkin. It requires different code to expand library view.
 *            - removed trailing empty lines at end of script
 *            - line 1749, change .75 to 0.75 in var gammaOutputRange 
 *            - added spacing in numerous place for easier reading
 *
 * To do - (maybe..)
 * ----------------
 *            - Add script to control "volume","filterHigh","filterMid","filterLow" and"crossfader"
 *              In order for engine.softTakeover to work for these controls
 *            - Add configuration option: enable Samplers Sync by default (True/False) default: 
 *              false
 *            - Add brake effect when pausing track with Play button
 *            - Add 4 deck support for script
 *            - Allow Beat knobs to control FX parameter; this button is Binary.
 *              knob has 20 notches - it sends only 1 or 127 as output.
 *              Note to developer: I considered hard using the beat knob to adjust parameterK 
 *              (Deck 1: 1st parameter of effect 1; Deck 2: 2nd parameter of effect 1) for 
 *              units 1 to 3, by selecting the target effect unit using LONG_PRESS + FX buttons 
 *              but since all but one of the 8 effects have more than 2 parameters, 
 *              I could not convince myself of the added value, since you still 
 *              need to adjust effect on the GUI.
 *
 ***********************************************************************
 *                           GPL v2 licence
 *                           -------------- 
 * Numark Mixtrack Pro 3 controller script 1.0 beta 7 for Mixxx 2.0+
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
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////

// Line below to remove warnings from JSHINT page regarding dot notation
/*jshint sub:true*/

function NumarkMixtrack3() {}

NumarkMixtrack3(); // Very important ! Initializes some reusable objects.

// Array of Objects can be created
NumarkMixtrack3.group = "[Master]";
NumarkMixtrack3.decknum = 0;
NumarkMixtrack3.decks = [];

// Global constants/variables
var ON = 0x7F, OFF = 0x00, DOWN = 0x7F;
var QUICK_PRESS = 1, DOUBLE_PRESS = 2, LONG_PRESS = 3;

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
    "black" : 0,
    "blue"  : 32,
    "yellow": 96,
    "purple": 127
};

// Utilities
// =====================================================================

function printInfo(string) {
    if (NumarkMixtrack3.debug) print(string);
}

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

function toggleValue(group,key) { 
    engine.setValue(group,key,!engine.getValue(group,key));
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
        var d2 = engine.getValue(group, "track_samples") / engine.getValue(group, "track_samplerate");

        if (d1===d2) { 
            //it is mono
            return d2;
        } else {
            if  ( (d1 > Math.floor(d2)) && (d1 < Math.ceil(d2)) ) {
                //It is mono
                return d2;
            } else { 
                //It is stereo
                return d2/2;
            }
        }
    }
}

function sendShortMsg(control,midino,value) {   
    midi.sendShortMsg(control, midino, value);
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
        
        this.flashTimer = engine.beginTimer( num_ms_on + num_ms_off, function() { myself.flashOnceOn(false); } );
    }
    if (flashCount > 1) {
        // flashcount>0 , means temporary flash, first flash already done,
        // so we don't need this part  if flashcount=1
        // temporary timer. The end of this timer stops the permanent flashing
        
        this.flashTimer2 = engine.beginTimer(flashCount * (num_ms_on + num_ms_off) - num_ms_off, function() { myself.Stopflash(relight); }, true);
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
    this.flashOnceTimer = engine.beginTimer(this.num_ms_on - scriptpause, function() { myself.flashOnceOff(relight); }, true);
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
                              function() { myself.ButtonDecide(); }, true);
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
    this.ButtonLongPressTimer = engine.beginTimer(this.LongPressThreshold, function() { myself.ButtonAssertLongPress(); }, true);
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
//                      the button or value=UP because you have realeased
//                      the button only once before it becomes a long press).
// LongPressThreshold : delay in ms above which a firts press on the
//                      button will be considered as a Long press (default = 500ms).
// DoublePressTimeOut : delay in ms above wich a second press on the
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
        //take action
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
    
    if (this.ButtonCount === 0) { //first press (inits)
        // 1st press
        this.ButtonCount = 1;
        // and short press
        this.ButtonLongPress = false;
        this.ButtonLongPressTimer =
            engine.beginTimer(this.LongPressThreshold,
                              function() { myself.ButtonAssertLongPress(); },
                              true);
        this.ButtonTimer =
            engine.beginTimer(this.DoublePressTimeOut,
                              function() { myself.ButtonAssert1Press(); },
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
            this.ButtonTimer = 0 ;
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
        this.Callback(this.channel, this.control, this.value, this.status, this.group, LONG_PRESS);
    } else {
        if (this.ButtonCount === 2) {
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
// this mode simulates a scratch routine. When the jog wheel is turned back
// the crossfader closes, when the jog wheel is turned forward the crossfader
// will open.

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
        if (this.deckNum === 1) {
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
// Your controller is a "Model A" controller for scratching,
// if it centers on 0.
// See http://www.mixxx.org/wiki/doku.php/midi_scripting#scratching

var Jogger = function (group, deckNum, model) {
    this.deckNum = deckNum;
    this.group = group;
    this.wheelTouchInertiaTimer = 0;
    this.iCUT = new AutoCut(deckNum);
    this.model = model;
};

NumarkMixtrack3.SamplerBank = function() {
    this.bankactive = 1;
    this.loaded = [];
    this.loaded.length = 17;
};

//Sample action
NumarkMixtrack3.SamplerBank.prototype.play = function(samplerindex,value) {
    
    var deck = NumarkMixtrack3.decks["D" + samplerindex];
    var isplaying = engine.getValue("[Sampler" + samplerindex + "]", "play");
    
    if (value) {
        if (!isplaying) {
            if (deck.shiftKey) {
                //Shift is on, play sampler with no Sync
                engine.setValue("[Sampler" + samplerindex + "]", "beatsync", 0);
                engine.setValue("[Sampler" + samplerindex + "]", "cue_gotoandplay", 1);
                deck.LEDs["PADsampler" + samplerindex].flashOn(300, PADcolors.purple, 300);             
            } else {
                //play sampler with Sync
                engine.setValue("[Sampler" + samplerindex + "]", "cue_gotoandplay", 1);
                engine.setValue("[Sampler" + samplerindex + "]", "beatsync", 1);
                deck.LEDs["PADsampler" + samplerindex].flashOn(300, PADcolors.purple, 300);
            }
        } else {
            engine.setValue("[Sampler" + samplerindex + "]", "stop", 1);
            NumarkMixtrack3.decks["D"+ samplerindex].LEDs["PADsampler"+ samplerindex].onOff(ON);
        }
    } else {
        engine.setValue("[Sampler" + samplerindex + "]", "stop", 1);
        NumarkMixtrack3.decks["D"+ samplerindex].LEDs["PADsampler"+ samplerindex].onOff(ON);
    }
};

// ******************************************************************
// Samplers - create object
// *********

NumarkMixtrack3.samplers = new NumarkMixtrack3.SamplerBank();

// ******************************************************************
// Decks
// *********
NumarkMixtrack3.deck = function(deckNum) {
    this.deckNum = deckNum;
    this.group = "[Channel" + deckNum + "]";
    this.loaded = false;
    this.LoadInitiated = false;
    this.jogWheelsInScratchMode = false;
    this.PADMode = 0; //0=Manual Loop;1=Auto Loop; 2=Sampler ??? // Needed?
    this.shiftKey = false;
    this.shiftLock = false;
    this.touch = false;
    this.faderstart = false;
    this.PitchFaderHigh = 0;
    this.lastfadervalue = 0;
    this.scratchTimer = 0;
    this.seekingfast = false;
    this.iCutStatus = false;    
    this.LEDs=[];
    // NMTP3 is a "Model A" controller for scratching, it centers on 0.
    // See http://www.mixxx.org/wiki/doku.php/midi_scripting#scratching
    // and see "Jogger" object constructor
    this.Jog = new Jogger(this.group, this.deckNum, "A");
};

NumarkMixtrack3.deck.prototype.TrackIsLoaded = function() {
    return TrackIsLoaded(this.group);
};

NumarkMixtrack3.deck.prototype.StripEffect = function(value, decknum) {
    // var decknum = NumarkMixtrack3.deckFromGroup(group);
    // var deck = NumarkMixtrack3.decks["D" + decknum];
    
    if (decknum === 1) {
        engine.setValue("[EffectRack1_EffectUnit1]", "super1", value/127);
        engine.setValue("[EffectRack1_EffectUnit2]", "super1", value/127);
        engine.setValue("[EffectRack1_EffectUnit3]", "super1", value/127);
    } else {
        engine.setValue("[EffectRack1_EffectUnit1]", "mix", value/127);
        engine.setValue("[EffectRack1_EffectUnit2]", "mix", value/127);
        engine.setValue("[EffectRack1_EffectUnit3]", "mix", value/127);
    }
};

// =====================================================================
// Initialization of the mapping
// =====================================================================
// Create decks

// not completely clean... D1 and D2 are for the actual decks, D1 to D8 are for samplers.
// this will need to be fixed if someone is to enable the ability to map Deck 3 and 4
NumarkMixtrack3.decks.D1 = new NumarkMixtrack3.deck("1");
NumarkMixtrack3.decks.D2 = new NumarkMixtrack3.deck("2");
NumarkMixtrack3.decks.D3 = new NumarkMixtrack3.deck("3");
NumarkMixtrack3.decks.D4 = new NumarkMixtrack3.deck("4");
NumarkMixtrack3.decks.D5 = new NumarkMixtrack3.deck("5");
NumarkMixtrack3.decks.D6 = new NumarkMixtrack3.deck("6");
NumarkMixtrack3.decks.D7 = new NumarkMixtrack3.deck("7");
NumarkMixtrack3.decks.D8 = new NumarkMixtrack3.deck("8");


NumarkMixtrack3.initLEDsObjects = function() {
    var i;
    // Lets create some LEDs
    NumarkMixtrack3.AllLeds =
        new LED(0x90+ledCategories.master, leds.all);
    NumarkMixtrack3.decks["D1"].LEDs.PADsampler1 =
        new LED(0x91,leds.PADsampler1);
    NumarkMixtrack3.decks["D2"].LEDs.PADsampler2 =
        new LED(0x91,leds.PADsampler2);
    NumarkMixtrack3.decks["D3"].LEDs.PADsampler3 =
        new LED(0x91,leds.PADsampler3);
    NumarkMixtrack3.decks["D4"].LEDs.PADsampler4 =
        new LED(0x91,leds.PADsampler4);
    NumarkMixtrack3.decks["D5"].LEDs.PADsampler5 =
        new LED(0x92,leds.PADsampler5);
    NumarkMixtrack3.decks["D6"].LEDs.PADsampler6 =
        new LED(0x92,leds.PADsampler6);
    NumarkMixtrack3.decks["D7"].LEDs.PADsampler7 =
        new LED(0x92,leds.PADsampler7);
    NumarkMixtrack3.decks["D8"].LEDs.PADsampler8=
        new LED(0x92,leds.PADsampler8); 
        
        
    for (i=1;i<=2;i++) {
        NumarkMixtrack3.decks["D"+i].LEDs.headphones =
            new LED(0x90+ledCategories.master,leds.headphones1-1+i);
        NumarkMixtrack3.decks["D"+i].LEDs.jogWheelsInScratchMode =
            new LED(0x90+i,leds.jogWheelsInScratchMode);
        NumarkMixtrack3.decks["D"+i].LEDs.loopin =
            new LED(0x90+i,leds.loopin);
        NumarkMixtrack3.decks["D"+i].LEDs.loopout =
            new LED(0x90+i,leds.loopout);
        NumarkMixtrack3.decks["D"+i].LEDs.reloop_exit =
            new LED(0x90+i,leds.reloop_exit);
        NumarkMixtrack3.decks["D"+i].LEDs.loop_halve =
            new LED(0x90+i,leds.loop_halve);
        NumarkMixtrack3.decks["D"+i].LEDs.hotCue1 =
            new LED(0x90+i,leds.hotCue1);
        NumarkMixtrack3.decks["D"+i].LEDs.hotCue2 =
            new LED(0x90+i,leds.hotCue2);
        NumarkMixtrack3.decks["D"+i].LEDs.hotCue3 =
            new LED(0x90+i,leds.hotCue3);
        NumarkMixtrack3.decks["D"+i].LEDs.hotCue4 =
            new LED(0x90+i,leds.hotCue4);
        NumarkMixtrack3.decks["D"+i].LEDs.Cue =
            new LED(0x90+i,leds.Cue);
        NumarkMixtrack3.decks["D"+i].LEDs.sync =
            new LED(0x90+i,leds.sync);
        NumarkMixtrack3.decks["D"+i].LEDs.play =
            new LED(0x90+i,leds.play);
        NumarkMixtrack3.decks["D"+i].LEDs.fx1 =
            new LED(0x90+i,leds.fx1);
        NumarkMixtrack3.decks["D"+i].LEDs.fx2 =
            new LED(0x90+i,leds.fx2);
        NumarkMixtrack3.decks["D"+i].LEDs.fx3 =
            new LED(0x90+i,leds.fx3);
        NumarkMixtrack3.decks["D"+i].LEDs.tap =
            new LED(0x90+i,leds.tap);
        NumarkMixtrack3.decks["D"+i].LEDs.PADloop1 =
            new LED(0x90+i,leds.PADloop1);
        NumarkMixtrack3.decks["D"+i].LEDs.PADloop2 =
            new LED(0x90+i,leds.PADloop2);
        NumarkMixtrack3.decks["D"+i].LEDs.PADloop3 =
            new LED(0x90+i,leds.PADloop3);
        NumarkMixtrack3.decks["D"+i].LEDs.PADloop4 =
            new LED(0x90+i,leds.PADloop4);
    
        NumarkMixtrack3.decks["D"+i].LEDs.meter =
            new LED(0x90+ledCategories.meters,leds.meter1-1+i);
    }
};

NumarkMixtrack3.initButtonsObjects = function() {
    var i;
    for (i=1;i<=2;i++) {
        NumarkMixtrack3.decks["D"+i].LoadButtonControl =
            new LongShortBtn(NumarkMixtrack3.OnLoadButton);
        NumarkMixtrack3.decks["D"+i].SyncButtonControl =
            new LongShortDoubleBtn(NumarkMixtrack3.OnSyncButton);
        NumarkMixtrack3.decks["D"+i].ShiftButtonControl =
            new SingleDoubleBtn(NumarkMixtrack3.OnShiftButton);
        NumarkMixtrack3.decks["D"+i].ShiftedPFLButtonControl =
            new SingleDoubleBtn(NumarkMixtrack3.OnShiftedPFLButton);
    }
};

// Called when the MIDI device is opened & set up
NumarkMixtrack3.init = function(id,debug) { 
    print("********* Initialisation process engaged *****************");
    print("   Mapping initialization");
    print("============================");

    var i,j,k;
    NumarkMixtrack3.id = id; // Store the ID of this device for later use
    NumarkMixtrack3.debug = debug;
    //print comments on prompt screen in order to facilitate debugging
    print ("   Debug and printComments setting: " + NumarkMixtrack3.debug);

    NumarkMixtrack3.libraryMode = false;

    print("   Init LEDs");
    NumarkMixtrack3.initLEDsObjects();
    print("   Init Buttons");
    NumarkMixtrack3.initButtonsObjects();

    // Turn ON all the lights: the only way PADMode Leds light up 
    NumarkMixtrack3.AllLeds.onOff(ON);
    // Initialise some others (PAD LEDs)
    for (i=1;i<=2;i++) {
        for (j=1;j<=4;j++) {
            NumarkMixtrack3.decks["D"+i].LEDs["PADloop"+j].onOff(PADcolors.black);
        }
    for (k=1;k<=8;k++) { 
        NumarkMixtrack3.decks["D"+k].LEDs["PADsampler"+k].onOff(PADcolors.black);
        }
        NumarkMixtrack3.decks["D"+i].LEDs.jogWheelsInScratchMode.onOff(OFF);
        NumarkMixtrack3.decks["D"+i].LEDs.headphones.onOff(OFF);
        NumarkMixtrack3.decks["D"+i].LEDs.loopin.onOff(OFF);
        NumarkMixtrack3.decks["D"+i].LEDs.loopout.onOff(OFF);
        NumarkMixtrack3.decks["D"+i].LEDs.reloop_exit.onOff(OFF);
        NumarkMixtrack3.decks["D"+i].LEDs.loop_halve.onOff(OFF);
        NumarkMixtrack3.decks["D"+i].LEDs.hotCue1.onOff(OFF);       
        NumarkMixtrack3.decks["D"+i].LEDs.hotCue2.onOff(OFF);       
        NumarkMixtrack3.decks["D"+i].LEDs.hotCue3.onOff(OFF);       
        NumarkMixtrack3.decks["D"+i].LEDs.hotCue4.onOff(OFF);           
        NumarkMixtrack3.decks["D"+i].LEDs.Cue.onOff(OFF);   
        NumarkMixtrack3.decks["D"+i].LEDs.sync.onOff(OFF);  
        NumarkMixtrack3.decks["D"+i].LEDs.play.onOff(OFF);  
        NumarkMixtrack3.decks["D"+i].LEDs.fx1.onOff(OFF);   
        NumarkMixtrack3.decks["D"+i].LEDs.fx2.onOff(OFF);       
        NumarkMixtrack3.decks["D"+i].LEDs.fx3.onOff(OFF);
        NumarkMixtrack3.decks["D"+i].LEDs.tap.onOff(OFF);
        NumarkMixtrack3.decks["D"+i].LEDs.meter.onOff(OFF);

        print("   set LEDs state "+"D"+i);
    }
    
    print("   Init Soft Takeovers");
    // Enable soft-takeover for Pitch slider
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);
    
    // Set soft-takeover for all Sampler volumes
    for (i = engine.getValue("[Master]", "num_samplers"); i >= 1; i--) {
        engine.softTakeover("[Sampler" + i + "]", "pregain", true);
    }
    // Set soft-takeover for all applicable Deck controls
    // the following controls are mapped in the XML file, not by script
    // therefore this is not required
    
    /*  
    for (i = engine.getValue("[Master]", "num_decks"); i >= 1; i--) {
        engine.softTakeover("[Channel" + i + "]", "volume", true);
        engine.softTakeover("[Channel" + i + "]", "filterHigh", true);
        engine.softTakeover("[Channel" + i + "]", "filterMid", true);
        engine.softTakeover("[Channel" + i + "]", "filterLow", true);
    } 

    engine.softTakeover("[Master]", "crossfader", true);
    */
    for (i = 1; i <= 4; i++) {
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "]", "super1", true);
    }
    
    for (i = 1; i <= 4; i++) {
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "]", "mix", true);
    }
    
    print("   Init Connect controls");
    // Add event listeners
    engine.connectControl("[Channel1]", "hotcue_1_enabled", function(value, group, control) {
        NumarkMixtrack3.OnHotcueChange(value, group, control, 1);
    });
    engine.connectControl("[Channel2]", "hotcue_1_enabled", function(value, group, control) {
        NumarkMixtrack3.OnHotcueChange(value, group, control, 1);
    });
    engine.connectControl("[Channel1]", "hotcue_2_enabled", function(value, group, control) {
        NumarkMixtrack3.OnHotcueChange(value, group, control, 2);
    });
    engine.connectControl("[Channel2]", "hotcue_2_enabled", function(value, group, control) {
        NumarkMixtrack3.OnHotcueChange(value, group, control, 2);
    });
    engine.connectControl("[Channel1]", "hotcue_3_enabled", function(value, group, control) {
        NumarkMixtrack3.OnHotcueChange(value, group, control, 3);
    });
    engine.connectControl("[Channel2]", "hotcue_3_enabled", function(value, group, control) {
        NumarkMixtrack3.OnHotcueChange(value, group, control, 3);
    });
    engine.connectControl("[Channel1]", "hotcue_4_enabled", function(value, group, control) {
        NumarkMixtrack3.OnHotcueChange(value, group, control, 4);
    });
    engine.connectControl("[Channel2]", "hotcue_4_enabled", function(value, group, control) {
        NumarkMixtrack3.OnHotcueChange(value, group, control, 4);
    });
    
    // The track_samples control is being used to tell if a track has successfully loaded
    engine.connectControl("[Channel1]", "track_samples", "NumarkMixtrack3.OnTrackLoaded");
    engine.connectControl("[Channel2]", "track_samples", "NumarkMixtrack3.OnTrackLoaded");

    // VU Meters
    engine.connectControl("[Channel1]", "VuMeter", "NumarkMixtrack3.OnVuMeterChange");
    engine.connectControl("[Channel2]", "VuMeter", "NumarkMixtrack3.OnVuMeterChange");
    
    //other lights
    engine.connectControl("[Channel1]", "playposition", "NumarkMixtrack3.OnPlaypositionChange");
    engine.connectControl("[Channel2]", "playposition", "NumarkMixtrack3.OnPlaypositionChange");
    engine.connectControl("[Channel1]", "volume", "NumarkMixtrack3.OnVolumeChange");
    engine.connectControl("[Channel2]", "volume", "NumarkMixtrack3.OnVolumeChange");
    engine.connectControl("[Channel1]", "pfl", "NumarkMixtrack3.OnPFLStatusChange");
    engine.connectControl("[Channel2]", "pfl", "NumarkMixtrack3.OnPFLStatusChange");
    engine.connectControl("[Channel1]", "play_indicator", "NumarkMixtrack3.OnPlayIndicatorChange");
    engine.connectControl("[Channel2]", "play_indicator", "NumarkMixtrack3.OnPlayIndicatorChange");
    engine.connectControl("[Channel1]", "beat_active", "NumarkMixtrack3.OnBeatActive");
    engine.connectControl("[Channel2]", "beat_active", "NumarkMixtrack3.OnBeatActive");
    engine.connectControl("[Channel1]", "cue_indicator", "NumarkMixtrack3.OnCuePointChange");
    engine.connectControl("[Channel2]", "cue_indicator", "NumarkMixtrack3.OnCuePointChange");
    engine.connectControl("[Channel1]", "loop_start_position", "NumarkMixtrack3.OnLoopInOutChange");
    engine.connectControl("[Channel2]", "loop_start_position", "NumarkMixtrack3.OnLoopInOutChange");
    engine.connectControl("[Channel1]", "loop_end_position", "NumarkMixtrack3.OnLoopInOutChange");
    engine.connectControl("[Channel2]", "loop_end_position", "NumarkMixtrack3.OnLoopInOutChange");
    engine.connectControl("[Channel1]", "loop_enabled", "NumarkMixtrack3.OnLoopInOutChange");
    engine.connectControl("[Channel2]", "loop_enabled", "NumarkMixtrack3.OnLoopInOutChange");
    engine.connectControl("[Channel1]", "sync_enabled", "NumarkMixtrack3.OnSyncButtonChange");
    engine.connectControl("[Channel2]", "sync_enabled", "NumarkMixtrack3.OnSyncButtonChange");
    
    var loopsize = [0.125, 0.25, 0.5, 1, 2, 4, 8, 16];
    var l;
        for (l=0;l<loopsize.length;l++) {  
        engine.connectControl("[Channel1]", "beatloop_" + loopsize[l] + "_enabled", "NumarkMixtrack3.OnPADLoopButtonChange");
        } 

        for (k=0;k<loopsize.length;k++) {  
        engine.connectControl("[Channel2]", "beatloop_" + loopsize[k] + "_enabled", "NumarkMixtrack3.OnPADLoopButtonChange");
        }
    
    engine.connectControl("[EffectRack1_EffectUnit1_Effect1]", "loaded", function(value, group, control) {
        NumarkMixtrack3.OnEffectLoaded(value, group, control, 1);
    });
    engine.connectControl("[EffectRack1_EffectUnit2_Effect1]", "loaded", function(value, group, control) {
        NumarkMixtrack3.OnEffectLoaded(value, group, control, 2);
    });
    engine.connectControl("[EffectRack1_EffectUnit3_Effect1]", "loaded", function(value, group, control) {
        NumarkMixtrack3.OnEffectLoaded(value, group, control, 3);
    });
        
    for (i = 1; i <= 8; i++) {
          engine.connectControl("[Sampler" + i + "]", "play", "NumarkMixtrack3.OnSamplePlayStop");    
    }

    // check if there is already something loaded on each deck (when script reinitialize)
    print("   Init triggers");
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

    var m;
        for (m=0;m<loopsize.length;m++) {  
        engine.trigger("[Channel1]", "beatloop_" + loopsize[m] + "_enabled");
        }
    var n;
        for (n=0;n<loopsize.length;n++) {  
        engine.trigger("[Channel2]", "beatloop_" + loopsize[n] + "_enabled");
        }   
    
    for (i = 1; i <= 4; i++) {
        engine.trigger("[Channel1]", "hotcue_" + i + "_position");
        engine.trigger("[Channel2]", "hotcue_" + i + "_position");
    }

    for (i = 1; i <= 8; i++) {
        engine.trigger("[Sampler" + i + "]", "play");
    }

    print("Controller is ready");
    print("********* End of Initialisation process *****************");
};

NumarkMixtrack3.shutdown = function() { 
    print("********* Starting Controller Shutdown ********** ");
    print("           Turning off LEDs");
    NumarkMixtrack3.AllLeds.onOff(OFF);  
    print("********* Controller shutdown completed********* ");
    
};

/******************     Shift Button :
 * - Press                              : Temporary SHIFT
 * - Double press (like a double click) : SHIFT Lock enable
 * - Press and release                  : toggle off SHIFT Lock
 * *********************************************************************/
NumarkMixtrack3.ShiftButton = function(channel, control, value, status,
    group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (value === DOWN) {
        deck.shiftLock = false;
        deck.shiftKey = true;       
        deck.ShiftButtonControl.ButtonDown(channel, control, value, status, group);
    } else {
        deck.shiftKey = deck.shiftLock;
    }
};

// Callback for the SHIFT Button
NumarkMixtrack3.OnShiftButton = function(channel, control, value, status, group, eventkind) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (eventkind === DOUBLE_PRESS) {
        // Double press : shift lock        
        deck.shiftKey = true;
        deck.shiftLock = true;
        deck.LEDs.tap.onOff(ON);        
    } else {
        // Single press : temporary SHIFT
        
        // deck.shiftKey = false;
        deck.shiftLock = false;
        deck.LEDs.tap.onOff((value) ? ON : OFF); 
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
                engine.setValue(group, "LoadSelectedTrackAndPlay", true);
            } else {
                // Else play/pause
                toggleValue(group, "play");
            }
        } else {
            // shifted : stutter
            engine.setValue(group, "play_stutter", true);
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
    var shifted  = NumarkMixtrack3.decks.D1.shiftKey || NumarkMixtrack3.decks.D1.shiftKey;
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
    
    if (shifted) {
        // SHIFT+ BROWSE push : directory mode -- > Open/Close selected side bar item
        engine.setValue(group, "ToggleSelectedSidebarItem", true);
    } else {
        // Browse push : maximize/minimize library view
        if (value === ON) {
            
            NumarkMixtrack3.libraryMode = !NumarkMixtrack3.libraryMode;         
            if (maxview) {
                engine.setValue(LibraryGroup,LibraryCommand,expand);
                
            } else {
                engine.setValue(LibraryGroup,LibraryCommand,contract);
                
            }           
        }       
    }
};

NumarkMixtrack3.PadModeButton = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var loopsize = [0.125, 0.25, 0.5, 1, 2, 4, 8, 16];
    var i;

NumarkMixtrack3.PadModeButton = !NumarkMixtrack3.PadModeButton;
    
    if (value === DOWN) {
        //ensure all LEDs are ON (default)
        if (decknum === 1) {
            NumarkMixtrack3.decks["D1"].LEDs["PADsampler1"].onOff(PADcolors.purple);
            NumarkMixtrack3.decks["D2"].LEDs["PADsampler2"].onOff(PADcolors.purple);
            NumarkMixtrack3.decks["D3"].LEDs["PADsampler3"].onOff(PADcolors.purple);
            NumarkMixtrack3.decks["D4"].LEDs["PADsampler4"].onOff(PADcolors.purple);
            NumarkMixtrack3.decks["D1"].LEDs["PADloop1"].onOff(PADcolors.yellow);
            NumarkMixtrack3.decks["D1"].LEDs["PADloop2"].onOff(PADcolors.yellow);
            NumarkMixtrack3.decks["D1"].LEDs["PADloop3"].onOff(PADcolors.yellow);
            NumarkMixtrack3.decks["D1"].LEDs["PADloop4"].onOff(PADcolors.yellow);
        }
        
        if (decknum === 2) {
            NumarkMixtrack3.decks["D5"].LEDs["PADsampler5"].onOff(PADcolors.purple);
            NumarkMixtrack3.decks["D6"].LEDs["PADsampler6"].onOff(PADcolors.purple);
            NumarkMixtrack3.decks["D7"].LEDs["PADsampler7"].onOff(PADcolors.purple);
            NumarkMixtrack3.decks["D8"].LEDs["PADsampler8"].onOff(PADcolors.purple);
            NumarkMixtrack3.decks["D2"].LEDs["PADloop1"].onOff(PADcolors.yellow);
            NumarkMixtrack3.decks["D2"].LEDs["PADloop2"].onOff(PADcolors.yellow);
            NumarkMixtrack3.decks["D2"].LEDs["PADloop3"].onOff(PADcolors.yellow);
            NumarkMixtrack3.decks["D2"].LEDs["PADloop4"].onOff(PADcolors.yellow);
        }
    }
    
    // Now check which one should be blinking
    // Need to check if loop is enabled; if yes, stop it , else start it 
    //Autoloop
    if (value === DOWN) {    
        for (i=0;i<loopsize.length;i++) {  
            var index = i+1;
            if (index >4) {
                index = index - 4;
            }
            
            if (engine.getValue(group, "beatloop_" + loopsize[i] + "_enabled")) { 
            deck.LEDs["PADloop" + index].flashOn(300, PADcolors.yellow, 300);
            }   
        }
        
        //Sampler
        for (i = 1; i <= 8; i++) {
            engine.trigger("[Sampler" + i + "]", "play");
        }               
    }   
    
};

NumarkMixtrack3.BrowseKnob = function(channel, control, value, status, group) {
    var i;
    var shifted  = NumarkMixtrack3.decks.D1.shiftKey || NumarkMixtrack3.decks.D1.shiftKey;
    // value = 1 / 2 / 3 ... for positive //value = 1 / 2 / 3  
    var nval = (value>0x40 ? value-0x80 : value);

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
    
    //deck.faderstart = false;
    var groupOff;
    var deckOff;
    if (decknum === 1) {
        groupOff = "[Channel2]";
        deckOff = NumarkMixtrack3.decks["D2"];
    }else{
        groupOff = "[Channel1]";
        deckOff = NumarkMixtrack3.decks["D1"];
    }
        
    if (value === DOWN) {
        deck.LEDs["headphones"].onOff(ON);
        deck.faderstart = false;
        
        if (smartPFL) {
            print("smartPFL" +smartPFL);
            engine.setValue(group, 'pfl', true);
            engine.setValue(groupOff, 'pfl', false);
            deck.LEDs["headphones"].onOff(ON);
            deckOff.LEDs["headphones"].onOff(ON);
        }
        
                
        if (deck.shiftKey) {
            // SHIFT + Load = fader start activated
        
            deck.faderstart = true;
            deck.LEDs["headphones"].flashOn(250, ON, 250);
                
                if (!deck.TrackIsLoaded) {
                    printInfo("track not loaded, load track");
                    engine.setValue(group, 'LoadSelectedTrack', true);
                }

        }
            
        deck.LoadButtonControl.ButtonDown(channel, control, value, status, group);
    } else {

        deck.LoadButtonControl.ButtonUp();
    }   
};

// Callback for the Load Button
NumarkMixtrack3.OnLoadButton = function(channel, control, value, status, group, eventkind) {
    // var decknum = NumarkMixtrack3.deckFromGroup(group);
    // var deck = NumarkMixtrack3.decks["D" + decknum];
    
    if (eventkind === LONG_PRESS) {
        engine.setValue(group, 'eject', true);
    } else {
        engine.setValue(group, 'LoadSelectedTrack', true);
    }
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

            deck.SyncButtonControl.ButtonDown(channel, control, value, status, group);
        } else {

            deck.SyncButtonControl.ButtonUp();
        }
    } else {
        if (value === DOWN) {
            toggleValue(group,"keylock");
        }
    }
};

// Callback for the SYNC Button
NumarkMixtrack3.OnSyncButton = function(channel, control, value, status, group, eventkind) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
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
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var valIn = engine.getValue(group, 'sync_enabled');
    
    if (valIn) {
        deck.LEDs.sync.onOff(ON);
    }else{
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
        if (engine.getValue(group, "playposition") <= 0.97) {
            engine.setValue(group, "cue_default", value ? 1 : 0);
        } else {
            engine.setValue(group, "cue_preview", value ? 1 : 0);
        }
    } else {
        engine.setValue(group, "start", true);
    }
};

// Pitch faders send 2*7bits
NumarkMixtrack3.PitchFaderHighValue  = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.PitchFaderHigh = value;
};

NumarkMixtrack3.PitchFaderLowValue  = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var calcvalue = (8192-((deck.PitchFaderHigh*128)+value))/8192;
    engine.setValue(group, "rate", calcvalue);
};


NumarkMixtrack3.toggleJogMode = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    
    if (value === DOWN) {
        // Toggle setting and light
        deck.jogWheelsInScratchMode = !deck.jogWheelsInScratchMode;
        deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ? ON : OFF);
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

    deck.touch = false;
    deck.iCutStatus = false;
    deck.seekingfast = false;
    
    printInfo("Start WheelMove decknum=" + decknum +"; deck="+deck+"; group="+group+";    WheelTouch = DOWN (on) ");
        
    if (value === DOWN) {
        printInfo("   WheelTouch = DOWN (on) ");    
                
        if (deck.jogWheelsInScratchMode) {
        
        engine.scratchEnable(decknum, intervalsPerRev, rpm, alpha, beta);
            
            // Wheel is On - test for Shift Key");
    
            if (deck.shiftKey && iCutEnabled) { 
                printInfo("   Wheel ON - Shift true - ICUT = true");
                deck.iCutStatus = true;
                deck.Jog.iCUT.On();
                
            } else {
                printInfo("   Wheel ON - Shift = false - Scratching - deck.touch = true");
                deck.iCutStatus = false;
                deck.touch = true;
                deck.Jog.iCUT.Off();
            }
            
        } else {
                        
            if (fastSeekEnabled && deck.shiftKey) {
                printInfo("   Wheel Off - Shift true - Fast Seek");
                deck.seekingfast = true;    
            }                   
        }
    } else {
        
        printInfo("   WheelTouch = UP (off) ");     
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
        printInfo("   WheelMove - ICUT = true - adjustedJog =" +adjustedJog +" value="+value);  
        deck.Jog.iCUT.On();
        deck.Jog.iCUT.FaderCut(adjustedJog);
    } 
    
    // the 2 conditions below may not be required as the simply default to 
    if (deck.touch) {
        printInfo("   WheelMove - Scratching = true - adjustedJog =" +adjustedJog+" value="+value);
        // scratch is enabled in wheel touch, we just record ticks
    } 
    
    if (!deck.seekingfast && !deck.iCutStatus && !deck.touch) {
        printInfo("   WheelMove - Jog / Pitch bend");
    }
    
    if (deck.seekingfast) {
        printInfo("   WheelMove - seekingfast = true - adjustedJog =" +adjustedJog+" value="+value);
        engine.setValue(deck.Jog.group, "beatjump", adjustedJog * 2);
    }
        
    engine.scratchTick(decknum, adjustedJog);
    
    printInfo( "engine.scratchTick("+ decknum +"," +adjustedJog+") value="+value);
    
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
};  

NumarkMixtrack3.HotCueButton = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var hotCue = control - leds.hotCue1 + 1;

    // onHotCueChange called automatically
    if (deck.shiftKey) {
        if (value === DOWN) {
            engine.setValue(group, "hotcue_" + hotCue + "_clear", true);
            deck.LEDs["hotCue" + hotCue].onOff(OFF);
        }
    } else {
        if (value === DOWN) {
            engine.setValue(group, "hotcue_" + hotCue + "_activate", 1);
        } else {
            engine.setValue(group, "hotcue_" + hotCue + "_activate", 0);
        }
    }
};

// Returns the deck number of a "ChannelN" or "SamplerN" group
// copied from the common script because for some reason the section for extracting the sampler number 
// is commented out in the common script
NumarkMixtrack3.deckFromGroup = function (group) {
    var deck = 0;
    if (group.substring(2,8)=="hannel") {
        // Extract deck number from the group text
        deck = group.substring(8,group.length-1);
    }

    else if (group.substring(2,8)=="ampler") {
        // Extract sampler number from the group text
        deck = group.substring(8,group.length-1);
    }

    return parseInt(deck);
};

NumarkMixtrack3.SamplerButton = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var padindex = decknum;

    if (value=== DOWN) {
        NumarkMixtrack3.samplers.play(padindex,true);
    }
    
    if (value === OFF && PADSampleButtonHold) {
    NumarkMixtrack3.samplers.play(padindex,false);
    }   
};

NumarkMixtrack3.PADLoopButton = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var padindex = control - leds.PADloop1 + 1;
    var loopsize = 1;
    var trueFalse;
    
    if (deck.shiftKey) {
        loopsize = Math.pow(2, padindex);
    } else {
        loopsize = Math.pow(2, padindex - 4);
    }
    
    var loopCommand1; //verify if loop is active
    var loopCommand2; //enable loop
    var loopCommand3; //stop loop
    
    if (beatlooprollActivate) {
        loopCommand1 = "beatlooproll_" + loopsize + "_activate";
        loopCommand2 = "beatlooproll_" + loopsize + "_activate";
        loopCommand3 = "beatlooproll_" + loopsize + "_activate";
        trueFalse = false;
    } else {
        loopCommand1 = "beatloop_" + loopsize + "_enabled";
        loopCommand2 = "beatloop_" + loopsize + "_toggle";
        loopCommand3 = "reloop_exit";
        trueFalse = true;
    }
                
    if (value === DOWN) { 
            // make sure all LED are ON
            deck.LEDs["PADloop1"].onOff(PADcolors.yellow);
            deck.LEDs["PADloop2"].onOff(PADcolors.yellow);
            deck.LEDs["PADloop3"].onOff(PADcolors.yellow);
            deck.LEDs["PADloop4"].onOff(PADcolors.yellow);  
            
        if (engine.getValue(group,loopCommand1)) {
            // Loop is active, turn it off
            print("LED ON - loop OFF "+ padindex);
            engine.setValue(group,loopCommand3, trueFalse);
            deck.LEDs["PADloop" + padindex].onOff(PADcolors.yellow);
            
        } else { 
            // Loop is not active, turn it on
            print("Flash LED - loop "+ padindex);                   
            deck.LEDs["PADloop" + padindex].flashOn(250, PADcolors.yellow, 250);
            print("loopCommand2 "+ loopCommand2);
            engine.setValue(group,loopCommand2,true);  

        }   
    }
    
    if (value === OFF && PADLoopButtonHold) { 
        engine.setValue(group,loopCommand2,false); 
        deck.LEDs["PADloop" + padindex].onOff(PADcolors.yellow);    
    }
};


NumarkMixtrack3.StripTouchEffect = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (deck.shiftKey) {
        engine.setValue(group, "playposition", value / 127);            
    } else {
        deck.StripEffect(value, decknum); 
    }
};

NumarkMixtrack3.StripTouchSearch = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (deck.shiftKey) {
        engine.setValue(group, "playposition", value / 127);            
    } else {
        deck.StripEffect(value, decknum); 
    }
};

NumarkMixtrack3.FXButton = function(channel, control, value, status, group) {
    if (!value) return;
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var ButtonNum = control - leds.fx1+1;
    if (value === DOWN) {
        if (deck.shiftKey) {
            // Select Effect 
            if (decknum===1) {
                engine.setValue("[EffectRack1_EffectUnit" + ButtonNum + "]", "prev_chain", true);
            } else {
                engine.setValue("[EffectRack1_EffectUnit" + ButtonNum + "]", "next_chain", true);
            }
        } else {
            // Toggle effect            
            var new_value = !engine.getValue("[EffectRack1_EffectUnit" + ButtonNum + "]","group_" + group + "_enable");
            engine.setValue("[EffectRack1_EffectUnit" + ButtonNum + "]","group_" + group + "_enable",new_value);
            deck.LEDs["fx" + ButtonNum].onOff(new_value?ON:OFF);
        }
    }
}; 

/******************     Shift Button :
 * - Press                : toggle PFL
 * - SHIFT + press        : toggle slip mode
 * - SHIFT + double press : toggle quantize mode
 * *********************************************************************/
NumarkMixtrack3.PFLButton  = function(channel, control, value, status, group) {
    if (!value) return;
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (value === DOWN) {
        if (deck.shiftKey) {
            deck.ShiftedPFLButtonControl.ButtonDown(channel, control, value, status, group);
        } else {
            toggleValue(group,"pfl");
        }
    }
}; 

// Callback for the PFL Button
NumarkMixtrack3.OnShiftedPFLButton = function(channel, control, value, status, group, eventkind) {
    if (eventkind === DOUBLE_PRESS) {
        // Double press : toggle slip mode
        toggleValue(group,"slip_enabled");
    } else {
        // Single press : toggle quantize mode
        toggleValue(group,"quantize");
    }
};

NumarkMixtrack3.LoopHalveButton = function(channel, control, value, status, group) {
    if (value===DOWN) {
        var decknum = NumarkMixtrack3.deckFromGroup(group);
        var deck = NumarkMixtrack3.decks["D" + decknum];

        if (deck.shiftKey) {        
            engine.setValue(group, "loop_double", true);
        } else {
            engine.setValue(group, "loop_halve", true);
        }
    }
};

NumarkMixtrack3.PitchBendMinusButton = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (value===DOWN) {
        if (deck.shiftKey) {        
            engine.setValue(group, "beatjump_1_backward", true);
        } else {
            engine.setValue(group, "rate_temp_down", true);
        }
    } else if (!deck.shiftKey) {
        engine.setValue(group, "rate_temp_down", false);
    }
};

NumarkMixtrack3.PitchBendPlusButton = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    
    if (value===DOWN) {

        if (deck.shiftKey) {        
            engine.setValue(group, "beatjump_1_forward", true);
        } else {
            engine.setValue(group, "rate_temp_up", true);
        }
    }
    else if (!deck.shiftKey) {
        engine.setValue(group, "rate_temp_up", false);
    }
};

NumarkMixtrack3.BeatKnob = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var gainIncrement;
    var gainValue = [];
    var i,j;
    var knobValue;
    
    if (!BeatKnobAsSamplerVolume) {
        // Default configuration option, use knob for Beatgrid only
        if (value-64 > 0) {
            knobValue = value-128;
        } else {
            knobValue = value;
        }

        if (deck.shiftKey) {  
            if (knobValue<0) {
                engine.setValue(group, "beats_adjust_slower", true);
            } else {
                engine.setValue(group, "beats_adjust_faster", true);
            }
        } else {
            if (knobValue<0) {
                engine.setValue(group, "beats_translate_earlier", true);
            } else {
                engine.setValue(group, "beats_translate_later", true);
            }
        }
    
    } else {
        // Option: use BeatKnob to adjust Sampler volumes
        if (deck.shiftKey) {    
            if (value-64 > 0) {
                knobValue = value-128;
            } else {
                knobValue = value;
            }
            
            if (knobValue<0) {
                engine.setValue(group, "beats_translate_earlier", true);
            } else {
                engine.setValue(group, "beats_translate_later", true);
            }
        } else {
            // Shift key is not used, we adjust Sampler volumes
            // Define new value of sampler pregain
            // Off = 1, centered = 1, max = 4
            if (decknum === 1) {
                for (i = 1; i <= 4; i++) {
                    gainValue[i-1] = engine.getValue("[Sampler" + [i] + "]", "pregain");    
                    
                    if (gainValue[i-1] <= 1) { 
                        // increment value between 0 and 1
                        gainIncrement = 1/20;  // 20 increments in one full knob turn   
                    }else{
                        // increment value between 1 and 4
                        gainIncrement = 3/20;  // 20 increments in one full knob turn
                    }
                    
                    // beat knobs sends 1 or 127 as value. If value = 127, turn is counterclockwise, we reduce gain
                    if (value === 127) {
                        gainIncrement = - gainIncrement;
                    }   
                                    
                    gainValue[i-1] = gainValue[i-1] + gainIncrement;

                    if ((gainValue[i-1] + gainIncrement) <0) {
                        gainValue[i-1] = 0;
                    }
                    
                    if ((gainValue[i-1] + gainIncrement) >4) {
                        gainValue[i-1] = 4;
                    }
                        
                }
            } else {
                for (j = 5; j <= 8; j++) {
                    gainValue[j-1] = engine.getValue("[Sampler" + [j] + "]", "pregain");    
                
                    if (gainValue[j-1] <= 1) { 
                        // increment value between 0 and 1
                        gainIncrement = 1/20;  // 20 increments in one full knob turn   
                    }else{
                        // increment value between 1 and 4
                        gainIncrement = 3/20;  // 20 increments in one full knob turn
                    }
                    
                    // beat knobs sends 1 or 127 as value. If value = 127, turn is counterclockwise, we reduce gain
                    if (value === 127) {
                        gainIncrement = - gainIncrement;
                    }   
                    
                    gainValue[j-1] = gainValue[j-1] + gainIncrement;

                    if ((gainValue[j-1] + gainIncrement) <0) {
                        gainValue[j-1] = 0;
                    }
                    
                    if ((gainValue[j-1] + gainIncrement) >4) {
                        gainValue[j-1] = 4;
                    }           
                }   
            }

            // we adjust pregain with adjusted value
            if (decknum === 1) {
                for (i = 1; i <= 4; i++) {  
                    if (gainValue[i-1] >=0 && gainValue[i-1] <= 4) {  
                        engine.setValue("[Sampler" + i + "]", "pregain", gainValue[i-1]);   
                    }
                }
            } else {
                for (i = 5; i <= 8; i++) {
                    if (gainValue[i-1] >= 0 && gainValue[i-1] <= 4) { 
                        engine.setValue("[Sampler" + i + "]", "pregain", gainValue[i-1]);   
                    }
                }   
            }
        }
    }
};

NumarkMixtrack3.bpmTap  = function(channel, control, value, status, group) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];

    if (value===DOWN) {
        engine.setValue(group, "bpm_tap", true);
    } else {
        engine.setValue(group, "bpm_tap", false);
    }   
};

// ************************ Connected controls
NumarkMixtrack3.OnVuMeterChange = function(value, group, control) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.LEDs.meter.onOff(120*value);
};

NumarkMixtrack3.OnPlaypositionChange  = function(value, group, control) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
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
                deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ? ON : OFF);
                break;

            case 1: // if less than 30 seconds before end of track : flashing slowly
                if (deck.LEDs.jogWheelsInScratchMode.getFlashDuration() !== 1000) {
                    deck.LEDs.jogWheelsInScratchMode.flashOn(1000, ON, 1000);
                }
                break;

            case 2: // if less than 10 seconds before end of track : flashing fast
                if (deck.LEDs.jogWheelsInScratchMode.getFlashDuration() !== 300) {
                    deck.LEDs.jogWheelsInScratchMode.flashOn(300, ON, 300);
                }
                break;

            case 3: // end of strack : full ring lit
                deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ? ON : OFF);
                break;
            default:
                break;
        }
    } else {
        deck.LEDs.jogWheelsInScratchMode.onOff(deck.jogWheelsInScratchMode ? ON : OFF);
    }
};

NumarkMixtrack3.OnHotcueChange = function(value, group, control, padindex) {
    var decknum = parseInt(group.substring(8,9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.LEDs["hotCue" + padindex].onOff((value) ? ON : OFF);
};

NumarkMixtrack3.OnTrackLoaded = function(value, group, control) {
    var decknum = parseInt(group.substring(8,9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
    
    if (value!==0) {
        if ( !deck.faderstart ) {
            // Light up the PFL light indicating that a track is loaded
            deck.LEDs["headphones"].onOff(ON);
        } else {
            // Flash up the PFL light button indicating that a track is loaded with fader start
            deck.LEDs["headphones"].flashOn(300,ON,300);
            
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

NumarkMixtrack3.OnVolumeChange = function(value, group, control) {
    var decknum = parseInt(group.substring(8,9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var delta = value - deck.lastfadervalue;
        
    if (deck.faderstart) {
        if (value<=0.01) {
            engine.setValue(group, "play", 0);
        } else {
            if (delta>0) {
                engine.setValue(group, "play", 1);
            }
        }
    }
    deck.lastfadervalue = value;
};

NumarkMixtrack3.OnPFLStatusChange = function(value, group, control) {
    var decknum = parseInt(group.substring(8,9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.LEDs.headphones.onOff((value) ? ON : OFF);
};

NumarkMixtrack3.OnPlayIndicatorChange = function(value, group, control) {
    var decknum = parseInt(group.substring(8,9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.LEDs.play.onOff((value) ? ON : OFF);
};

NumarkMixtrack3.OnBeatActive = function(value, group, control) {
    var decknum = parseInt(group.substring(8,9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
    if (!deck.shiftKey && OnBeatActiveFlash) {
    deck.LEDs.tap.onOff((value) ? ON : OFF);
    }
};

NumarkMixtrack3.OnCuePointChange = function(value, group, control) {
    var decknum = parseInt(group.substring(8,9));
    var deck = NumarkMixtrack3.decks["D" + decknum];
    deck.LEDs.Cue.onOff((value) ? ON : OFF);
};

NumarkMixtrack3.OnLoopInOutChange = function(value, group, key) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var valIn = engine.getValue(group, "loop_start_position");
    var valOut = engine.getValue(group, "loop_end_position");
    var valEnabled = engine.getValue(group, "loop_enabled");

    if (valIn==-1) {
        if (deck.LEDs.loopin.getFlashDuration()!==300) {
            deck.LEDs.loopin.flashOn(300,PADcolors.blue,300);
        }
        deck.LEDs.loopout.onOff(OFF);
        deck.LEDs.reloop_exit.onOff(OFF);
        deck.LEDs.loop_halve.onOff(OFF);
    } else if (valOut==-1) {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        if (deck.LEDs.loopout.getFlashDuration()!==300) {
            deck.LEDs.loopout.flashOn(300,PADcolors.blue,300);
        }
        deck.LEDs.reloop_exit.onOff(OFF);
        deck.LEDs.loop_halve.onOff(OFF);
    } else if (!valEnabled) {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        deck.LEDs.loopout.onOff(PADcolors.blue);
        if (deck.LEDs.reloop_exit.getFlashDuration()!==300) {
            deck.LEDs.reloop_exit.flashOn(300,PADcolors.blue,300);
        }
        deck.LEDs.loop_halve.onOff(PADcolors.blue);
    } else {
        deck.LEDs.loopin.onOff(PADcolors.blue);
        deck.LEDs.loopout.onOff(PADcolors.blue);
        deck.LEDs.reloop_exit.onOff(PADcolors.blue);
        deck.LEDs.loop_halve.onOff(PADcolors.blue);
    }    
};

NumarkMixtrack3.OnEffectLoaded = function(value, group, control, index) {
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

NumarkMixtrack3.OnSamplePlayStop = function(value, group, control) {
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    
    if (value === 1) {   
    deck.LEDs["PADsampler" + decknum].flashOn(300, PADcolors.purple, 300);
    } else {
    deck.LEDs["PADsampler" + decknum].onOff(ON);
    }    
};

NumarkMixtrack3.OnPADLoopButtonChange = function(value, group, control) {   
    var loopsize = [0.125, 0.25, 0.5, 1, 2, 4, 8, 16];
    var decknum = NumarkMixtrack3.deckFromGroup(group);
    var deck = NumarkMixtrack3.decks["D" + decknum];
    var l;
    var index;
    
    if (value === 1) {   
        for (l=0;l<loopsize.length;l++) {  
        
            if (engine.getValue(group, "beatloop_" + loopsize[l] + "_enabled")) { 
                index = l+1;
                if (index >4) {
                    index = index - 4;
                }
                    
                deck.LEDs["PADloop" + index].flashOn(300, PADcolors.yellow, 300);
                
            }
        }
    } else {
        
        for (l=0;l<loopsize.length;l++) {  
        
            if (!engine.getValue(group, "beatloop_" + loopsize[l] + "_enabled")) {    
                index = l+1;
                    if (index >4) {
                        index = index - 4;
                    }
                
                deck.LEDs["PADloop"+ index].onOff(PADcolors.yellow);
            } 
        }
    }
        
};
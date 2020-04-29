// ****************************************************************************
// * Mixxx mapping script file for the Behringer CMD Studio 4a.
// * Author: Craig Easton
// * Version 1.4 (Jan 2016)
// * Forum: http://www.mixxx.org/forums/viewtopic.php?f=7&amp;t=7868
// * Wiki: http://www.mixxx.org/wiki/doku.php/behringer_cmd_studio_4a
// ****************************************************************************

////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
//////////////////////////////////////////////////////////////////////// 

// Master function definition.
function BehringerCMDStudio4a() {}


// ***************************** Global Vars **********************************

// Shift/mode state variables.
BehringerCMDStudio4a.delButtonState = [false,false,false,false];
BehringerCMDStudio4a.scratchButtonState = [false,false,false,false];

// Button push/release state variables.
BehringerCMDStudio4a.pitchPushed = [[false,false,false,false], [false,false,false,false]];
BehringerCMDStudio4a.delPushed = false;
BehringerCMDStudio4a.delShiftUsed = false;
BehringerCMDStudio4a.fxAssignPushed = false;
BehringerCMDStudio4a.fxAssignShiftUsed = false;
BehringerCMDStudio4a.fxAssignLastGroup = "";

// ************************ Initialisation stuff. *****************************

BehringerCMDStudio4a.vuMeterUpdate = function (value, group, control){
    value = (value*15)+48;
    switch(control) {
    case "VuMeterL":
        midi.sendShortMsg(0xB0, 0x7E, value);
        break;
    case "VuMeterR":
        midi.sendShortMsg(0xB0, 0x7F, value);
        break;
    }
}

BehringerCMDStudio4a.initLEDs = function () {
    // (re)Initialise any LEDs that are direcctly controlled by this script.
    // DEL buttons (one for each virtual deck).
    midi.sendShortMsg(0x90, 0x2A, 0x00);
    midi.sendShortMsg(0x91, 0x4A, 0x00);
    midi.sendShortMsg(0x92, 0x2A, 0x00);
    midi.sendShortMsg(0x93, 0x4A, 0x00);
    // Scratch buttons (one for each virtual deck).
    midi.sendShortMsg(0x90, 0x16, 0x00);
    midi.sendShortMsg(0x91, 0x36, 0x00);
    midi.sendShortMsg(0x92, 0x16, 0x00);
    midi.sendShortMsg(0x93, 0x36, 0x00);    
}

BehringerCMDStudio4a.init = function () {
    // Initialise anything that might not be in the correct state.
    BehringerCMDStudio4a.initLEDs();
    // Connect the VUMeters
    engine.connectControl("[Master]","VuMeterL","BehringerCMDStudio4a.vuMeterUpdate");
    engine.connectControl("[Master]","VuMeterR","BehringerCMDStudio4a.vuMeterUpdate");
}
 
BehringerCMDStudio4a.shutdown = function() {
    // Leave the deck in a properly initialised state.
    BehringerCMDStudio4a.initLEDs();

    // Disconnect the VUMeters.
// Maybe not! It seems you don't have to do this even though the connection
// in done in init(), in fact if you try it throws an error.
//  engine.connectControl("[Master]","VuMeterL","BehringerCMDStudio4a.vuMeterUpdate",true);
//  engine.connectControl("[Master]","VuMeterR","BehringerCMDStudio4a.vuMeterUpdate",true);
}


// *************************** Control Stuff. *********************************
// The code below is primarily "shift/mode" key functionality as there is no
// native support for this in Mixxx at the moment.
// I suspect that the vast majority of controller mappings could be completed
// with little or no scripting if Mixxx supported shift/mode buttons in the
// XML (together with standard wheel/scratching functionality).

// Function to deal with the DEL "shift/mode" buttons.
BehringerCMDStudio4a.del = function (channel, control, value, status, group) {
    if (value == 127) {
        // Button pushed.
        BehringerCMDStudio4a.delPushed = true;
        BehringerCMDStudio4a.delShiftUsed = false;
    } else {
        // Button released.
        BehringerCMDStudio4a.delPushed = false;
        // Only toggle the DEL-mode if the "shift" function wasn't used.
        if (!BehringerCMDStudio4a.delShiftUsed) {
            BehringerCMDStudio4a.delButtonState[channel] = !BehringerCMDStudio4a.delButtonState[channel];
            midi.sendShortMsg(0x90 + channel, control, BehringerCMDStudio4a.delButtonState[channel] ? 0x01 : 0x00);
        }
    }
}

// Function to deal with the play buttons, (because they have a DEL-mode behaviour).
BehringerCMDStudio4a.play = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-mode is active, do reverse-roll (slip).
        engine.setValue(group, "reverseroll", (value == 127) ? 1 : 0);
    } else {
        // DEL-mode is not active, just toggle play (on push only).
        if (value == 127) {
            script.toggleControl(group,"play");
        }
    }
}

// Function to deal with the cue buttons, (because they have a DEL-mode behaviour).
BehringerCMDStudio4a.cue = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-mode is active, do reverse play.
        engine.setValue(group, "reverse", (value == 127) ? 1 : 0);
    } else {
        // DEL-mode is not active so just cue.
        engine.setValue(group, "cue_default", (value == 127) ? 1 : 0);
    }
}

// Function to deal with the scratch mode buttons.
BehringerCMDStudio4a.scratch = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.scratchButtonState[channel] = !BehringerCMDStudio4a.scratchButtonState[channel];
    midi.sendShortMsg(status, control, BehringerCMDStudio4a.scratchButtonState[channel] ? 0x01 : 0x00);
}

// Function to deal with the FX Assign buttons, (because they also act as "shift" buttons).
BehringerCMDStudio4a.fxAssign = function (channel, control, value, status, group) {
    // FX Assign buttons start at 0x52.
    var fxAssignButton = (control - 0x52) & 1;  // Either 0 or 1 depending on button (1 or 2).
    if (value == 127) {
        // Button pushed.
        BehringerCMDStudio4a.fxAssignPushed = true;
        BehringerCMDStudio4a.fxAssignShiftUsed = false;
        BehringerCMDStudio4a.fxAssignLastGroup = group;
    }
    else
    {
        // Button released.
        BehringerCMDStudio4a.fxAssignPushed = false;
        // Only toggle the effect on release if the "shift" function wasn't used.
        if (!BehringerCMDStudio4a.fxAssignShiftUsed) {
            script.toggleControl(group,"group_[Channel"+(channel+1)+"]_enable");
        }
    }
}

// Function to deal with the browse left/right buttons, (because they have an "FX Assign mode" behaviour).
BehringerCMDStudio4a.browseLR = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.fxAssignPushed) {
        BehringerCMDStudio4a.fxAssignShiftUsed = true;
        if (control == 0x2) {
            // Left.
            engine.setValue(BehringerCMDStudio4a.fxAssignLastGroup,"prev_chain", 1);
        } else {
            // Right.
            engine.setValue(BehringerCMDStudio4a.fxAssignLastGroup,"next_chain", 1);
        }
    } else {
        if (control == 0x2) {
            // Left.
            engine.setValue(group, "SelectPrevPlaylist",1)
        } else {
            // Right.
            engine.setValue(group, "SelectNextPlaylist",1)
        }
    }
}

// Functions to deal with the hot-cue buttons, (because they have a DEL-mode behaviour).
BehringerCMDStudio4a.hotcue = function (channel, control, value, status, group) {
    // Translate the button to the actual hotcue.
    var hotcue = control-0x21;  // Hotcue buttons on left deck go from 0x22 to 0x29
    if (hotcue>8) {
        // Right deck, buttons are 0x20 higher so we need to compensate.
        hotcue = hotcue-0x20;
    }
    if (BehringerCMDStudio4a.delPushed) {
        // DEL button is being held so delete the hotcue.
        engine.setValue(group, "hotcue_"+hotcue+"_clear", 1);
        BehringerCMDStudio4a.delShiftUsed = true;
    } else {
        // DEL button is not being held down.
        if (BehringerCMDStudio4a.delButtonState[channel]) {
            // DEL-mode is active, lets do auto-loops.
            engine.setValue(group, "beatloop_"+(1/8)*Math.pow(2,hotcue-1)+"_toggle", 1);
            if (value == 0) {
                // Button is being released. Disable then re-enable slip if it
                // is active. This "re-syncs" the playback after every
                // auto-loop in slip-mode which is a nice effect and probably
                // what you want most of the time if slip is on.
                if (engine.getValue(group, "slip_enabled") == 1) {
                    engine.setValue(group, "slip_enabled", 0);
                    // It seems we can't just flip a param off and on in the
                    // same call! Since we've just turned slip off, I can't now
                    // turn it on directly here, the only work-around I could
                    // think of was to create a (very short) timed call-back
                    // to turn it off!
                    // Raised bug about this:
                    // https://bugs.launchpad.net/mixxx/+bug/1538200
                    // Changed timer from 50 to 100 after the pathology of this
                    // bug was explined in the bug report.
                    engine.beginTimer(100, function() { engine.setValue(group, "slip_enabled", 1); }, 1);
                }
            }
        } else {
            // DEL-mode is not active so do the set/jump-to hotcue function as normal.
            engine.setValue(group, "hotcue_"+hotcue+"_activate", (value == 127) ? 1 : 0);
        }
    }
}

// Functions to deal with the pitch inc/dec buttons, (because they have a DEL-mode behaviour).
BehringerCMDStudio4a.pitch = function (channel, control, value, status, group) {
    // Work out the direction.
    var direction = ((control & 0x01) == 0) ? "down" : "up";
    // Work out the type (and join) by looking at the DEL button state.
    var type = BehringerCMDStudio4a.delButtonState[channel] ? "pitch" : "rate";
    var join = BehringerCMDStudio4a.delButtonState[channel] ? "" : "_perm";
    // Pushed or released?
    if (value == 127) {
        // Button pushed.
        BehringerCMDStudio4a.pitchPushed[control & 0x01][channel] = true;
        // Is the other button pushed too?
        if (BehringerCMDStudio4a.pitchPushed[(~control) & 0x01][channel]) {
            engine.setValue(group, type, 0); // Yep! reset the control.
        } else {
            engine.setValue(group, type+join+"_"+direction, 1);
        }
    } else {
        // Button released.
        BehringerCMDStudio4a.pitchPushed[control & 0x01][channel] = false;
        engine.setValue(group, "rate_perm_"+direction, 0); // Keeps the UI in sync with the button state.
    }
}

// Functions to deal with the wheel (i.e. scratcing and jog).
// Why is there no (XML) support in Mixxx for this most basic of functions?
// I suspect the vast majority of controller mappings use the same code
// (provided in the Wiki).
BehringerCMDStudio4a.wheelTouch = function (channel, control, value, status, group) {
    channel = channel+1;
    if (value > 0) {
        // We're touching the wheel.
        var alpha = 1.0/8;
        var beta = alpha/32;
        engine.scratchEnable(channel, 600, 33+1/3, alpha, beta);
    } else {
        // We've released the wheel.
        engine.scratchDisable(channel);
    }
};
BehringerCMDStudio4a.wheelTurn = function (channel, control, value, status, group) {
    var deck = channel+1;
    var newValue = value-64;
    if (BehringerCMDStudio4a.scratchButtonState[channel]){
        if (engine.isScratching(deck)){
            engine.scratchTick(deck,newValue);  // Scratch!
        }
    } else {
        engine.setValue(group, "jog", newValue); // Jog.
    }
};

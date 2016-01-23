// ****************************************************************************
// * Mixxx mapping script file for the Behringer CMD Studio 4a.
// * Author: Craig Easton
// * Version 1.2 (Jan 2016)
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
BehringerCMDStudio4a.pitchDec = [false,false,false,false];
BehringerCMDStudio4a.pitchInc = [false,false,false,false];


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
    BehringerCMDStudio4a.delButtonState[channel] = !BehringerCMDStudio4a.delButtonState[channel];
    midi.sendShortMsg(status, control, BehringerCMDStudio4a.delButtonState[channel] ? 0x01 : 0x00);
}

// Functions to deal with the play buttons, (because they have a DEL-mode behaviour).
BehringerCMDStudio4a.playPush = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-mode is active, do reverse-roll (slip).
        engine.setValue(group, "reverseroll", 1);
    } else {
        // DEL-mode is not active, just play.
		script.toggleControl(group,"play")
    }
}
BehringerCMDStudio4a.playRelease = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-mode is active, do reverse-roll (slip) release.
        engine.setValue(group, "reverseroll", 0);
    }
}

// Functions to deal with the cue buttons, (because they have a DEL-mode behaviour).
BehringerCMDStudio4a.cuePush = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-mode is active, do reverse play.
        engine.setValue(group, "reverse", 1);
    } else {
        // DEL-mode is not active so just cue.
        engine.setValue(group, "cue_default", 1);
    }
}
BehringerCMDStudio4a.cueRelease = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-modde is active, do reverse-roll release.
        engine.setValue(group, "reverse", 0);
    } else {
        // DEL-mode is not active so just release the cue.
        engine.setValue(group, "cue_default", 0);
	}
}


// Function to deal with the scratch mode buttons.
BehringerCMDStudio4a.scratch = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.scratchButtonState[channel] = !BehringerCMDStudio4a.scratchButtonState[channel];
    midi.sendShortMsg(status, control, BehringerCMDStudio4a.scratchButtonState[channel] ? 0x01 : 0x00);
}

// Functions to deal with the hot-cue buttons, (because they have a DEL-mode behaviour).
BehringerCMDStudio4a.hotcuePush = function (channel, control, value, status, group) {
    // Translate the button to the actual hotcue.
    var hotcue = control-0x21;  // Hotcue buttons on left ddeck go from 0x22 to 0x29
    if (hotcue>8) {
        // Right deck, buttons are 0x20 higher so we need to compensate.
        hotcue = hotcue-0x20;
    }
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-mode is active, so delete the hotcue.
        engine.setValue(group, "hotcue_"+hotcue+"_clear", 1);
    } else {
        // DEL-mode is not active, do the set/jup-top hotcue function as normal.
        engine.setValue(group, "hotcue_"+hotcue+"_activate", 1);
    }
}
BehringerCMDStudio4a.hotcueRelease = function (channel, control, value, status, group) {
    // Translate the button to the actual hotcue.
    var hotcue = control-0x21;  // Hotcue buttons on left deck go from 0x22 to 0x29
    if (hotcue>8) {
        // Right deck, buttons are 0x20 higher so we need to compensate.
        hotcue = hotcue-0x20;
    }
    if (!BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-mode is not active, do the release hotcue function as normal.
        engine.setValue(group, "hotcue_"+hotcue+"_activate", 0);
    }
}

// Functions to deal with the pitch inc/dec buttons, (because they have a DEL-mode behaviour).
BehringerCMDStudio4a.pitchDecPush = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.pitchDec[channel] = true;
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL mode is active, do "key" changes instead.
        // Check if the other button is pressed too, if so we reset the pitch.
        if (BehringerCMDStudio4a.pitchInc[channel]) {
            engine.setValue(group, "pitch", 0);
        } else {
            engine.setValue(group, "pitch_down", 1);
        }
    } else {
        // DEL-mode is not active, just do "rate" changes.
        engine.setValue(group, "rate_perm_down", 1);
        // Check if the other button is pressed too, if so we reset the pitch.
        if (BehringerCMDStudio4a.pitchInc[channel]) {
            engine.setValue(group, "rate", 0);
        }
    }
    // NB: We do the reset check for pitch before triggering the pitch down as
    //     Mixxx does not seem to like rapid rate switches. We trigger rate
    //     down regardless so as to preserve the GUI button press indicators.
}
BehringerCMDStudio4a.pitchDecRelease = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.pitchDec[channel] = false;
    engine.setValue(group, "rate_perm_down", 0);
}
BehringerCMDStudio4a.pitchIncPush = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.pitchInc[channel] = true;
    if (BehringerCMDStudio4a.delButtonState[channel]) {
        // DEL-mode is active, do "key" changes instead.
        // Check if the other button is pressed too, if so we reset the pitch.
        if (BehringerCMDStudio4a.pitchDec[channel]) {
            engine.setValue(group, "pitch", 0);
        } else {
            engine.setValue(group, "pitch_up", 1);          
        }
    } else {
        // DEL-mode is not active, just do "rate" changes.
        engine.setValue(group, "rate_perm_up", 1);
        // Check if the other button is pressed too, if so we reset the pitch.
        if (BehringerCMDStudio4a.pitchDec[channel]) {
            engine.setValue(group, "rate", 0);
        }
    }
    // NB: We do the reset check for pitch before triggering the pitch up as
    //     Mixxx does not seem to like rapid rate switches. We trigger rate
    //     up regardless so as to preserve the GUI button press indicators.
}
BehringerCMDStudio4a.pitchIncRelease = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.pitchInc[channel] = false;
    engine.setValue(group, "rate_perm_up", 0);
}

// Functions to deal with the wheel (i.e. scratcing and jog).
// Why is there no standard support in Mixxx for this most basic of functions?
// I suspect the vast majority of controller mappings use the same code
// (provided in the Wiki).
BehringerCMDStudio4a.wheelTouch = function (channel, control, value, status, group) {
    channel = channel+1;
    if (value > 0) {
        // Were touching the wheel.
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

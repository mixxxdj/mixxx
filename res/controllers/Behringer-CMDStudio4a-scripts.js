// ********************************************************************************************************************
// * Mixxx mapping script file for the Behringer CMD Studio 4a.
// * Author: Craig Easton
// * Version 1.2 (Jan 2016)
// * Forum: http://www.mixxx.org/forums/viewtopic.php?f=7&amp;t=7868
// * Wiki: http://www.mixxx.org/wiki/doku.php/behringer_cmd_studio_4a
// ********************************************************************************************************************

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


// *************************************************** Global Vars ****************************************************

// "Shift" mode state variables.
BehringerCMDStudio4a.delState = {0:false,1:false,2:false,3:false};
BehringerCMDStudio4a.scratchState = {0:false,1:false,2:false,3:false};

// Button push/release state variables.
BehringerCMDStudio4a.pitchDec = {0:false,1:false,2:false,3:false};
BehringerCMDStudio4a.pitchInc = {0:false,1:false,2:false,3:false};


// *********************************************** Initialisation stuff. **********************************************

BehringerCMDStudio4a.vuMeterUpdate = function (value, group, control){
    value=(value*15)+48;
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
    // Initialise any LEDs that are direcctly controlled by this script.
    // Del buttons.
    midi.sendShortMsg(0x90, 0x2A, 0x00);
    midi.sendShortMsg(0x91, 0x4A, 0x00);
    midi.sendShortMsg(0x92, 0x2A, 0x00);
    midi.sendShortMsg(0x93, 0x4A, 0x00);
    // Scratch buttons.
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
    // Maybe not! This doesn't seem to work here!
//  engine.connectControl("[Master]","VuMeterL","BehringerCMDStudio4a.vuMeterUpdate",true);
//  engine.connectControl("[Master]","VuMeterR","BehringerCMDStudio4a.vuMeterUpdate",true);
}


// ******************************************** Control Stuff. ********************************************************
// NB: All the crap below is only requied because there is no "shift/mode" key functionality in Mixxx.
//     I suspect that the vast majority of controller mappings could be completed without any scripting at all if
//     Mixxx supported shift/mode buttons in the XML and standard deck (scratching) code.

// Function to deal with the del "shift" buttons.
BehringerCMDStudio4a.del = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.delState[channel]=!BehringerCMDStudio4a.delState[channel];
    midi.sendShortMsg(status, control, BehringerCMDStudio4a.delState[channel] ? 0x01 : 0x00);
}

// Functions to deal with the play button.
BehringerCMDStudio4a.playPush = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.delState[channel]) {
        // DEL is active, reverse play!
        engine.setValue(group, "reverse", 1);
    } else {
        // No DEL, just play.
        if (engine.getValue(group, "play")==0) {
            engine.setValue(group, "play", 1);
        } else {
            engine.setValue(group, "play", 0);
        }
    }
}
BehringerCMDStudio4a.playRelease = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.delState[channel]) {
        // DEL is active, reverse play release!
        engine.setValue(group, "reverse", 0);
    }
}


// Function to deal with the scratch "shift" buttons.
BehringerCMDStudio4a.scratch = function (channel, control, value, status, group) {
    if (BehringerCMDStudio4a.scratchState[channel]) {
        BehringerCMDStudio4a.scratchState[channel] = false;
        midi.sendShortMsg(status, control, 0x00);
    } else
    {
        BehringerCMDStudio4a.scratchState[channel] = true;
        midi.sendShortMsg(status, control, 0x01);
    }
}

// Functions to deal with the hot-queues.
BehringerCMDStudio4a.hotcuePush = function (channel, control, value, status, group) {
    // Translate the button to the actual hotcue.
    var hotcue = control-0x21;  // Hotcue buttons on left ddeck go from 0x22 to 0x29
    if (hotcue>8) {
        // We are dealing with the right deck, buttons are 0x20 higher so we need to compensate.
        hotcue = hotcue-0x20;
    }
    print(hotcue)
    if (BehringerCMDStudio4a.delState[channel]) {
        // Del "shift" is active, delete hotcue instead.
        engine.setValue(group, "hotcue_"+hotcue+"_clear", 1);
    } else {
        // Del "shift" is not active, do the push hotcue function as normal.
        engine.setValue(group, "hotcue_"+hotcue+"_activate", 1);
    }
}
BehringerCMDStudio4a.hotcueRelease = function (channel, control, value, status, group) {
    // Translate the button to the actual hotcue.
    var hotcue = control-0x21;  // Hotcue buttons on left ddeck go from 0x22 to 0x29
    if (hotcue>8) {
        // We are dealing with the right deck, buttons are 0x20 higher so we need to compensate.
        hotcue = hotcue-0x20;
    }
    if (BehringerCMDStudio4a.delState[channel]) {
        // Del "shift" is active, do nothing on button release.
    } else {
        // Del "shift" is not active, do the release hotcue function as normal.
        engine.setValue(group, "hotcue_"+hotcue+"_activate", 0);
    }
}

// Functions to deal with the pitch inc/dec buttons.
BehringerCMDStudio4a.pitchDecPush = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.pitchDec[channel] = true;
    if (BehringerCMDStudio4a.delState[channel]) {
        // Del "shift" is active, do "key" changes instead.
        // Check if the other button is pressed too, if so we reset the pitch.
        if (BehringerCMDStudio4a.pitchInc[channel]) {
            engine.setValue(group, "pitch", 0);
        } else {
            engine.setValue(group, "pitch_down", 1);
        }
    } else {
        // Del "shift" is not active, just do "rate" changes.
        engine.setValue(group, "rate_perm_down", 1);
        // Check if the other button is pressed too, if so we reset the pitch.
        if (BehringerCMDStudio4a.pitchInc[channel]) {
            engine.setValue(group, "rate", 0);
        }
    }
    // NB: We do the reset check for pitch before triggering the pitch down as Mixxx does not seem to like rapid rate switches.
    //     We trigger rate down regardless so as to preserve the GUI button press indicators.
}
BehringerCMDStudio4a.pitchDecRelease = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.pitchDec[channel] = false;
    engine.setValue(group, "rate_perm_down", 0);
}
BehringerCMDStudio4a.pitchIncPush = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.pitchInc[channel] = true;
    if (BehringerCMDStudio4a.delState[channel]) {
        // Del "shift" is active, do "key" changes instead.
        // Check if the other button is pressed too, if so we reset the pitch.
        if (BehringerCMDStudio4a.pitchDec[channel]) {
            engine.setValue(group, "pitch", 0);
        } else {
            engine.setValue(group, "pitch_up", 1);          
        }
    } else {
        // Del "shift" is not active, just do "rate" changes.
        engine.setValue(group, "rate_perm_up", 1);
        // Check if the other button is pressed too, if so we reset the pitch.
        if (BehringerCMDStudio4a.pitchDec[channel]) {
            engine.setValue(group, "rate", 0);
        }
    }
    // NB: We do the reset check for pitch before triggering the pitch up as Mixxx does not seem to like rapid rate switches.
    //     We trigger rate up regardless so as to preserve the GUI button press indicators.
}
BehringerCMDStudio4a.pitchIncRelease = function (channel, control, value, status, group) {
    BehringerCMDStudio4a.pitchInc[channel] = false;
    engine.setValue(group, "rate_perm_up", 0);
}

// Functions to deal with the wheel (i.e. scratcing and jog).
// NB: Why is there no standard support in Mixxx for this most basic of functions?
//     The vast majority of controller mappings use the same code (provided in the Wiki!) so why is this
//     most basic function not provided via an XML definition??
BehringerCMDStudio4a.wheelTouch = function (channel, control, value, status, group) {
    channel=channel+1;
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
    var deck=channel+1;
    var newValue=value-64;
    if (BehringerCMDStudio4a.scratchState[channel]){
        if (engine.isScratching(deck)){
            engine.scratchTick(deck,newValue);  // Scratch!
        }
    } else {
        engine.setValue(group, "jog", newValue); // Jog.
    }
};

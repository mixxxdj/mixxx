// Pioneer-DDJ-400-script.js
//
// ****************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-400.
// * Author: Warker
// * Version 0.1 (April 25 2019)
// * Forum: https://mixxx.org/forums/viewtopic.php?f=7&t=TBD
// * Wiki: https://mixxx.org/wiki/doku.php/pioneer_ddj_400

/*
            Working:
                * Mixer Section (Faders, EQ, Filter, Gain, Cue)
                * Browsing and loading + Waveform zoom (shift)
                * Jogwheels, Scratching, Bending
                * cycle Temporange
                * Beat Sync
                * Beat Loop Mode
                * Sampler Mode 
            
            Testing:
                * Keyboard Mode (check pitch value)
                * Keyshift Mode (check pitch value)
                * Beatjump mode
                * Hot Cue Mode (including loops)
                * Loop Section: Loop in / Out + Adjust, Call, Double, Half

            Partially:
                * PAD FX (only slots A-H, Q-P)
                
            Not working:
                * Effect Section (Left, right, fx select, level/depth)
                * Output (lights)

            TODO:
                * revive github account :)

            Changelog: 

            Version 0.1 - 13.05.2019:
                * added Keyboard mode (temporary shifting key by halftones - implemnted by pitch)
                * changed function Shift + PAD in Sampler Mode 
                    - while playing Sample stops play and jump to start 
                    - while not playing load selected track into PAD
                * added Keyshift Mode (Sampler +Shift)
                * added BROWSE +Shift - Waveform zoom +/-

            Version 0.2 - 15.05.2019:
                * added Beatjump Beatcount manipulation increase (pad 8 + shift), decrease (pad 7 +shift)
                * fixed a bug in beatloop with 1/4 and 1/2 Beat loops
                * fixed a bug in BEAT SYNC + Shift (cycle tempo range)
                * added support for saving and playing loops on Hot Cue Pads
                * added support for loop in + out adjust (while looping press and hold in or out + jog wheel to adjust the loop poit position)
                * fixed a bug in Tempo Range switch (Beat Sync + shift)
*/ 

//TODO: Functions that could be implemented to the script:
// ****************************************************************************

var PioneerDDJ400 = {};

// Save the Shift State 
PioneerDDJ400.shiftState = [0, 0];

// JogWheel 
PioneerDDJ400.vinylMode = true;
PioneerDDJ400.alpha = 1.0/8;
PioneerDDJ400.beta = PioneerDDJ400.alpha/32;
PioneerDDJ400.highspeedScale = 2;


PioneerDDJ400.tempoRanges = [ 0.06, 0.10, 0.16, 0.25 ]; // WIDE = 25%?

// Keyboard Mode Variables and Settings
PioneerDDJ400.keyboardHotCuePoint = [ 0, 0 ]; // selected HotCue point (eg. PAD) in Keyboard mode per Deck 0 = unset
PioneerDDJ400.keyboardModeRefCount = [ 0, 0 ]; // count the currently pressed Pads per Deck
PioneerDDJ400.halftoneToPadMap = [4, 5, 6, 7, 0, 1, 2, 3 ];

// Beatjump Pads
PioneerDDJ400.beatjumpPad = [-1, 1, -2, 2, -4, 4, -8, 8 ]; // < 0 = left; else right
PioneerDDJ400.beatjumpMulitplier = [ 1, 1 ]; // Beatjump Beatcount per Deck

// Hotcue Pads saved Loop points
PioneerDDJ400.hotcueLoopPoints = {
    '[Channel1]' : [],
    '[Channel2]' : []
};
    

// Modes
const mode = {
    HOTCUE : 0,
    BEATLOOP : 1,
    BEATJUMP : 2,
    SAMPLER : 3,
    KEYBOARD : 4,
    PADFX1 : 5,
    PADFX2 : 6,
    KEYSHIFT : 7 
};
// map control-value to mode
const modeMap = {
    0x1B : mode.HOTCUE,
    0x69 : mode.KEYBOARD,
    0x6D : mode.BEATLOOP,
    0x1E : mode.PADFX1,
    0x20 : mode.BEATJUMP,
    0x6B : mode.PADFX2,
    0x22 : mode.SAMPLER,
    0x6F : mode.KEYSHIFT 
};
// current Mode per Deck
PioneerDDJ400.currentMode = [mode.HOTCUE, mode.HOTCUE];

// Loop Section
PioneerDDJ400.loopin4beat = [ false, false ]; // inn4loop is pressed
PioneerDDJ400.loopout = [ false, false ]; // out loop is pressed
PioneerDDJ400.loopAdjustMultiply = 5;

PioneerDDJ400.init = function() {
    // init controler
    // init tempo Range to 10% (default = 6%)
    engine.setValue('[Channel1]', 'rateRange', PioneerDDJ400.tempoRanges[1]);
    engine.setValue('[Channel2]', 'rateRange', PioneerDDJ400.tempoRanges[1]);
    // Todo: set Mode on start!

};


PioneerDDJ400.jogTurn = function(channel, control, value, status, group) {
    const deckNum = channel + 1;
    // wheel center at 64; <64 rew >64 fwd
    var newVal = value - 64;

    // loop_in / out adjust
    const loopEnabled = engine.getValue(group, 'loop_enabled');
    if(loopEnabled > 0){
        if(PioneerDDJ400.loopin4beat[channel]){
            newVal = newVal * PioneerDDJ400.loopAdjustMultiply + engine.getValue(group, 'loop_start_position'); // multiply?
            engine.setValue(group, 'loop_start_position', newVal);
            return;
        }
        if(PioneerDDJ400.loopout[channel]){
            newVal = newVal * PioneerDDJ400.loopAdjustMultiply + engine.getValue(group, 'loop_end_position'); // multiply?
            engine.setValue(group, 'loop_end_position', newVal);
            return;
        } 
    }

    if(engine.isScratching(deckNum)){ 
        engine.scratchTick(deckNum, newVal);
    }
    else{ // fallback
        engine.setValue(group, 'jog', newVal);
    }

};


PioneerDDJ400.jogSearch = function(channel, control, value, status, group) {
    // "highspeed" (scaleup value) pitch bend
    const newVal = (value - 64) * PioneerDDJ400.highspeedScale;
    engine.setValue(group, 'jog', newVal);
};

PioneerDDJ400.jogTouch = function(channel, control, value, status, group) {
    const deckNum = channel + 1;   

    // skip scratchmode if we adjust the loop points
    if(PioneerDDJ400.loopin4beat[channel] || PioneerDDJ400.loopout[channel]){
        return;
    }

    // on touch jog with vinylmode enabled -> enable scratchmode
    if(value != 0 && PioneerDDJ400.vinylMode){
        engine.scratchEnable(deckNum, 800, 33+1/3, PioneerDDJ400.alpha, PioneerDDJ400.beta);
    }
    else{
        // on release jog (value==0) disable pitch bend mode or scratch mode
        engine.scratchDisable(deckNum);
    }
};

PioneerDDJ400.cycleTempoRange = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    const currRange = engine.getValue(group, 'rateRange');
    var idx = 0;

    for(var i = 0; i < PioneerDDJ400.tempoRanges.length; i++){
        if(currRange == PioneerDDJ400.tempoRanges[i]){
            idx = (i + 1) % PioneerDDJ400.tempoRanges.length;
            break;
        }
    }
    engine.setValue(group, 'rateRange', PioneerDDJ400.tempoRanges[idx]);
};

PioneerDDJ400.cueLoopCallLeft = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    const loop_on = engine.getValue(group, 'loop_enabled');
    
    if (loop_on){
        // loop halve
        engine.setValue(group, 'loop_scale', 0.5);
    }
    else{
        engine.setValue(group, 'loop_in_goto', 1);
    }
};

PioneerDDJ400.cueLoopCallRight = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    const loop_on = engine.getValue(group, 'loop_enabled');
    if (loop_on){
        // loop double
        engine.setValue(group, 'loop_scale', 2.0);
    }
    else{
        engine.setValue(group, 'loop_out_goto', 1);
    }
};

PioneerDDJ400.keyboardMode = function(channel, control, value, status, group){
    if(value > 0){
        // clear current set hotcue point and refcount for keyboard mode
        PioneerDDJ400.keyboardHotCuePoint[channel] = 0;
        PioneerDDJ400.keyboardModeRefCount[channel] = 0;
        // reset pitch
        engine.setValue(group, 'pitch', 0.0);
        // clear PAD LEDs of the Deck
    }
};

PioneerDDJ400.keyboardModePad = function(channel, control, value, status, group){
    channel = (channel & 0xf) < 10 ? 0 : 1;
    const padNum = (control & 0xf) + 1;
    var hotcuePad = PioneerDDJ400.keyboardHotCuePoint[channel];
    // if no hotcue is set for keyboard mode set on first press on a pad
    if(hotcuePad === 0 && value !== 0){
        hotcuePad = padNum;
        PioneerDDJ400.keyboardHotCuePoint[channel] = hotcuePad;
        // if there is no hotcue at this pad, set current play position
        const hotcuePos = engine.getValue(group, 'hotcue_'+hotcuePad+'_position');
        if(hotcuePos < 0){
            engine.setValue(group, 'hotcue_'+hotcuePad+'_set', 1);
        }
        PioneerDDJ400.keyboardModeRefCount[channel] = 0; // reset count
        // TODO enable LED of the Pad!
        return;
    }
        
    // if hotcue point is set perform coresponding halftone operation
    if(value > 0){
        // count pressed Pad per deck
        PioneerDDJ400.keyboardModeRefCount[channel] += 1;
        const newValue = PioneerDDJ400.halftoneToPadMap[padNum-1];
        engine.setValue(group, 'pitch', newValue);
        engine.setValue(group, 'hotcue_'+hotcuePad+'_gotoandplay', 1);
    }
    else{
        // decrease the number of active Pads, this should minimize unwanted stops
        PioneerDDJ400.keyboardModeRefCount[channel] -= 1;
        if(PioneerDDJ400.keyboardModeRefCount[channel] <= 0){
            engine.setValue(group, 'hotcue_'+hotcuePad+'_gotoandstop', 1);
            engine.setValue(group, 'pitch', 0.0); // reset pitch
            PioneerDDJ400.keyboardModeRefCount[channel] = 0; // reset refcount to 0
        }
    }
};

PioneerDDJ400.keyshiftModePad = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    const padNum = (control & 0xf) +1;
    engine.setValue(group, 'pitch', PioneerDDJ400.halftoneToPadMap[padNum-1]);
};

PioneerDDJ400.samplerModeShiftPadPressed = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    var playing = engine.getValue(group, 'play');
    // when playing stop and return to start/cue point
    if(playing > 0){
        engine.setValue(group, 'cue_gotoandstop', 1);
    }
    else{ // load selected track
        engine.setValue(group, 'LoadSelectedTrack', 1);
    }
};

PioneerDDJ400.beatjumpPadPressed = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    const padNum = (control & 0xf) + 1;
    const newVal = PioneerDDJ400.beatjumpPad[padNum-1] * PioneerDDJ400.beatjumpMulitplier;
    engine.setValue(group, 'beatjump', newVal);
};

PioneerDDJ400.beatjumpShiftByOne = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    var direction = status <= 0x98 ? -1 : 1;
    if ( direction == -1 && PioneerDDJ400.beatjumpMulitplier <= 1){
        direction = 0;
    }
    PioneerDDJ400.beatjumpMulitplier += direction;
};


PioneerDDJ400.shiftPressed = function(channel, control, value, status, group){
    PioneerDDJ400.shiftState[channel] = value;
};

PioneerDDJ400.modeChange = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    PioneerDDJ400.currentMode[channel] = modeMap[control];
};


PioneerDDJ400.hotcuePadPressed = function(channel, control, value, status, group){
    const loopPlaying = engine.getValue(group, 'loop_enabled');
    const padNum = (control & 0xf) +1;
    const loopPoint = PioneerDDJ400.hotcueLoopPoints[group][padNum-1];
    const hotcueSet = engine.getValue(group, 'hotcue_'+padNum+'_enabled');

    // play hotcue if set or set if not set and no loop is playing
    if(hotcueSet > 0 || (loopPlaying == 0 && (!loopPoint || loopPoint.start <= -1))){
        engine.setValue(group, 'hotcue_'+padNum+'_activate', value);
        return;
    }

    // play saved loop if set
    if(value > 0 && loopPoint && loopPoint.start >= 0){
        engine.setValue(group, 'loop_start_position', loopPoint.start);
        engine.setValue(group, 'loop_end_position', loopPoint.end);
        engine.setValue(group, 'loop_in_goto', 1);
        engine.setValue(group, 'reloop_toggle', 1);
        // This is a hack, TODO find a command like loop_in_goto_and_play
        if(engine.getValue(group, 'loop_enabled') == 0){
            engine.setValue(group, 'reloop_toggle', 1);
        }
        return;
    }

    // save playing loop on hotcue pad
    if(value > 0 && loopPlaying > 0){
        const loopStart = engine.getValue(group, 'loop_start_position');
        const loopEnd = engine.getValue(group, 'loop_end_position'); 
        PioneerDDJ400.hotcueLoopPoints[group][padNum-1] = {start: loopStart, end: loopEnd};
        return;
    }

};

PioneerDDJ400.hotcuePadShiftPressed = function(channel, control, value, status, group){
    const loopPlaying = engine.getValue(group, 'loop_enabled');
    const padNum = (control & 0xf) +1;
    const loopPoint = PioneerDDJ400.hotcueLoopPoints[group][padNum-1];
    
    // shift is pressed -> delete hotcue or loop point
    if(value > 0){
        engine.setValue(group, 'hotcue_'+padNum+'_clear', 1);
        if ( loopPoint && loopPoint.start >=0 ){
            PioneerDDJ400.hotcueLoopPoints[group][padNum-1] = {start: -1, end: -1};
            if(loopPlaying > 0 ){
                engine.setValue(group, 'reloop_toggle', 1);
            }
        }
        return;
    }
};

PioneerDDJ400.waveFormRotate = function(channel, control, value, status, group){
    // select the Waveform to zoom left shift = deck1, right shift = deck2
    const deckNum = PioneerDDJ400.shiftState[0] > 0 ? 1 : 2; 
    const oldVal = engine.getValue('[Channel'+deckNum+']', 'waveform_zoom');
    const newVal = oldVal + (value > 0x64 ? 1 : -1);
    engine.setValue('[Channel'+deckNum+']', 'waveform_zoom', newVal);
};

PioneerDDJ400.loopin4beatPressed = function(channel, control, value, status, group){
    const loopEnabled = engine.getValue(group, 'loop_enabled');
    PioneerDDJ400.loopin4beat[channel] = (value > 0);
    if(loopEnabled == 0 && value > 0){
        engine.setValue(group, 'loop_in', 1);
    }
};

PioneerDDJ400.loopin4beatPressedLong = function(channel, control, value, status, group){
    const loopEnabled = engine.getValue(group, 'loop_enabled');
    if(!loopEnabled && value > 0){
        engine.setValue(group, 'beatloop_4_activate', 1);
    }
};

PioneerDDJ400.loopoutPressed = function(channel, control, value, status, group){
    const loopEnabled = engine.getValue(group, 'loop_enabled');
    PioneerDDJ400.loopout[channel] = (value > 0);

    if(loopEnabled == 0 && value > 0){
        engine.setValue(group, 'loop_out', 1);
    }
};

PioneerDDJ400.shutdown = function() {

};

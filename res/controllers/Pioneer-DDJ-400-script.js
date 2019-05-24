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
                * Effect Section (Beat FX left + Right - select the Effect Slot (not Effect BPM))

            Partially:
                * PAD FX (only slots A-H, Q-P)
                * Effect Section (without Beat FX left + Right - no equivalent function found)
                * Output (lights)

            Not working/implemented:
                * Channel & Crossfader Start
                

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
             
            Version 0.3 - 18.05.2019:
                * added Effect Section funtions
                    - Beat FX left and right selects the Effect Slot in FX3
                    - Shift + Beat FX On/Off disables all Effect Slots
            
            version 0.4:
                * added Cue/Loop left and richt to navigate thru Loop / hotcue points
                * initial support for some LEDs (VuMeter, Play, Cue, Loop in/out, sync)

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
PioneerDDJ400.bendScale = 0.5;

PioneerDDJ400.pointJumpSpace = 0.005; // amount in percent of the Song we can jump back to previous Cue or loop point

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
 
// Beat FX 
PioneerDDJ400.beatFxEffect = 0;
PioneerDDJ400.beatFXEffectSlots = 3;

// Loop Section
PioneerDDJ400.loopin4beat = [ false, false ]; // inn4loop is pressed
PioneerDDJ400.loopout = [ false, false ]; // out loop is pressed
PioneerDDJ400.loopAdjustMultiply = 5;

PioneerDDJ400.init = function() {
    // init controler
    // init tempo Range to 10% (default = 6%)
    engine.setValue('[Channel1]', 'rateRange', PioneerDDJ400.tempoRanges[1]);
    engine.setValue('[Channel2]', 'rateRange', PioneerDDJ400.tempoRanges[1]);
    
    // enable effect fous on FX3 for Effect Section 
    engine.setValue('[EffectRack1_EffectUnit3]', 'show_focus', 1);

    // Connect the VU-Meter LEDS
    engine.connectControl('[Channel1]','VuMeter','PioneerDDJ400.vuMeterUpdate');
    engine.connectControl('[Channel2]','VuMeter','PioneerDDJ400.vuMeterUpdate');
    midi.sendShortMsg(0xB0, 0x02, 0); // reset vumeter
    midi.sendShortMsg(0xB1, 0x02, 0); // reset vumeter

};


PioneerDDJ400.jogTurn = function(channel, control, value, status, group) {
    const deckNum = channel + 1;
    // wheel center at 64; <64 rew >64 fwd
    var newVal = value - 64;

    // loop_in / out adjust
    const loopEnabled = engine.getValue(group, 'loop_enabled');
    if(loopEnabled > 0){
        if(this.loopin4beat[channel]){
            newVal = newVal * this.loopAdjustMultiply + engine.getValue(group, 'loop_start_position');
            engine.setValue(group, 'loop_start_position', newVal);
            return;
        }
        if(this.loopout[channel]){
            newVal = newVal * this.loopAdjustMultiply + engine.getValue(group, 'loop_end_position');
            engine.setValue(group, 'loop_end_position', newVal);
            return;
        } 
    }

    if(engine.isScratching(deckNum)){ 
        engine.scratchTick(deckNum, newVal);
    }
    else{ // fallback
        engine.setValue(group, 'jog', newVal * this.bendScale);
    }

};


PioneerDDJ400.jogSearch = function(channel, control, value, status, group) {
    // "highspeed" (scaleup value) pitch bend
    const newVal = (value - 64) * this.highspeedScale;
    engine.setValue(group, 'jog', newVal);
};

PioneerDDJ400.jogTouch = function(channel, control, value, status, group) {
    const deckNum = channel + 1;   

    // skip scratchmode if we adjust the loop points
    if(this.loopin4beat[channel] || this.loopout[channel]){
        return;
    }

    // on touch jog with vinylmode enabled -> enable scratchmode
    if(value != 0 && this.vinylMode){
        engine.scratchEnable(deckNum, 800, 33+1/3, this.alpha, this.beta);
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

    for(var i = 0; i < this.tempoRanges.length; i++){
        if(currRange == this.tempoRanges[i]){
            idx = (i + 1) % this.tempoRanges.length;
            break;
        }
    }
    engine.setValue(group, 'rateRange', this.tempoRanges[idx]);
};


function sortAsc(a, b){
    return a > b ? 1 : b > a ? -1 : 0; 
}

PioneerDDJ400.initCuePointsAndLoops = function(group){
    var points = [];
    for(var padNum = 1; padNum <= 8; padNum++){
        points.push( engine.getValue(group, 'hotcue_'+padNum+'_position'));
    }
    points.push( engine.getValue(group, 'cue_point'));
    points.push( engine.getValue(group, 'loop_start_position'));
    points.push( engine.getValue(group, 'loop_end_position'));
    points.sort(sortAsc); // sort asc
    return points;
};

PioneerDDJ400.cueLoopCallLeft = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    const loop_on = engine.getValue(group, 'loop_enabled');
    
    if (loop_on){
        // loop halve
        engine.setValue(group, 'loop_scale', 0.5);
    }
    else{
        const currentPosition = engine.getValue(group, 'playposition') - this.pointJumpSpace;
        const trackSamples = engine.getValue(group, 'track_samples');
        const points = this.initCuePointsAndLoops(group);
        var newPosition = currentPosition;
        for(var i = 1; i <= points.length; i++){
            if(i == points.length || points[i] >= currentPosition * trackSamples){
                newPosition = points[i-1] / trackSamples;
                break;
            }
        }
        //engine.setValue(group, 'loop_in_goto', 1);
        engine.setValue(group, 'playposition', newPosition);
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
        const currentPosition = engine.getValue(group, 'playposition');
        const trackSamples = engine.getValue(group, 'track_samples');
        const points = this.initCuePointsAndLoops(group);
        print('currPos: '+ currentPosition);
        print('trackSamples: '+ trackSamples);
        print('points: '+ points);
        var newPosition = currentPosition;
        for(var i = 0; i < points.length; i++){
            if(points[i] > currentPosition * trackSamples){
                newPosition = points[i] / trackSamples;
                break;
            }
        }
        print('newPos: '+ newPosition);
        engine.setValue(group, 'playposition', newPosition);
        //engine.setValue(group, 'loop_out_goto', 1);
    }
};

PioneerDDJ400.keyboardMode = function(channel, control, value, status, group){
    if(value > 0){
        // clear current set hotcue point and refcount for keyboard mode
        this.keyboardHotCuePoint[channel] = 0;
        this.keyboardModeRefCount[channel] = 0;
        // reset pitch
        engine.setValue(group, 'pitch', 0.0);
        this.keyboardModeEnabledOutput(channel, group);
    }
};


PioneerDDJ400.keyboardModeEnabledOutput = function(channel, group){
    const status = channel == 0 ? 0x97 : 0x99; 
    if(this.keyboardHotCuePoint[channel] == 0){
        for(var hotcuePad = 1; hotcuePad <= 8; hotcuePad++){
            var hotcueEnabled = engine.getValue(group, 'hotcue_'+hotcuePad+'_enabled');
            midi.sendShortMsg(status, 0x40 + hotcuePad-1, hotcueEnabled > 0 ? 0x7F : 0);
            // shift lights on if hotcue is set
            midi.sendShortMsg(status+1 , 0x40 + hotcuePad-1, hotcueEnabled > 0 ? 0x7F : 0);
        }
    }
    else{
        // enable all LEDs
        for(var hotcuePad = 1; hotcuePad <= 8; hotcuePad++){
            midi.sendShortMsg(status , 0x40 + hotcuePad-1, 0x7F);
        }
    }
    // shift keyboard Pad 7 and 8 are always enabled
    midi.sendShortMsg(status+1 , 0x46, 0x7F); 
    midi.sendShortMsg(status+1 , 0x47, 0x7F);
};


PioneerDDJ400.keyboardModePad = function(channel, control, value, status, group){
    channel = (channel & 0xf) < 10 ? 0 : 1;
    const padNum = (control & 0xf) + 1;
    var hotcuePad = this.keyboardHotCuePoint[channel];

    // if no hotcue is set for keyboard mode set on first press on a pad
    if(hotcuePad === 0 && value !== 0){
        hotcuePad = padNum;
        this.keyboardHotCuePoint[channel] = hotcuePad;
        // if there is no hotcue at this pad, set current play position
        const hotcuePos = engine.getValue(group, 'hotcue_'+hotcuePad+'_position');
        if(hotcuePos < 0){
            engine.setValue(group, 'hotcue_'+hotcuePad+'_set', 1);
        }
        this.keyboardModeRefCount[channel] = 0; // reset count
        this.keyboardModeEnabledOutput(channel, group);
        return;
    }
        
    // if hotcue point is set perform coresponding halftone operation
    if(value > 0){
        // count pressed Pad per deck
        this.keyboardModeRefCount[channel] += 1;
        const newValue = this.halftoneToPadMap[padNum-1];
        
        engine.setValue(group, 'pitch', newValue);
        engine.setValue(group, 'hotcue_'+hotcuePad+'_gotoandplay', 1);
    }
    else{
        // decrease the number of active Pads, this should minimize unwanted stops
        this.keyboardModeRefCount[channel] -= 1;
        if(this.keyboardModeRefCount[channel] <= 0){
            engine.setValue(group, 'hotcue_'+hotcuePad+'_gotoandstop', 1);
            engine.setValue(group, 'pitch', 0.0); // reset pitch
            this.keyboardModeRefCount[channel] = 0; // reset refcount to 0
        }
    }
};

PioneerDDJ400.keyshiftModePad = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    const padNum = (control & 0xf) +1;
    engine.setValue(group, 'pitch', this.halftoneToPadMap[padNum-1]);
};

PioneerDDJ400.samplerModeShiftPadPressed = function(channel, control, value, status, group){
    if(value == 0) {
        return; // ignore release
    }
    var playing = engine.getValue(group, 'play');
    // when playing stop and return to start/cue point
    if(playing > 0){
        engine.setValue(group, 'cue_gotoandstop', 1);
    }
    else{ // load selected track
        engine.setValue(group, 'LoadSelectedTrack', 1);
    }
    // TODO: while playing a sample blink playing PAD?
};

PioneerDDJ400.beatjumpPadPressed = function(channel, control, value, status, group){
    if(value == 0) {
        midi.sendShortMsg(status, control, 0); // turn off LED
        return; // ignore release
    }
    const padNum = (control & 0xf) + 1;
    const newVal = this.beatjumpPad[padNum-1] * this.beatjumpMulitplier;
    engine.setValue(group, 'beatjump', newVal);
    midi.sendShortMsg(status, control, 0x7f); // turn LED back on
};

PioneerDDJ400.beatjumpShiftByOne = function(channel, control, value, status, group){
    if(value == 0) {
        midi.sendShortMsg(status, control, 0); // turn off LED
        return; // ignore release
    }
    var direction = status <= 0x98 ? -1 : 1;
    if ( direction == -1 && this.beatjumpMulitplier <= 1){
        direction = 0;
    }
    this.beatjumpMulitplier += direction;
    midi.sendShortMsg(status, control, 0x7f); // turn LED back on
};


PioneerDDJ400.shiftPressed = function(channel, control, value, status, group){
    this.shiftState[channel] = value;
};


PioneerDDJ400.hotcuePadPressed = function(channel, control, value, status, group){
    const loopPlaying = engine.getValue(group, 'loop_enabled');
    const padNum = (control & 0xf) +1;
    const loopPoint = PioneerDDJ400.hotcueLoopPoints[group][padNum-1];
    const hotcueSet = engine.getValue(group, 'hotcue_'+padNum+'_enabled');

    // play hotcue if set or set if not set and no loop is playing
    if(hotcueSet > 0 || (loopPlaying == 0 && (!loopPoint || loopPoint.start <= -1))){
        engine.setValue(group, 'hotcue_'+padNum+'_activate', value);
        midi.sendShortMsg(status, control, 0x7f); // enable PAD LED
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
        this.hotcueLoopPoints[group][padNum-1] = {start: loopStart, end: loopEnd};
        midi.sendShortMsg(status, control, 0x7f); // enable pad LED
        return;
    }

};

PioneerDDJ400.hotcuePadShiftPressed = function(channel, control, value, status, group){
    const loopPlaying = engine.getValue(group, 'loop_enabled');
    const padNum = (control & 0xf) +1;
    const loopPoint = this.hotcueLoopPoints[group][padNum-1];
    
    // shift is pressed -> delete hotcue or loop point
    if(value > 0){
        midi.sendShortMsg(status-1, control, 0); // disable Pad LED
        engine.setValue(group, 'hotcue_'+padNum+'_clear', 1);
        if ( loopPoint && loopPoint.start >=0 ){
            this.hotcueLoopPoints[group][padNum-1] = {start: -1, end: -1};
            if(loopPlaying > 0 ){
                engine.setValue(group, 'reloop_toggle', 1);
            }
        }
        return;
    }
};

PioneerDDJ400.waveFormRotate = function(channel, control, value, status, group){
    // select the Waveform to zoom left shift = deck1, right shift = deck2
    const deckNum = this.shiftState[0] > 0 ? 1 : 2; 
    const oldVal = engine.getValue('[Channel'+deckNum+']', 'waveform_zoom');
    const newVal = oldVal + (value > 0x64 ? 1 : -1);
    engine.setValue('[Channel'+deckNum+']', 'waveform_zoom', newVal);
};

PioneerDDJ400.loopin4beatPressed = function(channel, control, value, status, group){
    const loopEnabled = engine.getValue(group, 'loop_enabled');
    this.loopin4beat[channel] = (value > 0);
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
    this.loopout[channel] = (value > 0);

    if(loopEnabled == 0 && value > 0){
        engine.setValue(group, 'loop_out', 1);
    }
};

PioneerDDJ400.beatFxLevelDepthRotate = function(channel, control, value, status, group){
    // which bit we want to use? MSB 0x02 or LSB 0x22
    const newVal = value == 0 ? 0 : (value / 0x7F);
    const effectOn = engine.getValue('[EffectRack1_EffectUnit3_Effect'+(this.beatFxEffect+1)+']', 'enabled');
    
    if(effectOn == 0){
        engine.setValue('[EffectRack1_EffectUnit3]', 'mix', newVal);
    }
    else{
        engine.setValue('[EffectRack1_EffectUnit3_Effect'+(this.beatFxEffect+1)+']', 'meta', newVal);
    }
};

PioneerDDJ400.beatFxSelectPressed = function(channel, control, value, status, group){
    engine.setValue('[EffectRack1_EffectUnit3_Effect'+(this.beatFxEffect+1)+']', 'next_effect', value);
};

PioneerDDJ400.beatFxSelectShiftPressed = function(channel, control, value, status, group){
    engine.setValue('[EffectRack1_EffectUnit3_Effect'+(this.beatFxEffect+1)+']', 'prev_effect', value);
};

PioneerDDJ400.beatFxLeftPressed = function(channel, control, value, status, group){
    if(this.beatFxEffect == 0) return; // dont cycle
    this.beatFxEffect = (this.beatFxEffect - (value & 0x1) ) % this.beatFXEffectSlots; 
    engine.setValue(group, 'focused_effect', this.beatFxEffect+1);
};

PioneerDDJ400.beatFxRightPressed = function(channel, control, value, status, group){
    if(this.beatFxEffect >= this.beatFXEffectSlots-1) return; // dont cycle
    this.beatFxEffect = (this.beatFxEffect + (value & 0x1) ) % this.beatFXEffectSlots; 
    engine.setValue(group, 'focused_effect', this.beatFxEffect+1);
};

PioneerDDJ400.beatFxOnOffPressed = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    const lastVal = engine.getValue('[EffectRack1_EffectUnit3_Effect'+(this.beatFxEffect+1)+']', 'enabled')
    engine.setValue('[EffectRack1_EffectUnit3_Effect'+(this.beatFxEffect+1)+']', 'enabled', !lastVal);
    engine.setValue(group, 'focused_effect', this.beatFxEffect+1);

    midi.sendShortMsg(status, control, !lastVal ? 0x7F : 0x00);
};

PioneerDDJ400.beatFxOnOffShiftPressed = function(channel, control, value, status, group){
    if(value == 0) return; // ignore release
    for(var i = 0; i < this.beatFXEffectSlots; i++){
        engine.setValue('[EffectRack1_EffectUnit3_Effect'+(i+1)+']', 'enabled', 0);
    }

    midi.sendShortMsg(status, control, 0);
};

PioneerDDJ400.vuMeterUpdate = function(value, group, control){
    const newVal = value * 150;
    switch(group){
        case '[Channel1]':
            midi.sendShortMsg(0xB0, 0x02, newVal);
            break;

        case '[Channel2]':
            midi.sendShortMsg(0xB1, 0x02, newVal);
            break;
    }
    
};

PioneerDDJ400.shutdown = function() {
    midi.sendShortMsg(0xB0, 0x02, 0); // reset vumeter
    midi.sendShortMsg(0xB1, 0x02, 0); // reset vumeter
};

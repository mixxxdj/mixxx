//Controller
var INPULSE500 = {};

//enables/disables vinyl mode
var vinylEnabled = [
    1,
    1,
];

//This helps implement the beatmatch guide
var leader = 0;

//These store the timestamp when the song started playing
var timestamps = [
    Number.MAX_VALUE,
    Number.MAX_VALUE,
];

//These store if the loop in and loop out markers are set
var loopIn = 0;
var loopOut = 0;

//These store if the slicer mode is on
var slicer = [
    0,
    0,
];

//These store the beat the song should be playing
var slicerBeat = [
    0,
    0,
];

//These store the beat the song is playing
var slicerBeatPlaying = [
    0,
    0,
];

//these store if the slicer needs to jump
var slicerJump = [
    false,
    false,
];

//these store the position the slicer needs to jump to
var slicerJumpPos = [
    0,
    0,
];

//these store if the slicer recently jumped
var jumped = [
    false,
    false,
];

//loopsizes used by the loop roll pads
const loopSizes = [0.125, 0.25, 0.5, 1, 2, 4, 8, 16];

//These store the last beat played timestamp in milliseconds to implement the beatmatch guide
var beatTimes = [
    0,
    0,
];

var lastDelta = 0;

//Init function
INPULSE500.init = function() {
    //turn on vinyl mode
    midi.sendShortMsg(0x91, 0x03, 0x7F);
    midi.sendShortMsg(0x92, 0x03, 0x7F);
    //looproll pads (green)
    for (let i = 0; i < 8; i++) {
        midi.sendShortMsg(0x96, 0x10+i, 16);
        midi.sendShortMsg(0x97, 0x10+i, 16);
    }
    //slicer pads (red)
    for (let i = 0; i < 8; i++) {
        midi.sendShortMsg(0x96, 0x20+i, 64);
        midi.sendShortMsg(0x97, 0x20+i, 64);
    }
    
    //callback connections
    engine.makeConnection("[Channel1]", "play", channelLEDsCallback);
    engine.makeConnection("[Channel2]", "play", channelLEDsCallback);
    engine.makeConnection("[Channel1]", "volume", channelLEDsCallback);
    engine.makeConnection("[Channel2]", "volume", channelLEDsCallback);
    engine.makeConnection("[Master]", "crossfader", channelLEDsCallback);
    engine.makeConnection("[Channel1]", "orientation", channelLEDsCallback);
    engine.makeConnection("[Channel1]", "bpm", beatmatchHelperCallback);
    engine.makeConnection("[Channel2]", "bpm", beatmatchHelperCallback);
    engine.makeConnection("[Channel1]", "play", beatmatchHelperCallback);
    engine.makeConnection("[Channel2]", "play", beatmatchHelperCallback);
    engine.makeUnbufferedConnection("[Channel1]", "beat_active", beatmatchHelperCallback);
    engine.makeUnbufferedConnection("[Channel2]", "beat_active", beatmatchHelperCallback);
    engine.makeConnection("[Channel1]", "hotcue_1_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_2_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_3_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_4_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_5_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_6_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_7_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_8_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_1_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_2_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_3_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_4_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_5_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_6_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_7_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_8_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_1_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_2_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_3_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_4_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_5_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_6_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_7_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_8_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_1_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_2_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_3_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_4_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_5_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_6_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_7_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "hotcue_8_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_1_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_2_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_3_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_4_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_5_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_6_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_7_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_8_set", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_1_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_2_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_3_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_4_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_5_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_6_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_7_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_8_clear", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_1_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_2_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_3_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_4_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_5_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_6_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_7_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_8_activate", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_1_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_2_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_3_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_4_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_5_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_6_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_7_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel2]", "hotcue_8_color", hotcueLEDsCallback);
    engine.makeConnection("[Channel1]", "play", getLeaderCallback);
    engine.makeConnection("[Channel2]", "play", getLeaderCallback);
    engine.makeConnection("[Channel1]", "cue_default", getLeaderCallback);
    engine.makeConnection("[Channel2]", "cue_default", getLeaderCallback);
    engine.makeConnection("[Channel1]", "LoadSelectedTrack", getLeaderCallback);
    engine.makeConnection("[Channel2]", "LoadSelectedTrack", getLeaderCallback);
    engine.makeConnection("[Channel1]", "playposition", getLeaderCallback);
    engine.makeConnection("[Channel2]", "playposition", getLeaderCallback);
    engine.makeConnection("[Channel1]", "loop_in", loopLEDsCallback);
    engine.makeConnection("[Channel2]", "loop_in", loopLEDsCallback);
    engine.makeConnection("[Channel1]", "loop_out", loopLEDsCallback);
    engine.makeConnection("[Channel2]", "loop_out", loopLEDsCallback);
    engine.makeConnection("[Channel1]", "loop_enabled", loopLEDsCallback);
    engine.makeConnection("[Channel2]", "loop_enabled", loopLEDsCallback);
    engine.makeConnection("[Channel1]", "VuMeter", vuMeterCallback);
    engine.makeConnection("[Channel2]", "VuMeter", vuMeterCallback);
    engine.makeUnbufferedConnection("[Channel1]", "beat_active", beatLEDCallback);
    engine.makeUnbufferedConnection("[Channel2]", "beat_active", beatLEDCallback);
    engine.makeConnection("[Sampler1]", "cue_gotoandplay", samplerLEDCallback);
    engine.makeConnection("[Sampler1]", "cue_default", samplerLEDCallback);
    engine.makeConnection("[Sampler1]", "eject", samplerLEDCallback);
    engine.makeConnection("[Sampler1]", "track_loaded", samplerLEDCallback);
    engine.makeConnection("[Sampler2]", "cue_gotoandplay", samplerLEDCallback);
    engine.makeConnection("[Sampler2]", "cue_default", samplerLEDCallback);
    engine.makeConnection("[Sampler2]", "eject", samplerLEDCallback);
    engine.makeConnection("[Sampler2]", "track_loaded", samplerLEDCallback);
    engine.makeConnection("[Sampler3]", "cue_gotoandplay", samplerLEDCallback);
    engine.makeConnection("[Sampler3]", "cue_default", samplerLEDCallback);
    engine.makeConnection("[Sampler3]", "eject", samplerLEDCallback);
    engine.makeConnection("[Sampler3]", "track_loaded", samplerLEDCallback);
    engine.makeConnection("[Sampler4]", "cue_gotoandplay", samplerLEDCallback);
    engine.makeConnection("[Sampler4]", "cue_default", samplerLEDCallback);
    engine.makeConnection("[Sampler4]", "eject", samplerLEDCallback);
    engine.makeConnection("[Sampler4]", "track_loaded", samplerLEDCallback);
    engine.makeConnection("[Sampler5]", "cue_gotoandplay", samplerLEDCallback);
    engine.makeConnection("[Sampler5]", "cue_default", samplerLEDCallback);
    engine.makeConnection("[Sampler5]", "eject", samplerLEDCallback);
    engine.makeConnection("[Sampler5]", "track_loaded", samplerLEDCallback);
    engine.makeConnection("[Sampler6]", "cue_gotoandplay", samplerLEDCallback);
    engine.makeConnection("[Sampler6]", "cue_default", samplerLEDCallback);
    engine.makeConnection("[Sampler6]", "eject", samplerLEDCallback);
    engine.makeConnection("[Sampler6]", "track_loaded", samplerLEDCallback);
    engine.makeConnection("[Sampler7]", "cue_gotoandplay", samplerLEDCallback);
    engine.makeConnection("[Sampler7]", "cue_default", samplerLEDCallback);
    engine.makeConnection("[Sampler7]", "eject", samplerLEDCallback);
    engine.makeConnection("[Sampler7]", "track_loaded", samplerLEDCallback);
    engine.makeConnection("[Sampler8]", "cue_gotoandplay", samplerLEDCallback);
    engine.makeConnection("[Sampler8]", "cue_default", samplerLEDCallback);
    engine.makeConnection("[Sampler8]", "eject", samplerLEDCallback);
    engine.makeConnection("[Sampler8]", "track_loaded", samplerLEDCallback);
    engine.makeUnbufferedConnection("[Channel1]", "beat_active", slicerPadsCallback);
    engine.makeUnbufferedConnection("[Channel2]", "beat_active", slicerPadsCallback);
    engine.makeUnbufferedConnection("[Channel1]", "beat_active", slicerJumpsCallback);
    engine.makeUnbufferedConnection("[Channel2]", "beat_active", slicerJumpsCallback);
};

//shutdown
INPULSE500.shutdown = function() {

};

//Scroll through library with browse knob
INPULSE500.browseLibrary = function(channel, control, value) {
    let newValue;
    if (value-64 > 0) { newValue = value-128; } else { newValue = value; }
    engine.setValue("[Library]", "MoveVertical", newValue);
};

//Nudge track for beat matching when wheel is turned
INPULSE500.nudge = function(channel, control, value, status, group) {
    const newValue = value-64;
    // 0.5 * val / 64 seems to yield the best results
    engine.setValue(group, "jog", -1*newValue/64);
};

//Start a beatloop when autoloop is pressed
INPULSE500.beatloop = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (engine.getParameter(group, "loop_enabled") === 1) {
            engine.setParameter(group, "reloop_toggle", 1);
        } else {
            engine.setParameter(group, "beatloop_activate", 1);
        }
    }
};

//The button that enables/disables scratching
INPULSE500.wheelTouch = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const alpha = 1.0/8;
    const beta = alpha/32;
    if (value === 0x7F && vinylEnabled[status-0x91] === 1) {
        engine.scratchEnable(deckNumber, 16384, 33+1/3, alpha, beta);
    } else {
        engine.scratchDisable(deckNumber);
    }
};

//The wheel that actually controls the scratching
INPULSE500.wheelTurn = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    const newValue = value - 64;
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, -newValue);
    }
};

//multiply/divide loop size when autoloop wheel is turned
INPULSE500.autoloopMultiply = function(channel, control, value, status, group) {
    const newValue = value-64;
    if (newValue > 0) {
        engine.setParameter(group, "loop_halve", 1);
    }
    if (newValue < 0) {
        engine.setParameter(group, "loop_double", 1);
    }
};

INPULSE500.vinylEnable = function(channel, control, value, status) {
    if (value === 0x7F) {
        vinylEnabled[status-0x91] = 1 - vinylEnabled[status-0x91];
        midi.sendShortMsg(status, 0x03, 0x7F*vinylEnabled[status-0x91]);
    }
};

//change tempo rate in 8% increments (32% max) with shift + keylock button
INPULSE500.setTempoRange = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        let curr = engine.getValue(group, "rateRange");
        if (curr > 0.31) {
            curr = 0.08;
        } else {
            curr += 0.08;
        }
        engine.setValue(group, "rateRange", curr);
    }
};

//move quickly through the track when shift + jog wheel touch + turn
INPULSE500.searchTrack = function(channel, control, value, status, group) {
    const newValue = value-64;
    engine.setValue(group, "playposition", engine.getValue(group, "playposition")-0.00001*newValue);
};

//move the current loop by one beat when shift + in/out
INPULSE500.loopMove = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (control === 0x09) {
            engine.setValue(group, "loop_move", -1);
        }
        if (control === 0x0A) {
            engine.setValue(group, "loop_move", 1);
        }
    }
};

//move the beat markers to match the beats with shift + auto loop
INPULSE500.adjustGrid = function(channel, control, value, status, group) {
    let newValue;
    if (value-64 > 0) { newValue = value-128; } else { newValue = value; }
    if (newValue > 0) {
        engine.setValue(group, "beats_translate_later", 1);
    }
    if (newValue < 0) {
        engine.setValue(group, "beats_translate_earlier", 1);
    }
};

//this opens/closes directories when browsing the sidebar and shift + turn the browse wheel
INPULSE500.browseSidebar = function(channel, control, value) {
    let newValue;
    if (value-64 > 0) { newValue = value-128; } else { newValue = value; }
    if (newValue > 0) {
        engine.setParameter("[Library]", "MoveRight", 1);
    }
    if (newValue < 0) {
        engine.setParameter("[Library]", "MoveLeft", 1);
    }
};


//this turns on/off crossfader when hardware switch is toggled
INPULSE500.toggleCrossfader = function(channel, control, value) {
    if (value === 0x7F) {
        engine.setValue("[Channel1]", "orientation", 0);
        engine.setValue("[Channel2]", "orientation", 2);
    } else {
        engine.setValue("[Channel1]", "orientation", 1);
        engine.setValue("[Channel2]", "orientation", 1);
    }
};

//this triggers a looproll with the loop pads
INPULSE500.toggleLoopPad = function(channel, control, value, status, group) {
    const pad = control - 0x10;
    engine.setParameter(group, "beatlooproll_X_activate".replace("X", loopSizes[pad]), value/0x7F);
    midi.sendShortMsg(0x90+channel, 0x10+pad, value === 0x7F ? 28 : 16);
};

//this enables a slicer loop
INPULSE500.slicerOn = function(channel, control, value, status, group) {
    channel = group === "[Channel1]" ? 0 : 1;
    if (value === 0x7F) {
        if (slicer[channel] === 0) {
            slicer[channel] = 1;
            slicerBeat[channel] = 0;
            midi.sendShortMsg(0x96+channel, 0x20, 96);
        } else {
            slicer[channel] = 0;
            midi.sendShortMsg(0x96+channel, 0x20+slicerBeat[channel], 64);
        }
    }
};

//this queues a slicer jump
INPULSE500.slicer = function(channel, control, value, status, group) {
    channel = group === "[Channel1]" ? 0 : 1;
    if (value === 0x7F) {
        if (slicer[channel] === 1) {
            midi.sendShortMsg(0x96+channel, control, 96);
            slicerJump[channel] = true;
            slicerJumpPos[channel] = control - 0x20;
        }
    }  else {
        midi.sendShortMsg(0x96+channel, control, 64);
    }
};

//this lights up deck numbers when playing in master
const channelLEDsCallback = function() {
    const channel1on = (engine.getParameter("[Channel1]", "play") === 1 && engine.getValue("[Channel1]", "volume") > 0) ? 1 : 0;
    const channel2on = (engine.getParameter("[Channel2]", "play") === 1 && engine.getValue("[Channel2]", "volume") > 0) ? 1 : 0;
    if (engine.getValue("[Channel1]", "orientation") === 1) {
        midi.sendShortMsg(0x91, 0x30, 0x7F*channel1on);
        midi.sendShortMsg(0x92, 0x30, 0x7F*channel2on);
    } else {
        if (channel1on && engine.getValue("[Master]", "crossfader") < 1) {
            midi.sendShortMsg(0x91, 0x30, 0x7F);
        } else {
            midi.sendShortMsg(0x91, 0x30, 0x00);
        }
        if (channel2on && engine.getValue("[Master]", "crossfader") > -1) {
            midi.sendShortMsg(0x92, 0x30, 0x7F);
        } else {
            midi.sendShortMsg(0x92, 0x30, 0x00);
        }
    }
};

//this lights up the beatmatch helper LEDs
const beatmatchHelperCallback = function(value, group, control) {
    //only get diff when beat is active
    if (value === 0) {
        return;
    }
    //get tempos
    const bpm1 = engine.getValue("[Channel1]", "bpm");
    const bpm2 = engine.getValue("[Channel2]", "bpm");

    //leader gets tempo LED and turns off all arrows
    midi.sendShortMsg(0x91+leader, 0x2C, 0x7F);
    midi.sendShortMsg(0x91+leader, 0x1C, 0x00);
    midi.sendShortMsg(0x91+leader, 0x1D, 0x00);
    midi.sendShortMsg(0x91+leader, 0x1E, 0x00);
    midi.sendShortMsg(0x91+leader, 0x1F, 0x00);

    if (bpm1 === bpm2) {
        //tempo LED on follower
        midi.sendShortMsg(0x91+(1-leader), 0x2C, 0x7F);
        //turn off tempo arrows since tempos are the same
        midi.sendShortMsg(0x91+(1-leader), 0x1E, 0x00);
        midi.sendShortMsg(0x91+(1-leader), 0x1F, 0x00);

        if (control === "beat_active") {
            const channel = group === "[Channel1]" ? 0 : 1;
            beatTimes[channel] = Date.now();
            if (channel !== leader) {
                const delta = beatTimes[leader] - beatTimes[1-leader];
                const bpm = engine.getValue(group, "bpm");
                const millis = 1/(bpm/60000);
                const avgDelta = (lastDelta + delta)/2;
                lastDelta = delta;
                //beats are matched
                if (Math.abs(avgDelta/millis) < 0.1) {
                    midi.sendShortMsg(0x91+leader, 0x2D, 0x7F);
                    midi.sendShortMsg(0x91+(1-leader), 0x2D, 0x7F);
                    midi.sendShortMsg(0x91+(1-leader), 0x1C, 0x00);
                    midi.sendShortMsg(0x91+(1-leader), 0x1D, 0x00);
                } else {
                    //find if lagging or following
                    const sign = avgDelta > 0 ? 0 : 1;
                    //turn off beat match LEDs
                    midi.sendShortMsg(0x91+leader, 0x2D, 0x00);
                    midi.sendShortMsg(0x91+(1-leader), 0x2D, 0x00);
                    //turn on corresponding beat match guide LEDs
                    if (Math.abs(avgDelta/millis) > 0.5) {
                        midi.sendShortMsg(0x91+(1-leader), 0x1C+(1-sign), 0x00);
                        midi.sendShortMsg(0x91+(1-leader), 0x1C+(sign), 0x7F);
                    } else {
                        midi.sendShortMsg(0x91+(1-leader), 0x1C+(sign), 0x00);
                        midi.sendShortMsg(0x91+(1-leader), 0x1C+(1-sign), 0x7F);
                    }
                }
            }
        }

    } else {
        //turn off tempo and beat match LEDs
        midi.sendShortMsg(0x91+(1-leader), 0x2C, 0x00);
        midi.sendShortMsg(0x91+(1-leader), 0x2D, 0x00);
        const gr = bpm1 > bpm2 ? 1 : 0;
        //turn on corresponding tempo arrows
        midi.sendShortMsg(0x91, 0x1E, 0x7F*(leader)*gr);
        midi.sendShortMsg(0x91, 0x1F, 0x7F*(leader)*(1-gr));
        midi.sendShortMsg(0x92, 0x1E, 0x7F*(1-leader)*(1-gr));
        midi.sendShortMsg(0x92, 0x1F, 0x7F*(1-leader)*gr);
    }
};


//callback to color and light up hotcue pads
const hotcueLEDsCallback = function(value, group, control){
    const channel = group === "[Channel1]" ? 0 : 1;
    const hotcue = Number(control.match(/\d+/)[0]);
    const colorRGB = engine.getParameter(group, "hotcue_X_color".replace("X", hotcue));
    let r = Math.floor(colorRGB / (256*256));
    let g = Math.floor(colorRGB / 256) % 256;
    let b = colorRGB % 256;
    midiValHigh = Math.round(3*r/255) * 32 + Math.round(7*g/255) * 4 + Math.round(3*b/256);
    midiValLow = Math.round(3*(r/2)/255) * 32 + Math.round(7*(g/2)/255) * 4 + Math.round(3*(b/2)/256);
    if (control.includes("color")){
        midi.sendShortMsg(0x96+channel, hotcue-1, midiValLow);
        midi.sendShortMsg(0x98+channel, 8+hotcue-1, midiValLow);
    }
    if (control.includes("activate")){
        if (engine.getParameter(group, control) === 1) {
            midi.sendShortMsg(0x96+channel, hotcue-1, midiValHigh);
        } else {
            midi.sendShortMsg(0x96+channel, hotcue-1, midiValLow);
        }
        midi.sendShortMsg(0x98+channel, 8+hotcue-1, midiValLow);
    } else {
        if (engine.getParameter(group, control) === 1) {
            midi.sendShortMsg(0x96+channel, hotcue-1, midiValLow);
            midi.sendShortMsg(0x98+channel, 8+hotcue-1, midiValLow);
        } else if (control.includes("clear")){
            midi.sendShortMsg(0x96+channel, hotcue-1, 0x00);
            midi.sendShortMsg(0x98+channel, 8+hotcue-1, 0x00);
        }
    }
};

//this gets the leading song for beatmatch guide
const getLeaderCallback = function(value, group, control) {
    const channel = group === "[Channel1]" ? 0 : 1;
    if (control === "LoadSelectedTrack") {
        timestamps[channel] = Number.MAX_VALUE;
    }
    if (value === 1 && (control === "play" || control === "cue_default")) {
        timestamps[channel] = Date.now();
    }
    if (value === 1 && control === "playposition") {
        timestamps[channel] = Number.MAX_VALUE;
    }
    leader = timestamps[1] > timestamps[0] ? 0 : 1;
};

//callback to turn on/off loop in/out LEDs based on clicks and loop enabled/disabled
const loopLEDsCallback = function(value, group, control) {
    //loop in clicked or autoloop
    if (control === "loop_in" || engine.getParameter(group, "loop_enabled") === 1) {
        loopIn = 1;
    }
    //loop out clicked or autoloop
    if (control === "loop_out" || engine.getParameter(group, "loop_enabled") === 1) {
        loopOut = 1;
    }
    //loop was disabled
    if (control === "loop_enabled" && engine.getParameter(group, "loop_enabled") === 0) {
        loopIn = 0;
        loopOut = 0;
    }
    const channel = group === "[Channel1]" ? 0 : 1;
    midi.sendShortMsg(0x91+channel, 0x09, loopIn*0x7F);
    midi.sendShortMsg(0x91+channel, 0x0A, loopOut*0x7F);
};

//callback to update vumeter
const vuMeterCallback = function(value, group) {
    const channel = group === "[Channel1]" ? 0 : 1;
    midi.sendShortMsg(0xB1+channel, 0x40, value*0x7F);
};

//callback for the beat led
const beatLEDCallback = function(value, group) {
    if (group === "[ChannelX]".replace("X", leader+1)) {
        midi.sendShortMsg(0x90, 0x05, value*0x63);
    }
};

//callback to rotate the slicer pad
const slicerPadsCallback = function(value, group) {
    const channel = group === "[Channel1]" ? 0 : 1;
    if (value === 1) {
        if (slicer[channel] === 1) {
            midi.sendShortMsg(0x96+channel, 0x20+slicerBeat[channel], 64);
            slicerBeat[channel]++;
            slicerBeat[channel] = slicerBeat[channel] % 8;
            if (slicerBeat[channel] === 0) {
                engine.setValue(group, "beatjump", -8);
            }
            midi.sendShortMsg(0x96+channel, 0x20+slicerBeat[channel], 96);
        }
    }
};

//callback for slicer jumps
const slicerJumpsCallback = function(value, group) {
    const channel = group === "[Channel1]" ? 0 : 1;
    if (value === 1) {
        if (slicerJump[channel]) {
            slicerJump[channel] = false;
            const distance = slicerJumpPos[channel] - slicerBeatPlaying[channel];
            engine.setValue(group, "beatjump", distance-1);
            slicerBeatPlaying[channel] = slicerJumpPos[channel];
            jumped[channel] = 1;
        } else if (jumped[channel]) {
            jumped[channel] = 0;
            const distance = slicerBeatPlaying[channel] - slicerBeat[channel];
            engine.setValue(group, "beatjump", -distance-1);
            slicerBeatPlaying[channel] = slicerBeat[channel] + 1;
        } else {
            slicerBeatPlaying[channel] = slicerBeat[channel];
        }
    }
};

//callback for sampler leds
const samplerLEDCallback = function(value, group, control) {
    const sample = Number(group.match(/\d+/)[0])-1;
    if (control === "eject" && value === 1){
        midi.sendShortMsg(0x96, 0x30+sample, 0x00);
        midi.sendShortMsg(0x96, 0x38+sample, 0x00);
        midi.sendShortMsg(0x97, 0x30+sample, 0x00);
        midi.sendShortMsg(0x97, 0x38+sample, 0x00);
    }
    if (control === "track_loaded" && value === 1){
        midi.sendShortMsg(0x96, 0x30+sample, 18);
        midi.sendShortMsg(0x96, 0x38+sample, 18);
        midi.sendShortMsg(0x97, 0x30+sample, 18);
        midi.sendShortMsg(0x97, 0x38+sample, 18);
    }
    if (control === "cue_gotoandplay" && value === 1 && engine.getParameter(group, "track_loaded") === 1){
        midi.sendShortMsg(0x96, 0x30+sample, 31);
        midi.sendShortMsg(0x96, 0x38+sample, 31);
        midi.sendShortMsg(0x97, 0x30+sample, 31);
        midi.sendShortMsg(0x97, 0x38+sample, 31);
    }
    if (control === "cue_default" && value === 1){
        midi.sendShortMsg(0x96, 0x30+sample, 18);
        midi.sendShortMsg(0x96, 0x38+sample, 18);
        midi.sendShortMsg(0x97, 0x30+sample, 18);
        midi.sendShortMsg(0x97, 0x38+sample, 18);
    }
};

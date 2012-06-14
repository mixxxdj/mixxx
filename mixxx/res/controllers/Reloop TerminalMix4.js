/****************************************************************
 *      Reloop TerminalMix4 MIDI controller script v1.00        *
 *          Copyright (C) 2012, Sean M. Pappalardo              *
 *      but feel free to tweak this to your heart's content!    *
 *      For Mixxx version 1.11.x                                *
 ****************************************************************/

function TerminalMix4() {}

// ----------   Customization variables ----------
//      See http://mixxx.org/wiki/doku.php/reloop_terminalmix4_user_guide  for details
TerminalMix4.pitchRanges = [ 0.08, 0.12, 0.25, 0.5, 1.0 ];

// ----------   Other global variables    ----------
TerminalMix4.timers = [];
TerminalMix4.state = [];
TerminalMix4.faderStart = [];
TerminalMix4.lastFader = [];   // Last value of each channel/cross fader
TerminalMix4.lastEQs = [[]];
TerminalMix4.traxKnobMode = "tracks";

// ----------   Functions   ----------

TerminalMix4.init = function (id,debug) {
    TerminalMix4.id = id;
    
    // Extinguish all LEDs
    for (var i=0; i<=3; i++) {  // 4 decks
        for (var j=1; j<=120; j++) {
            midi.sendShortMsg(0x90+i,j,0x00);
        }
    }
    
    // Set soft-takeover for all Sampler volumes
    for (var i=engine.getValue("[Master]","num_samplers"); i>=1; i--) {
        engine.softTakeover("[Sampler"+i+"]","pregain",true);
    }
    // Set soft-takeover for all applicable Deck controls
    for (var i=engine.getValue("[Master]","num_decks"); i>=1; i--) {
        engine.softTakeover("[Channel"+i+"]","volume",true);
        engine.softTakeover("[Channel"+i+"]","filterHigh",true);
        engine.softTakeover("[Channel"+i+"]","filterMid",true);
        engine.softTakeover("[Channel"+i+"]","filterLow",true);
    }

    engine.softTakeover("[Master]","crossfader",true);

    engine.connectControl("[Channel1]","beat_active","TerminalMix4.tapLEDL");
    engine.connectControl("[Channel2]","beat_active","TerminalMix4.tapLEDR");

    TerminalMix4.timers["fstartflash"] = -1;
//     TerminalMix4.timers["qtrSec"] = engine.beginTimer(250,"TerminalMix4.qtrSec");
    TerminalMix4.timers["halfSec"] = engine.beginTimer(500,"TerminalMix4.halfSec");

    if (TerminalMix4.traxKnobMode == "tracks") {
        midi.sendShortMsg(0x90,0x37,0x7F);  // light Back button
    }

    print ("Reloop TerminalMix4: "+id+" initialized.");
}

TerminalMix4.shutdown = function () {
    // Stop all timers
    for (var i=0; i<TerminalMix4.timers.length; i++) {
        engine.stopTimer(TerminalMix4.timers[i]);
    }
    // Extinguish all LEDs
    for (var i=0; i<=3; i++) {  // 4 decks
        for (var j=1; j<=120; j++) {
            midi.sendShortMsg(0x90+i,j,0x00);
        }
    }
    print ("Reloop TerminalMix4: "+TerminalMix4.id+" shut down.");
}

TerminalMix4.qtrSec = function () {
    
}

TerminalMix4.halfSec = function () {
    TerminalMix4.faderStartFlash();
    TerminalMix4.samplerPlayFlash();
    TerminalMix4.activeLoopFlash();
}

// The button that enables/disables scratching
TerminalMix4.wheelTouch = function (channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    if (value == 0x7F) {
        var alpha = 1.0/8;
        var beta = alpha/32;
        engine.scratchEnable(deck, 800, 33+1/3, alpha, beta);
    }
    else {    // If button up
        engine.scratchDisable(deck);
    }
}

// The wheel that actually controls the scratching
TerminalMix4.wheelTurn = function (channel, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var newValue=(value-64);
    // See if we're scratching. If not, do wheel jog.
    if (!engine.isScratching(deck)) {
        engine.setValue(group, "jog", newValue/4);
        return;
    }

    // Register the movement
    engine.scratchTick(deck,newValue);
}

TerminalMix4.samplerVolume = function (channel, control, value) {
    // Link all sampler volume controls to the Sampler Volume knob
    for (var i=engine.getValue("[Master]","num_samplers"); i>=1; i--) {
        engine.setValue("[Sampler"+i+"]","pregain",
                        script.absoluteNonLin(value, 0.0, 1.0, 4.0));
    }
}

TerminalMix4.pitchRange = function (channel, control, value, status, group) {
    midi.sendShortMsg(status,control,value); // Make button light or extinguish
    if (value<=0) return;
    
    var set = false;
    // Round to two decimal places to avoid double-precision comparison problems
    var currentRange = Math.round(engine.getValue(group,"rateRange")*100)/100;
    var currentPitch = engine.getValue(group,"rate") * currentRange;
    var items = TerminalMix4.pitchRanges.length;
    for(i=0; i<items; i++) {
        if (currentRange<TerminalMix4.pitchRanges[i]) {
            engine.setValue(group,"rateRange",TerminalMix4.pitchRanges[i]);
            engine.setValue(group,"rate",currentPitch/TerminalMix4.pitchRanges[i]);
            set = true;
            break;
        }
    }

    if (!set && currentRange>=TerminalMix4.pitchRanges[items-1]) {
        engine.setValue(group,"rateRange",TerminalMix4.pitchRanges[0]);
        engine.setValue(group,"rate",currentPitch/TerminalMix4.pitchRanges[0]);
    }
}

TerminalMix4.crossfaderCurve = function (channel, control, value, status, group) {
    script.crossfaderCurve(value);
}

TerminalMix4.loopLengthKnob = function (channel, control, value, status, group) {
    if ((value-64)>0) {
        engine.setValue(group,"loop_double",1);
        engine.setValue(group,"loop_double",0);
    }
    else {
        engine.setValue(group,"loop_halve",1);
        engine.setValue(group,"loop_halve",0);
    }
}

TerminalMix4.loopMoveKnob = function (channel, control, value, status, group) {
    script.loopMove(group,value-64);
}

TerminalMix4.cfAssignL = function (channel, control, value, status, group) {
    engine.setValue(group,"orientation",0);
}

TerminalMix4.cfAssignM = function (channel, control, value, status, group) {
    engine.setValue(group,"orientation",1);
}

TerminalMix4.cfAssignR = function (channel, control, value, status, group) {
    engine.setValue(group,"orientation",2);
}

TerminalMix4.faderStart = function (channel, control, value, status, group) {
    if (value<=0) return;

    TerminalMix4.faderStart[group]=!TerminalMix4.faderStart[group];
}

TerminalMix4.faderStartFlash = function () {
    TerminalMix4.state["fStartFlash"]=!TerminalMix4.state["fStartFlash"];

    var value, group;
    for (var i=1; i<=4; i++) { // 4 decks
        value = 0x00;
        group = "[Channel"+i+"]";
        if (TerminalMix4.faderStart[group]) {
            if (TerminalMix4.state["fStartFlash"]) value = 0x7F;
        } else {
            if (engine.getValue(group,"duration")>0) value = 0x7F;
        }
        // Don't send redundant messages
        if (TerminalMix4.state[group+"fStart"]==value) continue;
        TerminalMix4.state[group+"fStart"] = value;
        if (engine.getValue(group,"duration")>0 || value<=0) midi.sendShortMsg(0x90+i-1,0x32,value);
        midi.sendShortMsg(0x90+i-1,0x78,value); // Shifted
    }
}

TerminalMix4.samplerPlayFlash = function () {
    TerminalMix4.state["sPlayFlash"]=!TerminalMix4.state["sPlayFlash"];

    var value, group;
    for (var i=1; i<=4; i++) { // 4 samplers
        value = 0x00;
        group = "[Sampler"+i+"]";
        if (engine.getValue(group,"play")>0) {
            if (TerminalMix4.state["sPlayFlash"]) value = 0x7F;
        } else {
            if (engine.getValue(group,"duration")>0) value = 0x7F;
        }
        // Don't send redundant messages
        if (TerminalMix4.state[group+"sFlash"]==value) continue;
        TerminalMix4.state[group+"sFlash"] = value;
        for (var j=1; j<=4; j++) {  // Same buttons on all 4 controller decks
            midi.sendShortMsg(0x90+j-1,0x14+i-1,value);
            midi.sendShortMsg(0x90+j-1,0x1C+i-1,value);  // Scissor on
            // Shifted
            midi.sendShortMsg(0x90+j-1,0x5A+i-1,value);
            midi.sendShortMsg(0x90+j-1,0x62+i-1,value);  // Scissor on
        }
    }
}

TerminalMix4.activeLoopFlash = function () {
    TerminalMix4.state["loopFlash"]=!TerminalMix4.state["loopFlash"];
    
    var value, group;
    for (var i=1; i<=4; i++) { // 4 decks
        value = 0x00;
        group = "[Channel"+i+"]";
        if (engine.getValue(group,"loop_enabled")>0) {
            if (TerminalMix4.state["loopFlash"]) value = 0x7F;
        }
        // Don't send redundant messages
        if (TerminalMix4.state[group+"loop"]==value) continue;
        TerminalMix4.state[group+"loop"] = value;
        midi.sendShortMsg(0x90+i-1,0x0C,value);
        midi.sendShortMsg(0x90+i-1,0x0D,value);
    }
}

TerminalMix4.channelFader = function (channel, control, value, status, group) {
    engine.setValue(group,"volume",script.absoluteLin(value,0,1));

    // Fader start logic
    if (!TerminalMix4.faderStart[group]) return;
    if (TerminalMix4.lastFader[group]==value) return;
    
    if (value==0 && engine.getValue(group,"play")==1) {
        engine.setValue(group,"cue_default",1);
        engine.setValue(group,"cue_default",0);
    }
    if (TerminalMix4.lastFader[group]==0) engine.setValue(group,"play",1);
    
    TerminalMix4.lastFader[group]=value;
}

TerminalMix4.crossFader = function (channel, control, value, status, group) {
    var cfValue = script.absoluteNonLin(value,-1,0,1);
    engine.setValue("[Master]","crossfader",cfValue);

    // Fader start logic
    if (TerminalMix4.lastFader["crossfader"]==cfValue) return;

    var group;

    // If CF is now full left and decks assigned to R are playing, cue them
    if (cfValue==-1.0) {
        for (var i=engine.getValue("[Master]","num_decks"); i>=1; i--) {
            group = "[Channel"+i+"]";
            if (TerminalMix4.faderStart[group]
                && engine.getValue(group,"orientation")==2
                && engine.getValue(group,"play")==1) {
                    engine.setValue(group,"cue_default",1);
                    engine.setValue(group,"cue_default",0);
            }
        }
    }

    if (cfValue==1.0) {
        // If CF is now full right and decks assigned to L are playing, cue them
        for (var i=engine.getValue("[Master]","num_decks"); i>=1; i--) {
            group = "[Channel"+i+"]";
            if (TerminalMix4.faderStart[group]
                && engine.getValue(group,"orientation")==0
                && engine.getValue(group,"play")==1) {
                    engine.setValue(group,"cue_default",1);
                    engine.setValue(group,"cue_default",0);
            }
        }
    }

    // If the CF is moved from full left, start any decks assigned to R
    if (TerminalMix4.lastFader["crossfader"]==-1.0) {
        for (var i=engine.getValue("[Master]","num_decks"); i>=1; i--) {
            group = "[Channel"+i+"]";
            if (TerminalMix4.faderStart[group]
                && engine.getValue(group,"orientation")==2) {
                engine.setValue(group,"play",1);
            }
        }
    }

    if (TerminalMix4.lastFader["crossfader"]==1.0) {
        // If the CF is moved from full right, start any decks assigned to L
        for (var i=engine.getValue("[Master]","num_decks"); i>=1; i--) {
            group = "[Channel"+i+"]";
            if (TerminalMix4.faderStart[group]
                && engine.getValue(group,"orientation")==0) {
                engine.setValue(group,"play",1);
            }
        }
    }

    TerminalMix4.lastFader["crossfader"] = cfValue;
}

TerminalMix4.filterReset = function (channel, control, value, status, group) {
    if (value>0) {
        var index = group+"high";
        TerminalMix4.lastEQs[index] = engine.getValue(group,"filterHigh");
        index = group+"mid";
        TerminalMix4.lastEQs[index] = engine.getValue(group,"filterMid");
        index = group+"low";
        TerminalMix4.lastEQs[index] = engine.getValue(group,"filterLow");
    }
    else {
        // Centered, so reset EQs
        var index = group+"high";
        engine.setValue(group,"filterHigh",TerminalMix4.lastEQs[index]);
        TerminalMix4.lastEQs[index] = undefined;
        index = group+"mid";
        engine.setValue(group,"filterMid",TerminalMix4.lastEQs[index]);
        TerminalMix4.lastEQs[index] = undefined;
        index = group+"low";
        engine.setValue(group,"filterLow",TerminalMix4.lastEQs[index]);
        TerminalMix4.lastEQs[index] = undefined;
    }
}

TerminalMix4.filterTurn = function (channel, control, value, status, group) {
    var index = group+"high";
    if (TerminalMix4.lastEQs[index] == undefined) return;
    index = group+"low";
    if (TerminalMix4.lastEQs[index] == undefined) return;
    index = group+"mid";
    if (TerminalMix4.lastEQs[index] == undefined) return;
    
    if (value < 0x40) {
        engine.setValue(group,"filterMid",script.absoluteLin(value,0,TerminalMix4.lastEQs[index],0x00,0x1F));
        index = group+"high";
        engine.setValue(group,"filterHigh",script.absoluteLin(value,0,TerminalMix4.lastEQs[index],0x20,0x3F));
    } else {
        engine.setValue(group,"filterMid",script.absoluteLin(value,TerminalMix4.lastEQs[index],0,0x60,0x7F));
        index = group+"low";
        engine.setValue(group,"filterLow",script.absoluteLin(value,TerminalMix4.lastEQs[index],0,0x40,0x5F));
    }
}

TerminalMix4.traxKnobTurn = function (channel, control, value, status, group) {
    var newValue = (value-64);
    if (TerminalMix4.traxKnobMode == "tracks") {
        engine.setValue(group,"SelectTrackKnob",newValue);
    } else {
        if (newValue>0) engine.setValue(group,"SelectNextPlaylist",newValue);
        if (newValue<0) engine.setValue(group,"SelectPrevPlaylist",newValue);
    }
}

TerminalMix4.traxKnobPress = function (channel, control, value, status, group) {
    if (value>0) {
        if (TerminalMix4.traxKnobMode == "tracks") {
            engine.setValue(group,"LoadSelectedIntoFirstStopped",1);
            engine.setValue(group,"LoadSelectedIntoFirstStopped",0);
        } else {
            TerminalMix4.traxKnobMode = "tracks";
            midi.sendShortMsg(0x90,0x37,0x7F);
        }
    }
}

TerminalMix4.backButton = function (channel, control, value, status, group) {
    if (value>0) {
        TerminalMix4.traxKnobMode = "sections";
        midi.sendShortMsg(0x90,0x37,0);
    }
}

// ----------- LED Output functions -------------

TerminalMix4.tapLED = function (deck,value) {
    deck--;
    if (value>0) midi.sendShortMsg(0x90+deck,0x0A,0x7F);
    else midi.sendShortMsg(0x90+deck,0x0A,0);
}

TerminalMix4.tapLEDL = function (value) {
    TerminalMix4.tapLED(1,value);
}

TerminalMix4.tapLEDR = function (value) {
    TerminalMix4.tapLED(2,value);
}

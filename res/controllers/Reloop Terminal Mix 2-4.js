/****************************************************************
*       Reloop Terminal Mix MIDI controller script v2.1         *
*           Copyright (C) 2012-2013, Sean M. Pappalardo         *
*                         2018, ronso0 (2.1 update)             *
*       but feel free to tweak this to your heart's content!    *
*       For Mixxx version 2.1.x                                 *
*                                                               *
*       Documentation in the Mixxx wiki:                        *
*       https://mixxx.org/wiki/doku.php/reloop_terminal_mix     *
****************************************************************/

function TerminalMix() {}

// ----------   Customization variables ----------
// Push the 'RANGE' button at the top edge of the pitch fader to
// cycle through the following pitch ranges. Edit the array to choose
// the ranges you need. For example '0.08' means +/-8%
TerminalMix.pitchRanges = [ 0.08, 0.12, 0.25, 0.5, 1.0 ];

// ----------   Other global variables    ----------
TerminalMix.timers = [];
TerminalMix.state = [];
TerminalMix.faderStart = [];
TerminalMix.lastFader = [];   // Last value of each channel/cross fader
TerminalMix.lastEQs = [[]];
TerminalMix.traxKnobMode = "tracks";
TerminalMix.shifted = false;
TerminalMix.shiftedL = false;
TerminalMix.shiftedR = false;
TerminalMix.loopMovePressedL = false;
TerminalMix.loopMovePressedR = false;

// ----------   Functions   ----------

TerminalMix.init = function (id,debug) {
    TerminalMix.id = id;

    // Extinguish all LEDs
    for (var i=0; i<=3; i++) {  // 4 decks
        for (var j=1; j<=120; j++) {
            midi.sendShortMsg(0x90+i,j,0x00);
        }
    }

    // New mapping of FX units using midi-components-0.0.js
    // EffectUnits 1 & 3. Usage:
    // new components.EffectUnit([int list EffUnit numbers], bool allowFocusWhenParametersHidden)
    TerminalMix.effectUnit13 = new components.EffectUnit([1,3]);
    TerminalMix.effectUnit13.enableButtons[1].midi = [0x90, 0x07];
    TerminalMix.effectUnit13.enableButtons[2].midi = [0x90, 0x08];
    TerminalMix.effectUnit13.enableButtons[3].midi = [0x90, 0x09];
    TerminalMix.effectUnit13.knobs[1].midi = [0xB0, 0x03];
    TerminalMix.effectUnit13.knobs[2].midi = [0xB0, 0x04];
    TerminalMix.effectUnit13.knobs[3].midi = [0xB0, 0x05];
    TerminalMix.effectUnit13.dryWetKnob.midi = [0xB0, 0x06];
    TerminalMix.effectUnit13.dryWetKnob.input = function (channel, control, value, status, group) {
        if (value === 63) {
          this.inSetParameter(this.inGetParameter() - .07);
        } else if (value === 65) {
          this.inSetParameter(this.inGetParameter() + .07);
        }
    };
    TerminalMix.effectUnit13.effectFocusButton.midi = [0x90, 0x0A];
    TerminalMix.effectUnit13.init();

    // EffectUnits 2 & 4
    TerminalMix.effectUnit24 = new components.EffectUnit([2,4]);
    TerminalMix.effectUnit24.enableButtons[1].midi = [0x91, 0x07];
    TerminalMix.effectUnit24.enableButtons[2].midi = [0x91, 0x08];
    TerminalMix.effectUnit24.enableButtons[3].midi = [0x91, 0x09];
    TerminalMix.effectUnit24.knobs[1].midi = [0xB1, 0x03];
    TerminalMix.effectUnit24.knobs[2].midi = [0xB1, 0x04];
    TerminalMix.effectUnit24.knobs[3].midi = [0xB1, 0x05];
    TerminalMix.effectUnit24.dryWetKnob.midi = [0xB1, 0x06];
    TerminalMix.effectUnit24.dryWetKnob.input = function (channel, control, value, status, group) {
        if (value === 63) {
          this.inSetParameter(this.inGetParameter() - .07);
        } else if (value === 65) {
          this.inSetParameter(this.inGetParameter() + .07);
        }
    };
    TerminalMix.effectUnit24.effectFocusButton.midi = [0x91, 0x0A];
    TerminalMix.effectUnit24.init();

    // Enable four decks in v1.11.x
    engine.setValue("[App]", "num_decks", 4);
    const samplerCount = 8;
    if (engine.getValue("[App]", "num_samplers") < samplerCount) {
        engine.setValue("[App]", "num_samplers", samplerCount);
    }

    // Set soft-takeover for all Sampler volumes
    for (let i=samplerCount; i>=1; i--) {
        engine.softTakeover("[Sampler"+i+"]","pregain",true);
    }
    // Set soft-takeover for all applicable Deck controls
    for (let j=engine.getValue("[App]", "num_decks"); j>=1; j--) {
        engine.softTakeover("[Channel"+j+"]", "volume", true);
        engine.softTakeover("[Channel"+j+"]", "filterHigh", true);
        engine.softTakeover("[Channel"+j+"]", "filterMid", true);
        engine.softTakeover("[Channel"+j+"]", "filterLow", true);
        engine.softTakeover("[Channel"+j+"]", "rate", true);
    }

    engine.softTakeover("[Master]","crossfader",true);

    engine.connectControl("[Channel1]","beat_active","TerminalMix.tapLEDL");
    engine.connectControl("[Channel2]","beat_active","TerminalMix.tapLEDR");

    TerminalMix.timers["fstartflash"] = -1;
//     TerminalMix.timers["qtrSec"] = engine.beginTimer(250, TerminalMix.qtrSec);
    TerminalMix.timers["halfSec"] = engine.beginTimer(500, TerminalMix.halfSec);

    if (TerminalMix.traxKnobMode == "tracks") {
        midi.sendShortMsg(0x90,0x37,0x7F);  // light Back button
    }

    print ("Reloop TerminalMix: "+id+" initialized.");
}

TerminalMix.shutdown = function () {
    // Stop all timers
    for (var i=0; i<TerminalMix.timers.length; i++) {
        engine.stopTimer(TerminalMix.timers[i]);
    }
    // Extinguish all LEDs
    for (var i=0; i<=3; i++) {  // 4 decks
        for (var j=1; j<=120; j++) {
            midi.sendShortMsg(0x90+i,j,0x00);
        }
    }
    print ("Reloop TerminalMix: "+TerminalMix.id+" shut down.");
}

TerminalMix.qtrSec = function () {

}

TerminalMix.halfSec = function () {
    TerminalMix.faderStartFlash();
    TerminalMix.samplerPlayFlash();
    TerminalMix.activeLoopFlash();
}

// The button that enables/disables scratching
TerminalMix.wheelTouch = function (channel, control, value, status, group) {
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
TerminalMix.wheelTurn = function (channel, control, value, status, group) {
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

TerminalMix.samplerVolume = function (channel, control, value) {
    // Link all sampler volume controls to the Sampler Volume knob
    for (var i=engine.getValue("[App]", "num_samplers"); i>=1; i--) {
        engine.setValue("[Sampler"+i+"]","pregain",
                        script.absoluteNonLin(value, 0.0, 1.0, 4.0));
    }
}

TerminalMix.pitchSlider = function (channel, control, value, status, group) {
    // invert pitch slider (down=faster) so it matches the labels on controller
    engine.setValue(group,"rate",-script.midiPitch(control, value, status));
}

TerminalMix.pitchRange = function (channel, control, value, status, group) {
    midi.sendShortMsg(status,control,value); // Make button light or extinguish
    if (value<=0) return;

    var set = false;
    // Round to two decimal places to avoid double-precision comparison problems
    var currentRange = Math.round(engine.getValue(group,"rateRange")*100)/100;
    var currentPitch = engine.getValue(group,"rate") * currentRange;
    var items = TerminalMix.pitchRanges.length;
    for(i=0; i<items; i++) {
        if (currentRange<TerminalMix.pitchRanges[i]) {
            engine.setValue(group,"rateRange",TerminalMix.pitchRanges[i]);
            engine.setValue(group,"rate",currentPitch/TerminalMix.pitchRanges[i]);
            set = true;
            break;
        }
    }

    if (!set && currentRange>=TerminalMix.pitchRanges[items-1]) {
        engine.setValue(group,"rateRange",TerminalMix.pitchRanges[0]);
        engine.setValue(group,"rate",currentPitch/TerminalMix.pitchRanges[0]);
    }
}

TerminalMix.crossfaderCurve = function (channel, control, value, status, group) {
    script.crossfaderCurve(value);
}

TerminalMix.loopLengthPress = function (channel, control, value, status, group) {
  if (value) {
    if (engine.getValue(group,"loop_enabled") === 0) {
      script.triggerControl(group,"beatloop_activate",100);
    } else {
      script.triggerControl(group,"reloop_toggle",100);
    }
  }
}

TerminalMix.shiftedLoopLengthPress = function (channel, control, value, status, group) {
  if (value) {
    script.triggerControl(group,"reloop_toggle",100);
  }
}

TerminalMix.loopLengthTurn = function (channel, control, value, status, group) {
    if (value === 65) {
        script.triggerControl(group,"loop_double",100);
    }
    else if (value === 63) {
        script.triggerControl(group,"loop_halve",100);
    }
}

TerminalMix.loopMovePress = function (channel, control, value, status, group) {
  /* Press the Move encoder to switch between two layers:
  a turn knob to jump X beats back or forth, or move any active loop by Y beats
  b Press & turn to adjust the beatjump/loopmove size
  This function stores the <pressed>/<unpressed> state because the knob sends
  the same signals independent from the pushbutton state, and we can't
  set 'dynamic variables' like ``bool loopMovePressed[channel number]``. */
  /* Left decks */
  if (channel === 1 || channel === 3) {
    if (value) {
      TerminalMix.loopMovePressedL = true;
    } else {
      TerminalMix.loopMovePressedL = false;
    }
  /* Right decks */
  } else {
    if (value) {
      TerminalMix.loopMovePressedR = true;
    } else {
      TerminalMix.loopMovePressedR = false;
    }
  }
}

TerminalMix.loopMoveTurn = function (channel, control, value, status, group) {
  if (channel === 1 || channel === 3) {
    /* If loopmove encoder is pressed while we turn it, we change beatjump_size */
    if (TerminalMix.loopMovePressedL) {
      TerminalMix.setBeatjumpSize(channel, control, value, status, group);
    /* Else we move the loop or make the play position jump */
    } else {
      TerminalMix.loopMove(channel, control, value, status, group);
    }
  /* Right decks */
  } else {
    if (TerminalMix.loopMovePressedR) {
      TerminalMix.setBeatjumpSize(channel, control, value, status, group);
    } else {
      TerminalMix.loopMove(channel, control, value, status, group);
    }
  }
}

TerminalMix.shiftedLoopMoveTurn = function (channel, control, value, status, group) {
  if (value === 65) {
    script.triggerControl(group,"beatjump_forward",100);
  }
  else if (value === 63) {
    script.triggerControl(group,"beatjump_backward",100);
  }
}



TerminalMix.loopMove = function (channel, control, value, status, group) {
  /* Loop enabled */
  if (engine.getValue(group,"loop_enabled") === 1) {
    if (engine.getValue(group,"quantize") === 1) {
    /* With 'quantize' enabled the loop_in marker might not snap to the beat
      we want the loop to start at, but to the next or previous beat.
      So we move the loop by one beat. */
      script.loopMove(group,value-64,1);
    } else {
    /* With 'quantize' OFF we might have missed the sweet spot, so we probably
      want to move the loop only by a fraction of a beat. Default = 1/8th beat */
      script.loopMove(group,value-64,0.125);
    }
  /* Loop disabled */
  } else {
    // jump by 'beatjump_size' beats
    if (value === 65) {
      script.triggerControl(group,"beatjump_forward",100);
    }
    else if (value === 63) {
      script.triggerControl(group,"beatjump_backward",100);
    }
  }
}

TerminalMix.setBeatjumpSize = function (channel, control, value, status, group) {
    if (value === 65) {
      engine.setValue(group,"beatjump_size",engine.getValue(group,"beatjump_size")*2);
    }
    else if (value === 63) {
      engine.setValue(group,"beatjump_size",engine.getValue(group,"beatjump_size")/2);
    }
}

TerminalMix.cfAssignL = function (channel, control, value, status, group) {
    engine.setValue(group,"orientation",0);
}

TerminalMix.cfAssignM = function (channel, control, value, status, group) {
    engine.setValue(group,"orientation",1);
}

TerminalMix.cfAssignR = function (channel, control, value, status, group) {
    engine.setValue(group,"orientation",2);
}

TerminalMix.faderStart = function (channel, control, value, status, group) {
    if (value<=0) return;

    TerminalMix.faderStart[group]=!TerminalMix.faderStart[group];
}

TerminalMix.brake = function (channel, control, value, status, group) {
    // var1 Start brake effect on button press, don't care about button release.
    // Can be stopped by shortly tapping wheel (when it's in touch mode).
    if (value) {
        script.brake(channel, control, value, status, group);
    }
    // var2 Start brake effect on button press, stop on release.
    /*if (value) {
        script.brake(channel, control, value, status, group);
    }	*/
}

TerminalMix.faderStartFlash = function () {
    TerminalMix.state["fStartFlash"]=!TerminalMix.state["fStartFlash"];

    var value, group;
    for (var i=1; i<=4; i++) { // 4 decks
        value = 0x00;
        group = "[Channel"+i+"]";
        if (TerminalMix.faderStart[group]) {
            if (TerminalMix.state["fStartFlash"]) value = 0x7F;
        } else {
            if (engine.getValue(group,"duration")>0) value = 0x7F;
        }
        // Don't send redundant messages
        if (TerminalMix.state[group+"fStart"]==value) continue;
        TerminalMix.state[group+"fStart"] = value;
        if (engine.getValue(group,"duration")>0 || value<=0) midi.sendShortMsg(0x90+i-1,0x32,value);
        midi.sendShortMsg(0x90+i-1,0x78,value); // Shifted
    }
}

TerminalMix.samplerPlayFlash = function () {
    TerminalMix.state["sPlayFlash"]=!TerminalMix.state["sPlayFlash"];

    var value, group;
    for (var i=1; i<=4; i++) { // 4 samplers
        value = 0x00;
        group = "[Sampler"+i+"]";
        if (engine.getValue(group,"play")>0) {
            if (TerminalMix.state["sPlayFlash"]) value = 0x7F;
        } else {
            if (engine.getValue(group,"duration")>0) value = 0x7F;
        }
        // Don't send redundant messages
        if (TerminalMix.state[group+"sFlash"]==value) continue;
        TerminalMix.state[group+"sFlash"] = value;
        for (var j=1; j<=4; j++) {  // Same buttons on all 4 controller decks
            midi.sendShortMsg(0x90+j-1,0x14+i-1,value);
            midi.sendShortMsg(0x90+j-1,0x1C+i-1,value);  // Scissor on
            // Shifted
            midi.sendShortMsg(0x90+j-1,0x5A+i-1,value);
            midi.sendShortMsg(0x90+j-1,0x62+i-1,value);  // Scissor on
        }
    }
}

TerminalMix.activeLoopFlash = function () {
    TerminalMix.state["loopFlash"]=!TerminalMix.state["loopFlash"];

    var value, group;
    for (var i=1; i<=4; i++) { // 4 decks
        value = 0x00;
        group = "[Channel"+i+"]";
        if (engine.getValue(group,"loop_enabled")>0) {
            if (TerminalMix.state["loopFlash"]) value = 0x7F;
        }
        // Don't send redundant messages
        if (TerminalMix.state[group+"loop"]==value) continue;
        TerminalMix.state[group+"loop"] = value;
        midi.sendShortMsg(0x90+i-1,0x0C,value);
        midi.sendShortMsg(0x90+i-1,0x0D,value);
    }
}

TerminalMix.channelFader = function (channel, control, value, status, group) {
    engine.setValue(group,"volume",script.absoluteLin(value,0,1));

    // Fader start logic
    if (!TerminalMix.faderStart[group]) return;
    if (TerminalMix.lastFader[group]==value) return;

    if (value==0 && engine.getValue(group,"play")==1) {
        script.triggerControl(group,"cue_default",100);
    }
    if (TerminalMix.lastFader[group]==0) engine.setValue(group,"play",1);

    TerminalMix.lastFader[group]=value;
}




TerminalMix.crossFader = function (channel, control, value, status, group) {
    var cfValue = script.absoluteNonLin(value,-1,0,1);
    engine.setValue("[Master]","crossfader",cfValue);

    // Fader start logic
    if (TerminalMix.lastFader["crossfader"]==cfValue) return;

    var group;

    // If CF is now full left and decks assigned to R are playing, cue them
    if (cfValue==-1.0) {
        for (var i=engine.getValue("[App]", "num_decks"); i>=1; i--) {
            group = "[Channel"+i+"]";
            if (TerminalMix.faderStart[group]
                && engine.getValue(group,"orientation")==2
                && engine.getValue(group,"play")==1) {
                    script.triggerControl(group,"cue_default",100);
            }
        }
    }

    if (cfValue==1.0) {
        // If CF is now full right and decks assigned to L are playing, cue them
        for (var i=engine.getValue("[App]", "num_decks"); i>=1; i--) {
            group = "[Channel"+i+"]";
            if (TerminalMix.faderStart[group]
                && engine.getValue(group,"orientation")==0
                && engine.getValue(group,"play")==1) {
                    script.triggerControl(group,"cue_default",100);
            }
        }
    }

    // If the CF is moved from full left, start any decks assigned to R
    if (TerminalMix.lastFader["crossfader"]==-1.0) {
        for (var i=engine.getValue("[App]", "num_decks"); i>=1; i--) {
            group = "[Channel"+i+"]";
            if (TerminalMix.faderStart[group]
                && engine.getValue(group,"orientation")==2) {
                engine.setValue(group,"play",1);
            }
        }
    }

    if (TerminalMix.lastFader["crossfader"]==1.0) {
        // If the CF is moved from full right, start any decks assigned to L
        for (var i=engine.getValue("[App]", "num_decks"); i>=1; i--) {
            group = "[Channel"+i+"]";
            if (TerminalMix.faderStart[group]
                && engine.getValue(group,"orientation")==0) {
                engine.setValue(group,"play",1);
            }
        }
    }

    TerminalMix.lastFader["crossfader"] = cfValue;
}

// Move cursor vertically with Trax knob, scroll with Shift pressed
TerminalMix.traxKnobTurn = function (channel, control, value, status, group) {
  if (TerminalMix.shifted) {
      engine.setValue(group,"ScrollVertical", value-64);
    } else {
      engine.setValue(group,"MoveVertical", value-64);
    }
}

// Move focus right between tracks table and side panel.
// Shift moves the focus to the left. Right now there are only two possible
// focus regions (panel + tracks table) so left/right have the same result,
// but the redesigned Library yet to come may have more regions.
TerminalMix.backButton = function (channel, control, value, status, group) {
    if (value>0) {
      if (TerminalMix.shifted) {
      engine.setValue(group,"MoveFocus",-1);
    } else {
      engine.setValue(group,"MoveFocus",1);
    }
  }
}

// Left shift button
TerminalMix.shiftButtonL = function (channel, control, value, status, group) {
  if (value === 127) {
    TerminalMix.effectUnit13.shift();
    TerminalMix.effectUnit24.shift();
    TerminalMix.shifted = true;
    TerminalMix.shiftedL = true;
  } else {
    TerminalMix.effectUnit13.unshift();
    TerminalMix.effectUnit24.unshift();
    TerminalMix.shifted = false;
    TerminalMix.shiftedL = false;
  }
};
// Right shift button
TerminalMix.shiftButtonR = function (channel, control, value, status, group) {
  if (value === 127) {
    TerminalMix.effectUnit13.shift();
    TerminalMix.effectUnit24.shift();
    TerminalMix.shifted = true;
    TerminalMix.shiftedR = true;
  } else {
    TerminalMix.effectUnit13.unshift();
    TerminalMix.effectUnit24.unshift();
    TerminalMix.shifted = false;
    TerminalMix.shiftedR = false;
  }
}

// ----------- LED Output functions -------------

TerminalMix.tapLED = function (deck,value) {
    deck--;
    if (value>0) midi.sendShortMsg(0x90+deck,0x0A,0x7F);
    else midi.sendShortMsg(0x90+deck,0x0A,0);
}

TerminalMix.tapLEDL = function (value) {
    TerminalMix.tapLED(1,value);
}

TerminalMix.tapLEDR = function (value) {
    TerminalMix.tapLED(2,value);
}

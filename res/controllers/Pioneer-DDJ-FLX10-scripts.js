// -------------------------------------------------------------------
// ------------------- DDJ-FLX10 script file v.1.0.1 -------------------
// -------------------------------------------------------------------

// *************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-FLX10.
// * Mostly adapted from the DDJ-1000 mapping script from Arnold Kalambani
// * Author: Marc Zischka (Zim)
// ****************************************************************************
//
//  Implemented (as per manufacturer's manual):
//      * Mixer Section (Faders, EQ, Filter, Gain, Cue)
//      * Browsing and loading 
//      * Jogwheels, Scratching, Bending, Loop adjust ?
//      * Cycle Tempo Range
//      * Beat Sync
//      * Hot Cue Mode
//      * Beat Loop Mode
//      * Beat Jump Mode
//      * Sampler Mode
//      * Toggle quantize
//      * Toggle slip
//		* Reverse play 
//++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check :
//  Custom (Mixxx specific mappings):
//      * BeatFX: Assigned Effect Unit 1
//                v FX_SELECT focus EFFECT1.
//                < LEFT focus EFFECT2
//                > RIGHT focus EFFECT3
//                ON/OFF toggles focused effect slot
//                SHIFT + ON/OFF disables all three effect slots.
//                SHIFT + < loads previous effect
//                SHIFT + > loads next effect
//
//      * 32 beat jump forward & back (Shift + </> CUE/LOOP CALL arrows)
//      * Toggle quantize (Shift + channel cue)
//      * Pad FX1 (see mapping infos)
//      * Pad FX2
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// To fix
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// To develop ?
//      * Loop Section:
//        * -4BEAT auto loop (hacky---prefer a clean way to set a 4 beat loop
//                            from a previous position on long press)
//
//        * CUE/LOOP CALL - memory & delete (complex and not useful. Hot cues are sufficient)
//
//      * Secondary pad modes (trial attempts complex and too experimental)
//        * Keyboard mode

//        * Keyshift mode
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Global variables
var PioneerDDJFLX10 = {};

// Global state
PioneerDDJFLX10.shiftActive = false;
PioneerDDJFLX10._rateMSB = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10._rateLSB = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10._jogTouches = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._jogLastValue = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10._reverseSlipActive = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._currentTimeMode = {1: 0, 2: 0, 3: 0, 4: 0}; // 0=Remaining, 1=Elapsed
PioneerDDJFLX10._lastShiftPress = 0;
PioneerDDJFLX10._playCueBlinkTimer = {};
PioneerDDJFLX10._playCueBlinkState = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._playCueShouldBlink = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._cuePressed = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._playShouldBlink = {1: false, 2: false, 3: false, 4: false};

// Utility function to extract deck number from group
PioneerDDJFLX10._getDeckFromGroup = function(group) {
    var match = group.match(/\d+/);
    return match ? parseInt(match[0], 10) : 1;
};

// Initialization
PioneerDDJFLX10.init = function(id) {
    print("Pioneer DDJ-FLX10 PROD - Initialisation");
    // Plage de pitch par défaut (±16%)
    for (var i = 1; i <= 4; i++) {
        engine.setValue("[Channel" + i + "]", "rateRange", 0.16);
        // Connect VU meters
        engine.connectControl("[Channel" + i + "]", "VuMeter", "PioneerDDJFLX10.LedVuMeterCH" + i);
        // Force off au démarrage pour éviter LEDs fantômes
        midi.sendShortMsg(0xB0 + (i - 1), 0x02, 0x00);
    }
    // LEDs Play/Cue avancées (Pioneer-like)
    PioneerDDJFLX10._initAdvancedLeds();

    return true;
};

// Tempo management (14-bit) — inversion Pioneer
PioneerDDJFLX10.rate_msb = function(channel, control, value, status, group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    PioneerDDJFLX10._rateMSB[deck] = value;
    PioneerDDJFLX10._updateRate(deck);
};

PioneerDDJFLX10.rate_lsb = function(channel, control, value, status, group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    PioneerDDJFLX10._rateLSB[deck] = value;
    PioneerDDJFLX10._updateRate(deck);
};

PioneerDDJFLX10._updateRate = function(deck) {
    var group = "[Channel" + deck + "]";
    var msb = PioneerDDJFLX10._rateMSB[deck] || 0;
    var lsb = PioneerDDJFLX10._rateLSB[deck] || 0;
    var value = (msb << 7) | lsb;                 // 0..16383
    var norm  = (value / 16383.0) * 2.0 - 1.0;    // -1..+1
    norm = -norm;                                 // inversion Pioneer
    if (norm > 1) norm = 1;
    if (norm < -1) norm = -1;
    engine.setValue(group, "rate", norm);
};

// Shutdown
PioneerDDJFLX10.shutdown = function() {
    print("Pioneer DDJ-FLX10 PROD - Arrêt");
    
    // Turn off all LEDs
    for (var i = 1; i <= 4; i++) {
        var group = "[Channel" + i + "]";
        midi.sendShortMsg(0x90 + (i-1), 0x00, 0x00); // Cue LED off
        midi.sendShortMsg(0x90 + (i-1), 0x0B, 0x00); // Play LED off
        // Add other LEDs to turn off if needed
    }
};

// Jog Wheel Management
PioneerDDJFLX10.wheelTurn = function(channel, control, value, status, group) {
    var newValue = value - 64;
    var deckNumber = PioneerDDJFLX10._getDeckFromGroup(group);
    
    print("wheelTurn: deck " + deckNumber + " value " + newValue + " isScratching " + engine.isScratching(deckNumber)); // Debug
    
    if (engine.isScratching(deckNumber)) {
        // Scratch mode
        var scratchValue = PioneerDDJFLX10.sensitivityMaximizer(newValue, 1.5);
        print("Scratching with value " + scratchValue); // Debug
        engine.scratchTick(deckNumber, scratchValue);
    } else {
        // Pitch bend mode - using working implementation from Untitled-1.js
        var bendValue = PioneerDDJFLX10.sensitivityMinimizer(newValue, 16);
        print("Pitch bend with value " + bendValue); // Debug
        engine.setValue(group, 'jog', bendValue);
    }
};

// VU-meters (LEDs faders)
PioneerDDJFLX10._sendVu = function(status, ctrl, value) {
    var v = Math.floor(Math.max(0, Math.min(1, value)) * 127);
    midi.sendShortMsg(status, ctrl, v);
};
PioneerDDJFLX10.LedVuMeterCH1 = function (value) { PioneerDDJFLX10._sendVu(0xB0, 0x02, value); };
PioneerDDJFLX10.LedVuMeterCH2 = function (value) { PioneerDDJFLX10._sendVu(0xB1, 0x02, value); };
PioneerDDJFLX10.LedVuMeterCH3 = function (value) { PioneerDDJFLX10._sendVu(0xB2, 0x02, value); };
PioneerDDJFLX10.LedVuMeterCH4 = function (value) { PioneerDDJFLX10._sendVu(0xB3, 0x02, value); };

PioneerDDJFLX10.wheelTouch = function(channel, control, value, status, group) {
    var deckNumber = PioneerDDJFLX10._getDeckFromGroup(group);
    
    if (value == 0x7F) {
        // Enable scratch mode
        var alpha = 1.0/8;
        var beta = alpha/32;
        print("Enabling scratch for deck " + deckNumber + " group " + group); // Debug
        engine.scratchEnable(deckNumber, 32767, 33+1/3, alpha, beta);
    } else {
        // Disable scratch mode
        print("Disabling scratch for deck " + deckNumber + " group " + group); // Debug
        engine.scratchDisable(deckNumber);
    }
};

// Sensitivity functions
PioneerDDJFLX10.sensitivityMinimizer = function (value, factor) {
    return (value/factor);
};

PioneerDDJFLX10.sensitivityMaximizer = function (value, factor) {
    return (value*factor);
};

// Shift buttons management
PioneerDDJFLX10.shiftHandler = function(channel, control, value, status, group) {
    PioneerDDJFLX10.shiftActive = (value === 0x7F);
    var now = new Date().getTime();
    
    // Detect double-click (within 300ms)
    PioneerDDJFLX10._lastShiftPress = now;
};

// Reverse with slip mode management
PioneerDDJFLX10.reverse = function(channel, control, value, status, group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    
    if (value === 0x7F) { // Button pressed
        if (PioneerDDJFLX10.shiftActive) {
            // Toggle current state
            var currentReverse = engine.getValue(group, "reverse") || 0;
            engine.setValue(group, "reverse", currentReverse ? 0 : 1);
            engine.setValue(group, "slip_enabled", 0); // Disable slip mode
        } else {
            // Enable slip mode and reverse
            engine.setValue(group, "slip_enabled", 1);
            engine.setValue(group, "reverse", 1);
            PioneerDDJFLX10._reverseSlipActive[deck] = true;
        }
    } else if (value === 0x00 && PioneerDDJFLX10._reverseSlipActive[deck]) {
        // Button release with slip mode active
        engine.setValue(group, "reverse", 0);
        engine.setValue(group, "slip_enabled", 0);
        PioneerDDJFLX10._reverseSlipActive[deck] = false;
    }
};

// Loop functions with shift support - RESTORED for PROD version
PioneerDDJFLX10.LoopHalveShift = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (PioneerDDJFLX10.shiftActive) {
            engine.setValue(group, "loop_halve", 1);
    } else {
            engine.setValue(group, "loop_in", 1);
        }
    }
};

PioneerDDJFLX10.LoopDoubleShift = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (PioneerDDJFLX10.shiftActive) {
            engine.setValue(group, "loop_double", 1);
        } else {
            engine.setValue(group, "loop_out", 1);
        }
    }
};

// Rate range selector — Pioneer 6/10/16/20 %
PioneerDDJFLX10._rateRanges = [0.06, 0.10, 0.16, 0.20];
PioneerDDJFLX10.RangeSelector = function(channel, control, value, status, group) {
    if (value !== 0x7F) return;
    var current = engine.getValue(group, "rateRange");
    var idx = 0, best = 1e9;
    for (var i = 0; i < PioneerDDJFLX10._rateRanges.length; i++) {
        var d = Math.abs(current - PioneerDDJFLX10._rateRanges[i]);
        if (d < best) { best = d; idx = i; }
    }
    idx = (idx + 1) % PioneerDDJFLX10._rateRanges.length;
    engine.setValue(group, "rateRange", PioneerDDJFLX10._rateRanges[idx]);
};

// Time display mode management
PioneerDDJFLX10.TimeTypeChange = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        var deck = PioneerDDJFLX10._getDeckFromGroup(group);
        PioneerDDJFLX10._currentTimeMode[deck] = (PioneerDDJFLX10._currentTimeMode[deck] + 1) % 2;
        PioneerDDJFLX10._updateTimeMode(group, deck);
    }
};

PioneerDDJFLX10._updateTimeMode = function(group, deck) {
    var timeMode = PioneerDDJFLX10._currentTimeMode[deck];
    engine.setValue(group, "show_seconds_elapsed", timeMode === 1);
};

// Tempo reset button management
PioneerDDJFLX10.rate_reset = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        engine.setValue(group, "rate_set_default", 1);
        
        // Reset internal values
        var deck = PioneerDDJFLX10._getDeckFromGroup(group);
        PioneerDDJFLX10._rateMSB[deck] = 0x40; // Center value (0x40 = 64)
        PioneerDDJFLX10._rateLSB[deck] = 0x00;
    }
};

// Cue button management
PioneerDDJFLX10.cueButton = function(channel, control, value, status, group) {
    var pressed = (value === 0x7F);
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);

    if (pressed) {
        if (PioneerDDJFLX10.shiftActive) {
            // SHIFT + Cue : efface le point Cue
            engine.setValue(group, "cue_clear", 1);
        } else {
            // Cue standard : lecture tant que maintenu
            engine.setValue(group, "cue_default", 1);
        }
        PioneerDDJFLX10._cuePressed[deck] = true;
    } else if (!PioneerDDJFLX10.shiftActive) {
        // Relâchement : stop/retour au point cue
        engine.setValue(group, "cue_default", 0);
        PioneerDDJFLX10._cuePressed[deck] = false;
    } else {
        PioneerDDJFLX10._cuePressed[deck] = false;
    }
    PioneerDDJFLX10._updatePlayCueLEDs(group);
};

// Pilotage Play/Cue LEDs façon Pioneer (blink en pause sur cue)
PioneerDDJFLX10._sendPlayCue = function(deck, note, on) {
    var status = 0x90 + (deck - 1);
    midi.sendShortMsg(status, note, on ? 0x7F : 0x00);
};

PioneerDDJFLX10._updatePlayCueLEDs = function(group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    var playing = engine.getValue(group, "play") > 0;
    var cueLit = engine.getValue(group, "cue_indicator") > 0;
    var cuePressed = PioneerDDJFLX10._cuePressed[deck];
    var hasTrack = engine.getValue(group, "track_samples") > 0;
    var atEnd = engine.getValue(group, "playposition") >= 0.999;

    // Modes :
    // - playing : Play ON, Cue OFF
    // - cuePressed : Cue ON fixe, Play OFF
    // - pause avec piste chargée (standby) : Play blink, Cue OFF
    // - fin de piste / pas de piste : tout OFF
    if (playing) {
        PioneerDDJFLX10._playCueShouldBlink[deck] = false;
        PioneerDDJFLX10._playShouldBlink[deck] = false;
        PioneerDDJFLX10._playCueBlinkState[deck] = false;
        PioneerDDJFLX10._sendPlayCue(deck, 0x0B, true);
        PioneerDDJFLX10._sendPlayCue(deck, 0x0C, false);
    } else if (cuePressed) {
        PioneerDDJFLX10._playCueShouldBlink[deck] = false;
        PioneerDDJFLX10._playShouldBlink[deck] = false;
        PioneerDDJFLX10._playCueBlinkState[deck] = false;
        PioneerDDJFLX10._sendPlayCue(deck, 0x0B, false);
        PioneerDDJFLX10._sendPlayCue(deck, 0x0C, true);
    } else if (hasTrack && !atEnd) {
        // Standby : Play blink, Cue off
        PioneerDDJFLX10._playCueShouldBlink[deck] = false;
        PioneerDDJFLX10._playShouldBlink[deck] = true;
    } else {
        PioneerDDJFLX10._playCueShouldBlink[deck] = false;
        PioneerDDJFLX10._playShouldBlink[deck] = false;
        PioneerDDJFLX10._playCueBlinkState[deck] = false;
        PioneerDDJFLX10._sendPlayCue(deck, 0x0B, false);
        PioneerDDJFLX10._sendPlayCue(deck, 0x0C, false);
    }
};

PioneerDDJFLX10._startPlayCueBlink = function(deck) {
    // Blink period ~ 500ms (2 Hz)
    PioneerDDJFLX10._playCueBlinkTimer[deck] = engine.beginTimer(500, function() {
        // Priorité au blink Play (standby). Si non, blink Cue (rarement utilisé ici).
        if (PioneerDDJFLX10._playShouldBlink[deck]) {
            PioneerDDJFLX10._playCueBlinkState[deck] = !PioneerDDJFLX10._playCueBlinkState[deck];
            PioneerDDJFLX10._sendPlayCue(deck, 0x0B, PioneerDDJFLX10._playCueBlinkState[deck]);
            PioneerDDJFLX10._sendPlayCue(deck, 0x0C, false);
            return;
        }
        if (PioneerDDJFLX10._playCueShouldBlink[deck]) {
            PioneerDDJFLX10._playCueBlinkState[deck] = !PioneerDDJFLX10._playCueBlinkState[deck];
            PioneerDDJFLX10._sendPlayCue(deck, 0x0B, false);
            PioneerDDJFLX10._sendPlayCue(deck, 0x0C, PioneerDDJFLX10._playCueBlinkState[deck]);
            return;
        }
        // Sinon: rien ne clignote
        PioneerDDJFLX10._playCueBlinkState[deck] = false;
        PioneerDDJFLX10._sendPlayCue(deck, 0x0B, false);
        PioneerDDJFLX10._sendPlayCue(deck, 0x0C, false);
    });
};

PioneerDDJFLX10._stopPlayCueBlink = function(deck) {
    if (PioneerDDJFLX10._playCueBlinkTimer[deck]) {
        engine.stopTimer(PioneerDDJFLX10._playCueBlinkTimer[deck]);
        PioneerDDJFLX10._playCueBlinkTimer[deck] = null;
    }
    PioneerDDJFLX10._playCueBlinkState[deck] = false;
    PioneerDDJFLX10._playCueShouldBlink[deck] = false;
};

// === Advanced Pioneer-like LED policy (Hotfix A) ===
PioneerDDJFLX10._adv = PioneerDDJFLX10._adv || {
  1:{play:0,cue:0,hold:false,phase:false},
  2:{play:0,cue:0,hold:false,phase:false},
  3:{play:0,cue:0,hold:false,phase:false},
  4:{play:0,cue:0,hold:false,phase:false},
};

PioneerDDJFLX10._noteStatus = PioneerDDJFLX10._noteStatus || (function(d){ return 0x90 + (d-1); });
PioneerDDJFLX10._sendLed = PioneerDDJFLX10._sendLed || function(d, note, on) {
    midi.sendShortMsg(PioneerDDJFLX10._noteStatus(d), note, on ? 0x7F : 0x00);
};

PioneerDDJFLX10._onCueHold = function(deck, pressed){
  PioneerDDJFLX10._adv[deck].hold = !!pressed;
  PioneerDDJFLX10._renderAdvanced(deck);
};

PioneerDDJFLX10._renderAdvanced = function(deck){
  var st = PioneerDDJFLX10._adv[deck];
  var playOn=0, cueOn=0;

  if (st.play) {
    playOn = 1;
    cueOn  = st.hold ? 1 : 0;
  } else {
    if (st.hold) {
      playOn = 0; cueOn = 1;
    } else if (st.cue) {
      playOn = st.phase ? 1 : 0;
      cueOn  = st.phase ? 0 : 1;
    } else {
      playOn = 0; cueOn = 0;
    }
  }

  PioneerDDJFLX10._sendLed(deck, 0x0B, playOn);
  PioneerDDJFLX10._sendLed(deck, 0x0C, cueOn);
};

PioneerDDJFLX10._connectAdvanced = function(deck){
  var g = "[Channel"+deck+"]";
  engine.connectControl(g, "play_indicator", function(v){
    PioneerDDJFLX10._adv[deck].play = (v>=0.5)?1:0;
    PioneerDDJFLX10._renderAdvanced(deck);
  });
  engine.connectControl(g, "cue_indicator", function(v){
    PioneerDDJFLX10._adv[deck].cue = (v>=0.5)?1:0;
    PioneerDDJFLX10._renderAdvanced(deck);
  });
};

if (!PioneerDDJFLX10._advTimer) {
  PioneerDDJFLX10._advTimer = engine.beginTimer(500, function(){
    [1,2,3,4].forEach(function(d){
      PioneerDDJFLX10._adv[d].phase = !PioneerDDJFLX10._adv[d].phase;
      PioneerDDJFLX10._renderAdvanced(d);
    });
  });
}

(function(){
  var _cue = PioneerDDJFLX10.cueButton;
  PioneerDDJFLX10.cueButton = function(ch, ctl, val, st, group){
    var m = /\[Channel(\d)\]/.exec(group||""); var deck = m ? parseInt(m[1],10) : 0;
    if (deck) PioneerDDJFLX10._onCueHold(deck, val>0);
    if (typeof _cue === "function") return _cue(ch, ctl, val, st, group);
  };
})();

PioneerDDJFLX10._initAdvancedLeds = PioneerDDJFLX10._initAdvancedLeds || function(){
  [1,2,3,4].forEach(PioneerDDJFLX10._connectAdvanced);
};

// Key Sync handler (protégé par SHIFT)
PioneerDDJFLX10.syncKeyHandler = function(channel, control, value, status, group) {
    if (value !== 0x7F) return;
    if (PioneerDDJFLX10.shiftActive) return; // ignore si SHIFT
    engine.setValue(group, "key_sync", 1);
};

// Function to expand playlists folder (currently disabled)
// This feature requires a different approach that will be implemented later

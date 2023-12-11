/****************************************************************
*       Reloop Beatmix script v1.0                              *
*       Author: Eugene Erokhin, uncle.eugene@gmail.com          *
****************************************************************/

var Beatmix = {};

// This constant defines the volume fader threshold at which fader start kicks in.
const faderStartThr = 0x20;

// Midi notes constants
const CH1 = 0x90;
const CH2 = 0x91;
const ON = 0x7F;
const OFF = 0x00;

const bNAVKNOB = 0x18;
const bBEATMASH1 = 0x21;
const bBEATMASH2 = 0x22;
const QUICKJOGR = 0x26;
const bPLUS = 0x27;
const QUICKJOGL = 0x28;
const bMINUS = 0x26;
const bSHIFTMINUS = 0x46;
const bSHIFTPLUS = 0x47;
const bSHIFTCUE = 0x4E;
const bKEYLOCK = 0x4F;
const bSHIFTLOAD = 0x52;

const LIBRARYMAIN = 3;
const LIBRARYTREE = 2;

// Internal variables
Beatmix.scratchEnabled = [false, false];
Beatmix.faderStartEnabled = [false, false];
Beatmix.faderLastValue = [0, 0];

// ----------   Functions   ----------

// Beatmix.init initializes the controller.
Beatmix.init = function(_id, _debug) {

    // Wash out lights
    for (let ch = 0; ch <= 1; ch++) {
        for (let ctl = 1; ctl <= 120; ctl++) {
            midi.sendShortMsg(CH1 + ch, ctl, OFF);
        }
    }

    // Setting up constant lights. Those are "+" and "-" buttons in both normal and shift modes
    midi.sendShortMsg(CH1, bMINUS, FULL);
    midi.sendShortMsg(CH1, bPLUS, FULL);
    midi.sendShortMsg(CH1, bSHIFTMINUS, FULL);
    midi.sendShortMsg(CH1, bSHIFTPLUS, FULL);
    midi.sendShortMsg(CH2, bMINUS, FULL);
    midi.sendShortMsg(CH2, bPLUS, FULL);
    midi.sendShortMsg(CH2, bSHIFTMINUS, FULL);
    midi.sendShortMsg(CH2, bSHIFTPLUS, FULL);

    // Turning off FX units 1 and 2
    engine.setValue("[EffectRack1_EffectUnit1]", "enabled", 0);
    engine.setValue("[EffectRack1_EffectUnit2]", "enabled", 0);

    // Getting some functions state from the host
    midi.sendShortMsg(CH1, bKEYLOCK, engine.getValue("[Channel1]", "keylock"));
    midi.sendShortMsg(CH2, bKEYLOCK, engine.getValue("[Channel2]", "keylock"));
    midi.sendShortMsg(CH1, bBEATMASH1, engine.getValue("[Channel1]", "quantize"));
    midi.sendShortMsg(CH2, bBEATMASH2, engine.getValue("[Channel2]", "quantize"));

    // Debug message
    console.log("Reloop Beatmix: controller initialized.");
};

// Beatmix.FaderStart handles fader start feature. The feature is enabled by Shift-Cue
// and its state is indicated by Shift-Cue light.
Beatmix.faderStart = function(channel, _control, value, _status, group) {
    const deck = script.deckFromGroup(group);
    if (value === FULL) {
        if (Beatmix.faderStartEnabled[deck]) {
            Beatmix.faderStartEnabled[deck] = false;
            midi.sendShortMsg(CH1 + channel, bSHIFTCUE, OFF);
        } else {
            Beatmix.faderStartEnabled[deck] = true;
            midi.sendShortMsg(CH1 + channel, bSHIFTCUE, FULL);
        }
    }
};

// Beatmix.Fader handles volume faders. If fader start is enabled it will start the
// corresponding deck once volume fader is above a certain position.
// No fader stop implemented.
// If you want to use fader as a hotcue button then what are you, a monster? :)
Beatmix.fader = function(_channel, _control, value, _status, group) {
    const deck = script.deckFromGroup(group);
    if (Beatmix.faderStartEnabled[deck] && value > faderStartThr && Beatmix.faderLastValue[deck] < faderStartThr) {
        engine.setValue(group, "play", 1);
    }
    engine.setParameter(group, "volume", value/127);
    Beatmix.faderLastValue[deck] = value;
};

// Beatmix.ToggleScratchMode toggles scratch mode on/off. Unfortunately
// the scratch button LED on Reloop Beatmix is controlled by hardware so
// there's no way to set this LED in accordance to current scratch mode.
// So i deceided to leave the dedicated button alone. It does nothing at all.
// Instead scratch mode is enabled and indicated by Shift-Load.
Beatmix.toggleScratchMode = function(channel, _control, value, _status, group) {
    const deck = script.deckFromGroup(group);
    if (value === FULL) {
        Beatmix.scratchEnabled[deck] = !Beatmix.scratchEnabled[deck];
    }
    if (Beatmix.scratchEnabled[deck]) {
        midi.sendShortMsg(CH1+channel, bSHIFTLOAD, FULL);
    } else {
        midi.sendShortMsg(CH1+channel, bSHIFTLOAD, OFF);
    }
};

// Beatmix.wheelTouch handles the touch of jog top plate. When scratch mode is selected
// touching the plate will switch deck into scratch mode.
Beatmix.wheelTouch = function(_channel, _control, value, _status, group) {
    const deck = script.deckFromGroup(group);
    if (Beatmix.scratchEnabled[deck]) {
        if (value === FULL) {
            const alpha = 1.0 / 8;
            const beta = alpha / 32;
            engine.scratchEnable(deck, 800, 33 + 1/3, alpha, beta);
        } else {
            engine.scratchDisable(deck);
        }
    }
};

// Beatmix.wheelTurn process jogwheel movement depending on selected jog mode.
// There are two modes. One (default) is more gentle and another (kicked in with
// the search button just next to the jog) is four time faster.
Beatmix.wheelTurn = function(_channel, control, value, _status, group) {
    const deck = script.deckFromGroup(group);
    const newValue = value - 64;
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, newValue);
        return;
    }
    if (control === QUICKJOGR || control === QUICKJOGL) {
        engine.setValue(group, "jog", newValue/2);
    } else {
        engine.setValue(group, "jog", newValue/16);
    }
};

// Beatmix.navKnob handles navigation dial. It will scroll through library.
Beatmix.navKnob = function(_channel, control, value, _status, _group) {
    //console.log("control: " + control + ", value: " + value + ", status: " + status);
    const newValue = value - 64;
    if (control === bNAVKNOB) {
        engine.setValue("[Library]", "focused_widget", LIBRARYMAIN);
    } else {
        engine.setValue("[Library]", "focused_widget", LIBRARYTREE);
    }
    engine.setValue("[Library]", "MoveVertical", newValue);
};

// Beatmix.expandCollapse handles Shift-Knob-Click event. It will expand/collapse
// currently selected library tree entry.
Beatmix.expandCollapse = function(_channel, _control, value, _status, _group) {
    if (value === FULL && engine.getValue("[Library]", "focused_widget") === LIBRARYTREE) {
        engine.setValue("[Library]", "GoToItem", 1);
    } else {
        engine.setValue("[Library]", "focused_widget", LIBRARYTREE);
    }
};

// Beatmix.loopSize handles Loop Size knob. This know ajusts auto loop size.
Beatmix.loopSize = function(_channel, _control, value, _status, group) {
    const newValue = value - 64;
    if (newValue < 0) {
        engine.setValue(group, "loop_halve", 1);
    } else {
        engine.setValue(group, "loop_double", 1);
    }
};

// Beatmix.loopMove handles Loop Move knob (Shift Loop Size). This knob adjusts auto loop position.
Beatmix.loopMove = function(_channel, _control, value, _status, group) {
    const newValue = value - 64;
    engine.setValue(group, "loop_move", newValue);

};

// Beatmix.fxSelClick toggles FX rack visibility.
Beatmix.fxSelClick = function(_channel, _control, value, _status, _group) {
    if (value === FULL) {
        script.toggleControl("[EffectRack1]", "show");
    }
};

// Beatmix.smplSelClick toggles samplers visibility.
Beatmix.smplSelClick = function(_channel, _control, value, _status, _group) {
    if (value === FULL) {
        script.toggleControl("[Samplers]", "show_samplers");
    }
};

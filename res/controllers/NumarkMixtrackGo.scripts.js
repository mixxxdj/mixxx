// Some variables for ease of access.
const autoLoadedEchoMetaValue = 0.5;
const autoLoadedEchoMixValue = 0.5;
const autoLoadedFlangerMetaValue = 1.0;
const autoLoadedFlangerMixValue = 0.75;
const autoLoadedReverbMetaValue = 0.7;
const autoLoadedReverbMixValue = 0.5;

const beatloopPad1Size = 1;
const beatloopPad2Size = 2;
const beatloopPad3Size = 4;
const beatloopPad4Size = 8;

const beatloopActivePadBlinking = true;
const beatloopPadBlinkingFast = false;
const samplerActiveBlinkingFast = false;

const beatloopRollSize = 1;

const shiftCueDoubleTapTimer = 300;
// modifies the fraction of the duration of the track the encoder moves per detent while rotating
// default: 0.02  (2%)
const previewDeckStripSearchPace = 0.02;


const NumarkMixtrackGo = {};

/**
 * Holds all the used midi signals sent by the controller.
 */
NumarkMixtrackGo.controls = {

    // [status, ctrl] are the pair of codes that identify the midi signals sent from the controller
    // [[status, ctrl], [status, ctrl]] holds the left control on the first position and the right
    // control on the second position.

    // [Master]
    "main_level": [0xBF, 0x0A],
    "cue_level": [0xBF, 0x0C],
    "crossfader": [0xBF, 0x08],
    "fadefx": [0x9F, 0x46],
    "fadefx_shift": [0x9F, 0x47], // currently free

    // [Library]
    "browse_encoder_rotate": [0xBF, 0x00],
    "browse_encoder_rotate_shift": [0xBF, 0x01], // currently free
    "browse_encoder_push_on": [0x9F, 0x07],
    "load": [[0x9F, 0x02], [0x9F, 0x03]],
    "load_shift": [[0x9F, 0x48], [0x9F, 0x49]],

    // Mixer area
    "level": [[0xB0, 0x16], [0xB1, 0x16]],
    "filter_low": [[0xB0, 0x1A], [0xB1, 0x1A]],
    "pfl": [[0x90, 0x1B], [0x91, 0x1B]],
    "pfl_shift": [[0x90, 0x54], [0x91, 0x54]], // currently free

    // Decks
    "rate_lsb": [[0xB0, 0x29], [0xB1, 0x29]],
    "rate_msb": [[0xB0, 0x09], [0xB1, 0x09]],
    "mode": [[0x94, 0x00], [0x91, 0x50]],
    "mode_long_press": [[0x90, 0x20], [0x91, 0x20]], // not used atm

    "jog_touch_press": [[0x90, 0x50], [0x91, 0x50]],
    "jog_touch_release": [[0x80, 0x50], [0x81, 0x50]],
    "jog_turn": [[0xB0, 0x06], [0xB1, 0x06]],
    "jog_turn_shift": [[0xB0, 0x2B], [0xB1, 0x2B]], // not used atm

    "acapel": [[0x90, 0x46], [0x91, 0x46]],
    "acapel_shift": [[0x90, 0x47], [0x91, 0x47]],
    "instru": [[0x90, 0x48], [0x91, 0x48]],
    "instru_shift": [[0x90, 0x49], [0x91, 0x49]],
    "sync": [[0x90, 0x02], [0x91, 0x02]],
    "sync_shift": [[0x90, 0x03], [0x91, 0x03]],
    "cue": [[0x90, 0x01], [0x91, 0x01]],
    "cue_shift": [[0x90, 0x05], [0x91, 0x05]],
    "play": [[0x90, 0x00], [0x091, 0x00]],
    "play_shift": [[0x90, 0x04], [0x91, 0x04]],
    "tempo_fader": [[0xB0, 0x00], [0xB1, 0x00]],

    // Pads
    "pad1": [[0x94, 0x14], [0x95, 0x14]],
    "pad1_shift": [[0x94, 0x1C], [0x95, 0x1C]],

    "pad2": [[0x94, 0x15], [0x95, 0x15]],
    "pad2_shift": [[0x94, 0x1D], [0x95, 0x1D]],

    "pad3": [[0x94, 0x16], [0x95, 0x16]],
    "pad3_shift": [[0x94, 0x1E], [0x95, 0x1E]],

    "pad4": [[0x94, 0x17], [0x95, 0x17]],
    "pad4_shift": [[0x94, 0x1F], [0x95, 0x1F]],
};

/**
 * Holds all the led midi information and functions to operate leds.
 */
NumarkMixtrackGo.led = {

    // [status, ctrl] are the pair of codes that identify the midi signals that need to be sent
    // to the controller to set a led
    // [[status, ctrl], [status, ctrl]] holds the left side's pair of codes on the first position and
    // the right side's pair of codes on the second position.

    //unused - there are others but didn't take notes
    //[0xB0, 0x1F, 0x00] - value from 0 to 5 lights the mode lights and play cumulatively

    // values
    "off": 0x00,
    "dim": 0x01,
    "bright": 0x7F,


    "all": [0x9E, 0x7F],

    "mode_hotcue": [[0x94, 0x01], [0x95, 0x01]],
    "mode_loops": [[0x94, 0x02], [0x95, 0x02]],
    "mode_sampler": [[0x94, 0x03], [0x95, 0x03]],
    "mode_stems": [[0x94, 0x04], [0x95, 0x04]],

    "play": [[0x90, 0x00], [0x91, 0x00]],
    "play_shift": [[0x90, 0x04], [0x91, 0x04]],
    "cue": [[0x90, 0x01], [0x91, 0x01]],
    "cue_shift": [[0x90, 0x05], [0x91, 0x05]],
    "sync": [[0x90, 0x02], [0x91, 0x02]],
    "sync_shift": [[0x90, 0x03], [0x91, 0x03]],
    "acapel": [[0x90, 0x46], [0x91, 0x46]],
    "instru": [[0x90, 0x48], [0x91, 0x48]],
    "fadefx": [0x9F, 0x46],
    "fadefx_shift": [0x9F, 0x47],
    "load": [[0x9F, 0x02], [0x9F, 0x03]],
    "load_shift": [[0x9F, 0x48], [0x9F, 0x49]],
    "pfl": [[0x80, 0x1B], [0x81, 0x1B]],
    "pfl_shift": [[0x90, 0x54], [0x91, 0x54]],

    "pad1": [[0x94, 0x14], [0x95, 0x14]],
    "pad1_shift": [[0x94, 0x1C], [0x95, 0x1C]],
    "pad2": [[0x94, 0x15], [0x95, 0x15]],
    "pad2_shift": [[0x94, 0x1D], [0x95, 0x1D]],
    "pad3": [[0x94, 0x16], [0x95, 0x16]],
    "pad3_shift": [[0x94, 0x1E], [0x95, 0x1E]],
    "pad4": [[0x94, 0x17], [0x95, 0x17]],
    "pad4_shift": [[0x94, 0x1F], [0x95, 0x1F]],

    // maintaining some led states so the controller doesn't get spammed with useless messages
    fadefxState: this.off,
    fadefxShiftState: this.off,
    pad1State: [this.off, this.off],
    pad2State: [this.off, this.off],
    pad3State: [this.off, this.off],
    pad4State: [this.off, this.off],

    // below are the functions that switch the leds.
    setAllOff: function() {
        midi.sendShortMsg(this.all[0], this.all[1], this.off);
    },
    setAllBright: function() {
        midi.sendShortMsg(this.all[0], this.all[1], this.bright);
    },

    setModeHotcueOff: function(deckIndex) {
        midi.sendShortMsg(this.mode_hotcue[deckIndex][0], this.mode_hotcue[deckIndex][1], this.off);
    },
    setModeHotcueDim: function(deckIndex) {
        midi.sendShortMsg(this.mode_hotcue[deckIndex][0], this.mode_hotcue[deckIndex][1], this.dim);
    },
    setModeHotcueBright: function(deckIndex) {
        midi.sendShortMsg(this.mode_hotcue[deckIndex][0], this.mode_hotcue[deckIndex][1], this.bright);
    },

    setModeLoopsOff: function(deckIndex) {
        midi.sendShortMsg(this.mode_loops[deckIndex][0], this.mode_loops[deckIndex][1], this.off);
    },
    setModeLoopsDim: function(deckIndex) {
        midi.sendShortMsg(this.mode_loops[deckIndex][0], this.mode_loops[deckIndex][1], this.dim);
    },
    setModeLoopsBright: function(deckIndex) {
        midi.sendShortMsg(this.mode_loops[deckIndex][0], this.mode_loops[deckIndex][1], this.bright);
    },

    setModeSamplerOff: function(deckIndex) {
        midi.sendShortMsg(this.mode_sampler[deckIndex][0], this.mode_sampler[deckIndex][1], this.off);
    },
    setModeSamplerDim: function(deckIndex) {
        midi.sendShortMsg(this.mode_sampler[deckIndex][0], this.mode_sampler[deckIndex][1], this.dim);
    },
    setModeSamplerBright: function(deckIndex) {
        midi.sendShortMsg(this.mode_sampler[deckIndex][0], this.mode_sampler[deckIndex][1], this.bright);
    },

    setModeStemsOff: function(deckIndex) {
        midi.sendShortMsg(this.mode_stems[deckIndex][0], this.mode_stems[deckIndex][1], this.off);
    },
    setModeStemsDim: function(deckIndex) {
        midi.sendShortMsg(this.mode_stems[deckIndex][0], this.mode_stems[deckIndex][1], this.dim);
    },
    setModeStemsBright: function(deckIndex) {
        midi.sendShortMsg(this.mode_stems[deckIndex][0], this.mode_stems[deckIndex][1], this.bright);
    },


    setPlayOff: function(deckIndex) {
        midi.sendShortMsg(this.play[deckIndex][0], this.play[deckIndex][1], this.off);
    },
    setPlayDim: function(deckIndex) {
        midi.sendShortMsg(this.play[deckIndex][0], this.play[deckIndex][1], this.dim);
    },
    setPlayBright: function(deckIndex) {
        midi.sendShortMsg(this.play[deckIndex][0], this.play[deckIndex][1], this.bright);
    },

    setPlayShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.play_shift[deckIndex][0], this.play_shift[deckIndex][1], this.off);
    },
    setPlayShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.play_shift[deckIndex][0], this.play_shift[deckIndex][1], this.dim);
    },
    setPlayShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.play_shift[deckIndex][0], this.play_shift[deckIndex][1], this.bright);
    },


    setCueOff: function(deckIndex) {
        midi.sendShortMsg(this.cue[deckIndex][0], this.cue[deckIndex][1], this.off);
    },
    setCueDim: function(deckIndex) {
        midi.sendShortMsg(this.cue[deckIndex][0], this.cue[deckIndex][1], this.dim);
    },
    setCueBright: function(deckIndex) {
        midi.sendShortMsg(this.cue[deckIndex][0], this.cue[deckIndex][1], this.bright);
    },

    setCueShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.cue_shift[deckIndex][0], this.cue_shift[deckIndex][1], this.off);
    },
    setCueShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.cue_shift[deckIndex][0], this.cue_shift[deckIndex][1], this.dim);
    },
    setCueShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.cue_shift[deckIndex][0], this.cue_shift[deckIndex][1], this.bright);
    },


    setSyncOff: function(deckIndex) {
        midi.sendShortMsg(this.sync[deckIndex][0], this.sync[deckIndex][1], this.off);
    },
    setSyncDim: function(deckIndex) {
        midi.sendShortMsg(this.sync[deckIndex][0], this.sync[deckIndex][1], this.dim);
    },
    setSyncBright: function(deckIndex) {
        midi.sendShortMsg(this.sync[deckIndex][0], this.sync[deckIndex][1], this.bright);
    },

    setSyncShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.sync_shift[deckIndex][0], this.sync_shift[deckIndex][1], this.off);
    },
    setSyncShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.sync_shift[deckIndex][0], this.sync_shift[deckIndex][1], this.dim);
    },
    setSyncShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.sync_shift[deckIndex][0], this.sync_shift[deckIndex][1], this.bright);
    },


    setAcapelOff: function(deckIndex) {
        midi.sendShortMsg(this.acapel[deckIndex][0], this.acapel[deckIndex][1], this.off);
    },
    setAcapelDim: function(deckIndex) {
        midi.sendShortMsg(this.acapel[deckIndex][0], this.acapel[deckIndex][1], this.dim);
    },
    setAcapelBright: function(deckIndex) {
        midi.sendShortMsg(this.acapel[deckIndex][0], this.acapel[deckIndex][1], this.bright);
    },

    setInstruOff: function(deckIndex) {
        midi.sendShortMsg(this.instru[deckIndex][0], this.instru[deckIndex][1], this.off);
    },
    setInstruDim: function(deckIndex) {
        midi.sendShortMsg(this.instru[deckIndex][0], this.instru[deckIndex][1], this.dim);
    },
    setInstruBright: function(deckIndex) {
        midi.sendShortMsg(this.instru[deckIndex][0], this.instru[deckIndex][1], this.bright);
    },


    setFadefxOff: function() {
        if (this.fadefxState !== this.off) {
            midi.sendShortMsg(this.fadefx[0], this.fadefx[1], this.off);
            this.fadefxState = this.off;
        }

    },
    setFadefxDim: function() {
        if (this.fadefxState !== this.dim) {
            midi.sendShortMsg(this.fadefx[0], this.fadefx[1], this.dim);
            this.fadefxState = this.dim;
        }
    },
    setFadefxBright: function() {
        if (this.fadefxState !== this.bright) {
            midi.sendShortMsg(this.fadefx[0], this.fadefx[1], this.bright);
            this.fadefxState = this.bright;
        }
    },

    setFadefxShiftOff: function() {
        if (this.fadefxShiftState !== this.off) {
            midi.sendShortMsg(this.fadefx_shift[0], this.fadefx_shift[1], this.off);
            this.fadefxShiftState = this.off;
        }

    },
    setFadefxShiftDim: function() {
        if (this.fadefxShiftState !== this.dim) {
            midi.sendShortMsg(this.fadefx_shift[0], this.fadefx_shift[1], this.dim);
            this.fadefxShiftState = this.dim;
        }

    },
    setFadefxShiftBright: function() {
        if (this.fadefxShiftState !== this.bright) {
            midi.sendShortMsg(this.fadefx_shift[0], this.fadefx_shift[1], this.bright);
            this.fadefxShiftState = this.bright;
        }
    },


    setLoadOff: function(deckIndex) {
        midi.sendShortMsg(this.load[deckIndex][0], this.load[deckIndex][1], this.off);
    },
    setLoadDim: function(deckIndex) {
        midi.sendShortMsg(this.load[deckIndex][0], this.load[deckIndex][1], this.dim);
    },
    setLoadBright: function(deckIndex) {
        midi.sendShortMsg(this.load[deckIndex][0], this.load[deckIndex][1], this.bright);
    },

    setLoadShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.load_shift[deckIndex][0], this.load_shift[deckIndex][1], this.off);
    },
    setLoadShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.load_shift[deckIndex][0], this.load_shift[deckIndex][1], this.dim);
    },
    setLoadShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.load_shift[deckIndex][0], this.load_shift[deckIndex][1], this.bright);
    },


    setPflOff: function(deckIndex) {
        midi.sendShortMsg(this.pfl[deckIndex][0], this.pfl[deckIndex][1], this.off);
    },
    setPflDim: function(deckIndex) {
        midi.sendShortMsg(this.pfl[deckIndex][0], this.pfl[deckIndex][1], this.dim);
    },
    setPflBright: function(deckIndex) {
        midi.sendShortMsg(this.pfl[deckIndex][0], this.pfl[deckIndex][1], this.bright);
    },

    setPflShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.pfl_shift[deckIndex][0], this.pfl_shift[deckIndex][1], this.off);
    },
    setPflShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.pfl_shift[deckIndex][0], this.pfl_shift[deckIndex][1], this.dim);
    },
    setPflShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.pfl_shift[deckIndex][0], this.pfl_shift[deckIndex][1], this.bright);
    },


    setAllPadsOff: function(deckIndex) {
        for (let i = 1; i <= 4; i++) {
            this.setPadOff(i, deckIndex);
        }
    },
    setAllPadsDim: function(deckIndex) {
        for (let i = 1; i <= 4; i++) {
            this.setPadDim(i, deckIndex);
        }
    },
    setAllPadsShiftOff: function(deckIndex) {
        for (let i = 1; i <= 4; i++) {
            this.setPadShiftOff(i, deckIndex);
        }
    },

    setPadOff: function(padNumber, deckIndex) {
        switch (padNumber) {
        case 1:
            this.setPad1Off(deckIndex);
            break;
        case 2:
            this.setPad2Off(deckIndex);
            break;
        case 3:
            this.setPad3Off(deckIndex);
            break;
        case 4:
            this.setPad4Off(deckIndex);
            break;
        }
    },

    setPadDim: function(padNumber, deckIndex) {
        switch (padNumber) {
        case 1:
            this.setPad1Dim(deckIndex);
            break;
        case 2:
            this.setPad2Dim(deckIndex);
            break;
        case 3:
            this.setPad3Dim(deckIndex);
            break;
        case 4:
            this.setPad4Dim(deckIndex);
            break;
        }
    },
    setPadBright: function(padNumber, deckIndex) {
        switch (padNumber) {
        case 1:
            this.setPad1Bright(deckIndex);
            break;
        case 2:
            this.setPad2Bright(deckIndex);
            break;
        case 3:
            this.setPad3Bright(deckIndex);
            break;
        case 4:
            this.setPad4Bright(deckIndex);
            break;
        }
    },

    setPadShiftOff: function(padNumber, deckIndex) {
        switch (padNumber) {
        case 1:
            this.setPad1ShiftOff(deckIndex);
            break;
        case 2:
            this.setPad2ShiftOff(deckIndex);
            break;
        case 3:
            this.setPad3ShiftOff(deckIndex);
            break;
        case 4:
            this.setPad4ShiftOff(deckIndex);
            break;
        }
    },


    setPadShiftDim: function(padNumber, deckIndex) {
        switch (padNumber) {
        case 1:
            this.setPad1ShiftDim(deckIndex);
            break;
        case 2:
            this.setPad2ShiftDim(deckIndex);
            break;
        case 3:
            this.setPad3ShiftDim(deckIndex);
            break;
        case 4:
            this.setPad4ShiftDim(deckIndex);
            break;
        }
    },
    setPadShiftBright: function(padNumber, deckIndex) {
        switch (padNumber) {
        case 1:
            this.setPad1ShiftBright(deckIndex);
            break;
        case 2:
            this.setPad2ShiftBright(deckIndex);
            break;
        case 3:
            this.setPad3ShiftBright(deckIndex);
            break;
        case 4:
            this.setPad4ShiftBright(deckIndex);
            break;
        }
    },


    setPad1Off: function(deckIndex) {
        if (this.pad1State[deckIndex] !== this.off) {
            midi.sendShortMsg(this.pad1[deckIndex][0], this.pad1[deckIndex][1], this.off);
            this.pad1State[deckIndex] = this.off;
        }
    },
    setPad1Dim: function(deckIndex) {
        if (this.pad1State[deckIndex] !== this.dim) {
            midi.sendShortMsg(this.pad1[deckIndex][0], this.pad1[deckIndex][1], this.dim);
            this.pad1State[deckIndex] = this.dim;
        }
    },

    setPad1Bright: function(deckIndex) {
        if (this.pad1State[deckIndex] !== this.bright) {
            midi.sendShortMsg(this.pad1[deckIndex][0], this.pad1[deckIndex][1], this.bright);
            this.pad1State[deckIndex] = this.bright;
        }
    },

    setPad1ShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.pad1_shift[deckIndex][0], this.pad1_shift[deckIndex][1], this.off);
    },
    setPad1ShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.pad1_shift[deckIndex][0], this.pad1_shift[deckIndex][1], this.dim);
    },
    setPad1ShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.pad1_shift[deckIndex][0], this.pad1_shift[deckIndex][1], this.bright);
    },


    setPad2Off: function(deckIndex) {
        if (this.pad2State[deckIndex] !== this.off) {
            midi.sendShortMsg(this.pad2[deckIndex][0], this.pad2[deckIndex][1], this.off);
            this.pad2State[deckIndex] = this.off;
        }
    },
    setPad2Dim: function(deckIndex) {
        if (this.pad2State[deckIndex] !== this.dim) {
            midi.sendShortMsg(this.pad2[deckIndex][0], this.pad2[deckIndex][1], this.dim);
            this.pad2State[deckIndex] = this.dim;
        }
    },
    setPad2Bright: function(deckIndex) {
        if (this.pad2State[deckIndex] !== this.bright) {
            midi.sendShortMsg(this.pad2[deckIndex][0], this.pad2[deckIndex][1], this.bright);
            this.pad2State[deckIndex] = this.bright;
        }

    },

    setPad2ShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.pad2_shift[deckIndex][0], this.pad2_shift[deckIndex][1], this.off);
    },
    setPad2ShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.pad2_shift[deckIndex][0], this.pad2_shift[deckIndex][1], this.dim);
    },
    setPad2ShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.pad2_shift[deckIndex][0], this.pad2_shift[deckIndex][1], this.bright);
    },

    setPad3Off: function(deckIndex) {
        if (this.pad3State[deckIndex] !== this.off) {
            midi.sendShortMsg(this.pad3[deckIndex][0], this.pad3[deckIndex][1], this.off);
            this.pad3State[deckIndex] = this.off;
        }
    },
    setPad3Dim: function(deckIndex) {
        if (this.pad3State[deckIndex] !== this.dim) {
            midi.sendShortMsg(this.pad3[deckIndex][0], this.pad3[deckIndex][1], this.dim);
            this.pad3State[deckIndex] = this.dim;
        }
    },
    setPad3Bright: function(deckIndex) {
        if (this.pad3State[deckIndex] !== this.bright) {
            midi.sendShortMsg(this.pad3[deckIndex][0], this.pad3[deckIndex][1], this.bright);
            this.pad3State[deckIndex] = this.bright;
        }
    },

    setPad3ShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.pad3_shift[deckIndex][0], this.pad3_shift[deckIndex][1], this.off);
    },
    setPad3ShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.pad3_shift[deckIndex][0], this.pad3_shift[deckIndex][1], this.dim);
    },
    setPad3ShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.pad3_shift[deckIndex][0], this.pad3_shift[deckIndex][1], this.bright);
    },


    setPad4Off: function(deckIndex) {
        if (this.pad4State[deckIndex] !== this.off) {
            midi.sendShortMsg(this.pad4[deckIndex][0], this.pad4[deckIndex][1], this.off);
            this.pad4State[deckIndex] = this.off;
        }
    },
    setPad4Dim: function(deckIndex) {
        if (this.pad4State[deckIndex] !== this.dim) {
            midi.sendShortMsg(this.pad4[deckIndex][0], this.pad4[deckIndex][1], this.dim);
            this.pad4State[deckIndex] = this.dim;
        }
    },
    setPad4Bright: function(deckIndex) {
        if (this.pad4State[deckIndex] !== this.bright) {
            midi.sendShortMsg(this.pad4[deckIndex][0], this.pad4[deckIndex][1], this.bright);
            this.pad4State[deckIndex] = this.bright;
        }
    },

    setPad4ShiftOff: function(deckIndex) {
        midi.sendShortMsg(this.pad4_shift[deckIndex][0], this.pad4_shift[deckIndex][1], this.off);
    },
    setPad4ShiftDim: function(deckIndex) {
        midi.sendShortMsg(this.pad4_shift[deckIndex][0], this.pad4_shift[deckIndex][1], this.dim);
    },
    setPad4ShiftBright: function(deckIndex) {
        midi.sendShortMsg(this.pad4_shift[deckIndex][0], this.pad4_shift[deckIndex][1], this.bright);
    },


    setStemLeds: function(currentPadMode, deckIndex, drumsStemGroup, bassStemGroup, synthsStemGroup, voiceStemGroup) {

        const isDrumStemMuted = engine.getValue(drumsStemGroup, "mute") === 1;
        const isBassStemMuted = engine.getValue(bassStemGroup, "mute") === 1;
        const isSynthsStemMuted = engine.getValue(synthsStemGroup, "mute") === 1;
        const isVoiceStemMuted = engine.getValue(voiceStemGroup, "mute") === 1;

        if (currentPadMode === 4) {
            if (isDrumStemMuted) {
                NumarkMixtrackGo.led.setPad1Off(deckIndex);
            } else {
                NumarkMixtrackGo.led.setPad1Bright(deckIndex);
            }

            if (isBassStemMuted) {
                NumarkMixtrackGo.led.setPad2Off(deckIndex);
            } else {
                NumarkMixtrackGo.led.setPad2Bright(deckIndex);
            }

            if (isSynthsStemMuted) {
                NumarkMixtrackGo.led.setPad3Off(deckIndex);
            } else {
                NumarkMixtrackGo.led.setPad3Bright(deckIndex);
            }

            if (isVoiceStemMuted) {
                NumarkMixtrackGo.led.setPad4Off(deckIndex);
            } else {
                NumarkMixtrackGo.led.setPad4Bright(deckIndex);
            }
        }

        if (isDrumStemMuted && isBassStemMuted && isSynthsStemMuted && !isVoiceStemMuted) {
            NumarkMixtrackGo.led.setAcapelBright(deckIndex);
            NumarkMixtrackGo.led.setInstruDim(deckIndex);
        } else if (!isDrumStemMuted && !isBassStemMuted && !isSynthsStemMuted && isVoiceStemMuted) {
            NumarkMixtrackGo.led.setAcapelDim(deckIndex);
            NumarkMixtrackGo.led.setInstruBright(deckIndex);
        } else {
            NumarkMixtrackGo.led.setAcapelDim(deckIndex);
            NumarkMixtrackGo.led.setInstruDim(deckIndex);
        }
    },
};


// Effect indexes
// if mixxx devs change the order of the effects, echoEffectIndex needs to be updated.
// if they ever implement methods to set effects by name or some id, a more robust solution should be implemented.
const reverbEffectIndex = 2;
const flangerEffectIndex = 12;
const echoEffectIndex = 14;

const beatloopRollControl = `beatlooproll_${beatloopRollSize}_activate`;
const beatloopPadSizes = [beatloopPad1Size, beatloopPad2Size, beatloopPad3Size, beatloopPad4Size];

// Fade FX state
let isFadeFxOn = false;

// only used for it's length
const padModes = ["hotcue", "loops", "fx", "sampler", "stems"];

// 0 for filter, 1 for low
let filterLowSwitch = 0;


let vinylModeEnabled = true;

// to disable the lights demo, from npredella@mixxxforums
const IdentityRequestSysex = [0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7];

// to retrieve controller state
// https://github.com/mixxxdj/mixxx/wiki/Serato%20sysex
const ControllerStatusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];


/**
 * Required function.
 * Sets the leds, instantiates the 2 deck objects and sets Mixxx with the controls values of the controller.
 */
NumarkMixtrackGo.init = function() {

    // to disable the lights demo, from npredella@mixxxforums
    midi.sendSysexMsg(IdentityRequestSysex, IdentityRequestSysex.length);

    this.deck = new components.ComponentContainer();
    NumarkMixtrackGo.leftDeck = new NumarkMixtrackGo.Deck(0, 1);
    NumarkMixtrackGo.rightDeck = new NumarkMixtrackGo.Deck(1, 2);

    // https://github.com/mixxxdj/mixxx/wiki/Serato%20sysex
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);

    // Led setting needs to be done after we receive a sysex response from
    // midi.sendSysexMsg(IdentityRequestSysex, IdentityRequestSysex.length);
    // Since we can't catch sysex messages and the controller sends it well under 10ms
    // after we send the Identity request message 50ms is more than enough.
    engine.beginTimer(50, () => {
        NumarkMixtrackGo.led.setCueDim(0);
        NumarkMixtrackGo.led.setCueDim(1);
        NumarkMixtrackGo.led.setPlayDim(0);
        NumarkMixtrackGo.led.setPlayDim(1);
        NumarkMixtrackGo.led.setSyncDim(0);
        NumarkMixtrackGo.led.setSyncDim(1);

        NumarkMixtrackGo.led.setModeHotcueBright(0);
        NumarkMixtrackGo.led.setModeHotcueBright(1);
        NumarkMixtrackGo.led.setModeLoopsDim(0);
        NumarkMixtrackGo.led.setModeLoopsDim(1);
        NumarkMixtrackGo.led.setModeSamplerDim(0);
        NumarkMixtrackGo.led.setModeSamplerDim(1);
        NumarkMixtrackGo.led.setModeStemsDim(0);
        NumarkMixtrackGo.led.setModeStemsDim(1);

        NumarkMixtrackGo.led.setPflDim(0);
        NumarkMixtrackGo.led.setPflDim(1);
        NumarkMixtrackGo.led.setLoadDim(0);
        NumarkMixtrackGo.led.setLoadDim(1);
        NumarkMixtrackGo.led.setFadefxDim();

        // setting shift led states
        NumarkMixtrackGo.led.setLoadShiftBright(0);
        NumarkMixtrackGo.led.setLoadShiftBright(1);
        NumarkMixtrackGo.led.setFadefxShiftDim();
        NumarkMixtrackGo.led.setCueShiftDim(0);
        NumarkMixtrackGo.led.setCueShiftDim(1);
        NumarkMixtrackGo.led.setPlayShiftDim(0);
        NumarkMixtrackGo.led.setPlayShiftDim(1);
        NumarkMixtrackGo.led.setSyncShiftDim(0);
        NumarkMixtrackGo.led.setSyncShiftDim(1);
        NumarkMixtrackGo.led.setPflShiftDim(0);
        NumarkMixtrackGo.led.setPflShiftDim(1);
    }, true);


    // Fade fx blinking
    engine.makeConnection("[App]", "indicator_500ms", function() {
        if (isFadeFxOn) {
            if (engine.getValue("[App]", "indicator_500ms") === 0) {
                NumarkMixtrackGo.led.setFadefxDim();
                NumarkMixtrackGo.led.setFadefxShiftDim();
            } else {
                NumarkMixtrackGo.led.setFadefxBright();
                NumarkMixtrackGo.led.setFadefxShiftBright();
            }
        } else {
            NumarkMixtrackGo.led.setFadefxDim();
            NumarkMixtrackGo.led.setFadefxShiftDim();
        }
    });
};

/**
 * Required function.
 */
NumarkMixtrackGo.shutdown = function() {
    NumarkMixtrackGo.led.setAllOff();
};


NumarkMixtrackGo.browseEncoder = new components.Encoder({

    isLibraryScrolling: true,

    input: function(_channel, control, value, status) {

        if (status === NumarkMixtrackGo.controls.browse_encoder_rotate[0]) {
            // also need to check for control because there is a 'browse_encoder_rotate_shift' with a different control
            if (control === NumarkMixtrackGo.controls.browse_encoder_rotate[1]) {
                if (value === 127) {
	                this.onEnconderRotateEvent(-1); //rotate right
	            } else {
	                this.onEnconderRotateEvent(1); //rotate left
	            }
            }
        } else {
            this.onButtonPushEvent();
        }
    },

    onEnconderRotateEvent: function(rotateValue) {

        if (this.isLibraryScrolling) {
            engine.setValue("[Playlist]", "SelectTrackKnob", rotateValue);
        } else { // isStripSearching
            let newPosition = engine.getValue("[PreviewDeck1]", "playposition") + previewDeckStripSearchPace*rotateValue;
            // prevents the trackhead from going past where the GUI can show, as we get the deck in playing status, but hear nothing
            if (newPosition < 0) {
                newPosition = 0;
            }
            engine.setValue("[PreviewDeck1]", "playposition", newPosition);

            // recover playing state after the track stopping without having to click the encoder again
            // if you are moving the trackhead back after hitting the end it is because you want to hear more
            if (engine.getValue("[PreviewDeck1]", "play") === 0) { // reached the end after setValue with playposition and stopped playing
                engine.setValue("[PreviewDeck1]", "play", 1);
            }
        }
    },

    onButtonPushEvent: function() {
        if (this.isLibraryScrolling) {
            this.isLibraryScrolling = false;
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
        } else {
            this.isLibraryScrolling = true;
            script.triggerControl("[PreviewDeck1]", "stop");
        }
    },
});

NumarkMixtrackGo.fadeFx = new components.Button({
    quickEffectRack1Channel1Super1Value: 0,
    quickEffectRack1Channel2Super1Value: 0,
    input: function(_channel, control, value) {
        if (control === NumarkMixtrackGo.controls.fadefx[1]) {
            if (value === 127) {
                if (isFadeFxOn === false) {
                    this.quickEffectRack1Channel1Super1Value = engine.getValue("[QuickEffectRack1_[Channel1]]", "super1");
                    this.quickEffectRack1Channel2Super1Value = engine.getValue("[QuickEffectRack1_[Channel2]]", "super1");
                    isFadeFxOn = true;
                } else {
                    isFadeFxOn = false;
                    engine.setValue("[QuickEffectRack1_[Channel1]]", "super1", this.quickEffectRack1Channel1Super1Value);
                    engine.setValue("[QuickEffectRack1_[Channel2]]", "super1", this.quickEffectRack1Channel2Super1Value);
                }
            }
        }
    },
});

// TODO should eventually be reviewed to get at least a better fader curve and/or
// implement a better solution than using quick effects
NumarkMixtrackGo.crossFader = new components.Pot({
    input: function(_channel, _control, value) {
        const newValue = script.absoluteLin(value, 0, 1, 0, 100);
        const invertedValue = script.absoluteLin((127 - value), 0, 1, 0, 100);

        if (isFadeFxOn) {
            if (value === 127) {
                engine.setValue("[QuickEffectRack1_[Channel1]]", "super1", 0);
            }
            if (value === 0) {
                engine.setValue("[QuickEffectRack1_[Channel2]]", "super1", 0);
            }

            if (value < 63) {
                engine.setValue("[Master]", "crossfader", script.absoluteLin(value, -1.0, 0, 23, 63));
            } else { // tracks get cut at about 10% past midpoint
                engine.setValue("[Master]", "crossfader", script.absoluteLin(value, 0, 1, 63, 103));
            }

            const channel1Super1StoredValue = engine.getValue("[QuickEffectRack1_[Channel1]]", "super1");
            const channel1ValueChange = channel1Super1StoredValue - newValue;
            //takeover at 5% distance
            if ((channel1ValueChange < 0.05 && channel1ValueChange > -0.05)) {
                engine.setValue("[QuickEffectRack1_[Channel1]]", "super1", newValue);
            }

            const channel2Super1StoredValue = engine.getValue("[QuickEffectRack1_[Channel2]]", "super1");
            const channel2ValueChange = channel2Super1StoredValue - invertedValue;
            //takeover at 5% distance
            if ((channel2ValueChange < 0.05 && channel2ValueChange > -0.05)) {
                engine.setValue("[QuickEffectRack1_[Channel2]]", "super1", invertedValue);
            }
        } else {
            engine.setValue("[Master]", "crossfader", script.absoluteLin(value, -1.0, 1.0, 0, 127));
        }
    }
});

NumarkMixtrackGo.Deck = function(deckIndex, deckNumber) {
    components.Deck.call(this, deckNumber);
    const group = `[Channel${deckNumber}]`;
    const padModesNumber = padModes.length;
    let currentPadMode = 0;

    let beatloopPadIndicatorControl = "indicator_500ms";
    if (beatloopPadBlinkingFast) {
        beatloopPadIndicatorControl = "indicator_250ms";
    }

    let samplerPadIndicatorControl = "indicator_500ms";
    if (samplerActiveBlinkingFast) {
        samplerPadIndicatorControl = "indicator_250ms";
    }

    // pfl status led control
    engine.makeConnection(group, "pfl", function() {
        if (engine.getValue(group, "pfl") === 1) {
            NumarkMixtrackGo.led.setPflBright(deckIndex);
        } else {
            NumarkMixtrackGo.led.setPflDim(deckIndex);
        }
    });

    // hotcue status led controls
    for (let i = 1; i <= 4; i++) {
        const hotcueStatusControl = `hotcue_${i}_status`;
        engine.makeConnection(group, hotcueStatusControl, function() {
            if (currentPadMode === 0) {
                const hotCueStatus = engine.getValue(group, hotcueStatusControl);
                if (hotCueStatus === 0) {
                    NumarkMixtrackGo.led.setPadOff(i, deckIndex);
                    NumarkMixtrackGo.led.setPadShiftOff(i, deckIndex);
                } else { //status = 1 or 2
                    NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                    NumarkMixtrackGo.led.setPadShiftBright(i, deckIndex);
                }
            }
        });
    }

    // pads beat loop led control
    if (beatloopActivePadBlinking) {
        for (let i = 1; i <= 4; i++) {
            engine.makeConnection("[App]", beatloopPadIndicatorControl, function() {
                if (currentPadMode === 1) {
                    if (engine.getValue(group, `beatloop_${beatloopPadSizes[i-1]}_enabled`) === 1) {
                        if (engine.getValue("[App]", beatloopPadIndicatorControl) === 1) {
                            NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                        } else {
                            NumarkMixtrackGo.led.setPadOff(i, deckIndex);
                        }
                    } else {
                        NumarkMixtrackGo.led.setPadOff(i, deckIndex);
                    }
                }
            });
        }
    } else {
        for (let i = 1; i <= 4; i++) {
            const beatLoopEnabledControl = `beatloop_${beatloopPadSizes[i-1]}_enabled`;
            engine.makeConnection(group, beatLoopEnabledControl, function() {
                if (currentPadMode === 1) {
                    if (engine.getValue(group, beatLoopEnabledControl) === 1) {
                        NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                    } else {
                        NumarkMixtrackGo.led.setPadOff(i, deckIndex);
                    }
                }
            });
        }
    }

    // pads [1-3] fx mode led control
    for (let i = 1; i <= 3; i++) {
        const effectRackEffectUnitGroup = `[EffectRack1_EffectUnit${i}]`;
        const effectEnableControl = `group_${group}_enable`;
        engine.makeConnection(effectRackEffectUnitGroup, effectEnableControl, function() {
            if (engine.getValue(effectRackEffectUnitGroup, effectEnableControl) === 1) {
                NumarkMixtrackGo.led.setPadBright(i, deckIndex);
            } else {
                NumarkMixtrackGo.led.setPadDim(i, deckIndex);
            }
        });
    }
    // pad 4 fx mode led control
    engine.makeConnection(group, beatloopRollControl, function() {
        if (engine.getValue(group, beatloopRollControl) === 1) {
            NumarkMixtrackGo.led.setPadBright(4, deckIndex);
        } else {
            NumarkMixtrackGo.led.setPadDim(4, deckIndex);
        }
    });

    // sampler pad loaded and playing led control
    for (let i = 1; i <= 4; i++) {
        const samplerGroup = `[Sampler${i}]`;
        engine.makeConnection(samplerGroup, "track_loaded", function() {
            if (currentPadMode === 3) {
                if (engine.getValue(samplerGroup, "track_loaded") === 1) {
                    NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                    NumarkMixtrackGo.led.setPadShiftBright(i, deckIndex);
                } else {
                    NumarkMixtrackGo.led.setPadOff(i, deckIndex);
                    NumarkMixtrackGo.led.setPadShiftOff(i, deckIndex);
                }
            }
        });
        engine.makeConnection("[App]", samplerPadIndicatorControl, function() {
            if (currentPadMode === 3) {
                if (engine.getValue(samplerGroup, "track_loaded") === 1) {
                    if (engine.getValue(samplerGroup, "play_indicator") === 1) {
                        if (engine.getValue("[App]", samplerPadIndicatorControl) === 1) {
                            NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                        } else {
                            NumarkMixtrackGo.led.setPadDim(i, deckIndex);
                        }
                    } else {
                        NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                    }
                }
            }
        });
    }



    let isStemsTrackLoaded = false;
    let isLoadedTrackBeingCheckedForStems = false;

    engine.makeConnection(group, "track_loaded", function() {
        if (engine.getValue(group, "track_loaded") === 1) {
            isStemsTrackLoaded = false;
            NumarkMixtrackGo.led.setLoadBright(deckIndex);
            NumarkMixtrackGo.led.setPlayShiftBright(deckIndex);
            NumarkMixtrackGo.led.setCueShiftBright(deckIndex);
            isLoadedTrackBeingCheckedForStems = true;
            engine.beginTimer(
                50,
                () => {
                    if (engine.getValue(group, "stem_count") > 0) {
                        isStemsTrackLoaded = true;
                        if (currentPadMode === 4) {
                            NumarkMixtrackGo.led.setPad1Bright(deckIndex);
                            NumarkMixtrackGo.led.setPad2Bright(deckIndex);
                            NumarkMixtrackGo.led.setPad3Bright(deckIndex);
                            NumarkMixtrackGo.led.setPad4Bright(deckIndex);
                        }
                        NumarkMixtrackGo.led.setAcapelDim(deckIndex);
                        NumarkMixtrackGo.led.setInstruDim(deckIndex);
                    } else {
                        if (currentPadMode === 4) {
                            isStemsTrackLoaded = false;
                            NumarkMixtrackGo.led.setPad1Off(deckIndex);
                            NumarkMixtrackGo.led.setPad2Off(deckIndex);
                            NumarkMixtrackGo.led.setPad3Off(deckIndex);
                            NumarkMixtrackGo.led.setPad4Off(deckIndex);
                        }
                        NumarkMixtrackGo.led.setAcapelOff(deckIndex);
                        NumarkMixtrackGo.led.setInstruOff(deckIndex);
                    }
                    isLoadedTrackBeingCheckedForStems = false;
                },
                true);
        } else {
            NumarkMixtrackGo.led.setLoadDim(deckIndex);
            NumarkMixtrackGo.led.setPlayShiftDim(deckIndex);
            NumarkMixtrackGo.led.setCueShiftDim(deckIndex);
            isStemsTrackLoaded = false;
            if (currentPadMode === 4) {
                NumarkMixtrackGo.led.setPad1Off(deckIndex);
                NumarkMixtrackGo.led.setPad2Off(deckIndex);
                NumarkMixtrackGo.led.setPad3Off(deckIndex);
                NumarkMixtrackGo.led.setPad4Off(deckIndex);
            }
            NumarkMixtrackGo.led.setAcapelOff(deckIndex);
            NumarkMixtrackGo.led.setInstruOff(deckIndex);
        }
    });

    // stem led coordination
    const drumsStemGroup = `[Channel${deckNumber}_Stem1]`;
    const bassStemGroup = `[Channel${deckNumber}_Stem2]`;
    const synthsStemGroup = `[Channel${deckNumber}_Stem3]`;
    const voiceStemGroup = `[Channel${deckNumber}_Stem4]`;

    engine.makeConnection(drumsStemGroup, "mute", function() {
        if (!isLoadedTrackBeingCheckedForStems) {
            NumarkMixtrackGo.led.setStemLeds(currentPadMode, deckIndex, drumsStemGroup, bassStemGroup, synthsStemGroup, voiceStemGroup);
        }
    });
    engine.makeConnection(bassStemGroup, "mute", function() {
        if (!isLoadedTrackBeingCheckedForStems) {
            NumarkMixtrackGo.led.setStemLeds(currentPadMode, deckIndex, drumsStemGroup, bassStemGroup, synthsStemGroup, voiceStemGroup);
        }
    });
    engine.makeConnection(synthsStemGroup, "mute", function() {
        if (!isLoadedTrackBeingCheckedForStems) {
            NumarkMixtrackGo.led.setStemLeds(currentPadMode, deckIndex, drumsStemGroup, bassStemGroup, synthsStemGroup, voiceStemGroup);
        }
    });
    engine.makeConnection(voiceStemGroup, "mute", function() {
        if (!isLoadedTrackBeingCheckedForStems) {
            NumarkMixtrackGo.led.setStemLeds(currentPadMode, deckIndex, drumsStemGroup, bassStemGroup, synthsStemGroup, voiceStemGroup);
        }
    });

    // sync led control
    engine.makeConnection(group, "sync_enabled", function() {
        const syncIndicatorState = engine.getValue(group, "sync_enabled");
        if (syncIndicatorState === 0) {
            NumarkMixtrackGo.led.setSyncDim(deckIndex);
        } else if (syncIndicatorState === 1) {
            NumarkMixtrackGo.led.setSyncBright(deckIndex);
        }
    });
    // cue led control
    engine.makeConnection(group, "cue_indicator", function() {
        const cueIndicatorState = engine.getValue(group, "cue_indicator");
        if (cueIndicatorState === 0) {
            NumarkMixtrackGo.led.setCueDim(deckIndex);
        } else if (cueIndicatorState === 1 || cueIndicatorState === 2) {
            NumarkMixtrackGo.led.setCueBright(deckIndex);
        }
    });
    // play led control - if play stops and fade fx is on, the effects need to be reset (same on serato)
    engine.makeConnection(group, "play", function() {
        const playState = engine.getValue(group, "play");
        if (playState === 0) {
            NumarkMixtrackGo.led.setPlayDim(deckIndex);
            if (isFadeFxOn) {
                engine.setValue("[QuickEffectRack1_[Channel1]]", "super1", 0);
                engine.setValue("[QuickEffectRack1_[Channel2]]", "super1", 0);
            }
        } else if (playState === 1) {
            NumarkMixtrackGo.led.setPlayBright(deckIndex);
        }
    });
    // had to implement this because the midi learning tool's reverse wasn't working
    this.tempoFader = new components.Pot({
        msb: 0,
        lsb: 0,
        input: function(_channel, control, value) {
            if (control === 0x09) { // midi control received for the msb value
            // this.msb = value;
            }
            if (control === 0x29) { // midi control received for the lsb value
                this.lsb = value;
            }

            const forteenBitValue = (this.msb << 7) | this.lsb;// Construct the 14-bit number

            // Range is 0x0000..0x3FFF center @ 0x2000, i.e. 0..16383 center @ 8192
            const rate = (forteenBitValue - 8192) / 8191;

            // on this specific controller the resulting value needs to be inverted
            // (the midi learning tool invert was failing and this is why i needed to implement the control in js)
            const invertedRate = rate * -1;

            engine.setValue(group, "rate", invertedRate);
        }
    });
    // changes the mode and sets the pad leds accordingly
    this.mode = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                if (currentPadMode < padModesNumber - 1) {
                    currentPadMode = currentPadMode + 1;
                } else {
                    currentPadMode = 0;
                }

                switch (currentPadMode) {
                case 0: { // hotcue
                    NumarkMixtrackGo.led.setModeStemsDim(deckIndex);
                    NumarkMixtrackGo.led.setModeHotcueBright(deckIndex);

                    for (let i = 1; i <= 4; i++) {
                        const hotCueStatus = engine.getValue(group, `hotcue_${i}_status`);
                        if (hotCueStatus === 0) {
                            NumarkMixtrackGo.led.setPadOff(i, deckIndex);
                            NumarkMixtrackGo.led.setPadShiftOff(i, deckIndex);
                        } else { //status = 1 or 2
                            NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                            NumarkMixtrackGo.led.setPadShiftBright(i, deckIndex);
                        }
                    }
                }
                    break;
                case 1: // loops
                    NumarkMixtrackGo.led.setModeHotcueDim(deckIndex);
                    NumarkMixtrackGo.led.setModeLoopsBright(deckIndex);

                    NumarkMixtrackGo.led.setAllPadsOff(deckIndex);
                    NumarkMixtrackGo.led.setAllPadsShiftOff(deckIndex);
                    break;
                case 2: // fx
                    NumarkMixtrackGo.led.setModeSamplerBright(deckIndex);

                    NumarkMixtrackGo.led.setAllPadsDim(deckIndex);
                    NumarkMixtrackGo.led.setAllPadsShiftOff(deckIndex);
                    break;
                case 3: // sampler
                    NumarkMixtrackGo.led.setModeLoopsDim(deckIndex);
                    NumarkMixtrackGo.led.setAllPadsOff(deckIndex);

                    for (let i = 1; i <= 4; i++) {
                        if (engine.getValue(`[Sampler${i}]`, "track_loaded") === 1) {
                            NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                            NumarkMixtrackGo.led.setPadShiftBright(i, deckIndex);
                        } else {
                            NumarkMixtrackGo.led.setPadOff(i, deckIndex);
                            NumarkMixtrackGo.led.setPadShiftOff(i, deckIndex);
                        }
                    }
                    break;
                case 4: // stems
                    NumarkMixtrackGo.led.setModeSamplerDim(deckIndex);
                    NumarkMixtrackGo.led.setModeStemsBright(deckIndex);

                    NumarkMixtrackGo.led.setAllPadsOff(deckIndex);
                    NumarkMixtrackGo.led.setAllPadsShiftOff(deckIndex);

                    if (engine.getValue(group, "stem_count") > 0) {
                        isStemsTrackLoaded = true;
                        for (let i = 1; i <= 4; i++) {
                            if (engine.getValue(`[Channel${deckNumber}_Stem${i}]`, "mute") === 0) {
                                NumarkMixtrackGo.led.setPadBright(i, deckIndex);
                            } else {
                                NumarkMixtrackGo.led.setPadOff(i, deckIndex);
                            }
                        }
                    }
                    break;
                }
            }
        }
    });//mode end

    this.jogTouch = function(_channel, _control, value) {
        if (vinylModeEnabled) {
            if (value === 127) {
                const alpha = 1.0/8;
                engine.scratchEnable(deckNumber, 128, 33+1/3, alpha, alpha/32);
            } else {
                engine.scratchDisable(deckNumber);
            }
        }
    };

    this.jogTurn = function(_channel, _control, value) {
        let newValue;
        if (value < 64) {
            newValue = value;
        } else {
            newValue = value - 128;
        }

        if (engine.isScratching(deckNumber)) {
            engine.scratchTick(deckNumber, newValue); // Scratch!
        } else {
            engine.setValue(group, "jog", newValue); // Pitch bend
        }
    };

    this.filterLowSwitcher = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                if (filterLowSwitch === 0) {
                    filterLowSwitch = 1;
                } else {
                    filterLowSwitch = 0;
                }
            }
        },
    });

    this.vinylModeSwitcher = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                if (vinylModeEnabled) {
                    vinylModeEnabled = false; ;
                } else {
                    vinylModeEnabled = true;
                }
            }
        },
    });

    this.filterLowPot = new components.Pot({
        quickEffectRackGroup: `[QuickEffectRack1_[Channel${deckNumber}]]`,
        lowEqGroup: `[EqualizerRack1_[Channel${deckNumber}]_Effect1]`,
        super1StoredValue: 0,
        parameter1StoredValue: 0,
        newValue: 0,
        valueChange: 0,

        input: function(_channel, _control, value) {
            this.newValue = Math.round(script.absoluteLin(value, 0, 1, 0, 127) * 100) / 100;
            this.valueChange = 0;

            if (filterLowSwitch === 0) {
                // Filter - since this will use QuickEffectRack1, the effect is whatever the user has set
                // Setting the filter programmatically is a bad idea because whatever is set, if it's parameters where modified,
                // they'd be reset if filter was set programmatically and the sound could change drastically.
                this.super1StoredValue = engine.getValue(this.quickEffectRackGroup, "super1");
                this.valueChange = this.super1StoredValue - this.newValue;

                //takeover at 2% distance
                if ((this.valueChange < 0.02 && this.valueChange > -0.02)) {
                    this.super1StoredValue = this.newValue;
                    engine.setValue(this.quickEffectRackGroup, "super1", this.newValue);
                }
            } else {
                // Low
                // warning [Main] "EffectParameter(Low)" WARNING: Value was outside of limits, clamped.
                // getting this warning when script.absoluteLin returns 1
                this.parameter1StoredValue = engine.getParameter(this.lowEqGroup, "parameter1");
                this.valueChange = this.parameter1StoredValue - this.newValue;

                //takeover at 2% distance
                if ((this.valueChange < 0.02 && this.valueChange > -0.02)) {
                    this.parameter1StoredValue = this.newValue;
                    engine.setParameter(this.lowEqGroup, "parameter1", this.newValue);
                }
            }
        },
    });

    this.pflButton = new components.Button({
        input: function(_channel, _control, value, _status, group) {
            if (value === 127) {
                script.toggleControl(group, "pfl");
                if (engine.getValue(group, "pfl") === 1) {
                    NumarkMixtrackGo.led.setPflBright(deckIndex);
                } else {
                    NumarkMixtrackGo.led.setPflDim(deckIndex);
                }
            }
        }
    });

    this.acapelButton = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                if (isStemsTrackLoaded) {
                    if (engine.getValue(drumsStemGroup, "mute") === 1 &&
                    engine.getValue(bassStemGroup, "mute") === 1 &&
                    engine.getValue(synthsStemGroup, "mute") === 1 &&
                    engine.getValue(voiceStemGroup, "mute") === 0) {

                        engine.setValue(drumsStemGroup, "mute", 0);
                        engine.setValue(bassStemGroup, "mute", 0);
                        engine.setValue(synthsStemGroup, "mute", 0);
                    } else {
                        engine.setValue(drumsStemGroup, "mute", 1);
                        engine.setValue(bassStemGroup, "mute", 1);
                        engine.setValue(synthsStemGroup, "mute", 1);
                        engine.setValue(voiceStemGroup, "mute", 0);
                    }
                }
            }
        }
    });

    this.instruButton = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                if (isStemsTrackLoaded) {
                    if (engine.getValue(drumsStemGroup, "mute") === 0 &&
                    engine.getValue(bassStemGroup, "mute") === 0 &&
                    engine.getValue(synthsStemGroup, "mute") === 0 &&
                    engine.getValue(voiceStemGroup, "mute") === 1) {
                        engine.setValue(voiceStemGroup, "mute", 0);
                    } else {
                        engine.setValue(drumsStemGroup, "mute", 0);
                        engine.setValue(bassStemGroup, "mute", 0);
                        engine.setValue(synthsStemGroup, "mute", 0);
                        engine.setValue(voiceStemGroup, "mute", 1);
                    }
                }
            }
        }
    });

    this.syncButton = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                script.toggleControl(group, "sync_enabled");
            }
        }
    });

    this.cueGoToAndStopButton = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                script.triggerControl(group, "cue_gotoandstop");
            }
        }
    });

    this.cueGoToAndPlayDoubleTapButton = new components.Button({
        timerFinished: true,
        timerId: 0,
        input: function(_channel, _control, value) {
            if (value === 127) {
                if (this.timerFinished) {
                    this.timerFinished = false;
                    script.triggerControl(group, "cue_gotoandplay");
                    this.timerId = engine.beginTimer(shiftCueDoubleTapTimer, () => {
                        this.timerFinished = true;
                    }, true);
                } else {
                    this.timerFinished = true;
                    engine.stopTimer(this.timerId);
                    engine.setValue("[Playlist]", "SelectTrackKnob", -1);
                    // load previous track and cue go to and play
                    script.triggerControl(group, "LoadSelectedTrackAndPlay");
                }
            }
        }
    });

    this.playButton = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                script.toggleControl(group, "play");
            }
        }
    });

    this.playCueGoToAndPlayButton = new components.Button({
        input: function(_channel, _control, value) {
            if (value === 127) {
                script.triggerControl(group, "cue_gotoandplay");
            }
        }
    });

    this.pads = [];
    for (let i = 0; i < 4; i++) {
        this.pads[i] = new components.Button({
            padNumber: i+1,
            hotcueStatusControl: `hotcue_${(i+1)}_status`,
            hotcueSetControl: `hotcue_${(i+1)}_set`,
            hotcueGoToAndPlayControl: `hotcue_${(i+1)}_gotoandplay`,
            hotcueClearControl: `hotcue_${(i+1)}_clear`,
            effectUnitGroup: `[EffectRack1_EffectUnit${(i+1)}]`,
            effectUnitEffect1Group: `[EffectRack1_EffectUnit${(i+1)}_Effect1]`,
            effectEnableControl: `group_${group}_enable`,
            samplerGroup: `[Sampler${(i+1)}]`,
            stemGroup: `[Channel${deckNumber}_Stem${(i+1)}]`,

            input: function(_channel, control, value) {
                switch (currentPadMode) {
                case 0: // hotcue
                    if (value === 127) {
                        if (control === 0x14 || control === 0x15 || control === 0x16 || control === 0x17) {
                            if (engine.getValue(group, this.hotcueStatusControl) === 0) {
                                engine.setValue(group, this.hotcueSetControl, 1);
                            } else if (engine.getValue(group, this.hotcueStatusControl) === 1) {
                                engine.setValue(group, this.hotcueGoToAndPlayControl, 1);
                            } else { //status = 2
                                //Hotcue X is active (saved loop is enabled or hotcue is previewing)
                                console.log("ignoring hotcue status 2");
                            }
                        } else { //control = 0x1C or 0x1D or 0x1E or 0x1F
                            engine.setValue(group, this.hotcueClearControl, 1);
                        }
                    }
                    break;
                case 1: // loops
                    if (value === 127) {
                        engine.setValue(group, `beatloop_${beatloopPadSizes[this.padNumber - 1]}_toggle`, 1);
                        // to speed up the feedback from cliking the button since engine.connection may take up to 500ms
                        NumarkMixtrackGo.led.setAllPadsOff(deckIndex);
                        if (engine.getValue(group, `beatloop_${beatloopPadSizes[this.padNumber - 1]}_enabled`) === 1) {
                            NumarkMixtrackGo.led.setPadBright(this.padNumber, deckIndex);
                        } else {
                            NumarkMixtrackGo.led.setPadOff(this.padNumber, deckIndex);
                        }
                    }
                    break;
                case 2: // fx - if effect unit not loaded sets some fx and waits 200ms to set params on some
                    if (value === 127) {
                        if (this.padNumber < 4) {
                            if (engine.getValue(this.effectUnitEffect1Group, "loaded") === 0) {
                                if (this.padNumber === 1) {
                                    for (let i = 0; i <= echoEffectIndex; i++) {
                                        engine.setValue(this.effectUnitEffect1Group, "next_effect", 1);
                                    }
                                    engine.beginTimer(200, () => {
                                        engine.setValue(this.effectUnitEffect1Group, "meta", autoLoadedEchoMetaValue);
                                        engine.setValue(this.effectUnitGroup, "mix", autoLoadedEchoMixValue);
                                        engine.setValue(this.effectUnitEffect1Group, "enabled", 1);
                                        engine.setValue(this.effectUnitGroup, this.effectEnableControl, 1);
                                    }, true);
                                } else if (this.padNumber === 2) {
                                    for (let i = 0; i <= flangerEffectIndex; i++) {
                                        engine.setValue(this.effectUnitEffect1Group, "next_effect", 1);
                                    }
                                    engine.beginTimer(200, () => {
                                        engine.setValue(this.effectUnitEffect1Group, "meta", autoLoadedFlangerMetaValue);
                                        engine.setValue(this.effectUnitGroup, "mix", autoLoadedFlangerMixValue);
                                        engine.setValue(this.effectUnitEffect1Group, "enabled", 1);
                                        engine.setValue(this.effectUnitGroup, this.effectEnableControl, 1);
                                    }, true);
                                } else { // padnumber = 3
                                    for (let i = 0; i <= reverbEffectIndex; i++) {
                                        engine.setValue(this.effectUnitEffect1Group, "next_effect", 1);
                                    }
                                    engine.beginTimer(200, () => {
                                        engine.setValue(this.effectUnitEffect1Group, "meta", autoLoadedReverbMetaValue);
                                        engine.setValue(this.effectUnitGroup, "mix", autoLoadedReverbMixValue);
                                        engine.setValue(this.effectUnitEffect1Group, "enabled", 1);
                                        engine.setValue(this.effectUnitGroup, this.effectEnableControl, 1);
                                    }, true);
                                }
                            } else { // the effect unit was loaded
                                engine.setValue(this.effectUnitEffect1Group, "enabled", 1);
                                engine.setValue(this.effectUnitGroup, this.effectEnableControl, 1);
                            }
                        } else { // pad 4 action
                            engine.setValue(group, beatloopRollControl, 1);
                        }
                    } else { // pad released
                        if (this.padNumber < 4) {
                            engine.setValue(this.effectUnitGroup, this.effectEnableControl, 0);
                        } else {
                            engine.setValue(group, beatloopRollControl, 0);
                        }
                    }
                    break;
                case 3:
                    if (value === 127) {
                        if (control === 0x14 || control === 0x15 || control === 0x16 || control === 0x17) {
                            if (engine.getValue(this.samplerGroup, "track_loaded") === 0) {
                                engine.setValue(this.samplerGroup, "LoadSelectedTrack", 1);

                            } else {
                                engine.setValue(this.samplerGroup, "cue_gotoandplay", 1);
                            }
                        } else { //control = 0x1C or 0x1D or 0x1E or 0x1F
                            engine.setValue(this.samplerGroup, "eject", 1);
                        }
                    }
                    break;
                case 4: // stems
                    if (value === 127) {
                        if (isStemsTrackLoaded) {
                            script.toggleControl(this.stemGroup, "mute");
                        }
                    }
                    break;
                }
            }
        });
    }// pads end
};

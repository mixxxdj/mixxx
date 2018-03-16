var DJ202 = {};

DJ202.tempoRange = [0.08, 0.16, 0.5]

DJ202.init = function () {

    DJ202.leftDeck = new DJ202.Deck([1,3], 0);
    DJ202.rightDeck = new DJ202.Deck([2,4], 1);
    
    DJ202.effectUnit = [];
    DJ202.effectUnit[1] = new DJ202.EffectUnit(1);
    DJ202.effectUnit[2] = new DJ202.EffectUnit(2);
    
    
    if (engine.getValue('[Master]', 'num_samplers') < 16) {
        engine.setValue('[Master]', 'num_samplers', 16);
    }
};

DJ202.shutdown = function () {
};

DJ202.browseEncoder = new components.Encoder({
    midi: [0xBF, 0x00],
    group: '[Playlist]',
    inKey: 'SelectTrackKnob',
    input: function (channel, control, value, status, group) {
        if (value === 1) {
            this.inSetParameter(1);
        } else if (value === 127) {
            this.inSetParameter(-1);
        }
    },
});

DJ202.crossfader = new components.Pot({
    midi: [0xBF, 0x08],
    group: '[Master]',
    inKey: 'crossfader',
});

DJ202.Deck = function (deckNumbers, offset) {
    components.Deck.call(this, deckNumbers);
    channel = offset+1;

    this.shiftButton = function (channel, control, value, status, group) {
        if (value === 127) {
            this.shift();
        } else {
            this.unshift();
        }
    };

    this.loadTrack = new components.Button({
        midi: [0x9F, 0x02 + offset],
        unshift: function () {
            this.inKey = 'LoadSelectedTrack';
        },
        shift: function () {
            this.inKey = 'eject';
        },
    });

    this.keylock = new components.Button({
        midi: [0x90 + offset, 0x0D],
        shiftOffset: 1,
        shiftControl: true,
        sendShifted: true,
        outKey: 'keylock',
        currentRangeIndex: 0,
        unshift: function () {
            this.type = components.Button.prototype.types.toggle;
            this.input = components.Button.prototype.input;
            this.inKey = 'keylock';
        },
        shift: function () {
            this.inKey = 'rateRange';
            this.type = undefined;
            this.input = function (channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status)) {
                    print(this.currentRangeIndex)
                    this.currentRangeIndex++;
                    if (this.currentRangeIndex >= DJ202.tempoRange.length) {
                        this.currentRangeIndex = 0;
                    }
                    this.inSetValue(DJ202.tempoRange[this.currentRangeIndex]);
                }
            }
        }
    });
    
    engine.setValue(this.currentDeck, "rate_dir", -1);
    this.tempoFader = new components.Pot({
        midi: [0xB0 + offset, 0x09],
        inKey: 'rate',
    });
    
    
    // ============================= JOG WHEELS =================================
    this.wheelTouch = function (channel, control, value, status, group) {
      if (value === 0x7F) {
            var alpha = 1.0/8;
            var beta = alpha/32;
            engine.scratchEnable(script.deckFromGroup(this.currentDeck), 512, 45, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    }
    
    this.wheelTurn = function (channel, control, value, status, group) {
        var newValue = value - 64;
        if (engine.isScratching(1)) {
            engine.scratchTick(script.deckFromGroup(this.currentDeck), newValue); // Scratch!
        } else {
            engine.setValue(this.currentDeck, 'jog', newValue); // Pitch bend
        }
    }
    
    
    
    // ========================== PERFORMANCE PADS ==============================
    this.hotcueButton = [];
    this.samplerButton = [];
    this.loopButton = [];
    
    samplerNumber = [];
    samplerNumber[1] = [1,2,3,4, 9,10,11,12]
    samplerNumber[2] = [5,6,7,8, 13,14,15,16]
    
    for (var i = 1; i <= 8; i++) {
        this.hotcueButton[i] = new components.HotcueButton({
            midi: [0x94 + offset, 0x00 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i,
        });
        
        this.samplerButton[i] = new components.SamplerButton({
            midi: [0x94 + offset, 0x20 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: samplerNumber[channel][i],
        });
    }
    

    for (var i = 1; i <= 4; i++) {
        this.loopButton[i] = new components.Button({
            midi: [0x94 + offset, 0x10 + i],
            inKey: 'beatloop_'+ Math.pow(2,i-1) +'_activate',
        });
    }
    
    this.loopIn = new components.Button({
        midi: [0x94 + offset, 0x15],
        inKey: 'loop_in',
    });
    this.loopOut = new components.Button({
        midi: [0x94 + offset, 0x16],
        inKey: 'loop_out',
    });
    this.loopToggle = new components.Button({
        midi: [0x94 + offset, 0x18],
        inKey: 'reloop_toggle',
    });

    
    // ============================= TRANSPORT ==================================
    this.play = new components.PlayButton([0x90 + offset, 0x00]); // LED doesn't stay on
    this.cue = new components.CueButton([0x90 + offset, 0x01]);
    this.sync = new components.SyncButton([0x90 + offset, 0x02]); // doesn't work properly

    // =============================== MIXER ====================================
    this.pregain = new components.Pot({
            midi: [0xB0 + offset, 0x16],
            inKey: 'pregain',
    });
    
    this.eqKnob = [];
    for (var k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            midi: [0xB0 + offset, 0x20 - k],
            group: '[EqualizerRack1_' + this.currentDeck + '_Effect1]',
            inKey: 'parameter' + k,
        });
    }
    
    this.filter = new components.Pot({
        midi: [0xB0 + offset, 0x1A],
        group: '[QuickEffectRack1_' + this.currentDeck + ']',
        inKey: 'super1',
    });

    this.pfl = new components.Button({
        midi: [0x90 + offset, 0x1B],
        type: components.Button.prototype.types.toggle,
        inKey: 'pfl',
        outKey: 'pfl',
        // missing: shift -> TAP
    });

    this.volume = new components.Pot({
        midi: [0xB0 + offset, 0x1C],
        inKey: 'volume',
    });

    this.reconnectComponents(function (component) {
        if (component.group === undefined) {
            component.group = this.currentDeck;
        }
    });
};
DJ202.Deck.prototype = new components.Deck();




///////////////////////////////////////////////////////////////
//                             FX                            //
///////////////////////////////////////////////////////////////

DJ202.EffectUnit = function (unitNumber) {
    var eu = this;
    this.group = '[EffectRack1_EffectUnit' + unitNumber + ']';
    engine.setValue(this.group, 'show_focus', 1);

    this.EffectButton = function (buttonNumber) {
        this.buttonNumber = buttonNumber;

        this.group = eu.group;
        this.midi = [0x98 + unitNumber-1, 0x00 + buttonNumber-1];

        components.Button.call(this);
    };
    this.EffectButton.prototype = new components.Button({
        input: function (channel, control, value, status) {
            if (this.isPress(channel, control, value, status)) {
                this.isLongPressed = false;
                this.longPressTimer = engine.beginTimer(this.longPressTimeout, function () {
                    var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + this.buttonNumber + ']';
                    script.toggleControl(effectGroup, 'enabled');
                    this.isLongPressed = true;
                }, true);
            } else {
                if (!this.isLongPressed) {
                    var focusedEffect = engine.getValue(eu.group, 'focused_effect');
                    if (focusedEffect === this.buttonNumber) {
                        engine.setValue(eu.group, 'focused_effect', 0);
                    } else {
                        engine.setValue(eu.group, 'focused_effect', this.buttonNumber);
                    }
                }
                this.isLongPressed = false;
                engine.stopTimer(this.longPressTimer);
            }
        },
        outKey: 'focused_effect',
        output: function (value, group, control) {
            this.send((value === this.buttonNumber) ? this.on : this.off);
        },
        sendShifted: true,
        shiftOffset: 0x0B,
    });

    this.button = [];
    for (var i = 1; i <= 3; i++) {
        this.button[i] = new this.EffectButton(i);

        var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + i + ']';
        engine.softTakeover(effectGroup, 'meta', true);
        engine.softTakeover(eu.group, 'mix', true);
    }

    this.knob = new components.Pot({
        unshift: function () {
            this.input = function (channel, control, value, status) {
                value = (this.MSB << 7) + value;

                var focusedEffect = engine.getValue(eu.group, 'focused_effect');
                if (focusedEffect === 0) {
                    engine.setParameter(eu.group, 'mix', value / this.max);
                } else {
                    var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + focusedEffect + ']';
                    engine.setParameter(effectGroup, 'meta', value / this.max);
                }
            };
        },
    });

    this.knobSoftTakeoverHandler = engine.makeConnection(eu.group, 'focused_effect', function (value, group, control) {
        if (value === 0) {
            engine.softTakeoverIgnoreNextValue(eu.group, 'mix');
        } else {
            var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + value + ']';
            engine.softTakeoverIgnoreNextValue(effectGroup, 'meta');
        }
    });
};

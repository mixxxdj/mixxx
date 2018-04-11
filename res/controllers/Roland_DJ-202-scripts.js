var DJ202 = {};

DJ202.tempoRange = [0.08, 0.16, 0.5]

DJ202.init = function () {

    DJ202.shiftButton = function (channel, control, value, status, group) {
        DJ202.deck.concat(DJ202.effectUnit).forEach(
            value
                ? function (module) { module.shift(); }
                : function (module) { module.unshift(); }
        );
    };

    DJ202.leftDeck = new DJ202.Deck([1,3], 0);
    DJ202.rightDeck = new DJ202.Deck([2,4], 1);
    DJ202.deck = [DJ202.leftDeck, DJ202.rightDeck];

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
    input: function (channel, control, value, status, group) {
        var isShifted = control % 2 != 0;
        switch (status) {
        case 0xBF: // Rotate.
            if (value === 1) {
                script.triggerControl(group, isShifted ? 'ScrollUp' : 'MoveUp');
            } else if (value === 127) {
                script.triggerControl(group, isShifted ? 'ScrollDown' : 'MoveDown');
            }
            break;
        case 0x9F: // Push.
            if (value) {
                script.triggerControl(group, isShifted ? 'MoveFocusBackward' : 'MoveFocusForward');
            }
        }
    }
});

DJ202.crossfader = new components.Pot({
    midi: [0xBF, 0x08],
    group: '[Master]',
    inKey: 'crossfader',
});

DJ202.Deck = function (deckNumbers, offset) {
    components.Deck.call(this, deckNumbers);
    channel = offset+1;

    this.loadTrack = new components.Button({
        midi: [0x9F, 0x02 + offset],
        unshift: function () {
            this.inKey = 'LoadSelectedTrack';
        },
        shift: function () {
            this.inKey = 'eject';
        },
    });

    this.paramUp = function (channel, control, value, status, group) {
        if (value) {
            this.paramUp.active = true;
            if (this.paramDown.active) {
                script.triggerControl(group, 'reset_key');
            } else if (this.keylock.is_held) {
                var adjust = engine.getValue(group, 'pitch_adjust');
                engine.setValue(group, 'pitch_adjust', Math.min(7, adjust + 1));
            }
        } else {
            this.paramUp.active = false;
        }
    };

    this.paramDown = function (channel, control, value, status, group) {
        if (value) {
            this.paramDown.active = true;
            if (this.paramUp.active) {
                script.triggerControl(group, 'reset_key');
            } else if (this.keylock.is_held) {
                var adjust = engine.getValue(group, 'pitch_adjust');
                engine.setValue(group, 'pitch_adjust', Math.max(-7, adjust - 1));
            }
        } else {
            this.paramDown.active = false;
        }
    };

    this.keylock = new components.Button({
        midi: [0x90 + offset, 0x0D],
        shiftOffset: 1,
        shiftControl: true,
        sendShifted: true,
        outKey: 'keylock',
        currentRangeIndex: 0,
        unshift: function () {
            this.input = function (channel, control, value, status, group) {
                if (value) {
                    this.longPressTimer = engine.beginTimer(this.longPressTimeout, function () {
                        this.is_held = true;
                    }, true);
                } else {
                    if (!this.is_held) {
                        script.toggleControl(this.group, this.outKey);
                    };
                    engine.stopTimer(this.longPressTimer);
                    this.is_held = false;
                }
            };
            this.inKey = 'keylock';
            this.outKey = 'keylock';
            // The DJ-202 disables the keylock LED when the button is pressed
            // shifted. Restore the LED when shift is released.
            this.send(this.outGetValue());
            midi.sendShortMsg(0x84, 0x00, 0x3);
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
      if (value === 0x7F && !this.isShifted) {
            var alpha = 1.0/8;
            var beta = alpha/32;
            engine.scratchEnable(script.deckFromGroup(this.currentDeck), 512, 45, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    }
    
    this.wheelTurn = function (channel, control, value, status, group) {
        var newValue = value - 64;
        var deck = script.deckFromGroup(this.currentDeck);
        if (engine.isScratching(deck)) {
            engine.scratchTick(deck, newValue); // Scratch!
        } else if (this.isShifted) {
            // Strip search.
            var oldPos = engine.getValue(this.currentDeck, 'playposition');
            var newPos = Math.max(0, oldPos + newValue / 0xff);
            engine.setValue(this.currentDeck, 'playposition', newPos);
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
    var SyncButton = function (options) {
        components.SyncButton.call(this, options);
    };

    SyncButton.prototype = new components.SyncButton({
        doubleTapTimeout: 500,
        unshift: function () {
            this.input = function (channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status)) {
                    if (this.isDoubleTap) {
                        var fileBPM = engine.getValue(this.group, 'file_bpm');
                        engine.setValue(this.group, 'bpm', fileBPM);
                    } else if (engine.getValue(this.group, 'sync_enabled') === 0) {
                        engine.setValue(this.group, 'beatsync', 1);
                        this.longPressTimer = engine.beginTimer(this.longPressTimeout, function () {
                            engine.setValue(this.group, 'sync_enabled', 1);
                            this.longPressTimer = 0;
                        }, true);
                        this.isDoubleTap = true; // For the next call.
                        this.doubleTapTimer = engine.beginTimer(this.doubleTapTimeout, function () {
                            this.isDoubleTap = false;
                        }, true);
                    } else {
                        engine.setValue(this.group, 'sync_enabled', 0);
                    };
                } else {
                    if (this.longPressTimer !== 0) {
                        engine.stopTimer(this.longPressTimer);
                        this.longPressTimer = 0;
                    };
                    // Apparently some DJ-202 button LEDS reset themselves when
                    // a button is released, so we need to re-enable the LED
                    // again.
                    if(engine.getValue(group, 'sync_enabled')) {
                        midi.sendShortMsg(0x90 + offset, 0x02, 0x7f);
                    }
                };
            };

            var ledControl = engine.getValue(this.group, 'sync_enabled') ? 0x02 : 0x03;
            this.output = function (value, group, control) {
                // To disable the sync LED, the note must be shifted. This
                // corresponds to the DJ-202 surface, which has sync off on the
                // shift layer.
                midi.sendShortMsg(0x90 + offset, ledControl, 0x7f);
            };
            // Pressing shift + sync off on the controller will disable the sync
            // LED even when sync is still enabled within mixxx.
            midi.sendShortMsg(0x90 + offset, ledControl, 0x7f);
        },

    });

    this.sync = new SyncButton();

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
    });

    this.tapBPM = new components.Button({
        input: function (channel, control, value, status, group) {
        if (value == 127) {
                script.triggerControl(group, 'beats_translate_curpos');
                bpm.tapButton(script.deckFromGroup(group));
                this.longPressTimer = engine.beginTimer(
                    this.longPressTimeout,
                    function () {
                        script.triggerControl(group, 'beats_translate_match_alignment');
                    },
                    true
                );
            } else {
                engine.stopTimer(this.longPressTimer);
        }
        }
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

    this.shift = function () {
        this.button.forEach(function (button) {
            button.shift();
        });
        this.knob.shift();
    };

    this.unshift = function () {
        this.button.forEach(function (button) {
            button.unshift();
        });
        this.knob.unshift();
    };

    this.EffectButton = function (buttonNumber) {
        this.buttonNumber = buttonNumber;

        this.group = eu.group;
        this.midi = [0x98 + unitNumber-1, 0x00 + buttonNumber-1];

        components.Button.call(this);
    };
    this.EffectButton.prototype = new components.Button({
        unshift: function() {
            this.input = function (channel, control, value, status) {
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
            }
            this.outKey = 'focused_effect';
            this.output = function (value, group, control) {
                this.send((value === this.buttonNumber) ? this.on : this.off);
            };
            this.sendShifted = true;
            this.shiftOffset = 0x0B;
        },
        shift: function () {
            this.input = function (channel, control, value, status) {
                var group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + this.buttonNumber + ']';
                script.toggleControl(group, 'next_effect');
            };
        }
    });

    this.button = [];
    for (var i = 1; i <= 3; i++) {
        this.button[i] = new this.EffectButton(i);

        var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + i + ']';
        engine.softTakeover(effectGroup, 'meta', true);
        engine.softTakeover(eu.group, 'mix', true);
    }

    this.headphones = new components.Button({
        group: '[EffectRack1_EffectUnit' + unitNumber + ']',
        midi: [0x98, 0x04],
        unshift: function() {
            this.outKey = 'group_[Headphone]_enable';
            this.inKey = this.outKey;
            this.input = function (channel, control, value, status) {
                // FIXME Trigger *after* release, to work-around the device
                // disabling the LED on release. Refactor this once a customized
                // ‘Button’ class is available.
                if (!value) {
                    script.toggleControl(this.group, this.outKey);
                };
            };
        }
    });

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
        shift: function() {
            this.input = function (channel, control, value, status) {
                var group = '[EffectRack1_EffectUnit' + unitNumber + ']';
                engine.setParameter(group, 'super1', value / 0x7f);
            }
        }
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

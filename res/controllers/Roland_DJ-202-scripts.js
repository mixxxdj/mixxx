var DJ202 = {};

/////////////////
// Tweakables. //
/////////////////

DJ202.stripSearchScaling = 50;
DJ202.tempoRange = [0.08, 0.16, 0.5];
DJ202.autoFocusEffects = false;


///////////
// Code. //
///////////

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
            if (value === 127) {
                script.triggerControl(group, isShifted ? 'ScrollUp' : 'MoveUp');
            } else if (value === 1) {
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

    this.paramPlusMinus = function (channel, control, value, status, group) {
        var isPlus = control % 2 == 0;

        this.paramButtonsActive[isPlus ? 0 : 1] = Boolean(value);

        if (!value) {
            return
        }

        if (this.paramButtonsActive.every(Boolean)) {
            script.triggerControl(group, 'reset_key');
            return;
        }

        if (this.keylock.is_held) {
            var adjust = engine.getValue(group, 'pitch_adjust');
            var new_adjust = isPlus ? Math.min(7, adjust + 1) : Math.max(-7, adjust - 1);
            engine.setValue(group, 'pitch_adjust', new_adjust);
            return;
        }

        var beatjumpSize = engine.getValue(group, 'beatjump_size');
        var beatloopSize = engine.getValue(group, 'beatloop_size');

        switch (control) {
        case 0x41:                                                 // Loop mode.
        case 0x42:
            engine.setValue(group, 'loop_move', isPlus ? beatjumpSize : -beatjumpSize);
            break;
        case 0x43:                                              // Hot-Cue mode.
        case 0x44:
            script.triggerControl(group, isPlus ? 'beatjump_forward' : 'beatjump_backward');
            break;
        case 0x49:                                       // Loop mode (shifted).
        case 0x4A:
            engine.setValue(group, 'beatloop_size', isPlus ? beatloopSize*2 : beatloopSize/2);
            break;
        case 0x4B:                                    // Hot-Cue mode (shifted).
        case 0x4C:
            engine.setValue(group, 'beatjump_size', isPlus ? beatjumpSize*2 : beatjumpSize/2);
            break;
        }
    };

    this.paramButtonsActive = [false, false];

    this.keylock = new components.Button({
        midi: [0x90 + offset, 0x0D],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
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
                // The DJ-202 disables the keylock LED when the button is
                // pressed shifted. Restore the LED when shift is released.
                this.trigger();
            };
            this.inKey = 'keylock';
            this.outKey = 'keylock';
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
            // Scale to interval [0,1].
            newValue = newValue / 0xff;
            // Since ‘playposition’ is normalized to unity, we need to scale by
            // song duration in order for the jog wheel to cover the same amount
            // of time given a constant turning angle.
            var duration = engine.getValue(this.currentDeck, 'duration');
            newValue = newValue / duration;
            var newPos = Math.max(0, oldPos + newValue * DJ202.stripSearchScaling);
            engine.setValue(this.currentDeck, 'playposition', newPos);
        } else {
            engine.setValue(this.currentDeck, 'jog', newValue); // Pitch bend
        }
    }
    
    
    
    // ========================== PERFORMANCE PADS ==============================

    this.setPadState = function (channel, control, value, status, group) {
        switch (value) {
        case 0x13:              // Loop mode button.
            if (this.padState == 'loop') {
                var isRolling = engine.getValue(group, 'beatlooproll_activate');
                engine.setValue(group, 'beatlooproll_activate', !isRolling);
            } else {
                this.padState = 'loop';
            }
            return
        case 0x3:               // Hot-cue mode button.
            if (this.padState == 'hotcue') {
                var isLooping = engine.getValue(group, 'loop_enabled');
                script.triggerControl(group, isLooping ? 'reloop_toggle' : 'beatloop_activate');
            } else {
                this.padState = 'hotcue';
            }
            return
        }
    };

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
            number: samplerNumber[channel][i-1],
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

    this.cue = new components.CueButton({
        midi: [0x90 + offset, 0x1],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        reverseRollOnShift: true
    });

    this.play = new components.Button({
        midi: [0x90 + offset, 0],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        outKey: 'play_indicator',
        unshift: function () {
            this.inKey = 'play';
            this.input = function (channel, control, value, status, group) {

                if (value) {                                    // Button press.
                    this.longPressStart = new Date();
                    this.longPressTimer = engine.beginTimer(
                        this.longPressTimeout,
                        function () { this.longPressed = true; },
                        true
                    );
                    return;
                }                                       // Else: Button release.

                var isPlaying = engine.getValue(group, 'play');

                // Normalize ‘isPlaying’ – we consider the braking state
                // equivalent to being stopped, so that pressing play again can
                // trigger a soft-startup even before the brake is complete.
                if (this.isBraking !== undefined) {
                    isPlaying = isPlaying && !this.isBraking
                }

                if (this.longPressed) {             // Release after long press.
                    var deck = script.deckFromGroup(group);
                    var pressDuration = new Date() - this.longPressStart;
                    if (isPlaying && !this.isBraking) {
                        engine.brake(deck, true, 1000 / pressDuration);
                        this.isBraking = true;
                    } else {
                        engine.softStart(deck, true, 1000 / pressDuration);
                        this.isBraking = false;
                    }
                    this.longPressed = false;
                    return;
                }                            // Else: Release after short press.

                this.isBraking = false;
                script.toggleControl(group, 'play', !isPlaying);

                if (this.longPressTimer) {
                    engine.stopTimer(this.longPressTimer);
                    this.longPressTimer = null;
                }
            };
        },
        shift: function () {
            this.inKey = 'reverse';
            this.input = components.Button.prototype.input;
        }
    });

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

    this.setDeck = new components.Button({
        midi: [0x90 + offset, 0x08],
        deck: this,
        input: function (channel, control, value, status, group) {
            var currentDeck = script.deckFromGroup(this.deck.currentDeck);
            var otherDeck = currentDeck == deckNumbers[0] ? deckNumbers[1] : deckNumbers[0];

            otherDeck = '[Channel' + otherDeck + ']';

            if (value) {                                        // Button press.
                this.longPressTimer = engine.beginTimer(
                    this.longPressTimeout,
                    function () { this.isLongPressed = true },
                    true
                );
                this.deck.setCurrentDeck(otherDeck);
                return;
            }                                           // Else: Button release.

            if (this.longPressTimer) {
                engine.stopTimer(this.longPressTimer);
                this.longPressTimer = null;
            }

            // Since we are in the release phase, currentDeck still reflects the
            // switched decks. So if we are now using deck 1/3, we were
            // originally using deck 2/4 and vice versa.
            var deckWasVanilla = currentDeck == deckNumbers[1]

            if (this.isLongPressed) {                     // Release long press.
                this.isLongPressed = false;
                // Return to the original state.
                this.send(deckWasVanilla ? 0 : 0x7f);
                this.deck.setCurrentDeck(otherDeck);
                return;
            }                                      // Else: Release short press.

            // Invert the deck state.
            this.send(deckWasVanilla ? 0x7f : 0);
        }
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

    // FIXME The LED should only be toggled *after* a long press was registered,
    // so one immediately sees that focus has switched without looking at the
    // GUI. If the hardware always flashes immediately upon button-down, one
    // could indicate focus switch by toggling it off for a second before
    // re-enabling it.
    this.EffectButton.prototype = new components.Button({
        unshift: function() {
            this.input = function (channel, control, value, status) {
                if (this.isPress(channel, control, value, status)) {
                    this.isLongPressed = false;
                    this.longPressTimer = engine.beginTimer(this.longPressTimeout, function () {
                        var focusedEffect = engine.getValue(eu.group, 'focused_effect');
                        if (focusedEffect === this.buttonNumber) {
                            engine.setValue(eu.group, 'focused_effect', 0);
                        } else {
                            engine.setValue(eu.group, 'focused_effect', this.buttonNumber);
                        }
                        this.isLongPressed = true;
                    }, true);
                } else {
                    if (!this.isLongPressed) {
                        var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + this.buttonNumber + ']';
                        var wasEnabled = engine.getValue(effectGroup, 'enabled');
                        script.toggleControl(effectGroup, 'enabled');
                        if (!wasEnabled && DJ202.autoFocusEffects) {
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

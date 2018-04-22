var DJ202 = {};

/////////////////
// Tweakables. //
/////////////////

DJ202.stripSearchScaling = 50;
DJ202.tempoRange = [0.08, 0.16, 0.5];
DJ202.autoFocusEffects = false;
DJ202.autoShowFourDecks = false;


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

    engine.makeConnection('[Channel3]', 'track_loaded', DJ202.autoShowDecks);
    engine.makeConnection('[Channel4]', 'track_loaded', DJ202.autoShowDecks);

    if (engine.getValue('[Master]', 'num_samplers') < 16) {
        engine.setValue('[Master]', 'num_samplers', 16);
    }
};

DJ202.autoShowDecks = function (value, group, control) {
    var any_loaded = engine.getValue('[Channel3]', 'track_loaded')
        || engine.getValue('[Channel4]', 'track_loaded')
    if (!DJ202.autoShowFourDecks) {
        return
    }
    engine.setValue('[Master]', 'show_4decks', any_loaded);
}

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

    this.slipModeButton = new components.Button({
        inKey: 'slip_enabled',
        outKey: 'slip_enabled',
        connect: function () {
            var deck = script.deckFromGroup(this.group);
            this.midi = [0x90 + deck - 1, 0x7];
            components.Button.prototype.connect.call(this);
        }
    });

    this.paramPlusMinus = new components.Button({
        deck: this,
        input: function (channel, control, value, status, group) {

            var isPlus = control % 2 == 0;

            this.deck.paramButtonsActive[isPlus ? 0 : 1] = Boolean(value);

            // FIXME: This make the LEDs light up on press, but doesn’t properly
            // connect the output controls, so the buttons won’t light when
            // manipulated from within the GUI.
            var deck = script.deckFromGroup(group);
            midi.sendShortMsg(0x94 + deck - 1, control, value);

            if (!value) {
                return
            }

            if (this.deck.paramButtonsActive.every(Boolean)) {
                script.triggerControl(group, 'reset_key');
                return;
            }

            if (this.deck.keylock.is_held) {
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
        }
    });

    this.paramButtonsActive = [false, false];

    this.keylock = new components.Button({
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        outKey: 'keylock',
        currentRangeIndex: 0,
        unshift: function () {
            this.midi = [0x90 + offset, 0x0D];
            this.trigger();
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
        },
        shift: function () {
            this.midi = [0x90 + offset, 0x0E];
            this.send(0);
            this.inKey = 'rateRange';
            this.type = undefined;
            this.input = function (channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status)) {
                    this.send(0x7f);
                    this.currentRangeIndex++;
                    if (this.currentRangeIndex >= DJ202.tempoRange.length) {
                        this.currentRangeIndex = 0;
                    }
                    this.inSetValue(DJ202.tempoRange[this.currentRangeIndex]);
                    return;
                }
                this.send(0);
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

    for (var i = 1; i <= 8; i++) {
        this.hotcueButton[i] = new components.HotcueButton({
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i,
        });

        this.hotcueButton[i].connect = function () {
            var deck = script.deckFromGroup(this.group);
            this.midi = [0x94 + deck - 1, this.number];
            components.HotcueButton.prototype.connect.call(this);
        }

        this.samplerButton[i] = new components.SamplerButton({
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + offset * 8,
        });

        this.samplerButton[i].send = function (value) {
            var isLeftDeck = this.number <= 8;
            var channel = isLeftDeck ? 0x94 : 0x95;
            this.midi = [channel, 0x20 + this.number - (isLeftDeck ? 0 : 8)];
            components.SamplerButton.prototype.send.call(this, value);
            this.midi = [channel + 2, 0x20 + this.number - (isLeftDeck ? 0 : 8)];
            components.SamplerButton.prototype.send.call(this, value);
        }
    }
    

    for (var i = 1; i <= 4; i++) {
        this.loopButton[i] = new components.Button({
            midi: [0x94 + offset, 0x10 + i],
            inKey: 'beatloop_'+ Math.pow(2,i-1) +'_activate',
        });
    }
    
    this.loopIn = new components.Button({
        midi: [0x94 + offset, 0x15],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        inKey: 'loop_in',
        outKey: 'loop_start_position'
    });
    this.loopOut = new components.Button({
        midi: [0x94 + offset, 0x16],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        inKey: 'loop_out',
        outKey: 'loop_end_position',
    });
    this.loopToggle = new components.Button({
        midi: [0x94 + offset, 0x18],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        inKey: 'reloop_toggle',
        outKey: 'loop_enabled'
    });

    
    // ============================= TRANSPORT ==================================

    this.cue = new components.CueButton({
        midi: [0x90 + offset, 0x1],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        reverseRollOnShift: true,
        input: function (channel, control, value, status, group) {
            components.CueButton.prototype.input.call(this, channel, control, value, status, group);
            if (value) {
                return
            }
            var state = engine.getValue(group, 'cue_indicator');
            if (state) {
                this.trigger();
            }
        }
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
            this.input = function (channel, control, value, status, group) {
                components.Button.prototype.input.apply(this, arguments);
                if(!value) {
                    this.trigger();
                }

            };
        }
    });

    this.sync = new DJ202.SyncButton({group: this.currentDeck});

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
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
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

DJ202.Deck.prototype = Object.create(components.Deck.prototype);


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
        this.effectMode.shift();
        this.knob.shift();
    };

    this.unshift = function () {
        this.button.forEach(function (button) {
            button.unshift();
        });
        this.effectMode.unshift();
        this.knob.unshift();
    };

    this.button = [];
    for (var i = 1; i <= 3; i++) {
        this.button[i] = new DJ202.EffectButton(unitNumber, i);
        var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + i + ']';
        engine.softTakeover(effectGroup, 'meta', true);
        engine.softTakeover(eu.group, 'mix', true);
    }

    this.effectMode = new DJ202.EffectModeButton(unitNumber);

    this.knob = new components.Pot({
        unshift: function () {
            this.input = function (channel, control, value, status) {
                value = (this.MSB << 7) + value;

                var focusedEffect = engine.getValue(eu.group, 'focused_effect');
                if (focusedEffect !== 0) {
                    var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + focusedEffect + ']';
                    engine.setParameter(effectGroup, 'meta', value / this.max);
                }
                engine.softTakeoverIgnoreNextValue(eu.group, 'mix');
            };
        },
        shift: function() {
            this.input = function (channel, control, value, status) {
                engine.setParameter(eu.group, 'mix', value / 0x7f);
                var focusedEffect = engine.getValue(eu.group, 'focused_effect');
                var effectGroup = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + focusedEffect + ']';
                engine.softTakeoverIgnoreNextValue(effectGroup, 'meta');
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


////////////////////////
// Custom components. //
////////////////////////
DJ202.FlashingButton = function () {
    components.Button.call(this);
    this.flashFreq = 50;
};

DJ202.FlashingButton.prototype = Object.create(components.Button.prototype);

DJ202.FlashingButton.prototype.flash = function (cycles) {
    if (cycles == 0) {
        // Reset to correct value after flashing phase ends.
        this.trigger();
        return
    }

    if (cycles === undefined) {
        cycles = 10;
    }

    var value = cycles % 2 == 0 ? 0x7f : 0;
    this.send(value);

    engine.beginTimer(
        this.flashFreq,
        function () {
            var value = value ? 0 : 0x7f;
            this.send(value);
            this.flash(cycles - 1);
        },
        true
    );
};

DJ202.EffectButton = function (effectUnitNumber, effectNumber) {
    this.effectUnitNumber = effectUnitNumber;
    this.effectNumber = effectNumber;
    this.effectUnitGroup = '[EffectRack1_EffectUnit' + effectUnitNumber + ']';
    this.group = '[EffectRack1_EffectUnit' + effectUnitNumber + '_Effect' + effectNumber + ']';
    this.midi = [0x98 + effectUnitNumber - 1, 0x00 + effectNumber - 1];
    this.sendShifted = true;
    this.shiftOffset = 0x0B;
    this.outKey = 'enabled';
    DJ202.FlashingButton.call(this);
};

DJ202.EffectButton.prototype = Object.create(DJ202.FlashingButton.prototype);

DJ202.EffectButton.prototype.unshift = function() {
    this.input = function (channel, control, value, status) {
        if (this.isPress(channel, control, value, status)) {
            this.isLongPressed = false;
            this.longPressTimer = engine.beginTimer(
                this.longPressTimeout,
                function () {
                    engine.setValue(
                        this.effectUnitGroup,
                        'focused_effect',
                        this.effectNumber
                    );
                    this.isLongPressed = true;
                    this.flash();
                },
                true
            );
            return;
        }                                            // Else: on button release.

        if (this.longPressTimer) {
            engine.stopTimer(this.longPressTimer);
            this.longPressTimer = null;
        }

        // Work-around the indicator LED self-disabling itself on release.
        this.trigger();

        if (!this.isLongPressed) {                  // Release after long press.
            var wasEnabled = engine.getValue(this.group, 'enabled');
            script.toggleControl(this.group, 'enabled');
            if (!wasEnabled && DJ202.autoFocusEffects) {
                engine.setValue(this.effectUnitGroup, 'focused_effect', this.effectNumber);
                this.flash();
            }
            return;
        }                                    // Else: release after short press.

        this.isLongPressed = false;
    }
};

DJ202.EffectButton.prototype.shift = function () {
    this.input = function (channel, control, value, status) {
        // Work-around the indicator LED self-disabling itself on release.
        if (!value) {
            this.trigger();
        }
    };
};

DJ202.EffectModeButton = function (effectUnitNumber) {
    this.effectUnitNumber = effectUnitNumber;
    this.group = '[EffectRack1_EffectUnit' + effectUnitNumber + ']';
    this.outKey = 'group_[Headphone]_enable';
    this.inKey = this.outKey;
    this.midi = [0x98 + effectUnitNumber - 1, 0x04];
    DJ202.FlashingButton.call(this);
};

DJ202.EffectModeButton.prototype = Object.create(DJ202.FlashingButton.prototype);

DJ202.EffectModeButton.prototype.input = function (channel, control, value, status) {

    if (value) {                                                // Button press.
        this.isLongPressed = false;
        this.longPressTimer = engine.beginTimer(
            this.longPressTimeout,
            function () {
                this.isLongPressed = true;
                this.longPressTimer = null;
            },
            true
        );
        return;
    }                                                   // Else: Button release.

    // Work-around the indicator LED self-disabling itself on release.
    this.trigger();

    if (this.longPressTimer) {
        engine.stopTimer(this.longPressTimer)
        this.longPressTimer = null;
    }

    if (this.isLongPressed) {                       // Release after long press.
        script.toggleControl(this.group, this.outKey);
        return
    }                                        // Else: Release after short press.

    var focusedEffect = engine.getValue(this.group, 'focused_effect');
    if (!focusedEffect) {
        return
    }

    var effectGroup = '[EffectRack1_EffectUnit' + this.effectUnitNumber + '_Effect' + focusedEffect +']';
    engine.setValue(effectGroup, 'effect_selector',  this.shifted ? -1 : 1);
};

DJ202.EffectModeButton.prototype.shift = function () {
    this.shifted = true;
}

DJ202.EffectModeButton.prototype.unshift = function () {
    this.shifted = false;
}

DJ202.SyncButton = function (options) {
    components.SyncButton.call(this, options);
    this.doubleTapTimeout = 500;
};

DJ202.SyncButton.prototype = Object.create(components.SyncButton.prototype);

DJ202.SyncButton.prototype.connect = function () {
    this.connections = [
        engine.makeConnection(this.group, 'sync_enabled', this.output),
        engine.makeConnection(this.group, 'quantize', this.output)
    ];
    this.deck = script.deckFromGroup(this.group);
    this.midi_enable = [0x90 + this.deck - 1, 0x02];
    this.midi_disable = [0x90 + this.deck - 1, 0x03];
};

DJ202.SyncButton.prototype.send = function (value) {
    var midi_ = value ? this.midi_enable : this.midi_disable;
    midi.sendShortMsg(midi_[0], midi_[1], 0x7f);
};

DJ202.SyncButton.prototype.output = function (value, group, control) {
    // Multiplex between several keys without forcing a reconnect.
    if (control != this.outKey) {
        return
    }
    this.send(value);
}

DJ202.SyncButton.prototype.unshift = function () {
    this.inKey = 'sync_enabled';
    this.outKey = 'sync_enabled';
    this.trigger();
    this.input = function (channel, control, value, status, group) {
        if (this.isPress(channel, control, value, status)) {
            if (this.isDoubleTap) {                               // Double tap.
                var fileBPM = engine.getValue(this.group, 'file_bpm');
                engine.setValue(this.group, 'bpm', fileBPM);
                return
            }                                               // Else: Single tap.

            var syncEnabled = engine.getValue(this.group, 'sync_enabled');

            if (!syncEnabled) {                // Single tap when sync disabled.
                engine.setValue(this.group, 'beatsync', 1);
                this.longPressTimer = engine.beginTimer(
                    this.longPressTimeout,
                    function () {
                        engine.setValue(this.group, 'sync_enabled', 1);
                        this.longPressTimer = null;
                    },
                    true
                );
                // For the next call.
                this.isDoubleTap = true;
                this.doubleTapTimer = engine.beginTimer(
                    this.doubleTapTimeout,
                    function () { this.isDoubleTap = false },
                    true
                );
                return
            }                                          // Else: Sync is enabled.

            engine.setValue(this.group, 'sync_enabled', 0);
            return;
        }                                            // Else: On button release.

        if (this.longPressTimer) {
            engine.stopTimer(this.longPressTimer);
            this.longPressTimer = null;
        };

        // Work-around button LED disabling itself on release.
        this.trigger();
    };
};


DJ202.SyncButton.prototype.shift = function () {
    this.outKey = 'quantize';
    this.inKey = 'quantize';
    this.trigger();
    this.input = function (channel, control, value, status, group) {
        if (value) {
            this.inToggle();
        } else {
            // Work-around LED self-disable issue.
            this.trigger();
        }
    };
};

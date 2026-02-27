// eslint-disable-next-line no-var
var DJ202 = {};

/////////////////
// Tweakables. //
/////////////////

DJ202.stripSearchScaling = 0.15;
DJ202.tempoRange = [0.08, 0.16, 0.5];
DJ202.cueLoopLength = 2;
DJ202.slicerBeatsWindow = 8;
DJ202.autoFocusEffects = false;
DJ202.autoShowFourDecks = false;
DJ202.bindSamplerControls = false;
DJ202.vinylModeEnabledOnStart = true;


///////////
// Code. //
///////////

DJ202.PadMode = {
    HOTCUE: 1,
    CUELOOP: 2,
    PITCHPLAY: 3,

    LOOP: 4,
    ROLL: 5,

    SEQUENCER: 6,
    // mode: Use Pads 1-8 to play your samples in time with the sequencer.
    INSTRUMENTRECORD: 7,
    PATTERN: 8,

    SAMPLER: 9,
    SLICERLOOP: 10,
    SLICER: 11,
};

DJ202.pitchplayRange = {
    UP: 0,
    MID: 1,
    DOWN: 2,
};

DJ202.init = function() {
    DJ202.shiftButton = function(_channel, _control, value, _status, _group) {
        DJ202.deck.concat(DJ202.effectUnit, DJ202.sampler, DJ202.browseEncoder)
            .forEach(value ? function(module) {
                module.shift();
            } :
                function(module) {
                    module.unshift();
                });
    };

    DJ202.leftDeck = new DJ202.Deck([1, 3], 0);
    DJ202.rightDeck = new DJ202.Deck([2, 4], 1);
    DJ202.deck = [DJ202.leftDeck, DJ202.rightDeck];

    DJ202.sampler = new DJ202.Sampler();
    DJ202.sampler.reconnectComponents();

    DJ202.effectUnit = [];
    DJ202.effectUnit[1] = new DJ202.EffectUnit(1);
    DJ202.effectUnit[2] = new DJ202.EffectUnit(2);

    engine.makeConnection("[Channel3]", "track_loaded", DJ202.autoShowDecks);
    engine.makeConnection("[Channel4]", "track_loaded", DJ202.autoShowDecks);

    if (engine.getValue("[Master]", "num_samplers") < 16) {
        engine.setValue("[Master]", "num_samplers", 16);
    }

    // request initial state
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7], 6);
    // unlock pad layers
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x01, 0xF7], 6);

    // engine.beginTimer(500, function() {
    //   // Keep sending this message to enable performance pad LEDs
    //   // on the DJ 505, not sure this is neerded for the 202
    //   midi.sendShortMsg(0xBF, 0x64, 0x00);
    // });

    DJ202.leftDeck.setCurrentDeck("[Channel1]");
    DJ202.rightDeck.setCurrentDeck("[Channel2]");

    // Initialize vinyl led state
    DJ202.leftDeck.setVinylModeLED();
    DJ202.rightDeck.setVinylModeLED();
};

DJ202.autoShowDecks = function(_value, _group, _control) {
    const anyLoaded = engine.getValue("[Channel3]", "track_loaded") ||
    engine.getValue("[Channel4]", "track_loaded");
    if (!DJ202.autoShowFourDecks) {
        return;
    }
    engine.setValue("[Master]", "show_4decks", anyLoaded);
};

DJ202.shutdown = function() {};

DJ202.sortLibrary = function(_channel, control, value, _status, _group) {
    if (value === 0) {
        return;
    }

    let sortColumn;
    switch (control) {
    case 0x12: // SONG
        sortColumn = script.LIBRARY_COLUMNS.ARTIST;
        break;
    case 0x13: // BPM
        sortColumn = script.LIBRARY_COLUMNS.BPM;
        break;
    default:
    // unknown sort column
        return;
    }
    engine.setValue("[Library]", "sort_column_toggle", sortColumn);
};

DJ202.browseEncoder = new components.Encoder({
    longPressTimer: 0,
    longPressTimeout: 250,
    previewSeekEnabled: false,
    previewSeekHappened: false,
    unshift: function() {
        this.onKnobEvent = function(rotateValue) {
            if (rotateValue !== 0) {
                if (this.previewSeekEnabled) {
                    const oldPos = engine.getValue("[PreviewDeck1]", "playposition");
                    const newPos = Math.max(0, oldPos + (0.05 * rotateValue));
                    engine.setValue("[PreviewDeck1]", "playposition", newPos);
                } else {
                    engine.setValue("[Playlist]", "SelectTrackKnob", rotateValue);
                }
            }
        };
        this.onButtonEvent = function(value) {
            if (value) {
                this.isLongPressed = false;
                this.longPressTimer = engine.beginTimer(
                    this.longPressTimeout,
                    function() {
                        this.isLongPressed = true;
                    },
                    true
                );

                this.previewStarted = false;
                if (!engine.getValue("[PreviewDeck1]", "play")) {
                    engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
                    this.previewStarted = true;
                }
                // Track in PreviewDeck1 is playing, either the user
                // wants to stop the track or seek in it
                this.previewSeekEnabled = true;
            } else {
                if (this.longPressTimer !== 0) {
                    engine.stopTimer(this.longPressTimer);
                    this.longPressTimer = 0;
                }

                if (!this.isLongPressed && !this.previewStarted && engine.getValue("[PreviewDeck1]", "play")) {
                    script.triggerControl("[PreviewDeck1]", "stop");
                }
                this.previewSeekEnabled = false;
                this.previewStarted = false;
            }
        };
    },
    shift: function() {
        this.onKnobEvent = function(rotateValue) {
            if (rotateValue !== 0) {
                engine.setValue("[Playlist]", "SelectPlaylist", rotateValue);
            }
        };
        this.onButtonEvent = function(value) {
            if (value) {
                script.triggerControl("[Playlist]", "ToggleSelectedSidebarItem");
            }
        };
    },
    input: function(_channel, control, value, status, _group) {
        switch (status) {
        case 0xBF: // Rotate.
            var rotateValue = (value === 127) ? -1 : ((value === 1) ? 1 : 0);
            this.onKnobEvent(rotateValue);
            break;
        case 0x9F: // Push.
            this.onButtonEvent(value);
        }
    }
});

DJ202.crossfader = new components.Pot({
    midi: [0xBF, 0x08],
    group: "[Master]",
    inKey: "crossfader",
    input: function(_channel, _control, value, _status, group) {
    // We need a weird max. for the crossfader to make it cut cleanly.
    // However, components.js resets max. to 0x3fff when the first value is
    // received. Hence, we need to set max. here instead of within the
    // constructor.
        this.max = (0x7f << 7) + 0x70;
        value = (this.MSB << 7) + value;
        const newValue = this.inValueScale(value);
        engine.setParameter(group, this.inKey, newValue);
    }
});

DJ202.Deck = function(deckNumbers, offset) {
    components.Deck.call(this, deckNumbers);

    this.isVinylMode = DJ202.vinylModeEnabledOnStart;

    this.loadTrack = new components.Button({
        midi: [0x9F, 0x02 + offset],
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
        },
        shift: function() {
            this.inKey = "eject";
        },
    });

    this.setVinylModeLED = function() {
        midi.sendShortMsg(0x90 + offset, 0x0F, this.isVinylMode ? 0x7F: 0x00);
    };

    this.slipModeButton = new DJ202.SlipModeButton();
    this.vinylModeButton = function(_channel, _control, value, _status, _group) {
        if (value) { this.isVinylMode = !this.isVinylMode; }
        this.setVinylModeLED();
    };


    engine.setValue(this.currentDeck, "rate_dir", -1);
    this.tempoFader = new components.Pot({
        midi: [0xB0 + offset, 0x09],
        unshift: function() {
            this.inKey = "rate";
            this.inSetParameter = components.Pot.prototype.inSetParameter;
            engine.softTakeoverIgnoreNextValue(this.group, "pitch");
        },
        shift: function() {
            this.inKey = "pitch";
            this.inSetParameter = function(value) {
                // Scale to interval ]-7…7[; invert direction as per controller
                // labeling.
                value = 14 * value - 7;
                value *= -1;
                components.Pot.prototype.inSetValue.call(this, value);
            };
            engine.softTakeoverIgnoreNextValue(this.group, "rate");
        }
    });

    // ============================= JOG WHEELS =================================

    this.wheelTouch = function(_channel, _control, value, _status, _group) {
        if (value === 0x7F && !this.isShifted && this.isVinylMode) {
            const alpha = 1.0 / 8;
            const beta = alpha / 32;
            engine.scratchEnable(script.deckFromGroup(this.currentDeck), 512, 45,
                alpha, beta);
        } else { // If button up
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    };

    this.wheelTurn = function(_channel, _control, value, _status, _group) {
    // When the jog wheel is turned in clockwise direction, value is
    // greater than 64 (= 0x40). If it's turned in counter-clockwise
    // direction, the value is smaller than 64.
        const newValue = value - 64;
        const deck = script.deckFromGroup(this.currentDeck);
        if (engine.isScratching(deck)) {
            engine.scratchTick(deck, newValue); // Scratch!
        } else if (this.isShifted) {
            const oldPos = engine.getValue(this.currentDeck, "playposition");
            // Since ‘playposition’ is normalized to unity, we need to scale by
            // song duration in order for the jog wheel to cover the same amount
            // of time given a constant turning angle.
            const duration = engine.getValue(this.currentDeck, "duration");
            const newPos = Math.max(0, oldPos + (newValue * DJ202.stripSearchScaling / duration));
            engine.setValue(this.currentDeck, "playposition", newPos); // Strip search
        } else {
            engine.setValue(this.currentDeck, "jog", newValue); // Pitch bend
        }
    };

    // ========================== PERFORMANCE PADS ==============================

    this.padSection = new DJ202.PadSection(this);
    this.keylock = new DJ202.KeylockButton(this.padSection.paramPlusMinus);

    // ============================= TRANSPORT ==================================

    this.cue = new components.CueButton({
        midi: [0x90 + offset, 0x1],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        reverseRollOnShift: true,
        input: function(channel, control, value, status, group) {
            components.CueButton.prototype.input.call(this, channel, control, value,
                status, group);
            if (value) {
                return;
            }
            const state = engine.getValue(group, "cue_indicator");
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
        outKey: "play_indicator",
        unshift: function() {
            this.inKey = "play";
            this.input = function(_channel, _control, value, _status, group) {
                if (value) { // Button press.
                    this.longPressStart = new Date();
                    this.longPressTimer =
            engine.beginTimer(this.longPressTimeout,
                function() {
                    this.longPressed = true;
                }, true);
                    return;
                } // Else: Button release.

                let isPlaying = engine.getValue(group, "play");

                // Normalize ‘isPlaying’ – we consider the braking state
                // equivalent to being stopped, so that pressing play again can
                // trigger a soft-startup even before the brake is complete.
                if (this.isBraking !== undefined) {
                    isPlaying = isPlaying && !this.isBraking;
                }

                if (this.longPressed) { // Release after long press.
                    const deck = script.deckFromGroup(group);
                    const pressDuration = new Date() - this.longPressStart;
                    if (isPlaying && !this.isBraking) {
                        engine.brake(deck, true, 1000 / pressDuration);
                        this.isBraking = true;
                    } else {
                        engine.softStart(deck, true, 1000 / pressDuration);
                        this.isBraking = false;
                    }
                    this.longPressed = false;
                    return;
                } // Else: Release after short press.

                this.isBraking = false;
                script.toggleControl(group, "play", !isPlaying);

                if (this.longPressTimer) {
                    engine.stopTimer(this.longPressTimer);
                    this.longPressTimer = null;
                }
            };
        },
        shift: function() {
            this.inKey = "reverse";
            this.input = function(_channel, _control, value, _status, _group) {
                components.Button.prototype.input.apply(this, arguments);
                if (!value) {
                    this.trigger();
                }
            };
        }
    });

    this.sync = new DJ202.SyncButton({
        group: this.currentDeck
    });

    // =============================== MIXER ====================================

    this.potInput = function(_channel, _control, value, _status, group) {
        value = (this.MSB << 7) + value;
        const newValue = this.inValueScale(value);
        engine.setParameter(group, this.inKey, newValue);
    };

    this.pregain = new components.Pot({
        midi: [0xB0 + offset, 0x16],
        inKey: "pregain",
        input: this.potInput
    });

    this.eqKnob = [];
    for (let k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            midi: [0xB0 + offset, 0x20 - k],
            group: `[EqualizerRack1_${  this.currentDeck  }_Effect1]`,
            inKey: `parameter${  k}`,
            input: this.potInput
        });
    }

    this.filter = new components.Pot({
        midi: [0xB0 + offset, 0x1A],
        group: `[QuickEffectRack1_${  this.currentDeck  }]`,
        inKey: "super1",
        input: this.potInput
    });

    this.pfl = new components.Button({
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        midi: [0x90 + offset, 0x1B],
        type: components.Button.prototype.types.toggle,
        inKey: "pfl",
        outKey: "pfl",
    });

    this.tapBPM = new components.Button({
        input: function(_channel, _control, value, _status, group) {
            if (value === 127) {
                script.triggerControl(group, "beats_translate_curpos");
                bpm.tapButton(script.deckFromGroup(group));
                this.longPressTimer =
          engine.beginTimer(this.longPressTimeout, function() {
              script.triggerControl(group, "beats_translate_match_alignment");
          }, true);
            } else {
                engine.stopTimer(this.longPressTimer);
            }
        }
    });

    this.volume = new components.Pot({
        midi: [0xB0 + offset, 0x1C],
        inKey: "volume",
        input: this.potInput
    });

    this.setDeck = new components.Button({
        midi: [0x90 + offset, 0x08],
        deck: this,
        input: function(_channel, _control, value, _status, _group) {
            const currentDeck = script.deckFromGroup(this.deck.currentDeck);
            let otherDeck =
        currentDeck === deckNumbers[0] ? deckNumbers[1] : deckNumbers[0];

            otherDeck = `[Channel${  otherDeck  }]`;

            if (value) { // Button press.
                this.longPressTimer =
          engine.beginTimer(this.longPressTimeout,
              function() {
                  this.isLongPressed = true;
              }, true);
                this.deck.setCurrentDeck(otherDeck);
                return;
            } // Else: Button release.

            if (this.longPressTimer) {
                engine.stopTimer(this.longPressTimer);
                this.longPressTimer = null;
            }

            // Since we are in the release phase, currentDeck still reflects the
            // switched decks. So if we are now using deck 1/3, we were
            // originally using deck 2/4 and vice versa.
            const deckWasVanilla = currentDeck === deckNumbers[1];

            if (this.isLongPressed) { // Release long press.
                this.isLongPressed = false;
                // Return to the original state.
                this.send(deckWasVanilla ? 0 : 0x7f);
                this.deck.setCurrentDeck(otherDeck);
                return;
            } // Else: Release short press.
            // Invert the deck state.
            this.send(deckWasVanilla ? 0x7f : 0);
        }
    });

    this.setCurrentDeck = function(deck) {
        components.Deck.prototype.setCurrentDeck.call(this, deck);
        DJ202.effectUnit[offset + 1].focusedDeck = script.deckFromGroup(deck);
        DJ202.effectUnit[offset + 1].reconnect();
    };
};

DJ202.Deck.prototype = Object.create(components.Deck.prototype);

///////////////////////////////////////////////////////////////
//                             FX                            //
///////////////////////////////////////////////////////////////

DJ202.EffectUnit = function(unitNumber) {
    components.ComponentContainer.call(this);

    const eu = this;
    this.unitNumber = unitNumber;
    this.focusedDeck = unitNumber;
    this.group = `[EffectRack1_EffectUnit${  unitNumber  }]`;
    engine.setValue(this.group, "show_focus", 1);

    this.shift = function() {
        this.button.forEach(function(button) {
            button.shift();
        });
        this.effectMode.shift();
        this.knob.shift();
    };

    this.unshift = function() {
        this.button.forEach(function(button) {
            button.unshift();
        });
        this.effectMode.unshift();
        this.knob.unshift();
    };

    this.button = [];
    for (let i = 1; i <= 3; i++) {
        this.button[i] = new DJ202.EffectButton(this, i);
        const effectGroup =
      `[EffectRack1_EffectUnit${  unitNumber  }_Effect${  i  }]`;
        engine.softTakeover(effectGroup, "meta", true);
        engine.softTakeover(eu.group, "mix", true);
    }

    this.effectMode = new DJ202.EffectModeButton(unitNumber);

    this.knob = new components.Pot({
        unshift: function() {
            this.input = function(_channel, _control, value, _status) {
                value = (this.MSB << 7) + value;

                const focusedEffect = engine.getValue(eu.group, "focused_effect");
                if (focusedEffect !== 0) {
                    const effectGroup = `[EffectRack1_EffectUnit${  unitNumber  }_Effect${
                        focusedEffect  }]`;
                    engine.setParameter(effectGroup, "meta", value / this.max);
                }
                engine.softTakeoverIgnoreNextValue(eu.group, "mix");
            };
        },
        shift: function() {
            this.input = function(_channel, _control, value, _status) {
                engine.setParameter(eu.group, "mix", value / 0x7f);
                const focusedEffect = engine.getValue(eu.group, "focused_effect");
                const effectGroup = `[EffectRack1_EffectUnit${  unitNumber  }_Effect${
                    focusedEffect  }]`;
                engine.softTakeoverIgnoreNextValue(effectGroup, "meta");
            };
        }
    });

    this.knobSoftTakeoverHandler = engine.makeConnection(
        eu.group, "focused_effect",
        function(value, _group, _control) {
            if (value === 0) {
                engine.softTakeoverIgnoreNextValue(eu.group, "mix");
            } else {
                const effectGroup =
          `[EffectRack1_EffectUnit${  unitNumber  }_Effect${  value  }]`;
                engine.softTakeoverIgnoreNextValue(effectGroup, "meta");
            }
        });
};

DJ202.EffectUnit.prototype =
  Object.create(components.ComponentContainer.prototype);

DJ202.EffectUnit.prototype.reconnect = function() {
    this.forEachComponent(function(component) {
        component.disconnect();
        component.connect();
    });
};

//////////////////////////////
// Sampler.                 //
//////////////////////////////

DJ202.SamplerButton =
  function() {
      components.SamplerButton.apply(this, arguments);
  };

DJ202.SamplerButton.prototype =
  Object.create(components.SamplerButton.prototype);

DJ202.SamplerButton.prototype.connect = function() {
    const deck = script.deckFromGroup(this.group);
    this.midi = [0x94 + deck - 1, 0x20 + this.number];
    components.SamplerButton.prototype.connect.apply(this, arguments);
};

DJ202.SamplerButton.prototype.send = function(value) {
    const isLeftDeck = this.number <= 8;
    const channel = isLeftDeck ? 0x94 : 0x95;
    this.midi = [channel, 0x20 + this.number - (isLeftDeck ? 0 : 8)];
    components.SamplerButton.prototype.send.call(this, value);
    this.midi = [channel + 2, 0x20 + this.number - (isLeftDeck ? 0 : 8)];
    components.SamplerButton.prototype.send.call(this, value);
};

DJ202.SamplerButton.prototype.unshift = function() {
    this.input = function(_channel, _control, value, _status, _group) {
        const isLeftDeck = this.number <= 8;
        const padMode = isLeftDeck ? DJ202.leftDeck.padSection.mode :
            DJ202.rightDeck.padSection.mode;
        if (padMode === DJ202.PadMode.SLICER) {
            DJ202.slicer(value, this.number, false);
            return;
        } else if (padMode === DJ202.PadMode.SLICERLOOP) {
            DJ202.slicer(value, this.number, true);
            return;
        }
        components.SamplerButton.prototype.input.apply(this, arguments);
    };
};

/////

DJ202.Sampler = function() {
    components.ComponentContainer.call(this);
    this.syncDeck = -1;
    this.button = [];

    for (let i = 1; i <= 16; i++) {
        this.button[i] = new DJ202.SamplerButton({
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i
        });
    }

    this.level = new components.Pot({
        inValueScale: function(value) {
            // FIXME: The sampler gain knob has a dead zone and appears to
            // scale non-linearly.
            return components.Pot.prototype.inValueScale.call(this, value) * 4;
        },
        input: function(_channel, _control, value, _status, _group) {
            if (!DJ202.bindSamplerControls) {
                return;
            }
            for (let i = 1; i <= 16; i++) {
                const group = `[Sampler${  i  }]`;
                engine.setValue(group, "pregain", this.inValueScale(value));
            }
        }
    });

    this.pfl = new components.Button({
        sampler: this,
        midi: [0x9f, 0x1d],
        connect: function() {
            if (!DJ202.bindSamplerControls) {
                return;
            }
            components.Button.prototype.connect.call(this);
            // Ensure a consistent state between mixxx and device.
            for (let i = 1; i <= 16; i++) {
                const group = `[Sampler${  i  }]`;
                engine.setValue(group, "pfl", false);
            }
            this.send(0);
        },
        input: function(_channel, _control, value, _status, _group) {
            if (!value || !DJ202.bindSamplerControls) {
                return;
            }
            for (let i = 1; i <= 16; i++) {
                const group = `[Sampler${  i  }]`;
                script.toggleControl(group, "pfl");
            }
        }
    });

    const getActiveDeck = function() {
        const deckvolume = new Array(0, 0, 0, 0);
        let volumemax = -1;
        let newdeck = -1;

        // get volume from the decks and check it for use
        for (let z = 0; z <= 3; z++) {
            if (engine.getValue(`[Channel${  z + 1  }]`, "track_loaded") > 0) {
                deckvolume[z] = engine.getValue(`[Channel${  z + 1  }]`, "volume");
                if (deckvolume[z] > volumemax) {
                    volumemax = deckvolume[z];
                    newdeck = z;
                }
            }
        }

        return newdeck;
    };

    this.syncButtonPressed = function(_channel, control, value, _status, _group) {
        if (value !== 0x7F) {
            return;
        }
        const isShifted = (control === 0x55);
        if (isShifted || this.syncDeck >= 0) {
            this.syncDeck = -1;
        } else {
            const deck = getActiveDeck();
            if (deck < 0) {
                return;
            }
            const bpm = engine.getValue(`[Channel${  deck + 1  }]`, "bpm");

            // Minimum BPM is 5.0 (0xEA 0x32 0x00), maximum BPM is 800.0 (0xEA 0x40
            // 0x3e).
            if (!(bpm >= 5 && bpm <= 800)) {
                return;
            }
            const bpmValue = Math.round(bpm * 10);
            midi.sendShortMsg(0xEA, bpmValue & 0x7f, (bpmValue >> 7) & 0x7f);
            this.syncDeck = deck;
        }
    };

    this.startStopButtonPressed = function(_channel, _control, _value, status,
        _group) {
        if (status === 0xFA) {
            this.playbackCounter = 1;
            this.playbackTimer = engine.beginTimer(500, function() {
                midi.sendShortMsg(0xBA, 0x02, this.playbackCounter);
                this.playbackCounter = (this.playbackCounter % 4) + 1;
            });
        } else if (status === 0xFC) {
            if (this.playbackTimer) {
                engine.stopTimer(this.playbackTimer);
            }
        }
    };

    this.customSamplePlayback = function(_channel, _control, value, _status, group) {
        if (value) {
            engine.setValue(group, "cue_gotoandplay", 1);
        }
    };
};

DJ202.Sampler.prototype =
  Object.create(components.ComponentContainer.prototype);

////////////////////////
// Custom components. //
////////////////////////

DJ202.FlashingButton = function() {
    components.Button.call(this);
    // used for flashing button every 50ms
    this.flashFreq = 50;
};

DJ202.FlashingButton.prototype = Object.create(components.Button.prototype);

DJ202.FlashingButton.prototype.flash = function(cycles) {
    if (cycles === 0) {
    // Reset to correct value after flashing phase ends.
        this.trigger();
        return;
    }

    if (cycles === undefined) {
        cycles = 10;
    }

    const value = cycles % 2 === 0 ? 0x7f : 0;
    this.send(value);

    engine.beginTimer(this.flashFreq, function() {
        var value = value ? 0 : 0x7f;
        this.send(value);
        this.flash(cycles - 1);
    }, true);
};

DJ202.EffectButton = function(effectUnit, effectNumber) {
    this.effectUnit = effectUnit;
    this.effectUnitNumber = effectUnit.unitNumber;
    this.effectNumber = effectNumber;
    this.effectUnitGroup =
    `[EffectRack1_EffectUnit${  this.effectUnitNumber  }]`;
    this.effectGroup = (`[EffectRack1_EffectUnit${  this.effectUnitNumber
    }_Effect${  this.effectNumber  }]`);
    this.midi = [0x98 + this.effectUnitNumber - 1, 0x00 + effectNumber - 1];
    this.sendShifted = true;
    this.shiftOffset = 0x0B;
    this.outKey = "enabled";
    DJ202.FlashingButton.call(this);
};

DJ202.EffectButton.prototype = Object.create(DJ202.FlashingButton.prototype);

DJ202.EffectButton.prototype.connect = function() {
    if (this.effectNumber === 3) {
        this.routingGroup = this.effectUnitGroup;
    } else {
        this.routingGroup = `[EffectRack1_EffectUnit${  this.effectNumber  }]`;
    }

    const deck = this.effectUnit.focusedDeck;

    this.routingControl =
    (`group_${
        this.effectNumber === 3 ? "[Headphone]" : `[Channel${  deck  }]`
    }_enable`);

    this.connections = [
        engine.makeConnection(this.effectGroup, "enabled", this.output),
        engine.makeConnection(this.routingGroup, this.routingControl, this.output)
    ];
};

DJ202.EffectButton.prototype.output = function(value, group, control) {
    if (control !== this.outKey) {
        return;
    }
    DJ202.FlashingButton.prototype.output.apply(this, arguments);
};

DJ202.EffectButton.prototype.unshift = function() {
    this.group = this.effectGroup;
    this.outKey = "enabled";
    this.inKey = this.outKey;
    this.trigger();
    this.input = function(channel, control, value, status) {
        if (this.isPress(channel, control, value, status)) {
            this.isLongPressed = false;
            this.longPressTimer =
        engine.beginTimer(this.longPressTimeout, function() {
            engine.setValue(this.effectUnitGroup, "focused_effect",
                this.effectNumber);
            this.isLongPressed = true;
        }, true);
            return;
        } // Else: on button release.

        if (this.longPressTimer) {
            engine.stopTimer(this.longPressTimer);
            this.longPressTimer = null;
        }

        // Work-around the indicator LED self-disabling itself on release.
        this.trigger();

        if (!this.isLongPressed) { // Release after long press.
            const wasEnabled = engine.getValue(this.group, "enabled");
            script.toggleControl(this.group, "enabled");
            if (!wasEnabled && DJ202.autoFocusEffects) {
                engine.setValue(this.effectUnitGroup, "focused_effect",
                    this.effectNumber);
                this.flash();
            }
            return;
        } // Else: release after short press.

        this.isLongPressed = false;
    };
};

DJ202.EffectButton.prototype.shift = function() {
    this.group = this.routingGroup;
    this.outKey = this.routingControl;
    this.inKey = this.outKey;
    this.trigger();
    this.input = function(_channel, _control, value, _status) {
        if (value) {
            this.inToggle();
        } else {
            // Work-around the indicator LED self-disabling itself on release.
            this.trigger();
        }
    };
};

DJ202.EffectModeButton = function(effectUnitNumber) {
    this.effectUnitNumber = effectUnitNumber;
    this.group = `[EffectRack1_EffectUnit${  effectUnitNumber  }]`;
    this.midi = [0x98 + effectUnitNumber - 1, 0x04];
    DJ202.FlashingButton.call(this);
};

DJ202.EffectModeButton.prototype =
  Object.create(DJ202.FlashingButton.prototype);

DJ202.EffectModeButton.prototype.input = function(_channel, _control, value,
    _status) {
    if (value) { // Button press.
        return;
    } // Else: Button release.

    // Work-around the indicator LED self-disabling itself on release.
    this.trigger();

    const focusedEffect = engine.getValue(this.group, "focused_effect");
    if (!focusedEffect) {
        return;
    }

    const effectGroup = `[EffectRack1_EffectUnit${  this.effectUnitNumber
    }_Effect${  focusedEffect  }]`;
    engine.setValue(effectGroup, "effect_selector", this.shifted ? -1 : 1);
};

DJ202.EffectModeButton.prototype.shift =
  function() {
      this.shifted = true;
  };

DJ202.EffectModeButton.prototype.unshift =
  function() {
      this.shifted = false;
  };

DJ202.SyncButton = function(options) {
    components.SyncButton.call(this, options);
    this.doubleTapTimeout = 500;
};

DJ202.SyncButton.prototype = Object.create(components.SyncButton.prototype);

DJ202.SyncButton.prototype.connect = function() {
    this.connections = [
        engine.makeConnection(this.group, "sync_enabled", this.output),
        engine.makeConnection(this.group, "quantize", this.output)
    ];
    this.deck = script.deckFromGroup(this.group);
    this.midiEnable = [0x90 + this.deck - 1, 0x02];
    this.midiDisable = [0x90 + this.deck - 1, 0x03];
};

DJ202.SyncButton.prototype.send = function(value) {
    const midi_ = value ? this.midiEnable : this.midiDisable;
    midi.sendShortMsg(midi_[0], midi_[1], 0x7f);
};

DJ202.SyncButton.prototype.output = function(value, _group, control) {
    // Multiplex between several keys without forcing a reconnect.
    if (control !== this.outKey) {
        return;
    }
    this.send(value);
};

DJ202.SyncButton.prototype.unshift = function() {
    this.inKey = "sync_enabled";
    this.outKey = "sync_enabled";
    this.trigger();
    this.input = function(channel, control, value, status, _group) {
        if (this.isPress(channel, control, value, status)) {
            if (this.isDoubleTap) { // Double tap.
                const fileBPM = engine.getValue(this.group, "file_bpm");
                engine.setValue(this.group, "bpm", fileBPM);
                return;
            } // Else: Single tap.

            const syncEnabled = engine.getValue(this.group, "sync_enabled");

            if (!syncEnabled) { // Single tap when sync disabled.
                engine.setValue(this.group, "beatsync", 1);
                this.longPressTimer =
          engine.beginTimer(this.longPressTimeout, function() {
              engine.setValue(this.group, "sync_enabled", 1);
              this.longPressTimer = null;
          }, true);
                // For the next call.
                this.isDoubleTap = true;
                this.doubleTapTimer =
          engine.beginTimer(this.doubleTapTimeout,
              function() {
                  this.isDoubleTap = false;
              }, true);
                return;
            } // Else: Sync is enabled.

            engine.setValue(this.group, "sync_enabled", 0);
            return;
        } // Else: On button release.

        if (this.longPressTimer) {
            engine.stopTimer(this.longPressTimer);
            this.longPressTimer = null;
        }

        // Work-around button LED disabling itself on release.
        this.trigger();
    };
};

DJ202.SyncButton.prototype.shift = function() {
    this.outKey = "quantize";
    this.inKey = "quantize";
    this.trigger();
    this.input = function(_channel, _control, value, _status, _group) {
        if (value) {
            this.inToggle();
        } else {
            // Work-around LED self-disable issue.
            this.trigger();
        }
    };
};

DJ202.HotcueButton = function() {
    components.HotcueButton.apply(this, arguments);
    this.sendShifted = true;
    this.shiftControl = true;
    this.shiftOffset = 8;
};

DJ202.HotcueButton.prototype = Object.create(components.HotcueButton.prototype);

DJ202.HotcueButton.prototype.connect = function() {
    const deck = script.deckFromGroup(this.group);
    this.midi = [0x94 + deck - 1, this.number];
    components.HotcueButton.prototype.connect.call(this);
};

DJ202.HotcueButton.prototype.unshift = function() {
    this.inKey = `hotcue_${  this.number  }_activate`;
    this.input = function(_channel, _control, value, _status, group) {
        if (this.pad.mode === DJ202.PadMode.PITCHPLAY) {
            if (value > 0 && this.pad.pitchplayCue > 0) {
                let pitchAdjustment = 0;
                switch (this.pad.pitchplayRange) {
                case DJ202.pitchplayRange.UP:
                    pitchAdjustment = this.number + ((this.number <= 4) ? 4 : -5);
                    break;
                case DJ202.pitchplayRange.MID:
                    pitchAdjustment = this.number - ((this.number <= 4) ? 1 : 9);
                    break;
                case DJ202.pitchplayRange.DOWN:
                    pitchAdjustment = this.number - ((this.number <= 4) ? 4 : 12);
                }
                engine.setValue(this.group, "pitch_adjust", pitchAdjustment);
                engine.setValue(this.group,
                    `hotcue_${  this.pad.pitchplayCue  }_activate`, 1);
            }
        } else if (this.pad.mode === DJ202.PadMode.CUELOOP) {
            if (engine.getValue(this.group, `hotcue_${  this.number  }_enabled`)) {
                if (value) {
                    // jump to existing cue and loop
                    const startpos =
            engine.getValue(group, `hotcue_${  this.number  }_position`);
                    const loopseconds =
            DJ202.cueLoopLength * (1 / (engine.getValue(group, "bpm") / 60));
                    const loopsamples =
            loopseconds * engine.getValue(group, "track_samplerate") * 2;
                    const endpos = startpos + loopsamples;
                    // disable loop if currently enabled
                    if (engine.getValue(group, "loop_enabled")) {
                        engine.setValue(group, "reloop_toggle", 1);
                    }
                    // set start and endpoints
                    engine.setValue(group, "loop_start_position", startpos);
                    engine.setValue(group, "loop_end_position", endpos);
                    // enable loop
                    engine.setValue(group, "reloop_toggle", 1);
                    engine.setValue(group, "loop_in_goto", 1);
                } else {
                    engine.setValue(group, "reloop_toggle", 1);
                }
            } else {
                // set a new cue point and loop
                engine.setValue(group, `hotcue_${  this.number  }_activate`, 1);
                engine.setValue(group, `hotcue_${  this.number  }_activate`, 0);
                engine.setValue(group, `beatloop_${  DJ202.cueLoopLength  }_activate`,
                    1);
            }
        } else {
            components.HotcueButton.prototype.input.apply(this, arguments);
        }
    };
};

DJ202.HotcueButton.prototype.shift = function() {
    this.input = function(_channel, _control, value, _status, _group) {
        if (this.pad.mode === DJ202.PadMode.PITCHPLAY) {
            this.pad.pitchplayCue = this.number;
            this.pad.setHotcueLED(this.number);
        } else {
            if (!value) {
                return;
            }
            script.triggerControl(this.group, `hotcue_${  this.number  }_clear`);
            if (engine.getValue(this.group, "play")) {
                script.triggerControl(this.group, `hotcue_${  this.number  }_set`);
            }
        }
    };
};

DJ202.LoopButton = function() {
    components.Button.apply(this, arguments);
};

DJ202.LoopButton.prototype = Object.create(components.Button.prototype);

DJ202.LoopButton.prototype.connect = function() {
    const deck = script.deckFromGroup(this.group);
    this.midi = [0x94 + deck - 1, 0x10 + this.number];
    components.Button.prototype.connect.apply(this, arguments);
};

DJ202.LoopButton.prototype.input = function(_channel, _control, value, _status,
    _group) {
    switch (this.pad.mode) {
    case DJ202.PadMode.ROLL:
        engine.setValue(this.group,
            `beatlooproll_${  1 / Math.pow(2, this.number - 1)
            }_activate`,
            value > 0);
        break;
    case DJ202.PadMode.LOOP:
    default:
        engine.setValue(this.group,
            `beatloop_${  Math.pow(2, this.number - 1)  }_activate`,
            value > 0);
        break;
    }
};

DJ202.SlipModeButton = function() {
    components.Button.apply(this, arguments);
    this.inKey = "slip_enabled";
    this.outKey = "slip_enabled";
    this.doubleTapTimeout = 500;
};

DJ202.ManualLoopButton = function() {
    DJ202.LoopButton.apply(this, arguments);
};

DJ202.ManualLoopButton.prototype =
  Object.create(components.Button.prototype);

DJ202.ManualLoopButton.prototype.connect = function() {
    const deck = script.deckFromGroup(this.group);
    this.midi = [0x94 + deck - 1, this.cc];
    components.Button.prototype.connect.call(this);
};

DJ202.SlipModeButton.prototype = Object.create(components.Button.prototype);

DJ202.SlipModeButton.prototype.connect = function() {
    const deck = script.deckFromGroup(this.group);
    this.midi = [0x90 + deck - 1, 0x7];
    components.Button.prototype.connect.call(this);
};

DJ202.SlipModeButton.prototype.input = function(_channel, _control, value, _status,
    _group) {
    if (value) { // Button press.
        this.inSetValue(true);
        return;
    } // Else: button release.

    if (!this.doubleTapped) {
        this.inSetValue(false);
    }

    // Work-around LED disabling itself on release.
    this.trigger();

    this.doubleTapped = true;

    if (this.doubleTapTimer) {
        engine.stopTimer(this.doubleTapTimer);
        this.doubleTapTimer = null;
    }

    this.doubleTapTimer = engine.beginTimer(this.doubleTapTimeout, function() {
        this.doubleTapped = false;
        this.doubleTapTimer = null;
    }, true);
};

DJ202.KeylockButton = function(paramButtons) {
    components.Button.call(this, {
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        outKey: "keylock",
        currentRangeIndex: 0,
        doubleTapTimeout: 500,
        paramPlusMinus: paramButtons
    });
};

DJ202.KeylockButton.prototype = Object.create(components.Button.prototype);

DJ202.KeylockButton.prototype.unshift = function() {
    if (this.deck) {
        this.midi = [0x90 + this.deck - 1, 0x0D];
        this.trigger();
    }
    this.input = function(_channel, _control, value, _status, _group) {
        if (value) { // Button press.

            this.longPressTimer =
        engine.beginTimer(this.longPressTimeout, function() {
            this.paramPlusMinus.songKeyMode(true);
            this.isHeld = true;
        }, true);

            return;
        } // Else: Button release.

        // The DJ-202 disables the keylock LED when the button is pressed
        // shifted. Restore the LED when shift is released.
        this.trigger();

        if (this.longPressTimer) {
            engine.stopTimer(this.longPressTimer);
            this.longPressTimer = null;
        }

        if (this.isHeld) { // Release after hold.
            this.paramPlusMinus.songKeyMode(false);
            this.isHeld = false;
            return;
        } // Else: release after short tap.

        script.toggleControl(this.group, this.outKey);
    };
    this.inKey = "keylock";
};

DJ202.KeylockButton.prototype.connect = function() {
    this.deck = script.deckFromGroup(this.group);
    components.Button.prototype.connect.call(this);
    // components.Component automatically unshifts upon component instantiation.
    // However, we need to trigger side-effects upon unshifting (button LED
    // issue). Hence, we need to unshift again after we are connected.
    this.unshift();
};

DJ202.KeylockButton.prototype.shift = function() {
    this.midi = [0x90 + this.deck - 1, 0x0E];
    this.send(0);
    this.inKey = "rateRange";
    this.type = undefined;
    this.input = function(channel, control, value, status, _group) {
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
    };
};

DJ202.ParamButtons = function() {
    components.Button.apply(this, arguments);
    this.isSongKeyMode = false;
    this.active = [false, false];
};

DJ202.ParamButtons.prototype = Object.create(components.Button.prototype);

DJ202.ParamButtons.prototype.setLEDs = function(plusValue, minusValue) {
    const deck = script.deckFromGroup(this.group);
    const channel = 0x94 + deck - 1;
    [0, 2, 4, 8, 10].forEach(function(offSet) {
        midi.sendShortMsg(channel, 0x41 + offSet, plusValue);
        midi.sendShortMsg(channel, 0x42 + offSet, minusValue);
    });
};

DJ202.ParamButtons.prototype.connect = function() {
    components.Button.prototype.connect.call(this);
    const keyConnection =
    engine.makeConnection(this.group, "pitch_adjust", this.output);
    this.connections.push(keyConnection);
};

DJ202.ParamButtons.prototype.output = function(value, group, control) {
    if (!this.isSongKeyMode) {
        return;
    }

    if (this.isSongKeyMode && control !== "pitch_adjust") {
        return;
    }

    // The control value returned has floating point jitter, so 0 can be
    // 0.00…1 and 1 can be 0.99.
    if (value < 0.5 && value > -0.5) {
        this.setLEDs(0, 0);
    }
    if (value < -0.5) {
        this.setLEDs(0x7f, 0);
        return;
    }
    if (value > 0.5) {
        this.setLEDs(0, 0x7f);
    }
};

DJ202.ParamButtons.prototype.songKeyMode = function(toggle) {
    this.isSongKeyMode = toggle;
    if (toggle) {
        this.trigger();
    } else {
        this.setLEDs(0, 0);
    }
};

DJ202.ParamButtons.prototype.input = function(channel, control, value, status,
    group) {
    const isPlus = control % 2 === 0;

    this.active[isPlus ? 0 : 1] = Boolean(value);

    // FIXME: This make the LEDs light up on press, but doesn’t properly
    // connect the output controls, so the buttons won’t light when
    // manipulated from within the GUI.
    const deck = script.deckFromGroup(group);
    midi.sendShortMsg(0x94 + deck - 1, control, value);

    if (!value) {
    // Work-around LED self-reset on release.
        this.trigger();
        return;
    }

    if (this.active.every(Boolean)) {
        script.triggerControl(group, "reset_key");
        return;
    }

    if (this.isSongKeyMode) {
        const adjust = engine.getValue(group, "pitch_adjust");
        const newAdjust =
      isPlus ? Math.min(7, adjust + 1) : Math.max(-7, adjust - 1);
        engine.setValue(group, "pitch_adjust", newAdjust);
        return;
    }

    const beatjumpSize = engine.getValue(group, "beatjump_size");
    const beatloopSize = engine.getValue(group, "beatloop_size");

    switch (control) {
    case 0x41: // Loop mode.
    case 0x42:
        engine.setValue(group, "loop_move", isPlus ? beatjumpSize : -beatjumpSize);
        break;
    case 0x43: // Hot-Cue mode.
    case 0x44:
    // get pad mode
        if (this.pad.mode === DJ202.PadMode.PITCHPLAY) {
            if (this.pad.pitchplayRange === DJ202.pitchplayRange.UP) {
                this.pad.pitchplayRange = isPlus ? DJ202.pitchplayRange.DOWN : DJ202.pitchplayRange.MID;
            } else if (this.pad.pitchplayRange === DJ202.pitchplayRange.MID) {
                this.pad.pitchplayRange = isPlus ? DJ202.pitchplayRange.UP : DJ202.pitchplayRange.DOWN;
            } else {
                this.pad.pitchplayRange = isPlus ? DJ202.pitchplayRange.MID : DJ202.pitchplayRange.UP;
            }
        } else {
            script.triggerControl(group,
                isPlus ? "beatjump_forward" : "beatjump_backward");
        }
        break;
    case 0x49: // Loop mode (shifted).
    case 0x4A:
        engine.setValue(group, "beatloop_size",
            isPlus ? beatloopSize * 2 : beatloopSize / 2);
        break;
    case 0x4B: // Hot-Cue mode (shifted).
    case 0x4C:
        engine.setValue(group, "beatjump_size",
            isPlus ? beatjumpSize * 2 : beatjumpSize / 2);
        break;
    }
};

///////////////////////////////////////////////////////////////////////////////
// SLICER
DJ202.slicerConnections = [null, null, null, null];
DJ202.slicerBeatsPassed = [0, 0, 0, 0];
DJ202.slicerPreviousBeatsPassed = [0, 0, 0, 0];
DJ202.slicerAlreadyJumped = [false, false, false, false];

DJ202.slicerBeatCounter = function(_value, group, _control) {
    if (engine.getValue(group, "beat_closest") ===
    engine.getValue(group, "beat_next")) {
        return;
    }
    const deck = script.deckFromGroup(group);
    const isLeftDeck = deck % 2;
    const channel = isLeftDeck ? 0x94 : 0x95;
    const padMode = isLeftDeck ? DJ202.leftDeck.padSection.mode :
        DJ202.rightDeck.padSection.mode;

    const playposition = engine.getValue(group, "playposition");
    const bpm = engine.getValue(group, "bpm");
    const duration = engine.getValue(group, "duration");
    DJ202.slicerBeatsPassed[deck] =
    Math.round((playposition * duration) * (bpm / 60));
    const slicerPosInSection =
    Math.floor((DJ202.slicerBeatsPassed[deck] % DJ202.slicerBeatsWindow) /
      (DJ202.slicerBeatsWindow / 8));

    if (padMode === DJ202.PadMode.SLICERLOOP) {
    // jump to loop the slicer section
        if (((DJ202.slicerBeatsPassed[deck] - 1) % DJ202.slicerBeatsWindow) ===
      (DJ202.slicerBeatsWindow - 1) &&
      !DJ202.slicerAlreadyJumped[deck] &&
      DJ202.slicerPreviousBeatsPassed[deck] < DJ202.slicerBeatsPassed[deck]) {
            engine.setValue(group, "beatjump", -DJ202.slicerBeatsWindow);
            DJ202.slicerAlreadyJumped[deck] = true;
        } else {
            DJ202.slicerAlreadyJumped[deck] = false;
        }
    }

    // light the current position LED
    for (let i = 0; i < 8; ++i) {
        const toggleLED =
      (padMode === DJ202.PadMode.SLICER ? slicerPosInSection === i :
          slicerPosInSection !== i);
        midi.sendShortMsg(channel, 0x20 + i + 1, (toggleLED ? 0x7F : 0x0));
    }
};

DJ202.slicer = function(value, number, isSlicerLoop) {
    const isLeftDeck = number <= 8;
    const group =
    isLeftDeck ? DJ202.leftDeck.currentDeck : DJ202.rightDeck.currentDeck;
    const deck = script.deckFromGroup(group);
    const index = isLeftDeck ? number : number - 8;
    const channel = isLeftDeck ? 0x94 : 0x95;

    const domain = DJ202.slicerBeatsWindow;
    let beatsToJump = 0;

    // LED indicator
    midi.sendShortMsg(channel, 0x20 + index + 1, (value ? 0x7F : 0x0));

    if (value) {
        engine.setValue(group, "reloop_toggle", 1);
        beatsToJump =
      (index * (domain / 8)) - ((DJ202.slicerBeatsPassed[deck] % domain) + 1);
        if (index === 0 && beatsToJump === -domain) {
            beatsToJump = 0;
        }
        if (DJ202.slicerBeatsPassed[deck] >= Math.abs(beatsToJump) &&
      DJ202.slicerPreviousBeatsPassed[deck] !==
      DJ202.slicerBeatsPassed[deck]) {
            DJ202.slicerPreviousBeatsPassed[deck] = DJ202.slicerBeatsPassed[deck];
            if (Math.abs(beatsToJump) > 0) {
                engine.setValue(group, "beatjump", beatsToJump);
            }
        }
    }

    if (!isSlicerLoop) {
        engine.setValue(group, "slip_enabled", value);
        engine.setValue(group, "beatloop_size", 1);
        engine.setValue(group, "beatloop_activate", value);
    }
};

///////////////////////////////////////////////////////////////////////////////
// Pad Section

DJ202.PadSection = function(_deck) {
    components.ComponentContainer.call(this);

    this.mode = DJ202.PadMode.HOTCUE;
    this.pitchplayRange = DJ202.pitchplayRange.MID;
    this.hotcueButton = [];

    for (var i = 1; i <= 8; i++) {
        this.hotcueButton[i] = new DJ202.HotcueButton({
            number: i,
            pad: this
        });
    }

    this.loopButton = [];

    for (i = 1; i <= 4; i++) {
        this.loopButton[i] = new DJ202.LoopButton({
            number: i,
            pad: this
        });
    }

    this.loopIn = this.loopButton[5] = new DJ202.ManualLoopButton({
        cc: 0x15,
        inKey: "loop_in",
        outKey: "loop_start_position",
    });
    this.loopOut = this.loopButton[6] = new DJ202.ManualLoopButton({
        cc: 0x16,
        inKey: "loop_out",
        outKey: "loop_end_position",
    });
    this.loopExit = this.loopButton[7] = new DJ202.ManualLoopButton({
        cc: 0x17,
        inKey: "reloop_andstop",
        outKey: "reloop_andstop",
    });
    this.loopToggle = this.loopButton[8] = new DJ202.ManualLoopButton({
        cc: 0x18,
        inKey: "reloop_toggle",
        outKey: "loop_enabled",
    });

    this.paramPlusMinus = new DJ202.ParamButtons({
        pad: this
    });

    this.setHotcueLED = function(active) {
        for (let i = 1; i <= 8; i++) {
            this.hotcueButton[i].send(0);
        }
        this.hotcueButton[active].send(0x7F);
    };
};

DJ202.PadSection.prototype =
  Object.create(components.ComponentContainer.prototype);

DJ202.PadSection.prototype.setState = function(channel, control, value, status,
    group) {
    // reset to defaults first
    // stop the slicer
    if (DJ202.slicerConnections[script.deckFromGroup(group)]) {
        DJ202.slicerConnections[script.deckFromGroup(group)].disconnect();
    }
    this.reconnectComponents();
    DJ202.sampler.reconnectComponents();

    switch (value) {
    case 0x0:
    case 0x3:
        this.mode = DJ202.PadMode.HOTCUE;
        break;
    case 0x1:
        this.mode = DJ202.PadMode.CUELOOP;
        break;
    case 0x2:
        this.mode = DJ202.PadMode.PITCHPLAY;
        this.pitchplayCue = -1;
        // activate the first available hotcue for pitchplay
        for (let i = 1; i <= 8; ++i) {
            if (engine.getValue(group, `hotcue_${  i  }_enabled`)) {
                this.pitchplayCue = i;
                this.setHotcueLED(i);
                break;
            }
        }
        break;

    case 0x10:
    case 0x12:
        this.mode = DJ202.PadMode.LOOP;
        break;
    case 0x11:
    case 0x13:
        this.mode = DJ202.PadMode.ROLL;
        break;

    case 0x20:
        this.mode = DJ202.PadMode.SEQUENCER;
        break;
    case 0x21:
        this.mode = DJ202.PadMode.INSTRUMENTRECORD;
        break;
    case 0x22:
        this.mode = DJ202.PadMode.PATTERN;
        break;

    case 0x30:
        this.mode = DJ202.PadMode.SAMPLER;
        break;
    case 0x32:
        DJ202.slicerConnections[script.deckFromGroup(group)] =
      engine.makeConnection(group, "beat_active", DJ202.slicerBeatCounter);
        this.mode = DJ202.PadMode.SLICERLOOP;
        break;
    case 0x31:
        DJ202.slicerConnections[script.deckFromGroup(group)] =
      engine.makeConnection(group, "beat_active", DJ202.slicerBeatCounter);
        this.mode = DJ202.PadMode.SLICER;
        break;
    }
};

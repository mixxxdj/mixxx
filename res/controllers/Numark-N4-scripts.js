var NumarkN4 = {};

NumarkN4.scratchSettings = {
    "alpha": 1.0 / 8,
    "beta": 1.0 / 8 / 32,
    "jogResolution": 600,
    "vinylSpeed": 33 + 1 / 3,
};

NumarkN4.searchAmplification = 5; // multiplier for the jogwheel when the search button is held down.

NumarkN4.warnAfterTime = 30; // Acts like the "End of Track warning" setting within the waveform settings.

NumarkN4.blinkInterval=1000; //blinkInterval for the triangular Leds over the channels in milliseconds.

NumarkN4.encoderResolution=0.05; // 1/encoderResolution = number of steps going from 0% to 100%

NumarkN4.resetHotCuePageOnTrackLoad=true; // resets the page of the Hotcue back to 1 after loading a new track.

NumarkN4.cueReverseRoll=true; // enables the ability to do a reverse roll while shift-pressing the cue button

// true = wrap around => scrolling past 4 will reset the page to the first page and vice versa
// false = clamp the the pages to the [1:4] range
NumarkN4.hotcuePageIndexBehavior=true;

// possible ranges (0.0..3.0 where 0.06=6%)
NumarkN4.rateRanges = [0,   // default (gets set via script later; don't modify)
    0.06, // one semitone
    0.24, // for maximum freedom
];

//
// CONSTANTS DO NOT CHANGE (if you don't know what you are doing)
//
NumarkN4.QueryStatusMessage=[0xF0, 0x00, 0x01, 0x3F, 0x7F, 0x47, 0x60, 0x00, 0x01, 0x54, 0x01, 0x00, 0x00, 0x00, 0x00, 0xF7];
//NumarkN4.ShutoffSequence=[0xF0,0x00,0x01,0x3F,0x7F,0x47,0xB0,0x39,0x00,0x01,0xF7]; // Invalid Midibyte?

NumarkN4.vinylTouched = [false, false, false, false];

NumarkN4.globalShift = false;

NumarkN4.scratchXFader = {
    xFaderMode: 0, // fast cut (additive)
    xFaderCurve: 999.60,
    xFaderCalibration: 1.0
};

components.Encoder.prototype.input = function(_channel, _control, value, _status, _group) {
    this.inSetParameter(
        this.inGetParameter()+(
            (value===0x01)?
                NumarkN4.encoderResolution:
                -NumarkN4.encoderResolution
        )
    );
};

components.Component.prototype.send = function(value) {
    // This Override is supposed to make integration automatic assignment of elements easier.
    // Right now it just allows specifying the input and output bytes (even though the input bytes dont do anything right now.)
    if (this.midi === undefined || this.midi[0] === undefined || this.midi[1] === undefined) {
        return;
    }
    if (this.midi[2]===undefined) { //check if output channel/type not explicitly defined
        this.midi[2]=this.midi[0];
    }
    if (this.midi[3]===undefined) { //check if output control not explicitly defined
        this.midi[3]=this.midi[1];
    }
    midi.sendShortMsg(this.midi[2], this.midi[3], value);
    if (this.sendShifted) {
        if (this.shiftChannel) {
            midi.sendShortMsg(this.midi[2] + this.shiftOffset, this.midi[3], value);
        } else if (this.shiftControl) {
            midi.sendShortMsg(this.midi[2], this.midi[3] + this.shiftOffset, value);
        }
    }
};

// gets filled via trigger of the callbacks in NumarkN4.crossfaderCallbackConnections
NumarkN4.storedCrossfaderParams = {};
NumarkN4.crossfaderCallbackConnections = [];
NumarkN4.CrossfaderChangeCallback = function(value, group, control) {
    // indicates that the crossfader settings were changed while during session
    this.changed = true;
    NumarkN4.storedCrossfaderParams[control] = value;
};

NumarkN4.init = function() {
    NumarkN4.rateRanges[0]=engine.getValue("[Channel1]", "rateRange");
    NumarkN4.Decks=[];
    for (var i=1; i<=4; i++) {
    // Array is based on 1 because it makes more sense in the XML
        NumarkN4.Decks[i] = new NumarkN4.Deck(i);
    }
    // create xFader callbacks and trigger them to fill NumarkN4.storedCrossfaderParams
    _.forEach(NumarkN4.scratchXFader, function(value, control) {
        var connectionObject = engine.makeConnection("[Mixer Profile]", control, NumarkN4.CrossfaderChangeCallback.bind(this));
        connectionObject.trigger();
        NumarkN4.crossfaderCallbackConnections.push(connectionObject);
    });

    NumarkN4.Mixer = new NumarkN4.MixerTemplate();

    //query controller for component status
    midi.sendSysexMsg(NumarkN4.QueryStatusMessage, NumarkN4.QueryStatusMessage.length);

};

NumarkN4.topContainer = function(channel) {
    this.group = "[Channel"+channel+"]";
    var theContainer = this;

    this.btnEffect1 = new components.Button({
        midi: [0x90+channel, 0x13, 0xB0+channel, 0x0B],
        shift: function() {
            this.group="[EffectRack1_EffectUnit1]";
            this.type=components.Button.prototype.types.toggle;
            this.inKey="group_[Channel"+channel+"]_enable";
            this.outKey="group_[Channel"+channel+"]_enable";
        },
        unshift: function() {
            this.group=theContainer.group;
            this.type=components.Button.prototype.types.push;
            this.inKey="loop_in";
            this.outKey="loop_in";
        },
    });
    this.btnEffect2 = new components.Button({
        midi: [0x90+channel, 0x14, 0xB0+channel, 0x0C],
        shift: function() {
            this.group="[EffectRack1_EffectUnit2]";
            this.type=components.Button.prototype.types.toggle;
            this.inKey="group_[Channel"+channel+"]_enable";
            this.outKey="group_[Channel"+channel+"]_enable";
        },
        unshift: function() {
            this.group=theContainer.group;
            this.type=components.Button.prototype.types.push;
            this.inKey="loop_out";
            this.outKey="loop_out";
        },
    });
    this.btnSample3 = new components.Button({
        midi: [0x90+channel, 0x15, 0xB0+channel, 0x0D],
        shift: function() {
            this.type=components.Button.prototype.types.toggle;
            this.inKey="slip_enabled";
            this.outKey="slip_enabled";
        },
        unshift: function() {
            this.type=components.Button.prototype.types.push;
            this.inKey="beatloop_activate";
            this.outKey="beatloop_activate";
        },
    });
    this.btnSample4 = new components.Button({
        midi: [0x90+channel, 0x16, 0xB0+channel, 0x0E],
        outKey: "loop_enabled",
        shift: function() {
            this.type=components.Button.prototype.types.toggle;
            this.inKey="reloop_andstop";
        },
        unshift: function() {
            this.type=components.Button.prototype.types.push;
            this.inKey="reloop_toggle";
        },
    });
    // custom Hotcue Buttons
    this.hotcueButtons=[];

    for (var counter=0; counter<=3; counter++) {
        this.hotcueButtons[counter] = new components.HotcueButton({
            midi: [0x90+channel, 0x27+counter, 0xB0+channel, 0x18+counter],
            number: counter+1,
        });
    }
    this.encFxParam1 = new components.Encoder({
        midi: [0xB0+channel, 0x57],
        group: "[EffectRack1_EffectUnit1]",
        shift: function() {
            this.inKey="mix";
        },
        unshift: function() {
            this.inKey="super1";
        },
    });
    this.encFxParam2 = new components.Encoder({
        midi: [0xB0+channel, 0x58],
        group: "[EffectRack1_EffectUnit2]",
        shift: function() {
            this.inKey="mix";
        },
        unshift: function() {
            this.inKey="super1";
        },
    });
    this.encSample3 = new components.Encoder({
        midi: [0xB0+channel, 0x5A],
        hotCuePage: 0,
        applyHotcuePage: function(layer, displayFeedback) {
            // ES3 doesn't allow default values in the function signature
            // Could be replaced after migration to QJSEngine by "displayFeedback=true"
            // in the function arguments.
            if (displayFeedback === undefined) {
                displayFeedback = true;
            }
            // when the layer becommes negative, the (layer+4) will force a positive/valid page indexOf
            layer = NumarkN4.hotcuePageIndexBehavior ? (layer+4)%4 : Math.max(Math.min(layer, 3), 0); // clamp layer value to [0;3] range
            this.hotCuePage = layer;
            if (this.timer !== 0) {
                engine.stopTimer(this.timer);
                this.timer = 0;
            }
            var number = 0;
            for (var i=0; i<theContainer.hotcueButtons.length; ++i) {
                number = (i+1)+theContainer.hotcueButtons.length*this.hotCuePage;
                theContainer.hotcueButtons[i].disconnect();
                theContainer.hotcueButtons[i].number=number;
                theContainer.hotcueButtons[i].outKey="hotcue_" + number + "_enabled";
                theContainer.hotcueButtons[i].unshift(); // for setting inKey based on number property.
                theContainer.hotcueButtons[i].connect();
                theContainer.hotcueButtons[i].trigger();
            }
            //  displays the current hotcuepage index within the upper row of the buttongrid
            if (displayFeedback) {
                for (i=0; i<4; ++i) {
                    midi.sendShortMsg(0xB0+channel, 0x0B+i, (i-this.hotCuePage)?0x00:0x7F);
                }
            }
            this.timer = engine.beginTimer(1000, function() {
                theContainer.reconnectComponents();
                this.timer = 0;
            }.bind(this), true);
        },
        shift: function() {
            this.group=theContainer.group;
            this.input = function(_channel, _control, value, _status, _group) {
                if (value === 0x01) {
                    engine.setParameter(this.group, "loop_double", 1);
                } else {
                    engine.setParameter(this.group, "loop_halve", 1);
                }
            };
        },
        unshift: function() {
            this.input = function(_channel, _control, value, _status, _group) {
                this.applyHotcuePage(this.hotCuePage+(value===0x01?1:-1));
            };
        },
    });
    this.encSample4 = new components.Encoder({
        midi: [0xB0+channel, 0x59],
        shift: function() {
            this.inKey="beatjump_size";
            this.input = function(_channel, _control, value, _status, _group) {
                this.inSetValue(this.inGetValue() * (value===0x01 ? 2 : 0.5));
            };
        },
        unshift: function() {
            this.input = function(_channel, _control, value, _status, _group) {
                script.triggerControl(this.group, (value===1)?"beatjump_forward":"beatjump_backward");
            };
        },
    });
    this.shutdown = function() {
    // turn off hotcueButtons
        for (var i=0; i<theContainer.hotcueButtons.length; i++) {
            theContainer.hotcueButtons[i].send(0);
        }
        // turn all remaining LEDS of the topContainer
        theContainer.btnEffect1.send(0);
        theContainer.btnEffect2.send(0);
        theContainer.btnSample3.send(0);
        theContainer.btnSample4.send(0);
    };

    if (NumarkN4.resetHotCuePageOnTrackLoad) {
        engine.makeConnection(this.group, "track_loaded", function(_value, _group, _control) {
            theContainer.encSample3.applyHotcuePage(0, false);
            // resets the hotcuepage to 0 hidden (without feedback to the user);
        });
    }
};
NumarkN4.topContainer.prototype = new components.ComponentContainer();

NumarkN4.MixerTemplate = function() {
    //channel will always be 0 it can be "hardcoded" into the components
    this.deckChangeL = new components.Button ({
        midi: [0xB0, 0x50],
        input: function(_channel, _control, value, _status, _group) {
            this.output(value);
            //just "echos" the midi since the controller knows the deck its on itself but doesn't update the corresponding leds.
        },
    });
    this.deckChangeR = new components.Button ({
        midi: [0xB0, 0x51],
        input: function(_channel, _control, value, _status, _group) {
            this.output(value);
        },
    });

    this.channelInputSwitcherL = new components.Button({
        midi: [0x90, 0x49],
        group: "[Channel3]",
        inKey: "mute",
    });
    this.channelInputSwitcherR = new components.Button({
        midi: [0x90, 0x4A],
        group: "[Channel4]",
        inKey: "mute",
    });

    this.changeCrossfaderContour = new components.Button({
        midi: [0x90, 0x4B],
        state: false,
        input: function(channel, control, value, status, _group) {
            _.forEach(NumarkN4.crossfaderCallbackConnections, function(callbackObject) {
                callbackObject.disconnect();
            });
            NumarkN4.crossfaderCallbackConnections = [];
            this.state=this.isPress(channel, control, value, status);
            if (this.state) {
                _.forEach(NumarkN4.scratchXFader, function(value, control) {
                    engine.setValue("[Mixer Profile]", control, value);
                    NumarkN4.crossfaderCallbackConnections.push(
                        engine.makeConnection("[Mixer Profile]", control, NumarkN4.CrossfaderChangeCallback.bind(this))
                    );
                });
            } else {
                _.forEach(NumarkN4.storedCrossfaderParams, function(value, control) {
                    engine.setValue("[Mixer Profile]", control, value);
                    NumarkN4.crossfaderCallbackConnections.push(
                        engine.makeConnection("[Mixer Profile]", control, NumarkN4.CrossfaderChangeCallback.bind(this))
                    );
                });
            }
        }
    });

    this.navigationEncoderTick = new components.Encoder({
        midi: [0xB0, 0x44],
        group: "[Library]",
        stepsize: 1,
        shift: function() {
            this.inKey="MoveFocus";
        },
        unshift: function() {
            this.inKey="MoveVertical";
        },
        input: function(_midiChannel, _control, value, _status, _group) {
            this.inSetValue(value===0x01?this.stepsize:-this.stepsize); // value "rescaling"; possibly ineffiecent.
        },
    });
    this.navigationEncoderButton = new components.Button({
        shift: function() {
            this.type=components.Button.prototype.types.toggle;
            this.group="[Master]";
            this.inKey="maximize_library";
        },
        unshift: function() {
            this.type=components.Button.prototype.types.push;
            this.group="[Library]";
            this.inKey="GoToItem";
        },
    });
};

NumarkN4.MixerTemplate.prototype = new components.ComponentContainer();

NumarkN4.Deck = function(channel) {
    components.Deck.call(this, channel);
    this.group = "[Channel" + channel + "]";
    this.rateRangeEntry=1;
    this.lastOrientation = (channel % 2) ? 0 : 2;
    this.isSearching=false;
    var theDeck = this;
    this.topContainer = new NumarkN4.topContainer(channel);
    this.topContainer.reconnectComponents(function(component) {
        if (component.group === undefined) {
            component.group = this.group;
        }
    });
    this.eqKnobs = [];
    for (var i = 1; i <= 3; i++) {
        this.eqKnobs[i] = new components.Pot({
            midi: [0xB0, 0x29 + i + 5*(channel-1)],
            group: "[EqualizerRack1_"+theDeck.group+"_Effect1]",
            inKey: "parameter" + i,

            // The exact center of the Pots on my N4 are roughly around 0x3e instead of 0x40
            // This is a Hack which adds that offset back when the pot is in the center range.
            // The Pot snaps physically between values of 7700 and 8300.
            // 0.469970703125=7700/(1<<14) 0.506591796875=8300/(1<<14)
            // 0.015625=(0x40-0x3e)/0x80 => normalized offset
            inValueScale: function(value) {
                if (value > this.max*0.469970703125 && value < this.max*0.506591796875) {
                    return (value + this.max*0.015625) / this.max;
                } else {
                    return value / this.max;
                }
            },
        });
    }
    // for some reason the gainKnobs don't suffer the same issues as the EQKnobs
    this.gainKnob = new components.Pot({
        midi: [0xB0, 0x2C + 5*(channel-1)],
        shift: function() {
            this.group="[QuickEffectRack1_"+theDeck.group+"]";
            this.inKey="super1";
        },
        unshift: function() {
            this.group=theDeck.group;
            this.inKey="pregain";
        }
    });
    this.shiftButton = new components.Button({
        midi: [0x90+channel, 0x12, 0xB0+channel, 0x15],
        type: components.Button.prototype.types.powerWindow,
        state: false, //custom property
        inToggle: function() {
            this.state=!this.state;
            if (this.state) {
                theDeck.shift();
                NumarkN4.Mixer.shift();
            } else {
                theDeck.unshift();
                NumarkN4.Mixer.unshift();
            }
            this.output(this.state);
            theDeck.topContainer.reconnectComponents(function(component) {
                if (component.group === undefined) {
                    component.group = this.group;
                }
            });
        },
    });

    // NOTE: THE ORIENTATION BUTTONS BEHAVE REALLY WEIRD AND THE FOLLOWING IS REALLY CONFUSING BUT WORKS!
    this.orientationButtonLeft = new components.Button({
        midi: [0x90, 0x32+channel*2, 0xB0, 0x42+channel*2],
        key: "orientation",
        input: function(_channel, _control, value, _status, _group) {
            if (!this.ignoreNext) {
                if (value===0x7F) {
                    this.inSetValue(0);
                    theDeck.orientationButtonRight.ignoreNextOff = true;
                    this.ignoreNextOff=false;
                } else if (!this.ignoreNextOff && value===0x00) {
                    this.inSetValue(1);
                }
            } else { this.ignoreNext=false; }
        },
        output: function(value, _group, _control) {
            this.send(value===0?0x7F:0x00);
            this.ignoreNext=true;
            if (value===0) { theDeck.orientationButtonRight.ignoreNextOff = true; }
        },
    });
    this.orientationButtonRight = new components.Button({
        midi: [0x90, 0x33+channel*2, 0xB0, 0x43+channel*2],
        key: "orientation",
        input: function(_channel, _control, value, _status, _group) {
            if (!this.ignoreNext) {
                if (value===0x7F) {
                    this.inSetValue(2);
                    theDeck.orientationButtonLeft.ignoreNextOff = true;
                    this.ignoreNextOff=false;
                } else if (!this.ignoreNextOff && value===0x00) {
                    this.inSetValue(1);
                }
            } else { this.ignoreNext=false; }
        },
        output: function(value, _group, _control) {
            this.send(value===2?0x7F:0x00);
            if (value===2) { theDeck.orientationButtonLeft.ignoreNextOff = true; }
            this.ignoreNext=true;
        },
    });

    this.pflButton = new components.Button({
        midi: [0x90, 0x30+channel, 0xB0, 0x3F+channel],
        key: "pfl",
        // The controller echos every change to the pfl lights which would cause
        // an infinite feedback loop (flicker)
        // this workaround uses a timer (100ms) to ignore the echoing messages.
        flickerSafetyTimeout: true,
        input: function(_channel, _control, value, _status, _group) {
            if (this.flickerSafetyTimeout) {
                this.flickerSafetyTimeout=false;
                value/=0x7F;
                if (this.inGetParameter()!==value) {
                    this.inSetParameter(value);
                }
                engine.beginTimer(100, function() {
                    this.flickerSafetyTimeout=true;
                }.bind(this), true);
            }
        },
    });
    this.loadButton = new components.Button({
        midi: [0x90+channel, 0x06],
        shift: function() { this.inKey="eject"; },
        unshift: function() { this.inKey="LoadSelectedTrack"; },
    });
    this.playButton = new components.PlayButton({
        midi: [0x90+channel, 0x11, 0xB0+channel, 0x09],
    });

    this.cueButton = new components.CueButton({
        midi: [0x90+channel, 0x10, 0xB0+channel, 0x08],
        reverseRollOnShift: NumarkN4.cueReverseRoll,
    });

    this.jogWheelScratchEnable = new components.Button({
        midi: [0x90+channel, 0x2C],
        scratchEnabled: true,
        input: function(_channelmidi, control, value, status, _group) {
            if (this.isPress(channel, control, value, status)&&this.scratchEnabled) {
                engine.scratchEnable(channel,
                    NumarkN4.scratchSettings.jogResolution,
                    NumarkN4.scratchSettings.vinylSpeed,
                    NumarkN4.scratchSettings.alpha,
                    NumarkN4.scratchSettings.beta);
            } else {
                engine.scratchDisable(channel);
            }
        },
    });

    this.searchButton = new components.Button({
        midi: [0x90+channel, 0x00, 0xB0+channel, 0x12],
        shift: function() {
            this.input = function(channelmidi, control, value, status, _group) {
                if (this.isPress(channelmidi, control, value, status)) {
                    theDeck.isSearching=!theDeck.isSearching;
                    this.output(theDeck.isSearching?0x7F:0x00);
                }
            };
            this.output(theDeck.isSearching?0x7F:0x00);
        },
        unshift: function() {
            this.input = function(channelmidi, control, value, status, _group) {
                if (this.isPress(channelmidi, control, value, status)) {
                    theDeck.jogWheelScratchEnable.scratchEnabled=!theDeck.jogWheelScratchEnable.scratchEnabled;
                    this.output(theDeck.jogWheelScratchEnable.scratchEnabled?0x7F:0x00);
                }
            };
            this.output(theDeck.jogWheelScratchEnable.scratchEnabled?0x7F:0x00);
        },
    });
    this.jogWheelTurn = new components.Pot({
        midi: [0xB0+channel, 0x2C],
        inKey: "jog",
        group: theDeck.group,
        input: function(_channelmidi, _control, value, _status, _group) {
            value=(value<0x40?value:value-0x80); // centers values at 0
            if (theDeck.isSearching) { value*=NumarkN4.searchAmplification; }
            if (engine.isScratching(channel)) {
                engine.scratchTick(channel, value);
            } else {
                this.inSetValue(value);
            }
        },
    });

    this.manageChannelIndicator = function() {
        this.duration=engine.getParameter(theDeck.group, "duration");
        // checks if the playposition is in the warnTimeFrame
        if (engine.getParameter(theDeck.group, "playposition") * this.duration > (this.duration - NumarkN4.warnAfterTime)) {
            this.alternating=!this.alternating; //mimics a static variable
            midi.sendShortMsg(0xB0, 0x1D+channel, this.alternating?0x7F:0x0);
        } else {
            midi.sendShortMsg(0xB0, 0x1D+channel, 0x7F);
        }
    };
    engine.makeConnection(this.group, "track_loaded", function(value) {
        if (value === 0) {
            // track ejected, stop timer and manager
            engine.stopTimer(theDeck.blinkTimer);
            theDeck.blinkTimer=0;
            return; // return early so no new timer gets created.
        }
        // this previouslyLoaded guard is needed because everytime a new track gets
        // loaded into a deck without previously ejecting, a new timer would get
        // spawned which conflicted with the old (still running) timers.
        if (!this.previouslyLoaded) {
            //timer is more efficient is this case than a callback because it would be called too often.
            theDeck.blinkTimer=engine.beginTimer(NumarkN4.blinkInterval, theDeck.manageChannelIndicator.bind(this));
        }
        this.previouslyLoaded=value;
    }.bind(this));
    this.pitchBendMinus = new components.Button({
        midi: [0x90+channel, 0x18, 0xB0+channel, 0x3D],
        key: "rate_temp_down",
        shift: function() {
            this.inkey = "rate_temp_down_small";
        },
        unshift: function() {
            this.inkey = "rate_temp_down";
        }
    });
    this.pitchBendPlus = new components.Button({
        midi: [0x90+channel, 0x19, 0xB0+channel, 0x3C],
        key: "rate_temp_up",
        shift: function() {
            this.inkey = "rate_temp_up_small";
        },
        unshift: function() {
            this.inkey = "rate_temp_up";
        }
    });
    this.syncButton = new components.SyncButton({
        midi: [0x90+channel, 0x0F, 0xB0+channel, 0x07],
    });
    this.tapButton = new components.Button({
        midi: [0x90+channel, 0x1E, 0xB0+channel, 0x16],
        bpm: [],
        input: function(channelmidi, control, value, status, _group) {
            if (this.isPress(channelmidi, control, value, status)) {
                bpm.tapButton(channel);
            }
            this.output(value);
        },
    });

    this.keylockButton = new components.Button({
        midi: [0x90+channel, 0x1B, 0xB0+channel, 0x10],
        type: components.Button.prototype.types.toggle,
        shift: function() {
            // quantize is already handled by the components syncButton
            this.inKey="sync_key";
            this.outKey="sync_key";
        },
        unshift: function() {
            this.inKey="keylock";
            this.outKey="keylock";
        }
    });
    this.bpmSlider = new components.Pot({
        midi: [0xB0+channel, 0x01, 0xB0+channel, 0x37], //only specifying input MSB
        inKey: "rate",
        group: theDeck.group,
        invert: false,
    });
    this.pitchLedHandler = engine.makeConnection(this.group, "rate", function(value) {
    // Turns on when rate slider is centered
        midi.sendShortMsg(0xB0+channel, 0x37, value===0 ? 0x7F : 0x00);
    }.bind(this));
    this.pitchLedHandler.trigger();


    this.pitchRange = new components.Button({
        midi: [0x90+channel, 0x1A, 0xB0+channel, 0x1C],
        key: "rateRange",
        ledState: false,
        input: function() {
            if (theDeck.rateRangeEntry===NumarkN4.rateRanges.length) {
                theDeck.rateRangeEntry=0;
            }
            this.inSetValue(NumarkN4.rateRanges[theDeck.rateRangeEntry++]);
        },
        // NOTE: Just toggles to provide some visual Feedback.
        output: function() {
            this.send(this.ledState);
            this.ledState=!this.ledState;
        },
    });

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            // 'this' inside a function passed to reconnectComponents refers to the ComponentContainer
            // so 'this' refers to the custom Deck object being constructed
            c.group = this.currentDeck;
        }
    });
    this.shutdown = function() {
        this.topContainer.shutdown();
        this.pitchLedHandler.disconnect();
        midi.sendShortMsg(0xB0+channel, 0x37, 0); // turn off pitchLED
        this.pitchRange.send(0);
        this.keylockButton.send(0);
        this.searchButton.send(0);
        this.tapButton.send(0);
        this.syncButton.send(0);
        this.pitchBendPlus.send(0);
        this.pitchBendMinus.send(0);
        this.cueButton.send(0);
        this.playButton.send(0);
        this.shiftButton.send(0);
        if (theDeck.blinkTimer !== 0) {
            engine.stopTimer(theDeck.blinkTimer);
        }
        midi.sendShortMsg(0xB0, 0x1D+channel, 0); // turn off small triangle above LOAD button.
    };
};

NumarkN4.Deck.prototype = new components.Deck();

NumarkN4.shutdown = function() {
    for (var i=1; i<=4; i++) {
    // View Definition of Array for explanation.
        NumarkN4.Decks[i].shutdown();
    }
    // revert the crossfader parameters only if they haven't been changed by the
    // user and if they are currently set to scratch
    if (!NumarkN4.CrossfaderChangeCallback.changed || NumarkN4.changeCrossfaderContour.state) {
        _.forEach(NumarkN4.storedCrossfaderParams, function(value, control) {
            engine.setValue("[Mixer Profile]", control, value);
        });
    }
    // midi.sendSysexMsg(NumarkN4.ShutoffSequence,NumarkN4.ShutoffSequence.length);
};

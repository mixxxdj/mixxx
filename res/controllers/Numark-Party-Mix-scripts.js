// Author: Ryli Dunlap (rylito)

// Thanks to authors of other scripts used as a reference and to DJ Dexter and DarkPoubelle
// for the initial PartyMix mappings posted on the forum.

////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global midi                                                        */
//////////////////////////////////////////////////////////////////////// 

var NumarkPartyMix = function() {

    var SCRATCH_LONGPRESS_DELAY = 500;
    var LIBRARY_LONGPRESS_DELAY = 500;
    var FLASH_DELAY = 500;
    var USE_FLASH = false;
    var USE_SAMPLE_BANK = true;

    var RESOLUTION = 300;
    var RECORD_SPEED = 33 + (1 / 3);
    var ALPHA = 1.0 / 8;
    var BETA = ALPHA / 32;
    var RAMP_DOWN = true;
    var RAMP_UP = true;

    var ON = 0x7F;
    var OFF = 0x00;
    var DIM = 0x01;
    var FLASH = 0x40; // not recognized by the controller, but used as a flag to flash this with the script

    var SELF = 'SELF';
    var NOOP = 'NOOP';
    var PAD_PRESS = 'PAD_PRESS';

    var PFL_CONTROL = 0x1B;

    var forEach = function(array, func) {
        for (var i = 0; i < array.length; i++) {
            func(array[i]);
        }
    };

    var lookup = function(dict) {
        iterItems(dict, function(k, v) {
            dict[v] = k;
        });
        return dict;
    };

    //begin flash timer;
    var flashTimer = 0;
    var flashVal = DIM;

    var flashSet = {};
    var flashCount = 0;

    var flashLoop = function() {
        flashVal = (flashVal === DIM) ? ON : DIM;
        iterItems(flashSet, function(key, controlBytes) {
            midi.sendShortMsg(controlBytes[0], controlBytes[1], flashVal);
        });
    };

    var makeFlash = function(statusByte, controlByte) {
        var key = [statusByte, controlByte];
        if (!(key in flashSet)) {
            flashSet[key] = key;
            flashCount += 1;

            if (!flashTimer) {
                flashTimer = engine.beginTimer(FLASH_DELAY, flashLoop, false);
            }
        }
    };

    var stopFlash = function(statusByte, controlByte) {
        var key = [statusByte, controlByte];
        var val = flashSet[key];
        if (val !== undefined) {
            delete flashSet[key];
            flashCount -= 1;

            if (!flashCount && flashTimer) {
                engine.stopTimer(flashTimer);
                flashTimer = 0;
            }
        }
    };
    //end flash timer

    var deckPadMode = {};

    var padCallbackMappings = {};

    var syncPadLedCallbackHelper = function(group, control, valueByte) {
        var key = [group, control];
        var mappings = padCallbackMappings[key];
        forEach(mappings, function(mapping) {
            if (deckPadMode[mapping.deck] === mapping.modeName) {
                if (valueByte === FLASH) {
                    makeFlash(mapping.statusByte, mapping.controlByte);
                } else {
                    stopFlash(mapping.statusByte, mapping.controlByte); // clear if flashing
                    midi.sendShortMsg(mapping.statusByte, mapping.controlByte, valueByte);
                }
            }
        });
    };

    //used to select relevent pad if callback has multiple mappings
    var syncSelfCallbackHelper = function(group, control, statusByte, controlByte, valueByte) {
        var key = [group, control];
        var mappings = padCallbackMappings[key];
        forEach(mappings, function(mapping) {
            if (deckPadMode[mapping.deck] === mapping.modeName && mapping.statusByte === statusByte && mapping.controlByte === controlByte) {
                midi.sendShortMsg(mapping.statusByte, mapping.controlByte, valueByte);
            }
        });
    };

    var padDefProto = {
        getCallbackKeyMappings: function() {
            var callbackKeyMappings = {};
            callbackKeyMappings[[this.group, this.bindingControl]] = function(value, group, control) {
                syncPadLedCallbackHelper(group, control, value ? ON : DIM);
            };
            return callbackKeyMappings;
        },
        handle: function(isPressed) {
            if (this.toggle) {
                if (isPressed) {
                    script.toggleControl(this.group, this.actionControl);
                }
            } else {
                engine.setValue(this.group, this.actionControl, isPressed);
            }
        },
    };

    var padDefCue = function(deck, cueNum) {
        this.group = '[Channel' + deck + ']';
        this.actionControl = 'hotcue_' + cueNum + '_activate';
        this.bindingControl = 'hotcue_' + cueNum + '_enabled';
        this.toggle = false;
    };
    padDefCue.prototype = padDefProto;

    var padDefLoop = function(deck, beatloopNum) {
        this.group = '[Channel' + deck + ']';
        this.actionControl = 'beatloop_' + beatloopNum;
        this.bindingControl = this.actionControl;
        this.toggle = true;
    };
    padDefLoop.prototype = padDefProto;

    var padDefSampler = function(samplerNum) {
        var trackLoaded = {};

        var getCurrentBankedGroup = function() {
            if (USE_SAMPLE_BANK) {
                var bank = engine.getValue('[Deere]', 'sampler_bank_current'); //0,1,2,3
                return '[Sampler' + ((bank * 4) + samplerNum) + ']'; //4 is num of pads
            }
            return '[Sampler' + samplerNum + ']'; //4 is num of pads
        };

        var trackPlayCallback = function(value, group, control) {
            if (getCurrentBankedGroup() === group) {
                var isLoaded = trackLoaded[group];
                var setLED = OFF;

                if (USE_FLASH && value) {
                    setLED = FLASH;
                } else if ((USE_FLASH && isLoaded && !value) || (!USE_FLASH && value)) {
                    setLED = ON;
                } else if ((USE_FLASH && !isLoaded) || (!USE_FLASH && isLoaded)) {
                    setLED = DIM;
                }

                syncPadLedCallbackHelper(group, control, setLED);
            }
        };

        var trackLoadedCallback = function(value, group, control) {
            trackLoaded[group] = value > 0;
            if (getCurrentBankedGroup() === group) {
                var isLoaded = trackLoaded[group];
                var setLED = ON;

                if (!isLoaded && !USE_FLASH) {
                    setLED = OFF;
                } else if ((isLoaded && !USE_FLASH) || (!isLoaded && USE_FLASH)) {
                    setLED = DIM;
                }

                syncPadLedCallbackHelper(group, control, setLED);
            }
        };

        this.getCallbackKeyMappings = function() {
            var callbackKeyMappings = {};
            for (var i = 0; i < 4; i++) { //number of banks
                var bindingGroup = '[Sampler' + (samplerNum + (i * 4)) + ']'; //number of pads
                callbackKeyMappings[[bindingGroup, 'play']] = trackPlayCallback,
                    /* jshint expr: true */
                    callbackKeyMappings[[bindingGroup, 'track_samples']] = trackLoadedCallback;
                /* jshint expr: false */
            }
            return callbackKeyMappings;
        };

        this.handle = function(isPressed) {
            if (isPressed) {
                var bankedGroup = getCurrentBankedGroup();
                if (engine.getValue(bankedGroup, 'play')) {
                    engine.setValue(bankedGroup, 'cue_gotoandstop', 1);
                } else if (trackLoaded[bankedGroup]) {
                    engine.setValue(bankedGroup, 'cue_gotoandplay', 1);
                } else {
                    engine.setValue(bankedGroup, 'LoadSelectedTrack', 1);
                }
            }
        };
    };

    var padDefSampleBank = function(sampleBankNum) {
        var samplerBankChangeCallback = function(value, group, control) {
            var key = [group, control];
            var mappings = padCallbackMappings[key];
            var targetControlByte = PAD_NUM_CONTROL_BYTE['PAD' + (value + 1)];
            forEach(mappings, function(mapping) {
                if (deckPadMode[mapping.deck] === mapping.modeName) {
                    var isActive = mapping.controlByte === targetControlByte;
                    midi.sendShortMsg(mapping.statusByte, mapping.controlByte, isActive ? ON : DIM);
                }
            });

            var bankOffset = value * 4;
            for (var i = 1; i <= 4; i++) {
                var bankGroup = '[Sampler' + (bankOffset + i) + ']';
                engine.trigger(bankGroup, 'track_samples');
                engine.trigger(bankGroup, 'play');
            }
        };

        this.getCallbackKeyMappings = function() {
            var callbackKeyMappings = {};
            callbackKeyMappings[['[Deere]', 'sampler_bank_current']] = samplerBankChangeCallback;
            return callbackKeyMappings;
        };

        this.handle = function(isPressed) {
            if (isPressed) {
                engine.setValue('[Deere]', 'sampler_bank_' + sampleBankNum, 1);
            }
        };
    };

    var padDefNoOp = {
        getCallbackKeyMappings: function() {
            var callbackKeyMappings = {};
            callbackKeyMappings[[SELF, NOOP]] = null;
            return callbackKeyMappings;
        },
        handle: function(isPressed) {},
    };

    var padDefSimpleEffect = function(func) {
        this.getCallbackKeyMappings = function() {
            var callbackKeyMappings = {};
            callbackKeyMappings[[SELF, PAD_PRESS]] = null;
            return callbackKeyMappings;
        };

        this.handle = function(isPressed) {
            func(isPressed);
        };
    };

    var padDefGeneric = function(group, control, toggle) {
        this.group = group;
        this.actionControl = control;
        this.bindingControl = control;
        this.toggle = true;
    };
    padDefGeneric.prototype = padDefProto;

    // Begin pad mappings
    var PAD_MAPPINGS = {
        DECK1: {
            PAD1: {
                CUE: new padDefCue(1, 1),
                LOOP: new padDefLoop(1, 1),
                SAMPLER: new padDefSampler(1),
                EFFECT: new padDefGeneric('[EffectRack1_EffectUnit1]', 'enabled'),
            },
            PAD2: {
                CUE: new padDefCue(1, 2),
                LOOP: new padDefLoop(1, 2),
                SAMPLER: new padDefSampler(2),
                EFFECT: new padDefGeneric('[EffectRack1_EffectUnit2]', 'enabled'),
            },
            PAD3: {
                CUE: new padDefCue(1, 3),
                LOOP: new padDefLoop(1, 4),
                SAMPLER: new padDefSampler(3),
                EFFECT: new padDefSimpleEffect(function(val) {
                    engine.brake(1, val);
                }),
            },
            PAD4: {
                CUE: new padDefCue(1, 4),
                LOOP: new padDefLoop(1, 8),
                SAMPLER: new padDefSampler(4),
                EFFECT: new padDefSimpleEffect(function(val) {
                    engine.spinback(1, val);
                })
            },
        },
        DECK2: {
            PAD1: {
                CUE: new padDefCue(2, 1),
                LOOP: new padDefLoop(2, 1),
                SAMPLER: USE_SAMPLE_BANK ? new padDefSampleBank(1) : padDefNoOp,
                EFFECT: new padDefGeneric('[EffectRack1_EffectUnit3]', 'enabled'),
            },
            PAD2: {
                CUE: new padDefCue(2, 2),
                LOOP: new padDefLoop(2, 2),
                SAMPLER: USE_SAMPLE_BANK ? new padDefSampleBank(2) : padDefNoOp,
                EFFECT: new padDefGeneric('[EffectRack1_EffectUnit4]', 'enabled'),
            },
            PAD3: {
                CUE: new padDefCue(2, 3),
                LOOP: new padDefLoop(2, 4),
                SAMPLER: USE_SAMPLE_BANK ? new padDefSampleBank(3) : padDefNoOp,
                EFFECT: new padDefSimpleEffect(function(val) {
                    engine.brake(2, val);
                }),
            },
            PAD4: {
                CUE: new padDefCue(2, 4),
                LOOP: new padDefLoop(2, 8),
                SAMPLER: USE_SAMPLE_BANK ? new padDefSampleBank(4) : padDefNoOp,
                EFFECT: new padDefSimpleEffect(function(val) {
                    engine.spinback(2, val);
                }),
            },
        }
    };
    // End pad mappings

    var iterItems = function(obj, func) {
        for (var k in obj) {
            if (!obj.hasOwnProperty(k)) {
                continue;
            }
            func(k, obj[k]);
        }
    };

    var PAD_MODE_CONTROL_BYTE = lookup({
        CUE: 0x00,
        LOOP: 0x0B,
        SAMPLER: 0x0E,
        EFFECT: 0x18,
    });

    var PAD_NUM_CONTROL_BYTE = lookup({
        PAD1: 0x14,
        PAD2: 0x15,
        PAD3: 0x16,
        PAD4: 0x17,
    });

    var DECK_PAD_CHANNEL = lookup({
        DECK1: 4,
        DECK2: 5,
    });

    var initPads = function() {
        iterItems(PAD_MAPPINGS, function(deck, pads) {
            iterItems(pads, function(pad, modes) {
                iterItems(modes, function(mode, defs) {
                    var deckPadChannel = DECK_PAD_CHANNEL[deck];
                    var statusByte = deckPadChannel + 0x90;
                    var controlByte = PAD_NUM_CONTROL_BYTE[pad];
                    var callbackKeys = defs.getCallbackKeyMappings();
                    iterItems(callbackKeys, function(key, callbackFunc) {
                        var existing = padCallbackMappings[key];
                        if (existing === undefined) {
                            existing = [];
                            padCallbackMappings[key] = existing;
                            var groupAndControl = key.split(',');
                            if (groupAndControl[0] !== SELF) {
                                engine.connectControl(groupAndControl[0], groupAndControl[1], callbackFunc);
                            }
                        }
                        existing.push({
                            'deck': deck,
                            'modeName': mode,
                            'statusByte': statusByte,
                            'controlByte': controlByte
                        });
                    });
                });
            });
        });
    };

    this.init = function(id, debugging) {

        var pflLED = function(value, group, control) {

            var channel = (group === '[Channel1]') ? 0 : 1;

            if (value) {
                midi.sendShortMsg(0x90 + channel, PFL_CONTROL, ON);
            } else {
                midi.sendShortMsg(0x80 + channel, PFL_CONTROL, OFF);
            }
        };

        //TODO this syntax changes to engine.makeConnection in 2.1
        engine.connectControl('[Channel1]', 'pfl', pflLED);
        engine.connectControl('[Channel2]', 'pfl', pflLED);

        initPads();

        // The SysEx message to send to the controller to force the midi controller
        // to send the status of every item on the control surface.
        // 0x00 0x01 0x3F is Numark mfg. ID used in SysEx messages.
        var ControllerStatusSysex = [0xF0, 0x00, 0x01, 0x3F, 0x38, 0x48, 0xF7];

        // After midi controller receives this Outbound Message request SysEx Message,
        // midi controller will send the status of every item on the
        // control surface. (Mixxx will be initialized with current values)
        //
        // Explanation of Serato's Sysex message is here which helped figure out what Numark
        // was using for this controller:
        // https://www.mixxx.org/wiki/doku.php/serato_sysex
        midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
    };

    var longPressTimers = {};

    var longPressHelper = function(status, control, delay, onDownCallback, onTimerEndWhileDownCallback, onUpBeforeTimerEndCallback, onUpAfterTimerEndCallback) {
        /*jslint bitwise: true */
        var opcode = status & 0xF0;
        /*jslint bitwise: false */
        var channel = (status - opcode);
        var timerKey = [channel, control];
        var timer = longPressTimers[timerKey];

        var resetTimer = function() {
            longPressTimers[timerKey] = 0;
        };

        var call = function(func) {
            if (func) {
                func();
            }
        };

        if (opcode === 0x80) {
            if (timer) {
                engine.stopTimer(timer);
                resetTimer();
                call(onUpBeforeTimerEndCallback);
            } else {
                call(onUpAfterTimerEndCallback);
            }
        } else if (opcode === 0x90) {
            call(onDownCallback);
            timer = engine.beginTimer(delay, function() {
                resetTimer();
                call(onTimerEndWhileDownCallback);
            }, true);
            longPressTimers[timerKey] = timer;
        }
    };

    this.handlePfl = function(channel, control, value, status, group) {
        engine.setValue(group, 'pfl', value ? 1 : 0);
    };

    this.setPadMode = function(channel, control, value, status, group) {

        var deck = DECK_PAD_CHANNEL[channel];
        var modeName = PAD_MODE_CONTROL_BYTE[control];
        deckPadMode[deck] = modeName;

        //trigger
        iterItems(padCallbackMappings, function(key, mappings) {
            forEach(mappings, function(bindings) {
                if (bindings.deck === deck && bindings.modeName === modeName) {
                    var groupAndControl = key.split(',');
                    var triggerGroup = groupAndControl[0];
                    var triggerControl = groupAndControl[1];
                    if (groupAndControl[0] !== SELF) {
                        engine.trigger(triggerGroup, triggerControl);
                    } else {
                        syncPadLedCallbackHelper(triggerGroup, triggerControl, triggerControl == NOOP ? OFF : DIM);
                    }
                }
            });
        });
    };

    this.handlePad = function(channel, control, value, status, group) {

        var deck = DECK_PAD_CHANNEL[channel];
        var modeName = deckPadMode[deck];
        if (modeName === undefined) {
            return;
        }

        var padNum = PAD_NUM_CONTROL_BYTE[control];
        var padDefinition = PAD_MAPPINGS[deck][padNum][modeName];

        padDefinition.handle(value ? 1 : 0);

        syncSelfCallbackHelper(SELF, PAD_PRESS, 0x90 + channel, control, value ? ON : DIM);
    };

    this.scratch = function(channel, control, value, status, group) {

        var stopScratching = function() {
            if (engine.isScratching(script.deckFromGroup(group))) {
                engine.scratchDisable(script.deckFromGroup(group), RAMP_UP);
                midi.sendShortMsg(status, control, DIM);
                return false;
            }
            return true;
        };

        var onDownCallback = function() {
            if (stopScratching()) {
                engine.scratchEnable(script.deckFromGroup(group), RESOLUTION, RECORD_SPEED, ALPHA, BETA, RAMP_DOWN);
                midi.sendShortMsg(status, control, ON);
            }
        };

        longPressHelper(status, control, SCRATCH_LONGPRESS_DELAY, onDownCallback, null, null, stopScratching);
    };

    this.wheelTurn = function(channel, control, value, status, group) {

        // A: For a control that centers on 0:
        var newValue = (value < 64) ? value : value - 128;

        // In either case, register the movement
        if (engine.isScratching(script.deckFromGroup(group))) {
            engine.scratchTick(script.deckFromGroup(group), newValue); // Scratch!
        } else {
            engine.setValue(group, 'jog', newValue); // Pitch bend
        }
    };

    //TODO The library functions have been improved greatly in 2.1. Update this to use them. For now, this will do

    var focusSidePane = true;

    this.moveVertical = function(channel, control, value, status, group) {

        var encoderValue = (value == 0x01) ? 1 : -1;

        if (focusSidePane) {
            engine.setValue('[Playlist]', 'SelectPlaylist', encoderValue);
        } else {
            engine.setValue('[Playlist]', 'SelectTrackKnob', encoderValue);
        }
    };

    this.toggleView = function(channel, control, value, status, group) {

        var toggleFocus = function() {
            focusSidePane = !focusSidePane;
        };

        var selectSidebar = function() {
            if (focusSidePane) {
                //TODO this is deprecated in 2.1
                engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1);
            }
        };

        longPressHelper(status, control, LIBRARY_LONGPRESS_DELAY, null, selectSidebar, toggleFocus, null);
    };


    this.shutdown = function() {

        // set modes back to CUE
        var cueByte = PAD_MODE_CONTROL_BYTE.CUE;
        midi.sendShortMsg(0x94, cueByte, ON);
        midi.sendShortMsg(0x95, cueByte, ON);

        // dim pads
        iterItems(PAD_MAPPINGS, function(deck, pads) {
            iterItems(pads, function(pad, modes) {
                midi.sendShortMsg(DECK_PAD_CHANNEL[deck] + 0x90, PAD_NUM_CONTROL_BYTE[pad], DIM);
            });
        });

        forEach([0x90, 0x91], function(deck) {
            // turn off LEDs for sync/play/cue
            forEach([0x00, 0x01, 0x02], function(control) {
                midi.sendShortMsg(deck, control, OFF);
            });

            // dim LEDs for scratch buttons
            midi.sendShortMsg(deck, 0x07, DIM);
        });

        // untoggle (dim) PFL switches
        forEach([0x80, 0x81], function(deck) {
            midi.sendShortMsg(deck, PFL_CONTROL, OFF);
        });
    };
};

NumarkPartyMix = new NumarkPartyMix();

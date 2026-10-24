/****************************************************************
*   Reloop Terminal Mix MIDI controller script v2.6             *
*       Copyright (C) 2012-2013, Sean M. Pappalardo             *
*                     2018, ronso0 (2.1 update)                 *
*                     2026, Christian (2.6 update)              *
*   but feel free to tweak this to your heart's content!        *
*   For Mixxx version 2.6.x                                     *
*                                                               *
*   Documentation in the Mixxx wiki:                            *
*   https://github.com/mixxxdj/mixxx/wiki/Reloop-Terminal-Mix   *
*****************************************************************/
(function(global) {

    /* Shortcut variables */
    const c = components;
    const e = global.behringer.extension;
    const toggle = c.Button.prototype.types.toggle;
    const script = global.script;

    /* Template variables */
    const $cc = Symbol();
    const $note = Symbol();
    const $deckNumber = Symbol();
    const $brake = Symbol();
    const $softStart = Symbol();
    const $spinback = Symbol();
    const $unitNumbers = Symbol();
    const $group = Symbol();
    const $left = Symbol();
    const $center = Symbol();
    const $right = Symbol();

    const TerminalMixJogWheel = function(options) {
        options = options || {};
        const defaultOptions = {
            basic: {wheelResolution: 800, alpha: 1.0/8},
            overrides: {
                inValueScale: function(rawValue) {
                    const value = rawValue - 64;
                    return engine.isScratching(this.deck) ? value : value / 4;
                }
            }
        };
        e.JogWheelUnit.call(this, Object.assign(defaultOptions, options));
    };
    TerminalMixJogWheel.prototype = e.deriveFrom(e.JogWheelUnit);

    const TerminalMix = new e.GenericMidiController({
        configurationProvider: function() {
            return {
                init: function() {
                    /* light BACK button */
                    midi.sendShortMsg(0x90, 0x37, c.Button.prototype.on);
                },
                decks: [{
                    bindings: [
                        {[$deckNumber]: 1, [$cc]: 0xB0, [$note]: 0x90, [$brake]: 0x68, [$softStart]: 0x69, [$spinback]: 0x6A},
                        {[$deckNumber]: 2, [$cc]: 0xB1, [$note]: 0x91, [$brake]: 0x6B, [$softStart]: 0x6A, [$spinback]: 0x69},
                        {[$deckNumber]: 3, [$cc]: 0xB2, [$note]: 0x92, [$brake]: 0x68, [$softStart]: 0x69, [$spinback]: 0x6A},
                        {[$deckNumber]: 4, [$cc]: 0xB3, [$note]: 0x93, [$brake]: 0x6B, [$softStart]: 0x6A, [$spinback]: 0x69},
                    ],
                    init: function() {
                        engine.softTakeover(this.currentDeck, "rate", true);
                    },
                    deckNumbers: [$deckNumber],
                    defaultDefinition: {type: c.Button},
                    components: [
                        {
                            type: e.FaderStartToggleButton,
                            options: {
                                midi: [$note, 0x78], volume: [$cc, 0x31], crossfader: [$cc, 0x3A],
                                sendShifted: true, shiftControl: true, shiftOffset: 0x32-0x78
                            },
                        },

                        /*
                         * Push the 'RANGE' button at the top edge of the pitch fader to cycle through the following pitch ranges.
                         * Edit the array to choose the ranges you need. For example '0.08' means +/-8%
                         */
                        {
                            type: e.EnumToggleButton,
                            options: {midi: [$note, 0x01], inKey: "rateRange", values: [0.08, 0.12, 0.25, 0.5, 1.0], feedback: true}
                        },                                                                                                                              // RANGE
                        {options: {midi: [$note, 0x47], effectUnit: 1}, type: c.EffectAssignmentButton},                                                // RANGE + SHIFT/DEL
                        {options: {midi: [$note, 0x02], key: "keylock"}, type: e.KeyButton},                                                            // KEYLOCK
                        {options: {midi: [$note, 0x48], effectUnit: 2}, type: c.EffectAssignmentButton},                                                // KEYLOCK + SHIFT/DEL
                        {options: {midi: [$note, 0x0B], inKey: "beatloop_activate", inKeyOff: "reloop_toggle", type: toggle}, type: e.TriggerButton},   // LOOP: LENGTH Button
                        {options: {midi: [$note, 0x51], inKey: "reloop_toggle", type: toggle}, type: e.TriggerButton},                                  // LOOP: LENGTH Button + SHIFT/DEL
                        {options: {midi: [$cc,   0x0B], on: 0x41, off: 0x3F, inKey: "loop_double", inKeyOff: "loop_halve"}, type: e.TriggerButton},     // LOOP: LENGTH Encoder
                        {options: {midi: [$note, 0x0C], inKey: "loop_in", outKey: "loop_enabled"}, type: e.BlinkingButton},                             // LOOP: IN
                        {options: {midi: [$note, 0x0D], inKey: "loop_out", outKey: "loop_enabled"}, type: e.BlinkingButton},                            // LOOP: OUT
                        {
                            type: e.LoopMoveUnit,
                            options: {
                                shiftButton: {midi: [$note, 0x0E]},                                                                                     // LOOP: MOVE Button
                                encoder: {midi: [$cc,   0x0E], min: 0x3F, max: 0x41, size: 1},                                                          // LOOP: MOVE Encoder
                                button: {midi: [$cc,   0x0E], off: 0x3F, on: 0x41},                                                                     // LOOP: MOVE Encoder + MOVE Button
                            },
                        },
                        {                                                                                                                               // LOOP: MOVE Encoder + SHIFT/DEL
                            type: e.TriggerButton,
                            options: {midi: [$cc, 0x54], off: 0x3F, on: 0x41, inKey: "beatjump_forward", inKeyOff: "beatjump_backward"},
                        },
                        {options: {midi: [$note, 0x10], number: 1}, type: c.HotcueButton},                // HOT CUE 1
                        {options: {midi: [$note, 0x11], number: 2}, type: c.HotcueButton},                // HOT CUE 2
                        {options: {midi: [$note, 0x12], number: 3}, type: c.HotcueButton},                // HOT CUE 3
                        {options: {midi: [$note, 0x13], number: 4}, type: c.HotcueButton},                // HOT CUE 4
                        {options: {midi: [$note, 0x18], number: 5}, type: c.HotcueButton},                // HOT CUE 1 + ✂
                        {options: {midi: [$note, 0x19], number: 6}, type: c.HotcueButton},                // HOT CUE 2 + ✂
                        {options: {midi: [$note, 0x1A], number: 7}, type: c.HotcueButton},                // HOT CUE 3 + ✂
                        {options: {midi: [$note, 0x1B], number: 8}, type: c.HotcueButton},                // HOT CUE 4 + ✂
                        {options: {midi: [$note, 0x56], number: 1}, type: c.HotcueButton, shift: true},   // HOT CUE 1 + SHIFT/DEL
                        {options: {midi: [$note, 0x57], number: 2}, type: c.HotcueButton, shift: true},   // HOT CUE 2 + SHIFT/DEL
                        {options: {midi: [$note, 0x58], number: 3}, type: c.HotcueButton, shift: true},   // HOT CUE 3 + SHIFT/DEL
                        {options: {midi: [$note, 0x59], number: 4}, type: c.HotcueButton, shift: true},   // HOT CUE 4 + SHIFT/DEL
                        {options: {midi: [$note, 0x5E], number: 5}, type: c.HotcueButton, shift: true},   // HOT CUE 1 + ✂ + SHIFT/DEL
                        {options: {midi: [$note, 0x5F], number: 6}, type: c.HotcueButton, shift: true},   // HOT CUE 2 + ✂ + SHIFT/DEL
                        {options: {midi: [$note, 0x60], number: 7}, type: c.HotcueButton, shift: true},   // HOT CUE 3 + ✂ + SHIFT/DEL
                        {options: {midi: [$note, 0x61], number: 8}, type: c.HotcueButton, shift: true},   // HOT CUE 4 + ✂ + SHIFT/DEL
                        {options: {midi: [$note, 0x54], key: "quantize", type: toggle}},                  // LOOP MOVE + SHIFT/DEL
                        {options: {midi: [$note, 0x06], key: "bpm_tap"}},                                 // BEATS KNOB
                        {options: {midi: [$note, 0x4C], key: "beats_translate_curpos"}},                  // BEATS KNOB + SHIFT/DEL
                        {options: {midi: [$note, $brake], effect: "brake"}, type: e.EffectButton},        // ▶◀ or ▶⏸ + SHIFT/DEL
                        {options: {midi: [$note, $softStart], effect: "softStart"}, type: e.EffectButton}, // ⛾ CUP or Q Cue + SHIFT/DEL
                        {options: {midi: [$note, $spinback], effect: "spinback"}, type: e.EffectButton},  // Q Cue or ⛾ CUP + SHIFT/DEL
                        {options: {midi: [$note, 0x22]}, type: c.SyncButton},                             // ▶◀ SYNC
                        {options: {midi: [$note, 0x23], key: "cue_gotoandplay"}},                         // ⛾ CUP
                        {options: {midi: [$note, 0x24], outKey: "cue_default"}, type: c.CueButton},       // Q Cue
                        {options: {midi: [$note, 0x25], outKey: "play"}, type: c.PlayButton},             // ▶⏸ Play/Pause
                        //{options: {midi: [$note, 0x26], type: toggle}},                                 // DECK
                        {options: {midi: [$cc,   0x31], inKey: "volume"}, type: c.Pot},                   // Linefader
                        {options: {midi: [$note, 0x32]}, type: e.TrackLoadButton},                        // 1/2/3/4
                        {options: {midi: [$note, 0x30], key: "pfl", type: toggle}},                       // PFL
                        {options: {midi: [$cc,   0x2B], inKey: "pregain"}, type: c.Pot},                  // GAIN
                        {options: {midi: [$note, 0x0A], outKey: "beat_active"}},                          // TAP
                        {options: {midi: [$cc,   0x29], inKey: "playposition"}, type: e.Rot64Encoder},    // JogWheel + 🖸
                    ],
                    equalizerUnit: {midi: {parameterKnobs: {1: [$cc, 0x2E], 2: [$cc, 0x2D], 3: [$cc, 0x2C]}}}, // LOW, MID, HIGH
                    quickEffectUnit: {midi: {super1: [$cc, 0x2F]}},                                            // FILTER
                    jogWheelUnit: {
                        type: TerminalMixJogWheel,
                        options: {
                            midi: {
                                jog: [$cc, 0x27],
                                vinylMode: {toggle: [$note, 0x20], touch: [$note, 0x28], jog: [$cc, 0x28]},
                            },
                        }
                    },
                }],
                effectUnits: [{
                    bindings: [
                        {[$unitNumbers]: [1, 3], [$cc]: 0xB0, [$note]: 0x90},
                        {[$unitNumbers]: [2, 4], [$cc]: 0xB1, [$note]: 0x91},
                    ],
                    unitNumbers: $unitNumbers,
                    midi: {
                        effectFocusButton: [$note, 0x0A],                                      // TAP
                        enableButtons: {1: [$note, 0x07], 2: [$note, 0x08], 3: [$note, 0x09]}, // FX1, FX2, FX3: On
                        knobs: {1: [$cc,   0x03], 2: [$cc,   0x04], 3: [$cc,   0x05]},         // FX1, FX2, FX3: Knob
                    },
                    midiShifted: {
                        effectFocusButton: [$note, 0x50],                                      // TAP
                        enableButtons: {1: [$note, 0x4D], 2: [$note, 0x4E], 3: [$note, 0x4F]}, // FX1, FX2, FX3: On
                        knobs: {1: [$cc,   0x49], 2: [$cc,   0x4A], 3: [$cc,   0x4B]},         // FX1, FX2, FX3: Knob
                    },
                    defaultDefinition: {type: e.SteppingEncoder, options: {min: 0x3F, max: 0x41, step: 0.07}},
                    components: [                                                              // FX: BEATS Knob (dryWetKnob)
                        {options: {midi: [$cc, 0x06], inKey: "mix"}},
                        {options: {midi: [$cc, 0x4C], inKey: "super1"}},
                    ],
                    init: function(definition) {
                        /* A proxy is required because `input` function of fx components is replaced at runtime */
                        const inputProxy = function(...args) {
                            this.input(...args);
                        };
                        const handler = function(midiAddress, implementation, _path) {
                            midi.makeInputHandler(midiAddress[0], midiAddress[1], inputProxy.bind(implementation));
                        };
                        e.processMidiAddresses(definition.midiShifted, this, handler, `effectUnit.${this.currentUnitNumber}`);
                    }
                }],
                containers: [
                    { // Shift
                        defaultDefinition: {type: e.ShiftButton, options: {target: this}},
                        components: [
                            {options: {midi: [0x90, 0x21]}}, // SHIFT/DEL Left
                            {options: {midi: [0x91, 0x21]}}, // SHIFT/DEL Right
                        ]
                    },
                    { // Master
                        components: [
                            {type: c.Pot, options: {midi: [0xB0, 0x33], inKey: "headMix", group: "[Master]"}} // CUE MIX
                        ]
                    },
                    { // Sampler
                        bindings: [
                            {[$cc]: 0xB0, [$note]: 0x90},
                            {[$cc]: 0xB1, [$note]: 0x91},
                            {[$cc]: 0xB2, [$note]: 0x92},
                            {[$cc]: 0xB3, [$note]: 0x93},
                        ],
                        defaultDefinition: {type: e.BlinkingSamplerButton},
                        components: [
                            {options: {midi: [$cc,   0x34], channelType: "Sampler"}, type: e.SuperPot}, // SAMPLER VOL
                            {options: {midi: [$note, 0x14], number: 1}}, // SAMPLER: 1▶
                            {options: {midi: [$note, 0x15], number: 2}}, // SAMPLER: 2▶
                            {options: {midi: [$note, 0x16], number: 3}}, // SAMPLER: 3▶
                            {options: {midi: [$note, 0x17], number: 4}}, // SAMPLER: 4▶
                            {options: {midi: [$note, 0x1C], number: 5}}, // SAMPLER: 1▶ + ✂
                            {options: {midi: [$note, 0x1D], number: 6}}, // SAMPLER: 2▶ + ✂
                            {options: {midi: [$note, 0x1E], number: 7}}, // SAMPLER: 3▶ + ✂
                            {options: {midi: [$note, 0x1F], number: 8}}, // SAMPLER: 4▶ + ✂
                            {options: {midi: [$note, 0x5A], number: 1}}, // SAMPLER: 1▶ + SHIFT/DEL
                            {options: {midi: [$note, 0x5B], number: 2}}, // SAMPLER: 2▶ + SHIFT/DEL
                            {options: {midi: [$note, 0x5C], number: 3}}, // SAMPLER: 3▶ + SHIFT/DEL
                            {options: {midi: [$note, 0x5D], number: 4}}, // SAMPLER: 4▶ + SHIFT/DEL
                            {options: {midi: [$note, 0x62], number: 5}}, // SAMPLER: 1▶ + ✂ + SHIFT/DEL
                            {options: {midi: [$note, 0x63], number: 6}}, // SAMPLER: 2▶ + ✂ + SHIFT/DEL
                            {options: {midi: [$note, 0x64], number: 7}}, // SAMPLER: 3▶ + ✂ + SHIFT/DEL
                            {options: {midi: [$note, 0x65], number: 8}}, // SAMPLER: 4▶ + ✂ + SHIFT/DEL
                        ]
                    },
                    { // Library
                        defaultDefinition: {type: c.Button, options: {group: "[Library]"}},
                        components: [
                            {options: {midi: [0x90, 0x35], group: "[Playlist]", key: "ToggleSelectedSidebarItem"}},                                // CRATES
                            {options: {midi: [0x90, 0x36], key: "show_track_menu", type: toggle}},                                                 // VIEW
                            /* Move focus right between tracks table and side panel. Shift moves the focus to the left. */
                            {options: {midi: [0x90, 0x37], inKey: "MoveFocusForward"}},                                                            // BACK
                            {options: {midi: [0x90, 0x37], inKey: "MoveFocusBackward"}, shift: true},                                              // BACK + SHIFT/DEL
                            {options: {midi: [0x90, 0x38], group: "[Skin]", key: "show_maximized_library", type: toggle}},                         // PREP
                            /* Move cursor vertically with Trax knob, scroll with Shift pressed */
                            {options: {midi: [0xB0, 0x39], inKey: "MoveVertical", min: 0x3F, max: 0x41}, type: e.DirectionEncoder},                // TRAX Knob
                            {options: {midi: [0xB0, 0x39], inKey: "ScrollVertical", min: 0x3F, max: 0x41}, type: e.DirectionEncoder, shift: true}, // TRAX Knob
                            {options: {midi: [0x90, 0x39], inKey: "GoToItem"}},                                                                    // TRAX Button
                        ],
                    },
                    { // Crossfader
                        components: [
                            {type: c.Pot, options: {midi: [0xB0, 0x3A], inKey: "crossfader", group: "[Master]"}}, // Crossfader
                            {type: e.CrossfaderCurvePot, options: {midi: [0xB0, 0x46], low: 0.01, high: 100}},    // CF CURVE
                        ]
                    },
                    { // Crossfader Assignment
                        bindings: [
                            {[$group]: "[Channel1]", [$left]: 0x3B, [$center]: 0x3C, [$right]: 0x3D},
                            {[$group]: "[Channel2]", [$left]: 0x3E, [$center]: 0x3F, [$right]: 0x40},
                            {[$group]: "[Channel3]", [$left]: 0x41, [$center]: 0x42, [$right]: 0x43},
                            {[$group]: "[Channel4]", [$left]: 0x44, [$center]: 0x45, [$right]: 0x46},
                        ],
                        defaultDefinition: {options: {group: $group, inKey: "orientation"}, type: e.EnumToggleButton},
                        components: [
                            {options: {midi: [0x90, $left],   values: [0]}}, // THRU Left
                            {options: {midi: [0x90, $center], values: [1]}}, // THRU Center
                            {options: {midi: [0x90, $right],  values: [2]}}, // THRU Right
                        ]
                    },
                ],
            };
        }
    });

    /*
     * Handle `rate` via XML until `midi.makeInputHandler()` has
     * support for 14-bit values (status byte only, without control byte)
     */
    TerminalMix.rate = function(_channel, control, value, status, group) {
        engine.setValue(group, "rate", -script.midiPitch(control, value, status));
    };

    global.TerminalMix = TerminalMix;
})(this);

/**
 * Mixxx controller mapping for a Behringer BCR2000 controller.
 */

/* Globally available objects are declared as variables to avoid linter errors */
var behringer = behringer, BCR2000Preset = BCR2000Preset;

var BCR2000 = new behringer.extension.GenericMidiController({
    configurationProvider: function() {

        /* Shortcut variables */
        var c = components;
        var e = behringer.extension;
        var p = BCR2000Preset;
        var cc = p.STATUS_CONTROL_CHANGE;

        return {
            init: function() {
                p.setPreset(1);
            },
            decks: [{
                deckNumbers: [1],
                components: [
                    {
                        type: e.ShiftButton, options:
                        {
                            midi: [cc, p.buttonRow1[3]],
                            group: "[Controls]",
                            key: "touch_shift",
                            target: this
                        }
                    },
                    {
                        type: e.RangeAwareEncoder, options: {
                            midi: [cc, p.pushEncoderGroup1[0].encoder], key: "pitch", bound: 6
                        }
                    },
                    {
                        type: e.Trigger, options: {
                            midi: [cc, p.pushEncoderGroup1[0].button], inKey: "pitch_set_zero"
                        }
                    },
                    {
                        type: c.Button, options: {midi: [cc, p.buttonRow1[0]], key: "keylock"}
                    },
                    {
                        type: e.EnumToggleButton, shift: true, options: {
                            midi: [cc, p.buttonRow1[0]], inKey: "vinylcontrol_mode", maxValue: 2
                        }
                    },
                    {
                        type: e.BackLoopButton,
                        options: {midi: [cc, p.buttonRow1[1]], outKey: "loop_enabled"}
                    },
                    {
                        type: c.Button, shift: true,
                        options: {midi: [cc, p.buttonRow1[1]], key: "beatlooproll_activate"}
                    },
                    {
                        type: c.Button, options: {midi: [cc, p.buttonRow1[2]], key: "reverse"}
                    },
                    {
                        type: c.Button, shift: true,
                        options: {midi: [cc, p.buttonRow1[2]], key: "reverseroll"}
                    },
                    {
                        type: e.LoopEncoder, options: {
                            midi: [cc, p.pushEncoderGroup1[1].encoder], key: "beatloop_size"
                        }
                    },
                    {
                        type: e.LoopMoveEncoder, options: {
                            midi: [cc, p.pushEncoderGroup1[2].encoder],
                            inKey: "loop_move",
                            sizeControl: "beatjump_size"
                        }
                    },
                    {
                        type: e.LoopEncoder, options: {
                            midi: [cc, p.pushEncoderGroup1[3].encoder], key: "beatjump_size"
                        }
                    },
                    {
                        type: c.EffectAssignmentButton, options: {
                            midi: [cc, p.buttonBox[0]],
                            effectUnit: 1,
                            type: c.Button.prototype.types.push
                        }
                    },
                    {
                        type: c.EffectAssignmentButton, options: {
                            midi: [cc, p.buttonBox[2]],
                            effectUnit: 2,
                            type: c.Button.prototype.types.push
                        }
                    },
                ],
                equalizerUnit: {
                    feedback: true,
                    midi: {
                        enabled: [cc, p.pushEncoderGroup2[0].button],
                        super1: [cc, p.pushEncoderGroup2[0].encoder],
                        parameterKnobs: {
                            1: [cc, p.pushEncoderGroup2[1].encoder],
                            2: [cc, p.pushEncoderGroup2[2].encoder],
                            3: [cc, p.pushEncoderGroup2[3].encoder],
                        },
                        parameterButtons: {
                            1: [cc, p.pushEncoderGroup2[1].button],
                            2: [cc, p.pushEncoderGroup2[2].button],
                            3: [cc, p.pushEncoderGroup2[3].button],
                        },
                    }
                },
            },
            {
                deckNumbers: [2],
                components: [
                    {
                        type: e.ShiftButton, options:
                        {
                            midi: [cc, p.buttonRow1[7]],
                            group: "[Controls]",
                            key: "touch_shift",
                            target: this
                        }
                    },
                    {
                        type: e.RangeAwareEncoder, options: {
                            midi: [cc, p.pushEncoderGroup1[4].encoder], key: "pitch", bound: 6
                        }
                    },
                    {
                        type: e.Trigger, options: {
                            midi: [cc, p.pushEncoderGroup1[4].button], inKey: "pitch_set_zero"
                        }
                    },
                    {
                        type: c.Button, options: {midi: [cc, p.buttonRow1[4]], key: "keylock"}
                    },
                    {
                        type: e.EnumToggleButton, shift: true, options: {
                            midi: [cc, p.buttonRow1[4]], inKey: "vinylcontrol_mode", maxValue: 2
                        }
                    },
                    {
                        type: e.BackLoopButton,
                        options: {midi: [cc, p.buttonRow1[5]], outKey: "loop_enabled"}
                    },
                    {
                        type: c.Button, shift: true,
                        options: {midi: [cc, p.buttonRow1[5]], key: "beatlooproll_activate"}
                    },
                    {
                        type: c.Button, options: {midi: [cc, p.buttonRow1[6]], key: "reverse"}
                    },
                    {
                        type: c.Button, shift: true,
                        options: {midi: [cc, p.buttonRow1[6]], key: "reverseroll"}
                    },
                    {
                        type: e.LoopEncoder, options: {
                            midi: [cc, p.pushEncoderGroup1[5].encoder], key: "beatloop_size"
                        }
                    },
                    {
                        type: e.LoopMoveEncoder, options: {
                            midi: [cc, p.pushEncoderGroup1[6].encoder],
                            inKey: "loop_move",
                            sizeControl: "beatjump_size"
                        }
                    },
                    {
                        type: e.LoopEncoder, options: {
                            midi: [cc, p.pushEncoderGroup1[7].encoder], key: "beatjump_size"
                        }
                    },
                    {
                        type: c.EffectAssignmentButton, options: {
                            midi: [cc, p.buttonBox[1]],
                            effectUnit: 1,
                            type: c.Button.prototype.types.push
                        }
                    },
                    {
                        type: c.EffectAssignmentButton, options: {
                            midi: [cc, p.buttonBox[3]],
                            effectUnit: 2,
                            type: c.Button.prototype.types.push
                        }
                    },
                ],
                equalizerUnit: {
                    feedback: true,
                    midi: {
                        enabled: [cc, p.pushEncoderGroup2[4].button],
                        super1: [cc, p.pushEncoderGroup2[4].encoder],
                        parameterKnobs: {
                            1: [cc, p.pushEncoderGroup2[5].encoder],
                            2: [cc, p.pushEncoderGroup2[6].encoder],
                            3: [cc, p.pushEncoderGroup2[7].encoder],
                        },
                        parameterButtons: {
                            1: [cc, p.pushEncoderGroup2[5].button],
                            2: [cc, p.pushEncoderGroup2[6].button],
                            3: [cc, p.pushEncoderGroup2[7].button],
                        },
                    }
                }
            }],
            effectUnits: [{
                feedback: true,
                unitNumbers: [1, 3],
                midi: {
                    effectFocusButton: [cc, p.buttonRow2[0]],
                    enableButtons: {
                        1: [cc, p.buttonRow2[1]],
                        2: [cc, p.buttonRow2[2]],
                        3: [cc, p.buttonRow2[3]],
                    },
                    dryWetKnob: [cc, p.encoderRow1[0]],
                    knobs: {
                        1: [cc, p.encoderRow1[1]],
                        2: [cc, p.encoderRow1[2]],
                        3: [cc, p.encoderRow1[3]],
                    },
                },
            },
            {
                feedback: true,
                unitNumbers: [2, 4],
                midi: {
                    effectFocusButton: [cc, p.buttonRow2[4]],
                    enableButtons: {
                        1: [cc, p.buttonRow2[5]],
                        2: [cc, p.buttonRow2[6]],
                        3: [cc, p.buttonRow2[7]],
                    },
                    dryWetKnob: [cc, p.encoderRow1[4]],
                    knobs: {
                        1: [cc, p.encoderRow1[5]],
                        2: [cc, p.encoderRow1[6]],
                        3: [cc, p.encoderRow1[7]],
                    },
                }
            }],
        };
    }
});

/* this statement exists to avoid linter errors */
BCR2000;

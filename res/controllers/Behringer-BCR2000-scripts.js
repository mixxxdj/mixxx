/**
 * Mixxx controller mapping for a Behringer BCR2000 controller.
 */

/* Globally available objects are declared as variables to avoid linter errors */
var BCR2000Preset = BCR2000Preset;

var BCR2000 = new components.extension.GenericMidiController({
    configurationProvider: function() {

        /* Shortcut variables */
        var c = components;
        var e = components.extension;
        var p = BCR2000Preset;
        var status = p.STATUS_CONTROL_CHANGE;

        return {
            init: function() {
                p.setPreset(1);
            },
            shutdown: function() {
                p.setPreset(31);
            },
            decks: [{
                deckNumbers: [1],
                components: [
                    {
                        type: e.ShiftButton, options: {
                            midi: [status, p.buttonRow1[3]], target: this
                        }
                    },
                    {
                        type: e.RangeAwareEncoder, options: {
                            midi: [status, p.pushEncoderGroup1[0].encoder], key: "pitch", bound: 6
                        }
                    },
                    {
                        type: e.Trigger, options: {
                            midi: [status, p.pushEncoderGroup1[0].button], inKey: "pitch_set_zero"
                        }
                    },
                    {
                        type: c.Button, options: {midi: [status, p.buttonRow1[0]], key: "keylock"}
                    },
                    {
                        type: e.EnumToggleButton, shift: true, options: {
                            midi: [status, p.buttonRow1[0]], inKey: "vinylcontrol_mode", maxValue: 3
                        }
                    },
                    {
                        type: e.BackLoopButton,
                        options: {midi: [status, p.buttonRow1[1]], outKey: "loop_enabled"}
                    },
                    {
                        type: c.Button, shift: true,
                        options: {midi: [status, p.buttonRow1[1]], key: "beatlooproll_activate"}
                    },
                    {
                        type: c.Button,
                        options: {midi: [status, p.buttonRow1[2]], key: "reverse"}
                    },
                    {
                        type: c.Button, shift: true,
                        options: {midi: [status, p.buttonRow1[2]], key: "reverseroll"}
                    },
                    {
                        type: e.LoopEncoder, options: {
                            midi: [status, p.pushEncoderGroup1[1].encoder], key: "beatloop_size"
                        }
                    },
                    {
                        type: e.LoopMoveEncoder, options: {
                            midi: [status, p.pushEncoderGroup1[2].encoder],
                            inKey: "loop_move",
                            sizeControl: "beatjump_size"
                        }
                    },
                    {
                        type: e.LoopEncoder, options: {
                            midi: [status, p.pushEncoderGroup1[3].encoder], key: "beatjump_size"
                        }
                    },
                    {
                        type: c.EffectAssignmentButton, options: {
                            midi: [status, p.buttonBox[0]],
                            effectUnit: 1,
                            type: c.Button.prototype.types.push
                        }
                    },
                    {
                        type: c.EffectAssignmentButton, options: {
                            midi: [status, p.buttonBox[1]],
                            effectUnit: 2,
                            type: c.Button.prototype.types.push
                        }
                    },
                ],
            },
            {
                deckNumbers: [2],
                components: [
                    {
                        type: e.ShiftButton,
                        options: {midi: [status, p.buttonRow1[7]], target: this}
                    },
                    {
                        type: e.RangeAwareEncoder, options: {
                            midi: [status, p.pushEncoderGroup1[4].encoder], key: "pitch", bound: 6
                        }
                    },
                    {
                        type: e.Trigger, options: {
                            midi: [status, p.pushEncoderGroup1[4].button], inKey: "pitch_set_zero"
                        }
                    },
                    {
                        type: c.Button, options: {midi: [status, p.buttonRow1[4]], key: "keylock"}
                    },
                    {
                        type: e.EnumToggleButton, shift: true, options: {
                            midi: [status, p.buttonRow1[4]], inKey: "vinylcontrol_mode", maxValue: 3
                        }
                    },
                    {
                        type: e.BackLoopButton,
                        options: {midi: [status, p.buttonRow1[5]], outKey: "loop_enabled"}
                    },
                    {
                        type: c.Button, shift: true,
                        options: {midi: [status, p.buttonRow1[5]], key: "beatlooproll_activate"}
                    },
                    {
                        type: c.Button, options: {midi: [status, p.buttonRow1[6]], key: "reverse"}
                    },
                    {
                        type: c.Button, shift: true,
                        options: {midi: [status, p.buttonRow1[6]], key: "reverseroll"}
                    },
                    {
                        type: e.LoopEncoder, options: {
                            midi: [status, p.pushEncoderGroup1[5].encoder], key: "beatloop_size"
                        }
                    },
                    {
                        type: e.LoopMoveEncoder, options: {
                            midi: [status, p.pushEncoderGroup1[6].encoder],
                            inKey: "loop_move",
                            sizeControl: "beatjump_size"
                        }
                    },
                    {
                        type: e.LoopEncoder, options: {
                            midi: [status, p.pushEncoderGroup1[7].encoder], key: "beatjump_size"
                        }
                    },
                    {
                        type: c.EffectAssignmentButton, options: {
                            midi: [status, p.buttonBox[2]],
                            effectUnit: 1,
                            type: c.Button.prototype.types.push
                        }
                    },
                    {
                        type: c.EffectAssignmentButton, options: {
                            midi: [status, p.buttonBox[3]],
                            effectUnit: 2,
                            type: c.Button.prototype.types.push
                        }
                    },
                ]
            }],
            effectUnits: [{
                unitNumbers: [1, 3],
                components: {
                    effectFocusButton: [status, p.buttonRow2[0]],
                    enableButtons: {
                        1: [status, p.buttonRow2[1]],
                        2: [status, p.buttonRow2[2]],
                        3: [status, p.buttonRow2[3]],
                    },
                    dryWetKnob: [status, p.encoderRow1[0]],
                    knobs: {
                        1: [status, p.encoderRow1[1]],
                        2: [status, p.encoderRow1[2]],
                        3: [status, p.encoderRow1[3]],
                    },
                },
            },
            {
                unitNumbers: [2, 4],
                components: {
                    effectFocusButton: [status, p.buttonRow2[4]],
                    enableButtons: {
                        1: [status, p.buttonRow2[5]],
                        2: [status, p.buttonRow2[6]],
                        3: [status, p.buttonRow2[7]],
                    },
                    dryWetKnob: [status, p.encoderRow1[4]],
                    knobs: {
                        1: [status, p.encoderRow1[5]],
                        2: [status, p.encoderRow1[6]],
                        3: [status, p.encoderRow1[7]],
                    },
                }
            }],
        };
    }
});

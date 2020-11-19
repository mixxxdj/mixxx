/**
 * Mixxx controller mapping for a Behringer B-Control BCR2000 controller.
 *
 * The BCR2000 is a general purpose controller that allows different mappings.
 * The mapping may be configured in `BCR2000.userConfig` which is structured as follows:
 *
 * userConfig
 * |
 * +- presets
 * |  +- onInit: Number of the BCR2000 preset that is activated when Mixxx is started
 * |  +- onShutdown: Number of the BCR2000 preset that is activated when Mixxx is shutting down
 * |
 * +- decks: an array of deck definitions
 * |  +- deck:
 * |     +- deckNumbers: as defined by `components.Deck`
 * |     +- components: an array of component definitions for the deck
 * |        +- component:
 * |           +- type:    Component type (constructor function, required)
 * |           |           Example: components.Button
 * |           +- midi:    MIDI address of the component (number array, required)
 * |           |           Example: [0xB0, 0x43]
 * |           +- shift:   Active only when a Shift button is pressed? (boolean, optional)
 * |           |           Example: true
 * |           +- options: Additional options for the component (object, required)
 * |                       Example: {key: "reverse"}
 * |
 * +-effectUnits: an array of effect unit definitions
 *   +- effectUnit
 *      +- unitNumbers: as defined by `components.EffectUnit`
 *      +- components: an object of component definitions for the effect unit. Each definition is
 *                     a key-value pair for a component of `components.EffectUnit` where key is the
 *                     name of the component and value is the MIDI address. Example:
 *                     `effectFocusButton: [0xB0, 0x15]`
 *
 */

/* Globally available objects are declared as variables to avoid linter errors */
var BCR2000 = BCR2000;

BCR2000.userConfig = function() {

    /* Shortcut variables */
    var p = BCR2000.preset;
    var c = components;
    var e = components.extension;
    var status = p.STATUS_CONTROL_CHANGE;

    return {
        presets: {
            onInit: 1,
            onShutdown: 31,
        },

        decks: [{
            deckNumbers: [1],
            components: [
                {
                    type: e.ShiftButton, midi: [status, p.buttonRow1[3]],
                    options: {key: "touch_shift", group: "[Controls]", target: BCR2000}
                },
                {
                    type: e.RangeAwareEncoder, midi: [status, p.pushEncoderGroup1[0].encoder],
                    options: {key: "pitch", bound: 6}
                },
                {
                    type: e.Trigger, midi: [status, p.pushEncoderGroup1[0].button],
                    options: {inKey: "pitch_set_zero"}
                },
                {
                    type: c.Button, midi: [status, p.buttonRow1[0]],
                    options: {key: "keylock"}
                },
                {
                    type: e.EnumToggleButton, midi: [status, p.buttonRow1[0]], shift: true,
                    options: {inKey: "vinylcontrol_mode", maxValue: 3}
                },
                {
                    type: e.BackLoopButton, midi: [status, p.buttonRow1[1]],
                    options: {outKey: "loop_enabled"}
                },
                {
                    type: c.Button, midi: [status, p.buttonRow1[1]], shift: true,
                    options: {key: "beatlooproll_activate"}
                },
                {
                    type: c.Button, midi: [status, p.buttonRow1[2]],
                    options: {key: "reverse"}
                },
                {
                    type: c.Button, midi: [status, p.buttonRow1[2]], shift: true,
                    options: {key: "reverseroll"}
                },
                {
                    type: e.LoopEncoder, midi: [status, p.pushEncoderGroup1[1].encoder],
                    options: {key: "beatloop_size"}
                },
                {
                    type: e.LoopMoveEncoder, midi: [status, p.pushEncoderGroup1[2].encoder],
                    options: {inKey: "loop_move", sizeControl: "beatjump_size"}
                },
                {
                    type: e.LoopEncoder, midi: [status, p.pushEncoderGroup1[3].encoder],
                    options: {key: "beatjump_size"}
                },
                {
                    type: c.EffectAssignmentButton, midi: [status, p.buttonBox[0]],
                    options: {effectUnit: 1, type: c.Button.prototype.types.push}
                },
                {
                    type: c.EffectAssignmentButton, midi: [status, p.buttonBox[1]],
                    options: {effectUnit: 2, type: c.Button.prototype.types.push}
                },
            ],
        },
        {
            deckNumbers: [2],
            components: [
                {
                    type: e.ShiftButton, midi: [status, p.buttonRow1[7]],
                    options: {key: "touch_shift", group: "[Controls]", target: BCR2000}
                },
                {
                    type: e.RangeAwareEncoder, midi: [status, p.pushEncoderGroup1[4].encoder],
                    options: {key: "pitch", bound: 6}
                },
                {
                    type: e.Trigger, midi: [status, p.pushEncoderGroup1[4].button],
                    options: {inKey: "pitch_set_zero"}
                },
                {
                    type: c.Button, midi: [status, p.buttonRow1[4]],
                    options: {key: "keylock"}
                },
                {
                    type: e.EnumToggleButton, midi: [status, p.buttonRow1[4]], shift: true,
                    options: {inKey: "vinylcontrol_mode", maxValue: 3}
                },
                {
                    type: e.BackLoopButton, midi: [status, p.buttonRow1[5]],
                    options: {outKey: "loop_enabled"}
                },
                {
                    type: c.Button, midi: [status, p.buttonRow1[5]], shift: true,
                    options: {key: "beatlooproll_activate"}
                },
                {
                    type: c.Button, midi: [status, p.buttonRow1[6]],
                    options: {key: "reverse"}
                },
                {
                    type: c.Button, midi: [status, p.buttonRow1[6]], shift: true,
                    options: {key: "reverseroll"}
                },
                {
                    type: e.LoopEncoder, midi: [status, p.pushEncoderGroup1[5].encoder],
                    options: {key: "beatloop_size"}
                },
                {
                    type: e.LoopMoveEncoder, midi: [status, p.pushEncoderGroup1[6].encoder],
                    options: {inKey: "loop_move", sizeControl: "beatjump_size"}
                },
                {
                    type: e.LoopEncoder, midi: [status, p.pushEncoderGroup1[7].encoder],
                    options: {key: "beatjump_size"}
                },
                {
                    type: c.EffectAssignmentButton, midi: [status, p.buttonBox[2]],
                    options: {effectUnit: 1, type: c.Button.prototype.types.push}
                },
                {
                    type: c.EffectAssignmentButton, midi: [status, p.buttonBox[3]],
                    options: {effectUnit: 2, type: c.Button.prototype.types.push}
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
};

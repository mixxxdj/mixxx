/**
 * Mixxx controller mapping for a Behringer DDM4000 mixer.
 */

var DDM4000 = new components.extension.GenericMidiController({
    configurationProvider: function() {

        /* Shortcut variables */
        var c = components;
        var cc = 0xB0;
        var note = 0x90;

        return {
            decks: [

                /* Channel 1 */
                {
                    deckNumbers: [1],
                    components: [
                        {type: c.Pot,    options: {midi: [cc,   0x07],  inKey: "volume"}}, // Volume
                        {type: c.Button, options: {midi: [note, 0x20],  inKey: ""}}, // CF Assign
                        {type: c.Button, options: {midi: [cc,   0x20], outKey: ""}}, // CF Assign: A
                        {type: c.Button, options: {midi: [cc,   0x21], outKey: ""}}, // CF Assign: B
                        {type: c.Button, options: {midi: [note, 0x3F],    key: "pfl", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // PFL
                        {type: c.Button, options: {midi: [note, 0x03],  inKey: ""}}, // Mode
                        {type: c.Button, options: {midi: [cc,   0x38], outKey: ""}}, // Mode: Multi
                        {type: c.Button, options: {midi: [cc,   0x37], outKey: ""}}, // Mode: Single
                    ],
                },

                /* Channel 2 */
                {
                    deckNumbers: [2],
                    components: [
                        {type: c.Pot,    options: {midi: [cc,   0x0B],  inKey: "volume"}}, // Volume
                        {type: c.Button, options: {midi: [note, 0x22],  inKey: ""}}, // CF Assign
                        {type: c.Button, options: {midi: [cc,   0x22], outKey: ""}}, // CF Assign: A
                        {type: c.Button, options: {midi: [cc,   0x23], outKey: ""}}, // CF Assign: B
                        {type: c.Button, options: {midi: [note, 0x49],    key: "pfl", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // PFL
                        {type: c.Button, options: {midi: [note, 0x07],  inKey: ""}}, // Mode
                        {type: c.Button, options: {midi: [cc,   0x42], outKey: ""}}, // Mode: Multi
                        {type: c.Button, options: {midi: [cc,   0x41], outKey: ""}}, // Mode: Single
                    ],
                },

                /* Channel 3 */
                {
                    deckNumbers: [3],
                    components: [
                        {type: c.Pot,    options: {midi: [cc,   0x0F],  inKey: "volume"}}, // Volume
                        {type: c.Button, options: {midi: [note, 0x24],  inKey: ""}}, // CF Assign
                        {type: c.Button, options: {midi: [cc,   0x24], outKey: ""}}, // CF Assign: A
                        {type: c.Button, options: {midi: [cc,   0x25], outKey: ""}}, // CF Assign: B
                        {type: c.Button, options: {midi: [note, 0x53],    key: "pfl", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // PFL
                        {type: c.Button, options: {midi: [note, 0x0B],  inKey: ""}}, // Mode
                        {type: c.Button, options: {midi: [cc,   0x4C], outKey: ""}}, // Mode: Multi
                        {type: c.Button, options: {midi: [cc,   0x4B], outKey: ""}}, // Mode: Single
                    ],
                },

                /* Channel 4 */
                {
                    deckNumbers: [4],
                    components: [
                        {type: c.Pot,    options: {midi: [cc,   0x13],  inKey: "volume"}}, // Volume
                        {type: c.Button, options: {midi: [note, 0x26],  inKey: ""}}, // CF Assign
                        {type: c.Button, options: {midi: [cc,   0x26], outKey: ""}}, // CF Assign: A
                        {type: c.Button, options: {midi: [cc,   0x27], outKey: ""}}, // CF Assign: B
                        {type: c.Button, options: {midi: [note, 0x5D],    key: "pfl", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // PFL
                        {type: c.Button, options: {midi: [note, 0x0F],  inKey: ""}}, // Mode
                        {type: c.Button, options: {midi: [cc,   0x56], outKey: ""}}, // Mode: Multi
                        {type: c.Button, options: {midi: [cc,   0x55], outKey: ""}}, // Mode: Single
                    ],
                },
            ],

            equalizerUnits: [

                /* Channel 1: Equalizer */
                {
                    channel: 1,
                    components: {
                        //               Low,           Mid,           High
                        parameterKnobs: {1: [cc, 0x06], 2: [cc, 0x05], 3: [cc, 0x04]},
                        //                 P3 (Low),        P2 (Mid),        P1 (High)
                        parameterButtons: {1: [note, 0x02], 2: [note, 0x01], 3: [note, 0x00]},
                    },
                    output: {
                        //                 P3 (Low),      P2 (Mid),      P1 (High)
                        parameterButtons: {1: [cc, 0x3D], 2: [cc, 0x3B], 3: [cc, 0x39]} // Amber
                        // parameterButtons: {1: [cc, 0x3E], 2: [cc, 0x3C], 3: [cc, 0x3A]} // Green
                    },
                },

                /* Channel 2: Equalizer */
                {
                    channel: 2,
                    components: {
                        //               Low,           Mid,           High
                        parameterKnobs: {1: [cc, 0x0A], 2: [cc, 0x09], 3: [cc, 0x08]},
                        //                 P3 (Low),        P2 (Mid),        P1 (High)
                        parameterButtons: {1: [note, 0x06], 2: [note, 0x05], 3: [note, 0x04]},
                    },
                    output: {
                        //                 P3 (Low),      P2 (Mid),      P1 (High)
                        parameterButtons: {1: [cc, 0x47], 2: [cc, 0x45], 3: [cc, 0x43]} // Amber
                        // parameterButtons: {1: [cc, 0x48], 2: [cc, 0x46], 3: [cc, 0x44]} // Green
                    },
                },

                /* Channel 3: Equalizer */
                {
                    channel: 3,
                    components: {
                        //               Low,           Mid,           High
                        parameterKnobs: {1: [cc, 0x0E], 2: [cc, 0x0D], 3: [cc, 0x0C]},
                        //                 P3 (Low),        P2 (Mid),        P1 (High)
                        parameterButtons: {1: [note, 0x0A], 2: [note, 0x09], 3: [note, 0x08]},
                    },
                    output: {
                        //                 P3 (Low),      P2 (Mid),      P1 (High)
                        parameterButtons: {1: [cc, 0x51], 2: [cc, 0x4F], 3: [cc, 0x4D]} // Amber
                        // parameterButtons: {1: [cc, 0x52], 2: [cc, 0x50], 3: [cc, 0x4E]} // Green
                    },
                },

                /* Channel 4: Equalizer */
                {
                    channel: 4,
                    components: {
                        //               Low,           Mid,           High
                        parameterKnobs: {1: [cc, 0x12], 2: [cc, 0x11], 3: [cc, 0x10]},
                        //                 P3 (Low),        P2 (Mid),        P1 (High)
                        parameterButtons: {1: [note, 0x0E], 2: [note, 0x0D], 3: [note, 0x0C]},
                    },
                    output: {
                        //                 P3 (Low),      P2 (Mid),      P1 (High)
                        parameterButtons: {1: [cc, 0x5B], 2: [cc, 0x59], 3: [cc, 0x57]} // Amber
                        // parameterButtons: {1: [cc, 0x5C], 2: [cc, 0x5A], 3: [cc, 0x58]} // Green
                    },
                },

                /* Microphone: Equalizer */
                {
                    channel: 5,
                    components: {
                        //               Low,           Mid,           High
                        parameterKnobs: {1: [cc, 0x02], 2: [cc, 0x01], 3: [cc, 0x00]},
                        //                 Talk On,         Mic FX On,       XMC On
                        parameterButtons: {1: [note, 0x34], 2: [note, 0x33], 3: [note, 0x32]},
                    },
                    //                          Talk On,       Mic FX On,     XMC On
                    output: {parameterButtons: {1: [cc, 0x34], 2: [cc, 0x33], 3: [cc, 0x32]}},
                },
            ],

            containers: [{
                components: [

                    /* Microphone */
                    {type: c.Button, options: {midi: [note, 0x31], group: "[Microphone]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Mic: Setup
                    {type: c.Button, options: {midi: [note, 0x35], group: "[Microphone]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Mic: On/Off

                    /* Crossfader */
                    {type: c.Pot,    options: {midi: [cc,   0x14], group: "[Master]",  inKey: ""}}, // Crossfader: Curve
                    {type: c.Pot,    options: {midi: [cc,   0x15], group: "[Master]",  inKey: "crossfader"}}, // Crossfader
                    {type: c.Button, options: {midi: [note, 0x17], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: A Full Freq

                    {type: c.Button, options: {midi: [note, 0x18], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: A High
                    {type: c.Button, options: {midi: [note, 0x19], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: A Mid
                    {type: c.Button, options: {midi: [note, 0x1A], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: A Low
                    {type: c.Button, options: {midi: [note, 0x1B], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: B Full Freq
                    {type: c.Button, options: {midi: [note, 0x1C], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: B High
                    {type: c.Button, options: {midi: [note, 0x1D], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: B Mid
                    {type: c.Button, options: {midi: [note, 0x1E], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: B Low
                    {type: c.Button, options: {midi: [note, 0x1F], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: On
                    {type: c.Button, options: {midi: [note, 0x28], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: Reverse Tap
                    {type: c.Button, options: {midi: [note, 0x29], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: Reverse Hold
                    {type: c.Button, options: {midi: [note, 0x2A], group: "[Master]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Crossfader: Bounce to MIDI Clock
                    {type: c.Button, options: {midi: [note, 0x2B], group: "[Master]",  inKey: ""}}, // Crossfader: Beat (Left)
                    {type: c.Button, options: {midi: [note, 0x2C], group: "[Master]",  inKey: ""}}, // Crossfader: Beat (Right)
                    {type: c.Button, options: {midi: [cc,   0x2B], group: "[Master]", outKey: ""}}, // Crossfader: Beat 1
                    {type: c.Button, options: {midi: [cc,   0x2C], group: "[Master]", outKey: ""}}, // Crossfader: Beat 2
                    {type: c.Button, options: {midi: [cc,   0x2D], group: "[Master]", outKey: ""}}, // Crossfader: Beat 4
                    {type: c.Button, options: {midi: [cc,   0x2E], group: "[Master]", outKey: ""}}, // Crossfader: Beat 8
                    {type: c.Button, options: {midi: [cc,   0x2F], group: "[Master]", outKey: ""}}, // Crossfader: Beat 16

                    /* Sampler */
                    {type: c.Pot,    options: {midi: [cc,   0x03], group: "[Sampler1]",  inKey: "volume"}}, // Sampler: Volume/Mix
                    {type: c.Button, options: {midi: [note, 0x5F], group: "[Sampler1]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Sampler: Insert
                    {type: c.Button, options: {midi: [note, 0x60], group: "[Sampler1]",  inKey: ""}}, // Sampler: REC Source (Right)
                    {type: c.Button, options: {midi: [note, 0x61], group: "[Sampler1]",  inKey: ""}}, // Sampler: REC Source (Left)
                    {type: c.Button, options: {midi: [cc,   0x60], group: "[Sampler1]", outKey: ""}}, // Sampler: REC Source 1
                    {type: c.Button, options: {midi: [cc,   0x61], group: "[Sampler1]", outKey: ""}}, // Sampler: REC Source 2
                    {type: c.Button, options: {midi: [cc,   0x62], group: "[Sampler1]", outKey: ""}}, // Sampler: REC Source 3
                    {type: c.Button, options: {midi: [cc,   0x63], group: "[Sampler1]", outKey: ""}}, // Sampler: REC Source 4
                    {type: c.Button, options: {midi: [cc,   0x64], group: "[Sampler1]", outKey: ""}}, // Sampler: REC Source Microphone
                    {type: c.Button, options: {midi: [cc,   0x65], group: "[Sampler1]", outKey: ""}}, // Sampler: REC Source Master
                    {type: c.Button, options: {midi: [note, 0x66], group: "[Sampler1]",  key: "pfl", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Sampler: PFL
                    {type: c.Button, options: {midi: [note, 0x67], group: "[Sampler1]",  inKey: ""}}, // Sampler: Sample Length (Right)
                    {type: c.Button, options: {midi: [note, 0x68], group: "[Sampler1]",  inKey: ""}}, // Sampler: Sample Length (Left)
                    {type: c.Button, options: {midi: [cc,   0x67], group: "[Sampler1]", outKey: ""}}, // Sampler: Sample Length 1
                    {type: c.Button, options: {midi: [cc,   0x68], group: "[Sampler1]", outKey: ""}}, // Sampler: Sample Length 2
                    {type: c.Button, options: {midi: [cc,   0x69], group: "[Sampler1]", outKey: ""}}, // Sampler: Sample Length 4
                    {type: c.Button, options: {midi: [cc,   0x6A], group: "[Sampler1]", outKey: ""}}, // Sampler: Sample Length 8
                    {type: c.Button, options: {midi: [cc,   0x6B], group: "[Sampler1]", outKey: ""}}, // Sampler: Sample Length 16
                    {type: c.Button, options: {midi: [cc,   0x6C], group: "[Sampler1]", outKey: ""}}, // Sampler: Sample Length ∞
                    {type: c.Button, options: {midi: [note, 0x6D], group: "[Sampler1]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Sampler: Record / In
                    {type: c.Button, options: {midi: [note, 0x6C], group: "[Sampler1]",  inKey: ""}}, // Sampler: Bank Assign
                    {type: c.Button, options: {midi: [note, 0x6E], group: "[Sampler1]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Sampler: Bank 1 Play / Out
                    {type: c.Button, options: {midi: [cc,   0x6F], group: "[Sampler1]", outKey: ""}}, // Sampler: Bank 1 Reverse
                    {type: c.Button, options: {midi: [cc,   0x70], group: "[Sampler1]", outKey: ""}}, // Sampler: Bank 1 Loop
                    {type: c.Button, options: {midi: [note, 0x71], group: "[Sampler1]",  inKey: ""}}, // Sampler: Bank 1 Mode
                    {type: c.Button, options: {midi: [cc,   0x71], group: "[Sampler1]", outKey: ""}}, // Sampler: Bank 1 Mode Amber
                    {type: c.Button, options: {midi: [cc,   0x72], group: "[Sampler1]", outKey: ""}}, // Sampler: Bank 1 Mode Blue
                    {type: c.Button, options: {midi: [note, 0x73], group: "[Sampler1]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Sampler: Bank 2 Play / Out
                    {type: c.Button, options: {midi: [cc,   0x74], group: "[Sampler1]", outKey: ""}}, // Sampler: Bank 2 Reverse
                    {type: c.Button, options: {midi: [cc,   0x75], group: "[Sampler1]", outKey: ""}}, // Sampler: Bank 2 Loop
                    {type: c.Button, options: {midi: [note, 0x76], group: "[Sampler1]",  inKey: ""}}, // Sampler: Bank 2 Mode
                    {type: c.Button, options: {midi: [cc,   0x76], group: "[Sampler1]", outKey: ""}}, // Sampler: Bank 2 Mode Amber
                    {type: c.Button, options: {midi: [cc,   0x77], group: "[Sampler1]", outKey: ""}}, // Sampler: Bank 2 Mode Blue
                    {type: c.Button, options: {midi: [note, 0x78], group: "[Sampler1]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Sampler: FX On
                    {type: c.Button, options: {midi: [note, 0x79], group: "[Sampler1]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Sampler: Select
                    {type: c.Button, options: {midi: [note, 0x7A], group: "[Sampler1]",  inKey: ""}}, // Sampler: CF Assign
                    {type: c.Button, options: {midi: [cc,   0x7A], group: "[Sampler1]", outKey: ""}}, // Sampler: CF Assign A
                    {type: c.Button, options: {midi: [cc,   0x7B], group: "[Sampler1]", outKey: ""}}, // Sampler: CF Assign B
                    {type: c.Button, options: {midi: [note, 0x7C], group: "[Sampler1]",  key: "", sendShifted: true, shiftChannel: true, shiftOffset: 0x20}}, // Sampler: CF Start
                ]
            }],
        };
    }
});

/* this statement exists to avoid linter errors */
DDM4000;

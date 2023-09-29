var MidiFighterTwister = {
    init: function() {
        var cc = 0xB0;

        function linearize(value) {
            return Math.pow(value / 4, 0.5) * 127;
        }

        MidiFighterTwister.connections = [
            engine.makeConnection("[Channel1]", "rate", function(value) {
                midi.sendShortMsg(cc, 0x04, (value + 1) / 2 * 127);
            }),
            engine.makeConnection("[Channel2]", "rate", function(value) {
                midi.sendShortMsg(cc, 0x07, (value + 1) / 2 * 127);
            }),
            engine.makeConnection("[Mixer]", "crossfader", function(value) {
                var scaled = (value + 1) / 2 * 127;
                midi.sendShortMsg(cc, 0x0C, scaled);
            }),
            engine.makeConnection("[Channel1]", "pregain", function(value) {
                midi.sendShortMsg(cc, 0x00, linearize(value));
            }),
            engine.makeConnection("[Channel2]", "pregain", function(value) {
                midi.sendShortMsg(cc, 0x03, linearize(value));
            }),
            engine.makeConnection("[Channel1]", "volume", function(value) {
                midi.sendShortMsg(cc, 0x08, linearize(value) * 2);
            }),
            engine.makeConnection("[Channel2]", "volume", function(value) {
                midi.sendShortMsg(cc, 0x0B, linearize(value) * 2);
            }),
            engine.makeConnection("[Channel1]", "pfl", function(value) {
                midi.sendShortMsg(cc + 1, 0x08, value * 127);
            }),
            engine.makeConnection("[Channel2]", "pfl", function(value) {
                midi.sendShortMsg(cc + 1, 0x0B, value * 127);
            }),
            // High EQ
            engine.makeConnection("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", function(value) {
                midi.sendShortMsg(cc, 0x01, linearize(value));
            }),
            engine.makeConnection("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", function(value) {
                midi.sendShortMsg(cc, 0x02, linearize(value));
            }),
            // Mid EQ
            engine.makeConnection("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", function(value) {
                midi.sendShortMsg(cc, 0x05, linearize(value));
            }),
            engine.makeConnection("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", function(value) {
                midi.sendShortMsg(cc, 0x06, linearize(value));
            }),
            // Low EQ
            engine.makeConnection("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", function(value) {
                midi.sendShortMsg(cc, 0x09, linearize(value));
            }),
            engine.makeConnection("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", function(value) {
                midi.sendShortMsg(cc, 0x0A, linearize(value));
            }),
            // Quick Effect Super Knob
            engine.makeConnection("[QuickEffectRack1_[Channel1]]", "super1", function(value) {
                midi.sendShortMsg(cc, 0x0D, value * 127);
            }),
            engine.makeConnection("[QuickEffectRack1_[Channel2]]", "super1", function(value) {
                midi.sendShortMsg(cc, 0x0E, value * 127);
            }),
        ];

        // Initialize Twister's LED
        for (var i = 0; i < MidiFighterTwister.connections.length; i++) {
            MidiFighterTwister.connections[i].trigger();
        }

        // Initialize BPM LED
        midi.sendShortMsg(cc, 0x00, 64)
        midi.sendShortMsg(cc, 0x03, 64)
    },
    shutdown: function() {
        for (var i = 0; i < MidiFighterTwister.connections.length; i++) {
            MidiFighterTwister.connections[i].disconnect();
        }
    },
    resetSuperKnob: function(channel, control, value, status, group) {
        engine.setValue(group, "super1", 0.5);
    },
}

var MidiFighterTwister = {
    init: function(id) {
        var cc = 0xB0;

        function linearize(value) {
            return Math.pow(value / 4, 0.5) * 128;
        }

        function updateBpmLed(group, ctrl, bpm) {
            var fileBpm = engine.getValue(group, "file_bpm");
            var bpmRangeRatio = engine.getValue(group, "rateRange");
            var minBpm = fileBpm - fileBpm * bpmRangeRatio;
            var bpmRange = fileBpm * bpmRangeRatio * 2;
            var scaled = Math.max(0, Math.min(127, (bpm - minBpm) / bpmRange * 128));
            midi.sendShortMsg(cc, ctrl, scaled);
        }

        MidiFighterTwister.connections = [
            engine.makeConnection("[Channel1]", "bpm", function(value) {
                updateBpmLed("[Channel1]", 0x00, value);
            }),
            engine.makeConnection("[Channel2]", "bpm", function(value) {
                updateBpmLed("[Channel2]", 0x03, value);
            }),
            engine.makeConnection("[Master]", "crossfader", function(value) {
                var scaled = (value + 1) / 2 * 128;
                midi.sendShortMsg(cc, 0x0C, scaled);
            }),
            engine.makeConnection("[Channel1]", "pregain", function(value) {
                midi.sendShortMsg(cc, 0x04, linearize(value));
            }),
            engine.makeConnection("[Channel2]", "pregain", function(value) {
                midi.sendShortMsg(cc, 0x07, linearize(value));
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
                midi.sendShortMsg(cc, 0x0D, value * 128);
            }),
            engine.makeConnection("[QuickEffectRack1_[Channel2]]", "super1", function(value) {
                midi.sendShortMsg(cc, 0x0E, value * 128);
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
    updateBpm: function(channel, control, value, status, group) {
        var fileBpm = engine.getValue(group, "file_bpm");
        var bpmRangeRatio = engine.getValue(group, "rateRange");
        var minBpm = fileBpm - fileBpm * bpmRangeRatio;
        var bpmRange = fileBpm * bpmRangeRatio * 2;
        var scaledBpm = value / 128 * bpmRange + minBpm;
        engine.setValue(group, "bpm", scaledBpm);
    },
    resetSuperKnob: function(channel, control, value, status, group) {
        engine.setValue(group, "super1", 0.5);
    },
}

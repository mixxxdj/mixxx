var DDJ200 = {
 headMix_switch : 0
};

DDJ200.init = function () {};

DDJ200.shutdown = function () {};

DDJ200.scratch = function (channel, control, value, status, group) {
    // For a control that centers on 0x40 (64):
    // Convert value down to +1/-1
    // Register the movement
    engine.scratchTick(script.deckFromGroup(group), value - 64);
};

DDJ200.jog = function (channel, control, value, status, group) {
    // For a control that centers on 0x40 (64):
    // Convert value down to +1/-1
    // Register the movement
    engine.setValue(group, 'jog', value - 64);
};

DDJ200.touch = function (channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    if (value == 0) {
        // disable scratch
        engine.scratchDisable(deckNumber);
    } else {
        // enable scratch
        var alpha = 1.0 / 8;
        engine.scratchEnable(deckNumber, 128, 33 + 1 / 3, alpha, alpha / 32);
    }
};

DDJ200.seek = function (channel, control, value, status, group) {
    var oldPos = engine.getValue(group, "playposition");
    // Since ‘playposition’ is normalized to unity, we need to scale by
    // song duration in order for the jog wheel to cover the same amount
    // of time given a constant turning angle.
    var duration = engine.getValue(group, "duration");
    var newPos = Math.max(0, oldPos + ((value - 64) * 0.2 / duration));
    engine.setValue(group, "playposition", newPos); // Strip search
};

DDJ200.headmix = function (channel, control, value, status, group) {
    if (value == 0) { return; }
    DDJ200.headMix_switch = 1 - DDJ200.headMix_switch;
    // toggle headMix knob to values of -1 and 1
    engine.setValue("[Master]", "headMix", 2 * DDJ200.headMix_switch - 1);
    
    // now switch headMix LED with midi.sendShortMsg(0x96, 0x63, 0x7F or 0);
    midi.sendShortMsg(0x90+channel, control, 0x7F * DDJ200.headMix_switch);
};

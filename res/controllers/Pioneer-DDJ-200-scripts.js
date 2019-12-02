var DDJ200 = {};

DDJ200.scratching = [];

DDJ200.init = function () {}

DDJ200.shutdown = function () {}

DDJ200.scratch = function (channel, control, value, status, group) {
    // enable scractch
    var deckNumber = script.deckFromGroup(group);
    if (!DDJ200.scratching[deckNumber]) {
        var alpha = 1.0 / 8;
        var beta = alpha / 32;
        engine.scratchEnable(deckNumber, 128, 33 + 1 / 3, alpha, beta);
        DDJ200.scratching[deckNumber] = true;
    }
    // For a control that centers on 0x40 (64):
    // Convert value down to +1/-1
    // Register the movement
    engine.scratchTick(deckNumber, value - 64);
}

DDJ200.jog = function (channel, control, value, status, group) {
    // disable scractch
    var deckNumber = script.deckFromGroup(group);
    if (DDJ200.scratching[deckNumber]) {
        engine.scratchDisable(script.deckFromGroup(group));
        DDJ200.scratching[deckNumber] = false;
    }
    // For a control that centers on 0x40 (64):
    // Convert value down to +1/-1
    // Register the movement
    engine.setValue(group, 'jog', value - 64);
}

// eslint-disable-next-line no-var
var TEK2 = {};
TEK2.jogScratchSensitivity = 1;
TEK2.jogPitchSensitivity = 1.6;

TEK2.init = function() {};

TEK2.shutdown = function() {};

// The button that enables/disables scratching
TEK2.wheelTouch = function(channel, control, value, status, group) {
    const deckNumber = script.deckFromGroup(group);
    if (value === 0x7f) {
    // If button downs
        const alpha = 1.0 / 8;
        const beta = alpha / 32;
        engine.scratchEnable(deckNumber, 100, 33 + 1 / 3, alpha, beta);
    } else {
    // If button up
        engine.scratchDisable(deckNumber);
    }
};

TEK2.wheelTurn = function(channel, control, value, status, group) {
    const newValue = value - 64;

    const deckNumber = script.deckFromGroup(group);
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue / TEK2.jogScratchSensitivity); // Scratch!
    } else {
        engine.setValue(group, "jog", newValue / TEK2.jogPitchSensitivity); // Pitch bend
    }
};

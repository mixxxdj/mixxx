/**
 * Krüger & Matz DJ-001 Controller Script
 *
 * @author Jairo Master Zion
 */

function KrugerMatzDJ001() {}

// -- Speed multipliers --------------------------------------------------------
// Tune these values if needed to adjust jog wheel scratch & pitch bend response.
KrugerMatzDJ001.deck1Forward = 0.33; // Deck 1 -- turning forward
KrugerMatzDJ001.deck1Backward = 0.33; // Deck 1 -- turning backward
KrugerMatzDJ001.deck2Forward = 0.33; // Deck 2 -- turning forward
KrugerMatzDJ001.deck2Backward = 0.33; // Deck 2 -- turning backward

// -- Wheel touch (enables / disables scratch mode) ----------------------------
KrugerMatzDJ001.wheelTouch = function (channel, control, value, status, group) {
    var currentDeck = (group === "[Channel2]") ? 2 : 1;

    if ((status & 0xF0) === 0x90) { // Note On = platter touched
        var alpha = 1.0 / 8;
        var beta = alpha / 32;
        engine.scratchEnable(currentDeck, 128, 33 + 1 / 3, alpha, beta);
    } else { // Note Off = platter released
        engine.scratchDisable(currentDeck, true); // true = ramp down naturally
    }
};

// -- Wheel turn (scratch or pitch bend) ---------------------------------------
KrugerMatzDJ001.wheelTurn = function (channel, control, value, status, group) {
    var currentDeck = (group === "[Channel2]") ? 2 : 1;

    // Ignore touch events that leak into wheelTurn (they cause a large position jump)
    if ((status & 0xF0) === 0x90) {
        return;
    }

    // Two's complement decode -- center is 64:
    //   value  1- 63 -> small positive delta (one direction)
    //   value 65-127 -> small negative delta (other direction, e.g. 127 = -1)
    var delta = (value < 64) ? value : (value - 128);

    // Pick the speed multiplier for this deck + direction
    var scale;
    if (currentDeck === 1) {
        scale = (delta > 0) ? KrugerMatzDJ001.deck1Forward : KrugerMatzDJ001.deck1Backward;
        delta = delta * -1; // invert physical direction for deck 1
    } else {
        scale = (delta > 0) ? KrugerMatzDJ001.deck2Forward : KrugerMatzDJ001.deck2Backward;
    }

    var newValue = delta * scale;

    if (engine.isScratching(currentDeck)) {
        engine.scratchTick(currentDeck, newValue); // Scratch
    } else {
        engine.setValue("[Channel" + currentDeck + "]", "jog", newValue); // Pitch bend
    }
};

// -- Rotary selector (library navigation) -------------------------------------
KrugerMatzDJ001.rotarySelector = function (channel, control, value, status, group) {
    if (value === 0x7F) {
        engine.setValue("[Library]", "MoveDown", 1);
    } else {
        engine.setValue("[Library]", "MoveUp", 1);
    }
};
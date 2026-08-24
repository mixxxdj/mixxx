/**
 * Krüger & Matz DJ-001 Controller Script
 *
 * @author Jairo Master Zion
 */

function KrugerMatzDJ001() { }

// -- Jog wheel sensitivity ---------------------------------------------------
// Tune this value if needed to adjust jog wheel scratch & pitch bend response.
KrugerMatzDJ001.jogSensitivity = 6;

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

    /*
    if (currentDeck === 1) {
        delta = -delta; // Invert physical direction for deck 1
    }
*/
    var newValue = delta * KrugerMatzDJ001.jogSensitivity;

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
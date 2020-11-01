var CMDPL1 = {};
 
CMDPL1.init = function (id, debugging) {
   // mixxx --controllerDebug
   print('> Init CMDPL1 OK');
}

CMDPL1.pitchSlider = function (channel, control, value, status, group) {   // Lower the sensitivity of the pitch slider
    var currentValue = engine.getValue(group,"rate");
    print('> Current : '+currentValue);
    print('> value : '+value);
    engine.setValue(group,"rate",(value-64)/64);
}

CMDPL1.wheel = function (channel, control, value, status, group) {   // Lower the sensitivity of the pitch slider
    var currentValue = engine.getValue(group,"rate");
    print('> Current : '+currentValue);
    print('> value : '+value);
    engine.setValue(group,"rate",currentValue+(value-64)/128);
   // engine.setValue(group,"rate",(value-64)/64);
}

// The wheel that actually controls the scratching
CMDPL1.wheelTurn = function (channel, control, value, status, group) {
    var deckNumber = script.deckFromGroup(group);
    print('> value : '+deckNumber);

    // A: For a control that centers on 0:
    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }
 
    // B: For a control that centers on 0x40 (64):
    var newValue = value - 64;
 
    // --- End choice
 
    // In either case, register the movement
    var deckNumber = script.deckFromGroup(group);
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue); // Scratch!
    } else {
        engine.setValue(group, 'jog', newValue); // Pitch bend
    }
}

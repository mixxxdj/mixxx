var TEK2 = {};
TEK2.jogScratchSensitivity = 1;
TEK2.jogPitchSensitivity = 3;

TEK2.init = function (id, debugging) {};

TEK2.shutdown = function () {};

// The button that enables/disables scratching
TEK2.wheelTouch = function (channel, control, value, status, group) {
  var deckNumber = script.deckFromGroup(group);
  if (value === 0x7F) {  // If button down
      var alpha = 1.0/8;
      var beta = alpha/32;
      engine.scratchEnable(deckNumber, 128, 33+1/3, alpha, beta);
  } else {    // If button up
      engine.scratchDisable(deckNumber);
  }
}

TEK2.wheelTurn = function (channel, control, value, status, group) {
  var newValue = value - 64;

  var deckNumber = script.deckFromGroup(group);
  if (engine.isScratching(deckNumber)) {
    engine.scratchTick(deckNumber, newValue / TEK2.jogScratchSensitivity); // Scratch!
  } else {
    engine.setValue(group, "jog", newValue / TEK2.jogPitchSensitivity); // Pitch bend
  }
};

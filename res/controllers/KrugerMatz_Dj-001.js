function KrugerMatz001(){}

KrugerMatz001.wheelTouch = function (channel, control, value, status, group) {
  if ('[Channel2]' == group) {
    currentDeck = 2;
  } else {
    currentDeck = 1;
  }

  if ((status & 0xF0) === 0x90) {    // If button down
      var alpha = 1.0/8;
      var beta = alpha/32;
      engine.scratchEnable(currentDeck, 128, 15+1/3, alpha, beta);
  } else {    // If button up
      engine.scratchDisable(currentDeck);
  }
}

// The wheel that actually controls the scratching
KrugerMatz001.wheelTurn = function (channel, control, value, status, group) {
  if ('[Channel2]' == group) {
    currentDeck = 2;
  } else {
    currentDeck = 1;
  }

  var newValue = value - 32;

  if (currentDeck == 1) {
      newValue = newValue * -1;
  }
  print(newValue)
  if (engine.isScratching(currentDeck)) {
      engine.scratchTick(currentDeck, newValue); // Scratch!
  } else {
      engine.setValue('[Channel'+currentDeck+']', 'jog', newValue); // Pitch bend
  }
}

KrugerMatz001.rotarySelector = function(channel, control, value, status) {
    if (value === 0x7F) {
        engine.setValue("[Library]", "MoveDown", 1);
    } else {
        engine.setValue("[Library]", "MoveUp", 1);
    }
}

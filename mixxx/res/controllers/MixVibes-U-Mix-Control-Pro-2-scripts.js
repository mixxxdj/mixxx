function ControlPro2() {}

ControlPro2.scratching = [];

// The button that enables/disables scratching
ControlPro2.wheelTouch = function (channel, control, value, status, group) {
  var deck = script.deckFromGroup(group);
  if (value == 0x7F) {
        var alpha = 1.0/8;
        var beta = alpha/32;
        engine.scratchEnable(deck, 340, 33+1/3, alpha, beta);
    }
    else {    // If button up
        engine.scratchDisable(deck);
    }
}
 
// The wheel that actually controls the scratching
ControlPro2.wheelTurn = function (channel, control, value, status, group) {
    // See if we're scratching. If not, do wheel jog.
    var deck = script.deckFromGroup(group);
    var newValue=(value-64);
    if (!engine.isScratching(deck)) {
        engine.setValue(group, "jog", newValue/4);
        return;
    }
    engine.scratchTick(deck,newValue);
}
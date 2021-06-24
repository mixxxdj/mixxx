var Prime4 = {};
function Prime4() {}

var primeHotcueColors = new ColorMapper({
  // default Engine Prime colors
  0xF4D345: 1, // yellow
  0xEF8130: 2, // orange
  0x9253C4: 3, // purple
  0xCE3232: 4, // red
  0x87C63E: 5, // lime
  0x1EBC61: 6, // green
  0x30C1B9: 7, // cyan
  0x4B88E2: 8, // blue
  // extra colors
  0xDE3294: 9, // pink
  0xFFFFFF: 10, // full white
  0xFFFF00: 11, // full yellow
  0xFFA000: 12, // full orange
  0xA000FF: 13, // full purple
  0xFF0000: 14, // full red
  0xA0FF00: 15, // full lime
  0x00FF00: 16, // full green
  0x00FFFF: 17, // full cyan
  0x0000FF: 18, // full blue
  0xFF00FF: 19, // full magenta
});

// common LED values for non-RGB lights
Prime4.colorCodes = {
  'off': 0x00,
  'dim': 0x01,
  'on': 0x7F
}



Prime4.activeButtons = {};

Prime4.unshiftedButtons = {
  censor = function (channel, control, value, status, group) {
    // code for reverse + slip mode
  }
};

Prime4.shiftedButtons = {
  censor = function (channel, control, value, status, group) {
    // code for reverse mode
  }
};

Prime4.init = function (id, debugging) {
	Prime4.activeButtons = Prime4.unshiftedButtons; // initialize script with shift button unpressed
  Prime4.initDeck('[Channel1]')
  Prime4.initDeck('[Channel2]')
}

// code for the shift buttons
Prime4.shiftButton = function (channel, control, value, status, group) {
  if (value === 127) {
    engine.connectControl(group, key, true);
    Prime4.activeButtons = Prime4.shiftedButtons;
    engine.connectControl(group, key);
  } else {
    engine.connectControl(group, key, true);
    Prime4.activeButtons = Prime4.unshiftedButtons;
    engine.connectControl(group, key);
  }
}

Prime4.shutdown = function () {
	// shutdown script
}







// allowing the Prime 4 to control all four decks as intended

Prime4.deck = {
  '[Channel1]': '[Channel1]',
  '[Channel2]': '[Channel2]'
}

Prime4.buttons = {
  '[Channel1]': {
    'deckToggle': // deck 1/3 LED MIDI code
  }
  '[Channel2]': {
    'deckToggle': // deck 2/4 LED MIDI code
  }
}
Prime4.buttons['[Channel3]'] = Prime4.buttons['[Channel1]']
Prime4.buttons['[Channel4]'] = Prime4.buttons['[Channel2]']

Prime4.channelRegEx = /\[Channel(\d+)\]/
Prime4.deckToggleButton = function (channel, control, value, status, group) {
  if (value) {
    var deckNumber = parseInt(
      Prime4.channelRegEx.exec(
        Prime4.deck[group]
      )[1]
    )
    if (deckNumber <= 2) {
      deckNumber += 2
    } else {
      deckNumber -= 2
    }
    Prime4.deck[group] = '[Channel' + deckNumber + ']'
    Prime4.initDeck(Prime4.deck[group])
  }
}

Prime4.initDeck = function (group) {
  var disconnectDeck = parseInt(Prime4.channelRegEx.exec(group)[1])
  if (disconnectDeck <= 2) {
    disconnectDeck += 2
  } else {
    disconnectDeck -= 2
  }
  Prime4.connectDeckControls('[Channel' + disconnectDeck +  ']')
  Prime4.connectDeckControls(group)
  midi.sendShortMsg(
    0x90,
    Prime4.buttons[group]['deckToggle'],
    (disconnectDeck > 2) ? 0x7F : 0x00
  )
}

Prime4.connectDeckControls = function (group, remove) {
  remove = (typeof remove !== 'undefined') ? remove : false
  var controlsToFunctions = {
    // 'mixxxControl': 'Prime4.scriptFunction'
  }
  for (var control in controlsToFunctions) {
    engine.connectControl(group, control, controlsToFunctions[control], remove)
    if (! remove) {
      engine.trigger(group, control)
    }
  }
}

Prime4.playButton = function (channel, control, value, status, group) {
  group = Prime4.deck[group]
  if (value) {
    engine.setValue(group, 'play', ! (engine.getValue(group, 'play')))
  }
}

var DJ202 = {};

DJ202.init = function () {

    DJ202.leftDeck = new DJ202.Deck([1,3], 0);
    DJ202.rightDeck = new DJ202.Deck([2,4], 1);
};

DJ202.shutdown = function () {
};

DJ202.browseEncoder = new components.Encoder({
    midi: [0xBF, 0x00],
    group: '[Playlist]',
    inKey: 'SelectTrackKnob',
    input: function (channel, control, value, status, group) {
        if (value === 1) {
            this.inSetParameter(1);
        } else if (value === 127) {
            this.inSetParameter(-1);
        }
    },
});

DJ202.crossfader = new components.Pot({
    midi: [0xBF, 0x08],
    group: '[Master]',
    inKey: 'crossfader',
});

DJ202.Deck = function (deckNumbers, offset) {
    components.Deck.call(this, deckNumbers);
    channel = offset+1;

    this.shiftButton = function (channel, control, value, status, group) {
        if (value === 127) {
            this.shift();
        } else {
            this.unshift();
        }
    };

    this.loadTrack = new components.Button({
        midi: [0x9F, 0x02 + offset],
        unshift: function () {
            this.inKey = 'LoadSelectedTrack';
        },
        shift: function () {
            this.inKey = 'eject';
        },
    });

    this.keylock = new components.Button({
        midi: [0x90 + offset, 0x0D],
        type: components.Button.prototype.types.toggle,
        inKey: 'keylock',
        outKey: 'keylock',
    });
    
    this.rate = new components.Pot({
        midi: [0xB0 + offset, 0x09],
        inKey: 'rate',
    });
    
    
    // ============================= JOG WHEELS =================================
    this.wheelTouch = function (channel, control, value, status, group) {
      if (value === 0x7F) {
            var alpha = 1.0/8;
            var beta = alpha/32;
            engine.scratchEnable(script.deckFromGroup(this.currentDeck), 512, 45, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    }
    
    this.wheelTurn = function (channel, control, value, status, group) {
        var newValue = value - 64;
        if (engine.isScratching(1)) {
            engine.scratchTick(script.deckFromGroup(this.currentDeck), newValue); // Scratch!
        } else {
            engine.setValue(this.currentDeck, 'jog', newValue); // Pitch bend
        }
    }
    

    // ============================= TRANSPORT ==================================
    this.play = new components.PlayButton([0x90 + offset, 0x00]); // LED doesn't stay on
    this.cue = new components.CueButton([0x90 + offset, 0x01]);
    this.sync = new components.SyncButton([0x90 + offset, 0x02]); // doesn't work properly

    // =============================== MIXER ====================================
    this.pregain = new components.Pot({
            midi: [0xB0 + offset, 0x16],
            inKey: 'pregain',
    });
    
    this.eqKnob = [];
    for (var k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            midi: [0xB0 + offset, 0x20 - k],
            group: '[EqualizerRack1_' + this.currentDeck + '_Effect1]',
            inKey: 'parameter' + k,
        });
    }
    
    this.filter = new components.Pot({
        midi: [0xB0 + offset, 0x1A],
        group: '[QuickEffectRack1_' + this.currentDeck + ']',
        inKey: 'super1',
    });

    this.pfl = new components.Button({
        midi: [0x90 + offset, 0x1B],
        type: components.Button.prototype.types.toggle,
        inKey: 'pfl',
        outKey: 'pfl',
        // missing: shift -> TAP
    });

    this.volume = new components.Pot({
        midi: [0xB0 + offset, 0x1C],
        inKey: 'volume',
    });

    this.reconnectComponents(function (component) {
        if (component.group === undefined) {
            component.group = this.currentDeck;
        }
    });
};
DJ202.Deck.prototype = new components.Deck();

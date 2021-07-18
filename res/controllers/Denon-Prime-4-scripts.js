var Prime4 = {};

Prime4.init = function() {
    Prime4.leftDeck = new Prime4.Deck([1, 3], 1);
    Prime4.rightDeck = new Prime4.Deck([2, 4], 2);
    // TODO: Add initialize sequence
};

Prime4.shutdown = function() {
    // TODO: Add shutdown sequence
};

/*
Prime4.Deck = function(deckNumbers, midiChannel) {
    components.Deck.call(this, deckNumbers);
    // TODO: Define necessary components
    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};
*/

Prime4.Deck.prototype = new components.Deck();
